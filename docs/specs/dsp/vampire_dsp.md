# Technical DSP Specification: `Vampire` (Synthetic Reverb Tail Generator & Feedback Pitch Shifter)

**Engine ID:** `FX-16`  
**Sovereign Name:** `Vampire`  
**Corporate Boring Name:** `Pitch-Tracking Resonant Reverb Tail & Sub-Octave Screamer`  
**Category:** Dark Ambient & Pitch-Shifting Resonator  
**APVTS Prefix:** `vamp_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section C](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Eventide H910 / H3000 Harmonizer (micro-pitch shift delay feedback loop), Electro-Harmonix Octave Multiplexer, Ursa Major Space Station (multi-tap granular feedback cluster).
* **Upstream Offline Reference:** [electro_harmonix_octave_multiplexer.md](file:///C:/Dev/Research/library/manuals/electro_harmonix_octave_multiplexer.md) · [ROSETTA_STONE.md#section-c](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c).
* **C++ Implementation Header:** [`VampireTailGenerator.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/VampireTailGenerator.h).

---

## 2. Mathematical Theory & DSP Architecture

Vampire generates haunting, infinite decaying reverberation tails infused with pitch-shifted feedback and sub-harmonic extraction:
1. **The Shimmer & Shiver Pitch-Feedback Loop:**
   An FDN reverb core is cross-patched with dual micro-pitch shifters in its internal feedback matrix:
   $$v_{tank}[n] = \text{FDN}\left(x[n] + k_{feed} \cdot \text{Shift}_{\pm \Delta f}(v_{tank}[n-1])\right)$$
   When shifting up ($+12\,\text{st}$), it creates celestial crystals of infinite ascending shimmer; when shifting down ($-12\,\text{st}$ or $-24\,\text{st}$), it spirals downward into a suffocating, blood-curdling sub-bass vortex.
2. **Sub-Octave Pulse Division (The Vampire Fang):**
   A Schmitt-trigger comparator extracts the fundamental zero-crossings and drives a flip-flop divider to inject a raw, phase-locked $-1\text{ oct}$ or $-2\text{ oct}$ sub-bass square wave directly into the reverb tank.
3. **Non-Linear Dark Damping & Tape Bleed:**
   Aggressive 4-pole highpass/lowpass filters in the tank prevent runaway screeching, turning feedback into a warm, undulating dark drone.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`DECAY / TANK SIZE`):** Internal tank duration ($1.0\,\text{s} \dots 60.0\,\text{s}$ + Infinite Freeze, Profile B). *Push: Infinite Freeze*.
* **Knob 2 (`PITCH SHIFT`):** Feedback pitch transposition ($-24.0\,\text{st} \dots 0.0 \dots +24.0\,\text{st}$, Profile B Bipolar). *Push: Snap Octave Jump*.
* **Knob 3 (`SHIMMER / FEEDBACK`):** Regeneration into pitch shifter ($0.0\% \dots 100.0\%$, Profile B). *Push: Snap Max Bloom*.
* **Knob 4 (`SUB-OCTAVE FANG`):** Level of $-1\text{ oct} / -2\text{ oct}$ sub-pulse generator ($0.0\% \dots 100.0\%$, Profile B). *Push: Toggle -1 / -2 Oct*.
* **Knob 5 (`DAMPING / DARKNESS`):** High-frequency absorption shelf ($400\,\text{Hz} \dots 10.0\,\text{kHz}$, Profile D). *Push: Pitch Black (400Hz)*.
* **Knob 6 (`MODULATION DEPTH`):** Delay tank chorusing and pitch detune ($0.0\% \dots 100.0\%$, Profile B). *Push: Mute Mod*.
* **Knob 7 (`BLOOD DRIVE`):** Tube/diode clipping in the feedback loop ($0.0\,\text{dB} \dots +24.0\,\text{dB}$, Profile E). *Push: Clean (0dB)*.
* **Knob 8 (`MIX`):** Universal Tri-Zone Dry/Wet Mix (Profile M: 7:00 Wet Audition, 12:00 Pure Dry, 5:00 Parallel). *Push: Phase Invert ($\varnothing = 180^\circ$)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Pitch Shifter Algorithm):** `[ Dual-Head Grain Window | Phase Vocoder Clean | Lo-Fi Eventide H910 Glitch ]`.
* **DIP 2 (Sub-Octave Source):** `[ Analog Zero-Crossing Flip-Flop | DSP Tracking PLL | Digital Monophonic ]`.
* **DIP 3 (Stereo Tank Routing):** `[ Dual Mono Independent | Cross-Coupled Stereo FDN | Antiphase Orbital ]`.
* **DIP 4 (Freeze Mode):** `[ Soft Infinite Sustain | Hard Glitch Repeat | Decaying Ambient Loop ]`.
* **TRIM 1 (Grain Size):** Window length for granular pitch shifting ($10\,\text{ms} \dots 120\,\text{ms}$).
* **TRIM 2 (Sub-Octave Lowpass Cutoff):** Anti-click smoothing filter on the flip-flop pulse wave.
* **TRIM 3 (Feedback Low-Cut Filter):** Highpass filter to eliminate DC buildup ($20\,\text{Hz} \dots 200\,\text{Hz}$).
* **TRIM 4 (Limiter Soft-Knee Threshold):** Maximum saturation ceiling in the internal feedback tank.

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Pitch Mode (DIP 1) | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`H910 Glitch Mode`** | `Knob 2` | `H910 RATIO` | `0.5..2.0` | `Profile B` | Direct control of vintage glitchy grain playback ratio |
| **`Sub-Octave Solo`** | `Knob 4` | `SUB GAIN` | `0..+18 dB` | `Profile E` | Direct boost of sub-harmonic tracking fundamental |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Pitch Shifter Splice Windowing:** Granular pitch shifting utilizes Hann-windowed overlapping dual delay taps with zero crossfade click artifacts.
* **Denormal Protection:** State variables in feedback delay networks flushed to zero using SSE FTZ intrinsics.
