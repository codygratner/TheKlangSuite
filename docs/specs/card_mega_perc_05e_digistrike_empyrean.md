# Master Specification: `Principia Empyrea` (`MEGA-PERC-05E`)
## Dual-Layer PCM Wavetable & 14-Pole 3D Z-Plane Synthesis Engine

**Document Status:** CANONICAL ECOSYSTEM SPECIFICATION 📜  
**Target Systems:** The Klang Suite (`TKS`), ToadTracker (`TT`), The Klang Mill (`TKM`), Argon DSP  
**Compliance Standard:** Universal 16-Control Card Architecture (`master_control_canon_and_template.md`)  
**IP Clearance Standard:** Clean-Room IP Scrubber Invariant (`tkdsp_16_control_canon.md`)  
**Authors:** Antigravity x Cody G. (Klang Industries / ToadTracker Audio Division)  
**Date:** 2026-10-21  

---

## 1. Executive Summary & The "Proteus Preset Machine" Paradigm

`Principia Empyrea` (`MEGA-PERC-05E` / `toad::PrincipiaEmpyrea`) establishes the definitive celestial synthesis instrument within the ecosystem, housing the complete vintage sound universe of the **E-mu Proteus 2000, Morpheus, Audity 2000, and Command Station ROM series**, encompassing **21 authentic hardware ROM soundbanks**, **14,653 instruments**, and **43,959 sample-accurate 16-bit WAV files**.

While sharing the ultra-low-latency real-time PCM playback and modulation core of `DigiStrike`, `Principia Empyrea` completely reorients the user interface and sound-sculpting engine toward the **Proteus / Z-Plane Synthesis Paradigm**:
1. **Dynamic 14-Pole 3D Z-Plane Filtering**: Rather than basic 2-pole or 4-pole lowpasses, every voice card features a full 14-pole (7-biquad cascade) morphing filter capable of vowel formants, physical acoustic cavities, sweeping comb resonators, and radical multi-peak textures.
2. **Dual-Layer Architecture**: Every voice card hosts **two independent PCM layers** (Layer 1: Primary + Layer 2: Secondary / Sub-Octave / Detune).
3. **Tri-Encoder Layer Swap Push Invariant**: Pushing **ANY** of the three rear browsing encoders (**DIP 1**, **DIP 2**, or **DIP 3**) performs the identical action: **`TOGGLE EDIT FOCUS: SOUND 1 (PRIMARY) ↔ SOUND 2 (SECONDARY)`**. This empowers the user to swap, select, and customize Layer 2 to any sound from any ROM card or category with zero menu diving.
4. **Push vs. Push+Twist Front Panel Ergonomics**: To prevent catastrophic accidental sound changes ("pushed intending to turn, hesitated, and unpushed"), front performance encoders enforce a strict single-personality rule:
   * **Push Encoders (Discrete Selectors)**: Click cycles modes cleanly (`[ Mode A | Mode B | Mode C ]`).
   * **Push+Twist Encoders (Continuous Sub-Trims)**: Alt-drag / push-and-turn edits the companion sub-trim. Releasing without rotation is a strict safety no-op!
5. **3D Wavetable Morph & Frame Traverse (Approach 2 Canon)**:
   * **K03 (`TABLE MORPH / POSITION`)**: Front-panel 3D wavetable position scrubber ($0\% \dots 100\%$). On multi-sample wavetables, sweeps smoothly across timbre profiles (`Sub` $\to$ `Deep` $\to$ `Body` $\to$ `Crisp` $\to$ `Bite`). On single-cycle / 1-shot samples, dynamically transforms into a non-linear Spectral Tilt / Wavefolder Drive so it is never a dead knob. Push+Twist modulates Morph Envelope Depth.
   * **K04 (`TRAVERSE / LENGTH`)**: Frame traverse rate & time-stretch speed ($0.0\times \dots 10.0\times$). Setting to $0.0\times$ freezes the wavetable at the current K03 position. Push cycles speed presets (`[ 1.0x Stock | Freeze (0.0x) | 0.5x Half | 2.0x Double ]`).
