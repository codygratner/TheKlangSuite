# Technical DSP Specification: `Optical Pan & Trem` (Harmonic Tremolo & Stereo Auto-Panner)

**Engine ID:** `FX-11`  
**Sovereign Name:** `Optical Pan & Trem`  
**Corporate Boring Name:** `Dual-LFO Stereo Panner, Harmonic Tremolo & Dynamic Vibrato`  
**Category:** Modulation & Dynamic Spatial Amplitude  
**APVTS Prefix:** `pantrem_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section C](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Fender 1961 "Brownface" Super/Concert/Vibrosonic Amp Harmonic Tremolo (dual-band crossover amplitude modulation), vintage Rhodes Suitcase 73 stereo optical panner, Vox AC30 optical tremolo.
* **Upstream Offline Reference:** [fender_vibrato_manual.md](file:///C:/Dev/Research/library/manuals/fender_vibrato_manual.md) · [ROSETTA_STONE.md#section-c](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c).
* **C++ Implementation Header:** [`HarmonicPanTrem.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/HarmonicPanTrem.h).

---

## 2. Mathematical Theory & DSP Architecture

Optical Pan & Trem synthesizes three classic spatial modulation topologies:
1. **The Fender "Harmonic Tremolo" Engine (True Phase-Shift Vibrato):**
   Unlike standard tremolo (which merely turns volume up and down), Harmonic Tremolo splits incoming audio with a 2-pole crossover network:
   - Low band ($f < 800\,\text{Hz}$) modulated by $\cos(\omega_{lfo} t)$.
   - High band ($f > 800\,\text{Hz}$) modulated by $\sin(\omega_{lfo} t)$ ($90^\circ$ or $180^\circ$ out of phase).
   When the bands recombine, the total energy remains nearly constant, but the phase and frequency spectrum undulate wildly, producing a lush, chewy, pseudo-Doppler rotary effect without any pitch artifacts.
2. **Opto-Coupler Non-Linear Vactrol Shunting:**
   Optical light-dependent resistor non-linearities create a distinctive curved volume modulation wave that softens abrupt edges and breathes organically with audio transients.
3. **Stereo Panning Laws & Auto-Pan Matrix:**
   Provides constant-power ($+3\,\text{dB}$ center dip) or linear panning across stereo space with variable pan laws.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`SPEED`):** Modulation rate ($0.1\,\text{Hz} \dots 20.0\,\text{Hz}$, Profile C). *Push: Tempo Sync Division*.
* **Knob 2 (`DEPTH`):** Amplitude modulation depth ($0.0\% \dots 100.0\%$, Profile B). *Push: Max 100% Chop*.
* **Knob 3 (`HARMONIC / VINTAGE`):** Continuous morph between Vintage Optical Amplitude Tremolo (0%) and Fender Brownface Harmonic Tremolo (100%). Profile B. *Push: Toggle Mode*.
* **Knob 4 (`WAVEFORM`):** Sine, Triangle, Square (Hard Chop), Saw Up, Saw Down, S&H Random. Discrete Choice. *Push: Next Waveform*.
* **Knob 5 (`TUBE WARMTH`):** Pre-tremolo 12AX7 preamp tube warmth ($0.0\,\text{dB} \dots +18.0\,\text{dB}$, Profile E). *Push: Clean vs Saturated*.
* **Knob 6 (`CROSSOVER FREQ`):** Harmonic tremolo crossover split ($200\,\text{Hz} \dots 1200\,\text{Hz}$, Profile D). *Push: Snap 500Hz*.
* **Knob 7 (`STEREO PHASE`):** Left/Right LFO phase offset ($0^\circ = \text{Mono Trem} \dots 180^\circ = \text{Auto-Pan Ping-Pong}$, Profile B). *Push: Snap 180° Ping-Pong*.
* **Knob 8 (`MIX`):** Universal Tri-Zone Dry/Wet Mix (Profile M: 7:00 Wet Audition, 12:00 Pure Dry, 5:00 Parallel). *Push: Phase Invert (\varnothing = 180^\circ)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Modulation Topology):** `[ Fender Harmonic Tremolo | Standard VCA Optical Tremolo | Rhodes Stereo Auto-Pan | Dynamic Env Tremolo ]`.
* **DIP 2 (Pan Law):** `[ -3dB Constant Power | -4.5dB Modern Compromise | -6dB Linear Acoustic ]`.
* **DIP 3 (Dynamic Envelope Direction):** `[ Upward (Play Harder = Faster Rate) | Downward (Play Harder = Slow Down) | Depth Mod ]`.
* **DIP 4 (Bulb Inertia):** `[ Incandescent Lamp Glow (Warm Rise/Fall) | Fast Neon Strobe | Instant Photodiode ]`.
* **TRIM 1 (Envelope Follower Sensitivity):** Controls responsiveness of dynamic rate/depth modulation.
* **TRIM 2 (Crossover Filter Q):** Resonance damping at the harmonic split frequency.
* **TRIM 3 (DC Bias / Asymmetry):** Asymmetric pulse duty cycle on the optical bulb ($30\% \dots 70\%$).
* **TRIM 4 (Noise Floor Bleed):** Subtle tube preamp thermal hum and opto hiss.

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Active Topology (DIP 1) | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`Rhodes Auto-Pan`** | `Knob 7` | `PAN LAW TAPER` | `-6dB..0dB` | `Profile B` | Adjusts center attenuation on stereo field |
| **`Dynamic Env Tremolo`**| `Knob 5` | `ENV SENSITIVITY` | `0..100%` | `Profile B` | Envelope velocity to tremolo speed multiplier |

--- | :--- | :--- | :--- | :--- | :--- |
| **`Rhodes Auto-Pan`** | `Knob 3` | `PAN LAW TAPER` | `-6dB..0dB` | `Profile B` | Adjusts center attenuation on stereo field |
| **`Dynamic Env Tremolo`**| `Knob 7` | `ENV SENSITIVITY` | `0..100%` | `Profile B` | Envelope velocity to tremolo speed multiplier |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Phase-Coherent Linkwitz-Riley Crossover:** Dual 2-pole allpass-summed Butterworth crossover ensuring zero comb filtering when dry/wet bands recombine.
* **Lookup-Table Free Sines:** LFO phase accumulators computed via high-precision polynomial cosine approximation.
