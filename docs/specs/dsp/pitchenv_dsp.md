# Technical DSP Specification: `PitchEnv` (Universal 4-Mode Pitch Envelope)

**Engine ID:** `CARD-04`  
**Sovereign Name:** `PitchEnv`  
**Corporate Boring Name:** `Universal 4-Mode Pitch Envelope & Transient Modulator`  
**Category:** Expressive Pitch & Frequency Articulation Generator  
**APVTS Prefix:** `penv_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section D](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Minimoog Model D pitch bend circuitry, Oberheim Matrix-12 multi-stage pitch envelope, Simmons SDS-V drum synthesizer pitch drop, Dirtywave M8 pitch envelopes.
* **Upstream Offline Reference:** [m8_shortcut_guide.md](file:///C:/Dev/Research/library/manuals/m8_shortcut_guide.md) · [01_renoise_complete_offline_guide.md](file:///C:/Dev/Research/library/01_renoise_complete_offline_guide.md).
* **C++ Implementation Header:** [`PitchEnvelopeGenerator.h`](file:///c:/Dev/TheKlangDSP/include/dsp/envelopes/PitchEnvelopeGenerator.h).

---

## 2. Mathematical Theory & DSP Architecture

PitchEnv shapes pitch sweeps, laser zap transients, 808 kick drops, and subtle acoustic bloom:
1. **Four Specialized Pitch Topologies:**
   - **Mode 1 (Drum Pitch Drop):** Fast exponential pitch dive ($+48\,\text{st} \to 0\,\text{st}$ in $5\,\text{ms} \dots 150\,\text{ms}$) designed for colossal kick punch and Simmons toms:
     $$f(t) = f_{base} \cdot 2^{\frac{\Delta \text{st} \cdot e^{-t/\tau_{drop}}}{12}}$$
   - **Mode 2 (Analog AHDSR):** Full musical pitch envelope with attack portamento, peak hold, decay, sustain offset, and release glide.
   - **Mode 3 (Laser Zap / Chirp):** High-speed linear or quadratic frequency chirps for sci-fi FX and EDM transient snaps.
   - **Mode 4 (Acoustic Bloom):** Delayed subtle pitch drift that models horn and acoustic instrument onset warming.
2. **Dual Carrier/Modulator Targeting:**
   Routes to **Carrier Pitch**, **Modulator Pitch (FM Ratio)**, **Both**, or **Opposite (Inharmonic Divergence)**.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`ENV TYPE`):** Discrete choice: `Drum Pitch Drop`, `Analog AHDSR`, `Laser Zap / Chirp`, `Acoustic Bloom`. Discrete Choice. *Push: Next Type*.
* **Knob 2 (`MOD TARGET`):** Routing target: `Carrier Pitch Only`, `Modulator FM Pitch`, `Both (Co-Pitch)`, `Opposite (FM Ratio Bloom)`. Discrete Choice. *Push: Cycle Target*.
* **Knob 3 (`ATTACK / GLIDE`):** Rise time or portamento glide ($0.0\,\text{ms} \dots 5.0\,\text{s}$, Profile A Log). *Push: Instant Snap (0ms)*.
* **Knob 4 (`HOLD / PUNCH`):** Peak pitch hold plateau before sweep begins ($0.0\,\text{ms} \dots 200.0\,\text{ms}$, Profile A). *Push: Reset Zero*.
* **Knob 5 (`DECAY / DROP TIME`):** Pitch drop recovery time ($2.0\,\text{ms} \dots 5.0\,\text{s}$, Profile A). *Push: 20ms Kick Punch*.
* **Knob 6 (`SUSTAIN OFFSET`):** Resting pitch transposition offset ($-24.0\,\text{st} \dots +24.0\,\text{st}$, Profile B). *Push: Reset Neutral (0)*.
* **Knob 7 (`RELEASE GLIDE`):** Pitch sweep after key release ($1.0\,\text{ms} \dots 5.0\,\text{s}$, Profile A). *Push: Snap to Decay*.
* **Knob 8 (`BIPOLAR DEPTH`):** Master sweep extent in semitones ($-48.0\,\text{st} \dots +48.0\,\text{st}$, Profile B Bipolar). *Push: Invert Polarity ($\pm 1.0$)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Quantize Mode):** `[ Smooth Continuous Microtonal | Quantize 12-TET Semitones | Quantize Octaves Only ]`.
* **DIP 2 (Retrigger Behavior):** `[ Retrig from Peak (+Depth) | Legato Catch from Current Pitch ]`.
* **DIP 3 (Keytrack Scale):** `[ Constant Pitch Sweep All Keys | Track 1V/Oct (High Notes Drop Faster) ]`.
* **DIP 4 (Pitch Drop Shape):** `[ Pure Exponential Dive | Hyperbolic Punch | Linear Sweep ]`.
* **TRIM 1 (1V/Oct Keytrack Scaling):** Fine tuning of pitch envelope time keyboard tracking.
* **TRIM 2 (Microtonal Cent Detune):** Micro-cent detune trim on sustain resting point.
* **TRIM 3 (High-Frequency Pitch Floor):** Safety ceiling preventing pitch sweeps from exceeding Nyquist ($0.45 \cdot f_s$).
* **TRIM 4 (Velocity Sensitivity):** Dynamic scaling of pitch sweep extent to key velocity.

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Active Type (Knob 1) | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`Drum Pitch Drop`** | `Knobs 3, 5` | `CLICK SPEED`, `BASS DROP` | `0.1..20ms`, `5..500ms` | `Profile A`, `Profile A` | Direct controls for electronic kick drum transient drops |
| **`Acoustic Bloom`** | `Knobs 3, 8` | `BLOOM ONSET`, `BLOOM CENTS` | `10ms..2s`, `±50 cents` | `Profile A`, `Profile B` | Microtonal pitch swell for expressive acoustic strings |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Log-Domain Pitch Calculation:** Pitch calculations evaluated as additive semitone values, transformed to frequency ratios via SIMD vectorized $2^{x/12}$ approximations.
* **Phase-Accumulator Continuity:** Oscillator phase accumulators maintain continuous phase across violent pitch sweeps with zero waveform tears.
