# Master Specification: `DigiStrike` (`MEGA-PERC-05`)

**Document Status:** CANONICAL ECOSYSTEM SPECIFICATION 📜  
**Target Systems:** The Klang Suite (`TKS`), ToadTracker (`TT`), The Klang Mill (`TKM`), Argon DSP  
**Compliance Standard:** Universal 16-Control Card Architecture (`master_control_canon_and_template.md`)  
**IP Clearance Standard:** Clean-Room IP Scrubber Invariant (`tkdsp_16_control_canon.md`)  
**Authors:** Antigravity x Cody G. (Klang Industries / ToadTracker Audio Division)  
**Date:** 2026-10-20  

---

## 1. Executive Summary & The "Singles" Paradigm

Following the Universal 16-Control Architecture, `DigiStrike` (`MEGA-PERC-05`) serves as the ecosystem's **Universal Vintage Digital Drum & PCM Single-Voice Engine**.

Rather than cramming an entire drum kit into a single compromised card with shared macros, `DigiStrike` adopts the **Singles-First Paradigm**:
* **100% Parameter Independence**: Every single drum voice (Kick, Snare, Rimshot, Hi-Hat, Tom, Cymbal) is instantiated as an independent, fully autonomous 16-control card. A Kick on Track 1 has its own variable 555-clock pitch tuning, Curtis CEM3320 VCF sweep, and Am6070 non-linear drive completely isolated from the Snare on Track 2 or the Hats on Track 3.
* **Elektron-Inspired Real-Time Performance Mangling**: Pitch bend and dynamic pitch dive envelopes are handled natively by the tracker/host modulators (P-locks, note slides, glide columns). This frees the voice card's front panel to host real-time **Machinedrum (MD), Monomachine (MnM), Octatrack (OT), and Digitakt (DT)** glitch controls (`STRT`, `LEN` + `LOOP`, `REV`, and `SRR` decimation).
* **Hardware-Calibrated Preset Defaults**: When any drum voice is loaded, all 16 controls automatically initialize to **authentic factory hardware trims** (exact 555 clock rate, CEM3320 cutoff, and analog decay).

---

## 2. 🛡️ The Clean-Room IP Scrubber & Serial Number Filing Matrix

In strict compliance with the Clean-Room IP Scrubber Invariant, all commercial trademarks and third-party entity names are completely scrubbed across public C++ code, headers, class definitions, UI displays, and documentation:

* **Sovereign Card Name:** `DigiStrike` (C++ namespace: `toad::DigiStrike` / class: `DigiStrikeEngine`)
* **Corporate Boring Name:** `Vintage Digital EPROM & PCM Drum Voice Processor`

