# Technical DSP Specification: `Argent Slayer` (Phase-Locked Sub-Bass Generator)

**Engine ID:** `OSC-05`  
**Sovereign Name:** `Argent Slayer`  
**Corporate Boring Name:** `Phase-Locked Sub-Bass Generator`  
**Category:** Sub-Bass & Low-End Synthesizer  
**APVTS Prefix:** `argentslayer_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section B](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-b)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Roland TR-808 bridged-T sub kick, Mick Gordon DOOM sub-sine synthesizers.
* **Upstream Offline Reference:** [ROSETTA_STONE_MASTER_IP_REFERENCE.md](file:///C:/Dev/Research/library/manuals/ROSETTA_STONE_MASTER_IP_REFERENCE.md).
* **C++ Implementation Header:** [`ArgentSlayer.h`](file:///c:/Dev/TheKlangDSP/include/dsp/oscillators/ArgentSlayer.h).

---

## 2. Mathematical Theory & DSP Architecture

`Argent Slayer` delivers immovable low-end weight ($20\text{ Hz} \dots 120\text{ Hz}$) locked to MIDI pitch, combined with recursive wavefolding and audio-rate noise ring modulation to slice through phone speakers without losing sub energy.

### 1. Phase-Locked Pure Sub Core
A clean fundamental sine wave phase-locked to incoming note triggers:
$$s_0(t) = \sin(2\pi f_0 t)$$
Includes zero-crossing phase reset to eliminate low-frequency click transients and destructive phase cancellation with kick drums.

### 2. Recursive Asymmetric Wavefolder
Instead of standard symmetrical folding, `Argent Slayer` injects dynamic DC offset before the fold, producing heavy 2nd-harmonic octaves:
$$x_{fold}[n] = \sin\left( \pi \cdot \left( k_{drive} \cdot s_0[n] + V_{DC} \right) \right)$$

### 3. Noise Ring Modulation (The "DOOM Bite")
Injects audio-rate bandpassed noise multiplied by the sub fundamental:
$$y_{bite}[n] = s_0[n] \cdot \text{BPF}_{800\text{Hz}}(n_{white}[n])$$
This creates psychoacoustic phantom fundamentals that allow sub-bass to be heard clearly on laptop and mobile speakers.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`PITCH`):** Sub fundamental ($20\text{ Hz} \dots 200\text{ Hz}$, Profile D). *Push: Lock C1 (32.7 Hz)*.
* **Knob 2 (`DECAY`):** Sub boom decay time ($50\text{ ms} \dots 10.0\text{ s}$, Profile A). *Push: Infinite Drone*.
* **Knob 3 (`FOLD DEPTH`):** Asymmetric sine wavefold depth ($0\% \dots 100\%$, Profile E). *Push: Max Squelch*.
* **Knob 4 (`DC SYMMETRY`):** DC bias injecting 2nd-order tube harmonic bloom. Profile B. *Push: Zero Bias*.
* **Knob 5 (`DOOM BITE`):** Sub-sine noise ring-modulation level ($0\% \dots 100\%$, Profile B). *Push: Mute Bite*.
* **Knob 6 (`PITCH BEND / GLIDE`):** Pitch drop envelope depth on attack ($0 \dots +48\text{ st}$, Profile A). *Push: 808 Kick Drop*.
* **Knob 7 (`SUB DRIVE`):** Pre-transformer core saturation ($0\text{ dB} \dots +18\text{ dB}$, Profile E). *Push: Clean Sub*.
* **Knob 8 (`OUTPUT CEILING`):** Brickwall true-peak limiter ceiling ($-12\text{ dBFS} \dots 0\text{ dBFS}$, Profile B). *Push: Snap -0.1 dBFS*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Sub Phase Reset):** `[ Hard Reset 0° Punch | Zero-Crossing Soft Reset | Free-Running Phase ]`.
* **DIP 2 (Fold Algorithm):** `[ Buchla Sine Fold | Serge Triangle Fold | Germanium Diode Hard Clip ]`.
* **DIP 3 (Bite Noise Band):** `[ 800 Hz Snarl | 2 kHz Metal Tear | Wideband Noise ]`.
* **DIP 4 (Bass Mono Lock):** `[ 100% Phase-Locked Mono Sub (<120Hz) | Stereo Upper Harmonics ]`.
* **TRIM 1 (Pitch Drop Decay Speed):** Speed of the 808 kick transient pitch plunge ($5\text{ ms} \dots 150\text{ ms}$).
* **TRIM 2 (Sub-Harmonic Divider):** Level of $-1$ octave square sub-harmonic divider.
* **TRIM 3 (Transformer Saturation Knee):** Low-frequency saturation warmth threshold.
* **TRIM 4 (Noise Gate Threshold):** Post-decay tail silence gate.
