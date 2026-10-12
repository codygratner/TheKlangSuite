#!/usr/bin/env python3
"""
tools/build_shell_template.py — Universal Dynamic Shell Generator
================================================================
Harvests all 18 UI/UX innovations across disparate boards into a canonical
shell.html template:
- Amber Focus Pulse (@keyframes milestonePulse)
- Dynamic Progress Fill Bars
- Interactive Strike-Through Task Checklists
- Colored Decision Ribbons (APPROVED, COMPLETED, GRAVEYARD, RESEARCH)
- 2-Column Sidebar Layout with Sticky Table of Contents
- Mini Wireframe & Pod Styling (tinted underlay plates)
- Pros & Cons Dialectic Lists (✓ and ✗)
- Dynamic Font Zoom Controls (A- / 100% / A+)
- Web Audio Procedural Haptics, 44px Touch Targets, Mobile Thumb Bar
Universal UTF-8 compliance guaranteed.
"""

import sys
from pathlib import Path

# Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent

def build_shell():
    master_tmpl_path = Path(r"c:\Dev\nkai\core\master_template.html")
    if not master_tmpl_path.exists():
        master_tmpl_path = REPO_ROOT / ".agents" / "sidecar" / "master_template.html"

    tmpl = master_tmpl_path.read_text(encoding="utf-8")

    # 1. Add CSS Additions before </style>
    css_harvest = """
    /* ==========================================================================
       Amber Focus Pulse (Harvested from roadmap_sidecar.html)
       ========================================================================== */
    @keyframes milestonePulse {
      0% {
        box-shadow: 0 0 0 0 rgba(251, 191, 36, 0.7), 0 0 16px rgba(251, 191, 36, 0.4);
        border-color: var(--accent-amber);
      }
      40% {
        box-shadow: 0 0 0 8px rgba(251, 191, 36, 0.15), 0 0 28px rgba(251, 191, 36, 0.6);
        border-color: #fde68a;
      }
      100% {
        box-shadow: 0 0 0 0 rgba(251, 191, 36, 0);
        border-color: var(--accent-amber);
      }
    }
    .focus-pulse {
      animation: milestonePulse 2.4s cubic-bezier(0.16, 1, 0.3, 1) infinite !important;
    }

    /* ==========================================================================
       Dynamic Progress Fill Bars (Harvested from roadmap_sidecar.html)
       ========================================================================== */
    .progress-bar-bg {
      background: #080d16;
      border-radius: 4px;
      height: 8px;
      width: 100%;
      overflow: hidden;
      margin: 8px 0 10px 0;
      border: 1px solid var(--border);
    }
    .progress-bar-fill {
      height: 100%;
      border-radius: 4px;
      background: linear-gradient(90deg, #38bdf8, #34d399);
      transition: width 0.3s ease;
    }
    .progress-bar-fill.fill-active {
      background: linear-gradient(90deg, #38bdf8, #fbbf24);
    }

    /* ==========================================================================
       Task Checklists with Strike-Through (Harvested from roadmap_sidecar.html)
       ========================================================================== */
    .tasks-list {
      display: flex;
      flex-direction: column;
      gap: 4px;
      margin: 8px 0;
    }
    .task-item {
      display: flex;
      align-items: center;
      gap: 8px;
      font-size: 12px;
      padding: 3px 6px;
      border-radius: 4px;
      color: var(--text-main);
      cursor: pointer;
      user-select: none;
      transition: background 0.15s ease;
    }
    .task-item:hover {
      background: rgba(255, 255, 255, 0.04);
    }
    .task-item.done {
      color: var(--text-muted);
      text-decoration: line-through;
    }

    /* ==========================================================================
       Colored Decision Ribbons (Harvested from visual_grill_me.html)
       ========================================================================== */
    .card-ribbon-approved { border-left: 4px solid #34d399 !important; }
    .card-ribbon-completed { border-left: 4px solid var(--accent-blue) !important; }
    .card-ribbon-graveyard { border-left: 4px solid #f87171 !important; }
    .card-ribbon-research { border-left: 4px solid var(--accent-purple) !important; }
    .card-ribbon-backlog { border-left: 4px solid #38bdf8 !important; }

    /* ==========================================================================
       2-Column Sidebar Layout (Harvested from nkai_deep_research.html)
       ========================================================================== */
    .sidebar-layout {
      display: grid;
      grid-template-columns: 240px 1fr;
      gap: 16px;
      width: 100%;
      align-items: start;
    }
    @media (max-width: 900px) {
      .sidebar-layout { grid-template-columns: 1fr; }
      .shell-sidebar { display: none; }
    }
    .shell-sidebar {
      background: var(--bg-panel);
      border: 1px solid var(--border);
      border-radius: 8px;
      padding: 14px;
      position: sticky;
      top: 80px;
      max-height: calc(100vh - 100px);
      overflow-y: auto;
    }
    .sidebar-toc-title {
      font-family: var(--font-mono);
      font-size: 11px;
      font-weight: 700;
      color: var(--text-muted);
      text-transform: uppercase;
      margin-bottom: 8px;
      letter-spacing: 0.05em;
    }
    .sidebar-toc-link {
      display: block;
      padding: 5px 8px;
      font-size: 12px;
      color: var(--text-main);
      text-decoration: none;
      border-radius: 4px;
      transition: all 0.15s ease;
      margin-bottom: 2px;
    }
    .sidebar-toc-link:hover {
      background: rgba(56, 189, 248, 0.12);
      color: var(--accent-blue);
    }

    /* ==========================================================================
       Mini Wireframes, Pods & Pros/Cons (Harvested from ui_ux_research_board.html)
       ========================================================================== */
    .wireframe-box {
      background: #080d16;
      border: 1px solid var(--border);
      border-radius: 6px;
      padding: 10px;
      font-family: var(--font-mono);
      font-size: 10.5px;
      margin: 8px 0;
      line-height: 1.45;
      overflow-x: auto;
    }
    .pod-plate {
      border-radius: 8px;
      padding: 10px;
      margin: 8px 0;
      border: 1px solid var(--border);
    }
    .pod-cyan { background: rgba(56, 189, 248, 0.05); border-color: rgba(56, 189, 248, 0.25); }
    .pod-purple { background: rgba(168, 85, 247, 0.05); border-color: rgba(168, 85, 247, 0.25); }
    .pod-amber { background: rgba(251, 191, 36, 0.05); border-color: rgba(251, 191, 36, 0.25); }
    .pro-item { color: #34d399; display: flex; align-items: flex-start; gap: 6px; margin: 4px 0; font-size: 12px; }
    .con-item { color: #f87171; display: flex; align-items: flex-start; gap: 6px; margin: 4px 0; font-size: 12px; }

    /* ==========================================================================
       Font Zoom Controls (Harvested from visual_grill_me.html)
       ========================================================================== */
    .zoom-cluster {
      display: flex;
      align-items: center;
      border: 1px solid var(--border);
      background: var(--bg-panel);
      border-radius: 6px;
      overflow: hidden;
      font-size: 11px;
    }
    .zoom-btn {
      background: transparent;
      border: none;
      color: var(--text-muted);
      padding: 5px 8px;
      cursor: pointer;
      font-weight: 700;
      font-family: var(--font-mono);
      transition: all 0.15s ease;
    }
    .zoom-btn:hover {
      background: rgba(255, 255, 255, 0.08);
      color: var(--text-main);
    }
    .zoom-val {
      padding: 5px 6px;
      font-family: var(--font-mono);
      color: var(--accent-blue);
      font-weight: 700;
      border-left: 1px solid var(--border);
      border-right: 1px solid var(--border);
      min-width: 42px;
      text-align: center;
    }
"""

    style_end = tmpl.find("</style>")
    tmpl = tmpl[:style_end] + css_harvest + tmpl[style_end:]

    # 2. Add Zoom Cluster into .utility-cluster
    zoom_markup = """      <div class="zoom-cluster">
        <button class="zoom-btn" onclick="changeZoom(-0.1)" title="Decrease font size">A-</button>
        <span class="zoom-val" id="zoom-val">100%</span>
        <button class="zoom-btn" onclick="changeZoom(0.1)" title="Increase font size">A+</button>
      </div>"""
    util_idx = tmpl.find('<div class="utility-cluster">')
    if util_idx != -1:
        ins_pos = tmpl.find(">", util_idx) + 1
        tmpl = tmpl[:ins_pos] + "\n" + zoom_markup + tmpl[ins_pos:]

    # 3. Add JS for Zoom & Checklists
    js_harvest = """
    /* ==========================================================================
       Font Zoom & Task Checklist Controls (Harvested Innovations)
       ========================================================================== */
    let currentZoom = 1.0;
    function changeZoom(delta) {
      currentZoom = Math.min(1.4, Math.max(0.7, Math.round((currentZoom + delta) * 10) / 10));
      document.documentElement.style.setProperty('--zoom', currentZoom.toString());
      document.body.style.fontSize = `${currentZoom}rem`;
      const el = document.getElementById('zoom-val');
      if (el) el.textContent = `${Math.round(currentZoom * 100)}%`;
      localStorage.setItem('nkai_shell_zoom', currentZoom.toString());
      if (typeof playHaptic === 'function') playHaptic('tick');
    }

    function initZoom() {
      const saved = localStorage.getItem('nkai_shell_zoom');
      if (saved) {
        currentZoom = parseFloat(saved);
        document.documentElement.style.setProperty('--zoom', currentZoom.toString());
        document.body.style.fontSize = `${currentZoom}rem`;
        const el = document.getElementById('zoom-val');
        if (el) el.textContent = `${Math.round(currentZoom * 100)}%`;
      }
    }

    function toggleTaskDone(el) {
      el.classList.toggle('done');
      if (typeof playHaptic === 'function') playHaptic('tab');
    }
"""
    script_end = tmpl.rfind("</script>")
    tmpl = tmpl[:script_end] + js_harvest + tmpl[script_end:]

    # Inject initZoom call
    tmpl = tmpl.replace("initAudioHaptics();", "initAudioHaptics();\n      initZoom();")

    targets = [
        Path(r"c:\Dev\nkai\core\shell.html"),
        Path(r"c:\Dev\nkai\sidecars\panel\shell.html"),
        Path(r"c:\Dev\nkai\bundle\templates\shell.html"),
        REPO_ROOT / ".agents" / "sidecar" / "shell.html",
    ]

    for t in targets:
        t.parent.mkdir(parents=True, exist_ok=True)
        t.write_text(tmpl, encoding="utf-8")
        print(f"✅ Generated shell template: {t} ({len(tmpl)} chars)")

if __name__ == "__main__":
    build_shell()
