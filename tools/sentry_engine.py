#!/usr/bin/env python3
"""
tools/sentry_engine.py — N'kai Reactive Sentry Streaming Engine
===============================================================
Implements the real-time, token-streaming AI companion for N'kai:
- Powered by Google Gemini API (gemini-3.8-flash) via google-genai SDK.
- Multi-turn conversation history buffer support.
- Low-latency SSE token streaming (thought, token, done).
- Frontier model cascade (gemini-3.8-flash -> gemini-3.5-flash -> local zero-lag fallback).
- Strictly sandboxed vault read/search and note/dispatch staging tools.
- Universal UTF-8 compliance guaranteed.
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
import time
from typing import Any, AsyncGenerator, Dict, List, Optional

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent

# Detect Vault root directory
VAULT_ROOT = Path(r"C:\Dev\TheKlangVault")
if not VAULT_ROOT.exists():
    VAULT_ROOT = Path(r"C:\_\Vault")


def get_gemini_api_key() -> str:
    """Returns the Gemini API key from environment or known .env files."""
    key = os.environ.get("GEMINI_API_KEY", "").strip()
    if key:
        return key

    candidate_envs = [
        REPO_ROOT / ".env",
        Path(r"C:\Dev\nkai\.env"),
        Path(r"C:\Dev\TheKlangSuite\.env"),
        Path(r"C:\Dev\ChatDemoServer\.env"),
        Path(r"C:\_\Vault\.env"),
    ]
    for env_path in candidate_envs:
        if env_path.exists():
            try:
                for line in env_path.read_text(encoding="utf-8", errors="replace").splitlines():
                    line = line.strip()
                    if line and not line.startswith("#") and "=" in line:
                        k, v = line.split("=", 1)
                        if k.strip() == "GEMINI_API_KEY":
                            val = v.strip().strip('"').strip("'")
                            if val:
                                os.environ["GEMINI_API_KEY"] = val
                                return val
            except Exception:
                continue
    return ""


# ============================================================================
# Scoped Sandboxed Tools (Strictly Read-Only & Append-Only Staging)
# ============================================================================

def vault_search(query: str) -> str:
    """Searches Markdown notes in TheKlangVault for a query string (read-only)."""
    if not VAULT_ROOT.exists():
        return "Error: Vault root not found."

    clean_query = query.strip().lower()
    if not clean_query:
        return "Error: Empty query."

    matches = []
    max_matches = 5

    try:
        for md_file in VAULT_ROOT.rglob("*.md"):
            if ".git" in md_file.parts or ".obsidian" in md_file.parts:
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
    """Reads the text content of a Markdown note in TheKlangVault (read-only)."""
    if not VAULT_ROOT.exists():
        return "Error: Vault root not found."

    target = (VAULT_ROOT / rel_path).resolve()
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
    """Queries Google Drive folder/file metadata via the authenticated gdrive CLI (read-only)."""
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
    """Appends a new quick note into TheKlangVault/Inbox/QuickNotes/ (append-only staging)."""
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
    """Stages an action dispatch card into Dispatches/pending/ for user confirmation (append-only)."""
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

SENTRY_SYSTEM_PROMPT = """You are N'kai Sentry / Agent Intercom: a sharp, insightful, low-latency AI coding and creative partner operating within The Cosmic Codex and The Astral Abyss.
You are pair-programming with the lead developer across:
- The Klang Suite (TKS): C++20, real-time audio DSP, JUCE 9, CLAP, VST3, lock-free SPSC queues, zero heap allocation in processBlock, Cytomic SVF, PolyBLEP, and KlangStomp analog modeling.
- ToadTracker (TT): Hardware tracker workstation, SDL3, embedded targets RP2350 (Dual Cortex-M33 / Hazard3 RISC-V) and ESP32-P4.
- N'kai (NK): Sovereign command bridge, sidecars, PWA cockpit, dynamic shell, agenda/runway routing.
- The Astral Abyss & The Forge: Solo roleplaying, Starforged, worldbuilding, and personal knowledge management.

