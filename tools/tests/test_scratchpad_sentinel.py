#!/usr/bin/env python3
"""
Unit tests for ScratchpadSentinel and scratchpad API endpoints in nkai_serve.py.
Part of Flight 21 (Zero-Token Scratchpad Sentinel & Ergonomics Suite).
"""

import json
import os
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch, MagicMock

# Configure UTF-8 streams for Windows console compatibility
if hasattr(sys.stdout, 'reconfigure'):
    sys.stdout.reconfigure(encoding='utf-8')
if hasattr(sys.stderr, 'reconfigure'):
    sys.stderr.reconfigure(encoding='utf-8')

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.nkai_serve import ScratchpadSentinel


class TestScratchpadSentinel(unittest.TestCase):
    """Verifies sub-0.1ms filesystem inspection and urgency categorization."""

    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()
        self.test_root = Path(self.temp_dir.name)
        self.sp_path = self.test_root / "SCRATCHPAD.md"

    def tearDown(self):
        self.temp_dir.cleanup()

    def test_missing_file_status(self):
        """Asserts missing file returns exists=False, lines=0, urgency=clean."""
        status = ScratchpadSentinel.inspect(custom_path=self.sp_path)
        self.assertFalse(status["exists"])
        self.assertEqual(status["lines"], 0)
        self.assertEqual(status["bytes"], 0)
        self.assertEqual(status["urgency"], "clean")
        self.assertFalse(status["has_unread"])

    def test_urgency_clean_level(self):
        """Asserts file with < 25 lines is categorized as 'clean'."""
        lines = ["# Title\n", "\n"] + [f"- Line {i}\n" for i in range(15)]
        self.sp_path.write_text("".join(lines), encoding="utf-8")

        status = ScratchpadSentinel.inspect(custom_path=self.sp_path)
        self.assertTrue(status["exists"])
        self.assertEqual(status["lines"], 17)
        self.assertGreater(status["bytes"], 0)
        self.assertEqual(status["urgency"], "clean")
        self.assertTrue(status["has_unread"])

    def test_urgency_growing_level(self):
        """Asserts file with 25-80 lines is categorized as 'growing'."""
        lines = [f"- Line {i}\n" for i in range(40)]
        self.sp_path.write_text("".join(lines), encoding="utf-8")

        status = ScratchpadSentinel.inspect(custom_path=self.sp_path)
        self.assertEqual(status["lines"], 40)
        self.assertEqual(status["urgency"], "growing")

    def test_urgency_urgent_level(self):
        """Asserts file with > 80 lines is categorized as 'urgent'."""
        lines = [f"- Line {i}\n" for i in range(95)]
        self.sp_path.write_text("".join(lines), encoding="utf-8")

        status = ScratchpadSentinel.inspect(custom_path=self.sp_path)
        self.assertEqual(status["lines"], 95)
        self.assertEqual(status["urgency"], "urgent")

    def test_safe_stat_retry_on_permission_error(self):
        """Asserts safe_stat recovers from transient Windows WinError 32 PermissionError."""
        self.sp_path.write_text("Test content", encoding="utf-8")

        call_count = 0

        def mock_stat(*args, **kwargs):
            nonlocal call_count
            call_count += 1
            if call_count == 1:
                raise PermissionError("[WinError 32] The process cannot access the file")
            return os.stat(self.sp_path)

        with patch.object(Path, "stat", side_effect=mock_stat):
            st = ScratchpadSentinel.safe_stat(self.sp_path, retries=3, delay=0.01)
            self.assertIsNotNone(st)
            self.assertEqual(call_count, 2)


import socketserver
import threading
import urllib.request
from tools.nkai_serve import NkaiHTTPRequestHandler


class EphemeralServer(socketserver.TCPServer):
    allow_reuse_address = True


