#!/usr/bin/env python3
"""
N'kai "Check the Mail" Vault Inbox Harvester & Interactive Triage Lab (Topic 6.11)
Discovers and extracts raw thoughts, design ideas, and inbox notes from
Obsidian Vault Inbox (or local fallback), generating an interactive N'kai triage
board (morning_mail_inbox.html) or dumping a terminal-friendly Markdown ledger (--chat).

Features:
- Two-Tier Header with live Allowance Meter & Web Audio Haptics toggle
- Curated Test-Drive Docket (12 active topics) + Full Vault Archive (24 notes)
- Procedural Web Audio API sound effects (tick detents & action chimes)
- Real-time instant JavaScript filtering by keyword, status, and tab
- Dynamic token impact estimation for cumulative quota governance
- Tri-mode environment dispatching (NkaiEnv, Clipboard, or Prompt Box)

Zero external dependencies: Python standard library only.
"""

import argparse
import datetime
import html
import json
import os
from pathlib import Path
import re
import shutil
import sys
import webbrowser

# Universal UTF-8 Environment Invariant (Flight 18)
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

# Import Pydantic models for persistent Mailbag session (Flight 5 & 15)
try:
    from tools.models import MailItem, MailQueue, PriorityLevel, TriageStatus, TransactionReceipt, EcosystemPillar, PILLAR_METADATA, infer_pillars, BoardSchema
except ImportError:
    try:
        from models import MailItem, MailQueue, PriorityLevel, TriageStatus, TransactionReceipt, EcosystemPillar, PILLAR_METADATA, infer_pillars, BoardSchema
    except ImportError:
        _script_dir = Path(__file__).resolve().parent
        if str(_script_dir) not in sys.path:
            sys.path.insert(0, str(_script_dir))
        from models import MailItem, MailQueue, PriorityLevel, TriageStatus, TransactionReceipt, EcosystemPillar, PILLAR_METADATA, infer_pillars, BoardSchema

try:
    from tools.board_renderer import render_board
except ImportError:
    try:
        from board_renderer import render_board
    except ImportError:
        _script_dir = Path(__file__).resolve().parent
        if str(_script_dir) not in sys.path:
            sys.path.insert(0, str(_script_dir))
        from board_renderer import render_board

DEFAULT_VAULT_INBOX = Path(r"C:\_\Vault\Inbox")
if not DEFAULT_VAULT_INBOX.exists():
    alt_inbox = Path(r"C:\Dev\TheKlangVault\Inbox")
    if alt_inbox.exists():
        DEFAULT_VAULT_INBOX = alt_inbox

DEFAULT_VAULT_ARCHIVE = Path(r"C:\_\Vault\Archive")
if not DEFAULT_VAULT_ARCHIVE.exists():
    alt_archive = Path(r"C:\Dev\TheKlangVault\Archive")
    if alt_archive.exists():
        DEFAULT_VAULT_ARCHIVE = alt_archive

FALLBACK_LOCAL_INBOX = Path("docs/inbox")
DEFAULT_OUTPUT_HTML = Path(r".agents/sidecar/morning_mail_inbox.html")
DEFAULT_SANDBOX_OUTPUT_HTML = Path(r".agents/sidecar/sandbox_mail_inbox.html")
DEFAULT_SANDBOX_FIXTURE = Path(r"assets/fixtures/sandbox_mail_queue.json")
DEFAULT_DOCKET_PATH = Path(r".agents/sidecar/morning_mail_topics.md")
DEFAULT_MAIL_QUEUE = Path(r".agents/pipeline/mailbag/mail_queue.json")
DEFAULT_TX_RECEIPT = Path(r".agents/pipeline/staging/mail_tx_latest.json")
DEFAULT_REJIGGER_PATH = None  # Retired in Audit Item #5: pure dynamic multi-key sorting
DEFAULT_BACKLOG_PATH = Path("docs/BACKLOG.md")
if not DEFAULT_BACKLOG_PATH.exists():
    alt_backlog = Path(__file__).resolve().parent.parent / "docs" / "BACKLOG.md"
    if alt_backlog.exists():
        DEFAULT_BACKLOG_PATH = alt_backlog


def resolve_archive_directory(inbox_dir: Optional[Path] = None) -> Path:
    """Finds the designated Archive directory outside Inbox."""
    if inbox_dir and (inbox_dir / "Archive").exists():
        return inbox_dir / "Archive"
    if inbox_dir and (inbox_dir.parent / "Archive").exists():
        return inbox_dir.parent / "Archive"
    if DEFAULT_VAULT_ARCHIVE.exists():
        return DEFAULT_VAULT_ARCHIVE
    if DEFAULT_VAULT_INBOX.exists():
        cand = DEFAULT_VAULT_INBOX.parent / "Archive"
        cand.mkdir(parents=True, exist_ok=True)
        return cand
    if inbox_dir:
        cand = inbox_dir.parent / "Archive"
        cand.mkdir(parents=True, exist_ok=True)
        return cand
    return Path.cwd() / "Archive"


def resolve_inbox_directory(dir_override=None, scan_archive=False):
    """Resolves target inbox directory with hierarchical fallbacks."""
    if dir_override:
        target = Path(dir_override).resolve()
        if scan_archive:
            archive_target = resolve_archive_directory(target)
            if archive_target.exists():
                return archive_target
        return target

    if scan_archive:
        if DEFAULT_VAULT_ARCHIVE.exists():
            return DEFAULT_VAULT_ARCHIVE
        return DEFAULT_VAULT_INBOX

    if DEFAULT_VAULT_INBOX.exists():
        return DEFAULT_VAULT_INBOX

    if FALLBACK_LOCAL_INBOX.exists():
        return FALLBACK_LOCAL_INBOX

    return Path.cwd()


def is_obsidian_running() -> bool:
    """Checks if Obsidian desktop app is currently running on Windows."""
    if sys.platform != "win32":
        return True
    try:
        import subprocess
        res = subprocess.run(["tasklist", "/FI", "IMAGENAME eq Obsidian.exe"], capture_output=True, text=True, check=False)
        return "Obsidian.exe" in res.stdout
    except Exception:
        return False


def prewarm_obsidian_sync(vault_name: str = "Vault", wait_seconds: int = 300) -> bool:
    """
    If Obsidian is closed, launches it via URI protocol and pauses to allow
    Obsidian Sync to pull down latest mobile notes into the local inbox.
    """
    if sys.platform != "win32":
        return False
    if is_obsidian_running():
        return False

    print(f"🔄 [PRE-FLIGHT] Obsidian desktop is closed. Launching Obsidian ({vault_name}) to sync mobile notes...")
    try:
        import os
        import time
        # Open vault via native Obsidian URI protocol
        os.startfile(f"obsidian://open?vault={vault_name}")
        print(f"⏳ [PRE-FLIGHT] Waiting {wait_seconds}s ({wait_seconds//60}m) for Obsidian Sync to reconcile remote changes & PDFs...")
        for remaining in range(wait_seconds, 0, -15):
            print(f"   Syncing... {remaining}s remaining")
            time.sleep(min(15, remaining))
        print("✅ [PRE-FLIGHT] Obsidian is running and synchronized.")
        return True
    except Exception as e:
        print(f"⚠️ [PRE-FLIGHT] Could not launch Obsidian automatically: {e}")
        return False


