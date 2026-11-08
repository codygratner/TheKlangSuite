# 🧠 Autoplan 3.0: Nyarlat Additive Oscillator Bay Overhaul
### *Tactile 2×2 Vector Icon Grid, Squarer Bitmask OLED Scope & Zero-Detune Architecture*

> **Status:** `SPECIFICATION_RATIFIED (AUTOPLAN_ACTIVE — LOOP TURN 2)`  
> **Domain:** `TRACK: HYBRID` (UI/UX Geometry & Iconography + Parameter Contract & Voice DSP Retirement)  
> **Tier:** `STANDARD` (Flash Tier 2 Orchestration × Claude Sonnet Co-Design)  
> **Zero-Build Airgap:** 🔒 **Locked** (Zero files modified; zero compilation until approved)  

---

## 🏛️ Ratified 2×2 Grid Layout & Geometry

Per your directive, the 2×2 grid layout is locked into the following dipole architecture:

```
┌────────────────────────────────────────────────────────────────────────┐
│                        OSCILLATOR POD ARCHITECTURE                     │
├────────────────────────────────────────────────────────────────────────┤
│ ROW 1: Bank Stepper Audition Bar [ < ] [ Bank Name v ] [ > ]  (332x20) │
├────────────────────────────────────────────────────────────────────────┤
│ ROW 2: Full-Width Octave Radio [-2] [-1] [ 0] [+1] [+2]       (332x20) │
├───────────────────────────────────────────────────────┬────────────────┤
│ ROW 3: RECESSED BITMASK OLED DISPLAY                  │ 2x2 ICON GRID  │
│        (260 x 64 px — 256px inner scope)              │ (64 x 64 px)   │
│                                                       │ ┌─────┬─────┐  │
│        Dynamic 64-partial waveform rendering          │ │ 🔒  │ /\  │  │
│        Cyan stator sweep cursor + zero-crossing line  │ ├─────┼─────┤  │
│                                                       │ │ /|  │ |\  │  │
│                                                       │ └─────┴─────┘  │
├───────────────────────────────────────────────────────┴────────────────┤
│ ROW 5a: [MORPH] (48px)        [ODD/EV] (48px)        [TILT] (48px)     │
├────────────────────────────────────────────────────────────────────────┤
│ ROW 5b: [FORM]  (48px)        [MASK]   (48px)        [SPEED](48px)     │
└────────────────────────────────────────────────────────────────────────┘
```

### 🧭 2×2 Grid Mapping ($30 \times 30\text{px}$ cells, $4\text{px}$ seam)

| Cell Coordinates | Control | APVTS Target | Icon & Visual Logic |
| :--- | :--- | :--- | :--- |
| **Top-Left (0, 0)** | `Speed Lock` | `oscX_speed_lock` (bool) | **Padlock:** Gold `#FFD700` closed shackle when locked; dimmed gray `#8B949E` open shackle when unlocked |
| **Top-Right (1, 0)** | `Triangle` | `oscX_loop` (choice index 1) | **Triangle Wave:** Symmetrical ascending & descending peak, filled accent when active |
| **Bottom-Left (0, 1)** | `Saw Up` | `oscX_loop` (choice index 0) | **Saw Up Wave:** Ascending ramp from bottom-left to top-right, filled accent when active |
| **Bottom-Right (1, 1)** | `Saw Down` | `oscX_loop` (choice index 2) | **Saw Down Wave:** Descending ramp from top-left to bottom-right, filled accent when active |

> *Aesthetic Advantage:* The bottom row forms a balanced sawtooth dipole pair (`Saw Up` /| side-by-side with `Saw Down` |\), crowned by the symmetrical `Triangle` /\ in the top right, with the independent `Lock` 🔒 toggle firmly stationed in the command top-left corner.

---

## 📐 Exact Coordinate Map

Within each pod ($by \in \{50, 326, 602\}$, width $348\text{px}$, content $x \in [28, 360]$, usable width $332\text{px}$):

```cpp
// Row 1: Bank Stepper Audition Bar
placeStepper(oPrefix + "bank", 28, by + 14, 332, 20);

// Row 2: Full-Width Octave Radio (332px across 5 segments)
if (auto* r = findRadio(oPrefix + "octave")) {
    r->setBounds(28, by + 38, 332, 20);
}

// Row 3: Squarer Recessed OLED Scope (260 x 64 px)
if (oscWaveViews_[o] != nullptr) {
    oscWaveViews_[o]->setBounds(28, by + 62, 260, 64);
}

// Row 3 Right: 2x2 Vector Icon Grid (64 x 64 px)
constexpr int cell = 30;
constexpr int gap = 4;
constexpr int gx = 296; // 360 - (2 * 30 + 4) = 296
const int gy = by + 62;

// Top-Left: Lock
placeIconButton(oPrefix + "speed_lock", gx,              gy,              cell, cell);
// Top-Right: Triangle (Loop choice 1)
placeIconButton(oPrefix + "loop_tri",   gx + cell + gap, gy,              cell, cell);
// Bottom-Left: Saw Up (Loop choice 0)
placeIconButton(oPrefix + "loop_up",    gx,              gy + cell + gap, cell, cell);
// Bottom-Right: Saw Down (Loop choice 2)
placeIconButton(oPrefix + "loop_down",  gx + cell + gap, gy + cell + gap, cell, cell);

// Row 5a: Morph, Odd/Even, Tilt Knobs (by + 130)
placeKnob(oPrefix + "morph",    38,  by + 130, 48, 52, 11);
placeKnob(oPrefix + "odd_even", 150, by + 130, 48, 52, 11);
placeKnob(oPrefix + "tilt",     262, by + 130, 48, 52, 11);

// Row 5b: Formant, Mask, Speed Knobs (by + 194)
placeKnob(oPrefix + "formant",  38,  by + 194, 48, 52, 11);
placeKnob(oPrefix + "mask",     150, by + 194, 48, 52, 11);
placeKnob(oPrefix + "speed",    262, by + 194, 48, 52, 11);
```

