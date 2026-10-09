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
  - **Strict Tone & Style Invariant (ZERO INTERNAL LORE)**:
    - **100% Zero-Lore Mandate**: Strictly ZERO references to project mythology, Mayor Toad, fictional factions, inside jokes, or creative worldbuilding.
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

---

## 🛡️ Acceptance Criteria
1. `vibe_coding_field_guide.md` contains zero JUCE or C++ audio thread code blocks (100% stack-agnostic).
2. `audio_dsp_field_guide.md` contains comprehensive, deep C++ audio engineering invariants.
3. 100% of touched markdown links resolve cleanly.
4. Git commits pushed to `TheKlangResearch` and committed on `TheKlangSuite`.
