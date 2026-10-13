#!/usr/bin/env python3
"""
tools/tests/test_research_daemon.py — Unit tests for N'kai Automode (Flight 3)
=============================================================================
"""

from datetime import datetime
import json
import os
from pathlib import Path
import sys
import tempfile
import unittest

# Ensure UTF-8 console output for Windows terminal safety
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(REPO_ROOT))

from tools.models import DossierDrawer, MailItem, MailQueue, PriorityLevel, TriageStatus
from tools.research_daemon import (
    execute_pass_1_divergence,
    execute_pass_2_synthesis,
    run_automode_batch,
)


class TestResearchDaemon(unittest.TestCase):
    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()
        self.tmp_path = Path(self.temp_dir.name)

    def tearDown(self):
        self.temp_dir.cleanup()

    def test_pass_1_divergence(self):
        thesis, antithesis = execute_pass_1_divergence("Polyphonic Daisy Synth", "MTS-ESP tuning")
        self.assertIn("AFFIRMATIVE THESIS", thesis)
        self.assertIn("ADVERSARIAL CONTRARIAN STRESS-TEST", antithesis)
        self.assertIn("Where I Disagree With The Prompt", antithesis)

    def test_pass_2_synthesis_rule_of_2_bound(self):
        thesis = "Affirmative points for Daisy Synth"
        antithesis = "Adversarial stress-test for Daisy Synth"
        dossier = execute_pass_2_synthesis(
            topic="Daisy Synth",
            thesis=thesis,
            antithesis=antithesis,
            source_note="test_note.md",
            source_markdown="# Daisy Synth\nContent here",
        )

        self.assertIsInstance(dossier, DossierDrawer)
        # Invariant: Must strictly enforce pass_count == 2
        self.assertEqual(dossier.pass_count, 2)
        self.assertIn("pros", dossier.pcda_dialectic)
        self.assertIn("cons", dossier.pcda_dialectic)
        self.assertIn("lenses", dossier.pcda_dialectic)
        self.assertIn("citations", dossier.research_dossier)
        self.assertIn("gotchas", dossier.research_dossier)

    def test_mail_item_dossier_serialization(self):
        dossier = DossierDrawer(
            source_note="Idea.md",
            source_markdown="Content",
            pcda_dialectic={"pros": ["Fast"], "cons": ["Complex"]},
            pass_count=2,
            researched_at="2026-10-12T23:00:00",
        )
        item = MailItem(
            id="item_dossier_test",
            title="Dossier Test Item",
            status=TriageStatus.RESEARCH,
            dossier=dossier,
        )

        data = json.loads(item.model_dump_json())
        self.assertIn("dossier", data)
        self.assertEqual(data["dossier"]["pass_count"], 2)

        rehydrated = MailItem.model_validate(data)
        self.assertIsNotNone(rehydrated.dossier)
        self.assertEqual(rehydrated.dossier.pass_count, 2)

    def test_batch_limit_and_dry_run(self):
        # Create a mock queue with 5 items marked RESEARCH
        items = [
            MailItem(
                id=f"research_item_{i}",
                title=f"Research Item {i}",
                status=TriageStatus.RESEARCH,
            )
            for i in range(5)
        ]
        queue = MailQueue(items=items)

        # Mock queue path in temp directory
        queue_file = self.tmp_path / "mail_queue.json"
        queue_file.write_text(queue.model_dump_json(indent=2), encoding="utf-8")

        import tools.research_daemon as rd
        original_repo = rd.REPO_ROOT
        try:
            # Point REPO_ROOT to temp directory
            pipeline_dir = self.tmp_path / ".agents" / "pipeline" / "mailbag"
            pipeline_dir.mkdir(parents=True, exist_ok=True)
            mock_queue = pipeline_dir / "mail_queue.json"
            mock_queue.write_text(queue.model_dump_json(indent=2), encoding="utf-8")

            rd.REPO_ROOT = self.tmp_path

            # Run with limit 2
            results = run_automode_batch(limit=2, dry_run=True)
            self.assertEqual(len(results), 2)
            self.assertEqual(results[0]["id"], "research_item_0")
            self.assertEqual(results[1]["id"], "research_item_1")

        finally:
            rd.REPO_ROOT = original_repo


if __name__ == "__main__":
    unittest.main()
