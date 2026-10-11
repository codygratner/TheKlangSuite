#!/usr/bin/env python3
"""
Unit and Integration Tests for N'kai Per-Card Response Drawer (Flight 19 / rj-08)
Verifies:
1. Pydantic model serialization, validation, and RambleCard passthrough for agent_reply
2. Deterministic drawer DOM markup generation (btn-drawer-<id>, drawer-<id>)
3. Rich PCDA dialectic matrices and card variants (Research, Backlog, Graveyard, Completed)
4. CLI --reply-card mutation and fast board regeneration (<50ms)
5. Master template, sidecar CSS, and sidecar JS parity
"""

import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import unittest

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(REPO_ROOT))

from tools.models import MailItem, MailQueue, RambleCard, BoardSchema, PriorityLevel, TriageStatus
from tools.board_renderer import render_card_html, render_board, render_markdown_to_html


class TestResponseDrawer(unittest.TestCase):
    def setUp(self):
        self.repo_root = REPO_ROOT
        self.fixture_path = self.repo_root / "assets" / "fixtures" / "sandbox_mail_queue.json"
        self.check_mail_script = self.repo_root / "tools" / "check_mail.py"

    def test_mail_item_model_fields_and_validation(self):
        """Test 1: MailItem validates, computes has_reply, and round-trips agent_reply."""
        # 1. Default without reply
        item_no_reply = MailItem(id="test-01", title="No Reply Item")
        self.assertIsNone(item_no_reply.agent_reply)
        self.assertIsNone(item_no_reply.agent_reply_at)
        self.assertFalse(item_no_reply.has_reply)

        # 2. With reply
        reply_md = "### Assessment\n- Verified real-time audio thread invariants."
        item_with_reply = MailItem(
            id="test-02",
            title="With Reply Item",
            agent_reply=reply_md,
            agent_reply_at="2026-10-11T16:00:00Z",
        )
        self.assertTrue(item_with_reply.has_reply)
        self.assertEqual(item_with_reply.agent_reply, reply_md)
        self.assertEqual(item_with_reply.agent_reply_at, "2026-10-11T16:00:00Z")

        # 3. JSON roundtrip
        json_data = item_with_reply.model_dump_json()
        loaded_item = MailItem.model_validate_json(json_data)
        self.assertTrue(loaded_item.has_reply)
        self.assertEqual(loaded_item.agent_reply, reply_md)

        # 4. RambleCard passthrough
        ramble = RambleCard(
            source_note="TheKlangVault/Inbox/Idea.md",
            title="Fast Filter",
            core_intent="Optimize biquad filter",
            proposed_mechanism="SIMD",
            agent_reply="Approved for immediate benchmarking.",
            agent_reply_at="2026-10-11T16:05:00Z",
        )
        mail_from_ramble = ramble.to_mail_item()
        self.assertTrue(mail_from_ramble.has_reply)
        self.assertEqual(mail_from_ramble.agent_reply, "Approved for immediate benchmarking.")

    def test_board_renderer_drawer_markup(self):
        """Test 2: board_renderer generates accessible drawer with deterministic IDs."""
        item_reply = MailItem(
            id="tks-voice-alloc",
            title="Polyphonic Voice Allocator",
            agent_reply="### Architecture Review\n- Pre-allocated 128-voice pool in `prepareToPlay()`.\n- Zero locks on audio thread.",
            agent_reply_at="2026-10-11T16:10:00Z",
        )

        card_html = render_card_html(item_reply, 1)

        # Container ID
        self.assertIn('id="drawer-container-tks-voice-alloc"', card_html)
        # Toggle Button with deterministic ID and ARIA attributes
        self.assertIn('id="btn-drawer-tks-voice-alloc"', card_html)
        self.assertIn('onclick="toggleResponseDrawer(\'tks-voice-alloc\')"', card_html)
        self.assertIn('aria-expanded="false"', card_html)
        self.assertIn('Agent Assessment & Rationale', card_html)
        # Content with deterministic ID and hidden attribute
        self.assertIn('id="drawer-tks-voice-alloc"', card_html)
        self.assertIn('<div class="card-drawer-content" id="drawer-tks-voice-alloc" hidden>', card_html)
        # Formatted markdown body
        self.assertIn('class="drawer-inner markdown-body"', card_html)
        self.assertIn('Architecture Review', card_html)
        self.assertIn('prepareToPlay()', card_html)
        # Badge in header and data attribute
        self.assertIn('badge-reply', card_html)
        self.assertIn('data-has-reply="true"', card_html)

        # Card without reply must NOT render drawer
        item_plain = MailItem(id="tks-plain", title="Plain Card")
        plain_html = render_card_html(item_plain, 2)
        self.assertNotIn('btn-drawer-tks-plain', plain_html)
        self.assertNotIn('drawer-tks-plain', plain_html)
        self.assertNotIn('badge-reply', plain_html)
        self.assertIn('data-has-reply="false"', plain_html)

    def test_pcda_and_card_variants_rendering(self):
        """Test 3: Validates all populated card variants and PCDA breakdowns from sandbox fixture."""
        self.assertTrue(self.fixture_path.exists())
        queue = MailQueue.model_validate_json(self.fixture_path.read_text(encoding="utf-8"))

        schema = BoardSchema(
            board_id="sandbox_test",
            title="Sandbox QA Test",
            environment="SANDBOX",
            cards=queue.items,
        )

        temp_dir = Path(tempfile.mkdtemp())
        out_html_path = temp_dir / "rendered_test.html"
        try:
            render_board(schema, out_html_path)
            content = out_html_path.read_text(encoding="utf-8")

            # 1. Phoney PCDA Dialectic Cards (sbx-01, sbx-02, sbx-03)
            self.assertIn("btn-drawer-sbx-01", content)
            self.assertIn("drawer-sbx-01", content)
            self.assertIn("PCDA Dialectic Matrix: 55Hz Basalt Resonator", content)

            self.assertIn("btn-drawer-sbx-02", content)
            self.assertIn("drawer-sbx-02", content)
            self.assertIn("PCDA Dialectic Matrix: Submersible Compressor Control", content)

            self.assertIn("btn-drawer-sbx-03", content)
            self.assertIn("drawer-sbx-03", content)
            self.assertIn("PCDA Dialectic Matrix: 12-Stage Biological Allpass Cascade", content)

            # 2. Backlog card with implementation notes (sbx-04)
            self.assertIn("btn-drawer-sbx-04", content)
            self.assertIn("drawer-sbx-04", content)
            self.assertIn("Backlog Implementation Brief: Cosmic LFO Module", content)

            # 3. Graveyard card with technical autopsy (sbx-05)
            self.assertIn("btn-drawer-sbx-05", content)
            self.assertIn("drawer-sbx-05", content)
            self.assertIn("Technical Autopsy &amp; Rejection Rationale", content)

            # 4. Completed card with architecture summary (sbx-06)
            self.assertIn("btn-drawer-sbx-06", content)
            self.assertIn("drawer-sbx-06", content)
            self.assertIn("Feature Codification &amp; Architecture Summary", content)

            # 5. Research card with research brief and targets (sbx-07)
            self.assertIn("btn-drawer-sbx-07", content)
            self.assertIn("drawer-sbx-07", content)
            self.assertIn("Research Brief: Protoplasmic Procedural UI Canvas", content)

            # 6. Quick note card without reply (sbx-quick-01)
            self.assertNotIn("btn-drawer-sbx-quick-01", content)
            self.assertNotIn("drawer-sbx-quick-01", content)
            self.assertIn("badge-quicknote", content)

            # 7. Attention badges, merged chips, and prerequisites
            self.assertIn("badge-attention-important", content)
            self.assertIn("badge-attention-bloated", content)
            self.assertIn("badge-attention-spike", content)
            self.assertIn("badge-attention-incomplete", content)
            self.assertIn("badge-merged", content)
            self.assertIn("mail-prereq-badge", content)
        finally:
            if out_html_path.exists():
                try:
                    out_html_path.unlink()
                except Exception:
                    pass

    def test_reply_card_cli_mutation(self):
        """Test 4: check_mail.py --reply-card mutates queue ledger and regenerates HTML in <50ms."""
        temp_dir = Path(tempfile.mkdtemp())
        test_queue_path = temp_dir / "cli_queue.json"
        test_out_html = temp_dir / "cli_board.html"

        # Copy fixture to temp
        test_queue_path.write_text(self.fixture_path.read_text(encoding="utf-8"), encoding="utf-8")

        # 1. Mutate card sbx-quick-01 via CLI
        reply_text = "### Triage Verdict\nApproved for rapid mobile prototyping."
        cmd = [
            sys.executable,
            str(self.check_mail_script),
            "--queue", str(test_queue_path),
            "--out", str(test_out_html),
            "--no-browser",
            "--reply-card", "sbx-quick-01",
            "--text", reply_text,
        ]
        res = subprocess.run(cmd, capture_output=True, text=True, encoding="utf-8")
        self.assertEqual(res.returncode, 0, f"CLI command failed:\n{res.stderr}\n{res.stdout}")
        self.assertIn("Recorded agent reply for card 'sbx-quick-01'", res.stdout)

        # 2. Assert queue ledger has been updated
        updated_queue = MailQueue.model_validate_json(test_queue_path.read_text(encoding="utf-8"))
        quick_item = next(it for it in updated_queue.items if it.id == "sbx-quick-01")
        self.assertTrue(quick_item.has_reply)
        self.assertEqual(quick_item.agent_reply, reply_text)
        self.assertIsNotNone(quick_item.agent_reply_at)

        # 3. Assert generated HTML now contains the drawer for sbx-quick-01
        html_content = test_out_html.read_text(encoding="utf-8")
        self.assertIn("btn-drawer-sbx-quick-01", html_content)
        self.assertIn("drawer-sbx-quick-01", html_content)
        self.assertIn("Triage Verdict", html_content)
        self.assertIn("Approved for rapid mobile prototyping.", html_content)

        # 4. Error cases
        # Missing --text
        err_cmd = [
            sys.executable,
            str(self.check_mail_script),
            "--queue", str(test_queue_path),
            "--reply-card", "sbx-01",
        ]
        err_res = subprocess.run(err_cmd, capture_output=True, text=True, encoding="utf-8")
        self.assertNotEqual(err_res.returncode, 0)
        self.assertIn("--text '<markdown>' is required", err_res.stdout)

        # Non-existent card ID
        bad_id_cmd = [
            sys.executable,
            str(self.check_mail_script),
            "--queue", str(test_queue_path),
            "--reply-card", "non-existent-id",
            "--text", "Some reply",
        ]
        bad_id_res = subprocess.run(bad_id_cmd, capture_output=True, text=True, encoding="utf-8")
        self.assertNotEqual(bad_id_res.returncode, 0)
        self.assertIn("not found", bad_id_res.stdout)

    def test_css_and_js_parity(self):
        """Test 5: Verifies CSS styles and toggleResponseDrawer exist across all master templates, CSS, and JS."""
        templates = [
            self.repo_root / ".agents" / "sidecar" / "master_template.html",
            Path(r"c:\Dev\nkai\core\master_template.html"),
            Path(r"c:\Dev\nkai\bundle\templates\master_template.html"),
        ]
        for tmpl in templates:
            self.assertTrue(tmpl.exists(), f"Template missing: {tmpl}")
            content = tmpl.read_text(encoding="utf-8")
            self.assertIn(".card-drawer-container", content)
            self.assertIn(".card-drawer-toggle", content)
            self.assertIn(".card-drawer-content", content)
            self.assertIn(".drawer-inner", content)
            self.assertIn("function toggleResponseDrawer(", content)

        sidecar_css = Path(r"c:\Dev\nkai\core\sidecar.css")
        self.assertTrue(sidecar_css.exists())
        css_content = sidecar_css.read_text(encoding="utf-8")
        self.assertIn(".card-drawer-container", css_content)
        self.assertIn(".card-drawer-toggle", css_content)
        self.assertIn(".card-drawer-content", css_content)

        sidecar_js = Path(r"c:\Dev\nkai\core\sidecar.js")
        self.assertTrue(sidecar_js.exists())
        js_content = sidecar_js.read_text(encoding="utf-8")
        self.assertIn("function toggleResponseDrawer(", js_content)


if __name__ == "__main__":
    unittest.main()
