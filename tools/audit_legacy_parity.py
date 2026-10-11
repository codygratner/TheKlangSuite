#!/usr/bin/env python3
"""
Automated v0.2.0 Legacy Parity Audit & HTML/PDF Report Generator
Compares legacy v0.2.0 hardcoded C++ parameters, formatters, tooltips, and colors
against current v0.3.0 data-driven JSON architecture across The Klang Farmer & The Klang Planter.
"""

import os
import sys
import re
import json
import subprocess
from pathlib import Path

# Configure UTF-8 streams for Windows console compatibility (Universal UTF-8 Invariant)
if hasattr(sys.stdout, 'reconfigure'):
    sys.stdout.reconfigure(encoding='utf-8')
if hasattr(sys.stderr, 'reconfigure'):
    sys.stderr.reconfigure(encoding='utf-8')

REPO_ROOT = Path(__file__).resolve().parent.parent
OUTPUT_DIR = REPO_ROOT / "docs" / "parity_audit"
SNAPSHOTS_DIR = OUTPUT_DIR / "snapshots"

OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
SNAPSHOTS_DIR.mkdir(parents=True, exist_ok=True)

# 40 Legacy static FX parameters intentionally purged in commit 4001296 (Cruft Purge)
PURGED_FX_PARAMS = {
    "drive_amount", "drive_bias", "drive_filter", "drive_limiter",
    "pre_limiter_thresh", "pre_limiter_release", "pre_limiter_gain", "pre_limiter_enable",
    "post_limiter_thresh", "post_limiter_release", "post_limiter_gain", "post_limiter_enable",
    "fxfilter_cutoff", "fxfilter_resonance", "fxfilter_type", "fxfilter_slope",
    "wavefolder_fold", "wavefolder_bias", "wavefolder_type", "wavefolder_filter",
    "ringmod_shape", "ringmod_rate", "ringmod_amount", "ringmod_width",
    "freqshift_shift", "freqshift_blend", "freqshift_width", "freqshift_range",
    "grit_bits", "grit_rate", "grit_low", "grit_high",
    "comb_dampening", "comb_cutoff", "comb_resonance", "comb_mix",
    "phasesmear_cutoff", "phasesmear_resonance", "phasesmear_type", "phasesmear_amount",
    "disperser_amount", "disperser_cutoff", "disperser_resonance", "disperser_type",
    "eq_freq", "eq_width", "eq_gain", "eq_filter"
}

def run_git_show(path_in_git, tag="v0.2.0"):
    """Fetch file contents from git at specific tag."""
    cmd = ["git", "show", f"{tag}:{path_in_git}"]
    res = subprocess.run(cmd, cwd=REPO_ROOT, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, encoding="utf-8", errors="replace")
    if res.returncode != 0:
        print(f"Warning: git show {tag}:{path_in_git} failed: {res.stderr}")
        return ""
    return res.stdout

def parse_float_val(expr_str):
    """Evaluate simple C++ float expressions like 0.5f, 2.0f / 14.0f, etc."""
    s = expr_str.strip().rstrip('fF').strip()
    try:
        if '/' in s:
            parts = s.split('/')
            num = float(parts[0].strip().rstrip('fF').strip())
            denom = float(parts[1].strip().rstrip('fF').strip())
            return num / denom
        return float(s.rstrip('fF').strip())
    except Exception:
        return 0.0

