#!/usr/bin/env python3
"""
verify_utf8.py - Universal UTF-8 Invariant Linter & Auditor (Flight 18 / Mailbag rj-02)

Audits Python scripts across The Klang Suite, N'kai, and ToadTracker to enforce:
1. Entrypoint UTF-8 console output (sys.stdout/stderr.reconfigure(encoding='utf-8')).
2. Explicit encoding="utf-8" on all text file I/O (open(), read_text(), write_text()).
3. Zero reliance on Windows default cp1252 ANSI code page.
"""

import ast
import argparse
import os
import sys
from pathlib import Path
from typing import List, Tuple, Dict, Any

# Bootstrap UTF-8 console streams immediately
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

VALID_UTF8_ENCODINGS = {"utf-8", "utf_8", "utf8", "utf-8-sig", "utf_8_sig", "utf8_sig"}


class FileIOVisitor(ast.NodeVisitor):
    def __init__(self, filename: str):
        self.filename = filename
        self.violations: List[Tuple[int, str, str]] = []

    def visit_Call(self, node: ast.Call):
        # 1. Check open(...) calls
        if isinstance(node.func, ast.Name) and node.func.id == "open":
            self._check_open_call(node)
        # 2. Check path.read_text(...) or path.write_text(...) calls
        elif isinstance(node.func, ast.Attribute) and node.func.attr in {"read_text", "write_text"}:
            self._check_path_text_call(node)

        self.generic_visit(node)

    def _check_open_call(self, node: ast.Call):
        # Determine mode
        mode = "r"
        # Check positional args: open(file, mode, ...)
        if len(node.args) >= 2:
            if isinstance(node.args[1], ast.Constant) and isinstance(node.args[1].value, str):
                mode = node.args[1].value

        # Check keyword args: open(file, mode='...', encoding='...')
        for kw in node.keywords:
            if kw.arg == "mode" and isinstance(kw.value, ast.Constant) and isinstance(kw.value.value, str):
                mode = kw.value.value

        # Binary modes are exempt from encoding
        if "b" in mode:
            return

        # Check for explicit encoding keyword
        encoding_kw = next((kw for kw in node.keywords if kw.arg == "encoding"), None)
        if not encoding_kw:
            self.violations.append((
                node.lineno,
                "open()",
                f"Missing explicit encoding='utf-8' in text mode '{mode}'"
            ))
        else:
            if isinstance(encoding_kw.value, ast.Constant) and isinstance(encoding_kw.value.value, str):
                enc_val = encoding_kw.value.value.lower()
                if enc_val not in VALID_UTF8_ENCODINGS:
                    self.violations.append((
                        node.lineno,
                        "open()",
                        f"Non-UTF-8 encoding '{enc_val}' specified (expected 'utf-8')"
                    ))

    def _check_path_text_call(self, node: ast.Call):
        method_name = node.func.attr
        # read_text(encoding=None, errors=None)
        # write_text(data, encoding=None, errors=None)
        encoding_kw = next((kw for kw in node.keywords if kw.arg == "encoding"), None)
        
        # In write_text, encoding can be the 2nd positional argument: write_text(data, 'utf-8')
        if not encoding_kw:
            pos_enc = None
            if method_name == "read_text" and len(node.args) >= 1:
                pos_enc = node.args[0]
            elif method_name == "write_text" and len(node.args) >= 2:
                pos_enc = node.args[1]

            if pos_enc and isinstance(pos_enc, ast.Constant) and isinstance(pos_enc.value, str):
                enc_val = pos_enc.value.lower()
                if enc_val not in VALID_UTF8_ENCODINGS:
                    self.violations.append((
                        node.lineno,
                        f"Path.{method_name}()",
                        f"Non-UTF-8 encoding '{enc_val}' specified (expected 'utf-8')"
                    ))
                return

            self.violations.append((
                node.lineno,
                f"Path.{method_name}()",
                f"Missing explicit encoding='utf-8' on Path.{method_name}() call"
            ))
        else:
            if isinstance(encoding_kw.value, ast.Constant) and isinstance(encoding_kw.value.value, str):
                enc_val = encoding_kw.value.value.lower()
                if enc_val not in VALID_UTF8_ENCODINGS:
                    self.violations.append((
                        node.lineno,
                        f"Path.{method_name}()",
                        f"Non-UTF-8 encoding '{enc_val}' specified (expected 'utf-8')"
                    ))


