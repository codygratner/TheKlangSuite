#!/usr/bin/env python3
"""
tools/tests/test_dynamic_triage.py — Unit Tests for Dynamic Triage Engine
========================================================================
Tests dynamic multi-key sorting (priority, status, date), contiguous dynamic
rank assignments, atomic card archival, and backing file movement.
Part of Flight 5 / rj-07.
"""

import json
import shutil
import sys
import tempfile
import unittest
from pathlib import Path

# Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

from tools.check_mail import handle_archive_card
from tools.models import MailItem, MailQueue, PriorityLevel, TriageStatus


class TestDynamicTriage(unittest.TestCase):
    def setUp(self):
        self.test_dir = Path(tempfile.mkdtemp(prefix="test_dyn_triage_"))
        self.inbox_dir = self.test_dir / "Inbox"
        self.inbox_dir.mkdir(parents=True)
        self.archive_dir = self.inbox_dir / "Archive"
        self.archive_dir.mkdir(parents=True)

        self.queue_path = self.test_dir / "mail_queue.json"
        self.out_html = self.test_dir / "test_board.html"

    def tearDown(self):
        if self.test_dir.exists():
            shutil.rmtree(self.test_dir, ignore_errors=True)

    def test_dynamic_multi_key_sorting(self):
        # Create items with different priority, status, and dates
        items = [
            MailItem(id="p2_pending_old", title="P2 Old", priority=PriorityLevel.P2, status=TriageStatus.PENDING, created_at="2026-10-01"),
            MailItem(id="p0_completed", title="P0 Done", priority=PriorityLevel.P0, status=TriageStatus.COMPLETED, created_at="2026-10-12"),
            MailItem(id="p0_pending_new", title="P0 New", priority=PriorityLevel.P0, status=TriageStatus.PENDING, created_at="2026-10-12"),
            MailItem(id="p0_pending_old", title="P0 Old", priority=PriorityLevel.P0, status=TriageStatus.PENDING, created_at="2026-10-05"),
            MailItem(id="p1_pending", title="P1 Pending", priority=PriorityLevel.P1, status=TriageStatus.PENDING, created_at="2026-10-10"),
            MailItem(id="p2_pending_new", title="P2 New", priority=PriorityLevel.P2, status=TriageStatus.PENDING, created_at="2026-10-11"),
        ]

        queue = MailQueue(schema_version="1.0.0", items=items)
        queue.sort_by_priority()

        ids = [it.id for it in queue.items]
        # P0 pending (newest first), then P0 completed, then P1 pending, then P2 pending (newest first)
        expected = [
            "p0_pending_new",
            "p0_pending_old",
            "p0_completed",
            "p1_pending",
            "p2_pending_new",
            "p2_pending_old",
        ]
        self.assertEqual(ids, expected)

        # Contiguous indices
        indices = [it.idx for it in queue.items]
        self.assertEqual(indices, [1, 2, 3, 4, 5, 6])
        self.assertEqual(queue.p0_count, 3)

    def test_handle_archive_card_moves_file_and_updates_ledger(self):
        # Create a backing note in Inbox
        note_file = self.inbox_dir / "my_idea.md"
        note_file.write_text("# My Great Idea\nSome description.", encoding="utf-8")

        item = MailItem(
            id="card_01",
            title="My Great Idea",
            priority=PriorityLevel.P1,
            status=TriageStatus.PENDING,
            source_file="my_idea.md",
            created_at="2026-10-12",
        )
        queue = MailQueue(schema_version="1.0.0", items=[item])
        self.queue_path.write_text(queue.model_dump_json(indent=2), encoding="utf-8")

        res = handle_archive_card(
            target_id="card_01",
            queue_path=self.queue_path,
            out_path=self.out_html,
            is_sandbox=False,
            inbox_dir_override=self.inbox_dir,
        )
        self.assertEqual(res, 0)

        # Verify note moved to Archive/
        self.assertFalse(note_file.exists())
        archived_note = self.archive_dir / "my_idea.md"
        self.assertTrue(archived_note.exists())

        # Verify ledger updated
        updated_data = json.loads(self.queue_path.read_text(encoding="utf-8"))
        updated_item = updated_data["items"][0]
        self.assertEqual(updated_item["status"], "COMPLETED")
        self.assertEqual(updated_item["verdict"], "ARCHIVED")

    def test_handle_archive_card_sandbox_mode(self):
        item = MailItem(
            id="sbx_card",
            title="Sandbox Card",
            priority=PriorityLevel.P2,
            status=TriageStatus.PENDING,
            created_at="2026-10-12",
        )
        queue = MailQueue(schema_version="1.0.0", items=[item])
        self.queue_path.write_text(queue.model_dump_json(indent=2), encoding="utf-8")

        res = handle_archive_card(
            target_id="sbx_card",
            queue_path=self.queue_path,
            out_path=self.out_html,
            is_sandbox=True,
        )
        self.assertEqual(res, 0)

        updated_data = json.loads(self.queue_path.read_text(encoding="utf-8"))
        self.assertEqual(updated_data["items"][0]["status"], "COMPLETED")


if __name__ == "__main__":
    unittest.main()
