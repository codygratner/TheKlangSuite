# N'kai PCDA Dialectic Governance & SOTA Benchmarks (Topics 6.1, 6.2 & 6.15)

## 1. Architectural Intent & Scope

The **N'kai Asymmetric Sidecar Framework** (`nkai`) provides high-density, interactive visual workbenches paired with conversational AI coding agents across the Four Pillars (`TheKlangSuite`, `ToadTracker`, `nkai`, `TheKlangResearch`).

As technical explorations evolve from initial brainstorms to production-grade engineering decisions, teams face two opposing failure modes:
1. **Unchecked Cognitive Friction & Batch Fatigue**: Deferring all triage and evaluation to a massive, grueling end-of-milestone meeting (batch post-mortem), resulting in rushed compromises and forgotten technical nuances.
2. **Chat Transcript Bloat & Token Combustion**: Dumping exhaustive multi-page pros/cons matrices, devil's advocate dissections, and benchmark teardowns directly into conversational chat windows, exhausting context windows and user patience.

This specification codifies the architectural solution across three interconnected domains:
- **Topic 6.1 (Continuous Live Triage Architecture)**: Real-time decision streaming and continuous session persistence contracts that replace batch post-mortems with ongoing triage.
- **Topic 6.2 (On-Demand SOTA Progressive Disclosure & PCDA Triad)**: The 4-part Hegelian Dialectic Triad (Thesis, Thesis Friction, Antithesis, Synthesis), strict acronym disambiguation (PCDA vs PCA), and ultra-terse chat disclosure.
- **Topic 6.15 (SOTA Precedents & Competitive Teardowns)**: Rigorous benchmarking against industry-leading developer tools (Anthropic Claude Artifacts, Playwright UI, Vitest UI, Cursor Composer).

---

## 2. Topic 6.1: Continuous Live Triage Architecture

### 2.1 The Paradigm Shift: Real-Time Streaming vs. Batch Post-Mortem

Historically, research sessions followed a "batch post-mortem" model: hours of brainstorming and subagent exploration took place, generating 10–20 proposals, which were then dumped into a final review pass. This created severe cognitive load, decision fatigue, and loss of intermediate context.

The **Continuous Live Triage** lifecycle shifts evaluation into a real-time, event-driven stream:

```
┌────────────────────────────────────────────────────────────────────────────────────────┐
│ CONTINUOUS LIVE TRIAGE LIFECYCLE                                                      │
│                                                                                        │
│   [ Prototyping / Exploration ] ──► [ Instant Verdict Action ] ──► [ Real-Time HUD ]   │
│                 │                                │                         │           │
│                 ▼                                ▼                         ▼           │
│         Workshop Stage Card              Action Chips               Ongoing Autopsy    │
│         - Interactive Widget             [ ✅ Approve ]             - Metric Pill HUD  │
│         - Live Parameter Sliders         [ ⏳ Table   ]             - Status Filter    │
│         - PCDA Dialectic Accordion       [ 💀 Kill    ]             - Auto-Save Event  │
│                                          [ ⚖️ PCDA    ]                                │
│                                                  │                                     │
│                                                  ▼                                     │
│                                      [ LocalStorage & Session ]                        │
│                                      - nkai_triage_session_state                       │
│                                      - Dual-Storage File Sync                          │
└────────────────────────────────────────────────────────────────────────────────────────┘
```

### 2.2 Lifecycle State Machine

Every proposal or research topic exists in one of four states:
- `PENDING` (Default): Topic authored, interactive controls live, awaiting executive review.
- `APPROVE` (Backlog Promotion): Promoted to immediate roadmap execution or active milestone.
- `TABLE` (Future R&D): Technically valid, deferred to avoid scope creep or awaiting physical hardware verification.
- `KILL` (Strategy Graveyard): Formally rejected. Architectural rationale is preserved permanently to prevent repeating historical anti-patterns.

### 2.3 Verdict Streaming & Session Persistence Contract

Under Continuous Live Triage:
1. **Immediate State Propagation**: Assigning a verdict (`setVerdict(itemId, verdict, note)`) immediately updates:
   - Topic card visual state (outline glow and status badge).
   - The Ongoing Autopsy Scoreboard banner (total, approved, tabled, killed, pending counters).
   - Filter chips and feed row DOM elements.
   - The Markdown Decision Composer textarea.