def extract_note_metadata(file_path: Path):
    """Parses frontmatter, title, date, excerpt, and raw body from a Markdown note."""
    try:
        content = file_path.read_text(encoding="utf-8", errors="replace")
    except Exception:
        return None

    title = None
    date_str = None
    body = content
    fm_dict = {}

    # 1. Parse YAML frontmatter if present
    fm_match = re.match(r"^---\s*\r?\n(.*?)\r?\n---\s*\r?\n?(.*)$", content, re.DOTALL)
    if fm_match:
        fm_text = fm_match.group(1)
        body = fm_match.group(2)

        current_key = None
        for line in fm_text.splitlines():
            trimmed = line.strip()
            if not trimmed or trimmed.startswith("#"):
                continue
            if trimmed.startswith("- ") and current_key:
                val = trimmed[2:].strip().strip("'\"")
                if isinstance(fm_dict.get(current_key), list):
                    fm_dict[current_key].append(val)
                else:
                    fm_dict[current_key] = [val] if val else []
                continue
            km = re.match(r"^([a-zA-Z0-9_\-\.]+)\s*:\s*(.*)$", trimmed)
            if km:
                k = km.group(1).lower()
                current_key = k
                v = km.group(2).strip().strip("'\"")
                if v.startswith("[") and v.endswith("]"):
                    items = [x.strip().strip("'\"") for x in v[1:-1].split(",") if x.strip()]
                    fm_dict[k] = items
                elif v:
                    fm_dict[k] = v
                else:
                    fm_dict[k] = []

        if "title" in fm_dict and isinstance(fm_dict["title"], str):
            title = fm_dict["title"].strip()
        if "date" in fm_dict and isinstance(fm_dict["date"], str):
            date_str = fm_dict["date"].strip()

    # 1b. Parse and strip loose/pseudo-frontmatter lines from body
    METADATA_KEYS = {
        "tags", "tag", "date", "priority", "pillars", "pillar", "status",
        "author", "type", "tier", "version", "prereq", "prerequisites", "depends", "depends_on"
    }

    body_clean_lines = []
    for line in body.splitlines():
        trimmed = line.strip()
        km = re.match(r"^([a-zA-Z0-9_\-]+)\s*:\s*(.*)$", trimmed)
        if km and km.group(1).lower() in METADATA_KEYS:
            k = km.group(1).lower()
            v = km.group(2).strip().strip("'\"")
            if v.startswith("[") and v.endswith("]"):
                items = [x.strip().strip("'\"") for x in v[1:-1].split(",") if x.strip()]
                fm_dict[k] = items
            else:
                fm_dict[k] = v
            if k == "date" and not date_str:
                date_str = v
            continue  # Strip metadata line so it never leaks into message text
        body_clean_lines.append(line)
    body = "\n".join(body_clean_lines)

    # Phase 2 Quick Notes: Check if body is empty or whitespace only
    is_quick_note = (not body.strip())

    # 2. Extract title from first Markdown heading if not in frontmatter
    if not title or is_quick_note:
        heading_m = re.search(r"^#\s+(.+)$", body, re.MULTILINE)
        if heading_m and not is_quick_note:
            title = heading_m.group(1).strip()
        else:
            title = file_path.stem.replace("_", " ")

    # 3. Extract creation / modification date
    if not date_str:
        try:
            mtime = file_path.stat().st_mtime
            date_str = datetime.datetime.fromtimestamp(mtime).strftime("%Y-%m-%d")
        except Exception:
            date_str = datetime.date.today().strftime("%Y-%m-%d")

    # 4. Generate clean body excerpt
    if is_quick_note:
        excerpt = "(quick note — title only)"
        clean_body = "(quick note — title only)"
    else:
        clean_lines = []
        for line in body.splitlines():
            trimmed = line.strip()
            if not trimmed or trimmed.startswith("#") or trimmed.startswith("---"):
                continue
            clean_lines.append(trimmed)

        clean_text = " ".join(clean_lines)
        clean_text = re.sub(r"\[(.*?)\]\(.*?\)", r"\1", clean_text)  # strip links
        clean_text = re.sub(r"[*_`]", "", clean_text)  # strip formatting
        clean_text = re.sub(r"\s+", " ", clean_text).strip()

        excerpt = clean_text[:280] + ("..." if len(clean_text) > 280 else "")
        if not excerpt:
            excerpt = "(Empty note body)"
        clean_body = body.strip()

    # Determine priority heuristic
    priority = "P2"
    all_text = (title + " " + body).lower()
    fm_priority = None
    if "priority" in fm_dict:
        fm_priority = str(fm_dict["priority"]).strip().upper()

    if fm_priority in ["P0", "P1", "P2", "P3"]:
        priority = fm_priority
    elif any(k in all_text for k in ["#p0", "priority: p0", "priority:p0", "highest priority", "top priority", "most highest priority"]):
        priority = "P0"
    elif any(k in all_text for k in ["#p1", "priority: p1", "priority:p1"]):
        priority = "P1"
    elif any(k in all_text for k in ["#p3", "priority: p3", "priority:p3"]):
        priority = "P3"

    # Extract tags
    tags = list(set(re.findall(r"#([a-zA-Z0-9_\-/]+)", content)))
    fm_tags = fm_dict.get("tags") or fm_dict.get("tag")
    if fm_tags:
        if isinstance(fm_tags, list):
            for t in fm_tags:
                cleaned = str(t).strip().lstrip("#").lower()
                if cleaned and cleaned not in tags:
                    tags.append(cleaned)
        elif isinstance(fm_tags, str):
            for t in re.split(r"[, ]+", fm_tags):
                cleaned = t.strip().lstrip("#").lower()
                if cleaned and cleaned not in tags:
                    tags.append(cleaned)

    if is_quick_note and "quick-note" not in tags:
        tags.append("quick-note")

    # Phase 3 Tabled: Tag if file resides in a Tabled directory
    if file_path.parent.name.lower() == "tabled":
        if "tabled" not in tags:
            tags.append("tabled")

    # Phase 4 Attention Tags: Scan note frontmatter and body for attention markers
    attention_matches = re.findall(r"!(IMPORTANT|BLOATED|INCOMPLETE|SPIKE)\b", content, re.IGNORECASE)
    for am in attention_matches:
        am_tag = am.lower()
        if am_tag not in tags:
            tags.append(am_tag)

    # Extract dependencies / prerequisites
    dependencies = []
    dep_matches = re.findall(r"(?:prerequisite(?:s|\s+for)?|depends\s+on):\s*([^\n\r]+)", content, re.IGNORECASE)
    for dm in dep_matches:
        cleaned_dep = re.sub(r"[*_`\[\]]", "", dm).strip()
        if cleaned_dep and cleaned_dep not in dependencies:
            dependencies.append(cleaned_dep)

    # Determine default model tier heuristic
    title_lower = title.lower()
    if any(k in title_lower for k in ["dsp", "ladder", "filter", "polyphon", "thread", "concurren", "audio"]):
        tier = "tier1"
    elif any(k in title_lower for k in ["plugin", "lore", "doc", "format", "clean", "naming", "catalog"]):
        tier = "tier3"
    else:
        tier = "tier2"

    # Extract or infer ecosystem target pillars (Flight 6)
    pillars = []
    fm_pil = fm_dict.get("pillars") or fm_dict.get("pillar")
    if fm_pil:
        if isinstance(fm_pil, list):
            pillars = [str(p).strip().upper() for p in fm_pil if str(p).strip().upper() in PILLAR_METADATA]
        elif isinstance(fm_pil, str):
            p_items = [p.strip().strip("'\"").upper() for p in fm_pil.split(",") if p.strip()]
            pillars = [p for p in p_items if p in PILLAR_METADATA]
    if not pillars:
        pillars = infer_pillars(title, tags, body)

    has_domain_tag = any(t in [x.lower() for x in tags] for t in ["d", "dev", "factory", "p", "personal", "abyss", "life", "gear", "f", "forge", "tks", "tt", "nk", "tkr", "agy", "tbd-16"])
    if not has_domain_tag and not (fm_dict.get("pillars") or fm_dict.get("pillar")):
        if "untagged" not in tags:
            tags.append("untagged")

    return {
        "id": file_path.stem,
        "file_name": file_path.name,
        "rel_path": str(file_path),
        "title": title,
        "date": date_str,
        "preview": excerpt,
        "excerpt": excerpt,
        "body": clean_body,
        "char_count": len(clean_body),
        "word_count": len(clean_body.split()),
        "tier": tier,
        "priority": priority,
        "tags": tags,
        "dependencies": dependencies,
        "target_pillars": pillars,
        "frontmatter": fm_dict,
    }


def scan_note(file_path: Path) -> Optional[MailItem]:
    """Parses a single Markdown note into a validated MailItem model."""
    meta = extract_note_metadata(file_path)
    if not meta:
        return None
    return MailItem(
        id=meta.get("id", file_path.stem),
        title=meta.get("title", file_path.stem),
        date=meta.get("date", datetime.date.today().strftime("%Y-%m-%d")),
        created_at=meta.get("date", datetime.date.today().strftime("%Y-%m-%d")),
        priority=meta.get("priority", "P2"),
        status=TriageStatus.PENDING,
        tags=meta.get("tags", []),
        target_pillars=meta.get("target_pillars", []),
        source_file=meta.get("file_name", file_path.name),
        preview=meta.get("preview", ""),
        dependencies=meta.get("dependencies", []),
        char_count=meta.get("char_count", 0),
        word_count=meta.get("word_count", 0),
        tier=meta.get("tier", "tier2"),
        body=meta.get("body", ""),
        excerpt=meta.get("excerpt", ""),
        file_name=meta.get("file_name", file_path.name),
        rel_path=meta.get("rel_path", str(file_path)),
        frontmatter=meta.get("frontmatter", {}),
    )


def scan_inbox(inbox_dir: Path):
    """Scans directory for .md files and extracts metadata."""
    if not inbox_dir.exists():
        return []

    notes = []
    md_files = sorted(inbox_dir.glob("*.md"), key=lambda f: f.stat().st_mtime, reverse=True)
    for f in md_files:
        if f.is_file() and not f.name.startswith(".") and f.name.upper() not in ["README.MD", "INDEX.MD", "ARCHIVE_INDEX.MD", "WELCOME_TO_THE_UNIVERSAL_INBOX.MD", "SCRATCHPAD.MD", "HOUSEHOLD.MD"]:
            data = extract_note_metadata(f)
            if data:
                notes.append(data)
    return notes


def sweep_root_inbox_to_staging(base_dir: Path) -> list:
    """
    Sweeps loose markdown notes from base_dir into base_dir/Staging/,
    injecting or normalizing YAML frontmatter (title, date, staged, priority, pillars, tags, status).
    Preserves existing body content and ignores system/persistent files.
    """
    if not base_dir.exists():
        return []

    staging_dir = base_dir / "Staging"
    staging_dir.mkdir(parents=True, exist_ok=True)

    skip_names = {
        "README.MD", "INDEX.MD", "ARCHIVE_INDEX.MD",
        "WELCOME_TO_THE_UNIVERSAL_INBOX.MD", "SCRATCHPAD.MD", "HOUSEHOLD.MD"
    }

    swept_files = []
    now_str = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    for f in sorted(base_dir.glob("*.md"), key=lambda x: x.stat().st_mtime):
        if not f.is_file() or f.name.startswith(".") or f.name.upper() in skip_names:
            continue

        try:
            content = f.read_text(encoding="utf-8", errors="replace")
            meta = extract_note_metadata(f) or {}

            clean_title = meta.get("title") or f.stem.replace("_", " ")

            # File creation/capture date
            date_val = meta.get("date")
            # Preserve minimal user content without forcing 8-line frontmatter bloat!
            # If the file lacks YAML frontmatter entirely, add a minimal tags block
            full_content = content
            if not content.strip().startswith("---"):
                full_content = f"---\ntags: []\n---\n\n{content.strip()}\n"

            target_path = staging_dir / f.name
            target_path.write_text(full_content, encoding="utf-8")
            f.unlink()
            swept_files.append(target_path)
            print(f"📦 [SWEEP] Staged '{f.name}' -> Staging/{f.name}")
        except Exception as e:
            print(f"⚠️ [SWEEP] Could not sweep '{f.name}': {e}")

    return swept_files


def scan_all_inbox_notes(inbox_dir: Path, scan_archive: bool = False) -> list:
    """
    Scans root inbox and its Tabled/ subfolder, combining and deduping notes by id.
    Only scans Archive/ if scan_archive=True is explicitly passed.
    Live root inbox notes win on content over Archive and Tabled notes.
    """
    if inbox_dir.name.lower() in ["archive", "tabled", "staging", "quicknotes"]:
        base_dir = inbox_dir.parent
    else:
        base_dir = inbox_dir

    # Sweep loose root inbox notes into Staging/
    sweep_root_inbox_to_staging(base_dir)

    tabled_dir = base_dir / "Tabled"
    staging_dir = base_dir / "Staging"
    quick_dir = base_dir / "QuickNotes"
    live_dir = base_dir

    arch_notes = []
    if scan_archive:
        archive_dir = resolve_archive_directory(base_dir)
        if archive_dir and archive_dir.exists():
            arch_notes = scan_inbox(archive_dir)

    tabled_notes = scan_inbox(tabled_dir) if tabled_dir and tabled_dir.exists() else []
    for tn in tabled_notes:
        if "tabled" not in tn.get("tags", []):
            tn["tags"].append("tabled")
        tn["is_tabled"] = True

    staging_notes = scan_inbox(staging_dir) if staging_dir and staging_dir.exists() else []
    for sn in staging_notes:
        if "staged" not in sn.get("tags", []):
            sn["tags"].append("staged")
        sn["is_staged"] = True

    quick_notes = scan_inbox(quick_dir) if quick_dir and quick_dir.exists() else []
    for qn in quick_notes:
        if "quick-note" not in qn.get("tags", []):
            qn["tags"].append("quick-note")
        qn["is_quick_note"] = True

    live_notes = scan_inbox(live_dir) if live_dir and live_dir.exists() and live_dir.resolve() not in (tabled_dir.resolve(), staging_dir.resolve(), quick_dir.resolve()) else []

    notes_by_id = {}
    for n in arch_notes:
        if n.get("id"):
            notes_by_id[str(n["id"])] = n
    for n in tabled_notes:
        if n.get("id"):
            notes_by_id[str(n["id"])] = n
    for n in staging_notes:
        if n.get("id"):
            notes_by_id[str(n["id"])] = n
    for n in quick_notes:
        if n.get("id"):
            notes_by_id[str(n["id"])] = n
    # Live notes overwrite on content
    for n in live_notes:
        if n.get("id"):
            notes_by_id[str(n["id"])] = n

    # Scan secondary vault inbox if distinct and running default vault scan
    if base_dir.resolve() == DEFAULT_VAULT_INBOX.resolve():
        alt_inbox = Path(r"C:\Dev\TheKlangVault\Inbox")
        if alt_inbox.exists() and alt_inbox.resolve() != base_dir.resolve():
            sweep_root_inbox_to_staging(alt_inbox)
            for n in scan_inbox(alt_inbox):
                if n.get("id"):
                    notes_by_id[str(n["id"])] = n
            alt_tabled = alt_inbox / "Tabled"
            if alt_tabled.exists():
                for tn in scan_inbox(alt_tabled):
                    if "tabled" not in tn.get("tags", []):
                        tn["tags"].append("tabled")
                    tn["is_tabled"] = True
                    if tn.get("id"):
                        notes_by_id[str(tn["id"])] = tn
            alt_staging = alt_inbox / "Staging"
            if alt_staging.exists():
                for sn in scan_inbox(alt_staging):
                    if "staged" not in sn.get("tags", []):
                        sn["tags"].append("staged")
                    sn["is_staged"] = True
                    if sn.get("id"):
                        notes_by_id[str(sn["id"])] = sn
            alt_quick = alt_inbox / "QuickNotes"
            if alt_quick.exists():
                for qn in scan_inbox(alt_quick):
                    if "quick-note" not in qn.get("tags", []):
                        qn["tags"].append("quick-note")
                    qn["is_quick_note"] = True
                    if qn.get("id"):
                        notes_by_id[str(qn["id"])] = qn

    return list(notes_by_id.values())


