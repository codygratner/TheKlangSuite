---
title: "The Sliver Engine Standalone VST & Sovereign Bass Envelope"
date: "2026-10-23"
staged: "2026-10-23"
priority: "P1"
pillars: ["TKS", "SYNTH_LAB", "DSP", "CODEX"]
tags: ["sliver-engine", "vst3", "au", "bass-envelope", "cody-design", "tabled", "dsp", "ui-ux"]
status: "tabled"
unlock_trigger: "Faceplate Studio v4.0 CAD Upgrades Complete"
resumption_command: "resume sliver engine"
---

# 🎛️ The Sliver Engine Standalone VST & Sovereign Bass Envelope

> [!NOTE] 📦 TABLED TASK DOSSIER (100% RECREATION & RESUMPTION SPECIFICATION)
> **Status**: Tabled under **Gate A: The Concrete External Predicate** (per `workflow_and_runway.md`).
> **Unlock Trigger**: Complete Klang Faceplate Studio v4.0 CAD Upgrades (Component Library, Undo/Redo, Container System).
> **Quick Resume**: Say `"resume sliver engine"` or execute the resumption flight path at the bottom of this document.

---

## 1. Executive Concept & Mission

**The Sliver Engine** is a dedicated, boutique standalone VST3/AudioUnit instrument designed to package an authentic hardware synth experience into a compact half-rack/Eurorack footprint (640px width).

### Core Features:
1. **Single Selectable Engine Slot**: Dual Saw Morph, Wavetable Scan, FM Quad Matrix, or Phase Distortion.
2. **Master 16-Control Sound Architecture**:
   - 4 Continuous Front Performance Encoders.
   - 4 Continuous Push-Action Performance Encoders.
   - 4 Rear Sound Lab DIP Switches (2-state toggles).
   - 4 Rear Sound Lab Screwdriver Trimpots.
3. **Master Amp & Envelope Subsystem**:
   - Anchored by **Cody's Coupled-Slope A/D(S)D/R Bass & Lead Envelope**.
4. **Hardware-Inspired Compact 6-Row Chassis**:
   - Row 1: Header, FFT Scope, Preset selector, Panic button.
   - Row 2: Front Performance Encoders.
   - Row 3: Front Push-Action Encoders.
   - Row 4: Rear Sound Lab DIP Switches.
   - Row 5: Rear Sound Lab Trimpots.
   - Row 6: Master Amp + Envelope Section.

---

## 2. Cody's Sovereign Bass & Lead Envelope Architecture

- **Inventor**: Cody (The Sovereign Architect, Oct 2026).
- **Invention**: **Coupled-Slope A/D(S)D/R Envelope** with Dynamic One-Shot/Sustain Gating and Curvature Morphing.

### Mathematical Formulation & Behavior
1. **Coupled Duration Budget**:
   - Total time budget $T_{\text{total}} \in [1\text{ms}, 10000\text{ms}]$, default $750\text{ms}$.
   - Bipolar Slope $S \in [0.01, 0.99]$, default $0.50$.
   - Dynamic apportionment:
     $$T_A = S \cdot T_{\text{total}}$$
     $$T_D = (1.0 - S) \cdot T_{\text{total}}$$
2. **Curvature Morphing**:
   - $S < 0.5$: Snappy / Staccato. Exponential attack into steep exponential decay.
   - $S = 0.5$: Linear symmetrical triangle ramp.
   - $S > 0.5$: Boomy / Swell. Slow logarithmic attack into elongated sustained decay.
3. **Dynamic One-Shot vs Sustain Gate**:
   - Sustain Level $L_{\text{sus}} \in [0.0, 1.0]$.
   - When $L_{\text{sus}} == 0.0$: Envelope operates in pure percussive **One-Shot** mode (ignores note-off, cascades directly from Decay into Release). Ideal for 808 sub kicks and percussion hits.
   - When $L_{\text{sus}} > 0.0$: Standard keyboard sustain mode. Holds at $L_{\text{sus}}$ while key is held; releases at $T_R$ on note-off.
