# Third-Party Open-Source & Algorithmic Attribution Ledger

*The Klang Suite* stands upon the shoulders of brilliant audio engineers, mathematicians, and open-source pioneers whose published research, open-source code (FOSS/POSS), and hardware designs have advanced the craft of electronic music software.

This document serves as our official licensing ledger and attribution directory, acknowledging upstream inspirations, open-source codebases, mathematical publications, and original authors.

---

## 0. Primary Project License: GNU General Public License v3.0 (GPLv3) — "FOSS Forever"

*The Klang Suite* is published under the **GNU General Public License Version 3 (GPLv3)** (see [LICENSE](../LICENSE)).

### The Copyleft Commitment:
- **User Freedom First**: We believe that music technology and sound design tools should remain accessible, transparent, and user-empowering forever. Under GPLv3, anyone is free to inspect, study, modify, fork, and redistribute this software.
- **Copyleft Reciprocity**: Any derivative works, ports (including embedded hardware ports for the TBD-16 or Zynthian), or modified distributions that incorporate code from *The Klang Suite* must also be released under the GPLv3 with full source code made available to the public.
- **Permissive Ingestion**: We happily adapt and vectorize algorithms originating from permissively licensed open-source projects (MIT, BSD-3-Clause, Apache 2.0, CC0/Public Domain). Under the terms of those licenses, their code can be incorporated into a GPLv3 work provided their original copyright notices, license conditions, and disclaimers are preserved.

---

## 1. Algorithmic Inspirations & Open-Source DSP

### Émilie Gillet / Mutable Instruments
- **Contributions & Inspiration**: 
  - Granular texture synthesis and real-time audio buffer freezing (inspired by the open-source DSP in *Clouds* and *Beads*).
  - Modal resonator networks and physical modeling of vibrating structures (inspired by *Rings*).
  - Algorithmic macro-oscillator models and band-limited waveform generation (inspired by *Braids* and *Plaits*).
