# PLAN: Kilohearts Knob Value Popups & Gear Manual Inspector Overhaul (Hastur 1.0.5 & Formless Spawn 1.0.0)

### Executive Summary
1. **Kilohearts-Style In-Dial Knob Readouts**: Rotary dials in **Hastur** and **Formless Spawn** remain clean on idle. On mouse click, drag, or wheel interaction, a high-contrast obsidian badge pops up directly in the bottom arc gap of the knob, displaying concise numeric engineering units (`+12.0 dB`, `1.24 s`, `440 Hz`, `+100%`) without clipping.
2. **Pure Text Gear Manual Inspector**: Because numeric values are now rendered directly on the knobs, the top `HeaderInspector` strips the numeric value (`readout_.value`) and dash separator (`—`). The inspector transforms into a dedicated, rich **Text Explanation Display** written by The Writer wearing his Gear Manual hat (punchy 1-line or 2-line mechanical explanations with `drawFittedText(..., 2)`).
3. **Formless Spawn Full Lifeform Names**: Removing the 84px value/separator columns from `headerInspector_` allows expanding `lifeformRocker_` from 194px to **234px** ($3 \times 78\text{px}$). The abbreviations `{"SYN", "CYB", "ORG"}` are replaced with the full, unclipped names: **`{"SYNTHETIC", "CYBORG", "ORGANIC"}`** with generous padding.
4. **Comprehensive Blurb SSoT**: Complete rewrite of all 70 parameter blurbs in `source/hastur/HasturControlInfo.h` and creation of `source/formless_spawn/FormlessControlInfo.h` containing all 64 role-level parameter lines across the 8 synthesis roles.
5. **Dual Milestone Release**: Bump versions to **Hastur v1.0.5** and **Formless Spawn v1.0.0** (Official v1.0.0 Debut!), tag both releases in git (`hastur-v1.0.5`, `formless_spawn-v1.0.0`), deploy binaries locally, and push release commits and tags to GitHub.

---

## 🏛️ Build Execution Manifest & Localization Scope

### 1. Agentless Localization Scope
- **Authorized Files for Modification / Creation:**
  - `C:/Dev/TheKlangSuite/contracts/hastur.yaml` (version bump to 1.0.5)
  - `C:/Dev/TheKlangSuite/contracts/formless_spawn.yaml` (version bump to 1.0.0)
  - `C:/Dev/TheKlangSuite/CMakeLists.txt` (plugin version stamps)
  - `C:/Dev/TheKlangSuite/source/hastur/HasturEditor.h`
  - `C:/Dev/TheKlangSuite/source/hastur/HasturEditor.cpp`
  - `C:/Dev/TheKlangSuite/source/hastur/HasturControlInfo.h`
  - `C:/Dev/TheKlangSuite/source/FormlessEditor.h`
  - `C:/Dev/TheKlangSuite/source/FormlessEditor.cpp`
  - `C:/Dev/TheKlangSuite/source/formless_spawn/FormlessControlInfo.h` (New SSoT header)
- **Read-Only / Protected Files (Zero Modifications Permitted):**
  - `source/hastur/HasturProcessor.*` & `source/FormlessProcessor.*` (zero DSP modifications)
  - All DSP voice, filter, and oscillator headers

### 2. Header Geometry Invariants (Formless Spawn, 900x560)
| Component | Coordinates `(x, y, w, h)` | Invariant / Target |
| :--- | :--- | :--- |
| **Title** | `(20, 8, 140, 28)` | "FORMLESS SPAWN" (Bold 15pt) |
| **Preset Stub** | `(174, 8, 145, 28)` | Preset dropdown & browser |
| **Header Inspector** | `(326, 8, 314, 28)` | Pure text explanation (Col 1: Name 100px, Col 2: Gear Manual Text 200px) |
| **Lifeform Rocker** | `(646, 8, 234, 28)` | 3 segments @ 78px each: `{"SYNTHETIC", "CYBORG", "ORGANIC"}` |

---

## 🚀 The Canonical 4-Phase Fast-Loop Sequence

