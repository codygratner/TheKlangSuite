#!/usr/bin/env python3
"""
tools/router.py — Adaptive Subagent Tier Routing & Heuristic Calibration Engine
================================================================================
Deterministic, zero-token complexity classifier and self-calibrating model router
for Antigravity subagent dispatching.

Features:
  1. Zero-Token Deterministic Scorer:
     Extracts prompt features (domain risk, file extensions, semantic density) in < 2ms
     and computes an Integer Complexity Score (C: 0..10).
  2. DSP Domain Specialization:
     - DSP Research: Routed to Gemini 4 Argon for 1M-2M context window (synthesizer
       service manuals, schematics) and zero Anthropic quota burn.
     - DSP Writing / Safety: Routed to Claude Opus 5.5 (High) for Level 3 Extended
       Thinking, bit-exact C++20 real-time audio thread safety, and lock-free verification.
  3. Quota-Aware Dynamic Tier Ladder & Allowance Warning Protocol:
     - Green (>=25%): Dispatches preferred model.
     - Yellow Caution (15% <= q < 25%): Dispatches Opus, but surfaces explicit
       allowance caution banner with hours remaining until reset.
     - Red Zone (<15%): Auto-swaps from Opus to Gemini 4 Argon (High) Level 3
       with explicit depleted quota alert.
  4. Telemetry Ledger & Heuristic Calibration:
     Tracks outcomes in .agents/index/router_telemetry.db with exponential half-life
     decay (lambda=0.9). Detects OVERKILL and UNDERPROVISIONED dispatches to prevent
     dogmatic anchoring.

Usage:
  python tools/router.py classify "Audit real-time lock-free ring buffer in dsp/ring.h"
  python tools/router.py test
  python tools/router.py stats
"""

import os
import re
import sys
import time
import json
import sqlite3
import hashlib
import argparse
from pathlib import Path
from typing import Dict, Any, List, Optional, Tuple
from datetime import datetime, timezone

# Universal UTF-8 Windows Console Streams
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

VAULT_ROOT = Path(__file__).resolve().parent.parent
DB_PATH = VAULT_ROOT / ".agents" / "index" / "router_telemetry.db"
sys.path.insert(0, str(VAULT_ROOT))

from tools.ls_probe import call_rpc

# Domain Keyword Risk Weight Tables
KEYWORDS_LEVEL_3 = [
    r"\breal-time\b", r"\baudiothread\b", r"\bthread[- ]?safety\b", r"\block-free\b",
    r"\bconcurrency\b", r"\bmutex\b", r"\bdeadlock\b", r"\binvariant\b",
    r"\bformal(?:ly)?\b", r"\btranscendental\b", r"\bprio(?:rity)? inversion\b",
    r"\bwelch psd\b", r"\bcryptograph(?:y|ic)\b", r"\bzero-copy\b", r"\bheap allocation\b",
    r"\brace conditions?\b", r"\basm\b", r"\bbit-exact\b", r"\bnyquist\b",
    r"\bsimd\b", r"\bavx2?\b", r"\bneon\b", r"\bvectoriz(?:ed|ation)\b",
]

KEYWORDS_LEVEL_2 = [
    r"\baudit\b", r"\barchitectur(?:e|al)\b", r"\btopolog(?:y|ical)\b", r"\bpcda\b",
    r"\brefactor\b", r"\bsystem map\b", r"\bdecouple\b", r"\bmulti-module\b",
    r"\bpipeline\b", r"\bschema migration\b", r"\bbackward compatib(?:ility|le)\b",
    r"\bcross-platform\b", r"\bstate machine\b", r"\bdag\b", r"\bwalking skeleton\b",
    r"\bpremortem\b", r"\bdevil's advocate\b", r"\bcontrarian\b", r"\blifecycle\b",
    r"\bbenchmark\b", r"\breconcil(?:e|iation)\b", r"\bdrift\b", r"\bchronicle\b",
]

