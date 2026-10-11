#!/usr/bin/env python3
"""
N'kai Data-Driven Board Renderer Engine (Flight 15 / Topic 6.11 / rj-04)
Renders valid, zero-defect interactive N'kai sidecars and webviews from
declarative Pydantic BoardSchema instances and master_template.html.
Universal UTF-8 compliance guaranteed.
"""

import html
import os
from pathlib import Path
import re
import sys
from typing import Optional

# Universal UTF-8 Environment Invariant (Flight 18)
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

try:
    from tools.models import BoardSchema, MailItem, PriorityLevel, TriageStatus, PILLAR_METADATA, infer_pillars
except ImportError:
    try:
        from models import BoardSchema, MailItem, PriorityLevel, TriageStatus, PILLAR_METADATA, infer_pillars
    except ImportError:
        _script_dir = Path(__file__).resolve().parent
        if str(_script_dir) not in sys.path:
            sys.path.insert(0, str(_script_dir))
        from models import BoardSchema, MailItem, PriorityLevel, TriageStatus, PILLAR_METADATA, infer_pillars


DEFAULT_TEMPLATE_PATHS = [
    Path(r"c:\Dev\nkai\core\master_template.html"),
    Path(__file__).resolve().parent.parent / ".agents" / "sidecar" / "master_template.html",
    Path(__file__).resolve().parent.parent / "core" / "master_template.html",
    Path(r"c:\Dev\nkai\bundle\templates\master_template.html"),
]


def resolve_master_template(custom_path: Optional[Path] = None) -> Path:
    """Finds the master_template.html across local workspace or nkai core."""
    if custom_path and custom_path.exists():
        return custom_path

    for cand in DEFAULT_TEMPLATE_PATHS:
        if cand.exists():
            return cand

    raise FileNotFoundError("Could not locate master_template.html in nkai core or sidecar directory.")


def render_markdown_to_html(md_text: str) -> str:
    """Lightweight, zero-dependency Markdown to HTML parser for card notes."""
    if not md_text:
        return ""

    lines = md_text.splitlines()
    html_out = []
    in_code_block = False
    in_list = False
    code_buffer = []

    def format_inline(text: str) -> str:
        escaped = html.escape(text)
        escaped = re.sub(
            r'\[([^\]]+)\]\(([^)]+)\)',
            r'<a href="\2" target="_blank" rel="noopener noreferrer" class="note-link">\1</a>',
            escaped
        )
        escaped = re.sub(r'\*\*([^*]+)\*\*', r'<strong>\1</strong>', escaped)
        escaped = re.sub(r'__([^_]+)__', r'<strong>\1</strong>', escaped)
        escaped = re.sub(r'\*([^*]+)\*', r'<em>\1</em>', escaped)
        escaped = re.sub(r'_([^_]+)_', r'<em>\1</em>', escaped)
        escaped = re.sub(r'`([^`]+)`', r'<code class="inline-code">\1</code>', escaped)
        return escaped

    for line in lines:
        stripped = line.strip()

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

        if stripped in ["---", "***", "___"]:
            if in_list:
                html_out.append("</ul>")
                in_list = False
            html_out.append("<hr />")
            continue

        if not stripped:
            if in_list:
                html_out.append("</ul>")
                in_list = False
            continue

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

        if stripped.startswith(">"):
            if in_list:
                html_out.append("</ul>")
                in_list = False
            bq_text = format_inline(stripped.lstrip("> ").strip())
            html_out.append(f"<blockquote>{bq_text}</blockquote>")
            continue

        list_match = re.match(r'^[-*]\s+(.*)', stripped)
        if list_match:
            if not in_list:
                html_out.append("<ul>")
                in_list = True
            item_html = format_inline(list_match.group(1))
            html_out.append(f"  <li>{item_html}</li>")
            continue

        num_list_match = re.match(r'^\d+\.\s+(.*)', stripped)
        if num_list_match:
            if not in_list:
                html_out.append("<ul>")
                in_list = True
            item_html = format_inline(num_list_match.group(1))
            html_out.append(f"  <li>{item_html}</li>")
            continue

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


def render_pillar_badges(pillars: list) -> str:
    """Renders Rule of Two pillar badges with overflow count chip."""
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


