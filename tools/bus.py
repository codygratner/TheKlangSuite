#!/usr/bin/env python3
"""
tools/bus.py
Inter-Project Data Bus & Queue for The Klang Suite Ecosystem.
Asynchronously transports tagged data packets and handoffs between:
- The Klang Suite (TKS)
- ToadTracker (TT)
- N'kai (NK)
- The Klang Research (TR)
- The Cosmos (COSMOS)
"""

import argparse
import datetime
import os
import re
import sys
from pathlib import Path
from typing import Dict, List, Optional

# Enforce Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")
 
CANDIDATE_BUS_PATHS = [
    Path(r"C:\Dev\TheKlangVault\Dispatches"),
    Path(r"C:\_\Vault\New Klang City\Dispatches"),
    Path(__file__).resolve().parent.parent / "docs" / "dispatches",
]


def check_junction_integrity() -> bool:
    """Verifies that the primary NTFS junction C:\\Dev\\TheKlangVault is alive and healthy."""
    vault_junction = Path(r"C:\Dev\TheKlangVault")
    if vault_junction.is_symlink() or vault_junction.is_junction():
        try:
            # Resolving should point to existing directory
            resolved = vault_junction.resolve()
            if not resolved.exists():
                print(f"⚠️ JUNCTION ALERT: C:\\Dev\\TheKlangVault points to missing target: {resolved}", file=sys.stderr)
                return False
        except Exception as e:
            print(f"⚠️ JUNCTION ERROR: Failed to resolve C:\\Dev\\TheKlangVault: {e}", file=sys.stderr)
            return False
    return True


def resolve_bus_dir() -> Path:
    """Finds the active cross-project Dispatches directory with junction health checks."""
    check_junction_integrity()
    for p in CANDIDATE_BUS_PATHS:
        try:
            if p.exists():
                return p
        except Exception:
            continue
    # Fallback: create in primary location if accessible
    for p in CANDIDATE_BUS_PATHS[:2]:
        try:
            p.mkdir(parents=True, exist_ok=True)
            return p
        except Exception:
            continue
    local_p = CANDIDATE_BUS_PATHS[-1]
    local_p.mkdir(parents=True, exist_ok=True)
    return local_p


def detect_current_pillar() -> str:
    """Infers current pillar based on working directory."""
    cwd = Path.cwd().resolve()
    name = cwd.name.lower()
    if "klangsuite" in name:
        return "TKS"
    elif "toadtracker" in name:
        return "TT"
    elif "nkai" in name:
        return "NK"
    elif "research" in name:
        return "TR"
    elif "vault" in name or "cosmos" in name:
        return "COSMOS"
    return "TKS"


def parse_frontmatter(content: str) -> tuple[Dict[str, str], str]:
    """Parses simple YAML frontmatter and returns (metadata_dict, body_text)."""
    meta = {}
    body = content
    if content.startswith("---"):
        parts = content.split("---", 2)
        if len(parts) >= 3:
            raw_meta = parts[1]
            body = parts[2].strip()
            for line in raw_meta.splitlines():
                line = line.strip()
                if not line or line.startswith("#"):
                    continue
                if ":" in line:
                    key, val = line.split(":", 1)
                    key = key.strip()
                    val = val.strip().strip('"').strip("'")
                    if val.startswith("[") and val.endswith("]"):
                        # Parse list
                        val = [item.strip().strip('"').strip("'") for item in val[1:-1].split(",") if item.strip()]
                    meta[key] = val
    return meta, body


def serialize_dispatch(meta: Dict[str, str], body: str) -> str:
    """Serializes metadata and body to a clean Markdown dispatch file."""
    lines = ["---"]
    for k, v in meta.items():
        if isinstance(v, list):
            lines.append(f"{k}: [{', '.join(v)}]")
        else:
            lines.append(f'{k}: "{v}"')
    lines.append("---")
    lines.append("")
    lines.append(body.strip())
    lines.append("")
    return "\n".join(lines)


def load_dispatches(bus_dir: Path, include_archived: bool = False) -> List[Dict]:
    """Scans and loads all dispatches from the bus directory."""
    dispatches = []
    target_dirs = [bus_dir]
    if include_archived:
        archive_dir = bus_dir / "Archive"
        if archive_dir.exists():
            target_dirs.append(archive_dir)

    for d in target_dirs:
        for f in d.glob("*.md"):
            if f.name.lower() == "readme.md":
                continue
            try:
                raw = f.read_text(encoding="utf-8", errors="replace")
                meta, body = parse_frontmatter(raw)
                meta["_path"] = str(f)
                meta["_filename"] = f.name
                meta["_is_archived"] = "archive" in str(f).lower()
                meta["_body"] = body
                if "id" not in meta:
                    meta["id"] = f.stem
                dispatches.append(meta)
            except Exception as e:
                print(f"⚠️ Error reading {f.name}: {e}", file=sys.stderr)

    return sorted(dispatches, key=lambda x: str(x.get("date", "")), reverse=True)


