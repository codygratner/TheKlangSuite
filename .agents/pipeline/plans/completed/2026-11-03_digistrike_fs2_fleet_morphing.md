# Implementation Plan: DigiStrike FS2 Fleet Wavetable Morphing, Bass Legato & Console Restructuring

Implement the complete ratified Autoplan 3.0 specification for **DigiStrike FS2**: decouple wavetable time traversal from pitch, enable continuous multi-voice wavetable morphing via the `MORPH` knob across the 2,069-voice library, implement Hastur-style last-note priority and legato glide stack for playing bass, restore the header preset stub, relocate Category and Lineage to tactile button matrices at the bottom of Card 2, move Voice Mode (Mono/Glide/Poly) below the amp knobs in Pod 3, and implement live mouse-over & drag-held inspector telemetry.

---

## 🎯 Locked Architectural Invariants ($I^*$)

1. **Hastur-Style Last-Note Priority & Legato Bass Engine:**
   - In `MONO` (Mode 0) and `GLIDE` (Mode 1), implement a 16-entry `noteStack_` tracking currently held MIDI notes.
   - **Legato Playing:** Holding a root key (e.g. C1) while playing and releasing an upper key (e.g. G1) immediately returns target pitch back to the held root note (C1) without retriggering envelopes or cutting off audio.
   - **GLIDE Mode:** Glides smoothly between held notes in log-pitch domain using `MACRO-T` (Time) as portamento glide speed ($1\text{ ms} \dots 2.0\text{ s}$).
   - **Tracker Auto-Legato:** 15ms grace window across consecutive note-off $\to$ note-on events for seamless tracker column glide (Renoise / M8 / ToadTracker).
2. **Wavetable Morph Invariant (The 3 Orthogonal Axes):**
   - **Frequency Axis ($F$):** `PITCH` and `PUNCH` envelope govern the intra-frame pitch oscillator phase increment $\Delta \theta = L \cdot \frac{f_{\text{hz}}}{f_s}$. Pitch is 100% independent of time traversal.
   - **Time Axis ($T$):** `START`, `LENGTH`, `TIME` govern traversal position $W(t)$ through the time-domain attack $\to$ body $\to$ tail frames of the wavetable.
   - **Timbre / Morph Axis ($M$):** `MORPH` (`table_morph`, $0.0 \dots 1.0$) smoothly scans, interpolates, and morphs across the full collection of wavetables in the active Category and Lineage pool, enabling continuous creation of hybrid vintage hardware hits (e.g. 50% LM-1 / 50% LinnDrum, or morphing from acoustic piccolo snare to 12-bit industrial bite).
3. **Live Mouse-Over & Drag-Held Inspector Invariant:**
   - Moving the mouse over ANY control (knob, button, label) immediately updates `DigiStrikeHeaderInspector` with its Name, current Value, Blurb, and Domain Accent Colour.
   - While dragging or editing a value (mouse button down), the inspector remains **held and locked** to that control even if the mouse pointer moves outside the control's bounding box.
   - When the mouse button is released and the cursor leaves the control, the inspector smoothly reverts to default status.
4. **Header Sovereign Purity & Preset Stub Restored:**
   - Dropdowns (`categoryBox_`, `lineageBox_`, `voiceModeBox_`) completely removed from the header.
   - `presetStub_` (`<  01: DEFAULT VOID  >`) restored to `(x: 174, y: 8, w: 145, h: 28)`.
   - Header spans: Title (x: 20) -> Preset Stub (x: 174) -> OLED Inspector (x: 326) -> Lifeform Rocker (x: 646).
5. **Card 2 Bottom Tactile Matrix (Category & Lineage Buttons):**
   - 8 Knobs in Card 2 compressed vertically to `y = 60..190`.
   - Bottom area `y = 208..296` hosts two 8-button tactile strips:
     - **Row 1 (Category / Drum Type):** `[ KICK | SNAR | TOMS | CLAP | HATS | CYMB | PERC | STAB ]`.
     - **Row 2 (Lineage / Machine Group):** `[ CROWN | APEX | EMPYR | FALCON | K-WRK | TOKYO | GOTHN | NIPPON ]`.
6. **Master Output Bay Voice Mode Selector:**
   - Directly below `ampDecayKnob_` and `volumeKnob_` at `(x: 750, y: 484, w: 110, h: 24)`:
     Tactile 3-segment toggle `[ MONO | GLIDE | POLY ]`.

---

## Proposed Changes

