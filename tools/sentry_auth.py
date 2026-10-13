#!/usr/bin/env python3
"""
tools/sentry_auth.py — Cryptographic Bearer Token Airlock for N'kai Sentry
==========================================================================
Generates, persists, and verifies pre-shared bearer tokens for LAN/Tailscale
endpoints on tools/nkai_serve.py (port 3030), immunizing the mobile companion
and Satellite relay against unauthorized remote prompt injection or scraping.
"""

from __future__ import annotations

import hmac
import json
import os
from pathlib import Path
import secrets
import sys
from typing import Optional

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

DEFAULT_AUTH_FILE = Path(__file__).resolve().parent.parent / ".agents" / "pipeline" / "sentry_auth.json"


def get_or_create_sentry_token(auth_file: Optional[Path | str] = None) -> str:
    """
    Retrieves the existing sentry token or generates a new cryptographically
    secure 32-byte hex token and persists it to auth_file.
    """
    target = Path(auth_file) if auth_file else DEFAULT_AUTH_FILE

    if target.exists():
        try:
            with open(target, "r", encoding="utf-8") as f:
                data = json.load(f)
                token = data.get("token")
                if token and isinstance(token, str) and len(token) >= 32:
                    return token
        except (json.JSONDecodeError, OSError):
            pass

    # Generate new token
    token = secrets.token_hex(32)
    target.parent.mkdir(parents=True, exist_ok=True)
    payload = {
        "token": token,
        "created_at": secrets.token_hex(4),  # lightweight salt/id
        "description": "Pre-shared Bearer token for N'kai Sentry mobile companion",
    }
    with open(target, "w", encoding="utf-8") as f:
        json.dump(payload, f, indent=2, ensure_ascii=False)

    return token


def verify_sentry_token(auth_header: Optional[str], expected_token: str) -> bool:
    """
    Validates an incoming Authorization header using constant-time comparison
    to prevent timing attacks. Expects header format: 'Bearer <token>'.
    """
    if not auth_header or not expected_token:
        return False

    parts = auth_header.strip().split()
    if len(parts) != 2:
        return False

    scheme, token = parts[0], parts[1]
    if scheme.lower() != "bearer":
        return False

    return hmac.compare_digest(token.strip(), expected_token.strip())


if __name__ == "__main__":
    t = get_or_create_sentry_token()
    print(f"Sentry Bearer Token: {t}")
