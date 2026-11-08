#!/usr/bin/env python3
"""
tools/agenda_router.py — Instant Flight Deck & Agenda CLI Router
================================================================
Fast CLI and clipboard router to fling ideas, active flights, and parked debates
directly into C:\\_\\Vault\\AGENDA.md without manually opening Obsidian.

Usage:
  # 1. Add to Today's Live Runway (Active Flights)
  python tools/agenda_router.py runway "Flight 49: DSP Limiter" --priority P1 --track "🎛️ TKS" --action "Tune soft clipping curve"

  # 2. Add to Active Open Loops
  python tools/agenda_router.py loop "Evaluate Kokoro-82M TTS" --summary "Local voice model for The Forge" --dilemma "Local audio vs cloud API"

  # 3. Park on Tabled Talks with an Unlock Trigger
  python tools/agenda_router.py table "AUv3 iPad Port" --trigger "iPad Pro Acquisition" --why "Need physical hardware target"

  # 4. Route directly from Windows Clipboard
  python tools/agenda_router.py --clipboard --section loop
  python tools/agenda_router.py --clipboard --section runway --track "🎛️ TKS" --priority P1
"""

import argparse
import os
import subprocess
import sys
import time
from pathlib import Path
from typing import Optional

# Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

try:
    from tools.agenda_engine import route_card_to_agenda, parse_agenda, get_agenda_path
    from tools.sync_canon import sync_canon
except ImportError:
    # Direct import fallback
    from agenda_engine import route_card_to_agenda, parse_agenda, get_agenda_path
    try:
        from sync_canon import sync_canon
    except ImportError:
        sync_canon = None


def get_clipboard_text() -> str:
    """Reads text from Windows clipboard via PowerShell with zero pip dependencies."""
    try:
        res = subprocess.run(
            ["powershell", "-NoProfile", "-Command", "Get-Clipboard"],
            capture_output=True,
            text=True,
            encoding="utf-8",
            check=False,
        )
        return res.stdout.strip()
    except Exception as e:
        print(f"⚠️ Warning: Could not read clipboard: {e}", file=sys.stderr)
        return ""


