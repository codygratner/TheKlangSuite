#!/usr/bin/env python3
"""
N'kai "Check the Mail" Vault Inbox Harvester & Interactive Triage Lab (Topic 6.11)
Discovers and extracts raw thoughts, design ideas, and inbox notes from
Obsidian Vault Inbox (or local fallback), generating an interactive N'kai triage
board (morning_mail_inbox.html) or dumping a terminal-friendly Markdown ledger (--chat).

Features:
- Two-Tier Header with live Allowance Meter & Web Audio Haptics toggle
- Curated Test-Drive Docket (12 active topics) + Full Vault Archive (24 notes)
- Procedural Web Audio API sound effects (tick detents & action chimes)
- Real-time instant JavaScript filtering by keyword, status, and tab
- Dynamic token impact estimation for cumulative quota governance
- Tri-mode environment dispatching (NkaiEnv, Clipboard, or Prompt Box)

Zero external dependencies: Python standard library only.
"""

import argparse
import datetime
import html
import json
import os
from pathlib import Path
import re
import shutil
import sys
import webbrowser

# Universal UTF-8 Environment Invariant (Flight 18)
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

# Import Pydantic models for persistent Mailbag session (Flight 5 & 15)
try:
    from tools.models import MailItem, MailQueue, PriorityLevel, TriageStatus, TransactionReceipt, EcosystemPillar, PILLAR_METADATA, infer_pillars, BoardSchema
except ImportError:
    try:
        from models import MailItem, MailQueue, PriorityLevel, TriageStatus, TransactionReceipt, EcosystemPillar, PILLAR_METADATA, infer_pillars, BoardSchema
    except ImportError:
        _script_dir = Path(__file__).resolve().parent
        if str(_script_dir) not in sys.path:
            sys.path.insert(0, str(_script_dir))
        from models import MailItem, MailQueue, PriorityLevel, TriageStatus, TransactionReceipt, EcosystemPillar, PILLAR_METADATA, infer_pillars, BoardSchema

try:
    from tools.board_renderer import render_board
except ImportError:
    try:
        from board_renderer import render_board
    except ImportError:
        _script_dir = Path(__file__).resolve().parent
        if str(_script_dir) not in sys.path:
            sys.path.insert(0, str(_script_dir))
        from board_renderer import render_board

DEFAULT_VAULT_INBOX = Path(r"C:\Dev\TheKlangVault\Inbox")
FALLBACK_LOCAL_INBOX = Path("docs/inbox")
DEFAULT_OUTPUT_HTML = Path(r".agents/sidecar/morning_mail_inbox.html")
DEFAULT_SANDBOX_OUTPUT_HTML = Path(r".agents/sidecar/sandbox_mail_inbox.html")
DEFAULT_SANDBOX_FIXTURE = Path(r"assets/fixtures/sandbox_mail_queue.json")
DEFAULT_DOCKET_PATH = Path(r".agents/sidecar/morning_mail_topics.md")
DEFAULT_MAIL_QUEUE = Path(r".agents/pipeline/mailbag/mail_queue.json")
DEFAULT_TX_RECEIPT = Path(r".agents/pipeline/staging/mail_tx_latest.json")
DEFAULT_REJIGGER_PATH = Path(r".agents/pipeline/mailbag/rejigger.json")
DEFAULT_BACKLOG_PATH = Path("docs/BACKLOG.md")
if not DEFAULT_BACKLOG_PATH.exists():
    alt_backlog = Path(__file__).resolve().parent.parent / "docs" / "BACKLOG.md"
    if alt_backlog.exists():
        DEFAULT_BACKLOG_PATH = alt_backlog


def resolve_inbox_directory(dir_override=None, scan_archive=False):
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


def extract_note_metadata(file_path: Path):
    """Parses frontmatter, title, date, excerpt, and raw body from a Markdown note."""
    try:
        content = file_path.read_text(encoding="utf-8", errors="replace")
    except Exception:
        return None

    title = None
    date_str = None
    body = content

    # 1. Parse YAML frontmatter if present
    fm_match = re.match(r"^---\s*\n(.*?)\n---\s*\n(.*)$", content, re.DOTALL)
    if fm_match:
        fm_text = fm_match.group(1)
        body = fm_match.group(2)

        title_m = re.search(r"^title:\s*[\"']?(.*?)[\"']?\s*$", fm_text, re.MULTILINE | re.IGNORECASE)
        if title_m:
            title = title_m.group(1).strip()

        date_m = re.search(r"^date:\s*[\"']?(.*?)[\"']?\s*$", fm_text, re.MULTILINE | re.IGNORECASE)
        if date_m:
            date_str = date_m.group(1).strip()

    # 2. Extract title from first Markdown heading if not in frontmatter
    if not title:
        heading_m = re.search(r"^#\s+(.+)$", body, re.MULTILINE)
        if heading_m:
            title = heading_m.group(1).strip()
        else:
            title = file_path.stem

    # 3. Extract creation / modification date
    if not date_str:
        try:
            mtime = file_path.stat().st_mtime
            date_str = datetime.datetime.fromtimestamp(mtime).strftime("%Y-%m-%d")
        except Exception:
            date_str = datetime.date.today().strftime("%Y-%m-%d")

    # 4. Generate clean body excerpt
    clean_lines = []
    for line in body.splitlines():
        trimmed = line.strip()
        if not trimmed or trimmed.startswith("#") or trimmed.startswith("---"):
            continue
        clean_lines.append(trimmed)

    clean_text = " ".join(clean_lines)
    clean_text = re.sub(r"\[(.*?)\]\(.*?\)", r"\1", clean_text)  # strip links
    clean_text = re.sub(r"[*_`]", "", clean_text)  # strip formatting
    clean_text = re.sub(r"\s+", " ", clean_text).strip()

    excerpt = clean_text[:280] + ("..." if len(clean_text) > 280 else "")
    if not excerpt:
        excerpt = "(Empty note body)"

    # Determine priority heuristic
    priority = "P2"
    all_text = (title + " " + body).lower()
    fm_priority = None
    if fm_match:
        pm = re.search(r"^priority:\s*[\"']?(.*?)[\"']?\s*$", fm_match.group(1), re.MULTILINE | re.IGNORECASE)
        if pm:
            fm_priority = pm.group(1).strip().upper()

    if fm_priority in ["P0", "P1", "P2", "P3"]:
        priority = fm_priority
    elif any(k in all_text for k in ["#p0", "priority: p0", "priority:p0", "highest priority", "top priority", "most highest priority"]):
        priority = "P0"
    elif any(k in all_text for k in ["#p1", "priority: p1", "priority:p1"]):
        priority = "P1"
    elif any(k in all_text for k in ["#p3", "priority: p3", "priority:p3"]):
        priority = "P3"

    # Extract tags
    tags = list(set(re.findall(r"#([a-zA-Z0-9_\-/]+)", content)))

    # Extract dependencies / prerequisites
    dependencies = []
    dep_matches = re.findall(r"(?:prerequisite(?:s|\s+for)?|depends\s+on):\s*([^\n\r]+)", content, re.IGNORECASE)
    for dm in dep_matches:
        cleaned_dep = re.sub(r"[*_`\[\]]", "", dm).strip()
        if cleaned_dep and cleaned_dep not in dependencies:
            dependencies.append(cleaned_dep)

    # Determine default model tier heuristic
    title_lower = title.lower()
    if any(k in title_lower for k in ["dsp", "ladder", "filter", "polyphon", "thread", "concurren", "audio"]):
        tier = "tier1"
    elif any(k in title_lower for k in ["plugin", "lore", "doc", "format", "clean", "naming", "catalog"]):
        tier = "tier3"
    else:
        tier = "tier2"

    # Extract or infer ecosystem target pillars (Flight 6)
    pillars = []
    if fm_match:
        fm_pil = re.search(r"^pillars?:\s*\[?(.*?)\]?\s*$", fm_match.group(1), re.MULTILINE | re.IGNORECASE)
        if fm_pil:
            p_items = [p.strip().strip("'\"").upper() for p in fm_pil.group(1).split(",") if p.strip()]
            pillars = [p for p in p_items if p in PILLAR_METADATA]
    if not pillars:
        pillars = infer_pillars(title, tags, body)

    return {
        "id": file_path.stem,
        "file_name": file_path.name,
        "rel_path": str(file_path),
        "title": title,
        "date": date_str,
        "excerpt": excerpt,
        "body": body.strip(),
        "char_count": len(body),
        "word_count": len(body.split()),
        "tier": tier,
        "priority": priority,
        "tags": tags,
        "dependencies": dependencies,
        "target_pillars": pillars,
    }