def has_main_entrypoint(source: str) -> bool:
    return '__name__ == "__main__"' in source or "__name__ == '__main__'" in source


def has_stdout_reconfigure(source: str) -> bool:
    return "stdout.reconfigure" in source and "utf-8" in source.lower()


def audit_file(filepath: Path) -> Dict[str, Any]:
    try:
        content = filepath.read_text(encoding="utf-8")
    except UnicodeDecodeError as e:
        return {
            "path": filepath,
            "error": f"File is NOT valid UTF-8: {e}",
            "io_violations": [],
            "entrypoint_missing_reconfigure": False,
        }

    try:
        tree = ast.parse(content, filename=str(filepath))
    except SyntaxError as e:
        return {
            "path": filepath,
            "error": f"Python syntax error: {e}",
            "io_violations": [],
            "entrypoint_missing_reconfigure": False,
        }

    visitor = FileIOVisitor(str(filepath))
    visitor.visit(tree)

    is_entrypoint = has_main_entrypoint(content)
    missing_reconfigure = is_entrypoint and not has_stdout_reconfigure(content)

    return {
        "path": filepath,
        "error": None,
        "io_violations": visitor.violations,
        "entrypoint_missing_reconfigure": missing_reconfigure,
        "is_entrypoint": is_entrypoint,
    }


def fix_entrypoint_reconfigure(filepath: Path) -> bool:
    """Safely injects sys.stdout/stderr reconfigure into an entrypoint script."""
    content = filepath.read_text(encoding="utf-8")
    if has_stdout_reconfigure(content):
        return False

    injection = (
        "\n# Configure UTF-8 streams for Windows console compatibility (Universal UTF-8 Invariant)\n"
        "if hasattr(sys.stdout, 'reconfigure'):\n"
        "    sys.stdout.reconfigure(encoding='utf-8')\n"
        "if hasattr(sys.stderr, 'reconfigure'):\n"
        "    sys.stderr.reconfigure(encoding='utf-8')\n"
    )

    # Insert after imports
    lines = content.splitlines(keepends=True)
    insert_idx = 0
    in_docstring = False
    docstring_char = None

    for i, line in enumerate(lines):
        stripped = line.strip()
        if i == 0 and stripped.startswith("#!"):
            insert_idx = i + 1
            continue
        if not in_docstring and (stripped.startswith('"""') or stripped.startswith("'''")):
            in_docstring = True
            docstring_char = stripped[:3]
            if stripped.count(docstring_char) >= 2 and len(stripped) > 3:
                in_docstring = False
                insert_idx = i + 1
            continue
        if in_docstring:
            if docstring_char in stripped:
                in_docstring = False
                insert_idx = i + 1
            continue
        if stripped.startswith("import ") or stripped.startswith("from "):
            insert_idx = i + 1
        elif stripped and not stripped.startswith("#") and insert_idx > 0:
            break

    # If sys is not imported, add import sys
    sys_import_needed = "import sys" not in content and "from sys" not in content
    if sys_import_needed:
        injection = "import sys\n" + injection

    new_lines = lines[:insert_idx] + [injection] + lines[insert_idx:]
    filepath.write_text("".join(new_lines), encoding="utf-8")
    return True


