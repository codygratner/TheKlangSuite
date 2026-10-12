#!/usr/bin/env python3
"""
tools/tests/test_plan_manager.py — Unit Tests for 1-Command Plan Manager
=======================================================================
Tests phase checking, status parsing, receipt status updates, code-block
safety, and progress header calculation.
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

from tools.plan_manager import (
    cmd_advance,
    cmd_check,
    cmd_status,
    cmd_uncheck,
    parse_plan_sections,
    update_progress_header,
)

SAMPLE_PLAN = """# Implementation Plan: Test Suite Project

> **Target Pipeline:** `TARGET: TOOLING_HARNESS`  
> **Status:** `IN_PROGRESS` 🔨  
> **Progress:** `0/3 Flights Completed (0%)`  

---

### Flight 1: Alpha Step
- [ ] Task 1A: Setup
- [ ] Task 1B: Configure

```bash
# Example code snippet with checkboxes that should never change
- [ ] do not change this code block
```

### Flight 2: Beta Step
- [ ] Task 2A: Build feature
- [ ] Task 2B: Run test

### Flight 3: Gamma Step
- [ ] Task 3A: Finalize
"""


class TestPlanManager(unittest.TestCase):
    def setUp(self):
        self.test_dir = Path(tempfile.mkdtemp(prefix="test_plan_mgr_"))
        self.plan_path = self.test_dir / "PLAN.md"
        self.plan_path.write_text(SAMPLE_PLAN, encoding="utf-8")

        # Mock communique receipts
        self.comm_dir = self.test_dir / ".agents" / "pipeline" / "communique"
        self.comm_dir.mkdir(parents=True)
        self.receipt = self.comm_dir / "tooling_receipt.md"
        self.receipt.write_text("## Status: IDLE\n\nActive step: None.\n", encoding="utf-8")

    def tearDown(self):
        if self.test_dir.exists():
            shutil.rmtree(self.test_dir, ignore_errors=True)

    def test_parse_plan_sections(self):
        content = self.plan_path.read_text(encoding="utf-8")
        header, sections, footer = parse_plan_sections(content)
        self.assertEqual(len(sections), 3)
        self.assertEqual(sections[0]["phase_id"], "1")
        self.assertEqual(sections[0]["total_items"], 2)
        self.assertEqual(sections[0]["completed_items"], 0)
        self.assertFalse(sections[0]["is_complete"])

    def test_code_block_safety(self):
        # Check off Flight 1
        args = argparse.Namespace(plan=str(self.plan_path), phase="1", dry_run=False)
        self.assertEqual(cmd_check(args), 0)

        content = self.plan_path.read_text(encoding="utf-8")
        # Code block should remain intact with [ ] unchecked
        self.assertIn("- [ ] do not change this code block", content)
        # Real tasks should be checked
        self.assertIn("- [x] Task 1A: Setup", content)
        self.assertIn("- [x] Task 1B: Configure", content)
        # Header should reflect progress
        self.assertIn("> **Progress:** `1/3 Flights Completed (33%)`", content)

    def test_uncheck_phase(self):
        # Check then uncheck
        args_check = argparse.Namespace(plan=str(self.plan_path), phase="1", dry_run=False)
        cmd_check(args_check)

        args_uncheck = argparse.Namespace(plan=str(self.plan_path), phase="1", dry_run=False)
        self.assertEqual(cmd_uncheck(args_uncheck), 0)

        content = self.plan_path.read_text(encoding="utf-8")
        self.assertIn("- [ ] Task 1A: Setup", content)
        self.assertIn("> **Progress:** `0/3 Flights Completed (0%)`", content)

    def test_advance_and_receipt_updates(self):
        # Monkey patch communique dir to use test dir
        import tools.plan_manager as pm
        orig_comm = pm.get_communique_dir
        pm.get_communique_dir = lambda: self.comm_dir

        try:
            # Advance Flight 1
            args = argparse.Namespace(plan=str(self.plan_path), dry_run=False)
            self.assertEqual(cmd_advance(args), 0)

            content = self.plan_path.read_text(encoding="utf-8")
            self.assertIn("> **Progress:** `1/3 Flights Completed (33%)`", content)
            receipt_content = self.receipt.read_text(encoding="utf-8")
            self.assertIn("BUILDING 🔨 (Flight 2: Beta Step)", receipt_content)

            # Advance Flight 2
            self.assertEqual(cmd_advance(args), 0)
            receipt_content = self.receipt.read_text(encoding="utf-8")
            self.assertIn("BUILDING 🔨 (Flight 3: Gamma Step)", receipt_content)

            # Advance Flight 3 (final)
            self.assertEqual(cmd_advance(args), 0)
            final_plan = self.plan_path.read_text(encoding="utf-8")
            self.assertIn("> **Progress:** `3/3 Flights Completed (100%)`", final_plan)
            self.assertIn("> **Status:** `COMPLETED ✅`", final_plan)

            final_receipt = self.receipt.read_text(encoding="utf-8")
            self.assertIn("## Status: COMPLETED ✅", final_receipt)
        finally:
            pm.get_communique_dir = orig_comm


if __name__ == "__main__":
    unittest.main()
