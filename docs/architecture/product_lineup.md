# The Cult of Klang: Canonical Product Lineup & Lore Matrix

The Cult of Klang is an underground, sovereign audio synthesis and sound design imprint developed in modern C++20 and JUCE 9. This document formalizes the official **Product Matrix, Acronym Taxonomy, and Core Intent** for every instrument, editor, and hardware offering across the ecosystem.

---

## 🏛️ Acronym Taxonomy & Clean-Room Lineup

To ensure brand clarity, acoustic prestige, and zero naming collisions across C++ header guards, logging macros, and file formats, the following three-letter acronyms are ratified:

| Code | Canonical Name | Old Device | Role & Category | Primary Form Factor |
| :---: | :--- | :--- | :--- | :--- |
| **`TT`** | **ToadTracker** | ToadTracker | Flagship Standalone Workstation | Standalone Desktop (16:9 Widescreen) |
| **`TR`** | **The Relic** | TKR1 / TKR4 / Kombine | Flagship Vector, Choke & Multi-SEM Synth | VST3 / AU / CLAP / Standalone (The "Redux" to TT) |
| **`TF`** | **The Foundry** | The Klang Mill | 6-Pedal Industrial Multi-Effects Rack | VST3 / AU Audio Plugin (Housing Klang Mega-Effects) |
| **`TP`** | **The Prophet** | The Klang Planter | Autonomous Generative Resonator Machine | VST3 / AU / Standalone Audio Plugin |
| **`TM`** | **The Monolith** | The Klang Farmer | Subterranean Semi-Modular Biosphere Lab | VST3 / AU / Standalone Audio Plugin |
| **`TB`** | **The Box** | The Klang Box | Embedded Hardware Platform | Dedicated Microcontroller Desktop / Eurorack Hardware |
| **`KE`** | **Klang Editor** | The Klang Editor | Preset, Combi, Theme & Modder Deck | Standalone JUCE Application |

> [!NOTE]
> **Complete Retirement of Agrarian Nomenclature**:
> All agrarian/farming names (*The Farmer*, *The Planter*, *The Kombine*, *The Mill*) are officially eradicated from UI surfaces in favor of lore-native monolith names:
> * **The Monolith (`TM`)**: The grand semi-modular synthesis biosphere.
> * **The Foundry (`TF`)**: The molten metal multi-effects playground.
> * **The Prophet (`TP`)**: The autonomous generative polyrhythm organism.
> * **The Relic (`TR`)**: The flagship vector & 12-patch Oberheim Multi-SEM workstation.
> * The "Klang" prefix is reserved strictly for the **5 Signature Klang Mega-Effects** (KlangWarp, KlangDelay, KlangSpace, KlangMod, KlangEquinox) and **Klang Editor (`KE`)**.

---

## 🎹 The Core Instrument & Tool Lineup

```
 ┌────────────────────────────────────────────────────────────────────────┐
 │                      THE CULT OF KLANG                                 │
 ├────────────────────────┬───────────────────────┬───────────────────────┤
 │  MODULAR BIOSPHERE     │  GENERATIVE ORGANISM  │  EFFECTS & FORGE      │
 │  TM: The Monolith      │  TP: The Prophet      │  TF: The Foundry      │
 │  (Patchbay & Resonator)│  (Seed & Germinate)   │  (6-Pedal Playground) │
 ├────────────────────────┼───────────────────────┼───────────────────────┤
 │  VECTOR, CHOKE & SEM   │  DEVELOPER DECK       │  PHYSICAL HARDWARE    │
 │  TR: The Relic         │  KE: Klang Editor     │  TB: The Box          │
 │  (Multi-SEM & Vector)  │  (Data & Theme Deck)  │  (Embedded Seed,      │
 │  [replaces Kombine/R1] │                       │   Daisy, Teensy 8-Out)│
 └────────────────────────┴───────────────────────┴───────────────────────┘
```

---

