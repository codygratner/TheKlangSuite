# Technical DSP Specification: `Annihilator Punch` (VCA Bus Compressor & Saturation Limiter)

**Engine ID:** `FX-04`  
**Sovereign Name:** `The Annihilator (NUKE)` / `Annihilator Punch`  
**Corporate Boring Name:** `Multi-Topology VCA Bus Compressor & Saturation Limiter`  
**Category:** Dynamics & Punch Processor  
**APVTS Prefix:** `nuke_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section C](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Empirical Labs Distressor EL8 ("Nuke" brickwall limiting curve and British opto-coupler mode), Solid State Logic SSL 4000 G Master Bus Compressor, Neve 33609 diode-bridge limiter.
* **Upstream Offline Reference:** [ssl_g_bus_manual.md](file:///C:/Dev/Research/library/manuals/ssl_g_bus_manual.md) · [01_renoise_complete_offline_guide.md](file:///C:/Dev/Research/library/01_renoise_complete_offline_guide.md).
* **C++ Implementation Header:** [`AnnihilatorCompressor.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/AnnihilatorCompressor.h).

---

## 2. Mathematical Theory & DSP Architecture

The Annihilator compressor features an ultra-responsive feed-forward VCA detection topology:
1. **Feed-Forward Level Detection with RMS/Peak Hybrid Filter:**
   $$x_{det}[n] = (1 - \beta) |x[n]| + \beta \sqrt{x_{det}^2[n-1]}, \quad \beta = e^{-1 / (\tau_{det} f_s)}$$
2. **Non-Linear Logarithmic Gain Computer:**
   Depending on the active compression topology, the gain reduction $g(t)$ in decibels is calculated via variable-ratio soft-knee curves:
   $$g_{dB} = \begin{cases} 
   0 & \text{if } x_{dB} < T - W/2 \\
   -\frac{1 - 1/R}{2W} (x_{dB} - T + W/2)^2 & \text{if } T - W/2 \le x_{dB} \le T + W/2 \\
   -(1 - 1/R)(x_{dB} - T) & \text{if } x_{dB} > T + W/2
   \end{cases}$$
   In **`NUKE`** mode, $R \to \infty$ with an aggressive reverse-log knee that clamps transients while pulling up microscopic room reverberation.
3. **Harmonic Saturation Injection (Dist 2 & Dist 3):**
   - **Dist 2:** Injects second harmonics (tube/triode emulation) via parabolic transfer: $y = x - 0.15 x^2$.
   - **Dist 3:** Injects third harmonics (tape saturation emulation) via cubic soft clipper: $y = 1.5 x - 0.5 x^3$.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`THRESHOLD`):** Compression engagement floor ($-40.0\,\text{dB} \dots 0.0\,\text{dB}$, Profile B). *Push: Auto-Set to Current Peak*.
* **Knob 2 (`RATIO`):** Compression ratio `1.5:1`, `2:1`, `4:1`, `8:1`, `10:1`, `20:1`, `NUKE (Inf:1)`. Discrete Choice. *Push: Instant NUKE Snap*.
* **Knob 3 (`ATTACK`):** Transient pass window ($0.05\,\text{ms} \dots 100.0\,\text{ms}$, Profile A Log). *Push: Fast 0.1ms*.
* **Knob 4 (`RELEASE`):** Gain recovery time ($10.0\,\text{ms} \dots 1.2\,\text{s}$ + Auto-Release). Profile A. *Push: Toggle Auto-Release*.
* **Knob 5 (`PUNCH DRIVE / HARMONICS`):** Dist 2 (Tube) and Dist 3 (Tape) harmonic saturation drive ($0.0\% \dots 100.0\%$, Profile E). *Push: Dist 2/3 Toggle*.
* **Knob 6 (`SIDECHAIN HPF`):** Sidechain low-cut filter ($20\,\text{Hz} \dots 350\,\text{Hz}$, Profile D). *Push: 120Hz Bus Standard*.
* **Knob 7 (`MAKEUP GAIN`):** Post-compression linear makeup level ($0.0\,\text{dB} \dots +30.0\,\text{dB}$, Profile B). *Push: Auto-Makeup Link*.
* **Knob 8 (`MIX`):** Universal Tri-Zone Dry/Wet Mix (Profile M: 7:00 Wet Audition, 12:00 Pure Dry, 5:00 Parallel New York Compression). *Push: Phase Invert (\varnothing = 180^\circ)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Detector Circuit):** `[ SSL 4000 G VCA | Distressor Opto- ब्रिटिश | Fairchild 670 Feedback | 1176 All-Buttons-In ]`.
* **DIP 2 (Knee Character):** `[ Razor Hard Knee | Gentle Soft Knee (6dB) | Ultra-Smooth Broad Knee (12dB) ]`.
* **DIP 3 (Harmonic Profile):** `[ Clean Transparent | Dist 2 (Triode Tube) | Dist 3 (Tape Saturation) | Class-A Discrete ]`.
* **DIP 4 (Stereo Link):** `[ 100% Linked (Summed RMS) | Dual Mono Unlinked | Sidechain Mid-Dominant ]`.
* **TRIM 1 (Sidechain Bell Boost):** Presence boost ($+3\,\text{dB}$ at $3\,\text{kHz}$) in sidechain to tame harsh sibilance.
* **TRIM 2 (Transient Expansion Bias):** Pre-detection crest factor modifier.
* **TRIM 3 (Release Slew Rate):** Non-linear release curve acceleration under massive gain reduction.
* **TRIM 4 (Ceiling Brickwall Limiter):** Post-makeup zero-lookahead safety clipper threshold ($-0.5\,\text{dB} \dots 0.0\,\text{dBFS}$).

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Active Ratio / Mode | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`NUKE (Inf:1)`** | `Knob 3` | `BRICK ATTACK` | `0.01..10 ms` | `Profile A` | Clamps microscopic drum transient spikes |
| **`Auto-Release`** | `Knob 4` | `AUTO SENS` | `0..100%` | `Profile B` | Balances fast transient recovery vs slow program bass |
| **`Dist 2 / Dist 3`** | `Knob 5` | `TUBE/TAPE BLEND`| `0..100%` | `Profile B` | Continuously morphs between 2nd and 3rd harmonics |

--- | :--- | :--- | :--- | :--- | :--- |
| **`NUKE (Inf:1)`** | `Knob 3` | `BRICK ATTACK` | `0.01..10 ms` | `Profile A` | Clamps microscopic drum transient spikes |
| **`Auto-Release`** | `Knob 4` | `AUTO SENS` | `0..100%` | `Profile B` | Balances fast transient recovery vs slow program bass |
| **`Dist 2 / Dist 3`** | `Knob 6` | `TUBE/TAPE BLEND`| `0..100%` | `Profile B` | Continuously morphs between 2nd and 3rd harmonics |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Vectorized Gain Application:** Gain reduction calculation separated into scalar detection and SIMD vectorized audio block multiplication.
* **Denormal Free Smoothing:** Log-domain ballistic smoothing filters use single-pole IIR with zero coefficient recalculation overhead during audio processing.
