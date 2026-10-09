# N'kai Skill Taxonomy & Invocation Architecture (Topics 6.4 & 6.14)

## 1. Architectural Intent & Scope

The **N'kai Asymmetric Sidecar Framework** (`nkai`) and **The Klang Suite** (`TheKlangSuite`) share a rich library of specialized agent skills and automated workflows. Over project evolution, skills have expanded from single-purpose scripts into multi-layered automation engines spanning real-time C++ audio thread audits, interactive HTML webview canvases, multi-repository documentation synchronization, and continuous factory build validation.

Without formal architectural stratification, agent skill systems suffer from two distinct failure modes:
1. **The Monolithic Tool Trap**: Forcing all developer actions through opaque, multi-stage mega-commands where individual diagnostic steps cannot be inspected or manually re-triggered.
2. **The Fragmented Script Trap**: Exposing dozens of tiny, disconnected scripts without deterministic pipelines, forcing the developer or agent to remember dozens of CLI flags and sequence them manually.
3. **The Webview-Only Lock-in**: Assuming a rich side-panel browser view is always present, stranding developers who work in headless terminals, SSH environments, or text-only AI CLIs (Claude Code CLI, Codex CLI, Aider).

### The Solution: Option C (Layered Hybrid Skills) & Option B (N'kai-First with `--chat` Fallback)
Approved under **Topic 6.4** and **Topic 6.14**, this specification establishes:
1. **The 3-Layer Skill Hierarchy**: Discrete stratification into **Layer 1: Atomic Micro-Skills**, **Layer 2: Macro Orchestrators**, and **Layer 3: Interactive Visual Experiences**.
2. **The N'kai-First Visual Default**: All interactive design, research, and triage workflows default to generating dual-storage HTML canvases, executing the **Sidecar Ready Handshake**, and adhering to the **Asymmetric Sidecar Split**.
3. **The `--chat` / `--text-only` Fallback Standard**: A universal escape hatch across all visual experiences allowing seamless degradation to inline Markdown chat for terminal CLIs, low-bandwidth environments, or keyboard-first flows.
4. **Deterministic Domain Routing**: Strict pipeline routing preventing concurrent clobbering between native C++ audio engineering (`TARGET: FACTORY_FLOOR`) and tooling/harness automation (`TARGET: TOOLING_HARNESS`).

---

## 2. The 3-Layer Skill Hierarchy

```
┌────────────────────────────────────────────────────────────────────────┐
│              LAYER 3: INTERACTIVE VISUAL EXPERIENCES (N'KAI)           │
│  • visual-grill-me      • deep-research           • post-mortem        │
│  (Live HTML Canvas, Web Audio Sandbox, Tri-Mode Asymmetric Split)      │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │ (delegates / triggers)
┌───────────────────────────────────▼────────────────────────────────────┐
│                    LAYER 2: MACRO ORCHESTRATORS                        │
│  • execute-task         • qa                      • update-docs        │
│  • cut-release          • check-mail              • audit-reorder      │
│  (Deterministic Pipelines, Communiqué Routing, Multi-Repo Sync)        │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │ (sequences / invokes)
┌───────────────────────────────────▼────────────────────────────────────┐
│                      LAYER 1: ATOMIC MICRO-SKILLS                      │
│  • audiothread-guard    • build-validate          • step-verify        │
│  • web-qa               • clean-plan              • compat-check       │
│  • clear                • refresh-context                              │
│  (Single-Purpose, Zero-Side-Effect Diagnostics, Focused Correctness)   │
└────────────────────────────────────────────────────────────────────────┘
```

### 2.1 Layer 1: Atomic Micro-Skills
Atomic micro-skills are precision, single-purpose engineering tools. They perform a single deterministic diagnostic, audit, or transformation, and exit cleanly with structured status codes and reports. They never spawn subagents and never trigger multi-stage pipeline loops.