def main():
    parser = argparse.ArgumentParser(description="The Cosmic Codex — Instant Agenda & Runway Router")
    subparsers = parser.add_subparsers(dest="command", help="Target section in AGENDA.md")

    # Command: runway
    p_runway = subparsers.add_parser("runway", help="Add flight to Today's Live Flight Runway")
    p_runway.add_argument("title", nargs="?", default="", help="Focus topic or Flight title")
    p_runway.add_argument("--priority", "-p", default="P1", choices=["P0", "P1", "P2", "P3"], help="Priority level")
    p_runway.add_argument("--track", "-t", default="🎛️ TKS", help="Pillar/Track (e.g. 🎛️ TKS, 🤖 AGY, 📱 Nkai)")
    p_runway.add_argument("--action", "-a", default="", help="Immediate next action")
    p_runway.add_argument("--clipboard", "-c", action="store_true", help="Read from Windows clipboard")

    # Command: loop
    p_loop = subparsers.add_parser("loop", help="Add topic to Active Open Loops")
    p_loop.add_argument("title", nargs="?", default="", help="Discussion or dilemma title")
    p_loop.add_argument("--summary", "-s", default="", help="Executive summary / context")
    p_loop.add_argument("--dilemma", "-d", default="", help="Core architectural dilemma")
    p_loop.add_argument("--clipboard", "-c", action="store_true", help="Read from Windows clipboard")

    # Command: table
    p_table = subparsers.add_parser("table", help="Park topic in Tabled Talks with an Unlock Trigger")
    p_table.add_argument("title", nargs="?", default="", help="Topic to table")
    p_table.add_argument("--trigger", "-g", required=False, default="Manual 30-Day Review Gate", help="Non-negotiable unlock condition")
    p_table.add_argument("--why", "-w", default="", help="Architectural rationale for parking")
    p_table.add_argument("--clipboard", "-c", action="store_true", help="Read from Windows clipboard")

    # Top-level global flags
    parser.add_argument("--clipboard", "-c", action="store_true", help="Quick route clipboard content")
    parser.add_argument("--section", choices=["runway", "loop", "table"], default="loop", help="Target section when using top-level --clipboard")
    parser.add_argument("--status", action="store_true", help="Show current Agenda Flight Deck stats")

    args = parser.parse_args()

    # Fast status check
    if args.status or (len(sys.argv) == 1):
        agenda = parse_agenda()
        meta = agenda.get("meta", {})
        print("=" * 64)
        print("🧭 THE COSMIC CODEX — LIVE FLIGHT DECK STATUS")
        print("=" * 64)
        print(f"📄 Path:         {meta.get('path', 'AGENDA.md')}")
        print(f"🛫 Runway Tasks: {meta.get('runway_count', 0)} flights active")
        print(f"🔄 Open Loops:   {meta.get('open_loops_count', 0)} active ({meta.get('resolved_loops_count', 0)} resolved)")
        print(f"🔒 Tabled Talks: {meta.get('tabled_count', 0)} parked concepts")
        print("-" * 64)
        print("Runway Flights:")
        for r in agenda.get("runway", []):
            print(f"  • [{r.get('priority')}] {r.get('track')} ➔ {r.get('topic')}")
            print(f"    Action: {r.get('next_action')}")
        print("=" * 64)
        print("💡 Usage:")
        print("   python tools/agenda_router.py runway \"Flight Title\" --action \"Next action\"")
        print("   python tools/agenda_router.py loop \"Open Loop Title\" --dilemma \"Core Dilemma\"")
        print("   python tools/agenda_router.py table \"Topic\" --trigger \"Unlock condition\" --why \"Why parked\"")
        print("   python tools/agenda_router.py --clipboard --section loop")
        print("=" * 64)
        return

    # Handle routing
    cmd = args.command
    use_clipboard = getattr(args, "clipboard", False)
    clip_text = get_clipboard_text() if use_clipboard else ""

    if not cmd and use_clipboard:
        cmd = args.section

    if cmd == "runway":
        title = args.title.strip()
        action = args.action.strip()
        if use_clipboard and clip_text:
            lines = [l.strip() for l in clip_text.splitlines() if l.strip()]
            if not title and lines:
                title = lines[0]
            if not action and len(lines) > 1:
                action = " ".join(lines[1:])
            elif not action:
                action = "Investigate and implement."

        if not title:
            print("❌ Error: Missing title for runway flight. Specify title or copy text to clipboard.", file=sys.stderr)
            sys.exit(1)
        if not action:
            action = "Immediate execution pending."

        res = route_card_to_agenda(
            title=title,
            summary=action,
            target_section="runway",
            priority=args.priority,
            track=args.track,
        )
        if res.get("success"):
            print(f"🛫 Successfully routed to Today's Runway: [{args.priority}] {args.track} ➔ {title}")
            print(f"   Next Action: {action}")
            if sync_canon:
                sync_canon()
        else:
            print(f"❌ Failed to route: {res.get('error')}", file=sys.stderr)

    elif cmd == "loop":
        title = args.title.strip()
        summary = getattr(args, "summary", "").strip()
        dilemma = getattr(args, "dilemma", "").strip()

        if use_clipboard and clip_text:
            lines = [l.strip() for l in clip_text.splitlines() if l.strip()]
            if not title and lines:
                title = lines[0]
            if not summary and len(lines) > 1:
                summary = " ".join(lines[1:])

        if not title:
            print("❌ Error: Missing title for open loop. Specify title or copy text to clipboard.", file=sys.stderr)
            sys.exit(1)

        body_parts = []
        if summary:
            body_parts.append(summary)
        if dilemma:
            body_parts.append(f"Dilemma: {dilemma}")
        full_summary = " ".join(body_parts) if body_parts else "Captured via Agenda CLI Router."

        res = route_card_to_agenda(
            title=title,
            summary=full_summary,
            target_section="open_loops",
        )
        if res.get("success"):
            print(f"🔄 Successfully routed to Active Open Loops: {title}")
            print(f"   Context: {full_summary}")
            if sync_canon:
                sync_canon()
        else:
            print(f"❌ Failed to route: {res.get('error')}", file=sys.stderr)

    elif cmd == "table":
        title = args.title.strip()
        trigger = args.trigger.strip()
        why = args.why.strip()

        if use_clipboard and clip_text:
            lines = [l.strip() for l in clip_text.splitlines() if l.strip()]
            if not title and lines:
                title = lines[0]
            if not why and len(lines) > 1:
                why = " ".join(lines[1:])

        if not title:
            print("❌ Error: Missing title for tabled talk. Specify title or copy text to clipboard.", file=sys.stderr)
            sys.exit(1)
        if not why:
            why = "Parked to protect active cognitive bandwidth."

        res = route_card_to_agenda(
            title=title,
            summary=why,
            target_section="tabled_talks",
            unlock_trigger=trigger,
        )
        if res.get("success"):
            print(f"🔒 Successfully parked in Tabled Talks: {title}")
            print(f"   Unlock Trigger: {trigger}")
            print(f"   Why Parked:     {why}")
            if sync_canon:
                sync_canon()
        else:
            print(f"❌ Failed to route: {res.get('error')}", file=sys.stderr)


if __name__ == "__main__":
    main()
