#!/usr/bin/env python3
"""
N'kai "Rambles" Ingestion Engine (Topic 2, Flight 1)
Decomposes stream-of-consciousness voice memos, unformatted phone scratchpads,
and Obsidian inbox dumps into structured 3-Bullet Decision Cards for 1-click N'kai triage.

Invariants:
- Zero external dependencies: Standard library + tools/models.py (Pydantic v2).
- Zero Premature Mutation: Never directly mutates docs/BACKLOG.md or PLAN.md.
- Hardened UTF-8 stdout for Windows terminals.
"""

import argparse
import datetime
import json
from pathlib import Path
import re
import shutil
import sys
from typing import Any, Dict, List, Optional, Tuple

# Reconfigure stdout/stderr for clean UTF-8 rendering on Windows PowerShell
if sys.stdout and hasattr(sys.stdout, "reconfigure"):
    try:
        sys.stdout.reconfigure(encoding="utf-8")
    except Exception:
        pass
if sys.stderr and hasattr(sys.stderr, "reconfigure"):
    try:
        sys.stderr.reconfigure(encoding="utf-8")
    except Exception:
        pass

# Import Pydantic models with resilient module resolution
try:
    from tools.models import (
        MailItem,
        MailQueue,
        PriorityLevel,
        RambleCard,
        RambleExtractionResult,
        TriageActionType,
        TriageStatus,
    )
except ImportError:
    try:
        from models import (
            MailItem,
            MailQueue,
            PriorityLevel,
            RambleCard,
            RambleExtractionResult,
            TriageActionType,
            TriageStatus,
        )
    except ImportError:
        _script_dir = Path(__file__).resolve().parent
        if str(_script_dir) not in sys.path:
            sys.path.insert(0, str(_script_dir))
        from models import (
            MailItem,
            MailQueue,
            PriorityLevel,
            RambleCard,
            RambleExtractionResult,
            TriageActionType,
            TriageStatus,
        )

DEFAULT_VAULT_INBOX = Path(r"C:\Dev\TheKlangVault\Inbox")
FALLBACK_LOCAL_INBOX = Path("docs/inbox")
DEFAULT_MAIL_QUEUE = Path(r".agents/pipeline/mailbag/mail_queue.json")
DEFAULT_OVERFLOW_FILE = Path(r".agents/pipeline/mailbag/overflow.json")


def resolve_inbox_directory(dir_override: Optional[str] = None, scan_archive: bool = False) -> Path:
    """Resolves target inbox directory with hierarchical fallbacks."""
    if dir_override:
        target = Path(dir_override).resolve()
        if scan_archive:
            archive_target = target / "Archive"
            if archive_target.exists():
                return archive_target
        return target

    if DEFAULT_VAULT_INBOX.exists():
        if scan_archive:
            archive_path = DEFAULT_VAULT_INBOX / "Archive"
            if archive_path.exists():
                return archive_path
        return DEFAULT_VAULT_INBOX

    if FALLBACK_LOCAL_INBOX.exists():
        if scan_archive:
            archive_path = FALLBACK_LOCAL_INBOX / "Archive"
            if archive_path.exists():
                return archive_path
        return FALLBACK_LOCAL_INBOX

    return Path.cwd()


def clean_markdown_text(text: str) -> str:
    """Removes excessive markdown styling and normalizes whitespace."""
    t = re.sub(r"\[(.*?)\]\(.*?\)", r"\1", text)
    t = re.sub(r"[*_`]", "", t)
    t = re.sub(r"\s+", " ", t).strip()
    return t