| Index | Historical Manufacturer | Sovereign Brand Name (DIP 1) | Corporate Boring Title | Canonical Machine Lineage (DIP 2) |
| :---: | :--- | :--- | :--- | :--- |
| **01** | **Linn Electronics** | **`Crown Audio`** | *Vintage Studio PCM Voice* | `Crown-80 (LM-1)` · `Crown-82 (LinnDrum)` · `Crown-9k (Linn 9000)` |
| **02** | **Oberheim** | **`Apex Electronics`** | *Companded 8-Bit Drum Voice* | `Apex-81 (DMX)` · `Apex-83 (DX)` · `Apex Burner (Prommer)` |
| **03** | **E-mu Systems** | **`Empyrean Systems`** | *12-Bit Ringing Slew Voice* | `Empyrean-83 (Drumulator)` · `Empyrean-12 (SP-12/1200)` · `Command-XL (XL-7)` · `Command-MP (MP-7)` |
| **04** | **Red Falcon (Akai)** | **`Red Falcon`** | *16-Bit Velocity Pad Sampler* | `Falcon-10 (XR10)` · `Falcon-60 (MPC60)` · `Falcon-3k (MPC3000)` · `Rack-8 (XE8)` |
| **05** | **K-Works (Kawai)** | **`K-Works`** | *16-Bit Filtered Percussion Synth* | `K-Beat 50 (R-50)` · `K-Electro 50 (R-50e)` · `K-Mark 3 (R-50 III)` · `K-Perc 5 (XD-5)` |
| **06** | **Compact Beat (Boss)** | **`Compact Beat`** | *Portable Lo-Fi Rhythm Generator* | `Doc-110 (DR-110)` · `Doc-220A (DR-220A)` · `Doc-220E (DR-220E)` · `Doc-550` · `Doc-660` |
| **07** | **Tokyo Beat (Roland)** | **`Tokyo Beat`** | *Linear Matrix Rhythm Voice* | `Matrix-50 (TR-505)` · `Matrix-70 (TR-707)` · `LatinMatrix-72 (TR-727)` · `Hybrid-90 (TR-909 PCM)` · `HumanRhythm-8 (R-8)` |
| **08** | **Gothenburg Lab (Elektron)**| **`Gothenburg Lab`** | *Swedish Algorithmic Percussion* | `Gothenburg-Digi (Monomachine DigiPro)` · `Gothenburg-E12 (Machinedrum E12)` |
| **09** | **Circuits Lab (Sequential)**| **`Circuits Lab`** | *CEM3320 Tunable Digital Rhythm* | `Traks-84 (Drumtraks)` · `Tom-85 (TOM)` · `Studio-44 (Studio 440)` |
| **10** | **Phase Beat (Casio)** | **`Phase Beat`** | *Dynamic Lo-Fi Sampler Voice* | `Cosmo-RZ (RZ-1)` · `Cosmo-SK (SK-1)` · `Cosmo-FZ (FZ-1)` |
| **11** | **Black Studio (Alesis)** | **`Black Studio`** | *Dynamic 16-Bit Sample Player* | `Studio-HR (HR-16)` · `Studio-HRB (HR-16B)` · `Studio-SR (SR-16)` |
| **12** | **Keio Sound (Korg)** | **`Keio Sound`** | *Dynamic Digital Drum Voice* | `Keio-D1 (DDD-1)` · `Keio-D5 (DDD-5)` · `Keio-S3 (S3)` |
| **13** | **Nippon Sound (Yamaha)** | **`Nippon Sound`** | *12-Bit Waveform Memory Voice* | `Nippon-RX5 (RX5)` · `Nippon-RX11 (RX11)` · `Nippon-RY30 (RY30)` |
| **20** | **M-Audio (Avid / inMusic)** | **`Adios`** | *Single-Generation Hybrid PCM Drum Voice Processor* | `Mayhem-11 (Venom)` |

---

## 3. 🎛️ The 16-Control Invariant Architecture

### A. Front 8 Performance Encoders (Knobs 1..8)

```
Page 1A (Top Row / Left Hand: Tone & Wavetable Warp)
[ Knob 1: PITCH ]          [ Knob 2: DECAY ]          [ Knob 3: LENGTH / TIME ]  [ Knob 4: WINDOW / RANGE ]
(Variable 555 Pitch Clock) (VCA Envelope Release)     (Wavetable Time Stretch)   (Transient/Tail Window)

Page 1B (Bottom Row / Right Hand: Filter, Grit & Destruction)
[ Knob 5: CUTOFF ]         [ Knob 6: RESO / Q ]       [ Knob 7: DRIVE / COMPAND] [ Knob 8: SRR / CRUSH ]
(Curtis CEM3320 24dB VCF)  (Filter Ring / Resonance)  (Am6070 Saturation Drive)  (Machinedrum Decimation)
```

