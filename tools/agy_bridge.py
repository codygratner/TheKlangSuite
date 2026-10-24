#!/usr/bin/env python3
"""
tools/agy_bridge.py — Sovereign Direct Antigravity Language Server Bridge
========================================================================
Connects N'kai directly to local language_server.exe via ConnectRPC over HTTPS.
Features:
  1. Dual-Vector Kernel Socket + Root HTML CSRF Discovery (Zero-CLI sniffing).
  2. Self-Healing Reconnect Loop with Exponential Jitter Retry on 401 / WinError 10061.
  3. Pinned Chat Discovery from app_storage.json and conversation_summaries.db.
  4. Sub-15ms Local Transcript Turn Extraction from transcript.jsonl.
  5. ConnectRPC SendUserCascadeMessage Execution.
  6. Pydantic v2 Schema Hardening (tools.agy_models).
"""

import os
import sys
import time
import ssl
import json
import re
import urllib.request
import urllib.error
import sqlite3
from pathlib import Path
from typing import Dict, Any, List, Tuple, Optional

# Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    try:
        sys.stdout.reconfigure(encoding="utf-8")
    except Exception:
        pass
if hasattr(sys.stderr, "reconfigure"):
    try:
        sys.stderr.reconfigure(encoding="utf-8")
    except Exception:
        pass

# Add Vault and TKS roots to path
for r in [Path(r"c:\_\Vault"), Path(r"C:\Dev\TheKlangSuite"), Path(r"C:\Dev\Nkai")]:
    if str(r) not in sys.path:
        sys.path.insert(0, str(r))

from tools.agy_models import (
    AgyConversation,
    SendCascadeRequest,
    TextOrScopeItem,
    TrajectoryTurn,
    QuotaBucket,
)

CACHE_PATH = Path(".agents") / "cache" / "antigravity_rpc_cache.json"
APPDATA_DIR = Path(os.environ.get("APPDATA", os.path.expanduser("~\\AppData\\Roaming"))) / "Antigravity"
GEMINI_DIR = Path(os.path.expanduser("~/.gemini/antigravity"))
BRAIN_DIR = GEMINI_DIR / "brain"
SUMMARIES_DB = GEMINI_DIR / "conversation_summaries.db"
APP_STORAGE_JSON = APPDATA_DIR / "app_storage.json"
LOG_PATH = APPDATA_DIR / "logs" / "language_server.log"

_SSL_CTX = ssl.create_default_context()
_SSL_CTX.check_hostname = False
_SSL_CTX.verify_mode = ssl.CERT_NONE


def _get_active_presence_conv_id() -> Optional[str]:
    """Finds the active foreground desktop conversation from presence locks."""
    presence_dir = GEMINI_DIR / "presence"
    if not presence_dir.exists():
        return None
    locks = sorted(presence_dir.glob("*.lock"), key=lambda f: f.stat().st_mtime, reverse=True)
    if locks:
        return locks[0].stem
    return None


def _discover_port_via_kernel() -> Optional[int]:
    """Inspects Win32 OS TCP table via psutil to find language_server listening HTTPS port."""
    try:
        import psutil
        for p in psutil.process_iter(["pid", "name"]):
            name = (p.info["name"] or "").lower()
            if "language_server" in name:
                try:
                    for conn in p.net_connections(kind="inet"):
                        if conn.status == "LISTEN" and conn.laddr.ip in ("127.0.0.1", "0.0.0.0"):
                            port = conn.laddr.port
                            # Test if port responds to HTTPS ConnectRPC
                            if _probe_https_port(port):
                                return port
                except Exception:
                    pass
    except Exception:
        pass
    return None


def _probe_https_port(port: int) -> bool:
    """Verifies that a port is serving the Antigravity HTTPS server."""
    try:
        url = f"https://127.0.0.1:{port}/"
        req = urllib.request.Request(url)
        with urllib.request.urlopen(req, timeout=0.35, context=_SSL_CTX) as res:
            text = res.read(1024).decode("utf-8", errors="ignore")
            return "antigravity" in text.lower() or "csrfToken" in text
    except Exception:
        return False


