import sys
import os
import zipfile
import json
from pathlib import Path

# Universal UTF-8 stdout
sys.stdout.reconfigure(encoding='utf-8')

BANK_NAMES = [
    "00 - Factory Signatures",
    "01 - Bass Vault",
    "02 - Strings & Orchestral",
    "03 - Keys & Timbral",
    "04 - Pianos & Organs",
    "05 - Pads & Bells",
    "06 - Piano Collection",
    "07 - 1987 Analog & Textures",
    "08 - Digital Classics",
    "09 - E-Pianos & Leads"
]

def decode_patch(raw_bytes: bytes, patch_idx: int, bank_idx: int):
    # Name: bytes 0..7 (8 ASCII characters)
    name = ''.join(chr(b) if 32 <= b <= 126 else ' ' for b in raw_bytes[:8]).strip()
    if not name:
        name = f"PATCH_{bank_idx}_{patch_idx+1:02d}"

    # Harmonics S1 & S2 (63 pairs starting at byte 62)
    s1_harmonics = [float(raw_bytes[62 + 2*k]) for k in range(63)]
    s2_harmonics = [float(raw_bytes[62 + 2*k + 1]) for k in range(63)]

    # DHG Envelope assignments (bytes 194..225: 16 pairs of bytes)
    # Each byte has 4 2-bit fields
    s1_assign = []
    s2_assign = []
    for k in range(63):
        pair_byte_idx = 194 + (k // 4) * 2
        shift = (k % 4) * 2
        s1_assign.append((raw_bytes[pair_byte_idx] >> shift) & 0x03)
        s2_assign.append((raw_bytes[pair_byte_idx + 1] >> shift) & 0x03)

    harmonic_env_assign = s1_assign + s2_assign

    # DHG Envelopes 1..4 (6 stages each: rates & levels)
    # On K5: envelope rate/level pairs
    dhg_envs = []
    for e in range(4):
        # Scan stages
        stages = []
        base_rate = 280 + e * 24
        for s in range(6):
            r = float(raw_bytes[base_rate + 2*s] & 0x7F) if base_rate + 2*s < len(raw_bytes) else 50.0
            l = float(raw_bytes[base_rate + 12 + 2*s] & 0x7F) if base_rate + 12 + 2*s < len(raw_bytes) else (99.0 if s == 0 else 0.0)
            stages.append({"rate": min(r, 99.0), "level": min(l, 99.0)})
        dhg_envs.append({"stages": stages, "loop34": (e == 1)})

    # Construct Preset Dictionary
    preset = {
        "$schema": "https://crackedchimes.com/schemas/tkpreset-v1.json",
        "schema_version": 1,
        "plugin_id": "Shoggoth",
        "plugin_version": "1.0.0",
        "name": name,
        "bank": BANK_NAMES[bank_idx],
        "category": "Vintage 1987 Additive",
        "author": "Vintage 1987 Sound Design Team",
        "description": f"Authentic 1987 additive synthesis patch #{patch_idx+1:02d} from Bank {bank_idx}.",
        "tags": ["vintage", "1987", "additive", BANK_NAMES[bank_idx].split(" - ")[-1].lower()],
        "parameters": {
            "filter_cutoff": 20000.0,
            "filter_resonance": 0.0,
            "spectral_tilt": 0.0,
            "source_morph": 0.0,
            "odd_even_bal": 0.0,
            "dissonance_warp": 1.0,
            "post_ddf_cutoff": 20000.0,
            "post_ddf_reso": 0.0,
            "amp_attack": 0.01,
            "amp_decay": 0.35,
            "amp_sustain": 0.80,
            "amp_release": 0.35,
            "filter_env_depth": 0.0,
            "pitch_punch_depth": 0.0,
            "glide_time": 0.0,
            "chaos_amount": 0.0,
            "alpha_chorus": 0,
            "waterbed_depth": 0.0,
            "swarm_coupling": 0.0,
            "fold_wall": 24000.0,
            "mixer_seethe": 0.0,
            "master_gain_db": 0.0,
            "mpe_slide_depth": 0.0,
            "mpe_pressure_depth": 0.0,
            "osc_slop": 0.02,
            "air_absorption": 0.1,
            "vel_brightness": 0.3,
            "modal_release": 0.1,
            "life_mode": 0,
            "vintage_engine_mode": 0,
            "voice_mode": 0,
            "cpu_tier": 1
        },
        "custom_state": {
            "num_harmonics": 63,
            "s1_harmonics": s1_harmonics,
            "s2_harmonics": s2_harmonics,
            "harmonic_env_assign": harmonic_env_assign,
            "dhg_envelopes": dhg_envs
        }
    }
    return preset

def convert_all():
    zip_path = Path(r"C:\_\Dropbox\Gear\Kawai\K5\kawai-k5\KAWAI.K5\sounds-sysex.zip")
    out_base = Path(r"C:\Dev\TheKlangSuite\assets\presets\shoggoth")
    out_base.mkdir(parents=True, exist_ok=True)

    if not zip_path.exists():
        print(f"ERROR: Cannot find SysEx zip at {zip_path}")
        return False

    total_converted = 0
    with zipfile.ZipFile(zip_path) as z:
        for bank_idx in range(10):
            bank_file = f"K5/BANK-{bank_idx}.SYX"
            data = z.read(bank_file)

            bank_dir = out_base / BANK_NAMES[bank_idx]
            bank_dir.mkdir(parents=True, exist_ok=True)

            packets = []
            idx = 0
            while idx < len(data):
                start = data.find(b'\xf0', idx)
                if start == -1: break
                end = data.find(b'\xf7', start)
                if end == -1: break
                pkt = data[start:end+1]
                idx = end + 1
                if len(pkt) == 993:
                    packets.append(pkt)

            print(f"Processing {BANK_NAMES[bank_idx]}: {len(packets)} patches...")
            for p_idx, pkt in enumerate(packets):
                nibbles = pkt[8:-1]
                raw = bytes([(nibbles[2*i] << 4) | (nibbles[2*i+1] & 0x0f) for i in range(492)])
                preset = decode_patch(raw, p_idx, bank_idx)

                # Clean filename
                safe_name = "".join(c if c.isalnum() or c in " _-+" else "_" for c in preset["name"]).strip()
                if not safe_name:
                    safe_name = f"Patch_{p_idx+1:02d}"
                filename = f"{p_idx+1:02d}_{safe_name}.json"

                file_path = bank_dir / filename
                with open(file_path, "w", encoding="utf-8") as f:
                    json.dump(preset, f, indent=2, ensure_ascii=False)

                total_converted += 1

    print(f"SUCCESS: Converted {total_converted} presets across 10 banks into {out_base}")
    return True

if __name__ == "__main__":
    convert_all()
