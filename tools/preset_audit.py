#!/usr/bin/env python3
"""
TKPL — The Klang Preset Linter CLI (Flight 6 / Topic 6.16)
Validates, audits, and fixes preset JSON files against Pydantic models (PresetPayload)
and APVTS master control contracts (master_control_schema.json).
"""

import argparse
import glob
import json
import os
import re
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

# Ensure stdout handles UTF-8 on Windows terminals
if hasattr(sys.stdout, "reconfigure"):
    try:
        sys.stdout.reconfigure(encoding="utf-8")
    except Exception:
        pass

# Import Pydantic models from tools.models
try:
    from tools.models import PresetMetadata, PresetPayload
except ImportError:
    # Fallback if executed from within tools/
    try:
        from models import PresetMetadata, PresetPayload
    except ImportError:
        from pydantic import BaseModel, ConfigDict, Field, field_validator

        class PresetMetadata(BaseModel):
            model_config = ConfigDict(extra="allow")
            name: str = Field(min_length=1, max_length=64, description="Human-readable name of the preset")
            author: str = Field(default="Cody Gratner", description="Creator or author of the preset")
            category: str = Field(description="Sonic classification")
            tags: List[str] = Field(default_factory=list, description="Searchable descriptive tags")
            theme_hint: Optional[str] = Field(default=None, description="Optional UI theme identifier")

        class PresetPayload(BaseModel):
            model_config = ConfigDict(extra="allow")
            schema_version: str = Field(default="1.0.0", description="SemVer version")
            target_plugin: str = Field(pattern="^(TKF|TKP|TKM|TKR)$", description="Target plugin identifier")
            metadata: PresetMetadata = Field(description="Preset metadata block")
            parameters: Dict[str, float] = Field(description="Normalized APVTS parameter float values [0.0, 1.0]")

            @field_validator("parameters")
            @classmethod
            def validate_parameter_ranges(cls, v: Dict[str, float]) -> Dict[str, float]:
                for param_id, val in v.items():
                    if not isinstance(val, (int, float)):
                        raise ValueError(f"Parameter '{param_id}' value must be a number, got {type(val).__name__}")
                    if val < 0.0 or val > 1.0:
                        raise ValueError(f"Parameter '{param_id}' value {val} is outside normalized range [0.0, 1.0]")
                return v


DEFAULT_CONTROLS_DIR = Path("assets/controls")
DEFAULT_PRESETS_DIR = Path("assets/presets")
DEFAULT_SCHEMA_FILE = Path("assets/presets/$schema/preset.schema.json")
DEFAULT_MASTER_SCHEMA = Path("assets/controls/master_control_schema.json")


class LintResult:
    def __init__(self, file_path: Path):
        self.file_path = file_path
        self.preset_name: str = ""
        self.target_plugin: str = ""
        self.param_count: int = 0
        self.errors: List[str] = []
        self.warnings: List[str] = []
        self.passed: bool = False
        self.metadata_fixed: bool = False


def compile_master_control_schema(controls_dir: Path = DEFAULT_CONTROLS_DIR) -> Dict[str, Any]:
    """
    Compiles all individual module schemas in assets/controls/*.json into an authoritative
    master parameter schema dictionary.
    """
    if not controls_dir.exists():
        raise FileNotFoundError(f"Controls directory not found: {controls_dir}")

    all_params: Dict[str, Any] = {}
    modules_summary: Dict[str, int] = {}

    json_files = sorted(controls_dir.glob("*.json"))
    for file_path in json_files:
        if file_path.name == "master_control_schema.json":
            continue

        mod_name = file_path.stem
        try:
            with open(file_path, "r", encoding="utf-8") as f:
                data = json.load(f)
        except Exception as e:
            print(f"[!] Warning: Failed to parse control file {file_path}: {e}", file=sys.stderr)
            continue

        mod_count = 0
        for key, val in data.items():
            if isinstance(val, dict) and "type" in val:
                param_def = dict(val)
                param_def["source_module"] = mod_name
                all_params[key] = param_def
                mod_count += 1

        modules_summary[mod_name] = mod_count

    master_schema = {
        "$schema": "https://json-schema.org/draft/2020-12/schema",
        "version": "1.0.0",
        "title": "The Klang Suite Master APVTS Parameter Schema",
        "description": "Authoritative unified parameter contracts compiled from assets/controls/*.json",
        "total_parameters": len(all_params),
        "modules": modules_summary,
        "parameters": all_params,
    }

    return master_schema


