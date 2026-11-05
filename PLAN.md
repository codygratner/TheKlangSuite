# Implementation Plan: DigiStrike FS2 KLANG Modal Engine & C3 Root Tuning

## 🧭 Autoplan Frame: [TIER: STANDARD] × [TRACK: HYBRID]
**Target:** `digistrike_fs2` (DigiStrike FS2 Sovereign Vintage PCM Workstation)  
**Origin:** Autoplan 3.0 ACF-5 Dual-Track Funnel (3 Adversarial Crucible Rounds Complete — 12 Defects Hardened)  
**Core Objective:** Replace obsolete `noise_mix` with the **Gordon-Smith Modal Strike Engine** (`klang_morph`, short `KLANG`, range `[-100%, +100%]`), integrate the **Triad Sonic Canon** (Synthetic/Cyborg/Organic modal shading), and calibrate **C3 Concert Root Tuning** across all tonal percussion.

---

### 📜 Build Execution Manifest

#### 1. Agentless Localization Scope (Xia et al.)
- **Primary Modification Targets:**
  - `contracts/digistrike_fs2.yaml` (Replace `noise_mix` with `klang_morph`, $\Delta N = 0$)
  - `source/digistrike_fs2/DigiStrikeFS2Catalog.h` (Standardize $f_0$ and `rootMidi = 60` for tonal voices)
  - `source/dsp/drums/DigiStrikeFS2Voice.h` (Implement `ModalBank16`, Gordon-Smith resonators, 5ms slew crossfade, Lifeform damping)
  - `source/digistrike_fs2/DigiStrikeFS2Editor.h` (Replace `noiseSlider_` with `klangSlider_`)
  - `source/digistrike_fs2/DigiStrikeFS2Editor.cpp` (Bind `klangSlider_`, label `"KLANG"`, double-click center `0.0f`)
  - `test/dsp_tests.cpp` (Add Suite 58: DigiStrike FS2 KLANG Modal Engine & C3 Tuning Verification)
- **Read-Only Verification References:**
  - `source/digistrike_fs2/DigiStrikeFS2ROM.h`
  - `source/digistrike_fs2/DigiStrikeFS2Processor.cpp`
  - `source/FastMath.h`

#### 2. Immutable Symbol & Invariant Table (Plan-and-Solve+)
| Symbol / Component | Current Location / State | Required Remediation | Locked Invariant ($I^*$) |
| :--- | :--- | :--- | :--- |
| `noise_mix` | `contracts/digistrike_fs2.yaml:177` | Replace with `klang_morph` | APVTS ID `klang_morph`, short `KLANG`, range `[-100.0, 100.0]`, default `0.0`, $\Delta N = 0$. |
| `ModalBank16` | `DigiStrikeFS2Voice.h` (New struct) | 32-byte aligned Structure-of-Arrays (SoA) layout | Unconditionally stable Gordon-Smith coupled form ($\epsilon_k = 2\sin(\pi f_k / f_s)$). |
| `w_PCM`, `w_Modal` | `DigiStrikeFS2Voice.h` (New members) | 5ms one-pole linear parameter slew | Zero zipper noise during automation sweeps; noon ($K=0.0$) hard-bypasses modal bank ($w_{\text{Modal}} = 0$). |
| `kHeroVoiceMap` | `DigiStrikeFS2Catalog.h` | Set `rootMidi = 60` and calibrated $f_0$ for BD, SN, TM, MC | MIDI note 60 triggers concert pitch C across all tonal percussion; noise classes flagged `isPitched = false`. |
| `lifeMode` | `DigiStrikeFS2Voice.h:880` | Modulate mode damping slope $\alpha$ | Synthetic ($\alpha = 0.75$, exact), Cyborg ($\alpha = 0.45$, rad scream), Organic ($\alpha = 1.10$, vactrol droop). |
| `klangSlider_` | `DigiStrikeFS2Editor.h / .cpp` | Replace `noiseSlider_` | Card 2 Pod 2 Row 2; double-click center returns to `0.0f`; 2D bounds $\Delta\Omega = 0\text{px}$. |

---

