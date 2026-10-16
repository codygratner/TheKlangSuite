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
| **Korg Nu:Tekt HD-S** | **`KlangMieda`** | Fumio Mieda 3-circuit harmonic distortion synthesizer |
| **EDP Wasp Filter** | **`Hornet`** | CMOS CD4069UB leaky inverter SVF with "sandy" resonance |
| **Arturia / Steiner MiniBrute** | **`Vandal Park`** | Steiner-Parker diode SVF with "Vandal Factor" feedback |
| **Moog Ladder Filter** | **`Transistor Ladder`** | 4-pole cascaded BJT ladder with thermal voltage saturation |
| **Roland TB-303 Filter** | **`Diode Acid Squelch`** | Diode ladder filter with resonance-induced cutoff slump |
| **Sherman Filterbank 2** | **`Belgian Warbank`** | Dual analog filter, audio-rate FM, tube overdrive, self-oscillation |
| **Roland Alpha Juno IR3R05** | **`Alpha Dual-12`** | Dual cascaded -12dB SVF cores with dual independent resonance |
| **Korg MS-20 Filter** | **`Screamer 35`** | Korg35 OTA non-linear screaming filter |
| **Pultec EQP-1A Trick** | **`Passive Bass Contour`** | Simultaneous low boost + mid-bass cut resonant shelf |
| **Buchla 259/292 Wavefolder** | **`West Coast Fold`** | Non-linear recursive sine wavefolder |
| **WMD Geiger Counter** | **`KlangFallout`** | 256-wavetable amplitude waveshaping & bit-mask glitch |
| **Ableton + Argent DOOM** | **`KlangSilver`** | Multiband dynamics with +100dB upward DOOM compression |
| **Empirical Labs Distressor** | **`The Annihilator (NUKE)`** | Brickwall limiter with reverse knee + Dist 2/3 harmonics |
| **Boss HM-2** | **`Swedish Chainsaw`** | Unbiased complementary BJT dead-band + dual gyrators |
| **Boss MT-2 Metal Zone** | **`Industrial Zone`** | Dual op-amp hard clip + sweepable semi-parametric mid EQ |
| **ProCo RAT** | **`Rodent`** | LM308 slew-rate limiting distortion + reverse tone filter |
| **Ibanez TS9 / TS808** | **`Screamer 808`** | Symmetrical feedback diodes + 720 Hz high-pass rolloff |
| **Klon Centaur** | **`Sovereign Centaur`** | Dual-gang clean blend + germanium clip + 18V charge pump |
| **Electro-Harmonix Big Muff** | **`Russian Muff`** | 4-stage discrete BJT booster with 1 kHz mid-notch tone stack |
| **DOD 250 / MXR Dist+** | **`Yellow Preamp 250`** | LM741 op-amp overdrive with back-to-back clipping diodes |
| **Fuzz Face** | **`Velcro Face`** | 2-transistor Germanium/Silicon direct-coupled voltage starve |
| **Lexicon 224 / 480L** | **`Digital Void Hall`** | 12-bit Dattorro continuous algorithmic hall |
| **EMT-140 Plate** | **`Vintage Steel Plate`** | High-density physical steel plate reverberator |
| **Roland TR-808 + Mick Gordon** | **`KlangSlayer`** | Phase-locked sub-bass with recursive wavefolder & noise ring |
| **Game Boy DMG-01** | **`Pocket Matrix`** | 4-bit 32-sample wave channel, dual pulse & LFSR noise |
| **NES 2A03** | **`Family 8`** | 4-bit stepped triangle bass & non-linear DAC curves |
| **Konami VRC6** | **`Demon Castle Saw`** | 6-bit stepped saw generator & 8-step variable pulse |
| **Commodore 64 SID** | **`Breadbin SID`** | MOS SID tri/saw/pulse with audio-rate Hard Sync & Ring Mod |
| **PC Engine / TG-16** | **`Turbo Engine 16`** | 6-channel 5-bit 32-sample wavetable with channel cross-FM |

---

## 3. The 7 S-Tier Sound Engines (Oscillator Catalog)

