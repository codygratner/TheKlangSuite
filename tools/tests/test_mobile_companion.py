#!/usr/bin/env python3
"""
Unit and Integration Tests for N'kai Mobile Companion & Touch Ergonomics (v0.3.0 / Section 14)
Asserts responsive CSS rules, 44px Tap Target Law, sticky bottom thumb bar markup,
local Wi-Fi IP detection, zero-dependency QR code generation, and /qr endpoints.
Universal UTF-8 compliance guaranteed.
"""

from pathlib import Path
import re
import socket
import sys
import unittest

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.qr_code import QRCode, generate_qr_terminal, generate_qr_svg
from tools.nkai_serve import get_local_ip, find_available_port, NkaiHTTPRequestHandler


class TestMobileCompanionAndTouchErgonomics(unittest.TestCase):
    def setUp(self):
        self.repo_root = REPO_ROOT
        self.sidecar_css_path = Path(r"c:\Dev\nkai\core\sidecar.css")
        self.master_template_path = Path(r"c:\Dev\nkai\core\master_template.html")
        self.local_template_path = self.repo_root / ".agents" / "sidecar" / "master_template.html"
        self.morning_mail_path = self.repo_root / ".agents" / "sidecar" / "morning_mail_inbox.html"
        self.sidecar_js_path = Path(r"c:\Dev\nkai\core\sidecar.js")

    def test_sidecar_css_mobile_architecture(self):
        """Test 1: Asserts 100dvh viewport stability, 44px tap targets, and thumb bar in sidecar.css."""
        self.assertTrue(self.sidecar_css_path.exists(), f"Missing {self.sidecar_css_path}")
        css_content = self.sidecar_css_path.read_text(encoding="utf-8")

        # 1. Viewport stability (100dvh)
        self.assertIn("100dvh", css_content, "Missing 100dvh dynamic viewport height fallback")

        # 2. Responsive media query (max-width: 640px)
        self.assertIn("@media (max-width: 640px)", css_content, "Missing 640px mobile breakpoint")

        # 3. Touch Target Law: min-height 44px and min-width 44px on interactive controls
        self.assertIn("min-height: 44px", css_content, "Missing 44px minimum tap target height")
        self.assertIn("min-width: 44px", css_content, "Missing 44px minimum tap target width")
        self.assertIn("[id^=\"btn-approve-\"]", css_content, "Missing button ID selector for touch target enforcement")

        # 4. Momentum scrolling and 38px filter pills
        self.assertIn("-webkit-overflow-scrolling: touch", css_content, "Missing iOS momentum touch scrolling")
        self.assertIn("min-height: 38px", css_content, "Missing 38px filter pill touch target")

        # 5. Mobile thumb bar sticky navigation & desktop suppression
        self.assertIn(".mobile-thumb-bar", css_content, "Missing .mobile-thumb-bar selector")
        self.assertIn("position: sticky", css_content, "Missing sticky positioning on thumb bar")
        self.assertIn("env(safe-area-inset-bottom", css_content, "Missing safe-area-inset-bottom padding")
        self.assertIn("display: none !important", css_content, "Missing desktop suppression rule for thumb bar")

    def test_master_template_viewport_and_thumb_bar(self):
        """Test 2: Asserts viewport meta and .mobile-thumb-bar markup in master_template.html."""
        self.assertTrue(self.master_template_path.exists(), f"Missing {self.master_template_path}")
        html_content = self.master_template_path.read_text(encoding="utf-8")

        # 1. Enhanced viewport meta with viewport-fit=cover
        self.assertIn("viewport-fit=cover", html_content, "Missing viewport-fit=cover in meta viewport")
        self.assertIn("user-scalable=no", html_content, "Missing user-scalable=no in meta viewport")

        # 2. Markup for .mobile-thumb-bar with all 4 target buttons
        self.assertIn("id=\"mobile-thumb-bar\"", html_content, "Missing #mobile-thumb-bar element")
        self.assertIn("id=\"thumb-btn-untriaged\"", html_content, "Missing #thumb-btn-untriaged button")
        self.assertIn("id=\"thumb-btn-p0\"", html_content, "Missing #thumb-btn-p0 button")
        self.assertIn("id=\"thumb-btn-all\"", html_content, "Missing #thumb-btn-all button")
        self.assertIn("id=\"thumb-btn-hub\"", html_content, "Missing #thumb-btn-hub button")

        # 3. Frontend controller handlers and haptic vibration
        self.assertIn("handleThumbNav", html_content, "Missing handleThumbNav controller in script")
        self.assertIn("navigator.vibrate", html_content, "Missing navigator.vibrate haptic support in script")

    def test_template_mirroring_consistency(self):
        """Test 3: Asserts byte-level synchronization between core and sidecar master templates."""
        core_content = self.master_template_path.read_text(encoding="utf-8")
        local_content = self.local_template_path.read_text(encoding="utf-8")
        self.assertEqual(core_content, local_content, "master_template.html is out of sync between nkai and TheKlangSuite")

    def test_local_ip_detection(self):
        """Test 4: Asserts get_local_ip returns a valid IPv4 address."""
        ip = get_local_ip()
        self.assertIsInstance(ip, str)
        # Check valid IPv4 pattern
        parts = ip.split(".")
        self.assertEqual(len(parts), 4, f"Invalid IP format: {ip}")
        for p in parts:
            val = int(p)
            self.assertTrue(0 <= val <= 255, f"Invalid octet in IP: {val}")

    def test_qr_code_generator_terminal_and_svg(self):
        """Test 5: Asserts pure Python QR generator produces valid terminal blocks and SVG vector output."""
        test_url = "http://192.168.1.100:3030/morning_mail_inbox.html"
        qr = QRCode(test_url, level="L")

        # Matrix dimensions
        self.assertGreaterEqual(qr.version, 1)
        self.assertEqual(qr.size, 21 + 4 * (qr.version - 1))
        self.assertEqual(len(qr.matrix), qr.size)
        self.assertEqual(len(qr.matrix[0]), qr.size)

        # Terminal block output (Unicode half-blocks)
        terminal_qr = qr.render_terminal(border=2)
        self.assertIn("█", terminal_qr, "Missing block character in terminal QR output")
        lines = terminal_qr.strip().split("\n")
        self.assertGreater(len(lines), 10, "Terminal QR output has insufficient rows")

        # Convenience helper
        term_helper = generate_qr_terminal(test_url, border=2)
        self.assertEqual(terminal_qr, term_helper)

        # SVG vector output
        svg_output = qr.render_svg(border=2, size=256)
        self.assertIn("<svg", svg_output)
        self.assertIn("</svg>", svg_output)
        self.assertIn("viewBox=", svg_output)
        self.assertIn("<rect", svg_output)

        # SVG helper
        svg_helper = generate_qr_svg(test_url, border=2, size=256)
        self.assertEqual(svg_output, svg_helper)

    def test_sidecar_js_haptic_and_thumb_integration(self):
        """Test 6: Asserts sidecar.js contains navigator.vibrate and initMobileThumbBar."""
        self.assertTrue(self.sidecar_js_path.exists(), f"Missing {self.sidecar_js_path}")
        js_content = self.sidecar_js_path.read_text(encoding="utf-8")
        self.assertIn("navigator.vibrate", js_content, "Missing navigator.vibrate in sidecar.js")
        self.assertIn("initMobileThumbBar", js_content, "Missing initMobileThumbBar in sidecar.js")

    def test_rendered_morning_mail_mobile_elements(self):
        """Test 7: Asserts rendered morning_mail_inbox.html includes all mobile companion features."""
        if self.morning_mail_path.exists():
            rendered_html = self.morning_mail_path.read_text(encoding="utf-8")
            self.assertIn("id=\"mobile-thumb-bar\"", rendered_html)
            self.assertIn("thumb-btn-untriaged", rendered_html)
            self.assertIn("min-height: 44px", rendered_html)
            self.assertIn("viewport-fit=cover", rendered_html)


if __name__ == "__main__":
    unittest.main()
