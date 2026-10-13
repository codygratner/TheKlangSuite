# ⚡ The Klang Suite: Master Workflow & Architecture Cheat Sheet

> **Single-Page Developer Reference Card**: The essential mental model, acronyms, aviation metaphors, and toolchain commands across The Klang Suite ecosystem.

> [!TIP] 🧠 Memory & Process Check: What is `language_server.exe`?
> If Windows Task Manager reports `language_server.exe` consuming ~1.5 GB+ RAM, **it is NOT N'kai**! It is Google Antigravity's core background Language Server Protocol (LSP) daemon indexing C++ symbols and powering the agent runtime. N'kai's sidecar is a pure HTML5/JS webview taking only ~30–50 MB. To flush the LSP symbol cache back to ~300 MB after multi-hour flights, simply restart Antigravity or execute a clean `/clear`.

---

## 🏛️ The Five Ecosystem Pillars

| Pillar | Workspace Path | Role & Technology | Repository Scope |
| :--- | :--- | :--- | :--- |
| **`TheKlangSuite`** | `c:\Dev\TheKlangSuite` | Desktop Audio Plugin Suite (C++20, JUCE 9.0.3, VST3/AU) | Native Audio Factory Floor |
| **`ToadTracker`** | `c:\Dev\ToadTracker` | Embedded Hardware Tracker & Groovebox Engine (C++20, HAL) | Hardware Firmware & Embedded DSP |
| **`nkai`** | `c:\Dev\nkai` | Asymmetric Sidecar Framework & Interactive Triage Lab | Webview UI & CLI Harness Tools |
| **`TheKlangResearch`** | `C:\Dev\Research` | Eternal DSP Mathematics, Central RFCs & Soul Harvests | Continuous Living Archive (Git is Changelog; Zero SemVer) |
| **`TheKlangVault`** | `C:\Dev\TheKlangVault` | Private Studio Hub, Raw Notes Mailbox & Pydantic Pipelines | Private Obsidian Vault (Git-backed) |

---

## 🎛️ Product Acronym Registry (`TK<X>`)

*Every plugin, application, and hardware module has an immutable 3-letter acronym:*

* **`TKS`** — **The Klang Suite** *(Umbrella project, shared libraries, and `TKS_LOG` logging tags)*
* **`TKF`** — **The Klang Farmer** *(Dual-FM percussion and bass synthesizer)*
* **`TKP`** — **The Klang Planter** *(Multi-layer percussive sound machine)*
* **`TKM`** — **The Klang Mill** *(1x6 modular multi-effects pedalboard rack)*
* **`TKB`** — **The Klang Box** *(Embedded hardware synth: Daisy Seed / Teensy 4.1 / dadamachines TBD-16)*
* **`TKR`** — **The Klang R1** *(7-voice Electribe ER-1 tribute rhythm synthesizer)*
* **`TKC`** — **The Klang Cultivator** *(Standalone MIDI CC / CV / MPE modulation generator rack)*
* **`TKE`** — **The Klang Editor** *(Standalone JUCE JSON schema & curve inspection app)*

---

## 🛫 The Aviation Flight-Deck Metaphor

*Replacing corporate Agile jargon with spatial, intuitive engineering concepts:*

```
  [ Docket ] ──> [ Holding Pattern ] ──> [ Flight Plan ] ──> [ Runway ] ──> [ Wheels Up ] ──> [ Touchdown ]
  (Pre-flight)    (Staged Drafts)         (Active PLAN.md)    (Prerequisites) (Execution)       (Archived)
```

* **`Flight` 🛫**: A single discrete, bounded implementation step or milestone increment (`Flight N`).
* **`Flight Plan` 📋**: The synthesized, topologically ordered implementation plan (`PLAN.md`).
* **`Runway` 🛫**: The prerequisite sequence of foundational flights built before a major system can launch.
* **`Test Flight` / `Test-Drive` 🧪**: Taking an interactive sidecar or tool out for a spin using realistic data.
* **`Docket` 📑**: The curated queue of active topics/notes pre-flighted and ready for triage review.
* **`Holding Pattern` ⏳**: Staging draft plans in `.agents/pipeline/plans/drafts/` until the factory is idle.
* **`Re-sequence / Rejigger` 🔄**: Topological dependency sorting ensuring foundations precede surfaces.
* **`Wheels Up` 🚀**: Formally starting implementation on an approved flight plan (`STATUS: IN_PROGRESS`).
* **`Touchdown` 🛬**: Passing all tests, verifying acceptance criteria, committing cleanly, and archiving.

