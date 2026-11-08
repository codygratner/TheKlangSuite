# Technical DSP Specification: `KlangPunch` (Dynamics, Transients & Bus Glue Lab)

**Engine ID:** `MEGA-02`  
**Sovereign Name:** `KlangPunch`  
**Corporate Boring Name:** `Master Bus Dynamics, Multi-Band Compressor & Transient Workstation`  
**Category:** Flagship Studio Dynamics Workstation  
**APVTS Prefix:** `kpunch_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section D](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** SSL 4000 G Master Bus Compressor, Empirical Labs Distressor EL8, Fairchild 670 variable-mu limiter, SPL Transient Designer.
* **Upstream Offline Reference:** [ssl_g_bus_manual.md](file:///C:/Dev/Research/library/manuals/ssl_g_bus_manual.md) · [ROSETTA_STONE.md#section-d](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d).
* **C++ Implementation Header:** [`KlangPunchDynamics.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/KlangPunchDynamics.h).

---

## 2. Mathematical Theory & DSP Architecture

KlangPunch is the master dynamics laboratory unifying feed-forward VCA bus compression, vintage optical/tube dynamics, and envelope transient sculpting:
1. **Dynamic Topologies:**
   - **Topology A (SSL VCA):** Ultra-punchy feed-forward VCA with classic stepped ratios ($2:1, 4:1, 10:1$) and logarithmic auto-release.
   - **Topology B (Distressor Nuke):** Brickwall limiting with British opto detector and variable 2nd/3rd harmonic saturation.
   - **Topology C (Fairchild 670):** Feedback variable-mu tube compression with 6 program-dependent time constants.
   - **Topology D (Transient Shaper):** Differential envelope follower comparing fast ($1\,\text{ms}$) and slow ($20\,\text{ms}$) peak detectors to isolate and amplify transients independently of volume.
2. **Frequency-Selective Sidechain & Wet/Dry Parallel Routing:**
   Equipped with dedicated highpass ($20\,\text{Hz} \dots 400\,\text{Hz}$) and presence bell ($3\,\text{kHz}$) sidechain filters, plus true parallel New York compression mix law.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`THRESHOLD`):** Compression engagement level ($-48.0\,\text{dB} \dots 0.0\,\text{dB}$, Profile B). *Push: Auto-Threshold*.
* **Knob 2 (`RATIO / SQUASH`):** Compression ratio `1.5:1` to `NUKE (Inf:1)`. Discrete Choice. *Push: Instant NUKE*.
* **Knob 3 (`ATTACK`):** Transient onset passage ($0.02\,\text{ms} \dots 100.0\,\text{ms}$, Profile A Log). *Push: Fast 0.1ms*.
* **Knob 4 (`RELEASE`):** Recovery time ($10.0\,\text{ms} \dots 2000.0\,\text{ms}$ + Auto). Profile A. *Push: Toggle Auto-Release*.
* **Knob 5 (`TRANSIENT ATTACK`):** Dedicated transient strike gain ($-12.0\,\text{dB} \dots +12.0\,\text{dB}$, Profile B). *Push: Reset Zero*.
* **Knob 6 (`TRANSIENT SUSTAIN`):** Tail sustain gain ($-12.0\,\text{dB} \dots +12.0\,\text{dB}$, Profile B). *Push: Reset Zero*.
* **Knob 7 (`MAKEUP / GLUE`):** Linear post-makeup gain ($0.0\,\text{dB} \dots +24.0\,\text{dB}$, Profile B). *Push: Auto-Makeup*.
* **Knob 8 (`MIX`):** Universal Tri-Zone Dry/Wet Mix (Profile M: 7:00 Wet Audition, 12:00 Pure Dry, 5:00 Parallel). *Push: Phase Invert ($\varnothing = 180^\circ$)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Compressor Topology):** `[ SSL 4000 G Bus | Distressor Opto | Fairchild 670 Tube | SPL Transient Only ]`.
* **DIP 2 (Sidechain Filter Mode):** `[ 120Hz Bus HPF | 80Hz Tight | 3kHz Presence Bell | Full Bandwidth ]`.
* **DIP 3 (Stereo Detection Link):** `[ Linked (Summed RMS) | Dual Mono | Mid/Side Unlinked ]`.
* **DIP 4 (Limiter Mode):** `[ True Peak ISP Limiter | Analog Clipper | Off ]`.
* **TRIM 1 (Sidechain Filter Cutoff):** Precision frequency sweep for sidechain highpass ($20\,\text{Hz} \dots 400\,\text{Hz}$).
* **TRIM 2 (Harmonic Saturation Injection):** Pre-limiter tube/tape harmonic drive ($0.0\% \dots 100.0\%$).
* **TRIM 3 (Knee Width):** Variable soft-knee width ($0.0\,\text{dB} \dots 18.0\,\text{dB}$).
* **TRIM 4 (Ceiling Margin):** Brickwall true-peak safety ceiling ($-0.1\,\text{dB} \dots -1.0\,\text{dBFS}$).

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Topology (DIP 1) | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`Transient Only`** | `Knob 1, 2` | `DETECTOR SENS`, `PUMP SPEED` | `0..100%`, `1..100ms` | `Profile B`, `Profile A` | Direct control of differential envelope detectors |
| **`Fairchild 670`** | `Knob 4` | `TIME CONSTANT`| `Position 1..6` | `Discrete` | Selects historical 6-preset dual-decay curves |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Zero Latency Feed-Forward Detection:** Detection loop requires zero lookahead samples; optional 1ms true-peak lookahead limiter isolated to master stage.
* **Vectorized VCA Gain:** Attenuation coefficients smoothed in log domain and applied via SIMD aligned streaming instructions.
