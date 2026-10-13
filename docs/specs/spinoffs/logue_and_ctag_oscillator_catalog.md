# Korg Logue SDK & CTAG DSP Oscillator Catalog & Integration Specification

**Status:** APPROVED ARCHITECTURAL SPECIFICATION 📜  
**Target:** The Klang Farmer (`TKF`) & The Klang Planter (`TKP`)  
**Scope:** Native C++ DSP Oscillator Ingestion (`Source/dsp/OscillatorModule.h`)  
**Ecosystem Pillars:** TKS / TKR  
**Author:** New Klang City Architecture & Tooling Engineering  
**Date:** 2026-10-13  

---

## 1. Executive Summary & Dialectical Intent

### 1.1 The Core Intent
To radically expand the sonic palette of *The Klang Farmer* and *The Klang Planter* beyond basic CRT virtual-analog waveforms, by integrating battle-tested, high-character open-source (FOSS) oscillator algorithms from two prominent digital synthesis platforms:
1. **The Korg Logue SDK Ecosystem** (`prologue`, `minilogue xd`, `drumlogue`, `NTS-1 / mkII / NTS-3`).
2. **CTAG (Creative Technologies Audio Group) / TBD DSP Modular Synthesis**.

### 1.2 The Dialectical Invariant: FOSS Only & Zero Commercial Entanglement
- **Permissive FOSS Exclusivity**: Under NO circumstances will proprietary, paid, or closed-source commercial user units (e.g. Sinevibes commercial units, Tim Shoebridge commercial plugins) be reverse-engineered or referenced.
- **Strict License Audit**: Only algorithms licensed under **MIT**, **BSD-3-Clause**, **Apache-2.0**, or **GPLv3** are cataloged and ingested.
- **Brand Disclaimer**: "Korg", "logue SDK", "minilogue", and "prologue" are registered trademarks of Korg Inc. Our implementation is an independent clean-room host adapter consuming public open-source C++ DSP routines.

---

## 2. Platform Architecture & Audio Thread Invariant Alignment

### 2.1 The Embedded MCU Advantage: Built-in Real-Time Safety
Korg Logue SDK user oscillators are compiled for bare-metal ARM Cortex-M4 / Cortex-M7 microcontrollers with severe hardware constraints (32KB–64KB SRAM, zero OS, single-core real-time execution).

As a direct consequence of this hardware environment:
| Audio Thread Invariant | Embedded Logue Constraint | TKS Real-Time Safety Guarantee |
| :--- | :--- | :--- |
| **Zero Allocations** | `malloc()`, `new`, dynamic containers fail compilation | **100% Compliant by Design** |
| **Zero Locks** | Bare-metal single-core execution; no mutexes | **100% Lock-Free by Design** |
| **Zero Blocking I/O** | No filesystem, no console, no network stack | **100% Zero-I/O Compliant** |
| **Buffer-Oriented Render**| Standard 64-sample frames rendered in-place | **Direct 1:1 Match with JUCE buffers** |

---

## 3. The FOSS Oscillator Catalog

### 3.1 Mutable Instruments Ports (Émilie Gillet / B-System / Gary Scavone)
*License:* **MIT License**  
*Source Heritage:* Open-source firmware of Mutable Instruments Plaits & Braids ported to Logue C++.

| Oscillator Name | Synthesis Algorithm | Parameter 1 (`Shape`) | Parameter 2 (`Shift-Shape`) | Character & Sonic Role |
| :--- | :--- | :--- | :--- | :--- |
| **`PLAITS_VA`** | Dual virtual-analog with continuous waveshaping & sync | Variable saw-square morph | Sub-oscillator level & sync ratio | Classic Roland/Sequential analog beef, hard sync sweeps. |
| **`PLAITS_FM`** | 2-Operator variable feedback FM with phase modulation | FM modulation index | Frequency ratio & feedback amount | Metallic bells, DX7 bass, glassy tine plucks. |
| **`PLAITS_CHORDS`** | 4-voice paraphonic chord generator with wavetables | Chord type & inversion selector | Harmonic spread & waveform morph | Instant techno/house stabs, lush pads with zero CPU voice cost. |
| **`PLAITS_GRAIN`** | Granular formant & particle synthesis | Formant frequency & vowel morph | Particle density & dispersion jitter | Vocal vowels, choir textures, alien granular chatter. |
| **`PLAITS_ADDITIVE`** | 24-harmonic additive generator with Chebyshev distribution | Spectral tilt (odd/even balance) | Harmonic dispersion & frequency spread | Organ draws, bell resonances, pristine digital overtones. |

---

### 3.2 Casio CZ Phase Distortion Synthesis
*License:* **MIT / BSD-3-Clause**  
*Source Heritage:* Mathematical clean-room emulation of the Casio CZ-101 / CZ-1 Cosmo Synthesizer series.

Instead of traditional low-pass filtering, Phase Distortion alters the readout angle of a cosine accumulator based on an inverted breakpoint curve:

$$\theta_{distorted} = f(\theta_{linear}, \text{shape})$$

| Model ID | Distortion Curve | Parameter 1 (`Shape`) | Parameter 2 (`Shift-Shape`) | Sonic Role |
| :--- | :--- | :--- | :--- | :--- |
| **`CZ_RESO_SAW`** | Resonant Sawtooth (cosine carrier $\times$ windowed sync ramp) | Filter resonance cutoff (0–100%) | Resonance peak sharpness | Classic 1985 laser bass, resonant synth sweeps without filter delay. |
| **`CZ_RESO_TRI`** | Resonant Triangle | Peak resonance frequency | Envelope drive amount | Warm plucks, hollow woodwinds, vintage synth leads. |
| **`CZ_DOUBLE_SINE`**| Symmetrical double-lobe cosine | Frequency multiplication ratio | Waveform clipping threshold | Unique FM-like metallic bite with warm fundamentals. |

