# The Immutable Sound & Effect Canon: 16-Control Specifications for All Oscillators & Effects

**Document Status:** MASTER ECOSYSTEM CANONICAL SPECIFICATION 📜  
**Target Projects:** The Klang Suite (`TKS`), ToadTracker (`TT`), The Klang Mill (`TKM`), dadamachines TBD-16  
**Ecosystem Standard:** Universal 16-Control Architecture (8 Front Performance + 8 Rear Deck ⚙️)  
**Authors:** Antigravity x Cody G. (Klang Industries / ToadTracker Audio Division)  
**Date:** 2026-10-18  

---

## 1. Architectural Contract & Invariant

Every synthesizer oscillator engine (`SoundEngine8`) and audio effect processor (`AudioEffect8`) implements the **Universal 16-Control Architecture**:
1. **8 Front Performance Encoders (Performance View: 1..8)**: Direct hardware mapping to Page 1A (Knobs 1..4) and Page 1B (Knobs 5..8) on the dadamachines TBD-16 and TKS Flip-Cards. MUST be strictly performance-focused (Cutoffs, Resonance, Overdrive, Delay Time, Feedback, Mix, Expressive Ratios) so an artist playing an 8-knob controller never needs to flip to Page 2 during a live set. Control 8 is **highly preferred** to be the canonical Tri-Zone Law `MIX` knob (`Profile M` with Push Phase Invert), but is optional if an engine does not utilize an internal wet/dry mix (e.g. inline master EQs, summing consoles, or raw oscillators). Each encoder has an intentional **Push Action** and references a perceptual curve profile (**Profiles A..F, M, H**) from [`knob_curves_and_tapering_guide.md`](file:///c:/Dev/TheKlangSuite/docs/specs/knob_curves_and_tapering_guide.md).
2. **8 Rear Deck Controls (Settings View: 9..16 / Sound Lab / Deck ⚙️)**: Accessible by flipping the card over. Dedicated strictly to the machine room and settings side of things. Controls 9..16 may be **any combination** of digital DIP switches, dropdown selectors, and precision trimpots (canonically preferring Controls 9..12 as switches/selectors and Controls 13..16 as precision trimpots). Any combination is permitted as long as all 8 remain firmly in the settings/sound lab domain.

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

# PART I: THE OSCILLATOR CANON (`OSC-01` .. `OSC-12`)

---

### `OSC-01`: `SlatelyFM` (4-Op Linear FM Synthesizer Core)
* **Deep DSP Spec:** [`slately_fm_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/slately_fm_dsp.md)
* **Upstream Reference:** [`m8_shortcut_guide.md`](file:///C:/Dev/Research/library/manuals/m8_shortcut_guide.md) · [`ROSETTA_STONE.md#section-b`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-b)
* **Pedigree & Lineage:** Dirtywave M8 `FMSYNTH`, Yamaha TX81Z (LatelyBass / SolidBass), and the 4-Op percussive cluster lineage.
* **Architecture:** 4-operator linear phase modulation engine with M8-style algorithm matrix, global waveform palettes, and dedicated hardware Ratio/Level performance encoders across TBD-16 Page 1A and Page 1B.
* **Front 8 Encoders:**
  1. `RATIO A`: Operator A frequency ratio ($0.25 \dots 32.00$). Profile B. *Push: Snap Integer vs Fine Inharmonic Fractional (e.g. 08.45)*.
  2. `RATIO B`: Operator B frequency ratio ($0.25 \dots 32.00$). Profile B. *Push: Snap Integer vs Fine Inharmonic Fractional (e.g. 01.00)*.
  3. `RATIO C`: Operator C frequency ratio ($0.25 \dots 32.00$). Profile B. *Push: Snap Integer vs Fine Inharmonic Fractional (e.g. 03.31)*.
  4. `RATIO D`: Operator D frequency ratio ($0.25 \dots 32.00$). Profile B. *Push: Snap Integer vs Fine Inharmonic Fractional (e.g. 05.46)*.
  5. `LEVEL A`: Operator A output level / mod index ($0.0\% \dots 100.0\%$, $00 \dots FF$). Profile B / E. *Push: Mute / Solo Operator A*.
  6. `LEVEL B`: Operator B output level / mod index ($0.0\% \dots 100.0\%$). Profile B / E. *Push: Mute / Solo Operator B*.
  7. `LEVEL C`: Operator C output level / mod index ($0.0\% \dots 100.0\%$). Profile B / E. *Push: Mute / Solo Operator C*.
  8. `LEVEL D`: Operator D output level / mod index ($0.0\% \dots 100.0\%$). Profile B / E. *Push: Mute / Solo Operator D*.
* **Contextual Front-Panel Remapping (Dynamic Mode Matrix):**
  | Active Algorithm / Sub-Mode | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
  | :--- | :--- | :--- | :--- | :--- | :--- |
  | **`10: 2-Op R-1 / KlangCore`** | `Knobs 3, 4, 7, 8` | `MOD SHAPE`, `MOD DECAY`, `FEEDBACK`, `AIR TILT` | `Discrete`, `10ms..20s`, `0..100%`, `±100%` | `Discrete`, `Profile A`, `Profile F`, `Profile B` | Collapses operators C/D into direct 2-Op drum controls |
  | **`04: Hat Sizzle (A+B+C->D)`**| `Knob 4` | `CARRIER D PITCH` | `±48 st` | `Profile B` | Direct tuning of the body strike carrier |
  | **`08: Crash Shimmer`** | `Knob 1` | `STRIKE A MOD` | `0.25..32.00` | `Profile B` | Primary transient strike modulator fanning to B, C, D |
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (12 Canon FM Algorithms)`: `[ 01: Cascade A▶B▶C▶D | 02: Dual 2-Op (A▶B)+(C▶D) | 03: Branch A▶(B+C+D) | 04: Hat Sizzle (A+B+C)▶D | 08: Crash Shimmer (A▶B)+(A▶C)+(A▶D) | 09: Cross-Feedback Ring | 10: 2-Op R-1 Core | 11: All-Parallel Organ ]`
  * `DIP 2 (Global Waveform Palette / Shape for All)`: `[ All Pure Sine (DX7 Clean) | TX81Z Vintage 8-Waves | Carrier Square + Mod Sine (M8 Cymbal Hybrid) | All Square Chiptune | All Sawtooth ]`
  * `DIP 3 (Operator Freq Mode)`: `[ 1V/Oct Keytrack All | Fixed Drum Hz All | Fixed Op A + Tracked B/C/D ]`
  * `DIP 4 (Phase Reset)`: `[ Hard Reset 0° Punch Snap | Free-Running DCO | Random Phase Scatter ]`
  * `TRIM 1 (Op A Feedback)`: $0 \dots 100\%$ self-feedback loop drive on Operator A.
  * `TRIM 2 (Op D / Cross-Feedback)`: $0 \dots 100\%$ second feedback loop or carrier saturation.
  * `TRIM 3 (Inharmonic Detune Spread)`: Micro-drift detune spread across background modulators.
  * `TRIM 4 (Velocity Sensitivity)`: Dynamic velocity scaling to modulator levels.

---

### `OSC-02`: `AlphaKraft` (Dual DCO & Unison Synthesizer)
* **Deep DSP Spec:** [alphakraft_dsp.md](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/aalphakraft_dsp.md)
* **Upstream Reference:** [alpha_juno_manual.md](file:///C:/Dev/Research/library/manuals/aalpha_juno_manual.md) · [ROSETTA_STONE.md#section-b](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-b)
* **Architecture:** 14 Alpha Juno band-limited DCO waves (`PolyBLEP::alphaWave14`), 7-voice detuned Supersaw stack, sub-divider, and stereo BBD chorus.
* **Front 8 Encoders:**
  1. `PITCH`: $20\text{ Hz} \dots 4000\text{ Hz}$. Profile D Log. *Push: Octave Transpose*.
  2. `WAVE 14`: Discrete `0..13` (Off, Saw, Comb Saw 1..2, Pulse, Narrow, Sub Pulse, Octaves). *Push: Next Wave*.
  3. `PWM / SPEED`: Pulse width duty cycle ($0.08 \dots 0.92$) & LFO speed ($0.2\text{ Hz} \dots 12\text{ Hz}$). Profile B. *Push: Manual PWM vs LFO*.
  4. `SUB LEVEL`: Level of $-1\text{ oct} / -2\text{ oct}$ sub oscillator. Profile B. *Push: Toggle -1 Oct / -2 Oct*.
  5. `HOOVER DETUNE`: 7-voice Supersaw satellite detune spread. Profile B. *Push: Mono Center vs Wide Stack*.
  6. `HPF / CUTOFF`: 2-pole ZDF filter ($20\text{ Hz} \dots 16\text{ kHz}$). Profile D Log. *Push: HPF Bass Clean vs Full Bypass*.
  7. `ENV DECAY`: $10\text{ ms} \dots 4.0\text{ s}$ decay. Profile A. *Push: Tempo Sync*.
  8. `CHORUS RATE`: Stereo Juno-60 / Alpha Juno BBD Chorus depth & swept clock rate. Profile C. *Push: Chorus I / Chorus II / Off*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (DCO Mode)`: `[ Roland MKS-50 Single | JP-8000 7-Voice Supersaw | Dual DCO Sync ]`
  * `DIP 2 (Sub Wave)`: `[ Square 50% | Pulse 25% | Narrow 12.5% | Stepped Staircase ]`
  * `DIP 3 (Chorus Circuit)`: `[ Juno-60 MN3009 Warm | Alpha Juno Clean BBD | Dimension D Spatial ]`
  * `DIP 4 (Keytrack Tracking)`: `[ 1V/Oct Clean | Micro-Detune 12-Tone | Vintage Temp Drift ]`
  * `TRIM 1`: BBD Bucket Noise Floor ($-\infty \dots -48\text{ dB}$).
  * `TRIM 2`: Sub-Oscillator Phase Offset ($0^\circ \dots 180^\circ$).
  * `TRIM 3`: Supersaw Stereo Pan Spread.
  * `TRIM 4`: Hoover Saturation Ceiling.

---

### `OSC-03`: `CosmoPhase` (Phase Distortion Synthesizer)
* **Deep DSP Spec:** [cosmophase_dsp.md](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/cosmophase_dsp.md)
* **Upstream Reference:** [cz101_manual.md](file:///C:/Dev/Research/library/manuals/cz101_manual.md) · [ROSETTA_STONE.md#section-b](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-b)
* **Architecture:** Casio CZ-101 / CZ-1 twin-line cosine angle warping with 8 waveforms and 7 line interaction modes.
* **Front 8 Encoders:**
  1. `PITCH`: Master fundamental phase frequency. Profile D. *Push: Quantize Chromatic*.
  2. `WAVE 1`: Line 1 waveform (Saw, Square, Pulse, Double Sine, Saw-Pulse, Res 1..3). Discrete `0..7`. *Push: Next Wave*.
  3. `WAVE 2`: Line 2 waveform. Discrete `0..7`. *Push: Next Wave*.
  4. `DCW 1`: Line 1 phase distortion depth / resonant multiplier ($1\times \dots 16\times$). Profile E. *Push: Max Squelch*.
  5. `DCW 2`: Line 2 phase distortion depth. Profile E. *Push: Max Squelch*.
  6. `LINE MODE`: Discrete `0..6` (`Line 1`, `Line 2`, `Mix 1+2`, `Detune 1+2`, `RingMod 1x2`, `NoiseMod`, `DoubleRing`). *Push: Next Mode*.
  7. `DETUNE`: Bipolar pitch offset between Line 1 and Line 2 ($-12\text{ st} \dots +12\text{ st}$). Profile B. *Push: Reset Detune to 0*.
  8. `DCW DECAY`: $5\text{ ms} \dots 4.0\text{ s}$ decay of the internal DCW envelope. Profile A. *Push: Tempo Snap*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (DAC Emulation)`: `[ Authentic 12-Bit CZ-101 Floating-Point | Modern 32-Bit Pristine ]`
  * `DIP 2 (Hard Sync)`: `[ Korg EA-1 Analog Style Sync | Free Phase Accumulator ]`
  * `DIP 3 (Resonance Window)`: `[ Casio Saw Window | Triangle Trapezoid | Cosine Bell ]`
  * `DIP 4 (Phase Reset)`: `[ Hard Key Trigger 0° | Free Running Continuous ]`
  * `TRIM 1`: Line 1/2 Amplitude Balance.
  * `TRIM 2`: 12-Bit Truncation Noise Bleed.
  * `TRIM 3`: Velocity $\to$ DCW Depth scaling.
  * `TRIM 4`: Sub-Octave Divider Injection.

---

### `OSC-04`: `Pluck` (Physical Waveguide & Modal Resonator)
* **Deep DSP Spec:** [pluck_dsp.md](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/pluck_dsp.md)
* **Upstream Reference:** [braids_manual.md](file:///C:/Dev/Research/library/manuals/bbraids_manual.md) · [ROSETTA_STONE.md#section-b](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-b)
* **Architecture:** Karplus-Strong physical modeling, 24 modal bank resonators, and sympathetic acoustic strings.
* **Front 8 Encoders:**
  1. `PITCH`: Fundamental string frequency ($20\text{ Hz} \dots 4000\text{ Hz}$). Profile D. *Push: Octave Snap*.
  2. `DECAY / DAMP`: $20\text{ ms} \dots 60.0\text{ s}$ physical string damping. Profile A (Cubic $x^3$). *Push: Infinite Ring Freeze*.
  3. `STRIKE / IMPULSE`: Exciter model (Felt Hammer, Metal Pick, Acoustic Bow, Noise Burst). Discrete `0..5`. *Push: Next Exciter*.
  4. `BRIGHTNESS`: High-frequency internal loop filter ($500\text{ Hz} \dots 20\text{ kHz}$). Profile D. *Push: Dark / Bright Toggle*.
  5. `BODY / MATERIAL`: Acoustic resonator material (Nylon, Steel, Paiste Bronze, Glass Tube, Marimba Wood). Profile B. *Push: Cycle Material*.
  6. `TENSION / DISPERSION`: Inharmonic string stiffness & dispersion. Profile B. *Push: Linear String vs Stiff Wire*.
  7. `PICK POSITION`: Comb notch location across the string ($0\% \dots 100\%$, Center 12:00 = 50% dead-center pluck). Profile B. *Push: Bridge Strike (95%)*.
  8. `SYMPATHETIC`: Level of acoustic sympathetic string resonance network. Profile B. *Push: Mute Sympathetic*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Resonator Engine)`: `[ Karplus Dual-Delay Loop | 24-Band Modal Bank | Cylindrical Pipe Tube ]`
  * `DIP 2 (Exciter Trigger)`: `[ Internal PCM Transient | Audio In / Mic Strike | Dual Burst Flam ]`
  * `DIP 3 (Non-Linearity)`: `[ Clean String | Magnetic Pickup Saturation | Buzzing Sitar Bridge ]`
  * `DIP 4 (Polyphony / Sympathetic)`: `[ Monophonic Solo | 4-String Chording | Paraphonic 4 ]`
  * `TRIM 1`: Body Cavity Air Resonance ($100\text{ Hz} \dots 250\text{ Hz}$ guitar body thump).
  * `TRIM 2`: High-Frequency Air Loss damping coefficient.
  * `TRIM 3`: Pickup Non-Linearity Drive.
  * `TRIM 4`: Strike Velocity Dynamic Floor.

---

### `OSC-05`: `Argent Slayer` (Phase-Locked Sub-Bass Generator)
* **Deep DSP Spec:** [argent_slayer_dsp.md](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/aargent_slayer_dsp.md)
* **Upstream Reference:** [tr808_service_manual.md](file:///C:/Dev/Research/library/manuals/tr808_service_manual.md) · [ROSETTA_STONE.md#section-b](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-b)
* **Architecture:** Dual bridged-T resonant filter tanks, recursive sine wavefolder, and noise ring modulation.
* **Front 8 Encoders:**
  1. `SUB PITCH`: Base fundamental ($20\text{ Hz} \dots 200\text{ Hz}$, calibrated for 808 subs). Profile D. *Push: Snap MIDI Note*.
  2. `SUB DECAY`: $10\text{ ms} \dots 60.0\text{ s}$ sub sustain tail. Profile A (Cubic $x^3$). *Push: Snap 808 Classic (1.8s)*.
  3. `CLICK / SNAP`: Transient Bridged-T attack click level. Profile B. *Push: Mute Click (Sub Bass Mode)*.
  4. `PITCH BEND DEPTH`: Transient pitch drop depth ($\pm 48\text{ st}$). Profile B. *Push: Zero Pitch Drop*.
  5. `PITCH BEND TIME`: $2\text{ ms} \dots 200\text{ ms}$ kick pitch sweep speed. Profile A. *Push: Ultra-Fast Snap (5ms)*.
  6. `SINE FOLD`: Recursive Buchla/Mick Gordon wavefolder on the sub. Profile E. *Push: Clean Sub vs Folded Growl*.
  7. `NOISE RING`: Ring-modulated analog noise transient level. Profile B. *Push: Velvet vs White Noise*.
  8. `DRIVE / HEADROOM`: Asymmetric transistor saturation into the output tank. Profile E. *Push: +12dB Boost*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Tank Model)`: `[ Roland TR-808 Bridged-T | Roland TR-909 Dual-Core | Modern Hyper-Compressed 808 | Pure Sine DCO ]`
  * `DIP 2 (Phase Init)`: `[ Hard 0° Punch Snap | Free Running Analog Sag ]`
  * `DIP 3 (Harmonic Wavefolder)`: `[ Odd-Harmonic Triangle Fold | Asymmetric Diode Fold | DOOM Fuzz ]`
  * `DIP 4 (Key Tracking)`: `[ Pitch-Locked 1V/Oct Bass | Fixed Drum Frequency (48Hz) ]`
  * `TRIM 1`: Sub-Harmonic Generator Injection ($-\infty \dots 0\text{ dB}$).
  * `TRIM 2`: Pitch Drop Exponential Curve Skew.
  * `TRIM 3`: Analog Power Supply Sag (dynamic compression under sustained bass).
  * `TRIM 4`: Low-Cut DC Blocker ($10\text{ Hz} \dots 30\text{ Hz}$).

---

### `OSC-06`: `BronzeLab` (Inharmonic Paiste Metallic Cluster)
* **Deep DSP Spec:** [`bronzelab_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/bronzelab_dsp.md)
* **Upstream Reference:** [`rings_manual.md`](file:///C:/Dev/Research/library/manuals/rings_manual.md) · [`ROSETTA_STONE.md#section-b`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-b)
* **Architecture:** 4 metallic square/FM cluster cores feeding an 8-stage all-pass dispersion network for authentic acoustic plate physics.
* **Front 8 Encoders:**
  1. `FUNDAMENTAL PITCH`: Center cluster pitch. Profile D. *Push: Octave Snap*.
  2. `CLUSTER SPREAD`: Inharmonic frequency ratio spread across the 4 metallic cores. Profile B. *Push: Lock Paiste 2002 Bronze Ratio*.
  3. `BODY DECAY`: Low-frequency clang decay ($10\text{ ms} \dots 10.0\text{ s}$). Profile A. *Push: Tight Choke (50ms)*.
  4. `SIZZLE DECAY`: High-frequency shimmer decay ($10\text{ ms} \dots 10.0\text{ s}$). Profile A. *Push: Open Wash (4s)*.
  5. `PLATE DISPERSION`: 8-stage all-pass delay dispersion (simulates travel time in real sheet bronze). Profile B. *Push: Dispersion Bypass*.
  6. `STRIKE / MALLET`: Exciter attack hardness (Wood Stick, Felt Mallet, Wire Brush, Steel Beater). Profile B. *Push: Cycle Mallet*.
  7. `BANDPASS CUTOFF`: Dual high-register spectral formant filter ($1\text{ kHz} \dots 18\text{ kHz}$). Profile D. *Push: Dark / Bright Sizzle*.
  8. `ACOUSTIC BLOOM`: Opposite pitch envelope depth (+Shimmer sweeps up / -Body drops down). Profile B. *Push: Bloom Polarity Flip*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Metal Topology)`: `[ Paiste B20 Bronze (Cymbals) | Roland TR-606 6-Square Metal | TR-808 Cowbell Dual-Square | Gamelan Cast Bell ]`
  * `DIP 2 (Choke Group)`: `[ Independent Free Ring | Closed-Open Hat Choke Link (Group 1) | Crash Choke (Group 2) ]`
  * `DIP 3 (Dispersion Quality)`: `[ 8-Stage Real-Time ZDF All-Pass | 4-Stage Lo-Fi Fast | Discrete Resonant Comb ]`
  * `DIP 4 (Phase Reset)`: `[ Free-Running Phase Clang | Hard-Synced Strike ]`
  * `TRIM 1`: High-Frequency Air Damping coefficient.
  * `TRIM 2`: Rim Hit Harmonic Multiplier.
  * `TRIM 3`: Analog VCA Bleed Floor.
  * `TRIM 4`: Stereo Width Phase Offset ($0^\circ \dots 180^\circ$).

---

### `OSC-07`: `K-Chip` (5-Console Retro Programmable Sound Generator)
* **Deep DSP Spec:** [kchip_dsp.md](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/kchip_dsp.md)
* **Upstream Reference:** [sid_6581_datasheet.md](file:///C:/Dev/Research/library/manuals/sid_6581_datasheet.md) · [ROSETTA_STONE.md#section-b](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-b)
* **Architecture:** Cycle-accurate multi-chip emulation engine uniting NES 2A03, Game Boy DMG, Konami VRC6, Commodore 64 SID, and PC Engine TG-16.
* **Front 8 Encoders:**
  1. `OCTAVE / PITCH`: Chromatic frequency & coarse octave ($-4 \dots +4$). Profile D. *Push: Octave Jump*.
  2. `CHIP CORE`: Selects among the 5 classic sound chips (`0..4`). Discrete Choice. *Push: Cycle Chip*.
  3. `VOICE / CHANNEL`: Selects active channel inside chosen chip (Pulse 1, Pulse 2, 4-bit Triangle, Stepped Saw, Noise, Wavetable). Discrete Choice. *Push: Next Channel*.
  4. `PULSE WIDTH / DUTY`: $12.5\%, 25\%, 50\%, 75\%$ (NES/DMG) or continuous 12-bit PWM (SID 6581). Profile B. *Push: Cycle Duty Presets*.
  5. `DAC CRUSH / STEP`: 4-bit staircase DAC attenuation vs pristine float. Profile H. *Push: 4-Bit Classic vs Modern Clean*.
  6. `HARD SYNC / RING MOD`: Audio-rate slave sync / ring modulation (SID 6581). Profile C. *Push: Sync On/Off*.
  7. `DECAY / MML RATE`: $10\text{ ms} \dots 10.0\text{ s}$ decay or stepped MML tick rate. Profile A. *Push: Free ms vs Tracker Ticks*.
  8. `RETRO SLOP / JITTER`: Authentic clock phase jitter and LFSR pseudo-random noise drift. Profile B. *Push: Jitter Mute*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Console Select)`: `[ Nintendo NES 2A03 | Game Boy DMG-01 | Konami VRC6 | Commodore 64 MOS 6581 | PC Engine TG-16 ]`
  * `DIP 2 (Filter Topology)`: `[ MOS 6581 Switched-Capacitor Filter | NES Non-Linear DAC Curve | DMG Headphone Amp Color ]`
  * `DIP 3 (MML Quantize)`: `[ Stepped 15-Level Volume Macro | Continuous RC Slew ]`
  * `DIP 4 (Arpeggio Mode)`: `[ Free Semitones | 3-Note Major Fast Arp | 3-Note Minor Fast Arp | Octave Hop ]`
  * `TRIM 1`: SID 6581 DC Bias Pop (authentic key-click crackle).
  * `TRIM 2`: NES 2A03 Non-Linear Bass Distortion amount.
  * `TRIM 3`: Game Boy LFSR 7-bit vs 15-bit noise mode toggle.
  * `TRIM 4`: TG-16 5-bit Wavetable Cross-FM depth.

---

### `OSC-08`: `Kaleidocore` (Macro Timbre Synthesizer)
* **Deep DSP Spec:** [kaleidocore_dsp.md](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/kaleidocore_dsp.md)
* **Upstream Reference:** [plaits_manual.md](file:///C:/Dev/Research/library/manuals/plaits_manual.md) · [ROSETTA_STONE.md#section-b](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-b)
* **Architecture:** Multi-model algorithmic macro oscillator (Virtual Analog, Waveshaping, 6-Op FM, Granular Formants, Modal Resonator, Chords).
* **Front 8 Encoders:**
  1. `PITCH`: Master root frequency. Profile D. *Push: Octave Transpose*.
  2. `ENGINE MODEL`: Selects algorithmic synthesis model (`0..15`). Discrete Choice. *Push: Next Engine Model*.
  3. `TIMBRE`: Primary spectral morphing macro (Filter cutoff, wavefold, FM index, formant vowel). Profile B. *Push: Invert Timbre*.
  4. `COLOR`: Secondary spectral parameter (Pulse width, symmetry, noise ratio, algorithm variant). Profile B. *Push: Neutral 12:00*.
  5. `MORPH`: Tertiary spatial/harmonic parameter (Sub level, chord inversion, formant gender). Profile B. *Push: Neutral 12:00*.
  6. `DECAY`: Internal low-pass gate (LPG) decay ($10\text{ ms} \dots 10.0\text{ s}$). Profile A. *Push: Infinite Drone*.
  7. `LPG DYNAMICS`: Low-Pass Gate strike vs sustained VCA. Profile B. *Push: Plucky LPG vs Continuous VCA*.
  8. `FM ATTENUATOR`: Bipolar external/auxiliary FM modulation depth. Profile B. *Push: Phase Invert FM*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Engine Bank)`: `[ Classic Melodic (VA/FM/Wavetable) | Noise & Physical Percussion | Speech & Formants | Chords ]`
  * `DIP 2 (LPG Physics)`: `[ Buchla 292 Vactrol Ring & Tail | Modern Zero-Delay Clean | Fast Snappy Gate ]`
  * `DIP 3 (Key Tracking)`: `[ 1V/Oct Continuous | Microtonal Quantized | Drone Fixed ]`
  * `DIP 4 (Out Mode)`: `[ Main Out + Aux Out (Harmonics) | Stereo Left/Right Spread ]`
  * `TRIM 1`: Vactrol Memory Slew (simulates light-dependent resistor decay lag).
  * `TRIM 2`: Internal Sub-Oscillator Gain.
  * `TRIM 3`: Timbre Modulation Input Attenuation.
  * `TRIM 4`: Color Modulation Input Attenuation.

---

### `OSC-09`: `SqueezeFold` (M8 Single-Cycle Multiplier & Wavefolder)
* **Deep DSP Spec:** [`squeezefold_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/squeezefold_dsp.md)
* **Upstream Reference:** [`m8_shortcut_guide.md`](file:///C:/Dev/Research/library/manuals/m8_shortcut_guide.md) · [`ROSETTA_STONE.md#section-b`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-b)
* **Architecture:** Single-cycle harmonic multiplier, phase-squeezer, and dual-stage wavefolder (inspired by Dirtywave M8 Wavsynth).
* **Front 8 Encoders:**
  1. `PITCH`: Master fundamental pitch. Profile D. *Push: Octave Snap*.
  2. `WAVEFORM`: Sine, Triangle, Saw, Pulse, Camel, Square. Discrete Choice. *Push: Cycle Waveform*.
  3. `MULTIPLY`: Harmonic frequency multiplier ($1\times \dots 16\times$). Profile B. *Push: Integer Ratio Lock*.
  4. `PHASE SQUEEZE`: Phase distortion duty compression (squeezes waveform into narrow window). Profile B. *Push: Zero Squeeze*.
  5. `WAVEFOLD`: Dual-stage symmetric wavefolder. Profile E. *Push: Max Fold Squelch*.
  6. `MIRROR / BIAS`: Asymmetric DC bias before folding. Profile B. *Push: Zero DC Bias*.
  7. `DECAY`: $10\text{ ms} \dots 10.0\text{ s}$ internal decay envelope. Profile A. *Push: Tempo Snap*.
  8. `DRIVE`: Pre-fold transistor drive. Profile E. *Push: +12dB Boost*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Fold Algorithm)`: `[ Buchla Triangle Fold | Serge Diode Fold | M8 Wavsynth Digital Clip | Sine Fold ]`
  * `DIP 2 (Phase Topology)`: `[ Symmetrical Center Squeeze | Left-Edge Pinch | Exponential Ramp ]`
  * `DIP 3 (Anti-Aliasing)`: `[ PolyBLEP Anti-Aliased | Raw Digital Chiptune Harshness ]`
  * `DIP 4 (Envelope Target)`: `[ Fold Depth | Multiply Ratio | Amp VCA Only ]`
  * `TRIM 1`: DC Offset Nulling Calibrator.
  * `TRIM 2`: High-Frequency Anti-Fold Damping.
  * `TRIM 3`: Velocity $\to$ Fold Sensitivity.
  * `TRIM 4`: Post-Fold Headroom Trim.

---

### `OSC-10`: `Flint & Dust` (Analog Noise & Transient Lab)
* **Deep DSP Spec:** [`flint_and_dust_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/flint_and_dust_dsp.md)
* **Upstream Reference:** [`ROSETTA_STONE.md#section-d`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)
* **Architecture:** Dedicated analog noise generator with 6 discrete noise topologies, 4-pole sweepable tilt filters, and sample & hold.
* **Front 8 Encoders:**
  1. `NOISE TOPOLOGY`: White, Pink ($1/f$), Red/Brownian ($1/f^2$), Metallic (6-Square 808 Cluster), Velvet, Vinyl Crackle. Discrete Choice. *Push: Next Topology*.
  2. `FILTER CUTOFF`: Master multimode noise filter ($20\text{ Hz} \dots 20\text{ kHz}$). Profile D Log. *Push: Hyper-Track Toggle*.
  3. `FILTER RESONANCE`: Resonance / Q of noise peak. Profile F. *Push: Tuned Sine Whistle*.
  4. `SPECTRAL TILT`: Continuous $-6\text{ dB/oct}$ bass warm tilt $\leftrightarrow$ flat $\leftrightarrow +6\text{ dB/oct}$ sizzling air. Profile B. *Push: Flat Neutral*.
  5. `SAMPLE & HOLD RATE`: Clock frequency for stepped S&H noise ($0.5\text{ Hz} \dots 10.0\text{ kHz}$). Profile C. *Push: Tempo Sync*.
  6. `DECAY`: Dedicated noise burst envelope decay ($5\text{ ms} \dots 20.0\text{ s}$). Profile A. *Push: Ultra-Fast Snap (15ms)*.
  7. `STEREO SPREAD`: Uncorrelated dual-mono noise width ($0\%\text{ mono} \dots 200\%\text{ super-wide}$). Profile B. *Push: Mono Check*.
  8. `CRACKLE DENSITY`: Random impulse density for Velvet & Vinyl modes ($1\text{ impulse/s} \dots 5000\text{ impulses/s}$). Profile B. *Push: Silent Bed*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Filter Mode)`: `[ 2-Pole Lowpass | 2-Pole Bandpass | 2-Pole Highpass | Notch ]`
  * `DIP 2 (S&H Quantize)`: `[ Continuous Random Voltage | 12-Tone Semitone Quantize | 4-Bit Stepped ]`
  * `DIP 3 (Envelope Mode)`: `[ One-Shot Transient Percussion | Gated Note | Continuous Bed ]`
  * `DIP 4 (Stereo Method)`: `[ Dual Independent Noise Cores | Single Core + Delay Phase Invert ]`
  * `TRIM 1`: LFSR Pseudo-Random Loop Length (16-bit vs 31-bit prime cycle).
  * `TRIM 2`: Vinyl Dust Frequency Balance ($3\text{ kHz} \dots 8\text{ kHz}$).
  * `TRIM 3`: 6-Square Metal Clang Inharmonic Ratios.
  * `TRIM 4`: Output Limiter Ceiling ($-12\text{ dB} \dots 0\text{ dBFS}$).

---

### `OSC-11`: `ReFont` (Acoustic Transient & Sample Strike Player)
* **Deep DSP Spec:** [`refont_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/refont_dsp.md)
* **Upstream Reference:** [`01_renoise_complete_offline_guide.md`](file:///C:/Dev/Research/library/01_renoise_complete_offline_guide.md) · [`ROSETTA_STONE.md#section-d`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)
* **Architecture:** Ultra-low latency single-cycle & percussion transient sample playback engine with Renoise `.xrni` and SoundFont `.sf2` isomorphism, real-time `0Sxx` scrub, bit-depth reduction, and dynamic envelope.
* **Front 8 Encoders:**
  1. `SAMPLE SELECT`: Scrolls through internal transient ROM / project sample library. Discrete Choice. *Push: Preview Audition*.
  2. `PITCH / SPEED`: Sample playback speed / chromatic pitch ($\pm 24\text{ st}$). Profile B. *Push: Reset Root Pitch (0)*.
  3. `SAMPLE START`: Waveform start offset ($0.0\% \dots 90.0\%$). Profile B. *Push: Instant Zero Crossing Snap*.
  4. `SAMPLE LENGTH / DECAY`: Playback window duration ($2\text{ ms} \dots 10.0\text{ s}$). Profile A. *Push: Full Sample Play*.
  5. `ATTACK / SNAP`: 0.1ms..100ms fade-in or transient boost. Profile A. *Push: Instant Snap*.
  6. `BIT DEPTH / DECIMATE`: $16\text{ bit} \dots 2\text{ bit}$ DAC reduction & sample rate decimation. Profile H. *Push: Modern 24-bit vs SP-1200 12-bit*.
  7. `FILTER CUTOFF`: 2-pole lowpass filter for transient dark/bright tailoring. Profile D Log. *Push: Bypass Filter*.
  8. `PLAY DIRECTION / LOOP`: Forward, Reverse, Ping-Pong, One-Shot. Discrete Choice. *Push: Reverse Instant*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Interpolation)`: `[ Sinc Bandlimited High-Res | Linear | Vintage Drop-Sample (SP-1200 / Amiga) ]`
  * `DIP 2 (Retrigger Action)`: `[ Hard Retrigger Snap | Polyphonic Overlap (4 Voice) | Choke Previous ]`
  * `DIP 3 (Velocity Link)`: `[ Velocity -> Level Only | Velocity -> Pitch & Start Time | Vel -> Bit Depth ]`
  * `DIP 4 (Trigger Mode)`: `[ One-Shot Drum Strike | Gated Gate-Hold | Looping Audio Oscillator ]`
  * `TRIM 1`: Zero-Crossing Threshold Sensitivity.
  * `TRIM 2`: SP-1200 Ring Modulation Aliasing Filter simulation.
  * `TRIM 3`: Velocity Dynamic Range floor.
  * `TRIM 4`: Output Gain Trim ($\pm 12\text{ dB}$).

---

### `OSC-12`: `56K Vocoder Internal Carrier` (Clean-Room MKS-50 Engine)
* **Deep DSP Spec:** [`vocoder_carrier_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/vocoder_carrier_dsp.md)
* **Upstream Reference:** [`aalpha_juno_manual.md`](file:///C:/Dev/Research/library/manuals/aalpha_juno_manual.md) · [`ROSETTA_STONE.md#section-b`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-b)
* **Architecture:** Dedicated clean-room software model of the Roland MKS-50 / Alpha Juno Fujitsu MB87130 DCO, optimized as the dedicated internal carrier synthesizer for vocoders.
* **Front 8 Encoders:**
  1. `CARRIER PITCH`: Chromatic root tracking or fixed drone tone. Profile D Log. *Push: Octave Snap*.
  2. `WAVE BLEND`: Continuous morph: Multi-Pulse $\to$ Saw $\to$ Sub $\to$ Chord Multi-Saw. Profile B. *Push: Next Waveform*.
  3. `PWM SPREAD`: Asymmetric pulse width modulation speed & depth. Profile B. *Push: LFO Sync*.
  4. `CHORD VOICING`: Triad, Minor 7th, Sus4, Octaves, Unison. Discrete Choice. *Push: Cycle Voicing*.
  5. `HIGH-PASS BRIGHTNESS`: Pre-carrier highpass filter to strip mud before vocoding. Profile D. *Push: Flat Clean*.
  6. `NOISE SIBILANCE`: Unvoiced high-frequency noise injection for consonants. Profile B. *Push: Mute Noise*.
  7. `STEREO DETUNE`: Twin DCO phase detune spread. Profile B. *Push: Mono Center*.
  8. `CARRIER DRIVE`: Pre-vocoder saturation drive. Profile E. *Push: +6dB Boost*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Chord Tracking)`: `[ MIDI Monophonic Root | Polyphonic MIDI Note Track | Fixed C3 Drone ]`
  * `DIP 2 (DCO Algorithm)`: `[ MB87130 Clean Room | Virtual Analog Moog Saw | Dual Sine Ring ]`
  * `DIP 3 (Sibilance Detector)`: `[ Auto-Voiced High-Pass Switch | Always On Blend | Key Tracked ]`
  * `DIP 4 (Chorus)`: `[ Roland Juno BBD Stereo Chorus | Bypass ]`
  * `TRIM 1`: Unvoiced Sibilance Detection Sensitivity.
  * `TRIM 2`: Sub-Oscillator Harmonic Phase Align.
  * `TRIM 3`: Noise Gate Threshold.
  * `TRIM 4`: Output Limiter Ceiling.

---

# PART II: THE AUDIO EFFECT CANON (`FX-01` .. `FX-18`)

---

### `FX-01`: `Cerberus` (Harmonic Distortion Synthesizer & Preamp)
* **Deep DSP Spec:** [`cerberus_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/cerberus_dsp.md)
* **Upstream Reference:** [`rat_ds1_pedal_manuals.md`](file:///C:/Dev/Research/library/manuals/rat_ds1_pedal_manuals.md) · [`ROSETTA_STONE.md#section-c`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)
* **Architecture:** 3 parallel discrete saturation paths (Warmth Head, Vowel Formant Head, and Crossover Bite Head) with master blend.
* **Front 8 Encoders:**
  1. `DRIVE`: Master input gain ($0\text{ dB} \dots +36\text{ dB}$). Profile E (Warmth $x^{1.8}$). *Push: +12dB Lead Boost*.
  2. `HEAD 1 (WARMTH)`: 2nd-order even harmonic triode tube saturation level. Profile B. *Push: Mute Head 1*.
  3. `HEAD 2 (VOWEL)`: Mid-frequency resonant clipping diode path. Profile B. *Push: Mute Head 2*.
  4. `HEAD 3 (BITE)`: Crossover distortion & high-frequency germanium fuzz. Profile B. *Push: Mute Head 3*.
  5. `VOWEL TUNE`: Center frequency of the Head 2 mid-band resonator ($300\text{ Hz} \dots 3500\text{ Hz}$). Profile D. *Push: Formant Snap*.
  6. `POST-TONE`: Tilt EQ ($100\text{ Hz} \dots 10\text{ kHz}$). Profile B. *Push: Neutral Flat*.
  7. `OUTPUT LEVEL`: Master makeup trim ($\pm 18\text{ dB}$). Profile B. *Push: Unity 0dB*.
  8. `MIX`: Universal Tri-Zone Parallel Mix ($7:00 = 100\%\text{ Wet}, 12:00 = \text{Pure Dry}, 5:00 = \text{Parallel Dry+Wet}$). Profile M. *Push: Phase Invert 180°*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Circuit Topology)`: `[ Korg Nu:Tekt HD-S Tri-Head | Tube Screamer Symmetrical | Boss HM-2 Chainsaw | Germanium Fuzz Face ]`
  * `DIP 2 (Oversampling)`: `[ 1x Zero Latency | 2x Studio | 4x Linear-Phase Pristine ]`
  * `DIP 3 (Bass Cut Pre)`: `[ Full Band Sub (No Cut) | 80 Hz Tighten | 720 Hz Screamer Rolloff ]`
  * `DIP 4 (Mix Mode)`: `[ Tri-Zone Parallel Standard | Pure Linear Crossfade ]`
  * `TRIM 1`: Head 3 Crossover Dead-Band Bias.
  * `TRIM 2`: Even/Odd Harmonic Symmetry Skew.
  * `TRIM 3`: Noise Gate Threshold ($-\infty \dots -40\text{ dB}$).
  * `TRIM 4`: Output True-Peak Brickwall Ceiling.

---

### `FX-02`: `Argent Vortex` (Phase-Fold Resonator & Liquid Filter)
* **Deep DSP Spec:** [`argent_vortex_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/argent_vortex_dsp.md)
* **Upstream Reference:** [`sherman_filterbank_manual.md`](file:///C:/Dev/Research/library/manuals/sherman_filterbank_manual.md) · [`ROSETTA_STONE.md#section-c`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)
* **Architecture:** Recirculating 4-stage Buchla 259 wavefolder paired with an all-pass phase dispersion network.
* **Front 8 Encoders:**
  1. `FOLD DEPTH`: 4-stage non-linear wavefolding depth. Profile E. *Push: Max Fold*.
  2. `SYMMETRY / BIAS`: DC voltage bias into folding stages. Profile B. *Push: Zero Bias*.
  3. `ALL-PASS DISPERSION`: Phase smear delay network across the folds. Profile B. *Push: Dispersion Bypass*.
  4. `RECIRCULATING FEEDBACK`: Chaos loop feedback ($0\% \dots 120\%$). Profile F. *Push: Feedback Invert ($\pm$)*.
  5. `HIGH-CUT CLEANUP`: Post-fold lowpass damping filter ($1\text{ kHz} \dots 20\text{ kHz}$). Profile D Log. *Push: Air Open*.
  6. `LOW-CUT TIGHTEN`: Pre-fold highpass filter ($20\text{ Hz} \dots 500\text{ Hz}$). Profile D Log. *Push: Deep Sub Pass*.
  7. `OUTPUT LEVEL`: Makeup gain ($\pm 18\text{ dB}$). Profile B. *Push: Reset 0dB*.
  8. `MIX`: Universal Tri-Zone Parallel Mix. Profile M. *Push: Phase Invert 180° (Dynamic Transient Extraction!)*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Fold Model)`: `[ Buchla 259 Classic | Serge Triple Waveshaper | Asymmetric Germanium Diode | Sine Shaper ]`
  * `DIP 2 (Routing Order)`: `[ Fold -> Dispersion | Dispersion -> Fold | Recirculating Dual Loop ]`
  * `DIP 3 (DC Blocker)`: `[ Ultra-Low DC Filter (5Hz) | Raw DC Coupled ]`
  * `DIP 4 (Oversampling)`: `[ 2x Fast | 4x HQ | 8x Master ]`
  * `TRIM 1`: Stage 1 Fold Knee Hardness.
  * `TRIM 2`: Chaos Self-Oscillation Safety Limiter.
  * `TRIM 3`: Pre-Drive Headroom Pad.
  * `TRIM 4`: Stereo Phase Offset between Left/Right fold paths.

---

### `FX-03`: `PipKlick` (Wavetable Waveshaper & Decimator)
* **Deep DSP Spec:** [`pipklick_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/pipklick_dsp.md)
* **Upstream Reference:** [`spl_transient_designer_manual.md`](file:///C:/Dev/Research/library/manuals/spl_transient_designer_manual.md) · [`ROSETTA_STONE.md#section-c`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)
* **Architecture:** WMD Geiger Counter style 256-wavetable amplitude waveshaper with bit-mask XOR corruption and sample rate decimation.
* **Front 8 Encoders:**
  1. `WAVESHAPER TABLE`: Scrolls through 256 transfer-function wavetables (`0..255`). Discrete Choice. *Push: Random Table Jump*.
  2. `PRE-GAIN / DRIVE`: Drive into wavetable ($0\text{ dB} \dots +40\text{ dB}$). Profile E. *Push: Clean Unity*.
  3. `SAMPLE RATE DECIMATE`: Clock reduction ($48\text{ kHz} \dots 200\text{ Hz}$). Profile D Log. *Push: Pristine 48kHz*.
  4. `BIT DEPTH CRUSH`: Quantization depth ($16\text{ bit} \dots 1\text{ bit}$). Profile H. *Push: 8-Bit Lo-Fi*.
  5. `BIT-MASK XOR`: Digital bit-mask corruption pattern (`0x00..0xFF`). Discrete Choice. *Push: Invert Mask*.
  6. `FILTER CUTOFF`: Post-digital reconstructive lowpass filter ($500\text{ Hz} \dots 20\text{ kHz}$). Profile D. *Push: Air Open*.
  7. `OUTPUT LEVEL`: Master level trim ($\pm 18\text{ dB}$). Profile B. *Push: Reset 0dB*.
  8. `MIX`: Universal Tri-Zone Parallel Mix. Profile M. *Push: Phase Invert 180°*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (DAC Architecture)`: `[ Linear Truncation | SP-1200 12-Bit Linear Companded | Amiga Paula 8-Bit | Raw Float ]`
  * `DIP 2 (Jitter / Clock Noise)`: `[ Stable Quartz Clock | Drifting Lo-Fi Battery Sag | LFSR Clock Jitter ]`
  * `DIP 3 (Routing Order)`: `[ Shaper -> Bitcrush -> Decimate | Decimate -> Bitcrush -> Shaper ]`
  * `DIP 4 (Anti-Aliasing Filter)`: `[ Post-Reconstruction Filter | Raw Unfiltered Metal Aliasing ]`
  * `TRIM 1`: Bit-Mask Threshold Gate.
  * `TRIM 2`: Wavefold Sub-Harmonic Bias.
  * `TRIM 3`: Noise Floor Bleed.
  * `TRIM 4`: Output Limiter Ceiling.

---

### `FX-04`: `The Annihilator (NUKE)` / `Annihilator Punch` (Multi-Topology Compressor & Limiter)
* **Deep DSP Spec:** [`annihilator_punch_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/annihilator_punch_dsp.md)
* **Upstream Reference:** [`ssl_g_bus_manual.md`](file:///C:/Dev/Research/library/manuals/ssl_g_bus_manual.md) · [`ROSETTA_STONE.md#section-c`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)
* **Architecture:** Multi-topology dynamics processor modeled after SSL G-Bus, 1176 FET, LA-2A Opto, and Distressor NUKE.
* **Front 8 Encoders:**
  1. `THRESHOLD`: Dynamic trigger point ($-48\text{ dB} \dots 0\text{ dB}$). Profile B. *Push: Auto-Threshold*.
  2. `RATIO`: Compression slope ($1:1 \dots 20:1$, plus **NUKE** mode). Discrete Choice. *Push: Slam NUKE (All-Buttons)*.
  3. `ATTACK`: $0.1\text{ ms} \dots 100\text{ ms}$. Profile A (Cubic $x^3$). *Push: Fast 1176 Snap (20µs)*.
  4. `RELEASE`: $10\text{ ms} \dots 1.5\text{ s}$ or Auto. Profile A. *Push: Auto Release Program-Dependent*.
  5. `VCA DRIVE / DIST`: Discrete harmonic saturation (Dist 2 2nd-order, Dist 3 3rd-order). Profile E. *Push: Dist Off / Dist 2 / Dist 3*.
  6. `SIDECHAIN HPF`: Low-cut filter in sidechain path ($20\text{ Hz} \dots 400\text{ Hz}$). Profile D. *Push: Bypass HPF*.
  7. `MAKEUP GAIN`: Output compensation ($0\text{ dB} \dots +30\text{ dB}$). Profile B. *Push: Auto Makeup*.
  8. `MIX`: Universal Tri-Zone Parallel Mix (New York Style parallel compression). Profile M. *Push: Phase Invert 180°*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Circuit Topology)`: `[ SSL G-Bus VCA | UREI 1176 FET Slam | Teletronix LA-2A Opto | Empirical Labs NUKE ]`
  * `DIP 2 (Stereo Coupling)`: `[ 100% Stereo Linked | True Dual-Mono Independent | Mid/Side Unlinked ]`
  * `DIP 3 (Knee Shape)`: `[ Hard Knee Punch | Soft Knee Musical | Over-Easy S-Curve ]`
  * `DIP 4 (Sidechain Source)`: `[ Internal Audio | External Key Input | High-Frequency De-Esser ]`
  * `TRIM 1`: Opto Light-Dependent Resistor Memory Lag.
  * `TRIM 2`: Downward Expander / Noise Gate Threshold.
  * `TRIM 3`: True-Peak Brickwall Limiter Ceiling.
  * `TRIM 4`: Output Transformer Saturation Bias.

---

### `FX-05`: `Argent Anvil` (DOOM Upward Compressor & Brickwall Limiter)
* **Deep DSP Spec:** [`argent_anvil_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/argent_anvil_dsp.md)
* **Upstream Reference:** [`ROSETTA_STONE.md#section-d`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)
* **Architecture:** 3-band upward/downward dynamics processor with up to $+100\text{ dB}$ of upward gain extraction and true-peak lookahead limiter (inspired by Mick Gordon's DOOM processing).
* **Front 8 Encoders:**
  1. `ANVIL DRIVE`: Master upward compression drive ($0\% \dots 100\%$, pulls low-level room sound to surface). Profile E. *Push: Slam 100%*.
  2. `DOWNWARD COMP`: Heavy peak downward squash ($0\text{ dB} \dots -36\text{ dB}$). Profile B. *Push: Bypass Downward*.
  3. `CROSSOVER LOW`: Low/Mid crossover frequency ($80\text{ Hz} \dots 500\text{ Hz}$). Profile D. *Push: Snap 120Hz*.
  4. `CROSSOVER HIGH`: Mid/High crossover frequency ($1.5\text{ kHz} \dots 8\text{ kHz}$). Profile D. *Push: Snap 3.5kHz*.
  5. `BAND BIAS / TILT`: Mid-band upward dominance tilt. Profile B. *Push: Flat Neutral*.
  6. `RELEASE SPEED`: Multi-band envelope release speed ($10\text{ ms} \dots 1.0\text{ s}$). Profile A. *Push: Tempo Snap*.
  7. `BRICKWALL CEILING`: Ultra-fast lookahead limiter ceiling ($-12\text{ dB} \dots 0\text{ dBFS}$). Profile B. *Push: -0.1 dB True-Peak*.
  8. `MIX`: Universal Tri-Zone Parallel Mix. Profile M. *Push: Phase Invert 180°*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Dynamics Mode)`: `[ 3-Band Upward OTT / DOOM | Full-Band Aggro Smash | Parallel Upward Only ]`
  * `DIP 2 (Limiter Knee)`: `[ Transparent Lookahead (1ms) | Soft Diode Clip | Hard Digital Brickwall ]`
  * `DIP 3 (Sidechain Filter)`: `[ Flat 3-Band | Bass Safety Choke (30Hz) | Mid-Scoop Metal ]`
  * `DIP 4 (Stereo Width)`: `[ Natural Stereo | Low-End Mono Collapse (<120Hz) | Super-Wide M/S ]`
  * `TRIM 1`: Noise Gate Upward Floor (prevents pulling up noise floor during silence).
  * `TRIM 2`: Lookahead Buffer Latency ($0.1\text{ ms} \dots 5.0\text{ ms}$).
  * `TRIM 3`: High-Band De-Esser Clamping.
  * `TRIM 4`: Output DC Blocker Filter.

---

### `FX-06`: `KlangVCF` (11-Model Resonant Filter Lab)
* **Deep DSP Spec:** [`klangvcf_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/klangvcf_dsp.md) · [`vandal_park_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/vandal_park_dsp.md)
* **Upstream Reference:** [`arturia_pigments_manual.md`](file:///C:/Dev/Research/library/manuals/arturia_pigments_manual.md) · [`ROSETTA_STONE.md#section-e`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-e)
* **Deep DSP Spec (Vandal Park):** [andal_park_dsp.md](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/vandal_park_dsp.md)
* **Upstream Reference:** [rturia_pigments_manual.md](file:///C:/Dev/Research/library/manuals/arturia_pigments_manual.md) · [ROSETTA_STONE.md#section-e](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-e)
* **Architecture:** Dual-core state-variable multimode filter featuring the 11 Immutable Canon Models, pre-drive, and audio-rate FM.
* **Front 8 Encoders:**
  1. `CUTOFF`: $20\text{ Hz} \dots 20\text{ kHz}$. Profile D Log. *Push: Toggle HYPER-TRACK (±400%)*.
  2. `RESONANCE`: $0.5 \dots 25.0$. Profile F (Hyperbolic). *Push: Lock Self-Oscillation*.
  3. `FILTER TYPE`: **The 11 Canon Models** (Clean SVF, Moog Ladder, Korg Screamer 35, 303 Diode, Hornet Wasp CMOS, Vandal Park, Sherman Warbank, Alpha Dual-12, Vowel Formant, DJ Isolator, Tuned Comb). Discrete `0..10`. *Push: Next Category*.
  4. `SLOPE`: 6, 12, 18, 24, 36 dB/oct. Discrete `0..4`. *Push: Cycle Slope*.
  5. `PRE-DRIVE`: Asymmetric soft-clipping warmer before resonance. Profile E. *Push: Bypass Drive*.
  6. `KEYTRACK`: Bipolar $-100\% \dots +100\%$ tracking. Profile B. *Push: Snap 1V/Oct (100%)*.
  7. `PRE-HPF CLEAN`: $20\text{ Hz} \dots 500\text{ Hz}$ pre-filter highpass mud stripper. Profile D. *Push: Flat 20Hz*.
  8. `STEREO SPREAD`: Left/Right antiphase cutoff frequency offset. Profile B. *Push: Pure Mono Center*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Filter Topology Sub-Variant)`: `[ Lowpass | Bandpass | Highpass | Notch ]`
  * `DIP 2 (Bass Drop Behavior)`: `[ Authentic Ladder Bass Drop | Modern Resonance Bass Compensation ]`
  * `DIP 3 (Audio-Rate FM Source)`: `[ Oscillator Core | Internal LFO | External Mod ]`
  * `DIP 4 (Non-Linear Saturation)`: `[ Clean Linear Math | Warm Diode Non-Linear | Screaming Germanium ]`
  * `TRIM 1`: Audio-Rate Filter FM Depth index.
  * `TRIM 2`: Thermal Resonance Drift / Analog Slop amount.
  * `TRIM 3`: High-Frequency Phase Correction.
  * `TRIM 4`: Output Headroom Makeup Trim.

---

### `FX-07`: `Passive Bass Contour` (Dual-Choke Inductor Low-End Resonator)
* **Deep DSP Spec:** [`passive_bass_contour_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/passive_bass_contour_dsp.md)
* **Upstream Reference:** [`pultec_eqp1a_manual.md`](file:///C:/Dev/Research/library/manuals/pultec_eqp1a_manual.md) · [`ROSETTA_STONE.md#section-c`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)
* **Architecture:** Clean-room passive equalizer modeling the Pultec EQP-1A simultaneous low-boost/low-cut trick, inductors, and high-frequency Maag-style Air Band.
* **Front 8 Encoders:**
  1. `BASS FREQ`: Low shelf center frequency ($20\text{ Hz}, 30\text{ Hz}, 60\text{ Hz}, 100\text{ Hz}$). Discrete Choice. *Push: Cycle Freq*.
  2. `BASS BOOST`: Low-end inductor boost ($0\text{ dB} \dots +16\text{ dB}$). Profile B. *Push: Reset 0dB*.
  3. `BASS ATTEN`: Simultaneous low-end cut ($0\text{ dB} \dots -16\text{ dB}$, creating the classic Pultec resonant scoop). Profile B. *Push: Reset 0dB*.
  4. `MID FREQ`: Parametric mid bell frequency ($200\text{ Hz} \dots 7.0\text{ kHz}$). Profile D Log. *Push: Bell vs Notch*.
  5. `MID GAIN`: Parametric mid boost/cut ($\pm 12\text{ dB}$). Profile B. *Push: Reset 0dB*.
  6. `MID Q`: Mid bandwidth ($0.5 \dots 10.0$). Profile B. *Push: Snap 0.707 Musical*.
  7. `AIR BAND`: Ultra-high peaking shelf ($20\text{ kHz} \dots 40\text{ kHz}$ air boost, $0\text{ dB} \dots +12\text{ dB}$). Profile B. *Push: Mute Air*.
  8. `TUBE OUTPUT`: Output transformer & 12AX7 tube makeup gain ($\pm 12\text{ dB}$). Profile E. *Push: Reset Unity*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (EQ Circuit Model)`: `[ Pultec EQP-1A Tube | Solid-State Neve 1073 Inductor | API 550 Proportional Q ]`
  * `DIP 2 (Transformer Iron)`: `[ Clean Modern Jensen | Vintage Triad A-11J Iron Core Saturation ]`
  * `DIP 3 (Air Band Frequency)`: `[ 10 kHz | 20 kHz | 40 kHz Stratosphere ]`
  * `DIP 4 (Phase Mode)`: `[ Minimum Phase Analog | Linear Phase Clean Master ]`
  * `TRIM 1`: Inductor Core Saturation Threshold.
  * `TRIM 2`: Subsonic DC Cutoff Filter ($5\text{ Hz} \dots 25\text{ Hz}$).
  * `TRIM 3`: Mid-Band Proportional Q scaling factor.
  * `TRIM 4`: Output Tube Headroom Trim.

---

### `FX-08`: `Dimension BBD` (Analog Bucket-Brigade Chorus & Spatial Expander)
* **Deep DSP Spec:** [`dimension_bbd_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/dimension_bbd_dsp.md)
* **Upstream Reference:** [`roland_dimension_d_sdd320_manual.md`](file:///C:/Dev/Research/library/manuals/roland_dimension_d_sdd320_manual.md) · [`ROSETTA_STONE.md#section-c`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)
* **Architecture:** True-stereo 4-stage bucket brigade delay chorus with cross-coupled feedback and anti-aliasing reconstruction filters.
* **Front 8 Encoders:**
  1. `DIMENSION MODE`: Discrete modes `1..4` (Roland Dimension D spatial buttons) or `Manual`. Discrete Choice. *Push: Cycle Buttons (1, 2, 3, 4, 1+4)*.
  2. `RATE`: Modulation LFO speed ($0.1\text{ Hz} \dots 10.0\text{ Hz}$). Profile C. *Push: Tempo Sync*.
  3. `DEPTH`: BBD delay time modulation depth ($0.5\text{ ms} \dots 12.0\text{ ms}$). Profile B. *Push: Zero Depth (Vibrato Bypass)*.
  4. `BBD COLOR`: Analog clock filter darkening ($2\text{ kHz} \dots 16\text{ kHz}$). Profile D Log. *Push: Juno-60 Warm vs Dimension Clean*.
  5. `FEEDBACK`: Flanger-like chorus feedback ($\pm 80\%$). Profile F. *Push: Polarity Invert*.
  6. `OUTPUT LEVEL`: Makeup gain ($\pm 12\text{ dB}$). Profile B. *Push: Reset 0dB*.
  7. `STEREO SPREAD`: Cross-coupled phase offset between Left/Right BBD chips ($0^\circ \dots 180^\circ$). Profile B. *Push: True 180° Dimension Split*.
  8. `MIX`: Universal Tri-Zone Parallel Mix ($7:00 = 100\%\text{ Wet Vibrato}, 12:00 = \text{Dry}, 5:00 = \text{Parallel Dimension}$). Profile M. *Push: Phase Invert 180°*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (BBD Chip Core)`: `[ Panasonic MN3007 (Juno-60) | Reticon SAD1024 (Electric Mistress) | Dimension D Clean ]`
  * `DIP 2 (LFO Waveform)`: `[ Triangle Smooth | Dual Quadrature Sine | Random S&H Drift ]`
  * `DIP 3 (Clock Bleed Noise)`: `[ BBD Ultrasonic Clock Bleed On | Filtered Silent ]`
  * `DIP 4 (Input Headroom)`: `[ Vintage -10dBV Instrument Level (Warm Clip) | Studio +4dBu Headroom ]`
  * `TRIM 1`: BBD Compander Pre-Emphasis / De-Emphasis calibration.
  * `TRIM 2`: Delay Time Center Offset ($2.5\text{ ms} \dots 8.0\text{ ms}$).
  * `TRIM 3`: Clock Noise Whistle Bleed volume ($-\infty \dots -60\text{ dBFS}$).
  * `TRIM 4`: Output Limiter Ceiling.

---

### `FX-09`: `ZeroFlange` (Through-Zero Tape Flanger)
* **Deep DSP Spec:** [`zeroflange_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/zeroflange_dsp.md)
* **Upstream Reference:** [`through_zero_flanging_pedal_manuals.md`](file:///C:/Dev/Research/library/manuals/through_zero_flanging_pedal_manuals.md) · [`ROSETTA_STONE.md#section-c`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)
* **Architecture:** Through-zero flanger with dedicated reference delay and swept delay paths crossing at true $0.0\text{ ms}$, producing complete phase cancellation.
* **Front 8 Encoders:**
  1. `THROUGH-ZERO OFFSET`: Manual delay center offset crossing $0.0\text{ ms}$ ($-5\text{ ms} \dots 0.0\text{ ms} \dots +5\text{ ms}$). Profile B. *Push: Snap True 0ms Zero-Crossing*.
  2. `RATE`: Flange sweep rate ($0.02\text{ Hz} \dots 10.0\text{ Hz}$). Profile C. *Push: Tempo Sync*.
  3. `SWEEP DEPTH`: LFO modulation depth across zero ($0\% \dots 100\%$). Profile B. *Push: Zero LFO (Manual Comb Cut)*.
  4. `REGENERATION / FEEDBACK`: Runaway feedback ($\pm 98\%$). Profile F. *Push: Negative Feedback Polarity Flip*.
  5. `TAPE SATURATION`: Discrete reel-to-reel tape head saturation. Profile E. *Push: Clean vs Gritty Tape*.
  6. `TAPE TONE / DAMP`: High-frequency tape rolloff filter ($1\text{ kHz} \dots 20\text{ kHz}$). Profile D Log. *Push: Bright / Dark*.
  7. `STEREO SPREAD`: Left/Right through-zero time offset. Profile B. *Push: Mono Flange*.
  8. `MIX`: Universal Tri-Zone Parallel Mix. Profile M. *Push: Phase Invert 180° (Unleashes Hollow Comb Cancellation!)*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Flange Mode)`: `[ Authentic Through-Zero Tape | Classic BBD Flanger | Barberpole Infinite Flange ]`
  * `DIP 2 (LFO Shape)`: `[ Sine Symmetrical | Hyperbolic Logarithmic (Equal Pitch Rise/Fall) ]`
  * `DIP 3 (Tape Wow & Flutter)`: `[ Studer A800 Precision | Ampex 350 Drifting Tape Reel ]`
  * `DIP 4 (Zero Point Anchor)`: `[ Center Zero | Edge Zero | Floating Slew ]`
  * `TRIM 1`: Tape Head Bias Voltage.
  * `TRIM 2`: Negative Regeneration Peak Limiter.
  * `TRIM 3`: Reference Delay Static Latency ($1\text{ ms} \dots 10\text{ ms}$).
  * `TRIM 4`: Output Wet Balance.

---

### `FX-10`: `Bi-Phase 12` (12-Stage Optical Dual Phaser)
* **Deep DSP Spec:** [`biphase12_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/biphase12_dsp.md)
* **Upstream Reference:** [`mu_tron_bi_phase_manual.md`](file:///C:/Dev/Research/library/manuals/mu_tron_bi_phase_manual.md) · [`ROSETTA_STONE.md#section-c`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)
* **Architecture:** Dual 6-stage / single 12-stage optical all-pass phaser modeled after the Mu-Tron Bi-Phase with dual LFO routing.
* **Front 8 Encoders:**
  1. `SWEEP RATE A`: Section A optical LFO rate ($0.05\text{ Hz} \dots 18.0\text{ Hz}$). Profile C. *Push: Tempo Sync*.
  2. `SWEEP RATE B`: Section B optical LFO rate. Profile C. *Push: Lock Rate B to Rate A*.
  3. `DEPTH A`: Section A phase modulation depth. Profile B. *Push: Invert Sweep*.
  4. `DEPTH B`: Section B phase modulation depth. Profile B. *Push: Invert Sweep*.
  5. `FEEDBACK / COLOR`: Resonance feedback across 12 all-pass poles ($\pm 95\%$). Profile F. *Push: Polarity Invert*.
  6. `STAGES`: Discrete pole count (`4-Pole Classic`, `6-Pole Vintage`, `8-Pole Deep`, `12-Pole Jet Airplane`). Discrete `0..3`. *Push: Next Stage Count*.
  7. `ROUTING TOPOLOGY`: `[ Series (12-Pole Jet) | Parallel Stereo (Dual 6-Pole) | Ping-Pong Dual ]`. Discrete Choice. *Push: Cycle Routing*.
  8. `MIX`: Universal Tri-Zone Parallel Mix. Profile M. *Push: Phase Invert 180°*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Optocoupler Model)`: `[ Vintage Vactrol VTL5C3 (Asymmetric Lag) | Modern Fast Opto | Clean JFET Phaser ]`
  * `DIP 2 (LFO Sync / Link)`: `[ Independent LFOs | Section A Drives Both | Inverted Anti-Phase Sweep ]`
  * `DIP 3 (Sweep Shape)`: `[ Triangle Smooth | Sine Optical | Random Stepped S&H ]`
  * `DIP 4 (Center Frequency Range)`: `[ Guitar / Mid-Band (200Hz..4kHz) | Deep Bass Sweep (50Hz..1.5kHz) | Wide Audio ]`
  * `TRIM 1`: Optical Vactrol Turn-On vs Turn-Off Decay Lag.
  * `TRIM 2`: Feedback Path Soft-Clipper headroom.
  * `TRIM 3`: Section A/B Phase Alignment.
  * `TRIM 4`: Output Limiter Ceiling.

---

### `FX-11`: `Optical Pan & Trem` (Harmonic Tremolo & Stereo Auto-Panner)
* **Deep DSP Spec:** [`pan_and_trem_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/pan_and_trem_dsp.md)
* **Upstream Reference:** [`fender_vibrato_manual.md`](file:///C:/Dev/Research/library/manuals/fender_vibrato_manual.md) · [`ROSETTA_STONE.md#section-c`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)
* **Architecture:** Dual-path Fender brownface harmonic tremolo (crossover split into highpass/lowpass out-of-phase modulation) and optical amplitude chop.
* **Front 8 Encoders:**
  1. `SPEED`: Modulation rate ($0.1\text{ Hz} \dots 20.0\text{ Hz}$, with Profile C audio-rate buzz). Profile C. *Push: Tempo Sync Division*.
  2. `DEPTH`: Amplitude modulation depth ($0\% \dots 100\%$). Profile B. *Push: Max 100% Chop*.
  3. `HARMONIC / VINTAGE`: Continuous morph between **Vintage Optical Amplitude Tremolo** (0%) and **Fender Brownface Harmonic Tremolo** (100%). Profile B. *Push: Toggle Mode*.
  4. `WAVEFORM`: Sine, Triangle, Square (Hard Chop), Saw Up, Saw Down, S&H Random. Discrete Choice. *Push: Next Waveform*.
  5. `TUBE WARMTH`: Pre-tremolo 12AX7 preamp tube warmth. Profile E. *Push: Clean vs Saturated*.
  6. `CROSSOVER FREQ`: Harmonic tremolo crossover split ($200\text{ Hz} \dots 1200\text{ Hz}$). Profile D. *Push: Snap 500Hz*.
  7. `STEREO PHASE`: Left/Right LFO phase offset ($0^\circ = \text{Mono Trem} \dots 180^\circ = \text{Auto-Pan Ping-Pong}$). Profile B. *Push: Snap 180° Ping-Pong*.
  8. `MIX`: Universal Tri-Zone Parallel Mix. Profile M. *Push: Phase Invert 180°*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Circuit Ancestry)`: `[ 1960 Fender Bandmaster Harmonic | 1965 Blackface Optical Bug | Vox AC30 Vib-Trem ]`
  * `DIP 2 (LFO Dynamic Envelope)`: `[ Static Speed | Dynamic Attack Acceleration | Dynamic Speed Decay ]`
  * `DIP 3 (Chop Envelope)`: `[ Soft Sine Glide | Hard Optical Resistor Rise/Fall ]`
  * `DIP 4 (Bass Safety)`: `[ Full-Band Tremolo | Bass Untouched (<100Hz Mono Anchor) ]`
  * `TRIM 1`: Optocoupler Fall-Time Slew.
  * `TRIM 2`: Harmonic Crossover Resonant Bump.
  * `TRIM 3`: Dynamic Envelope Follower Sensitivity.
  * `TRIM 4`: Output Gain Trim ($\pm 12\text{ dB}$).

---

### `FX-12`: `FreakShifter` (Single-Sideband Frequency Shifter & Ring Modulator)
* **Deep DSP Spec:** [`freakshifter_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/freakshifter_dsp.md)
* **Upstream Reference:** [`moog_bode_1630_manual.md`](file:///C:/Dev/Research/library/manuals/moog_bode_1630_manual.md) · [`ROSETTA_STONE.md#section-c`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)
* **Architecture:** Harald Bode frequency shifter utilizing dual 90-degree Hilbert transform all-pass networks with antiphase up/down shift, plus 4-quadrant analog ring modulation.
* **Front 8 Encoders:**
  1. `SHIFT FREQUENCY`: Bipolar frequency shift amount ($-2000.0\text{ Hz} \dots 0.0\text{ Hz} \dots +2000.0\text{ Hz}$). Profile B (Center Deadzone Plateau). *Push: Zero Shift (True Neutral)*.
  2. `FINE SHIFT`: Fine precision frequency shift ($\pm 10.0\text{ Hz}$ for barberpole phasing). Profile B. *Push: Zero Fine*.
  3. `MODE SELECT`: `[ Bode Frequency Shift (Single-Sideband) | 4-Quadrant Ring Modulator | Dual Hilbert Barberpole ]`. Discrete Choice. *Push: Cycle Mode*.
  4. `UP / DOWN MIX`: Balance between Up-Shifted and Down-Shifted sidebands ($-100\%\text{ Down} \dots 0\%\text{ Dual} \dots +100\%\text{ Up}$). Profile B. *Push: Pure Up vs Pure Down*.
  5. `FEEDBACK / DISSOLVE`: Shifter feedback loop ($0\% \dots 110\%$, spiraling infinite pitch staircases). Profile F. *Push: Feedback Invert*.
  6. `CARRIER SHAPE`: Sine, Triangle, Square (Chippy), Noise. Discrete Choice. *Push: Next Shape*.
  7. `FEEDBACK DELAY`: Delay time inside frequency-shifter feedback loop ($5\text{ ms} \dots 1.5\text{ s}$). Profile A. *Push: Tempo Snap*.
  8. `MIX`: Universal Tri-Zone Parallel Mix. Profile M. *Push: Phase Invert 180°*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Hilbert Filter Order)`: `[ 8-Stage Classic Bode | 16-Stage Ultra-Low Sideband Leakage ]`
  * `DIP 2 (Carrier Squelch Gate)`: `[ Auto Carrier Mute on Silence | Free Running Continuous Carrier ]`
  * `DIP 3 (Feedback Insertion)`: `[ Pre-Delay Loop | Post-Delay Loop | Cross-Coupled Antiphase ]`
  * `DIP 4 (Stereo Shift)`: `[ Left Up / Right Down (Opposite Spatial Bloom) | Mono Matched ]`
  * `TRIM 1`: Carrier Feedthrough Suppression nulling pot.
  * `TRIM 2`: Hilbert Transform Phase Deviation calibrator.
  * `TRIM 3`: Feedback High-Cut Damping ($1\text{ kHz} \dots 16\text{ kHz}$).
  * `TRIM 4`: Output Limiter Ceiling.

---

### `FX-13`: `EchoMill` (Recursive Insert Tape Delay)
* **Deep DSP Spec:** [`echomill_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/echomill_dsp.md)
* **Upstream Reference:** [`roland_space_echo_re201_manual.md`](file:///C:/Dev/Research/library/manuals/roland_space_echo_re201_manual.md) · [`ROSETTA_STONE.md#section-c`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)
* **Architecture:** Multi-head magnetic tape delay with simulated 3-head tape loop, motorized tape speed flutter, and non-linear tape head compression.
* **Front 8 Encoders:**
  1. `TAPE SPEED / TIME`: Motorized tape speed / delay time ($20\text{ ms} \dots 2.0\text{ s}$). Profile A (Cubic $x^3$). *Push: Tempo Sync Division*.
  2. `HEAD SELECT`: Multi-head selection (`Head 1`, `Head 2`, `Head 3`, `1+2`, `2+3`, `All 3 Heads - Binson Echorec`). Discrete Choice. *Push: Cycle Heads*.
  3. `REGEN / FEEDBACK`: Tape repeat feedback ($0\% \dots 120\%$ self-oscillation). Profile F. *Push: Dub Explosion Slam*.
  4. `TAPE WEAR / WOW`: Motor wow/flutter and crinkled tape loop degradation. Profile B. *Push: Pristine Tape vs Worn Out Cartridge*.
  5. `BASS TONE`: Active low-frequency tape playback equalization ($\pm 12\text{ dB}$). Profile B. *Push: Flat 0dB*.
  6. `TREBLE TONE`: High-frequency tape damping and saturation rolloff ($\pm 12\text{ dB}$). Profile B. *Push: Flat 0dB*.
  7. `PREAMP DRIVE`: RE-201 JFET preamp input drive. Profile E. *Push: Clean vs Overdriven Preamp*.
  8. `MIX`: Universal Tri-Zone Parallel Mix. Profile M. *Push: Phase Invert 180°*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Echo Mechanism)`: `[ Roland RE-201 Space Echo Tape | Binson Echorec Magnetic Wire Disc | Echoplex EP-3 Solid State ]`
  * `DIP 2 (Motor Slew Physics)`: `[ Real-Time Pitch Slew (Doppler Pitch Warp on Speed Change) | Seamless Crossfade ]`
  * `DIP 3 (Splice Bump Click)`: `[ Physical Tape Loop Joint Splice Bump Enabled | Seamless Digital Buffer ]`
  * `DIP 4 (Stereo Imaging)`: `[ Multi-Head Panned Stereo Array | Authentic Mono Tape Path ]`
  * `TRIM 1`: Tape Head Asymmetry / Hysteresis Bias.
  * `TRIM 2`: Motor Capstan Drift LFO Frequency ($0.2\text{ Hz} \dots 2.5\text{ Hz}$).
  * `TRIM 3`: Self-Oscillation Safety Headroom Pad.
  * `TRIM 4`: Spring Reverb Bleed Level (simulates RE-201 internal spring tank bleed).

---

### `FX-14`: `ClockBleed` (Analog Bucket-Brigade Delay Line)
* **Deep DSP Spec:** [`clockbleed_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/clockbleed_dsp.md)
* **Upstream Reference:** [`sp1200_service_manual.md`](file:///C:/Dev/Research/library/manuals/sp1200_service_manual.md) · [`ROSETTA_STONE.md#section-c`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)
* **Architecture:** 4096-stage MN3005 BBD delay line with realistic bucket loss, compander artifacts, runaway self-oscillation, and ultrasonic clock whine bleed.
* **Front 8 Encoders:**
  1. `DELAY TIME`: Bucket clock rate ($15\text{ ms} \dots 800\text{ ms}$). Profile A. *Push: Tempo Sync Division*.
  2. `FEEDBACK`: Runaway analog regeneration ($0\% \dots 120\%$). Profile F (Hyperbolic). *Push: Infinite Hold*.
  3. `CLOCK BLEED`: Ultrasonic clock whine leak into audio path ($-\infty \dots -36\text{ dBFS}$). Profile B. *Push: Clock Whine Mute*.
  4. `BUCKET DAMPING`: High-frequency bucket loss per stage ($500\text{ Hz} \dots 8\text{ kHz}$). Profile D Log. *Push: Dark Dub vs Clean*.
  5. `DRIVE`: Analog input saturation into first bucket stage. Profile E. *Push: +6dB Boost*.
  6. `COMPANDER CHARACTER`: NE570 analog compander squashing and breathing artifacts. Profile B. *Push: Clean vs Pumping*.
  7. `STEREO SPREAD / PING-PONG`: Cross-delayed stereo ping-pong width. Profile B. *Push: Mono Dub*.
  8. `MIX`: Universal Tri-Zone Parallel Mix. Profile M. *Push: Phase Invert 180°*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (BBD Chip)`: `[ Dual MN3005 (Memory Man) | Single MN3007 (Short Dark) | High-Voltage SAD1024 ]`
  * `DIP 2 (Clock Overclocking)`: `[ Standard Clock (Max 400ms) | Dark Lo-Fi Overclock (800ms Grit) ]`
  * `DIP 3 (Modulation LFO)`: `[ Chorus Vibrato LFO | Off | Random Bucket Drift ]`
  * `DIP 4 (Bypass Mode)`: `[ True Hard Bypass | Trails Decay Spillover ]`
  * `TRIM 1`: Clock Whine Center Frequency nulling pot.
  * `TRIM 2`: Compander Attack/Release Time Constant.
  * `TRIM 3`: Feedback Limiting Diode Threshold.
  * `TRIM 4`: Wet Signal DC Offset Null.

---

### `FX-15`: `Vintage Steel Plate` (Physical EMT-140 Plate & Digital Hall Reverb)
* **Deep DSP Spec:** [`steel_plate_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/steel_plate_dsp.md)
* **Upstream Reference:** [`emt_140_plate_manual.md`](file:///C:/Dev/Research/library/manuals/emt_140_plate_manual.md) · [`ROSETTA_STONE.md#section-c`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)
* **Architecture:** Hybrid physical steel plate model (finite difference mesh) and 12-bit Dattorro continuous figure-eight algorithmic reverb.
* **Front 8 Encoders:**
  1. `DECAY TIME / RT60`: Reverb tail duration ($0.2\text{ s} \dots 60.0\text{ s}$). Profile A (Cubic $x^3$). *Push: Infinite Reverb Freeze*.
  2. `PRE-DELAY`: $0\text{ ms} \dots 250\text{ ms}$. Profile A. *Push: Tempo Sync Division*.
  3. `DAMPING / HIGH-CUT`: High-frequency absorption ($1\text{ kHz} \dots 20\text{ kHz}$). Profile D Log. *Push: Bright / Dark*.
  4. `LOW-CUT / BASS ROLLOFF`: Sub-bass rumble filter ($20\text{ Hz} \dots 500\text{ Hz}$). Profile D Log. *Push: Flat Clean*.
  5. `DIFFUSION`: Early echo density & wall reflection smear ($0\% \dots 100\%$). Profile B. *Push: Grainy Clicks vs Smooth Wash*.
  6. `MODULATION / CHORUS`: Internal delay line pitch modulation depth. Profile B. *Push: Modulation Mute*.
  7. `SPATIAL WIDTH`: Stereo cross-coupling width ($0\%\text{ mono} \dots 200\%\text{ super-wide}$). Profile B. *Push: Mono Check*.
  8. `MIX`: Universal Tri-Zone Parallel Mix. Profile M. *Push: Phase Invert 180°*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Reverb Engine)`: `[ EMT-140 Physical Steel Plate | Lexicon 224 Concert Hall | Lexicon 480L Rich Plate | Dark Ambient Void ]`
  * `DIP 2 (DAC Resolution)`: `[ Vintage 12-Bit Fixed Point (224 Grit) | 32-Bit Floating Point Modern ]`
  * `DIP 3 (Freeze Action)`: `[ Infinite Freeze Maintains Tail | Freeze Clears On Key Off ]`
  * `DIP 4 (Input Highpass)`: `[ 80 Hz Vocal Protect | Wide Bandwidth Full Sub ]`
  * `TRIM 1`: EMT-140 Damper Mechanical Tension.
  * `TRIM 2`: Modulator LFO Speed ($0.1\text{ Hz} \dots 2.0\text{ Hz}$).
  * `TRIM 3`: Tank Cross-Coupling Coefficient.
  * `TRIM 4`: Output Limiter Safety Ceiling.

---

### `FX-16`: `Vampire` (Synthetic Reverb Tail Generator & Feedback Pitch Shifter)
* **Deep DSP Spec:** [`vampire_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/vampire_dsp.md)
* **Upstream Reference:** [`electro_harmonix_octave_multiplexer.md`](file:///C:/Dev/Research/library/manuals/electro_harmonix_octave_multiplexer.md) · [`ROSETTA_STONE.md#section-c`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)
* **Architecture:** 32-delay Feedback Delay Network (FDN) with non-linear saturating tank feedback and octave pitch-shifting shimmer loop.
* **Front 8 Encoders:**
  1. `DECAY / REGEN`: FDN tank decay ($0.5\text{ s} \dots \infty\text{ infinite}$). Profile A. *Push: Infinite Freeze Lock*.
  2. `SHIMMER PITCH`: Pitch shift amount inside the feedback loop ($+12\text{ st Octave Up}, -12\text{ st Sub}, +7\text{ st 5th}, +24\text{ st}$). Discrete Choice. *Push: Cycle Pitch*.
  3. `SHIMMER AMOUNT`: Shimmer pitch-shift feedback injection level ($0\% \dots 100\%$). Profile B. *Push: Mute Shimmer*.
  4. `INTERNAL SATURATION`: Non-linear tank saturation drive (creates blooming ambient distortion). Profile E. *Push: Clean vs Gritty FDN*.
  5. `TANK TONE`: High/low spectral balance inside feedback loop. Profile B. *Push: Neutral Flat*.
  6. `DIFFUSION / SMEAR`: FDN unitary scattering matrix complexity. Profile B. *Push: Dense vs Sparse*.
  7. `MODULATION JITTER`: Random multi-delay line modulation depth. Profile B. *Push: Mute Jitter*.
  8. `MIX`: Universal Tri-Zone Parallel Mix. Profile M. *Push: Phase Invert 180°*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Tank Topology)`: `[ Clean Shimmer Cloud | Vampire Dark Distortion | Bleak Cold Metal | Glitch S&H Buffer ]`
  * `DIP 2 (Shimmer Filter)`: `[ Pre-Pitch Filter | Post-Pitch Filter | Dual Bandpass ]`
  * `DIP 3 (Pitch Algorithm)`: `[ Dual Splice Granular | Phase-Locked Resampling | 12-Bit Retro Shifter ]`
  * `DIP 4 (Trails On Bypass)`: `[ Natural Spillover Trails | Hard Panic Cut ]`
  * `TRIM 1`: Pitch Shifter Grain Window Size ($20\text{ ms} \dots 120\text{ ms}$).
  * `TRIM 2`: Feedback Saturation Soft-Clipper Knee.
  * `TRIM 3`: Low-End Resonator Damping.
  * `TRIM 4`: Stereo Cross-Leak Factor.

---

### `FX-17`: `56K Vocoder` (32-Band Spectral Vocoder Processor)
* **Deep DSP Spec:** [`vocoder_56k_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/vocoder_56k_dsp.md)
* **Upstream Reference:** [`vp330_service_manual.md`](file:///C:/Dev/Research/library/manuals/vp330_service_manual.md) · [`ROSETTA_STONE.md#section-c`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)
* **Architecture:** 32-band spectral analysis/synthesis vocoder with integrated MKS-50 internal carrier, formant tracking, vocal freeze, and sibilance generator.
* **Front 8 Encoders:**
  1. `CARRIER SOURCE`: Internal MKS-50 Synthesizer, External Sidechain, White Noise, Auto-Self Vocode. Discrete Choice. *Push: Cycle Carrier*.
  2. `FORMANT SHIFT`: Modulator analysis filter bank frequency shift ($\pm 12\text{ semitones}$, Gender Warper). Profile B. *Push: Reset Formants to 0*.
  3. `BANDWIDTH / Q`: Filter Q of the 32 bandpass filter pairs ($0.5 \dots 20.0$). Profile F. *Push: Vocal Formant vs Metallic Ring*.
  4. `ENVELOPE DECAY`: Analysis envelope follower tracking speed ($2\text{ ms} \dots 500\text{ ms}$). Profile A. *Push: Instant 2ms Snap*.
  5. `SIBILANCE LEVEL`: High-frequency unvoiced consonant replacement level. Profile B. *Push: Mute Sibilance*.
  6. `VOCAL FREEZE`: Infinite spectral envelope freeze. Toggle Continuous. *Push: Latch Freeze On/Off*.
  7. `HIGH AIR TILT`: Post-synthesis high-frequency air enhancement. Profile B. *Push: Flat Neutral*.
  8. `MIX`: Universal Tri-Zone Parallel Mix. Profile M. *Push: Phase Invert 180°*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Filter Bank Resolution)`: `[ 32-Band Studio Pristine | 16-Band Vintage Roland SVC-350 | 8-Band Kraftwerk Lo-Fi ]`
  * `DIP 2 (Internal Carrier Wave)`: `[ MKS-50 Multi-Saw | Dual Detuned Pulse | Octave Square ]`
  * `DIP 3 (Sibilance Routing)`: `[ Direct Highpass Modulator Bleed | Noise Generator Replacement ]`
  * `DIP 4 (Auto-Vocode Drum Mode)`: `[ Self-Vocoder On (Modulator IS Carrier) | Standard Carrier/Mod ]`
  * `TRIM 1`: Analysis Envelope Follower Attack Time.
  * `TRIM 2`: Sibilance Detector Frequency Threshold ($4.5\text{ kHz} \dots 8.0\text{ kHz}$).
  * `TRIM 3`: Noise Gate Threshold on Modulator Input.
  * `TRIM 4`: Carrier Input Headroom Trim.

---

### `FX-18`: `Rotary Horn & Drum` / `Rotary Speaker` (Rotating Doppler Speaker & Amp)
* **Deep DSP Spec:** [`rotary_speaker_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/rotary_speaker_dsp.md)
* **Upstream Reference:** [`leslie_122_service_manual.md`](file:///C:/Dev/Research/library/manuals/leslie_122_service_manual.md) · [`ROSETTA_STONE.md#section-c`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)
* **Architecture:** Physical model of the Leslie 122 rotary speaker with separate rotating high-frequency horn, low-frequency bass rotor drum, Doppler pitch shift, and 40-watt 6550 tube amplifier.
* **Front 8 Encoders:**
  1. `SPEED SWITCH`: Chorale (Slow), Tremolo (Fast), Brake (Stop). Discrete Choice. *Push: Cycle Speed (Slow / Fast / Brake)*.
  2. `FAST SPEED`: Target rotational velocity when in Fast mode ($5.0\text{ Hz} \dots 9.0\text{ Hz}$). Profile C. *Push: Classic 6.8Hz*.
  3. `SLOW SPEED`: Target rotational velocity when in Slow mode ($0.5\text{ Hz} \dots 1.5\text{ Hz}$). Profile C. *Push: Classic 0.8Hz*.
  4. `HORN / DRUM BALANCE`: Volume balance between top high-frequency horn and bottom bass drum. Profile B. *Push: Equal 50/50 Balance*.
  5. `TUBE DRIVE`: 40-watt 6550 tube power amp saturation. Profile E. *Push: Clean vs Screaming Overdrive*.
  6. `ACCELERATION / INERTIA`: Motor ramp-up/ramp-down transition time ($0.5\text{ s} \dots 8.0\text{ s}$, horns ramp faster than heavy drums!). Profile A. *Push: Snap 2.0s*.
  7. `MIC DISTANCE / SPREAD`: Virtual stereo microphone placement distance ($0.2\text{ m} \dots 3.0\text{ m}$) and stereo angle. Profile B. *Push: Close Studio Mics*.
  8. `MIX`: Universal Tri-Zone Parallel Mix. Profile M. *Push: Phase Invert 180°*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Speaker Cabinet)`: `[ Leslie 122 Wooden Cabinet | Leslie 147 Bright | Fender Vibratone Guitar Drum ]`
  * `DIP 2 (Microphone Array)`: `[ Stereo XY Front | Blümlein Pair | 8-Point Quad Room Mics ]`
  * `DIP 3 (Horn / Drum Inertia Link)`: `[ Authentic Decoupled Inertia (Horn accelerates 3x faster) | Locked Synchronous ]`
  * `DIP 4 (Cabinet Resonance)`: `[ Wooden Box Cavity Resonance On | Free Field Clean ]`
  * `TRIM 1`: Doppler Pitch Modulation Depth ($0\text{ cents} \dots \pm 60\text{ cents}$).
  * `TRIM 2`: Wind Noise / Rotor Air Chuff volume.
  * `TRIM 3`: Bass Drum Mechanical Drag damping.
  * `TRIM 4`: Output Limiter Safety Ceiling.

---

### `FX-19`: `Argent Reactor` (Time-Woven Dual Resonant Filter Matrix & Chaotic Delay)
* **Deep DSP Spec:** [`argent_reactor_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/argent_reactor_dsp.md)
* **Upstream Reference:** [`jomox_t_resonator_manual.md`](file:///C:/Dev/Research/library/manuals/jomox_t_resonator_manual.md) · [`ROSETTA_STONE.md#section-d`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)
* **Architecture:** Hardware clone of the Jomox T-Resonator II infused with DOOM Argent energy: dual 24dB Zero-Delay Feedback transistor ladders with bipolar resonance (Godzilla sub-harmonic quake to screaming whistle), cross-phase matrix, audio-rate cross-FM, 8 delay/waveguide/reverb programs, and an analog-saturated loop feedback path.
* **Front 8 Encoders (Performance View: 1..8):**
  1. `CUTOFF L`: Left 24dB ladder cutoff ($20\,\text{Hz} \dots 20\,\text{kHz}$). Profile D. *Push: Snap 1 kHz / Link L+R*.
  2. `CUTOFF R`: Right 24dB ladder cutoff ($20\,\text{Hz} \dots 20\,\text{kHz}$). Profile D. *Push: Snap 1 kHz / Invert Sweep*.
  3. `RES L (±)`: Left bipolar resonance ($-100\% \dots +100\%$). Profile B. *Push: Zero Resonance*. (Godzilla Sub-Quake $\leftrightarrow$ Screaming Whistle).
  4. `RES R (±)`: Right bipolar resonance ($-100\% \dots +100\%$). Profile B. *Push: Zero Resonance*. (Godzilla Sub-Quake $\leftrightarrow$ Screaming Whistle).
  5. `ARGENT BURN`: Input diode saturation & DOOM overdrive ($0\,\text{dB} \dots +24\,\text{dB}$). Profile E. *Push: +6dB Extreme Boost*.
  6. `TIME / PARA 1`: Primary delay time or acoustic waveguide pitch ($0.6\,\text{ms} \dots 1000\,\text{ms}$). Profile C. *Push: Tap / Sync Tempo*.
  7. `LOOP FEEDBACK`: Analog filter re-injection back into digital delay loop ($0\% \dots 130\%$). Profile E. *Push: Infinite Hold Latch*.
  8. `MIX`: Universal Tri-Zone Wet/Dry with internal limiter clamp. Profile M. *Push: Phase Invert ($\varnothing = 180^\circ$)*.
* **Contextual Front-Panel Remapping (Dynamic Mode Matrix):**
  | Active Program | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
  | :--- | :--- | :--- | :--- | :--- | :--- |
  | **`03 & 04: Waveguide I & II`** | `Knob 06` / `TRIM 3` | `PIPE LENGTH 1 / 2` | $0.6 \dots 30\,\text{ms}$ | `Profile C` | Tunes resonant acoustic physical model pitch |
  | **`07: Cathedral of Flesh`** | `TRIM 3` | `REVERB DECAY` | $0.1 \dots 30\,\text{s}$ | `Profile A` | Non-linear dense plate decay time |
  | **`08: Void Singularity`** | `TRIM 3` | `SINGULARITY` | $0 \dots 100\%$ | `Profile B` | Controls runaway non-linear density collapse |
* **Rear Deck ⚙️ (4 DIP Switches + 4 Precision Trimpots) (Settings View: 9..16):**
  * `DIP 1 / Control 09 (Algorithm Selector)`: `[ 01: Plasma Chorus | 02: Acid Wash Flange | 03: Bone Waveguide I | 04: Hell Waveguide II | 05: Stygian Delay I | 06: Doom Slap / Grinder | 07: Cathedral of Flesh | 08: Void Singularity ]`
  * `DIP 2 / Control 10 (Delay Injection Routing & Phase)`: `[ Dual In-Phase (+Phase) | Anti-Phase (-Phase Comb Cancel) | Cross-Inject (Delay L▶Flt R, Delay R▶Flt L) | Post-Filter Parallel ]`
  * `DIP 3 / Control 11 (Modulation Sync & Trigger Mode)`: `[ Audio Transient Retrigger (Drum Peak Reset) | Free-Running Sine | Tempo Sync (1/16 Beat) | Envelope Follower Direct ]`
  * `DIP 4 / Control 12 (Argent Limiter & Saturation Model)`: `[ Argent Soft-Knee (+6dBFS Safe) | DOOM BJT Diode Carnage (Hard Clip) | Germanium Sag & Voltage Droop | Unbounded (DANGER / Passthrough) ]`
  * `TRIM 1 / Control 13 (Cross-Phase Coupling)`: Bipolar cross-phase mixing between filter outputs and opposite inputs ($-100\% \dots +100\%$, Profile B). *Push: Toggle Symmetrical ($L=R$) vs Antiphase ($L=-R$)*.
  * `TRIM 2 / Control 14 (Cross-FM Intensity)`: Audio-rate frequency modulation intensity between filter cores ($0\% \dots 100\%$, Profile E). *Push: Toggle Dual-Symmetric (L mod R & R mod L) vs One-Way (L mod R)*.
  * `TRIM 3 / Control 15 (Decay / Para 2)`: Secondary delay decay, reverb tail, or physical waveguide pipe 2 length ($0\% \dots 100\%$, Profile C). *Push: Reset Default ($50\%$)*.
  * `TRIM 4 / Control 16 (LFO Rate & Dynamics)`: Audio-retriggerable sine LFO speed ($0.15\,\text{Hz} \dots 22\,\text{Hz}$, Profile A). *Push: Toggle Envelope-Masked Burst Mode vs Free-Run Sine*.

---

# PART III: THE 8 KLANG MEGA-EFFECTS (`MEGA-01` .. `MEGA-08`)

---

### `MEGA-01`: `KlangWarp` (Studio Saturation, Wavefolder & Clipper Lab)
* **Deep DSP Spec:** [`klangwarp_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/klangwarp_dsp.md)
* **Upstream Reference:** [`arturia_pigments_manual.md`](file:///C:/Dev/Research/library/manuals/arturia_pigments_manual.md) · [`ROSETTA_STONE.md#section-d`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)
* **Architecture:** 6-engine master saturation workstation combining Padé tanh, diode clipping, the XXXL harmonic warp, recursive sine wavefolding, 12-bit decimation, and full-wave rectification.
* **Front 8 Encoders:**
  1. `DRIVE`: Master input drive ($-6\text{ dB} \dots +36\text{ dB}$). Profile E (Warmth $x^{1.8}$). *Push: +12dB Lead Boost*.
  2. `FOLD / BITS`: Multi-stage wavefold depth or digital bit depth. Profile B. *Push: Invert Fold Direction*.
  3. `ASYM / BIAS`: DC voltage offset / 2nd-harmonic asymmetry. Profile B. *Push: Zero Neutral Bias*.
  4. `WARP CHARACTER`: 6 engines (Soft Tanh, Diode Clamp, XXXL Harmonic, West Coast Fold, 12-Bit Decimate, Full-Wave Rectifier). Discrete `0..5`. *Push: Next Character*.
  5. `TONE / TILT`: Post-saturation tilt EQ. Profile B. *Push: Flat Neutral*.
  6. `HEADROOM / CLAMP`: Output ceiling & diode clamp threshold. Profile B. *Push: 0dB Brickwall*.
  7. `LOW CLEAN`: High-pass sidechain protecting sub-bass from distortion ($20\text{ Hz} \dots 500\text{ Hz}$). Profile D. *Push: Pass Full Sub (20Hz)*.
  8. `MIX`: Universal Tri-Zone Parallel Mix. Profile M. *Push: Phase Invert 180° (Dynamic Transient Extraction)*.
* **Contextual Front-Panel Remapping (Dynamic Mode Matrix):**
  | Active Warp Character | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
  | :--- | :--- | :--- | :--- | :--- | :--- |
  | **`3: West Coast Fold`** | `Knob 2, Knob 3` | `FOLD STAGES`, `SYMMETRY` | `1..8 Stages`, `±100%` | `Discrete`, `Profile B` | Recursive sine wavefolding & DC mirror |
  | **`4: 12-Bit Decimate`** | `Knob 2, Knob 3` | `BIT DEPTH`, `SAMPLE RATE`| `16..2 Bits`, `48kHz..200Hz` | `Profile H`, `Profile D` | SP-1200 / S950 retro DAC crunch |
  | **`2: XXXL Harmonic`** | `Knob 2, Knob 3` | `EVEN 2ND HARMONIC`, `ODD 3RD` | `0..100%`, `0..100%` | `Profile B`, `Profile B` | Discrete tape warmth vs tube roar balance |
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Oversampling)`: `[ 1x Zero Latency | 2x HQ Studio | 4x Linear Phase | 8x Master Archive ]`
  * `DIP 2 (Bass Protection Routing)`: `[ Sidechain HPF Only | Parallel Sub Injection (Sub stays 100% clean) ]`
  * `DIP 3 (DC Filter)`: `[ Ultra-Low 5Hz DC Blocker | Raw DC-Coupled ]`
  * `DIP 4 (Mix Law)`: `[ Tri-Zone Parallel Standard | Pure Linear Crossfade ]`
  * `TRIM 1`: Diode Breakdown Knee Hardness.
  * `TRIM 2`: 12-Bit Truncation Dither Bleed.
  * `TRIM 3`: Fold Stage 1 Thermal Bias Drift.
  * `TRIM 4`: Output Limiter Safety Ceiling.

---

### `MEGA-02`: `KlangPunch` (Dynamics, Transients & Bus Glue Lab)
* **Deep DSP Spec:** [`klangpunch_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/klangpunch_dsp.md) · [`argent_anvil_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/argent_anvil_dsp.md)
* **Upstream Reference:** [`ssl_g_bus_manual.md`](file:///C:/Dev/Research/library/manuals/ssl_g_bus_manual.md) · [`ROSETTA_STONE.md#section-d`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)
* **Architecture:** Multi-ballistic dynamics workstation combining SSL G-Bus VCA, SPL Transient Shaper, 1176 British FET, LA-2A Opto, Rhythmic Ducking Pump, and the Distressor Annihilator NUKE.
* **Front 8 Encoders:**
  1. `THRESHOLD`: Compressor trigger point ($-60\text{ dB} \dots 0\text{ dB}$). Profile B. *Push: Auto-Threshold*.
  2. `ATTACK / PUNCH`: Attack time ($0.1\text{ ms} \dots 100\text{ ms}$) or Transient Attack ($\pm 100\%$). Profile A / B. *Push: Fast 20µs Snap*.
  3. `RELEASE / SUSTAIN`: Release time ($10\text{ ms} \dots 2000\text{ ms}$) or Transient Sustain ($\pm 100\%$). Profile A / B. *Push: Auto Program-Dependent*.
  4. `RATIO / ENGINE`: 6 engines (VCA Bus Glue, Transient Shaper, 1176 FET, LA-2A Opto, Sidechain Pump, The Annihilator NUKE). Discrete `0..5`. *Push: Slam NUKE Mode*.
  5. `KNEE / NUKE`: Knee curvature $\leftrightarrow$ Brickwall reverse-knee NUKE. Profile B. *Push: Hard vs Soft Knee*.
  6. `SIDECHAIN HPF`: Low-cut filter in sidechain path ($20\text{ Hz} \dots 400\text{ Hz}$). Profile D. *Push: Bypass HPF*.
  7. `MAKEUP GAIN`: Output compensation ($0\text{ dB} \dots +30\text{ dB}$). Profile B. *Push: Auto-Makeup*.
  8. `MIX`: Universal Tri-Zone Parallel Mix (New York Style parallel compression). Profile M. *Push: Phase Invert 180°*.
* **Contextual Front-Panel Remapping (Dynamic Mode Matrix):**
  | Active Ratio / Engine | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
  | :--- | :--- | :--- | :--- | :--- | :--- |
  | **`1: SPL Transient Shaper`** | `Knob 1, Knob 2, Knob 3` | `SENSITIVITY`, `ATTACK PUNCH`, `SUSTAIN GAIN` | `0..100%`, `±100%`, `±100%` | `Profile B` | Dedicated dual-ballistic envelope differential |
  | **`4: Sidechain Pump`** | `Knob 1, Knob 2, Knob 3` | `DUCK DEPTH`, `RELEASE SYNC`, `CURVE SKEW` | `0..100%`, `1/64..1 bar`, `±100%` | `Profile B`, `Profile A` | Cableguys style rhythmic volume pumping |
  | **`5: The Annihilator NUKE`**| `Knob 4, Knob 5` | `NUKE SLAM`, `DIST HARMONICS` | `Brickwall`, `Clean / Dist 2 / Dist 3` | `Discrete` | British all-buttons smash with tape/tube grit |
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Harmonic Saturation)`: `[ Clean Transparent | Dist 2 (Tube 2nd) | Dist 3 (Tape 3rd) ]`
  * `DIP 2 (Stereo Coupling)`: `[ 100% Stereo Linked | True Dual-Mono Independent | Mid/Side Unlinked ]`
  * `DIP 3 (Sidechain Source)`: `[ Internal Audio | External Key Input | De-Esser Bandpass ]`
  * `DIP 4 (Limiter Post)`: `[ True-Peak Brickwall Limiter Active | Direct Output ]`
  * `TRIM 1`: Opto Light-Dependent Resistor Memory Slew.
  * `TRIM 2`: Downward Expander / Noise Gate Threshold.
  * `TRIM 3`: True-Peak Brickwall Limiter Ceiling.
  * `TRIM 4`: Output Transformer Saturation Bias.

---

### `MEGA-03`: `KlangFilter` (Spectral Synthesis & Resonator Lab)
* **Deep DSP Spec:** [`klangfilter_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/klangfilter_dsp.md) · [`klangvcf_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/klangvcf_dsp.md)
* **Upstream Reference:** [`sherman_filterbank_manual.md`](file:///C:/Dev/Research/library/manuals/sherman_filterbank_manual.md) · [`ROSETTA_STONE.md#section-d`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)
* **Architecture:** Flagship dual-core state-variable multimode filter featuring the **11 Immutable Canon Models**, pre-drive, audio-rate FM, and formant morphing.
* **Front 8 Encoders:**
  1. `CUTOFF`: $20\text{ Hz} \dots 20\text{ kHz}$. Profile D Log. *Push: Toggle HYPER-TRACK (±400%)*.
  2. `RESONANCE`: $0.5 \dots 25.0$. Profile F (Hyperbolic). *Push: Lock Self-Oscillation*.
  3. `MODEL`: **The 11 Immutable Canon Models** (Clean SVF, Moog Ladder, Screamer 35, 303 Diode, Hornet Wasp CMOS, Vandal Park, Sherman Warbank, Alpha Dual-12, Vowel Formant, DJ Isolator, Tuned Comb). Discrete `0..10`. *Push: Next Category*.
  4. `DRIVE`: Pre-filter analog saturation. Profile E. *Push: Bypass Drive*.
  5. `ENVELOPE DEPTH`: Bipolar filter mod depth ($\pm 100\%$). Profile B. *Push: Invert Polarity*.
  6. `KEYTRACK`: Bipolar $-100\% \dots +100\%$ tracking. Profile B. *Push: Snap 1V/Oct (100%)*.
  7. `SPREAD / FACTOR`: Detune / Vandal Factor / Dual Resonance Notch / Formant Morph. Profile B. *Push: Mono Center*.
  8. `MIX`: Universal Tri-Zone Parallel Mix. Profile M. *Push: Phase Invert 180°*.
* **Contextual Front-Panel Remapping (Dynamic Mode Matrix):**
  | Active Filter Model | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
  | :--- | :--- | :--- | :--- | :--- | :--- |
  | **`5: Vandal Park`** | `Knob 7` | `VANDAL FACTOR` | `0..100%` | `Profile E` | Non-linear diode feedback brute factor |
  | **`6: Sherman Warbank`** | `Knob 7` | `AUDIO FM DEPTH` | `0..100%` | `Profile E` | Intermodulation audio-rate FM into filter core |
  | **`7: Alpha Dual-12`** | `Knob 7` | `RES 2 SPREAD` | `±100%` | `Profile B` | Independent secondary resonance peak offset |
  | **`8: Vowel Formant`** | `Knob 1, Knob 7` | `VOWEL MORPH`, `GENDER` | `A-E-I-O-U`, `±12 st` | `Profile B` | 3-band parallel vocal formant morphing |
  | **`10: Tuned Comb`** | `Knob 1, Knob 7` | `CHROMATIC PITCH`, `DAMPING` | `C0..C8`, `1kHz..20kHz` | `Profile D` | Chromatic physical resonator string/tube |
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Filter Pass Mode)`: `[ Lowpass | Bandpass | Highpass | Notch ]`
  * `DIP 2 (Bass Drop Behavior)`: `[ Authentic Ladder Bass Drop | Modern Resonance Bass Compensation ]`
  * `DIP 3 (Audio-Rate FM Source)`: `[ Oscillator Core | Internal LFO | External Mod ]`
  * `DIP 4 (Non-Linear Saturation)`: `[ Clean Linear Math | Warm Diode Non-Linear | Screaming Germanium ]`
  * `TRIM 1`: Audio-Rate Filter FM Depth index.
  * `TRIM 2`: Thermal Resonance Drift / Analog Slop amount.
  * `TRIM 3`: High-Frequency Phase Correction.
  * `TRIM 4`: Output Headroom Makeup Trim.

---

### `MEGA-04`: `KlangEQ` (Studio, Console & DJ Performance Equalizer)
* **Deep DSP Spec:** [`klangeq_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/klangeq_dsp.md) · [`passive_bass_contour_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/passive_bass_contour_dsp.md)
* **Upstream Reference:** [`pultec_eqp1a_manual.md`](file:///C:/Dev/Research/library/manuals/pultec_eqp1a_manual.md) · [`ROSETTA_STONE.md#section-d`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)
* **Architecture:** 4-topology console and DJ equalizer combining British Neve 1073 inductors, American API 550 proportional Q, modern clean ZDF biquads, and passive Pultec tubes.
* **Front 8 Encoders:**
  1. `LOW GAIN`: $\pm 15\text{ dB}$. Profile B. *Push: Shelf $\leftrightarrow$ Bell $\leftrightarrow$ Passive Bass Contour (Pultec Trick)*.
  2. `MID FREQ`: $100\text{ Hz} \dots 8\text{ kHz}$. Profile D Log. *Push: Cycle Q Width (0.5, 1.2, 4.0)*.
  3. `MID GAIN`: $\pm 15\text{ dB}$. Profile B. *Push: Band Mute*.
  4. `HIGH GAIN`: $\pm 15\text{ dB}$. Profile B. *Push: Shelf $\leftrightarrow$ Bell $\leftrightarrow$ Air Band*.
  5. `DJ FILTER`: Bipolar center-flat filter sweep (12:00 Flat, CCW = LPF to 20Hz, CW = HPF to 20kHz). Profile B. *Push: Pioneer DJM vs Xone 92 vs 303 Diode*.
  6. `TILT / TONE`: Spectral tilt pivoted at 1.0kHz ($\pm 6\text{ dB}$). Profile B. *Push: Shift Pivot (650Hz / 1kHz / 2.5kHz)*.
  7. `DRIVE / COLOR`: Neve 1073 inductor / transformer saturation ($0\text{ dB} \dots +18\text{ dB}$). Profile E. *Push: Clean vs Iron Saturated*.
  8. `OUTPUT / TRIM`: Output gain ($\pm 18\text{ dB}$). Profile B. *Push: Phase Invert 180°*.
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (EQ Topology)`: `[ British Inductor (Neve 1073) | American Console (API 550) | Modern Clean ZDF Biquad | Passive Tube Pultec ]`
  * `DIP 2 (DJ Filter Curve)`: `[ Sharp 24dB/oct Resonance | Smooth 12dB/oct Musical | Isolator Full Kill ]`
  * `DIP 3 (Air Band Frequency)`: `[ 16 kHz | 24 kHz | 40 kHz Stratosphere ]`
  * `DIP 4 (Phase Architecture)`: `[ Minimum Phase Analog | Linear Phase Mastering ]`
  * `TRIM 1`: Inductor Core Saturation Threshold.
  * `TRIM 2`: Subsonic DC Cutoff Filter ($5\text{ Hz} \dots 30\text{ Hz}$).
  * `TRIM 3`: Mid-Band Proportional Q scaling factor.
  * `TRIM 4`: Pultec Low-End Resonant Scoop Depth.

---

### `MEGA-05`: `KlangMod` (Chorus, Ensemble & Modulation Lab)
* **Deep DSP Spec:** [`klangmod_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/klangmod_dsp.md) · [`dimension_bbd_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/dimension_bbd_dsp.md)
* **Upstream Reference:** [`roland_dimension_d_sdd320_manual.md`](file:///C:/Dev/Research/library/manuals/roland_dimension_d_sdd320_manual.md) · [`ROSETTA_STONE.md#section-d`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)
* **Architecture:** 8-engine modulation workstation combining Roland Juno BBD, Alpha Juno swept Hoover chorus, 16-voice hyper-ensemble, Dimension D spatial expansion, through-zero flanging, optical phasing, and harmonic tremolo.
* **Front 8 Encoders:**
  1. `RATE`: Modulation LFO speed ($0.05\text{ Hz} \dots 20.0\text{ Hz}$ / Host Sync). Profile C. *Push: Tempo Sync Division*.
  2. `DEPTH`: Modulation depth ($0\% \dots 100\%$). Profile B. *Push: Zero Depth (Bypass Mod)*.
  3. `FEEDBACK`: Regeneration feedback ($\pm 100\%$). Profile F. *Push: Feedback Invert*.
  4. `ENGINE`: 8 algorithms (Clean Digital Stereo, Vintage Juno BBD, Swept Hoover Chorus, 16-Voice Hyper-Ensemble, Dimension D Spatial, Through-Zero Flanger, 8-Stage Optical Phaser, Harmonic Tremolo). Discrete `0..7`. *Push: Next Engine*.
  5. `PRE-DELAY`: Delay time center offset ($0\text{ ms} \dots 25\text{ ms}$). Profile A. *Push: Snap 0ms Through-Zero*.
  6. `TONE / COLOR`: Analog BBD high-cut damping ($1\text{ kHz} \dots 18\text{ kHz}$). Profile D Log. *Push: Juno Warm vs Clean Modern*.
  7. `STEREO WIDTH`: Spatial spread ($0\% \dots 200\%$). Profile B. *Push: True 180° Dimension Split*.
  8. `MIX`: Universal Tri-Zone Parallel Mix. Profile M. *Push: Phase Invert 180°*.
* **Contextual Front-Panel Remapping (Dynamic Mode Matrix):**
  | Active Modulation Engine | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
  | :--- | :--- | :--- | :--- | :--- | :--- |
  | **`5: Through-Zero Flanger`**| `Knob 5` | `TZ OFFSET` | `±5.0 ms` | `Profile B` | True 0ms zero-crossing point |
  | **`6: 8-Stage Phaser`** | `Knob 3, Knob 5` | `RESONANCE Q`, `POLE STAGES` | `0..100%`, `4..12 Poles` | `Profile F`, `Discrete` | Optical all-pass feedback and stage count |
  | **`7: Harmonic Tremolo`** | `Knob 3, Knob 5` | `CROSSOVER FREQ`, `HARMONIC / VINT` | `200..1200Hz`, `0..100%` | `Profile D`, `Profile B` | High/low crossover split & optical morph |
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (BBD Chip Core)`: `[ Panasonic MN3007 (Juno) | Reticon SAD1024 | Dimension Clean ]`
  * `DIP 2 (LFO Waveform)`: `[ Triangle Smooth | Dual Quadrature Sine | Random S&H Drift ]`
  * `DIP 3 (Clock Bleed Noise)`: `[ BBD Ultrasonic Clock Bleed On | Filtered Silent ]`
  * `DIP 4 (Phase Alignment)`: `[ Stereo Anti-Phase 180° | In-Phase Mono Chorus ]`
  * `TRIM 1`: BBD Compander Pre-Emphasis/De-Emphasis balance.
  * `TRIM 2`: 16-Voice Ensemble Detune Variance.
  * `TRIM 3`: Optical Phaser Vactrol Turn-Off Slew.
  * `TRIM 4`: Output Limiter Safety Ceiling.

---

### `MEGA-06`: `KlangShift` (Pitch, Granular & Heterodyne Lab)
* **Deep DSP Spec:** [`klangshift_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/klangshift_dsp.md) · [`freakshifter_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/freakshifter_dsp.md)
* **Upstream Reference:** [`moog_bode_1630_manual.md`](file:///C:/Dev/Research/library/manuals/moog_bode_1630_manual.md) · [`ROSETTA_STONE.md#section-d`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)
* **Architecture:** 5-engine pitch and heterodyne processor combining Eventide H3000 dual micro-pitch detune, scale-quantized granular harmonizer, 4-quadrant ring modulator, Leslie 122 rotary speaker, and dbx 120A sub-boom.
* **Front 8 Encoders:**
  1. `PITCH / SHIFT`: $\pm 24\text{ semitones}$ or Carrier Hz. Profile B / C. *Push: Octave Snap (12st)*.
  2. `FINE DETUNE`: $\pm 50\text{ cents}$ stereo detune. Profile B. *Push: Zero Fine Detune*.
  3. `GRAIN SIZE`: Granular window size ($10\text{ ms} \dots 250\text{ ms}$). Profile A. *Push: Snap 50ms*.
  4. `SCALE SNAP`: Scale Quantizer (Off, Chromatic, Project Key, Phrygian, Phrygian Dominant, Harmonic Minor). Discrete `0..5`. *Push: Cycle Scale*.
  5. `CARRIER WAVE`: Sine, Tri, Saw, Square (for Ring Mod / Bode modes). Discrete Choice. *Push: Next Carrier*.
  6. `FEEDBACK / SHIMMER`: Recirculating pitch-shift feedback ($0\% \dots 100\%$). Profile F. *Push: Feedback Invert*.
  7. `STEREO SPREAD`: Left Up / Right Down micro-offset. Profile B. *Push: Mono Matched*.
  8. `MIX`: Universal Tri-Zone Parallel Mix. Profile M. *Push: Phase Invert 180°*.
* **Contextual Front-Panel Remapping (Dynamic Mode Matrix):**
  | Active Heterodyne Engine | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
  | :--- | :--- | :--- | :--- | :--- | :--- |
  | **`2: 4-Quadrant Ring Mod`** | `Knob 1, Knob 4` | `CARRIER HERTZ`, `RING SQUELCH` | `0.1Hz..5kHz`, `0..100%` | `Profile C`, `Profile B` | Audio-rate carrier oscillator & noise gate |
  | **`3: Leslie 122 Rotary`** | `Knob 1, Knob 5, Knob 6` | `SPEED (SLOW/FAST)`, `TUBE DRIVE`, `ACCEL INERTIA` | `0.5..8Hz`, `0..24dB`, `0.5..8s`| `Profile C`, `Profile E`, `Profile A` | Rotating Doppler horn/drum & tube amp |
  | **`4: dbx 120A Sub-Boom`** | `Knob 1, Knob 5, Knob 6` | `SUB HARMONIC (f/2)`, `SINE FOLD`, `LOW CORNER` | `0..100%`, `0..100%`, `30..120Hz`| `Profile B`, `Profile E`, `Profile D` | Subharmonic octave synthesis & wavefold |
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Engine Select)`: `[ Dual Micro-Pitch Detune (H3000) | Granular Harmonizer | 4-Quadrant Ring Mod | Leslie 122 Rotary | dbx 120A Sub-Boom ]`
  * `DIP 2 (Grain Window Shape)`: `[ Hanning Smooth | Tukey Sharp Transient | Triangular ]`
  * `DIP 3 (Scale Root)`: `[ Follow MIDI Root Key | Fixed Global Project Key ]`
  * `DIP 4 (Feedback Highpass)`: `[ 80 Hz Low-Cut (Prevents Mud) | Full-Range Feedback ]`
  * `TRIM 1`: Grain Overlap Ratio ($2\times \dots 8\times$).
  * `TRIM 2`: Sub-Boom Phase Invert & Delay Alignment.
  * `TRIM 3`: Ring Modulator Carrier Feedthrough Null.
  * `TRIM 4`: Output Limiter Safety Ceiling.

---

### `MEGA-07`: `KlangDelay` (Echo, Delay & Repetition Lab)
* **Deep DSP Spec:** [`klangdelay_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/klangdelay_dsp.md) · [`echomill_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/echomill_dsp.md)
* **Upstream Reference:** [`roland_space_echo_re201_manual.md`](file:///C:/Dev/Research/library/manuals/roland_space_echo_re201_manual.md) · [`ROSETTA_STONE.md#section-d`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)
* **Architecture:** 6-engine master delay workstation combining digital ping-pong, Roland RE-201 Space Echo tape delay, Panasonic MN3005 BBD, reverse delay buffers, granular cloud freeze, and Tel-Ray electrostatic oil can delay.
* **Front 8 Encoders:**
  1. `TIME / DIV`: Tempo-synced $1/64 \dots 2\text{ bars}$ or $1\text{ ms} \dots 2000\text{ ms}$. Profile A / C. *Push: Free ms vs Tempo Division*.
  2. `FEEDBACK`: Regeneration ($0\% \dots 115\%$ runaway). Profile F. *Push: Dub Explosion Slam*.
  3. `TONE / DAMP`: Active high-cut & low-cut tape damping. Profile B. *Push: Flat Clean*.
  4. `ALGORITHM`: 6 engines (Digital Ping-Pong, Space Echo RE-201 Tape, Memory Man BBD, Reverse Delay Buffer, Granular Cloud Freeze, Oil Can Delay). Discrete `0..5`. *Push: Next Algorithm*.
  5. `DRIVE`: Analog tape preamp / BBD input saturation. Profile E. *Push: Clean vs Overdriven Preamp*.
  6. `MOD / FLUTTER`: Tape wow/flutter & BBD chorusing depth. Profile B. *Push: Pristine vs Warped Tape*.
  7. `STEREO OFFSET`: Ping-Pong cross-bounce width & delay time differential. Profile B. *Push: Center Dual Mono*.
  8. `MIX`: Universal Tri-Zone Parallel Mix. Profile M. *Push: Phase Invert 180°*.
* **Contextual Front-Panel Remapping (Dynamic Mode Matrix):**
  | Active Delay Algorithm | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
  | :--- | :--- | :--- | :--- | :--- | :--- |
  | **`3: Reverse Delay`** | `Knob 1, Knob 6` | `REVERSE CHUNK`, `WINDOW SLEW` | `50ms..2000ms`, `5..50ms` | `Profile A`, `Profile B` | Reverse slice size & zero-crossing de-click |
  | **`4: Granular Cloud`**| `Knob 2, Knob 6` | `FREEZE LATCH`, `GRAIN SPRAY` | `Toggle`, `0..100%` | `Discrete`, `Profile B` | Infinite buffer lock & stochastic grain jitter |
  | **`5: Oil Can Delay`** | `Knob 5, Knob 6` | `DRIVE / MOTOR DRAG`, `FLUID WOBBLE` | `0..100%`, `±100%` | `Profile E`, `Profile B` | Rotational friction & electrostatic pickup wobble |
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Tape Heads)`: `[ Single Head | RE-201 3-Head Multi-Tap | Binson Magnetic Wire Disc ]`
  * `DIP 2 (Motor Slew Physics)`: `[ Real-Time Pitch Slew (Doppler Warp on Time Change) | Crossfade Buffer ]`
  * `DIP 3 (Splice Bump & Degrade)`: `[ Physical Tape Joint Click On | Silent Digital Buffer ]`
  * `DIP 4 (Trails On Bypass)`: `[ Spillover Tails Decay | Hard Mute ]`
  * `TRIM 1`: Oil Can Electrostatic Fluid Wobble rate.
  * `TRIM 2`: Feedback Saturation Soft-Clipper Knee.
  * `TRIM 3`: Tape Head Bias Hysteresis.
  * `TRIM 4`: Output Limiter Safety Ceiling.

---

### `MEGA-08`: `KlangSpace` (Reverb & Acoustic Chamber Lab with Universal Gate)
* **Deep DSP Spec:** [`klangspace_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/klangspace_dsp.md) · [`steel_plate_dsp.md`](file:///c:/Dev/TheKlangSuite/docs/specs/dsp/steel_plate_dsp.md)
* **Upstream Reference:** [`emt_140_plate_manual.md`](file:///C:/Dev/Research/library/manuals/emt_140_plate_manual.md) · [`ROSETTA_STONE.md#section-d`](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)
* **Architecture:** 6-engine master reverb workstation combining EMT-140 physical steel plate, Lexicon 480L digital void hall, dual mechanical spring tanks, celestial shimmer, shoegaze reverse crescendo, and tight drum rooms, paired with a Universal Tri-Mode Gate.
* **Front 8 Encoders:**
  1. `DECAY / RT60`: Reverb tail duration ($0.2\text{ s} \dots 60.0\text{ s}$). Profile A (Cubic $x^3$). *Push: Infinite Reverb Freeze*.
  2. `PRE-DELAY`: $0\text{ ms} \dots 250\text{ ms}$. Profile A. *Push: Tempo Sync Division*.
  3. `DAMPING / HIGH-CUT`: High-frequency absorption ($1\text{ kHz} \dots 20\text{ kHz}$). Profile D Log. *Push: Bright / Dark*.
  4. `ALGORITHM`: 6 engines (Vintage Steel Plate EMT-140, Digital Void Hall Lexicon 480L, Spring Twang, Celestial Shimmer, Shoegaze Reverse, Tight Drum Room). Discrete `0..5`. *Push: Next Algorithm*.
  5. `DIFFUSION`: Early echo density ($0\% \dots 100\%$). Profile B. *Push: Grainy Clicks vs Smooth Wash*.
  6. `UNIVERSAL GATE`: Tri-Mode Gate (Off, Phil Collins 80s Snare Gate, Beats Gate [Tempo Synced $1/64 \dots 2\text{ bars}$], Level Gate dB Threshold). Discrete Choice. *Push: Cycle Gate Modes*.
  7. `SPATIAL WIDTH`: Stereo cross-coupling width ($0\%\text{ mono} \dots 200\%\text{ super-wide}$). Profile B. *Push: Mono Check*.
  8. `MIX`: Universal Tri-Zone Parallel Mix. Profile M. *Push: Phase Invert 180°*.
* **Contextual Front-Panel Remapping (Dynamic Mode Matrix):**
  | Active Reverb Algorithm | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
  | :--- | :--- | :--- | :--- | :--- | :--- |
  | **`3: Celestial Shimmer`** | `Knob 5` | `SHIMMER PITCH` | `+12st / -12st / +7st / +24st` | `Discrete` | Pitch shift interval inside the feedback tank |
  | **`4: Shoegaze Reverse`**| `Knob 1, Knob 5` | `RISE TIME`, `CRESCENDO SKEW`| `100ms..4000ms`, `±100%` | `Profile A`, `Profile B` | Non-linear swell duration & curve shape |
  | **`Active Gate Mode`** | `Knob 6` | `GATE DURATION / THRESH` | `10ms..1000ms / -60..0dB` | `Profile A / B` | Snaps tail shut in exact rhythmic lock or dB threshold |
* **Rear Deck ⚙️ (4 DIPs + 4 Trims):**
  * `DIP 1 (Reverb Engine Heritage)`: `[ EMT-140 Steel Plate | Lexicon 224 Hall | Lexicon 480L Rich Plate | Physical Dual Spring Tank ]`
  * `DIP 2 (DAC Resolution)`: `[ Vintage 12-Bit Fixed Point (224 Grit) | 32-Bit Floating Point Modern ]`
  * `DIP 3 (Gate Release Curve)`: `[ Hard Brickwall Choke (80s Drum) | Smooth Exponential Fade ]`
  * `DIP 4 (Input Highpass)`: `[ 80 Hz Sub Protection | Wide Bandwidth Full Sub ]`
  * `TRIM 1`: Spring Tank Chirp & Boing Dispersion.
  * `TRIM 2`: Modulator LFO Speed ($0.1\text{ Hz} \dots 2.5\text{ Hz}$).
  * `TRIM 3`: Shimmer Feedback Loop Headroom.
  * `TRIM 4`: Output Limiter Safety Ceiling.

---

## 3. Master Verification & Parity Invariant

This document represents the **Single Source of Truth (SSOT)** for all 12 sound engines, 18 bespoke audio effects, and 8 mega-effect workstations across The Klang Suite, ToadTracker, and The Klang Mill. 

* Whenever a UI card, APVTS parameter list, or tracker instrument screen is implemented, it **MUST match this specification 1:1**.
* No parameter may be renamed, dropped, or reduced without an explicit update to this canon.