def _discover_port_via_log() -> Optional[int]:
    """Secondary fallback: parses language_server.log for HTTPS port."""
    if LOG_PATH.exists():
        try:
            text = LOG_PATH.read_text(encoding="utf-8", errors="ignore")
            matches = re.findall(r"Language server listening on random port at (\d+) for HTTPS", text)
            if matches:
                return int(matches[-1])
        except Exception:
            pass
    return None


def _extract_csrf_from_html(port: int) -> Optional[str]:
    """Extracts CSRF token directly from the unauthenticated root HTML page."""
    try:
        url = f"https://127.0.0.1:{port}/"
        req = urllib.request.Request(url)
        with urllib.request.urlopen(req, timeout=1.5, context=_SSL_CTX) as res:
            html = res.read(4096).decode("utf-8", errors="ignore")
            m = re.search(r'"csrfToken":\s*"([a-f0-9\-]+)"', html)
            if m:
                return m.group(1)
    except Exception:
        pass
    return None


def _extract_csrf_from_cmdline() -> Optional[str]:
    """Secondary fallback: extracts --csrf_token from Windows process command line."""
    try:
        import subprocess
        script = 'Get-CimInstance Win32_Process -Filter "name = \'language_server.exe\' and CommandLine like \'%--standalone%\'" | Select-Object -ExpandProperty CommandLine'
        res = subprocess.run(["powershell", "-NoProfile", "-Command", script], capture_output=True, text=True, encoding="utf-8")
        m = re.search(r"--csrf_token[=\s]+([a-f0-9\-]+)", res.stdout)
        if m:
            return m.group(1)
    except Exception:
        pass
    return None


def discover_session(force_refresh: bool = False) -> Tuple[int, str]:
    """Discovers active Antigravity session port and CSRF token using dual-vector discovery."""
    if not force_refresh and CACHE_PATH.exists():
        try:
            cached = json.loads(CACHE_PATH.read_text(encoding="utf-8"))
            port = int(cached["port"])
            csrf = str(cached["csrf_token"])
            # Quick probe
            if _probe_https_port(port):
                return port, csrf
        except Exception:
            pass

    # 1. Discover Port (Kernel socket -> Log file)
    port = _discover_port_via_kernel()
    if not port:
        port = _discover_port_via_log()
    if not port:
        raise RuntimeError("Antigravity language_server.exe is not currently running or listening on HTTPS.")

    # 2. Discover CSRF Token (HTML scrape -> CLI flag)
    csrf = _extract_csrf_from_html(port)
    if not csrf:
        csrf = _extract_csrf_from_cmdline()
    if not csrf:
        raise RuntimeError("Could not extract Antigravity CSRF token from HTML or process command line.")

    # Save Cache
    CACHE_PATH.parent.mkdir(parents=True, exist_ok=True)
    CACHE_PATH.write_text(json.dumps({"port": port, "csrf_token": csrf, "timestamp": time.time()}), encoding="utf-8")
    return port, csrf


def execute_rpc(method: str, payload: Dict[str, Any], port: Optional[int] = None, csrf: Optional[str] = None) -> Dict[str, Any]:
    """Executes a ConnectRPC method against language_server.exe with auto-healing reconnect."""
    if not port or not csrf:
        port, csrf = discover_session()

    url = f"https://127.0.0.1:{port}/exa.language_server_pb.LanguageServerService/{method}"
    headers = {
        "x-codeium-csrf-token": csrf,
        "Content-Type": "application/json",
        "Connect-Protocol-Version": "1",
    }
    body = json.dumps(payload).encode("utf-8")

    try:
        req = urllib.request.Request(url, headers=headers, data=body)
        with urllib.request.urlopen(req, timeout=3.0, context=_SSL_CTX) as res:
            return json.loads(res.read().decode("utf-8"))
    except (urllib.error.HTTPError, urllib.error.URLError, ConnectionRefusedError, TimeoutError) as e:
        # Auto-heal: wipe cache, retry with exponential backoff
        CACHE_PATH.unlink(missing_ok=True)
        for delay in (0.1, 0.3, 0.8):
            time.sleep(delay)
            try:
                new_port, new_csrf = discover_session(force_refresh=True)
                new_url = f"https://127.0.0.1:{new_port}/exa.language_server_pb.LanguageServerService/{method}"
                headers["x-codeium-csrf-token"] = new_csrf
                req = urllib.request.Request(new_url, headers=headers, data=body)
                with urllib.request.urlopen(req, timeout=3.0, context=_SSL_CTX) as res:
                    return json.loads(res.read().decode("utf-8"))
            except Exception:
                continue
        raise RuntimeError(f"ConnectRPC {method} failed after auto-heal retries: {e}")


