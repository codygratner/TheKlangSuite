# Technical DSP Specification: `ClockBleed` (Analog Bucket-Brigade Delay Line)

**Engine ID:** `FX-14`  
**Sovereign Name:** `ClockBleed`  
**Corporate Boring Name:** `Lo-Fi Analog Bucket-Brigade Delay & SSM2044 Filter`  
**Category:** Analog BBD Delay & Lo-Fi Texture  
**APVTS Prefix:** `cbleed_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section C](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Electro-Harmonix Deluxe Memory Man (1976, Panasonic MN3005 BBD delay line), Boss DM-2 (MN3005 / MN3101 clock), E-mu SP-1200 (SSM2044 dynamic lowpass reconstruction filter).
* **Upstream Offline Reference:** [sp1200_service_manual.md](file:///C:/Dev/Research/library/manuals/sp1200_service_manual.md) · [ROSETTA_STONE.md#section-c](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c).
* **C++ Implementation Header:** [`ClockBleedBBD.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/ClockBleedBBD.h).

---

## 2. Mathematical Theory & DSP Architecture

ClockBleed models the dark, noisy, euphonic imperfections of long Bucket-Brigade Device delay lines:
1. **The BBD Bucket-Clock Equation:**
   In an $N$-stage BBD chip (e.g. twin MN3005 chips, $N = 8192$ stages), delay time is strictly governed by the high-frequency clock oscillator $f_{clock}$:
   $$\tau = \frac{N}{2 f_{clock}}$$
   To achieve long delay times ($500\,\text{ms} \dots 1000\,\text{ms}$), the clock frequency must drop dramatically down toward the audible band ($f_{clock} \approx 8\,\text{kHz} \dots 16\,\text{kHz}$). This creates iconic, high-pitched clock whine ("clock bleed") and requires aggressive anti-aliasing filtering.
2. **SSM2044 4-Pole Lowpass Reconstruction Filter:**
   To tame clock aliasing, ClockBleed feeds the BBD output through an SSM2044 24dB/oct resonant filter that tracks the clock rate:
   $$f_c(t) = k_{track} \cdot f_{clock}(t)$$
3. **Internal Chorus & Vibrato LFO:**
   Modulating the BBD clock with a low-frequency triangle wave creates the classic lush pitch vibrato and warm acoustic chorusing of the vintage Memory Man.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`DELAY TIME`):** Bucket clock rate ($15.0\,\text{ms} \dots 800.0\,\text{ms}$, Profile A / C). *Push: Tempo Sync Division*.
* **Knob 2 (`FEEDBACK`):** Runaway analog regeneration ($0.0\% \dots 120.0\%$, Profile F Hyperbolic). *Push: Infinite Hold*.
* **Knob 3 (`CLOCK BLEED`):** Ultrasonic clock whine leak into audio path ($-\infty \dots -36.0\,\text{dBFS}$, Profile B). *Push: Clock Whine Mute*.
* **Knob 4 (`BUCKET DAMPING`):** High-frequency bucket loss per stage ($500\,\text{Hz} \dots 8.0\,\text{kHz}$, Profile D Log). *Push: Dark Dub vs Clean*.
* **Knob 5 (`DRIVE`):** Analog input saturation into first bucket stage ($0.0\,\text{dB} \dots +20.0\,\text{dB}$, Profile E). *Push: +6dB Boost / Clean*.
* **Knob 6 (`COMPANDER CHARACTER`):** NE570 analog compander squashing and breathing artifacts ($0.0\% \dots 100.0\%$, Profile B). *Push: Clean vs Pumping*.
* **Knob 7 (`STEREO SPREAD / PING-PONG`):** Cross-delayed stereo ping-pong width ($0.0\% \dots 100.0\%$, Profile B). *Push: Mono Dub*.
* **Knob 8 (`MIX`):** Universal Tri-Zone Dry/Wet Mix (Profile M: 7:00 Wet Audition, 12:00 Pure Dry, 5:00 Parallel). *Push: Phase Invert (\varnothing = 180^\circ)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (BBD Stage Model):** `[ Twin MN3005 (8192 Stages) | Single MN3005 (4096) | Reticon SAD1024 Dark | Clean Digital ]`.
* **DIP 2 (Modulation Mode):** `[ Pitch Vibrato | Stereo Chorus | Rotary Wobble ]`.
* **DIP 3 (Anti-Aliasing Filter Mode):** `[ SSM2044 4-Pole Resonant | Sallen-Key 2-Pole Dark | Clean 8-Pole Cheby ]`.
* **DIP 4 (Input Dynamic Compander):** `[ NE570 Compander Active | Uncompressed Raw Distortion ]`.
* **TRIM 1 (Clock Bleed Pitch Tuning):** Adjusts carrier frequency offset of the clock bleed leak.
* **TRIM 2 (BBD DC Bias Asymmetry):** Induces even-harmonic saturation in delay buckets.
* **TRIM 3 (High-Frequency Degradation):** Bucket-to-bucket charge transfer inefficiency ($0.001\% \dots 0.05\%$).
* **TRIM 4 (Self-Oscillation Warmth):** Soft-knee limiter character in the feedback loop.

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Mod Mode (DIP 2) | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`Stereo Chorus`** | `Knob 7` | `CHORUS SPREAD` | `0..100%` | `Profile B` | Balances L/R spatial width of BBD chorus |
| **`Pitch Vibrato`** | `Knob 6, 7` | `VIB SPEED`, `VIB EXTENT` | `0.1..15 Hz`, `±2 st` | `Profile C`, `Profile B` | Pure pitch vibrato depth without dry signal comb |

--- | :--- | :--- | :--- | :--- | :--- |
| **`Stereo Chorus`** | `Knob 4` | `CHORUS SPREAD` | `0..100%` | `Profile B` | Balances L/R spatial width of BBD chorus |
| **`Pitch Vibrato`** | `Knob 3, 4` | `VIB SPEED`, `VIB EXTENT` | `0.1..15 Hz`, `±2 st` | `Profile C`, `Profile B` | Pure pitch vibrato depth without dry signal comb |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Charge-Transfer Inefficiency (CTI) Model:** High-frequency loss per tap modeled using a discrete state integrator update per audio block.
* **Anti-Aliasing Filter Stability:** SSM2044 filter model uses zero-delay feedback state-variable topology with guaranteed stability under extreme cutoff sweeps.
