# Technical DSP Specification: `ZeroFlange` (Through-Zero Tape Flanger)

**Engine ID:** `FX-09`  
**Sovereign Name:** `ZeroFlange`  
**Corporate Boring Name:** `Through-Zero Dual-Capstan Tape Flanger`  
**Category:** Tape & BBD Time-Domain Modulation  
**APVTS Prefix:** `tzf_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section C](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Vintage 1960s Abbey Road twin reel-to-reel tape decks (Ken Townsend through-zero flanging), MXR Flanger / Doubler, Eventide Instant Flanger.
* **Upstream Offline Reference:** [through_zero_flanging_pedal_manuals.md](file:///C:/Dev/Research/library/manuals/through_zero_flanging_pedal_manuals.md) · [ROSETTA_STONE.md#section-c](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c).
* **C++ Implementation Header:** [`ZeroFlangeTZF.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/ZeroFlangeTZF.h).

---

## 2. Mathematical Theory & DSP Architecture

Through-Zero Flanging (TZF) differs fundamentally from standard flanging by advancing the modulated delay line *ahead* of the dry reference path:
1. **The "True Zero" Crossing Mechanism:**
   In physical tape flanging, one tape deck is slowed down by pressing a thumb on the reel flange. As its playback head lags behind the second deck, passes through exact synchronicity ($t_{diff} = 0$), and moves ahead, complete destructive acoustic cancellation occurs.
   In DSP, this requires a fixed "reference lookahead delay" $\tau_0$ applied to the dry signal:
   $$y[n] = x[n - \tau_0] \pm x[n - \tau(t)]$$
   Where the swept delay $\tau(t) = \tau_0 + A \cdot \text{LFO}(t)$. When $\tau(t) = \tau_0$, the two paths are identical in latency. If polarities are inverted ($\varnothing = 180^\circ$), total silence (100% cancellation) occurs at the null point, creating the iconic "jet-engine swoosh into the void".
2. **Non-Linear Tape Saturation & Wow/Flutter:**
   Tape head hysteresis and capstan motor irregularity are modeled via a magnetic saturation curve:
   $$v_{tape} = \frac{v}{\sqrt{1 + v^2}} + \epsilon_{\text{flutter}}(t)$$
3. **Quadrature Dual-Phase Regeneration:**
   A feedback path passes through a variable damping lowpass filter and soft limiter before re-entering the modulated line.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`MANUAL / DELAY`):** Static center delay offset ($0.0\,\text{ms} \dots 10.0\,\text{ms}$, Profile B). *Push: Snap Exact Through-Zero Null*.
* **Knob 2 (`SWEEP RATE`):** Flange LFO frequency ($0.02\,\text{Hz} \dots 10.0\,\text{Hz}$, Profile C). *Push: Tempo Sync Toggle*.
* **Knob 3 (`SWEEP DEPTH`):** Modulation amplitude across the zero boundary ($0.0\% \dots 100.0\%$, Profile B). *Push: Max Jet Excursion*.
* **Knob 4 (`RESONANCE / REGEN`):** Bipolar feedback regeneration ($-95.0\% \dots 0.0\% \dots +95.0\%$, Profile B). *Push: Invert Polarity*.
* **Knob 5 (`TAPE TAPE DRIVE`):** Magnetic tape head saturation ($0.0\,\text{dB} \dots +18.0\,\text{dB}$, Profile E). *Push: Clean Digital Bypass*.
* **Knob 6 (`TAPE DAMPING`):** High-frequency tape head loss filter ($2.0\,\text{kHz} \dots 20.0\,\text{kHz}$, Profile D). *Push: Pristine Open*.
* **Knob 7 (`MOTOR WOW & FLUTTER`):** Capstan irregular motor drift ($0.0\% \dots 100.0\%$, Profile E). *Push: Mute Flutter*.
* **Knob 8 (`MIX`):** Universal Tri-Zone Dry/Wet Mix (Profile M: 7:00 Wet Audition, 12:00 Pure Dry, 5:00 Parallel). *Push: Phase Invert ($\varnothing = 180^\circ$)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Delay Architecture):** `[ Dual Reel-to-Reel Tape | Analog BBD (Reticon SAD1024) | Digital Pristine | Vintage Barber-Pole ]`.
* **DIP 2 (Zero Crossing Point):** `[ Dynamic Center | Top Edge | Bottom Edge ]`.
* **DIP 3 (LFO Waveshape):** `[ Exponential Hyper-Sine | Linear Triangle | Dual-Speed Chaos ]`.
* **DIP 4 (Stereo Phasing):** `[ Twin Parallel Monophonic | Quadrature 90° | Antiphase 180° ]`.
* **TRIM 1 (Reference Delay Latency):** Internal fixed latency buffer $\tau_0$ ($1.0\,\text{ms} \dots 20.0\,\text{ms}$).
* **TRIM 2 (Feedback Non-Linearity):** Soft-knee diode clamp in the regeneration loop.
* **TRIM 3 (Tape Hiss Floor):** Authentic reel-to-reel magnetic tape hiss ($-\infty \dots -48\,\text{dB}$).
* **TRIM 4 (Capstan Motor Inertia):** LFO acceleration damping for realistic flywheel momentum.

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Architecture Mode (DIP 1) | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`Tape Reel Mode`** | `Knob 5, 7` | `HEAD DRIVE`, `CAPSTAN WOW` | `0..+18 dB`, `0..100%` | `Profile E`, `Profile E` | Simulates magnetic oxide saturation and motor inertia |
| **`BBD SAD1024`** | `Knob 5, 6` | `CLOCK LEAK`, `ANTI-ALIAS` | `-inf..0 dB`, `1k..12k` | `Profile B`, `Profile D` | Authentic Reticon BBD clock whine and dark bucket roll-off |
| **`Barber-Pole`** | `Knob 2` | `SHEPARDIAN RATE`| `±5.0 Hz` | `Profile B` | Continuous infinite rising or falling barber-pole flange |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Windowed Sinc Delay Interpolation:** Through-zero crossing requires sub-sample time delay accuracy; evaluated using 8-point Kaiser-windowed sinc interpolation.
* **Zero Glitch Invariant:** Modulated pointer smoothly crosses reference pointer without discontinuity clicks or buffer wrap clicks.
