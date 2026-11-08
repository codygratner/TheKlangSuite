# SPEC-TKDSP-001: 32-Bit Linear Congruential Pseudorandom Number Generator

- **Status:** Ratified Clean-Room Specification
- **Domain:** DSP & Embedded Sequencer Core
- **Author:** Klang Industries Clean-Room Architecture Team
- **Date:** 2026-10-27

---

## 1. Provenance & Legal Clearance
1. **Statutory Basis**:
   - Formulated strictly for file format and tracker playback interoperability under the Digital Millennium Copyright Act (17 U.S.C. § 1201(f)).
   - Unprotectable mathematical algorithm under 17 U.S.C. § 102(b) ("In no case does copyright protection for an original work of authorship extend to any idea, procedure, process, system, method of operation, concept, principle, or discovery").
2. **Academic Prior Art**:
   - The recurrence formula and constant parameters were published by Donald E. Knuth in *The Art of Computer Programming, Volume 2: Seminumerical Algorithms*, Section 3.2.1 (Addison-Wesley, 1969).
3. **Open-Source Prior Art**:
   - Identical linear congruential generator coefficients are utilized in open-source synthesis firmware, notably Mutable Instruments *Braids* (authored by Olivier Gillet, released under the permissive MIT License).
4. **Clean-Room Attestation**:
   - This specification contains pure mathematical relations. No decompiled source code, assembly instructions, processor registers, or proprietary firmware addresses are utilized or described herein.

---

## 2. Mathematical Definition

### 2.1 State Recurrence Relation
The generator maintains a 32-bit unsigned state variable $X_n \in [0, 2^{32}-1]$. Successive pseudorandom states are produced by the linear congruence:

$$X_{n+1} = (a \cdot X_n + c) \pmod{m}$$

Where:
- Multiplier: $a = 1664525$ (`0x0019660D`)
- Increment: $c = 1013904223$ (`0x3C6EF35F`)
- Modulus: $m = 2^{32}$ (computed naturally via 32-bit unsigned modular integer overflow)

### 2.2 Output Extraction Formulations

#### High-Entropy 8-Bit Tracker Byte
To maximize spectral randomness and avoid lower-bit correlation artifacts characteristic of linear congruential sequences, the 8-bit output byte is extracted from the upper half of the 32-bit state:

$$Y_n = \left( X_n \gg 16 \right) \ \& \ 255$$

#### Normalized Unit Floating-Point Value
For continuous modulation, random pan, and detune calculations:

$$U_n = \frac{X_n}{4294967296.0} \quad (0.0 \le U_n < 1.0)$$

#### Bounded Integer Range $[0, N-1]$
For selecting a random step, instrument index, or table row where $N \le 65536$:

$$R_n = \left( \left( X_n \gg 16 \right) \cdot N \right) \gg 16$$

---

## 3. Seeding & Sequencer Integration

### 3.1 Initial Conditions
- Default Seed: $X_0 = 1$ (or any non-zero initial seed vector).
- Reset Semantics: When an explicit seed command (`SED val`) is executed by a tracker lane or voice engine, the state variable is directly assigned:
  $$X_0 = \text{val}$$
- Successive evaluation occurs once per randomized event or sequencer tick.

### 3.2 Test Vectors (Golden Deterministic Sequence)
Given initial seed $X_0 = 0$:
1. $X_1 = (1664525 \cdot 0 + 1013904223) \pmod{2^{32}} = 1013904223$ (`0x3C6EF35F`) $\to Y_1 = 0x6E$
2. $X_2 = (1664525 \cdot 1013904223 + 1013904223) \pmod{2^{32}} = 1196435770$ (`0x474E293A`) $\to Y_2 = 0x4E$
3. $X_3 = (1664525 \cdot 1196435770 + 1013904223) \pmod{2^{32}} = 3450510101$ (`0xCDAA5315`) $\to Y_3 = 0xAA$

---

## 4. Implementation Guidance
- The implementation MUST use standard `uint32_t` arithmetic.
- Store state variables per track or per voice to prevent cross-track deterministic interference.
- C++ implementations must mark the step function `noexcept` and ensure zero dynamic allocations on real-time audio threads.
