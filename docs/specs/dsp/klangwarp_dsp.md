# Technical DSP Specification: `KlangWarp` (Studio Saturation, Wavefolder & Clipper Lab)

**Engine ID:** `MEGA-01`  
**Sovereign Name:** `KlangWarp`  
**Corporate Boring Name:** `Multi-Stage Harmonic Saturation, Wavefolder & Clipper Workstation`  
**Category:** Flagship Studio Saturation Workstation  
**APVTS Prefix:** `kwarp_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section D](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Thermionic Culture Culture Vulture (triode/pentode distortion), Neve 1073 transformer saturation, Buchla 259 non-linear wavefolder, Elysia Karacter discrete Class-A saturator.
* **Upstream Offline Reference:** [arturia_pigments_manual.md](file:///C:/Dev/Research/library/manuals/arturia_pigments_manual.md) · [ROSETTA_STONE.md#section-d](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d).
* **C++ Implementation Header:** [`KlangWarpSaturator.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/KlangWarpSaturator.h).

---

## 2. Mathematical Theory & DSP Architecture

KlangWarp provides 6 distinct saturation and waveshaping engines under a unified 16-control architecture:
1. **Multi-Model Non-Linear Transfer Functions:**
   - **Engine 1: Triode Tube:** Smooth even harmonics via quadratic Taylor series: $y = x - 0.2 x^2 + 0.05 x^3$.
   - **Engine 2: Pentode Tube:** Aggressive odd harmonics and high-level compression: $y = \frac{x}{\sqrt{1 + x^2}}$.
   - **Engine 3: Tape Saturation:** Magnetic hysteresis and high-frequency saturation compression with tape head loss.
   - **Engine 4: Germanium Fuzz:** Asymmetric soft-knee clipping with temperature leakage: $y = \text{clamp}(x + 0.1 x^2, -0.6, +1.2)$.
   - **Engine 5: Buchla Wavefolder:** Multi-stage sine fold algebra: $y = \sin(2\pi \cdot k_{fold} \cdot x)$.
   - **Engine 6: Modern Hard Clipper:** Clean digital ceiling with zero-overshoot anti-aliasing.
2. **Dynamic Harmonics Injection & Bias:**
   A DC bias trim allows continuous morphing between pure symmetric odd harmonics ($3^{\text{rd}}, 5^{\text{th}}$) and asymmetric even harmonics ($2^{\text{nd}}, 4^{\text{th}}$).
3. **Pre/Post Tone Filtering:**
   Integrated tilt equalizer and variable highpass/lowpass shelving filters allow targeting saturation strictly to mid/high frequencies while leaving sub-bass unmolested.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`DRIVE`):** Primary input gain ($0.0\,\text{dB} \dots +48.0\,\text{dB}$, Profile E). *Push: +12dB Turbo Boost*.
* **Knob 2 (`WARP MODEL`):** Discrete engine select: `Triode Tube`, `Pentode Tube`, `Tape Hysteresis`, `Germanium Fuzz`, `Buchla Wavefold`, `Hard Clipper`. Discrete Choice. *Push: Next Model*.
* **Knob 3 (`FOLD / CRUNCH`):** Wavefolder multiplier or secondary diode crunch ($0.0\% \dots 100.0\%$, Profile E). *Push: Max Fold*.
* **Knob 4 (`TONE / TILT`):** Continuous pre/post spectral tilt ($-6\,\text{dB/oct} \dots +6\,\text{dB/oct}$, Profile B). *Push: Reset Flat*.
* **Knob 5 (`BASS PRE-CUT`):** Pre-saturation low-cut filter ($20\,\text{Hz} \dots 500\,\text{Hz}$, Profile D). *Push: 80Hz Tight*.
* **Knob 6 (`EVEN BIAS`):** Asymmetry bias injecting even-order harmonics ($0.0\% \dots 100.0\%$, Profile B). *Push: Pure Symmetrical (0%)*.
* **Knob 7 (`CEILING / MAKEUP`):** Output stage makeup level and safety limiter ($-24\,\text{dB} \dots +12\,\text{dB}$, Profile B). *Push: Unity (0dB)*.
* **Knob 8 (`MIX`):** Universal Tri-Zone Dry/Wet Mix (Profile M: 7:00 Wet Audition, 12:00 Pure Dry, 5:00 Parallel). *Push: Phase Invert ($\varnothing = 180^\circ$)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Oversampling Factor):** `[ 2x Fast | 4x Linear Phase | 8x Pristine Polyphase FIR | 1x Direct ]`.
* **DIP 2 (Transformer Emulation):** `[ Neve St. Ives Input/Output Iron | Clean Transformerless | Vintage UTC ]`.
* **DIP 3 (Stereo Coupling):** `[ Linked Stereo | Dual Mono | Mid/Side Saturation Split ]`.
* **DIP 4 (DC Removal):** `[ 5Hz DC Servo Trap | Transparent Highpass | DC Coupled Raw ]`.
* **TRIM 1 (High-Frequency Slew Limiting):** Simulates op-amp slew rate limits ($0.2\,\text{V}/\mu\text{s} \dots 15\,\text{V}/\mu\text{s}$).
* **TRIM 2 (Tape Flutter Speed):** Irregular mechanical capstan speed fluctuations.
* **TRIM 3 (Mains Hum Bleed):** Subtle 60Hz/50Hz power supply hum injection.
* **TRIM 4 (Noise Gate Threshold):** Pre-drive optical silence gate floor.

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Model (Knob 2) | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`Buchla Wavefold`** | `Knob 3` | `SYMMETRY` | `±100%` | `Profile B` | Offsets fold center for complex even harmonics |
| **`Tape Hysteresis`** | `Knob 3` | `TAPE SPEED` | `7.5..30 IPS` | `Profile B` | Shifts tape head saturation and low-end bump |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Polyphase FIR Oversampling:** 4x and 8x oversampling implemented using zero-phase equiripple FIR halfband polyphase filter stages.
* **Vectorized Processing:** Saturation functions computed on aligned 128-bit/256-bit SIMD registers with zero memory allocations.
