#!/usr/bin/env python3
"""
tools/plan_manager.py — 1-Command Phase Advancement & Telemetry for PLAN.md
==========================================================================
Enables 1-command phase checking, automatic receipt status updates, and
header progress synchronization in <5ms without LLM file clobbering.
Saves ~3,500 tokens per phase advancement.
Part of the Zero-Token Mechanical Automation Suite (Flight 3).
"""

import argparse
import json
import os
import re
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

# Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent


def get_default_plan_path() -> Path:
    return REPO_ROOT / "PLAN.md"


def get_communique_dir() -> Path:
    return REPO_ROOT / ".agents" / "pipeline" / "communique"


def parse_plan_sections(content: str) -> Tuple[str, List[Dict[str, Any]], str]:
    """Parses PLAN.md into header, structured phase/flight sections, and footer.
    Returns (header_text, sections, footer_text).
    """
    matches = list(re.finditer(r"^###\s+((?:Phase|Flight)\s+([0-9A-Za-z_.-]+)[^\n]*)", content, re.MULTILINE))
    if not matches:
        return content, [], ""

    header_text = content[:matches[0].start()]
    sections = []

    for i, m in enumerate(matches):
        full_title = m.group(1).strip()
        phase_id = m.group(2).strip()
        start_idx = m.start()
        end_idx = matches[i + 1].start() if i + 1 < len(matches) else len(content)

        # Separate section chunk
        section_chunk = content[start_idx:end_idx]

        # Extract checkboxes inside this section (ignoring fenced code blocks)
        checkboxes = []
        in_code_block = False
        for line in section_chunk.splitlines():
            if line.strip().startswith("```"):
                in_code_block = not in_code_block
                continue
            if not in_code_block:
                cb_match = re.match(r"^\s*-\s+\[([ xX])\]\s+(.*)", line)
                if cb_match:
                    checked = cb_match.group(1).lower() == "x"
                    desc = cb_match.group(2).strip()
                    checkboxes.append({"checked": checked, "desc": desc, "raw": line})

        sections.append({
            "full_title": full_title,
            "phase_id": phase_id,
            "start": start_idx,
            "end": end_idx,
            "chunk": section_chunk,
            "checkboxes": checkboxes,
            "total_items": len(checkboxes),
            "completed_items": sum(1 for c in checkboxes if c["checked"]),
            "is_complete": len(checkboxes) > 0 and all(c["checked"] for c in checkboxes),
        })

    footer_text = ""
    return header_text, sections, footer_text


def update_progress_header(content: str, completed_phases: int, total_phases: int) -> str:
    """Updates the '> **Progress:** N/M Flights/Phases Completed (X%)' line in the plan header."""
    if total_phases == 0:
        return content

    pct = int(round((completed_phases / total_phases) * 100))
    is_flights = "flight" in content.lower()
    unit = "Flights" if is_flights else "Phases"
    new_progress_line = f"> **Progress:** `{completed_phases}/{total_phases} {unit} Completed ({pct}%)`"

    # Replace existing progress line
    pattern = r"> \*\*Progress:\*\*\s*`?[^`\n]+`?"
    if re.search(pattern, content):
        content = re.sub(pattern, new_progress_line, content, count=1)

    # If 100% complete, update status to COMPLETED
    if completed_phases == total_phases and completed_phases > 0:
        status_pattern = r">\s*\*\*Status:\*\*\s*`?[A-Za-z_]+`?"
        if re.search(status_pattern, content) and "COMPLETED" not in content[:500]:
            content = re.sub(status_pattern, "> **Status:** `COMPLETED ✅`", content, count=1)

    return content


def get_receipt_paths(plan_content: str) -> List[Path]:
    """Determines the active receipt paths based on TARGET pipeline."""
    target_match = re.search(r"\*\*(?:TARGET|Target Pipeline):\*\*\s*(?:`?TARGET:\s*)?`?([A-Z_]+)", plan_content, re.IGNORECASE)
    target = target_match.group(1).strip().upper() if target_match else "FACTORY_FLOOR"

    cdir = get_communique_dir()
    if target == "TOOLING_HARNESS":
        return [cdir / "tooling_receipt.md", cdir / "subagent_receipt.md"]
    return [cdir / "factory_receipt.md", cdir / "build_to_plan.md"]


