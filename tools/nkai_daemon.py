#!/usr/bin/env python3
"""
tools/nkai_daemon.py — N'kai Headless Supervisor & Telemetry Engine
===================================================================
Manages the always-on N'kai HTTP service (:3030), implements the Single-Instance
Latch Protocol, and provides ultra-fast in-process health telemetry for the
Windows System Tray LED.
"""

import os
import sys
import time
import json
import socket
import urllib.request
import urllib.error
import subprocess
from pathlib import Path
from typing import Dict, Any, Optional

# Enforce Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

VAULT_ROOT = Path(r"C:\_\Vault")
if str(VAULT_ROOT) not in sys.path:
    sys.path.insert(0, str(VAULT_ROOT))

CLAUDE_LEDGER_PATH = VAULT_ROOT / ".agents" / "scratch" / "claude_quota_ledger.json"
ACTIVE_JOB_LOCK = VAULT_ROOT / ".agents" / "scratch" / "claude_live" / "active_job.lock"

DEFAULT_PORT = 3030
STATUS_URL = f"http://127.0.0.1:{DEFAULT_PORT}/api/status"
PID_FILE = REPO_ROOT / ".agents" / "pipeline" / "nkai_daemon.pid"

STATE_NOMINAL = 0   # 🟢 Green
STATE_ATTENTION = 1 # 🟡 Yellow
STATE_CRITICAL = 2  # 🔴 Red

STATE_NAMES = {
    STATE_NOMINAL: "NOMINAL",
    STATE_ATTENTION: "ATTENTION",
    STATE_CRITICAL: "CRITICAL",
}


def is_port_in_use(port: int = DEFAULT_PORT, host: str = "127.0.0.1") -> bool:
    """Quick socket connect check with 0.1s timeout."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.settimeout(0.15)
        try:
            s.connect((host, port))
            return True
        except (socket.timeout, ConnectionRefusedError, OSError):
            return False


def probe_api_status(timeout: float = 1.0) -> Optional[Dict[str, Any]]:
    """Probes http://localhost:3030/api/status to verify daemon is healthy."""
    try:
        req = urllib.request.Request(STATUS_URL, headers={"User-Agent": "NkaiDaemonWatchdog/1.0"})
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            if resp.status == 200:
                data = json.loads(resp.read().decode("utf-8"))
                return data
    except Exception:
        pass
    return None


def get_pending_cosmos_dispatches() -> int:
    """In-process, zero-token check for unread COSMOS dispatches."""
    try:
        from tools.bus import resolve_bus_dir, load_dispatches
        bus_dir = resolve_bus_dir()
        dispatches = load_dispatches(bus_dir, include_archived=False)
        pending = [
            d for d in dispatches
            if str(d.get("status", "")).upper() == "PENDING"
            and str(d.get("to", "")).upper() in ("COSMOS", "ALL")
            and str(d.get("from", "")).upper() != "COSMOS"
        ]
        return len(pending)
    except Exception:
        return 0


MAX_ACTIVE_TRANSCRIPT_MB = 3.0


def get_active_transcript_stats() -> dict:
    """Inspects transcript size for the currently active session."""
    try:
        from tools.chat_engine import get_active_conversation_path
        active_path = get_active_conversation_path()
        if not active_path:
            return {"conversation_id": None, "size_bytes": 0, "size_mb": 0.0, "is_heavy": False}
        transcript = active_path / ".system_generated" / "logs" / "transcript.jsonl"
        if transcript.exists():
            st = transcript.stat()
            size_mb = round(st.st_size / (1024 * 1024), 2)
            is_heavy = st.st_size > (MAX_ACTIVE_TRANSCRIPT_MB * 1024 * 1024)
            return {
                "conversation_id": active_path.name,
                "size_bytes": st.st_size,
                "size_mb": size_mb,
                "is_heavy": is_heavy,
            }
        return {"conversation_id": active_path.name if active_path else None, "size_bytes": 0, "size_mb": 0.0, "is_heavy": False}
    except Exception:
        return {"conversation_id": None, "size_bytes": 0, "size_mb": 0.0, "is_heavy": False}


def get_active_heavy_transcripts() -> list:
    """Inspects transcript size for the currently active session (>3.0MB)."""
    stats = get_active_transcript_stats()
    if stats.get("is_heavy", False):
        return [stats]
    return []


def get_mailbag_p0_count() -> int:
    """Checks for urgent P0 unhandled mail cards."""
    queue_file = REPO_ROOT / ".agents" / "pipeline" / "mailbag" / "mail_queue.json"
    if not queue_file.exists():
        return 0
    try:
        data = json.loads(queue_file.read_text(encoding="utf-8"))
        p0_count = data.get("p0_count", 0)
        return int(p0_count)
    except Exception:
        return 0


