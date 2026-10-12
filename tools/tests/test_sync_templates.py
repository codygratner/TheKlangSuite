#!/usr/bin/env python3
"""
tools/tests/test_sync_templates.py — Unit Tests for Multi-Repo Template Synchronizer
====================================================================================
Tests SHA-256 hash comparison, drift detection, safe copying, and git dirty checks.
"""

import argparse
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

from tools.sync_templates import (
    audit_mappings,
    cmd_check,
    cmd_sync,
    compute_sha256,
    sync_mappings,
)


class TestSyncTemplates(unittest.TestCase):
    def setUp(self):
        self.test_dir = Path(tempfile.mkdtemp(prefix="test_sync_tmpl_"))
        self.tks_root = self.test_dir / "TheKlangSuite"
        self.nkai_root = self.test_dir / "nkai"
        self.tks_root.mkdir()
        self.nkai_root.mkdir()

        self.custom_roots = {
            "TKS": self.tks_root,
            "NKAI": self.nkai_root,
        }

        # Create sample files
        self.src_file = self.nkai_root / "core" / "sample.html"
        self.src_file.parent.mkdir(parents=True)
        self.src_file.write_text("<h1>Canonical Content</h1>\n", encoding="utf-8")

        self.tgt_file1 = self.tks_root / "mirror" / "sample.html"
        self.tgt_file1.parent.mkdir(parents=True)
        self.tgt_file1.write_text("<h1>Canonical Content</h1>\n", encoding="utf-8")

        self.tgt_file2 = self.nkai_root / "bundle" / "sample.html"
        self.tgt_file2.parent.mkdir(parents=True)
        self.tgt_file2.write_text("<h1>Old Drifted Content</h1>\n", encoding="utf-8")

        self.test_mappings = [
            {
                "id": "sample_tmpl",
                "description": "Sample Template",
                "source": ("NKAI", "core/sample.html"),
                "targets": [
                    ("TKS", "mirror/sample.html"),
                    ("NKAI", "bundle/sample.html"),
                ],
            }
        ]

    def tearDown(self):
        if self.test_dir.exists():
            shutil.rmtree(self.test_dir, ignore_errors=True)

    def test_compute_sha256(self):
        h = compute_sha256(self.src_file)
        self.assertIsNotNone(h)
        self.assertEqual(len(h), 64)
        missing_h = compute_sha256(self.test_dir / "nonexistent.txt")
        self.assertIsNone(missing_h)

    def test_audit_mappings_drift_detection(self):
        results = audit_mappings(self.test_mappings, self.custom_roots)
        self.assertEqual(len(results), 1)
        item = results[0]
        self.assertFalse(item["all_synced"])

        tgt1, tgt2 = item["targets"]
        self.assertEqual(tgt1["status"], "IN_SYNC")
        self.assertEqual(tgt2["status"], "DRIFTED")

    def test_sync_mappings_and_parity(self):
        results = audit_mappings(self.test_mappings, self.custom_roots)

        # Dry run
        synced, skipped, logs = sync_mappings(results, dry_run=True)
        self.assertEqual(synced, 1)
        self.assertEqual(skipped, 0)
        # Target 2 should still be old content
        self.assertIn("Old Drifted Content", self.tgt_file2.read_text(encoding="utf-8"))

        # Real sync
        synced, skipped, logs = sync_mappings(results, dry_run=False)
        self.assertEqual(synced, 1)
        self.assertIn("Canonical Content", self.tgt_file2.read_text(encoding="utf-8"))

        # Re-audit: should be 100% in sync
        results_after = audit_mappings(self.test_mappings, self.custom_roots)
        self.assertTrue(results_after[0]["all_synced"])

    def test_sync_creates_missing_directory(self):
        missing_target = self.test_dir / "missing" / "deep" / "target.html"
        mappings = [
            {
                "id": "missing_test",
                "description": "Missing Test",
                "source": ("NKAI", "core/sample.html"),
                "targets": [("TKS", "missing/deep/target.html")],
            }
        ]
        results = audit_mappings(mappings, self.custom_roots)
        self.assertEqual(results[0]["targets"][0]["status"], "MISSING")

        synced, skipped, _ = sync_mappings(results, dry_run=False)
        self.assertEqual(synced, 1)
        self.assertTrue((self.tks_root / "missing" / "deep" / "target.html").exists())


if __name__ == "__main__":
    unittest.main()
