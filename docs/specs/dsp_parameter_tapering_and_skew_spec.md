# Perceptual Parameter Tapering & Skewing Standard

**Document Status:** CANONICAL DSP ERGONOMIC SPECIFICATION 📜  
**Target Milestone:** v0.5.0 ("The Sound & Chaos Update")  
**Ecosystem Pillars:** The Klang Suite (`TKS`), ToadTracker (`TT`), The Klang Mill (`TKM`)  
**Lead Architect:** Antigravity x Cody G.  
**Date:** 2026-10-15  

---

## 1. Executive Summary & Philosophy

Human perception of acoustic phenomena—frequency, loudness, time, and harmonic saturation—is fundamentally non-linear (logarithmic, exponential, or power-law). 

When software synthesizers employ raw linear parameter ranges or uncalibrated curves, they inflict **"Ergonomic Cliffing"**: 80% of the useful musical sweet-spot is crammed into a tiny 5-degree arc of an encoder, while the remaining 90% of travel feels dead, jumpy, or unmusical.

**The Golden Invariant for v0.5.0**:
> No parameter in The Klang Suite shall use an unskewed linear scale unless it controls a fundamentally linear discrete entity (e.g. integer octaves, voice counts, or binary toggles). Every continuous parameter must be mathematically tapered to its perceptual sweet-spot so that **100% of physical encoder rotational sweep feels musical, responsive, and predictable.**

---

## 2. The 6 Canonical Perceptual Taper Profiles

```
PROFILE A: Temporal / Envelope (Cubic Power Warping y = x^3)
[ 10 ms ───────────────────────── 1.5 s ───────────────────────── 60.0 s ]
▲ 0%–50% Travel: High-res transient punch     ▲ 50%–100%: Long ambient drones

PROFILE B: Envelope Slope / Tension (Bipolar Sigmoidal S-Curve)
[ Extreme Concave ────────────── [ LINEAR 12:00 ] ───────────── Extreme Convex ]
▲ Center deadzone plateau prevents accidental curve bending around zero

PROFILE C: Wide-Range Mod Speed (Dual-Regime Geometric Skew)
[ 0.02 Hz ─────────────────────── [ 20 Hz 12:00 ] ────────────── 5.0 kHz ]
▲ 0%–50%: Hypnotic rhythmic LFO               ▲ 50%–100%: Searing audio-rate FM

PROFILE D: Filter Frequency (Equal-Octave Logarithmic Skew)
[ 20 Hz ───────────────────────── [ 800 Hz 12:00 ] ───────────── 20.0 kHz ]
▲ Exactly equal degrees of encoder rotation per musical octave

PROFILE E: Non-Linear Drive & Fold (Warmth Expansion + End-Stage Fuzz)
[ 0 dB ────────────────────────── [ +6 dB 12:00 ] ────────────── +36 dB ]
▲ 0%–60%: Delicate tape & diode warmth        ▲ 60%–100%: Screaming saturation

PROFILE F: Runaway Feedback & Q (Hyperbolic End-Stop Expansion)
[ 0% ──────────────────────────── 70% ──────────── 95% ───────── 115% ]
▲ Surgical millimeter precision in the dub self-oscillation danger zone
```

---

### Profile A: Temporal / Envelope Decay Taper ($y = x^3$ Cubic)
* **Target Parameters:**
  - `OSC-01`: Voice Decay ($10\,\text{ms} \dots 60.0\,\text{s}$)
  - `OSC-05`: Pluck $RT_{60}$ ($20\,\text{ms} \dots 60.0\,\text{s}$)
  - `OSC-09`: Sub Decay ($10\,\text{ms} \dots 60.0\,\text{s}$)
  - `OSC-11`: Body Decay & Sizzle Decay ($10\,\text{ms} \dots 60.0\,\text{s}$)
  - Master Amplitude, Filter, and Pitch Envelope Decay/Release stages.
* **Mathematical Function:**
  $$T(x) = T_{\min} + (T_{\max} - T_{\min}) \cdot x^3 \quad \text{for } x \in [0.0, 1.0]$$