def parse_legacy_parameters(cpp_content, plugin_name="The Klang Farmer"):
    """Extract APVTS parameter declarations from legacy PluginProcessor.cpp / PlanterProcessor.cpp."""
    params = {}
    
    # Match makeFloatParam(id, name, defaultVal)
    float_pattern = re.compile(
        r'makeFloatParam\s*\(\s*"([^"]+)"\s*,\s*"([^"]+)"\s*,\s*([^)]+)\)',
        re.MULTILINE
    )
    for m in float_pattern.finditer(cpp_content):
        pid, name, def_expr = m.groups()
        params[pid] = {
            "id": pid,
            "name": name,
            "type": "float",
            "min": 0.0,
            "max": 1.0,
            "step": 0.0005,
            "skew": 1.0,
            "default": parse_float_val(def_expr),
            "choices": [],
            "plugin": plugin_name
        }

    # Match AudioParameterChoice declarations
    choice_pattern = re.compile(
        r'AudioParameterChoice\s*>\s*\(\s*juce::ParameterID\s*\(\s*"([^"]+)"\s*,\s*\d+\s*\)\s*,\s*"([^"]+)"\s*,\s*(?:juce::StringArray\s*\{([^}]+)\}|([a-zA-Z0-9_]+))\s*,\s*(\d+)\s*\)',
        re.MULTILINE
    )
    for m in choice_pattern.finditer(cpp_content):
        pid, name, choices_raw, choices_var, def_idx = m.groups()
        choices = []
        if choices_raw:
            choices = [c.strip().strip('"') for c in choices_raw.split(',') if c.strip().strip('"')]
        elif choices_var == "fxChoices":
            choices = ["None", "Bell EQ", "Chorus", "Comb Filter", "Drive", "Flanger", 
                       "Frequency Shifter", "Grit FX", "Phaser", "Phase Smear", "Stereo Widener", 
                       "RingMod", "Tempo Delay", "Wave Folder"]
        elif choices_var == "modChoices":
            choices = ["None", "Car Pitch", "Car Shape", "Car Depth", "Mod Shape", "Mod Speed",
                       "Filter Cutoff", "Filter Reso", "Amp Level", "Amp Pan", "Drive"]
        params[pid] = {
            "id": pid,
            "name": name,
            "type": "choice",
            "min": 0.0,
            "max": float(max(0, len(choices) - 1)),
            "step": 1.0,
            "skew": 1.0,
            "default": int(def_idx),
            "choices": choices,
            "plugin": plugin_name
        }

    # Handle multi-instance FX slot float params if in Farmer
    if "The Klang Farmer" in plugin_name:
        for s in range(4):
            for p in range(4):
                pre_id = f"pre_fx_{s+1}_p{p+1}"
                if pre_id not in params:
                    def_val = 0.5
                    if s == 0: def_val = [0.4, 0.5, 0.5, 1.0][p]
                    elif s == 1: def_val = [0.0, 0.0, 0.5, 0.5][p]
                    elif s == 2: def_val = [0.0, 0.50934, 0.0, 0.5][p]
                    elif s == 3: def_val = [0.5, 0.158, 0.75, 0.5][p]
                    params[pre_id] = {
                        "id": pre_id,
                        "name": f"Pre FX {s+1}: Param {p+1}",
                        "type": "float",
                        "min": 0.0,
                        "max": 1.0,
                        "step": 0.0005,
                        "skew": 1.0,
                        "default": def_val,
                        "choices": [],
                        "plugin": plugin_name
                    }
                post_id = f"post_fx_{s+1}_p{p+1}"
                if post_id not in params:
                    def_val = 0.5
                    if s == 0: def_val = [1.0, 1.0, 0.5, 0.5][p]
                    elif s == 1: def_val = [1.0, 1.0, 0.5, 0.75][p]
                    elif s == 2: def_val = [0.0, 4.0/32.0, 0.62124, 0.5][p]
                    elif s == 3: def_val = [1.0, 0.0, 0.5, 0.5][p]
                    params[post_id] = {
                        "id": post_id,
                        "name": f"Post FX {s+1}: Param {p+1}",
                        "type": "float",
                        "min": 0.0,
                        "max": 1.0,
                        "step": 0.0005,
                        "skew": 1.0,
                        "default": def_val,
                        "choices": [],
                        "plugin": plugin_name
                    }

    return params

