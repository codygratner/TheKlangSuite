# Technical DSP Specification: `PipKlick` (Wavetable Waveshaper & Decimator)

**Engine ID:** `FX-03`  
**Sovereign Name:** `PipKlick`  
**Corporate Boring Name:** `Dynamic Decimator, Bitcrusher & Wavetable Waveshaper`  
**Category:** Lo-Fi & Digital Disruption Processor  
**APVTS Prefix:** `pip_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section C](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-c)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** OTO Machines Biscuit (8-bit stereo effects processor with bit-inversion and analog filter), WMD Geiger Counter (wavetable waveshaper + wavetable distortion), SPL Transient Designer (dual VCA transient engine).
* **Upstream Offline Reference:** [spl_transient_designer_manual.md](file:///C:/Dev/Research/library/manuals/spl_transient_designer_manual.md) · [01_renoise_complete_offline_guide.md](file:///C:/Dev/Research/library/01_renoise_complete_offline_guide.md).
* **C++ Implementation Header:** [`PipKlickDecimator.h`](file:///c:/Dev/TheKlangDSP/include/dsp/fx/PipKlickDecimator.h).

---

## 2. Mathematical Theory & DSP Architecture

PipKlick deconstructs audio transients and destroys bitstream fidelity through three interactive stages:
1. **Sample Rate Decimation & Sample-and-Hold:**
   Downsamples audio by holding the signal over $N$ fractional samples using a variable clock accumulator:
   $$y[n] = x\left[\lfloor n / M \rfloor \cdot M\right], \quad M = \frac{f_s}{f_{crush}}$$
2. **Dynamic Bit-Depth Reduction with Dither Manipulation:**
   Quantizes the linear dynamic range to $B$ bits ($1 \le B \le 16$), with selectable TPDF dither or raw truncation:
   $$y_q[n] = \frac{\text{round}\left(x[n] \cdot (2^{B-1} - 1)\right)}{2^{B-1} - 1}$$
3. **8-Bit Logic Inversion & Wavetable Waveshaping:**
   Incoming byte values are mapped into a 256-entry transfer function lookup table or bitwise boolean mutated:
   $$\text{Byte}_{out} = \text{Byte}_{in} \oplus \text{BitMask}$$
   This allows individual bits (e.g. MSB or Bit 3) to be muted, inverted, or crossed with zero-crossing gates.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`SAMPLE CRUSH`):** Effective sample rate ($200\,\text{Hz} \dots 48.0\,\text{kHz}$, Profile D Log). *Push: Snap 8kHz Telephone*.
* **Knob 2 (`BIT DEPTH`):** Word length resolution ($1.0 \dots 16.0\,\text{bits}$, Profile A). *Push: Snap 8-Bit Vintage*.
* **Knob 3 (`WAVETABLE SHAPE`):** Discrete index `0..15` of transfer tables (Sine Fold, Chebyshev, Diode, Broken Glass). *Push: Next Shape*.
* **Knob 4 (`BIT MASK / INVERT`):** Boolean mask `0x00..0xFF` for bit muting and polarity flipping. *Push: Invert MSB (Bit 7)*.
* **Knob 5 (`JITTER / FLUTTER`):** High-frequency clock jitter causing noisy sideband dispersion ($0.0\% \dots 100.0\%$, Profile E). *Push: Mute Jitter*.
* **Knob 6 (`POST ANALOG TILT`):** 2-pole analog reconstruction lowpass filter ($1.0\,\text{kHz} \dots 20.0\,\text{kHz}$, Profile D). *Push: Bypass Filter*.
* **Knob 7 (`DRIVE / PRE-GAIN`):** Input gain into the quantizer ($0.0\,\text{dB} \dots +24.0\,\text{dB}$, Profile E). *Push: Zero dB*.
* **Knob 8 (`MIX`):** Universal Tri-Zone Dry/Wet Mix (Profile M: 7:00 Wet Audition, 12:00 Pure Dry, 5:00 Parallel). *Push: Phase Invert ($\\varnothing = 180^\\circ$)*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Quantizer Law):** `[ Linear Uniform | A-Law Companding | Mu-Law Telephony | Logarithmic ]`.
* **DIP 2 (Dither Generation):** `[ Rectangular Dither (RPDF) | Triangular (TPDF) | High-Pass Noise Shaped | Raw Truncation ]`.
* **DIP 3 (Bit Inversion Target):** `[ MSB Only | Bits 3 & 4 | Odd Bits | Even Bits ]`.
* **DIP 4 (Reconstruction Topology):** `[ Crisp Zero-Order Hold | 4-Pole Sallen-Key | Clean Butterworth 24dB ]`.
* **TRIM 1 (Clock Drift Speed):** Thermal drift speed on the sample rate clock ($0.05\,\text{Hz} \dots 2.0\,\text{Hz}$).
* **TRIM 2 (Sub-Bass Bleed):** Dry low-frequency passthrough below $120\,\text{Hz}$ to keep kicks heavy.
* **TRIM 3 (Aliasing Resonance):** Peak resonance boost around the Nyquist mirror frequency.
* **TRIM 4 (Noise Gate Ceiling):** Mutes lo-fi digital hash when input drops below silence threshold.

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Quantizer Mode (DIP 1) | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`A-Law / Mu-Law`** | `Knob 2` | `COMPAND EXP` | `1.0..8.0` | `Profile B` | Adjusts logarithmic compression slope before crushing |
| **`Wavetable Shaping`** | `Knob 4` | `DRIVE FOLD` | `0..100%` | `Profile E` | Intercept drive for non-linear table lookups |
| **`Raw Truncation`** | `Knob 5` | `QUANT NOISE` | `-inf..0 dB` | `Profile B` | Direct control of truncation noise ceiling |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Fast Bit Manipulation:** Bitmasking and logic inversion operate directly on integer casts without branching in the inner DSP loop.
* **Anti-Denormal Safeguard:** All accumulator floats flushed to zero using FTZ/DAZ SSE register intrinsics.
