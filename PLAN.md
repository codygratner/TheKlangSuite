# 🥋 Unified Plan: Hastur HS-01 Elevated Lifeform Anti-Aliasing Architecture (Opus + Argon)

> **PCDACore Specification Standard** (Pillar 1: Dialectic Cognition · Pillar 2: Resequence DAG · Pillar 3: Crucible Execution)  
> **Heavy Tier Synthesis**: Gemini 4 Argon & Claude Opus 5.5 / Sonnet Pro Bridge  
> **Track:** `TRACK: DSP` (2x Unified Monolithic Enclosure, ADAA1 Non-Linearities, Rate-Invariant Vactrol) | **Target:** `hastur`  
> **Autoplan State Dossier:** `.agents/scratch/autoplan_state/hastur_lifeform_alias_fix/`  
> **Status:** 🟢 **COMPLETED & VERIFIED (52/52 SUITES GREEN)**

---

## 🔬 Section 1: The Research Triad & Forensic Breakthrough

### 1. The Root Cause & Why All Three Modes Aliased at High Notes
* **The Downstream 1x Saturation Seam**:
  In `HasturVoice.h`, the 2x oversampler decimator was placed *before* the Lifeform stage (`masterLpg_`) and `postAmpDrive`. The non-linear saturation polynomials (Synthetic 4th-order algebraic soft clipper, Organic 12AX7 cubic tube, Cyborg MS-10 OTA, and post-amp $\tanh$) executed at **1x host sample rate** (44.1/48 kHz).
* **Why 2x Oversampling Alone is Insufficient for Hard $\tanh$**:
  As Opus proved, at C8 ($4186\text{ Hz}$), high drive in `postAmpDrive` ($5\times$ gain) pushes $\tanh$ into hard saturation where harmonics decay as $\sim 1/n$. Even with 2x oversampling (Nyquist $= 44.1\text{ kHz}$ at 44.1k host), harmonic $n \ge 17$ folds back into the audible band at $-25\text{ dBc}$.
* **The Cyborg 1x Failure**:
  Cyborg was previously running completely at 1x host rate. At high notes (C6–C8), its asymmetric diode clamp, MS-10 OTA, and 8x chopper threw dozens of loud inharmonic foldover spurs across the entire spectrum.

### 2. The 4 Foundational Architectural Upgrades (OP-1 to OP-4)
* **OP-1 (Unified 2x Domain & Zero Rate Switching)**:
  - Run **ALL 3 modes (Synthetic, Cyborg, Organic)** inside the monolithic 2x core (`voiceCoreSr = 2.0 * hostSampleRate_`).
  - `voiceCoreSr` is a compile-time constant for the voice. **Delete dynamic rate re-preparation in `setLifeMode()`**. Live mode switching becomes 100% click-free and zero-latency!
  - Cyborg retains its full industrial personality via its asymmetric diode seam, 17.5 kHz bus filter (now running inside the 2x loop to actively roll off ultrasonics), and BBD decimation.
  - Linear sub-sample interpolation of `ampGateSig` across the two 2x ticks.
* **OP-2 (1st-Order Antiderivative Anti-Aliasing - ADAA1)**:
  - Apply 1st-order ADAA to `postAmpDrive` $\tanh$, Cyborg MS-10 OTA $\tanh$, and Cyborg's asymmetric diode clamp $[-0.72, +0.32]$.
  - Tanh antiderivative: $F(x) = |x| + \frac{1}{g} \ln(1 + e^{-2g|x|}) - \frac{\ln(2)}{g}$, with L'Hôpital midpoint fallback when $|x_n - x_{n-1}| < 10^{-5}$.
  - Diode clamp antiderivative: exact piecewise quadratic $F(x) = \int \text{clamp}(x, L, U) dx$.
* **OP-3 (Rate-Invariant Vactrol & Stereo Optimization)**:
  - Scale Vactrol dual-exponential decay via $c_{2x} = \sqrt{c_{\text{ref}}}$ to preserve exact physical release timing.
  - Use exact exponential for leaky DC blocker: $\alpha = 1 - e^{-2\pi f_c T}$.
  - Share the optical follower integration and SVF coefficient $g = \tan(\pi f_c / f_s)$ across L and R, roughly halving transcendental calls while keeping independent stereo SVF state.
  - Center Cyborg edge-tear smoothstep on 0.75 over $[0.70, 0.80]$.
* **OP-4 (8192-Point Blackman-Harris Coherent FFT Gauntlet - Suite 52)**:
  - Benchmark on coherent frequency bins near C6 (1046 Hz), C7 (2093 Hz), and C8 (4186 Hz).
  - Maximum torture configuration: `drive = 1.0`, `resonance = 0.85`, open cutoff.
  - Assert: Synthetic $\ge 42\text{ dB}$, Organic $\ge 40\text{ dB}$, Cyborg $\ge 34\text{ dB}$ spur rejection.

---

## 🎯 Section 2: Build Execution Manifest Anchors

### 1. Agentless Localization Scope
* **Authorized Files**:
  - `source/hastur/VactrolLPG.h` (rate-invariant coeffs, shared optical follower, ADAA1 helper, centered smoothstep)
  - `source/hastur/HasturVoice.h` (unified 2x core for all modes, eliminate re-prep in `setLifeMode`, enclose LPG and post-amp drive in 2x loop with sub-sample amp interpolation)
  - `source/hastur/AllPassFilter4.h` (clamp max cutoff to $0.42 \times f_s$)
  - `test/dsp_tests.cpp` (implement Suite 52 coherent FFT gauntlet)