def scan_inbox(inbox_dir: Path):
    """Scans directory for .md files and extracts metadata."""
    if not inbox_dir.exists():
        return []

    notes = []
    md_files = sorted(inbox_dir.glob("*.md"), key=lambda f: f.stat().st_mtime, reverse=True)
    for f in md_files:
        if f.is_file() and not f.name.startswith(".") and f.name.upper() not in ["README.MD", "INDEX.MD", "ARCHIVE_INDEX.MD"]:
            data = extract_note_metadata(f)
            if data:
                notes.append(data)
    return notes


def scan_all_inbox_notes(inbox_dir: Path) -> list:
    """
    Scans both root inbox and its Archive/ subfolder, combining and deduping notes by id.
    Live root inbox notes win on content over Archive notes.
    """
    if inbox_dir.name.lower() == "archive":
        archive_dir = inbox_dir
        live_dir = inbox_dir.parent
    else:
        live_dir = inbox_dir
        archive_dir = inbox_dir / "Archive"

    arch_notes = scan_inbox(archive_dir) if archive_dir and archive_dir.exists() else []
    live_notes = scan_inbox(live_dir) if live_dir and live_dir.exists() and live_dir.resolve() != archive_dir.resolve() else []

    notes_by_id = {}
    for n in arch_notes:
        if n.get("id"):
            notes_by_id[str(n["id"])] = n
    # Live notes overwrite on content
    for n in live_notes:
        if n.get("id"):
            notes_by_id[str(n["id"])] = n

    return list(notes_by_id.values())


def parse_backlog_flight_statuses(backlog_path: Path = DEFAULT_BACKLOG_PATH) -> dict:
    """
    Scans docs/BACKLOG.md for flight headings and roadmap entries to determine
    the triage status of each flight in Act III.
    Returns: { flight_idx (int): { 'status': 'completed' | 'backlog' | 'untriaged', 'badge': str, 'section': str, 'sec_num': str, 'title': str } }
    """
    if not backlog_path.exists():
        return {}
    try:
        content = backlog_path.read_text(encoding="utf-8")
    except Exception:
        return {}

    statuses = {}

    # Locate Act III runway (Section 15 onwards)
    act3_pos = content.find("### 15. N'kai Sidecar Architecture Roadmap")
    search_content = content[act3_pos:] if act3_pos != -1 else content

    # 1. Parse roadmap check-list items:
    # Example: - [x] **Flight 1: The "Rambles" Ingestion Engine (Topic 2, P0)** — ✅ COMPLETED...
    # Example: - [ ] **Flight 2: N'kai to High Reasoning Prompt Compiler (Topic 5, P1 — TOP ACTIVE PRIORITY)** — ⏳ QUEUED...
    road_matches = re.finditer(
        r'^-\s+\[(x| )\]\s+\*\*Flight\s+(\d+)[:.]\s*([^*]+)\*\*\s*—\s*([^\n]+)$',
        search_content,
        re.MULTILINE
    )
    for m in road_matches:
        checked, f_num, title, status_text = m.groups()
        f_idx = int(f_num)
        st_upper = status_text.upper()
        if "GRAVEYARD" in st_upper or "KILLED" in st_upper:
            statuses[f_idx] = {
                "status": "graveyard",
                "badge": "🪦 GRAVEYARD",
                "section": f"Strategy Graveyard (Flight {f_idx})",
                "sec_num": "",
                "title": title.strip()
            }
        elif checked.lower() == "x" or "COMPLETED" in st_upper or "SHIPPED" in st_upper:
            statuses[f_idx] = {
                "status": "completed",
                "badge": "✅ COMPLETED",
                "section": f"Flight {f_idx}: {title.strip()}",
                "sec_num": "",
                "title": title.strip()
            }
        else:
            statuses[f_idx] = {
                "status": "untriaged",
                "badge": "⚪ UNTRIAGED",
                "section": "",
                "sec_num": "",
                "title": title.strip()
            }

    # 2. Parse dedicated flight sections in Act III:
    # Example: ### 16. Flight 2: N'kai to High Reasoning Prompt Compiler (P1 — TOP ACTIVE PRIORITY)
    # Example: ### 17. Flight 3: N'kai Automode: 2-Pass Iterative Research Daemon & Multi-Drawer Dossier Cards (P1 — TOP PRIORITY)
    # Example: ### 18. Flight 4: General Research Assistant & Configurable Lens Profiles (P2)
    sec_matches = re.finditer(
        r'^###\s+(\d+)\.\s+Flight\s+(\d+)[:.]\s*([^\n]+)$',
        search_content,
        re.MULTILINE
    )
    for m in sec_matches:
        sec_num, f_num, sec_title = m.groups()
        f_idx = int(f_num)
        clean_title = sec_title.strip()
        # If not already completed, a dedicated section means it has been triaged to the BACKLOG!
        if statuses.get(f_idx, {}).get("status") != "completed":
            statuses[f_idx] = {
                "status": "backlog",
                "badge": "✅ BACKLOG",
                "section": f"Section {sec_num}: Flight {f_idx}",
                "sec_num": sec_num,
                "title": clean_title
            }

    return statuses


