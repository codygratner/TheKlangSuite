# Technical DSP Specification: `Bi-Phase 12` (12-Stage Optical Dual Phaser)

**Engine ID:** `FX-10`  
**Sovereign Name:** `Bi-Phase 12`  
**Corporate Boring Name:** `Dual 6-Stage Optical Swept Phaser`  
**Category:** Optical Modulation & Phase Shifter  
**APVTS Prefix:** `biphase_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section C](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Mu-Tron Bi-Phase (1974, designed by Mike Beigel), two independent 6-stage optical all-pass phaser sections with opto-isolator (vactrol) LDR cells.
* **Upstream Offline Reference:** [mu_tron_bi_phase_manual.md](file:///C:/Dev/Research/library/manuals/mu_tron_bi_phase_manual.md) · [ROSETTA_STONE.md#section-c](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c).
* **C++ Implementation Header:** [`BiPhase12Optical.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/BiPhase12Optical.h).

---

## 2. Mathematical Theory & DSP Architecture

Bi-Phase 12 models twelve first-order all-pass filter stages grouped into two independent 6-pole phasers (Phaser A and Phaser B):
1. **First-Order All-Pass Phase Shift Network:**
   Each optical stage imparts a frequency-dependent phase shift without changing amplitude:
   $$H_{ap}(s) = \frac{s - \omega_0(t)}{s + \omega_0(t)}, \quad H_{ap}(z) = \frac{-a(t) + z^{-1}}{1 - a(t) z^{-1}}$$
   Where $a(t) = \frac{1 - \tan(\pi f_0 / f_s)}{1 + \tan(\pi f_0 / f_s)}$. Cascading 6 stages produces 3 deep phase notches; cascading all 12 stages produces 6 deep notches.
2. **Opto-Coupler (LDR/Vactrol) Non-Linear Physics:**
   Cadmium-sulfide (CdS) photoresistors exhibit asymmetrical temporal response:
   - **Turn-on time (light flare):** Fast decay in resistance ($\tau_{on} \approx 2\,\text{ms}$).
   - **Turn-off time (light decay):** Sluggish memory tail / dark relaxation ($\tau_{off} \approx 50\,\text{ms} \dots 200\,\text{ms}$).
   This produces the organic, rubbery, liquid sweep unique to vintage optical phasers.
3. **Routing Flexibility:**
   Phaser A and Phaser B can be routed in **`Series (12-Stage Monolithic)`**, **`Parallel Stereo (6-Stage Dual Mono)`**, or **`Cross-Feedback Matrix`**.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`RATE A`):** Phaser A LFO modulation frequency ($0.05\,\text{Hz} \dots 18.0\,\text{Hz}$, Profile C). *Push: Tempo Sync*.
* **Knob 2 (`DEPTH A`):** Phaser A sweep width ($0.0\% \dots 100.0\%$, Profile B). *Push: Snap 50%*.
* **Knob 3 (`FEEDBACK A`):** Phaser A resonance / color ($-95.0\% \dots +95.0\%$, Profile B). *Push: Invert Polarity*.
* **Knob 4 (`RATE B`):** Phaser B LFO modulation frequency ($0.05\,\text{Hz} \dots 18.0\,\text{Hz}$, Profile C). *Push: Link Rate to LFO A*.
* **Knob 5 (`DEPTH B`):** Phaser B sweep width ($0.0\% \dots 100.0\%$, Profile B). *Push: Snap 50%*.
* **Knob 6 (`FEEDBACK B`):** Phaser B resonance / color ($-95.0\% \dots +95.0\%$, Profile B). *Push: Invert Polarity*.
* **Knob 7 (`LFO INTERACTION / PHASE`):** Phase offset between LFO A and LFO B ($0^\circ \dots 360^\circ$, Profile B). *Push: Snap Quadrature (90°)*.
* **Knob 8 (`MIX`):** Universal Tri-Zone Dry/Wet Mix (Profile M: 7:00 Wet Audition, 12:00 Pure Dry, 5:00 Parallel). *Push: Phase Invert ($\varnothing = 180^\circ$)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Phaser Routing Topology):** `[ Series (A -> B, 12-Stage) | Parallel Dual Mono | Stereo Cross-Feedback | Ring Phase ]`.
* **DIP 2 (LFO Sync Mode):** `[ Independent Dual LFOs | Master LFO A Tracks B | Inverted LFO B (180°) ]`.
* **DIP 3 (Opto-Coupler Model):** `[ Vintage CdS Vactrol (Sluggish Decay) | Fast LED Photodiode | Pure Linear Digital ]`.
* **DIP 4 (Stage Allocation):** `[ 6 + 6 Stages | 4 + 8 Stages | 2 + 10 Stages | 12-Stage Mono Ultra ]`.
* **TRIM 1 (Vactrol Dark Decay Time):** Relaxes the phototransistor decay tail ($20\,\text{ms} \dots 400\,\text{ms}$).
* **TRIM 2 (Feedback Saturation Ceiling):** Non-linear diode clipping ceiling in the feedback tank.
* **TRIM 3 (Center Frequency Bias A):** Resting center frequency of Phaser A ($200\,\text{Hz} \dots 2.5\,\text{kHz}$).
* **TRIM 4 (Center Frequency Bias B):** Resting center frequency of Phaser B ($200\,\text{Hz} \dots 2.5\,\text{kHz}$).

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Routing Mode (DIP 1) | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`Series (12-Stage)`**| `Knob 4, 6` | `STAGES 7-12 SPREAD`, `TOTAL REGEN` | `±12 st`, `0..100%` | `Profile B`, `Profile B` | Consolidates into monolithic 12-stage deep comb phaser |
| **`Stereo Linked`** | `Knob 7` | `STEREO L/R SPREAD` | `0°..180°` | `Profile B` | Controls spatial ping-pong phase rotation width |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Vectorized All-Pass Pipeline:** All-pass filter state updates grouped into SIMD registers, calculating 4 all-pass stages simultaneously.
* **Non-Linear Ballistics:** Vactrol resistance calculated using branchless asymmetrical one-pole tracking.
