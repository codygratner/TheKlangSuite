# Technical DSP Specification: `KlangFilter` (Spectral Synthesis & Resonator Lab)

**Engine ID:** `MEGA-03`  
**Sovereign Name:** `KlangFilter`  
**Corporate Boring Name:** `Flagship Multi-Mode Spectral Resonator & Filter Workstation`  
**Category:** Flagship Studio Filter Workstation  
**APVTS Prefix:** `kfilt_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section D & E](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Sherman Filterbank 2, Mutronics Mutator, Moog 904A, Oberheim SEM, Korg MS-20, Roland TB-303, Mutable Instruments Rings modal filter.
* **Upstream Offline Reference:** [sherman_filterbank_manual.md](file:///C:/Dev/Research/library/manuals/sherman_filterbank_manual.md) · [ROSETTA_STONE.md#section-e](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-e).
* **C++ Implementation Header:** [`KlangFilterLab.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/KlangFilterLab.h).

---

## 2. Mathematical Theory & DSP Architecture

KlangFilter is the flagship dual-core spectral shaping engine:
1. **Dual Zero-Delay Feedback (ZDF) Filter Engines:**
   Houses two complete 4-pole ZDF multi-mode filter cores that can run in Series, Parallel, Split Stereo, or Cross-Modulating Feedback.
2. **Integrated Modulation Matrix:**
   Includes dedicated envelope followers, dual audio-rate LFOs, and filter-to-filter FM modulation:
   $$f_{c, 2}[n] = f_{c, 2, base} \cdot 2^{k_{fm} \cdot y_1[n]}$$
3. **Resonant Feedback Saturation:**
   Both filter feedback loops feature switchable non-linear diode, tube, or transistor saturators that maintain musicality under self-oscillation.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`CUTOFF 1`):** Primary filter cutoff ($10.0\,\text{Hz} \dots 22.0\,\text{kHz}$, Profile D Log). *Push: Hyper-Track Toggle (±400%)*.
* **Knob 2 (`RESONANCE 1`):** Filter 1 Q ($0.5 \dots 35.0$, Profile F Hyperbolic). *Push: Lock Self-Oscillation*.
* **Knob 3 (`CUTOFF 2`):** Secondary filter cutoff ($10.0\,\text{Hz} \dots 22.0\,\text{kHz}$, Profile D Log). *Push: Link Cutoff 1 & 2*.
* **Knob 4 (`RESONANCE 2`):** Filter 2 Q ($0.5 \dots 35.0$, Profile F Hyperbolic). *Push: Lock Self-Oscillation*.
* **Knob 5 (`ROUTING / MORPH`):** Series $\leftrightarrow$ Parallel $\leftrightarrow$ Ping-Pong continuous morph. Profile B. *Push: Toggle Series/Parallel*.
* **Knob 6 (`FILTER FM`):** Audio-rate FM from Filter 1 to Filter 2 ($0.0\% \dots 100.0\%$, Profile E). *Push: Cross-FM Toggle*.
* **Knob 7 (`DRIVE / WARMTH`):** Pre-filter non-linear saturation ($0.0\,\text{dB} \dots +30.0\,\text{dB}$, Profile E). *Push: Bypass Drive*.
* **Knob 8 (`MIX`):** Universal Tri-Zone Dry/Wet Mix (Profile M: 7:00 Wet Audition, 12:00 Pure Dry, 5:00 Parallel). *Push: Phase Invert ($\varnothing = 180^\circ$)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Filter 1 Model):** `[ Moog 24dB Ladder | TB-303 Diode | MS-20 K35 Sallen-Key | SEM 12dB SVF ]`.
* **DIP 2 (Filter 2 Model):** `[ SEM 12dB SVF | Vandal Park | Wasp CMOS | 3-Peak Formant ]`.
* **DIP 3 (LFO 1 Waveshape):** `[ Triangle | Sine | Random S&H | Stepped Pattern ]`.
* **DIP 4 (Bass Retention):** `[ Compensated Bass (No Drop) | Authentic Drop | Bass Boost (+6dB) ]`.
* **TRIM 1 (Envelope Follower Attack):** Rise time of the input envelope follower ($0.5\,\text{ms} \dots 50\,\text{ms}$).
* **TRIM 2 (Envelope Follower Release):** Decay time of the input envelope follower ($10\,\text{ms} \dots 1.5\,\text{s}$).
* **TRIM 3 (Stereo Cutoff Offset):** Fixed L/R stereo spread offset on both filters.
* **TRIM 4 (Resonance Ceiling):** Safety soft-saturation clamp on the feedback nodes.

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Filter Mode | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`Link Cutoff Mode`**| `Knob 3` | `CUTOFF OFFSET` | `±36 st` | `Profile B` | Secondary cutoff tracks primary with fixed pitch ratio |
| **`Formant Mode`** | `Knob 1, 3` | `VOWEL`, `GENDER` | `A-E-I-O-U`, `±12 st` | `Discrete`, `Profile B` | Morphs dual resonant peaks across vocal tract formants |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Newton-Raphson ZDF Solver:** Guaranteed quadratic convergence within 2 iterations for non-linear feedback loops.
* **Denormal & NaN Free:** State buffers clamped to prevent numerical underflow under zero-signal conditions.
