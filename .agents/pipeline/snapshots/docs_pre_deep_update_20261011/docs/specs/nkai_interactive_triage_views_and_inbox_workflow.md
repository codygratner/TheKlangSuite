# N'kai Interactive Triage Views & Inbox Workflow (Topics 6.8 & 6.11)

## 1. Architectural Intent & Scope

The **N'kai Asymmetric Sidecar Framework** (`nkai`) provides high-density, interactive visual workbenches paired with conversational AI coding agents across the Four Pillars (`TheKlangSuite`, `ToadTracker`, `nkai`, `TheKlangResearch`).

As research boards and brainstorming sessions scale to dozens of proposals, managing executive decisions, active status, and incoming raw thoughts creates cognitive friction. Specifically, two critical workflows required architectural formalization:

1. **Topic 6.8 (Approved Option B — "Ongoing Autopsy": Live Triage Feed & Scoreboard)**:
   - Previously, post-mortem decisions were sequestered at the bottom of the long canvas or in a disconnected static summary. The user had to manually jump between proposal cards and summary text with no continuous feedback on decision tallies.
   - The **Ongoing Autopsy View** establishes a dedicated, interactive HUD positioned prominently in **Slot 2 of the navigation rail** (`#tab-autopsy` / `.view-autopsy`), displaying a live metric scoreboard, instantaneous DOM filter chips, compact topic rows, and 1-click cross-tab jump navigation back to workshop cards with a visual pulse highlight.
2. **Topic 6.11 (Approved Option B — "Check the Mail" Inbox Workflow & Tooling)**:
   - Creative notes, midnight musings, and feedback often accumulate in the user's Obsidian Vault inbox (`C:\Dev\TheKlangVault\Inbox\`). Historically, these notes remained untriaged or required manual copy-pasting into chat prompts.
   - The **"Check the Mail" Harvester** (`tools/check_mail.py`) scans the vault inbox (with archive and repo fallbacks), generates an interactive N'kai triage board (`morning_mail_inbox.html`) equipped with rapid decision action chips (`[ 📥 Backlog ]`, `[ 🔬 Research ]`, `[ 🪦 Graveyard ]`, `[ ⚖️ PCDA ]`), and adheres to the Flight 4 terminal standard with a `--chat` / `--text-only` console dump.

---

## 2. Topic 6.8: Ongoing Autopsy View (`#tab-autopsy`)

### 2.1 Navigation Rail Placement & Slot Hierarchy
The Ongoing Autopsy View occupies **Slot 2** in the bottom tier (`.header-tier-bottom`) of the N'kai header, immediately adjacent to the Active Workshop:

```
┌─────────────────────────────────────────────────────────────────────────────────────────┐
│ .header-tier-bottom (Horizontal Navigation Rail)                                        │
│ ┌───────────────────┐ ┌───────────────────┐ ┌───────────────────┐  ┌───────┐ ┌───────┐  │
│ │ 🛠️ WORKSHOP       │ │ 📊 AUTOPSY        │ │ ⚖️ POST-MORTEM   │  │ 1 📐  │ │ 2 🎨  │  │
│ │ (Slot 1 - Cards)  │ │ (Slot 2 - Feed)   │ │ (Slot 3 - Slates) │  │ (1.1) │ │ (2.1) │  │
│ └───────────────────┘ └───────────────────┘ └───────────────────┘  └───────┘ └───────┘  │
└─────────────────────────────────────────────────────────────────────────────────────────┘
```

- **Slot 1 (`#btn-workshop`)**: Active Workshop stage where individual cards and interactive simulators are manipulated.
- **Slot 2 (`#btn-autopsy`)**: Ongoing Autopsy Feed presenting aggregate score metrics, status filters, and instant jump targets.
- **Slot 3 (`#btn-pm`)**: Full Post-Mortem & Triage Slates detailing deep rationale lists by final milestone promotion.

### 2.2 Triage Scoreboard Banner (`.triage-scoreboard`)
At the top of the Autopsy view, a high-contrast scoreboard banner displays real-time verdict tallies calculated dynamically from the active state:

```html
<div class="triage-scoreboard">
  <div class="scoreboard-pill pill-total">
    <span class="pill-label">TOPICS</span>
    <span class="pill-value" id="autopsy-count-total">7</span>
  </div>
  <div class="scoreboard-pill pill-approved">
    <span class="pill-label">APPROVED</span>
    <span class="pill-value" id="autopsy-count-approved">0</span>
  </div>
  <div class="scoreboard-pill pill-tabled">
    <span class="pill-label">TABLED</span>
    <span class="pill-value" id="autopsy-count-tabled">0</span>
  </div>
  <div class="scoreboard-pill pill-killed">
    <span class="pill-label">KILLED</span>
    <span class="pill-value" id="autopsy-count-killed">0</span>
  </div>
  <div class="scoreboard-pill pill-pending">
    <span class="pill-label">PENDING</span>
    <span class="pill-value" id="autopsy-count-pending">7</span>
  </div>
  <button class="autopsy-report-btn" onclick="copyPostMortemReport()" title="Copy GitHub Markdown Report to Clipboard">
    <span>📋</span> Copy Report
  </button>
</div>
```

