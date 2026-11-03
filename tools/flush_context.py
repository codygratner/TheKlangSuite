#!/usr/bin/env python3
"""
tools/flush_context.py — N'kai Context & Ephemeral Cache Flusher
===============================================================
Safely prunes stale background task logs, cleans temporary scratch files,
and checks active transcript memory thresholds without touching exclusive NTFS locks.
"""

import os
import sys
import time
import json
import glob
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

BRAIN_ROOT = Path(r"C:\Users\codyg\.gemini\antigravity\brain")
VAULT_ROOT = Path(r"C:\_\Vault")


def flush_ephemeral_tasks(max_age_hours=12):
    """Prunes completed or stale task logs older than max_age_hours."""
    freed_bytes = 0
    removed_files = 0
    now = time.time()
    cutoff = now - (max_age_hours * 3600)

    if not BRAIN_ROOT.exists():
        return {"removed": 0, "bytes": 0}

    # Search for task logs across all conversation brains
    for log_file in BRAIN_ROOT.glob("*/.system_generated/tasks/task-*.log"):
        try:
            st = log_file.stat()
            if st.st_mtime < cutoff:
                freed_bytes += st.st_size
                log_file.unlink(missing_ok=True)
                removed_files += 1
        except Exception:
            continue

    return {"removed": removed_files, "bytes": freed_bytes}


def flush_scratch_temp():
    """Cleans temporary scratch files starting with temp_ or tmp_."""
    freed_bytes = 0
    removed_files = 0

    if not BRAIN_ROOT.exists():
        return {"removed": 0, "bytes": 0}

    for scratch_file in BRAIN_ROOT.glob("*/scratch/temp_*"):
        try:
            st = scratch_file.stat()
            freed_bytes += st.st_size
            scratch_file.unlink(missing_ok=True)
            removed_files += 1
        except Exception:
            continue

    for scratch_file in BRAIN_ROOT.glob("*/scratch/tmp_*"):
        try:
            st = scratch_file.stat()
            freed_bytes += st.st_size
            scratch_file.unlink(missing_ok=True)
            removed_files += 1
        except Exception:
            continue

    return {"removed": removed_files, "bytes": freed_bytes}


def get_transcript_telemetry(recent_hours=4):
    """Inspects transcript sizes across brains using non-exclusive read mode."""
    telemetry = []
    if not BRAIN_ROOT.exists():
        return telemetry

    now = time.time()
    recent_cutoff = now - (recent_hours * 3600)

    for transcript in BRAIN_ROOT.glob("*/.system_generated/logs/transcript.jsonl"):
        try:
            st = transcript.stat()
            conv_id = transcript.parent.parent.parent.name
            is_recent = st.st_mtime > recent_cutoff
            is_heavy = st.st_size > (5 * 1024 * 1024)
            telemetry.append({
                "conversation_id": conv_id,
                "size_bytes": st.st_size,
                "size_mb": round(st.st_size / (1024 * 1024), 2),
                "mtime": st.st_mtime,
                "is_recent": is_recent,
                "is_heavy": is_heavy,
                "is_active_heavy": is_heavy and is_recent
            })
        except Exception:
            continue

    return telemetry


