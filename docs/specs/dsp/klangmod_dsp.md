# Technical DSP Specification: `KlangMod` (Chorus, Ensemble & Modulation Lab)

**Engine ID:** `MEGA-05`  
**Sovereign Name:** `KlangMod`  
**Corporate Boring Name:** `Multi-Voice Chorus, BBD Ensemble & Phase Modulation Workstation`  
**Category:** Flagship Studio Modulation Workstation  
**APVTS Prefix:** `kmod_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section D](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Roland Dimension D (SDD-320), Boss CE-1 Chorus Ensemble, Solina String Ensemble (triple BBD bucket brigade), Mu-Tron Bi-Phase, Fender Brownface Harmonic Tremolo.
* **Upstream Offline Reference:** [roland_dimension_d_sdd320_manual.md](file:///C:/Dev/Research/library/manuals/roland_dimension_d_sdd320_manual.md) · [ROSETTA_STONE.md#section-d](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d).
* **C++ Implementation Header:** [`KlangModLab.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/KlangModLab.h).

---

## 2. Mathematical Theory & DSP Architecture

KlangMod unifies time-domain, optical, and phase-delay modulation into a flexible multi-voice workstation:
1. **Four Specialized Modulation Engines:**
   - **Engine 1 (Dimension Spatial):** Dual antiphase BBD chorus with zero comb-filtering cancellation in mono.
   - **Engine 2 (Solina Triple Ensemble):** Three delay lines driven by two independent LFOs (Slow $0.6\,\text{Hz}$ + Fast $6.0\,\text{Hz}$) locked at $120^\circ$ phase angles, producing the legendary lush string ensemble texture.
   - **Engine 3 (Optical Phaser):** 6-stage to 12-stage swept optical all-pass filter cascade with sluggish vactrol decay.
   - **Engine 4 (Harmonic Tremolo):** Crossover-based phase vibrato and auto-panning.
2. **True BBD Compander & Clock Emulation:**
   Includes NE570 compander dynamic compression and variable anti-aliasing reconstruction filters.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`RATE`):** Modulation LFO speed ($0.05\,\text{Hz} \dots 20.0\,\text{Hz}$ / Host Sync, Profile C). *Push: Tempo Sync Division*.
* **Knob 2 (`DEPTH`):** Modulation depth ($0.0\% \dots 100.0\%$, Profile B). *Push: Zero Depth (Bypass Mod)*.
* **Knob 3 (`FEEDBACK`):** Regeneration feedback ($\pm 100.0\%$, Profile F). *Push: Feedback Invert*.
* **Knob 4 (`ENGINE`):** 8 algorithms (Clean Digital Stereo, Vintage Juno BBD, Swept Hoover Chorus, 16-Voice Hyper-Ensemble, Dimension D Spatial, Through-Zero Flanger, 8-Stage Optical Phaser, Harmonic Tremolo). Discrete `0..7`. *Push: Next Engine*.
* **Knob 5 (`PRE-DELAY`):** Delay time center offset ($0.0\,\text{ms} \dots 25.0\,\text{ms}$, Profile A). *Push: Snap 0ms Through-Zero*.
* **Knob 6 (`TONE / COLOR`):** Analog BBD high-cut damping ($1.0\,\text{kHz} \dots 18.0\,\text{kHz}$, Profile D Log). *Push: Juno Warm vs Clean Modern*.
* **Knob 7 (`STEREO WIDTH`):** Spatial spread ($0.0\% \dots 200.0\%$, Profile B). *Push: True 180° Dimension Split / Mono Check*.
* **Knob 8 (`MIX`):** Universal Tri-Zone Dry/Wet Mix (Profile M: 7:00 Wet Audition, 12:00 Pure Dry, 5:00 Parallel). *Push: Phase Invert (\varnothing = 180^\circ)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (BBD Stage Model):** `[ MN3007 1024-Stage | MN3009 256-Stage Fast | Reticon SAD1024 | Clean Digital ]`.
* **DIP 2 (LFO Waveshape):** `[ Sine | Linear Triangle | Dual-Phase Quadrature | Hyper-Warble ]`.
* **DIP 3 (Ensemble Voice Count):** `[ 2-Voice Stereo | 3-Voice Solina Tri-Phase | 4-Voice Dimension ]`.
* **DIP 4 (Bass Crossover Trap):** `[ Keep Lows Mono & Dry (<150Hz) | Full Bandwidth Modulation ]`.
* **TRIM 1 (Solina Fast LFO Speed):** Secondary fast vibrato LFO frequency ($4.0\,\text{Hz} \dots 8.0\,\text{Hz}$).
* **TRIM 2 (Vactrol Dark Time):** Photocell decay inertia for the optical phaser engine.
* **TRIM 3 (BBD Clock Bleed Level):** High-frequency clock whine injection level.
* **TRIM 4 (Input Dynamic Expansion):** Compander expansion ratio and recovery rate.

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Active Modulation Engine | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`5: Through-Zero Flanger`**| `Knob 5` | `TZ OFFSET` | `±5.0 ms` | `Profile B` | True 0ms zero-crossing point |
| **`6: 8-Stage Phaser`** | `Knob 3, Knob 5` | `RESONANCE Q`, `POLE STAGES` | `0..100%`, `4..12 Poles` | `Profile F`, `Discrete` | Optical all-pass feedback and stage count |
| **`7: Harmonic Tremolo`** | `Knob 3, Knob 5` | `CROSSOVER FREQ`, `HARMONIC / VINT` | `200..1200Hz`, `0..100%` | `Profile D`, `Profile B` | High/low crossover split & optical morph |

--- | :--- | :--- | :--- | :--- | :--- |
| **`Solina Ensemble`** | `Knob 1` | `SLOW/FAST BLEND`| `0..100%` | `Profile B` | Balances slow 0.6Hz undulation with fast 6Hz shimmer |
| **`12-Stage Phaser`** | `Knob 5` | `OPTICAL REGEN` | `0..100%` | `Profile B` | Controls liquid resonance peak squelch |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Hermite Fractional Delay:** Variable delay lines interpolated using 4-point cubic Hermite splines.
* **Zero Allocations:** Modulation buffers allocated statically at plugin initialization.
