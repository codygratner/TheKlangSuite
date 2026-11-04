#!/usr/bin/env python3
"""
tools/bake_digistrike_rom.py — Fleet VitalTable ROM Ingestion & Binary Packer
=============================================================================
Part of The Klang Suite SOCEP Engine. Copyright (c) 2026 Klang Industries.
Extracts, resamples, and packs authentic hardware .vitaltable wave frames
across all 8 categories x 20 lineages (160 slots + 1 secret core = 161 slots)
into a compact 10.5 MB binary asset: assets/digistrike_fs2_rom.bin.
"""

from __future__ import annotations

import glob
import math
import os
import struct
import sys
from pathlib import Path
import numpy as np

# Universal UTF-8 for Windows
if hasattr(sys.stdout, "reconfigure"):
    try:
        sys.stdout.reconfigure(encoding="utf-8")
    except Exception:
        pass
if hasattr(sys.stderr, "reconfigure"):
    try:
        sys.stderr.reconfigure(encoding="utf-8")
    except Exception:
        pass

NUM_CATEGORIES = 8
NUM_LINEAGES = 20
NUM_HERO_SLOTS = 160
SECRET_CORE_SLOT = 160
NUM_SLOTS = 161
NUM_MORPH_STEPS = 4
NUM_FRAMES = 4
TABLE_SIZE = 2048

LINEAGES = [
    ('CROWN', 'linn'),
    ('APEX', 'dmx'),
    ('EMPYR', 'emu'),
    ('FALCON', 'akai'),
    ('K-WRK', 'kawai'),
    ('COMPACT', 'boss'),
    ('TOKYO', 'roland'),
    ('GOTHN', 'elektron'),
    ('CIRCUITS', 'sequential'),
    ('PHASE', 'casio'),
    ('STUDIO', 'alesis'),
    ('KEIO', 'korg'),
    ('NIPPON', 'yamaha'),
    ('ADIOS', 'curios'),
    ('FAIRL', 'fairlight'),
    ('SYNCL', 'synclavier'),
    ('ENSON', 'ensoniq'),
    ('SIMM', 'simmons'),
    ('JOMOX', 'jomox'),
    ('CURIOS', 'curios')
]

CAT_KEYWORDS = [
    # 0: KICK
    ['bd', 'kick', 'kik', 'bass_drum', 'b_drum'],
    # 1: SNARE
    ['sd', 'snare', 'snr', 'sn', 'rim'],
    # 2: TOMS
    ['tom', 'tm', 'etom', 'revtom', 'drum'],
    # 3: CLAP
    ['clap', 'cp', 'snap', 'slap', 'claptrap'],
    # 4: HATS
    ['hat', 'hh', 'hhc', 'hho', 'oh', 'ch'],
    # 5: CYMBALS
    ['cym', 'crs', 'crash', 'rid', 'ride', 'china', 'splash'],
    # 6: PERCUSSION
    ['perc', 'cowb', 'bongo', 'conga', 'timb', 'agog', 'cabasa', 'claves', 'wood', 'shak', 'tamb', 'laser', 'zap', 'metal', 'iron', 'bell', 'whistle'],
    # 7: STABS & BASS
    ['hit', 'orch', 'stab', 'bass', 'slap', 'organ', 'chord', 'techno', 'saw', 'growl', 'fm', 'syn']
]


def read_vitaltable(filepath: str) -> np.ndarray | None:
    """Reads a .vitaltable file returning an (N, 2048) float32 array in [-1.0, 1.0]."""
    try:
        with open(filepath, 'rb') as f:
            header = f.read(16)
            if len(header) < 16 or header[:4] != b'VTBL':
                return None
            raw = f.read()
            samples = np.frombuffer(raw, dtype=np.int16)
            num_frames = len(samples) // TABLE_SIZE
            if num_frames == 0:
                return None
            frames = samples[:num_frames * TABLE_SIZE].reshape(num_frames, TABLE_SIZE)
            return frames.astype(np.float32) * (1.0 / 32768.0)
    except Exception as e:
        print(f"Error reading {filepath}: {e}", file=sys.stderr)
        return None