2. **Zero Data Loss Invariant**: Verdict mutations are committed synchronously to browser `localStorage` under the session key `nkai_triage_session_state`.
3. **Dual-Storage Sidecar Contract**: Sidecars operate under the dual-storage rule:
   - Primary Interactive Webview: `<appDataDir>\brain\<conversation-id>\<tool_name>.html`
   - Local Repository Mirror: `c:\Dev\TheKlangSuite\.agents\sidecar\<tool_name>.html`
4. **Non-Blocking Offline Durability**: Zero server round-trips are required for triage operations. When running under the CLI harness (`nkai_serve.py`), verdict events trigger an asynchronous background dispatch (`fetch('/api/dispatch')`) without stalling UI rendering.

---

## 3. Topic 6.2: On-Demand SOTA Progressive Disclosure & PCDA Triad

### 3.1 Acronym Disambiguation: PCDA vs. PCA

In audio DSP, machine learning, and embedded hardware engineering, naming collisions can create profound technical confusion. The framework strictly enforces **PCDA** rather than PCA:

| Acronym | Canonical Meaning | Domain Context | Collision Prevention Rule |
| :--- | :--- | :--- | :--- |
| **PCDA** | **Pros, Cons, Devil's Advocate** | Dialectic Evaluation & Governance | Reserved strictly for architectural review triads. |
| **PCA** | **Principal Component Analysis** | Audio DSP / Machine Learning | Unsupervised linear dimensionality reduction (e.g. timbre latent space projection). Never use PCA for reviews. |
| **PCA / PCBA** | **Printed Circuit Assembly** | Embedded Hardware Engineering | Physical soldered board BOM and assembly (e.g. TBD-16 ESP32-P4 carrier board). |

### 3.2 The 4-Part Hegelian Dialectic Triad

The PCDA framework adapts classical Hegelian dialectics (Thesis $\rightarrow$ Antithesis $\rightarrow$ Synthesis) into a battle-tested software engineering evaluation model:

```
┌───────────────────────────────────────────────────────────────────────────────────────┐
│ THE PCDA DIALECTIC TRIAD                                                              │
│                                                                                       │
│   1. PROS [Thesis]                Affirmative benefits, user value, and immediate     │
│                                   engineering velocity.                               │
│                                                                                       │
│   2. CONS [Thesis Friction]       Direct implementation costs, operational surface    │
│                                   friction, and known architectural tradeoffs.        │
│                                                                                       │
│   3. DEVIL'S ADVOCATE             Structural failure modes, worst-case production     │
│      [Antithesis]                 traps, hidden maintenance debt, and token bloat.    │
│                                                                                       │
│   4. THE 80/20 RESOLUTION         The battle-tested pragmatic architecture capturing  │
│      [Synthesis]                  80% of the value with 20% of the complexity.        │
└───────────────────────────────────────────────────────────────────────────────────────┘
```

#### Detailed Stage Definitions
1. **Pros [Thesis]**:
   - What problems does this solve immediately?
   - How does this improve sonic quality, developer speed, or UI clarity?
   - What affirmative advantages justify spending developer attention here?
2. **Cons [Thesis Friction]**:
   - What are the direct costs (LOC, binary size, runtime CPU cycles)?
   - What new APIs, dependencies, or conventions must team members learn?
   - What friction is introduced into day-to-day coding?
3. **Devil's Advocate [Antithesis]**:
   - *Adversarial stress-test*: Assume this proposal passes and catastrophic failure occurs six months later. What caused it?
   - Where are the hidden maintenance debts (e.g. cross-platform timer bugs, cache race conditions, token burn)?
   - Is this premature optimization or architectural over-engineering?
4. **The 80/20 Resolution [Synthesis]**:
   - Reconciles the affirmative thesis with the adversarial antithesis.
   - Identifies the minimal viable implementation delivering 80% of the desired utility while discarding the 80% of complexity causing 80% of the risks.
   - Outputs a concrete, uncompromised technical invariant.

### 3.3 On-Demand Progressive Disclosure Protocol

To aggressively protect LLM context windows and human cognitive capacity, PCDA matrices are **never dumped into chat proactively**:

1. **Ultra-Terse Chat Default**: Routine conversational turns strictly output 1–3 punchy sentences summarizing progress or stating conclusions:
   > *"Flight 7 PCDA styling and validator scripts are complete. Templates verified with zero syntax errors."*
2. **Explicit Trigger Requirement**: Full dialectic blocks are surfaced in chat **only** when:
   - The user explicitly requests "pros and cons", "PCDA", or "tradeoffs" in chat.
   - An interactive action chip (`[ ⚖️ Request Pros & Cons ]` or `[ ⚖️ PCDA ]`) is clicked in the N'kai sidecar canvas.
