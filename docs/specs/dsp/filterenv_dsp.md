# Technical DSP Specification: `FilterEnv` (Universal 4-Mode Filter Envelope)

**Engine ID:** `CARD-02B`  
**Sovereign Name:** `FilterEnv`  
**Corporate Boring Name:** `Universal 4-Mode Filter Modulation Envelope`  
**Category:** Expressive Modulation & Envelope Generator  
**APVTS Prefix:** `fenv_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section D](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Minimoog Model D envelope contours, Roland Juno-106 analog EG, Sequential Prophet-5 envelope generators, Buchla 281 Quad Function Generator.
* **Upstream Offline Reference:** [m8_shortcut_guide.md](file:///C:/Dev/Research/library/manuals/m8_shortcut_guide.md) · [01_renoise_complete_offline_guide.md](file:///C:/Dev/Research/library/01_renoise_complete_offline_guide.md).
* **C++ Implementation Header:** [`FilterEnvelopeGenerator.h`](file:///c:/Dev/TheKlangDSP/include/dsp/envelopes/FilterEnvelopeGenerator.h).

---

## 2. Mathematical Theory & DSP Architecture

FilterEnv generates mathematically continuous exponential, logarithmic, and multi-stage curves with sub-millisecond precision:
1. **The 4 Canonical Envelope Topologies:**
   - **Mode 1 (Analog AHDSR):** Attack, Hold (punch snap), Decay, Sustain, Release with analog RC exponential charge/discharge curves:
     $$v(t) = v_{target} + (v(0) - v_{target}) e^{-t / \tau}$$
   - **Mode 2 (Snappy Drum Punch):** Dedicated single-shot percussive decay with instant zero-attack snap and variable hyperbolic curve tension.
   - **Mode 3 (Buchla Function / Slope):** Looping or gated Rise/Fall generator with continuous curve skew between logarithmic, linear, and exponential.
   - **Mode 4 (Multi-Segment MSEG):** 8-breakpoint tempo-synced arbitrary modulation spline.
2. **Dual-Target Routing & Bipolar Depth:**
   Routes simultaneously or inversely to **Cutoff**, **Resonance**, or **Both**, allowing acoustic blooming where cutoff closes as resonance rises.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`ENV TYPE`):** Discrete choice: `Analog AHDSR`, `Drum Punch`, `Buchla Slope`, `Multi-Stage MSEG`. Discrete Choice. *Push: Next Type*.
* **Knob 2 (`MOD TARGET`):** Routing target: `Cutoff Only`, `Resonance Only`, `Both (Co-Mod)`, `Opposite (Cut Down / Res Up)`. Discrete Choice. *Push: Cycle Target*.
* **Knob 3 (`ATTACK`):** Onset duration ($0.1\,\text{ms} \dots 10.0\,\text{s}$, Profile A Log). *Push: Instant Snap (0.1ms)*.
* **Knob 4 (`HOLD / PUNCH`):** Peak level hold plateau before decay engages ($0.0\,\text{ms} \dots 500.0\,\text{ms}$, Profile A). *Push: Reset Zero*.
* **Knob 5 (`DECAY`):** Decay time down to sustain level ($1.0\,\text{ms} \dots 20.0\,\text{s}$, Profile A). *Push: Tempo Sync*.
* **Knob 6 (`SUSTAIN`):** Resting level during gate hold ($0.0\% \dots 100.0\%$, Profile B). *Push: Snap 50%*.
* **Knob 7 (`RELEASE`):** Gain recovery time after note-off ($1.0\,\text{ms} \dots 20.0\,\text{s}$, Profile A). *Push: Snap Release to Decay*.
* **Knob 8 (`BIPOLAR DEPTH`):** Master modulation amplitude ($-100.0\% \dots 0.0\% \dots +100.0\%$, Profile B Bipolar). *Push: Invert Polarity ($\pm 1.0$)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Retrigger Behavior):** `[ Retrig from 0V (Aggressive Hard Snap) | Legato Catch (Smooth Analog Ride) ]`.
* **DIP 2 (Gate Mode):** `[ Standard Note Gate | One-Shot Percussion Trigger | Looping LFO Function ]`.
* **DIP 3 (Curve Tension):** `[ Exponential Analog (Moog) | Linear Musical | Logarithmic Snappy ]`.
* **DIP 4 (EOF Trigger Out):** `[ End-of-Fall Pulse Active | End-of-Attack Pulse | Off ]`.
* **TRIM 1 (Curve Skew Trim):** Fine continuous tension bias between punchy log and swelling exponential.
* **TRIM 2 (Anti-Click Floor):** Minimum rise/fall clamp to eliminate DC thumps ($0.05\,\text{ms} \dots 2.0\,\text{ms}$).
* **TRIM 3 (Velocity Depth Scaling):** Sensitivity of envelope depth to incoming MIDI note velocity.
* **TRIM 4 (Keytrack Decay Rate):** Shortens envelope decay times at higher keyboard pitches.

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Mode (Knob 1) | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`Drum Punch`** | `Knobs 3, 4` | `CLICK TRANSIENT`, `PUNCH DECAY` | `0..100%`, `5..1500ms` | `Profile B`, `Profile A` | Direct transient punch controls for kicks and snares |
| **`Buchla Slope`**| `Knobs 3, 5` | `RISE TIME`, `FALL TIME` | `1ms..20s`, `1ms..20s` | `Profile A`, `Profile A` | Standard dual function generator nomenclature |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **State Machine Invariant:** Zero branches in sample calculation loop; state transitions (Attack $\to$ Hold $\to$ Decay) evaluated via threshold comparisons.
* **Subnormal Protection:** Exponential decays naturally clamped to zero when amplitude drops below $10^{-6}$.