6. **Decoupled Amplitude Envelope**: VCA amplitude attack/decay parameters are decoupled from the voice card and offloaded to the host channel strip, keeping the 16 controls laser-focused on timbre, looping, time-stretch, and Z-plane morphing.
7. **Triple-WAV Library Standard**: Every waveform is rendered in three complementary formats containing authentic **RIFF `smpl` chunk loop markers** for hardware samplers (Elektron Tonverk, Dirtywave M8, Akai MPC):
   * `*_original.wav`: Unaltered 16-bit PCM extracted from ROM pools with native sample rate and factory loop points.
   * `*_romrender.wav`: 44.1 kHz resampled through E-mu G-chip anti-aliased interpolation, 2-pole Butterworth reconstruction filter, and 0.95 peak normalization.
   * `*_tkdsp.wav`: 44.1 kHz processed through non-linear analog saturation ($y = \tanh(1.15 x)$) with 0.95 peak normalization, preserving full dynamic punch and sub-bass clarity.

---

## 2. 🛡️ Clean-Room IP Scrubber & Product Filing Matrix

In compliance with the Clean-Room IP Scrubber Invariant, all commercial trademarks and third-party names are scrubbed across public code, class names, UI elements, and specs:

* **Sovereign Card Name:** `Principia Empyrea` (C++ namespace: `toad::PrincipiaEmpyrea` / class: `PrincipiaEmpyreaEngine`)
* **Sovereign Brand Name:** `Empyrean Systems`
* **Corporate Boring Title:** `16-Bit Dual-Layer Linear PCM & 14-Pole Z-Plane Dynamic Synthesizer`

| Sovereign ROM Name (DIP 1) | Historical E-mu ROM Lineage | Size | Sound Focus | Primary Categories |
| :--- | :--- | :---: | :--- | :--- |
| **`Pulse Garden`** | **Beat Garden (Rob Papen)** | 32 MB | Tech-Groove, Techno, Industrial, D&B | `kit`, `bas`, `syn`, `bpm`, `sfx` |
| **`Street Phatt`** | **Mo' Phatt (Pure Phatt)** | 32 MB | Hip-Hop, R&B, Trap, Funk 808/909 | `kit`, `bas`, `hit`, `vox`, `prc` |
| **`Protean Core`** | **Composer (Proteus 2000)** | 32 MB | General MIDI, Acoustic, World, Pop | `kit`, `bas`, `pad`, `led`, `syn` |
| **`Aero Garden`** | **Techno Synth Construction Yard** | 32 MB | Trance, Hard House, Aggressive Acid | `syn`, `led`, `bas`, `bpm`, `hit` |
| **`Club Xtreme`** | **Xtreme Lead-1** | 32 MB | Rave, Eurodance, Hard Trance, Sync | `led`, `syn`, `bas`, `bpm`, `sfx` |
| **`Planet Groove`** | **Planet Earth** | 32 MB | World Percussion, Ethnic Wind, Chimes | `prc`, `kit`, `pad`, `vox`, `bpm` |
| **`Vintage Pantheon`**| **Vintage Pro** | 32 MB | Classic Keys, Mellotron, Rhodes, Modular | `syn`, `pad`, `bas`, `led`, `kit` |
| **`Protean Kit`** | **Protean Drums** | 32 MB | Studio Drums, Acoustic Snares, Cymbals | `kit`, `prc` |
| **`Command Percussion`**| **PX-7 Command Station** | 32 MB | Urban Percussion, Grime Kits, Industrial | `kit`, `prc`, `hit`, `sfx` |
| **`Orbit 9090`** | **Orbit-3 Dance** | 32 MB | 90s House, Breakbeat, Jungle, Rave | `kit`, `bpm`, `syn`, `bas`, `hit` |
| **`Planet Classic`** | **Planet Phatt** | 16 MB | Golden-Era Hip-Hop, Funk Basses | `bas`, `kit`, `prc`, `syn` |
| **`Vintage Keys Classic`**| **Vintage Keys** | 16 MB | Hammond B3, Wurlitzer, Clavinet, Farfisa | `syn`, `pad`, `bas` |
| **`Virtuoso Symphonic`**| **Virtuoso 2000** | 64 MB | Full Orchestral Strings, Woodwinds, Brass | `pad`, `led`, `hit`, `syn` |
| **`Proteus Prime`** | **Proteus/1 Pop/Rock** | 8 MB | Early 90s Pop/Rock Staples, Guitars | `syn`, `bas`, `kit`, `pad` |
| **`Proteus Symphonic`**| **Proteus/2 Orchestral** | 8 MB | Classic Orchestral Solo & Ensemble | `pad`, `led`, `syn` |
| **`Proteus Terra`** | **Proteus/3 World** | 8 MB | Authentic World Folk Instruments | `prc`, `syn`, `pad`, `vox` |
| **`PROcussion Pro`** | **PROcussion Drum Module** | 8 MB | Multi-Velocity Acoustic & Electronic Perc | `kit`, `prc`, `hit` |
| **`Synth Mania`** | **Synth Mania** | 32 MB | Analog Synthesizer Collections & Modular | `syn`, `bas`, `led`, `pad` |
| **`Empyrean-12`** | **SP-1200** | 4 MB | Gritty 12-Bit Vintage Sampling Icon | `kit`, `prc`, `hit`, `sfx` |
| **`Production Studio`**| **E-mu Production Set** | 32 MB | High-Definition Studio Production Staples | `syn`, `kit`, `pad`, `bas` |
| **`AstroTones`** | **AstroTones** | 2 MB | Celestial Formants & Ambient Textures | `syn`, `pad` |

