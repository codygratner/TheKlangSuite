# Technical Specification: `ReMod` (Impulse AM Modular Synthesis Engine)
### *1:1 Renoise Hybrid Instrument Standard, Dirac Impulse Oscillators, Universal 16-Control Presets & 8-Macro Dedicated Drum Architecture*

**Document Status:** CANONICAL SPECIFICATION 📜  
**Target Ecosystem:** ToadTracker (`TT`), The Klang Suite (`TKS`), TheKlangDSP (`TKDSP`), Renoise 3.4/3.5 (`.xrni`)  
**Designation:** `toad::ReMod` / `INST_TYPE_REMOD` / `CARD-15` (Modular Impulse Oscillator & Percussion Synthesizer)  
**Hardware & Platform Heritage:** Renoise Sample Modulation Architecture, Buchla 292 LPG, Mick Gordon DOOM Parallel DSP, Noise Engineering Modular FM, Vital Spectral Wavetable  
**Version:** 1.2.0  
**Date:** 2026-10-20  

---

## 1. Executive Summary & Design Manifesto

The **`ReMod`** engine formalizes Cody Gratner's signature sound-design workflow: turning **1-sample Dirac delta impulses** into rich, continuously oscillating subtractive, additive, and modular synthesizers via Renoise's **Amplitude Modulation (AM) filter bank**, coupled with non-linear multi-chain parallel DSP topologies (Mick Gordon DOOM architecture) and Cody's top production VST heuristics (Vital, Basimilus Iteritas, Streichfett, TAL-Pha, Snap Heap, Multipass, Lossy, Ruina, Disperser).

### 1.1 The Dedicated 8-Macro Per-Drum Paradigm
In traditional tracker drum kits, a single multi-sample instrument forces all drum pieces (kick, snare, hat, clap, tom, cymbal) to share 8 global macros—leaving only 1 or 2 controls per drum.
**In `ReMod`, every drum and synth is an autonomous, standalone instrument**:
1. **Renoise Native (`.xrni`)**: Each drum gets its own dedicated instrument panel with **ALL 8 MACROS WIRED AND ACTIVE** to sculpt the transient, body, filter, saturation, and decay of that specific drum in real time.
2. **ToadTracker & TheKlangDSP (`.toadinst`)**: Each instrument adheres to the **Universal 16-Control Card Canon**:
   - **Front Deck (`K01..K08`)**: Exactly mirrors the 8 Renoise performance macros 1:1.
   - **Rear Deck (`D09..D12` & `T13..T16`)**: Exposes algorithmic models, saturation topologies, envelope curves, micro-timing trims, and phase resets.
3. **Zero Audio Sample Bloat**: All instruments utilize 1-sample Dirac delta impulses (<6 KB total file size).

---

## 2. Mathematical DSP Architecture

### 2.1 The Dirac Delta Excitation
The voice excitation buffer is a single impulse looped forward:
$$\delta[n] = \begin{cases} 1.0 & n = 0 \\ 0.0 & n > 0 \end{cases}$$

### 2.2 The AM Filter Oscillator Formulation
Rather than filtering away harmonics, the AM filter stage functions as a pitch-tracked multiplier with Wet set to 100%:
$$y_{\text{AM}}[n] = x[n] \cdot \left[ (1 - W) + W \cdot \mathcal{M}_{\text{type}}(\omega_0 n + \phi) \right]$$
Where:
- $W = 1.000$ (100% wet pure oscillator mode).
- $\omega_0 = 2\pi \frac{f_{\text{MIDI}}}{f_s}$ (keyboard-tracked frequency).
- $\mathcal{M}_{\text{type}}$ is the continuous waveform generator:
  - **Type 18 (AM Sine)**: $\sin(\theta)$
  - **Type 19 (AM Triangle)**: $\frac{2}{\pi} \arcsin(\sin(\theta))$
  - **Type 20 (AM Saw)**: $2 \left( \frac{\theta}{2\pi} - \lfloor \frac{\theta}{2\pi} + \frac{1}{2} \rfloor \right)$
  - **Type 21 (AM Pulse)**: $\text{sgn}(\sin(\theta) - \text{PWM})$

