#!/usr/bin/env python3
"""
tools/check_lore.py
Lore Dispatch Queue Inspector for The Klang Suite and The Cosmic Codex.

Checks for incoming transmissions between the Stellar Zenith (The Cosmic Codex / Vault)
and the Subterranean Foundry (Mayor Toad / Klang Industries).
"""

import sys
import os
from pathlib import Path

# Ensure UTF-8 console output for Windows terminal safety
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent
VAULT_DISPATCH = Path(r"C:\_\Vault\New Klang City\Lore\Dispatch")
LOCAL_DISPATCH = REPO_ROOT / "docs" / "lore" / "dispatch"


def inspect_dispatch():
    print("=" * 60)
    print("📜 LORE DISPATCH CONDUIT: STELLAR ZENITH <===> FOUNDRY")
    print("=" * 60)

    dispatch_dirs = [VAULT_DISPATCH, LOCAL_DISPATCH]
    found_transmissions = []

    seen = set()
    for d in dispatch_dirs:
        if d.exists():
            for f in d.glob("*.md"):
                if f.name.lower() == "readme.md":
                    continue
                if f.name not in seen:
                    seen.add(f.name)
                    found_transmissions.append(f)

    if not found_transmissions:
        print("🌌 All channels clear. No pending transmissions in the Lore Queue.")
        print(f"   [Zenith Drop]:  {VAULT_DISPATCH}")
        print(f"   [Foundry Drop]: {LOCAL_DISPATCH}")
        print("=" * 60)
        return 0

    print(f"⚡ INCOMING TRANSMISSIONS DETECTED ({len(found_transmissions)} pending):")
    for f in found_transmissions:
        size = f.stat().st_size
        print(f"\n📄 {f.name} ({size} bytes) -> {f}")
        try:
            content = f.read_text(encoding="utf-8", errors="replace")
            lines = content.strip().splitlines()
            preview = lines[0] if lines else "(empty)"
            for line in lines:
                sline = line.strip()
                if sline.startswith("# ") or sline.startswith("title:"):
                    preview = sline
                    break
            print(f"   Transmission: {preview[:80]}")
        except Exception as e:
            print(f"   Error reading preview: {e}")

    print("=" * 60)
    return len(found_transmissions)


if __name__ == "__main__":
    count = inspect_dispatch()
    sys.exit(0)
