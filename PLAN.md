# 🧠 Master Implementation Plan: Cracked Chimes Fleet Preset & Branding Architecture
### *Unified PresetManager, DuckGate Sleep-Bypass Watchdog, Log-Domain Smart Dice, HeaderTitleBlock, In-Editor Overlays & Suite 82 SQA Gauntlet*

> **Status:** `PLAN_LOCKED_PENDING_OPERATOR_ENGAGE`  
> **Modality:** `AUTOPLAN_ACTIVE` (Universal Zero-Build Airgap Enforced)  
> **Target Fleet:** `Hastur` (v1.2.0), `Nyarlat` (v0.2.0), `Formless Spawn` (v1.1.0), `DigiStrike FS2` (v1.1.0)  
> **Excluded Scope:** `Shoggoth` (Tabled / Graveyarded per Loop Turn 12)  
> **Recent Upstream Sync:** Reconciled with Hastur v1.2.0 modernization, -110dBFS sleep detector, and Suite 81 completion.  
> **Peer Reviewers:** Gemini 4 Argon & Claude Sonnet/Opus (Dual-Adversarial Ratified)  

---

## 🏛️ Comprehensive Architectural Overview

This unified plan establishes an indestructible, commercial-grade preset and branding infrastructure across the Cracked Chimes fleet. It replaces inert dummy capsules (`"<  01: ACID PRIME  >"`) and duplicate inlined code with shared, high-performance UI components in `CrackedChimesUI.h`, an atomic `DuckGate` audio engine with lock-free idle & sleep bypass, log-domain Smart Dice mutation, and comprehensive SQA verification:

```text
┌─────────────────────────────────────────────────────────────────────────────────────────────────┐
│                     CRACKED CHIMES HARDENED PRESET & BRANDING ARCHITECTURE                      │
├──────────────────────────┬──────────────────────────┬─────────────────────┬─────────────────────┤
│         WAVE 1:          │         WAVE 2:          │       WAVE 3:       │       WAVE 4:       │
│     ENGINE & STORAGE     │      SHARED HEADER       │    MODAL OVERLAYS   │  FLEET INTEGRATION  │
├──────────────────────────┼──────────────────────────┼─────────────────────┼─────────────────────┤
│ • DuckGate Watchdog      │ • HeaderTitleBlock       │ • AboutOverlay      │ • Hastur 70-param   │
│   (Lock-Free Sleep-Bypass│   (14, 4, 140, 36)       │   (560x400 modal)   │ • Nyarlat migration │
│ • Log-Domain Smart Dice  │ • StepperAuditionBar     │ • PresetBrowser     │ • Formless Purple   │
│ • Section Locks Bitmask  │   (Vector Paths, Star,   │   (680x440 modal)   │ • DigiStrike Rust   │
│ • RCU In-Memory Index    │    Dirty Pip, Wheel Step)│ • Zero-Alloc ListBox│ • Factory Seed -18dB│
│ • %APPDATA% + BinaryData │ • Adaptive Width         │ • Scrim Shield      │ • 6-Gate Gauntlet   │
│ • Single NotifyHost Bump │   (220px / 288px)        │ • Esc-Revert Safety │ • Suite 82 Proof    │
└──────────────────────────┴──────────────────────────┴─────────────────────┴─────────────────────┘
```

---

## 📋 Four-Wave Implementation Gauntlet

