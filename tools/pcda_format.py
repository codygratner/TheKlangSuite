#!/usr/bin/env python3
"""
N'kai PCDA Dialectic Formatter & Validator (tools/pcda_format.py)

A zero-dependency CLI utility to parse, validate, and format PCDA
(Pros, Cons, Devil's Advocate, 80/20 Resolution) dialectic review blocks
across Markdown documents and N'kai sidecars.

Usage:
    python tools/pcda_format.py --validate <file.md>
    python tools/pcda_format.py --to-html <file.md> [--output <file.html>]
    python tools/pcda_format.py --chat <file.md> [--output <file.md>]
"""

import argparse
import html
import os
import re
import sys
from pathlib import Path
from typing import Dict, List, Optional, Tuple


class DialecticBlock:
    def __init__(self, topic_id: str, topic_title: str, line_number: int):
        self.topic_id = topic_id
        self.topic_title = topic_title
        self.line_number = line_number
        self.pros: str = ""
        self.cons: str = ""
        self.advocate: str = ""
        self.synthesis: str = ""

    def is_complete(self) -> Tuple[bool, List[str]]:
        missing = []
        if not self.pros.strip():
            missing.append("Pros [Thesis]")
        if not self.cons.strip():
            missing.append("Cons [Thesis Friction]")
        if not self.advocate.strip():
            missing.append("Devil's Advocate [Antithesis]")
        if not self.synthesis.strip():
            missing.append("The 80/20 Resolution [Synthesis]")
        return len(missing) == 0, missing

    def slug(self) -> str:
        s = re.sub(r'[^a-zA-Z0-9_\-\.]', '-', self.topic_id or self.topic_title)
        s = re.sub(r'-+', '-', s).strip('-').lower()
        return s or "topic-pcda"


