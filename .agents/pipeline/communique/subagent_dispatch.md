STATUS: COMPLETED

# Subagent Dispatch Charter: Tooling Implementation
> *Target Pipeline:* Dual-Track Communiqué — Tooling Pipeline (`TARGET: TOOLING_HARNESS`)
> *Active Branch:* `0.4.0-dev` (TKS) / `main` (nkai)
> *Orchestrator:* Antigravity Main Chat (New Klang City)
> *Executor:* Flash Builder Subagent

---

## 1. Assignment Header
- **STATUS**: `COMPLETED`
- **ROLE**: `Flash Tooling Builder`
- **MODEL TIER**: `Tier 2 Flash (High)`
- **TASK TYPE**: `TOOLING_IMPLEMENTATION`
- **TASK SLUG**: `unified-master-template`
- **TARGET REPOSITORIES**: `TheKlangSuite`, `nkai`
- **TICKET ID**: `plan-010` (Flight 15 / `rj-04`)

## 2. Scope & Instructions

Follow the active plan in `c:\Dev\TheKlangSuite\PLAN.md` and deliver Phases 0 through 5:

### Phase 0: Codify the Deep Docs Queue
1. Author `.agents/pipeline/docs/deep_docs_queue.json`:
   ```json
   {
     "$schema": "https://json-schema.org/draft/2020-12/schema",
     "version": "1.0.0",
     "queue": [
       {
         "id": "doc-01",
         "title": "N'kai Sidecar UI Extension Setup Guide (ELI5 Level)",
         "audience": "Developers & Musicians",
         "target_file": "TheKlangResearch/guides/nkai_sidecar_quickstart_eli5.md",
         "status": "QUEUED",
         "priority": "P1",
         "created_at": "2026-10-11T20:35:00Z"
       },
       {
         "id": "doc-02",
         "title": "DAW Backend Dilemma: Replacing the Ardour Monolith with Headless C++20 (No-BS Case Study)",
         "audience": "Senior Audio Software Architects",
         "target_file": "TheKlangResearch/qa_autopsies/daw_backend_replacement_ardour_vs_bespoke.md",
         "status": "QUEUED",
         "priority": "P1",
         "created_at": "2026-10-11T20:35:00Z"
       }
     ]
   }
   ```
2. Mirror to `c:\Dev\nkai\tools\deep_docs_queue.json` if applicable.

### Phase 1: Declarative Board Data Models (`tools/models.py`)
1. In `c:\Dev\TheKlangSuite\tools\models.py`:
   - Add models:
     - `HeroStageItem`: `id: str`, `title: str`, `subtitle: str = ""`, `badge: str = ""`, `body_html: str = ""`, `context_target: Optional[str] = None`.
     - `FilterRailItem`: `id: str`, `label: str`, `icon: str`, `count: int = 0`, `filter_key: str`, `filter_type: str = "status"`, `active: bool = False`.
     - `FilterRail`: `rail_id: str`, `label: str`, `items: List[FilterRailItem] = []`.
     - `BoardActionDefinition`: `action_id: str`, `label: str`, `icon: str`, `color_var: str = "var(--accent-blue)"`, `prompt_template: str = ""`, `shortcut: str = ""`.
     - `BoardSchema`:
       - `schema_version: str = "1.0.0"`
       - `board_id: str` (e.g. `"morning_mail"`, `"sandbox_mail"`, `"deep_research"`, `"sqa_triage"`)
       - `title: str`
       - `subtitle: str = ""`
       - `environment: str = "PRODUCTION"` (`"PRODUCTION"` or `"SANDBOX"`)
       - `has_hero_stage: bool = False`
       - `hero_items: List[HeroStageItem] = []`
       - `filter_rails: List[FilterRail] = []`
       - `available_actions: List[BoardActionDefinition] = []`
       - `cards: List[MailItem] = []`
       - `allowance_budget: int = 200000`
2. Export JSON schema to `c:\Dev\nkai\core\schema.json` via `BoardSchema.model_json_schema()`.
3. Mirror `tools/models.py` to `c:\Dev\nkai\tools\models.py`.

