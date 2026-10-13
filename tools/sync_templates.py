#!/usr/bin/env python3
"""
tools/sync_templates.py — Multi-Repo Template & Asset Synchronizer
==================================================================
Ensures 100% SHA-256 hash parity across mirrored template files in
TheKlangSuite and N'kai. Eliminates ~20,000 tokens of manual file copying
per UI design iteration.
Part of the Zero-Token Mechanical Automation Suite (Flight 4).
"""

import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

# Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent
DEV_ROOT = REPO_ROOT.parent

# Repository roots
ROOTS = {
    "TKS": REPO_ROOT,
    "NKAI": DEV_ROOT / "nkai",
}

DEFAULT_MAPPINGS = [
    {
        "id": "shell_template",
        "description": "Universal Dynamic Shell Template",
        "source": ("NKAI", "core/shell.html"),
        "targets": [
            ("TKS", ".agents/sidecar/shell.html"),
            ("NKAI", "sidecars/panel/shell.html"),
            ("NKAI", "bundle/templates/shell.html"),
        ],
    },
    {
        "id": "master_template",
        "description": "Master Sidecar Two-Tier Board Template",
        "source": ("NKAI", "core/master_template.html"),
        "targets": [
            ("TKS", ".agents/sidecar/master_template.html"),
            ("NKAI", "bundle/templates/master_template.html"),
        ],
    },
    {
        "id": "template_legacy",
        "description": "Legacy Sidecar Template",
        "source": ("NKAI", "bundle/templates/template.html"),
        "targets": [
            ("TKS", ".agents/sidecar/template.html"),
        ],
    },
    {
        "id": "context_clues_template",
        "description": "0-Turn Cold Boot Context Clues Template",
        "source": ("TKS", "docs/templates/context_clues_template.md"),
        "targets": [
            ("NKAI", "bundle/templates/context_clues_template.md"),
        ],
    },
    {
        "id": "chat_board",
        "description": "N'kai Agent Intercom & Live Chat Board",
        "source": ("TKS", ".agents/sidecar/chat_board.html"),
        "targets": [
            ("NKAI", "sidecars/panel/chat_board.html"),
            ("NKAI", "plugin/sidecars/panel/chat_board.html"),
        ],
    },
]


def compute_sha256(path: Path) -> Optional[str]:
    """Computes SHA-256 hex digest of a file; returns None if file does not exist."""
    if not path.is_file():
        return None
    try:
        data = path.read_bytes()
        return hashlib.sha256(data).hexdigest()
    except Exception:
        return None


def resolve_path(pillar: str, rel_path: str, custom_roots: Optional[Dict[str, Path]] = None) -> Path:
    roots = custom_roots or ROOTS
    base = roots.get(pillar.upper(), REPO_ROOT)
    return base / rel_path


def is_file_git_dirty(file_path: Path) -> bool:
    """Checks if the file has uncommitted changes in its git repository."""
    try:
        repo_dir = file_path.parent
        while repo_dir != repo_dir.parent:
            if (repo_dir / ".git").exists():
                break
            repo_dir = repo_dir.parent
        else:
            return False

        res = subprocess.run(
            ["git", "status", "--porcelain", str(file_path)],
            cwd=str(repo_dir),
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            check=False,
        )
        return bool(res.stdout.strip())
    except Exception:
        return False


def audit_mappings(
    mappings: List[Dict[str, Any]],
    custom_roots: Optional[Dict[str, Path]] = None,
) -> List[Dict[str, Any]]:
    """Audits SHA-256 hashes of all sources against their target mirrors."""
    results = []

    for m in mappings:
        src_pillar, src_rel = m["source"]
        src_path = resolve_path(src_pillar, src_rel, custom_roots)
        src_hash = compute_sha256(src_path)

        targets_info = []
        all_synced = True

        for tgt_pillar, tgt_rel in m["targets"]:
            tgt_path = resolve_path(tgt_pillar, tgt_rel, custom_roots)
            tgt_hash = compute_sha256(tgt_path)

            if not tgt_path.exists():
                status = "MISSING"
                all_synced = False
            elif tgt_hash == src_hash and src_hash is not None:
                status = "IN_SYNC"
            else:
                status = "DRIFTED"
                all_synced = False

            targets_info.append({
                "pillar": tgt_pillar,
                "rel_path": tgt_rel,
                "path": str(tgt_path),
                "hash": tgt_hash,
                "status": status,
                "dirty": is_file_git_dirty(tgt_path) if tgt_path.exists() else False,
            })

        results.append({
            "id": m["id"],
            "description": m["description"],
            "source": {
                "pillar": src_pillar,
                "rel_path": src_rel,
                "path": str(src_path),
                "hash": src_hash,
                "exists": src_path.exists(),
            },
            "targets": targets_info,
            "all_synced": all_synced and (src_hash is not None),
        })

    return results


