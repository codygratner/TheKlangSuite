#!/usr/bin/env python3
"""
tools/lint_frontmatter.py — Universal Vault & Docs Frontmatter Auditor and Fixer
================================================================================
Audits Markdown files across the vault and documentation trees for compliant
YAML frontmatter (title, date, tags, status). Automatically backfills missing
keys without altering or truncating note bodies.
Zero external dependencies; sub-5ms native execution for 0 tokens.
"""

import argparse
import datetime
import json
import os
import re
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional, Set, Tuple

# Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

# Common aliases for baseline fields
KEY_ALIASES = {
    "title": {"title", "id", "name"},
    "date": {"date", "created", "created_at", "datetime"},
    "tags": {"tags", "categories", "keywords"},
    "status": {"status", "type", "state"},
}

DEFAULT_EXCLUDES = {
    ".git",
    ".obsidian",
    ".agents",
    "build",
    "node_modules",
    ".system_generated",
    "target",
    "dist",
}


def parse_frontmatter(content: str) -> Tuple[Optional[Dict[str, Any]], str, str, str]:
    """Parses frontmatter from content.
    Returns (meta_dict, raw_frontmatter, body, newline_char).
    If no frontmatter exists, returns (None, '', content, newline_char).
    """
    newline = "\r\n" if "\r\n" in content[:200] else "\n"
    lines = content.splitlines(keepends=True)

    if not lines or not lines[0].strip() == "---":
        return None, "", content, newline

    closing_idx = -1
    for i in range(1, len(lines)):
        if lines[i].strip() == "---":
            closing_idx = i
            break

    if closing_idx == -1:
        return None, "", content, newline

    raw_fm_lines = lines[1:closing_idx]
    body = "".join(lines[closing_idx + 1:])
    raw_fm = "".join(raw_fm_lines)

    meta = parse_yaml_lines(raw_fm_lines)
    return meta, raw_fm, body, newline


def parse_yaml_lines(lines: List[str]) -> Dict[str, Any]:
    """Parses simple YAML key-value pairs and bullet lists without external dependencies."""
    meta: Dict[str, Any] = {}
    current_list_key = None

    for line in lines:
        stripped = line.strip()
        if not stripped or stripped.startswith("#"):
            continue

        # Check for bullet list item under a key
        if stripped.startswith("- ") and current_list_key:
            item = stripped[2:].strip().strip('"').strip("'")
            if isinstance(meta.get(current_list_key), list):
                meta[current_list_key].append(item)
            continue

        if ":" in line:
            key, val = line.split(":", 1)
            key = key.strip()
            val = val.strip()

            if not val:
                # Potential block list
                current_list_key = key
                meta[key] = []
                continue
            else:
                current_list_key = None

            # Handle inline list [a, b, c]
            if val.startswith("[") and val.endswith("]"):
                items = [it.strip().strip('"').strip("'") for it in val[1:-1].split(",") if it.strip()]
                meta[key] = items
            else:
                # Scalar value
                clean_val = val.strip('"').strip("'")
                meta[key] = clean_val

    return meta


def derive_title(path: Path, body: str) -> str:
    """Derives a title from the first H1 header in the body, or the filename stem."""
    h1_match = re.search(r"^#\s+(.+)$", body, re.MULTILINE)
    if h1_match:
        t = h1_match.group(1).strip()
        # Clean markdown formatting like bold/italics
        t = re.sub(r"[*_`]", "", t).strip()
        if t:
            return t
    stem = path.stem
    # If stem has date prefix like 2026-10-12_Title, strip date prefix for title
    date_prefix = re.match(r"^\d{4}-\d{2}-\d{2}[-_ ]+(.+)$", stem)
    if date_prefix:
        stem = date_prefix.group(1)
    return stem.replace("_", " ").strip()


