# Technical DSP Specification: `AmpEnv` (Universal 4-Mode Amp Envelope)

**Engine ID:** `CARD-03B`  
**Sovereign Name:** `AmpEnv`  
**Corporate Boring Name:** `Universal 4-Mode Amplitude Envelope & VCA Modulator`  
**Category:** Expressive Amplitude & Articulation Generator  
**APVTS Prefix:** `aenv_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section D](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Roland TR-808 / TR-909 analog VCA decay circuits, Roland Juno-106 analog VCA envelope, Oberheim OB-Xa linear/exponential VCAs, Buchla 292 Low Pass Gate.
* **Upstream Offline Reference:** [tr808_service_manual.md](file:///C:/Dev/Research/library/manuals/tr808_service_manual.md) · [01_renoise_complete_offline_guide.md](file:///C:/Dev/Research/library/01_renoise_complete_offline_guide.md).
* **C++ Implementation Header:** [`AmpEnvelopeGenerator.h`](file:///c:/Dev/TheKlangDSP/include/dsp/envelopes/AmpEnvelopeGenerator.h).

---

## 2. Mathematical Theory & DSP Architecture

AmpEnv shapes primary acoustic volume, dynamic tremolo swell, and percussive hit articulation:
1. **Four Specialized Amplitude Topologies:**
   - **Mode 1 (Analog AHDSR):** Attack, Hold, Decay, Sustain, Release with exponential capacitor charge curves.
   - **Mode 2 (TR-808/909 One-Shot Drum):** Dedicated zero-attack decaying transient with flam trigger support and analog VCA bleed.
   - **Mode 3 (LPG Vactrol Strike):** Models the organic decay of a vactrol low-pass gate (Buchla 292), combining simultaneous volume decay with gentle high-frequency damping.
   - **Mode 4 (Gated Strobe / Chopper):** Rhythmic square/sine pulsing amplitude chopper synchronized to host tempo.
2. **Dual Target Modulation (Level vs VCA Drive):**
   Modulates **Output Level**, **VCA Drive Saturation**, or **Both**, allowing louder notes to automatically distort harder.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`ENV TYPE`):** Discrete choice: `Analog AHDSR`, `TR-808/909 Drum`, `LPG Vactrol Strike`, `Gated Chopper`. Discrete Choice. *Push: Next Type*.
* **Knob 2 (`MOD TARGET`):** Routing target: `Level Only`, `VCA Drive Only`, `Both (Volume + Distortion)`, `Opposite (Swell Drive)`. Discrete Choice. *Push: Next Target*.
* **Knob 3 (`ATTACK`):** Onset duration ($0.1\,\text{ms} \dots 10.0\,\text{s}$, Profile A Log). *Push: Instant Snap (0.1ms)*.
* **Knob 4 (`HOLD / CLAPS`):** Peak level hold time or multi-trigger handclap burst ($0.0\,\text{ms} \dots 500.0\,\text{ms}$, Profile A). *Push: Reset Zero*.
* **Knob 5 (`DECAY`):** Decay time down to sustain level ($1.0\,\text{ms} \dots 20.0\,\text{s}$, Profile A). *Push: Tempo Sync*.
* **Knob 6 (`SUSTAIN`):** Resting amplitude level ($0.0\% \dots 100.0\%$, Profile B). *Push: Snap 0% (Drum Mode)*.
* **Knob 7 (`RELEASE / SPREAD`):** Gain recovery time after note-off or stereo decay spread ($1.0\,\text{ms} \dots 20.0\,\text{s}$, Profile A). *Push: Snap to Decay*.
* **Knob 8 (`BIPOLAR DEPTH`):** Master modulation amplitude ($0.0\% \dots 100.0\%$, Profile B). *Push: Snap Max (100%)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Retrigger Mode):** `[ Retrig from 0V Snap | Legato Catch (Smooth) ]`.
* **DIP 2 (Gate Mode):** `[ Note Gate (Sustain Enabled) | One-Shot Percussion Trigger ]`.
* **DIP 3 (Curve Tension):** `[ Exponential Analog (Juno) | Linear Modern | Hyper-Punch Log ]`.
* **DIP 4 (Anti-Click Protection):** `[ Micro-Fade In (1ms) | Fast Direct Edge (0.1ms) | Zero Slew ]`.
* **TRIM 1 (Velocity Sensitivity Scaling):** Velocity dynamic range sensitivity ($0.0\% \dots 100.0\%$).
* **TRIM 2 (Flam Multi-Trigger Count):** Multi-burst count for 808/909 handclap simulation ($1 \dots 4$ bursts).
* **TRIM 3 (Vactrol Dark Ring Damping):** Tunes tail decay length in LPG mode.
* **TRIM 4 (VCA Silence Floor):** Noise gate threshold cutting off minute envelope tails ($-\infty \dots -90\,\text{dBFS}$).

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Active Type (Knob 1) | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`TR-808/909 Drum`**| `Knobs 4, 7` | `CLAP BURSTS`, `FLAM SPREAD` | `1..4`, `5..50ms` | `Discrete`, `Profile A` | Creates multi-burst handclap transient clusters |
| **`LPG Strike`** | `Knobs 3, 5` | `STRIKE VEL`, `RING DAMP` | `0..100%`, `10ms..5s` | `Profile B`, `Profile A` | Buchla 292 vactrol strike dynamic parameters |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Anti-Denormal Threshold:** Amplitude multipliers flushed directly to zero below $-96\,\text{dBFS}$.
* **Lock-Free Concurrency:** Trigger events consume atomic integer ring buffer without mutex contention.