### Phase 2: Master Template HTML/CSS/JS Consolidation
1. Author `c:\Dev\nkai\core\master_template.html` (mirror to `c:\Dev\TheKlangSuite\.agents\sidecar\master_template.html` and `c:\Dev\nkai\bundle\templates\master_template.html`):
   - Synthesize the Two-Tier header, font-scaler, Web Audio API haptics, allowance meter, `#sandbox-banner`, optional hero stage panel, two-rail filter chips, deterministic IDs (`btn-<action>-<id>`, `pill-<rail>-<key>`), and native IPC adapter (`window.sidecar.agent.sendMessage()`).
   - Clean out dead loopback bridge URLs (`:4040`, `:8000` fetches when offline).
   - In Sandbox mode: prepend `[QA_TEST_MOCK] `, namespace localStorage to `nkai_sbx_`, and bypass network POSTs.
   - Clean, validated HTML5 with zero tag imbalances.

### Phase 3: Zero-HTML-Edit Generator Engine (`tools/board_renderer.py`)
1. Author `c:\Dev\TheKlangSuite\tools\board_renderer.py`:
   - `render_board(schema: BoardSchema, out_path: Path) -> Path`: Ingests `BoardSchema`, loads `master_template.html`, serializes cards, rails, and metadata, and writes output HTML with UTF-8 encoding.
2. In `c:\Dev\TheKlangSuite\tools\check_mail.py`:
   - Refactor `generate_html_inbox_board()` to construct a `BoardSchema` object and call `render_board()`.
   - Maintain 100% backward compatibility for `--sandbox`, `--archive`, `--chat`, etc.
3. Mirror `tools/board_renderer.py` and `tools/check_mail.py` to `c:\Dev\nkai\tools/` and `c:\Dev\nkai\bundle/`.

### Phase 4: Automated Verification & Sandbox Regression Gauntlet
1. Run `python -m unittest tools/tests/test_sandbox_board.py` (ensure existing 4 tests pass).
2. Add a 5th test in `test_sandbox_board.py`: `test_board_schema_roundtrip()` validating `BoardSchema` serialization and rendering.
3. Run `python -m unittest tools/tests/test_sandbox_board.py` (assert 5/5 PASS).
4. Run `python tools/check_mail.py --sandbox --no-browser`.
5. Run `python tools/audit_sidecar.py .agents/sidecar/sandbox_mail_inbox.html` (assert PERFECT PASS).
6. Run `python tools/check_mail.py --archive --no-browser`.
7. Run `python tools/audit_sidecar.py .agents/sidecar/morning_mail_inbox.html` (assert PERFECT PASS).
8. Run `python tools/verify_utf8.py --all --strict` (assert 100% compliance).
9. Run `python tools/verify_context_clues.py` (assert 100% pass).

### Phase 5: Documentation, Sync & Multi-Repo Commits
1. Author `c:\Dev\TheKlangSuite\docs\specs\nkai_unified_master_template_and_schema.md`:
   - Document the data-driven `BoardSchema` model, the master template lifecycle, and how new board types are created with zero HTML changes.
2. Register spec in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.
3. Update `docs/GLOSSARY.md` (Section 20: Unified Master Template & Schema-Driven Boards) and `docs/CHEAT_SHEET.md`.
4. Update `docs/BACKLOG.md` (mark Flight 15 as `[x] COMPLETED`) and `rejigger.json` (mark Card `rj-04` as `status: "COMPLETED"`, `verdict: "COMPLETED"`).
5. In `PLAN.md`: check all phase boxes `[x]` and update Status to `COMPLETED ✅`.
6. Append execution receipt to `subagent_receipt.md` and set dispatch `STATUS: COMPLETED`.
7. Commit cleanly across both repositories (`0.4.0-dev` on TKS, `main` on nkai).

## 3. Acceptance Criteria
- `BoardSchema` defined in `models.py` and exported to `core/schema.json`.
- `master_template.html` and `board_renderer.py` created and operational.
- Monolithic HTML f-string extracted out of `check_mail.py`.
- `tools/tests/test_sandbox_board.py` passes 5/5.
- Both sandbox and production HTML pass `audit_sidecar.py` with 0 issues.
- Spec authored, registered, and committed.
