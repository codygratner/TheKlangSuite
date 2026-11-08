# Master Card Architecture, Specification Template & Canonical Control Registry

**Document Status:** CANONICAL REUSABLE COMPONENT STANDARD 📜  
**Target Ecosystem:** The Klang Suite (`TKS`), ToadTracker (`TT`), The Klang Mill (`TKM`), dadamachines TBD-16  
**Version:** 1.0.0 (The Immutable Canon Standard)  
**Authors:** Antigravity x Cody G. (Klang Industries / ToadTracker Audio Division)  
**Date:** 2026-10-18  

---

## 1. Executive Summary & Architectural Invariant

To guarantee absolute consistency across the ecosystem, eliminate menu-diving, and prevent "amnesia regressions" (such as dropping canonical filter models or losing push-actions), every sound engine, audio effect, and modulation voice adheres to the **Universal 16-Control Card Architecture**:

* **8 Front Panel Performance Controls (Controls 1..8)**: Mapped directly to hardware Page 1A (Knobs 1..4) and Page 1B (Knobs 5..8) on the dadamachines TBD-16 and TKS Flip-Cards. Every encoder includes a dedicated **Push Action** (encoder click).
* **8 Rear Deck Controls (Controls 9..16 / Deck ⚙️ / Sound Lab)**: Mapped to 4 Digital DIP switches and 4 Precision Trimpots. Accessible by flipping the card over (or diving into the Sound Lab overlay). Provides deep circuit configuration without cluttering the performance faceplate.
* **The Component Canon ("The Steal Library")**: Standardized controls (Cutoff, Resonance, Envelope Stages, Saturation Drive, Tri-Zone Mix, etc.) are cataloged as reusable modular building blocks with fixed transfer profiles, ranges, and click-actions.

> [!IMPORTANT]
> ### 🎛️ The Universal Partition Principle: Performance (1..8) vs. Settings (9..16)
> Both sides of the card are critical to the sound architecture, but their ergonomic roles must **NEVER** be blurred or inverted:
> 
> 1. **Controls 1..8 MUST Be Strictly Performance-Focused**:
>    * These are the hands-on controls that a performer rides in real time during a live set, jam, tracking session, or automation sweep: Filter Cutoffs, Resonances, Overdrive/Drive heat, Primary Delay Times, Loop Feedback, Operator Ratios/Levels, Pitch/Bends, and Master Tri-Zone Mix.
>    * **The 8-Knob Live Controller Invariant**: A musician on any standard 8-knob controller (e.g. Akai MIDI Mix, Launch Control, Faderfox, or TBD-16 Page 1A+1B) must be able to perform a complete, dynamic live set without **EVER** needing to flip over to the settings deck.
>    * **Control 8 Preference (Tri-Zone Mix)**: Control 8 is **highly preferred** to be the canonical Tri-Zone Law `MIX` knob (`Profile M` with Push Phase Invert). However, this is NOT strictly mandatory: if an effect or sound generator does not require an internal wet/dry blend (e.g. inline master equalizers, summing consoles like IronSum-4, or raw oscillator cores), Control 8 may be assigned to another primary performance parameter.
>    * **Anti-Pattern Warning**: Never strand primary play controls (like delay time, loop feedback, distortion drive, or primary filter sweeps) on Controls 9..16 just to fill space.
> 
> 2. **Controls 9..16 MUST Be Strictly Settings-Focused**:
>    * This is the machine room, circuit architecture, and deep sound lab configuration:
>      * **Flexible Composition Rule**: Controls 9..16 can be **any combination** of digital DIP switches, dropdown selectors, and precision trimpots.
>      * **Canonical Preference**: The default standard prefers **Controls 9..12 as DIP switches / dropdown selectors** and **Controls 13..16 as precision trimpots**.
>      * **Permitted Architectural Flexibility**: If a specific engine requires more switches/selectors (e.g. 6 switches + 2 trimpots) or more trimpots (e.g. 2 selectors + 6 trimpots, or 8 trimpots), any combination is fully permitted provided all 8 remain strictly in the settings/sound lab domain.
>      * **DIP Switches & Dropdown Selectors**: Algorithmic topology selection, circuit models, routing order (pre/post/parallel), quantization laws, and clock/trigger sync modes.
>      * **Precision Trimpots**: Secondary calibration parameters (reverb tail decay, secondary pipe length), dynamic thresholds, saturation ceilings, internal sidechain/loop biases, modulation envelope crossfaders, and analog slop tolerances.

