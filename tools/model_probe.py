#!/usr/bin/env python3
"""
tools/model_probe.py — Model Availability Sentinel & Circuit Breaker
====================================================================
Sub-second heartbeat probe that continuously monitors upstream AI gateway
latency and model availability, preventing operator hang loops during
HTTP 503 / 504 outages.

Features:
  1. Low-overhead (<10ms) local ConnectRPC / ModelConfig ping
  2. Circuit breaker tripwire after 2 consecutive failures
  3. Writes atomic state to .agents/cache/model_sentinel.json for Systray UI consumption
  4. Machine Invariant #10 zero-flicker subprocess compliance

Usage:
  python tools/model_probe.py probe
  python tools/model_probe.py status
  python tools/model_probe.py reset
"""

import os
import sys
import time
import json
import argparse
from pathlib import Path
from typing import Dict, Any

# Universal UTF-8 Windows Console Streams
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

VAULT_ROOT = Path(__file__).resolve().parent.parent
CACHE_DIR = VAULT_ROOT / ".agents" / "cache"
STATE_FILE = CACHE_DIR / "model_sentinel.json"

sys.path.insert(0, str(VAULT_ROOT))
from tools.ls_probe import call_rpc


def load_sentinel_state() -> Dict[str, Any]:
    """Loads current sentinel status from cache, or returns default baseline."""
    if STATE_FILE.exists():
        try:
            with open(STATE_FILE, "r", encoding="utf-8") as f:
                return json.load(f)
        except Exception:
            pass
    return {
        "status": "HEALTHY",
        "latency_ms": 0.0,
        "consecutive_failures": 0,
        "circuit_breaker_tripped": False,
        "last_probe_time": None,
        "last_error": None,
        "quota_summary": {}
    }


def save_sentinel_state(state: Dict[str, Any]) -> None:
    """Atomically saves sentinel status to disk."""
    CACHE_DIR.mkdir(parents=True, exist_ok=True)
    tmp_path = STATE_FILE.with_suffix(".tmp")
    with open(tmp_path, "w", encoding="utf-8") as f:
        json.dump(state, f, indent=2)
    tmp_path.replace(STATE_FILE)


def probe_gateway(timeout: float = 2.0) -> Dict[str, Any]:
    """Probes the upstream gateway via language_server ConnectRPC."""
    start_t = time.perf_counter()
    state = load_sentinel_state()
    now_iso = time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())

    try:
        data = call_rpc("GetCascadeModelConfigData", timeout=timeout)
        latency = (time.perf_counter() - start_t) * 1000.0

        quotas = {}
        for cfg in data.get("clientModelConfigs", []):
            label = cfg.get("label", "")
            qinfo = cfg.get("quotaInfo", {})
            rem = qinfo.get("remainingFraction")
            if rem is not None:
                if "Argon" in label or "Flash" in label:
                    quotas["google"] = round(float(rem), 3)
                elif "Opus" in label or "Sonnet" in label:
                    quotas["anthropic"] = round(float(rem), 3)

        state["status"] = "HEALTHY" if latency < 1200 else "DEGRADED"
        state["latency_ms"] = round(latency, 2)
        state["consecutive_failures"] = 0
        state["circuit_breaker_tripped"] = False
        state["last_probe_time"] = now_iso
        state["last_error"] = None
        state["quota_summary"] = quotas

    except Exception as e:
        latency = (time.perf_counter() - start_t) * 1000.0
        state["consecutive_failures"] += 1
        state["latency_ms"] = round(latency, 2)
        state["last_probe_time"] = now_iso
        state["last_error"] = str(e)

        if state["consecutive_failures"] >= 2:
            state["status"] = "OUTAGE"
            state["circuit_breaker_tripped"] = True
        else:
            state["status"] = "WARNING"

    save_sentinel_state(state)
    return state


def main():
    parser = argparse.ArgumentParser(description="Model Availability Sentinel & Circuit Breaker")
    subparsers = parser.add_subparsers(dest="command", required=True)

    subparsers.add_parser("probe", help="Run a single probe and update sentinel state")
    subparsers.add_parser("status", help="Read current sentinel state without probing")
    subparsers.add_parser("reset", help="Manually reset circuit breaker to HEALTHY")

    args = parser.parse_args()

    if args.command == "probe":
        res = probe_gateway()
        status_icon = "🟢" if res["status"] == "HEALTHY" else ("🟡" if res["status"] in ("DEGRADED", "WARNING") else "🔴")
        print(f"{status_icon} [Sentinel Probe] Status: {res['status']} | Latency: {res['latency_ms']}ms | Tripped: {res['circuit_breaker_tripped']}")
        if res.get("quota_summary"):
            print(f"   Quota Headroom: {res['quota_summary']}")
        if res.get("last_error"):
            print(f"   Last Error: {res['last_error']}")

    elif args.command == "status":
        state = load_sentinel_state()
        print(json.dumps(state, indent=2))

    elif args.command == "reset":
        state = load_sentinel_state()
        state["status"] = "HEALTHY"
        state["consecutive_failures"] = 0
        state["circuit_breaker_tripped"] = False
        state["last_error"] = None
        save_sentinel_state(state)
        print("✅ Circuit breaker reset to HEALTHY.")


if __name__ == "__main__":
    main()
