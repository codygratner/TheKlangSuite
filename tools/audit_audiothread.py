#!/usr/bin/env python3
"""
Stand-Alone Real-Time Audio-Thread Safety Linter (audiothread-guard)
Scans C++ DSP source code for heap allocations, blocking locks, file/console I/O,
and unvectorized transcendentals in real-time audio render paths.

Features:
- Scope-aware analysis: Distinguishes hot render loops (process(), processBlock(), processStereo())
  from pre-allocation setup routines (prepareToPlay(), init(), constructors).
- Zero external dependencies: Python standard library only.
- Pre-commit & CI integration: Supports --strict mode for GitHub Actions.
"""

import argparse
import os
from pathlib import Path
import re
import subprocess
import sys
from typing import Dict, List, Optional, Tuple

# Reconfigure stdout/stderr for clean UTF-8 rendering on Windows PowerShell
if sys.stdout and hasattr(sys.stdout, "reconfigure"):
    try:
        sys.stdout.reconfigure(encoding="utf-8")
    except Exception:
        pass

# Critical real-time violations
HOT_PATH_PATTERNS = {
    "ALLOCATION": [
        (re.compile(r"\bnew\s+[A-Za-z0-9_:]+"), "Heap allocation (new) on audio thread"),
        (re.compile(r"\bmalloc\s*\("), "C heap allocation (malloc) on audio thread"),
        (re.compile(r"\bcalloc\s*\("), "C heap allocation (calloc) on audio thread"),
        (re.compile(r"\brealloc\s*\("), "C heap allocation (realloc) on audio thread"),
        (re.compile(r"\bfree\s*\("), "C heap deallocation (free) on audio thread"),
        (re.compile(r"\bstd::make_unique\s*<"), "Heap allocation (std::make_unique) on audio thread"),
        (re.compile(r"\bstd::make_shared\s*<"), "Heap allocation (std::make_shared) on audio thread"),
        (re.compile(r"\.push_back\s*\("), "Dynamic vector resize (push_back) on audio thread"),
        (re.compile(r"\.emplace_back\s*\("), "Dynamic vector resize (emplace_back) on audio thread"),
        (re.compile(r"\.resize\s*\("), "Dynamic container resize on audio thread"),
    ],
    "BLOCKING_LOCK": [
        (re.compile(r"\bstd::mutex\b"), "Standard mutex (std::mutex) in audio path"),
        (re.compile(r"\bstd::lock_guard\b"), "Mutex acquisition (std::lock_guard) in audio path"),
        (re.compile(r"\bstd::unique_lock\b"), "Mutex acquisition (std::unique_lock) in audio path"),
        (re.compile(r"\bjuce::CriticalSection\b"), "JUCE CriticalSection in audio path"),
        (re.compile(r"\bjuce::ScopedLock\b"), "JUCE ScopedLock in audio path"),
        (re.compile(r"\bstd::this_thread::sleep_for\b"), "Thread sleep (sleep_for) in audio path"),
        (re.compile(r"\bstd::this_thread::sleep_until\b"), "Thread sleep (sleep_until) in audio path"),
    ],
    "BLOCKING_IO": [
        (re.compile(r"\bstd::cout\b"), "Console I/O (std::cout) in audio path"),
        (re.compile(r"\bstd::cerr\b"), "Console I/O (std::cerr) in audio path"),
        (re.compile(r"\bprintf\s*\("), "Console output (printf) in audio path"),
        (re.compile(r"\bDBG\s*\("), "JUCE debug logging (DBG) in audio path"),
        (re.compile(r"\bjuce::Logger::writeToLog\b"), "JUCE Logger write in audio path"),
        (re.compile(r"\bstd::ofstream\b"), "Filesystem output stream in audio path"),
        (re.compile(r"\bstd::ifstream\b"), "Filesystem input stream in audio path"),
    ],
    "TRANSCENDENTAL_WARNING": [
        (re.compile(r"\bstd::sin\s*\("), "CRT transcendental (std::sin) - prefer TbdAudio::FastMath::sin"),
        (re.compile(r"\bstd::cos\s*\("), "CRT transcendental (std::cos) - prefer TbdAudio::FastMath::cos"),
        (re.compile(r"\bstd::tan\s*\("), "CRT transcendental (std::tan) - prefer TbdAudio::FastMath"),
        (re.compile(r"\bstd::tanh\s*\("), "CRT transcendental (std::tanh) - prefer TbdAudio::FastMath::tanh"),
        (re.compile(r"\bstd::pow\s*\("), "CRT transcendental (std::pow) - prefer TbdAudio::FastMath"),
        (re.compile(r"\bstd::exp\s*\("), "CRT transcendental (std::exp) - prefer TbdAudio::FastMath"),
        (re.compile(r"\bstd::log\s*\("), "CRT transcendental (std::log) - prefer TbdAudio::FastMath"),
    ],
}

# Regex to detect hot audio-thread method signatures
HOT_METHOD_RE = re.compile(
    r"\b(processBlock|processStereo|process|render|renderAudio|processAudio|evaluateWaveform|applyEnvelopeSlope)\s*\(",
    re.IGNORECASE
)

# Regex to detect setup/initialization method signatures
SETUP_METHOD_RE = re.compile(
    r"\b(prepareToPlay|releaseResources|init|reset|createFXBlock|setup|configure)\s*\(",
    re.IGNORECASE
)