def detect_pillar(text: str) -> str:
    """Identifies target ecosystem pillar using keyword scoring."""
    t_lower = text.lower()

    scores = {
        "NK": 0,
        "TKS": 0,
        "TT": 0,
        "TBD-16": 0,
        "TKR": 0,
    }

    # NK (N'kai) keywords
    nk_words = ["nkai", "sidecar", "triage", "mailbag", "autopsy", "vault", "inbox", "ramble", "canvas", "bridge", "subagent", "prompt"]
    for w in nk_words:
        if w in t_lower:
            scores["NK"] += 3

    # TT (ToadTracker) keywords
    tt_words = ["tracker", "toad", "m8", "dirtywave", "engine", "tracktion", "groovebox", "step", "pattern", "table", "cydrums"]
    for w in tt_words:
        if w in t_lower:
            scores["TT"] += 3

    # TBD-16 hardware keywords
    tbd_words = ["esp32", "p4", "tbd", "tbd-16", "rp2350", "oled", "encoder", "gpio", "hardware", "teensy"]
    for w in tbd_words:
        if w in t_lower:
            scores["TBD-16"] += 3

    # TKR (The Klang Research) keywords
    tkr_words = ["rfc", "math", "dsp paper", "teardown", "aliasing", "proof", "academic", "benchmark"]
    for w in tkr_words:
        if w in t_lower:
            scores["TKR"] += 3

    # TKS (The Klang Suite) keywords
    tks_words = [
        "vst", "synth", "oscillator", "filter", "macro", "sound", "patch", "preset",
        "juce", "adsr", "envelope", "chorus", "delay", "reverb", "distortion", "flanger",
        "phaser", "amp", "waveshaper", "compressor", "effect", "effects", "slice eq",
        "snapheap", "snap heap", "tkm", "tkf", "tkp", "physical modeling", "lossy", "808", "909"
    ]
    for w in tks_words:
        if w in t_lower:
            scores["TKS"] += 2

    # Return pillar with maximum score (minimum 2 points)
    best_pillar = max(scores, key=scores.get)
    if scores[best_pillar] >= 2:
        return best_pillar

    # Default to TKS if general audio context, else NK
    if any(k in t_lower for k in ["sound", "audio", "music", "knob", "card"]):
        return "TKS"
    return "NK"


def detect_action_route(text: str) -> Tuple[TriageActionType, float]:
    """Determines suggested action route and confidence score."""
    t_lower = text.lower()

    # 1. Highest priority / concrete proposals take precedence
    if any(k in t_lower for k in [
        "highest priority", "most higest priority", "most highest priority",
        "top priority", "#p0", "priority: p0", "definitely", "need to make",
        "should have", "we can combine"
    ]):
        return TriageActionType.BACKLOG, 0.90

    # 2. Graveyard cues (unless hypothetical, questioning, or testing bad workflow)
    if any(k in t_lower for k in ["bad idea", "maybe not", "stupid", "kill this", "waste of time"]):
        if any(h in t_lower for h in [
            "if you don't tell me", "seeing if it makes for bad", "might be a bad idea",
            "not totally sure", "tell me if", "try out both"
        ]):
            if any(p in t_lower for p in ["better with", "hierarchy", "meta container", "container", "vs", "or,"]):
                return TriageActionType.PCDA, 0.88
            return TriageActionType.RESEARCH, 0.85
        return TriageActionType.GRAVEYARD, 0.88

    # 3. Trade-offs, taxonomies, and architectural comparisons -> PCDA
    if any(k in t_lower for k in [
        "better with", "or, less files", "tons of files? or", "vs", "versus",
        "trade-off", "tradeoff", "hierarchy", "meta container", "this might just be tkm"
    ]):
        return TriageActionType.PCDA, 0.88

    # 4. Interrogatives & Explorations -> RESEARCH
    if any(k in t_lower for k in [
        "?", "figure out", "wondering", "is this a good", "can we try to",
        "should we", "not totally sure", "throwing everything at the wall", "try out"
    ]):
        return TriageActionType.RESEARCH, 0.85

    # 5. Add / implement / backlog default
    if any(k in t_lower for k in ["add", "implement", "could have", "combine"]):
        return TriageActionType.BACKLOG, 0.85

    return TriageActionType.BACKLOG, 0.80