_allowance_cache: Dict[str, Any] = {
    "data": None,
    "last_fetched": 0.0,
}


def _enrich_allowance_live_state(base: Dict[str, Any]) -> Dict[str, Any]:
    """Enriches cached ConnectRPC allowances with real-time Claude Pro rest windows and active job locks."""
    out = dict(base)
    now = time.time()

    # 1. Claude Pro Quota Ledger & Rest Window
    c_resting = False
    c_rest_until = 0
    c_rest_str = None
    if CLAUDE_LEDGER_PATH.exists():
        try:
            ledger = json.loads(CLAUDE_LEDGER_PATH.read_text(encoding="utf-8"))
            rest_until = ledger.get("rest_until", 0)
            if rest_until > now:
                c_resting = True
                c_rest_until = rest_until
                c_rest_str = time.strftime('%I:%M%p', time.localtime(rest_until))
                out["c_5h"] = 0.0  # Visually empty when in rest window
        except Exception:
            pass

    out["c_resting"] = c_resting
    out["c_rest_until"] = c_rest_until
    out["c_rest_str"] = c_rest_str

    # 2. Claude Active Job Execution Lock
    c_active = False
    c_active_pid = None
    if ACTIVE_JOB_LOCK.exists():
        try:
            j_data = json.loads(ACTIVE_JOB_LOCK.read_text(encoding="utf-8"))
            pid = j_data.get("pid", 0)
            if is_pid_alive(pid):
                c_active = True
                c_active_pid = pid
        except Exception:
            pass

    out["c_active"] = c_active
    out["c_active_pid"] = c_active_pid

    # 3. 4-Quadrant Canonical Layout Mapping (Standard H-1)
    # Left = 5-Hour, Right = Weekly, Top = Gemini, Bottom = Claude
    c_status = "ACTIVE" if c_active else ("RESTING" if c_resting else "NOMINAL")
    out["quadrants"] = {
        "top_left": {
            "label": "Gemini 5h",
            "fraction": float(out.get("g_5h", 1.0)),
            "engine": "gemini",
            "window": "5h"
        },
        "top_right": {
            "label": "Gemini Wk",
            "fraction": float(out.get("g_wk", 1.0)),
            "engine": "gemini",
            "window": "weekly"
        },
        "bottom_left": {
            "label": "Claude 5h",
            "fraction": float(out.get("c_5h", 1.0)),
            "engine": "claude",
            "window": "5h",
            "status": c_status,
            "resting": c_resting,
            "active": c_active,
            "rest_until_str": c_rest_str
        },
        "bottom_right": {
            "label": "Claude Wk",
            "fraction": float(out.get("c_wk", 1.0)),
            "engine": "claude",
            "window": "weekly"
        }
    }
    return out


def get_model_allowances(force_refresh: bool = False) -> Dict[str, Any]:
    """
    Synthesizes unified 4-quadrant allowance telemetry:
      • Top-Left:     Gemini 5-Hour Allowance
      • Top-Right:    Gemini Weekly Allowance
      • Bottom-Left:  Claude 5-Hour Allowance (ConnectRPC + Claude Pro session limit)
      • Bottom-Right: Claude Weekly Allowance
    """
    now = time.time()
    if not force_refresh and _allowance_cache["data"] and (now - _allowance_cache["last_fetched"] < 15.0):
        return _enrich_allowance_live_state(_allowance_cache["data"])

    res = {
        "g_5h": 1.0,
        "g_wk": 1.0,
        "c_5h": 1.0,
        "c_wk": 1.0,
        "timestamp": now
    }

    try:
        from tools import quota
        q_data = quota.get_quota_summary(force_refresh=force_refresh)
        for group in q_data.get("groups", []):
            name = str(group.get("displayName", "")).lower()
            is_gemini = "gemini" in name
            is_claude = "claude" in name or "gpt" in name
            for b in group.get("buckets", []):
                win = str(b.get("window", "")).lower()
                frac = float(b.get("remainingFraction", 1.0))
                if is_gemini:
                    if "5h" in win:
                        res["g_5h"] = frac
                    elif "week" in win:
                        res["g_wk"] = frac
                elif is_claude:
                    if "5h" in win:
                        res["c_5h"] = frac
                    elif "week" in win:
                        res["c_wk"] = frac
    except Exception:
        pass

    _allowance_cache["data"] = res
    _allowance_cache["last_fetched"] = now
    return _enrich_allowance_live_state(res)


