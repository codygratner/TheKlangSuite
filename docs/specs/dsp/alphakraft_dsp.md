# Technical DSP Specification: `AlphaKraft` (Dual DCO & Unison Synthesizer)

**Engine ID:** `OSC-02`  
**Sovereign Name:** `AlphaKraft`  
**Corporate Boring Name:** `Dual DCO & Unison Synthesizer`  
**Category:** Subtractive & Unison Oscillator Core  
**APVTS Prefix:** `alphakraft_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section B](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-b)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Roland MKS-50 / Alpha Juno 1/2 DCO (IR3R05), Roland JP-8000 Supersaw.
* **Upstream Offline Reference:** [03_mutable_instruments_dsp_encyclopedia.md](file:///C:/Dev/Research/library/03_mutable_instruments_dsp_encyclopedia.md).
* **C++ Implementation Header:** [`AlphaKraft.h`](file:///c:/Dev/TheKlangDSP/include/dsp/oscillators/AlphaKraft.h).

---

## 2. Mathematical Theory & DSP Architecture

`AlphaKraft` combines the 14 band-limited waveforms of the Roland Alpha Juno DCO with a 7-voice detuned Supersaw array:

### 14 Alpha Juno DCO Waveforms (`PolyBLEP::alphaWave14`)
Unlike standard geometric oscillators, the Alpha Juno synthesized complex waveforms by XORing digital divider counters:
* Sawtooth, Comb Saw 1-2, Pulse, Narrow Pulse, Sub-divided pulse variants, and High-Frequency Octave stacks.
* Bandlimited anti-aliasing via Polynomial Band-Limited Step (PolyBLEP) residual correction:
$$R_{BLEP}(t) = \begin{cases} 2\left(\frac{t}{\Delta t}\right) - \left(\frac{t}{\Delta t}\right)^2 - 1 & 0 \le t < \Delta t \\ \left(\frac{t}{\Delta t}\right)^2 + 2\left(\frac{t}{\Delta t}\right) + 1 & -\Delta t < t < 0 \\ 0 & \text{otherwise} \end{cases}$$

### 7-Voice Hoover / Supersaw Stack
Spreads 6 satellite saw waves around a center frequency using prime-ratio detune offsets to eliminate cyclic phase cancellations:
$$y_{super}[n] = \frac{1}{\sqrt{7}} \left( \text{Saw}(\theta_0[n]) + \sum_{k=1}^{3} \left[ \text{Saw}(\theta_k^+[n]) + \text{Saw}(\theta_k^-[n]) \right] \right)$$
where detune ratios $\delta_k \in \{\pm 1.8\%, \pm 3.9\%, \pm 6.7\%\}$ scale dynamically with Knob 5 (`HOOVER DETUNE`).

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`PITCH`):** $20\text{ Hz} \dots 4000\text{ Hz}$ (Profile D Log). *Push: Octave Transpose*.
* **Knob 2 (`WAVE 14`):** Discrete 0..13 Alpha Juno DCO shapes. *Push: Next Wave*.
* **Knob 3 (`PWM / SPEED`):** Pulse width duty cycle ($0.08 \dots 0.92$) & LFO speed. Profile B. *Push: Manual vs LFO*.
* **Knob 4 (`SUB LEVEL`):** Sub-oscillator level ($0\% \dots 100\%$, Profile B). *Push: Toggle -1 / -2 Oct*.
* **Knob 5 (`HOOVER DETUNE`):** 7-voice Supersaw satellite detune spread. Profile B. *Push: Mono vs Wide*.
* **Knob 6 (`HPF / BRILLIANCE`):** 1-pole highpass shelf ($20\text{ Hz} \dots 2000\text{ Hz}$, Profile D). *Push: Flat 20Hz*.
* **Knob 7 (`CHORUS RATE`):** Integrated Roland BBD chorus rate & depth. Profile B. *Push: Chorus Off/I/II*.
* **Knob 8 (`OUTPUT LEVEL`):** Master output gain trim ($\pm 18\text{ dB}$, Profile B). *Push: Unity 0dB*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (DCO Clock Method):** `[ Clean PolyBLEP | 8-Bit Hardware Divider Emulation | Oversampled 4x ]`.
* **DIP 2 (Sub Oscillator Wave):** `[ Pure Square | Narrow Pulse | Asymmetric Staircase ]`.
* **DIP 3 (Chorus Topology):** `[ Juno-60 Mode I | Juno-60 Mode II | Mode I+II Noise Slam ]`.
* **DIP 4 (Stereo Spread Mode):** `[ Alternate Left/Right Satellites | Mid/Side Unlinked ]`.
* **TRIM 1 (BBD Bucket Loss):** High-frequency rolloff of internal chorus delay line.
* **TRIM 2 (Sub-Osc Phase Shift):** Phase alignment of sub-oscillator relative to fundamental.
* **TRIM 3 (Detune Skew):** Asymmetry between positive and negative detuned saw waves.
* **TRIM 4 (Anti-Click Slew):** Pitch modulation portamento smoothing.
