# Technical DSP Specification: `Flint & Dust` (Transient Strike & Noise Engine)

**Engine ID:** `CARD-07A` / `OSC-10`  
**Sovereign Name:** `Flint & Dust`  
**Corporate Boring Name:** `Transient Strike & Noise Engine`  
**Category:** Percussion & Noise Synthesizer  
**APVTS Prefix:** `flint_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section D](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Roland TR-909 kick pulse circuit, TR-808 transistor noise cluster, vinyl surface groove dust, velvet noise.
* **Upstream Offline Reference:** [03_mutable_instruments_dsp_encyclopedia.md](file:///C:/Dev/Research/library/03_mutable_instruments_dsp_encyclopedia.md) & [ROSETTA_STONE_MASTER_IP_REFERENCE.md](file:///C:/Dev/Research/library/manuals/ROSETTA_STONE_MASTER_IP_REFERENCE.md).
* **C++ Implementation Header:** [`FlintAndDust.h`](file:///c:/Dev/TheKlangDSP/include/dsp/oscillators/FlintAndDust.h).

---

## 2. Mathematical Theory & DSP Architecture

`Flint & Dust` is a dual-path acoustic spark and textured bed engine:

### Path 1: "Flint" (Acoustic Transient Spark)
Generates high-velocity initial impact clicks and acoustic transient pops:
$$p(t) = A \cdot \left(\frac{t}{\tau}\right) e^{-t/\tau} \cdot \cos(2\pi f_{strike} t)$$
where $\tau$ is the decay time ($0.5\text{ ms} \dots 40\text{ ms}$) and $f_{strike}$ is the transient pitch ($200\text{ Hz} \dots 8\text{ kHz}$).
* **6 Transient Models:** `909 Pulse`, `808 Pop`, `Wood Rim`, `Metal Clack`, `Plastic Snap`, `Vinyl Drop`.

### Path 2: "Dust" (Textured Noise Bed)
Generates 6 continuous noise topologies routed through an integrated Cytomic ZDF state-variable filter and spectral tilt network:
1. **White Noise:** Uniform Gaussian distribution.
2. **Pink Noise ($1/f$):** 3-pole Paul Kellet filter network ($3\text{ dB/oct}$ rolloff).
3. **Red / Brownian Noise ($1/f^2$):** Leaky integration ($6\text{ dB/oct}$ rolloff).
4. **808 Metallic Cluster:** 6 Schmitt-trigger square waves running at inharmonic Paiste bronze ratios.
5. **Velvet Noise:** Sparse pseudo-random Poisson impulse train ($Q$ impulses/sec).
6. **Vinyl Crackle:** Random surface dust pops with variable surface scratch density.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Page 1A (Flint - Transient Spark):**
  * `Knob 1 (TRANS PITCH):` Transient tuning ($100\text{ Hz} \dots 8\text{ kHz}$, Profile D). *Push: Reset Default*.
  * `Knob 2 (TRANS DECAY):` Transient duration ($0.5\text{ ms} \dots 100\text{ ms}$, Profile A). *Push: Ultra-Fast Snap (2ms)*.
  * `Knob 3 (TRANS LEVEL):` Transient spark volume ($0\% \dots 100\%$, Profile B). *Push: Mute Transient*.
  * `Knob 4 (TRANS MODEL):` Discrete selector (909, 808, Rim, Clack, Snap, Drop). *Push: Cycle Model*.
* **Page 1B (Dust - Noise Bed):**
  * `Knob 5 (NOISE COLOR):` 6 Topologies (White, Pink, Red, 808, Velvet, Vinyl). *Push: Next Color*.
  * `Knob 6 (NOISE CUTOFF):` Dedicated noise filter cutoff ($20\text{ Hz} \dots 20\text{ kHz}$, Profile D). *Push: 1kHz Detent*.
  * `Knob 7 (NOISE DECAY):` Dedicated noise burst decay ($5\text{ ms} \dots 20\text{ s}$, Profile A). *Push: Gate Mode*.
  * `Knob 8 (NOISE LEVEL):` Noise bed volume ($0\% \dots 100\%$, Profile B). *Push: Mute Noise*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Filter Pass Mode):** `[ 2-Pole Bandpass | 2-Pole Highpass | 2-Pole Lowpass | Notch ]`.
* **DIP 2 (Envelope Mode):** `[ 1-Shot Percussive Strike | Gated Note-On | Continuous Ambient Bed ]`.
* **DIP 3 (Stereo Width):** `[ True Dual-Mono Uncorrelated | Phase Inverted Wide | Pure Mono ]`.
* **DIP 4 (Punch Ducking):** `[ Off | Transient Ducks Noise Bed (Sidechain Auto-Pinch) ]`.
* **TRIM 1 (Filter Resonance Q):** Resonance peak sharpness on the noise filter.
* **TRIM 2 (Spectral Tilt Balance):** Continuous $\pm 6\text{ dB/oct}$ bass-to-treble tilt.
* **TRIM 3 (Vinyl Dust Density):** Average clicks per second in Vinyl mode.
* **TRIM 4 (Transient Saturation):** Pre-clipping drive on the transient spark.
