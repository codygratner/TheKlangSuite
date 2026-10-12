# Feature: New Effects Processors Catalog Expansion (Effects 14–26) & Universal Mix Standard

## Objective
Implement 13 new studio-grade, zero-allocation multi-instance effects for `TheKlangFarmer`'s 8-slot FX rack:
1. **Transient Shaper** (Envelope dynamics: Attack, Pump, Sustain, Speed)
2. **Custom Waveshaper** (Nonlinear transfer curve morphing: Sine/Tri/Saw/Square/PWM, Drive, DJ Filter, Universal Mix)
3. **Channel Mixer** (4-knob cross-channel matrix mixer: L->L, R->L, L->R, R->R)
4. **Stereo Enhancer** (Mid/Side field shaper: Mid, Width up to 600%, Pan, Analog Slop drift)
5. **Haas Delay** (Micro-delay spatial widener: Bipolar -100ms..+100ms, Tone damping, Feedback, Universal Mix)
6. **Lo-Fi Gated Reverb** (Dense 80s early reflections: Room Time, DJ Filter, BPM-synced Gate Time, Universal Mix)
7. **Juno-60 Chorus** (Roland BBD emulation: 4-State Mode, Dirt/Vintage filtering & hiss, Stereo Width, 50% 1:1 Mix)
8. **Waveguide Resonator** (Volca Drum physical model: Tube vs String modes, Tune, Decay resonance, Universal Mix)
9. **Sub Generator** (Phase-locked 808 sub oscillator: Tune 30-120 Hz, Decay tail, Harmonic Warmth drive, Universal Mix)
10. **Analog Tape Warmth** (Magnetic tape/cassette: Tape Drive soft clipping, Tone/Bias HF damping, Wow/Flutter, Universal Mix)
11. **Dynamic Envelope Filter** (Transient-following auto-sweep: Cutoff, Bipolar Env Depth, Resonance/Q, Universal Mix)
12. **Harmonic Pitch Transposer** (Semitone/cent transposition: Semitones -12..+12, Detune cents, Feedback loop, Universal Mix)
13. **Rhythmic Stutter Gate** (Tempo-synced chopper: Synced Rate divisions, Duty/Gate slice duration, Smoothing, Universal Mix)

And standardize **Universal Dual-Mode Mix** (Knob 4: `-100%` Wet Crossfade ↔ `0%` Dry ↔ `+100%` Parallel Blend) across all applicable effects.

### Invariants & Technical Constraints
- **Audio-Thread Safety (CRITICAL):** Zero heap allocations (`new`, `malloc`, `std::vector::push_back`, dynamic resizing) in `processStereo()` or `process()`. All delay lines, reflection buffers, and envelope history buffers must be pre-allocated in `init()` for maximum sample rate ($96\,\text{kHz}$).
- **Zero Locks & Zero Blocking I/O:** No mutexes, atomics in hot loops, or console/disk I/O.
- **FastMath Acceleration:** All trigonometric and transcendental functions must utilize `TbdAudio::FastMath` (`fastTanh`, `fastSin`, `fastCos`, `fastPow2`).
- **Backward Compatibility:** Append new algorithms to the end of the catalog (`fxChoices` 14..26) so existing DAW projects and presets (types 0..13) maintain exact 1:1 index alignment without XML migration drift.

---

### Phase 1: Core DSP Block Implementations (`source/ModularBlocks.h`)

