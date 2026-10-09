# Draft Blueprint: Post-Milestone Deep Docs & Field Guides Gauntlet
> **Status:** QUEUED (Staged for execution during the post-milestone `/update-docs --deep` pass)  
> **Target Execution:** Milestone Capstone / Pre-Release Gauntlet  
> **Estimated Model Tier:** Tier 2 Flash High (with optional 1-turn Pro harvest for DSP math)  
> **Source Agreements:** 2026-10-09 System Sanity Audit & Field Guide Separation RFC  

---

## 🎯 Purpose & Scope

This blueprint serves as the permanent, hard-coded itinerary for the next comprehensive **Five-Pillar Deep Documentation Gauntlet** (`/update-docs --deep`). It ensures that in the event of context clearing, transcript truncation, or multi-week hiatuses, zero institutional decisions are lost.

---

## 📋 Itinerary Tasks for the Next Deep Docs Pass

### Task 1: Field Guide Separation of Concerns (Universal vs. Domain)
- [ ] **`TheKlangResearch/guides/vibe_coding_field_guide.md` (Universal Vibe-Coding Manifesto)**:
  - Strip out audio-specific C++ parameter blocks, JUCE audio thread invariants, and sample-rate math.
  - Generalize into a 100% stack-agnostic manual for autonomous AI pair-programming (applicable to Python, Rust, TypeScript, C++, web, embedded).
  - Emphasize: Dual-Chat division of labor (Ivory Tower vs Factory Floor), Model Tier economics, Asymmetric Sidecar Split, Tri-Engine subagent swarms (`explore`, `contrarian`, `research`), Strategy Graveyards, Negative Architecture, and bounded `/goal` sprints vs runaway loops.
- [ ] **`TheKlangResearch/guides/audio_dsp_field_guide.md` (Real-Time C++ & Audio DSP Bible)**:
  - Elevate from the current 33-line stub into the comprehensive, definitive manual for modern AI-assisted audio development.
  - Ingest all audio invariants from `GEMINI.md`: Zero allocations in `processBlock`, zero locks, zero blocking I/O, lock-free SPSC FIFOs, atomics for audio-to-UI telemetry.
  - Document SIMD `FastMath` Padé approximations, rational polynomial waveshapers, and transcendental cycle budgets.
  - Ingest filter topologies (SVF, ZDF, Ladder, Comb/Karplus-Strong) and anti-aliasing protocols (PolyBLEP, Half-Band FIR/IIR upsampling, MIP-mapped wavetables).
  - Codify modern C++20 standards for audio (RAII, zero raw ownership, JUCE 9.0.3 timer hygiene).
  - Document headless testing harness patterns (`dsp_tests`, `gui_tests`, dynamic reflection, `pluginval`).
- [ ] **Local Pointers Synchronization**:
  - Update `c:\Dev\TheKlangSuite\docs\history\vibe_coding_field_guide.md` and `audio_dsp_field_guide.md` to reflect the updated domain split.

### Task 2: Subagent Soul Harvest & Autopsy Ingestion
- [ ] Sweep `.agents/pipeline/staging/souls/` for any new staged soul harvests.
- [ ] Verify `TheKlangResearch/qa_autopsies/` contains all verified post-mortems.
- [ ] Reset `.agents/pipeline/communique/subagent_receipt.md` to `STATUS: IDLE`.

### Task 3: Five-Pillar Hyperlink Integrity Gauntlet
- [ ] Run programmatic verification across all markdown documents in `TheKlangSuite`, `ToadTracker`, `nkai`, `TheKlangResearch`, and `TheKlangVault`.
- [ ] Ensure 100% of local markdown file links resolve to real files on disk.

### Task 4: Central Obsidian Vault Recompilation
- [ ] Run `powershell -ExecutionPolicy Bypass -File .\tools\sync_obsidian_vault.ps1`.
- [ ] Verify `TheKlangVault/Research/Master_Research_Index.md` compiles cleanly with 0 errors.

