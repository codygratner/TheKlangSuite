# Backlog Archive: Completed Milestones & Engineering History

### [COMPLETED] Right-Click Parameter Edit Modal: Quick-Snap Preset Buttons
- **Waveshape Edit Modal**:
  - Add quick preset buttons directly into the right-click edit modal: **Sine** (0.0), **Triangle** (0.25), **Saw** (0.5), and **Square** (0.75 / 1.0).
- **Contextual Quick-Snap Buttons Across the Synth**:
  - Added via JSON `snap_points` system.


This document serves as the permanent historical record of completed engineering tasks, feature implementations, bug fixes, and architectural decisions for **The Klang Farmer** and **The Klang Planter**.

---

## 🏆 Release v0.1.8 (2026-10-04)

### 1. FastMath.h SIMD Acceleration Core (Desktop & Embedded)
- **Linear Issue:** [THE-8](https://linear.app/the-klang-farmer/issue/THE-8/fastmathh-and-simd-accelerated-dsp-core-desktop-and-embedded)
- **Summary & Technical Design**:
  - Author pure C++ `source/FastMath.h` (`TbdAudio::FastMath`):
    - `fastPow2(float x)`: IEEE 754 exponent bit-manipulation with 3rd-order minimax polynomial correction (< 0.015% error, **3.38x faster** than CRT `std::pow(2.0f, x)`).
    - `fastExp(float x)`: Fast percussive decay curve generator (< 0.015% error).
    - `fastSinNorm(float phase)` & `fastCosNorm(float phase)`: Normalized phase branchless polynomial approximations (< 0.016% error, **2.08x faster** than `std::sin`).
    - `fastTanh(float x)`: Padé rational saturation approximation $\frac{x \cdot (27 + x^2)}{27 + 9x^2}$ (**9.34x faster** than `std::tanh`).
    - `fastTanhPrecise(float x)`: Padé [7/6] saturation approximation (< 0.002% peak error).
    - `fastDbToGain(float db)`: Direct 2-cycle dB-to-linear conversion via `fastPow2(db * 0.1660964f)`.
    - `PhaseAccumulator32`: Branchless 32-bit integer accumulator with natural overflow wrapping.
  - Integrated into all inner DSP loops across `source/ModularBlocks.h` and `source/PlanterEngine.h` (Carrier, Modulator, Drive, Filter, Saturation, Limiters, Chorus, Phaser, Flanger, Comb Filter, Frequency Shifter).
  - Built `test/benchmark_dsp.cpp` (`benchmark_dsp.exe`) to measure cycle counts, verify accuracy, and compare with CRT standard math.
- **Verification**:
  - Zero audio-thread violations detected via `audiothread-guard`.
  - All 34 unit tests in `dsp_tests.exe` passed cleanly.
  - Full VST3 plugin builds validated.

---

### 2. Mac & Linux Compatibility Hardening, Dynamic Host Loading CI (`pluginval` & `auval`), & Docker Linux Container
- **Linear Issue:** [THE-5](https://linear.app/the-klang-farmer/issue/THE-5/mac-and-linux-compatibility-dynamic-host-loading-ci-and-dockerization)
- **Summary & Technical Design**:
  - `CMakeLists.txt` Cross-Platform Declarations:
    - Declared `CMAKE_OSX_DEPLOYMENT_TARGET="11.0"` and `CMAKE_OSX_ARCHITECTURES="arm64;x86_64"` **before** `project(TheKlangFarmer ...)` so Apple Clang initializes the macOS 11.0 SDK target triple prior to compiler detection.
    - Added Linux static runtime linking `-static-libstdc++ -static-libgcc` to prevent `GLIBCXX` mismatches across distributions.
    - Set `VST3_AUTO_MANIFEST FALSE` on `TheKlangFarmer` and `TheKlangPlanter`.
    - Declared `JUCE_VST3_CAN_REPLACE_VST2=0`, `JUCE_WEB_BROWSER=0`, and `JUCE_USE_CURL=0` globally on plugin targets.
  - macOS Gatekeeper Quarantine & Packaging:
    - Added ad-hoc codesigning (`codesign --force --deep -s -`) across all macOS bundles.
    - Shipped `scripts/mac/Unlock_and_Install_Mac.command` (with `xattr -cr`, component copy, and CoreAudio reload) and `scripts/mac/README_MAC_INSTALL.txt` inside release archives.
  - Automated Multi-Platform CI Workflow (`.github/workflows/ci.yml`):
    - Runs CMake build, `dsp_tests`, and `pluginval` v1.0.4 host validation across macOS, Linux (under `Xvfb`), and Windows on every push and pull request.
    - Added native macOS `auval` verification (`auval -v aumu Tkf1 Rlyh` and `Tkp1`).
  - Docker Multi-Platform Container:
    - Created `docker/Dockerfile.linux` and `docker/docker-compose.yml` with pre-installed `ninja`, `xvfb`, and `pluginval` v1.0.4.
    - Provided `scripts/test_docker_linux.bat` for single-command verification on Windows.
- **Verification**:
  - Full compatibility audit passed (`compat-check`).
  - CMake configuration and build verified on Windows.

---

## 🏆 Release v0.1.7 (2026-10-04)

### 1. The Klang Planter UI Color Routing & "Vel Min Level" Label Polish
- **Linear Issue:** [THE-7](https://linear.app/the-klang-farmer/issue/THE-7/the-klang-planter-ui-color-routing-and-vel-min-level-label-polish)  
- **Commits:** `fa088df`, `b441b3e`  
- **Plan Archive:** [`2026-10-04_THE-7_planter-ui-color-routing-and-label-polish.md`](../../.agents/pipeline/plans/completed/2026-10-04_THE-7_planter-ui-color-routing-and-label-polish.md)
- **Summary & Technical Design**:
  - **Carrier Card — Mod Depth Slider (Cyan)**:
    - Updated `carrierDepthSlider` accent to Modulator Cyan (`colCyan`), visually communicating that this slider attenuates modulation incoming from the Modulator.
  - **Filter Env Card — Pre-Filter Drive Slider (Blue)**:
    - Updated `filterEnvDriveSlider` accent to Filter Blue (`colBlue`), communicating that this drive stage feeds into the Filter module.
  - **Pitch Env Card — Dual-Accent Destination Buttons**:
    - Extended `LedSelectorComponent` in `source/UIComponents.h` and `source/UIComponents.cpp` to support custom per-item dual-color styling:
      - `Car`: Carrier Red accent (`colRed`).
      - `Mod`: Modulator Cyan accent (`colCyan`).
      - `Both`: Carrier Red text with Modulator Cyan outline and dual Red/Cyan LED halo.
      - `Opp`: Modulator Cyan text with Carrier Red outline and dual Cyan/Red LED halo.
      - Applied 28% border tint and 38% text tint against `#b0bdd0` for clear legibility in unselected states.
  - **Amplifier Card — Velocity Floor Label**:
    - Standardized Knob 4 label from `"Velocity"` to **`"Vel Min Level"`** (aligning with `"Vel Slope"` directly above it on Knob 3).
- **Verification**:
  - All 33 DSP unit tests passed.
  - Rendered GUI verified via `capture_planter_screenshot` (`screenshots/TheKlangPlanter_GUI.png`).

---

### 2. Fix FX Catalog Alphabetical Ordering (Phase Smear Slot 4 Fix)
- **Linear Issue:** [THE-6](https://linear.app/the-klang-farmer/issue/THE-6/fix-fx-catalog-alphabetical-ordering-phase-smear-slot-4-fix)  
- **Commits:** `3f16f87`, `b441b3e`  
- **Plan Archive:** [`2026-10-04_THE-6_fix-fx-catalog-alphabetical-ordering.md`](../../.agents/pipeline/plans/completed/2026-10-04_THE-6_fix-fx-catalog-alphabetical-ordering.md)
- **Summary & Technical Design**:
  - Re-ordered the FX catalog so **Phase Smear** (formerly PhaseSmear) sits in its proper alphabetical position:
    - Slot 0: `None`
    - Slot 1: `Bitcrusher`
    - Slot 2: `Chorus`
    - Slot 3: `Comb Filter`
    - Slot 4: `Drive` (shifted from 5)
    - Slot 5: `Filter` (shifted from 6)
    - Slot 6: `Flanger` (shifted from 7)
    - Slot 7: `Frequency Shifter` (shifted from 8)
    - Slot 8: `Grit FX` (shifted from 9)
    - Slot 9: **`Phase Smear`** (moved from legacy slot 4)
    - Slots 10–13: `Phaser`, `RingMod`, `Tempo Delay`, `Wave Folder` (unchanged)
  - **Backward-Compatible State Migration**:
    - In `source/PluginProcessor.cpp` `setStateInformation()`, added `fxCatalogVersion = 2` attribute.
    - Legacy presets (missing `fxCatalogVersion` or `< 2`) automatically remap slot indices:
      - 4 → 9 (legacy PhaseSmear → new Phase Smear)
      - 5..9 → 4..8 (shifted one position down)
    - Guarantees 100% sonic fidelity when users open older projects or presets.
- **Verification**:
  - Added unit test 29 in `test/dsp_tests.cpp` asserting canonical alphabetical order.
  - All 33 DSP unit tests passed.

---

### 3. Autonomous Engineering Pipeline & Git Protection
- **Commits:** `09a0e66`, `fb80d0d`
- **Summary & Technical Design**:
  - Established workspace-level `.agents/` infrastructure:
    - `.agents/rules.md`: Audio thread invariants, C++20 toolchain rules, pre-approved Linear/Git autonomy, and **Git Branch Protection Guardrail** to prevent unapproved edits or pipeline execution on `master`/`main`.
    - `.agents/skills/context-extract/SKILL.md`: Extracts focused schemas, specs, tree, and Linear context for external planning (`/contextextract`).
    - `.agents/skills/paste-plan/SKILL.md`: Ingests plans, enforces branch protection gate, and executes automated multi-phase build loop (`/pasteplan`).
    - `.agents/skills/task-finish/SKILL.md`: Verifies completion, commits changes, syncs Linear tickets to Done, and prompts for `/clean-plan`.

---

## 🏆 Release v0.1.6 (2026-10-03)

### 4. "The Klang Planter" Standalone Synthesizer Plugin
- **Commit:** `d04353c`
- **Summary & Technical Design**:
  - Implemented **The Klang Planter**, an 8-card compact single-voice FM drum synthesizer:
    - **Carrier**: Waveform morphing (Sine/Tri/Saw/Square/PWM), Semitones, Fine Tune, Mod Depth slider.
    - **Modulator**: FM Operator with independent frequency ratios, envelope modulation, and feedback.
    - **Pitch Envelope**: Percussive pitch envelope with destination routing (`Car`, `Mod`, `Both`, `Opp`).
    - **Filter Envelope & Drive**: Dedicated envelope with pre-filter drive stage.
    - **Amp Envelope**: Exponential decay and sustain controls with velocity tracking.
    - **Transients / Noise**: Metallic noise generator, crackle, and transient punch.
    - **Filter**: 12/24/36 dB slopes with Resonance, Drive, and Cutoff.
    - **Master Output**: Pan, Gain, Limiter, and stereo metering.
  - Built and deployed as a separate VST3 bundle alongside The Klang Farmer to `C:\Program Files\Common Files\VST3\The Klang Planter.vst3`.

---

## 🏆 Release v0.1.0 – v0.1.5 (Foundational Releases)

### 5. The Klang Farmer Core Architecture
- **Summary & Technical Design**:
  - Dual-voice FM / wavetable synthesis engine with real-time morphing.
  - Multi-bus routing architecture (Pre-FX rack, Post-FX rack, Master Limiter).
  - 13 dynamic audio effects processors (Bitcrusher, Chorus, Comb Filter, Drive, Filter, Flanger, Frequency Shifter, Grit FX, Phase Smear, Phaser, RingMod, Tempo Delay, Wave Folder).
  - Real-time stereo modulation visualizer and oscilloscope XY plotting modes.
  - Fully real-time audio thread compliant (zero allocations, zero locks, zero blocking I/O).


### [COMPLETED] Core Architecture Base Class Refactor
Refactor codebase to use a "Common Core" base-class architecture (KlangCoreProcessor, KlangCoreEditor) to share standard JUCE boilerplate, UI LookAndFeel, preset management, and APVTS loading/saving across TheKlangFarmer, TheKlangPlanter, and The Klang Seed (Hardware).

### [COMPLETED] Data-Driven UI Layout & Colors (	heme.json)
Extract the remaining hardcoded UI configuration out of C++ (PluginEditor.cpp) into JSON assets (ssets/controls/theme.json or layout.json).

---

## 🚫 Archived Explorations & Deprecated Concepts

### [DEPRECATED] 2x5 Eurorack Modular Layout Exploration
- **Original Proposal**: Investigate moving from the current 2x4 (8-card) chassis to an expanded 2-row by 5-column (2x5, 10-card) layout.
- **Reason for Deprecation**: Closed in favor of strict opinionated 2x4 (8-card) design identity. Expanding to 10 cards breaks the mathematical 16-encoder banking on TBD-16 hardware ($4 	imes 4 = 16$), breaks existing JSON layout schemas, and causes screen layout bloat on standard laptop displays. Preserved for historical context only.
