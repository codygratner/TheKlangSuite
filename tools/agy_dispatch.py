#!/usr/bin/env python3
"""
tools/agy_dispatch.py — Sovereign Multi-Chat & Model Dispatcher for Antigravity
=============================================================================
Reverse-engineered ConnectRPC client capable of:
  1. Dynamically resolving live model placeholders (Gemini 4 Argon, Claude Opus, Flash).
  2. Enforcing the 20% Quota Floor Gate (prevents runaway quota collapse).
  3. Verifying target conversation idle status in conversation_summaries.db (prevents EBUSY).
  4. Dispatching prompt turns with explicit model & thinking level configurations.
"""

import os
import sys
import time
import json
import argparse
import sqlite3
from pathlib import Path
from typing import Dict, Any, List, Optional, Tuple

# Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    try:
        sys.stdout.reconfigure(encoding="utf-8")
    except Exception:
        pass
if hasattr(sys.stderr, "reconfigure"):
    try:
        sys.stderr.reconfigure(encoding="utf-8")
    except Exception:
        pass

# Add Vault and TKS roots to path
for r in [Path(r"c:\_\Vault"), Path(r"C:\Dev\TheKlangSuite"), Path(r"C:\Dev\Nkai")]:
    if str(r) not in sys.path:
        sys.path.insert(0, str(r))

from tools.agy_bridge import execute_rpc, discover_session, BRAIN_DIR, SUMMARIES_DB

# Canonical Cockpit Conversation IDs
KNOWN_CONVERSATIONS = {
    "ASTRAL_ABYSS": "6d5f68bf-c7b4-4129-bf75-1f5dc5e1fa41",
    "THE_TOWER": "3bb3d5eb-7be3-44f2-90ea-a4f6bbbb56d2",
    "GAME_ROOM": "d111d41d-7e07-40cc-bd61-caf640404a22",
    "KLANG_INDUSTRIES": "8fe6b97b-f027-4cc9-8cc2-d5f67ccfb0c9",
}

# Alias normalization dictionary
MODEL_ALIASES = {
    "argon": "Gemini 4 Argon (High)",
    "argon-high": "Gemini 4 Argon (High)",
    "argon-med": "Gemini 4 Argon (Medium)",
    "argon-medium": "Gemini 4 Argon (Medium)",
    "argon-low": "Gemini 4 Argon (Low)",
    "opus": "Claude Opus 5.5 (High)",
    "opus-high": "Claude Opus 5.5 (High)",
    "opus-med": "Claude Opus 5.5 (Medium)",
    "opus-low": "Claude Opus 5.5 (Low)",
    "sonnet": "Claude Sonnet 5.5 (High)",
    "sonnet-high": "Claude Sonnet 5.5 (High)",
    "sonnet-med": "Claude Sonnet 5.5 (Medium)",
    "sonnet-low": "Claude Sonnet 5.5 (Low)",
    "flash": "Gemini 3.8 Flash (High)",
    "flash-high": "Gemini 3.8 Flash (High)",
    "flash-med": "Gemini 3.8 Flash (Medium)",
    "flash-low": "Gemini 3.8 Flash (Low)",
    "gpt-oss": "GPT-OSS 120B (Medium)",
}


def get_live_model_catalog() -> Dict[str, Dict[str, Any]]:
    """
    Fetches the live clientModelConfigs from language_server.exe via ConnectRPC.
    Returns a dictionary keyed by exact label and normalized alias.
    """
    res = execute_rpc("GetCascadeModelConfigData", {})
    configs = res.get("clientModelConfigs", [])
    catalog = {}
    for c in configs:
        label = c.get("label", "")
        model_id = c.get("modelOrAlias", {}).get("model") or c.get("modelOrAlias", {}).get("alias")
        quota = c.get("quotaInfo", {}).get("remainingFraction", 1.0)
        reset_time = c.get("quotaInfo", {}).get("resetTime", "")
        catalog[label.lower()] = {
            "label": label,
            "model_id": model_id,
            "quota_remaining": quota,
            "reset_time": reset_time,
            "tag": c.get("tagTitle", ""),
        }
    return catalog


def resolve_model(query: str) -> Tuple[str, str]:
    """
    Resolves a friendly model query (e.g. 'argon', 'opus-high') to its live label and placeholder ID.
    Returns (label, model_id).
    """
    q = query.strip().lower()
    canonical_label = MODEL_ALIASES.get(q, q)
    catalog = get_live_model_catalog()

    # Exact match on label
    for k, v in catalog.items():
        if k == canonical_label.lower() or canonical_label.lower() in k:
            return v["label"], v["model_id"]

    raise ValueError(f"Unknown model query '{query}'. Available models: {list(MODEL_ALIASES.keys())}")


def check_quota_floor(min_threshold: float = 0.20) -> Tuple[bool, float, str]:
    """
    Enforces the 20% Quota Floor Gate against RetrieveUserQuotaSummary.
    Returns (is_safe, remaining_fraction, window_desc).
    """
    res = execute_rpc("RetrieveUserQuotaSummary", {})
    groups = res.get("response", {}).get("groups", [])
    for g in groups:
        for b in g.get("buckets", []):
            if b.get("bucketId") == "gemini-5h":
                rem = float(b.get("remainingFraction", 1.0))
                desc = b.get("description", "")
                return (rem >= min_threshold), rem, desc
    return True, 1.0, "Quota info unavailable"


