# The Klang Suite: Product Lineup & Core Intent Matrix

The Klang Suite is an open-source, data-driven audio synthesis and sound design ecosystem developed in modern C++20 and JUCE 9. This document formalizes the official **Product Matrix, Acronym Taxonomy, and Core Intent** for every instrument, editor, and hardware offering in the suite.

---

## 🏛️ Acronym Taxonomy & Collision Safeguards

To prevent naming collisions across C++ header guards, logging macros (`TKS_LOG`), Git repositories, and documentation, the following three-letter acronyms are permanently reserved:

| Acronym | Product Name | Category | Primary Form Factor |
| :--- | :--- | :--- | :--- |
| **TKS** | **The Klang Suite** | Master Ecosystem / Framework | The entire software, DSP, and tooling umbrella |
| **TKF** | **The Klang Farmer** | Flagship Instrument | VST3 / AU / Standalone Audio Plugin |
| **TKP** | **The Klang Planter** | Performance Synthesizer | VST3 / AU / Standalone Audio Plugin |
| **TKE** | **The Klang Editor** | Developer & Modder Deck | Standalone JUCE Application |
| **TKM** | **The Klang Mill** | Multi-Effects Playground | VST3 / AU Audio Plugin |
| **TKR** | **The Klang R1 (TKR-1)** | 7-Voice Rhythm Synthesizer | VST3 / AU / Standalone Audio Plugin |
| **TKB** | **The Klang Box** | Embedded Hardware Platform | Dedicated Microcontroller Desktop / Eurorack Hardware |

> [!IMPORTANT]
> **TKS vs. TKB Convention**:  
> `TKS` is reserved exclusively for **The Klang Suite** (framework code, `TKS_LOG`, system architecture).  
> All embedded microcontroller hardware offerings use **`TKB`** (**The Klang Box**), with hardware variant designations (e.g. `TKB-Daisy`, `TKB-8`).

---

## 🎹 The Core Instrument & Tool Lineup

```
 ┌────────────────────────────────────────────────────────────────────────┐
 │                      THE KLANG SUITE (TKS)                             │
 ├────────────────────────┬───────────────────────┬───────────────────────┤
 │  FLAGSHIP SYNTHESIS    │  PERFORMANCE & SPEED  │  EFFECTS & RACK       │
 │  TKF: The Klang Farmer │  TKP: The Klang       │  TKM: The Klang Mill  │
 │  (Modular Sound Lab)   │       Planter         │  (1x6 Pedalboard)     │
 │                        │  (BIA Drums & Bass)   │                       │
 ├────────────────────────┼───────────────────────┼───────────────────────┤
 │  RHYTHM & GROOVE       │  DEVELOPER DECK       │  PHYSICAL HARDWARE    │
 │  TKR: The Klang R1     │  TKE: The Klang       │  TKB: The Klang Box   │
 │  (7-Voice ER-1 Homage) │       Editor          │  (Embedded Seed,      │
 │                        │  (Data & Theme Deck)  │   Daisy, Teensy 8-Out)│
 └────────────────────────┴───────────────────────┴───────────────────────┘
```

---

### 1. The Klang Farmer (TKF)
**"The Flagship Percussion Synthesizer"**

- **Core Intent**: The deep, uncompromising sound design laboratory. Designed for complex, modular percussive sound sculpting, layered FM synthesis, metallic transients, and extended acoustic modeling.
- **Form Factor**: VST3 / AU / Standalone.
- **Architecture**:
  - Full 8-card modular rack surface (800x600 to 4K scalable).
  - Dual Carrier / Modulator FM operator architecture with morphable waveforms.
  - Multi-mode resonant filter with dedicated sample & hold drive.
  - Dedicated Transient Shaper with independent pitch/noise envelope decay curves.
  - Dynamic 8-slot multi-instance DSP effect rack with serial/parallel routing.
  - Full chromatic MIDI tracking, microtonal tuning, and polyphonic voice allocation.
  - **The Klang-Brain Modulation Engine**: 4-mode algorithmic modulation generator (Fugue counterpoint, Matriceal polymeter, Metropolix/M8 accumulators, Turing LFSR) operating in Continuous DAW sync and DSI Pro 2 Triggered Key-Advance modes.

---

### 2. The Klang Planter (TKP)
**"The Lean & Mean Drum and Bass Synth"**

