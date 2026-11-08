# Technical DSP Specification: `IronSum-4` (4-Channel Analog Summing Console)

**Engine ID:** `CARD-08`  
**Sovereign Name:** `IronSum-4`  
**Corporate Boring Name:** `4-Channel Analog Summing Console`  
**Category:** Analog Summing Bus & Spatial Mixer  
**APVTS Prefix:** `ironsum_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section D](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Rupert Neve 80-series Marinair transformer console, API 2520 discrete op-amp bus, vintage Telefunken tube console.
* **Upstream Offline Reference:** [ROSETTA_STONE_MASTER_IP_REFERENCE.md](file:///C:/Dev/Research/library/manuals/ROSETTA_STONE_MASTER_IP_REFERENCE.md).
* **C++ Implementation Header:** [`IronSum4.h`](file:///c:/Dev/TheKlangDSP/include/dsp/mixer/IronSum4.h).

---

## 2. Mathematical Theory & DSP Architecture

Unlike sterile digital addition ($y[n] = \sum x_i[n]$), `IronSum-4` reproduces the complex inter-channel loading, reactive transformer inductance, and non-linear saturation of vintage console summing networks.

### 4 Discrete Summing Bus Topologies
1. **Clean Active (Virtual Ground Op-Amp):** Low-noise, perfectly linear addition ($THD < 0.0001\%$), maximum dynamic headroom.
2. **Passive Resistor (Neve Marinair Transformer):** Channels pass through a passive resistor attenuation matrix into a high-ratio transformer. Heavy low-frequency saturation (2nd and 3rd harmonic bloom) and gentle inter-channel crosstalk.
3. **Tube Saturation (Telefunken V72):** Triode valve saturation curve with warm even-harmonic coloration:
   $$y[n] = \frac{x[n] + 0.2 x[n]^2}{1 + |x[n]|}$$
4. **Ring Mod Injection:** Injects cross-modulation between channel pairs:
   $$y_{ring}[n] = (Ch_1 \times Ch_2) + (Ch_3 \times Ch_4)$$

### Pan Law & Spatial Energy Preservation
Knobs 5..8 implement switchable pan laws:
* **$-3.0\text{ dB}$ Equal Power:** Constant acoustic energy across the stereo field ($L = \cos(\theta), R = \sin(\theta)$).
* **$-4.5\text{ dB}$ British Desk:** Subtle center dip for crowded mixes.
* **$-6.0\text{ dB}$ Linear Voltage:** Classic mono sum compatibility.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Page 1A (Channel Levels):**
  * `Knob 1 (CORE A LEVEL):` Level of Voice 1 / Core A ($0\% \dots 100\%$, Profile B). *Push: Mute/Solo*.
  * `Knob 2 (CORE B LEVEL):` Level of Voice 2 / Core B ($0\% \dots 100\%$, Profile B). *Push: Mute/Solo*.
  * `Knob 3 (NOISE LEVEL):` Level of Flint & Dust ($0\% \dots 100\%$, Profile B). *Push: Mute/Solo*.
  * `Knob 4 (SAMPLE LEVEL):` Level of ReFont Sample Player ($0\% \dots 100\%$, Profile B). *Push: Mute/Solo*.
* **Page 1B (Channel Pans):**
  * `Knob 5 (CORE A PAN):` Core A stereo position ($-100\% \dots +100\%$, Profile B). *Push: Center*.
  * `Knob 6 (CORE B PAN):` Core B stereo position ($-100\% \dots +100\%$, Profile B). *Push: Center*.
  * `Knob 7 (NOISE PAN):` Noise stereo position ($-100\% \dots +100\%$, Profile B). *Push: Center*.
  * `Knob 8 (SAMPLE PAN):` Sample stereo position ($-100\% \dots +100\%$, Profile B). *Push: Center*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Bus Topology):** `[ Active Clean | Passive Neve Resistor | Tube Saturation | Ring Mod Injection ]`.
* **DIP 2 (Pan Law):** `[ -3dB Equal Power | -4.5dB British Desk | -6dB Linear Voltage ]`.
* **DIP 3 (Phase Inversion):** `[ Normal Polarity | Core B Phase Invert 180° | Noise Invert 180° ]`.
* **DIP 4 (VCF Split Routing):** `[ All into VCF | Split: Core A/B to VCF, Noise/Sample direct to VCA ]`.
* **TRIM 1 (Transformer Iron Bias):** Saturation knee on the Marinair transformer stage.
* **TRIM 2 (Stereo Crosstalk Amount):** Inter-channel capacitive bleed ($0.0\% \dots 5.0\%$).
* **TRIM 3 (Master Bus Drive):** Saturation drive into the summing bus amplifier.
* **TRIM 4 (Headroom Ceiling):** True-peak output limiting ceiling ($-6\text{ dB} \dots 0\text{ dBFS}$).
