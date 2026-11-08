# Technical DSP Specification: `KlangShift` (Pitch, Granular & Heterodyne Lab)

**Engine ID:** `MEGA-06`  
**Sovereign Name:** `KlangShift`  
**Corporate Boring Name:** `Micro-Pitch, Granular Pitch Shifter & Frequency Shifter Workstation`  
**Category:** Flagship Studio Pitch & Spectral Workstation  
**APVTS Prefix:** `kshift_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section D](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Eventide H910 / H3000 Harmonizer, Moog / Harald Bode 1630 Frequency Shifter, Mutable Instruments Clouds granular pitch engine.
* **Upstream Offline Reference:** [moog_bode_1630_manual.md](file:///C:/Dev/Research/library/manuals/moog_bode_1630_manual.md) · [03_mutable_instruments_dsp_encyclopedia.md](file:///C:/Dev/Research/library/03_mutable_instruments_dsp_encyclopedia.md).
* **C++ Implementation Header:** [`KlangShiftWorkstation.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/KlangShiftWorkstation.h).

---

## 2. Mathematical Theory & DSP Architecture

KlangShift houses three distinct pitch manipulation philosophies:
1. **Granular Pitch Shifting (Micro-Pitch Harmonizer):**
   Splits audio into windowed grains ($20\,\text{ms} \dots 100\,\text{ms}$) and plays them back at variable playback speed ratios:
   $$R = 2^{\frac{\Delta \text{st}}{12}}$$
   Overlapping grains are crossfaded using cosine or Hann envelopes to prevent splice clicks.
2. **Weaver Single-Sideband (SSB) Frequency Shifting:**
   Linear frequency shifting via wideband Hilbert transform network:
   $$y(t) = x_I(t) \cos(\omega_s t) \mp x_Q(t) \sin(\omega_s t)$$
3. **Multi-Tap Shimmer Feedback Network:**
   Re-routes shifted grains through an internal delay buffer with damping filters to create infinite climbing or descending ambient clusters.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`PITCH / SHIFT`):** Semitone transposition ($\pm 24.0\,\text{semitones}$ or Carrier Hz, Profile B / C). *Push: Octave Snap (12st)*.
* **Knob 2 (`FINE DETUNE`):** Microtonal detune ($\pm 50.0\,\text{cents}$, Profile B). *Push: Zero Fine Detune*.
* **Knob 3 (`GRAIN SIZE`):** Granular window size ($10.0\,\text{ms} \dots 250.0\,\text{ms}$, Profile A). *Push: Snap 50ms*.
* **Knob 4 (`SCALE SNAP`):** Scale Quantizer (Off, Chromatic, Project Key, Phrygian, Phrygian Dominant, Harmonic Minor). Discrete Choice `0..5`. *Push: Cycle Scale*.
* **Knob 5 (`CARRIER WAVE`):** Sine, Tri, Saw, Square (for Ring Mod / Bode modes). Discrete Choice. *Push: Next Carrier*.
* **Knob 6 (`FEEDBACK / SHIMMER`):** Recirculating pitch-shift feedback ($0.0\% \dots 100.0\%$, Profile F). *Push: Feedback Invert*.
* **Knob 7 (`STEREO SPREAD`):** Left Up / Right Down micro-offset ($0.0\% \dots 100.0\%$, Profile B). *Push: Mono Matched*.
* **Knob 8 (`MIX`):** Universal Tri-Zone Dry/Wet Mix (Profile M: 7:00 Wet Audition, 12:00 Pure Dry, 5:00 Parallel). *Push: Phase Invert (\varnothing = 180^\circ)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Pitch Engine Quality):** `[ Dual-Tap Smooth Hann | Vintage 8-Bit H910 Glitch | Phase Vocoder Pristine ]`.
* **DIP 2 (Bode Shift Direction):** `[ Up Only | Down Only | Stereo Up L / Down R | Ring Mod ]`.
* **DIP 3 (Stereo Detune Spread):** `[ Centered Mono | Inverted Cents (L +cent / R -cent) | Wide Decorrelated ]`.
* **DIP 4 (Quantize Semitones):** `[ Free Continuous Floating | Quantize 12-TET Semitones | Microtonal 24-EDO ]`.
* **TRIM 1 (Frequency Shifter Slew):** Portamento slew time on Bode carrier transitions.
* **TRIM 2 (Feedback DC High-Pass):** Highpass filter preventing DC offset buildup in the feedback loop.
* **TRIM 3 (Grain Jitter Amount):** Random time jitter on grain start markers to avoid metallic flange artifacts.
* **TRIM 4 (Safety Limiter Threshold):** Peak limiter threshold in the recursive feedback loop.

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Active Heterodyne Engine | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`2: 4-Quadrant Ring Mod`** | `Knob 1, Knob 4` | `CARRIER HERTZ`, `RING SQUELCH` | `0.1Hz..5kHz`, `0..100%` | `Profile C`, `Profile B` | Audio-rate carrier oscillator & noise gate |
| **`3: Leslie 122 Rotary`** | `Knob 1, Knob 5, Knob 6` | `SPEED (SLOW/FAST)`, `TUBE DRIVE`, `ACCEL INERTIA` | `0.5..8Hz`, `0..24dB`, `0.5..8s`| `Profile C`, `Profile E`, `Profile A` | Rotating Doppler horn/drum & tube amp |
| **`4: dbx 120A Sub-Boom`** | `Knob 1, Knob 5, Knob 6` | `SUB HARMONIC (f/2)`, `SINE FOLD`, `LOW CORNER` | `0..100%`, `0..100%`, `30..120Hz`| `Profile B`, `Profile E`, `Profile D` | Subharmonic octave synthesis & wavefold |

--- | :--- | :--- | :--- | :--- | :--- |
| **`Bode Freq Shifter`**| `Knobs 1, 2` | `HERTZ SHIFT`, `FINE HZ` | `±5000 Hz`, `±20 Hz` | `Profile B`, `Profile B` | Direct linear Hertz frequency offset controls |
| **`Micro-Pitch Detune`**| `Knob 1` | `SPREAD CENTS`| `±100 cents` | `Profile B` | Symmetrical stereo widening detune |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Hann Crossfade Splicing:** Dual grain read heads crossfaded using look-up tables with exact unity-gain summation.
* **Denormal Immunity:** Feedback lines protected against subnormal floats.
