# Implementation Plan: Flight 8/8 — Cumulative Allowance & Token Impact Ledger
> **TARGET:** TOOLING_HARNESS (New Klang City) 🚀  
> **Flight Target:** Flight 8/8 (Topics 6.3 & 6.19: Monolithic Deep-Linked Board & Cumulative Cost Meter)  
> **Status:** COMPLETED ✅  
> **Target Branch:** `0.4.0-dev`  
> **Recommended Model:** Gemini 3.8 Flash (Thinking: High) — 🟢 SUSTAINABLE  
> **Execution Strategy:** Delegated to Tier 2 Flash Builder Subagent via `subagent_dispatch.md`  

---

## 🎯 Architectural Intent & Scope

Flight 8/8 is the capstone runway flight establishing token economics transparency, live allowance budgeting, and deep-linked topic organization across N'kai:
1. **Topic 6.3 (Approved Option B — Deep-Linked Monolithic Board Architecture):**
   - Solidifies the single, highly structured living research canvas with hash-anchor deep linking (`#t1`, `#t2`, `#topic-header`).
   - Ensures hash routing preserves viewport context across reloads and tab jumps with smooth focus highlights (`.anchor-highlight`).
2. **Topic 6.19 (Approved Option C — Live Cumulative Cost Meter & Budget Ledger):**
   - Implements the live **Cumulative Allowance & Quota Impact Ledger** inside N'kai:
     - Header/Autopsy metric bar showing estimated cumulative token burn across all approved topics:
       - **Tier 1 (Pro High)**: ~40k tokens/task (heavy math, complex architectural synthesis).
       - **Tier 2 (Flash High)**: ~10k tokens/task (balanced UI, TS/HTML, standard phase execution).
       - **Tier 3 (Flash Low/Medium)**: ~2k tokens/task (mechanical chores, JSON schemas, documentation sweeps).
     - Proactive quota warning thresholds (e.g. green $\le 100\text{k}$, amber $101\text{k}$–$200\text{k}$, red $>200\text{k}$) alerting planners before dispatching massive multi-task marathons.
     - Interactive breakdown drawer/tooltip itemizing token estimates per approved card.
3. **Formal Specification & Registrations**:
   - Authors `docs/specs/nkai_allowance_ledger_and_token_governance.md`.
   - Registers in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.

---

## 📋 Phase-by-Phase Execution Plan

### Phase 1/5: Specification & Catalog Registration
- [x] Author `docs/specs/nkai_allowance_ledger_and_token_governance.md`:
  - Document the Deep-Linked Monolithic Board URL routing contracts and hash change listeners.
  - Document the Model Tier token estimation heuristics (Tier 1 vs. Tier 2 vs. Tier 3 burn rates).
  - Document the Cumulative Allowance Ledger data model, threshold color bands, and warning triggers.
- [x] Register spec in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.

### Phase 2/5: Core N'kai Allowance & Meter Components (`sidecar.css` & `sidecar.js`)
- [x] In `c:\Dev\nkai\core\sidecar.css`:
  - Style `.allowance-meter`, `.meter-bar-track`, `.meter-bar-fill`, and tier segment indicators.
  - Style semantic threshold badges (`.meter-safe` green, `.meter-warning` amber, `.meter-critical` rose).
  - Style `.allowance-breakdown` table and popover tooltip.
  - Style `:target` and `.anchor-highlight` animations for smooth-scrolled hash links.
- [x] In `c:\Dev\nkai\core\sidecar.js`:
  - Implement `updateAllowanceLedger()` to compute cumulative estimated tokens from approved cards dynamically.
  - Implement hash-change routing and smooth scrolling with auto-activating matching tabs.

### Phase 3/5: Zero-Dependency Allowance Estimator Tool (`tools/nkai_ledger.py`)
- [x] Author `c:\Dev\nkai\tools\nkai_ledger.py` (and mirror to `c:\Dev\TheKlangSuite\tools\nkai_ledger.py`):
  - Standard Python library only (`os`, `sys`, `pathlib`, `argparse`, `re`, `json`).
  - Supports `--audit <board.html>`: parses HTML board, tallies approved cards by model tier, and outputs estimated token burn.
  - Supports `--chat`: prints a concise, terminal-friendly allowance scorecard.
  - Windows UTF-8 stdout hardening.

### Phase 4/5: Canonical Template Synchronization
- [x] Update `c:\Dev\nkai\dist\standalone-template.html` with allowance meter in header/scoreboard and hash-anchor deep links.
- [x] Update `c:\Dev\nkai\bundle\templates\template.html` with identical canonical markup.
- [x] Update `c:\Dev\TheKlangSuite\.agents\sidecar\template.html`.

### Phase 5/5: Verification Gauntlet & Commits
- [x] Run `python tools/audit_sidecar.py` across updated templates (100% tag balance, zero syntax errors).
- [x] Test `python tools/nkai_ledger.py --help` and run an audit dry-run.
- [x] Run `python tools/verify_context_clues.py` in `c:\Dev\TheKlangSuite`.
- [x] Commit changes cleanly across `TheKlangSuite` and `nkai` with conventional commits: `feat(ledger): implement Flight 8 cumulative allowance ledger and token governance`.

---

## 🛡️ Verification & Acceptance Criteria
1. `docs/specs/nkai_allowance_ledger_and_token_governance.md` is valid and indexed in catalog/system map.
2. N'kai templates feature an interactive allowance meter dynamically updating with approved cards.
3. `tools/nkai_ledger.py` runs with zero dependencies, accurately calculating token footprint and quota impact.
4. Deep linking via URL hashes navigates smoothly to target cards across tabs with focus pulse.
5. `audit_sidecar.py` and `verify_context_clues.py` pass 100% with zero errors.
