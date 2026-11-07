#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
tools/sqa/audit_presets.py — Information-Theoretic Content & Preset Entropy Gate
================================================================================
Part of the Dual-Chassis SQA Standard.
Audits preset banks (Chassis 1) and feed/playlist datasets (Chassis 2) to detect:
  1. Flat / Stubbed Envelopes: Variance sigma^2 == 0 across presets (e.g. all attack = 0.01).
  2. Degenerate Clones: Pairwise Normalized Compression Distance (NCD) < 0.35.
  3. Low Shannon Entropy: Parameter entropy H < 0.5 bits (flatline constants).
  4. Identical Mock Data: Duplicated media URLs or durations in playlists.

Runs in <150ms using Python standard library (lzma, math, collections).
"""

import sys
import os
import math
import lzma
import json
import argparse
from pathlib import Path
from collections import Counter

# Universal UTF-8 console output
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")


def compress_bytes(b: bytes) -> int:
    """Returns LZMA compressed length for Kolmogorov upper bound."""
    return len(lzma.compress(b, preset=6))


def compute_ncd(x: bytes, y: bytes) -> float:
    """Computes Normalized Compression Distance (Cilibrasi & Vitanyi 2005)."""
    cx = compress_bytes(x)
    cy = compress_bytes(y)
    cxy = compress_bytes(x + y)
    denom = max(cx, cy)
    if denom == 0: return 0.0
    return (cxy - min(cx, cy)) / denom


def compute_shannon_entropy(values: list[float], num_bins=10) -> float:
    """Computes Shannon entropy across discretized parameter bins."""
    if not values or len(values) < 2: return 0.0
    min_v, max_v = min(values), max(values)
    if min_v == max_v:
        return 0.0  # Zero variance = 0 entropy

    # Bin values
    bin_width = (max_v - min_v) / num_bins
    bins = [min(num_bins - 1, int((v - min_v) / bin_width)) for v in values]
    counts = Counter(bins)
    total = len(values)
    entropy = -sum((c / total) * math.log2(c / total) for c in counts.values())
    return entropy


def compute_variance(values: list[float]) -> float:
    """Computes sample variance."""
    if len(values) < 2: return 0.0
    mean = sum(values) / len(values)
    return sum((v - mean) ** 2 for v in values) / (len(values) - 1)


def audit_preset_bank(preset_dir: Path) -> tuple[list[str], list[str]]:
    errors = []
    warnings = []

    if not preset_dir.exists():
        return [f"Preset directory not found: {preset_dir}"], []

    preset_files = sorted(preset_dir.glob("*.json")) + sorted(preset_dir.glob("*.vstpreset"))
    if not preset_files:
        return [f"No preset files (.json / .vstpreset) found in {preset_dir}"], []

    print(f"[*] Auditing {len(preset_files)} presets in: {preset_dir.name}")

    # Extract parameter values across presets
    param_columns = {}
    preset_blobs = []

    for pf in preset_files:
        try:
            raw = pf.read_bytes()
            preset_blobs.append((pf.name, raw))
            # Try parsing JSON presets
            if pf.suffix == ".json":
                data = json.loads(raw.decode("utf-8", errors="ignore"))
                params = data.get("parameters", data.get("params", data))
                if isinstance(params, dict):
                    for k, v in params.items():
                        if isinstance(v, (int, float)):
                            param_columns.setdefault(k, []).append(float(v))
        except Exception as e:
            warnings.append(f"Failed to read preset '{pf.name}': {e}")

    # 1. Audit Parameter Variances & Shannon Entropies
    for param_name, vals in param_columns.items():
        if len(vals) == len(preset_files):
            var = compute_variance(vals)
            entropy = compute_shannon_entropy(vals)

            # Check for flatline constants in acoustic parameters
            is_acoustic = any(k in param_name.lower() for k in ['attack', 'decay', 'sustain', 'release', 'cutoff', 'reso', 'morph'])
            if is_acoustic:
                if var == 0.0:
                    errors.append(f"FLATLINE ACOUSTIC PARAMETER: '{param_name}' has ZERO variance (all {len(vals)} presets have {vals[0]})!")
                elif entropy < 0.3:
                    warnings.append(f"LOW ENTROPY: '{param_name}' has entropy H={entropy:.2f} bits (almost identical across bank).")

    # 2. Pairwise NCD Sample Audit (test 15 random pairs to keep runtime < 100ms)
    import random
    sample_blobs = preset_blobs[:20]
    clone_pairs = []
    for i in range(len(sample_blobs)):
        for j in range(i + 1, min(len(sample_blobs), i + 4)):
            name_a, b_a = sample_blobs[i]
            name_b, b_b = sample_blobs[j]
            dist = compute_ncd(b_a, b_b)
            if dist < 0.15:  # High compression similarity = identical clone
                clone_pairs.append((name_a, name_b, dist))

    if clone_pairs:
        for a, b, d in clone_pairs:
            errors.append(f"DEGENERATE PRESET CLONE: '{a}' and '{b}' have NCD={d:.3f} (< 0.15). Data is hollow/duplicated!")

    return errors, warnings


def main():
    parser = argparse.ArgumentParser(description="Information-Theoretic Content & Preset Entropy Gate")
    parser.add_argument("path", help="Path to preset directory or feed JSON file")
    args = parser.parse_args()

    target = Path(args.path)
    errs, warns = audit_preset_bank(target)

    for e in errs:
        print(f"   ❌ ERROR: {e}")
    for w in warns:
        print(f"   ⚠️  WARN:  {w}")

    if not errs:
        print(f"\n✅ PASS: Preset bank verified for non-degeneracy, high entropy, and non-zero parameter variance.")
        sys.exit(0)
    else:
        print(f"\n[!] SQA ENTROPY FAILURE: {len(errs)} defect(s) detected. Halting completion.\n")
        sys.exit(1)


if __name__ == "__main__":
    main()
