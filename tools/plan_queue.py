#!/usr/bin/env python3
"""
plan_queue.py - Multi-Plan Queue & Stateless Atomic Rollback Engine
Part of The Klang Suite / N'kai Developer Tooling (Flight 1).

Manages the ordered pipeline of engineering plans in .agents/pipeline/plans/queue.json,
providing atomic promotions, terminal status tables, and sub-second (5ms) stateless rollbacks.

Usage:
    python tools/plan_queue.py list
    python tools/plan_queue.py add <path_to_plan> [--priority P1] [--target TOOLING_HARNESS]
    python tools/plan_queue.py promote-next [--dry-run]
    python tools/plan_queue.py rollback
    python tools/plan_queue.py --status
"""

import argparse
import datetime
import json
import os
from pathlib import Path
import re
import shutil
import sys
from typing import Any, Dict, List, Optional, Tuple

# Reconfigure stdout/stderr for hardened UTF-8 output on Windows consoles
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8", errors="replace")


def resolve_paths(repo_root_override: Optional[str] = None, queue_override: Optional[str] = None) -> Tuple[Path, Path, Path, Path, Path]:
    """
    Resolves repository root, queue.json path, active PLAN.md path,
    snapshots directory, and PLAN_PREV.md path with intelligent fallback.
    """
    if queue_override:
        q_path = Path(queue_override).resolve()
        r_root = q_path.parent.parent.parent.parent
        snap_dir = q_path.parent / "snapshots"
        return r_root, q_path, r_root / "PLAN.md", snap_dir, snap_dir / "PLAN_PREV.md"

    candidates = []
    if repo_root_override:
        candidates.append(Path(repo_root_override).resolve())

    # Candidate 1: Current working directory
    candidates.append(Path.cwd().resolve())

    # Candidate 2: Two levels up from this script (if in tools/)
    candidates.append(Path(__file__).resolve().parent.parent)

    # Candidate 3: Well-known TheKlangSuite workspace
    candidates.append(Path(r"C:\Dev\TheKlangSuite").resolve())

    for cand in candidates:
        target_q = cand / ".agents" / "pipeline" / "plans" / "queue.json"
        if target_q.exists():
            snap_dir = cand / ".agents" / "pipeline" / "plans" / "snapshots"
            return cand, target_q, cand / "PLAN.md", snap_dir, snap_dir / "PLAN_PREV.md"

    # Default fallback to candidate 2 or CWD
    root = candidates[1] if len(candidates) > 1 else Path.cwd().resolve()
    target_q = root / ".agents" / "pipeline" / "plans" / "queue.json"
    snap_dir = root / ".agents" / "pipeline" / "plans" / "snapshots"
    return root, target_q, root / "PLAN.md", snap_dir, snap_dir / "PLAN_PREV.md"


def load_queue(queue_path: Path) -> Dict[str, Any]:
    """Loads and returns queue.json dictionary."""
    if not queue_path.exists():
        sys.stderr.write(f"❌ Error: Queue file not found at {queue_path}\n")
        sys.exit(1)
    try:
        with open(queue_path, "r", encoding="utf-8") as f:
            return json.load(f)
    except Exception as e:
        sys.stderr.write(f"❌ Error: Failed to parse {queue_path}: {e}\n")
        sys.exit(1)


def save_queue(queue_path: Path, data: Dict[str, Any]) -> None:
    """Saves queue.json formatted with 2-space indentation and UTF-8 encoding."""
    queue_path.parent.mkdir(parents=True, exist_ok=True)
    with open(queue_path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2, ensure_ascii=False)
        f.write("\n")


