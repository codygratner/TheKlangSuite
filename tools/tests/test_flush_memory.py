#!/usr/bin/env python3
"""
tools/tests/test_flush_memory.py — Unit Tests for tools/flush_memory.py
"""

import sys
import unittest
from pathlib import Path

# Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

from tools import flush_memory


class TestFlushMemory(unittest.TestCase):
    def test_check_staged_souls(self):
        staged = flush_memory.check_staged_souls()
        self.assertIsInstance(staged, list)

    def test_dry_run_append_dev_history(self):
        res = flush_memory.append_dev_history(
            role="Test Runner",
            summary="Unit testing dry run",
            dry_run=True,
        )
        self.assertTrue(res)

    def test_dry_run_archive_plan(self):
        dest = flush_memory.archive_completed_plan(
            slug="test_slug",
            force=True,
            dry_run=True,
        )
        self.assertIsNotNone(dest)
        self.assertTrue(str(dest).endswith("test_slug.md"))

    def test_dry_run_full_flush(self):
        exit_code = flush_memory.run_flush(
            role="Test Runner",
            summary="Dry run validation",
            dry_run=True,
        )
        self.assertEqual(exit_code, 0)


if __name__ == "__main__":
    unittest.main()
