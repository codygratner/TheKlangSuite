# Architectural Plan: The Klang R1 (TKR-1)
**Product Category**: Sibling Spin-Off Drum Synthesizer  
**Inspiration**: Korg Electribe R (ER-1) Rhythm Synthesizer  
**Status**: Planned (Post-1.0 Sibling Target)

---

## 1. Concept & Vision
*The Klang R1* is a stripped-down, lightning-fast 7-voice drum synthesizer engineered to deliver the raw, immediate, tactile groovebox workflow of the legendary **Korg Electribe ER-1** directly inside modern DAWs. By stripping away chromatic MIDI note tracking and focusing on dedicated percussive voices mapped across the white keys in any octave, it turns any MIDI keyboard into a mistake-proof, zero-latency finger drumming console with dedicated multi-out routing.

---

## 2. Voice Architecture & Sound Engine
The synthesizer features 7 concurrent monophonic rhythm voices divided into two specialized sonic tiers:

### Tier 1: Voices 1–4 (Pure Synthesizer Parts)
- **Role**: Kicks, Sub-Basses, Snares, Tuned Toms, Laser Zaps, and Resonant Sci-Fi Percussion.
- **Controls per Voice**:
  - `Pitch`: Base pitch knob ($20\,\text{Hz} - 2\,\text{kHz}$) with exponential tuning curve.
  - `Decay`: Exponential amplitude decay envelope ($5\,\text{ms} - 3.5\,\text{s}$).
  - `Mod Type`: Selectable modulation waveform:
    1. `Saw Drop`: Fast exponential pitch drop (classic 90s FM techno kick & zap transient).
    2. `Sine LFO`: Smooth vibrato / FM wobble.
    3. `Tri LFO`: Linear frequency sweep.
    4. `Noise`: Sample & hold noise modulation for industrial snares and crunchy percs.
  - `Mod Speed`: Modulation frequency ($0.1\,\text{Hz} - 2.5\,\text{kHz}$).
  - `Mod Depth`: Bipolar modulation depth ($-100\%$ to $+100\%$).
  - `Pan`: Stereo placement ($L100$ to $R100$).
  - `Level`: Output gain ($-\infty$ to $+6\,\text{dB}$).
  - `Delay Send`: Toggle button routing voice into the Master Tempo Delay.

### Tier 2: Voices 5–7 (Percussion & Metallic Parts)
- **Voice 5 (Closed Hi-Hat)**: Filtered metallic noise + bandpass ring generator with tight decay ($10\,\text{ms} - 400\,\text{ms}$).
- **Voice 6 (Open Hi-Hat)**: Sustained metallic sizzle with longer decay envelope ($50\,\text{ms} - 2.5\,\text{s}$).
  - **Automatic Choke Group**: Triggering Voice 5 immediately cuts Voice 6 with a click-free $1\,\text{ms}$ release ramp, perfectly replicating acoustic hi-hat foot pedals and classic drum machines.
- **Voice 7 (Cymbal / Crash / Metal Perc)**: Rich, dense multi-oscillator metallic cluster with tone filter and extended decay.

### Master Section & ER-1 Heritage Effects
- **Low Boost**: Classic Electribe resonant sub-bass boost ($40\,\text{Hz} - 80\,\text{Hz}$ peaking saturation) designed to give kicks seismic weight.
- **Ring Mod**: Cross-multiplication toggle (`Voice 1 × Voice 2`) producing aggressive clangorous metallic timbres and metallic snares.
- **Master Tempo Delay**:
  - Synced to host tempo ($1/32$ through $1/2$).
  - Feedback/Depth control with soft-clipping tape feedback.
  - Independent per-voice `Delay Send` buttons.
- **Master Limiter**: Transparent brickwall safety ceiling.

---

## 3. Performance & Keyboard Mapping
- **Octave-Invariant White Key Triggering**:
  - Incoming MIDI note pitch tracking is completely decoupled from voice frequency.
  - The voice triggered is determined by musical pitch class:
    - `C` (Note % 12 == 0) &rarr; **Voice 1 (Synth 1 / Kick)**
    - `D` (Note % 12 == 2) &rarr; **Voice 2 (Synth 2 / Snare/Zap)**
    - `E` (Note % 12 == 4) &rarr; **Voice 3 (Synth 3 / Low Tom)**
    - `F` (Note % 12 == 5) &rarr; **Voice 4 (Synth 4 / High Tom)**
    - `G` (Note % 12 == 7) &rarr; **Voice 5 (Perc 1 / Closed Hat)**
    - `A` (Note % 12 == 9) &rarr; **Voice 6 (Perc 2 / Open Hat)**
    - `B` (Note % 12 == 11) &rarr; **Voice 7 (Perc 3 / Cymbal)**
