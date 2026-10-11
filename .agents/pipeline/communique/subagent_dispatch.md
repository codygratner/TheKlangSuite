STATUS: COMPLETED

# Subagent Dispatch Charter: Tooling Implementation
> *Target Pipeline:* Dual-Track Communiqué — Tooling Pipeline (`TARGET: TOOLING_HARNESS`)
> *Active Branch:* `0.4.0-dev` (TKS) / `main` (nkai)
> *Orchestrator:* Antigravity Main Chat (New Klang City)
> *Executor:* Flash Builder Subagent

---

## 1. Assignment Header
- **STATUS**: `DISPATCHED`
- **ROLE**: `Flash Tooling Builder`
- **MODEL TIER**: `Tier 2 Flash (High)`
- **TASK TYPE**: `TOOLING_IMPLEMENTATION`
- **TASK SLUG**: `mailbag-intake-v2`
- **TARGET REPOSITORIES**: `TheKlangSuite`, `nkai`
- **TICKET ID**: `plan-012` (Flight 7 / `rj-07` & `rj-17`)

## 2. Scope & Instructions

Follow the active plan in `c:\Dev\TheKlangSuite\PLAN.md` and deliver Phases 1 through 6:

### Phase 1: Ramble Overflow Queue (`tools/ramble_ingest.py`)
1. In `c:\Dev\TheKlangSuite\tools\models.py`:
   - Add `split_group: str = ""` to `RambleCard` and `MailItem`.
2. In `c:\Dev\TheKlangSuite\tools\ramble_ingest.py`:
   - When decomposing a note into $> 3$ ideas:
     - Top 3 become active `RambleCard`s returned in the extraction result.
     - Remaining cards are written to `c:\Dev\TheKlangSuite\.agents\pipeline\mailbag\overflow.json` (as a JSON array of `RambleCard` objects).
     - Each card stores `source_note` (path to original note) and `split_group` (stem of original note).
3. In `master_template.html` & `tools/board_renderer.py`:
   - If a card has overflow items in `overflow.json` with matching `split_group`, render a chip `⤵ N in overflow`.
   - In Rail 2 (Status filters), add an `Overflow` filter pill after `All` if overflow items exist.
4. Mirror changes to `c:\Dev\nkai\tools/` and `c:\Dev\nkai\bundle/`.

### Phase 2: Filename-Only Quick Notes (`tools/check_mail.py`)
1. In `c:\Dev\TheKlangSuite\tools\check_mail.py`:
   - When reading a note: if the body (after frontmatter stripping) is empty or whitespace only:
     - Set card `title` to the note's filename stem.
     - Add tag `"quick-note"`.
     - Set `preview` to `"(quick note — title only)"`.
2. Add a quick-note card fixture to `assets/fixtures/sandbox_mail_queue.json` (`id="sbx-quick-01"`, `title="Quick Mobile Thought"`, `tags=["quick-note", "tks"]`).
3. Verify clean rendering on `sandbox_mail_inbox.html`.

