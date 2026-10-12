#!/usr/bin/env python3
"""
tools/tests/test_state_manager.py — Unit Tests for tools/state_manager.py
"""

import json
import sys
import unittest
from pathlib import Path

# Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

from tools import state_manager


class TestStateManager(unittest.TestCase):
    def test_git_state_returns_dict(self):
        state = state_manager.get_git_state()
        self.assertIn("branch", state)
        self.assertIn("commit", state)
        self.assertIn("is_clean", state)
        self.assertIsInstance(state["recent_commits"], list)

    def test_plan_state_parsing(self):
        plan = state_manager.parse_plan_state()
        self.assertIn("has_plan", plan)
        self.assertIn("title", plan)
        self.assertIn("target", plan)
        self.assertIn("status", plan)

    def test_communique_state_parsing(self):
        comm = state_manager.parse_communique_state()
        self.assertIn("factory_dispatch", comm)
        self.assertIn("factory_receipt", comm)
        self.assertIn("tooling_dispatch", comm)
        self.assertIn("tooling_receipt", comm)

    def test_render_active_context_markdown(self):
        full_state = state_manager.compute_workspace_state()
        md = state_manager.render_active_context_markdown(full_state)
        self.assertIn("# Active Workspace Context", md)
        self.assertIn(full_state["git"]["branch"], md)

    def test_render_console_card(self):
        full_state = state_manager.compute_workspace_state()
        card = state_manager.render_console_card(full_state)
        self.assertIn("BLACK BOX WORKSPACE STATE", card)
        self.assertIn("Branch:", card)

    def test_cmd_dump_default_args(self):
        import argparse
        parser = argparse.ArgumentParser()
        args = parser.parse_args([])
        res = state_manager.cmd_dump(args)
        self.assertEqual(res, 0)


if __name__ == "__main__":
    unittest.main()