3. **The Sidecar Asymmetric Split**:
   - **Chat (Terse Orchestration)**: 1–3 sentence status updates, confirmation gates, and clickable artifact links.
   - **Sidecar (Rich Visualization)**: Full 4-lens comparison grids, interactive parameter widgets, and foldable PCDA accordions.

### 3.4 The 4-Lens Framework

When a deep-dive architectural review is triggered, proposals are systematically evaluated across four distinct analytical lenses:

| Lens | Focus Scope | Key Evaluation Questions |
| :--- | :--- | :--- |
| 🌍 **1. Real-World / Daily Ergonomics** | Developer experience & daily friction | How does this feel on Tuesday morning? Does it require 5 extra clicks or complex flags? Can a sleep-deprived engineer debug this quickly? |
| 🎛️ **2. Audio Software Industry** | Native plugin & synth precedents | How do Vital, Kilohearts, FabFilter, Elektron, Bitwig, or Reaper solve this? What standard conventions do audio users expect? *(Omitted during pure web tooling tasks to conserve tokens)* |
| 🏛️ **3. Wider Software Industry / SOTA** | State of the art in modern systems | How do Anthropic, OpenAI, GitHub, Vite, Playwright, or the Linux kernel solve this? What patterns have won in production across millions of users? |
| 🏆 **4. Best Practices & Sustainability** | Code longevity & token economics | Does this leave the codebase cleaner (Boy Scout rule)? Does it prevent technical debt? What is the token/allowance impact on LLM workflows? |

---

## 4. Topic 6.15: SOTA Precedent Teardowns & Industry Benchmarks

To ensure N'kai remains a world-class developer harness, its architecture was rigorously benchmarked against four state-of-the-art developer tooling ecosystems:

```
┌────────────────────────────────────────────────────────────────────────────────────────┐
│ SOTA PRECEDENT TEARDOWN MATRIX                                                         │
│                                                                                        │
│   Tool Precedent          Key Architectural Strength      N'kai Translation            │
│   ─────────────────────   ────────────────────────────    ───────────────────────────  │
│   Anthropic Artifacts     Isolated side-panel Webview     Asymmetric Sidecar Canvas    │
│                           with real-time DOM render.      with Zero-Bundler Durability │
│                                                                                        │
│   Playwright UI           Fast filter chips, live state   Slot 2 Ongoing Autopsy View  │
│                           scoreboard, instant scrubbers.  with 1-Click Jump & Pulse    │
│                                                                                        │
│   Vitest UI               Reactive watch-mode telemetry   Live Test Harness Telemetry  │
│                           and instant DOM pass/fail.      with Audio Haptic Feedback   │
│                                                                                        │
│   Cursor Composer         Multi-file diff inspection &    Dual-Track Communiqué        │
│                           rapid accept/reject actions.    Dispatch & Action Chips      │
└────────────────────────────────────────────────────────────────────────────────────────┘
```

### 4.1 Anthropic Claude Artifacts
- **Precedent Strengths**: Pioneered the asymmetric side-panel UX pattern. The chat conversation remains clean and focused on intent, while rich visual assets (diagrams, React components, SVGs, documents) render in an adjacent dedicated pane.
- **Precedent Vulnerabilities**: Heavy dependency on online cloud infrastructure, complex React transpilation pipelines, and brittle sandbox boundaries that can fail without network connectivity.
- **The N'kai Translation**:
  - Adopted the clean asymmetric side-panel layout.
  - Hardened with **Zero-Dependency Offline Durability**: 100% vanilla ES6 + CSS variables. Opens instantly via standard `file:///` protocols without node_modules, bundlers, or internet access.

### 4.2 Playwright UI & Trace Viewer
- **Precedent Strengths**: Exceptional ergonomic density. Offers instant DOM filter chips (All / Passed / Failed / Flaky), time-travel telemetry, and 1-click cross-panel jumping to exact error lines with visual pulse highlights.
- **Precedent Vulnerabilities**: High initial setup overhead and monolithic Electron/browser process consumption.
- **The N'kai Translation**:
  - Codified the **Ongoing Autopsy Scoreboard** (Total, Approved, Tabled, Killed, Pending).
  - Implemented instantaneous DOM filter chips (`filterAutopsyFeed(status)`).
  - Implemented 1-click cross-tab jump navigation (`jumpToWorkshopCard(topicId)`) paired with CSS `@keyframes cardPulse` visual highlight.

