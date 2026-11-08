# Technical DSP Specification: `KlangEQ` (Studio, Console & DJ Performance Equalizer)

**Engine ID:** `MEGA-04`  
**Sovereign Name:** `KlangEQ`  
**Corporate Boring Name:** `Parametric, Console & 4-Band DJ Kill Equalizer Workstation`  
**Category:** Flagship Studio Equalization Workstation  
**APVTS Prefix:** `keq_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section D](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Neve 1073 (inductor mid-band + Baxandall shelves), SSL 4000 E "Black Knob" / "Brown Knob" parametric EQ, Pultec EQP-1A passive tube EQ, Allen & Heath Xone:92 (4-band DJ EQ with infinite kill switches).
* **Upstream Offline Reference:** [pultec_eqp1a_manual.md](file:///C:/Dev/Research/library/manuals/pultec_eqp1a_manual.md) · [ROSETTA_STONE.md#section-d](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d).
* **C++ Implementation Header:** [`KlangEQWorkstation.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/KlangEQWorkstation.h).

---

## 2. Mathematical Theory & DSP Architecture

KlangEQ unifies surgical parametric equalization with vintage musical console curves and performance DJ kills:
1. **Three Distinct Equalization Modes:**
   - **Mode A (Studio Parametric 4-Band):** Fully parametric Low, Low-Mid, High-Mid, and High bands with continuously variable Q ($0.3 \dots 12.0$).
   - **Mode B (Console Neve 1073 / SSL):** Proportional-Q curves where boost/cut depth automatically narrows bandwidth, avoiding muddy phase buildup.
   - **Mode C (DJ Performance 4-Band Kill):** Asymmetric curves providing $+6\,\text{dB}$ gentle boost clockwise, but plunging down to total silence ($-\infty\,\text{dB}$ infinite kill) counter-clockwise.
2. **De-Cramped Biquad Filters (Orfanidis Transform):**
   Utilizes the Orfanidis digital filter transform to prevent asymmetric frequency cramping near Nyquist, preserving analog-correct high-end response up to $22\,\text{kHz}$.
3. **Discrete Analog Saturation Stage:**
   Selectable output transformer and discrete Class-A operational amplifier models.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`LOW GAIN`):** Low shelf / bell gain ($-18.0\,\text{dB} \dots +18.0\,\text{dB}$, Profile B). *Push: Instant Low Kill ($-\infty\,\text{dB}$)*.
* **Knob 2 (`LOW-MID GAIN`):** Low-mid parametric band gain ($-18.0\,\text{dB} \dots +18.0\,\text{dB}$, Profile B). *Push: Instant Low-Mid Kill*.
* **Knob 3 (`HIGH-MID GAIN`):** High-mid parametric band gain ($-18.0\,\text{dB} \dots +18.0\,\text{dB}$, Profile B). *Push: Instant High-Mid Kill*.
* **Knob 4 (`HIGH GAIN`):** High shelf / bell gain ($-18.0\,\text{dB} \dots +18.0\,\text{dB}$, Profile B). *Push: Instant High Kill ($-\infty\,\text{dB}$)*.
* **Knob 5 (`EQ MODE SELECT`):** Discrete choice: `Studio Parametric`, `Neve 1073 Console`, `SSL 4000 E Black`, `Xone DJ 4-Band Kill`. Discrete Choice. *Push: Next Mode*.
* **Knob 6 (`HIGH-PASS FILTER`):** 18dB/oct steep low-cut filter ($10.0\,\text{Hz} \dots 400.0\,\text{Hz}$, Profile D). *Push: Bypass HPF*.
* **Knob 7 (`CONSOLE SATURATION`):** Neve/SSL transformer and discrete preamp warmth ($0.0\,\text{dB} \dots +18.0\,\text{dB}$, Profile E). *Push: Clean (0dB)*.
* **Knob 8 (`OUTPUT TRIM / MIX`):** Master output level ($-18.0\,\text{dB} \dots +18.0\,\text{dB}$, Profile B). *Push: Unity Gain (0dB)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Curve Topology):** `[ Proportional-Q Musical | Constant-Q Surgical | Asymmetric DJ Kill ]`.
* **DIP 2 (Low Band Shape):** `[ Low Shelf (Baxandall) | Low Bell Peaking | 4-Pole Sub Cut ]`.
* **DIP 3 (High Band Shape):** `[ High Shelf (Baxandall) | High Bell Peaking | 40kHz Air Band ]`.
* **DIP 4 (Mid/Side Mode):** `[ Stereo Linked | Mid Only | Side Only | M/S Split ]`.
* **TRIM 1 (Low Band Frequency):** Center frequency of Low band ($30\,\text{Hz} \dots 350\,\text{Hz}$).
* **TRIM 2 (Low-Mid Frequency):** Center frequency of Low-Mid band ($200\,\text{Hz} \dots 2.5\,\text{kHz}$).
* **TRIM 3 (High-Mid Frequency):** Center frequency of High-Mid band ($800\,\text{Hz} \dots 8.0\,\text{kHz}$).
* **TRIM 4 (High Band Frequency):** Center frequency of High band ($3.0\,\text{kHz} \dots 18.0\,\text{kHz}$).

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Active EQ Mode | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`DJ 4-Band Kill`** | `Knobs 1..4` | `BASS`, `L-MID`, `H-MID`, `TREBLE` | `-inf..+6 dB` | `Profile B` | Asymmetric DJ curve with instant kill push actions |
| **`Studio Parametric`**| `Knobs 6, 7`| `LM-Q`, `HM-Q` | `0.3..12.0` | `Profile B` | Direct bandwidth Q controls for parametric midrange |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Transposed Direct-Form II Biquads:** High numerical precision filter implementation minimizing coefficient roundoff noise.
* **Click-Free Parameter Smoothing:** Biquad coefficients recalculated using single-pole target interpolation at block boundaries.
