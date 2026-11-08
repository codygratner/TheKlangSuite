# SPEC-TKS-064: Universal Engine & Wrapper Architecture (TSE / TT / The Relic)

- **Status:** Ratified Architectural Specification (`LOOP-33`)
- **Domain:** DSP Voice Topologies, Presets & Instrument Formats
- **Authors:** Cody Gratner & Antigravity (Klang Industries)
- **Date:** 2026-10-27

---

## 1. Architectural Summary & Scope
This specification unifies the synthesizer voice models and preset schemas across **The Sliver Engine** (`TSE`), **ToadTracker** (`TT`), and **The Relic** (`TKR`).

### 1.1 Decoupled Voice Model
Synthesizer voices are split cleanly into two orthogonal layers:
1. **Core Sound Engine (`tkdsp::EngineBase<Capacity>`)**:
   - Generates raw polyphonic or monophonic timbre.
   - Contains zero integrated filtering, volume envelopes, or voice allocation.
   - Constrained to one of 4 standardized control sizes ($8, 16, 24, 32$).
2. **Universal Synth Wrapper (`tkdsp::SynthWrapper<EngineT>`)**:
   - Provides playable performance context (Drum one-shot, Mono legato/glide, Polyphonic).
   - Multi-mode SVF / Ladder filter with keytrack and dedicated envelope.
   - Triple envelope generator (Pitch/Mod, Filter, Amp).
   - Machinedrum Lo-Fi suite (SRR/BRR) + Sequential Pro 2 4-grit distortion suite.

---

## 2. The 4 Engine Sizing Standards

All engine parameters are grouped in banks of 8 controls:

```
Tier 1: Short   (8 controls)  -> [Bank 0: Main (8)]
Tier 2: Tall    (16 controls) -> [Bank 0: Main (8)] [Bank 1: Timbre (8)]
Tier 3: Grande  (24 controls) -> [Bank 0: Main (8)] [Bank 1: Timbre (8)] [Bank 2: Modulation (8)]
Tier 4: Venti   (32 controls) -> [Bank 0: Main (8)] [Bank 1: Timbre (8)] [Bank 2: Modulation (8)] [Bank 3: Chaos/DIP (8)]
```

### 2.1 Short (8 Controls)
- **Primary Domain**: Clean-room Machinedrum (`MD`), Monomachine (`MnM`), chiptune chips, simple 2-op FM, acoustic transient clicks.
- **Benefits**: Zero dummy controls, instantaneous muscle memory, low CPU.

### 2.2 Tall (16 Controls)
- **Primary Domain**: Flagship TKDSP standard engines (`AlphaKraft`, `CosmoPhase`, `TheR1`, `Pluck`, `ArgentSlayer`).
- **Topology**: 2×8 banks matching the established TKDSP 16-control canon.

### 2.3 Grande (24 Controls)
- **Primary Domain**: Deep sound-design, 4-operator FM with matrix feedback, physical modeling waveguides with sympathetic strings (Mutable Rings).
- **Topology**: 3×8 banks.

### 2.4 Venti (32 Controls / 0x20 Hex)
- **Primary Domain**: Absurdist flagships (HyperStrike 4D Hypercube, 56K Vocoder with EMS pin matrix, multi-engine spatial resonators).
- **Topology**: 4×8 quadrant grid.

---

## 3. Universal Synth Voice Wrappers

### 3.1 Parameter Topology (Compact 16 vs Extended 32)

#### Compact Wrapper (16 Controls — Optimized for Handheld Tracker Screens)
- **Bank 1 (Filter & Grit — 8 Controls)**:
  1. `CUTOFF`: Filter cutoff frequency (20 Hz – 20 kHz).
  2. `RESONANCE`: Filter resonance / Q (0.0 – 1.0, self-oscillation capable).
  3. `TYPE`: Filter model (SVF LP/BP/HP/Notch, 4-Pole Ladder).
  4. `DRIVE`: Pre-filter saturation.
  5. `SRR`: Machinedrum Sample Rate Reduction (48 kHz $\to$ 500 Hz).
  6. `BRR`: Machinedrum Bit Rate Reduction (16-bit $\to$ 2-bit).
  7. `GRIT`: Sequential Pro 2 Character Macro (Drive/Hack blend).
  8. `KEYTRACK`: Filter keyboard pitch tracking (0% – 100%).
- **Bank 2 (Envelopes & Voice — 8 Controls)**:
  1. `AMP_ATTACK`: Amp attack time (0.1 ms – 10 s).
  2. `AMP_DECAY`: Amp decay / release time.
  3. `AMP_SUSTAIN`: Amp sustain level (0.0 – 1.0).
  4. `FILT_ENV`: Filter envelope depth (-1.0 to +1.0 bipolar).
  5. `FILT_DECAY`: Filter envelope decay time.
  6. `PITCH_ENV`: Pitch envelope depth (-48 to +48 semitones).
  7. `PITCH_DECAY`: Pitch envelope decay time.
  8. `GLIDE_CHOKE`: Portamento glide rate (Mono) or Choke Group index (Drum).

#### Extended Wrapper (32 Controls — Desktop VST & CAD Faceplates)
- Adds dedicated 4-grit controls (`Drive`, `Hack`, `Decimation`, `Girth/Air`), full ADSR for all 3 envelopes, and stereo spread/unison detune.

---

## 4. The Relic Ingestion & Cross-Tier Preset Architecture

### 4.1 Canonical Instrument Formats in ToadTracker
- `TT-Sliver`: Native single-engine or triple-engine instrument format.
- `TT-Relic`: 4-part multi-timbral instrument format (4 slots $\times$ 3 engines).

### 4.2 Ingestion & Translation Rules
- **Downgrade (Relic $\to$ Sliver)**:
  - Preserves Slot 1 engine and wrapper parameters.
  - Automatically folds active sub-engines 2 and 3 into the Sliver 3x mixer if populated.
  - Safely drops slots 2, 3, and 4 with zero memory corruption or hanging pointers.
- **Upgrade (Sliver $\to$ Relic)**:
  - Instantiates a fresh `TT-Relic` instance.
  - Assigns `TT-Sliver` into Slot 1 with 1:1 parameter parity.
  - Leaves Slots 2, 3, and 4 initialized to neutral default states for multi-timbral expansion.

---

## 5. Architectural Invariants & Guardrails

1. **The Handheld 2-Card Navigation Ceiling**:
   - On 320×240 screens (ToadTracker / M8), instruments are strictly partitioned into **Card 1 (Engine)** and **Card 2 (Wrapper)**.
   - Paging depth never exceeds 4 banks for the engine and 2 banks for the compact wrapper.
2. **The 3-Engine Complexity Cap**:
   - In `Sliver 3-Engine Mode`, sub-engines are restricted to **Short (8)** or **Tall (16)**. Stacking multiple Venti engines in a single Sliver track is prohibited to prevent parameter explosion.
