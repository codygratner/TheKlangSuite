#!/usr/bin/env python3
"""
N'kai Multi-Board Hub & Session Registry CLI (Flight 14 / Topic 6.11 / rj-05)
Manages canonical registry of operational boards and historical research sessions.
Universal UTF-8 compliance guaranteed.
"""

import argparse
from datetime import datetime
import json
import os
from pathlib import Path
import re
import sys
from typing import Optional

# Universal UTF-8 Environment Invariant (Flight 18)
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

try:
    from tools.models import BoardMetadata, BoardRegistry
except ImportError:
    try:
        from models import BoardMetadata, BoardRegistry
    except ImportError:
        _script_dir = Path(__file__).resolve().parent
        if str(_script_dir) not in sys.path:
            sys.path.insert(0, str(_script_dir))
        from models import BoardMetadata, BoardRegistry


DEFAULT_REGISTRY_PATHS = [
    REPO_ROOT / ".agents" / "pipeline" / "boards" / "registry.json",
    Path(r"c:\Dev\nkai\boards\registry.json"),
    Path(r"c:\Dev\nkai\bundle\boards\registry.json"),
    REPO_ROOT / "boards" / "registry.json",
]


def resolve_registry_path(custom_path: Optional[str] = None) -> Path:
    """Resolves registry.json file path across workspace candidates."""
    if custom_path:
        return Path(custom_path).resolve()
    for cand in DEFAULT_REGISTRY_PATHS:
        if cand.exists():
            return cand
    return DEFAULT_REGISTRY_PATHS[0]


def load_registry(path: Optional[Path] = None) -> BoardRegistry:
    """Loads and validates BoardRegistry from JSON file."""
    reg_path = path or resolve_registry_path()
    if not reg_path.exists():
        # Return default initialized registry
        return BoardRegistry(
            version="1.0.0",
            default_board_id="morning_mail",
            boards=[
                BoardMetadata(
                    id="morning_mail",
                    title="Morning Mail Inbox & Triage Lab",
                    description="Daily operational triage for developer voice memos and backlog tickets",
                    category="active",
                    board_type="evergreen",
                    file_path=".agents/sidecar/morning_mail_inbox.html",
                    environment="PRODUCTION",
                    card_count=0,
                ),
                BoardMetadata(
                    id="runway",
                    title="Ecosystem Delivery Runway",
                    description="Cross-project strategic delivery horizon mapped from docs/BACKLOG.md Act III",
                    category="active",
                    board_type="evergreen",
                    file_path=".agents/sidecar/runway_board.html",
                    environment="PRODUCTION",
                    card_count=38,
                ),
                BoardMetadata(
                    id="system_sanity_audit",
                    title="System Sanity Audit & Triage Lab",
                    description="Forensic Method B Dialectics across Messaging, Capture, Sidecars & Memory",
                    category="archived",
                    board_type="lab",
                    file_path=".agents/sidecar/audit_triage_board.html",
                    environment="PRODUCTION",
                    card_count=12,
                ),
                BoardMetadata(
                    id="sandbox_mail",
                    title="Canonical QA Sandbox Test Board",
                    description="Deterministic test fixture with 7 synthetic cards for regression testing",
                    category="sandbox",
                    board_type="custom",
                    file_path=".agents/sidecar/sandbox_mail_inbox.html",
                    environment="SANDBOX",
                    card_count=7,
                ),
            ],
        )
    content = reg_path.read_text(encoding="utf-8")
    return BoardRegistry.model_validate_json(content)


def save_registry(registry: BoardRegistry, path: Optional[Path] = None) -> Path:
    """Saves BoardRegistry to JSON file and mirrors across candidates."""
    target_path = path or resolve_registry_path()
    target_path.parent.mkdir(parents=True, exist_ok=True)
    json_str = registry.model_dump_json(indent=2) + "\n"
    target_path.write_text(json_str, encoding="utf-8")

    # Mirror to nkai repository and bundle locations if accessible
    mirror_paths = [
        Path(r"c:\Dev\nkai\boards\registry.json"),
        Path(r"c:\Dev\nkai\bundle\boards\registry.json"),
    ]
    for m in mirror_paths:
        if m.resolve() != target_path.resolve():
            try:
                m.parent.mkdir(parents=True, exist_ok=True)
                m.write_text(json_str, encoding="utf-8")
            except Exception:
                pass

    return target_path


def count_html_cards(file_path: Path) -> int:
    """Inspects an HTML board file to determine card count."""
    if not file_path.exists():
        return 0
    try:
        content = file_path.read_text(encoding="utf-8", errors="replace")
        # Match mail-card or research card elements
        cards = re.findall(r'class=["\'][^"\']*\bmail-card\b[^"\']*["\']', content)
        if cards:
            return len(cards)
        research_cards = re.findall(r'class=["\'][^"\']*\btopic-card\b[^"\']*["\']', content)
        if research_cards:
            return len(research_cards)
        generic_cards = re.findall(r'class=["\'][^"\']*\bcard\b[^"\']*["\']', content)
        return len(generic_cards)
    except Exception:
        return 0


