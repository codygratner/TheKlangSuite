# Technical DSP Specification: `BronzeLab` (Inharmonic Paiste Metallic Cluster)

**Engine ID:** `OSC-06`  
**Sovereign Name:** `BronzeLab`  
**Corporate Boring Name:** `Inharmonic Metallic Generator`  
**Category:** Metallic & Cymbal Synthesizer Core  
**APVTS Prefix:** `bronzelab_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section B](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-b)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Roland TR-808 cymbal cluster, Roland TR-606 metallic hat generator, Paiste 2002 / Formula 602 B20 bronze cymbal alloys.
* **Upstream Offline Reference:** [03_mutable_instruments_dsp_encyclopedia.md](file:///C:/Dev/Research/library/03_mutable_instruments_dsp_encyclopedia.md).
* **C++ Implementation Header:** [`BronzeLab.h`](file:///c:/Dev/TheKlangDSP/include/dsp/oscillators/BronzeLab.h).

---

## 2. Mathematical Theory & DSP Architecture

Real acoustic cymbals do not vibrate in harmonic series; they produce dense, non-linear, inharmonic 2D circular plate eigenmodes:
$$f_{m,n} = \frac{\alpha_{m,n}}{2\pi R} \sqrt{\frac{E}{\rho (1 - \nu^2)}}$$
`BronzeLab` synthesizes authentic cymbal strike and sizzle using a 4-layer cluster architecture:

### 1. Inharmonic 6-Square Oscillator Core
Modeled after the TR-808/606 Schmitt-trigger cluster:
$$f_i = f_0 \cdot \{ 1.0, 1.442, 1.838, 2.378, 2.822, 3.491 \}$$
Phase-modulated by an audio-rate metal mod source to dissolve distinct square tones into a dense bronze haze.

### 2. 8-Stage Allpass Phase Disperser
Passes the metallic cluster through an 8-stage cascaded allpass filter network ($H_{APF}(z) = \frac{a + z^{-1}}{1 + a z^{-1}}$) with non-linear delay taps, smearing transients into authentic acoustic wash.

### 3. TR-606 RC Discharge Choke
Models the analog capacitor discharge envelope of the TR-606 open/closed hat circuit, producing snappy decay curves that breathe naturally with re-triggers.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`CLUSTER TUNE`):** Base tuning of the metallic cluster ($200\text{ Hz} \dots 4000\text{ Hz}$, Profile D). *Push: Snap 808 Hat Pitch*.
* **Knob 2 (`INHARMONIC SPREAD`):** Cluster ratio detuning ($0\% \dots 200\%$, Profile B). *Push: Classic Paiste Ratios*.
* **Knob 3 (`DECAY`):** Cymbal decay time ($10\text{ ms} \dots 15.0\text{ s}$, Profile A). *Push: Choke / Mute*.
* **Knob 4 (`METALLIC RESONANCE`):** Q of the bandpass body filter ($0.5 \dots 15.0$, Profile F). *Push: High Q Whistle*.
* **Knob 5 (`ALLPASS SMEAR`):** Phase dispersion depth smearing square buzz into smooth bronze wash. Profile B. *Push: Pure Raw Cluster*.
* **Knob 6 (`TILT EQ`):** Sizzle tilt balance ($-6\text{ dB/oct}$ dark &rarr; $+6\text{ dB/oct}$ brilliant air, Profile B). *Push: Flat*.
* **Knob 7 (`TRANSIENT CRACK`):** Velocity-sensitive strike click level ($0\% \dots 100\%$, Profile B). *Push: Mute Crack*.
* **Knob 8 (`STEREO DISPERSION`):** Antiphase high-frequency stereo width. Profile B. *Push: Pure Mono Center*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Cluster Topology):** `[ 6-Square 808 Vintage | 4-Op FM Sizzle | 12-Band Modal Plate ]`.
* **DIP 2 (Envelope Response):** `[ TR-606 Analog RC Discharge | Linear Digital Ramp | Reverse Cymbal Swell ]`.
* **DIP 3 (Choke Mode):** `[ Open/Closed Hat Exclusive Choke | Free Polyphonic Ring ]`.
* **DIP 4 (Plate Alloy):** `[ B20 Bronze (Dark Ride) | B8 Bronze (Bright Crash) | Brass (Sizzly Sheet) ]`.
* **TRIM 1 (Schmitt-Trigger Noise Floor):** Vintage analog diode hiss in the oscillator core.
* **TRIM 2 (RC Choke Damping):** Discharge speed of closed hat mute.
* **TRIM 3 (High-Band Comb Filter Frequency):** High-frequency cymbal bell resonant node.
* **TRIM 4 (Drive Saturation):** Pre-output transformer saturation drive.
