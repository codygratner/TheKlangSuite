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
import urllib.parse


# Universal UTF-8 Environment Invariant (Flight 18)
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

try:
    from tools.models import BoardSchema, DossierDrawer, MailItem, PriorityLevel, TriageStatus, PILLAR_METADATA, infer_pillars
except ImportError:
    try:
        from models import BoardSchema, DossierDrawer, MailItem, PriorityLevel, TriageStatus, PILLAR_METADATA, infer_pillars
    except ImportError:
        _script_dir = Path(__file__).resolve().parent
        if str(_script_dir) not in sys.path:
            sys.path.insert(0, str(_script_dir))
        from models import BoardSchema, DossierDrawer, MailItem, PriorityLevel, TriageStatus, PILLAR_METADATA, infer_pillars


DEFAULT_TEMPLATE_PATHS = [
    Path(r"c:\Dev\nkai\core\shell.html"),
    Path(__file__).resolve().parent.parent / ".agents" / "sidecar" / "shell.html",
    Path(__file__).resolve().parent.parent / "core" / "shell.html",
    Path(r"c:\Dev\nkai\bundle\templates\shell.html"),
    Path(r"c:\Dev\nkai\core\master_template.html"),
    Path(__file__).resolve().parent.parent / ".agents" / "sidecar" / "master_template.html",
    Path(__file__).resolve().parent.parent / "core" / "master_template.html",
    Path(r"c:\Dev\nkai\bundle\templates\master_template.html"),
]

DEFAULT_OVERFLOW_PATH = Path(r".agents/pipeline/mailbag/overflow.json")


def load_overflow_map(overflow_path: Optional[Path] = None) -> dict:
    """Loads overflow.json and returns a mapping of split_group -> count."""
    p = overflow_path or DEFAULT_OVERFLOW_PATH
    if not p.exists():
        for base in [Path.cwd(), Path(__file__).resolve().parent.parent]:
            cand = base / ".agents" / "pipeline" / "mailbag" / "overflow.json"
            if cand.exists():
                p = cand
                break
    if not p.exists():
        return {}
    try:
        data = json.loads(p.read_text(encoding="utf-8"))
        counts = {}
        if isinstance(data, list):
            for it in data:
                sg = it.get("split_group") or Path(it.get("source_note", "")).stem
                if sg:
                    counts[sg] = counts.get(sg, 0) + 1
        return counts
    except Exception:
        return {}


def resolve_master_template(custom_path: Optional[Path] = None) -> Path:
    """Finds the shell.html (or fallback master_template.html) across local workspace or nkai core."""
    if custom_path and custom_path.exists():
        return custom_path

    for cand in DEFAULT_TEMPLATE_PATHS:
        if cand.exists():
            return cand

    raise FileNotFoundError("Could not locate shell.html or master_template.html in nkai core or sidecar directory.")


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


def render_dossier_drawer_html(clean_id: str, dossier: DossierDrawer) -> str:
    """Renders the 3-tab unified dossier drawer (Flight 3 / Topic 17 / rj-25)."""
    src_md = dossier.source_markdown or "No source markdown recorded."
    src_rendered = render_markdown_to_html(src_md)

    pcda = dossier.pcda_dialectic or {}
    pros = pcda.get("pros", [])
    cons = pcda.get("cons", [])
    da = pcda.get("devils_advocate", "")
    res = pcda.get("resolution", "")
    lenses = pcda.get("lenses", {})
    profile_name = pcda.get("profile_name", "")
    profile_badge = (
        f'<div style="margin-bottom: 8px;">'
        f'<span class="badge-lens-active">'
        f'🔬 Active Lens: {html.escape(profile_name)}</span></div>'
    ) if profile_name else ""

    pros_html = "".join([f"<li>✅ {html.escape(str(p))}</li>" for p in pros])
    cons_html = "".join([f"<li>⚠️ {html.escape(str(c))}</li>" for c in cons])

    lenses_html = ""
    if lenses:
        lenses_items = []
        for lk, lv in lenses.items():
            lenses_items.append(
                f'<div class="lens-item-card">'
                f'<div class="lens-item-title">{html.escape(lk.replace("_", " ").title())}</div>'
                f'<div class="lens-item-desc">{html.escape(str(lv))}</div></div>'
            )
        lenses_html = f'<div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(180px, 1fr)); gap: 8px; margin-top: 10px;">{"".join(lenses_items)}</div>'

    rd = dossier.research_dossier or {}
    citations = rd.get("citations", [])
    precedents = rd.get("precedents", [])
    gotchas = rd.get("gotchas", [])

    c_html = "".join([
        f'<li><strong>{html.escape(str(c.get("title", "")))}</strong> ({html.escape(str(c.get("source", "")))})</li>'
        for c in citations if isinstance(c, dict)
    ])
    p_html = "".join([f"<li>🏛️ {html.escape(str(p))}</li>" for p in precedents])
    g_html = "".join([f"<li>⚡ {html.escape(str(g))}</li>" for g in gotchas])

    da_html = f'<div style="background: rgba(239, 68, 68, 0.08); border-left: 2px solid #ef4444; padding: 8px 10px; border-radius: 4px; margin-bottom: 8px;"><strong style="color: #ef4444;">🔥 Devil\'s Advocate:</strong><p style="margin: 4px 0 0 0;">{html.escape(da)}</p></div>' if da else ""
    res_html = f'<div style="background: rgba(139, 92, 246, 0.08); border-left: 2px solid #8b5cf6; padding: 8px 10px; border-radius: 4px; margin-bottom: 8px;"><strong style="color: #a78bfa;">⚖️ Sleepy Architecture Resolution:</strong><p style="margin: 4px 0 0 0;">{html.escape(res)}</p></div>' if res else ""

    citations_block = f'<div style="margin-bottom: 8px;"><strong style="color: #38bdf8;">Citations & Benchmarks:</strong><ul style="margin: 4px 0 0 16px; padding: 0;">{c_html}</ul></div>' if c_html else ""
    precedents_block = f'<div style="margin-bottom: 8px;"><strong style="color: #38bdf8;">Upstream SOTA Precedents:</strong><ul style="margin: 4px 0 0 16px; padding: 0;">{p_html}</ul></div>' if p_html else ""
    gotchas_block = f'<div style="margin-bottom: 8px;"><strong style="color: #fbbf24;">Platform Gotchas:</strong><ul style="margin: 4px 0 0 16px; padding: 0;">{g_html}</ul></div>' if g_html else ""

    return f"""
    <div class="card-drawer-container mail-dossier-drawer" id="dossier-container-{clean_id}" style="margin-top: 8px;">
      <button class="card-drawer-toggle dossier-toggle" id="btn-dossier-{clean_id}" onclick="toggleDossierDrawer('{clean_id}')" aria-expanded="false" style="background: rgba(56, 189, 248, 0.08); border-color: rgba(56, 189, 248, 0.3);">
        <span class="drawer-toggle-icon">📂</span>
        <span class="drawer-toggle-label" style="color: #38bdf8; font-weight: 600;">View Full Dossier (3-Tab Bounded Research)</span>
        <span class="drawer-toggle-caret">▾</span>
      </button>
      <div class="card-drawer-content dossier-content" id="dossier-{clean_id}" hidden style="padding: 12px; background: rgba(15, 23, 42, 0.7); border: 1px solid rgba(56, 189, 248, 0.2); border-radius: 6px; margin-top: 6px;">
        <div class="dossier-tab-strip" style="display: flex; gap: 8px; border-bottom: 1px solid rgba(255,255,255,0.1); padding-bottom: 8px; margin-bottom: 12px;">
          <button class="action-chip active" id="tab-src-{clean_id}" onclick="switchDossierTab('{clean_id}', 'source', event)" style="cursor: pointer;">📄 Source Note</button>
          <button class="action-chip" id="tab-pcda-{clean_id}" onclick="switchDossierTab('{clean_id}', 'pcda', event)" style="cursor: pointer;">⚖️ PCDA Dialectic</button>
          <button class="action-chip" id="tab-res-{clean_id}" onclick="switchDossierTab('{clean_id}', 'research', event)" style="cursor: pointer;">🔬 Research Dossier</button>
        </div>
        <div class="dossier-pane" id="pane-{clean_id}-source">
          <div class="markdown-body">{src_rendered}</div>
        </div>
        <div class="dossier-pane" id="pane-{clean_id}-pcda" style="display: none;">
          {profile_badge}
          <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 12px; margin-bottom: 10px;">
            <div style="background: rgba(16, 185, 129, 0.08); border-left: 2px solid #10b981; padding: 8px 10px; border-radius: 4px;">
              <strong style="color: #10b981;">Pros & Capabilities:</strong>
              <ul style="margin: 4px 0 0 16px; padding: 0;">{pros_html}</ul>
            </div>
            <div style="background: rgba(245, 158, 11, 0.08); border-left: 2px solid #f59e0b; padding: 8px 10px; border-radius: 4px;">
              <strong style="color: #f59e0b;">Cons & Friction:</strong>
              <ul style="margin: 4px 0 0 16px; padding: 0;">{cons_html}</ul>
            </div>
          </div>
          {da_html}
          {res_html}
          {lenses_html}
        </div>
        <div class="dossier-pane" id="pane-{clean_id}-research" style="display: none;">
          {citations_block}
          {precedents_block}
          {gotchas_block}
        </div>
      </div>
    </div>"""


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


