# Master Plan: Milestone v0.4.0 "The Interface & Experience Update"

> **Milestone:** v0.4.0  
> **Tracking:** [Backlog](../BACKLOG.md)
> **Theme:** Complete Clean-Slate UX Overhaul & Modern Neo-Slate Vector UI  
> **Ethos:** *"UX IS KING"* (Zero Prototype Debt, Pure Workflow Elegance)  
> **Status:** QUEUED (Scheduled immediately following v0.3.2 release)  

---

## 1. Executive Summary & Design Philosophy

Up through v0.3.2, The Klang Suite maintained strict 1:1 parity with the legacy v0.2.0 monolithic prototype. While this preserved functional continuity while establishing tests and data-driven modularity, it forced the user interface to carry forward prototype design debt: sprawling 204-parameter lists, an awkward 8-card grid where navigation and visualizers consumed 25% of the playing surface, and visual clutter.

With **Milestone v0.4.0**, we execute a **Total Clean-Slate Parity Break**:
1. **The "Anti-Tacky" Modern Hybrid Aesthetic**:
   - Rejects faux-vintage skeuomorphism (zero fake 3D knobs, zero faux-aluminum glare, zero fake drop shadows, zero fake rack screws).
   - **Renoise**: High-density data precision, mathematical clarity, and zero wasted margin space.
   - **Ableton Live**: Distraction-free flat geometry, unified clean rectangular containers, and Tabular Figures (fixed-width digits preventing numeric text jitter during automation).
   - **Vital**: Luminous animated vector curves, real-time envelope playhead tracking dots, and clean futuristic contrast.
   - **Kilohearts**: Modular card containers, horizontal Meter Sliders with embedded labels and values, and immediate tactile drag resolution.
2. **The 4-Controls-Per-Card Mandate**: Every Card front panel is strictly limited to **exactly 4 high-impact performance controls**, organized as a vertical 1x4 stack of horizontal Meter Sliders.
3. **Hardware-Informed Synergy (dadamachines TBD-16)**: While software-first, the 4-control limit creates immediate 1:1 synergy with 1x4 encoder controllers, matching the physical layout of the TBD-16.
4. **Header-Integrated Navigation & Scope**: Page navigation tabs (`[VOICE 1]`, `[VOICE 2]`, `[TRANSIENTS]`, `[FX 1-4]`, `[FX 5-8]`, `[MOD]`) and the master stereo oscilloscope/meters move to the top Chassis Header, freeing up 100% of the rack canvas for synth modules.
5. **Right-Click Callout Deep Dive**: Advanced and secondary parameters (fine-tuning, tracking modes, snap points, velocity curves) are tucked into sleek popover CalloutBoxes, keeping the primary performance surface distraction-free.

---

## 2. Core Architecture & UI Layout

```
┌────────────────────────────────────────────────────────────────────────────────────────┐
│  THE KLANG FARMER   [VOICE 1] [VOICE 2] [NOISE] [FX 1-4] [FX 5-8] [MOD]  [===SCOPE===] │  <-- CHASSIS HEADER
├────────────────────────────────────────────────────────────────────────────────────────┤
│ ┌── CARRIER 1 ─────┐ ┌── MODULATOR 1 ──┐ ┌── PITCH ENV 1 ──┐ ┌── FILTER 1 ─────┐       │
│ │ [Shape    ~~~ 0] │ │ [Speed  1.00x]  │ │ [Slope   \ 0.5] │ │ [Type       LPF]│       │
│ │ [Pitch  +12.0st] │ │ [Depth   45%]   │ │ [Depth    +24st]│ │ [Slope    24 dB]│       │
│ │ [Detune +0.10st] │ │ [Track   NOTE]  │ │ [Decay   140ms] │ │ [Cutoff 2.4 kHz]│       │
│ │ [Level   0.0 dB] │ │ [Shape   /\/\]  │ │ [Curve     EXP] │ │ [Resonance  35%]│       │
│ └──────────────────┘ └─────────────────┘ └─────────────────┘ └─────────────────┘       │
│ ┌── AMP ENV 1 ─────┐ ┌── VELOCITY 1 ───┐ ┌── MIXER ────────┐ ┌── OUTPUT/PANIC ─┐       │
│ │ [Attack     2ms] │ │ [Vol Sens 80%]  │ │ [Voice 1  0 dB] │ │ [Master  0.0 dB]│       │
│ │ [Decay    280ms] │ │ [Pitch Sens 0%] │ │ [Voice 2 -6 dB] │ │ [Pan     Center]│       │
│ │ [Sustain    0%]  │ │ [Filter Sens]   │ │ [Noise  -12 dB] │ │ [Limiter   +3dB]│       │
│ │ [Drive    +4 dB] │ │ [Floor    10%]  │ │ [RingMod    0%] │ │ [PANIC!   FLASH]│       │
│ └──────────────────┘ └─────────────────┘ └─────────────────┘ └─────────────────┘       │
├────────────────────────────────────────────────────────────────────────────────────────┤
│ Carrier 1: Shape  Sine -> Saw [0.42]  |  Right-Click: Snap Points  |  Double-Click: 0.0│  <-- STATUS BAR
└────────────────────────────────────────────────────────────────────────────────────────┘
```

