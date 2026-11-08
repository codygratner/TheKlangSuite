#!/usr/bin/env python3
"""
tools/bake_nyarlat_rom.py — Vital Phase-Aligned AKWF ROM Baker for Nyarlat
=============================================================================
Part of The Klang Suite. Copyright (c) 2026 Cracked Chimes / Klang Industries.

Ingests all ~4,358 single-cycle waveforms from C:/Dev/Research/upstream/AKWF-FREE/AKWF,
applies Vital-style fundamental phase neutralization (circular time alignment:
phi'_k = phi_k - k * phi_1), resamples to 512 float32 samples, strips DC,
and packs into assets/nyarlat_akwf_rom.bin.
"""

import os
import sys
import struct
import numpy as np
from pathlib import Path
import scipy.io.wavfile as wavfile

# UTF-8 console output
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

ROOT_TKS = Path(__file__).resolve().parent.parent
AKWF_DIR = Path(r"C:\Dev\Research\upstream\AKWF-FREE\AKWF")
OUTPUT_BIN = ROOT_TKS / "assets" / "nyarlat_akwf_rom.bin"
OUTPUT_H = ROOT_TKS / "source" / "nyarlat" / "NyarlatRomMetadata.h"

TABLE_SIZE = 512

def process_waveform(file_path: Path) -> np.ndarray:
    """Reads a WAV file, aligns fundamental phase, strips DC, and returns 512 float32 samples."""
    try:
        sr, data = wavfile.read(file_path)
    except Exception as e:
        # Fallback raw read
        with open(file_path, "rb") as f:
            f.seek(44)
            raw = f.read()
        data = np.frombuffer(raw, dtype=np.int16)

    # Convert to mono float in [-1.0, 1.0]
    if data.ndim > 1:
        data = data[:, 0]
    
    if data.dtype == np.int16:
        signal = data.astype(np.float64) / 32768.0
    elif data.dtype == np.int32:
        signal = data.astype(np.float64) / 2147483648.0
    elif data.dtype == np.uint8:
        signal = (data.astype(np.float64) - 128.0) / 128.0
    else:
        signal = data.astype(np.float64)

    N = len(signal)
    if N < 8:
        return np.zeros(TABLE_SIZE, dtype=np.float32)

    # FFT analysis
    X = np.fft.rfft(signal)
    
    # Strip DC
    X[0] = 0.0

    # Fundamental harmonic (k=1)
    if len(X) > 1 and np.abs(X[1]) > 1e-7:
        phi_1 = np.angle(X[1])
        # Circular phase rotation: phi'_k = phi_k - k * phi_1
        k_indices = np.arange(len(X))
        rotation = np.exp(-1j * k_indices * phi_1)
        X_aligned = X * rotation
    else:
        X_aligned = X

    # Bandlimit to avoid Nyquist aliasing at 512 samples
    max_k = TABLE_SIZE // 2
    if len(X_aligned) > max_k:
        X_aligned = X_aligned[:max_k]
    elif len(X_aligned) < max_k:
        X_aligned = np.pad(X_aligned, (0, max_k - len(X_aligned)))

    # IFFT directly to TABLE_SIZE (512)
    aligned_512 = np.fft.irfft(X_aligned, n=TABLE_SIZE)

    # Peak normalization to 0.98 to avoid intersample clipping
    peak = np.max(np.abs(aligned_512))
    if peak > 1e-5:
        aligned_512 = (aligned_512 / peak) * 0.98

    return aligned_512.astype(np.float32)