def parse_backlog_flight_statuses(backlog_path: Path = DEFAULT_BACKLOG_PATH) -> dict:
    """
    Scans docs/BACKLOG.md for flight headings and roadmap entries to determine
    the triage status of each flight in Act III.
    Returns: { flight_idx (int): { 'status': 'completed' | 'backlog' | 'untriaged', 'badge': str, 'section': str, 'sec_num': str, 'title': str } }
    """
    if not backlog_path.exists():
        return {}
    try:
        content = backlog_path.read_text(encoding="utf-8")
    except Exception:
        return {}

    statuses = {}

    # Locate Act III runway (Section 15 onwards)
    act3_pos = content.find("### 15. N'kai Sidecar Architecture Roadmap")
    search_content = content[act3_pos:] if act3_pos != -1 else content

    # 1. Parse roadmap check-list items:
    # Example: - [x] **Flight 1: The "Rambles" Ingestion Engine (Topic 2, P0)** — ✅ COMPLETED...
    # Example: - [ ] **Flight 2: N'kai to High Reasoning Prompt Compiler (Topic 5, P1 — TOP ACTIVE PRIORITY)** — ⏳ QUEUED...
    road_matches = re.finditer(
        r'^-\s+\[(x| )\]\s+\*\*Flight\s+(\d+)[:.]\s*([^*]+)\*\*\s*—\s*([^\n]+)$',
        search_content,
        re.MULTILINE
    )
    for m in road_matches:
        checked, f_num, title, status_text = m.groups()
        f_idx = int(f_num)
        st_upper = status_text.upper()
        if "GRAVEYARD" in st_upper or "KILLED" in st_upper:
            statuses[f_idx] = {
                "status": "graveyard",
                "badge": "🪦 GRAVEYARD",
                "section": f"Strategy Graveyard (Flight {f_idx})",
                "sec_num": "",
                "title": title.strip()
            }
        elif checked.lower() == "x" or "COMPLETED" in st_upper or "SHIPPED" in st_upper:
            statuses[f_idx] = {
                "status": "completed",
                "badge": "✅ COMPLETED",
                "section": f"Flight {f_idx}: {title.strip()}",
                "sec_num": "",
                "title": title.strip()
            }
        else:
            statuses[f_idx] = {
                "status": "untriaged",
                "badge": "⚪ UNTRIAGED",
                "section": "",
                "sec_num": "",
                "title": title.strip()
            }

    # 2. Parse dedicated flight sections in Act III:
    # Example: ### 16. Flight 2: N'kai to High Reasoning Prompt Compiler (P1 — TOP ACTIVE PRIORITY)
    # Example: ### 17. Flight 3: N'kai Automode: 2-Pass Iterative Research Daemon & Multi-Drawer Dossier Cards (P1 — TOP PRIORITY)
    # Example: ### 18. Flight 4: General Research Assistant & Configurable Lens Profiles (P2)
    sec_matches = re.finditer(
        r'^###\s+(\d+)\.\s+Flight\s+(\d+)[:.]\s*([^\n]+)$',
        search_content,
        re.MULTILINE
    )
    for m in sec_matches:
        sec_num, f_num, sec_title = m.groups()
        f_idx = int(f_num)
        clean_title = sec_title.strip()
        # If not already completed, a dedicated section means it has been triaged to the BACKLOG!
        if statuses.get(f_idx, {}).get("status") != "completed":
            statuses[f_idx] = {
                "status": "backlog",
                "badge": "✅ BACKLOG",
                "section": f"Section {sec_num}: Flight {f_idx}",
                "sec_num": sec_num,
                "title": clean_title
            }

    return statuses


def parse_docket_file(docket_path: Path, backlog_path: Path = None):
    """Parses curated docket items from morning_mail_topics.md cross-referenced with docs/BACKLOG.md."""
    if not docket_path.exists():
        return []

    try:
        content = docket_path.read_text(encoding="utf-8")
    except Exception:
        return []

    backlog_statuses = {}
    target_backlog = backlog_path or DEFAULT_BACKLOG_PATH
    if target_backlog and target_backlog.exists():
        backlog_statuses = parse_backlog_flight_statuses(target_backlog)

    pattern = r"^#{3,4}\s*(?:[🚀⏳⚡🆕✅🪦❌💀]\s*)?(?:Flight\s+)?(\d+)[:.]\s*([^\n]+)$"
    matches = list(re.finditer(pattern, content, re.MULTILINE))

    items = []
    for i, m in enumerate(matches):
        full_heading = m.group(0)
        raw_title = m.group(2).strip()
        idx = int(m.group(1))

        # Clean title of completed/shipped/graveyard suffix if present
        clean_title = re.sub(r"\s*—\s*(?:SHIPPED|COMPLETED|APPROVED|GRAVEYARD|KILLED)[^—\n]*", "", raw_title, flags=re.IGNORECASE).strip()

        start_pos = m.end()
        end_pos = matches[i + 1].start() if i + 1 < len(matches) else len(content)
        # Stop at next major ## section if present
        next_sec = re.search(r"\n##\s+", content[start_pos:end_pos])
        if next_sec:
            end_pos = start_pos + next_sec.start()

        section_text = content[start_pos:end_pos].strip()

        # Extract source note
        sn_m = re.search(r"-\s*\*\*Source Note(?: / User Prompt)?:\*\*\s*(.+)", section_text)
        source_note = sn_m.group(1).strip() if sn_m else "Conversation & Vibe Coding"

        # Extract executive pitch if explicitly present, else clean excerpt
        pitch_m = re.search(r"-\s*\*\*Executive Pitch:\*\*\s*([^\n\r]+(?:\n[ \t]+[^\n\r]+)*)", section_text)
        if pitch_m:
            raw_pitch = pitch_m.group(1).strip()
            excerpt = re.sub(r"[*_`]", "", raw_pitch)
        else:
            lines = [
                line.strip()
                for line in section_text.splitlines()
                if line.strip() and not line.strip().startswith("#") and not line.strip().startswith("---")
                and not re.match(r"^-\s*\*\*(?:Source Note|Tier|Priority|Pillars|Dependencies|Tags)", line.strip())
            ]
            clean_lines = [re.sub(r"[*_`]", "", l) for l in lines]
            excerpt = " ".join(clean_lines[:3])[:280] + ("..." if len(" ".join(clean_lines[:3])) > 280 else "")
            if not excerpt:
                excerpt = " ".join([re.sub(r"[*_`]", "", l) for l in section_text.splitlines() if l.strip()][:3])[:280]

        # Model tier classification (explicit metadata first, keyword fallback second)
        tier_m = re.search(r"\bTier:\s*(?:Tier\s*)?(\d+|pro|flash|minimal)", section_text, re.IGNORECASE)
        if tier_m:
            t_val = tier_m.group(1).lower()
            if t_val in ["1", "pro"]:
                tier = "tier1"
            elif t_val in ["3", "minimal"]:
                tier = "tier3"
            else:
                tier = "tier2"
        else:
            t_low = clean_title.lower()
            if any(k in t_low for k in ["ladder filter", "audio filter", "filter dsp", "dsp", "saturation", "differential", "polyphon", "audio thread"]):
                tier = "tier1"
            elif any(k in t_low for k in ["rebranding", "lore", "cheat sheet", "plugins for the knowledge", "use cases", "lens profiles", "pill", "counter"]):
                tier = "tier3"
            else:
                tier = "tier2"

        # Priority detection
        all_docket_text = (clean_title + " " + section_text).lower()
        if any(k in all_docket_text for k in ["#p0", "priority: p0", "priority:p0", "highest priority", "top priority", "most highest priority"]):
            priority = "P0"
        elif any(k in all_docket_text for k in ["#p1", "priority: p1", "priority:p1"]):
            priority = "P1"
        elif any(k in all_docket_text for k in ["#p3", "priority: p3", "priority:p3"]):
            priority = "P3"
        else:
            priority = "P2"

        # Dependencies
        dependencies = []
        dep_matches = re.findall(r"(?:prerequisite(?:s|\s+for)?|depends\s+on):\s*([^\n\r]+)", section_text, re.IGNORECASE)
        for dm in dep_matches:
            cleaned_dep = re.sub(r"[*_`\[\]]", "", dm).strip()
            if cleaned_dep and cleaned_dep not in dependencies:
                dependencies.append(cleaned_dep)

        tags = list(set(re.findall(r"#([a-zA-Z0-9_\-/]+)", section_text)))

        # Status resolution from docs/BACKLOG.md
        heading_completed = "✅" in full_heading or any(s in raw_title.lower() for s in ["completed", "shipped", "approved"])
        b_info = backlog_statuses.get(idx)
        if b_info:
            status = b_info["status"]
            badge_text = b_info["badge"]
            section_info = b_info.get("section", "")
            sec_num = b_info.get("sec_num", "")
        elif heading_completed:
            status = "completed"
            badge_text = "✅ COMPLETED"
            section_info = f"Flight {idx}"
            sec_num = ""
        else:
            status = "untriaged"
            badge_text = "⚪ UNTRIAGED"
            section_info = ""
            sec_num = ""

        target_pillars = infer_pillars(clean_title, tags, section_text)

        items.append({
            "id": f"docket-{idx}",
            "idx": idx,
            "title": clean_title,
            "source_note": source_note,
            "excerpt": excerpt,
            "body": section_text,
            "word_count": len(section_text.split()),
            "date": "2026-10-09",
            "tier": tier,
            "priority": priority,
            "tags": tags,
            "dependencies": dependencies,
            "target_pillars": target_pillars,
            "status": status,
            "badge_text": badge_text,
            "section_info": section_info,
            "sec_num": sec_num,
        })

    return items


def render_chat_summary(inbox_dir: Path, notes: list, docket_items: list = None):
    """Outputs a clean, terminal-friendly Markdown summary (--chat standard)."""
    lines = [
        f"# 📬 N'kai Morning Mail Summary",
        f"**Source:** `{inbox_dir}`",
        f"**Generated:** {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}",
        "",
    ]

    if docket_items:
        lines.append(f"## 🌟 Active Test-Drive Docket ({len(docket_items)} Topics)")
        for it in docket_items:
            lines.append(f"### #{it['idx']}. {it['title']}")
            lines.append(f"- **Source:** {it['source_note']} | **Tier:** {it['tier'].upper()} | **Words:** {it['word_count']}")
            lines.append(f"- **Excerpt:** > {it['excerpt']}")
            lines.append("- **Quick Actions:** `[📥 Backlog]` `[🔬 Research]` `[🪦 Graveyard]` `[⚖️ PCDA]`")
            lines.append("")

    if notes:
        lines.append(f"## 📦 Vault Archive ({len(notes)} Notes)")
        for idx, note in enumerate(notes, 1):
            lines.append(f"### {idx}. {note['title']}")
            lines.append(f"- **File:** `{note['file_name']}` | **Date:** {note['date']} | **Words:** {note['word_count']}")
            lines.append(f"- **Excerpt:** > {note['excerpt']}")
            lines.append("")

    if not docket_items and not notes:
        lines.append("_No unhandled inbox notes found. Mailbox is completely clean!_")

    return "\n".join(lines)