def evaluate_system_health(assume_online: bool = False) -> Dict[str, Any]:
    """
    Evaluates complete system state across port :3030, Data Bus, session memory, and model allowances.
    Returns state code (0: Green, 1: Yellow, 2: Red), tooltip label, and telemetry.
    """
    allowances = get_model_allowances()

    if assume_online:
        is_online = True
    else:
        api_data = probe_api_status(timeout=1.0)
        is_online = (api_data is not None)

    g5_pct = allowances.get("g_5h", 1.0) * 100
    gw_pct = allowances.get("g_wk", 1.0) * 100
    c_status = "ACTIVE" if allowances.get("c_active") else (f"RESTING ({allowances.get('c_rest_str')})" if allowances.get("c_resting") else f"{allowances.get('c_5h', 1.0)*100:.0f}%")
    cw_pct = allowances.get("c_wk", 1.0) * 100
    quota_summary_line = f"Gemini: 5h {g5_pct:.0f}% · Wk {gw_pct:.0f}%\nClaude: 5h {c_status} · Wk {cw_pct:.0f}%"

    if not is_online:
        return {
            "state_code": STATE_CRITICAL,
            "state_name": STATE_NAMES[STATE_CRITICAL],
            "is_online": False,
            "port": DEFAULT_PORT,
            "tooltip": f"N'kai: CRITICAL — Server Offline (:3030)\n{quota_summary_line}",
            "pending_dispatches": 0,
            "active_heavy_transcripts": 0,
            "active_transcript_mb": 0.0,
            "p0_mail_count": 0,
            "allowances": allowances,
            "reasons": ["Server on port 3030 is offline or unresponsive"],
            "timestamp": time.time(),
        }

    # Port is online; check attention conditions
    pending_dispatches = get_pending_cosmos_dispatches()
    heavy_transcripts = get_active_heavy_transcripts()
    active_stats = get_active_transcript_stats()
    p0_mail = get_mailbag_p0_count()
    active_mb = active_stats.get("size_mb", 0.0)

    # Check Convention Radar status
    try:
        from tools import convention_radar
        c_state = convention_radar.load_state()
        c_state = convention_radar.evaluate_active_conventions(c_state)
        is_rush = (c_state.get("mode") == "CONVENTION_RUSH")
        con_interval = c_state.get("interval_hours", 12.0)
        con_names = [c["name"] for c in c_state.get("active_conventions", {}).values()]
    except Exception:
        is_rush = False
        con_interval = 12.0
        con_names = []

    reasons = []
    if pending_dispatches > 0:
        reasons.append(f"{pending_dispatches} unread dispatch(es) for COSMOS")
    if len(heavy_transcripts) > 0:
        reasons.append(f"Active context heavy ({active_mb:.2f} MB > {MAX_ACTIVE_TRANSCRIPT_MB:.1f} MB)")
    if p0_mail > 0:
        reasons.append(f"{p0_mail} P0 urgent mail card(s)")

    if reasons:
        tooltip = f"N'kai: Attention Needed (Chat Log: {active_mb:.2f} MB / {MAX_ACTIVE_TRANSCRIPT_MB:.1f} MB)\n{quota_summary_line}\n• " + "\n• ".join(reasons)
        return {
            "state_code": STATE_ATTENTION,
            "state_name": STATE_NAMES[STATE_ATTENTION],
            "is_online": True,
            "port": DEFAULT_PORT,
            "tooltip": tooltip[:127],
            "pending_dispatches": pending_dispatches,
            "active_heavy_transcripts": len(heavy_transcripts),
            "active_transcript_mb": active_mb,
            "p0_mail_count": p0_mail,
            "convention_rush": is_rush,
            "convention_names": con_names,
            "convention_interval_hours": con_interval,
            "allowances": allowances,
            "reasons": reasons,
            "timestamp": time.time(),
        }

    return {
        "state_code": STATE_NOMINAL,
        "state_name": STATE_NAMES[STATE_NOMINAL],
        "is_online": True,
        "port": DEFAULT_PORT,
        "tooltip": f"N'kai: Nominal · Chat: {active_mb:.2f}MB\n{quota_summary_line}"[:127],
        "pending_dispatches": 0,
        "active_heavy_transcripts": 0,
        "active_transcript_mb": active_mb,
        "p0_mail_count": 0,
        "convention_rush": is_rush,
        "convention_names": con_names,
        "convention_interval_hours": con_interval,
        "allowances": allowances,
        "reasons": [],
        "timestamp": time.time(),
    }


_last_start_time = 0.0


