# Technical DSP Specification: `KlangVCF` (11-Model Resonant Filter Lab)

**Engine ID:** `CARD-02A` / `FX-06`  
**Sovereign Name:** `KlangVCF`  
**Corporate Boring Name:** `11-Model Analog Resonant Filter Workstation`  
**Category:** Flagship Multi-Topology Filter Lab  
**APVTS Prefix:** `kvcf_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section E](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-e)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** The 11 canonical synthesizer filter circuits of electronic music history: Moog 904A Transistor Ladder, Roland TB-303 Diode Ladder, Oberheim SEM State Variable Filter, Korg MS-20 (K35 & OTA), Steiner-Parker Synthacon (Vandal Park), Curtis CEM3320, SSM2040, EDP Wasp CMOS Inverter, Polivoks Formant, Russian Diode, and 3-Peak Vocal Formant.
* **Upstream Offline Reference:** [arturia_pigments_manual.md](file:///C:/Dev/Research/library/manuals/arturia_pigments_manual.md) · [03_mutable_instruments_dsp_encyclopedia.md](file:///C:/Dev/Research/library/03_mutable_instruments_dsp_encyclopedia.md).
* **C++ Implementation Header:** [`KlangVCFCore.h`](file:///c:/Dev/TheKlangDSP/include/dsp/filters/KlangVCFCore.h).

---

## 2. Mathematical Theory & DSP Architecture

KlangVCF consolidates 11 distinct non-linear filter topologies into a zero-latency, zero-delay feedback framework:
1. **Zero-Delay Feedback (ZDF) Trapezoidal Integrator:**
   Continuous-time transfer functions $s^{-1}$ are mapped to digital using trapezoidal integration with feedback delay elimination:
   $$s = \frac{2}{T} \frac{1 - z^{-1}}{1 + z^{-1}}$$
2. **The 11 Canonical Circuit Models:**
   - **Model 1: `Moog 4-Pole 24dB Ladder`:** 4-stage transistor ladder with $\tanh$ saturation at each integrator tap. Includes selectable bass-drop compensation.
   - **Model 2: `Roland TB-303 18dB Diode Ladder`:** 3-pole/4-pole hybrid with unbuffered diode network, asymmetric resonance feedback, and iconic rubbery squelch.
   - **Model 3: `Oberheim SEM 12dB SVF`:** Dual-integrator state variable topology offering simultaneous, phase-coherent Lowpass, Bandpass, Highpass, and Notch outputs.
   - **Model 4: `Korg MS-20 (OTA / K35)`:** Sallen-Key topology with aggressive non-linear clipping in the resonance feedback loop, capable of violent self-oscillation.
   - **Model 5: `Vandal Park (Steiner-Parker)`:** Diode-switched 12dB SVF with internal "Vandal Factor" intermodulation feedback loop.
   - **Model 6: `Curtis CEM3320 24dB`:** Tight, laser-precise punch characteristic of Prophet-5 and Oberheim OB-8.
   - **Model 7: `SSM2040 Discrete Ladder`:** Rich, creamy phase response with euphonic even-order saturation (Prophet-5 Rev 2).
   - **Model 8: `EDP Wasp CMOS Inverter`:** 4069UB CMOS digital inverter running in linear amplifier mode; chaotic, gritty, and noisy.
   - **Model 9: `Formant Bank (3-Peak Vocal)`:** Triple parallel bandpass resonators tuned to vowel formant frequencies ($F_1, F_2, F_3$).
   - **Model 10: `Polivoks Diode Multimode`:** Soviet op-amp filter without capacitors, utilizing internal op-amp slew and diode resistances.
   - **Model 11: `Comb Filter / Resonator`:** Pitch-tracking delay line with positive/negative feedback for physical metallic strings and flanged combs.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`CUTOFF`):** Master filter cutoff frequency ($10\,\text{Hz} \dots 24.0\,\text{kHz}$, Profile D Log). *Push: Hyper-Track Toggle (±400%)*.
* **Knob 2 (`RESONANCE`):** Resonance / Q factor ($0.5 \dots 35.0$, Profile F Hyperbolic). *Push: Lock Self-Oscillating Sine*.
* **Knob 3 (`MODEL SELECT`):** Continuous / Stepped selection through the **11 Canonical Filter Topologies**. Discrete `0..10`. *Push: Next Family*.
* **Knob 4 (`SLOPE / PASS MODE`):** Filter slope (12dB vs 24dB) or Pass Mode (LP, BP, HP, Notch, Formant Morph). Profile B. *Push: Toggle Mode*.
* **Knob 5 (`PRE-DRIVE`):** Analog input stage saturation ($0.0\,\text{dB} \dots +30.0\,\text{dB}$, Profile E). *Push: Bypass Drive*.
* **Knob 6 (`KEYTRACK`):** Acoustic pitch tracking ($-100.0\% \dots 0.0\% \dots +100.0\%$, Profile B). *Push: Snap 1V/Oct (100%)*.
* **Knob 7 (`SPECIAL / CHAR`):** Model-specific character parameter (e.g. Vandal Factor on M5, Bass Comp on M1, Squelch on M2). Profile B. *Push: Max Special*.
* **Knob 8 (`STEREO SPREAD`):** Antiphase cutoff detuning between Left and Right channels ($0.0\% \dots 100.0\%$, Profile B). *Push: Mono Center*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Bass Compensation Mode):** `[ Authentic Vintage Bass Drop | Active Low-End Compensation (+6dB Res) | Ultra-Sub Passthrough ]`.
* **DIP 2 (Non-Linear Saturation Engine):** `[ Pure Tanh Symmetric | Bipolar Diode Shockley | Germanium Asymmetric | Clean Linear ZDF ]`.
* **DIP 3 (Resonance Limit / Safety):** `[ Analog Rail Hard Clamp (±15V) | Soft Saturation Tamer | Unbounded Screamer ]`.
* **DIP 4 (Audio Rate FM Source):** `[ Core Oscillator A | External Audio Input | Envelope Generator 1 | LFO 1 ]`.
* **TRIM 1 (Audio FM Modulation Depth):** Fine trimpot for audio-rate cutoff frequency modulation ($0.0\% \dots 100.0\%$).
* **TRIM 2 (Analog Thermal Slop):** Random thermal drift applied to cutoff frequency and integrator damping.
* **TRIM 3 (Integrator DC Offset Bias):** Injects minute DC bias into feedback paths for second-harmonic warm coloration.
* **TRIM 4 (Makeup Gain Ceiling):** Post-filter level trim to equalize perceptual loudness across all 11 models.

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Active Model | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`Model 1: Moog Ladder`** | `Knob 7` | `BASS COMP` | `0.0..100.0%` | `Profile B` | Restores low fundamental lost to high resonance |
| **`Model 2: TB-303 Diode`** | `Knob 7` | `SQUELCH ACC` | `0.0..100.0%` | `Profile E` | Accented non-linear feedback bias and diode sag |
| **`Model 5: Vandal Park`** | `Knob 7` | `VANDAL FACTOR` | `0.0..100.0%` | `Profile E` | Inter-diode feedback loop drive |
| **`Model 9: Formants`** | `Knob 4, 7` | `VOWEL`, `GENDER` | `A-E-I-O-U`, `±12 st` | `Discrete`, `Profile B` | Vowel morphing and formant throat length |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Polymorphic Zero-Cost Dispatch:** Filter models encapsulated as compile-time templated ZDF cores or function-pointer tables with zero branch prediction penalty.
* **Unconditional Stability:** Diode Shockley solvers use fast analytic approximations (e.g. Lambert-W function approximations) with guaranteed convergence in $\le 2$ cycles.
