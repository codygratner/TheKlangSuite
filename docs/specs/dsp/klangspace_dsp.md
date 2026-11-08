# Technical DSP Specification: `KlangSpace` (Reverb & Acoustic Chamber Lab with Universal Gate)

**Engine ID:** `MEGA-08`  
**Sovereign Name:** `KlangSpace`  
**Corporate Boring Name:** `Multi-Algorithm Acoustic Chamber, Plate & Gated Reverb Workstation`  
**Category:** Flagship Studio Reverb Workstation  
**APVTS Prefix:** `kspace_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section D](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** EMT 140 Steel Plate, Lexicon 224 / 480L Concert Hall, AMS RMX16 Non-Linear & Gated Reverb, Ursa Major Space Station.
* **Upstream Offline Reference:** [emt_140_plate_manual.md](file:///C:/Dev/Research/library/manuals/emt_140_plate_manual.md) · [ROSETTA_STONE.md#section-d](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d).
* **C++ Implementation Header:** [`KlangSpaceWorkstation.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/KlangSpaceWorkstation.h).

---

## 2. Mathematical Theory & DSP Architecture

KlangSpace houses four canonical reverberation topologies paired with a universal dynamic gate:
1. **Four Specialized Reverb Algorithms:**
   - **Algorithm 1 (Vintage Steel Plate):** High-density modal dispersion mesh modeling physical sheet metal vibration.
   - **Algorithm 2 (Concert Hall):** 16-delay Feedback Delay Network (FDN) with randomized delay modulation for rich, lush orchestral decay.
   - **Algorithm 3 (Non-Linear Gated Reverb):** Truncated early reflection cluster modeling the legendary Phil Collins / 1980s drum gated sound with snappy linear decay and brickwall cutoff.
   - **Algorithm 4 (Ambient Shimmer):** Integrated pitch-shifted feedback loop producing celestial crystalline reverb tails.
2. **Universal Dynamic Noise Gate:**
   Integrated optical gate detector that triggers off input transients and violently chops off reverb tails at selectable hold times ($10\,\text{ms} \dots 500\,\text{ms}$).
3. **Orthogonal FDN Energy Conservation:**
   The FDN matrix uses a Householder or Hadamard transformation matrix to ensure lossless energy circulation before absorption filtering is applied:
   $$y = A \cdot x, \quad A^T A = I$$

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`DECAY TIME / T60`):** Reverb tail length ($0.2\,\text{s} \dots 30.0\,\text{s}$ + Infinite Freeze, Profile B). *Push: Infinite Freeze*.
* **Knob 2 (`PRE-DELAY`):** Initial reflection gap ($0.0\,\text{ms} \dots 300.0\,\text{ms}$, Profile C). *Push: Tempo Sync*.
* **Knob 3 (`REVERB ALGORITHM`):** Discrete choice: `EMT-140 Plate`, `Concert Hall`, `80s Non-Linear Gate`, `Shimmer Chamber`. Discrete Choice. *Push: Next Algo*.
* **Knob 4 (`SIZE / DIFFUSION`):** Room geometry and reflection density ($0.0\% \dots 100.0\%$, Profile B). *Push: Snap Max Size*.
* **Knob 5 (`DAMPING / TILT`):** High-frequency acoustic absorption ($1.0\,\text{kHz} \dots 20.0\,\text{kHz}$, Profile D). *Push: Reset Bright*.
* **Knob 6 (`GATE HOLD / DECAY`):** Dynamic gate hold time for 80s gated reverb ($10.0\,\text{ms} \dots 800.0\,\text{ms}$, Profile A). *Push: Toggle Gate On/Off*.
* **Knob 7 (`MODULATION DEPTH`):** Tank chorusing and acoustic air swirl ($0.0\% \dots 100.0\%$, Profile B). *Push: Mute Mod*.
* **Knob 8 (`MIX`):** Universal Tri-Zone Dry/Wet Mix (Profile M: 7:00 Wet Audition, 12:00 Pure Dry, 5:00 Parallel). *Push: Phase Invert ($\varnothing = 180^\circ$)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Gate Trigger Source):** `[ Internal Audio Envelope | External Sidechain Drum | Always Open ]`.
* **DIP 2 (Shimmer Pitch Interval):** `[ +12 Semitones (Octave Up) | +7 Semitones (Fifth) | -12 Semitones (Dark Sub) ]`.
* **DIP 3 (Stereo Width Mode):** `[ Wide Decorrelated Stereo | True Stereo L/R | Mono Summed ]`.
* **DIP 4 (Bass Damping Mode):** `[ Pre-Tank Bass Cut (150Hz) | Full Low-End Bloom | Resonant Sub Trap ]`.
* **TRIM 1 (Gate Fast Release Slew):** Brickwall slope cutoff speed of the dynamic gate ($1.0\,\text{ms} \dots 20.0\,\text{ms}$).
* **TRIM 2 (Early Reflections Balance):** Ratio between direct early reflections and diffuse late reverberation tail.
* **TRIM 3 (Modulation Rate):** Speed of the internal FDN delay modulation LFOs.
* **TRIM 4 (Safety Limiter Ceiling):** Feedback clamp ceiling preventing runaway resonance in Shimmer mode.

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Active Algo (Knob 3) | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`80s Non-Linear Gate`**| `Knobs 1, 6` | `GATE THRESH`, `GATE HOLD` | `-40..0 dB`, `10..500ms` | `Profile B`, `Profile A` | Direct controls for classic Phil Collins drum gate |
| **`Shimmer Chamber`** | `Knob 7` | `SHIMMER FEEDBACK`| `0..100%` | `Profile B` | Controls intensity of pitch-shifted shimmer wash |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Householder Matrix FDN:** 16-channel lossless unitary scattering matrix vectorized via SIMD fused multiply-add (FMA).
* **Zero Allocation Invariant:** All reverberation buffers and all-pass delay meshes pre-allocated during `prepareToPlay()`.
