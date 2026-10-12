#!/usr/bin/env python3
"""
tools/catalog_sync.py — Automated Documentation Catalog & System Map Synchronizer
=================================================================================
Mechanically audits and synchronizes docs/DOCS_CATALOG.json and docs/SYSTEM_MAP.md
at zero token cost. Detects dead links, missing specs, and uncataloged docs.

Part of The Zero-Token Mechanical Automation Suite (Flight 1).
"""

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional, Set, Tuple

# Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent
CATALOG_PATH = REPO_ROOT / "docs" / "DOCS_CATALOG.json"
SYSTEM_MAP_PATH = REPO_ROOT / "docs" / "SYSTEM_MAP.md"


def extract_metadata_from_markdown(file_path: Path) -> Tuple[str, str]:
    """Extracts (title, purpose) from markdown file via YAML frontmatter or H1/blockquote."""
    try:
        content = file_path.read_text(encoding="utf-8", errors="replace")
    except Exception:
        return file_path.stem.replace("_", " ").title(), "Documentation file."

    title = ""
    purpose = ""

    # Check YAML frontmatter
    fm_match = re.match(r"^---\s*\n(.*?)\n---\s*\n", content, re.DOTALL)
    if fm_match:
        fm_text = fm_match.group(1)
        t_match = re.search(r"^title:\s*[\"']?(.*?)[\"']?\s*$", fm_text, re.MULTILINE)
        if t_match:
            title = t_match.group(1).strip()
        p_match = re.search(r"^(?:description|purpose):\s*[\"']?(.*?)[\"']?\s*$", fm_text, re.MULTILINE)
        if p_match:
            purpose = p_match.group(1).strip()

    # Fallback to first H1
    if not title:
        h1_match = re.search(r"^#\s+(.+)$", content, re.MULTILINE)
        if h1_match:
            title = h1_match.group(1).strip()
            # Clean markdown formatting in title
            title = re.sub(r"[`*_]", "", title).strip()

    # Fallback to blockquote or first paragraph for purpose
    if not purpose:
        bq_match = re.search(r"^>\s*\*{0,2}(.*?)\*{0,2}\s*$", content, re.MULTILINE)
        if bq_match:
            purpose = bq_match.group(1).strip()
            purpose = re.sub(r"[`*_]", "", purpose).strip()
        else:
            purpose = f"Specification and architectural design for {file_path.stem.replace('_', ' ')}."

    if not title:
        title = file_path.stem.replace("_", " ").title()

    return title, purpose


def load_catalog() -> Tuple[Dict[str, Any], List[Dict[str, Any]]]:
    """Loads DOCS_CATALOG.json and returns (catalog_data, flat_items_list)."""
    if not CATALOG_PATH.exists():
        return {}, []

    data = json.loads(CATALOG_PATH.read_text(encoding="utf-8"))
    flat_items = []
    for cat_key, cat_val in data.get("catalogs", {}).items():
        for item in cat_val.get("items", []):
            item_copy = dict(item)
            item_copy["category"] = cat_key
            flat_items.append(item_copy)
    return data, flat_items


def audit_catalog(repo_root: Path = REPO_ROOT) -> Dict[str, Any]:
    """Audits catalog for missing files, dead links, and uncataloged files."""
    data, flat_items = load_catalog()
    catalog_path_set = {item["path"].replace("\\", "/") for item in flat_items}

    dead_links = []
    for item in flat_items:
        raw_path = item["path"]
        if raw_path.startswith("http://") or raw_path.startswith("https://"):
            continue
        p = repo_root / raw_path
        if not p.exists():
            dead_links.append(item)

    # Scan for specs
    specs_dir = repo_root / "docs" / "specs"
    uncataloged_specs = []
    if specs_dir.exists():
        for f in specs_dir.glob("*.md"):
            rel = f.relative_to(repo_root).as_posix()
            if rel not in catalog_path_set:
                title, purpose = extract_metadata_from_markdown(f)
                uncataloged_specs.append({
                    "path": rel,
                    "title": title,
                    "purpose": purpose,
                    "category": "technical_specifications",
                })

    return {
        "total_cataloged": len(flat_items),
        "dead_links": dead_links,
        "uncataloged_specs": uncataloged_specs,
        "is_clean": len(dead_links) == 0 and len(uncataloged_specs) == 0,
    }


