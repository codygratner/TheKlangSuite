# Technical DSP Specification: `Cerberus` (Harmonic Distortion Synthesizer & Preamp)

**Engine ID:** `FX-01`  
**Sovereign Name:** `Cerberus`  
**Corporate Boring Name:** `Multi-Stage Harmonic Distortion & Preamp`  
**Category:** Saturation & Overdrive Processor  
**APVTS Prefix:** `cerb_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section C](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** ProCo Rat (1978, LM308 op-amp slew limiting + symmetrical silicon diode clipping), BOSS DS-1 (transistor booster + diode clipper), Korg MS-20 External Signal Processor (ESP) Schmitt-trigger preamp.
* **Upstream Offline Reference:** [rat_ds1_pedal_manuals.md](file:///C:/Dev/Research/library/manuals/rat_ds1_pedal_manuals.md) · [korg_dist_service_manual.md](file:///C:/Dev/Research/library/manuals/korg_dist_service_manual.md).
* **C++ Implementation Header:** [`CerberusDistortion.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/CerberusDistortion.h).

---

## 2. Mathematical Theory & DSP Architecture

The Cerberus distortion core cascades three non-linear processing stages:
1. **Op-Amp Input Saturation with Slew-Rate Limiting:**
   Simulating the classic LM308 operational amplifier which exhibits a low slew rate ($SR = 0.5\,\text{V}/\mu\text{s}$), rounding off high-frequency spikes into smooth odd harmonics:
   $$\frac{dv_{out}}{dt} = \text{clamp}\left(\frac{v_{in} - v_{out}}{\tau},\, -SR,\, +SR\right)$$
2. **Asymmetric Diode Shunt Clipper:**
   The output stage shunts signal across antiparallel diode pairs (Silicon 1N4148, Germanium 1N34A, or LED):
   $$i_d(v) = I_s \left( e^{\frac{v}{\eta V_t}} - 1 \right) - I_s \left( e^{\frac{-v}{\eta V_t}} - 1 \right)$$
   Germanium diodes provide soft compression with a low threshold ($V_f \approx 0.3\,\text{V}$), while Silicon yields aggressive hard clipping ($V_f \approx 0.7\,\text{V}$) and LEDs punch through with high headroom ($V_f \approx 1.8\,\text{V}$).
3. **Passive Reverse-Log Tone Filter (Rat "Filter" Circuit):**
   A variable single-pole lowpass filter where turning the knob clockwise cuts high frequencies rather than boosting them:
   $$H(z) = \frac{1 - \alpha}{1 - \alpha z^{-1}}, \quad \alpha = e^{-2\pi f_c / f_s}$$

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`DRIVE`):** Input gain into the LM308 op-amp stage ($0.0\,\text{dB} \dots +48.0\,\text{dB}$, Profile E). *Push: Toggle Clean Boost (+12dB)*.
* **Knob 2 (`FILTER / TONE`):** Reverse-log high-frequency attenuation ($800\,\text{Hz} \dots 20\,\text{kHz}$, Profile D Log). *Push: Reset Flat*.
* **Knob 3 (`DIODE MODEL`):** Discrete choice `0..3` (`0: Silicon 1N4148`, `1: Germanium 1N34A`, `2: Red LED Turbo`, `3: Asymmetric Hybrid`). *Push: Next Model*.
* **Knob 4 (`SLEW RATE`):** Operational amplifier slew limit ($0.1\,\text{V}/\mu\text{s} \dots 10.0\,\text{V}/\mu\text{s}$, Profile B). *Push: Pristine Infinite Slew*.
* **Knob 5 (`BASS PRE-CUT`):** Pre-distortion highpass filter ($20\,\text{Hz} \dots 400\,\text{Hz}$) to prevent low-end intermodulation mud. Profile D. *Push: 80Hz Tight*.
* **Knob 6 (`POST CLIP GAIN`):** Output stage makeup level ($-24\,\text{dB} \dots +12\,\text{dB}$, Profile B). *Push: Unity Gain (0 dB)*.
* **Knob 7 (`WARMTH BIAS`):** DC offset injection before diode clipping for even-harmonic tube bloom ($0.0\% \dots 100.0\%$, Profile B). *Push: Symmetrical Center*.
* **Knob 8 (`MIX`):** Universal Tri-Zone Dry/Wet Mix (Profile M: 7:00 Wet Audition, 12:00 Pure Dry, 5:00 Parallel). *Push: Phase Invert ($\\varnothing = 180^\\circ$)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Clipping Topology):** `[ Symmetrical Shunt | Asymmetrical Single-Diode | Series Op-Amp Hard Clip | Germanium Foldback ]`.
* **DIP 2 (Oversampling):** `[ 1x Direct | 2x Min-Phase | 4x Linear-Phase Polyphase FIR ]`.
* **DIP 3 (DC Blocking):** `[ 5Hz DC Servo Trap | Transparent Highpass | DC Coupled Unfiltered ]`.
* **DIP 4 (Input Buffer):** `[ High-Z JFET Guitar Buffer (1MΩ) | Low-Z Line Studio Buffer (10kΩ) ]`.
* **TRIM 1 (Diode Leakage Current $I_s$):** Micro-ampere leakage tuning for vintage temperature drift.
* **TRIM 2 (Op-Amp Rail Sag):** Simulates 9V battery voltage droop under heavy power draw.
* **TRIM 3 (High-Shelf Air):** Post-filter $+3\,\text{dB}$ presence shelf at $10\,\text{kHz}$.
* **TRIM 4 (Noise Gate Threshold):** Pre-distortion silent optical gate floor ($-\infty \dots -50\,\text{dB}$).

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Active Diode Model | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`0: Silicon 1N4148`** | `Knob 4` | `LM308 SLEW` | `0.1..5.0 V/us` | `Profile B` | Slew rate limiting characteristic of vintage Rat |
| **`1: Germanium 1N34A`** | `Knob 7` | `GERM LEAKAGE` | `0.0..100.0%` | `Profile E` | Temperature-dependent diode compression softness |
| **`2: Red LED Turbo`** | `Knob 1` | `TURBO HEADROOM`| `0..+36 dB` | `Profile E` | Wide-swing high headroom dynamic clipping |
| **`3: Asymmetric Hybrid`** | `Knob 7` | `EVEN HARMONICS`| `0..100%` | `Profile B` | Single-ended second-order saturation bias |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Zero-Allocation Invariant:** No dynamic buffer reallocations during oversampling or diode table updates.
* **Look-Up Table (LUT) / FastMath:** Anti-parallel diode equations evaluated via 1024-point SIMD vectorized cubic polynomial spline approximation or fast Padé approximants.
* **De-Cramped Filters:** The post-tone filter uses bilinear transform with frequency warping to avoid high-frequency cramping near Nyquist.
