# Technical DSP Specification: `FreakShifter` (Single-Sideband Frequency Shifter & Ring Modulator)

**Engine ID:** `FX-12`  
**Sovereign Name:** `FreakShifter`  
**Corporate Boring Name:** `Bode Single-Sideband Frequency Shifter & Ring Modulator`  
**Category:** Spectral & Heterodyne Pitch Shifter  
**APVTS Prefix:** `fsh_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section C](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Moog Model 1630 / Harald Bode Frequency Shifter (1973, single-sideband heterodyne multiplier), Roland VP-330 ring modulator.
* **Upstream Offline Reference:** [moog_bode_1630_manual.md](file:///C:/Dev/Research/library/manuals/moog_bode_1630_manual.md) · [ROSETTA_STONE.md#section-c](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c).
* **C++ Implementation Header:** [`BodeFreakShifter.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/BodeFreakShifter.h).

---

## 2. Mathematical Theory & DSP Architecture

Frequency shifting is radically different from pitch shifting: while pitch shifting multiplies all frequencies by a constant ratio (preserving harmonic intervals), frequency shifting adds a constant linear offset $\Delta f$ in Hertz to every harmonic, shattering harmonic relationships into eerie, metallic, sci-fi bell tones:
1. **The Weaver Single-Sideband (SSB) Method:**
   The incoming signal $x(t)$ is passed through a wideband Hilbert Transform filter network to split it into two quadrature signals ($90^\circ$ phase difference):
   $$x_I(t) = x(t), \quad x_Q(t) = \mathcal{H}\{x(t)\}$$
   Two quadrature sinusoidal carriers at shifting frequency $\omega_s = 2\pi \Delta f$ modulate the channels:
   $$y_{up}(t) = x_I(t) \cos(\omega_s t) - x_Q(t) \sin(\omega_s t) \quad (\text{Shift Up})$$
   $$y_{down}(t) = x_I(t) \cos(\omega_s t) + x_Q(t) \sin(\omega_s t) \quad (\text{Shift Down})$$
2. **Ring Modulation Hybrid:**
   When the quadrature cancellation term is bypassed, the system reverts to standard double-sideband four-quadrant analog ring modulation:
   $$y_{ring}(t) = x(t) \cdot \cos(\omega_s t)$$
3. **Feedback Tank & Spectral Detuning:**
   Re-injecting the frequency-shifted output back into the Hilbert input creates spiraling, unending ascending or descending acoustic chirps.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`SHIFT FREQ COARSE`):** Linear frequency offset ($-5000.0\,\text{Hz} \dots 0.0\,\text{Hz} \dots +5000.0\,\text{Hz}$, Profile B Bipolar). *Push: Zero Shift (Bypass)*.
* **Knob 2 (`SHIFT FREQ FINE`):** Micro-Hertz detune ($-20.0\,\text{Hz} \dots +20.0\,\text{Hz}$, Profile B). *Push: Zero Fine*.
* **Knob 3 (`SIDEBAND MODE`):** Discrete choice: `Up Shift Only`, `Down Shift Only`, `Dual Stereo Split (Up L / Down R)`, `Ring Modulator`. Discrete Choice. *Push: Next Mode*.
* **Knob 4 (`FEEDBACK`):** Regeneration into Hilbert shift network ($-95.0\% \dots +95.0\%$, Profile B). *Push: Invert Polarity*.
* **Knob 5 (`FEEDBACK DAMPING`):** Lowpass damping in feedback path ($1.0\,\text{kHz} \dots 20.0\,\text{kHz}$, Profile D). *Push: Full Bright*.
* **Knob 6 (`CARRIER SLEW / GLIDE`):** Portamento smoothing on shifting frequency changes ($0.0\,\text{ms} \dots 1000.0\,\text{ms}$, Profile A). *Push: Instant Step*.
* **Knob 7 (`DRIVE / SATURATION`):** Analog diode carrier leakage and input saturation ($0.0\,\text{dB} \dots +18.0\,\text{dB}$, Profile E). *Push: Pristine Clean*.
* **Knob 8 (`MIX`):** Universal Tri-Zone Dry/Wet Mix (Profile M: 7:00 Wet Audition, 12:00 Pure Dry, 5:00 Parallel). *Push: Phase Invert ($\varnothing = 180^\circ$)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Hilbert Transform Engine):** `[ 12-Pole IIR All-Pass Cascade | 128-Tap Linear-Phase FIR | 64-Tap Low-Latency ]`.
* **DIP 2 (Carrier Generator):** `[ Pure Sine Clean | Triangle Saturated | Squaring Diode Harmonic ]`.
* **DIP 3 (Negative Frequency Folding):** `[ Spectral Reflection (Bounce at 0Hz) | Pure Phase Inversion | Hard Zero Clamp ]`.
* **DIP 4 (Stereo Shifting Offset):** `[ Mono Center | Inverted Shift (L Up / R Down) | Quadrature Carrier ]`.
* **TRIM 1 (Carrier Bleed Balance):** Trims carrier suppression rejection ratio ($-60\,\text{dB} \dots -\infty\,\text{dB}$).
* **TRIM 2 (Feedback High-Pass Filter):** Prevents DC accumulation in the recursive feedback loop ($10\,\text{Hz} \dots 200\,\text{Hz}$).
* **TRIM 3 (Hilbert Phase Precision):** Fine tuning of quadrature phase error across $100\,\text{Hz} \dots 10\,\text{kHz}$.
* **TRIM 4 (Limiter Ceiling):** Feedback loop protection clamp threshold.

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Active Sideband Mode | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`Dual Stereo Split`** | `Knob 3` | `STEREO WIDTH` | `0..200%` | `Profile B` | Balances separation between Up and Down shifts |
| **`Ring Modulator`** | `Knob 1` | `CARRIER FREQ` | `20..10000 Hz` | `Profile D` | Absolute carrier frequency for classic ring mod |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **IIR Hilbert Phase Splitter:** Implemented using 6 dual-allpass pole pairs (Weaver architecture) with $<0.5^\circ$ phase error from $30\,\text{Hz}$ to $22\,\text{kHz}$.
* **Anti-Denormal Guard:** Recursive feedback loop protected by additive $10^{-25}$ float dither.
