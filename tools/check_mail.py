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
import sys
import webbrowser

DEFAULT_VAULT_INBOX = Path(r"C:\Dev\TheKlangVault\Inbox")
FALLBACK_LOCAL_INBOX = Path("docs/inbox")
DEFAULT_OUTPUT_HTML = Path(r".agents/sidecar/morning_mail_inbox.html")
DEFAULT_DOCKET_PATH = Path(r".agents/sidecar/morning_mail_topics.md")


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

    # Determine default model tier heuristic
    title_lower = title.lower()
    if any(k in title_lower for k in ["dsp", "ladder", "filter", "polyphon", "thread", "concurren", "audio"]):
        tier = "tier1"
    elif any(k in title_lower for k in ["plugin", "lore", "doc", "format", "clean", "naming", "catalog"]):
        tier = "tier3"
    else:
        tier = "tier2"

    return {
        "file_name": file_path.name,
        "rel_path": str(file_path),
        "title": title,
        "date": date_str,
        "excerpt": excerpt,
        "body": body.strip(),
        "char_count": len(body),
        "word_count": len(body.split()),
        "tier": tier,
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

    pattern = r"^###\s+(\d+)\.\s*(?:🆕\s*)?([^\n]+)$"
    matches = list(re.finditer(pattern, content, re.MULTILINE))

    items = []
    for i, m in enumerate(matches):
        idx = int(m.group(1))
        title = m.group(2).strip()
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

        items.append({
            "idx": idx,
            "title": title,
            "source_note": source_note,
            "excerpt": excerpt,
            "body": section_text,
            "word_count": len(section_text.split()),
            "date": "2026-10-09",
            "tier": tier,
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


def generate_html_inbox_board(inbox_dir: Path, notes: list, docket_items: list, out_path: Path):
    """Generates a standalone, interactive N'kai triage board with Two-Tier Header & Web Audio."""
    docket_count = len(docket_items)
    archive_count = len(notes)
    total_count = docket_count + archive_count

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
        body_esc = html.escape(it["body"])
        tier = it["tier"]
        tier_label = "Tier 1 Pro" if tier == "tier1" else ("Tier 3 Low" if tier == "tier3" else "Tier 2 Flash")
        card_id = f"docket-card-{idx}"

        docket_cards_html.append(f"""
      <div class="mail-card" id="{card_id}" data-card-idx="{idx}" data-scope="docket" data-tier="{tier}" data-title="{title_esc}" data-source="{source_esc}">
        <div class="mail-card-header">
          <div class="mail-header-left">
            <span class="mail-idx-badge">#{idx}</span>
            <span class="badge tier-badge { 'tier-1' if tier == 'tier1' else ('tier-3' if tier == 'tier3' else 'tier-2') }">{tier_label}</span>
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
          <summary class="mail-body-toggle">👁️ View Full Analysis & PCDA Dossier ({it['word_count']} words)</summary>
          <pre class="mail-body-raw">{body_esc}</pre>
        </details>

        <div class="mail-card-actions">
          <span class="actions-label">Dispatch Action:</span>
          <div class="mail-chip-group">
            <button class="action-chip chip-backlog" onclick="dispatchMailAction('{card_id}', '{title_esc}', 'backlog', '{tier}')" title="Add to docs/BACKLOG.md">
              <span>📥</span> Backlog
            </button>
            <button class="action-chip chip-research" onclick="dispatchMailAction('{card_id}', '{title_esc}', 'research', '{tier}')" title="Route to TheKlangResearch">
              <span>🔬</span> Research
            </button>
            <button class="action-chip chip-graveyard" onclick="dispatchMailAction('{card_id}', '{title_esc}', 'graveyard', '{tier}')" title="Reject and record in Graveyard">
              <span>🪦</span> Graveyard
            </button>
            <button class="action-chip chip-pcda" onclick="dispatchMailAction('{card_id}', '{title_esc}', 'pcda', '{tier}')" title="Request 4-Lens PCDA Dialectic">
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
        body_esc = html.escape(note["body"])
        tier = note["tier"]
        tier_label = "Tier 1 Pro" if tier == "tier1" else ("Tier 3 Low" if tier == "tier3" else "Tier 2 Flash")
        card_id = f"archive-card-{idx}"

        archive_cards_html.append(f"""
      <div class="mail-card" id="{card_id}" data-card-idx="{idx}" data-scope="archive" data-tier="{tier}" data-file="{file_esc}" data-title="{title_esc}">
        <div class="mail-card-header">
          <div class="mail-header-left">
            <span class="mail-idx-badge">#{idx}</span>
            <span class="badge tier-badge { 'tier-1' if tier == 'tier1' else ('tier-3' if tier == 'tier3' else 'tier-2') }">{tier_label}</span>
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
          <summary class="mail-body-toggle">👁️ View Full Note Body ({note['word_count']} words)</summary>
          <pre class="mail-body-raw">{body_esc}</pre>
        </details>

        <div class="mail-card-actions">
          <span class="actions-label">Dispatch Action:</span>
          <div class="mail-chip-group">
            <button class="action-chip chip-backlog" onclick="dispatchMailAction('{card_id}', '{title_esc}', 'backlog', '{tier}')" title="Add to docs/BACKLOG.md">
              <span>📥</span> Backlog
            </button>
            <button class="action-chip chip-research" onclick="dispatchMailAction('{card_id}', '{title_esc}', 'research', '{tier}')" title="Route to TheKlangResearch">
              <span>🔬</span> Research
            </button>
            <button class="action-chip chip-graveyard" onclick="dispatchMailAction('{card_id}', '{title_esc}', 'graveyard', '{tier}')" title="Reject and record in Graveyard">
              <span>🪦</span> Graveyard
            </button>
            <button class="action-chip chip-pcda" onclick="dispatchMailAction('{card_id}', '{title_esc}', 'pcda', '{tier}')" title="Request 4-Lens PCDA Dialectic">
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
        <span>📦</span> Vault Archive ({archive_count})
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

    /* ==========================================================================
       Action Dispatching & Live Allowance Tracking
       ========================================================================== */
    function dispatchMailAction(cardId, title, actionType, tier) {{
      playHaptic('action');

      let promptText = '';
      if (actionType === 'backlog') {{
        promptText = `/plan feat: add '${{title}}' to docs/BACKLOG.md and create implementation roadmap`;
      }} else if (actionType === 'research') {{
        promptText = `/deep-research '${{title}}' across TheKlangResearch`;
      }} else if (actionType === 'graveyard') {{
        promptText = `Reject concept '${{title}}' and document strategic rationale in Strategy Graveyard`;
      }} else if (actionType === 'pcda') {{
        promptText = `/grill-me Conduct 4-Lens PCDA dialectic evaluation (Pros, Cons, Devil's Advocate, 80/20 Resolution) for '${{title}}'`;
      }}

      // Dispatch via environment or clipboard
      if (window.NkaiEnv && typeof window.NkaiEnv.dispatchAction === 'function') {{
        window.NkaiEnv.dispatchAction(promptText);
      }} else if (navigator.clipboard && navigator.clipboard.writeText) {{
        navigator.clipboard.writeText(promptText).catch(() => {{}});
      }}

      // Update Card UI
      const card = document.getElementById(cardId);
      if (card) {{
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

      showToast(`⚡ Dispatched [${{actionType.toUpperCase()}}]: "${{title}}"`);
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

    args = parser.parse_args()

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

    if args.chat:
        summary_md = render_chat_summary(inbox_dir, notes, docket_items)
        sys.stdout.reconfigure(encoding="utf-8")
        print(summary_md)
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