def parse_docket_file(docket_path: Path, backlog_path: Path = None):
    """Parses curated docket items from morning_mail_topics.md cross-referenced with docs/BACKLOG.md."""
    if not docket_path.exists():
        return []

    try:
        content = docket_path.read_text(encoding="utf-8")
    except Exception:
        return []

    backlog_statuses = {}
    target_backlog = backlog_path or DEFAULT_BACKLOG_PATH
    if target_backlog and target_backlog.exists():
        backlog_statuses = parse_backlog_flight_statuses(target_backlog)

    pattern = r"^#{3,4}\s*(?:[🚀⏳⚡🆕✅🪦❌💀]\s*)?(?:Flight\s+)?(\d+)[:.]\s*([^\n]+)$"
    matches = list(re.finditer(pattern, content, re.MULTILINE))

    items = []
    for i, m in enumerate(matches):
        full_heading = m.group(0)
        raw_title = m.group(2).strip()
        idx = int(m.group(1))

        # Clean title of completed/shipped/graveyard suffix if present
        clean_title = re.sub(r"\s*—\s*(?:SHIPPED|COMPLETED|APPROVED|GRAVEYARD|KILLED)[^—\n]*", "", raw_title, flags=re.IGNORECASE).strip()

        start_pos = m.end()
        end_pos = matches[i + 1].start() if i + 1 < len(matches) else len(content)
        # Stop at next major ## section if present
        next_sec = re.search(r"\n##\s+", content[start_pos:end_pos])
        if next_sec:
            end_pos = start_pos + next_sec.start()

        section_text = content[start_pos:end_pos].strip()

        # Extract source note
        sn_m = re.search(r"-\s*\*\*Source Note(?: / User Prompt)?:\*\*\s*(.+)", section_text)
        source_note = sn_m.group(1).strip() if sn_m else "Conversation & Vibe Coding"

        # Extract clean excerpt
        lines = [
            line.strip()
            for line in section_text.splitlines()
            if line.strip() and not line.strip().startswith("#") and not line.strip().startswith("---")
        ]
        clean_lines = [re.sub(r"[*_`]", "", l) for l in lines]
        excerpt = " ".join(clean_lines[:3])[:280] + ("..." if len(" ".join(clean_lines[:3])) > 280 else "")

        # Model tier classification (explicit metadata first, keyword fallback second)
        tier_m = re.search(r"\bTier:\s*(?:Tier\s*)?(\d+|pro|flash|minimal)", section_text, re.IGNORECASE)
        if tier_m:
            t_val = tier_m.group(1).lower()
            if t_val in ["1", "pro"]:
                tier = "tier1"
            elif t_val in ["3", "minimal"]:
                tier = "tier3"
            else:
                tier = "tier2"
        else:
            t_low = clean_title.lower()
            if any(k in t_low for k in ["ladder filter", "audio filter", "filter dsp", "dsp", "saturation", "differential", "polyphon", "audio thread"]):
                tier = "tier1"
            elif any(k in t_low for k in ["rebranding", "lore", "cheat sheet", "plugins for the knowledge", "use cases", "lens profiles", "pill", "counter"]):
                tier = "tier3"
            else:
                tier = "tier2"

        # Priority detection
        all_docket_text = (clean_title + " " + section_text).lower()
        if any(k in all_docket_text for k in ["#p0", "priority: p0", "priority:p0", "highest priority", "top priority", "most highest priority"]):
            priority = "P0"
        elif any(k in all_docket_text for k in ["#p1", "priority: p1", "priority:p1"]):
            priority = "P1"
        elif any(k in all_docket_text for k in ["#p3", "priority: p3", "priority:p3"]):
            priority = "P3"
        else:
            priority = "P2"

        # Dependencies
        dependencies = []
        dep_matches = re.findall(r"(?:prerequisite(?:s|\s+for)?|depends\s+on):\s*([^\n\r]+)", section_text, re.IGNORECASE)
        for dm in dep_matches:
            cleaned_dep = re.sub(r"[*_`\[\]]", "", dm).strip()
            if cleaned_dep and cleaned_dep not in dependencies:
                dependencies.append(cleaned_dep)

        tags = list(set(re.findall(r"#([a-zA-Z0-9_\-/]+)", section_text)))

        # Status resolution from docs/BACKLOG.md
        heading_completed = "✅" in full_heading or any(s in raw_title.lower() for s in ["completed", "shipped", "approved"])
        b_info = backlog_statuses.get(idx)
        if b_info:
            status = b_info["status"]
            badge_text = b_info["badge"]
            section_info = b_info.get("section", "")
            sec_num = b_info.get("sec_num", "")
        elif heading_completed:
            status = "completed"
            badge_text = "✅ COMPLETED"
            section_info = f"Flight {idx}"
            sec_num = ""
        else:
            status = "untriaged"
            badge_text = "⚪ UNTRIAGED"
            section_info = ""
            sec_num = ""

        target_pillars = infer_pillars(clean_title, tags, section_text)

        items.append({
            "id": f"docket-{idx}",
            "idx": idx,
            "title": clean_title,
            "source_note": source_note,
            "excerpt": excerpt,
            "body": section_text,
            "word_count": len(section_text.split()),
            "date": "2026-10-09",
            "tier": tier,
            "priority": priority,
            "tags": tags,
            "dependencies": dependencies,
            "target_pillars": target_pillars,
            "status": status,
            "badge_text": badge_text,
            "section_info": section_info,
            "sec_num": sec_num,
        })

    return items


def render_chat_summary(inbox_dir: Path, notes: list, docket_items: list = None):
    """Outputs a clean, terminal-friendly Markdown summary (--chat standard)."""
    lines = [
        f"# 📬 N'kai Morning Mail Summary",
        f"**Source:** `{inbox_dir}`",
        f"**Generated:** {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}",
        "",
    ]

    if docket_items:
        lines.append(f"## 🌟 Active Test-Drive Docket ({len(docket_items)} Topics)")
        for it in docket_items:
            lines.append(f"### #{it['idx']}. {it['title']}")
            lines.append(f"- **Source:** {it['source_note']} | **Tier:** {it['tier'].upper()} | **Words:** {it['word_count']}")
            lines.append(f"- **Excerpt:** > {it['excerpt']}")
            lines.append("- **Quick Actions:** `[📥 Backlog]` `[🔬 Research]` `[🪦 Graveyard]` `[⚖️ PCDA]`")
            lines.append("")

    if notes:
        lines.append(f"## 📦 Vault Archive ({len(notes)} Notes)")
        for idx, note in enumerate(notes, 1):
            lines.append(f"### {idx}. {note['title']}")
            lines.append(f"- **File:** `{note['file_name']}` | **Date:** {note['date']} | **Words:** {note['word_count']}")
            lines.append(f"- **Excerpt:** > {note['excerpt']}")
            lines.append("")

    if not docket_items and not notes:
        lines.append("_No unhandled inbox notes found. Mailbox is completely clean!_")

    return "\n".join(lines)