def update_receipt_status(receipt_paths: List[Path], new_status: str, active_note: str = "", dry_run: bool = False) -> None:
    """Updates the status banner in the relevant receipt file."""
    for p in receipt_paths:
        if not p.exists():
            continue
        try:
            raw = p.read_text(encoding="utf-8")
            # Replace ## Status: ...
            pattern = r"^##\s+Status:[^\n]+"
            replacement = f"## Status: {new_status}"
            if re.search(pattern, raw, re.MULTILINE):
                updated = re.sub(pattern, replacement, raw, count=1, flags=re.MULTILINE)
            else:
                updated = f"{replacement}\n\n{raw}"

            if active_note:
                # Update Active flight / phase note if present
                note_pattern = r"^Active\s+(?:flight|phase|step):[^\n]+"
                if re.search(note_pattern, updated, re.MULTILINE | re.IGNORECASE):
                    updated = re.sub(note_pattern, active_note, updated, count=1, flags=re.MULTILINE | re.IGNORECASE)

            if not dry_run:
                p.write_text(updated, encoding="utf-8")
        except Exception as e:
            print(f"⚠️ Failed to update receipt {p.name}: {e}", file=sys.stderr)


def cmd_status(args: argparse.Namespace) -> int:
    plan_path = Path(args.plan).resolve()
    if not plan_path.exists():
        print(f"❌ Error: PLAN.md not found at {plan_path}", file=sys.stderr)
        return 1

    content = plan_path.read_text(encoding="utf-8")
    header, sections, _ = parse_plan_sections(content)

    title_match = re.search(r"^#\s+(.+)$", content, re.MULTILINE)
    title = title_match.group(1).strip() if title_match else plan_path.stem

    target_match = re.search(r"\*\*(?:TARGET|Target Pipeline):\*\*\s*(?:`?TARGET:\s*)?`?([A-Z_]+)", content, re.IGNORECASE)
    target = target_match.group(1).strip().upper() if target_match else "FACTORY_FLOOR"

    status_match = re.search(r">\s*\*\*Status:\*\*\s*`?([A-Za-z_ ✅🔨]+)", content)
    status = status_match.group(1).strip() if status_match else "UNKNOWN"

    total_phases = len(sections)
    completed_phases = sum(1 for s in sections if s["is_complete"])
    active_phase = "All phases complete"
    for s in sections:
        if not s["is_complete"]:
            active_phase = s["full_title"]
            break

    if getattr(args, "json", False):
        payload = {
            "title": title,
            "target": target,
            "status": status,
            "completed_phases": completed_phases,
            "total_phases": total_phases,
            "active_phase": active_phase,
            "phases": [
                {
                    "title": s["full_title"],
                    "phase_id": s["phase_id"],
                    "completed": s["is_complete"],
                    "items": f"{s['completed_items']}/{s['total_items']}",
                }
                for s in sections
            ]
        }
        print(json.dumps(payload, indent=2, ensure_ascii=False))
        return 0

    print("=" * 70)
    print(f"📋 PLAN STATUS: {title}")
    print("=" * 70)
    print(f"🎯 Target Pipeline: {target} | Status: {status}")
    pct = int(round((completed_phases / total_phases) * 100)) if total_phases else 0
    print(f"📊 Progress:        {completed_phases}/{total_phases} ({pct}%)")
    print(f"⚡ Active Step:     {active_phase}")
    print("-" * 70)
    for s in sections:
        badge = "✅" if s["is_complete"] else "🟡"
        items_str = f"({s['completed_items']}/{s['total_items']})" if s["total_items"] > 0 else ""
        print(f"  {badge} {s['full_title']} {items_str}")
    print("=" * 70)
    return 0