- **Core Intent**: A punchy, fast, highly expressive synthesizer inspired by the immediacy of Eurorack modules like the Noise Engineering *Basimilus Iteritas Alter* (BIA)—but built with chromatic polyphony, DAW automation, and full **Note-Off** ADSR release.
- **Form Factor**: VST3 / AU / Standalone.
- **Architecture**:
  - Curated 8-module surface: Carrier, Modulator, Harmonic Wavefolder, Spread, Noise/Transient, SVF Filter, Envelope Shaper, and Master Limiter.
  - Dual operating personas: **Percussion Mode** (rapid transient strikes and metallic decay) and **Bass Mode** (deep FM sub-bass, glide/portamento, dual-oscillator hard sync).
  - Integrated high-performance header visualizer (30 Hz smoothed oscilloscope with peak VU meters).
  - Popover Callout Mini-Cards for deep-dive settings without cluttering the main surface.

---

### 3. The Klang Editor (TKE)
**"The Standalone Data, Theme & Curve Inspector"**

- **Core Intent**: The developer, modder, and sound designer control deck. Decouples parameter calibration, JSON schema authoring, and UI theme design from DAW testing sessions.
- **Form Factor**: Standalone JUCE Desktop Utility.
- **Architecture**:
  - Unified Master Tree View with multi-state layer filters (`[Controls]`, `[Layout]`, `[Theme]`).
  - Interactive parameter calibration workspace: test skew factors, snap points, and tactile responses live before saving.
  - Real-time LookAndFeel and color theme editor with live preview across Farmer and Planter chassis.
  - Two-way pre-indexed Cross-Reference ("Where Used") inspection map.
  - One-click snapshot generation and test artifact auditing.

---

### 4. The Klang Mill (TKM)
**"The Industrial Multi-Effects Playground"**

- **Core Intent**: A standalone multi-effects rack inspired by classic analog studio rackmounts and boutique stompbox pedalboards (e.g. *Soundtoys Effect Rack*). Designed to dirty, smear, widen, and mangle any incoming audio track in seconds.
- **Form Factor**: VST3 / AU Audio Plugin.
- **Architecture**:
  - Tactile 1x6 horizontal chassis: Input Stage $\to$ 4 Serial Multi-FX Slots $\to$ Master Limiter & Output.
  - Access to the full 26-algorithm DSP effects catalog (Phase Smear, Tape Saturation, Resonant Comb, Spectral Reverb, Granular Glitch).
  - **Global Slop Control**: Injects organic, non-linear analog component drift and micro-fluctuations across all 4 active pedals simultaneously.
  - Instant one-click stomp bypass toggles on every pedal slot with zero routing matrix friction.

---

### 5. The Klang R1 (TKR-1)
**"The 7-Voice Rhythm Synthesizer"**

- **Core Intent**: A fast-paced, tactile, zero-tab drum machine paying tribute to the iconic *Korg Electribe ER-1*. Engineered for rapid-fire beat-making, intuitive live finger drumming, and aggressive synthetic rhythm grooves.
- **Form Factor**: VST3 / AU / Standalone.
- **Architecture**:
  - 7 dedicated voice strips displayed simultaneously with zero tab switching:
    - **Voices 1–4**: Pure FM Synth (Kicks, resonant sub-bass, punchy snares, toms, laser zaps).
    - **Voices 5–7**: Percussion & Metallic (Closed Hat, Open Hat with auto-choke, Cymbal/Crash).
  - **Octave-Invariant White Key Triggering**: White keys in any octave map to Voices 1–7 (`C` through `B`), keeping pitches fixed to front-panel knobs for foolproof live finger drumming.
  - **Ring Modulator**: Audio-rate cross-modulation between Voice 1 and Voice 2 for metallic industrial clangs.
  - **DAW Multi-Out Bus Architecture**: 8 stereo output pairs (Master Mix + 7 individual voice stems) with auto-mute routing when routed to aux channels.
  - **The Klang-Brain Rhythm Engine**: Integrated 4-mode generative sequencer (Fugue, Matriceal, Metropolix/M8, Turing LFSR) generating procedural rhythms with Continuous DAW sync and DSI Pro 2 Triggered Key-Advance modes.

---

### 6. The Klang Box (TKB)
**"The Embedded Hardware Synthesizer Platform"**