class TestScratchpadAPI(unittest.TestCase):
    """Verifies REST endpoints for status, content, append, and quick-note redirection."""

    @classmethod
    def setUpClass(cls):
        cls.temp_dir = tempfile.TemporaryDirectory()
        cls.test_root = Path(cls.temp_dir.name)
        cls.sp_path = cls.test_root / "SCRATCHPAD.md"
        cls.sp_path.write_text("# Scratchpad\n- Initial line\n", encoding="utf-8")

        # Patch ScratchpadSentinel path
        cls.patcher = patch.object(ScratchpadSentinel, "get_path", return_value=cls.sp_path)
        cls.patcher.start()

        # Start ephemeral server
        cls.server = EphemeralServer(("127.0.0.1", 0), NkaiHTTPRequestHandler)
        cls.port = cls.server.server_address[1]
        cls.thread = threading.Thread(target=cls.server.serve_forever, daemon=True)
        cls.thread.start()

    @classmethod
    def tearDownClass(cls):
        cls.server.shutdown()
        cls.server.server_close()
        cls.patcher.stop()
        cls.temp_dir.cleanup()

    def setUp(self):
        self.sp_path.write_text("# Scratchpad\n- Initial line\n", encoding="utf-8")

    def test_api_status(self):
        url = f"http://127.0.0.1:{self.port}/api/scratchpad/status"
        req = urllib.request.Request(url)
        with urllib.request.urlopen(req) as resp:
            self.assertEqual(resp.status, 200)
            data = json.loads(resp.read().decode("utf-8"))
            self.assertTrue(data["exists"])
            self.assertEqual(data["lines"], 2)
            self.assertEqual(data["urgency"], "clean")

    def test_api_content(self):
        url = f"http://127.0.0.1:{self.port}/api/scratchpad/content"
        req = urllib.request.Request(url)
        with urllib.request.urlopen(req) as resp:
            self.assertEqual(resp.status, 200)
            data = json.loads(resp.read().decode("utf-8"))
            self.assertTrue(data["ok"])
            self.assertIn("Initial line", data["content"])

    def test_api_append(self):
        url = f"http://127.0.0.1:{self.port}/api/scratchpad/append"
        payload = json.dumps({"text": "Fresh test thought"}).encode("utf-8")
        req = urllib.request.Request(url, data=payload, headers={"Content-Type": "application/json"})
        with urllib.request.urlopen(req) as resp:
            self.assertEqual(resp.status, 200)
            data = json.loads(resp.read().decode("utf-8"))
            self.assertTrue(data["ok"])
            self.assertEqual(data["status"]["lines"], 3)

        content = self.sp_path.read_text(encoding="utf-8")
        self.assertIn("Fresh test thought", content)

    def test_quick_note_default_scratchpad(self):
        url = f"http://127.0.0.1:{self.port}/api/quick-note"
        payload = json.dumps({"text": "Quick thought to scratchpad"}).encode("utf-8")
        req = urllib.request.Request(url, data=payload, headers={"Content-Type": "application/json"})
        with urllib.request.urlopen(req) as resp:
            self.assertEqual(resp.status, 200)
            data = json.loads(resp.read().decode("utf-8"))
            self.assertTrue(data["ok"])
            self.assertEqual(data["target"], "scratchpad")

        content = self.sp_path.read_text(encoding="utf-8")
        self.assertIn("Quick thought to scratchpad", content)

    def test_api_bulletin(self):
        url = f"http://127.0.0.1:{self.port}/api/bulletin"
        req = urllib.request.Request(url)
        with urllib.request.urlopen(req) as resp:
            self.assertEqual(resp.status, 200)
            data = json.loads(resp.read().decode("utf-8"))
            self.assertIn("briefing_title", data)
            self.assertIn("briefing_text", data)

    def test_api_card_status(self):
        import urllib.error
        url = f"http://127.0.0.1:{self.port}/api/card/non_existent_card_999"
        req = urllib.request.Request(url)
        with self.assertRaises(urllib.error.HTTPError) as ctx:
            urllib.request.urlopen(req)
        self.assertEqual(ctx.exception.code, 404)


if __name__ == "__main__":
    unittest.main()
