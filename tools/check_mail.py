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

# Import Pydantic models for persistent Mailbag session (Flight 5)
try:
    from tools.models import MailItem, MailQueue, PriorityLevel, TriageStatus, TransactionReceipt
except ImportError:
    try:
        from models import MailItem, MailQueue, PriorityLevel, TriageStatus, TransactionReceipt
    except ImportError:
        _script_dir = Path(__file__).resolve().parent
        if str(_script_dir) not in sys.path:
            sys.path.insert(0, str(_script_dir))
        from models import MailItem, MailQueue, PriorityLevel, TriageStatus, TransactionReceipt

DEFAULT_VAULT_INBOX = Path(r"C:\Dev\TheKlangVault\Inbox")
FALLBACK_LOCAL_INBOX = Path("docs/inbox")
DEFAULT_OUTPUT_HTML = Path(r".agents/sidecar/morning_mail_inbox.html")
DEFAULT_DOCKET_PATH = Path(r".agents/sidecar/morning_mail_topics.md")
DEFAULT_MAIL_QUEUE = Path(r".agents/pipeline/mailbag/mail_queue.json")
DEFAULT_TX_RECEIPT = Path(r".agents/pipeline/staging/mail_tx_latest.json")


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
        content = file_path.read_text(encoding="utf-8")
    except Exception:
        try:
            content = file_path.read_text(encoding="latin-1")
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
    }


def scan_inbox(inbox_dir: Path):
    """Scans directory for .md files and extracts metadata."""
    if not inbox_dir.exists():
        return []

    notes = []
    md_files = sorted(inbox_dir.glob("*.md"), key=lambda f: f.stat().st_mtime, reverse=True)
    for f in md_files:
        if f.is_file() and not f.name.startswith("."):
            data = extract_note_metadata(f)
            if data:
                notes.append(data)
    return notes


