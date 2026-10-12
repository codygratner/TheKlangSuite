#!/usr/bin/env python3
"""
da_bench.py - Six-Way Devil's Advocate Benchmark Harness (Card rj-22)

Harness for the six-way Devil's Advocate benchmark across Flash, Pro, and Opus.
Provides CLI subcommands:
  extract         - Asserts report availability or extracts from subagent transcripts.
  sanitize        - Strips blindness headers, part markers, redacts model names,
                    shuffles reports deterministically, and seals the key.
  audit-blindness - Audits judge transcripts for forbidden file/workspace access.
  aggregate       - Aggregates multi-judge scores, Kendall tau concordance,
                    prompt method deltas (B - A), self-preference bias, and length stats.
  manifest        - Manages benchmark lifecycle, gate verification, and state transitions.

Standard library only. Universal UTF-8 compliance.
"""

import argparse
from datetime import datetime, timezone
import hashlib
import json
import math
import os
from pathlib import Path
import random
import re
import sys
from typing import Any, Dict, List, Optional, Set, Tuple

# Bootstrap UTF-8 console streams immediately
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

DEFAULT_BENCHMARK_DIR = Path(r"C:\Dev\Research\agentic\rj22_da_benchmark")
CANONICAL_CELLS = ["flash-A", "flash-B", "pro-A", "pro-B", "opus-A", "opus-B"]
MODELS = ["flash", "pro", "opus"]
PROMPT_METHODS = ["A", "B"]

# Terms to preserve during model redaction
PROTECTED_TERMS_PATTERN = re.compile(
    r"\b(?:Apple\s+)?ProRes\b|\bPro\s*Tools\b|\bProTools\b|\bLogic\s+Pro\b|\bMac\s*Pro\b|\bFinal\s+Cut\s+Pro\b|\bPro\s+Audio\b",
    re.IGNORECASE,
)

# Standalone model tokens to redact (case-insensitive)
MODEL_TOKENS_PATTERN = re.compile(
    r"\b(Opus|Claude|Anthropic|Gemini|Flash|Pro)\b",
    re.IGNORECASE,
)


# =====================================================================
# Sanitization & Redaction Helpers
# =====================================================================

def sanitize_report_text(text: str) -> str:
    """
    Sanitizes raw model report text for blind evaluation:
    1. Strips blindness rule preambles.
    2. Strips multi-part continuation markers and header part numbers.
    3. Redacts model names to [MODEL], strictly preserving Pro Tools / ProRes.
    4. Cleans trailing whitespace and collapses excessive empty lines.
    """
    # 1. Blindness preambles
    text = re.sub(
        r"\(Written under (?:the )?blindness rule:[^\)]*\)\n?",
        "",
        text,
        flags=re.IGNORECASE,
    )
    text = re.sub(r"BLINDNESS RULE:[^\n]*\n?", "", text, flags=re.IGNORECASE)

    # 2. Part continuation lines
    text = re.sub(
        r"\n+\([^\n]*\bPART\s+\d+/\d+[^\n]*\)\.?\n*",
        "\n\n",
        text,
        flags=re.IGNORECASE,
    )
    text = re.sub(
        r"\n+\(Part\s+\d+\s+follows[^\n]*\)\.?\n*",
        "\n\n",
        text,
        flags=re.IGNORECASE,
    )
    text = re.sub(
        r"\n+\(Continued in PART[^\n]*\)\.?\n*",
        "\n\n",
        text,
        flags=re.IGNORECASE,
    )

    # 3. Inline or header part markers like (PART 1/2), (PART 2/2):
    text = re.sub(
        r"\s*\(PART\s+\d+/\d+(?::\s*[^)]*)?\):?",
        "",
        text,
        flags=re.IGNORECASE,
    )

    # 4. Clean leftover standalone lines that are just 'PCDA REPORT'
    cleaned_lines = []
    for line in text.splitlines():
        if re.fullmatch(r"#*\s*PCDA\s+REPORT\s*", line.strip(), re.IGNORECASE):
            continue
        cleaned_lines.append(line)
    text = "\n".join(cleaned_lines)

    # 5. Model name redaction with preservation of protected terms
    placeholders: List[str] = []

    def protect_match(m: re.Match) -> str:
        placeholders.append(m.group(0))
        return f"__PRESERVED_TERM_{len(placeholders) - 1}__"

    text = PROTECTED_TERMS_PATTERN.sub(protect_match, text)

    # Replace benchmark indicators
    text = re.sub(
        r"(\*\*Benchmark\*\*:\s*)\w+(\s*Model)",
        r"\1[MODEL]\2",
        text,
        flags=re.IGNORECASE,
    )
    text = re.sub(
        r"\(Benchmark:\s*\w+\s*Model\)",
        r"(Benchmark: [MODEL] Model)",
        text,
        flags=re.IGNORECASE,
    )

    # Redact model names
    text = MODEL_TOKENS_PATTERN.sub("[MODEL]", text)

    # Restore protected terms
    for idx, orig in enumerate(placeholders):
        text = text.replace(f"__PRESERVED_TERM_{idx}__", orig)

    # Normalize whitespace
    text = re.sub(r"\n{3,}", "\n\n", text)
    if not text.endswith("\n"):
        text += "\n"

    return text