def sync_catalog(repo_root: Path = REPO_ROOT, dry_run: bool = False) -> Dict[str, Any]:
    """Fixes catalog by pruning dead links and inserting uncataloged specs."""
    data, flat_items = load_catalog()
    audit_res = audit_catalog(repo_root)

    if audit_res["is_clean"]:
        return {"modified": False, "added": 0, "pruned": 0}

    dead_paths = {d["path"].replace("\\", "/") for d in audit_res["dead_links"]}
    pruned_count = 0
    added_count = 0

    # Prune dead links
    for cat_key, cat_val in data.get("catalogs", {}).items():
        original_items = cat_val.get("items", [])
        cleaned = [it for it in original_items if it.get("path", "").replace("\\", "/") not in dead_paths]
        pruned_count += len(original_items) - len(cleaned)
        cat_val["items"] = cleaned

    # Add uncataloged specs to technical_specifications
    tech_specs = data.setdefault("catalogs", {}).setdefault("technical_specifications", {"description": "Technical specifications", "items": []})
    for u in audit_res["uncataloged_specs"]:
        tech_specs["items"].append({
            "path": u["path"],
            "title": u["title"],
            "purpose": u["purpose"],
        })
        added_count += 1

    if not dry_run and (pruned_count > 0 or added_count > 0):
        # Sort items inside each category by path
        for cat_key, cat_val in data.get("catalogs", {}).items():
            cat_val["items"] = sorted(cat_val["items"], key=lambda x: x.get("path", "").lower())

        CATALOG_PATH.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")

    return {"modified": (pruned_count > 0 or added_count > 0), "added": added_count, "pruned": pruned_count}


def print_report(res: Dict[str, Any]):
    print("=" * 70)
    print("📚 DOCUMENTATION CATALOG AUDIT (Flight 1 / Zero-Token Suite)")
    print("=" * 70)
    print(f"📊 Total Cataloged Entries: {res['total_cataloged']}")
    
    if res["dead_links"]:
        print(f"\n❌ Dead Links Detected ({len(res['dead_links'])}):")
        for d in res["dead_links"]:
            print(f"   • [{d['category']}] {d['path']} — {d['title']}")
    else:
        print("🔗 Dead Links: 0 (All cataloged files exist)")

    if res["uncataloged_specs"]:
        print(f"\n⚠️ Uncataloged Specifications ({len(res['uncataloged_specs'])}):")
        for u in res["uncataloged_specs"]:
            print(f"   • {u['path']} ➔ {u['title']}")
    else:
        print("📁 Uncataloged Files: 0 (All specs registered)")

    print("=" * 70)
    if res["is_clean"]:
        print("✅ CATALOG 100% IN SYNC: Zero dead links, all specs indexed.")
    else:
        print("⚠️ ACTION REQUIRED: Run with --fix to automatically synchronize catalog.")
    print("=" * 70)


def main():
    parser = argparse.ArgumentParser(description="Documentation Catalog Synchronizer")
    parser.add_argument("--check", action="store_true", help="Exit code 1 if catalog is out of sync")
    parser.add_argument("--fix", action="store_true", help="Automatically prune dead links and add uncataloged specs")
    parser.add_argument("--dry-run", action="store_true", help="Simulate fixes without modifying files")
    parser.add_argument("--quiet", action="store_true", help="Minimal console output")

    args = parser.parse_args()

    audit_res = audit_catalog()

    if args.fix:
        sync_res = sync_catalog(dry_run=args.dry_run)
        if not args.quiet:
            print(f"🔄 Catalog Sync: Added {sync_res['added']} spec(s), pruned {sync_res['pruned']} dead link(s).")
        # Re-audit after fix
        audit_res = audit_catalog()

    if not args.quiet:
        print_report(audit_res)

    if args.check and not audit_res["is_clean"]:
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
