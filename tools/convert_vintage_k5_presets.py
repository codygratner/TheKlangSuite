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

def decode_patch(payload: bytes, patch_idx: int, bank_idx: int):
    # Name: bytes 0..9 (10 ASCII chars)
    name = ''.join(chr(b) if 32 <= b <= 126 else ' ' for b in payload[:10]).strip()
    if not name:
        name = f"PATCH_{bank_idx}_{patch_idx+1:02d}"

    # Harmonics S1 & S2 (63 bytes each) at 10..72 and 73..135
    s1_harmonics = [float(payload[10 + k] & 0x7F) for k in range(63)]
    s2_harmonics = [float(payload[73 + k] & 0x7F) for k in range(63)]

    # DHG Envelope assignments (packed 2 bits per harmonic) at 136..167
    s1_assign = []
    s2_assign = []
    for k in range(63):
        byte_idx1 = 136 + (k // 4)
        shift = (k % 4) * 2
        s1_assign.append((payload[byte_idx1] >> shift) & 0x03)
        byte_idx2 = 152 + (k // 4)
        s2_assign.append((payload[byte_idx2] >> shift) & 0x03)
    harmonic_env_assign = s1_assign + s2_assign

    # DHG Envelopes 1..4 (each has 6 rates and 6 levels = 12 bytes each) starting at 168
    dhg_envs = []
    for e in range(4):
        base = 168 + e * 12
        stages = []
        for s in range(6):
            r = float(payload[base + s] & 0x7F)
            l = float(payload[base + 6 + s] & 0x7F)
            stages.append({"rate": min(r, 99.0), "level": min(l, 99.0)})
        dhg_envs.append({"stages": stages, "loop34": (payload[216 + e] != 0 if 216 + e < len(payload) else False)})

    # Authentic DDA Amplitude Envelope Ingestion (Bytes 220..231)
    dda_r = [payload[220 + s] & 0x7F for s in range(6)]
    dda_l = [payload[226 + s] & 0x7F for s in range(6)]
    r1 = dda_r[0]
    r3 = dda_r[2]
    r6 = dda_r[5]
    l3 = dda_l[2]
    l4 = dda_l[3]

    amp_att = max(0.003, min(5.0, 10.0 * (0.0001 ** (r1 / 99.0)))) if r1 > 0 else 0.005
    amp_dec = max(0.02, min(10.0, 15.0 * (0.0001 ** (r3 / 99.0)))) if r3 > 0 else 0.45
    amp_sus = max(0.20, min(1.0, max(l3, l4) / 99.0))
    amp_rel = max(0.02, min(12.0, 15.0 * (0.0001 ** (r6 / 99.0)))) if r6 > 0 else 0.35

    # Authentic DDF Cutoff & Slope Ingestion (Bytes 232..245)
    ddf_l = [payload[238 + s] & 0x7F for s in range(6)]
    cutoff_raw = payload[244] & 0x7F if 244 < len(payload) else 99
    reso_raw = payload[245] & 0x7F if 245 < len(payload) else 10

    # Musical K5 cutoff scaling (400 Hz .. 20000 Hz)
    filter_cutoff = min(20000.0, max(400.0, 400.0 * (50.0 ** (cutoff_raw / 99.0)))) if cutoff_raw > 0 else 18000.0
    filter_reso = min(0.85, reso_raw / 99.0) if reso_raw > 0 else 0.10

    env_diff = (ddf_l[1] - ddf_l[3]) / 99.0
    filter_env_depth = max(-1.0, min(1.0, env_diff * 1.2)) if abs(env_diff) > 0.05 else 0.0

    # Source 1 / Source 2 Balance
    source_morph = 0.0
    if 246 < len(payload):
        bal = payload[246] & 0x7F
        if bal <= 99:
            source_morph = bal / 99.0

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
            "filter_cutoff": round(filter_cutoff, 1),
            "filter_resonance": round(filter_reso, 3),
            "spectral_tilt": 0.0,
            "source_morph": round(source_morph, 3),
            "odd_even_bal": 0.0,
            "dissonance_warp": 1.0,
            "post_ddf_cutoff": 20000.0,
            "post_ddf_reso": 0.0,
            "amp_attack": round(amp_att, 4),
            "amp_decay": round(amp_dec, 4),
            "amp_sustain": round(amp_sus, 3),
            "amp_release": round(amp_rel, 4),
            "filter_env_depth": round(filter_env_depth, 3),
            "pitch_punch_depth": 0.0,
            "glide_time": 0.0,
            "chaos_amount": 0.04,
            "alpha_chorus": 0,
            "waterbed_depth": 0.0,
            "swarm_coupling": 0.0,
            "fold_wall": 24000.0,
            "mixer_seethe": 0.0,
            "master_gain_db": 0.0,
            "mpe_slide_depth": 0.0,
            "mpe_pressure_depth": 0.0,
            "osc_slop": 0.03,
            "air_absorption": 0.1,
            "vel_brightness": 0.35,
            "modal_release": 0.15,
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