---

## 3. 📂 The Canonical E-mu Sound Navigator Categories

Grounded directly in the official **E-mu Proteus 2000 / Command Station Sound Navigator** standard:

| Category Code | Display Name | Functional Description & Sound Palette |
| :---: | :--- | :--- |
| **`kit`** | **Drum Kits & Beats** | Full drum arrangements, acoustic & electronic kick/snare/hat combinations, 808/909 kits. |
| **`prc`** | **Percussion & Bells** | Shakers, congas, bongos, cowbells, ethnic percussion, metallic chimes, industrial metal clangs. |
| **`bas`** | **Basses & Subs** | Heavy sub-basses, Dutch gabber kicks, Moog/303 squelches, acoustic upright, slap bass, Reese/growl. |
| **`led`** | **Synth Leads** | Screaming hard-sync leads, analog saw leads, mono glide solos, vocal formant leads. |
| **`pad`** | **Atmospheres & Pads** | Lush ambient filter beds, evolving motion textures, dreamscapes, string ensembles, cathedral choirs. |
| **`syn`** | **Poly Synths & Arps** | Polyphonic analog brass, digital FM bells, plucked arpeggios, trance stabs, chord textures. |
| **`hit`** | **Orchestra & Rave Hits** | Rave chord stabs, orchestra crashes, horror hits, industrial gunshots, metallic impacts. |
| **`bpm`** | **Tempo Rhythms** | Synchronized breakbeats, sliced rhythm loops, tempo-locked modulation sequences. |
| **`vox`** | **Vocal & Choirs** | Formant vowel chants (A-E-I-O-U), robot talkbox, ethereal solo voices, choir sweeps. |
| **`sfx`** | **Special Effects & Noise** | Laser zaps, vinyl scratches, white noise sweeps, modular feedback bursts, alien sweeps. |

---

## 4. 🎛️ The 16-Control Parameter Canon (Proteus Edition)

```
┌────────────────────────────────────────────────────────────────────────┐
│                   FRONT 8 ENCODERS (Performance View)                  │
├───────────────────────────────────┬────────────────────────────────────┤
│   Page 1A: Wavetable Generation   │      Page 1B: Sculpt & Master      │
│   [ K01: PITCH / DETUNE ]         │      [ K05: CUTOFF / MORPH X ]     │
│   [ K02: START OFFSET / REVERSE ] │      [ K06: RESO / DRIVE MODEL ]   │
│   [ K03: TABLE MORPH / POSITION ] │      [ K07: TRANSFORM / MORPH Z ]  │
│   [ K04: TRAVERSE / LENGTH ]      │      [ K08: LAYER MIX / DETUNE ]   │
└───────────────────────────────────┴────────────────────────────────────┘
                                     │ Flip Card (Settings / Sound Lab)
                                     ▼
┌────────────────────────────────────────────────────────────────────────┐
│                   REAR DECK ⚙️ (Controls 9..16 / Sound Lab)            │
├───────────────────────────────────┬────────────────────────────────────┤
│   DIP 1..4: Sound Navigator & Loop│      TRIM 1..4: Sound Lab Trims    │
│   [ DIP 1: Product / ROM Card ]   │      [ TRIM 1: PatchCord LFO Rate] │
│   [ DIP 2: Navigator Category ]   │      [ TRIM 2: Master Z-Env Depth] │
│   [ DIP 3: Sound / Waveform ]     │      [ TRIM 3: Z-Plane Topology ]  │
│   [ DIP 4: Looping & Window Chop] │      [ TRIM 4: Output Ceiling ]    │
└───────────────────────────────────┴────────────────────────────────────┘
```

### A. Front 8 Performance Encoders (Knobs 1..8)