### 🏢 Sound Engine Corporate Boring Naming Matrix
When the Master Settings Toggle is set to **`🏢 CORPORATE BORING`**, all sound engines strip their creative lore and trademark references into utilitarian engineering names:

| Engine Code | 👑 Klang Maximum (Default) | 🏢 Corporate Boring (Serious Mode) | Pedagogical & Heritage Circuit Architecture |
| :--- | :--- | :--- | :--- |
| **`OSC-01`** | **`KlangTribe R`** | **`Percussion Synthesizer`** | Carrier (Sine/Tri) + Cyclic Ring-Modulated Noise |
| **`OSC-02`** | **`KlangAlpha`** | **`Dual DCO & Unison Synthesizer`** | Dual-core unipolar XOR DCO + 7-voice Supersaw stack |
| **`OSC-03`** | **`KlangCZ Phase-X`** | **`Phase Distortion Synthesizer`** | Dual-line cosine angle warping + EA-1 Hard Sync |
| **`OSC-05`** | **`KlangPluck KS`** | **`Physical Waveguide Pluck`** | Tuned circular delay line, Lagrange pitch & loop drive |
| **`OSC-09`** | **`KlangSlayer`** | **`Phase-Locked Sub-Bass Generator`** | MIDI-tracked sub sine + recursive sine wavefold & noise |
| **`OSC-11`** | **`KlangCymbal Lab`** | **`Inharmonic Metallic Generator`** | 4 metallic cores + 8-stage all-pass bronze dispersion |
| **`OSC-12`** | **`KlangChip 8-Bit`** | **`Programmable Sound Generator (PSG)`**| 5 retro gaming chips (Pocket Matrix, Family 8, Demon Castle Saw, Breadbin SID, Turbo Engine 16) |

---

### 🔊 OSC-01: KlangTribe R (ER-1 Tribute Drum Core) — [S-TIER / LOCKED]
*(Corporate Boring: **`Percussion Synthesizer`**)*
* **Pedigree:** Clean-room emulation of the 1999 Korg Electribe R percussion synthesizer voice.
* **Architecture:** Carrier oscillator (Sine $\leftrightarrow$ Triangle) frequency-modulated and amplitude-modulated by a multi-mode modulation generator. Pitch sweeps are handled via the Farmer bipolar Pitch Envelope. Mod Type 5 generates continuous white noise ring-modulated with a sine wave at `Mod Speed` ($M(t) = \text{Noise}(t) \times \sin(2\pi f_{\text{speed}} t)$), producing rhythmic chatter at LFO rates and metallic snare sizzle at audio rates.
* **8 Performance Knobs:** `PITCH` (Hz / Note), `WAVE` (Sine $\leftrightarrow$ Tri), `DECAY` (Snappy $10\,\text{ms} \dots 10.0\,\text{s}$ / Drone $60.0\,\text{s}$), `LEVEL` (dB), `MOD TYPE` (Saw, Square, Tri, S&H, Cyclic Noise), `MOD SPEED` ($0.1\,\text{Hz} \dots 5\,\text{kHz}$), `MOD DEPTH` ($-100\% \dots +100\%$), `FEEDBACK` (Self-mod intensity).

---

