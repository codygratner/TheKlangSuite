#!/usr/bin/env python3
"""
N'kai Zero-Dependency CLI Server (nkai_serve.py)
Part of N'kai Universal AI Harness Portability (Topic 6.13) &
N'kai v0.3.0 Mobile Companion & Touch Ergonomics (Section 14).

Provides instant HTTP serving for N'kai sidecars, research boards, and interactive
dialog labs across terminal-first harnesses (Claude Code CLI, Aider), mobile devices,
and IDEs. Guarantees 100% standard library compliance, automatic port recovery,
RFC MIME mapping, terminal ASCII/Unicode QR pairing, and SVG pairing endpoints.
"""

import argparse
import base64
import email
import email.policy
import http.server
from datetime import datetime
import ipaddress
import json
import mimetypes
import os
import re
import secrets
import shutil
import socket
import socketserver
import struct
import subprocess
import sys
import threading
import time
from typing import Optional
import traceback
import webbrowser
import zlib
from pathlib import Path
from urllib.parse import unquote, parse_qs, urlparse

REPO_ROOT = Path(__file__).resolve().parent.parent
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

# Configure UTF-8 streams for Windows console compatibility (Universal UTF-8 Invariant)
if hasattr(sys.stdout, 'reconfigure'):
    sys.stdout.reconfigure(encoding='utf-8')
if hasattr(sys.stderr, 'reconfigure'):
    sys.stderr.reconfigure(encoding='utf-8')

def set_thread_execution_state(enable: bool = True):
    """Prevents Windows from entering Modern Standby / Sleep while server is active."""
    if sys.platform == "win32":
        try:
            import ctypes
            ES_CONTINUOUS = 0x80000000
            ES_SYSTEM_REQUIRED = 0x00000001
            flags = ES_CONTINUOUS | (ES_SYSTEM_REQUIRED if enable else 0)
            ctypes.windll.kernel32.SetThreadExecutionState(flags)
        except Exception:
            pass

# Import zero-dependency QR code generator
try:
    from tools.qr_code import QRCode, generate_qr_terminal, generate_qr_svg
except ImportError:
    try:
        from qr_code import QRCode, generate_qr_terminal, generate_qr_svg
    except ImportError:
        _tools_dir = Path(__file__).resolve().parent
        if str(_tools_dir) not in sys.path:
            sys.path.insert(0, str(_tools_dir))
        from qr_code import QRCode, generate_qr_terminal, generate_qr_svg

# Sentry Bearer Token Auth & Streaming Engine
try:
    from tools.sentry_auth import get_or_create_sentry_token, verify_sentry_token
    from tools.sentry_engine import stream_sentry_response, get_gemini_api_key
except ImportError:
    try:
        from sentry_auth import get_or_create_sentry_token, verify_sentry_token
        from sentry_engine import stream_sentry_response, get_gemini_api_key
    except ImportError:
        _tools_dir = Path(__file__).resolve().parent
        if str(_tools_dir) not in sys.path:
            sys.path.insert(0, str(_tools_dir))
        from sentry_auth import get_or_create_sentry_token, verify_sentry_token
        from sentry_engine import stream_sentry_response, get_gemini_api_key

# Register canonical MIME types
CUSTOM_MIME_TYPES = {
    ".html": "text/html; charset=utf-8",
    ".js": "application/javascript",
    ".css": "text/css",
    ".json": "application/json",
    ".wav": "audio/wav",
    ".svg": "image/svg+xml",
    ".png": "image/png",
    ".jpg": "image/jpeg",
    ".jpeg": "image/jpeg",
    ".woff2": "font/woff2",
}

for ext, mime in CUSTOM_MIME_TYPES.items():
    mimetypes.add_type(mime, ext)

PRESENCE_FILE = REPO_ROOT / ".agents" / "pipeline" / "active_presence.json"
PRESENCE_STATE = {
    "is_active": False,
    "last_heartbeat": 0.0,
    "client": None,
    "board": None
}


def record_presence(client: str, board: str, focused: bool):
    global PRESENCE_STATE
    now = time.time()
    PRESENCE_STATE = {
        "is_active": bool(focused),
        "last_heartbeat": now,
        "client": client,
        "board": board,
        "updated_at": datetime.now().isoformat()
    }
    try:
        PRESENCE_FILE.parent.mkdir(parents=True, exist_ok=True)
        PRESENCE_FILE.write_text(json.dumps(PRESENCE_STATE, indent=2), encoding="utf-8")
    except Exception:
        pass


def get_presence_state() -> dict:
    global PRESENCE_STATE
    now = time.time()
    # Heartbeat expires after 12 seconds
    is_live = PRESENCE_STATE.get("is_active", False) and (now - PRESENCE_STATE.get("last_heartbeat", 0.0) < 12.0)
    return {
        "is_active": is_live,
        "client": PRESENCE_STATE.get("client") if is_live else None,
        "board": PRESENCE_STATE.get("board") if is_live else None,
        "last_seen_seconds_ago": round(now - PRESENCE_STATE.get("last_heartbeat", 0.0), 1) if PRESENCE_STATE.get("last_heartbeat") else None
    }


def get_local_ip() -> str:
    """Detects local LAN / Wi-Fi IP address, avoiding VPN tunnel interfaces."""
    try:
        host_ips = socket.gethostbyname_ex(socket.gethostname())[2]
        # Prioritize standard home/office subnets: 192.168.x.x, 172.16-31.x.x
        for ip in host_ips:
            if ip.startswith("192.168."):
                return ip
        for ip in host_ips:
            if ip.startswith("172.") and len(ip.split(".")) == 4:
                try:
                    second_octet = int(ip.split(".")[1])
                    if 16 <= second_octet <= 31:
                        return ip
                except ValueError:
                    pass
        # Non-VPN 10.x.x.x (exclude NordLynx 10.5.x, OpenVPN 10.100.x, Tailscale 100.x)
        for ip in host_ips:
            if ip.startswith("10.") and not (ip.startswith("10.5.") or ip.startswith("10.100.")):
                return ip
    except Exception:
        pass

    try:
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
            s.connect(("8.8.8.8", 80))
            cand = s.getsockname()[0]
            if not (cand.startswith("10.5.") or cand.startswith("10.100.") or cand.startswith("100.")):
                return cand
    except Exception:
        pass

    return "192.168.1.3"


def get_tailscale_ip() -> Optional[str]:
    """Detects Tailscale IPv4 address via CLI or interface scan."""
    tailscale_exe = Path(r"C:\Program Files\Tailscale\tailscale.exe")
    if tailscale_exe.exists():
        try:
            res = subprocess.run(
                [str(tailscale_exe), "ip", "-4"],
                capture_output=True,
                text=True,
                timeout=2,
                check=False,
            )
            ip = res.stdout.strip()
            if ip and ip.startswith("100."):
                return ip
        except Exception:
            pass

    try:
        host_ips = socket.gethostbyname_ex(socket.gethostname())[2]
        for ip in host_ips:
            if ip.startswith("100."):
                return ip
    except Exception:
        pass

    return None


def generate_touch_icon(mode: str = "lan") -> bytes:
    """Generates a dynamic 180x180 PNG touch icon for iOS Safari / Android homescreen bookmarks.

    Provides instant visual contrast:
    - 'lan' / 'home': Warm Amber & Sky Blue palette with 'H' (Home / Studio) glyph.
    - 'remote' / 'tailscale': Neon Emerald & Cyan palette with 'R' (Remote / Mesh) glyph.
    - 'codex' / 'forge': Royal Amethyst & Cosmic Violet palette with 'C' (Cosmic Codex) glyph.
    Zero third-party dependencies; 100% standard library struct & zlib.
    """
    w, h = 180, 180
    bg = (9, 13, 19, 255)       # #090d13 deep basalt
    card = (24, 32, 44, 255)    # #18202c panel

    mode_lower = (mode or "").lower()
    if "remote" in mode_lower or "tailscale" in mode_lower:
        accent = (52, 211, 153, 255)   # #34d399 emerald
        badge_bg = (16, 185, 129, 255) # emerald-600
        text_char = "R"
    elif "codex" in mode_lower or "forge" in mode_lower:
        accent = (168, 85, 247, 255)   # #a855f7 purple
        badge_bg = (126, 34, 206, 255) # purple-700
        text_char = "C"
    else:  # lan / home
        accent = (251, 191, 36, 255)   # #fbbf24 amber
        badge_bg = (56, 189, 248, 255) # sky-400
        text_char = "H"

    pixels = []
    # 5x7 bitmap font for H, R, C
    font = {
        "H": [0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001],
        "R": [0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001],
        "C": [0b01111, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b01111],
    }
    glyph = font.get(text_char, font["H"])

    for y in range(h):
        row = [0]  # filter byte (None)
        for x in range(w):
            border_dist = min(x, w - 1 - x, y, h - 1 - y)
            is_border = (border_dist < 6)

            # Center badge: scale 12x (5*12=60w, 7*12=84h)
            gx = (x - 60) // 12
            gy = (y - 48) // 12
            in_glyph = False
            if 0 <= gx < 5 and 0 <= gy < 7:
                if (glyph[gy] >> (4 - gx)) & 1:
                    in_glyph = True

            if in_glyph:
                row.extend(accent)
            elif is_border:
                row.extend(accent)
            elif border_dist < 10:
                row.extend(card)
            else:
                row.extend(bg)
        pixels.append(bytes(row))

    raw = b"".join(pixels)

    def chunk(tag, data):
        return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)

    return (
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 6, 0, 0, 0))
        + chunk(b"IDAT", zlib.compress(raw, 9))
        + chunk(b"IEND", b"")
    )


class ScratchpadSentinel:
    """Zero-token filesystem watcher tracking SCRATCHPAD.md metrics and status.

    Implements a 3-retry safe exponential backoff to handle Windows WinError 32
    during background Obsidian Sync or file lock events.
    """

    PRIMARY_VAULT_PATH = Path(r"C:\_\Vault\SCRATCHPAD.md")
    LEGACY_VAULT_PATH = Path(r"C:\_\Vault\Inbox\SCRATCHPAD.md")
    FALLBACK_PATH = REPO_ROOT / "docs" / "inbox" / "SCRATCHPAD.md"

    @classmethod
    def get_path(cls) -> Path:
        if cls.PRIMARY_VAULT_PATH.exists():
            return cls.PRIMARY_VAULT_PATH
        if cls.LEGACY_VAULT_PATH.exists():
            return cls.LEGACY_VAULT_PATH
        if cls.PRIMARY_VAULT_PATH.parent.exists():
            return cls.PRIMARY_VAULT_PATH
        return cls.FALLBACK_PATH

    @staticmethod
    def safe_stat(path: Path, retries: int = 3, delay: float = 0.05):
        for i in range(retries):
            try:
                return path.stat()
            except (PermissionError, OSError) as e:
                if i < retries - 1:
                    time.sleep(delay * (2 ** i))
                else:
                    raise e

    @classmethod
    def inspect(cls, custom_path: Optional[Path] = None) -> dict:
        sp_path = custom_path if custom_path is not None else cls.get_path()
        if not sp_path.exists():
            return {
                "exists": False,
                "path": str(sp_path),
                "lines": 0,
                "bytes": 0,
                "mtime": 0.0,
                "urgency": "clean",
                "has_unread": False,
            }

        try:
            st = cls.safe_stat(sp_path)
            size_bytes = st.st_size
            mtime = st.st_mtime

            # Safely count lines
            line_count = 0
            for i in range(3):
                try:
                    with open(sp_path, "r", encoding="utf-8", errors="replace") as f:
                        lines_list = f.readlines()
                    line_count = len(lines_list)
                    break
                except (PermissionError, OSError):
                    if i < 2:
                        time.sleep(0.05 * (2 ** i))

            if line_count < 25:
                urgency = "clean"
            elif line_count <= 80:
                urgency = "growing"
            else:
                urgency = "urgent"

            return {
                "exists": True,
                "path": str(sp_path),
                "lines": line_count,
                "bytes": size_bytes,
                "mtime": mtime,
                "urgency": urgency,
                "has_unread": (line_count > 0),
            }
        except Exception as e:
            return {
                "exists": False,
                "error": str(e),
                "path": str(sp_path),
                "lines": 0,
                "bytes": 0,
                "mtime": 0.0,
                "urgency": "clean",
                "has_unread": False,
            }


