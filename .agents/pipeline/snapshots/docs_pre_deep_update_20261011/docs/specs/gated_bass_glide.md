# Voice & Articulation Engine: Gated Staccato Bass, Note-Off Release & Portamento Glide

**Target Milestone**: v0.4.0 "The Sound & Chaos Update"  
**Scope**: Both Plugins (*The Klang Farmer* and *The Klang Planter*)  
**Architecture**: Data-Driven JSON Parameters + Audio Thread Invariants + Unified Voice & Articulation Modal  

---

## Executive Summary

While *The Klang Farmer* and *The Klang Planter* were originally conceptualized as one-shot dual-FM drum synthesizers, their versatile carrier/modulator engines, pitch envelopes, and multi-mode filters make them monstrous platforms for synthesizing modern electronic basslines (Reese bass, aggressive acid bass, 808 subs, and resonant FM talk-bass).

This architectural plan expands the synth engine from a purely percussive one-shot instrument into a dual-threat **Drum & Bass Synthesizer**. It unifies two critical performance capabilities into a dedicated **Voice & Articulation** system:
1. **Gated Trigger Mode & Settable Note-Off Release**: Releasing MIDI keys immediately cuts the voice with a customizable, click-free exponential release ramp (1 ms to 30 ms).
2. **Polyphonic/Monophonic Portamento Glide & Legato Retrigger**: Smooth pitch transitions between notes with dual-mode timing (Milliseconds vs Musical Tempo Sync), customizable 3-point slope curves (Exp &rarr; Lin &rarr; Log), and a Legato Retrigger switch for choosing between punchy 808 glides and continuous acid slides.

```
                     ┌───────────────────────────────────┐
                     │          MIDI Input Stream        │
                     └─────────────────┬─────────────────┘
                                       │
                     ┌─────────────────┴─────────────────┐
                     ▼                                   ▼
             [ Note-On Event ]                   [ Note-Off Event ]
                     │                                   │
                     ├─────────────────────────┐         ▼
                     ▼                         ▼    Is Mode == GATED?
             Is GLIDE active?             Retrigger? and note == activeNote?
             - Legato (overlap)           - OFF: Continuous     │
             - Always (all notes)         - ON: Re-strike  ┌────┴────┐
                     │                                     ▼         ▼
                     ▼                                    NO        YES
        Slew pitch in semitone space:                 (One-Shot)  Exp Ramp
        Exp / Lin / Log slope curve                    Ignore     to 0.0 in
        Time: 5ms–2000ms or 1/64–1/2 bar                          1–30 ms
```

---

## 1. User Interface & Experience

### 1.1 Front-Panel Header Badge (`VOICE / ARTICULATION`)
- Located in the global top-right header strip beside `INIT` and the visualizer toggles.
- **Dynamic Badge States**:
  - `ONE-SHOT`: Dim amber outline (classic drum mode; Note-Offs ignored; envelopes ring out).
  - `GATED`: Electric cyan/teal pill (staccato bass mode; notes cut cleanly on key release).
  - `GLIDE ~`: Displays an animated tilde glyph when Portamento Glide is active (`GLIDE: LEGATO` or `GLIDE: ALWAYS`).
- **Interactions**:
  - **Left-Click**: Instant toggle between `One-Shot` and `Gated` modes.
  - **Right-Click**: Launches the unified `VoiceArticulationCalloutComponent`.

### 1.2 Unified Modal Callout (`VoiceArticulationCalloutComponent`)
Styled with the project's signature tactile dark industrial chassis, recessed borders, and responsive LED accents:

```
┌─────────────────────────────────────────────────────────────┐
│                 VOICE & ARTICULATION                        │
├─────────────────────────────────────────────────────────────┤
│ TRIGGER MODE:        [ ONE-SHOT ]     [ GATED ]             │
│ NOTE-OFF RELEASE:    [───────●──────────────────]  5.0 ms   │
├─────────────────────────────────────────────────────────────┤
│ GLIDE MODE:          [ OFF ]   [ LEGATO ]   [ ALWAYS ]      │
│ GLIDE TIME / SYNC:   [ SYNC: OFF ] [────●───────]  60.0 ms  │
│                      (or when Sync: ON: [ 1/16 ] )          │
│ GLIDE SLOPE:         [───────────●──────────────]  Exp (0.0)│
│ LEGATO RETRIGGER:    [ OFF (Continuous) ]  [ ON (Punchy) ]  │
└─────────────────────────────────────────────────────────────┘
```

#### Controls Inside Callout:
1. **Trigger Mode**: Segmented two-state button (`[ One-Shot ]` `[ Gated ]`).
2. **Note-Off Release Slider**:
   - Range: `1.0 ms` to `30.0 ms` (default `5.0 ms`).
   - Logarithmic warp curve (`0.5`) providing fine precision in the critical `1 ms` to `10 ms` sweet spot.
   - Quick-Snap points: `1`, `2`, `5`, `10`, `15`, `20`, `30 ms`.
