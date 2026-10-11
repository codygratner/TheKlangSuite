# ⚡ The Klang Suite: Master Workflow & Architecture Cheat Sheet

> **Single-Page Developer Reference Card**: The essential mental model, acronyms, aviation metaphors, and toolchain commands across The Klang Suite ecosystem.

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

* **Tier 1 (Pro High)**: Reserved strictly for deep DSP differential equations, non-linear saturation proofs, lock-free ring-buffer concurrency, and complex multi-file architectural synthesis.
* **Tier 2 (Flash High)** *(Universal Daily Driver)*: Standard JUCE UI components, APVTS bindings, unit test expansions, and standard phase execution.
* **Tier 3 (Flash Low/Medium)**: Mechanical find-and-replace, documentation updates, git commits, and JSON schemas.
* **The "Orchestrator + Subagents" Rule**: Main chat never burns Pro allowance; delegates heavy math to on-demand Pro subagents (`invoke_subagent(Model="pro")`).
* **The Subagent Swarm**:
  * `research` (`Model="pro"`): Targeted Tier 1 Sprint for deep DSP differential equations, non-linear saturation proofs, and multi-file architectural synthesis.
  * `explore` (`Model="flash"`): Autonomous Tier 2 Marathon for broad UI/UX surveys, community sentiment, and competitive benchmarking.
  * `contrarian` (`Model="flash"`): The Devil's Advocate adversarial mode stress-testing proposals, failure modes, and cognitive debt.
* **Lean Chat Footer**: Chat footers provide direct 1-click side pane access via `[ 📬 Open N'kai Board ](sidecar://nkai/panel/)`, omitting legacy repetitive subagent labels while preserving the SemVer roadmap track.

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
* **The 4-Step Flush Protocol**:
  1. Record achievements in `docs/history/DEV_HISTORY.md`.
  2. Stage gotchas/autopsies in `.agents/pipeline/staging/souls/`.
  3. Harden `context_clues_plan.md`, `context_clues_build.md`, or `context_clues_subagent.md`.
  4. Validate links via `python tools/verify_context_clues.py`.
* **The 0-Turn Cold Boot Invariant**: Boot fresh sessions cold from `context_clues_*.md` in exactly **0 turns** without asking orientation questions.