def build_mail_queue(
    notes: list,
    docket_items: list,
    queue_path: Path,
    rejigger_path: Optional[Any] = None,
    use_rejigger: bool = True
) -> MailQueue:
    """
    Constructs a validated MailQueue from raw notes and docket items, applying the
    structural rejigger overlay if present, enforcing reverse-chronological sorting
    for raw queues or rank ordering for overlay queues, snapshots the prior ledger,
    and serializes the session ledger to queue_path.
    """
    # 0. Resolve rejigger overlay path
    overlay_path = None
    if use_rejigger and rejigger_path is not False:
        if rejigger_path:
            p = Path(rejigger_path)
            if p.is_absolute():
                if p.exists():
                    overlay_path = p
            else:
                for base in [Path.cwd(), Path(__file__).resolve().parent.parent]:
                    cand = base / p
                    if cand.exists():
                        overlay_path = cand
                        break
        else:
            for base in [Path.cwd(), Path(__file__).resolve().parent.parent]:
                cand = base / DEFAULT_REJIGGER_PATH
                if cand.exists():
                    overlay_path = cand
                    break

    # Preserve existing triage status and verdicts from active ledger
    existing_by_id = {}
    if queue_path.exists():
        try:
            old_data = json.loads(queue_path.read_text(encoding="utf-8"))
            for old_it in old_data.get("items", []):
                if old_it.get("id"):
                    existing_by_id[str(old_it["id"])] = old_it
        except Exception:
            pass

    raw_items = []

    # 1. Ingest docket items
    for it in docket_items:
        it_id = str(it.get("id") or f"docket-{it.get('idx', 0)}")
        old_it = existing_by_id.get(it_id)
        it_status = it.get("status", "untriaged")
        m_status = TriageStatus.PENDING
        if it_status == "backlog":
            m_status = TriageStatus.BACKLOG
        elif it_status == "completed":
            m_status = TriageStatus.COMPLETED

        verdict = None
        verdict_notes = None
        if old_it:
            old_st = old_it.get("status")
            if old_st and old_st != "PENDING" and it_status == "untriaged":
                try:
                    m_status = TriageStatus(old_st)
                except ValueError:
                    pass
            verdict = old_it.get("verdict")
            verdict_notes = old_it.get("verdict_notes")

        raw_items.append({
            "id": it_id,
            "title": str(it.get("title", "Untitled Docket Topic")),
            "created_at": str(it.get("date") or "2026-10-09"),
            "date": str(it.get("date") or "2026-10-09"),
            "priority": it.get("priority", "P2"),
            "status": m_status,
            "tags": it.get("tags", []),
            "target_pillars": it.get("target_pillars", []),
            "source_file": str(it.get("source_note", "")),
            "source_note": str(it.get("source_note", "")),
            "preview": str(it.get("excerpt", "")),
            "excerpt": str(it.get("excerpt", "")),
            "dependencies": it.get("dependencies", []),
            "verdict": verdict,
            "verdict_notes": verdict_notes,
            "char_count": len(str(it.get("body", ""))),
            "word_count": int(it.get("word_count", 0)),
            "tier": str(it.get("tier", "tier2")),
            "body": str(it.get("body", "")),
            "file_name": "",
            "rel_path": str(it.get("source_note", "")),
            "idx": int(it.get("idx", 0)),
        })

    # Collect known docket source note names to avoid duplicate cards
    docket_source_names = set()
    for it in docket_items:
        sn = str(it.get("source_note", ""))
        m = re.search(r"\[`?([^`\]]+)`?\]", sn)
        if m:
            docket_source_names.add(m.group(1).lower().strip())
        docket_source_names.add(Path(sn).name.lower().strip())
        docket_source_names.add(str(it.get("title", "")).lower().strip())

    # 2. Ingest notes (skipping those already formalized as curated docket items)
    for n in notes:
        fn = str(n.get("file_name", "")).lower().strip()
        stem = Path(fn).stem.lower().strip()
        t = str(n.get("title", "")).lower().strip()
        if fn in docket_source_names or stem in docket_source_names or t in docket_source_names:
            continue

        item_id = str(n.get("id") or n.get("file_name", "note"))
        old_it = existing_by_id.get(item_id)
        note_status = TriageStatus.PENDING
        verdict = None
        verdict_notes = None
        if old_it:
            old_st = old_it.get("status")
            if old_st and old_st != "PENDING":
                try:
                    note_status = TriageStatus(old_st)
                except ValueError:
                    pass
            verdict = old_it.get("verdict")
            verdict_notes = old_it.get("verdict_notes")

        raw_items.append({
            "id": item_id,
            "title": str(n.get("title", "Untitled Note")),
            "created_at": str(n.get("date") or datetime.date.today().strftime("%Y-%m-%d")),
            "date": str(n.get("date") or datetime.date.today().strftime("%Y-%m-%d")),
            "priority": n.get("priority", "P2"),
            "status": note_status,
            "tags": n.get("tags", []),
            "target_pillars": n.get("target_pillars", []),
            "source_file": str(n.get("rel_path") or n.get("file_name", "")),
            "preview": str(n.get("excerpt", "")),
            "excerpt": str(n.get("excerpt", "")),
            "dependencies": n.get("dependencies", []),
            "verdict": verdict,
            "verdict_notes": verdict_notes,
            "char_count": int(n.get("char_count", 0)),
            "word_count": int(n.get("word_count", 0)),
            "tier": str(n.get("tier", "tier2")),
            "body": str(n.get("body", "")),
            "file_name": str(n.get("file_name", "")),
            "rel_path": str(n.get("rel_path", "")),
            "source_note": "",
            "idx": 0,
        })

    # 3. Check for rejigger overlay
    if overlay_path and overlay_path.exists():
        try:
            rej_data = json.loads(overlay_path.read_text(encoding="utf-8"))
            cards = rej_data.get("cards", [])
        except Exception as e:
            print(f"⚠️ Failed to parse rejigger overlay at {overlay_path}: {e}")
            cards = []

        if cards:
            raw_by_id = {str(it["id"]): it for it in raw_items if it.get("id")}
            raw_by_title = {str(it["title"]).lower().strip(): it for it in raw_items if it.get("title")}

            referenced_raw_ids = set()
            overlay_mail_items = []

            for card in cards:
                card_id = str(card.get("id"))
                card_title = str(card.get("title", ""))
                card_rank = int(card.get("rank", 0))
                card_summary = str(card.get("summary", ""))
                angles = card.get("angles", [])
                next_action = str(card.get("next_action", ""))

                matched_raw_items = []
                for src in card.get("sources", []):
                    src_str = str(src).strip()
                    matched = None
                    if src_str in raw_by_id:
                        matched = raw_by_id[src_str]
                    elif src_str.lower() in raw_by_title:
                        matched = raw_by_title[src_str.lower()]
                    if matched:
                        matched_raw_items.append(matched)
                        referenced_raw_ids.add(str(matched["id"]))

                raw_match = raw_by_id.get(card_id)
                if card_title == card_id and raw_match:
                    card_title = str(raw_match.get("title") or card_title)

                is_placeholder_summary = card_summary.strip().lower() in ["completed.", "in backlog.", "graveyard."]

                if is_placeholder_summary and raw_match:
                    body = str(raw_match.get("body", ""))
                    preview = str(raw_match.get("excerpt") or raw_match.get("preview") or card_summary)
                    excerpt = preview
                    word_count = int(raw_match.get("word_count") or len(body.split()))
                    char_count = len(body)
                    source_file = str(raw_match.get("source_file") or raw_match.get("rel_path") or "")
                    file_name = str(raw_match.get("file_name", ""))
                    rel_path = str(raw_match.get("rel_path", ""))
                    source_note = str(raw_match.get("source_note", ""))
                    created_at = str(raw_match.get("date") or raw_match.get("created_at") or "2026-10-09")
                else:
                    body_parts = [card_summary]
                    if angles:
                        body_parts.append("\n".join(f"- {a}" for a in angles))
                    if next_action:
                        body_parts.append(f"**Next action:** {next_action}")
                    if matched_raw_items:
                        source_lines = ["**Source notes:**"]
                        for r in matched_raw_items:
                            r_title = str(r.get("title") or r.get("id") or "Note")
                            r_path = str(r.get("rel_path") or r.get("source_file") or "")
                            m_link = re.search(r'\((file:///[^)]+)\)', r_path)
                            if m_link:
                                file_url = m_link.group(1)
                            elif r_path:
                                file_url = "file:///" + r_path.replace("\\", "/")
                            else:
                                file_url = ""
                            if file_url:
                                source_lines.append(f"- [{r_title}]({file_url})")
                            else:
                                source_lines.append(f"- {r_title}")
                        body_parts.append("\n".join(source_lines))
                    body = "\n\n".join(p for p in body_parts if p.strip())
                    preview = card_summary
                    excerpt = card_summary
                    word_count = len(body.split())
                    char_count = len(body)
                    source_file = str(matched_raw_items[0].get("source_file") or matched_raw_items[0].get("rel_path") or "") if matched_raw_items else ""
                    file_name = str(matched_raw_items[0].get("file_name", "")) if matched_raw_items else ""
                    rel_path = str(matched_raw_items[0].get("rel_path", "")) if matched_raw_items else ""
                    source_note = str(matched_raw_items[0].get("source_note", "")) if matched_raw_items else ""
                    created_at = str(matched_raw_items[0].get("date") or matched_raw_items[0].get("created_at") or "2026-10-11") if matched_raw_items else "2026-10-11"

                # Status resolution: existing ledger wins on non-PENDING, else card status
                old_it = existing_by_id.get(card_id)
                card_status = card.get("status", "PENDING")
                final_status = card_status
                verdict = None
                verdict_notes = None
                if old_it:
                    old_st = old_it.get("status")
                    if old_st and old_st != "PENDING":
                        final_status = old_st
                    verdict = old_it.get("verdict")
                    verdict_notes = old_it.get("verdict_notes")

                try:
                    status_enum = TriageStatus(final_status)
                except ValueError:
                    status_enum = TriageStatus.PENDING

                merged_count = len(matched_raw_items)
                source_ids = [str(r.get("id")) for r in matched_raw_items if r.get("id")]
                tags = list(card.get("tags", []))
                if merged_count > 1 and f"merged:{merged_count}" not in tags:
                    tags.append(f"merged:{merged_count}")

                m_item = MailItem(
                    id=card_id,
                    title=card_title,
                    created_at=created_at,
                    priority=card.get("priority", "P2"),
                    status=status_enum,
                    tags=tags,
                    target_pillars=card.get("pillars", []),
                    source_file=source_file,
                    preview=preview,
                    dependencies=card.get("depends_on", []),
                    verdict=verdict,
                    verdict_notes=verdict_notes,
                    char_count=char_count,
                    word_count=word_count,
                    tier=card.get("tier", "tier2"),
                    body=body,
                    excerpt=excerpt,
                    file_name=file_name,
                    rel_path=rel_path,
                    source_note=source_note,
                    idx=card_rank,
                    merged_count=merged_count,
                    source_ids=source_ids,
                )
                overlay_mail_items.append(m_item)

            # Sort overlay cards by rank ascending
            overlay_mail_items.sort(key=lambda x: int(x.idx))

            # Collect unreferenced raw items (new mail)
            unreferenced_raw_items = []
            for r in raw_items:
                r_id = str(r.get("id"))
                if r_id not in referenced_raw_ids:
                    r_title = str(r.get("title", "Untitled Note"))
                    if not r_title.startswith("🆕 "):
                        r_title = f"🆕 {r_title}"

                    old_it = existing_by_id.get(r_id)
                    r_status = r.get("status", TriageStatus.PENDING)
                    verdict = None
                    verdict_notes = None
                    if old_it:
                        old_st = old_it.get("status")
                        if old_st and old_st != "PENDING":
                            try:
                                r_status = TriageStatus(old_st)
                            except ValueError:
                                pass
                        verdict = old_it.get("verdict")
                        verdict_notes = old_it.get("verdict_notes")

                    unref_item = MailItem(
                        id=r_id,
                        title=r_title,
                        created_at=str(r.get("date") or r.get("created_at") or datetime.date.today().strftime("%Y-%m-%d")),
                        priority=r.get("priority", "P2"),
                        status=r_status,
                        tags=r.get("tags", []),
                        target_pillars=r.get("target_pillars", []),
                        source_file=str(r.get("source_file") or r.get("rel_path") or ""),
                        preview=str(r.get("preview") or r.get("excerpt") or ""),
                        dependencies=r.get("dependencies", []),
                        verdict=verdict,
                        verdict_notes=verdict_notes,
                        char_count=int(r.get("char_count", 0)),
                        word_count=int(r.get("word_count", 0)),
                        tier=str(r.get("tier", "tier2")),
                        body=str(r.get("body", "")),
                        excerpt=str(r.get("excerpt", "")),
                        file_name=str(r.get("file_name", "")),
                        rel_path=str(r.get("rel_path", "")),
                        source_note=str(r.get("source_note", "")),
                        idx=0,
                        merged_count=1,
                        source_ids=[r_id],
                    )
                    unreferenced_raw_items.append(unref_item)

            # Order unreferenced new items by priority/date
            pri_rank = {PriorityLevel.P0: 0, PriorityLevel.P1: 1, PriorityLevel.P2: 2, PriorityLevel.P3: 3}
            unreferenced_raw_items.sort(key=lambda x: str(x.created_at or ""), reverse=True)
            unreferenced_raw_items.sort(key=lambda x: pri_rank.get(x.priority, 2))

            final_items = overlay_mail_items + unreferenced_raw_items
            queue = MailQueue(
                schema_version="1.0.0",
                last_scanned=datetime.datetime.now().isoformat(),
                total_items=len(final_items),
                p0_count=sum(1 for it in final_items if it.priority == PriorityLevel.P0),
                items=final_items,
            )

            # Snapshot existing ledger before overwriting
            if queue_path.exists():
                snap_dir = queue_path.parent / "snapshots"
                snap_dir.mkdir(parents=True, exist_ok=True)
                prev_snap = snap_dir / "mail_queue_prev.json"
                try:
                    shutil.copy2(str(queue_path), str(prev_snap))
                except Exception as e:
                    print(f"⚠️ Could not create snapshot: {e}")

            queue_path.parent.mkdir(parents=True, exist_ok=True)
            queue_path.write_text(queue.model_dump_json(indent=2), encoding="utf-8")
            return queue

    # 4. Standard raw queue if no overlay
    std_items = []
    for r in raw_items:
        std_items.append(MailItem(
            id=str(r["id"]),
            title=str(r["title"]),
            created_at=str(r.get("created_at") or r.get("date") or "2026-10-11"),
            priority=r.get("priority", "P2"),
            status=r.get("status", TriageStatus.PENDING),
            tags=r.get("tags", []),
            target_pillars=r.get("target_pillars", []),
            source_file=str(r.get("source_file") or r.get("rel_path") or ""),
            preview=str(r.get("preview") or r.get("excerpt") or ""),
            dependencies=r.get("dependencies", []),
            verdict=r.get("verdict"),
            verdict_notes=r.get("verdict_notes"),
            char_count=int(r.get("char_count", 0)),
            word_count=int(r.get("word_count", 0)),
            tier=str(r.get("tier", "tier2")),
            body=str(r.get("body", "")),
            excerpt=str(r.get("excerpt", "")),
            file_name=str(r.get("file_name", "")),
            rel_path=str(r.get("rel_path", "")),
            source_note=str(r.get("source_note", "")),
            idx=int(r.get("idx", 0)),
        ))

    queue = MailQueue(
        schema_version="1.0.0",
        last_scanned=datetime.datetime.now().isoformat(),
        items=std_items,
    )
    # Enforces reverse-chronological sorting for P0 items at top
    queue.sort_by_priority()

    # Snapshot existing ledger before overwriting
    if queue_path.exists():
        snap_dir = queue_path.parent / "snapshots"
        snap_dir.mkdir(parents=True, exist_ok=True)
        prev_snap = snap_dir / "mail_queue_prev.json"
        try:
            shutil.copy2(str(queue_path), str(prev_snap))
        except Exception as e:
            print(f"⚠️ Could not create snapshot: {e}")

    # Ensure output directory exists and write ledger
    queue_path.parent.mkdir(parents=True, exist_ok=True)
    queue_path.write_text(queue.model_dump_json(indent=2), encoding="utf-8")
    return queue




