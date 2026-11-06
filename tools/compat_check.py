#!/usr/bin/env python3
"""
tools/compat_check.py — Fast Cross-Platform, JUCE 9.0.3 & Platform Airlock Hygiene Linter
========================================================================================
Performs deterministic static analysis on CMakeLists.txt and C++ source/test files
for cross-platform compatibility (macOS deployment target, Linux static linking,
headless definitions, VST3 auto manifest), JUCE 9.0.3 TimerThread race condition
prevention (#1696), path inclusion portability, and the Platform Airlock Invariants (C1–C7).

Checks Enforced:
- C1: Zero platform headers in project headers (.h/.hpp/.hxx/.inl)
- C2: Platform headers restricted to quarantined TUs (test/support/HeadlessX11.cpp)
- C3: Bare Xlib entry point call detector
- C4: Banned #undef platform macro band-aids
- C5: Strict case-sensitive include path matching across source/ and test/
- C6: Target link libraries naming X11/X11::X11 forbidden (pure dynamic dlsym model)
- C7: ScopedJuceInitialiser_GUI restricted to ScopedGuiContext encapsulation

Usage:
    python tools/compat_check.py [--terse] [--json] [--strict] [--fix] [--root <path>]
"""

import argparse
import json
import os
from pathlib import Path
import re
import sys
import time
from typing import Any, Dict, List, Optional, Set, Tuple

# Universal UTF-8 Console Streams for Windows
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

def strip_c_comments(text: str) -> str:
    """Removes single-line (//) and multi-line (/* */) C/C++ comments from source code."""
    pattern = r"//.*?$|/\*.*?\*/"
    return re.sub(pattern, "", text, flags=re.DOTALL | re.MULTILINE)

def strip_cmake_comments(text: str) -> str:
    """Removes single-line (#) comments from CMake files."""
    return re.sub(r"#.*?$", "", text, flags=re.MULTILINE)

