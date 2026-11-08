#!/usr/bin/env python3
"""
tools/context_extract.py — Zero-Token Context Snapshot Compiler for External LLM Planning
==========================================================================================
Assembles a clean, self-contained planning dossier (context_snapshot.md) from
an extract_clues.md manifest or CLI targets, copies to clipboard (-c), and launches
Gemini Web (-o) in <10ms with zero LLM token consumption.

Usage:
    python tools/context_extract.py [-p] [-c] [-o] [--scope <name>] [--files <f1> <f2>...]
"""

import argparse
from datetime import datetime
import os
from pathlib import Path
import re
import subprocess
import sys
import time
from typing import Any, Dict, List, Optional, Set, Tuple
import webbrowser

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

IGNORED_DIRS: Set[str] = {
    ".git", ".agents", "build", "cmake-build-debug", "cmake-build-release",
    "cmake-build-*", "JuceLibraryCode", "node_modules", "__pycache__",
    ".obsidian", ".vs", "bin", "obj", "assets", "JUCE", ".idea", ".vscode"
}

def strip_license_header(content: str) -> str:
    """Strips leading license / boilerplate comment blocks to save tokens."""
    # Look for leading C/C++ or Markdown comments mentioning copyright/license
    c_comment = re.match(r"^\s*(/\*.*?\*/|//.*?(\r?\n))+", content, flags=re.DOTALL)
    if c_comment:
        chunk = c_comment.group(0).lower()
        if any(w in chunk for w in ("license", "copyright", "all rights reserved", "mit", "apache", "gpl")):
            return content[c_comment.end():].lstrip()
    return content

def build_shallow_tree(root_dir: Path, max_depth: int = 2, current_depth: int = 0, prefix: str = "") -> List[str]:
    """Generates a clean, shallow text directory tree excluding noise folders."""
    if current_depth >= max_depth:
        return []

    lines: List[str] = []
    try:
        entries = sorted(
            [e for e in root_dir.iterdir() if e.name not in IGNORED_DIRS and not e.name.startswith(".")],
            key=lambda x: (not x.is_dir(), x.name.lower())
        )
    except Exception:
        return lines

    for i, entry in enumerate(entries):
        is_last = (i == len(entries) - 1)
        connector = "└── " if is_last else "├── "
        lines.append(f"{prefix}{connector}{entry.name}{'/' if entry.is_dir() else ''}")

        if entry.is_dir() and current_depth + 1 < max_depth:
            extension = "    " if is_last else "│   "
            sub_lines = build_shallow_tree(entry, max_depth, current_depth + 1, prefix + extension)
            lines.extend(sub_lines)

    return lines

def parse_extract_clues(clues_path: Path, target_scope: str = "active") -> Dict[str, Any]:
    """Parses extract_clues.md manifest into structured scope entries."""
    if not clues_path.exists():
        return {
            "title": "Default Project Scope",
            "spec": "",
            "core_interfaces": [],
            "target_files": [],
        }

    text = clues_path.read_text(encoding="utf-8", errors="ignore")
    sections = re.split(r"(?m)^##\s+", text)

    matched_section: Optional[str] = None
    target_clean = target_scope.lower().strip()

    for sec in sections[1:]:
        header_line = sec.splitlines()[0].lower()
        if target_clean in ("active", "default") and "active scope" in header_line:
            matched_section = sec
            break
        elif target_clean in header_line:
            matched_section = sec
            break

    # Fallback to the first section if target not found
    if not matched_section and len(sections) > 1:
        matched_section = sections[1]

    if not matched_section:
        return {
            "title": "Default Project Scope",
            "spec": "",
            "core_interfaces": [],
            "target_files": [],
        }

    lines = matched_section.splitlines()
    header_title = lines[0].strip()

    title = header_title
    spec = ""
    core_interfaces: List[str] = []
    target_files: List[str] = []

    current_list: Optional[List[str]] = None

    for line in lines[1:]:
        line_str = line.strip()
        if not line_str:
            continue

        title_m = re.match(r"-\s*\*\*Title:\*\*\s*(.+)", line_str, re.IGNORECASE)
        if title_m:
            title = title_m.group(1).strip()
            current_list = None
            continue

        spec_m = re.match(r"-\s*\*\*Spec:\*\*\s*(.+)", line_str, re.IGNORECASE)
        if spec_m:
            spec = spec_m.group(1).strip().replace("`", "")
            current_list = None
            continue

        if re.match(r"-\s*\*\*Core Interfaces:\*\*", line_str, re.IGNORECASE):
            current_list = core_interfaces
            continue
        elif re.match(r"-\s*\*\*Target Files:\*\*", line_str, re.IGNORECASE):
            current_list = target_files
            continue
        elif re.match(r"-\s*\*\*", line_str):
            current_list = None
            continue

        if current_list is not None and (line_str.startswith("- ") or line_str.startswith("* ")):
            item = line_str[2:].strip().replace("`", "")
            if item:
                current_list.append(item)

    return {
        "title": title,
        "spec": spec,
        "core_interfaces": core_interfaces,
        "target_files": target_files,
    }