def render_card_html(it: MailItem, i: int, overflow_map: Optional[dict] = None) -> str:
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
    tags_lower = [t.lower().lstrip("#") for t in (it.tags or [])]
    fm_dict = getattr(it, "frontmatter", {}) if isinstance(getattr(it, "frontmatter", {}), dict) else {}
    fm_status = str(fm_dict.get("status", "")).lower().strip()
    is_tabled_item = (it.status == TriageStatus.TABLED or "tabled" in tags_lower or getattr(it, "is_tabled", False) or fm_status == "tabled")
    is_staged_item = (it.status == TriageStatus.STAGED or fm_status == "staged")

    effective_status = it.status
    if is_tabled_item:
        effective_status = TriageStatus.TABLED
    elif is_staged_item:
        effective_status = TriageStatus.STAGED
    elif it.status == TriageStatus.PENDING:
        effective_status = TriageStatus.PENDING

    status_map = {
        TriageStatus.PENDING: ("untriaged", "⚪ UNTRIAGED", "badge-status-untriaged"),
        TriageStatus.APPROVED: ("approved", "⚡ APPROVED", "badge-status-approved"),
        TriageStatus.BACKLOG: ("backlog", "📥 BACKLOG", "badge-status-backlog"),
        TriageStatus.COMPLETED: ("completed", "✅ COMPLETED", "badge-status-completed"),
        TriageStatus.GRAVEYARD: ("graveyard", "🪦 GRAVEYARD", "badge-status-graveyard"),
        TriageStatus.PCDA: ("pcda", "⚖️ PCDA", "badge-status-pcda"),
        TriageStatus.RESEARCH: ("research", "🔬 RESEARCH", "badge-status-research"),
        TriageStatus.TABLED: ("tabled", "🗂️ TABLED", "badge-status-tabled"),
        TriageStatus.INVESTIGATING: ("investigating", "🤖 PROBING", "badge-status-investigating"),
        TriageStatus.STAGED: ("staged", "📦 STAGED", "badge-status-staged"),
    }
    status, badge_text, badge_cls = status_map.get(
        effective_status,
        (effective_status.value.lower(), f"⚡ {effective_status.value.upper()}", "badge-status-untriaged")
    )

    is_dispatched = (status != "untriaged")
    priority = (it.priority.value if hasattr(it.priority, "value") else str(it.priority)).upper()
    pri_key = priority.lower()
    pri_icons = {"p0": "🚨", "p1": "🟠", "p2": "🟡", "p3": "⚪"}
    pri_icon = pri_icons.get(pri_key, "🟡")
    priority_badge = f'<span class="badge badge-{pri_key}">{pri_icon} {priority}</span>'

    # Attention tags (Phase 4)
    attention_badges = []
    if "important" in tags_lower:
        attention_badges.append('<span class="badge badge-attention badge-attention-important">⚠️ IMPORTANT</span>')
    if "bloated" in tags_lower:
        attention_badges.append('<span class="badge badge-attention badge-attention-bloated">📦 BLOATED (needs split)</span>')
    if "incomplete" in tags_lower:
        attention_badges.append('<span class="badge badge-attention badge-attention-incomplete">📝 INCOMPLETE (early draft)</span>')
    if "spike" in tags_lower:
        attention_badges.append('<span class="badge badge-attention badge-attention-spike">⚡ SPIKE</span>')
    attention_badges_html = "".join(attention_badges)

    # Quick Note chip (Phase 2)
    quicknote_chip = '<span class="badge badge-quicknote" title="Quick Note (Filename only)">📝 quick-note</span>' if "quick-note" in tags_lower else ""

    # Tabled chip (Phase 3)
    tabled_chip = '<span class="badge badge-status-tabled" title="Agent-authored note tabled in Inbox/Tabled/">🗂 Tabled by agent</span>' if "tabled" in tags_lower else ""

    # Staged chip (Phase 3 Staging Shelf) - only if not already primary status
    staged_chip = '<span class="badge badge-status-staged" title="Note held on Inbox/Staging workbench">📦 Staged</span>' if ("staged" in tags_lower and status != "staged") else ""

    # Overflow chip (Phase 1)
    overflow_chip = ""
    overflow_count = 0
    if overflow_map:
        sg = getattr(it, "split_group", "") or Path(it.source_file or it.source_note or it.file_name or "").stem
        overflow_count = overflow_map.get(sg, 0)
        if overflow_count > 0:
            overflow_chip = f'<span class="badge badge-overflow" title="{overflow_count} ideas in overflow queue">⤵ {overflow_count} in overflow</span>'

    has_overflow = (overflow_count > 0)

    deps = it.dependencies or []
    dep_badges = "".join([f'<span class="badge badge-p1">⚠️ Prereq: {html.escape(d)}</span>' for d in deps])
    dep_chip = f'<button class="action-chip chip-prereq" style="color: #fbbf24; border-color: rgba(251, 191, 36, 0.4);" onclick="filterByPrereq(this)" data-prereq="{html.escape(deps[0], quote=True)}" title="Triage prerequisite first"><span>💡</span> Triage Prereq First</button>' if deps else ""

    merged_chip = ""
    if getattr(it, "merged_count", 0) > 1:
        merged_chip = f'<span class="badge badge-merged" title="Merged from {it.merged_count} notes">🧩 merged from {it.merged_count} notes</span>'

    pillars = it.target_pillars or infer_pillars(it.title, it.tags, it.body)
    pillar_badges_list = []
    for pil in pillars:
        p_info = PILLAR_METADATA.get(pil, {})
        p_icon = p_info.get("icon", "🏷️")
        pillar_badges_list.append(f'<span class="badge badge-pillar" onclick="applyPillarFilter(\'{pil}\', event)" title="Filter by {pil}">{p_icon} {pil}</span>')
    pillar_badges = "".join(pillar_badges_list)
    pillars_attr = html.escape(",".join(pillars), quote=True)

    tier_icons = {"tier1": "💎", "tier2": "⚡", "tier3": "🍃"}
    tier_labels = {"tier1": "Tier 1 Pro", "tier2": "Tier 2 Flash", "tier3": "Tier 3 Lite"}
    t_icon = tier_icons.get(tier, "⚡")
    t_label = tier_labels.get(tier, "Tier 2 Flash")
    tier_badge_html = f'<span class="badge badge-tier-{tier}">{t_icon} {t_label}</span>'

    docket_badge = f'<span class="badge badge-docket" title="From Curated Morning Mail Docket">⭐ DOCKET #{idx}</span>' if is_docket else ""

    file_pill_text = source_esc if is_docket else file_esc
    file_pill_icon = "📌" if is_docket else "📄"
    file_pill_esc = html.escape(file_pill_text)
    file_pill_display = file_pill_esc[:42] + ("..." if len(file_pill_esc) > 42 else "")
    file_pill = f'<span class="file-pill" title="{file_pill_esc}">{file_pill_icon} {file_pill_display}</span>'

    dispatch_banner_html = ""
    if status == "approved":
        dispatch_banner_html = f"""
        <div class="mail-dispatch-banner">
          <div class="dispatch-banner-box banner-approved">
            <div class="dispatch-banner-header">
              <span>✅ Approved for Implementation</span>
              <span class="dispatch-banner-notes">{html.escape(it.verdict_notes or '')}</span>
            </div>
            <div class="dispatch-banner-body">
              Approved. Scheduled into active development runway.
            </div>
          </div>
        </div>"""
    elif status == "backlog":
        dispatch_banner_html = f"""
        <div class="mail-dispatch-banner">
          <div class="dispatch-banner-box banner-backlog">
            <div class="dispatch-banner-header">
              <span>📥 Cataloged in Backlog</span>
              <span class="dispatch-banner-notes">{html.escape(it.verdict_notes or '')}</span>
            </div>
            <div class="dispatch-banner-body">
              Documented in <strong>docs/BACKLOG.md</strong>.
            </div>
          </div>
        </div>"""
    elif status == "completed":
        dispatch_banner_html = f"""
        <div class="mail-dispatch-banner">
          <div class="dispatch-banner-box banner-completed">
            <div class="dispatch-banner-header">
              <span>✅ Completed & Codified</span>
              <span class="dispatch-banner-notes">{html.escape(it.verdict_notes or '')}</span>
            </div>
            <div class="dispatch-banner-body">
              Feature has shipped and is permanently codified in master specifications.
            </div>
          </div>
        </div>"""
    elif status == "graveyard":
        dispatch_banner_html = f"""
        <div class="mail-dispatch-banner">
          <div class="dispatch-banner-box banner-graveyard">
            <div class="dispatch-banner-header">
              <span>🪦 Archived in Strategy Graveyard</span>
              <span class="dispatch-banner-notes">{html.escape(it.verdict_notes or '')}</span>
            </div>
            <div class="dispatch-banner-body">
              Concept rejected with architectural rationale documented.
            </div>
          </div>
        </div>"""

    # Per-Card Response Drawer (Flight 19 / rj-08)
    drawer_html = ""
    reply_badge = ""
    has_reply = bool(it.agent_reply and it.agent_reply.strip())
    if has_reply:
        reply_rendered = render_markdown_to_html(it.agent_reply.strip())
        drawer_html = f"""
        <div class="card-drawer-container" id="drawer-container-{clean_id}">
          <button class="card-drawer-toggle" id="btn-drawer-{clean_id}" onclick="toggleResponseDrawer('{clean_id}')" aria-expanded="false">
            <span class="drawer-toggle-icon">💬</span>
            <span class="drawer-toggle-label">Agent Assessment & Rationale</span>
            <span class="drawer-toggle-caret">▾</span>
          </button>
          <div class="card-drawer-content" id="drawer-{clean_id}" hidden>
            <div class="drawer-inner markdown-body">
              {reply_rendered}
            </div>
          </div>
        </div>"""
        reply_badge = '<span class="badge badge-reply" title="Agent assessment attached">💬 Agent Reply</span>'

    # Multi-Drawer Dossier (Flight 3 / Topic 17 / rj-25)
    dossier_html = ""
    dossier_badge = ""
    if getattr(it, "dossier", None):
        dossier_html = render_dossier_drawer_html(clean_id, it.dossier)
        dossier_badge = '<span class="badge badge-dossier" title="2-Pass Bounded Research Dossier Available">🔬 Dossier Ready</span>'

    # Card frontmatter and tag chips
    fm_obj = getattr(it, "frontmatter", {}) or {}
    display_tags = [t for t in (it.tags or []) if t.lower() not in ("quick-note", "tabled", "staged")]
    if isinstance(fm_obj.get("tags"), list):
        for t in fm_obj["tags"]:
            t_clean = str(t).strip().lstrip("#")
            if t_clean and t_clean.lower() not in ("quick-note", "tabled", "staged") and t_clean not in display_tags:
                display_tags.append(t_clean)

    tag_chips_html = ""
    if display_tags:
        tag_chips_html = "".join([
            f'<span class="mail-tag-chip" onclick="applyTagFilter(\'{html.escape(t, quote=True)}\', event)" title="Filter by #{html.escape(t)}">#{html.escape(t)}</span>'
            for t in display_tags
        ])

    fm_pills = []
    excluded_fm_keys = {"title", "tags", "tag", "body", "excerpt", "preview", "pillars", "pillar", "priority", "date", "status"}
    for k, v in fm_obj.items():
        if k.lower() not in excluded_fm_keys and v:
            v_str = ", ".join(v) if isinstance(v, list) else str(v)
            fm_pills.append(
                f'<span class="mail-fm-pill" title="{html.escape(k)}: {html.escape(v_str)}"><span class="fm-key">{html.escape(k)}:</span> {html.escape(v_str)}</span>'
            )
    fm_pills_html = "".join(fm_pills)

    meta_row_html = ""
    if tag_chips_html or fm_pills_html:
        meta_row_html = f'<div class="mail-card-meta-row">{tag_chips_html}{fm_pills_html}</div>'

    # Structured frontmatter box inside details view
    grid_items = []
    if date_esc:
        grid_items.append(f'<div class="fm-grid-item"><span class="fm-key">Date</span><span class="fm-val">{date_esc}</span></div>')
    if priority:
        grid_items.append(f'<div class="fm-grid-item"><span class="fm-key">Priority</span><span class="fm-val">{priority}</span></div>')
    if pillars:
        grid_items.append(f'<div class="fm-grid-item"><span class="fm-key">Pillars</span><span class="fm-val">{html.escape(", ".join(pillars))}</span></div>')
    if it.tags:
        tags_str = ", ".join([f"#{t}" for t in it.tags])
        grid_items.append(f'<div class="fm-grid-item"><span class="fm-key">Tags</span><span class="fm-val">{html.escape(tags_str)}</span></div>')
    for k, v in fm_obj.items():
        if k.lower() not in {"title", "date", "priority", "pillars", "pillar", "tags", "tag", "body", "excerpt", "preview", "status"} and v:
            v_str = ", ".join(v) if isinstance(v, list) else str(v)
            grid_items.append(f'<div class="fm-grid-item"><span class="fm-key">{html.escape(k.title())}</span><span class="fm-val">{html.escape(v_str)}</span></div>')

    fm_box_html = ""
    if grid_items:
        fm_box_html = f"""<div class="mail-frontmatter-box">
              <div class="mail-frontmatter-title">📋 Front Matter & Metadata</div>
              <div class="mail-frontmatter-grid">{"".join(grid_items)}</div>
            </div>"""

    is_pending = (it.status == TriageStatus.PENDING)
    initial_display = "" if is_pending else ' style="display: none;"'

    # Obsidian deep link URI
    note_name = it.file_name or (Path(it.source_file).name if it.source_file else "Note.md")
    note_clean_name = re.sub(r"\.md$", "", note_name)
    obsidian_href = f"obsidian://open?vault=Vault&file={urllib.parse.quote(note_clean_name)}"

    # PCDA Drawer Content
    pcda_badge_text = "Audit"
    if getattr(it, "dossier", None) and it.dossier.pcda_dialectic:
        pcda = it.dossier.pcda_dialectic
        pros = pcda.get("pros", [])
        cons = pcda.get("cons", [])
        da = pcda.get("devils_advocate", "")
        res = pcda.get("resolution", "")
        pcda_badge_text = "Audit Ready"

        items = []
        if pros:
            p_list = "".join([f"<li>{html.escape(str(p))}</li>" for p in pros[:2]])
            items.append(f'<div class="pcda-lens lens-thesis"><strong style="color:#34d399;">🟢 Thesis:</strong><ul style="margin:4px 0 0 16px; padding:0;">{p_list}</ul></div>')
        if cons:
            c_list = "".join([f"<li>{html.escape(str(c))}</li>" for c in cons[:2]])
            items.append(f'<div class="pcda-lens lens-friction"><strong style="color:#fbbf24;">🟡 Friction:</strong><ul style="margin:4px 0 0 16px; padding:0;">{c_list}</ul></div>')
        if da:
            items.append(f'<div class="pcda-lens lens-antithesis"><strong style="color:#f87171;">🔴 Antithesis:</strong><p style="margin:4px 0 0 0;">{html.escape(str(da))}</p></div>')
        if res:
            items.append(f'<div class="pcda-lens lens-resolution"><strong style="color:#c084fc;">🟣 80/20 Resolution:</strong><p style="margin:4px 0 0 0;">{html.escape(str(res))}</p></div>')
        pcda_content_html = f'<div class="pcda-synopsis-grid">{"".join(items)}</div>' if items else '<div class="drawer-empty-notice">PCDA recorded.</div>'
    else:
        pcda_content_html = '<div class="drawer-empty-notice">No PCDA dialectic recorded yet. Dispatch via [🤖 Subagent: PCDA Audit] or [📋 Queue: PCDA].</div>'

    # Research Drawer Content
    rsch_badge_text = "Report"
    if getattr(it, "dossier", None) and it.dossier.research_dossier:
        rd = it.dossier.research_dossier
        citations = rd.get("citations", [])
        precedents = rd.get("precedents", [])
        gotchas = rd.get("gotchas", [])
        ref_count = len(citations) + len(precedents)
        rsch_badge_text = f"{ref_count} Refs" if ref_count > 0 else "Ready"

        r_items = []
        if citations:
            c_lines = "".join([f'<li>{html.escape(str(c.get("title", "")))} ({html.escape(str(c.get("source", "")))})</li>' for c in citations[:3] if isinstance(c, dict)])
            r_items.append(f'<div style="margin-bottom:6px;"><strong style="color:#38bdf8;">Citations & Benchmarks:</strong><ul style="margin:2px 0 0 16px; padding:0;">{c_lines}</ul></div>')
        if precedents:
            p_lines = "".join([f'<li>🏛️ {html.escape(str(p))}</li>' for p in precedents[:3]])
            r_items.append(f'<div style="margin-bottom:6px;"><strong style="color:#38bdf8;">Precedents:</strong><ul style="margin:2px 0 0 16px; padding:0;">{p_lines}</ul></div>')
        if gotchas:
            g_lines = "".join([f'<li>⚡ {html.escape(str(g))}</li>' for g in gotchas[:3]])
            r_items.append(f'<div style="margin-bottom:6px;"><strong style="color:#fbbf24;">Platform Gotchas:</strong><ul style="margin:2px 0 0 16px; padding:0;">{g_lines}</ul></div>')
        rsch_content_html = "".join(r_items) if r_items else '<div class="drawer-empty-notice">Research dossier attached.</div>'
    else:
        rsch_content_html = '<div class="drawer-empty-notice">No background research harvested yet. Dispatch via [🤖 Subagent: Local Code] or [🌐 Web / SOTA].</div>'

    is_personal_realm = any(p in ("PERSONAL", "FORGE", "ABYSS") for p in pillars)
    card_realm = "abyss" if is_personal_realm else "factory"
    if is_personal_realm:
        target_realm = "dev"
        realm_btn_label = "🛠️ Send to Factory"
        realm_btn_title = "Move card to Realm Factory (Dev)"
        realm_btn_cls = "to-dev"
    else:
        target_realm = "abyss"
        realm_btn_label = "🌌 Send to Abyss"
        realm_btn_title = "Move card to Realm Abyss (Creative/Lore)"
        realm_btn_cls = "to-personal"

    domain_btn_html = (
        f'<button id="btn-domain-toggle-{clean_id}" class="domain-toggle-btn {realm_btn_cls}" '
        f'onclick="toggleCardDomain(\'{card_id}\', \'{clean_id}\', \'{target_realm}\', event)" '
        f'title="{realm_btn_title}">'
        f'<span>{realm_btn_label}</span>'
        f'</button>'
    )

    summary_text = render_markdown_to_html(it.excerpt) if it.excerpt else (render_markdown_to_html(it.preview) if it.preview else '<p>No summary synthesized.</p>')

    return f"""
      <div class="mail-card {'dispatched' if is_dispatched else ''} {status}"{initial_display} id="{card_id}" data-card-id="{clean_id}" data-realm="{card_realm}" data-title="{title_esc}" data-tier="{tier}" data-status="{status}" data-scope="{scope}" data-priority="{priority.lower()}" data-pillars="{pillars_attr}" data-index="{i}" data-has-overflow="{str(has_overflow).lower()}" data-overflow-count="{overflow_count}" data-has-reply="{str(has_reply).lower()}">
        
        <!-- ROW 1: [Large #N] [Pin Button] [Date] ..................... [Domain Toggle] [Isolated Reload] -->
        <div class="header-row-1">
          <span class="card-idx-large">#{rank_val}</span>
          <button id="btn-pin-{clean_id}" class="card-pin-btn" onclick="togglePinCard('{card_id}', event)" title="Pin card to focus tray">
            <span class="pin-icon">📌</span>
          </button>
          <span class="date-badge-inline">{date_esc}</span>
          {domain_btn_html}
          <button id="btn-card-refresh-{clean_id}" class="reload-btn-isolated" onclick="refreshSingleCard('{card_id}', event)" title="Refresh card status">
            <svg viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="currentColor" stroke-width="2.3" stroke-linecap="round" stroke-linejoin="round">
              <path d="M21.5 2v6h-6M21.34 15.57a10 10 0 1 1-.57-8.38l5.67-5.67"/>
            </svg>
          </button>
        </div>

        <!-- ROW 2: PUNCHY TITLE (100% HEAVY LIFTING ON CARD FACE) -->
        <div class="header-row-2">
          <div class="card-title-heavy">{title_esc}</div>
          {meta_row_html}
        </div>

        <!-- ROW 3: UNIFIED METADATA RIBBON -->
        <div class="header-row-3">
          <span class="badge {badge_cls}" id="badge-{card_id}">{badge_text}</span>
          {priority_badge}
          {pillar_badges}
          {tier_badge_html}
          {file_pill}
          {attention_badges_html}
          {dep_badges}
          {merged_chip}
          {quicknote_chip}
          {tabled_chip}
          {staged_chip}
          {overflow_chip}
          {docket_badge}
        </div>

        <!-- ZERO BODY EXCERPT ON CARD FACE: STRAIGHT INTO THE 4-TAB DRAWER -->
        <div class="tabbed-drawer">
          <div class="drawer-tab-grid" id="drawer-tabs-{clean_id}">
            <button class="drawer-tab-btn" id="tab-btn-{clean_id}-orig" onclick="switchCardDrawerTab('{clean_id}', 'orig')">
              <span class="tab-icon">✍️</span>
              <span class="tab-label">Original</span>
              <span class="tab-badge-count">Raw Note</span>
            </button>
            <button class="drawer-tab-btn active" id="tab-btn-{clean_id}-summ" onclick="switchCardDrawerTab('{clean_id}', 'summ')">
              <span class="tab-icon">🎯</span>
              <span class="tab-label">Summary</span>
              <span class="tab-badge-count">Pitch</span>
            </button>
            <button class="drawer-tab-btn" id="tab-btn-{clean_id}-pcda" onclick="switchCardDrawerTab('{clean_id}', 'pcda')">
              <span class="tab-icon">⚖️</span>
              <span class="tab-label">PCDA</span>
              <span class="tab-badge-count">{pcda_badge_text}</span>
            </button>
            <button class="drawer-tab-btn" id="tab-btn-{clean_id}-rsch" onclick="switchCardDrawerTab('{clean_id}', 'rsch')">
              <span class="tab-icon">🔬</span>
              <span class="tab-label">Research</span>
              <span class="tab-badge-count">{rsch_badge_text}</span>
            </button>
          </div>

          <div class="drawer-panes-wrapper" id="drawer-panes-wrapper-{clean_id}">
            <!-- Tab 1: Original Raw Note -->
            <div class="drawer-content-pane" id="pane-{clean_id}-orig" style="display: none;">
              <div class="raw-note-box">
                <div class="raw-meta-header">
                  <span><strong>Note:</strong> <code>{file_esc}</code></span>
                  <span style="display: flex; align-items: center; gap: 6px;">
                    <span>Obsidian Inbox</span>
                    <a href="{obsidian_href}" class="obsidian-link" target="_blank" title="Open in Obsidian desktop app">Open in Obsidian ↗</a>
                  </span>
                </div>
                <div class="markdown-body">
                  {fm_box_html}
                  {body_rendered if body_rendered else excerpt_esc}
                </div>
              </div>
              <div class="tag-container-tray">
                <span class="tag-tray-label">TAGS:</span>
                {tag_chips_html}
              </div>
            </div>

            <!-- Tab 2: Summary Pitch (Active on Load) -->
            <div class="drawer-content-pane" id="pane-{clean_id}-summ" style="display: block;">
              <div class="summary-box">
                <div class="summary-box-title">🎯 Executive Pitch & Synthesis</div>
                {summary_text}
              </div>
            </div>

            <!-- Tab 3: PCDA Dialectic -->
            <div class="drawer-content-pane" id="pane-{clean_id}-pcda" style="display: none;">
              {pcda_content_html}
            </div>

            <!-- Tab 4: Research Harvest -->
            <div class="drawer-content-pane" id="pane-{clean_id}-rsch" style="display: none;">
              {rsch_content_html}
            </div>
          </div>
        </div>

        {dispatch_banner_html}

        <!-- PRIMARY ACTION SECTION: TERMINAL DISPATCHES (3 BUTTONS) -->
        <div class="action-deck" id="dispatch-section-{clean_id}">
          <div class="eyebrow-label">
            Terminal Dispatch &middot; Clears from Mailbag
          </div>
          <div class="grid-dispatch-adaptive">
            <button id="btn-approve-{clean_id}" class="btn-dispatch box-approve" onclick="dispatchMailAction('{card_id}', 'approve')" title="Approve for immediate runway build">
              <span class="btn-icon">✅</span>
              <span class="btn-label">Approve</span>
              <span class="btn-sublabel">(Runway)</span>
            </button>
            <button id="btn-backlog-{clean_id}" class="btn-dispatch box-backlog" onclick="dispatchMailAction('{card_id}', 'backlog')" title="Add to docs/BACKLOG.md">
              <span class="btn-icon">📥</span>
              <span class="btn-label">Backlog</span>
              <span class="btn-sublabel">(Roadmap)</span>
            </button>
            <button id="btn-graveyard-{clean_id}" class="btn-dispatch box-graveyard" onclick="dispatchMailAction('{card_id}', 'graveyard')" title="Reject concept & archive to Strategy Graveyard">
              <span class="btn-icon">🪦</span>
              <span class="btn-label">Graveyard</span>
              <span class="btn-sublabel">(Kill)</span>
            </button>
          </div>
        </div>

        <!-- SECONDARY ACTION SECTION: INVESTIGATION & STAGING (3 BUTTONS) -->
        <div class="action-deck" id="session-section-{clean_id}">
          <div class="eyebrow-label">
            Investigate & Stage &middot; Keeps in Flight
          </div>
          <div class="grid-session-adaptive">
            <button class="btn-session box-subagent" id="btn-action-subagent-{clean_id}" onclick="toggleActionTray('{clean_id}', 'subagent')">
              <span class="btn-icon">🤖</span>
              <span class="btn-label">Subagent</span>
              <span class="btn-sublabel">(Probe)</span>
            </button>
            <button class="btn-session box-queue" id="btn-action-queue-{clean_id}" onclick="toggleActionTray('{clean_id}', 'queue')">
              <span class="btn-icon">📋</span>
              <span class="btn-label">Queue</span>
              <span class="btn-sublabel">(Session)</span>
            </button>
            <button class="btn-session box-table" id="btn-action-table-{clean_id}" onclick="toggleActionTray('{clean_id}', 'table')">
              <span class="btn-icon">⏳</span>
              <span class="btn-label">Table</span>
              <span class="btn-sublabel">(Hold)</span>
            </button>
          </div>

          <!-- Micro-Tray: Subagent Probe -->
          <div class="micro-tray" id="tray-subagent-{clean_id}" style="display: none;">
            <div class="micro-tray-header">
              <span>🤖 <strong>Subagent Probe:</strong> Slide card to bottom of queue while agent works</span>
              <button onclick="toggleActionTray('{clean_id}', 'none')" style="background:none; border:none; color:#94a3b8; cursor:pointer;">✕</button>
            </div>
            <div class="micro-tray-grid-4">
              <button class="micro-grid-btn sub-btn-local" onclick="dispatchSubagentProbe('{clean_id}', 'local')">
                <span class="micro-icon">🔬</span>
                <span class="micro-label">Local Code</span>
              </button>
              <button class="micro-grid-btn sub-btn-web" onclick="dispatchSubagentProbe('{clean_id}', 'web')">
                <span class="micro-icon">🌐</span>
                <span class="micro-label">Web / SOTA</span>
              </button>
              <button class="micro-grid-btn sub-btn-pcda" onclick="dispatchSubagentProbe('{clean_id}', 'pcda')">
                <span class="micro-icon">⚖️</span>
                <span class="micro-label">PCDA Audit</span>
              </button>
              <button class="micro-grid-btn sub-btn-prompt" onclick="dispatchSubagentProbe('{clean_id}', 'prompt')">
                <span class="micro-icon">📦</span>
                <span class="micro-label">Ext Prompt</span>
              </button>
            </div>
          </div>

          <!-- Micro-Tray: Queue for Today's Session -->
          <div class="micro-tray" id="tray-queue-{clean_id}" style="display: none;">
            <div class="micro-tray-header">
              <span>📋 <strong>Stage for Post-Triage Session:</strong> Ready when you reach Inbox Zero</span>
              <button onclick="toggleActionTray('{clean_id}', 'none')" style="background:none; border:none; color:#94a3b8; cursor:pointer;">✕</button>
            </div>
            <div class="micro-tray-grid-4" style="margin-bottom: 6px;">
              <button class="micro-grid-btn queue-btn-grill" onclick="dispatchQueueSession('{clean_id}', 'Grill')">
                <span class="micro-icon">🔥</span>
                <span class="micro-label">Grill Me</span>
              </button>
              <button class="micro-grid-btn queue-btn-workshop" onclick="dispatchQueueSession('{clean_id}', 'Workshop')">
                <span class="micro-icon">🛠️</span>
                <span class="micro-label">Workshop</span>
              </button>
              <button class="micro-grid-btn queue-btn-pcda" onclick="dispatchQueueSession('{clean_id}', 'PCDA')">
                <span class="micro-icon">⚖️</span>
                <span class="micro-label">PCDA</span>
              </button>
              <button class="micro-grid-btn queue-btn-discuss" onclick="dispatchQueueSession('{clean_id}', 'Discuss')">
                <span class="micro-icon">💬</span>
                <span class="micro-label">Discuss</span>
              </button>
            </div>
            <div style="display: flex; gap: 6px; align-items: center; border-top: 1px dashed rgba(255,255,255,0.08); padding-top: 6px;">
              <button class="micro-grid-btn queue-btn-plan" onclick="dispatchQueueSession('{clean_id}', 'Plan')" style="flex: 1;">
                <span class="micro-icon">📐</span>
                <span class="micro-label">Plan Spec</span>
              </button>
              <button class="micro-grid-btn queue-btn-launch" onclick="dispatchImmediateChat('{clean_id}')" style="flex: 1;">
                <span class="micro-icon">🚀</span>
                <span class="micro-label">Launch Chat Now</span>
              </button>
            </div>
          </div>

          <!-- Micro-Tray: Table Gate -->
          <div class="micro-tray" id="tray-table-{clean_id}" style="display: none;">
            <div class="micro-tray-header">
              <span>⏳ <strong>Table Gate:</strong> Defer into Inbox/Tabled/</span>
              <button onclick="toggleActionTray('{clean_id}', 'none')" style="background:none; border:none; color:#94a3b8; cursor:pointer;">✕</button>
            </div>
            <div style="display: grid; grid-template-columns: repeat(3, 1fr); gap: 5px;">
              <button class="micro-grid-btn table-btn-hold" onclick="dispatchTableGate('{clean_id}', '14-Day Hold')">
                <span class="micro-icon">📅</span>
                <span class="micro-label">14-Day Hold</span>
              </button>
              <button class="micro-grid-btn table-btn-gate" onclick="dispatchTableGate('{clean_id}', 'Gate A: v0.4.1 Release')">
                <span class="micro-icon">🔒</span>
                <span class="micro-label">Gate A: v0.4.1</span>
              </button>
              <button class="micro-grid-btn table-btn-note" id="btn-trigger-custom-note-{clean_id}" onclick="toggleCustomTableInput('{clean_id}')">
                <span class="micro-icon">✍️</span>
                <span class="micro-label">Add Note</span>
              </button>
            </div>

            <!-- Inline Custom Note Input Drawer -->
            <div id="table-custom-row-{clean_id}" style="display: none; margin-top: 8px; padding-top: 6px; border-top: 1px dashed rgba(255,255,255,0.08);">
              <div style="display: flex; gap: 6px; align-items: center;">
                <input type="text" id="table-custom-input-{clean_id}" class="table-custom-input" placeholder="Type blocker or context note..." onkeydown="if(event.key==='Enter') submitCustomTable('{clean_id}')">
                <button class="micro-grid-btn table-btn-note table-custom-submit" onclick="submitCustomTable('{clean_id}')">Save & Table</button>
              </div>
            </div>
          </div>
        </div>

      </div>

"""