def update_archive_index(archive_dir: Path):
    """
    Generates a master markdown index (INDEX.md / README.md) of all notes in Archive/
    with clickable links, dates, priorities, and ecosystem pillars for rapid O(1) lookup.
    """
    md_files = sorted(archive_dir.glob("*.md"), key=lambda f: f.stat().st_mtime, reverse=True)
    real_files = [f for f in md_files if f.name.upper() not in ["README.MD", "INDEX.MD", "ARCHIVE_INDEX.MD"]]
    lines = [
        "# 🗄️ Obsidian Inbox Archive Index",
        "> Automatically maintained by N'kai Harvester (`tools/check_mail.py`).",
        f"> **Total Archived Notes:** {len(real_files)} | **Last Updated:** {datetime.datetime.now().strftime('%Y-%m-%d %H:%M')}",
        "",
        "| Note Title | Captured Date | Priority | Pillar | File Link |",
        "| :--- | :--- | :--- | :--- | :--- |",
    ]
    for f in real_files:
        meta = extract_note_metadata(f)
        if meta:
            title = meta.get("title") or f.stem
            date_str = meta.get("date") or ""
            pri = meta.get("priority") or "P2"
            pillars = meta.get("target_pillars") or ["TKS"]
            pillar_badges = []
            for p in pillars[:2]:
                m = PILLAR_METADATA.get(p.upper(), {})
                emoji = m.get("emoji", "🏷️")
                pillar_badges.append(f"{emoji} {p.upper()}")
            if len(pillars) > 2:
                pillar_badges.append(f"+{len(pillars) - 2}")
            pillar_str = " ".join(pillar_badges)
            lines.append(f"| **{title}** | `{date_str}` | `{pri}` | {pillar_str} | [{f.name}]({f.name}) |")

    content = "\n".join(lines) + "\n"
    (archive_dir / "INDEX.md").write_text(content, encoding="utf-8")
    (archive_dir / "README.md").write_text(content, encoding="utf-8")


