#!/usr/bin/env python3
"""
tools/tests/test_lens_profiles.py — Unit tests for Configurable Lens Profiles (Flight 4 / Topic 9)
=================================================================================================
"""

import json
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

from tools.models import (
    DossierDrawer,
    LensDefinition,
    LensProfile,
    LensProfilesConfig,
    detect_lens_profile,
    load_lens_profiles,
)
from tools.research_daemon import execute_pass_2_synthesis
from tools.compile_prompt import compile_payload_from_card, compile_prompt
from tools.board_renderer import render_dossier_drawer_html


class TestLensProfiles(unittest.TestCase):
    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()
        self.tmp_path = Path(self.temp_dir.name)

    def tearDown(self):
        self.temp_dir.cleanup()

    def test_load_lens_profiles_schema_and_contents(self):
        cfg = load_lens_profiles()
        self.assertEqual(cfg.version, "1.0.0")
        self.assertEqual(cfg.default_profile, "audio_dev")

        expected_profiles = ["audio_dev", "studio_hardware", "creative_general"]
        for p in expected_profiles:
            self.assertIn(p, cfg.profiles, f"Profile '{p}' must be present in lens_profiles.json")
            profile = cfg.profiles[p]
            self.assertTrue(profile.display_name)
            self.assertGreaterEqual(len(profile.tags), 1)
            self.assertEqual(len(profile.lenses), 4, f"Profile '{p}' must define exactly 4 lenses")
            for lens in profile.lenses:
                self.assertTrue(lens.id)
                self.assertTrue(lens.name)
                self.assertTrue(lens.prompt)

    def test_detect_lens_profile_by_tags(self):
        cfg = load_lens_profiles()

        # Studio / Hardware triggers
        self.assertEqual(detect_lens_profile(["#studio", "#eurorack"], config=cfg), "studio_hardware")
        self.assertEqual(detect_lens_profile(["hardware", "wiring"], config=cfg), "studio_hardware")
        self.assertEqual(detect_lens_profile(["#m8", "patchbay"], config=cfg), "studio_hardware")
        self.assertEqual(detect_lens_profile(["#tbd-16"], config=cfg), "studio_hardware")

        # Creative / General triggers
        self.assertEqual(detect_lens_profile(["#ttrpg"], config=cfg), "creative_general")
        self.assertEqual(detect_lens_profile(["writing", "lyrics"], config=cfg), "creative_general")
        self.assertEqual(detect_lens_profile(["#gamedesign"], config=cfg), "creative_general")

        # Audio Dev triggers
        self.assertEqual(detect_lens_profile(["#audio", "#dsp"], config=cfg), "audio_dev")
        self.assertEqual(detect_lens_profile(["vst", "juce"], config=cfg), "audio_dev")

        # Untagged / fallback
        self.assertEqual(detect_lens_profile([], config=cfg), "audio_dev")
        self.assertEqual(detect_lens_profile(["random_unrecognized_tag"], config=cfg), "audio_dev")

    def test_detect_lens_profile_by_text_keywords(self):
        cfg = load_lens_profiles()

        text_hardware = "Thinking about rewiring the patchbay and normalled jacks in the eurorack rack."
        self.assertEqual(detect_lens_profile([], text=text_hardware, config=cfg), "studio_hardware")

        text_creative = "Drafting new lyrics and narrative hooks for the next campaign #ttrpg"
        self.assertEqual(detect_lens_profile([], text=text_creative, config=cfg), "creative_general")

    def test_research_daemon_synthesis_per_profile(self):
        thesis = "Affirmative thesis test"
        antithesis = "Adversarial antithesis test"

        # 1. Studio Hardware Profile Override
        dossier_hw = execute_pass_2_synthesis(
            topic="Studio Rewiring & Eurorack Layout",
            thesis=thesis,
            antithesis=antithesis,
            profile_override="studio_hardware",
        )
        self.assertEqual(dossier_hw.pcda_dialectic["profile"], "studio_hardware")
        self.assertEqual(dossier_hw.pcda_dialectic["profile_name"], "Studio, Hardware & Eurorack Ergonomics")
        lenses_hw = dossier_hw.pcda_dialectic["lenses"]
        self.assertIn("signal_flow", lenses_hw)
        self.assertIn("physical_ergonomics", lenses_hw)
        self.assertIn("power_noise", lenses_hw)
        self.assertIn("bom_budget", lenses_hw)

        # 2. Creative General Profile Override
        dossier_cg = execute_pass_2_synthesis(
            topic="TTRPG Campaign Rules & Lyric Prose",
            thesis=thesis,
            antithesis=antithesis,
            profile_override="creative_general",
        )
        self.assertEqual(dossier_cg.pcda_dialectic["profile"], "creative_general")
        self.assertEqual(dossier_cg.pcda_dialectic["profile_name"], "General Research, TTRPG & Creative Writing")
        lenses_cg = dossier_cg.pcda_dialectic["lenses"]
        self.assertIn("intent_narrative", lenses_cg)
        self.assertIn("user_experience", lenses_cg)
        self.assertIn("feasibility", lenses_cg)
        self.assertIn("elegance", lenses_cg)

        # 3. Tag-based auto detection in synthesis
        dossier_auto = execute_pass_2_synthesis(
            topic="Custom Module Design",
            thesis=thesis,
            antithesis=antithesis,
            tags=["#hardware", "#m8"],
        )
        self.assertEqual(dossier_auto.pcda_dialectic["profile"], "studio_hardware")

    def test_compile_prompt_lens_profiles(self):
        # 1. Auto-detection from tags
        card_ttrpg = {
            "id": "ttrpg_mechanics",
            "title": "TTRPG Dice Balance Mechanics",
            "tags": ["ttrpg", "gamedesign"],
            "summary": "Design a d20 resolution system",
        }
        payload = compile_payload_from_card(card_ttrpg)
        self.assertIn("General Research, TTRPG & Creative Writing", payload.prompt_mandate)
        self.assertIn("Creative Intent & Narrative Resonance", payload.prompt_mandate)
        self.assertIn("User / Player Experience", payload.prompt_mandate)

        # 2. Explicit profile override via compile_prompt
        json_file = self.tmp_path / "test_card.json"
        json_file.write_text(json.dumps({
            "id": "patchbay_box",
            "title": "Boutique 48-Point Patchbay",
            "tags": ["audio"],  # tags say audio, but override will force studio_hardware
            "summary": "Build balanced TRS patchbay",
        }), encoding="utf-8")

        payload_hw = compile_prompt(file_path=str(json_file), profile="studio_hardware")
        self.assertIn("Studio, Hardware & Eurorack Ergonomics", payload_hw.prompt_mandate)
        self.assertIn("Signal Flow, Latency & Impedance", payload_hw.prompt_mandate)
        self.assertIn("Power Draw, Heat & Noise Floor", payload_hw.prompt_mandate)

    def test_board_renderer_lens_badge(self):
        dossier = DossierDrawer(
            source_note="test.md",
            source_markdown="# My Studio Project\nPatchbay details.",
            pcda_dialectic={
                "profile": "studio_hardware",
                "profile_name": "Studio, Hardware & Eurorack Ergonomics",
                "pros": ["Modular design"],
                "cons": ["High initial soldering time"],
                "lenses": {
                    "signal_flow": "Clean analog path",
                    "physical_ergonomics": "Front jacks",
                },
            },
            pass_count=2,
            researched_at="2026-10-13T00:00:00",
        )
        rendered_html = render_dossier_drawer_html("test_card_123", dossier)
        self.assertIn("🔬 Active Lens: Studio, Hardware &amp; Eurorack Ergonomics", rendered_html)
        self.assertIn("Signal Flow", rendered_html)
        self.assertIn("Physical Ergonomics", rendered_html)


if __name__ == "__main__":
    unittest.main()