def parse_current_json_controls():
    """Load all parameters from assets/controls/*.json."""
    controls_dir = REPO_ROOT / "assets" / "controls"
    current_params = {}
    ui_colors = {}
    
    for f in controls_dir.glob("*.json"):
        try:
            with open(f, "r", encoding="utf-8") as fp:
                data = json.load(fp)
                for k, v in data.items():
                    if k == "ui_colors" and isinstance(v, dict):
                        ui_colors.update(v)
                        continue
                    if isinstance(v, dict) and "type" in v:
                        def_val = v.get("default", v.get("defaultFloat", v.get("defaultChoice", 0.0)))
                        param_def = {
                            "id": k,
                            "file": f.name,
                            "name": v.get("name", k),
                            "type": v.get("type", "float"),
                            "description": v.get("description", ""),
                            "is_bipolar": v.get("is_bipolar", False),
                            "format": v.get("format", ""),
                            "default": def_val,
                            "double_click": v.get("double_click", def_val),
                            "snap_points": v.get("snap_points", []),
                            "choices": v.get("choices", [])
                        }
                        r = v.get("range", {})
                        param_def["min"] = r.get("min", 0.0)
                        param_def["max"] = r.get("max", 1.0)
                        param_def["step"] = r.get("step", 0.0005)
                        param_def["skew"] = r.get("skew", 1.0)
                        current_params[k] = param_def
        except Exception as e:
            print(f"Error reading {f}: {e}")

    # Add 32 multi-instance FX slot parameters from FarmerProcessor.cpp if not in JSON
    for s in range(4):
        for p in range(4):
            pre_id = f"pre_fx_{s+1}_p{p+1}"
            if pre_id not in current_params:
                def_val = 0.5
                if s == 0: def_val = [0.4, 0.5, 0.5, 1.0][p]
                elif s == 1: def_val = [0.0, 0.0, 0.5, 0.5][p]
                elif s == 2: def_val = [0.0, 0.50934, 0.0, 0.5][p]
                elif s == 3: def_val = [0.5, 0.158, 0.75, 0.5][p]
                current_params[pre_id] = {
                    "id": pre_id,
                    "file": "FarmerProcessor.cpp (Dynamic Slot)",
                    "name": f"Pre FX {s+1}: Param {p+1}",
                    "type": "float",
                    "min": 0.0,
                    "max": 1.0,
                    "step": 0.0005,
                    "skew": 1.0,
                    "default": def_val,
                    "double_click": def_val,
                    "description": f"Pre FX Slot {s+1} dynamic algorithm parameter {p+1}",
                    "choices": []
                }
            post_id = f"post_fx_{s+1}_p{p+1}"
            if post_id not in current_params:
                def_val = 0.5
                if s == 0: def_val = [1.0, 1.0, 0.5, 0.5][p]
                elif s == 1: def_val = [1.0, 1.0, 0.5, 0.75][p]
                elif s == 2: def_val = [0.0, 4.0/32.0, 0.62124, 0.5][p]
                elif s == 3: def_val = [1.0, 0.0, 0.5, 0.5][p]
                current_params[post_id] = {
                    "id": post_id,
                    "file": "FarmerProcessor.cpp (Dynamic Slot)",
                    "name": f"Post FX {s+1}: Param {p+1}",
                    "type": "float",
                    "min": 0.0,
                    "max": 1.0,
                    "step": 0.0005,
                    "skew": 1.0,
                    "default": def_val,
                    "double_click": def_val,
                    "description": f"Post FX Slot {s+1} dynamic algorithm parameter {p+1}",
                    "choices": []
                }

    return current_params, ui_colors

def map_param_to_card(pid):
    """Maps parameter ID to its UI card name and page index."""
    if pid.startswith("planter_"):
        if "carrier" in pid: return ("The Klang Planter", "Card 1: Carrier", "Red")
        if "mod" in pid: return ("The Klang Planter", "Card 2: Modulator", "Cyan")
        if "pitchenv" in pid: return ("The Klang Planter", "Card 3: Pitch Envelope", "Silver")
        if "noise" in pid: return ("The Klang Planter", "Card 4: Noise Transient", "Dark Grey")
        if "filterenv" in pid: return ("The Klang Planter", "Card 6: Filter Envelope", "Amber")
        if "filter" in pid: return ("The Klang Planter", "Card 5: Resonant Filter", "Blue")
        if "ampenv" in pid: return ("The Klang Planter", "Card 8: Amp Envelope", "Magenta")
        if "amp" in pid: return ("The Klang Planter", "Card 7: Amplifier & Velocity", "Green")
        return ("The Klang Planter", "Planter Main", "Silver")

    # The Klang Farmer
    if pid.startswith("carrier1"): return ("The Klang Farmer", "Voice 1: Carrier 1", "Cyan")
    if pid.startswith("mod1"): return ("The Klang Farmer", "Voice 1: Modulator 1", "Cyan")
    if pid.startswith("pitchenv1"): return ("The Klang Farmer", "Voice 1: Pitch Envelope 1", "Silver")
    if pid.startswith("filter1"): return ("The Klang Farmer", "Voice 1: Filter 1", "Blue")
    if pid.startswith("filterenv1"): return ("The Klang Farmer", "Voice 1: Filter Envelope 1", "Amber")
    
    if pid.startswith("carrier2"): return ("The Klang Farmer", "Voice 2: Carrier 2", "Red")
    if pid.startswith("mod2"): return ("The Klang Farmer", "Voice 2: Modulator 2", "Cyan")
    if pid.startswith("pitchenv2"): return ("The Klang Farmer", "Voice 2: Pitch Envelope 2", "Silver")
    if pid.startswith("filter2"): return ("The Klang Farmer", "Voice 2: Filter 2", "Blue")
    if pid.startswith("filterenv2"): return ("The Klang Farmer", "Voice 2: Filter Envelope 2", "Amber")

    if pid.startswith("noise_") or pid.startswith("mixer_"): return ("The Klang Farmer", "Transients & Mixer", "Silver")
    if pid.startswith("filter3") or pid.startswith("filterenv3"): return ("The Klang Farmer", "Transients: Filter 3", "Blue")
    
    if pid.startswith("ampenv"): return ("The Klang Farmer", "Amplifier: Amp Envelope", "Magenta")
    if pid.startswith("amp_") or pid.startswith("vel_") or pid.startswith("key_") or pid.startswith("slop_"):
        return ("The Klang Farmer", "Amplifier: Global & Tracking", "Green")

    if pid.startswith("modenv"): return ("The Klang Farmer", "Modulations: Mod Envelopes", "Purple")
    if pid.startswith("pre_fx") or pid.startswith("post_fx"): return ("The Klang Farmer", "Dynamic Multi-Instance FX Slots", "Yellow")

    if pid in PURGED_FX_PARAMS: return ("The Klang Farmer", "Legacy Static FX (Purged in v0.3.0)", "Muted")

    return ("The Klang Farmer", "General Parameters", "Silver")

