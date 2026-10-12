#!/usr/bin/env python3
"""
N'kai Pure-Python Zero-Dependency QR Code Generator (tools/qr_code.py)
Part of N'kai v0.3.0 Mobile Companion & Touch Ergonomics (Topic 6.13 / Section 14).

Standard-library only (ISO/IEC 18004 compliant subset for Versions 1-10, Levels L & M).
Produces scan-friendly Unicode block characters for terminal pairing and crisp SVG for web pairing.
Universal UTF-8 compliance guaranteed.
"""

import sys
from typing import List, Optional, Tuple

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

# ---------------------------------------------------------------------------
# Galois Field GF(256) Math
# ---------------------------------------------------------------------------
EXP = [0] * 512
LOG = [0] * 256
_val = 1
for _i in range(255):
    EXP[_i] = _val
    EXP[_i + 255] = _val
    LOG[_val] = _i
    _val <<= 1
    if _val >= 256:
        _val ^= 0x11D


def gf_mul(x: int, y: int) -> int:
    if x == 0 or y == 0:
        return 0
    return EXP[LOG[x] + LOG[y]]


def rs_generator_poly(ec_count: int) -> List[int]:
    g = [1]
    for i in range(ec_count):
        factor = [1, EXP[i]]
        new_g = [0] * (len(g) + 1)
        for j, c in enumerate(g):
            new_g[j] ^= gf_mul(c, factor[0])
            new_g[j + 1] ^= gf_mul(c, factor[1])
        g = new_g
    return g


def rs_encode(data: List[int], ec_count: int) -> List[int]:
    gen = rs_generator_poly(ec_count)
    msg = list(data) + [0] * ec_count
    for i in range(len(data)):
        lead = msg[i]
        if lead != 0:
            for j in range(len(gen)):
                msg[i + j] ^= gf_mul(gen[j], lead)
    return msg[len(data):]


# ---------------------------------------------------------------------------
# QR Version Specifications (Versions 1-10 for Level L and M)
# Format: (total_codewords, ec_per_block, g1_blocks, g1_data, g2_blocks, g2_data)
# ---------------------------------------------------------------------------
VERSION_SPECS_L = {
    1: (26, 7, 1, 19, 0, 0),
    2: (44, 10, 1, 34, 0, 0),
    3: (70, 15, 1, 55, 0, 0),
    4: (100, 20, 1, 80, 0, 0),
    5: (134, 26, 1, 108, 0, 0),
    6: (172, 18, 2, 68, 0, 0),
    7: (196, 20, 2, 78, 0, 0),
    8: (242, 24, 2, 97, 0, 0),
    9: (292, 30, 2, 116, 0, 0),
    10: (346, 18, 2, 68, 2, 69),
}

VERSION_SPECS_M = {
    1: (26, 10, 1, 16, 0, 0),
    2: (44, 16, 1, 28, 0, 0),
    3: (70, 26, 1, 44, 0, 0),
    4: (100, 18, 2, 32, 0, 0),
    5: (134, 24, 2, 43, 0, 0),
    6: (172, 16, 4, 27, 0, 0),
    7: (196, 18, 4, 31, 0, 0),
    8: (242, 22, 2, 38, 2, 39),
    9: (292, 22, 3, 36, 2, 37),
    10: (346, 26, 4, 43, 1, 44),
}

ALIGNMENT_LOCATIONS = {
    1: [],
    2: [6, 18],
    3: [6, 22],
    4: [6, 26],
    5: [6, 30],
    6: [6, 34],
    7: [6, 22, 38],
    8: [6, 24, 42],
    9: [6, 26, 46],
    10: [6, 28, 50],
}