def run_audit(target_paths: List[Path], auto_fix: bool = False, verbose: bool = False) -> int:
    files_to_check: List[Path] = []
    for p in target_paths:
        if p.is_file() and p.suffix == ".py":
            files_to_check.append(p)
        elif p.is_dir():
            files_to_check.extend(sorted(p.rglob("*.py")))

    # Deduplicate
    files_to_check = sorted(list(set(files_to_check)))

    print("=" * 72)
    print("🔤 Universal UTF-8 Invariant Auditor (Flight 18 / Topic rj-02)")
    print(f"📁 Target Scopes: {len(target_paths)} paths | {len(files_to_check)} Python files")
    print("=" * 72)

    total_violations = 0
    total_missing_reconfig = 0
    files_with_issues = 0

    for filepath in files_to_check:
        result = audit_file(filepath)
        path_str = str(filepath.resolve())

        if result["error"]:
            print(f"\n[❌ ERROR] {path_str}")
            print(f"   {result['error']}")
            files_with_issues += 1
            total_violations += 1
            continue

        io_count = len(result["io_violations"])
        missing_reconfig = result["entrypoint_missing_reconfigure"]

        if missing_reconfig and auto_fix:
            fixed = fix_entrypoint_reconfigure(filepath)
            if fixed:
                print(f"[🔧 FIXED] Injected UTF-8 reconfigure into {filepath.name}")
                # Re-audit after fix
                result = audit_file(filepath)
                missing_reconfig = result["entrypoint_missing_reconfigure"]

        if io_count > 0 or missing_reconfig:
            files_with_issues += 1
            total_violations += io_count
            if missing_reconfig:
                total_missing_reconfig += 1

            rel_path = filepath
            try:
                rel_path = filepath.relative_to(Path.cwd())
            except ValueError:
                pass

            print(f"\n[⚠️ VIOLATION] {rel_path}")
            if missing_reconfig:
                print("   • Line 1: Entrypoint has '__main__' but lacks sys.stdout.reconfigure(encoding='utf-8')")
            for lineno, call_type, msg in result["io_violations"]:
                print(f"   • Line {lineno} [{call_type}]: {msg}")
        elif verbose:
            print(f"[✅ PASS] {filepath.name}")

    print("\n" + "=" * 72)
    print("📊 UTF-8 AUDIT SCORECARD:")
    print(f"   • Total Files Audited: {len(files_to_check)}")
    print(f"   • Files Compliant:     {len(files_to_check) - files_with_issues}")
    print(f"   • Files with Issues:   {files_with_issues}")
    print(f"   • Missing Reconfigs:   {total_missing_reconfig}")
    print(f"   • Missing I/O Encodings: {total_violations - total_missing_reconfig}")
    print("=" * 72)

    if files_with_issues == 0:
        print("✅ 100% UNIVERSAL UTF-8 INVARIANT COMPLIANT: All streams and I/O hardened.")
        return 0
    else:
        print(f"❌ AUDIT FAILED: {files_with_issues} files require explicit UTF-8 hardening.")
        return 1


def main():
    parser = argparse.ArgumentParser(
        description="Verify and enforce Universal UTF-8 Invariant across Python tools."
    )
    parser.add_argument(
        "--path",
        type=str,
        default=None,
        help="Target file or directory to audit (defaults to 'tools/' in current repo)",
    )
    parser.add_argument(
        "--all",
        action="store_true",
        help="Scan all ecosystem repositories (TheKlangSuite, nkai, bundle)",
    )
    parser.add_argument(
        "--fix",
        action="store_true",
        help="Automatically inject UTF-8 stdout/stderr reconfiguration into entrypoints",
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Exit with code 1 if any violations are found",
    )
    parser.add_argument(
        "--verbose", "-v",
        action="store_true",
        help="Print every file audited even if passing",
    )

    args = parser.parse_args()

    targets: List[Path] = []

    if args.all:
        tks_tools = Path("c:/Dev/TheKlangSuite/tools")
        nkai_tools = Path("c:/Dev/nkai/tools")
        nkai_bundle_tools = Path("c:/Dev/nkai/bundle/tools")
        if tks_tools.exists():
            targets.append(tks_tools)
        if nkai_tools.exists():
            targets.append(nkai_tools)
        if nkai_bundle_tools.exists():
            targets.append(nkai_bundle_tools)
    elif args.path:
        targets.append(Path(args.path))
    else:
        default_tools = Path("tools")
        if default_tools.exists():
            targets.append(default_tools)
        else:
            targets.append(Path("."))

    exit_code = run_audit(targets, auto_fix=args.fix, verbose=args.verbose)

    if args.strict and exit_code != 0:
        sys.exit(exit_code)
    sys.exit(0)


if __name__ == "__main__":
    main()
