#!/usr/bin/env python3
"""
tools/flush_memory.py — Automated Pre-Compaction Memory Flush & Plan Closer
==========================================================================
Automates the 4-Step Pre-Compaction Memory Flush and plan archival at zero
token cost, eliminating manual markdown bookkeeping.
Part of the Automated Black Box System (Topic #7).
"""

import argparse
import datetime
import os
import re
import shutil
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional

# Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent

# Import state manager
try:
    from tools import state_manager
except ImportError:
    import state_manager


def append_dev_history(
    role: str = "Strategic Planning (New Klang City)",
    summary: Optional[str] = None,
    dry_run: bool = False,
) -> bool:
    """Scrapes git state and PLAN.md to append structured entry to DEV_HISTORY.md."""
    dev_history_path = REPO_ROOT / "docs" / "history" / "DEV_HISTORY.md"
    if not dev_history_path.exists():
        print(f"⚠️ DEV_HISTORY.md not found at: {dev_history_path}", file=sys.stderr)
        return False

    state = state_manager.compute_workspace_state()
    g = state["git"]
    p = state["plan"]

    now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M")
    conv_id = os.environ.get("CONVERSATION_ID", "manual_run")

    obj_text = summary or f"Executed and verified: {p['title']} ({p['progress']})"

    deliverables = []
    if g["recent_commits"]:
        deliverables.append(f"Recent Commits: {', '.join(g['recent_commits'][:2])}")
    deliverables.append(f"Branch: `{g['branch']}` ({g['commit']})")
    deliverables.append(f"Active Plan Target: `{p['target']}` · Status: `{p['status']}`")

    deliv_bullets = "\n".join(f"  - {d}" for d in deliverables)

    entry = f"""
### Session: {now} ({conv_id})
- **Chat Role:** {role}
- **Primary Objectives:** {obj_text}
- **Key Decisions & Deliverables:**
{deliv_bullets}
"""

    if dry_run:
        print("🔍 [DRY RUN] Would append to DEV_HISTORY.md:")
        print(entry.strip())
        return True

    current_txt = dev_history_path.read_text(encoding="utf-8", errors="replace")
    dev_history_path.write_text(current_txt.rstrip() + "\n" + entry, encoding="utf-8")
    print(f"📝 Appended session summary to: {dev_history_path.relative_to(REPO_ROOT)}")
    return True


def check_staged_souls() -> List[Path]:
    """Scans .agents/pipeline/staging/souls/ for pending soul captures."""
    souls_dir = REPO_ROOT / ".agents" / "pipeline" / "staging" / "souls"
    if not souls_dir.exists():
        return []
    
    files = [f for f in souls_dir.glob("*.md") if f.name.lower() != "readme.md"]
    return files


def archive_completed_plan(slug: str, force: bool = False, dry_run: bool = False) -> Optional[Path]:
    """Archives PLAN.md to completed/ and resets root PLAN.md to idle."""
    plan_path = REPO_ROOT / "PLAN.md"
    if not plan_path.exists():
        print("⚠️ No PLAN.md found at root to archive.", file=sys.stderr)
        return None

    state = state_manager.parse_plan_state()
    if not force and "COMPLETED" not in state["status"].upper() and not state["is_idle"]:
        print(f"⚠️ PLAN.md is not marked COMPLETED (current status: {state['status']}). Use --force to override.", file=sys.stderr)
        return None

    today = datetime.datetime.now().strftime("%Y-%m-%d")
    sanitized_slug = re.sub(r"[^a-zA-Z0-9_\-]+", "_", slug.strip().lower()).strip("_")
    archive_dir = REPO_ROOT / ".agents" / "pipeline" / "plans" / "completed"
    archive_dir.mkdir(parents=True, exist_ok=True)
    dest_path = archive_dir / f"{today}_{sanitized_slug}.md"

    if dry_run:
        print(f"🔍 [DRY RUN] Would archive PLAN.md to: {dest_path.relative_to(REPO_ROOT)}")
        print("🔍 [DRY RUN] Would reset root PLAN.md to idle.")
        return dest_path

    # Copy to completed
    shutil.copyfile(plan_path, dest_path)
    print(f"📦 Archived plan to: {dest_path.relative_to(REPO_ROOT)}")

    # Reset root PLAN.md
    idle_content = """# Implementation Plan

*No active plan. Ready for next assignment from New Klang City.*
"""
    plan_path.write_text(idle_content, encoding="utf-8")
    print(f"🔄 Reset PLAN.md at root to idle.")
    return dest_path


def run_flush(
    role: str = "Strategic Planning (New Klang City)",
    summary: Optional[str] = None,
    archive_slug: Optional[str] = None,
    force: bool = False,
    dry_run: bool = False,
) -> int:
    """Executes the full automated 4-step memory flush sequence."""
    print("=" * 70)
    print("🚀 AUTOMATED BLACK BOX MEMORY FLUSH (Topic #7)")
    print("=" * 70)

    # Step 1: Dev History Ingestion
    print("\n[Step 1/4] Ingesting Dev History...")
    append_dev_history(role=role, summary=summary, dry_run=dry_run)

    # Step 2: Soul Harvest Staging Check
    print("\n[Step 2/4] Checking Staged Souls...")
    staged = check_staged_souls()
    if staged:
        print(f"🌾 Found {len(staged)} staged soul harvest(s):")
        for s in staged:
            print(f"   • {s.name}")
    else:
        print("🌾 Staging inbox clean (0 pending souls).")

    # Optional: Plan Archival
    if archive_slug:
        print(f"\n[Optional] Archiving Completed Plan ('{archive_slug}')...")
        archive_completed_plan(slug=archive_slug, force=force, dry_run=dry_run)

    # Step 3: Synchronize Workspace Context State
    print("\n[Step 3/4] Updating Workspace Context State (tools/state_manager.py)...")
    if not dry_run:
        state = state_manager.compute_workspace_state()
        state_manager.cmd_sync(argparse.Namespace(quiet=False))
    else:
        print("🔍 [DRY RUN] Would generate context_state.json and ACTIVE_CONTEXT.md")

    # Step 4: Verification Check
    print("\n[Step 4/4] Verifying Workspace State...")
    state_manager.cmd_verify(argparse.Namespace())

    print("\n" + "=" * 70)
    print("✅ MEMORY FLUSH COMPLETE: Workspace state hardened and ready for /clear.")
    print("=" * 70)
    return 0


def main():
    parser = argparse.ArgumentParser(description="Automated Pre-Compaction Memory Flush")
    parser.add_argument("--role", default="Strategic Planning (New Klang City)", help="Chat role")
    parser.add_argument("--summary", default=None, help="Summary of achievements for DEV_HISTORY.md")
    parser.add_argument("--archive-plan", default=None, metavar="SLUG", help="Archive completed PLAN.md with slug")
    parser.add_argument("--force", action="store_true", help="Force archive even if status != COMPLETED")
    parser.add_argument("--dry-run", action="store_true", help="Preview actions without modifying files")

    args = parser.parse_args()
    return run_flush(
        role=args.role,
        summary=args.summary,
        archive_slug=args.archive_plan,
        force=args.force,
        dry_run=args.dry_run,
    )


if __name__ == "__main__":
    sys.exit(main())