---

## 1.1 Front-Panel Ergonomic Placement Conventions ("The Muscle-Memory Canon")

To build instant tactile muscle memory across hardware surfaces (dadamachines TBD-16, MIDI Mix, ToadTracker Flip-Cards) and eliminate mental friction when switching between engines, the 8 performance encoders adhere to standardized **Placement Conventions**.

> [!IMPORTANT]
> ### 🌊 The Flow Invariant (The Anti-Straitjacket Rule)
> These placement conventions are **canonical preferences**, NOT an unyielding straitjacket.  
> **Rule of Ergonomic Flow**: If strictly following a convention harms the intuitive musical flow of a specific instrument, breaks an indivisible stereo pair (e.g. dual Cutoffs or dual Resonances), or strands a critical performance knob, **the convention can and should be broken**. Simply note the ergonomic design rationale in the card specification.

```
Page 1A (Top Row / Left Hand: Core Generation & Engine Dynamics)
[ Knob 1: Primary Action ]    [ Knob 2: Reaction / Depth ]   [ Knob 3: Secondary Timbre ]   [ Knob 4: Mode / Slope ]
(Cutoff / Time / Rate / Drive) (Res / Feedback / Depth / Fine) (Tone / Damping / Stage 2)     (Algorithm / Slope / Ratio)

Page 1B (Bottom Row / Right Hand: Color, Space & Output Master)
[ Knob 5: Drive / Heat / Sub ] [ Knob 6: Tone / Damping / Tilt ] [ Knob 7: Width / Level ]   [ Knob 8: Tri-Zone Mix ]
(Overdrive / Preamp / Saturate) (High Cut / Damping / Tilt EQ)  (Stereo Spread / Makeup Gain) (Wet/Dry Law + Phase Invert)
```

### The 5 Standard Front-Panel Conventions:

1. **Control 8 &rarr; `MIX` (The Universal Balance Anchor)**:
   * **Convention:** Highly preferred for the universal Tri-Zone Law `MIX` knob (`Profile M` with Push Phase Invert $\varnothing = 180^\circ$).
   * **Scope:** 90%+ of effects cards (24 of 27 cards).
   * **Flow Exception:** Dedicated serial EQs, summing consoles, or raw oscillator voices where an internal wet/dry mix is architecturally irrelevant.

2. **Control 7 &rarr; `STEREO WIDTH` or `OUTPUT LEVEL / MAKEUP` (The Spatial / Gain Anchor)**:
   * **Spatial / Stereo Effects:** Highly preferred for `STEREO WIDTH` or `SPATIAL SPREAD` (`Profile B` with Push Mono Check). Placed directly adjacent to Control 8 (`MIX`), your right hand naturally commands both dimensions of the spatial presentation: Width (Knob 7) and Balance (Knob 8).
   * **Dynamics / Distortion / Non-Spatial Effects:** Highly preferred for `OUTPUT LEVEL` or `MAKEUP GAIN` (`Profile E` with Push 0dB / Bypass), setting post-processing ceiling right before the mix bus.

3. **Controls 1 & 2 &rarr; `THE PRIMARY DUAL AXIS` (Action & Reaction)**:
   * Control 1 is ALWAYS the primary operational axis (the "Where" or "Speed"), and Control 2 is its immediate counterpart or reaction (the "How Much" or "Depth"):
     * **Filters:** `CUTOFF` (Knob 1) &rarr; `RESONANCE` (Knob 2).
     * **Delays:** `DELAY TIME` (Knob 1) &rarr; `FEEDBACK` or `PRE-DELAY` (Knob 2).
     * **Modulations:** `LFO RATE / SPEED` (Knob 1) &rarr; `DEPTH / INTENSITY` (Knob 2).
     * **Dynamics:** `THRESHOLD` (Knob 1) &rarr; `RATIO` or `ATTACK` (Knob 2).
     * **Distortions:** `DRIVE` (Knob 1) &rarr; `FOLD / SHAPE / BIAS` (Knob 2).
     * **Pitch Shifters:** `PITCH / COARSE SHIFT` (Knob 1) &rarr; `FINE DETUNE` (Knob 2).

4. **Control 6 &rarr; `TONE / DAMPING / TILT` (The Wet Sculptor)**:
   * **Convention:** Highly preferred for high-frequency damping, tilt EQ, or lowpass tone filtration on the wet signal path (`Profile D` log or `Profile B`).
   * **Rationale:** Sits naturally between the engine/drive (Knobs 1..5) and the spatial/output stage (Knobs 7..8), allowing the artist to tame brightness, simulate tape/analog bucket loss, or darken reverb tails.