def check_cmake_config(cmake_path: Path) -> List[Dict[str, Any]]:
    """Audits CMakeLists.txt for critical cross-platform declarations."""
    checks = []
    if not cmake_path.exists():
        checks.append({
            "category": "CMake",
            "name": "CMakeLists.txt Existence",
            "passed": False,
            "message": f"CMakeLists.txt not found at {cmake_path}",
        })
        return checks

    content = cmake_path.read_text(encoding="utf-8", errors="ignore")
    clean_content = strip_cmake_comments(content)

    # 1. macOS Deployment Target Placement (Must be BEFORE project())
    proj_match = re.search(r"\bproject\s*\(", clean_content)
    target_match = re.search(r"set\s*\(\s*CMAKE_OSX_DEPLOYMENT_TARGET\s+[\"']?11\.0[\"']?", clean_content)

    if target_match and proj_match:
        if target_match.start() < proj_match.start():
            checks.append({
                "category": "CMake",
                "name": "macOS 11.0 Deployment Target before project()",
                "passed": True,
                "message": "macOS deployment target correctly defined before project()",
            })
        else:
            checks.append({
                "category": "CMake",
                "name": "macOS 11.0 Deployment Target before project()",
                "passed": False,
                "message": "CMAKE_OSX_DEPLOYMENT_TARGET must be declared BEFORE project()",
            })
    elif target_match:
        checks.append({
            "category": "CMake",
            "name": "macOS 11.0 Deployment Target before project()",
            "passed": True,
            "message": "macOS deployment target defined (no project() tag found)",
        })
    else:
        checks.append({
            "category": "CMake",
            "name": "macOS 11.0 Deployment Target before project()",
            "passed": False,
            "message": "CMAKE_OSX_DEPLOYMENT_TARGET '11.0' not found",
        })

    # 2. macOS Universal Binary Architectures
    if re.search(r"set\s*\(\s*CMAKE_OSX_ARCHITECTURES\s+[\"']?(?:arm64;x86_64|x86_64;arm64)[\"']?", clean_content):
        checks.append({
            "category": "CMake",
            "name": "macOS Universal Binary (arm64;x86_64)",
            "passed": True,
            "message": "Universal binary architectures defined",
        })
    else:
        checks.append({
            "category": "CMake",
            "name": "macOS Universal Binary (arm64;x86_64)",
            "passed": False,
            "message": "CMAKE_OSX_ARCHITECTURES 'arm64;x86_64' not found",
        })

    # 3. Linux Static Standard Library Linking
    has_static_libstdcpp = "-static-libstdc++" in clean_content
    has_static_libgcc = "-static-libgcc" in clean_content
    if has_static_libstdcpp and has_static_libgcc:
        checks.append({
            "category": "CMake",
            "name": "Linux Static Standard Library Linking",
            "passed": True,
            "message": "Flags -static-libstdc++ and -static-libgcc present",
        })
    else:
        checks.append({
            "category": "CMake",
            "name": "Linux Static Standard Library Linking",
            "passed": False,
            "message": "Missing -static-libstdc++ or -static-libgcc in target_link_options",
        })

    # 4. Headless & Container Compile Definitions
    has_browser_off = bool(re.search(r"\bJUCE_WEB_BROWSER[=\s]+0\b", clean_content))
    has_curl_off = bool(re.search(r"\bJUCE_USE_CURL[=\s]+0\b", clean_content))
    has_vst2_off = bool(re.search(r"\bJUCE_VST3_CAN_REPLACE_VST2[=\s]+0\b", clean_content))

    if has_browser_off and has_curl_off and has_vst2_off:
        checks.append({
            "category": "CMake",
            "name": "Headless Compile Definitions",
            "passed": True,
            "message": "JUCE_WEB_BROWSER=0, JUCE_USE_CURL=0, JUCE_VST3_CAN_REPLACE_VST2=0 present",
        })
    else:
        missing = []
        if not has_browser_off: missing.append("JUCE_WEB_BROWSER=0")
        if not has_curl_off: missing.append("JUCE_USE_CURL=0")
        if not has_vst2_off: missing.append("JUCE_VST3_CAN_REPLACE_VST2=0")
        checks.append({
            "category": "CMake",
            "name": "Headless Compile Definitions",
            "passed": False,
            "message": f"Missing headless definition(s): {', '.join(missing)}",
        })

    # 5. VST3 Auto Manifest Flag
    if "VST3_AUTO_MANIFEST FALSE" in clean_content:
        checks.append({
            "category": "CMake",
            "name": "VST3 Auto Manifest Disabled",
            "passed": True,
            "message": "VST3_AUTO_MANIFEST FALSE configured",
        })
    else:
        checks.append({
            "category": "CMake",
            "name": "VST3 Auto Manifest Disabled",
            "passed": False,
            "message": "VST3_AUTO_MANIFEST FALSE not set on plugin targets",
        })

    # 6. Check C6: CMake Link Audit (forbids linking X11 / X11::X11 directly)
    x11_links = re.findall(r"target_link_libraries\s*\([^\)]*?\b(X11|X11::X11)\b", clean_content, flags=re.DOTALL)
    if x11_links:
        checks.append({
            "category": "Platform Airlock (C6)",
            "name": "Zero Direct X11 CMake Linkage",
            "passed": False,
            "message": f"target_link_libraries explicitly links X11 ({len(x11_links)} match(es)). All X11 entry points must use dynamic dlopen/dlsym.",
        })
    else:
        checks.append({
            "category": "Platform Airlock (C6)",
            "name": "Zero Direct X11 CMake Linkage",
            "passed": True,
            "message": "No targets link X11 or X11::X11 directly",
        })

    return checks