def extract_4_temporal_frames(vt_frames: np.ndarray) -> np.ndarray:
    """Extracts 4 temporal frames (Attack, Punch, Sustain, Tail) from an (N, 2048) VitalTable."""
    N = vt_frames.shape[0]
    out = np.zeros((4, TABLE_SIZE), dtype=np.float32)
    if N == 1:
        for i in range(4):
            out[i] = vt_frames[0]
    elif N == 2:
        out[0] = vt_frames[0]
        out[1] = vt_frames[0]
        out[2] = vt_frames[1]
        out[3] = vt_frames[1]
    elif N == 3:
        out[0] = vt_frames[0]
        out[1] = vt_frames[1]
        out[2] = vt_frames[2]
        out[3] = vt_frames[2]
    else:
        # N >= 4
        i0 = 0
        i1 = int(math.floor(0.25 * (N - 1)))
        i2 = int(math.floor(0.60 * (N - 1)))
        i3 = N - 1
        out[0] = vt_frames[i0]
        out[1] = vt_frames[i1]
        out[2] = vt_frames[i2]
        out[3] = vt_frames[i3]
    return out


def generate_fallback_frames(cat: int, lin: int) -> np.ndarray:
    """Procedural fallback generating (4, 2048) frames if no VitalTable is found."""
    out = np.zeros((4, TABLE_SIZE), dtype=np.float32)
    two_pi = 2.0 * math.pi
    seed = (cat * 7919 + lin * 1999 + 0xDEADBEEF) & 0xFFFFFFFF
    
    for frame in range(4):
        attack_weight = 1.0 if frame == 0 else (0.35 if frame == 1 else 0.0)
        punch_weight = 1.0 if frame == 1 else (0.40 if frame == 0 else (0.25 if frame == 2 else 0.0))
        sustain_weight = 1.0 if frame == 2 else (0.30 if frame == 1 else (0.60 if frame == 3 else 0.0))
        
        for i in range(TABLE_SIZE):
            ph = two_pi * float(i) / float(TABLE_SIZE)
            seed = (seed * 1664525 + 1013904223) & 0xFFFFFFFF
            noise = float(struct.unpack('i', struct.pack('I', seed))[0]) / 2147483648.0
            
            fund = math.sin(ph)
            harm2 = math.sin(2.0 * ph)
            harm3 = math.sin(3.0 * ph)
            
            if cat == 0:  # KICK
                val = fund * (0.8 + 0.2 * punch_weight) + harm2 * 0.3 * punch_weight + harm3 * 0.1 * attack_weight
            elif cat == 1:  # SNARE
                val = fund * 0.4 * punch_weight + harm2 * 0.3 + noise * (0.8 * attack_weight + 0.5 * punch_weight)
            elif cat == 2:  # TOMS
                val = fund * (0.7 + 0.3 * sustain_weight) + harm2 * 0.35 * punch_weight + noise * 0.2 * attack_weight
            elif cat == 3:  # CLAP
                burst = noise * (0.6 + 0.4 * math.sin(ph * 3.0))
                val = burst * (0.9 * attack_weight + 0.5 * punch_weight) + fund * 0.2 * sustain_weight
            elif cat == 4:  # HATS
                val = (0.3 if math.sin(ph * 3.14) > 0 else -0.3) + noise * (0.7 * attack_weight + 0.4 * sustain_weight)
            elif cat == 5:  # CYMBALS
                val = (math.sin(ph * 2.37) * 0.3 + math.sin(ph * 4.19) * 0.25) + noise * (0.5 * attack_weight + 0.6 * sustain_weight)
            elif cat == 6:  # PERCUSSION
                val = math.sin(ph) * 0.5 + math.sin(ph * 2.76) * 0.35 + noise * 0.3 * attack_weight
            else:  # STABS & BASS
                val = fund * 0.6 + harm2 * 0.3 + harm3 * 0.2 * punch_weight
            out[frame, i] = max(-1.0, min(1.0, math.tanh(val * 1.2)))
    return out


def generate_secret_core() -> np.ndarray:
    """Generates (4, 4, 2048) frames for Slot 160 (Secret Core)."""
    out = np.zeros((NUM_MORPH_STEPS, NUM_FRAMES, TABLE_SIZE), dtype=np.float32)
    two_pi = 2.0 * math.pi
    
    for morph in range(NUM_MORPH_STEPS):
        for frame in range(NUM_FRAMES):
            frame_body = 1.0 if frame == 1 else (0.85 if frame == 2 else 0.5)
            frame_transient = 0.35 if frame == 0 else 0.0
            
            for i in range(TABLE_SIZE):
                ph = two_pi * float(i) / float(TABLE_SIZE)
                if morph == 0:
                    val = math.sin(ph)
                elif morph == 1:
                    val = math.tanh(1.85 * math.sin(ph)) * 0.82
                elif morph == 2:
                    saw = sum((1.0 / k) * (math.sin(math.pi * k / 17.0) / (math.pi * k / 17.0)) * math.sin(k * ph) for k in range(1, 17))
                    val = saw * 0.65
                else:
                    saw = sum((1.0 / k) * (math.sin(math.pi * k / 33.0) / (math.pi * k / 33.0)) * math.sin(k * ph) for k in range(1, 33))
                    formant = sum((0.35 / k) * (math.sin(math.pi * k / 17.0) / (math.pi * k / 17.0)) * math.sin(2.0 * k * ph) for k in range(1, 17))
                    val = math.tanh(1.6 * (saw * 0.65 + formant)) * 0.85
                
                if frame == 0 and i < 128:
                    click = 1.0 - float(i) / 128.0
                    val += click * frame_transient * math.sin(ph * 4.0)
                
                out[morph, frame, i] = max(-1.0, min(1.0, val * frame_body))
    return out


