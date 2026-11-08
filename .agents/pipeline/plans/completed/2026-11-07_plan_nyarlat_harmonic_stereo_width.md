# 🧠 PLAN: Nyarlat Harmonic Width & Coupled Spread Overhaul
### *Pitch-Relative Additive Width, Thue-Morse Symmetrical Pan, Authentic Lifeform Saturation & Mono Invariance*

> **Status:** `COMPLETED`  
> **Origin:** User Directives in Autoplan 3.0 (Turns 1 & 2)  
> **Target:** `Nyarlat` (`C:\Dev\TheKlangSuite`)  
> **Commit:** `7ecb81b`  
> **Architecture:** Linear Mid/Side Harmonic Additive Table Synthesis + Per-Partial Spread Coupling  

---

## 🏛️ Executive Summary

Nyarlat's `WIDTH` control in the Output & Expression card transforms from a generic post-mix Mid/Side scalar into an **acoustically intelligent, pitch-relative harmonic width engine**:
1. **0.0 (Min):** 100% pure mono across all partials, oscillators, chorus, and noise.
2. **0.5 (Noon):** Fundamental ($k=1$) is pinned dead-center in mono across all keys; top partial ($k=64$) is 100% wide stereo.
3. **1.0 (Max):** All partials (including the fundamental) bloom into 100% wide stereo immersion.
4. **Oscillator Spread Coupling:** Width scales Osc 2 and Osc 3 stereo spread per partial. At 75% width, the fundamental follows exactly 50% of the dialed spread ($W_1 = 0.5$). At noon, all fundamentals are mono while overtones fan out to full spread.
5. **Thue–Morse Sequence ($\sigma_k = (-1)^{\mathrm{popcount}(k)}$):** Eliminates lopsided panning on odd-only spectra (square, triangle, clarinet waves).
6. **Authentic Stereo Lifeform Saturation:** `CYBORG` rail-foldback and `ORGANIC` tube saturation run independently on Left and Right channels with shared-peak normalization (zero center image drift).
7. **Linear M/S Pan Law ($L+R = 2M$):** 100% comb-filtering-free, phase-cancellation-proof mono sum invariant.

---

## 📋 Canonical 4-Phase Execution Sequence

### Phase 1: Contract, Codegen & Parameter Layout Lock
- [x] Update `contracts/nyarlat.yaml`:
  - `global_width`: change `range: [-1.0, 1.0]` to `range: [0.0, 1.0]`, `default: 0.5`, `unit: "%"`.
- [x] Run `python tools/codegen_apvts.py -p nyarlat` from `C:\Dev\TheKlangSuite` to regenerate `NyarlatParameterIds.h` and `NyarlatParameterLayout.h`.
- [x] Update `NyarlatControlInfo.h` with new tooltip and documentation for `global_width`.

### Phase 2: Additive Engine & Voice DSP Implementation
- [x] In `source/nyarlat/NyarlatAdditiveOscillator.h`:
  - Add `partialWidth(int k, float widthNorm)` implementing the continuous piecewise function:
    $$W(k, x) = (x \le 0.5f) \ ? \ (2.0f \cdot x \cdot u_k) : (u_k + (2.0f \cdot x - 1.0f) \cdot (1.0f - u_k))$$
  - Add `thueMorseSign(int k)` via C++20 `std::popcount(static_cast<unsigned int>(k))`.
  - Update `NyarlatAdditiveEngine::synthesizeCycleStereo`:
    - Synthesize Mid and Side (or L and R) cycles with 64-bit quantized cache key and 64-slot cache.
    - Compute per-partial pan $p_{i, k} = W_k [\beta_i + \sigma_k (1 - |\beta_i|)]$.
    - Synthesize M/S 512-sample tables via IFFT.
    - Authentic Lifeform Saturation: run Cyborg foldback and Organic tube saturation independently on L/R.
    - Per-channel DC nullification and shared-peak normalization.
- [x] In `source/nyarlat/NyarlatOscillator.h` (`BitmaskOscillator`):
  - Support stereo playback and stator slicing from dual 512-sample cycle buffers (`buffer8kL_`, `buffer8kR_`).
- [x] In `source/nyarlat/NyarlatVoice.h`:
  - Wire oscillator stereo/MS outputs into `mixL` and `mixR`.
  - Pass width and effective oscillator spread ($\beta_i$) into oscillator rendering.
- [x] In `source/nyarlat/NyarlatProcessor.cpp`:
  - Master width stage: enforce master side collapse as $x \to 0$ ($\text{sideGain} = 2.0f \cdot \text{width}$) to guarantee total mono across noise and chorus.
  - State migration: remap legacy state with $x_{\text{new}} = (x_{\text{old}} + 1.0f) \cdot 0.5f$.

### Phase 3: CrackedChimes UI & Kilohearts In-Dial Badge
- [x] In `source/nyarlat/NyarlatEditor.cpp`:
  - Update `Ids::global_width` `textFromValueFunction` to render dynamic Kilohearts badges:
    - At `0.0`: `"MONO"`
    - At `0.5`: `"SPLIT"`
    - At `1.0`: `"WIDE"`
    - In-between: `"X%"`
  - Verify layout integrity and bounds via `Nyarlat_Preview` (0 collisions detected across 164 controls).

### Phase 4: Tower Compilation, DSP Verification & Release Packaging
- [x] Update `test/dsp_tests.cpp` (Suite 77 Test 1B):
  - Test 1: Mono sum invariance ($L+R = 2M$, zero phase cancellation).
  - Test 2: Fundamental pinned dead-center ($p_1 = 0$) at $x \le 0.5$.
  - Test 3: 75% width $\implies$ fundamental follows 50% spread.
  - Test 4: Thue-Morse spectral balance on square/odd waves.
  - Test 5: Authentic Lifeform stereo saturation energy symmetry ($\le 0.5\text{ dB}$ difference).
- [x] Compile via Tower Shielded Bridge: `python tools/context_bus_bridge.py build --target Nyarlat_All --dispatch`.
- [x] Run DSP test gauntlet: `dsp_tests.exe` (100% pass across all 80 verification suites).
- [x] Run headless preview: `Nyarlat_Preview.exe` to verify 0 layout collisions.
- [x] Deploy Release `.vst3` and emit silent harvest confirmation.
