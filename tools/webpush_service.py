#!/usr/bin/env python3
"""
tools/webpush_service.py — Native Web Push Service for N'kai Sovereign Deck (Flight 25)
========================================================================================
Implements RFC 8291 / RFC 8292 Web Push notifications for iOS 16.4+ Safari PWA,
Android Chrome, and desktop browsers using VAPID keys and pywebpush.
"""

from __future__ import annotations

import base64
import json
import os
from pathlib import Path
import sys
from typing import Any, Dict, List, Optional

# Universal UTF-8 Environment Invariant
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent
PIPELINE_DIR = REPO_ROOT / ".agents" / "pipeline"
VAPID_FILE = PIPELINE_DIR / "vapid_keys.json"
SUBSCRIPTIONS_FILE = PIPELINE_DIR / "push_subscriptions.json"

DEFAULT_SUBJECT = "mailto:codygratner@gmail.com"


def _b64url(b: bytes) -> str:
    """Encodes bytes into URL-safe base64 without trailing padding."""
    return base64.urlsafe_b64encode(b).decode("utf-8").rstrip("=")


def get_or_create_vapid_keys() -> Dict[str, str]:
    """Retrieves or generates ECDSA P-256 VAPID keys for Web Push."""
    PIPELINE_DIR.mkdir(parents=True, exist_ok=True)

    if VAPID_FILE.exists():
        try:
            with open(VAPID_FILE, "r", encoding="utf-8") as f:
                data = json.load(f)
                if "public_key" in data and "private_pem" in data:
                    return data
        except Exception:
            pass

    # Generate new P-256 VAPID keys
    from py_vapid import Vapid
    from cryptography.hazmat.primitives import serialization

    v = Vapid()
    v.generate_keys()

    pub_bytes = v.public_key.public_bytes(
        encoding=serialization.Encoding.X962,
        format=serialization.PublicFormat.UncompressedPoint,
    )
    priv_pem = v.private_pem().decode("utf-8")
    pub_b64 = _b64url(pub_bytes)

    data = {
        "public_key": pub_b64,
        "private_pem": priv_pem,
        "subject": DEFAULT_SUBJECT,
    }

    with open(VAPID_FILE, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2, ensure_ascii=False)

    return data


def load_subscriptions() -> List[Dict[str, Any]]:
    """Loads all active push subscriptions from disk."""
    if not SUBSCRIPTIONS_FILE.exists():
        return []
    try:
        with open(SUBSCRIPTIONS_FILE, "r", encoding="utf-8") as f:
            data = json.load(f)
            return data if isinstance(data, list) else []
    except Exception:
        return []


def save_subscription(sub: Dict[str, Any]) -> bool:
    """Adds or updates a push subscription on disk."""
    PIPELINE_DIR.mkdir(parents=True, exist_ok=True)
    subs = load_subscriptions()
    endpoint = sub.get("endpoint")
    if not endpoint:
        return False

    # Check if already registered
    existing_idx = None
    for i, s in enumerate(subs):
        if s.get("endpoint") == endpoint:
            existing_idx = i
            break

    if existing_idx is not None:
        subs[existing_idx] = sub
    else:
        subs.append(sub)

    with open(SUBSCRIPTIONS_FILE, "w", encoding="utf-8") as f:
        json.dump(subs, f, indent=2, ensure_ascii=False)

    return True


def remove_subscription(endpoint: str) -> None:
    """Removes an expired or invalid subscription endpoint."""
    subs = load_subscriptions()
    filtered = [s for s in subs if s.get("endpoint") != endpoint]
    with open(SUBSCRIPTIONS_FILE, "w", encoding="utf-8") as f:
        json.dump(filtered, f, indent=2, ensure_ascii=False)


def send_push_notification(
    title: str,
    body: str,
    url: str = "/",
    tag: str = "nkai-alert",
    channel: str = "research",
) -> Dict[str, Any]:
    """
    Broadcasts a Web Push notification to all registered client devices.
    Automatically prunes expired endpoints (HTTP 404/410).
    """
    from pywebpush import webpush, WebPushException
    from py_vapid import Vapid

    keys = get_or_create_vapid_keys()
    subs = load_subscriptions()
    vapid_instance = Vapid.from_pem(keys["private_pem"].encode("utf-8"))

    payload = json.dumps({
        "title": title,
        "body": body,
        "url": url,
        "tag": tag,
        "channel": channel,
    })

    success_count = 0
    failed_count = 0
    pruned = []

    for sub in subs:
        endpoint = sub.get("endpoint")
        try:
            webpush(
                subscription_info=sub,
                data=payload,
                vapid_private_key=vapid_instance,
                vapid_claims={"sub": keys["subject"]},
                ttl=3600,
            )
            success_count += 1
        except WebPushException as ex:
            failed_count += 1
            response = getattr(ex, "response", None)
            if response is not None and response.status_code in (404, 410):
                # Subscription expired or unregistered by user
                pruned.append(endpoint)
        except Exception:
            failed_count += 1

    # Prune dead subscriptions
    if pruned:
        for dead_ep in pruned:
            remove_subscription(dead_ep)

    return {
        "ok": True,
        "sent": success_count,
        "failed": failed_count,
        "pruned": len(pruned),
        "total_active": len(subs) - len(pruned),
    }


if __name__ == "__main__":
    k = get_or_create_vapid_keys()
    print("VAPID Public Key:", k["public_key"])
    subs = load_subscriptions()
    print(f"Active Subscriptions: {len(subs)}")
