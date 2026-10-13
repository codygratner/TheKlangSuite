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


def get_active_heavy_transcripts() -> list:
    """Inspects transcript sizes for currently active sessions (>5MB in last 4 hours)."""
    try:
        from tools.flush_context import get_transcript_telemetry
        telemetry = get_transcript_telemetry(recent_hours=4)
        return [t for t in telemetry if t.get("is_active_heavy")]
    except Exception:
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


def evaluate_system_health(assume_online: bool = False) -> Dict[str, Any]:
    """
    Evaluates complete system state across port :3030, Data Bus, and session memory.
    Returns state code (0: Green, 1: Yellow, 2: Red), tooltip label, and telemetry.
    """
    if assume_online:
        is_online = True
    else:
        api_data = probe_api_status(timeout=1.0)
        is_online = (api_data is not None)

    if not is_online:
        return {
            "state_code": STATE_CRITICAL,
            "state_name": STATE_NAMES[STATE_CRITICAL],
            "is_online": False,
            "port": DEFAULT_PORT,
            "tooltip": "N'kai: CRITICAL — Server Offline (:3030)",
            "pending_dispatches": 0,
            "active_heavy_transcripts": 0,
            "p0_mail_count": 0,
            "reasons": ["Server on port 3030 is offline or unresponsive"],
            "timestamp": time.time(),
        }

    # Port is online; check attention conditions
    pending_dispatches = get_pending_cosmos_dispatches()
    heavy_transcripts = get_active_heavy_transcripts()
    p0_mail = get_mailbag_p0_count()

    reasons = []
    if pending_dispatches > 0:
        reasons.append(f"{pending_dispatches} unread dispatch(es) for COSMOS")
    if len(heavy_transcripts) > 0:
        mb_str = ", ".join(f"{t['size_mb']}MB" for t in heavy_transcripts[:2])
        reasons.append(f"Active context heavy ({mb_str})")
    if p0_mail > 0:
        reasons.append(f"{p0_mail} P0 urgent mail card(s)")

    if reasons:
        tooltip = f"N'kai: Attention Needed\n• " + "\n• ".join(reasons)
        return {
            "state_code": STATE_ATTENTION,
            "state_name": STATE_NAMES[STATE_ATTENTION],
            "is_online": True,
            "port": DEFAULT_PORT,
            "tooltip": tooltip,
            "pending_dispatches": pending_dispatches,
            "active_heavy_transcripts": len(heavy_transcripts),
            "p0_mail_count": p0_mail,
            "reasons": reasons,
            "timestamp": time.time(),
        }

    return {
        "state_code": STATE_NOMINAL,
        "state_name": STATE_NAMES[STATE_NOMINAL],
        "is_online": True,
        "port": DEFAULT_PORT,
        "tooltip": "N'kai: Nominal (Port 3030 Live)",
        "pending_dispatches": 0,
        "active_heavy_transcripts": 0,
        "p0_mail_count": 0,
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
            cmd = f'powershell -Command "(Get-NetTCPConnection -LocalPort {DEFAULT_PORT} -ErrorAction SilentlyContinue).OwningProcess"'
            res = subprocess.run(cmd, shell=True, capture_output=True, text=True, check=False)
            owning_pid = res.stdout.strip()
            if owning_pid.isdigit():
                subprocess.run(["taskkill", "/F", "/PID", owning_pid], capture_output=True, check=False)
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
