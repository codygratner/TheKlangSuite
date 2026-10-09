# Implementation Plan: Flight 1 — Canonical State & Factory Handoff
> **Flight Target:** Topics 6.16 & 6.7 (Phase 1: Foundations)  
> **Status:** COMPLETED ✅  
> **Target Branch:** `0.4.0-dev`  
> **Recommended Model:** Gemini 3.8 Flash (Thinking: High) — 🟢 SUSTAINABLE  

---

## 🎯 Architectural Intent & Scope

Flight 1 establishes the rock-solid canonical state and context handoff foundation across all Five Pillars (`TheKlangSuite`, `ToadTracker`, `nkai`, `TheKlangResearch`, `TheKlangVault`):
1. **Topic 6.16 (Approved Option C):** Formalizes the **Hybrid Markdown Frontmatter + Client-Side LocalStorage + JSON Catalog** state contract in `docs/specs/nkai_canonical_state_architecture.md`. Guarantees zero SQLite daemon overhead, zero binary corruption risk, instant Git diffs, and offline `file:///` compliance.
2. **Topic 6.7 (Approved Option B):** Standardizes the **Structured Context Clues Architecture** by authoring a universal `docs/templates/context_clues_template.md`, harmonizing `context_clues_plan.md`, `context_clues_build.md`, and `context_clues_subagent.md`, and building `tools/verify_context_clues.py` to programmatically assert that 100% of context clues files have valid sections and zero broken links.

---

## 📋 Phase-by-Phase Execution Plan

### Phase 1: Canonical State Architecture Contract (Topic 6.16)
- [x] Author `docs/specs/nkai_canonical_state_architecture.md`:
  - Define Tier 1 (Frontmatter Markdown in Research/Autopsies).
  - Define Tier 2 (Session Packages in `.agents/sidecar/packages/<package_id>.json`).
  - Define Tier 3 (Client-side ephemeral UI state in browser `localStorage`).
  - Document the zero-binary database invariant and Git-diff preservation standard.
- [x] Register the new spec in `docs/DOCS_CATALOG.json` under `specifications`.
- [x] Add direct link in `docs/SYSTEM_MAP.md` under Section 7 (Design Specs).

### Phase 2: Context Clues Universal Template & File Harmonization (Topic 6.7)
- [x] Author canonical template: `docs/templates/context_clues_template.md`:
  - Section 1: Active SemVer, Branch State & Roadmap Sidecar Deep Links.
  - Section 2: Immediate Return Agenda & Status.
  - Section 3: Architectural Memory & "Unforgettables".
  - Section 4: Canonical Taxonomy, Product Lineup & Essential Spec Links.
  - Section 5: Audio Thread / Workflow Invariants & Handshake Gates.
- [x] Harmonize `context_clues_plan.md` to conform to the canonical template, updating the Return Agenda to Flight 1 / Flight 9 execution.
- [x] Harmonize `context_clues_build.md` with active directive references and subagent delegation protocols.
- [x] Harmonize `context_clues_subagent.md` ensuring strict parity across subagent roles (`research`, `explore`, `contrarian`).

### Phase 3: Automated Verification Tooling (`tools/verify_context_clues.py`)
- [x] Author `tools/verify_context_clues.py`:
  - Scans workspace root for all `context_clues_*.md` files.
  - Asserts existence of mandatory section headings (`Active SemVer`, `Return Agenda`, `Architectural Memory`, `Taxonomy`, `Invariants`).
  - Extracts all Markdown hyperlinks (`[label](path)`) and asserts that 100% of local file targets exist on disk.
  - Reports clean exit code 0 on perfect pass, non-zero with error lines on failure.
- [x] Execute `python tools/verify_context_clues.py` to validate clean pass.

### Phase 4: Field Guide Documentation & Ecosystem Gauntlet
- [x] Document the Factory Handshake Gate in `docs/history/vibe_coding_field_guide.md`.
- [x] Run `powershell -ExecutionPolicy Bypass -File .\tools\sync_obsidian_vault.ps1` to compile `TheKlangVault/Research/Master_Research_Index.md`.
- [x] Verify `git status` and commit changes using conventional commits: `feat(state): implement Flight 1 canonical state and context clues architecture`.

---

## 🛡️ Verification & Acceptance Criteria
1. `python tools/verify_context_clues.py` exits with code 0 (100% link resolution across all context clues files).
2. `docs/specs/nkai_canonical_state_architecture.md` is valid markdown, registered in `DOCS_CATALOG.json` and `SYSTEM_MAP.md`.
3. `tools/sync_obsidian_vault.ps1` runs with 0 errors.
4. Zero audio-thread impact (strictly architectural and documentation infrastructure).