### 1. The Monolith (TM)
**"The Subterranean Semi-Modular Biosphere & Macro-Patchbay Laboratory" (The Cosmic Pillar)**

- **Core Intent**: The deep, uncompromising semi-modular sound design laboratory. Liberated from static drum roles by The Relic, The Monolith becomes an expansive West Coast / East Coast hybrid synthesis biosphere—patching complex CV networks, chaotic feedback, wavefolders, and physical subterranean acoustic modeling.
- **Form Factor**: VST3 / AU / Standalone.
- **Architecture**:
  - Full modular rack surface (800x600 to 4K scalable) with interactive virtual patch-cables and oscilloscope monitors.
  - Semi-modular macro patchbay: cross-modulation between operators, complex function generators (Maths style), wavefolders, and lowpass gates.
  - Subterranean environmental acoustic engine: waveguide mesh physical modeling of acoustic resonators, tensioned membranes, and wooden cavities.
  - Deep microtonal polyphony (Scala / MTS-ESP) and the sovereign Klang-Brain 2.0 algorithmic counterpoint sequencer.

---

### 2. The Prophet (TP)
**"The Autonomous Generative 'Seed & Germinate' Machine" (The Oracle of Resonance)**

- **Core Intent**: An autonomous, bio-physical generative percussion and melodic organism. Where The Relic provides surgical tracker control and vintage Oberheim/choke polyphony, The Prophet is alive, evolving, and unpredictably organic.
- **Form Factor**: VST3 / AU / Standalone.
- **Architecture**:
  - Bio-rhythmic germination: internal self-mutating cellular automata sequencers that grow from deterministic "seeds" into complex polymetric grooves.
  - Modal resonator bank: non-linear physical models of struck bars, marimbas, metallic plates, bells, and membrane skins.
  - Micro-tuning drift and probabilistic envelope branching.

---

### 3. The Foundry (TF)
**"The 6-Pedal Industrial Multi-Effects Playground & Forge"**

- **Core Intent**: The sonic forge where signals are crushed, modulated, space-warped, and tempered through a brutalist 6-pedal virtual pedalboard.
- **Form Factor**: VST3 / AU Audio Plugin.
- **Architecture**:
  - Houses the **5 Signature Klang Mega-Effects** (KlangWarp, KlangDelay, KlangSpace, KlangMod, KlangEquinox).
  - Integrates the **KlangStomp Pantheon**: Swedish Saw (`HM`), Zone of Terror (`MZ`), Rodent (`RT`), Russian Pi (`MF`), Green Screamer (`TS`), Velcro Face (`FF`), Ixion Centaur (`KC`), Yellow 250 (`YD`), Geiger Decimator (`GC`), ChopChop (`CC`), PipKlick (`PK`), Vocoder 56K (`VC`).
  - Flexible routing: Serial cascade, dual parallel branches with cross-mixing, and mid/side splitting.

---

### 4. The Relic (TR)
**"The Flagship Vector, Choke & Multi-SEM Workstation Engine" (The Redux to ToadTracker)**

- **Core Intent**: The definitive standalone and hosted rhythm/synth engine of The Cult of Klang. Blends 12-sound triplet choking, SP-1200 chromatic mono choking, continuous 2D equal-power vector morphing, and Oberheim Multi-SEM independent voice card allocation.
- **Form Factor**: VST3 / AU / CLAP / Standalone.
- **Architecture**:
  - **Combi (`.ttrc`) & Patch (`.ttrp`) Architecture**: 12 independent sound slots with root notes, continuous tuning, vintage slop, and Ableton unison warp.
  - **QuadraMod Engine**: 4 continuous audio-rate modulators per voice (AHD, Drum, LFO, ADSR).
  - **TetraTable Engine**: 4 discrete tick-rate micro-sequencing tables (`ATBL`, `BTBL`, `CTBL`, `DTBL`) with `VECT-XXYY` coordinate automation.

---

### 5. Klang Editor (KE)
**"The Standalone Data, Preset & Theme Deck"**