def strip_comments_and_strings(line: str) -> str:
    """Removes single-line comments and string literals to prevent false positives."""
    comment_idx = line.find("//")
    if comment_idx != -1:
        line = line[:comment_idx]
    line = re.sub(r'"(?:\\.|[^"\\])*"', '""', line)
    return line


def audit_file(file_path: Path, hot_paths_only: bool = True) -> List[Dict]:
    """Audits a C++ DSP file, focusing strictly on real-time render scopes."""
    violations = []
    try:
        content = file_path.read_text(encoding="utf-8", errors="replace")
    except Exception as e:
        return [{"line": 0, "category": "ERROR", "message": f"Failed to read file: {e}", "snippet": ""}]

    lines = content.splitlines()
    in_block_comment = False
    brace_depth = 0
    in_hot_method = False
    current_method_name = ""
    hot_method_brace_level = 0

    for idx, raw_line in enumerate(lines, start=1):
        line = raw_line.strip()

        # Handle multi-line block comments
        if in_block_comment:
            if "*/" in line:
                in_block_comment = False
                line = line[line.find("*/") + 2:]
            else:
                continue

        if "/*" in line:
            if "*/" in line:
                line = re.sub(r"/\*.*?\*/", "", line)
            else:
                in_block_comment = True
                line = line[:line.find("/*")]

        cleaned = strip_comments_and_strings(line)
        if not cleaned:
            continue

        # Track method entry if hot_paths_only
        if hot_paths_only:
            hot_m = HOT_METHOD_RE.search(cleaned)
            setup_m = SETUP_METHOD_RE.search(cleaned)

            if hot_m and not in_hot_method:
                in_hot_method = True
                current_method_name = hot_m.group(1)
                hot_method_brace_level = brace_depth

            elif setup_m and in_hot_method:
                in_hot_method = False

        # Track brace scope
        open_braces = cleaned.count("{")
        close_braces = cleaned.count("}")
        brace_depth += (open_braces - close_braces)

        if in_hot_method and brace_depth <= hot_method_brace_level:
            in_hot_method = False

        # If we only want hot paths and are not currently in one, skip
        if hot_paths_only and not in_hot_method:
            continue

        # Check violation patterns
        for category, rules in HOT_PATH_PATTERNS.items():
            for pattern, msg in rules:
                if pattern.search(cleaned):
                    violations.append({
                        "file": str(file_path),
                        "line": idx,
                        "method": current_method_name if in_hot_method else "global/function",
                        "category": category,
                        "message": msg,
                        "snippet": raw_line.strip()
                    })

    return violations


CORE_DSP_FILES = [
    "DSPBlock.h",
    "FastMath.h",
    "ModularBlocks.h",
    "ModulationEngine.h",
    "ModulationEngine.cpp",
    "FarmerProcessor.h",
    "FarmerProcessor.cpp",
    "PlanterEngine.h",
    "PlanterEngine.cpp",
    "PlanterProcessor.h",
    "PlanterProcessor.cpp",
]


def resolve_files_to_scan(target_path: Optional[str] = None) -> List[Path]:
    """Resolves target files, prioritizing canonical Core DSP files."""
    if target_path:
        p = Path(target_path)
        if p.is_file():
            return [p]
        elif p.is_dir():
            files = []
            for ext in ("*.h", "*.hpp", "*.cpp"):
                files.extend(p.rglob(ext))
            return files

    # Default: Core DSP files in Source/
    source_dir = Path("Source")
    resolved = []
    for name in CORE_DSP_FILES:
        target = source_dir / name
        if target.exists():
            resolved.append(target)
    return resolved


def main():
    parser = argparse.ArgumentParser(description="Real-Time Audio Thread Safety Linter")
    parser.add_argument("--path", "-p", default=None, help="Directory or file to audit")
    parser.add_argument("--all-paths", action="store_true", help="Audit all code blocks, not just hot audio paths")
    parser.add_argument("--strict", action="store_true", help="Fail with non-zero exit code on any critical violation")
    args = parser.parse_args()

    files = resolve_files_to_scan(args.path)
    if not files:
        print("✓ No target DSP files found to audit.")
        sys.exit(0)

    total_violations = 0
    total_warnings = 0
    results_by_file = {}

    for f in sorted(files):
        violations = audit_file(f, hot_paths_only=not args.all_paths)
        if violations:
            results_by_file[f] = violations
            for v in violations:
                if v["category"] == "TRANSCENDENTAL_WARNING":
                    total_warnings += 1
                else:
                    total_violations += 1

    print("=" * 70)
    print(f"🛡️ Audio Thread Safety Linter: Audited {len(files)} Core DSP files")
    print(f"   Critical Violations: {total_violations} | FastMath Warnings: {total_warnings}")
    print("=" * 70)

    if not results_by_file:
        print("✅ 100% CLEAN: Zero real-time audio-thread violations detected.")
        sys.exit(0)

    for f, v_list in results_by_file.items():
        print(f"\n📄 {f}:")
        for v in v_list:
            icon = "⚠️" if v["category"] == "TRANSCENDENTAL_WARNING" else "🚨"
            scope = f"[{v['method']}]" if v['method'] else ""
            print(f"  {icon} [Line {v['line']}] {v['category']} {scope}: {v['message']}")
            print(f"     > {v['snippet']}")

    if args.strict and total_violations > 0:
        print(f"\n❌ FAILED: Strict mode enabled and {total_violations} critical violations found.")
        sys.exit(1)
    else:
        print("\n✓ Audit complete.")
        sys.exit(0)


if __name__ == "__main__":
    main()
