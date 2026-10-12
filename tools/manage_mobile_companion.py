#!/usr/bin/env python3
"""
N'kai Mobile Companion Lifecycle Manager (tools/manage_mobile_companion.py)
Part of N'kai v0.3.0 Mobile Companion & Remote Architecture.

Provides status probing, background daemon start/stop/restart, and zero-token
idempotent ensure checks for couch triage over Tailscale.
Universal UTF-8 compliance guaranteed.
"""

import argparse
import json
import os
from pathlib import Path
import signal
import socket
import subprocess
import sys
import time
from typing import Any, Dict, Optional
import urllib.request
import urllib.error

# Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

PID_FILE = REPO_ROOT / ".agents" / "pipeline" / "mobile_companion.pid"
DEFAULT_PORT = 3030


def is_port_in_use(port: int = DEFAULT_PORT, host: str = "127.0.0.1") -> bool:
    """Checks whether the target port is actively accepting TCP connections."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.settimeout(0.5)
        return s.connect_ex((host, port)) == 0


def fetch_companion_status(port: int = DEFAULT_PORT) -> Optional[Dict[str, Any]]:
    """Queries /api/status from the local running mobile server."""
    try:
        url = f"http://127.0.0.1:{port}/api/status"
        req = urllib.request.Request(url, headers={"User-Agent": "NkaiManager/1.0"})
        with urllib.request.urlopen(req, timeout=1.0) as resp:
            if resp.status == 200:
                data = json.loads(resp.read().decode("utf-8"))
                return data
    except Exception:
        pass
    return None


def get_pid_from_file() -> Optional[int]:
    """Reads stored process ID if PID file exists."""
    if PID_FILE.exists():
        try:
            pid = int(PID_FILE.read_text(encoding="utf-8").strip())
            return pid
        except Exception:
            pass
    return None


def is_process_alive(pid: int) -> bool:
    """Verifies whether a PID is currently running."""
    if sys.platform == "win32":
        try:
            # Query tasklist for PID
            out = subprocess.check_output(
                ["tasklist", "/FI", f"PID eq {pid}", "/FO", "CSV", "/NH"],
                text=True,
                encoding="utf-8",
                errors="replace",
            )
            return str(pid) in out
        except Exception:
            return False
    else:
        try:
            os.kill(pid, 0)
            return True
        except OSError:
            return False


def cmd_status(args: argparse.Namespace) -> int:
    """Probes status of N'kai Mobile Companion."""
    port = args.port
    in_use = is_port_in_use(port)
    status_data = fetch_companion_status(port)
    pid = get_pid_from_file()

    print("=" * 64)
    print("  📱 N'KAI MOBILE COMPANION STATUS")
    print("=" * 64)

    if in_use and status_data:
        print(f"  🟢 Status:          ONLINE & RESPONSIVE")
        print(f"  ⚡ Port:            {port}")
        if pid:
            print(f"  🆔 Process PID:     {pid}")
        ts_ip = status_data.get("tailscale_ip", "Unknown")
        print(f"  🔒 Tailscale IP:    {ts_ip}")
        print(f"  📬 Mobile Mailbag:  http://{ts_ip}:{port}/morning_mail_inbox.html")
        print(f"  🛫 Flight Runway:   http://{ts_ip}:{port}/runway_board.html")
        print(f"  📲 Pairing Route:   http://{ts_ip}:{port}/qr")
        print("=" * 64)
        return 0
    elif in_use:
        print(f"  🟡 Status:          PORT {port} OCCUPIED (Foreign or initializing process)")
        if pid:
            print(f"  🆔 Stored PID:      {pid} (Alive: {is_process_alive(pid)})")
        print("=" * 64)
        return 1
    else:
        print(f"  🔴 Status:          OFFLINE (Not listening on port {port})")
        if pid and is_process_alive(pid):
            print(f"  ⚠️ Stored PID {pid} is running but port {port} is closed")
        print(f"  💡 Run: 'python tools/manage_mobile_companion.py start' to launch")
        print("=" * 64)
        return 2


