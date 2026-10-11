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

# Import Pydantic models for persistent Mailbag session (Flight 5)
try:
    from tools.models import MailItem, MailQueue, PriorityLevel, TriageStatus, TransactionReceipt, EcosystemPillar, PILLAR_METADATA, infer_pillars
except ImportError:
    try:
        from models import MailItem, MailQueue, PriorityLevel, TriageStatus, TransactionReceipt, EcosystemPillar, PILLAR_METADATA, infer_pillars
    except ImportError:
        _script_dir = Path(__file__).resolve().parent
        if str(_script_dir) not in sys.path:
            sys.path.insert(0, str(_script_dir))
        from models import MailItem, MailQueue, PriorityLevel, TriageStatus, TransactionReceipt, EcosystemPillar, PILLAR_METADATA, infer_pillars

DEFAULT_VAULT_INBOX = Path(r"C:\Dev\TheKlangVault\Inbox")
FALLBACK_LOCAL_INBOX = Path("docs/inbox")
DEFAULT_OUTPUT_HTML = Path(r".agents/sidecar/morning_mail_inbox.html")
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


def generate_html_inbox_board(inbox_dir: Path, notes: list, docket_items: list, out_path: Path, queue: MailQueue = None):
    """
    Generates a standalone, unified interactive N'kai triage board with Two-Tier Header,
    Session-Stable Pillar Filters, and Live Consolidated Status Counter Pills (Flights 6 & 13).
    """
    if queue is None:
        queue = build_mail_queue(notes, docket_items, DEFAULT_MAIL_QUEUE)

    items = queue.items
    total_count = len(items)

    # 1. Tally Ecosystem Pillars across all items (Session-Stable Frequency Ranking)
    pillar_counts = {}
    for it in items:
        pils = it.target_pillars or ["TKS"]
        for p in pils:
            p_upper = p.upper()
            pillar_counts[p_upper] = pillar_counts.get(p_upper, 0) + 1

    sorted_pillars = sorted(pillar_counts.keys(), key=lambda p: (-pillar_counts[p], p))

    # Build Rail 1 Pillar buttons HTML
    pillar_buttons = []
    for p in sorted_pillars:
        meta = PILLAR_METADATA.get(p, {"name": p, "emoji": "🏷️", "desc": p})
        emoji = meta.get("emoji", "🏷️")
        p_slug = p.lower().replace("-", "")
        cnt = pillar_counts[p]
        p_title = f"{meta['name']}: {meta.get('desc', '')}"
        pillar_buttons.append(
            f'<button class="pill-btn" id="pillar-btn-{p_slug}" onclick="applyPillarFilter(\'{p}\', event)" title="{html.escape(p_title)}">'
            f'<span class="pill-icon">{emoji}</span> {p} <span class="pill-count" id="count-pillar-{p_slug}">{cnt}</span>'
            f'</button>'
        )
    pillar_buttons_html = "\n        ".join(pillar_buttons)

    # 2. Tally Status / Triage categories across all items
    untriaged_count = sum(1 for it in items if it.status == TriageStatus.PENDING)
    backlog_count = sum(1 for it in items if it.status == TriageStatus.BACKLOG)
    completed_count = sum(1 for it in items if it.status == TriageStatus.COMPLETED)
    research_count = sum(1 for it in items if it.status == TriageStatus.RESEARCH)
    graveyard_count = sum(1 for it in items if it.status == TriageStatus.GRAVEYARD)
    pcda_count = sum(1 for it in items if it.status == TriageStatus.PCDA)
    p0_count = sum(1 for it in items if it.priority == PriorityLevel.P0)
    docket_count = sum(1 for it in items if it.id.startswith("docket-"))
    dispatched_count = backlog_count + completed_count + research_count + graveyard_count + pcda_count

    # 3. Render all cards into a unified stream
    all_cards_html = []
    for i, it in enumerate(items, 1):
        idx = it.idx
        rank_val = idx if idx > 0 else i
        title_esc = html.escape(it.title)
        source_esc = html.escape(it.source_note or it.source_file or it.file_name or "Note")
        file_esc = html.escape(it.file_name or Path(it.source_file).name or "note.md")
        date_esc = html.escape(it.created_at)
        excerpt_esc = html.escape(it.excerpt or it.preview or "")
        body_rendered = render_markdown_to_html(it.body)
        tier = it.tier
        tier_label = "Tier 1 Pro" if tier == "tier1" else ("Tier 3 Low" if tier == "tier3" else "Tier 2 Flash")
        card_id = f"mail-card-{i}"

        is_docket = it.id.startswith("docket-")
        scope = "docket" if is_docket else "archive"

        # Determine status and badges
        if it.status == TriageStatus.PENDING:
            status = "untriaged"
            badge_text = "⚪ UNTRIAGED"
            badge_cls = "badge-unhandled"
        elif it.status == TriageStatus.BACKLOG:
            status = "backlog"
            badge_text = "📥 BACKLOG"
            badge_cls = "badge-dispatched"
        elif it.status == TriageStatus.COMPLETED:
            status = "completed"
            badge_text = "✅ COMPLETED"
            badge_cls = "badge-completed"
        elif it.status == TriageStatus.GRAVEYARD:
            status = "graveyard"
            badge_text = "🪦 GRAVEYARD"
            badge_cls = "badge-graveyard"
        elif it.status == TriageStatus.PCDA:
            status = "pcda"
            badge_text = "⚖️ PCDA"
            badge_cls = "badge-pcda"
        elif it.status == TriageStatus.RESEARCH:
            status = "research"
            badge_text = "🔬 RESEARCH"
            badge_cls = "badge-research"
        else:
            status = it.status.value.lower()
            badge_text = f"⚡ {it.status.value}"
            badge_cls = "badge-dispatched"

        is_dispatched = (status != "untriaged")

        priority = it.priority.value
        priority_badge = '<span class="mail-priority-badge badge-p0" style="background: rgba(239, 68, 68, 0.2); color: #f87171; border: 1px solid rgba(239, 68, 68, 0.4); padding: 2px 6px; border-radius: 4px; font-weight: 700; font-size: 11px;">🚨 P0 TOP PRIORITY</span>' if priority == "P0" else ""

        deps = it.dependencies or []
        dep_badges = "".join([f'<span class="mail-prereq-badge" style="background: rgba(245, 158, 11, 0.15); color: #fbbf24; border: 1px solid rgba(245, 158, 11, 0.3); padding: 2px 6px; border-radius: 4px; font-size: 11px;">⚠️ Prereq: {html.escape(d)}</span>' for d in deps])
        dep_chip = f'<button class="action-chip chip-prereq" style="color: #fbbf24; border-color: rgba(251, 191, 36, 0.4);" onclick="filterByPrereq(this)" data-prereq="{html.escape(deps[0], quote=True)}" title="Triage prerequisite first"><span>💡</span> Triage Prereq First</button>' if deps else ""

        merged_chip = ""
        if getattr(it, "merged_count", 0) > 1:
            merged_chip = f'<span class="badge badge-merged" style="background: rgba(168, 85, 247, 0.15); color: #c084fc; border: 1px solid rgba(168, 85, 247, 0.35); padding: 2px 6px; border-radius: 4px; font-size: 11px; font-weight: 600;" title="Merged from {it.merged_count} notes">🧩 merged from {it.merged_count} notes</span>'

        pillars = it.target_pillars or infer_pillars(it.title, it.tags, it.body)
        pillar_badges = render_pillar_badges(pillars)
        pillars_attr = html.escape(",".join(pillars), quote=True)

        docket_badge = f'<span class="badge badge-docket" title="From Curated Morning Mail Docket">⭐ DOCKET #{idx}</span>' if is_docket else ""

        file_pill_text = source_esc if is_docket else file_esc
        file_pill_icon = "📌" if is_docket else "📄"
        file_pill_esc = html.escape(file_pill_text)
        file_pill_display = file_pill_esc[:42] + ("..." if len(file_pill_esc) > 42 else "")
        file_pill = f'<span class="mail-file-pill" title="{file_pill_esc}">{file_pill_icon} {file_pill_display}</span>'

        dispatch_banner_html = ""
        if status == "backlog":
            dispatch_banner_html = f"""
        <div class="mail-dispatch-banner">
          <div style="background: rgba(52, 211, 153, 0.08); border: 1px solid rgba(52, 211, 153, 0.3); border-radius: 6px; padding: 10px 12px; margin: 10px 0; font-family: var(--font-mono); font-size: 11px;">
            <div style="color: #34d399; font-weight: 700; display: flex; align-items: center; justify-content: space-between; margin-bottom: 4px;">
              <span>📥 Cataloged in Backlog</span>
              <span style="font-size: 10px; color: var(--text-muted);">{html.escape(it.verdict_notes or '')}</span>
            </div>
            <div style="color: var(--text-muted); font-size: 10px; line-height: 1.4;">
              Documented in <strong>docs/BACKLOG.md</strong>.
            </div>
          </div>
        </div>"""
        elif status == "completed":
            dispatch_banner_html = f"""
        <div class="mail-dispatch-banner">
          <div style="background: rgba(56, 189, 248, 0.08); border: 1px solid rgba(56, 189, 248, 0.3); border-radius: 6px; padding: 10px 12px; margin: 10px 0; font-family: var(--font-mono); font-size: 11px;">
            <div style="color: #38bdf8; font-weight: 700; display: flex; align-items: center; justify-content: space-between; margin-bottom: 4px;">
              <span>✅ Completed & Codified</span>
              <span style="font-size: 10px; color: var(--text-muted);">Shipped</span>
            </div>
            <div style="color: var(--text-muted); font-size: 10px; line-height: 1.4;">
              Shipped and codified in <strong>docs/BACKLOG.md</strong>.
            </div>
          </div>
        </div>"""
        elif status == "graveyard":
            dispatch_banner_html = f"""
        <div class="mail-dispatch-banner">
          <div style="background: rgba(248, 113, 113, 0.08); border: 1px solid rgba(248, 113, 113, 0.3); border-radius: 6px; padding: 10px 12px; margin: 10px 0; font-family: var(--font-mono); font-size: 11px;">
            <div style="color: #f87171; font-weight: 700; display: flex; align-items: center; justify-content: space-between; margin-bottom: 4px;">
              <span>🪦 Strategy Graveyard (Rejected)</span>
              <span style="font-size: 10px; color: var(--text-muted);">Rejected</span>
            </div>
            <div style="color: var(--text-muted); font-size: 10px; line-height: 1.4;">
              Rejected and cataloged in <strong>docs/BACKLOG.md</strong> (Strategy Graveyard).
            </div>
          </div>
        </div>"""

        all_cards_html.append(f"""
      <div class="mail-card{' dispatched' if is_dispatched else ''}{' completed' if status == 'completed' else ''}{' graveyard' if status == 'graveyard' else ''}" id="{card_id}" data-card-idx="{i}" data-scope="{scope}" data-status="{status}" data-tier="{tier}" data-priority="{priority}" data-pillars="{pillars_attr}" data-title="{title_esc}" data-source="{source_esc}" data-file="{file_esc}">
        <div class="mail-card-header">
          <div class="mail-header-left">
            <span class="mail-idx-badge" title="Rank #{rank_val}">#{rank_val}</span>
            {docket_badge}
            <span class="badge tier-badge { 'tier-1' if tier == 'tier1' else ('tier-3' if tier == 'tier3' else 'tier-2') }">{tier_label}</span>
            {pillar_badges}
            {priority_badge}
            {dep_badges}
            {merged_chip}
            <span class="mail-date-badge">📅 {date_esc}</span>
            <span class="mail-status-badge {badge_cls}" id="badge-{card_id}">{badge_text}</span>
          </div>
          {file_pill}
        </div>


        <h3 class="mail-card-title">{title_esc}</h3>

        <div class="mail-excerpt-box">
          <p class="mail-excerpt-text">{excerpt_esc}</p>
        </div>

        <details class="mail-body-details"{' open' if is_dispatched else ''}>
          <summary class="mail-body-toggle">👁️ View Ingested Note & Context ({it.word_count} words)</summary>
          <div class="mail-body-rendered">{body_rendered}</div>
        </details>
        {dispatch_banner_html}
        <div class="mail-card-actions">
          <span class="actions-label">Dispatch Action:</span>
          <div class="mail-chip-group">
            {dep_chip}
            <button class="action-chip chip-backlog" onclick="dispatchMailAction('{card_id}', 'backlog')" title="Add to docs/BACKLOG.md">
              <span>📥</span> Backlog
            </button>
            <button class="action-chip chip-research" onclick="dispatchMailAction('{card_id}', 'research')" title="Route to TheKlangResearch">
              <span>🔬</span> Research
            </button>
            <button class="action-chip chip-graveyard" onclick="dispatchMailAction('{card_id}', 'graveyard')" title="Reject and record in Graveyard">
              <span>🪦</span> Graveyard
            </button>
            <button class="action-chip chip-pcda" onclick="dispatchMailAction('{card_id}', 'pcda')" title="Request 4-Lens PCDA Dialectic">
              <span>⚖️</span> PCDA
            </button>
          </div>
        </div>
      </div>
""")

    all_cards_joined = "".join(all_cards_html)
    inbox_dir_esc = html.escape(str(inbox_dir))

    html_content = f"""<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8"/>
  <meta content="width=device-width, initial-scale=1.0" name="viewport"/>
  <meta http-equiv="Cache-Control" content="no-cache, no-store, must-revalidate"/>
  <meta http-equiv="Pragma" content="no-cache"/>
  <meta http-equiv="Expires" content="0"/>
  <title>📬 N'kai Morning Mail Inbox & Triage Lab</title>
  <script src="/preload.js"></script>
  <style>
    :root {{
      --background: #090d13;
      --foreground: #cbd5e1;
      --bg-base: #090d13;
      --bg-panel: #111620;
      --bg-card: #18202c;
      --border: #263345;
      --text-main: #cbd5e1;
      --text-muted: #8494a7;
      --accent-blue: #38bdf8;
      --accent-green: #34d399;
      --accent-red: #f87171;
      --accent-neon: #22d3ee;
      --accent-amber: #fbbf24;
      --accent-pink: #f43f5e;
      --accent-purple: #a855f7;
      --font-mono: 'JetBrains Mono', 'Consolas', monospace;
      --font-sans: 'Inter', -apple-system, sans-serif;
    }}

    * {{ box-sizing: border-box; }}

    html, body {{
      min-height: 100vh;
      margin: 0;
      padding: 0;
      background: var(--bg-base, #090d13) !important;
      background-color: var(--bg-base, #090d13) !important;
      color: var(--text-main, #cbd5e1) !important;
      font-family: var(--font-sans);
      font-size: 1rem;
    }}

    .inbox-container {{
      background: var(--bg-base, #090d13) !important;
    }}

    /* ==========================================================================
       Two-Tier Header (Flight 5 Standard)
       ========================================================================== */
    .header-tier-top {{
      position: sticky;
      top: 0;
      z-index: 1000;
      background: rgba(9, 13, 19, 0.94);
      backdrop-filter: blur(12px);
      border-bottom: 1px solid var(--border);
      padding: 10px 20px;
      display: flex;
      flex-wrap: wrap;
      align-items: center;
      justify-content: space-between;
      gap: 12px;
    }}

    .header-left {{
      display: flex;
      align-items: center;
      gap: 10px;
      flex-wrap: wrap;
    }}

    .brand-title {{
      font-family: var(--font-mono);
      font-size: 14px;
      font-weight: 800;
      letter-spacing: 0.05em;
      color: var(--accent-amber);
      display: flex;
      align-items: center;
      gap: 8px;
    }}

    .brand-tag {{
      font-size: 10px;
      padding: 2px 7px;
      border-radius: 4px;
      background: rgba(251, 191, 36, 0.12);
      border: 1px solid rgba(251, 191, 36, 0.3);
      color: var(--accent-amber);
      font-family: var(--font-mono);
    }}

    .header-source {{
      font-family: var(--font-mono);
      font-size: 11px;
      color: var(--text-muted);
      background: var(--bg-panel);
      padding: 4px 8px;
      border-radius: 6px;
      border: 1px solid var(--border);
      max-width: 340px;
      overflow: hidden;
      text-overflow: ellipsis;
      white-space: nowrap;
    }}

    .utility-cluster {{
      display: flex;
      align-items: center;
      gap: 8px;
    }}

    .header-pill {{
      font-family: var(--font-mono);
      font-size: 11px;
      font-weight: 700;
      padding: 4px 10px;
      border-radius: 6px;
      border: 1px solid var(--border);
      background: var(--bg-panel);
      display: inline-flex;
      align-items: center;
      gap: 6px;
      cursor: pointer;
      user-select: none;
      transition: all 0.15s ease;
    }}

    .header-pill:hover {{
      border-color: var(--accent-blue);
    }}

    .meter-badge {{
      font-size: 9px;
      padding: 1px 5px;
      border-radius: 3px;
      font-weight: 800;
    }}
    .meter-safe {{ background: rgba(52, 211, 153, 0.2); color: #34d399; border: 1px solid rgba(52, 211, 153, 0.3); }}
    .meter-warning {{ background: rgba(251, 191, 36, 0.2); color: #fbbf24; border: 1px solid rgba(251, 191, 36, 0.3); }}
    .meter-critical {{ background: rgba(248, 113, 113, 0.2); color: #f87171; border: 1px solid rgba(248, 113, 113, 0.3); }}

    .header-btn {{
      padding: 4px 8px;
      border-radius: 6px;
      border: 1px solid var(--border);
      background: var(--bg-panel);
      color: var(--text-main);
      cursor: pointer;
      font-family: var(--font-mono);
      font-size: 11px;
      display: inline-flex;
      align-items: center;
      justify-content: center;
      transition: all 0.15s ease;
    }}

    .header-btn:hover {{
      background: var(--bg-card);
      border-color: var(--accent-blue);
      color: #fff;
    }}

    /* ==========================================================================
       Two-Tier Consolidated Navigation & Filter Rails (Flights 6 & 13)
       ========================================================================== */
    .header-tier-bottom {{
      position: sticky;
      top: 49px;
      z-index: 990;
      background: rgba(13, 18, 26, 0.96);
      backdrop-filter: blur(12px);
      border-bottom: 1px solid var(--border);
      padding: 10px 20px;
      display: flex;
      flex-direction: column;
      gap: 8px;
    }}

    .rail-row {{
      display: flex;
      align-items: center;
      gap: 10px;
      flex-wrap: wrap;
    }}

    .rail-label {{
      font-family: var(--font-mono);
      font-size: 11px;
      font-weight: 800;
      color: var(--text-muted);
      letter-spacing: 0.06em;
      text-transform: uppercase;
      min-width: 64px;
    }}

    .rail-chips {{
      display: flex;
      align-items: center;
      gap: 6px;
      flex-wrap: wrap;
      flex: 1;
    }}

    .pill-btn, .filter-chip {{
      font-family: var(--font-mono);
      font-size: 11px;
      font-weight: 700;
      padding: 4px 10px;
      border-radius: 6px;
      border: 1px solid var(--border);
      background: var(--bg-panel);
      color: var(--text-muted);
      cursor: pointer;
      display: inline-flex;
      align-items: center;
      gap: 6px;
      transition: all 0.15s ease;
      user-select: none;
    }}

    .pill-btn:hover, .filter-chip:hover {{
      color: var(--text-main);
      border-color: var(--accent-blue);
      transform: translateY(-1px);
    }}

    .pill-btn.active {{
      background: rgba(56, 189, 248, 0.16);
      border-color: var(--accent-blue);
      color: #fff;
      box-shadow: 0 0 10px rgba(56, 189, 248, 0.25);
    }}

    .filter-chip.active {{
      background: rgba(251, 191, 36, 0.14);
      border-color: var(--accent-amber);
      color: var(--accent-amber);
      box-shadow: 0 0 10px rgba(251, 191, 36, 0.2);
    }}

    /* Embedded Counter Badges (#26) */
    .pill-count, .chip-count {{
      font-family: var(--font-mono);
      font-size: 10px;
      font-weight: 800;
      padding: 1px 6px;
      border-radius: 4px;
      background: rgba(0, 0, 0, 0.35);
      border: 1px solid rgba(255, 255, 255, 0.1);
      color: var(--text-main);
    }}

    .pill-btn.active .pill-count {{
      background: rgba(56, 189, 248, 0.3);
      border-color: rgba(56, 189, 248, 0.5);
      color: #fff;
    }}

    .filter-chip.active .chip-count {{
      background: rgba(251, 191, 36, 0.25);
      border-color: rgba(251, 191, 36, 0.4);
      color: var(--accent-amber);
    }}

    /* Main Container */
    .inbox-container {{
      max-width: 1160px;
      margin: 0 auto;
      padding: 24px 20px 80px;
    }}

    /* Search Input */
    .search-box-wrapper {{
      margin-bottom: 20px;
      position: relative;
    }}

    .search-input {{
      width: 100%;
      background: var(--bg-panel);
      border: 1px solid var(--border);
      border-radius: 8px;
      padding: 10px 14px 10px 38px;
      font-size: 13px;
      color: var(--text-main);
      font-family: var(--font-sans);
      outline: none;
      transition: all 0.2s ease;
    }}

    .search-input:focus {{
      border-color: var(--accent-blue);
      box-shadow: 0 0 12px rgba(56, 189, 248, 0.2);
    }}

    .search-icon-pos {{
      position: absolute;
      left: 12px;
      top: 10px;
      color: var(--text-muted);
      pointer-events: none;
    }}

    /* Mail Cards */
    .mail-grid {{
      display: flex;
      flex-direction: column;
      gap: 16px;
    }}

    .mail-card {{
      background: var(--bg-panel);
      border: 1px solid var(--border);
      border-radius: 10px;
      padding: 18px 20px;
      transition: all 0.2s ease;
      position: relative;
    }}

    .mail-card:hover {{
      border-color: rgba(56, 189, 248, 0.45);
      box-shadow: 0 6px 20px rgba(0, 0, 0, 0.4);
    }}

    .mail-card.dispatched {{
      border-color: rgba(52, 211, 153, 0.45);
      background: rgba(6, 78, 59, 0.12);
    }}

    .mail-card.completed {{
      border-color: rgba(56, 189, 248, 0.45);
      background: rgba(12, 74, 110, 0.12);
    }}

    .mail-card.graveyard {{
      border-color: rgba(248, 113, 113, 0.45);
      background: rgba(127, 29, 29, 0.12);
    }}

    .mail-card-header {{
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 8px;
      margin-bottom: 8px;
      flex-wrap: wrap;
    }}

    .mail-header-left {{
      display: flex;
      align-items: center;
      gap: 8px;
      flex-wrap: wrap;
    }}

    .mail-idx-badge {{
      font-family: var(--font-mono);
      font-size: 11px;
      font-weight: 800;
      color: var(--accent-blue);
      background: rgba(56, 189, 248, 0.12);
      padding: 2px 7px;
      border-radius: 4px;
      border: 1px solid rgba(56, 189, 248, 0.3);
    }}

    .badge-docket {{
      background: rgba(251, 191, 36, 0.15);
      color: #fbbf24;
      border: 1px solid rgba(251, 191, 36, 0.35);
      font-family: var(--font-mono);
      font-size: 10px;
      font-weight: 800;
      padding: 2px 7px;
      border-radius: 4px;
    }}

    .tier-badge {{
      font-family: var(--font-mono);
      font-size: 10px;
      font-weight: 800;
      padding: 2px 7px;
      border-radius: 4px;
    }}
    .tier-1 {{ background: rgba(244, 63, 94, 0.15); color: #f43f5e; border: 1px solid rgba(244, 63, 94, 0.3); }}
    .tier-2 {{ background: rgba(56, 189, 248, 0.15); color: #38bdf8; border: 1px solid rgba(56, 189, 248, 0.3); }}
    .tier-3 {{ background: rgba(148, 163, 184, 0.15); color: #94a3b8; border: 1px solid rgba(148, 163, 184, 0.3); }}

    /* Platform & Ecosystem Pillar Badges (Flights 6 & 13) */
    .pillar-chip {{
      font-family: var(--font-mono);
      font-size: 10px;
      font-weight: 800;
      padding: 2px 7px;
      border-radius: 4px;
      display: inline-flex;
      align-items: center;
      gap: 3px;
      cursor: pointer;
      letter-spacing: 0.02em;
      transition: all 0.15s ease;
      user-select: none;
    }}
    .pillar-tks {{ background: rgba(2, 132, 199, 0.15); color: #38bdf8; border: 1px solid rgba(2, 132, 199, 0.35); }}
    .pillar-tt {{ background: rgba(16, 185, 129, 0.15); color: #34d399; border: 1px solid rgba(16, 185, 129, 0.35); }}
    .pillar-nk {{ background: rgba(139, 92, 246, 0.15); color: #a78bfa; border: 1px solid rgba(139, 92, 246, 0.35); }}
    .pillar-tkr {{ background: rgba(245, 158, 11, 0.15); color: #fbbf24; border: 1px solid rgba(245, 158, 11, 0.35); }}
    .pillar-agy {{ background: rgba(244, 63, 94, 0.15); color: #fb7185; border: 1px solid rgba(244, 63, 94, 0.35); }}
    .pillar-tbd16 {{ background: rgba(234, 88, 12, 0.15); color: #fb923c; border: 1px solid rgba(234, 88, 12, 0.35); }}
    .pillar-overflow {{
      background: rgba(148, 163, 184, 0.12);
      color: #94a3b8;
      border: 1px solid rgba(148, 163, 184, 0.25);
      cursor: pointer;
    }}
    .pillar-chip:hover {{
      transform: translateY(-2px);
      filter: brightness(1.25);
      box-shadow: 0 2px 8px rgba(0, 0, 0, 0.4);
    }}

    .mail-date-badge {{
      font-family: var(--font-mono);
      font-size: 11px;
      color: var(--text-muted);
    }}

    .mail-status-badge {{
      font-family: var(--font-mono);
      font-size: 10px;
      font-weight: 800;
      padding: 2px 7px;
      border-radius: 4px;
      text-transform: uppercase;
    }}

    .mail-status-badge.badge-unhandled {{
      color: #94a3b8;
      background: rgba(148, 163, 184, 0.15);
      border: 1px solid rgba(148, 163, 184, 0.25);
    }}

    .mail-status-badge.badge-dispatched {{
      color: #34d399;
      background: rgba(52, 211, 153, 0.15);
      border: 1px solid rgba(52, 211, 153, 0.35);
    }}

    .mail-status-badge.badge-completed {{
      color: #38bdf8;
      background: rgba(56, 189, 248, 0.15);
      border: 1px solid rgba(56, 189, 248, 0.35);
    }}

    .mail-status-badge.badge-graveyard {{
      color: #f87171;
      background: rgba(248, 113, 113, 0.15);
      border: 1px solid rgba(248, 113, 113, 0.35);
    }}

    .mail-status-badge.badge-research {{
      color: var(--accent-blue);
      background: rgba(56, 189, 248, 0.15);
      border: 1px solid rgba(56, 189, 248, 0.35);
    }}

    .mail-status-badge.badge-pcda {{
      color: var(--accent-amber);
      background: rgba(251, 191, 36, 0.15);
      border: 1px solid rgba(251, 191, 36, 0.35);
    }}

    .mail-file-pill {{
      font-family: var(--font-mono);
      font-size: 11px;
      color: var(--text-muted);
      background: var(--bg-card);
      padding: 3px 8px;
      border-radius: 4px;
      border: 1px solid var(--border);
    }}

    .mail-card-title {{
      font-size: 16px;
      font-weight: 700;
      color: #fff;
      margin: 4px 0 10px 0;
      line-height: 1.4;
    }}

    .mail-excerpt-box {{
      background: var(--bg-card);
      border-left: 3px solid var(--accent-blue);
      border-radius: 0 6px 6px 0;
      padding: 10px 14px;
      margin-bottom: 12px;
    }}

    .mail-excerpt-text {{
      margin: 0;
      font-size: 12.5px;
      color: var(--text-main);
      line-height: 1.55;
    }}

    .mail-body-details {{
      margin-bottom: 14px;
    }}

    .mail-body-toggle {{
      font-size: 11px;
      color: var(--accent-blue);
      font-family: var(--font-mono);
      cursor: pointer;
      user-select: none;
      font-weight: 600;
    }}

    .mail-body-toggle:hover {{
      text-decoration: underline;
    }}

    .mail-body-rendered {{
      margin-top: 8px;
      padding: 14px 16px;
      background: #080d16;
      border: 1px solid var(--border);
      border-radius: 6px;
      font-family: var(--font-sans);
      font-size: 12px;
      color: var(--text-main);
      max-height: 380px;
      overflow-y: auto;
      line-height: 1.6;
    }}

    .mail-body-rendered h3, .mail-body-rendered h4, .mail-body-rendered h5 {{
      color: #f1f5f9;
      margin: 12px 0 6px;
      font-weight: 700;
      font-size: 12.5px;
    }}

    .mail-body-rendered p {{ margin: 6px 0; }}
    .mail-body-rendered ul, .mail-body-rendered ol {{ margin: 6px 0 6px 18px; padding-left: 0; }}
    .mail-body-rendered li {{ margin: 3px 0; }}
    .mail-body-rendered code {{
      font-family: var(--font-mono);
      font-size: 11px;
      background: rgba(15, 23, 42, 0.9);
      border: 1px solid rgba(56, 189, 248, 0.25);
      border-radius: 4px;
      padding: 1px 5px;
      color: var(--accent-blue);
    }}
    .mail-body-rendered pre {{
      background: #030712;
      border: 1px solid var(--border);
      border-radius: 6px;
      padding: 10px 12px;
      margin: 8px 0;
      overflow-x: auto;
      font-family: var(--font-mono);
      font-size: 11px;
      line-height: 1.45;
    }}
    .mail-body-rendered pre code {{ background: transparent; border: none; padding: 0; color: #e2e8f0; }}
    .mail-body-rendered a {{ color: var(--accent-blue); text-decoration: underline; text-underline-offset: 2px; }}
    .mail-body-rendered a:hover {{ color: #fff; }}
    .mail-body-rendered blockquote {{
      border-left: 3px solid var(--accent-blue);
      margin: 8px 0;
      padding-left: 12px;
      color: var(--text-muted);
      font-style: italic;
    }}
    .mail-body-rendered hr {{ border: 0; border-top: 1px solid var(--border); margin: 12px 0; }}

    .mail-card-actions {{
      display: flex;
      flex-wrap: wrap;
      align-items: center;
      gap: 10px;
      padding-top: 10px;
      border-top: 1px solid rgba(38, 51, 69, 0.7);
    }}

    .actions-label {{
      font-family: var(--font-mono);
      font-size: 11px;
      color: var(--text-muted);
      font-weight: 600;
    }}

    .mail-chip-group {{
      display: flex;
      flex-wrap: wrap;
      gap: 6px;
    }}

    .action-chip {{
      display: inline-flex;
      align-items: center;
      gap: 5px;
      padding: 4px 10px;
      border-radius: 6px;
      font-family: var(--font-mono);
      font-size: 11px;
      font-weight: 700;
      cursor: pointer;
      transition: all 0.15s ease;
      border: 1px solid var(--border);
      background: var(--bg-card);
      user-select: none;
    }}

    .chip-backlog {{ color: #34d399; border-color: rgba(52, 211, 153, 0.3); }}
    .chip-backlog:hover {{ background: rgba(52, 211, 153, 0.15); border-color: #34d399; }}

    .chip-research {{ color: var(--accent-blue); border-color: rgba(56, 189, 248, 0.3); }}
    .chip-research:hover {{ background: rgba(56, 189, 248, 0.15); border-color: var(--accent-blue); }}

    .chip-graveyard {{ color: #f87171; border-color: rgba(248, 113, 113, 0.3); }}
    .chip-graveyard:hover {{ background: rgba(248, 113, 113, 0.15); border-color: #f87171; }}

    .chip-pcda {{ color: #fbbf24; border-color: rgba(251, 191, 36, 0.3); }}
    .chip-pcda:hover {{ background: rgba(251, 191, 36, 0.15); border-color: #fbbf24; }}

    /* Toast Notification */
    #action-toast {{
      position: fixed;
      bottom: 24px;
      right: 24px;
      padding: 11px 18px;
      background: #111827;
      border: 1px solid var(--accent-blue);
      border-radius: 8px;
      color: #fff;
      font-family: var(--font-mono);
      font-size: 12px;
      box-shadow: 0 10px 30px rgba(0, 0, 0, 0.7);
      opacity: 0;
      pointer-events: none;
      transform: translateY(12px);
      transition: all 0.22s ease;
      z-index: 99999;
      display: flex;
      align-items: center;
      gap: 8px;
    }}

    #action-toast.show {{
      opacity: 1;
      transform: translateY(0);
    }}

    @media (max-width: 640px) {{
      .header-tier-top, .header-tier-bottom {{
        flex-direction: column;
        align-items: flex-start;
      }}
      .header-source {{ max-width: 100%; }}
      .mail-card-header {{
        flex-direction: column;
        align-items: flex-start;
      }}
    }}
  </style>
</head>
<body>

  <!-- Top-Tier Header -->
  <header class="header-tier-top">
    <div class="header-left">
      <div class="brand-title">
        <span>📬</span>
        <span>N'KAI MORNING MAIL</span>
        <span class="brand-tag">v0.4.0-test</span>
      </div>
      <div class="header-source" title="{inbox_dir_esc}">📁 {inbox_dir_esc}</div>
    </div>

    <div class="utility-cluster">
      <div class="header-pill" id="header-allowance-pill" title="Cumulative approved tokens: 0k (Budget: 200k)">
        <span>🔋</span>
        <span id="allowance-val">~0k</span>
        <span class="meter-badge meter-safe" id="allowance-status">SAFE 🟢</span>
      </div>
      <button class="header-btn" id="btn-haptics" onclick="toggleAudioHaptics()" title="Audio Haptics: ON (Click to mute)">
        <span id="haptics-icon">🔊</span>
      </button>
      <button class="header-btn" onclick="forceReloadCanvas()" title="Hard Refresh Canvas">🔄</button>
    </div>
  </header>

  <!-- Bottom-Tier Navigation & Filter Rails (Flights 6 & 13) -->
  <div class="header-tier-bottom">
    <!-- Rail 1: Platform & Product Pillars -->
    <div class="rail-row rail-pillars">
      <span class="rail-label">Product:</span>
      <div class="rail-chips">
        <button class="pill-btn active" id="pillar-btn-all" onclick="applyPillarFilter('all', event)" title="Show All Platform Products">
          <span class="pill-icon">📋</span> All Mail <span class="pill-count" id="count-pillar-all">{total_count}</span>
        </button>
        {pillar_buttons_html}
      </div>
    </div>

    <!-- Rail 2: Status & Triage Filters -->
    <div class="rail-row rail-status">
      <span class="rail-label">Status:</span>
      <div class="rail-chips">
        <button class="filter-chip" id="filter-all" onclick="applyStatusFilter('all')" title="Show All Statuses">
          📋 All <span class="chip-count" id="count-status-all">{total_count}</span>
        </button>
        <button class="filter-chip active" id="filter-untriaged" onclick="applyStatusFilter('untriaged')" title="Show Untriaged Mail (Inbox Zero Focus)">
          ⚪ Untriaged <span class="chip-count" id="count-status-untriaged">{untriaged_count}</span>
        </button>
        <button class="filter-chip" id="filter-p0" onclick="applyStatusFilter('p0')" title="Filter by P0 Top Priority">
          🚨 P0 <span class="chip-count" id="count-status-p0">{p0_count}</span>
        </button>
        <button class="filter-chip" id="filter-backlog" onclick="applyStatusFilter('backlog')" title="Filter by Cataloged in Backlog">
          📥 Backlog <span class="chip-count" id="count-status-backlog">{backlog_count}</span>
        </button>
        <button class="filter-chip" id="filter-completed" onclick="applyStatusFilter('completed')" title="Filter by Completed & Codified">
          ✅ Completed <span class="chip-count" id="count-status-completed">{completed_count}</span>
        </button>
        <button class="filter-chip" id="filter-research" onclick="applyStatusFilter('research')" title="Filter by Dispatched to Research">
          🔬 Research <span class="chip-count" id="count-status-research">{research_count}</span>
        </button>
        <button class="filter-chip" id="filter-graveyard" onclick="applyStatusFilter('graveyard')" title="Filter by Strategy Graveyard">
          🪦 Graveyard <span class="chip-count" id="count-status-graveyard">{graveyard_count}</span>
        </button>
        <button class="filter-chip" id="filter-pcda" onclick="applyStatusFilter('pcda')" title="Filter by PCDA Dialectic">
          ⚖️ PCDA <span class="chip-count" id="count-status-pcda">{pcda_count}</span>
        </button>
        <button class="filter-chip" id="filter-docket" onclick="applyStatusFilter('docket')" title="Filter by Curated Docket Items">
          ⭐ Docket <span class="chip-count" id="count-status-docket">{docket_count}</span>
        </button>
      </div>
    </div>
  </div>

  <!-- Main Container -->
  <main class="inbox-container">
    <!-- Search Input -->
    <div class="search-box-wrapper">
      <span class="search-icon-pos">🔍</span>
      <input type="text" id="search-input" class="search-input" placeholder="Filter notes by title, keyword, filename, or pillar..." oninput="handleSearch()"/>
    </div>

    <!-- Unified Mail Stream Grid -->
    <div class="mail-grid" id="grid-all">
      {all_cards_joined if items else '<div class="mail-card" style="text-align:center;color:var(--text-muted);"><p>🎉 Mailbox zero! No active notes found.</p></div>'}
    </div>
  </main>

  <div id="action-toast">⚡ Action Dispatched!</div>

  <script>
    let activePillar = 'all';
    let activeFilter = 'untriaged'; // Sanity-preserving default filter!
    let audioMuted = false;
    let hapticAudioCtx = null;

    const triageStats = {{
      total: {total_count},
      untriaged: {untriaged_count},
      dispatched: {dispatched_count},
      backlog: {backlog_count},
      completed: {completed_count},
      research: {research_count},
      graveyard: {graveyard_count},
      pcda: {pcda_count},
      p0: {p0_count},
      docket: {docket_count}
    }};

    const tokenLedger = {{
      tokensPro: 0,
      tokensFlash: 0,
      tokensMinimal: 0,
      totalTokens: 0
    }};

    /* ==========================================================================
       Procedural Web Audio Haptics (Flight 5 Standard)
       ========================================================================== */
    function initAudioHaptics() {{
      const saved = localStorage.getItem('nkai_audio_muted');
      if (saved !== null) {{
        audioMuted = (saved === 'true');
      }}
      updateHapticsUI();
    }}

    function updateHapticsUI() {{
      const btn = document.getElementById('btn-haptics');
      const icon = document.getElementById('haptics-icon');
      if (!icon) return;
      if (!audioMuted) {{
        icon.textContent = '🔊';
        if (btn) btn.title = 'Audio Haptics: ON (Click to mute)';
      }} else {{
        icon.textContent = '🔇';
        if (btn) btn.title = 'Audio Haptics: MUTED (Click to unmute)';
      }}
    }}

    function toggleAudioHaptics() {{
      audioMuted = !audioMuted;
      localStorage.setItem('nkai_audio_muted', audioMuted ? 'true' : 'false');
      updateHapticsUI();
      if (!audioMuted) {{
        playHaptic('action');
        showToast('🔊 Audio Haptics Unmuted');
      }} else {{
        showToast('🔇 Audio Haptics Muted');
      }}
    }}

    function playHaptic(type) {{
      if (audioMuted) return;
      try {{
        const AudioContextClass = window.AudioContext || window.webkitAudioContext;
        if (!AudioContextClass) return;
        if (!hapticAudioCtx) {{
          hapticAudioCtx = new AudioContextClass();
        }}
        if (hapticAudioCtx.state === 'suspended') {{
          hapticAudioCtx.resume();
        }}
        const t = hapticAudioCtx.currentTime;
        const osc = hapticAudioCtx.createOscillator();
        const gain = hapticAudioCtx.createGain();

        if (type === 'action') {{
          osc.type = 'sine';
          osc.frequency.setValueAtTime(587.33, t);
          osc.frequency.exponentialRampToValueAtTime(880, t + 0.06);
          gain.gain.setValueAtTime(0.06, t);
          gain.gain.exponentialRampToValueAtTime(0.0001, t + 0.06);
          osc.connect(gain);
          gain.connect(hapticAudioCtx.destination);
          osc.start(t);
          osc.stop(t + 0.06);
        }} else if (type === 'tab') {{
          osc.type = 'triangle';
          osc.frequency.setValueAtTime(440, t);
          osc.frequency.exponentialRampToValueAtTime(660, t + 0.04);
          gain.gain.setValueAtTime(0.04, t);
          gain.gain.exponentialRampToValueAtTime(0.0001, t + 0.04);
          osc.connect(gain);
          gain.connect(hapticAudioCtx.destination);
          osc.start(t);
          osc.stop(t + 0.04);
        }} else {{
          osc.type = 'sine';
          osc.frequency.setValueAtTime(800, t);
          osc.frequency.exponentialRampToValueAtTime(400, t + 0.02);
          gain.gain.setValueAtTime(0.03, t);
          gain.gain.exponentialRampToValueAtTime(0.0001, t + 0.02);
          osc.connect(gain);
          gain.connect(hapticAudioCtx.destination);
          osc.start(t);
          osc.stop(t + 0.02);
        }}
      }} catch (e) {{}}
    }}

    /* ==========================================================================
       Two-Tier Filter Switching & Unified Search (Flights 6 & 13)
       ========================================================================== */
    function applyPillarFilter(pillarKey, ev) {{
      if (ev) ev.stopPropagation();
      playHaptic('tick');
      activePillar = pillarKey;
      document.querySelectorAll('.pill-btn').forEach(btn => btn.classList.remove('active'));
      const slug = pillarKey.toLowerCase().replace('-', '');
      const activeBtn = document.getElementById('pillar-btn-' + slug);
      if (activeBtn) activeBtn.classList.add('active');
      showToast(pillarKey === 'all' ? '📋 Showing All Products' : `🔍 Filtered by Pillar: ${{pillarKey}}`);
      handleSearch();
    }}

    function applyStatusFilter(statusKey) {{
      playHaptic('tick');
      activeFilter = statusKey;
      document.querySelectorAll('.filter-chip').forEach(c => c.classList.remove('active'));
      const activeChip = document.getElementById('filter-' + statusKey);
      if (activeChip) activeChip.classList.add('active');
      handleSearch();
    }}

    function handleSearch() {{
      const query = (document.getElementById('search-input').value || '').toLowerCase();
      const cards = document.querySelectorAll('#grid-all .mail-card');

      cards.forEach(card => {{
        const title = (card.getAttribute('data-title') || '').toLowerCase();
        const file = (card.getAttribute('data-file') || '').toLowerCase();
        const source = (card.getAttribute('data-source') || '').toLowerCase();
        const status = (card.getAttribute('data-status') || 'untriaged').toLowerCase();
        const priority = (card.getAttribute('data-priority') || 'p2').toLowerCase();
        const scope = (card.getAttribute('data-scope') || 'archive').toLowerCase();
        const rawPillars = (card.getAttribute('data-pillars') || '').split(',');
        const pillars = rawPillars.map(p => p.trim());
        const text = card.innerText.toLowerCase();

        const matchesQuery = !query || title.includes(query) || file.includes(query) || source.includes(query) || text.includes(query);
        const matchesPillar = (activePillar === 'all') || pillars.includes(activePillar);

        let matchesStatus = false;
        if (activeFilter === 'all') {{
          matchesStatus = true;
        }} else if (activeFilter === 'p0') {{
          matchesStatus = (priority === 'p0');
        }} else if (activeFilter === 'docket') {{
          matchesStatus = (scope === 'docket');
        }} else {{
          matchesStatus = (status === activeFilter);
        }}

        if (matchesQuery && matchesPillar && matchesStatus) {{
          card.style.display = 'block';
        }} else {{
          card.style.display = 'none';
        }}
      }});
    }}

    function updateCountersUI() {{
      const setVal = (id, val) => {{
        const el = document.getElementById(id);
        if (el) el.textContent = val;
      }};
      setVal('count-status-all', triageStats.total);
      setVal('count-status-untriaged', triageStats.untriaged);
      setVal('count-status-p0', triageStats.p0);
      setVal('count-status-backlog', triageStats.backlog);
      setVal('count-status-completed', triageStats.completed || 0);
      setVal('count-status-research', triageStats.research);
      setVal('count-status-graveyard', triageStats.graveyard);
      setVal('count-status-pcda', triageStats.pcda);
      setVal('count-status-docket', triageStats.docket);
      setVal('count-pillar-all', triageStats.total);
    }}

    function showToast(msg) {{
      const toast = document.getElementById('action-toast');
      if (!toast) return;
      toast.textContent = msg;
      toast.classList.add('show');
      setTimeout(() => toast.classList.remove('show'), 2400);
    }}

    function forceReloadCanvas() {{
      try {{
        const url = new URL(window.location.href);
        url.searchParams.set('t', Date.now().toString());
        window.location.href = url.toString();
      }} catch (e) {{
        window.location.href = window.location.pathname + '?t=' + Date.now();
      }}
    }}

    function filterByPrereq(target) {{
      playHaptic('tick');
      const depName = typeof target === 'string' ? target : (target && target.getAttribute ? target.getAttribute('data-prereq') : '');
      const searchBox = document.getElementById('search-input');
      if (searchBox && depName) {{
        searchBox.value = depName;
        searchBox.dispatchEvent(new Event('input'));
        showToast('🔍 Filtered by Prerequisite: ' + depName);
      }}
    }}

    function copyToClipboardSilently(text) {{
      if (!text) return false;
      let copied = false;
      try {{
        const ta = document.createElement('textarea');
        ta.value = text;
        ta.style.position = 'fixed';
        ta.style.top = '0';
        ta.style.left = '0';
        ta.style.width = '2em';
        ta.style.height = '2em';
        ta.style.opacity = '0.01';
        ta.style.zIndex = '999999';
        document.body.appendChild(ta);
        ta.focus();
        ta.select();
        ta.setSelectionRange(0, text.length);
        copied = document.execCommand('copy');
        document.body.removeChild(ta);
      }} catch (e) {{}}
      if (navigator.clipboard && navigator.clipboard.writeText) {{
        navigator.clipboard.writeText(text).catch(() => {{}});
      }}
      return copied;
    }}

    /* ==========================================================================
       Action Dispatching & Live Allowance Tracking
       ========================================================================== */
    function dispatchMailAction(cardId, actionOrTitle, maybeAction, maybeTier) {{
      playHaptic('action');

      let actionType = 'backlog';
      let title = '';
      let tier = 'tier2';

      const card = document.getElementById(cardId);
      if (card) {{
        title = card.getAttribute('data-title') || '';
        tier = card.getAttribute('data-tier') || 'tier2';
      }}

      if (['backlog', 'research', 'graveyard', 'pcda'].includes(actionOrTitle)) {{
        actionType = actionOrTitle;
      }} else if (maybeAction) {{
        actionType = maybeAction;
        if (actionOrTitle) title = actionOrTitle;
        if (maybeTier) tier = maybeTier;
      }}

      let promptText = '';
      if (actionType === 'backlog') {{
        promptText = `Add '${{title}}' to docs/BACKLOG.md under its target pillar with requirements and priority.`;
      }} else if (actionType === 'research') {{
        promptText = `Research '${{title}}' across TheKlangResearch and industry precedents.`;
      }} else if (actionType === 'graveyard') {{
        promptText = `Reject concept '${{title}}' and document strategic rationale in Strategy Graveyard.`;
      }} else if (actionType === 'pcda') {{
        promptText = `Conduct 4-Lens PCDA dialectic evaluation (Pros, Cons, Devil's Advocate, 80/20 Resolution) for '${{title}}'.`;
      }}

      // Dispatch via FastAPI bridge (:8000), environment adapter, or clipboard
      if (typeof fetch === 'function') {{
        fetch('http://127.0.0.1:8000/api/action', {{
          method: 'POST',
          headers: {{ 'Content-Type': 'application/json' }},
          body: JSON.stringify({{
            action: actionType.toUpperCase(),
            note_id: cardId.replace('mail-card-', ''),
            title: title,
            tier: tier,
            source: 'morning_mail_inbox'
          }})
        }}).then(res => {{
          if (res.ok) console.log('⚡ Action recorded to FastAPI bridge (:8000)');
        }}).catch(() => {{}});
      }}

      // 1. Direct Native Antigravity Extension IPC (if running in side pane)
      if (typeof window !== 'undefined' && window.sidecar && window.sidecar.agent && typeof window.sidecar.agent.sendMessage === 'function') {{
        try {{
          window.sidecar.agent.sendMessage(promptText);
          showToast(`⚡ Dispatched [${{actionType.toUpperCase()}}] directly to Agent!`);
        }} catch (e) {{
          console.warn('[Mail] Native IPC dispatch error:', e);
        }}
      }} else if (window.NkaiEnv && typeof window.NkaiEnv.dispatchAction === 'function') {{
        window.NkaiEnv.dispatchAction(promptText);
      }} else {{
        copyToClipboardSilently(promptText);
        showToast(`📋 Copied [${{actionType.toUpperCase()}}] prompt to clipboard! Paste in chat (Ctrl+V)`);
      }}

      // 2. Apply DOM modifications and persist to localStorage
      if (card) {{
        applyCardDOMStatus(card, cardId, actionType, title, tier, true);
        saveTriageAction(cardId, actionType, title, tier);
      }}
    }}

    function saveTriageAction(cardId, actionType, title, tier) {{
      try {{
        const key = 'nkai_mail_triage_actions';
        const raw = localStorage.getItem(key);
        const stored = raw ? JSON.parse(raw) : {{}};
        stored[cardId] = {{
          action: actionType,
          title: title,
          tier: tier,
          timestamp: Date.now()
        }};
        localStorage.setItem(key, JSON.stringify(stored));
      }} catch (e) {{
        console.warn('[Mail] LocalStorage save error:', e);
      }}
    }}

    function initLocalStorageSync() {{
      try {{
        const key = 'nkai_mail_triage_actions';
        const raw = localStorage.getItem(key);
        if (!raw) return;
        const stored = JSON.parse(raw);
        let changed = false;

        for (const [cardId, data] of Object.entries(stored)) {{
          const card = document.getElementById(cardId);
          if (!card) continue;
          const currentStatus = (card.getAttribute('data-status') || 'untriaged').toLowerCase();
          if (currentStatus === 'untriaged' && data && data.action) {{
            applyCardDOMStatus(card, cardId, data.action, data.title || '', data.tier || 'tier2', false);
            changed = true;
          }}
        }}
        if (changed) {{
          updateCountersUI();
          handleSearch();
        }}
      }} catch (e) {{
        console.warn('[Mail] LocalStorage restore error:', e);
      }}
    }}

    function applyCardDOMStatus(card, cardId, actionType, title, tier, isUserClick) {{
      const prevStatus = (card.getAttribute('data-status') || 'untriaged').toLowerCase();
      if (prevStatus === actionType) return;

      card.setAttribute('data-status', actionType);
      card.classList.add('dispatched');
      if (actionType === 'completed') {{
        card.classList.add('completed');
      }} else {{
        card.classList.remove('completed');
      }}

      const badge = document.getElementById('badge-' + cardId);
      if (badge) {{
        badge.className = 'mail-status-badge ' + (actionType === 'completed' ? 'badge-completed' : 'badge-dispatched');
        badge.textContent = '✅ ' + actionType.toUpperCase();
      }}

      let banner = card.querySelector('.mail-dispatch-banner');
      if (!banner) {{
        banner = document.createElement('div');
        banner.className = 'mail-dispatch-banner';
        const actionsBox = card.querySelector('.mail-card-actions');
        if (actionsBox) actionsBox.before(banner);
        else card.appendChild(banner);
      }}

      let promptText = '';
      if (actionType === 'backlog') {{
        promptText = `Add '${{title}}' to docs/BACKLOG.md under its target pillar with requirements and priority.`;
      }} else if (actionType === 'research') {{
        promptText = `Research '${{title}}' across TheKlangResearch and industry precedents.`;
      }} else if (actionType === 'graveyard') {{
        promptText = `Reject concept '${{title}}' and document strategic rationale in Strategy Graveyard.`;
      }} else if (actionType === 'pcda') {{
        promptText = `Conduct 4-Lens PCDA dialectic evaluation (Pros, Cons, Devil's Advocate, 80/20 Resolution) for '${{title}}'.`;
      }}

      const promptEsc = promptText.replace(/"/g, '&quot;').replace(/'/g, '&#x27;');
      const actionLabel = actionType.toUpperCase();
      const icon = actionType === 'pcda' ? '⚖️' : (actionType === 'research' ? '🔬' : (actionType === 'backlog' ? '📥' : '🪦'));
      const accentColor = actionType === 'pcda' ? '#fbbf24' : (actionType === 'research' ? 'var(--accent-blue)' : (actionType === 'backlog' ? '#34d399' : '#f87171'));
      const bgTint = actionType === 'pcda' ? 'rgba(251, 191, 36, 0.08)' : (actionType === 'research' ? 'rgba(56, 189, 248, 0.08)' : (actionType === 'backlog' ? 'rgba(52, 211, 153, 0.08)' : 'rgba(248, 113, 113, 0.08)'));
      const borderTint = actionType === 'pcda' ? 'rgba(251, 191, 36, 0.3)' : (actionType === 'research' ? 'rgba(56, 189, 248, 0.3)' : (actionType === 'backlog' ? 'rgba(52, 211, 153, 0.3)' : 'rgba(248, 113, 113, 0.3)'));

      banner.innerHTML = `
        <div style="background: ${{bgTint}}; border: 1px solid ${{borderTint}}; border-radius: 6px; padding: 10px 12px; margin: 10px 0; font-family: var(--font-mono); font-size: 11px;">
          <div style="color: ${{accentColor}}; font-weight: 700; display: flex; align-items: center; justify-content: space-between; margin-bottom: 6px;">
            <span>${{icon}} ${{actionLabel}} Prompt Ready</span>
            <span style="font-size: 10px; color: var(--text-muted);">Paste in Chat or Copy Below</span>
          </div>
          <div style="display: flex; gap: 6px; margin-bottom: 6px;">
            <input type="text" readonly value="${{promptEsc}}" onclick="this.select()" style="flex: 1; background: #0f172a; border: 1px solid ${{borderTint}}; border-radius: 4px; padding: 6px 8px; color: #f8fafc; font-family: var(--font-mono); font-size: 11px; outline: none; cursor: text;" />
            <button onclick="copyToClipboardSilently(this.previousElementSibling.value); this.textContent='✅ Copied!'; setTimeout(() => this.textContent='📋 Copy', 2000)" style="background: #1e293b; border: 1px solid ${{borderTint}}; border-radius: 4px; padding: 6px 12px; color: ${{accentColor}}; font-family: var(--font-mono); font-size: 11px; font-weight: 700; cursor: pointer; white-space: nowrap;">📋 Copy</button>
          </div>
          <div style="color: var(--text-muted); font-size: 10px; line-height: 1.4;">
            Press <kbd style="background:#1e293b; color:#fff; padding:1px 4px; border-radius:3px; border:1px solid #334155;">Ctrl+V</kbd> in chat to execute this action with Antigravity!
          </div>
        </div>`;

      const details = card.querySelector('.mail-body-details');
      if (details) {{
        details.open = true;
      }}

      // Update Stats
      if (prevStatus === 'untriaged') {{
        triageStats.untriaged = Math.max(0, triageStats.untriaged - 1);
        triageStats.dispatched++;
      }} else if (prevStatus in triageStats) {{
        triageStats[prevStatus] = Math.max(0, triageStats[prevStatus] - 1);
      }}
      if (actionType in triageStats) {{
        triageStats[actionType]++;
      }}

      updateCountersUI();

      if (isUserClick) {{
        const costMap = {{ tier1: 40000, tier2: 10000, tier3: 2000 }};
        const cost = costMap[tier] || 10000;
        if (actionType === 'backlog' || actionType === 'research') {{
          tokenLedger.totalTokens += cost;
          updateAllowanceUI();
        }}
        handleSearch();
      }}
    }}

    function updateAllowanceUI() {{
      const valEl = document.getElementById('allowance-val');
      const statusEl = document.getElementById('allowance-status');
      const pill = document.getElementById('header-allowance-pill');
      if (!valEl || !statusEl || !pill) return;

      const total = tokenLedger.totalTokens;
      const formattedK = total > 0 ? (total >= 1000 ? Math.round(total / 1000) + 'k' : total) : '0k';
      valEl.textContent = `~${{formattedK}}`;

      let band = 'safe';
      let bandLabel = 'SAFE 🟢';
      if (total > 200000) {{
        band = 'critical';
        bandLabel = 'CRITICAL 🔴';
      }} else if (total > 100000) {{
        band = 'warning';
        bandLabel = 'WARNING 🟡';
      }}

      statusEl.className = `meter-badge meter-${{band}}`;
      statusEl.textContent = bandLabel;
      pill.title = `Cumulative approved tokens: ${{total.toLocaleString()}} / 200,000 budget`;
    }}

    function forceDarkBody() {{
      if (document.body) {{
        document.body.style.setProperty('background', '#090d13', 'important');
        document.body.style.setProperty('background-color', '#090d13', 'important');
        document.body.style.setProperty('color', '#cbd5e1', 'important');
      }}
      document.documentElement.style.setProperty('--background', '#090d13');
      document.documentElement.style.setProperty('--foreground', '#cbd5e1');
    }}

    function initMailboard() {{
      forceDarkBody();
      initAudioHaptics();
      initLocalStorageSync();
      updateCountersUI();
      handleSearch();
    }}

    forceDarkBody();
    if (document.readyState === 'loading') {{
      document.addEventListener('DOMContentLoaded', initMailboard);
    }} else {{
      initMailboard();
    }}
  </script>
</body>
</html>
"""

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(html_content, encoding="utf-8")

    # Mirror to Antigravity Artifact directory for interactive IDE side-panel rendering
    artifact_dir = Path(r"C:\Users\codyg\.gemini\antigravity\brain\9241988e-1227-466e-a0a5-446d61a69d77")
    if artifact_dir.exists():
        try:
            artifact_file = artifact_dir / out_path.name
            artifact_file.write_text(html_content, encoding="utf-8")
        except Exception as e:
            pass

    # Mirror to active Antigravity Extension panel
    plugin_panel_file = Path(r"C:\Users\codyg\.gemini\config\plugins\nkai\sidecars\panel\index.html")
    if plugin_panel_file.parent.exists():
        try:
            plugin_panel_file.write_text(html_content, encoding="utf-8")
        except Exception as e:
            pass

    # Mirror to nkai repository sidecar panel
    nkai_panel_file = Path(r"c:\Dev\nkai\sidecars\panel\index.html")
    if nkai_panel_file.parent.exists():
        try:
            nkai_panel_file.write_text(html_content, encoding="utf-8")
        except Exception as e:
            pass

    return out_path



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
        "--out",
        type=str,
        default=str(DEFAULT_OUTPUT_HTML),
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

    out_file = Path(args.out).resolve()
    generated = generate_html_inbox_board(inbox_dir, notes, docket_items, out_file, queue=queue)
    print(f"✅ Generated Morning Mail Inbox: {generated} ({len(docket_items)} docket items, {len(notes)} archive notes)")

    if not args.no_browser:
        try:
            webbrowser.open(generated.as_uri())
        except Exception as e:
            print(f"⚠️ Could not launch browser: {e}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
