# Context Clues: [Role Name / Agent Title]

> **Step 0 Bootstrapping**: Agents or subagents starting a session or waking up from a context refresh MUST read this document on Turn 1 to orient on the active milestone, immediate return agenda, architectural memory, canonical taxonomy, and inviolable guardrails without prompt bloat.

---

## 📌 Active SemVer, Branch State & Roadmap Sidecar Deep Links
- **Current Git Branch:** `0.4.0-dev`
- **Ecosystem SemVer Matrix:**
  - `The Klang Suite (TKS)`: `v0.4.0-dev`
  - `ToadTracker (TT)`: `v0.1.0`
  - `N'kai (NK)`: `v0.1.0`
  - `The Klang Research (TKR)`: `v0.1.0`
- **Roadmap Sidecar Deep Links**:
  - [The Klang Suite v0.4.0 Roadmap](file:///c:/Dev/TheKlangSuite/.agents/sidecar/roadmap_sidecar.html#tks-v0.4.0)
  - [ToadTracker v0.1.0 Milestone](file:///c:/Dev/TheKlangSuite/.agents/sidecar/roadmap_sidecar.html#tt-v0.1.0)
  - [N'kai v0.1.0 Sidecar Framework](file:///c:/Dev/TheKlangSuite/.agents/sidecar/roadmap_sidecar.html#nk-v0.1.0)
  - [The Klang Research Central Hub](https://github.com/codygratner/TheKlangResearch)
- **Active Working Milestone:** [Current milestone name and status]
- **Upcoming Target Milestone:** [Next scheduled milestone]

---

## 🎯 Immediate Return Agenda & Status
- **Current Objective:** [Primary objective of this role]
- **Execution State:** [IDLE / PLANNING / IN_PROGRESS / REVIEW]
- **Active Blueprint:** [`PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md)
- **Immediate Next Steps**:
  1. [Step 1 description]
  2. [Step 2 description]
  3. [Step 3 description]

---

## 🧠 Architectural Memory & "Unforgettables"
1. **Model Advisory & Quota Discipline**:
   - Universal default: **Gemini 3.8 Flash (Thinking: High)** for near-zero quota burn and rapid iteration.
   - Pro Tier 1 (`Model="pro"`) reserved strictly for high-reasoning DSP differential equations, non-linear saturation proofs, and complex architectural synthesis.
2. **Data-Driven Architecture (CRITICAL)**:
   - DSP parameters live exclusively in `assets/controls/*.json`.
   - Structural UI hierarchy lives in `assets/layouts/*.json`.
   - Visual styling and color schemes live in `assets/themes/*.json`.
   - Text, tooltips, and copy live in `assets/text/*.json`.
3. **State & Persistence Standards (Topic 6.16)**:
   - Zero embedded binary databases (zero SQLite, zero LevelDB, zero RocksDB).
   - 3-Tier State: Frontmatter Markdown (canonical knowledge) $\rightarrow$ Deterministic JSON packages (session interchange) $\rightarrow$ Browser `localStorage` (ephemeral client settings).
4. **Synthesis Engine & Memory Invariants**:
   - Continuous `_slope` controls exist natively on all envelopes.
   - Hardware Target: dadamachines TBD-16 (ESP32-P4 @ 400MHz, PSRAM budget).

---

## 📖 Canonical Taxonomy, Product Lineup & Essential Spec Links
- **Master Documentation Catalog**: [`docs/DOCS_CATALOG.json`](file:///c:/Dev/TheKlangSuite/docs/DOCS_CATALOG.json) (100% indexed documentation ledger).
- **System Architecture Map**: [`docs/SYSTEM_MAP.md`](file:///c:/Dev/TheKlangSuite/docs/SYSTEM_MAP.md) (high-level cross-repo index).
- **Architectural Taxonomy & Glossary**: [`docs/GLOSSARY.md`](file:///c:/Dev/TheKlangSuite/docs/GLOSSARY.md)
  - *Cards* = UI containers, *Modules* = DSP units, *Sliders* = meter bars, *Knobs* = rotary controls.
  - Acronym Registry: `TKS` (Suite), `TKF` (Farmer), `TKP` (Planter), `TKM` (Mill), `TKB` (Box/Hardware), `TKR` (R1), `TKC` (Cultivator).
- **Product Intent Matrix**: [`docs/architecture/product_lineup.md`](file:///c:/Dev/TheKlangSuite/docs/architecture/product_lineup.md) ("Stay in Your Lane" drift guardrail).
- **Universal Project Backlog**: [`docs/BACKLOG.md`](file:///c:/Dev/TheKlangSuite/docs/BACKLOG.md) (single source of truth for all planned work).
- **Core Design Specs**:
  - [N'kai Canonical State & Persistence Architecture](file:///c:/Dev/TheKlangSuite/docs/specs/nkai_canonical_state_architecture.md)
  - [The Klang R1 Modular Drum Synthesizer](file:///c:/Dev/TheKlangSuite/docs/specs/spinoffs/the_klang_r1.md)
  - [The Klang Mill Pedalboard Multi-FX](file:///c:/Dev/TheKlangSuite/docs/specs/spinoffs/the_klang_mill.md)
  - [TBD-16 Klang Seed Hardware Integration](file:///c:/Dev/TheKlangSuite/docs/specs/tbd16_klang_seed_effects.md)

---

## ⚡ Audio Thread / Workflow Invariants & Handshake Gates
1. **Audio Thread Invariants (CRITICAL)**:
   - **Zero Allocations**: Never call `new`, `malloc`, `free`, or resize dynamic containers (`vector::push_back`) in `processBlock()`.
   - **Zero Locks**: Never acquire `std::mutex`, `juce::CriticalSection`, or sleep on thread synchronization in the audio path.
   - **Zero Blocking I/O**: Zero filesystem, network, or console output (`DBG`, `printf`, `std::cout`) on the audio thread.
   - **FastMath**: Prefer `TbdAudio::FastMath` over CRT transcendentals (`std::pow`, `std::sin`, `std::tanh`).
2. **Factory Handshake Gate**:
   - Inspect `.agents/pipeline/communique/plan_to_build.md` $\rightarrow$ Transition status to `IN_PROGRESS`.
   - Execute phase by phase, validating acceptance criteria before updating `.agents/pipeline/communique/build_to_plan.md`.
   - Conclude task: archive `PLAN.md` to `.agents/pipeline/plans/completed/`, transition communique to `COMPLETED ✅`, and emit the "JOB'S DONE" signal.
3. **Subagent Delegation Protocol (The 20-Line / 1-File Rule)**:
   - *< 20 lines, 1 file*: Execute directly in-chat (Tier 2 Flash active).
   - *> 20 lines, multi-file, or running builds/tests*: Delegate to Flash subagent (`invoke_subagent(Model="flash")`).
   - *2-Strike Escalation*: If Flash fails 2 compilation/test attempts, pause and escalate to Pro (`Model="pro"`).