def archive_harvested_notes(inbox_dir: Path, notes: list, receipt_path: Path, queue: MailQueue, queue_path: Path):
    """
    Moves all harvested markdown notes from the root inbox into inbox_dir/Archive,
    ensuring clean YAML frontmatter, updating the MailQueue ledger, writing an atomic
    TransactionReceipt for 5ms rollback, and compiling the master archive index.
    """
    archive_dir = inbox_dir / "Archive" if inbox_dir.name.lower() != "archive" else inbox_dir
    archive_dir.mkdir(parents=True, exist_ok=True)
    live_dir = inbox_dir if inbox_dir.name.lower() != "archive" else inbox_dir.parent

    moved_files = []
    for note in notes:
        rel_p = note.get("rel_path")
        if not rel_p:
            continue
        src = Path(rel_p).resolve()
        # Only archive files that are directly in live_dir (not already in Archive)
        if src.parent.resolve() == live_dir.resolve() and src.parent.resolve() != archive_dir.resolve():
            dst = archive_dir / src.name
            try:
                # 1. Inject clean YAML frontmatter if missing
                text_content = src.read_text(encoding="utf-8", errors="replace")

                if not re.match(r"^---\s*\n.*?\n---\s*\n", text_content, re.DOTALL):
                    clean_title = note.get("title") or src.stem
                    note_date = note.get("date") or datetime.date.today().strftime("%Y-%m-%d")
                    note_pri = note.get("priority") or "P2"
                    note_pillars = note.get("target_pillars") or infer_pillars(clean_title, note.get("tags", []), text_content)
                    pil_json = json.dumps(note_pillars)
                    frontmatter = f"---\ntitle: \"{clean_title}\"\ndate: \"{note_date}\"\npriority: \"{note_pri}\"\ntags: [\"inbox\", \"raw-ramble\"]\npillars: {pil_json}\nstatus: \"untriaged\"\n---\n\n"
                    src.write_text(frontmatter + text_content, encoding="utf-8")

                shutil.move(str(src), str(dst))
                moved_files.append({"src": str(src), "dst": str(dst), "name": src.name})
                note["rel_path"] = str(dst)
                note["source_file"] = str(dst)
                # Update item in queue as well
                for it in queue.items:
                    if it.file_name == src.name or it.id == src.stem or it.title == note.get("title"):
                        it.source_file = str(dst)
                        it.rel_path = str(dst)
            except Exception as e:
                print(f"⚠️ Failed to move {src.name} to archive: {e}")

    if moved_files:
        # Snapshot prior ledger before save
        if queue_path.exists():
            snap_dir = queue_path.parent / "snapshots"
            snap_dir.mkdir(parents=True, exist_ok=True)
            try:
                shutil.copy2(str(queue_path), str(snap_dir / "mail_queue_prev.json"))
            except Exception:
                pass

        # Save updated queue
        queue_path.write_text(queue.model_dump_json(indent=2), encoding="utf-8")

        # Compile master archive index
        try:
            update_archive_index(archive_dir)
        except Exception as e:
            print(f"⚠️ Failed to update archive index: {e}")

        # Write atomic rollback receipt
        receipt_data = {
            "tx_id": f"mail-arch-{int(datetime.datetime.now().timestamp())}",
            "timestamp": datetime.datetime.now().isoformat(),
            "action": "archive_harvested",
            "note_id": f"{len(moved_files)} files archived",
            "prev_status": "PENDING",
            "new_status": "ARCHIVED",
            "moved_files": moved_files,
            "can_rollback": True,
        }
        receipt_path.parent.mkdir(parents=True, exist_ok=True)
        receipt_path.write_text(json.dumps(receipt_data, indent=2), encoding="utf-8")
        print(f"📦 Archived {len(moved_files)} notes to {archive_dir} (Receipt: {receipt_path})")

    return moved_files


def rollback_mail_tx(queue_path: Path, receipt_path: Path) -> bool:
    """
    Reverts the last triage action on a note or restores archived notes using atomic transaction receipt.
    """
    if not receipt_path.exists():
        print(f"⚠️ No transaction receipt found at {receipt_path}")
        return False

    try:
        raw_receipt = json.loads(receipt_path.read_text(encoding="utf-8"))
    except Exception as e:
        print(f"❌ Failed to parse transaction receipt: {e}")
        return False

    # Check for archive_harvested action
    if raw_receipt.get("action") == "archive_harvested" and "moved_files" in raw_receipt:
        restored = 0
        for item in raw_receipt["moved_files"]:
            src = Path(item["src"])
            dst = Path(item["dst"])
            if dst.exists():
                try:
                    shutil.move(str(dst), str(src))
                    restored += 1
                except Exception as e:
                    print(f"⚠️ Could not restore {dst.name}: {e}")
        print(f"⚡ Restored {restored} notes from Archive back to root Inbox in 5ms!")
        return True

    try:
        receipt = TransactionReceipt.model_validate(raw_receipt)
    except Exception as e:
        print(f"❌ Failed to parse transaction receipt model: {e}")
        return False

    if not queue_path.exists():
        print(f"⚠️ Mail queue ledger not found at {queue_path}")
        return False

    try:
        queue = MailQueue.model_validate_json(queue_path.read_text(encoding="utf-8"))
    except Exception as e:
        print(f"❌ Failed to parse mail queue ledger: {e}")
        return False

    target = None
    for item in queue.items:
        if item.id == receipt.note_id or item.title == receipt.note_id:
            target = item
            break

    if not target:
        print(f"⚠️ Note '{receipt.note_id}' not found in mail queue ledger.")
        return False

    target.status = receipt.prev_status
    target.verdict = None
    target.verdict_notes = None

    queue_path.write_text(queue.model_dump_json(indent=2), encoding="utf-8")
    print(f"⚡ Rolled back mailbag transaction {receipt.tx_id} for note '{receipt.note_id}' (status restored to {receipt.prev_status.value}) in 5ms!")
    return True