---

## 3. Phased Implementation Roadmap

### Phase 1: Curated 4-Control Data Schema & Parameter Pruning
- Author new curated parameter definitions in `assets/controls/*.json`.
- Group each module into exactly 4 primary parameters and define secondary callout parameters.
- Update `assets/layouts/tkf_layout.json` and `tkp_layout.json` to reflect the clean 4-control cards.
- Update `assets/text/strings.json` with polished, descriptive copy for the curated set.

### Phase 2: Neo-Slate Chassis Header & Integrated Oscilloscope
- Evict the legacy Navigation Card and Visualizer Card from `FarmerEditor`.
- Build the modern horizontal Header Tab Bar with active page indicators.
- Port and adapt `PlanterHeaderVisualizer` into a universal `KlangHeaderVisualizer` featuring real-time stereo waveform scope, dual peak VU meters, and Limiter trigger badge.
- Expand the main canvas layout from 6 modular slots to 8 full-width sound-sculpting card slots.

### Phase 3: Vertical 1x4 Meter Slider Stacks & Real-Time Diagrams
- Refactor `ModuleCardComponent` to render 4 horizontal Meter Sliders in a clean vertical stack.
- Integrate real-time vector curve drawing (`DiagramType::Waveform`, `DiagramType::EnvelopeSlope`, `DiagramType::FilterSlope`) inside slider troughs.
- Tabular figures (fixed-width digits) to prevent numeric text jitter during automation.

### Phase 4: Right-Click Callout Deep-Dive Engine
- Standardize right-click listeners across all module cards and sliders.
- Implement floating `juce::CallOutBox` dialogs exposing secondary parameters:
  - Pitch & Harmonic Snap Point detent grids.
  - S&H clock jitter and frequency tracking modes.
  - Velocity mapping curves and bipolar modulation offsets.