- **Core Intent**: The developer, modder, and sound designer control deck. Decouples parameter calibration, Combi/Patch authoring, and UI LookAndFeel design from DAW runtimes.
- **Form Factor**: Standalone JUCE Desktop Utility.
- **Architecture**:
  - Unified Master Tree View with multi-state layer filters (`[Controls]`, `[Layout]`, `[Theme]`).
  - Interactive parameter calibration workspace: test skew factors, snap points, and tactile responses live before saving.
  - Real-time LookAndFeel and color theme editor with live preview across all Cult of Klang instruments.

---

### 6. The Box (TB)
**"The Embedded Hardware Synthesizer Platform"**

- **Core Intent**: The physical hardware embodiment of The Cult of Klang's pure C++ DSP engine running on dedicated microcontrollers, bringing zero-allocation real-time sound synthesis into standalone hardware grooveboxes and Eurorack setups.
- **Form Factor**: Physical Hardware Desktop Units & Eurorack Modules.
- **Hardware Editions**:
  - **TB-Daisy (Electro-Smith Daisy Seed)**: STM32H750 ARM Cortex-M7 @ 480 MHz with 64 MB SDRAM, stereo 24-bit 96 kHz codec, 4 endless encoders, and OLED display.
  - **TB-8 (Studio Edition — PJRC Teensy 4.1)**: Flagship studio drum synthesizer powered by an NXP i.MX RT1062 Cortex-M7 @ 600 MHz and a Cirrus Logic CS42448 multi-channel codec. Features **8 discrete 1/4" analog outputs** for routing individual voices to outboard gear.
  - **TB-TBD (dadamachines tbd-16 Integration)**: Direct deployment onto the open-source dual-core ESP32-P4 RISC-V + RP2350 groovebox.
  - **Zynthian V5 Port**: Headless Linux LV2/CLAP integration on Raspberry Pi 5 with 1:1 optical encoder mapping.

---

## 🚨 Product Identity Drift Guardrail (The "Stay in Your Lane" Policy)

To preserve the focus and punch of each offering, **The Cult of Klang** monitors incoming ideas for **scope drift**:

| Product | The Core Trap (Scope Drift) | The Guardrail Response |
| :--- | :--- | :--- |
| **`TM`** *(The Monolith)* | **Dumbing it down**: Stripping out modular modulation, operator routing, or parameter depth to make it "easy". | *Flag*: "The Monolith is the flagship sound-design laboratory. Keep the depth; if you want immediate generative patches, route this to The Prophet!" |
| **`TP`** *(The Prophet)* | **Feature creep & menu bloat**: Adding multi-page tabs, deep sub-menus, or multi-slot FX racks. | *Flag*: "The Prophet is an immediate generative organism. Keep it single-surface and alive. If it needs 8 pages of routing, it belongs in The Monolith!" |
| **`TF`** *(The Foundry)* | **Identity confusion**: Adding synth oscillators, MIDI note playback, or complex tracker sequencers. | *Flag*: "The Foundry is a pure 6-pedal multi-effects forge (Soundtoys style). It processes external audio; it does not generate notes." |
| **`TR`** *(The Relic)* | **Breaking the Redux paradigm**: Stripping the 4 tables or 2D vector morphing. | *Flag*: "The Relic is the sovereign Redux instrument. Keep the 12 patches and 4 tables accessible and 100% bi-directional with ToadTracker!" |
| **`TB`** *(The Box)* | **Embedded unfriendliness**: Allocating heap memory or exceeding microcontroller SRAM/Flash budgets. | *Flag*: "The Box is dedicated bare-metal hardware. Keep DSP zero-allocation and enforce hardware memory limits." |
| **`KE`** *(Klang Editor)* | **Engine bloat**: Adding audio playback, recording, or turning the developer deck into a mini-DAW. | *Flag*: "Klang Editor is an offline declarative data, curve, and theme inspector. Keep it lean and decoupled from DAW runtimes." |