### Phase 3: `Inbox/Tabled/` for Agent-Authored Notes
1. Create `C:\Dev\TheKlangVault\Inbox\Tabled\` directory (and support local fallback `docs/inbox/tabled/`).
2. In `tools/check_mail.py`:
   - Scan `Inbox/Tabled/` alongside `Inbox/` and `Archive/`.
   - Items from `Tabled/` receive `tags += ["tabled"]` and render a `🗂 Tabled by agent` chip.
3. Update `GEMINI.md` (Strict TODO & Backlog Handling) and documentation with the tabling protocol.

### Phase 4: Attention Tags & One-Click Session Routing Launchers (`rj-17`)
1. In `tools/check_mail.py` & `tools/models.py`:
   - Parse markers from frontmatter and note body:
     - `!IMPORTANT` $\rightarrow$ tag `important`, renders `⚠️ IMPORTANT` badge.
     - `!BLOATED` $\rightarrow$ tag `bloated`, renders `📦 BLOATED (needs split)` badge.
     - `!INCOMPLETE` $\rightarrow$ tag `incomplete`, renders `📝 INCOMPLETE (early draft)` badge.
     - `!SPIKE` $\rightarrow$ tag `spike`, renders `⚡ SPIKE` badge.
2. In `master_template.html` & `sidecar.css`:
   - Add styles for `.badge-attention-important`, `.badge-attention-bloated`, `.badge-attention-incomplete`, `.badge-attention-spike`.
   - Add session launcher buttons in card action rows:
     - `[ 🔬 Grill-Lab ]`: Dispatches `/grill-lab --topic "<clean_title>"`
     - `[ 🧠 Deep Research ]`: Dispatches targeted research turn
     - `[ 🔨 Draft Plan ]`: Dispatches plan drafting turn
3. In `sidecar.js`:
   - Handle button clicks: in extension mode dispatches via `window.sidecar.agent.sendMessage()`; in file mode copies to clipboard and flashes toast.

### Phase 5: `/mail-add` CLI with Position Splice (`tools/mail_add.py`)
1. Author `c:\Dev\TheKlangSuite\tools\mail_add.py`:
   - Python stdlib + Pydantic v2 + Universal UTF-8.
   - Arguments: `title` (required), `--body TEXT`, `--pillar NK` (default TKS), `--priority P1` (default P1), `--after ID` or `--rank N`, `--dry-run`.
   - Writes note to `C:\Dev\TheKlangVault\Inbox/<title>.md` (or local fallback).
   - Splices card into `rejigger.json` at target rank and renumbers subsequent cards' `rank` contiguously.
2. Author skill `.agents/skills/mail-add/SKILL.md` (triggers: `/mail-add`, "add to mailbag").
3. Mirror to `c:\Dev\nkai\tools/` and `c:\Dev\nkai\bundle/`.

### Phase 6: Automated Verification & Documentation Closeout
1. Author unit tests `c:\Dev\TheKlangSuite\tools\tests\test_intake_v2.py`:
   - Test ramble overflow slicing (> 3 ideas).
   - Test filename-only quick-note parsing.
   - Test attention tag extraction (`!IMPORTANT`, `!BLOATED`, etc.).
   - Test `mail_add.py` position splicing and contiguous rank renumbering.
2. Run test suites:
   `python -m unittest tools/tests/test_intake_v2.py tools/tests/test_board_hub.py tools/tests/test_sandbox_board.py`
3. Audit templates:
   `python tools/audit_sidecar.py .agents/sidecar/sandbox_mail_inbox.html`
   `python tools/audit_sidecar.py .agents/sidecar/morning_mail_inbox.html`
   `python tools/audit_sidecar.py .agents/sidecar/master_template.html`
4. Run `python tools/verify_utf8.py --all --strict` (assert 100% compliance).
5. Run `python tools/verify_context_clues.py` (assert 100% pass).
6. Run `python tools/install_nkai_plugin.py`.
7. Author specification `docs/specs/nkai_mailbag_intake_and_triage_v2.md`.
8. Register in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.
9. Update `docs/GLOSSARY.md` (§22: Mailbag Intake & Triage v2) and `docs/CHEAT_SHEET.md`.
10. Update `docs/BACKLOG.md` (mark Flight 7 complete) and Cards `rj-07`/`rj-17` in `rejigger.json`.
11. Check all boxes `[x]` in `PLAN.md` and set `Status: COMPLETED ✅`.
12. Append execution receipt to `subagent_receipt.md` and set dispatch `STATUS: COMPLETED`.
13. Commit `TheKlangSuite`: `feat(mailbag): intake & triage v2 — overflow, quick notes, attention tags, session routing, tabled folder, mail-add`.
14. Commit `nkai`: `feat(mailbag): sync intake & triage v2 tools, models, and template`.