| Micro-Skill | Scope / Function | Invariants & Guardrails |
| :--- | :--- | :--- |
| `audiothread-guard` | Static & heuristic AST audit of C++ DSP code in `source/`. | Scans for heap allocations (`new`, `malloc`, vector resize), blocking mutexes/locks, file/console I/O (`DBG`, `cout`), and unvectorized CRT transcendentals in hot loops. |
| `build-validate` | CMake build runner and test filter. | Invokes native CMake builds (`Release`), filters noisy MSBuild/Clang compiler output down to actionable errors, and executes native test suites (`dsp_tests`, `gui_tests`, `pluginval`). |
| `step-verify` | Incremental phase verification & staging. | Audits modified source files against active phase criteria in `PLAN.md`, checks checklist boxes (`- [x]`), and stages files in Git (`git add`). |
| `web-qa` | Sidecar HTML/JS/CSS diagnostic linter. | Scans for 8 webview hazards: JS Unicode path escapes, HTML tag imbalance, strict-mode event leaks, click-trapping overlays, Chromium memory cache traps, Web Audio autoplay safety, PNA loopback CORS, and phantom bridges. |
| `audit-reorder` | Topological triage & backlog sorter. | Executes a 2-pass convergence gate (Flash PCDA exploration + Pro topological synthesis) to sequence prerequisite-first flight paths. |
| `clean-plan` | Planning artifact hygiene manager. | Cleans up temporary snapshots (`context_snapshot*.md`, `PLAN_BACKUP*.md`) and archives completed plans to `.agents/pipeline/plans/completed/`. |
| `compat-check` | Cross-platform build & JUCE hygiene auditor. | Verifies macOS deployment targets (`11.0`, `arm64;x86_64`), Linux static runtime linking (`-static-libstdc++`), and JUCE 9.0.3 timer destructor safety (`#1696`). |
| `clear` | Context window wipe manager. | Cleans active chat context by wiping underlying transcript files on demand. |
| `refresh-context` | Institutional memory harvester. | Distills active chat takeaways into `docs/history/DEV_HISTORY.md` and generates context clue artifacts. |

### 2.2 Layer 2: Macro Orchestrators
Macro orchestrators compose multiple Layer 1 atomic micro-skills into deterministic, end-to-end operational pipelines. They manage dispatch routing, state transitions, conventional Git commits, and handshakes between planners and builders.

| Macro Orchestrator | Primary Pipeline Chain | Routing & Escalation Policy |
| :--- | :--- | :--- |
| `execute-task` | `audiothread-guard` ➔ `build-validate` ➔ `step-verify` ➔ `task-finish` | **Domain Routing**: Sourced strictly from `PLAN.md`. `TARGET: FACTORY_FLOOR` routes to Klang Industries (native C++); `TARGET: TOOLING_HARNESS` routes to Flash Builder Subagent (New Klang City). Enforces 2-strike escalation on compile failures. |
| `qa` | Context-Aware Routing: <br>• Native Audio: `audiothread-guard` + `build-validate`<br>• Web/Sidecar: `web-qa` | Dispatches based on active file context or prompt keywords. When inspecting `.html` or webview scripts, triggers `web-qa`. When inspecting `.cpp`/`.h` or DSP DSP algorithms, triggers audio thread audit and build validation. |
| `update-docs` | Docs Reconciler & Vault Mirror Bridge: <br>• Incremental: Catalog + System Map + Backlog<br>• Deep (`--deep`): 5-Pillar Gauntlet + Soul Ingestion | Audits, synchronizes, and reconciles all project documentation (`docs/BACKLOG.md`, `docs/specs/`, `DEV_HISTORY.md`, `SYSTEM_MAP.md`, and `DOCS_CATALOG.json`). Bridges records to Obsidian Vault. |
| `cut-release` | Release Gauntlet: <br>• Pre-flight cruft audit ➔ Dual-config build ➔ Automated version bump ➔ Git tagging ➔ Artifact deployment | Enforces zero uncommitted changes, clean test runs on both Debug and Release, tag annotation (`vX.Y.Z`), and institutional memory harvesting. |

### 2.3 Layer 3: Interactive Visual Experiences
Interactive visual experiences are N'kai's asymmetric pair-programming canvases. They pair an ultra-terse agent prompt with an interactive HTML+CSS+JS surface rendered in the IDE's side panel or external browser.

| Experience | Canvas Artifact | Core Interactive Capabilities |
| :--- | :--- | :--- |
| `visual-grill-me` | `visual_grill_me.html` | Interactive architectural and UI design interview. Live comparative mockups (Option A vs B vs C), real-time Web Audio audition synth, canvas oscilloscope, Decision Composer, and state synchronization. |
| `deep-research` | `<slug>_research_board.html` | Multi-domain research board. Tri-engine subagent delegation (`research` Pro, `explore` Flash, `contrarian` Advocate), live in-flight card triage (`APPROVE`, `TABLE`, `KILL`), negative architecture (Strategy Graveyard), and soul harvesting. |
| `post-mortem` | `post_mortem.html` / `packages/` | Structured post-mortem triage lab. One-click Executive Ledger ingestion, continuous in-flight voting, state snapshot packaging, and resume support (`/resume-post-mortem`). |