### 2.3 Non-Linear Low-Pass Gate (LPG) Vactrol Model
In `ReMod-Bass` (Gross LPG mode), the vactrol optocoupler response is modeled with asymmetric slew kinetics:
$$\tau_{\text{rise}} \approx 1.5\,\text{ms}, \quad \tau_{\text{fall}}[n] = \tau_0 \cdot \left( 1 + \alpha \cdot e^{-\beta \cdot y[n-1]} \right)$$

---

## 3. The 16 Canonical ReMod Templates

### Category A: Drums & Percussion (10 Templates)

#### 1. `ReMod Kick 808` (`ImpulseKick_808.toadinst` / `ReMod_Kick_808.xrni`)
- **Front Deck / Renoise Macros (1..8)**:
  - `K01: PITCH` (30Hz - 120Hz)
  - `K02: SWEEP` (0st - 48st pitch drop)
  - `K03: DECAY` (50ms - 1800ms sub release)
  - `K04: SNAP` (-24dB - +12dB transient click)
  - `K05: DRIVE` (Tape saturation overdrive)
  - `K06: TONE` (High-cut damping filter)
  - `K07: SUB LEVEL` (Fundamental weight boost)
  - `K08: OUT LEVEL / PUNCH` (Master output)
- **Rear Deck (9..16)**: `D09: AM OSC WAVE`, `D10: SWEEP CURVE`, `D11: TRIGGER MODE`, `D12: SAT TOPOLOGY`, `T13: CLICK TIME`, `T14: HOLD TIME`, `T15: PITCH VEL`, `T16: PHASE RESET`.

#### 2. `ReMod Kick Punch 909` (`ImpulseKick_Punch909.toadinst` / `ReMod_Kick_Punch909.xrni`)
- **Front Deck / Renoise Macros (1..8)**:
  - `K01: ROOT PITCH` (35Hz - 90Hz)
  - `K02: PITCH DROP` (Dual-stage rapid attack envelope)
  - `K03: BODY DECAY` (40ms - 600ms tight punch)
  - `K04: BEATER CLICK` (Transient beater slap level)
  - `K05: FET DRIVE` (Limiter clipper drive)
  - `K06: TONE CUT` (Top end roll-off)
  - `K07: BODY WEIGHT` (Low end shelf boost)
  - `K08: OUT PUNCH` (Master compressor & punch)
- **Rear Deck (9..16)**: `D09: BEATER WAVE`, `D10: COMPRESSOR`, `D11: ENVELOPE LAW`, `D12: OVERSAMPLING`, `T13: ATTACK PUNCH`, `T14: DROP SPEED`, `T15: VELOCITY GAIN`, `T16: LOW SHELF Q`.

#### 3. `ReMod Snare Crisp` (`ImpulseSnare_Crisp.toadinst` / `ReMod_Snare_Crisp.xrni`)
- **Front Deck / Renoise Macros (1..8)**:
  - `K01: BODY TUNE` (120Hz - 320Hz tonal body)
  - `K02: BODY DECAY` (40ms - 500ms body ring)
  - `K03: SNAP DECAY` (50ms - 800ms noise wire sustain)
  - `K04: TONE<>NOISE` (Body vs noise crossfader)
  - `K05: DRIVE` (Clipper overdrive)
  - `K06: FILTER` (Wire bandpass filter frequency)
  - `K07: WIDTH` (StereoExpander spatial spread)
  - `K08: OUT LEVEL` (Master level)
- **Rear Deck (9..16)**: `D09: BODY WAVE`, `D10: NOISE TYPE`, `D11: TRIGGER MODE`, `D12: COMPRESSOR`, `T13: PITCH DROP`, `T14: DROP TIME`, `T15: SNARE BUZZ`, `T16: PRE-DELAY`.