def detect_spec_file(repo_root: Path) -> Optional[Path]:
    """Finds the primary specification or architecture document in the repo."""
    candidates = [
        repo_root / "docs" / "specs" / "master_sound_and_fx_catalog.md",
        repo_root / "docs" / "specs" / "fx_catalog_expansion.md",
        repo_root / "docs" / "FOCUS.md",
        repo_root / "README.md",
    ]
    for c in candidates:
        if c.exists():
            return c
    specs = list((repo_root / "docs").glob("**/*SPEC*.md")) + list(repo_root.glob("*SPEC*.md"))
    return specs[0] if specs else None

def get_code_language(file_path: Path) -> str:
    """Maps file extension to markdown code fence language."""
    ext = file_path.suffix.lower()
    return {
        ".h": "cpp", ".hpp": "cpp", ".cpp": "cpp", ".cxx": "cpp", ".c": "c",
        ".py": "python", ".js": "javascript", ".mjs": "javascript", ".ts": "typescript",
        ".html": "html", ".css": "css", ".json": "json", ".cmake": "cmake",
        ".md": "markdown", ".sh": "bash", ".ps1": "powershell"
    }.get(ext, "")

def compile_context_snapshot(
    repo_root: Path,
    scope_data: Dict[str, Any],
    direct_plan_mode: bool = False,
    override_spec: Optional[str] = None,
    override_files: Optional[List[str]] = None,
    tree_depth: int = 2,
) -> Tuple[str, Dict[str, Any]]:
    """Assembles the complete Markdown context snapshot."""
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M")
    mode_str = "Direct Plan (--plan / -p)" if direct_plan_mode else "Conversational Discovery (Default)"

    lines: List[str] = [
        "# Planning Context Snapshot",
        f"**Timestamp:** {timestamp}  ",
        f"**Scope:** {scope_data.get('title', 'Project Context')}  ",
        f"**Mode:** {mode_str}",
        "",
    ]

    # Instruction Banner for Gemini Web
    if direct_plan_mode:
        lines.extend([
            "> [!IMPORTANT]",
            "> **INSTRUCTIONS FOR GEMINI (DIRECT PLAN MODE):**",
            "> - The task context below is 100% self-contained.",
            "> - Immediately produce a clean, phased implementation plan formatted for Antigravity's `/pasteplan` skill.",
            "> - Structure the plan with clear milestones, file paths, acceptance criteria, and verification commands.",
            "",
        ])
    else:
        lines.extend([
            "> [!IMPORTANT]",
            "> **INSTRUCTIONS FOR GEMINI (CONVERSATIONAL DISCOVERY MODE):**",
            "> - **DO NOT jump straight into writing the final implementation plan.**",
            "> - The user wants to explore and brainstorm this feature with you first.",
            "> - Review the codebase context below, discuss architectural design options and trade-offs, ask clarifying questions about UX/DSP details, and conduct an interactive design interview.",
            "> - Only output the structured, phased implementation plan (formatted for Antigravity's `/pasteplan` skill) when the user explicitly directs you to generate the plan.",
            "",
        ])

    # Section 1: Project Specification & Architecture
    spec_path_str = override_spec or scope_data.get("spec", "")
    spec_path = (repo_root / spec_path_str).resolve() if spec_path_str else detect_spec_file(repo_root)

    lines.append("## 1. Project Specification & Architecture")
    if spec_path and spec_path.exists():
        rel_spec = spec_path.relative_to(repo_root) if spec_path.is_relative_to(repo_root) else spec_path
        lines.append(f"*(Source: `{rel_spec}`)*\n")
        spec_content = spec_path.read_text(encoding="utf-8", errors="ignore")
        lines.append(spec_content.strip())
    else:
        lines.append("*No primary specification document provided.*")
    lines.append("")

    # Section 2: Directory Tree
    lines.append("## 2. Directory Tree")
    tree_lines = build_shallow_tree(repo_root, max_depth=tree_depth)
    lines.append("```text")
    lines.append(f"{repo_root.name}/")
    lines.extend(tree_lines)
    lines.append("```\n")

    # Section 3: Core Schemas, Structs & Interfaces
    core_interfaces = scope_data.get("core_interfaces", [])
    if core_interfaces:
        lines.append("## 3. Core Schemas, Structs & Interfaces")
        for item in core_interfaces:
            p = (repo_root / item).resolve()
            if p.exists() and p.is_file():
                rel = p.relative_to(repo_root) if p.is_relative_to(repo_root) else p
                lang = get_code_language(p)
                content = strip_license_header(p.read_text(encoding="utf-8", errors="ignore"))
                lines.append(f"### `{rel}`")
                lines.append(f"```{lang}\n{content.strip()}\n```\n")
            else:
                lines.append(f"<!-- Interface file not found on disk: {item} -->")

    # Section 4: Target Source Files
    target_files = override_files or scope_data.get("target_files", [])
    files_included = 0
    if target_files:
        lines.append("## 4. Target Source Files")
        for item in target_files:
            p = (repo_root / item).resolve()
            if p.exists() and p.is_file():
                rel = p.relative_to(repo_root) if p.is_relative_to(repo_root) else p
                lang = get_code_language(p)
                content = strip_license_header(p.read_text(encoding="utf-8", errors="ignore"))
                lines.append(f"### `{rel}`")
                lines.append(f"```{lang}\n{content.strip()}\n```\n")
                files_included += 1
            else:
                lines.append(f"<!-- Source file not found on disk: {item} -->")

    full_markdown = "\n".join(lines)
    metadata = {
        "timestamp": timestamp,
        "mode": mode_str,
        "spec_file": str(spec_path) if spec_path else None,
        "interfaces_count": len(core_interfaces),
        "source_files_count": files_included,
        "char_count": len(full_markdown),
        "size_kb": round(len(full_markdown.encode("utf-8")) / 1024.0, 1),
    }

    return full_markdown, metadata

