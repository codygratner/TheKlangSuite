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
import json
import os
import re
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional

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


def cmd_status(args: argparse.Namespace) -> int:
    """Ultra-fast, zero-token status checker for agent hooks and response footers.
    Emits zero bytes when the queue has no pending dispatches for the target pillar.
    """
    bus_dir = resolve_bus_dir()
    dispatches = load_dispatches(bus_dir, include_archived=False)

    target_pillar = (args.for_pillar or detect_current_pillar()).upper()

    pending = []
    for d in dispatches:
        status = str(d.get("status", "")).upper()
        if status != "PENDING":
            continue

        to_pillar = str(d.get("to", "")).upper()
        from_pillar = str(d.get("from", "")).upper()

        if to_pillar in [target_pillar, "ALL"]:
            # Self-sender exclusion: do not alert on dispatches we authored for another pillar
            if from_pillar == target_pillar and to_pillar != target_pillar:
                continue
            pending.append(d)

    if not pending:
        # Zero-token quiet invariant: emit nothing
        return 0

    if getattr(args, "json", False):
        import json
        print(json.dumps([{"id": d.get("id"), "title": d.get("title"), "from": d.get("from"), "priority": d.get("priority")} for d in pending], ensure_ascii=False))
        return 0

    count = len(pending)
    items_summary = ", ".join(f"{d.get('id')} ({d.get('priority', 'P1')})" for d in pending[:3])
    if count > 3:
        items_summary += f", ... +{count - 3} more"

    print(f"📬 [DISPATCH PENDING] {count} unread dispatch(es) for {target_pillar}: {items_summary}")
    return 0


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


def get_locks_dir(bus_dir: Path) -> Path:
    d = bus_dir / "Locks"
    d.mkdir(parents=True, exist_ok=True)
    return d


def load_active_locks(bus_dir: Path) -> List[Dict[str, Any]]:
    locks_dir = get_locks_dir(bus_dir)
    active = []
    now = datetime.datetime.now()

    for p in locks_dir.glob("*_lock.json"):
        try:
            data = json.loads(p.read_text(encoding="utf-8"))
            expires_at_str = data.get("expires_at")
            if expires_at_str:
                exp = datetime.datetime.fromisoformat(expires_at_str)
                if now > exp:
                    # Stale lock expired: auto garbage collect
                    try:
                        p.unlink()
                    except Exception:
                        pass
                    continue
            data["_path"] = str(p)
            active.append(data)
        except Exception:
            continue
    return active


