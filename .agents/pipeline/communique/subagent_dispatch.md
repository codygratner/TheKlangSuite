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
- **TASK SLUG**: `card-response-drawer`
- **TARGET REPOSITORIES**: `TheKlangSuite`, `nkai`
- **TICKET ID**: `plan-013` (Flight 19 / `rj-08` split)

## 2. Scope & Instructions

Follow the active plan in `c:\Dev\TheKlangSuite\PLAN.md` and deliver Phases 1 through 5:

### Phase 1: Data Model Expansion (`tools/models.py`)
1. In `c:\Dev\TheKlangSuite\tools\models.py`:
   - Add fields to `MailItem`:
     - `agent_reply: Optional[str] = None`
     - `agent_reply_at: Optional[str] = None`
     - `has_reply: bool = False` (set `True` if `agent_reply` is present and non-empty)
   - Add `agent_reply: Optional[str] = None` to `RambleCard` and pass through to `to_mail_item()`.
2. Mirror `tools/models.py` to `c:\Dev\nkai\tools/models.py` and `nkai/bundle/tools/models.py`.

### Phase 2: Response Drawer Markup & Styling (`master_template.html` & `sidecar.css`)
1. In `c:\Dev\nkai\core\master_template.html` (and `.agents/sidecar/master_template.html`):
   - Render drawer container if card has `agent_reply`:
     `<div class="card-drawer-container" id="drawer-container-{{CARD_ID}}">`
       `<button class="card-drawer-toggle" id="btn-drawer-{{CARD_ID}}" onclick="toggleResponseDrawer('{{CARD_ID}}')" aria-expanded="false">`
         `<span class="drawer-toggle-icon">💬</span>`
         `<span class="drawer-toggle-label">Agent Assessment & Rationale</span>`
         `<span class="drawer-toggle-caret">▾</span>`
       `</button>`
       `<div class="card-drawer-content" id="drawer-{{CARD_ID}}" hidden>`
         `<div class="drawer-inner markdown-body">{{AGENT_REPLY_HTML}}</div>`
       `</div>`
     `</div>`
2. In `c:\Dev\nkai\core\sidecar.css`:
   - Style `.card-drawer-container`, `.card-drawer-toggle`, `.card-drawer-content`, `.drawer-inner`.
   - Subtle glassmorphic background, purple/indigo accent border, smooth max-height animation.
3. Mirror to `c:\Dev\TheKlangSuite\.agents\sidecar\master_template.html` and `nkai/bundle/templates/master_template.html`.

### Phase 3: Drawer Controller & CLI Reply Tooling (`sidecar.js` & `check_mail.py`)
1. In `c:\Dev\nkai\core\sidecar.js`:
   - Implement `toggleResponseDrawer(cardId)`: toggles `hidden` and `.is-open`, updates `aria-expanded`, plays haptic tick.
2. In `c:\Dev\TheKlangSuite\tools\board_renderer.py`:
   - Format `agent_reply` markdown to safe HTML using markdown renderer.
   - Conditionally inject response drawer markup per card.
3. In `c:\Dev\TheKlangSuite\tools\check_mail.py`:
   - Add CLI argument `--reply-card <id> --text "<markdown>"`:
     - Updates `agent_reply` and `agent_reply_at` on matching item in `mail_queue.json`.
     - Regenerates the board in <50ms.
4. In `assets/fixtures/sandbox_mail_queue.json`:
   - Add an `agent_reply` fixture to card `sbx-02` ("Synthetic Deep Research Specimen").

### Phase 4: Automated Verification Gauntlet
1. Author `c:\Dev\TheKlangSuite\tools\tests\test_response_drawer.py`:
   - Test `MailItem` serialization with `agent_reply`.
   - Test drawer rendering in `board_renderer.py`.
   - Test `--reply-card` CLI updates `mail_queue.json` and regenerates HTML.
2. Run test suites:
   `python -m unittest tools/tests/test_response_drawer.py tools/tests/test_intake_v2.py tools/tests/test_board_hub.py tools/tests/test_sandbox_board.py`
3. Audit templates:
   `python tools/audit_sidecar.py .agents/sidecar/sandbox_mail_inbox.html`
   `python tools/audit_sidecar.py .agents/sidecar/morning_mail_inbox.html`
   `python tools/audit_sidecar.py .agents/sidecar/master_template.html`
4. Run `python tools/verify_utf8.py --all --strict` (assert 100% compliance).
5. Run `python tools/verify_context_clues.py` (assert 100% pass).
6. Run `python tools/install_nkai_plugin.py`.

### Phase 5: Design Spec, Documentation & Multi-Repo Commits
1. Author `docs/specs/nkai_per_card_response_drawer.md`.
2. Register in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.
3. Update `docs/GLOSSARY.md` (§23: Per-Card Response Drawer) and `docs/CHEAT_SHEET.md`.
4. Update `docs/BACKLOG.md` (mark Flight 19 complete) and Card `rj-08` in `rejigger.json`.
5. Check all boxes `[x]` in `PLAN.md` and set `Status: COMPLETED ✅`.
6. Append execution receipt to `subagent_receipt.md` and set dispatch `STATUS: COMPLETED`.
7. Commit `TheKlangSuite`: `feat(drawer): implement Flight 19 per-card response drawer`.
8. Commit `nkai`: `feat(drawer): sync response drawer markup, styles, and models`.