def copy_file_to_clipboard(file_path: Path) -> bool:
    """Copies file contents directly to OS clipboard across Windows, macOS, and Linux."""
    try:
        if sys.platform == "win32":
            cmd = f'Get-Content -LiteralPath "{file_path}" -Raw -Encoding utf8 | Set-Clipboard'
            res = subprocess.run(["powershell", "-NoProfile", "-Command", cmd], capture_output=True, timeout=5)
            return res.returncode == 0
        elif sys.platform == "darwin":
            with open(file_path, "rb") as f:
                res = subprocess.run(["pbcopy"], stdin=f, timeout=5)
                return res.returncode == 0
        else:
            # Linux: try wl-copy then xclip
            try:
                with open(file_path, "rb") as f:
                    if subprocess.run(["wl-copy"], stdin=f, timeout=5).returncode == 0:
                        return True
            except FileNotFoundError:
                pass
            with open(file_path, "rb") as f:
                return subprocess.run(["xclip", "-selection", "clipboard"], stdin=f, timeout=5).returncode == 0
    except Exception:
        return False

def print_reminder_banner() -> None:
    """Emits the high-visibility Pro Model verification banner."""
    print("\n" + "=" * 76)
    print(" ⚠️   REMINDER: VERIFY / SELECT THE PRO MODEL IN GEMINI WEB (gemini.google.com)")
    print("=" * 76 + "\n")

