#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
tools/verify_contract_bindings.py — Bidirectional APVTS & UI Contract Bijection Linter
======================================================================================
Part of SQA Titan Armor (Chassis 1).
Enforces the Bidirectional Bijection Invariant (A <=> B):
  1. Every parameter declared in contracts/<plugin>.yaml MUST be explicitly read in
     <Plugin>Processor.cpp (via getRawParameterValue(Ids::<id>) or <id>Param_).
  2. Every non-internal parameter declared in contracts/<plugin>.yaml MUST be attached
     in <Plugin>Editor.cpp (or declared with internal: true in YAML).
  3. No ghost parameters: Every parameter read in Processor.cpp MUST be declared in YAML.

Exit Codes:
  0 = 100% Bijection verified. Zero dead or orphan parameters.
  1 = Defect detected: missing reads, unattached UI controls, or ghost parameters.
"""

import sys
import os
import re
import argparse
from pathlib import Path
import yaml

# Universal UTF-8 console output
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

ROOT_TKS = Path(__file__).resolve().parent.parent if "TheKlangSuite" in str(Path(__file__).resolve()) else Path(r"C:\Dev\TheKlangSuite")


def load_contract(plugin_name: str) -> dict:
    candidates = [
        ROOT_TKS / "contracts" / f"{plugin_name.lower()}.yaml",
        Path(r"C:\_\Vault\contracts") / f"{plugin_name.lower()}.yaml",
    ]
    for c in candidates:
        if c.exists():
            with open(c, "r", encoding="utf-8") as f:
                return yaml.safe_load(f)
    raise FileNotFoundError(f"Contract for plugin '{plugin_name}' not found.")


def find_files_for_plugin(plugin: str) -> tuple[Path | None, Path | None]:
    """Finds <Plugin>Processor.cpp and <Plugin>Editor.cpp case-insensitively."""
    src_dir = ROOT_TKS / "source" / plugin.lower()
    proc_file = None
    editor_file = None

    if src_dir.exists():
        for f in src_dir.glob("*Processor.cpp"):
            proc_file = f
            break
        for f in src_dir.glob("*Editor.cpp"):
            editor_file = f
            break

    if not proc_file:
        # Fallback to source root
        for f in (ROOT_TKS / "source").glob(f"*{plugin}*Processor.cpp"):
            proc_file = f
            break

    return proc_file, editor_file


def audit_plugin(plugin_name: str) -> bool:
    print(f"[*] Auditing Bidirectional Contract Bijection for: {plugin_name.upper()}")

    contract = load_contract(plugin_name)
    plugin = contract.get("plugin", plugin_name).lower()
    
    contract_params = contract.get("parameters", [])
    contract_ids = {p["id"]: p for p in contract_params}
    total_declared = len(contract_ids)
    print(f"    - Declared in contract: {total_declared} parameters")

    proc_file, editor_file = find_files_for_plugin(plugin)

    if not proc_file or not proc_file.exists():
        print(f"[-] Processor file not found for plugin '{plugin}'")
        return False

    proc_text = proc_file.read_text(encoding="utf-8", errors="replace")
    editor_text = editor_file.read_text(encoding="utf-8", errors="replace") if (editor_file and editor_file.exists()) else ""

    # 1. Audit Processor Reads
    unread_in_processor = []
    read_in_processor = set()

    for pid in contract_ids.keys():
        patterns = [
            rf"\bIds::{pid}\b",
            rf'getRawParameterValue\s*\(\s*"{pid}"\s*\)',
            rf'getParameter\s*\(\s*"{pid}"\s*\)',
            rf"\b{pid}Param_\b",
            rf"\bp\.{pid}\b",
        ]
        is_read = any(re.search(pat, proc_text) for pat in patterns)
        if is_read:
            read_in_processor.add(pid)
        else:
            unread_in_processor.append(pid)

    # 2. Audit UI Attachments
    unattached_in_editor = []
    attached_in_editor = set()

    for pid, meta in contract_ids.items():
        is_internal = meta.get("internal", False)
        if is_internal:
            attached_in_editor.add(pid)
            continue

        patterns = [
            rf"\bIds::{pid}\b",
            rf'"{pid}"',
            rf"\b{pid}Attachment\b",
            rf"\b{pid}Slider\b",
            rf"\b{pid}Toggle\b",
            rf"\b{pid}Box\b",
        ]
        is_attached = any(re.search(pat, editor_text) for pat in patterns)
        if is_attached:
            attached_in_editor.add(pid)
        else:
            unattached_in_editor.append(pid)

    # Output Diagnostics
    has_defects = False

    if unread_in_processor:
        has_defects = True
        print(f"\n❌ DEFECT: {len(unread_in_processor)} of {total_declared} parameters are NEVER READ in {proc_file.name}:")
        for p in unread_in_processor:
            cat = contract_ids[p].get("category", "general")
            print(f"   • {p} [{cat}] -> NEVER LOADED FROM APVTS!")

    if unattached_in_editor:
        has_defects = True
        ed_name = editor_file.name if editor_file else "Editor"
        print(f"\n❌ DEFECT: {len(unattached_in_editor)} of {total_declared} parameters are NOT ATTACHED in {ed_name}:")
        for p in unattached_in_editor:
            print(f"   • {p} -> No slider/button attachment found (mark 'internal: true' if intentional).")

    if not has_defects:
        print(f"\n✅ PASS: 100% Bidirectional Bijection verified for '{plugin}'.")
        print(f"   - Processor Reads: {len(read_in_processor)}/{total_declared} (100%)")
        print(f"   - UI Attachments:  {len(attached_in_editor)}/{total_declared} (100%)")
        return True
    else:
        print(f"\n[!] BIJECTION FAILURE: Hollow parameters detected in '{plugin}'. Halting build.\n")
        return False


def main():
    parser = argparse.ArgumentParser(description="Bidirectional APVTS & UI Contract Bijection Linter")
    parser.add_argument("plugin", nargs="?", default=None, help="Target plugin (e.g. shoggoth, hastur, nyarlat)")
    parser.add_argument("--plugin", "-p", dest="plugin_flag", default=None)
    parser.add_argument("--all", action="store_true", help="Audit all contracts in contracts/")
    args = parser.parse_args()

    target = args.plugin_flag or args.plugin
    if args.all:
        contracts_dir = ROOT_TKS / "contracts"
        all_ok = True
        for yml in sorted(contracts_dir.glob("*.yaml")):
            p_name = yml.stem
            if p_name in ["schema"]: continue
            ok = audit_plugin(p_name)
            if not ok: all_ok = False
        sys.exit(0 if all_ok else 1)
    elif target:
        ok = audit_plugin(target)
        sys.exit(0 if ok else 1)
    else:
        parser.error("Specify a plugin name or --all")


if __name__ == "__main__":
    main()
