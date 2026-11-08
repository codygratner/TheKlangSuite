# Technical DSP Specification: `Argent Reactor` (Time-Woven Demonic Filter Matrix & Chaotic Delay)

**Engine ID:** `FX-15`  
**Sovereign Name:** `Argent Reactor`  
**Corporate Boring Name:** `Time-Woven Dual Resonant Filter Matrix & Chaotic Delay`  
**Category:** Chaotic Filter Matrix & Demonic Resonator  
**APVTS Prefix:** `areactor_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section D](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Jomox T-Resonator / T-Resonator II (Jürgen Michaelis, Berlin, 2008), Mick Gordon DOOM "Instrument of Doom" analog feedback network.
* **Upstream Offline Reference:** [`jomox_t_resonator_manual.md`](file:///C:/Dev/Research/library/manuals/jomox_t_resonator_manual.md) · [`UPSTREAM_REFERENCE_INDEX.md` Section 1.7](file:///C:/Dev/Research/library/manuals/UPSTREAM_REFERENCE_INDEX.md#17-hardware-stereo-resonators--chaotic-filter-matrices-1).
* **C++ Implementation Header:** [`ArgentReactor.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/ArgentReactor.h).

---

## 2. Mathematical Theory & DSP Architecture

The `Argent Reactor` reconstructs and radically amplifies the chaotic feedback topology of the Jomox T-Resonator II, fusing two discrete 24dB transistor ladder filters, audio-rate cross-frequency modulation, cross-coupled phase mixing, and two digital delay lines woven into an analog-modeled saturation loop.

```
       [ Input Audio ] (DR-220A, 808, Synth, Guitar)
              │
              ▼
   ┌──►[ Argent Burn / Input Diode Saturation ]
   │          │
   │    ┌─────┴────────────────────────┐
   │    ▼                              ▼
   │ [ Peak Envelope ]           [ Sine LFO ]
   │   Follower                  0.15Hz - 22Hz
   │    │                        (Transient Retrigger)
   │    └─────────────┬────────────────┘
   │                  ▼
   │          [ Env<LFO Crossfader ]
   │                  │ (Bipolar Env Amt: Plunging vs Soaring)
   │                  ▼
   │    ┌─────────────┴────────────────┐
   │    ▼                              ▼
   │ [ 24dB Ladder: Plasma L ]   [ 24dB Ladder: Plasma R ]
   │    Bipolar: Sub-Quake ◄► Whistle  Bipolar: Sub-Quake ◄► Whistle
   │    ├─── Cross-Phase Mix L►R ──────►│ (Phase Cancellation / Doubling)
   │    │◄── Cross-Phase Mix R►L ───────┤
   │    ├─── Audio-Rate Cross-FM L►R ──►│ (Metallic Laser Screech)
   │    │◄── Audio-Rate Cross-FM R►L ───┤
   │    │                              │
   │    ▼                              ▼
   │ [ Delay In L (±Phase) ]     [ Delay In R (±Phase) ]
   │    │                              │
   │    ▼                              ▼
   │ ┌─────────────────────────────────────────┐
   │ │   8 Argent DSP Delay/Reverb Programs   │
   │ │  (Bone Waveguide, Slap, Void Cavity)   │
   │ └──────────────────┬──────────────────────┘
   │                    │
   └─── Loop Feedback ──┘ (Filtered audio dumped BACK into delay inputs!)
```

### 1. Dual 24dB Transistor Ladder Core with Bipolar Resonance
The filter core implements two independent 4-pole Zero-Delay Feedback (ZDF) transistor ladders:
$$H(s) = \left( \frac{\omega_c}{s + \omega_c} \right)^4$$

Each ladder stage incorporates non-linear differential pair saturation:
$$v_{out} = V_T \cdot \tanh\left( \frac{v_{in}}{2 V_T} \right)$$
Calculated via `FastMath::FastTanh` for real-time vectorization.