def build_mail_queue(
    notes: list,
    docket_items: list,
    queue_path: Path,
    rejigger_path: Optional[Any] = None,
    use_rejigger: bool = False
) -> MailQueue:
    """
    Constructs a validated MailQueue from raw notes and docket items, applying the
    structural rejigger overlay if present, enforcing reverse-chronological sorting
    for raw queues or rank ordering for overlay queues, snapshots the prior ledger,
    and serializes the session ledger to queue_path.
    """
    # 0. Resolve rejigger overlay path
    overlay_path = None
    if use_rejigger and rejigger_path is not False:
        if rejigger_path:
            p = Path(rejigger_path)
            if p.is_absolute():
                if p.exists():
                    overlay_path = p
            else:
                for base in [Path.cwd(), Path(__file__).resolve().parent.parent]:
                    cand = base / p
                    if cand.exists():
                        overlay_path = cand
                        break
        elif DEFAULT_REJIGGER_PATH:
            for base in [Path.cwd(), Path(__file__).resolve().parent.parent]:
                cand = base / DEFAULT_REJIGGER_PATH
                if cand.exists():
                    overlay_path = cand
                    break

    # Preserve existing triage status and verdicts from active ledger
    existing_by_id = {}
    if queue_path.exists():
        try:
            old_data = json.loads(queue_path.read_text(encoding="utf-8"))
            for old_it in old_data.get("items", []):
                if old_it.get("id"):
                    existing_by_id[str(old_it["id"])] = old_it
        except Exception:
            pass

    raw_items = []

    # 1. Ingest docket items
    for it in docket_items:
        it_id = str(it.get("id") or f"docket-{it.get('idx', 0)}")
        old_it = existing_by_id.get(it_id)
        it_status = it.get("status", "untriaged")
        m_status = TriageStatus.PENDING
        if it_status == "backlog":
            m_status = TriageStatus.BACKLOG
        elif it_status == "completed":
            m_status = TriageStatus.COMPLETED

        verdict = None
        verdict_notes = None
        if old_it:
            old_st = old_it.get("status")
            if old_st and old_st != "PENDING" and it_status == "untriaged":
                try:
                    m_status = TriageStatus(old_st)
                except ValueError:
                    pass
            verdict = old_it.get("verdict")
            verdict_notes = old_it.get("verdict_notes")

        agent_reply = old_it.get("agent_reply") if old_it else None
        agent_reply_at = old_it.get("agent_reply_at") if old_it else None
        has_reply = bool(agent_reply and str(agent_reply).strip())

        raw_items.append({
            "id": it_id,
            "title": str(it.get("title", "Untitled Docket Topic")),
            "created_at": str(it.get("date") or "2026-10-09"),
            "date": str(it.get("date") or "2026-10-09"),
            "priority": it.get("priority", "P2"),
            "status": m_status,
            "tags": it.get("tags", []),
            "target_pillars": it.get("target_pillars", []),
            "source_file": str(it.get("source_note", "")),
            "source_note": str(it.get("source_note", "")),
            "preview": str(it.get("excerpt", "")),
            "excerpt": str(it.get("excerpt", "")),
            "dependencies": it.get("dependencies", []),
            "verdict": verdict,
            "verdict_notes": verdict_notes,
            "char_count": len(str(it.get("body", ""))),
            "word_count": int(it.get("word_count", 0)),
            "tier": str(it.get("tier", "tier2")),
            "body": str(it.get("body", "")),
            "file_name": "",
            "rel_path": str(it.get("source_note", "")),
            "idx": int(it.get("idx", 0)),
            "split_group": str(it.get("split_group", "")),
            "agent_reply": agent_reply,
            "agent_reply_at": agent_reply_at,
            "has_reply": has_reply,
            "frontmatter": it.get("frontmatter", {}),
        })

    # 1b. Collect existing synthesized cards (e.g. ramble cards) to preserve them across runs
    existing_synthesized_items = []
    existing_synthesized_sources = set()
    for it_id, old_it in existing_by_id.items():
        is_ramble = it_id.startswith("ramble-") or old_it.get("src_type") == "ramble" or "ramble" in old_it.get("tags", [])
        if is_ramble:
            try:
                syn_item = MailItem.model_validate(old_it)
                existing_synthesized_items.append(syn_item)
                if syn_item.file_name:
                    existing_synthesized_sources.add(syn_item.file_name.lower().strip())
                if syn_item.source_file:
                    existing_synthesized_sources.add(Path(syn_item.source_file).name.lower().strip())
                    existing_synthesized_sources.add(Path(syn_item.source_file).stem.lower().strip())
                if syn_item.split_group:
                    existing_synthesized_sources.add(syn_item.split_group.lower().strip())
            except Exception:
                pass

    # Collect known docket source note names to avoid duplicate cards
    docket_source_names = set()
    for it in docket_items:
        sn = str(it.get("source_note", ""))
        m = re.search(r"\[`?([^`\]]+)`?\]", sn)
        if m:
            docket_source_names.add(m.group(1).lower().strip())
        docket_source_names.add(Path(sn).name.lower().strip())
        docket_source_names.add(str(it.get("title", "")).lower().strip())

    # 2. Ingest notes (skipping those already formalized as curated docket items or synthesized cards)
    for n in notes:
        fn = str(n.get("file_name", "")).lower().strip()
        stem = Path(fn).stem.lower().strip()
        t = str(n.get("title", "")).lower().strip()
        if fn in docket_source_names or stem in docket_source_names or t in docket_source_names:
            continue
        if fn in existing_synthesized_sources or stem in existing_synthesized_sources:
            continue

        item_id = str(n.get("id") or n.get("file_name", "note"))
        old_it = existing_by_id.get(item_id)
        note_status = TriageStatus.PENDING
        verdict = None
        verdict_notes = None
        if old_it:
            old_st = old_it.get("status")
            if old_st and old_st not in ("PENDING", "STAGED"):
                try:
                    note_status = TriageStatus(old_st)
                except ValueError:
                    pass
            verdict = old_it.get("verdict")
            verdict_notes = old_it.get("verdict_notes")

        fm_dict = n.get("frontmatter", {}) if isinstance(n.get("frontmatter"), dict) else {}
        fm_status = str(fm_dict.get("status", "")).lower().strip()

        if not verdict:
            if n.get("is_tabled") or "tabled" in [t.lower() for t in n.get("tags", [])] or fm_status == "tabled":
                note_status = TriageStatus.TABLED
            elif fm_status == "staged":
                note_status = TriageStatus.STAGED
            elif fm_status == "untriaged" or note_status in (TriageStatus.PENDING, TriageStatus.STAGED):
                note_status = TriageStatus.PENDING

        agent_reply = old_it.get("agent_reply") if old_it else None
        agent_reply_at = old_it.get("agent_reply_at") if old_it else None
        has_reply = bool(agent_reply and str(agent_reply).strip())

        raw_items.append({
            "id": item_id,
            "title": str(n.get("title", "Untitled Note")),
            "created_at": str(n.get("date") or datetime.date.today().strftime("%Y-%m-%d")),
            "date": str(n.get("date") or datetime.date.today().strftime("%Y-%m-%d")),
            "priority": n.get("priority", "P2"),
            "status": note_status,
            "tags": n.get("tags", []),
            "target_pillars": n.get("target_pillars", []),
            "source_file": str(n.get("rel_path") or n.get("file_name", "")),
            "preview": str(n.get("excerpt", "")),
            "excerpt": str(n.get("excerpt", "")),
            "dependencies": n.get("dependencies", []),
            "verdict": verdict,
            "verdict_notes": verdict_notes,
            "char_count": int(n.get("char_count", 0)),
            "word_count": int(n.get("word_count", 0)),
            "tier": str(n.get("tier", "tier2")),
            "body": str(n.get("body", "")),
            "file_name": str(n.get("file_name", "")),
            "rel_path": str(n.get("rel_path", "")),
            "source_note": "",
            "idx": 0,
            "split_group": str(n.get("split_group", "")),
            "agent_reply": agent_reply,
            "agent_reply_at": agent_reply_at,
            "has_reply": has_reply,
            "frontmatter": n.get("frontmatter", {}),
        })

    # 3. Check for rejigger overlay
    if overlay_path and overlay_path.exists():
        try:
            rej_data = json.loads(overlay_path.read_text(encoding="utf-8"))
            cards = rej_data.get("cards", [])
        except Exception as e:
            print(f"⚠️ Failed to parse rejigger overlay at {overlay_path}: {e}")
            cards = []

        if cards:
            raw_by_id = {str(it["id"]): it for it in raw_items if it.get("id")}
            raw_by_title = {str(it["title"]).lower().strip(): it for it in raw_items if it.get("title")}

            referenced_raw_ids = set()
            overlay_mail_items = []

            for card in cards:
                card_id = str(card.get("id"))
                card_title = str(card.get("title", ""))
                card_rank = int(card.get("rank", 0))
                card_summary = str(card.get("summary", ""))
                angles = card.get("angles", [])
                next_action = str(card.get("next_action", ""))

                matched_raw_items = []
                for src in card.get("sources", []):
                    src_str = str(src).strip()
                    matched = None
                    if src_str in raw_by_id:
                        matched = raw_by_id[src_str]
                    elif src_str.lower() in raw_by_title:
                        matched = raw_by_title[src_str.lower()]
                    if matched:
                        matched_raw_items.append(matched)
                        referenced_raw_ids.add(str(matched["id"]))

                raw_match = raw_by_id.get(card_id)
                if card_title == card_id and raw_match:
                    card_title = str(raw_match.get("title") or card_title)

                is_placeholder_summary = card_summary.strip().lower() in ["completed.", "in backlog.", "graveyard."]

                if is_placeholder_summary and raw_match:
                    body = str(raw_match.get("body", ""))
                    preview = str(raw_match.get("excerpt") or raw_match.get("preview") or card_summary)
                    excerpt = preview
                    word_count = int(raw_match.get("word_count") or len(body.split()))
                    char_count = len(body)
                    source_file = str(raw_match.get("source_file") or raw_match.get("rel_path") or "")
                    file_name = str(raw_match.get("file_name", ""))
                    rel_path = str(raw_match.get("rel_path", ""))
                    source_note = str(raw_match.get("source_note", ""))
                    created_at = str(raw_match.get("date") or raw_match.get("created_at") or "2026-10-09")
                else:
                    body_parts = [card_summary]
                    if angles:
                        body_parts.append("\n".join(f"- {a}" for a in angles))
                    if next_action:
                        body_parts.append(f"**Next action:** {next_action}")
                    all_sources = card.get("sources", [])
                    if all_sources or matched_raw_items:
                        source_lines = ["**Source notes:**"]
                        matched_by_key = {}
                        for r in matched_raw_items:
                            r_id = str(r.get("id") or "").lower().strip()
                            r_title = str(r.get("title") or "").lower().strip()
                            r_fn = Path(str(r.get("file_name") or "")).stem.lower().strip()
                            matched_by_key[r_id] = r
                            matched_by_key[r_title] = r
                            if r_fn:
                                matched_by_key[r_fn] = r

                        seen_sources = set()
                        for src in all_sources:
                            src_str = str(src).strip()
                            if not src_str or src_str.lower() in seen_sources:
                                continue
                            seen_sources.add(src_str.lower())
                            r = matched_by_key.get(src_str.lower())
                            if r:
                                r_title = str(r.get("title") or r.get("id") or src_str)
                                r_path = str(r.get("rel_path") or r.get("source_file") or "")
                                m_link = re.search(r'\((file:///[^)]+)\)', r_path)
                                file_url = m_link.group(1) if m_link else ("file:///" + r_path.replace("\\", "/") if r_path else "")
                                if file_url:
                                    source_lines.append(f"- [{r_title}]({file_url})")
                                else:
                                    source_lines.append(f"- {r_title}")
                            else:
                                # Check Archive locations
                                cand_paths = [
                                    Path(r"C:\_\Vault\Archive") / f"{src_str}.md",
                                    Path(r"C:\_\Vault\Archive") / src_str,
                                    Path(r"C:\Dev\TheKlangVault\Archive") / f"{src_str}.md",
                                    Path(r"C:\Dev\TheKlangVault\Archive") / src_str,
                                ]
                                found_path = None
                                for cp in cand_paths:
                                    if cp.exists():
                                        found_path = cp
                                        break
                                if found_path:
                                    file_url = "file:///" + str(found_path).replace("\\", "/")
                                    source_lines.append(f"- [{src_str}]({file_url}) *(archived)*")
                                else:
                                    source_lines.append(f"- {src_str}")

                        for r in matched_raw_items:
                            r_id = str(r.get("id") or "")
                            if r_id.lower() not in seen_sources:
                                seen_sources.add(r_id.lower())
                                r_title = str(r.get("title") or r_id)
                                r_path = str(r.get("rel_path") or r.get("source_file") or "")
                                file_url = "file:///" + r_path.replace("\\", "/") if r_path else ""
                                source_lines.append(f"- [{r_title}]({file_url})" if file_url else f"- {r_title}")

                        body_parts.append("\n".join(source_lines))

                    # Relational cross-card linking (Phase 5 / rejiggering feedback)
                    related = card.get("related_cards", [])
                    deps = card.get("depends_on", [])
                    if related or deps:
                        rel_lines = ["**Related topics:**"]
                        for d in deps:
                            rel_lines.append(f"- ⚠️ Prereq: `{d}`")
                        for rc in related:
                            if rc not in deps:
                                rel_lines.append(f"- 🔗 Related: `{rc}`")
                        body_parts.append("\n".join(rel_lines))

                    body = "\n\n".join(p for p in body_parts if p.strip())
                    preview = card_summary
                    excerpt = card_summary
                    word_count = len(body.split())
                    char_count = len(body)
                    source_file = str(matched_raw_items[0].get("source_file") or matched_raw_items[0].get("rel_path") or "") if matched_raw_items else ""
                    file_name = str(matched_raw_items[0].get("file_name", "")) if matched_raw_items else ""
                    rel_path = str(matched_raw_items[0].get("rel_path", "")) if matched_raw_items else ""
                    source_note = str(matched_raw_items[0].get("source_note", "")) if matched_raw_items else ""
                    created_at = str(matched_raw_items[0].get("date") or matched_raw_items[0].get("created_at") or "2026-10-11") if matched_raw_items else "2026-10-11"

                # Status resolution: existing ledger wins on non-PENDING, else card status
                old_it = existing_by_id.get(card_id)
                card_status = card.get("status", "PENDING")
                final_status = card_status
                verdict = card.get("verdict")
                verdict_notes = card.get("verdict_notes")
                if old_it:
                    old_st = old_it.get("status")
                    if old_st and old_st != "PENDING":
                        final_status = old_st
                    if old_it.get("verdict"):
                        verdict = old_it.get("verdict")
                    if old_it.get("verdict_notes"):
                        verdict_notes = old_it.get("verdict_notes")

                try:
                    status_enum = TriageStatus(final_status)
                except ValueError:
                    status_enum = TriageStatus.PENDING

                merged_count = len(matched_raw_items)
                source_ids = [str(r.get("id")) for r in matched_raw_items if r.get("id")]
                tags = list(card.get("tags", []))
                if merged_count > 1 and f"merged:{merged_count}" not in tags:
                    tags.append(f"merged:{merged_count}")

                agent_reply = old_it.get("agent_reply") if old_it else None
                agent_reply_at = old_it.get("agent_reply_at") if old_it else None
                has_reply = bool(agent_reply and str(agent_reply).strip())

                m_item = MailItem(
                    id=card_id,
                    title=card_title,
                    created_at=created_at,
                    priority=card.get("priority", "P2"),
                    status=status_enum,
                    tags=tags,
                    target_pillars=card.get("pillars", []),
                    source_file=source_file,
                    preview=preview,
                    dependencies=card.get("depends_on", []),
                    verdict=verdict,
                    verdict_notes=verdict_notes,
                    char_count=char_count,
                    word_count=word_count,
                    tier=card.get("tier", "tier2"),
                    body=body,
                    excerpt=excerpt,
                    file_name=file_name,
                    rel_path=rel_path,
                    source_note=source_note,
                    idx=card_rank,
                    merged_count=merged_count,
                    source_ids=source_ids,
                    agent_reply=agent_reply,
                    agent_reply_at=agent_reply_at,
                    has_reply=has_reply,
                )
                overlay_mail_items.append(m_item)

            # Sort overlay cards by rank ascending
            overlay_mail_items.sort(key=lambda x: int(x.idx))

            # Collect unreferenced raw items (new mail)
            unreferenced_raw_items = []
            for r in raw_items:
                r_id = str(r.get("id"))
                if r_id not in referenced_raw_ids:
                    r_title = str(r.get("title", "Untitled Note"))
                    if not r_title.startswith("🆕 "):
                        r_title = f"🆕 {r_title}"

                    old_it = existing_by_id.get(r_id)
                    r_status = r.get("status", TriageStatus.PENDING)
                    verdict = None
                    verdict_notes = None
                    if old_it:
                        old_st = old_it.get("status")
                        if old_st and old_st != "PENDING":
                            try:
                                r_status = TriageStatus(old_st)
                            except ValueError:
                                pass
                        verdict = old_it.get("verdict")
                        verdict_notes = old_it.get("verdict_notes")

                    agent_reply = old_it.get("agent_reply") if old_it else r.get("agent_reply")
                    agent_reply_at = old_it.get("agent_reply_at") if old_it else r.get("agent_reply_at")
                    has_reply = bool(agent_reply and str(agent_reply).strip())

                    unref_item = MailItem(
                        id=r_id,
                        title=r_title,
                        created_at=str(r.get("date") or r.get("created_at") or datetime.date.today().strftime("%Y-%m-%d")),
                        priority=r.get("priority", "P2"),
                        status=r_status,
                        tags=r.get("tags", []),
                        target_pillars=r.get("target_pillars", []),
                        source_file=str(r.get("source_file") or r.get("rel_path") or ""),
                        preview=str(r.get("preview") or r.get("excerpt") or ""),
                        dependencies=r.get("dependencies", []),
                        verdict=verdict,
                        verdict_notes=verdict_notes,
                        char_count=int(r.get("char_count", 0)),
                        word_count=int(r.get("word_count", 0)),
                        tier=str(r.get("tier", "tier2")),
                        body=str(r.get("body", "")),
                        excerpt=str(r.get("excerpt", "")),
                        file_name=str(r.get("file_name", "")),
                        rel_path=str(r.get("rel_path", "")),
                        source_note=str(r.get("source_note", "")),
                        idx=0,
                        merged_count=1,
                        source_ids=[r_id],
                        agent_reply=agent_reply,
                        agent_reply_at=agent_reply_at,
                        has_reply=has_reply,
                    )
                    unreferenced_raw_items.append(unref_item)

            # Order unreferenced new items by priority/date
            pri_rank = {PriorityLevel.P0: 0, PriorityLevel.P1: 1, PriorityLevel.P2: 2, PriorityLevel.P3: 3}
            unreferenced_raw_items.sort(key=lambda x: str(x.created_at or ""), reverse=True)
            unreferenced_raw_items.sort(key=lambda x: pri_rank.get(x.priority, 2))

            final_items = overlay_mail_items + unreferenced_raw_items + existing_synthesized_items
            queue = MailQueue(
                schema_version="1.0.0",
                last_scanned=datetime.datetime.now().isoformat(),
                total_items=len(final_items),
                p0_count=sum(1 for it in final_items if it.priority == PriorityLevel.P0),
                items=final_items,
            )
            # Enforce dynamic priority, status, and date sorting with contiguous 1-based indexing (Flight 5)
            queue.sort_by_priority()

            # Snapshot existing ledger before overwriting
            if queue_path.exists():
                snap_dir = queue_path.parent / "snapshots"
                snap_dir.mkdir(parents=True, exist_ok=True)
                prev_snap = snap_dir / "mail_queue_prev.json"
                try:
                    shutil.copy2(str(queue_path), str(prev_snap))
                except Exception as e:
                    print(f"⚠️ Could not create snapshot: {e}")

            queue_path.parent.mkdir(parents=True, exist_ok=True)
            queue_path.write_text(queue.model_dump_json(indent=2), encoding="utf-8")
            return queue

    # 4. Standard raw queue if no overlay
    std_items = []
    for r in raw_items:
        std_items.append(MailItem(
            id=str(r["id"]),
            title=str(r["title"]),
            created_at=str(r.get("created_at") or r.get("date") or "2026-10-11"),
            priority=r.get("priority", "P2"),
            status=r.get("status", TriageStatus.PENDING),
            tags=r.get("tags", []),
            target_pillars=r.get("target_pillars", []),
            source_file=str(r.get("source_file") or r.get("rel_path") or ""),
            preview=str(r.get("preview") or r.get("excerpt") or ""),
            dependencies=r.get("dependencies", []),
            verdict=r.get("verdict"),
            verdict_notes=r.get("verdict_notes"),
            char_count=int(r.get("char_count", 0)),
            word_count=int(r.get("word_count", 0)),
            tier=str(r.get("tier", "tier2")),
            body=str(r.get("body", "")),
            excerpt=str(r.get("excerpt", "")),
            file_name=str(r.get("file_name", "")),
            rel_path=str(r.get("rel_path", "")),
            source_note=str(r.get("source_note", "")),
            idx=int(r.get("idx", 0)),
            agent_reply=r.get("agent_reply"),
            agent_reply_at=r.get("agent_reply_at"),
            has_reply=bool(r.get("agent_reply") and str(r.get("agent_reply")).strip()),
        ))

    # Append preserved synthesized items (e.g. decomposed ramble cards)
    std_items.extend(existing_synthesized_items)

    queue = MailQueue(
        schema_version="1.0.0",
        last_scanned=datetime.datetime.now().isoformat(),
        items=std_items,
    )
    # Enforces reverse-chronological sorting for P0 items at top
    queue.sort_by_priority()

    # Snapshot existing ledger before overwriting
    if queue_path.exists():
        snap_dir = queue_path.parent / "snapshots"
        snap_dir.mkdir(parents=True, exist_ok=True)
        prev_snap = snap_dir / "mail_queue_prev.json"
        try:
            shutil.copy2(str(queue_path), str(prev_snap))
        except Exception as e:
            print(f"⚠️ Could not create snapshot: {e}")

    # Ensure output directory exists and write ledger
    queue_path.parent.mkdir(parents=True, exist_ok=True)
    queue_path.write_text(queue.model_dump_json(indent=2), encoding="utf-8")
    return queue