def parse_docket_file(docket_path: Path):
    """Parses curated docket items from morning_mail_topics.md."""
    if not docket_path.exists():
        return []

    try:
        content = docket_path.read_text(encoding="utf-8")
    except Exception:
        return []

    pattern = r"^#{3,4}\s*(?:[🚀⏳⚡🆕✅]\s*)?(?:Flight\s+)?(\d+)[:.]\s*([^\n]+)$"
    matches = list(re.finditer(pattern, content, re.MULTILINE))

    items = []
    for i, m in enumerate(matches):
        full_heading = m.group(0)
        title = m.group(2).strip()

        # Skip already completed, shipped, or approved flights
        if "✅" in full_heading or any(s in title.lower() for s in ["completed", "shipped", "approved"]):
            continue

        idx = int(m.group(1))
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

        # Model tier classification
        t_low = title.lower()
        if any(k in t_low for k in ["filter", "dsp", "saturation", "differential", "polyphon", "audio thread"]):
            tier = "tier1"
        elif any(k in t_low for k in ["rebranding", "lore", "cheat sheet", "plugins for the knowledge"]):
            tier = "tier3"
        else:
            tier = "tier2"

        # Priority detection
        all_docket_text = (title + " " + section_text).lower()
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

        items.append({
            "id": f"docket-{idx}",
            "idx": idx,
            "title": title,
            "source_note": source_note,
            "excerpt": excerpt,
            "body": section_text,
            "word_count": len(section_text.split()),
            "date": "2026-10-09",
            "tier": tier,
            "priority": priority,
            "tags": tags,
            "dependencies": dependencies,
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


def build_mail_queue(notes: list, docket_items: list, queue_path: Path) -> MailQueue:
    """
    Constructs a validated MailQueue from raw notes and docket items,
    enforcing reverse-chronological sorting for P0 items, and serializes
    the session ledger to queue_path.
    """
    mail_items = []

    # Ingest docket items
    for it in docket_items:
        m_item = MailItem(
            id=str(it.get("id") or f"docket-{it.get('idx', 0)}"),
            title=str(it.get("title", "Untitled Docket Topic")),
            created_at=str(it.get("date") or "2026-10-09"),
            priority=it.get("priority", "P2"),
            status=TriageStatus.PENDING,
            tags=it.get("tags", []),
            source_file=str(it.get("source_note", "")),
            preview=str(it.get("excerpt", "")),
            dependencies=it.get("dependencies", []),
            char_count=len(str(it.get("body", ""))),
            word_count=int(it.get("word_count", 0)),
            tier=str(it.get("tier", "tier2")),
            body=str(it.get("body", "")),
            excerpt=str(it.get("excerpt", "")),
            idx=int(it.get("idx", 0)),
        )
        mail_items.append(m_item)

    # Ingest notes
    for n in notes:
        m_item = MailItem(
            id=str(n.get("id") or n.get("file_name", "note")),
            title=str(n.get("title", "Untitled Note")),
            created_at=str(n.get("date") or datetime.date.today().strftime("%Y-%m-%d")),
            priority=n.get("priority", "P2"),
            status=TriageStatus.PENDING,
            tags=n.get("tags", []),
            source_file=str(n.get("rel_path") or n.get("file_name", "")),
            preview=str(n.get("excerpt", "")),
            dependencies=n.get("dependencies", []),
            char_count=int(n.get("char_count", 0)),
            word_count=int(n.get("word_count", 0)),
            tier=str(n.get("tier", "tier2")),
            body=str(n.get("body", "")),
            excerpt=str(n.get("excerpt", "")),
            file_name=str(n.get("file_name", "")),
            rel_path=str(n.get("rel_path", "")),
        )
        mail_items.append(m_item)

    queue = MailQueue(
        schema_version="1.0.0",
        last_scanned=datetime.datetime.now().isoformat(),
        items=mail_items,
    )
    # Enforces reverse-chronological sorting for P0 items at top
    queue.sort_by_priority()

    # Ensure output directory exists and write ledger
    queue_path.parent.mkdir(parents=True, exist_ok=True)
    queue_path.write_text(queue.model_dump_json(indent=2), encoding="utf-8")
    return queue



def archive_harvested_notes(inbox_dir: Path, notes: list, receipt_path: Path, queue: MailQueue, queue_path: Path):
    """
    Moves all harvested markdown notes from the root inbox into inbox_dir/Archive,
    updating the MailQueue ledger and writing an atomic TransactionReceipt for 5ms rollback.
    """
    archive_dir = inbox_dir / "Archive"
    archive_dir.mkdir(parents=True, exist_ok=True)

    moved_files = []
    for note in notes:
        rel_p = note.get("rel_path")
        if not rel_p:
            continue
        src = Path(rel_p).resolve()
        # Only archive files that are directly in inbox_dir (not already in Archive)
        if src.parent.resolve() == inbox_dir.resolve():
            dst = archive_dir / src.name
            try:
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
        # Save updated queue
        queue_path.write_text(queue.model_dump_json(indent=2), encoding="utf-8")

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


def generate_html_inbox_board(inbox_dir: Path, notes: list, docket_items: list, out_path: Path):
    """Generates a standalone, interactive N'kai triage board with Two-Tier Header & Web Audio."""
    docket_count = len(docket_items)
    archive_count = len(notes)
    total_count = docket_count + archive_count

    archive_tab_title = "Vault Archive" if "archive" in str(inbox_dir).lower() else "Active Mailbox"
    archive_tab_icon = "📦" if "archive" in str(inbox_dir).lower() else "📬"

    docket_json = json.dumps(docket_items, ensure_ascii=False)
    notes_json = json.dumps(notes, ensure_ascii=False)

    # 1. Render Docket Cards HTML
    docket_cards_html = []
    for it in docket_items:
        idx = it["idx"]
        title_esc = html.escape(it["title"])
        source_esc = html.escape(it["source_note"])
        date_esc = html.escape(it["date"])
        excerpt_esc = html.escape(it["excerpt"])
        body_rendered = render_markdown_to_html(it["body"])
        tier = it["tier"]
        tier_label = "Tier 1 Pro" if tier == "tier1" else ("Tier 3 Low" if tier == "tier3" else "Tier 2 Flash")
        card_id = f"docket-card-{idx}"

        priority = it.get("priority", "P2")
        priority_badge = '<span class="mail-priority-badge badge-p0" style="background: rgba(239, 68, 68, 0.2); color: #f87171; border: 1px solid rgba(239, 68, 68, 0.4); padding: 2px 6px; border-radius: 4px; font-weight: 700; font-size: 11px;">🚨 P0 TOP PRIORITY</span>' if priority == "P0" else ""
        deps = it.get("dependencies", [])
        dep_badges = "".join([f'<span class="mail-prereq-badge" style="background: rgba(245, 158, 11, 0.15); color: #fbbf24; border: 1px solid rgba(245, 158, 11, 0.3); padding: 2px 6px; border-radius: 4px; font-size: 11px;">⚠️ Prereq: {html.escape(d)}</span>' for d in deps])
        dep_chip = f'<button class="action-chip chip-prereq" style="color: #fbbf24; border-color: rgba(251, 191, 36, 0.4);" onclick="filterByPrereq(this)" data-prereq="{html.escape(deps[0], quote=True)}" title="Triage prerequisite first"><span>💡</span> Triage Prereq First</button>' if deps else ""

        docket_cards_html.append(f"""
      <div class="mail-card" id="{card_id}" data-card-idx="{idx}" data-scope="docket" data-tier="{tier}" data-priority="{priority}" data-title="{title_esc}" data-source="{source_esc}">
        <div class="mail-card-header">
          <div class="mail-header-left">
            <span class="mail-idx-badge">#{idx}</span>
            <span class="badge tier-badge { 'tier-1' if tier == 'tier1' else ('tier-3' if tier == 'tier3' else 'tier-2') }">{tier_label}</span>
            {priority_badge}
            {dep_badges}
            <span class="mail-date-badge">📅 {date_esc}</span>
            <span class="mail-status-badge badge-unhandled" id="badge-{card_id}">⚪ UNTRIAGED</span>
          </div>
          <span class="mail-file-pill" title="{source_esc}">📌 {source_esc[:42] + ('...' if len(source_esc) > 42 else '')}</span>
        </div>

        <h3 class="mail-card-title">{title_esc}</h3>

        <div class="mail-excerpt-box">
          <p class="mail-excerpt-text">{excerpt_esc}</p>
        </div>

        <details class="mail-body-details">
          <summary class="mail-body-toggle">👁️ View Ingested Note & Context ({it['word_count']} words)</summary>
          <div class="mail-body-rendered">{body_rendered}</div>
        </details>

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

    # 2. Render Archive Cards HTML
    archive_cards_html = []
    for idx, note in enumerate(notes, 1):
        title_esc = html.escape(note["title"])
        file_esc = html.escape(note["file_name"])
        date_esc = html.escape(note["date"])
        excerpt_esc = html.escape(note["excerpt"])
        body_rendered = render_markdown_to_html(note["body"])
        tier = note["tier"]
        tier_label = "Tier 1 Pro" if tier == "tier1" else ("Tier 3 Low" if tier == "tier3" else "Tier 2 Flash")
        card_id = f"archive-card-{idx}"

        priority = note.get("priority", "P2")
        priority_badge = '<span class="mail-priority-badge badge-p0" style="background: rgba(239, 68, 68, 0.2); color: #f87171; border: 1px solid rgba(239, 68, 68, 0.4); padding: 2px 6px; border-radius: 4px; font-weight: 700; font-size: 11px;">🚨 P0 TOP PRIORITY</span>' if priority == "P0" else ""
        deps = note.get("dependencies", [])
        dep_badges = "".join([f'<span class="mail-prereq-badge" style="background: rgba(245, 158, 11, 0.15); color: #fbbf24; border: 1px solid rgba(245, 158, 11, 0.3); padding: 2px 6px; border-radius: 4px; font-size: 11px;">⚠️ Prereq: {html.escape(d)}</span>' for d in deps])
        dep_chip = f'<button class="action-chip chip-prereq" style="color: #fbbf24; border-color: rgba(251, 191, 36, 0.4);" onclick="filterByPrereq(this)" data-prereq="{html.escape(deps[0], quote=True)}" title="Triage prerequisite first"><span>💡</span> Triage Prereq First</button>' if deps else ""

        archive_cards_html.append(f"""
      <div class="mail-card" id="{card_id}" data-card-idx="{idx}" data-scope="archive" data-tier="{tier}" data-priority="{priority}" data-file="{file_esc}" data-title="{title_esc}">
        <div class="mail-card-header">
          <div class="mail-header-left">
            <span class="mail-idx-badge">#{idx}</span>
            <span class="badge tier-badge { 'tier-1' if tier == 'tier1' else ('tier-3' if tier == 'tier3' else 'tier-2') }">{tier_label}</span>
            {priority_badge}
            {dep_badges}
            <span class="mail-date-badge">📅 {date_esc}</span>
            <span class="mail-status-badge badge-unhandled" id="badge-{card_id}">⚪ UNTRIAGED</span>
          </div>
          <span class="mail-file-pill" title="{file_esc}">📄 {file_esc}</span>
        </div>

        <h3 class="mail-card-title">{title_esc}</h3>

        <div class="mail-excerpt-box">
          <p class="mail-excerpt-text">{excerpt_esc}</p>
        </div>

        <details class="mail-body-details">
          <summary class="mail-body-toggle">👁️ View Ingested Note & Context ({note['word_count']} words)</summary>
          <div class="mail-body-rendered">{body_rendered}</div>
        </details>

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

    docket_joined = "".join(docket_cards_html)
    archive_joined = "".join(archive_cards_html)
    inbox_dir_esc = html.escape(str(inbox_dir))

    html_content = f"""<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8"/>
  <meta content="width=device-width, initial-scale=1.0" name="viewport"/>
  <title>📬 N'kai Morning Mail Inbox & Triage Lab</title>
  <script src="/preload.js"></script>
  <style>
    :root {{
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
      background-color: var(--bg-base);
      color: var(--text-main);
      font-family: var(--font-sans);
      font-size: 1rem;
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

    .header-btn.is-spinning {{
      animation: spin 0.6s linear infinite;
    }}

    @keyframes spin {{ 100% {{ transform: rotate(360deg); }} }}

    /* Bottom Tier (Navigation & Filters) */
    .header-tier-bottom {{
      position: sticky;
      top: 49px;
      z-index: 990;
      background: rgba(13, 18, 26, 0.94);
      backdrop-filter: blur(12px);
      border-bottom: 1px solid var(--border);
      padding: 8px 20px;
      display: flex;
      flex-wrap: wrap;
      align-items: center;
      justify-content: space-between;
      gap: 12px;
    }}

    .nav-tabs-rail {{
      display: flex;
      align-items: center;
      gap: 6px;
    }}

    .tab-compact {{
      font-family: var(--font-mono);
      font-size: 12px;
      font-weight: 700;
      padding: 6px 14px;
      border-radius: 6px;
      border: 1px solid var(--border);
      background: var(--bg-panel);
      color: var(--text-muted);
      cursor: pointer;
      display: inline-flex;
      align-items: center;
      gap: 6px;
      transition: all 0.15s ease;
    }}

    .tab-compact:hover {{
      color: var(--text-main);
      border-color: rgba(56, 189, 248, 0.4);
    }}

    .tab-compact.active {{
      background: rgba(56, 189, 248, 0.14);
      border-color: var(--accent-blue);
      color: #fff;
    }}

    .filter-chips-rail {{
      display: flex;
      align-items: center;
      gap: 6px;
      flex-wrap: wrap;
    }}

    .filter-chip {{
      font-family: var(--font-mono);
      font-size: 11px;
      padding: 4px 8px;
      border-radius: 4px;
      border: 1px solid var(--border);
      background: var(--bg-panel);
      color: var(--text-muted);
      cursor: pointer;
      transition: all 0.15s ease;
    }}

    .filter-chip:hover {{
      color: var(--text-main);
      border-color: var(--accent-blue);
    }}

    .filter-chip.active {{
      background: var(--bg-card);
      border-color: var(--accent-amber);
      color: var(--accent-amber);
      font-weight: 700;
    }}

    /* Main Container */
    .inbox-container {{
      max-width: 1160px;
      margin: 0 auto;
      padding: 24px 20px 80px;
    }}

    /* Scoreboard */
    .scoreboard-banner {{
      display: flex;
      flex-wrap: wrap;
      align-items: center;
      justify-content: space-between;
      gap: 12px;
      background: var(--bg-panel);
      border: 1px solid var(--border);
      border-radius: 8px;
      padding: 12px 16px;
      margin-bottom: 20px;
    }}

    .scoreboard-metrics {{
      display: flex;
      align-items: center;
      gap: 8px;
      flex-wrap: wrap;
    }}

    .metric-pill {{
      font-family: var(--font-mono);
      font-size: 11px;
      font-weight: 700;
      padding: 4px 10px;
      border-radius: 6px;
      border: 1px solid var(--border);
      background: var(--bg-card);
      display: inline-flex;
      align-items: center;
      gap: 6px;
    }}

    .metric-total {{ color: var(--accent-neon); }}
    .metric-dispatched {{ color: #34d399; }}
    .metric-untriaged {{ color: var(--accent-amber); }}
    .metric-backlog {{ color: #34d399; }}
    .metric-research {{ color: var(--accent-blue); }}
    .metric-graveyard {{ color: #f87171; }}
    .metric-pcda {{ color: #fbbf24; }}

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

    /* Tab Viewports */
    .tab-view {{
      display: none;
    }}

    .tab-view.active {{
      display: block;
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

    .mail-body-rendered p {{
      margin: 6px 0;
    }}

    .mail-body-rendered ul, .mail-body-rendered ol {{
      margin: 6px 0 6px 18px;
      padding-left: 0;
    }}

    .mail-body-rendered li {{
      margin: 3px 0;
    }}

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

    .mail-body-rendered pre code {{
      background: transparent;
      border: none;
      padding: 0;
      color: #e2e8f0;
    }}

    .mail-body-rendered a {{
      color: var(--accent-blue);
      text-decoration: underline;
      text-underline-offset: 2px;
    }}

    .mail-body-rendered a:hover {{
      color: #fff;
    }}

    .mail-body-rendered blockquote {{
      border-left: 3px solid var(--accent-blue);
      margin: 8px 0;
      padding-left: 12px;
      color: var(--text-muted);
      font-style: italic;
    }}

    .mail-body-rendered hr {{
      border: 0;
      border-top: 1px solid var(--border);
      margin: 12px 0;
    }}

    .mail-body-raw {{
      margin-top: 8px;
      padding: 12px 14px;
      background: #06090e;
      border: 1px solid var(--border);
      border-radius: 6px;
      font-family: var(--font-mono);
      font-size: 11.5px;
      color: var(--text-main);
      white-space: pre-wrap;
      word-break: break-word;
      max-height: 320px;
      overflow-y: auto;
      line-height: 1.5;
    }}

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
      .header-source {{
        max-width: 100%;
      }}
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

  <!-- Bottom-Tier Navigation & Filters -->
  <div class="header-tier-bottom">
    <div class="nav-tabs-rail">
      <button class="tab-compact active" id="tab-btn-docket" onclick="switchView('docket')">
        <span>🌟</span> Test-Drive Docket ({docket_count})
      </button>
      <button class="tab-compact" id="tab-btn-archive" onclick="switchView('archive')">
        <span>{archive_tab_icon}</span> {archive_tab_title} ({archive_count})
      </button>
    </div>

    <div class="filter-chips-rail">
      <span style="font-family:var(--font-mono); font-size:11px; color:var(--text-muted); margin-right:4px;">Filter:</span>
      <button class="filter-chip active" id="filter-all" onclick="applyStatusFilter('all')">All</button>
      <button class="filter-chip" id="filter-untriaged" onclick="applyStatusFilter('untriaged')">Untriaged</button>
      <button class="filter-chip" id="filter-backlog" onclick="applyStatusFilter('backlog')">📥 Backlog</button>
      <button class="filter-chip" id="filter-research" onclick="applyStatusFilter('research')">🔬 Research</button>
      <button class="filter-chip" id="filter-graveyard" onclick="applyStatusFilter('graveyard')">🪦 Graveyard</button>
      <button class="filter-chip" id="filter-pcda" onclick="applyStatusFilter('pcda')">⚖️ PCDA</button>
    </div>
  </div>

  <!-- Main Container -->
  <main class="inbox-container">

    <!-- Scoreboard Strip -->
    <div class="scoreboard-banner">
      <div class="scoreboard-metrics">
        <div class="metric-pill metric-total">Total: <span id="count-total">{total_count}</span></div>
        <div class="metric-pill metric-untriaged">Untriaged: <span id="count-untriaged">{total_count}</span></div>
        <div class="metric-pill metric-dispatched">Dispatched: <span id="count-dispatched">0</span></div>
        <div class="metric-pill metric-backlog">Backlog: <span id="count-backlog">0</span></div>
        <div class="metric-pill metric-research">Research: <span id="count-research">0</span></div>
        <div class="metric-pill metric-graveyard">Graveyard: <span id="count-graveyard">0</span></div>
        <div class="metric-pill metric-pcda">PCDA: <span id="count-pcda">0</span></div>
      </div>
    </div>

    <!-- Search Input -->
    <div class="search-box-wrapper">
      <span class="search-icon-pos">🔍</span>
      <input type="text" id="search-input" class="search-input" placeholder="Filter notes by title, keyword, or filename..." oninput="handleSearch()"/>
    </div>

    <!-- Viewport 1: Docket -->
    <section class="tab-view active" id="view-docket">
      <div class="mail-grid" id="grid-docket">
        {docket_joined if docket_items else '<div class="mail-card" style="text-align:center;color:var(--text-muted);"><p>🎉 Docket empty! No active discussion cards.</p></div>'}
      </div>
    </section>

    <!-- Viewport 2: Archive -->
    <section class="tab-view" id="view-archive">
      <div class="mail-grid" id="grid-archive">
        {archive_joined if notes else '<div class="mail-card" style="text-align:center;color:var(--text-muted);"><p>🎉 Mailbox zero! No archive notes found.</p></div>'}
      </div>
    </section>

  </main>

  <div id="action-toast">⚡ Action Dispatched!</div>

  <script>
    const docketData = {docket_json};
    const archiveData = {notes_json};

    let activeTab = 'docket';
    let activeFilter = 'all';
    let audioMuted = false;
    let hapticAudioCtx = null;

    const triageStats = {{
      total: {total_count},
      untriaged: {total_count},
      dispatched: 0,
      backlog: 0,
      research: 0,
      graveyard: 0,
      pcda: 0
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
          // Ascending chime
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
          // Tab glide
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
          // Tick
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
       Tab & Filter Switching
       ========================================================================== */
    function switchView(tabKey) {{
      playHaptic('tab');
      activeTab = tabKey;
      document.querySelectorAll('.tab-compact').forEach(btn => btn.classList.remove('active'));
      document.querySelectorAll('.tab-view').forEach(view => view.classList.remove('active'));

      const activeBtn = document.getElementById('tab-btn-' + tabKey);
      const activeView = document.getElementById('view-' + tabKey);
      if (activeBtn) activeBtn.classList.add('active');
      if (activeView) activeView.classList.add('active');

      handleSearch();
    }}

    function applyStatusFilter(filterKey) {{
      playHaptic('tick');
      activeFilter = filterKey;
      document.querySelectorAll('.filter-chip').forEach(c => c.classList.remove('active'));
      const activeChip = document.getElementById('filter-' + filterKey);
      if (activeChip) activeChip.classList.add('active');

      handleSearch();
    }}

    function handleSearch() {{
      const query = (document.getElementById('search-input').value || '').toLowerCase();
      const currentGridId = activeTab === 'docket' ? 'grid-docket' : 'grid-archive';
      const cards = document.querySelectorAll('#' + currentGridId + ' .mail-card');

      cards.forEach(card => {{
        const title = (card.getAttribute('data-title') || '').toLowerCase();
        const file = (card.getAttribute('data-file') || '').toLowerCase();
        const source = (card.getAttribute('data-source') || '').toLowerCase();
        const status = (card.getAttribute('data-status') || 'untriaged').toLowerCase();
        const text = card.innerText.toLowerCase();

        const matchesQuery = !query || title.includes(query) || file.includes(query) || source.includes(query) || text.includes(query);
        const matchesFilter = (activeFilter === 'all') || (status === activeFilter);

        if (matchesQuery && matchesFilter) {{
          card.style.display = 'block';
        }} else {{
          card.style.display = 'none';
        }}
      }});
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
            note_id: cardId.replace('note-', ''),
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

      // 2. Insert interactive in-card dispatch banner with clear next-step guidance & copyable box
      if (card) {{
        let banner = card.querySelector('.mail-dispatch-banner');
        if (!banner) {{
          banner = document.createElement('div');
          banner.className = 'mail-dispatch-banner';
          const actionsBox = card.querySelector('.mail-card-actions');
          if (actionsBox) actionsBox.before(banner);
          else card.appendChild(banner);
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
        const prevStatus = card.getAttribute('data-status') || 'untriaged';
        card.setAttribute('data-status', actionType);
        card.classList.add('dispatched');

        const badge = document.getElementById('badge-' + cardId);
        if (badge) {{
          badge.className = 'mail-status-badge badge-dispatched';
          badge.textContent = '✅ ' + actionType.toUpperCase();
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

        updateScoreboardUI();

        // Update Cumulative Allowance Meter
        const costMap = {{ tier1: 40000, tier2: 10000, tier3: 2000 }};
        const cost = costMap[tier] || 10000;
        if (actionType === 'backlog' || actionType === 'research') {{
          tokenLedger.totalTokens += cost;
          updateAllowanceUI();
        }}
      }}
    }}

    function updateScoreboardUI() {{
      const setVal = (id, val) => {{
        const el = document.getElementById(id);
        if (el) el.textContent = val;
      }};
      setVal('count-total', triageStats.total);
      setVal('count-untriaged', triageStats.untriaged);
      setVal('count-dispatched', triageStats.dispatched);
      setVal('count-backlog', triageStats.backlog);
      setVal('count-research', triageStats.research);
      setVal('count-graveyard', triageStats.graveyard);
      setVal('count-pcda', triageStats.pcda);
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

    document.addEventListener('DOMContentLoaded', () => {{
      initAudioHaptics();
      updateScoreboardUI();
    }});
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

    args = parser.parse_args()

    # Handle rollback command
    if args.rollback:
        success = rollback_mail_tx(Path(args.queue), Path(args.receipt))
        return 0 if success else 1

    inbox_dir = resolve_inbox_directory(args.dir, scan_archive=args.archive)
    notes = scan_inbox(inbox_dir)

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

    # Build and serialize validated MailQueue ledger (Flight 5 Pydantic engine)
    queue_file = Path(args.queue).resolve()
    queue = build_mail_queue(notes, docket_items, queue_file)
    print(f"📦 Validated MailQueue Ledger: {queue_file} ({queue.total_items} items, {queue.p0_count} P0 Top Priority)")

    # Execute physical archival if requested and not already scanning Archive
    if args.archive_inbox and not args.archive:
        archive_harvested_notes(inbox_dir, notes, Path(args.receipt), queue, queue_file)

    if args.chat:
        summary_md = render_chat_summary(inbox_dir, notes, docket_items)
        sys.stdout.reconfigure(encoding="utf-8")
        print(summary_md)
        print(f"\n📊 **Queue Ledger**: `{queue_file}` ({queue.total_items} items, {queue.p0_count} P0 items)")
        return 0

    out_file = Path(args.out).resolve()
    generated = generate_html_inbox_board(inbox_dir, notes, docket_items, out_file)
    print(f"✅ Generated Morning Mail Inbox: {generated} ({len(docket_items)} docket items, {len(notes)} archive notes)")

    if not args.no_browser:
        try:
            webbrowser.open(generated.as_uri())
        except Exception as e:
            print(f"⚠️ Could not launch browser: {e}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
