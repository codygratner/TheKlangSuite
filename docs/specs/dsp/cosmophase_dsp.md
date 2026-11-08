# Technical DSP Specification: `CosmoPhase` (Phase Distortion Synthesizer)

**Engine ID:** `OSC-03`  
**Sovereign Name:** `CosmoPhase`  
**Corporate Boring Name:** `Phase Distortion Synthesizer`  
**Category:** Digital Phase Distortion Oscillator Core  
**APVTS Prefix:** `cosmophase_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section B](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-b)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Casio CZ-101, CZ-1000, CZ-1 (Twin-Line Phase Distortion & Digital Controlled Wave DCW).
* **Upstream Offline Reference:** [04_tracker_pantheon_architecture_comparison.md](file:///C:/Dev/Research/library/04_tracker_pantheon_architecture_comparison.md).
* **C++ Implementation Header:** [`CosmoPhase.h`](file:///c:/Dev/TheKlangDSP/include/dsp/oscillators/CosmoPhase.h).

---

## 2. Mathematical Theory & DSP Architecture

Phase Distortion (PD) synthesizes rich harmonic timbres without analog filters by applying non-linear transfer functions to the phase angle $\phi \in [0, 1)$ of a cosine wave before lookup:
$$y[n] = \cos\left( 2\pi \cdot f_{PD}(\phi[n], \text{DCW}) \right)$$

### 8 Canonical Phase Distortion Waveforms
1. **Sawtooth:** Variable inflection knee $k = 0.5 \cdot (1 - \text{DCW})$:
$$f_{saw}(\phi) = \begin{cases} \frac{0.5}{k} \phi & 0 \le \phi < k \\ 0.5 + \frac{0.5}{1-k} (\phi - k) & k \le \phi < 1 \end{cases}$$
2. **Square:** Phase plateau clamp holding cosine at $-1$ for portion of cycle.
3. **Pulse:** Narrow phase compression spike.
4. **Double Sine:** $2\times$ frequency acceleration over single cycle.
5. **Saw-Pulse:** Hybrid asymmetric slope with square plateau.
6. **Resonant 1 (Saw Window):** Windowed resonant cosine ringing at integer harmonic multiples ($1\times \dots 16\times f_0$).
7. **Resonant 2 (Triangle Window):** Resonant cosine with smooth triangular envelope.
8. **Resonant 3 (Trapezoid Window):** Piercing simulated bandpass resonance.

### 7 Twin-Line Interaction Modes
`CosmoPhase` runs dual independent phase distortion lines (`Line 1` and `Line 2`):
`Line 1`, `Line 2`, `Mix (L1+L2)`, `Detune (L1+L2 with chorusing)`, `Ring Mod (L1 × L2)`, `Noise Mod (LFSR Phase Jitter)`, and `Double Ring (Mix + Ring)`.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`PITCH`):** $20\text{ Hz} \dots 4000\text{ Hz}$ (Profile D). *Push: Octave Transpose*.
* **Knob 2 (`WAVE 1`):** Line 1 Waveform (Saw, Square, Pulse, DoubleSine, Reso 1..3). *Push: Cycle Wave 1*.
* **Knob 3 (`WAVE 2`):** Line 2 Waveform. *Push: Cycle Wave 2*.
* **Knob 4 (`DCW 1`):** Line 1 Phase Distortion Depth ($0\% \dots 100\%$, Profile E). *Push: Max DCW*.
* **Knob 5 (`DCW 2`):** Line 2 Phase Distortion Depth ($0\% \dots 100\%$, Profile E). *Push: Max DCW*.
* **Knob 6 (`LINE MODE`):** 7 Interaction Modes (L1, L2, Mix, Detune, Ring, Noise, DoubleRing). *Push: Next Mode*.
* **Knob 7 (`LINE DETUNE`):** Bipolar pitch offset between lines ($-12\text{ st} \dots +12\text{ st}$, Profile B). *Push: Zero Detune*.
* **Knob 8 (`OUTPUT MIX`):** Line 1 / Line 2 balance (Profile M). *Push: Phase Invert 180°*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Phase Accumulator Resolution):** `[ 16-Bit Vintage Casio CZ | 32-Bit Floating Point Clean ]`.
* **DIP 2 (Ring Mod Method):** `[ Four-Quadrant Multiplier | Absolute Diode Rectified | Hard Logic XOR ]`.
* **DIP 3 (Resonant Sync Mode):** `[ Hard Window Reset 0° | Smooth Window Cosine Crossfade ]`.
* **DIP 4 (Envelope DCW Link):** `[ Linear DCW Scaling | Exponential Acoustic Bloom ]`.
* **TRIM 1 (Noise Mod Jitter Depth):** Phase noise injection intensity in Noise Mod mode.
* **TRIM 2 (Sub-Harmonic Resonant Bias):** Even/odd harmonic emphasis on resonant windows.
* **TRIM 3 (Line 2 Phase Offset):** Static phase offset between Line 1 and Line 2.
* **TRIM 4 (High-Frequency De-Harsh):** Gentle 1-pole lowpass filter for extreme DCW settings.
