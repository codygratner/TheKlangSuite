# Specification: N'kai Unified Master Template & Schema-Driven Boards

> **Status:** APPROVED & CODIFIED ✅  
> **Flight Target:** N'kai Flight 15 (`rj-04`)  
> **Scope:** N'kai Sidecar Architecture & Declarative Webviews  
> **Primary Components:** `c:\Dev\nkai\core\master_template.html`, `tools/board_renderer.py`, `tools/models.py`, `c:\Dev\nkai\core\schema.json`  
> **Target Repositories:** `TheKlangSuite`, `nkai`  

---

## 1. Executive Summary & Problem Statement

Prior to Flight 15, the N'kai user interface evolved along two disconnected paths:
1. **The Research Board (`nkai_deep_research.html`, `standalone-template.html`)**: Rich hero stage panels, 4-lens PCDA accordions, and deep-link jump navigation, but carrying legacy localhost loopback bridge code (`:4040` / `:8000`) and missing the modern Two-Tier header and allowance meter.
2. **The Mailbag Inbox (`morning_mail_inbox.html`, `check_mail.py`)**: Two-Tier header, live token allowance meter, session-stable two-rail filters, deterministic element IDs, and sandbox mode, but embedded as a 1,570-line Python f-string that required editing Python code whenever UI layout evolved.

**Flight 15** establishes the **Unified Master Template & Schema-Driven Boards** architecture. It separates data definitions from presentation markup completely, allowing any tool, script, or subagent to generate an interactive, production-grade N'kai board with **zero HTML edits**.

---

## 2. Declarative Data Architecture: `BoardSchema`

The interface is driven by a validated Pydantic model in [`tools/models.py`](file:///c:/Dev/TheKlangSuite/tools/models.py):

```python
class BoardSchema(BaseModel):
    schema_version: str = "1.0.0"
    board_id: str                      # Unique slug e.g. "morning_mail", "sandbox_mail", "deep_research"
    title: str                         # Header brand title
    subtitle: str = ""                 # Optional context or scope subtitle
    environment: str = "PRODUCTION"    # "PRODUCTION" | "SANDBOX"
    has_hero_stage: bool = False       # Enables top hero showcase panel
    hero_items: List[HeroStageItem] = []
    filter_rails: List[FilterRail] = []
    available_actions: List[BoardActionDefinition] = []
    cards: List[MailItem] = []
    allowance_budget: int = 200000
```

The canonical JSON schema is exported directly to [`c:\Dev\nkai\core\schema.json`](file:///c:/Dev/nkai/core/schema.json).

---

## 3. The Unified Master Template (`master_template.html`)

Located at [`c:\Dev\nkai\core\master_template.html`](file:///c:/Dev/nkai/core/master_template.html) (and mirrored in `.agents/sidecar/` and `bundle/templates/`):

### 3.1 Kept Capabilities
- **Two-Tier Header**: Sticky top tier with brand title, font zoom scaler (`A-`/`100%`/`A+`), Web Audio haptics toggle (`🔊`), token allowance meter, and refresh action. Bottom tier hosts horizontal scrollable filter rails.
- **Session-Stable Rails**: Rail 1 (Platform Pillars) and Rail 2 (Status/Triage) with deterministic IDs (`pill-<rail>-<slug>`).
- **Deterministic Action IDs**: Every interactive action button on every card is assigned a deterministic ID: `btn-<action>-<clean_id>` (e.g. `btn-approve-sbx-01`, `btn-backlog-rj-04`).
- **Sandbox Mode Isolation**: When `environment === "SANDBOX"`, renders `#sandbox-banner`, prepends `[QA_TEST_MOCK]` to prompts, uses `nkai_sbx_` localStorage key namespace, and bypasses persistence.
- **Zero-Copy Native IPC**: Prioritizes `window.sidecar.agent.sendMessage()` when running inside the Antigravity auxiliary pane (`SIDECAR_UI_ENTRYPOINT_AUX_PANE`), falling back to clipboard copy.

### 3.2 Excised Anti-Patterns
- **No Localhost Loopback Bridges**: Dead `:4040` and `:8000` `fetch()` endpoints removed. No phantom requests or failed console logs when running offline in `file:///` mode.
- **No Monolithic Python f-Strings**: HTML/CSS/JS is authored once in `master_template.html`, not string-concatenated in Python scripts.

---

## 4. The Renderer Engine: `tools/board_renderer.py`

[`tools/board_renderer.py`](file:///c:/Dev/TheKlangSuite/tools/board_renderer.py) provides the single entrypoint for board generation:

```python
from tools.models import BoardSchema
from tools.board_renderer import render_board

schema = BoardSchema(
    board_id="morning_mail",
    title="N'kai Morning Mail Inbox & Triage Lab",
    subtitle="Vault Inbox Ledger",
    cards=items,
)

out_file = render_board(schema, Path(".agents/sidecar/morning_mail_inbox.html"))
```

The renderer:
1. Ingests the `BoardSchema` object.
2. Resolves `master_template.html` dynamically.
3. Renders card DOM elements and filter rails.
4. Injects template variables.
5. Emits UTF-8 encoded, self-contained HTML.
6. Automatically mirrors output to the active Antigravity extension panel (`%USERPROFILE%/.gemini/config/plugins/nkai/sidecars/panel/index.html`) and the artifact directory.

---

## 5. Verification & Testing

Validated by automated test suite [`tools/tests/test_sandbox_board.py`](file:///c:/Dev/TheKlangSuite/tools/tests/test_sandbox_board.py):
1. `test_fixture_schema_valid`: Validates fixture data integrity.
2. `test_sandbox_zero_mutation`: Asserts zero mutations to real queue files or vault notes.
3. `test_sandbox_html_content`: Asserts sandbox banner, `[SBX]` title, and deterministic IDs.
4. `test_production_regression`: Asserts production mode stability without sandbox leakage.
5. `test_board_schema_roundtrip`: Asserts end-to-end `BoardSchema` serialization and `render_board()` output.

All generated HTML passes [`tools/audit_sidecar.py`](file:///c:/Dev/TheKlangSuite/tools/audit_sidecar.py) with 0 errors.