def main() -> None:
    t0 = time.perf_counter()

    parser = argparse.ArgumentParser(
        description="Zero-token context snapshot compiler for external LLM planning",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("-p", "--plan", action="store_true", help="Direct Plan Mode (instructs Gemini to output plan immediately)")
    parser.add_argument("-c", "--clipboard", action="store_true", help="Auto-copy snapshot to system clipboard")
    parser.add_argument("-o", "--open", action="store_true", help="Auto-launch https://gemini.google.com in default browser")
    parser.add_argument("--scope", type=str, default="active", help="Scope name in extract_clues.md (e.g. active, oscillators, shell)")
    parser.add_argument("--files", nargs="*", default=None, help="Explicit target file path(s) to include")
    parser.add_argument("--spec", type=str, default=None, help="Explicit spec document path to include")
    parser.add_argument("--clues", type=str, default=None, help="Path to extract_clues.md manifest")
    parser.add_argument("--root", type=str, default=None, help="Root repository path")
    parser.add_argument("--output", type=str, default=None, help="Output markdown path (default: context_snapshot.md)")
    parser.add_argument("--depth", type=int, default=2, help="Directory tree depth (default: 2)")
    parser.add_argument("--terse", action="store_true", help="Emit single-line summary output")

    args = parser.parse_args()

    # Determine repository root
    if args.root:
        repo_root = Path(args.root).resolve()
    else:
        script_repo = Path(__file__).resolve().parent.parent
        repo_root = script_repo if (script_repo / "CMakeLists.txt").exists() or (script_repo / ".git").exists() else Path.cwd().resolve()

    # Locate extract_clues.md
    if args.clues:
        clues_path = Path(args.clues).resolve()
    else:
        candidates = [
            repo_root / ".agents" / "extract_clues.md",
            repo_root / "extract_clues.md",
            repo_root / "docs" / "extract_clues.md",
        ]
        clues_path = next((c for c in candidates if c.exists()), candidates[0])

    scope_data = parse_extract_clues(clues_path, target_scope=args.scope)

    # Compile the snapshot markdown
    content, meta = compile_context_snapshot(
        repo_root=repo_root,
        scope_data=scope_data,
        direct_plan_mode=args.plan,
        override_spec=args.spec,
        override_files=args.files,
        tree_depth=args.depth,
    )

    output_path = Path(args.output).resolve() if args.output else (repo_root / "context_snapshot.md")
    output_path.write_text(content, encoding="utf-8")

    duration_ms = round((time.perf_counter() - t0) * 1000.0, 2)

    # Handle clipboard action
    copied = False
    if args.clipboard:
        copied = copy_file_to_clipboard(output_path)

    # Handle browser launch
    opened = False
    if args.open:
        opened = webbrowser.open("https://gemini.google.com")

    if args.terse:
        status_parts = [
            f"✅ Context snapshot compiled to {output_path.name}",
            f"({meta['size_kb']} KB, {meta['source_files_count']} files)",
            f"[{duration_ms}ms]"
        ]
        if copied:
            status_parts.append("[📋 Copied]")
        if opened:
            status_parts.append("[🌐 Browser Opened]")
        print(" ".join(status_parts))
        sys.exit(0)

    print(f"📦 Context Snapshot Compiled: {output_path}")
    print(f"   - Scope:       {scope_data.get('title', 'Custom Scope')}")
    print(f"   - Mode:        {meta['mode']}")
    print(f"   - Size:        {meta['size_kb']} KB ({meta['char_count']:,} chars)")
    print(f"   - Files:       {meta['source_files_count']} source files, {meta['interfaces_count']} interfaces")
    print(f"   - Spec:        {meta['spec_file'] or 'None'}")
    print(f"   - Duration:    {duration_ms}ms")

    if copied:
        print("   - Clipboard:   ✅ Successfully copied to OS clipboard")
    elif args.clipboard:
        print("   - Clipboard:   ⚠️ Failed to copy to clipboard")

    if opened:
        print("   - Browser:     🌐 Launched https://gemini.google.com")

    print_reminder_banner()

if __name__ == "__main__":
    main()
