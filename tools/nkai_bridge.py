#!/usr/bin/env python3
"""
N'kai Micro-Bridge Server (v1.1.0)
Robust, zero-dependency local loopback IPC server bridging N'kai HTML sidecars
directly into the AI coding agent harness.

Listens on http://127.0.0.1:4040
Includes Private Network Access (PNA) and CORS headers for Chromium/Electron webviews.
Appends received actions to .agents/pipeline/inbox/action_queue.jsonl
"""

import sys
import os
import json
import time
import uuid
import socketserver
from http.server import HTTPServer, BaseHTTPRequestHandler

# Ensure UTF-8 output on Windows consoles
if sys.stdout.encoding != 'utf-8':
    try:
        sys.stdout.reconfigure(encoding='utf-8', errors='replace')
        sys.stderr.reconfigure(encoding='utf-8', errors='replace')
    except Exception:
        pass

DEFAULT_PORT = 4040
WORKSPACE_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
ACTION_QUEUE_DIR = os.path.join(WORKSPACE_ROOT, ".agents", "pipeline", "inbox")
ACTION_QUEUE_FILE = os.path.join(ACTION_QUEUE_DIR, "action_queue.jsonl")
LATEST_ACTION_FILE = os.path.join(ACTION_QUEUE_DIR, "latest_action.json")

class ReusableThreadingServer(socketserver.ThreadingTCPServer):
    allow_reuse_address = True
    daemon_threads = True

class NkaiBridgeHandler(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"

    def _send_cors_headers(self):
        origin = self.headers.get("Origin", "*")
        if not origin or origin == "null":
            origin = "*"
        self.send_header("Access-Control-Allow-Origin", origin)
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        req_headers = self.headers.get("Access-Control-Request-Headers", "")
        allowed_headers = "Content-Type, Authorization, X-Requested-With, Access-Control-Request-Private-Network"
        if req_headers:
            allowed_headers += f", {req_headers}"
        self.send_header("Access-Control-Allow-Headers", allowed_headers)
        self.send_header("Access-Control-Allow-Private-Network", "true")
        self.send_header("Access-Control-Max-Age", "86400")
        self.send_header("Connection", "close")
        self.close_connection = True

    def do_OPTIONS(self):
        self.send_response(204)
        self._send_cors_headers()
        self.send_header("Content-Length", "0")
        self.end_headers()

    def do_GET(self):
        if self.path == "/api/status" or self.path == "/":
            resp = {
                "status": "online",
                "bridge": "nkai-micro-bridge-v1.1",
                "port": self.server.server_address[1],
                "time": time.time(),
                "queue_file": ACTION_QUEUE_FILE
            }
            body = json.dumps(resp, indent=2).encode("utf-8")
            self.send_response(200)
            self._send_cors_headers()
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
        elif self.path == "/api/actions":
            actions = []
            if os.path.exists(ACTION_QUEUE_FILE):
                with open(ACTION_QUEUE_FILE, "r", encoding="utf-8") as f:
                    for line in f:
                        line = line.strip()
                        if line:
                            try:
                                actions.append(json.loads(line))
                            except Exception:
                                pass
            body = json.dumps({"actions": actions[-50:]}).encode("utf-8")
            self.send_response(200)
            self._send_cors_headers()
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
        else:
            self.send_response(404)
            self._send_cors_headers()
            self.send_header("Content-Length", "0")
            self.end_headers()

    def do_POST(self):
        if self.path == "/api/action":
            content_length = int(self.headers.get("Content-Length", 0))
            body = self.rfile.read(content_length)
            try:
                data = json.loads(body.decode("utf-8"))
            except Exception as e:
                err_body = json.dumps({"error": f"Invalid JSON: {e}"}).encode("utf-8")
                self.send_response(400)
                self._send_cors_headers()
                self.send_header("Content-Type", "application/json")
                self.send_header("Content-Length", str(len(err_body)))
                self.end_headers()
                self.wfile.write(err_body)
                return

            action_entry = {
                "id": str(uuid.uuid4())[:8],
                "timestamp": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
                "action": data.get("action", "unknown"),
                "topicId": data.get("topicId"),
                "verdict": data.get("verdict"),
                "prompt": data.get("prompt"),
                "source": data.get("source", "nkai-sidecar"),
                "payload": data
            }

            os.makedirs(ACTION_QUEUE_DIR, exist_ok=True)
            with open(ACTION_QUEUE_FILE, "a", encoding="utf-8") as f:
                f.write(json.dumps(action_entry) + "\n")

            with open(LATEST_ACTION_FILE, "w", encoding="utf-8") as f:
                json.dump(action_entry, f, indent=2)

            print(f"[Nkai Bridge] [ACTION] Dispatched: {action_entry['action']} (Topic: {action_entry['topicId']}, Verdict: {action_entry['verdict']})", flush=True)

            resp_data = json.dumps({
                "success": True,
                "message": "Action received and dispatched to agent queue",
                "action": action_entry
            }).encode("utf-8")
            self.send_response(200)
            self._send_cors_headers()
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(resp_data)))
            self.end_headers()
            self.wfile.write(resp_data)
        else:
            self.send_response(404)
            self._send_cors_headers()
            self.send_header("Content-Length", "0")
            self.end_headers()


    def log_message(self, format, *args):
        # Concise logging
        sys.stderr.write(f"[Nkai Bridge {time.strftime('%H:%M:%S')}] {args[0]} {args[1]}\n")

def run(port=DEFAULT_PORT):
    server_address = ("127.0.0.1", port)
    try:
        httpd = ReusableThreadingServer(server_address, NkaiBridgeHandler)
    except OSError:
        port = port + 1
        server_address = ("127.0.0.1", port)
        httpd = ReusableThreadingServer(server_address, NkaiBridgeHandler)

    print(f"[Nkai Bridge] [ONLINE] Micro-Bridge v1.1 running on http://127.0.0.1:{port}", flush=True)
    print(f"[Nkai Bridge] Action queue target: {ACTION_QUEUE_FILE}", flush=True)
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\n[Nkai Bridge] Shutting down.", flush=True)
        httpd.server_close()

if __name__ == "__main__":
    port = DEFAULT_PORT
    if len(sys.argv) > 1:
        try:
            port = int(sys.argv[1])
        except ValueError:
            pass
    run(port)