def render_pillar_badges(pillars: list) -> str:
    """
    Renders up to 2 pillar badges with dedicated styling, plus a '+N' overflow chip
    if there are 3 or more pillars (The Rule of Two).
    Pillars are interactive buttons with onclick="applyPillarFilter('{p}', event)".
    """
    if not pillars:
        pillars = ["TKS"]

    badge_htmls = []
    visible = pillars[:2]
    overflow = pillars[2:]

    for p in visible:
        p_upper = p.upper()
        meta = PILLAR_METADATA.get(p_upper, {
            "name": p_upper, "emoji": "🏷️", "desc": p_upper, "color": "#94a3b8"
        })
        emoji = meta["emoji"]
        cls_name = f"pillar-{p_upper.lower().replace('-', '')}"
        badge_htmls.append(
            f'<span class="pillar-chip {cls_name}" title="Filter by {html.escape(meta["name"])}: {html.escape(meta["desc"])}" onclick="applyPillarFilter(\'{p_upper}\', event)">{emoji} {p_upper}</span>'
        )

    if overflow:
        overflow_titles = ", ".join([PILLAR_METADATA.get(op.upper(), {}).get("name", op) for op in overflow])
        first_overflow = overflow[0].upper()
        badge_htmls.append(
            f'<span class="pillar-chip pillar-overflow" title="Also: {html.escape(overflow_titles)} (Click to filter by {first_overflow})" onclick="applyPillarFilter(\'{first_overflow}\', event)">+{len(overflow)}</span>'
        )

    return "".join(badge_htmls)


def render_markdown_to_html(md_text: str) -> str:
    """Zero-dependency Markdown to styled HTML renderer for note dossiers."""
    if not md_text or not md_text.strip():
        return "<p><em>No content available.</em></p>"

    lines = md_text.strip().splitlines()
    html_out = []
    in_code_block = False
    code_buffer = []
    in_list = False

    def format_inline(text: str) -> str:
        escaped = html.escape(text, quote=True)
        # Links: [label](url)
        escaped = re.sub(
            r'\[([^\]]+)\]\(([^)]+)\)',
            r'<a href="\2" target="_blank" rel="noopener noreferrer">\1</a>',
            escaped
        )
        # Inline code: `code`
        escaped = re.sub(r'`([^`]+)`', r'<code>\1</code>', escaped)
        # Bold: **bold**
        escaped = re.sub(r'\*\*([^*]+)\*\*', r'<strong>\1</strong>', escaped)
        # Italic: *italic*
        escaped = re.sub(r'\*([^*]+)\*', r'<em>\1</em>', escaped)
        return escaped

    for line in lines:
        stripped = line.strip()

        # Handle fenced code blocks
        if stripped.startswith("```"):
            if in_code_block:
                code_content = html.escape("\n".join(code_buffer))
                html_out.append(f"<pre><code>{code_content}</code></pre>")
                code_buffer = []
                in_code_block = False
            else:
                if in_list:
                    html_out.append("</ul>")
                    in_list = False
                in_code_block = True
            continue

        if in_code_block:
            code_buffer.append(line)
            continue

        # Horizontal rule
        if stripped in ["---", "***", "___"]:
            if in_list:
                html_out.append("</ul>")
                in_list = False
            html_out.append("<hr />")
            continue

        # Blank line
        if not stripped:
            if in_list:
                html_out.append("</ul>")
                in_list = False
            continue

        # Headings (##, ###, ####, #####)
        heading_match = re.match(r'^(#{1,6})\s+(.*)', stripped)
        if heading_match:
            if in_list:
                html_out.append("</ul>")
                in_list = False
            level = len(heading_match.group(1))
            heading_text = format_inline(heading_match.group(2))
            h_tag = f"h{min(level + 2, 6)}"
            html_out.append(f"<{h_tag}>{heading_text}</{h_tag}>")
            continue

        # Blockquote
        if stripped.startswith(">"):
            if in_list:
                html_out.append("</ul>")
                in_list = False
            bq_text = format_inline(stripped.lstrip("> ").strip())
            html_out.append(f"<blockquote>{bq_text}</blockquote>")
            continue

        # Bullet list item (- , * )
        list_match = re.match(r'^[-*]\s+(.*)', stripped)
        if list_match:
            if not in_list:
                html_out.append("<ul>")
                in_list = True
            item_html = format_inline(list_match.group(1))
            html_out.append(f"  <li>{item_html}</li>")
            continue

        # Numbered list item (1. , 2. )
        num_list_match = re.match(r'^\d+\.\s+(.*)', stripped)
        if num_list_match:
            if not in_list:
                html_out.append("<ul>")
                in_list = True
            item_html = format_inline(num_list_match.group(1))
            html_out.append(f"  <li>{item_html}</li>")
            continue

        # Normal paragraph
        if in_list:
            html_out.append("</ul>")
            in_list = False
        p_html = format_inline(stripped)
        html_out.append(f"<p>{p_html}</p>")

    if in_code_block:
        code_content = html.escape("\n".join(code_buffer))
        html_out.append(f"<pre><code>{code_content}</code></pre>")

    if in_list:
        html_out.append("</ul>")

    return "\n".join(html_out)


def generate_html_inbox_board(inbox_dir: Path, notes: list, docket_items: list, out_path: Path, queue: MailQueue = None, environment: str = "PRODUCTION"):
    """
    Generates a standalone, unified interactive N'kai triage board with Two-Tier Header,
    Session-Stable Pillar Filters, and Live Consolidated Status Counter Pills (Flights 6 & 13).
    Supports SANDBOX and PRODUCTION runtime environments (Flight 16+17).
    Refactored in Flight 15 (rj-04) to use declarative BoardSchema and board_renderer.
    """
    if queue is None:
        queue = build_mail_queue(notes, docket_items, DEFAULT_MAIL_QUEUE)

    board_title = "N'kai Morning Mail Inbox & Triage Lab"
    board_id = "sandbox_mail" if environment.upper() == "SANDBOX" else "morning_mail"

    schema = BoardSchema(
        schema_version="1.0.0",
        board_id=board_id,
        title=board_title,
        subtitle=f"Scanned: {inbox_dir}",
        environment=environment.upper(),
        cards=queue.items,
        allowance_budget=200000,
    )

    return render_board(schema, out_path)



