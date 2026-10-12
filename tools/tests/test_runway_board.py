#!/usr/bin/env python3
"""
Unit and Integration Tests for Flight Runway Board (Flight 20 / Act III)
Verifies BoardSchema compliance, Act III flight parsing, milestone generation,
and filter rail rendering in runway_board.html.
"""

from datetime import datetime
import json
from pathlib import Path
import sys
import tempfile
import unittest

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(REPO_ROOT))

from tools.models import BoardSchema, BoardRegistry, BoardMetadata, TriageStatus, PriorityLevel
from tools.build_runway import parse_act3_flights, build_ecosystem_milestones, build_runway


class TestRunwayBoard(unittest.TestCase):
    def setUp(self):
        self.repo_root = REPO_ROOT
        self.backlog_path = self.repo_root / "docs" / "BACKLOG.md"

    def test_backlog_parsing(self):
        """Test 1: Asserts parse_act3_flights extracts flight cards from docs/BACKLOG.md."""
        self.assertTrue(self.backlog_path.exists(), "BACKLOG.md must exist")
        text = self.backlog_path.read_text(encoding="utf-8")
        cards = parse_act3_flights(text)

        self.assertGreater(len(cards), 10, "Expected at least 10 flight cards parsed from Act III")
        # Check flight 0 and flight 20 presence
        ids = [c.id for c in cards]
        self.assertIn("flight_0", ids, "Flight 0 card should be present")
        self.assertIn("flight_20", ids, "Flight 20 card should be present")

        f20_card = next(c for c in cards if c.id == "flight_20")
        self.assertEqual(f20_card.status, TriageStatus.COMPLETED, "Flight 20 should be COMPLETED")
        self.assertIn("NK", f20_card.pillars, "Flight 20 should target NK pillar")

    def test_milestone_cards(self):
        """Test 2: Asserts ecosystem milestones are created with valid status and pillars."""
        milestones = build_ecosystem_milestones()
        self.assertGreaterEqual(len(milestones), 8, "Expected at least 8 milestone cards")
        
        m_ids = [m.id for m in milestones]
        self.assertIn("ms_tks_v040", m_ids)
        self.assertIn("ms_nk_v035", m_ids)
        self.assertIn("ms_tt_v010", m_ids)

        tks_040 = next(m for m in milestones if m.id == "ms_tks_v040")
        self.assertIn("TKS", tks_040.pillars)
        self.assertEqual(tks_040.status, TriageStatus.APPROVED)

    def test_board_schema_validation(self):
        """Test 3: Asserts BoardSchema accepts board_type='evergreen' and serializes cleanly."""
        milestones = build_ecosystem_milestones()
        schema = BoardSchema(
            schema_version="1.0.0",
            board_id="runway",
            title="Ecosystem Delivery Runway",
            subtitle="Cross-Project Strategic Delivery Horizon",
            board_type="evergreen",
            cards=milestones
        )
        self.assertEqual(schema.board_type, "evergreen")
        self.assertEqual(len(schema.cards), len(milestones))

        dumped = schema.model_dump_json()
        loaded = BoardSchema.model_validate_json(dumped)
        self.assertEqual(loaded.board_id, "runway")
        self.assertEqual(loaded.board_type, "evergreen")

    def test_runway_html_generation(self):
        """Test 4: Asserts build_runway generates valid HTML with filter rails and cards."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            tmp_out = Path(tmp_dir) / "test_runway.html"
            out_file = build_runway(
                backlog_path=self.backlog_path,
                output_path=tmp_out,
                register_board=False
            )
            self.assertTrue(out_file.exists())
            html_text = out_file.read_text(encoding="utf-8")

            # Asserts structure
            self.assertIn("Ecosystem Delivery Runway", html_text)
            self.assertIn("pill-pillar-tks", html_text)
            self.assertIn("pill-pillar-nk", html_text)
            self.assertIn("pill-status-approved", html_text)
            self.assertIn("pill-status-completed", html_text)
            self.assertIn("mail-card", html_text)


if __name__ == "__main__":
    unittest.main()