def compute_sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


# =====================================================================
# Kendall Rank Correlation (Tau-b)
# =====================================================================

def calculate_kendall_tau(
    ranking_a: List[str],
    ranking_b: List[str],
    items: Optional[List[str]] = None,
) -> float:
    """
    Calculates Kendall rank correlation coefficient (tau-b) between two rankings.
    Concordance range: [-1.0, 1.0].
    """
    if items is None:
        items = sorted(list(set(ranking_a) | set(ranking_b)))

    n = len(items)
    if n < 2:
        return 1.0

    rank_a = {item: idx for idx, item in enumerate(ranking_a)}
    rank_b = {item: idx for idx, item in enumerate(ranking_b)}

    concordant = 0
    discordant = 0
    ties_a = 0
    ties_b = 0

    for i in range(n):
        for j in range(i + 1, n):
            it_i, it_j = items[i], items[j]
            ra_i, ra_j = rank_a.get(it_i, n), rank_a.get(it_j, n)
            rb_i, rb_j = rank_b.get(it_i, n), rank_b.get(it_j, n)

            diff_a = (ra_i > ra_j) - (ra_i < ra_j)
            diff_b = (rb_i > rb_j) - (rb_i < rb_j)

            if diff_a == 0 and diff_b == 0:
                continue
            elif diff_a == 0:
                ties_a += 1
            elif diff_b == 0:
                ties_b += 1
            elif (diff_a > 0 and diff_b > 0) or (diff_a < 0 and diff_b < 0):
                concordant += 1
            else:
                discordant += 1

    total_pairs = n * (n - 1) / 2.0
    denom = math.sqrt((total_pairs - ties_a) * (total_pairs - ties_b))
    if denom == 0.0:
        return 0.0

    return (concordant - discordant) / denom


# =====================================================================
# Subcommand: extract
# =====================================================================

