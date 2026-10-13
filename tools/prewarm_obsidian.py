#!/usr/bin/env python3
"""
tools/prewarm_obsidian.py — Obsidian Pre-Warm & Sync Automator
=============================================================
Checks if Obsidian desktop is running on Windows. If closed, launches
Obsidian via URI protocol ('obsidian://open?vault=<Vault>') so that
Obsidian Sync has time (e.g., 5-10 minutes) to reconcile remote mobile
notes before morning mail sweeps or automated inbox pipelines execute.
"""

import sys
import os
import subprocess
import time
from pathlib import Path

# Enforce Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")


def is_obsidian_running() -> bool:
    """Checks if Obsidian desktop app is currently running on Windows."""
    if sys.platform != "win32":
        return True
    try:
        res = subprocess.run(
            ["tasklist", "/FI", "IMAGENAME eq Obsidian.exe"],
            capture_output=True,
            text=True,
            check=False
        )
        return "Obsidian.exe" in res.stdout
    except Exception:
        return False


def prewarm_obsidian(vault_name: str = "Vault", wait_seconds: int = 300) -> bool:
    """Launches Obsidian if closed and optionally pauses to allow initial sync."""
    if is_obsidian_running():
        print(f"🟢 [PREWARM] Obsidian desktop is already running. Sync is active.")
        return True

    print(f"🔄 [PREWARM] Obsidian is closed. Launching obsidian://open?vault={vault_name} ...")
    try:
        os.startfile(f"obsidian://open?vault={vault_name}")
        print(f"✅ [PREWARM] Successfully launched Obsidian ({vault_name}).")
        if wait_seconds > 0:
            print(f"⏳ [PREWARM] Pausing {wait_seconds}s ({wait_seconds//60}m) for Obsidian Sync to pull remote notes & PDFs...")
            for remaining in range(wait_seconds, 0, -15):
                print(f"   Syncing... {remaining}s remaining")
                time.sleep(min(15, remaining))
            print("✅ [PREWARM] Sync pre-warm window complete.")
        return True
    except Exception as e:
        print(f"❌ [PREWARM] Failed to launch Obsidian: {e}", file=sys.stderr)
        return False


def main():
    vault = "Vault"
    wait_sec = 300

    if len(sys.argv) > 1:
        if sys.argv[1].isdigit():
            wait_sec = int(sys.argv[1])
        else:
            vault = sys.argv[1]
    if len(sys.argv) > 2 and sys.argv[2].isdigit():
        wait_sec = int(sys.argv[2])

    success = prewarm_obsidian(vault_name=vault, wait_seconds=wait_sec)
    return 0 if success else 1


if __name__ == "__main__":
    sys.exit(main())