class PcdaParser:
    """Parses Markdown files to extract PCDA dialectic blocks."""

    # Component heading patterns
    RE_PROS = re.compile(
        r'^(?:#{2,5}\s*(?:1\.?\s*)?|\*\*(?:1\.?\s*)?)Pros(?:\s*\[(?:Thesis)?\])?(?:\*\*|:)?',
        re.IGNORECASE
    )
    RE_CONS = re.compile(
        r'^(?:#{2,5}\s*(?:2\.?\s*)?|\*\*(?:2\.?\s*)?)Cons(?:\s*\[(?:Thesis\s+)?Friction\])?(?:\*\*|:)?',
        re.IGNORECASE
    )
    RE_ADVOCATE = re.compile(
        r'^(?:#{2,5}\s*(?:3\.?\s*)?|\*\*(?:3\.?\s*)?)(?:Devil\'?s\s+Advocate|Advocate)(?:\s*\[Antithesis\])?(?:\*\*|:)?',
        re.IGNORECASE
    )
    RE_SYNTHESIS = re.compile(
        r'^(?:#{2,5}\s*(?:4\.?\s*)?|\*\*(?:4\.?\s*)?)(?:(?:The\s+)?80/20\s+Resolution|Resolution|Synthesis)(?:\s*\[Synthesis\])?(?:\*\*|:)?',
        re.IGNORECASE
    )

    # Topic header pattern (e.g. "### Topic 6.1: Continuous Live Triage" or "## 1. Monolithic Faceplate")
    RE_TOPIC = re.compile(
        r'^(#{1,3})\s+(?:Topic\s+)?([0-9]+(?:\.[0-9]+)*[:\s].*|.+?)$',
        re.IGNORECASE
    )

    @classmethod
    def parse_file(cls, file_path: Path) -> List[DialecticBlock]:
        if not file_path.exists():
            raise FileNotFoundError(f"File not found: {file_path}")

        lines = file_path.read_text(encoding='utf-8').splitlines()
        blocks: List[DialecticBlock] = []

        current_block: Optional[DialecticBlock] = None
        current_section: Optional[str] = None
        current_buffer: List[str] = []

        def flush_section():
            nonlocal current_section, current_buffer, current_block
            if current_block and current_section:
                content = "\n".join(current_buffer).strip()
                if current_section == "pros":
                    current_block.pros = content
                elif current_section == "cons":
                    current_block.cons = content
                elif current_section == "advocate":
                    current_block.advocate = content
                elif current_section == "synthesis":
                    current_block.synthesis = content
            current_buffer = []

        for idx, line in enumerate(lines, start=1):
            stripped = line.strip()

            # Check if this line is a new topic heading
            # We treat H1-H3 as topic boundaries if they don't look like dialectic subheadings
            is_dialectic_header = (
                cls.RE_PROS.match(stripped) or
                cls.RE_CONS.match(stripped) or
                cls.RE_ADVOCATE.match(stripped) or
                cls.RE_SYNTHESIS.match(stripped)
            )

            topic_match = cls.RE_TOPIC.match(stripped) if not is_dialectic_header else None

            # Look for explicit "Topic X" or dialectic container
            is_new_topic = False
            if topic_match:
                topic_text = stripped.lstrip('#').strip()
                # Check if it has "Topic" or numeric prefix or PCDA title
                if re.search(r'\b(?:topic|item|review|pcda)\b', topic_text, re.IGNORECASE) or re.match(r'^[0-9]+(\.[0-9]+)*', topic_text):
                    is_new_topic = True

            if is_new_topic:
                flush_section()
                if current_block and (current_block.pros or current_block.cons or current_block.advocate or current_block.synthesis):
                    blocks.append(current_block)

                topic_str = stripped.lstrip('#').strip()
                # Try to extract topic ID
                m_id = re.match(r'^(?:Topic\s+)?([0-9]+(?:\.[0-9]+)*)[:\s]*(.*)', topic_str, re.IGNORECASE)
                if m_id:
                    t_id = m_id.group(1).strip()
                    t_title = m_id.group(2).strip() or topic_str
                else:
                    t_id = topic_str
                    t_title = topic_str

                current_block = DialecticBlock(t_id, t_title, idx)
                current_section = None
                continue

            # Check for dialectic sections
            if cls.RE_PROS.match(stripped):
                flush_section()
                if not current_block:
                    current_block = DialecticBlock("1", "PCDA Dialectic Review", idx)
                current_section = "pros"
                # If there's text on the same line after bold header
                remainder = cls.RE_PROS.sub('', stripped).lstrip(':* ').strip()
                if remainder:
                    current_buffer.append(remainder)
                continue

            if cls.RE_CONS.match(stripped):
                flush_section()
                if not current_block:
                    current_block = DialecticBlock("1", "PCDA Dialectic Review", idx)
                current_section = "cons"
                remainder = cls.RE_CONS.sub('', stripped).lstrip(':* ').strip()
                if remainder:
                    current_buffer.append(remainder)
                continue

            if cls.RE_ADVOCATE.match(stripped):
                flush_section()
                if not current_block:
                    current_block = DialecticBlock("1", "PCDA Dialectic Review", idx)
                current_section = "advocate"
                remainder = cls.RE_ADVOCATE.sub('', stripped).lstrip(':* ').strip()
                if remainder:
                    current_buffer.append(remainder)
                continue

            if cls.RE_SYNTHESIS.match(stripped):
                flush_section()
                if not current_block:
                    current_block = DialecticBlock("1", "PCDA Dialectic Review", idx)
                current_section = "synthesis"
                remainder = cls.RE_SYNTHESIS.sub('', stripped).lstrip(':* ').strip()
                if remainder:
                    current_buffer.append(remainder)
                continue

            # Accumulate content into active section
            if current_section:
                current_buffer.append(line)

        flush_section()
        if current_block and (current_block.pros or current_block.cons or current_block.advocate or current_block.synthesis):
            blocks.append(current_block)

        return blocks