def sync_mappings(
    audit_results: List[Dict[str, Any]],
    force: bool = False,
    dry_run: bool = False,
) -> Tuple[int, int, List[str]]:
    """Synchronizes drifted or missing targets from canonical sources.
    Returns (synced_count, skipped_count, log_messages).
    """
    synced = 0
    skipped = 0
    logs = []

    for item in audit_results:
        src_info = item["source"]
        src_path = Path(src_info["path"])

        if not src_path.exists():
            logs.append(f"⚠️ Source missing for '{item['id']}': {src_path}")
            skipped += len(item["targets"])
            continue

        for tgt in item["targets"]:
            if tgt["status"] == "IN_SYNC":
                continue

            tgt_path = Path(tgt["path"])

            if tgt["dirty"] and not force:
                logs.append(f"🛑 Skipping dirty target without --force: {tgt_path.name} ({tgt['pillar']})")
                skipped += 1
                continue

            if dry_run:
                logs.append(f"🔍 [DRY-RUN] Would copy {src_path.name} ({src_info['pillar']}) ➔ {tgt_path} ({tgt['status']})")
                synced += 1
                continue

            try:
                tgt_path.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(src_path, tgt_path)
                logs.append(f"✅ Synced: {src_path.name} ({src_info['pillar']}) ➔ {tgt_path} [{tgt['status']} ➔ IN_SYNC]")
                synced += 1
            except Exception as e:
                logs.append(f"❌ Failed to copy to {tgt_path}: {e}")
                skipped += 1

    return synced, skipped, logs


def cmd_check(args: argparse.Namespace, custom_roots: Optional[Dict[str, Path]] = None) -> int:
    results = audit_mappings(DEFAULT_MAPPINGS, custom_roots)
    all_in_sync = all(r["all_synced"] for r in results)

    if getattr(args, "json", False):
        print(json.dumps(results, indent=2, ensure_ascii=False))
        return 0 if all_in_sync else 1

    print("=" * 75)
    print("🔄 MULTI-REPO TEMPLATE DRIFT AUDITOR (Flight 4)")
    print("=" * 75)

    drift_count = 0
    for r in results:
        src = r["source"]
        src_short_hash = src["hash"][:10] if src["hash"] else "MISSING"
        print(f"📦 {r['description']} [{r['id']}]")
        print(f"   • Source:  [{src['pillar']}] {src['rel_path']} (SHA: {src_short_hash})")

        for t in r["targets"]:
            icon = "✅" if t["status"] == "IN_SYNC" else "⚠️"
            t_short_hash = t["hash"][:10] if t["hash"] else "MISSING"
            dirty_badge = " [DIRTY]" if t["dirty"] else ""
            print(f"   {icon} Target:  [{t['pillar']}] {t['rel_path']} ➔ {t['status']}{dirty_badge} (SHA: {t_short_hash})")
            if t["status"] != "IN_SYNC":
                drift_count += 1
        print("-" * 75)

    print("=" * 75)
    print("📊 TEMPLATE AUDIT SCORECARD:")
    print(f"   • Mappings Checked: {len(results)}")
    print(f"   • Total Targets:    {sum(len(r['targets']) for r in results)}")
    print(f"   • Drift / Missing:  {drift_count}")
    print("=" * 75)

    if all_in_sync:
        print("✅ 100% TEMPLATE PARITY: All template mirrors match canonical sources.")
        return 0
    else:
        print(f"⚠️ DRIFT DETECTED: {drift_count} template mirror(s) out of sync. Run 'sync' to update.", file=sys.stderr)
        return 1


def cmd_sync(args: argparse.Namespace, custom_roots: Optional[Dict[str, Path]] = None) -> int:
    results = audit_mappings(DEFAULT_MAPPINGS, custom_roots)
    synced, skipped, logs = sync_mappings(results, force=args.force, dry_run=args.dry_run)

    for l in logs:
        print(l)

    if args.dry_run:
        print(f"\n🔍 [DRY-RUN COMPLETE] Would sync {synced} file(s), skip {skipped}.")
        return 0

    print(f"\n🎉 Sync completed: {synced} file(s) synchronized, {skipped} skipped.")
    # Re-audit
    post_results = audit_mappings(DEFAULT_MAPPINGS, custom_roots)
    all_in_sync = all(r["all_synced"] for r in post_results)
    if all_in_sync:
        print("✅ 100% TEMPLATE PARITY ACHIEVED: All mirrors verified.")
        return 0
    return 1


def main() -> int:
    parser = argparse.ArgumentParser(
        description="🔄 Multi-Repo Template & Asset Synchronizer (Zero-Token Mechanical Clerk)"
    )
    subparsers = parser.add_subparsers(dest="command", required=True)

    # Subcommand: check
    parser_check = subparsers.add_parser("check", help="Audit SHA-256 hashes across template mirrors and report drift")
    parser_check.add_argument("--json", action="store_true", help="Output audit results as JSON")

    # Subcommand: sync
    parser_sync = subparsers.add_parser("sync", help="Copy canonical templates to all mirrors")
    parser_sync.add_argument("--force", "-f", action="store_true", help="Overwrite even if target is uncommitted in git")
    parser_sync.add_argument("--dry-run", action="store_true", help="Show what would be copied without writing to disk")

    args = parser.parse_args()

    if args.command == "check":
        return cmd_check(args)
    elif args.command == "sync":
        return cmd_sync(args)

    return 0


if __name__ == "__main__":
    sys.exit(main())
