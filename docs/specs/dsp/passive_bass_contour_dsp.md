# Technical DSP Specification: `Passive Bass Contour` (Parametric & Air Band Equalizer)

**Engine ID:** `FX-07`  
**Sovereign Name:** `Passive Bass Contour`  
**Corporate Boring Name:** `Dual-Choke Passive Inductor Program Equalizer`  
**Category:** Equalization & Tone Sculpting Processor  
**APVTS Prefix:** `pbc_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section C](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Pulse Techniques Pultec EQP-1A (passive R-C-L filter network with push-pull vacuum tube makeup amp), Lang PEQ-2, Mäag Audio EQ4 (legendary 40kHz Air Band).
* **Upstream Offline Reference:** [pultec_eqp1a_manual.md](file:///C:/Dev/Research/library/manuals/pultec_eqp1a_manual.md) · [ROSETTA_STONE.md#section-c](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c).
* **C++ Implementation Header:** [`PassiveBassContourEQ.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/PassiveBassContourEQ.h).

---

## 2. Mathematical Theory & DSP Architecture

The Passive Bass Contour accurately models the passive inductor-capacitor-resistor (LCR) network and inter-band phase interactions of the Pultec EQP-1A:
1. **The Iconic "Pultec Low-End Trick" (Simultaneous Boost & Attenuate):**
   In the physical hardware, the Low Boost and Low Atten potentiometers tap different intermediate impedances of the LC circuit, meaning their shelving center frequencies and Q values are slightly offset:
   $$H_{low}(s) = \frac{s^2 + \frac{\omega_0}{Q_b} s + \omega_0^2}{s^2 + \frac{\omega_1}{Q_a} s + \omega_1^2}$$
   When both Boost and Attenuate are turned up simultaneously on $60\,\text{Hz}$, the circuit produces a massive, tight sub-bass boost combined with a musical scoop directly above it ($200\,\text{Hz} \dots 350\,\text{Hz}$), removing low-end boxiness while anchoring colossal bottom weight.
2. **Inductor Core Saturation & Hysteresis:**
   The toroid inductors saturate gradually at high signal levels, adding subtle 3rd-harmonic compression:
   $$B(t) = \mu_0 (H(t) + M(t))$$
3. **High-Frequency Air Band Shelf:**
   A dedicated transformer-coupled minimum-phase high shelf peaking smoothly above the audible band ($20\,\text{kHz} \dots 40\,\text{kHz}$) that injects breath and open dimension without harsh sibilance.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`LOW FREQ SELECT`):** Discrete bass frequency selection: `20Hz`, `30Hz`, `60Hz`, `100Hz`. Discrete Choice. *Push: Next Frequency*.
* **Knob 2 (`LOW BOOST`):** Low shelf boost amount ($0.0\,\text{dB} \dots +16.0\,\text{dB}$, Profile B). *Push: Reset Zero*.
* **Knob 3 (`LOW ATTEN`):** Low shelf attenuation amount ($0.0\,\text{dB} \dots -18.0\,\text{dB}$, Profile B). *Push: Reset Zero*.
* **Knob 4 (`HIGH FREQ SELECT`):** Peak frequency selection: `3kHz`, `4kHz`, `5kHz`, `8kHz`, `10kHz`, `12kHz`, `16kHz`. Discrete Choice. *Push: Next Frequency*.
* **Knob 5 (`HIGH BOOST`):** High peak boost amount ($0.0\,\text{dB} \dots +18.0\,\text{dB}$, Profile B). *Push: Reset Zero*.
* **Knob 6 (`HIGH BANDWIDTH / Q`):** Variable Q bandwidth for high boost ($0.5 \dots 3.0$, Profile B). *Push: Sharp vs Broad*.
* **Knob 7 (`HIGH AIR SHELF`):** 40kHz Air Band presence boost ($0.0\,\text{dB} \dots +12.0\,\text{dB}$, Profile B). *Push: Bypass Air Band*.
* **Knob 8 (`TUBE DRIVE / MAKEUP`):** Post-filter push-pull tube amplifier drive and makeup level ($0.0\,\text{dB} \dots +18.0\,\text{dB}$, Profile E). *Push: Clean Solid-State*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Inductor Model):** `[ Authentic Toroidal Iron Core | Precision Modern Air Core | Mu-Metal Shielded ]`.
* **DIP 2 (Tube Amplifier Topology):** `[ 12AX7 / 12AU7 Triode Pair | Discrete Class-A Transistor | Clean Transparent ]`.
* **DIP 3 (Mains Hum Bleed):** `[ 60Hz US Transformer Hum | 50Hz EU Transformer Hum | Pure Digital Silent ]`.
* **DIP 4 (Air Band Frequency):** `[ 20kHz Classic | 27kHz Open | 40kHz Maag Air ]`.
* **TRIM 1 (Inductor Core Non-Linearity):** Trims magnetic saturation depth under high low-frequency energy.
* **TRIM 2 (Transformer Saturation):** Injects low-frequency harmonic distortion from input/output transformers.
* **TRIM 3 (High-Cut Frequency):** Passive high-frequency attenuation shelf ($5\,\text{kHz}, 10\,\text{kHz}, 20\,\text{kHz}$).
* **TRIM 4 (High-Cut Amount):** Attenuation depth of the passive high shelf ($0.0\,\text{dB} \dots -16.0\,\text{dB}$).

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Mode | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`Pultec Trick Auto-Link`**| `Knob 2` | `PUNCH CONTOUR` | `0..100%` | `Profile B` | Automatically links Boost & Atten in classic 60Hz ratio |
| **`Air Band Solo`** | `Knob 7` | `AIR FREQ` | `20k..40k` | `Profile D` | Variable continuous sweep of ultrasonic shelf |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Minimum-Phase Biquad Cascade:** Filter coefficients derived directly from analog component nodal analysis, guaranteed minimum-phase with no pre-ringing.
* **Oversampling Free:** Non-linear tube harmonics calculated with high-efficiency Padé polynomial saturator requiring zero latency buffer delay.