- **Safe Mode Black Keys**:
  - Black keys (`C#`, `D#`, `F#`, `G#`, `A#`) are completely inactive.
  - Live finger drumming can be played anywhere on a keyboard with zero risk of triggering false notes.

---

## 4. DAW Multi-Out Bus Architecture
- **Bus Configuration (`juce::AudioProcessor::BusesProperties`)**:
  - `Bus 0`: Master Stereo Mix (Default L/R)
  - `Bus 1`: Voice 1 Stereo Out
  - `Bus 2`: Voice 2 Stereo Out
  - `Bus 3`: Voice 3 Stereo Out
  - `Bus 4`: Voice 4 Stereo Out
  - `Bus 5`: Voice 5 Stereo Out
  - `Bus 6`: Voice 6 Stereo Out
  - `Bus 7`: Voice 7 Stereo Out
- **Intelligent Auto-Muting**:
  - When the DAW activates an aux bus, that voice's dry signal is automatically removed from the Master Mix.
  - Wet Delay and Low Boost returns can remain on the Master Bus or follow user routing preferences.

---

## 5. UI Layout: All-in-One Console (Zero Tabs)
- **Single-Screen Form Factor**:
  - 7 vertical mixer-style channel strips arranged horizontally side-by-side.
  - Each strip contains:
    - Voice label & activity LED (lights up on trigger, click to audition).
    - Large tactile `Pitch` and `Decay` knobs.
    - `Mod Type` selector button + `Speed` / `Depth` knobs.
    - `Pan` and `Level` faders/knobs.
    - Glowing red `[DELAY SEND]` button.
  - Far right section: Master Console housing **Low Boost**, **Ring Mod 1x2**, **Tempo Delay**, and Master Output Meter.

---

## 6. Codebase Architecture & Reusability
- Built as a sibling build target in `CMakeLists.txt`:
  ```cmake
  juce_add_plugin(TheKlangR1
      COMPANY_NAME "R'lyeh Sound"
      IS_SYNTH TRUE
      NEEDS_MIDI_INPUT TRUE
      FORMATS ${TKF_FORMATS}
      PRODUCT_NAME "The Klang R1"
  )
  ```
- Inherits from `KlangCoreProcessor` and `KlangCoreEditor`.
- Reuses:
  - `TbdAudio::FastMath` for all DSP calculations.
  - `assets/controls/r1_voices.json` for APVTS definitions.
  - Universal theme system and vector LookAndFeel.
  - Automated `gui_tests` and `dsp_tests` reflection suites.

---

## 7. The "Klang-Brain" Generative Engine (TKR-1 & TKF Integration)

The **Klang-Brain** is an algorithmic sequencing and modulation engine shared across the ecosystem. In **The Klang R1 (TKR-1)**, it drives procedural rhythm and drum triggers across the 7 voices. In **The Klang Farmer (TKF)**, it functions as a multi-lane stepped and continuous modulation generator.

### 5 Algorithmic Operating Modes
1. **Fugue Machine (Multi-Playhead Counterpoint)**:
   - 4 independent playheads reading a single pattern buffer simultaneously.
   - Each playhead possesses its own clock divider ($1/4$, $1/8$, $1/16$, $1/32$), playback direction (`Forward`, `Reverse`, `Ping-Pong`), and octave/velocity scaling.
2. **Matriceal Polymeter (Decoupled Parameter Lanes)**:
   - Inspired by the Oxi One Matriceal mode and iPad modular sequencers (Rozeta/CYKLE).
   - Independent loop lengths for discrete parameter lanes:
     - Lane 1: **Triggers / Gates** (e.g. 5 steps)
     - Lane 2: **Velocity / Accent** (e.g. 7 steps)
     - Lane 3: **Pitch / Octave Offset** (e.g. 3 steps)
     - Lane 4: **Modulation CC / Morph** (e.g. 11 steps)
   - Generates ever-shifting, non-repeating polyrhythmic grooves that stay musically coherent.