def render_board(schema: BoardSchema, out_path: Path, template_path: Optional[Path] = None) -> Path:
    """Renders a self-contained, validated HTML board from a BoardSchema instance."""
    tmpl_file = resolve_master_template(template_path)
    template_str = tmpl_file.read_text(encoding="utf-8")

    def resolve_effective_status(item):
        t_low = [t.lower().lstrip("#") for t in (item.tags or [])]
        fm_dict = getattr(item, "frontmatter", {}) if isinstance(getattr(item, "frontmatter", {}), dict) else {}
        fm_status = str(fm_dict.get("status", "")).lower().strip()
        if item.status == TriageStatus.TABLED or "tabled" in t_low or getattr(item, "is_tabled", False) or fm_status == "tabled":
            return TriageStatus.TABLED
        if item.status == TriageStatus.STAGED or fm_status == "staged":
            return TriageStatus.STAGED
        return item.status

    # Air Traffic Control Principle: Exclude historical completed items unless explicitly requested
    include_completed = getattr(schema, "include_completed", False) or (schema.environment.upper() == "SANDBOX")
    if not include_completed:
        items = [it for it in schema.cards if resolve_effective_status(it) != TriageStatus.COMPLETED]
    else:
        items = list(schema.cards)

    total_count = len(items)

    # Load overflow mappings for Phase 1
    overflow_map = load_overflow_map()
    total_overflow = sum(overflow_map.values())
    overflow_pill_html = (
        f'<button class="filter-chip" id="pill-status-overflow" onclick="applyStatusFilter(\'overflow\')" title="Filter cards with ideas in overflow queue">'
        f'⤵ Overflow <span class="chip-count" id="count-status-overflow">{total_overflow}</span>'
        f'</button>'
        if total_overflow > 0 else ""
    )

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

    # 2. Tally Status / Triage categories across all active items
    untriaged_count = sum(1 for it in items if resolve_effective_status(it) == TriageStatus.PENDING)
    approved_count = sum(1 for it in items if resolve_effective_status(it) == TriageStatus.APPROVED)
    backlog_count = sum(1 for it in items if resolve_effective_status(it) == TriageStatus.BACKLOG)
    completed_count = sum(1 for it in items if resolve_effective_status(it) == TriageStatus.COMPLETED)
    research_count = sum(1 for it in items if resolve_effective_status(it) in (TriageStatus.RESEARCH, TriageStatus.PCDA))
    graveyard_count = sum(1 for it in items if resolve_effective_status(it) == TriageStatus.GRAVEYARD)
    pcda_count = sum(1 for it in items if resolve_effective_status(it) == TriageStatus.PCDA)
    staged_count = sum(1 for it in items if resolve_effective_status(it) == TriageStatus.STAGED)
    tabled_count = sum(1 for it in items if resolve_effective_status(it) == TriageStatus.TABLED)
    p0_count = sum(1 for it in items if it.priority == PriorityLevel.P0)
    docket_count = sum(1 for it in items if it.id.startswith("docket-"))

    # Zone 1 vs Zone 2 tallies
    active_count = untriaged_count + staged_count + tabled_count
    dealt_with_count = approved_count + backlog_count + completed_count + research_count + graveyard_count
    dispatched_count = dealt_with_count

    # 3. Render Cards
    cards_html = [render_card_html(it, i, overflow_map) for i, it in enumerate(items, 1)]
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
    output_html = output_html.replace("{{OVERFLOW_FILTER_PILL}}", overflow_pill_html)
    output_html = output_html.replace("{{CARDS_HTML}}", cards_joined)
    output_html = output_html.replace("{{ENVIRONMENT}}", "SANDBOX" if is_sandbox else "PRODUCTION")
    output_html = output_html.replace("{{ACTIVE_COUNT}}", str(active_count))
    output_html = output_html.replace("{{DEALT_WITH_COUNT}}", str(dealt_with_count))
    output_html = output_html.replace("{{TOTAL_COUNT}}", str(total_count))
    output_html = output_html.replace("{{OVERFLOW_COUNT}}", str(total_overflow))
    output_html = output_html.replace("{{UNTRIAGED_COUNT}}", str(untriaged_count))
    output_html = output_html.replace("{{APPROVED_COUNT}}", str(approved_count))
    output_html = output_html.replace("{{BACKLOG_COUNT}}", str(backlog_count))
    output_html = output_html.replace("{{COMPLETED_COUNT}}", str(completed_count))
    output_html = output_html.replace("{{RESEARCH_COUNT}}", str(research_count))
    output_html = output_html.replace("{{GRAVEYARD_COUNT}}", str(graveyard_count))
    output_html = output_html.replace("{{PCDA_COUNT}}", str(pcda_count))
    output_html = output_html.replace("{{STAGED_COUNT}}", str(staged_count))
    output_html = output_html.replace("{{TABLED_COUNT}}", str(tabled_count))
    output_html = output_html.replace("{{P0_COUNT}}", str(p0_count))
    output_html = output_html.replace("{{DOCKET_COUNT}}", str(docket_count))
    output_html = output_html.replace("{{DISPATCHED_COUNT}}", str(dispatched_count))
    output_html = output_html.replace("{{ALLOWANCE_BUDGET}}", str(schema.allowance_budget))

    # Flight 14: Board Title & Injected Registry Manifest
    board_title = f"[SBX] {schema.title}" if is_sandbox else schema.title
    output_html = output_html.replace("{{BOARD_TITLE}}", html.escape(board_title))

    source_path_str = schema.subtitle if schema.subtitle else "TheKlangVault/Inbox"
    output_html = output_html.replace("{{SOURCE_PATH}}", html.escape(source_path_str))

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

    # Guard: Check if running inside test suite, or if out_path is a temporary / isolated test file
    is_test_env = (
        "unittest" in sys.modules
        or bool(os.environ.get("NKAI_TEST_MODE"))
        or bool(os.environ.get("PYTEST_CURRENT_TEST"))
    )

    # Check if out_path is a canonical production sidecar directory
    is_canonical_path = False
    try:
        resolved_parent = out_path.resolve().parent
        allowed_canonical_dirs = [
            (Path(__file__).resolve().parent.parent / ".agents" / "sidecar").resolve(),
            Path(r"c:\Dev\TheKlangSuite\.agents\sidecar").resolve(),
            Path(r"c:\Dev\nkai\sidecars\panel").resolve(),
        ]
        is_canonical_path = any(resolved_parent == d for d in allowed_canonical_dirs)
    except Exception:
        is_canonical_path = False

    # Mirror to Antigravity Artifact directory for interactive side-panel ONLY if canonical and not in tests
    if not is_test_env and is_canonical_path:
        artifact_dir = Path(r"C:\Users\codyg\.gemini\antigravity\brain\9241988e-1227-466e-a0a5-446d61a69d77")
        if artifact_dir.exists():
            try:
                (artifact_dir / out_path.name).write_text(output_html, encoding="utf-8")
            except Exception:
                pass

    # Mirror canonical boards to active Antigravity Extension panel and nkai repository ONLY if canonical and not in tests
    canonical_boards = {
        "morning_mail_inbox.html",
        "sandbox_mail_inbox.html",
        "audit_triage_board.html",
        "system_sanity_audit.html",
        "nkai_deep_research.html",
        "roadmap_sidecar.html",
        "shell.html",
    }
    if not is_test_env and is_canonical_path and out_path.name in canonical_boards:
        plugin_panel = Path(r"C:\Users\codyg\.gemini\config\plugins\nkai\sidecars\panel\index.html")
        if plugin_panel.parent.exists():
            try:
                (plugin_panel.parent / out_path.name).write_text(output_html, encoding="utf-8")
                if out_path.name == "morning_mail_inbox.html":
                    plugin_panel.write_text(output_html, encoding="utf-8")
            except Exception:
                pass

        nkai_panel = Path(r"c:\Dev\nkai\sidecars\panel\index.html")
        if nkai_panel.parent.exists():
            try:
                (nkai_panel.parent / out_path.name).write_text(output_html, encoding="utf-8")
                if out_path.name == "morning_mail_inbox.html":
                    nkai_panel.write_text(output_html, encoding="utf-8")
            except Exception:
                pass

    return out_path