def dispatch_prompt_to_harness(prompt: str, client_ip: str = "127.0.0.1", source: str = "chat_board") -> Tuple[bool, Optional[str]]:
    """Relays prompt to action_history.jsonl, agent_bulletin.json, Data Bus, and Antigravity desktop."""
    if not prompt or not prompt.strip():
        return False, None
    prompt = prompt.strip()
    now_iso = datetime.now().isoformat()

    # 1. Log to action_history.jsonl
    action_log = REPO_ROOT / ".agents" / "pipeline" / "mailbag" / "action_history.jsonl"
    action_log.parent.mkdir(parents=True, exist_ok=True)
    try:
        with open(action_log, "a", encoding="utf-8") as f:
            f.write(json.dumps({
                "timestamp": now_iso,
                "action": "PROMPT_DISPATCH",
                "prompt": prompt,
                "source": source,
                "client_ip": client_ip
            }) + "\n")
    except Exception:
        pass

    # 2. Auto-sync to Windows Clipboard for instant Ctrl+V into Antigravity desktop chat
    if sys.platform == "win32":
        try:
            import win32clipboard, win32con
            win32clipboard.OpenClipboard()
            win32clipboard.EmptyClipboard()
            win32clipboard.SetClipboardText(prompt, win32con.CF_UNICODETEXT)
            win32clipboard.CloseClipboard()
        except Exception:
            pass

    # 3. Update agent_bulletin.json
    try:
        bulletin_file = REPO_ROOT / ".agents" / "pipeline" / "mailbag" / "agent_bulletin.json"
        b_data = {
            "schema_version": "1.0.0",
            "updated_at": now_iso,
            "briefing_title": f"Mobile Prompt: {prompt[:40]}",
            "briefing_text": prompt,
            "status_badge": "DISPATCHED 🚀",
            "pending_decisions_count": 0,
            "last_flight": "Flight 22",
            "git_branch": "0.4.0-dev",
            "target_pipeline": "TARGET: TOOLING_HARNESS"
        }
        bulletin_file.write_text(json.dumps(b_data, indent=2), encoding="utf-8")
    except Exception:
        pass

    # 4. Autonomous Mobile Relay: Post dispatch packet to Inter-Project Data Bus for COSMOS
    dispatch_id = None
    try:
        from tools.bus import resolve_bus_dir, serialize_dispatch
        bus_dir = resolve_bus_dir()
        today_str = datetime.now().strftime("%Y-%m-%d")
        now_slug = datetime.now().strftime("%Y%m%d_%H%M%S")
        clean_title = re.sub(r"[^a-zA-Z0-9_\- ]", " ", prompt).strip()
        prompt_slug = re.sub(r"[^a-zA-Z0-9_\-]", "_", clean_title[:24].lower()).strip("_") or "prompt"
        dispatch_id = f"{today_str}_mobile_to_cosmos_{prompt_slug}_{now_slug[-6:]}"

        meta = {
            "id": dispatch_id,
            "date": today_str,
            "from": "MOBILE",
            "to": "COSMOS",
            "tags": ["mobile", "intercom", "prompt"],
            "priority": "P1",
            "status": "PENDING",
            "title": f"Mobile Intercom Prompt: {clean_title[:35]}",
        }
        body = (
            f"# 📱 Mobile Intercom Prompt\n\n"
            f"**Received At**: `{now_iso}`  \n"
            f"**Origin IP**: `{client_ip}`  \n"
            f"**Client Surface**: N'kai Intercom Companion  \n\n"
            f"### 💬 User Prompt:\n\n{prompt}\n"
        )
        dispatch_content = serialize_dispatch(meta, body)
        target_file = bus_dir / f"{dispatch_id}.md"
        target_file.write_text(dispatch_content, encoding="utf-8")
        try:
            print(f"🚀 [RELAY] Dispatched mobile prompt to Data Bus: {dispatch_id}")
        except Exception:
            pass
    except Exception as e:
        try:
            print(f"⚠️ [RELAY] Could not post mobile dispatch to Data Bus: {e}")
        except Exception:
            pass

    # 5. Direct Antigravity Injection via AGY CLI / Agent API (100% Silent Background Execution)
    injected_ok = False
    try:
        from tools.chat_engine import get_active_conversation_path
        conv_dir = get_active_conversation_path()
        if conv_dir and conv_dir.exists():
            active_conv_id = conv_dir.name
            server_exe = Path(os.path.expanduser(r"~\AppData\Local\Programs\antigravity\resources\bin\language_server.exe"))
            agentapi_bat = Path(os.path.expanduser("~/.gemini/antigravity/bin/agentapi.bat"))

            flags = 0
            if sys.platform == "win32":
                flags = getattr(subprocess, "CREATE_NO_WINDOW", 0x08000000)

            if server_exe.exists():
                cmd = [str(server_exe), "agentapi", "send-message", "--title=📱 Mobile Prompt", active_conv_id, prompt]
            elif agentapi_bat.exists():
                cmd = [str(agentapi_bat), "send-message", "--title=📱 Mobile Prompt", active_conv_id, prompt]
            else:
                cmd = None

            run_env = os.environ.copy()
            for env_file in [
                REPO_ROOT / ".agents" / "pipeline" / "antigravity_env.json",
                Path(r"C:\_\Vault\.agents\pipeline\antigravity_env.json"),
                Path(r"C:\Dev\nkai\.agents\pipeline\antigravity_env.json"),
                Path(r"C:\Dev\TheKlangSuite\.agents\pipeline\antigravity_env.json"),
            ]:
                if env_file.exists():
                    try:
                        env_data = json.loads(env_file.read_text(encoding="utf-8"))
                        for ek, ev in env_data.items():
                            if ev:
                                run_env[ek] = str(ev)
                        break
                    except Exception:
                        pass

            if cmd:
                res = subprocess.run(
                    cmd,
                    env=run_env,
                    capture_output=True,
                    text=True,
                    timeout=5,
                    check=False,
                    creationflags=flags
                )
                if res.returncode == 0:
                    injected_ok = True
                    try:
                        print(f"⚡ [AGY BRIDGE] Silently injected prompt into Antigravity: {active_conv_id}")
                    except Exception:
                        pass
                else:
                    err_file = REPO_ROOT / ".agents" / "pipeline" / "logs" / "agentapi_debug.txt"
                    err_file.parent.mkdir(parents=True, exist_ok=True)
                    err_file.write_text(f"RC: {res.returncode}\nCMD: {cmd}\nOUT: {res.stdout}\nERR: {res.stderr}", encoding="utf-8")
        else:
            err_file = REPO_ROOT / ".agents" / "pipeline" / "logs" / "agentapi_debug.txt"
            err_file.parent.mkdir(parents=True, exist_ok=True)
            err_file.write_text(f"conv_dir not found: {conv_dir}", encoding="utf-8")
    except Exception as e:
        err_file = REPO_ROOT / ".agents" / "pipeline" / "logs" / "agentapi_debug.txt"
        err_file.parent.mkdir(parents=True, exist_ok=True)
        err_file.write_text(f"Exception in dispatch: {e}\n{traceback.format_exc()}", encoding="utf-8")

    return injected_ok, dispatch_id


class NkaiHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    """Specialized HTTP handler with audio/web MIME types, CORS, and mobile QR routes."""

    def __init__(self, *args, directory=None, **kwargs):
        super().__init__(*args, directory=directory, **kwargs)

    def guess_type(self, path):
        """Enforces precise MIME types for audio, SVG, and web files."""
        ext = Path(path).suffix.lower()
        if ext in CUSTOM_MIME_TYPES:
            return CUSTOM_MIME_TYPES[ext]
        return super().guess_type(path)

    def end_headers(self):
        """Appends permissive development CORS headers and auth cookie injection."""
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type, Authorization, Cookie")
        self.send_header("Cache-Control", "no-cache, no-store, must-revalidate")

        # Inject 30-day authentication cookie if ?auth= is in request path or client is from trusted network loading HTML
        sentry_token = getattr(self.server, "sentry_token", None)
        if sentry_token:
            try:
                should_set_cookie = False
                token_val = sentry_token.strip()
                if "?" in self.path:
                    qs = parse_qs(urlparse(self.path).query)
                    if "auth" in qs and qs["auth"]:
                        import hmac
                        if hmac.compare_digest(qs["auth"][0].strip(), token_val):
                            should_set_cookie = True

                if not should_set_cookie:
                    clean_path = urlparse(self.path).path
                    if clean_path.endswith(".html") or clean_path in ("", "/", "/chat"):
                        should_set_cookie = True

                if should_set_cookie:
                    self.send_header(
                        "Set-Cookie",
                        f"nkai_auth={token_val}; Path=/; SameSite=Lax; Max-Age=2592000"
                    )
            except Exception:
                pass

        super().end_headers()

    def do_OPTIONS(self):
        """Handles pre-flight CORS checks."""
        self.send_response(204)
        self.end_headers()

    def is_authorized(self) -> bool:
        """Validates Bearer token from header, query param (?auth=), cookie, or localhost/private networks."""
        sentry_token = getattr(self.server, "sentry_token", None)
        if not sentry_token:
            return True

        # Check Cloudflare Access Authenticated Headers
        if self.headers.get("Cf-Access-Authenticated-User-Email") or self.headers.get("Cf-Access-Jwt-Assertion"):
            return True

        # Check Host: nkai.cykranosh.com (Protected by Cloudflare Zero Trust tunnel)
        host = self.headers.get("Host", "").lower()
        if "cykranosh.com" in host:
            return True

        # Check Authorization header (Bearer <token>)
        auth_header = self.headers.get("Authorization")
        if auth_header and verify_sentry_token(auth_header, sentry_token):
            return True

        # Check Cookie (nkai_auth=<token>)
        cookie_header = self.headers.get("Cookie")
        if cookie_header:
            from http.cookies import SimpleCookie
            try:
                cookie = SimpleCookie(cookie_header)
                if "nkai_auth" in cookie and cookie["nkai_auth"].value:
                    token_val = cookie["nkai_auth"].value.strip()
                    import hmac
                    if hmac.compare_digest(token_val, sentry_token.strip()):
                        return True
            except Exception:
                pass

        # Check query param ?auth=<token>
        try:
            qs = parse_qs(urlparse(self.path).query)
            if "auth" in qs and qs["auth"]:
                import hmac
                if hmac.compare_digest(qs["auth"][0].strip(), sentry_token.strip()):
                    return True
        except Exception:
            pass

        # Allow localhost / loopback, private LAN (192.168.x, 10.x, 172.16-31.x), and Tailscale (100.x)
        allow_local = getattr(self.server, "allow_unauthenticated_localhost", True)
        client_ip = self.client_address[0]
        if allow_local:
            if client_ip in ("127.0.0.1", "::1", "localhost"):
                return True
            try:
                ip = ipaddress.ip_address(client_ip)
                if ip.is_loopback or ip.is_private or (ip.version == 4 and client_ip.startswith("100.")):
                    return True
            except Exception:
                pass

        # Allow public access to PWA icons, manifest, and service worker
        clean_path = urlparse(self.path).path.rstrip("/")
        if clean_path in (
            "/manifest.json",
            "/apple-touch-icon.png",
            "/apple-touch-icon-precomposed.png",
            "/favicon.png",
            "/favicon.ico",
            "/apple-touch-icon",
            "/sw.js",
            "/sw",
        ):
            return True

        return False

    def translate_path(self, path):
        """Translates virtual URL routes and sidecar aliases to physical files."""
        parsed = urlparse(path)
        clean_path = parsed.path
        query = parse_qs(parsed.query)

        # Handle /cockpit route
        if clean_path in ("/cockpit", "/cockpit/"):
            clean_path = "/cockpit.html"
        elif clean_path in ("/chat", "/chat/", "/chat_board", "/chat_board.html", "/intercom", "/intercom/"):
            clean_path = "/chat_board.html"
        elif clean_path in ("/sliver", "/sliver/", "/sliver_engine"):
            clean_path = "/sliver_engine_panel.html"
        elif clean_path in ("/studio", "/studio/", "/faceplate", "/faceplate/"):
            clean_path = "/klang_faceplate_studio.html"

        # Handle ?board=<id> parameter on root or default to morning_mail
        if clean_path in ("", "/"):
            board_id = query.get("board", [None])[0]
            if board_id == "cockpit":
                clean_path = "/cockpit.html"
            elif board_id:
                try:
                    reg_file = REPO_ROOT / ".agents" / "pipeline" / "boards" / "registry.json"
                    if reg_file.exists():
                        reg_data = json.loads(reg_file.read_text(encoding="utf-8"))
                        for b in reg_data.get("boards", []):
                            if b.get("id") == board_id:
                                fname = b.get("file_path", "").split("/")[-1]
                                for cand in [
                                    REPO_ROOT / ".agents" / "sidecar" / fname,
                                    Path(r"c:\Dev\nkai\sidecars\panel") / fname,
                                    Path(r"c:\Dev\nkai\plugin\sidecars\panel") / fname,
                                ]:
                                    if cand.exists():
                                        return str(cand)
                except Exception:
                    pass
            if clean_path in ("", "/"):
                for cand_cockpit in [
                    REPO_ROOT / ".agents" / "sidecar" / "cockpit.html",
                    Path(r"c:\Dev\nkai\sidecars\panel\cockpit.html"),
                    REPO_ROOT / "cockpit.html",
                ]:
                    if cand_cockpit.exists():
                        return str(cand_cockpit)
                clean_path = "/morning_mail_inbox.html"

        filename = clean_path.lstrip("/")

        # Check known sidecar locations dynamically
        sidecar_candidates = [
            REPO_ROOT / ".agents" / "sidecar" / filename,
            Path(r"c:\Dev\nkai\sidecars\panel") / filename,
            Path(r"c:\Dev\nkai\plugin\sidecars\panel") / filename,
            Path(r"c:\Dev\TheKlangSuite") / filename,
            Path(r"C:\_\Vault\New Klang City\Research\ui_ux") / filename,
        ]
        for cand in sidecar_candidates:
            if cand.is_file():
                return str(cand)

        return super().translate_path(path)

    def do_GET(self):
        """Serves mobile QR code vector graphics, pairing page, service worker, and web push keys."""
        clean_path = self.path.split("?")[0].rstrip("/")

        # Service Worker endpoint (Flight 25)
        if clean_path in ("/sw.js", "/sw"):
            sw_path = REPO_ROOT / ".agents" / "sidecar" / "sw.js"
            if not sw_path.exists():
                sw_path = REPO_ROOT / "sw.js"
            if sw_path.exists():
                sw_data = sw_path.read_bytes()
                self.send_response(200)
                self.send_header("Content-Type", "application/javascript; charset=utf-8")
                self.send_header("Service-Worker-Allowed", "/")
                self.send_header("Cache-Control", "no-cache, must-revalidate")
                self.send_header("Content-Length", str(len(sw_data)))
                self.end_headers()
                self.wfile.write(sw_data)
                return

        # VAPID Public Key for Web Push (Flight 25)
        if clean_path == "/api/push/vapid-public-key":
            try:
                from tools.webpush_service import get_or_create_vapid_keys
                keys = get_or_create_vapid_keys()
                resp = json.dumps({"ok": True, "publicKey": keys["public_key"]}).encode("utf-8")
            except Exception as e:
                resp = json.dumps({"ok": False, "error": str(e)}).encode("utf-8")
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Cache-Control", "no-cache")
            self.send_header("Content-Length", str(len(resp)))
            self.end_headers()
            self.wfile.write(resp)
            return

        if clean_path in ("/qr", "/mobile") or self.path.startswith("/qr?") or self.path.startswith("/mobile?"):
            target_url = getattr(self.server, "mobile_url", f"http://{self.server.server_address[0]}:{self.server.server_address[1]}/")

            # Raw SVG endpoint for direct image tags
            if self.path.endswith(".svg") or "?format=svg" in self.path or "&format=svg" in self.path:
                svg_data = generate_qr_svg(target_url, border=2, size=320)
                self.send_response(200)
                self.send_header("Content-Type", "image/svg+xml")
                self.end_headers()
                self.wfile.write(svg_data.encode("utf-8"))
                return

            svg_markup = generate_qr_svg(target_url, border=2, size=260)
            html_page = f"""<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8"/>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, viewport-fit=cover"/>
  <title>📱 N'kai Mobile Companion Pairing</title>
  <style>
    :root {{
      --bg: #090d13;
      --card-bg: #111620;
      --border: #263345;
      --text: #cbd5e1;
      --muted: #8494a7;
      --accent: #38bdf8;
      --accent-green: #34d399;
      --font-mono: 'JetBrains Mono', Consolas, monospace;
      --font-sans: 'Inter', -apple-system, sans-serif;
    }}
    * {{ box-sizing: border-box; margin: 0; padding: 0; }}
    body {{
      background: var(--bg);
      color: var(--text);
      font-family: var(--font-sans);
      min-height: 100vh;
      min-height: 100dvh;
      display: flex;
      align-items: center;
      justify-content: center;
      padding: 20px;
    }}
    .pair-card {{
      background: var(--card-bg);
      border: 1px solid var(--border);
      border-radius: 16px;
      padding: 32px 24px;
      max-width: 440px;
      width: 100%;
      text-align: center;
      box-shadow: 0 12px 32px rgba(0, 0, 0, 0.5);
    }}
    .pair-badge {{
      display: inline-flex;
      align-items: center;
      gap: 6px;
      font-family: var(--font-mono);
      font-size: 11px;
      font-weight: 700;
      color: var(--accent-green);
      background: rgba(52, 211, 153, 0.12);
      border: 1px solid rgba(52, 211, 153, 0.3);
      padding: 4px 10px;
      border-radius: 20px;
      margin-bottom: 16px;
    }}
    h1 {{
      font-size: 20px;
      font-weight: 800;
      margin-bottom: 8px;
      color: #f8fafc;
    }}
    p.desc {{
      font-size: 13px;
      color: var(--muted);
      line-height: 1.5;
      margin-bottom: 24px;
    }}
    .qr-container {{
      background: #ffffff;
      padding: 16px;
      border-radius: 12px;
      display: inline-block;
      margin-bottom: 20px;
      box-shadow: 0 4px 16px rgba(0, 0, 0, 0.2);
    }}
    .qr-container svg {{
      display: block;
      width: 240px;
      height: 240px;
    }}
    .url-box {{
      background: #0d1117;
      border: 1px solid var(--border);
      border-radius: 8px;
      padding: 10px 12px;
      font-family: var(--font-mono);
      font-size: 12px;
      word-break: break-all;
      margin-bottom: 20px;
      color: var(--accent);
    }}
    .url-box a {{
      color: inherit;
      text-decoration: none;
    }}
    .btn-row {{
      display: flex;
      gap: 10px;
    }}
    .btn {{
      flex: 1;
      min-height: 44px;
      padding: 10px 16px;
      border-radius: 8px;
      font-weight: 600;
      font-size: 13px;
      cursor: pointer;
      display: inline-flex;
      align-items: center;
      justify-content: center;
      gap: 8px;
      text-decoration: none;
      transition: all 0.15s ease;
    }}
    .btn-primary {{
      background: var(--accent);
      color: #090d13;
      border: 1px solid var(--accent);
    }}
    .btn-secondary {{
      background: var(--card-bg);
      color: var(--text);
      border: 1px solid var(--border);
    }}
  </style>
</head>
<body>
  <div class="pair-card">
    <div class="pair-badge">📶 LAN ACTIVE</div>
    <h1>📱 N'kai Mobile Companion</h1>
    <p class="desc">Scan this QR code with your phone camera (iOS Safari or Android Chrome) to open your active triage board.</p>
    <div class="qr-container">
      {svg_markup}
    </div>
    <div class="url-box">
      <a href="{target_url}" target="_blank">{target_url}</a>
    </div>
    <div class="btn-row">
      <a href="{target_url}" class="btn btn-primary">🚀 Open Board</a>
      <button onclick="navigator.clipboard.writeText('{target_url}'); alert('URL copied!');" class="btn btn-secondary">📋 Copy URL</button>
    </div>
  </div>
  <script>
    try {{
      const urlParams = new URLSearchParams(window.location.search);
      const auth = urlParams.get('auth');
      if (auth) {{
        localStorage.setItem('nkai_sentry_auth', auth);
      }}
    }} catch (e) {{}}
  </script>
</body>
</html>"""
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.end_headers()
            self.wfile.write(html_page.encode("utf-8"))
            return

        if clean_path in ("/api/status", "/api/health"):
            try:
                from tools.nkai_daemon import evaluate_system_health
                health = evaluate_system_health(assume_online=True)
            except Exception as e:
                health = {"state_code": 0, "state_name": "NOMINAL", "error": str(e), "is_online": True}

            ts_ip = getattr(self.server, "tailscale_ip", None) or get_tailscale_ip() or get_local_ip()
            health["tailscale_ip"] = ts_ip
            health["port"] = self.server.server_address[1]
            health["version"] = "0.4.0"

            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.wfile.write(json.dumps(health, indent=2).encode("utf-8"))
            return

        if clean_path in ("/api/telemetry", "/api/system/telemetry"):
            try:
                from tools.telemetry_engine import get_unified_telemetry
                force_refresh = ("force=1" in self.path) or ("refresh=1" in self.path)
                telemetry_data = get_unified_telemetry(force_refresh_quota=force_refresh)
            except Exception as e:
                telemetry_data = {"error": str(e), "timestamp": time.strftime("%H:%M:%S")}

            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.send_header("Cache-Control", "no-cache, must-revalidate")
            self.end_headers()
            self.wfile.write(json.dumps(telemetry_data, indent=2).encode("utf-8"))
            return

        if clean_path in ("/api/synth-news", "/api/news", "/api/synth_news"):
            news_file = REPO_ROOT / ".agents" / "sidecar" / "synth_news.json"
            if not news_file.exists():
                news_file = Path(r"C:\Dev\TheKlangSuite\.agents\sidecar\synth_news.json")
            if news_file.exists():
                with open(news_file, "r", encoding="utf-8") as f:
                    content = f.read()
            else:
                content = json.dumps({"articles": []})
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.send_header("Cache-Control", "no-cache, must-revalidate")
            self.end_headers()
            self.wfile.write(content.encode("utf-8"))
            return

        if clean_path in ("/api/mail", "/api/rejigger", "/api/mailbag"):
            mail_file = REPO_ROOT / ".agents" / "pipeline" / "mailbag" / "rejigger.json"
            if not mail_file.exists():
                mail_file = Path(r"C:\Dev\TheKlangSuite\.agents\pipeline\mailbag\rejigger.json")
            if mail_file.exists():
                with open(mail_file, "r", encoding="utf-8") as f:
                    content = f.read()
            else:
                content = json.dumps({"cards": []})
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.send_header("Cache-Control", "no-cache, must-revalidate")
            self.end_headers()
            self.wfile.write(content.encode("utf-8"))
            return

        if clean_path == "/api/presence":
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.wfile.write(json.dumps(get_presence_state()).encode("utf-8"))
            return

        if clean_path in ("/api/auth/session", "/api/auth/token"):
            sentry_token = getattr(self.server, "sentry_token", "")
            is_auth = self.is_authorized()
            res = {
                "ok": True,
                "authenticated": is_auth,
                "token": sentry_token if is_auth else None
            }
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.wfile.write(json.dumps(res).encode("utf-8"))
            return

        if clean_path == "/api/boards":
            try:
                from tools.board_registry import load_registry
                reg = load_registry()
                data = reg.model_dump()
            except Exception:
                reg_file = REPO_ROOT / ".agents" / "pipeline" / "boards" / "registry.json"
                if reg_file.exists():
                    data = json.loads(reg_file.read_text(encoding="utf-8"))
                else:
                    data = {"boards": []}
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.end_headers()
            self.wfile.write(json.dumps(data).encode("utf-8"))
            return

        if clean_path in ("/api/agenda", "/api/flight-deck"):
            try:
                import importlib
                if "tools.agenda_engine" in sys.modules:
                    importlib.reload(sys.modules["tools.agenda_engine"])
                from tools.agenda_engine import parse_agenda
                data = parse_agenda()
            except Exception as e:
                data = {"success": False, "error": str(e), "runway": [], "open_loops": [], "tabled_talks": []}
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.wfile.write(json.dumps(data).encode("utf-8"))
            return

        if clean_path == "/api/agy/conversations":
            try:
                from tools.agy_bridge import get_conversations
                data = get_conversations()
            except Exception as e:
                data = {"error": str(e), "conversations": [], "current_id": None, "pinned_ids": []}
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.send_header("Cache-Control", "no-cache, must-revalidate")
            self.end_headers()
            self.wfile.write(json.dumps(data, indent=2).encode("utf-8"))
            return

        if clean_path == "/api/agy/history":
            try:
                from urllib.parse import parse_qs as _parse_qs, urlparse as _urlparse
                from tools.agy_bridge import get_conversation_history, _get_active_presence_conv_id
                q = _parse_qs(_urlparse(self.path).query)
                conv_id = q.get("conv_id", [None])[0]
                limit = int(q.get("limit", [50])[0])
                if not conv_id:
                    conv_id = _get_active_presence_conv_id() or "6d5f68bf-c7b4-4129-bf75-1f5dc5e1fa41"
                turns = get_conversation_history(conv_id, limit=limit)
                data = {"ok": True, "conv_id": conv_id, "turns": turns}
            except Exception as e:
                data = {"ok": False, "error": str(e), "turns": []}
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.send_header("Cache-Control", "no-cache, must-revalidate")
            self.end_headers()
            self.wfile.write(json.dumps(data).encode("utf-8"))
            return

        if clean_path == "/api/chat/stream":
            self.send_response(200)
            self.send_header("Content-Type", "text/event-stream; charset=utf-8")
            self.send_header("Cache-Control", "no-cache, no-transform")
            self.send_header("Connection", "keep-alive")
            self.send_header("X-Accel-Buffering", "no")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.handle_chat_sse_stream()
            return

        if clean_path == "/api/chat/history":
            try:
                from tools.chat_engine import parse_transcript_turns
                data = parse_transcript_turns()
            except Exception as e:
                data = {"ok": False, "error": str(e), "turns": []}
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.end_headers()
            self.wfile.write(json.dumps(data).encode("utf-8"))
            return

        if clean_path == "/api/scratchpad/status":
            status = ScratchpadSentinel.inspect()
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.end_headers()
            self.wfile.write(json.dumps(status).encode("utf-8"))
            return

        if clean_path == "/api/scratchpad/content":
            sp_path = ScratchpadSentinel.get_path()
            content = ""
            if sp_path.exists():
                for i in range(3):
                    try:
                        with open(sp_path, "r", encoding="utf-8", errors="replace") as f:
                            content = f.read()
                        break
                    except (PermissionError, OSError):
                        if i < 2:
                            time.sleep(0.05 * (2 ** i))
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.end_headers()
            self.wfile.write(json.dumps({"ok": True, "content": content, "path": str(sp_path)}).encode("utf-8"))
            return

        if clean_path == "/api/bulletin":
            bulletin_file = REPO_ROOT / ".agents" / "pipeline" / "mailbag" / "agent_bulletin.json"
            if bulletin_file.exists():
                try:
                    data = json.loads(bulletin_file.read_text(encoding="utf-8"))
                except Exception:
                    data = {"briefing_title": "Agent Standby", "briefing_text": "System operational."}
            else:
                data = {"briefing_title": "Agent Standby", "briefing_text": "System operational."}
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.end_headers()
            self.wfile.write(json.dumps(data).encode("utf-8"))
            return

        if clean_path.startswith("/api/card/"):
            card_id = clean_path[len("/api/card/"):].strip()
            queue_file = REPO_ROOT / ".agents" / "pipeline" / "mailbag" / "mail_queue.json"
            clean_target = re.sub(r"[^a-zA-Z0-9_-]", "_", card_id).lower()
            found_card = None
            if queue_file.exists():
                try:
                    q_data = json.loads(queue_file.read_text(encoding="utf-8"))
                    items = q_data.get("items", [])
                    for it in items:
                        it_id = str(it.get("id", "")).lower()
                        clean_it_id = re.sub(r"[^a-zA-Z0-9_-]", "_", it_id)
                        if (
                            it_id == card_id.lower()
                            or clean_it_id == clean_target
                            or card_id.lower() in (f"mail-card-{it.get('idx')}", f"mail-card-{items.index(it)}")
                            or clean_target == clean_it_id.strip("_")
                        ):
                            found_card = it
                            break
                except Exception:
                    pass

            if found_card:
                self.send_response(200)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.end_headers()
                self.wfile.write(json.dumps({"ok": True, "card": found_card}).encode("utf-8"))
            else:
                self.send_response(404)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.end_headers()
                self.wfile.write(json.dumps({"ok": False, "error": f"Card '{card_id}' not found"}).encode("utf-8"))
            return

        if clean_path == "/api/compile-prompt":
            parsed = parse_qs(urlparse(self.path).query)
            topic = parsed.get("topic", [None])[0] or parsed.get("id", [None])[0]
            to_clipboard = parsed.get("clipboard", ["true"])[0].lower() in ("true", "1", "yes")

            if not topic:
                self.send_response(400)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.send_header("Access-Control-Allow-Origin", "*")
                self.end_headers()
                self.wfile.write(json.dumps({"ok": False, "error": "Missing 'topic' or 'id' query parameter"}).encode("utf-8"))
                return

            try:
                from tools.compile_prompt import compile_prompt
                payload = compile_prompt(topic=topic, clipboard=to_clipboard)
                self.send_response(200)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.send_header("Access-Control-Allow-Origin", "*")
                self.end_headers()
                self.wfile.write(json.dumps({
                    "ok": True,
                    "thread_id": payload.thread_id,
                    "markdown": payload.to_markdown(),
                    "copied": to_clipboard
                }, ensure_ascii=False).encode("utf-8"))
            except Exception as e:
                self.send_response(500)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.send_header("Access-Control-Allow-Origin", "*")
                self.end_headers()
                self.wfile.write(json.dumps({"ok": False, "error": str(e)}).encode("utf-8"))
            return

        if clean_path == "/preload.js":
            shim = b"""// Standalone Mobile/LAN Webview Shim for N'kai Sidecar SDK
window.sidecar = window.sidecar || {
  isStandalone: true,
  token: null,
  agent: {
    sendMessage: function(prompt) {
      return fetch('/api/chat/send', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ prompt: prompt, source: 'mobile_preload_shim' })
      });
    }
  }
};
"""
            self.send_response(200)
            self.send_header("Content-Type", "application/javascript; charset=utf-8")
            self.send_header("Content-Length", str(len(shim)))
            self.send_header("Cache-Control", "public, max-age=3600")
            self.end_headers()
            self.wfile.write(shim)
            return

        if clean_path.startswith("/uploads/"):
            fn = Path(clean_path[len("/uploads/"):].lstrip("/")).name
            upload_file = REPO_ROOT / ".agents" / "pipeline" / "uploads" / fn
            if upload_file.exists() and upload_file.is_file():
                mime, _ = mimetypes.guess_type(str(upload_file))
                self.send_response(200)
                self.send_header("Content-Type", mime or "application/octet-stream")
                self.send_header("Content-Length", str(upload_file.stat().st_size))
                self.send_header("Cache-Control", "public, max-age=3600")
                self.end_headers()
                with open(upload_file, "rb") as f:
                    shutil.copyfileobj(f, self.wfile)
                return
            else:
                self.send_response(404)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.end_headers()
                self.wfile.write(json.dumps({"ok": False, "error": f"Upload '{fn}' not found"}).encode("utf-8"))
                return

        if clean_path in (
            "/apple-touch-icon.png",
            "/apple-touch-icon-precomposed.png",
            "/favicon.png",
            "/favicon.ico",
            "/apple-touch-icon",
        ):
            mode = "lan"
            if "?" in self.path:
                q = self.path.split("?", 1)[1].lower()
                if "remote" in q or "tailscale" in q:
                    mode = "remote"
                elif "codex" in q or "forge" in q:
                    mode = "codex"
            host = self.headers.get("Host", "")
            if host.startswith("100.") or "cykranosh" in host:
                mode = "remote"

            icon_file = None
            is_fav = clean_path in ("/favicon.png", "/favicon.ico")
            target_name = "favicon.png" if is_fav else "apple-touch-icon.png"

            search_dirs = [
                Path(self.directory) if hasattr(self, "directory") else None,
                REPO_ROOT / ".agents" / "sidecar",
                REPO_ROOT / "sidecars" / "panel",
                REPO_ROOT,
                REPO_ROOT / "plugin" / "assets",
            ]
            for sdir in search_dirs:
                if sdir and (sdir / target_name).exists():
                    icon_file = sdir / target_name
                    break

            if icon_file and icon_file.exists():
                icon_data = icon_file.read_bytes()
            else:
                icon_data = generate_touch_icon(mode)

            self.send_response(200)
            self.send_header("Content-Type", "image/png")
            self.send_header("Content-Length", str(len(icon_data)))
            self.send_header("Cache-Control", "no-cache, must-revalidate")
            self.end_headers()
            self.wfile.write(icon_data)
            return

        if clean_path == "/manifest.json":
            mode = "lan"
            if "?" in self.path:
                q = self.path.split("?", 1)[1].lower()
                if "remote" in q or "tailscale" in q:
                    mode = "remote"
                elif "codex" in q or "forge" in q:
                    mode = "codex"
            host = self.headers.get("Host", "")
            if host.startswith("100.") or "cykranosh" in host:
                mode = "remote"

            app_title = "N'kai" if "cykranosh" in host else ("N'kai (Remote)" if mode == "remote" else "N'kai (Home)")
            manifest = {
                "name": "N'kai — The Sovereign Command Deck",
                "short_name": app_title,
                "start_url": f"/?mode={mode}",
                "display": "standalone",
                "background_color": "#090d13",
                "theme_color": "#090d13",
                "icons": [
                    {
                        "src": "/apple-touch-icon.png?v=cykranosh",
                        "sizes": "180x180",
                        "type": "image/png",
                    }
                ],
            }
            manifest_bytes = json.dumps(manifest, indent=2).encode("utf-8")
            self.send_response(200)
            self.send_header("Content-Type", "application/manifest+json; charset=utf-8")
            self.send_header("Content-Length", str(len(manifest_bytes)))
            self.send_header("Cache-Control", "no-cache, must-revalidate")
            self.end_headers()
            self.wfile.write(manifest_bytes)
            return

        super().do_GET()

    def do_POST(self):
        """Captures mobile UI action requests, quick notes, and mail sync."""
        clean_path = self.path.split("?")[0].rstrip("/")
        content_length = int(self.headers.get("Content-Length", 0))
        post_body = self.rfile.read(content_length) if content_length > 0 else b""

        try:
            body_json = json.loads(post_body.decode("utf-8")) if post_body else {}
        except Exception:
            body_json = {}

        # Sentry Authorization Check on protected API endpoints
        protected_routes = (
            "/api/chat/upload",
            "/api/chat/send",
            "/api/action",
            "/api/quick-note",
            "/api/dispatch",
            "/api/sentry/stream",
            "/api/chat",
            "/api/flush",
            "/api/context/flush",
            "/api/agenda/route",
            "/api/agenda/add",
        )
        if clean_path in protected_routes or clean_path.startswith("/api/sentry/"):
            if not self.is_authorized():
                self.send_response(401)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.send_header("Access-Control-Allow-Origin", "*")
                self.end_headers()
                self.wfile.write(json.dumps({
                    "ok": False,
                    "error": "Unauthorized: Sentry Bearer token required or invalid.",
                    "hint": "Provide 'Authorization: Bearer <token>' or scan QR pairing code at /qr"
                }).encode("utf-8"))
                return

        # Context Flush & Session Harvest Endpoint
        if clean_path in ("/api/flush", "/api/context/flush"):
            try:
                from tools.flush_context import run_flush
                flush_res = run_flush()
            except Exception as e:
                flush_res = {"ok": False, "error": str(e)}

            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.wfile.write(json.dumps(flush_res, indent=2).encode("utf-8"))
            return

        # Web Push API Routes (Flight 25)
        if clean_path == "/api/push/subscribe":
            try:
                from tools.webpush_service import save_subscription
                saved = save_subscription(body_json)
                resp = json.dumps({"ok": saved, "message": "Subscription registered"}).encode("utf-8")
            except Exception as e:
                resp = json.dumps({"ok": False, "error": str(e)}).encode("utf-8")
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.wfile.write(resp)
            return

        if clean_path == "/api/push/unsubscribe":
            try:
                from tools.webpush_service import remove_subscription
                endpoint = body_json.get("endpoint")
                if endpoint:
                    remove_subscription(endpoint)
                resp = json.dumps({"ok": True, "message": "Subscription removed"}).encode("utf-8")
            except Exception as e:
                resp = json.dumps({"ok": False, "error": str(e)}).encode("utf-8")
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.wfile.write(resp)
            return

        if clean_path in ("/api/push/test", "/api/push/send"):
            try:
                from tools.webpush_service import send_push_notification
                title = body_json.get("title", "🛸 N'kai Sovereign Ping")
                body = body_json.get("body", "The Tindalos Conduit Web Push is live and operational!")
                url = body_json.get("url", "/?pillar=chat")
                tag = body_json.get("tag", "nkai-test")
                channel = body_json.get("channel", "test")
                res = send_push_notification(title=title, body=body, url=url, tag=tag, channel=channel)
                resp = json.dumps(res).encode("utf-8")
            except Exception as e:
                resp = json.dumps({"ok": False, "error": str(e)}).encode("utf-8")
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.wfile.write(resp)
            return

        # Sentry SSE Streaming Route (supports both /api/sentry/stream and /api/chat)
        if clean_path in ["/api/sentry/stream", "/api/chat"]:
            prompt = body_json.get("prompt", "").strip()
            attachments = body_json.get("attachments", [])
            history = body_json.get("history", [])
            model = body_json.get("model", "gemini-3.8-flash")
            api_key = getattr(self.server, "gemini_api_key", None) or os.environ.get("GEMINI_API_KEY") or get_gemini_api_key()
            client_ip = self.client_address[0] if hasattr(self, "client_address") else "127.0.0.1"

            # Auto-dispatch prompt to Antigravity desktop & Data Bus
            if prompt:
                try:
                    dispatch_prompt_to_harness(prompt, client_ip, source="mobile_stream")
                except Exception as e:
                    print(f"⚠️ [SENTRY DISPATCH] Could not relay prompt: {e}")

            try:
                if hasattr(self, "request") and self.request:
                    self.request.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
            except Exception:
                pass

            self.send_response(200)
            self.send_header("Content-Type", "text/event-stream; charset=utf-8")
            self.send_header("Cache-Control", "no-cache, no-transform")
            self.send_header("Connection", "keep-alive")
            self.send_header("X-Accel-Buffering", "no")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()

            import asyncio
            loop = asyncio.new_event_loop()
            try:
                gen = stream_sentry_response(prompt, attachments, history=history, api_key=api_key, model=model)
                while True:
                    try:
                        ev = loop.run_until_complete(gen.__anext__())
                        ev_type = ev.get("type", "message")
                        msg = f"event: {ev_type}\ndata: {json.dumps(ev, ensure_ascii=False)}\n\n"
                        self.wfile.write(msg.encode("utf-8"))
                        self.wfile.flush()
                    except StopAsyncIteration:
                        break
            except (BrokenPipeError, ConnectionResetError):
                pass
            finally:
                loop.close()
                self.close_connection = True
            return

        if clean_path == "/api/chat/upload":
            content_type_header = self.headers.get("Content-Type", "")
            saved_files = []
            now_prefix = datetime.now().strftime("%Y%m%d_%H%M%S")
            upload_dir = REPO_ROOT / ".agents" / "pipeline" / "uploads"
            upload_dir.mkdir(parents=True, exist_ok=True)

            brain_upload_dir = None
            try:
                from tools.chat_engine import get_active_conversation_path
                active_conv = get_active_conversation_path()
                if active_conv:
                    brain_upload_dir = active_conv / ".user_uploaded"
                    brain_upload_dir.mkdir(parents=True, exist_ok=True)
            except Exception:
                pass

            if "multipart/form-data" in content_type_header:
                msg = email.message_from_bytes(
                    f"Content-Type: {content_type_header}\r\n\r\n".encode("utf-8") + post_body,
                    policy=email.policy.default
                )
                parts = list(msg.iter_parts()) if msg.is_multipart() else [msg]
                for part in parts:
                    fn = part.get_filename()
                    if not fn:
                        continue
                    safe_fn = re.sub(r"[^\w\s.-]", "", Path(fn).name).strip().replace(" ", "_")
                    if not safe_fn:
                        safe_fn = "upload.bin"
                    rand_suffix = secrets.token_hex(3)
                    target_fn = f"{now_prefix}_{rand_suffix}_{safe_fn}"
                    file_data = part.get_payload(decode=True)
                    if file_data is None:
                        continue

                    target_path = upload_dir / target_fn
                    target_path.write_bytes(file_data)

                    if brain_upload_dir:
                        try:
                            (brain_upload_dir / target_fn).write_bytes(file_data)
                        except Exception:
                            pass

                    ct = part.get_content_type()
                    saved_files.append({
                        "filename": target_fn,
                        "original_name": fn,
                        "abs_path": str(target_path.resolve()),
                        "rel_path": f".agents/pipeline/uploads/{target_fn}",
                        "url": f"/uploads/{target_fn}",
                        "size": len(file_data),
                        "mime_type": ct,
                        "is_image": ct.startswith("image/")
                    })

            elif "application/json" in content_type_header or body_json:
                fn = body_json.get("filename", "upload.bin")
                b64_data = body_json.get("content_base64", "")
                ct = body_json.get("mime_type", "application/octet-stream")
                safe_fn = re.sub(r"[^\w\s.-]", "", Path(fn).name).strip().replace(" ", "_")
                if not safe_fn:
                    safe_fn = "upload.bin"
                rand_suffix = secrets.token_hex(3)
                target_fn = f"{now_prefix}_{rand_suffix}_{safe_fn}"

                try:
                    file_data = base64.b64decode(b64_data)
                    target_path = upload_dir / target_fn
                    target_path.write_bytes(file_data)

                    if brain_upload_dir:
                        try:
                            (brain_upload_dir / target_fn).write_bytes(file_data)
                        except Exception:
                            pass

                    saved_files.append({
                        "filename": target_fn,
                        "original_name": fn,
                        "abs_path": str(target_path.resolve()),
                        "rel_path": f".agents/pipeline/uploads/{target_fn}",
                        "url": f"/uploads/{target_fn}",
                        "size": len(file_data),
                        "mime_type": ct,
                        "is_image": ct.startswith("image/")
                    })
                except Exception as e:
                    self.send_response(400)
                    self.send_header("Content-Type", "application/json; charset=utf-8")
                    self.end_headers()
                    self.wfile.write(json.dumps({"ok": False, "error": f"Base64 decode failed: {e}"}).encode("utf-8"))
                    return

            if not saved_files:
                self.send_response(400)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.end_headers()
                self.wfile.write(json.dumps({"ok": False, "error": "No files uploaded"}).encode("utf-8"))
                return

            res_payload = {
                "ok": True,
                "count": len(saved_files),
                "file": saved_files[0],
                "files": saved_files
            }
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.end_headers()
            self.wfile.write(json.dumps(res_payload, indent=2).encode("utf-8"))
            return

        if clean_path == "/api/presence":
            client = body_json.get("client", "web_client")
            board = body_json.get("board", "unknown")
            focused = body_json.get("focused", True)
            record_presence(client, board, focused)
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.wfile.write(json.dumps({"ok": True, "active": focused}).encode("utf-8"))
            return

        if clean_path in ("/api/chat/send", "/api/agy/send"):
            prompt = body_json.get("prompt", "").strip()
            conv_id = body_json.get("conv_id", "").strip()
            model = body_json.get("model", "flash-high").strip()
            if not prompt:
                self.send_response(400)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.send_header("Access-Control-Allow-Origin", "*")
                self.end_headers()
                self.wfile.write(json.dumps({"ok": False, "error": "Empty prompt"}).encode("utf-8"))
                return

            if not conv_id:
                try:
                    from tools.agy_bridge import _get_active_presence_conv_id
                    conv_id = _get_active_presence_conv_id() or "6d5f68bf-c7b4-4129-bf75-1f5dc5e1fa41"
                except Exception:
                    conv_id = "6d5f68bf-c7b4-4129-bf75-1f5dc5e1fa41"

            # 1. Audit trail logging
            client_ip = self.client_address[0] if hasattr(self, "client_address") else "127.0.0.1"
            injected_ok, dispatch_id = dispatch_prompt_to_harness(prompt, client_ip=client_ip, source="chat_board")

            # 2. Direct ConnectRPC Injection into language_server.exe
            try:
                from tools.agy_dispatch import dispatch_prompt
                disp_res = dispatch_prompt(
                    target_conv=conv_id,
                    prompt=prompt,
                    model_query=model,
                    bypass_quota_gate=False,
                    bypass_idle_gate=True
                )
                self.send_response(200)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.send_header("Access-Control-Allow-Origin", "*")
                self.end_headers()
                self.wfile.write(json.dumps({
                    "ok": True,
                    "status": "dispatched",
                    "conv_id": conv_id,
                    "model": disp_res.get("model_label"),
                    "elapsed_ms": disp_res.get("elapsed_ms"),
                    "dispatch_id": dispatch_id
                }).encode("utf-8"))
                return
            except Exception as e:
                # Fallback to direct send_user_message if agy_dispatch threw error
                try:
                    from tools.agy_bridge import send_user_message
                    rpc_res = send_user_message(conv_id, prompt)
                    self.send_response(200)
                    self.send_header("Content-Type", "application/json; charset=utf-8")
                    self.send_header("Access-Control-Allow-Origin", "*")
                    self.end_headers()
                    self.wfile.write(json.dumps({
                        "ok": True,
                        "status": "dispatched_fallback",
                        "conv_id": conv_id,
                        "rpc_result": rpc_res,
                        "dispatch_id": dispatch_id
                    }).encode("utf-8"))
                    return
                except Exception as e2:
                    self.send_response(500)
                    self.send_header("Content-Type", "application/json; charset=utf-8")
                    self.send_header("Access-Control-Allow-Origin", "*")
                    self.end_headers()
                    self.wfile.write(json.dumps({
                        "ok": False,
                        "error": f"Dispatch failed: {e} | Fallback failed: {e2}",
                        "conv_id": conv_id
                    }).encode("utf-8"))
                    return

        if clean_path in ("/api/scratchpad/append", "/api/sp/append"):
            text = body_json.get("text", "").strip()
            if not text:
                self.send_response(400)
                self.send_header("Content-Type", "application/json")
                self.end_headers()
                self.wfile.write(json.dumps({"ok": False, "error": "Empty note text"}).encode("utf-8"))
                return

            now = datetime.now()
            time_str = now.strftime("%H:%M")
            sp_path = ScratchpadSentinel.get_path()
            sp_path.parent.mkdir(parents=True, exist_ok=True)
            if not sp_path.exists():
                header = "# 📝 Scratchpad & Live Reading Notes\n\n---\n\n## ⚡ Quick Notes & Fleeting Ideas\n\n"
                sp_path.write_text(header, encoding="utf-8")

            for i in range(3):
                try:
                    with open(sp_path, "a", encoding="utf-8") as f:
                        f.write(f"- [{time_str}] {text}\n")
                    break
                except (PermissionError, OSError):
                    if i < 2:
                        time.sleep(0.05 * (2 ** i))

            status = ScratchpadSentinel.inspect()
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.end_headers()
            self.wfile.write(json.dumps({"ok": True, "status": status}).encode("utf-8"))
            return

        if clean_path in ("/api/scratchpad/save", "/api/scratchpad/write"):
            content = body_json.get("content", "")
            sp_path = ScratchpadSentinel.get_path()
            sp_path.parent.mkdir(parents=True, exist_ok=True)
            for i in range(3):
                try:
                    sp_path.write_text(content, encoding="utf-8")
                    break
                except (PermissionError, OSError):
                    if i < 2:
                        time.sleep(0.05 * (2 ** i))
            status = ScratchpadSentinel.inspect()
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.end_headers()
            self.wfile.write(json.dumps({"ok": True, "status": status}).encode("utf-8"))
            return

        if clean_path == "/api/scratchpad/triage":
            status = ScratchpadSentinel.inspect()
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.end_headers()
            self.wfile.write(json.dumps({"ok": True, "message": "Scratchpad triaged", "status": status}).encode("utf-8"))
            return

        if clean_path == "/api/quick-note":
            text = body_json.get("text", "").strip()
            target = body_json.get("target", "scratchpad").lower()
            if not text:
                self.send_response(400)
                self.send_header("Content-Type", "application/json")
                self.end_headers()
                self.wfile.write(json.dumps({"ok": False, "error": "Empty note text"}).encode("utf-8"))
                return

            now = datetime.now()
            time_str = now.strftime("%H:%M")

            if target in ("inbox", "mail", "daily"):
                date_str = now.strftime("%Y-%m-%d")
                inbox_dir = Path(r"C:\_\Vault\Inbox")
                if not inbox_dir.exists():
                    inbox_dir = REPO_ROOT / "docs" / "inbox"
                inbox_dir.mkdir(parents=True, exist_ok=True)

                catchall_path = inbox_dir / f"{date_str}_Quick_Notes.md"
                if not catchall_path.exists():
                    header = f'---\ntitle: "Daily Quick Notes ({date_str})"\ndate: {date_str}\ntags: [quick-notes, daily-catchall]\npriority: P2\npillars: []\n---\n\n# Daily Quick Notes ({date_str})\n\n'
                    catchall_path.write_text(header, encoding="utf-8")

                for i in range(3):
                    try:
                        with open(catchall_path, "a", encoding="utf-8") as f:
                            f.write(f"- [{time_str}] {text}\n")
                        break
                    except (PermissionError, OSError):
                        if i < 2:
                            time.sleep(0.05 * (2 ** i))

                check_mail_script = REPO_ROOT / "tools" / "check_mail.py"
                if check_mail_script.exists():
                    subprocess.run([sys.executable, str(check_mail_script), "--no-browser"], cwd=REPO_ROOT, check=False)

                self.send_response(200)
                self.send_header("Content-Type", "application/json")
                self.end_headers()
                self.wfile.write(json.dumps({"ok": True, "target": "inbox", "file": str(catchall_path)}).encode("utf-8"))
                return
            else:
                # Default to Scratchpad! Zero inbox spam!
                sp_path = ScratchpadSentinel.get_path()
                sp_path.parent.mkdir(parents=True, exist_ok=True)
                if not sp_path.exists():
                    header = "# 📝 Scratchpad & Live Reading Notes\n\n---\n\n## ⚡ Quick Notes & Fleeting Ideas\n\n"
                    sp_path.write_text(header, encoding="utf-8")

                for i in range(3):
                    try:
                        with open(sp_path, "a", encoding="utf-8") as f:
                            f.write(f"- [{time_str}] {text}\n")
                        break
                    except (PermissionError, OSError):
                        if i < 2:
                            time.sleep(0.05 * (2 ** i))

                status = ScratchpadSentinel.inspect()
                self.send_response(200)
                self.send_header("Content-Type", "application/json")
                self.end_headers()
                self.wfile.write(json.dumps({"ok": True, "target": "scratchpad", "status": status}).encode("utf-8"))
                return

        if clean_path == "/api/check-mail":
            check_mail_script = REPO_ROOT / "tools" / "check_mail.py"
            if check_mail_script.exists():
                subprocess.run([sys.executable, str(check_mail_script), "--no-browser"], cwd=REPO_ROOT, check=False)
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.end_headers()
            self.wfile.write(json.dumps({"ok": True, "message": "Mail checked"}).encode("utf-8"))
            return

        if clean_path == "/api/compile-prompt":
            topic = body_json.get("topic") or body_json.get("topic_id") or body_json.get("id") or body_json.get("card_id")
            to_clipboard = body_json.get("clipboard", True)

            if not topic:
                self.send_response(400)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.end_headers()
                self.wfile.write(json.dumps({"ok": False, "error": "Missing topic identifier"}).encode("utf-8"))
                return

            try:
                from tools.compile_prompt import compile_prompt
                payload = compile_prompt(topic=str(topic), clipboard=bool(to_clipboard))
                self.send_response(200)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.send_header("Access-Control-Allow-Origin", "*")
                self.end_headers()
                self.wfile.write(json.dumps({
                    "ok": True,
                    "thread_id": payload.thread_id,
                    "markdown": payload.to_markdown(),
                    "copied": bool(to_clipboard)
                }, ensure_ascii=False).encode("utf-8"))
            except Exception as e:
                self.send_response(500)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.send_header("Access-Control-Allow-Origin", "*")
                self.end_headers()
                self.wfile.write(json.dumps({"ok": False, "error": str(e)}).encode("utf-8"))
            return

        if clean_path in ("/api/action", "/api/dispatch"):
            action = str(body_json.get("action", "")).upper()
            raw_card_id = str(body_json.get("card_id") or body_json.get("note_id", ""))
            card_id = raw_card_id.replace("mail-card-", "")
            title = str(body_json.get("title", "")).strip()
            pillar = str(body_json.get("pillar") or body_json.get("target") or "TKS").upper()
            priority = str(body_json.get("priority", "P1")).upper()
            client_ip = self.client_address[0] if hasattr(self, "client_address") else "127.0.0.1"

            print(f"\n[N'KAI MECHANICAL TRIAGE] Processing '{action}' for '{card_id}' ({title}) [Pillar: {pillar}] from {client_ip}")

            # 1. Append to triage_ledger.jsonl (Write-Ahead Log)
            ledger_dir = REPO_ROOT / ".agents" / "pipeline" / "mailbag"
            ledger_dir.mkdir(parents=True, exist_ok=True)
            ledger_file = ledger_dir / "triage_ledger.jsonl"

            now_iso = datetime.now().isoformat()
            tx_record = {
                "timestamp": now_iso,
                "action": action,
                "card_id": card_id,
                "title": title,
                "pillar": pillar,
                "priority": priority,
                "client_ip": client_ip,
            }
            try:
                with open(ledger_file, "a", encoding="utf-8") as f:
                    f.write(json.dumps(tx_record, ensure_ascii=False) + "\n")
            except Exception as e:
                print(f"⚠️ Failed to write to triage_ledger.jsonl: {e}")

            # 2. Atomic card archival in mail_queue.json & note movement to Inbox/Archive/
            archived = False
            if action in ("APPROVE", "ARCHIVE", "COMPLETED", "BACKLOG", "GRAVEYARD") and card_id:
                try:
                    from tools.check_mail import handle_archive_card
                    q_target = REPO_ROOT / ".agents" / "pipeline" / "mailbag" / "mail_queue.json"
                    o_target = REPO_ROOT / ".agents" / "sidecar" / "morning_mail_inbox.html"
                    if q_target.exists():
                        res = handle_archive_card(
                            target_id=card_id,
                            queue_path=q_target,
                            out_path=o_target,
                            is_sandbox=False,
                            inbox_dir_override=None,
                        )
                        archived = (res == 0)
                except Exception as e:
                    print(f"⚠️ Failed handle_archive_card: {e}")

            # 3. Cross-Pillar Bus Dispatch: write dispatch ticket to Dispatches/ for external pillars
            dispatched_bus = False
            if pillar in ("COSMOS", "TT", "TR", "FORGE"):
                try:
                    from tools.bus import resolve_bus_dir, serialize_dispatch
                    bus_dir = resolve_bus_dir()
                    today = datetime.now().strftime("%Y-%m-%d")
                    slug_base = re.sub(r"[^a-zA-Z0-9_-]", "_", title.lower()).strip("_")[:30] or card_id
                    dispatch_id = f"{today}_nk_to_{pillar.lower()}_{slug_base}"
                    target_file = bus_dir / f"{dispatch_id}.md"

                    meta = {
                        "id": dispatch_id,
                        "date": today,
                        "from": "NK",
                        "to": pillar,
                        "tags": [pillar.lower(), "mobile-triage", action.lower()],
                        "priority": priority or "P1",
                        "status": "PENDING",
                        "title": f"Triage Action [{action}]: {title}",
                    }
                    body = (
                        f"# Triage Action: {action} for {title}\n\n"
                        f"- **Action Verdict**: `{action}`\n"
                        f"- **Source Card ID**: `{card_id}`\n"
                        f"- **Target Pillar**: `{pillar}`\n"
                        f"- **Logged At**: `{now_iso}`\n"
                        f"- **Origin**: N'kai Mobile Companion (`{client_ip}`)\n\n"
                        f"### Executive Instructions:\n"
                        f"The user triaged this card via N'kai mobile. Execute or schedule this ticket in the {pillar} repository backlog/workspace.\n"
                    )
                    content = serialize_dispatch(meta, body)
                    target_file.write_text(content, encoding="utf-8")
                    print(f"🚀 Bus dispatch automatically posted to {pillar}: {target_file.name}")
                    dispatched_bus = True
                except Exception as e:
                    print(f"⚠️ Failed auto-bus post: {e}")

            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.end_headers()
            self.wfile.write(json.dumps({
                "ok": True,
                "status": "action_processed",
                "card_id": card_id,
                "action": action,
                "archived": archived,
                "dispatched_bus": dispatched_bus
            }).encode("utf-8"))
            return

        if clean_path in ("/api/agenda/route", "/api/agenda/add"):
            try:
                from tools.agenda_engine import route_card_to_agenda
                title = str(body_json.get("title", "")).strip()
                summary = str(body_json.get("summary") or body_json.get("notes") or "").strip()
                target_section = str(body_json.get("target_section", "open_loops")).strip().lower()
                unlock_trigger = body_json.get("unlock_trigger")
                priority = str(body_json.get("priority", "P2")).strip().upper()
                track = str(body_json.get("track", "📬 Mail")).strip()
                card_id = body_json.get("card_id")

                res = route_card_to_agenda(
                    title=title,
                    summary=summary,
                    target_section=target_section,
                    unlock_trigger=unlock_trigger,
                    priority=priority,
                    track=track,
                    card_id=card_id,
                )
                self.send_response(200)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.send_header("Access-Control-Allow-Origin", "*")
                self.end_headers()
                self.wfile.write(json.dumps(res).encode("utf-8"))
            except Exception as e:
                self.send_response(500)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.send_header("Access-Control-Allow-Origin", "*")
                self.end_headers()
                self.wfile.write(json.dumps({"success": False, "error": str(e)}).encode("utf-8"))
            return

        if clean_path == "/api/conclude-lab":
            slug = body_json.get("slug", "mobile_lab")
            conclude_script = REPO_ROOT / "tools" / "conclude_lab.py"
            if conclude_script.exists():
                subprocess.run([sys.executable, str(conclude_script), "--slug", slug], cwd=REPO_ROOT, check=False)
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.end_headers()
            self.wfile.write(json.dumps({"ok": True}).encode("utf-8"))
            return

    def _send_sse(self, event_name: str, payload_dict: dict) -> bool:
        """Sends an SSE event to self.wfile, returning False if disconnected."""
        try:
            line = f"event: {event_name}\ndata: {json.dumps(payload_dict, ensure_ascii=False)}\n\n"
            self.wfile.write(line.encode("utf-8"))
            self.wfile.flush()
            return True
        except (ConnectionResetError, BrokenPipeError, ConnectionAbortedError, OSError):
            return False

    def _send_sse_comment(self, comment: str = "keep-alive") -> bool:
        """Sends an SSE comment ping to keep proxies and tunnels warm."""
        try:
            self.wfile.write(f": {comment}\n\n".encode("utf-8"))
            self.wfile.flush()
            return True
        except (ConnectionResetError, BrokenPipeError, ConnectionAbortedError, OSError):
            return False

    def handle_chat_sse_stream(self):
        """Streams real-time chat updates and tool execution events via Server-Sent Events (SSE).

        Flight 27: Reactive Streaming Chat & SSE Feedback Loop.
        Monitors active Antigravity transcript for new turns, tool execution progress,
        and turn completion. Broadcasts automated push notifications when an agent turn
        finishes while user presence is inactive.
        """
        from tools.chat_engine import get_active_conversation_path, parse_transcript_turns

        conv_dir = get_active_conversation_path()
        if not conv_dir or not conv_dir.exists():
            self._send_sse("error", {"error": "No active conversation found"})
            return

        tf = conv_dir / ".system_generated" / "logs" / "transcript_full.jsonl"
        if not tf.exists():
            tf = conv_dir / ".system_generated" / "logs" / "transcript.jsonl"

        # Emit initial snapshot
        try:
            initial_data = parse_transcript_turns(conv_dir=conv_dir, max_turns=30)
            if not self._send_sse("snapshot", initial_data):
                return
        except Exception as e:
            self._send_sse("error", {"error": str(e)})
            return

        last_mtime = tf.stat().st_mtime if tf.exists() else 0.0
        last_size = tf.stat().st_size if tf.exists() else 0
        was_running = initial_data.get("is_running", False)
        last_heartbeat = time.time()
        last_notified_turn = -1

        while True:
            now = time.time()
            # Send keep-alive comment every 15s to keep proxy/Cloudflare connections open
            if now - last_heartbeat >= 15.0:
                if not self._send_sse_comment("keep-alive"):
                    break
                last_heartbeat = now

            time.sleep(0.3)

            # Check if active conversation directory switched
            curr_conv = get_active_conversation_path()
            if curr_conv and curr_conv != conv_dir:
                conv_dir = curr_conv
                tf = conv_dir / ".system_generated" / "logs" / "transcript_full.jsonl"
                if not tf.exists():
                    tf = conv_dir / ".system_generated" / "logs" / "transcript.jsonl"
                last_mtime = 0.0
                last_size = 0

            if not tf.exists():
                continue

            try:
                st = tf.stat()
            except OSError:
                continue

            # If file size and mtime haven't changed, nothing to do
            if st.st_mtime == last_mtime and st.st_size == last_size:
                continue

            last_mtime = st.st_mtime
            last_size = st.st_size
            last_heartbeat = now

            # Parse updated turns
            try:
                data = parse_transcript_turns(conv_dir=conv_dir, max_turns=30)
            except Exception:
                continue

            is_running = data.get("is_running", False)
            turns = data.get("turns", [])
            if not turns:
                continue

            latest_turn = turns[-1]
            turn_idx = latest_turn.get("turn_index", 0)
            turn_id = latest_turn.get("turn_id") or latest_turn.get("step_index") or turn_idx
            tools = latest_turn.get("tool_calls", [])
            agent_resp = latest_turn.get("agent_response", "")
            thinking = latest_turn.get("thinking", "")

            # 1. Turn Completion Transition (either was_running or new completed turn arrived)
            if (was_running or last_notified_turn != turn_id) and not is_running and agent_resp:
                done_payload = {
                    "event": "done",
                    "turn_index": turn_idx,
                    "turn_id": turn_id,
                    "is_running": False,
                    "total_turns": data.get("total_turns", len(turns)),
                    "agent_response": agent_resp,
                    "thinking": thinking,
                    "tool_calls": tools,
                    "timestamp": latest_turn.get("timestamp", ""),
                    "turns": turns
                }
                if not self._send_sse("done", done_payload):
                    break
                was_running = False

                # Automated Web Push Notification
                if last_notified_turn != turn_id:
                    last_notified_turn = turn_id
                    try:
                        presence = get_presence_state()
                        if not presence.get("is_active"):
                            from tools.webpush_service import send_push_notification
                            clean_snippet = re.sub(r'```[\s\S]*?```', '[Code Block]', agent_resp)
                            clean_snippet = re.sub(r'[#*_`]', '', clean_snippet).strip()[:140]
                            send_push_notification(
                                title="N'kai: Turn Complete ⚡",
                                body=clean_snippet or "Agent completed response.",
                                url="/?board=chat",
                                tag=f"turn-{turn_id}",
                                channel="chat"
                            )
                    except Exception:
                        pass

            # 2. In-Progress Execution Update
            elif is_running:
                was_running = True
                last_tool = tools[-1] if tools else None
                progress_payload = {
                    "event": "progress",
                    "turn_index": turn_idx,
                    "turn_id": turn_id,
                    "is_running": True,
                    "tool_count": len(tools),
                    "last_tool": last_tool,
                    "thinking_snippet": (thinking[-200:] if thinking else ""),
                    "has_thinking": bool(thinking),
                    "turns": turns
                }
                if not self._send_sse("progress", progress_payload):
                    break

            # 3. Idle update (new turn started, user prompt queued, etc.)
            else:
                update_payload = {
                    "event": "update",
                    "turn_index": turn_idx,
                    "turn_id": turn_id,
                    "is_running": is_running,
                    "total_turns": data.get("total_turns", len(turns)),
                    "turns": turns
                }
                if not self._send_sse("update", update_payload):
                    break

    def log_message(self, format, *args):
        """Concise server logging for developer terminals."""
        sys.stderr.write(f"  [N'KAI HTTP] {self.address_string()} - {format % args}\n")


class ThreadedTCPServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    allow_reuse_address = (sys.platform != "win32")
    daemon_threads = True

    def get_request(self):
        conn, addr = super().get_request()
        try:
            conn.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        except Exception:
            pass
        return conn, addr


def find_available_port(host: str, starting_port: int, max_attempts: int = 50) -> int:
    """Finds the next available local port starting from starting_port."""
    for port in range(starting_port, starting_port + max_attempts):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            if sys.platform != "win32":
                sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            try:
                sock.bind((host, port))
                return port
            except OSError:
                continue
    raise RuntimeError(f"Could not find an open port between {starting_port} and {starting_port + max_attempts}")


def start_canon_watcher():
    """
    Zero-touch background daemon thread watching AGENDA.md, ToadTracker BACKLOG.md,
    TheKlangSuite docs/BACKLOG.md, and morning_mail_topics.md.
    Debounces file updates by 3.0 seconds, then triggers tools/sync_canon.py.
    Explicitly ignores CANON.md and .agents/canon.json to prevent recursive write loops.
    """
    watched_paths = [
        Path(r"c:\_\Vault\AGENDA.md"),
        Path(r"C:\Dev\ToadTracker\BACKLOG.md"),
        Path(r"C:\Dev\TheKlangSuite\docs\BACKLOG.md"),
        REPO_ROOT / ".agents" / "sidecar" / "morning_mail_topics.md",
        Path(r"C:\Dev\nkai\.agents\sidecar\morning_mail_topics.md"),
        Path(r"c:\_\Vault\tools\sync_canon.py"),
    ]

    sync_script = Path(r"c:\_\Vault\tools\sync_canon.py")

    def _watcher_loop():
        last_mtimes = {}
        for p in watched_paths:
            try:
                if p.exists():
                    last_mtimes[str(p)] = p.stat().st_mtime
            except OSError:
                pass

        dirty = False
        last_change_time = 0.0

        while True:
            try:
                time.sleep(1.0)
                now = time.time()
                changed = False

                for p in watched_paths:
                    p_str = str(p)
                    try:
                        if p.exists():
                            mtime = p.stat().st_mtime
                            prev = last_mtimes.get(p_str)
                            if prev is None or mtime > prev:
                                last_mtimes[p_str] = mtime
                                changed = True
                    except OSError:
                        continue

                if changed:
                    dirty = True
                    last_change_time = now

                # 3.0-second quiet debounce window
                if dirty and (now - last_change_time >= 3.0):
                    dirty = False
                    if sync_script.exists():
                        try:
                            res = subprocess.run(
                                [sys.executable, str(sync_script)],
                                capture_output=True,
                                text=True,
                                encoding="utf-8",
                                timeout=15,
                            )
                            if res.returncode == 0:
                                sys.stderr.write("  [CANON WATCHER] ✅ Auto-synchronized CANON.md & .agents/canon.json\n")
                            else:
                                sys.stderr.write(f"  [CANON WATCHER] ⚠️ Sync error: {res.stderr.strip()}\n")
                        except Exception as e:
                            sys.stderr.write(f"  [CANON WATCHER] ⚠️ Execution failed: {e}\n")
            except Exception:
                time.sleep(2.0)

    watcher_thread = threading.Thread(target=_watcher_loop, daemon=True, name="CanonWatcherThread")
    watcher_thread.start()
    sys.stderr.write("  [CANON WATCHER] 🛰️ Background file watcher active (debounced: 3.0s)\n")
    return watcher_thread