### 🚀 Canonical 4-Phase Fast-Loop Sequence

### Phase 1: Contract, Catalog & Codegen Lock (`40ms` Gate)
- [ ] Update `contracts/digistrike_fs2.yaml`: retire `noise_mix`, insert `klang_morph`.
- [ ] Run `python tools/codegen_apvts.py` to regenerate `digistrike_fs2ParameterIds.h` and layout.
- [ ] Update `DigiStrikeFS2Catalog.h`: standardize $f_0$ and `rootKey = 60.0f` across tonal categories (BD=C1, TM=C2, SN=C3, MC=C4). Add `bool isPitched` flag.
- [ ] Assert codegen parity via `python tools/context_bus_bridge.py fast-sweep`.

### Phase 2: Core DSP & Voice Dispatch Implementation (`~3s` Gate)
- [ ] In `DigiStrikeFS2Voice.h`: Define `alignas(32) struct ModalBank16` (SoA layout).
- [ ] Implement Gordon-Smith coupled-form resonator update with Nyquist mode culling ($f_k \ge 0.48 f_s$).
- [ ] Implement 5ms parameter slewed equal-power crossfade ($w_{\text{PCM}} = \cos(\frac{\pi}{2}|K|), w_{\text{Modal}} = \sin(\frac{\pi}{2}|K|)$).
- [ ] Implement Triad Lifeform damping table (`kAlphaTable[3] = { 0.75f, 0.45f, 1.10f }`).
- [ ] Implement 32-sample retrigger quench ($d_{\text{quench}} = 0.80$) and sub-sample MIDI timestamp impulse injection ($n = t_{\text{stamp}}$).
- [ ] Wire `klang_morph` to `DigiStrikeFS2Params` and Arbitrator (`formless::bipolarArbitrate`).
- [ ] Add **Suite 58** to `test/dsp_tests.cpp`:
  - 58.1: Noon Bit-Accurate Bypass Null Test (residual $< -120\text{ dBFS}$).
  - 58.2: Harmonic Snapping Ratio Assertion (left-swing integer overtones).
  - 58.3: Nyquist Aliasing Immunity Assertion (right-swing stiff-plate bounds).
  - 58.4: Retrigger DC Quench Assertion ($|\Delta y| < 0.05$).
  - 58.5: Lifeform Triad Damping Profile Verification.
  - 58.6: Metamorphic Block-Size Invariance (MR1: 32 vs 64 vs 128 vs 512).
- [ ] Run native DSP tests: `python tools/context_bus_bridge.py test`.

### Phase 3: Tri-Layer UI Integration & Spatial Preflight (`~2s` Gate)
- [ ] In `DigiStrikeFS2Editor.h`: replace `noiseSlider_` / `noiseAttachment_` with `klangSlider_` / `klangAttachment_`.
- [ ] In `DigiStrikeFS2Editor.cpp`:
  - Bind `klangSlider_` to APVTS `klang_morph`.
  - Set double-click return to center: `klangSlider_.setDoubleClickReturnValue(true, 0.0f)`.
  - Set label to `"KLANG"` with tooltip.
- [ ] Compile UI Preview: `python tools/context_bus_bridge.py build --target DigiStrikeFS2_Preview --dispatch`.
- [ ] Run headless spatial radar: `python tools/ui_gate.py --plugin digistrike_fs2 --pillars A,C`.
- [ ] Verify 0 bounding collisions ($\Omega = 0\text{px}$) and pass the 5-Gate Visual Preflight Audit.

### Phase 4: Final VST3 Packaging & Hot-Swap Deployment
- [ ] Compile release VST3 target: `python tools/context_bus_bridge.py build --target DigiStrikeFS2_VST3 --dispatch`.
- [ ] Deploy binary via `deploy_vst3.bat` to `C:\Program Files\Common Files\VST3\DigiStrike FS2.vst3`.
- [ ] Execute live window capture with `python tools/capture_ui.py "DigiStrike FS2"` to prove visual integrity.
- [ ] Run `python tools/plan_manager.py finish` to execute the silent session harvest to `DevLogs`, `chronicle.db`, and `SESSION_BATON.md`.