* **Read-Only / Protected Files**:
  - `contracts/hastur.yaml` (Zero APVTS parameter changes)
  - `source/hastur/HasturEditor.*` (Zero UI changes required)

---

## 🚀 Section 3: The Canonical 4-Phase Fast-Loop Sequence

### Phase 1: ADAA1 Math Primitives & Vactrol Rate Invariance [GATE 1]
- [x] 1.1 In `source/hastur/VactrolLPG.h`:
  - Implement struct `ADAA`:
    - `static inline float tanh(float x, float x1, float g, float& s)`
    - `static inline float clamp(float x, float x1, float minVal, float maxVal)`
  - Update `VactrolLPG::prepare(double sampleRate)`:
    - Use rate-invariant dual-exponential formula: $c = \sqrt{0.72 e^{-T_h/\tau_1} + 0.28 e^{-T_h/\tau_2}}$.
    - Use exact DC blocker: $\alpha = 1.0 - \exp(-2.0 \pi f_c T)$.
  - In `mode == BackendMode::CYBORG`:
    - Replace hard edge-tear branch with smoothstep centered on 0.75 over $[0.70, 0.80]$:
      ```cpp
      const float absIn = std::abs(audioIn);
      float tearWeight = 0.0f;
      if (absIn > 0.70f) {
          const float u = std::min(1.0f, (absIn - 0.70f) * 10.0f);
          tearWeight = u * u * (3.0f - 2.0f * u);
      }
      const float edgeTear = (audioIn - cyborgFilterState_) * (0.08f * cEff * tearWeight);
      ```
    - Apply ADAA1 to MS-10 OTA saturation $\tanh(1.75 x + 0.22 x |x|)$.
  - Add `processStereo(float inL, float inR, float controlLevel, float& outL, float& outR)` to evaluate optical follower and SVF $g$ coefficient once per stereo pair.
- [x] 1.2 In `source/hastur/AllPassFilter4.h`:
  - Clamp internal break frequency to $0.42 \times f_s$ in `processAPFLifeform` and `processInvertedAPF`.

### Phase 2: Unified 2x Voice Domain & Monolithic Enclosure [GATE 2]
- [x] 2.1 In `source/hastur/HasturVoice.h`:
  - In `prepare(double sampleRate)`:
    - Unify `voiceCoreSr = hostSampleRate_ * 2.0` across **all three modes** (Synthetic, Cyborg, Organic).
    - Prepare `osc1_`, `osc2_`, `filter_`, `filterR_`, `cyborgBusFilterL_`, `cyborgBusFilterR_`, `masterLpg_`, `masterLpgR_` at `voiceCoreSr`.
  - In `setLifeMode(LifeMode mode)`:
    - Set mode state only (`lifeMode_ = mode`).
    - **Remove calls to `prepare()` and `decimator_.reset()`** — zero state wipes, zero mode-switch clicks!
- [x] 2.2 In `processStereoSample()`:
  - Enclose the entire signal chain inside the 2x oversampling loop ($k < 2$):
    - Sub-sample interpolate `ampGateSig`: $A[k] = A_{\text{prev}} + (k + 1) \cdot 0.5 \cdot (A_{\text{curr}} - A_{\text{prev}})$.
    - For Cyborg mode: apply ADAA1 diode clamp $[-0.72, +0.32]$ and run 17.5 kHz bus filter at 2x rate.
    - Evaluate `masterLpg_.processStereo(subFiltL[k], subFiltR[k], A[k], lpgOutL[k], lpgOutR[k])`.
    - Apply `postAmpDrive` using `ADAA::tanh` at 2x rate.
  - Downsample the saturated, shaped stereo output via `decimator_.downsample(postL[0], postL[1], postR[0], postR[1], outSigL, outSigR)`.
  - Tap `subFiltL/R` before the LPG stage to maintain exact SSoT semantics for `prevPostFilter_` feeding `ModMatrix`.

### Phase 3: Empirical Anti-Aliasing Gauntlet in `dsp_tests.cpp` (Suite 52) [GATE 3]
- [x] 3.1 In `test/dsp_tests.cpp`:
  - Add **SUITE 52: Hastur HS-01 Elevated Lifeform Anti-Aliasing Gauntlet**:
    - Synthesize steady-state 8192-sample buffers at coherent bin frequencies near C6 (1046.5 Hz), C7 (2093.0 Hz), and C8 (4186.0 Hz).
    - Test all 3 Lifeform modes under maximum drive (`postAmpDrive = 1.0`, `mixerDrive = 1.0`, `resonance = 0.85`, cutoff open).
    - Apply 7-term Blackman-Harris window and 8192-point FFT.
    - Measure peak fundamental vs. maximum inharmonic aliased spur in the audio band ($20\text{ Hz} \dots 20\text{ kHz}$).
    - Assert:
      - Synthetic Mode: $\ge 42\text{ dB}$ spur rejection.
      - Organic Mode: $\ge 40\text{ dB}$ spur rejection.
      - Cyborg Mode: $\ge 34\text{ dB}$ spur rejection.
  - Verify all 52 DSP test suites pass 100%.

### Phase 4: Tower Build & Fast-Sweep Validation [GATE 4 & 5]
- [x] 4.1 Run Tower build bridge:
  `python tools/context_bus_bridge.py build --target dsp_tests --dispatch`
- [x] 4.2 Confirm 52/52 test suites green.
- [x] 4.3 Verify clean packaging of `Hastur_VST3`.
