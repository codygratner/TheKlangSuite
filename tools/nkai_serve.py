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
import http.server
from datetime import datetime
import json
import mimetypes
import os
import socket
import socketserver
import struct
import subprocess
import sys
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
    """Detects local LAN / Wi-Fi IP address via dummy UDP connection."""
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
            s.connect(("8.8.8.8", 80))
            return s.getsockname()[0]
    except Exception:
        try:
            return socket.gethostbyname(socket.gethostname())
        except Exception:
            return "127.0.0.1"


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

        if clean_path == "/api/quick-note":
            text = body_json.get("text", "").strip()
            if not text:
                self.send_response(400)
                self.send_header("Content-Type", "application/json")
                self.end_headers()
                self.wfile.write(json.dumps({"ok": False, "error": "Empty note text"}).encode("utf-8"))
                return

            now = datetime.now()
            date_str = now.strftime("%Y-%m-%d")
            time_str = now.strftime("%H:%M")
            inbox_dir = Path(r"C:\_\Vault\Inbox")
            if not inbox_dir.exists():
                inbox_dir = REPO_ROOT / "docs" / "inbox"
            inbox_dir.mkdir(parents=True, exist_ok=True)

            catchall_path = inbox_dir / f"{date_str}_Quick_Notes.md"
            if not catchall_path.exists():
                header = f'---\ntitle: "Daily Quick Notes ({date_str})"\ndate: {date_str}\ntags: [quick-notes, daily-catchall]\npriority: P2\npillars: []\n---\n\n# Daily Quick Notes ({date_str})\n\n'
                catchall_path.write_text(header, encoding="utf-8")

            with open(catchall_path, "a", encoding="utf-8") as f:
                f.write(f"- [{time_str}] {text}\n")

            check_mail_script = REPO_ROOT / "tools" / "check_mail.py"
            if check_mail_script.exists():
                subprocess.run([sys.executable, str(check_mail_script), "--no-browser"], cwd=REPO_ROOT, check=False)

            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.end_headers()
            self.wfile.write(json.dumps({"ok": True, "file": str(catchall_path)}).encode("utf-8"))
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

        if clean_path in ("/api/action", "/api/dispatch"):
            print(f"\n[N'KAI MOBILE ACTION] Received from mobile UI: {body_json}")
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.end_headers()
            self.wfile.write(json.dumps({"ok": True, "status": "action_recorded"}).encode("utf-8"))
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
        default="127.0.0.1",
        help="Host address to bind to (default: 127.0.0.1)",
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
    qr_web_url = f"http://{local_ip}:{active_port}/qr"

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
    if active_port != args.port:
        print(f"  ⚡ Note: Port {args.port} was occupied, bound to {active_port}")
    if args.host == "0.0.0.0":
        print(f"  🛡️ Bound to 0.0.0.0 (Accessible across local Wi-Fi & Tailnet)")
    print("=" * 64)

    if show_qr:
        print("\n  📱 SCAN TO PAIR WITH MOBILE COMPANION:\n")
        try:
            print(generate_qr_terminal(mobile_url, border=2))
        except Exception as e:
            print(f"  (Could not render terminal QR: {e})")
        print("\n" + "=" * 64)

    print("  ⌨️  Press Ctrl+C to terminate server\n")

    handler = lambda *h_args, **h_kwargs: NkaiHTTPRequestHandler(
        *h_args, directory=str(directory_to_serve), **h_kwargs
    )

    try:
        with ThreadedTCPServer((args.host, active_port), handler) as httpd:
            # Attach mobile url to server instance for /qr route
            httpd.mobile_url = mobile_url
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


if __name__ == "__main__":
    main()