class QRCode:
    def __init__(self, data: str, level: str = "L"):
        self.data_bytes = data.encode("utf-8")
        self.level = level.upper()
        if self.level not in ("L", "M"):
            self.level = "L"

        self.specs = VERSION_SPECS_L if self.level == "L" else VERSION_SPECS_M
        self.version = self._find_smallest_version(len(self.data_bytes))
        self.size = 21 + 4 * (self.version - 1)
        self.matrix: List[List[bool]] = []
        self._generate()

    def _find_smallest_version(self, byte_len: int) -> int:
        for v in range(1, 11):
            total, ec, g1_b, g1_d, g2_b, g2_d = self.specs[v]
            total_data = (g1_b * g1_d) + (g2_b * g2_d)
            # Byte mode header: 4 bits mode + 8 bits length (for V1-9) or 16 bits (for V10)
            header_bits = 4 + (8 if v < 10 else 16)
            cap_bytes = (total_data * 8 - header_bits) // 8
            if byte_len <= cap_bytes:
                return v
        raise ValueError(f"Payload too large for versions 1-10 (length: {byte_len} bytes)")

    def _build_codewords(self) -> List[int]:
        v = self.version
        total, ec_count, g1_b, g1_d, g2_b, g2_d = self.specs[v]
        total_data = (g1_b * g1_d) + (g2_b * g2_d)

        # 1. Mode indicator: 0100 (Byte mode)
        bits = [0, 1, 0, 0]

        # 2. Character count indicator
        length_bits_count = 8 if v < 10 else 16
        length_val = len(self.data_bytes)
        for i in range(length_bits_count - 1, -1, -1):
            bits.append((length_val >> i) & 1)

        # 3. Data payload
        for b in self.data_bytes:
            for i in range(7, -1, -1):
                bits.append((b >> i) & 1)

        # 4. Terminator (up to 4 zeroes)
        max_bits = total_data * 8
        term_len = min(4, max_bits - len(bits))
        bits.extend([0] * term_len)

        # 5. Pad to byte boundary
        while len(bits) % 8 != 0:
            bits.append(0)

        # 6. Convert to bytes
        data_bytes = []
        for i in range(0, len(bits), 8):
            byte_val = 0
            for bit in bits[i : i + 8]:
                byte_val = (byte_val << 1) | bit
            data_bytes.append(byte_val)

        # 7. Fill pad bytes (0xEC, 0x11)
        pad_toggle = True
        while len(data_bytes) < total_data:
            data_bytes.append(0xEC if pad_toggle else 0x11)
            pad_toggle = not pad_toggle

        # 8. Divide into blocks & compute Reed-Solomon EC
        blocks_data = []
        blocks_ec = []
        ptr = 0

        for _ in range(g1_b):
            blk = data_bytes[ptr : ptr + g1_d]
            ptr += g1_d
            blocks_data.append(blk)
            blocks_ec.append(rs_encode(blk, ec_count))

        for _ in range(g2_b):
            blk = data_bytes[ptr : ptr + g2_d]
            ptr += g2_d
            blocks_data.append(blk)
            blocks_ec.append(rs_encode(blk, ec_count))

        # 9. Interleave data codewords
        interleaved = []
        max_data_len = max(len(b) for b in blocks_data)
        for i in range(max_data_len):
            for blk in blocks_data:
                if i < len(blk):
                    interleaved.append(blk[i])

        # 10. Interleave EC codewords
        for i in range(ec_count):
            for ec in blocks_ec:
                interleaved.append(ec[i])

        return interleaved

    def _generate(self):
        N = self.size
        matrix = [[False] * N for _ in range(N)]
        reserved = [[False] * N for _ in range(N)]

        def set_func(r, c, val):
            matrix[r][c] = val
            reserved[r][c] = True

        # 1. Finder patterns
        def place_finder(r_top, c_left):
            for r in range(7):
                for c in range(7):
                    is_border = (r in (0, 6) or c in (0, 6))
                    is_center = (2 <= r <= 4 and 2 <= c <= 4)
                    set_func(r_top + r, c_left + c, is_border or is_center)

        place_finder(0, 0)
        place_finder(0, N - 7)
        place_finder(N - 7, 0)

        # 2. Separators around finders
        for i in range(8):
            for j in range(8):
                # Top-left separator
                if (i == 7 or j == 7) and i < N and j < N:
                    set_func(i, j, False)
                # Top-right separator
                c = N - 8 + j
                if (i == 7 or j == 0) and i < N and 0 <= c < N:
                    set_func(i, c, False)
                # Bottom-left separator
                r = N - 8 + i
                if (i == 0 or j == 7) and 0 <= r < N and j < N:
                    set_func(r, j, False)

        # 3. Alignment patterns (V >= 2)
        locs = ALIGNMENT_LOCATIONS.get(self.version, [])
        for r_center in locs:
            for c_center in locs:
                # Skip if overlaps with finders
                if (r_center <= 8 and c_center <= 8) or \
                   (r_center <= 8 and c_center >= N - 9) or \
                   (r_center >= N - 9 and c_center <= 8):
                    continue
                for r in range(-2, 3):
                    for c in range(-2, 3):
                        is_outer = (abs(r) == 2 or abs(c) == 2)
                        is_center = (r == 0 and c == 0)
                        set_func(r_center + r, c_center + c, is_outer or is_center)

        # 4. Timing patterns
        for i in range(8, N - 8):
            if not reserved[6][i]:
                set_func(6, i, (i % 2 == 0))
            if not reserved[i][6]:
                set_func(i, 6, (i % 2 == 0))

        # 5. Dark module
        set_func(4 * self.version + 9, 8, True)

        # 6. Reserve format info areas
        for i in range(9):
            if not reserved[8][i]:
                reserved[8][i] = True
            if not reserved[i][8]:
                reserved[i][8] = True
        for i in range(N - 8, N):
            reserved[8][i] = True
        for i in range(N - 7, N):
            reserved[i][8] = True

        # 7. Place data bits
        codewords = self._build_codewords()
        data_bits = []
        for cw in codewords:
            for i in range(7, -1, -1):
                data_bits.append((cw >> i) & 1)

        bit_idx = 0
        upward = True
        c = N - 1

        while c > 0:
            if c == 6:  # Skip vertical timing pattern
                c -= 1
            rows = range(N - 1, -1, -1) if upward else range(N)
            for r in rows:
                for col in (c, c - 1):
                    if not reserved[r][col]:
                        val = data_bits[bit_idx] if bit_idx < len(data_bits) else 0
                        matrix[r][col] = bool(val)
                        bit_idx += 1
            upward = not upward
            c -= 2

        # 8. Select best mask
        best_mask = 0
        min_penalty = float("inf")
        best_matrix = matrix

        for mask_idx in range(8):
            masked = [row[:] for row in matrix]
            for r in range(N):
                for col in range(N):
                    if not reserved[r][col]:
                        if self._mask_condition(mask_idx, r, col):
                            masked[r][col] = not matrix[r][col]

            self._apply_format_info(masked, mask_idx)
            penalty = self._calculate_penalty(masked)
            if penalty < min_penalty:
                min_penalty = penalty
                best_mask = mask_idx
                best_matrix = masked

        self.matrix = best_matrix

    def _mask_condition(self, mask: int, r: int, c: int) -> bool:
        if mask == 0:
            return (r + c) % 2 == 0
        if mask == 1:
            return r % 2 == 0
        if mask == 2:
            return c % 3 == 0
        if mask == 3:
            return (r + c) % 3 == 0
        if mask == 4:
            return ((r // 2) + (c // 3)) % 2 == 0
        if mask == 5:
            return ((r * c) % 2) + ((r * c) % 3) == 0
        if mask == 6:
            return (((r * c) % 2) + ((r * c) % 3)) % 2 == 0
        if mask == 7:
            return (((r + c) % 2) + ((r * c) % 3)) % 2 == 0
        return False

    def _apply_format_info(self, mat: List[List[bool]], mask: int):
        N = len(mat)
        # Level indicator: L=01 (1), M=00 (0)
        level_indicator = 1 if self.level == "L" else 0
        format_data = (level_indicator << 3) | mask

        rem = format_data << 10
        for i in range(4, -1, -1):
            if (rem >> (i + 10)) & 1:
                rem ^= (0x537 << i)
        bch = (format_data << 10) | rem
        bits = bch ^ 0x5412

        # 15 format bits:
        # Top-left finders
        coords_top = [
            (8, 0), (8, 1), (8, 2), (8, 3), (8, 4), (8, 5),
            (8, 7), (8, 8), (7, 8), (5, 8), (4, 8), (3, 8),
            (2, 8), (1, 8), (0, 8),
        ]
        for i in range(15):
            val = bool((bits >> (14 - i)) & 1)
            r, c = coords_top[i]
            mat[r][c] = val

        # Split around other finders:
        # Bottom-left: bits 0..6 (LSB to MSB)
        for i in range(7):
            val = bool((bits >> i) & 1)
            mat[N - 1 - i][8] = val

        # Top-right: bits 7..14
        for i in range(8):
            val = bool((bits >> (7 + i)) & 1)
            mat[8][N - 8 + i] = val

    def _calculate_penalty(self, mat: List[List[bool]]) -> int:
        N = len(mat)
        penalty = 0

        # Feature 1: 5+ consecutive modules
        for r in range(N):
            cnt = 1
            for c in range(1, N):
                if mat[r][c] == mat[r][c - 1]:
                    cnt += 1
                else:
                    if cnt >= 5:
                        penalty += 3 + (cnt - 5)
                    cnt = 1
            if cnt >= 5:
                penalty += 3 + (cnt - 5)

        for c in range(N):
            cnt = 1
            for r in range(1, N):
                if mat[r][c] == mat[r - 1][c]:
                    cnt += 1
                else:
                    if cnt >= 5:
                        penalty += 3 + (cnt - 5)
                    cnt = 1
            if cnt >= 5:
                penalty += 3 + (cnt - 5)

        # Feature 2: 2x2 blocks of same color
        for r in range(N - 1):
            for c in range(N - 1):
                val = mat[r][c]
                if mat[r + 1][c] == val and mat[r][c + 1] == val and mat[r + 1][c + 1] == val:
                    penalty += 3

        # Feature 3: 1:1:3:1:1 pattern
        pattern = [True, False, True, True, True, False, True]
        for r in range(N):
            for c in range(N - 6):
                if [mat[r][c + i] for i in range(7)] == pattern:
                    if (c >= 4 and [mat[r][c - 4 + i] for i in range(4)] == [False] * 4) or \
                       (c + 11 <= N and [mat[r][c + 7 + i] for i in range(4)] == [False] * 4):
                        penalty += 40

        for c in range(N):
            for r in range(N - 6):
                if [mat[r + i][c] for i in range(7)] == pattern:
                    if (r >= 4 and [mat[r - 4 + i][c] for i in range(4)] == [False] * 4) or \
                       (r + 11 <= N and [mat[r + 7 + i][c] for i in range(4)] == [False] * 4):
                        penalty += 40

        # Feature 4: Dark module ratio
        total_modules = N * N
        dark_modules = sum(row.count(True) for row in mat)
        percentage = (dark_modules * 100) // total_modules
        k = abs(percentage - 50) // 5
        penalty += k * 10

        return penalty

    def render_terminal(self, border: int = 2) -> str:
        """
        Renders QR code to terminal using Unicode half-block characters (▀, ▄, █, ' ').
        Each character represents 2 vertical modules, producing a compact, square code.
        """
        N = len(self.matrix)
        total_dim = N + 2 * border
        padded = [[False] * total_dim for _ in range(total_dim)]

        for r in range(N):
            for c in range(N):
                padded[r + border][c + border] = self.matrix[r][c]

        lines = []
        for r in range(0, total_dim, 2):
            row_chars = []
            for c in range(total_dim):
                top_dark = padded[r][c]
                bottom_dark = padded[r + 1][c] if r + 1 < total_dim else False

                # Scan-friendly dark-terminal mapping (background is dark, foreground is bright white):
                # Light quiet zone around code.
                # In standard developer terminal:
                # Top dark + Bottom dark => ' '
                # Top dark + Bottom light => '▄'
                # Top light + Bottom dark => '▀'
                # Top light + Bottom light => '█'
                if top_dark and bottom_dark:
                    row_chars.append(" ")
                elif top_dark and not bottom_dark:
                    row_chars.append("▄")
                elif not top_dark and bottom_dark:
                    row_chars.append("▀")
                else:
                    row_chars.append("█")
            lines.append("  " + "".join(row_chars))

        return "\n".join(lines)

    def render_svg(self, border: int = 2, size: int = 256) -> str:
        """
        Renders crisp SVG vector QR code.
        """
        N = len(self.matrix)
        total_dim = N + 2 * border
        scale = max(1, size // total_dim)
        viewbox_dim = total_dim

        rects = []
        for r in range(N):
            for c in range(N):
                if self.matrix[r][c]:
                    rects.append(f'<rect x="{c + border}" y="{r + border}" width="1" height="1" fill="#000000"/>')

        rect_str = "\n    ".join(rects)
        return f"""<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {viewbox_dim} {viewbox_dim}" width="{size}" height="{size}" shape-rendering="crispEdges">
  <rect width="100%" height="100%" fill="#ffffff" rx="4"/>
  <g>
    {rect_str}
  </g>
</svg>"""


def generate_qr_terminal(text: str, border: int = 2) -> str:
    """Convenience helper returning terminal ASCII / Unicode QR string."""
    qr = QRCode(text, level="L")
    return qr.render_terminal(border=border)


def generate_qr_svg(text: str, border: int = 2, size: int = 256) -> str:
    """Convenience helper returning SVG XML QR string."""
    qr = QRCode(text, level="L")
    return qr.render_svg(border=border, size=size)


def main():
    if len(sys.argv) < 2:
        print("Usage: python tools/qr_code.py <url_or_text> [--svg]")
        sys.exit(1)

    text = sys.argv[1]
    qr = QRCode(text, level="L")

    if "--svg" in sys.argv:
        print(qr.render_svg())
    else:
        print(f"\n  📱 QR Code for: {text}\n")
        print(qr.render_terminal())
        print("\n")


if __name__ == "__main__":
    main()
