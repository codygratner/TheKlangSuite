# Technical DSP Specification: `EchoMill` (Recursive Insert Tape Delay)

**Engine ID:** `FX-13`  
**Sovereign Name:** `EchoMill`  
**Corporate Boring Name:** `Multi-Head Magnetic Tape Delay & Sound-on-Sound Mill`  
**Category:** Tape Echo & Recursive Delay  
**APVTS Prefix:** `emill_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section C](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Roland Space Echo RE-201 (1974, 3 playback heads + 1 sound-on-sound head with spring reverb tank), Maestro Echoplex EP-3 (solid-state tape echo with discrete preamp).
* **Upstream Offline Reference:** [roland_space_echo_re201_manual.md](file:///C:/Dev/Research/library/manuals/roland_space_echo_re201_manual.md) · [ROSETTA_STONE.md#section-c](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c).
* **C++ Implementation Header:** [`EchoMillTapeDelay.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/EchoMillTapeDelay.h).

---

## 2. Mathematical Theory & DSP Architecture

EchoMill accurately reconstructs the multi-head tape transport, motor physics, and magnetic hysteresis of vintage tape echo machines:
1. **Multi-Head Delay Tap Topology:**
   A continuous tape loop travels past three physical playback heads positioned at fixed distances ($d_1, d_2, d_3$):
   $$t_1 = \frac{d_1}{v_{motor}}, \quad t_2 = \frac{d_2}{v_{motor}}, \quad t_3 = \frac{d_3}{v_{motor}}$$
   Changing the tape speed knob scales all three delay times proportionately while introducing realistic motor acceleration slew:
   $$\frac{dv}{dt} = \frac{v_{target} - v}{\tau_{motor}}$$
2. **Magnetic Oxide Saturation & Bass Loss:**
   Repeated trips through the feedback loop degrade the high and low frequencies:
   $$H_{head}(s) = \frac{s}{s + \omega_{low}} \cdot \frac{1}{1 + s/\omega_{high}}$$
   A non-linear tape compression function softens transients as feedback approaches $100\%$, allowing infinite self-oscillation without digital harshness.
3. **Sound-on-Sound (SOS) Tape Loop Mode:**
   Freezes the tape buffer into an endless decaying magnetic soundscape with manual record bias.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`TAPE SPEED / TIME`):** Capstan motor speed ($50.0\,\text{ms} \dots 1500.0\,\text{ms}$, Profile C). *Push: Tempo Sync (1/16..1/1)*.
* **Knob 2 (`FEEDBACK / REPEAT`):** Tape regeneration level ($0.0\% \dots 130.0\%$, Profile B). *Push: Infinite Loop Freeze*.
* **Knob 3 (`HEAD COMBINATION`):** Discrete head selection: `Head 1`, `Head 2`, `Head 3`, `Heads 1+2`, `Heads 2+3`, `Heads 1+2+3`. Discrete Choice. *Push: Next Combo*.
* **Knob 4 (`WOW & FLUTTER`):** Tape capstan wobble and tape transport friction ($0.0\% \dots 100.0\%$, Profile E). *Push: Mute Flutter*.
* **Knob 5 (`TAPE AGE / WEAR`):** High-frequency loss, dropout probability, and tape splice thumps ($0.0\% \dots 100.0\%$, Profile B). *Push: Mint Fresh Tape*.
* **Knob 6 (`PREAMP DRIVE`):** EP-3 discrete JFET preamp saturation ($0.0\,\text{dB} \dots +24.0\,\text{dB}$, Profile E). *Push: Clean Solid-State*.
* **Knob 7 (`SPRING REVERB LEVEL`):** Internal 2-spring virtual reverb tank wet level ($0.0\% \dots 100.0\%$, Profile B). *Push: Mute Spring*.
* **Knob 8 (`MIX`):** Universal Tri-Zone Dry/Wet Mix (Profile M: 7:00 Wet Audition, 12:00 Pure Dry, 5:00 Parallel). *Push: Phase Invert ($\varnothing = 180^\circ$)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Delay Pan Mode):** `[ Mono Center | Ping-Pong Multi-Head | Stereo Head Spatial Split ]`.
* **DIP 2 (Motor Inertia Physics):** `[ Heavy RE-201 Flywheel (Slow Pitch Glide) | Fast EP-3 Capstan | Instant Digital Jump ]`.
* **DIP 3 (Splice Bump):** `[ Authentic Tape Splice Bump Every Loop | Silent Continuous Loop ]`.
* **DIP 4 (Self-Oscillation Limiter):** `[ Analog Magnetic Soft Tanh | Asymmetric Germanium | Modern Brickwall ]`.
* **TRIM 1 (Tape Hiss & Hum Floor):** Background magnetic tape noise level ($-\infty \dots -45\,\text{dB}$).
* **TRIM 2 (Dropout Frequency):** Average rate of momentary magnetic particle dropouts.
* **TRIM 3 (Spring Reverb Decay):** T60 reverberation time of the internal dual-spring tank ($1.0\,\text{s} \dots 5.0\,\text{s}$).
* **TRIM 4 (Head Azimuth Alignment):** Minute stereo phase misalignment between playback heads.

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Active Head Combo | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`Multi-Head Modes`** | `Knob 3` | `HEAD RHYTHM` | `1..6 Pat` | `Discrete` | Selects polyrhythmic rhythmic echo taps |
| **`Sound-on-Sound`** | `Knob 2` | `LOOP DECAY` | `0..100%` | `Profile B` | Decay rate per tape pass in endless looper |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Continuous Sinc Resampling:** Capstan speed changes interpolate tape buffer using 4-point cubic Hermite spline to simulate tape scrub pitch warping.
* **Bounded Saturation:** Feedback accumulation governed by $\tanh$ curve preventing float overflow under runaway self-oscillation.
