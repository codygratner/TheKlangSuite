# Technical DSP Specification: `Rotary Speaker` (Rotating Doppler Speaker & Amp)

**Engine ID:** `FX-18`  
**Sovereign Name:** `Rotary Horn & Drum` / `Rotary Speaker`  
**Corporate Boring Name:** `Leslie 122 Rotating Horn & Drum Simulator`  
**Category:** Physical Modeling & Mechanical Doppler Speaker  
**APVTS Prefix:** `rot_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section C](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Leslie 122 / 147 Rotating Speaker Cabinet (1941, designed by Donald Leslie, featuring a spinning treble horn, rotating wooden bass drum rotor, and 40W 6550 tube power amplifier).
* **Upstream Offline Reference:** [leslie_122_service_manual.md](file:///C:/Dev/Research/library/manuals/leslie_122_service_manual.md) · [ROSETTA_STONE.md#section-c](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c).
* **C++ Implementation Header:** [`RotarySpeakerModel.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/RotarySpeakerModel.h).

---

## 2. Mathematical Theory & DSP Architecture

The iconic Leslie sound is an acoustic composite of three simultaneous physical phenomena:
1. **Acoustic Doppler Pitch Modulation:**
   As the horn rotates toward the listener at tangential velocity $v_t = \omega r$, the perceived frequency shifts upward; as it moves away, frequency shifts downward:
   $$f_{obs}(t) = f_{src} \left( 1 + \frac{\omega r \sin(\theta(t))}{c} \right)$$
   Where $c = 343\,\text{m/s}$ is the speed of sound.
2. **Directional Radiation Pattern (Amplitude & Filter Modulation):**
   The horn has a cardioid/dipole acoustic dispersion pattern. Sound radiates with high intensity and full treble when pointed at the microphone, but drops in volume and becomes dark/muffled when pointing away.
3. **Dual Rotor Physics (Horn vs Drum Momentum):**
   - **Treble Horn:** Low mechanical inertia ($M_{horn} \approx 0.2\,\text{kg}$). Speeds up quickly ($\tau_{acc} \approx 1.0\,\text{s}$), slows down in $\approx 1.5\,\text{s}$.
   - **Bass Drum:** Heavy wooden rotor ($M_{drum} \approx 5.0\,\text{kg}$). Speeds up slowly ($\tau_{acc} \approx 4.0\,\text{s}$), coasts down in $\approx 6.0\,\text{s}$.
   The phase and speed differential between the fast horn and sluggish bass drum produces a rich, non-repeating acoustic interplay.
4. **Crossover Network & 6550 Tube Overdrive:**
   An $800\,\text{Hz}$ crossover splits audio between horn and drum. A push-pull 6550 power amplifier stage injects warm transformer growl and asymmetric tube clipping when pushed hard.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`SPEED SWITCH`):** Chorale (Slow), Tremolo (Fast), Brake (Stop). Discrete Choice. *Push: Cycle Speed (Slow / Fast / Brake)*.
* **Knob 2 (`FAST SPEED`):** Target rotational velocity in Fast mode ($5.0\,\text{Hz} \dots 9.0\,\text{Hz}$, Profile C). *Push: Classic 6.8Hz*.
* **Knob 3 (`SLOW SPEED`):** Target rotational velocity in Slow mode ($0.5\,\text{Hz} \dots 1.5\,\text{Hz}$, Profile C). *Push: Classic 0.8Hz*.
* **Knob 4 (`HORN / DRUM BALANCE`):** Volume balance between top high-frequency horn and bottom bass drum. Profile B. *Push: Equal 50/50 Balance*.
* **Knob 5 (`TUBE DRIVE`):** 40-watt 6550 tube power amp saturation ($0.0\,\text{dB} \dots +24.0\,\text{dB}$, Profile E). *Push: Clean vs Screaming Overdrive*.
* **Knob 6 (`ACCELERATION / INERTIA`):** Motor ramp-up/ramp-down transition time ($0.5\,\text{s} \dots 8.0\,\text{s}$, Profile A). *Push: Snap 2.0s*.
* **Knob 7 (`MIC DISTANCE / SPREAD`):** Virtual stereo microphone placement distance ($0.2\,\text{m} \dots 3.0\,\text{m}$) and stereo angle. Profile B. *Push: Close Studio Mics*.
* **Knob 8 (`MIX`):** Universal Tri-Zone Dry/Wet Mix (Profile M: 7:00 Wet Audition, 12:00 Pure Dry, 5:00 Parallel). *Push: Phase Invert (\varnothing = 180^\circ)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Cabinet Model):** `[ Leslie 122 (Warm Wooden Vintage) | Leslie 147 (High-Gain Rock) | Open Baffle Doppler | Custom Modern ]`.
* **DIP 2 (Brake Action):** `[ Coast to Stop (Gradual Friction) | Hard Motor Brake | Free Coast Endless ]`.
* **DIP 3 (Microphone Configuration):** `[ Stereo Horn + Mono Drum | Dual Stereo Horn & Drum | Single Point Mono ]`.
* **DIP 4 (Horn Deflector Cap):** `[ Installed (Diffused Mellow Tone) | Removed (Aggressive Laser Punch) ]`.
* **TRIM 1 (Bass Drum Acceleration):** Momentum inertia of the heavy bass rotor ($2.0\,\text{s} \dots 10.0\,\text{s}$).
* **TRIM 2 (Crossover Split Frequency):** Adjusts acoustic divider ($600\,\text{Hz} \dots 1200\,\text{Hz}$).
* **TRIM 3 (Motor Mechanical Belt Noise):** Subtle belt squeak and motor hum ($-\infty \dots -45\,\text{dB}$).
* **TRIM 4 (Cabinet Internal Room Resonance):** Simulates wooden box acoustic resonance and reflection.

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Speed Selection (Knob 1) | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`Fast (Tremolo)`** | `Knob 2` | `FAST SPEED` | `300..480 RPM` | `Profile B` | Fine calibration of top-speed tremolo rotation |
| **`Slow (Chorale)`** | `Knob 3` | `SLOW SPEED` | `25..60 RPM` | `Profile B` | Fine calibration of slow chorale swirl |
| **`Stop (Brake)`** | `Knob 7` | `HORN STOP ANGLE`| `0°..360°` | `Profile B` | Static pointing direction of stationary horn |

--- | :--- | :--- | :--- | :--- | :--- |
| **`Fast (Tremolo)`** | `Knob 1` | `FAST SPEED` | `300..480 RPM` | `Profile B` | Fine calibration of top-speed tremolo rotation |
| **`Slow (Chorale)`** | `Knob 1` | `SLOW SPEED` | `25..60 RPM` | `Profile B` | Fine calibration of slow chorale swirl |
| **`Stop (Brake)`** | `Knob 6` | `HORN STOP ANGLE`| `0°..360°` | `Profile B` | Static pointing direction of stationary horn |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Variable Delay Fractional Doppler Line:** Interpolated using 4-point Hermite spline with continuous motor angle phase integration.
* **Zero Allocation Invariant:** Fixed circular delay buffers with power-of-two lengths, strictly zero heap allocations on audio thread.