3. **Stage Pulses & Accumulators (Metropolix & M8 Tables)**:
   - Intellijel Metropolix-style stage sequencing: each stage defines a pitch, gate type, and a pulse count ($1-8$) before advancing to the next stage.
   - Dual Accumulators: Increment or decrement fixed interval values on each loop cycle or stage trigger.
   - Dirtywave M8 sub-tick micro-tables: Execute micro-chops, retriggers, and probability hops per step.
4. **Turing Machine (LFSR Generative Mutation)**:
   - Classic Music Thing Modular 16-bit Linear Feedback Shift Register (LFSR) topology (as seen in the Moog Labyrinth / Buchla 266).
   - A single **Mutation / Chaos** control:
     - At $0\%$ (`Lock`): The pattern loops in an exact, repeating 8/16-step cycle.
     - At $1-99\%$: Bit flips occur probabilistically, generating organic melodic and rhythmic drift that gradually morphs.
     - At $100\%$ (`Random`): Pure Brownian pseudorandom sequence.
5. **Axon Neural Network (Leaky Integrate-and-Fire Biological Neurons)**:
   - Inspired by Audio Damage's *Axon* (1, 2 & 3) neural network drum sequencer.
   - A 7-neuron interconnected biological network mapped **1:1 to TKR-1's 7 voices** (or a 4-to-8 node modulation lattice in TKF).
   - Each neuron acts as a leaky accumulator with an excitation threshold, decay rate, and synaptic connections to other neurons.
   - When an incoming clock pulse or neighboring neuron's output charges a neuron past its threshold, it fires an action potential:
     - Instantly triggers that drum voice (e.g. Kick).
     - Propagates positive (excitatory) or negative (inhibitory) energy down its synaptic axons to other voices (e.g. firing Voice 1 excites Voice 5/Hat or inhibits Voice 2/Snare).
   - Generates organic, biologically interdependent polyrhythmic grooves that breathe and self-evolve without rigid step programming.

### Playback & Advance Modes: Continuous vs. DSI Pro 2 Triggered Key-Advance
The engine features a dedicated **Advance Mode** switch:
1. **Continuous Mode (DAW Playhead Sync)**:
   - Free-running sequence locked to host DAW tempo, transport playhead, and MIDI clock.
   - Automatically pauses/resumes with DAW playback.
2. **Triggered / Key-Advance Mode (Dave Smith Instruments Pro 2 Gated Style)**:
   - The sequencer lanes **do not run on an internal timer or free clock**.
   - Instead, the lanes remain stationary until an incoming **MIDI Note-On** event or gate trigger is received.
   - Each incoming trigger advances the active step counters across all lanes by exactly $+1$.
   - **Polymetric Magic**: Because each lane has an independent step length (e.g., 5 steps of trigger probability, 7 steps of velocity, 3 steps of pitch offset), sequentially tapping or playing keys steps through an evolving mathematical lattice of parameter states!
   - In **TKF**, this turns the keyboard into a gated polymetric modulation advance, altering FM ratio, filter cutoff, and distortion drive with every successive note played.
   - In **TKR-1**, this allows external sequencers or keyboard players to step the rhythm engine interactively per note.

---

### Tabled Algorithmic Engines & Future Deep Research Lab
The following esoteric generative paradigms are officially **Tabled for Future R&D**:
- **Conway's Game of Life (Cellular Automata)**: 2D living pixel grid triggering gates on perimeter collision or cell death.
- **Wolfram Elementary Automata**: 1D deterministic cellular automata (Rule 30 chaotic pseudorandom sequences, Rule 110 universal computation).
- **Markov Chain Matrix**: Probabilistic state-transition matrix determining next-step musical decisions based on preceding history.
- **L-Systems (Lindenmayer Systems)**: Recursive fractal grammar re-writing producing self-similar branching rhythms and melodies.

> [!NOTE]  
> **Dedicated Deep Research Lab Scheduled**: A comprehensive Deep Research Lab session will be conducted when initiating the active TKR-1 development milestone to benchmark, prototype, and architect these advanced generative systems.


