# 🧠 PLAN: Nyarlat Additive Timbral Consolidation & Chaos Swarm
### *Full Architectural Modernization: Additive Oscillators, 10m Chaos Matrix, Silky Analog Filter & 8x Unison*

> **Status:** COMPLETED & DEPLOYED (`DEPLOYED_RELEASE`)  
> **Origin:** User Autoplan Directive ("build and deploy!")  
> **Execution Channel:** Shielded Tower Compilation Bridge (`python tools/context_bus_bridge.py`)  

---

## 🏛️ Executive Summary

Nyarlat transforms from a disjointed hybrid (3 bitmask oscillators competing with a 14-pole E-mu Z-plane cube) into a focused, unified masterwork of **living timbral synthesis**:
1. **Bay 1 (3× Additive Bitmask Oscillators):** 6 orthogonal parameters per oscillator (`Morph`, `Odd/Even`, `Tilt`, `Formant`, `Mask`, `Speed`). Dynamically bakes 64 harmonics into 512-sample cycles at seams/morphs, rendered via Nyarlat's sub-1% Casio CZ Bitmask Stator engine.
2. **Bay 2 (Silky ZDF Analog Filter & Envelopes):** Replaces the Z-cube with a musical Zero-Delay Feedback analog filter (12dB LP, 24dB LP, 12dB BP) with liquid resonance, tube warmth/drive, and snappy clav-to-pad ADSR envelopes + Clavinet hammer-snap noise.
3. **Bay 3 (6×3 Chaos Timbral Matrix):** 10-minute down to 1-second continuous Brownian wander without artificial slew floors. 18 bipolar attenuverters mapping 1:1 to the 6 oscillator parameters across all 3 oscillators.
4. **Master & Voicing Deck:** First-class `Mono`, `Legato` (glide without retrigger), and `8x Chaos Unison Swarm` (24 additive oscillators with independent per-voice Brownian modulation seeds and golden-ratio stereo spread). Continuous zero-latency Alpha Chorus (0.2 to 6.0 Hz).
5. **Universal PresetManager:** Full `StepperAuditionBar` in header with 32 curated factory presets across 4 signature banks.
6. **Z-Plane Preservation:** The complete 288-setup Z-Plane engine and 3D radar are archived cleanly for the upcoming standalone E-mu flagship plugin.

---

## 📋 Phase Breakdown & Execution Sequence

### Phase 1: Contract, Codegen & Parameter Layout Lock
- [x] Update `contracts/nyarlat.yaml` with the new schema (6 oscillator macros, ZDF filter parameters, 6×3 chaos matrix, continuous alpha chorus).
- [x] Run `python tools/codegen_apvts.py -p nyarlat` to regenerate `NyarlatParameterIds.h` and `NyarlatParameterLayout.h`.
- [x] Update `NyarlatControlInfo.h` with full documentation and tooltips for all new parameters.

### Phase 2: Core DSP Engine Implementation
- [x] Implement `NyarlatAdditiveOscillator`:
  - 64-partial harmonic synthesis with `Morph` (seed spectra), `Odd/Even` balance, `Tilt` (spectral slope), `Formant` (peaking bell curve).
  - Triad Lifeform physics: `SYNTHETIC` (pure integer harmonics), `ORGANIC` (stiff-string dispersion $f_k = k f_0 \sqrt{1 + B k^2}$ + 2nd harmonic tube saturation), `CYBORG` (6/8-bit amplitude quantization + rail foldback).
  - Bakes single cycle into `BitmaskOscillator` stator cache.
- [x] Implement `NyarlatAnalogFilter`:
  - Zero-Delay Feedback (ZDF) SVF / 4-pole cascade (12dB LP, 24dB LP, 12dB BP).
  - Soft-limiting liquid resonance and analog drive.
- [x] Overhaul `NyarlatChaosMatrix.h`:
  - Remove `0.001f` minimum slew floor.
  - Implement 10-minute to 1-second cubic Hermite wander with per-voice polyphonic decorrelation offsets ($\phi_v = v \cdot 0.3927f \cdot \text{polySpread}$).
  - 18 output channels (6 parameters × 3 oscs).
- [x] Overhaul `NyarlatProcessor.cpp` Voicing & Master Effects:
  - First-class `Mono`, `Legato` (pitch glide with `glide_time` without ADSR retrigger), and `8x Unison` (triggers 8 voices with golden-ratio detune and independent per-voice chaos seeds).
  - Continuous Alpha Chorus: analytic quadrature phase rotation without delay buffers.
  - Clavinet hammer-snap noise transient envelope.
  - Headroom scaling: calibrated $-12\text{ dB}$ ($0.25\times$) voice accumulation.

### Phase 3: CrackedChimes UI & Vector OLED Canvas
- [x] Overhaul `NyarlatEditor.h` and `NyarlatEditor.cpp`:
  - Remove `ZCubeVisualizer`. Install `VectorWaveOledCanvas` rendering true additive waveform + bitmask aperture + cyan stator sweep cursor.
  - Bay 1: 3 Additive Oscillators (6 rows × 3 columns of knobs/selectors).
  - Bay 2: Center Analog Filter (Cutoff, Reso, Slope, Drive) + Filter/Amp ADSR envelopes.
  - Bay 3: Master Chaos Deck (Rate [10m..1s], Drift, Poly) + 6×3 Bipolar Attenuverter Grid.
  - Header: Install `StepperAuditionBar` wired to `PresetManager` + `RockerToggle3` for Lifeform modes.
  - In-dial value badges: Rate formats as `10m 00s`..`1.0 s`; Chorus formats as `0.2 Hz`..`6.0 Hz`.

### Phase 4: Factory Presets, Tower Compilation & Verification Gauntlet
- [x] Generate 32 signature JSON factory presets in `assets/presets/nyarlat/` across 4 categories:
  - `01 - Tectonic Pads & Atmospheres` (10-minute slow chaos drift)
  - `02 - Vintage Clavs & Keys` (12dB filter, odd/even bite, hammer snap)
  - `03 - Choral & Morphing Strings` (additive formant peaks, organic stiffness)
  - `04 - Unison Swarm Leads & Bass` (8x Chaos Unison, Legato portamento glide)
- [x] Update DSP tests in `test/dsp_tests.cpp` (Suite 77) to verify additive harmonics across all 3 Lifeforms, ZDF filter stability, 10-minute chaos timebase, and 8x unison audio torture.
- [x] Run shielded Tower compilation: `python tools/context_bus_bridge.py build --target Nyarlat_All --dispatch`.
- [x] Verify test suite: `dsp_tests.exe` (100% pass across all 79 verification suites).
- [x] Headless UI Pre-flight: `Nyarlat_Preview.exe` verifying 0 collisions and rendering visual proof `nyarlat_ui_proof.png`.
- [x] Deploy Release `Nyarlat.vst3` to system VST3 directory and emit silent harvest confirmation.
