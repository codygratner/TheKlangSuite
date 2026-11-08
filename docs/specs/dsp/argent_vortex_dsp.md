# Technical DSP Specification: `Argent Vortex` (Phase-Fold Resonator & Liquid Filter)

**Engine ID:** `FX-02`  
**Sovereign Name:** `Argent Vortex`  
**Corporate Boring Name:** `Dual Resonant Filterbank & Phase-Fold Resonator`  
**Category:** Liquid Multimode Filter & Resonance Shaper  
**APVTS Prefix:** `vortex_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section C](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Sherman Filterbank 2 (dual 4-pole multimode filters with aggressive FM and harmonics generator), Mutronics Mutator (envelope-following discrete analog stereo filter).
* **Upstream Offline Reference:** [sherman_filterbank_manual.md](file:///C:/Dev/Research/library/manuals/sherman_filterbank_manual.md) · [03_mutable_instruments_dsp_encyclopedia.md](file:///C:/Dev/Research/library/03_mutable_instruments_dsp_encyclopedia.md).
* **C++ Implementation Header:** [`ArgentVortexFilter.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/ArgentVortexFilter.h).

---

## 2. Mathematical Theory & DSP Architecture

Argent Vortex pairs two 4-pole Zero-Delay Feedback (ZDF) state-variable filters with an inter-filter phase-folding feedback network:
1. **Dual Resonant ZDF Core:**
   Filter 1 and Filter 2 operate in parallel or series:
   $$\dot{x}_1 = A_1 x_1 + B_1 u, \quad y_1 = C_1 x_1 + D_1 u$$
   $$\dot{x}_2 = A_2 x_2 + B_2 (y_1 \text{ or } u)$$
2. **Phase-Fold Resonant Feedback:**
   The output of Filter 2 is cross-modulated with the phase derivative of Filter 1 and passed through a non-linear sine wavefolder:
   $$v_{fb}[n] = \sin\left(\gamma \cdot y_2[n-1] + \beta \cdot \frac{d}{dt}y_1[n-1]\right)$$
   This produces wild vowel formants, liquid vocal squelches, and screaming harmonic rings.
3. **Dynamic Envelope & FM Tracking:**
   An internal envelope follower and transient detector dynamically sweep Filter 1 cutoff while Filter 2 tracks audio-rate frequency modulation (Filter FM).

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`CUTOFF 1`):** Filter 1 master cutoff ($20\,\text{Hz} \dots 20\,\text{kHz}$, Profile D Log). *Push: Octave Snap*.
* **Knob 2 (`RESONANCE 1`):** Filter 1 Q/damping ($0.5 \dots 30.0$, Profile F Hyperbolic). *Push: Self-Oscillation Lock*.
* **Knob 3 (`CUTOFF 2`):** Filter 2 secondary cutoff ($20\,\text{Hz} \dots 20\,\text{kHz}$, Profile D Log). *Push: Link to Cutoff 1*.
* **Knob 4 (`RESONANCE 2`):** Filter 2 Q/damping ($0.5 \dots 30.0$, Profile F Hyperbolic). *Push: Self-Oscillation Lock*.
* **Knob 5 (`PHASE FOLD`):** Sine fold drive in the inter-filter feedback loop ($0.0\% \dots 100.0\%$, Profile E). *Push: Max Fold*.
* **Knob 6 (`FILTER FM`):** Audio-rate FM depth from Filter 1 output to Filter 2 cutoff ($0.0\% \dots 100.0\%$, Profile E). *Push: Cross-FM Toggle*.
* **Knob 7 (`ENV MOD`):** Envelope follower depth to dual cutoffs ($-100.0\% \dots +100.0\%$, Profile B). *Push: Invert Polarity*.
* **Knob 8 (`MIX`):** Universal Tri-Zone Dry/Wet Mix (Profile M: 7:00 Wet Audition, 12:00 Pure Dry, 5:00 Parallel). *Push: Phase Invert ($\\varnothing = 180^\\circ$)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Filter Topology Routing):** `[ Series (F1 -> F2) | Parallel (F1 + F2) | Ping-Pong Stereo | Cross-Feedback Ring ]`.
* **DIP 2 (Filter 1 Pass Mode):** `[ Lowpass 24dB | Bandpass 12dB | Highpass 24dB | Notch ]`.
* **DIP 3 (Filter 2 Pass Mode):** `[ Lowpass 24dB | Bandpass 12dB | Highpass 24dB | Peaking ]`.
* **DIP 4 (Envelope Detector Mode):** `[ Fast Peak Transient | RMS Smooth | Slow Optical Lag ]`.
* **TRIM 1 (Envelope Attack Speed):** Detector rise time ($0.5\,\text{ms} \dots 50\,\text{ms}$).
* **TRIM 2 (Envelope Decay Speed):** Detector release time ($10\,\text{ms} \dots 2.0\,\text{s}$).
* **TRIM 3 (Saturation Drive Ceiling):** Soft-knee limiter ceiling preventing runaway feedback ($+3\,\text{dB} \dots +18\,\text{dB}$).
* **TRIM 4 (Stereo Separation):** Antiphase L/R cutoff spread ($0\,\text{st} \dots 12\,\text{st}$).

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Routing Mode (DIP 1) | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`Series (F1 -> F2)`** | `Knob 3` | `F2 SPREAD` | `±24 st` | `Profile B` | Sets relative offset of Filter 2 from Filter 1 |
| **`Parallel (F1 + F2)`** | `Knob 5` | `F1/F2 BALANCE` | `100:0..0:100` | `Profile B` | Blends between the two resonant peaks |
| **`Cross-Feedback`** | `Knob 6` | `CHAOS FEEDBACK`| `0..100%` | `Profile E` | Inter-filter chaotic non-linear ring driver |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Zero-Delay Feedback Solver:** Solved per sample using scalar Newton-Raphson iteration (max 3 iterations) with analytical Jacobian for unconditional stability.
* **Denormal & NaN Shield:** Hard clamp on filter state vectors to prevent IEEE floating-point denormals under decaying resonance.
