#!/usr/bin/env python3
"""
N'kai Zero-Dependency CLI Server (nkai_serve.py)
Part of N'kai Universal AI Harness Portability (Topic 6.13).

Provides instant HTTP serving for N'kai sidecars, research boards, and interactive
dialog labs across terminal-first harnesses (Claude Code CLI, Aider) and IDEs.
Guarantees 100% standard library compliance, automatic port recovery, RFC MIME mapping,
and optional system browser launching.
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


class NkaiHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    """Specialized HTTP handler with audio/web MIME types, CORS, and CLI dispatch loopback."""

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
        description="N'kai Zero-Dependency CLI Server (Topic 6.13)",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python tools/nkai_serve.py .agents/sidecar/visual_grill_me.html
  python tools/nkai_serve.py --port 3030 --no-browser
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
        "--no-browser",
        action="store_true",
        help="Suppress automatic system browser launch",
    )

    args = parser.parse_args()

    target_path = Path(args.path).resolve()
    if not target_path.exists():
        print(f"Error: Target path does not exist: {target_path}", file=sys.stderr)
        sys.exit(1)

    if target_path.is_file():
        directory_to_serve = target_path.parent
        file_name = target_path.name
    else:
        directory_to_serve = target_path
        file_name = ""

    # Port resolution
    try:
        active_port = find_available_port(args.host, args.port)
    except Exception as e:
        print(f"Error resolving port: {e}", file=sys.stderr)
        sys.exit(1)

    url = f"http://{args.host}:{active_port}/{file_name}".rstrip("/")

    print("=" * 60)
    print("  🚀 N'KAI UNIVERSAL CLI SERVER (v1.0.0)")
    print("  Universal AI Harness Portability (Topic 6.13)")
    print("=" * 60)
    print(f"  📂 Serving Directory: {directory_to_serve}")
    print(f"  🌐 Local URL:         {url}")
    if active_port != args.port:
        print(f"  ⚡ Note: Port {args.port} was occupied, bound to {active_port}")
    print(f"  🛡️ MIME Types:        .html, .js, .css, .json, .wav, .svg registered")
    print(f"  ⌨️  Press Ctrl+C to terminate server")
    print("=" * 60 + "\n")

    handler = lambda *h_args, **h_kwargs: NkaiHTTPRequestHandler(
        *h_args, directory=str(directory_to_serve), **h_kwargs
    )

    try:
        with ThreadedTCPServer((args.host, active_port), handler) as httpd:
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