def get_master_control_schema(
    controls_dir: Path = DEFAULT_CONTROLS_DIR,
    master_schema_path: Path = DEFAULT_MASTER_SCHEMA,
    force_recompile: bool = False,
) -> Dict[str, Any]:
    """
    Loads existing master_control_schema.json or compiles it if missing / requested.
    """
    if not force_recompile and master_schema_path.exists():
        try:
            with open(master_schema_path, "r", encoding="utf-8") as f:
                return json.load(f)
        except Exception as e:
            print(f"[!] Warning reading master schema: {e}. Recompiling...", file=sys.stderr)

    # Compile and save
    master_schema = compile_master_control_schema(controls_dir)
    try:
        master_schema_path.parent.mkdir(parents=True, exist_ok=True)
        with open(master_schema_path, "w", encoding="utf-8") as f:
            json.dump(master_schema, f, indent=2)
            f.write("\n")
    except Exception as e:
        print(f"[!] Warning saving master schema: {e}", file=sys.stderr)

    return master_schema


def export_schemas(
    presets_schema_path: Path = DEFAULT_SCHEMA_FILE,
    master_schema_path: Path = DEFAULT_MASTER_SCHEMA,
    controls_dir: Path = DEFAULT_CONTROLS_DIR,
) -> Tuple[Path, Path]:
    """
    Exports both the Pydantic Preset JSON Schema ($schema/preset.schema.json)
    and the compiled Master Parameter Schema (assets/controls/master_control_schema.json).
    """
    # 1. Export Preset JSON Schema via Pydantic model_json_schema()
    preset_schema = PresetPayload.model_json_schema()
    preset_schema["title"] = "The Klang Suite Preset Schema (TKPL)"
    preset_schema["description"] = "JSON schema contract for The Klang Suite synthesizer and audio effect presets."

    presets_schema_path.parent.mkdir(parents=True, exist_ok=True)
    with open(presets_schema_path, "w", encoding="utf-8") as f:
        json.dump(preset_schema, f, indent=2)
        f.write("\n")

    # 2. Compile and export Master Parameter Schema
    master_schema = compile_master_control_schema(controls_dir)
    master_schema_path.parent.mkdir(parents=True, exist_ok=True)
    with open(master_schema_path, "w", encoding="utf-8") as f:
        json.dump(master_schema, f, indent=2)
        f.write("\n")

    return presets_schema_path, master_schema_path


def fix_preset_file(file_path: Path) -> bool:
    """
    Normalizes metadata and standardizes JSON structure in-place.
    Returns True if modifications were made.
    """
    try:
        with open(file_path, "r", encoding="utf-8") as f:
            raw = json.load(f)
    except Exception:
        return False

    modified = False

    # Ensure schema_version
    if "schema_version" not in raw or not raw["schema_version"]:
        raw["schema_version"] = "1.0.0"
        modified = True

    # Ensure metadata exists
    if "metadata" not in raw or not isinstance(raw["metadata"], dict):
        raw["metadata"] = {}
        modified = True

    meta = raw["metadata"]
    if "name" not in meta or not meta["name"]:
        meta["name"] = file_path.stem.replace("_", " ").title()
        modified = True
    if "author" not in meta or not meta["author"]:
        meta["author"] = "Cody Gratner"
        modified = True
    if "category" not in meta or not meta["category"]:
        meta["category"] = "General"
        modified = True
    if "tags" not in meta or not isinstance(meta["tags"], list):
        meta["tags"] = []
        modified = True
    if "theme_hint" not in meta:
        meta["theme_hint"] = None
        modified = True

    # Ensure target_plugin
    if "target_plugin" not in raw or not raw["target_plugin"]:
        raw["target_plugin"] = "TKF"
        modified = True

    # Ensure parameters dict
    if "parameters" not in raw or not isinstance(raw["parameters"], dict):
        raw["parameters"] = {}
        modified = True

    # Standardize key order
    ordered: Dict[str, Any] = {
        "schema_version": raw["schema_version"],
        "target_plugin": raw["target_plugin"],
        "metadata": {
            "name": meta["name"],
            "author": meta["author"],
            "category": meta["category"],
            "tags": meta["tags"],
            "theme_hint": meta.get("theme_hint"),
        },
        "parameters": raw["parameters"],
    }

    with open(file_path, "w", encoding="utf-8") as f:
        json.dump(ordered, f, indent=2)
        f.write("\n")

    return modified