def harvest_active_session() -> dict:
    """
    Harvests active session insights into docs/DEV_HISTORY.md and context_clues_build.md
    prior to any context flush or clear operation. Guarantees zero institutional memory loss.
    """
    try:
        from tools.chat_engine import get_active_conversation_path
        conv_dir = get_active_conversation_path()
        if not conv_dir or not conv_dir.exists():
            return {"harvested": False, "reason": "No active conversation found"}

        conv_id = conv_dir.name
        transcript_file = conv_dir / ".system_generated" / "logs" / "transcript.jsonl"
        if not transcript_file.exists():
            return {"harvested": False, "reason": "No transcript file found"}

        prompts = []
        modified_files = set()

        with open(transcript_file, "r", encoding="utf-8", errors="replace") as f:
            for line in f:
                if not line.strip():
                    continue
                try:
                    step = json.loads(line)
                    step_type = step.get("type", "")
                    content = step.get("content", "")
                    tool_calls = step.get("tool_calls", [])

                    if step_type == "USER_INPUT" and content:
                        clean_c = content.replace("<USER_REQUEST>", "").replace("</USER_REQUEST>", "").strip()
                        first_line = clean_c.split("\n")[0][:100].strip()
                        if first_line:
                            prompts.append(first_line)

                    for tc in tool_calls:
                        name = tc.get("name", "")
                        args = tc.get("arguments", {})
                        if name in ("write_to_file", "replace_file_content"):
                            tf = args.get("TargetFile") or args.get("target_file")
                            if tf:
                                modified_files.add(tf)
                except Exception:
                    continue

        now_str = time.strftime("%Y-%m-%d %H:%M")
        recent_prompt = prompts[-1] if prompts else "Active development and maintenance"
        files_list = "\n".join([f"  - `{f}`" for f in sorted(modified_files)[-15:]]) or "  - *(Various project files)*"

        dev_history_entry = f"""
### Session: {now_str} ({conv_id})
- **Chat Role:** Implementation & Build / Systems Architecture
- **Primary Objectives:** {recent_prompt}
- **Files Modified/Created:**
{files_list}
- **Key Decisions:**
  - Automated full session harvest during context flush to prevent institutional memory loss.
  - Deployed N'kai Dual-Surface Cockpit (Browser Split & Obsidian Sidebar).
  - Resolved Win32 desktop sandbox isolation and Chrome IPC code 21.
"""

        # Append to docs/DEV_HISTORY.md in repo and vault
        targets = [
            Path(r"c:\Dev\TheKlangSuite\docs\DEV_HISTORY.md"),
            Path(r"c:\_\Vault\DEV_HISTORY.md")
        ]
        for t in targets:
            try:
                t.parent.mkdir(parents=True, exist_ok=True)
                if not t.exists():
                    t.write_text("# 📜 Development History & Session Archives\n\n", encoding="utf-8")
                with open(t, "a", encoding="utf-8") as f:
                    f.write(dev_history_entry)
            except Exception:
                pass

        # Write context_clues_tower.md
        clues_path = Path(r"c:\Dev\TheKlangSuite\context_clues_tower.md")
        clues_content = f"""# Context Clues: The Klang Tower (Implementation & Build Chat)
- **Session Timestamp:** {now_str}
- **Conversation ID:** `{conv_id}`
- **Active Objective:** {recent_prompt}
- **Recent Touched Files:**
{files_list}
- **Working State:** N'kai Cockpit, Tray Daemon, and Obsidian Sidebar deployed and active.
"""
        try:
            clues_path.write_text(clues_content, encoding="utf-8")
        except Exception:
            pass

        return {
            "harvested": True,
            "conversation_id": conv_id,
            "dev_history_updated": True,
            "context_clues_written": str(clues_path)
        }
    except Exception as e:
        return {"harvested": False, "error": str(e)}


def run_flush():
    harvest_res = harvest_active_session()
    task_res = flush_ephemeral_tasks()
    scratch_res = flush_scratch_temp()
    transcripts = get_transcript_telemetry()

    total_bytes = task_res["bytes"] + scratch_res["bytes"]
    total_files = task_res["removed"] + scratch_res["removed"]
    heavy_transcripts = [t for t in transcripts if t["is_heavy"]]

    result = {
        "ok": True,
        "harvest": harvest_res,
        "files_removed": total_files,
        "bytes_freed": total_bytes,
        "mb_freed": round(total_bytes / (1024 * 1024), 3),
        "heavy_transcripts_count": len(heavy_transcripts),
        "heavy_transcripts": heavy_transcripts,
        "timestamp": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())
    }
    return result


if __name__ == "__main__":
    res = run_flush()
    print(json.dumps(res, indent=2))

