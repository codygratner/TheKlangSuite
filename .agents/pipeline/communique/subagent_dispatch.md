STATUS: IDLE

# Subagent Dispatch Charter: Tooling Implementation
> *Target Pipeline:* Dual-Track Communiqué — Tooling Pipeline (`TARGET: TOOLING_HARNESS`)  
> *Active Branch:* `0.4.0-dev`  
> *Orchestrator:* Antigravity Main Chat (New Klang City)  
> *Executor:* Idle (Ready for next flight dispatch)  

---

## 1. Assignment Header
- **STATUS**: `IDLE`
- **ROLE**: `None`
- **MODEL TIER**: `Tier 2 Flash (High)`
- **TASK TYPE**: `TOOLING_IMPLEMENTATION`
- **TASK SLUG**: `idle`
- **ACTIVE BLUEPRINT**: [`c:\Dev\TheKlangSuite\PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md)
- **TARGET REPOSITORIES**:
  - [x] `TheKlangSuite` (`c:\Dev\TheKlangSuite`)
  - [x] `nkai` (`c:\Dev\nkai`)

---

## 2. Scope & Target Files
- **Primary Objective**:
  1. Author `docs/specs/nkai_interactive_triage_views_and_inbox_workflow.md` (Topics 6.8 & 6.11: Autopsy scoreboard, filter chips, 1-click cross-tab jump, vault mail harvester) and index in `DOCS_CATALOG.json` and `SYSTEM_MAP.md`.
  2. Implement Ongoing Autopsy View in `c:\Dev\nkai\core\sidecar.css` and `sidecar.js` (`#tab-autopsy`, `.triage-scoreboard`, `.filter-chip`, `jumpToWorkshopCard`, `copyPostMortemReport`).
  3. Author `tools/check_mail.py` in `c:\Dev\nkai\tools/` and `c:\Dev\TheKlangSuite\tools/` (scanning `TheKlangVault/Inbox/` with `--archive` and `--chat` fallbacks, generating `morning_mail_inbox.html`).
  4. Synchronize canonical templates: `c:\Dev\nkai\dist\standalone-template.html`, `c:\Dev\nkai\bundle\templates\template.html`, and `c:\Dev\TheKlangSuite\.agents\sidecar\template.html`.
  5. Run `python tools/audit_sidecar.py` across updated templates (100% tag balance) and test `python tools/check_mail.py --help`.
  6. Commit changes cleanly across `TheKlangSuite` and `nkai` with conventional commits: `feat(triage): implement Flight 6 interactive autopsy feed and check-mail workflow`.
- **Target Files / Boundaries**:
  - `docs/specs/nkai_interactive_triage_views_and_inbox_workflow.md`
  - `docs/DOCS_CATALOG.json`
  - `docs/SYSTEM_MAP.md`
  - `c:\Dev\nkai\core\sidecar.css`
  - `c:\Dev\nkai\core\sidecar.js`
  - `c:\Dev\nkai\tools\check_mail.py`
  - `c:\Dev\TheKlangSuite\tools\check_mail.py`
  - `c:\Dev\nkai\dist\standalone-template.html`
  - `c:\Dev\nkai\bundle\templates\template.html`
  - `c:\Dev\TheKlangSuite\.agents\sidecar\template.html`
  - `.agents/pipeline/communique/subagent_receipt.md`

---

## 3. Mandatory Invariants & Constraints
- **Zero Allocations & Audio Safety**: Tooling implementation; zero native C++ audio thread code touched.
- **Context Protection**: Complete all file writing, script creation, and verification inside subagent context. Return only a concise 3-line receipt.
- **Boy Scout Cleanliness**: Zero external dependencies in `check_mail.py` (standard Python library only). 100% valid HTML in generated sidecar boards.

