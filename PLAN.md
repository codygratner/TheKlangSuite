# 🎨 Flight 15: Unified Master Template & Schema-Driven Boards (`rj-04`)

> **ID:** plan-010  
> **TARGET:** TOOLING_HARNESS (New Klang City) 🚀  
> **Topic Target:** Mailbag Card `rj-04` / BACKLOG Section 15 ("Unified Master Template + Schema-Driven Boards")  
> **Status:** COMPLETED ✅  
> **Priority:** P0 (Top Priority)  
> **Target Branch:** `0.4.0-dev`  
> **Recommended Setting:** Gemini 3.8 Flash (Thinking: High) — 🟢 SUSTAINABLE  
> **Source Note:** [`C:\Dev\TheKlangVault\Inbox\Archive\Nkai.md`](file:///C:/Dev/TheKlangVault/Inbox/Archive/Nkai.md) + [`how stupid is it to make nkai's backend data driven like TKS.md`](file:///C:/Dev/TheKlangVault/Inbox/Archive/how%20stupid%20is%20it%20to%20make%20nkai's%20backend%20data%20driven%20like%20TKS.md)  
> **Prerequisites:** Flight 18 (UTF-8) ✅, Flight 16+17 (QA Sandbox Board) ✅  
> **Dependents:** Flight 14 (Multi-Board Hub & Session Manager) ⏳, Flight 19 (Per-Card Response Drawer) ⏳  

---

## 🎯 Architectural Intent & Storyline

Previously, N'kai boards diverged into two parallel evolutionary branches:
1. **The Research Board (`standalone-template.html` / `nkai_deep_research.html`)**: Rich hero stage panels, 4-lens PCDA accordions, and deep-link jump navigation, but carrying legacy localhost loopback bridge code (`:4040` / `:8000`) and missing the modern Two-Tier header and allowance meter.
2. **The Mailbag Inbox (`check_mail.py` / `morning_mail_inbox.html`)**: Two-Tier header, live token allowance meter, session-stable two-rail filters, deterministic IDs, and sandbox mode, but embedded as a 1,500-line Python f-string that required editing Python code whenever UI layout evolved.

**Flight 15** synthesizes the best of both into a **single, declarative Unified Master Template** powered by one clean schema JSON:
- **Zero HTML editing for new boards**: Whether generating a Morning Mail inbox, a Deep Research sprint, a Milestone Triage canvas, or an SQA audit board, the UI structure is driven entirely by a validated Pydantic model (`BoardSchema`).
- **Keep the Gems**: Hero stage showcase, in-canvas 4-lens accordions, deterministic DOM IDs, Two-Tier sticky header, procedural Web Audio haptics, and zero-copy native Antigravity IPC (`window.sidecar.agent.sendMessage`).
- **Cut the Cruft**: Completely excise phantom localhost loopback bridge endpoints (`:4040`/`:8000`), buried toasts, and misleading "sent" claims.
- **Anti-Over-Engineering Guardrail**: No heavyweight JavaScript web frameworks (no React, Vue, Svelte, or Webpack). Pure zero-dependency vanilla HTML5/CSS3/ES6 and Python standard library.

---

## 📋 Phase Breakdown

### Phase 0: Codify the Deep Docs Queue (`.agents/pipeline/docs/deep_docs_queue.json`)
- [x] Initialize `.agents/pipeline/docs/deep_docs_queue.json` schema:
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
- [x] Add helper function in `tools/` or documentation to append items in 1 line.

### Phase 1: Declarative Board Data Models (`tools/models.py`)
- [x] In `tools/models.py`, define Pydantic models for declarative boards:
  - `HeroStageItem`: `id`, `title`, `subtitle`, `badge`, `content_html` or `markdown`, `context_target`.
  - `FilterRailItem`: `id`, `label`, `icon`, `count`, `filter_key`, `filter_type` (`"pillar"` | `"status"` | `"custom"`).
  - `FilterRail`: `rail_id`, `label`, `items: List[FilterRailItem]`.
  - `BoardActionDefinition`: `action_id`, `label`, `icon`, `color_var`, `prompt_template`, `shortcut`.
  - `BoardSchema`:
    - `board_id`: str (e.g. `"mailbag"`, `"sandbox"`, `"deep_research"`, `"audit"`)
    - `title`: str
    - `subtitle`: str = ""
    - `environment`: str = "PRODUCTION" (`"PRODUCTION"` | `"SANDBOX"`)
    - `has_hero_stage`: bool = False
    - `hero_items`: List[HeroStageItem] = []
    - `filter_rails`: List[FilterRail] = []
    - `available_actions`: List[BoardActionDefinition] = []
    - `cards`: List[MailItem] = []
- [x] Export canonical JSON schema to `c:\Dev\nkai\core\schema.json`.
- [x] Mirror models to `c:\Dev\nkai\tools\models.py`.

### Phase 2: Master Template HTML/CSS/JS Consolidation
- [x] Author `c:\Dev\nkai\core\master_template.html` (and mirror to `c:\Dev\TheKlangSuite\.agents\sidecar\master_template.html`):
  - **Header**: Two-Tier Sticky Header with live brand title, font zoom scaler (`A-`/`100%`/`A+`), Web Audio haptics toggle (`🔊`), allowance meter, and refresh action.
  - **Sandbox Banner**: Conditional sticky amber banner `#sandbox-banner` when `environment === "SANDBOX"`.
  - **Hero Stage**: Conditional Hero Stage panel with context navigation, active item selection, and smooth transitions.
  - **Consolidated Rails**: Declarative Rail 1 (Pillars/Categories) and Rail 2 (Status/Triage) with deterministic IDs (`pill-<rail>-<key>`).
  - **Cards Stream**: Responsive CSS grid with deterministic card IDs, priority badges, prerequisite chips, merged-note chips, 4-lens PCDA accordions, and action buttons (`btn-<action>-<id>`).
  - **Deterministic Dispatch Engine**:
    - Mode 1: Antigravity IDE UI Extension (`window.sidecar.agent.sendMessage`).
    - Mode 2: Local Sidecar Server / CLI harness fallback.
    - Mode 3: Silent clipboard copy with clear toast.
    - Sandbox isolation: Prepend `[QA_TEST_MOCK] `, bypass network POSTs, namespace localStorage keys to `nkai_sbx_`.

### Phase 3: Zero-HTML-Edit Generator Engine (`tools/board_renderer.py`)
- [x] Author `tools/board_renderer.py`:
  - `render_board(schema: BoardSchema, out_path: Path) -> Path`:
    - Injects schema JSON and card items into `master_template.html`.
    - Generates 100% self-contained, valid HTML artifact.
    - Preserves UTF-8 encoding across all streams.
- [x] Refactor `tools/check_mail.py` to use `BoardSchema` and `board_renderer.py`:
  - Decomposes the ~1,500-line monolithic inline HTML string in `check_mail.py` into clean data structure construction.
  - Generates `.agents/sidecar/morning_mail_inbox.html` and `.agents/sidecar/sandbox_mail_inbox.html` using the master renderer.
- [x] Mirror `board_renderer.py` to `c:\Dev\nkai\tools/board_renderer.py` and `c:\Dev\nkai\bundle\tools/board_renderer.py`.

### Phase 4: Automated Verification & Sandbox Regression Gauntlet
- [x] Run `python -m unittest tools/tests/test_sandbox_board.py` (assert 4/4 PASS).
- [x] Expand `test_sandbox_board.py` with `test_board_schema_validation()` to assert `BoardSchema` parsing.
- [x] Run `python tools/audit_sidecar.py .agents/sidecar/sandbox_mail_inbox.html` (assert 0 errors, PERFECT PASS).
- [x] Run `python tools/audit_sidecar.py .agents/sidecar/morning_mail_inbox.html` (assert 0 errors, PERFECT PASS).
- [x] Run `python tools/check_mail.py --archive --no-browser` (confirm 37 production cards, audit PERFECT PASS).
- [x] Verify `python tools/verify_utf8.py --all --strict` (assert 100% compliance).
- [x] Verify `python tools/verify_context_clues.py` (assert 100% pass).

### Phase 5: Design Spec, Documentation & Multi-Repo Commits
- [x] Author design spec `docs/specs/nkai_unified_master_template_and_schema.md`:
  - Document the unified data-driven board architecture.
  - Document the `BoardSchema` JSON specification and rendering lifecycle.
- [x] Register spec in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.
- [x] Update `docs/GLOSSARY.md` (§20: Unified Master Template) and `docs/CHEAT_SHEET.md`.
- [x] Mark Flight 15 as `[x] COMPLETED` in `docs/BACKLOG.md` and Card `rj-04` in `.agents/pipeline/mailbag/rejigger.json`.
- [x] Commit changes cleanly across `TheKlangSuite` and `nkai`.
