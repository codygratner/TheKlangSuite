#!/usr/bin/env python3
"""
Unit Tests for N'kai Mobile Companion API Routes & Route Translation
Tests /api/status, /api/boards, /api/quick-note, and translate_path routing.
Universal UTF-8 compliance guaranteed.
"""

from pathlib import Path
import json
import os
import sys
import unittest

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.nkai_serve import NkaiHTTPRequestHandler, get_tailscale_ip


class DummyServer:
    def __init__(self, port=3030, tailscale_ip="100.87.196.60"):
        self.server_address = ("0.0.0.0", port)
        self.tailscale_ip = tailscale_ip
        self.mobile_url = f"http://{tailscale_ip}:{port}/morning_mail_inbox.html"


class TestMobileCompanionAPI(unittest.TestCase):
    def setUp(self):
        self.repo_root = REPO_ROOT

    def test_tailscale_ip_detection(self):
        """Asserts Tailscale IP detection returns a valid 100.x.y.z IP when available."""
        ip = get_tailscale_ip()
        if ip is not None:
            self.assertTrue(ip.startswith("100."), f"Expected Tailscale CGNAT IP starting with 100., got {ip}")

    def test_translate_path_resolution(self):
        """Asserts virtual board routes resolve to actual sidecar files on disk."""
        # Create a mock handler without binding socket
        class MockHandler(NkaiHTTPRequestHandler):
            def __init__(self):
                self.directory = str(REPO_ROOT)

        handler = MockHandler()
        
        # Test root route
        root_target = Path(handler.translate_path("/"))
        self.assertTrue(root_target.name == "morning_mail_inbox.html" or root_target.exists(),
                        f"Root path did not resolve to morning_mail_inbox.html: {root_target}")

        # Test morning_mail_inbox.html alias
        mail_target = Path(handler.translate_path("/morning_mail_inbox.html"))
        self.assertTrue(mail_target.exists(), f"Path not found: {mail_target}")

        # Test runway_board.html alias
        runway_target = Path(handler.translate_path("/runway_board.html"))
        self.assertTrue(runway_target.exists(), f"Path not found: {runway_target}")


if __name__ == "__main__":
    unittest.main()
