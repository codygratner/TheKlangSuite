# Specification: `SoundEngine8` 8-Parameter Synthesizer Oscillator Suite (`include/dsp/oscillators/`)

## 1. Executive Summary & Architectural Contract

The **`SoundEngine8`** oscillator suite in [`C:\Dev\TheKlangDSP\include\dsp\oscillators\`](file:///C:/Dev/TheKlangDSP/include/dsp/oscillators/) provides a unified family of zero-allocation, hard real-time C++20 synthesizer sound engines shared 1:1 between **TheKlangSuite** (`TheKlangFarmer` / `TheKlangPlanter`) and **ToadTracker** (`BACKLOG-12`).

Every engine implements the **Universal 8-Control Card Standard** (`setParameter(0..7, float normalizedValue)`), mapping 1:1 to:
- Dual 4-Control Neo-Slate cards in **TheKlangSuite**.
- Page 1A / Page 1B physical push-encoders on the **dadamachines TBD-16** groovebox.
- The 8-slider instrument editor screen (`VIEW_INSTRUMENT`) in **ToadTracker**.

### Cross-Referenced Hardware & Synth Manuals
- [**Rosetta Stone Master IP Reference**](file:///C:/Dev/Research/library/manuals/ROSETTA_STONE_MASTER_IP_REFERENCE.md) — Sovereign nomenclature and trademark sanitization ledger (`AlphaKraft`, `CosmoPhase`, `TheR1`, `Pluck`, `ArgentSlayer`).
- [**Noise Engineering Basimilus Iteritas Alter (BIA) Manual**](file:///C:/Dev/Research/library/manuals/noise_engineering_bia_manual.md) — Multi-operator additive/FM percussion, harmonic spread, and West Coast wavefolding inspiration.
- [**Noise Engineering Manis Iteritas Manual**](file:///C:/Dev/Research/library/manuals/noise_engineering_manis_manual.md) — Industrial sawtooth modulation, pitch-envelope smash, and aggressive saturation staging.
- [**Arturia Pigments Manual**](file:///C:/Dev/Research/library/manuals/arturia_pigments_manual.md) — Multi-engine phase modulation, harmonic unison spread, and visual modulation ergonomics.
- [**ALM Busy Circuits ALM021 MCO Mk1 Manual**](file:///C:/Dev/Research/library/manuals/alm021_mco_mk1_manual.md) — Compact digital wavetable/pulse/phase-distortion oscillator ergonomics.
- [**TheKlangDSP Decoupling Spec**](klang_dsp_decoupling.md) & [**FX Catalog Expansion Spec**](fx_catalog_expansion.md).

---

## 2. Engine 1: `KlangJuno` / `AlphaKraft` (`include/dsp/oscillators/KlangJuno.h` & `AlphaKraft.h`)

Modeled after the **Roland MKS-50 / Alpha Juno-2 DCO** and **JP-8000 Supersaw**, combining 14 band-limited digital-controlled oscillator waveforms (`PolyBLEP::alphaWave14`), a 7-voice detuned Hoover/Supersaw stack, high-pass/sub shaping, and an integrated stereo BBD Chorus.

### 8-Parameter Mapping (`AlphaKraft` / `KlangJuno`)
| Param | Name | Range / Law | DSP Function |
| :---: | :--- | :--- | :--- |
| **0** | **Pitch** | $20\,\text{Hz} \dots 4000\,\text{Hz}$ (Exp) | Master fundamental DCO frequency ($20 \cdot 200^p$). |
| **1** | **Wave14** | Discrete `0..13` | Selects among the 14 Alpha Juno DCO waveforms (`PolyBLEP::alphaWave14`): Off, Saw, Comb Saw 1–2, Pulse, Narrow Pulse, Trill Pulse, Sub-Square/Pulse variants, and High-Freq Octaves. |
| **2** | **PWM / Speed** | $0.0 \dots 1.0$ | Modulates pulse width duty cycle ($0.08 \dots 0.92$) and internal PWM LFO modulation speed ($0.2\,\text{Hz} \dots 12\,\text{Hz}$). |
| **3** | **Sub Level** | $0.0 \dots 1.0$ | Level of the $-1$ octave / $-2$ octave digital sub-oscillator divider. |
| **4** | **Hoover Detune** | $0.0 \dots 1.0$ | Spreads the 6 satellite JP-8000 Supersaw oscillators around the center core using asymmetric prime detune ratios ($\pm 1.8\%$, $\pm 3.9\%$, $\pm 6.7\%$). |
| **5** | **HPF / Cutoff** | $20\,\text{Hz} \dots 16\,\text{kHz}$ (Exp) | 2-pole Cytomic ZDF state-variable filter shaping brightness and Hoover resonance. |
| **6** | **Env Decay** | $10\,\text{ms} \dots 4.0\,\text{s}$ | Internal amplitude and filter/pitch sweep decay envelope. |
| **7** | **Chorus Rate** | $0.0 \dots 1.0$ | Integrated stereo Juno-60 / Alpha Juno BBD Chorus depth & swept clock rate. |

---

## 3. Engine 2: `KlangCZ` / `CosmoPhase` (`include/dsp/oscillators/KlangCZ.h` & `CosmoPhase.h`)

Modeled after the **Casio CZ-101 / CZ-1 Twin-Line Phase Distortion** synthesis architecture. Instead of filtering a static waveform, `KlangCZ` warps the phase readout angle $\phi \in [0, 1)$ of a cosine lookup table via piecewise-linear transfer functions controlled by Dual Digital Controlled Wave (`DCW 1` & `DCW 2`) timbre parameters.

### 8 Waveforms (`WaveType 0..7`)
- **0–4 (Classic Phase Warps)**: `Sawtooth` (variable inflection knee $k \in [0.02, 0.5]$), `Square` (plateau phase hold), `Pulse` (narrow spike), `DoubleSine` ($2\times$ harmonic pinch), `SawPulse` (hybrid asymmetric half-cycle).
- **5–7 (Windowed Resonant Sync)**: `Resonance1` (Saw window $\times$ hard-synced resonant cosine at $1\times \dots 16\times f_0$), `Resonance2` (Triangle window), and `Resonance3` (Trapezoid window).

### 7 Twin-Line Interaction Modes (`LineMode 0..6`)
- `Line1` (`0`), `Line2` (`1`), `Mix12` (`2`: $\text{L1}+\text{L2}$), `Detune12` (`3`: chorused twin lines), `RingMod` (`4`: $\text{L1} \times \text{L2}$ metallic clangs), `NoiseMod` (`5`: LFSR phase jitter), and `DoubleRing` (`6`: $\text{Mix} + \text{Ring}$).

### 8-Parameter Mapping (`CosmoPhase` / `KlangCZ`)
| Param | Name | Range / Law | DSP Function |
| :---: | :--- | :--- | :--- |
| **0** | **Pitch** | $20\,\text{Hz} \dots 4000\,\text{Hz}$ (Exp) | Fundamental phase accumulator frequency. |
| **1** | **Wave 1** | Discrete `0..7` | Line 1 Phase Distortion waveform selector. |
| **2** | **Wave 2** | Discrete `0..7` | Line 2 Phase Distortion waveform selector. |
| **3** | **DCW 1** | $0.0 \dots 1.0$ | Line 1 phase angle distortion depth / resonant harmonic multiplier ($1\times \dots 16\times$). |
| **4** | **DCW 2** | $0.0 \dots 1.0$ | Line 2 phase angle distortion depth. |
| **5** | **Line Mode** | Discrete `0..6` | Selects among the 7 Twin-Line combination, ring-mod, and noise-mod topologies. |
| **6** | **Detune** | $-12\,\text{st} \dots +12\,\text{st}$ | Bipolar pitch offset and fine beating between Line 1 and Line 2. |
| **7** | **DCW Decay** | $5\,\text{ms} \dots 4.0\,\text{s}$ | Exponential decay time of the internal DCW timbre and VCA envelopes. |

---

## 4. Engine 3: `KlangFM` / `TheR1` (`include/dsp/oscillators/KlangFM.h` & `TheR1.h`)

A **4-Operator Linear Phase Modulation** synthesizer featuring 7 classic Yamaha DX/TX/OPL routing algorithms (`Algorithms 0..6`) plus **Algorithm 7: Korg Electribe ER-1 Cyclic Ring-Modulated Noise Percussion** (`TheR1`).

### 8 Algorithms (`0..7`)
- **Alg 0 (Serial Cascade)**: $\text{Op4} \to \text{Op3} \to \text{Op2} \to \text{Op1}$ (deep metallic/industrial FM).
- **Alg 1 (Branched Tree)**: $(\text{Op4} + \text{Op3}) \to \text{Op2} \to \text{Op1}$.
- **Alg 2 (Top Hat / Y-Stack)**: $(\text{Op4} \to \text{Op3} \to \text{Op2}) + (\text{Op4} \to \text{Op1})$.
- **Alg 3 (Dual 2-Op Parallel Pairs)**: $(\text{Op2} \to \text{Op1}) + (\text{Op4} \to \text{Op3})$.
- **Alg 4 (3-Modulator Fan-In)**: $(\text{Op2} + \text{Op3} + \text{Op4}) \to \text{Op1}$.
- **Alg 5 (Shared Modulator Broadcast)**: $\text{Op4} \to (\text{Op1} + \text{Op2} + \text{Op3})$.
- **Alg 6 (4-Op Additive Organ)**: $\text{Op1} + \text{Op2} + \text{Op3} + \text{Op4}$ (with Op4 feedback).
- **Alg 7 (Electribe ER-1 Cyclic Ring-Noise)**: $\text{Carrier}(\text{Op1}) \times \text{Mod}(\text{Op2}) + \text{RingMod}(\text{Op3} \times \text{WhiteNoise})$.

### 8-Parameter Mapping (`TheR1` / `KlangFM`)
| Param | Name | Range / Law | DSP Function |
| :---: | :--- | :--- | :--- |
| **0** | **Pitch** | $20\,\text{Hz} \dots 4000\,\text{Hz}$ (Exp) | Base carrier frequency ($f_0$). |
| **1** | **Algorithm** | Discrete `0..7` | Selects among the 7 FM topologies and Alg 7 ER-1 Ring-Noise mode. |
| **2** | **FM Depth** | $0.0 \dots 4\pi\,\text{rad}$ | Global modulator phase index scaling across Operators 2, 3, and 4. |
| **3** | **Ratio 2** | Discrete `0..15` | Coarse harmonic/inharmonic table (`0.5x` to `16.0x`, including $\sqrt{2}$ bell ratios $1.414\times$ and $2.828\times$) for Op2. |
| **4** | **Ratio 3/4** | Discrete `0..15` | Coarse harmonic/inharmonic multiplier for Op3 & Op4 ($1.5\times$ staggered). |
| **5** | **Feedback** | $0.0 \dots 1.0$ | Operator 4 2-sample anti-hunting phase feedback + pitch-envelope punch depth. |
| **6** | **Mod Decay** | $5\,\text{ms} \dots 3.0\,\text{s}$ | Decay rate of modulator envelopes (`Op2..Op4`) relative to carrier sustain. |
| **7** | **Car Decay** | $10\,\text{ms} \dots 5.0\,\text{s}$ | Master carrier amplitude decay and stereo operator spread. |

---

## 5. Engine 4: `KlangPluck` / `Pluck` (`include/dsp/oscillators/KlangPluck.h` & `Pluck.h`)

An **Extended Karplus-Strong Physical Waveguide** synthesizer modeling plucked strings, struck marimbas, hollow wooden tubes, and overdriven acoustic resonators.

### Physical Waveguide Signal Chain
1. **4 Exciters (`Param 1: Exciter`)**:
   - `0: Impulse + Noise Burst` (crisp plectrum pluck).
   - `1: Shaped Pink/Low-Passed Noise` (soft nylon/finger pluck).
   - `2: Half-Sine Wooden Mallet Strike` (marimba/kalimba hammer).
   - `3: Bowed / Continuous Sustained Noise` (hurdy-gurdy / bowed string excitation).
2. **Pick-Position Comb Filter (`Param 2: PickPos`)**: Delays and subtracts the exciter pulse at fraction $\beta \in [0.05, 0.5]$ of the string length, creating authentic spatial comb notches.
3. **3rd-Order Lagrange Fractional Delay Waveguide**: $4096$-sample circular buffer with cubic Lagrange polynomial interpolation for click-free sub-sample pitch accuracy across $20\,\text{Hz}\text{--}4\,\text{kHz}$.
4. **All-Pass String Stiffness Dispersion (`Param 5: Stiffness`)**: First-order all-pass filter inside the waveguide loop stretching upper partials sharp like stiff piano wire or metallic bars.
5. **Internal Loop Saturation (`Param 7: Drive`) & ZDF Body Resonator (`Param 6: Body`)**: Soft `FastMath::fastTanh` waveguide loop saturation feeding a multi-mode wooden body resonance SVF.

---

## 6. Engine 5: `ArgentSlayer` / `Klang808` (`include/dsp/oscillators/ArgentSlayer.h`)

A dedicated **Sub-Bass 808 Pitch-Envelope Punch & West Coast Wavefolding Oscillator** inspired by classic TR-808 bridged-T networks and the [**Noise Engineering BIA / Manis Iteritas**](file:///C:/Dev/Research/library/manuals/noise_engineering_bia_manual.md) industrial percussion paradigm:
- **Exponential Pitch-Drop Envelope**: Instantaneous multi-octave transient snap decaying smoothly into phase-locked sub-bass fundamental ($25\,\text{Hz}\text{--}180\,\text{Hz}$).
- **Asymmetric Wavefolder & Padé Clippers**: Multi-stage sine wavefolding and asymmetric diode saturation adding aggressive mid-range bark without losing low-end weight.
- **Transient Click / Noise Exciter**: Phase-coherent impulse and filtered noise burst for cutting through dense industrial mixes.

---

## 7. Engine 6: `TitanWav` / `KlangTitan` (`include/dsp/oscillators/TitanWav.h`)

The flagship **Universal Wavetable, Spectral & Waveshaping Oscillator**, shared 1:1 between **TheKlangSuite** (`TKF` / `TKP`) and **ToadTracker** (`toad::SuperWavsynth` / `toad::WavetableVoice` / `INST_TYPE_TITAN_WAVSYNTH`).

### Cross-Ecosystem Export Invariant
`TitanWav` guarantees that ToadTracker songs, instruments, and `.tkp` presets utilizing the M8 Wavsynth or Titan Wavsynth engines load bit-for-bit with identical timbre, frame alignment, and modulation response inside The Klang Farmer (TKF) running as a VST3/AU in any DAW.

### Architectural Heritage
Synthesizes the core innovations from 5 legendary wavetable/additive instruments while maintaining 1:1 backward compatibility with Dirtywave M8 Wavsynth:
- **Dirtywave M8**: 128 wavetables, frame size, position scan, forward/reverse/pingpong playback, mirror, cut.
- **Vital Audio Vital**: Frequency-domain Fourier spectral tilt, spectral comb/notch masks, continuous Barber Pole (Shepard-Risset) phase rotation.
- **Xfer Records Serum 2**: Phase distortion opcodes (`Bend+`, `Bend-`, `Bend+/-`, `Asym`, `Flip`, `Mirror-2`, `Quantize` phase decimation).
- **Cakewalk Z3TA+**: 16-point non-linear waveshaping matrix with cycle-level resonance (`RESO`) and diode rectification.
- **Arturia Pigments**: Oscillator-core Buchla wavefolding and single-voice unison chord stacking.
- **Native Instruments Razor**: 320-partial additive resynthesis with uniform frequency shifting ($\Delta f$) and zero-phase formant waterbed filtering.

### 8-Parameter Mapping (`TitanWav` / `KlangTitan`)
| Param | Name | Range / Law | DSP Function |
| :---: | :--- | :--- | :--- |
| **0** | **Pitch** | $20\,\text{Hz} \dots 4000\,\text{Hz}$ (Exp) | Fundamental oscillator frequency ($f_0$). |
| **1** | **Table / Wave** | Discrete `0..127` | Selects from 128 single-cycle or multi-frame wavetables (M8 factory tables + user `.wave.json` / `.wt` tables). |
| **2** | **Scan / Pos** | $0.0 \dots 1.0$ | Continuous wavetable frame position scan ($0\dots 255$) with spectral cross-fading. |
| **3** | **Warp Mode** | Discrete `0..6` | `0: M8 Classic Warp`, `1: Vital Spectral Tilt`, `2: Vital Barber Pole`, `3: Serum Bend+/-`, `4: Serum Quantize`, `5: Z3TA+ Cycle Reso`, `6: Razor Frequency Shift`. |
| **4** | **Warp Depth** | $0.0 \dots 1.0$ | Intensity and transfer-curve depth of the active warp algorithm. |
| **5** | **Shaper / Fold** | $0.0 \dots 1.0$ | Pre-filter oscillator-core wavefolding and asymmetric saturation drive. |
| **6** | **Formant / Tilt** | $0.0 \dots 1.0$ | Multi-peak vowel formant filter navigation ($A, E, I, O, U$) and high-frequency spectral tilt. |
| **7** | **Unison / Chord** | $0.0 \dots 1.0$ | Unison detune spread ($0.0\dots 0.5$) and single-voice Chord Stacking ($0.6$: Octave, $0.7$: 5th, $0.8$: Min7, $0.9$: Maj7, $1.0$: Sus4). |

