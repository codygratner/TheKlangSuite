# Technical DSP Specification: `K-Chip` (5-Console Retro Chiptune Lab)

**Engine ID:** `OSC-07`  
**Sovereign Name:** `K-Chip`  
**Corporate Boring Name:** `Programmable Sound Generator (PSG)`  
**Category:** Cycle-Accurate Vintage Chiptune Synthesizer Core  
**APVTS Prefix:** `kchip_`  
**Rosetta Stone Cross-Reference:** [ROSETTA_STONE.md Section B](file:///c:/Dev/TheKlangSuite/docs/ROSETTA_STONE.md#section-b)

---

## 1. Pedigree & Upstream Hardware Reference
* **Hardware Heritage:** Nintendo Game Boy (DMG-01 APU), NES / Famicom (Ricoh 2A03 + Konami VRC6), Commodore 64 (MOS 6581/8580 SID), PC Engine / TurboGrafx-16 (HuC6280).
* **Upstream Offline Reference:** [04_tracker_pantheon_architecture_comparison.md](file:///C:/Dev/Research/library/04_tracker_pantheon_architecture_comparison.md).
* **C++ Implementation Header:** [`KChipPSG.h`](file:///c:/Dev/TheKlangDSP/include/dsp/oscillators/KChipPSG.h).

---

## 2. Mathematical Theory & DSP Architecture

`K-Chip` provides cycle-accurate emulation of 5 legendary video game sound chips:

### 1. Game Boy (DMG-01 APU)
* Pulse 1 & 2 with hardware sweep and 4 duty cycles ($12.5\%, 25\%, 50\%, 75\%$).
* Custom 4-bit 32-sample wave channel (Wave RAM).
* 7-bit / 15-bit Galois Linear-Feedback Shift Register (LFSR) noise.

### 2. NES (Ricoh 2A03 + Konami VRC6)
* 4-bit stepped staircase triangle oscillator with authentic low-frequency harmonics and zero anti-aliasing.
* VRC6 Sawtooth accumulator ($0..7$ steps per cycle).

### 3. Commodore 64 (MOS 6581 / 8580 SID)
* Continuous phase accumulator with combined waveforms (Tri+Saw, Pulse+Saw).
* Analog state-variable filter with non-linear FET distortion and authentic voice bleed.

### 4. PC Engine (HuC6280)
* 6-channel 5-bit wavetable synthesis with 32-sample loops and hardware DDA streaming.

---

## 3. Universal 16-Control Layout

### Front Panel: 8 Performance Encoders
* **Knob 1 (`PITCH`):** Pitch with semitone snap ($20\text{ Hz} \dots 4000\text{ Hz}$, Profile D). *Push: Octave Transpose*.
* **Knob 2 (`CHIP MODE`):** Discrete Console Selector (`DMG-01`, `2A03 NES`, `VRC6`, `SID 6581`, `PCE HuC6280`). *Push: Cycle Console*.
* **Knob 3 (`WAVE / DUTY`):** Duty cycle ($12.5\% \dots 75\%$) or Wavetable Bank. Profile B. *Push: Snap 50% Square*.
* **Knob 4 (`DAC BITS`):** Vintage DAC quantize ($4\text{ bit}, 5\text{ bit}, 8\text{ bit}, 16\text{ bit}$, Profile H). *Push: 4-bit Lo-Fi Snap*.
* **Knob 5 (`ARPEGGIATOR SPEED`):** Tracker-style chip arpeggio ($0\text{ Hz} \dots 50\text{ Hz}$ or $1..16\text{ ticks}$, Profile C). *Push: Arp Off*.
* **Knob 6 (`VINTAGE FILTER CUTOFF`):** SID 6581 multi-mode filter cutoff ($30\text{ Hz} \dots 12\text{ kHz}$, Profile D). *Push: Bypass Filter*.
* **Knob 7 (`CRUSH / CLOCK JITTER`):** CPU clock decimation & crystal drift. Profile E. *Push: Pristine Clock*.
* **Knob 8 (`OUTPUT LEVEL`):** Master makeup trim ($\pm 18\text{ dB}$, Profile B). *Push: Unity 0dB*.

### Rear Deck ⚙️: 8 Sound Lab Controls
* **DIP 1 (SID Filter Revision):** `[ 6581 Dark & Gritty Non-Linear | 8580 Clean Linear | Filter Bypass ]`.
* **DIP 2 (Noise LFSR Mode):** `[ 7-Bit Metallic Chipped | 15-Bit White Noise | Periodic Tonality ]`.
* **DIP 3 (Anti-Aliasing):** `[ Raw Zero-Filter Aliased (Authentic) | PolyBLEP Bandlimited Studio ]`.
* **DIP 4 (NES Triangle Vol Leak):** `[ Authentic 4-Bit Pop Bleed | Clean Filtered Staircase ]`.
* **TRIM 1 (SID Distortion Drive):** Non-linear FET amplifier saturation.
* **TRIM 2 (Arp Semitone Offset 1):** Major/Minor third chord interval for auto-arpeggiator.
* **TRIM 3 (Arp Semitone Offset 2):** Fifth/Octave interval for auto-arpeggiator.
* **TRIM 4 (Voice Bleed Crosstalk):** Inter-channel audio bleed between sound channels.