def update_archive_index(archive_dir: Path):
    """
    Generates a master markdown index (INDEX.md / README.md) of all notes in Archive/
    with clickable links, dates, priorities, and ecosystem pillars for rapid O(1) lookup.
    """
    md_files = sorted(archive_dir.glob("*.md"), key=lambda f: f.stat().st_mtime, reverse=True)
    real_files = [f for f in md_files if f.name.upper() not in ["README.MD", "INDEX.MD", "ARCHIVE_INDEX.MD"]]
    lines = [
        "# 🗄️ Obsidian Inbox Archive Index",
        "> Automatically maintained by N'kai Harvester (`tools/check_mail.py`).",
        f"> **Total Archived Notes:** {len(real_files)} | **Last Updated:** {datetime.datetime.now().strftime('%Y-%m-%d %H:%M')}",
        "",
        "| Note Title | Captured Date | Priority | Pillar | File Link |",
        "| :--- | :--- | :--- | :--- | :--- |",
    ]
    for f in real_files:
        meta = extract_note_metadata(f)
        if meta:
            title = meta.get("title") or f.stem.replace("_", " ")
            date_str = meta.get("date") or ""
            pri = meta.get("priority") or "P2"
            pillars = meta.get("target_pillars") or ["TKS"]
            pillar_badges = []
            for p in pillars[:2]:
                m = PILLAR_METADATA.get(p.upper(), {})
                emoji = m.get("emoji", "🏷️")
                pillar_badges.append(f"{emoji} {p.upper()}")
            if len(pillars) > 2:
                pillar_badges.append(f"+{len(pillars) - 2}")
            pillar_str = " ".join(pillar_badges)
            lines.append(f"| **{title}** | `{date_str}` | `{pri}` | {pillar_str} | [{f.name}]({f.name}) |")

    content = "\n".join(lines) + "\n"
    (archive_dir / "INDEX.md").write_text(content, encoding="utf-8")
    (archive_dir / "README.md").write_text(content, encoding="utf-8")