def synthesize_card_fields(
    raw_block: str,
    source_name: str,
    idea_index: int,
    pillar: str,
    route: TriageActionType,
    confidence: float,
) -> Dict[str, Any]:
    """Synthesizes title, Core Intent, Proposed Mechanism, and tags from block text."""
    clean_text = clean_markdown_text(raw_block)
    t_lower = clean_text.lower()

    # Tags extraction
    tags = ["ramble"]
    explicit_tags = re.findall(r"#([a-zA-Z0-9_\-]+)", raw_block)
    for et in explicit_tags:
        tags.append(et.lower())

    if "p0" in t_lower or "highest priority" in t_lower or "most higest priority" in t_lower:
        tags.append("p0")
    if "workflow" in t_lower:
        tags.append("workflow")
    if "macro" in t_lower or "knob" in t_lower:
        tags.append("ui")
    if any(k in t_lower for k in ["dsp", "effect", "effects", "oscillator", "filter", "synth"]):
        tags.append("dsp")

    # Domain specific synthesis rules
    title = ""
    core_intent = ""
    mechanism = ""

    # Rule 1: Ramble ingestion workflow
    if "ramble" in t_lower and ("discrete ideas" in t_lower or "figure out what to do with them" in t_lower or "wanted nkai" in t_lower):
        title = "Stream-of-Consciousness Ramble Ingestion Workflow"
        core_intent = "Establish an automated workflow in N'kai to ingest stream-of-consciousness rambles and decompose them into actionable research items."
        mechanism = "N'kai sidecar intake pipeline & ramble ingestor CLI (tools/ramble_ingest.py)"

    # Rule 2: Note granularity & vault architecture
    elif "short rambles" in t_lower or "tons of files" in t_lower or "less files with longer rambles" in t_lower:
        title = "Ramble Note Granularity & Vault Architecture"
        core_intent = "Evaluate trade-offs between short targeted notes across many files versus fewer files with longer rambles."
        mechanism = "Obsidian vault information architecture & batch note ingestion strategy"

    # Rule 3: 4-Macro popover panel (Slice EQ / Snap Heap)
    elif "macro" in t_lower and ("popover" in t_lower or "snapheap" in t_lower or "slice eq" in t_lower):
        title = "4-Macro Popover Panels & Sub-Rack Control"
        core_intent = "Implement 4-macro knobs with a popover settings panel (Snap Heap / Slice EQ style) for complex synthesizer and effect modules."
        mechanism = "JUCE UI popover callout component & APVTS macro parameter mapping (TKS/TKM)"

    # Rule 4: Catalog expansion / wishlist
    elif any(k in t_lower for k in ["goodhertzlossy", "808", "machinedrum", "k5 additive", "plaits and braids", "analog heat"]):
        title = "FX & Oscillator Catalog Expansion Wishlist"
        core_intent = "Explore and prototype a wide catalog of vintage drum models, lo-fi degradation, chiptune oscillators, and modulation effects."
        mechanism = "Modular DSP blocks and algorithm expansion in ModularBlocks.h"

    # Rule 5: Combining algorithm variants
    elif "variants of the same idea" in t_lower or "combine a lot of them" in t_lower:
        title = "Consolidation of Variant FX Algorithms"
        core_intent = "Combine closely related modulation and oscillator variants into unified multi-algorithm DSP blocks."
        mechanism = "Multi-algorithm DSP effect classes with mode switching"

    # Rule 6: Effect hierarchy / meta container
    elif "categorize all the effects" in t_lower or "meta container" in t_lower or "hierarchy" in t_lower:
        title = "Categorized Multi-FX Meta-Container Hierarchy"
        core_intent = "Create a hierarchical taxonomy grouping related effects (filters, spatial audio, delays) into shared meta-containers."
        mechanism = "Modular FX rack architecture & container routing engine"

    # Fallback generic synthesis
    else:
        # Extract first salient sentence
        sentences = [s.strip() for s in re.split(r"(?<=[.!?])\s+", clean_text) if s.strip()]
        first_s = sentences[0] if sentences else clean_text
        # Clean filler prefixes
        first_s_clean = re.sub(
            r"^(you seemed to be|i kinda think|i was wondering if|just trying to figure out|can we try to|is this a good|i’m throwing|im throwing)\s+",
            "",
            first_s,
            flags=re.IGNORECASE,
        ).strip()
        first_s_clean = first_s_clean[0].upper() + first_s_clean[1:] if first_s_clean else "Exploration"

        title_candidate = first_s_clean[:60]
        if len(first_s_clean) > 60:
            title_candidate = title_candidate.rsplit(" ", 1)[0]
        title = title_candidate or f"{source_name} Idea {idea_index}"

        core_intent = first_s if first_s.endswith((".", "!", "?")) else f"{first_s}."
        mechanism = f"{pillar} module architecture & pipeline implementation"

    # Enforce title length limit
    title = title[:64]

    # Generate raw snippet
    snippet = clean_text[:280] + ("..." if len(clean_text) > 280 else "")

    return {
        "title": title,
        "core_intent": core_intent,
        "proposed_mechanism": mechanism,
        "tags": list(dict.fromkeys(tags)),
        "raw_snippet": snippet,
    }


