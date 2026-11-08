# Technical DSP Specification: `56K Vocoder Internal Carrier` (Clean-Room MKS-50 Engine)

**Engine ID:** `OSC-12`  
**Sovereign Name:** `56K Vocoder Internal Carrier`  
**Corporate Boring Name:** `Clean-Room MKS-50 Carrier Engine`  
**Category:** Synthesizer Carrier Oscillator  
**APVTS Prefix:** `carrier56k_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section B](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-b)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Electrix WarpFactory 25th Anniversary Edition internal carrier, Roland MKS-50 Alpha Juno.
* **Upstream Offline Reference:** [electrix_warpfactory_manual.md](file:///C:/Dev/Research/library/manuals/electrix_warpfactory_manual.md) & [10_electrix_warpfactory_and_vocoder_reference.md](file:///C:/Dev/Research/library/10_electrix_warpfactory_and_vocoder_reference.md).
* **C++ Implementation Header:** [`VocoderCarrier56K.h`](file:///c:/Dev/TheKlangDSP/include/dsp/oscillators/VocoderCarrier56K.h).

---

## 2. Mathematical Theory & DSP Architecture

A vocoder's intelligibility and presence depends critically on its carrier's harmonic density. A pure sine wave cannot vocode speech because it lacks high-frequency energy to excite vocal formant filter bands ($F_1, F_2, F_3$).

`56K Vocoder Internal Carrier` is a custom-tailored polyphonic rich-harmonic oscillator combining:
1. **Multi-Saw Cascade:** Dual anti-aliased saw waves with variable phase offset ($0^{\circ} \dots 180^{\circ}$) ensuring flat spectral energy from $100\text{ Hz} \dots 12\text{ kHz}$.
2. **Noise Fizz Injection:** High-frequency filtered pink noise injected above $4\text{ kHz}$ to guarantee unvoiced sibilants ($S, T, SH, CH$) cut through with razor-sharp intelligibility.
3. **Auto-Pitch Tracking:** Real-time pitch follower deriving $f_0$ directly from incoming microphone vocal fundamental.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`CARRIER TUNE`):** Carrier fundamental frequency ($50\text{ Hz} \dots 2000\text{ Hz}$, Profile D). *Push: Snap A4 (440 Hz)*.
* **Knob 2 (`WAVE SHAPE`):** Saw, Pulse, Saw+Noise, Supersaw, Full Rich Spectrum. Discrete. *Push: Cycle Wave*.
* **Knob 3 (`OCTAVE / DETUNE`):** Dual oscillator detune spread ($-12\text{ st} \dots +12\text{ st}$, Profile B). *Push: Pure Unison*.
* **Knob 4 (`NOISE FIZZ`):** High-frequency sibilance noise injection ($0\% \dots 100\%$, Profile B). *Push: Mute Fizz*.
* **Knob 5 (`AUTO-PITCH TRACK`):** Microphone pitch-following tracking speed ($0\% \dots 100\%$, Profile B). *Push: Lock Manual Pitch*.
* **Knob 6 (`SPECTRAL TILT`):** Carrier frequency balance (Bass Rich &rarr; Sibilant Air, Profile B). *Push: Flat Neutral*.
* **Knob 7 (`POLYPHONY CHORD`):** Single Note, Major Triad, Minor Triad, Dominant 7th, Sus4. Discrete. *Push: Cycle Chord*.
* **Knob 8 (`OUTPUT LEVEL`):** Carrier output gain trim ($\pm 18\text{ dB}$, Profile B). *Push: Unity 0dB*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Pitch Tracker Mode):** `[ Autocorrelation YIN | Zero-Crossing Fast | Fixed MIDI Pitch ]`.
* **DIP 2 (Sibilance Inject Point):** `[ Pre-Filter Carrier Mix | Dynamic Sibilance Sidechain Switch ]`.
* **DIP 3 (Oscillator Sync):** `[ Hard Sync on Microphone Transient | Free Running DCO ]`.
* **DIP 4 (Chord Inversion):** `[ Root Position | 1st Inversion | Open Voicing ]`.
* **TRIM 1 (Pitch Tracker Hysteresis):** Prevents octave jitter during vocal flutter.
* **TRIM 2 (Sub-Bass Carrier Boost):** Low-frequency reinforcement below $150\text{ Hz}$.
* **TRIM 3 (High-Cut Reconstruction Ceiling):** Lowpass filter suppressing ultrasonic alias harmonics.
* **TRIM 4 (Microphone Noise Gate):** Prevents room bleed from triggering the carrier engine.