- **Core Intent**: The physical embodiment of The Klang Suite's pure C++ DSP engine running on dedicated microcontrollers, bringing zero-allocation real-time sound synthesis into standalone hardware grooveboxes and Eurorack setups.
- **Form Factor**: Physical Hardware Desktop Units & Eurorack Modules.
- **Hardware Editions**:
  - **TKB-Daisy (Electro-Smith Daisy Seed)**: STM32H750 ARM Cortex-M7 @ 480 MHz with 64 MB SDRAM, stereo 24-bit 96 kHz codec, 4 endless encoders, and OLED display. Available as a compact stereo tabletop unit and Eurorack module.
  - **TKB-8 (Studio Edition — PJRC Teensy 4.1)**: Flagship studio drum synthesizer powered by an NXP i.MX RT1062 Cortex-M7 @ 600 MHz and a Cirrus Logic CS42448 multi-channel codec. Features **8 discrete 1/4" analog outputs** with switched normalled jacks for routing individual voices to outboard analog gear.
  - **TKB-TBD (dadamachines tbd-16 Integration)**: Direct deployment onto the open-source dual-core ESP32-P4 RISC-V + RP2350 groovebox, mapping 4-knob cards 1:1 onto the unit's 4 physical push-encoders.
  - **Zynthian V5 Port**: Headless Linux LV2/CLAP integration on Raspberry Pi 5 with 1:1 optical encoder mapping.

---

## 🛠️ Foundational Ecosystem & Infrastructure

Behind the musician-facing instruments lies the architectural infrastructure that powers the suite:

1. **The Klang Boilerplate**:
   - Modern C++20 / JUCE 9 open-source plugin starter template repository.
   - Packages the zero-allocation DSP core, `TbdAudio::FastMath`, JSON-driven APVTS parameter schemas, and the headless `gui_tests` reflection suite for the open-source audio developer community.
2. **The Klang Vault**:
   - Private Obsidian Second Brain and Asymmetric Sync Bridge (`tools/sync_obsidian_vault.ps1`).
   - Enables conflict-free mobile note capture via Obsidian Sync, offline documentation reading, and live build/error telemetry reporting.

---

## 🚨 Product Identity Drift Guardrail (The "Stay in Your Lane" Policy)

To preserve the focus and punch of each offering, **New Klang City (Planning Headquarters)** is officially chartered to monitor incoming ideas, `/plan` drafts, and `/grill-me` interviews for **scope drift**.

Whenever a proposed feature threatens to blur a product's core identity, the agent will flag it:

| Product | The Core Trap (Scope Drift) | The Guardrail Response |
| :--- | :--- | :--- |
| **TKF** *(Farmer)* | **Dumbing it down**: Stripping out modular modulation, operator routing, or parameter depth to make it "easy". | *Flag*: "Farmer is the flagship sound-design laboratory. Keep the depth; if you want rapid sweet-spots, route this idea to Planter!" |
| **TKP** *(Planter)* | **Feature creep & menu bloat**: Adding multi-page tabs, deep sub-menus, or multi-slot FX racks. | *Flag*: "Planter is the lean & mean BIA machine. Keep it single-surface and immediate. If it needs 8 pages, it belongs in Farmer!" |
| **TKM** *(Mill)* | **Identity confusion**: Adding synth oscillators, MIDI note playback, or complex modulation matrices. | *Flag*: "Mill is a pure multi-effects rack (Soundtoys style). It processes external audio; it does not generate notes." |
| **TKR** *(R1)* | **Tab & complexity clutter**: Adding tabs, sample layering, or micro-editing menus that break live finger drumming. | *Flag*: "R1 is an ER-1 tribute console. All 7 voice strips must stay on one screen with octave-invariant white-key triggers." |
| **TKB** *(The Klang Box)* | **Embedded unfriendliness**: Allocating heap memory, exceeding microcontroller SRAM/Flash budgets, or confusing acronyms (`TKS`). | *Flag*: "TKB is dedicated bare-metal hardware. Keep DSP zero-allocation, enforce hardware budgets, and protect the `TKB` designation." |
| **TKE** *(Editor)* | **Engine bloat**: Adding audio playback, recording, or turning the developer deck into a mini-DAW. | *Flag*: "The Klang Editor is an offline declarative data, curve, and theme inspector. Keep it lean and decoupled from DAW runtimes." |

