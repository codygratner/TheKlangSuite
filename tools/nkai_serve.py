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
import json
import mimetypes
import os
import socket
import socketserver
import sys
import webbrowser
from pathlib import Path
from urllib.parse import unquote

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

        super().do_GET()

    def do_POST(self):
        """Captures /api/dispatch actions from sidecars for terminal CLI harness pairing."""
        if self.path == "/api/dispatch":
            content_length = int(self.headers.get("Content-Length", 0))
            post_body = self.rfile.read(content_length)
            try:
                data = json.loads(post_body.decode("utf-8"))
                action = data.get("action", "")
                print(f"\n[N'KAI CLI DISPATCH] Action received from sidecar UI:")
                print(f"--------------------------------------------------")
                if isinstance(action, str):
                    print(action)
                else:
                    print(json.dumps(action, indent=2))
                print(f"--------------------------------------------------\n")
            except Exception as e:
                print(f"\n[N'KAI CLI DISPATCH] Raw payload: {post_body.decode('utf-8', errors='ignore')}")

            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.end_headers()
            response = json.dumps({"status": "ok", "source": "nkai_cli_server"}).encode("utf-8")
            self.wfile.write(response)
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
    host_display = "127.0.0.1" if args.host == "0.0.0.0" else args.host
    url = f"http://{host_display}:{active_port}/{file_name}".rstrip("/")
    mobile_url = f"http://{local_ip}:{active_port}/{file_name}".rstrip("/")
    qr_web_url = f"http://{local_ip}:{active_port}/qr"

    print("=" * 64)
    print("  📱 N'KAI MOBILE COMPANION READY!")
    print("  Universal AI Harness Portability & Touch Ergonomics (v0.3.0)")
    print("=" * 64)
    print(f"  📂 Serving Directory: {directory_to_serve}")
    print(f"  🌐 Local Host URL:    {url}")
    print(f"  📶 Local Wi-Fi URL:   {mobile_url}")
    print(f"  📱 Web Pairing Route: {qr_web_url}")
    if active_port != args.port:
        print(f"  ⚡ Note: Port {args.port} was occupied, bound to {active_port}")
    if args.host == "0.0.0.0":
        print(f"  🛡️ Bound to 0.0.0.0 (Accessible across local Wi-Fi LAN)")
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
