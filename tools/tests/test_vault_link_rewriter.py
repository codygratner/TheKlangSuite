#!/usr/bin/env python3
"""
tools/tests/test_vault_link_rewriter.py — Unit tests for Vault Link Rewriter & Underscore Naming (Flight 23)
=========================================================================================================
"""

from pathlib import Path
import shutil
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

from tools.vault_link_rewriter import (
    rewrite_link_target,
    rewrite_markdown_links_for_vault,
)
from tools.check_mail import extract_note_metadata


class TestVaultLinkRewriter(unittest.TestCase):
    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()
        self.tmp_path = Path(self.temp_dir.name)

    def tearDown(self):
        self.temp_dir.cleanup()

    def test_mirrored_docs_link_conversion(self):
        # TheKlangSuite/docs/
        link1 = "file:///c:/Dev/TheKlangSuite/docs/FOCUS.md"
        self.assertEqual(rewrite_link_target(link1), "Docs/FOCUS.md")

        # Nested docs spec
        link2 = "file:///c:/Dev/TheKlangSuite/docs/specs/dsp_spec.md"
        self.assertEqual(rewrite_link_target(link2), "Docs/specs/dsp_spec.md")

        # Root PLAN.md
        link3 = "file:///c:/Dev/TheKlangSuite/PLAN.md"
        self.assertEqual(rewrite_link_target(link3), "Docs/PLAN.md")

        # TheKlangVault/
        link4 = "file:///C:/Dev/TheKlangVault/Inbox/MyNote.md"
        self.assertEqual(rewrite_link_target(link4), "Inbox/MyNote.md")

        # TheKlangResearch/
        link5 = "file:///c:/Dev/TheKlangResearch/rfc/RFC_001.md"
        self.assertEqual(rewrite_link_target(link5), "Research/rfc/RFC_001.md")

    def test_line_anchor_stripping_and_heading_preservation(self):
        # Strips range anchor
        link_range = "file:///c:/Dev/TheKlangSuite/docs/FOCUS.md#L123-L145"
        self.assertEqual(rewrite_link_target(link_range), "Docs/FOCUS.md")

        # Strips single line anchor
        link_single = "file:///c:/Dev/TheKlangSuite/docs/FOCUS.md#L42"
        self.assertEqual(rewrite_link_target(link_single), "Docs/FOCUS.md")

        # Preserves markdown heading anchor
        link_heading = "file:///c:/Dev/TheKlangSuite/docs/FOCUS.md#active-focus"
        self.assertEqual(rewrite_link_target(link_heading), "Docs/FOCUS.md#active-focus")

    def test_native_code_preservation(self):
        # C++ files must NOT be rewritten to Docs/
        cpp_link = "file:///c:/Dev/TheKlangSuite/Source/dsp/Oscillator.cpp"
        self.assertEqual(rewrite_link_target(cpp_link), "file:///c:/Dev/TheKlangSuite/Source/dsp/Oscillator.cpp")

        # Header files
        h_link = "file:///c:/Dev/TheKlangSuite/Source/dsp/Oscillator.h"
        self.assertEqual(rewrite_link_target(h_link), "file:///c:/Dev/TheKlangSuite/Source/dsp/Oscillator.h")

        # JSON schema
        json_link = "file:///c:/Dev/TheKlangSuite/assets/controls/params.json"
        self.assertEqual(rewrite_link_target(json_link), "file:///c:/Dev/TheKlangSuite/assets/controls/params.json")

    def test_url_decoding_and_spaces(self):
        encoded_link = "file:///c:/Dev/TheKlangSuite/docs/My%20Awesome%20Doc.md"
        self.assertEqual(rewrite_link_target(encoded_link), "Docs/My Awesome Doc.md")

    def test_markdown_full_text_rewriting(self):
        input_text = """
Check our roadmap in [FOCUS](file:///c:/Dev/TheKlangSuite/docs/FOCUS.md#L10-L20) and review
the native DSP engine in [`Oscillator`](file:///c:/Dev/TheKlangSuite/Source/dsp/Oscillator.cpp).
Also see [Subdoc](file:///c:/Dev/TheKlangSuite/docs/specs/ui_spec.md#header-anchor).
"""
        expected = """
Check our roadmap in [FOCUS](Docs/FOCUS.md) and review
the native DSP engine in [`Oscillator`](file:///c:/Dev/TheKlangSuite/Source/dsp/Oscillator.cpp).
Also see [Subdoc](Docs/specs/ui_spec.md#header-anchor).
"""
        result = rewrite_markdown_links_for_vault(input_text)
        self.assertEqual(result, expected)

    def test_underscore_naming_title_resolution(self):
        # Create a note named with underscores in archive format without explicit title in frontmatter
        test_file = self.tmp_path / "My_Synthesizer_Architecture_Idea.md"
        test_file.write_text("# My Synthesizer Architecture Idea\n\nSome body text.", encoding="utf-8")
        meta = extract_note_metadata(test_file)
        self.assertEqual(meta["title"], "My Synthesizer Architecture Idea")

        # Test note with only file stem fallback (no heading, no frontmatter)
        quick_file = self.tmp_path / "Quick_Note_With_Underscores.md"
        quick_file.write_text("", encoding="utf-8")
        meta_quick = extract_note_metadata(quick_file)
        self.assertEqual(meta_quick["title"], "Quick Note With Underscores")


if __name__ == "__main__":
    unittest.main()