#### The Bipolar Resonance Mechanism:
Resonance feedback $k_{res} \in [-1.2, +1.2]$ is fully bipolar:
* **Positive Feedback ($k_{res} > 0$):** Standard 4-pole resonance, creating a sharp, whistling high-Q resonant peak that easily self-oscillates into pure sine tones.
* **Negative Feedback ($k_{res} < 0$, "Godzilla Mode"):** Inverts the feedback phase, destabilizing the integrator chain into sub-harmonic frequency division. When driven by transient pulses (e.g. Roland TR or Boss DR-220A kicks and snares), the filter breaks into an earth-shaking sub-bass drone ($20\,\text{Hz} \dots 50\,\text{Hz}$).

### 2. Cross-Coupled Phase Mixing Matrix
The audio outputs of the two filters cross-inject into the opposite filter inputs with variable phase:
$$x_1[n] = x_{in,1}[n] + M_{21} \cdot y_2[n]$$
$$x_2[n] = x_{in,2}[n] + M_{12} \cdot y_1[n]$$
Where $M_{12}, M_{21} \in [-1.0, +1.0]$.
* **Counter-Phase ($M_{12} = -M_{21}$):** Destructive phase cancellation hollows out the body of the sound, leaving razor-sharp, screaming comb-filter teeth.
* **In-Phase ($M_{12} = M_{21} > 0$):** Phase doubling reinforces low-frequency mass and creates massive analog weight.

### 3. Audio-Rate Cross-Frequency Modulation (Cross-FM)
Filter 1's audio output frequency-modulates Filter 2's cutoff frequency, and vice versa:
$$\omega_{c1}[n] = \omega_{base1} \cdot 2^{FM_{21} \cdot y_2[n]}$$
$$\omega_{c2}[n] = \omega_{base2} \cdot 2^{FM_{12} \cdot y_1[n]}$$
Because the modulators are the self-oscillating audio signals themselves, this produces wild, non-linear sidebands, metallic clangor, chainsaw buzzes, and chaotic attractors.

### 4. Time-Woven Digital Delay Matrix (8 Argent Programs)
The digital delay section features two independent delay lines woven into the analog loop:
* **`Delay In (1+2)`**: Injects the delay outputs into the analog filters with positive or negative phase.
* **`Delay Feedback (1+2)`**: Routes the filtered, saturated analog output **back into the digital delay inputs**, repeatedly degrading and coloring every echo.

| Program | Name | Topology | Para 1 | Para 2 | LFO Target |
| :---: | :--- | :--- | :--- | :--- | :--- |
| **01** | **Plasma Chorus** | Stereo detuned BBD chorus + dark chamber | Chorus Rate | Reverb Mix | Chorus Intensity |
| **02** | **Acid Wash Flanger** | Negative zero-crossing comb flanger | Flange Rate | Reverb Mix | Flange Depth |
| **03** | **Bone Waveguide I** | 4 micro-delay lines ($0.6\,\text{ms} \dots 30\,\text{ms}$) | Pipe 1 Length | Pipe 2 Length | Pipe 3+4 Length |
| **04** | **Hell Waveguide II** | 6 tuned delay pipes (5th & Octave ratios) | Pipe 1 ($0.6 \dots 30\,\text{ms}$) | Pipe 2 ($0.3 \dots 15\,\text{ms}$) | Pipe 1-6 Master Pitch |
| **05** | **Stygian Delay I** | Dual rhythmic delays ($3\,\text{ms} \dots 500\,\text{ms}$) | Delay 1 Time | Delay 2 Time | Delay Flutter |
| **06** | **Doom Slap / Grinder** | Asymmetric delay ($1\,\text{s}$ delay L + $20\,\text{ms}$ slap R) | Delay 1 ($3\,\text{ms} \dots 1000\,\text{ms}$) | Delay 2 ($0.6\,\text{ms} \dots 60\,\text{ms}$) | Slap Comb Depth |
| **07** | **Cathedral of Flesh** | Non-linear dense plate reverb tank | HP Filter | Reverb Decay Time | LP Damping Filter |
| **08** | **Void Singularity** | Infinite freeze-reverb with runaway loop limiter | HP Filter | Reverb Size / Density | Feedback Freeze Loop |

