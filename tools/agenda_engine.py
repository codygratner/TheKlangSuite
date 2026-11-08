#!/usr/bin/env python3
"""
tools/agenda_engine.py — N'kai Conversational Flight Deck & Agenda Parser
========================================================================
Parses, updates, and routes items into C:\\_\\Vault\\Agenda.md.
Provides 100% standard library support for the N'kai Agenda Board and Mail-to-Agenda pipeline.
Hardened against escaped Markdown table pipes (\\|) inside Obsidian wikilinks and emoji badges.
"""

import json
import re
import sys
import time
from pathlib import Path
from typing import Dict, Any, List, Optional

# Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

VAULT_ROOT = Path(r"c:\_\Vault")
AGENDA_PATH = VAULT_ROOT / "AGENDA.md"
AGENDA_LEGACY = VAULT_ROOT / "Agenda.md"
REPO_ROOT = Path(__file__).resolve().parent.parent


def get_agenda_path() -> Path:
    if AGENDA_PATH.exists():
        return AGENDA_PATH
    if AGENDA_LEGACY.exists():
        return AGENDA_LEGACY
    fallback = Path(r"c:\Dev\TheKlangSuite\docs\Agenda.md")
    return fallback


def split_markdown_table_row(line: str) -> List[str]:
    """Splits a markdown table row by unescaped pipes | while respecting \\| inside wikilinks or text."""
    line = line.strip()
    if line.startswith("|"):
        line = line[1:]
    if line.endswith("|") and not line.endswith(r"\|"):
        line = line[:-1]
    raw_cols = re.split(r"(?<!\\)\|", line)
    return [c.replace(r"\|", "|").strip() for c in raw_cols]


def clean_display_title(text: str) -> str:
    """Extracts a clean, human-readable title from markdown and Obsidian wikilinks."""
    # [[path/to/note|Display Title]] -> Display Title
    text = re.sub(r"\[\[(?:[^|\]]*\|)?([^\]]+)\]\]", r"\1", text)
    # Strip markdown bold, italics, code markers
    text = text.replace("**", "").replace("*", "").replace("`", "").strip()
    return text


