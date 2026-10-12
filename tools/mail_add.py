#!/usr/bin/env python3
"""
N'kai Mailbag Add CLI with Position Splicing (Flight 7 / rj-07)
Adds a new note to the inbox and splices a corresponding card into rejigger.json
at an exact target position (--after <id> or --rank <n>), automatically renumbering
subsequent cards contiguously.

Usage:
  python tools/mail_add.py "My New Idea" --body "Details here" --pillar NK --priority P1 --after rj-05
  python tools/mail_add.py "Quick Note" --rank 3 --dry-run
"""

import argparse
import datetime
import json
from pathlib import Path
import re
import sys
from typing import Any, Dict, List, Optional

# Universal UTF-8 Environment Invariant (Flight 18)
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

# Import models
try:
    from tools.models import EcosystemPillar, PriorityLevel, PILLAR_METADATA
except ImportError:
    try:
        from models import EcosystemPillar, PriorityLevel, PILLAR_METADATA
    except ImportError:
        _script_dir = Path(__file__).resolve().parent
        if str(_script_dir) not in sys.path:
            sys.path.insert(0, str(_script_dir))
        from models import EcosystemPillar, PriorityLevel, PILLAR_METADATA

DEFAULT_VAULT_INBOX = Path(r"C:\_\Vault\Inbox") if Path(r"C:\_\Vault\Inbox").exists() else Path(r"C:\Dev\TheKlangVault\Inbox")
FALLBACK_LOCAL_INBOX = Path("docs/inbox")
DEFAULT_REJIGGER_PATH = Path(r".agents/pipeline/mailbag/rejigger.json")


def resolve_inbox_dir(custom_path: Optional[str] = None) -> Path:
    if custom_path:
        p = Path(custom_path).resolve()
        p.mkdir(parents=True, exist_ok=True)
        return p
    if DEFAULT_VAULT_INBOX.exists():
        return DEFAULT_VAULT_INBOX
    FALLBACK_LOCAL_INBOX.mkdir(parents=True, exist_ok=True)
    return FALLBACK_LOCAL_INBOX


def resolve_rejigger_file(custom_path: Optional[str] = None) -> Path:
    if custom_path:
        return Path(custom_path).resolve()
    for base in [Path.cwd(), Path(__file__).resolve().parent.parent]:
        cand = base / DEFAULT_REJIGGER_PATH
        if cand.exists():
            return cand
    return DEFAULT_REJIGGER_PATH


def sanitize_filename(title: str) -> str:
    cleaned = re.sub(r'[\\/*?:"<>|]', "", title).strip()
    return cleaned or "Untitled_Note"


