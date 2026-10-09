# Implementation Plan: Flight 7/8 — PCDA Dialectic Governance & SOTA Benchmarks
> **TARGET:** TOOLING_HARNESS (New Klang City) 🚀  
> **Flight Target:** Flight 7/8 (Topics 6.1, 6.2 & 6.15: Continuous Triage, On-Demand PCDA, SOTA Benchmarks)  
> **Status:** COMPLETED ✅  
> **Target Branch:** `0.4.0-dev`  
> **Recommended Model:** Gemini 3.8 Flash (Thinking: High) — 🟢 SUSTAINABLE  
> **Execution Strategy:** Delegated to Tier 2 Flash Builder Subagent via `subagent_dispatch.md`  

---

## 🎯 Architectural Intent & Scope

Flight 7/8 codifies and standardizes the PCDA Dialectic Review Framework, On-Demand SOTA Progressive Disclosure, and Competitive Industry Benchmarks across N'kai:
1. **Topic 6.1 (Approved Option B — Continuous Live Triage Architecture):**
   - Establishes the real-time verdict streaming contract: decisions are committed continuously throughout a session rather than batched at the very end.
   - Integrates with the Autopsy Scoreboard and live verdict state machines.
2. **Topic 6.2 (Approved Option B — On-Demand SOTA Progressive Disclosure & PCDA Triad):**
   - Standardizes the 4-part Hegelian Dialectic Triad across all tooling and sidecars:
     - **Pros [Thesis]**: Affirmative benefits, developer velocity, immediate value.
     - **Cons [Thesis Friction]**: Direct implementation cost, surface friction, known tradeoffs.
     - **Devil's Advocate [Antithesis]**: Structural failure modes, worst-case maintenance debt, token bloat traps.
     - **The 80/20 Resolution [Synthesis]**: The pragmatic architecture capturing 80% of value with 20% complexity.
   - Discloses tradeoffs strictly on-demand (via `[ ⚖️ Request Pros & Cons ]` or `[ ⚖️ PCDA ]`) while keeping chat responses ultra-terse (1–3 sentences).
3. **Topic 6.15 (Approved Option B — SOTA Precedents & Competitive Teardowns):**
   - Documents and integrates competitive benchmarks against Anthropic Claude Artifacts, Playwright UI, Vitest UI, Cursor Composer, and WebAssembly sidecars.
   - Introduces `.badge-sota` indicators and standard 4-Lens tradeoff grids (Ergonomics, Audio Industry, SOTA, Best Practices).
4. **Formal Specification & Registrations**:
   - Authors `docs/specs/nkai_pcda_governance_and_sota_benchmarks.md`.
   - Registers in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.

---

## 📋 Phase-by-Phase Execution Plan

### Phase 1/5: Specification & Catalog Registration
- [x] Author `docs/specs/nkai_pcda_governance_and_sota_benchmarks.md`:
  - Document the Continuous Live Triage lifecycle and session persistence contracts.
  - Document the 4-part PCDA dialectic standard (Thesis, Friction, Antithesis, Synthesis) and acronym disambiguation (PCDA vs PCA).
  - Document the On-Demand SOTA progressive disclosure protocol.
  - Document the SOTA precedent teardown matrix (Claude Artifacts, Playwright/Vitest UI, Cursor Composer).
- [x] Register spec in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.

### Phase 2/5: Core N'kai PCDA & Benchmark Components (`sidecar.css` & `sidecar.js`)
- [x] In `c:\Dev\nkai\core\sidecar.css`:
  - Implement `.pcda-container`, `.pcda-block`, and semantic dialectic cards:
    - `.pcda-pro` (emerald left-border and subtle tinted background).
    - `.pcda-con` (amber left-border and subtle tinted background).
    - `.pcda-advocate` (rose left-border, bold devil's advocate styling).
    - `.pcda-synthesis` (purple/indigo gradient border, high-contrast resolution badge).
  - Implement `.badge-sota` and `.lens-grid` (responsive 4-lens tradeoff matrix).
- [x] In `c:\Dev\nkai\core\sidecar.js`:
  - Standardize `togglePcdaBlock(blockId)` accordion controller with haptic tick triggers.
  - Add helper `copyPcdaMarkdown(topicId)` to extract clean GitHub-flavored PCDA Markdown to clipboard.

### Phase 3/5: Zero-Dependency PCDA Formatter & Validator Tool (`tools/pcda_format.py`)
- [x] Author `c:\Dev\nkai\tools\pcda_format.py` (and mirror to `c:\Dev\TheKlangSuite\tools\pcda_format.py`):
  - Standard Python library only (`os`, `sys`, `pathlib`, `argparse`, `re`, `json`).
  - Supports `--validate <file.md>`: verifies all 4 PCDA components exist in topic sections.
  - Supports `--to-html <file.md>`: converts Markdown PCDA sections into standalone, drop-in N'kai HTML snippets.
  - Supports `--chat <file.md>`: formats PCDA sections into terse, chat-friendly summaries.

### Phase 4/5: Canonical Template Synchronization
- [x] Update `c:\Dev\nkai\dist\standalone-template.html` with standardized PCDA structure and CSS classes.
- [x] Update `c:\Dev\nkai\bundle\templates\template.html` with identical markup.
- [x] Update `c:\Dev\TheKlangSuite\.agents\sidecar\template.html`.

### Phase 5/5: Verification Gauntlet & Commits
- [x] Run `python tools/audit_sidecar.py` across updated templates (100% tag balance, zero syntax errors).
- [x] Test `python tools/pcda_format.py --help` and execute a validation test run.
- [x] Run `python tools/verify_context_clues.py` in `c:\Dev\TheKlangSuite`.
- [x] Commit changes cleanly across `TheKlangSuite` and `nkai` with conventional commits: `feat(pcda): implement Flight 7 PCDA dialectic governance and SOTA benchmarks`.

---

## 🛡️ Verification & Acceptance Criteria
1. `docs/specs/nkai_pcda_governance_and_sota_benchmarks.md` is valid and indexed in catalog/system map.
2. N'kai templates feature clean, standardized PCDA accordion components with semantic color coding.
3. `tools/pcda_format.py` runs with zero dependencies, validating and converting PCDA blocks accurately.
4. `audit_sidecar.py` and `verify_context_clues.py` pass 100% with zero errors.
5. All commits follow conventional commit standards.
