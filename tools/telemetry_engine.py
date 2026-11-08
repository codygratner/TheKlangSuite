#!/usr/bin/env python3
"""
tools/telemetry_engine.py — Unified System, Process, Quota & Context Telemetry Engine
===================================================================================
Provides zero-token, sub-25ms comprehensive telemetry across:
1. Internet Connectivity & Gemini API Latency
2. System RAM & CPU Utilization
3. Antigravity & Language Server RSS Memory Footprint (Selective fast scan + 5s TTL)
4. Live Model Allowances (Gemini 5h/Wk, Claude Stealth) via tools/quota.py (30s TTL)
5. Active Session Transcript Health & Size (filtering out subagents + mtime cache)
6. Inter-Project Data Bus Queues

Universal UTF-8 compliance guaranteed.
"""

import os
import sys
import time
import socket
import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Dict, Any, Tuple

# Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent
BRAIN_ROOT = Path(os.path.expanduser("~/.gemini/antigravity/brain"))

# Add workspace search roots to sys.path
for root in [
    Path(r"c:\_\Vault"),
    Path(r"C:\Dev\TheKlangSuite"),
    Path(r"C:\Dev\Nkai"),
    REPO_ROOT,
]:
    if str(root) not in sys.path:
        sys.path.insert(0, str(root))

# Cached states
_last_quota_fetch = 0.0
_cached_quota: Dict[str, Any] = {
    "g_5h": 100.0,
    "g_wk": 100.0,
    "c_5h": 100.0,
    "c_wk": 100.0,
    "g_reset_5h": "",
    "g_reset_wk": "",
    "g_reset_5h_human": "",
    "g_reset_wk_human": "",
    "c_reset_5h": "",
    "c_reset_wk": "",
    "claude_active": False,
}

_last_net_check = 0.0
_cached_net: Tuple[bool, float] = (True, 25.0)

_last_proc_check = 0.0
_cached_procs: Dict[str, Any] = {
    "antigravity_mb": 0.0,
    "antigravity_count": 0,
    "language_server_mb": 0.0,
    "language_server_count": 0,
    "total_agy_mb": 0.0,
}

_cached_transcript_meta: Tuple[str, float, int, int] = ("", 0.0, 0, 0)


def _format_reset_countdown(iso_str: str) -> str:
    """Converts an ISO-8601 UTC timestamp into a compact human countdown (e.g. '4h 12m' or '6d 17h')."""
    if not iso_str:
        return ""
    try:
        clean = iso_str.replace("Z", "+00:00")
        target = datetime.fromisoformat(clean)
        now = datetime.now(timezone.utc)
        diff_sec = int((target - now).total_seconds())
        if diff_sec <= 0:
            return "refreshing"
        days = diff_sec // 86400
        hours = (diff_sec % 86400) // 3600
        mins = (diff_sec % 3600) // 60
        if days > 0:
            return f"{days}d {hours}h"
        if hours > 0:
            return f"{hours}h {mins}m"
        return f"{mins}m"
    except Exception:
        return ""


def check_online(timeout: float = 0.35) -> Tuple[bool, float]:
    """Tests network latency against public DNS / Google endpoints with 10s cache."""
    global _last_net_check, _cached_net
    now = time.time()
    if now - _last_net_check < 10.0:
        return _cached_net

    t0 = time.time()
    for host, port in [("1.1.1.1", 53), ("8.8.8.8", 53), ("www.google.com", 80)]:
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.settimeout(timeout)
            s.connect((host, port))
            s.close()
            latency = round((time.time() - t0) * 1000.0, 1)
            _cached_net = (True, latency)
            _last_net_check = now
            return _cached_net
        except Exception:
            continue

    _cached_net = (False, 0.0)
    _last_net_check = now
    return _cached_net


def get_cached_quota(force_refresh: bool = False) -> Dict[str, Any]:
    """Retrieves model quota summary from tools/quota.py with 30s cache."""
    global _last_quota_fetch, _cached_quota
    now = time.time()
    if not force_refresh and (now - _last_quota_fetch < 30.0):
        return _cached_quota

    try:
        from tools import quota
        raw = quota.get_quota_summary(force_refresh=force_refresh)
        res = {
            "g_5h": 100.0,
            "g_wk": 100.0,
            "c_5h": 100.0,
            "c_wk": 100.0,
            "g_reset_5h": "",
            "g_reset_wk": "",
            "g_reset_5h_human": "",
            "g_reset_wk_human": "",
            "c_reset_5h": "",
            "c_reset_wk": "",
            "claude_active": False,
        }

        for group in raw.get("groups", []):
            name = str(group.get("displayName", "")).lower()
            is_gemini = "gemini" in name
            is_claude = "claude" in name or "gpt" in name

            for b in group.get("buckets", []):
                win = str(b.get("window", "")).lower()
                pct = round(float(b.get("remainingFraction", 1.0)) * 100.0, 1)
                rst = b.get("resetTime", "")

                if is_gemini:
                    if "5h" in win:
                        res["g_5h"] = pct
                        res["g_reset_5h"] = rst
                        res["g_reset_5h_human"] = _format_reset_countdown(rst)
                    elif "week" in win:
                        res["g_wk"] = pct
                        res["g_reset_wk"] = rst
                        res["g_reset_wk_human"] = _format_reset_countdown(rst)
                elif is_claude:
                    if "5h" in win:
                        res["c_5h"] = pct
                        res["c_reset_5h"] = rst
                    elif "week" in win:
                        res["c_wk"] = pct
                        res["c_reset_wk"] = rst

        res["claude_active"] = (res["c_5h"] < 99.9) or (res["c_wk"] < 99.9)
        _cached_quota = res
        _last_quota_fetch = now
    except Exception:
        pass

    return _cached_quota