def extract_from_transcript(transcript_path: Path, out_path: Path) -> int:
    """Extracts send_message payloads from a subagent transcript into a markdown file."""
    if not transcript_path.exists():
        print(f"ERROR: Transcript not found at {transcript_path}", file=sys.stderr)
        return 1

    messages: List[str] = []
    with open(transcript_path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            try:
                data = json.loads(line)
            except json.JSONDecodeError:
                continue
            for call in data.get("tool_calls", []):
                if call.get("name") == "send_message":
                    msg = call.get("args", {}).get("Message", "")
                    if msg:
                        messages.append(msg)

    if not messages:
        print(f"ERROR: No send_message tool calls found in {transcript_path}", file=sys.stderr)
        return 1

    out_path.parent.mkdir(parents=True, exist_ok=True)
    joined_text = "\n\n".join(messages) + "\n"
    with open(out_path, "w", encoding="utf-8", newline="\n") as f:
        f.write(joined_text)

    print(f"Extracted {len(messages)} message(s) ({len(joined_text)} chars) to {out_path}")
    return 0


def handle_extract(args: argparse.Namespace) -> int:
    bench_dir = Path(args.benchmark_dir).resolve()
    reports_dir = bench_dir / "reports"

    if args.from_transcript and args.out_report:
        out_file = reports_dir / args.out_report
        return extract_from_transcript(Path(args.from_transcript), out_file)

    reports_dir.mkdir(parents=True, exist_ok=True)
    print(f"--- Benchmark Reports Status ({reports_dir}) ---")

    found_count = 0
    rows: List[Tuple[str, str, str, str, str]] = []

    for cell in CANONICAL_CELLS:
        rf = reports_dir / f"{cell}.md"
        if rf.exists() and rf.stat().st_size > 0:
            found_count += 1
            data = rf.read_bytes()
            text = data.decode("utf-8", errors="replace")
            words = len(text.split())
            sha = compute_sha256(data)[:12]
            rows.append((cell, "FOUND", f"{len(data)} B", f"{words} w", sha))
        else:
            rows.append((cell, "MISSING", "-", "-", "-"))

    # Print summary table
    print(f"{'Cell':<10} | {'Status':<8} | {'Size':<10} | {'Words':<8} | {'SHA-256'}")
    print("-" * 55)
    for c, st, sz, w, sha in rows:
        print(f"{c:<10} | {st:<8} | {sz:<10} | {w:<8} | {sha}")

    print("-" * 55)
    print(f"Total reports present: {found_count}/{len(CANONICAL_CELLS)}")

    if found_count == len(CANONICAL_CELLS) or args.allow_partial:
        return 0
    print(f"ERROR: Expected all {len(CANONICAL_CELLS)} reports. Run with --allow-partial to bypass.", file=sys.stderr)
    return 1


# =====================================================================
# Subcommand: sanitize
# =====================================================================

def handle_sanitize(args: argparse.Namespace) -> int:
    bench_dir = Path(args.benchmark_dir).resolve()
    reports_dir = Path(args.reports_dir).resolve() if args.reports_dir else bench_dir / "reports"
    anonymized_dir = Path(args.out_dir).resolve() if args.out_dir else bench_dir / "anonymized"
    key_file = Path(args.key_file).resolve() if args.key_file else bench_dir / "sealed" / "key.json"

    anonymized_dir.mkdir(parents=True, exist_ok=True)
    key_file.parent.mkdir(parents=True, exist_ok=True)

    available_cells: List[str] = []
    for cell in CANONICAL_CELLS:
        rf = reports_dir / f"{cell}.md"
        if rf.exists() and rf.stat().st_size > 0:
            available_cells.append(cell)

    if not available_cells:
        print(f"ERROR: No reports found in {reports_dir}", file=sys.stderr)
        return 1

    if len(available_cells) < len(CANONICAL_CELLS) and not args.allow_partial:
        print(
            f"ERROR: Only {len(available_cells)}/{len(CANONICAL_CELLS)} reports present. Use --allow-partial for partial sanitization.",
            file=sys.stderr,
        )
        return 1

    seed = args.seed if args.seed is not None else random.randint(10000, 999999)
    rng = random.Random(seed)
    shuffled_cells = list(available_cells)
    rng.shuffle(shuffled_cells)

    mapping: Dict[str, str] = {}
    reverse_mapping: Dict[str, str] = {}
    reports_meta: Dict[str, Any] = {}

    for idx, cell in enumerate(shuffled_cells, start=1):
        r_id = f"R{idx}"
        mapping[r_id] = cell
        reverse_mapping[cell] = r_id

        src_file = reports_dir / f"{cell}.md"
        with open(src_file, "r", encoding="utf-8") as f:
            raw_text = f.read()

        cleaned_text = sanitize_report_text(raw_text)
        out_file = anonymized_dir / f"{r_id}.md"
        with open(out_file, "w", encoding="utf-8", newline="\n") as f:
            f.write(cleaned_text)

        cleaned_bytes = cleaned_text.encode("utf-8")
        reports_meta[r_id] = {
            "cell": cell,
            "source_file": src_file.name,
            "sha256": compute_sha256(cleaned_bytes),
            "chars": len(cleaned_text),
            "words": len(cleaned_text.split()),
            "lines": len(cleaned_text.splitlines()),
        }

    key_data = {
        "benchmark_id": "rj-22",
        "created_at": datetime.now(timezone.utc).isoformat(),
        "seed": seed,
        "cell_count": len(shuffled_cells),
        "mapping": mapping,
        "reverse_mapping": reverse_mapping,
        "reports": reports_meta,
    }

    with open(key_file, "w", encoding="utf-8", newline="\n") as f:
        json.dump(key_data, f, indent=2)

    print("=================================================================")
    print("🔒 SANITIZATION & ANONYMIZATION COMPLETE")
    print(f"Seed: {seed} | Reports anonymized: {len(shuffled_cells)}")
    print(f"Output: {anonymized_dir}")
    print(f"Sealed Key: {key_file}")
    print("=================================================================")
    print(f"{'ID':<6} | {'Cell':<10} | {'Chars':<8} | {'Words':<8} | {'SHA-256'}")
    print("-" * 55)
    for r_id, meta in reports_meta.items():
        print(f"{r_id:<6} | {meta['cell']:<10} | {meta['chars']:<8} | {meta['words']:<8} | {meta['sha256'][:12]}")
    print("-" * 55)

    return 0


# =====================================================================
# Subcommand: audit-blindness
# =====================================================================

FORBIDDEN_KEYWORDS = [
    "sealed/key.json",
    "sealed\\key.json",
    "key.json",
    "ground_truth.md",
    "rj22_ground_truth",
    "souls/rj22_benchmark",
    "souls\\rj22_benchmark",
    "flash_report.md",
    "pro_report.md",
    "opus_report.md",
]


def audit_single_transcript(transcript_path: Path, bench_dir: Path) -> List[Dict[str, Any]]:
    violations: List[Dict[str, Any]] = []
    anonymized_dir_str = str(bench_dir / "anonymized").lower()
    reports_dir_str = str(bench_dir / "reports").lower()
    sealed_dir_str = str(bench_dir / "sealed").lower()
    prompts_dir_str = str(bench_dir / "prompts").lower()

    with open(transcript_path, "r", encoding="utf-8") as f:
        for line_no, line in enumerate(f, start=1):
            line = line.strip()
            if not line:
                continue
            try:
                data = json.loads(line)
            except json.JSONDecodeError:
                continue

            tool_calls = data.get("tool_calls", [])
            for call in tool_calls:
                tool_name = call.get("name", "")
                args = call.get("args", {})

                # Check view_file
                if tool_name == "view_file":
                    target = str(args.get("AbsolutePath", "")).lower()
                    if reports_dir_str in target:
                        violations.append({
                            "line": line_no,
                            "tool": tool_name,
                            "target": target,
                            "reason": "Accessed un-anonymized reports directory",
                        })
                    elif sealed_dir_str in target:
                        violations.append({
                            "line": line_no,
                            "tool": tool_name,
                            "target": target,
                            "reason": "Accessed sealed de-anonymization key",
                        })
                    elif prompts_dir_str in target or "ground_truth.md" in target:
                        violations.append({
                            "line": line_no,
                            "tool": tool_name,
                            "target": target,
                            "reason": "Accessed ground truth or prompt files",
                        })
                    elif any(kw in target for kw in FORBIDDEN_KEYWORDS):
                        violations.append({
                            "line": line_no,
                            "tool": tool_name,
                            "target": target,
                            "reason": "Accessed forbidden benchmark source",
                        })

                # Check run_command
                elif tool_name == "run_command":
                    cmd = str(args.get("CommandLine", "")).lower()
                    if any(kw in cmd for kw in FORBIDDEN_KEYWORDS):
                        violations.append({
                            "line": line_no,
                            "tool": tool_name,
                            "target": cmd,
                            "reason": "Executed command referencing forbidden benchmark artifacts",
                        })
                    if reports_dir_str in cmd or sealed_dir_str in cmd:
                        violations.append({
                            "line": line_no,
                            "tool": tool_name,
                            "target": cmd,
                            "reason": "Command accessed reports/ or sealed/",
                        })

                # Check web queries
                elif tool_name in {"search_web", "read_url_content"}:
                    query_val = str(args.get("query", "") or args.get("Url", "")).lower()
                    if any(kw in query_val for kw in ["rj22", "key.json", "flash_report.md", "opus_report.md"]):
                        violations.append({
                            "line": line_no,
                            "tool": tool_name,
                            "target": query_val,
                            "reason": "Web query leaked confidential benchmark keys",
                        })

    return violations


def handle_audit_blindness(args: argparse.Namespace) -> int:
    bench_dir = Path(args.benchmark_dir).resolve()
    transcripts: List[Path] = []

    if args.transcript:
        transcripts.append(Path(args.transcript).resolve())
    elif args.transcript_dir:
        td = Path(args.transcript_dir).resolve()
        transcripts = sorted(list(td.glob("*.jsonl")))

    if not transcripts:
        print("ERROR: No transcript files provided. Use --transcript or --transcript-dir.", file=sys.stderr)
        return 1

    total_violations = 0
    print(f"Auditing {len(transcripts)} transcript(s) for blindness compliance...")

    for t_path in transcripts:
        if not t_path.exists():
            print(f"WARNING: Transcript does not exist: {t_path}", file=sys.stderr)
            continue
        v_list = audit_single_transcript(t_path, bench_dir)
        if v_list:
            total_violations += len(v_list)
            print(f"❌ VIOLATION in {t_path.name}: {len(v_list)} forbidden access event(s)")
            for v in v_list:
                print(f"   [Line {v['line']}] {v['tool']} -> {v['reason']} ({v['target']})")
        else:
            print(f"✅ CLEAN: {t_path.name} (100% blind compliance)")

    if total_violations > 0:
        print(f"\nAUDIT FAILED: Found {total_violations} blindness breach(es).", file=sys.stderr)
        return 1

    print("\nAUDIT PASSED: 100% blind compliance verified across all transcripts.")
    return 0


# =====================================================================
# Subcommand: aggregate
# =====================================================================

def parse_score_block_from_text(text: str) -> Optional[Dict[str, Any]]:
    """Finds and parses the embedded JSON scorecard from a review markdown."""
    fence_pattern = re.compile(r"```(?:json)?\s*(\{[\s\S]*?\})\s*```")
    for match in fence_pattern.finditer(text):
        try:
            parsed = json.loads(match.group(1))
            if "scores" in parsed or "ranking" in parsed:
                return parsed
        except json.JSONDecodeError:
            continue

    # Fallback: search for bare JSON containing "scores" and "ranking"
    bare_pattern = re.compile(r"(\{\s*\"scores\"[\s\S]*?\"ranking\"[\s\S]*?\})")
    for match in bare_pattern.finditer(text):
        try:
            return json.loads(match.group(1))
        except json.JSONDecodeError:
            continue

    return None


def handle_aggregate(args: argparse.Namespace) -> int:
    bench_dir = Path(args.benchmark_dir).resolve()
    reviews_dir = Path(args.reviews_dir).resolve() if args.reviews_dir else bench_dir / "reviews"
    key_file = Path(args.key_file).resolve() if args.key_file else bench_dir / "sealed" / "key.json"
    anonymized_dir = bench_dir / "anonymized"

    key_data: Dict[str, Any] = {}
    mapping: Dict[str, str] = {}
    if key_file.exists():
        with open(key_file, "r", encoding="utf-8") as f:
            key_data = json.load(f)
            mapping = key_data.get("mapping", {})

    review_files = sorted(list(reviews_dir.glob("*.md")))
    if not review_files:
        print(f"ERROR: No review files found in {reviews_dir}", file=sys.stderr)
        return 1

    judges_data: Dict[str, Dict[str, Any]] = {}
    for rf in review_files:
        with open(rf, "r", encoding="utf-8") as f:
            content = f.read()
        score_block = parse_score_block_from_text(content)
        if not score_block:
            print(f"WARNING: Could not parse JSON score block from {rf.name}", file=sys.stderr)
            continue

        judge_name = score_block.get("judge")
        if not judge_name:
            m = re.search(r"judge[-_](\w+)", rf.stem, re.IGNORECASE)
            judge_name = m.group(1).lower() if m else rf.stem

        judges_data[judge_name] = score_block

    if not judges_data:
        print("ERROR: No valid judge scorecards could be parsed.", file=sys.stderr)
        return 1

    # Extract all report IDs
    all_report_ids: Set[str] = set()
    for j_data in judges_data.values():
        all_report_ids.update(j_data.get("ranking", []))
        all_report_ids.update(j_data.get("scores", {}).keys())

    sorted_r_ids = sorted(list(all_report_ids))

    # Calculate per-judge scores and ranks
    scores_by_judge_report: Dict[str, Dict[str, float]] = {}
    ranks_by_judge_report: Dict[str, Dict[str, int]] = {}

    for j_name, j_data in judges_data.items():
        scores_by_judge_report[j_name] = {}
        raw_scores = j_data.get("scores", {})
        for r_id in sorted_r_ids:
            val = raw_scores.get(r_id)
            if isinstance(val, dict):
                overall = val.get("overall")
                if overall is not None:
                    score_val = float(overall)
                else:
                    nums = [float(v) for v in val.values() if isinstance(v, (int, float))]
                    score_val = sum(nums) / len(nums) if nums else 0.0
            elif isinstance(val, (int, float)):
                score_val = float(val)
            else:
                score_val = 0.0
            scores_by_judge_report[j_name][r_id] = score_val

        # Ranking
        ranking = j_data.get("ranking", [])
        ranks_by_judge_report[j_name] = {}
        for rank_idx, r_id in enumerate(ranking, start=1):
            ranks_by_judge_report[j_name][r_id] = rank_idx

    # Kendall tau concordance matrix
    judge_names = sorted(list(judges_data.keys()))
    tau_matrix: Dict[Tuple[str, str], float] = {}
    for i in range(len(judge_names)):
        for j in range(len(judge_names)):
            j1 = judge_names[i]
            j2 = judge_names[j]
            if i == j:
                tau_matrix[(j1, j2)] = 1.0
            else:
                r1 = judges_data[j1].get("ranking", [])
                r2 = judges_data[j2].get("ranking", [])
                tau_matrix[(j1, j2)] = calculate_kendall_tau(r1, r2, sorted_r_ids)

    # Compute per-cell statistics
    cell_stats: Dict[str, Dict[str, Any]] = {}
    for r_id in sorted_r_ids:
        cell_name = mapping.get(r_id, r_id)
        sc_list = [scores_by_judge_report[j][r_id] for j in judge_names]
        rk_list = [ranks_by_judge_report[j].get(r_id, len(sorted_r_ids)) for j in judge_names]
        mean_score = sum(sc_list) / len(sc_list) if sc_list else 0.0
        mean_rank = sum(rk_list) / len(rk_list) if rk_list else 0.0

        # Word count from anonymized or report meta
        word_count = 0
        char_count = 0
        anonymized_file = anonymized_dir / f"{r_id}.md"
        if anonymized_file.exists():
            txt = anonymized_file.read_text(encoding="utf-8")
            char_count = len(txt)
            word_count = len(txt.split())

        cell_stats[cell_name] = {
            "r_id": r_id,
            "cell": cell_name,
            "scores": {j: scores_by_judge_report[j][r_id] for j in judge_names},
            "mean_score": round(mean_score, 2),
            "ranks": {j: ranks_by_judge_report[j].get(r_id) for j in judge_names},
            "mean_rank": round(mean_rank, 2),
            "words": word_count,
            "chars": char_count,
            "tokens_est": char_count // 4,
        }

    # Effect of Prompt Method (Method B - Method A)
    method_deltas: Dict[str, Dict[str, float]] = {}
    for mod in MODELS:
        cell_a = f"{mod}-A"
        cell_b = f"{mod}-B"
        if cell_a in cell_stats and cell_b in cell_stats:
            d_score = cell_stats[cell_b]["mean_score"] - cell_stats[cell_a]["mean_score"]
            d_rank = cell_stats[cell_a]["mean_rank"] - cell_stats[cell_b]["mean_rank"]  # lower rank is better
            method_deltas[mod] = {
                "score_delta": round(d_score, 2),
                "rank_improvement": round(d_rank, 2),
            }

    # Self-preference bias
    self_bias: Dict[str, Dict[str, Any]] = {}
    for j in judge_names:
        mod_match = next((m for m in MODELS if m in j.lower()), None)
        if mod_match:
            own_scores: List[float] = []
            other_scores: List[float] = []
            for c_name, st in cell_stats.items():
                sc = st["scores"][j]
                if c_name.startswith(f"{mod_match}-"):
                    own_scores.append(sc)
                else:
                    other_scores.append(sc)
            avg_own = sum(own_scores) / len(own_scores) if own_scores else 0.0
            avg_other = sum(other_scores) / len(other_scores) if other_scores else 0.0
            self_bias[j] = {
                "model": mod_match,
                "avg_own_score": round(avg_own, 2),
                "avg_other_score": round(avg_other, 2),
                "bias_delta": round(avg_own - avg_other, 2),
            }

    # Print / Output results
    if args.json:
        summary_payload = {
            "judges": judge_names,
            "cell_stats": cell_stats,
            "method_deltas": method_deltas,
            "kendall_tau": {f"{k[0]}_vs_{k[1]}": round(v, 4) for k, v in tau_matrix.items()},
            "self_preference_bias": self_bias,
        }
        print(json.dumps(summary_payload, indent=2))
        return 0

    print("=================================================================")
    print("📊 SIX-WAY DEVIL'S ADVOCATE BENCHMARK: SCORECARD")
    print("=================================================================")
    print(f"\n### 1. Overall Rankings & Scores (Judges: {', '.join(judge_names)})\n")

    header_cols = ["ID", "Cell", "Words"] + [f"J-{j}" for j in judge_names] + ["Mean Sc", "Mean Rk"]
    print(f"{header_cols[0]:<5} | {header_cols[1]:<10} | {header_cols[2]:<7} | " +
          " | ".join(f"{h:<6}" for h in header_cols[3:-2]) +
          f" | {header_cols[-2]:<7} | {header_cols[-1]:<7}")
    print("-" * (50 + 9 * len(judge_names)))

    sorted_cells = sorted(cell_stats.values(), key=lambda x: x["mean_score"], reverse=True)
    for c in sorted_cells:
        scores_str = " | ".join(f"{c['scores'][j]:<6.1f}" for j in judge_names)
        print(f"{c['r_id']:<5} | {c['cell']:<10} | {c['words']:<7} | {scores_str} | {c['mean_score']:<7.2f} | {c['mean_rank']:<7.2f}")

    if method_deltas:
        print("\n### 2. Method Effect Delta (Method B minus Method A)\n")
        print(f"{'Model':<10} | {'Score Delta (B - A)':<20} | {'Rank Improvement':<18}")
        print("-" * 55)
        for mod, d in method_deltas.items():
            print(f"{mod:<10} | {d['score_delta']:<+20.2f} | {d['rank_improvement']:<+18.2f}")

    if len(judge_names) > 1:
        print("\n### 3. Inter-Judge Concordance (Kendall's Tau-b Matrix)\n")
        print(f"{'Judge Pair':<25} | {'Tau-b':<8} | {'Concordance'}")
        print("-" * 55)
        for i in range(len(judge_names)):
            for j in range(i + 1, len(judge_names)):
                j1, j2 = judge_names[i], judge_names[j]
                tau_val = tau_matrix[(j1, j2)]
                qual = "Strong" if tau_val >= 0.7 else ("Moderate" if tau_val >= 0.4 else "Low/Disagreement")
                print(f"{f'{j1} vs {j2}':<25} | {tau_val:<+8.4f} | {qual}")

    if self_bias:
        print("\n### 4. Self-Preference Bias Indicator\n")
        print(f"{'Judge':<12} | {'Own Model Avg':<14} | {'Other Models Avg':<16} | {'Bias Delta'}")
        print("-" * 60)
        for j, b in self_bias.items():
            print(f"{j:<12} | {b['avg_own_score']:<14.2f} | {b['avg_other_score']:<16.2f} | {b['bias_delta']:<+10.2f}")

    return 0


# =====================================================================
# Subcommand: manifest
# =====================================================================

def load_manifest(manifest_path: Path) -> Dict[str, Any]:
    if not manifest_path.exists():
        return {}
    with open(manifest_path, "r", encoding="utf-8") as f:
        return json.load(f)


def save_manifest(manifest_path: Path, data: Dict[str, Any]) -> None:
    manifest_path.parent.mkdir(parents=True, exist_ok=True)
    data["updated_at"] = datetime.now(timezone.utc).isoformat()
    with open(manifest_path, "w", encoding="utf-8", newline="\n") as f:
        json.dump(data, f, indent=2)


def handle_manifest(args: argparse.Namespace) -> int:
    bench_dir = Path(args.benchmark_dir).resolve()
    manifest_path = bench_dir / "manifest.json"

    action = args.manifest_action

    if action == "show":
        manifest = load_manifest(manifest_path)
        if not manifest:
            print(f"ERROR: manifest.json not found at {manifest_path}", file=sys.stderr)
            return 1

        print("=================================================================")
        print(f"📋 BENCHMARK MANIFEST: {manifest.get('title', 'Unknown')} ({manifest.get('benchmark_id')})")
        print(f"Status: {manifest.get('status')} | Updated: {manifest.get('updated_at')}")
        print("=================================================================")

        print("\nStages:")
        stages = manifest.get("stages", {})
        for s_id, s_info in stages.items():
            st = s_info.get("status", "UNKNOWN")
            print(f"  [{st:<9}] Stage {s_id}: {s_info.get('name')}")

        print("\nCells:")
        cells = manifest.get("cells", {})
        for c_id, c_info in cells.items():
            print(f"  {c_id:<10}: model={c_info.get('model')} method={c_info.get('method')} status={c_info.get('status')}")

        return 0

    elif action == "check":
        through = getattr(args, "through", "W2")
        manifest = load_manifest(manifest_path)
        if not manifest:
            print(f"ERROR: manifest.json not found at {manifest_path}", file=sys.stderr)
            return 1

        errors: List[str] = []

        # W1 & W2 required artifacts
        required_w2 = [
            bench_dir / "prompts" / "method_A.txt",
            bench_dir / "prompts" / "method_B.txt",
            bench_dir / "prompts" / "prompts.sha256",
            bench_dir / "reports" / "flash-A.md",
            bench_dir / "reports" / "pro-A.md",
            bench_dir / "reports" / "opus-A.md",
            bench_dir / "reports" / "opus-B.md",
            bench_dir / "ground_truth.md",
        ]

        for p in required_w2:
            if not p.exists() or p.stat().st_size == 0:
                errors.append(f"Missing or empty W2 artifact: {p.relative_to(bench_dir)}")

        # Verify prompt hashes
        prompt_sha = bench_dir / "prompts" / "prompts.sha256"
        if prompt_sha.exists():
            sha_lines = prompt_sha.read_text(encoding="utf-8").splitlines()
            expected_hashes: Dict[str, str] = {}
            for line in sha_lines:
                parts = line.split()
                if len(parts) >= 2:
                    expected_hashes[parts[1]] = parts[0]

            method_a = bench_dir / "prompts" / "method_A.txt"
            if method_a.exists():
                h_a = compute_sha256(method_a.read_bytes())
                if expected_hashes.get("method_A.txt") and h_a != expected_hashes["method_A.txt"]:
                    errors.append(f"method_A.txt SHA mismatch: got {h_a}, expected {expected_hashes['method_A.txt']}")

            method_b = bench_dir / "prompts" / "method_B.txt"
            if method_b.exists():
                h_b = compute_sha256(method_b.read_bytes())
                if expected_hashes.get("method_B.txt") and h_b != expected_hashes["method_B.txt"]:
                    errors.append(f"method_B.txt SHA mismatch: got {h_b}, expected {expected_hashes['method_B.txt']}")

        if through in {"W3", "W4"}:
            required_w3 = [
                bench_dir / "reports" / "flash-B.md",
                bench_dir / "reports" / "pro-B.md",
                bench_dir / "sealed" / "key.json",
                bench_dir / "reviews" / "judge-flash.md",
                bench_dir / "reviews" / "judge-pro.md",
                bench_dir / "meta" / "provisional_flash.md",
                bench_dir / "meta" / "provisional_pro.md",
            ]
            for i in range(1, 7):
                required_w3.append(bench_dir / "anonymized" / f"R{i}.md")

            for p in required_w3:
                if not p.exists() or p.stat().st_size == 0:
                    errors.append(f"Missing or empty W3 artifact: {p.relative_to(bench_dir)}")

        if through == "W4":
            required_w4 = [
                bench_dir / "reviews" / "judge-opus.md",
                bench_dir / "meta" / "final_meta.md",
                bench_dir / "meta" / "real_world_impact.md",
            ]
            for p in required_w4:
                if not p.exists() or p.stat().st_size == 0:
                    errors.append(f"Missing or empty W4 artifact: {p.relative_to(bench_dir)}")

        if errors:
            print(f"❌ MANIFEST CHECK FAILED (Threshold: {through}):", file=sys.stderr)
            for err in errors:
                print(f"  • {err}", file=sys.stderr)
            return 1

        print(f"✅ MANIFEST CHECK PASSED: All stage artifacts through {through} verified.")
        return 0

    elif action == "set":
        manifest = load_manifest(manifest_path)
        if not manifest:
            manifest = {
                "benchmark_id": "rj-22",
                "title": "Six-Way Devil's Advocate Benchmark",
                "created_at": datetime.now(timezone.utc).isoformat(),
            }

        key = args.key
        val = args.val

        # Type conversion for booleans and numbers
        parsed_val: Any = val
        if val.lower() == "true":
            parsed_val = True
        elif val.lower() == "false":
            parsed_val = False
        else:
            try:
                parsed_val = int(val)
            except ValueError:
                try:
                    parsed_val = float(val)
                except ValueError:
                    parsed_val = val

        # Handle dotted keys: stages.W3.status
        parts = key.split(".")
        curr = manifest
        for part in parts[:-1]:
            if part not in curr or not isinstance(curr[part], dict):
                curr[part] = {}
            curr = curr[part]

        curr[parts[-1]] = parsed_val
        save_manifest(manifest_path, manifest)
        print(f"Updated manifest: {key} = {parsed_val}")
        return 0

    print(f"ERROR: Unknown manifest action '{action}'", file=sys.stderr)
    return 1


# =====================================================================
# Main CLI Entrypoint
# =====================================================================

def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Six-Way Devil's Advocate Benchmark Harness (rj-22)",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "--benchmark-dir",
        "-d",
        default=str(DEFAULT_BENCHMARK_DIR),
        help="Root directory of rj22_da_benchmark",
    )

    subparsers = parser.add_subparsers(dest="subcommand", required=True)

    # Subcommand: extract
    p_extract = subparsers.add_parser("extract", help="Asserts reports status or extracts from transcripts")
    p_extract.add_argument("--allow-partial", action="store_true", help="Allow partial reports count")
    p_extract.add_argument("--from-transcript", help="Path to transcript.jsonl to extract from")
    p_extract.add_argument("--out-report", help="Target filename in reports/ (e.g. opus-A.md)")

    # Subcommand: sanitize
    p_sanitize = subparsers.add_parser("sanitize", help="Strips preambles, redacts model names, and anonymizes")
    p_sanitize.add_argument("--seed", type=int, default=None, help="Random seed for shuffling")
    p_sanitize.add_argument("--allow-partial", action="store_true", help="Sanitize whichever reports are present")
    p_sanitize.add_argument("--reports-dir", help="Override reports directory")
    p_sanitize.add_argument("--out-dir", help="Override anonymized output directory")
    p_sanitize.add_argument("--key-file", help="Override sealed key path")

    # Subcommand: audit-blindness
    p_audit = subparsers.add_parser("audit-blindness", help="Audits JSONL transcripts for blindness violations")
    p_audit.add_argument("--transcript", help="Path to a single .jsonl transcript")
    p_audit.add_argument("--transcript-dir", help="Directory of .jsonl transcripts")

    # Subcommand: aggregate
    p_aggregate = subparsers.add_parser("aggregate", help="Aggregates judge reviews, scores, and correlations")
    p_aggregate.add_argument("--reviews-dir", help="Override reviews directory")
    p_aggregate.add_argument("--key-file", help="Override sealed key path")
    p_aggregate.add_argument("--json", action="store_true", help="Output raw summary as JSON")

    # Subcommand: manifest
    p_manifest = subparsers.add_parser("manifest", help="Manages benchmark manifest lifecycle")
    p_manifest_sub = p_manifest.add_subparsers(dest="manifest_action", required=True)

    p_man_show = p_manifest_sub.add_parser("show", help="Shows manifest summary")
    p_man_check = p_manifest_sub.add_parser("check", help="Validates stage artifacts and checksums")
    p_man_check.add_argument("--through", choices=["W2", "W3", "W4"], default="W2", help="Stage verification threshold")

    p_man_set = p_manifest_sub.add_parser("set", help="Sets a key in manifest.json")
    p_man_set.add_argument("key", help="Key or dotted path (e.g. status, stages.W3.status)")
    p_man_set.add_argument("val", help="Value to set")

    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()

    if args.subcommand == "extract":
        return handle_extract(args)
    elif args.subcommand == "sanitize":
        return handle_sanitize(args)
    elif args.subcommand == "audit-blindness":
        return handle_audit_blindness(args)
    elif args.subcommand == "aggregate":
        return handle_aggregate(args)
    elif args.subcommand == "manifest":
        return handle_manifest(args)

    return 0


if __name__ == "__main__":
    sys.exit(main())
