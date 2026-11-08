# Technical DSP Specification: `KlangDelay` (Echo, Delay & Repetition Lab)

**Engine ID:** `MEGA-07`  
**Sovereign Name:** `KlangDelay`  
**Corporate Boring Name:** `Multi-Tap Tape, BBD & Ping-Pong Diffusion Delay Workstation`  
**Category:** Flagship Studio Delay Workstation  
**APVTS Prefix:** `kdelay_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section D](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Roland Space Echo RE-201, Electro-Harmonix Deluxe Memory Man, Watkins Copicat, modern diffusion echo processors.
* **Upstream Offline Reference:** [roland_space_echo_re201_manual.md](file:///C:/Dev/Research/library/manuals/roland_space_echo_re201_manual.md) · [ROSETTA_STONE.md#section-d](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d).
* **C++ Implementation Header:** [`KlangDelayWorkstation.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/KlangDelayWorkstation.h).

---

## 2. Mathematical Theory & DSP Architecture

KlangDelay is the master delay workstation combining four distinct historical delay technologies:
1. **Four Specialized Delay Cores:**
   - **Core 1 (Tape Echo):** Multi-head tape loops with capstan motor slew, magnetic saturation, and mechanical wow/flutter.
   - **Core 2 (Analog BBD):** Panasonic MN3005 bucket brigade emulation with dark frequency loss and clock bleed.
   - **Core 3 (Digital Ping-Pong):** Crystal-clear stereo bouncing delay with tempo-synced dotted and triplet subdivisions.
   - **Core 4 (Diffusion Echo):** Cascaded all-pass diffusion network that smears delay repeats into lush ambient reverb clouds.
2. **Dynamic Ducking & Saturation:**
   Features an integrated sidechain ducking envelope follower that automatically pushes delays down by $3\,\text{dB} \dots 18\,\text{dB}$ while dry signal is active, preventing vocal clutter.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`TIME / DIV`):** Tempo-synced $1/64 \dots 2\,\text{bars}$ or $1.0\,\text{ms} \dots 2000.0\,\text{ms}$. Profile A / C. *Push: Free ms vs Tempo Division*.
* **Knob 2 (`FEEDBACK`):** Regeneration ($0.0\% \dots 115.0\%$ runaway). Profile F Hyperbolic. *Push: Dub Explosion Slam*.
* **Knob 3 (`TONE / DAMP`):** Active high-cut & low-cut tape damping ($800\,\text{Hz} \dots 20.0\,\text{kHz}$, Profile B / D). *Push: Flat Clean*.
* **Knob 4 (`ALGORITHM`):** 6 engines (Digital Ping-Pong, Space Echo RE-201 Tape, Memory Man BBD, Reverse Delay Buffer, Granular Cloud Freeze, Oil Can Delay). Discrete `0..5`. *Push: Next Algorithm*.
* **Knob 5 (`DRIVE`):** Analog tape preamp / BBD input saturation ($0.0\,\text{dB} \dots +18.0\,\text{dB}$, Profile E). *Push: Clean vs Overdriven Preamp*.
* **Knob 6 (`MOD / FLUTTER`):** Tape wow/flutter & BBD chorusing depth ($0.0\% \dots 100.0\%$, Profile B). *Push: Pristine vs Warped Tape*.
* **Knob 7 (`STEREO OFFSET`):** Ping-Pong cross-bounce width & delay time differential ($0.0\% \dots 100.0\%$, Profile B). *Push: Center Dual Mono*.
* **Knob 8 (`MIX`):** Universal Tri-Zone Dry/Wet Mix (Profile M: 7:00 Wet Audition, 12:00 Pure Dry, 5:00 Parallel). *Push: Phase Invert (\varnothing = 180^\circ)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Tape Head Config):** `[ Single Head | Multi-Head Pattern 1 | Multi-Head Pattern 2 | Dual Stereo ]`.
* **DIP 2 (Time Change Behavior):** `[ Analog Tape Scrub (Pitch Warps) | Clean Crossfade Jump ]`.
* **DIP 3 (Diffusion Stages):** `[ 0 Diffusers (Clean Echo) | 2 Diffusers Soft | 4 Diffusers Cloud ]`.
* **DIP 4 (Limiter Character):** `[ Soft Magnetic Tanh | Germanium Diode | Pristine Clean ]`.
* **TRIM 1 (Tape Wow Speed):** Capstan irregular motor wobble speed ($0.2\,\text{Hz} \dots 4.0\,\text{Hz}$).
* **TRIM 2 (Low-Cut Damping):** Highpass filter in feedback path to strip bass build-up ($20\,\text{Hz} \dots 400\,\text{Hz}$).
* **TRIM 3 (Ducking Release Time):** Recovery speed of the sidechain ducking envelope.
* **TRIM 4 (Noise Hiss Floor):** Authentic tape hiss and BBD clock noise level.

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Active Delay Algorithm | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`3: Reverse Delay`** | `Knob 1, Knob 6` | `REVERSE CHUNK`, `WINDOW SLEW` | `50ms..2000ms`, `5..50ms` | `Profile A`, `Profile B` | Reverse slice size & zero-crossing de-click |
| **`4: Granular Cloud`**| `Knob 2, Knob 6` | `FREEZE LATCH`, `GRAIN SPRAY` | `Toggle`, `0..100%` | `Discrete`, `Profile B` | Infinite buffer lock & stochastic grain jitter |
| **`5: Oil Can Delay`** | `Knob 5, Knob 6` | `DRIVE / MOTOR DRAG`, `FLUID WOBBLE` | `0..100%`, `±100%` | `Profile E`, `Profile B` | Rotational friction & electrostatic pickup wobble |

--- | :--- | :--- | :--- | :--- | :--- |
| **`Diffusion Echo`** | `Knob 6` | `DIFFUSION SPREAD`| `0..100%` | `Profile B` | Smears discrete echo taps into continuous reverb |
| **`Tempo Synced`** | `Knob 1` | `SYNC DIVISION` | `1/32..2/1` | `Discrete` | Snaps delay times to musical note subdivisions |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Cubic Hermite Interpolation:** Continuously modulated delay line read heads evaluated via 4-point Hermite spline interpolation.
* **Zero Allocation Delay Lines:** Fixed circular buffers statically allocated for max $5.0\,\text{s}$ at $96\,\text{kHz}$.