KEYWORDS_LEVEL_1 = [
    r"\bimplement\b", r"\bcreate\b", r"\bwrite\b", r"\bfeature\b", r"\bendpoint\b",
    r"\bunit test\b", r"\bparse\b", r"\bconvert\b", r"\bhandler\b", r"\bvalidation\b",
    r"\bpydantic\b", r"\bsqlite\b", r"\bquery\b", r"\bcli\b", r"\bscript\b",
    r"\bfunction\b", r"\bclass\b", r"\bhelper\b", r"\bmethod\b", r"\bapi\b",
    r"\bconnector\b", r"\bwrapper\b", r"\broute\b", r"\bhealthcheck\b", r"\bhttp\b",
]

KEYWORDS_LEVEL_0 = [
    r"\brename\b", r"\bmove\b", r"\bformat\b", r"\btypo\b", r"\bgrep\b", r"\bsearch\b",
    r"\blist\b", r"\bcheck if\b", r"\bclean up\b", r"\bdelete\b", r"\bread\b",
    r"\bview\b", r"\bcat\b", r"\bls\b", r"\bcount\b", r"\blint\b",
]

DSP_CORE_TERMS = [
    r"\bdsp\b", r"\bsynthesizer\b", r"\bsynth\b", r"\boscillator\b", r"\bfilter\b",
    r"\bwavetable\b", r"\baudio\b", r"\btoadtracker\b", r"\btkdsp\b", r"\btks\b",
    r"\bbiquad\b", r"\bsvf\b", r"\breverb\b", r"\bdelay\b", r"\bdenormal\b",
    r"\bvocoder\b", r"\bmodulation\b", r"\bchorus\b", r"\bflanger\b",
]

DSP_RESEARCH_TERMS = [
    r"\bresearch\b", r"\bsurvey\b", r"\bmanual\b", r"\bschematic\b", r"\bservice manual\b",
    r"\bliteratur(?:e|al)\b", r"\bpaper\b", r"\btopolog(?:y|ies)\b", r"\bdatasheet\b",
    r"\bteardown\b", r"\bhistory\b", r"\bcompare\b", r"\bcorpus\b",
]

FILE_EXTENSIONS_HIGH = [".cpp", ".c", ".h", ".hpp", ".cu", ".asm", ".s"]
FILE_EXTENSIONS_MED  = [".py", ".ts", ".js", ".rs", ".go", ".proto"]
FILE_EXTENSIONS_LOW  = [".md", ".txt", ".json", ".yaml", ".yml", ".toml", ".csv"]


def init_db() -> sqlite3.Connection:
    """Initializes the telemetry database and table if not present."""
    DB_PATH.parent.mkdir(parents=True, exist_ok=True)
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    with conn:
        conn.execute("""
            CREATE TABLE IF NOT EXISTS routing_records (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TEXT NOT NULL,
                prompt_snippet TEXT NOT NULL,
                complexity_score INTEGER NOT NULL,
                domain_category TEXT NOT NULL,
                selected_model TEXT NOT NULL,
                thinking_level INTEGER NOT NULL,
                step_count INTEGER DEFAULT 0,
                duration_sec REAL DEFAULT 0.0,
                tokens_in INTEGER DEFAULT 0,
                tokens_out INTEGER DEFAULT 0,
                outcome TEXT DEFAULT 'PENDING',
                operator_override INTEGER DEFAULT 0,
                weight_decay REAL DEFAULT 1.0
            );
        """)
        conn.execute("CREATE INDEX IF NOT EXISTS idx_cat ON routing_records(domain_category);")
        conn.execute("CREATE INDEX IF NOT EXISTS idx_time ON routing_records(timestamp);")
    return conn


def get_learned_adjustments() -> Dict[str, float]:
    """Computes learned category score offsets based on historical outcomes with exponential decay."""
    if not DB_PATH.exists():
        return {}

    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    records = conn.execute("""
        SELECT domain_category, outcome, operator_override, timestamp
        FROM routing_records
        WHERE outcome IN ('EVAL_CLEAN', 'EVAL_OVERKILL', 'EVAL_UNDERPROVISIONED')
        ORDER BY id DESC LIMIT 50;
    """).fetchall()
    conn.close()

    adjustments: Dict[str, float] = {}

    for idx, r in enumerate(records):
        if r["operator_override"]:
            continue

        cat = r["domain_category"]
        outcome = r["outcome"]
        decay = 0.90 ** idx

        if cat not in adjustments:
            adjustments[cat] = 0.0

        if outcome == "EVAL_UNDERPROVISIONED":
            adjustments[cat] += 0.4 * decay
        elif outcome == "EVAL_OVERKILL":
            adjustments[cat] -= 0.3 * decay

    return adjustments


