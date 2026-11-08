# Technical DSP Specification: `Vandal Park` (Steiner-Parker Diode Multimode Filter)

**Engine ID:** `CARD-02A` (Model 5) / `FX-06` (Model 5)  
**Sovereign Name:** `Vandal Park`  
**Corporate Boring Name:** `Multimode Diode-Ladder Resonant Filter`  
**Category:** Resonant Audio Filter  
**APVTS Prefix:** `vandal_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section E](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-e)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Steiner-Parker Synthacon (1975, designed by Niel Steiner), Arturia MiniBrute ("Brute Factor" feedback loop).
* **Upstream Offline Reference:** [arturia_pigments_manual.md](file:///C:/Dev/Research/library/manuals/arturia_pigments_manual.md) & [03_mutable_instruments_dsp_encyclopedia.md](file:///C:/Dev/Research/library/03_mutable_instruments_dsp_encyclopedia.md).
* **C++ Implementation Header:** [`VandalParkSVF.h`](file:///c:/Dev/TheKlangDSP/include/dsp/filters/VandalParkSVF.h).

---

## 2. Mathematical Theory & DSP Architecture

The Steiner-Parker Synthacon filter is unique among analog filters: instead of switching the output tap from a single filtered node, the inputs are injected directly into different nodes of a 2-pole diode-switched state variable topology (LP, BP, HP), while the diodes act as non-linear variable resistors controlled by control current $I_{ctrl}$.

### Diode Conduction Non-Linearity
The dynamic resistance of each diode in the feedback ladder is governed by the Shockley diode equation:
$$I_d = I_s \left( e^{\frac{V_d}{n V_t}} - 1 \right)$$
Under high resonance, the diode ladder clips symmetrically and compresses the resonance peak, creating screaming, unstable screeching without causing the catastrophic low-end bass drop characteristic of 4-pole transistor ladders.

### The "Vandal Factor" Feedback Loop
Knob 7 controls the **`VANDAL FACTOR`**: a direct post-filter audio feedback loop patched back into the diode input stage:
$$x_{in}[n] = x[n] + k_{vandal} \cdot \text{sat}\left(y_{lp}[n-1] \cdot \text{rect}(y_{bp}[n-1])\right)$$
At low settings ($0\% \dots 30\%$), it injects thick sub-harmonic warmth and overdrive. At high settings ($70\% \dots 100\%$), it forces chaotic intermodulation, diode tearing, and screaming self-oscillation.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`CUTOFF`):** $20\text{ Hz} \dots 20\text{ kHz}$ (Profile D Log). *Push: Toggle Hyper-Track (±400%)*.
* **Knob 2 (`RESONANCE`):** $0.5 \dots 25.0$ (Profile F Hyperbolic). *Push: Lock Self-Oscillation*.
* **Knob 3 (`MODEL`):** Set to `5: Vandal Park` in the 11-Model Palette. *Push: Next Category*.
* **Knob 4 (`SLOPE`):** 12 dB/oct (2-Pole native) or 24 dB/oct dual cascade. *Push: Toggle 12/24 dB*.
* **Knob 5 (`PRE-DRIVE`):** Input stage diode warmer ($0\text{ dB} \dots +24\text{ dB}$, Profile E). *Push: Bypass Drive*.
* **Knob 6 (`KEYTRACK`):** $-100\% \dots +100\%$ (Profile B). *Push: Snap 1V/Oct*.
* **Knob 7 (`VANDAL FACTOR`):** Non-linear diode feedback loop ($0\% \dots 100\%$, Profile E). *Push: Slam Max*.
* **Knob 8 (`STEREO SPREAD`):** Antiphase cutoff detune for dual stereo cards. Profile B. *Push: Mono Center*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Filter Routing):** `[ Lowpass | Bandpass | Highpass | Allpass Phase Notch ]`.
* **DIP 2 (Diode Model):** `[ 1N4148 Silicon Snarl | 1N34A Germanium Soft | 1N4001 Heavy Iron ]`.
* **DIP 3 (Bass Compensation):** `[ Authentic Bass Retention | Hyper-Bass Boost (+6dB) | Thin & Clean ]`.
* **DIP 4 (Limiter Mode):** `[ Hard Diode Clip | Soft Tanh Saturator | Clean ZDF Floating ]`.
* **TRIM 1 (Vandal Factor Slew):** Integrator smoothing on the feedback path ($0.1\text{ ms} \dots 10\text{ ms}$).
* **TRIM 2 (Resonance DC Bias):** Asymmetry trim on the self-oscillating sine wave.
* **TRIM 3 (Temperature Drift):** Simulates thermal drift in unbuffered diode junctions.
* **TRIM 4 (Output Makeup Gain):** Post-filter output compensation level ($-6\text{ dB} \dots +12\text{ dB}$).