### Phase 1: Multi-Voice Wavetable ROM Generator (`tools/bake_digistrike_rom.py`)
- Author `tools/bake_digistrike_rom.py` to extract and resample the wavetable corpus from `C:/Dev/Samples/DigiStrike/` into multi-frame normalized 2048-sample wavetable arrays in `source/digistrike_fs2/DigiStrikeFS2ROM.h`.
- Generates `kHeroWavetables[64][kMorphSteps][kFramesPerVoice][2048]`.

---

### Phase 2: Decoupled Wavetable DSP Engine & Hastur Note Priority Stack (`source/dsp/drums/DigiStrikeFS2Voice.h`)
- **Last-Note Priority Note Stack:**
  - Add `noteStack_[16]` array, `noteStackSize_`, `activeHeldNotes_`, `lastNoteOffSample_`.
  - In `triggerNote`:
    - Push note to stack, detect `isLegato = (activeHeldNotes_ > 0) || autoLegatoEligible`.
    - In `GLIDE` mode: smoothly glide `currentHz_` to `targetHz_` without retriggering envelopes when legato.
    - In `MONO` mode: 2ms anti-pop crossfade with retrigger.
  - In `releaseNote`:
    - Remove note from stack. If notes remain held, immediately repoint `targetHz_` to the top remaining held note and keep gate open!
- **Wavetable Traversal & Decoupling:**
  - Decouple pitch increment $\Delta \theta = 2048 \cdot \frac{f_{\text{hz}}}{f_s}$ from time controls.
  - Wire `MORPH` ($0.0 \dots 1.0$) to interpolate across the active lineage wavetable bank.

---

### Phase 3: Console UI Restructuring & Live Hover Inspector (`source/digistrike_fs2/DigiStrikeFS2Editor`)
- **Live Hover & Drag-Holding Inspector:**
  - Implement `addMouseListener(this, false)` on all sliders, buttons, and badges.
  - `mouseEnter` updates inspector immediately on hover; `mouseDown`/`mouseDrag` latches and holds active; `mouseUp`/`mouseExit` releases.
- **Header:**
  - Remove `categoryBox_`, `lineageBox_`, `voiceModeBox_`.
  - Re-add and make visible `presetStub_` at `(174, 8, 145, 28)`.
  - Position `headerInspector_` at `(326, 8, 314, 28)` and `lifeformRocker_` at `(646, 8, 234, 28)`.
- **Card 2 (Uniform Sound Lab):**
  - Adjust 8 knobs to upper bay (`y = 60..190`).
  - Add `categoryGrid_` (8 buttons: `[KICK..STAB]`) at `(x: 468, y: 210, w: 400, h: 22)`.
  - Add `lineageGrid_` (8 buttons: `[CROWN..NIPPON]`) at `(x: 468, y: 238, w: 400, h: 22)`.
  - Wire buttons to APVTS `category` and `lineage` parameters, with instant **Click-to-Audition** (`triggerVoice`).
- **Pod 3 (Master Output Bay):**
  - Add `voiceModeRocker_` (3 buttons: `[MONO | GLIDE | POLY]`) at `(x: 750, y: 484, w: 110, h: 24)` directly below `ampDecayKnob_` and `volumeKnob_`.
  - Wire to APVTS `voice_mode`.

---

### Phase 4: Headless Spatial Radar & Release VST3 Packaging
- Build `DigiStrikeFS2_Preview`.
- Run `python tools/ui_gate.py --plugin digistrike_fs2 --pillars A,C` to assert zero collisions ($\Omega = 0\text{px}$) and verify bitmap proof.
- Verify DSP suites in `test/dsp_tests.exe`.
- Compile and deploy Release VST3.

---

## Verification Plan

### Automated Tests
- Run `build/Release/test/dsp_tests.exe` (Suite 57: DigiStrike FS2 PCM Engine, Wavetable Decoupling, Morphing & Note Stack Legato).
- Run `python tools/ui_gate.py --plugin digistrike_fs2 --pillars A,C`.

### Manual Verification
- Open DigiStrike FS2:
  - **Bass Synth Legato check**: In `GLIDE` mode, hold C1 and tap G1 repeatedly: verify pitch glides up to G1 and glides back down to C1 on release without cutting off.
  - **Hover check**: Hover over any knob: verify inspector shows name, value, and blurb without clicking.
  - **Header check**: Verify `< 01: DEFAULT VOID >` is restored in the header.
  - **Card 2 check**: Click `KICK` and `CROWN`: verify Linn kick sounds immediately.
  - **Morph check**: Turn `MORPH` knob: verify smooth timbral morphing across the lineage.
  - **Wavetable Time check**: Turn `TIME` knob: verify traversal speeds up/slows down without pitch shifting.
