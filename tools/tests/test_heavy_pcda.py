#!/usr/bin/env python3
"""
tools/tests/test_heavy_pcda.py — Unit tests for 2-Round Heavy PCDA Automation Engine
====================================================================================
Asserts:
1. Strict 2-round cap (round_count == 2).
2. Proper schema output across Round 1, Round 2, and Final 80/20 Synthesis.
3. Correct rendering of Markdown and self-contained HTML canvas.
4. File I/O and UTF-8 verification.
"""

import sys
import tempfile
import unittest
from pathlib import Path

# Configure UTF-8 streams for Windows console compatibility
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.heavy_pcda import HeavyPCDAEngine, HeavyPCDAResult


class TestHeavyPCDAEngine(unittest.TestCase):
    """Test suite for HeavyPCDAEngine."""

    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()
        self.out_dir = Path(self.temp_dir.name)
        self.engine = HeavyPCDAEngine(online=True)

    def tearDown(self):
        self.temp_dir.cleanup()

    def test_strict_two_round_cap(self):
        """Asserts that the dialectic crucible is hard-capped at exactly 2 rounds."""
        res = self.engine.run_crucible("Test Topic: Low-Latency DSP", mock=True)
        self.assertEqual(res.round_count, 2, "Heavy PCDA must be strictly capped at 2 rounds.")
        self.assertTrue(res.online_authorized)

    def test_schema_completeness(self):
        """Asserts all dialectic lenses and synthesis sections are populated."""
        res = self.engine.run_crucible("Memory Management Invariants", mock=True)

        # Round 1
        self.assertTrue(res.round_1.proposer_thesis)
        self.assertGreaterEqual(len(res.round_1.proposer_points), 1)
        self.assertGreaterEqual(len(res.round_1.critic_disagreements), 1)
        self.assertGreaterEqual(len(res.round_1.critic_inversions), 1)
        self.assertGreaterEqual(len(res.round_1.critic_failure_modes), 1)

        # Round 2
        self.assertTrue(res.round_2.proposer_rebuttal)
        self.assertGreaterEqual(len(res.round_2.proposer_proofs), 1)
        self.assertTrue(res.round_2.critic_counter)
        self.assertGreaterEqual(len(res.round_2.critic_residual_traps), 1)
        self.assertGreaterEqual(len(res.round_2.empirical_evidence), 1)

        # Final Synthesis
        self.assertTrue(res.final_synthesis.synthesis_rationale)
        self.assertGreaterEqual(len(res.final_synthesis.the_80_20_resolution), 1)
        self.assertGreaterEqual(len(res.final_synthesis.discarded_mechanisms), 1)
        self.assertGreaterEqual(len(res.final_synthesis.falsification_tests), 1)

    def test_markdown_rendering(self):
        """Asserts Markdown output contains mandatory dialectic headers."""
        res = self.engine.run_crucible("Mobile Navigation Architecture", mock=True)
        md = self.engine.render_markdown(res)

        self.assertIn("# 🔬 Heavy PCDA Dialectic Crucible", md)
        self.assertIn("## Round 1: Divergent Theses & Assumption Inversion", md)
        self.assertIn("## Round 2: Cross-Critique & Empirical Falsification", md)
        self.assertIn("## ⚖️ Final Synthesis: The Pragmatic 80/20 Resolution", md)
        self.assertIn("Crucible Rounds:** 2 (Hard Capped)", md)

    def test_html_rendering(self):
        """Asserts HTML output is valid, styled, and contains all lenses."""
        res = self.engine.run_crucible("Mobile Navigation Architecture", mock=True)
        html_out = self.engine.render_html(res)

        self.assertIn("<!DOCTYPE html>", html_out)
        self.assertIn("Round Cap: 2 / 2 Strict", html_out)
        self.assertIn("🟢 Proposer Thesis", html_out)
        self.assertIn("🔴 Critic Inversion", html_out)
        self.assertIn("80/20 Synthesis Protocol", html_out)
        self.assertIn("--bg: #090d13;", html_out)

    def test_save_results_and_utf8_encoding(self):
        """Asserts saving produces valid UTF-8 files on disk."""
        res = self.engine.run_crucible("UTF-8 Symbol Test: 🔬 🛠️ ⚡ 🪦", mock=True)
        html_path, md_path = self.engine.save_results(res, out_dir=self.out_dir)

        self.assertTrue(html_path.exists())
        self.assertTrue(md_path.exists())

        html_text = html_path.read_text(encoding="utf-8")
        md_text = md_path.read_text(encoding="utf-8")

        self.assertIn("UTF-8 Symbol Test", html_text)
        self.assertIn("UTF-8 Symbol Test", md_text)
        self.assertIn("🔬", md_text)


if __name__ == "__main__":
    unittest.main()
