#!/usr/bin/env python3
"""
N'kai "Check the Mail" Vault Inbox Harvester (Topic 6.11)
Discovers and extracts raw thoughts, design ideas, and inbox notes from
Obsidian Vault Inbox (or local fallback), generating an interactive N'kai triage
board (morning_mail_inbox.html) or dumping a terminal-friendly Markdown ledger (--chat).

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
    # Strip markdown headers, bold, bullets for preview
    clean_lines = []
    for line in body.splitlines():
        trimmed = line.strip()
        if not trimmed:
            continue
        if trimmed.startswith("#"):
            continue
        clean_lines.append(trimmed)

    clean_text = " ".join(clean_lines)
    clean_text = re.sub(r"\[(.*?)\]\(.*?\)", r"\1", clean_text)  # strip links
    clean_text = re.sub(r"[*_`]", "", clean_text)  # strip formatting
    clean_text = re.sub(r"\s+", " ", clean_text).strip()

    excerpt = clean_text[:280] + ("..." if len(clean_text) > 280 else "")
    if not excerpt:
        excerpt = "(Empty note body)"

    return {
        "file_name": file_path.name,
        "rel_path": str(file_path),
        "title": title,
        "date": date_str,
        "excerpt": excerpt,
        "body": body.strip(),
        "char_count": len(body),
        "word_count": len(body.split()),
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


def render_chat_summary(inbox_dir: Path, notes: list):
    """Outputs a clean, terminal-friendly Markdown summary (--chat standard)."""
    count = len(notes)
    lines = [
        f"# 📬 Morning Mail Inbox ({count} Note{'s' if count != 1 else ''} Found)",
        f"**Source:** `{inbox_dir}`",
        f"**Generated:** {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}",
        "",
    ]

    if not notes:
        lines.append("_No unhandled inbox notes found. Mailbox is completely clean!_")
        return "\n".join(lines)

    for idx, note in enumerate(notes, 1):
        lines.append(f"### {idx}. {note['title']}")
        lines.append(f"- **File:** `{note['file_name']}` | **Date:** {note['date']} | **Words:** {note['word_count']}")
        lines.append(f"- **Excerpt:** > {note['excerpt']}")
        lines.append("- **Quick Actions:**")
        lines.append(f"  - `[ 📥 Add to Backlog ]` ➔ `/plan feat: implement {note['title']}`")
        lines.append(f"  - `[ 🔬 Move to Research ]` ➔ `/deep-research \"{note['title']}\"`")
        lines.append(f"  - `[ 🪦 Reject / Graveyard ]` ➔ Document rejection rationale in `docs/BACKLOG.md`")
        lines.append(f"  - `[ ⚖️ Request PCDA ]` ➔ `/grill-me Request PCDA dialectic for \"{note['title']}\"`")
        lines.append("")

    return "\n".join(lines)


def generate_html_inbox_board(inbox_dir: Path, notes: list, out_path: Path):
    """Generates a standalone, interactive N'kai triage board."""
    count = len(notes)
    notes_json = json.dumps(notes, ensure_ascii=False)

    cards_html = []
    for idx, note in enumerate(notes, 1):
        title_esc = html.escape(note["title"])
        file_esc = html.escape(note["file_name"])
        date_esc = html.escape(note["date"])
        excerpt_esc = html.escape(note["excerpt"])
        body_esc = html.escape(note["body"])
        card_id = f"mail-card-{idx}"

        cards_html.append(f"""
      <div class="mail-card" id="{card_id}" data-card-idx="{idx}" data-file="{file_esc}" data-title="{title_esc}">
        <div class="mail-card-header">
          <div class="mail-header-left">
            <span class="mail-idx-badge">#{idx}</span>
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
            <button class="action-chip chip-backlog" onclick="dispatchMailAction('{card_id}', '{title_esc}', 'backlog')" title="Add to docs/BACKLOG.md">
              <span>📥</span> Backlog
            </button>
            <button class="action-chip chip-research" onclick="dispatchMailAction('{card_id}', '{title_esc}', 'research')" title="Route to TheKlangResearch">
              <span>🔬</span> Research
            </button>
            <button class="action-chip chip-graveyard" onclick="dispatchMailAction('{card_id}', '{title_esc}', 'graveyard')" title="Reject and record in Graveyard">
              <span>🪦</span> Graveyard
            </button>
            <button class="action-chip chip-pcda" onclick="dispatchMailAction('{card_id}', '{title_esc}', 'pcda')" title="Request 4-Lens PCDA Dialectic">
              <span>⚖️</span> PCDA
            </button>
          </div>
        </div>
      </div>
""")

    cards_joined = "".join(cards_html)
    inbox_dir_esc = html.escape(str(inbox_dir))

    html_content = f"""<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8"/>
  <meta content="width=device-width, initial-scale=1.0" name="viewport"/>
  <title>📬 N'kai Morning Mail Inbox</title>
  <style>
    :root {{
      --bg-base: #0d1117;
      --bg-panel: #161b22;
      --bg-card: #21262d;
      --border: #30363d;
      --text-main: #c9d1d9;
      --text-muted: #8b949e;
      --accent-blue: #58a6ff;
      --accent-green: #3fb950;
      --accent-red: #f85149;
      --accent-neon: #0ff;
      --accent-amber: #ffab70;
      --accent-pink: #f43f5e;
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

    /* Header */
    .inbox-header {{
      position: sticky;
      top: 0;
      z-index: 1000;
      background: rgba(13, 17, 23, 0.95);
      backdrop-filter: blur(10px);
      border-bottom: 1px solid var(--border);
      padding: 12px 20px;
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
    }}

    .inbox-title {{
      font-family: var(--font-mono);
      font-size: 14px;
      font-weight: 700;
      color: var(--accent-amber);
      display: flex;
      align-items: center;
      gap: 6px;
    }}

    .inbox-badge {{
      font-size: 11px;
      padding: 2px 8px;
      border-radius: 4px;
      background: var(--bg-card);
      border: 1px solid var(--border);
      color: var(--text-muted);
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
      max-width: 320px;
      overflow: hidden;
      text-overflow: ellipsis;
      white-space: nowrap;
    }}

    /* Scoreboard */
    .scoreboard-strip {{
      display: flex;
      align-items: center;
      gap: 8px;
    }}

    .score-item {{
      font-family: var(--font-mono);
      font-size: 11px;
      font-weight: 700;
      padding: 4px 8px;
      border-radius: 6px;
      border: 1px solid var(--border);
      background: var(--bg-panel);
    }}

    .score-total {{ color: var(--accent-neon); }}
    .score-dispatched {{ color: #34d399; }}
    .score-remaining {{ color: var(--accent-amber); }}

    /* Main Container */
    .inbox-container {{
      max-width: 1080px;
      margin: 0 auto;
      padding: 24px 20px 60px;
    }}

    /* Search & Filter Bar */
    .inbox-search-bar {{
      margin-bottom: 20px;
      display: flex;
      gap: 10px;
      align-items: center;
    }}

    .inbox-search-input {{
      flex: 1;
      background: var(--bg-panel);
      border: 1px solid var(--border);
      border-radius: 6px;
      padding: 8px 12px;
      font-size: 13px;
      color: var(--text-main);
      font-family: var(--font-sans);
      outline: none;
      transition: border-color 0.15s ease;
    }}

    .inbox-search-input:focus {{
      border-color: var(--accent-blue);
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
    }}

    .mail-card:hover {{
      border-color: rgba(88, 166, 255, 0.4);
      box-shadow: 0 4px 16px rgba(0, 0, 0, 0.3);
    }}

    .mail-card.dispatched {{
      border-color: rgba(16, 185, 129, 0.4);
      background: rgba(6, 78, 59, 0.08);
    }}

    .mail-card-header {{
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 8px;
      margin-bottom: 8px;
    }}

    .mail-header-left {{
      display: flex;
      align-items: center;
      gap: 8px;
    }}

    .mail-idx-badge {{
      font-family: var(--font-mono);
      font-size: 11px;
      font-weight: 700;
      color: var(--accent-blue);
      background: rgba(88, 166, 255, 0.1);
      padding: 2px 6px;
      border-radius: 4px;
      border: 1px solid rgba(88, 166, 255, 0.25);
    }}

    .mail-date-badge {{
      font-family: var(--font-mono);
      font-size: 11px;
      color: var(--text-muted);
    }}

    .mail-status-badge {{
      font-family: var(--font-mono);
      font-size: 10px;
      font-weight: 700;
      padding: 2px 6px;
      border-radius: 4px;
    }}

    .mail-status-badge.badge-unhandled {{
      color: #94a3b8;
      background: rgba(148, 163, 184, 0.15);
      border: 1px solid rgba(148, 163, 184, 0.25);
    }}

    .mail-status-badge.badge-dispatched {{
      color: #34d399;
      background: rgba(16, 185, 129, 0.15);
      border: 1px solid rgba(16, 185, 129, 0.3);
    }}

    .mail-file-pill {{
      font-family: var(--font-mono);
      font-size: 11px;
      color: var(--text-muted);
      background: var(--bg-card);
      padding: 2px 8px;
      border-radius: 4px;
      border: 1px solid var(--border);
    }}

    .mail-card-title {{
      font-size: 15px;
      font-weight: 700;
      color: #fff;
      margin: 0 0 10px 0;
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
      font-size: 12px;
      color: var(--text-main);
      line-height: 1.5;
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
    }}

    .mail-body-raw {{
      margin-top: 8px;
      padding: 12px;
      background: #090d13;
      border: 1px solid var(--border);
      border-radius: 6px;
      font-family: var(--font-mono);
      font-size: 11px;
      color: var(--text-main);
      white-space: pre-wrap;
      word-break: break-word;
      max-height: 240px;
      overflow-y: auto;
    }}

    .mail-card-actions {{
      display: flex;
      flex-wrap: wrap;
      align-items: center;
      gap: 10px;
      padding-top: 10px;
      border-top: 1px solid rgba(48, 54, 61, 0.6);
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
    }}

    .chip-backlog {{
      color: #34d399;
      border-color: rgba(16, 185, 129, 0.3);
    }}
    .chip-backlog:hover {{
      background: rgba(16, 185, 129, 0.15);
      border-color: #34d399;
    }}

    .chip-research {{
      color: var(--accent-blue);
      border-color: rgba(88, 166, 255, 0.3);
    }}
    .chip-research:hover {{
      background: rgba(88, 166, 255, 0.15);
      border-color: var(--accent-blue);
    }}

    .chip-graveyard {{
      color: #f87171;
      border-color: rgba(239, 68, 68, 0.3);
    }}
    .chip-graveyard:hover {{
      background: rgba(239, 68, 68, 0.15);
      border-color: #f87171;
    }}

    .chip-pcda {{
      color: #fbbf24;
      border-color: rgba(245, 158, 11, 0.3);
    }}
    .chip-pcda:hover {{
      background: rgba(245, 158, 11, 0.15);
      border-color: #fbbf24;
    }}

    /* Toast */
    #action-toast {{
      position: fixed;
      bottom: 24px;
      right: 24px;
      padding: 10px 18px;
      background: #1f2937;
      border: 1px solid var(--accent-blue);
      border-radius: 8px;
      color: #fff;
      font-family: var(--font-mono);
      font-size: 12px;
      box-shadow: 0 8px 24px rgba(0, 0, 0, 0.6);
      opacity: 0;
      pointer-events: none;
      transform: translateY(10px);
      transition: all 0.2s ease;
      z-index: 9999;
    }}

    #action-toast.show {{
      opacity: 1;
      transform: translateY(0);
    }}

    @media (max-width: 640px) {{
      .inbox-header {{
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

  <!-- Inbox Header -->
  <header class="inbox-header">
    <div class="header-left">
      <div class="inbox-title">
        <span>📬</span>
        <span>MORNING MAIL INBOX</span>
      </div>
      <span class="inbox-badge">{count} Note{'s' if count != 1 else ''}</span>
      <div class="header-source" title="{inbox_dir_esc}">📁 {inbox_dir_esc}</div>
    </div>

    <div class="scoreboard-strip">
      <div class="score-item score-total">Total: <span id="count-total">{count}</span></div>
      <div class="score-item score-dispatched">Dispatched: <span id="count-dispatched">0</span></div>
      <div class="score-item score-remaining">Untriaged: <span id="count-remaining">{count}</span></div>
    </div>
  </header>

  <!-- Container -->
  <main class="inbox-container">
    <div class="inbox-search-bar">
      <input type="text" id="search-input" class="inbox-search-input" placeholder="🔍 Filter notes by title, keyword, or filename..." oninput="filterNotes()"/>
    </div>

    <div class="mail-grid" id="mail-grid">
      {cards_joined if notes else '<div class="mail-card" style="text-align: center; color: var(--text-muted);"><p>🎉 Mailbox zero! No notes awaiting triage.</p></div>'}
    </div>
  </main>

  <div id="action-toast">Action Copied!</div>

  <script>
    const notesData = {notes_json};
    let dispatchedCount = 0;

    function filterNotes() {{
      const query = (document.getElementById('search-input').value || '').toLowerCase();
      const cards = document.querySelectorAll('.mail-card');
      cards.forEach(card => {{
        const title = (card.getAttribute('data-title') || '').toLowerCase();
        const file = (card.getAttribute('data-file') || '').toLowerCase();
        const text = card.innerText.toLowerCase();
        if (!query || title.includes(query) || file.includes(query) || text.includes(query)) {{
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
      setTimeout(() => toast.classList.remove('show'), 2200);
    }}

    function dispatchMailAction(cardId, title, actionType) {{
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

      // Dispatch to environment or clipboard
      if (window.NkaiEnv && typeof window.NkaiEnv.dispatchAction === 'function') {{
        window.NkaiEnv.dispatchAction(promptText);
      }} else if (navigator.clipboard && navigator.clipboard.writeText) {{
        navigator.clipboard.writeText(promptText).catch(() => {{}});
      }}

      // Update card UI state
      const card = document.getElementById(cardId);
      if (card && !card.classList.contains('dispatched')) {{
        card.classList.add('dispatched');
        dispatchedCount++;
        document.getElementById('count-dispatched').textContent = dispatchedCount;
        document.getElementById('count-remaining').textContent = Math.max(0, notesData.length - dispatchedCount);

        const badge = document.getElementById('badge-' + cardId);
        if (badge) {{
          badge.className = 'mail-status-badge badge-dispatched';
          badge.textContent = '✅ ' + actionType.toUpperCase();
        }}
      }}

      showToast(`Dispatched: [${{actionType.toUpperCase()}}] "${{title}}"`);
    }}
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
        description="N'kai 'Check the Mail' Vault Inbox Harvester (Topic 6.11)"
    )
    parser.add_argument(
        "--dir",
        type=str,
        default=None,
        help="Custom inbox directory to scan (default: C:\\Dev\\TheKlangVault\\Inbox or docs/inbox)"
    )
    parser.add_argument(
        "--archive",
        action="store_true",
        help="Scans Archive subfolder instead of root inbox"
    )
    parser.add_argument(
        "--chat",
        "--text-only",
        dest="chat",
        action="store_true",
        help="Outputs clean Markdown summary to terminal without opening browser"
    )
    parser.add_argument(
        "--out",
        type=str,
        default=str(DEFAULT_OUTPUT_HTML),
        help="Custom destination for generated HTML triage board"
    )
    parser.add_argument(
        "--no-browser",
        action="store_true",
        help="Do not auto-launch browser when generating HTML board"
    )

    args = parser.parse_args()

    inbox_dir = resolve_inbox_directory(args.dir, scan_archive=args.archive)
    notes = scan_inbox(inbox_dir)

    if args.chat:
        summary_md = render_chat_summary(inbox_dir, notes)
        sys.stdout.reconfigure(encoding="utf-8")
        print(summary_md)
        return 0

    out_file = Path(args.out).resolve()
    generated = generate_html_inbox_board(inbox_dir, notes, out_file)
    print(f"✅ Generated Morning Mail Inbox: {generated} ({len(notes)} notes)")

    if not args.no_browser:
        try:
            webbrowser.open(generated.as_uri())
        except Exception as e:
            print(f"⚠️ Could not launch browser: {e}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
