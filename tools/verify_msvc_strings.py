#!/usr/bin/env python3
"""
verify_msvc_strings.py - Anti-Mojibake MSVC & Clean String Linter
Scans C++ sources in The Klang Suite for forbidden printf-style string conversions
in juce::String::formatted / formattedRaw that trigger MSVC CRT _vsnwprintf mojibake.
"""
import sys
import re
from pathlib import Path

if hasattr(sys.stdout, 'reconfigure'):
    sys.stdout.reconfigure(encoding='utf-8')
if hasattr(sys.stderr, 'reconfigure'):
    sys.stderr.reconfigure(encoding='utf-8')

# Matches printf-style string conversion specifiers: %s, %S, %hs, %ls, %c, %C
SPEC_RE = re.compile(r'%[-+ #0]*(?:\*|\d+)?(?:\.(?:\*|\d+))?(?:hh|h|ll|l|w|z|j|t|L|I32|I64)?[sScC]')
# Matches juce::String::formatted, String::formatted, String::formattedRaw
CALL_RE = re.compile(r'(?:\b(?:juce::)?String::\s*formatted(?:Raw)?)\s*\(')
LIT_RE  = re.compile(r'"((?:[^"\\\n]|\\.)*)"')

def audit_file(filepath: Path) -> list:
    errors = []
    try:
        content = filepath.read_text(encoding='utf-8', errors='replace')
    except Exception as e:
        return [f"{filepath}: CRITICAL: Could not read file: {e}"]

    # Paren-balanced scanner to handle multi-line calls
    for m in CALL_RE.finditer(content):
        j = m.end()
        depth = 1
        while j < len(content) and depth > 0:
            if content[j] == '(':
                depth += 1
            elif content[j] == ')':
                depth -= 1
            j += 1

        call_str = content[m.end():j]
        line_num = content.count('\n', 0, m.start()) + 1

        # Check if first non-whitespace argument is a string literal
        call_stripped = call_str.lstrip()
        if not call_stripped.startswith('"'):
            # Non-literal or variable format string
            errors.append(f"{filepath}:{line_num}: CRITICAL: Non-literal format string passed to formatted(). Use explicit concatenation.")
        else:
            # Extract literal strings
            literals = LIT_RE.findall(call_str)
            if literals:
                fmt_str = ''.join(literals).replace('%%', '')
                if SPEC_RE.search(fmt_str):
                    errors.append(
                        f"{filepath}:{line_num}: CRITICAL: Forbidden string conversion specifier in formatted(): '{fmt_str}'. "
                        f"Causes MSVC CRT _vsnwprintf to misinterpret 8-bit bytes as 16-bit wchar_t (CJK Mojibake). "
                        f"Use juce::String::fromUTF8() and operator+ instead."
                    )
    return errors

def main():
    root = Path(__file__).resolve().parent.parent
    target_dirs = [root / "source", root / "test"]
    extensions = {".cpp", ".h", ".hpp", ".cc", ".cxx", ".inl"}

    all_errors = []
    files_checked = 0

    for d in target_dirs:
        if not d.exists():
            continue
        for p in d.rglob("*"):
            if p.suffix in extensions:
                files_checked += 1
                all_errors.extend(audit_file(p))

    print(f"=== Anti-Mojibake MSVC String Linter ===")
    print(f"Audited {files_checked} C++ source files across source/ and test/.")

    if all_errors:
        print(f"\n[FAIL] Found {len(all_errors)} string formatting violations:")
        for err in all_errors:
            print(f"  • {err}")
        sys.exit(1)

    print(f"[PASS] 100% compliant: Zero MSVC CRT wide/narrow formatting hazards found.\n")
    sys.exit(0)

if __name__ == "__main__":
    main()