### Task 5: 🆕 The "Anti-Slop" Vibe-Coding Executive Summary (2-to-3 Minute Manifesto)
- [ ] **Author `TheKlangResearch/guides/anti_slop_vibe_coding_manifesto.md`**:
  - **Reading Time Target**: 2 to 3 minutes (~600 words, high-density, punchy, practical).
  - **Target Audience**: Professional software engineers and SQA practitioners who are skeptical of AI code generation and frustrated by "AI slop", hallucinations, broken whitespace, and silent regressions.
  - **Strict Tone & Style Invariant (ZERO INTERNAL LORE & ZERO PRODUCT NAMES)**:
    - **100% Zero-Lore & Zero-Product Mandate**: Strictly ZERO references to project mythology, Mayor Toad, fictional factions, inside jokes, or creative worldbuilding. Furthermore, strictly ZERO mentions of specific proprietary product names (no *The Klang Farmer*, *The Klang Planter*, *The Klang Suite*, *ToadTracker*, *N'kai*, or author names).
    - **"Process, Not Products"**: The document must read as an objective, stack-agnostic software engineering methodology whitepaper. It is 100% about the *process and controls*, not about the author or specific applications.
    - **Professional Industry Standards**: Written strictly in industry-standard software engineering and SQA terminology (sandboxing, bounded context, static analysis, invariant assertions, deterministic verification gauntlets, AST codemods).
    - **No-BS Senior Engineering Voice**: Crisp, pragmatic, and respectful of experienced developers' time.
  - **Core Controls & Mechanisms to Highlight**:
    1. *The Context Shield & Flash/Pro Subagent Slicing*: Keeping main reasoning conversations lean while delegating messy mechanical diffs to ephemeral subagents.
    2. *Strict Domain Division & Facility Gates*: Tooling and dev scripts never clobber production runtime code.
    3. *Granular Blueprinting (Pro Plans, Flash Builds)*: High-reasoning plans provide exact method signatures, test assertion lines, and contracts, eliminating "creative hallucinations" during build.
    4. *Strict Architectural Invariants & Automated Auditing*: Real-time safety guards (zero allocations, zero locks, zero blocking I/O) enforced by static analysis tools (`audiothread-guard`) before code lands.
    5. *The Boy Scout Rule & Ruthless Deletion*: Zero tolerance for dead code, commented-out blocks, or messy indentation. Modern C++20 / clean architecture only.
    6. *PCDA & The Adversarial Contrarian*: Deploying dedicated Devil's Advocate subagents to attack proposals and stress-test failure modes before implementation.
    7. *Deterministic Test Gauntlets*: Code is never approved on "vibes"—it must pass automated headless test runners (`dsp_tests`, `gui_tests`, `audit_sidecar.py`, `pluginval`).
  - [ ] Add executive summary link and excerpt to `TheKlangResearch/README.md` and `TheKlangVault/Research/Master_Research_Index.md`.

### Task 6: 🆕 The Evergreen Workflow Cheat Sheet (`docs/WORKFLOW_CHEAT_SHEET.md`)
- [ ] **Author `c:\Dev\TheKlangSuite\docs\WORKFLOW_CHEAT_SHEET.md`**:
  - **Purpose**: A concise, 1-page, high-density reference sheet for human developers to instantly recall how we operate without reading through dense rules or forgetting workflow mechanics.
  - **Key Sections**:
    1. *The 3 Facility Spheres*: Tooling Harness (New Klang City / `TARGET: TOOLING_HARNESS`) vs. Native Audio Factory (Klang Industries / `TARGET: FACTORY_FLOOR`) vs. Central Research Hub (`TheKlangResearch`).
    2. *Model Tiers & Thinking Budgets*: Main chat stays on Tier 2 Flash High; Pro High reserved for on-demand subagents (`research`); Flash High is King at the Factory Floor (2-strike safety net).
    3. *The Tri-Engine Subagent Swarm*: `research` (Tier 1 Pro), `explore` (Tier 2 Flash), `contrarian` (Tier 2 Flash Devil's Advocate), and Flash Tooling Builder.
    4. *The Communiqué Dispatch Flow*: Dual-track pipelines (`subagent_dispatch.md` for tooling vs. `plan_to_build.md` for native audio C++).
    5. *The 5-Phase Execution Standard*: `Phase 1/5` Spec $\to$ `2/5` Core $\to$ `3/5` Tooling $\to$ `4/5` Templates $\to$ `5/5` Verification & Commits.
    6. *N'kai Quick CLI Reference*: `python tools/nkai_serve.py`, `python tools/check_mail.py`, `python tools/pcda_format.py`, and `tools/nkai_ledger.py`.
    7. *Strict Backlog & Milestone Governance*: Record only in `docs/BACKLOG.md` (no rogue files, no premature implementation); strict thematic milestone scoping.
    8. *Git Branch & Commit Hygiene*: Never commit directly to `main`/`master`; conventional commits always.
    9. *Subagent Naming & Role Reference*: Embed the `[ 🔬 Subagents: research (Pro) / explore (Flash) / contrarian (Advocate) ]` guide permanently into the cheat sheet, and retire that badge line from `GEMINI.md` routine chat footers once deployed to keep chat responses lean and uncluttered.
  - **Maintenance Invariant**: Establish rule that whenever operational workflows change, `docs/WORKFLOW_CHEAT_SHEET.md` is updated in the same pass.
  - [ ] Register in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.

### Task 7: 🆕 Tri-State Model Advisory Architecture (Topic 4 & GEMINI.md Streamline)
- [ ] **Update `GEMINI.md` ("Strict Model Advisory Protocol & Anti-Spam Gate")**:
  - Replace the legacy multi-line kickoff warning banner with the **Tri-State Model Advisory Architecture**:
    - **State 0 (95% Happy Path)**: Permanent Flash 3.8 High orchestrator daily driver. Routine turns, phase advances, and plan kickoffs run without multi-line advisory blocks or dropdown-check pauses, displaying only an ambient single-line footer badge (`[ ⚡ Engine: Flash High | Pro Subagents on Demand ]`).
    - **State 1 (Internal Guardrail - No Direct DSP)**: The Flash orchestrator is strictly forbidden from writing differential equations, non-linear saturation curves, SIMD DSP loops, or lock-free atomics directly in-session; it MUST delegate Tier 1 math tasks to `invoke_subagent(Model="pro")`.
    - **State 2 (Emergency Tripwire)**: A loud blocking advisory banner and hard-pause in chat text is strictly reserved for true emergency escalations: 2-Strike Factory Build Failures, Milestone Capstone Sanity Gates, and Release Gauntlets (`/cut-release`).
  - Delete legacy 5-minute downgrade timer rules.
  - Synchronize updated guidelines across documentation catalogs and cheat sheets.

---

## 🛡️ Acceptance Criteria
1. `vibe_coding_field_guide.md` contains zero JUCE or C++ audio thread code blocks (100% stack-agnostic).
2. `audio_dsp_field_guide.md` contains comprehensive, deep C++ audio engineering invariants.
3. `GEMINI.md` implements the Tri-State Model Advisory Architecture, eliminating banner fatigue while preserving critical audio invariants and the 2-strike escalation tripwire.
4. 100% of touched markdown links resolve cleanly.
5. Git commits pushed to `TheKlangResearch` and committed on `TheKlangSuite`.