def markdown_to_html_snippet(text: str) -> str:
    """Zero-dependency markdown fragment to clean HTML."""
    if not text.strip():
        return "<p><em>None specified.</em></p>"

    lines = text.strip().splitlines()
    html_lines = []
    in_list = False

    for line in lines:
        s = line.strip()
        if not s:
            if in_list:
                html_lines.append("</ul>")
                in_list = False
            continue

        # Check list item
        list_match = re.match(r'^[-*]\s+(.*)', s)
        if list_match:
            if not in_list:
                html_lines.append("<ul>")
                in_list = True
            item_text = list_match.group(1)
            item_html = _format_inline(item_text)
            html_lines.append(f"  <li>{item_html}</li>")
        else:
            if in_list:
                html_lines.append("</ul>")
                in_list = False
            p_html = _format_inline(s)
            html_lines.append(f"<p>{p_html}</p>")

    if in_list:
        html_lines.append("</ul>")

    return "\n".join(html_lines)


def _format_inline(text: str) -> str:
    escaped = html.escape(text)
    # Inline code
    escaped = re.sub(r'`([^`]+)`', r'<code>\1</code>', escaped)
    # Bold
    escaped = re.sub(r'\*\*([^*]+)\*\*', r'<strong>\1</strong>', escaped)
    # Italic
    escaped = re.sub(r'\*([^*]+)\*', r'<em>\1</em>', escaped)
    return escaped


def format_chat_summary(blocks: List[DialecticBlock]) -> str:
    """Formats blocks into ultra-terse, punchy Markdown summaries for chat."""
    lines = ["# ⚖️ PCDA Dialectic Summary\n"]
    for b in blocks:
        lines.append(f"### Topic {b.topic_id}: {b.topic_title}")

        def first_sentence(t: str) -> str:
            clean = re.sub(r'^[-*]\s+', '', t.strip().splitlines()[0] if t.strip() else '')
            clean = clean.split('.')[0] + '.' if '.' in clean else clean
            return clean.strip() or 'None recorded.'

        lines.append(f"- **Pros [Thesis]**: {first_sentence(b.pros)}")
        lines.append(f"- **Cons [Friction]**: {first_sentence(b.cons)}")
        lines.append(f"- **Devil's Advocate [Antithesis]**: {first_sentence(b.advocate)}")
        lines.append(f"- **🎯 80/20 Resolution [Synthesis]**: {first_sentence(b.synthesis)}")
        lines.append("")
    return "\n".join(lines).strip()


def format_html_blocks(blocks: List[DialecticBlock]) -> str:
    """Formats blocks into drop-in N'kai HTML snippets using sidecar.css classes."""
    output = []
    output.append("<!-- N'kai PCDA Dialectic Blocks Container -->")
    output.append("<div class=\"pcda-container\">")

    for b in blocks:
        slug = b.slug()
        pros_html = markdown_to_html_snippet(b.pros)
        cons_html = markdown_to_html_snippet(b.cons)
        adv_html = markdown_to_html_snippet(b.advocate)
        syn_html = markdown_to_html_snippet(b.synthesis)
        escaped_title = html.escape(b.topic_title)

        block_html = f"""  <div class="pcda-block" id="pcda-block-{slug}" data-topic-id="{html.escape(b.topic_id)}" data-topic-title="{escaped_title}">
    <div class="pcda-header" onclick="togglePcdaBlock('pcda-body-{slug}')">
      <div class="pcda-header-left">
        <span class="pcda-toggle-icon" id="pcda-body-{slug}-icon">▼</span>
        <span>⚖️ Topic {html.escape(b.topic_id)}: {escaped_title}</span>
      </div>
      <div class="pcda-header-actions">
        <button class="pcda-copy-btn" onclick="event.stopPropagation(); copyPcdaMarkdown('{slug}')" title="Copy clean Markdown to clipboard">
          <span>📋</span> Copy PCDA
        </button>
      </div>
    </div>
    <div class="pcda-body" id="pcda-body-{slug}">
      <div class="pcda-card pcda-pro">
        <div class="pcda-card-title"><span>✅</span> 1. Pros [Thesis]</div>
        <div class="pcda-card-content">
{pros_html}
        </div>
      </div>
      <div class="pcda-card pcda-con">
        <div class="pcda-card-title"><span>⚠️</span> 2. Cons [Thesis Friction]</div>
        <div class="pcda-card-content">
{cons_html}
        </div>
      </div>
      <div class="pcda-card pcda-advocate">
        <div class="pcda-card-title"><span>👹</span> 3. Devil's Advocate [Antithesis]</div>
        <div class="pcda-card-content">
{adv_html}
        </div>
      </div>
      <div class="pcda-card pcda-synthesis">
        <div class="pcda-card-title">
          <span class="pcda-resolution-badge">🎯 80/20 RESOLUTION</span>
          <span>4. Synthesis</span>
        </div>
        <div class="pcda-card-content">
{syn_html}
        </div>
      </div>
    </div>
  </div>"""
        output.append(block_html)

    output.append("</div>")
    return "\n".join(output)


