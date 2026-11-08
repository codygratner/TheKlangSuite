#!/usr/bin/env python3
"""
tools/launch_tray.py — Dedicated WinSta0\\Default Launcher for N'kai Tray Daemon
=============================================================================
Kills any existing tray supervisor processes and starts tools/nkai_tray.py
explicitly targeted at the interactive desktop (WinSta0\\Default) using Win32 API.
This prevents sandbox desktop inheritance and guarantees proper Shell/Browser IPC.
"""

import sys
import time
import subprocess
from pathlib import Path
import win32process
import win32con

REPO_ROOT = Path(__file__).resolve().parent.parent

# 1. Terminate existing tray processes
cmd_find = 'powershell -Command "(Get-CimInstance Win32_Process | Where-Object { $_.CommandLine -like \\"*nkai_tray*\\" }).ProcessId"'
try:
    res = subprocess.run(cmd_find, shell=True, capture_output=True, text=True)
    pids = [int(p.strip()) for p in res.stdout.strip().splitlines() if p.strip().isdigit()]
    for pid in pids:
        try:
            subprocess.run(["taskkill", "/F", "/PID", str(pid)], capture_output=True)
        except Exception:
            pass
    if pids:
        time.sleep(0.5)
except Exception as e:
    print(f"Cleanup warning: {e}", file=sys.stderr)

# 2. Locate pythonw
python_exe = sys.executable
pythonw = Path(python_exe).parent / "pythonw.exe"
if pythonw.exists():
    python_exe = str(pythonw)

tray_script = REPO_ROOT / "tools" / "nkai_tray.py"
cmd_line = f'"{python_exe}" "{tray_script}" --restart'

# 3. Create process targeted directly at WinSta0\\Default
si = win32process.STARTUPINFO()
si.lpDesktop = r"WinSta0\Default"

hp, ht, pid, tid = win32process.CreateProcess(
    None,
    cmd_line,
    None,
    None,
    False,
    win32con.NORMAL_PRIORITY_CLASS,
    None,
    str(REPO_ROOT),
    si
)

print(f"Spawned N'kai Tray on WinSta0\\Default (PID: {pid})")