---

## 3. The N'kai-First Visual Default Standard

By default, every Layer 3 Interactive Visual Experience MUST adhere to the N'kai-First invocation standard:

### 3.1 Dual-Storage Canvas Invariant
All generated HTML canvases must be written simultaneously to two locations:
1. **The Antigravity Artifact Directory**: `<appDataDir>\brain\<conversation-id>\<name>.html`  
   *Purpose:* Enables native side-panel Webview rendering via Antigravity's artifact previewer.
2. **The Workspace Repository Directory**: `c:\Dev\TheKlangSuite\.agents\sidecar\<name>.html`  
   *Purpose:* Provides persistent local storage, version tracking, and CLI browser server access (`nkai_serve.py`).

### 3.2 The Sidecar Ready Handshake
To prevent overwhelming the developer with verbose chat dumps while a visual canvas is loading, agents MUST execute the 3-step handshake:
1. **Prominent Clickable Artifact Link**: Output the exact file URI pointing to the artifact directory:
   ```markdown
   👉 **[Open <Canvas Name> Canvas](file:///<appDataDir>/brain/<conversation-id>/<name>.html)**
   ```
   *(Antigravity opens workspace repo paths in the text editor, but intercepts artifact directory paths to render in the rich interactive side-panel Webview).*
2. **The Ready Pause**: Output a polite, concise prompt instructing the developer to open the panel:
   > *"Please open the canvas link above in your side panel, then reply 'ready' to begin!"*
3. **Execution Gate**: Pause completely and wait for the user to confirm ("ready", "open", or direct input) before presenting complex question blocks or comparative mockups.

### 3.3 The Asymmetric Sidecar Split
Once the sidecar canvas is verified active:
- **Chat Responses**: Must be ultra-terse, punchy, and conversational (1–3 sentences or quick prompts).
- **Sidecar Canvas**: Contains all verbose explanations, comparative tradeoff tables, interactive mockups, architectural diagrams, and option selectors.

---

## 4. The `--chat` / `--text-only` Fallback Standard

While N'kai sidecars provide rich visual pair-programming, developers frequently work in environments where an embedded browser pane is unavailable or unwanted:
- **Terminal AI CLIs**: Claude Code CLI, OpenAI Codex CLI, Aider, or SSH terminal sessions.
- **Low-Bandwidth / Remote Work**: Remote headless workstations over slow X11/VNC links.
- **Keyboard-First Focus**: Rapid architectural alignment where switching to a mouse or browser interrupts typing flow.

### 4.1 Invocation Syntax
Every Layer 3 skill accepts explicit fallback flags:
```bash
/visual-grill-me --chat          # Skips HTML canvas; runs interactive Q&A directly in chat
/visual-grill-me --text-only     # Alias for --chat
/deep-research --chat            # Skips research board HTML; outputs structured Markdown summary in chat
/post-mortem --chat              # Skips triage canvas; conducts sequential or bulk triage in chat
```

### 4.2 Fallback Behavior by Skill

```
┌────────────────────────────────────────────────────────────────────────┐
│                   SKILL INVOCATION WITH --CHAT FLAG                    │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │
         ┌──────────────────────────┼──────────────────────────┐
         ▼                          ▼                          ▼
  visual-grill-me            deep-research                post-mortem
  • Skips sidecar HTML       • Skips research board     • Skips triage canvas
  • Uses ask_question modal    HTML                     • Ingests backlog items
    or numbered Markdown     • Dispatches subagents       via Markdown tables
    options in chat            in background            • Conducts conversational
  • Immediate text capture   • Synthesizes findings       decision capture
    into PLAN.md               directly into chat       • Ingests into BACKLOG.md
```

#### 1. `visual-grill-me --chat`
- **Canvas Generation**: Skipped entirely. Zero HTML or state JSON files written.
- **Question Presentation**:
  - If interactive modals are preferred: triggers `ask_question` with selectable options.
  - If running in modal-free mode: outputs clean numbered Markdown options (`[1] Option A`, `[2] Option B`, `[3] Option C`) directly in chat.
- **Decision Capture**: The agent directly extracts user responses from chat and immediately updates working notes or `PLAN.md`.