def cmd_list(args: argparse.Namespace) -> int:
    bus_dir = resolve_bus_dir()
    dispatches = load_dispatches(bus_dir, include_archived=args.include_archived)

    # Filtering
    if args.for_pillar:
        target = args.for_pillar.upper()
        dispatches = [d for d in dispatches if str(d.get("to", "")).upper() in [target, "ALL"]]
    if args.from_pillar:
        src = args.from_pillar.upper()
        dispatches = [d for d in dispatches if str(d.get("from", "")).upper() == src]
    if args.tag:
        t = args.tag.lower().lstrip("#")
        dispatches = [d for d in dispatches if t in [str(x).lower().lstrip("#") for x in (d.get("tags") if isinstance(d.get("tags"), list) else [d.get("tags", "")])]]

    print("=" * 80)
    print(f"🛰️  INTER-PROJECT DATA BUS: {bus_dir}")
    print(f"   Showing {len(dispatches)} dispatch(es)" + (" (including Archive)" if args.include_archived else ""))
    print("=" * 80)

    if not dispatches:
        print("🌌 All channels clear. No dispatches matching criteria.")
        print("=" * 80)
        return 0

    header = f"{'ID':<28} | {'ROUTE':<12} | {'PRI':<4} | {'STATUS':<12} | {'TITLE'}"
    print(header)
    print("-" * 80)

    for d in dispatches:
        d_id = str(d.get("id", d.get("_filename", "")))[:28]
        route = f"{d.get('from', '?')} ➔ {d.get('to', '?')}"
        pri = str(d.get("priority", "P1"))
        stat = str(d.get("status", "PENDING"))
        if d.get("_is_archived"):
            stat = "📁 ARCHIVED"
        title = str(d.get("title", "(untitled)"))[:30]
        print(f"{d_id:<28} | {route:<12} | {pri:<4} | {stat:<12} | {title}")

    print("=" * 80)
    return 0


def cmd_post(args: argparse.Namespace) -> int:
    bus_dir = resolve_bus_dir()
    sender = (args.from_pillar or detect_current_pillar()).upper()
    recipient = args.to.upper()
    today = datetime.date.today().isoformat()
    now_slug = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")

    slug_base = re.sub(r"[^a-zA-Z0-9_-]", "_", args.title.lower()).strip("_")[:30]
    dispatch_id = f"{today}_{sender.lower()}_to_{recipient.lower()}_{slug_base}"
    filename = f"{dispatch_id}.md"
    target_file = bus_dir / filename

    raw_tags = [t.strip().lstrip("#") for t in (args.tags.split(",") if args.tags else []) if t.strip()]
    if sender.lower() not in raw_tags:
        raw_tags.append(sender.lower())
    if recipient.lower() != "all" and recipient.lower() not in raw_tags:
        raw_tags.append(recipient.lower())

    meta = {
        "id": dispatch_id,
        "date": today,
        "from": sender,
        "to": recipient,
        "tags": raw_tags,
        "priority": args.priority.upper(),
        "status": "PENDING",
        "title": args.title,
    }

    body = args.body or f"# {args.title}\n\n(No detailed description provided.)"
    if args.payload_file:
        pf = Path(args.payload_file).resolve()
        if pf.exists():
            payload_content = pf.read_text(encoding="utf-8", errors="replace")
            body += f"\n\n### 📦 Attached Payload (`{pf.name}`):\n```text\n{payload_content}\n```"

    content = serialize_dispatch(meta, body)
    target_file.write_text(content, encoding="utf-8")
    print(f"🚀 Dispatched successfully to {recipient}!")
    print(f"   ID:       {dispatch_id}")
    print(f"   Location: {target_file}")
    return 0