def cmd_check(args: argparse.Namespace) -> int:
    plan_path = Path(args.plan).resolve()
    if not plan_path.exists():
        print(f"❌ Error: PLAN.md not found at {plan_path}", file=sys.stderr)
        return 1

    content = plan_path.read_text(encoding="utf-8")
    header, sections, _ = parse_plan_sections(content)

    target_id = str(args.phase).strip().lower()
    matched_section = None

    for s in sections:
        if s["phase_id"].lower() == target_id or target_id in s["full_title"].lower():
            matched_section = s
            break

    if not matched_section and target_id not in ("all", "*"):
        print(f"❌ Phase/Flight matching '{args.phase}' not found.", file=sys.stderr)
        return 1

    # Modify chunk: flip all '- [ ]' to '- [x]'
    new_content = content
    if target_id in ("all", "*"):
        # Flip all checkboxes across all sections
        for s in sections:
            old_chunk = s["chunk"]
            # Only flip outside code blocks
            lines = []
            in_code = False
            for line in old_chunk.splitlines(keepends=True):
                if line.strip().startswith("```"):
                    in_code = not in_code
                if not in_code:
                    line = re.sub(r"^(\s*-\s+)\[\s*\]", r"\1[x]", line)
                lines.append(line)
            new_chunk = "".join(lines)
            new_content = new_content.replace(old_chunk, new_chunk, 1)
        completed_phases = len(sections)
    else:
        old_chunk = matched_section["chunk"]
        lines = []
        in_code = False
        for line in old_chunk.splitlines(keepends=True):
            if line.strip().startswith("```"):
                in_code = not in_code
            if not in_code:
                line = re.sub(r"^(\s*-\s+)\[\s*\]", r"\1[x]", line)
            lines.append(line)
        new_chunk = "".join(lines)
        new_content = new_content.replace(old_chunk, new_chunk, 1)

        # Recalculate completed phases
        _, updated_sections, _ = parse_plan_sections(new_content)
        completed_phases = sum(1 for s in updated_sections if s["is_complete"])

    new_content = update_progress_header(new_content, completed_phases, len(sections))

    if args.dry_run:
        print(f"🔍 [DRY-RUN] Would check off: {matched_section['full_title'] if matched_section else 'All phases'}")
        print(f"   • Updated progress: {completed_phases}/{len(sections)}")
        return 0

    plan_path.write_text(new_content, encoding="utf-8")
    print(f"✅ Checked off: {matched_section['full_title'] if matched_section else 'All phases'}")
    print(f"   • Updated progress: {completed_phases}/{len(sections)}")
    return 0


def cmd_uncheck(args: argparse.Namespace) -> int:
    plan_path = Path(args.plan).resolve()
    if not plan_path.exists():
        print(f"❌ Error: PLAN.md not found at {plan_path}", file=sys.stderr)
        return 1

    content = plan_path.read_text(encoding="utf-8")
    header, sections, _ = parse_plan_sections(content)

    target_id = str(args.phase).strip().lower()
    matched_section = None

    for s in sections:
        if s["phase_id"].lower() == target_id or target_id in s["full_title"].lower():
            matched_section = s
            break

    if not matched_section:
        print(f"❌ Phase/Flight matching '{args.phase}' not found.", file=sys.stderr)
        return 1

    old_chunk = matched_section["chunk"]
    lines = []
    in_code = False
    for line in old_chunk.splitlines(keepends=True):
        if line.strip().startswith("```"):
            in_code = not in_code
        if not in_code:
            line = re.sub(r"^(\s*-\s+)\[[xX]\]", r"\1[ ]", line)
        lines.append(line)
    new_chunk = "".join(lines)
    new_content = content.replace(old_chunk, new_chunk, 1)

    # Recalculate completed phases
    _, updated_sections, _ = parse_plan_sections(new_content)
    completed_phases = sum(1 for s in updated_sections if s["is_complete"])
    new_content = update_progress_header(new_content, completed_phases, len(sections))

    if args.dry_run:
        print(f"🔍 [DRY-RUN] Would uncheck: {matched_section['full_title']}")
        return 0

    plan_path.write_text(new_content, encoding="utf-8")
    print(f"🔄 Unchecked: {matched_section['full_title']}")
    print(f"   • Updated progress: {completed_phases}/{len(sections)}")
    return 0