- **Original Licenses**: MIT License and Creative Commons Attribution-ShareAlike 3.0 Unported (CC BY-SA 3.0).
- **Trademark & Attribution Notice**: 
  - *"Mutable Instruments"*, *"Clouds"*, *"Rings"*, *"Braids"*, and *"Plaits"* are trademarks of Émilie Gillet.
  - In strict compliance with trademark protection and respect for Émilie's guidelines, *The Klang Suite* strictly avoids using any Mutable Instruments trademark names in user-facing UI labels, FX pickers, parameter names, preset titles, or documentation.
  - All granular and modal algorithms are implemented under independent, original architectures with proprietary thematic names (e.g., **THE MIST** for granular particle synthesis & buffer freeze—a nod to Stephen King's eerie particulate atmosphere, and **MODAL RESONATOR** for physical impulse modeling).
  - We express immense gratitude to Émilie Gillet for her monumental contributions to open-source modular synthesis and modern DSP culture.
- **Upstream Repositories & Resources**:
  - GitHub: [https://github.com/pichenettes/eurorack](https://github.com/pichenettes/eurorack)
  - Official Archive: [https://mutable-instruments.net/](https://mutable-instruments.net/)

---

### Jatin Chowdhury / ChowDSP
- **Contributions & Inspiration**: 
  - Wave Digital Filter (WDF) modeling techniques for non-linear analog circuits and diode clippers.
  - High-performance SIMD polynomial math approximations and fast hyperbolic tangent approximations (`FastMath`).
- **Original License**: BSD 3-Clause License / GNU General Public License v3.0 (GPLv3).
- **Upstream Repositories & Resources**:
  - GitHub: [https://github.com/jatinchowdhury18](https://github.com/jatinchowdhury18)
  - Research & Papers: [https://chowdsp.com/](https://chowdsp.com/)

---

### Vadim Zavalishin (Native Instruments) & Will Pirkle
- **Contributions & Inspiration**: 
  - Theoretical foundations of Topology-Preserving Transform (TPT) and Zero-Delay Feedback (ZDF) state-variable and ladder filter architectures.
  - Virtual Analog (VA) filter non-linear saturation integration.
- **Reference Publications**:
  - Vadim Zavalishin: *"The Art of VA Filter Design"* (Native Instruments).
  - Will Pirkle: *"Designing Software Synthesizer Plugins in C++"* and *"Designing Audio Effect Plugins in C++"*.
  - Resources: [https://www.willpirkle.com/](https://www.willpirkle.com/)

---

### Sean Costello / Valhalla DSP
- **Contributions & Inspiration**: 
  - Feedback Delay Networks (FDN), prime-number delay distribution, householder feedback matrices, and diffuse tank reverberation topologies.
- **Reference Publications & Articles**:
  - Valhalla DSP Research Blog: [https://valhalladsp.com/blog/](https://valhalladsp.com/blog/)

---

### Nigel Redmon (EarLevel Engineering) & Paul Kellett
- **Contributions & Inspiration**: 
  - Polyphase half-band decimation and interpolation filter structures for low-CPU oversampling.
  - Biquad direct-form filter coefficient calculation routines.
- **Reference Resources**:
  - EarLevel Engineering: [https://www.earlevel.com/](https://www.earlevel.com/)
  - MusicDSP Source Archive: [https://www.musicdsp.org/](https://www.musicdsp.org/)

---

### Tom Whitwell / Music Thing Modular
- **Contributions & Inspiration**: 
  - The *Turing Machine* 16-bit Linear Feedback Shift Register (LFSR) generative pseudo-random sequence loop topology with controlled write/mutation probability.
- **Original License**: Open Source Hardware / MIT License.
- **Upstream Repositories & Resources**:
  - GitHub: [https://github.com/TomWhitwell/TuringMachine](https://github.com/TomWhitwell/TuringMachine)

---

### Dave Smith Instruments / Sequential
- **Contributions & Inspiration**: 
  - The gated, key-triggered step sequencer architecture from the Dave Smith Instruments Pro 2 (advancing independent polymetric modulation and gate lanes on incoming MIDI note/gate triggers rather than continuous DAW transport lock).

---

### Audio Damage (Chris Randall & Adam Schabtach)
- **Contributions & Inspiration**: 
  - The *Axon* neural network percussion sequencing topology (leaky integrate-and-fire artificial neurons with mutual excitatory/inhibitory synaptic connections generating biologically evolving polyrhythms).
- **Resources**: [https://www.audiodamage.com/](https://www.audiodamage.com/)

---

### dadamachines & CTAG
- **Contributions & Inspiration**: 
  - Open-source hardware groovebox architecture, dual ESP32-P4 / RP2350B multi-core embedded topology, and CTAG audio driver framework for the **TBD-16**.
- **Original License**: Open Source Hardware / Apache License 2.0.
- **Upstream Repositories & Resources**:
  - Documentation: [https://docs.dadamachines.com/](https://docs.dadamachines.com/)
  - GitHub: [https://github.com/dadamachines](https://github.com/dadamachines)

---

## 2. Core Frameworks & Libraries

### JUCE Framework
- **Copyright**: © Raw Material Software / PACE Anti-Piracy Inc.
- **Version**: 9.0.3
- **License**: GNU General Public License v3.0 (GPLv3) / Commercial License.
- **Website**: [https://juce.com/](https://juce.com/)

---

## 3. In-App User Credits & About Dialog Integration

As specified in **Milestone v0.6.0 Item 10**, *The Klang Suite* includes a dedicated, prominent **[ Credits & Open-Source Licenses ]** button inside the in-app **Settings & About** modal dialog. 

This dialog renders clickable hyperlinks directly to each author's website, repository, and original paper, ensuring complete transparency, ethical attribution, and ease of discovery for users and fellow developers.

---

## 4. Code Provenance & Inline Attribution Standard (SPDX)

To ensure lifelong GPLv3 integrity and crystal-clear attribution, all source files in *The Klang Suite* adhere to the following standards:

1. **SPDX License Identifiers**:
   All original `.h` and `.cpp` files in `source/` and `test/` begin with:
   ```cpp
   // SPDX-License-Identifier: GPL-3.0-or-later
   // Copyright (C) 2026 Cody Gratner & The Klang Suite Contributors
   ```

2. **Inline Adaptation Docblocks**:
   Whenever an algorithm, DSP structure, or mathematical function is adapted from an external open-source project or academic paper, it MUST be preceded by an explicit provenance docblock:
   ```cpp
   /**
    * @brief [Algorithm Name / Functionality Description]
    * 
    * Adapted from: [Original Project / Author Name]
    * Source URL:    [Upstream Repository or Paper Link]
    * Original License: [MIT / BSD-3-Clause / Apache 2.0 / Public Domain]
    * 
    * Modifications for The Klang Suite:
    * - [e.g., Vectorized using TbdAudio::FastMath polynomials]
    * - [e.g., Converted to Zero-Delay Feedback TPT structure]
    * - [e.g., Parameterized to fit 4-control Mega Module interface]
    */
   ```

3. **GPLv3 License Compatibility Rule**:
   No code licensed under copyleft-incompatible terms (such as CC-BY-NC non-commercial restrictions, original 4-clause BSD with advertising clauses, or proprietary NDA SDKs) may ever be merged into the repository.

