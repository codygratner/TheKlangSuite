#!/usr/bin/env python3
"""
tools/tests/test_compile_prompt.py — Unit tests for Prompt Compiler (Flight 2)
=============================================================================
"""

import json
import os
from pathlib import Path
import sys
import tempfile
import unittest

# Ensure UTF-8 console output for Windows terminal safety
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(REPO_ROOT))

from tools.compile_prompt import (
    compile_payload_from_card,
    compile_payload_from_note,
    compile_prompt,
    copy_to_clipboard,
    find_card_data,
    find_vault_note,
    parse_markdown_note,
)
from tools.models import (
    AntithesisBlock,
    PromptHandoffPayload,
    SynthesisTargetConfig,
    ThesisBlock,
)


class TestPromptCompiler(unittest.TestCase):
    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()
        self.tmp_path = Path(self.temp_dir.name)

    def tearDown(self):
        self.temp_dir.cleanup()

    def test_pydantic_models_and_markdown(self):
        thesis = ThesisBlock(
            core_intent="Design a stateless worker",
            user_goals=["Avoid runaway autonomous loops", "Implement bearer auth"],
            source_note="TheKlangVault/Inbox/Stateless Sentry.md",
        )
        antithesis = AntithesisBlock(
            reviewer_persona="The Auditor",
            echo_chamber_shatter="Testing edge-case network dropouts",
            structural_vulnerabilities=["Mobile client disconnect during SSE stream"],
            orthogonal_vectors=["Single-turn HTTP timeout and idempotent retry"],
        )
        targets = SynthesisTargetConfig(
            materialization="Python Tooling",
            the_blueprint="Definitive implementation spec",
            effort_level="MINIMAL",
            impact_level="MAXIMUM",
        )
        payload = PromptHandoffPayload(
            thread_id="test_flight_24",
            reconciliation_protocol="Method B+",
            thesis=thesis,
            antithesis=antithesis,
            targets=targets,
        )

        md = payload.to_markdown()
        self.assertIn("# SYSTEM HANDOFF: DIALECTICAL SYNTHESIS ENGINE", md)
        self.assertIn("## [THESIS: CORE INTENT]", md)
        self.assertIn("## [ANTITHESIS: ADVERSARIAL STRESS-TEST]", md)
        self.assertIn("## [CONSTRAINTS & TARGETS]", md)
        self.assertIn("## PROMPT / COMMAND TO EXECUTE", md)
        self.assertIn("Mobile client disconnect during SSE stream", md)
        self.assertIn("Single-turn HTTP timeout and idempotent retry", md)

    def test_compile_from_card_dictionary(self):
        card = {
            "id": "card_test_99",
            "title": "Hardware Daisy Bridge",
            "synthesis_intent": "Port JUCE DSP to Daisy Seed microcontroller",
            "synthesis_mechanism": "Create C++ wrapper around TbdAudio math",
            "synthesis_route": "Direct Export",
            "pillar": "TBD-16",
        }
        payload = compile_payload_from_card(card)
        self.assertEqual(payload.thread_id, "card_test_99")
        self.assertIn("Hardware Daisy Bridge", payload.thesis.core_intent)
        self.assertIn("ESP32-P4 / Microcontroller Firmware", payload.targets.materialization)

    def test_compile_from_markdown_note(self):
        note_file = self.tmp_path / "test_note.md"
        note_content = """---
title: "Test Architecture Idea"
pillar: "TKS"
status: "BACKLOG"
---
# Test Architecture Idea

We need branchless tanh saturation curves for our polyphonic oscillator.
- [ ] Profile FastMath
- [ ] Measure CPU cycles
"""
        note_file.write_text(note_content, encoding="utf-8")

        payload = compile_payload_from_note(note_file)
        self.assertEqual(payload.thread_id, "test_note")
        self.assertIn("Test Architecture Idea", payload.thesis.core_intent)
        self.assertIn("Native C++20 / JUCE 9.0.3", payload.targets.materialization)

    def test_compile_prompt_to_file(self):
        card_file = self.tmp_path / "card.json"
        card_data = {
            "id": "topic_abc",
            "title": "Topic ABC",
            "synthesis_intent": "Test writing to file destination",
        }
        card_file.write_text(json.dumps(card_data), encoding="utf-8")

        out_file = self.tmp_path / "out_prompt.md"
        payload = compile_prompt(file_path=str(card_file), out_file=str(out_file))

        self.assertTrue(out_file.exists())
        content = out_file.read_text(encoding="utf-8")
        self.assertIn("Topic ABC", content)
        self.assertEqual(payload.thread_id, "topic_abc")

    def test_copy_to_clipboard_basic(self):
        # Verify clipboard function executes without crashing
        success = copy_to_clipboard("Testing N'kai prompt compiler clipboard transport")
        # On Windows or headless CI, returns bool
        self.assertIsInstance(success, bool)


if __name__ == "__main__":
    unittest.main()
