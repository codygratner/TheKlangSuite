# 🎨 Flight 14: Multi-Board Hub & Session Manager (`rj-05`)

> **ID:** plan-011  
> **TARGET:** TOOLING_HARNESS (New Klang City) 🚀  
> **Topic Target:** Mailbag Card `rj-05` / BACKLOG Section 15 ("Multi-Board Hub & Session Manager")  
> **Status:** COMPLETED ✅  
> **Priority:** P0 (Top Priority)  
> **Target Branch:** `0.4.0-dev` (TheKlangSuite) / `main` (nkai)  
> **Recommended Setting:** Gemini 3.8 Flash (Thinking: High) — 🟢 SUSTAINABLE  
> **Source Notes:** [`C:\Dev\TheKlangVault\Inbox\Archive\Nkai Multi-Board Hub and Archive Catalog.md`](file:///C:/Dev/TheKlangVault/Inbox/Archive/Nkai%20Multi-Board%20Hub%20and%20Archive%20Catalog.md) + [`archived nkai boards probably need their own space.md`](file:///C:/Dev/TheKlangVault/Inbox/Archive/archived%20nkai%20boards%20probably%20need%20their%20own%20space.md)  
> **Prerequisites:** Flight 15 (Unified Master Template & Schema-Driven Boards) ✅  
> **Dependents:** Flight 19 (Per-Card Response Drawer) ⏳, Flight 21 (Checkpoint Resumption Engine & Opus Deep Audit) ⏳  

---

## 🎯 Architectural Intent & Storyline

Users navigating between tasks previously experienced two major friction points:
1. **Aggressive Webview Caching**: The Antigravity IDE sidecar panel (Chromium/Electron) aggressively cached static GET responses, causing the panel to display stale or outdated board snapshots without explicit `no-cache` headers.
2. **Missing Board Switcher & Archive Hub**: There was no interactive directory to switch between multiple active boards (e.g., Morning Mail Inbox vs. QA Sandbox Board) or reopen past, read-only archived research sessions without manually editing plugin entrypoint files.

**Flight 14** delivers the **Multi-Board Hub & Session Manager**:
- **Cache-Busting Backend**: `sidecars/panel/main.mjs` sets strict `Cache-Control: no-store, no-cache, must-revalidate` and `Pragma: no-cache` headers across all HTML/CSS/JS endpoints, plus query parameter routing (`/?board=<id>`).
- **Interactive Header Switcher**: A persistent board selector button in the Top Tier Header of `master_template.html` (`[ 📋 Current Board Title ▾ ]`) that opens an accessible modal/popover with two distinct sections:
  - 🟢 **Active Boards**: Live operational boards (`morning_mail`, `sandbox_mail`).
  - 📁 **Archived Boards**: Historical research and audit sessions preserved read-only.
- **Dual-Mode Registry Support**:
  - *Extension/Server Mode*: Fetches `/api/boards` dynamically from the sidecar server.
  - *File/Offline Mode*: Reads pre-baked inline registry manifest interpolated by `tools/board_renderer.py`.
- **Anti-Over-Engineering Guardrail**: Zero external npm dependencies. Pure vanilla ES6, semantic HTML5, and CSS glassmorphism.

---

## 📋 Phase Breakdown

### Phase 1: Board Registry Data Model & Storage
- [x] In `tools/models.py`, define Pydantic models for board registry:
  - `BoardMetadata`: `id`, `title`, `description`, `category` (`"active"` | `"archived"`), `file_path`, `environment` (`"PRODUCTION"` | `"SANDBOX"`), `card_count`, `updated_at`.
  - `BoardRegistry`: `version`, `default_board_id`, `boards: List[BoardMetadata]`.
- [x] Initialize canonical registry at `.agents/pipeline/boards/registry.json`:
  - Registers `morning_mail` (Active, Production, `.agents/sidecar/morning_mail_inbox.html`).
  - Registers `sandbox_mail` (Active, Sandbox, `.agents/sidecar/sandbox_mail_inbox.html`).
  - Registers `system_sanity_audit` (Archived, Research, `.agents/sidecar/nkai_deep_research.html`).
- [x] Author CLI helper `tools/board_registry.py` with `list`, `register`, and `rescan` commands.
- [x] Mirror models and registry tool to `c:\Dev\nkai\tools/` and `c:\Dev\nkai\bundle\tools/`.

### Phase 2: Sidecar Backend Cache Invalidation & Routing (`main.mjs`)
- [x] In `c:\Dev\nkai\sidecars\panel\main.mjs`:
  - Add explicit anti-cache HTTP response headers:
    - `Cache-Control: no-store, no-cache, must-revalidate, max-age=0`
    - `Pragma: no-cache`
    - `Expires: 0`
  - Implement `/api/boards` endpoint returning the parsed board registry.
  - Enhance `app.page('/', ...)` to parse query parameter `?board=<id>`:
    - If `board` matches a registered board, serve its target HTML file with cache-busting headers.
    - If no parameter or unknown, serve default active board (`morning_mail_inbox.html`).
- [x] Update `sidecars/panel/index.html` to load default board cleanly.

### Phase 3: Header Board Switcher UI & Controller (`master_template.html` & `sidecar.js`)
- [x] In `c:\Dev\nkai\core\master_template.html` (and `.agents/sidecar/master_template.html`):
  - In `.header-tier-top`, replace static title with interactive Board Switcher trigger:
    ```html
    <button id="btn-board-switcher" class="board-switcher-btn" onclick="toggleBoardSwitcherModal()">
      <span class="board-icon">📋</span>
      <span class="board-title">{{BOARD_TITLE}}</span>
      <span class="board-caret">▾</span>
    </button>
    ```
  - Add accessible Modal / Popover dialog `#board-switcher-modal`:
    - Search/filter input for rapid board location.
    - Group 1: 🟢 **Active Boards** (with status badges & card counters).
    - Group 2: 📁 **Archived Sessions** (with timestamp & read-only tags).
    - Close button `[ ✕ ]` and keyboard shortcut (`Escape` to close).
- [x] In `c:\Dev\nkai\core\sidecar.css`:
  - Style `.board-switcher-btn` with subtle hover glow and border accents.
  - Style `#board-switcher-modal`, `.board-group`, `.board-entry`, and `.board-entry:hover`.
- [x] In `c:\Dev\nkai\core\sidecar.js`:
  - Implement `toggleBoardSwitcherModal()`, `loadBoardList()`, and `switchBoard(boardId)`.
  - Handle extension mode (`window.location.search = '?board=' + boardId + '&_t=' + Date.now()`) vs offline file fallback.
- [x] Update `tools/board_renderer.py` to embed board registry JSON into template for offline support.

### Phase 4: Automated Verification Gauntlet
- [x] Author `tools/tests/test_board_hub.py`:
  - Asserts `BoardRegistry` schema validation and serialization.
  - Asserts `main.mjs` contains proper cache-busting headers.
  - Asserts `master_template.html` renders board switcher modal with deterministic IDs.
- [x] Run `python -m unittest tools/tests/test_board_hub.py` and `tools/tests/test_sandbox_board.py` (100% PASS).
- [x] Run `python tools/audit_sidecar.py` across updated templates (0 errors, PERFECT PASS).
- [x] Run `python tools/verify_utf8.py --all --strict` (assert 100% compliance).
- [x] Run `python tools/verify_context_clues.py` (assert 100% pass).
- [x] Reinstall plugin using `python tools/install_nkai_plugin.py` and assert validation pass.

### Phase 5: Design Spec, Documentation & Multi-Repo Commits
- [x] Author design spec `docs/specs/nkai_multi_board_hub_and_session_manager.md`.
- [x] Register in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.
- [x] Update `docs/GLOSSARY.md` (§21: Multi-Board Hub & Session Manager) and `docs/CHEAT_SHEET.md`.
- [x] Mark Flight 14 as `[x] COMPLETED` in `docs/BACKLOG.md` and Card `rj-05` in `.agents/pipeline/mailbag/rejigger.json`.
- [x] Commit changes cleanly across `TheKlangSuite` and `nkai`.
