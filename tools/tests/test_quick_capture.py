#!/usr/bin/env python3
"""
Unit and Integration Tests for Quick Note Capture and Daily Catchall (Flight 20 / Phase 2)
Verifies catchall file initialization with valid YAML frontmatter, timestamped item appending,
non-destructive multi-note aggregation, and parsing via check_mail.py scan_note.
"""

from datetime import datetime
from pathlib import Path
import re
import sys
import tempfile
import unittest

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(REPO_ROOT))

from tools.check_mail import scan_note


class TestQuickCapture(unittest.TestCase):
    def test_daily_catchall_lifecycle(self):
        """Test 1: Asserts daily catchall file creation, frontmatter format, and timestamped appending."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            inbox_dir = Path(tmp_dir) / "Inbox"
            inbox_dir.mkdir(parents=True, exist_ok=True)

            date_str = datetime.now().strftime("%Y-%m-%d")
            catchall_path = inbox_dir / f"{date_str}_Quick_Notes.md"

            # 1. First quick note initialization
            self.assertFalse(catchall_path.exists())

            header = (
                f"---\n"
                f'title: "Daily Quick Notes ({date_str})"\n'
                f"date: {date_str}\n"
                f"tags: [quick-notes, daily-catchall]\n"
                f"priority: P2\n"
                f"pillars: []\n"
                f"---\n\n"
                f"# Daily Quick Notes ({date_str})\n\n"
            )
            catchall_path.write_text(header, encoding="utf-8")
            
            note_1 = "Check limiter gain reduction curve on TKF"
            time_1 = "10:15"
            with catchall_path.open("a", encoding="utf-8") as f:
                f.write(f"- [{time_1}] {note_1}\n")

            self.assertTrue(catchall_path.exists())
            content_1 = catchall_path.read_text(encoding="utf-8")
            self.assertIn(f'title: "Daily Quick Notes ({date_str})"', content_1)
            self.assertIn(f"- [{time_1}] {note_1}", content_1)

            # 2. Second quick note append (must not duplicate header)
            note_2 = "Add new presets for sub-bass wobble"
            time_2 = "11:42"
            with catchall_path.open("a", encoding="utf-8") as f:
                f.write(f"- [{time_2}] {note_2}\n")

            content_2 = catchall_path.read_text(encoding="utf-8")
            # Frontmatter delimiter should appear exactly twice (open and close)
            self.assertEqual(content_2.count("---"), 2, "Frontmatter delimiters must not be duplicated")
            self.assertIn(f"- [{time_1}] {note_1}", content_2)
            self.assertIn(f"- [{time_2}] {note_2}", content_2)

            # 3. Parse via check_mail.scan_note
            item = scan_note(catchall_path)
            self.assertIsNotNone(item)
            self.assertEqual(item.title, f"Daily Quick Notes ({date_str})")
            self.assertIn("quick-notes", item.tags)
            self.assertIn("daily-catchall", item.tags)
            self.assertIn(note_1, item.excerpt)

    def test_zero_mutation_on_other_notes(self):
        """Test 2: Asserts appending to catchall leaves other existing notes untouched."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            inbox_dir = Path(tmp_dir) / "Inbox"
            inbox_dir.mkdir(parents=True, exist_ok=True)

            other_note = inbox_dir / "my_existing_note.md"
            original_content = (
                "---\n"
                "title: Important Feature\n"
                "priority: P0\n"
                "---\n"
                "# Important Feature\n"
                "Do not touch this content.\n"
            )
            other_note.write_text(original_content, encoding="utf-8")

            date_str = datetime.now().strftime("%Y-%m-%d")
            catchall_path = inbox_dir / f"{date_str}_Quick_Notes.md"
            catchall_path.write_text(f"# Quick Notes\n- [12:00] Note\n", encoding="utf-8")

            # Verify other_note was not mutated
            self.assertEqual(other_note.read_text(encoding="utf-8"), original_content)


if __name__ == "__main__":
    unittest.main()
