# Context Clues: New Klang City (Planning Headquarters)

> **Step 0 Bootstrapping**: Planners waking up from a context refresh or starting an architectural session MUST read this document on Turn 1 to orient on active milestones, return agenda, architectural memory, canonical taxonomy, and workflow gates without prompt bloat.

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
- **Active Working Milestone:** `v0.4.0 "The Interface & Experience Update"` (🔨 Execution in Progress)
- **Next Target Milestone:** `v0.4.1 "The Complete Control Surface & UI Shells"` (Staged Draft Plan in `.agents/pipeline/plans/drafts/`)
- **Follow-up Milestone:** `v0.4.5 "The Klang Planter Refresh"` (Rebuilding Planter on TKF's Neo-Slate & v0.4.1 Shells)
- **Upcoming Sonic Milestone:** `v0.5.0 "The Sound & Chaos Update"` (Sonic Expansion & DSP Hardening)
- **Completed & Locked Milestones (NEVER GO BACK):** 
  - `v0.3.0` Architecture & Parity Audit — ✅ COMPLETED
  - `v0.3.1` Editor Quality & Data Schema — ✅ COMPLETED
  - `v0.3.2` Agent Infrastructure & Logging — ✅ COMPLETED
  - `v0.3.3` The Hardening Gauntlet — ✅ COMPLETED

---

## 🎯 Immediate Return Agenda & Status
- **Current Objective:** Factory Execution & Roadmap Flight Orchestration
- **Active Agenda:**
  1. **Flight 1: Canonical State & Factory Handoff (Topics 6.16 & 6.7)**: ✅ COMPLETED by Klang Industries.
  2. **Flight 9: Distribution Packaging & Key Ergonomics (Topics 6.12 & 6.6)**: ⏳ READY_FOR_EXECUTION on factory floor. Universal bundle assembly, installer script (`install_nkai_bundle.ps1`), and dynamic keyboard shortcuts.
  3. **The 9-Flight N'kai Implementation Track**: Staging blueprints sequentially for sustainable Flash High factory delivery.
  4. **Post-Milestone Deep Docs Gauntlet**: Staged in `.agents/pipeline/plans/drafts/post_milestone_deep_docs_gauntlet.md` for pre-release execution.
- **Active Blueprint:** [`PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md) (Flight 9 Ready for Execution)

---

## 🧠 Architectural Memory & "Unforgettables"
1. **Agent Model Protocol**:
   - Main Chat / Orchestrator runs strictly on **Gemini 3.8 Flash (Thinking: High)** for sustainable, near-zero quota burn.
   - When deep DSP math, circuit analysis, or complex architecture is required, spawn an on-demand **Pro Subagent** (`invoke_subagent(Model="pro")`).
2. **DSP & Audio Thread Invariants**:
   - Zero runtime allocations (`new`/`malloc`) or locks (`std::mutex`) in `processBlock`.
   - Prefer `TbdAudio::FastMath` over standard CRT transcendentals (`std::pow`, `std::sin`, `std::tanh`).
   - Hardware Target: dadamachines TBD-16 (ESP32-P4 @ 400MHz, PSRAM memory budget).
3. **Synthesis Engine Realities**:
   - **Envelopes**: Continuous `_slope` controls ALREADY exist on all envelopes (`ampenv_slope`, `filterenv_slope`, etc.). Do not propose duplicate parameters; optimizations focus on replacing CRT `std::pow` with `FastMath`.
   - **Approved v0.5.0 Modules**: Casio CZ Phase Distortion, Alpha Juno "Hoover" Saw-PWM, Alpha Juno IR3R05 Dual-Stage Resonant VCF (Zero Bass Drop), Buchla Wavefolder + Pre-APF Phase Dispersion, Elektron Octatrack Tuned Comb Filter / Karplus-Strong Resonator.
   - **Approved v0.6.0 Workflow**: MTS-ESP / Scala Microtonal Support (Desktop Only), Offline Pitch-Tracked Oversampling, Fullscreen Splice-Style Browser with Embedded Macros.
4. **Data-Driven Hierarchy**:
   - All parameters live in `assets/controls/*.json`.
   - Structural UI lives in `assets/layouts/*.json`.
   - Themes live in `assets/themes/*.json`.
   - Strings live in `assets/text/*.json`.

---

## 📖 Canonical Taxonomy, Product Lineup & Essential Spec Links
- **Master Documentation Catalog**: [`docs/DOCS_CATALOG.json`](file:///c:/Dev/TheKlangSuite/docs/DOCS_CATALOG.json) (100% indexed documentation ledger).
- **System Architecture Map**: [`docs/SYSTEM_MAP.md`](file:///c:/Dev/TheKlangSuite/docs/SYSTEM_MAP.md) (central ecosystem index).
- **Official Architectural Glossary & Taxonomy**: [`docs/GLOSSARY.md`](file:///c:/Dev/TheKlangSuite/docs/GLOSSARY.md)
  - UI vs DSP Terminology (*Chassis*, *Pages*, *Cards*, *Modules*, *Sliders* vs *Knobs*).
  - Acronym Collision Registry: `TKS` (Suite), `TKF` (Farmer), `TKP` (Planter), `TKM` (Mill), `TKE` (Editor), `TKB` (Box / Hardware), `TKR` (R1), `TKC` (Cultivator).
  - Generative Engine: `The Klang-Brain` (internal code `TKG`).
- **Product Lineup & Core Intent Matrix**: [`docs/architecture/product_lineup.md`](file:///c:/Dev/TheKlangSuite/docs/architecture/product_lineup.md)
  - "Stay in Your Lane" drift guardrail (TKF = deep sound design, TKP = lean & mean BIA machine, TKM = pure multi-FX rack, TKR = 7-voice zero-tab rhythm console, TKB = bare-metal zero-allocation hardware).
- **Universal Project Backlog**: [`docs/BACKLOG.md`](file:///c:/Dev/TheKlangSuite/docs/BACKLOG.md)
- **Core Design Specs**:
  - [N'kai Canonical State & Persistence Architecture](file:///c:/Dev/TheKlangSuite/docs/specs/nkai_canonical_state_architecture.md)
  - [The Klang R1 Modular Drum Synthesizer](file:///c:/Dev/TheKlangSuite/docs/specs/spinoffs/the_klang_r1.md)
  - [The Klang Mill Pedalboard Multi-FX](file:///c:/Dev/TheKlangSuite/docs/specs/spinoffs/the_klang_mill.md)
  - [TBD-16 Klang Seed Hardware Integration](file:///c:/Dev/TheKlangSuite/docs/specs/tbd16_klang_seed_effects.md)

---

## ⚡ Audio Thread / Workflow Invariants & Handshake Gates
1. **The Staged Planning Buffer (`.agents/pipeline/plans/drafts/`)**:
   - New Klang City MUST author all new plans and draft blueprints into `.agents/pipeline/plans/drafts/<task_slug>.md`.
   - Never overwrite active `PLAN.md` while factory is `IN_PROGRESS`.
2. **The Factory-Idle Promotion Gate**:
   - Only after Klang Industries sounds the "JOB'S DONE" signal and archives the previous plan may New Klang City promote an approved draft to `PLAN.md` and dispatch via `plan_to_build.md`.
3. **The Pro Sanity Gate (Flash Drafts, Pro Audits, Flash Builds)**:
   - For major features or milestone kickoffs, draft on Flash High, perform a 1-turn Pro High audit, then dispatch for sustainable Flash High factory execution.
4. **Real-Time Audio Safety**:
   - Zero dynamic memory allocations, zero locks, and zero blocking I/O in the audio rendering path.