def main():
    parser = argparse.ArgumentParser(
        description="N'kai Zero-Dependency CLI Server (Topic 6.13 / Section 14)",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python tools/nkai_serve.py .agents/sidecar/morning_mail_inbox.html
  python tools/nkai_serve.py --lan
  python tools/nkai_serve.py --port 3030 --no-browser --qr
  python tools/nkai_serve.py docs/history/research/ --host 0.0.0.0
""",
    )
    parser.add_argument(
        "path",
        nargs="?",
        default=".",
        help="Path to an HTML sidecar file or directory to serve (default: current directory)",
    )
    parser.add_argument(
        "--port",
        type=int,
        default=3030,
        help="Preferred port number to bind (default: 3030 with automatic fallback)",
    )
    parser.add_argument(
        "--host",
        type=str,
        default="0.0.0.0",
        help="Host address to bind to (default: 0.0.0.0 across all network interfaces)",
    )
    parser.add_argument(
        "--lan",
        action="store_true",
        help="Enable local Wi-Fi / LAN serving (binds to 0.0.0.0 and displays mobile companion QR code)",
    )
    parser.add_argument(
        "--qr",
        action="store_true",
        help="Display terminal ASCII / Unicode QR code for mobile phone pairing",
    )
    parser.add_argument(
        "--no-browser",
        action="store_true",
        help="Suppress automatic system browser launch",
    )

    args = parser.parse_args()

    # When --lan is requested and host is default, bind to all interfaces
    if args.lan and args.host == "127.0.0.1":
        args.host = "0.0.0.0"

    show_qr = args.lan or args.qr or (args.host == "0.0.0.0")

    target_path = Path(args.path).resolve()
    if not target_path.exists():
        print(f"Error: Target path does not exist: {target_path}", file=sys.stderr)
        sys.exit(1)

    if target_path.is_file():
        directory_to_serve = target_path.parent
        file_name = target_path.name
    else:
        directory_to_serve = target_path
        # Auto-detect canonical inbox if serving directory root
        if (directory_to_serve / "morning_mail_inbox.html").exists():
            file_name = "morning_mail_inbox.html"
        elif (directory_to_serve / ".agents" / "sidecar" / "morning_mail_inbox.html").exists():
            file_name = ".agents/sidecar/morning_mail_inbox.html"
        elif (directory_to_serve / "sandbox_mail_inbox.html").exists():
            file_name = "sandbox_mail_inbox.html"
        else:
            file_name = ""

    # Port resolution
    try:
        active_port = find_available_port(args.host, args.port)
    except Exception as e:
        print(f"Error resolving port: {e}", file=sys.stderr)
        sys.exit(1)

    local_ip = get_local_ip()
    tailscale_ip = get_tailscale_ip()
    host_display = "127.0.0.1" if args.host == "0.0.0.0" else args.host
    url = f"http://{host_display}:{active_port}/{file_name}".rstrip("/")
    mobile_url = f"http://{local_ip}:{active_port}/{file_name}".rstrip("/")
    tailscale_url = f"http://{tailscale_ip}:{active_port}/{file_name}".rstrip("/") if tailscale_ip else None
    sentry_token = get_or_create_sentry_token()
    mobile_url_with_auth = f"{mobile_url}?auth={sentry_token}"
    tailscale_url_with_auth = f"{tailscale_url}?auth={sentry_token}" if tailscale_url else None
    qr_web_url = f"http://{local_ip}:{active_port}/qr?auth={sentry_token}"

    print("=" * 64)
    print("  📱 N'KAI MOBILE COMPANION READY!")
    print("  Universal AI Harness Portability & Touch Ergonomics (v0.3.0)")
    print("=" * 64)
    print(f"  📂 Serving Directory: {directory_to_serve}")
    print(f"  🌐 Local Host URL:    {url}")
    print(f"  📶 Local Wi-Fi URL:   {mobile_url}")
    if tailscale_url:
        print(f"  🔒 Tailscale Mesh:    {tailscale_url}")
    print(f"  📱 Web Pairing Route: {qr_web_url}")
    print(f"  🛡️ Sentry Auth Token: {sentry_token[:8]}...{sentry_token[-8:]}")
    if active_port != args.port:
        print(f"  ⚡ Note: Port {args.port} was occupied, bound to {active_port}")
    if args.host == "0.0.0.0":
        print(f"  🛡️ Bound to 0.0.0.0 (Accessible across local Wi-Fi & Tailnet)")
    print("=" * 64)

    if show_qr:
        print("\n  📱 SCAN TO PAIR WITH MOBILE COMPANION:\n")
        try:
            print(generate_qr_terminal(mobile_url_with_auth, border=2))
        except Exception as e:
            print(f"  (Could not render terminal QR: {e})")
        print("\n" + "=" * 64)

    print("  ⌨️  Press Ctrl+C to terminate server\n")

    handler = lambda *h_args, **h_kwargs: NkaiHTTPRequestHandler(
        *h_args, directory=str(directory_to_serve), **h_kwargs
    )

    pid_file = REPO_ROOT / ".agents" / "pipeline" / "nkai_daemon.pid"
    try:
        pid_file.parent.mkdir(parents=True, exist_ok=True)
        pid_file.write_text(str(os.getpid()), encoding="utf-8")
        set_thread_execution_state(True)
        start_canon_watcher()
        with ThreadedTCPServer((args.host, active_port), handler) as httpd:
            # Attach mobile url, auth token, and API key to server instance
            httpd.sentry_token = sentry_token
            httpd.gemini_api_key = get_gemini_api_key()
            httpd.allow_unauthenticated_localhost = True
            httpd.mobile_url = mobile_url_with_auth
            httpd.local_ip = local_ip
            httpd.tailscale_ip = tailscale_ip

            if not args.no_browser:
                print(f"  🌐 Opening browser to {url} ...")
                webbrowser.open(url)

            httpd.serve_forever()
    except KeyboardInterrupt:
        print("\n[N'KAI CLI] Server stopped by user. Goodbye!")
        sys.exit(0)
    except Exception as e:
        print(f"\n[N'KAI CLI] Server error: {e}", file=sys.stderr)
        sys.exit(1)
    finally:
        set_thread_execution_state(False)
        try:
            if pid_file.exists() and pid_file.read_text(encoding="utf-8").strip() == str(os.getpid()):
                pid_file.unlink(missing_ok=True)
        except Exception:
            pass


if __name__ == "__main__":
    main()
