# Technical DSP Specification: `Pluck` (Physical Waveguide & Modal Resonator)

**Engine ID:** `OSC-04`  
**Sovereign Name:** `Pluck`  
**Corporate Boring Name:** `Physical Waveguide & Modal Resonator`  
**Category:** Physical Modeling Acoustic Synthesizer  
**APVTS Prefix:** `pluck_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section B](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-b)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Karplus-Strong string synthesis, Mutable Instruments Rings / Elements (Émilie Gillet modal synthesis).
* **Upstream Offline Reference:** [03_mutable_instruments_dsp_encyclopedia.md](file:///C:/Dev/Research/library/03_mutable_instruments_dsp_encyclopedia.md).
* **C++ Implementation Header:** [`PluckResonator.h`](file:///c:/Dev/TheKlangDSP/include/dsp/oscillators/PluckResonator.h).

---

## 2. Mathematical Theory & DSP Architecture

`Pluck` merges a recirculating Karplus-Strong fractional delay line with a bank of 24 parallel 2-pole bandpass modal resonators:

### 1. Karplus-Strong Fractional Waveguide
A delay line of length $N = \frac{f_s}{f_0} - \Delta$ recirculates through an internal lowpass damping filter $H_{damp}(z) = \frac{1 + z^{-1}}{2}$ and fractional Hermite/allpass interpolator:
$$y[n] = x_{strike}[n] + R \cdot H_{damp}(y[n-N])$$
where $R \in [0.95, 0.9999]$ controls string decay time, and non-linear string dispersion models inharmonic stiffness:
$$f_k = k f_0 \sqrt{1 + B k^2}$$

### 2. 24-Mode Resonant Modal Bank
For struck bars, membranes, glass bowls, and bells:
$$y_{modal}[n] = \sum_{m=1}^{24} A_m \cdot \text{BPF}\left(f_0 \cdot r_m, Q_m\right) * x_{strike}[n]$$
where mode ratios $r_m$ match physical materials:
* **String:** Integer harmonics ($1, 2, 3, 4, \dots$).
* **Bar / Marimba:** Clamped bar ratios ($1, 2.76, 5.40, 8.93, \dots$).
* **Membrane / Drumhead:** Bessel zeros ($1, 1.59, 2.14, 2.30, \dots$).
* **Plate / Gong:** 2D non-linear modal cluster.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`PITCH`):** Chromatic frequency ($20\text{ Hz} \dots 2000\text{ Hz}$, Profile D). *Push: Octave Snap*.
* **Knob 2 (`STRUCTURE`):** Harmonic vs inharmonic material (String &rarr; Bar &rarr; Plate &rarr; Bell, Profile B). *Push: Snap Pure String*.
* **Knob 3 (`BRIGHTNESS`):** High-frequency damping & string loss ($500\text{ Hz} \dots 20\text{ kHz}$, Profile D). *Push: Open Bright*.
* **Knob 4 (`DAMPING / DECAY`):** Acoustic ring time ($50\text{ ms} \dots 20.0\text{ s}$, Profile A). *Push: Infinite Ring Hold*.
* **Knob 5 (`POSITION / STRIKE`):** Excitation strike point along the string ($0.0 \dots 0.5$, Profile B). *Push: Center Node (0.5)*.
* **Knob 6 (`STRIKE MODEL`):** Excitation impulse (Plectrum, Felt Mallet, Bow Friction, Metallic Clack). *Push: Cycle Strike*.
* **Knob 7 (`DISPERSION / INHARM`):** Non-linear wire stiffness and pitch glide on strike. Profile E. *Push: Zero Stiffness*.
* **Knob 8 (`POLYPHONY / SPACE`):** Monophonic vs 4-voice sympathetic string resonance spread. Profile B. *Push: Pure Mono*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Resonator Mode):** `[ Karplus Waveguide | Modal Resonator Bank | Sympathetic String Array ]`.
* **DIP 2 (Non-Linearity):** `[ Pure Linear | String Collision (Fret Buzz) | Bowed Chaos Stick-Slip ]`.
* **DIP 3 (Fractional Interpolation):** `[ Hermite 4-Point | 1st-Order Allpass | Linear ]`.
* **DIP 4 (External Audio Input):** `[ Internal Exciter Only | Process External Audio Key ]`.
* **TRIM 1 (Sympathetic Coupling Factor):** Energy exchange between sympathetic strings.
* **TRIM 2 (Fret Buzz Threshold):** String displacement limit for fret collision buzzing.
* **TRIM 3 (Body Resonance Frequency):** Acoustic instrument soundboard formant peak ($180\text{ Hz} \dots 400\text{ Hz}$).
* **TRIM 4 (Air Damping Loss):** High-frequency room radiation decay speed.