def cmd_list(args) -> int:
    """Prints registered boards in a clean tabular format."""
    reg = load_registry()
    print("=" * 80)
    print("🗂️  N'kai Multi-Board Hub Registry")
    print(f"   Version: {reg.version} | Default Board: '{reg.default_board_id}'")
    print(f"   Total Boards: {len(reg.boards)} (Active: {len(reg.active_boards())}, Staging: {len(reg.staging_boards())}, Archived: {len(reg.archived_boards())}, Sandbox: {len(reg.sandbox_boards())})")
    print("=" * 80)

    header = f"{'ID':<20} {'Category':<12} {'Env':<11} {'Cards':<6} {'Title'}"
    print(header)
    print("-" * 80)

    cat_map = {
        "active": "🟢 Active",
        "staging": "🟡 Staging",
        "archived": "📁 Archived",
        "sandbox": "🧪 Sandbox",
    }
    for b in reg.boards:
        cat_badge = cat_map.get(b.category.lower(), b.category)
        env_badge = b.environment.upper()
        cards_str = str(b.card_count)
        print(f"{b.id:<20} {cat_badge:<12} {env_badge:<11} {cards_str:<6} {b.title}")

    print("=" * 80)
    return 0


def cmd_register(args) -> int:
    """Registers or updates a board in the registry."""
    reg = load_registry()
    existing = reg.get_board(args.id)

    card_count = args.card_count
    if card_count is None:
        cand_path = REPO_ROOT / args.file_path
        if cand_path.exists():
            card_count = count_html_cards(cand_path)
        else:
            card_count = existing.card_count if existing else 0

    now_iso = datetime.now().isoformat()
    if existing:
        existing.title = args.title or existing.title
        existing.description = args.description or existing.description
        existing.category = args.category or existing.category
        existing.file_path = args.file_path or existing.file_path
        existing.environment = args.environment or existing.environment
        existing.card_count = card_count
        existing.updated_at = now_iso
        print(f"[✓] Updated board '{args.id}': {existing.title}")
    else:
        new_board = BoardMetadata(
            id=args.id,
            title=args.title,
            description=args.description or "",
            category=args.category or "active",
            file_path=args.file_path,
            environment=args.environment or "PRODUCTION",
            card_count=card_count,
            updated_at=now_iso,
        )
        reg.boards.append(new_board)
        print(f"[✓] Registered new board '{args.id}': {new_board.title}")

    save_registry(reg)
    return 0


def cmd_rescan(args) -> int:
    """Rescans workspace sidecar HTML files and reconciles registry."""
    reg = load_registry()
    print("[*] Rescanning sidecar directory for board templates...")

    search_dirs = [
        REPO_ROOT / ".agents" / "sidecar",
        Path(r"c:\Dev\nkai\sidecars\panel"),
    ]

    known_ids = {b.id: b for b in reg.boards}
    updated_count = 0

    for s_dir in search_dirs:
        if not s_dir.exists():
            continue
        for html_file in s_dir.glob("*.html"):
            if html_file.name in ["master_template.html", "template.html"]:
                continue

            # Derive board ID from filename
            name_stem = html_file.stem
            clean_id = name_stem.replace("_inbox", "").replace("_sidecar", "")

            cards = count_html_cards(html_file)
            rel_path = str(html_file.relative_to(REPO_ROOT)).replace("\\", "/") if html_file.is_relative_to(REPO_ROOT) else str(html_file)

            if clean_id in known_ids:
                b = known_ids[clean_id]
                if b.card_count != cards:
                    b.card_count = cards
                    b.updated_at = datetime.now().isoformat()
                    updated_count += 1
                    print(f"    [~] Updated cards for '{clean_id}': {cards}")
            elif name_stem in known_ids:
                b = known_ids[name_stem]
                if b.card_count != cards:
                    b.card_count = cards
                    b.updated_at = datetime.now().isoformat()
                    updated_count += 1
                    print(f"    [~] Updated cards for '{name_stem}': {cards}")

    save_registry(reg)
    print(f"[✓] Rescan complete. {updated_count} board(s) updated.")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description="N'kai Multi-Board Hub Registry CLI")
    subparsers = parser.add_subparsers(dest="command", help="Sub-commands")

    # list
    subparsers.add_parser("list", help="List registered boards")

    # register
    reg_parser = subparsers.add_parser("register", help="Register or update a board")
    reg_parser.add_argument("id", help="Board identifier (slug)")
    reg_parser.add_argument("title", help="Human-readable title")
    reg_parser.add_argument("file_path", help="Relative path to HTML board file")
    reg_parser.add_argument("--category", choices=["active", "staging", "archived", "sandbox"], default="active", help="Board category")
    reg_parser.add_argument("--environment", choices=["PRODUCTION", "SANDBOX"], default="PRODUCTION", help="Runtime environment")
    reg_parser.add_argument("--description", default="", help="Board summary description")
    reg_parser.add_argument("--card-count", type=int, default=None, help="Explicit card count")

    # rescan
    subparsers.add_parser("rescan", help="Rescan sidecar HTML files and refresh counts")

    args = parser.parse_args()
    if args.command == "list" or not args.command:
        return cmd_list(args)
    elif args.command == "register":
        return cmd_register(args)
    elif args.command == "rescan":
        return cmd_rescan(args)
    else:
        parser.print_help()
        return 1


if __name__ == "__main__":
    sys.exit(main())