### 4.3 Vitest UI
- **Precedent Strengths**: Instantaneous watch-mode test updates, compact summary cards, and clean visual typography with high-contrast status pills.
- **Precedent Vulnerabilities**: Requires active Vite/Node development server running in the background.
- **The N'kai Translation**:
  - Compact summary cards with semantic status badges (`.badge-approved`, `.badge-tabled`, `.badge-killed`).
  - Web Audio API haptic sound ticks (`playHaptic('tab')`, `playHaptic('action')`, `playHapticTick()`) providing subconscious auditory confirmation of actions.

### 4.4 Cursor Composer
- **Precedent Strengths**: Streamlined action-oriented review interfaces. Code and proposal diffs can be accepted, rejected, or iterated with one keypress or click.
- **Precedent Vulnerabilities**: In-line chat widgets can clutter long conversations and lack holistic spatial overview.
- **The N'kai Translation**:
  - Rapid Action Chips (`[ 📥 Backlog ]`, `[ 🔬 Research ]`, `[ 🪦 Graveyard ]`, `[ ⚖️ PCDA ]`).
  - One-click clipboard and dispatch bridges (`copyPostMortemReport()`, `NkaiEnv.dispatchAction()`) facilitating frictionless round-trips back to AI agent prompts.

---

## 5. Component Architecture & Design Contracts

### 5.1 Semantic CSS Dialectic Classes (`sidecar.css`)

PCDA blocks feature distinct, high-contrast semantic borders and subtle tinted backgrounds:

```css
/* Container & Accordion Shell */
.pcda-container {
  display: flex;
  flex-direction: column;
  gap: 12px;
  margin-top: 14px;
}

.pcda-block {
  border: 1px solid var(--border);
  border-radius: 8px;
  background: var(--bg-panel);
  overflow: hidden;
  transition: border-color 0.15s ease, box-shadow 0.15s ease;
}

.pcda-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 10px 14px;
  background: var(--bg-card);
  cursor: pointer;
  user-select: none;
  font-family: var(--font-mono);
  font-size: 12px;
  font-weight: 600;
}

/* Dialectic Semantic Cards */
.pcda-pro {
  border-left: 4px solid #10b981; /* Emerald */
  background: rgba(16, 185, 129, 0.05);
  padding: 12px 14px;
  margin: 6px 0;
  border-radius: 0 6px 6px 0;
}

.pcda-con {
  border-left: 4px solid #f59e0b; /* Amber */
  background: rgba(245, 158, 11, 0.05);
  padding: 12px 14px;
  margin: 6px 0;
  border-radius: 0 6px 6px 0;
}

.pcda-advocate {
  border-left: 4px solid #f43f5e; /* Rose */
  background: rgba(244, 63, 94, 0.05);
  padding: 12px 14px;
  margin: 6px 0;
  border-radius: 0 6px 6px 0;
}

.pcda-synthesis {
  border-left: 4px solid #8b5cf6; /* Indigo/Purple Gradient */
  border-image: linear-gradient(to bottom, #6366f1, #8b5cf6) 1 100%;
  background: rgba(139, 92, 246, 0.08);
  padding: 12px 14px;
  margin: 8px 0;
  border-radius: 0 6px 6px 0;
}

/* SOTA Benchmark Badge */
.badge-sota {
  display: inline-flex;
  align-items: center;
  gap: 4px;
  font-family: var(--font-mono);
  font-size: 10px;
  font-weight: 600;
  padding: 2px 7px;
  border-radius: 10px;
  background: rgba(88, 166, 255, 0.15);
  border: 1px solid rgba(88, 166, 255, 0.4);
  color: #58a6ff;
  box-shadow: 0 0 8px rgba(88, 166, 255, 0.2);
}

/* 4-Lens Tradeoff Grid */
.lens-grid {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 10px;
  margin-top: 10px;
}

@media (max-width: 860px) {
  .lens-grid {
    grid-template-columns: repeat(2, 1fr);
  }
}

@media (max-width: 520px) {
  .lens-grid {
    grid-template-columns: 1fr;
  }
}
```

### 5.2 JavaScript Accordion & Extraction Contract (`sidecar.js`)