def lint_single_preset(
    file_path: Path,
    valid_param_ids: set,
    fix_metadata: bool = False,
) -> LintResult:
    """
    Audits a single preset JSON file against Pydantic schema and valid APVTS parameter IDs.
    """
    res = LintResult(file_path)

    if not file_path.exists():
        res.errors.append(f"File not found: {file_path}")
        return res

    # Auto-fix metadata if requested
    if fix_metadata:
        if fix_preset_file(file_path):
            res.metadata_fixed = True

    try:
        with open(file_path, "r", encoding="utf-8") as f:
            raw_data = json.load(f)
    except json.JSONDecodeError as jde:
        res.errors.append(f"JSON Syntax Error: {jde}")
        return res
    except Exception as ex:
        res.errors.append(f"File Read Error: {ex}")
        return res

    if not isinstance(raw_data, dict):
        res.errors.append(f"Root JSON is not an object (got {type(raw_data).__name__})")
        return res

    # 1. Pydantic Model Validation
    try:
        payload = PresetPayload.model_validate(raw_data)
        res.preset_name = payload.metadata.name
        res.target_plugin = payload.target_plugin
        res.param_count = len(payload.parameters)
    except Exception as val_err:
        res.errors.append(f"Pydantic Validation Error: {val_err}")
        return res

    # 2. Master Parameter Contract Audit
    for param_id, val in payload.parameters.items():
        if valid_param_ids and param_id not in valid_param_ids:
            res.errors.append(f"Unknown parameter ID '{param_id}' (not in master_control_schema.json)")
        if not (0.0 <= val <= 1.0):
            res.errors.append(f"Parameter '{param_id}' value {val} out of normalized range [0.0, 1.0]")

    # 3. Soft Warnings / Quality Checks
    if not payload.metadata.tags:
        res.warnings.append("Preset has empty tags list")
    if res.param_count == 0:
        res.warnings.append("Preset contains 0 parameters")
    if not payload.metadata.category:
        res.warnings.append("Missing category classification")

    res.passed = len(res.errors) == 0
    return res


def run_lint_pipeline(
    target_path: Path,
    controls_dir: Path = DEFAULT_CONTROLS_DIR,
    fix_metadata: bool = False,
    chat_mode: bool = False,
    quiet: bool = False,
) -> Tuple[int, List[LintResult]]:
    """
    Discovers and audits all preset files in target_path.
    Returns (exit_code, results).
    """
    master_schema = get_master_control_schema(controls_dir)
    params_dict = master_schema.get("parameters", {})
    valid_param_ids = set(params_dict.keys())

    # Find preset files
    files_to_check: List[Path] = []
    if target_path.is_file():
        files_to_check = [target_path]
    elif target_path.is_dir():
        for p in sorted(target_path.rglob("*.json")):
            # Ignore schema definitions
            if "$schema" in p.parts or p.name.endswith(".schema.json"):
                continue
            files_to_check.append(p)
    else:
        print(f"[!] Target path does not exist: {target_path}", file=sys.stderr)
        return 1, []

    if not files_to_check:
        if not quiet:
            print(f"[i] No preset JSON files found in {target_path}")
        return 0, []

    results: List[LintResult] = []
    for f in files_to_check:
        res = lint_single_preset(f, valid_param_ids, fix_metadata=fix_metadata)
        results.append(res)

    total_files = len(results)
    passed_files = sum(1 for r in results if r.passed)
    failed_files = total_files - passed_files
    fixed_count = sum(1 for r in results if r.metadata_fixed)

    # Output Presentation
    if chat_mode:
        print("### 🎛️ TKPL — Preset Linter Report")
        print(f"**Target:** `{target_path}` | **Total:** {total_files} | **Passed:** {passed_files} | **Failed:** {failed_files}")
        if fixed_count > 0:
            print(f"**Auto-Fixed Metadata:** {fixed_count} files")
        print("")
        print("| Status | Preset Name | Plugin | Params | File | Issues |")
        print("| :---: | :--- | :---: | :---: | :--- | :--- |")
        for r in results:
            status_icon = "🟢 PASS" if r.passed else "🔴 FAIL"
            name = r.preset_name or r.file_path.stem
            issues = "; ".join(r.errors + [f"⚠️ {w}" for w in r.warnings]) if (r.errors or r.warnings) else "Clean"
            rel_file = str(r.file_path.as_posix())
            print(f"| {status_icon} | **{name}** | `{r.target_plugin}` | {r.param_count} | `{rel_file}` | {issues} |")
    else:
        print("=" * 80)
        print("🎛️  TKPL — THE KLANG PRESET LINTER CLI")
        print("=" * 80)
        print(f"Target Directory: {target_path}")
        print(f"Master APVTS Schema: {len(valid_param_ids)} recognized parameter contracts")
        print("-" * 80)

        for r in results:
            status_badge = " [✓ PASS] " if r.passed else " [✗ FAIL] "
            fix_badge = " (Fixed)" if r.metadata_fixed else ""
            print(f"{status_badge} {r.file_path.name}{fix_badge}")
            print(f"         Name: {r.preset_name or '(None)'} | Plugin: {r.target_plugin or 'Unknown'} | Parameters: {r.param_count}")
            for err in r.errors:
                print(f"         🔴 ERROR: {err}")
            for warn in r.warnings:
                print(f"         ⚠️  WARN:  {warn}")

        print("-" * 80)
        print(f"Summary: {passed_files}/{total_files} passed. {failed_files} failed. {fixed_count} metadata repaired.")
        print("=" * 80)

    exit_code = 0 if failed_files == 0 else 1
    return exit_code, results


