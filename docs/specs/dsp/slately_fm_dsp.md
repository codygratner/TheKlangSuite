# Technical DSP Specification: `SlatelyFM` (4-Operator Linear FM Synthesizer Core)

**Engine ID:** `CARD-01` / `OSC-01`  
**Sovereign Name:** `SlatelyFM`  
**Corporate Boring Name:** `4-Operator Linear FM Synthesizer`  
**Category:** Synthesizer Oscillator Core  
**APVTS Prefix:** `slately_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section B](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-b)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Yamaha TX81Z (Preset C15 `LatelyBass` & `SolidBass`), Yamaha DX100, Dirtywave M8 `FMSYNTH`.
* **Upstream Offline Reference:** [m8_shortcut_guide.md](file:///C:/Dev/Research/library/manuals/m8_shortcut_guide.md) & [02_dirtywave_m8_complete_offline_reference.md](file:///C:/Dev/Research/library/02_dirtywave_m8_complete_offline_reference.md).
* **C++ Implementation Header:** [`SlatelyFM.h`](file:///c:/Dev/TheKlangDSP/include/dsp/oscillators/SlatelyFM.h).

---

## 2. Mathematical Theory & DSP Architecture

`SlatelyFM` implements a phase modulation engine featuring 4 phase-accumulating operators ($A, B, C, D$). Unlike classic analog exponential FM which causes wild pitch drift, linear phase modulation guarantees absolute pitch stability across extreme modulation indices.

### Phase Accumulator & Instantaneous Output
For operator $i \in \{A, B, C, D\}$ with fundamental frequency $f_0$, sampling frequency $f_s$, and frequency ratio $R_i$:
$$\Delta \theta_i = \frac{f_0 \cdot R_i}{f_s}$$
$$\theta_i[n] = \left( \theta_i[n-1] + \Delta \theta_i \right) \pmod{1.0}$$

The operator audio output $y_i[n]$ with waveform function $W(\cdot)$ and output level $A_i \in [0, 1]$ is governed by:
$$y_i[n] = A_i \cdot W\left(2\pi \theta_i[n] + \sum_{j} I_{ji} y_j[n] + \beta_i y_i[n-1]\right)$$
where $I_{ji}$ represents the modulation index matrix determined by the active algorithm, and $eta_i$ is the self-feedback coefficient.

### Ratio Quantization & Inharmonic Snap
Knobs 1..4 operate in two switchable regimes via encoder push action:
1. **Harmonic Integer Snap:** Snaps to pure musical integers ($1, 2, 3, 4, 5, \dots, 32$) for stable harmonic timbres (bells, organs, Rhodes, basses).
2. **Inharmonic Fractional:** High-resolution fractional tuning (e.g. $08.45, 14.12$) for metallic clusters, cowbells, and industrial noise.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`RATIO A`):** Operator A ratio ($0.25 \dots 32.00$, Profile B). *Push: Integer Snap vs Inharmonic Decimal*.
* **Knob 2 (`RATIO B`):** Operator B ratio ($0.25 \dots 32.00$, Profile B). *Push: Integer Snap vs Inharmonic Decimal*.
* **Knob 3 (`RATIO C`):** Operator C ratio ($0.25 \dots 32.00$, Profile B). *Push: Integer Snap vs Inharmonic Decimal*.
* **Knob 4 (`RATIO D`):** Operator D ratio ($0.25 \dots 32.00$, Profile B). *Push: Integer Snap vs Inharmonic Decimal*.
* **Knob 5 (`LEVEL A`):** Operator A Level ($0.0\% \dots 100.0\%$, Profile B/E). *Push: Mute/Solo Op A*.
* **Knob 6 (`LEVEL B`):** Operator B Level ($0.0\% \dots 100.0\%$, Profile B/E). *Push: Mute/Solo Op B*.
* **Knob 7 (`LEVEL C`):** Operator C Level ($0.0\% \dots 100.0\%$, Profile B/E). *Push: Mute/Solo Op C*.
* **Knob 8 (`LEVEL D`):** Operator D Level ($0.0\% \dots 100.0\%$, Profile B/E). *Push: Mute/Solo Op D*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Algorithm Matrix):** 12 Topologies (`01: Cascade A▶B▶C▶D`, `02: Dual 2-Op`, `04: Hat Sizzle`, `08: Crash Shimmer`, `10: 2-Op R-1 Core`, `11: All-Parallel Organ`).
* **DIP 2 (Global Waveform Palette):** `[ Pure Sine | TX81Z 8-Wave | M8 Cymbal Hybrid | Chiptune Square | Saw ]`.
* **DIP 3 (Frequency Scaling):** `[ 1V/Oct Keytrack All | Fixed Drum Hz All | Fixed Op A + Tracked B/C/D ]`.
* **DIP 4 (Phase Reset):** `[ Hard Reset 0° Punch Snap | Free-Running DCO | Random Phase Scatter ]`.
* **TRIM 1 (Op A Feedback):** $0 \dots 100\%$ self-feedback loop drive on Operator A.
* **TRIM 2 (Op D / Cross Feedback):** $0 \dots 100\%$ second feedback loop or carrier saturation.
* **TRIM 3 (Detune Spread):** Micro-drift detune spread across background modulators.
* **TRIM 4 (Velocity Sensitivity):** Dynamic velocity scaling to modulator levels.

---

## 4. Contextual Dynamic Mode Matrix
When **Algorithm 10 (`2-Op R-1 / KlangCore`)** is active:
* `Knob 3` &rarr; `MOD SHAPE` (Sine, Tri, Square, Noise).
* `Knob 4` &rarr; `MOD DECAY` ($10\text{ ms} \dots 20\text{ s}$, Profile A).
* `Knob 7` &rarr; `FEEDBACK` ($0\% \dots 100\%$, Profile F).
* `Knob 8` &rarr; `AIR TILT` ($\pm 100\%$, Profile B).

---

## 5. Audio-Thread Safety & C++ Invariants
* Zero dynamic memory allocations (`noexcept`, no `new`/`malloc` in `process()`).
* SIMD vectorized sinusoidal computation using Padé polynomial approximations or 2048-point linear lookup tables.
* Denormal protection via `FastMath::flushToZero()`.
