# 📥 Flight 7: Mailbag Intake & Triage v2: Overflow, Quick Notes, Attention Tags & Session Routing (`rj-07` & `rj-17`)

> **ID:** plan-012  
> **TARGET:** TOOLING_HARNESS (New Klang City → Flash Builder Subagent) 🚀  
> **Topic Target:** Mailbag Card `rj-07` & `rj-17` / BACKLOG Section 13 (Flight 7: Streamlined Mailbag Ingestion)  
> **Status:** COMPLETED ✅  
> **Priority:** P1  
> **Target Branch:** `0.4.0-dev` (TheKlangSuite) / `main` (nkai)  
> **Recommended Setting:** Gemini 3.8 Flash (Thinking: High) — 🟢 SUSTAINABLE  
> **Prerequisites:** Flight 18 (UTF-8) ✅, Flight 16+17 (Sandbox Board) ✅, Flight 15 (Master Template) ✅, Flight 14 (Multi-Board Hub) ✅  
> **Dependents:** Flight 19 (Per-Card Response Drawer) ⏳  
> **Supersedes:** BACKLOG Flight 7  

---

## 🎯 Architectural Intent & Core Capabilities

Ideas get lost on the way IN (throwaway lines inside long rambles, empty quick-notes) and stall on the way OUT (cards lack quick triage routing). Adding an item at a specific position previously required manual edits, and cards lacked high-visibility callouts when a note was bloated, incomplete, or urgent. This flight merges Intake v2 (`rj-07`) and Triage v2 (`rj-17`) into a cohesive, high-velocity mailbag pipeline:

1. **Ramble Overflow Queue**: When a ramble decomposes into $> 3$ ideas, the top 3 become cards on the active board; remaining ideas move to `.agents/pipeline/mailbag/overflow.json`, rendering a `⤵ N in overflow` chip on the parent card so long rambles never flood the triage feed.
2. **Filename-Only Quick Notes**: Empty/whitespace notes use the filename as title with `[quick-note]` tag and `(quick note — title only)` preview, allowing frictionless mobile capture.
3. **`Inbox/Tabled/` Subdirectory**: Agent-authored notes move to a dedicated folder and get an explicit `🗂 Tabled by agent` chip.
4. **Attention Tags**: Notes containing `!IMPORTANT`, `!BLOATED`, `!INCOMPLETE`, or `!SPIKE` render high-visibility semantic chips.
5. **One-Click Session Routing Launchers**: Direct action buttons on cards (`[ 🔬 Grill-Lab ]`, `[ 🧠 Deep Research ]`, `[ 🔨 Draft Plan ]`) to launch focused agent workflows with zero copy-paste friction.
6. **`/mail-add` CLI with Position Splice**: Insert new cards at an exact rank (e.g. `--after rj-05`), automatically renumbering subsequent cards contiguously.

---

## 📋 Phase Breakdown

### Phase 1: Ramble Overflow Queue (`tools/ramble_ingest.py`)
- [x] In `tools/models.py`, add `split_group: str = ""` to `RambleCard` and `MailItem`.
- [x] Update `tools/ramble_ingest.py`:
  - When decomposing a note with $> 3$ ideas, promote the top 3 ideas to the active queue.
  - Write remaining ideas to `.agents/pipeline/mailbag/overflow.json` (as `List[RambleCard]`).
  - Store shared original note path in `source_note` for each split card.
- [x] In `master_template.html` & `board_renderer.py`:
  - Render `⤵ N in overflow` chip on parent card.
  - Render an "Overflow" pill in the status filter rail if overflow items exist.
- [x] Mirror models and tools across `TheKlangSuite` and `nkai`.

### Phase 2: Filename-Only Quick Notes (`tools/check_mail.py`)
- [x] Update `tools/check_mail.py`:
  - If a note body is empty or whitespace (after frontmatter strip), treat as quick-note.
  - Set title = filename stem, add tag `quick-note`, and preview = `(quick note — title only)`.
- [x] Add quick-note mock card to `assets/fixtures/sandbox_mail_queue.json`.
- [x] Verify clean rendering on sandbox board.