def render_card_html(it: MailItem, i: int) -> str:
    """Renders a single MailItem into an accessible, deterministic DOM card."""
    idx = it.idx
    rank_val = idx if idx > 0 else i
    clean_id = re.sub(r"[^a-zA-Z0-9_-]", "_", it.id)
    title_esc = html.escape(it.title)
    source_esc = html.escape(it.source_note or it.source_file or it.file_name or "Note")
    file_esc = html.escape(it.file_name or (Path(it.source_file).name if it.source_file else "note.md"))
    date_esc = html.escape(it.created_at)
    excerpt_esc = html.escape(it.excerpt or it.preview or "")
    body_rendered = render_markdown_to_html(it.body)
    tier = it.tier
    tier_label = "Tier 1 Pro" if tier == "tier1" else ("Tier 3 Low" if tier == "tier3" else "Tier 2 Flash")
    card_id = f"mail-card-{i}"

    is_docket = it.id.startswith("docket-")
    scope = "docket" if is_docket else "archive"

    # Status resolution
    if it.status == TriageStatus.PENDING:
        status = "untriaged"
        badge_text = "⚪ UNTRIAGED"
        badge_cls = "badge-unhandled"
    elif it.status == TriageStatus.APPROVED:
        status = "approved"
        badge_text = "✅ APPROVED"
        badge_cls = "badge-approved"
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
    if status == "approved":
        dispatch_banner_html = f"""
        <div class="mail-dispatch-banner">
          <div style="background: rgba(52, 211, 153, 0.08); border: 1px solid rgba(52, 211, 153, 0.3); border-radius: 6px; padding: 10px 12px; margin: 10px 0; font-family: var(--font-mono); font-size: 11px;">
            <div style="color: #34d399; font-weight: 700; display: flex; align-items: center; justify-content: space-between; margin-bottom: 4px;">
              <span>✅ Approved for Implementation</span>
              <span style="font-size: 10px; color: var(--text-muted);">{html.escape(it.verdict_notes or '')}</span>
            </div>
            <div style="color: var(--text-muted); font-size: 10px; line-height: 1.4;">
              Approved. Scheduled into active development runway.
            </div>
          </div>
        </div>"""
    elif status == "backlog":
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
            <div style="color: var(--accent-blue); font-weight: 700; display: flex; align-items: center; justify-content: space-between; margin-bottom: 4px;">
              <span>✅ Completed & Codified</span>
              <span style="font-size: 10px; color: var(--text-muted);">{html.escape(it.verdict_notes or '')}</span>
            </div>
            <div style="color: var(--text-muted); font-size: 10px; line-height: 1.4;">
              Feature has shipped and is permanently codified in master specifications.
            </div>
          </div>
        </div>"""
    elif status == "graveyard":
        dispatch_banner_html = f"""
        <div class="mail-dispatch-banner">
          <div style="background: rgba(248, 113, 113, 0.08); border: 1px solid rgba(248, 113, 113, 0.3); border-radius: 6px; padding: 10px 12px; margin: 10px 0; font-family: var(--font-mono); font-size: 11px;">
            <div style="color: #f87171; font-weight: 700; display: flex; align-items: center; justify-content: space-between; margin-bottom: 4px;">
              <span>🪦 Archived in Strategy Graveyard</span>
              <span style="font-size: 10px; color: var(--text-muted);">{html.escape(it.verdict_notes or '')}</span>
            </div>
            <div style="color: var(--text-muted); font-size: 10px; line-height: 1.4;">
              Concept rejected with architectural rationale documented.
            </div>
          </div>
        </div>"""

    return f"""
      <div class="mail-card {'dispatched' if is_dispatched else ''} {status}" id="{card_id}" data-card-id="{clean_id}" data-title="{title_esc}" data-tier="{tier}" data-status="{status}" data-scope="{scope}" data-priority="{priority.lower()}" data-pillars="{pillars_attr}" data-index="{i}">
        <div class="mail-card-header">
          <div class="mail-header-left">
            <span class="mail-idx-badge">#{rank_val}</span>
            <span class="mail-status-badge {badge_cls}" id="badge-{card_id}">{badge_text}</span>
            {priority_badge}
            {dep_badges}
            {merged_chip}
            {docket_badge}
          </div>
          <div class="mail-header-right">
            <div class="mail-pill-group">
              {pillar_badges}
            </div>
            <span class="mail-tier-badge badge-{tier}">{tier_label}</span>
            {file_pill}
            <span class="mail-date">{date_esc}</span>
          </div>
        </div>
        <div class="mail-card-title">{title_esc}</div>
        <div class="mail-card-excerpt">{excerpt_esc}</div>
        <details class="mail-body-details">
          <summary class="mail-body-toggle">👁️ View Ingested Note & Context ({it.word_count} words)</summary>
          <div class="mail-body-rendered">{body_rendered}</div>
        </details>
        {dispatch_banner_html}
        <div class="mail-card-actions">
          <span class="actions-label">Dispatch Action:</span>
          <div class="mail-chip-group">
            {dep_chip}
            <button id="btn-approve-{clean_id}" class="action-chip chip-approve" onclick="dispatchMailAction('{card_id}', 'approve')" title="Approve for immediate implementation">
              <span>✅</span> Approve
            </button>
            <button id="btn-backlog-{clean_id}" class="action-chip chip-backlog" onclick="dispatchMailAction('{card_id}', 'backlog')" title="Add to docs/BACKLOG.md">
              <span>📥</span> Backlog
            </button>
            <button id="btn-research-{clean_id}" class="action-chip chip-research" onclick="dispatchMailAction('{card_id}', 'research')" title="Route to TheKlangResearch">
              <span>🔬</span> Research
            </button>
            <button id="btn-graveyard-{clean_id}" class="action-chip chip-graveyard" onclick="dispatchMailAction('{card_id}', 'graveyard')" title="Reject and record in Graveyard">
              <span>🪦</span> Graveyard
            </button>
            <button id="btn-pcda-{clean_id}" class="action-chip chip-pcda" onclick="dispatchMailAction('{card_id}', 'pcda')" title="Request 4-Lens PCDA Dialectic">
              <span>⚖️</span> PCDA
            </button>
          </div>
        </div>
      </div>
