import os
import sys
import unittest
from pathlib import Path
import tempfile
import shutil

# Ensure UTF-8 console output for Windows terminal safety
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(REPO_ROOT))

from tools import bus


class TestInterProjectBus(unittest.TestCase):
    def setUp(self):
        self.test_dir = Path(tempfile.mkdtemp(prefix="test_bus_"))
        self.orig_paths = bus.CANDIDATE_BUS_PATHS
        bus.CANDIDATE_BUS_PATHS = [self.test_dir]

    def tearDown(self):
        bus.CANDIDATE_BUS_PATHS = self.orig_paths
        shutil.rmtree(self.test_dir, ignore_errors=True)

    def test_post_and_list_dispatch(self):
        class Args:
            to = "TT"
            title = "Test Handoff To TT"
            from_pillar = "TKS"
            body = "Handshake payload content"
            tags = "dsp,vst3"
            priority = "P1"
            payload_file = None

        ret = bus.cmd_post(Args())
        self.assertEqual(ret, 0)

        dispatches = bus.load_dispatches(self.test_dir)
        self.assertEqual(len(dispatches), 1)
        d = dispatches[0]
        self.assertEqual(d["from"], "TKS")
        self.assertEqual(d["to"], "TT")
        self.assertIn("vst3", d["tags"])
        self.assertIn("dsp", d["tags"])
        self.assertEqual(d["status"], "PENDING")

    def test_ack_and_archive_dispatch(self):
        class PostArgs:
            to = "NK"
            title = "Board Sync"
            from_pillar = "COSMOS"
            body = "Syncing template"
            tags = "nk,ui"
            priority = "P2"
            payload_file = None

        bus.cmd_post(PostArgs())
        dispatches = bus.load_dispatches(self.test_dir)
        d_id = dispatches[0]["id"]

        class AckArgs:
            id = d_id
            archive = True

        ret = bus.cmd_ack(AckArgs())
        self.assertEqual(ret, 0)

        # Pending should now be empty
        active = bus.load_dispatches(self.test_dir, include_archived=False)
        self.assertEqual(len(active), 0)

        # Archived should contain 1
        all_d = bus.load_dispatches(self.test_dir, include_archived=True)
        self.assertEqual(len(all_d), 1)
        self.assertEqual(all_d[0]["status"], "ACKNOWLEDGED")
        self.assertTrue(all_d[0]["_is_archived"])


if __name__ == "__main__":
    unittest.main()