### Phase 3: `Inbox/Tabled/` for Agent-Authored Notes
- [x] Create `C:\Dev\TheKlangVault\Inbox\Tabled\` directory (and support fallback `docs/inbox/tabled/`).
- [x] In `tools/check_mail.py`, scan `Inbox/Tabled/` alongside `Inbox/` and `Archive/`.
  - Items from `Tabled/` receive `tags += ["tabled"]` and render a `🗂 Tabled by agent` chip.
- [x] Update `GEMINI.md` (Strict TODO & Backlog Handling) and documentation with the tabling protocol.

### Phase 4: Attention Tags & One-Click Session Routing Launchers (`rj-17`)
- [x] **Attention Tag Parser (`tools/check_mail.py` & `tools/models.py`)**:
  - Scan note frontmatter and body for attention markers:
    - `!IMPORTANT` $\rightarrow$ tag `important`, renders `⚠️ IMPORTANT` badge.
    - `!BLOATED` $\rightarrow$ tag `bloated`, renders `📦 BLOATED (needs split)` badge.
    - `!INCOMPLETE` $\rightarrow$ tag `incomplete`, renders `📝 INCOMPLETE (early draft)` badge.
    - `!SPIKE` $\rightarrow$ tag `spike`, renders `⚡ SPIKE` badge.
- [x] **One-Click Session Routing Launchers**:
  - Add session launcher action buttons to `master_template.html`:
    - `[ 🔬 Grill-Lab ]`: Dispatches `/grill-lab --topic "<title>"`
    - `[ 🧠 Deep Research ]`: Dispatches deep research prompt
    - `[ 🔨 Draft Plan ]`: Dispatches plan drafting prompt
  - In `sidecar.js`, handle IPC dispatch (Extension mode) or prompt box prefill + toast (File mode).

### Phase 5: `/mail-add` CLI with Position Splice (`tools/mail_add.py`)
- [x] Author `tools/mail_add.py`:
  - Usage: `python tools/mail_add.py "<title>" [--body TEXT] [--pillar NK] [--priority P1] [--after rj-05 | --rank N] [--dry-run]`
  - Writes new note into `C:\Dev\TheKlangVault\Inbox/`.
  - Slices new card into `.agents/pipeline/mailbag/rejigger.json` at the target position.
  - Renumbers subsequent cards' `rank` fields contiguously.
- [x] Author skill stub `.agents/skills/mail-add/SKILL.md` (triggers: `/mail-add`, "add to mailbag").
- [x] Mirror `tools/mail_add.py` to `c:\Dev\nkai\tools/` and `nkai/bundle/tools/`.

### Phase 6: Automated Verification & Documentation Closeout
- [x] Author unit test suite `tools/tests/test_intake_v2.py`:
  - Test ramble overflow slicing (> 3 ideas).
  - Test filename-only quick-note parsing.
  - Test attention tag extraction (`!IMPORTANT`, `!BLOATED`, etc.).
  - Test `mail_add.py` position splicing and rank renumbering.
- [x] Run full test suite:
  - `python -m unittest tools/tests/test_intake_v2.py tools/tests/test_board_hub.py tools/tests/test_sandbox_board.py`
- [x] Run Web QA linter `tools/audit_sidecar.py` across updated templates (0 errors).
- [x] Run `python tools/verify_utf8.py --all --strict` (assert 100% compliance).
- [x] Run `python tools/verify_context_clues.py` (assert 100% pass).
- [x] Author specification `docs/specs/nkai_mailbag_intake_and_triage_v2.md`.
- [x] Register in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.
- [x] Update `docs/GLOSSARY.md` (§22) and `docs/CHEAT_SHEET.md`.
- [x] Mark Flight 7 complete in `docs/BACKLOG.md` and Cards `rj-07`/`rj-17` in `rejigger.json`.
- [x] Check off all `[x]` in `PLAN.md` and set `Status: COMPLETED ✅`.
- [x] Append execution receipt to `subagent_receipt.md` and set dispatch `STATUS: COMPLETED`.
- [x] Commit cleanly across `TheKlangSuite` and `nkai`.
