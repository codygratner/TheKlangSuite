#!/usr/bin/env python3
"""
tools/state_manager.py — Deterministic Zero-Token Workspace State Sniffer
========================================================================
Extracts and renders active git state, plan progress, communique locks,
and immediate next actions in <5ms without LLM overhead or prose bloat.
Part of the Automated Black Box System (Topic #7).
"""

import argparse
import json
import os
import re
import subprocess
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional

# Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))


def get_peer_locks() -> List[Dict[str, Any]]:
    """Sniffs active peer intent locks across the data bus."""
    try:
        from tools.bus import resolve_bus_dir, load_active_locks
        bus_dir = resolve_bus_dir()
        locks = load_active_locks(bus_dir)
        return [l for l in locks if l.get("pillar", "").upper() != "TKS"]
    except Exception:
        return []


def run_git(args: List[str]) -> str:
    """Runs a git command safely and returns stripped stdout."""
    try:
        res = subprocess.run(
            ["git"] + args,
            cwd=REPO_ROOT,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            check=False,
        )
        return res.stdout.strip()
    except Exception as e:
        return f"git error: {e}"


def get_git_state() -> Dict[str, Any]:
    """Sniffs branch, commit, and dirty status."""
    branch = run_git(["rev-parse", "--abbrev-ref", "HEAD"]) or "unknown"
    commit = run_git(["rev-parse", "--short", "HEAD"]) or "unknown"
    porcelain = run_git(["status", "--porcelain"])
    lines = [line for line in porcelain.splitlines() if line.strip()]
    
    modified = sum(1 for line in lines if line.startswith(" M") or line.startswith("M "))
    untracked = sum(1 for line in lines if line.startswith("??"))
    staged = sum(1 for line in lines if line[0] in "MADRC" and line[0] != " ")

    recent_commits = []
    log_raw = run_git(["log", "-n", "3", "--oneline"])
    if log_raw:
        recent_commits = log_raw.splitlines()

    return {
        "branch": branch,
        "commit": commit,
        "is_clean": len(lines) == 0,
        "dirty_count": len(lines),
        "staged": staged,
        "modified": modified,
        "untracked": untracked,
        "recent_commits": recent_commits,
    }


def parse_plan_state() -> Dict[str, Any]:
    """Inspects root PLAN.md for active goal, target, status, and phases."""
    plan_path = REPO_ROOT / "PLAN.md"
    if not plan_path.exists():
        return {"has_plan": False, "status": "NO_PLAN", "title": "No PLAN.md found"}

    content = plan_path.read_text(encoding="utf-8", errors="replace")
    
    # Extract Title (# ...)
    title_match = re.search(r"^#\s+(.+)$", content, re.MULTILINE)
    title = title_match.group(1).strip() if title_match else "Untitled Plan"

    # Extract Target
    target_match = re.search(r"\*\*(?:TARGET|Target Pipeline):\*\*\s*(?:`?TARGET:\s*)?`?([A-Z_]+)", content, re.IGNORECASE)
    target = target_match.group(1).strip().upper() if target_match else "FACTORY_FLOOR"

    # Extract Status
    status_match = re.search(r">\s*\*\*Status:\*\*\s*`?([A-Za-z_]+)", content)
    status = status_match.group(1).strip() if status_match else "UNKNOWN"
    if "COMPLETED" in status.upper():
        status = "COMPLETED"
    elif "IN_PROGRESS" in status.upper():
        status = "IN_PROGRESS"
    elif "READY" in status.upper():
        status = "READY_FOR_EXECUTION"

    # Phase Checklists
    total_items = len(re.findall(r"-\s+\[[ xX]\]", content))
    completed_items = len(re.findall(r"-\s+\[[xX]\]", content))

    # Active Phase / Flight Detection
    matches = list(re.finditer(r"^###\s+((?:Phase|Flight)\s+\d+[^:\n]*):?([^\n]*)", content, re.MULTILINE))
    active_phase = "All phases complete"
    for i, m in enumerate(matches):
        p_name = f"{m.group(1).strip()} {m.group(2).strip()}".strip()
        start_idx = m.start()
        end_idx = matches[i + 1].start() if i + 1 < len(matches) else len(content)
        phase_chunk = content[start_idx:end_idx]
        if "- [ ]" in phase_chunk:
            active_phase = p_name
            break

    is_idle = "No active plan" in content or status == "COMPLETED" or total_items == 0

    return {
        "has_plan": True,
        "is_idle": is_idle,
        "title": title,
        "target": target,
        "status": status,
        "progress": f"{completed_items}/{total_items}" if total_items > 0 else "0/0",
        "active_phase": active_phase if not is_idle else "Idle / Standby",
    }


