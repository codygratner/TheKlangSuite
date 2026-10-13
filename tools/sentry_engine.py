#!/usr/bin/env python3
"""
tools/sentry_engine.py — AGY SDK Stateless Sentry Engine
=========================================================
Implements the Stateless Sentry worker for N'kai: an on-demand, single-turn
agent powered by the Google Antigravity SDK (`google-antigravity`).
Enforces hard budget limits, zero shell execution, read-only search tools,
and append-only note/dispatch staging.
"""

from __future__ import annotations

import asyncio
from datetime import datetime
import json
import os
from pathlib import Path
import re
import subprocess
import sys
from typing import AsyncGenerator, Optional

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent

# Detect Vault root directory
VAULT_ROOT = Path(r"C:\Dev\TheKlangVault")
if not VAULT_ROOT.exists():
    VAULT_ROOT = Path(r"C:\_\Vault")


# ============================================================================
# Scoped Sandboxed Tools (Strictly Read-Only & Append-Only Staging)
# ============================================================================

def vault_search(query: str) -> str:
    """Searches Markdown notes in TheKlangVault for a query string (read-only).

    Args:
        query: Search term or keyword.
    """
    if not VAULT_ROOT.exists():
        return "Error: Vault root not found."

    clean_query = query.strip().lower()
    if not clean_query:
        return "Error: Empty query."

    matches = []
    max_matches = 5

    try:
        for md_file in VAULT_ROOT.rglob("*.md"):
            if ".git" in md_file.parts:
                continue
            try:
                content = md_file.read_text(encoding="utf-8", errors="replace")
                lines = content.splitlines()
                for i, line in enumerate(lines):
                    if clean_query in line.lower():
                        rel_path = md_file.relative_to(VAULT_ROOT).as_posix()
                        matches.append(f"[{rel_path}:{i+1}] {line.strip()[:140]}")
                        break
                if len(matches) >= max_matches:
                    break
            except Exception:
                continue
    except Exception as e:
        return f"Search error: {e}"

    if not matches:
        return f"No matches found for '{query}' in vault."

    return "Vault search results:\n" + "\n".join(matches)


def vault_read(rel_path: str) -> str:
    """Reads the text content of a Markdown note in TheKlangVault (read-only).

    Args:
        rel_path: Relative path to note inside vault (e.g. 'Inbox/TODO.md').
    """
    if not VAULT_ROOT.exists():
        return "Error: Vault root not found."

    target = (VAULT_ROOT / rel_path).resolve()
    # Path traversal check
    try:
        target.relative_to(VAULT_ROOT.resolve())
    except ValueError:
        return "Error: Access denied. Cannot read files outside vault root."

    if not target.exists() or not target.is_file():
        return f"Error: Note '{rel_path}' not found."

    try:
        content = target.read_text(encoding="utf-8", errors="replace")
        if len(content) > 6000:
            content = content[:6000] + "\n\n... [Truncated for brevity]"
        return content
    except Exception as e:
        return f"Read error: {e}"


def gdrive_search(query: str) -> str:
    """Queries Google Drive folder/file metadata via the authenticated gdrive CLI (read-only).

    Args:
        query: Optional folder ID or search term.
    """
    gdrive_cmd = Path(r"C:\Users\codyg\.gemini\config\plugins\gdrive\skills\gdrive\scripts\gdrive.cmd")
    if not gdrive_cmd.exists():
        return "Error: gdrive CLI not found."

    try:
        res = subprocess.run(
            [str(gdrive_cmd), "readonly", "ls", "--max", "5"],
            capture_output=True,
            text=True,
            timeout=8,
            check=False,
        )
        out = res.stdout.strip()
        if not out and res.stderr:
            return f"gdrive error: {res.stderr.strip()}"
        return out if out else "Google Drive folder is empty or not accessible."
    except Exception as e:
        return f"gdrive CLI error: {e}"


def stage_quick_note(title: str, body: str) -> str:
    """Appends a new quick note into TheKlangVault/Inbox/QuickNotes/ (append-only staging).

    Args:
        title: Short descriptive note title.
        body: Markdown note content.
    """
    inbox_dir = VAULT_ROOT / "Inbox" / "QuickNotes"
    inbox_dir.mkdir(parents=True, exist_ok=True)

    safe_title = re.sub(r"[^\w\s-]", "", title).strip().replace(" ", "_")
    if not safe_title:
        safe_title = "quick_note"

    timestamp = datetime.now().strftime("%Y-%m-%d_%H%M%S")
    filename = f"{timestamp}_{safe_title}.md"
    target = inbox_dir / filename

    content = f"""# {title}
Date: {datetime.now().strftime("%Y-%m-%d %H:%M")}
Source: N'kai Sentry Mobile Companion
Tags: #quick-note #sentry

{body}
"""
    try:
        target.write_text(content, encoding="utf-8")
        return f"Successfully staged quick note to: Inbox/QuickNotes/{filename}"
    except Exception as e:
        return f"Error staging note: {e}"