ASTRAL_ABYSS_CONV_ID = "6d5f68bf-c7b4-4129-bf75-1f5dc5e1fa41"


def get_conversations() -> Dict[str, Any]:
    """
    Returns active conversation, pinned conversations, and recent conversations
    normalized via Pydantic v2, guaranteeing The Astral Abyss is always #1 and default.
    """
    current_id = _get_active_presence_conv_id() or ASTRAL_ABYSS_CONV_ID

    # 1. Read Pinned conversations from app_storage.json
    pinned_ids: List[str] = []
    if APP_STORAGE_JSON.exists():
        try:
            d = json.loads(APP_STORAGE_JSON.read_text(encoding="utf-8"))
            pinned_ids = d.get("pinned_conversations_order", [])
            if isinstance(pinned_ids, str):
                pinned_ids = json.loads(pinned_ids)
        except Exception:
            pass

    # 2. Read summaries from SQLite with schema reflection
    results: List[AgyConversation] = []
    seen_ids = set()

    if SUMMARIES_DB.exists():
        try:
            conn = sqlite3.connect(f"file:{SUMMARIES_DB}?mode=ro", uri=True)
            cur = conn.cursor()
            # Schema reflection
            cols = {row[1] for row in cur.execute("PRAGMA table_info(conversation_summaries)").fetchall()}
            title_col = "title" if "title" in cols else "''"
            status_col = "status" if "status" in cols else "''"
            mod_col = "last_modified_time" if "last_modified_time" in cols else "''"
            steps_col = "step_count" if "step_count" in cols else "0"
            proj_col = "project_id" if "project_id" in cols else "''"

            query = f"""
                SELECT conversation_id, {title_col}, {status_col}, {mod_col}, {steps_col}, {proj_col}
                FROM conversation_summaries
                WHERE parent_conversation_id = ''
                ORDER BY {mod_col} DESC
                LIMIT 30
            """
            rows = cur.execute(query).fetchall()
            for r in rows:
                cid = r[0]
                is_abyss = (cid == ASTRAL_ABYSS_CONV_ID or "astral abyss" in (r[1] or "").lower())
                c = AgyConversation(
                    id=cid,
                    title=r[1] or "Untitled Chat",
                    status=r[2] or "IDLE",
                    last_active=str(r[3]),
                    step_count=r[4] or 0,
                    project_id=str(r[5] or ""),
                    is_current=(cid == current_id),
                    is_pinned=(cid in pinned_ids),
                    is_astral_abyss=is_abyss,
                )
                results.append(c)
                seen_ids.add(cid)

            # Also fetch Astral Abyss and any pinned or current chats that were beyond the LIMIT
            missing_ids = [pid for pid in (pinned_ids + [current_id, ASTRAL_ABYSS_CONV_ID]) if pid and pid not in seen_ids]
            if missing_ids:
                placeholders = ",".join("?" * len(missing_ids))
                missing_query = f"""
                    SELECT conversation_id, {title_col}, {status_col}, {mod_col}, {steps_col}, {proj_col}
                    FROM conversation_summaries
                    WHERE conversation_id IN ({placeholders})
                """
                for r in cur.execute(missing_query, missing_ids).fetchall():
                    cid = r[0]
                    is_abyss = (cid == ASTRAL_ABYSS_CONV_ID or "astral abyss" in (r[1] or "").lower())
                    c = AgyConversation(
                        id=cid,
                        title=r[1] or "Untitled Chat",
                        status=r[2] or "IDLE",
                        last_active=str(r[3]),
                        step_count=r[4] or 0,
                        project_id=str(r[5] or ""),
                        is_current=(cid == current_id),
                        is_pinned=(cid in pinned_ids),
                        is_astral_abyss=is_abyss,
                    )
                    results.append(c)
                    seen_ids.add(cid)

            conn.close()
        except Exception:
            pass

    # Ensure Astral Abyss exists even if not in DB
    if ASTRAL_ABYSS_CONV_ID not in seen_ids:
        results.append(AgyConversation(
            id=ASTRAL_ABYSS_CONV_ID,
            title="🌌 The Astral Abyss",
            is_pinned=True,
            is_astral_abyss=True,
            is_current=(ASTRAL_ABYSS_CONV_ID == current_id)
        ))
        seen_ids.add(ASTRAL_ABYSS_CONV_ID)

    # Ensure pinned chats exist even if older than limit
    for pid in pinned_ids:
        if pid not in seen_ids:
            results.append(AgyConversation(id=pid, title="Pinned Chat", is_pinned=True, is_current=(pid == current_id)))
            seen_ids.add(pid)

    # Ensure active chat is included
    if current_id not in seen_ids:
        results.insert(0, AgyConversation(id=current_id, title="Active Chat", is_current=True))

    # Sort: Astral Abyss ALWAYS #1 (top), then Pinned, then Current, then Last Active
    sorted_convs = sorted(results, key=lambda x: (not x.is_astral_abyss, not x.is_pinned, not x.is_current))

    return {
        "current_id": current_id,
        "default_id": ASTRAL_ABYSS_CONV_ID,
        "pinned_ids": pinned_ids,
        "conversations": [c.model_dump() for c in sorted_convs],
    }


