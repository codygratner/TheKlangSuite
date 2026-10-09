# Implementation Plan: Flight 6 — Interactive Triage Views & Inbox Workflow
> **TARGET:** TOOLING_HARNESS (New Klang City) 🚀  
> **Flight Target:** Topics 6.8 & 6.11 (Ongoing Autopsy Feed & Check the Mail Workflow)  
> **Status:** COMPLETED ✅  
> **Target Branch:** `0.4.0-dev`  
> **Recommended Model:** Gemini 3.8 Flash (Thinking: High) — 🟢 SUSTAINABLE  
> **Execution Strategy:** Delegated to Tier 2 Flash Builder Subagent via `subagent_dispatch.md`  

---

## 🎯 Architectural Intent & Scope

Flight 6 delivers the interactive triage surfaces and mail ingestion workflows for N'kai:
1. **Topic 6.8 (Approved Option B — "Ongoing Autopsy": Live Triage Feed & Scoreboard):**
   - Implements the dedicated Autopsy view (`#tab-autopsy` / `.view-autopsy`) positioned in Slot 2 of the navigation rail:
     - **Triage Scoreboard**: High-contrast summary bar displaying Total Topics, Approved (`[N APPROVED]`), Tabled (`[N TABLED]`), Killed (`[N KILLED]`), and Pending (`[N PENDING]`).
     - **Filter Chips**: Real-time JavaScript filtering (`[All]`, `[Approved]`, `[Tabled]`, `[Killed]`, `[Pending]`).
     - **Triage Feed Stream**: Compact row cards displaying Topic ID, Title, Status badge, and a 1-click `[ 🔍 Jump to Workshop Card ]` button that switches view to `#tab-workshop`, scrolls smoothly to the target card, and flashes an accent pulse.
     - **Report Serializer**: `[ 📋 Copy Post-Mortem Report ]` action that compiles the active verdicts into clean, GitHub-flavored Markdown ready for clipboard or disk.
2. **Topic 6.11 (Approved Option B — "Check the Mail" Workflow & Sidecar Board Default):**
   - Authors `tools/check_mail.py` in `TheKlangSuite` and `nkai`:
     - Discovers and parses Markdown notes in `C:\Dev\TheKlangVault\Inbox\` (with `--archive` and local `docs/inbox/` fallbacks).
     - Generates an interactive N'kai triage board (`morning_mail_inbox.html`) presenting notes as interactive triage cards with action chips (`[ 📥 Add to Backlog ]`, `[ 🔬 Move to Research ]`, `[ 🪦 Reject / Graveyard ]`, `[ ⚖️ Request PCDA ]`).
     - Supports `--chat` / `--text-only` flag to dump a clean markdown list in terminal without opening HTML.
3. **Formal Specification & Registration**:
   - Authors `docs/specs/nkai_interactive_triage_views_and_inbox_workflow.md` formalizing data contracts, scoreboard state machines, and inbox triage schemas.
   - Registers in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.

---

## 📋 Phase-by-Phase Execution Plan

### Phase 1: Specification & Catalog Registration (Topics 6.8 & 6.11)
- [x] Author `docs/specs/nkai_interactive_triage_views_and_inbox_workflow.md`:
  - Document the Autopsy scoreboard data model and DOM filter event contracts.
  - Document the 1-click cross-tab jump mechanics and visual pulse animation.
  - Document `tools/check_mail.py` CLI interface, note parsing heuristics, and board generator template.
- [x] Register spec in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.

### Phase 2: Ongoing Autopsy View & Filterable Scoreboard in `nkai/core`
- [x] In `c:\Dev\nkai\core\sidecar.css`:
  - Style the Scoreboard banner (`.triage-scoreboard`, metric pills with semantic colors).
  - Style the filter chips (`.filter-chip`, `.filter-chip.active`).
  - Style the compact autopsy feed rows (`.autopsy-feed-item`, status badges).
  - Add `@keyframes cardPulse` for the cross-tab jump target focus.
- [x] In `c:\Dev\nkai\core\sidecar.js`:
  - Implement `updateAutopsyScoreboard()` to dynamically count and render current verdicts.
  - Implement `filterAutopsyFeed(status)` for instantaneous DOM filtering.
  - Implement `jumpToWorkshopCard(topicId)` to switch tabs, scroll target into view, and apply `.pulse-target`.
  - Implement `copyPostMortemReport()` to serialize the scorecard into Markdown and copy to clipboard.

### Phase 3: Zero-Dependency "Check the Mail" Harvester (`tools/check_mail.py`)
- [x] Author `c:\Dev\nkai\tools\check_mail.py` (and mirror to `c:\Dev\TheKlangSuite\tools\check_mail.py`):
  - Standard library only (`os`, `sys`, `pathlib`, `argparse`, `re`, `json`, `webbrowser`).
  - Scans `C:\Dev\TheKlangVault\Inbox\` (default) or `--dir <path>` or `--archive`.
  - Extracts title, creation date, and body content from `.md` files.
  - Generates standalone interactive HTML triage board (`.agents/sidecar/morning_mail_inbox.html`).
  - Supports `--chat` / `--text-only` flag to output a clean markdown summary in console.
  - Supports auto-opening browser unless `--no-browser` is passed.

### Phase 4: Canonical Template Synchronization
- [x] Update `c:\Dev\nkai\dist\standalone-template.html` with `#tab-autopsy` markup and scoreboard skeleton.
- [x] Update `c:\Dev\nkai\bundle\templates\template.html` with identical markup.
- [x] Update `c:\Dev\TheKlangSuite\.agents\sidecar\template.html`.

### Phase 5: Verification Gauntlet & Closeout
- [x] Run `python tools/audit_sidecar.py` across updated templates (100% tag balance, zero syntax errors).
- [x] Test `python tools/check_mail.py --help` and verify `--chat` dry run on `C:\Dev\TheKlangVault\Inbox\Archive`.
- [x] Run `python tools/verify_context_clues.py` in `c:\Dev\TheKlangSuite`.
- [x] Commit changes cleanly across `TheKlangSuite` and `nkai` with conventional commits: `feat(triage): implement Flight 6 interactive autopsy feed and check-mail workflow`.

---

## 🛡️ Verification & Acceptance Criteria
1. `docs/specs/nkai_interactive_triage_views_and_inbox_workflow.md` is valid and indexed.
2. `#tab-autopsy` renders with live scoreboard counts and instant filter chips.
3. Clicking `[ 🔍 Jump ]` in Autopsy switches to Workshop and pulses the target card.
4. `tools/check_mail.py` runs cleanly with zero third-party dependencies, correctly parsing vault notes and generating valid HTML boards or `--chat` summaries.
5. `audit_sidecar.py` and `verify_context_clues.py` pass 100%.
