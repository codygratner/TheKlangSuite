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
- **TASK SLUG**: `multi-board-hub`
- **TARGET REPOSITORIES**: `TheKlangSuite`, `nkai`
- **TICKET ID**: `plan-011` (Flight 14 / `rj-05`)

## 2. Scope & Instructions

Follow the active plan in `c:\Dev\TheKlangSuite\PLAN.md` and deliver Phases 1 through 5:

### Phase 1: Board Registry Data Model & Storage
1. In `c:\Dev\TheKlangSuite\tools\models.py`:
   - Add Pydantic models:
     - `BoardMetadata`: `id: str`, `title: str`, `description: str = ""`, `category: str = "active"` (`"active"` | `"archived"`), `file_path: str`, `environment: str = "PRODUCTION"` (`"PRODUCTION"` | `"SANDBOX"`), `card_count: int = 0`, `updated_at: str = Field(default_factory=...)`.
     - `BoardRegistry`: `version: str = "1.0.0"`, `default_board_id: str = "morning_mail"`, `boards: List[BoardMetadata] = Field(default_factory=list)`.
       - Method `get_board(board_id: str) -> Optional[BoardMetadata]`
       - Method `active_boards() -> List[BoardMetadata]`
       - Method `archived_boards() -> List[BoardMetadata]`
2. Author `c:\Dev\TheKlangSuite\.agents\pipeline\boards\registry.json`:
   - Initialize with:
     - `morning_mail`: Active, Production, `.agents/sidecar/morning_mail_inbox.html`, "Morning Mail Inbox & Triage Lab"
     - `sandbox_mail`: Active, Sandbox, `.agents/sidecar/sandbox_mail_inbox.html`, "Canonical QA Sandbox Test Board"
     - `system_sanity_audit`: Archived, Research, `.agents/sidecar/nkai_deep_research.html`, "System Sanity Audit & Deep Research"
3. Author `c:\Dev\TheKlangSuite\tools\board_registry.py`:
   - Python stdlib + Pydantic v2 + Universal UTF-8.
   - CLI commands: `list`, `register <id> <title> <file_path> [--category active] [--environment PRODUCTION]`, `rescan`.
4. Mirror `tools/models.py` and `tools/board_registry.py` to `c:\Dev\nkai\tools/` and `c:\Dev\nkai\bundle\tools/`.

### Phase 2: Sidecar Backend Cache Invalidation & Routing (`main.mjs`)
1. In `c:\Dev\nkai\sidecars\panel\main.mjs`:
   - In all HTTP route responses (`app.page` and `app.api`), inject strict anti-cache headers:
     `'Cache-Control': 'no-store, no-cache, must-revalidate, max-age=0'`
     `'Pragma': 'no-cache'`
     `'Expires': '0'`
   - Implement `GET /api/boards`:
     - Reads `.agents/pipeline/boards/registry.json` (or plugin bundle fallback `boards/registry.json`).
     - Returns JSON `{ ok: true, registry: ... }`.
   - Update `app.page('/', (req) => ...)`:
     - Support query parameter `?board=<id>` (if accessible via SDK or URL parsing).
     - If board param provided, resolve target board HTML file and return with no-cache headers.
     - Fall back to `index.html` / `morning_mail_inbox.html`.
2. Update `c:\Dev\nkai\plugin\sidecars\panel\main.mjs` and run `python tools/install_nkai_plugin.py`.

### Phase 3: Header Board Switcher UI & Controller (`master_template.html` & `sidecar.js`)
1. In `c:\Dev\nkai\core\master_template.html` (and `.agents/sidecar/master_template.html`):
   - Replace static brand title in `.header-tier-top` with an interactive button:
     `<button id="btn-board-switcher" class="board-switcher-btn" onclick="toggleBoardSwitcherModal()">`
       `<span class="board-icon">📋</span>`
       `<span class="board-title">{{BOARD_TITLE}}</span>`
       `<span class="board-caret">▾</span>`
     `</button>`
   - Add modal dialog `#board-switcher-modal`:
     - Filter input `#board-search-input`
     - `.board-modal-section` for 🟢 **Active Boards**
     - `.board-modal-section` for 📁 **Archived Boards**
     - Close button `[ ✕ ]`
2. In `c:\Dev\nkai\core\sidecar.css`:
   - Add styles for `.board-switcher-btn`, `#board-switcher-modal`, `.board-modal-content`, `.board-card-item`.
3. In `c:\Dev\nkai\core\sidecar.js`:
   - Implement `toggleBoardSwitcherModal()`, `filterBoardList()`, and `switchBoard(boardId)`.
   - When switching board in extension mode: navigate to `/?board=${boardId}&_t=${Date.now()}`.
   - When in file mode: navigate to relative file path if available, or update location.
4. Update `tools/board_renderer.py` to bake board registry JSON into template for offline support.
5. Mirror updated template and assets across `nkai/bundle/` and `TheKlangSuite/.agents/sidecar/`.

### Phase 4: Automated Verification Gauntlet
1. Author `c:\Dev\TheKlangSuite\tools\tests\test_board_hub.py`:
   - Tests `BoardRegistry` schema parsing and serialization.
   - Tests `main.mjs` contains anti-cache headers.
   - Tests `master_template.html` renders board switcher button and modal.
2. Run test suites:
   `python -m unittest tools/tests/test_board_hub.py`
   `python -m unittest tools/tests/test_sandbox_board.py`
3. Audit templates:
   `python tools/audit_sidecar.py .agents/sidecar/sandbox_mail_inbox.html`
   `python tools/audit_sidecar.py .agents/sidecar/morning_mail_inbox.html`
   `python tools/audit_sidecar.py .agents/sidecar/master_template.html`
4. Run `python tools/verify_utf8.py --all --strict` (assert 100% compliance).
5. Run `python tools/verify_context_clues.py` (assert 100% pass).
6. Run `python tools/install_nkai_plugin.py`.

### Phase 5: Design Spec, Documentation & Multi-Repo Commits
1. Author `c:\Dev\TheKlangSuite\docs\specs\nkai_multi_board_hub_and_session_manager.md`.
2. Register in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.
3. Update `docs/GLOSSARY.md` (§21: Multi-Board Hub & Session Manager) and `docs/CHEAT_SHEET.md`.
4. Update `docs/BACKLOG.md` (mark Flight 14 as complete) and Card `rj-05` in `.agents/pipeline/mailbag/rejigger.json`.
5. Check all boxes `[x]` in `PLAN.md` and set `Status: COMPLETED ✅`.
6. Append execution receipt to `.agents/pipeline/communique/subagent_receipt.md` and set dispatch `STATUS: COMPLETED`.
7. Commit `TheKlangSuite`: `feat(hub): implement Flight 14 multi-board hub and cache invalidation`.
8. Commit `nkai`: `feat(hub): add board switcher modal, cache-busting headers, and BoardRegistry`.