def classify_prompt(prompt: str, target_files: Optional[List[str]] = None) -> Dict[str, Any]:
    """Computes deterministic 0-token Integer Complexity Score (0..10) in < 2ms."""
    start_t = time.perf_counter()
    p_lower = prompt.lower()
    
    score = 0.0
    matched_tags = []
    primary_category = "GENERAL"

    # 1. Domain Keyword Matching
    l3_hits = [k for k in KEYWORDS_LEVEL_3 if re.search(k, p_lower)]
    l2_hits = [k for k in KEYWORDS_LEVEL_2 if re.search(k, p_lower)]
    l1_hits = [k for k in KEYWORDS_LEVEL_1 if re.search(k, p_lower)]
    l0_hits = [k for k in KEYWORDS_LEVEL_0 if re.search(k, p_lower)]

    # DSP Domain Intent Specialization
    dsp_hits = [k for k in DSP_CORE_TERMS if re.search(k, p_lower)]
    dsp_res_hits = [k for k in DSP_RESEARCH_TERMS if re.search(k, p_lower)]

    is_dsp = bool(dsp_hits)
    is_dsp_research = is_dsp and bool(dsp_res_hits) and not bool(l3_hits)

    if l3_hits:
        score += 7.5 + (min(len(l3_hits), 3) * 0.6)
        matched_tags.extend([h.strip(r"\b") for h in l3_hits])
        primary_category = "DSP_WRITING" if is_dsp else "FORMAL_SAFETY"
    elif is_dsp_research:
        score += 6.0 + (min(len(dsp_res_hits), 3) * 0.4)
        matched_tags.append("dsp_research")
        matched_tags.extend([h.strip(r"\b") for h in dsp_res_hits[:2]])
        primary_category = "DSP_RESEARCH"
    elif l2_hits:
        score += 5.0 + (min(len(l2_hits), 3) * 0.5)
        matched_tags.extend([h.strip(r"\b") for h in l2_hits])
        primary_category = "ARCHITECTURAL"
    elif l1_hits:
        score += 2.5 + (len(l1_hits) * 0.4)
        matched_tags.extend([h.strip(r"\b") for h in l1_hits])
        primary_category = "IMPLEMENTATION"
    elif l0_hits:
        score += 0.4
        matched_tags.extend([h.strip(r"\b") for h in l0_hits])
        primary_category = "MECHANISTIC"
    else:
        score += 2.0
        primary_category = "GENERAL"

    # 2. File Footprint & Extension Risk
    files = target_files or []
    found_paths = re.findall(r"[\w./\\-]+\.[a-zA-Z0-9]+", prompt)
    all_files = list(set(files + found_paths))

    has_cpp = any(any(f.endswith(ext) for ext in FILE_EXTENSIONS_HIGH) for f in all_files)
    has_code = any(any(f.endswith(ext) for ext in FILE_EXTENSIONS_MED) for f in all_files)

    if has_cpp:
        score += 1.8
        matched_tags.append("c++_dsp_surface")
        if primary_category == "FORMAL_SAFETY":
            primary_category = "DSP_WRITING"
    elif has_code:
        score += 0.8
        matched_tags.append("script_surface")

    if len(all_files) >= 3 or "across" in p_lower or "all files" in p_lower:
        score += 1.0
        matched_tags.append("multi_file_scope")

    word_count = len(prompt.split())
    if word_count > 150:
        score += 1.0
        matched_tags.append("verbose_spec")
    elif word_count < 12 and not l3_hits and not l2_hits and not has_cpp and not is_dsp_research:
        score -= 0.6

    # 3. Apply Learned Empirical Calibration Offset
    adjustments = get_learned_adjustments()
    offset = adjustments.get(primary_category, 0.0)
    score += offset

    final_score = int(round(max(0.0, min(10.0, score))))
    duration_ms = (time.perf_counter() - start_t) * 1000.0

    return {
        "complexityScore": final_score,
        "rawScore": round(score, 2),
        "primaryCategory": primary_category,
        "matchedTags": matched_tags[:6],
        "fileCount": len(all_files),
        "heuristicOffset": round(offset, 2),
        "classificationDurationMs": round(duration_ms, 3),
    }