5. **Control 5 &rarr; `DRIVE / HEAT / SATURATION` (The Colorizer)**:
   * **Convention:** On Row 2 / Page 1B (first knob, directly beneath Knob 1), Control 5 is highly preferred for input drive, saturation heat, tape warm-up, or secondary engine excitation when present on non-distortion cards (e.g. `Pre-Drive` on KlangVCF, `Tape Sat` on ZeroFlange, `Argent Burn` on Argent Reactor).

---

## 2. The 16-Control Specification Template

When specifying or implementing any new card, copy and populate this exact template:

```markdown
# Card Specification: [Sovereign Name] ([Corporate Boring Name])
**Engine ID:** [CARD-XX / OSC-XX / FX-XX / MEGA-XX]  
**Sovereign Name:** [e.g. SlatelyFM / Velocitron / IronSum-4 / Vandal Park]  
**Corporate Boring Name:** [e.g. 4-Operator Linear FM Synthesizer / 8-Channel Expressive Velocity Matrix]  
**Category:** [Oscillator | Filter | Dynamics | Modulation | Effect]  
**Pedigree / Lineage:** [Hardware / Tracker inspiration]  
**Architecture:** [Brief 2-line DSP summary]

---

### Front Panel: 8 Performance Encoders (Performance View: 1..8)

> 🎛️ **Performance Focus Invariant**: Controls 1..8 are strictly for hands-on, live performance controls (Cutoffs, Resonance, Overdrive, Delay Time, Feedback, Mix, Expressive Ratios). An artist playing live on an 8-knob controller should never need to flip to Page 2 during a performance!

| # | Parameter ID | Display Label | Control Type | Push / Click Action | Taper Profile | Default | Min / 12:00 / Max | Units & Display | Musical Function |
| :-: | :--- | :--- | :--- | :--- | :--- | :-: | :--- | :--- | :--- |
| **1** | `param_1` | `LABEL 1` | `Continuous` | `None` | `Profile A` | `0.5` | `10ms / 1.5s / 60s`| `ms / s` | Primary timing / core function |
| **2** | `param_2` | `LABEL 2` | `Choice` | `Cycle Mode` | `Linear` | `0` | `0 / 2 / 4` | `Names` | Discrete mode selector |
| **3** | `param_3` | `LABEL 3` | `Continuous` | `Hyper Toggle` | `Profile B` | `0.0` | `-100% / 0% / +100%`| `%` | Bipolar depth with deadzone |
| **4** | `param_4` | `LABEL 4` | `Continuous` | `Fine Tune` | `Profile D` | `800 Hz`| `20Hz / 800Hz / 20kHz`| `Hz / kHz`| Equal-octave frequency sweep |
| **5** | `param_5` | `LABEL 5` | `Continuous` | `Bypass Sat` | `Profile E` | `0 dB` | `0dB / +6dB / +36dB` | `dB` | Warmth-expanded drive |
| **6** | `param_6` | `LABEL 6` | `Continuous` | `Q-Lock` | `Profile F` | `0.7` | `0.5 / 1.5 / 25.0` | `Q` | Hyperbolic resonance / feedback |
| **7** | `param_7` | `LABEL 7` | `Continuous` | `Center Mono` | `Profile B` | `100%` | `0% / 100% / 200%` | `%` | Mid/Side stereo width |
| **8** | `param_8` | `MIX` (or Custom) | `Continuous` | `Phase Invert`| `Profile M` (or Custom) | `0.5` | `100%W / DRY / PAR` | `Mix` | Tri-Zone parallel injection (Highly Preferred; Optional if unneeded) |

> 🎛️ **Control 8 Convention**: Control 8 is **highly preferred** to be the canonical Tri-Zone Law `MIX` knob (`Profile M` with Push Phase Invert $\varnothing = 180^\circ$). However, this is NOT strictly required: if a specific card architecturally has no use for an internal wet/dry mix (e.g. an inline serial EQ, a summing console like IronSum-4, or a dedicated oscillator voice), Control 8 may be assigned to another high-impact performance control.

---

### Contextual Front-Panel Remapping (Dynamic Mode Matrix)
*When a rear-deck DIP switch (or front-panel Mode encoder) fundamentally alters the operational engine of the card, document the contextual remapping of the 8 front encoders here:*

| Active Mode / DIP State | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`Mode A (e.g. Vintage Tape)`** | `Knob 5, Knob 6` | `WOW / FLUTTER`, `TAPE TONE` | `0..100%`, `1kHz..16kHz` | `Profile B`, `Profile D` | Motor speed wobble & oxide loss |
| **`Mode B (e.g. Reverse Delay)`** | `Knob 1, Knob 5` | `BUFFER SIZE`, `CROSSFADE` | `50ms..2000ms`, `5..50ms` | `Profile A`, `Profile B` | Reverse slice size & anti-click slew |
| **`Mode C (e.g. Granular Freeze)`**| `Knob 2, Knob 5` | `FREEZE LATCH`, `GRAIN SPRAY` | `Toggle`, `0..100%` | `Discrete`, `Profile B` | Infinite buffer lock & grain scatter |

---

### Rear Deck ⚙️: 8 Sound Lab Controls (Settings View: 9..16)

> ⚙️ **Settings Focus Invariant**: Controls 9..16 house the machine room, circuit topologies, algorithm selectors, and fine calibrations.
> * **Flexible Composition**: Controls 9..16 may be **any combination** of digital DIP switches, dropdown selectors, and precision trimpots.
> * **Canonical Preference**: The default preference is **Controls 9..12 as DIP switches / dropdown selectors** and **Controls 13..16 as precision trimpots**.
> * **Alternative Valid Combos**: If an engine needs more switches (e.g. 6 switches + 2 trimpots) or more trimpots (e.g. 2 selectors + 6 trimpots, or 8 trimpots), any combination is fully permitted.

#### Standard Layout (4 DIPs/Selectors + 4 Precision Trimpots):

##### Controls 9..12 (Digital DIP Switches / Dropdown Selectors):
* **`DIP 1 / Control 09 (Algorithm / Mode Topology)`**: `[ Option A | Option B | Option C | Option D ]`
* **`DIP 2 / Control 10 (Secondary Routing / Injection Order)`**: `[ Pre-Stage | Post-Stage | Parallel ]`
* **`DIP 3 / Control 11 (Sync Mode / Quantization Behavior)`**: `[ Free Running | Note Reset | Pulse Out ]`
* **`DIP 4 / Control 12 (Circuit Model / Ceiling Law)`**: `[ Mode A | Mode B | Mode C | Mode D ]`

##### Controls 13..16 (Precision Trimpots):
* **`TRIM 1 / Control 13 (Sensitivity / Depth Floor / Cross-Mod)`**: `[ Range / Units ]` — Sets lower limit, bias, or dynamic threshold.
* **`TRIM 2 / Control 14 (Ceiling / Compression Knee / Cross-Mod)`**: `[ Range / Units ]` — Sets upper limit or saturation ceiling.
* **`TRIM 3 / Control 15 (Secondary Decay / Bleed / Dry-Wet Mix)`**: `[ Range / Units ]` — Secondary timing parameter or internal blend.
* **`TRIM 4 / Control 16 (Modulation Speed / Analog Slop Margin)`**: `[ Range / Units ]` — Modulation rate or circuit drift margin.
```