def cmd_read(args: argparse.Namespace) -> int:
    bus_dir = resolve_bus_dir()
    dispatches = load_dispatches(bus_dir, include_archived=True)
    target_id = args.id.lower()

    found = None
    for d in dispatches:
        if d.get("id", "").lower() == target_id or target_id in d.get("_filename", "").lower():
            found = d
            break

    if not found:
        print(f"❌ Dispatch '{args.id}' not found on the bus.", file=sys.stderr)
        return 1

    print("=" * 80)
    print(f"📄 DISPATCH: {found.get('title', 'Untitled')}")
    print("=" * 80)
    print(f"ID:       {found.get('id')}")
    print(f"Route:    {found.get('from')} ➔ {found.get('to')}")
    print(f"Date:     {found.get('date')}")
    print(f"Priority: {found.get('priority')}")
    print(f"Status:   {found.get('status')}")
    tags_display = ", ".join(f"#{t}" for t in found.get("tags", [])) if isinstance(found.get("tags"), list) else found.get("tags")
    print(f"Tags:     {tags_display}")
    print(f"Path:     {found.get('_path')}")
    print("-" * 80)
    print(found.get("_body", ""))
    print("=" * 80)
    return 0


def cmd_ack(args: argparse.Namespace) -> int:
    bus_dir = resolve_bus_dir()
    dispatches = load_dispatches(bus_dir, include_archived=False)
    target_id = args.id.lower()

    found = None
    for d in dispatches:
        if d.get("id", "").lower() == target_id or target_id in d.get("_filename", "").lower():
            found = d
            break

    if not found:
        print(f"❌ Active dispatch '{args.id}' not found.", file=sys.stderr)
        return 1

    p = Path(found["_path"])
    raw = p.read_text(encoding="utf-8")
    meta, body = parse_frontmatter(raw)
    meta["status"] = "ACKNOWLEDGED"
    meta["acked_at"] = datetime.datetime.now().isoformat()

    new_content = serialize_dispatch(meta, body)
    p.write_text(new_content, encoding="utf-8")
    print(f"✅ Marked dispatch '{meta.get('id')}' as ACKNOWLEDGED.")

    if args.archive:
        archive_dir = bus_dir / "Archive"
        archive_dir.mkdir(parents=True, exist_ok=True)
        dest = archive_dir / p.name
        p.replace(dest)
        print(f"📦 Archived to: {dest}")

    return 0


def main() -> int:
    parser = argparse.ArgumentParser(
        description="🛰️ Inter-Project Data Bus & Queue for The Klang Suite, ToadTracker, N'kai & The Cosmos"
    )
    subparsers = parser.add_subparsers(dest="command", required=True)

    # list
    p_list = subparsers.add_parser("list", help="List active dispatches on the bus")
    p_list.add_argument("--for", dest="for_pillar", help="Filter dispatches addressed to this pillar (e.g. TT, TKS, NK)")
    p_list.add_argument("--from", dest="from_pillar", help="Filter dispatches sent by this pillar")
    p_list.add_argument("--tag", help="Filter by tag (e.g. dsp, xrns, lore)")
    p_list.add_argument("--all", dest="include_archived", action="store_true", help="Include archived dispatches")

    # post
    p_post = subparsers.add_parser("post", help="Post a new cross-project dispatch packet")
    p_post.add_argument("--to", required=True, help="Target pillar (TKS, TT, NK, TR, COSMOS, ALL)")
    p_post.add_argument("--title", required=True, help="Short descriptive title of handoff")
    p_post.add_argument("--from", dest="from_pillar", help="Originating pillar (defaults to auto-detected)")
    p_post.add_argument("--body", help="Detailed markdown description / instructions")
    p_post.add_argument("--tags", help="Comma-separated tags (e.g. dsp,xrns,vst3)")
    p_post.add_argument("--priority", default="P1", choices=["P0", "P1", "P2", "P3"], help="Priority level")
    p_post.add_argument("--payload-file", help="Path to a data file to attach to the body")

    # read
    p_read = subparsers.add_parser("read", help="Read and inspect a dispatch packet")
    p_read.add_argument("id", help="Dispatch ID or substring of filename")

    # ack
    p_ack = subparsers.add_parser("ack", help="Acknowledge receipt of a dispatch")
    p_ack.add_argument("id", help="Dispatch ID or substring of filename")
    p_ack.add_argument("--archive", action="store_true", help="Move dispatch to Archive/ after acknowledging")

    args = parser.parse_args()

    if args.command == "list":
        return cmd_list(args)
    elif args.command == "post":
        return cmd_post(args)
    elif args.command == "read":
        return cmd_read(args)
    elif args.command == "ack":
        return cmd_ack(args)

    return 0


if __name__ == "__main__":
    sys.exit(main())