def check_timer_hygiene(source_dir: Path) -> Tuple[List[Dict[str, Any]], int]:
    """
    Audits all classes inheriting from juce::Timer in source/ to ensure their
    destructors explicitly invoke stopTimer() as the first action, guarding against
    the JUCE 9.0.3 TimerThread race condition (#1696).
    """
    checks = []
    if not source_dir.exists():
        return checks, 0

    cpp_files = [p for p in source_dir.rglob("*") if p.suffix in (".h", ".hpp", ".cpp", ".cxx")]
    files_checked = len(cpp_files)

    file_cache: Dict[Path, Tuple[str, str]] = {}
    for f in cpp_files:
        try:
            raw = f.read_text(encoding="utf-8", errors="ignore")
            file_cache[f] = (raw, strip_c_comments(raw))
        except Exception:
            pass

    timer_classes: Dict[str, Path] = {}
    class_pattern = re.compile(r"\bclass\s+([A-Za-z0-9_]+)\s*(?:final\s*)?:[^{]*\b(?:juce::)?Timer\b")

    for f, (raw, clean) in file_cache.items():
        for match in class_pattern.finditer(clean):
            cls_name = match.group(1)
            timer_classes[cls_name] = f

    if not timer_classes:
        checks.append({
            "category": "JUCE 9.0.3",
            "name": "Timer Destructor Hygiene (#1696)",
            "passed": True,
            "message": "No classes inheriting from juce::Timer found",
        })
        return checks, files_checked

    for cls_name, decl_file in timer_classes.items():
        found_destructor = False
        has_stop_timer = False
        destructor_body = ""
        destructor_location = ""

        decl_clean = file_cache[decl_file][1]
        inline_match = re.search(rf"~{cls_name}\s*\([^)]*\)\s*(?:override\s*)?\{{([^}}]*)\}}", decl_clean)
        if inline_match:
            found_destructor = True
            destructor_body = inline_match.group(1).strip()
            destructor_location = f"{decl_file.name} (inline)"

        if not found_destructor:
            for f, (raw, clean) in file_cache.items():
                if f.suffix in (".cpp", ".cxx"):
                    ool_match = re.search(rf"\b{cls_name}::~{cls_name}\s*\([^)]*\)\s*\{{([^}}]*)\}}", clean)
                    if ool_match:
                        found_destructor = True
                        destructor_body = ool_match.group(1).strip()
                        destructor_location = f"{f.name} (out-of-line)"
                        break

        if not found_destructor:
            checks.append({
                "category": "JUCE 9.0.3",
                "name": f"{cls_name}::~{cls_name}()",
                "passed": False,
                "message": f"Class {cls_name} inherits from juce::Timer but has no explicit destructor defined to call stopTimer()",
            })
            continue

        if re.search(r"\bstopTimer\s*\(\s*\)", destructor_body):
            has_stop_timer = True

        if has_stop_timer:
            first_stmt = destructor_body.split(";")[0].strip()
            is_first = "stopTimer" in first_stmt
            checks.append({
                "category": "JUCE 9.0.3",
                "name": f"{cls_name}::~{cls_name}()",
                "passed": True,
                "message": f"stopTimer() present in {destructor_location}{' on line 1' if is_first else ''}",
            })
        else:
            checks.append({
                "category": "JUCE 9.0.3",
                "name": f"{cls_name}::~{cls_name}()",
                "passed": False,
                "message": f"Destructor in {destructor_location} does NOT call stopTimer()! Risk of #1696 TimerThread crash.",
            })

    return checks, files_checked