def add_mail_item(
    title: str,
    body: str = "",
    pillar: str = "TKS",
    priority: str = "P1",
    after_id: Optional[str] = None,
    rank: Optional[int] = None,
    inbox_dir_override: Optional[str] = None,
    rejigger_path_override: Optional[str] = None,
    dry_run: bool = False,
) -> Dict[str, Any]:
    """
    Creates note in inbox directory and splices a new card into rejigger.json at the target position.
    Returns details of the created item and its spliced rank.
    """
    pillar_clean = pillar.strip().upper()
    valid_pillars = [p.value for p in EcosystemPillar]
    if pillar_clean not in valid_pillars:
        pillar_clean = "TKS"

    priority_clean = priority.strip().upper()
    if priority_clean not in ["P0", "P1", "P2", "P3"]:
        priority_clean = "P1"

    inbox_dir = resolve_inbox_dir(inbox_dir_override)
    rejigger_file = resolve_rejigger_file(rejigger_path_override)

    today_str = datetime.date.today().strftime("%Y-%m-%d")
    file_stem = sanitize_filename(title)
    note_path = inbox_dir / f"{file_stem}.md"

    # 1. Format Markdown Note Content
    note_content = f"""---
title: "{title}"
date: {today_str}
priority: {priority_clean}
pillar: {pillar_clean}
---

{body.strip()}
"""

    if not dry_run:
        note_path.write_text(note_content, encoding="utf-8")

    # 2. Splice into rejigger.json
    rej_data: Dict[str, Any] = {"schema_version": "1.0.0", "cards": []}
    if rejigger_file.exists():
        try:
            rej_data = json.loads(rejigger_file.read_text(encoding="utf-8"))
        except Exception:
            rej_data = {"schema_version": "1.0.0", "cards": []}

    cards: List[Dict[str, Any]] = rej_data.get("cards", [])

    # Determine unique card ID
    existing_ids = {c.get("id") for c in cards}
    max_num = 0
    for cid in existing_ids:
        if cid and cid.startswith("rj-"):
            m = re.match(r"^rj-(\d+)$", cid)
            if m:
                max_num = max(max_num, int(m.group(1)))
    new_card_id = f"rj-{max_num + 1:02d}"

    # Determine insertion index (0-indexed)
    target_idx = len(cards)
    if after_id:
        found_idx = None
        for i, c in enumerate(cards):
            if c.get("id", "").lower() == after_id.lower():
                found_idx = i
                break
        if found_idx is not None:
            target_idx = found_idx + 1
        else:
            print(f"⚠️ Warning: Target card '{after_id}' not found; appending to end.", file=sys.stderr)
    elif rank is not None:
        target_idx = max(0, min(rank - 1, len(cards)))

    # Construct new card
    new_card = {
        "id": new_card_id,
        "rank": target_idx + 1,
        "priority": priority_clean,
        "status": "PENDING",
        "tier": "tier2",
        "pillars": [pillar_clean],
        "title": title,
        "summary": body.strip()[:280] if body.strip() else title,
        "angles": [body.strip()] if body.strip() else [title],
        "next_action": "Triage in morning mail.",
        "sources": [file_stem],
        "verdict": None,
        "verdict_notes": None,
    }

    # Splice and renumber
    cards.insert(target_idx, new_card)
    for idx, c in enumerate(cards, 1):
        c["rank"] = idx

    rej_data["cards"] = cards
    rej_data["generated_at"] = datetime.datetime.now().isoformat()

    if not dry_run:
        rejigger_file.parent.mkdir(parents=True, exist_ok=True)
        rejigger_file.write_text(json.dumps(rej_data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")

    return {
        "id": new_card_id,
        "rank": target_idx + 1,
        "title": title,
        "pillar": pillar_clean,
        "priority": priority_clean,
        "note_path": str(note_path),
        "rejigger_file": str(rejigger_file),
        "total_cards": len(cards),
        "dry_run": dry_run,
    }


def main():
    parser = argparse.ArgumentParser(
        description="N'kai Mail-Add CLI: Adds a note and splices card into rejigger.json."
    )
    parser.add_argument("title", type=str, help="Title of the new note or idea.")
    parser.add_argument("--body", "-b", type=str, default="", help="Note body or creative description.")
    parser.add_argument("--pillar", "-p", type=str, default="TKS", help="Ecosystem pillar (TKS, TT, NK, TKR, AGY, TBD-16).")
    parser.add_argument("--priority", type=str, default="P1", help="Priority level (P0, P1, P2, P3).")
    pos_group = parser.add_mutually_exclusive_group()
    pos_group.add_argument("--after", "-a", type=str, help="Splice immediately after existing card ID (e.g. rj-05).")
    pos_group.add_argument("--rank", "-r", type=int, help="Exact 1-indexed target rank position.")
    parser.add_argument("--dry-run", action="store_true", help="Simulate without modifying files.")
    parser.add_argument("--inbox-dir", type=str, help="Custom inbox directory path.")
    parser.add_argument("--rejigger-path", type=str, help="Custom rejigger.json path.")
    parser.add_argument("--json", action="store_true", help="Output machine-readable JSON.")

    args = parser.parse_args()

    result = add_mail_item(
        title=args.title,
        body=args.body,
        pillar=args.pillar,
        priority=args.priority,
        after_id=args.after,
        rank=args.rank,
        inbox_dir_override=args.inbox_dir,
        rejigger_path_override=args.rejigger_path,
        dry_run=args.dry_run,
    )

    if args.json:
        print(json.dumps(result, indent=2))
    else:
        dry_tag = " (DRY RUN)" if result["dry_run"] else ""
        print(f"✅ Mail Item Added{dry_tag}:")
        print(f"   • Card ID:    {result['id']} (Rank #{result['rank']} of {result['total_cards']})")
        print(f"   • Title:      {result['title']}")
        print(f"   • Pillar:     [{result['pillar']}] | Priority: {result['priority']}")
        print(f"   • Note File:  {result['note_path']}")
        print(f"   • Rejigger:   {result['rejigger_file']}")


if __name__ == "__main__":
    main()