#### 4. `ReMod Snare Fat 808` (`ImpulseSnare_Fat808.toadinst` / `ReMod_Snare_Fat808.xrni`)
- **Front Deck / Renoise Macros (1..8)**:
  - `K01: RESONATOR TUNE` (Dual-resonator pitch)
  - `K02: RESONATOR Q` (Head decay damping)
  - `K03: WIRE DECAY` (Snappy noise release)
  - `K04: SNAPPY BLEND` (Wire level)
  - `K05: WARMTH DRIVE` (Discrete transistor warmth)
  - `K06: HIGH CUT` (Smooth vintage filter cut)
  - `K07: SNAP CRACK` (Initial impact transient)
  - `K08: OUT PUNCH` (Master punch)
- **Rear Deck (9..16)**: `D09: RESONATOR RATIO`, `D10: NOISE FILTER`, `D11: FLAM GAP`, `D12: SATURATION`, `T13: CLICK DURATION`, `T14: WIRE RUMBLE HPF`, `T15: PITCH VELOCITY`, `T16: RES B FREQ`.

#### 5. `ReMod Hat Closed` (`ImpulseHats_Closed.toadinst` / `ReMod_Hat_Closed.xrni`)
- **Front Deck / Renoise Macros (1..8)**:
  - `K01: CARRIER FREQ` (6-Schmitt trigger cluster root)
  - `K02: SIZZLE / RES` (Bandpass metallic resonance peak)
  - `K03: CHOKE DECAY` (15ms - 600ms ultra-tight choke)
  - `K04: HPF FLOOR` (Sub-rumble cut frequency)
  - `K05: 12-BIT GRIT` (SP-1200 bit reduction drive)
  - `K06: AIR / TILT` (High shelf shimmer)
  - `K07: WIDTH` (Stereo widening)
  - `K08: OUT LEVEL` (Master level)
- **Rear Deck (9..16)**: `D09: OSC CLUSTER`, `D10: FILTER SLOPE`, `D11: VELOCITY LAW`, `D12: BIT DEPTH`, `T13: ATTACK TIME`, `T14: METALLIC RATIO`, `T15: SWING BIAS`, `T16: HPF FLOOR TRIM`.

#### 6. `ReMod Hat Open` (`ImpulseHats_Open.toadinst` / `ReMod_Hat_Open.xrni`)
- **Front Deck / Renoise Macros (1..8)**:
  - `K01: CLUSTER FREQ` (Metallic cluster tuning)
  - `K02: METALLIC RING` (Ring-mod cross-modulation)
  - `K03: WASH DECAY` (150ms - 2400ms open sizzle tail)
  - `K04: PEDAL CHOKE` (Choke group channel & mute velocity)
  - `K05: TAPE HEAT` (Warm saturation)
  - `K06: AIR SHIMMER` (12kHz+ air lift)
  - `K07: STEREO PAN` (Spatial divergence)
  - `K08: OUT LEVEL` (Master level)
- **Rear Deck (9..16)**: `D09: CHOKE MECHANIC`, `D10: RING MOD MODEL`, `D11: RELEASE CURVE`, `D12: AIR TOPOLOGY`, `T13: PEDAL SENSITIVITY`, `T14: RING MOD FREQ B`, `T15: STEREO COMB DELAY`, `T16: LOW CUT CEILING`.

#### 7. `ReMod Clap 808` (`ImpulseClap_808.toadinst` / `ReMod_Clap_808.xrni`)
- **Front Deck / Renoise Macros (1..8)**:
  - `K01: FLAM GAP` (6ms - 35ms burst spacing)
  - `K02: TAIL DECAY` (Reverb / noise release tail)
  - `K03: CLAP TONE` (Bandpass center frequency)
  - `K04: BURST DENSITY` (1 to 6 flam transients)
  - `K05: DRIVE CRUNCH` (Overdrive saturation)
  - `K06: ROOM WET` (Reverb room size & send)
  - `K07: STEREO SPREAD` (Dual-channel Haas offset)
  - `K08: OUT LEVEL` (Master level)
- **Rear Deck (9..16)**: `D09: BURST PATTERN`, `D10: ROOM REVERB MODEL`, `D11: FLAM SPACING LAW`, `D12: SATURATION`, `T13: INITIAL CLICK BOOST`, `T14: BANDPASS Q`, `T15: TAIL PRE-DELAY`, `T16: HPF CHOP`.

