# Technical DSP Specification: `56K Vocoder` (32-Band Spectral Vocoder Processor)

**Engine ID:** `FX-17`  
**Sovereign Name:** `56K Vocoder`  
**Corporate Boring Name:** `32-Band Digital Spectral Filterbank Vocoder`  
**Category:** Spectral Analysis & Vocoder Workstation  
**APVTS Prefix:** `voc_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section C](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Roland VP-330 Vocoder Plus (1979, 10-band analog filterbank), Sennheiser VSM 201 (20-band studio vocoder), EMS Vocoder 2000, Motorola DSP56001 fixed-point vocoder algorithms.
* **Upstream Offline Reference:** [vp330_service_manual.md](file:///C:/Dev/Research/library/manuals/vp330_service_manual.md) · [alpha_juno_manual.md](file:///C:/Dev/Research/library/manuals/alpha_juno_manual.md).
* **C++ Implementation Header:** [`Vocoder56kEngine.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/Vocoder56kEngine.h).

---

## 2. Mathematical Theory & DSP Architecture

56K Vocoder is a dual-filterbank channel vocoder combining speech analysis with synthetic carrier modulation:
1. **Analysis & Synthesis Filterbank:**
   Incoming **Modulator** (speech/vocal) and **Carrier** (internal `OSC-12` or external synth) are passed through matched banks of 16 to 32 fourth-order bandpass filters spaced along the Bark auditory scale:
   $$f_k = 300 \cdot \left(e^{0.15 \cdot k} - 1\right)\,\text{Hz}, \quad k = 1 \dots 32$$
2. **Envelope Extraction & VCA Modulation:**
   Each analysis band extracts the spectral envelope using a precision full-wave rectifier and ballistics filter:
   $$e_k[n] = (1 - \alpha) |y_{mod, k}[n]| + \alpha e_k[n-1]$$
   The extracted envelope $e_k[n]$ then modulates the amplitude of the corresponding carrier filter band $y_{car, k}[n]$ via a dedicated digital VCA:
   $$y_{out}[n] = \sum_{k=1}^{N} e_k[n] \cdot y_{car, k}[n]$$
3. **Voiced / Unvoiced Sibilance Detector:**
   Detects high-frequency consonant friction (plosives, "S", "T", "Ch"). When unvoiced speech is detected, the carrier is smoothly crossfaded with highpass noise (`OSC-10` / Flint & Dust style) to maintain crisp intelligibility.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`NUM BANDS`):** Discrete active band count: `8 Bands (Vintage VP-330)`, `16 Bands (Studio Pro)`, `24 Bands`, `32 Bands (Ultra Intelligible)`. Discrete Choice. *Push: Next Count*.
* **Knob 2 (`FORMANT SHIFT`):** Spectral envelope shift factor ($-12.0\,\text{st} \dots +12.0\,\text{st}$, Profile B Bipolar). *Push: Reset Formant (0)*.
* **Knob 3 (`ATTACK / SNAP`):** Envelope detector attack time ($0.5\,\text{ms} \dots 50.0\,\text{ms}$, Profile A). *Push: Instant 1ms*.
* **Knob 4 (`RELEASE / DECAY`):** Envelope detector release time ($5.0\,\text{ms} \dots 500.0\,\text{ms}$, Profile A). *Push: Snap 50ms*.
* **Knob 5 (`SIBILANCE / NOISE`):** High-frequency unvoiced consonant noise injection ($0.0\% \dots 100.0\%$, Profile B). *Push: Auto-Sibilance Link*.
* **Knob 6 (`SPECTRAL TILT / EQ`):** High/Low spectral tilt balance ($-6\,\text{dB/oct} \dots +6\,\text{dB/oct}$, Profile B). *Push: Flat Neutral*.
* **Knob 7 (`CARRIER BLEND`):** Blends between internal clean-room DCO carrier (`OSC-12`) and external audio input. Profile B. *Push: Solo Internal Carrier*.
* **Knob 8 (`MIX`):** Universal Tri-Zone Dry/Wet Mix (Profile M: 7:00 Wet Audition, 12:00 Pure Dry, 5:00 Parallel). *Push: Phase Invert ($\varnothing = 180^\circ$)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Filter Spacing Scale):** `[ Bark Psychoacoustic | Critical Band ERB | Linear Spacing | Mel Scale ]`.
* **DIP 2 (Carrier Source):** `[ Internal OSC-12 Clean MKS-50 | External Stereo Sidechain | Pulse Train | Noise Generator ]`.
* **DIP 3 (Unvoiced Sibilance Mode):** `[ Highpass White Noise Injection | Direct Modulator High-Bleed | Bandpass Gated ]`.
* **DIP 4 (Bandpass Filter Order):** `[ 2-Pole 12dB (Vintage Soft) | 4-Pole 24dB (Sharp Separation) | 8-Pole Razor ]`.
* **TRIM 1 (Voiced/Unvoiced Threshold):** High-frequency zero-crossing detector sensitivity for sibilant detection.
* **TRIM 2 (Filter Bank Resonance Q):** Bandpass filter Q factor ($2.0 \dots 20.0$).
* **TRIM 3 (Envelope Detector Non-Linearity):** Compresses spectral peaks for increased speech intelligibility.
* **TRIM 4 (Noise Gate Floor):** Mutes background noise on modulator speech input during pauses.

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Band Count (Knob 1) | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`8 Bands (VP-330)`** | `Knob 2` | `ENSEMBLE DEPTH`| `0..100%` | `Profile B` | Injects classic VP-330 string chorus ensemble |
| **`External Carrier`** | `Knob 7` | `EXT SENSITIVITY`| `-24..+12 dB` | `Profile B` | Calibrates external carrier preamp headroom |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Vectorized Biquad Bank:** 32 bandpass filters computed simultaneously using AVX2 / NEON 8-channel SIMD registers.
* **Denormal Immunity:** Envelope detector state values protected against underflow using hardware subnormal flushing.