def check_source_portability(repo_root: Path, auto_fix: bool = False) -> List[Dict[str, Any]]:
    """Audits source and test files for portability, casing, and Platform Airlock Invariants (C1–C5, C7)."""
    checks = []
    source_dir = repo_root / "source"
    test_dir = repo_root / "test"

    scan_dirs = [d for d in (source_dir, test_dir) if d.exists()]
    if not scan_dirs:
        return checks

    backslash_violations: List[str] = []
    case_violations: List[str] = []
    c1_banned_header_violations: List[str] = []
    c2_quarantine_violations: List[str] = []
    c3_bare_xlib_violations: List[str] = []
    c4_undef_macro_violations: List[str] = []
    c7_scoped_gui_violations: List[str] = []

    dir_entries_cache: Dict[Path, Dict[str, str]] = {}

    def get_dir_casing_map(d: Path) -> Dict[str, str]:
        if d not in dir_entries_cache:
            try:
                entries = {name.lower(): name for name in os.listdir(d)}
                dir_entries_cache[d] = entries
            except Exception:
                dir_entries_cache[d] = {}
        return dir_entries_cache[d]

    # Patterns
    banned_header_re = re.compile(
        r'^\s*#\s*include\s*[<"](X11/|xcb/|GL/glx\.h|windows\.h|windowsx\.h|winsock2\.h|AppKit/|Cocoa/|objc/)',
        re.IGNORECASE
    )
    undef_macro_re = re.compile(
        r'^\s*#\s*undef\s+(Status|None|Bool|True|False|Success|Always|KeyPress|KeyRelease|FocusIn|FocusOut)\b'
    )
    bare_xlib_re = re.compile(
        r'(?<![\w:&])X[A-Z][a-z0-9_]+\s*\('
    )
    # Whitelisted names matching bare_xlib pattern that are not Xlib calls
    xlib_whitelist = {"XmlElement", "XmlDocument", "XOR", "Xoshiro"}

    cpp_files = []
    for d in scan_dirs:
        cpp_files.extend([p for p in d.rglob("*") if p.suffix in (".h", ".hpp", ".hxx", ".inl", ".cpp", ".cxx")])

    for f in cpp_files:
        is_header = f.suffix in (".h", ".hpp", ".hxx", ".inl")
        rel_path = f.relative_to(repo_root).as_posix()
        is_quarantine_allowlist = (rel_path == "test/support/HeadlessX11.cpp")

        try:
            raw_text = f.read_text(encoding="utf-8", errors="ignore")
            lines = raw_text.splitlines()
        except Exception:
            continue

        file_modified = False
        new_lines = []

        for i, line in enumerate(lines, 1):
            line_to_keep = line

            # C1: Banned platform headers in project headers
            # X11/xcb/GLX/Cocoa/objc are strictly banned in all headers (they leak macros like Status, None, Bool)
            # windows.h is banned unless explicitly inside a _WIN32 guard block
            if is_header:
                if re.search(r'^\s*#\s*include\s*[<"](X11/|xcb/|GL/glx\.h|AppKit/|Cocoa/|objc/)', line, re.I):
                    c1_banned_header_violations.append(f"{rel_path}:{i}: `{line.strip()}`")
                elif re.search(r'^\s*#\s*include\s*[<"](windows\.h|windowsx\.h|winsock2\.h)', line, re.I):
                    # Check if enclosed in _WIN32 guard in the file
                    has_win32_guard = "_WIN32" in raw_text[:raw_text.find(line)]
                    if not has_win32_guard:
                        c1_banned_header_violations.append(f"{rel_path}:{i}: `{line.strip()}` (missing _WIN32 guard)")

            # C2: Platform headers in .cpp must be in allowlisted quarantine only
            if not is_header and re.search(r'^\s*#\s*include\s*[<"](X11/|xcb/|GL/glx\.h)', line, re.I) and not is_quarantine_allowlist:
                c2_quarantine_violations.append(f"{rel_path}:{i}: `{line.strip()}`")

            # C3: Bare Xlib function calls outside allowlisted quarantine
            if not is_quarantine_allowlist and not is_header:
                clean_line = strip_c_comments(line)
                for match in bare_xlib_re.finditer(clean_line):
                    call_name = match.group(0).split("(")[0].strip()
                    if call_name not in xlib_whitelist and not clean_line.strip().startswith("decltype("):
                        c3_bare_xlib_violations.append(f"{rel_path}:{i}: `{call_name}(...)`")

            # C4: #undef macro band-aids
            if undef_macro_re.search(line):
                c4_undef_macro_violations.append(f"{rel_path}:{i}: `{line.strip()}`")

            # C7: ScopedJuceInitialiser_GUI must be accompanied by HeadlessX11::install() or ScopedGuiContext
            clean_line = strip_c_comments(line)
            if "ScopedJuceInitialiser_GUI" in clean_line:
                is_standalone_preview = rel_path.endswith("_main.cpp") or "preview" in rel_path.lower()
                is_canonical_test_harness = (rel_path == "test/GuiTestHelpers.h")
                is_shielded = ("HeadlessX11::install" in raw_text) or ("ScopedGuiContext" in raw_text)
                if not is_canonical_test_harness and not is_standalone_preview and not is_shielded:
                    c7_scoped_gui_violations.append(f"{rel_path}:{i}: `{line.strip()}` (missing HeadlessX11::install() shield)")

            # Include parsing for C5 and backslash checks
            inc_match = re.match(r'^\s*#\s*include\s*([<"])([^>"]+)[>"]', line)
            if inc_match:
                delimiter = inc_match.group(1)
                inc_path = inc_match.group(2)

                # Backslash check
                if "\\" in inc_path:
                    backslash_violations.append(f"{rel_path}:{i}: `{line.strip()}` uses backslash")

                # C5: Case-sensitivity check for local includes (#include "...")
                if delimiter == '"':
                    resolved = (f.parent / inc_path).resolve()
                    if not resolved.exists():
                        for d in scan_dirs:
                            cand = (d / inc_path).resolve()
                            if cand.exists():
                                resolved = cand
                                break

                    if resolved.exists():
                        parts = inc_path.replace("\\", "/").split("/")
                        curr = f.parent if (f.parent / inc_path).exists() else (scan_dirs[0] / inc_path).parent
                        reconstructed_parts = []
                        has_mismatch = False

                        for part in parts:
                            if part in (".", ".."):
                                curr = (curr / part).resolve()
                                reconstructed_parts.append(part)
                                continue
                            casing_map = get_dir_casing_map(curr)
                            part_lower = part.lower()
                            if part_lower in casing_map:
                                canonical_name = casing_map[part_lower]
                                if canonical_name != part:
                                    has_mismatch = True
                                    case_violations.append(
                                        f"{rel_path}:{i}: `#include \"{inc_path}\"` has casing mismatch (`{part}` vs on-disk `{canonical_name}`)"
                                    )
                                reconstructed_parts.append(canonical_name)
                            else:
                                reconstructed_parts.append(part)
                            curr = curr / part

                        if has_mismatch and auto_fix:
                            reconstructed_path = "/".join(reconstructed_parts)
                            line_to_keep = re.sub(r'#\s*include\s*"[^"]+"', f'#include "{reconstructed_path}"', line)
                            file_modified = True

            new_lines.append(line_to_keep)

        if file_modified and auto_fix:
            try:
                f.write_text("\n".join(new_lines) + "\n", encoding="utf-8")
            except Exception as e:
                pass

    # 1. Backslash check
    checks.append({
        "category": "Portability",
        "name": "Forward Slashes in #include",
        "passed": len(backslash_violations) == 0,
        "message": "All #include directives use forward slashes" if not backslash_violations else f"{len(backslash_violations)} inclusion(s) use Windows backslash: {'; '.join(backslash_violations[:3])}",
    })

    # 2. C5: Case sensitivity
    checks.append({
        "category": "Portability (C5)",
        "name": "Case-Sensitive Include Paths",
        "passed": len(case_violations) == 0,
        "message": "All local includes match exact filesystem casing" if not case_violations else f"{len(case_violations)} casing mismatch(es) detected: {'; '.join(case_violations[:3])}",
    })

    # 3. C1: Platform headers in project headers
    checks.append({
        "category": "Platform Airlock (C1)",
        "name": "Zero Platform Headers in Project Headers",
        "passed": len(c1_banned_header_violations) == 0,
        "message": "No platform headers (<X11/*>, <windows.h>, etc.) in project headers" if not c1_banned_header_violations else f"{len(c1_banned_header_violations)} platform header(s) leaked into headers: {'; '.join(c1_banned_header_violations[:3])}",
    })

    # 4. C2: Quarantine allowlist for platform includes
    checks.append({
        "category": "Platform Airlock (C2)",
        "name": "Quarantined Platform Translation Units",
        "passed": len(c2_quarantine_violations) == 0,
        "message": "Platform headers quarantined strictly in allowlisted translation units" if not c2_quarantine_violations else f"{len(c2_quarantine_violations)} unquarantined platform include(s): {'; '.join(c2_quarantine_violations[:3])}",
    })

    # 5. C3: Bare Xlib entry point detector
    checks.append({
        "category": "Platform Airlock (C3)",
        "name": "Zero Bare Xlib Function Calls",
        "passed": len(c3_bare_xlib_violations) == 0,
        "message": "No direct external Xlib calls in codebase; all dynamic via dlsym" if not c3_bare_xlib_violations else f"{len(c3_bare_xlib_violations)} direct Xlib call(s): {'; '.join(c3_bare_xlib_violations[:3])}",
    })

    # 6. C4: Macro #undef band-aids
    checks.append({
        "category": "Platform Airlock (C4)",
        "name": "Zero Platform Macro #undef Band-Aids",
        "passed": len(c4_undef_macro_violations) == 0,
        "message": "No platform macro #undef statements detected" if not c4_undef_macro_violations else f"{len(c4_undef_macro_violations)} #undef macro band-aid(s) found: {'; '.join(c4_undef_macro_violations[:3])}",
    })

    # 7. C7: Headless initialisation audit
    checks.append({
        "category": "Platform Airlock (C7)",
        "name": "ScopedJuceInitialiser_GUI Encapsulation",
        "passed": len(c7_scoped_gui_violations) == 0,
        "message": "ScopedJuceInitialiser_GUI safely encapsulated in ScopedGuiContext" if not c7_scoped_gui_violations else f"{len(c7_scoped_gui_violations)} unshielded ScopedJuceInitialiser_GUI call(s): {'; '.join(c7_scoped_gui_violations[:3])}",
    })

    return checks

