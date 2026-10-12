#!/usr/bin/env python3
"""
tools/tests/test_bus_locks.py — Unit Tests for tools/bus.py Intent Locks
=======================================================================
Part of The Zero-Token Mechanical Automation Suite (Flight 1B).
"""

import argparse
import datetime
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

from tools import bus


class TestBusLocks(unittest.TestCase):
    def setUp(self):
        self.test_dir = Path(tempfile.mkdtemp(prefix="test_bus_locks_"))
        self.dispatches_dir = self.test_dir / "Dispatches"
        self.dispatches_dir.mkdir(parents=True, exist_ok=True)

        self.orig_candidate_paths = bus.CANDIDATE_BUS_PATHS
        bus.CANDIDATE_BUS_PATHS = [self.dispatches_dir]

    def tearDown(self):
        bus.CANDIDATE_BUS_PATHS = self.orig_candidate_paths
        shutil.rmtree(self.test_dir, ignore_errors=True)

    def test_lock_and_unlock_lifecycle(self):
        # 1. Acquire lock
        args = argparse.Namespace(
            pillar="COSMOS",
            scope="TheKlangVault/Docs,TheKlangVault/Inbox",
            activity="Tag audit",
            ttl=15,
        )
        rc = bus.cmd_lock(args)
        self.assertEqual(rc, 0)

        # 2. Check lock exists
        active = bus.load_active_locks(self.dispatches_dir)
        self.assertEqual(len(active), 1)
        self.assertEqual(active[0]["pillar"], "COSMOS")
        self.assertEqual(active[0]["activity"], "Tag audit")
        self.assertIn("theklangvault/docs", [s.lower() for s in active[0]["scope"]])

        # 3. Check scope collision for another pillar
        coll_args = argparse.Namespace(
            pillar="TKS",
            scope="TheKlangVault/Docs/spec.md",
            quiet=True,
        )
        coll_rc = bus.cmd_check_scope(coll_args)
        self.assertEqual(coll_rc, 1)  # Collision!

        # 4. Check clear scope
        clear_args = argparse.Namespace(
            pillar="TKS",
            scope="Source/dsp/Oscillator.cpp",
            quiet=True,
        )
        clear_rc = bus.cmd_check_scope(clear_args)
        self.assertEqual(clear_rc, 0)  # Clear!

        # 5. Check own pillar ignores own lock
        own_args = argparse.Namespace(
            pillar="COSMOS",
            scope="TheKlangVault/Docs/spec.md",
            quiet=True,
        )
        own_rc = bus.cmd_check_scope(own_args)
        self.assertEqual(own_rc, 0)  # Self-lock ignored!

        # 6. Release lock
        unlock_args = argparse.Namespace(pillar="COSMOS")
        un_rc = bus.cmd_unlock(unlock_args)
        self.assertEqual(un_rc, 0)

        active_after = bus.load_active_locks(self.dispatches_dir)
        self.assertEqual(len(active_after), 0)

    def test_stale_lock_ttl_expiration(self):
        locks_dir = bus.get_locks_dir(self.dispatches_dir)
        # Create an expired lock file
        past = (datetime.datetime.now() - datetime.timedelta(minutes=10)).isoformat()
        expired_data = {
            "pillar": "TT",
            "activity": "Old tracker task",
            "scope": ["ToadTracker"],
            "started_at": past,
            "ttl_minutes": 5,
            "expires_at": past,
        }
        lock_file = locks_dir / "tt_lock.json"
        lock_file.write_text(json.dumps(expired_data), encoding="utf-8")

        # Loading locks should prune the expired lock
        active = bus.load_active_locks(self.dispatches_dir)
        self.assertEqual(len(active), 0)
        self.assertFalse(lock_file.exists())  # Auto-deleted!


if __name__ == "__main__":
    unittest.main()
