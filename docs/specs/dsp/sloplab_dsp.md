# Technical DSP Specification: `SlopLab` (8-Channel Analog Drift & Sag Matrix)

**Engine ID:** `CARD-06`  
**Sovereign Name:** `SlopLab`  
**Corporate Boring Name:** `8-Channel Analog Drift & Sag Matrix`  
**Category:** Expressive Modulation & Analog Drift Matrix  
**APVTS Prefix:** `sloplab_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section D](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Dave Smith Instruments / Sequential Prophet-6 & OB-6 "Slop", Roland Jupiter-8 VCO temperature walk.
* **Upstream Offline Reference:** [doepfer_a1491_source_of_uncertainty_manual.md](file:///C:/Dev/Research/library/manuals/doepfer_a1491_source_of_uncertainty_manual.md).
* **C++ Implementation Header:** [`SlopLab.h`](file:///c:/Dev/TheKlangDSP/include/dsp/modulation/SlopLab.h).

---

## 2. Mathematical Theory & DSP Architecture

`SlopLab` generates organic, microscopic analog instability by modeling three distinct electro-physical drift regimes across 8 independent modulation channels:

### 1. Thermal Brownian Random Walk
Each channel runs a 1D Brownian random walk with leaky integrator damping to prevent runaway DC offsets:
$$w_i[n] = \alpha \cdot w_i[n-1] + (1 - \alpha) \cdot \mathcal{N}(0, \sigma^2)$$
$$s_{thermal, i}[n] = \tanh(k_{drift} \cdot w_i[n])$$
where $\alpha \approx 0.9998$ controls the thermal inertia (speed of wandering).

### 2. Power Supply Rail Sag
Under heavy bass or loud chord strikes, real analog power supplies experience voltage droop due to finite transformer regulation. `SlopLab` taps the master sub-bass RMS level to dynamically sag circuit voltages:
$$V_{sag}[n] = k_{sag} \cdot \text{LPF}_{20\text{Hz}}(|x_{sub}[n]|^2)$$
This pulls pitch downwards, softens filter cutoffs, and expands VCA drive proportionally.

### 3. Polyphonic Cross-Coupling
Analog voices packed onto adjacent PCB traces exhibit weak capacitive and inductive coupling:
$$s_i[n] = (1 - \epsilon) s_{thermal, i}[n] + \frac{\epsilon}{2} \left( s_{thermal, i-1}[n] + s_{thermal, i+1}[n] \right)$$

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
8 assignable amount knobs routing slop to selected voice parameters:
* **Knobs 1..8 (`SLOT 1..8 AMT`):** Bipolar slop depth ($-100\% \dots +100\%$, Profile B).
  * **Push Action:** Cycles 3-way polarity:
    * `+/-` (Bipolar Brownian random walk)
    * `+` (Unipolar upward thermal expansion)
    * `-` (Unipolar voltage droop & sag)
  * **Shift + Turn:** Directly scrolls through the target destination parameter without diving into menus!

### Rear Deck ⚙️: 8 Sound Lab Controls
* **Top Deck: 8 Destination Pickers (`DEST 1..8`):** Assigns slots to Pitch, Cutoff, Resonance, VCA Level, Attack, Decay, FM Index, or Pan.
* **Bottom Deck (Physics & Topology):**
  * **DIP 1 (Drift Algorithm):** `[ Brownian Walk (Vintage DSI) | Perlin Smooth Noise | S&H Stepped Clock ]`.
  * **DIP 2 (Thermal Model):** `[ Discrete Transistor (Fast Jitter) | CEM3340 IC (Slow Wave) | Tube Chassis ]`.
  * **DIP 3 (Voice Coupling):** `[ Fully Independent Voices | Adjacent Card Crosstalk | Global Ground Loop ]`.
  * **DIP 4 (Bass Sag Link):** `[ Linked to Master Sub | Linked to Kick Transient | Internal LFO ]`.
  * **TRIM 1 (Drift Speed Rate):** Master thermal timescale ($0.01\text{ Hz} \dots 5.0\text{ Hz}$).
  * **TRIM 2 (Power Sag Depth):** Micro-volt voltage sag coefficient ($0\% \dots 100\%$).
  * **TRIM 3 (Jitter Micro-Burst):** High-frequency pink noise jitter injection.
  * **TRIM 4 (Voice Stereo Detune):** Static calibration offset between left and right channels.