def parse_agenda(path: Optional[Path] = None, use_cache: bool = True) -> Dict[str, Any]:
    target = path or get_agenda_path()
    if not target.exists():
        return {
            "success": False,
            "error": f"Agenda file not found at {target}",
            "runway": [],
            "open_loops": [],
            "tabled_talks": [],
            "meta": {"path": str(target), "exists": False},
        }

    # Asymmetric st_mtime JSON Shadow Cache (<0.1ms fast-path)
    cache_file = target.parent / ".agents" / "agenda_cache.json"
    target_mtime = 0.0
    try:
        target_mtime = target.stat().st_mtime
    except Exception:
        pass

    if use_cache and cache_file.exists() and target_mtime > 0:
        try:
            cached_data = json.loads(cache_file.read_text(encoding="utf-8"))
            if abs(cached_data.get("meta", {}).get("modified_time", 0.0) - target_mtime) < 0.001:
                return cached_data
        except Exception:
            pass

    try:
        content = target.read_text(encoding="utf-8")
    except Exception as e:
        return {
            "success": False,
            "error": f"Failed to read {target}: {e}",
            "runway": [],
            "open_loops": [],
            "tabled_talks": [],
            "meta": {"path": str(target), "exists": True},
        }

    runway: List[Dict[str, Any]] = []
    open_loops: List[Dict[str, Any]] = []
    tabled_talks: List[Dict[str, Any]] = []

    # 1. Parse Runway Table
    runway_match = re.search(
        r"## 🎯 Today's Live Flight Runway.*?\n(\|.+?\n)\s*(?:---|##)",
        content,
        re.DOTALL,
    )
    if runway_match:
        table_text = runway_match.group(1).strip()
        lines = [line.strip() for line in table_text.splitlines() if line.strip().startswith("|")]
        if len(lines) >= 3:
            for line in lines[2:]:
                cols = split_markdown_table_row(line)
                if len(cols) >= 5:
                    runway.append({
                        "priority": cols[0].replace("`", "").strip(),
                        "track": clean_display_title(cols[1]),
                        "raw_track": cols[1].strip(),
                        "topic": clean_display_title(cols[2]),
                        "raw_topic": cols[2].strip(),
                        "status": clean_display_title(cols[3]),
                        "raw_status": cols[3].strip(),
                        "next_action": cols[4].strip(),
                    })

    # 2. Parse Open Loops (Numbered blocks)
    open_loops_match = re.search(
        r"## 🔄 Active Open Loops.*?\n(.*?)(?:---|## ⏸️)",
        content,
        re.DOTALL,
    )
    if open_loops_match:
        loops_text = open_loops_match.group(1).strip()
        blocks = re.split(r"\n(?=\d+\.\s+)", "\n" + loops_text)
        for block in blocks:
            block = block.strip()
            if not block or not re.match(r"^\d+\.", block):
                continue
            title_match = re.match(r"^(\d+)\.\s*(.*?)\s*\*\*(.+?)\*\*:\s*(.*)", block, re.DOTALL)
            if title_match:
                loop_id = int(title_match.group(1))
                badge = title_match.group(2).strip()
                loop_title = title_match.group(3).strip()
                body = title_match.group(4).strip()

                is_resolved = (
                    "✅" in badge
                    or "(RESOLVED)" in loop_title.upper()
                    or "[X]" in badge.upper()
                    or "RESOLVED" in body[:100].upper()
                )

                status = "RESOLVED" if is_resolved else "OPEN"
                status_m = re.search(r"-\s+\*Status:\*\s+(.+)", body)
                if status_m:
                    status = status_m.group(1).strip()
                    if "RESOLVED" in status.upper():
                        is_resolved = True

                dilemma = ""
                dilemma_m = re.search(r"-\s+\*(?:Core Dilemma|The Dilemma):\*\s+(.+)", body)
                if dilemma_m:
                    dilemma = dilemma_m.group(1).strip()

                summary = ""
                summary_m = re.search(r"-\s+\*(?:Current State|Summary):\*\s+(.+)", body)
                if summary_m:
                    summary = summary_m.group(1).strip()

                resolution = ""
                res_m = re.search(r"-\s+\*Resolution:\*\s+(.+)", body)
                if res_m:
                    resolution = res_m.group(1).strip()

                open_loops.append({
                    "id": loop_id,
                    "title": loop_title,
                    "badge": badge,
                    "status": status,
                    "is_resolved": is_resolved,
                    "dilemma": dilemma,
                    "summary": summary,
                    "resolution": resolution,
                    "raw_body": body,
                })

    # 3. Parse Tabled Talks Table
    tabled_match = re.search(
        r"## ⏸️ Tabled Talks & The Parking Lot.*?\n(\|.+?\n)\s*(?:---|##)",
        content,
        re.DOTALL,
    )
    if tabled_match:
        table_text = tabled_match.group(1).strip()
        lines = [line.strip() for line in table_text.splitlines() if line.strip().startswith("|")]
        if len(lines) >= 3:
            for line in lines[2:]:
                cols = split_markdown_table_row(line)
                if len(cols) >= 4:
                    tabled_talks.append({
                        "topic": clean_display_title(cols[0]),
                        "raw_topic": cols[0].strip(),
                        "artifact": clean_display_title(cols[1]),
                        "raw_artifact": cols[1].strip(),
                        "unlock_trigger": clean_display_title(cols[2]),
                        "raw_unlock_trigger": cols[2].strip(),
                        "why_parked": cols[3].strip(),
                    })

    st = target.stat()
    result = {
        "success": True,
        "runway": runway,
        "open_loops": open_loops,
        "tabled_talks": tabled_talks,
        "meta": {
            "path": str(target),
            "modified_time": st.st_mtime,
            "modified_iso": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime(st.st_mtime)),
            "runway_count": len(runway),
            "open_loops_count": len([l for l in open_loops if not l.get("is_resolved")]),
            "resolved_loops_count": len([l for l in open_loops if l.get("is_resolved")]),
            "total_loops_count": len(open_loops),
            "tabled_count": len(tabled_talks),
        },
    }

    if use_cache:
        try:
            cache_file.parent.mkdir(parents=True, exist_ok=True)
            cache_file.write_text(json.dumps(result, indent=2, ensure_ascii=False), encoding="utf-8")
        except Exception:
            pass

    return result


