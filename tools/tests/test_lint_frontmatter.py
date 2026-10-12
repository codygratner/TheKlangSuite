#!/usr/bin/env python3
"""
tools/tests/test_lint_frontmatter.py — Unit Tests for Universal Frontmatter Auditor
===================================================================================
Tests frontmatter detection, key-alias resolution, safe backfilling, tag normalization,
idempotency, and zero body corruption.
"""

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

from tools.lint_frontmatter import (
    collect_markdown_files,
    fix_file,
    lint_file,
    parse_frontmatter,
    parse_yaml_lines,
)


class TestLintFrontmatter(unittest.TestCase):
    def setUp(self):
        self.test_dir = Path(tempfile.mkdtemp(prefix="test_lint_fm_"))

    def tearDown(self):
        if self.test_dir.exists():
            shutil.rmtree(self.test_dir, ignore_errors=True)

    def test_parse_yaml_lines_scalars_and_lists(self):
        lines = [
            'title: "My Great Note"',
            "date: 2026-10-12",
            "tags: [tag1, tag2, tag3]",
            "status: active",
            "# Comment line",
            "",
            "extra_list:",
            "  - itemA",
            "  - itemB",
        ]
        meta = parse_yaml_lines(lines)
        self.assertEqual(meta["title"], "My Great Note")
        self.assertEqual(meta["date"], "2026-10-12")
        self.assertEqual(meta["tags"], ["tag1", "tag2", "tag3"])
        self.assertEqual(meta["status"], "active")
        self.assertEqual(meta["extra_list"], ["itemA", "itemB"])

    def test_lint_file_compliant(self):
        note = self.test_dir / "compliant.md"
        note.write_text(
            """---
title: "Compliant Document"
date: "2026-10-12"
tags: [test, docs]
status: "active"
---

# Compliant Document

This document is fully compliant.
""",
            encoding="utf-8",
        )
        info = lint_file(note, required_keys=["title", "date", "tags", "status"])
        self.assertEqual(info["status"], "COMPLIANT")
        self.assertEqual(len(info["missing_keys"]), 0)
        self.assertTrue(info["has_frontmatter"])

    def test_lint_file_with_aliases(self):
        # uses id instead of title, created instead of date, type instead of status
        note = self.test_dir / "aliases.md"
        note.write_text(
            """---
id: "custom_id"
created: "2026-10-10"
categories: [specs]
type: "RFC"
---

# Title In Body
Body content here.
""",
            encoding="utf-8",
        )
        info = lint_file(note, required_keys=["title", "date", "tags", "status"])
        self.assertEqual(info["status"], "COMPLIANT")

    def test_lint_file_missing_frontmatter_and_fix(self):
        note = self.test_dir / "no_fm.md"
        body_text = "# The Great Journey\n\nHere is the body text that must never be corrupted.\n\n- Bullet 1\n- Bullet 2\n"
        note.write_text(body_text, encoding="utf-8")

        info = lint_file(note, required_keys=["title", "date", "tags", "status"])
        self.assertEqual(info["status"], "NON_COMPLIANT")
        self.assertFalse(info["has_frontmatter"])
        self.assertEqual(set(info["missing_keys"]), {"title", "date", "tags", "status"})

        # Dry run should not modify file
        changed, msg = fix_file(info, default_status="untriaged", dry_run=True)
        self.assertTrue(changed)
        self.assertIn("[DRY-RUN]", msg)
        self.assertEqual(note.read_text(encoding="utf-8"), body_text)

        # Real fix
        changed, msg = fix_file(info, default_status="untriaged", dry_run=False)
        self.assertTrue(changed)

        fixed_content = note.read_text(encoding="utf-8")
        self.assertTrue(fixed_content.startswith("---"))
        self.assertIn('title: "The Great Journey"', fixed_content)
        self.assertIn("tags: [untriaged]", fixed_content)
        self.assertIn('status: "untriaged"', fixed_content)
        # Verify body is completely preserved
        self.assertIn(body_text, fixed_content)

        # Verify idempotency
        info2 = lint_file(note, required_keys=["title", "date", "tags", "status"])
        self.assertEqual(info2["status"], "COMPLIANT")
        changed2, _ = fix_file(info2, dry_run=False)
        self.assertFalse(changed2)

    def test_lint_file_partial_frontmatter_and_fix(self):
        note = self.test_dir / "partial.md"
        content = """---
title: "Existing Title"
custom_field: "preserve_me"
---

# Existing Title
Body content.
"""
        note.write_text(content, encoding="utf-8")

        info = lint_file(note, required_keys=["title", "date", "tags", "status"])
        self.assertEqual(info["status"], "NON_COMPLIANT")
        self.assertTrue(info["has_frontmatter"])
        self.assertEqual(set(info["missing_keys"]), {"date", "tags", "status"})

        changed, msg = fix_file(info, default_status="active", dry_run=False)
        self.assertTrue(changed)

        fixed_content = note.read_text(encoding="utf-8")
        self.assertIn('custom_field: "preserve_me"', fixed_content)
        self.assertIn('title: "Existing Title"', fixed_content)
        self.assertIn('status: "active"', fixed_content)
        self.assertIn("tags: [untriaged]", fixed_content)
        self.assertIn("Body content.", fixed_content)

        # Re-lint must be compliant
        info2 = lint_file(note, required_keys=["title", "date", "tags", "status"])
        self.assertEqual(info2["status"], "COMPLIANT")

    def test_tag_normalization(self):
        note = self.test_dir / "tag_norm.md"
        content = """---
title: "Normalizing Tags"
date: "2026-10-12"
status: "active"
tags: "tag1, tag2, tag3"
---

Body.
"""
        note.write_text(content, encoding="utf-8")

        info = lint_file(note, required_keys=["title", "date", "tags", "status"])
        self.assertEqual(info["status"], "NON_COMPLIANT")
        self.assertTrue(info["needs_tag_normalization"])

        changed, msg = fix_file(info, dry_run=False)
        self.assertTrue(changed)

        fixed_content = note.read_text(encoding="utf-8")
        self.assertIn("tags: [tag1, tag2, tag3]", fixed_content)

        info2 = lint_file(note, required_keys=["title", "date", "tags", "status"])
        self.assertEqual(info2["status"], "COMPLIANT")

    def test_collect_markdown_files_excludes(self):
        (self.test_dir / "allowed.md").write_text("allowed", encoding="utf-8")
        sub = self.test_dir / "subdir"
        sub.mkdir()
        (sub / "nested.md").write_text("nested", encoding="utf-8")

        exc = self.test_dir / ".obsidian"
        exc.mkdir()
        (exc / "ignored.md").write_text("ignored", encoding="utf-8")

        files = collect_markdown_files(self.test_dir, excludes={".obsidian"})
        names = [f.name for f in files]
        self.assertIn("allowed.md", names)
        self.assertIn("nested.md", names)
        self.assertNotIn("ignored.md", names)


if __name__ == "__main__":
    unittest.main()