def save_overflow_cards(
    overflow_cards: List[RambleCard],
    overflow_path: Optional[Path] = None,
    split_group: str = "",
    dry_run: bool = False,
) -> int:
    """
    Writes overflow cards to overflow.json as a JSON array of RambleCard objects.
    Preserves cards from other split groups and replaces cards for this split_group idempotently.
    """
    target = overflow_path or DEFAULT_OVERFLOW_FILE
    existing: List[Dict[str, Any]] = []
    if target.exists():
        try:
            content = target.read_text(encoding="utf-8")
            parsed = json.loads(content)
            if isinstance(parsed, list):
                existing = parsed
        except Exception:
            existing = []

    # Filter out previous items for this split_group
    if split_group:
        kept = [c for c in existing if c.get("split_group") != split_group]
    else:
        kept = existing

    # Append new overflow cards
    for oc in overflow_cards:
        kept.append(oc.model_dump())

    if not dry_run:
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(json.dumps(kept, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")

    return len(overflow_cards)


def parse_ramble_file(
    file_path: Path,
    overflow_path: Optional[Path] = None,
    write_overflow: bool = True,
    dry_run: bool = False,
) -> RambleExtractionResult:
    """Parses a markdown ramble note and decomposes it into RambleCards (top 3 active, remainder to overflow)."""
    try:
        content = file_path.read_text(encoding="utf-8", errors="replace")
    except Exception:
        return RambleExtractionResult(
            source_file=str(file_path),
            total_ideas=0,
            cards=[],
        )

    # 1. Parse and strip YAML frontmatter
    fm_match = re.match(r"^---\s*\n(.*?)\n---\s*\n(.*)$", content, re.DOTALL)
    body = content
    fm_tags = []
    fm_priority = None
    if fm_match:
        fm_text = fm_match.group(1)
        body = fm_match.group(2)
        fm_tags = re.findall(r"#([a-zA-Z0-9_\-]+)", fm_text)
        pm = re.search(r"^priority:\s*[\"']?(.*?)[\"']?\s*$", fm_text, re.MULTILINE | re.IGNORECASE)
        if pm:
            fm_priority = pm.group(1).strip().upper()

    # 2. Extract top markdown heading if present
    note_heading = None
    heading_m = re.match(r"^#\s+(.+)$", body.strip())
    if heading_m:
        note_heading = heading_m.group(1).strip()
        body = body[heading_m.end():].strip()

    # 3. Identify discrete paragraphs and structural blocks
    raw_blocks = [b.strip() for b in re.split(r"\n\s*\n", body) if b.strip()]

    # Group coherent blocks
    grouped_blocks: List[str] = []
    list_accumulator: List[str] = []

    for block in raw_blocks:
        lines = [line.strip() for line in block.splitlines() if line.strip()]
        is_bullet_list = (sum(1 for l in lines if l.startswith(("- ", "* ", "1.", "2.", "3.", "4.", "5.")) or re.match(r"^\d+\.", l)) >= len(lines) * 0.5)

        # Check if this block is an intro or continuation of a bullet list
        if is_bullet_list:
            if list_accumulator:
                # Flush previous list accumulator
                grouped_blocks.append("\n\n".join(list_accumulator))
                list_accumulator = []
            # Check if previous block was an intro to this list
            if grouped_blocks and ("throwing everything at the wall" in grouped_blocks[-1].lower() or "options" in grouped_blocks[-1].lower()):
                intro = grouped_blocks.pop()
                grouped_blocks.append(f"{intro}\n\n{block}")
            else:
                grouped_blocks.append(block)
            continue

        # Check for short trailing continuations (< 75 chars)
        is_continuation = (
            len(block) < 75
            and not block.lower().startswith(("also,", "another idea", "what about", "can we try to"))
            and not is_bullet_list
        )

        if is_continuation and grouped_blocks:
            grouped_blocks[-1] = f"{grouped_blocks[-1]}\n\n{block}"
        else:
            grouped_blocks.append(block)

    # 4. Synthesize cards for each grouped block
    cards: List[RambleCard] = []
    source_name = file_path.stem

    for idx, g_block in enumerate(grouped_blocks, 1):
        pillar = detect_pillar(g_block)
        route, confidence = detect_action_route(g_block)
        synthesis = synthesize_card_fields(g_block, source_name, idx, pillar, route, confidence)

        # Add frontmatter tags if any
        for ft in fm_tags:
            if ft.lower() not in synthesis["tags"]:
                synthesis["tags"].append(ft.lower())
        if fm_priority and fm_priority.lower() not in synthesis["tags"]:
            synthesis["tags"].append(fm_priority.lower())

        card = RambleCard(
            source_note=str(file_path),
            idea_index=idx,
            title=synthesis["title"],
            core_intent=synthesis["core_intent"],
            proposed_mechanism=synthesis["proposed_mechanism"],
            action_route=route,
            target_pillar=pillar,
            tags=synthesis["tags"],
            raw_snippet=synthesis["raw_snippet"],
            confidence_score=confidence,
            split_group=source_name,
        )
        cards.append(card)

    # Phase 1 Overflow Queue: If > 3 ideas, promote top 3 to active queue, remainder to overflow.json
    if len(cards) > 3:
        active_cards = cards[:3]
        overflow_cards = cards[3:]
        if write_overflow:
            save_overflow_cards(
                overflow_cards,
                overflow_path=overflow_path,
                split_group=source_name,
                dry_run=dry_run,
            )
        cards = active_cards

    return RambleExtractionResult(
        source_file=str(file_path),
        extracted_at=datetime.datetime.now().isoformat(),
        total_ideas=len(cards),
        cards=cards,
    )


def format_chat_output(results: List[RambleExtractionResult]) -> str:
    """Formats extraction results as a clean Markdown report for terminal output."""
    lines = [
        "# 🎙️ N'kai Ramble Extraction Report",
        f"**Generated:** {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}",
        f"**Processed Notes:** {len(results)}",
        "",
    ]

    total_cards = sum(r.total_ideas for r in results)
    lines.append(f"**Total Decomposed Ideas:** {total_cards}")
    lines.append("")

    for res in results:
        p_name = Path(res.source_file).name
        lines.append(f"## 📄 `{p_name}` ({res.total_ideas} Ideas)")
        lines.append(f"- **Path:** `{res.source_file}`")
        lines.append("")

        for card in res.cards:
            route_emoji = {
                TriageActionType.BACKLOG: "📥 Backlog",
                TriageActionType.RESEARCH: "🔬 Research",
                TriageActionType.GRAVEYARD: "🪦 Graveyard",
                TriageActionType.PCDA: "⚖️ PCDA",
            }.get(card.action_route, "📥 Backlog")

            tags_str = " ".join([f"#{t}" for t in card.tags])

            lines.append(f"### 🎙️ [{card.target_pillar}] {card.title} (#{card.idea_index})")
            lines.append(f"- 🎯 **Core Intent**: {card.core_intent}")
            lines.append(f"- 🛠️ **Proposed Mechanism**: {card.proposed_mechanism}")
            lines.append(f"- 🚦 **Action Route**: `[ {route_emoji} ]` (Confidence: {card.confidence_score:.2f})")
            if tags_str:
                lines.append(f"- 🏷️ **Tags**: {tags_str}")
            if card.raw_snippet:
                lines.append(f"> *Snippet*: \"{card.raw_snippet}\"")
            lines.append("")

    return "\n".join(lines)


def append_cards_to_mail_queue(cards: List[RambleCard], queue_path: Path, dry_run: bool = False) -> Tuple[int, int]:
    """
    Converts RambleCards to MailItems, updates/appends to mail_queue.json,
    and sorts the queue with P0 items at top.
    Returns (added_count, updated_count).
    """
    if queue_path.exists():
        try:
            queue = MailQueue.model_validate_json(queue_path.read_text(encoding="utf-8"))
        except Exception:
            queue = MailQueue()
    else:
        queue = MailQueue()

    existing_map: Dict[str, int] = {it.id: idx for idx, it in enumerate(queue.items)}
    added = 0
    updated = 0

    for card in cards:
        m_item = card.to_mail_item()
        if m_item.id in existing_map:
            idx = existing_map[m_item.id]
            queue.items[idx] = m_item
            updated += 1
        else:
            queue.items.append(m_item)
            existing_map[m_item.id] = len(queue.items) - 1
            added += 1

    queue.sort_by_priority()

    if not dry_run:
        queue_path.parent.mkdir(parents=True, exist_ok=True)
        queue_path.write_text(queue.model_dump_json(indent=2), encoding="utf-8")

    return added, updated


def archive_processed_notes(processed_files: List[Path], inbox_dir: Path, dry_run: bool = False) -> List[Path]:
    """Moves successfully ingested notes from inbox_dir to inbox_dir/Archive."""
    archive_dir = inbox_dir / "Archive"
    moved: List[Path] = []

    for f in processed_files:
        src = f.resolve()
        # Only move files that are directly in inbox_dir (not already inside Archive)
        if src.parent.resolve() == inbox_dir.resolve() and src.is_file():
            dst = archive_dir / src.name
            if not dry_run:
                archive_dir.mkdir(parents=True, exist_ok=True)
                shutil.move(str(src), str(dst))
            moved.append(dst)

    return moved


def main():
    parser = argparse.ArgumentParser(
        description="N'kai 'Rambles' Ingestion Engine: Decomposes unstructured thoughts into 3-Bullet Decision Cards."
    )
    parser.add_argument("--file", "-f", type=str, help="Path to a single markdown ramble note.")
    parser.add_argument("--inbox", action="store_true", help="Scan active notes in TheKlangVault/Inbox/.")
    parser.add_argument("--scan-archive", action="store_true", help="Scan archived notes in TheKlangVault/Inbox/Archive/.")
    parser.add_argument("--chat", action="store_true", default=True, help="Output clean Markdown cards to console (default).")
    parser.add_argument("--json", action="store_true", help="Output machine-readable JSON structure.")
    parser.add_argument("--add-to-queue", action="store_true", help="Append extracted cards to mail_queue.json.")
    parser.add_argument("--archive", action="store_true", help="Move processed notes from Inbox/ to Inbox/Archive/.")
    parser.add_argument("--dry-run", action="store_true", help="Simulate without modifying files or queues.")
    parser.add_argument("--queue-file", type=str, default=str(DEFAULT_MAIL_QUEUE), help="Custom path to mail_queue.json.")
    parser.add_argument("--overflow-file", type=str, default=str(DEFAULT_OVERFLOW_FILE), help="Custom path to overflow.json.")

    args = parser.parse_args()

    files_to_process: List[Path] = []
    inbox_dir: Optional[Path] = None

    if args.file:
        target_f = Path(args.file).resolve()
        if not target_f.exists():
            print(f"❌ Error: Specified file not found: {target_f}", file=sys.stderr)
            sys.exit(1)
        files_to_process.append(target_f)
        inbox_dir = target_f.parent
    else:
        inbox_dir = resolve_inbox_directory(scan_archive=args.scan_archive)
        if not inbox_dir.exists():
            print(f"❌ Error: Inbox directory not found: {inbox_dir}", file=sys.stderr)
            sys.exit(1)
        files_to_process = sorted(inbox_dir.glob("*.md"), key=lambda f: f.stat().st_mtime, reverse=True)

    if not files_to_process:
        if args.json:
            print(json.dumps({"status": "empty", "message": "No markdown files found."}, indent=2))
        else:
            print("📬 No markdown files found to process.")
        return

    # Process all notes
    results: List[RambleExtractionResult] = []
    all_cards: List[RambleCard] = []
    overflow_path = Path(args.overflow_file)

    for f in files_to_process:
        res = parse_ramble_file(f, overflow_path=overflow_path, write_overflow=True, dry_run=args.dry_run)
        results.append(res)
        all_cards.extend(res.cards)

    # Queue ingestion if requested
    queue_file = Path(args.queue_file)
    added_q = 0
    updated_q = 0
    if args.add_to_queue and all_cards:
        added_q, updated_q = append_cards_to_mail_queue(all_cards, queue_file, dry_run=args.dry_run)

    # Archival if requested
    moved_files: List[Path] = []
    if args.archive and inbox_dir:
        moved_files = archive_processed_notes(files_to_process, inbox_dir, dry_run=args.dry_run)

    # Output rendering
    if args.json:
        payload = {
            "total_notes": len(results),
            "total_cards": len(all_cards),
            "queue_added": added_q,
            "queue_updated": updated_q,
            "archived_files": [str(p) for p in moved_files],
            "results": [r.model_dump() for r in results],
        }
        print(json.dumps(payload, indent=2, ensure_ascii=False))
    else:
        report = format_chat_output(results)
        print(report)
        if args.add_to_queue:
            status_tag = " (DRY RUN)" if args.dry_run else ""
            print(f"⚡ Mail Queue Updated{status_tag}: {added_q} added, {updated_q} updated in `{queue_file}`.")
        if args.archive and moved_files:
            status_tag = " (DRY RUN)" if args.dry_run else ""
            print(f"📦 Archived {len(moved_files)} notes to `{inbox_dir / 'Archive'}`{status_tag}.")


if __name__ == "__main__":
    main()