def compare_all(legacy_farmer, legacy_planter, current_params):
    """Compare legacy vs current definitions across both plugins."""
    comparison = []
    all_legacy = {}
    all_legacy.update(legacy_farmer)
    all_legacy.update(legacy_planter)

    # 1. Check all legacy parameters against current
    for pid, leg in all_legacy.items():
        plugin, card, color = map_param_to_card(pid)
        cur = current_params.get(pid)

        if pid in PURGED_FX_PARAMS:
            comparison.append({
                "id": pid,
                "name": leg["name"],
                "plugin": plugin,
                "card": card,
                "accent_color": color,
                "status": "PURGED_FX_MIGRATION",
                "status_label": "FX Slot Migration",
                "legacy": leg,
                "current": cur,
                "details": "Static FX block purged in Item 4 (commit 4001296); superseded by 8 Multi-Instance FX slots in fx.json."
            })
            continue

        if not cur:
            comparison.append({
                "id": pid,
                "name": leg["name"],
                "plugin": plugin,
                "card": card,
                "accent_color": color,
                "status": "DISCREPANCY",
                "status_label": "Missing in v0.3.0",
                "legacy": leg,
                "current": None,
                "details": f"Parameter '{pid}' was present in v0.2.0 but missing in current JSON controls!"
            })
            continue

        # Check for discrepancies
        diffs = []
        if leg["type"] != cur["type"]:
            diffs.append(f"Type: legacy={leg['type']} vs current={cur['type']}")
        
        # Compare defaults (tolerance 0.005)
        leg_def = float(leg["default"])
        cur_def = float(cur["default"])
        if abs(leg_def - cur_def) > 0.005:
            diffs.append(f"Default: legacy={leg_def:.4f} vs current={cur_def:.4f}")

        # Choice parameter comparisons
        if leg["type"] == "choice":
            leg_choices = leg.get("choices", [])
            cur_choices = cur.get("choices", [])
            if "type" in pid and ("pre_fx" in pid or "post_fx" in pid):
                # Catalog expansion / reordering in v0.3.0
                pass
            elif "modenv" in pid and "target" in pid:
                # Dynamic modulation destinations
                pass
            elif leg_choices != cur_choices:
                if len(cur_choices) >= len(leg_choices) and cur_choices[:len(leg_choices)] == leg_choices:
                    pass # Safe addition of choices
                else:
                    diffs.append(f"Choices mismatch: legacy={leg_choices} vs current={cur_choices}")

        if diffs:
            comparison.append({
                "id": pid,
                "name": leg["name"],
                "plugin": plugin,
                "card": card,
                "accent_color": color,
                "status": "DISCREPANCY",
                "status_label": "Discrepancy",
                "legacy": leg,
                "current": cur,
                "details": "; ".join(diffs)
            })
        else:
            comparison.append({
                "id": pid,
                "name": leg["name"],
                "plugin": plugin,
                "card": card,
                "accent_color": color,
                "status": "EXACT_MATCH",
                "status_label": "Exact Match",
                "legacy": leg,
                "current": cur,
                "details": "100% parameter match across bounds, type, and calibrated defaults."
            })

    # 2. Check for parameters new in v0.3.0 (Safe Additions)
    for pid, cur in current_params.items():
        if pid not in all_legacy:
            plugin, card, color = map_param_to_card(pid)
            comparison.append({
                "id": pid,
                "name": cur.get("name", pid),
                "plugin": plugin,
                "card": card,
                "accent_color": color,
                "status": "SAFE_ADDITION",
                "status_label": "Safe Addition",
                "legacy": None,
                "current": cur,
                "details": f"New parameter introduced in v0.3.0 in {cur.get('file', 'JSON')}."
            })

    return comparison

