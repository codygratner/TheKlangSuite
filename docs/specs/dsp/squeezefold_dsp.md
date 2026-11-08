# Technical DSP Specification: `SqueezeFold` (Wave-Multiply & Fold Synthesizer)

**Engine ID:** `OSC-09`  
**Sovereign Name:** `SqueezeFold`  
**Corporate Boring Name:** `Wave-Multiply & Fold Synthesizer`  
**Category:** West Coast Wavefolding & Multiplier Synthesizer Core  
**APVTS Prefix:** `squeezefold_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section B](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-b)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Dirtywave M8 `WAVSYNTH`, Serge Modular Triple Waveshaper, Buchla 259 Wave Multiplier.
* **Upstream Offline Reference:** [m8_shortcut_guide.md](file:///C:/Dev/Research/library/manuals/m8_shortcut_guide.md).
* **C++ Implementation Header:** [`SqueezeFold.h`](file:///c:/Dev/TheKlangDSP/include/dsp/oscillators/SqueezeFold.h).

---

## 2. Mathematical Theory & DSP Architecture

`SqueezeFold` implements West Coast additive timbral generation via horizontal phase duty-cycle compression ("Squeeze") and vertical non-linear wavefolding ("Fold"):

### 1. Horizontal Phase Squeezing
Warping the phase accumulator $\theta \in [0, 1)$ of a single-cycle wave by compressing its active duty cycle into a narrow portion of the period:
$$\theta_{sq}[n] = \begin{cases} \frac{\theta[n]}{S} & 0 \le \theta[n] < S \\ 1.0 & S \le \theta[n] < 1.0 \end{cases}$$
where $S \in [0.05, 1.0]$ is the squeeze factor. As $S \to 0$, the wave accelerates into a high-frequency chirp followed by silence, generating formant-like sidebands.

### 2. Multi-Stage Non-Linear Wavefolding
Passing the squeezed wave $x[n]$ through a multi-stage hyperbolic wavefolder with dynamic DC bias:
$$y_{fold}[n] = \sin\left( \pi \cdot k_{fold} \cdot x[n] + V_{bias} \right)$$
For extreme settings, cascaded Serge diode equations introduce gritty intermodulation and harmonic multiplication.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`PITCH`):** Master fundamental ($20\text{ Hz} \dots 4000\text{ Hz}$, Profile D). *Push: Octave Transpose*.
* **Knob 2 (`SHAPE`):** Single-Cycle Seed Wave (Sine, Triangle, Saw, Pulse, Custom SCW). *Push: Cycle Wave*.
* **Knob 3 (`MULTIPLY`):** Frequency multiplier harmonic integer ($1\times \dots 16\times$, Profile B). *Push: Reset 1x*.
* **Knob 4 (`PHASE SQUEEZE`):** Horizontal phase duty compression ($0\% \dots 90\%$, Profile B). *Push: Zero Squeeze*.
* **Knob 5 (`WAVEFOLD`):** Non-linear wavefolding depth ($0\% \dots 100\%$, Profile E). *Push: Max Fold Squelch*.
* **Knob 6 (`MIRROR / BIAS`):** Asymmetric DC bias before folding. Profile B. *Push: Zero DC Bias*.
* **Knob 7 (`DECAY`):** Internal envelope decay ($10\text{ ms} \dots 10.0\text{ s}$, Profile A). *Push: Tempo Snap*.
* **Knob 8 (`DRIVE`):** Pre-fold transistor drive ($0\text{ dB} \dots +24\text{ dB}$, Profile E). *Push: +12dB Boost*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Fold Algorithm):** `[ Buchla Triangle Fold | Serge Diode Fold | M8 Wavsynth Digital Clip | Sine Fold ]`.
* **DIP 2 (Phase Topology):** `[ Symmetrical Center Squeeze | Left-Edge Pinch | Exponential Ramp ]`.
* **DIP 3 (Anti-Aliasing):** `[ PolyBLEP Anti-Aliased | Raw Digital Chiptune Aliasing ]`.
* **DIP 4 (Envelope Target):** `[ Fold Depth | Multiply Ratio | Amp VCA Only ]`.
* **TRIM 1 (DC Offset Nulling):** Calibrates zero DC output offset.
* **TRIM 2 (Anti-Fold Damping):** High-frequency damping filter to prevent harsh aliasing.
* **TRIM 3 (Velocity -> Fold Sens):** Dynamic velocity control over folding intensity.
* **TRIM 4 (Post-Fold Headroom):** Output stage limiter ceiling.