- **Target Files:** `source/ModularBlocks.h`, `source/DSPBlock.h`
- **Action Items:**
  - [ ] **1.1: `TransientShaperBlock : public DSPBlock` (Algorithm 14):**
    - Dual envelope follower architecture: Fast attack peak detector ($1\,\text{ms} - 20\,\text{ms}$) vs Slow sustain envelope ($50\,\text{ms} - 300\,\text{ms}$).
    - Knob 0: **Attack** (bipolar -100% to +100%, default 0% / 0.5): scales differential envelope transient gain up to +12 dB or -24 dB.
    - Knob 1: **Pump** (0% to 100%, default 0% / 0.0): dynamic ducking and post-transient upward expansion.
    - Knob 2: **Sustain** (bipolar -100% to +100%, default 0% / 0.5): body/tail amplification via slow envelope leveler.
    - Knob 3: **Speed** (1 ms to 50 ms, default 10 ms / 0.3): detection tracking time coefficient.
  - [ ] **1.2: `ThresherBlock : public DSPBlock` (Algorithm 15 — "The Thresher" Transfer Function Waveshaper):**
    - Clean-room mathematical transfer curve shaper built with zero third-party ROMs or code.
    - Pre-gain input drive via `fastDbToGain(-12 dB to +36 dB)`.
    - **64+ Curated Mathematical Transfer Curves (LUT Tables)**:
      - *Bank 0 (Synthesizer Oscillator Morph)*: Leverages the core FM engine's morphing waveform equations (Sine → Triangle → Saw → Square → PWM) as dynamic transfer curves.
      - *Bank 1 (Harmonic & Fold)*: Chebyshev polynomials ($T_2$ through $T_8$ harmonic multipliers), West Coast sine wavefolders, and soft-rectifiers.
      - *Bank 2 (Bit-Crunch & Logic)*: Bitwise boolean masking (`x ^ (x >> mask)`), staircase stepped quantizers (2-bit to 8-bit dynamic reduction), and crossover distortion dead-bands.
      - *Bank 3 (Mechanical Dirt & Chaos)*: Magnetic tape hysteresis curves, asymmetric tanh cliffs, and randomized smooth-spline steps.
    - Sample-rate reduction jitter and pre/post DJ-style tilt filter.
    - Universal Dual-Mode Mix on Knob 4: `-100%` (pure wet) ↔ `0%` (dry) ↔ `+100%` (parallel blend).
    - Optional **DOOM Brickwall Mode**: Pushes signal into 0 dBFS ceiling with aggressive soft-saturation curve.
  - [ ] **1.3: `ChannelMixerBlock : public DSPBlock` (Algorithm 16):**
    - Matrix mixer processing:
      - $L_\text{out} = L_\text{in} \cdot (L \to L) + R_\text{in} \cdot (R \to L)$
      - $R_\text{out} = L_\text{in} \cdot (L \to R) + R_\text{in} \cdot (R \to R)$
    - Bipolar gains from $-100\%$ to $+100\%$ with center $0\%$ detents.
    - Defaults: $L \to L = +100\%$, $R \to L = 0\%$, $L \to R = 0\%$, $R \to R = +100\%$.
  - [ ] **1.4: `StereoEnhancerBlock : public DSPBlock` (Algorithm 17):**
    - Mid/Side matrix transformer:
      - $\text{Mid} = 0.5 \cdot (L + R)$, $\text{Side} = 0.5 \cdot (L - R)$
      - $\text{Mid}' = \text{Mid} \cdot \text{gain}_\text{mid}$ ($0\%$ to $100\%$)
      - $\text{Side}' = \text{Side} \cdot \text{gain}_\text{width}$ ($0\%$ to $600\%$)
      - Reconstruct $L = \text{Mid}' + \text{Side}'$, $R = \text{Mid}' - \text{Side}'$.
    - Pan law balance control across reconstructed stereo channels.
    - Slop modulation input: Stepped analog drift per trigger hit modulating Mid gain, Width, and Pan.
  - [ ] **1.5: `HaasDelayBlock : public DSPBlock` (Algorithm 18):**
    - Pre-allocated circular stereo delay buffer ($9600$ samples, $100\,\text{ms}$ @ $96\,\text{kHz}$).
    - Single bipolar delay slider:
      - Negative values: Left channel delayed up to $100\,\text{ms}$ (sound localized Right).
      - Zero: Dry center ($0\,\text{ms}$).
      - Positive values: Right channel delayed up to $100\,\text{ms}$ (sound localized Left).
    - Tone damping one-pole low-pass filter on delayed channel ($200\,\text{Hz} - 20\,\text{kHz}$).
    - Subtle cross-feedback resonance loop ($0\%$ to $80\%$).
    - Universal Dual-Mode Mix on Knob 4.
  - [ ] **1.6: `GatedReverbBlock : public DSPBlock` (Algorithm 19):**
    - Vintage early reflections tapped delay diffuser network (8 decorrelated stereo taps).
    - Integrated bipolar DJ tilt filter for vintage dark vs crispy metallic reflection coloring.
    - BPM-synchronized gate envelope cutoff timer (1/64, 1/32, 1/16, 1/8, 1/4 notes) tied to `ctx.bpm` and `ctx.isTriggered`.
    - Universal Dual-Mode Mix on Knob 4.
  - [ ] **1.7: `BalerBlock : public DSPBlock` (Algorithm 27 — "The Baler" / Working Title: "OTT Multiband Dynamics"):**
    - 3-Band Upward/Downward dynamics processor inspired by Ableton Live Multiband Dynamics & Xfer OTT.
    - Crossover network: 4th-order Linkwitz-Riley (LR4, 24 dB/oct) filters at 250 Hz and 2.5 kHz.
    - Front-Panel 4-Knob Layout (Option A):
      - Knob 0: **Depth** (0% to 100%, default 100% / 1.0): scales both upward and downward compression ratios simultaneously.
      - Knob 1: **Time** (10% to 500%, default 100% / 0.5): scales attack (fast 5ms - 50ms) and release (20ms - 400ms) times across all 3 bands.
      - Knob 2: **Tone** (bipolar -100% to +100%, default 0% / 0.5): 3-band tilt EQ balancing sub-bass punch vs high-end sizzle.
      - Knob 3: **Mix** (Universal Dual-Mode: -100% wet crossfade ↔ 0% dry ↔ +100% parallel blend).
    - Front-Panel Card Visuals: 3 mini vertical LED meters in card header displaying real-time downward gain reduction (cyan) and upward expansion (amber) for Low, Mid, and High bands.
    - Right-Click Inspector Popover Deep Dive:
      - **Profiles**: `[ Classic OTT (Default) | Analog Farm Iron (Warm) | Dark & Heavy (Sub/808) ]`.
      - **Independent Dynamics**: Separate Upward Comp % and Downward Comp % unlinked sliders.
      - **Crossover Tuning**: Adjustable Low-Mid (100 Hz - 500 Hz) and Mid-High (1.5 kHz - 6.0 kHz) crossover frequencies.
      - **Noise Floor Gate**: Silence threshold (Off, -60 dB, -48 dB) preventing analog noise amplification during note decays.
      - **DOOM Push Switch**: +12 dB input drive clamped into a 0 dBFS soft-clipping saturation ceiling.
  - [ ] **1.8: `TheKlangMillBlock : public DSPBlock` (Algorithm 28 — "The Klang Mill" Nested Multi-FX Sub-Rack Container):**
    - Recursive multi-effect container allowing an entire 6-slot Klang Mill pedalboard chain to be embedded inside a single FX card slot ("Mill-ception").
    - Front-Panel 4-Knob Layout:
      - Knob 0: **Macro 1** (Assignable sub-macro mapped to internal parameters).
      - Knob 1: **Macro 2** (Assignable sub-macro).
      - Knob 2: **Macro 3** (Assignable sub-macro).
      - Knob 3: **Macro 4 / Mix** (Assignable macro or Universal Dual-Mode Mix).
    - Popover Inspector: Clicking the card's edit button opens a spacious floating window rendering the full 6-slot pedalboard rack where sub-effects can be inserted, reordered, and macro-mapped.
    - Nesting Architecture: Follows the Kilohearts Snap Heap model—arbitrary nesting depth constrained only by host CPU performance, with recursion guardrails to prevent circular feedback locks.
  - [ ] **1.9: Engine Factory Registry:**
    - Update `ModularDrumEngine::createFXBlock(int type)` to instantiate cases 14..28.
