#!/usr/bin/env python3
"""
tools/strike_pack.py — High-Voltage Deterministic Diagnostic Packager
====================================================================
Part of The Klang Suite SOCEP Engine (v3.1).
Mechanically extracts, deduplicates, and bounds compiler diagnostics,
unit test failures, and UI collisions into clean, unfolded markdown
artifacts (<6KB) with line-agnostic error fingerprinting.
Zero LLM parent-context token tax.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import sys
from typing import Dict, List, Optional, Tuple, Any

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


def normalize_msg(msg: str) -> str:
    """Normalizes compiler messages for semantic fingerprinting by stripping quotes and ephemeral paths."""
    # Collapse pointer addresses and temp numbers
    msg = re.sub(r'0x[0-9a-fA-F]+', '0xADDR', msg)
    msg = re.sub(r'\s+', ' ', msg).strip()
    return msg


def find_enclosing_symbol(file_path: Path, line_num: int) -> str:
    """Scans backwards in source file to find enclosing method/function/class signature."""
    if not file_path.exists():
        return "global"
    try:
        lines = file_path.read_text(encoding="utf-8", errors="replace").splitlines()
        target_idx = max(0, min(line_num - 1, len(lines) - 1))
        
        # Walk upward up to 300 lines to find function definition or class/struct
        for i in range(target_idx, max(-1, target_idx - 300), -1):
            l = lines[i].strip()
            if not l or l.startswith(("//", "/*", "*", "#", "if", "else", "switch", "case", "for", "while", "return")):
                continue
            if ";" in l or "=" in l:
                continue
            if re.search(r'\b[a-zA-Z0-9_~:]+\s*\([^;]*\)', l) and ("{" in l or not l.endswith(";")):
                m = re.search(r'([a-zA-Z0-9_]+::[a-zA-Z0-9_~]+|[a-zA-Z0-9_~]+)\s*\(', l)
                if m:
                    return m.group(1)
                return l[:60]
            if l.startswith(("class ", "struct ", "namespace ")):
                return l.split("{")[0].strip()[:60]
        return "global"
    except Exception:
        return "unknown"


def extract_source_slice(file_path: Path, line_num: int, radius: int = 15) -> str:
    """Extracts +-radius lines of numbered unfolded source code."""
    if not file_path.exists():
        return f"// Source file not found: {file_path}"
    try:
        lines = file_path.read_text(encoding="utf-8", errors="replace").splitlines()
        start = max(0, line_num - radius - 1)
        end = min(len(lines), line_num + radius)
        
        out = []
        for i in range(start, end):
            curr_line = i + 1
            marker = " > " if curr_line == line_num else "   "
            out.append(f"{curr_line:5d}{marker}{lines[i]}")
        return "\n".join(out)
    except Exception as e:
        return f"// Error reading source slice: {e}"


def parse_compiler_errors(log_text: str, root_dir: Path) -> Tuple[List[Dict[str, Any]], str]:
    """Parses MSVC, GCC, Clang, and LNK errors into structured records."""
    records = []
    seen = set()

    # Pattern for MSVC column and non-column formats:
    # Foo.cpp(142,17): error C2065: 'x': undeclared identifier
    # C:\Dev\Foo.cpp(142): error C2065: ...
    msvc_pat = re.compile(
        r'^\s*(?:\d+>)?(?P<file>[a-zA-Z0-9_:\\\/\.\-\s]+?)\((?P<line>\d+)(?:,\d+)?\):\s+(?:fatal\s+)?error\s+(?P<code>[A-Z0-9]+):\s*(?P<msg>.*)$',
        re.MULTILINE
    )
    # Pattern for GCC/Clang:
    # Foo.cpp:142:17: error: ...
    # C:/Dev/Foo.cpp:142:17: error: ...
    gcc_pat = re.compile(
        r'^\s*(?P<file>(?:[a-zA-Z]:)?[a-zA-Z0-9_\\\/\.\-]+?):(?P<line>\d+)(?::\d+)?:\s+(?:fatal\s+)?error:\s*(?P<msg>.*)$',
        re.MULTILINE
    )
    # Pattern for Linker errors:
    # error LNK2019: unresolved external symbol "..."
    lnk_pat = re.compile(
        r'^\s*(?:\d+>)?(?P<file>.*?)?\s*:?\s*error\s+(?P<code>LNK\d+):\s*(?P<msg>unresolved external symbol\s+["\']?(?P<sym>[^"\'\r\n]+)["\']?.*)$',
        re.MULTILINE
    )

    for m in msvc_pat.finditer(log_text):
        f = m.group("file").strip()
        l = int(m.group("line"))
        c = m.group("code").strip()
        msg = m.group("msg").strip()
        
        # Normalize relative path
        abs_p = (root_dir / f).resolve() if not Path(f).is_absolute() else Path(f).resolve()
        rel_p = str(abs_p.relative_to(root_dir)).replace("\\", "/") if str(abs_p).startswith(str(root_dir)) else f
        
        key = (rel_p, c, normalize_msg(msg))
        if key not in seen:
            seen.add(key)
            records.append({
                "type": "compiler",
                "file": rel_p,
                "abs_file": abs_p,
                "line": l,
                "code": c,
                "msg": msg,
                "sym": find_enclosing_symbol(abs_p, l)
            })

    for m in gcc_pat.finditer(log_text):
        f = m.group("file").strip()
        l = int(m.group("line"))
        c = "GCC_ERR"
        msg = m.group("msg").strip()
        abs_p = (root_dir / f).resolve() if not Path(f).is_absolute() else Path(f).resolve()
        rel_p = str(abs_p.relative_to(root_dir)).replace("\\", "/") if str(abs_p).startswith(str(root_dir)) else f
        key = (rel_p, c, normalize_msg(msg))
        if key not in seen:
            seen.add(key)
            records.append({
                "type": "compiler",
                "file": rel_p,
                "abs_file": abs_p,
                "line": l,
                "code": c,
                "msg": msg,
                "sym": find_enclosing_symbol(abs_p, l)
            })

    for m in lnk_pat.finditer(log_text):
        f = m.group("file").strip() or "CMakeLists.txt"
        c = m.group("code").strip()
        msg = m.group("msg").strip()
        sym_name = m.group("sym") if "sym" in m.groupdict() else "unresolved"
        key = (f, c, normalize_msg(msg))
        if key not in seen:
            seen.add(key)
            records.append({
                "type": "linker",
                "file": f,
                "abs_file": root_dir / f,
                "line": 1,
                "code": c,
                "msg": msg,
                "sym": sym_name
            })

    # Line-agnostic fingerprint: SHA1(sorted{(file, code, normalize(msg), sym)})
    fp_items = sorted((r["file"], r["code"], normalize_msg(r["msg"]), r["sym"]) for r in records)
    fp_str = json.dumps(fp_items, sort_keys=True)
    fingerprint = hashlib.sha1(fp_str.encode("utf-8")).hexdigest()

    return records, fingerprint


def parse_test_failures(log_text: str, root_dir: Path) -> Tuple[List[Dict[str, Any]], str]:
    """Parses Catch2 or dsp_tests failures into structured records."""
    records = []
    seen = set()

    # Match Catch2 assertion failures:
    # C:\Dev\TheKlangSuite\test\dsp_tests.cpp(156): FAILED:
    #   REQUIRE( abs(diff) < 1e-4f )
    # with expansion:
    #   0.0012f < 0.0001f
    pat = re.compile(
        r'(?P<file>[a-zA-Z0-9_\\\/\.\-]+?)\((?P<line>\d+)\):\s+FAILED:\s*\n\s*(?P<expr>REQUIRE[^\n]+)\s*(?:\n\s*with expansion:\s*\n\s*(?P<exp>[^\n]+))?',
        re.MULTILINE
    )

    for m in pat.finditer(log_text):
        f = m.group("file").strip()
        l = int(m.group("line"))
        expr = m.group("expr").strip()
        expansion = (m.group("exp") or "").strip()
        abs_p = Path(f).resolve()
        rel_p = str(abs_p.relative_to(root_dir)).replace("\\", "/") if str(abs_p).startswith(str(root_dir)) else f
        
        key = (rel_p, expr)
        if key not in seen:
            seen.add(key)
            records.append({
                "type": "test_failure",
                "file": rel_p,
                "abs_file": abs_p,
                "line": l,
                "code": "CATCH2_ASSERT",
                "msg": f"{expr} -> {expansion}" if expansion else expr,
                "sym": find_enclosing_symbol(abs_p, l)
            })

    fp_items = sorted((r["file"], r["code"], normalize_msg(r["msg"]), r["sym"]) for r in records)
    fp_str = json.dumps(fp_items, sort_keys=True)
    fingerprint = hashlib.sha1(fp_str.encode("utf-8")).hexdigest()

    return records, fingerprint


def parse_ui_collisions(log_text: str) -> Tuple[List[Dict[str, Any]], str]:
    """Parses [RADAR] collisions from ui_gate.py log."""
    records = []
    seen = set()
    pat = re.compile(
        r'\[RADAR\] Collision:\s+(?P<n1>\S+)\s+(?P<x1>-?\d+)\s+(?P<y1>-?\d+)\s+(?P<w1>\d+)\s+(?P<h1>\d+)\s+overlaps\s+(?P<n2>\S+)\s+(?P<x2>-?\d+)\s+(?P<y2>-?\d+)\s+(?P<w2>\d+)\s+(?P<h2>\d+)'
    )
    for m in pat.finditer(log_text):
        n1 = m.group("n1")
        n2 = m.group("n2")
        key = (n1, n2)
        if key not in seen:
            seen.add(key)
            records.append({
                "type": "radar_collision",
                "file": "Editor.cpp",
                "code": "RADAR_OVERLAP",
                "msg": m.group(0),
                "sym": f"{n1} overlaps {n2}"
            })
    fp_items = sorted((r["file"], r["code"], normalize_msg(r["msg"]), r["sym"]) for r in records)
    fingerprint = hashlib.sha1(json.dumps(fp_items, sort_keys=True).encode("utf-8")).hexdigest()
    return records, fingerprint


def pack_diagnostics(log_path: Path, out_path: Path, diag_type: str = "compiler", root_dir: Optional[Path] = None) -> Dict[str, Any]:
    """Reads raw log from disk and writes a bounded <6KB diagnostic markdown card."""
    root = root_dir or Path(r"C:\Dev\TheKlangSuite")
    log_text = log_path.read_text(encoding="utf-8", errors="replace") if log_path.exists() else ""
    
    if diag_type == "compiler":
        records, fingerprint = parse_compiler_errors(log_text, root)
    elif diag_type == "test":
        records, fingerprint = parse_test_failures(log_text, root)
    elif diag_type == "ui":
        records, fingerprint = parse_ui_collisions(log_text)
    else:
        records, fingerprint = parse_compiler_errors(log_text, root)

    total_count = len(records)
    selected = records[:3]  # Focus strictly on the first 3 root-cause errors
    
    md_lines = [
        f"# SOCEP Diagnostic Artifact",
        f"- **Type**: `{diag_type.upper()}`",
        f"- **Total Unique Errors**: {total_count}",
        f"- **Primary Root Causes Shown**: {len(selected)}",
        f"- **Semantic Fingerprint**: `{fingerprint}`",
        f"",
        f"---",
    ]

    for idx, r in enumerate(selected, 1):
        md_lines.append(f"### Error {idx}: `{r.get('code', 'ERR')}` in `{r.get('file', 'unknown')}`")
        md_lines.append(f"- **Scope**: `{r.get('sym', 'global')}` (Line {r.get('line', '?')})")
        md_lines.append(f"- **Diagnostic**: `{r.get('msg', '')}`")
        md_lines.append(f"")
        md_lines.append("```cpp")
        abs_p = r.get("abs_file")
        if abs_p and isinstance(abs_p, Path) and abs_p.exists():
            md_lines.append(extract_source_slice(abs_p, r.get("line", 1), radius=15))
        else:
            md_lines.append(f"// (Source file {r.get('file')} not found on disk)")
        md_lines.append("```")
        md_lines.append("")

    content = "\n".join(md_lines)
    # Ensure hard cap of 6144 bytes (<6KB)
    if len(content.encode("utf-8")) > 6000:
        content = content[:5800] + "\n\n// ... [TRUNCATED TO 6KB BUDGET] ...\n"

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(content, encoding="utf-8")

    return {
        "status": "success",
        "diag_path": str(out_path).replace("\\", "/"),
        "total_errors": total_count,
        "shown_errors": len(selected),
        "fingerprint": fingerprint,
        "bytes": len(content.encode("utf-8"))
    }


def main():
    parser = argparse.ArgumentParser(description="SOCEP Diagnostic Packager CLI")
    parser.add_argument("--log", required=True, help="Path to raw compiler or test log")
    parser.add_argument("--out", required=True, help="Output path for packaged markdown")
    parser.add_argument("--type", choices=["compiler", "test", "ui"], default="compiler", help="Diagnostic category")
    parser.add_argument("--root", default=r"C:\Dev\TheKlangSuite", help="Repository root path")
    args = parser.parse_args()

    res = pack_diagnostics(Path(args.log), Path(args.out), diag_type=args.type, root_dir=Path(args.root))
    print(json.dumps(res))


if __name__ == "__main__":
    main()
