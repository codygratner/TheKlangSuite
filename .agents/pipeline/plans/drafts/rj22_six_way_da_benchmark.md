# rj-22 Six-Way Devil's Advocate Benchmark (rev 4: allowance-aware split, hold until Sunday)

> **TARGET:** TOOLING_HARNESS (research and evaluation only) · Mailbag card `rj-22`
> **Durable data home:** `C:\Dev\Research\agentic\rj22_da_benchmark\`
> **Active Model Setting:** Gemini 3.8 Flash (Thinking: High) — 🟢 SUSTAINABLE
> **Allowance at planning time:** 51% of the 5-hour window, 25% of the week (Opus-capable plan). Weekly reset: **Sunday 2026-10-18**.

## Goal

Benchmark our three DA engines as **writers** (two prompt methods each) and as **judges**, under real-workflow conditions. This week, finish
everything that doesn't need further Opus judging or analysis, deliver **two provisional meta-analyses** written by Flash and Pro, then **hold**
until the weekly reset for the Opus judge, the final Opus meta-analysis, and the real-world impact report.

### Engines (real-workflow definitions)
| Cell | Produced by | Dispatching main chat (verified) |
| :--- | :--- | :--- |
| **Flash** | `research` subagent, `Model="flash"` (whatever version the platform supplies) | `Gemini 3.8 Flash (High)` |
| **Pro** | `research` subagent, `Model="pro"` (whatever version the platform supplies) | `Gemini 3.8 Flash (High)` |
| **Opus** | `research` subagent, `Model="inherit"` | `Claude Opus 5.5 (High)` |

### Methods
- **Method A, "Original":** the original Pro prompt, byte-for-byte.
- **Method B, "Hardened":** the `opus-B` prompt, byte-for-byte: Method A plus the blindness rule, fact-check web search, and the "Where I Disagree With The Prompt" section.

### The six cells
| | Method A | Method B |
| :--- | :--- | :--- |
| **Flash** | `flash-A` ✅ existing | **`flash-B`**: W3 |
| **Pro** | `pro-A` ✅ existing | **`pro-B`**: W3 |
| **Opus** | **`opus-A`**: W1 ✅ completed | `opus-B` ✅ existing |

---

## 📋 Phase Breakdown

### Stage W1: Prompt freeze + `opus-A` (Opus)
- [x] Freeze prompts `method_A.txt` and `method_B.txt` with SHA-256 hashes
- [x] Run `opus-A` subagent with Method A instructions (`b2b1b96b-a69d-464d-9282-f9af6f95c612`)

### Stage W2: Prep & Tooling Harness (Flash Builder Subagent)
- [ ] Extract `opus-A.md` raw report from subagent transcript to `C:\Dev\Research\agentic\rj22_da_benchmark\reports\opus-A.md`
- [ ] Setup `C:\Dev\Research\agentic\rj22_da_benchmark\` directories: `prompts/`, `reports/`, `anonymized/`, `reviews/`, `meta/`, `sealed/`
- [ ] Copy existing reports (`flash-A.md`, `pro-A.md`, `opus-B.md`) and `ground_truth.md`
- [ ] Copy frozen prompts and write `prompts.sha256`
- [ ] Author `tools/da_bench.py` (CLI with `extract`, `sanitize`, `audit-blindness`, `aggregate`, `manifest`)
- [ ] Author `tools/tests/test_da_bench.py` and verify all tests pass
- [ ] Initialize `manifest.json` with Stage W2 complete

### Stage W3: Runs, Blind Judges & Provisional Metas (Flash)
- [ ] Launch `flash-B` (`Model="flash"`) and `pro-B` (`Model="pro"`) with Method B prompt
- [ ] Save outputs to `reports/flash-B.md` and `reports/pro-B.md`
- [ ] Run `python tools/da_bench.py sanitize` -> generate `R1.md` through `R6.md` and `sealed/key.json`
- [ ] Launch Blind Flash Judge (`Model="flash"`) on R1..R6 -> `reviews/judge-flash.md`
- [ ] Launch Blind Pro Judge (`Model="pro"`) on R1..R6 -> `reviews/judge-pro.md`
- [ ] Run `python tools/da_bench.py audit-blindness` on judge transcripts
- [ ] Author Provisional Flash Meta-Analysis -> `meta/provisional_flash.md`
- [ ] Author Provisional Pro Meta-Analysis -> `meta/provisional_pro.md`
- [ ] Transition manifest to `HOLD` state
- [ ] Present summary scorecard to user

### Stage ⏸️ HOLD (Until Sunday 2026-10-18 weekly reset)
- [ ] Await weekly allowance reset

### Stage W4: Opus Judge, Final Meta & Real-World Impact (Opus, Sunday+)
- [ ] Run Blind Opus Judge (`Model="inherit"` on Claude Opus 5.5 High)
- [ ] Run `da_bench.py audit-blindness`
- [ ] Run `da_bench.py aggregate`
- [ ] Author Final Meta-Analysis
- [ ] Author Real-World Impact Report
- [ ] Emit Downgrade Chime
