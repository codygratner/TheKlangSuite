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
import json
import mimetypes
import os
import re
import shutil
import socket
import socketserver
import struct
import subprocess
import sys
import time
from typing import Optional
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
    from tools.sentry_engine import stream_sentry_response
except ImportError:
    try:
        from sentry_auth import get_or_create_sentry_token, verify_sentry_token
        from sentry_engine import stream_sentry_response
    except ImportError:
        _tools_dir = Path(__file__).resolve().parent
        if str(_tools_dir) not in sys.path:
            sys.path.insert(0, str(_tools_dir))
        from sentry_auth import get_or_create_sentry_token, verify_sentry_token
        from sentry_engine import stream_sentry_response

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

    DEFAULT_VAULT_PATH = Path(r"C:\_\Vault\Inbox\SCRATCHPAD.md")
    FALLBACK_PATH = REPO_ROOT / "docs" / "inbox" / "SCRATCHPAD.md"

    @classmethod
    def get_path(cls) -> Path:
        if cls.DEFAULT_VAULT_PATH.exists():
            return cls.DEFAULT_VAULT_PATH
        if cls.DEFAULT_VAULT_PATH.parent.exists():
            return cls.DEFAULT_VAULT_PATH
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
        """Appends permissive development CORS headers."""
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type, Authorization")
        self.send_header("Cache-Control", "no-cache, no-store, must-revalidate")
        super().end_headers()

    def do_OPTIONS(self):
        """Handles pre-flight CORS checks."""
        self.send_response(204)
        self.end_headers()

    def is_authorized(self) -> bool:
        """Validates Bearer token from header, query param (?auth=), or localhost."""
        sentry_token = getattr(self.server, "sentry_token", None)
        if not sentry_token:
            return True

        # Check Authorization header (Bearer <token>)
        auth_header = self.headers.get("Authorization")
        if auth_header and verify_sentry_token(auth_header, sentry_token):
            return True

        # Check query param ?auth=<token>
        try:
            qs = parse_qs(urlparse(self.path).query)
            if "auth" in qs and qs["auth"]:
                import hmac
                if hmac.compare_digest(qs["auth"][0].strip(), sentry_token.strip()):
                    return True
        except Exception:
            pass

        # Allow localhost / loopback for local dev and automated tests
        allow_local = getattr(self.server, "allow_unauthenticated_localhost", True)
        client_ip = self.client_address[0]
        if allow_local and client_ip in ("127.0.0.1", "::1", "localhost"):
            return True

        return False

    def translate_path(self, path):
        """Translates virtual URL routes and sidecar aliases to physical files."""
        parsed = urlparse(path)
        clean_path = parsed.path
        query = parse_qs(parsed.query)

        # Handle ?board=<id> parameter on root or default to morning_mail
        if clean_path in ("", "/"):
            board_id = query.get("board", [None])[0]
            if board_id:
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
            clean_path = "/morning_mail_inbox.html"

        filename = clean_path.lstrip("/")

        # Check known sidecar locations dynamically
        sidecar_candidates = [
            REPO_ROOT / ".agents" / "sidecar" / filename,
            Path(r"c:\Dev\nkai\sidecars\panel") / filename,
            Path(r"c:\Dev\nkai\plugin\sidecars\panel") / filename,
            Path(r"c:\Dev\TheKlangSuite") / filename,
        ]
        for cand in sidecar_candidates:
            if cand.is_file():
                return str(cand)

        return super().translate_path(path)

    def do_GET(self):
        """Serves mobile QR code vector graphics and pairing page on /qr and /mobile."""
        clean_path = self.path.split("?")[0].rstrip("/")
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

        if clean_path == "/api/status":
            ts_ip = getattr(self.server, "tailscale_ip", None) or get_tailscale_ip() or get_local_ip()
            status_data = {
                "status": "online",
                "service": "nkai_mobile_companion",
                "version": "0.3.0",
                "tailscale_ip": ts_ip,
                "port": self.server.server_address[1],
                "active_boards": ["morning_mail", "runway", "sandbox_mail"]
            }
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.end_headers()
            self.wfile.write(json.dumps(status_data, indent=2).encode("utf-8"))
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
            from urllib.parse import parse_qs, urlparse
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
            if host.startswith("100."):
                mode = "remote"

            icon_data = generate_touch_icon(mode)
            self.send_response(200)
            self.send_header("Content-Type", "image/png")
            self.send_header("Content-Length", str(len(icon_data)))
            self.send_header("Cache-Control", "public, max-age=86400")
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
            if host.startswith("100."):
                mode = "remote"

            app_title = "N'kai (Remote)" if mode == "remote" else "N'kai (Home)"
            manifest = {
                "name": f"N'kai Triage Companion - {app_title}",
                "short_name": app_title,
                "start_url": f"/?mode={mode}",
                "display": "standalone",
                "background_color": "#090d13",
                "theme_color": "#090d13",
                "icons": [
                    {
                        "src": f"/apple-touch-icon.png?mode={mode}",
                        "sizes": "180x180",
                        "type": "image/png",
                    }
                ],
            }
            manifest_bytes = json.dumps(manifest, indent=2).encode("utf-8")
            self.send_response(200)
            self.send_header("Content-Type", "application/manifest+json; charset=utf-8")
            self.send_header("Content-Length", str(len(manifest_bytes)))
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

        # Sentry SSE Streaming Route
        if clean_path == "/api/sentry/stream":
            prompt = body_json.get("prompt", "").strip()
            attachments = body_json.get("attachments", [])
            api_key = getattr(self.server, "gemini_api_key", None)

            self.send_response(200)
            self.send_header("Content-Type", "text/event-stream")
            self.send_header("Cache-Control", "no-cache, no-transform")
            self.send_header("Connection", "keep-alive")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()

            import asyncio
            loop = asyncio.new_event_loop()
            try:
                gen = stream_sentry_response(prompt, attachments, api_key=api_key)
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
                    target_fn = f"{now_prefix}_{safe_fn}"
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
                target_fn = f"{now_prefix}_{safe_fn}"

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

        if clean_path == "/api/chat/send":
            prompt = body_json.get("prompt", "").strip()
            if not prompt:
                self.send_response(400)
                self.send_header("Content-Type", "application/json")
                self.end_headers()
                self.wfile.write(json.dumps({"ok": False, "error": "Empty prompt"}).encode("utf-8"))
                return

            now_iso = datetime.now().isoformat()
            client_ip = self.client_address[0] if hasattr(self, "client_address") else "127.0.0.1"

            # Log to action_history.jsonl
            action_log = REPO_ROOT / ".agents" / "pipeline" / "mailbag" / "action_history.jsonl"
            action_log.parent.mkdir(parents=True, exist_ok=True)
            with open(action_log, "a", encoding="utf-8") as f:
                f.write(json.dumps({
                    "timestamp": now_iso,
                    "action": "PROMPT_DISPATCH",
                    "prompt": prompt,
                    "source": "chat_board",
                    "client_ip": client_ip
                }) + "\n")

            # Update agent_bulletin.json
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

            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.end_headers()
            self.wfile.write(json.dumps({"ok": True, "status": "prompt_queued", "prompt": prompt}).encode("utf-8"))
            return

        if clean_path == "/api/scratchpad/append":
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

        self.send_response(404)
        self.end_headers()

    def log_message(self, format, *args):
        """Concise server logging for developer terminals."""
        sys.stderr.write(f"  [N'KAI HTTP] {self.address_string()} - {format % args}\n")


class ThreadedTCPServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    allow_reuse_address = True
    daemon_threads = True


def find_available_port(host: str, starting_port: int, max_attempts: int = 50) -> int:
    """Finds the next available local port starting from starting_port."""
    for port in range(starting_port, starting_port + max_attempts):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            try:
                sock.bind((host, port))
                return port
            except OSError:
                continue
    raise RuntimeError(f"Could not find an open port between {starting_port} and {starting_port + max_attempts}")


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

    try:
        set_thread_execution_state(True)
        with ThreadedTCPServer((args.host, active_port), handler) as httpd:
            # Attach mobile url and auth token to server instance
            httpd.sentry_token = sentry_token
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


if __name__ == "__main__":
    main()