def cmd_advance(args: argparse.Namespace) -> int:
    plan_path = Path(args.plan).resolve()
    if not plan_path.exists():
        print(f"❌ Error: PLAN.md not found at {plan_path}", file=sys.stderr)
        return 1

    content = plan_path.read_text(encoding="utf-8")
    header, sections, _ = parse_plan_sections(content)

    if not sections:
        print("⚠️ No phases or flights found in PLAN.md.", file=sys.stderr)
        return 1

    # Find the current incomplete phase
    cur_idx = -1
    for i, s in enumerate(sections):
        if not s["is_complete"]:
            cur_idx = i
            break

    if cur_idx == -1:
        print("🎉 All phases are already completed in PLAN.md!")
        receipt_paths = get_receipt_paths(content)
        update_receipt_status(receipt_paths, "COMPLETED ✅", "All flights completed.", dry_run=args.dry_run)
        return 0

    current_section = sections[cur_idx]
    next_section = sections[cur_idx + 1] if cur_idx + 1 < len(sections) else None

    # Step 1: Check off current section
    old_chunk = current_section["chunk"]
    lines = []
    in_code = False
    for line in old_chunk.splitlines(keepends=True):
        if line.strip().startswith("```"):
            in_code = not in_code
        if not in_code:
            line = re.sub(r"^(\s*-\s+)\[\s*\]", r"\1[x]", line)
        lines.append(line)
    new_chunk = "".join(lines)
    new_content = content.replace(old_chunk, new_chunk, 1)

    # Step 2: Recalculate progress
    _, updated_sections, _ = parse_plan_sections(new_content)
    completed_phases = sum(1 for s in updated_sections if s["is_complete"])
    new_content = update_progress_header(new_content, completed_phases, len(sections))

    # Step 3: Update receipt status
    receipt_paths = get_receipt_paths(new_content)
    if next_section:
        new_receipt_status = f"BUILDING 🔨 ({next_section['full_title']})"
        active_note = f"Active step: {next_section['full_title']}."
    else:
        new_receipt_status = "COMPLETED ✅"
        active_note = "All flights completed."

    if args.dry_run:
        print(f"🔍 [DRY-RUN] Would check off: {current_section['full_title']}")
        if next_section:
            print(f"🔍 [DRY-RUN] Would advance to: {next_section['full_title']}")
            print(f"🔍 [DRY-RUN] Would set receipt status: {new_receipt_status}")
        else:
            print("🔍 [DRY-RUN] All phases complete! Would set receipt to COMPLETED ✅")
        return 0

    plan_path.write_text(new_content, encoding="utf-8")
    update_receipt_status(receipt_paths, new_receipt_status, active_note, dry_run=False)

    print(f"✅ Checked off: {current_section['full_title']}")
    if next_section:
        print(f"🚀 Advanced to: {next_section['full_title']}")
        print(f"   • Updated receipts to: {new_receipt_status}")
    else:
        print("🎉 Final phase completed! Plan is 100% complete.")
        print(f"   • Updated receipts to: COMPLETED ✅")

    return 0


def main() -> int:
    parser = argparse.ArgumentParser(
        description="📋 1-Command Phase Advancement & Telemetry for PLAN.md (Zero-Token Mechanical Clerk)"
    )
    parser.add_argument(
        "--plan",
        "-p",
        type=str,
        default=str(get_default_plan_path()),
        help="Path to PLAN.md (default: repo root PLAN.md)",
    )
    subparsers = parser.add_subparsers(dest="command", required=True)

    # Subcommand: status
    parser_status = subparsers.add_parser("status", help="Show plan status, active phase, and completion percentage")
    parser_status.add_argument("--json", action="store_true", help="Output status as JSON")

    # Subcommand: check <phase>
    parser_check = subparsers.add_parser("check", help="Check off a phase or flight (flips - [ ] to - [x])")
    parser_check.add_argument("phase", type=str, help="Phase or Flight number/ID (e.g. 1, 1B, 2, 'Flight 2', or 'all')")
    parser_check.add_argument("--dry-run", action="store_true", help="Show what would be checked off without writing")

    # Subcommand: uncheck <phase>
    parser_uncheck = subparsers.add_parser("uncheck", help="Uncheck a phase or flight (flips - [x] to - [ ])")
    parser_uncheck.add_argument("phase", type=str, help="Phase or Flight number/ID (e.g. 1, 1B, 2)")
    parser_uncheck.add_argument("--dry-run", action="store_true", help="Show what would be unchecked without writing")

    # Subcommand: advance
    parser_advance = subparsers.add_parser("advance", help="Check off active phase and advance to next phase")
    parser_advance.add_argument("--dry-run", action="store_true", help="Show what would be advanced without writing")

    args = parser.parse_args()

    if args.command == "status":
        return cmd_status(args)
    elif args.command == "check":
        return cmd_check(args)
    elif args.command == "uncheck":
        return cmd_uncheck(args)
    elif args.command == "advance":
        return cmd_advance(args)

    return 0


if __name__ == "__main__":
    sys.exit(main())