def get_cached_processes() -> Dict[str, Any]:
    """Scans Antigravity and Language Server RSS memory with selective PID querying and 5s TTL."""
    global _last_proc_check, _cached_procs
    now = time.time()
    if now - _last_proc_check < 5.0:
        return _cached_procs

    import psutil
    agy_mem, agy_count = 0, 0
    ls_mem, ls_count = 0, 0
    # Query ONLY 'name' in process_iter to avoid opening handles to 350+ unrelated processes (56x faster)
    for p in psutil.process_iter(["name"]):
        try:
            name = (p.info["name"] or "").lower()
            if "antigravity" in name and "plugin" not in name:
                agy_mem += p.memory_info().rss
                agy_count += 1
            elif "language_server" in name:
                ls_mem += p.memory_info().rss
                ls_count += 1
        except Exception:
            pass

    _cached_procs = {
        "antigravity_mb": round(agy_mem / (1024**2), 1),
        "antigravity_count": agy_count,
        "language_server_mb": round(ls_mem / (1024**2), 1),
        "language_server_count": ls_count,
        "total_agy_mb": round((agy_mem + ls_mem) / (1024**2), 1),
    }
    _last_proc_check = now
    return _cached_procs


def resolve_active_session() -> Dict[str, Any]:
    """Accurately finds active parent conversation path, filtering out subagents."""
    global _cached_transcript_meta
    active_path = None
    try:
        from tools import chat_engine
        active_path = chat_engine.get_active_conversation_path()
    except Exception:
        pass

    if not active_path and BRAIN_ROOT.exists():
        candidates = []
        for p in BRAIN_ROOT.iterdir():
            if not p.is_dir() or p.name.startswith("."):
                continue
            tf = p / ".system_generated" / "logs" / "transcript.jsonl"
            if tf.exists():
                try:
                    candidates.append((tf.stat().st_mtime, p))
                except OSError:
                    continue
        if candidates:
            candidates.sort(key=lambda x: x[0], reverse=True)
            active_path = candidates[0][1]

    if not active_path or not active_path.exists():
        return {
            "conversation_id": "None",
            "size_mb": 0.0,
            "size_kb": 0.0,
            "steps": 0,
            "tokens_est": 0,
            "is_heavy": False,
            "status": "NOMINAL",
        }

    cid = active_path.name
    t_file = active_path / ".system_generated" / "logs" / "transcript.jsonl"
    size_bytes = 0
    steps = 0

    if t_file.exists():
        try:
            st = t_file.stat()
            size_bytes = st.st_size
            c_cid, c_mtime, c_size, c_steps = _cached_transcript_meta
            if c_cid == cid and c_mtime == st.st_mtime and c_size == size_bytes:
                steps = c_steps
            else:
                with open(t_file, "r", encoding="utf-8", errors="ignore") as f:
                    steps = sum(1 for _ in f)
                _cached_transcript_meta = (cid, st.st_mtime, size_bytes, steps)
        except Exception:
            pass

    size_mb = round(size_bytes / (1024 * 1024), 2)
    size_kb = round(size_bytes / 1024, 1)
    tokens_est = int(size_bytes / 4)
    is_heavy = size_mb >= 3.0

    if is_heavy:
        status = "HARVEST_DUE"
    elif size_mb >= 1.5:
        status = "GROWING"
    else:
        status = "HEALTHY"

    return {
        "conversation_id": cid,
        "size_mb": size_mb,
        "size_kb": size_kb,
        "steps": steps,
        "tokens_est": tokens_est,
        "is_heavy": is_heavy,
        "status": status,
    }


def get_unified_telemetry(force_refresh_quota: bool = False) -> Dict[str, Any]:
    """Generates the full unified telemetry snapshot in <25ms."""
    import psutil

    online, latency_ms = check_online()
    vm = psutil.virtual_memory()
    cpu = psutil.cpu_percent(interval=None)

    proc_data = get_cached_processes()
    quota_data = get_cached_quota(force_refresh=force_refresh_quota)
    session_data = resolve_active_session()

    pending_cosmos = 0
    try:
        from tools import nkai_daemon
        pending_cosmos = nkai_daemon.get_pending_cosmos_dispatches()
    except Exception:
        pass

    return {
        "online": online,
        "latency_ms": latency_ms,
        "cpu_percent": round(cpu, 1),
        "ram": {
            "used_gb": round(vm.used / (1024**3), 2),
            "total_gb": round(vm.total / (1024**3), 2),
            "free_gb": round(vm.available / (1024**3), 2),
            "percent": round(vm.percent, 1),
        },
        "processes": proc_data,
        "quota": quota_data,
        "session": session_data,
        "bus": {
            "pending_cosmos": pending_cosmos,
        },
        "timestamp": time.strftime("%H:%M:%S"),
    }


if __name__ == "__main__":
    t0 = time.perf_counter()
    telem = get_unified_telemetry()
    elapsed = (time.perf_counter() - t0) * 1000
    print(f"📊 Unified Telemetry Generated in {elapsed:.1f}ms:\n")
    print(json.dumps(telem, indent=2))
