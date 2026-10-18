# Feature: Effects Processors Catalog Expansion, KlangStomp, Spatial Lab & Experimental Granular Suites

## 1. Executive Summary & Cross-Ecosystem Architecture

The effects catalog across **TheKlangDSP** ([`C:\Dev\TheKlangDSP`](file:///C:/Dev/TheKlangDSP)), **TheKlangSuite** ([`C:\Dev\TheKlangSuite`](file:///C:/Dev/TheKlangSuite)), and **ToadTracker** ([`C:\Dev\ToadTracker`](file:///C:/Dev/ToadTracker)) encompasses both the 28-algorithm `ModularBlocks.h` rack (`Algorithms 0..28`) and the 4-Phase C++20 Sovereign DSP Processor Suites (`KlangStomp`, `SoundEngine8`, `Spatial & Modulation Lab`, and `Experimental & Granular Processors`).

### Cross-Referenced Research & Hardware Manuals
- [**Rosetta Stone Master IP Reference**](file:///C:/Dev/Research/library/manuals/ROSETTA_STONE_MASTER_IP_REFERENCE.md) — Canonical mapping between Sovereign Klang names (`SwedishKlangSaw`, `KlangZone`, `Rodent`, etc.) and classic hardware topologies.
- [**KlangStomp Circuit Topologies Deep Research**](file:///C:/Dev/Research/subagents/2026-10-15_klangstomp_circuit_topologies_research.md) — Component-level SPICE/schematic analysis of all 8 iconic analog distortion, overdrive, and fuzz circuits.
- [**Doepfer A-188-1 BBD Manual**](file:///C:/Dev/Research/library/manuals/doepfer_a1881_bbd_manual.md) — Bucket-Brigade Device (MN3005/MN3207) clock modulation, anti-aliasing/reconstruction filtering, and clock bleed dynamics.
- [**Doepfer A-199 Spring Reverb Manual**](file:///C:/Dev/Research/library/manuals/doepfer_a199_spring_reverb_manual.md) — Multi-spring dispersion all-pass cascades and metallic tank feedback topologies (`ArgentVortex` / `PipeAndWire`).
- [**Electrix WarpFactory Vocoder Manual**](file:///C:/Dev/Research/library/manuals/electrix_warpfactory_manual.md) — Formant freeze, gender/frequency shifting, and multi-band envelope-follower vocoding (`Vocoder56K`).
- [**TheKlangDSP Decoupling Spec**](klang_dsp_decoupling.md) & [**Sound Engines Expansion Spec**](sound_engines_expansion.md).

---

## 2. Phase 1: `KlangStomp` — 8 Iconic Component-Modeled Analog Pedals (`include/dsp/fx/klangstomp/`)

All 8 stompboxes inherit from [`KlangDSP::AudioEffect`](file:///C:/Dev/TheKlangDSP/include/dsp/common/AudioEffect.h), operate with zero heap allocations, use rational Padé `FastMath::fastTanh`, flush denormals (`1e-20f`), and clamp outputs to `[-4.0f, 4.0f]`.

| # | Sovereign Class & Header | Classic Alias Shim | Modeled Hardware & Nonlinear Circuit Topology |
| :--- | :--- | :--- | :--- |
| **1** | **`SwedishKlangSaw`** (`SwedishKlangSaw.h`) | `BossHM2` (`BossHM2.h`) | **Boss HM-2 Heavy Metal**: $72\,\text{Hz}$ pre-emphasis HPF $\to$ discrete BJT Stage 1 with unbiased $\pm 0.045\,\text{V}$ crossover dead-band $\to$ dual silicon diode hard clip $\to$ **Dual Parallel Active Gyrator EQ** ($98\,\text{Hz}$ Low resonance & $1050\,\text{Hz}$ + $1.35\,\text{kHz}$ dual-peak Chainsaw resonance, $\pm 20\,\text{dB}$). |
| **2** | **`KlangZone`** (`KlangZone.h`) | `BossMT2` (`BossMT2.h`) | **Boss MT-2 Metal Zone**: Fixed $1\,\text{kHz}$ bandpass pre-emphasis hump ($Q=1.8$) $\to$ cascading Stage 1 asymmetric + Stage 2 hard op-amp clippers $\to$ $4.5\,\text{kHz}$ fizz-taming LPF $\to$ **3-Band Active EQ** ($100\,\text{Hz}$ Low, $5.0\,\text{kHz}$ High, and sweepable $200\,\text{Hz}\text{--}5.0\,\text{kHz}$ Parametric Mid $\pm 15\,\text{dB}$). |
| **3** | **`Rodent`** (`Rodent.h`) | `ProCoRat` (`ProCoRat.h`) | **ProCo RAT**: Variable-bandwidth RC input HPF ($60\,\text{Hz}\text{--}1.5\,\text{kHz}$) $\to$ **LM308 Op-Amp Dynamic Slew-Rate Limiter** ($0.3\,\text{V}/\mu\text{s}$ bounded derivative $\Delta y/\Delta t$) $\to$ hard asymmetric 1N914 silicon diode clipping to ground $\to$ reverse-taper passive "Filter" LPF ($15\,\text{kHz} \to 475\,\text{Hz}$). |
| **4** | **`GreenScreamer`** (`GreenScreamer.h`) | `IbanezTS9` (`IbanezTS9.h`) | **Ibanez TS9 Tube Screamer**: **Active Op-Amp Feedback Loop** with $720.5\,\text{Hz}$ feedback HPF ($R=4.7\,\text{k}\Omega, C=0.047\,\mu\text{F}$) summing unity dry sub-bass with soft-clipped 1N914 midrange ($V_{\text{out}} = V_{\text{in}} + V_{\text{clip}}(V_{\text{HPF}})$) $\to$ active "G- Curve" Tone control ($220\,\text{Hz}\text{--}1\,\text{kHz}$). |
| **5** | **`RussianKlangPi`** (`RussianKlangPi.h`) | `BigMuffPi` (`BigMuffPi.h`) | **EHX Big Muff Pi**: **4-Stage Cascaded Transistor Sustainer** (Input Booster $\to$ Clipping Amp 1 with $100\,\text{Hz}$ interstage coupling & $470\,\text{pF}$ Miller LPF $\to$ Clipping Amp 2 $\to$ Recovery) $\to$ **Passive Mid-Scoop Tone Stack** ($450\,\text{Hz}$ LP vs $1.8\,\text{kHz}$ HP crossfade creating the signature $-10\,\text{dB}$ $1\,\text{kHz}$ scoop). |
| **6** | **`IxionGuild`** (`IxionGuild.h`) | `KlonCentaur` (`KlonCentaur.h`) | **Klon Centaur Professional Overdrive**: **Dual-Ganged Clean/Dirty Summing Potentiometer** (`2x2` inverse law): Path A filters clean bass/mids and scales with $+18\,\text{V}$ charge-pump headroom; Path B high-passes at $400\,\text{Hz}$ into 1N34A germanium hard clipping ($\pm 0.35\,\text{V}$) $\to$ active $1.0\text{--}3.2\,\text{kHz}$ high-shelf Treble boost. |
| **7** | **`Yellow250`** (`Yellow250.h`) | `DOD250` (`DOD250.h`) | **DOD Overdrive Preamp 250 (Grey Spec)**: **LM741 Op-Amp Preamp** with gain-dependent bass-tightening HPF ($35\,\text{Hz} \to 380\,\text{Hz}$ as Gain rises) $\to$ LM741 $0.5\,\text{V}/\mu\text{s}$ slew-rate ceiling $\to$ **Asymmetric 2x-vs-1x 1N914 Silicon Diode Ground Clipper** ($+0.85\,\text{V}$ vs $-0.42\,\text{V}$) generating rich 2nd-order even harmonics. |
| **8** | **`VelcroFace`** (`VelcroFace.h`) | `FuzzFace` (`FuzzFace.h`) | **Dallas Arbiter Fuzz Face**: **Dual-Transistor ($Q_1/Q_2$) Feedback Topology** with input-loading pickup rolloff, massive $2.2\,\mu\text{F}$ emitter bypass gain, and **Starved Collector Bias Control** (`Param 1: Bias` sweeping $Q_2$ supply from $9.0\,\text{V}$ down to $1.5\,\text{V}$ for dynamic dead-zone gating and tearing velcro sputter). |

---

## 3. Phase 3: Spatial & Modulation Lab (`include/dsp/fx/spatial/`)

### 3.1 `KlangMod` — 8-Mode BBD, Ensemble & Phase Modulator (`KlangMod.h`)
`KlangMod` unifies 8 classic analog/digital modulation topologies inside a pre-allocated dual-delay-line engine ($8192$ samples per channel, 3rd-order Hermite fractional interpolation, and authentic MN3005/MN3207 BBD non-linearities per [`doepfer_a1881_bbd_manual.md`](file:///C:/Dev/Research/library/manuals/doepfer_a1881_bbd_manual.md)):
1. **Mode 0 — Clean Digital Stereo Chorus**: Quadrature sine LFO ($90^\circ$ L/R offset), $7\,\text{ms}$ center delay, pristine full-bandwidth interpolation.
2. **Mode 1 — Vintage BBD Chorus (Juno-60 / CE-2)**: Triangle LFO, $4.5\,\text{ms}$ center delay, $8\,\text{kHz}$ anti-aliasing/reconstruction LPF, and soft BBD cell saturation.
3. **Mode 2 — Swept Hoover Chorus (Alpha Juno-2)**: Fast, deep triangle sweep ($3.5\,\text{ms} \pm 3.0\,\text{ms}$) for classic rave/industrial PWM hoover animation.
4. **Mode 3 — 16-Voice Hyper-Ensemble (Solina / String Machine)**: 8 decorrelated dual-rate LFO taps per stereo channel ($0.6\,\text{Hz}$ slow + $5.8\,\text{Hz}$ fast vibrato) for lush symphonic thickening.
5. **Mode 4 — Spatial Dimension BBD (Roland SDD-320 Dimension D)**: Cross-coupled opposite-phase stereo BBD lines with bass-preserving dry/wet crossover compensation.
6. **Mode 5 — Through-Zero Tape/BBD Flanger**: Fixed $2.0\,\text{ms}$ reference dry delay vs $0.1\text{--}4.0\,\text{ms}$ swept wet tap passing directly through $t=0.0\,\text{ms}$ phase cancellation.
7. **Mode 6 — 8-Stage Optical Phaser (Schulte Compact Phasing 'A' / Mutron Bi-Phase)**: 8 cascaded first-order all-pass filters per channel with photoresistor (LDR) attack/release lag asymmetry and resonant feedback.
8. **Mode 7 — Harmonic Tremolo (Vintage Fender Brownface)**: $450\,\text{Hz}$ state-variable crossover splitting Low and High bands into $180^\circ$ out-of-phase amplitude modulation.

### 3.2 `KlangDelay`, `KlangSpace`, `EchoMill`, and `Vocoder56K`
- [**`KlangDelay.h`**](file:///C:/Dev/TheKlangDSP/include/dsp/fx/spatial/KlangDelay.h): Stereo ping-pong, magnetic tape wow/flutter, and BBD dark bucket delay with resonant feedback HPF/LPF damping.
- [**`KlangSpace.h`**](file:///C:/Dev/TheKlangDSP/include/dsp/fx/spatial/KlangSpace.h): Multi-stage all-pass diffuser and modulated feedback delay network (FDN) hall/shimmer reverb.
- [**`EchoMill.h`**](file:///C:/Dev/TheKlangDSP/include/dsp/fx/spatial/EchoMill.h): Recursive insert-loop delay processor ("The Klang Loop Mill") embedding saturation, pitch shifting, and resonant filtering directly inside the regeneration path.
- [**`Vocoder56K.h`**](file:///C:/Dev/TheKlangDSP/include/dsp/fx/Vocoder56K.h): Multi-band channel vocoder and formant shifter inspired by the [**Electrix WarpFactory**](file:///C:/Dev/Research/library/manuals/electrix_warpfactory_manual.md) and Motorola DSP56001 voice processors, featuring formant freeze, gender shift, and internal MKS-50/Hoover carrier synthesis.

---

## 4. Phase 4: Experimental & Granular Processors (`include/dsp/experimental/`)

- [**`KlangResonator.h`**](file:///C:/Dev/TheKlangDSP/include/dsp/experimental/KlangResonator.h) (**4-Voice Modal & Tuned Waveguide Bank**):
  - **8 Parameters**: `Pitch` ($20\,\text{Hz}\text{--}2\,\text{kHz}$), `Structure` (4 tuning topologies), `Decay` ($10\,\text{ms}\text{--}10\,\text{s}$), `Damping` ($500\,\text{Hz}\text{--}18\,\text{kHz}$), `Position` (comb excitation node), `Drive` (exciter saturation), `Spread` (stereo voice panning), `Mix`.
  - **4 Tuning Topologies**:
    1. *Volca Cylindrical Tube*: Harmonic overtones $[1.0, 2.0, 3.0, 4.0]$ with open-pipe phase reflection.
    2. *Volca Open String*: Stiff-string harmonic ratios $[1.0, 1.5, 2.0, 3.01]$.
    3. *Bronze Bell Bessel Modes*: Inharmonic metallic Chladni ratios $[1.0, 1.504, 2.138, 2.761]$.
    4. *Dark Phrygian Cluster*: Semitone/fifth dissonance $[1.0, 1.05946, 1.4983, 2.0]$.
- [**`KlangFreeze.h`**](file:///C:/Dev/TheKlangDSP/include/dsp/experimental/KlangFreeze.h) (**4-Grain Hann-Windowed Cloud Freeze & Scale Quantizer**):
  - Pre-allocated $65,536$-sample stereo circular capture buffer ($\sim 1.36\,\text{s}$ @ $48\,\text{kHz}$).
  - 4 overlapping grains with raised-cosine Hann windowing ($w(\phi) = 0.5(1 - \cos(2\pi\phi))$), randomized position spray, and **4 Pitch Quantization Modes**: Free Continuous ($\pm 24\,\text{st}$), Octaves & Fifths, **Phrygian Dominant** ($\{0, 1, 4, 5, 7, 8, 10\}$), and **Harmonic Minor** ($\{0, 2, 3, 5, 7, 8, 11\}$).
- [**`KlangBode.h`**](file:///C:/Dev/TheKlangDSP/include/dsp/experimental/KlangBode.h) (**Harald Bode Dual $90^\circ$ Hilbert SSB Frequency Shifter**):
  - Dome-style 2x4 cascaded first-order all-pass IIR quadrature network maintaining $90^\circ$ phase separation across $20\,\text{Hz}\text{--}20\,\text{kHz}$.
  - Single-Sideband (SSB) quadrature multiplication ($\text{Up} = I\cos\omega t - Q\sin\omega t$, $\text{Down} = I\cos\omega t + Q\sin\omega t$), Coarse ($\pm 5000\,\text{Hz}$) + Fine ($\pm 50\,\text{Hz}$) linear frequency shift, **Barberpole LFO** mode, and $1\text{--}250\,\text{ms}$ feedback delay spiral.
- [**`ArgentVortex.h`**](file:///C:/Dev/TheKlangDSP/include/dsp/experimental/ArgentVortex.h), [**`PhaseShriek.h`**](file:///C:/Dev/TheKlangDSP/include/dsp/experimental/PhaseShriek.h), and [**`PipeAndWire.h`**](file:///C:/Dev/TheKlangDSP/include/dsp/experimental/PipeAndWire.h):
  - Physical multi-spring dispersion tank ([`doepfer_a199_spring_reverb_manual.md`](file:///C:/Dev/Research/library/manuals/doepfer_a199_spring_reverb_manual.md)), chaotic self-modulating FM shriek distortion, and industrial acoustic pipe/wire resonator.

---

## 5. ModularBlocks Rack Catalog (Effects 14–28 & Universal Dual-Mode Mix)

In addition to the standalone 8-parameter processor suites above, `include/ModularBlocks.h` provides 15 studio-grade, zero-allocation multi-instance rack blocks for `TheKlangFarmer`'s 8-slot FX rack, standardized on **Universal Dual-Mode Mix** (Knob 4: `-100%` Wet Crossfade $\leftrightarrow$ `0%` Dry $\leftrightarrow$ `+100%` Parallel Blend):

1. **`TransientShaperBlock` (Alg 14)**: Dual fast/slow envelope follower (Attack, Pump, Sustain, Speed).
2. **`ThresherBlock` (Alg 15 — "The Thresher" Waveshaper)**: 64+ mathematical transfer curves across 4 banks (Synth Morph, Chebyshev/West Coast Fold, Bit-Crunch/Logic, Tape Hysteresis) + DJ Tilt Filter + DOOM Brickwall mode.
3. **`ChannelMixerBlock` (Alg 16)**: 4-knob cross-channel matrix mixer ($L\to L, R\to L, L\to R, R\to R$).
4. **`StereoEnhancerBlock` (Alg 17)**: Mid/Side field shaper (Mid, Width up to 600%, Pan, Analog Slop drift).
5. **`HaasDelayBlock` (Alg 18)**: Bipolar $-100\,\text{ms}\dots+100\,\text{ms}$ psychoacoustic stereo widener with tone damping.
6. **`GatedReverbBlock` (Alg 19)**: 8-tap 80s early reflections diffuser with BPM-synced gate cutoff timer.
7. **`JunoChorusBlock` (Alg 20)**: 4-state Roland BBD chorus with analog hiss/dirt modeling.
8. **`WaveguideResonatorBlock` (Alg 21)**: Volca Drum physical tube/string waveguide resonator.
9. **`SubGeneratorBlock` (Alg 22)**: Phase-locked $30\text{--}120\,\text{Hz}$ 808 sub-oscillator with harmonic warmth.
10. **`TripleXLBlock` (Alg 23 — "The XXXL")**: Tech 21 XXL-inspired diode-less op-amp rail saturation with independent Odd (Padé tanh) and Even (asymmetric quadratic + $5\,\text{Hz}$ DC blocker) harmonic controls.
11. **`EnvelopeFilterBlock` (Alg 24)**: Transient-following ZDF auto-filter (Cutoff, Bipolar Env Depth, Resonance, Mix).
12. **`PitchTransposerBlock` (Alg 25)**: Dual-grain $\pm 12\,\text{st}$ pitch transposer with regeneration loop.
13. **`StutterGateBlock` (Alg 26)**: Tempo-synced rhythmic chopper with adjustable duty cycle and click smoothing.
14. **`BalerBlock` (Alg 27 — "The Baler" OTT Multiband Dynamics)**: 3-band 4th-order Linkwitz-Riley ($250\,\text{Hz}$ / $2.5\,\text{kHz}$) upward/downward compressor with Classic OTT, Analog Farm Iron, and Dark & Heavy profiles.
15. **`TheKlangMillBlock` (Alg 28 — "The Klang Mill")**: Recursive 6-slot pedalboard sub-rack container with 4 assignable macro knobs.

---

## 6. Verification & Automated Testing (`test/dsp_tests.cpp`)

- **73 / 73 Catch2 Test Cases Passing (36,518 Assertions)** across `C:\Dev\TheKlangDSP\test\dsp_tests.cpp` and `C:\Dev\TheKlangSuite\test\dsp_tests.cpp`.
- Verified across $44.1\,\text{kHz}$, $48\,\text{kHz}$, and $96\,\text{kHz}$ for zero `NaN`/`Inf`, strict `[-4.0f, 4.0f]` / `[-8.0f, 8.0f]` safety clamping, and 100% `audit_audiothread.py --strict` compliance.
