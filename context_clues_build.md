# Context Clues: Klang Industries (Implementation & Factory Floor)

> **Step 0 Bootstrapping**: Factory builders waking up from a context refresh or starting an implementation phase MUST read this document on Turn 1 to orient on active build directives, return agenda, architectural memory, canonical taxonomy, and factory handshake gates without prompt bloat.

---

## 📌 Active SemVer, Branch State & Roadmap Sidecar Deep Links
- **Current Git Branch:** `0.4.0-dev`
- **Ecosystem SemVer Matrix:**
  - `The Klang Suite (TKS)`: `v0.4.0-dev`
  - `ToadTracker (TT)`: `v0.1.0`
  - `N'kai (NK)`: `v0.2.0`
  - `The Klang Research (TKR)`: Continuous Living Archive (Zero SemVer)
- **Roadmap Sidecar Deep Links**:
  - [The Klang Suite v0.4.0 Roadmap](file:///c:/Dev/TheKlangSuite/.agents/sidecar/roadmap_sidecar.html#tks-v0.4.0)
  - [ToadTracker v0.1.0 Milestone](file:///c:/Dev/TheKlangSuite/.agents/sidecar/roadmap_sidecar.html#tt-v0.1.0)
  - [N'kai v0.1.0 Sidecar Framework](file:///c:/Dev/TheKlangSuite/.agents/sidecar/roadmap_sidecar.html#nk-v0.1.0)
  - [The Klang Research Central Hub](https://github.com/codygratner/TheKlangResearch)
- **Active Working Milestone:** `v0.4.0 "The Interface & Experience Update"` (🔨 In Factory Execution)
- **Recommended Factory Model:** Tier 2 (`Gemini 3.8 Flash (Thinking: High)`) — 🟢 SUSTAINABLE

---

## 🎯 Immediate Return Agenda & Status
- **Current Objective:** Flight 2: Open Source Anti-Slop Hardening & CI Safety Gauntlet (Card #29 / Section 15)
- **Execution State:** `COMPLETED` ✅
- **Active Blueprint:** [`PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md)
- **Factory Phase Status**:
  1. [x] **Phase 1: Real-Time Audio-Thread Memory Pre-Allocation** (`Source/ModularBlocks.h`)
  2. [x] **Phase 2: Universal Open-Source Governance Artifacts** (`CONTRIBUTING.md`, PR template)
  3. [x] **Phase 3: CI/CD Automation & Matrix Hardening** (`.github/workflows/ci.yml`)
  4. [x] **Phase 4: Local Regression & Validation Gauntlet** (`dsp_tests`, `gui_tests`, audio linter)
  5. [x] **Phase 5: Re-Run the Anti-Slop Audit & Scorecard Delivery** (10/10 EXCEED SOTA Verified)


---

## 🧠 Architectural Memory & "Unforgettables"
1. **Audio Thread Invariants (CRITICAL)**:
   - **Zero Allocations**: Never call `new`, `malloc`, `free`, or resize dynamic containers (`std::vector::push_back`, `juce::Array::add`) in `processBlock()`, `processStereo()`, or per-sample render loops. Pre-allocate in `prepareToPlay()`.
   - **Zero Locks**: Never acquire `std::mutex`, `juce::CriticalSection`, or wait on thread synchronization primitives in the audio path.
   - **Zero Blocking I/O**: Never call filesystem operations, network APIs, or logging/console output (`std::cout`, `printf`, `DBG()`, `juce::Logger`) on the audio thread.
   - **SIMD & Fast Math**: Prefer `TbdAudio::FastMath` over standard CRT transcendentals (`std::pow`, `std::sin`, `std::tanh`) in hot audio loops.
2. **Data-Driven Architecture (CRITICAL)**:
   - APVTS parameter definitions strictly in `assets/controls/*.json`.
   - Layout hierarchy strictly in `assets/layouts/*.json`.
   - Visual styles & color tokens strictly in `assets/themes/*.json`.
   - Text & localization strictly in `assets/text/*.json`.
3. **Test-Driven Guardrails & Dual-Configuration Gauntlet**:
   - Zero orphaned features: all new DSP or UI features must have tests in `test/dsp_tests.cpp` and `test/gui_tests.cpp`.
   - Full regression verification across both **Debug** and **Release** builds before release cut.
   - Always run `deploy.ps1` to sync `.exe` and `.vst3` artifacts to `current_build/` and system directories (`C:\Program Files\Common Files\VST3\`).

---

## 📖 Canonical Taxonomy, Product Lineup & Essential Spec Links
- **Master Documentation Catalog**: [`docs/DOCS_CATALOG.json`](file:///c:/Dev/TheKlangSuite/docs/DOCS_CATALOG.json) (100% indexed documentation ledger).
- **System Architecture Map**: [`docs/SYSTEM_MAP.md`](file:///c:/Dev/TheKlangSuite/docs/SYSTEM_MAP.md) (central ecosystem index).
- **Architectural Taxonomy & Glossary**: [`docs/GLOSSARY.md`](file:///c:/Dev/TheKlangSuite/docs/GLOSSARY.md)
  - *Cards* = UI containers, *Modules* = DSP units, *Sliders* = meter bars, *Knobs* = rotary controls.
  - Acronym Registry: `TKS` (Suite), `TKF` (Farmer), `TKP` (Planter), `TKM` (Mill), `TKB` (Box/Hardware), `TKR` (R1), `TKC` (Cultivator).
- **Product Intent Matrix**: [`docs/architecture/product_lineup.md`](file:///c:/Dev/TheKlangSuite/docs/architecture/product_lineup.md) ("Stay in Your Lane" drift guardrail).
- **Universal Project Backlog**: [`docs/BACKLOG.md`](file:///c:/Dev/TheKlangSuite/docs/BACKLOG.md)
- **Active Technical Specs**:
  - [N'kai Canonical State & Persistence Architecture](file:///c:/Dev/TheKlangSuite/docs/specs/nkai_canonical_state_architecture.md)
  - [The Klang R1 Modular Drum Synthesizer](file:///c:/Dev/TheKlangSuite/docs/specs/spinoffs/the_klang_r1.md)
  - [The Klang Mill Pedalboard Multi-FX](file:///c:/Dev/TheKlangSuite/docs/specs/spinoffs/the_klang_mill.md)
  - [TBD-16 Klang Seed Hardware Integration](file:///c:/Dev/TheKlangSuite/docs/specs/tbd16_klang_seed_effects.md)

---

## ⚡ Audio Thread / Workflow Invariants & Handshake Gates
1. **The Factory Handshake Gate**:
   - Inspect `.agents/pipeline/communique/plan_to_build.md` $\rightarrow$ Transition status to `IN_PROGRESS`.
   - Report progress and telemetry via `.agents/pipeline/communique/build_to_plan.md`.
   - Never use interactive modals (`ask_question`) during factory execution (freezes IDE dropdown).
   - Upon completion: update `build_to_plan.md` to `COMPLETED ✅`, archive `PLAN.md` to `.agents/pipeline/plans/completed/`, and emit the "JOB'S DONE" signal.
2. **Subagent Delegation Protocol (The 20-Line / 1-File Rule)**:
   - *< 20 lines, 1 file*: Execute directly in-chat (Tier 2 Flash active).
   - *> 20 lines, multi-file, or running builds/tests*: Delegate to Flash subagent (`invoke_subagent(Model="flash")`) to protect main context.
   - *2-Strike Factory Escalation*: If Flash fails 2 compilation/test attempts, pause and escalate to Pro (`Model="pro"`).
3. **Audio-UI Thread Separation**:
   - Never call UI methods (`repaint()`, `setValue()`) from `processBlock()` or `processStereo()`.
   - Audio-to-UI communication strictly via lock-free atomics (`std::atomic`) or bounded FIFO queues.