* **Perceptual Behavior:**
  - At $x = 0.0$: $10\,\text{ms}$ (instant tick/click).
  - At $x = 0.25$: $50\,\text{ms}$ (tight 808/606 closed hat).
  - At $x = 0.50$ (12 o'clock): $\approx 1.5\,\text{s} \dots 2.5\,\text{s}$ (natural open hat / ride / punchy bass note).
  - At $x = 1.0$: $60.0\,\text{s}$ (monumental drone wash).
* **Benefit:** 50% of the knob is dedicated to percussive drum hits; the upper 50% glides into cinematic ambient sustain.

---

### Profile B: Envelope Slope & Tension (Bipolar Sigmoidal S-Curve)
* **Target Parameters:**
  - Envelope Slope / Tension knobs (Attack Slope, Decay Slope, Pitch Glide Curve).
* **Mathematical Function:**
  $$S(x) = \text{sgn}(x) \cdot |x|^p \quad \text{where } x \in [-1.0, 1.0], \; p = 2.2$$
  Or normalized over $[0.0, 1.0]$ with center $0.5$:
  $$u = 2(x - 0.5), \quad S(u) = \text{sgn}(u) \cdot |u|^{2.2}$$
* **Perceptual Behavior:**
  - A gentle **center plateau** around 12 o'clock ($u = 0.0$): subtle nudges around center do not violently alter the envelope shape, making it effortless to dial in pure linear response.
  - As you approach $\pm 1.0$, the curvature accelerates rapidly into extreme exponential snaps or logarithmic drops.

---

### Profile C: Wide-Range Modulator Speed (Dual-Regime Geometric Skew)
* **Target Parameters:**
  - `OSC-01 KlangTribe R`: Mod Speed ($0.1\,\text{Hz} \dots 5.0\,\text{kHz}$)
  - `OSC-02 KlangAlpha`: PWM Speed ($0.02\,\text{Hz} \dots 2.5\,\text{kHz}$)
  - `FX-20`: Ring Modulator Carrier Frequency ($0.5\,\text{Hz} \dots 4.0\,\text{kHz}$)
* **Mathematical Function:**
  Split piecewise or log-scaled with a designated inflection point at **$20\,\text{Hz}$** (the perceptual threshold between rhythmic tempo and audible pitch):
  $$f(x) = \begin{cases} 
  f_{\min} \cdot \left(\frac{20.0}{f_{\min}}\right)^{2x} & x \le 0.5 \\
  20.0 \cdot \left(\frac{f_{\max}}{20.0}\right)^{2(x - 0.5)} & x > 0.5
  \end{cases}$$
* **Perceptual Behavior:**
  - **Left Half ($0.0 \dots 0.5$):** Glacial LFO sweeps, techno modulation, and polyrhythmic tremolos ($0.1\,\text{Hz} \dots 20\,\text{Hz}$).
  - **Right Half ($0.5 \dots 1.0$):** Growling sub-audio motorboating crossing into screaming metallic FM/AM sidebands ($20\,\text{Hz} \dots 5.0\,\text{kHz}$).

---

### Profile D: Filter Cutoff Frequency (Equal-Octave Logarithmic)
* **Target Parameters:**
  - All SVF, Moog Ladder, Formant, and Cymbal Sizzle Cutoff filters ($20\,\text{Hz} \dots 20.0\,\text{kHz}$).
* **Mathematical Function:**
  $$F(x) = 20.0 \cdot \left(\frac{20000.0}{20.0}\right)^x = 20.0 \cdot (1000.0)^x$$
* **Perceptual Behavior:**
  - $x = 0.0$: $20\,\text{Hz}$ (sub bass)
  - $x = 0.33$: $\approx 200\,\text{Hz}$ (punchy low-mids)
  - $x = 0.50$ (12 o'clock): $\approx 632\,\text{Hz} \dots 800\,\text{Hz}$ (vocal body / core instrument presence)
  - $x = 0.75$: $\approx 3.5\,\text{kHz}$ (presence / bite)
  - $x = 1.0$: $20.0\,\text{kHz}$ (air / wide open)
* **Benefit:** Each 10% of knob rotation equals exactly 1 musical octave of spectral expansion.

---

### Profile E: Non-Linear Drive & Folding (Warmth Expansion)
* **Target Parameters:**
  - `OSC-05`: Pluck Loop Drive
  - `OSC-09`: Sine Fold & Noise Ring
  - `OSC-11`: Wavefolder Pre-Diffusion
  - `FX-01 / FX-02 / FX-08`: Soft Clipper, Hard Clipper, The XXXL Harmonic Warp.
* **Mathematical Function:**
  $$D(x) = D_{\max} \cdot x^{1.8}$$
* **Perceptual Behavior:**
  - First 60% of travel provides delicate, creamy tape saturation, odd-order glue, and warm 2nd harmonic thickness.
  - Final 40% digs into aggressive diode clippers, gnarly square fuzz, and violent Buchla wavefolding.

---

### Profile F: Runaway Feedback & Q-Factor (Hyperbolic End-Stop)
* **Target Parameters:**
  - Delay Feedback ($0\% \dots 120\%$)
  - Comb Filter Feedback ($0\% \dots 99.5\%$)
  - Filter Resonance / Q ($0.5 \dots 25.0$)
* **Mathematical Function:**
  $$Q(x) = Q_{\min} + (Q_{\max} - Q_{\min}) \cdot \frac{1 - \exp(-3x)}{1 - \exp(-3)}$$
* **Perceptual Behavior:**
  - Expanded rotational resolution in the $70\% \dots 98\%$ range, allowing precise control over runaway dub echoes and singing resonant pings without accidental deafening feedback explosions.

---

### Profile M: Universal Tri-Zone Parallel Mix Law & Phase Invert Protocol
* **Target Parameters:**
  - Universal `DRY/WET MIX` knob across ALL Audio Effects (`KlangWarp`, `KlangStomp`, `KlangPunch`, `KlangFilter`, `KlangMod`, `KlangShift`, `KlangDelay`, `KlangSpace`, and Bespoke pedals).
* **The Ergonomic Breakthrough:**
  Standard linear 0%–100% dry/wet crossfaders force you to lose your dry punch, transients, and fundamental body as soon as you blend wet signal in. The **Tri-Zone Parallel Mix Law** redefines the knob into three dedicated operational zones with **12 o'clock as Pure Dry**:
  ```
  [ PURE WET (7:00) ────── [ PURE DRY (12:00) ] ────── PARALLEL DRY + WET (5:00) ]
  ▲ 100% Wet / 0% Dry      ▲ 100% Dry / 0% Wet         ▲ 100% Dry + 100% Wet Bleed
  ```
* **Mathematical Transfer Function ($u \in [0.0, 1.0]$, center at $u = 0.5$):**
  - **Left Wing ($u \in [0.0, 0.5)$ — Audition & Wet Crossfade):**
    $$\text{Dry}(u) = 2u, \quad \text{Wet}(u) = 1.0 - 2u$$
    *(At $u = 0.0$, output is 100% Pure Wet. Ideal for auditioning isolated effects, inserts used as aux sends, or pure vibrato/pitch shifting).*
  - **Center ($u = 0.5$ — Neutral Pure Dry):**
    $$\text{Dry}(0.5) = 1.0, \quad \text{Wet}(0.5) = 0.0$$
    *(Instant, intuitive zero-effect neutral home position without clicking bypass).*
  - **Right Wing ($u \in (0.5, 1.0]$ — True Parallel Injection):**
    $$\text{Dry}(u) = 1.0, \quad \text{Wet}(u) = 2(u - 0.5)$$
    *(Dry punch, transients, and sub-bass remain 100% locked at unity gain, while the wet processed signal is smoothly bled in parallel on top!)*
* **Phase Invert ($\varnothing \in \{+1, -1\}$) Push Action:**
  - **Pushing / Clicking the Mix Knob** (or flipping the rear deck dip switch) inverts the phase of the wet signal ($180^\circ$ polarity flip):
    $$y(t) = \text{Dry}(u) \cdot x(t) + \varnothing \cdot \text{Wet}(u) \cdot x_{\text{wet}}(t)$$
  - **Sonic Impact:**
    - On Chorus / Flanger: Inverts the wet delay, creating hollow through-zero tube cancellation and negative comb notches.
    - On Distortion / Fuzz: Subtracts the distortion from the dry signal, producing dynamic transient extraction and hollow harmonic filtering!
    - On Filters: Subtracting lowpass from dry yields highpass; subtracting bandpass yields notch!

---

## 3. C++ JUCE Implementation Standard (Milestone v0.5.0)

In JUCE APVTS definitions, all parameters MUST be instantiated using `juce::NormalisableRange<float>` configured with their exact profile lambdas or skew factors:

```cpp
// Example: Profile A (Cubic Decay) in JUCE
auto decayRange = juce::NormalisableRange<float>(
    0.010f,  // 10 ms
    60.0f,   // 60 s
    [](float min, float max, float norm) {
        return min + (max - min) * (norm * norm * norm); // y = x^3
    },
    [](float min, float max, float val) {
        return std::cbrt(juce::jlimit(0.0f, 1.0f, (val - min) / (max - min)));
    }
);

// Example: Profile C (Piecewise LFO-to-Audio Rate)
auto modSpeedRange = juce::NormalisableRange<float>(
    0.1f,    // 0.1 Hz
    5000.0f, // 5.0 kHz
    [](float min, float max, float norm) {
        if (norm <= 0.5f) {
            float n = norm * 2.0f;
            return min * std::pow(20.0f / min, n);
        } else {
            float n = (norm - 0.5f) * 2.0f;
            return 20.0f * std::pow(max / 20.0f, n);
        }
    },
    [](float min, float max, float val) {
        if (val <= 20.0f) {
            return 0.5f * (std::log(val / min) / std::log(20.0f / min));
        } else {
            return 0.5f + 0.5f * (std::log(val / 20.0f) / std::log(max / 20.0f));
        }
    }
);
```

---

## 4. Cross-Reference & Execution Checklist
* [x] Canonical specification created.
* [x] Linked into `master_sound_and_fx_catalog.md` Section 2.
* [ ] Staged as mandatory acceptance criteria for Milestone v0.5.0 C++ APVTS parameter registration.