| Control | Parameter | Lineage | Range | Curve | Push Action | Description |
| :---: | :--- | :--- | :---: | :---: | :---: | :--- |
| **K01** | `PITCH / CLOCK` | Vintage HW | $8.0\text{ kHz} \dots 60.0\text{ kHz}$ | Profile D (Log) | *[Factory Stock Pitch]* | Variable 555-timer pitch clock. Tunable across 4 octaves. |
| **K02** | `DECAY / GATE` | Vintage HW | $10\text{ ms} \dots 3.0\text{ s}$ | Profile B (Lin) | *[Full One-Shot Sample]* | Analog VCA release and gate time. |
| **K03** | `LENGTH / TIME` ⚡ | Vital / OT | $0.0 \dots 1.0$ (Center: $0.5$) | Profile A (Lin) | *[Toggle Direction: FWD / REV]* | **Wavetable Time-Stretch**: Controls the frame-traverse rate across the wavetable. At $0.5$ (default), plays back at authentic factory speed. $0.0$ speeds up traverse to a super-short, laser-fast transient sweep; $1.0$ stretches traverse into a massive, eerie spectral time-stretched drone (proving in-engine that voices are true resynthesized wavetables, not static one-shot samples!). **Push Action**: Toggles playback direction (`FORWARD` $\leftrightarrow$ `REVERSE`). In Reverse mode with Ping-Pong loop, starts backwards and bounces forward. |
| **K04** | `WINDOW / RANGE` ⚡ | Vital / TT | $0.0 \dots 1.0$ (Center: $0.5$) | Profile B (Lin) | *[Cycle Loop Mode (4 States)]* | **Bidirectional Playback Window**: At $0.5$ (default), plays 100% of the wavetable from start to finish. In the range $0.5 \dots 0.0$, sweeps the end-point back toward the start (truncating the tail into ultra-tight mechanical chops). In the range $0.5 \dots 1.0$, sweeps the start-point forward toward the end (skipping the initial transient to isolate the pure acoustic body ring, tail, or ambient room reverb). **Push Action**: Cycles the 4-state loop mode: `[ ONE-SHOT ➔ FORWARD LOOP ➔ PING-PONG ➔ SINGLE CYCLE (FRAME) ]`. Single-Cycle mode locks the playhead to a single 2048-sample waveform frame, instantly transforming any drum hit into a playable synth oscillator or sub-bass! |
| **K05** | `FILTER CUTOFF` | CEM3320 | $200\text{ Hz} \dots 22.0\text{ kHz}$ | Profile D (Log) | *[Wide Open Filter]* | Curtis 24dB resonant lowpass reconstruction VCF. |
| **K06** | `FILTER RESO / Q` | CEM3320 | $0.5 \dots 10.0$ | Profile B (Lin) | *[Flat Butterworth]* | Resonant peak / acoustic drum ringing tone. |
| **K07** | `DRIVE / COMPAND` | Am6070 | $0.0 \dots +24.0\text{ dB}$ | Profile B (Lin) | *[Pristine Linear DAC]* | Am6070 non-linear chord ladder expansion and analog saturation. |
| **K08** | `SRR / CRUSH` ⚡ | MD / MnM | $44.1\text{ kHz} \dots 1.0\text{ kHz}$ | Profile H (Exp) | *[Bypass Decimator]* | **Machinedrum `SRR`**: True sample-and-hold audio decimation independent of pitch clock, introducing savage aliasing and metallic crunch. |

### B. Rear Deck Controls (Controls 9..16 / Sound Lab ⚙️)

* **DIP 1 (`Company Select`)**: Selects the Sovereign Brand Family (`Crown`, `Apex`, `Empyrean`, `Red Falcon`, `K-Works`, `Compact Beat`, `Tokyo Beat`, `Gothenburg Lab`, `Circuits Lab`, `Phase Beat`, `Black Studio`, `Keio Sound`, `Nippon Sound`).
* **DIP 2 (`Machine Select`)**: Selects the canonical machine model dynamically filtered by the selected company (e.g. `Crown-80`, `Crown-82`, `Crown-9k`).
* **DIP 3 (`Drum Voice Select`)**: Selects the active drum voice (`Kick`, `Snare`, `Rimshot`, `Hi-Hat`, `Tom`, `Conga`, `Cowbell`, `Clap`, `Tambourine`, `Crash`, `Ride`).
* **DIP 4 (`DAC & Filter Circuit`)**: `[ Am6070 Piecewise Chord | 6-Bit BA662 VCA | 8-Bit Linear | 12-Bit Ringing Slew | 32-Bit Sinc Float ]`.
* **TRIM 1 (`555 Clock Flutter & Thermal Drift`)**: Simulates analog oscillator thermal instability ($0.0\% \dots 5.0\%$).
* **TRIM 2 (`White Noise / VCA Bleed Injection`)**: Authentically injects analog white noise (e.g. DMX / 808 snare card noise circuit).
* **TRIM 3 (`CEM3320 Slew / Warmth Trim`)**: Calibrates analog filter slew rate and operational amplifier bandwidth.
* **TRIM 4 (`Output Safety Limiter Ceiling`)**: Hardware headroom safety limiter ceiling ($\pm 0.0\text{ dBFS}$).