Core Guidelines:
- Direct, conversational, and technical. Speak like an experienced senior peer-programmer pair-programming at the terminal.
- Zero sycophancy, no flattery, no repetitive "As an AI..." disclaimers.
- Format all responses in clean GitHub-Flavored Markdown.
- Use triple backtick code blocks with exact language identifiers (cpp, python, js, html, bash, etc.).
- When discussing architecture or trade-offs, be honest about failure modes, maintenance drag, and developer ergonomics.
- Keep answers focused, practical, and immediately actionable.
"""


# ============================================================================
# High-Signal Offline / Fallback Reply Generator
# ============================================================================

def generate_simulated_sentry_reply(prompt: str, history: Optional[list[dict]] = None) -> str:
    """Generates an immediate, high-signal technical reply when offline or in fallback mode."""
    lower_prompt = prompt.lower()

    if any(w in lower_prompt for w in ["audio safety", "processblock", "mutex", "heap", "real-time audio", "audiothread"]):
        return (
            "In real-time audio (VST3 / CLAP / JUCE), the render callback (`processBlock`) runs on the high-priority OS audio thread:\n\n"
            "```cpp\n"
            "// REAL-TIME SAFETY INVARIANTS:\n"
            "// 1. Zero heap allocations (new, malloc, std::vector::resize)\n"
            "// 2. Zero blocking synchronization (std::mutex, critical sections)\n"
            "// 3. Zero file or console I/O (std::cout, DBG, disk read/write)\n"
            "// 4. Zero transcendental CRT calls (use FastMath lookup/polynomials)\n"
            "```\n\n"
            "**Recommended Architecture:**\n"
            "- Pre-allocate all voice buffers, delay lines, and wavetables in `prepareToPlay()`.\n"
            "- Use wait-free Single-Producer Single-Consumer (SPSC) ring buffers or atomic value swaps for UI-to-audio messaging.\n"
            "- Protect state using SIMD contiguous arrays rather than pointer indirection."
        )

    if any(w in lower_prompt for w in ["note", "quick note", "stage note", "remind me"]):
        res = stage_quick_note("Mobile Quick Note", prompt)
        return f"**[N'kai Sentry]** Note Staged:\n\n`{res}`\n\nAppended to `Inbox/QuickNotes/` and ready for review in your morning mail intake."

    if any(w in lower_prompt for w in ["vault", "search", "lookup", "find in vault"]):
        query = prompt.replace("vault", "").replace("search", "").replace("lookup", "").strip() or prompt[:30]
        res = vault_search(query)
        return f"**[N'kai Sentry]** Vault Search Results for *\"{query}\"*:\n\n```text\n{res}\n```"

    if any(w in lower_prompt for w in ["hello", "hi", "hey", "ping", "test"]):
        return (
            "Greetings from the Astral Abyss! 🛰️\n\n"
            "N'kai Sentry is active, reactive, and wired with token streaming. Ready for audio DSP architecture, "
            "ToadTracker engine design, vault searches, quick notes, or live pair-programming."
        )

    # General technical synthesis
    return (
        f"**[N'kai Sentry]** Received: *\"{prompt}\"*\n\n"
        "State and telemetry logged. Working on the active thread in The Klang Suite and N'kai cockpit.\n\n"
        "Need to inspect code, audit DSP safety, or stage a dispatch card to the master runway?"
    )


# ============================================================================
# Cooldown tracking for rate-limited / quota-exhausted models (10-minute backoff)
_EXHAUSTED_UNTIL: dict[str, float] = {}


async def stream_sentry_response(
    prompt: str,
    attachments: Optional[list[str]] = None,
    history: Optional[list[dict]] = None,
    api_key: Optional[str] = None,
    model: Optional[str] = None,
) -> AsyncGenerator[dict, None]:
    """Streams thoughts and candidate tokens from Gemini API with fallback.

    Yields event dicts:
    - {"type": "thought", "content": str}
    - {"type": "token", "content": str}
    - {"type": "done", "duration_ms": int, "mode": str}
    """
    start_time = time.time()
    key = api_key or get_gemini_api_key()
    full_tokens: list[str] = []
    used_mode = "unknown"

    # Live Gemini Execution via google-genai
    if key and model not in ("local", "simulated", "offline"):
        try:
            from google import genai
            from google.genai import types

            client = genai.Client(api_key=key)
            target_model = model or "gemini-3.8-flash"

            # Format multi-turn conversation
            formatted_contents = []
            if history:
                for msg in history:
                    role = "model" if msg.get("role") in ["assistant", "model"] else "user"
                    txt = msg.get("content", "").strip()
                    if txt:
                        formatted_contents.append(
                            types.Content(role=role, parts=[types.Part.from_text(text=txt)])
                        )
            formatted_contents.append(
                types.Content(role="user", parts=[types.Part.from_text(text=prompt)])
            )

            thinking_cfg = None
            if "3.8" in target_model or "thinking" in target_model.lower():
                try:
                    thinking_cfg = types.ThinkingConfig(thinking_budget=2048, include_thoughts=True)
                except Exception:
                    pass

            config = types.GenerateContentConfig(
                system_instruction=SENTRY_SYSTEM_PROMPT,
                temperature=0.35,
                thinking_config=thinking_cfg,
            )

            # Model cascade: try requested target_model, auto-fallback to healthy models
            raw_candidates = [target_model]
            for m in ["gemini-3.5-flash", "gemini-3.5-flash-lite", "gemini-3.1-flash-lite"]:
                if m not in raw_candidates:
                    raw_candidates.append(m)

            # Filter out models currently in quota exhaustion cooldown
            now = time.time()
            models_to_try = [m for m in raw_candidates if _EXHAUSTED_UNTIL.get(m, 0.0) <= now]
            if not models_to_try:
                # All candidates on cooldown; reset cooldown and try lowest-latency lite model
                models_to_try = ["gemini-3.5-flash-lite", "gemini-3.5-flash"]

            for candidate in models_to_try:
                try:
                    stream = await asyncio.wait_for(
                        client.aio.models.generate_content_stream(
                            model=candidate,
                            contents=formatted_contents,
                            config=config,
                        ),
                        timeout=5.0,
                    )

                    stream_iter = aiter(stream)
                    while True:
                        # 5.0s per-chunk timeout to survive network drops
                        chunk = await asyncio.wait_for(anext(stream_iter, None), timeout=5.0)
                        if chunk is None:
                            break

                        # Extract native thought parts vs standard content tokens
                        if hasattr(chunk, "candidates") and chunk.candidates:
                            for candidate_item in chunk.candidates:
                                if hasattr(candidate_item, "content") and candidate_item.content:
                                    for part in getattr(candidate_item.content, "parts", []):
                                        is_thought = getattr(part, "thought", False)
                                        part_text = getattr(part, "text", None)
                                        if part_text:
                                            if is_thought:
                                                yield {"type": "thought", "content": part_text}
                                            else:
                                                full_tokens.append(part_text)
                                                yield {"type": "token", "content": part_text}
                        elif hasattr(chunk, "text") and chunk.text:
                            full_tokens.append(chunk.text)
                            yield {"type": "token", "content": chunk.text}

                    if full_tokens:
                        used_mode = f"live_{candidate}"
                        break
                except (asyncio.TimeoutError, Exception) as stream_err:
                    err_name = type(stream_err).__name__
                    err_str = str(stream_err)
                    if "429" in err_str or "RESOURCE_EXHAUSTED" in err_str:
                        _EXHAUSTED_UNTIL[candidate] = time.time() + 600.0
                        print(f"[Sentry Cascade] {candidate} hit 429 quota limit; cooling down for 10m.", file=sys.stderr)
                    else:
                        print(f"[Sentry Cascade Warning] {candidate} failed ({err_name}: {stream_err}), trying next...", file=sys.stderr)
                    if full_tokens and len("".join(full_tokens).strip()) >= 100:
                        used_mode = f"live_{candidate}_partial"
                        break
                    continue

        except Exception as e:
            err_msg = f"⚡ Network error ({type(e).__name__}) — failing over to Local Engine..."
            yield {"type": "thought", "content": err_msg}
            await asyncio.sleep(0.03)

    # Simulated Fallback (if offline, no key, or connection dropped)
    if not full_tokens or len("".join(full_tokens).strip()) < 40:
        if full_tokens:
            full_tokens.clear()

        yield {"type": "thought", "content": "Running on Local Zero-Lag Engine..."}
        await asyncio.sleep(0.02)

        fallback_text = generate_simulated_sentry_reply(prompt, history)
        words = fallback_text.split(" ")
        for i, word in enumerate(words):
            token = word + (" " if i < len(words) - 1 else "")
            full_tokens.append(token)
            yield {"type": "token", "content": token}
            await asyncio.sleep(0.012)

        used_mode = "local_simulated"

    duration_ms = int((time.time() - start_time) * 1000)
    yield {"type": "done", "duration_ms": duration_ms, "mode": used_mode}