def parse_communique_state() -> Dict[str, Any]:
    """Inspects .agents/pipeline/communique/ for factory and tooling latches."""
    comm_dir = REPO_ROOT / ".agents" / "pipeline" / "communique"
    
    # Factory Dispatch (supports factory_dispatch.md with legacy fallback)
    f_disp = comm_dir / "factory_dispatch.md"
    if not f_disp.exists():
        f_disp = comm_dir / "plan_to_build.md"
    
    # Factory Receipt
    f_rcpt = comm_dir / "factory_receipt.md"
    if not f_rcpt.exists():
        f_rcpt = comm_dir / "build_to_plan.md"

    # Tooling Dispatch
    t_disp = comm_dir / "tooling_dispatch.md"
    if not t_disp.exists():
        t_disp = comm_dir / "subagent_dispatch.md"

    # Tooling Receipt
    t_rcpt = comm_dir / "tooling_receipt.md"
    if not t_rcpt.exists():
        t_rcpt = comm_dir / "subagent_receipt.md"

    def get_file_status(path: Path) -> str:
        if not path.exists():
            return "MISSING"
        txt = path.read_text(encoding="utf-8", errors="replace")
        m = re.search(r"STATUS:\s*([A-Za-z_]+)", txt, re.IGNORECASE)
        if m:
            return m.group(1).upper()
        m2 = re.search(r"##\s*Status:\s*([^\n]+)", txt, re.IGNORECASE)
        if m2:
            return m2.group(1).strip()
        return "UNKNOWN"

    return {
        "factory_dispatch": get_file_status(f_disp),
        "factory_receipt": get_file_status(f_rcpt),
        "tooling_dispatch": get_file_status(t_disp),
        "tooling_receipt": get_file_status(t_rcpt),
    }


def compute_workspace_state() -> Dict[str, Any]:
    """Aggregates git, plan, communique, and peer bus locks into unified state dict."""
    return {
        "git": get_git_state(),
        "plan": parse_plan_state(),
        "communique": parse_communique_state(),
        "peer_locks": get_peer_locks(),
    }


def render_console_card(state: Dict[str, Any]) -> str:
    """Formats the state dict into a high-density console card."""
    g = state["git"]
    p = state["plan"]
    c = state["communique"]
    locks = state.get("peer_locks", [])

    clean_icon = "🟢 Clean" if g["is_clean"] else f"🟡 Dirty ({g['dirty_count']} files)"
    
    lines = [
        "=" * 70,
        "🗄️  BLACK BOX WORKSPACE STATE (Topic #7: 0-Turn Cold Boot)",
        "=" * 70,
        f"🌳 Branch:     {g['branch']} ({g['commit']}) | Working Tree: {clean_icon}",
        f"📋 Active Plan: {p['title']}",
        f"🎯 Target:     {p['target']} | Status: {p['status']} ({p['progress']} items)",
        f"⚡ Active Step: {p['active_phase']}",
        "-" * 70,
        f"🏭 Factory:    Dispatch: {c['factory_dispatch']} | Receipt: {c['factory_receipt']}",
        f"🛠️  Tooling:    Dispatch: {c['tooling_dispatch']} | Receipt: {c['tooling_receipt']}",
    ]

    if locks:
        lines.append("-" * 70)
        lines.append("🔒 Active Peer Locks:")
        for l in locks:
            lines.append(f"   • {l.get('pillar')}: {l.get('activity')} [{', '.join(l.get('scope', []))}]")

    lines.append("-" * 70)

    # Recent commit
    if g["recent_commits"]:
        lines.append(f"📜 Last Commit: {g['recent_commits'][0]}")

    lines.append("=" * 70)
    return "\n".join(lines)