```javascript
/**
 * Toggles visibility of a PCDA dialectic accordion block.
 * Emits a tactile Web Audio haptic tick on state change.
 * @param {string} blockId - Unique ID of the PCDA block body.
 */
function togglePcdaBlock(blockId) {
  if (typeof playHapticTick === 'function') {
    playHapticTick();
  } else if (typeof playHaptic === 'function') {
    playHaptic('tick');
  }

  const block = document.getElementById(blockId);
  const icon = document.getElementById(blockId + '-icon');
  if (!block) return;

  const isCollapsed = block.classList.contains('collapsed') || block.style.display === 'none';
  if (isCollapsed) {
    block.classList.remove('collapsed');
    block.style.display = 'block';
    if (icon) icon.textContent = '▼';
  } else {
    block.classList.add('collapsed');
    block.style.display = 'none';
    if (icon) icon.textContent = '▶';
  }
}

/**
 * Copies clean GitHub-flavored PCDA Markdown for a topic to the clipboard.
 * Dispatches via NkaiEnv if running in extension/server harness.
 * @param {string} topicId - Unique identifier of the topic.
 */
function copyPcdaMarkdown(topicId) {
  const container = document.getElementById('pcda-' + topicId) || document.getElementById(topicId);
  if (!container) return;

  const topicTitle = container.getAttribute('data-topic-title') || ('Topic ' + topicId);
  const proText = container.querySelector('.pcda-pro')?.innerText.trim() || '';
  const conText = container.querySelector('.pcda-con')?.innerText.trim() || '';
  const advText = container.querySelector('.pcda-advocate')?.innerText.trim() || '';
  const synText = container.querySelector('.pcda-synthesis')?.innerText.trim() || '';

  const md = [
    `### ⚖️ PCDA Dialectic Review: ${topicTitle}`,
    ``,
    `#### 1. Pros [Thesis]`,
    proText,
    ``,
    `#### 2. Cons [Thesis Friction]`,
    conText,
    ``,
    `#### 3. Devil's Advocate [Antithesis]`,
    advText,
    ``,
    `#### 4. The 80/20 Resolution [Synthesis]`,
    synText,
    ``
  ].join('\n');

  if (window.NkaiEnv && typeof window.NkaiEnv.dispatchAction === 'function') {
    window.NkaiEnv.dispatchAction(md);
  } else if (navigator.clipboard && navigator.clipboard.writeText) {
    navigator.clipboard.writeText(md).then(() => {
      showToast('⚖️ PCDA Markdown copied to clipboard!');
    }).catch(() => {
      showToast('PCDA copied (fallback)!');
    });
  }
}
```

---

## 6. Tooling Contract: `tools/pcda_format.py`

### 6.1 Purpose & Execution Environment
`tools/pcda_format.py` is a zero-dependency CLI utility written in standard Python 3. It parses Markdown files containing PCDA dialectic blocks, validates that all four components are present, converts them to drop-in N'kai HTML snippets, and generates terse chat summaries.

### 6.2 Command-Line Interface Specification
```
usage: pcda_format.py [-h] (--validate FILE | --to-html FILE | --chat FILE) [--output OUT]

Zero-Dependency N'kai PCDA Formatter & Validator

options:
  -h, --help           show this help message and exit
  --validate FILE      Validate that all 4 PCDA components exist for each topic in FILE
  --to-html FILE       Convert Markdown PCDA blocks in FILE to drop-in N'kai HTML snippets
  --chat FILE          Generate terse, chat-friendly Markdown summaries of PCDA blocks
  --output OUT, -o OUT Optional destination output file (defaults to stdout)
```

### 6.3 Validation Rules
A topic block passes `--validate` if and only if:
1. It contains a topic identifier/header (e.g. `### Topic X.Y: ...` or `## Topic X.Y: ...`).
2. It contains all four dialectic components:
   - **Pros [Thesis]** (`Pros`, `Pros [Thesis]`, `1. Pros`)
   - **Cons [Thesis Friction]** (`Cons`, `Cons [Thesis Friction]`, `2. Cons`)
   - **Devil's Advocate [Antithesis]** (`Devil's Advocate`, `Advocate`, `3. Devil's Advocate`)
   - **The 80/20 Resolution [Synthesis]** (`80/20 Resolution`, `Resolution`, `Synthesis`, `4. The 80/20 Resolution`)

If any component is missing, `pcda_format.py` exits with code `1` and prints precise diagnostic errors detailing the missing section and line number.

---

## 7. Verification & Acceptance Gauntlet

1. **Catalog Integrity**: Spec registered in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.
2. **Offline Durability**: All CSS and JavaScript components execute 100% offline via `file:///` without external CDNs or network fetches.
3. **Tag Balance Verification**: Updated templates pass `tools/audit_sidecar.py` with zero open tag discrepancies and zero syntax errors.
4. **Validation Suite**: `tools/pcda_format.py` passes automated tests across `--validate`, `--to-html`, and `--chat` modes.
5. **Context Clues Parity**: `tools/verify_context_clues.py` completes with zero broken pointers.
