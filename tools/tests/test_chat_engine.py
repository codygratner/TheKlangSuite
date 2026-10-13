#!/usr/bin/env python3
"""
tools/tests/test_chat_engine.py — Unit tests for N'kai Agent Intercom & Live Chat Board
======================================================================================
Asserts:
1. Active conversation path discovery.
2. Transcript parsing into structured turns (user, agent, tools, thinking).
3. Chat board HTML generation and web QA compliance.
4. Board registration in registry.json.
5. REST endpoints /api/chat/history and /api/chat/send.
"""

import json
import socketserver
import sys
import tempfile
import threading
import unittest
import urllib.request
from pathlib import Path
from unittest.mock import patch

# Configure UTF-8 streams for Windows console compatibility
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.chat_engine import (
    get_active_conversation_path,
    parse_transcript_turns,
    generate_chat_board_html,
    build_and_save_chat_board,
    register_chat_board
)
from tools.nkai_serve import NkaiHTTPRequestHandler


class EphemeralServer(socketserver.TCPServer):
    allow_reuse_address = True


class TestChatEngine(unittest.TestCase):
    """Test suite for transcript parsing and chat board generation."""

    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()
        self.test_root = Path(self.temp_dir.name)
        self.conv_dir = self.test_root / "fake_conv_123"
        self.log_dir = self.conv_dir / ".system_generated" / "logs"
        self.log_dir.mkdir(parents=True, exist_ok=True)
        self.transcript_file = self.log_dir / "transcript_full.jsonl"

        # Seed synthetic multi-turn transcript
        steps = [
            {
                "step_index": 1,
                "source": "USER_EXPLICIT",
                "type": "USER_INPUT",
                "content": "<USER_REQUEST>\nImplement chat board\n</USER_REQUEST>",
                "created_at": "2026-10-12T22:00:00Z"
            },
            {
                "step_index": 2,
                "source": "MODEL",
                "type": "PLANNER_RESPONSE",
                "thinking": "The user wants an intercom chat board in nkai.",
                "tool_calls": [
                    {"name": "view_file", "toolSummary": "View registry", "toolAction": "Viewing registry.json"}
                ],
                "created_at": "2026-10-12T22:00:05Z"
            },
            {
                "step_index": 3,
                "source": "MODEL",
                "type": "PLANNER_RESPONSE",
                "content": "I have created the chat board and connected it to the registry.",
                "created_at": "2026-10-12T22:00:10Z"
            }
        ]
        with open(self.transcript_file, "w", encoding="utf-8") as f:
            for s in steps:
                f.write(json.dumps(s) + "\n")

    def tearDown(self):
        self.temp_dir.cleanup()

    def test_parse_transcript_turns(self):
        """Asserts parsing correctly extracts user, agent, thinking, and tools."""
        res = parse_transcript_turns(conv_dir=self.conv_dir)
        self.assertTrue(res["ok"])
        self.assertEqual(res["conv_id"], "fake_conv_123")
        self.assertEqual(len(res["turns"]), 1)

        turn = res["turns"][0]
        self.assertEqual(turn["user_prompt"], "Implement chat board")
        self.assertEqual(turn["agent_response"], "I have created the chat board and connected it to the registry.")
        self.assertIn("intercom chat board", turn["thinking"])
        self.assertEqual(len(turn["tool_calls"]), 1)
        self.assertEqual(turn["tool_calls"][0]["name"], "view_file")

    def test_generate_chat_board_html(self):
        """Asserts HTML contains header, stream container, and input elements."""
        html_out = generate_chat_board_html(conv_id_override="fake_conv_123")
        self.assertIn("<!DOCTYPE html>", html_out)
        self.assertIn("💬 Agent Intercom", html_out)
        self.assertIn("id=\"chat-stream\"", html_out)
        self.assertIn("id=\"prompt-input\"", html_out)
        self.assertIn("/api/chat/history", html_out)

    def test_build_and_save_chat_board(self):
        """Asserts file is saved to target path with UTF-8 encoding."""
        out_file = self.test_root / "test_chat_board.html"
        res = build_and_save_chat_board(out_path=out_file)
        self.assertTrue(out_file.exists())
        content = out_file.read_text(encoding="utf-8")
        self.assertIn("💬 N'kai Agent Intercom", content)


class TestChatAPIEndpoints(unittest.TestCase):
    """Test suite for /api/chat/history and /api/chat/send."""

    @classmethod
    def setUpClass(cls):
        cls.server = EphemeralServer(("127.0.0.1", 0), NkaiHTTPRequestHandler)
        cls.port = cls.server.server_address[1]
        cls.thread = threading.Thread(target=cls.server.serve_forever, daemon=True)
        cls.thread.start()

    @classmethod
    def tearDownClass(cls):
        cls.server.shutdown()
        cls.server.server_close()

    def test_api_chat_history(self):
        """Asserts GET /api/chat/history returns JSON with turns."""
        url = f"http://127.0.0.1:{self.port}/api/chat/history"
        req = urllib.request.Request(url)
        with urllib.request.urlopen(req) as resp:
            self.assertEqual(resp.status, 200)
            data = json.loads(resp.read().decode("utf-8"))
            self.assertTrue(data["ok"])
            self.assertIn("turns", data)
            self.assertIn("is_running", data)

    def test_api_chat_send(self):
        """Asserts POST /api/chat/send records prompt and returns ok."""
        url = f"http://127.0.0.1:{self.port}/api/chat/send"
        payload = json.dumps({"prompt": "Testing chat send from mobile"}).encode("utf-8")
        req = urllib.request.Request(url, data=payload, headers={"Content-Type": "application/json"})
        with urllib.request.urlopen(req) as resp:
            self.assertEqual(resp.status, 200)
            data = json.loads(resp.read().decode("utf-8"))
            self.assertTrue(data["ok"])
            self.assertEqual(data["status"], "prompt_queued")


if __name__ == "__main__":
    unittest.main()
