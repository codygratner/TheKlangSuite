# Architectural Plan: Pre-1.0 Sound, Workflow & Performance Expansion
**Target Milestones**: v0.4.0 ("The Sound & Chaos Update") & v0.5.0 ("The Pro Workflow Update")  
**Scope**: Undo/Redo & A/B State, Velocity Curves & MIDI CC Learn, Panic Audio Kill, Dual-Tier Oversampling, and 4 Performance Macros.

---

## 1. Sound Design Safety: Undo / Redo & A/B State Comparison (v0.4.0)
### Architecture & State Model
- **`juce::UndoManager` Integration**:
  - Add `juce::UndoManager undoManager;` to `KlangCoreProcessor` and pass to `juce::AudioProcessorValueTreeState` constructor.
  - Wrap UI slider interactions with `beginNewTransaction()` on mouse-down / drag start and mouse-up.
- **Randomization Transactions**:
  - The v0.4.0 d6 Randomizer triggers `undoManager.beginNewTransaction("Randomize <Target>")` before mutating APVTS parameters, ensuring any dice roll can be instantly reversed.
- **A/B State Comparison**:
  - Maintain two in-memory `juce::ValueTree` snapshots: `stateA` and `stateB`.
  - Active slot tracker: `enum class ActiveSlot { A, B }`.
  - Switching slots copies the corresponding `ValueTree` into the live APVTS smoothly.
  - Right-click menu on `[ A | B ]` button exposes:
    - `Copy A to B` (when A is active)
    - `Copy B to A` (when B is active)
- **UI & Keybindings**:
  - Front-panel header buttons: `↶` (Undo), `↷` (Redo), and `[ A | B ]` pill button.
  - Global hotkeys: `Ctrl+Z` / `Cmd+Z` for Undo, `Ctrl+Y` / `Cmd+Shift+Z` for Redo.

---

## 2. Velocity Sensitivity Curves & MIDI CC Learn (v0.4.0)
### Dynamic Velocity Response
- **Voice & Articulation Callout Modal Integration**:
  - Add Velocity controls directly to `VoiceArticulationCalloutComponent` alongside Gated Bass and Glide controls.
- **Selectable Curves**:
  1. `Linear`: Standard $1:1$ input velocity mapping.
  2. `Exponential`: S-curve with soft-touch dynamics ($v_{scaled} = v^{1.8}$).
  3. `Logarithmic`: Quick attack for hard/aggressive playing ($v_{scaled} = v^{0.5}$).
  4. `Fixed (127)`: Completely overrides velocity to full dynamic impact ($1.0$), crucial for electronic dance, techno, and industrial kicks.
- **Velocity Depth**: Continuous slider ($0\%$ to $100\%$) blending between fixed output and velocity-scaled output.

### MIDI CC Learn & Controller Mapping
- **Right-Click Context Menu**:
  - Any parameter slider or knob exposes `MIDI Learn` and `Clear MIDI CC`.
  - Selecting `MIDI Learn` enters an "Arm" state where the next incoming MIDI CC number on `MidiBuffer` is automatically bound to that parameter ID.
- **Persistence**:
  - Learned CC mappings are stored in the user preferences JSON (`settings.json`) in AppData / Application Support.
  - Hardcoded conflict prevention: Ignores CC 120 (All Sound Off), CC 123 (All Notes Off), and CC 64 (Sustain).

---

## 3. Panic / Kill Audio (Emergency Silence & DSP Flush) (v0.4.0)
### Emergency Shutoff Logic
- **Triggers**:
  1. Double-clicking the Master Peak Meter or CPU load indicator in the header.
  2. Incoming MIDI CC 120 (All Sound Off) or CC 123 (All Notes Off).
  3. "Panic" button in Settings & About modal.
- **Audio Thread Execution**:
  - **1ms Anti-Pop Ramp**: Triggers an ultra-fast linear/exponential fade to silence across 48 samples to prevent acoustic DC pops.
  - **DSP Buffer Flush**: Immediately zeroes internal feedback lines across:
    - Delay lines (`TempoDelayBlock`, `HaasDelayBlock`, `CombFilterBlock`).
    - Reverb tank buffers (`GatedReverbBlock`).
    - Resonators & Flanger feedback (`WaveguideResonatorBlock`, `FlangerBlock`).
  - **Voice State Reset**: Sets active voice status to idle and clears gate/legato tracking.

---

## 4. Dual-Tier 2x / 4x Oversampling Engine (v0.5.0)
### Anti-Aliasing Strategy
- **Why Oversampling?**
  - High-index FM synthesis (Carrier $\times$ Modulator) generates dense sidebands extending beyond the Nyquist limit, resulting in audible foldback aliasing in the 2 kHz–15 kHz range.
  - Non-linear effects (Wavefolder, Custom Waveshaper, Drive, Bitcrusher) generate upper harmonics that fold down into dissonant harshness.
- **`juce::dsp::Oversampling<float>` Implementation**:
  - Instantiated inside `TheKlangFarmerAudioProcessor`.
  - **Dual-Tier Settings**:
    - **Realtime / Live**: `[ Off (1x) | 2x | 4x ]` (defaults to 1x or 2x depending on host sample rate). Uses **Minimum-Phase IIR** filters for zero monitoring latency.
    - **Render / Export**: Up to `8x` oversampling automatically applied during offline bounce or WAV/SF2 rendering.
- **Real-Time Safety**:
  - All oversampling buffers pre-allocated in `prepareToPlay()`. Zero memory allocations inside `processBlock()`.

---

## 5. 4 Performance Macro Knobs (TBD-16 Hardware Aligned) (v0.5.0)
### Front-Panel & Hardware Mapping
- **Hardware Alignment**:
  - 4 endless push-encoders on **Page 1 of the dadamachines TBD-16** map directly to Macros 1–4.
  - On the desktop plugin, 4 macro knobs reside permanently in the header bar for instant tweaking regardless of the currently viewed sub-page.
- **Right-Click Assignment Matrix**:
  - Right-click any parameter knob &rarr; `Assign to Macro 1..4`.
  - Configurable bipolar modulation depth (`-100%` to `+100%`).
  - Macro modulation operates additively on the base parameter value with saturation clipping at parameter bounds.
- **Preset Serialization**:
  - Macro assignments, labels, and depths are saved directly inside each preset's JSON definition (`assets/presets/*.json`).

---

## 6. Automated Verification & Test Coverage
- **`dsp_tests`**:
  - Unit test verifying minimum-phase oversampling returns correct phase symmetry and zero NaN values.
  - Test verifying Panic instantly reduces buffer energy to zero and clears delay lines without DC offset.
  - Test verifying velocity curve mathematical models ($Linear, Exponential, Logarithmic, Fixed$).
- **`gui_tests`**:
  - Synthetic mouse click on `[ A | B ]` and Undo/Redo verifying parameter rollback.
  - Synthetic right-click test on Macro assignment and MIDI Learn arming.
