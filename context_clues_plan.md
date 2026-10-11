# Context Clues: New Klang City (Planning Headquarters)

> **Step 0 Bootstrapping**: Planners waking up from a context refresh or starting an architectural session MUST read this document on Turn 1 to orient on active milestones, return agenda, architectural memory, canonical taxonomy, and workflow gates without prompt bloat.

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
- **Current Objective:** Mailbag triage on the **Pro-rejiggered board** (70 notes → 37 cards; 25 PENDING ranked cards + 12 status cards). Resume at **card #1 `rj-01` Confirm-to-Close** (14 notes believed shipped; user confirms closures — never auto-close).
- **Board & Data**:
  - Ordering source of truth: [`.agents/pipeline/mailbag/rejigger.json`](file:///c:/Dev/TheKlangSuite/.agents/pipeline/mailbag/rejigger.json) (ranked cards with `sources[]` → raw ledger items).
  - Ledger: [`.agents/pipeline/mailbag/mail_queue.json`](file:///c:/Dev/TheKlangSuite/.agents/pipeline/mailbag/mail_queue.json); regen with `python tools/check_mail.py --archive --no-browser` (unreferenced new mail appears after cards as `🆕`).
  - Rollback: `.agents/pipeline/mailbag/snapshots/mail_queue_pre_rejigger_2026-10-11.json` (pre-rejigger) and `snapshots/mail_queue_prev.json` (auto, every regen).
  - Board UX invariants: default filter = Untriaged; "All" first on both rails; session-stable pillar order. Triage choices in chat text (no `ask_question`).
- **Last Commits**: TKS `8a4e5b8` (rejigger overlay + `--archive-inbox` wipe fix), nkai `1914108`. Staged soul: `.agents/pipeline/staging/souls/2026-10-11_pro_mailbag_rejigger.md`.
- **Model State**: Opus 5.5 High session concluded → main chat back on **Gemini 3.8 Flash (High)**. Opus weekly quota ~83% used (2026-10-11) — reserve for DSP / 2-strike only.
- **Immediate Next Command**: Open the N'kai board and present card #1 (`rj-01`) for confirm-to-close triage.
- **Active Blueprint:** [`PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md) (Branch: `0.4.0-dev`)


---

## 🧠 Architectural Memory & "Unforgettables"
1. **Agent Model Protocol**:
   - Main Chat / Orchestrator runs strictly on **Gemini 3.8 Flash (Thinking: High)** for sustainable, near-zero quota burn.
   - **Targeted Opus DSP Swap**: For heavy C++ DSP algorithms or 2-strike factory escalations, the agent emits a Tier 1 Model Advisory prompting the user to manually swap to `Claude 5.5 Opus (Thinking: High)` in the IDE footer. Upon completion, a Mandatory Downgrade Chime prompts switching back to Flash High.
   - **Argon GA Reconvene Trigger**: When Gemini 4 Argon reaches public General Availability in the IDE dropdown, immediately reconvene to benchmark Argon vs. Opus 5.5 High on C++ DSP.
   - For routine background deep dives, spawn an on-demand subagent (`invoke_subagent`).
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
