#!/usr/bin/env python3
"""
tools/sp.py — Scratchpad Ingestion & Vault Archive CLI
=====================================================
Automates atomic pull, archive, wipe, and status checks for C:\\_\\Vault\\SCRATCHPAD.md.

Commands:
  pull (default) : Archives non-empty SCRATCHPAD.md to Archive/Scratchpads/YYYY-MM-DD_HHMM_scratchpad.md,
                   wipes SCRATCHPAD.md to blank (0 bytes), and outputs the archive path & content.
  peek           : Displays SCRATCHPAD.md contents without archiving or wiping.
  status         : Reports whether SCRATCHPAD.md has pending content (byte size & line count).
"""

import sys
import os
from pathlib import Path
from datetime import datetime

# Enforce UTF-8 on Windows stdout/stderr
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

VAULT_ROOT = Path(__file__).resolve().parent.parent
SCRATCHPAD_PATH = VAULT_ROOT / "SCRATCHPAD.md"
ARCHIVE_DIR = VAULT_ROOT / "Archive" / "Scratchpads"


def ensure_archive_dir() -> Path:
    ARCHIVE_DIR.mkdir(parents=True, exist_ok=True)
    return ARCHIVE_DIR


def get_archive_target() -> Path:
    ensure_archive_dir()
    now = datetime.now()
    base_name = now.strftime("%Y-%m-%d_%H%M")
    target = ARCHIVE_DIR / f"{base_name}_scratchpad.md"
    if target.exists():
        # Disambiguate with seconds if multiple pulls occur within the same minute
        sec_name = now.strftime("%Y-%m-%d_%H%M%S")
        target = ARCHIVE_DIR / f"{sec_name}_scratchpad.md"
        counter = 1
        while target.exists():
            target = ARCHIVE_DIR / f"{sec_name}_{counter}_scratchpad.md"
            counter += 1
    return target


def cmd_pull() -> int:
    if not SCRATCHPAD_PATH.exists():
        print("[EMPTY] SCRATCHPAD.md does not exist.")
        return 0

    content = SCRATCHPAD_PATH.read_text(encoding="utf-8")
    stripped = content.strip()

    if not stripped:
        print("[EMPTY] SCRATCHPAD.md is empty (0 bytes or whitespace only).")
        return 0

    target = get_archive_target()
    timestamp_iso = datetime.now().astimezone().isoformat()

    archive_content = f"""---
type: scratchpad_archive
archived_at: {timestamp_iso}
source: SCRATCHPAD.md
---

{stripped}
"""

    # 1. Write archive
    target.write_text(archive_content, encoding="utf-8")

    # 2. Wipe original
    SCRATCHPAD_PATH.write_text("", encoding="utf-8")

    rel_target = target.relative_to(VAULT_ROOT)
    uri_target = target.as_uri()
    print(f"📦 Archived to: [{target.name}]({uri_target}) ({rel_target.as_posix()})")
    print(f"🧹 SCRATCHPAD.md wiped clean (0 bytes).\n")
    print("--- CONTENT ---")
    print(stripped)
    print("---------------")
    return 0


def cmd_peek() -> int:
    if not SCRATCHPAD_PATH.exists():
        print("[EMPTY] SCRATCHPAD.md does not exist.")
        return 0

    content = SCRATCHPAD_PATH.read_text(encoding="utf-8")
    stripped = content.strip()

    if not stripped:
        print("[EMPTY] SCRATCHPAD.md is empty.")
        return 0

    print(f"📝 SCRATCHPAD.md ({len(content.encode('utf-8'))} bytes):\n")
    print(stripped)
    return 0


def cmd_status() -> int:
    if not SCRATCHPAD_PATH.exists():
        print("Status: Missing (0 bytes)")
        return 0

    content = SCRATCHPAD_PATH.read_text(encoding="utf-8")
    stripped = content.strip()

    if not stripped:
        print("Status: Clean / Empty (0 bytes)")
        return 0

    byte_count = len(content.encode("utf-8"))
    line_count = len(content.splitlines())
    print(f"Status: Pending Content ({byte_count} bytes, {line_count} lines)")
    return 0


def main():
    cmd = sys.argv[1].lower() if len(sys.argv) > 1 else "pull"

    if cmd in ("pull", "flush", "read", "pop", "archive", "ingest", "cut"):
        return cmd_pull()
    elif cmd in ("peek", "view", "show", "preview", "draft"):
        return cmd_peek()
    elif cmd in ("status", "check", "stat"):
        return cmd_status()
    else:
        print(f"Unknown command: {cmd}")
        print("Usage: python tools/sp.py [pull|peek|status]")
        return 1


if __name__ == "__main__":
    sys.exit(main())