- **Verification Condition:** Code compiles cleanly with zero warnings under Clang/MSVC, all static buffers pre-allocated, zero memory allocations in process callbacks.

---

### Phase 2: Engine Integration & Parameter Layout (`source/PluginProcessor.*`)

- **Target Files:** `source/PluginProcessor.h`, `source/PluginProcessor.cpp`
- **Action Items:**
  - [ ] **2.1: Expand `fxChoices` List:**
    - Add `"Transient Shaper"`, `"Custom Waveshaper"`, `"Channel Mixer"`, `"Stereo Enhancer"`, `"Haas Delay"`, and `"Gated Reverb"` to `fxChoices` (indices 14..26).
  - [ ] **2.2: Slot Defaults Initialization:**
    - Update `setFXSlotDefaults(int slotIndex, bool isPost, int type)` in `PluginProcessor.cpp` and `PluginEditor.cpp` with musical default values for effects 14..26.
  - [ ] **2.3: Universal Mix Standardization Audit:**
    - Verify that all effects utilizing a Mix control (Drive, Comb Filter, Chorus, Flanger, Phaser, Tempo Delay, Waveshaper, Haas Delay, Gated Reverb) uniformly map to Knob 4 with bipolar `-100%` ↔ `0%` ↔ `+100%` dual-mode behavior.
