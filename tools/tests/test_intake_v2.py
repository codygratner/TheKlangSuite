#!/usr/bin/env python3
"""
Unit and Integration Tests for N'kai Mailbag Intake & Triage v2 (Flight 7 / rj-07 & rj-17)
Tests:
- Ramble Overflow Queue slicing (> 3 ideas) and overflow.json persistence
- Filename-only quick note extraction and metadata tagging
- Attention tag extraction (!IMPORTANT, !BLOATED, !INCOMPLETE, !SPIKE)
- mail_add.py position splicing, rank renumbering, and note generation
- Tabled folder scanning and tag assignment
- Board renderer badges (attention, overflow, tabled, quick-note) and session launchers
"""

import json
from pathlib import Path
import sys
import tempfile
import unittest

# Universal UTF-8 Environment Invariant (Flight 18)
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(REPO_ROOT))

from tools.models import MailItem, PriorityLevel, TriageStatus, RambleCard
from tools.check_mail import extract_note_metadata, scan_all_inbox_notes
from tools.ramble_ingest import parse_ramble_file, save_overflow_cards
from tools.mail_add import add_mail_item
from tools.board_renderer import render_card_html, load_overflow_map


class TestMailbagIntakeV2(unittest.TestCase):
    def setUp(self):
        self.temp_dir_obj = tempfile.TemporaryDirectory()
        self.temp_dir = Path(self.temp_dir_obj.name)

    def tearDown(self):
        self.temp_dir_obj.cleanup()

    def test_ramble_overflow_slicing(self):
        """Test 1: Decomposing a note with > 3 ideas promotes top 3 and stores remainder in overflow.json."""
        note_content = """# Stream of Consciousness Audio Ideas
First idea: Polyphonic distortion module with per-voice waveshaping and bias controls.
Also, should have low-pass filtering.

Second idea: Granular clouds engine running on TBD-16 hardware using circular DMA buffer.
We can combine this with audio feedback.

Third idea: N'kai sidecar interactive haptics detents for sliders and knobs in the UI.
Make sure Web Audio oscillator chimes properly.

Fourth idea: A simple chorus and flanger unit with BBD bucket-brigade delay lines.
This might be part of the planter effect suite.

Fifth idea: FastMath transcendental vectorized SIMD approximations for AVX2 and NEON.
Needed for low-latency oscillator algorithms.
"""
        note_path = self.temp_dir / "Multi_Ramble.md"
        note_path.write_text(note_content, encoding="utf-8")
        overflow_path = self.temp_dir / "overflow.json"

        result = parse_ramble_file(note_path, overflow_path=overflow_path, write_overflow=True)

        # Assert top 3 active cards
        self.assertEqual(len(result.cards), 3, "Only top 3 cards should remain in active extraction result")
        for c in result.cards:
            self.assertEqual(c.split_group, "Multi_Ramble", "Each active card must retain split_group")
            self.assertEqual(c.source_note, str(note_path), "Each card must record source_note path")

        # Assert overflow.json persistence
        self.assertTrue(overflow_path.exists(), "overflow.json should be created")
        overflow_data = json.loads(overflow_path.read_text(encoding="utf-8"))
        self.assertIsInstance(overflow_data, list)
        self.assertEqual(len(overflow_data), 2, "Remaining 2 cards must be stored in overflow.json")
        for oc in overflow_data:
            self.assertEqual(oc.get("split_group"), "Multi_Ramble")

    def test_quick_note_filename_only(self):
        """Test 2: Notes with empty or whitespace bodies derive title from filename stem and add quick-note tag."""
        # 1. 0-byte file
        empty_note = self.temp_dir / "Quick Mobile Thought.md"
        empty_note.write_text("", encoding="utf-8")
        meta1 = extract_note_metadata(empty_note)

        self.assertIsNotNone(meta1)
        self.assertEqual(meta1["title"], "Quick Mobile Thought")
        self.assertIn("quick-note", meta1["tags"])
        self.assertEqual(meta1["preview"], "(quick note — title only)")
        self.assertEqual(meta1["excerpt"], "(quick note — title only)")

        # 2. Whitespace-only file with frontmatter but no body
        ws_note = self.temp_dir / "Whitespace Thought.md"
        ws_note.write_text("---\npriority: P0\n---\n   \n\n  ", encoding="utf-8")
        meta2 = extract_note_metadata(ws_note)

        self.assertIsNotNone(meta2)
        self.assertEqual(meta2["title"], "Whitespace Thought")
        self.assertIn("quick-note", meta2["tags"])
        self.assertEqual(meta2["priority"], "P0")
        self.assertEqual(meta2["preview"], "(quick note — title only)")

    def test_attention_tag_extraction(self):
        """Test 3: Attention markers (!IMPORTANT, !BLOATED, !INCOMPLETE, !SPIKE) parse into clean semantic tags."""
        note_content = """---
title: Complex Synthesizer Spec
---
!IMPORTANT This must run zero allocations on the audio thread.
!BLOATED This feature has 12 sub-panels and needs to be split.
!INCOMPLETE Early draft of the DSP differential equations.
!SPIKE Fast spike exploring JUCE 9.0.3 timer hygiene.
"""
        note_path = self.temp_dir / "Attention_Test.md"
        note_path.write_text(note_content, encoding="utf-8")
        meta = extract_note_metadata(note_path)

        self.assertIsNotNone(meta)
        self.assertIn("important", meta["tags"])
        self.assertIn("bloated", meta["tags"])
        self.assertIn("incomplete", meta["tags"])
        self.assertIn("spike", meta["tags"])

    def test_mail_add_position_splicing(self):
        """Test 4: mail_add.py creates markdown note and splices card into rejigger.json with contiguous renumbering."""
        inbox_dir = self.temp_dir / "Inbox"
        inbox_dir.mkdir(parents=True)
        rejigger_path = self.temp_dir / "rejigger.json"

        # Initialize synthetic rejigger with 3 existing cards
        initial_rejigger = {
            "schema_version": "1.0.0",
            "cards": [
                {"id": "rj-01", "rank": 1, "title": "Card 1", "priority": "P0", "status": "PENDING"},
                {"id": "rj-02", "rank": 2, "title": "Card 2", "priority": "P1", "status": "PENDING"},
                {"id": "rj-03", "rank": 3, "title": "Card 3", "priority": "P2", "status": "PENDING"},
            ],
        }
        rejigger_path.write_text(json.dumps(initial_rejigger, indent=2), encoding="utf-8")

        # Splice new card after rj-01
        res = add_mail_item(
            title="Spliced Feature Topic",
            body="Inserted right between Card 1 and Card 2.",
            pillar="NK",
            priority="P1",
            after_id="rj-01",
            inbox_dir_override=str(inbox_dir),
            rejigger_path_override=str(rejigger_path),
            dry_run=False,
        )

        self.assertEqual(res["rank"], 2, "New card spliced after rj-01 should have rank 2")
        self.assertTrue(Path(res["note_path"]).exists(), "Markdown note file must be written")

        # Verify rejigger ranks are strictly contiguous [1, 2, 3, 4]
        updated_data = json.loads(rejigger_path.read_text(encoding="utf-8"))
        updated_cards = updated_data["cards"]
        self.assertEqual(len(updated_cards), 4)

        ranks = [c["rank"] for c in updated_cards]
        self.assertEqual(ranks, [1, 2, 3, 4], "Card ranks must be contiguous without gaps")

        card_ids = [c["id"] for c in updated_cards]
        self.assertEqual(card_ids[0], "rj-01")
        self.assertEqual(card_ids[1], res["id"])
        self.assertEqual(card_ids[2], "rj-02")
        self.assertEqual(card_ids[3], "rj-03")

    def test_tabled_directory_scanning(self):
        """Test 5: Notes inside Tabled/ directory receive the 'tabled' tag automatically."""
        inbox_dir = self.temp_dir / "Inbox"
        tabled_dir = inbox_dir / "Tabled"
        tabled_dir.mkdir(parents=True)

        note_file = tabled_dir / "Agent Research Proposal.md"
        note_file.write_text("# Agent Research Proposal\nExploratory topic staged by an autonomous subagent.\n", encoding="utf-8")

        scanned = scan_all_inbox_notes(inbox_dir)
        self.assertEqual(len(scanned), 1)
        self.assertIn("tabled", scanned[0]["tags"])
        self.assertTrue(scanned[0].get("is_tabled", False))

    def test_board_renderer_chips_and_session_launchers(self):
        """Test 6: render_card_html outputs chips for overflow, quick-note, tabled, attention tags, and session buttons."""
        item = MailItem(
            id="test-card-01",
            title="Comprehensive Triage Test Topic",
            priority=PriorityLevel.P0,
            status=TriageStatus.PENDING,
            tags=["quick-note", "tabled", "important", "bloated", "spike"],
            target_pillars=["NK", "TKS"],
            source_file="Inbox/Test_Topic.md",
            preview="Preview text here",
            body="Body content here",
            split_group="Test_Topic",
        )

        overflow_map = {"Test_Topic": 5}
        html_output = render_card_html(item, 1, overflow_map=overflow_map)

        # Assert badges and chips
        self.assertIn("⤵ 5 in overflow", html_output, "Missing overflow chip")
        self.assertIn("📝 quick-note", html_output, "Missing quick-note chip")
        self.assertIn("🗂 Tabled by agent", html_output, "Missing tabled chip")
        self.assertIn("⚠️ IMPORTANT", html_output, "Missing important attention badge")
        self.assertIn("📦 BLOATED (needs split)", html_output, "Missing bloated attention badge")
        self.assertIn("⚡ SPIKE", html_output, "Missing spike attention badge")
        self.assertIn('data-has-overflow="true"', html_output, "Missing data-has-overflow attribute")
        self.assertIn('data-overflow-count="5"', html_output, "Missing data-overflow-count attribute")

        # Assert session launchers
        self.assertIn("Grill-Lab", html_output, "Missing Grill-Lab launcher button")
        self.assertIn("Deep Research", html_output, "Missing Deep Research launcher button")
        self.assertIn("Draft Plan", html_output, "Missing Draft Plan launcher button")
        self.assertIn("launchSessionAction", html_output, "Missing launchSessionAction onclick handler")

    def test_frontmatter_extraction_and_card_rendering(self):
        """Test 7: Frontmatter & pseudo-metadata are parsed, stripped from message body/excerpt, and rendered on card."""
        note_content = """---
title: "Custom Synthesizer Frontmatter Test"
date: 2026-10-12
author: "Lead Architect"
---

# Custom Synthesizer Frontmatter Test
tags: [synthesizer, dsp, modular, audio]
priority: P1
pillars: [TKS, TKR]

## Core Architecture
This is the genuine human message text describing the modular DSP architecture.
It should not be polluted by frontmatter lines.
"""
        note_path = self.temp_dir / "Frontmatter_Test.md"
        note_path.write_text(note_content, encoding="utf-8")

        meta = extract_note_metadata(note_path)
        self.assertIsNotNone(meta)
        self.assertEqual(meta["priority"], "P1")
        self.assertIn("synthesizer", meta["tags"])
        self.assertIn("dsp", meta["tags"])
        self.assertEqual(meta["frontmatter"].get("author"), "Lead Architect")

        # Assert frontmatter lines stripped from message text
        self.assertNotIn("tags:", meta["excerpt"])
        self.assertNotIn("priority:", meta["excerpt"])
        self.assertTrue(meta["excerpt"].startswith("This is the genuine human message text"))
        self.assertNotIn("tags: [synthesizer", meta["body"])

        # Render card and assert frontmatter display
        item = MailItem(
            id="fm-card-01",
            title=meta["title"],
            priority=PriorityLevel.P1,
            status=TriageStatus.PENDING,
            tags=meta["tags"],
            target_pillars=meta["target_pillars"],
            source_file=str(note_path),
            preview=meta["preview"],
            excerpt=meta["excerpt"],
            body=meta["body"],
            frontmatter=meta["frontmatter"],
        )

        card_html = render_card_html(item, 1)
        self.assertIn("mail-card-meta-row", card_html, "Missing meta row on card")
        self.assertIn("#synthesizer", card_html, "Missing tag chip #synthesizer")
        self.assertIn("#modular", card_html, "Missing tag chip #modular")
        self.assertIn("author:", card_html, "Missing author frontmatter pill")
        self.assertIn("Lead Architect", card_html, "Missing author value")
        self.assertIn("mail-frontmatter-box", card_html, "Missing structured frontmatter box in details")

    def test_quicknotes_directory_scanning_and_tagging(self):
        """Test 8: Notes inside QuickNotes/ directory receive the 'quick-note' tag and are scanned."""
        inbox_dir = self.temp_dir / "Inbox"
        quick_dir = inbox_dir / "QuickNotes"
        quick_dir.mkdir(parents=True)

        note_file = quick_dir / "20261013_mobile_prompt.md"
        note_file.write_text("# Mobile Prompt\nQuick idea captured from mobile intercom.\n", encoding="utf-8")

        scanned = scan_all_inbox_notes(inbox_dir)
        self.assertEqual(len(scanned), 1)
        self.assertIn("quick-note", scanned[0]["tags"])
        self.assertTrue(scanned[0].get("is_quick_note", False))


if __name__ == "__main__":
    unittest.main()