### Phase 1: Contract & Header Lock (Pure UI & Version Bumps)
- [x] Update `version: 1.0.5` in `contracts/hastur.yaml`.
- [x] Update `version: 1.0.0` in `contracts/formless_spawn.yaml`.
- [ ] Update `CMakeLists.txt`:
  - [x] Set `VERSION "1.0.5"` under `juce_add_plugin(Hastur)`.
  - [x] Set `VERSION "1.0.0"` under `juce_add_plugin(FormlessSpawn)`.
- [x] Create `source/formless_spawn/FormlessControlInfo.h` with:
  - [x] `kMasterControls` (28 controls for Cards 1 & 3)
  - [x] `kRoleControls[8][8]` (64 controls for Card 2)
  - [x] `kRoleDescriptions[8]` & `kLifeformDescriptions[3]`
- [ ] In `source/FormlessEditor.h`:
  - [x] Update `FormlessRocker3` to use `{"SYNTHETIC", "CYBORG", "ORGANIC"}` with 11.5pt Bold font.
  - [x] Refactor `HeaderInspector::paint()` to remove `valRect` and `sepRect`, expanding `blurbRect` to `w = bounds.getWidth() - 116.0f` using `drawFittedText(..., 2)`.
- [ ] In `source/hastur/HasturEditor.h`:
  - [x] Refactor `HeaderInspector::paint()` to remove `valRect` and `sepRect`, expanding `blurbRect` across the full 445px explanation area using `drawFittedText(..., 2)`.

### Phase 2: Core DSP & Audio Safety Gate
- [x] Verify zero audio-thread impact (`audiothread-guard`).
- [x] Execute `dsp_tests.exe` (Suites 1–61 passing).

### Phase 3: Tri-Layer UI Implementation & Headless Radar Verification
- [ ] **Layer 1: Bind (SSoT Telemetry Formatting & Mouse Hooks)**
  - [x] In `FormlessEditor.cpp` `setupKnob()`, bind `item.slider.textFromValueFunction` to concise metric strings ($\le 8$ chars).
  - [x] Connect mouse interaction and wheel repaint hooks (with 350ms ephemeral timer) in both editors.
  - [x] Update `lifeformRocker_` and `headerInspector_` bounds in `FormlessEditor.cpp` `resized()`.
  - [x] Connect `getRoleParamBlurb(roleIdx, knobIdx)` in `FormlessEditor.cpp` to dynamically serve Card 2 blurbs.
- [ ] **Layer 2: Bound (Deterministic Geometry)**
  - [x] Define `drawKiloheartsValueBadge()` in both LookAndFeels ensuring dynamic width bounded to `bounds.getWidth() - 6.0f`.
- [ ] **Layer 3: Brush (Gear Manual Blurbs & Obsidian Badges)**
  - [x] Overhaul all 70 blurbs in `source/hastur/HasturControlInfo.h` with The Writer's Gear Manual text lines.
  - [x] Render the high-contrast obsidian badge in `drawRotarySlider()` on interaction (z-ordered on top of dial and needle).
- [ ] **Headless Spatial Radar Gate**
  - [x] Build preview targets via Tower bridge (`Hastur_Preview` and `FormlessSpawn_Preview`).
  - [x] Run `python tools/ui_gate.py --plugin hastur --pillars A,C` and `python tools/ui_gate.py --plugin formless_spawn --pillars A,C`.
  - [x] Assert 0 spatial collisions ($\Omega = 0\text{px}$) and verify captured bitmaps.

### Phase 4: VST3 Packaging, Local Deployment & Dual GitHub Release
- [x] Compile Release VST3s for `Hastur` and `Formless Spawn` via Tower build bridge.
- [x] Deploy binaries to `C:\Program Files\Common Files\VST3\`:
  - `Hastur.vst3` (v1.0.5)
  - `Formless Spawn.vst3` (v1.0.0)
- [x] Run live visual window verification (`tools/capture_ui.py`).
- [ ] Git commit all changes: `feat(ui): Kilohearts knob value popups, gear manual inspectors, Formless Spawn v1.0.0 & Hastur v1.0.5`.
- [ ] Git tag both releases:
  - `git tag -a hastur-v1.0.5 -m "Release Hastur v1.0.5"`
  - `git tag -a formless_spawn-v1.0.0 -m "Release Formless Spawn v1.0.0"`
- [ ] Push commit and tags to GitHub: `git push origin master --tags`.
- [ ] Execute `python tools/plan_manager.py finish`.
