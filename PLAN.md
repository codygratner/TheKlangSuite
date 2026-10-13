# Implementation Plan: Flight 21 (Physical Workshop Cutting Mat, Intercom HUD & Zero-Token Scratchpad Sentinel)

> **Task Name:** `nkai-workshop-intercom-scratchpad-sentinel`
> **Target Pipeline:** `TARGET: TOOLING_HARNESS`
> **Active Milestone:** `N'kai v0.3.6 / Ergonomics & Dialectic Suite`
> **Recommended Model Tier:** `Tier 2 (Gemini 3.8 Flash High)`
> **Status:** `COMPLETED ✅`

---

## 1. Flight Overview & Architecture
This flight transforms the N'kai companion ecosystem into a physical desktop analog, pairs it with a living two-way intercom HUD, and completely eliminates friction in mobile capturing and reading note workflows:
1. **The Physical Workshop Cutting Mat (`🛠️ Active Workshop`)** — Implements the physical desktop analog where the In-Tray (`morning_mail`) is a rapid-fire conveyor belt to zero, while the Cutting Mat (`workshop`) locks exactly ONE card on stage for deep Option A/B/C research. Exiting the cutting mat strictly requires a physical filing decision (`[ 📥 Backlog ]`, `[ 🗂️ Tabled ]`, `[ 🪦 Graveyard ]`).
2. **Card #0: Pinned Agent Intercom HUD Banner** — An expandable status banner docked below the header displaying the latest agent briefing, flight completion notes, pending human approval counts, and live Scratchpad urgency telemetry.
3. **Zero-Token Scratchpad Sentinel (`tools/nkai_serve.py`)** — Sub-0.1ms filesystem watcher tracking `C:\_\Vault\Inbox\SCRATCHPAD.md` line count and byte size for 0 LLM tokens, providing live urgency pills (`clean` ➔ `growing` ➔ `urgent`) and conditional delta ingestion.
4. **Mobile Touch Ergonomics & Native Keyboard Sheet** — Collapsible cards (saving 60% vertical space on mobile), per-card state refresh, and a native slide-out `<textarea>` sheet that reliably summons iOS Safari / iPad virtual keyboards without Monaco or webview focus traps.
5. **2-Round Heavy PCDA Automation Harness (`tools/heavy_pcda.py`)** — Codified CLI runner implementing the 2-round online dialectic crucible.

---

## 2. Phase-by-Phase Execution Checklist

### Phase 1: Zero-Token Scratchpad Sentinel & API Suite (`nkai_serve.py`)
- [x] 1.1. In `tools/nkai_serve.py`, implement `ScratchpadSentinel`:
  - `inspect_scratchpad()`: Performs `os.stat()` on `C:\_\Vault\Inbox\SCRATCHPAD.md` (fallback to `docs/inbox/SCRATCHPAD.md`).
  - Implements `safe_file_stat()` with 3-retry 50ms exponential backoff to eliminate Windows `PermissionError [WinError 32]` during background Obsidian Sync.
  - Calculates line count, byte size, hash, and urgency level (`clean` < 25 lines, `growing` 25-80 lines, `urgent` > 80 lines).
  - Compute cost: `<0.1ms` CPU, 0 LLM tokens.
- [x] 1.2. Implement REST endpoints in `nkai_serve.py`:
  - `GET /api/scratchpad/status`: Returns `{ exists, lines, bytes, mtime, urgency, has_unread }`.
  - `GET /api/scratchpad/content`: Returns raw text payload for the slide-out sheet.
  - `POST /api/scratchpad/append`: Receives `{ text }`, appends `- [<HH:MM>] <text>` to `SCRATCHPAD.md`.
  - `POST /api/scratchpad/triage`: Records current mtime as read, triggers agent ingest of delta lines.
- [x] 1.3. Update `/api/quick-note` to append to `SCRATCHPAD.md` by default (with optional `{ target: "inbox" }` override).
- [x] 1.4. Author unit test suite `tools/tests/test_scratchpad_sentinel.py` (assert 100% pass for status, delta detection, and zero-token behavior).

### Phase 2: Card #0 Pinned Agent Intercom HUD Banner
- [x] 2.1. In `.agents/pipeline/mailbag/`, define `agent_bulletin.json` schema (`briefing_title`, `briefing_text`, `status_badge`, `updated_at`, `pending_decisions_count`, `last_flight`).
- [x] 2.2. In `c:\Dev\nkai\core\master_template.html`:
  - Insert `#agent-intercom-hud` container docked immediately beneath `.header-tier-bottom`.
  - Structure as a compact ticker (28px height) with glowing Emerald/Cyan border, displaying:
    - `🤖 Agent Briefing: <briefing_title>`
    - Dynamic Scratchpad Urgency Pill: `<span id="hud-sp-pill" class="pill-sp">📝 SP: N lines</span>`
    - Quick Action Buttons: `[ 👁️ Peek SP ]` · `[ 📖 Ingest ]` · `[ 💬 Expand ]`
  - Clicking `[ 💬 Expand ]` toggles full markdown briefing accordion.