def derive_date(path: Path, stem: str) -> str:
    """Derives date from filename prefix or file mtime."""
    date_match = re.match(r"^(\d{4}-\d{2}-\d{2})", stem)
    if date_match:
        return date_match.group(1)
    try:
        mtime = path.stat().st_mtime
        return datetime.date.fromtimestamp(mtime).isoformat()
    except Exception:
        return datetime.date.today().isoformat()


def has_key_alias(meta: Dict[str, Any], required_key: str) -> bool:
    """Checks if meta contains the required key or any of its recognized aliases."""
    aliases = KEY_ALIASES.get(required_key, {required_key})
    return any(a in meta for a in aliases)


def lint_file(
    path: Path,
    required_keys: List[str],
    default_status: str = "untriaged",
    default_tags: Optional[List[str]] = None,
) -> Dict[str, Any]:
    """Audits a single markdown file and determines what frontmatter fixes are needed."""
    if default_tags is None:
        default_tags = ["untriaged"]

    try:
        content = path.read_text(encoding="utf-8", errors="replace")
    except Exception as e:
        return {
            "path": str(path),
            "status": "ERROR",
            "error": str(e),
            "missing_keys": [],
            "has_frontmatter": False,
        }

    meta, raw_fm, body, newline = parse_frontmatter(content)
    has_fm = meta is not None

    missing = []
    for req in required_keys:
        if not has_fm or not has_key_alias(meta, req):
            missing.append(req)

    # Check if tags need normalization (e.g. if present as a single string instead of list)
    needs_tag_normalization = False
    if has_fm and "tags" in meta:
        val = meta["tags"]
        if isinstance(val, str) and ("," in val or val.startswith("#")):
            needs_tag_normalization = True

    is_compliant = has_fm and len(missing) == 0 and not needs_tag_normalization

    return {
        "path": str(path),
        "status": "COMPLIANT" if is_compliant else "NON_COMPLIANT",
        "has_frontmatter": has_fm,
        "missing_keys": missing,
        "needs_tag_normalization": needs_tag_normalization,
        "meta": meta or {},
        "raw_fm": raw_fm,
        "body": body,
        "newline": newline,
    }


def fix_file(
    info: Dict[str, Any],
    default_status: str = "untriaged",
    default_tags: Optional[List[str]] = None,
    dry_run: bool = False,
) -> Tuple[bool, str]:
    """Backfills missing frontmatter keys into the file, preserving body and existing keys."""
    if info["status"] == "COMPLIANT":
        return False, "Already compliant"

    if default_tags is None:
        default_tags = ["untriaged"]

    path = Path(info["path"])
    body = info["body"]
    newline = info.get("newline", "\n")
    missing = info["missing_keys"]
    has_fm = info["has_frontmatter"]
    meta = dict(info.get("meta", {}))

    # Derive missing values
    additions: Dict[str, Any] = {}

    if "title" in missing:
        additions["title"] = derive_title(path, body)

    if "date" in missing:
        additions["date"] = derive_date(path, path.stem)

    if "tags" in missing:
        additions["tags"] = list(default_tags)
    elif info.get("needs_tag_normalization", False):
        cur_tags = meta.get("tags", "")
        if isinstance(cur_tags, str):
            clean_tags = [t.strip().lstrip("#").strip('"').strip("'") for t in re.split(r"[, ]+", cur_tags) if t.strip()]
            meta["tags"] = clean_tags

    if "status" in missing:
        additions["status"] = default_status

    if not has_fm:
        # Build brand new frontmatter
        ordered_keys = ["title", "date", "status", "tags"]
        fm_lines = ["---"]
        for k in ordered_keys:
            v = additions.get(k)
            if v is not None:
                if isinstance(v, list):
                    fm_lines.append(f"{k}: [{', '.join(v)}]")
                else:
                    fm_lines.append(f'{k}: "{v}"')
        # Add any other missing keys
        for k, v in additions.items():
            if k not in ordered_keys:
                if isinstance(v, list):
                    fm_lines.append(f"{k}: [{', '.join(v)}]")
                else:
                    fm_lines.append(f'{k}: "{v}"')
        fm_lines.append("---")
        fm_lines.append("")

        new_content = newline.join(fm_lines) + body.lstrip("\r\n")
    else:
        # Frontmatter exists, insert additions right before closing ---
        raw_fm = info["raw_fm"]
        fm_lines = raw_fm.splitlines()

        # If tag normalization needed, update tag line
        if info.get("needs_tag_normalization", False):
            updated_fm_lines = []
            for line in fm_lines:
                if line.strip().startswith("tags:"):
                    clean_tags = meta["tags"]
                    updated_fm_lines.append(f"tags: [{', '.join(clean_tags)}]")
                else:
                    updated_fm_lines.append(line)
            fm_lines = updated_fm_lines

        # Append missing keys
        for k, v in additions.items():
            if isinstance(v, list):
                fm_lines.append(f"{k}: [{', '.join(v)}]")
            else:
                fm_lines.append(f'{k}: "{v}"')

        new_fm = newline.join(fm_lines)
        new_content = f"---{newline}{new_fm}{newline}---{newline}{body}"

    if dry_run:
        added_desc = ", ".join(f"{k}='{v}'" for k, v in additions.items())
        return True, f"[DRY-RUN] Would add: {added_desc}"

    try:
        path.write_text(new_content, encoding="utf-8")
        added_desc = ", ".join(f"{k}='{v}'" for k, v in additions.items())
        return True, f"Fixed: added {added_desc}"
    except Exception as e:
        return False, f"Write error: {e}"