3. **Glide Mode**: Segmented three-state button (`[ Off ]` `[ Legato ]` `[ Always ]`).
   - `Off`: Instant pitch jumps.
   - `Legato`: Glides pitch only when playing overlapping notes.
   - `Always`: Glides between every played note, even when playing staccato/detached.
4. **Glide Time / Sync**:
   - `Sync` Toggle: Switches between free milliseconds and DAW beat divisions.
   - `Free Time`: `5.0 ms` to `2000.0 ms` (default `60.0 ms`), with quick-snaps at `10`, `25`, `50`, `100`, `250`, `500 ms`.
   - `Synced Length`: Musical divisions (`1/64`, `1/32`, `1/16T`, `1/16`, `1/8T`, `1/8`, `1/4`, `1/2 bar`).
5. **Glide Slope**:
   - Standard 3-point envelope curve: `Exponential (0.0)` &rarr; `Linear (0.5)` &rarr; `Logarithmic (1.0)`.
   - `Exp`: Starts fast, decelerates into target pitch (analog synthesizer RC response).
   - `Lin`: Constant semitone slew rate.
   - `Log`: Starts slow, accelerates into target pitch.
6. **Legato Retrigger**:
   - `Off (Continuous)`: When gliding between overlapping notes, pitch glides smoothly while envelopes continue their natural decay (ideal for acid basslines and fluid synth leads).
   - `On (Punchy)`: Re-strikes Attack and transient envelopes on every note while pitch glides (ideal for punchy modern 808 slides).

### 1.3 Envelope Gate-Hold Behavior & Smart Acoustic Defaults
To resolve the acoustic trap where global sustain freezes pitch envelopes at peak transient spikes, the engine implements **Smart Musical Defaults + Popover Overrides**:
- **1-Click Header Voice Switch**:
  - `[ ONE-SHOT ]` (Default Drum Mode): All envelopes ignore note-offs and decay naturally.
  - `[ GATED BASS ]` (Bass & Drone Mode): Automatically engages **Noise Engineering Gate-Hold** on the Amp Envelope while keeping Pitch Envelopes in immediate-decay mode. Releasing the key triggers the existing `Decay` knob as the release tail.
- **Per-Envelope Popover Override**:
  - Inside each Envelope's right-click **Inspector Popover**, sound designers can override the default:
    `Gate Behavior: [ ● Auto (Follow Voice) | ○ Always Decay | ○ Force Sustain ]`
  - Allows deep experimental sound design (e.g. holding a filter envelope open or creating rising pitch drones) without complicating standard percussion workflows.

---

## 2. Parameter Architecture (Data-Driven JSON)

All parameters are declared strictly in `assets/controls/global_settings.json` (and `planter_global_settings.json`):

```json
{
  "group": "voice_control",
  "parameters": [
    {
      "id": "trigger_mode",
      "name": "Trigger Mode",
      "type": "choice",
      "choices": ["One-Shot", "Gated"],
      "default": 0,
      "tooltip": "TRIGGER MODE: One-Shot ignores MIDI note-offs (ideal for drum hits). Gated cuts voice on key release (ideal for staccato bass)."
    },
    {
      "id": "note_off_release",
      "name": "Note-Off Release",
      "type": "float",
      "min": 0.001,
      "max": 0.030,
      "default": 0.005,
      "skew": 0.5,
      "unit": "s",
      "tooltip": "NOTE-OFF RELEASE: Fast exponential fade-out time (1ms to 30ms) applied when releasing a key in Gated mode to eliminate DC clicks."
    },
    {
      "id": "glide_mode",
      "name": "Glide Mode",
      "type": "choice",
      "choices": ["Off", "Legato", "Always"],
      "default": 0,
      "tooltip": "GLIDE MODE: Off (instant pitch jumps), Legato (glides only when playing overlapping notes), Always (glides between all notes)."
    },
    {
      "id": "glide_sync",
      "name": "Glide Sync",
      "type": "choice",
      "choices": ["Time", "Sync"],
      "default": 0,
      "tooltip": "GLIDE SYNC: Toggle between free millisecond glide time and DAW tempo-synced musical lengths."
    },
    {
      "id": "glide_time",
      "name": "Glide Time",
      "type": "float",
      "min": 0.005,
      "max": 2.000,
      "default": 0.060,
      "skew": 0.4,
      "unit": "s",
      "tooltip": "GLIDE TIME: Duration of the pitch transition in milliseconds."
    },
    {
      "id": "glide_sync_length",
      "name": "Glide Sync Length",
      "type": "choice",
      "choices": ["1/64", "1/32", "1/16T", "1/16", "1/8T", "1/8", "1/4", "1/2"],
      "default": 3,
      "tooltip": "GLIDE SYNC LENGTH: Tempo-synced duration of the pitch slide."
    },
    {
      "id": "glide_slope",
      "name": "Glide Slope",
      "type": "float",
      "min": 0.0,
      "max": 1.0,
      "default": 0.0,
      "tooltip": "GLIDE SLOPE: Pitch slide curve. Exp (0.0, classic analog RC slew), Linear (0.5), Log (1.0)."
    },
    {
      "id": "legato_retrigger",
      "name": "Legato Retrigger",
      "type": "choice",
      "choices": ["Off", "On"],
      "default": 0,
      "tooltip": "LEGATO RETRIGGER: Off keeps envelopes continuous during legato slides (fluid acid). On re-triggers envelopes on each note (punchy 808s)."
    }
  ]
}
```