- [x] 2.3. In `c:\Dev\nkai\core\sidecar.css` & `sidecar.js`:
  - Style `#agent-intercom-hud` with responsive styling and dynamic urgency color shifts (green/amber/coral).
  - Implement polling/fetch for `/api/scratchpad/status` on board load and window focus.
- [x] 2.4. In `tools/flush_memory.py` and `tools/check_mail.py`:
  - Auto-update `agent_bulletin.json` whenever memory flush, task finish, or mail reply runs.

### Phase 3: Mobile Ergonomics & Native Touch Input
- [x] 3.1. In `c:\Dev\nkai\core\master_template.html` & `sidecar.css`:
  - Update card rendering for screens $\le$ 640px:
    - Default to collapsed view: Header, priority, pillar, title, 1-line excerpt, and action chips visible.
    - Card body/frontmatter/PCDA accordion collapsed by default.
    - Tap card header to toggle expand/collapse with CSS height animation.
- [x] 3.2. Add per-card refresh button `#btn-card-refresh-<id>`:
  - Re-fetches single card status from `/api/card/<id>` or local ledger without reloading entire DOM.
- [x] 3.3. Implement Native Mobile Scratchpad Sheet (`#scratchpad-drawer`):
  - DOM-Resident Architecture: `<div id="scratchpad-drawer">` kept permanently rendered in the DOM (hidden via CSS `transform: translateY(100%)`) to satisfy iOS WebKit user-gesture security policies.
  - Zero Monaco, zero hidden buffers; triggers `textarea.focus()` synchronously inside the user's tap handler, guaranteeing 100% reliable iOS Safari virtual keyboard invocation.
  - Buttons: `[ ➕ Add Thought ]`, `[ 📋 Copy All ]`, `[ ✕ Close ]`.

### Phase 4: The Physical Workshop Cutting Mat Metaphor
- [x] 4.1. In `c:\Dev\nkai\core\master_template.html`:
  - Add primary named mode switcher to bottom rail: `[ 📬 INBOX ]` (`#mode-inbox`) and `[ 🛠️ WORKSHOP ]` (`#mode-workshop`).
- [x] 4.2. In `c:\Dev\nkai\core\sidecar.js`:
  - Implement Workshop latching logic:
    - Clicking `[ 🔬 Research ]` or `[ ⚖️ PCDA ]` on an inbox card sets `activeWorkshopCardId = cardId`, persists to `localStorage['nkai_active_workshop_item']`, and switches active mode to `workshop`.
    - In `workshop` mode, all other cards are hidden; only the active workshop card is rendered in the center of the screen ("The Cutting Mat").
    - Swipe-to-dismiss is strictly disabled on the Cutting Mat.
- [x] 4.3. Implement Physical Exit Gate & Ambient Return Hatch:
  - Render prominent routing exit buttons on the Cutting Mat:
    `[ 📥 Route to Backlog ]` · `[ 🗂️ Move to Tabled ]` · `[ 🪦 Send to Graveyard ]`
  - Render ambient `[ ↩ In-Tray ]` button allowing user to step away to the In-Tray without archiving the active card (card remains safely pinned on the Cutting Mat until filed).
  - Clicking any exit button executes the ledger update, clears `activeWorkshopCardId`, and smoothly transitions the view back to `[ 📬 INBOX ]`.
- [x] 4.4. In `tools/board_renderer.py`:
  - Support rendering cards with a `[ 🛠️ Send to Workshop ]` action button when viewing Backlog or Tabled archives.

### Phase 5: 2-Round Heavy PCDA Automation Engine
- [x] 5.1. Author `tools/heavy_pcda.py`:
  - CLI command: `python tools/heavy_pcda.py "<topic_or_plan_path>" [--online]`
  - Enforces the 2-Round Dialectic Crucible:
    - Round 1: Divergence with Assumption Inversion (Proposer vs Critic).
    - Round 2: Cross-Critique with empirical grounding (`search_web`).
    - Final Synthesis: 80/20 pragmatic resolution.
  - Outputs structured Markdown and interactive HTML report to `.agents/sidecar/` and artifacts directory.
- [x] 5.2. Author unit test `tools/tests/test_heavy_pcda.py` asserting 2-round cap and proper schema output.

---

## 3. Verification & Acceptance Criteria
1. **Zero-Token Sentinel**: `GET /api/scratchpad/status` executes in <1ms and consumes 0 LLM tokens; urgency pills shift from green to amber at 25 lines and coral at 80 lines.
2. **Keyboard Reliability**: Mobile Scratchpad sheet opens with a real `<textarea>` that reliably triggers software keyboards on touch devices without focus traps.
3. **Physical Workshop Gate**: Clicking `[ 🔬 Research ]` latches the card to `#mode-workshop`; leaving workshop mode is strictly impossible without selecting `[Backlog]`, `[Tabled]`, or `[Graveyard]`.
4. **Card #0 HUD**: Docked intercom banner renders briefing and live scratchpad count; expands and collapses cleanly on tap.
5. **Unit Tests**: 100% test pass across all new test files (`python -m unittest discover tools/tests/`).