def render_active_context_markdown(state: Dict[str, Any]) -> str:
    """Generates lean (<1KB) ACTIVE_CONTEXT.md for 0-turn LLM session bootstrapping."""
    g = state["git"]
    p = state["plan"]
    c = state["communique"]
    locks = state.get("peer_locks", [])
    locks_str = "None" if not locks else "; ".join(f"{l.get('pillar')}: {l.get('activity')} [{', '.join(l.get('scope', []))}]" for l in locks)

    return f"""# Active Workspace Context (0-Turn Cold Boot)
> *Generated mechanically by `tools/state_manager.py` at zero token cost.*

- **Branch / Tree:** `{g['branch']}` ({g['commit']}) · `{"Clean" if g['is_clean'] else f"Dirty ({g['dirty_count']} files)"}`
- **Active Plan:** {p['title']}
- **Target Pipeline:** `{p['target']}` · **Status:** `{p['status']}` ({p['progress']})
- **Active Phase:** {p['active_phase']}
- **Factory Semaphore:** Dispatch: `{c['factory_dispatch']}` · Receipt: `{c['factory_receipt']}`
- **Tooling Semaphore:** Dispatch: `{c['tooling_dispatch']}` · Receipt: `{c['tooling_receipt']}`
- **Peer Intent Locks:** `{locks_str}`
- **Recent Commit:** `{g['recent_commits'][0] if g['recent_commits'] else "None"}`
- **Process Runtime:** Base Antigravity (`Antigravity.exe` — companion app, not full IDE)
"""


def cmd_dump(args: argparse.Namespace) -> int:
    state = compute_workspace_state()
    if getattr(args, "json", False):
        print(json.dumps(state, indent=2))
    else:
        print(render_console_card(state))
    return 0


def cmd_sync(args: argparse.Namespace) -> int:
    state = compute_workspace_state()
    
    # Write JSON state
    json_path = REPO_ROOT / ".agents" / "pipeline" / "context_state.json"
    json_path.parent.mkdir(parents=True, exist_ok=True)
    json_path.write_text(json.dumps(state, indent=2), encoding="utf-8")

    # Write lean Markdown context (<1KB)
    md_path = REPO_ROOT / "ACTIVE_CONTEXT.md"
    md_path.write_text(render_active_context_markdown(state), encoding="utf-8")

    if not args.quiet:
        print(f"✅ Synchronized context state:")
        print(f"   • {json_path.relative_to(REPO_ROOT)}")
        print(f"   • {md_path.relative_to(REPO_ROOT)} ({md_path.stat().st_size} bytes)")
    return 0


def cmd_verify(args: argparse.Namespace) -> int:
    state = compute_workspace_state()
    issues = []
    
    # Check if communique and plan match
    p = state["plan"]
    c = state["communique"]
    if p["target"] == "FACTORY_FLOOR" and p["status"] == "READY_FOR_EXECUTION":
        if "READY" not in c["factory_dispatch"]:
            issues.append("PLAN.md is READY_FOR_EXECUTION but factory_dispatch is not.")
    
    if issues:
        print("⚠️ STATE VERIFICATION ISSUES DETECTED:")
        for iss in issues:
            print(f"   - {iss}")
        return 1
    
    print("✅ Workspace state verified healthy.")
    return 0


def main():
    parser = argparse.ArgumentParser(description="Deterministic Workspace State Sniffer")
    subparsers = parser.add_subparsers(dest="command")

    p_dump = subparsers.add_parser("dump", help="Print console state card")
    p_dump.add_argument("--json", action="store_true", help="Output raw JSON")

    p_sync = subparsers.add_parser("sync", help="Write context_state.json and ACTIVE_CONTEXT.md")
    p_sync.add_argument("--quiet", action="store_true", help="Suppress console feedback")

    subparsers.add_parser("verify", help="Assert workspace state coherence")

    args = parser.parse_args()
    cmd = args.command or "dump"

    if cmd == "dump":
        return cmd_dump(args)
    elif cmd == "sync":
        return cmd_sync(args)
    elif cmd == "verify":
        return cmd_verify(args)
    else:
        parser.print_help()
        return 1


if __name__ == "__main__":
    sys.exit(main())
