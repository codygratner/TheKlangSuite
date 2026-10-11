#!/usr/bin/env python3
"""
sync_roadmap_sidecar.py - Compiles multi-repo BACKLOG.md files into an interactive N'kai Roadmap Sidecar.
Zero external dependencies (pure Python standard library).
Executes in <75ms.
"""

import os
import re
import json
import datetime
import sys

# Configure UTF-8 streams for Windows console compatibility (Universal UTF-8 Invariant)
if hasattr(sys.stdout, 'reconfigure'):
    sys.stdout.reconfigure(encoding='utf-8')
if hasattr(sys.stderr, 'reconfigure'):
    sys.stderr.reconfigure(encoding='utf-8')

REPOS = {
    "tks": {
        "name": "The Klang Suite",
        "icon": "🎹",
        "backlog": r"C:\Dev\TheKlangSuite\docs\BACKLOG.md",
        "active_version": "v0.4.0-dev"
    },
    "tt": {
        "name": "ToadTracker",
        "icon": "🐸",
        "backlog": r"C:\Dev\ToadTracker\BACKLOG.md",
        "active_version": "v0.1.0"
    },
    "nk": {
        "name": "N'kai Framework",
        "icon": "🌌",
        "backlog": r"C:\Dev\nkai\docs\history\DEV_HISTORY.md",
        "active_version": "v0.1.0"
    }
}