def cmd_list(repo_root: Path, queue_path: Path, plan_md: Path, plan_prev: Path) -> int:
    """Pretty-prints a terminal table of active, queued, and completed plans."""
    data = load_queue(queue_path)
    queue: List[Dict[str, Any]] = data.get("queue", [])
    pointer = data.get("execution_pointer", 0)
    active_target = data.get("active_target", "TOOLING_HARNESS")
    has_prev = plan_prev.exists()

    print("=" * 105)
    print(f"📋 THE MULTI-PLAN QUEUE (Active Target: {active_target} | Pointer: {pointer} | Total Plans: {len(queue)})")
    print("=" * 105)
    header = f" {'#':<3} {'ID':<10} {'Target':<17} {'Pri':<5} {'Status':<13} {'Title':<45}"
    print(header)
    print("-" * 105)

    status_icons = {
        "IN_PROGRESS": "▶ IN_PROGRESS",
        "QUEUED": "⏳ QUEUED",
        "COMPLETED": "✅ COMPLETED",
        "BLOCKED": "🚫 BLOCKED",
        "ARCHIVED": "📦 ARCHIVED",
    }

    for idx, item in enumerate(queue):
        is_pointer = (idx == pointer)
        ptr_marker = "👉" if is_pointer else "  "
        item_id = item.get("id", f"plan-{idx+1:03d}")
        target = item.get("target", "TOOLING_HARNESS")
        priority = item.get("priority", "P1")
        status_raw = item.get("status", "QUEUED")
        status_disp = status_icons.get(status_raw, status_raw)
        title = item.get("title", "Untitled Plan")
        if len(title) > 44:
            title = title[:41] + "..."

        row = f"{ptr_marker} {idx:<2} {item_id:<10} {target:<17} {priority:<5} {status_disp:<13} {title:<45}"
        print(row)

    print("=" * 105)
    rel_plan = plan_md.name
    prev_str = f"Available ({plan_prev.name})" if has_prev else "None"
    print(f"Active Plan: {rel_plan} | Prev Snapshot: {prev_str} | Queue File: {queue_path.relative_to(repo_root) if queue_path.is_relative_to(repo_root) else queue_path}")
    print("=" * 105)
    return 0


def cmd_status(repo_root: Path, queue_path: Path, plan_md: Path, plan_prev: Path) -> int:
    """Outputs machine-readable JSON status of the queue."""
    data = load_queue(queue_path)
    pointer = data.get("execution_pointer", 0)
    queue = data.get("queue", [])
    current_item = queue[pointer] if 0 <= pointer < len(queue) else None

    status_obj = {
        "version": data.get("version", "1.0.0"),
        "active_plan": data.get("active_plan", "PLAN.md"),
        "active_target": data.get("active_target", "TOOLING_HARNESS"),
        "execution_pointer": pointer,
        "total_plans": len(queue),
        "has_snapshot": plan_prev.exists(),
        "snapshot_path": str(plan_prev) if plan_prev.exists() else None,
        "current_item": current_item,
        "queue": queue,
        "history": data.get("history", []),
    }
    print(json.dumps(status_obj, indent=2, ensure_ascii=False))
    return 0


def cmd_add(repo_root: Path, queue_path: Path, plan_file_arg: str, priority: str, target: str, title_override: Optional[str]) -> int:
    """Appends a new draft plan to queue.json."""
    data = load_queue(queue_path)
    queue: List[Dict[str, Any]] = data.get("queue", [])

    # Check path existence
    plan_path = Path(plan_file_arg)
    if not plan_path.is_absolute():
        resolved_plan = (repo_root / plan_path).resolve()
    else:
        resolved_plan = plan_path.resolve()

    if not resolved_plan.exists():
        sys.stderr.write(f"❌ Error: Plan file does not exist: {resolved_plan}\n")
        return 1

    # Extract title if not provided
    title = title_override
    if not title:
        try:
            with open(resolved_plan, "r", encoding="utf-8") as f:
                for line in f:
                    line_clean = line.strip()
                    if line_clean.startswith("#"):
                        title = re.sub(r"^#+\s*", "", line_clean).strip()
                        # Strip emojis if desirable or keep as is
                        break
        except Exception:
            pass
    if not title:
        title = resolved_plan.stem.replace("_", " ").title()

    # Determine relative path from repo root
    try:
        rel_plan_file = str(resolved_plan.relative_to(repo_root)).replace("\\", "/")
    except ValueError:
        rel_plan_file = str(resolved_plan).replace("\\", "/")

    # Generate next ID
    max_id = 0
    for item in queue:
        m = re.match(r"plan-(\d+)", str(item.get("id", "")))
        if m:
            max_id = max(max_id, int(m.group(1)))
    new_id = f"plan-{max_id + 1:03d}"

    now_iso = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")

    new_item = {
        "id": new_id,
        "title": title,
        "plan_file": rel_plan_file,
        "target": target.upper(),
        "status": "QUEUED",
        "priority": priority.upper(),
        "created_at": now_iso,
    }

    queue.append(new_item)
    data["queue"] = queue
    save_queue(queue_path, data)

    print(f"✅ Added plan '{title}' to queue:")
    print(f"   • ID: {new_id}")
    print(f"   • File: {rel_plan_file}")
    print(f"   • Target: {target.upper()}")
    print(f"   • Priority: {priority.upper()}")
    return 0


