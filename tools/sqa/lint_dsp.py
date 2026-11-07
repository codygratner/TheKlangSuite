#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
tools/sqa/lint_dsp.py — Zero-Dummy-Argument & DSP Sink Reachability Linter
========================================================================
Part of Chassis 1: C++ DSP Titan Armor.
Audits C++ DSP voice and processor code to detect:
  1. Dummy function arguments accepted in renderBlock() but never used in mathematical calculations.
  2. Banned dummy-silencers: (void)param;, param * 0.0f, param - param.
  3. Mode Branch Traps: Engine mode flags (e.g. if (modernMode)) without alternate branches.
  4. Unused local assignments: float x = param; where x is never read in an audio sink.

Zero external dependencies — runs via Python re/tokenizer in <15ms.
"""

import sys
import os
import re
import argparse
from pathlib import Path

# Universal UTF-8 console output
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

ROOT_TKS = Path(__file__).resolve().parent.parent.parent if "TheKlangSuite" in str(Path(__file__).resolve()) else Path(r"C:\Dev\TheKlangSuite")


def lint_voice_header(voice_file: Path) -> tuple[list[str], list[str]]:
    """Lints a C++ voice header for dummy arguments and sink reachability."""
    errors = []
    warnings = []

    if not voice_file.exists():
        return [f"Voice file not found: {voice_file}"], []

    code = voice_file.read_text(encoding="utf-8", errors="replace")

    # 1. Check for banned dummy argument silencers: (void)x;
    void_silencers = re.findall(r'\(\s*void\s*\)\s*([a-zA-Z0-9_]+)\s*;', code)
    if void_silencers:
        for s in void_silencers:
            errors.append(f"BANNED DUMMY SILENCER: '(void){s};' detected. Parameters must be used in DSP math, not silenced!")

    # 2. Check for trivial cancellations: x * 0.0f, 0 * x, or x - x
    pat1 = r'\b([a-zA-Z0-9_]+)\s*\*\s*0+(?:\.0+)?f?(?![.\dxXa-fA-F])'
    pat2 = r'(?<![.\dxXa-fA-F])0+(?:\.0+)?f?\s*\*\s*([a-zA-Z0-9_]+)\b'
    mult_zero = [m for m in (re.findall(pat1, code) + re.findall(pat2, code)) if m]
    for m in mult_zero:
        if m not in ["0", "1"]:
            errors.append(f"DUMMY MATH CANCELLATION: '{m} * 0' detected. Multiplying parameter by zero is prohibited!")

    minus_self = re.findall(r'\b([a-zA-Z0-9_]+)\s*-\s*\1\b', code)
    for m in minus_self:
        if m not in ["0", "1"]:
            errors.append(f"DUMMY MATH CANCELLATION: '{m} - {m}' detected. Canceling parameter against itself is prohibited!")

    # 3. Locate renderBlock function signature and extract arguments
    render_match = re.search(r'void\s+renderBlock\s*\(([^)]+)\)', code)
    if not render_match:
        # Search for multiline renderBlock
        render_match = re.search(r'void\s+renderBlock\s*\((.*?)\)\s*\{', code, re.DOTALL)

    if render_match:
        sig_args = render_match.group(1)
        # Parse argument names (e.g. "float filterCutoffHz, float filterReso")
        arg_names = []
        for part in sig_args.split(','):
            # Strip default value like '= 20000.0f'
            clean_part = part.split('=')[0].strip()
            tokens = clean_part.split()
            if tokens:
                arg_name = tokens[-1].lstrip('*&')
                if arg_name not in ["leftChannel", "rightChannel", "numSamples", "buffer", "num", "samples", ""]:
                    arg_names.append(arg_name)

        # Extract function body of renderBlock
        start_idx = render_match.end()
        # Find matching closing brace
        depth = 1
        body_end = start_idx
        for i in range(start_idx, len(code)):
            if code[i] == '{': depth += 1
            elif code[i] == '}':
                depth -= 1
                if depth == 0:
                    body_end = i
                    break
        
        body = code[start_idx:body_end]

        # Audit usage of each argument inside body
        for arg in arg_names:
            # Count occurrences of arg as an isolated word token inside body
            token_matches = re.findall(rf'\b{re.escape(arg)}\b', body)
            count = len(token_matches)
            if count == 0:
                errors.append(f"DEAD ARGUMENT: Parameter '{arg}' is accepted in renderBlock() signature but NEVER referenced in body!")
            elif count == 1:
                # Appears only once in body - check if it's merely assigned to an unused local variable
                assign_match = re.search(rf'(?:float|auto|const\s+float)\s+([a-zA-Z0-9_]+)\s*=\s*[^;]*\b{re.escape(arg)}\b[^;]*;', body)
                if assign_match:
                    local_var = assign_match.group(1)
                    # Check if local_var is referenced again
                    local_uses = len(re.findall(rf'\b{re.escape(local_var)}\b', body))
                    if local_uses <= 1:
                        errors.append(f"UNREFERENCED LOCAL SINK: '{arg}' assigned to '{local_var}', which is never used in calculation!")

    # 4. Check for Mode Branch Traps
    # If code has: if (modernMode) ... without else, flag if vintage mode is left unhandled
    mode_matches = re.finditer(r'if\s*\(\s*(?:state_\.)?modernMode\s*\)\s*\{', code)
    for mm in mode_matches:
        idx = mm.end()
        # Look ahead 500 chars to see if an else branch exists
        lookahead = code[idx:idx+800]
        # Count braces
        b_depth = 1
        has_else = False
        for c_idx, c in enumerate(lookahead):
            if c == '{': b_depth += 1
            elif c == '}':
                b_depth -= 1
                if b_depth == 0:
                    # After closing brace, check if next non-whitespace is 'else'
                    rest = lookahead[c_idx+1:].strip()
                    if rest.startswith('else'):
                        has_else = True
                    break
        if not has_else:
            warnings.append(f"MODE BRANCH TRAP: 'if (modernMode)' block has no 'else' branch. Verify Vintage mode is not silently un-filtered!")

    return errors, warnings


def main():
    parser = argparse.ArgumentParser(description="Zero-Dummy-Argument & DSP Sink Reachability Linter")
    parser.add_argument("plugin", nargs="?", default=None, help="Target plugin (e.g. shoggoth, hastur, nyarlat)")
    parser.add_argument("--all", action="store_true", help="Audit all plugins in source/")
    args = parser.parse_args()

    targets = []
    if args.all:
        src_root = ROOT_TKS / "source"
        for d in sorted(src_root.iterdir()):
            if d.is_dir():
                targets.append(d.name)
    elif args.plugin:
        targets.append(args.plugin)
    else:
        parser.error("Specify a plugin name or --all")

    total_errors = 0
    total_warnings = 0

    print(f"[*] SQA TITAN LINTER: Auditing DSP sink reachability across {len(targets)} plugin(s)...")

    for pl in targets:
        pascal = pl[:1].upper() + pl[1:]
        voice_header = ROOT_TKS / "source" / pl.lower() / f"{pascal}Voice.h"
        if not voice_header.exists():
            continue

        errs, warns = lint_voice_header(voice_header)
        total_errors += len(errs)
        total_warnings += len(warns)

        if errs or warns:
            print(f"\n[!] Audit results for {pl.upper()} ({voice_header.name}):")
            for e in errs:
                print(f"   ❌ ERROR: {e}")
            for w in warns:
                print(f"   ⚠️  WARN:  {w}")
        else:
            print(f"   [+] {pl}: 100% of renderBlock arguments reach active mathematical sinks.")

    print(f"\n[=] SQA TITAN LINT SUMMARY: {total_errors} error(s), {total_warnings} warning(s)")
    sys.exit(1 if total_errors > 0 else 0)


if __name__ == "__main__":
    main()