### 🔊 OSC-02: KlangAlpha (Alpha Juno Dual-DCO + JP-8000 Stack) — [S-TIER / LOCKED]
*(Corporate Boring: **`Dual DCO & Unison Synthesizer`**)*
* **Pedigree:** Roland Alpha Juno 1/2 & MKS-50 custom Fujitsu MB87123 DCO ASIC crossed with Roland JP-8000 Supersaw architecture. Monophonic, percussive, ripping electronic basslines and snarling Hoover leads.
* **Architecture:** Dual-core parallel DCO running two independent unipolar XOR wave generators with PolyBLEP anti-aliased step transitions. All 14 waveforms are normalized to the single-cycle fundamental period $[0.0, 1.0)$ so any wave can be played at any pitch without hardwired octave drops. Wave 1 feeds a JP-8000-style `SUPER` stack (up to 7 detuned PolyBLEP copies) while Wave 2 acts as a rock-solid anchor (sub, nasal 5th, or secondary layer). Features audio-rate PWM ($0.02\,\text{Hz} \dots 2.5\,\text{kHz}$) and fine bipolar detune ($-12 \dots +12$ semitones).
* **Waveforms (14 Shapes):** Saws 1–5 (Linear, PWM Saw, Stepped Saw 1/2, Double Saw), Squares 1–3 (Square, Pulse PWM, Narrow Spike), Shapes 9–14 (Sub 1/2, Asymmetric Notch, Dual Unequal, Stepped Pulse 1/2).
* **8 Performance Knobs:** `WAVE 1` (14 Shapes), `OCTAVE` ($-4 \dots +4$), `WAVE 2` (Off + 14 Shapes), `SUPER` ($0\% \dots 100\%$ voice density), `PWM AMOUNT` ($0\% \dots 100\%$), `PWM SPEED` ($0.02\,\text{Hz} \dots 2.5\,\text{kHz}$), `DETUNE` ($-12 \dots +12$ st), `RETRIGGER` (`FREE` DCO, `RESET` $0^\circ$, `RANDOM`).

---