| Encoder | Personality | Primary Action (Turn) | Secondary Action (Push OR Push+Twist) | Physical Behavior & Range |
| :---: | :---: | :--- | :--- | :--- |
| **K01** | **`Push+Twist`** | **Coarse Pitch** ($\pm 36\text{ st}$) | **Fine Detune** ($\pm 100\text{ cents}$) | Clutch to fine-tune; releasing without turning does nothing. |
| **K02** | **`Push+Twist`** | **Start Offset** ($0\% \dots 100\%$) | **Playback Direction / Reverse** | Scrub start point. Push+Twist toggles Forward $\leftrightarrow$ Reverse. |
| **K03** | **`Push+Twist`** | **Table Morph / Position** ($0\% \dots 100\%$) | **Table Morph Env Depth** ($-100\% \dots +100\%$) | **3D Wavetable Morph Scrubber.** Sweeps register dimensions (`Sub` $\to$ `Bite`). On 1-shots: acts as non-linear Wavefolder / Spectral Tilt. Clutch to dial in transient morph punch. |
| **K04** | **`Push`** | **Traverse / Length** ($0.0\times \dots 10.0\times$) | **[Freeze / Cycle Speed]** | Frame traverse rate & time-stretch speed. $0.0\times$ freezes the wavetable at current K03 position. Push cycles `[ 1.0x Stock | Freeze (0.0x) | 0.5x Half | 2.0x Double ]`. |
| **K05** | **`Push+Twist`** | **Cutoff / Morph X** ($20\text{Hz} \dots 20\text{kHz}$) | **Cutoff Env Depth** ($-100\% \dots +100\%$) | Clutch to dial in transient percussive filter crack. Logarithmic sweep. |
| **K06** | **`Push`** | **Resonance / Morph Y** ($Q\text{ }0.5 \dots 40$) | **Reso Drive Model** (`[ Clean | Warm | Saturated ]`) | Quick click steps through analog filter saturation stages. |
| **K07** | **`Push+Twist`** | **Transform / Morph Z** ($0\% \dots 100\%$) | **Morph Z Env Depth** ($-100\% \dots +100\%$) | Clutch to dial in dynamic 3D vowel/pole morph on every hit. |
| **K08** | **`Push+Twist`** | **Layer Mix** ($0\% \dots 100\%$) | **Layer 2 Pitch Detune** ($\pm 24\text{ st}$) | Blend Layer 1 $\leftrightarrow$ Layer 2. Push: [Both \| L1 \| L2]. Push+Twist: Layer 2 Detune. |

### B. Rear Deck Controls (Controls 9..16 / Sound Lab ⚙️)

| Control | Parameter Name | Hardware Lineage | Range / States | Push Action / Physical Functionality |
| :---: | :--- | :--- | :---: | :--- |
| **DIP 1** | **`Product / ROM Card`** | Proteus 2000 Card Slots | 21 ROM Cards | *[Layer Focus Swap]*: Pushing DIP 1 toggles Sound 1 $\leftrightarrow$ Sound 2 focus. |
| **DIP 2** | **`Navigator Category`** | Command Station Navigator| 10 Categories | *[Layer Focus Swap]*: Pushing DIP 2 toggles Sound 1 $\leftrightarrow$ Sound 2 focus. |
| **DIP 3** | **`Sound / Waveform`** | EBL Waveform Pool | 1..1899 Waves | *[Layer Focus Swap]*: Pushing DIP 3 toggles Sound 1 $\leftrightarrow$ Sound 2 focus. |
| **DIP 4** | **`Looping & Window Chop`**| E-mu Sampler Core | 4 Loop Modes | `[ Hard Wrap | Equal-Power Smooth Crossfade | Ping-Pong | 1-Shot / Gate Chop ]` |
| **TRIM 1** | **`PatchCord LFO Rate`** | E-mu PatchCord Matrix | $0.05\text{ Hz} \dots 25.0\text{ Hz}$ | Free-running or tempo-locked triangle/sine LFO modulation rate. |
| **TRIM 2** | **`Master Z-Env Depth`** | E-mu Filter Envelope | $-100\% \dots +100\%$ | Global scaling depth for transient envelopes feeding Cutoff and Morph Z. |
| **TRIM 3** | **`Z-Plane Filter Model`** | Morpheus H-Chip Models | 7 Topologies | `[ Vocal Formant | 6-Pole Resonant LP | Comb Resonator | Phaser Sweep | Bandpass Matrix | Notch Cluster | Acid Tube ]` |
| **TRIM 4** | **`Output Ceiling Limiter`**| Analog Output Stage | $-12.0\text{ dBFS} \dots 0.0\text{ dBFS}$ | Master output ceiling and headroom padding. |