4. **Integrated Portamento Glide**:
   - Exponential frequency glide filter with $T_{\text{glide}} \in [0.25\text{ms}, 250\text{ms}]$.

### C++ DSP Implementations & Documentation
- **C++ Header**: [`C:\Dev\TheKlangDSP\include\dsp\envelopes\N7BassEnvelope.h`](file:///c:/Dev/TheKlangDSP/include/dsp/envelopes/N7BassEnvelope.h)
  - Also aliased to [`M8BassEnvelope.h`](file:///c:/Dev/TheKlangDSP/include/dsp/envelopes/M8BassEnvelope.h).
  - Real-time audio thread safe: 0 heap allocations, 0 mutexes, denormal flushing enabled.
- **Modulator Suite**: Registered in [`C:\Dev\TheKlangDSP\include\dsp\modulators\N7Modulators.h`](file:///c:/Dev/TheKlangDSP/include/dsp/modulators/N7Modulators.h).
- **DSP Spec Document**: [`C:\_\Vault\New Klang City\Research\dsp\bassenvelope_dsp.md`](file:///c:/_/Vault/New%20Klang%20City/Research/dsp/bassenvelope_dsp.md) and [`C:\Dev\Research\dsp\bassenvelope_dsp.md`](file:///c:/Dev/Research/dsp/bassenvelope_dsp.md).
- **Rosetta Stone Entry**: [`C:\Dev\TheKlangDSP\docs\ROSETTA_STONE.md`](file:///c:/Dev/TheKlangDSP/docs/ROSETTA_STONE.md) Section D.
- **Inter-Project Data Bus Dispatch**: Handed off to TKS via `2026-10-23_cosmos_to_tks_sliver_engine_vst___sovereign_`.

---

## 3. Canon 640px Layout Specification

The immutable layout ground-truth is permanently stored in:
[`C:\Dev\TheKlangVault\Research\ui_ux\faceplate_project.json`](file:///C:/Dev/TheKlangVault/Research/ui_ux/faceplate_project.json)
(Mirrored to `C:\Dev\TheKlangSuite\faceplate_project.json` and `.agents/sidecar/faceplate_project.json`).

### Container Topography:
- **Chassis Dimensions**: $W = 640\text{px}, H = 600\text{px}$.
- **Container Structure**:
  - `sliver_row2` (Front Knobs): $x=0, y=16, w=640, h=120$
  - `sliver_row3` (Push Knobs): $x=0, y=140, w=640, h=120$
  - `sliver_row4` (DIP Switches): $x=0, y=264, w=640, h=96$
  - `sliver_row5` (Trimpots): $x=0, y=364, w=640, h=96$
  - `sliver_row6` (Amp+Env): $x=0, y=464, w=640, h=120$

### Full Parameter Canon (20 Controls):
| Row | Control ID | Label | Control Type | Range / Options | Default | Description |
|---|---|---|---|---|---|---|
| **Row 2** | `sliver_r2_c1` | K01 Morph | Knob 56px | 0.0 - 100.0% | 50.0% | Primary Timbre Morph |
| **Row 2** | `sliver_r2_c2` | K02 Cutoff | Knob 56px | 20.0 - 20000 Hz | 1200 Hz | Master VCF Cutoff |
| **Row 2** | `sliver_r2_c3` | K03 Reso | Knob 56px | 0.0 - 100.0% | 20.0% | VCF Resonance Peak |
| **Row 2** | `sliver_r2_c4` | K04 Drive | Knob 56px | 0.0 - 100.0% | 15.0% | Input Saturation Drive |
| **Row 3** | `sliver_r3_c1` | K05 Detune | Push Knob 48px | -50.0 - +50.0 ct | 0.0 ct | Push: Octave Jump |
| **Row 3** | `sliver_r3_c2` | K06 Spread | Push Knob 48px | 0.0 - 100.0% | 25.0% | Push: Mono Fold |
| **Row 3** | `sliver_r3_c3` | K07 Sub Lvl | Push Knob 48px | 0.0 - 100.0% | 50.0% | Push: Sub Octave Toggle |
| **Row 3** | `sliver_r3_c4` | K08 Noise | Push Knob 48px | 0.0 - 100.0% | 0.0% | Push: Noise Type Toggle |
| **Row 4** | `sliver_r4_c1` | DIP1 Warmth | Toggle Switch | Off / On | Off | Analog Drift Simulation |
| **Row 4** | `sliver_r4_c2` | DIP2 Pole | Toggle Switch | 2-Pole / 4-Pole | 4-Pole | 12dB vs 24dB Slope |
| **Row 4** | `sliver_r4_c3` | DIP3 Mono | Toggle Switch | Poly / Mono | Mono | Mono Legato Mode |
| **Row 4** | `sliver_r4_c4` | DIP4 Comp | Toggle Switch | Off / On | On | Output FET Limiter |
| **Row 5** | `sliver_r5_c1` | T01 Drift | Trimpot 28px | 0.0 - 100.0% | 10.0% | Pitch Instability Trim |
| **Row 5** | `sliver_r5_c2` | T02 Slop | Trimpot 28px | 0.0 - 100.0% | 5.0% | Filter Jitter Trim |
| **Row 5** | `sliver_r5_c3` | T03 Asym | Trimpot 28px | -50.0 - +50.0% | 0.0% | Diode Clipping Asymmetry |
| **Row 5** | `sliver_r5_c4` | T04 Noise | Trimpot 28px | -96.0 - -24.0 dB | -60.0 dB | Analog Noise Floor Level |
| **Row 6** | `sliver_r6_c1` | A01 Duration | Knob 56px | 1 - 10000 ms | 750 ms | Total Envelope Budget |
| **Row 6** | `sliver_r6_c2` | A02 Slope | Knob 56px | 1.0 - 99.0% | 50.0% | Attack vs Decay Apportionment |
| **Row 6** | `sliver_r6_c3` | A03 Sustain | Knob 56px | 0.0 - 100.0% | 0.0% (One-Shot) | Sustain Level (0 = Percussive) |
| **Row 6** | `sliver_r6_c4` | A04 Release | Knob 56px | 5 - 5000 ms | 250 ms | Note-Off Release Time |

---

## 4. Live Runtime & Serving Infrastructure

1. **Headless Tray Server**:
   - Managed automatically by `nkai_tray.py` + `pythonw.exe` on **Port 3030**.
   - Zero console or terminal popups on Windows 11.
2. **Active Endpoints**:
   - **Standalone Web Audio Synthesizer**: `http://localhost:3030/sliver` ([`sliver_engine_panel.html`](file:///c:/Dev/TheKlangSuite/sliver_engine_panel.html))
   - **CAD Faceplate Studio**: `http://localhost:3030/studio` ([`klang_faceplate_studio.html`](file:///c:/Dev/TheKlangSuite/klang_faceplate_studio.html))
     - Injected with `CANON_SLIVER_PROJECT` JSON; clicking `⚡` instantly recalls this layout.

---

## 5. Resumption Flight Path (How to Resume 100%)

When the user triggers `"resume sliver engine"`:

1. **Verify Files on Disk**:
   - `C:\Dev\TheKlangDSP\include\dsp\envelopes\N7BassEnvelope.h`
   - `C:\Dev\TheKlangVault\Research\ui_ux\faceplate_project.json`
2. **Execute Phase 1 (JUCE Parameter Tree)**:
   - In `TheKlangSuite/source/`, generate `SliverParameters.h` wrapping `AudioProcessorValueTreeState::createParameterLayout()` with all 20 IDs above.
3. **Execute Phase 2 (JUCE Editor Component Layout)**:
   - Generate `SliverEditor.h` and `SliverEditor.cpp` referencing the exact coordinates ($x, y, w, h$) from `faceplate_project.json`.
4. **Execute Phase 3 (Voice DSP Wiring)**:
   - Hook `toad::N7BassEnvelope` into the synth voice buffer processing block in `TheKlangDSP`.