---

## 4. 🔬 Mathematical DSP & Hardware Circuit Formulations

### A. The AMD Am6070 Piecewise Linear Chord Ladder DAC
The Am6070 converts an 8-bit companded byte $B$ into an exact piecewise linear chord ladder magnitude:
$$\text{Sign} = \begin{cases} -1 & \text{if } (B \ \& \ 0x80) == 0 \\ +1 & \text{if } (B \ \& \ 0x80) \neq 0 \end{cases}$$
$$m = B \ \& \ 0x7F, \quad \text{Chord} = (m \gg 4) \ \& \ 0x07, \quad \text{Step} = m \ \& \ 0x0F$$
$$\text{Base} = [0, 131, 395, 923, 1979, 4091, 8315, 16763][\text{Chord}]$$
$$\text{StepSize} = [8, 16, 32, 64, 128, 256, 512, 1024][\text{Chord}]$$
$$\text{Output} = \text{Sign} \cdot (\text{Base} + \text{Step} \cdot \text{StepSize})$$
This yields an exact symmetrical saturation ceiling of $\pm 32,123$ digital units, modeling the exact physical R-2R ladder voltages of the LM-1, LinnDrum, and DMX.

### B. Curtis CEM3320 2-Pole / 4-Pole ZDF Lowpass Filter
Implemented as an unconditionally stable Zero-Delay Feedback Topology-Preserving Transform (TPT) state variable filter:
$$g = \tan\left(\frac{\pi f_c}{f_s}\right), \quad R = \frac{1}{2Q}, \quad D = 1 + 2 R g + g^2$$
$$v_{hp}[n] = \frac{v_{in}[n] - (2R + g) s_1[n] - s_2[n]}{D}$$
$$v_{bp}[n] = g \cdot v_{hp}[n] + s_1[n], \quad v_{lp}[n] = g \cdot v_{bp}[n] + s_2[n]$$
State updates:
$$s_1[n+1] = g \cdot v_{hp}[n] + v_{bp}[n], \quad s_2[n+1] = g \cdot v_{bp}[n] + v_{lp}[n]$$

### C. Machinedrum Sample Rate Reduction (`SRR`)
Simulates true audio decimation via discrete sample-and-hold accumulation:
$$\text{StepSize} = \frac{f_s}{f_{\text{target}}}, \quad x_{\text{held}}[n] = x\left[\left\lfloor \frac{n}{\text{StepSize}} \right\rfloor \cdot \text{StepSize}\right]$$
Generating authentic metallic inharmonic fold-over frequencies without shifting the fundamental playback pitch of the EPROM.

---

## 5. 📂 Master Sample Tree & Provenance Architecture

All samples follow the strict 3-tier hierarchy:
```
C:\Dev\Samples\<Company>\<Product>\<Voice>
```

### Complete 20-Manufacturer & Curio Fleet (2,069 Voices · 6,207 Standardized Triads · 2,168 VitalTables · 29,194 Audio Frames):
1. **`Crown Audio` (Linn Electronics)**: 77 voices (231 WAVs)
   * `LM-1/` (19 voices · 57 WAVs) · `LinnDrum/` (29 voices · 87 WAVs from authentic factory EPROMs) · `Linn9000/` (29 voices · 87 WAVs from authentic EPROMs)
2. **`Apex Electronics` (Oberheim)**: 15 voices (45 WAVs)
   * `DMX/` (15 voices · 45 WAVs from authentic Electrongate EPROMs) + 22 VitalTables
3. **`Compact Beat` (Boss)**: 28 voices (84 WAVs)
   * `DR-110/` (6 voices) · `DR-220A/` (11 voices) · `DR-220E/` (11 voices)
4. **`Tokyo Beat` (Roland)**: 118 voices (354 WAVs)
   * `TR-505/` (16 voices) · `TR-707/` (15 voices) · `TR-727/` (15 voices) · `TR-626/` (20 voices) · `TR-909_PCM/` (4 voices) · `R-8 / R-70/` (48 voices including HI-Q Zap, Scratches, Basses, Bells, Cuicas, CR-78 beats)