def route_card_to_agenda(
    title: str,
    summary: str,
    target_section: str = "open_loops",
    unlock_trigger: Optional[str] = None,
    priority: str = "P2",
    track: str = "📬 Mail",
    card_id: Optional[str] = None,
) -> Dict[str, Any]:
    """Appends an incoming card to the specified section of Agenda.md and archives the mail card."""
    target = get_agenda_path()
    if not target.exists():
        return {"success": False, "error": f"Agenda file not found at {target}"}

    content = target.read_text(encoding="utf-8")
    today_str = time.strftime("%Y-%m-%d")

    clean_title = title.replace("\n", " ").strip()
    clean_summary = summary.replace("\n", " ").strip()

    if target_section == "open_loops":
        # Find highest loop number
        loop_numbers = [int(m) for m in re.findall(r"\n(\d+)\.\s*", content)]
        next_num = (max(loop_numbers) + 1) if loop_numbers else 1

        new_entry = (
            f"\n{next_num}. **{clean_title}**:\n"
            f"   - *Status:* Queued from Morning Mail ({today_str}).\n"
            f"   - *Summary:* {clean_summary}\n"
        )
        if unlock_trigger:
            new_entry += f"   - *Unlock Trigger:* **{unlock_trigger.strip()}**\n"

        # Insert right after the last open loop or before Tabled Talks
        tabled_pos = content.find("## ⏸️ Tabled Talks & The Parking Lot")
        if tabled_pos != -1:
            sep_pos = content.rfind("---", 0, tabled_pos)
            if sep_pos != -1:
                content = content[:sep_pos] + new_entry.rstrip() + "\n\n" + content[sep_pos:]
            else:
                content = content[:tabled_pos] + new_entry + "\n---\n\n" + content[tabled_pos:]
        else:
            content += new_entry

    elif target_section == "tabled_talks":
        trigger = unlock_trigger.strip() if unlock_trigger else "Manual 30-Day Review Gate"
        new_row = f"| 🔒 **{clean_title}** | [[from_mailbag]] | **{trigger}** | {clean_summary} |\n"
        marker = "## ⏸️ Tabled Talks & The Parking Lot"
        marker_pos = content.find(marker)
        if marker_pos != -1:
            sep_pos = content.find("---", marker_pos)
            if sep_pos != -1:
                content = content[:sep_pos] + new_row + "\n" + content[sep_pos:]
            else:
                content += "\n" + new_row
        else:
            content += f"\n## ⏸️ Tabled Talks & The Parking Lot\n| Topic | Primary Artifact | Unlock Trigger | Why It's Parked |\n| :--- | :--- | :--- | :--- |\n{new_row}"

    elif target_section == "runway":
        p_val = priority.strip().upper().replace("`", "") if priority else "P2"
        new_row = f"| `{p_val}` | {track} | **{clean_title}** | ⏳ **READY** | {clean_summary} |\n"
        runway_marker = "## 🎯 Today's Live Flight Runway"
        marker_pos = content.find(runway_marker)
        if marker_pos != -1:
            sep_pos = content.find("---", marker_pos)
            if sep_pos != -1:
                content = content[:sep_pos] + new_row + "\n" + content[sep_pos:]
            else:
                content += "\n" + new_row
        else:
            content = f"## 🎯 Today's Live Flight Runway (Active Discussion Queue)\n| Priority | Track | Focus Topic | Status | Immediate Next Action |\n| :---: | :---: | :--- | :---: | :--- |\n{new_row}\n---\n" + content

    target.write_text(content, encoding="utf-8")

    # Invalidate agenda cache
    cache_file = target.parent / ".agents" / "agenda_cache.json"
    if cache_file.exists():
        try:
            cache_file.unlink()
        except Exception:
            pass

    archived_from_mail = False
    if card_id:
        queue_path = REPO_ROOT / ".agents" / "pipeline" / "mailbag" / "mail_queue.json"
        if queue_path.exists():
            try:
                import json
                q_data = json.loads(queue_path.read_text(encoding="utf-8"))
                cards = q_data.get("items") or q_data.get("cards", [])
                updated_cards = []
                for c in cards:
                    if str(c.get("id")) == str(card_id):
                        c["status"] = "routed_to_agenda"
                        c["routed_to"] = target_section
                        c["routed_at"] = time.time()
                        archived_from_mail = True
                    else:
                        updated_cards.append(c)
                if "items" in q_data:
                    q_data["items"] = updated_cards
                    q_data["total_items"] = len(updated_cards)
                else:
                    q_data["cards"] = updated_cards
                    q_data["active_count"] = len(updated_cards)
                queue_path.write_text(json.dumps(q_data, indent=2), encoding="utf-8")
            except Exception:
                pass

    return {
        "success": True,
        "target_section": target_section,
        "title": clean_title,
        "archived_from_mail": archived_from_mail,
        "timestamp": time.time(),
    }


if __name__ == "__main__":
    import json
    data = parse_agenda()
    print(f"Agenda parsed: {len(data['runway'])} runway items, {len(data['open_loops'])} open loops ({data['meta']['open_loops_count']} active, {data['meta']['resolved_loops_count']} resolved), {len(data['tabled_talks'])} tabled talks.")
    print(json.dumps(data["meta"], indent=2))