---

## 3. The Master Canon Component Registry ("The Steal Library")

These standardized controls have been mathematically perfected, calibrated to human perception, and tested. **When building new cards, reuse these exact control definitions verbatim.**

### 3.1 Frequency & Filtering Controls

#### `CTRL-FLT-CUTOFF` (Universal Resonant Filter Cutoff)
* **ID:** `<prefix>_cutoff` | **Label:** `CUTOFF`
* **Type:** Continuous | **Taper Profile:** **Profile D** (Equal-Octave Log)
* **Range:** $20.0\,\text{Hz} \dots 20,000.0\,\text{Hz}$ | **Default:** $800.0\,\text{Hz}$ ($12\text{ o'clock}$)
* **Push Action:** Toggles **`HYPER-TRACK`** ($\pm 400\%$ keytrack mode) or **`BAND AUDITION`**.
* **Formula:** $F(x) = 20.0 \cdot (1000.0)^x$
* **Musical Behavior:** Exactly equal degrees of physical encoder rotation per musical octave.

#### `CTRL-FLT-RES` (Universal Filter Resonance & Q)
* **ID:** `<prefix>_resonance` | **Label:** `RESONANCE` (or `Q`)
* **Type:** Continuous | **Taper Profile:** **Profile F** (Hyperbolic End-Stop)
* **Range:** $0.5 \dots 25.0$ (or $0\% \dots 115\%$ self-oscillation) | **Default:** $0.707$ (Butterworth Flat)
* **Push Action:** Locks Resonance at pure self-oscillation ($Q = 20.0$) as an acoustic sine oscillator.
* **Formula:** $Q(x) = 0.5 + 24.5 \cdot \frac{1 - e^{-3x}}{1 - e^{-3}}$
* **Musical Behavior:** Expands physical resolution in the $70\% \dots 95\%$ singing formant sweet-spot.

#### `CTRL-FLT-TYPE` (The 11 Canon Filter Models — NEVER REDUCE TO 6!)
* **ID:** `<prefix>_filter_type` | **Label:** `TYPE`
* **Type:** Discrete Choice (`0..10`) | **Push Action:** Cycles Filter Category (*Classic &rarr; Acid/Aggro &rarr; Exotic*)
* **The 11 Immutable Canon Models:**
  1. `0: Clean SVF` (Zero-delay feedback Cytomic state-variable filter; pristine & neutral).
  2. `1: Moog Ladder 4-Pole` (Transistor ladder with authentic bass drop at high resonance).
  3. `2: Korg Screamer 35` (Sallen-Key topology with aggressive diode-clipping feedback).
  4. `3: 303 Diode Acid` (18 dB/oct 3-pole diode ladder with non-linear resonance sag).
  5. `4: Hornet Wasp CMOS` (CMOS inverter distortion filter with dirty, gritty bandpass).
  6. `5: Vandal Park` (Steiner-Parker Synthacon multimode diode filter with screaming resonance peaks).
  7. `6: Sherman Warbank` (Belgian industrial dual-saturation filter with extreme intermodulation).
  8. `7: Alpha Dual-12` (Roland Alpha Juno / MKS-50 cascading 2-pole resonant lowpass).
  9. `8: Human Vowel Formant` (Parallel 3-band formant filter morphing across $A-E-I-O-U$).
  10. `9: DJ Performance Isolator` (Steep $24\,\text{dB/oct}$ lowpass/highpass sweep with center kill).
  11. `10: Tuned Comb Filter` (Short delay resonator tuned to chromatic pitch for physical modeling).

#### `CTRL-FLT-SLOPE` (Filter Attenuation Slope)
* **ID:** `<prefix>_slope` | **Label:** `SLOPE`
* **Type:** Discrete Choice (`0..4`) | **Push Action:** Toggles Pole Coupling
* **Choices:** `6 dB/oct (1-Pole)` &bull; `12 dB/oct (2-Pole)` &bull; `18 dB/oct (3-Pole)` &bull; `24 dB/oct (4-Pole)` &bull; `36 dB/oct (6-Pole Extreme)`.

---

### 3.2 Universal Envelope Controls (`BACKLOG-11`)

#### `CTRL-ENV-TYPE` (Universal 4-Mode Envelope Selector)
* **ID:** `<prefix>_type` | **Label:** `TYPE`
* **Type:** Discrete Choice (`0..3`) | **Push Action:** Resets stage timings to classic 808/909 defaults
* **The 4 Immutable Canon Modes:**
  * `0: DRUM PUNCH` (M8 empirical lineage; pure instant attack + exponential decay; no sustain clutter).
  * `1: AHDSR (Elektron)` (Analog Rytm/Digitakt note-gate sync with dedicated pre-decay `HOLD` stage).
  * `2: CYCLING LFO / SLOPE` (Make Noise Maths / Serge DUSG looping rise/fall slew function).
  * `3: TABLE MSEG / MML` (Deflemask & tracker micro-stepper with sustain loop points & release markers).

#### `CTRL-ENV-TARGET` (Universal 4-Way Target Matrix)
* **ID:** `<prefix>_target` | **Label:** `TARGET`
* **Type:** Discrete Choice (`0..3`) | **Push Action:** Inverts target polarity
* **Choices:**
  * `0: Core A` (Carrier, Line 1, Tank 1, Cutoff, Level).
  * `1: Core B` (Modulator, Line 2, Tank 2, Resonance, VCA Drive).
  * `2: Both (A & B)` (In-phase parallel modulation).
  * `3: Opp (+A & -B)` (**Opposite Acoustic Bloom!** One sweeps up while the other dives down).

#### `CTRL-ENV-TIME` (Universal Envelope Temporal Stage: Attack, Decay, Release)
* **ID:** `<prefix>_<stage>` | **Label:** `ATTACK` / `DECAY` / `RELEASE`
* **Type:** Continuous | **Taper Profile:** **Profile A** (Cubic $y = x^3$)
* **Range:** $0.010\,\text{s} \dots 60.0\,\text{s}$ ($10\,\text{ms} \dots 60\,\text{s}$) | **12:00 Value:** $1.5\,\text{s}$
* **Push Action:** Snaps to Tempo Division ($1/64 \dots 2\text{ bars}$) or resets to instant $10\,\text{ms}$.
* **Musical Behavior:** 50% of the physical encoder travel is dedicated to punchy percussion ($10\,\text{ms} \dots 1.5\,\text{s}$).

#### `CTRL-ENV-DEPTH` (Universal Bipolar Modulation Depth)
* **ID:** `<prefix>_depth` | **Label:** `DEPTH`
* **Type:** Continuous Bipolar | **Taper Profile:** **Profile B** (Sigmoidal S-Curve)
* **Range:** $-100.0\% \dots +100.0\%$ | **Default / 12:00:** $0.0\%$ (Deadzone Plateau)
* **Push Action:** Inverts polarity ($\pm 1.0$) or resets to pure neutral $0.0\%$.

#### `CTRL-ENV-CURVE` (Envelope Tension & Skew)
* **ID:** `<prefix>_curve` | **Label:** `CURVE` (or `SKEW`)
* **Type:** Continuous Bipolar | **Taper Profile:** **Profile B** (Sigmoidal S-Curve)
* **Range:** $-100\% \dots 0\% \dots +100\%$ (*Logarithmic Punch* $\leftrightarrow$ *Linear* $\leftrightarrow$ *Exponential Swell*).
* **Push Action:** Resets to perfect Linear ($0\%$).

---

### 3.3 Dynamics, Saturation & Spatial Controls

#### `CTRL-AMP-DRIVE` (Universal Analog Warmth & Fuzz Drive)
* **ID:** `<prefix>_drive` | **Label:** `DRIVE` (or `WARMTH`)
* **Type:** Continuous | **Taper Profile:** **Profile E** (Warmth Expansion $x^{1.8}$)
* **Range:** $0.0\,\text{dB} \dots +36.0\,\text{dB}$ | **12:00 Value:** $+6.0\,\text{dB}$ (Console Tape Warmth)
* **Push Action:** Toggles Saturation Type (*Tape Warmth* $\leftrightarrow$ *Germanium Fuzz*).
* **Musical Behavior:** First 60% of travel provides delicate 2nd-harmonic glue; final 40% digs into aggressive fuzz.

#### `CTRL-STEREO-WIDTH` (Mid/Side Stereo Spatial Spreader)
* **ID:** `<prefix>_width` | **Label:** `WIDTH` (or `SPREAD`)
* **Type:** Continuous | **Taper Profile:** **Profile B** (Center Plateau at 100%)
* **Range:** $0.0\% \dots 200.0\%$ | **12:00 Value:** $100.0\%$ (Natural Stereo)
* **Push Action:** Instantly collapses to $0.0\%$ (Mono Check) or toggles to $200\%$ (Super-Wide).

#### `CTRL-MIX-PARALLEL` (Universal Tri-Zone Dry/Wet Mix Law)
* **ID:** `<prefix>_mix` | **Label:** `MIX`
* **Type:** Continuous Tri-Zone | **Taper Profile:** **Profile M** (Tri-Zone Law)
* **Layout:** $7\text{ o'clock} = 100\%\text{ Wet (Audition)}$ &bull; $12\text{ o'clock} = \text{Pure Dry}$ &bull; $5\text{ o'clock} = \text{Parallel (100\% Dry + 100\% Wet)}$.
* **Push Action:** **Phase Invert ($\varnothing = 180^\circ$)!** Flips wet polarity for through-zero tube flanging or distortion transient subtraction.
* **Architectural Status:** **Highly Preferred standard for Control 8** across audio effects, but optional if the card design does not utilize an internal wet/dry blend (e.g. serial EQs, dedicated summing consoles, or oscillator sources).

---

### 3.4 Expressive Modulation & Performance Controls

#### `CTRL-MOD-SPEED` (Dual-Regime LFO / Audio-Rate Modulator)
* **ID:** `<prefix>_speed` | **Label:** `SPEED` (or `RATE`)
* **Type:** Continuous | **Taper Profile:** **Profile C** (Dual-Regime Piecewise Log)
* **Range:** $0.02\,\text{Hz} \dots 5,000.0\,\text{Hz}$ | **12:00 Value:** Exactly $20.0\,\text{Hz}$ (Acoustic Boundary)
* **Push Action:** Snaps to Tempo Sync ($1/32 \dots 8\text{ bars}$) vs Free Hertz.

#### `CTRL-KEYTRACK-AMT` (Acoustic Key Tracking with Hyper-Track)
* **ID:** `<prefix>_keytrack` | **Label:** `KEYTRACK`
* **Type:** Continuous Bipolar | **Taper Profile:** **Profile B**
* **Range:** $-100.0\% \dots 0.0\% \dots +100.0\%$
* **Push Action:** **Toggles `HYPER-TRACK (±400%)`** for extreme negative/positive spectral inversions!

#### `CTRL-SLOP-CHANNEL` (Per-Channel Analog Drift & Sag)
* **ID:** `slop_chan_<n>` | **Label:** `SLOP <N>`
* **Type:** Continuous Bipolar with 3-Way Switch | **Taper Profile:** **Profile B**
* **Range:** $-100.0\% \dots +100.0\%$
* **Push Action (or 3-Way Slider):**
  * `+/-`: Bipolar thermal random walk.
  * `+`: Unipolar upward drift.
  * `-`: Unipolar voltage sag (simulates power-supply droop under heavy bass load).

---

## 4. Canonical Roster of Locked 16-Control Cards

| Card ID | Sovereign Moniker | Corporate Boring Name | Front 8 Encoders | Rear Deck ⚙️ (4 DIPs + 4 Trims) |
| :---: | :--- | :--- | :--- | :--- |
| **`CARD-01`** | [**`SlatelyFM`**](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/slately_fm_dsp.md) | **`4-Operator Linear FM Synthesizer`** | Ratio A..D (Page 1A), Level A..D (Page 1B) (Push: Integer Snap / Mute) | 12 Algos (Hat/Crash/Cascade), Waveform Palette for All, Keytrack/Hz, Op A/D Feedback Trims |
| **`CARD-02A`**| [**`KlangVCF`**](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/klangvcf_dsp.md) · [**`Vandal Park`**](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/vandal_park_dsp.md) | **`11-Model Resonant Filter Lab`** | Cutoff, Resonance, **11 Filter Types** (incl. Vandal Park), Slope, Pre-Drive, Keytrack (±400%), Pre-HPF Clean, Stereo Spread | Audio FM Depth, Bass Drop vs Comp, Analog Slop, Saturation Bias |
| **`CARD-02B`**| [**`FilterEnv`**](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/filterenv_dsp.md) | **`Universal 4-Mode Filter Envelope`** | Type (Punch/AHDSR/Slope/MSEG), Target (Cut/Res/Both/Opp), Attack, Hold, Decay, Sustain, Release, Bipolar Depth | Retrig 0V vs Legato Catch, Gate vs One-Shot, MML Quantize, EOF Trigger Out |
| **`CARD-03A`**| [**`AmpComp`**](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/ampcomp_dsp.md) | **`Master VCA & Dynamics Processor`** | Level, Pan, VCA Drive, Stereo Width, Comp Thresh, Comp Ratio, Punch Macro, Ceiling/Makeup | Comp Topologies (SSL, 1176, LA2A, Nuke), Sidechain HPF, Saturation Mode, NY Comp Trim |
| **`CARD-03B`**| [**`AmpEnv`**](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/ampenv_dsp.md) | **`Universal 4-Mode Amp Envelope`** | Type, Target (**Level / VCA Drive / Both / Opp**), Attack, Hold/Claps, Decay, Sustain, Release/Spread, Bipolar Depth | Retrig vs Legato, One-Shot Drum vs Gate, Flam Decay Damp Trim, Anti-Click Floor Trim |
| **`CARD-04`** | [**`PitchEnv`**](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/pitchenv_dsp.md) | **`Universal 4-Mode Pitch Envelope`** | Type, Target (**Car / Mod / Both / Opp Acoustic Bloom**), Attack, Hold, Decay, Sustain, Release, Bipolar Depth | Semitone Snap vs Microtonal, Retrig vs Legato, Keytrack Scale Trim, Invert Phase Push |
| **`CARD-05A`**| [**`Velocitron`**](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/velocitron_dsp.md) | **`8-Channel Expressive Velocity Matrix`**| Vel Curve/Sens, -> Amp Level, -> Cutoff, -> FM Timbre, -> Decay, -> Attack, -> Pitch Snap, -> VCA Drive | Linear vs MPC Curve, Humanize Jitter %, Ghost Note Mode, Velocity Floor/Ceiling Trims |
| **`CARD-05B`**| [**`Chromatron`**](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/chromatron_dsp.md) | **`Chromatic Key & Scale Tracking Matrix`**| Pivot/Center Note, -> Cutoff (±400%), -> Decay Damp, -> Tilt EQ, -> FM Timbre, -> Pan Spread, -> Attack, -> Pitch Stretch | Hyper-Track Range, 1V/Oct vs Quarter-Tone, Floating Pivot, Anti-Alias Ceiling Trim |
| **`CARD-06`** | [**`SlopLab`**](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/sloplab_dsp.md) | **`8-Channel Analog Drift & Sag Matrix`**| 8 Assignable Amount Knobs (Push: 3-Way Polarity `+`, `+/-`, `-`, Shift: Scroll Target) | Top Deck: 8 Destination Pickers; Bottom: Drift Physics (Perlin/Drunk/S&H), Temp Model, Speed/Sag Trims |
| **`CARD-07A`**| [**`Flint & Dust`**](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/flint_and_dust_dsp.md) | **`Transient Strike & Noise Engine`** | Trans Pitch/Decay/Level/Model (Page 1A), Noise Color/Cutoff/Decay/Level (Page 1B) | Filter Pass Mode (BP/HP/LP), Env Mode (1-Shot/Gate/Bed), Stereo Width, Punch Ducking, Q Trim |
| **`CARD-07B`**| [**`ReFont`**](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/refont_dsp.md) | **`Acoustic Sample Player`** | Sample Select/Pitch/Start/Decay (Page 1A), Attack/Crush/Cutoff/Loop (Page 1B) | Renoise .xrni/.sf2/SCW, Drop/Linear/Sinc, NNA Cut/Overlap, Velocity Map |
| **`CARD-08`** | [**`IronSum-4`**](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/ironsum4_dsp.md) | **`4-Channel Analog Summing Console`** | Core A, Core B, Noise, Sample Levels (Page 1A) & Pans (Page 1B) | Bus Topology (Active/Passive/Tube/Ring), Pan Law, Phase Invert, VCF Split Routing |
| **`CARD-09`** | [**`Argent Anvil`**](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/argent_anvil_dsp.md) / `KlangPunch` | **`Master Dynamics & Limiter Lab`** | Thresh/Drive, Ratio/Squash, Attack/XOver-L, Release/XOver-H, SC-HPF/Gate, Drive/Crush, Ceiling/Makeup, Tri-Zone Mix (Push: Phase Invert) | Engine Select (Punch vs Anvil), Topology/Bands, Detector/Curve, Lookahead/ISP, 4 Precision Dynamics Trims |

---

## 5. Verification & Acceptance Invariant
Whenever a new card is designed or reviewed:
1. Verify it implements **exactly 8 front encoders** and **8 rear deck controls**.
2. Verify all continuous knobs reference an explicit **Profile A..F, M, or H** from [`knob_curves_and_tapering_guide.md`](file:///c:/Dev/TheKlangSuite/docs/specs/knob_curves_and_tapering_guide.md).
3. Verify every encoder specifies a meaningful **Push Action**.
4. Confirm that canonical types (such as the 11 Filter Models) are never truncated or omitted.
5. **Strict Ergonomic Partitioning & Flexible Page 2**: Verify that Controls 1..8 pass the "8-Knob Live Performance Test" (zero paging required for live sets) and Controls 9..16 are dedicated strictly to settings (any combination of DIP switches, dropdown selectors, and precision trimpots; canonically preferring 9..12 as switches/selectors and 13..16 as precision trimpots).
6. **Control 8 Mix Preference**: Verify Control 8 defaults to the canonical Tri-Zone Law `MIX` knob (`Profile M`), or provides an explicit architectural rationale for why an internal wet/dry blend is not utilized (e.g. inline master equalizers, summing consoles like IronSum-4, or raw oscillator sources).
7. **Ergonomic Placement Flow Check**: Verify that Controls 1..7 adhere to the Muscle-Memory Placement Conventions (Dual Axis on 1..2, Heat on 5, Tone/Damping on 6, Stereo Width / Output on 7), or explicitly document the ergonomic rationale if a convention was broken to preserve intuitive musical flow.