def collect_markdown_files(
    root: Path,
    excludes: Set[str],
    recursive: bool = True,
) -> List[Path]:
    """Finds all markdown files under root, honoring exclusion directories."""
    files: List[Path] = []
    if not root.exists():
        return files

    if root.is_file():
        if root.suffix.lower() == ".md":
            return [root]
        return []

    iterator = root.rglob("*.md") if recursive else root.glob("*.md")
    for f in iterator:
        # Check if any parent or name matches excludes
        parts = set(p.lower() for p in f.parts)
        if any(exc.lower() in parts for exc in excludes):
            continue
        files.append(f)

    return sorted(files)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="📋 Universal Vault & Docs Frontmatter Auditor & Fixer (Zero-Token Mechanical Clerk)"
    )
    parser.add_argument(
        "--dir",
        "-d",
        type=str,
        default=".",
        help="Target directory to audit (default: current directory)",
    )
    parser.add_argument(
        "--file",
        "-f",
        type=str,
        default=None,
        help="Audit a single markdown file",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="Check only; return exit code 1 if any non-compliant files exist",
    )
    parser.add_argument(
        "--fix",
        action="store_true",
        help="Automatically backfill missing frontmatter keys in-place",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Show what would be fixed without modifying files",
    )
    parser.add_argument(
        "--required-keys",
        type=str,
        default="title,date,tags,status",
        help="Comma-separated list of required baseline keys (default: title,date,tags,status)",
    )
    parser.add_argument(
        "--default-status",
        type=str,
        default="untriaged",
        help="Default status value when missing (default: untriaged)",
    )
    parser.add_argument(
        "--default-tags",
        type=str,
        default="untriaged",
        help="Default comma-separated tags when missing (default: untriaged)",
    )
    parser.add_argument(
        "--exclude",
        type=str,
        default="",
        help="Additional directories to exclude, comma-separated",
    )
    parser.add_argument(
        "--quiet",
        "-q",
        action="store_true",
        help="Suppress compliant files; report only violations and summary",
    )
    parser.add_argument(
        "--json",
        action="store_true",
        help="Output audit scorecard as JSON",
    )

    args = parser.parse_args()

    required_keys = [k.strip().lower() for k in args.required_keys.split(",") if k.strip()]
    default_tags = [t.strip() for t in args.default_tags.split(",") if t.strip()]

    excludes = set(DEFAULT_EXCLUDES)
    if args.exclude:
        for exc in args.exclude.split(","):
            if exc.strip():
                excludes.add(exc.strip())

    if args.file:
        target_path = Path(args.file)
        if not target_path.exists():
            print(f"❌ Error: File '{args.file}' not found.", file=sys.stderr)
            return 1
        files = [target_path]
    else:
        target_dir = Path(args.dir).resolve()
        if not target_dir.exists():
            print(f"❌ Error: Directory '{args.dir}' not found.", file=sys.stderr)
            return 1
        files = collect_markdown_files(target_dir, excludes)

    if not args.quiet and not args.json:
        print("=" * 70)
        print("📋 UNIVERSAL FRONTMATTER AUDITOR & FIXER (Flight 2)")
        print(f"📁 Target Scope: {len(files)} files | Required: {', '.join(required_keys)}")
        print("=" * 70)

    results = []
    compliant_count = 0
    fixed_count = 0
    error_count = 0

    for f in files:
        info = lint_file(
            f,
            required_keys=required_keys,
            default_status=args.default_status,
            default_tags=default_tags,
        )

        if info["status"] == "COMPLIANT":
            compliant_count += 1
            if not args.quiet and not args.json:
                print(f"✅ {f.name}")
        elif info["status"] == "ERROR":
            error_count += 1
            if not args.json:
                print(f"❌ {f.name}: {info.get('error')}", file=sys.stderr)
        else:
            # Non-compliant
            missing_str = ", ".join(info["missing_keys"])
            if args.fix or args.dry_run:
                success, msg = fix_file(
                    info,
                    default_status=args.default_status,
                    default_tags=default_tags,
                    dry_run=args.dry_run,
                )
                if success:
                    fixed_count += 1
                    if not args.json:
                        print(f"🔧 {f.name} ➔ {msg}")
                else:
                    if not args.json:
                        print(f"⚠️ {f.name} ➔ {msg}", file=sys.stderr)
            else:
                if not args.json:
                    print(f"🟡 {f.name} ➔ Missing: [{missing_str}] (Has FM: {info['has_frontmatter']})")

        results.append(info)

    non_compliant_count = len(files) - compliant_count - error_count

    if args.json:
        payload = {
            "total": len(files),
            "compliant": compliant_count,
            "non_compliant": non_compliant_count,
            "fixed": fixed_count,
            "errors": error_count,
            "required_keys": required_keys,
            "files": [
                {
                    "path": r["path"],
                    "status": r["status"],
                    "missing_keys": r.get("missing_keys", []),
                    "has_frontmatter": r.get("has_frontmatter", False),
                }
                for r in results
            ],
        }
        print(json.dumps(payload, indent=2, ensure_ascii=False))
        return 0 if (compliant_count == len(files) or args.fix) else 1

    print("=" * 70)
    print("📊 FRONTMATTER AUDIT SCORECARD:")
    print(f"   • Total Files Audited: {len(files)}")
    print(f"   • Compliant:           {compliant_count}")
    print(f"   • Non-Compliant:       {non_compliant_count}")
    if args.fix or args.dry_run:
        action_word = "Would Fix" if args.dry_run else "Fixed"
        print(f"   • {action_word}:             {fixed_count}")
    if error_count > 0:
        print(f"   • Errors:              {error_count}")
    print("=" * 70)

    if args.check:
        if non_compliant_count > 0 or error_count > 0:
            print("❌ Audit Failed: non-compliant markdown notes detected.", file=sys.stderr)
            return 1
        print("✅ 100% FRONTMATTER AUDIT PASSED: All notes have valid frontmatter.")
        return 0

    if args.fix:
        print(f"✅ Fix Complete: updated {fixed_count} note(s).")
        return 0

    return 0


if __name__ == "__main__":
    sys.exit(main())