#### 8. `ReMod Perc Laser Zap` (`ImpulsePerc_LaserZap.toadinst` / `ReMod_Perc_LaserZap.xrni`)
- **Front Deck / Renoise Macros (1..8)**:
  - `K01: START FREQ` (1kHz - 14kHz zap start)
  - `K02: END FREQ` (30Hz - 300Hz end landing)
  - `K03: ZAP SPEED` (4ms - 120ms plunge time)
  - `K04: CURVE SHAPE` (Exponential pitch curve)
  - `K05: RESONANCE` (Self-oscillating filter peak)
  - `K06: DRIVE FOLD` (Wavefolder drive)
  - `K07: SLAP ECHO` (Slapback delay throw)
  - `K08: OUT LEVEL` (Master level)
- **Rear Deck (9..16)**: `D09: SWEEP OSC WAVE`, `D10: FILTER TOPOLOGY`, `D11: DELAY SYNC`, `D12: KEY FOLLOW`, `T13: SUB THUD WEIGHT`, `T14: ECHO FEEDBACK`, `T15: VELOCITY PITCH DEPTH`, `T16: FOLD THRESHOLD`.

#### 9. `ReMod Tom Resonant` (`ImpulseTom_Resonant.toadinst` / `ReMod_Tom_Resonant.xrni`)
- **Front Deck / Renoise Macros (1..8)**:
  - `K01: TUNE` (45Hz - 240Hz root pitch)
  - `K02: PITCH DROP` (Dynamic impact bend depth)
  - `K03: RING DECAY` (Membrane resonance decay)
  - `K04: STICK ATTACK` (Beater click transient level)
  - `K05: HEAD Q` (Resonance ring sustain)
  - `K06: DRIVE` (Warm saturation)
  - `K07: BEND SPEED` (Pitch drop rate)
  - `K08: OUT LEVEL` (Master level)
- **Rear Deck (9..16)**: `D09: MEMBRANE MODEL`, `D10: KEYBOARD TRACKING`, `D11: BEND CURVE`, `D12: STICK TRANSIENT`, `T13: RESONANCE HARMONIC 2`, `T14: STICK TONE FREQ`, `T15: VELOCITY FILTER SENS`, `T16: HEAD DAMPING RATE`.

#### 10. `ReMod Cymbal Ride Crash` (`ImpulseCymbal_RideCrash.toadinst` / `ReMod_Cymbal_RideCrash.xrni`)
- **Front Deck / Renoise Macros (1..8)**:
  - `K01: BELL PITCH` (300Hz - 2500Hz root bell)
  - `K02: BELL RING` (Tuned bell harmonic level)
  - `K03: WASH DECAY` (300ms - 4500ms sustain tail)
  - `K04: TRASH DIRT` (Ring-mod / lo-fi sizzle)
  - `K05: SHIMMER` (12kHz air sparkle)
  - `K06: DAMPING` (High frequency absorption)
  - `K07: STEREO COMB` (Comb filter widening)
  - `K08: OUT LEVEL` (Master level)
- **Rear Deck (9..16)**: `D09: CYMBAL PROFILE`, `D10: DIFFUSION TANK`, `D11: CHOKE LOGIC`, `D12: LO-FI FLAVOR`, `T13: BELL OVERTONE 1`, `T14: BELL OVERTONE 2`, `T15: STICK BEAT VELOCITY`, `T16: EDGE CRASH THRESHOLD`.

---

### Category B: Synths & Basses (6 Templates)

#### 11. `ReMod Gross LPG Bass` (`GrossLPG_Bass.toadinst` / `ReMod_GrossLPG_Bass.xrni`)
- **Front Deck / Renoise Macros (1..8)**:
  - `K01: CUTOFF` (20Hz - 20kHz Vactrol opening)
  - `K02: RESONANCE` (Vactrol peak resonance)
  - `K03: LPG DECAY` (Organic vactrol release slope)
  - `K04: WOBBLE RATE` (Stepper / LFO modulation speed)
  - `K05: DRIVE` (Wavefolder / saturation warmth)
  - `K06: OSC MIX` (AM Saw vs AM Pulse balance)
  - `K07: SUB GAIN` (Sub-octave fundamental weight)
  - `K08: OUT LEVEL` (Master level)
