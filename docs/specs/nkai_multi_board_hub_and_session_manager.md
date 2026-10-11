# N'kai Multi-Board Hub & Session Manager (Flight 14 / `rj-05`)

## 1. Architectural Intent & Storyline

During daily development across the Klang ecosystem, two major friction points emerged when using the N'kai Antigravity sidecar and webviews:
1. **Aggressive Webview Memory Caching**:
   The Antigravity IDE sidecar panel (powered by Electron/Chromium) aggressively cached static GET responses. Without explicit, strict HTTP response headers, updating or switching boards frequently served stale in-memory cached snapshots of prior sessions.
2. **Missing Board Switcher & Archive Hub**:
   As N'kai evolved from a single inbox triage view into multiple specialized surfaces (the daily Morning Mailbag, the QA Sandbox Board, and historical deep research sessions like `system_sanity_audit`), users had no native, interactive directory to switch between active boards or reopen past research sessions without manually editing plugin manifests or file URLs.

Source notes:
- [`Nkai Multi-Board Hub and Archive Catalog.md`](file:///C:/Dev/TheKlangVault/Inbox/Archive/Nkai%20Multi-Board%20Hub%20and%20Archive%20Catalog.md)
- [`archived nkai boards probably need their own space.md`](file:///C:/Dev/TheKlangVault/Inbox/Archive/archived%20nkai%20boards%20probably%20need%20their%20own%20space.md)

**Flight 14** synthesizes these requirements into the **Multi-Board Hub & Session Manager**:
- **Cache-Busting Backend Engine**: `sidecars/panel/main.mjs` sets mandatory `Cache-Control: no-store, no-cache, must-revalidate, max-age=0`, `Pragma: no-cache`, and `Expires: 0` headers across all endpoints, plus query parameter routing (`/?board=<id>&_t=<timestamp>`).
- **Interactive Header Switcher Button**: In the Top-Tier Header of `master_template.html`, the static title is replaced with an accessible trigger button (`#btn-board-switcher`).
- **Accessible Multi-Board Modal**: Opening the switcher reveals `#board-switcher-modal` with search filtering, separating live 🟢 **Active Boards** from read-only 📁 **Archived Sessions**.
- **Dual-Mode Offline & Server Resiliency**: In Antigravity Extension mode, queries `/api/boards` dynamically; in offline `file:///` mode, falls back gracefully to a pre-baked JSON manifest embedded in the HTML.

```
┌────────────────────────────────────────────────────────────────────────┐
│                   MULTI-BOARD HUB & SESSION MANAGER                    │
├────────────────────┬────────────────────┬──────────────────────────────┤
│ COMPONENT          │ IMPLEMENTATION     │ CAPABILITY                   │
├────────────────────┼────────────────────┼──────────────────────────────┤
│ Board Registry     │ tools/models.py    │ Pydantic BoardRegistry model │
│                    │ registry.json      │ Canonical boards catalog     │
├────────────────────┼────────────────────┼──────────────────────────────┤
│ Sidecar Backend    │ main.mjs           │ Anti-cache HTTP headers      │
│                    │                    │ /api/boards & ?board=<id>    │
├────────────────────┼────────────────────┼──────────────────────────────┤
│ Header Switcher    │ master_template    │ #btn-board-switcher trigger  │
│ & Modal Dialog     │ sidecar.css        │ #board-switcher-modal dialog │
├────────────────────┼────────────────────┼──────────────────────────────┤
│ Controller Logic   │ sidecar.js         │ Dynamic search & navigation  │
│                    │                    │ Dual-mode extension/file     │
└────────────────────┴────────────────────┴──────────────────────────────┘
```

---

## 2. Core Architecture

### 2.1 The Board Registry Data Model (`tools/models.py`)

The board registry is formalized as Pydantic models ensuring deterministic validation and type safety:

```python
class BoardMetadata(BaseModel):
    id: str           # Deterministic board slug, e.g. morning_mail, sandbox_mail
    title: str        # Human-readable display title
    description: str  # Purpose summary
    category: str     # "active" | "archived"
    file_path: str    # Canonical relative path to HTML board file
    environment: str  # "PRODUCTION" | "SANDBOX"
    card_count: int   # Number of cards or topics on board
    updated_at: str   # ISO timestamp of last update

class BoardRegistry(BaseModel):
    version: str = "1.0.0"
    default_board_id: str = "morning_mail"
    boards: List[BoardMetadata] = Field(default_factory=list)

    def get_board(self, board_id: str) -> Optional[BoardMetadata]: ...
    def active_boards(self) -> List[BoardMetadata]: ...
    def archived_boards(self) -> List[BoardMetadata]: ...
```

The canonical catalog is stored at `.agents/pipeline/boards/registry.json` and mirrored to `c:/Dev/nkai/boards/registry.json`.

---

### 2.2 Strict Cache Invalidation & Routing (`sidecars/panel/main.mjs`)

To eradicate stale Chromium memory caching in the sidecar webview, `main.mjs` injects anti-cache headers across all routes:

```javascript
export const ANTI_CACHE_HEADERS = {
  'Cache-Control': 'no-store, no-cache, must-revalidate, max-age=0',
  'Pragma': 'no-cache',
  'Expires': '0'
};
```

1. **`GET /api/boards`**: Reads `registry.json` and returns `{ ok: true, registry: ... }` with `ANTI_CACHE_HEADERS`.
2. **`app.page('/', (req) => ...)`**: Parses `?board=<id>` query parameter, resolves the corresponding HTML board file, and streams it with zero caching.
3. **Static Asset Responses**: CSS, JS, JSON schemas, and SVG icons return `ANTI_CACHE_HEADERS` via `sidecar_sdk.Response`.

---

### 2.3 Interactive Header Switcher & Accessible Modal (`master_template.html`)

In `master_template.html`, the static brand title in `.header-tier-top` is replaced with:

```html
<button id="btn-board-switcher" class="board-switcher-btn" onclick="toggleBoardSwitcherModal()" title="Switch active board or view archives">
  <span class="board-icon">📋</span>
  <span class="board-title" id="current-board-title">{{BOARD_TITLE}}</span>
  <span class="board-caret">▾</span>
</button>
```

Clicking triggers `#board-switcher-modal`:
- **Keyboard Navigation**: Pressing <kbd>Esc</kbd> closes the modal.
- **Instant Search**: `#board-search-input` filters active and archived boards in real-time as the user types.
- **Categorized Sections**:
  - 🟢 **Active Operational Boards**: Live boards with card counters (`morning_mail`, `sandbox_mail`).
  - 📁 **Archived Research & Audit Sessions**: Historical deep research sessions (`system_sanity_audit`).
- **State Preservation**: Highlights the currently active board with a `● CURRENT` badge.

---

### 2.4 Dual-Mode Controller Logic (`sidecar.js`)

`sidecar.js` implements seamless operation across environments:
1. **Server / Antigravity Extension Mode**:
   Navigates to `/?board=${encodeURIComponent(boardId)}&_t=${Date.now()}`.
2. **Offline File Mode**:
   Navigates to the relative filename with cache buster token: `${targetFile}?_t=${Date.now()}`.
3. **Embedded JSON Fallback**:
   `board_renderer.py` inlines `registry.json` into `<script id="nkai-board-registry-data" type="application/json">`, enabling instant offline modal rendering even without an active HTTP server.

---

## 3. CLI Registry Management (`tools/board_registry.py`)

A standalone CLI tool provides registry operations:
- `python tools/board_registry.py list`: Displays a formatted table of all registered boards, categories, environments, and card counts.
- `python tools/board_registry.py register <id> <title> <file_path> [--category active] [--environment PRODUCTION]`: Upserts a board entry.
- `python tools/board_registry.py rescan`: Discovers HTML boards in `.agents/sidecar/` and automatically updates card tallies.

---

## 4. Verification Gauntlet & Quality Gates

The Multi-Board Hub complies with all strict ecosystem guardrails:
1. **Automated Unit Tests (`tools/tests/test_board_hub.py`)**: Asserts schema validation, anti-cache headers in `main.mjs`, and modal elements in `master_template.html`.
2. **Sandbox Regression (`tools/tests/test_sandbox_board.py`)**: Asserts zero-mutation invariant and 100% test pass.
3. **Web QA Linter (`tools/audit_sidecar.py`)**: Perfect 0-error, 0-warning pass across `sandbox_mail_inbox.html`, `morning_mail_inbox.html`, and `master_template.html`.
4. **Universal UTF-8 Compliance (`tools/verify_utf8.py --all --strict`)**: 100% compliant across all Python scripts.
5. **Context Clues Integrity (`tools/verify_context_clues.py`)**: 100% link resolution.
