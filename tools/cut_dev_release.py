#!/usr/bin/env python3
"""
tools/cut_dev_release.py — The Shard / The Klang Suite Dev Release Packager
==========================================================================
Automates the packaging of developer release bundles (e.g. v0.4.0-dev.1)
containing:
1. Native VST3 plugin (The Shard.vst3 / The Klang Planter.vst3)
2. Standalone desktop executable (The Shard.exe)
3. Zero-install interactive browser wireframe (The Shard Wireframe.html)
4. Comprehensive 1-page tester brief & feedback guide (TESTER_GUIDE.md)

Outputs packaged zip to current_build/releases/ with verified SHA-256 hash.
"""

import argparse
import hashlib
import os
import shutil
import subprocess
import sys
import zipfile
from pathlib import Path

# Force UTF-8 terminal output on Windows
if sys.platform == "win32":
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path("C:/Dev/TheKlangSuite")
VAULT_ROOT = Path("C:/_/Vault")
CURRENT_BUILD = REPO_ROOT / "current_build"
RELEASES_DIR = CURRENT_BUILD / "releases"
BUILD_DIR = REPO_ROOT / "build"
SIDECAR_HTML = VAULT_ROOT / ".agents" / "sidecar" / "shard_ux_wireframe.html"

TESTER_GUIDE_CONTENT = """# 💠 The Shard (v{version}) — External Tester Guide
### The Klang Suite • Polyphonic Multi-Engine Synthesizer
*Feedback & Ergonomic Brief for Private Testing*

---

## 1. What is The Shard?

**The Shard** is the flagship polyphonic synthesis instrument within **The Klang Suite**.
Engineered for sound designers, tracker purists, and electronic musicians, it merges the raw, single-window immediacy of **Renoise device views** and **Ableton Live instruments** with high-precision modern C++20 DSP and zero menu-diving.

### Core Architecture
- **12 Switchable DSP Synthesis Engines**: VA, Kvltr1 (Acid), FM Dual-Op, Additive Morph, Macro Phase, Noise Karplus, Supersaw, Vector, Physical String, etc.
- **Dual Voice-Insert FX Architecture**:
  - **Insert 1 (Stomp)**: Boss HM-2 chainsaw, ProCo Rat, Ibanez TS9, EHX Big Muff, BBD Chorus.
  - **Insert 2 (Space)**: EchoMill multi-tap delay, KlangSpace algorithmic reverb, Freeze resonator.
- **16-Filter Matrix**: Dual cascaded biquads with Moog LP24, Oberheim LP12/BP12/HP12, Notch, Comb, and pre-filter saturating drive.
- **QuadraMod & 16-Control Canon**: 4 multi-mode modulation slots (AHD, Audio-Rate LFO, Multi-LFO, Macro) routed via 8 Front Encoders + 8 Rear Sound Lab Controls.
- **Universal Polyphony Engine**: True Mono (priority stack & re-trigger), Legato (smooth pitch glide), 4-Voice, and 8-Voice polyphony with oldest-voice stealing.

---

## 2. Package Contents & How to Run

This release bundle contains three ways to experience and test The Shard:

```
├── The Shard.exe                 -> Standalone desktop app (runs instantly, zero DAW required)
├── The Shard.vst3/               -> Native VST3 plugin bundle (drop into your DAW)
│   └── Contents/x86_64-win/
│       └── The Shard.vst3
├── The Klang Planter.vst3/       -> Backward-compatible VST3 binary alias
├── The Shard Wireframe.html      -> Zero-install interactive browser instrument (runs on any OS)
└── TESTER_GUIDE.md               -> This document
```

### Option A: Standalone Desktop App (Recommended for Quick Testing)
1. Double-click `The Shard.exe`.
2. Go to **Audio/MIDI Settings** (gear icon or menu) and select your audio interface & MIDI keyboard.
3. Play notes via MIDI keyboard or on-screen keys.

### Option B: Native VST3 in Your DAW
1. Copy the `The Shard.vst3` folder into your system VST3 directory:
   - **Windows**: `C:\\Program Files\\Common Files\\VST3\\`
2. Rescan plugins in your DAW (Ableton Live, Renoise, Bitwig, Reaper, FL Studio, Cubase).
3. Insert `The Shard` (or `The Klang Planter`) onto a MIDI track.

### Option C: Zero-Install Interactive Browser Wireframe
1. Double-click `The Shard Wireframe.html` to open it in **Chrome, Edge, Brave, or Firefox**.
2. Works on **Windows, macOS, Linux, iPad, and Android phones** with zero installation!
3. Click any key on the on-screen keyboard, or use your computer keyboard (`A, W, S, E, D, F, G...`).
4. Powered by real-time Web Audio DSP modeling all 12 engines, filter matrices, and stompboxes.

---

## 3. Rotary Knob Mechanics (Ratified Canon)

The Shard implements a distinct, purposeful rotary encoder standard:

```
       UNIPOLAR (0.0 to 1.0)                      BIPOLAR (-1.0 to +1.0)
             12:00 (0.5)                                12:00 (0.0)
                 │                                          │
       ╭─────────┴─────────╮                      ╭─────────┴─────────╮
       │      Clockwise    │                      │ Anti-Sun   Clock  │
       │                   │                      │ (Negative) (Pos)  │
       ╰─────────┬─────────╯                      ╰─────────┬─────────╯
                 │                                          │
             6:00 (0.0)                                 6:00 (±1.0)
           [Zero Point]                                 [Extremes]
```

- **Unipolar Controls (Cutoff, Res, Drive, Delay, Reverb, Volume)**:
  - **Zero Point is at 6:00 (`0.0`)**.
  - Sweeps **clockwise**: `6:00 (0.0)` $\\rightarrow$ `12:00 (0.5)` $\\rightarrow$ `6:00 (1.0)` (full 360° ring).
- **Bipolar Controls (Detune, Tone, Balance, Mod Depth, Pan)**:
  - **Zero / Center Point is at 12:00 (`0.0`)**.
  - Positive values sweep **clockwise** from `12:00` to `6:00` (`+100%`).
  - Negative values sweep **anti-sunwise (counter-clockwise)** from `12:00` to `6:00` (`-100%`).
- **Reset to Default**: Double-click any knob to instantly snap back to default (`12:00` for bipolar, standard value for unipolar).

---

## 4. Targeted Tester Feedback Questions

Please test the instrument and let us know your thoughts on the following 5 focus areas:

1. **Rotary Knob Ergonomics**:
   - How does the `6:00`-start unipolar and `12:00`-center bipolar feel compared to traditional 7:00–5:00 pot sweeps?
   - Is the visual needle and arc direction immediately obvious?
2. **Layout & Density**:
   - Does the flat, single-window Renoise/Ableton brutalist layout feel fast to work with?
   - Are any panels (Engines, Filter, Inserts, Rear Sound Lab) visually cramped or hard to read?
3. **Filter Scaling & Sweet Spots**:
   - How does the filter cutoff logarithmic response feel across the audio band? Does it move smoothly through the sub-bass and midrange?
4. **Polyphony & Voice Response**:
   - Test switching between `MONO`, `LEGATO`, `4-VOICE`, and `8-VOICE`.
   - How does the voice stealing feel under rapid, complex playing?
5. **Sonic Character**:
   - Which engine + filter + insert combinations stand out most to you? Any engine presets you would like to see expanded?

---

*Thank you for testing The Shard v{version}! Send your notes and feedback directly to the development team.*
"""


