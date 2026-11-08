# Technical DSP Specification: `Argent Anvil` (DOOM Upward Compressor & Brickwall Limiter)

**Engine ID:** `CARD-09` / `FX-05`  
**Sovereign Name:** `Argent Anvil`  
**Corporate Boring Name:** `Multiband Upward/Downward Dynamics & Limiter`  
**Category:** Master Dynamics Processor  
**APVTS Prefix:** `anvil_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section D](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-d)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Mick Gordon DOOM (2016) / DOOM Eternal sound design processing chain, Xfer OTT, Nimble Tools Argent.
* **Upstream Offline Reference:** [ROSETTA_STONE_MASTER_IP_REFERENCE.md](file:///C:/Dev/Research/library/manuals/ROSETTA_STONE_MASTER_IP_REFERENCE.md).
* **C++ Implementation Header:** [`ArgentAnvil.h`](file:///c:/Dev/TheKlangDSP/include/dsp/effects/ArgentAnvil.h).

---

## 2. Mathematical Theory & DSP Architecture

`Argent Anvil` is an extreme dynamics workstation designed to extract buried acoustic room details and aggressive punch through simultaneous 3-band upward expansion and downward peak compression.

### 3-Band Linkwitz-Riley Crossover Network
The audio stream is split into 3 phase-aligned frequency bands (Low, Mid, High) using cascaded 4th-order (24 dB/oct) Linkwitz-Riley filters ($H_{LR4} = H_{Butter2}^2$):
$$x[n] = x_{low}[n] + x_{mid}[n] + x_{high}[n]$$
Summing the three bands yields an all-pass phase response with perfectly flat magnitude ($\pm 0.0\text{ dB}$).

### Simultaneous Dual-Direction Dynamics
In each band, two independent gain curves operate concurrently:
1. **Downward Compression:** Clamps runaway peaks above threshold $T_{down}$ with ratio $R_{down} \approx 4:1 \dots 20:1$.
2. **Upward Compression:** Lifts low-level room sound, reverb tails, and breath noise below threshold $T_{up}$ by up to $+100\text{ dB}$:
$$G_{up}(x) = \min\left(G_{max}, \left(\frac{T_{up}}{|x|}\right)^{1 - 1/R_{up}}\right) \quad \text{for } |x| < T_{up}$$

### Asymmetric Saturation & True-Peak Lookahead Limiter
Before re-summing, the bands pass through asymmetric germanium diode soft-clippers. The summed output feeds a 1ms lookahead brickwall limiter with Inter-Sample Peak (ISP) detection to ensure 100% broadcast-safe delivery.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`ANVIL DRIVE`):** Master upward compression drive ($0\% \dots 100\%$, Profile E). *Push: Slam 100% DOOM*.
* **Knob 2 (`DOWNWARD COMP`):** Heavy peak downward squash ($0\text{ dB} \dots -36\text{ dB}$, Profile B). *Push: Bypass Downward*.
* **Knob 3 (`CROSSOVER LOW`):** Low/Mid crossover frequency ($40\text{ Hz} \dots 600\text{ Hz}$, Profile D). *Push: Snap 120Hz*.
* **Knob 4 (`CROSSOVER HIGH`):** Mid/High crossover frequency ($1.0\text{ kHz} \dots 12.0\text{ kHz}$, Profile D). *Push: Snap 3.5kHz*.
* **Knob 5 (`DOOM GATE`):** Chatter-free noise gate for upward floor silence ($-90\text{ dB} \dots -24\text{ dB}$, Profile B). *Push: Gate Bypass*.
* **Knob 6 (`SAT / CRUSH`):** Asymmetric diode clipping & crossover crunch ($0\% \dots 100\%$, Profile E). *Push: Crush Type*.
* **Knob 7 (`TRUE-PEAK CEILING`):** Brickwall limiter ceiling ($-12\text{ dBFS} \dots 0\text{ dBFS}$, Profile B). *Push: Snap -0.1 dBFS*.
* **Knob 8 (`MIX`):** Universal Tri-Zone Parallel Mix ($0\% \dots 100\%$, Profile M). *Push: Phase Invert 180°*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (Multiband Mode):** `[ 3-Band Full Split | 2-Band (Low+Mid Linked) | Single-Band Wideband Slam ]`.
* **DIP 2 (Upward Gain Curve):** `[ Linear +100dB DOOM | S-Curve Tamed Ambient | Hyperbolic Chaos ]`.
* **DIP 3 (Lookahead Window):** `[ 0ms Zero-Latency (Live/Tracker) | 1ms True-Peak Lookahead | 5ms Mastering ISP ]`.
* **DIP 4 (Crossover Phase):** `[ Minimum Phase Linkwitz-Riley | Linear Phase FIR ]`.
* **TRIM 1 (Upward Knee Softness):** Transition curvature between floor expansion and direct signal ($0\text{ dB} \dots 24\text{ dB}$).
* **TRIM 2 (Band 1 Low-End Punch Bias):** Low-frequency weight before upward compression ($-12\text{ dB} \dots +12\text{ dB}$).
* **TRIM 3 (Band 3 Air / Transient Recover):** Preserves extreme high-frequency cymbal/snare transients ($0\% \dots 100\%$).
* **TRIM 4 (Limiter Release Time):** Brickwall lookahead limiter recovery ($5\text{ ms} \dots 250\text{ ms}$).