def get_conversation_history(conv_id: str, limit: int = 50) -> List[Dict[str, Any]]:
    """Reads transcript.jsonl directly from disk, parsing user/assistant turns in sub-15ms."""
    transcript_path = BRAIN_DIR / conv_id / ".system_generated" / "logs" / "transcript.jsonl"
    if not transcript_path.exists():
        return []

    turns: List[TrajectoryTurn] = []
    try:
        lines = transcript_path.read_text(encoding="utf-8", errors="ignore").splitlines()
        for i, line in enumerate(lines):
            try:
                record = json.loads(line)
                typ = record.get("type")
                src = record.get("source")
                content = record.get("content") or ""
                thinking = record.get("thinking") or ""

                if typ == "USER_INPUT" and content.strip():
                    turns.append(TrajectoryTurn(
                        turn_index=record.get("step_index", i),
                        sender="user",
                        text=content.strip(),
                        created_at=record.get("created_at", ""),
                    ))
                elif typ == "PLANNER_RESPONSE":
                    if content.strip() or thinking.strip():
                        turns.append(TrajectoryTurn(
                            turn_index=record.get("step_index", i),
                            sender="assistant",
                            text=content.strip(),
                            thinking_text=thinking.strip(),
                            status=record.get("status", "done"),
                            created_at=record.get("created_at", ""),
                        ))
            except Exception:
                continue
    except Exception:
        pass

    return [t.model_dump() for t in turns[-limit:]]


def send_user_message(conv_id: str, prompt: str, model_id: Optional[str] = None) -> Dict[str, Any]:
    """Dispatches a user prompt to language_server.exe for the given conversation."""
    payload: Dict[str, Any] = {
        "cascadeId": conv_id,
        "messageText": prompt,
        "items": [
            {
                "text": prompt,
                "chunk": {
                    "case": "text",
                    "value": prompt
                }
            }
        ],
    }
    if model_id:
        payload["cascadeConfig"] = {
            "plannerConfig": {
                "planModel": model_id,
                "requestedModel": {
                    "choice": {
                        "case": "model",
                        "value": model_id
                    }
                },
                "supportsLatexRendering": True
            }
        }
    return execute_rpc("SendUserCascadeMessage", payload)


if __name__ == "__main__":
    t0 = time.perf_counter()
    port, csrf = discover_session()
    print(f"✅ Antigravity Session Discovered: Port {port} | CSRF {csrf[:8]}... (took {(time.perf_counter()-t0)*1000:.1f}ms)")
    convs = get_conversations()
    print(f"📋 Conversations ({len(convs['conversations'])} total, active: {convs['current_id']}):")
    for c in convs["conversations"][:5]:
        pin = "📌 " if c["is_pinned"] else "   "
        cur = "🟢 " if c["is_current"] else "   "
        print(f"  {cur}{pin}{c['title']} ({c['id'][:12]}...) [{c['status']}]")