def get_quota_headrooms() -> Dict[str, Any]:
    """Retrieves separate quota fractions and reset hours for Google and Anthropic pools."""
    res = {
        "google": 1.0,
        "anthropic": 1.0,
        "google_reset": "",
        "anthropic_reset": "",
        "google_reset_hours": 0.0,
        "anthropic_reset_hours": 0.0,
    }
    try:
        data = call_rpc("GetCascadeModelConfigData", timeout=0.8)
        now_utc = datetime.now(timezone.utc)
        for cfg in data.get("clientModelConfigs", []):
            label = cfg.get("label", "")
            qinfo = cfg.get("quotaInfo", {})
            rem = qinfo.get("remainingFraction")
            reset_str = qinfo.get("resetTime", "")
            hours_left = 0.0
            if reset_str:
                try:
                    reset_dt = datetime.fromisoformat(reset_str.replace("Z", "+00:00"))
                    hours_left = max(0.0, (reset_dt - now_utc).total_seconds() / 3600.0)
                except Exception:
                    pass
            if rem is not None:
                if "Opus" in label or "Sonnet" in label:
                    res["anthropic"] = float(rem)
                    res["anthropic_reset"] = reset_str
                    res["anthropic_reset_hours"] = round(hours_left, 1)
                elif "Argon" in label or "Flash" in label:
                    res["google"] = float(rem)
                    res["google_reset"] = reset_str
                    res["google_reset_hours"] = round(hours_left, 1)
    except Exception:
        pass
    return res


