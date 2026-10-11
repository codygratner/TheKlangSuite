#!/usr/bin/env python3
"""
Unit and Integration Tests for N'kai QA Sandbox Board (Flight 16+17 / rj-03)
Verifies zero-mutation invariant, synthetic fixture parsing, deterministic DOM IDs,
and environment isolation between SANDBOX and PRODUCTION runtime modes.
"""

import hashlib
from pathlib import Path
import subprocess
import sys
import unittest

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(REPO_ROOT))

from tools.models import MailQueue


class TestNkaiSandboxBoard(unittest.TestCase):
    def setUp(self):
        self.repo_root = REPO_ROOT
        self.fixture_path = self.repo_root / "assets" / "fixtures" / "sandbox_mail_queue.json"
        self.script_path = self.repo_root / "tools" / "check_mail.py"
        self.prod_queue_path = self.repo_root / ".agents" / "pipeline" / "mailbag" / "mail_queue.json"
        self.sandbox_html_path = self.repo_root / ".agents" / "sidecar" / "sandbox_mail_inbox.html"
        self.prod_html_path = self.repo_root / ".agents" / "sidecar" / "morning_mail_inbox.html"

    def _file_sha256(self, path: Path) -> str:
        if not path.exists():
            return ""
        return hashlib.sha256(path.read_bytes()).hexdigest()

    def test_fixture_schema_valid(self):
        """Test 1: Asserts sandbox fixture loads and conforms strictly to Pydantic MailQueue."""
        self.assertTrue(self.fixture_path.exists(), f"Fixture missing at {self.fixture_path}")
        raw_json = self.fixture_path.read_text(encoding="utf-8")
        queue = MailQueue.model_validate_json(raw_json)

        self.assertEqual(queue.total_items, 7, "Fixture should have exactly 7 items")
        self.assertEqual(len(queue.items), 7, "Fixture items list should contain 7 entries")
        self.assertEqual(queue.p0_count, 2, "Fixture should have exactly 2 P0 items")

        expected_ids = [f"sbx-0{i}" for i in range(1, 8)]
        actual_ids = [it.id for it in queue.items]
        self.assertEqual(actual_ids, expected_ids, "Expected sbx-01 through sbx-07 in order")

    def test_sandbox_zero_mutation(self):
        """Test 2: Asserts running check_mail.py in sandbox mode never mutates production ledger or vault."""
        prod_hash_before = self._file_sha256(self.prod_queue_path)
        staging_dir = self.repo_root / ".agents" / "pipeline" / "staging"
        staging_files_before = set(staging_dir.glob("*")) if staging_dir.exists() else set()

        cmd = [
            sys.executable,
            str(self.script_path),
            "--sandbox",
            "--no-browser",
        ]
        res = subprocess.run(cmd, cwd=str(self.repo_root), capture_output=True, text=True, encoding="utf-8")
        self.assertEqual(res.returncode, 0, f"check_mail.py --sandbox failed:\n{res.stderr}\n{res.stdout}")

        prod_hash_after = self._file_sha256(self.prod_queue_path)
        self.assertEqual(
            prod_hash_before,
            prod_hash_after,
            "CRITICAL: Production mail_queue.json was mutated by a sandbox execution run!",
        )

        staging_files_after = set(staging_dir.glob("*")) if staging_dir.exists() else set()
        self.assertEqual(
            staging_files_before,
            staging_files_after,
            "CRITICAL: Sandbox mode created unexpected staging files!",
        )

    def test_sandbox_html_content(self):
        """Test 3: Asserts generated sandbox HTML contains deterministic IDs, banner, and environment variable."""
        # Ensure sandbox HTML is generated
        cmd = [
            sys.executable,
            str(self.script_path),
            "--sandbox",
            "--no-browser",
        ]
        res = subprocess.run(cmd, cwd=str(self.repo_root), capture_output=True, text=True, encoding="utf-8")
        self.assertEqual(res.returncode, 0)
        self.assertTrue(self.sandbox_html_path.exists(), "sandbox_mail_inbox.html was not generated")

        content = self.sandbox_html_path.read_text(encoding="utf-8")

        # Visual indicator and environment variable
        self.assertIn("<title>[SBX]", content, "Sandbox page title missing [SBX] prefix")
        self.assertIn('id="sandbox-banner"', content, "Missing sandbox-banner DOM element")
        self.assertIn('window.NKAI_ENVIRONMENT = "SANDBOX"', content, "window.NKAI_ENVIRONMENT must be 'SANDBOX'")

        # Deterministic Action IDs
        self.assertIn('id="btn-approve-sbx-01"', content, "Missing deterministic button id btn-approve-sbx-01")
        self.assertIn('id="btn-backlog-sbx-02"', content, "Missing deterministic button id btn-backlog-sbx-02")
        self.assertIn('id="btn-research-sbx-03"', content, "Missing deterministic button id btn-research-sbx-03")
        self.assertIn('id="btn-graveyard-sbx-04"', content, "Missing deterministic button id btn-graveyard-sbx-04")
        self.assertIn('id="btn-pcda-sbx-05"', content, "Missing deterministic button id btn-pcda-sbx-05")

        # Deterministic Filter Pill IDs
        self.assertIn('id="pill-pillar-all"', content, "Missing Rail 1 pill-pillar-all filter element")
        self.assertIn('id="pill-status-untriaged"', content, "Missing Rail 2 pill-status-untriaged filter element")
        self.assertIn('id="pill-status-all"', content, "Missing Rail 2 pill-status-all filter element")
        self.assertIn('id="pill-status-p0"', content, "Missing Rail 2 pill-status-p0 filter element")

    def test_production_regression(self):
        """Test 4: Asserts production mode continues functioning with clean isolation and PRODUCTION environment."""
        cmd = [
            sys.executable,
            str(self.script_path),
            "--archive",
            "--no-browser",
        ]
        res = subprocess.run(cmd, cwd=str(self.repo_root), capture_output=True, text=True, encoding="utf-8")
        self.assertEqual(res.returncode, 0, f"check_mail.py --archive failed:\n{res.stderr}\n{res.stdout}")
        self.assertTrue(self.prod_html_path.exists(), "morning_mail_inbox.html missing")

        content = self.prod_html_path.read_text(encoding="utf-8")

        self.assertIn('window.NKAI_ENVIRONMENT = "PRODUCTION"', content, "Production HTML missing window.NKAI_ENVIRONMENT='PRODUCTION'")
        self.assertNotIn('id="sandbox-banner"', content, "Production HTML must NOT contain sandbox-banner element")
        self.assertNotIn("<title>[SBX]", content, "Production HTML title must NOT contain [SBX]")


    def test_board_schema_roundtrip(self):
        """Test 5: Asserts BoardSchema model serialization, JSON schema export, and board_renderer rendering."""
        from tools.models import BoardSchema
        from tools.board_renderer import render_board

        fixture_data = self.fixture_path.read_text(encoding="utf-8")
        queue = MailQueue.model_validate_json(fixture_data)

        schema = BoardSchema(
            board_id="unit_test_board",
            title="Unit Test Master Board",
            subtitle="Testing roundtrip schema-driven rendering",
            environment="SANDBOX",
            cards=queue.items,
            allowance_budget=150000,
        )

        test_out = self.repo_root / ".agents" / "sidecar" / "test_roundtrip_board.html"
        try:
            rendered_path = render_board(schema, test_out)
            self.assertTrue(rendered_path.exists(), "Rendered test HTML does not exist")
            rendered_html = rendered_path.read_text(encoding="utf-8")

            self.assertIn("Unit Test Master Board", rendered_html)
            self.assertIn('[SBX] Unit Test Master Board', rendered_html)
            self.assertIn('window.NKAI_ENVIRONMENT = "SANDBOX"', rendered_html)
            self.assertIn('id="sandbox-banner"', rendered_html)
            self.assertIn('btn-approve-sbx-01', rendered_html)
        finally:
            if test_out.exists():
                try:
                    test_out.unlink()
                except Exception:
                    pass


if __name__ == "__main__":
    unittest.main()