def stage_dispatch(title: str, payload_json: str) -> str:
    """Stages an action dispatch card into Dispatches/pending/ for user confirmation (append-only).

    Args:
        title: Short title of the dispatch task.
        payload_json: JSON string representing the dispatch payload.
    """
    dispatch_dir = VAULT_ROOT / "Dispatches" / "pending"
    dispatch_dir.mkdir(parents=True, exist_ok=True)

    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    clean_title = re.sub(r"[^\w\s-]", "", title).strip().replace(" ", "_")
    filename = f"dispatch_{timestamp}_{clean_title}.json"
    target = dispatch_dir / filename

    try:
        parsed_payload = json.loads(payload_json) if payload_json else {}
    except Exception:
        parsed_payload = {"raw": payload_json}

    dispatch_data = {
        "title": title,
        "created_at": datetime.now().isoformat(),
        "source": "nkai_sentry",
        "status": "pending_human_confirmation",
        "payload": parsed_payload,
    }

    try:
        target.write_text(json.dumps(dispatch_data, indent=2, ensure_ascii=False), encoding="utf-8")
        return f"Successfully staged dispatch to: Dispatches/pending/{filename}"
    except Exception as e:
        return f"Error staging dispatch: {e}"


SENTRY_TOOLS = [
    vault_search,
    vault_read,
    gdrive_search,
    stage_quick_note,
    stage_dispatch,
]

SENTRY_SYSTEM_PROMPT = """You are the N'kai Satellite Sentry: a sharp, concise, low-latency AI companion relay.
You assist the developer via their mobile device while they are away from their desk.
- Never flatter, compliment, or use sycophantic preambles.
- Keep responses direct, high-signal, and formatted in clean GitHub Markdown.
- You have strictly read-only search tools for the vault and Google Drive, and append-only staging tools.
- You NEVER execute shell commands or delete files. If complex execution is requested, stage a dispatch card.
"""


# ============================================================================
# Streaming Agent Executor
# ============================================================================

async def stream_sentry_response(
    prompt: str,
    attachments: Optional[list[str]] = None,
    api_key: Optional[str] = None,
) -> AsyncGenerator[dict, None]:
    """Streams thoughts, tool status, and candidate text tokens from the AGY SDK.

    Yields event dicts:
    - {"type": "thought", "content": str}
    - {"type": "tool_start", "tool": str, "args": dict}
    - {"type": "token", "content": str}
    - {"type": "done", "stats": dict}
    """
    key = api_key or os.environ.get("GEMINI_API_KEY")

    # If no SDK or no API key, execute deterministic mock Sentry response
    if not key:
        yield {"type": "thought", "content": "Evaluating prompt under Sentry budget limits..."}
        await asyncio.sleep(0.05)

        # Check for tool triggers in prompt
        lower = prompt.lower()
        if "vault" in lower or "search" in lower:
            yield {"type": "tool_start", "tool": "vault_search", "args": {"query": prompt[:30]}}
            res = vault_search(prompt[:30])
            yield {"type": "token", "content": f"### 🔍 Vault Search Results\n\n{res}\n\n"}
        elif "note" in lower or "quick" in lower:
            yield {"type": "tool_start", "tool": "stage_quick_note", "args": {"title": "Mobile Note"}}
            res = stage_quick_note("Mobile Note", prompt)
            yield {"type": "token", "content": f"### 📝 Note Staged\n\n{res}\n\n"}
        else:
            yield {"type": "thought", "content": "Generating direct high-signal response..."}
            reply_text = f"**[N'kai Sentry]** Received: '{prompt}'. Ready to assist with vault searches, quick notes, and dispatches."
            for word in reply_text.split(" "):
                yield {"type": "token", "content": word + " "}
                await asyncio.sleep(0.02)

        yield {"type": "done", "stats": {"turns": 1, "tokens": 42}}
        return

    # Live AGY SDK Execution
    try:
        from google.antigravity import Agent, LocalAgentConfig, types

        config = LocalAgentConfig(
            system_instructions=SENTRY_SYSTEM_PROMPT,
            tools=SENTRY_TOOLS,
            api_key=key,
            budget_config=types.BudgetConfig(
                max_model_calls=2,
                max_tool_calls=5,
                max_output_tokens=4000,
            ),
        )

        async with Agent(config) as agent:
            response = await agent.chat(prompt)

            # Stream thinking thoughts if available
            try:
                async for thought in response.thoughts:
                    yield {"type": "thought", "content": thought}
            except Exception:
                pass

            # Stream output text tokens
            async for token in response:
                yield {"type": "token", "content": token}

            yield {"type": "done", "stats": {"turns": 1, "stop_reason": str(getattr(response, "stop_reason", "complete"))}}

    except Exception as e:
        yield {"type": "error", "error": f"Sentry execution error: {e}"}