### Phase 5: Switchable Studio Theme Engine (`Cykranosh`, `Nord`, `Dracula`, `Cyberpunk`)
- Structure `assets/themes/theme.json` to define multiple curated studio palettes:
  - **`cykranosh` (Default / Creator's Signature)**: Deep slate navy (`#161B22` / `#1A202C`), muted deep blue cards, eerie ghostly teal (`#4EBEB1`), arctic ice blue, and starlight silver indicators (engineered for zero eye fatigue during marathon sessions).
  - **`nord`**: Arctic frost blue-grey (`#2E3440`) with icy cyan and pastel aurora highlights.
  - **`dracula`**: Iconic vampire purple (`#282A36`) with electric cyan, hot pink, and lime green accents.
  - **`cyberpunk`**: Deep obsidian black (`#090C12`) with glowing electric cyan and hot magenta.
- Wire live theme switcher dropdown into both the Settings & About modal and The Klang Editor.
- Instant, non-destructive theme swapping persisted in user properties without restarting DAW.

### Phase 6: The Klang Editor Interactive Curve & Snap Calibration Workspace
- Update The Klang Editor's property panel and preview canvas for the new 4-control card architecture.
- Integrate live interactive slider skew tuning and snap point testing.

### Phase 7: Automated Test Suite Overhaul & Validation
- Update `test/PluginIntensiveTestSuite.h`, `test/ParameterSchemaAuditTest.h`, and `test/EditorTestSuite.h` to validate the new curated parameter reflection, 4-control slot limits, header navigation, and theme palette switching.
- 100% assertion pass across `dsp_tests.exe` and `gui_tests.exe`.
- Compile Release binaries and deploy via `deploy.ps1`.

---

## 4. Finalized Architectural Additions (Session Consensus)

1. **Agrarian Modulation Taxonomy (The Sower / The Irrigator)**:
   - To honor Renoise while avoiding trademark collision and leaning into our agrarian IDM aesthetic, the 1-to-many meta-routing hubs (`Hydra 1..4`) are christened **`[SOWER 1..4]`** (or **`[IRRIGATOR 1..4]`**).
   - *Metaphor*: Drawing from 1 control or audio source and spraying/broadcasting modulation across up to 8 field rows (destinations) with individual depth, min/max bounds, and curves.
2. **The 3-Tier Vertical Layout**:
   - **Chassis Header**: Horizontal nav tabs, Master Oscilloscope/VU (double-click Panic flush), 4 Performance Macros (`M1..M4`), Undo/Redo (`↶`/`↷`), A/B comparison, and Global `[d6]`.
   - **Center Workspace**: Tabbed card pages (`[VOICE 1]`, `[VOICE 2]`, `[TRANSIENTS]`, `[EFFECTS]`, `[MOD]`).
   - **Persistent Lower Modulator Strip**: Permanently visible bottom lane containing animated "drag-from" tiles for all LFOs, Envelopes, and Sowers. Dragging a tile directly maps modulation onto any center workspace knob.
   - **Permanent Status Bar**: Un-toggleable bottom line displaying rich contextual tooltips and mouse shortcuts.
3. **Visual Modulation Tracer Cables (Eurorack Edge Sockets)**:
   - Renders glowing anti-aliased Bézier splines from driving source tiles up to modulated target knobs.
   - **Card Inspector Popover Docking**: In the expanded 4-knob inspector popover, cables terminate cleanly into 4 colored jack sockets on the bottom perimeter bezel of the window, preventing text/slider occlusion while preserving immediate visual patch awareness.
4. **Drag-and-Drop FX Rack with Console Anchors**:
   - Single unified `EFFECTS` page with two 5-slot horizontal lanes (Pre-Amp Top, Post-Amp Bottom).
   - Draggable slots 1–4 with full intra-lane, cross-lane, and empty-slot swapping.
   - Immutable Anchor 5 cards: `[PRE-AMP DRIVE & COLOR]` and `[MASTER LIMITER / OUT]`.
   - Optional Mick Gordon parallel routing split mode and **DOOM** brickwall saturation profile.
5. **Compact Modulator Cards on the `[MOD]` Page (2x4 Grid)**:
   - Replaces sprawling vertical lists with a tight 2x4 grid displaying 8 modulators simultaneously without scrolling.
   - Half-height compact cards feature a miniature animated vector waveform trough + 2 essential performance knobs (`Rate`/`Speed` and `Depth`), with right-click popovers exposing curves, clock sync, and destination tables.
   - **The Cultivator Block (Meta-Modulator)**: A special nested modulator type (`[TKC]`) that "eats" two adjacent horizontal slots in the grid (creating a double-width card).
     - Retains the vector waveform display but expands the front panel to house **8 Macro Knobs** (`M1`..`M8`).
     - Clicking the gear icon opens a full TKC interface popover for routing internal LFOs/Envelopes to those 8 Macros, achieving deep "modulations-inside-modulations".
     - **Architectural Guardrails (Macro Firewall)**: To prevent infinite APVTS parameter recursion, the DAW is only exposed to the 8 Macro Knobs. All internal routing inside the TKC popover is serialized privately to JSON. Max nesting depth is hardcoded to 1 (you cannot load a TKC block inside another TKC block).
6. **Modular 3-Slot Transient Architecture (`[TRANSIENTS]`)**:
   - Replaces the static noise page with 3 swappable slots: each slot independently selects `[Analog Noise Generator]`, `[Sample One-Shot Player]`, or `[Synthetic Impulse Click]`.
   - Features a dedicated Transient Sub-Mixer with individual Level, Pan, and Filter before routing to the main synth mixer.
7. **Maths-Style Dual Slope Function Generators (`[SLOPE 1..4]`)**:
   - Inspired by the legendary Make Noise Maths / Serge DUSG, introduces 4 dedicated looping slope generators.
   - Front panel: `[Rise]`, `[Fall]`, continuous `[Curve]` morph (Log <-> Lin <-> Exp), and `[Cycle]` (Looping LFO / VCO).
   - Generative Trigger Pulses: Emits discrete single-sample End-of-Fall (EOF) / End-of-Rise (EOR) trigger pulses upon cycle completion to re-trigger other Slopes or fire drum voices for cascading rhythms, polyrhythmic bursts, and ratchets.

