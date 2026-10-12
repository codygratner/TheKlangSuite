# Implementation Plan: Flight 20 (The 3-Surface System & Frictionless Triage Ergonomics)

> **Task Name:** `nkai-3surfaces-runway-lab`
> **Target Pipeline:** `TARGET: TOOLING_HARNESS`
> **Active Milestone:** `N'kai v0.3.5 / Ecosystem Tooling Hardening`
> **Recommended Model Tier:** `Tier 2 (Gemini 3.8 Flash High)`
> **Status:** `COMPLETED ✅`

---

## 1. Flight Overview & Architecture
This flight unifies the N'kai sidecar ecosystem into a clean, 3-surface workflow that eliminates context switching, prevents board sprawl, and delivers instant, zero-friction capture and triage:
1. **Evergreen #1: The Mailbag (`morning_mail`)** — Daily human intake stream with an in-board Quick Capture input bar and debounced "Grab New Mail" button.
2. **Evergreen #2: The Flight Runway (`runway`)** — Cross-project delivery horizon mapped directly from `docs/BACKLOG.md` Act III with `[ Active Flights | Shipped / Milestones | All ]` filter rails and Git-backed versioning.
3. **Session Surface: The Lab (`lab`)** — A single active ephemeral slot (Grill-Me, Research, Spikes) pinned in the chat footer with an "ALL CLEAR" triumph banner and 3-state lifecycle (`ACTIVE` ➔ `STAGED` ➔ `ARCHIVED`).
4. **Chat-Driven Re-sequencing** — Zero UI drag-and-drop complexity; strategic roadmap ordering is executed via chat commands (`/audit-reorder`, `/rejigger`).
5. **The Scratchpad Protocol** — Dedicated [`C:\_\Vault\Inbox\SCRATCHPAD.md`](file:///C:/_/Vault/Inbox/SCRATCHPAD.md) file for stream-of-consciousness reading notes, permanently excluded from mailbag scans and linked in chat footers.

---

## 2. Phase-by-Phase Execution Checklist

### Phase 1: Scratchpad & 3-Surface Ambient Navigation
- [x] 1.1. Codify ambient footer navigation rule in `GEMINI.md`:
  `Focus: <Focus> | TKS-<ver> · TT-<ver> · NK-<ver> · TR-<ver> · [ 📝 Scratchpad ](file:///C:/_/Vault/Inbox/SCRATCHPAD.md) · [ 📬 Mailbag ](sidecar://nkai/panel/?board=morning_mail) · [ 🛫 Runway ](sidecar://nkai/panel/?board=runway)` *(+ `· [ 🔬 Lab: <topic> ](...)` when active)*.
- [x] 1.2. In `tools/models.py`, add `board_type: str = "custom"` to `BoardSchema` (`"evergreen"`, `"lab"`, `"custom"`).
- [x] 1.3. In `tools/models.py`, update `BoardRegistry` with `active_lab_id: Optional[str] = None` and `staged_labs: List[str] = []`.
- [x] 1.4. Mirror `tools/models.py` to `c:\Dev\nkai\tools/models.py` and `nkai/bundle/tools/models.py`.

### Phase 2: Mailbag In-Board Quick Capture & Daily Catchall
- [x] 2.1. In `c:\Dev\nkai\sidecars\panel\main.mjs`, implement `POST /api/quick-note`:
  - Receives `{ text: "..." }`.
  - Determines today's catchall file path: `C:\_\Vault\Inbox/<YYYY-MM-DD>_Quick_Notes.md`.
  - Initializes file with valid YAML frontmatter if missing; appends `- [<HH:MM>] <text>`.
  - Ingests new item into `mail_queue.json` and returns created item payload.
- [x] 2.2. In `c:\Dev\nkai\sidecars\panel\main.mjs`, implement `POST /api/check-mail`:
  - Enforces 500ms debounce latch.
  - Executes `python tools/check_mail.py --no-browser` and returns status.
- [x] 2.3. In `c:\Dev\nkai\core\master_template.html` & `.agents/sidecar/master_template.html`:
  - Add `#quick-capture-container` with `#quick-capture-input` and `#btn-quick-capture` in header tier.
  - Add debounced `#btn-grab-mail` in header tier.
  - Guard both elements to render strictly when `window.NKAI_EXTENSION_MODE === true`.
- [x] 2.4. In `c:\Dev\nkai\core\sidecar.css` & `sidecar.js`:
  - Add responsive styling for quick capture bar and grab button.
  - Implement client-side `submitQuickNote()` and `triggerMailGrab()`.
- [x] 2.5. Mirror templates across `TheKlangSuite`, `nkai/core`, and `nkai/bundle`.

### Phase 3: The Flight Runway Board (`runway_board.html`)
- [x] 3.1. Author `tools/build_runway.py`:
  - Scans `docs/BACKLOG.md` Act III flights, milestone headers, and git tags.
  - Generates declarative `BoardSchema(board_id="runway", title="Ecosystem Delivery Runway")`.
  - Produces card hierarchy with Pillar, Status, and Action tags.
  - Configures Filter Rails: `[ 🚀 Active Flights ]`, `[ 📦 Shipped & Milestones ]`, `[ 📋 All ]`.
  - Outputs to `.agents/sidecar/runway_board.html`.
- [x] 3.2. Register `runway` in `.agents/pipeline/boards/registry.json` as `type: "evergreen"`.
- [x] 3.3. Verify canonical mirroring to `nkai/sidecars/panel/runway_board.html`.

### Phase 4: Lab Board Lifecycle & "ALL CLEAR" Triumph
- [x] 4.1. In `c:\Dev\nkai\core\master_template.html` & `sidecar.css`:
  - Add `#lab-all-clear-banner` container with triumph styling, copy, and `[ 🏁 Conclude Lab & Stage Insights ]` button.
- [x] 4.2. In `c:\Dev\nkai\core\sidecar.js`:
  - Implement `checkAllClearState()` dynamically checking untriaged card counts.
  - Implement `concludeCurrentLab()` posting to `/api/conclude-lab`.
- [x] 4.3. Author `tools/conclude_lab.py`:
  - Stages markdown takeaways to `.agents/pipeline/staging/souls/<YYYY-MM-DD>_<slug>_harvest.md`.
  - Updates `registry.json` transitioning active lab to `staged_labs` and unpinning footer link.
- [x] 4.4. In `main.mjs`, support `POST /api/conclude-lab` executing `tools/conclude_lab.py`.

### Phase 5: Automated Verification Gauntlet & Documentation
- [x] 5.1. Author `tools/tests/test_runway_board.py`:
  - Asserts schema validation, filter rail rendering, and clean generation.
- [x] 5.2. Author `tools/tests/test_quick_capture.py`:
  - Asserts catchall file creation, timestamped appending, and zero-mutation on other notes.
- [x] 5.3. Run full test suite: `python -m unittest discover tools/tests/` (assert 100% PASS).
- [x] 5.4. Run `python tools/audit_sidecar.py` across updated templates and boards (assert 0 errors).
- [x] 5.5. Run `python tools/verify_utf8.py --strict` (assert 100% compliance).
- [x] 5.6. Run `python tools/verify_context_clues.py` (assert 100% pass).
- [x] 5.7. Author design specification: `docs/specs/nkai_three_surfaces_and_runway_board.md`.
- [x] 5.8. Register in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.
- [x] 5.9. Update `docs/GLOSSARY.md` (§26: Three-Surface Navigation & The Flight Runway) and `docs/CHEAT_SHEET.md`.
- [x] 5.10. Mark Flight 20 complete in `docs/BACKLOG.md`.
- [x] 5.11. Run `python tools/install_nkai_plugin.py` and restart supervisor.
- [x] 5.12. Check all boxes `[x]` in `PLAN.md` and commit across `TheKlangSuite` and `nkai`.
