# Technical DSP Specification: `Dimension BBD` (Analog Bucket-Brigade Chorus & Spatial Expander)

**Engine ID:** `FX-08`  
**Sovereign Name:** `Dimension BBD`  
**Corporate Boring Name:** `4-Stage Analog Bucket-Brigade Spatial Chorus`  
**Category:** Spatial Modulation & Chorus Processor  
**APVTS Prefix:** `dim_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section C](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Roland Dimension D SDD-320 (1979, 4 push-button spatial enhancer), Boss CE-1 Chorus Ensemble, Roland Juno-60 stereo BBD chorus.
* **Upstream Offline Reference:** [roland_dimension_d_sdd320_manual.md](file:///C:/Dev/Research/library/manuals/roland_dimension_d_sdd320_manual.md) · [alpha_juno_manual.md](file:///C:/Dev/Research/library/manuals/alpha_juno_manual.md).
* **C++ Implementation Header:** [`DimensionBBDChorus.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/DimensionBBDChorus.h).

---

## 2. Mathematical Theory & DSP Architecture

Dimension BBD models two dual-phase Bucket-Brigade Device delay lines driven in perfect antiphase:
1. **Antiphase Dual-LFO Modulation:**
   Two BBD delay lines (MN3007 emulation) are modulated by triangle LFOs locked at $180^\circ$ phase offset:
   $$t_{delay, L}(t) = t_0 + A \cdot \text{tri}(\omega t)$$
   $$t_{delay, R}(t) = t_0 + A \cdot \text{tri}(\omega t + \pi)$$
   Because the delay times sweep in opposite directions, the pitch-shift Doppler effects in the left and right channels cancel out when summed to mono, yielding wide spatial stereo expansion without sounding like a heavy warbly chorus.
2. **Analog Compander Pre/De-Emphasis:**
   BBD chips suffer from severe clock noise and limited SNR. The physical circuit uses an NE570 compander:
   - **Compressor:** $2:1$ pre-emphasis compression before entering the bucket brigade.
   - **Expander:** $1:2$ dynamic expansion combined with a 7th-pole Elliptic reconstruction filter ($f_c \approx 10\,\text{kHz}$) after the delay.
3. **The 4 Canonical Dimension Buttons:**
   - **Mode 1:** Gentle spatial widening, slow rate ($0.25\,\text{Hz}$), subtle depth.
   - **Mode 2:** Moderate depth, medium rate ($0.35\,\text{Hz}$).
   - **Mode 3:** Full studio dimension, deeper modulation ($0.5\,\text{Hz}$).
   - **Mode 4:** Aggressive wide chorus ($0.8\,\text{Hz}$, maximum BBD excursion).
   - **"All-Buttons-In" (Secret Mode 5):** Maximum spatial width, nonlinear clock sweep, saturated BBD rails.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`DIMENSION MODE`):** Discrete selection: `Mode 1`, `Mode 2`, `Mode 3`, `Mode 4`, `Mode 5 (All-In)`. Discrete Choice. *Push: Cycle Buttons (1, 2, 3, 4, 1+4)*.
* **Knob 2 (`RATE`):** Modulation clock rate ($0.05\,\text{Hz} \dots 10.0\,\text{Hz}$, Profile C). *Push: Tempo Sync*.
* **Knob 3 (`DEPTH`):** BBD delay sweep excursion ($0.5\,\text{ms} \dots 12.0\,\text{ms}$, Profile B). *Push: Zero Depth (Vibrato Bypass)*.
* **Knob 4 (`BBD COLOR`):** Post-filter BBD high-frequency roll-off ($2.0\,\text{kHz} \dots 16.0\,\text{kHz}$, Profile D Log). *Push: Juno-60 Warm vs Dimension Clean*.
* **Knob 5 (`FEEDBACK`):** Flanger-like chorus feedback ($\pm 80.0\%$, Profile F). *Push: Polarity Invert*.
* **Knob 6 (`OUTPUT LEVEL`):** Makeup gain ($\pm 12.0\,\text{dB}$, Profile B). *Push: Reset 0dB*.
* **Knob 7 (`STEREO SPREAD`):** Cross-coupled phase offset between Left/Right BBD chips ($0^\circ \dots 180^\circ$, Profile B). *Push: True 180° Dimension Split*.
* **Knob 8 (`MIX`):** Universal Tri-Zone Dry/Wet Mix (Profile M: 7:00 Wet Audition, 12:00 Pure Dry, 5:00 Parallel). *Push: Phase Invert (\varnothing = 180^\circ)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (BBD Stage Model):** `[ MN3007 1024-Stage | MN3009 256-Stage Ultra-Fast | MN3207 Low-Voltage | Modern Pristine ]`.
* **DIP 2 (LFO Waveshape):** `[ Authentic Linear Triangle | Dual-Sine Smoothed | Hyper-Exponential Warble ]`.
* **DIP 3 (Compander Emulation):** `[ NE570 Hardware Compander Active | Clean Linear Dynamic | Saturated Tape Style ]`.
* **DIP 4 (Bass Trapping):** `[ Low-End Mono Retain (<150Hz) | Full-Bandwidth Stereo Spread ]`.
* **TRIM 1 (BBD DC Bias):** Calibrates symmetrical clipping point of the BBD delay buckets.
* **TRIM 2 (Anti-Aliasing Filter Cutoff):** Precision tuning of the 7th-pole reconstruction lowpass filter.
* **TRIM 3 (Compander Attack/Release Time):** Dynamic response speed of the NE570 expansion cell.
* **TRIM 4 (Center Cancellation Depth):** Tunes mid-channel acoustic cancellation depth in stereo matrix.

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Mode Select (Knob 1) | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`Mode 5: All-In`** | `Knob 5, 7` | `RUNAWAY FEEDBACK`, `WARP SPREAD` | `0..100%`, `0..200%` | `Profile F`, `Profile B` | Drives BBD feedback and cross-coupled stereo matrix into spatial breakup |
| **`Manual Rate/Depth`** | `Knob 2` | `FREE RATE` | `0.02..10.0 Hz` | `Profile C` | Unlocks continuous variable rate beyond preset buttons |

--- | :--- | :--- | :--- | :--- | :--- |
| **`Mode 5: All-In`** | `Knob 3, 5` | `WARP SPREAD`, `RAIL SAT` | `0..100%`, `0..100%` | `Profile E`, `Profile E` | Drives BBD rails into non-linear spatial breakup |
| **`Manual Rate/Depth`** | `Knob 2` | `FREE RATE` | `0.02..10.0 Hz` | `Profile C` | Unlocks continuous variable rate beyond preset buttons |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Hermite Polynomial Delay Interpolation:** Fractional sample BBD delay lookups use 4-point, 3rd-order Hermite interpolation for artifact-free modulation without high-frequency loss.
* **Circular Ring Buffer Safety:** Fixed power-of-two buffer with bitwise masking (`idx & MASK`), eliminating branch conditions in audio thread.
