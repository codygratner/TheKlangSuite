# Technical DSP Specification: `Velocitron` (8-Channel Expressive Velocity Matrix)

**Engine ID:** `CARD-05A`  
**Sovereign Name:** `Velocitron`  
**Corporate Boring Name:** `8-Channel Expressive MIDI Velocity Modulation Matrix`  
**Category:** Expressive Modulation & Dynamic Routing Matrix  
**APVTS Prefix:** `velo_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section D](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Akai MPC60 / MPC3000 velocity curves and 16-levels velocity pads, Sequential Prophet-VS expressive velocity matrix, Ensoniq EPS-16+ velocity layers.
* **Upstream Offline Reference:** [01_renoise_complete_offline_guide.md](file:///C:/Dev/Research/library/01_renoise_complete_offline_guide.md) · [ROSETTA_STONE.md#section-d](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d).
* **C++ Implementation Header:** [`VelocitronMatrix.h`](file:///c:/Dev/TheKlangDSP/include/dsp/matrix/VelocitronMatrix.h).

---

## 2. Mathematical Theory & DSP Architecture

Velocitron maps incoming MIDI velocity ($1 \dots 127$) through non-linear perceptual response curves and distributes modulation to 8 internal sound engine parameters simultaneously:
1. **Non-Linear Velocity Response Curves:**
   - **Linear ($x$):** Direct proportional scaling.
   - **Exponential ($x^2$):** Demands aggressive playing to reach maximum brightness.
   - **Logarithmic ($\sqrt{x}$):** High output even with delicate touch; ideal for unweighted synths.
   - **S-Curve (Sigmoidal):** Compressed low and high velocity bands with maximum expressive sensitivity in the mezzo-forte ($mf$) midrange.
   - **Akai MPC Vintage Curve:** Stepped, punchy curve with accentuated transient authority.
2. **8 Simultaneous Destination Matrix Routing:**
   - Channel 1: $\to$ **Amp Level** ($-\infty\,\text{dB} \dots 0\,\text{dB}$).
   - Channel 2: $\to$ **Filter Cutoff** ($-48\,\text{st} \dots +48\,\text{st}$).
   - Channel 3: $\to$ **FM Mod Index / Timbre** ($0.0\% \dots 100.0\%$).
   - Channel 4: $\to$ **Envelope Decay Time** (Harder = Longer / Shorter).
   - Channel 5: $\to$ **Envelope Attack Snappiness** (Harder = Faster Attack).
   - Channel 6: $\to$ **Transient Strike Level** ($0.0\,\text{dB} \dots +18.0\,\text{dB}$).
   - Channel 7: $\to$ **Pitch Snap Extent** (Harder = Stronger Initial Transient Zap).
   - Channel 8: $\to$ **VCA Saturation Drive** (Harder = Grittier Distortion).

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`VEL CURVE`):** Selects velocity mapping curve: `Linear`, `Exp 1`, `Exp 2`, `Log`, `S-Curve`, `MPC Vintage`. Discrete Choice. *Push: Toggle S-Curve*.
* **Knob 2 (`-> AMP LEVEL`):** Velocity depth to master amplitude ($0.0\% \dots 100.0\%$, Profile B). *Push: Snap 100%*.
* **Knob 3 (`-> CUTOFF`):** Velocity depth to filter cutoff frequency ($-100.0\% \dots +100.0\%$, Profile B). *Push: Snap +50%*.
* **Knob 4 (`-> FM TIMBRE`):** Velocity depth to FM modulation index ($0.0\% \dots 100.0\%$, Profile B). *Push: Snap 50%*.
* **Knob 5 (`-> DECAY TIME`):** Velocity scaling to envelope decay ($-100.0\% \dots +100.0\%$, Profile B). *Push: Reset Zero*.
* **Knob 6 (`-> ATTACK TIME`):** Velocity scaling to attack snappiness ($-100.0\% \dots +100.0\%$, Profile B). *Push: Reset Zero*.
* **Knob 7 (`-> PITCH SNAP`):** Velocity depth to transient pitch zap ($0.0\% \dots 100.0\%$, Profile B). *Push: Reset Zero*.
* **Knob 8 (`-> VCA DRIVE`):** Velocity depth to saturation warmth ($0.0\% \dots 100.0\%$, Profile B). *Push: Reset Zero*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Ghost Note Mode):** `[ Mute Ghost Notes (Vel < 15) | Soft Acoustic Response | Equal Dynamics ]`.
* **DIP 2 (Humanize Jitter):** `[ ±5% Random Velocity Jitter | ±10% Jitter | Off Pure Rigorous ]`.
* **DIP 3 (Velocity Polarity):** `[ Unipolar 0..127 | Bipolar Centered at Vel 64 ]`.
* **DIP 4 (Dynamic Limiter):** `[ Hard Velocity Clamp (Max 127) | Soft Companded Ceiling ]`.
* **TRIM 1 (Minimum Velocity Floor):** Clamps the quietest possible note ($1 \dots 40$).
* **TRIM 2 (Maximum Velocity Ceiling):** Clamps the maximum possible note ($80 \dots 127$).
* **TRIM 3 (Random Velocity Speed):** Speed of the humanize random walk generator.
* **TRIM 4 (Velocity Smoothing Slew):** Portamento slew applied to rapid velocity jumps ($0.0\,\text{ms} \dots 50.0\,\text{ms}$).

---

## 4. Contextual Front-Panel Remapping (Dynamic Mode Matrix)

| Ghost Mode (DIP 1) | Affected Encoders | New Display Label | New Units / Range | Taper Profile | Contextual Function |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`Ghost Note Active`** | `Knob 1` | `GHOST THRESHOLD` | `Vel 1..32` | `Profile A` | Sets threshold below which notes trigger ghost sample/muting |
| **`Bipolar Centered`** | `Knob 2` | `MF DEADZONE` | `±0..16` | `Profile B` | Width of neutral velocity plateau around value 64 |

---

## 5. C++ Implementation & Audio-Thread Invariants
* **Precomputed Response LUTs:** Velocity curves evaluated via 128-entry precalculated integer lookup tables for zero-CPU response in note-on callbacks.
* **Lock-Free Modulation Broadcasting:** Modulation values written directly to voice parameter targets via atomic float pointers.
