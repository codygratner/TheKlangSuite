# 🛡️ Flight 2: Open Source Anti-Slop Hardening & CI Safety Gauntlet (Card #29 / Section 15)

> **ID:** plan-006  
> **TARGET:** FACTORY_FLOOR (Klang Industries & New Klang City) 🚀  
> **Topic Target:** Card #29 / Section 15 (`Anti-Slop in Open Source`)  
> **Status:** COMPLETED ✅  
> **Priority:** P0 (Top Priority)  
> **Target Branch:** `0.4.0-dev`  
> **Recommended Setting:** Gemini 3.8 Flash (Thinking: High) — 🟢 SUSTAINABLE  
> **Source Note:** [`C:\Dev\TheKlangVault\Inbox\Archive\Anti-Slop in Open Source.md`](file:///C:/Dev/TheKlangVault/Inbox/Archive/Anti-Slop%20in%20Open%20Source.md)  
> **Benchmark Source:** `https://www.opensourceprojects.dev/post/anti-slop` (`miqdadbadjuber/anti-slop`)  

---

## 🎯 Architectural Intent & Storyline

In our initial research audit of `https://www.opensourceprojects.dev/post/anti-slop` (Card #29 in the N'kai Morning Mail Inbox), our ecosystem was evaluated across 10 critical dimensions:
1. **Internally, we decisively EXCEED the SOTA anti-slop standard**: Our AST-level audio thread safety linter (`audiothread-guard`), automated C++ schema separation assertions (`ParameterSchemaAuditTest.h`), 100% dynamic UI reflection suites (`ReflectionGuardrailSuite.h`), and Hegelian adversarial subagents (`contrarian`) go far beyond typical markdown rulebooks.
2. **Externally, towards the public open-source contributor surface, we FAIL**: Across `TheKlangSuite`, `ToadTracker`, `nkai`, and `TheKlangResearch`, there is **zero `CONTRIBUTING.md`**, zero `.github/PULL_REQUEST_TEMPLATE.md`, and zero CI static analysis in `.github/workflows/ci.yml` checking audio safety or schema leakage on external PRs.
3. **Audio-Thread Invariant Violation**: In `Source/ModularBlocks.h`, three runtime container resizes (`targetSig.resize`, `ctx.ampEnvSignal.resize`, `envSig.resize`) violate our strict zero-allocation audio-thread invariant.

**Flight 2** eliminates every identified blind spot, elevates our public open-source posture to true SOTA, hardens our GitHub CI pipeline, and concludes by re-running the full Anti-Slop Audit to achieve a clean **10/10 EXCEED** scorecard.

---

## 📋 Phase Breakdown

### Phase 1: Real-Time Audio-Thread Memory Pre-Allocation (`Source/ModularBlocks.h`)
- [x] Inspect and eliminate runtime container resizes in `Source/ModularBlocks.h`:
  - Line 505: `modSig.resize(numSamples);`
  - Line 608: `peSig.resize(numSamples);`
  - Line 1716: `targetSig.resize(numSamples);`
  - Line 2219: `ctx.ampEnvSignal.resize(numSamples);`
  - Line 2401: `envSig.resize(numSamples);`
- [x] Ensure `VoiceContext` pre-allocates `filterEnv1Signal`, `filterEnv2Signal`, `filterEnv3Signal`, `ampEnvSignal`, `modEnv1Signal`, `modEnv2Signal`, and `modEnv3Signal` to `maxBlockSize` inside `prepareToPlay()` / `prepareVoiceBuffers()`.
- [x] Clamp buffer access to pre-allocated capacity without reallocation in `processStereo()`.
- [x] Run `python tools/audit_audiothread.py --path Source/ModularBlocks.h --strict` and assert zero allocation violations.

### Phase 2: Author Universal Open-Source Governance Artifacts
- [x] Author `CONTRIBUTING.md` across active repositories (`TheKlangSuite`, `ToadTracker`, `nkai`):
  - **The Anti-Slop Manifesto**: Boy Scout Rule, zero decorative comment banners, no tautological comments, ruthless deletion.
  - **The Audio Thread Invariants**: Zero allocations in `processBlock()`, zero mutexes/locks, zero blocking I/O, SIMD `FastMath` preference.
  - **4-Layer Schema Separation Contract**: `controls/` (DSP contracts only), `layouts/` (structure only), `themes/` (visuals only), `text/` (strings only).
  - **FOSS & Algorithmic Attribution**: Mandatory SPDX GPL-3.0 headers and inline provenance docblocks for adapted algorithms.
  - **Conventional Commits**: Strict format `<type>(<scope>): <desc>`.
- [x] Author `.github/PULL_REQUEST_TEMPLATE.md` across `TheKlangSuite`, `ToadTracker`, and `nkai`:
  - Mandatory contributor checklist (Audio thread verification, schema leak test, local test pass, no AI slop).

### Phase 3: CI/CD Automation & Matrix Hardening (`.github/workflows/ci.yml`)
- [x] Update `.github/workflows/ci.yml` in `TheKlangSuite`:
  - Add `gui_tests` to Windows build and test steps (enforcing `ParameterSchemaAuditTest` and `ReflectionGuardrailSuite`).
  - Add `gui_tests` execution to Linux CI step under `xvfb-run -a`.
  - Add automated audio-thread safety check:
    ```yaml
    - name: Run Audio-Thread Safety Linter
      run: python tools/audit_audiothread.py --strict
    ```
  - Remove `|| true` failure masking from Linux and macOS `pluginval` runs.

### Phase 4: Local Regression & Validation Gauntlet
- [x] Execute `python tools/audit_audiothread.py --strict` across entire `Source/` tree.
- [x] Compile and run `dsp_tests.exe` via CMake.
- [x] Compile and run `gui_tests.exe` via CMake (asserting 100% pass on schema isolation and reflection).
- [x] Run `python tools/verify_context_clues.py` (confirm 100% link resolution).

### Phase 5: Re-Run the Anti-Slop Audit & Scorecard Delivery
- [x] Re-run the comprehensive audit against `https://www.opensourceprojects.dev/post/anti-slop`.
- [x] Generate the updated 10-point scorecard verifying that all 10 dimensions now register as **10/10 EXCEED / SOTA**.
- [x] Update `docs/BACKLOG.md`: Mark Section 15 (Open Source Anti-Slop Hardening & CI Safety Gauntlet) as `[x] COMPLETED`.
- [x] Mark Card #29 in `.agents/pipeline/mailbag/mail_queue.json` as `COMPLETED`.
- [x] Commit all changes cleanly across affected repositories.