def compute_sha256(file_path: Path) -> str:
    h = hashlib.sha256()
    with open(file_path, "rb") as f:
        while chunk := f.read(65536):
            h.update(chunk)
    return h.hexdigest()


def main():
    parser = argparse.ArgumentParser(description="Package The Shard Dev Release")
    parser.add_argument("--version", default="0.4.0-dev.1", help="Release version string")
    parser.add_argument("--no-build", action="store_true", help="Skip CMake build step")
    args = parser.parse_args()

    version = args.version
    print(f"============================================================")
    print(f"💠 PACKAGING THE SHARD DEV RELEASE: v{version}")
    print(f"============================================================")

    RELEASES_DIR.mkdir(parents=True, exist_ok=True)
    staging_dir = BUILD_DIR / f"staging_TheShard_v{version}"
    if staging_dir.exists():
        shutil.rmtree(staging_dir)
    staging_dir.mkdir(parents=True)

    # 1. Build targets if requested
    if not args.no_build:
        print("[1/5] Building Release targets with CMake / MSVC...")
        build_cmd = [
            "cmake",
            "--build",
            str(BUILD_DIR),
            "--config",
            "Release",
            "--target",
            "TheKlangPlanter_Standalone",
            "TheKlangPlanter_VST3",
            "-j",
            "8",
        ]
        res = subprocess.run(build_cmd, capture_output=True, text=True)
        if res.returncode != 0:
            print("ERROR: Build failed!")
            print(res.stderr[-1000:] if res.stderr else res.stdout[-1000:])
            sys.exit(1)
        print("  Build completed successfully.")
    else:
        print("[1/5] Skipping build step (--no-build).")

    # 2. Locate built binaries
    print("[2/5] Staging Standalone executable and VST3 bundles...")
    standalone_src = CURRENT_BUILD / "Standalone" / "The Klang Planter.exe"
    if not standalone_src.exists():
        standalone_src = BUILD_DIR / "TheKlangPlanter_artefacts" / "Release" / "Standalone" / "The Klang Planter.exe"
    if not standalone_src.exists():
        print(f"ERROR: Cannot find The Klang Planter.exe at {standalone_src}")
        sys.exit(1)

    vst3_src = CURRENT_BUILD / "VST3" / "The Klang Planter.vst3"
    if not vst3_src.exists() or not any(vst3_src.iterdir()):
        vst3_src = BUILD_DIR / "TheKlangPlanter_artefacts" / "Release" / "VST3" / "The Klang Planter.vst3"
    if not vst3_src.exists():
        print(f"ERROR: Cannot find The Klang Planter.vst3 at {vst3_src}")
        sys.exit(1)

    # Copy Standalone as The Shard.exe
    shutil.copy2(standalone_src, staging_dir / "The Shard.exe")
    print(f"  Copied: The Shard.exe ({standalone_src.stat().st_size:,} bytes)")

    # Copy VST3 bundle as The Shard.vst3
    shard_vst3_dest = staging_dir / "The Shard.vst3"
    shutil.copytree(vst3_src, shard_vst3_dest)
    # Rename inner binary if present
    win_bin_dir = shard_vst3_dest / "Contents" / "x86_64-win"
    if win_bin_dir.exists():
        old_bin = win_bin_dir / "The Klang Planter.vst3"
        new_bin = win_bin_dir / "The Shard.vst3"
        if old_bin.exists():
            shutil.copy2(old_bin, new_bin)
            old_bin.unlink()
    print(f"  Staged: The Shard.vst3 bundle")

    # Also include The Klang Planter.vst3 alias for full backward compatibility
    planter_vst3_dest = staging_dir / "The Klang Planter.vst3"
    shutil.copytree(vst3_src, planter_vst3_dest)
    print(f"  Staged: The Klang Planter.vst3 (compatibility alias)")

    # 3. Stage Interactive Web Wireframe
    print("[3/5] Staging interactive browser wireframe...")
    if not SIDECAR_HTML.exists():
        # Fallback to research directory
        alt_html = VAULT_ROOT / "New Klang City" / "Research" / "ui_ux" / "shard_ux_wireframe.html"
        if alt_html.exists():
            shutil.copy2(alt_html, staging_dir / "The Shard Wireframe.html")
        else:
            print(f"WARNING: Cannot find shard_ux_wireframe.html at {SIDECAR_HTML}")
    else:
        shutil.copy2(SIDECAR_HTML, staging_dir / "The Shard Wireframe.html")
    print(f"  Copied: The Shard Wireframe.html ({SIDECAR_HTML.stat().st_size:,} bytes)")

    # 4. Generate TESTER_GUIDE.md
    print("[4/5] Generating TESTER_GUIDE.md...")
    guide_path = staging_dir / "TESTER_GUIDE.md"
    guide_text = TESTER_GUIDE_CONTENT.format(version=version)
    with open(guide_path, "w", encoding="utf-8") as f:
        f.write(guide_text)
    print(f"  Generated: TESTER_GUIDE.md ({len(guide_text):,} bytes)")

    # 5. Create Distribution Zip Bundles
    print("[5/5] Compressing Tri-Bundle release archives...")
    zip_shard_name = f"TheShard_v{version}_win64.zip"
    zip_suite_name = f"TheKlangSuite_v{version}_win64.zip"
    zip_shard_path = RELEASES_DIR / zip_shard_name
    zip_suite_path = RELEASES_DIR / zip_suite_name

    with zipfile.ZipFile(zip_shard_path, "w", zipfile.ZIP_DEFLATED) as zf:
        for root, dirs, files in os.walk(staging_dir):
            for file in files:
                file_path = Path(root) / file
                rel_path = file_path.relative_to(staging_dir)
                zf.write(file_path, arcname=rel_path)

    # Mirror to suite name for standard naming
    shutil.copy2(zip_shard_path, zip_suite_path)

    # Cleanup staging directory
    shutil.rmtree(staging_dir)

    # Calculate SHA-256
    sha256_hash = compute_sha256(zip_shard_path)
    zip_size_mb = zip_shard_path.stat().st_size / (1024 * 1024)

    print()
    print("============================================================")
    print("✅ RELEASE PACKAGING COMPLETE")
    print("============================================================")
    print(f"📦 Archive: {zip_shard_path.name}")
    print(f"📁 Path:    {zip_shard_path}")
    print(f"📊 Size:    {zip_size_mb:.2f} MB ({zip_shard_path.stat().st_size:,} bytes)")
    print(f"🔒 SHA-256: {sha256_hash}")
    print(f"📋 Also mirrored to: {zip_suite_name}")
    print("============================================================")


if __name__ == "__main__":
    main()