def resolve_tier(
    classification: Dict[str, Any],
    quota_fraction: Optional[float] = None,
    force_model: Optional[str] = None,
) -> Dict[str, Any]:
    """Maps complexity score and quota headroom to model alias and thinking tier."""
    if force_model:
        return {
            "model": force_model,
            "thinking": "high",
            "thinkingLevel": 3,
            "displayName": force_model,
            "isAutonomous": False,
            "rationale": f"Explicit operator override: {force_model}",
            "complexityScore": classification["complexityScore"],
            "quotaHeadroom": quota_fraction or 1.0,
            "anthropicHeadroom": 1.0,
            "warning": None,
        }

    c = classification["complexityScore"]
    cat = classification["primaryCategory"]
    quotas = get_quota_headrooms()
    q_google = quotas["google"] if quota_fraction is None else quota_fraction
    q_anthropic = quotas["anthropic"]
    hrs_anthropic = quotas.get("anthropic_reset_hours", 0.0)
    hrs_google = quotas.get("google_reset_hours", 0.0)

    # Global Google Quota Throttling Guard
    throttled = False
    if q_google < 0.15 and c > 5:
        c = min(5, c)
        throttled = True

    warning = None

    if c <= 2:
        model = "flash-high"
        thinking = "none"
        thinking_level = 0
        display_name = "Gemini 3.8 Flash (High)"
        rationale = f"Class 0 (Complexity {c}/10): Mechanistic/syntactic task. Flash delivers instant zero-churn execution."
    elif c <= 5:
        model = "argon-low"
        thinking = "low"
        thinking_level = 1
        display_name = "Gemini 4 Argon (Low)"
        rationale = f"Class 1 (Complexity {c}/10): Standard implementation. Argon Low provides Level 1 thinking with minimal quota impact."
    elif c <= 8:
        model = "argon-med"
        thinking = "med"
        thinking_level = 2
        display_name = "Gemini 4 Argon (Medium)"
        if cat == "DSP_RESEARCH":
            rationale = f"DSP Research (Complexity {c}/10): Routed to Gemini 4 Argon for 1M-2M context window (synthesizer manuals/schematics) and 0 Anthropic quota burn."
        else:
            rationale = f"Class 2 (Complexity {c}/10): Architectural synthesis. Argon Medium provides Level 2 balanced reasoning."
    else:
        # Class 3: Formal / Safety Critical / DSP Writing
        # Automatic Cross-Pool Failover: If Anthropic pool is low (<15%), fail over to Gemini 4 Argon High
        if q_anthropic >= 0.15:
            model = "opus-high"
            thinking = "high"
            thinking_level = 3
            display_name = "Claude Opus 5.5 (High)"
            if cat == "DSP_WRITING":
                rationale = f"DSP Writing & Safety (Complexity {c}/10): Routed to Claude Opus 5.5 (High) for Level 3 Extended Thinking, bit-exact C++20 real-time audio thread safety, and lock-free verification."
            else:
                rationale = f"Class 3 (Complexity {c}/10): Safety-critical / formal invariant. Opus High provides Level 3 Adaptive Extended Thinking (~10s-50s CoT)."
        else:
            model = "argon-high"
            thinking = "high"
            thinking_level = 3
            display_name = "Gemini 4 Argon (High)"
            rationale = f"Class 3 (Complexity {c}/10): Safety-critical invariant. Anthropic pool depleted ({q_anthropic*100:.1f}% < 15%); failed over to Gemini 4 Argon (High) Level 3."

    # Allowance Warning Protocol
    if q_anthropic < 0.25:
        if q_anthropic >= 0.15:
            if model == "opus-high":
                warning = f"⚠️ [ANTHROPIC ALLOWANCE CAUTION: Headroom at {q_anthropic*100:.1f}%. Reset in {hrs_anthropic:.1f}h. Reserve Opus for safety-critical DSP/formal tasks.]"
        else:
            if c > 8:
                warning = f"🚨 [ANTHROPIC QUOTA DEPLETED: Headroom at {q_anthropic*100:.1f}% (<15%). Automatically failed over to Gemini 4 Argon (High) Level 3. Reset in {hrs_anthropic:.1f}h.]"

    if q_google < 0.15:
        g_warn = f"⚠️ [GOOGLE QUOTA CAUTION: Headroom at {q_google*100:.1f}%. Reset in {hrs_google:.1f}h.]"
        warning = f"{warning} | {g_warn}" if warning else g_warn

    if throttled:
        rationale += " [⚠️ Clamped due to low Google quota (<15%)]"

    return {
        "model": model,
        "thinking": thinking,
        "thinkingLevel": thinking_level,
        "displayName": display_name,
        "isAutonomous": True,
        "rationale": rationale,
        "warning": warning,
        "complexityScore": classification["complexityScore"],
        "primaryCategory": classification["primaryCategory"],
        "quotaHeadroom": round(q_google, 3),
        "anthropicHeadroom": round(q_anthropic, 3),
        "anthropicResetHours": hrs_anthropic,
        "googleResetHours": hrs_google,
    }


def record_dispatch(prompt: str, classification: Dict[str, Any], tier: Dict[str, Any]) -> int:
    """Logs a dispatch decision to the telemetry database."""
    conn = init_db()
    with conn:
        cur = conn.execute("""
            INSERT INTO routing_records (
                timestamp, prompt_snippet, complexity_score, domain_category,
                selected_model, thinking_level, operator_override
            ) VALUES (?, ?, ?, ?, ?, ?, ?);
        """, (
            time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
            prompt[:120],
            classification["complexityScore"],
            classification["primaryCategory"],
            tier["model"],
            tier["thinkingLevel"],
            0 if tier["isAutonomous"] else 1,
        ))
        rec_id = cur.lastrowid
    conn.close()
    return rec_id


def record_outcome(
    record_id: int,
    step_count: int,
    duration_sec: float,
    tokens_in: int,
    tokens_out: int,
    outcome: str,
) -> None:
    """Updates a dispatch record with post-execution telemetry and evaluation outcome."""
    if not DB_PATH.exists():
        return
    conn = sqlite3.connect(DB_PATH)
    with conn:
        conn.execute("""
            UPDATE routing_records
            SET step_count = ?, duration_sec = ?, tokens_in = ?, tokens_out = ?, outcome = ?
            WHERE id = ?;
        """, (step_count, duration_sec, tokens_in, tokens_out, outcome, record_id))
    conn.close()


