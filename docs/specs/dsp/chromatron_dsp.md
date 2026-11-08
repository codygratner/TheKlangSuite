# Technical DSP Specification: `Chromatron` (Chromatic Key & Scale Tracking Matrix)

**Engine ID:** `CARD-05B`  
**Sovereign Name:** `Chromatron`  
**Corporate Boring Name:** `Chromatic Key Tracking, Micro-Tuning & Scale Matrix`  
**Category:** Expressive Modulation & Acoustic Key Tracking Matrix  
**APVTS Prefix:** `chrom_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section D](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Oberheim Matrix-12 key-tracking matrix, Moog 1V/Oct keyboard control, Yamaha DX7 micro-tuning tables, Korg Prophecy acoustic modeling keytrack.
* **Upstream Offline Reference:** [01_renoise_complete_offline_guide.md](file:///C:/Dev/Research/library/01_renoise_complete_offline_guide.md) · [ROSETTA_STONE.md#section-d](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d).
* **C++ Implementation Header:** [`ChromatronKeyMatrix.h`](file:///c:/Dev/TheKlangDSP/include/dsp/matrix/ChromatronKeyMatrix.h).

---

## 2. Mathematical Theory & DSP Architecture

Chromatron models acoustic instrument physics across keyboard registers, providing key tracking, micro-tuning, and anti-aliasing safeguards:
1. **Acoustic Register Scaling Physics:**
   In physical acoustic instruments (violins, pianos, bells), higher notes possess naturally shorter decay times, less low-frequency energy, and higher spectral density. Chromatron uses MIDI note number $N$ ($0 \dots 127$) relative to a pivot center note $N_0$ (default C3 = 60) to modulate 8 parameters:
   $$\Delta_{\text{semitones}} = N - N_0$$
2. **Hyper-Track Filter Scaling ($\pm 400\%$):**
   Standard 1V/Oct filter key tracking opens the filter by 1 octave per keyboard octave ($100\%$). Chromatron's **Hyper-Track** extends this to $\pm 400\%$, allowing dramatic inverted tracking (high notes become deeply muffled, low notes scream) or supersonic opening.
3. **8 Simultaneous Key-Track Destinations:**
   - Channel 1: $\to$ **Filter Cutoff** ($-400\% \dots +400\%$).
   - Channel 2: $\to$ **Envelope Decay Damping** (High notes decay shorter).
   - Channel 3: $\to$ **Spectral Tilt EQ** (Balances high vs low keyboard energy).
   - Channel 4: $\to$ **FM Mod Index / Ratio** (Tracks inharmonic brightness).
   - Channel 5: $\to$ **Stereo Pan Spread** (Pans keyboard left-to-right like a grand piano).
   - Channel 6: $\to$ **Envelope Attack Snappiness** (Faster attack in upper register).
   - Channel 7: $\to$ **Inharmonic Stretch / Rail** (Piano string inharmonicity $f_k \approx k f_0 \sqrt{1 + B k^2}$).
   - Channel 8: $\to$ **Anti-Alias High Cut** (Automatically clamps high notes before Nyquist).

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`PIVOT NOTE`):** Center reference key ($C0 \dots C8$, Default `C3 / Note 60`, Profile B). *Push: Set Pivot to Current Key*.
* **Knob 2 (`-> CUTOFF (HYPER)`):** Key tracking depth to filter cutoff ($-400.0\% \dots +400.0\%$, Profile B). *Push: Snap 1V/Oct (100%)*.
* **Knob 3 (`-> DECAY DAMP`):** Key scaling to envelope decay ($-100.0\% \dots +100.0\%$, Profile B). *Push: Acoustic Piano Damp*.
* **Knob 4 (`-> TILT EQ`):** Keyboard register spectral tilt balance ($-6\,\text{dB/oct} \dots +6\,\text{dB/oct}$, Profile B). *Push: Reset Flat*.
* **Knob 5 (`-> FM TIMBRE`):** Key tracking to FM modulator level ($-100.0\% \dots +100.0\%$, Profile B). *Push: Reset Zero*.
* **Knob 6 (`-> PAN SPREAD`):** Grand piano stereo keyboard spread ($0.0\% \dots 100.0\%$, Profile B). *Push: Mono Center (0%)*.
* **Knob 7 (`-> ATTACK ACCEL`):** Upper register attack acceleration ($0.0\% \dots 100.0\%$, Profile B). *Push: Reset Zero*.
* **Knob 8 (`-> PITCH STRETCH`):** Rail inharmonicity pitch stretch ($0.0\% \dots 100.0\%$, Profile B). *Push: Concert Grand Rail*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Scale Micro-Tuning):** `[ 12-TET Standard | Just Intonation | Werckmeister III | 24-EDO Quarter-Tone ]`.
* **DIP 2 (Pivot Mode):** `[ Fixed C3 Anchor | Floating Dynamic Moving Average | Lowest Held Note ]`.
* **DIP 3 (Anti-Aliasing Shield):** `[ Active Brickwall Nyquist Shield (0.45 Fs) | Soft 6dB Roll-Off | Off ]`.
* **DIP 4 (Keytrack Curve):** `[ Linear 1V/Oct | Logarithmic Smooth | S-Curve Register Split ]`.
* **TRIM 1 (Micro-Tuning Root Key):** Root note for non-12-TET scale temperaments (C through B).
* **TRIM 2 (Upper Register High-Cut Ceiling):** Cutoff frequency of the ultrasonic protection filter ($14\,\text{kHz} \dots 22\,\text{kHz}$).
* **TRIM 3 (Keytrack Glide Slew):** Portamento slew applied to keytrack modulation offsets.
* **TRIM 4 (Sub-Bass Fundamental Boost):** Low-frequency shelving boost applied below note C1 ($32.7\,\text{Hz}$).

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Scale Mode (DIP 1) | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`Micro-Tuning Modes`**| `Knob 8` | `SCALE ROOT` | `C..B` | `Discrete` | Selects tonic note for historic temperaments |
| **`Hyper-Track Range`** | `Knob 2` | `HYPER SLOPE` | `±400%` | `Profile B` | Extreme inverted or supersonic spectral tracking |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Table-Driven Micro-Tuning:** 128-entry float frequency lookup table updated once upon tuning scale selection; zero transcendental calculations in audio block.
* **Nyquist Safeguard:** Filter cutoffs clamped to $0.48 \cdot f_s$ to prevent ZDF filter solver divergence.