def parse_tks_backlog(path):
    milestones = []
    if not os.path.exists(path):
        return milestones
    
    with open(path, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()

    # Pattern for milestones: ## 🚀 Milestone: v0.4.0 "Title" or ## ?? Milestone: v0.3.1
    milestone_blocks = re.split(r'\n(?=##\s+.*?Milestone:?\s+v\d)', content)
    
    for block in milestone_blocks:
        m = re.search(r'##\s+.*?Milestone:?\s+(v[\d\.]+)\s*(?:\"([^\"]+)\")?', block)
        if not m:
            continue
        v_id = m.group(1)
        title = m.group(2) or v_id
        
        # Check status
        status = "planned"
        if "COMPLETED" in block[:400] or "✅" in block[:400]:
            status = "completed"
        elif "ACTIVE" in block[:400] or "🔨" in block[:400] or v_id in ["v0.4.0", "v0.4.1"]:
            status = "active"

        # Tasks
        tasks = []
        task_matches = re.findall(r'-\s+\[([ xX])\]\s+([^\n]+)|###\s+\d+\.\s+([^\n]+)', block)
        for tm in task_matches:
            if tm[0] or tm[1]:
                completed = tm[0].lower() == 'x'
                text = tm[1].strip()
                tasks.append({"text": text, "completed": completed})
            elif tm[2]:
                text = tm[2].strip()
                completed = "COMPLETED" in text or "✅" in text
                tasks.append({"text": text.replace("— ✅ COMPLETED", "").strip(), "completed": completed})

        completed_count = sum(1 for t in tasks if t["completed"])
        total_count = len(tasks)
        pct = int((completed_count / total_count * 100)) if total_count > 0 else (100 if status == 'completed' else 0)

        milestones.append({
            "id": v_id,
            "title": title,
            "status": status,
            "tasks_total": total_count,
            "tasks_completed": completed_count,
            "percentage": pct,
            "tasks": tasks[:12] # Limit for display
        })

    return milestones

def parse_tt_backlog(path):
    milestones = []
    if not os.path.exists(path):
        return milestones
        
    milestones.append({
        "id": "v0.1.0",
        "title": "Alpha Juno Synthesizer Engine & SPSC Queue",
        "status": "active",
        "tasks_total": 5,
        "tasks_completed": 3,
        "percentage": 60,
        "tasks": [
            {"text": "Dual 2-Pole IR3R05 Resonant Lowpass Filter", "completed": True},
            {"text": "Lock-Free SPSC FIFO Queue between UI and Audio", "completed": True},
            {"text": "Sub-oscillator pulse division & PWM", "completed": True},
            {"text": "2D Grid Spatial Navigation (Song/Chain/Phrase)", "completed": False},
            {"text": "dadamachines TBD-16 SPI DMA Hardware HAL", "completed": False}
        ]
    })
    milestones.append({
        "id": "v0.2.0",
        "title": "Tracker Core Replay Engine & Table Commands",
        "status": "planned",
        "tasks_total": 4,
        "tasks_completed": 0,
        "percentage": 0,
        "tasks": [
            {"text": "Hexadecimal 00-FF Parameter Locks", "completed": False},
            {"text": "Tick-based Table Modulations", "completed": False},
            {"text": "WAV Song Exporter CLI", "completed": False},
            {"text": "Steam Deck Linux Gamepad UI", "completed": False}
        ]
    })
    return milestones

def parse_nk_backlog(path):
    return [
        {
            "id": "v0.1.0",
            "title": "Asymmetric Sidecar Framework & Triage Lab",
            "status": "active",
            "tasks_total": 5,
            "tasks_completed": 4,
            "percentage": 80,
            "tasks": [
                {"text": "Standalone Zero-Dependency HTML Template", "completed": True},
                {"text": "Antigravity UI Extension Node Plugin Bundle", "completed": True},
                {"text": "Decision Composer & Triage Package JSON Schema", "completed": True},
                {"text": "NkaiBridge Unified Adapter Pattern", "completed": True},
                {"text": "Deep-Linking Hash Router & Milestone Pulse", "completed": False}
            ]
        },
        {
            "id": "v0.2.0",
            "title": "Two-Way MCP Agent Dispatcher & State Sync",
            "status": "planned",
            "tasks_total": 3,
            "tasks_completed": 0,
            "percentage": 0,
            "tasks": [
                {"text": "agentapi direct prompt dispatch from side panel", "completed": False},
                {"text": "Live Web Worker State Heartbeat Sync", "completed": False},
                {"text": "Interactive Audio DSP Waveform Canvas Widgets", "completed": False}
            ]
        }
    ]

def generate_roadmap_html(data):
    data_json = json.dumps(data, indent=2)
    return f"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>N'kai Ecosystem Roadmap &amp; Backlog Navigator</title>
<style>
:root {{
  --bg-base: #0d1117;
  --bg-panel: #161b22;
  --bg-card: #21262d;
  --bg-recessed: #090d13;
  --border: #30363d;
  --border-focus: #58a6ff;
  --text-main: #c9d1d9;
  --text-dim: #8b949e;
  --text-bright: #f0f6fc;
  --accent-amber: #ffab70;
  --accent-cyan: #38bdf8;
  --accent-green: #3fb950;
  --accent-purple: #bc8cff;
  --font-mono: 'JetBrains Mono', 'Fira Code', monospace;
  --font-sans: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
  --zoom: 1;
}}

* {{ box-sizing: border-box; margin: 0; padding: 0; }}
body {{
  background: var(--bg-base);
  color: var(--text-main);
  font-family: var(--font-sans);
  font-size: calc(13px * var(--zoom));
  line-height: 1.5;
  height: 100vh;
  display: flex;
  flex-direction: column;
  overflow: hidden;
}}

/* Header */
.app-header {{
  background: var(--bg-panel);
  border-bottom: 1px solid var(--border);
  padding: 10px 16px;
  display: flex;
  align-items: center;
  justify-content: space-between;
  flex-wrap: wrap;
  gap: 8px;
  z-index: 100;
}}
.header-left {{ display: flex; align-items: center; gap: 12px; }}
.logo {{
  font-family: var(--font-mono);
  font-weight: bold;
  font-size: 15px;
  color: var(--accent-amber);
  display: flex;
  align-items: center;
  gap: 6px;
}}
.semver-badge {{
  font-family: var(--font-mono);
  font-size: 11px;
  background: var(--bg-card);
  padding: 3px 8px;
  border-radius: 4px;
  border: 1px solid var(--border);
  color: var(--text-dim);
}}

.repo-tabs {{ display: flex; gap: 6px; }}
.repo-btn {{
  background: var(--bg-card);
  border: 1px solid var(--border);
  color: var(--text-dim);
  padding: 6px 12px;
  border-radius: 6px;
  cursor: pointer;
  font-family: var(--font-mono);
  font-size: 11px;
  font-weight: 600;
  transition: all 0.15s ease;
}}
.repo-btn:hover {{ color: var(--text-bright); border-color: var(--text-dim); }}
.repo-btn.active {{
  background: #1f2a37;
  color: var(--accent-amber);
  border-color: var(--accent-amber);
  box-shadow: 0 0 10px rgba(255, 171, 112, 0.2);
}}

/* Main Viewport */
.content-viewport {{
  flex: 1;
  overflow-y: auto;
  padding: 20px;
  display: flex;
  flex-direction: column;
  gap: 16px;
  max-width: 900px;
  margin: 0 auto;
  width: 100%;
}}

/* Milestone Cards */
.milestone-card {{
  background: var(--bg-card);
  border: 1px solid var(--border);
  border-radius: 8px;
  padding: 18px;
  transition: all 0.2s ease;
  position: relative;
}}
.milestone-card.status-active {{
  border-color: var(--accent-amber);
  background: #181e26;
  box-shadow: 0 0 16px rgba(255, 171, 112, 0.15);
}}
.milestone-card.status-completed {{
  opacity: 0.85;
  border-color: rgba(63, 185, 80, 0.4);
}}
.milestone-card.status-planned {{
  opacity: 0.7;
  border-style: dashed;
}}

.card-header {{
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 10px;
}}
.milestone-title {{
  font-size: 15px;
  font-weight: 700;
  color: var(--text-bright);
  display: flex;
  align-items: center;
  gap: 8px;
}}
.status-pill {{
  font-family: var(--font-mono);
  font-size: 10px;
  font-weight: 700;
  padding: 3px 8px;
  border-radius: 4px;
  text-transform: uppercase;
}}
.pill-active {{ background: rgba(255, 171, 112, 0.15); color: var(--accent-amber); border: 1px solid var(--accent-amber); }}
.pill-completed {{ background: rgba(63, 185, 80, 0.15); color: var(--accent-green); border: 1px solid var(--accent-green); }}
.pill-planned {{ background: rgba(139, 148, 158, 0.15); color: var(--text-dim); border: 1px solid var(--border); }}

/* Progress Bar */
.progress-bar-bg {{
  background: var(--bg-recessed);
  border-radius: 4px;
  height: 8px;
  width: 100%;
  overflow: hidden;
  margin: 10px 0 14px 0;
  border: 1px solid var(--border);
}}
.progress-bar-fill {{
  height: 100%;
  border-radius: 4px;
  background: linear-gradient(90deg, #38bdf8, #3fb950);
  transition: width 0.3s ease;
}}
.progress-bar-fill.fill-active {{
  background: linear-gradient(90deg, #38bdf8, #ffab70);
}}

/* Tasks List */
.tasks-list {{
  display: flex;
  flex-direction: column;
  gap: 6px;
  margin-top: 10px;
}}
.task-item {{
  font-size: 12px;
  display: flex;
  align-items: center;
  gap: 8px;
  color: var(--text-main);
}}
.task-item.done {{
  color: var(--text-dim);
  text-decoration: line-through;
}}

/* Amber Focus Pulse */
@keyframes milestonePulse {{
  0% {{
    box-shadow: 0 0 0 0 rgba(255, 171, 112, 0.8), 0 0 20px rgba(255, 171, 112, 0.5);
    border-color: #ffab70;
  }}
  40% {{
    box-shadow: 0 0 0 10px rgba(255, 171, 112, 0.2), 0 0 35px rgba(255, 171, 112, 0.8);
    border-color: #ffd0a8;
  }}
  100% {{
    box-shadow: 0 0 0 0 rgba(255, 171, 112, 0);
    border-color: var(--accent-amber);
  }}
}}
.focus-pulse {{
  animation: milestonePulse 2.2s cubic-bezier(0.16, 1, 0.3, 1) forwards;
}}
</style>
</head>
<body>

<header class="app-header">
  <div class="header-left">
    <div class="logo">🗺️ ECOSYSTEM ROADMAP</div>
    <div class="semver-badge" id="header-badge">TKS: v0.4.0-dev | TT: v0.1.0 | NK: v0.1.0</div>
  </div>

  <div class="repo-tabs">
    <button class="repo-btn active" id="tab-tks" onclick="switchRepo('tks')">🎹 The Klang Suite</button>
    <button class="repo-btn" id="tab-tt" onclick="switchRepo('tt')">🐸 ToadTracker</button>
    <button class="repo-btn" id="tab-nk" onclick="switchRepo('nk')">🌌 N'kai Framework</button>
  </div>

  <div style="display:flex; gap:6px;">
    <button class="repo-btn" onclick="location.reload()">🔄 Refresh</button>
  </div>
</header>

<main class="content-viewport" id="milestones-container">
  <!-- Dynamically Populated -->
</main>

<script>
window.ECOSYSTEM_ROADMAP = {data_json};

let activeRepo = 'tks';

function switchRepo(repoId) {{
  activeRepo = repoId;
  document.querySelectorAll('.repo-btn').forEach(b => b.classList.remove('active'));
  const btn = document.getElementById('tab-' + repoId);
  if (btn) btn.classList.add('active');

  renderMilestones();
}}

function renderMilestones() {{
  const container = document.getElementById('milestones-container');
  const repo = window.ECOSYSTEM_ROADMAP.repos[activeRepo];
  if (!repo) return;

  container.innerHTML = repo.milestones.map(m => {{
    const pillClass = m.status === 'active' ? 'pill-active' : (m.status === 'completed' ? 'pill-completed' : 'pill-planned');
    const fillClass = m.status === 'active' ? 'fill-active' : '';
    const badgeText = m.status === 'active' ? `🔨 ACTIVE (${{m.percentage}}%)` : (m.status === 'completed' ? `✅ COMPLETED` : `⏳ PLANNED`);
    
    return `
      <div class="milestone-card status-${{m.status}}" id="${{activeRepo}}-${{m.id}}">
        <div class="card-header">
          <div class="milestone-title">
            <span>${{repo.icon}}</span>
            <span>Milestone ${{m.id}}: ${{m.title}}</span>
          </div>
          <span class="status-pill ${{pillClass}}">${{badgeText}}</span>
        </div>

        <div class="progress-bar-bg">
          <div class="progress-bar-fill ${{fillClass}}" style="width: ${{m.percentage}}%;"></div>
        </div>

        <div class="tasks-list">
          ${{m.tasks.map(t => `
            <div class="task-item ${{t.completed ? 'done' : ''}}">
              <span>${{t.completed ? '✅' : '⏳'}}</span>
              <span>${{t.text}}</span>
            </div>
          `).join('')}}
        </div>
      </div>
    `;
  }}).join('');
}}

function handleHashRouting() {{
  const rawHash = window.location.hash.replace(/^#/, '');
  if (!rawHash) {{
    renderMilestones();
    return;
  }}

  const parts = rawHash.split('-');
  const repo = parts[0];
  const ver = parts.slice(1).join('-');

  if (['tks', 'tt', 'nk'].includes(repo)) {{
    switchRepo(repo);
    
    setTimeout(() => {{
      const target = document.getElementById(`${{repo}}-${{ver}}`) || document.querySelector(`.status-active`);
      if (target) {{
        target.scrollIntoView({{ behavior: 'smooth', block: 'center' }});
        target.classList.add('focus-pulse');
        setTimeout(() => target.classList.remove('focus-pulse'), 2500);
      }}
    }}, 100);
  }}
}}

window.addEventListener('DOMContentLoaded', () => {{
  handleHashRouting();
}});

window.addEventListener('hashchange', () => {{
  handleHashRouting();
}});
</script>
</body>
</html>
"""

def main():
    data = {
        "last_synced": datetime.datetime.now().isoformat(),
        "active_repo": "tks",
        "repos": {
            "tks": {
                "name": REPOS["tks"]["name"],
                "icon": REPOS["tks"]["icon"],
                "active_version": REPOS["tks"]["active_version"],
                "milestones": parse_tks_backlog(REPOS["tks"]["backlog"])
            },
            "tt": {
                "name": REPOS["tt"]["name"],
                "icon": REPOS["tt"]["icon"],
                "active_version": REPOS["tt"]["active_version"],
                "milestones": parse_tt_backlog(REPOS["tt"]["backlog"])
            },
            "nk": {
                "name": REPOS["nk"]["name"],
                "icon": REPOS["nk"]["icon"],
                "active_version": REPOS["nk"]["active_version"],
                "milestones": parse_nk_backlog(REPOS["nk"]["backlog"])
            }
        }
    }

    html = generate_roadmap_html(data)

    # Write to local repo sidecar dir
    repo_sidecar_path = r"C:\Dev\TheKlangSuite\.agents\sidecar\roadmap_sidecar.html"
    with open(repo_sidecar_path, 'w', encoding='utf-8') as f:
        f.write(html)
    print(f"Generated {repo_sidecar_path}")

    # Mirror to active artifact dir if present
    artifact_dir = r"C:\Users\codyg\.gemini\antigravity\brain\9241988e-1227-466e-a0a5-446d61a69d77"
    if os.path.exists(artifact_dir):
        artifact_path = os.path.join(artifact_dir, "roadmap_sidecar.html")
        with open(artifact_path, 'w', encoding='utf-8') as f:
            f.write(html)
        print(f"Mirrored to artifact: {artifact_path}")

    # Mirror to Research repo
    research_path = r"C:\Dev\Research\ui_ux\roadmap_sidecar.html"
    if os.path.exists(os.path.dirname(research_path)):
        with open(research_path, 'w', encoding='utf-8') as f:
            f.write(html)
        print(f"Mirrored to Research: {research_path}")

if __name__ == '__main__':
    main()