def archive_harvested_notes(inbox_dir: Path, notes: list, receipt_path: Path, queue: MailQueue, queue_path: Path):
    """
    Moves all harvested markdown notes from the root inbox into inbox_dir/Archive,
    ensuring clean YAML frontmatter, updating the MailQueue ledger, writing an atomic
    TransactionReceipt for 5ms rollback, and compiling the master archive index.
    """
    archive_dir = inbox_dir / "Archive" if inbox_dir.name.lower() != "archive" else inbox_dir
    archive_dir.mkdir(parents=True, exist_ok=True)
    live_dir = inbox_dir if inbox_dir.name.lower() != "archive" else inbox_dir.parent

    moved_files = []
    for note in notes:
        rel_p = note.get("rel_path")
        if not rel_p:
            continue
        src = Path(rel_p).resolve()
        # Only archive files that are directly in live_dir or Staging/QuickNotes (not already in Archive)
        valid_parents = (live_dir.resolve(), (live_dir / "Staging").resolve(), (live_dir / "QuickNotes").resolve())
        if src.parent.resolve() in valid_parents and src.parent.resolve() != archive_dir.resolve():
            safe_name = src.name.replace(" ", "_")
            dst = archive_dir / safe_name
            try:
                # 1. Inject or update clean YAML frontmatter with archived timestamp
                text_content = src.read_text(encoding="utf-8", errors="replace")
                arch_now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

                fm_m = re.match(r"^---\s*\r?\n(.*?)\r?\n---\s*\r?\n?(.*)$", text_content, re.DOTALL)
                if fm_m:
                    fm_body = fm_m.group(1)
                    note_rest = fm_m.group(2)
                    if "archived:" not in fm_body:
                        fm_body += f'\narchived: "{arch_now}"'
                    fm_body = re.sub(r'status:\s*["\']?[^"\']*\n?', 'status: "archived"\n', fm_body)
                    src.write_text(f"---\n{fm_body.strip()}\n---\n\n{note_rest.lstrip()}", encoding="utf-8")
                else:
                    clean_title = note.get("title") or src.stem.replace("_", " ")
                    note_date = note.get("date") or datetime.date.today().strftime("%Y-%m-%d")
                    note_pri = note.get("priority") or "P2"
                    note_pillars = note.get("target_pillars") or infer_pillars(clean_title, note.get("tags", []), text_content)
                    pil_json = json.dumps(note_pillars)
                    frontmatter = f"---\ntitle: \"{clean_title}\"\ndate: \"{note_date}\"\nstaged: \"{note_date}\"\narchived: \"{arch_now}\"\npriority: \"{note_pri}\"\ntags: [\"inbox\", \"archived\"]\npillars: {pil_json}\nstatus: \"archived\"\n---\n\n"
                    src.write_text(frontmatter + text_content, encoding="utf-8")

                shutil.move(str(src), str(dst))
                moved_files.append({"src": str(src), "dst": str(dst), "name": safe_name})
                note["rel_path"] = str(dst)
                note["source_file"] = str(dst)
                # Update item in queue as well
                for it in queue.items:
                    if it.file_name in [src.name, safe_name] or it.id in [src.stem, dst.stem] or it.title == note.get("title"):
                        it.source_file = str(dst)
                        it.rel_path = str(dst)
                        it.file_name = safe_name
            except Exception as e:
                print(f"⚠️ Failed to move {src.name} to archive: {e}")

    if moved_files:
        # Snapshot prior ledger before save
        if queue_path.exists():
            snap_dir = queue_path.parent / "snapshots"
            snap_dir.mkdir(parents=True, exist_ok=True)
            try:
                shutil.copy2(str(queue_path), str(snap_dir / "mail_queue_prev.json"))
            except Exception:
                pass

        # Save updated queue
        queue_path.write_text(queue.model_dump_json(indent=2), encoding="utf-8")

        # Compile master archive index
        try:
            update_archive_index(archive_dir)
        except Exception as e:
            print(f"⚠️ Failed to update archive index: {e}")

        # Write atomic rollback receipt
        receipt_data = {
            "tx_id": f"mail-arch-{int(datetime.datetime.now().timestamp())}",
            "timestamp": datetime.datetime.now().isoformat(),
            "action": "archive_harvested",
            "note_id": f"{len(moved_files)} files archived",
            "prev_status": "PENDING",
            "new_status": "ARCHIVED",
            "moved_files": moved_files,
            "can_rollback": True,
        }
        receipt_path.parent.mkdir(parents=True, exist_ok=True)
        receipt_path.write_text(json.dumps(receipt_data, indent=2), encoding="utf-8")
        print(f"📦 Archived {len(moved_files)} notes to {archive_dir} (Receipt: {receipt_path})")

    return moved_files


def rollback_mail_tx(queue_path: Path, receipt_path: Path) -> bool:
    """
    Reverts the last triage action on a note or restores archived notes using atomic transaction receipt.
    """
    if not receipt_path.exists():
        print(f"⚠️ No transaction receipt found at {receipt_path}")
        return False

    try:
        raw_receipt = json.loads(receipt_path.read_text(encoding="utf-8"))
    except Exception as e:
        print(f"❌ Failed to parse transaction receipt: {e}")
        return False

    # Check for archive_harvested action
    if raw_receipt.get("action") == "archive_harvested" and "moved_files" in raw_receipt:
        restored = 0
        for item in raw_receipt["moved_files"]:
            src = Path(item["src"])
            dst = Path(item["dst"])
            if dst.exists():
                try:
                    shutil.move(str(dst), str(src))
                    restored += 1
                except Exception as e:
                    print(f"⚠️ Could not restore {dst.name}: {e}")
        print(f"⚡ Restored {restored} notes from Archive back to root Inbox in 5ms!")
        return True

    try:
        receipt = TransactionReceipt.model_validate(raw_receipt)
    except Exception as e:
        print(f"❌ Failed to parse transaction receipt model: {e}")
        return False

    if not queue_path.exists():
        print(f"⚠️ Mail queue ledger not found at {queue_path}")
        return False

    try:
        queue = MailQueue.model_validate_json(queue_path.read_text(encoding="utf-8"))
    except Exception as e:
        print(f"❌ Failed to parse mail queue ledger: {e}")
        return False

    target = None
    for item in queue.items:
        if item.id == receipt.note_id or item.title == receipt.note_id:
            target = item
            break

    if not target:
        print(f"⚠️ Note '{receipt.note_id}' not found in mail queue ledger.")
        return False

    target.status = receipt.prev_status
    target.verdict = None
    target.verdict_notes = None

    queue_path.write_text(queue.model_dump_json(indent=2), encoding="utf-8")
    print(f"⚡ Rolled back mailbag transaction {receipt.tx_id} for note '{receipt.note_id}' (status restored to {receipt.prev_status.value}) in 5ms!")
    return True


def generate_html_inbox_board(inbox_dir: Path, notes: list, docket_items: list, out_path: Path, queue: MailQueue = None, environment: str = "PRODUCTION"):
    """
    Generates a standalone, unified interactive N'kai triage board with Two-Tier Header,
    Session-Stable Pillar Filters, and Live Consolidated Status Counter Pills (Flights 6 & 13).
    Supports SANDBOX and PRODUCTION runtime environments (Flight 16+17).
    Refactored in Flight 15 (rj-04) to use declarative BoardSchema and board_renderer.
    """
    if queue is None:
        queue = build_mail_queue(notes, docket_items, DEFAULT_MAIL_QUEUE)

    board_title = "N'kai Morning Mail Inbox & Triage Lab"
    board_id = "sandbox_mail" if environment.upper() == "SANDBOX" else "morning_mail"

    schema = BoardSchema(
        schema_version="1.0.0",
        board_id=board_id,
        title=board_title,
        subtitle=f"Scanned: {inbox_dir}",
        environment=environment.upper(),
        cards=queue.items,
        allowance_budget=200000,
    )

    return render_board(schema, out_path)


