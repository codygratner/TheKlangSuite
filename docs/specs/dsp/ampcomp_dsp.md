# Technical DSP Specification: `AmpComp` (Master VCA & Dynamics Processor)

**Engine ID:** `CARD-03A`  
**Sovereign Name:** `AmpComp`  
**Corporate Boring Name:** `Master Output VCA, Compressor & Stereo Expander`  
**Category:** Output Dynamics & Level Processing  
**APVTS Prefix:** `acomp_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section D](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** SSL 4000 G Master Bus Compressor, UREI 1176 Peak Limiter, Teletronix LA-2A optical leveling amplifier, Neve discrete transformer output stage.
* **Upstream Offline Reference:** [ssl_g_bus_manual.md](file:///C:/Dev/Research/library/manuals/ssl_g_bus_manual.md) · [ROSETTA_STONE.md#section-d](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d).
* **C++ Implementation Header:** [`AmpCompMasterDynamics.h`](file:///c:/Dev/TheKlangDSP/include/dsp/dynamics/AmpCompMasterDynamics.h).

---

## 2. Mathematical Theory & DSP Architecture

AmpComp combines a precision analog-modeled Voltage Controlled Amplifier (VCA) with multi-topology bus dynamics and mid/side stereo spatial control:
1. **Analog VCA Circuitry & Discrete Drive:**
   Models a dB-linear Blackmer VCA cell with variable Class-A discrete transformer saturation:
   $$v_{out} = v_{in} \cdot 10^{\frac{G_{dB} + \text{Drive}}{20}} \cdot \tanh\left(1 + \frac{v_{in}}{V_{sat}}\right)$$
2. **Four Compressor Circuit Topologies:**
   - **SSL Bus:** Fast VCA compression with discrete $2:1, 4:1, 10:1$ ratios and program-dependent auto-release.
   - **1176 FET:** Ultra-fast FET gain reduction ($20\,\mu\text{s} \dots 800\,\mu\text{s}$) with aggressive 2nd-order harmonic bite.
   - **LA-2A Opto:** Electro-luminescent optical panel with dual-stage memory release (fast $50\%$ drop, sluggish several-second tail).
   - **Nuke Brickwall:** Distressor-style infinite limiter for violent room crushing.
3. **Mid/Side Spatial Width Spreader:**
   Phase-coherent mid/side matrix providing $0.0\% \dots 200.0\%$ variable stereo widening with mono bass anchoring.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`MASTER LEVEL`):** Output linear gain ($-60.0\,\text{dB} \dots +12.0\,\text{dB}$, Profile B). *Push: Mute / Unmute*.
* **Knob 2 (`PAN`):** Stereo spatial pan balance ($-100.0\% \dots 0.0\% \dots +100.0\%$, Profile B). *Push: Center (0%)*.
* **Knob 3 (`VCA DRIVE`):** Discrete output transformer warmth ($0.0\,\text{dB} \dots +24.0\,\text{dB}$, Profile E). *Push: Clean (0dB)*.
* **Knob 4 (`STEREO WIDTH`):** Mid/Side stereo expansion ($0.0\% \dots 200.0\%$, Profile B). *Push: Mono Check (0%)*.
* **Knob 5 (`COMP THRESHOLD`):** Dynamic compression engagement floor ($-40.0\,\text{dB} \dots 0.0\,\text{dB}$, Profile B). *Push: Auto-Set*.
* **Knob 6 (`COMP RATIO`):** Stepped ratio: `1.5:1`, `2:1`, `4:1`, `8:1`, `20:1`, `NUKE`. Discrete Choice. *Push: Instant NUKE*.
* **Knob 7 (`PUNCH MACRO`):** Interlocked attack/release punch contour ($0.0\% \dots 100.0\%$, Profile B). *Push: Default Glue*.
* **Knob 8 (`CEILING / MAKEUP`):** Makeup gain and safety ceiling limiter ($-12.0\,\text{dB} \dots +18.0\,\text{dB}$, Profile B). *Push: Auto-Makeup Link*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Compressor Topology):** `[ SSL 4000 G Bus | UREI 1176 FET | LA-2A Optical | Distressor Nuke ]`.
* **DIP 2 (Sidechain Filter Mode):** `[ 120Hz Bus HPF | 80Hz Tight | Full Bandwidth ]`.
* **DIP 3 (Saturation Topology):** `[ Neve St. Ives Transformer | Clean Discrete VCA | Tube Triode ]`.
* **DIP 4 (Parallel NY Mix Mode):** `[ 100% In-Line Dynamic | 50/50 New York Parallel Blend ]`.
* **TRIM 1 (Compressor Attack Time):** Precision manual attack time trim ($0.1\,\text{ms} \dots 50.0\,\text{ms}$).
* **TRIM 2 (Compressor Release Time):** Precision manual release time trim ($20.0\,\text{ms} \dots 1500.0\,\text{ms}$).
* **TRIM 3 (Soft-Knee Width):** Smooths threshold transition ($0.0\,\text{dB} \dots 12.0\,\text{dB}$).
* **TRIM 4 (True-Peak Safety Margin):** Master brickwall limiter margin ($-0.1\,\text{dBFS} \dots -1.0\,\text{dBFS}$).

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Topology (DIP 1) | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`1176 FET Mode`** | `Knobs 5, 7` | `INPUT GAIN`, `ALL-BUTTONS` | `0..+36 dB`, `Toggle` | `Profile E`, `Discrete` | Classic 1176 input-driven limiting and all-buttons squash |
| **`LA-2A Opto`** | `Knob 5, 6` | `PEAK REDUCTION`, `LIMIT/COMP` | `0..100%`, `Toggle` | `Profile B`, `Discrete` | Canonical two-knob optical leveling amplifier interface |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Zero Delay Feed-Forward VCA:** Detection and gain smoothing executed per block; gain applied via SIMD vector multiply.
* **Denormal Immunity:** Ballistic filters use subnormal float clamping.