def start_server_process(silent: bool = True) -> bool:
    """
    Starts nkai_serve.py in the background if not already running.
    Implements Single-Instance Latch: if :3030 is already alive, attaches cleanly.
    Guarantees no duplicate processes or port collisions occur.
    """
    global _last_start_time
    now = time.time()

    if is_port_in_use(DEFAULT_PORT):
        if probe_api_status(timeout=0.5):
            return True
        # Port is held by a hung or deadlocked process; clean it up before launching new one
        stop_server_process()
        time.sleep(0.3)

    # Rate-limit process spawns to avoid cascades
    if now - _last_start_time < 5.0:
        return False
    _last_start_time = now

    serve_script = REPO_ROOT / "tools" / "nkai_serve.py"
    if not serve_script.exists():
        return False

    # Choose pythonw for windowless silent execution on Windows
    python_exe = sys.executable
    if silent and sys.platform == "win32":
        pythonw = Path(python_exe).parent / "pythonw.exe"
        if pythonw.exists():
            python_exe = str(pythonw)

    cmd = [python_exe, str(serve_script), "--lan", "--port", str(DEFAULT_PORT), "--no-browser"]

    creation_flags = 0
    if sys.platform == "win32":
        creation_flags = getattr(subprocess, "CREATE_NO_WINDOW", 0x08000000)

    try:
        proc = subprocess.Popen(
            cmd,
            cwd=str(REPO_ROOT),
            stdout=subprocess.DEVNULL if silent else None,
            stderr=subprocess.DEVNULL if silent else None,
            creationflags=creation_flags,
        )
        PID_FILE.parent.mkdir(parents=True, exist_ok=True)
        PID_FILE.write_text(str(proc.pid), encoding="utf-8")

        # Wait up to 4s for port to be listening
        for _ in range(20):
            time.sleep(0.2)
            if is_port_in_use(DEFAULT_PORT):
                return True
        return False
    except Exception as e:
        print(f"Error starting server process: {e}", file=sys.stderr)
        return False


def stop_server_process() -> bool:
    """Stops the managed server process using PID file or port finding."""
    stopped = False
    if PID_FILE.exists():
        try:
            pid = int(PID_FILE.read_text(encoding="utf-8").strip())
            if sys.platform == "win32":
                subprocess.run(["taskkill", "/F", "/PID", str(pid)], capture_output=True, check=False)
            else:
                os.kill(pid, 9)
            stopped = True
        except Exception:
            pass
        finally:
            PID_FILE.unlink(missing_ok=True)

    # If still listening on 3030, find and terminate owning PID on Windows
    if sys.platform == "win32" and is_port_in_use(DEFAULT_PORT):
        try:
            cmd = f'powershell -Command "(Get-NetTCPConnection -LocalPort {DEFAULT_PORT} -ErrorAction SilentlyContinue).OwningProcess | Select-Object -Unique"'
            res = subprocess.run(cmd, shell=True, capture_output=True, text=True, check=False)
            for line in res.stdout.splitlines():
                p = line.strip()
                if p.isdigit() and int(p) > 0:
                    subprocess.run(["taskkill", "/F", "/PID", p], capture_output=True, check=False)
                    stopped = True
        except Exception:
            pass

    return stopped


def restart_server_process(silent: bool = True) -> bool:
    """Recycles the server process."""
    stop_server_process()
    time.sleep(0.5)
    return start_server_process(silent=silent)


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="N'kai Headless Supervisor & Telemetry Engine")
    parser.add_argument("action", nargs="?", default="status", choices=["status", "start", "stop", "restart", "probe"])
    parser.add_argument("--json", action="store_true", help="Output health telemetry in raw JSON")

    args = parser.parse_args()

    if args.action == "status" or args.action == "probe":
        health = evaluate_system_health()
        if args.json:
            print(json.dumps(health, indent=2))
        else:
            icon = "🟢" if health["state_code"] == STATE_NOMINAL else ("🟡" if health["state_code"] == STATE_ATTENTION else "🔴")
            print(f"{icon} State: {health['state_name']} | Online: {health['is_online']} | Port: {health['port']}")
            if health["reasons"]:
                print("  Alerts:")
                for r in health["reasons"]:
                    print(f"  - {r}")
    elif args.action == "start":
        ok = start_server_process(silent=False)
        print("Server start:", "SUCCESS" if ok else "FAILED")
    elif args.action == "stop":
        ok = stop_server_process()
        print("Server stop:", "STOPPED" if ok else "NOT RUNNING")
    elif args.action == "restart":
        ok = restart_server_process(silent=False)
        print("Server restart:", "SUCCESS" if ok else "FAILED")
