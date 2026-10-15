#!/usr/bin/env python3
"""
tools/tests/test_verify_context_clues.py — Unit Tests for tools/verify_context_clues.py
"""

import sys
import tempfile
import unittest
from pathlib import Path

# Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

from tools import verify_context_clues


class TestVerifyContextClues(unittest.TestCase):
    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()
        self.workspace_root = Path(self.temp_dir.name)

    def tearDown(self):
        self.temp_dir.cleanup()

    def test_custom_uri_schemes_skipped(self):
        test_file = self.workspace_root / "context_clues_test.md"
        content = """# Context Clues: Test
## Active SemVer
## Return Agenda
## Architectural Memory
## Taxonomy
## Invariants

- [Mailbag](sidecar://nkai/panel/?board=morning_mail)
- [Agenda](sidecar://nkai/panel/?board=agenda)
- [Chat](conversation://test-convo-id)
- [Web](https://example.com)
- [Email](mailto:dev@klang.city)
"""
        test_file.write_text(content, encoding="utf-8")

        res = verify_context_clues.check_file(test_file, self.workspace_root)
        self.assertEqual(len(res["missing_sections"]), 0)
        self.assertEqual(len(res["broken_links"]), 0)
        self.assertEqual(res["remote_links"], 5)
        self.assertEqual(res["local_links"], 0)

    def test_local_links_checked(self):
        existing_doc = self.workspace_root / "existing.md"
        existing_doc.write_text("# Existing Doc", encoding="utf-8")

        test_file = self.workspace_root / "context_clues_test.md"
        content = f"""# Context Clues: Test
## Active SemVer
## Return Agenda
## Architectural Memory
## Taxonomy
## Invariants

- [Good Relative](existing.md)
- [Good Absolute](file:///{str(existing_doc).replace('\\\\', '/')})
- [Bad Relative](missing_file.md)
"""
        test_file.write_text(content, encoding="utf-8")

        res = verify_context_clues.check_file(test_file, self.workspace_root)
        self.assertEqual(len(res["broken_links"]), 1)
        self.assertEqual(res["broken_links"][0]["label"], "Bad Relative")
        self.assertEqual(res["local_links"], 3)

    def test_active_context_validation(self):
        active_file = self.workspace_root / "ACTIVE_CONTEXT.md"
        active_file.write_text("# Active Workspace Context\nContent here", encoding="utf-8")

        res = verify_context_clues.check_file(active_file, self.workspace_root)
        self.assertTrue(res["is_active_context"])
        self.assertEqual(len(res["missing_sections"]), 0)


if __name__ == "__main__":
    unittest.main()