### Wave 1: CMake & Headless Engine Core (`PresetManager.h / .cpp`)
*Target: Engine correctness, atomic concurrency, and memory isolation.*
- [ ] **1.1 CMake Target Sources, Version Bumps & Post-Build Packaging ([`CMakeLists.txt`](file:///C:/Dev/TheKlangSuite/CMakeLists.txt))**:
  - **Minor Version Bumps**:
    - `TheKlangSuite`: `0.6.0` $\implies$ `0.7.0`
    - `Hastur`: `1.2.0` $\implies$ `1.3.0` (v1.2 acid squelch landed + v1.3 Preset & About overhaul)
    - `Formless Spawn`: `1.2.0` $\implies$ `1.3.0` (keeping lockstep with Hastur)
    - `DigiStrike FS2`: `1.1.0` $\implies$ `1.2.0`
    - `Nyarlat`: `0.1.0` $\implies$ `0.2.0` (synchronizing CMake with contract)
    - `Shoggoth`: `0.1.0` (frozen / graveyarded)
  - Synchronize corresponding `version` tags in `contracts/*.yaml`.
  - Add `source/PresetManager.cpp` to `target_sources` for `Hastur`, `Hastur_Preview`, `FormlessSpawn`, `FormlessSpawn_Preview`, `DigiStrikeFS2`, and `DigiStrikeFS2_Preview`.
  - Add post-build copy directory commands for `assets/presets/hastur`, `assets/presets/formless_spawn`, and `assets/presets/digistrike_fs2` into their respective VST3 bundle resources.
- [ ] **1.2 Audio-Thread DuckGate with Sleep-Bypass Watchdog ([`source/PresetManager.h`](file:///C:/Dev/TheKlangSuite/source/PresetManager.h))**:
  - Implement `DuckGate` atomic structure with `processBlockCounter` and 20ms timeout watchdog.
  - **Zero-CPU Sleep Integration**: If `processor.isSleeping()` or `!processor.isPrepared()` or `processBlockCounter` is stale:
    $\implies$ Audio is already silent! Immediately bypass ducking, snap parameters on message thread with zero wait.
  - Active audio path: Cosine ramp to $0.0$ over $8\text{ms}$, signal `isSilent`, swap parameters, cosine ramp back to $1.0$.
- [ ] **1.3 Log-Domain Smart Dice & 16-Step Undo Ring ([`source/PresetManager.cpp`](file:///C:/Dev/TheKlangSuite/source/PresetManager.cpp))**:
  - Implement log-domain mutation math $t = (\ln x - \ln lo) / (\ln hi - \ln lo)$ with boundary reflection $R(v)$.
  - Maintain `diceHistory_` ring buffer (16 states) with `undoDice()` and `keepAsPreset()` methods.
  - Enforce `LoadSkip` (session master volumes/pan) and `DiceDeny` (structural modes/voice masks) parameter exclusion sets.
- [ ] **1.4 Section Locks Bitmask & Contract Bijection ([`source/PresetManager.h`](file:///C:/Dev/TheKlangSuite/source/PresetManager.h))**:
  - Implement bitmask: `LockOscCore`, `LockFilterVCF`, `LockEnvelopes`, `LockMasterFX`.
  - Contract rule: "Destination owns amount, Source owns shape." Sourced from `contracts/hastur.yaml` Tier 1–4 metadata.
- [ ] **1.5 Host Recall Hygiene & Storage Portability ([`source/PresetManager.cpp`](file:///C:/Dev/TheKlangSuite/source/PresetManager.cpp))**:
  - Embed `<PRESET_META>` in `getStateInformation` with `restoring_` guard in `setStateInformation`.
  - Migrate user directory to `%APPDATA%\CrackedChimes\<Plugin>\Presets\` on Windows (eliminating OneDrive cloud placeholder deadlocks).
  - Implement embedded factory presets fallback (`EmbeddedFactoryPresets.h`).
  - Batch parameter loading with single `processor.updateHostDisplay()` to prevent host undo flooding.
- [ ] **1.6 Verification Barrier 1**:
  - Run `python tools/verify_contract_bindings.py` (Asserts 100% parameter bijection).
  - Compile `dsp_tests.exe` and execute Suite 82 foundation checks.

---

### Wave 2: Shared Header Components (`CrackedChimesUI.h`)
*Target: Uniform visual brand identity and ergonomic auditioning.*
- [ ] **2.1 Shared HeaderTitleBlock ([`source/ui/cracked_chimes/CrackedChimesHeaderTitle.h`](file:///C:/Dev/TheKlangSuite/source/ui/cracked_chimes/CrackedChimesHeaderTitle.h))**:
  - Standardized placement at `(14, 4, 140, 36)`. Seamlessly replaces `headerLabel` and `subheaderLabel` in `HasturEditor.cpp`.
  - 19pt bold title in signature accent hue + 9pt "CRACKED CHIMES" slate subtitle.
  - Hover glow and cursor hand; click triggers `onOpenAbout` callback.
- [ ] **2.2 Canonical StepperAuditionBar Modernization ([`source/ui/cracked_chimes/CrackedChimesPresets.h`](file:///C:/Dev/TheKlangSuite/source/ui/cracked_chimes/CrackedChimesPresets.h))**:
  - Vector paths (`juce::Path`) for chevrons, favorite star, amber dirty pip (`●`).
  - Adaptive width: $220\text{px}$ compact mode for DigiStrike FS2, $288\text{px}$ standard mode for Hastur/Nyarlat/Formless (exact replacement for `presetStub` at `(162, 8, 288, 28)`).
  - Mouse wheel preset stepping with 40ms debounce.
  - Hover telemetry dispatch to `HeaderInspector`.
  - Click on preset label triggers `onOpenBrowser` overlay callback.
- [ ] **2.3 Verification Barrier 2**:
  - Compile preview targets (`Hastur_Preview.exe`, `Nyarlat_Preview.exe`).
  - Run `python tools/ui_gate.py` to verify $\Omega = 0\text{px}$ spatial clearance across all headers.

---

### Wave 3: In-Editor Overlays (`PresetBrowserOverlay.h` & `AboutOverlay.h`)
*Target: Modal inspection, distraction-free browsing, and attribution transparency.*
- [ ] **3.1 AboutOverlay Modal ([`source/ui/cracked_chimes/AboutOverlay.h`](file:///C:/Dev/TheKlangSuite/source/ui/cracked_chimes/AboutOverlay.h))**:
  - $560 \times 400\text{px}$ card over `#0D1117CC` scrim.
  - Hero header with version (dynamically bound to `JucePlugin_VersionString`: `v1.2.0` for Hastur, `v1.1.0` for Formless/DigiStrike, `v0.2.0` for Nyarlat).
  - Author credits, clickable attributions (JUCE, AKWF, ZDF, Alpha Juno, TB-303 Acid), DAW host telemetry, docs shortcut (`docs/CHEAT_SHEET.md`), and `[×]` / Esc close.
- [ ] **3.2 PresetBrowserOverlay Modal ([`source/ui/cracked_chimes/PresetBrowserOverlay.h`](file:///C:/Dev/TheKlangSuite/source/ui/cracked_chimes/PresetBrowserOverlay.h))**:
  - $680 \times 440\text{px}$ card over `#0D1117CC` scrim.
  - Left pane: Collections (`All`, `Favorites ★`, `User`, `Factory`) & Categories (`Bass`, `Lead`, `Pad`, `FX`).
  - Search box with Spacebar host transport passthrough.
  - Virtualized `juce::ListBox` with zero-alloc flyweight rendering (`paintListBoxItem()`).
  - Section Locks bar `[🔒 OSC] [🔒 FILTER] [🔒 ENV] [🔒 FX]` + `[🎲 DICE]` + `[↩ Undo Dice]`.
  - Esc-revert snapshot safety (`Esc` restores starting sound, `Enter` commits).
- [ ] **3.3 Verification Barrier 3**:
  - Execute `Gate 1.2` Scrim Shield & Focus Trapping verification.

---

### Wave 4: Fleet Integration & Suite 82 SQA Gauntlet
*Target: Full fleet adoption, factory sound seeding, and zero-defect proof token.*
- [ ] **4.1 Fleet Editor & Processor Wiring**:
  - **Hastur**: Add `PresetManager` to processor, wire `HeaderTitleBlock` (🟡 Carcosa Gold `#F2E10F`) and `StepperAuditionBar`.
  - **Formless Spawn**: Add `PresetManager` to processor, wire `HeaderTitleBlock` (🟣 Sorcerous Purple `#BC8CFF`) and `StepperAuditionBar`.
  - **DigiStrike FS2**: Add `PresetManager` to processor, wire `HeaderTitleBlock` (🟧 Industrial Rust `#E08C00`) and `StepperAuditionBar` (compact mode).
  - **Nyarlat**: Replace private inlined `titleLabel` and `StepperAuditionBar` with shared components (🔵 Tron Blue `#00C8FF`).
  - **The Writer (Gear Manual Hat) Telemetry & Lifeform Overhaul**:
    - Update all 58 inspector blurbs in `NyarlatControlInfo.h` and the default idle blurb in `NyarlatEditor.h` per `18_gear_manual_inspector_strings.md` (excising all legacy "Z-plane Z-cube" stubs in favor of punchy $\le 69$-char additive wavetable & chaos matrix descriptors).
    - Deploy unified Fleet Lifeform Triad Canon descriptions ($\le 71$ chars, spatial-mechanical gear manual stylometry) across all 4 plugins (`HasturControlInfo.h`, `NyarlatControlInfo.h`, `FormlessControlInfo.h`, `DigiStrikeFS2ControlInfo.h`).
    - Wire `lifeformRocker_` in `FormlessEditor.cpp` and `lifeModeToggle_` in `NyarlatEditor.cpp` to live inspector telemetry matching `Hastur` and `DigiStrike FS2`.
- [ ] **4.2 Factory Preset Seeding & Level Matching**:
  - Expand `assets/presets/<plugin>/` from 8-parameter stubs to full 70-parameter contracts.
  - Level-match presets offline to standardized $-18\text{ LUFS-S}$ benchmark phrase.
- [ ] **4.3 Six-Gate Master SQA Gauntlet (Suite 82 in `dsp_tests.cpp`)**:
  - **Gate 1**: Spatial Collision Radar (`tools/ui_gate.py` $\implies \Omega = 0\text{px}$, padding $\ge 4\text{px}$).
  - **Gate 2**: Real-Time Audio Safety (`tools/sqa/lint_dsp.py` & `audiothread-guard` $\implies$ 0 malloc, 0 locks).
  - **Gate 3**: Host DAW Stability (`pluginval` Level 5 $\implies$ 0 leaks, 0 crashes).
  - **Gate 4**: Contract Bijection (`tools/verify_contract_bindings.py` $\implies$ 100% parameter coverage).
  - **Gate 5**: Information-Theoretic Entropy (`tools/sqa/audit_presets.py` $\implies \sigma^2 > 0.005, H > 0.5\text{b}, \text{NCD} \ge 0.35$).
  - **Gate 6 (Suite 82)**: Acoustic Oracles & 5 Metamorphic Relations (`dsp_tests.exe` Suites 82.1–82.15, MR-1 through MR-5).
- [ ] **4.4 Visual Pre-Flight & Cryptographic Token Seal**:
  - Capture rendered desktop window: `python tools/capture_ui.py "Hastur"`.
  - Finalize task and seal proof token: `python tools/plan_manager.py finish --target cracked_chimes_preset_system`.