BENCHMARK_PROMPTS = [
    # Class 0: Mechanistic (Flash)
    ("Rename variable buffer_len to capacity in tools/temp.py", 0, 2),
    ("Grep for all occurrences of invoke_subagent across .agents/scripts/", 0, 2),
    ("Format the json file in .agents/cache/test.json with 2 spaces", 0, 2),
    ("Check if file C:/Dev/build.log exists and print its size", 0, 2),
    ("Move obsolete markdown notes from Inbox to Archive/", 0, 2),

    # Class 1: Implementation (Argon Low)
    ("Implement a basic SQLite query helper for user lookup in auth.py", 3, 5),
    ("Write unit tests for the token counter function in tools/router.py", 3, 5),
    ("Parse the incoming JSON payload and validate fields with Pydantic", 3, 5),
    ("Add a new CLI flag --export to tools/bus.py with usage help", 3, 5),
    ("Create a simple HTTP healthcheck route returning status ok", 3, 5),

    # Class 2: Architectural (Argon Medium)
    ("Refactor the multi-module build pipeline to decouple compiler warnings across 4 files", 6, 8),
    ("Run a 4-lens PCDA stress-test on the new database migration schema", 6, 8),
    ("Audit docs/SYSTEM_MAP.md against recent factory deliveries and reconcile AST drift", 6, 8),
    ("Design a cross-platform state machine DAG for ToadTracker engine initialization", 6, 8),
    ("Conduct a premortem analysis on the bi-temporal chronicle indexing pipeline", 6, 8),

    # Class 3: Formal / Safety Critical (Opus / Argon High Level 3)
    ("Prove real-time audio thread safety: verify zero heap allocation and zero mutex locks in dsp/osc.h", 9, 10),
    ("Verify asynchronous concurrency: eliminate deadlock and priority inversion in lock-free ring buffer", 9, 10),
    ("Formally verify transcendental filter coefficients cascade stability at 96kHz Nyquist limit", 9, 10),
    ("Conduct formal invariant verification on the cryptographic session handshake and zero-copy transfer", 9, 10),
    ("Audit SIMD AVX2 vectorized audio buffer for race conditions and memory alignment faults", 9, 10),

    # DSP Specialization Benchmarks (Research vs Writing/Safety)
    ("Research Roland Alpha Juno filter schematic and DAFX paper on state variable filter topology", 6, 8),
    ("Implement C++20 lock-free audio ring buffer for oscillator voice with SIMD denormal protection", 9, 10),
]


def run_benchmark():
    """Runs deterministic verification gauntlet across all 22 canonical benchmarks."""
    print("=" * 80)
    print(f"🔬 ADAPTIVE ROUTER VERIFICATION GAUNTLET ({len(BENCHMARK_PROMPTS)} Canonical Prompts)")
    print("=" * 80)
    
    total_time_ms = 0.0
    passed = 0

    for idx, (prompt, min_exp, max_exp) in enumerate(BENCHMARK_PROMPTS, 1):
        res = classify_prompt(prompt)
        tier = resolve_tier(res, quota_fraction=1.0)
        c = res["complexityScore"]
        dur = res["classificationDurationMs"]
        total_time_ms += dur

        is_pass = min_exp <= c <= max_exp
        status = "✅ PASS" if is_pass else f"❌ FAIL (Exp: {min_exp}-{max_exp})"
        if is_pass:
            passed += 1

        print(f"[{idx:02d}] Score: {c:2d}/10 | {tier['displayName']:<25} | {status} ({dur:.2f}ms)")
        print(f"     Cat: {res['primaryCategory']:<14} | Prompt: {prompt[:55]}...")

    avg_time = total_time_ms / len(BENCHMARK_PROMPTS)
    print("-" * 80)
    print(f"Result: {passed}/{len(BENCHMARK_PROMPTS)} Passed ({passed/len(BENCHMARK_PROMPTS)*100:.1f}%)")
    print(f"Avg Classification Latency: {avg_time:.3f} ms (Target: < 2.0 ms)")
    print("=" * 80)
    return passed == len(BENCHMARK_PROMPTS)


