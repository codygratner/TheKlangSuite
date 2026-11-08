# Technical DSP Specification: `Kaleidocore` (Macro Timbre Synthesizer)

**Engine ID:** `OSC-08`  
**Sovereign Name:** `Kaleidocore`  
**Corporate Boring Name:** `Macro Timbre Synthesizer`  
**Category:** Multi-Model Macro Synthesis Engine  
**APVTS Prefix:** `kaleido_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section B](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-b)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Mutable Instruments Plaits & Braids (Émilie Gillet), SAM Speech Synthesizer, Lyle Mills DX7 FM SysEx translator.
* **Upstream Offline Reference:** [03_mutable_instruments_dsp_encyclopedia.md](file:///C:/Dev/Research/library/03_mutable_instruments_dsp_encyclopedia.md).
* **C++ Implementation Header:** [`Kaleidocore.h`](file:///c:/Dev/TheKlangDSP/include/dsp/oscillators/Kaleidocore.h).

---

## 2. Mathematical Theory & DSP Architecture

`Kaleidocore` unifies 40 macro synthesis models into an ergonomic 3-macro dimensional control surface (`TIMBRE`, `MORPH`, `HARMONICS`):

### 40 Macro Synthesis Models
* **8 Classic Analog & Wavefolding Models:** Virtual analog saw/square with hard sync, triangle wavefolder, dual pulse.
* **8 Direct Digital & Wavetable Models:** Formant vowel filter, harmonic additive oscillator, 2-operator FM, wave terrain, chiptune PSG.
* **8 Physical & Percussive Models:** Struck modal bar, acoustic string, bass drum, snare drum, hi-hats.
* **8 Speech & Vowel Models:** SAM (Software Automatic Mouth), LPC-10 linear predictive speech, Votrax phonetic synthesis.
* **8 Chord & Harmonic Models:** Lyle Mills 4-voice paraphonic chord tables with inversions and scale quantization.

### Ergonomic Tri-Macro Geometry
Every model maps its internal multi-dimensional physics to three primary normalized encoders:
1. $\text{TIMBRE} \in [0, 1]$: Primary spectral brightness or modulation index.
2. $\text{MORPH} \in [0, 1]$: Secondary structural or dimensional morph (waveform blend, formant vowel, algorithm).
3. $\text{HARMONICS} \in [0, 1]$: Tertiary frequency distribution (detune, chord quality, inharmonic spread).

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`PITCH`):** Master fundamental ($20\text{ Hz} \dots 4000\text{ Hz}$, Profile D). *Push: Octave Transpose*.
* **Knob 2 (`MODEL SELECT`):** 40 Synthesis Models. Discrete. *Push: Next Model Category*.
* **Knob 3 (`TIMBRE`):** Primary Macro Timbre ($0\% \dots 100\%$, Profile B). *Push: Center 50%*.
* **Knob 4 (`MORPH`):** Secondary Macro Morph ($0\% \dots 100\%$, Profile B). *Push: Center 50%*.
* **Knob 5 (`HARMONICS`):** Tertiary Macro Harmonics ($0\% \dots 100\%$, Profile B). *Push: Center 50%*.
* **Knob 6 (`DECAY`):** Internal dynamic lowpass gate (LPG) decay ($10\text{ ms} \dots 10.0\text{ s}$, Profile A). *Push: Infinite Gate*.
* **Knob 7 (`LPG RESPONSE`):** Buchla 292 Lowpass Gate response (VCA &rarr; VCF+VCA &rarr; VCF, Profile B). *Push: Full Open*.
* **Knob 8 (`OUTPUT LEVEL`):** Master output gain trim ($\pm 18\text{ dB}$, Profile B). *Push: Unity 0dB*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Model Category Lock):** `[ Melodic Synth | Speech & Vocal | Percussion & Drums | Acoustic Physical ]`.
* **DIP 2 (LPG Character):** `[ Fast Snappy Solid-State | Vactrol Memory Lag | Pure Math Instant ]`.
* **DIP 3 (Output Mode):** `[ Main Out + Aux Out | Stereo Panned Spatial | Mid/Side Split ]`.
* **DIP 4 (Chord Quantize):** `[ Semitone Free | Root Quantized Major/Minor | Microtonal Arabic 24-TET ]`.
* **TRIM 1 (Vactrol Slew Lag):** Light-dependent resistor decay lag in LPG mode.
* **TRIM 2 (Timbre Modulation Attenuverter):** Inbuilt velocity-to-timbre sensitivity.
* **TRIM 3 (Sub-Harmonic Mix):** Extra octave-down sub oscillator injection.
* **TRIM 4 (Drive Saturation):** Internal soft limiter drive.