def cmd_lock(args: argparse.Namespace) -> int:
    bus_dir = resolve_bus_dir()
    locks_dir = get_locks_dir(bus_dir)
    pillar = (args.pillar or detect_current_pillar()).upper()
    now = datetime.datetime.now()
    ttl = args.ttl if args.ttl else 30
    expires = now + datetime.timedelta(minutes=ttl)

    scopes = [s.strip().replace("\\", "/").rstrip("/") for s in args.scope.split(",") if s.strip()]

    lock_data = {
        "pillar": pillar,
        "activity": args.activity,
        "scope": scopes,
        "started_at": now.isoformat(),
        "ttl_minutes": ttl,
        "expires_at": expires.isoformat(),
    }

    lock_file = locks_dir / f"{pillar.lower()}_lock.json"
    lock_file.write_text(json.dumps(lock_data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(f"🔒 Acquired intent lock for {pillar}: {args.activity}")
    print(f"   • Scopes:  {', '.join(scopes)}")
    print(f"   • Expires: {expires.strftime('%H:%M:%S')} ({ttl}m TTL)")
    return 0


def cmd_unlock(args: argparse.Namespace) -> int:
    bus_dir = resolve_bus_dir()
    locks_dir = get_locks_dir(bus_dir)
    pillar = (args.pillar or detect_current_pillar()).upper()
    lock_file = locks_dir / f"{pillar.lower()}_lock.json"

    if lock_file.exists():
        try:
            lock_file.unlink()
            print(f"🔓 Released active lock for {pillar}.")
        except Exception as e:
            print(f"⚠️ Failed to release lock: {e}", file=sys.stderr)
            return 1
    else:
        print(f"ℹ️ No active lock found for {pillar}.")
    return 0


def cmd_locks(args: argparse.Namespace) -> int:
    bus_dir = resolve_bus_dir()
    active = load_active_locks(bus_dir)

    if getattr(args, "json", False):
        print(json.dumps(active, indent=2, ensure_ascii=False))
        return 0

    print("=" * 80)
    print("🔒 ACTIVE PEER INTENT LOCKS & CONCURRENCY RADAR")
    print("=" * 80)

    if not active:
        print("🌌 All peer channels clear. Zero active filesystem locks.")
        print("=" * 80)
        return 0

    print(f"{'PILLAR':<10} | {'EXPIRES':<10} | {'ACTIVITY':<30} | {'SCOPES'}")
    print("-" * 80)
    for l in active:
        pil = l.get("pillar", "?")
        exp = l.get("expires_at", "")[:19].split("T")[-1]
        act = l.get("activity", "")[:30]
        scopes = ", ".join(l.get("scope", []))
        print(f"{pil:<10} | {exp:<10} | {act:<30} | {scopes}")
    print("=" * 80)
    return 0


def cmd_check_scope(args: argparse.Namespace) -> int:
    bus_dir = resolve_bus_dir()
    active = load_active_locks(bus_dir)
    current_pillar = (getattr(args, "pillar", None) or detect_current_pillar()).upper()
    target_scopes = [s.strip().replace("\\", "/").rstrip("/").lower() for s in args.scope.split(",") if s.strip()]

    collisions = []
    for l in active:
        if l.get("pillar", "").upper() == current_pillar:
            # Ignore own locks
            continue
        peer_scopes = [s.lower() for s in l.get("scope", [])]
        for t in target_scopes:
            for p in peer_scopes:
                if t == p or t.startswith(p + "/") or p.startswith(t + "/"):
                    collisions.append((l, t, p))

    if collisions:
        print("⚠️ SCOPE COLLISION DETECTED:", file=sys.stderr)
        for lock, target, peer in collisions:
            print(f"   • {lock.get('pillar')} is actively locking '{peer}' for: {lock.get('activity')}", file=sys.stderr)
            print(f"     (Lock expires at {lock.get('expires_at')})", file=sys.stderr)
        return 1

    if not getattr(args, "quiet", False):
        print("✅ Scope clear. Zero peer lock collisions.")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(
        description="🛰️ Inter-Project Data Bus & Queue for The Klang Suite, ToadTracker, N'kai & The Cosmos"
    )
    subparsers = parser.add_subparsers(dest="command", required=True)

    # status (zero-token quiet checker)
    p_status = subparsers.add_parser("status", help="Quick zero-token check for pending dispatches")
    p_status.add_argument("--for", dest="for_pillar", help="Check for a specific pillar (default: auto-detected)")
    p_status.add_argument("--json", action="store_true", help="Output JSON array instead of single line")

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

    # lock
    p_lock = subparsers.add_parser("lock", help="Acquire an ephemeral intent lock declaring active blast radius")
    p_lock.add_argument("--scope", required=True, help="Comma-separated directory or file paths being modified")
    p_lock.add_argument("--activity", required=True, help="Description of operation (e.g. 'Tag audit and frontmatter backfill')")
    p_lock.add_argument("--ttl", type=int, default=30, help="Time to live in minutes (default: 30)")
    p_lock.add_argument("--pillar", help="Pillar acquiring the lock (default: auto-detected)")

    # unlock
    p_unlock = subparsers.add_parser("unlock", help="Release active intent lock for the host pillar")
    p_unlock.add_argument("--pillar", help="Pillar releasing the lock (default: auto-detected)")

    # locks
    p_locks = subparsers.add_parser("locks", help="List active peer intent locks across the ecosystem")
    p_locks.add_argument("--json", action="store_true", help="Output JSON array")

    # check-scope
    p_check = subparsers.add_parser("check-scope", help="Check if target scope collides with active peer locks")
    p_check.add_argument("--scope", required=True, help="Comma-separated paths to check against active locks")
    p_check.add_argument("--pillar", help="Current host pillar (locks by this pillar are ignored)")
    p_check.add_argument("--quiet", action="store_true", help="Omit success output")

    args = parser.parse_args()

    if args.command == "status":
        return cmd_status(args)
    elif args.command == "list":
        return cmd_list(args)
    elif args.command == "post":
        return cmd_post(args)
    elif args.command == "read":
        return cmd_read(args)
    elif args.command == "ack":
        return cmd_ack(args)
    elif args.command == "lock":
        return cmd_lock(args)
    elif args.command == "unlock":
        return cmd_unlock(args)
    elif args.command == "locks":
        return cmd_locks(args)
    elif args.command == "check-scope":
        return cmd_check_scope(args)

    return 0


if __name__ == "__main__":
    sys.exit(main())
