#!/usr/bin/env python3
"""
N'kai Sidecar Web QA Linter & Diagnostic Auditor (audit_sidecar.py)
Performs deterministic static analysis on HTML sidecars and webviews.
Catches tag imbalances, JS template literal escape errors, click-traps,
and webview sandbox violations.
"""

import argparse
import os
import re
import sys
from pathlib import Path
from bs4 import BeautifulSoup

class SidecarAuditor:
    def __init__(self, file_path: str, auto_fix: bool = False):
        self.file_path = Path(file_path)
        self.auto_fix = auto_fix
        self.issues = []
        self.warnings = []
        self.fixes_applied = []
        if not self.file_path.exists():
            raise FileNotFoundError(f"File not found: {self.file_path}")
        with open(self.file_path, "r", encoding="utf-8") as f:
            self.content = f.read()

    def audit_tag_balance(self):
        """Checks HTML div and section tag balance."""
        tags = ['div', 'main', 'section', 'header', 'aside', 'script', 'style']
        for tag in tags:
            opens = len(re.findall(rf'<{tag}[\s>]', self.content, re.IGNORECASE))
            closes = len(re.findall(rf'</{tag}>', self.content, re.IGNORECASE))
            diff = opens - closes
            if diff != 0:
                self.issues.append(f"HTML Tag Imbalance: <{tag}> has {opens} opens and {closes} closes (diff: {diff})")
            else:
                pass

    def audit_js_syntax_and_escapes(self):
        """Scans <script> blocks for fatal JavaScript syntax hazards like unescaped Windows paths."""
        scripts = re.findall(r'<script(?:\s+[^>]*)?>(.*?)</script>', self.content, re.DOTALL | re.IGNORECASE)
        for idx, script in enumerate(scripts):
            # Check for unescaped Windows paths in template literals or strings
            # Look for patterns like \Dev, \TheKlangSuite, \ui_ux, \sidecar
            unescaped_path_matches = re.finditer(r'([A-Za-z]:\\[^`"\'\n]+)', script)
            for m in unescaped_path_matches:
                raw_path = m.group(1)
                # Check if it has \u which is parsed as unicode escape in JS
                if re.search(r'\\u[0-9a-fA-F]{0,3}[^0-9a-fA-F]', raw_path) or r'\u' in raw_path:
                    self.issues.append(f"Fatal JS Syntax Hazard in <script #{idx+1}>: Unescaped Windows path with '\\u' sequence: '{raw_path}'. In JavaScript, '\\u' requires 4 hex digits and will throw SyntaxError!")
                    if self.auto_fix:
                        # Fix: convert backslashes to forward slashes or escape them
                        fixed_path = raw_path.replace('\\', '/')
                        self.content = self.content.replace(raw_path, fixed_path)
                        self.fixes_applied.append(f"Converted Windows path backslashes to forward slashes: '{raw_path}' -> '{fixed_path}'")

            # Check for implicit global event in functions
            # e.g., switchTab(tabId) using event.target without passing event
            fn_matches = re.finditer(r'function\s+([a-zA-Z0-9_]+)\s*\(([^)]*)\)\s*\{([^}]*)\}', script)
            for fn in fn_matches:
                fn_name = fn.group(1)
                fn_args = [a.strip() for a in fn.group(2).split(',') if a.strip()]
                fn_body = fn.group(3)
                if 'event.' in fn_body and 'event' not in fn_args:
                    self.warnings.append(f"Strict Mode Risk: Function '{fn_name}' accesses global 'event' but does not declare it as a parameter.")

    def audit_overlay_pointer_events(self):
        """Verifies that modal/drawer overlays do not trap clicks when inactive."""
        if 'drawer-overlay' in self.content:
            # Check CSS for .drawer-overlay without pointer-events: none
            overlay_css = re.search(r'\.drawer-overlay\s*\{([^}]+)\}', self.content)
            if overlay_css:
                css_body = overlay_css.group(1)
                if 'pointer-events: none' not in css_body:
                    self.issues.append("Click Trap Hazard: '.drawer-overlay' missing 'pointer-events: none;' in inactive state.")
                if 'visibility: hidden' not in css_body and 'display: none' not in css_body:
                    self.warnings.append("Visibility Trap: '.drawer-overlay' missing 'visibility: hidden;' in inactive state.")

    def audit_cache_busting_refresh(self):
        """Verifies that canvas reload buttons bypass Chromium file:// memory cache."""
        if 'location.reload()' in self.content:
            self.warnings.append("Chromium Cache Trap: Found raw 'location.reload()'. In Chromium file:/// webviews, this reloads stale memory cache. Prefer forceReloadCanvas() with timestamp query params.")

    def audit_audio_haptics(self):
        """Verifies Web Audio API patterns and default mute state."""
        if 'AudioContext' in self.content:
            if 'localStorage.getItem(\'nkai_audio_muted\')' not in self.content:
                self.warnings.append("Audio Hygiene: Web Audio found without persistent localStorage mute state.")

    def audit_navigation_and_scroll(self):
        """Verifies that sidebar anchor navigation and scroll targets are robust."""
        if 'sidebar-item' in self.content or 'nav-item' in self.content:
            if 'scroll-margin-top' not in self.content:
                self.warnings.append("Navigation Offset: CSS missing 'scroll-margin-top' on section targets. Sticky header may obscure scrolled content.")
            if '.sidebar-item' in self.content and 'preventDefault' not in self.content:
                self.issues.append("Navigation Trap: '.sidebar-item' click handler missing 'e.preventDefault()', which causes broken native hash jumps.")

    def audit_chrome_runtime_console(self):
        """Runs Chrome headless to detect runtime JS errors or SyntaxErrors."""
        chrome_paths = [
            Path(r"C:\Program Files\Google\Chrome\Application\chrome.exe"),
            Path(r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe")
        ]
        exe = next((p for p in chrome_paths if p.exists()), None)
        if not exe:
            return

        import subprocess
        try:
            url = self.file_path.as_uri()
            res = subprocess.run([
                str(exe),
                "--headless=new",
                "--disable-gpu",
                "--enable-logging=stderr",
                "--dump-dom",
                url
            ], capture_output=True, text=True, errors="replace", timeout=10)
            for line in res.stderr.splitlines():
                if "CONSOLE" in line and ("error" in line.lower() or "syntaxerror" in line.lower() or "uncaught" in line.lower()):
                    self.issues.append(f"Browser Runtime JS Error: {line.strip()}")
        except Exception as e:
            self.warnings.append(f"Headless Chrome Verification Skipped: {e}")

    def run(self) -> bool:
        self.audit_tag_balance()
        self.audit_js_syntax_and_escapes()
        self.audit_overlay_pointer_events()
        self.audit_cache_busting_refresh()
        self.audit_audio_haptics()
        self.audit_navigation_and_scroll()
        self.audit_chrome_runtime_console()

        passed = (len(self.issues) == 0)
        return passed

    def save_fixes(self):
        if self.fixes_applied:
            with open(self.file_path, "w", encoding="utf-8") as f:
                f.write(self.content)

def main():
    if hasattr(sys.stdout, 'reconfigure'):
        sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    parser = argparse.ArgumentParser(description="N'kai Sidecar Web QA Linter & Diagnostic Auditor")
    parser.add_argument("target", help="Path to the HTML sidecar file to audit")
    parser.add_argument("--fix", action="store_true", help="Automatically fix detected issues where safe")
    args = parser.parse_args()

    auditor = SidecarAuditor(args.target, auto_fix=args.fix)
    passed = auditor.run()

    print(f"\n========================================================")
    print(f" 🌐 N'KAI SIDECAR WEB QA AUDIT REPORT: {Path(args.target).name}")
    print(f"========================================================")

    if auditor.issues:
        print(f"\n❌ CRITICAL ISSUES ({len(auditor.issues)}):")
        for i, issue in enumerate(auditor.issues, 1):
            print(f"  {i}. {issue}")

    if auditor.warnings:
        print(f"\n⚠️ WARNINGS ({len(auditor.warnings)}):")
        for i, warn in enumerate(auditor.warnings, 1):
            print(f"  {i}. {warn}")

    if auditor.fixes_applied:
        print(f"\n🔧 FIXES APPLIED ({len(auditor.fixes_applied)}):")
        for i, fix in enumerate(auditor.fixes_applied, 1):
            print(f"  {i}. {fix}")
        auditor.save_fixes()

    if passed and not auditor.warnings:
        print("\n✅ PERFECT PASS: Zero issues or warnings detected!")
    elif passed:
        print("\n✅ PASS (WITH ADVISORIES): Zero critical errors; review warnings above.")
    else:
        print("\n❌ AUDIT FAILED: Critical issues must be resolved.")

    sys.exit(0 if passed else 1)

if __name__ == "__main__":
    main()