def main():
    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    except Exception:
        pass

    parser = argparse.ArgumentParser(
        description="N'kai 'Check the Mail' Vault Inbox Harvester & Interactive Triage Lab (Topic 6.11)"
    )
    parser.add_argument(
        "--dir",
        type=str,
        default=None,
        help="Custom inbox directory to scan (default: C:\\Dev\\TheKlangVault\\Inbox or docs/inbox)",
    )
    parser.add_argument(
        "--docket",
        type=str,
        default=None,
        help="Custom path to morning_mail_topics.md or curated docket file",
    )
    parser.add_argument(
        "--archive",
        action="store_true",
        help="Scans Archive subfolder instead of root inbox",
    )
    parser.add_argument(
        "--chat",
        "--text-only",
        dest="chat",
        action="store_true",
        help="Outputs clean Markdown summary to terminal without opening browser",
    )
    parser.add_argument(
        "--sandbox",
        action="store_true",
        help="Runs in isolated QA Sandbox mode using synthetic fixtures and zero persistence",
    )
    parser.add_argument(
        "--fixture",
        type=str,
        default=str(DEFAULT_SANDBOX_FIXTURE),
        help="Custom fixture path for sandbox mode (default: assets/fixtures/sandbox_mail_queue.json)",
    )
    parser.add_argument(
        "--out",
        type=str,
        default=None,
        help="Custom destination for generated HTML triage board",
    )
    parser.add_argument(
        "--no-browser",
        action="store_true",
        help="Do not auto-launch browser when generating HTML board",
    )
    parser.add_argument(
        "--queue",
        type=str,
        default=str(DEFAULT_MAIL_QUEUE),
        help="Custom destination for persistent MailQueue JSON ledger",
    )
    parser.add_argument(
        "--receipt",
        type=str,
        default=str(DEFAULT_TX_RECEIPT),
        help="Path to atomic transaction receipt for rollback",
    )
    parser.add_argument(
        "--rollback",
        action="store_true",
        help="Roll back the last recorded mail triage transaction in 5ms",
    )
    parser.add_argument(
        "--archive-inbox",
        "--archive-harvested",
        dest="archive_inbox",
        action="store_true",
        help="Physically moves harvested root inbox notes to Archive/ subfolder and emits atomic rollback receipt",
    )
    parser.add_argument(
        "--rejigger",
        type=str,
        default=str(DEFAULT_REJIGGER_PATH),
        help="Custom path to structural rejigger overlay JSON (default: .agents/pipeline/mailbag/rejigger.json)",
    )
    parser.add_argument(
        "--no-rejigger",
        action="store_true",
        help="Disable rejigger overlay and build raw mail queue directly",
    )

    args = parser.parse_args()

    # Handle rollback command
    if args.rollback:
        success = rollback_mail_tx(Path(args.queue), Path(args.receipt))
        return 0 if success else 1

    # Isolated QA Sandbox Mode (Flight 16+17)
    if args.sandbox:
        fixture_path = None
        cand_p = Path(args.fixture) if args.fixture else DEFAULT_SANDBOX_FIXTURE
        for base in [Path.cwd(), Path(__file__).resolve().parent.parent]:
            test_p = base / cand_p
            if test_p.exists():
                fixture_path = test_p
                break
        if not fixture_path and cand_p.exists():
            fixture_path = cand_p.resolve()
        if not fixture_path or not fixture_path.exists():
            print(f"❌ Sandbox fixture not found: {cand_p}")
            return 1

        queue = MailQueue.model_validate_json(fixture_path.read_text(encoding="utf-8"))
        print(f"🧪 [SANDBOX] Loaded Synthetic Fixture: {fixture_path} ({queue.total_items} items, {queue.p0_count} P0 Top Priority)")

        out_file = Path(args.out).resolve() if args.out else DEFAULT_SANDBOX_OUTPUT_HTML.resolve()

        if args.chat:
            print(f"🧪 [SANDBOX MODE] {queue.total_items} mock items loaded from fixture.")
            for item in queue.items:
                print(f"- [{item.priority.value}] {item.id}: {item.title} ({item.status.value})")
            return 0

        generated = generate_html_inbox_board(
            inbox_dir=Path("assets/fixtures"),
            notes=[],
            docket_items=[],
            out_path=out_file,
            queue=queue,
            environment="SANDBOX",
        )
        print(f"✅ Generated Sandbox Mail Inbox: {generated} ({queue.total_items} synthetic cards)")

        if not args.no_browser:
            try:
                webbrowser.open(generated.as_uri())
            except Exception as e:
                print(f"⚠️ Could not launch browser: {e}")

        return 0

    inbox_dir = resolve_inbox_directory(args.dir, scan_archive=args.archive)
    notes = scan_all_inbox_notes(inbox_dir)

    # Locate curated docket
    docket_path = None
    if args.docket:
        docket_path = Path(args.docket).resolve()
    elif DEFAULT_DOCKET_PATH.exists():
        docket_path = DEFAULT_DOCKET_PATH
    else:
        # Check artifact folder
        alt_docket = Path(r"C:\Users\codyg\.gemini\antigravity\brain\9241988e-1227-466e-a0a5-446d61a69d77\morning_mail_topics.md")
        if alt_docket.exists():
            docket_path = alt_docket

    docket_items = parse_docket_file(docket_path) if docket_path else []

    # Resolve rejigger overlay path
    rejigger_path = None
    if not args.no_rejigger:
        cand_p = Path(args.rejigger)
        if cand_p.is_absolute():
            if cand_p.exists():
                rejigger_path = cand_p
        else:
            for base in [Path.cwd(), Path(__file__).resolve().parent.parent]:
                test_p = base / cand_p
                if test_p.exists():
                    rejigger_path = test_p
                    break

    # Build and serialize validated MailQueue ledger (Flight 5 Pydantic engine)
    queue_file = Path(args.queue).resolve()
    queue = build_mail_queue(notes, docket_items, queue_file, rejigger_path=rejigger_path, use_rejigger=(not args.no_rejigger))
    print(f"📦 Validated MailQueue Ledger: {queue_file} ({queue.total_items} items, {queue.p0_count} P0 Top Priority)")

    # Execute physical archival if requested and not already scanning Archive
    if args.archive_inbox and not args.archive:
        archive_harvested_notes(inbox_dir, notes, Path(args.receipt), queue, queue_file)

    # Always maintain lean, zero-token Archive/INDEX.md if Archive directory exists (Option A)
    target_archive = inbox_dir if "archive" in inbox_dir.name.lower() else (inbox_dir / "Archive")
    if target_archive.exists():
        try:
            update_archive_index(target_archive)
        except Exception as e:
            pass

    if args.chat:
        summary_md = render_chat_summary(inbox_dir, notes, docket_items)
        sys.stdout.reconfigure(encoding="utf-8")
        print(summary_md)
        print(f"\n📊 **Queue Ledger**: `{queue_file}` ({queue.total_items} items, {queue.p0_count} P0 items)")
        return 0

    out_file = Path(args.out).resolve() if args.out else Path(DEFAULT_OUTPUT_HTML).resolve()
    generated = generate_html_inbox_board(inbox_dir, notes, docket_items, out_file, queue=queue, environment="PRODUCTION")
    print(f"✅ Generated Morning Mail Inbox: {generated} ({len(docket_items)} docket items, {len(notes)} archive notes)")

    if not args.no_browser:
        try:
            webbrowser.open(generated.as_uri())
        except Exception as e:
            print(f"⚠️ Could not launch browser: {e}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
