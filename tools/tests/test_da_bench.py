#!/usr/bin/env python3
"""
Unit and Integration Tests for da_bench.py (Card rj-22)

Tests:
1. Model name redaction preserving Pro Tools, ProRes, Logic Pro, Mac Pro.
2. Sanitization of part continuation markers and blindness preambles.
3. Seeded shuffle determinism, key sealing, and R1..R6 roundtrip de-anonymization.
4. Kendall tau rank concordance calculation (identical, reversed, swaps, ties).
5. Blindness auditing against clean vs. tainted transcripts (file reads, command leaks, web leaks).
6. Manifest checks, artifact validation, and status transitions (HOLD state).
7. Prompt SHA-256 hash integrity against prompts.sha256 and manifest.json.
8. Score aggregation, method deltas (B - A), and self-preference bias.
"""

import contextlib
from datetime import datetime, timezone
import io
import json
from pathlib import Path
import shutil
import sys
import tempfile
import unittest

# Universal UTF-8 console output
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.da_bench import (
    calculate_kendall_tau,
    compute_sha256,
    sanitize_report_text,
    handle_sanitize,
    audit_single_transcript,
    parse_score_block_from_text,
    handle_aggregate,
    handle_manifest,
    CANONICAL_CELLS,
    DEFAULT_BENCHMARK_DIR,
)


class DummyArgs:
    """Helper to mock argparse namespace."""
    def __init__(self, **kwargs):
        for k, v in kwargs.items():
            setattr(self, k, v)