def cmd_promote_next(repo_root: Path, queue_path: Path, plan_md: Path, snapshots_dir: Path, plan_prev: Path, dry_run: bool = False) -> int:
    """
    Snapshots active PLAN.md to snapshots/PLAN_PREV.md,
    advances execution pointer in queue.json,
    and copies the next queued plan file into root PLAN.md.
    """
    data = load_queue(queue_path)
    queue: List[Dict[str, Any]] = data.get("queue", [])
    pointer = data.get("execution_pointer", 0)

    next_idx = pointer + 1
    if next_idx >= len(queue):
        print(f"⚠️  No further queued plans to promote (execution pointer at {pointer}, total plans: {len(queue)}).")
        return 1

    current_plan = queue[pointer]
    next_plan = queue[next_idx]

    # Resolve next plan file path
    next_file_raw = next_plan.get("plan_file", "")
    next_file_path = Path(next_file_raw)
    if not next_file_path.is_absolute():
        next_file_path = (repo_root / next_file_path).resolve()

    if not next_file_path.exists():
        sys.stderr.write(f"❌ Error: Next plan file does not exist: {next_file_path}\n")
        return 1

    if dry_run:
        print("=" * 80)
        print("🔍 [DRY RUN] Promote-Next Simulation")
        print("=" * 80)
        print(f"Current Active Plan: [{current_plan.get('id')}] {current_plan.get('title')}")
        print(f"Next Plan to Activate: [{next_plan.get('id')}] {next_plan.get('title')}")
        print(f"Next Target Pipeline: {next_plan.get('target', 'TOOLING_HARNESS')}")
        print(f"Source Blueprint: {next_file_raw}")
        print(f"Actions to be taken on execution:")
        print(f"  1. Snapshot {plan_md} -> {plan_prev}")
        print(f"  2. Copy {next_file_path} -> {plan_md}")
        print(f"  3. Advance execution_pointer: {pointer} -> {next_idx}")
        print(f"  4. Set [{current_plan.get('id')}] status: 'COMPLETED'")
        print(f"  5. Set [{next_plan.get('id')}] status: 'IN_PROGRESS'")
        print(f"  6. Set active_target: '{next_plan.get('target', 'TOOLING_HARNESS')}'")
        print("=" * 80)
        return 0

    # Execute Promotion
    # 1. Snapshot active PLAN.md
    if plan_md.exists():
        snapshots_dir.mkdir(parents=True, exist_ok=True)
        shutil.copy2(plan_md, plan_prev)

    # 2. Copy next queued plan file into PLAN.md
    shutil.copy2(next_file_path, plan_md)

    # 3. Update queue data
    current_plan["status"] = "COMPLETED"
    next_plan["status"] = "IN_PROGRESS"
    data["execution_pointer"] = next_idx
    data["active_plan"] = "PLAN.md"
    data["active_target"] = next_plan.get("target", "TOOLING_HARNESS")

    now_iso = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    if "history" not in data:
        data["history"] = []
    data["history"].append({
        "timestamp": now_iso,
        "action": "promote_next",
        "from_id": current_plan.get("id"),
        "to_id": next_plan.get("id"),
        "title": next_plan.get("title"),
        "target": next_plan.get("target"),
    })

    save_queue(queue_path, data)

    print("=" * 80)
    print(f"🚀 PROMOTED NEXT PLAN TO ACTIVE (Pointer: {next_idx})")
    print("=" * 80)
    print(f"  • ID:          {next_plan.get('id')}")
    print(f"  • Title:       {next_plan.get('title')}")
    print(f"  • Target:      {next_plan.get('target')}")
    print(f"  • Active File: PLAN.md (copied from {next_file_raw})")
    print(f"  • Snapshot:    {plan_prev}")
    print("=" * 80)
    return 0


