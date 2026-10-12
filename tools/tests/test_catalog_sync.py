#!/usr/bin/env python3
"""
tools/tests/test_catalog_sync.py — Unit Tests for tools/catalog_sync.py
=====================================================================
Part of The Zero-Token Mechanical Automation Suite (Flight 1).
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

from tools import catalog_sync


class TestCatalogSync(unittest.TestCase):
    def setUp(self):
        self.test_dir = Path(tempfile.mkdtemp(prefix="test_catalog_sync_"))
        self.docs_dir = self.test_dir / "docs"
        self.specs_dir = self.docs_dir / "specs"
        self.specs_dir.mkdir(parents=True, exist_ok=True)

        # Mock catalog
        self.catalog_path = self.docs_dir / "DOCS_CATALOG.json"
        self.initial_catalog = {
            "$schema": "http://json-schema.org/draft-07/schema#",
            "version": "1.0.0",
            "catalogs": {
                "technical_specifications": {
                    "description": "Tech specs",
                    "items": [
                        {
                            "path": "docs/specs/existing_spec.md",
                            "title": "Existing Spec",
                            "purpose": "A test spec."
                        }
                    ]
                }
            }
        }
        self.catalog_path.write_text(json.dumps(self.initial_catalog, indent=2), encoding="utf-8")

        # Create existing spec file
        (self.specs_dir / "existing_spec.md").write_text("# Existing Spec\n> A test spec.", encoding="utf-8")

        # Point catalog_sync module paths to test dir
        self.orig_root = catalog_sync.REPO_ROOT
        self.orig_catalog = catalog_sync.CATALOG_PATH
        catalog_sync.REPO_ROOT = self.test_dir
        catalog_sync.CATALOG_PATH = self.catalog_path

    def tearDown(self):
        catalog_sync.REPO_ROOT = self.orig_root
        catalog_sync.CATALOG_PATH = self.orig_catalog
        shutil.rmtree(self.test_dir, ignore_errors=True)

    def test_extract_metadata_with_frontmatter(self):
        f = self.specs_dir / "fm_test.md"
        f.write_text("---\ntitle: 'FM Title'\npurpose: 'FM Purpose'\n---\n# Body Title", encoding="utf-8")
        title, purpose = catalog_sync.extract_metadata_from_markdown(f)
        self.assertEqual(title, "FM Title")
        self.assertEqual(purpose, "FM Purpose")

    def test_extract_metadata_with_h1_and_blockquote(self):
        f = self.specs_dir / "h1_test.md"
        f.write_text("# Markdown H1 Title\n> Architectural design statement.", encoding="utf-8")
        title, purpose = catalog_sync.extract_metadata_from_markdown(f)
        self.assertEqual(title, "Markdown H1 Title")
        self.assertEqual(purpose, "Architectural design statement.")

    def test_audit_detects_uncataloged_spec(self):
        # Create new uncataloged spec
        new_spec = self.specs_dir / "new_feature.md"
        new_spec.write_text("# New Feature Spec\n> Handles cool audio stuff.", encoding="utf-8")

        res = catalog_sync.audit_catalog(self.test_dir)
        self.assertFalse(res["is_clean"])
        self.assertEqual(len(res["uncataloged_specs"]), 1)
        self.assertEqual(res["uncataloged_specs"][0]["path"], "docs/specs/new_feature.md")
        self.assertEqual(res["uncataloged_specs"][0]["title"], "New Feature Spec")

    def test_audit_detects_dead_link(self):
        # Delete existing spec file
        (self.specs_dir / "existing_spec.md").unlink()

        res = catalog_sync.audit_catalog(self.test_dir)
        self.assertFalse(res["is_clean"])
        self.assertEqual(len(res["dead_links"]), 1)
        self.assertEqual(res["dead_links"][0]["path"], "docs/specs/existing_spec.md")

    def test_sync_fixes_catalog(self):
        # 1. Add uncataloged file
        (self.specs_dir / "auto_spec.md").write_text("# Auto Spec\n> Automated test.", encoding="utf-8")
        # 2. Delete existing file to create dead link
        (self.specs_dir / "existing_spec.md").unlink()

        sync_res = catalog_sync.sync_catalog(self.test_dir)
        self.assertTrue(sync_res["modified"])
        self.assertEqual(sync_res["added"], 1)
        self.assertEqual(sync_res["pruned"], 1)

        # Audit should now be 100% clean
        after_audit = catalog_sync.audit_catalog(self.test_dir)
        self.assertTrue(after_audit["is_clean"])


if __name__ == "__main__":
    unittest.main()