---

## 5. 🔬 Mathematical DSP & Filter Formulations

### A. 14-Pole 3D Morphing Z-Plane Transfer Function
The Z-Plane filter processor cascades seven 2-pole bi-quadratic filter sections in series, with real-time dynamic interpolation of poles and zeros across the 3D cube $[X, Y, Z]$:
$$H(z) = g(X, Y, Z) \prod_{i=1}^{7} \frac{b_{0i}(X, Y, Z) + b_{1i}(X, Y, Z) z^{-1} + b_{2i}(X, Y, Z) z^{-2}}{1 + a_{1i}(X, Y, Z) z^{-1} + a_{2i}(X, Y, Z) z^{-2}}$$

Where:
* $X \in [0.0, 1.0]$: Frequency / Cutoff axis ($20\text{ Hz} \dots 20.0\text{ kHz}$)
* $Y \in [0.0, 1.0]$: Resonance / Q-Factor axis ($0.5 \dots 40.0$)
* $Z \in [0.0, 1.0]$: Morph / Transform axis (interpolating between pole constellation $A$ and pole constellation $B$)

### B. Transient Cutoff & Morph Z Modulation
When an envelope trigger occurs, the instantaneous cutoff frequency $f_c(t)$ and morph position $Z(t)$ evolve according to:
$$f_c(t) = f_{\text{base}} \cdot 2^{\text{Env}(t) \cdot \text{Depth}_{\text{cutoff}} \cdot \text{Master}_{\text{depth}}}$$
$$Z(t) = \text{clamp}\left(Z_{\text{base}} + \text{Env}(t) \cdot \text{Depth}_{\text{morph}} \cdot \text{Master}_{\text{depth}}, 0.0, 1.0\right)$$
Where $\text{Env}(t) = e^{-t / \tau_{\text{decay}}}$ provides the sharp exponential transient dive for punchy kicks, acid squelches, and formant morphs.

### C. Equal-Power Smooth Loop Crossfading
In DIP 4 Mode 2 (`Equal-Power Smooth Crossfade`), a cosine-sine crossfade window of length $L_{\text{fade}}$ (typically 4ms to 16ms) is applied across the loop boundary to eliminate phase discontinuity clicks while preserving total acoustic energy:
$$x_{\text{out}}(t) = x_{\text{head}}(t) \cdot \cos\left(\frac{\pi t}{2 L_{\text{fade}}}\right) + x_{\text{tail}}(t) \cdot \sin\left(\frac{\pi t}{2 L_{\text{fade}}}\right), \quad t \in [0, L_{\text{fade}}]$$

---

## 6. 📂 Master Sample Library & Triple-Render Pipeline

