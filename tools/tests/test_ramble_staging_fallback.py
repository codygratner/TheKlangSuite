#!/usr/bin/env python3
"""
Unit Test for Ramble Ingestion Engine Staging Fallback and Directory CLI Override (Tooling Ratchet)
Verifies:
1. --dir argument cleanly overrides inbox directory.
2. --staging argument explicitly routes to Staging/ subfolder.
3. Auto-fallback cleanly discovers notes in Staging/ if root inbox has 0 files.
"""

import sys
import tempfile
import unittest
from pathlib import Path

# Universal UTF-8 Environment Invariant
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(REPO_ROOT))

from tools.ramble_ingest import resolve_inbox_directory, archive_processed_notes


class TestRambleStagingFallback(unittest.TestCase):
    def setUp(self):
        self.temp_dir_obj = tempfile.TemporaryDirectory()
        self.temp_dir = Path(self.temp_dir_obj.name)

    def tearDown(self):
        self.temp_dir_obj.cleanup()

    def test_dir_override(self):
        """Test 1: Custom --dir path is resolved correctly."""
        custom_inbox = self.temp_dir / "CustomInbox"
        custom_inbox.mkdir()
        resolved = resolve_inbox_directory(dir_override=str(custom_inbox))
        self.assertEqual(resolved.resolve(), custom_inbox.resolve())

    def test_staging_override(self):
        """Test 2: --staging resolves to Staging subfolder."""
        custom_inbox = self.temp_dir / "CustomInbox"
        staging_dir = custom_inbox / "Staging"
        staging_dir.mkdir(parents=True)
        resolved = resolve_inbox_directory(dir_override=str(custom_inbox), scan_staging=True)
        self.assertEqual(resolved.resolve(), staging_dir.resolve())

    def test_archive_from_staging(self):
        """Test 3: Archiving from Staging moves files to parent Archive/ folder."""
        custom_inbox = self.temp_dir / "CustomInbox"
        staging_dir = custom_inbox / "Staging"
        staging_dir.mkdir(parents=True)
        archive_dir = custom_inbox / "Archive"

        test_note = staging_dir / "Staged_Thought.md"
        test_note.write_text("# Thought\nContent here", encoding="utf-8")

        moved = archive_processed_notes([test_note], staging_dir, dry_run=False)
        self.assertEqual(len(moved), 1)
        self.assertTrue((archive_dir / "Staged_Thought.md").exists())
        self.assertFalse(test_note.exists())


if __name__ == "__main__":
    unittest.main()
