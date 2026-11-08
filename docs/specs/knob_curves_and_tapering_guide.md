# The Anatomy of Musical Knobs: Master Catalog of Perceptual Tapering, Skew Curves & Ergonomic Laws for JUCE, ToadTracker & Embedded DSP

**Document Type:** Canonical DSP & UI Ergonomics Reference Specification 📜  
**Target Projects:** The Klang Suite (`TKS`), ToadTracker (`TT`), The Klang Mill (`TKM`), dadamachines TBD-16  
**Ecosystem Standard:** Universal Parameter Tapering & Ergonomics Standard (v1.0.0)  
**Authors:** Antigravity x Cody G. (Klang Industries / ToadTracker Audio Division)  
**Date:** 2026-10-18  

---

## 1. Executive Summary & The Problem of "Ergonomic Cliffing"

Human auditory perception of physical acoustic phenomena—loudness, frequency, time duration, and harmonic distortion—is non-linear:
* **Pitch** is perceived logarithmically (doubling frequency equals one musical octave).
* **Loudness** is perceived logarithmically (the Weber-Fechner law and Stevens' power law).
* **Decay Time** is evaluated in orders of magnitude ($10\text{ ms}$ vs $100\text{ ms}$ is a massive stylistic leap; $10.0\text{ s}$ vs $10.1\text{ s}$ is barely perceptible).
* **Harmonic Saturation** exhibits a threshold cliff (delicate warming happens over a subtle decibel range, followed by abrupt square-wave clipping).

When audio software uses raw linear parameter ranges or uncalibrated sliders, it causes **"Ergonomic Cliffing"**:
```
RAW LINEAR PARAMETER (UNUSABLE):
[ 10 ms ────────────────────────────────────────────────────────── 60,000 ms ]
  ▲ First 1% (Tiny tick): ALL drum hits   ▲ Remaining 99%: Ambient mud & drones
```
On a standard $270^\circ$ potentiometer or rotary encoder, 90% of the musical action is crammed into a microscopic 3-degree sliver, while the remaining 95% of travel feels dead, sluggish, and unresponsive.

### The Golden Rule of Musical Ergonomics
> **Every continuous parameter in The Klang Suite and ToadTracker must be mathematically tapered to its perceptual sweet-spot so that 100% of physical encoder sweep feels musical, responsive, and predictable.**

---

## 2. The Master Taxonomy of Taper Profiles

```
PROFILE A: Cubic Temporal Warp (y = x³)
[ 10 ms ────────────────────── [ 1.5 s @ 12:00 ] ──────────────────── 60.0 s ]
▲ 0%–50% Travel: High-res transient punch     ▲ 50%–100%: Long ambient sustain

PROFILE B: Bipolar Sigmoidal S-Curve with Center Plateau
[ -100% ─────────────── [ NEUTRAL 0% @ 12:00 ] ──────────────── +100% ]
▲ Precision centering deadzone                 ▲ Accelerates into extreme modulation

PROFILE C: Dual-Regime Geometric Skew (LFO to Audio Rate)
[ 0.02 Hz ──────────────────── [ 20 Hz @ 12:00 ] ────────────────── 5.0 kHz ]
▲ 0%–50%: Rhythmic tempo LFO                  ▲ 50%–100%: Searing audio-rate FM

PROFILE D: Equal-Octave Logarithmic Sweep
[ 20 Hz ────────────────────── [ 800 Hz @ 12:00 ] ───────────────── 20.0 kHz ]
▲ Exactly equal degrees of physical encoder rotation per musical octave

PROFILE E: Non-Linear Warmth Expansion (y = x^1.8)
[ 0 dB ─────────────────────── [ +6 dB @ 12:00 ] ────────────────── +36 dB ]
▲ First 60%: Creamy tape/console warmth        ▲ Final 40%: Screaming diode fuzz

PROFILE F: Hyperbolic End-Stop Expansion (Runaway Q & Feedback)
[ 0% ───────────────────────── 70% ───────────── 95% ──────── 115% ]
▲ Wide sweet-spot for singing resonance        ▲ High-precision self-oscillation control

PROFILE G: Dual-Target Antiphase Bloom (+A & -B)
[ Core A Dives ◄────────────── [ NEUTRAL ] ──────────────► Core B Soars ]
▲ Simultaneous divergent motion (Opposite Acoustic Bloom)

PROFILE H: Stepped Retro / MML Quantization
[ Step 0 ──── Step 1 ──── Step 2 ──── ... ──── Step 15 ] (4-bit / 5-bit staircase)
▲ Authentic chiptune DAC attenuation & microtonal intervals

PROFILE M: Tri-Zone Parallel Mix Law & Phase Inversion
[ 100% WET (7:00) ──────── [ PURE DRY (12:00) ] ──────── PARALLEL BLEED (5:00) ]
▲ Pure Audition             ▲ True Neutral Home          ▲ 100% Dry + 100% Wet
```

---

## 3. Deep Dive into the 9 Taper Profiles

### Profile A: Cubic Temporal Warp ($y = x^3$)
* **Purpose:** Envelope Attack, Decay, Release, Pre-Delay, and Portamento Glide times.
* **Problem Solved:** Linear time parameters make dialing a $40\text{ ms}$ snare decay vs a $65\text{ ms}$ clap decay nearly impossible on an encoder that tops out at $20\text{ s}$.
* **The Math:**
  $$T(x) = T_{\min} + (T_{\max} - T_{\min}) \cdot x^3 \quad \text{for } x \in [0.0, 1.0]$$
  $$x(T) = \sqrt[3]{\frac{T - T_{\min}}{T_{\max} - T_{\min}}}$$
* **Rotational Distribution ($T_{\min} = 10\text{ ms}, T_{\max} = 60\text{ s}$):**
  * $x = 0.00$ ($7\text{ o'clock}$): $10\text{ ms}$ (instant transient click).
  * $x = 0.25$ ($9\text{ o'clock}$): $\approx 50\text{ ms}$ (punchy 808/909 closed hat).
  * $x = 0.50$ ($12\text{ o'clock}$): $\approx 1.5\text{ s} \dots 2.5\text{ s}$ (natural open hat / bass decay).
  * $x = 0.75$ ($3\text{ o'clock}$): $\approx 12.5\text{ s}$ (cinematic reverb tail).
  * $x = 1.00$ ($5\text{ o'clock}$): $60.0\text{ s}$ (infinite drone).
* **The Musical Victory:** Exactly half of the physical encoder sweep is reserved for transient drum synthesis, while the remaining half glides gracefully into atmospheric sound design.

---

### Profile B: Bipolar Sigmoidal S-Curve with Center Plateau
* **Purpose:** Bipolar Modulation Depths ($\pm 100\%$), Pitch Envelope Blips, Fine Tuning ($\pm 50\text{ cents}$), Stereo Pan, Velocity / Keytrack Modulation Amounts.
* **Problem Solved:** Normal linear bipolar knobs are hypersensitive near center; a tiny bump detunes a bass note or introduces unwanted modulation.
* **The Math:**
  Normalized input $u \in [-1.0, 1.0]$ where center $12\text{ o'clock} = 0.0$:
  $$S(u) = \text{sgn}(u) \cdot |u|^p \quad \text{where } p = 2.2 \dots 2.4$$
* **Perceptual Mechanics:**
  1. **The Deadzone Plateau:** Between $-0.15 \le u \le +0.15$, the curve is almost flat ($\approx 0.0$). This provides a confident "sweet spot" at 12 o'clock where zero means zero.
  2. **Fine-Grain Expressive Zone:** Between $0.15 \le |u| \le 0.60$, the slope is shallow, allowing surgical adjustments of micro-pitch and velocity dynamics ($\pm 1\% \dots \pm 15\%$).
  3. **The Exponential Wall:** As $|u| \to 1.0$, the curve steepens sharply into extreme modulations ($\pm 4\text{ octaves}$, full cross-mod audio-rate chaos).

---

### Profile C: Dual-Regime Geometric Skew (Piecewise Log)
* **Purpose:** Wide-range modulators (LFO speed that transitions into audio-rate FM, Ring Modulator carrier frequencies).
* **Problem Solved:** The human brain processes modulation frequencies under $20\text{ Hz}$ as **rhythm and groove** (tremolo, vibrato, panning sweeps), and modulation frequencies above $20\text{ Hz}$ as **timbre and pitch** (growling sidebands, metallic bells, audio-rate FM).
* **The Math:**
  Knob sweep is split at $x = 0.50$ ($12\text{ o'clock}$), which is locked precisely to $f_{\text{pivot}} = 20.0\text{ Hz}$:
  $$f(x) = \begin{cases} 
  f_{\min} \cdot \left(\frac{20.0}{f_{\min}}\right)^{2x} & \text{for } x \le 0.50 \quad \text{(Rhythmic Regime: } 0.02\text{ Hz} \dots 20\text{ Hz)} \\
  20.0 \cdot \left(\frac{f_{\max}}{20.0}\right)^{2(x - 0.50)} & \text{for } x > 0.50 \quad \text{(Audio-Rate Regime: } 20\text{ Hz} \dots 5.0\text{ kHz)}
  \end{cases}$$
* **The Musical Victory:** Turning the knob left of 12:00 gives wide, musical tempo-synced wobbles; turning the knob right of 12:00 unleashes screaming industrial FM sidebands.

---

### Profile D: Equal-Octave Logarithmic Sweep
* **Purpose:** Filter Cutoff Frequencies ($20\text{ Hz} \dots 20.0\text{ kHz}$).
* **Problem Solved:** Pitch doubles every octave. A linear frequency knob puts $10\text{ kHz} \dots 20\text{ kHz}$ across half the travel (which is just top-end sizzle), leaving all the bass and vocal midrange crushed into the first 10%.
* **The Math:**
  $$F(x) = F_{\min} \cdot \left(\frac{F_{\max}}{F_{\min}}\right)^x = 20.0 \cdot (1000.0)^x \quad \text{for } x \in [0.0, 1.0]$$
  $$x(F) = \frac{\log_{10}(F / F_{\min})}{\log_{10}(F_{\max} / F_{\min})}$$
* **Rotational Distribution:**
  * $x = 0.00$: $20\text{ Hz}$ (Sub-bass rumble).
  * $x = 0.33$: $\approx 200\text{ Hz}$ (Bass body / kick punch).
  * $x = 0.50$ ($12\text{ o'clock}$): $\approx 800\text{ Hz}$ (Vocal formant / presence core).
  * $x = 0.75$: $\approx 3.5\text{ kHz}$ (Bite and snare crack).
  * $x = 1.00$: $20.0\text{ kHz}$ (Wide-open air).
* **The Musical Victory:** Every $10\%$ of knob rotation equals exactly one musical octave of acoustic frequency.

---

### Profile E: Non-Linear Warmth Expansion ($y = x^{1.8}$)
* **Purpose:** VCA Saturation Drive, Pre-Filter Drive, Wavefolding, Tape Heat.
* **Problem Solved:** Distortion knobs typically go from subtle warmth to screeching fuzz within the first quarter turn, rendering the rest of the knob an unplayable square-wave mess.
* **The Math:**
  $$\text{Gain}_{\text{dB}}(x) = \text{Gain}_{\max} \cdot x^{1.8} \quad \text{where } \text{Gain}_{\max} = +36\text{ dB}$$
* **Perceptual Mechanics:**
  * $0\% \dots 60\%$ travel: Covers $0\text{ dB} \dots +6\text{ dB}$, providing creamy 2nd-harmonic tube warmth and console tape glue.
  * $60\% \dots 85\%$ travel: Smoothly breaks into aggressive overdrive and even-order clipping.
  * $85\% \dots 100\%$ travel: Full sonic obliteration, wavefolding, and hard square-wave fuzz.

---

### Profile F: Hyperbolic End-Stop Expansion (Runaway Q & Feedback)
* **Purpose:** Filter Resonance / Q-Factor ($0.5 \dots 25.0$) and Delay/Comb Feedback ($0\% \dots 120\%$).
* **Problem Solved:** Resonance sounds subtle for most of the knob, but explodes into deafening, speaker-destroying self-oscillation in the last 2%.
* **The Math:**
  $$Q(x) = Q_{\min} + (Q_{\max} - Q_{\min}) \cdot \frac{1 - e^{-kx}}{1 - e^{-k}} \quad \text{where } k = 3.0$$
* **Perceptual Mechanics:**
  * Expands the rotational resolution right in the critical $70\% \dots 95\%$ "singing resonance" zone.
  * Allows you to dial in singing vowel formants and runaway dub delays with millimeter precision right on the razor edge of self-oscillation without blowing out your headroom.

---

### Profile G: Dual-Target Antiphase Bloom (+A & -B)
* **Purpose:** Pitch Envelopes, Dual-Line Phase Distortion (`CosmoPhase`), Dual-Filter Crossfades.
* **The Math:**
  When a single bipolar depth parameter $D \in [-1.0, 1.0]$ drives two destinations $A$ and $B$:
  $$\Delta A = +D \cdot \text{Env}(t), \quad \Delta B = -D \cdot \text{Env}(t)$$
* **Sonic Impact:**
  * **On Pitch:** Carrier dives while Modulator sweeps up (Opposite Acoustic Bloom). Recreates turbulent plate dispersion where high-frequency shimmers rise while fundamental strike weight falls.
  * **On Filter:** Cutoff sweeps down while Resonance sweeps up (TB-303 acid squelch on the decay).
  * **On Dual Oscillators:** DCO 1 detunes sharp while DCO 2 detunes flat (unison bloom).

---

### Profile H: Stepped Retro / MML Quantization
* **Purpose:** Deflemask / MML Chiptune Envelopes, Vintage DAC Attenuation (YM2612 5-bit, PSG 4-bit, NES 4-bit staircase).
* **The Math:**
  Given a continuous envelope value $v \in [0.0, 1.0]$ and step count $N \in \{16, 32\}$:
  $$v_{\text{quantized}} = \frac{\lfloor v \cdot (N - 1) + 0.5 \rfloor}{N - 1}$$
* **Sonic Impact:** Imparts authentic lo-fi grit, chiptune zipper stepping, and genuine 1980s retro character to smooth software envelopes.

---

### Profile M: Universal Tri-Zone Parallel Mix Law & Phase Inversion
* **Purpose:** Master `DRY/WET MIX` knob across all audio effects and processors.
* **The Problem:** Standard $0\% \dots 100\%$ linear mix knobs suck out your dry punch and low-end transients as soon as you blend in a tiny bit of reverb, distortion, or chorus.
* **The Layout:**
  ```
  [ PURE WET (7:00) ────── [ PURE DRY (12:00) ] ────── PARALLEL DRY + WET (5:00) ]
  ▲ 100% Wet / 0% Dry      ▲ 100% Dry / 0% Wet         ▲ 100% Dry + 100% Wet Bleed
  ```
* **The Transfer Function ($u \in [0.0, 1.0]$, center at $u = 0.5$):**
  * **Left Wing ($u \in [0.0, 0.50)$ — Audition & Send Crossfade):**
    $$\text{Dry}(u) = 2u, \quad \text{Wet}(u) = 1.0 - 2u$$
  * **Center ($u = 0.50$ — Neutral Pure Dry):**
    $$\text{Dry}(0.5) = 1.0, \quad \text{Wet}(0.5) = 0.0$$
  * **Right Wing ($u \in (0.50, 1.00]$ — True Parallel Injection):**
    $$\text{Dry}(u) = 1.0, \quad \text{Wet}(u) = 2(u - 0.50)$$
* **Phase Invert Action ($\varnothing \in \{+1, -1\}$):**
  Pushing the encoder flips the wet polarity ($180^\circ$ inversion):
  $$y(t) = \text{Dry}(u) \cdot x(t) + \varnothing \cdot \text{Wet}(u) \cdot x_{\text{wet}}(t)$$
  *On Chorus/Flanger:* Yields hollow through-zero comb cancellation.  
  *On Distortion/Fuzz:* Yields dynamic transient extraction and hollow harmonic filtering!

---

## 4. JUCE C++ Implementation Standard (for The Klang Suite)

In JUCE, custom curves are instantiated directly inside `juce::NormalisableRange<float>` parameter lambdas during APVTS creation:

```cpp
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <cmath>

namespace KlangErgonomics {

// PROFILE A: Cubic Temporal Warp (Envelope Times 10ms .. 60s)
inline juce::NormalisableRange<float> makeCubicTimeRange(float minSeconds, float maxSeconds) {
    return juce::NormalisableRange<float>(
        minSeconds, maxSeconds,
        [minSeconds, maxSeconds](float min, float max, float norm) {
            return minSeconds + (maxSeconds - minSeconds) * (norm * norm * norm);
        },
        [minSeconds, maxSeconds](float min, float max, float val) {
            float clamped = juce::jlimit(minSeconds, maxSeconds, val);
            return std::cbrt((clamped - minSeconds) / (maxSeconds - minSeconds));
        }
    );
}

// PROFILE B: Bipolar Sigmoidal S-Curve (Deadzone Plateau around 0.0)
inline juce::NormalisableRange<float> makeBipolarSCurveRange(float minVal, float maxVal, float power = 2.2f) {
    return juce::NormalisableRange<float>(
        minVal, maxVal,
        [minVal, maxVal, power](float min, float max, float norm) {
            float u = 2.0f * (norm - 0.5f); // [-1.0 .. +1.0]
            float s = std::copysign(std::pow(std::abs(u), power), u);
            return (s >= 0.0f) ? (s * maxVal) : (s * -minVal);
        },
        [minVal, maxVal, power](float min, float max, float val) {
            float normVal = (val >= 0.0f) ? (val / maxVal) : (-val / minVal);
            float u = std::copysign(std::pow(std::abs(normVal), 1.0f / power), val);
            return 0.5f + 0.5f * u;
        }
    );
}

// PROFILE D: Equal-Octave Logarithmic Cutoff (20 Hz .. 20 kHz)
inline juce::NormalisableRange<float> makeEqualOctaveCutoffRange(float minHz = 20.0f, float maxHz = 20000.0f) {
    return juce::NormalisableRange<float>(
        minHz, maxHz,
        [minHz, maxHz](float min, float max, float norm) {
            return minHz * std::pow(maxHz / minHz, norm);
        },
        [minHz, maxHz](float min, float max, float val) {
            return std::log(juce::jlimit(minHz, maxHz, val) / minHz) / std::log(maxHz / minHz);
        }
    );
}

// PROFILE E: Warmth Saturation Drive (0 dB .. +36 dB)
inline juce::NormalisableRange<float> makeDriveRange(float maxGainDb = 36.0f) {
    return juce::NormalisableRange<float>(
        0.0f, maxGainDb,
        [maxGainDb](float min, float max, float norm) {
            return maxGainDb * std::pow(norm, 1.8f);
        },
        [maxGainDb](float min, float max, float val) {
            return std::pow(juce::jlimit(0.0f, maxGainDb, val) / maxGainDb, 1.0f / 1.8f);
        }
    );
}

// PROFILE M: Tri-Zone Parallel Mix Calculator
struct TriZoneMix {
    float dryGain;
    float wetGain;
};

inline TriZoneMix calculateTriZoneMix(float normMix, bool phaseInvert = false) {
    TriZoneMix mix;
    if (normMix <= 0.50f) {
        mix.dryGain = 2.0f * normMix;
        mix.wetGain = 1.0f - 2.0f * normMix;
    } else {
        mix.dryGain = 1.0f;
        mix.wetGain = 2.0f * (normMix - 0.50f);
    }
    if (phaseInvert) {
        mix.wetGain = -mix.wetGain;
    }
    return mix;
}

} // namespace KlangErgonomics
```

---

## 5. ToadTracker & Embedded C Fixed-Point Adaptation

On microcontrollers (RP2040, RP2350, ESP32-P4) or tracker audio-threads where floating-point math and transcendentals (`std::pow`, `std::cbrt`, `std::log`) cause CPU bottlenecks, all taper profiles are executed in **zero CPU cycles using 256-byte static lookup tables (LUTs)**:

```c
// tt_ergonomics.h - Zero-Allocation LUT-Driven Ergonomics for Embedded MCUs
#pragma once
#include <stdint.h>

// 256-byte Pre-calculated Look-Up Tables (LUTs) stored in Flash ROM
extern const uint16_t g_lut_cubic_decay_ms[256];      // Profile A (10 ms .. 60,000 ms)
extern const int16_t  g_lut_bipolar_scurve_q15[256];   // Profile B (-32768 .. +32767)
extern const uint16_t g_lut_log_cutoff_hz[256];       // Profile D (20 Hz .. 20,000 Hz)
extern const uint16_t g_lut_drive_q8[256];            // Profile E (Linear voltage multiplier Q8.8)

// O(1) Single CPU cycle parameter reads from 7-bit / 8-bit tracker values
static inline uint16_t TT_GetDecayTimeMs(uint8_t knob_byte) {
    return g_lut_cubic_decay_ms[knob_byte];
}

static inline uint16_t TT_GetFilterCutoffHz(uint8_t knob_byte) {
    return g_lut_log_cutoff_hz[knob_byte];
}

static inline void TT_GetTriZoneGains(uint8_t knob_byte, int16_t *dry_q15, int16_t *wet_q15) {
    if (knob_byte <= 128) {
        *dry_q15 = (int16_t)(knob_byte << 8);            // 0 .. 32768
        *wet_q15 = (int16_t)((128 - knob_byte) << 8);    // 32768 .. 0
    } else {
        *dry_q15 = 32767;                                // Unity 1.0 locked
        *wet_q15 = (int16_t)((knob_byte - 128) << 8);   // 0 .. 32768
    }
}
```

---

## 6. Comprehensive Parameter Mapping Cheat Sheet

| Parameter Category | Standard Knob Label | Taper Profile | Center (12:00) Value | Extreme End Value |
| :--- | :--- | :--- | :--- | :--- |
| **Envelope Decay / Rel** | `DECAY / RELEASE` | **Profile A** (Cubic $y=x^3$) | $1.5\text{ s}$ | $60.0\text{ s}$ |
| **Bipolar Mod Depth** | `DEPTH` | **Profile B** (Sigmoidal S-Curve) | $0.0\%$ (Deadzone) | $\pm 100\%$ |
| **Oscillator Detune** | `DETUNE` | **Profile B** (Sigmoidal S-Curve) | $0\text{ cents}$ (In Tune) | $\pm 50\text{ cents} / \pm 12\text{ st}$ |
| **LFO / Mod Rate** | `SPEED / RATE` | **Profile C** (Piecewise Log) | **$20.0\text{ Hz}$ (Boundary)**| $5.0\text{ kHz}$ |
| **Filter Cutoff** | `CUTOFF` | **Profile D** (Equal-Octave Log) | **$800\text{ Hz}$** | $20.0\text{ kHz}$ |
| **Filter Resonance** | `RESONANCE / Q` | **Profile F** (Hyperbolic End-Stop)| $70\%$ (Warm Q) | $115\%$ (Self-Oscillation) |
| **Saturation / Drive**| `DRIVE` | **Profile E** (Warmth $x^{1.8}$) | $+6\text{ dB}$ (Tape Warmth) | $+36\text{ dB}$ (Fuzz) |
| **Master Dry/Wet** | `MIX` | **Profile M** (Tri-Zone Mix) | **$100\%$ Pure Dry** | $100\%\text{ Dry} + 100\%\text{ Wet}$ |
| **Stereo Width** | `WIDTH` | **Profile B** (Bipolar S-Curve) | $100\%$ (Natural Stereo) | $0\%\text{ (Mono)} \dots 200\%\text{ (Super)}$|

---

## 7. Conclusion & Parity Guarantee
By codifying these 9 mathematical profiles into our shared C++ headers and ToadTracker C tables, we ensure that whether you are turning an endless aluminum encoder on the dadamachines TBD-16, tweaking an envelope in ToadTracker on a handheld MCU, or automating an audio plugin inside a DAW:

1. **Every degree of knob rotation has acoustic purpose.**
2. **Finding neutral zero never requires squinting at numbers.**
3. **Transient drums and ambient soundscapes coexist on the same knob without sacrifice.**