### 🔊 OSC-03: KlangCZ Phase-X (Casio CZ Twin-Line Phase Distortion) — [S-TIER / LOCKED]
*(Corporate Boring: **`Phase Distortion Synthesizer`**)*
* **Pedigree:** Casio CZ-101 / CZ-1000 / CZ-1 crossed with Korg EA-1 sync and modern audio-rate phase distortion.
* **Architecture:** Dual-line Phase Distortion (PD) engine running two independent DCO-DCW lines. Replaces recursive IIR filters with non-linear cosine angle warping ($\theta_{distorted} = f(\theta_{linear}, \text{DCW})$), generating razor-sharp resonant sweeps with zero filter phase lag or transient smearing. Features a 7-mode interaction matrix (including Korg EA-1 style wide-range Hard Sync up to +5 octaves, Kilohearts-style audio-rate Phase Distortion, and Logic XOR) paired with a skeuomorphic Backstage DIP switch governing the mixing mathematics.
* **8 Algorithms:** Saw, Square, Pulse, Double Sine, Saw-Pulse, Reso 1 (Saw sync), Reso 2 (Tri sync), Reso 3 (Trap sync).
* **7 Interaction Modes:** Normal ($L_1 + L_2$), Ring Mod ($L_1 \times L_2$), Phase Distort ($\theta_2' = \theta_2 + k L_1$), Cross Mod, Hard Sync ($L_1$ resets $L_2$), Logic XOR, Min/Max Wavefold.
* **8 Performance Knobs:** `WAVE 1`, `PD 1` (DCW 1), `WAVE 2`, `PD 2` (DCW 2), `OCTAVE`, `DETUNE 2` ($-1 \dots +5$ Oct), `AMP MIX`, `INTERACTION` (7 Modes).
* **Rear Deck DIP:** `MIX LAW` (`SAT BOOST` Padé Tanh, `DIODE SPLICE` Max/Min, `STEREO SPREAD` Vector M/S).

---

### 🔊 OSC-05: KlangPluck KS (Karplus-Strong Physical Model) — [S-TIER / LOCKED]
*(Corporate Boring: **`Physical Waveguide Pluck`**)*
* **Pedigree:** Kevin Karplus & Alex Strong physical modeling crossed with Elektron Octatrack tuned comb filter techniques. Monophonic, overdriven physical model engineered specifically for rubbery sub-bass, slap bass, kalimbas, and metallic percussion.
* **Architecture:** Short excitation burst (White Noise, Pink Noise, Click, Saturated Thump) injected into a tuned circular delay line with fractional-delay Lagrange pitch interpolation and a 1-pole low-pass loss filter. Features a variable pick position comb filter, all-pass string stiffness/inharmonicity dispersion, a secondary wooden body cavity resonator, and internal Padé tanh loop saturation for singing sustain and warm bass drive.
* **8 Performance Knobs:** `OCTAVE` ($-4 \dots +4$ Oct), `DECAY` ($RT_{60}$ $10\,\text{ms} \dots 10.0\,\text{s}$ Snappy / $60.0\,\text{s}$ Drone), `DAMPING` (HF loss filter), `EXCITER` (Encoder Turn: burst length; Encoder Press: White Noise, Pink Noise, Click, Thump), `PICK POSITION` (0.05 bridge bite $\leftrightarrow$ 0.50 hollow center), `STRETCH` (All-pass inharmonic stiffness), `BODY RES` (Wooden cavity resonance), `DRIVE` (Padé tanh loop drive).

---

### 🔊 OSC-09: KlangSlayer (Phase-Locked Sub & Sine Wavefold) — [S-TIER / LOCKED]
*(Corporate Boring: **`Phase-Locked Sub-Bass Generator`**)*
* **Pedigree:** Roland TR-808 kick oscillator crossed with Mick Gordon (DOOM 2016 / Eternal) pseudo-acoustic sub-harmonic synthesis techniques.
* **Architecture:** Dual-layer sub engine. Core 1 is a pure fundamental sine oscillator locked to MIDI pitch ($-4 \dots +4$ Octaves) with a continuous exponential decay tail up to $60.0\,\text{s}$. Core 2 is a selectable transient click / attack harmonic layer. Core 1 feeds a Mick Gordon recursive sine wavefolder ($f(x) = \sin(x \cdot \text{fold})$) with DC symmetry bias (even harmonics) and an audio-rate Noise Ring-Modulation circuit ($Sub \times Noise$) that generates a tearing pseudo-acoustic harmonic halo, ensuring earth-shaking sub-bass is clearly audible on iPhone and laptop speakers.
* **8 Performance Knobs:** `OCTAVE` ($-4 \dots +4$, tracked to MIDI), `SUB DECAY` ($10\,\text{ms} \dots 10.0\,\text{s}$ Snappy / $60.0\,\text{s}$ Drone), `LAYER 2 TYPE` (Press: Analog Click, Acoustic Mallet, 2nd Harm, 3rd Harm, Dirt Noise), `LAYER 2 MIX` ($0\% \dots 100\%$), `SINE FOLD` (Mick Gordon wavefolder drive), `NOISE RING` (Sub $\times$ Noise heterodyne sidebands), `SYMMETRY` (DC offset even harmonics), `PHASE LOCK` ($0^\circ$ Lock for punch $\leftrightarrow$ Free for continuous leads).

---

### 🔊 OSC-11: KlangCymbal Lab (Paiste Bronze & Metallic Clatter) — [S-TIER / LOCKED]
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

### 🔊 OSC-12: KlangChip 8-Bit (Vintage Console & Chiptune Lab) — [S-TIER / LOCKED]
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

## 5. The 8 Bespoke Standalone Processors

### 👑 1. `KlangStomp` (8-Pedal Analog Dirt Hall of Fame)
*(Corporate Boring: **`Analog Stompbox Suite`**)*
* **The 8 Circuit Models:**
  1. `Swedish Chainsaw` (Boss HM-2): Unbiased BJT dead-band zero-crossing distortion + dual gyrators (~100Hz low & ~1kHz high resonance).
  2. `Industrial Zone` (Boss MT-2): Dual op-amp hard clip + sweepable semi-parametric mid EQ ($200\,\text{Hz} \dots 5\,\text{kHz} \pm 15\,\text{dB}$).
  3. `Rodent` (ProCo RAT): LM308 op-amp slew-rate limiting + reverse passive low-pass filter.
  4. `Screamer 808` (Ibanez TS9/TS808): Non-inverting clipping amp with antiparallel silicon diodes + 720 Hz high-pass roll-off.
  5. `Sovereign Centaur` (Klon Centaur): Dual-gang clean blend pot + germanium soft clip + 18V charge pump headroom.
  6. `Russian Muff` (Electro-Harmonix Big Muff): 4 discrete BJT stages with 1024 Hz passive mid-notch tone stack.
  7. `Yellow Preamp 250` (DOD 250 / MXR Dist+): LM741 op-amp gain with back-to-back clipping diodes.
  8. `Velcro Face` (Fuzz Face): 2-transistor Germanium/Silicon direct-coupled BJTs with voltage starvation (gated velcro decay).
* **8 Performance Knobs:** `GAIN / DRIVE`, `TONE / FILTER`, `LEVEL / VOL`, `CIRCUIT MODEL` (8 Pedals), `MID / GYRATOR` (Parametric mid / Chainsaw peak), `STARVE / VOLTS` ($3\,\text{V} \dots 18\,\text{V}$ rail starve/pump), `CLEAN BLEND` ($0\% \dots 100\%$), `MIX / ∅` (Profile M Tri-Zone Mix + Phase Invert).

---

### 👑 2. `KlangMieda` (Harmonic Distortion Synthesizer)
*(Corporate Boring: **`Harmonic Distortion Synthesizer`**)*
* **Pedigree:** Clean-room emulation of Fumio Mieda’s 3-circuit parallel distortion synthesizer architecture.
* **Architecture:** 3 parallel intermodulating distortion circuits:
  - Circuit 1: Warmth (soft saturation & fundamental reinforcement).
  - Circuit 2: Vowel Wavefold (asymmetric formant waveshaping).
  - Circuit 3: Crossover Bite (dead-band zero-crossing high-frequency fuzz).
* **8 Performance Knobs:** `WARMTH DRIVE`, `VOWEL FOLD`, `BITE GAIN`, `BALANCE 1-2`, `BALANCE 3`, `TRIMMER / BIAS`, `GATE THRESH`, `MIX / ∅` (Profile M Tri-Zone Mix + Phase Invert).

---

### 👑 3. `KlangSilver` (Extreme Multiband Dynamics Suite)
*(Corporate Boring: **`Multiband Upward/Downward Dynamics`**)*
* **The Dual Engines:**
  - **Mode 1: Classic OTT:** 3-band Linkwitz-Riley 4th order crossover ($120\,\text{Hz} \dots 2.5\,\text{kHz}$) driving simultaneous upward and downward compression.
  - **Mode 2: Argent +100dB DOOM:** Inspired by Nimble Tools Argent (French/Latin for Silver). Pushes upward compression to $+100\,\text{dB}$ with an integrated zero-latency hard clipper and glitch knee.
* **8 Performance Knobs:** `DEPTH` ($0\% \dots 100\%$), `TIME` ($0.1\times \dots 10\times$ attack/release scale), `UPWARD GAIN` ($0\,\text{dB} \dots +36\,\text{dB}$ / $+100\,\text{dB}$ in Argent Mode), `DOWNWARD COMP` (Threshold & ratio), `LOW BAND` ($-24\,\text{dB} \dots +24\,\text{dB}$), `MID BAND` ($-24\,\text{dB} \dots +24\,\text{dB}$), `HIGH BAND` ($-24\,\text{dB} \dots +24\,\text{dB}$), `MIX / ∅` (Profile M Tri-Zone Mix + Phase Invert).

---

### 👑 4. `KlangSmear` (All-Pass Phase Disperser)
*(Corporate Boring: **`All-Pass Phase Disperser`**)*
* **Architecture:** Cascaded all-pass phase dispersion network ($1^{\text{st}}, 2^{\text{nd}}, 4^{\text{th}}$ Order stages) designed to smear transient energy across time without altering frequency response. Delivers laser drum thumps, squelchy Paiste bronze clatter, and neurofunk bass punch.
* **8 Performance Knobs:** `FREQUENCY` ($20\,\text{Hz} \dots 20\,\text{kHz}$, Profile D Log), `STAGES / AMOUNT` (1 to 32 all-pass filter poles), `RESONANCE / Q` ($0.1 \dots 18.0$), `ORDER TYPE` (1st, 2nd, 4th Order all-pass blocks), `STEREO SPREAD` (Phase offset between Left and Right channels), `PRE-DRIVE` ($0\,\text{dB} \dots +18\,\text{dB}$ Padé tanh saturation), `LOW DECOUPLE` (High-pass sidechain keeping sub-bass phase intact), `MIX / ∅` (Profile M Tri-Zone Mix + Phase Invert).

---

### 👑 5. `KlangBode` (Harald Bode Frequency Shifter & SSB Modulator)
*(Corporate Boring: **`Single-Sideband Frequency Shifter`**)*
* **Architecture:** Harald Bode single-sideband (SSB) frequency shifter using dual 90-degree Hilbert transform networks and quadrature sine/cosine carrier oscillators. Features dual-antiphase stereo shifting (Left shifts UP while Right shifts DOWN) and an integrated feedback delay loop.
* **8 Performance Knobs:** `SHIFT FREQ` (Bipolar $-5\,\text{kHz} \dots 0\,\text{Hz} \dots +5\,\text{kHz}$, Profile B S-Curve), `FINE TUNE` ($-20\,\text{Hz} \dots +20\,\text{Hz}$), `SIDEBAND BLEND` (Lower Sideband $\leftrightarrow$ Ring Mod $\leftrightarrow$ Upper Sideband), `FEEDBACK` ($0\% \dots 115\%$ spiraling feedback loop), `DELAY TIME` ($0\,\text{ms} \dots 1000\,\text{ms}$ or Host Sync), `STEREO DUAL` ($0\% \dots 100\%$ antiphase Left Up / Right Down width), `TONE / DAMP` (High-frequency damping in feedback loop), `MIX / ∅` (Profile M Tri-Zone Mix + Phase Invert).

---

### 👑 6. `KlangComb` (Tuned Waveguide Resonator & Acoustic Cavity Delay)
*(Corporate Boring: **`Acoustic Waveguide Resonator`**)*
* **Pedigree:** Kevin Karplus & Alex Strong waveguide modeling crossed with the Korg Volca Drum Wave Guide physical modeling resonator.
* **Architecture:** Tuned sub-millisecond circular delay line with fractional Lagrange pitch interpolation, all-pass inharmonic phase dispersion, high-loss damping, low-cut anti-mud decoupling, and Padé tanh loop saturation.
* **The 4 Waveguide Engines (Rear Deck DIP 1):**
  1. `Volca Tube`: Inverted reflection (Negative Feedback $-$). Sings purely in odd harmonics ($1, 3, 5, 7$), generating hollow bamboo slits, conga cavities, and woody percussive resonance!
  2. `Volca String`: Normal reflection (Positive Feedback $+$). Sings in all integer harmonics ($1, 2, 3, 4$) for metallic wire and ringing steel rims.
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

### 👑 7. `KlangFallout` (Wavetable Waveshaper & Glitch Lab)
*(Corporate Boring: **`Wavetable Waveshaper`**)*
* **Pedigree:** WMD Geiger Counter & Kilohearts ShaperTable.
* **Architecture:** Wavetable waveshaping engine where instantaneous audio amplitude directly indexes into a 256-wavetable lookup table containing mathematical transfer functions (Chebyshev polynomials, bitwise XOR/AND folds, digital staircases, and diode curves).
* **8 Performance Knobs:** `PRE-GAIN / DRIVE` ($-12\,\text{dB} \dots +36\,\text{dB}$), `TABLE / WAVE` (256 Curves across 4 Banks), `MORPH / SCAN` ($0\% \dots 100\%$ smooth wavetable interpolation), `BIT DEPTH` ($1 \dots 16\,\text{Bits}$), `SAMPLE RATE` ($200\,\text{Hz} \dots 48\,\text{kHz}$ decimator), `MASK / TONE` (Post-shaper high/low filter), `SLEW / SMOOTH` (Non-linear slew rate limiter rounding harsh digital spikes), `MIX / ∅` (Profile M Tri-Zone Mix + Phase Invert).

---

### 👑 8. `KlangVortex` (Phase-Fold Chaos Resonator)
*(Corporate Boring: **`Phase-Fold Resonator`**)*
* **Architecture:** The ultimate mad-scientist processor crossing a West Coast multi-stage Buchla wavefolder with a multi-order All-Pass Filter network in a recirculating, self-modulating feedback loop.
* **The 3 Routing Modes (Knob 3 Press / Rear Deck):**
  1. `Mode A: [ FOLD ──► APF ]`: Folds the waveform first to generate dense harmonics, then phase-disperses the overtones across time into singing laser drops.
  2. `Mode B: [ APF ──► FOLD ]`: Disperses the phase first! Different frequencies reach the wavefolder thresholds at different delay times, fracturing the waveform into wildly asymmetrical, liquid neurofunk snarls.
  3. **`Mode C: [ DUAL RECIRCULATING CHAOS LOOP ]`**: Both processors live inside the feedback loop, creating a closed-loop fractal generator that self-oscillates into infinite chaotic harmonic fractals!
* **8 Performance Knobs:** `FOLDS` ($1 \dots 8$ Stages), `SMEAR FREQ` ($20\,\text{Hz} \dots 20\,\text{kHz}$ APF center pitch), `APF STAGES & ROUTING` ($1^{\text{st}}, 2^{\text{nd}}, 4^{\text{th}}, 8^{\text{th}}$ Order; Press: Cycle Modes A, B, C), `FEEDBACK` ($-115\% \dots +115\%$ Profile F Hyperbolic), `SYMMETRY / BIAS` (DC offset even harmonics), `CROSS-FM` ($0\% \dots 100\%$ wavefolder output modulates APF cutoff), `DAMPING` ($1\,\text{kHz} \dots 20\,\text{kHz}$ loop filter), `MIX / ∅` (Profile M Tri-Zone Mix + Phase Invert).

---

## 6. Implementation Roadmap & Staging Gates

### 🎯 Milestone v0.5.0: The Sound & Chaos Update
1. **Core 7 S-Tier Sound Engines:**
   - `OSC-01: KlangTribe R` (ER-1 FM/AM Cyclic Noise)
   - `OSC-02: KlangAlpha` (Alpha Juno Dual-DCO + JP-8000 Stack)
   - `OSC-03: KlangCZ Phase-X` (Casio CZ Twin-Line Phase Distortion)
   - `OSC-05: KlangPluck KS` (Karplus-Strong Tuned Comb Resonator)
   - `OSC-09: KlangSlayer` (Phase-Locked Sub & Mick Gordon Wavefold)
   - `OSC-11: KlangCymbal Lab` (4-Core Metallic Clatter + 8-Stage APF Dispersion)
   - `OSC-12: KlangChip 8-Bit` (5-Console Vintage Chiptune Lab)
2. **Core 8 Mega-Effects:**
   - `KlangWarp`, `KlangPunch`, `KlangFilter`, `KlangEQ`, `KlangMod`, `KlangShift`, `KlangDelay`, `KlangSpace`.
3. **Core 8 Bespoke Processors:**
   - `KlangStomp`, `KlangMieda`, `KlangSilver`, `KlangSmear`, `KlangBode`, `KlangComb`, `KlangFallout`, `KlangVortex`.

---

### 🚀 Milestone v0.6.0: The Modular Fleet & The Klang Mill ("The Spinoff Spree")
Following the battle-testing and parameter stabilization of all 16 audio processors inside *The Klang Farmer* and *ToadTracker* during v0.5.0:

1. **Stamp Out the 16 Standalone Plugins (The Kilohearts-Style Modular Fleet):**
   - Extract all **8 Mega-Effects** and all **8 Bespoke Processors** into independent, featherweight, ultra-low-CPU VST3/CLAP plugins powered by the shared `TbdAudio::DSPBlock` core.
   - Modeled on the legendary **Kilohearts Snapin / Goodhertz / Melda** ecosystem: producers can insert single standalone processors (e.g. just `KlangSilver`, `KlangVortex`, or `KlangSpace`) across any DAW mixer track with zero bloat.
   - **The Standalone Invariant:** All 16 standalone spinoff plugins **ALWAYS STAY IN KLANG MODE PERMANENTLY**.
2. **Launch `The Klang Mill` (TKM):**
   - The sovereign multi-effect rack, host, and modulation matrix capable of chaining, routing, and modulating any combination of Klang modules in serial, parallel, or multiband splits with unified macro encoders.
3. **Shared DSP Binary Architecture (`libtks_dsp`):**
   - A single statically linked C++ DSP library compiled across Farmer, ToadTracker, Mill, and the 16 standalone plugin targets via unified CMake build targets.

