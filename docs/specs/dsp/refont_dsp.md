# Technical DSP Specification: `ReFont` (Acoustic Sample Player)

**Engine ID:** `CARD-07B` / `OSC-11`  
**Sovereign Name:** `ReFont`  
**Corporate Boring Name:** `Acoustic Sample Player`  
**Category:** Multi-Format PCM Sampler  
**APVTS Prefix:** `refont_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section D](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Renoise `.xrni` multi-sample instrument engine, SoundFont 2 (`.sf2`), Akai S950/S1000, Single-Cycle Waveform (SCW) synthesis.
* **Upstream Offline Reference:** [01_renoise_complete_offline_guide.md](file:///C:/Dev/Research/library/01_renoise_complete_offline_guide.md) & [05_upstream_repositories_manifest.md](file:///C:/Dev/Research/library/05_upstream_repositories_manifest.md).
* **C++ Implementation Header:** [`ReFontPlayer.h`](file:///c:/Dev/TheKlangDSP/include/dsp/oscillators/ReFontPlayer.h).

---

## 2. Mathematical Theory & DSP Architecture

`ReFont` is an ultra-low latency, zero-allocation micro-sampler engineered for acoustic strikes, multi-sampled instruments, and looping single-cycle waveforms.

### 1. High-Res Resampling & Interpolation
Supports three selectable DSP interpolation regimes:
1. **Bandlimited Sinc (Windowed Blackman 8-Point):** Pristine transposition with zero aliasing across $\pm 36$ semitones.
2. **Linear Interpolation:** Fast, balanced, classic 90s ROMpler character.
3. **Drop-Sample (SP-1200 / Amiga Paula):** Zero interpolation; raw nearest-neighbor decimation producing crunchy high-frequency aliasing mirrors.

### 2. Tracker `0Sxx` Scrubbing & Loop Modes
Sub-sample accurate phase accumulator offset allows instant tracker-style `0Sxx` sample start scrubbing:
$$\text{SampleOffset} = \text{floor}\left(\frac{xx}{256} \cdot \text{TotalSampleFrames}\right)$$
Four playback topologies: `One-Shot Drum`, `Forward Loop`, `Ping-Pong Bidirectional`, and `Reverse`.

### 3. New Note Action (NNA) Voice Handling
100% isomorphic with Renoise NNA:
* `Cut`: Instant hard clamp with 1ms anti-click micro-fade.
* `Overlap`: 4-voice polyphonic decay stealing.
* `Continue`: Preserves prior release tail while launching new transient.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Page 1A (Playback & Geometry):**
  * `Knob 1 (SAMPLE SELECT):` Scrolls internal ROM / project samples. Discrete. *Push: Audition Preview*.
  * `Knob 2 (PITCH / SPEED):` Chromatic tuning & rate ($\pm 24\text{ st}$, Profile B). *Push: Reset Root (0)*.
  * `Knob 3 (SAMPLE START):` Playhead start offset ($0.0\% \dots 90.0\%$, Profile B). *Push: Zero-Crossing Snap*.
  * `Knob 4 (LENGTH / DECAY):` Playback window ($2\text{ ms} \dots 10.0\text{ s}$, Profile A). *Push: Play Full Sample*.
* **Page 1B (Tone & Modulation):**
  * `Knob 5 (ATTACK / SNAP):` Transient attack ramp ($0.1\text{ ms} \dots 100\text{ ms}$, Profile A). *Push: Instant 0ms*.
  * `Knob 6 (BITS / CRUSH):` Bit-depth reduction ($16\text{ bit} \dots 2\text{ bit}$, Profile H). *Push: Modern vs 12-bit SP1200*.
  * `Knob 7 (FILTER CUTOFF):` 2-pole lowpass tone filter ($20\text{ Hz} \dots 20\text{ kHz}$, Profile D). *Push: Bypass Filter*.
  * `Knob 8 (DIRECTION / LOOP):` One-Shot, Forward, Ping-Pong, Reverse. Discrete. *Push: Reverse Instant*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Interpolation Engine):** `[ 8-Point Sinc | Linear | Drop-Sample (SP-1200 / Paula) ]`.
* **DIP 2 (NNA Voice Action):** `[ Cut (Anti-Click) | Overlap (4-Voice Poly) | Continue Release ]`.
* **DIP 3 (Velocity Mapping):** `[ Velocity -> Level Only | Vel -> Start Time Scrub | Vel -> Filter Cutoff ]`.
* **DIP 4 (Root Tracking):** `[ 1V/Oct Chromatic Keyboard | Constant Fixed Pitch (Percussion) ]`.
* **TRIM 1 (Anti-Click Crossfade Time):** Micro-fade duration on loop wrap & choke ($0.5\text{ ms} \dots 10\text{ ms}$).
* **TRIM 2 (High-Frequency Sinc Rolloff):** Sinc filter anti-aliasing reconstruction ceiling.
* **TRIM 3 (Velocity Dynamic Curve):** Exponential velocity-to-volume curvature.
* **TRIM 4 (Post-Sample Gain Trim):** Output normalization trim ($-12\text{ dB} \dots +12\text{ dB}$).