### Operatic Flight Stages (Acts & Flight Tiers)
* **Act I: Ground Control & Taxiway (Early Flights)** 🛫: Queues, zonal READMEs, git hardening, environment provisioning.
* **Act II: Ascent & Cruising Altitude (Main Flights)** ✈️: Operational data engines, Pydantic validation, linters, local APIs.
* **Act III: High-Altitude Reconnaissance & Gauntlet (Late Flights)** 🛰️: Field guide decoupling, universal manifestos, soul harvests.
* **Act IV: Final Approach & Touchdown (Pre-Release Capstone)** 🛬: Release verification gauntlet, catalog sync, SemVer tagging.

---

## 🛠️ Essential Slash Commands & Micro-Skills

| Trigger Command | Skill / Role | What It Does |
| :--- | :--- | :--- |
| **`/mailbag`** | `check-mail` | Scans Obsidian Mailbox, updates `mail_queue.json`, and renders interactive sidecar triage lab. |
| **`/audit-reorder`** | `audit-reorder` | Synthesizes and topologically rejiggers active audit/triage flights (2-Pass Convergence Gate). |
| **`/plan`** | Planning Gate | Drafts a structured implementation plan (staged to `.agents/pipeline/plans/drafts/`). |
| **`/grill-me`** | `visual-grill-me` | Conducts interactive design interview paired with live side-panel HTML visual mockup. |
| **`/read-plan`** | Ingestor | Ingests active `PLAN.md`, verifies model tier, transitions dispatch signal to `IN_PROGRESS`. |
| **`/execute-task`** | Factory Pipeline | Autonomous factory loop: Edits $\to$ `audiothread-guard` $\to$ `build-validate` $\to$ `step-verify`. |
| **`/step-verify`** | Gatekeeper | Audits code against phase acceptance criteria, updates checklist checkmarks, stages commits. |
| **`/task-finish`** | Closeout | Validates full checklist completion, conventional git commit, archives `PLAN.md` to `completed/`. |
| **`/rollback-plan`** | Plan Queue | Instantly restores previous `PLAN.md` state from `.agents/pipeline/plans/snapshots/` in 5ms. |
| **`/clean-plan`** | Hygiene | Purges temporary planning artifacts (`context_snapshot*.md`) and archives finished plans. |
| **`/build-validate`** | Compiler Suite | Headless CMake compilation, error filtering, and test execution (`dsp_tests`, `pluginval`). |
| **`/audiothread-guard`**| Audio Safety | Strict AST/regex audit checking audio path for heap allocations, mutexes, and blocking I/O. |
| **`/web-qa`** | Sidecar QA | Audits interactive sidecars in Headless Chrome for DOM balance, JS errors, and audio sandbox. |
| **`/update-docs`** | Sync Gauntlet | Reconciles Backlog, Catalog, Dev History, System Map, and mirrors Obsidian Vault. |

---

## 🧠 Model Tiers & Quota Economics

*The Orchestrator Invariant: Main chat session stays permanently on Tier 2 Flash High.*