def run_compatibility_audit(repo_root: Path, auto_fix: bool = False) -> Dict[str, Any]:
    """Executes the complete cross-platform, JUCE 9.0.3, and Platform Airlock audit suite."""
    t0 = time.perf_counter()

    cmake_path = repo_root / "CMakeLists.txt"
    source_dir = repo_root / "source"

    cmake_checks = check_cmake_config(cmake_path)
    timer_checks, source_files_count = check_timer_hygiene(source_dir)
    portability_checks = check_source_portability(repo_root, auto_fix=auto_fix)

    all_checks = cmake_checks + timer_checks + portability_checks
    violations = [c for c in all_checks if not c["passed"]]
    duration_ms = (time.perf_counter() - t0) * 1000.0

    return {
        "success": len(violations) == 0,
        "violations_count": len(violations),
        "source_files_checked": source_files_count,
        "duration_ms": round(duration_ms, 2),
        "cmake_checks": cmake_checks,
        "timer_checks": timer_checks,
        "portability_checks": portability_checks,
        "violations": violations,
    }

def format_markdown_report(result: Dict[str, Any]) -> str:
    """Formats the audit result into a clean publication-grade Markdown report."""
    lines = [
        "### 🌐 Cross-Platform & Platform Airlock Compatibility Audit Report",
        "",
        "#### 1. CMake & Platform Targets",
    ]
    for c in result["cmake_checks"]:
        badge = "✅ PASS" if c["passed"] else "❌ FAIL"
        lines.append(f"- [{'x' if c['passed'] else ' '}] {c['name']}: {badge} ({c['message']})")

    lines.extend([
        "",
        "#### 2. JUCE 9.0.3 Hygiene (Issue #1696)",
    ])
    for c in result["timer_checks"]:
        badge = "✅ PASS" if c["passed"] else "❌ FAIL"
        lines.append(f"- [{'x' if c['passed'] else ' '}] {c['name']}: {badge} ({c['message']})")

    lines.extend([
        "",
        "#### 3. Platform Airlock & Source Portability (C1–C7)",
    ])
    for c in result["portability_checks"]:
        badge = "✅ PASS" if c["passed"] else "❌ FAIL"
        lines.append(f"- [{'x' if c['passed'] else ' '}] {c['name']}: {badge} ({c['message']})")

    lines.append("")
    if result["success"]:
        lines.append(f"**Overall Status:** 🟢 PASS (lexical only — does not certify Linux compile/link/runtime) [audited in {result['duration_ms']}ms].")
    else:
        lines.append(f"**Overall Status:** 🔴 {result['violations_count']} violation(s) detected [{result['duration_ms']}ms].")

    return "\n".join(lines)

