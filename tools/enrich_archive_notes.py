#!/usr/bin/env python3
"""
tools/enrich_archive_notes.py
Batch YAML Frontmatter Enrichment for Archived Vault Notes (Card rj-30 / rj-42)

Enriches archived raw notes in TheKlangVault/Inbox/Archive/ with standardized
YAML frontmatter (title, date, pillar, status, card_id, tags) mapped from
rejigger.json and mail_queue.json, preserving the original note body verbatim.

Usage:
  python tools/enrich_archive_notes.py --dry-run
  python tools/enrich_archive_notes.py
"""

import argparse
import datetime
import json
import os
from pathlib import Path
import re
import sys
from typing import Any, Dict, List, Optional

# Universal UTF-8 Environment Invariant (Flight 18)
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

DEFAULT_ARCHIVE_DIR = Path(r"C:\Dev\TheKlangVault\Inbox\Archive")
FALLBACK_ARCHIVE_DIR = Path("docs/inbox/Archive")
DEFAULT_REJIGGER_PATH = Path(".agents/pipeline/mailbag/rejigger.json")
DEFAULT_QUEUE_PATH = Path(".agents/pipeline/mailbag/mail_queue.json")


def load_card_mappings(rejigger_path: Path, queue_path: Path) -> Dict[str, Dict[str, Any]]:
    """Loads card mappings indexed by source note filename stem (case-insensitive)."""
    mapping: Dict[str, Dict[str, Any]] = {}

    # 1. Ingest queue items first
    if queue_path.exists():
        try:
            q_data = json.loads(queue_path.read_text(encoding="utf-8"))
            for item in q_data.get("items", []):
                stem = Path(item.get("source_file", item.get("file_name", ""))).stem.lower()
                if not stem:
                    stem = item.get("id", "").lower()
                mapping[stem] = {
                    "card_id": item.get("id", ""),
                    "title": item.get("title", ""),
                    "status": item.get("status", "BACKLOG"),
                    "pillars": item.get("target_pillars", ["NK"]),
                    "tags": item.get("tags", []),
                }
        except Exception as e:
            print(f"⚠️ Could not parse mail_queue.json: {e}")

    # 2. Ingest rejigger cards (higher specificity for overlaid cards)
    if rejigger_path.exists():
        try:
            r_data = json.loads(rejigger_path.read_text(encoding="utf-8"))
            for card in r_data.get("cards", []):
                card_id = card.get("id", "")
                card_title = card.get("title", "")
                card_status = card.get("status", "BACKLOG")
                card_pillars = card.get("pillars", ["NK"])
                sources = card.get("sources", [])

                for src in sources:
                    src_stem = Path(src).stem.lower()
                    mapping[src_stem] = {
                        "card_id": card_id,
                        "title": card_title,
                        "status": card_status,
                        "pillars": card_pillars,
                        "tags": ["mailbag", card_pillars[0].lower() if card_pillars else "general"],
                    }
        except Exception as e:
            print(f"⚠️ Could not parse rejigger.json: {e}")

    return mapping


def enrich_note_file(file_path: Path, meta: Optional[Dict[str, Any]], dry_run: bool = False) -> bool:
    """Safely updates or prepends YAML frontmatter without altering note body."""
    try:
        content = file_path.read_text(encoding="utf-8", errors="replace")
    except Exception as e:
        print(f"❌ Failed to read {file_path.name}: {e}")
        return False

    title = file_path.stem
    date_str = datetime.date.today().isoformat()
    pillar = "NK"
    status = "BACKLOG"
    card_id = ""
    tags = ["mailbag"]

    if meta:
        pillar = meta.get("pillars", ["NK"])[0] if meta.get("pillars") else "NK"
        status = meta.get("status", "BACKLOG")
        card_id = meta.get("card_id", "")
        tags = list(set(tags + meta.get("tags", [])))

    # Parse existing frontmatter if present
    fm_pattern = r"^---\s*\n(.*?)\n---\s*\n"
    fm_match = re.match(fm_pattern, content, re.DOTALL)

    body = content
    if fm_match:
        body = content[fm_match.end():]
        raw_fm = fm_match.group(1)
        # Preserve original date if exists
        date_m = re.search(r"^date:\s*([^\n\r]+)", raw_fm, re.MULTILINE)
        if date_m:
            date_str = date_m.group(1).strip().strip("'\"")

    # Construct new standardized frontmatter
    fm_lines = [
        "---",
        f'title: "{title}"',
        f"date: {date_str}",
        f"pillar: {pillar}",
        f"status: {status}",
    ]
    if card_id:
        fm_lines.append(f"card_id: {card_id}")
    if tags:
        tag_list_str = ", ".join(t.lower() for t in tags)
        fm_lines.append(f"tags: [{tag_list_str}]")
    fm_lines.append("---")
    fm_lines.append("")

    new_content = "\n".join(fm_lines) + body.lstrip()

    if dry_run:
        print(f"[DRY-RUN] Would enrich: {file_path.name} (card: {card_id}, pillar: {pillar}, status: {status})")
        return True

    try:
        file_path.write_text(new_content, encoding="utf-8")
        return True
    except Exception as e:
        print(f"❌ Failed to write {file_path.name}: {e}")
        return False


def main():
    parser = argparse.ArgumentParser(description="Batch YAML Frontmatter Enrichment for Archived Notes")
    parser.add_argument("--dir", help="Path to Archive directory")
    parser.add_argument("--dry-run", action="store_true", help="Inspect actions without writing files")
    args = parser.parse_args()

    archive_dir = Path(args.dir).resolve() if args.dir else DEFAULT_ARCHIVE_DIR
    if not archive_dir.exists():
        archive_dir = FALLBACK_ARCHIVE_DIR.resolve()
        if not archive_dir.exists():
            print(f"❌ Archive directory not found: {archive_dir}")
            return 1

    mapping = load_card_mappings(DEFAULT_REJIGGER_PATH, DEFAULT_QUEUE_PATH)
    print(f"🔍 Loaded {len(mapping)} card source mappings from ledger.")

    note_files = [f for f in archive_dir.glob("*.md") if f.name.lower() != "index.md"]
    print(f"📂 Found {len(note_files)} notes in {archive_dir}.")

    enriched_count = 0
    for nf in note_files:
        meta = mapping.get(nf.stem.lower())
        if enrich_note_file(nf, meta, dry_run=args.dry_run):
            enriched_count += 1

    mode_str = "[DRY-RUN] Processed" if args.dry_run else "Successfully enriched"
    print(f"✅ {mode_str} {enriched_count}/{len(note_files)} archived notes.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