* **Tier 1 (High Reasoning / Critical DSP)**: `Claude 5.5 Opus (Thinking: High)` *(or Gemini 4 Argon upon GA)*. Reserved strictly for deep DSP differential equations, non-linear saturation proofs, lock-free ring-buffer concurrency, and 2-strike factory escalations.
* **Tier 2 (Flash High)** *(Universal Daily Driver)*: Standard JUCE UI components, APVTS bindings, unit test expansions, and standard phase execution.
* **Tier 3 (Flash Low/Medium)**: Mechanical find-and-replace, documentation updates, git commits, and JSON schemas.
* **The Targeted Opus DSP Swap Protocol**: Main chat stays on Gemini 3.8 Flash High for 95% of daily engineering. When kicking off heavy DSP or on a 2-strike factory escalation, the agent emits `🧠 MODEL ADVISORY: Tier 1` prompting the user to manually swap to Claude 5.5 Opus High in the IDE footer.
* **Mandatory Downgrade Chime (`🟢 TASK CONCLUDED`)**: Upon completing a Tier 1 phase or during `task-finish`, the agent MUST emit a prominent reminder prompting the user to swap back to Gemini 3.8 Flash High, immunizing against the "Forgetful Downgrade Trap" and preserving Opus allowance.
* **Gemini 4 Argon GA Reconvene Trigger**: When Gemini 4 Argon reaches public General Availability in the IDE dropdown, immediately reconvene to benchmark Argon vs. Opus 5.5 High on C++ DSP to determine the primary Tier 1 engine.
* **The "Orchestrator + Subagents" Rule**: Main chat never burns premium allowance unnecessarily; delegates background deep dives to on-demand subagents (`invoke_subagent`).
* **The Subagent Swarm**:
  * `research` (`Model="pro"`): Targeted Tier 1 Sprint for deep DSP differential equations, non-linear saturation proofs, and multi-file architectural synthesis.
  * `explore` (`Model="flash"`): Autonomous Tier 2 Marathon for broad UI/UX surveys, community sentiment, and competitive benchmarking.
  * `contrarian` (`Model="flash"`): The Devil's Advocate adversarial mode stress-testing proposals, failure modes, and cognitive debt.
* **Ambient Status Footer & 3-Surface Navigation Invariant**:
  Responses conclude with a 2-line ambient status footer:
  Line 1: `Focus: <Current Focus> | TKS-<ver> · TT-<ver> · NK-<ver> · TR-<ver>`
  Line 2: `[ 📬 Mailbag ](sidecar://nkai/panel/?board=morning_mail) · [ 🛫 Runway ](sidecar://nkai/panel/?board=runway) · [ 📝 Scratchpad ](file:///C:/_/Vault/Inbox/SCRATCHPAD.md)` *(+ `· [ 🔬 Lab: <topic> ](...)` when active)*.