5. **`K-Works` (Kawai)**: 179 voices (537 WAVs)
   * `R-50/` (26 voices) · `R-50e/` (40 voices: full canon including Orchestral Hit, Electric Bass, Funk Basses, Flanged Hit, Gated Hit, Timpani, Snap, Clicks) · `R-50_III/` (29 voices including Orch Hits, Slap Bass, Lasers) · `XD-5/` (21 voices) · `R-100/` (24 voices 12-bit PCM @ 32 kHz)
6. **`Red Falcon` (Akai)**: 132 voices (396 WAVs)
   * `XR-10/` (68 voices: full 64 front-panel sounds + 4 KB6 Misc Zaps/Slap Bass) · `MPC60/` (20 voices) · `MPC3000/` (20 voices) · `XE8/` (24 voices)
7. **`Empyrean Systems` (E-mu)**: 64 voices (192 WAVs)
   * `Drumulator/` (12 voices) · `SP-1200/` (20 voices) · `MP-7/` (16 voices) · `XL-7/` (16 voices)
8. **`Gothenburg Lab` (Elektron)**: 48 voices (144 WAVs)
   * `Machinedrum_E12/` (24 voices) · `Monomachine_DigiPro/` (24 voices)
9. **`Circuits Lab` (Sequential Circuits)**: 39 voices (117 WAVs)
   * `Drumtraks/` (13 voices) · `TOM/` (8 voices) · `Studio440/` (18 voices)
10. **`Phase Beat` (Casio)**: 46 voices (138 WAVs)
    * `RZ-1/` (12 voices) · `SK-1/` (26 voices including CZ-Bass, Synth Drums, Loop) · `SK-5/` (8 voices including Dog, Lion, Laser Gun)
11. **`Black Studio` (Alesis)**: 106 voices (318 WAVs)
    * `HR-16/` (20 voices) · `HR-16B/` (47 voices: full canon including Bottles, Pipe, Lip Pop, Ambient Puh, Scratch, Synth Block, Torque Wrench, Trash Can, Glass Break) · `SR-16/` (39 voices)
12. **`Keio Sound` (Korg)**: 67 voices (201 WAVs)
    * `DDD-1/` (18 voices) · `DDD-5/` (29 voices) · `S3/` (20 voices)
13. **`Nippon Sound` (Yamaha)**: 190 voices (570 WAVs)
    * `RX5/` (63 voices: full canon including Glass Crash, Cuica, FMPerc, Whistle, Castinet, Timpani, Heavy sets, EToms) · `RX11/` (20 voices) · `RY30/` (102 voices including Super Bass, Talk Scratch, Zap Snare, LFO Noise, Metal Smasher, Coke Up, Clay, Echo)
14. **`Sydney Sound` (Fairlight Instruments)**: 162 voices (486 WAVs)
    * `Page-IIx/` (69 voices: 8-bit variable-clock classic sampler canon including BOBSNARE, COLLINSR, CLAPTRAP, Drumbo, Gorgon, Pang, Symkick, SymSnre, Symtom) · `Page-III/` (93 voices: 16-bit 44.1 kHz digital mainframe canon including GlassBreak, Iron Hit, AmbiBell, Filter FX, Noise Slap, Pole Hit, Tjupp, Tjot)
15. **`Dartmouth Sound` (New England Digital / Synclavier)**: 45 voices (135 WAVs)
    * `Synclavier-II/` (45 voices: 16-bit additive/FM direct-to-disk mainframe canon: Bd1..12, Sn1..10, SN10..14, SNROLL1..5, Hh1..10, Drm1..3, Syn1, SynSn8)
16. **`Malvern Works` (Ensoniq Corporation)**: 137 voices (411 WAVs)
    * `Mirage/` (31 voices: 8-bit variable-clock sampler with Curtis CEM3328 24dB VCFs) · `EPS/` (20 voices: 13-bit variable-clock workstation) · `ASR-X/` (86 voices: 16-bit 44.1 kHz resampling workstation with digital resonant filters)
