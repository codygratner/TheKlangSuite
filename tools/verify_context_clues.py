#!/usr/bin/env python3
"""
verify_context_clues.py - Automated Context Clues & Link Resolution Validator
Part of The Klang Suite / N'kai Developer Tooling (Topic 6.7).

Scans workspace root for context_clues_*.md and docs/templates/context_clues_template.md,
validating:
1. Presence of all 5 mandatory canonical section headings:
   - 'Active SemVer'
   - 'Return Agenda'
   - 'Architectural Memory'
   - 'Taxonomy'
   - 'Invariants'
2. 100% resolution of local file hyperlinks (file:/// and relative paths).
   - Strips URL fragments (#hash).
   - Skips remote HTTP/HTTPS URLs.
   - Asserts each target exists on the local filesystem.
"""

import sys
import re
import urllib.parse
from pathlib import Path

# Ensure UTF-8 output on Windows consoles
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8", errors="replace")

MANDATORY_SECTIONS = [
    "Active SemVer",
    "Return Agenda",
    "Architectural Memory",
    "Taxonomy",
    "Invariants",
]

LINK_REGEX = re.compile(r'\[([^\]]+)\]\(([^)]+)\)')
HEADING_REGEX = re.compile(r'^#{1,6}\s+(.+)$')

def check_file(file_path: Path, workspace_root: Path) -> dict:
    rel_path = file_path.relative_to(workspace_root)
    result = {
        "file": str(rel_path),
        "is_active_context": file_path.name == "ACTIVE_CONTEXT.md",
        "missing_sections": [],
        "broken_links": [],
        "total_links": 0,
        "local_links": 0,
        "remote_links": 0,
    }

    try:
        content = file_path.read_text(encoding="utf-8")
    except Exception as e:
        result["error"] = f"Failed to read file: {e}"
        return result

    lines = content.splitlines()

    # 1. Section Heading Validation (only for legacy context clues or templates)
    if not result["is_active_context"]:
        headings = []
        for line in lines:
            m = HEADING_REGEX.match(line.strip())
            if m:
                headings.append(m.group(1))

        for sec in MANDATORY_SECTIONS:
            found = any(sec.lower() in h.lower() for h in headings)
            if not found:
                result["missing_sections"].append(sec)
    else:
        if not any("Active Workspace Context" in line for line in lines):
            result["missing_sections"].append("Active Workspace Context")

    # 2. Hyperlink Resolution
    for line_num, line in enumerate(lines, start=1):
        for match in LINK_REGEX.finditer(line):
            result["total_links"] += 1
            label = match.group(1)
            raw_target = match.group(2).strip()

            # Skip markdown internal anchor links (#heading)
            if raw_target.startswith("#"):
                continue

            # Skip remote HTTP/HTTPS links
            if raw_target.startswith("http://") or raw_target.startswith("https://"):
                result["remote_links"] += 1
                continue

            result["local_links"] += 1

            # Handle file:/// URIs
            target_path_str = raw_target
            if target_path_str.startswith("file:///"):
                target_path_str = target_path_str[8:]
            elif target_path_str.startswith("file://"):
                target_path_str = target_path_str[7:]

            # Unquote percent-encoding (e.g. %20 -> space)
            target_path_str = urllib.parse.unquote(target_path_str)

            # Strip fragments (#anchor)
            if "#" in target_path_str:
                target_path_str = target_path_str.split("#", 1)[0]

            # If empty after fragment strip (e.g. file:///#anchor)
            if not target_path_str:
                continue

            target_path = Path(target_path_str)

            # Resolve path: absolute, relative to current file, or relative to workspace root
            resolved = None
            if target_path.is_absolute():
                resolved = target_path
            else:
                # Try relative to the file first
                candidate = (file_path.parent / target_path).resolve()
                if candidate.exists():
                    resolved = candidate
                else:
                    # Try relative to workspace root
                    candidate_root = (workspace_root / target_path).resolve()
                    if candidate_root.exists():
                        resolved = candidate_root
                    else:
                        resolved = candidate

            if not resolved.exists():
                result["broken_links"].append({
                    "line": line_num,
                    "label": label,
                    "target": raw_target,
                    "resolved": str(resolved),
                })

    return result

def main() -> int:
    # Workspace root is parent of tools/ directory or current working directory
    script_dir = Path(__file__).resolve().parent
    workspace_root = script_dir.parent

    print("=" * 70)
    print("🔍 Context Clues & Link Resolution Validator (Topic 6.7)")
    print(f"📁 Workspace Root: {workspace_root}")
    print("=" * 70)

    # Locate all context clues files in root, plus ACTIVE_CONTEXT.md
    target_files = sorted(workspace_root.glob("context_clues_*.md"))
    active_context = workspace_root / "ACTIVE_CONTEXT.md"
    if active_context.exists():
        target_files.insert(0, active_context)

    # Also include canonical template if present
    template_path = workspace_root / "docs" / "templates" / "context_clues_template.md"
    if template_path.exists():
        target_files.append(template_path)

    if not target_files:
        print("❌ Error: Neither ACTIVE_CONTEXT.md nor context_clues_*.md found in workspace root.")
        return 1

    total_files = len(target_files)
    failed_files = 0
    total_links_checked = 0
    total_broken = 0

    for f in target_files:
        res = check_file(f, workspace_root)
        file_name = res["file"]
        missing_sec = res["missing_sections"]
        broken = res["broken_links"]
        total_links_checked += res["local_links"]
        total_broken += len(broken)

        has_errors = bool(missing_sec or broken or "error" in res)
        status_icon = "❌ FAIL" if has_errors else "✅ PASS"

        print(f"\n[{status_icon}] {file_name}")
        if res["is_active_context"]:
            print(f"  • Format: 0-Turn Cold Boot Header verified")
        else:
            print(f"  • Headings: {5 - len(missing_sec)}/5 mandatory sections present")
        print(f"  • Links: {res['local_links']} local files checked ({res['remote_links']} remote skipped)")

        if missing_sec:
            failed_files += 1
            print(f"  ⚠️  Missing Mandatory Sections:")
            for s in missing_sec:
                print(f"      - {s}")

        if broken:
            if not missing_sec:
                failed_files += 1
            print(f"  ⚠️  Broken Hyperlinks ({len(broken)}):")
            for b in broken:
                print(f"      - Line {b['line']}: [{b['label']}]({b['target']})")
                print(f"        -> Resolved to non-existent: {b['resolved']}")

    print("\n" + "=" * 70)
    print(f"📊 SUMMARY: {total_files - failed_files}/{total_files} files passed.")
    print(f"🔗 Local Links: {total_links_checked - total_broken}/{total_links_checked} resolved successfully.")

    if failed_files > 0:
        print(f"❌ VALIDATION FAILED: {failed_files} file(s) have errors.")
        print("=" * 70)
        return 1
    else:
        print("✅ VALIDATION SUCCESS: 100% compliant sections and zero broken links.")
        print("=" * 70)
        return 0

if __name__ == "__main__":
    sys.exit(main())