- **Verification Condition:** APVTS initializes without crashes, parameters bind to slots 1..4 correctly, and parameter value trees save/restore cleanly.

---

### Phase 3: UI Controls, Custom Formatting & Tooltips (`source/UIComponents.*`, `source/PluginEditor.*`)

- **Target Files:** `source/UIComponents.h`, `source/UIComponents.cpp`, `source/PluginEditor.h`, `source/PluginEditor.cpp`
- **Action Items:**
  - [ ] **3.1: Tooltip & Metadata Helper Additions (`source/UIComponents.cpp`):**
    - Add cases 14..26 to `TooltipHelper::getFxAlgorithmTooltip(int fxIndex)` with rich descriptions.
    - Add cases 14..26 to `TooltipHelper::getFxKnobTooltip(int fxIndex, int knobIndex)` specifying parameter titles, descriptions, default values, and bipolar flags.
  - [ ] **3.2: Custom Text Formatters & Parsers (`source/PluginEditor.cpp`):**
    - Add `formatHaasDelay(double val)`: displays `-XX.X ms (L)` / `Center (0 ms)` / `+XX.X ms (R)`.
    - Add `formatGateTime(double val)`: displays musical beat divisions (`1/64`, `1/32`, `1/16T`, `1/16`, `1/8`, etc.).
    - Add `formatWidthPct(double val)`: displays `0% (Mono)` up to `600% (Super-Wide)`.
    - Add `formatMatrixGain(double val)`: displays `-100%` to `+100%`.
  - [ ] **3.3: `FXSlotCardComponent::configureForType` Wiring:**
    - [ ] **3.4: Categorized FX Selection Modal (Kilohearts-Style Browser):**
      - Full 5-column categorized modal browser overlay replacing linear popup menus for all 26 algorithms.
      - Column categories (alphabetized within): Dynamics & Gain, Filters & Tone, Modulation & Pitch, Delay & Space, Lo-Fi & Character.
      - Left-click on FX card header launches browser modal; `<` / `>` steppers continue to cycle sequentially.
      - Real-time tooltips/descriptions on hover with active selection indicator dot.
    - Add cases 14..26 in `source/PluginEditor.cpp` to bind knob names, accent colors, bipolar detents, formatters, and diagram types to the card UI.
- **Verification Condition:** Switching to any of the 6 new effects renders proper labels, formatting readouts, and tooltips across all 4 knobs.

---

### Phase 4: Automated Testing & DSP Validation (`test/dsp_tests.cpp`)

- **Target Files:** `test/dsp_tests.cpp`
- **Action Items:**
  - [ ] **4.1: Multi-Rate Stability & Audio Energy Test:**
    - Instantiate each new effect (types 14..26) at $44.1\,\text{kHz}$, $48\,\text{kHz}$, and $96\,\text{kHz}$.
    - Feed impulse and sine test signals; assert zero `NaN`, zero `Inf`, and expected RMS gain changes.
  - [ ] **4.2: Channel Mixer & Stereo Width Phase Verification:**
    - Verify `ChannelMixerBlock` cross-feed mathematics ($L \to R$ and $R \to L$ isolation).
    - Verify `StereoEnhancerBlock` produces pure mono when Width = 0% and exaggerated side phase when Width = 600%.
  - [ ] **4.3: Haas Delay Buffer Bounds Check:**
    - Test maximum delay modulation at $96\,\text{kHz}$ to verify zero circular buffer overrun.
  - [ ] **4.4: Gated Reverb Beat Sync & Silence Verification:**
    - Verify that after the gate window expires, the tail drops strictly to zero.
- **Verification Condition:** `dsp_tests` passes 100% across Clang, GCC, and MSVC with zero test failures.