def cmd_start(args: argparse.Namespace) -> int:
    """Launches N'kai Mobile Companion as a persistent background daemon."""
    port = args.port
    if is_port_in_use(port):
        status = fetch_companion_status(port)
        if status:
            print(f"✅ N'kai Mobile Companion is ALREADY running on port {port}!")
            ts_ip = status.get("tailscale_ip", "127.0.0.1")
            print(f"   Tailscale URL: http://{ts_ip}:{port}/morning_mail_inbox.html")
            return 0
        else:
            print(f"⚠️ Port {port} is occupied by another process.")
            return 1

    script_path = REPO_ROOT / "tools" / "nkai_serve.py"
    if not script_path.exists():
        print(f"❌ Error: Could not find {script_path}", file=sys.stderr)
        return 1

    cmd = [
        sys.executable,
        str(script_path),
        "--port",
        str(port),
        "--lan",
        "--no-browser",
    ]

    print(f"🚀 Spawning N'kai Mobile Companion daemon on port {port}...")

    if sys.platform == "win32":
        # Windows detached background process
        creationflags = (
            subprocess.DETACHED_PROCESS
            | subprocess.CREATE_NEW_PROCESS_GROUP
        )
        proc = subprocess.Popen(
            cmd,
            cwd=str(REPO_ROOT),
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            stdin=subprocess.DEVNULL,
            creationflags=creationflags,
            close_fds=True,
        )
    else:
        proc = subprocess.Popen(
            cmd,
            cwd=str(REPO_ROOT),
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            stdin=subprocess.DEVNULL,
            start_new_session=True,
        )

    PID_FILE.parent.mkdir(parents=True, exist_ok=True)
    PID_FILE.write_text(str(proc.pid), encoding="utf-8")

    # Wait up to 3 seconds for server to bind
    for _ in range(15):
        time.sleep(0.2)
        if is_port_in_use(port):
            status = fetch_companion_status(port)
            ts_ip = status.get("tailscale_ip", "100.87.196.60") if status else "100.87.196.60"
            print(f"✅ N'kai Mobile Companion launched successfully!")
            print(f"   🆔 PID: {proc.pid}")
            print(f"   📬 Mobile URL: http://{ts_ip}:{port}/morning_mail_inbox.html")
            print(f"   🛫 Runway URL: http://{ts_ip}:{port}/runway_board.html")
            return 0

    print(f"⚠️ Process started with PID {proc.pid}, but port {port} did not respond within 3 seconds.")
    return 1


def cmd_stop(args: argparse.Namespace) -> int:
    """Stops the running N'kai Mobile Companion daemon."""
    pid = get_pid_from_file()
    stopped = False

    if pid and is_process_alive(pid):
        print(f"🛑 Terminating process with PID {pid}...")
        try:
            if sys.platform == "win32":
                subprocess.run(["taskkill", "/F", "/T", "/PID", str(pid)], capture_output=True, check=False)
            else:
                os.kill(pid, signal.SIGTERM)
            stopped = True
        except Exception as e:
            print(f"Error terminating PID {pid}: {e}")

    # Also clean up PID file
    if PID_FILE.exists():
        try:
            PID_FILE.unlink()
        except Exception:
            pass

    # Give OS a moment to free port
    time.sleep(0.5)

    if not is_port_in_use(args.port):
        print("✅ N'kai Mobile Companion stopped cleanly.")
        return 0
    else:
        print(f"⚠️ Port {args.port} is still in use.")
        return 1


def cmd_restart(args: argparse.Namespace) -> int:
    """Restarts the daemon."""
    cmd_stop(args)
    return cmd_start(args)


def cmd_ensure(args: argparse.Namespace) -> int:
    """Idempotent ensure: starts server if offline, does nothing if online."""
    if is_port_in_use(args.port) and fetch_companion_status(args.port):
        status = fetch_companion_status(args.port)
        ts_ip = status.get("tailscale_ip", "100.87.196.60") if status else "100.87.196.60"
        if not args.quiet:
            print(f"🟢 N'kai Mobile Companion is active: http://{ts_ip}:{args.port}/morning_mail_inbox.html")
        return 0
    return cmd_start(args)


def main():
    parser = argparse.ArgumentParser(
        description="N'kai Mobile Companion Lifecycle Manager",
    )
    parser.add_argument(
        "--port",
        type=int,
        default=DEFAULT_PORT,
        help=f"Target server port (default: {DEFAULT_PORT})",
    )
    parser.add_argument(
        "--quiet",
        action="store_true",
        help="Suppress informational output on ensure",
    )

    subparsers = parser.add_subparsers(dest="command", help="Management command")
    subparsers.add_parser("status", help="Probe mobile companion status")
    subparsers.add_parser("start", help="Start background daemon")
    subparsers.add_parser("stop", help="Stop background daemon")
    subparsers.add_parser("restart", help="Restart background daemon")
    subparsers.add_parser("ensure", help="Idempotently ensure daemon is running")

    args = parser.parse_args()

    if not args.command or args.command == "status":
        sys.exit(cmd_status(args))
    elif args.command == "start":
        sys.exit(cmd_start(args))
    elif args.command == "stop":
        sys.exit(cmd_stop(args))
    elif args.command == "restart":
        sys.exit(cmd_restart(args))
    elif args.command == "ensure":
        sys.exit(cmd_ensure(args))


if __name__ == "__main__":
    main()
