# PLAN: Hastur Tri-Phasic Bipolar Width & Bounded Headroom Modulation Overhaul

### Executive Summary
Implement the operator-ratified Tri-Phasic Bipolar Width Engine on the Master WIDTH dial (`global_width`) alongside the SOTA `boundedShift` modulation architecture across Hastur.

1. **Bipolar Width Dial ($W \in [-1, +1]$)**:
   - **Noon ($W = 0.0$)**: 100% Stadium Stereo ($S = 1.0$, $O = 1.0$, stereo quadrature Alpha Chorus).
   - **7:00 Min ($W < 0.0 \to -1.0$)**: Coherent True Mono ($S \to 0$, $O \to 0$ even filter cutoffs, constant-power rotated in-phase mono chorus).
   - **5:00 Max ($W > 0.0 \to +1.0$)**: Center-Panned Dual-Peak Mono ($S \to 0$, filters remain 100% split $O = 1.0$, stereo chorus panned center with rich comb/flange coloration).
2. **Constant-Power Alpha Chorus Rotation**: Replaces polarity-interpolation with unit-norm rotation vector $(-c_T, s_T)$, eliminating the mid-dial dead hole at $W = -0.5$.
3. **Bounded Headroom Modulation (`boundedShift`)**:
   - Morph LFO: smooth headroom scaling replacing triangle-fold direction reversals.
   - Filter LFO (Cutoff + Orbit): clamped semitone headroom $[10\text{ Hz}, 0.45 f_s]$ with $\pm 60\text{st}$ cap; smooth unclipped orbit swing.
   - X-Mod FM: $k_{\text{max}} = 3.0$ ratio cap with $1\text{ Hz}$ floor, eliminating phase stalls.

---

### Phase 1: Contract & Parameter Codegen
- [ ] Update `global_width` in `contracts/hastur.yaml` to `range: [-1.0, 1.0]`, default: `0.0`.
- [ ] Run `python tools/codegen_apvts.py -p hastur` to synchronize `HasturParameterIds.h` and `HasturParameterLayout.h`.
- [ ] Update `setupRotaryKnob(globalWidthKnob, ...)` in `HasturEditor.cpp` with center detent return (`0.0`) and dynamic text readout.

### Phase 2: Core DSP Bipolar Width, Constant-Power Chorus & Bounded Modulation
- [ ] In `HasturProcessor.cpp`:
  - [ ] Connect `global_width` to `voice_.processSampleStereo(...)` master spread.
  - [ ] Add mono-channel write protection for DAW mono tracks.
- [ ] In `HasturVoice.h`:
  - [ ] Add 10ms one-pole parameter smoother for $W$, computing $S$ and $O$.
  - [ ] Implement `boundedShift` helper function.
  - [ ] Apply `boundedShift` to Morph LFO (`dco_morph`).
  - [ ] Apply `boundedShift` and $O$ scaling to Filter Orbit (`filter_orbit`).
  - [ ] Apply clamped semitone headroom to Filter Cutoff LFO.
  - [ ] Apply constant-power trigonometric rotation to Alpha Chorus wet-R channel.
  - [ ] Apply $S$ to Master Mid/Side stage.
- [ ] In `ModMatrix.h`:
  - [ ] Apply ratio cap and $1\text{ Hz}$ floor to linear oscillator FM.
- [ ] Run `python tools/context_bus_bridge.py build --target dsp_tests --dispatch` in The Tower. Assert all test suites pass.

### Phase 3: Headless UI & Spatial Radar Gate
- [ ] Compile `Hastur_Preview` via Tower build bridge.
- [ ] Run `python tools/ui_gate.py --plugin hastur --pillars A,C`.

### Phase 4: VST3 Packaging & Hot-Swap Deployment
- [ ] Compile target `Hastur_VST3` via Tower build bridge.
- [ ] Deploy binary to `C:\Program Files\Common Files\VST3\Hastur.vst3`.
- [ ] Run `python tools/plan_manager.py finish`.