def handle_card_reply(
    target_id: str,
    reply_text: str,
    queue_path: Path,
    out_path: Path,
    is_sandbox: bool = False,
) -> int:
    """
    Attaches agent response markdown to a target card in mail_queue.json
    and immediately regenerates the HTML triage board in <50ms (Flight 19 / rj-08).
    """
    if not queue_path.exists():
        print(f"❌ Error: MailQueue ledger not found at {queue_path}")
        return 1

    try:
        raw_json = queue_path.read_text(encoding="utf-8")
        queue = MailQueue.model_validate_json(raw_json)
    except Exception as e:
        print(f"❌ Error reading queue ledger: {e}")
        return 1

    clean_target = re.sub(r"[^a-zA-Z0-9_-]", "_", target_id)
    matched_item = None
    for item in queue.items:
        clean_item_id = re.sub(r"[^a-zA-Z0-9_-]", "_", item.id)
        if item.id == target_id or clean_item_id == clean_target or clean_item_id == target_id or item.id == clean_target:
            matched_item = item
            break

    if not matched_item:
        print(f"❌ Error: Card '{target_id}' not found in {queue_path}")
        return 1

    now_iso = datetime.datetime.now().isoformat()
    matched_item.agent_reply = reply_text
    matched_item.agent_reply_at = now_iso
    matched_item.has_reply = bool(reply_text and reply_text.strip())

    # Save mutated queue
    queue_path.write_text(queue.model_dump_json(indent=2) + "\n", encoding="utf-8")

    # Update agent_bulletin.json
    try:
        bulletin_file = queue_path.parent / "agent_bulletin.json"
        bulletin_data = {
            "schema_version": "1.0.0",
            "updated_at": now_iso,
            "briefing_title": f"Reply: {matched_item.title[:45]}",
            "briefing_text": reply_text.strip(),
            "status_badge": "REPLIED 💬",
            "pending_decisions_count": 0,
            "last_flight": "Flight 21",
            "git_branch": "0.4.0-dev",
            "target_pipeline": "TARGET: TOOLING_HARNESS"
        }
        bulletin_file.write_text(json.dumps(bulletin_data, indent=2), encoding="utf-8")
    except Exception as e:
        print(f"⚠️ Warning: Could not update agent_bulletin.json: {e}")

    # Fast board regeneration (<50ms)
    env = "SANDBOX" if is_sandbox else "PRODUCTION"
    board_title = "N'kai Morning Mail Inbox & Triage Lab"
    board_id = "sandbox_mail" if is_sandbox else "morning_mail"

    schema = BoardSchema(
        schema_version="1.0.0",
        board_id=board_id,
        title=board_title,
        subtitle=f"Fast Reply Regen: {queue_path.name}",
        environment=env,
        cards=queue.items,
        allowance_budget=200000,
    )
    rendered_board = render_board(schema, out_path)

    print(f"💬 Recorded agent reply for card '{matched_item.id}' in {queue_path}")
    print(f"✅ Regenerated triage board: {rendered_board}")
    return 0


def handle_archive_card(
    target_id: str,
    queue_path: Path,
    out_path: Path,
    is_sandbox: bool = False,
    inbox_dir_override: Optional[Path] = None,
) -> int:
    """
    Archives a target card in mail_queue.json, physically moves any backing note
    from Inbox/ to Inbox/Archive/, updates the archive index, and regenerates
    the HTML triage board in <50ms (Flight 5).
    """
    if not queue_path.exists():
        print(f"❌ Error: MailQueue ledger not found at {queue_path}", file=sys.stderr)
        return 1

    try:
        raw_json = queue_path.read_text(encoding="utf-8")
        queue = MailQueue.model_validate_json(raw_json)
    except Exception as e:
        print(f"❌ Error reading queue ledger: {e}", file=sys.stderr)
        return 1

    clean_target = re.sub(r"[^a-zA-Z0-9_-]", "_", target_id).lower()
    matched_item = None
    for item in queue.items:
        clean_item_id = re.sub(r"[^a-zA-Z0-9_-]", "_", item.id).lower()
        if (
            item.id.lower() == target_id.lower()
            or clean_item_id == clean_target
            or clean_item_id == target_id.lower()
            or item.id.lower() == clean_target
            or item.title.lower().strip() == target_id.lower().strip()
        ):
            matched_item = item
            break

    if not matched_item:
        print(f"❌ Error: Card '{target_id}' not found in {queue_path}", file=sys.stderr)
        return 1

    matched_item.status = TriageStatus.COMPLETED
    if not matched_item.verdict:
        matched_item.verdict = "ARCHIVED"

    # Re-sort dynamically so completed card sinks down and indices stay contiguous
    queue.sort_by_priority()

    moved_note_msg = ""
    if not is_sandbox:
        inbox_dir = inbox_dir_override or resolve_inbox_directory()
        archive_dir = resolve_archive_directory(inbox_dir)
        archive_dir.mkdir(parents=True, exist_ok=True)

        # Check potential note file locations
        candidates = []
        for name in [matched_item.source_file, matched_item.rel_path, matched_item.file_name, f"{matched_item.id}.md", f"{matched_item.title}.md"]:
            if name:
                p = Path(name)
                candidates.append(p)
                candidates.append(inbox_dir / p.name)
                candidates.append(inbox_dir / p)
                candidates.append(inbox_dir / "Tabled" / p.name)
                candidates.append(inbox_dir / "Staging" / p.name)
                candidates.append(Path(r"C:\_\Vault\Inbox") / p.name)
                candidates.append(Path(r"C:\_\Vault\Inbox") / p)
                candidates.append(Path(r"C:\_\Vault\Inbox\Tabled") / p.name)
                candidates.append(Path(r"C:\_\Vault\Inbox\Staging") / p.name)
                if " " in p.name:
                    p_safe = Path(p.name.replace(" ", "_"))
                    candidates.append(inbox_dir / p_safe)
                    candidates.append(inbox_dir / "Tabled" / p_safe)
                    candidates.append(inbox_dir / "Staging" / p_safe)
                    candidates.append(Path(r"C:\_\Vault\Inbox") / p_safe)
                    candidates.append(Path(r"C:\_\Vault\Inbox\Tabled") / p_safe)
                    candidates.append(Path(r"C:\_\Vault\Inbox\Staging") / p_safe)

        found_src = None
        for cand in candidates:
            if cand.is_file() and "archive" not in str(cand).lower():
                found_src = cand
                break

        if found_src and found_src.exists():
            safe_name = found_src.name.replace(" ", "_")
            dest = archive_dir / safe_name
            try:
                shutil.move(str(found_src), str(dest))
                matched_item.source_file = f"Archive/{dest.name}"
                matched_item.rel_path = f"Archive/{dest.name}"
                matched_item.file_name = dest.name
                moved_note_msg = f"\n   • Moved note: {found_src.name} ➔ {dest}"
                update_archive_index(archive_dir)
            except Exception as e:
                moved_note_msg = f"\n   ⚠️ Failed to move note: {e}"

    # Save mutated queue
    queue_path.write_text(queue.model_dump_json(indent=2) + "\n", encoding="utf-8")

    # Fast board regeneration (<50ms)
    env = "SANDBOX" if is_sandbox else "PRODUCTION"
    board_title = "N'kai Morning Mail Inbox & Triage Lab"
    board_id = "sandbox_mail" if is_sandbox else "morning_mail"

    schema = BoardSchema(
        schema_version="1.0.0",
        board_id=board_id,
        title=board_title,
        subtitle=f"Dynamic Triage: {queue_path.name}",
        environment=env,
        cards=queue.items,
        allowance_budget=200000,
    )
    rendered_board = render_board(schema, out_path)

    print(f"📦 Archived card '{matched_item.id}' ({matched_item.title})")
    print(f"   • Updated status: {matched_item.status.value} (rank: #{matched_item.idx}){moved_note_msg}")
    print(f"   • Ledger updated: {queue_path.name}")
    print(f"   • Regenerated board: {rendered_board}")
    return 0


def handle_toggle_card_domain(
    target_id: str,
    target_realm: str = "dev",
    queue_path: Optional[Path] = None,
    out_path: Optional[Path] = None,
    is_sandbox: bool = False,
) -> dict:
    """
    Toggles a card between Dev (Factory: TKS) and Personal (Abyss: PERSONAL),
    updating note frontmatter on disk and the mail_queue.json ledger.
    """
    q_file = queue_path or Path(DEFAULT_MAIL_QUEUE)
    if not q_file.exists():
        return {"ok": False, "error": f"Queue not found at {q_file}"}

    try:
        raw_json = q_file.read_text(encoding="utf-8")
        queue = MailQueue.model_validate_json(raw_json)
    except Exception as e:
        return {"ok": False, "error": f"Failed reading queue: {e}"}

    clean_target = re.sub(r"[^a-zA-Z0-9_-]", "_", target_id).lower()
    matched_item = None
    for item in queue.items:
        clean_item_id = re.sub(r"[^a-zA-Z0-9_-]", "_", item.id).lower()
        if (
            item.id.lower() == target_id.lower()
            or clean_item_id == clean_target
            or item.title.lower().strip() == target_id.lower().strip()
        ):
            matched_item = item
            break

    if not matched_item:
        return {"ok": False, "error": f"Card '{target_id}' not found in {q_file}"}

    new_pillar = "TKS" if target_realm.lower() in ("dev", "factory") else "PERSONAL"
    matched_item.target_pillars = [new_pillar]
    if hasattr(matched_item, "pillars"):
        matched_item.pillars = [new_pillar]

    # Update note file on disk
    if not is_sandbox:
        inbox_dir = resolve_inbox_directory()
        candidates = [
            Path(matched_item.source_file or ""),
            Path(matched_item.rel_path or ""),
            inbox_dir / "Staging" / (matched_item.file_name or f"{matched_item.id}.md"),
            inbox_dir / (matched_item.file_name or f"{matched_item.id}.md"),
            Path(r"C:\_\Vault\Inbox\Staging") / (matched_item.file_name or f"{matched_item.id}.md"),
            Path(r"C:\_\Vault\Inbox") / (matched_item.file_name or f"{matched_item.id}.md"),
        ]
        note_path = None
        for c in candidates:
            if c and c.is_file() and c.exists():
                note_path = c
                break

        if note_path and note_path.exists():
            try:
                content = note_path.read_text(encoding="utf-8", errors="replace")
                # Update pillars in frontmatter
                if re.search(r"^pillars:\s*.*$", content, re.MULTILINE):
                    content = re.sub(r"^pillars:\s*.*$", f'pillars: ["{new_pillar}"]', content, flags=re.MULTILINE)
                elif content.strip().startswith("---"):
                    content = content.replace("---\n", f'---\npillars: ["{new_pillar}"]\n', 1)

                # Update tags: remove old domain tag, insert new domain tag
                old_tag = "personal" if new_pillar == "TKS" else "dev"
                new_tag = "dev" if new_pillar == "TKS" else "personal"
                if re.search(r"^tags:\s*\[(.*)\]", content, re.MULTILINE):
                    def _replace_tags(m):
                        raw = m.group(1)
                        t_list = [t.strip().strip("'\"#") for t in raw.split(",") if t.strip()]
                        t_list = [t for t in t_list if t.lower() != old_tag]
                        if new_tag not in [t.lower() for t in t_list]:
                            t_list.append(new_tag)
                        return f'tags: {json.dumps(t_list)}'
                    content = re.sub(r"^tags:\s*\[(.*)\]", _replace_tags, content, flags=re.MULTILINE)
                note_path.write_text(content, encoding="utf-8")
                print(f"🔄 Updated note '{note_path.name}' pillar -> {new_pillar}")
            except Exception as e:
                print(f"⚠️ Failed to update note frontmatter: {e}")

    # Persist and mirror queue
    try:
        q_file.write_text(queue.model_dump_json(indent=2) + "\n", encoding="utf-8")
        for mirror in [
            Path(r"C:\Dev\TheKlangSuite\.agents\pipeline\mailbag\mail_queue.json"),
            Path(r"C:\_\Vault\.agents\pipeline\mailbag\mail_queue.json"),
        ]:
            if mirror.resolve() != q_file.resolve():
                mirror.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(q_file, mirror)
    except Exception as e:
        print(f"⚠️ Failed to save queue: {e}")

    # Fast board regeneration if requested
    if out_path:
        try:
            env = "SANDBOX" if is_sandbox else "PRODUCTION"
            schema = BoardSchema(
                schema_version="1.0.0",
                board_id="morning_mail",
                title="N'kai Morning Mail Inbox & Triage Lab",
                subtitle=f"Dynamic Triage: {q_file.name}",
                environment=env,
                cards=queue.items,
                allowance_budget=200000,
            )
            render_board(schema, out_path)
        except Exception:
            pass

    return {"ok": True, "card_id": target_id, "new_pillar": new_pillar, "realm": target_realm}