---

## 🎨 Vector Glyphs (Normalized $0.0 \dots 1.0$)

```cpp
namespace Icons {

inline juce::Path sawUp() {
    juce::Path p;
    p.startNewSubPath(0.12f, 0.78f);
    p.lineTo(0.50f, 0.22f);
    p.lineTo(0.50f, 0.78f);
    p.lineTo(0.88f, 0.22f);
    return p;
}

inline juce::Path triangle() {
    juce::Path p;
    p.startNewSubPath(0.10f, 0.78f);
    p.lineTo(0.30f, 0.22f);
    p.lineTo(0.50f, 0.78f);
    p.lineTo(0.70f, 0.22f);
    p.lineTo(0.90f, 0.78f);
    return p;
}

inline juce::Path sawDown() {
    juce::Path p;
    p.startNewSubPath(0.12f, 0.22f);
    p.lineTo(0.50f, 0.78f);
    p.lineTo(0.50f, 0.22f);
    p.lineTo(0.88f, 0.78f);
    return p;
}

inline juce::Path padlockBody() {
    juce::Path p;
    p.addRoundedRectangle(0.20f, 0.44f, 0.60f, 0.46f, 0.08f);
    return p;
}

inline juce::Path shackleClosed() {
    juce::Path p;
    p.startNewSubPath(0.32f, 0.44f);
    p.lineTo(0.32f, 0.28f);
    p.cubicTo(0.32f, 0.08f, 0.68f, 0.08f, 0.68f, 0.28f);
    p.lineTo(0.68f, 0.44f);
    return p;
}

inline juce::Path shackleOpen() {
    juce::Path p;
    p.startNewSubPath(0.32f, 0.44f);
    p.lineTo(0.32f, 0.26f);
    p.cubicTo(0.32f, 0.04f, 0.68f, 0.04f, 0.68f, 0.18f);
    p.lineTo(0.68f, 0.24f); // Lifted clear of body
    return p;
}

} // namespace Icons
```

---

## 🚀 Canonical 4-Phase Build Execution Sequence

Once you issue an execution key (`"build"`, `"engage"`, `"make it so"`):

### Phase 1: Contract & Parameter Codegen Lock ($40\text{ms}$)
- Remove 5 retired parameters from `contracts/nyarlat.yaml`:
  - `osc1_detune`, `osc2_detune`, `osc3_detune`
  - `osc2_sync`, `osc3_sync`
- Run `python tools/codegen_apvts.py -p nyarlat` to regenerate `NyarlatParameterIds.h` and `NyarlatParameterLayout.h`.
- Synchronize `NyarlatControlInfo.h`.

### Phase 2: Core DSP Engine Cleanup ($\sim 3\text{s}$ `dsp_tests`)
- Remove `oscDetune` and `oscSync` arguments from `NyarlatVoice.h` and `NyarlatProcessor.cpp`.
- Clean dead hard-sync branches from audio thread.
- Run `dsp_tests.exe` on Tower to verify zero regressions.

### Phase 3: Tri-Layer UI Overhaul ($\sim 2\text{s}$ `<Plugin>_Preview` + `ui_gate.py`)
- Add `VectorIconButton` to `NyarlatEditor.h`.
- Wire the 2×2 icon grid (`Lock`, `Triangle`, `Saw Up`, `Saw Down`) in `NyarlatEditor.cpp`.
- Resize OLED display to $(28, by+62, 260, 64)$ with $256\text{px}$ inner waveform canvas.
- Expand Octave radio to full $332\text{px}$ width.
- Run `python tools/context_bus_bridge.py build --target Nyarlat_Preview --dispatch` and `python tools/ui_gate.py --plugin nyarlat --pillars A,C`.

### Phase 4: Final VST3 Packaging & Harvest
- Build release `Nyarlat_All` on Tower.
- Deploy `Nyarlat.vst3` to system VST3 directory.
- Capture live UI window proof to `nyarlat_ui_proof.png`.
- Emit silent harvest receipt and save session baton.