- **Rear Deck (9..16)**: `D09: LPG MODEL`, `D10: STEPPER PATTERN`, `D11: KEY TRACKING`, `D12: WAVE WARP`, `T13: VACTROL MEMORY`, `T14: STEPPER DEPTH`, `T15: LFO WAVE`, `T16: STEREO DETUNE`.

#### 12. `ReMod DoomRig Lead` (`DoomRig_Lead.toadinst` / `ReMod_DoomRig_Lead.xrni`)
- **Front Deck / Renoise Macros (1..8)**:
  - `K01: SINE DRIVE` (Pre-amp saturation drive)
  - `K02: CAB BITE` (4x12 Doom cabinet contour)
  - `K03: DIRT WET` (Parallel distortion balance)
  - `K04: SPACE SEND` (88s doom reverb send)
  - `K05: FB SQUEAL` (Comb filter feedback screaming)
  - `K06: RELEASE BURST` (Note-off squeal trigger level)
  - `K07: GLUE SQUASH` (16:1 limiter smash)
  - `K08: MASTER PUNCH` (Output gain)
- **Rear Deck (9..16)**: `D09: SOURCE WAVE`, `D10: CAB MODEL`, `D11: FB KEY TRACK`, `D12: COMP RATIO`, `T13: FLANGER RATE`, `T14: NOTE-OFF DURATION`, `T15: DUCKING SENS`, `T16: HIGH CUT CEILING`.

#### 13. `ReMod Basimilus BassPerc` (`Basimilus_BassPerc.toadinst` / `ReMod_Basimilus_BassPerc.xrni`)
*Heuristic derived from Noise Engineering Basimilus Iteritas (717 uses) + Ruina (97 uses) + Disperser (34 uses).*
- **Front Deck / Renoise Macros (1..8)**:
  - `K01: PITCH` (30Hz - 400Hz root)
  - `K02: DECAY` (20ms - 2000ms harmonic tail)
  - `K03: ATTACK PUNCH` (Disperser-style all-pass transient pop)
  - `K04: MORPH` (Sine $\to$ Triangle $\to$ Saw cross-harmonic blend)
  - `K05: FOLD` (Wavefolder drive / Ruina distortion)
  - `K06: HARMONICS` (Additive overtone spread)
  - `K07: SPREAD` (Stereo widening)
  - `K08: OUT LEVEL` (Master level)
- **Rear Deck (9..16)**: `D09: ALGORITHM`, `D10: FOLD TOPOLOGY`, `D11: TRANSIENT CHIRP`, `D12: PITCH ENVELOPE`, `T13: ALLPASS POLES`, `T14: MODULATOR FREQ RATIO`, `T15: VELOCITY FOLD DEPTH`, `T16: DC OFFSET COMPENSATION`.

#### 14. `ReMod Vital NeuroBass` (`Vital_NeuroBass.toadinst` / `ReMod_Vital_NeuroBass.xrni`)
*Heuristic derived from Vital Audio: Vital (775 uses) + Snap Heap (2,548 uses) + Multipass (656 uses) + Lossy (219 uses).*
- **Front Deck / Renoise Macros (1..8)**:
  - `K01: CUTOFF` (40Hz - 18kHz spectral filter)
  - `K02: RESONANCE` (Filter peak)
  - `K03: WOBBLE RATE` (LFO / Stepper modulation speed)
  - `K04: FORMANT` (Vowel formant sweep)
  - `K05: DRIVE / FOLD` (Wavefolder saturation)
  - `K06: SUB WEIGHT` (Low end fundamental boost)
  - `K07: LOSSY GRIT` (Bit reduction & digital codec artifacts)
  - `K08: OUT PUNCH` (OTT multi-band squash & master level)
- **Rear Deck (9..16)**: `D09: SPECTRAL WARP`, `D10: COMPRESSION`, `D11: LOSSY ARTIFACT`, `D12: STEREO SEPARATION`, `T13: BAND SPLIT LOW/MID`, `T14: BAND SPLIT MID/HIGH`, `T15: SUB SATURATION DRIVE`, `T16: SMOOTH OPERATOR RES DAMP`.

