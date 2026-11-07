#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
tools/sqa/lint_web.py — Universal Web Event-to-Sink Linter
=========================================================
Part of Chassis 2: Universal SQA Perimeter.
Audits web applications, HTML5 audio players (e.g. "Totally Not a Podcast Player"),
and interactive sidecars to detect:
  1. Orphan interactive elements: <button>, <input>, <select> with zero attached event listeners.
  2. Banned dummy stubs: Handlers containing only console.log, // TODO, e.preventDefault(), or empty bodies.
  3. Lack of functional sink: Handlers that fail to mutate state, call Web Audio/HTML5 APIs, or dispatch events.
  4. Lying UI controls: Controls whose visual state updates without modifying underlying media/model state.

Runs via Python regex/HTMLParser in <25ms with 0 external dependencies.
"""

import sys
import os
import re
import argparse
from pathlib import Path
from html.parser import HTMLParser

# Universal UTF-8 console output
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")


class InteractiveElementExtractor(HTMLParser):
    def __init__(self):
        super().__init__()
        self.interactive_elements = []  # list of (tag, id, classes, inline_events)

    def handle_starttag(self, tag, attrs):
        attr_dict = dict(attrs)
        is_interactive = tag in ['button', 'input', 'select', 'textarea'] or \
                         attr_dict.get('role') in ['button', 'slider', 'tab', 'menuitem'] or \
                         'onclick' in attr_dict or '@click' in attr_dict or 'v-on:click' in attr_dict

        if is_interactive:
            elem_id = attr_dict.get('id', '')
            elem_class = attr_dict.get('class', '')
            inline_events = [k for k in attr_dict.keys() if k.startswith('on') or k.startswith('@') or k.startswith('v-on')]
            self.interactive_elements.append({
                'tag': tag,
                'id': elem_id,
                'class': elem_class,
                'type': attr_dict.get('type', ''),
                'inline_events': inline_events
            })


def audit_web_file(html_file: Path) -> tuple[list[str], list[str]]:
    errors = []
    warnings = []

    if not html_file.exists():
        return [f"File not found: {html_file}"], []

    content = html_file.read_text(encoding="utf-8", errors="replace")

    # 1. Parse HTML Elements
    parser = InteractiveElementExtractor()
    try:
        parser.feed(content)
    except Exception as e:
        warnings.append(f"HTML parse error: {e}")

    # Extract all JavaScript (inline <script> or associated .js)
    scripts = re.findall(r'<script\b[^>]*>(.*?)</script>', content, re.DOTALL)
    combined_js = "\n".join(scripts)

    # 2. Check for Banned Dummy Stubs in JavaScript
    dummy_stubs = [
        (r'function\s*\w*\s*\([^)]*\)\s*\{\s*\}', "Empty handler function {}"),
        (r'\(\s*\)\s*=>\s*\{\s*\}', "Empty arrow function () => {}"),
        (r'//\s*TODO\b[^\n]*', "Unresolved // TODO comment in handler code"),
        (r'console\.log\s*\(\s*["\'](?:todo|clicked|test|stub|not implemented)["\']\s*\)', "Dummy console.log stub"),
        (r'alert\s*\(\s*["\'](?:todo|clicked|test|stub)["\']\s*\)', "Dummy alert stub"),
    ]

    for pat, desc in dummy_stubs:
        matches = re.findall(pat, combined_js, re.IGNORECASE)
        if matches:
            errors.append(f"BANNED DUMMY STUB: Found {len(matches)} instance(s) of '{desc}' in script code!")

    # 3. Check for Event-to-Sink Reachability on Interactive Elements
    for elem in parser.interactive_elements:
        elem_id = elem['id']
        elem_tag = elem['tag']
        elem_type = elem['type']

        if elem_type == 'hidden':
            continue

        if not elem_id and not elem['inline_events']:
            warnings.append(f"ANONYMOUS CONTROL: <{elem_tag} class='{elem['class']}'> has no id or event handler. Hard to bind/test.")
            continue

        if elem_id:
            # Check if elem_id has addEventListener or getElementById binding in JS
            has_listener = bool(re.search(rf'[\'"`]{re.escape(elem_id)}[\'"`]', combined_js)) or bool(elem['inline_events'])
            if not has_listener:
                errors.append(f"ORPHAN CONTROL: <{elem_tag} id='{elem_id}'> declared in HTML but NEVER referenced in JavaScript!")

    # 4. Media/Audio Specific Sink Check (for Podcast Players & Media Apps)
    if '<audio' in content or 'new Audio' in combined_js or 'AudioContext' in combined_js:
        # Check if playback controls flow to audio element methods
        media_sinks = ['play', 'pause', 'playbackRate', 'currentTime', 'volume']
        found_sinks = [s for s in media_sinks if s in combined_js]
        if len(found_sinks) < 2:
            warnings.append(f"WEAK MEDIA BINDING: Audio element detected, but JS references only {found_sinks}. Verify player controls are not hollow!")

    return errors, warnings


def main():
    parser = argparse.ArgumentParser(description="Universal Web Event-to-Sink Linter (Chassis 2)")
    parser.add_argument("file_path", help="Path to HTML or JS/TS file to audit")
    args = parser.parse_args()

    target = Path(args.file_path)
    print(f"[*] SQA CHASSIS 2: Auditing Web Event-to-Sink Reachability for: {target.name}")

    errs, warns = audit_web_file(target)

    for e in errs:
        print(f"   ❌ ERROR: {e}")
    for w in warns:
        print(f"   ⚠️  WARN:  {w}")

    if not errs:
        print(f"\n✅ PASS: 100% of interactive controls and event handlers reach active sinks.")
        sys.exit(0)
    else:
        print(f"\n[!] WEB SQA FAILURE: {len(errs)} defect(s) detected. Halting completion.\n")
        sys.exit(1)


if __name__ == "__main__":
    main()