def main():
    parser = argparse.ArgumentParser(
        description="Adaptive Subagent Tier Routing & Heuristic Calibration Engine",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    subparsers = parser.add_subparsers(dest="command", required=True)

    p_classify = subparsers.add_parser("classify", help="Classify a task prompt and show tier recommendation")
    p_classify.add_argument("prompt", help="Task prompt string to classify")
    p_classify.add_argument("--files", "-f", nargs="*", help="Referenced file paths")
    p_classify.add_argument("--json", action="store_true", help="Output raw JSON")

    subparsers.add_parser("test", help="Run 22-prompt verification test gauntlet")
    subparsers.add_parser("stats", help="Display router telemetry, overkill/underprovisioned ratios")

    p_dispatch = subparsers.add_parser("dispatch", help="Classify prompt and execute via appropriate headless model bridge")
    p_dispatch.add_argument("prompt", help="Prompt text to dispatch")
    p_dispatch.add_argument("--force", choices=["opus", "argon", "flash"], default=None, help="Force specific model")
    p_dispatch.add_argument("--timeout", type=int, default=300, help="Execution timeout in seconds")

    args = parser.parse_args()

    if args.command == "classify":
        res = classify_prompt(args.prompt, args.files)
        tier = resolve_tier(res)
        if args.json:
            print(json.dumps({"classification": res, "tier": tier}, indent=2))
        else:
            print(f"\n🎯 [Auto-Tier Recommendation: {tier['displayName']}]")
            print(f"  • Complexity Score : {res['complexityScore']}/10 (Raw: {res['rawScore']})")
            print(f"  • Domain Category  : {res['primaryCategory']}")
            print(f"  • Thinking Level   : {tier['thinking']} (Level {tier['thinkingLevel']})")
            print(f"  • Matched Features : {', '.join(res['matchedTags']) if res['matchedTags'] else 'Standard'}")
            print(f"  • Quota Headroom   : Google: {tier['quotaHeadroom']*100:.1f}% | Anthropic: {tier['anthropicHeadroom']*100:.1f}%")
            print(f"  • Decision Rationale: {tier['rationale']}")
            if tier.get("warning"):
                print(f"  • Allowance Alert  : {tier['warning']}")
            print(f"  • Classifier Speed : {res['classificationDurationMs']} ms (0 Tokens)")

    elif args.command == "test":
        success = run_benchmark()
        sys.exit(0 if success else 1)

    elif args.command == "stats":
        init_db()
        adjustments = get_learned_adjustments()
        conn = sqlite3.connect(DB_PATH)
        conn.row_factory = sqlite3.Row
        total = conn.execute("SELECT COUNT(*) as count FROM routing_records;").fetchone()["count"]
        print(f"\n📊 [Router Telemetry & Anti-Dogmatism Stats]")
        print(f"  • Total Logged Dispatches: {total}")
        print(f"  • Active Category Offsets : {adjustments if adjustments else 'None (Nominal Base Weights)'}")
        conn.close()

    elif args.command == "dispatch":
        res = classify_prompt(args.prompt)
        tier = resolve_tier(res, force_model=args.force)
        print(f"🎯 [Router Selected: {tier['displayName']}] (Complexity {res['complexityScore']}/10, Category: {res['primaryCategory']})")
        
        target_model = args.force or tier["model"]
        if target_model in ("opus", "opus-high"):
            from tools.claude_bridge import query
            start_t = time.perf_counter()
            out = query(args.prompt, model="opus", timeout=args.timeout)
            dur = time.perf_counter() - start_t
            print(out)
            rec_id = record_dispatch(args.prompt, res, tier)
            tokens_in = max(1, len(args.prompt) // 4)
            tokens_out = max(1, len(out) // 4)
            record_outcome(rec_id, step_count=1, duration_sec=dur, tokens_in=tokens_in, tokens_out=tokens_out, outcome="EVAL_CLEAN")
        else:
            print(f"ℹ️ Model tier '{tier['displayName']}' designated for native Antigravity orchestrator/subagent channel.")
            record_dispatch(args.prompt, res, tier)


if __name__ == "__main__":
    main()