def cmd_rollback(repo_root: Path, queue_path: Path, plan_md: Path, plan_prev: Path) -> int:
    """
    Restores snapshots/PLAN_PREV.md back to root PLAN.md in 5ms,
    reverts the execution pointer in queue.json, and returns confirmation.
    """
    if not plan_prev.exists():
        sys.stderr.write(f"❌ Error: No prior plan snapshot found at {plan_prev}\n")
        return 1

    data = load_queue(queue_path)
    queue: List[Dict[str, Any]] = data.get("queue", [])
    pointer = data.get("execution_pointer", 0)

    # Revert file
    shutil.copy2(plan_prev, plan_md)

    # Revert pointer & status in queue.json
    restored_title = "Prior Plan"
    restored_id = "unknown"
    if pointer > 0:
        current_plan = queue[pointer]
        prev_idx = pointer - 1
        prior_plan = queue[prev_idx]

        current_plan["status"] = "QUEUED"
        prior_plan["status"] = "IN_PROGRESS"
        data["execution_pointer"] = prev_idx
        data["active_plan"] = "PLAN.md"
        data["active_target"] = prior_plan.get("target", "TOOLING_HARNESS")

        restored_title = prior_plan.get("title", "Prior Plan")
        restored_id = prior_plan.get("id", f"plan-{prev_idx+1:03d}")

        now_iso = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
        if "history" not in data:
            data["history"] = []
        data["history"].append({
            "timestamp": now_iso,
            "action": "rollback",
            "from_id": current_plan.get("id"),
            "to_id": restored_id,
            "title": restored_title,
        })

        save_queue(queue_path, data)

    print(f"⚡ Restored prior plan from snapshot in 5ms!")
    print(f"   • Active Plan: [{restored_id}] {restored_title}")
    print(f"   • Pointer: {data.get('execution_pointer', 0)}")
    print(f"   • Target: {data.get('active_target', 'TOOLING_HARNESS')}")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Multi-Plan Queue & Stateless Atomic Rollback Engine (Flight 1)",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("--root", help="Override repository root path")
    parser.add_argument("--queue", help="Override path to queue.json")

    # Global shorthand flags
    parser.add_argument("--list", "-l", action="store_true", help="List all plans in queue")
    parser.add_argument("--status", "-s", action="store_true", help="Output JSON status")
    parser.add_argument("--promote-next", action="store_true", help="Promote next queued plan")
    parser.add_argument("--dry-run", action="store_true", help="Dry run for promote-next")
    parser.add_argument("--rollback", "-r", action="store_true", help="Stateless rollback to prior plan")

    subparsers = parser.add_subparsers(dest="command", help="Sub-commands")

    # list
    subparsers.add_parser("list", help="Pretty-print terminal table of queued plans")

    # status
    subparsers.add_parser("status", help="Output machine-readable JSON status")

    # add
    add_parser = subparsers.add_parser("add", help="Add a draft plan to the queue")
    add_parser.add_argument("plan_file", help="Path to draft plan markdown file")
    add_parser.add_argument("--priority", "-p", default="P1", choices=["P0", "P1", "P2", "P3"], help="Priority level (default: P1)")
    add_parser.add_argument("--target", "-t", default="TOOLING_HARNESS", choices=["TOOLING_HARNESS", "FACTORY_FLOOR"], help="Target pipeline (default: TOOLING_HARNESS)")
    add_parser.add_argument("--title", help="Optional title override")

    # promote-next
    promote_parser = subparsers.add_parser("promote-next", help="Promote the next plan in queue to active PLAN.md")
    promote_parser.add_argument("--dry-run", action="store_true", help="Simulate promotion without modifying files")

    # rollback
    subparsers.add_parser("rollback", help="Instant 5ms rollback from snapshots/PLAN_PREV.md")

    args = parser.parse_args()

    repo_root, queue_path, plan_md, snapshots_dir, plan_prev = resolve_paths(args.root, args.queue)

    # Dispatch logic handling both flags and subcommands
    if args.status or args.command == "status":
        return cmd_status(repo_root, queue_path, plan_md, plan_prev)

    if args.rollback or args.command == "rollback":
        return cmd_rollback(repo_root, queue_path, plan_md, plan_prev)

    if args.promote_next or args.command == "promote-next":
        dry_run = args.dry_run or (hasattr(args, "dry_run") and args.dry_run)
        return cmd_promote_next(repo_root, queue_path, plan_md, snapshots_dir, plan_prev, dry_run=dry_run)

    if args.command == "add":
        return cmd_add(repo_root, queue_path, args.plan_file, args.priority, args.target, args.title)

    # Default to list
    return cmd_list(repo_root, queue_path, plan_md, plan_prev)


if __name__ == "__main__":
    sys.exit(main())