---

## 3. Real-Time Audio DSP Implementation

### 3.1 Audio Thread Invariants
- Zero heap allocations (`malloc`, `new`, vector resizing) in `processStereo()` or `noteOn()` / `noteOff()`.
- Zero thread locks or mutexes.
- Vectorized transcendentals via `TbdAudio::FastMath::fastPow2` and `TbdAudio::FastMath::fastExp`.

### 3.2 Semitone-Space Slew Filtering
Portamento pitch slewing is performed in **semitone (MIDI note) space** rather than linear Hz. This ensures a 1-octave glide across low bass (C1 &rarr; C2) takes the exact same musical duration and perceptual curve as a 1-octave glide in higher registers (C3 &rarr; C4):

```cpp
// In ModularEngine / PlanterEngine
void updatePitchSlew(int numSamples) {
    if (currentPitchSemitones == targetPitchSemitones || glideMode == 0) {
        currentPitchSemitones = targetPitchSemitones;
        ctx.currentPitchHz = 440.0f * FastMath::fastPow2((currentPitchSemitones - 69.0f) / 12.0f);
        return;
    }

    float glideDuration = (glideSync == 1) ? calculateSyncTime(glideSyncLength, ctx.bpm) : glideTime;
    float step = (numSamples * ctx.invSr) / std::max(glideDuration, 0.001f);
    
    // Apply Glide Slope (Exp, Lin, Log) to progress
    glideProgress = std::clamp(glideProgress + step, 0.0f, 1.0f);
    float curvedProgress = applyEnvelopeSlope(glideProgress, glideSlope);
    
    currentPitchSemitones = startPitchSemitones + (targetPitchSemitones - startPitchSemitones) * curvedProgress;
    ctx.currentPitchHz = 440.0f * FastMath::fastPow2((currentPitchSemitones - 69.0f) / 12.0f);
}
```

### 3.3 Active Note & Overlap Tracking
```cpp
void noteOn(int noteNumber, float velocity) {
    bool isOverlapping = (activeNotesCount > 0);
    activeNotesCount++;
    activeMidiNote.store(noteNumber, std::memory_order_relaxed);
    inReleaseFade = false;

    bool shouldGlide = (glideMode == 2) || (glideMode == 1 && isOverlapping);
    if (shouldGlide && hasPreviousNote) {
        startPitchSemitones = currentPitchSemitones;
        targetPitchSemitones = static_cast<float>(noteNumber);
        glideProgress = 0.0f;
    } else {
        startPitchSemitones = targetPitchSemitones = static_cast<float>(noteNumber);
        currentPitchSemitones = targetPitchSemitones;
        ctx.currentPitchHz = 440.0f * FastMath::fastPow2((currentPitchSemitones - 69.0f) / 12.0f);
    }
    hasPreviousNote = true;

    // Retrigger check
    bool shouldRetrigger = (!isOverlapping) || (legatoRetrigger == 1);
    if (shouldRetrigger) {
        trigger(velocity);
    }
}

void noteOff(int noteNumber) {
    if (activeNotesCount > 0) activeNotesCount--;

    if (triggerMode == 1) { // Gated Mode
        if (noteNumber == activeMidiNote.load(std::memory_order_relaxed) && activeNotesCount == 0) {
            inReleaseFade = true;
            releaseStartAmp = currentAmpEnv;
            releaseProgress = 0.0f;
        }
    }
}
```

---

## 4. Verification & Automated Test Plan

### 4.1 DSP Stability Tests (`test/dsp_tests.cpp`)
1. **One-Shot vs Gated Cutoff**: Verify `Note-Off` is ignored in One-Shot mode and silences voice by $> 40\,\text{dB}$ within `note_off_release` window in Gated mode.
2. **Portamento Semitone Slew Verification**: Assert that frequency smoothly transitions from 55 Hz (A1) to 110 Hz (A2) over the specified glide time without discontinuities.
3. **Legato vs Always Glide Test**: Verify that in `Legato` mode, separated notes jump instantly while overlapping notes glide. Verify `Always` mode glides on both.
4. **Legato Retrigger Verification**: Assert that with `Legato Retrigger: Off`, envelope phase continues uninterrupted during overlapping note transitions.

### 4.2 GUI Reflection Tests (`test/gui_tests.cpp`)
1. **APVTS Reflection Sweep**: 100% sweep of all 8 Voice & Articulation parameters.
2. **Header Badge Click Toggle**: Verify clicking badge cycles `trigger_mode`.
3. **Right-Click Callout Simulation**: Verify right-click opens `VoiceArticulationCalloutComponent`, tests slider interactions, and dismisses on outside click.