def create_sample_preset(
    name: str,
    target_plugin: str = "TKF",
    out_dir: Path = DEFAULT_PRESETS_DIR,
    controls_dir: Path = DEFAULT_CONTROLS_DIR,
) -> Path:
    """
    Creates a compliant sample preset file for testing and templating.
    """
    master_schema = get_master_control_schema(controls_dir)
    params_dict = master_schema.get("parameters", {})

    # Select representative parameters based on plugin
    chosen_params: Dict[str, float] = {}
    if target_plugin == "TKF":
        # Farmer default parameters
        for pid in ["carrier1_pitch", "carrier1_shape", "filter1_cutoff", "filter1_resonance", "pitchenv1_decay"]:
            if pid in params_dict:
                default_val = float(params_dict[pid].get("default", 0.5))
                chosen_params[pid] = default_val
    elif target_plugin == "TKP":
        # Planter default parameters
        for pid in ["planter_filter_cutoff", "planter_pitchenv_decay", "planter_noise_filter", "planter_mod_speed"]:
            if pid in params_dict:
                default_val = float(params_dict[pid].get("default", 0.5))
                chosen_params[pid] = default_val
    else:
        # Generic first 4 params
        for pid in list(params_dict.keys())[:4]:
            chosen_params[pid] = float(params_dict[pid].get("default", 0.5))

    preset = PresetPayload(
        schema_version="1.0.0",
        target_plugin=target_plugin,
        metadata=PresetMetadata(
            name=name,
            author="Cody Gratner",
            category="Bass" if target_plugin == "TKF" else "Kick",
            tags=["Factory", "Default", target_plugin.lower()],
            theme_hint="dark_terminal",
        ),
        parameters=chosen_params,
    )

    out_dir.mkdir(parents=True, exist_ok=True)
    slug = re.sub(r"[^a-zA-Z0-9_-]", "_", name.lower()).strip("_")
    out_file = out_dir / f"{slug}.json"

    with open(out_file, "w", encoding="utf-8") as f:
        json.dump(preset.model_dump(), f, indent=2)
        f.write("\n")

    return out_file


def main() -> int:
    parser = argparse.ArgumentParser(
        prog="preset_audit.py",
        description="TKPL — The Klang Preset Linter CLI. Audits, verifies, and exports preset schemas.",
    )
    parser.add_argument(
        "--lint",
        action="store_true",
        help="Run lint audit on preset JSON files (default if no mode specified)",
    )
    parser.add_argument(
        "--export-schema",
        action="store_true",
        help="Export Pydantic JSON schema to assets/presets/$schema/ and compile master_control_schema.json",
    )
    parser.add_argument(
        "--fix-metadata",
        action="store_true",
        help="Automatically fix missing metadata and normalize structure in-place",
    )
    parser.add_argument(
        "--dir",
        type=str,
        default="assets/presets",
        help="Path to presets directory (default: assets/presets)",
    )
    parser.add_argument(
        "--file",
        type=str,
        default=None,
        help="Audit a single specific preset file",
    )
    parser.add_argument(
        "--create-sample",
        type=str,
        metavar="PRESET_NAME",
        help="Generate a valid sample preset with master APVTS parameters",
    )
    parser.add_argument(
        "--plugin",
        type=str,
        default="TKF",
        choices=["TKF", "TKP", "TKM", "TKR"],
        help="Target plugin for --create-sample (default: TKF)",
    )
    parser.add_argument(
        "--chat",
        action="store_true",
        help="Output results as clean Markdown table for AI chat or PR summaries",
    )
    parser.add_argument(
        "--quiet",
        action="store_true",
        help="Minimal output (only report summary and errors)",
    )

    args = parser.parse_args()

    # Mode 1: Export Schema
    if args.export_schema:
        ps, ms = export_schemas()
        print(f"[✓] Exported Preset JSON Schema: {ps}")
        print(f"[✓] Compiled Master APVTS Schema: {ms}")
        return 0

    # Mode 2: Create Sample Preset
    if args.create_sample:
        out_path = create_sample_preset(args.create_sample, target_plugin=args.plugin)
        print(f"[✓] Created compliant sample preset: {out_path}")
        return 0

    # Mode 3: Lint Presets (Default)
    target = Path(args.file) if args.file else Path(args.dir)
    exit_code, _ = run_lint_pipeline(
        target_path=target,
        fix_metadata=args.fix_metadata,
        chat_mode=args.chat,
        quiet=args.quiet,
    )
    return exit_code


if __name__ == "__main__":
    sys.exit(main())
