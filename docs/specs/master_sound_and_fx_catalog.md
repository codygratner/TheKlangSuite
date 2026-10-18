# The Master Sound Engine & Effects Catalog (TKS, ToadTracker & The Klang Mill)

**Document Status:** CANONICAL ARCHITECTURAL REFERENCE 📜  
**Target Milestones:** v0.4.0 (UI/UX Redesign & Topology) & v0.5.0 ("The Sound & Chaos Update")  
**Ecosystem Pillars:** The Klang Suite (`TKS`), ToadTracker (`TT`), The Klang Mill (`TKM`)  
**Hardware Readiness:** dadamachines TBD-16 (Pages 1A/1B), ESP32-P4 RISC-V, Universal Desktop VST3/CLAP  
**Date:** 2026-10-15  

---

## 1. Executive Overview & The Unified Architectural Standard

This master specification establishes the definitive architecture, parameter contracts, and real-time DSP topologies for all **Sound Engines (Oscillators)** and **Audio Effects (FX)** across The Klang Suite ecosystem.

### 🔗 Related Architectural Specifications
- [**`the_klang_r1.md`**](file:///c:/Dev/TheKlangSuite/docs/specs/spinoffs/the_klang_r1.md): Dedicated ER-1 tribute drum synthesis engine specification.
- [**`fx_catalog_expansion.md`**](file:///c:/Dev/TheKlangSuite/docs/specs/fx_catalog_expansion.md): Detailed DSP equations and parameter tables for expanded FX algorithms.
- [**`dsp_parameter_tapering_and_skew_spec.md`**](file:///c:/Dev/TheKlangSuite/docs/specs/dsp_parameter_tapering_and_skew_spec.md): Canonical perceptual tapering, warping ($y = x^3$), and skew standards for v0.5.0 APVTS parameters.
- [**`v040_neo_slate_ux.md`**](file:///c:/Dev/TheKlangSuite/docs/specs/v040_neo_slate_ux.md): Neo-Slate spatial layout, responsive docking, and card topologies.
- [**`tbd16_klang_seed_effects.md`**](file:///c:/Dev/TheKlangSuite/docs/specs/tbd16_klang_seed_effects.md): Hardware mapping and TBD-16 encoder bindings.

---

## 2. Universal Hardware & Ergonomic Standards

### 🎛️ The Universal 8-Control Card Layout & Dual-Deck Architecture
All sound engines, mega-effects, and bespoke processors follow the **Universal 8-Control Card Standard**, designed specifically for 4 endless push-encoders on the dadamachines TBD-16 across Page 1A and Page 1B.

```
       ┌────────────────────────────────────────────────────────┐
       │  [ ⚡ PERFORM ]              [ 🔬 SOUND LAB (Deck ⚙️) ] │
       ├────────────────────────────────────────────────────────┤
       │  KNOB 1        KNOB 2        KNOB 3        KNOB 4      │
       │  (Page 1A)     (Page 1A)     (Page 1A)     (Page 1A)   │
       │                                                        │
       │  KNOB 5        KNOB 6        KNOB 7        KNOB 8      │
       │  (Page 1B)     (Page 1B)     (Page 1B)     (Page 1B)   │
       └────────────────────────────────────────────────────────┘
```

1. **`⚡ PERFORM MODE` (Live & Sequencer View):**
   - High-contrast, streamlined 8-knob front faceplate.
   - Protected against accidental parameter corruption during live performance.
   - Encoder pushes trigger safe performance modifiers (Mute, Solo, Reset, Octave Lock, Polarity Invert).
2. **`🔬 SOUND LAB MODE` (Architecture & Deep Edit View):**
   - Unrolls the card horizontally into a **Side-by-Side Dual-Wing View**:
     - **Left Wing:** The 8 continuous performance controls (Front Faceplate).
     - **Right Wing:** Calibration trimpots, sub-engine selectors, choke groups, and routing switches (Back of the Panel / Rear Deck ⚙️).

---

### 🎚️ Ergonomic DSP Tapering Profiles
All continuous parameters adhere to the canonical mathematical curves defined in `dsp_parameter_tapering_and_skew_spec.md`:
* **`Profile A` (Cubic Decay $y = x^3$):** Sub-second precision ($10\,\text{ms} \dots 10.0\,\text{s}$ Snappy / $60.0\,\text{s}$ Drone).
* **`Profile B` (Bipolar Sigmoid S-Curve):** Deadzone plateau at 12:00 for bipolar controls, tilt filters, and DJ sweeps.
* **`Profile C` (Dual-Regime Geometric Skew):** LFOs and pitch modulations with 12:00 parked at $20\,\text{Hz}$.
* **`Profile D` (Logarithmic Cutoff):** Equal-octave filter sweeps ($20\,\text{Hz} \dots 20\,\text{kHz}$) with 12:00 at $800\,\text{Hz}$.
* **`Profile E` (Warped Saturation $y = x^{1.8}$):** Tube and non-linear distortion drive curves.
* **`Profile F` (Hyperbolic End-Stop):** Feedback loops expanding from $70\%$ to $115\%$ self-oscillating runaway.
* **`Profile M` (Universal Tri-Zone Parallel Mix Law):**
  - **7:00 (Full CCW):** $100\%$ Pure Wet (Dedicated send / solo).
  - **12:00 (Center):** $100\%$ Pure Dry (True bypass / zero phase alteration).
  - **5:00 (Full CW):** $100\%$ Dry + $100\%$ Parallel Wet (Maximum parallel punch with zero transient loss!).
  - **Encoder Push:** **Phase Invert ($\varnothing$)** on the wet path across all effects cards.

---

### 🏢 The Master Branding Protocol: Klang Maximum vs. Corporate Boring
To eliminate clutter on individual card rear decks, branding mode is governed by **ONE Master Global Toggle** in the Settings Deck (and Top Header):
`[ 👑 KLANG MAXIMUM (Default) | 🏢 CORPORATE BORING (Serious Mode) ]`

* **Global Ecosystem Scope:** Toggling this switch in Settings updates every card across The Klang Farmer, ToadTracker, and The Klang Mill simultaneously.
* **Standalone Plugin Invariant:** Standalone VST3/CLAP spinoff plugins **ALWAYS STAY IN KLANG MODE PERMANENTLY**.

---

### 🛡️ The Trademark Sanitization Matrix
All hardware and circuit references have their serial numbers filed off:

| Vintage / Commercial Reference | Sanitized Canonical Name | Character & Circuit Architecture |
| :--- | :--- | :--- |
| **Korg Nu:Tekt HD-S** | **`Cerberus`** | 3 parallel distortion heads: Warmth, Vowel, and Crossover bite |
| **EDP Wasp Filter / Doepfer A-124** | **`Hornet`** | CMOS CD4069UB leaky inverter SVF with "sandy" resonance |
| **Arturia / Steiner MiniBrute** | **`Vandal Park`** | Steiner-Parker diode SVF with "Vandal Factor" feedback |
| **Moog Ladder Filter** | **`Transistor Ladder`** | 4-pole cascaded BJT ladder with thermal voltage saturation |
| **Roland TB-303 Filter** | **`Diode Acid Squelch`** | Diode ladder filter with resonance-induced cutoff slump |
| **Sherman Filterbank 2** | **`Belgian Warbank`** | Dual analog filter, audio-rate FM, tube overdrive, self-oscillation |
| **Roland Alpha Juno IR3R05** | **`Alpha Dual-12`** | Dual cascaded -12dB SVF cores with dual independent resonance |
| **Korg MS-20 Filter** | **`Screamer 35`** | Korg35 OTA non-linear screaming filter |
| **Pultec EQP-1A Trick** | **`Passive Bass Contour`** | Simultaneous low boost + mid-bass cut resonant shelf |
| **Buchla 259/292 Wavefold + APF Loop** | **`Argent Vortex`** | Recirculating Buchla wavefolder & all-pass phase dispersion chaos loop |
| **WMD Geiger Counter / ShaperTable** | **`PipKlick`** | 256-wavetable amplitude waveshaping & bit-mask glitch |
| **Ableton + Argent DOOM** | **`Argent Anvil`** | Multiband dynamics with +100dB upward DOOM compression |
| **Empirical Labs Distressor** | **`The Annihilator (NUKE)`** | Brickwall limiter with reverse knee + Dist 2/3 harmonics |
| **Kilohearts Disperser** | **`Smear`** | Cascaded all-pass phase dispersion network |
| **Boss HM-2** | **`Swedish KlangSaw`** | Unbiased complementary BJT dead-band + dual gyrators |
| **Boss MT-2 Metal Zone** | **`KlangZone`** | Dual op-amp hard clip + sweepable semi-parametric mid EQ |
| **ProCo RAT** | **`Rodent`** | LM308 slew-rate limiting distortion + reverse tone filter |
| **Ibanez TS9 / TS808** | **`Green Screamer`** | Symmetrical feedback diodes + 720 Hz high-pass rolloff |
| **Klon Centaur** | **`Ixion Guild`** | Dual-gang clean blend + germanium clip + 18V charge pump |
| **Electro-Harmonix Big Muff** | **`Russian KlangPi`** | 4-stage discrete BJT booster with 1 kHz mid-notch tone stack |
| **DOD 250 / MXR Dist+** | **`Yellow 250`** | LM741 op-amp overdrive with back-to-back clipping diodes |
| **Fuzz Face** | **`Velcro Face`** | 2-transistor Germanium/Silicon direct-coupled voltage starve |
| **Doepfer A-196 PLL** | **`PhaseShriek`** | Phase locked loop tracking oscillator with XOR tearing |
| **Doepfer A-110-4 Thru-Zero** | **`ZeroCore`** | Linear thru-zero quadrature oscillator crossing 0 Hz |
| **Doepfer A-188-1 Bucket Brigade** | **`ClockBleed`** | MN3005 BBD delay line with clock bleed & compander warmth |
| **Doepfer A-149-1 Uncertainty** | **`Heisenberg`** | Quantized 2^N stepped voltage & stored continuous random |
| **Electrix WarpFactory Vocoder** | **`56K Vocoder`** | 32-band vocoder with MKS-50 carrier, vocal tracker & freeze |
| **Noise Eng Basimilus Alter** | **`Zombie`** | 6-sine additive/FM drum voice with wavefolder |
| **Noise Eng Manis Iteritas** | **`Ghost`** | Dual saw/noise into low-pass gate and dual wavefolder |
| **Noise Eng Desmodus Versio** | **`Vampire`** | 32-delay FDN modulated synthetic reverb tail generator |
| **ALM Busy Circuits MCO** | **`KCO`** | Dual morphing wavetable voice with Alpha Juno PWM & sub |
| **ALM Busy Circuits Tyso Daiko** | **`DualDaiko`** | Dual-voice 12-bit wavetable & FM drum with analog tilt EQ |
| **ALM Busy Circuits Pamela's** | **`ClockWorx`** | 8-channel clocked Euclidean modulation brain |
| **ALM Busy Circuits Squid Salmple** | **`OctoSampler`** | 8-channel independent multi-trigger sampler with real-time scrub |
| **ALM Busy Circuits ASQ-1** | **`TactSeq`** | 2 CV/gate melodic + 4 trigger drum tracker sequencer |
| **Hydra 1:8 Modulator** | **`OctoMod`** | Master LFO/Envelope 1-to-8 fan-out modulation routing matrix |
| **Roland JP-8000 Supersaw** | **`KG-8080`** | 7-voice detuned unison sawtooth oscillator array |
| **Lexicon 224 / 480L** | **`Digital Void Hall`** | 12-bit Dattorro continuous algorithmic hall |
| **EMT-140 Plate** | **`Vintage Steel Plate`** | High-density physical steel plate reverberator |
| **Roland TR-808 + Mick Gordon** | **`Argent Slayer`** | Phase-locked sub-bass with recursive wavefolder & noise ring |
| **Roland TR-606/808 & Paiste Bronze** | **`BronzeLab`** | 4 metallic square/FM cluster cores through 8-stage all-pass dispersion |
| **Game Boy DMG-01** | **`Pocket Matrix`** | 4-bit 32-sample wave channel, dual pulse & LFSR noise |
| **NES 2A03** | **`Family 8`** | 4-bit stepped triangle bass & non-linear DAC curves |
| **Konami VRC6** | **`Demon Castle Saw`** | 6-bit stepped saw generator & 8-step variable pulse |
| **Commodore 64 SID** | **`Breadbin SID`** | MOS SID tri/saw/pulse with audio-rate Hard Sync & Ring Mod |
| **PC Engine / TG-16** | **`Turbo Engine 16`** | 6-channel 5-bit 32-sample wavetable with channel cross-FM |
| **5-Console Chiptune Lab** | **`K-Chip`** | Cycle-accurate 5-console retro chiptune engine |
| **Mutable Instruments Plaits & Braids** | **`Kaleidocore`** | Macro timbral synthesizer, 6-op DX7 FM, SAM speech, wave terrain |
| **Dirtywave M8 Wavsynth** | **`SqueezeFold`** | Harmonic cycle-multiplier, phase-squeezer & wavefolder |
| **EMS Vocoder 2000 / Roland VP-330** | **`KlangVocoder`** | 32-band spectral analysis/synthesis vocoder with sibilance replacement |
| **Mutable Instruments Clouds & Beads** | **`Nimbus`** | Granular texture, pitch-time dislocation, buffer freeze & diffusion |
| **Korg Volca Drum Wave Guide** | **`Pipe & Wire`** | Tuned cylindrical pipe (tube) & stretched wire (string) resonator |
| **Harald Bode Frequency Shifter** | **`FreakShifter`** | Dual 90-degree Hilbert transform all-pass network with antiphase shift |
| **The Klang Mill Delay** | **`EchoMill` (`TKMD`)** | Recursive 2-slot insert-rack echo & degradation mill |

---

## 3. The 9 S-Tier Sound Engines (Oscillator Catalog)

### 🏢 Sound Engine Corporate Boring Naming Matrix
When the Master Settings Toggle is set to **`🏢 CORPORATE BORING`**, all sound engines strip their creative lore and trademark references into utilitarian engineering names:

| Engine Code | 👑 Klang Maximum (Default) | 🏢 Corporate Boring (Serious Mode) | Pedagogical & Heritage Circuit Architecture |
| :--- | :--- | :--- | :--- |
| **`OSC-01`** | **`The R-1`** *(Plugin: `The Klang R-1`)* | **`Percussion Synthesizer`** | Carrier (Sine/Tri) + Cyclic Ring-Modulated Noise |
| **`OSC-02`** | **`AlphaKraft`** | **`Dual DCO & Unison Synthesizer`** | Dual-core unipolar XOR DCO + 7-voice Supersaw stack |
| **`OSC-03`** | **`CosmoPhase`** | **`Phase Distortion Synthesizer`** | Dual-line cosine angle warping + EA-1 Hard Sync |
| **`OSC-05`** | **`Pluck`** *(FX: `Pluck FX`)* | **`Physical Waveguide & Modal Resonator`** | Karplus waveguide + 24 modal modes + sympathetic strings (Mono & Poly) |
| **`OSC-09`** | **`Argent Slayer`** | **`Phase-Locked Sub-Bass Generator`** | MIDI-tracked sub sine + recursive sine wavefold & noise |
| **`OSC-11`** | **`BronzeLab`** | **`Inharmonic Metallic Generator`** | 4 metallic cores + 8-stage all-pass bronze dispersion |
| **`OSC-12`** | **`K-Chip`** | **`Programmable Sound Generator (PSG)`**| 5 retro gaming chips (Pocket Matrix, Family 8, Demon Castle Saw, Breadbin SID, Turbo Engine 16) |
| **`OSC-13`** | **`Kaleidocore`** | **`Macro Timbre Synthesizer`** | MI Plaits + Braids Super-Engine + 6-Op DX7 FM + SAM C64 Speech + Wave Terrain |
| **`OSC-14`** | **`SqueezeFold`** | **`Wave-Multiply & Fold Synthesizer`** | M8-Style Core: Single-cycle multiplier, phase squeeze & wavefold |

---

### 🔊 OSC-01: The R-1 (ER-1 Tribute Drum Core) — [S-TIER / LOCKED]
*(Standalone Spinoff: **`The Klang R-1`** | Corporate Boring: **`Percussion Synthesizer`**)*
* **Pedigree:** Clean-room emulation of the 1999 Korg Electribe R percussion synthesizer voice.
* **Architecture:** Carrier oscillator (Sine $\leftrightarrow$ Triangle) frequency-modulated and amplitude-modulated by a multi-mode modulation generator. Pitch sweeps are handled via the Farmer bipolar Pitch Envelope. Mod Type 5 generates continuous white noise ring-modulated with a sine wave at `Mod Speed` ($M(t) = \text{Noise}(t) \times \sin(2\pi f_{\text{speed}} t)$), producing rhythmic chatter at LFO rates and metallic snare sizzle at audio rates.
* **8 Performance Knobs:** `PITCH` (Hz / Note), `WAVE` (Sine $\leftrightarrow$ Tri), `DECAY` (Snappy $10\,\text{ms} \dots 10.0\,\text{s}$ / Drone $60.0\,\text{s}$), `LEVEL` (dB), `MOD TYPE` (Saw, Square, Tri, S&H, Cyclic Noise), `MOD SPEED` ($0.1\,\text{Hz} \dots 5\,\text{kHz}$), `MOD DEPTH` ($-100\% \dots +100\%$), `FEEDBACK` (Self-mod intensity).

---

### 🔊 OSC-02: AlphaKraft (Alpha Juno Dual-DCO + JP-8000 Stack) — [S-TIER / LOCKED]
*(Corporate Boring: **`Dual DCO & Unison Synthesizer`**)*
* **Pedigree:** Roland Alpha Juno 1/2 & MKS-50 custom Fujitsu MB87123 DCO ASIC crossed with Roland JP-8000 Supersaw architecture. Monophonic, percussive, ripping electronic basslines and snarling Hoover leads.
* **Architecture:** Dual-core parallel DCO running two independent unipolar XOR wave generators with PolyBLEP anti-aliased step transitions. All 14 waveforms are normalized to the single-cycle fundamental period $[0.0, 1.0)$ so any wave can be played at any pitch without hardwired octave drops. Wave 1 feeds a JP-8000-style `SUPER` stack (up to 7 detuned PolyBLEP copies) while Wave 2 acts as a rock-solid anchor (sub, nasal 5th, or secondary layer). Features audio-rate PWM ($0.02\,\text{Hz} \dots 2.5\,\text{kHz}$) and fine bipolar detune ($-12 \dots +12$ semitones).
* **Waveforms (14 Shapes):** Saws 1–5 (Linear, PWM Saw, Stepped Saw 1/2, Double Saw), Squares 1–3 (Square, Pulse PWM, Narrow Spike), Shapes 9–14 (Sub 1/2, Asymmetric Notch, Dual Unequal, Stepped Pulse 1/2).
* **8 Performance Knobs:** `WAVE 1` (14 Shapes), `OCTAVE` ($-4 \dots +4$), `WAVE 2` (Off + 14 Shapes), `SUPER` ($0\% \dots 100\%$ voice density), `PWM AMOUNT` ($0\% \dots 100\%$), `PWM SPEED` ($0.02\,\text{Hz} \dots 2.5\,\text{kHz}$), `DETUNE` ($-12 \dots +12$ st), `RETRIGGER` (`FREE` DCO, `RESET` $0^\circ$, `RANDOM`).

---

### 🔊 OSC-03: CosmoPhase (Casio CZ Twin-Line Phase Distortion) — [S-TIER / LOCKED]
*(Corporate Boring: **`Phase Distortion Synthesizer`**)*
* **Pedigree:** Casio CZ-101 / CZ-1000 / CZ-1 crossed with Korg EA-1 sync and modern audio-rate phase distortion.
* **Architecture:** Dual-line Phase Distortion (PD) engine running two independent DCO-DCW lines. Replaces recursive IIR filters with non-linear cosine angle warping ($\theta_{distorted} = f(\theta_{linear}, \text{DCW})$), generating razor-sharp resonant sweeps with zero filter phase lag or transient smearing. Features a 7-mode interaction matrix (including Korg EA-1 style wide-range Hard Sync up to +5 octaves, Kilohearts-style audio-rate Phase Distortion, and Logic XOR) paired with a skeuomorphic Backstage DIP switch governing the mixing mathematics.
* **8 Algorithms:** Saw, Square, Pulse, Double Sine, Saw-Pulse, Reso 1 (Saw sync), Reso 2 (Tri sync), Reso 3 (Trap sync).
* **7 Interaction Modes:** Normal ($L_1 + L_2$), Ring Mod ($L_1 \times L_2$), Phase Distort ($\theta_2' = \theta_2 + k L_1$), Cross Mod, Hard Sync ($L_1$ resets $L_2$), Logic XOR, Min/Max Wavefold.
* **8 Performance Knobs:** `WAVE 1`, `PD 1` (DCW 1), `WAVE 2`, `PD 2` (DCW 2), `OCTAVE`, `DETUNE 2` ($-1 \dots +5$ Oct), `AMP MIX`, `INTERACTION` (7 Modes).
* **Rear Deck DIP:** `MIX LAW` (`SAT BOOST` Padé Tanh, `DIODE SPLICE` Max/Min, `STEREO SPREAD` Vector M/S).

---

### 🔊 OSC-05: Pluck (Physical Waveguide & Modal Resonator) — [S-TIER / LOCKED]
*(Companion VST Effect: **`Pluck FX`** | Corporate Boring: **`Physical Waveguide & Modal Resonator`**)*
* **Pedigree:** Kevin Karplus & Alex Strong waveguide physical modeling merged with Mutable Instruments Rings modal synthesis and sympathetic strings. Plays in monophonic slap/rubbery mode or 4-voice polyphonic / paraphonic strumming mode. The companion **`Pluck FX`** plugin exposes external sidechain inputs to excite and resonate any external audio track through the tuned string/modal body.
* **Pedigree:** Kevin Karplus & Alex Strong physical modeling crossed with Elektron Octatrack tuned comb filter techniques. Monophonic, overdriven physical model engineered specifically for rubbery sub-bass, slap bass, kalimbas, and metallic percussion.
* **Architecture:** Short excitation burst (White Noise, Pink Noise, Click, Saturated Thump) injected into a tuned circular delay line with fractional-delay Lagrange pitch interpolation and a 1-pole low-pass loss filter. Features a variable pick position comb filter, all-pass string stiffness/inharmonicity dispersion, a secondary wooden body cavity resonator, and internal Padé tanh loop saturation for singing sustain and warm bass drive.
* **8 Performance Knobs:** `OCTAVE` ($-4 \dots +4$ Oct), `DECAY` ($RT_{60}$ $10\,\text{ms} \dots 10.0\,\text{s}$ Snappy / $60.0\,\text{s}$ Drone), `DAMPING` (HF loss filter), `EXCITER` (Encoder Turn: burst length; Encoder Press: White Noise, Pink Noise, Click, Thump), `PICK POSITION` (0.05 bridge bite $\leftrightarrow$ 0.50 hollow center), `STRETCH` (All-pass inharmonic stiffness), `BODY RES` (Wooden cavity resonance), `DRIVE` (Padé tanh loop drive).

---

### 🔊 OSC-09: Argent Slayer (Phase-Locked Sub & Sine Wavefold) — [S-TIER / LOCKED]
*(Corporate Boring: **`Phase-Locked Sub-Bass Generator`**)*
* **Pedigree:** Roland TR-808 kick oscillator crossed with Mick Gordon (DOOM 2016 / Eternal) pseudo-acoustic sub-harmonic synthesis techniques.
* **Architecture:** Dual-layer sub engine. Core 1 is a pure fundamental sine oscillator locked to MIDI pitch ($-4 \dots +4$ Octaves) with a continuous exponential decay tail up to $60.0\,\text{s}$. Core 2 is a selectable transient click / attack harmonic layer. Core 1 feeds a Mick Gordon recursive sine wavefolder ($f(x) = \sin(x \cdot \text{fold})$) with DC symmetry bias (even harmonics) and an audio-rate Noise Ring-Modulation circuit ($Sub \times Noise$) that generates a tearing pseudo-acoustic harmonic halo, ensuring earth-shaking sub-bass is clearly audible on iPhone and laptop speakers.
* **8 Performance Knobs:** `OCTAVE` ($-4 \dots +4$, tracked to MIDI), `SUB DECAY` ($10\,\text{ms} \dots 10.0\,\text{s}$ Snappy / $60.0\,\text{s}$ Drone), `LAYER 2 TYPE` (Press: Analog Click, Acoustic Mallet, 2nd Harm, 3rd Harm, Dirt Noise), `LAYER 2 MIX` ($0\% \dots 100\%$), `SINE FOLD` (Mick Gordon wavefolder drive), `NOISE RING` (Sub $\times$ Noise heterodyne sidebands), `SYMMETRY` (DC offset even harmonics), `PHASE LOCK` ($0^\circ$ Lock for punch $\leftrightarrow$ Free for continuous leads).

---

### 🔊 OSC-11: BronzeLab (Paiste Bronze & Metallic Clatter) — [S-TIER / LOCKED]
*(Corporate Boring: **`Inharmonic Metallic Generator`**)*
* **Pedigree:** Roland TR-606 & TR-808, Boss DR-110, Korg Volca Beats, Au5 FM unison, and all-pass phase dispersion physical bronze modeling.
* **Architecture:** Hybrid metallic percussion laboratory:
  1. **4 Metallic Cores:** TR-606/808 6-Square Schmitt cluster (~205, 305, 365, 520, 800, 875 Hz), DR-110 4-Square + Pink Noise, Au5 Chaotic FM Unison, Volca Grain 10-bit clatter.
  2. **8-Stage All-Pass Phase Dispersion:** Smears the phase across frequencies, instantly transforming harsh synthetic square buzz into liquid acoustic Paiste bronze sheen.
  3. **Decoupled Dual Envelopes:** Independent `BODY DECAY` ($10\,\text{ms} \dots 10.0\,\text{s}$ Snappy / $60.0\,\text{s}$ Drone) for bandpass bell/cup ring, and `SIZZLE DECAY` ($10\,\text{ms} \dots 10.0\,\text{s}$ Snappy / $60.0\,\text{s}$ Drone) for high-pass splash wash.
  4. **TR-606 "Slosh" & RC Choke:** Continuous `SLOSH` control emulating the TR-606 discharging capacitor curve (delivering authentic black metal blast-beat half-open hat slosh).
  5. **Opposite Acoustic Bloom Pitch Envelope:** Target 1 dives down while Target 2 sweeps up, recreating outward turbulent acoustic plate dispersion!
* **8 Performance Knobs:** `PITCH` (Root tuning), `BODY DECAY` ($10\,\text{ms} \dots 10.0\,\text{s}$ / $60.0\,\text{s}$), `INHARMONIC` (Detune spread; Press: Cycle 4 Cores), `SIZZLE DECAY` (Air splash time), `DISPERSION` (8-stage all-pass diffusion; Press: Toggle Wavefolder), `FILTER / COLOR` (Bandpass sweep $2\,\text{kHz} \dots 12\,\text{kHz}$), `NOISE & STICK` (High wash blend; Press: Cycle Stick Impact), `SLOSH` (TR-606 RC discharge curve; Press: Cycle Choke Mode).

---

### 🔊 OSC-12: K-Chip (Vintage Console & Chiptune Lab) — [S-TIER / LOCKED]
*(Corporate Boring: **`Programmable Sound Generator (PSG)`**)*
* **Pedigree:** Nintendo Game Boy DMG-01, NES 2A03, Konami VRC6, Commodore 64 SID 6581/8580, and PC Engine / TurboGrafx-16 HuC6280.
* **Architecture:** Authentic, cycle-accurate multi-chip emulation engine uniting the 5 greatest retro gaming sound chips into a single unified performance card. Includes hardware pitch sweeps (coin/laser drops), stepped volume envelopes, authentic resistor ladder DAC non-linearities, and cross-channel modulation.
* **The 5 Chip Cores:**
  1. `Pocket Matrix` (Handheld 4-Bit Core): 4-bit 32-sample Wave Channel + dual pulse + 7/15-bit Galois LFSR noise.
  2. `Family 8` (Console 8-Bit Core): Stepped 4-bit staircase Triangle wave (the punchiest chiptune sub-bass in history) + pulse.
  3. `Demon Castle Saw` (6-Bit Sawtooth Core): 6-bit stepped saw generator (aggressive Castlevania power) + 8-step pulse.
  4. `Breadbin SID` (3-Voice SID Core): Tri/Saw/Pulse/Noise with audio-rate Hard Sync, Ring Mod, and dirty multi-mode filter.
  5. `Turbo Engine 16` (5-Bit Wavetable Core): 5-bit 32-sample custom wavetable synthesis with channel cross-FM!
* **8 Performance Knobs:** `OCTAVE` ($-4 \dots +4$), `CHIP CORE` (Selects 5 chips; Press: Cycle voice channel), `WAVE / DUTY` (Pulse duty, 4-bit tri, 6-bit saw, wavetable), `HARD SWEEP` (Hardware pitch sweep; Press: Flip Up/Down), `DAC RESOLUTION` (4, 5, 8-bit quantization vs. PolyBLEP), `CHIP MOD` (SID Hard Sync / Ring Mod / TG-16 FM), `NOISE MASK` (LFSR noise blend; Press: 7-bit periodic clatter vs 15-bit white hiss), `DECAY / VOL` (Hardware stepped envelope $0 \dots 15$ steps / smooth decay).

---

### 🔊 OSC-13: Kaleidocore (MI Plaits & Braids Unified Super-Engine) — [S-TIER / LOCKED]
*(Corporate Boring: **`Macro Timbre Synthesizer`**)*
* **Pedigree:** Émilie Gillet (Mutable Instruments Plaits & Braids), Plaits Firmware 1.2 "Orange Bank", Rubato Audio Plaits Palette, Braids Renaissance (Tom Burns SAM speech), Lyle P. Mills chord tables, and Bees-in-the-Trees (Tim Churches paraphonic multi-osc).
* **Architecture:** The ultimate macro-timbral sound laboratory, uniting all 24 Plaits models, the cult-classic Braids synthesis algorithms, and community mods into a unified dual-bank macro engine. Provides 1:1 preset mapping for the 556 `Macrosynth` songs in the Dirtywave M8 library while providing full continuous 96kHz floating-point synthesis. Features an integrated virtual Low-Pass Gate (VCFA) with selectable opto-vactrol resonance characteristics, an AUX output crossfader, and direct Yamaha DX7 6-operator SysEx bank compatibility!
* **The 3 Architecture Banks (40 Total Models):**
  1. `Bank A: Plaits Factory (16 Models)`: Classic Virtual Analog, Waveshaping, 2-Op FM, Granular Formant, Harmonic Additive, 24-Wave Wavetable, 11-Chord Cluster, Vowel & Speech, Granular Cloud, Inharmonic String, Modal Resonator, Analog Bass Drum, Snare, Hi-Hat, Synthetic Bass Drum, Particle Noise.
  2. `Bank B: Plaits Orange 1.2 (8 Models)`: Classic 24dB/12dB Filter VA, Phase Distortion & Phase Modulation, 2-Voice 6-Op FM Synth (Model 1), 6-Op FM Synth (Model 2), 6-Op FM Synth (Model 3 with 32-patch DX7 SysEx loader), Wave Terrain 2D Synthesis, String Machine with Stereo Ensemble Chorus, 4-Voice Chiptune Square Chords.
  3. `Bank C: Braids Renaissance & Mods (16 Models)`: SAM C64 Robotic Speech Synthesizer, West Coast Wavefolder, Z-Filter Formant, QPSK Phase Modulation, Bees-in-the-Trees Paraphonic Triple-Saw/Tri/Sine, Bell/Bowed Physical, Casio CZ Resonant PD, Filtered Noise Clusters.
* **8 Performance Knobs:**
  - `KNOB 1: MODEL / BANK` (Model selector $0 \dots 15$; Press: Cycle Banks `[A: Plaits 16 | B: Orange 1.2 | C: Braids Classics]`).
  - `KNOB 2: TIMBRE` (Continuous primary spectral shape / cutoff / modulation index / wavefolder).
  - `KNOB 3: MORPH` (Secondary sonic axis: odd/even harmonic balance, formant vowel, FM ratio, wave terrain scan).
  - `KNOB 4: HARMONICS` (Tertiary axis: unison spread, chord selection, sub-osc level, DX7 algorithm).
  - `KNOB 5: OCTAVE / TUNE` ($-4 \dots +4$ octaves, Profile B Bipolar; Press: Toggle Semitone vs. $\pm 50\,\text{cents}$ Fine Tune).
  - `KNOB 6: DECAY / LPG` (Built-in VCFA / Low-Pass Gate decay: Snappy $10\,\text{ms} \dots 10.0\,\text{s}$ / Drone $60.0\,\text{s}$; Press: Cycle LPG Mode `[LowPass Gate | VCA Only | VCF Only | Bypass]`).
  - `KNOB 7: AUX BLEND` ($0\% \dots 100\%$ crossfade between Primary Out and Auxiliary Out: sub-octave, noise sideband, chord root, or second DX7 voice).
  - `KNOB 8: LEVEL / VELO` (Master output gain $0\,\text{dB} \dots +12\,\text{dB}$; Press: Velocity-to-LPG Dynamic Response On/Off).
* **Rear Deck (⚙️ Back of the Panel):**
  - `DIP 1: LPG MODEL`: `[ OPTO-VACTROL (Classic Bouncy Ring) | CLEAN DIGITAL (Instant Slew-Free) | DIRTY DIODE (Slight Asymmetric Sag) ]`
  - `DIP 2: CHORD TABLE`: `[ PLAITS CLASSIC (11 Chords) | JOE MCMULLEN JAZZ (Maj9, m11, 13, Sus) | NEOCLASSICAL (Open 5ths, Octaves) | CHIPTUNE ARPS ]`
  - `DIP 3: DX7 SYSEX BANK`: `[ ROM1A Classic | ROM1B Synth/Bass | DX7 E.PIANO/BELLS | USER CUSTOM SYSEX ]`
  - `DIP 4: M8 MACRO COMPATIBILITY`: `[ OFF (Full 96kHz Oversampled Floating-Point) | M8 BIT-ACCURATE (44.1kHz Cortex-M7 Fixed-Point Emulation) ]`
  - `TRIMPOT 1: ANALOG SLOP / DRIFT`: Virtual component tolerance drift on pitch and LPG response.
  - `TRIMPOT 2: SAM SPEECH THROAT BIAS`: Vocal tract length and glottal roughness for the SAM C64 robotic speech model.
  - `TRIMPOT 3: LPG BLEED / RESIDUAL`: Simulates vintage hardware vactrol dark leakage floor when the gate is fully closed.

---

### 🔊 OSC-14: SqueezeFold (M8-Style Multiplier & Wavefolder) — [S-TIER / LOCKED]
*(Corporate Boring: **`Wave-Multiply & Fold Synthesizer`**)*
* **Pedigree:** Dirtywave M8 Wavsynth engine (the beating heart of Cody's tracks *cyber god*, *bitterkitten*, and *back alley*).
* **Architecture:** High-velocity digital synthesis core combining single-cycle waveforms with arithmetic wave multiplication ($1\times \dots 16\times$), horizontal phase squeezing/warping, Serge/Buchla non-linear wavefolding, and digital bit-decimation. Delivers ripping industrial sync leads, hollow metallic basses, and alien digital textures with surgical harmonic control.
* **8 Performance Knobs:**
  - `KNOB 1: WAVE SHAPE` (Sine, Triangle, Saw, Pulse 12.5%, Pulse 25%, Pulse 50%, Sync Saw, Digital Parabolic, Noise; Press: Cycle Waveforms).
  - `KNOB 2: MULTIPLY` ($1\times \dots 16\times$ integer wave multiplication, packing harmonic replicas into each fundamental cycle).
  - `KNOB 3: WARP / SQUEEZE` ($-100\% \dots +100\%$ asymmetrical phase compression: squeezes the wave toward cycle boundaries).
  - `KNOB 4: FOLD / MIRROR` ($0\% \dots 100\%$ non-linear wavefolding / full-wave rectification mirror).
  - `KNOB 5: OCTAVE` ($-4 \dots +4$ octaves; Press: Fine Detune $\pm 50\,\text{cents}$).
  - `KNOB 6: PULSE WIDTH / SYMMETRY` ($1\% \dots 99\%$ duty cycle and waveform symmetry).
  - `KNOB 7: CRUSH / DECIMATE` ($16 \dots 2\,\text{Bits}$ bit-depth quantization and sample decimation).
  - `KNOB 8: SUB / DRY LEVEL` ($0\% \dots 100\%$ independent pure sub-harmonic sine blend ensuring rock-solid low-end fundamentals).
* **Rear Deck (⚙️ Back of the Panel):**
  - `DIP 1: FOLD TOPOLOGY`: `[ SERGE TRI-FOLD (Smooth Symmetrical) | BUCHLA SINE (Recursive Sine Fold) | RECTIFIER (Hard Invert) ]`
  - `DIP 2: MULTI-WINDOW`: `[ HARD RESTART (M8 Sharp Edges) | HANNING WINDOWED (Anti-Aliased Smooth Transitions) ]`
  - `DIP 3: PHASE RESET`: `[ FREE-RUNNING (Analog DCO feel) | HARD RESET (0° Punch on Trigger) | RANDOM (Stereo Scatter) ]`
  - `TRIMPOT 1: DC SERVO BIAS`: Auto-zero DC balance compensation preventing subsonic offset during extreme wavefolding.
  - `TRIMPOT 2: THERMAL WOBBLE`: LFO-free micro-thermal pitch wander for organic analog drift.
  - `TRIMPOT 3: SATURATION CEILING`: Post-shaper soft-clip threshold ($0\,\text{dBFS} \dots +6\,\text{dB}$ saturation).


---

## 4. The 8 Canonical Mega-Effects (Workstations)

### 🎛️ 1. `KlangWarp` (Studio Saturation & Clipper Lab)
*(Corporate Boring: **`Studio Saturator`**)*
* **Algorithms:** (1) Soft Padé Tanh, (2) Hard Diode Clamp, (3) The XXXL Harmonic Warp (Independent Odd/Even harmonics + 5Hz DC block), (4) West Coast Fold (Multi-stage recursive sine wavefolder), (5) 12-Bit Vintage Decimator (SP-1200 / S950 with post low/high tone sculpting), (6) Full-Wave Octave Rectifier.
* **8 Performance Knobs:** `DRIVE` ($-6\,\text{dB} \dots +36\,\text{dB}$), `FOLD / BITS` (Wavefold stages / Bit depth), `ASYM / BIAS` (DC symmetry even harmonics), `WARP CHARACTER` (Soft Tanh, Diode, XXXL, Fold, 12-Bit, Rectifier), `TONE / TILT` (Post-saturation tone filter), `HEADROOM / CLAMP` (Ceiling threshold), `LOW CLEAN` (High-pass sidechain protecting sub-bass from distortion), `MIX / ∅` (Profile M Tri-Zone Mix + Phase Invert).

---

### 🎛️ 2. `KlangPunch` (Dynamics, Transients & Bus Glue Lab)
*(Corporate Boring: **`Dynamics & Envelope Processor`**)*
* **Algorithms:** (1) SSL / dbx VCA Bus Glue, (2) SPL Dual-Ballistic Transient Shaper, (3) 1176 British FET (All-Buttons in), (4) LA-2A Optical Leveler, (5) Cableguys Rhythmic Sidechain Ducking Pumper, (6) **The Annihilator (Empirical Labs Distressor NUKE Mode with Dist 2 tube and Dist 3 tape harmonics)**.
* **8 Performance Knobs:** `THRESHOLD` ($-60\,\text{dB} \dots 0\,\text{dB}$), `ATTACK / PUNCH` ($0.1\,\text{ms} \dots 100\,\text{ms}$ or Transient Attack $\pm 100\%$), `RELEASE / SUSTAIN` ($10\,\text{ms} \dots 2000\,\text{ms}$ or Transient Sustain $\pm 100\%$), `RATIO / ENGINE` (VCA Glue, Transient Shaper, 1176 FET, LA-2A Opto, Sidechain Pump, The Annihilator NUKE), `KNEE / NUKE` (Soft knee $\leftrightarrow$ Brickwall reverse-knee NUKE), `SIDECHAIN HPF` ($20\,\text{Hz} \dots 400\,\text{Hz}$), `MAKEUP GAIN` ($0\,\text{dB} \dots +30\,\text{dB}$), `MIX / ∅` (Profile M Tri-Zone Mix + Phase Invert).

---

### 🎛️ 3. `KlangFilter` (Spectral Synthesis & Resonator Lab)
*(Corporate Boring: **`Multimode Filter & Resonator`**)*
* **Algorithms:** (1) ZDF State-Variable Filter (SVF), (2) Transistor Ladder (Moog 24dB), (3) Screamer 35 (Korg MS-20), (4) **Diode Acid Squelch (Roland TB-303 / Vital Diode Ladder with resonance cutoff slump)**, (5) Alpha Dual-12 (Roland Alpha Juno IR3R05 dual -12dB SVF cores with dual independent resonance), (6) **Vandal Park (Steiner-Parker diode SVF with Vandal Factor feedback loop)**, (7) Hornet (EDP Wasp CMOS CD4069UB leaky inverter SVF), (8) **Belgian Warbank (Sherman Filterbank 2 dual filter with audio-rate FM and tube overdrive)**, (9) Human Vowel Formant Bank, (10) One-Knob DJ Performance Filter, (11) Tuned Comb.
* **8 Performance Knobs:** `CUTOFF` ($20\,\text{Hz} \dots 20\,\text{kHz}$, Profile D Log), `RESONANCE` ($0\% \dots 115\%$ self-oscillation), `DRIVE` (Pre-filter analog saturation), `MODEL` (11 Analog Filter Engines), `ENVELOPE DEPTH` (Bipolar filter mod $-100\% \dots +100\%$), `TRACKING` ($0\% \dots 100\%$ MIDI keytracking), `SPREAD / FACTOR` (Detune / Vandal Factor / Dual Resonance Notch), `MIX / ∅` (Profile M Tri-Zone Mix + Phase Invert).

---

### 🎛️ 4. `KlangEQ` (Studio, Console & DJ Performance Equalizer)
*(Corporate Boring: **`Parametric & DJ Equalizer`**)*
* **Algorithms:** British Inductor (Neve 1073 curves), American Console (API 550 proportional Q), Modern Clean ZDF Biquad, Passive Tube.
* **8 Performance Knobs:**
  - `LOW GAIN` ($-15\,\text{dB} \dots +15\,\text{dB}$; Press: Cycle Shelf, Bell, **Passive Bass Contour [Pultec Trick]**).
  - `MID FREQ` ($100\,\text{Hz} \dots 8\,\text{kHz}$, Profile D Log; Press: Q/Width 0.5, 1.2, 4.0).
  - `MID GAIN` ($-15\,\text{dB} \dots +15\,\text{dB}$; Press: Band Mute).
  - `HIGH GAIN` ($-15\,\text{dB} \dots +15\,\text{dB}$; Press: Shelf $\leftrightarrow$ Bell).
  - **`DJ FILTER`** (Bipolar Profile B S-Curve: 12:00 Flat, CCW = LPF sweep down to 20Hz, CW = HPF sweep up to 20kHz; Press: Pioneer DJM, Allen & Heath Xone, Diode Reso).
  - `TILT / TONE` ($-6\,\text{dB} \dots +6\,\text{dB}$ pivoted at $1.0\,\text{kHz}$; Press: Shift pivot 650Hz, 1kHz, 2.5kHz).
  - `DRIVE / COLOR` ($0\,\text{dB} \dots +18\,\text{dB}$ Neve transformer / inductor saturation).
  - `OUTPUT / TRIM` ($-18\,\text{dB} \dots +18\,\text{dB}$; Press: Phase Invert $\varnothing$).

---

### 🎛️ 5. `KlangMod` (Chorus, Ensemble & Modulation Lab)
*(Corporate Boring: **`Modulation & Chorus Processor`**)*
* **Algorithms:** (1) Clean Digital Stereo, (2) Vintage BBD Chorus (Roland Juno-60/106 Modes I, II, I+II), (3) Swept Hoover Chorus (Alpha Juno continuous variable LFO rate), (4) 16-Voice Hyper-Ensemble (Kilohearts style dense detune), (5) Spatial Dimension BBD (Roland Dimension D true spatial expansion), (6) Through-Zero Flanger, (7) 8-Stage Optical Phaser, (8) Harmonic Tremolo.
* **8 Performance Knobs:** `RATE` ($0.05\,\text{Hz} \dots 20\,\text{Hz}$ / Host Sync), `DEPTH` ($0\% \dots 100\%$), `FEEDBACK` ($-100\% \dots +100\%$), `ENGINE` (8 Modulation Algorithms), `STEREO WIDTH` ($0\% \dots 200\%$), `TONE / COLOR` (BBD analog high-cut), `PRE-DELAY` ($0\,\text{ms} \dots 20\,\text{ms}$), `MIX / ∅` (Profile M Tri-Zone Mix + Phase Invert).

---

### 🎛️ 6. `KlangShift` (Pitch, Granular & Heterodyne Lab)
*(Corporate Boring: **`Pitch & Granular Processor`**)*
* **Algorithms:** (1) Dual Micro-Pitch Detune (Eventide H3000 style $\pm 50\,\text{cents}$ stereo detune), (2) Granular Harmonizer (Semitone pitch shifting $\pm 24\,\text{st}$ with **Scale Quantizer locked to Phrygian, Phrygian Dominant, Harmonic Minor, and Project Key Center**), (3) 4-Quadrant Ring Modulator (with Sine $\rightarrow$ Tri $\rightarrow$ Saw $\rightarrow$ Square carrier wave morphing), (4) Rotary Drum & Horn (Leslie 122 rotating Doppler speaker), (5) Sub-Boom Generator (dbx 120A subharmonic synthesis).
* **8 Performance Knobs:** `PITCH / SHIFT` ($-24 \dots +24$ Semitones or Carrier Hz), `FINE DETUNE` ($-50 \dots +50$ Cents), `GRAIN SIZE` ($10\,\text{ms} \dots 250\,\text{ms}$), `SCALE SNAP` (Off, Chromatic, Project Key, Phrygian, Phrygian Dominant, Harmonic Minor), `CARRIER WAVE` (Sine, Tri, Saw, Square), `STEREO SPREAD` (Left Up / Right Down micro-offset), `FEEDBACK / SHIMMER` ($0\% \dots 100\%$), `MIX / ∅` (Profile M Tri-Zone Mix + Phase Invert).

---

### 🎛️ 7. `KlangDelay` (Echo, Delay & Repetition Lab)
*(Corporate Boring: **`Stereo Delay & Echo`**)*
* **Algorithms:** (1) Digital Stereo Ping-Pong, (2) Green Tape Echo (Roland Space Echo RE-201 3-head tape flutter & saturation), (3) Deluxe Memory BBD (Panasonic MN3005 bucket-brigade), (4) Reverse Delay Buffer, (5) Granular Cloud Freeze (Infinite ambient freeze), (6) Oil Can Delay (Tel-Ray Adineko electrostatic fluid dark wobble).
* **8 Performance Knobs:** `TIME / DIV` (Tempo synced $1/64 \dots 2$ bars or $1\,\text{ms} \dots 2000\,\text{ms}$), `FEEDBACK` ($0\% \dots 115\%$ runaway), `TONE / DAMP` (High-cut / low-cut damping), `ALGORITHM` (6 Delay Engines), `MOD / FLUTTER` (Tape wow, flutter, and BBD chorusing), `STEREO OFFSET` (Ping-Pong cross-bounce width), `DRIVE` (Analog tape / BBD preamp drive), `MIX / ∅` (Profile M Tri-Zone Mix + Phase Invert).

---

### 🎛️ 8. `KlangSpace` (Reverb & Acoustic Chamber Lab with Universal Gate)
*(Corporate Boring: **`Reverb & Acoustic Chamber`**)*
* **Algorithms:** (1) Vintage Steel Plate (EMT-140 metallic plate), (2) Digital Void Hall (Lexicon 480L 60s hall), (3) Spring Twang (Physical mechanical spring with chirp), (4) Celestial Shimmer (Octave-up recirculating feedback), (5) Shoegaze Reverse (Non-linear crescendo bloom), (6) Tight Drum Room.
* **The Universal Tri-Mode Gate:** Every model passes through the integrated drum gate with 3 switchable trigger engines:
  1. `TIME GATE (ms)` ($10\,\text{ms} \dots 2000\,\text{ms}$): Fixed 80s Phil Collins / Hugh Padgham drum gate.
  2. `BEATS GATE (Tempo Synced)` ($1/64 \dots 2\,\text{bars}$): Snaps tail shut in exact rhythmic lockstep with host BPM / tracker groove.
  3. `DYNAMIC / LEVEL GATE (dB Threshold)` ($-60\,\text{dB} \dots 0\,\text{dB}$): Envelope follower noise gate.
* **8 Performance Knobs:**
  - `PRE-DELAY` ($0\,\text{ms} \dots 250\,\text{ms}$ / Host Sync; Press: Sync toggle).
  - `SIZE / DECAY` ($100\,\text{ms} \dots 10.0\,\text{s}$ Snappy / $60.0\,\text{s}$ Drone; Press: **`INFINITE FREEZE`** latch).
  - `DAMPING / TONE` ($500\,\text{Hz} \dots 20\,\text{kHz}$, Profile D Log; Press: Filter Target).
  - `DIFFUSION` ($0\% \dots 100\%$ reflection density).
  - **`GATE TIME / WINDOW`** (Universal Gate: ms, Sync Beats, or dB Threshold; Press: Cycle 3 Gate Modes).
  - **`GATE SHAPE / RELEASE`** ($0\,\text{ms}$ Brickwall Clamp $\dots 500\,\text{ms}$ exponential; Press: Toggle Reverse Bloom Gate).
  - `STEREO WIDTH` ($0\% \dots 200\%$ with $120\,\text{Hz}$ Mono-Bass Anchor).
  - `MIX / ∅` (Profile M Tri-Zone Mix + Phase Invert).

---

## 5. The 11 Bespoke Standalone Processors

### 👑 1. `KlangStomp` (8-Pedal Analog Dirt Hall of Fame)
*(Corporate Boring: **`Analog Stompbox Suite`**)*
* **The 8 Circuit Models:**
  1. `Swedish Chainsaw` (Boss HM-2): Unbiased BJT dead-band zero-crossing distortion + dual gyrators (~100Hz low & ~1kHz high resonance).
  2. `Industrial Zone` (Boss MT-2): Dual op-amp hard clip + sweepable semi-parametric mid EQ ($200\,\text{Hz} \dots 5\,\text{kHz} \pm 15\,\text{dB}$).
  3. `Rodent` (ProCo RAT): LM308 op-amp slew-rate limiting + reverse passive low-pass filter.
  4. `Screamer 808` (Ibanez TS9/TS808): Non-inverting clipping amp with antiparallel silicon diodes + 720 Hz high-pass roll-off.
  5. `Ixion Guild` (Klon Centaur): Dual-gang clean blend pot + germanium soft clip + 18V charge pump headroom.
  6. `Russian Muff` (Electro-Harmonix Big Muff): 4 discrete BJT stages with 1024 Hz passive mid-notch tone stack.
  7. `Yellow Preamp 250` (DOD 250 / MXR Dist+): LM741 op-amp gain with back-to-back clipping diodes.
  8. `Velcro Face` (Fuzz Face): 2-transistor Germanium/Silicon direct-coupled BJTs with voltage starvation (gated velcro decay).
* **8 Performance Knobs:** `GAIN / DRIVE`, `TONE / FILTER`, `LEVEL / VOL`, `CIRCUIT MODEL` (8 Pedals), `MID / GYRATOR` (Parametric mid / Chainsaw peak), `STARVE / VOLTS` ($3\,\text{V} \dots 18\,\text{V}$ rail starve/pump), `CLEAN BLEND` ($0\% \dots 100\%$), `MIX / ∅` (Profile M Tri-Zone Mix + Phase Invert).

---

### 👑 2. `Cerberus` (Harmonic Distortion Synthesizer)
*(Corporate Boring: **`Harmonic Distortion Synthesizer`**)*
* **Pedigree:** Clean-room recreation of Fumio Mieda’s 3-circuit parallel distortion synthesizer architecture.
* **Architecture:** 3 parallel intermodulating distortion circuits (the 3 heads of Cerberus):
  - Head 1: Warmth (soft saturation & fundamental reinforcement).
  - Head 2: Vowel Wavefold (asymmetric formant waveshaping).
  - Head 3: Crossover Bite (dead-band zero-crossing high-frequency fuzz).
* **8 Performance Knobs:** `WARMTH DRIVE`, `VOWEL FOLD`, `BITE GAIN`, `BALANCE 1-2`, `BALANCE 3`, `TRIMMER / BIAS`, `GATE THRESH`, `MIX / ∅` (Profile M Tri-Zone Mix + Phase Invert).

---

### 👑 3. `Argent Anvil` (Extreme Multiband Dynamics Suite)
*(Corporate Boring: **`Multiband Upward/Downward Dynamics`**)*
* **The Dual Engines:**
  - **Mode 1: Classic OTT:** 3-band Linkwitz-Riley 4th order crossover ($120\,\text{Hz} \dots 2.5\,\text{kHz}$) driving simultaneous upward and downward compression.
  - **Mode 2: Argent +100dB DOOM:** Pushes upward compression to $+100\,\text{dB}$ with an integrated zero-latency hard clipper and glitch knee.
* **8 Performance Knobs:** `DEPTH` ($0\% \dots 100\%$), `TIME` ($0.1\times \dots 10\times$ attack/release scale), `UPWARD GAIN` ($0\,\text{dB} \dots +36\,\text{dB}$ / $+100\,\text{dB}$ in Argent Mode), `DOWNWARD COMP` (Threshold & ratio), `LOW BAND` ($-24\,\text{dB} \dots +24\,\text{dB}$), `MID BAND` ($-24\,\text{dB} \dots +24\,\text{dB}$), `HIGH BAND` ($-24\,\text{dB} \dots +24\,\text{dB}$), `MIX / ∅` (Profile M Tri-Zone Mix + Phase Invert).

---

### 👑 4. `Smear` (All-Pass Phase Disperser)
*(Corporate Boring: **`All-Pass Phase Disperser`**)*
* **Architecture:** Cascaded all-pass phase dispersion network ($1^{\text{st}}, 2^{\text{nd}}, 4^{\text{th}}$ Order stages) designed to smear transient energy across time without altering frequency response. Delivers laser drum thumps, squelchy Paiste bronze clatter, and neurofunk bass punch.
* **8 Performance Knobs:** `FREQUENCY` ($20\,\text{Hz} \dots 20\,\text{kHz}$, Profile D Log), `STAGES / AMOUNT` (1 to 32 all-pass filter poles), `RESONANCE / Q` ($0.1 \dots 18.0$), `ORDER TYPE` (1st, 2nd, 4th Order all-pass blocks), `STEREO SPREAD` (Phase offset between Left and Right channels), `PRE-DRIVE` ($0\,\text{dB} \dots +18\,\text{dB}$ Padé tanh saturation), `LOW DECOUPLE` (High-pass sidechain keeping sub-bass phase intact), `MIX / ∅` (Profile M Tri-Zone Mix + Phase Invert).

---

### 👑 5. `FreakShifter` (Harald Bode Frequency Shifter & SSB Modulator)
*(Corporate Boring: **`Single-Sideband Frequency Shifter`**)*
* **Architecture:** Harald Bode single-sideband (SSB) frequency shifter using dual 90-degree Hilbert transform networks and quadrature sine/cosine carrier oscillators. Features dual-antiphase stereo shifting (Left shifts UP while Right shifts DOWN) and an integrated feedback delay loop.
* **8 Performance Knobs:** `SHIFT FREQ` (Bipolar $-5\,\text{kHz} \dots 0\,\text{Hz} \dots +5\,\text{kHz}$, Profile B S-Curve), `FINE TUNE` ($-20\,\text{Hz} \dots +20\,\text{Hz}$), `SIDEBAND BLEND` (Lower Sideband $\leftrightarrow$ Ring Mod $\leftrightarrow$ Upper Sideband), `FEEDBACK` ($0\% \dots 115\%$ spiraling feedback loop), `DELAY TIME` ($0\,\text{ms} \dots 1000\,\text{ms}$ or Host Sync), `STEREO DUAL` ($0\% \dots 100\%$ antiphase Left Up / Right Down width), `TONE / DAMP` (High-frequency damping in feedback loop), `MIX / ∅` (Profile M Tri-Zone Mix + Phase Invert).

---

### 👑 6. `Pipe & Wire` (Tuned Waveguide Resonator & Acoustic Cavity Delay)
*(Corporate Boring: **`Acoustic Waveguide Resonator`**)*
* **Pedigree:** Kevin Karplus & Alex Strong waveguide modeling crossed with the Korg Volca Drum Wave Guide physical modeling resonator.
* **Architecture:** Tuned sub-millisecond circular delay line with fractional Lagrange pitch interpolation, all-pass inharmonic phase dispersion, high-loss damping, low-cut anti-mud decoupling, and Padé tanh loop saturation.
* **The 4 Waveguide Engines (Rear Deck DIP 1):**
  1. `Pipe (Volca Tube)`: Inverted reflection (Negative Feedback $-$). Sings purely in odd harmonics ($1, 3, 5, 7$), generating hollow bamboo slits, conga cavities, and woody percussive resonance!
  2. `Wire (Volca String)`: Normal reflection (Positive Feedback $+$). Sings in all integer harmonics ($1, 2, 3, 4$) for metallic wire and ringing steel rims.
  3. `Clatter / Bell`: Positive feedback + High Inharmonic Stiffness ($f_n \neq n f_0$) for vibrating metal bars and struck bronze gongs.
  4. `Comb Flange`: Feedforward FIR notch flanging.
* **8 Performance Knobs:**
  - `PITCH / TUNE` ($20\,\text{Hz} \dots 20\,\text{kHz}$, C0 to B9; Press: Keytrack Lock).
  - `FEEDBACK / DECAY` ($-115\% \dots +115\%$ Profile F Hyperbolic; Press: Toggle % vs RT60 ms).
  - `DAMPING / HF LOSS` ($1\,\text{kHz} \dots 20\,\text{kHz}$, Profile D Log).
  - `LOW CUT / HPF` ($20\,\text{Hz} \dots 1.5\,\text{kHz}$ loop anti-mud filter).
  - `STIFFNESS / INHARMONIC` (All-pass dispersion stretching harmonics into stiff metal bars/chimes).
  - `DRIVE / LOOP SAT` ($0\,\text{dB} \dots +24\,\text{dB}$ Padé tanh loop saturation).
  - `STEREO SPREAD` ($0 \dots 50\,\text{cents}$ / micro-delay $\Delta \tau$ 3D binaural width).
  - `MIX / ∅` (Profile M Tri-Zone Mix + Phase Invert).

---

### 👑 7. `PipKlick` (Wavetable Waveshaper & Glitch Lab)
*(Corporate Boring: **`Wavetable Waveshaper`**)*
* **Pedigree:** WMD Geiger Counter & Kilohearts ShaperTable.
* **Architecture:** Wavetable waveshaping engine where instantaneous audio amplitude directly indexes into a 256-wavetable lookup table containing mathematical transfer functions (Chebyshev polynomials, bitwise XOR/AND folds, digital staircases, and diode curves).
* **8 Performance Knobs:** `PRE-GAIN / DRIVE` ($-12\,\text{dB} \dots +36\,\text{dB}$), `TABLE / WAVE` (256 Curves across 4 Banks), `MORPH / SCAN` ($0\% \dots 100\%$ smooth wavetable interpolation), `BIT DEPTH` ($1 \dots 16\,\text{Bits}$), `SAMPLE RATE` ($200\,\text{Hz} \dots 48\,\text{kHz}$ decimator), `MASK / TONE` (Post-shaper high/low filter), `SLEW / SMOOTH` (Non-linear slew rate limiter rounding harsh digital spikes), `MIX / ∅` (Profile M Tri-Zone Mix + Phase Invert).

---

### 👑 8. `Argent Vortex` (Phase-Fold Chaos Resonator)
*(Corporate Boring: **`Phase-Fold Resonator`**)*
* **Architecture:** The ultimate mad-scientist processor crossing a West Coast multi-stage Buchla wavefolder with a multi-order All-Pass Filter network in a recirculating, self-modulating feedback loop.
* **The 3 Routing Modes (Knob 3 Press / Rear Deck):**
  1. `Mode A: [ FOLD ──► APF ]`: Folds the waveform first to generate dense harmonics, then phase-disperses the overtones across time into singing laser drops.
  2. `Mode B: [ APF ──► FOLD ]`: Disperses the phase first! Different frequencies reach the wavefolder thresholds at different delay times, fracturing the waveform into wildly asymmetrical, liquid neurofunk snarls.
  3. **`Mode C: [ DUAL RECIRCULATING CHAOS LOOP ]`**: Both processors live inside the feedback loop, creating a closed-loop fractal generator that self-oscillates into infinite chaotic harmonic fractals!
* **8 Performance Knobs:** `FOLDS` ($1 \dots 8$ Stages), `SMEAR FREQ` ($20\,\text{Hz} \dots 20\,\text{kHz}$ APF center pitch), `APF STAGES & ROUTING` ($1^{\text{st}}, 2^{\text{nd}}, 4^{\text{th}}, 8^{\text{th}}$ Order; Press: Cycle Modes A, B, C), `FEEDBACK` ($-115\% \dots +115\%$ Profile F Hyperbolic), `SYMMETRY / BIAS` (DC offset even harmonics), `CROSS-FM` ($0\% \dots 100\%$ wavefolder output modulates APF cutoff), `DAMPING` ($1\,\text{kHz} \dots 20\,\text{kHz}$ loop filter), `MIX / ∅` (Profile M Tri-Zone Mix + Phase Invert).

---

### 👑 9. `EchoMill` / `TKMD` (Recursive Insert-Rack Delay & Degradation Mill)
*(Corporate Boring: **`Recursive Insert Delay`**)*
* **Detailed Spec:** [`the_klang_mill_delay.md`](spinoffs/the_klang_mill_delay.md)
* **Architecture:** Sovereign standalone delay plugin and internal TKS spatial engine featuring an embedded 2-slot recursive mini-pedalboard directly inside the delay recirculation loop. Every echo repeatedly passes through active insert processors (Bode frequency shifters, HM-2 chainsaw distortion, pitch shifters, Buchla wavefolders, and analog slop drift), creating dynamic compounding coloration and organic disintegration over time.
* **8 Performance Knobs:**
  - `KNOB 1: TIME / RATE`: $1\,\text{ms} \dots 2500\,\text{ms}$ (Smooth Hermite/Lagrange fractional read; Press: BPM Synced Divisions `1/64` to `2/1`).
  - `KNOB 2: STEREO / PING-PONG`: $-100\% \dots +100\%$ (Negative = Crossfeed ping-pong bounce; Positive = Haas micro-delay stereo offset).
  - `KNOB 3: SLOT 1 ENGINE`: Selects insert processor (`0: Bypass`, `1: Bode Frequency Shifter`, `2: Pitch Shifter`, `3: KlangStomp Chainsaw`, `4: Buchla Phase-Fold`, `5: Modal Resonator`; Press: Slot 1 Mute/Bypass).
  - `KNOB 4: SLOT 1 INTENSITY`: Context-sensitive control (Shift Hz, Semitones, Drive gain, Fold depth, Resonance).
  - `KNOB 5: SLOT 2 ENGINE`: Selects insert processor (`0: Bypass`, `1: Analog Slop / Tape Wow`, `2: Tape Damping / HF Loss`, `3: Bitcrush & Slew`, `4: Juno BBD Chorus`; Press: Slot 2 Mute/Bypass).
  - `KNOB 6: SLOT 2 DEPTH`: Context-sensitive control (Mod depth, Damping frequency, Bit-depth decimation, Chorus rate).
  - `KNOB 7: FEEDBACK & FREEZE`: $0\% \dots 120\%$ hyperbolic curve (Profile F); Press: **`INFINITE FREEZE HOLD`** (locks write head; echoes recirculate endlessly through insert modules).
  - `KNOB 8: MIX / ∅`: Profile M Tri-Zone Mix ($0.0 = 100\%\,\text{Wet}$, $0.5 = 100\%\,\text{Dry}$, $1.0 = 50/50\,\text{Mix}$; Press: Phase Invert $\varnothing$).
* **Rear Deck (⚙️ Back of the Panel):**
  - `DIP 1: DELAY INTERPOLATION`: `[ HERMITE (Pitch-Glide Tape Warp) | LAGRANGE (Granular Lo-Fi Read) | MOTOR SLEW (Inertial Capstan Motor Acceleration) ]`
  - `DIP 2: INSERT ROUTING ORDER`: `[ SLOT 1 ──► SLOT 2 (Dirt into Mod) | SLOT 2 ──► SLOT 1 (Mod into Dirt) | PARALLEL SPLIT (Independent Echo Sub-Paths) ]`
  - `DIP 3: LOOP LIMITER CHARACTER`: `[ PADE TANH (Symmetric Tube Compression) | DIODE CLIP (Crisp Shunt Clamp) | OPTO LEVELER (Pumping Dub Sag) ]`
  - `DIP 4: STEREO COUPLING`: `[ TRUE STEREO (Dual Independent Delay Paths) | CROSS-COUPLED PING-PONG | MID/SIDE MATRIX ]`
  - `TRIMPOT 1: TAPE HYSTERESIS & NOISE`: Analog tape hiss and magnetic saturation floor injected into recirculation loop.
  - `TRIMPOT 2: DC OFFSET REJECTION`: Cutoff frequency for feedback DC blocking filter ($2\,\text{Hz} \dots 40\,\text{Hz}$).
  - `TRIMPOT 3: FREEZE SEAM CROSSFADE`: Crossfade window duration ($2\,\text{ms} \dots 50\,\text{ms}$) on freeze buffer wrap boundary.

---

### 👑 10. `56K Vocoder` (Electrix WarpFactory Tribute & 32-Band Spectral Warper)
*(Corporate Boring: **`Spectral Vocoder Processor`**)*
* **Pedigree:** Clean-room recreation and hyper-expansion of the 1999 **Electrix WarpFactory** hardware vocoder, crossed with dual **Roland MKS-50 (Alpha Juno)** carrier synthesis and modern zero-latency spectral filterbanks.
* **Architecture:** High-velocity performance vocoder running up to 32 stereo bandpass filter pairs with non-linear envelope followers. Captures the wild, aggressive, hands-on character of the WarpFactory:
  1. **Gender / Formant Warping**: Smooth, artifact-free continuous formant warping ($\pm 24\,\text{semitones}$) that shifts the vocal tract / formant profile independently of pitch (from rumbling demonic growls to razor-sharp chipmunk squeaks).
  2. **Q / Resonance**: Variable filter bandwidth and peak resonance ($Q = 0.5 \dots 24.0$). At high settings, the filter bands break into singing, screaming, liquid metallic self-oscillation.
  3. **Order / Resolution**: Variable filter slope ($12\,\text{dB/oct} \dots 48\,\text{dB/oct}$) and band resolution (8, 16, 24, 32 bands). Lower order gives raw, punchy, vintage 70s/80s intelligible grit; higher order gives silky, ultra-HD modern cybernetic sheen.
  4. **The WarpFactory "Auto Mode" (Self-Vocoder)**: Routes the input modulator to both the Formant and Carrier paths simultaneously! Running drum loops, breaks, or guitar riffs through Auto Mode causes the sound to rhythmically vocode and filter-modulate itself, generating wild, pumping, organic resonant textures.
  5. **Internal MKS-50 Carrier Engine**: Clean-room model of the Roland MKS-50 / Alpha Juno Fujitsu MB87123 DCO (saws, stepped pulses, sub-oscillators, and authentic analog BBD stereo chorus I/II), allowing you to recreate Cody's signature dual-rack MKS-50 carrier setup internally with zero external routing!
  6. **Ess / Sibilance Preservation**: Real-time spectral centroid detection routes unvoiced speech fricatives ('s', 't', 'k') directly around the vocoder core or substitutes high-passed noise for 100% articulate lyric clarity.
  7. **Source Kill**: Front-panel button/press that instantaneously mutes any dry carrier bleed from the output.
* **8 Performance Knobs:**
  - `KNOB 1: GENDER / WARP`: Continuous Formant Shift ($-24 \dots 0 \dots +24\,\text{semitones}$, Profile B Bipolar); Press: Formant Reset ($0\,\text{st}$).
  - `KNOB 2: Q / RESONANCE`: $0.5 \dots 24.0$ filter bandwidth and resonance (gentle vowel body $\leftrightarrow$ screaming liquid self-oscillation).
  - `KNOB 3: ORDER / FILTER MODE`: Turn: Filter order & band count ($8 \dots 32$ bands, $12 \dots 48\,\text{dB/oct}$); Press: Filter Tilt Mode `[ Bandpass (Standard) | Lowpass Tilt | Highpass Tilt ]`.
  - `KNOB 4: ESS / SIBILANCE`: $0\% \dots 100\%$ unvoiced consonant detector sensitivity and noise replacement depth.
  - `KNOB 5: CARRIER ENGINE`: Discrete selector (`0: MKS-50 Alpha Juno DCO`, `1: Robot Pitch Saw`, `2: Supersaw Unison`, `3: 4-Op FM Bell`, `4: Auto Mode [Self-Vocode]`, `5: External Sidechain Input`); Press: Mute Carrier.
  - `KNOB 6: ROBOT PITCH / TUNE`: Internal carrier pitch (C0 to B6); Press: Toggle MIDI Keytrack Lock On/Off.
  - `KNOB 7: DECAY / ENVELOPE`: $2\,\text{ms} \dots 2000\,\text{ms}$ envelope follower tracking attack/release (fast speech articulation $\leftrightarrow$ infinite choral freeze sustain).
  - `KNOB 8: MIX / SOURCE KILL`: Profile M Tri-Zone Mix (CCW = Pure Vocoded Wet, Center = Dry Voice, CW = 50/50 Parallel Blend); Press: **`SOURCE KILL`** (instantly mutes dry carrier signal).
* **Rear Deck (⚙️ Back of the Panel):**
  - `DIP 1: WARP TOPOLOGY`: `[ WARPFACTORY CLASSIC (Aggressive DSP Model) | ZERO-DELAY SVF (Pristine Analog Curve) | BESSEL LINEAR-PHASE ]`
  - `DIP 2: AUTO-MODE CROSSFEED`: `[ 1:1 RECURSIVE (Classic WarpFactory Drum Pumping) | PHASE-INVERTED DUCK | HARMONIC FREQUENCY DOUBLER ]`
  - `DIP 3: MKS-50 CARRIER CHORUS`: `[ OFF | JUNO CHORUS I | JUNO CHORUS II | JUNO CHORUS I+II (Hoover Ensemble) ]`
  - `DIP 4: SPECTRAL STEREO SPREAD`: `[ MONO (Centered 90s Rack Unit) | STEREO PAN-COMB (Odd Bands Left, Even Bands Right) | HAAS ROTATION ]`
  - `TRIMPOT 1: ESS SENSITIVITY THRESHOLD`: Detection threshold ($-60\,\text{dB} \dots -12\,\text{dB}$) for sibilant unvoiced consonant triggers.
  - `TRIMPOT 2: MKS-50 SUB-OSC LEVEL`: Gain of the -1 octave sub-oscillator inside the internal MKS-50 carrier engine.
  - `TRIMPOT 3: SPECTRAL NOISE GATE`: Downward expansion threshold preventing microphone background noise from exciting carrier bands.

---

### 👑 11. `Nimbus` (Granular Texture & Infinite Freeze Lab)
*(Corporate Boring: **`Granular Texture & Freeze Processor`**)*
* **Pedigree:** Émilie Gillet (Mutable Instruments Clouds & Beads), granular synthesis, micro-sound, and continuous time-stretching.
* **Architecture:** Real-time granular audio processor recording incoming audio into a circular RAM buffer (up to 32 seconds) and spawning up to 32 simultaneous, overlapping grains with independent pitch transposition, spatial dispersion, window shaping, and feedback. Features 4 operational modes (Granular Texture, Pitch-Shift / Time-Stretch, Looping Delay, and Spectral Madrigal / FFT Phase Vocoder) paired with an integrated high-density all-pass diffusion reverberator.
* **8 Performance Knobs:**
  - `KNOB 1: POSITION / SCRUB`: Buffer read head scrub position $0\% \dots 100\%$; Press: Instant Buffer Direction Reverse toggle.
  - `KNOB 2: GRAIN SIZE`: $10\,\text{ms}$ micro-sound clicks $\dots 500\,\text{ms}$ vast acoustic washes; Press: Cycle Grain Window `[Hann Bell | Smooth Tukey | Percussive Triangle | Stepped Rectangular]`.
  - `KNOB 3: PITCH / DETUNE`: $-24 \dots +24\,\text{semitones}$ grain pitch transposition (Profile B Bipolar; Press: Scale Quantizer `[Off | Chromatic | 5ths & Octaves | Phrygian | Harmonic Minor]`).
  - `KNOB 4: DENSITY / RATE`: Grain generation rate (CCW = Periodic rhythmic taps, Center = Sparse random Poisson distribution, CW = Ultra-dense 32-grain blizzard).
  - `KNOB 5: TEXTURE / SHAPE`: Grain envelope morphing and internal bandpass tilt coloring.
  - `KNOB 6: SPREAD / PAN SCATTER`: $0\% \dots 100\%$ random stereo panning scatter across grain generation.
  - `KNOB 7: DIFFUSION / SPACE`: High-density all-pass diffusion network ($0\% \dots 100\%$ lush reverb wash); Press: **`FREEZE`** (instantly halts buffer recording and loops grains within the frozen soundscape indefinitely!).
  - `KNOB 8: MIX / ∅`: Profile M Tri-Zone Mix (CCW = Pure Granular Cloud, Center = 100% Dry, CW = 50/50 Parallel Blend; Press: Phase Invert $\varnothing$).
* **Rear Deck (⚙️ Back of the Panel):**
  - `DIP 1: BUFFER AUDIO FIDELITY`: `[ 32-BIT FLOAT 96kHz (Crystal Clean Studio Pristine) | 16-BIT 48kHz (Clouds Eurorack Original) | 8-BIT MU-LAW 16kHz (Gritty Vintage Sampler Decimation) ]`
  - `DIP 2: GRANULAR PLAYBACK ENGINE`: `[ GRANULAR TEXTURE (Standard Clouds) | TIME-STRETCH / PITCH (Phase-Synchronized) | SPECTRAL MADRIGAL (FFT Resynthesis) | OLIVERB REVERB (Endless Shimmer) ]`
  - `DIP 3: DRY SIGNAL ARCHITECTURE`: `[ ANALOG DRY BYPASS | PROCESSED DIGITAL DRY (Phase-Aligned Sample Delay) ]`
  - `DIP 4: GRAIN WINDOW ENVELOPE`: `[ HANN AMBIENT | SHARP TUKEY TRANSIENT | EXPONENTIAL REVERSE BLOOM ]`
  - `TRIMPOT 1: BROWNIAN JITTER DEPTH`: Micro-randomness applied to grain spawn intervals and playback pitch.
  - `TRIMPOT 2: RECIRCULATION FEEDBACK`: Internal grain-to-buffer feedback gain for self-generating evolving ambient drones.
  - `TRIMPOT 3: INPUT DC BLOCKING POLE`: Calibrates high-pass DC blocker cutoff on live recording buffer inputs ($5\,\text{Hz} \dots 35\,\text{Hz}$).

---

## 6. Implementation Roadmap & Staging Gates

### 🎯 Milestone v0.5.0: The Sound & Chaos Update
1. **Core 9 S-Tier Sound Engines:**
   - `OSC-01: The R-1` (ER-1 FM/AM Cyclic Noise; Standalone: `The Klang R-1`)
   - `OSC-02: AlphaKraft` (Alpha Juno Dual-DCO + JP-8000 Stack)
   - `OSC-03: CosmoPhase` (Casio CZ Twin-Line Phase Distortion)
   - `OSC-05: Pluck` (Merged Waveguide + MI Rings Modal Resonator; FX: `Pluck FX`)
   - `OSC-09: Argent Slayer` (Phase-Locked Sub & Mick Gordon Wavefold)
   - `OSC-11: BronzeLab` (4-Core Metallic Clatter + 8-Stage APF Dispersion)
   - `OSC-12: K-Chip` (5-Console Vintage Chiptune Lab)
   - `OSC-13: Kaleidocore` (MI Plaits + Braids Super-Engine + 6-Op DX7 FM + SAM C64 Speech + Wave Terrain)
   - `OSC-14: SqueezeFold` (M8-Style Wave-Multiply & Fold Synthesizer)
2. **Core 8 Mega-Effects:**
   - `KlangWarp`, `KlangPunch`, `KlangFilter`, `KlangEQ`, `KlangMod`, `KlangShift`, `KlangDelay`, `KlangSpace`.
3. **Core 11 Bespoke Processors:**
   - `KlangStomp`, `Cerberus`, `Argent Anvil`, `Smear`, `FreakShifter`, `Pipe & Wire`, `PipKlick`, `Argent Vortex`, `EchoMill`, `56K Vocoder`, `Nimbus`.

---

### 🚀 Milestone v0.6.0: The Modular Fleet & The Klang Mill ("The Spinoff Spree")
Following the battle-testing and parameter stabilization of all 19 audio processors inside *The Klang Farmer* and *ToadTracker* during v0.5.0:

1. **Stamp Out the 19 Standalone Plugins (The Kilohearts-Style Modular Fleet):**
   - Extract all **8 Mega-Effects** and all **11 Bespoke Processors** into independent, featherweight, ultra-low-CPU VST3/CLAP plugins powered by the shared `TbdAudio::DSPBlock` core.
   - Modeled on the legendary **Kilohearts Snapin / Goodhertz / Melda** ecosystem: producers can insert single standalone processors (e.g. just `Argent Anvil`, `Argent Vortex`, `EchoMill`, or `Nimbus`) across any DAW mixer track with zero bloat.
   - **The Standalone Invariant:** All standalone spinoff plugins **ALWAYS STAY IN KLANG MODE PERMANENTLY**.
2. **Launch `The Klang Mill` (TKM):**
   - The sovereign multi-effect rack, host, and modulation matrix capable of chaining, routing, and modulating any combination of Klang modules in serial, parallel, or multiband splits with unified macro encoders.
3. **Shared DSP Binary Architecture (`libtks_dsp`):**
   - A single statically linked C++ DSP library compiled across Farmer, ToadTracker, Mill, and the 19 standalone plugin targets via unified CMake build targets.