#### 2. `deep-research --chat`
- **Board Generation**: Skipped entirely. Zero standalone research board HTML written.
- **Subagent Delegation**: Subagents (`research` Pro, `explore` Flash, `contrarian` Advocate) still execute in the background with full capability.
- **Result Presentation**: Upon subagent return, findings are synthesized into a high-density Markdown report directly in chat:
  - Executive Overview
  - Technical Derivations / Mathematical Proofs
  - Tradeoff Table (PCDA: Pros, Cons, Devil's Advocate, 80/20 Resolution)
  - Recommended Next Steps

#### 3. `post-mortem --chat`
- **Triage Canvas**: Skipped entirely.
- **Triage Flow**:
  - Presents a compact Markdown roster of pending items with assigned recommendations.
  - Allows bulk text approval (e.g. *"approve 1, 3; kill 2; table 4"*) or single-item sequential review.
  - Updates `docs/BACKLOG.md` and `docs/history/DEV_HISTORY.md` directly upon completion.

---

## 5. Domain Routing & Communiqué State Machine

To prevent cross-discipline contamination and accidental file clobbering, macro orchestrators strictly respect domain boundaries:

```
┌────────────────────────────────────────────────────────────────────────┐
│                     MACRO PIPELINE DISPATCH GATE                       │
│                     (Target Blueprint in PLAN.md)                      │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │
            ┌───────────────────────┴───────────────────────┐
            ▼                                               ▼
 TARGET: FACTORY_FLOOR                            TARGET: TOOLING_HARNESS
 (Klang Industries)                               (New Klang City)
 • C++ Audio DSP Source                           • Python / PowerShell Automation
 • JUCE GUI Components                            • Documentation & Catalogs
 • CMake / MSBuild Native Targets                 • N'kai Sidecars & HTML Canvases
 • APVTS Parameter Schemas                        • Agent Skills & Pipeline Tooling
 • Audio Thread Safety Audits                     • Subagent Delegation via Flash
            │                                               │
            ▼                                               ▼
 plan_to_build.md ➔ build_to_plan.md               subagent_dispatch.md ➔ subagent_receipt.md
```

### 5.1 Factory Floor Routing (`TARGET: FACTORY_FLOOR`)
- **Recipient**: Klang Industries (C++ Factory Floor).
- **Toolchain**: Native MSBuild, CMake, Clang, Catch2/JUCE UTs, `audiothread-guard`, `build-validate`.
- **Signaling**: Sourced through `.agents/pipeline/communique/plan_to_build.md` and resolved via `build_to_plan.md`.
- **Constraint**: Klang Industries NEVER ingests tooling, documentation, or HTML sidecar tickets.

### 5.2 Tooling Harness Routing (`TARGET: TOOLING_HARNESS`)
- **Recipient**: New Klang City (Tooling Harness / Flash Builder Subagent).
- **Toolchain**: Python scripts, Node.js packages, Markdown specs, HTML/CSS sidecars.
- **Signaling**: Sourced through `.agents/pipeline/communique/subagent_dispatch.md` and resolved via `subagent_receipt.md`.
- **Constraint**: Tooling harnesses NEVER modify native C++ audio rendering loops or touch real-time DSP pipelines.

---

## 6. Verification Gauntlet & Compliance Matrix

Every skill implementation in `.agents/skills/` and `c:\Dev\nkai\bundle\skills/` must pass this verification gauntlet:

```
┌────────────────────────────────────────────────────────────────────────┐
│                      SKILL COMPLIANCE VERIFICATION                     │
├───────────────────────────────────┬────────────────────────────────────┤
│ 1. Documentation Indexing         │ • Cataloged in DOCS_CATALOG.json   │
│                                   │ • Linked in docs/SYSTEM_MAP.md     │
├───────────────────────────────────┼────────────────────────────────────┤
│ 2. Dual-Storage & Link URI        │ • Dual path: brain/ + .agents/     │
│                                   │ • Link points to brain artifact    │
├───────────────────────────────────┼────────────────────────────────────┤
│ 3. Ready Handshake Standard       │ • Explicit prompt to open in side  │
│                                   │ • Pauses until "ready" confirmed   │
├───────────────────────────────────┼────────────────────────────────────┤
│ 4. Fallback Flag Standard         │ • Supports --chat and --text-only  │
│                                   │ • Clean degradation to chat text   │
├───────────────────────────────────┼────────────────────────────────────┤
│ 5. Bundle Parity                  │ • 100% sync to nkai/bundle/skills/ │
│                                   │ • audit_sidecar.py passes (code 0) │
└───────────────────────────────────┴────────────────────────────────────┘
```

This ensures a robust, portable, and predictable developer experience across any AI coding environment.
