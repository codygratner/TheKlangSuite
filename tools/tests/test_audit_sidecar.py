#!/usr/bin/env python3
"""
tools/tests/test_audit_sidecar.py — Unit Tests for N'kai Sidecar Web QA Linter
=============================================================================
Tests deterministic static analysis of HTML webviews: tag balance,
JS syntax hazards (unescaped \\u paths), overlay pointer-events, and
Windows file:// navigation safety (preventing Chromium ENOENT 404s).
"""

from pathlib import Path
import shutil
import sys
import tempfile
import unittest

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.audit_sidecar import SidecarAuditor


class TestAuditSidecar(unittest.TestCase):
    def setUp(self):
        self.test_dir = Path(tempfile.mkdtemp(prefix="test_audit_sidecar_"))

    def tearDown(self):
        if self.test_dir.exists():
            shutil.rmtree(self.test_dir, ignore_errors=True)

    def _write_temp_html(self, content: str) -> Path:
        fpath = self.test_dir / "test.html"
        fpath.write_text(content, encoding="utf-8")
        return fpath

    def test_clean_html_passes_with_zero_critical_issues(self):
        clean_html = """<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8"/>
  <title>Clean Board</title>
</head>
<body>
  <div class="container">
    <h1>Clean Board</h1>
  </div>
</body>
</html>"""
        fpath = self._write_temp_html(clean_html)
        auditor = SidecarAuditor(str(fpath))
        passed = auditor.run()
        self.assertTrue(passed)
        self.assertEqual(len(auditor.issues), 0)

    def test_detects_unclosed_tags(self):
        imbalanced_html = """<!DOCTYPE html>
<html>
<body>
  <div>
    <div>Unclosed inner div
  </div>
</body>
</html>"""
        fpath = self._write_temp_html(imbalanced_html)
        auditor = SidecarAuditor(str(fpath))
        auditor.audit_tag_balance()
        self.assertGreater(len(auditor.issues), 0)
        self.assertTrue(any("HTML Tag Imbalance: <div>" in issue for issue in auditor.issues))

    def test_detects_unescaped_unicode_hazard_in_script(self):
        hazardous_html = r"""<!DOCTYPE html>
<html>
<body>
  <script>
    const userPath = "C:\Users\codyg\AppData\Local";
  </script>
</body>
</html>"""
        fpath = self._write_temp_html(hazardous_html)
        auditor = SidecarAuditor(str(fpath))
        auditor.audit_js_syntax_and_escapes()
        self.assertGreater(len(auditor.issues), 0)
        self.assertTrue(any("Fatal JS Syntax Hazard" in issue for issue in auditor.issues))

    def test_detects_unshielded_file_query_strings(self):
        broken_nav_html = """<!DOCTYPE html>
<html>
<body>
  <script>
    function switchBoard(boardId) {
      let targetFile = 'morning_mail_inbox.html';
      window.location.href = `${targetFile}?_t=12345`;
    }
  </script>
</body>
</html>"""
        fpath = self._write_temp_html(broken_nav_html)
        auditor = SidecarAuditor(str(fpath))
        auditor.audit_local_file_navigation_safety()
        self.assertGreater(len(auditor.issues), 0)
        self.assertTrue(any("Windows file:// Navigation Hazard" in issue for issue in auditor.issues))

    def test_safe_navigation_with_isserver_guard_passes(self):
        safe_nav_html = """<!DOCTYPE html>
<html>
<body>
  <script>
    function switchBoard(boardId) {
      let targetFile = 'morning_mail_inbox.html';
      const isServer = window.location.protocol.startsWith('http');
      if (isServer) {
        window.location.href = `/${targetFile}?_t=${Date.now()}`;
      } else {
        window.location.href = targetFile;
      }
    }
  </script>
</body>
</html>"""
        fpath = self._write_temp_html(safe_nav_html)
        auditor = SidecarAuditor(str(fpath))
        auditor.audit_local_file_navigation_safety()
        self.assertEqual(len(auditor.issues), 0)


if __name__ == "__main__":
    unittest.main()