17. **`Hexagon Sound` (Simmons Drums Ltd)**: 165 voices (495 WAVs)
    * `SDS-V/` (45 voices: 1981 discrete analog drum synth with SSM2044 VCF and noise generator from authentic master line captures) · `SDS-7/` (20 voices: 8-bit EPROM sound cards with analog VCF/VCA) · `SDS-9/` (44 voices: 12-bit digital PROM & analog hybrid) · `SDX/` (56 voices: 16-bit 44.1 kHz sampling drum computer)
18. **`Berlin Beat` (Jomox Elektronische Musikinstrumente)**: 117 voices (351 WAVs)
    * `XBase-09/` (42 voices: discrete analog 909-killer kick & snare with sample hats/rides) · `AirBase-99/` (75 voices: 1U discrete analog rackmount percussion synth)
19. **`Curio Foundry` (The Boutique Curio Cabinet)**: 293 voices (879 WAVs)
    * `Cheetah_MD16/` (95 voices: Cheetah-16 British 16-bit expanded grit) · `Vermona_DRM1/` (40 voices: Erzgebirge-1 discrete analog percussion) · `DSI_Tempest/` (37 voices: Tempest-DSI analog DCO + digital PCM hybrid) · `Elka_Drumstar/` (31 voices: Ancona-80 Italian PCM disco rhythm) · `Simmons_ClapTrap/` (25 voices: ClapTrap-82 1982 analog noise clap synthesizer) · `Solton_Disco/` (25 voices: Ancona-Disco-64 1982 Italian analog/PCM disco rhythm) · `Synare_3/` (40 voices: Star-Synare-3 1978 flying saucer analog percussion synth)
20. **`Adios` (M-Audio / Avid)**: 41 voices (123 WAVs)
    * `Mayhem-11/` (41 voices · 123 WAV triads from authentic Dream SAM3308B PCM soundbank: 13 Kicks, 11 Snares, 13 Toms/Perc, 4 Cymbals) + 41 VitalTables

Every voice folder contains the **Standardized 44.1 kHz 16-Bit Triad**:
1. `*_original.wav`: Clean, unpadded raw PCM resampled to 44.1 kHz.
2. `*_romrender.wav`: Filtered through Curtis CEM3320 / Sallen-Key ZDF lowpass at hardware-calibrated cutoff.
3. `*_tkdsp.wav`: Processed with Am6070 non-linear saturation knee and analog thermal slop.

All 2,069 voices are accompanied by:
- Hardware-calibrated 16-control presets in `recipe.json` and `recipe.md` (mirrored in `TheKlangDSP/data/recipes/`).
- 2048-sample morph wavetables (`.vitaltable`) in `TheKlangDSP/data/wavetables/<company>/` (2,127 tables deployed).
- Full FTS5 BM25 search index in `.agents/index/wavetable_corpus.db` (29,194 audio frames).

---

## 6. 🛡️ Dual-Tier Privacy Enclave Architecture

```
┌─────────────────────────────────────────────────────────┐
│         PRIVATE ENCLAVE (Local Vault / Dropbox)         │
│  - Raw .BIN EPROM binaries & memory address maps        │
│  - Factory service manuals & hardware schematics        │
│  - Unmodified sample triads (*_original, *_romrender)   │
│  - Trademarked entity names & historical teardowns     │
└────────────────────────────┬────────────────────────────┘
                             │ Clean-Room Modeled
                             ▼
┌─────────────────────────────────────────────────────────┐
│        PUBLIC SOVEREIGN ENGINE (TheKlangDSP / TKS)       │
│  - DigiStrike (MEGA-PERC-05) C++ DSP Classes             │
│  - Resynthesized Multi-Frame Vitaltables (*.vitaltable)  │
│  - Clean-Room IP Scrubber Matrix (All Names Filed Off)  │
│  - 100% Free to distribute / Open-source MIT             │
└─────────────────────────────────────────────────────────┘
```

---

## 7. 🔍 Agent FTS5 Lexicon & Keyword Search Index

`digistrike`, `mega-perc-05`, `linn`, `lm-1`, `linndrum`, `lm-2`, `linn-9000`, `crown-80`, `crown-82`, `crown-9k`, `am6070`, `companding`, `chord-ladder`, `cem3320`, `curtis-vcf`, `machinedrum-srr`, `octatrack-strt-len`, `eprom-decoder`, `sample-triad`, `clean-room-ip`.
