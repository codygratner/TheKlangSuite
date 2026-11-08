# Technical DSP Specification: `Vintage Steel Plate` (Physical EMT-140 Plate & Digital Hall Reverb)

**Engine ID:** `FX-15`  
**Sovereign Name:** `Vintage Steel Plate`  
**Corporate Boring Name:** `Physical EMT-140 Steel Plate & Digital Hall Reverb`  
**Category:** Reverb & Spatial Acoustic Environment  
**APVTS Prefix:** `plate_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section C](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Elektro-Mess-Technik EMT 140 (1957, 600-pound cold-rolled steel sheet with moving-coil transducer and piezo pickups), Lexicon 224 / 480L Concert Hall reverb.
* **Upstream Offline Reference:** [emt_140_plate_manual.md](file:///C:/Dev/Research/library/manuals/emt_140_plate_manual.md) · [ROSETTA_STONE.md#section-c](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c).
* **C++ Implementation Header:** [`VintageSteelPlateReverb.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/VintageSteelPlateReverb.h).

---

## 2. Mathematical Theory & DSP Architecture

The vintage steel plate is renowned for its instant, high-density modal dispersion and bright, shimmering diffusion:
1. **2D Thin-Plate Modal Wave Equation:**
   Vibrations in a tensioned rectangular plate ($2\,\text{m} \times 1\,\text{m}$) follow the Kirchhoff-Love biharmonic plate equation:
   $$\rho h \frac{\partial^2 w}{\partial t^2} + D \nabla^4 w + 2\rho h \gamma \frac{\partial w}{\partial t} = f(x, y, t)$$
   Where $D = \frac{E h^3}{12(1 - \nu^2)}$ is flexural rigidity. Because phase velocity is proportional to frequency ($v_p \propto \sqrt{\omega}$), high frequencies travel faster than low frequencies across the plate, creating the iconic "shimmering dispersion chirp" characteristic of real sheet metal.
2. **Mechanical Damper Pad Simulation:**
   In physical plates, a motorized porous damping pad moves closer to the steel sheet to adjust reverb decay ($T_{60}$ from $0.5\,\text{s}$ to $5.5\,\text{s}$). This is modeled via frequency-dependent damping filters in an interleaved waveguide mesh:
   $$\alpha(f) = \alpha_0 + \alpha_1 f + \alpha_2 f^2$$
3. **Dual Piezo Pickup Matrix:**
   Two pickup sensors positioned at asymmetric coordinates pick up the vibrating sheet, producing wide, uncorrelated decorrelated stereo signals.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`DECAY TIME / T60`):** Mechanical damper pad position ($0.5\,\text{s} \dots 8.0\,\text{s}$, Profile B). *Push: Infinite Freeze*.
* **Knob 2 (`PRE-DELAY`):** Acoustic initial reflection gap ($0.0\,\text{ms} \dots 250.0\,\text{ms}$, Profile C). *Push: Tempo Sync Toggle*.
* **Knob 3 (`PLATE DAMPING / TONE`):** High-frequency mechanical damping ($1.0\,\text{kHz} \dots 18.0\,\text{kHz}$, Profile D). *Push: Reset Bright*.
* **Knob 4 (`LOW BASS CUT`):** Pre-tank highpass filter to eliminate low-end mud ($20\,\text{Hz} \dots 500\,\text{Hz}$, Profile D). *Push: 100Hz Standard*.
* **Knob 5 (`DISPERSION / SHIMMER`):** Biharmonic wave dispersion amount ($0.0\% \dots 100.0\%$, Profile B). *Push: Max Metal Chirp*.
* **Knob 6 (`STEREO WIDTH`):** Piezo pickup stereo spread ($0.0\% \dots 200.0\%$, Profile B). *Push: Mono Center (0%)*.
* **Knob 7 (`INPUT TUBE DRIVE`):** EMT V84 tube drive preamp ($0.0\,\text{dB} \dots +18.0\,\text{dB}$, Profile E). *Push: Clean (0dB)*.
* **Knob 8 (`MIX`):** Universal Tri-Zone Dry/Wet Mix (Profile M: 7:00 Wet Audition, 12:00 Pure Dry, 5:00 Parallel). *Push: Phase Invert ($\varnothing = 180^\circ$)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Plate Material / Geometry):** `[ Cold-Rolled Steel EMT-140 | Modern Gold Foil | Heavy Lead Sheet | Lexicon Digital Hall ]`.
* **DIP 2 (Pickup Electronics):** `[ Vintage EMT V84 Tube Preamp | Solid-State Discrete 162 | Clean Pristine ]`.
* **DIP 3 (Modal Density):** `[ Ultra-Dense 64-Delay Matrix | Classic 32-Delay FDN | 16-Delay Vintage Grain ]`.
* **DIP 4 (Input Diffusion Stages):** `[ 4 Cascaded All-Pass Diffusers | 2 Diffusers Soft | Zero Direct Strike ]`.
* **TRIM 1 (Transducer Non-Linearity):** Magnetic saturation of the moving-coil input driver.
* **TRIM 2 (Piezo Resonance Peak):** Mechanical resonance frequency of the contact pickups ($3.5\,\text{kHz} \dots 6.0\,\text{kHz}$).
* **TRIM 3 (High-Shelf Air):** Ultrasonic air boost above $12\,\text{kHz}$.
* **TRIM 4 (Noise Floor Floor):** Vintage thermal preamp hiss and acoustic room rumble ($-\infty \dots -50\,\text{dB}$).

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Material Mode (DIP 1) | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`Gold Foil Plate`** | `Knob 5` | `FOIL SHINE` | `0..100%` | `Profile B` | Brighter modal dispersion with airy high decay |
| **`Digital Hall`** | `Knob 5` | `DIFFUSION` | `0..100%` | `Profile B` | Modulates digital delay line chorusing and diffusion |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Feedback Delay Network (FDN):** 16-channel orthogonal feedback delay network using Householder reflection matrix for perfect energy preservation.
* **Zero Latency Dry Path:** Pure parallel dry path with zero samples of latency; wet path uses pre-delay buffer.