All waveforms are extracted into `C:\Dev\Samples\Empyrean\<ROM>\<category>\`:

| Sovereign ROM Name | Target Directory | Instruments | Rendered WAVs (3x) |
| :--- | :--- | :---: | :---: |
| **`Beat Garden`** (Pulse Garden) | [`C:\Dev\Samples\Empyrean\Beat_Garden\`](file:///C:/Dev/Samples/Empyrean/Beat_Garden/) | 767 | 2,301 |
| **`Mo' Phatt`** (Street Phatt) | [`C:\Dev\Samples\Empyrean\Mo_Phatt\`](file:///C:/Dev/Samples/Empyrean/Mo_Phatt/) | 1,283 | 3,849 |
| **`Composer`** (Protean Core) | [`C:\Dev\Samples\Empyrean\Composer\`](file:///C:/Dev/Samples/Empyrean/Composer/) | 1,899 | 5,697 |
| **`Orbit-3`** (Orbit 9090) | [`C:\Dev\Samples\Empyrean\Orbit_3\`](file:///C:/Dev/Samples/Empyrean/Orbit_3/) | 1,298 | 3,894 |
| **`PROcussion`** (PROcussion Pro) | [`C:\Dev\Samples\Empyrean\PROcussion\`](file:///C:/Dev/Samples/Empyrean/PROcussion/) | 191 | 573 |
| **`PX-7`** (Command Percussion) | [`C:\Dev\Samples\Empyrean\PX7\`](file:///C:/Dev/Samples/Empyrean/PX7/) | 702 | 2,106 |
| **`Planet Earth`** (Planet Groove) | [`C:\Dev\Samples\Empyrean\Planet_Earth\`](file:///C:/Dev/Samples/Empyrean/Planet_Earth/) | 748 | 2,244 |
| **`Planet Phatt`** (Planet Classic) | [`C:\Dev\Samples\Empyrean\Planet_Phatt\`](file:///C:/Dev/Samples/Empyrean/Planet_Phatt/) | 516 | 1,548 |
| **`Production Set`** (Production Studio) | [`C:\Dev\Samples\Empyrean\Production_Set\`](file:///C:/Dev/Samples/Empyrean/Production_Set/) | 384 | 1,152 |
| **`Protean Drums`** (Protean Kit) | [`C:\Dev\Samples\Empyrean\Protean_Drums\`](file:///C:/Dev/Samples/Empyrean/Protean_Drums/) | 703 | 2,109 |
| **`Proteus/1`** (Proteus Prime) | [`C:\Dev\Samples\Empyrean\Proteus_1\`](file:///C:/Dev/Samples/Empyrean/Proteus_1/) | 217 | 651 |
| **`Proteus/2`** (Proteus Symphonic) | [`C:\Dev\Samples\Empyrean\Proteus_2\`](file:///C:/Dev/Samples/Empyrean/Proteus_2/) | 298 | 894 |
| **`Proteus/3`** (Proteus Terra) | [`C:\Dev\Samples\Empyrean\Proteus_3\`](file:///C:/Dev/Samples/Empyrean/Proteus_3/) | 171 | 513 |
| **`SP-1200`** (Empyrean-12) | [`C:\Dev\Samples\Empyrean\SP1200\`](file:///C:/Dev/Samples/Empyrean/SP1200/) | 334 | 1,002 |
| **`Synth Mania`** (Synth Mania) | [`C:\Dev\Samples\Empyrean\Synth_Mania\`](file:///C:/Dev/Samples/Empyrean/Synth_Mania/) | 311 | 933 |
| **`Techno Yard`** (Aero Garden) | [`C:\Dev\Samples\Empyrean\Techno_Yard\`](file:///C:/Dev/Samples/Empyrean/Techno_Yard/) | 573 | 1,719 |
| **`Vintage Keys`** (Vintage Keys Classic) | [`C:\Dev\Samples\Empyrean\Vintage_Keys\`](file:///C:/Dev/Samples/Empyrean/Vintage_Keys/) | 727 | 2,181 |
| **`Vintage Pro`** (Vintage Pantheon) | [`C:\Dev\Samples\Empyrean\Vintage_Pro\`](file:///C:/Dev/Samples/Empyrean/Vintage_Pro/) | 1,061 | 3,183 |
| **`Virtuoso 2000`** (Virtuoso Symphonic) | [`C:\Dev\Samples\Empyrean\Virtuoso_2000\`](file:///C:/Dev/Samples/Empyrean/Virtuoso_2000/) | 1,094 | 3,282 |
| **`Xtreme Lead`** (Club Xtreme) | [`C:\Dev\Samples\Empyrean\Xtreme_Lead\`](file:///C:/Dev/Samples/Empyrean/Xtreme_Lead/) | 1,372 | 4,116 |
| **`AstroTones`** (AstroTones) | [`C:\Dev\Samples\Empyrean\AstroTones\`](file:///C:/Dev/Samples/Empyrean/AstroTones/) | 4 | 12 |
| **TOTAL FLEET SCALE** | **21 Hardware ROM Soundbanks** | **14,653 Instruments** | **43,959 16-Bit WAVs** |

---

## 7. 🔗 Canonical Cross-References & Ecosystem Deliverables

* **JSON Voice Recipes:** Deployed in [`C:\Dev\TheKlangDSP\data\recipes\`](file:///C:/Dev/TheKlangDSP/data/recipes/)
* **Markdown Voice Recipes:** Deployed in [`C:\Dev\TheKlangDSP\data\recipes\`](file:///C:/Dev/TheKlangDSP/data/recipes/) and mirrored into [`New Klang City/Docs/specs/`](file:///C:/_/Vault/New%20Klang%20City/Docs/specs/)
* **Master Specification Mirror:** [`C:\Dev\TheKlangSuite\docs\specs\card_mega_perc_05e_digistrike_empyrean.md`](file:///C:/Dev/TheKlangSuite/docs/specs/card_mega_perc_05e_digistrike_empyrean.md)
* **Core C++ Headers:** `toad::PrincipiaEmpyrea` in `TheKlangDSP/include/dsp/oscillators/`