* **The Scratchpad Protocol**: Dedicated [`C:\_\Vault\Inbox\SCRATCHPAD.md`](file:///C:/_/Vault/Inbox/SCRATCHPAD.md) file for live reading notes and multi-point stream-of-consciousness chat reactions, structured with `# Multi-Point Replies` and `# Quick Notes`, permanently excluded from mailbag scans and directly accessible via the chat footer or *"Check scratchpad"* prompt.

---

## ⚡ Audio Thread Invariants (CRITICAL)

* **Zero Allocations**: Never call `new`, `malloc`, `free`, or resize containers (`push_back`, `Array::add`) in `processBlock()`. Pre-allocate in `prepareToPlay()`.
* **Zero Locks**: Never acquire a `std::mutex`, `std::lock_guard`, or `CriticalSection` in the audio thread. Use lock-free atomics (`std::atomic`) or bounded FIFOs.
* **Zero Blocking I/O**: Never call filesystem, network, or console output (`std::cout`, `DBG()`, `printf`) in the audio path.
* **FastMath**: Prefer `TbdAudio::FastMath` over standard CRT transcendentals (`std::sin`, `std::pow`, `std::tanh`).

---

## 🛡️ Obsidian Vault & Sync Guardrails

* **Independent Git Repository**: `TheKlangVault` is an initialized, Git-tracked private hub.
* **App Ignores**: Defined in `TheKlangVault/.obsidian/app.json` (`userIgnoreFilters` excludes `.git/`, `tools/`, `*.py`, `*.pyc`).
* **Obsidian Sync Invariant**: Obsidian Sync natively ignores all dotfiles (`.git/`, `.gitignore`, `.venv/`, `.agents/`). Git will never trigger sync panic loops.
* **Agent Ownership**: Agents automatically maintain `.gitignore` and `app.json` whenever creating tools or staging files.

---

## 🏗️ Architectural Zonal Boundaries (The 3-Zone Standard)

* **Zone 1: Native Audio DSP (`Source/dsp/README.md`)**: Real-time safety, zero allocations in `processBlock()`, zero locks, zero blocking I/O, SIMD `FastMath`.
* **Zone 2: Data Schemas & Assets (`assets/README.md`)**: 4-layer parameter separation (`controls/`, `layouts/`, `themes/`, `text/`), normalized ranges $[0.0, 1.0]$.
* **Zone 3: Tooling Harness (`tools/README.md`)**: Dev scripts, sidecars, zero external runtime dependencies (Python standard library only, UTF-8 stdlib).
* **Anti-Pattern Guard**: Strictly NO per-folder blackbox files. Follow the 3-Zone boundary to prevent README cemeteries and token bloat.

---

## 📜 Permanent Documentation Invariant

* **Mandatory Dual-File Synchronization**: Whenever terminology, workflow metaphors (such as aviation flight stages), agent skills, or system guardrails are created, renamed, or modified, the agent MUST proactively update both [`docs/GLOSSARY.md`](file:///c:/Dev/TheKlangSuite/docs/GLOSSARY.md) and [`docs/CHEAT_SHEET.md`](file:///c:/Dev/TheKlangSuite/docs/CHEAT_SHEET.md) in the exact same commit pass.

---

## 🏺 Telescoping Soul Harvest Standard

* **Opportunistic Post-Flight Staging**: Drop raw notes/gotchas into `.agents/pipeline/staging/souls/` immediately (0 tokens, fresh memory).
* **Batch Post-Plan Distillation**: Formalize staged souls into `TheKlangResearch` RFCs during deep docs passes, wiping staging to Inbox Zero.
* **Noise Filter**: Never harvest routine mechanical changes; only stage novel architecture, unexpected gotchas, and PCDA dialectics.

---

## 📚 The "Field Guides Always Last" Invariant

* **Synthesis Sequence**: In deep docs sweeps (`/update-docs --deep` / Act III), ingest souls, autopsies, and ground specs *first*; synthesize Field Guides & Manifestos **LAST**.
* **Panoramic Synthesis**: Complete bird's-eye context empowers bold rewriting and creation of new mental models rather than piecemeal bullet additions.
* **History Safety**: Full Git and Obsidian sync history eliminates fear of radical refactoring.
 
---
 
## 💾 Rolling Blackbox Context Compaction (Topic #7)

* **Compaction Tripwires**: Triggered at Act/Flight boundaries, when chat transcript exceeds ~150k tokens, or strictly *before* executing `/clear`.
* **The Automated 4-Step Flush**: Run `python tools/flush_memory.py --role "<Role>" --summary "<Objective>"` (0 tokens).
  1. Record achievements in `docs/history/DEV_HISTORY.md` (auto-scrapes git & plan).
  2. Stage gotchas/autopsies in `.agents/pipeline/staging/souls/`.
  3. Synchronize `ACTIVE_CONTEXT.md` (<600 bytes) and `.agents/pipeline/context_state.json` via `tools/state_manager.py`.
  4. Validate links via `python tools/verify_context_clues.py`.
* **The 0-Turn Cold Boot Invariant**: Boot fresh sessions cold from `ACTIVE_CONTEXT.md` in exactly **0 turns** without asking orientation questions.

---

## 🗄️ Obsidian Archive Index & Asynchronous Enrichment Standard

* **Mechanical Lean Index**: `TheKlangVault/Inbox/Archive/INDEX.md` maintained deterministically by `tools/check_mail.py` (0 tokens). Table format: `Title`, `Date`, `Priority`, `Pillar`, `Link` (<200 lines).
* **Zero Live Note Rewriting (Curator's Trap Defense)**: Never edit raw archived notes mid-session or during live triage; record verdicts in `mail_queue.json` in <5ms.
* **Deferred Note Enrichment**: Batch back-propagation of resolution footers (status badges, pillar crosslinks, PCDA summaries) into archived notes strictly under `/update-docs --deep` via Flash subagents.
* **Single Source of Truth**: `docs/BACKLOG.md` is canonical; `TheKlangVault/Inbox/Archive/` is historical provenance.

---

## 🔎 Trust, But Verify (Pre-Flight Grounding Rule)

* **Verify premises before building on them**: local first (`CMakeLists.txt`, `GEMINI.md`, version headers, the file itself), web second (model/vendor/upstream claims).
* **Wrong premise?** Lead with a polite one-line reality check, then answer.

---

## 🪞 One-Way Vault Mirror (ABSOLUTE)

* **Repo → vault only.** Nothing in `TheKlangVault` is ever synced/copied back into a repo; vault edits outside `Inbox/` get overwritten.
* **Inbox exception:** notes are *read* by `check_mail.py` / `ramble_ingest.py` into repo ledgers; the files never enter a repo.

---

## 🛡️ Audit Cadence & Checkpoint Resumption
 
* **Light vs Deep**: Light audit at every patch tag (Flash High); Deep Opus audit once per minor version (`v0.4.0`, `v0.5.0`) or before quota reset.
* **Checkpoints**: Progress saves to `audit_checkpoint.json` per repo; resume in 0 turns if quota cuts off. Findings land on an N'kai board.

---

## 🧭 The North Star Focus & Ambient Status Line (Mailbag `rj-09`)

* **The Living Compass (`docs/FOCUS.md`)**: Single source of truth for active project precedence (`N'kai Solidification` → `TKS v0.4.0 UI Shells / v0.5.0-dev Sound & Chaos` → `ToadTracker PoC`) and strict Negative Architecture (what we are *not* doing right now).
* **Gentle Re-Centering Nudge**: A single-sentence compass check after research detours: `🧭 North Star Check: Returning from [Detour] to active focus: [Target]` (nudge, never a blocking gate).
* **Ambient Status Footer**: The single line at the end of every response:
  `Focus: <Current Focus> | TKS-<ver> · TT-<ver> · NK-<ver> · TR-<ver>`
  (append `· [ 📬 Open N'kai Board ](sidecar://nkai/panel/)` only when an N'kai board session is active).

---

## 🔤 Universal UTF-8 Invariant (Mailbag `rj-02`)

* **Strict Stream Reconfiguration**: All Python entrypoints configure `sys.stdout/stderr.reconfigure(encoding='utf-8')` to prevent `cp1252` encoding crashes on Windows.
* **Explicit File I/O**: Every file read/write uses explicit `encoding='utf-8'` (fallback safely with `errors='replace'`, never `latin-1`).
* **Linter Command**: `python tools/verify_utf8.py --all --strict` audits all Python tools across `TheKlangSuite`, `nkai`, and `bundle/`.

---

## 🧪 Sandbox Mode & Multi-Stage Environments (Flight 16+17 / `rj-03`)

* **Sandbox CLI**: `python tools/check_mail.py --sandbox --no-browser` runs fully isolated with synthetic fixtures, zero persistence, and output to `.agents/sidecar/sandbox_mail_inbox.html`.
* **The 80/20 Resolution**: One codebase, two runtime modes (`SANDBOX` vs `PRODUCTION`), and Git branches for staging (`feat/*` -> `0.4.0-dev` -> release tags), replacing enterprise server bloat.
* **Deterministic Element IDs**: Action chips (`btn-<action>-<id>`) and filter pills (`pill-<rail>-<slug>`) provide stable DOM anchors for automated testing.
* **Mock Action Echoing**: Actions in sandbox prepend `[QA_TEST_MOCK]`, bypass API persistence, and namespace local storage to `nkai_sbx_mail_triage_actions`.

## 🎨 Unified Master Template & Schema-Driven Boards (Flight 15 / `rj-04`)

* **Master Template**: `c:\Dev\nkai\core\master_template.html` canonical standalone shell combining Two-Tier header, live allowance meter, audio haptics, hero stage, and in-canvas 4-lens accordions with zero dead loopback bridge code.
* **BoardSchema**: Pydantic model (`tools/models.py`) and JSON schema (`c:\Dev\nkai\core\schema.json`) parameterizing board metadata, rails, and card streams.
* **Renderer Engine**: `tools/board_renderer.py` provides `render_board(schema, out_path)`, eliminating ~1,500 lines of monolithic HTML f-string from `check_mail.py` for zero-HTML-edit board generation.

---

## 🗂️ Multi-Board Hub & Session Manager (Flight 14 / `rj-05`)

* **Board Switcher Button & Modal**: `#btn-board-switcher` in Top-Tier Header triggers accessible modal `#board-switcher-modal` with search filter, active boards group, and archived sessions group.
* **BoardRegistry Data Model**: Pydantic schema in `tools/models.py` backed by canonical `.agents/pipeline/boards/registry.json`.
* **Strict Anti-Cache Backend**: `sidecars/panel/main.mjs` injects `Cache-Control: no-store, no-cache, must-revalidate, max-age=0`, `Pragma: no-cache`, `Expires: 0` headers, plus `GET /api/boards` and query routing (`?board=<id>&_t=<timestamp>`).
* **Registry CLI**: `python tools/board_registry.py list|register|rescan` manages ecosystem board discovery and card counts.

---

## 📬 Mailbag Intake & Triage v2 (Flight 7 / `rj-07` & `rj-17`)

* **Ramble Overflow Queue**: Notes with >3 ideas keep top 3 active; remainder written to `overflow.json` with linked `split_group`; displays `⤵ N in overflow` chip and Rail 2 `Overflow` filter pill.
* **Filename-Only Quick Notes**: 0-byte or whitespace-only files use filename stem as title; tagged `quick-note`; preview/body set to `(quick note — title only)`.
* **Agent Tabled Folder (`Inbox/Tabled/`)**: Dedicated folder for agent-authored notes; tagged `tabled`; rendered with `🗂 Tabled by agent` chip; protects human inbox purity.
* **Attention Tags & Session Launchers**: Markdown markers (`!IMPORTANT`, `!BLOATED`, `!INCOMPLETE`, `!SPIKE`) parsed into attention badges; card footer renders `[ 🔬 Grill-Lab ]`, `[ 🧠 Deep Research ]`, `[ 🔨 Draft Plan ]` launchers with extension IPC / clipboard fallback.
* **`/mail-add` CLI**: `python tools/mail_add.py <title> --after <id> | --rank <n>` splices into queue with contiguous rank renumbering and generates structured inbox notes.

---

## 💬 Per-Card Response Drawer (Flight 19 / `rj-08`)

* **In-Card Reasoning (`agent_reply`)**: Attaches rich Markdown assessments, PCDA matrices, code signatures, and benchmark targets directly to cards, displayed in an accessible collapsible drawer (`#drawer-<id>`).
* **Chat-Minimal Pair Programming**: Preserves context window capacity by keeping chat responses to 1–2 crisp status lines (`"Triaged rj-08 to Backlog. Architectural breakdown committed to card drawer."`).
* **Deterministic Drawer Hierarchy**: Accessible button `#btn-drawer-<id>` with dynamic `aria-expanded`, content wrapper `#drawer-<id>` with `hidden` attribute, and `.drawer-inner.markdown-body`.
* **Fast CLI Reply Tooling**: `python tools/check_mail.py --reply-card <id> --text "<markdown>"` commits replies to `mail_queue.json` and regenerates the HTML board in <50ms without vault re-scanning.
---

## 😈 Devil's Advocate Methodology & Prompt Hardening (`rj-22`)

* **Method B+ Constructive Adversarial Invariant (Dialectical Inquiry)**: Mandatory requirement for all DA reviews, `/grill-me` sessions, and subagent dialectics to append both: (1) `"Where I Disagree With The Prompt"` (skepticism of flawed premises), and (2) `"Steel-Manned Intent & Constructive Lateral Alternatives"` (inventing viable, low-cost or zero-token architectural solutions). Eliminates the "Cynical Auditor Trap" where critics tear down a proposal without offering a path forward.
* **Pre-Downgrade Soul Harvest**: Mandatory single-turn capture of raw Opus working hypotheses and failure modes to `.agents/pipeline/staging/souls/` immediately before sounding the downgrade chime back to Flash, capturing peak reasoning at near-zero token cost.
* **Audio Strangler Fallacy**: Real-time audio engines cannot use runtime Strangler Fig routing because multiple engines cannot concurrently own a hardware ASIO/CoreAudio device stream; migrations must use Branch by Abstraction verified by offline-bounce golden null tests.
* **Benchmark Tooling**: `python tools/da_bench.py` provides `sanitize` (seeded anonymization & key sealing), `audit-blindness` (zero-leak transcript verification), `aggregate` (multi-judge Kendall's tau & composite scoring), and `manifest` lifecycle governance.

---

## 📱 Mobile Companion & Touch Ergonomics (Milestone v0.3.0 / Section 14)

* **Multi-Device Companion**: Zero-friction pair-programming, note triage, and PCDA reviewing on iOS Safari or Android Chrome over local Wi-Fi LAN.
* **44px Touch Target Law**: Minimum 44px x 44px tap targets on all card actions (`.card-btn`, `.action-chip`, `#btn-approve-*`, drawer toggles) on screens $\le$ 640px.
* **Sticky Bottom Thumb Bar**: Docked 4-target bar (`[ ⚪ Untriaged ]`, `[ 🚨 P0 ]`, `[ 📋 All ]`, `[ 🗂 Switch Board ]`) with `navigator.vibrate(15)` device haptics; hidden on desktop.
* **100dvh Viewport Stability**: Dynamic viewport height fallback eliminating mobile browser URL bar jumps.
* **Zero-Dependency Terminal & Web QR Pairing**: `python tools/nkai_serve.py --lan` detects local Wi-Fi IP, prints Unicode half-block QR code in terminal, and serves `/qr` SVG pairing endpoint for second monitors.

---

## 🌌 The Cosmos & Scoped Enclave (`rj-44`)

* **The Cosmos (`C:\_\Vault`)**: Unified master Obsidian Sync vault consolidating all personal, creative, solo TTRPG, and technical knowledge.
* **Air-Gapped Scoped Enclave (`New Klang City`)**: Dev tools and Git operate strictly inside `C:\_\Vault\🏢 New Klang City` via junction `C:\Dev\TheKlangVault`. Personal notes remain 100% blind to Git.
* **Lore Dispatch System**: Conduit between Stellar Zenith (The Cosmos / Codex) and Subterranean Foundry (Mayor Toad / Klang Industries), inspected via `python tools/check_lore.py`.

---

## 🛰️ Inter-Project Data Bus (`Dispatches/` & `tools/bus.py`)

* **Universal Handoff Bus**: Zero-daemon, zero-lock file queue across TKS, TT, NK, TR, and The Cosmos over NTFS junction `C:\Dev\TheKlangVault\Dispatches\` $\longleftrightarrow$ `C:\_\Vault\🏢 New Klang City\Dispatches\`.
* **Standard Packet Format**: Markdown file with YAML frontmatter (`id`, `date`, `from`, `to`, `tags`, `priority`, `status`, `title`).
* **Packet Lifecycle**: `PENDING` $\to$ `ACKNOWLEDGED` $\to$ `ARCHIVED` (in `Archive/`).
* **Universal Bus CLI**: `python tools/bus.py list [--for <PILLAR>]` | `python tools/bus.py post --to <PILLAR> ...` | `python tools/bus.py read <id>` | `python tools/bus.py ack <id> [--archive]`.
* **Live Agenda Flight Deck**: `C:\_\Vault\Agenda.md` reflects live bus dispatches and active 5-pillar queue.
* **Intent Locks & Concurrency Telemetry**: Ephemeral multi-agent filesystem locks with dead-man's auto-TTL: `python tools/bus.py lock --scope <paths> --activity "<desc>" [--ttl <min>]` | `python tools/bus.py unlock` | `python tools/bus.py locks` | `python tools/bus.py check-scope --scope <paths>`. Emits collision warnings directly into `tools/state_manager.py` 0-turn cold boot state card.

---

## ✍️ Multipoint Prompting Protocol ("Reading & Responding")

* **Format**: Simple numbered blocks separated by a blank line:
  ```markdown
  1)
  First topic or feedback on a section...

  2)
  Second thought, question, or approval...
  ```
* **Optional Excerpts**: Quote the agent's prior text with `> "quote"` directly below the number.
* **Zero Formatting Tax**: No need for markdown headers or bold titles—simple `1)`, `2)`, `3)` allows the agent to address every point with 1:1 fidelity without missing sub-clauses or getting confused.

---

## 🔧 The Tooling Ratchet Invariant (Zero-Token Immune System)

* **Code the Invariant, Not Just the Fix**: When an agent detects a regression, invariant violation, or repetitive manual chore, the agent **MUST** update the relevant Python CLI tool (`tools/audit_audiothread.py`, `tools/lint_frontmatter.py`, `tools/catalog_sync.py`, `tools/verify_utf8.py`) and author a unit test in `tools/tests/`.
* **Zero Token Churn**: Once encoded into Python, the check runs in sub-10ms for 0 LLM tokens, permanently immunizing the codebase against that failure class.

---

## 🔄 Rejiggering: The Topological Dialectical Sorter (TDS)

* **The Metaphor**: "Rejiggering" is our conversational term for transforming a messy, unranked bag of items into a linear, zero-drag execution sequence.
* **The Formal Pattern**: **Topological Dialectical Sorting (TDS)**:
  $$\text{Rejiggering} = \text{TopologicalSort}(\mathcal{G}) \circ \text{HegelianDialectic}(\mathcal{I})$$
* **The 4 Steps**:
  1. *The Unranked Bag*: Gather candidate items without arbitrary initial ranking.
  2. *Method B Adversarial Dialectic*: Stress-test each item through Thesis $\to$ Friction $\to$ Antithesis $\to$ 80/20 Resolution (with *"Where I Disagree With The Prompt"*).
  3. *DAG Prerequisite Extraction*: Identify hard technical dependencies (infrastructure before UI, saturation before filters).
  4. *Contiguous Critical Path*: Assign contiguous ranks `1..N` where every step has an explicit "Why Here?" causal rationale.
* **When to Call for a Rejigger**: Whenever facing backlog paralysis (>20 unranked tickets), flight deadlocks (missing unbuilt dependencies), or signal-chain aliasing (audio graph out of order).
* **Reference**: [Topological Dialectical Sorting RFC](file:///C:/Dev/Research/agentic/topological_dialectical_sorting_the_rejigger_pattern.md) | [Spec](file:///c:/Dev/TheKlangSuite/docs/specs/topological_dialectical_sorting_rejigger.md).

---

## 🛫 The 3-Surface System & The Flight Runway (Flight 20)

* **The 3 Triage Surfaces**:
  1. **Surface 1 (Evergreen #1: The Mailbag — `morning_mail`)**: Daily capture stream with in-board `#quick-capture-input` bar and debounced `#btn-grab-mail` appending to `C:\_\Vault\Inbox/<YYYY-MM-DD>_Quick_Notes.md`.
  2. **Surface 2 (Evergreen #2: The Flight Runway — `runway`)**: Delivery horizon built by `tools/build_runway.py` from `docs/BACKLOG.md` Act III with `[ 🚀 Active Flights ]`, `[ 📦 Shipped & Milestones ]`, and `[ 📋 All ]` filter rails.
  3. **Surface 3 (Session Surface: The Lab — `lab`)**: Single active ephemeral research slot. Emits `#lab-all-clear-banner` celebration when cards reach 0; concluded via `tools/conclude_lab.py` staging harvests to `.agents/pipeline/staging/souls/`.
* **The Scratchpad Protocol**: Stream-of-consciousness reading notes file [`C:\_\Vault\Inbox\SCRATCHPAD.md`](file:///C:/_/Vault/Inbox/SCRATCHPAD.md) permanently excluded from mailbag harvesting.
* **Ambient Navigation Line**: `Focus: <Focus> | TKS-<ver> · TT-<ver> · NK-<ver> · TR-<ver> · [ 📝 Scratchpad ](file:///C:/_/Vault/Inbox/SCRATCHPAD.md) · [ 📬 Mailbag ](sidecar://nkai/panel/?board=morning_mail) · [ 🛫 Runway ](sidecar://nkai/panel/?board=runway)`.
* **Reference**: [Design Spec](file:///c:/Dev/TheKlangSuite/docs/specs/nkai_three_surfaces_and_runway_board.md).

---

## 🌐 Web Access State Machine & Dual-Track Dispatch (Flight 20 / PCDA)

* **Web Access State Machine**:
  1. `RESTRICTED` (Default): All web calls (`search_web`, `read_url_content`, `curl`) blocked.
  2. `QUERY_ONLY`: Single-query approval ("ok" / "yes").
  3. `SKILL_SESSION`: Session-wide approval ("ok for this session" / during `/deep-research`).
  4. `GLOBAL_TEMPORARY`: Permanent window approval ("ok globally").
  5. **Heavy / Aggressive PCDA Auto-Elevation**: Explicit requests for "heavy", "aggressive", "deep", or "strongest" PCDA, dialectic, or audit automatically elevate to `SKILL_SESSION` without prompt fatigue, displaying the mandatory `[ 🌐 Web Access: ACTIVE (SKILL_SESSION) ]` security footer.
* **Dual-Track Dispatch Contract**:
  - **Native AGY Sidecar Extension (`sidecar://nkai/panel`)**: Uses Native Zero-Copy IPC (`window.sidecar.agent.sendMessage`) to directly trigger the active workspace agent turn.
  - **External Browser & Mobile (`nkai_serve.py`, port 3030)**: Standard browser sandbox blocks IDE IPC; automatically falls back to HTTP `/api/action` queueing (`tools/bus.py` in `Dispatches/`) and clipboard banner for 0-token passive safety.













