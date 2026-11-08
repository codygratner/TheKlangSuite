# SPEC-TKDSP-002: Serial SLIP Protocol & Display Framing Specification

- **Status:** Ratified Clean-Room Specification (`TARGET-02`)
- **Domain:** Embedded Communications & Hardware Interoperability
- **Authors:** Klang Industries Clean-Room Architecture Team
- **Date:** 2026-10-27

---

## 1. Provenance & Legal Clearance
1. **Statutory Basis**:
   - Formulated strictly for display monitoring and controller input interoperability under the Digital Millennium Copyright Act (17 U.S.C. § 1201(f)).
   - Pure framing algorithms, mathematical state machines, and packet protocols are unprotectable under 17 U.S.C. § 102(b).
2. **Prior Art Citations**:
   - Framing layer: Standard Serial Line Internet Protocol (RFC 1055, J. Romkey, June 1988).
   - Display & command protocol: Open-source reference implementation in `m8c` (authored by laamaa, released under the permissive MIT License).
3. **Clean-Room Attestation**:
   - This specification describes input/output packet structures, byte encodings, and state transition equations. Zero proprietary firmware disassembly, decompiler output, or internal chip registers are utilized.

---

## 2. Framing Layer: RFC 1055 SLIP Framing

Communication occurs over full-duplex USB CDC virtual serial at 115200 baud (or hardware USB line speed) with 8 data bits, 1 stop bit, no parity.

### 2.1 Protocol Constants
- `END` = `0xC0` (Decimal 192): Frame boundary delimiter.
- `ESC` = `0xDB` (Decimal 219): Framing escape character.
- `ESC_END` = `0xDC` (Decimal 220): Encoded byte for literal `0xC0` within payload.
- `ESC_ESC` = `0xDD` (Decimal 221): Encoded byte for literal `0xDB` within payload.

### 2.2 Decoder State Machine
The receiver implements a deterministic 2-state finite automaton $S \in \{\text{NORMAL}, \text{ESCAPED}\}$:

$$\begin{aligned}
S_0 &= \text{NORMAL} \\
(S_{n+1}, B_{out}) &= 
\begin{cases}
(\text{NORMAL}, \text{None}) & \text{if } B_{in} = \text{END} \text{ (dispatches frame)} \\
(\text{ESCAPED}, \text{None}) & \text{if } S_n = \text{NORMAL} \land B_{in} = \text{ESC} \\
(\text{NORMAL}, \text{END}) & \text{if } S_n = \text{ESCAPED} \land B_{in} = \text{ESC\_END} \\
(\text{NORMAL}, \text{ESC}) & \text{if } S_n = \text{ESCAPED} \land B_{in} = \text{ESC\_ESC} \\
(\text{NORMAL}, B_{in}) & \text{otherwise}
\end{cases}
\end{aligned}$$

---

## 3. Packet Architecture & Command Set

A decoded SLIP frame consists of a 1-byte command identifier followed by typed payload fields.

### 3.1 Device-to-Host Display Packets

#### Command 0xFE: Draw Rectangle (`DRAW_RECT`)
Renders a solid color rectangle on the 320x240 display grid.
- **Payload Length**: 12 bytes
- **Layout**:
  - `[0..1]`: $X$ coordinate (16-bit unsigned, Little-Endian)
  - `[2..3]`: $Y$ coordinate (16-bit unsigned, Little-Endian)
  - `[4..5]`: Width $W$ (16-bit unsigned, Little-Endian)
  - `[6..7]`: Height $H$ (16-bit unsigned, Little-Endian)
  - `[8]`: Red component ($R \in [0, 255]$)
  - `[9]`: Green component ($G \in [0, 255]$)
  - `[10]`: Blue component ($B \in [0, 255]$)

#### Command 0xFF: Draw Character (`DRAW_CHAR`)
Draws a single fixed-width font glyph (typically 8x8 or 8x10 pixels).
- **Payload Length**: 12 bytes
- **Layout**:
  - `[0]`: Character code / ASCII value ($C \in [0, 255]$)
  - `[1..2]`: $X$ coordinate (16-bit unsigned, Little-Endian)
  - `[3..4]`: $Y$ coordinate (16-bit unsigned, Little-Endian)
  - `[5..7]`: Foreground Color ($R, G, B$)
  - `[8..10]`: Background Color ($R, G, B$)

#### Command 0xFC: Draw Oscilloscope Waveform (`DRAW_WAVEFORM`)
Renders the real-time audio visualization strip.
- **Payload Length**: 323 bytes
- **Layout**:
  - `[0..2]`: Stroke Color ($R, G, B$)
  - `[3..322]`: 320 vertical sample offsets $Y_i \in [0, 239]$ across screen columns $i \in [0, 319]$.

---

## 4. Host-to-Device Input & Handshake Packets

### 4.1 Handshake Initialization
- **Connect (`0x43` / ASCII 'C')**: Host sends single byte `'C'` followed by `0xC0`. Device acknowledges and begins streaming screen refresh frames.
- **Disconnect (`0x44` / ASCII 'D')**: Host sends single byte `'D'` followed by `0xC0`. Device halts display streaming and enters low-overhead background state.

### 4.2 Keypress Input Packet
Host transmits controller state updates whenever physical button states change.
- **Payload Length**: 2 bytes
- **Layout**:
  - Byte 0: `0x01` (Input Command)
  - Byte 1: 8-bit Button Bitmask:
    - Bit 0: `UP`
    - Bit 1: `DOWN`
    - Bit 2: `LEFT`
    - Bit 3: `RIGHT`
    - Bit 4: `A` (Play / Confirm)
    - Bit 5: `B` (Back / Cut)
    - Bit 6: `SELECT` (Option)
    - Bit 7: `START` (Edit)

---

## 5. TCP Forwarding Interface
For desktop integration, the serial stream is encapsulated in TCP localhost sockets (`127.0.0.1:17891`).
- Sockets must enable `TCP_NODELAY` to prevent Nagle packet coalescing delays.
- Incoming TCP payload bytes are written directly to USB serial.
- Outgoing serial SLIP frames are multiplexed to all connected TCP clients.