#### Metric Pill Styling & Semantic Palette
- **Total Topics (`.pill-total`)**: Border `#30363d`, Text `#c9d1d9`, Background `#161b22`.
- **Approved (`.pill-approved`)**: Border `rgba(16, 185, 129, 0.4)`, Text `#34d399` (Emerald), Background `rgba(6, 78, 59, 0.25)`.
- **Tabled (`.pill-tabled`)**: Border `rgba(245, 158, 11, 0.4)`, Text `#fbbf24` (Amber), Background `rgba(69, 26, 3, 0.25)`.
- **Killed (`.pill-killed`)**: Border `rgba(239, 68, 68, 0.4)`, Text `#f87171` (Rose), Background `rgba(69, 10, 10, 0.25)`.
- **Pending (`.pill-pending`)**: Border `rgba(148, 163, 184, 0.3)`, Text `#94a3b8` (Slate), Background `rgba(30, 41, 59, 0.25)`.

### 2.3 Instant Real-Time Filter Chips (`.filter-chips`)
Directly below the scoreboard, filter chips allow single-click DOM filtering without page reloads or layout shift:

```html
<div class="autopsy-filter-bar">
  <span class="filter-label">Filter Feed:</span>
  <div class="filter-chips">
    <button class="filter-chip active" data-filter="all" onclick="filterAutopsyFeed('all')">All (<span id="chip-count-all">7</span>)</button>
    <button class="filter-chip" data-filter="approved" onclick="filterAutopsyFeed('approved')">✅ Approved (<span id="chip-count-approved">0</span>)</button>
    <button class="filter-chip" data-filter="tabled" onclick="filterAutopsyFeed('tabled')">⏳ Tabled (<span id="chip-count-tabled">0</span>)</button>
    <button class="filter-chip" data-filter="killed" onclick="filterAutopsyFeed('killed')">💀 Killed (<span id="chip-count-killed">0</span>)</button>
    <button class="filter-chip" data-filter="pending" onclick="filterAutopsyFeed('pending')">⚪ Pending (<span id="chip-count-pending">7</span>)</button>
  </div>
</div>
```

When a chip is clicked:
1. Active styling is updated across `.filter-chip` elements.
2. Feed items matching the status remain visible (`display: flex`); non-matching items are hidden (`display: none`).
3. If no items match the filter, a clean empty state card is displayed.

### 2.4 Compact Triage Feed Stream & Cross-Tab Jump Mechanics
The feed stream renders each topic as a sleek, single-row card (`.autopsy-feed-item`):

```html
<div class="autopsy-feed-item" id="autopsy-item-1.1" data-topic-id="1.1" data-status="pending">
  <div class="autopsy-feed-left">
    <span class="autopsy-topic-id">ITEM 1.1</span>
    <span class="autopsy-status-badge badge-pending">⚪ PENDING</span>
    <span class="autopsy-topic-title">Monolithic Sculpted Titanium Faceplate vs. Modular Cards</span>
  </div>
  <div class="autopsy-feed-right">
    <span class="autopsy-feed-note" id="autopsy-note-1.1">Awaiting verdict...</span>
    <button class="autopsy-jump-btn" onclick="jumpToWorkshopCard('1.1')" title="Jump to Active Workshop Card">
      <span>🔍</span> Jump to Card
    </button>
  </div>
</div>
```

#### 1-Click Cross-Tab Jump & Pulse Glow
Clicking `[ 🔍 Jump to Card ]` invokes `jumpToWorkshopCard(topicId)`:
1. Calls `switchMode('workshop')` to activate `#workshop-mode` (or `#tab-workshop`).
2. Calls `showHeroItem(topicId)` to expose the target topic panel.
3. Smoothly scrolls the element into viewport center: `target.scrollIntoView({ behavior: 'smooth', block: 'center' })`.
4. Applies `.pulse-target` trigger class, activating the CSS keyframe animation:

```css
@keyframes cardPulse {
  0% {
    box-shadow: 0 0 0 0 rgba(88, 166, 255, 0.7);
    border-color: var(--accent-blue);
  }
  50% {
    box-shadow: 0 0 24px 6px rgba(88, 166, 255, 0.9);
    border-color: #58a6ff;
  }
  100% {
    box-shadow: 0 0 0 0 rgba(88, 166, 255, 0);
  }
}

.pulse-target {
  animation: cardPulse 1.8s cubic-bezier(0.4, 0, 0.6, 1) 2;
  border-color: var(--accent-blue) !important;
}
```