---

### 3.3 Non-Linear Wavefolding & Chebyshev Polynominals
*License:* **MIT License**  
*Source Heritage:* Serge / Buchla West Coast modular synthesis & Chebyshev harmonic generators.

| Model ID | Algorithm | Parameter 1 (`Shape`) | Parameter 2 (`Shift-Shape`) | Sonic Role |
| :--- | :--- | :--- | :--- | :--- |
| **`SERGE_FOLD`** | Multi-stage diode wavefolder with DC blocker | Fold drive (1x to 8x fold stages) | Symmetry bias (even harmonic injection) | Rich, aggressive West Coast harmonic generation from pure sines. |
| **`CHEBYSHEV_H`** | Orthogonal Chebyshev polynomials $T_n(x)$ | Order morph (fundamental ➔ $T_5(x)$) | Odd/even harmonic ratio | Mathematically pure harmonic addition without intermodulation noise. |

---

### 3.4 CTAG (Creative Technologies Audio Group) / TBD DSP Modules
*License:* **GPLv3 / BSD-3-Clause**  
*Source Heritage:* University of Applied Sciences Bremen digital eurorack hardware & TBD Audio core DSP.

| Model ID | Algorithm | Parameter 1 (`Shape`) | Parameter 2 (`Shift-Shape`) | Sonic Role |
| :--- | :--- | :--- | :--- | :--- |
| **`CTAG_WAVETABLE`**| Multi-frame wavetable interpolator with bandlimiting | Table index (morph through 64 single cycles) | Interpolation smoothing (linear vs hermite)| Modern digital morphing wavetable synthesis (Serum/Vital style). |
| **`CTAG_VECTOR`** | 2D Quad-corner vector crossfader | X-axis vector position | Y-axis vector position | Dynamic Prophet VS style joystick pads and shifting timbres. |
| **`CTAG_POLYBLEP`** | Low-overhead anti-aliased virtual analog core | Pulse width / waveform morph | Blep transition width | Zero-aliasing, ultra-low CPU everyday daily driver oscillator. |

---

## 4. Architectural Integration Path into The Klang Farmer

### 4.1 The C++ Logue Host Adapter (`Source/dsp/LogueOscillatorAdapter.h`)

To ingest these algorithms without modifying their core DSP routines, we author a zero-allocation C++20 adapter class:

```cpp
namespace TbdAudio
{
    template <typename OscCore>
    class LogueOscillatorAdapter
    {
    public:
        void prepare(double sampleRate, int maxBlockSize) noexcept
        {
            currentSampleRate = sampleRate;
            oscCore.init(sampleRate);
        }

        void setParameters(float pitchHz, float shape, float shiftShape, float param1, float param2) noexcept
        {
            params.pitch = pitchHz;
            params.shape = static_cast<uint16_t>(juce::jlimit(0.0f, 1.0f, shape) * 1023.0f);
            params.shiftshape = static_cast<uint16_t>(juce::jlimit(0.0f, 1.0f, shiftShape) * 1023.0f);
            oscCore.setParams(param1, param2);
        }

        void renderBlock(float* outputBuffer, int numSamples) noexcept
        {
            // Embedded oscillators render in 64-sample sub-frames:
            int samplesRemaining = numSamples;
            float* dest = outputBuffer;
            while (samplesRemaining > 0)
            {
                const int chunk = std::min(samplesRemaining, 64);
                oscCore.process(dest, chunk);
                dest += chunk;
                samplesRemaining -= chunk;
            }
        }

    private:
        double currentSampleRate = 48000.0;
        user_osc_param_t params {};
        OscCore oscCore;
    };
}
```

### 4.2 Data-Driven Schema Parameter Binding (`assets/controls/oscillator.json`)

Parameter contracts remain strictly separated from C++ code per our data-driven architecture:

```json
{
  "id": "osc1_type",
  "name": "Oscillator 1 Engine",
  "type": "choice",
  "default": "Analog_Saw",
  "choices": [
    "Analog_Saw",
    "Analog_Pulse",
    "Analog_Triangle",
    "Plaits_VA",
    "Plaits_FM",
    "Plaits_Chords",
    "Plaits_Grain",
    "Plaits_Additive",
    "CZ_Reso_Saw",
    "CZ_Reso_Tri",
    "Serge_Fold",
    "Chebyshev_Harmonics",
    "CTAG_Wavetable",
    "CTAG_Vector"
  ]
}
```

---

## 5. Implementation Roadmap & Verification

1. **Phase 1 (Ingestion & Harness)**:
   - Create `Source/dsp/engines/` directory with clean-room C++ header-only FOSS routines.
   - Author `Source/dsp/LogueOscillatorAdapter.h`.
2. **Phase 2 (Unit Testing & Audio Thread Audit)**:
   - Author `dsp_tests` asserting zero allocations during sample processing.
   - Verify zero transcendental calls in hot loops via `tools/audit_audiothread.py`.
   - Measure CPU performance benchmark (<0.8% CPU per voice on M1/Ryzen 5).
3. **Phase 3 (APVTS Binding & UI Controls)**:
   - Update `assets/controls/oscillator.json` and `assets/text/strings.json`.
   - Update UI card bindings in `assets/layouts/farmer_page.json`.