### 5. Dynamic Transient Envelope & Audio-Retriggerable Sine LFO
* **Peak Detector:** Analyzes incoming transient energy with instantaneous attack and variable decay.
* **Audio-Retriggered Sine LFO:** Frequency range $0.15\,\text{Hz} \dots 22\,\text{Hz}$. Sharp transient peaks reset the LFO phase accumulator to zero, aligning modulation bursts with drum strikes.
* **`Env<LFO` Crossfader:**
  * **Turning Left (Envelope-Masked LFO):** The envelope dynamics multiply the LFO amplitude. Drum transients trigger high-intensity modulation bursts that naturally decay into stillness.
  * **Center:** Modulation depth = $0$.
  * **Turning Right (Free LFO):** Un-gated, continuous sinusoidal modulation.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders (Performance View: 1..8)
> 🎛️ **Performance Focus Invariant**: Controls 1..8 are strictly hands-on, live performance controls. A performer using an 8-knob controller (or Page 1 on ToadTracker) can execute a complete, dynamic live set (filter sweeps, sub-bass earthquakes, DOOM overdrive, dub echoes, and runaway feedback) without ever needing to flip to Page 2!

| # | Parameter ID | Display Label | Control Type | Push / Click Action | Taper Profile | Default | Min / 12:00 / Max | Units & Display | Musical Function |
| :-: | :--- | :--- | :--- | :--- | :--- | :-: | :--- | :--- | :--- |
| **01** | `areactor_cutoff_l` | `CUTOFF L` | `Continuous` | `Snap 1 kHz / Link L+R` | `Profile D` | `800 Hz` | `20Hz / 800Hz / 20kHz` | `Hz / kHz` | Left 24dB ladder filter cutoff |
| **02** | `areactor_cutoff_r` | `CUTOFF R` | `Continuous` | `Snap 1 kHz / Invert Sweep` | `Profile D` | `800 Hz` | `20Hz / 800Hz / 20kHz` | `Hz / kHz` | Right 24dB ladder filter cutoff |
| **03** | `areactor_res_l` | `RES L (±)` | `Continuous Bipolar` | `Zero Res (0%)` | `Profile B` | `0.0` | `-100% / 0% / +100%` | `%` | Left bipolar resonance: Godzilla sub-quake $\leftrightarrow$ whistle |
| **04** | `areactor_res_r` | `RES R (±)` | `Continuous Bipolar` | `Zero Res (0%)` | `Profile B` | `0.0` | `-100% / 0% / +100%` | `%` | Right bipolar resonance: Godzilla sub-quake $\leftrightarrow$ whistle |
| **05** | `areactor_burn` | `ARGENT BURN` | `Continuous` | `+6 dB Boost` | `Profile E` | `0 dB` | `0dB / +6dB / +24dB` | `dB` | Input diode saturation & DOOM overdrive gain |
| **06** | `areactor_time` | `TIME / PARA 1` | `Continuous` | `Tap / Sync Tempo` | `Profile C` | `120 ms` | `0.6ms / 120ms / 1000ms` | `ms / s` | Primary delay time / acoustic waveguide pitch |
| **07** | `areactor_feedback` | `LOOP FEEDBACK` | `Continuous` | `Infinite Hold Latch` | `Profile E` | `30%` | `0% / 50% / 130%` | `%` | Analog filter re-injection back into digital delay |
| **08** | `areactor_mix` | `MIX` | `Continuous Tri-Zone` | `Phase Invert (Ø=180°)` | `Profile M` | `0.5` | `100%W / DRY / PAR` | `Mix` | Tri-Zone wet/dry with internal safety limiter clamp |

---

### Rear Deck ⚙️: 8 Sound Lab Controls (Settings View: 9..16)
> ⚙️ **Settings Focus Invariant**: Controls 9..16 house the machine room, circuit topologies, algorithm selectors, and fine calibrations:
> * **Controls 9..12 (DIP 1..4 / Selectors)**: Discrete algorithm choice, injection phase routing, modulation trigger modes, and limiter ceilings.
> * **Controls 13..16 (TRIM 1..4 / Precision Trimpots)**: Secondary decay parameters, cross-phase balance, cross-FM intensity, and LFO rate/dynamics.

