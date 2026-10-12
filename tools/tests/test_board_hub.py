#!/usr/bin/env python3
"""
Unit and Integration Tests for N'kai Multi-Board Hub & Session Manager (Flight 14 / rj-05)
Asserts BoardRegistry data models, cache-busting headers in sidecar backend,
modal switcher markup in master_template.html, and board_registry.py CLI functionality.
"""

from pathlib import Path
import subprocess
import sys
import unittest

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.models import BoardMetadata, BoardRegistry


class TestBoardHubAndSessionManager(unittest.TestCase):
    def setUp(self):
        self.repo_root = REPO_ROOT
        self.registry_path = self.repo_root / ".agents" / "pipeline" / "boards" / "registry.json"
        self.cli_path = self.repo_root / "tools" / "board_registry.py"
        self.main_mjs_path = Path(r"c:\Dev\nkai\sidecars\panel\main.mjs")
        self.template_path = Path(r"c:\Dev\nkai\core\master_template.html")
        self.rendered_sandbox_path = self.repo_root / ".agents" / "sidecar" / "sandbox_mail_inbox.html"
        self.rendered_morning_path = self.repo_root / ".agents" / "sidecar" / "morning_mail_inbox.html"

    def test_board_registry_schema_and_methods(self):
        """Test 1: Asserts BoardRegistry Pydantic model parses, validates, and filters boards correctly."""
        self.assertTrue(self.registry_path.exists(), f"registry.json missing at {self.registry_path}")
        raw_json = self.registry_path.read_text(encoding="utf-8")
        registry = BoardRegistry.model_validate_json(raw_json)

        self.assertEqual(registry.version, "1.0.0")
        self.assertEqual(registry.default_board_id, "morning_mail")
        self.assertGreaterEqual(len(registry.boards), 3)

        # Method get_board
        morning = registry.get_board("morning_mail")
        self.assertIsNotNone(morning)
        self.assertEqual(morning.category, "active")
        self.assertEqual(morning.environment, "PRODUCTION")

        sandbox = registry.get_board("sandbox_mail")
        self.assertIsNotNone(sandbox)
        self.assertEqual(sandbox.category, "sandbox")
        self.assertEqual(sandbox.environment, "SANDBOX")
        self.assertEqual(sandbox.card_count, 7)

        runway = registry.get_board("runway")
        self.assertIsNotNone(runway)
        self.assertEqual(runway.category, "active")
        self.assertEqual(runway.board_type, "evergreen")

        audit = registry.get_board("system_sanity_audit")
        self.assertIsNotNone(audit)
        self.assertEqual(audit.category, "archived")

        nonexistent = registry.get_board("nonexistent_id")
        self.assertIsNone(nonexistent)

        # Method active_boards, staging_boards, archived_boards, sandbox_boards
        actives = registry.active_boards()
        stagings = registry.staging_boards()
        archiveds = registry.archived_boards()
        sandboxes = registry.sandbox_boards()
        self.assertTrue(any(b.id == "morning_mail" for b in actives))
        self.assertTrue(any(b.id == "runway" for b in actives))
        self.assertTrue(any(b.id == "sandbox_mail" for b in sandboxes))
        self.assertTrue(any(b.id == "system_sanity_audit" for b in archiveds))
        self.assertEqual(len(actives) + len(stagings) + len(archiveds) + len(sandboxes), len(registry.boards))

    def test_sidecar_backend_cache_invalidation_and_routing(self):
        """Test 2: Asserts sidecars/panel/main.mjs defines strict anti-cache headers and board endpoints."""
        self.assertTrue(self.main_mjs_path.exists(), f"main.mjs missing at {self.main_mjs_path}")
        content = self.main_mjs_path.read_text(encoding="utf-8")

        # Strict Anti-Cache Headers
        self.assertIn("Cache-Control", content)
        self.assertIn("no-store, no-cache, must-revalidate, max-age=0", content)
        self.assertIn("Pragma", content)
        self.assertIn("Expires", content)

        # /api/boards endpoint
        self.assertIn("/api/boards", content)
        self.assertIn("loadBoardRegistry", content)

        # Query param ?board=<id> routing
        self.assertIn("extractBoardParam", content)
        self.assertIn("resolveBoardFile", content)

    def test_master_template_board_switcher_ui(self):
        """Test 3: Asserts master_template.html contains interactive switcher button and modal elements."""
        self.assertTrue(self.template_path.exists(), f"master_template.html missing at {self.template_path}")
        content = self.template_path.read_text(encoding="utf-8")

        # Interactive Header Switcher Trigger
        self.assertIn('id="btn-board-switcher"', content)
        self.assertIn('class="board-switcher-btn"', content)
        self.assertIn("toggleBoardSwitcherModal()", content)
        self.assertIn('id="current-board-title"', content)

        # Accessible Modal Dialog with 4 Category Sections
        self.assertIn('id="board-switcher-modal"', content)
        self.assertIn('class="board-switcher-modal"', content)
        self.assertIn('id="board-search-input"', content)
        self.assertIn('id="toggle-show-sandbox"', content)
        self.assertIn('id="active-boards-list"', content)
        self.assertIn('id="staging-boards-list"', content)
        self.assertIn('id="archived-boards-list"', content)
        self.assertIn('id="sandbox-boards-list"', content)
        self.assertIn('id="active-boards-count"', content)
        self.assertIn('id="staging-boards-count"', content)
        self.assertIn('id="archived-boards-count"', content)
        self.assertIn('id="sandbox-boards-count"', content)

        # Embedded Registry Data Tag
        self.assertIn('id="nkai-board-registry-data"', content)
        self.assertIn('type="application/json"', content)

        # Controller Script
        self.assertIn("initBoardRegistry", content)
        self.assertIn("switchBoard", content)
        self.assertIn("filterBoardList", content)
        self.assertIn("handleToggleSandbox", content)
        self.assertIn("isSandboxVisible", content)

    def test_rendered_board_interpolation(self):
        """Test 4: Asserts rendered boards interpolate BOARD_TITLE and BOARD_REGISTRY_JSON."""
        self.assertTrue(self.rendered_sandbox_path.exists(), "sandbox_mail_inbox.html missing")
        sbx_content = self.rendered_sandbox_path.read_text(encoding="utf-8")
        self.assertNotIn("{{BOARD_TITLE}}", sbx_content, "Unsubstituted {{BOARD_TITLE}} found in sandbox board")
        self.assertNotIn("{{BOARD_REGISTRY_JSON}}", sbx_content, "Unsubstituted {{BOARD_REGISTRY_JSON}} found in sandbox board")
        self.assertNotIn("{{SOURCE_PATH}}", sbx_content, "Unsubstituted {{SOURCE_PATH}} found in sandbox board")
        self.assertNotIn("{inbox_dir_esc}", sbx_content, "Unsubstituted {inbox_dir_esc} found in sandbox board")
        self.assertNotIn('Number("{{', sbx_content, "Unsubstituted Number('{{') expression found in sandbox board")
        self.assertIn('id="btn-board-switcher"', sbx_content)
        self.assertIn('id="header-source-path"', sbx_content)
        self.assertIn("Canonical QA Sandbox Test Board", sbx_content)

        self.assertTrue(self.rendered_morning_path.exists(), "morning_mail_inbox.html missing")
        morn_content = self.rendered_morning_path.read_text(encoding="utf-8")
        self.assertNotIn("{{BOARD_TITLE}}", morn_content, "Unsubstituted {{BOARD_TITLE}} found in morning board")
        self.assertNotIn("{{BOARD_REGISTRY_JSON}}", morn_content, "Unsubstituted {{BOARD_REGISTRY_JSON}} found in morning board")
        self.assertNotIn("{{SOURCE_PATH}}", morn_content, "Unsubstituted {{SOURCE_PATH}} found in morning board")
        self.assertNotIn("{inbox_dir_esc}", morn_content, "Unsubstituted {inbox_dir_esc} found in morning board")
        self.assertNotIn('Number("{{', morn_content, "Unsubstituted Number('{{') expression found in morning board")
        self.assertIn('id="btn-board-switcher"', morn_content)
        self.assertIn('id="header-source-path"', morn_content)

    def test_board_registry_cli_execution(self):
        """Test 5: Asserts board_registry.py CLI commands execute successfully and output valid data."""
        self.assertTrue(self.cli_path.exists(), f"board_registry.py missing at {self.cli_path}")

        # Test list command
        cmd = [sys.executable, str(self.cli_path), "list"]
        res = subprocess.run(cmd, cwd=str(self.repo_root), capture_output=True, text=True, encoding="utf-8")
        self.assertEqual(res.returncode, 0, f"board_registry.py list failed:\n{res.stderr}\n{res.stdout}")
        self.assertIn("N'kai Multi-Board Hub Registry", res.stdout)
        self.assertIn("morning_mail", res.stdout)
        self.assertIn("sandbox_mail", res.stdout)
        self.assertIn("system_sanity_audit", res.stdout)

    def test_switch_board_omits_query_string_on_file_protocol(self):
        """Test 6 (Regression): Asserts switchBoard omits ?_t= on file:// to prevent Windows Chromium 404s."""
        files_to_check = [
            self.template_path,
            Path(r"c:\Dev\nkai\core\sidecar.js"),
            self.rendered_sandbox_path,
            self.rendered_morning_path,
        ]
        for fpath in files_to_check:
            self.assertTrue(fpath.exists(), f"Target file missing: {fpath}")
            text = fpath.read_text(encoding="utf-8")
            self.assertIn("function switchBoard", text, f"switchBoard function missing in {fpath.name}")
            # Assert file branch uses bare targetFile
            self.assertIn("window.location.href = targetFile;", text,
                          f"Regression detected in {fpath.name}: file protocol must navigate strictly to bare targetFile without query strings")
            # Assert isServer check exists
            self.assertIn("isServer", text, f"Missing isServer protocol check in {fpath.name}")

    def test_sidecar_main_mjs_routes_all_registry_boards(self):
        """Test 7 (Regression): Asserts main.mjs registers explicit routes for every board in registry.json."""
        self.assertTrue(self.registry_path.exists(), "registry.json missing")
        self.assertTrue(self.main_mjs_path.exists(), "main.mjs missing")

        raw_json = self.registry_path.read_text(encoding="utf-8")
        registry = BoardRegistry.model_validate_json(raw_json)
        main_mjs_content = self.main_mjs_path.read_text(encoding="utf-8")

        for board in registry.boards:
            target_filename = board.file_path.split("/")[-1]
            route_str = f"/{target_filename}"
            self.assertIn(route_str, main_mjs_content,
                          f"Board '{board.id}' ({target_filename}) lacks an explicit route in main.mjs, risking 404s in SidecarApp")

    def test_board_files_mirrored_across_nkai_and_tks(self):
        """Test 8 (Regression): Asserts all boards in registry.json exist in both TKS and nkai sidecars/panel."""
        raw_json = self.registry_path.read_text(encoding="utf-8")
        registry = BoardRegistry.model_validate_json(raw_json)
        nkai_panel_dir = Path(r"c:\Dev\nkai\sidecars\panel")
        tks_sidecar_dir = self.repo_root / ".agents" / "sidecar"

        for board in registry.boards:
            target_filename = board.file_path.split("/")[-1]
            tks_file = tks_sidecar_dir / target_filename
            nkai_file = nkai_panel_dir / target_filename
            self.assertTrue(tks_file.exists(), f"Board file '{target_filename}' missing in TKS sidecar dir: {tks_file}")
            self.assertTrue(nkai_file.exists(), f"Board file '{target_filename}' missing in nkai panel dir: {nkai_file}")


if __name__ == "__main__":
    unittest.main()