"""


def render_board(schema: BoardSchema, out_path: Path, template_path: Optional[Path] = None) -> Path:
    """Renders a self-contained, validated HTML board from a BoardSchema instance."""
    tmpl_file = resolve_master_template(template_path)
    template_str = tmpl_file.read_text(encoding="utf-8")

    items = schema.cards
    total_count = len(items)

    # 1. Tally Pillars across all items (Session-Stable Frequency Ranking)
    pillar_counts = {}
    for it in items:
        pils = it.target_pillars or ["TKS"]
        for p in pils:
            p_upper = p.upper()
            pillar_counts[p_upper] = pillar_counts.get(p_upper, 0) + 1

    sorted_pillars = sorted(pillar_counts.keys(), key=lambda p: (-pillar_counts[p], p))

    # Build Rail 1 Pillar buttons HTML (Deterministic IDs: pill-pillar-{slug})
    pillar_buttons = []
    for p in sorted_pillars:
        meta = PILLAR_METADATA.get(p, {"name": p, "emoji": "🏷️", "desc": p})
        emoji = meta.get("emoji", "🏷️")
        p_slug = p.lower().replace("-", "")
        cnt = pillar_counts[p]
        p_title = f"{meta['name']}: {meta.get('desc', '')}"
        pillar_buttons.append(
            f'<button class="pill-btn" id="pill-pillar-{p_slug}" onclick="applyPillarFilter(\'{p}\', event)" title="{html.escape(p_title)}">'
            f'<span class="pill-icon">{emoji}</span> {p} <span class="pill-count" id="count-pillar-{p_slug}">{cnt}</span>'
            f'</button>'
        )
    pillar_buttons_html = "\n        ".join(pillar_buttons)

    # 2. Tally Status / Triage categories across all items
    untriaged_count = sum(1 for it in items if it.status == TriageStatus.PENDING)
    approved_count = sum(1 for it in items if it.status == TriageStatus.APPROVED)
    backlog_count = sum(1 for it in items if it.status == TriageStatus.BACKLOG)
    completed_count = sum(1 for it in items if it.status == TriageStatus.COMPLETED)
    research_count = sum(1 for it in items if it.status == TriageStatus.RESEARCH)
    graveyard_count = sum(1 for it in items if it.status == TriageStatus.GRAVEYARD)
    pcda_count = sum(1 for it in items if it.status == TriageStatus.PCDA)
    p0_count = sum(1 for it in items if it.priority == PriorityLevel.P0)
    docket_count = sum(1 for it in items if it.id.startswith("docket-"))
    dispatched_count = approved_count + backlog_count + completed_count + research_count + graveyard_count + pcda_count

    # 3. Render Cards
    cards_html = [render_card_html(it, i) for i, it in enumerate(items, 1)]
    cards_joined = "".join(cards_html) if cards_html else '<div class="mail-card" style="text-align:center;color:var(--text-muted);"><p>🎉 Mailbox zero! No active notes found.</p></div>'

    # 4. Environment & Banners
    is_sandbox = (schema.environment.upper() == "SANDBOX")
    page_title = f"[SBX] {schema.title}" if is_sandbox else f"📬 {schema.title}"
    sandbox_banner = (
        '<div id="sandbox-banner" class="sandbox-banner">🧪 <strong>SANDBOX MODE</strong> — Synthetic test fixture (7 mock cards). Button clicks echo <code>[QA_TEST_MOCK]</code> with zero persistence.</div>'
        if is_sandbox else ""
    )

    # 5. Substitute Placeholders
    output_html = template_str
    output_html = output_html.replace("{{PAGE_TITLE}}", page_title)
    output_html = output_html.replace("{{SANDBOX_BANNER}}", sandbox_banner)
    output_html = output_html.replace("{{RAIL_1_BUTTONS}}", pillar_buttons_html)
    output_html = output_html.replace("{{CARDS_HTML}}", cards_joined)
    output_html = output_html.replace("{{ENVIRONMENT}}", "SANDBOX" if is_sandbox else "PRODUCTION")
    output_html = output_html.replace("{{TOTAL_COUNT}}", str(total_count))
    output_html = output_html.replace("{{UNTRIAGED_COUNT}}", str(untriaged_count))
    output_html = output_html.replace("{{APPROVED_COUNT}}", str(approved_count))
    output_html = output_html.replace("{{BACKLOG_COUNT}}", str(backlog_count))
    output_html = output_html.replace("{{COMPLETED_COUNT}}", str(completed_count))
    output_html = output_html.replace("{{RESEARCH_COUNT}}", str(research_count))
    output_html = output_html.replace("{{GRAVEYARD_COUNT}}", str(graveyard_count))
    output_html = output_html.replace("{{PCDA_COUNT}}", str(pcda_count))
    output_html = output_html.replace("{{P0_COUNT}}", str(p0_count))
    output_html = output_html.replace("{{DOCKET_COUNT}}", str(docket_count))
    output_html = output_html.replace("{{DISPATCHED_COUNT}}", str(dispatched_count))
    output_html = output_html.replace("{{ALLOWANCE_BUDGET}}", str(schema.allowance_budget))

    # Flight 14: Board Title & Injected Registry Manifest
    board_title = f"[SBX] {schema.title}" if is_sandbox else schema.title
    output_html = output_html.replace("{{BOARD_TITLE}}", html.escape(board_title))

    registry_path = Path(__file__).resolve().parent.parent / ".agents" / "pipeline" / "boards" / "registry.json"
    if not registry_path.exists():
        registry_path = Path(r"c:\Dev\nkai\boards\registry.json")
    if registry_path.exists():
        registry_json_str = registry_path.read_text(encoding="utf-8")
    else:
        registry_json_str = '{"version":"1.0.0","default_board_id":"morning_mail","boards":[]}'
    output_html = output_html.replace("{{BOARD_REGISTRY_JSON}}", registry_json_str)

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(output_html, encoding="utf-8")

    # Mirror to Antigravity Artifact directory for interactive side-panel
    artifact_dir = Path(r"C:\Users\codyg\.gemini\antigravity\brain\9241988e-1227-466e-a0a5-446d61a69d77")
    if artifact_dir.exists():
        try:
            (artifact_dir / out_path.name).write_text(output_html, encoding="utf-8")
        except Exception:
            pass

    # Mirror to active Antigravity Extension panel
    plugin_panel = Path(r"C:\Users\codyg\.gemini\config\plugins\nkai\sidecars\panel\index.html")
    if plugin_panel.parent.exists():
        try:
            plugin_panel.write_text(output_html, encoding="utf-8")
        except Exception:
            pass

    # Mirror to nkai repository sidecar panel
    nkai_panel = Path(r"c:\Dev\nkai\sidecars\panel\index.html")
    if nkai_panel.parent.exists():
        try:
            nkai_panel.write_text(output_html, encoding="utf-8")
        except Exception:
            pass

    return out_path