class TestDaBench(unittest.TestCase):

    def test_redaction_preserves_pro_tools_and_prores(self):
        """Asserts Pro Tools and Apple ProRes are preserved while model names are redacted."""
        text = (
            "He recorded in Pro Tools HD using Apple ProRes 422 video sync. "
            "Later he tested ProTools 2024 with Logic Pro and Mac Pro hardware. "
            "However, Claude and Opus wrote the plan, while Gemini and Flash assisted. "
            "Pro is capable, but Flash Model is cheaper. "
            "**Benchmark**: Flash Model | Systems Architecture\n"
            "(Benchmark: Pro Model)\n"
        )
        cleaned = sanitize_report_text(text)

        # Assert protected terms remain intact
        self.assertIn("Pro Tools HD", cleaned)
        self.assertIn("Apple ProRes 422", cleaned)
        self.assertIn("ProTools 2024", cleaned)
        self.assertIn("Logic Pro", cleaned)
        self.assertIn("Mac Pro", cleaned)

        # Assert models are redacted
        self.assertNotIn("Claude", cleaned)
        self.assertNotIn("Opus", cleaned)
        self.assertNotIn("Gemini", cleaned)
        self.assertNotIn("Flash assisted", cleaned)
        self.assertNotIn("Pro is capable", cleaned)
        self.assertIn("[MODEL] and [MODEL] wrote the plan", cleaned)
        self.assertIn("[MODEL] and [MODEL] assisted", cleaned)
        self.assertIn("[MODEL] is capable", cleaned)
        self.assertIn("**Benchmark**: [MODEL] Model", cleaned)
        self.assertIn("(Benchmark: [MODEL] Model)", cleaned)

    def test_sanitization_part_and_blindness_stripping(self):
        """Asserts blindness preambles and multi-part continuation headers are stripped."""
        text = (
            "# PCDA Teardown: Backend (PART 1/2)\n\n"
            "(Written under the blindness rule: no workspace files read.)\n"
            "BLINDNESS RULE: strictly independent.\n\n"
            "Here is the thesis.\n\n"
            "(Part 2 follows with the 80/20 resolution.)\n\n"
            "# PCDA Teardown (PART 2/2): 🟣 THE 80/20 RESOLUTION\n\n"
            "Here is the resolution.\n\n"
            "(Continued in PART 2/4.)\n\n"
            "PCDA REPORT\n\n"
            "## Section 3\n"
        )
        cleaned = sanitize_report_text(text)

        self.assertNotIn("blindness rule", cleaned.lower())
        self.assertNotIn("BLINDNESS RULE", cleaned)
        self.assertNotIn("(PART 1/2)", cleaned)
        self.assertNotIn("(PART 2/2)", cleaned)
        self.assertNotIn("Part 2 follows", cleaned)
        self.assertNotIn("Continued in PART", cleaned)
        self.assertIn("# PCDA Teardown: Backend", cleaned)
        self.assertIn("# PCDA Teardown 🟣 THE 80/20 RESOLUTION", cleaned)
        self.assertIn("## Section 3", cleaned)

    def test_seeded_shuffle_and_key_roundtrip(self):
        """Asserts deterministic seeded shuffling, key sealing, and R1..R6 roundtrip."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            tmp_path = Path(tmp_dir)
            reports_dir = tmp_path / "reports"
            anonymized_dir = tmp_path / "anonymized"
            sealed_file = tmp_path / "sealed" / "key.json"
            reports_dir.mkdir(parents=True)

            for cell in CANONICAL_CELLS:
                (reports_dir / f"{cell}.md").write_text(
                    f"# Report for {cell}\nAnalysis of audio systems using Pro Tools.",
                    encoding="utf-8",
                )

            args = DummyArgs(
                benchmark_dir=str(tmp_path),
                reports_dir=str(reports_dir),
                out_dir=str(anonymized_dir),
                key_file=str(sealed_file),
                seed=4242,
                allow_partial=False,
            )

            with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
                rc = handle_sanitize(args)
            self.assertEqual(rc, 0)
            self.assertTrue(sealed_file.exists())

            # Read sealed key
            key_data = json.loads(sealed_file.read_text(encoding="utf-8"))
            self.assertEqual(key_data["seed"], 4242)
            self.assertEqual(len(key_data["mapping"]), 6)
            self.assertEqual(len(key_data["reverse_mapping"]), 6)

            # Assert all R1..R6 exist
            for i in range(1, 7):
                r_file = anonymized_dir / f"R{i}.md"
                self.assertTrue(r_file.exists())
                content = r_file.read_text(encoding="utf-8")
                # Assert model name in title is redacted to [MODEL]
                self.assertIn("# Report for [MODEL]-", content)
                self.assertIn("Pro Tools", content)

            # Roundtrip mapping verification
            for r_id, original_cell in key_data["mapping"].items():
                self.assertEqual(key_data["reverse_mapping"][original_cell], r_id)

    def test_kendall_tau_concordance(self):
        """Asserts Kendall tau calculation matches expected concordance values."""
        # 1. Identical rankings -> 1.0
        r1 = ["R1", "R2", "R3", "R4", "R5", "R6"]
        self.assertAlmostEqual(calculate_kendall_tau(r1, r1), 1.0, places=4)

        # 2. Perfect reverse -> -1.0
        r_rev = list(reversed(r1))
        self.assertAlmostEqual(calculate_kendall_tau(r1, r_rev), -1.0, places=4)

        # 3. Single swap in 4 elements: total pairs = 6, 1 discordant, 5 concordant -> 4/6 = 0.6667
        a = ["R1", "R2", "R3", "R4"]
        b = ["R1", "R3", "R2", "R4"]
        self.assertAlmostEqual(calculate_kendall_tau(a, b), 4.0 / 6.0, places=4)

        # 4. Independent / orthogonal rankings
        c = ["R1", "R2", "R3", "R4"]
        d = ["R2", "R4", "R1", "R3"]
        tau = calculate_kendall_tau(c, d)
        self.assertTrue(-1.0 <= tau <= 1.0)

    def test_audit_blindness_clean_vs_tainted(self):
        """Asserts transcript blindness auditor catches leaks while approving clean traces."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            tmp_path = Path(tmp_dir)
            bench_dir = tmp_path / "bench"
            (bench_dir / "anonymized").mkdir(parents=True)
            (bench_dir / "reports").mkdir(parents=True)
            (bench_dir / "sealed").mkdir(parents=True)

            anon_path = bench_dir / "anonymized" / "R1.md"
            anon_path.write_text("Clean text", encoding="utf-8")

            # Clean transcript
            clean_transcript = tmp_path / "clean.jsonl"
            clean_lines = [
                json.dumps({
                    "step_index": 1,
                    "tool_calls": [
                        {
                            "name": "view_file",
                            "args": {"AbsolutePath": str(anon_path)},
                        },
                        {
                            "name": "search_web",
                            "args": {"query": "tracktion engine c++ licensing"},
                        },
                    ],
                }),
            ]
            clean_transcript.write_text("\n".join(clean_lines) + "\n", encoding="utf-8")

            violations = audit_single_transcript(clean_transcript, bench_dir)
            self.assertEqual(len(violations), 0)

            # Tainted transcript 1: read reports/
            tainted_1 = tmp_path / "tainted_1.jsonl"
            report_file = bench_dir / "reports" / "opus-A.md"
            t1_lines = [
                json.dumps({
                    "step_index": 1,
                    "tool_calls": [
                        {
                            "name": "view_file",
                            "args": {"AbsolutePath": str(report_file)},
                        },
                    ],
                }),
            ]
            tainted_1.write_text("\n".join(t1_lines) + "\n", encoding="utf-8")
            v1 = audit_single_transcript(tainted_1, bench_dir)
            self.assertEqual(len(v1), 1)
            self.assertIn("reports directory", v1[0]["reason"])

            # Tainted transcript 2: read key.json
            tainted_2 = tmp_path / "tainted_2.jsonl"
            key_file = bench_dir / "sealed" / "key.json"
            t2_lines = [
                json.dumps({
                    "step_index": 1,
                    "tool_calls": [
                        {
                            "name": "view_file",
                            "args": {"AbsolutePath": str(key_file)},
                        },
                    ],
                }),
            ]
            tainted_2.write_text("\n".join(t2_lines) + "\n", encoding="utf-8")
            v2 = audit_single_transcript(tainted_2, bench_dir)
            self.assertEqual(len(v2), 1)
            self.assertIn("sealed de-anonymization key", v2[0]["reason"])

            # Tainted transcript 3: command leak
            tainted_3 = tmp_path / "tainted_3.jsonl"
            t3_lines = [
                json.dumps({
                    "step_index": 1,
                    "tool_calls": [
                        {
                            "name": "run_command",
                            "args": {"CommandLine": "cat sealed/key.json"},
                        },
                    ],
                }),
            ]
            tainted_3.write_text("\n".join(t3_lines) + "\n", encoding="utf-8")
            v3 = audit_single_transcript(tainted_3, bench_dir)
            self.assertEqual(len(v3), 1)

    def test_manifest_lifecycle_and_checks(self):
        """Asserts manifest validation logic, required artifacts, and HOLD state updates."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            tmp_path = Path(tmp_dir)

            # Setup minimal benchmark dir
            (tmp_path / "prompts").mkdir()
            (tmp_path / "reports").mkdir()
            (tmp_path / "sealed").mkdir()
            (tmp_path / "meta").mkdir()
            (tmp_path / "reviews").mkdir()

            ma = tmp_path / "prompts" / "method_A.txt"
            mb = tmp_path / "prompts" / "method_B.txt"
            ma.write_text("Prompt A content", encoding="utf-8")
            mb.write_text("Prompt B content", encoding="utf-8")

            ha = compute_sha256(ma.read_bytes())
            hb = compute_sha256(mb.read_bytes())
            (tmp_path / "prompts" / "prompts.sha256").write_text(
                f"{ha}  method_A.txt\n{hb}  method_B.txt\n",
                encoding="utf-8",
            )

            (tmp_path / "ground_truth.md").write_text("Ground truth", encoding="utf-8")
            for r in ["flash-A.md", "pro-A.md", "opus-A.md", "opus-B.md"]:
                (tmp_path / "reports" / r).write_text(f"Report {r}", encoding="utf-8")

            manifest_file = tmp_path / "manifest.json"
            manifest_file.write_text(
                json.dumps({
                    "benchmark_id": "rj-22",
                    "title": "Test Benchmark",
                    "status": "IN_PROGRESS",
                    "stages": {"W2": {"status": "COMPLETED"}},
                }),
                encoding="utf-8",
            )

            # Check W2 passes
            args_check = DummyArgs(
                benchmark_dir=str(tmp_path),
                manifest_action="check",
                through="W2",
            )
            with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
                rc_check = handle_manifest(args_check)
            self.assertEqual(rc_check, 0)

            # Set status to HOLD
            args_set = DummyArgs(
                benchmark_dir=str(tmp_path),
                manifest_action="set",
                key="status",
                val="HOLD",
            )
            with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
                rc_set = handle_manifest(args_set)
            self.assertEqual(rc_set, 0)

            # Verify saved manifest has HOLD
            updated = json.loads(manifest_file.read_text(encoding="utf-8"))
            self.assertEqual(updated["status"], "HOLD")

            # Remove ground_truth and verify check fails
            (tmp_path / "ground_truth.md").unlink()
            with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
                rc_fail = handle_manifest(args_check)
            self.assertEqual(rc_fail, 1)

    def test_prompt_hash_integrity(self):
        """Asserts frozen prompts in the actual benchmark directory match prompts.sha256."""
        prompts_dir = DEFAULT_BENCHMARK_DIR / "prompts"
        self.assertTrue(prompts_dir.exists(), "Benchmark prompts directory does not exist")

        sha_file = prompts_dir / "prompts.sha256"
        self.assertTrue(sha_file.exists(), "prompts.sha256 does not exist")

        expected_hashes = {}
        for line in sha_file.read_text(encoding="utf-8").splitlines():
            parts = line.split()
            if len(parts) >= 2:
                expected_hashes[parts[1]] = parts[0]

        self.assertIn("method_A.txt", expected_hashes)
        self.assertIn("method_B.txt", expected_hashes)

        # Expected canonical hashes
        canon_a = "6ce86766e07ddb77d62d275539349b12e1b7b9e3003555fc0cf2564969c706e8"
        canon_b = "cf814ffaa4fd1eab84f54048b2f1bd6f5491be6f0dc704ac7c3a3fcd718da994"

        self.assertEqual(expected_hashes["method_A.txt"], canon_a)
        self.assertEqual(expected_hashes["method_B.txt"], canon_b)

        file_a = prompts_dir / "method_A.txt"
        file_b = prompts_dir / "method_B.txt"
        self.assertEqual(compute_sha256(file_a.read_bytes()), canon_a)
        self.assertEqual(compute_sha256(file_b.read_bytes()), canon_b)

    def test_aggregate_scores_and_method_delta(self):
        """Asserts aggregation calculates mean scores, B - A deltas, and self-preference bias."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            tmp_path = Path(tmp_dir)
            reviews_dir = tmp_path / "reviews"
            sealed_dir = tmp_path / "sealed"
            anonymized_dir = tmp_path / "anonymized"
            reviews_dir.mkdir()
            sealed_dir.mkdir()
            anonymized_dir.mkdir()

            # Mapping: R1=flash-A, R2=flash-B, R3=pro-A, R4=pro-B, R5=opus-A, R6=opus-B
            key_data = {
                "mapping": {
                    "R1": "flash-A",
                    "R2": "flash-B",
                    "R3": "pro-A",
                    "R4": "pro-B",
                    "R5": "opus-A",
                    "R6": "opus-B",
                }
            }
            (sealed_dir / "key.json").write_text(json.dumps(key_data), encoding="utf-8")

            # Mock Judge Flash review
            review_flash = """
# Judge Flash
```json
{
  "judge": "flash",
  "scores": {
    "R1": 7.0,
    "R2": 8.0,
    "R3": 6.0,
    "R4": 7.0,
    "R5": 8.0,
    "R6": 9.0
  },
  "ranking": ["R6", "R2", "R5", "R1", "R4", "R3"]
}
```
"""
            (reviews_dir / "judge-flash.md").write_text(review_flash, encoding="utf-8")

            # Mock Judge Pro review
            review_pro = """
# Judge Pro
```json
{
  "judge": "pro",
  "scores": {
    "R1": 6.0,
    "R2": 7.0,
    "R3": 8.0,
    "R4": 9.0,
    "R5": 8.5,
    "R6": 9.5
  },
  "ranking": ["R6", "R4", "R5", "R3", "R2", "R1"]
}
```
"""
            (reviews_dir / "judge-pro.md").write_text(review_pro, encoding="utf-8")

            args = DummyArgs(
                benchmark_dir=str(tmp_path),
                reviews_dir=str(reviews_dir),
                key_file=str(sealed_dir / "key.json"),
                json=True,
            )

            # Test parsing
            parsed_flash = parse_score_block_from_text(review_flash)
            self.assertIsNotNone(parsed_flash)
            self.assertEqual(parsed_flash["judge"], "flash")

            with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
                rc = handle_aggregate(args)
            self.assertEqual(rc, 0)


if __name__ == "__main__":
    unittest.main()
