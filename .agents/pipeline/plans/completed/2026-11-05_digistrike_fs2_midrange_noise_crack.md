# 🎯 Crucible Plan: DigiStrike FS2 Midrange Noise Crack (1kHz – 3kHz)

## 📌 Executive Summary
The operator requested re-voicing DigiStrike FS2's noise generator from a wideband high-frequency sizzle to a focused **1kHz to 3kHz "crack / snap"** midrange band. In vintage PCM drum workstations (SP-1200, Linn, Fairlight, Alesis HR-16), wideband high-frequency noise creates acoustic masking across upper harmonics, smoothing out the wavetable's raw digital aliasing and grit.

By implementing a **2-pole Zero-Delay Feedback State Variable Filter (ZDF SVF) Bandpass** centered at $1.8\text{ kHz}$ with $Q = 0.9$, the noise is tightly concentrated between **$1,066\text{ Hz}$ and $3,023\text{ Hz}$**, providing snappy snare/rim crack while rolling off high frequencies by $-15.3\text{ dB}$ at $10\text{ kHz}$ and $-80\text{ dB}$ at $20\text{ kHz}$.

---

## 🔬 Acoustic & Numerical Comparison

| Metric | Legacy Differentiator HPF | Proposed ZDF SVF Bandpass |
| :--- | :--- | :--- |
| **Transfer Function** | $y[n] = x[n] - x[n-1] + 0.85 y[n-1]$ | Trapezoidal ZDF SVF ($f_c = 1.8\text{ kHz}, Q = 0.9$) |
| **Passband (-3dB)** | $2.5\text{ kHz} \dots \text{Nyquist}$ (Wide Open) | **$1,066\text{ Hz} \dots 3,023\text{ Hz}$** |
| **Gain @ 1 kHz** | $-3.38\text{ dB}$ | $+7.08\text{ dB}$ ($-3\text{ dB}$ from peak) |
| **Peak Frequency / Gain** | $24\text{ kHz}$ ($+0.68\text{ dB}$) | **$1,793\text{ Hz}$ ($+10.08\text{ dB}$ w/ RMS matching)** |
| **Gain @ 10 kHz** | $+0.63\text{ dB}$ (Boosted Hiss) | **$-5.20\text{ dB}$ ($-15.3\text{ dB}$ vs peak)** |
| **Gain @ 20 kHz** | $+0.68\text{ dB}$ (Full Sizzle) | **$-69.9\text{ dB}$ ($-80\text{ dB}$ vs peak)** |
| **PCM Wavetable Grit** | ❌ Masked & smoothed out | ✅ **Pristine & unmasked** |
| **Per-Sample CPU Cost** | 2 adds, 1 mul | 4 adds, 4 muls (0 transcendentals) |
| **Voice State Memory** | 2 floats per voice | 2 floats per voice (Reused $s_1, s_2$) |

---

## 🔒 Invariants ($I^*$)
1. **Control Budget Invariant:** Keep the canonical `NOISE` knob on Card 2 Pod 2 (`noise_mix`, K06). Zero APVTS additions, zero UI footprint churn.
2. **Audio-Thread Safety:** Zero heap allocation, zero locks, zero per-sample transcendentals (`std::tan` evaluated strictly in `prepare()` upon sample rate change).
3. **Sample-Rate Invariance:** Exact bilinear pre-warping ensures consistent 1k–3k corner frequencies across 44.1k, 48k, 88.2k, 96k, and 192k.
4. **Denormal Immunity:** Flush-to-zero when $|s_1|, |s_2| < 10^{-15}$.
5. **Loudness Normalization:** RMS power-matched via $noiseGain\_ \approx 3.19\times$ so the tactile throw of the `NOISE` knob matches existing patches and velocity scaling.

---

## 📋 The Canonical 4-Phase Execution Sequence

### Phase 1: Contract & Voice Struct Verification (Gate 1)
- Verify `contracts/digistrike_fs2.yaml` (unchanged, 32-control canon preserved).
- Document the role of SVF integrator states in `DigiStrikeFS2Voice.h`:
  - `v.noiseHpState` $\to$ bandpass integrator state $s_1$ (`ic1eq`).
  - `v.noiseRawPrev` $\to$ lowpass integrator state $s_2$ (`ic2eq`).

### Phase 2: Core DSP Implementation & Unit Verification (Gate 2)
- In [`DigiStrikeFS2Voice.h`](file:///C:/Dev/TheKlangSuite/source/dsp/drums/DigiStrikeFS2Voice.h):
  1. Add precomputed SVF coefficients and gain factor in class private scope:
     ```cpp
     float noiseA1_{0.872967f};
     float noiseA2_{0.103323f};
     float noiseA3_{0.012229f};
     float noiseGain_{3.193f};
     ```
  2. In `prepare(float sampleRate)`:
     ```cpp
     const float fc = std::min(1800.0f, 0.45f * sampleRate_);
     const float k = 1.0f / 0.9f;
     const float g = std::tan(3.14159265f * fc * invSampleRate_);
     noiseA1_ = 1.0f / (1.0f + g * (g + k));
     noiseA2_ = g * noiseA1_;
     noiseA3_ = g * noiseA2_;
     const float enbwHz = 1.57079633f * fc * k;
     noiseGain_ = k * std::sqrt(1.0811f * 0.5f * sampleRate_ / enbwHz);
     ```
  3. In `renderVoice()` (Step 8: Enveloped Noise Mix):
     ```cpp
     if (v.effNoiseMix > 0.001f) {
         float rawNoise = 2.0f * (nextFloat() - 0.5f);
         const float nv3 = rawNoise - v.noiseRawPrev;
         const float nv1 = noiseA1_ * v.noiseHpState + noiseA2_ * nv3;
         const float nv2 = v.noiseRawPrev + noiseA2_ * v.noiseHpState + noiseA3_ * nv3;
         v.noiseHpState = 2.0f * nv1 - v.noiseHpState;
         v.noiseRawPrev = 2.0f * nv2 - v.noiseRawPrev;
         if (std::abs(v.noiseHpState) < 1e-15f) v.noiseHpState = 0.0f;
         if (std::abs(v.noiseRawPrev) < 1e-15f) v.noiseRawPrev = 0.0f;

         const float bpNoise = nv1 * noiseGain_;
         float envNoise = v.ampEnv * (0.35f + 0.65f * v.pitchEnv);
         sampleVal += bpNoise * envNoise * v.effNoiseMix;
     }
     ```
- Verify compilation and regression suite via The Tower build channel (`dsp_tests.exe`).

### Phase 3: Visual & Spatial Verification (Gate 3)
- Pure DSP voicing overhaul — UI layout remains 100% byte-identical.
- Verify zero UI collisions or regression.

### Phase 4: Tower Release Build & Deployment (Gate 4)
- Shielded build of `DigiStrikeFS2_VST3` and `DigiStrikeFS2_Standalone` in Release mode on The Tower (`C:\Dev\TheKlangSuite`).
- Deploy binary to `current_build/VST3/DigiStrike FS2.vst3`.
- Execute automated devlog harvest & session baton update.