def main():
    if hasattr(sys.stdout, 'reconfigure'):
        sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    if hasattr(sys.stderr, 'reconfigure'):
        sys.stderr.reconfigure(encoding='utf-8', errors='replace')

    parser = argparse.ArgumentParser(
        description="Zero-Dependency N'kai PCDA Formatter & Validator"
    )
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument(
        "--validate",
        metavar="FILE",
        help="Validate that all 4 PCDA components exist for each topic in FILE"
    )
    group.add_argument(
        "--to-html",
        metavar="FILE",
        help="Convert Markdown PCDA blocks in FILE to drop-in N'kai HTML snippets"
    )
    group.add_argument(
        "--chat",
        metavar="FILE",
        help="Generate terse, chat-friendly Markdown summaries of PCDA blocks"
    )
    parser.add_argument(
        "--output", "-o",
        metavar="OUT",
        help="Optional destination output file (defaults to stdout)"
    )

    args = parser.parse_args()

    target_file = Path(args.validate or args.to_html or args.chat)
    if not target_file.exists():
        print(f"[ERROR] Target file does not exist: {target_file}", file=sys.stderr)
        sys.exit(1)

    blocks = PcdaParser.parse_file(target_file)

    if not blocks:
        print(f"[WARN] No PCDA dialectic blocks detected in: {target_file}", file=sys.stderr)
        if args.validate:
            sys.exit(1)
        sys.exit(0)

    if args.validate:
        all_passed = True
        print(f"\n[PCDA VALIDATOR] Auditing {target_file} ({len(blocks)} topics detected)")
        for b in blocks:
            ok, missing = b.is_complete()
            if ok:
                print(f"  [PASS] Topic {b.topic_id} ('{b.topic_title}', line {b.line_number}): All 4 components present.")
            else:
                all_passed = False
                missing_str = ", ".join(missing)
                print(f"  [FAIL] Topic {b.topic_id} ('{b.topic_title}', line {b.line_number}): Missing: {missing_str}", file=sys.stderr)

        if all_passed:
            print("\n[SUCCESS] All PCDA dialectic blocks validated successfully (100% complete).")
            sys.exit(0)
        else:
            print("\n[ABORT] PCDA validation failed. Please address missing dialectic sections.", file=sys.stderr)
            sys.exit(1)

    elif args.to_html:
        html_out = format_html_blocks(blocks)
        if args.output:
            out_path = Path(args.output)
            out_path.parent.mkdir(parents=True, exist_ok=True)
            out_path.write_text(html_out, encoding='utf-8')
            print(f"[PCDA] HTML snippets written to: {out_path}")
        else:
            print(html_out)

    elif args.chat:
        chat_out = format_chat_summary(blocks)
        if args.output:
            out_path = Path(args.output)
            out_path.parent.mkdir(parents=True, exist_ok=True)
            out_path.write_text(chat_out, encoding='utf-8')
            print(f"[PCDA] Chat summary written to: {out_path}")
        else:
            print(chat_out)


if __name__ == "__main__":
    main()