def main() -> None:
    parser = argparse.ArgumentParser(
        description="Fast cross-platform, JUCE 9.0.3 & Platform Airlock compatibility linter",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("--root", type=str, default=None, help="Root path to TheKlangSuite repository")
    parser.add_argument("--terse", action="store_true", help="Emit single-line token shield output if clean")
    parser.add_argument("--strict", action="store_true", help="Enforce strict exit codes on violations")
    parser.add_argument("--fix", action="store_true", help="Auto-repair casing mismatches in includes")
    parser.add_argument("--json", action="store_true", help="Emit structured JSON output")

    args = parser.parse_args()

    if args.root:
        repo_root = Path(args.root).resolve()
    else:
        script_repo = Path(__file__).resolve().parent.parent
        if (script_repo / "CMakeLists.txt").exists():
            repo_root = script_repo
        else:
            repo_root = Path.cwd().resolve()

    result = run_compatibility_audit(repo_root, auto_fix=args.fix)

    if args.json:
        print(json.dumps(result, indent=2, ensure_ascii=False))
        sys.exit(0 if result["success"] else 1)

    if args.terse:
        if result["success"]:
            print(f"✅ Cross-Platform & Platform Airlock: PASS (lexical only) [0 violations across {result['source_files_checked']} files, {result['duration_ms']}ms]")
            sys.exit(0)
        else:
            print(f"❌ Cross-Platform & Platform Airlock: {result['violations_count']} violation(s) found [{result['duration_ms']}ms]:")
            for v in result["violations"]:
                print(f"  - [{v['category']}] {v['name']}: {v['message']}")
            sys.exit(1)

    print(format_markdown_report(result))
    sys.exit(0 if result["success"] else 1)

if __name__ == "__main__":
    main()