#### 15. `ReMod Streichfett Strings` (`Streichfett_Strings.toadinst` / `ReMod_Streichfett_Strings.xrni`)
*Heuristic derived from Waldorf Streichfett (87 uses).*
- **Front Deck / Renoise Macros (1..8)**:
  - `K01: OCTAVE DIVIDE` (Violin / Viola / Cello / Bass octave balance)
  - `K02: CRESCENDO` (Slow bowing attack envelope time)
  - `K03: RELEASE` (Reverberant release tail)
  - `K04: ENSEMBLE` (Tri-phase chorus depth & rate)
  - `K05: CHOIR FORMANT` (A-E-I-O-U vocal vowel morph)
  - `K06: BRIGHTNESS` (High shelf string sheen)
  - `K07: STEREO SPREAD` (Haas widening)
  - `K08: OUT LEVEL` (Master level)
- **Rear Deck (9..16)**: `D09: STRING VOICING`, `D10: ENSEMBLE CIRCUIT`, `D11: CHOIR VOWELS`, `D12: REVERB SPACE`, `T13: ENSEMBLE RATE`, `T14: VIBRATO DEPTH`, `T15: KEYBOARD TRACKING TILT`, `T16: CELLO OCTAVE WEIGHT`.

#### 16. `ReMod TALPha Hoover` (`TALPha_Hoover.toadinst` / `ReMod_TALPha_Hoover.xrni`)
*Heuristic derived from TAL-Pha / Alpha Juno (25 uses).*
- **Front Deck / Renoise Macros (1..8)**:
  - `K01: TUNE` (30Hz - 220Hz root pitch)
  - `K02: PWM SPEED` (Pulse width modulation LFO rate)
  - `K03: PWM DEPTH` (Pulse width modulation depth)
  - `K04: PITCH GLIDE` (Multi-stage pitch fall time / drop range)
  - `K05: CHORUS` (Dual-mode BBD stereo chorus)
  - `K06: SUB LEVEL` (Sub-oscillator pulse level)
  - `K07: FILTER CUT` (Lowpass brightness)
  - `K08: OUT LEVEL` (Master level)
- **Rear Deck (9..16)**: `D09: DCO WAVE MODEL`, `D10: CHORUS MODE`, `D11: PITCH FALL ENVELOPE`, `D12: SUB OSC TOPOLOGY`, `T13: PORTAMENTO TIME`, `T14: HIGH-PASS BASS BOOST`, `T15: PWM PHASE RESET`, `T16: FILTER KEY TRACK`.

---

## 4. Storage & Filesystem Architecture

```
C:/Dev/ToadTracker/assets/instruments/renoise/
├── kick/
│   ├── ImpulseKick_808.toadinst
│   └── ImpulseKick_Punch909.toadinst
├── snare/
│   ├── ImpulseSnare_Crisp.toadinst
│   └── ImpulseSnare_Fat808.toadinst
├── hats/
│   ├── ImpulseHats_Closed.toadinst
│   └── ImpulseHats_Open.toadinst
├── perc/
│   ├── ImpulseClap_808.toadinst
│   └── ImpulsePerc_LaserZap.toadinst
├── toms/
│   └── ImpulseTom_Resonant.toadinst
├── cymbal/
│   └── ImpulseCymbal_RideCrash.toadinst
├── bass/
│   ├── GrossLPG_Bass.toadinst
│   ├── Vital_NeuroBass.toadinst
│   └── TALPha_Hoover.toadinst
├── synth/
│   ├── Basimilus_BassPerc.toadinst
│   └── Streichfett_Strings.toadinst
└── doom/
    └── DoomRig_Lead.toadinst
```

*Mirrored natively to:* `C:\_\Dropbox\Renoise\instruments\templates\remod\` and category folders (`drums/kick/`, `drums/snare/`, `drums/hats/`, `drums/perc/`, `drums/toms/`, `drums/cymbal/`, `bass/`, `synth/`).