def check_conversation_idle(conv_id: str) -> Tuple[bool, str]:
    """
    Inspects conversation_summaries.db to verify target chat is not currently executing.
    Returns (is_idle, status_string).
    """
    if not SUMMARIES_DB.exists():
        return True, "UNKNOWN"

    try:
        conn = sqlite3.connect(f"file:{SUMMARIES_DB}?mode=ro", uri=True)
        cur = conn.cursor()
        cur.execute("SELECT status, not_fully_idle FROM conversation_summaries WHERE conversation_id = ?", (conv_id,))
        row = cur.fetchone()
        conn.close()
        if row:
            status, not_idle = row
            if not_idle == 1 or status == "CASCADE_RUN_STATUS_RUNNING":
                return False, f"BUSY (status={status}, not_fully_idle={not_idle})"
            return True, f"IDLE (status={status})"
        return True, "IDLE (not in DB)"
    except Exception as e:
        return True, f"ERROR_CHECKING ({e})"


def dispatch_prompt(
    target_conv: str,
    prompt: str,
    model_query: str = "flash-high",
    bypass_quota_gate: bool = False,
    bypass_idle_gate: bool = False
) -> Dict[str, Any]:
    """
    Dispatches a prompt to target conversation with explicit model configuration.
    """
    # 1. Resolve Target Conversation ID
    conv_id = KNOWN_CONVERSATIONS.get(target_conv.upper(), target_conv)

    # 2. Resolve Model
    label, model_id = resolve_model(model_query)

    # 3. Quota Safety Gate
    if not bypass_quota_gate:
        safe, rem, desc = check_quota_floor(0.20)
        if not safe:
            raise RuntimeError(
                f"🛑 QUOTA FLOOR BREACH: Gemini 5-hour allowance is at {rem*100:.1f}% (below 20% safety threshold). "
                f"Dispatch rejected to protect account. {desc}"
            )

    # 4. Concurrency Idle Gate
    if not bypass_idle_gate:
        is_idle, status_desc = check_conversation_idle(conv_id)
        if not is_idle:
            raise RuntimeError(
                f"🛑 CONCURRENCY GATE: Target conversation {conv_id[:8]}... is currently {status_desc}. "
                "Dispatch rejected to prevent EBUSY trajectory corruption."
            )

    # 5. Build ConnectRPC Payload
    payload = {
        "cascadeId": conv_id,
        "messageText": prompt,
        "items": [
            {
                "text": prompt,
                "chunk": {
                    "case": "text",
                    "value": prompt
                }
            }
        ],
        "cascadeConfig": {
            "plannerConfig": {
                "planModel": model_id,
                "requestedModel": {
                    "choice": {
                        "case": "model",
                        "value": model_id
                    }
                },
                "supportsLatexRendering": True
            }
        }
    }

    t0 = time.perf_counter()
    res = execute_rpc("SendUserCascadeMessage", payload)
    elapsed_ms = (time.perf_counter() - t0) * 1000

    return {
        "status": "DISPATCHED",
        "conversation_id": conv_id,
        "model_label": label,
        "model_id": model_id,
        "elapsed_ms": elapsed_ms,
        "rpc_response": res,
    }


def main():
    parser = argparse.ArgumentParser(description="Antigravity Multi-Chat & Model Dispatcher")
    subparsers = parser.add_subparsers(dest="command")

    # Subcommand: models
    subparsers.add_parser("models", help="List live models and current quota")

    # Subcommand: probe
    subparsers.add_parser("probe", help="Probe language_server port, CSRF, and quota health")

    # Subcommand: dispatch
    p_disp = subparsers.add_parser("dispatch", help="Dispatch prompt to conversation with explicit model")
    p_disp.add_argument("--to", required=True, help="Target conversation ID or alias (THE_TOWER, ASTRAL_ABYSS, GAME_ROOM)")
    p_disp.add_argument("--model", default="flash-high", help="Model alias (argon-high, opus-high, flash-high)")
    p_disp.add_argument("--prompt", required=True, help="Prompt text to dispatch")
    p_disp.add_argument("--force-quota", action="store_true", help="Bypass 20 percent quota floor gate")
    p_disp.add_argument("--force-idle", action="store_true", help="Bypass conversation idle gate")

    args = parser.parse_args()

    if args.command == "models":
        catalog = get_live_model_catalog()
        print(f"\n📡 Live Antigravity Model Catalog ({len(catalog)} models):")
        print("-" * 75)
        for _, info in catalog.items():
            print(f"  • {info['label']:<30} | ID: {info['model_id']:<26} | Quota: {info['quota_remaining']*100:>5.1f}%")
        print("-" * 75)

    elif args.command == "probe":
        port, csrf = discover_session()
        safe, rem, desc = check_quota_floor(0.20)
        status_sym = "🟢 NOMINAL" if safe else "🔴 WARNING (LOW)"
        print(f"\n🔍 Antigravity Session Probe:")
        print(f"  • Port:             {port}")
        print(f"  • CSRF:             {csrf[:12]}...")
        print(f"  • 5-Hour Allowance: {rem*100:.1f}% ({status_sym})")
        print(f"  • Quota Status:     {desc}")

    elif args.command == "dispatch":
        try:
            print(f"🚀 Dispatching to {args.to} using model '{args.model}'...")
            res = dispatch_prompt(
                target_conv=args.to,
                prompt=args.prompt,
                model_query=args.model,
                bypass_quota_gate=args.force_quota,
                bypass_idle_gate=args.force_idle
            )
            print(f"✅ Turn successfully dispatched in {res['elapsed_ms']:.1f}ms!")
            print(f"   Target: {res['conversation_id']}")
            print(f"   Model:  {res['model_label']} ({res['model_id']})")
        except Exception as e:
            print(f"❌ Dispatch failed: {e}")
            sys.exit(1)

    else:
        parser.print_help()


if __name__ == "__main__":
    main()
