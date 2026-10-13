#!/usr/bin/env python3
"""
tools/tests/test_sentry_engine.py — Unit tests for AGY SDK Sentry Engine
========================================================================
"""

import asyncio
import json
import os
import sys
import tempfile
import unittest
from pathlib import Path

# Ensure UTF-8 console output for Windows terminal safety
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

import tools.sentry_engine as sentry_engine
from tools.sentry_engine import (
    vault_search,
    vault_read,
    stage_quick_note,
    stage_dispatch,
    stream_sentry_response,
    SENTRY_TOOLS,
)


class TestSentryEngine(unittest.TestCase):
    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()
        self.test_vault = Path(self.temp_dir.name)
        # Point VAULT_ROOT directly to test_vault
        self.orig_vault_root = sentry_engine.VAULT_ROOT
        sentry_engine.VAULT_ROOT = self.test_vault

        # Create a mock note in test vault
        (self.test_vault / "Inbox").mkdir(parents=True, exist_ok=True)
        (self.test_vault / "Inbox" / "test_note.md").write_text(
            "# Synth Architecture\nThis is a note about Vital and JUCE audio engines.",
            encoding="utf-8"
        )

    def tearDown(self):
        sentry_engine.VAULT_ROOT = self.orig_vault_root
        self.temp_dir.cleanup()

    def test_sandboxed_tools_invariants(self):
        # Assert strictly scoped tool count and names
        tool_names = [t.__name__ for t in SENTRY_TOOLS]
        self.assertIn("vault_search", tool_names)
        self.assertIn("vault_read", tool_names)
        self.assertIn("gdrive_search", tool_names)
        self.assertIn("stage_quick_note", tool_names)
        self.assertIn("stage_dispatch", tool_names)

        # Assert no shell execution tools exist
        for forbidden in ["run_command", "exec", "eval", "bash", "cmd", "powershell", "shell"]:
            self.assertNotIn(forbidden, tool_names)

    def test_vault_search(self):
        res = vault_search("Vital")
        self.assertIn("test_note.md", res)
        self.assertIn("Vault search results", res)

    def test_vault_read_sandboxing(self):
        # Normal read
        content = vault_read("Inbox/test_note.md")
        self.assertIn("Synth Architecture", content)

        # Path traversal attempt should fail
        traversal = vault_read("../../../secret.txt")
        self.assertIn("Access denied", traversal)

    def test_stage_quick_note(self):
        quick_dir = self.test_vault / "Inbox" / "QuickNotes"

        msg = stage_quick_note("Idea for Polyphonic LFO", "Should use branchless wavetable scanning.")
        self.assertIn("Successfully staged quick note", msg)

        # Verify staged file exists
        files = list(quick_dir.glob("*.md"))
        self.assertEqual(len(files), 1)
        self.assertIn("Idea for Polyphonic LFO", files[0].read_text(encoding="utf-8"))

    def test_stage_dispatch(self):
        dispatch_dir = self.test_vault / "Dispatches" / "pending"

        msg = stage_dispatch("Triage Google Drive", '{"target": "audio_samples"}')
        self.assertIn("Successfully staged dispatch", msg)

        files = list(dispatch_dir.glob("*.json"))
        self.assertEqual(len(files), 1)
        data = json.loads(files[0].read_text(encoding="utf-8"))
        self.assertEqual(data["title"], "Triage Google Drive")
        self.assertEqual(data["status"], "pending_human_confirmation")

    def test_stream_sentry_response_mock(self):
        async def run_stream():
            events = []
            async for ev in stream_sentry_response("Hello from iPhone"):
                events.append(ev)
            return events

        events = asyncio.run(run_stream())
        types = [e["type"] for e in events]
        self.assertIn("thought", types)
        self.assertIn("token", types)
        self.assertIn("done", types)


if __name__ == "__main__":
    unittest.main()