def main():
    print("=== Nyarlat AKWF ROM Baker ===")
    if not AKWF_DIR.exists():
        print(f"Error: {AKWF_DIR} does not exist!", file=sys.stderr)
        sys.exit(1)

    OUTPUT_BIN.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT_H.parent.mkdir(parents=True, exist_ok=True)

    # Collect all banks
    bank_dirs = sorted([d for d in AKWF_DIR.iterdir() if d.is_dir()])
    print(f"Found {len(bank_dirs)} AKWF bank directories.")

    total_waves = 0
    banks_meta = []
    all_wave_buffers = []
    all_spectral_mags = []
    all_spectral_phases = []

    for bank_idx, bdir in enumerate(bank_dirs):
        bank_name = bdir.name.replace("AKWF_", "")
        wav_files = sorted(list(bdir.glob("*.wav")))
        count = len(wav_files)
        offset = total_waves

        print(f"  [{bank_idx:02d}] {bank_name:<20} : {count:3d} waves (offset {offset:4d})")

        for wpath in wav_files:
            wave_512 = process_waveform(wpath)
            all_wave_buffers.append(wave_512)

            # Compute 64 partial FFT (magnitudes & phases)
            X_512 = np.fft.rfft(wave_512)
            m = np.abs(X_512[1:65]).astype(np.float32)
            max_m = np.max(m)
            if max_m > 1e-5:
                m = m / max_m
            p = np.angle(X_512[1:65]).astype(np.float32)
            all_spectral_mags.append(m)
            all_spectral_phases.append(p)

        banks_meta.append({
            "name": bank_name,
            "count": count,
            "offset": offset
        })
        total_waves += count

    print(f"\nTotal waveforms processed: {total_waves}")
    print(f"Packing into {OUTPUT_BIN}...")

    # Binary Format (Version 2):
    # Header:
    #   magic: char[4] = 'NYRT'
    #   version: uint32 = 2
    #   num_banks: uint32
    #   total_waves: uint32
    #   table_size: uint32 = 512
    #   num_partials: uint32 = 64
    # Bank Directory (num_banks records):
    #   name: char[32] (null-terminated)
    #   count: uint32
    #   offset: uint32
    # Spectral Magnitudes:
    #   total_waves * 64 * float32
    # Spectral Phases:
    #   total_waves * 64 * float32
    # Waveform samples:
    #   total_waves * 512 * float32

    with open(OUTPUT_BIN, "wb") as f:
        # Header (Version 2)
        f.write(b"NYRT")
        f.write(struct.pack("<IIIII", 2, len(banks_meta), total_waves, TABLE_SIZE, 64))

        # Bank Directory
        for b in banks_meta:
            name_bytes = b["name"].encode("ascii")[:31].ljust(32, b"\x00")
            f.write(name_bytes)
            f.write(struct.pack("<II", b["count"], b["offset"]))

        # Spectral Magnitudes (64 floats per wave)
        for m in all_spectral_mags:
            f.write(m.tobytes())

        # Spectral Phases (64 floats per wave)
        for p in all_spectral_phases:
            f.write(p.tobytes())

        # Waveform samples (512 floats per wave)
        for w in all_wave_buffers:
            f.write(w.tobytes())

    bin_size = OUTPUT_BIN.stat().st_size
    print(f"ROM baked successfully: {bin_size / (1024*1024):.2f} MB ({bin_size:,} bytes).")

    # Generate C++ metadata header
    with open(OUTPUT_H, "w", encoding="utf-8") as f:
        f.write("#pragma once\n\n")
        f.write("// Auto-generated by tools/bake_nyarlat_rom.py. DO NOT EDIT.\n\n")
        f.write("#include <cstdint>\n")
        f.write("#include <array>\n\n")
        f.write("namespace NyarlatRom {\n")
        f.write(f"    constexpr uint32_t kMagic = 0x5452594E; // 'NYRT'\n")
        f.write(f"    constexpr uint32_t kVersion = 2;\n")
        f.write(f"    constexpr uint32_t kNumBanks = {len(banks_meta)};\n")
        f.write(f"    constexpr uint32_t kTotalWaves = {total_waves};\n")
        f.write(f"    constexpr uint32_t kTableSize = {TABLE_SIZE};\n")
        f.write(f"    constexpr uint32_t kNumPartials = 64;\n\n")
        f.write("    struct BankInfo {\n")
        f.write("        const char* name;\n")
        f.write("        uint32_t count;\n")
        f.write("        uint32_t offset;\n")
        f.write("    };\n\n")
        f.write(f"    constexpr std::array<BankInfo, {len(banks_meta)}> kBanks = {{{{\n")
        for b in banks_meta:
            f.write(f'        {{ "{b["name"]}", {b["count"]}, {b["offset"]} }},\n')
        f.write("    }};\n")
        f.write("}\n")

    print(f"Emitted {OUTPUT_H}.")

if __name__ == "__main__":
    main()