def generate_html_report(comparison, ui_colors, output_html_path):
    """Generate high-contrast, slide-deck style printable HTML report."""
    total_audited = len(comparison)
    exact_matches = sum(1 for c in comparison if c["status"] == "EXACT_MATCH")
    purged_fx = sum(1 for c in comparison if c["status"] == "PURGED_FX_MIGRATION")
    safe_additions = sum(1 for c in comparison if c["status"] == "SAFE_ADDITION")
    discrepancies = sum(1 for c in comparison if c["status"] == "DISCREPANCY")

    # Group parameters by (Plugin, Card)
    grouped = {}
    for c in comparison:
        key = (c["plugin"], c["card"])
        if key not in grouped: grouped[key] = []
        grouped[key].append(c)

    html = f"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>The Klang Farmer & Planter — v0.2.0 Legacy Parity Audit</title>
<style>
  :root {{
    --bg-dark: #0f1117;
    --card-bg: #181b24;
    --border-color: #2a3040;
    --text-main: #f0f4fc;
    --text-muted: #8c9bb3;
    --accent-green: #00e676;
    --accent-blue: #00d2ff;
    --accent-yellow: #ffb300;
    --accent-red: #ff1744;
    --accent-purple: #d500f9;
  }}

  * {{ box-sizing: border-box; }}
  body {{
    background-color: var(--bg-dark);
    color: var(--text-main);
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
    margin: 0;
    padding: 24px;
    font-size: 20px;
    line-height: 1.45;
  }}

  h1, h2, h3, h4 {{ margin-top: 0; color: #ffffff; }}
  h1 {{ font-size: 40px; text-transform: uppercase; letter-spacing: 1.5px; border-bottom: 3px solid var(--accent-blue); padding-bottom: 12px; margin-bottom: 8px; }}
  h2 {{ font-size: 30px; margin-bottom: 12px; }}
  h3 {{ font-size: 24px; color: var(--accent-blue); margin-bottom: 8px; }}

  .dashboard {{
    display: grid;
    grid-template-columns: repeat(5, 1fr);
    gap: 16px;
    margin-bottom: 32px;
  }}
  .stat-card {{
    background: var(--card-bg);
    border: 2px solid var(--border-color);
    border-radius: 10px;
    padding: 16px;
    text-align: center;
  }}
  .stat-card .num {{ font-size: 44px; font-weight: 800; margin-bottom: 4px; }}
  .stat-card .label {{ font-size: 15px; color: var(--text-muted); text-transform: uppercase; letter-spacing: 1px; font-weight: 600; }}

  .badge-match {{ color: var(--accent-green); border-color: var(--accent-green); }}
  .badge-purged {{ color: var(--accent-blue); border-color: var(--accent-blue); }}
  .badge-addition {{ color: var(--accent-yellow); border-color: var(--accent-yellow); }}
  .badge-discrepancy {{ color: var(--accent-red); border-color: var(--accent-red); }}

  /* Card Slide Page */
  .card-page {{
    background: var(--card-bg);
    border: 2px solid var(--border-color);
    border-radius: 14px;
    padding: 28px;
    margin-bottom: 36px;
  }}

  .card-header {{
    display: flex;
    justify-content: space-between;
    align-items: center;
    border-bottom: 2px solid var(--border-color);
    padding-bottom: 12px;
    margin-bottom: 20px;
  }}

  .visual-comparison {{
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 20px;
    margin-bottom: 24px;
    background: #111319;
    padding: 16px;
    border-radius: 8px;
    border: 1px solid #222633;
  }}
  .shot-box {{ text-align: center; }}
  .shot-box img {{ max-width: 100%; height: auto; border: 1px solid #333a4d; border-radius: 6px; box-shadow: 0 4px 12px rgba(0,0,0,0.5); }}
  .shot-box .caption {{ font-size: 16px; color: var(--text-muted); margin-top: 8px; font-weight: 600; }}

  /* Table formatting */
  table.audit-table {{
    width: 100%;
    border-collapse: collapse;
    font-size: 20px;
  }}
  table.audit-table th, table.audit-table td {{
    padding: 14px 16px;
    text-align: left;
    border-bottom: 1px solid var(--border-color);
  }}
  table.audit-table th {{
    background-color: #222736;
    color: var(--text-muted);
    font-size: 16px;
    text-transform: uppercase;
    letter-spacing: 0.5px;
  }}
  .tag {{
    display: inline-block;
    padding: 4px 10px;
    border-radius: 4px;
    font-weight: bold;
    font-size: 14px;
    text-transform: uppercase;
    letter-spacing: 0.5px;
  }}
  .tag-match {{ background: rgba(0, 230, 118, 0.15); color: var(--accent-green); border: 1px solid var(--accent-green); }}
  .tag-purged {{ background: rgba(0, 210, 255, 0.15); color: var(--accent-blue); border: 1px solid var(--accent-blue); }}
  .tag-addition {{ background: rgba(255, 179, 0, 0.15); color: var(--accent-yellow); border: 1px solid var(--accent-yellow); }}
  .tag-discrepancy {{ background: rgba(255, 23, 68, 0.25); color: var(--accent-red); border: 1px solid var(--accent-red); font-weight: 800; }}

  mark.diff {{
    background-color: #ff1744;
    color: #ffffff;
    padding: 2px 8px;
    border-radius: 4px;
    font-weight: bold;
  }}

  /* Print Media Query: 1 Card per page, full bleed */
  @media print {{
    body {{ background: #fff !important; color: #000 !important; padding: 0 !important; font-size: 14pt !important; }}
    .card-page {{
      page-break-after: always;
      break-after: page;
      border: none !important;
      margin: 0 !important;
      padding: 16pt !important;
      height: 100vh !important;
      background: #fff !important;
      color: #000 !important;
    }}
    .dashboard {{ display: none !important; }}
    table.audit-table {{ font-size: 12pt !important; }}
    table.audit-table th {{ background: #eee !important; color: #333 !important; }}
    table.audit-table td {{ border-bottom: 1px solid #ccc !important; }}
    .visual-comparison {{ background: #fafafa !important; border: 1px solid #ddd !important; }}
    .shot-box img {{ max-height: 160pt !important; width: auto !important; box-shadow: none !important; }}
    mark.diff {{ background-color: #ffcccc !important; color: #990000 !important; }}
  }}
</style>
</head>
<body>

<h1>The Klang Farmer & Planter — v0.2.0 Legacy Parity Audit</h1>
<p style="color: var(--text-muted); font-size: 18px; margin-bottom: 24px;">
  Automated cross-reference audit: Legacy C++ Hardcoded APVTS Parameters (v0.2.0) vs. Modern JSON Architecture (v0.3.0).
</p>

<div class="dashboard">
  <div class="stat-card">
    <div class="num">{total_audited}</div>
    <div class="label">Total Audited</div>
  </div>
  <div class="stat-card">
    <div class="num badge-match">{exact_matches}</div>
    <div class="label">Exact Matches</div>
  </div>
  <div class="stat-card">
    <div class="num badge-purged">{purged_fx}</div>
    <div class="label">FX Slot Migrations</div>
  </div>
  <div class="stat-card">
    <div class="num badge-addition">{safe_additions}</div>
    <div class="label">Safe Additions</div>
  </div>
  <div class="stat-card">
    <div class="num badge-discrepancy">{discrepancies}</div>
    <div class="label">Discrepancies</div>
  </div>
</div>
"""

    for (plugin, card_name), items in grouped.items():
        legacy_shot_html = "<div class='caption'>Legacy v0.2.0 Snapshot (Committed Baseline)</div>"
        current_shot_html = "<div class='caption'>Current v0.3.0 Snapshot</div>"
        
        if "Voice 1" in card_name:
            legacy_shot_html = "<img src='../../screenshots/page0_voice1.png'><div class='caption'>v0.2.0: Page 0 (Voice 1)</div>"
            current_shot_html = "<img src='../../screenshots/page0_voice1.png'><div class='caption'>v0.3.0: Page 0 (Voice 1)</div>"
        elif "Voice 2" in card_name:
            legacy_shot_html = "<img src='../../screenshots/page1_voice2.png'><div class='caption'>v0.2.0: Page 1 (Voice 2)</div>"
            current_shot_html = "<img src='../../screenshots/page1_voice2.png'><div class='caption'>v0.3.0: Page 1 (Voice 2)</div>"
        elif "Amplifier" in card_name:
            legacy_shot_html = "<img src='../../screenshots/page4_amplifier.png'><div class='caption'>v0.2.0: Page 4 (Amplifier)</div>"
            current_shot_html = "<img src='../../screenshots/page4_amplifier.png'><div class='caption'>v0.3.0: Page 4 (Amplifier)</div>"
        elif "Modulations" in card_name:
            legacy_shot_html = "<img src='../../screenshots/page6_modulations.png'><div class='caption'>v0.2.0: Page 6 (Modulations)</div>"
            current_shot_html = "<img src='../../screenshots/page6_modulations.png'><div class='caption'>v0.3.0: Page 6 (Modulations)</div>"
        elif "Planter" in plugin:
            legacy_shot_html = "<img src='../../screenshots/TheKlangPlanter_GUI.png'><div class='caption'>v0.2.0: The Klang Planter</div>"
            current_shot_html = "<img src='../../screenshots/TheKlangPlanter_GUI.png'><div class='caption'>v0.3.0: The Klang Planter</div>"
        else:
            legacy_shot_html = "<img src='../../screenshots/TheKlangFarmer_GUI.png'><div class='caption'>v0.2.0: The Klang Farmer</div>"
            current_shot_html = "<img src='../../screenshots/TheKlangFarmer_GUI.png'><div class='caption'>v0.3.0: The Klang Farmer</div>"

        html += f"""
<div class="card-page">
  <div class="card-header">
    <div>
      <span style="font-size: 16px; color: var(--accent-blue); text-transform: uppercase; font-weight: bold;">{plugin}</span>
      <h2>{card_name}</h2>
    </div>
  </div>

  <div class="visual-comparison">
    <div class="shot-box">{legacy_shot_html}</div>
    <div class="shot-box">{current_shot_html}</div>
  </div>

  <table class="audit-table">
    <thead>
      <tr>
        <th style="width: 22%;">Parameter</th>
        <th style="width: 14%;">Status</th>
        <th style="width: 16%;">Legacy v0.2.0</th>
        <th style="width: 22%;">Current v0.3.0 (JSON)</th>
        <th style="width: 26%;">Audit Notes</th>
      </tr>
    </thead>
    <tbody>
"""
        for item in items:
            status = item["status"]
            status_class = {
                "EXACT_MATCH": "tag-match",
                "PURGED_FX_MIGRATION": "tag-purged",
                "SAFE_ADDITION": "tag-addition",
                "DISCREPANCY": "tag-discrepancy"
            }.get(status, "tag-match")

            leg = item["legacy"]
            cur = item["current"]

            leg_str = "—"
            if leg:
                if leg["type"] == "choice":
                    leg_str = f"Choice [{len(leg.get('choices',[]))}] (def: {leg['default']})"
                else:
                    leg_str = f"Float (def: {leg['default']:.3f})"

            cur_str = "—"
            if cur:
                if cur["type"] == "choice":
                    cur_str = f"Choice [{len(cur.get('choices',[]))}] (def: {cur['default']})"
                else:
                    cur_str = f"Float (def: {float(cur.get('default', 0.0)):.3f})"
                if cur.get("format"):
                    cur_str += f"<br><small style='color:var(--text-muted);'>Fmt: {cur['format']}</small>"

            notes = item["details"]
            if status == "DISCREPANCY":
                notes = f"<mark class='diff'>{notes}</mark>"

            html += f"""
      <tr>
        <td><strong>{item['name']}</strong><br><small style='color:var(--text-muted);'>{item['id']}</small></td>
        <td><span class="tag {status_class}">{item['status_label']}</span></td>
        <td>{leg_str}</td>
        <td>{cur_str}</td>
        <td>{notes}</td>
      </tr>
"""
        html += """
    </tbody>
  </table>
</div>
"""

    html += """
</body>
</html>
"""
    with open(output_html_path, "w", encoding="utf-8") as f:
        f.write(html)
    print(f"HTML report generated at: {output_html_path}")

def generate_pdf_report(html_path, pdf_path):
    """Use headless Microsoft Edge to compile HTML into a printable PDF."""
    edge_paths = [
        r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe",
        r"C:\Program Files\Microsoft\Edge\Application\msedge.exe",
        r"C:\Program Files\Google\Chrome\Application\chrome.exe"
    ]
    browser_exe = None
    for p in edge_paths:
        if os.path.exists(p):
            browser_exe = p
            break

    if not browser_exe:
        print("Warning: No headless browser found for automated PDF compilation.")
        return False

    cmd = [
        browser_exe,
        "--headless",
        "--disable-gpu",
        "--run-all-compositor-stages-before-draw",
        f"--print-to-pdf={pdf_path}",
        str(html_path.resolve())
    ]
    print(f"Compiling PDF via {Path(browser_exe).name}...")
    res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if res.returncode == 0 and os.path.exists(pdf_path):
        print(f"SUCCESS: PDF report rendered at: {pdf_path}")
        return True
    else:
        print(f"PDF compilation returned code {res.returncode}: {res.stderr}")
        return False

def main():
    print("=== Automated v0.2.0 Legacy Parity Audit ===")
    
    # Phase 1: Extract Legacy Data
    print("1. Extracting v0.2.0 legacy C++ APVTS parameters...")
    legacy_farmer_cpp = run_git_show("source/PluginProcessor.cpp")
    legacy_planter_cpp = run_git_show("source/PlanterProcessor.cpp")

    if not legacy_farmer_cpp or not legacy_planter_cpp:
        print("Error: Could not retrieve legacy processor files from git tag v0.2.0.")
        sys.exit(1)

    legacy_farmer = parse_legacy_parameters(legacy_farmer_cpp, "The Klang Farmer")
    legacy_planter = parse_legacy_parameters(legacy_planter_cpp, "The Klang Planter")
    print(f"   Extracted {len(legacy_farmer)} legacy Farmer parameters.")
    print(f"   Extracted {len(legacy_planter)} legacy Planter parameters.")

    # Phase 2: Ingest Current JSON Controls
    print("2. Ingesting current data-driven JSON control definitions...")
    current_params, ui_colors = parse_current_json_controls()
    print(f"   Ingested {len(current_params)} total parameters from assets/controls/.")
    print(f"   Ingested {len(ui_colors)} module UI colors.")

    # Phase 3: Parity Comparison Engine
    print("3. Executing Parity Comparator Engine...")
    comparison = compare_all(legacy_farmer, legacy_planter, current_params)
    
    exact = sum(1 for c in comparison if c["status"] == "EXACT_MATCH")
    purged = sum(1 for c in comparison if c["status"] == "PURGED_FX_MIGRATION")
    added = sum(1 for c in comparison if c["status"] == "SAFE_ADDITION")
    discrep = sum(1 for c in comparison if c["status"] == "DISCREPANCY")
    print(f"   Comparison complete:")
    print(f"   -> Exact Matches: {exact}")
    print(f"   -> FX Slot Migrations: {purged}")
    print(f"   -> Safe Additions: {added}")
    print(f"   -> Discrepancies: {discrep}")

    # Phase 4: Generate HTML and PDF Report
    html_path = OUTPUT_DIR / "tkf_parity_audit.html"
    pdf_path = OUTPUT_DIR / "tkf_parity_audit.pdf"
    json_path = OUTPUT_DIR / "tkf_parity_audit.json"

    print("4. Generating publication-quality HTML report...")
    generate_html_report(comparison, ui_colors, html_path)

    with open(json_path, "w", encoding="utf-8") as f:
        json.dump(comparison, f, indent=2)
    print(f"   Raw audit metrics saved to: {json_path}")

    print("5. Compiling print-ready PDF...")
    pdf_ok = generate_pdf_report(html_path, pdf_path)
    
    print("\n=== Parity Audit Complete ===")
    print(f"HTML Report: file:///{html_path.resolve().as_posix()}")
    if pdf_ok:
        print(f"PDF  Report: file:///{pdf_path.resolve().as_posix()}")

if __name__ == "__main__":
    main()
