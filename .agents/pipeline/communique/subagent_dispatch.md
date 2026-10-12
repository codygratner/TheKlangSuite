STATUS: COMPLETED

# Subagent Dispatch Charter: Tooling Implementation
> *Target Pipeline:* Dual-Track Communiqué — Tooling Pipeline (`TARGET: TOOLING_HARNESS`)
> *Active Branch:* `0.4.0-dev` (TKS)
> *Orchestrator:* Antigravity Main Chat (New Klang City)
> *Executor:* Flash Builder Subagent

---

## 1. Assignment Header
- **STATUS**: `COMPLETED`
- **ROLE**: `Flash Tooling Builder`
- **MODEL TIER**: `Tier 2 Flash (High)`
- **TASK TYPE**: `TOOLING_IMPLEMENTATION`
- **TASK SLUG**: `rj22-da-bench-tooling`
- **TARGET REPOSITORIES**: `TheKlangSuite`, `TheKlangResearch` (`C:\Dev\Research`)
- **TICKET ID**: `rj-22` (Stage W2: Prep & Tooling Harness)

## 2. Scope & Instructions

Deliver Stage W2 per `c:\Dev\TheKlangSuite\PLAN.md`:

### Phase 1: Corpus Directories & Report Ingestion
1. Ensure the following directories exist under `C:\Dev\Research\agentic\rj22_da_benchmark\`:
   - `prompts/`
   - `reports/`
   - `anonymized/`
   - `reviews/`
   - `meta/`
   - `sealed/`
2. Extract `reports/opus-A.md`:
   - Read subagent transcript `C:\Users\codyg\.gemini\antigravity\brain\b2b1b96b-a69d-464d-9282-f9af6f95c612\.system_generated\logs\transcript_full.jsonl` (or fallback `transcript.jsonl`).
   - Extract the two `send_message` tool call payloads (Msg 0: Part 1/2, Msg 1: Part 2/2).
   - Concatenate them verbatim and write to `C:\Dev\Research\agentic\rj22_da_benchmark\reports\opus-A.md`.
3. Ingest existing reports:
   - Copy `c:\Dev\TheKlangSuite\.agents\pipeline\staging\souls\rj22_benchmark\flash_report.md` to `C:\Dev\Research\agentic\rj22_da_benchmark\reports\flash-A.md`.
   - Copy `c:\Dev\TheKlangSuite\.agents\pipeline\staging\souls\rj22_benchmark\pro_report.md` to `C:\Dev\Research\agentic\rj22_da_benchmark\reports\pro-A.md`.
   - Copy `c:\Dev\TheKlangSuite\.agents\pipeline\staging\souls\rj22_benchmark\opus_report.md` to `C:\Dev\Research\agentic\rj22_da_benchmark\reports\opus-B.md`.
   - Copy `c:\Dev\TheKlangSuite\.agents\pipeline\staging\souls\2026-10-11_rj22_ground_truth.md` to `C:\Dev\Research\agentic\rj22_da_benchmark\ground_truth.md`.
4. Freeze prompts:
   - Copy `C:\Users\codyg\.gemini\antigravity\brain\9241988e-1227-466e-a0a5-446d61a69d77\scratch\rj22_prompts\pro_A.txt` to `C:\Dev\Research\agentic\rj22_da_benchmark\prompts\method_A.txt`.
   - Copy `C:\Users\codyg\.gemini\antigravity\brain\9241988e-1227-466e-a0a5-446d61a69d77\scratch\rj22_prompts\opus_B.txt` to `C:\Dev\Research\agentic\rj22_da_benchmark\prompts\method_B.txt`.
   - Compute SHA-256 hashes of both files and write to `C:\Dev\Research\agentic\rj22_da_benchmark\prompts\prompts.sha256`.

### Phase 2: Author `tools/da_bench.py`
Zero external dependencies (Python standard library only). Universal UTF-8 compliance.
Subcommands:
1. `extract`: Helper to assert reports exist.
2. `sanitize`:
   - Inputs: 6 reports (`flash-A.md`, `pro-A.md`, `opus-A.md`, `flash-B.md`, `pro-B.md`, `opus-B.md`).
   - If not all 6 are present, support `--allow-partial` or sanitize whichever are present, but for production R1..R6 requires all 6.
   - Strip part markers: `(PART 1/2)`, `(PART 2/2)`, etc.
   - Strip blindness preamble (e.g. `BLINDNESS RULE: ...`).
   - Model name redaction:
     - Regex replace `Opus`, `Claude`, `Anthropic`, `Gemini`, `Flash`, `Pro` with `[MODEL]`.
     - CRITICAL EXCEPTION: Do NOT redact `Pro` if it is part of `Pro Tools`, `ProTools`, `Apple ProRes`, etc.
     - Replace `(Benchmark: \w+ Model)` with `(Benchmark: [MODEL] Model)`.
   - Deterministic / seeded shuffle:
     - Record mapping from `R1..R6` to cell names in `C:\Dev\Research\agentic\rj22_da_benchmark\sealed\key.json`.
     - Write `C:\Dev\Research\agentic\rj22_da_benchmark\anonymized\R1.md` through `R6.md`.
3. `audit-blindness`:
   - Takes `--transcript <path_to_jsonl>` or `--transcript-dir <dir>`.
   - Audits tool calls: asserts no file access or search occurred outside `anonymized/` or allowed web searches.
   - Returns exit code 0 if 100% blind, exit code 1 with report of violations if breached.
4. `aggregate`:
   - Parses judge review files in `reviews/` looking for the JSON score block:
     `{"scores": {"R1": {"accuracy": n, ...}, ...}, "ranking": ["R?", ...]}`
   - Computes:
     - Per-cell means across judges.
     - Method B minus Method A effect delta per model.
     - Kendall rank correlation (tau-b) between judges.
     - Self-preference bias indicator (did Flash/Pro/Opus rank their own model higher?).
     - Report length and token count statistics.
   - Outputs Markdown table and optional `--json` summary.
5. `manifest`:
   - `manifest show`: Prints human-readable status of the benchmark.
   - `manifest check [--through W2|W3|W4]`: Asserts all required artifacts and gate evidences exist and checksums match.
   - `manifest set <key> <val>`: Updates manifest state (e.g. setting state to `HOLD`).

### Phase 3: Author `tools/tests/test_da_bench.py`
Unit tests covering:
- Redaction logic: assert "Pro Tools" is preserved while "Pro Model", "Claude Opus", "Gemini Flash" are redacted to "[MODEL]".
- Sanitization: part headers and blindness preambles stripped.
- Key sealing and shuffling: roundtrip test ensuring R1..R6 can be de-anonymized via `key.json`.
- Kendall tau calculation: test concordance algorithm with known synthetic rankings.
- Audit blindness: test against clean mock transcript and tainted mock transcript (forbidden file read).
- Manifest check: test validation logic and `HOLD` state support.
- Prompt hash integrity test.

### Phase 4: Initialize `manifest.json` & Run Verification Gauntlet
- Initialize `C:\Dev\Research\agentic\rj22_da_benchmark\manifest.json`.
- Run `python -m unittest tools/tests/test_da_bench.py` (assert 100% PASS).
- Run `python tools/verify_utf8.py --strict`.
- Check off Stage W2 tasks in `c:\Dev\TheKlangSuite\PLAN.md`.
- Append receipt to `c:\Dev\TheKlangSuite\.agents\pipeline\communique\subagent_receipt.md` and set dispatch status to COMPLETED.