#### 4 Digital DIP Switches / Selectors (Controls 9..12):
* **`DIP 1 / Control 09 (Algorithm / Mode Topology)`**:  
  `[ 01: Plasma Chorus | 02: Acid Wash Flange | 03: Bone Waveguide I | 04: Hell Waveguide II | 05: Stygian Delay I | 06: Doom Slap / Grinder | 07: Cathedral of Flesh | 08: Void Singularity ]`
* **`DIP 2 / Control 10 (Delay Injection Routing & Phase)`**:  
  `[ Dual In-Phase (+Phase) | Anti-Phase (-Phase Comb Cancel) | Cross-Inject (Delay L▶Flt R, Delay R▶Flt L) | Post-Filter Parallel ]`
* **`DIP 3 / Control 11 (Modulation Sync & Trigger Mode)`**:  
  `[ Audio Transient Retrigger (Drum Peak Reset) | Free-Running Sine | Tempo Sync (1/16 Beat) | Envelope Follower Direct ]`
* **`DIP 4 / Control 12 (Argent Limiter & Saturation Model)`**:  
  `[ Argent Soft-Knee (+6dBFS Safe) | DOOM BJT Diode Carnage (Hard Clip) | Germanium Sag & Voltage Droop | Unbounded (DANGER / Passthrough) ]`

#### 4 Precision Trimpots (Controls 13..16):
* **`TRIM 1 / Control 13 (Cross-Phase Coupling)`**: `areactor_cross_phase` | **Range:** $-100\% \dots 0\% \dots +100\%$ | **Profile B**  
  *Bipolar cross-phase mixing between filter outputs and opposite inputs. Left: Destructive phase comb teeth; Right: In-phase sub-bass doubling.*  
  *Push Action: Toggle Symmetrical ($L = R$) vs Antiphase ($L = -R$).*
* **`TRIM 2 / Control 14 (Cross-FM Intensity)`**: `areactor_cross_fm` | **Range:** $0\% \dots 100\%$ | **Profile E**  
  *Audio-rate frequency modulation intensity between filter cores. Tunnels into non-linear sidebands, metallic shrieks, and chaotic attractors.*  
  *Push Action: Toggle Dual-Symmetric (L mod R & R mod L) vs One-Way (L mod R only).*
* **`TRIM 3 / Control 15 (Decay / Para 2)`**: `areactor_para2` | **Range:** $0\% \dots 100\%$ | **Profile C**  
  *Secondary delay parameter: sets reverb tail decay time, secondary delay spacing, or physical waveguide pipe 2 length.*  
  *Push Action: Reset Default ($50\%$).*
* **`TRIM 4 / Control 16 (LFO Rate & Dynamics)`**: `areactor_lfo_rate` | **Range:** $0.15\,\text{Hz} \dots 22\,\text{Hz}$ | **Profile A**  
  *Sets sine LFO modulation speed ($7\,\text{s} \dots 45\,\text{ms}$).*  
  *Push Action: Toggle Envelope-Masked Burst Mode (drum transient burst that decays) vs Free-Running Continuous Sine.*

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Active Program | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`03 & 04: Waveguide I & II`** | `Knob 06` / `TRIM 3` | `PIPE LENGTH 1 / 2` | $0.6 \dots 30\,\text{ms}$ | `Profile C` | Tunes resonant acoustic physical model pitch |
| **`07: Cathedral of Flesh`** | `TRIM 3` | `REVERB DECAY` | $0.1 \dots 30\,\text{s}$ | `Profile A` | Non-linear dense plate decay time |
| **`08: Void Singularity`** | `TRIM 3` | `SINGULARITY` | $0 \dots 100\%$ | `Profile B` | Controls runaway non-linear density collapse |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Zero Allocations & Real-Time Safety:** All delay buffers, waveguide delay lines, and state arrays are pre-allocated in `prepareToPlay()`. Zero dynamic allocations (`new`, `vector::resize`, `malloc`) on audio thread.
* **FastMath Saturation:** All four integrator stages in each ladder filter utilize `FastMath::FastTanh` to prevent numerical divergence under runaway positive feedback.
* **Monitor Saver Hard Clamp:** Bounded feedback loop incorporates a soft-knee saturation ceiling at $+6.0\,\text{dBFS}$ to guarantee that unstable self-oscillations cannot clip digital converters or damage speakers.