def main():
    repo_root = Path(r"C:\Dev\TheKlangSuite")
    vt_base = Path(r"C:\Dev\TheKlangDSP\data\wavetables")
    output_bin = repo_root / "assets" / "digistrike_fs2_rom.bin"
    output_bin.parent.mkdir(parents=True, exist_ok=True)

    print("=" * 70)
    print("DigiStrike FS2 Fleet VitalTable ROM Baker")
    print(f"Source Directory: {vt_base}")
    print(f"Target Binary:    {output_bin}")
    print("=" * 70)

    # Master ROM tensor: (161, 4, 4, 2048) in float32
    rom_tensor = np.zeros((NUM_SLOTS, NUM_MORPH_STEPS, NUM_FRAMES, TABLE_SIZE), dtype=np.float32)

    mapped_count = 0
    fallback_count = 0

    for cat in range(NUM_CATEGORIES):
        kw_list = CAT_KEYWORDS[cat]
        for lin, (lin_name, folder) in enumerate(LINEAGES):
            slot = cat * NUM_LINEAGES + lin
            dir_path = vt_base / folder
            all_tables = sorted(glob.glob(str(dir_path / "**/*.vitaltable"), recursive=True))
            
            # Special case for Kawai Orch Stab (Slot 144: Cat 7, Lin 4)
            if cat == 7 and lin == 4:
                orch_tables = [
                    dir_path / "KE50_HIT_ORCH.vitaltable",
                    dir_path / "R50_III_ORCH_HIT_01.vitaltable",
                    dir_path / "R50_III_ORCH_HIT_02.vitaltable",
                    dir_path / "KE50_HIT_FLNG.vitaltable"
                ]
                chosen = [str(p) for p in orch_tables if p.exists()]
            else:
                matches = [f for f in all_tables if any(k in os.path.basename(f).lower() for k in kw_list)]
                if matches:
                    chosen = matches
                elif all_tables:
                    # Fallback to any available table in lineage
                    chosen = all_tables
                else:
                    chosen = []

            if chosen:
                mapped_count += 1
                # Populate 4 morph steps
                for morph in range(NUM_MORPH_STEPS):
                    table_path = chosen[morph % len(chosen)]
                    vt_data = read_vitaltable(table_path)
                    if vt_data is not None:
                        frames_4 = extract_4_temporal_frames(vt_data)
                        rom_tensor[slot, morph] = frames_4
                    else:
                        rom_tensor[slot, morph] = generate_fallback_frames(cat, lin)
            else:
                fallback_count += 1
                frames_4 = generate_fallback_frames(cat, lin)
                for morph in range(NUM_MORPH_STEPS):
                    rom_tensor[slot, morph] = frames_4

    # Slot 160: Secret Core
    rom_tensor[SECRET_CORE_SLOT] = generate_secret_core()

    print(f"Mapped Slots from VitalTables: {mapped_count} / {NUM_HERO_SLOTS}")
    print(f"Procedural Fallback Slots:     {fallback_count} / {NUM_HERO_SLOTS}")
    print(f"Secret Core Slot 160:          Configured (4 morph steps x 4 frames)")

    # Convert to 16-bit signed PCM
    int16_tensor = np.clip(rom_tensor * 32767.0, -32768.0, 32767.0).astype(np.int16)
    expected_bytes = NUM_SLOTS * NUM_MORPH_STEPS * NUM_FRAMES * TABLE_SIZE * 2
    raw_bytes = int16_tensor.tobytes()
    assert len(raw_bytes) == expected_bytes, f"Size mismatch: {len(raw_bytes)} vs {expected_bytes}"

    with open(output_bin, 'wb') as f:
        f.write(raw_bytes)

    print(f"Successfully baked ROM to {output_bin} ({len(raw_bytes)} bytes / {len(raw_bytes) / (1024*1024):.2f} MB)")
    return 0


if __name__ == '__main__':
    sys.exit(main())