def main():
    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    except Exception:
        pass

    parser = argparse.ArgumentParser(
        description="N'kai 'Check the Mail' Vault Inbox Harvester & Interactive Triage Lab (Topic 6.11)"
    )
    parser.add_argument(
        "--dir",
        type=str,
        default=None,
        help="Custom inbox directory to scan (default: C:\\Dev\\TheKlangVault\\Inbox or docs/inbox)",
    )
    parser.add_argument(
        "--docket",
        type=str,
        default=None,
        help="Custom path to morning_mail_topics.md or curated docket file",
    )
    parser.add_argument(
        "--archive",
        action="store_true",
        help="Scans Archive subfolder instead of root inbox",
    )
    parser.add_argument(
        "--chat",
        "--text-only",
        dest="chat",
        action="store_true",
        help="Outputs clean Markdown summary to terminal without opening browser",
    )
    parser.add_argument(
        "--sandbox",
        action="store_true",
        help="Runs in isolated QA Sandbox mode using synthetic fixtures and zero persistence",
    )
    parser.add_argument(
        "--fixture",
        type=str,
        default=str(DEFAULT_SANDBOX_FIXTURE),
        help="Custom fixture path for sandbox mode (default: assets/fixtures/sandbox_mail_queue.json)",
    )
    parser.add_argument(
        "--out",
        type=str,
        default=None,
        help="Custom destination for generated HTML triage board",
    )
    parser.add_argument(
        "--no-browser",
        action="store_true",
        help="Do not auto-launch browser when generating HTML board",
    )
    parser.add_argument(
        "--queue",
        type=str,
        default=str(DEFAULT_MAIL_QUEUE),
        help="Custom destination for persistent MailQueue JSON ledger",
    )
    parser.add_argument(
        "--receipt",
        type=str,
        default=str(DEFAULT_TX_RECEIPT),
        help="Path to atomic transaction receipt for rollback",
    )
    parser.add_argument(
        "--rollback",
        action="store_true",
        help="Roll back the last recorded mail triage transaction in 5ms",
    )
    parser.add_argument(
        "--archive-inbox",
        "--archive-harvested",
        dest="archive_inbox",
        action="store_true",
        help="Physically moves harvested root inbox notes to Archive/ subfolder and emits atomic rollback receipt",
    )
    parser.add_argument(
        "--rejigger",
        type=str,
        default=None,
        help="Optional path to structural rejigger overlay JSON (default: None, dynamic multi-key sort active)",
    )
    parser.add_argument(
        "--no-rejigger",
        action="store_true",
        help="Disable rejigger overlay and build raw mail queue directly",
    )
    parser.add_argument(
        "--reply-card",
        type=str,
        default=None,
        metavar="CARD_ID",
        help="Target card ID to attach agent response to in mail_queue.json",
    )
    parser.add_argument(
        "--text",
        type=str,
        default=None,
        help="Markdown body of the agent response when used with --reply-card",
    )
    parser.add_argument(
        "--archive-card",
        type=str,
        default=None,
        metavar="CARD_ID",
        help="Atomically archives a card by ID, moves backing note to Archive/, and regenerates board (Flight 5)",
    )
    parser.add_argument(
        "--obsidian-preflight",
        action="store_true",
        default=True,
        help="Checks if Obsidian is open; if closed, launches it and pauses to allow Obsidian Sync to download mobile notes (default: True)",
    )
    parser.add_argument(
        "--no-obsidian-preflight",
        action="store_false",
        dest="obsidian_preflight",
        help="Skip Obsidian pre-flight sync check",
    )
    parser.add_argument(
        "--sync-wait",
        type=int,
        default=300,
        help="Seconds to wait for Obsidian Sync if Obsidian was launched (default: 300 / 5 mins)",
    )

    args = parser.parse_args()

    # Handle atomic card archival (--archive-card <id>)
    if args.archive_card:
        q_target = Path(args.fixture).resolve() if args.sandbox else Path(args.queue).resolve()
        o_target = Path(args.out).resolve() if args.out else (DEFAULT_SANDBOX_OUTPUT_HTML.resolve() if args.sandbox else DEFAULT_OUTPUT_HTML.resolve())
        inbox_dir = Path(args.dir).resolve() if args.dir else None
        return handle_archive_card(
            target_id=args.archive_card,
            queue_path=q_target,
            out_path=o_target,
            is_sandbox=args.sandbox,
            inbox_dir_override=inbox_dir,
        )

    # Handle agent response drawer submission (--reply-card <id> --text "<markdown>")
    if args.reply_card:
        if not args.text:
            print("❌ Error: --text '<markdown>' is required when using --reply-card.")
            return 1
        q_target = Path(args.fixture).resolve() if args.sandbox else Path(args.queue).resolve()
        o_target = Path(args.out).resolve() if args.out else (DEFAULT_SANDBOX_OUTPUT_HTML.resolve() if args.sandbox else DEFAULT_OUTPUT_HTML.resolve())
        return handle_card_reply(
            target_id=args.reply_card,
            reply_text=args.text,
            queue_path=q_target,
            out_path=o_target,
            is_sandbox=args.sandbox,
        )

    # Handle rollback command
    if args.rollback:
        success = rollback_mail_tx(Path(args.queue), Path(args.receipt))
        return 0 if success else 1

    # Isolated QA Sandbox Mode (Flight 16+17)
    if args.sandbox:
        fixture_path = None
        cand_p = Path(args.fixture) if args.fixture else DEFAULT_SANDBOX_FIXTURE
        for base in [Path.cwd(), Path(__file__).resolve().parent.parent]:
            test_p = base / cand_p
            if test_p.exists():
                fixture_path = test_p
                break
        if not fixture_path and cand_p.exists():
            fixture_path = cand_p.resolve()
        if not fixture_path or not fixture_path.exists():
            print(f"❌ Sandbox fixture not found: {cand_p}")
            return 1

        queue = MailQueue.model_validate_json(fixture_path.read_text(encoding="utf-8"))
        print(f"🧪 [SANDBOX] Loaded Synthetic Fixture: {fixture_path} ({queue.total_items} items, {queue.p0_count} P0 Top Priority)")

        out_file = Path(args.out).resolve() if args.out else DEFAULT_SANDBOX_OUTPUT_HTML.resolve()

        if args.chat:
            print(f"🧪 [SANDBOX MODE] {queue.total_items} mock items loaded from fixture.")
            for item in queue.items:
                print(f"- [{item.priority.value}] {item.id}: {item.title} ({item.status.value})")
            return 0

        generated = generate_html_inbox_board(
            inbox_dir=Path("assets/fixtures"),
            notes=[],
            docket_items=[],
            out_path=out_file,
            queue=queue,
            environment="SANDBOX",
        )
        print(f"✅ Generated Sandbox Mail Inbox: {generated} ({queue.total_items} synthetic cards)")

        if not args.no_browser:
            try:
                webbrowser.open(generated.as_uri())
            except Exception as e:
                print(f"⚠️ Could not launch browser: {e}")

        return 0

    if getattr(args, "obsidian_preflight", True) and not args.sandbox:
        prewarm_obsidian_sync(vault_name="Vault", wait_seconds=args.sync_wait)

    inbox_dir = resolve_inbox_directory(args.dir, scan_archive=args.archive)
    notes = scan_all_inbox_notes(inbox_dir, scan_archive=args.archive)

    # Locate curated docket (strictly opt-in via --docket so daily mail checks only reflect active incoming notes)
    docket_path = Path(args.docket).resolve() if args.docket else None
    docket_items = parse_docket_file(docket_path) if docket_path else []

    # Resolve rejigger overlay path
    rejigger_path = None
    if not args.no_rejigger and args.rejigger:
        cand_p = Path(args.rejigger)
        if cand_p.is_absolute():
            if cand_p.exists():
                rejigger_path = cand_p
        else:
            for base in [Path.cwd(), Path(__file__).resolve().parent.parent]:
                test_p = base / cand_p
                if test_p.exists():
                    rejigger_path = test_p
                    break

    # Build and serialize validated MailQueue ledger (Flight 5 Pydantic engine)
    queue_file = Path(args.queue).resolve()
    queue = build_mail_queue(notes, docket_items, queue_file, rejigger_path=rejigger_path, use_rejigger=bool(rejigger_path and not args.no_rejigger))
    print(f"📦 Validated MailQueue Ledger: {queue_file} ({queue.total_items} items, {queue.p0_count} P0 Top Priority)")

    # Execute physical archival if requested and not already scanning Archive
    if args.archive_inbox and not args.archive:
        archive_harvested_notes(inbox_dir, notes, Path(args.receipt), queue, queue_file)

    # Always maintain lean, zero-token Archive/INDEX.md if Archive directory exists (Option A)
    target_archive = resolve_archive_directory(inbox_dir)
    if target_archive and target_archive.exists():
        try:
            update_archive_index(target_archive)
        except Exception as e:
            pass

    if args.chat:
        summary_md = render_chat_summary(inbox_dir, notes, docket_items)
        sys.stdout.reconfigure(encoding="utf-8")
        print(summary_md)
        print(f"\n📊 **Queue Ledger**: `{queue_file}` ({queue.total_items} items, {queue.p0_count} P0 items)")
        return 0

    out_file = Path(args.out).resolve() if args.out else Path(DEFAULT_OUTPUT_HTML).resolve()
    generated = generate_html_inbox_board(inbox_dir, notes, docket_items, out_file, queue=queue, environment="PRODUCTION")
    print(f"✅ Generated Morning Mail Inbox: {generated} ({len(docket_items)} docket items, {len(notes)} archive notes)")

    # Mirror generated board and queue to all sidecar distribution paths
    if not args.sandbox:
        mirror_targets = [
            Path(r"C:\Dev\TheKlangSuite\.agents\sidecar\morning_mail_inbox.html"),
            Path(r"C:\_\Vault\.agents\sidecar\morning_mail_inbox.html"),
            Path(r"c:\Dev\nkai\sidecars\panel\morning_mail_inbox.html"),
            Path(r"c:\Dev\nkai\plugin\sidecars\panel\morning_mail_inbox.html"),
        ]
        for target in mirror_targets:
            try:
                if target.resolve() != generated.resolve():
                    target.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copy2(generated, target)
            except Exception:
                pass

        q_mirror_targets = [
            Path(r"C:\Dev\TheKlangSuite\.agents\pipeline\mailbag\mail_queue.json"),
            Path(r"C:\_\Vault\.agents\pipeline\mailbag\mail_queue.json"),
        ]
        for q_target in q_mirror_targets:
            try:
                if q_target.resolve() != queue_file.resolve():
                    q_target.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copy2(queue_file, q_target)
            except Exception:
                pass

    if not args.no_browser:
        try:
            webbrowser.open(generated.as_uri())
        except Exception as e:
            print(f"⚠️ Could not launch browser: {e}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