### 2.5 Post-Mortem Report Serializer (`copyPostMortemReport()`)
Clicking `[ 📋 Copy Report ]` generates a clean, GitHub-flavored Markdown post-mortem synthesis:
- Header with timestamp and total counts.
- Categorized sections: `## ✅ Approved for Backlog`, `## ⏳ Tabled for Future R&D`, `## 💀 Strategy Graveyard`, and `## ⚪ Pending Review`.
- Dispatches via `window.NkaiEnv.dispatchAction` (extension mode) or fallback `navigator.clipboard.writeText`, providing visual confirmation feedback ("✅ Copied!").

---

## 3. Topic 6.11: "Check the Mail" (Vault Inbox) Workflow & Tooling

### 3.1 The Problem
During development, users write quick thoughts, design ideas, and feedback into their Obsidian Vault inbox (`C:\Dev\TheKlangVault\Inbox\`). These notes often become siloed, disconnected from active sprint cycles, or forgotten unless manually reviewed.

### 3.2 The Harvester Tool (`tools/check_mail.py`)
`tools/check_mail.py` is a zero-dependency CLI tool built with standard Python libraries (`os`, `sys`, `pathlib`, `argparse`, `re`, `json`, `webbrowser`, `datetime`).

#### Discovery Hierarchy
1. `--dir <path>`: Explicit directory passed by user.
2. `--archive`: Scans `<dir>/Archive` or `C:\Dev\TheKlangVault\Inbox\Archive`.
3. Default primary: `C:\Dev\TheKlangVault\Inbox`.
4. Workspace fallback: `docs/inbox` or current directory.

#### Metadata & Note Extraction Heuristics
For each `.md` file discovered:
- **Title**:
  1. YAML frontmatter `title: ...`
  2. First Markdown heading (`# Heading`)
  3. Cleaned filename stem (`path.stem`)
- **Date**:
  1. YAML frontmatter `date: ...`
  2. File modification time (`mtime`) formatted as `YYYY-MM-DD`
- **Body & Excerpt**:
  - Frontmatter stripped cleanly.
  - Excerpt generated from the first 250 characters of prose.
  - Complete body preserved for full card view.

### 3.3 Interactive Inbox Triage Board (`morning_mail_inbox.html`)
The harvester compiles discovered notes into a standalone, self-contained N'kai triage board (`morning_mail_inbox.html`):

#### Triage Action Chips
Each mail card features four actionable dispatch chips:
1. `[ 📥 Add to Backlog ]`: Pre-formats an entry for `docs/BACKLOG.md`.
2. `[ 🔬 Move to Research ]`: Routes idea to `TheKlangResearch` RFC backlog.
3. `[ 🪦 Reject / Graveyard ]`: Documents the concept in the post-mortem graveyard with rationale.
4. `[ ⚖️ Request PCDA ]`: Generates a dialectic evaluation prompt (`Pros`, `Cons`, `Devil's Advocate`, `80/20 Resolution`).

### 3.4 Terminal Agent Standard (`--chat` / `--text-only`)
Adhering to the Flight 4 / Topic 6.14 `--chat` fallback standard, running:
```bash
python tools/check_mail.py --chat
```
bypasses browser launch, outputting a concise Markdown ledger directly to standard output:

```markdown
# 📬 Morning Mail Inbox (N Notes Found)
Source: C:\Dev\TheKlangVault\Inbox

- [ ] **Bass mode!** (2026-10-07)
  > This might be diluting the plan, but maybe there can be a version of Planter where the 2-op FM can be dual oscillator instead? ...
  *Actions:* [Backlog: /plan] | [Research: /deep-research] | [PCDA: /grill-me]

- [ ] **Deep Research Subagents** (2026-10-09)
  > When we do a deep research session? For the flash explorers, wouldn't it be more efficient to spin up several subagents...
  *Actions:* [Backlog: /plan] | [Research: /deep-research] | [PCDA: /grill-me]
```

---

## 4. Architectural Invariants & Cross-Platform Guarantees

1. **Zero External Dependencies**: `tools/check_mail.py` relies exclusively on Python standard library modules.
2. **Deterministic Offline Operation**: All HTML and JS operate without external CDNs, fully compliant with Chromium `file:///` sandboxes and air-gapped workstations.
3. **Audio-Safety Alignment**: Pure tooling harness code; zero C++ audio processing or dynamic allocation in real-time DSP paths.
4. **Synchronized Template State**: The `#tab-autopsy` view skeleton is preserved across all three canonical distribution targets (`nkai/core`, `nkai/bundle/templates`, and `TheKlangSuite/.agents/sidecar`).
