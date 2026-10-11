# 🎨 Flight 19: Per-Card Response Drawer (`rj-08` split)

> **ID:** plan-013  
> **TARGET:** TOOLING_HARNESS (New Klang City → Flash Builder Subagent) 🚀  
> **Topic Target:** Mailbag Card `rj-08` (split) / BACKLOG Section 15 (Flight 19: Per-Card Response Drawer)  
> **Status:** COMPLETED ✅  
> **Priority:** P1 (Small, High Velocity)  
> **Target Branch:** `0.4.0-dev` (TheKlangSuite) / `main` (nkai)  
> **Recommended Setting:** Gemini 3.8 Flash (Thinking: High) — 🟢 SUSTAINABLE  
> **Source Note:** [`C:\Dev\TheKlangVault\Inbox\Archive\Fullscreen Sidecar.md`](file:///C:/Dev/TheKlangVault/Inbox/Archive/Fullscreen%20Sidecar.md) + [`Second Monitor and Other Uses for the Nkai Sidecar Integration.md`](file:///C:/Dev/TheKlangVault/Inbox/Archive/Second%20Monitor%20and%20Other%20Uses%20for%20the%20Nkai%20Sidecar%20Integration.md)  
> **Prerequisites:** Flight 15 (Master Template) ✅, Flight 14 (Multi-Board Hub) ✅, Flight 7 (Intake v2) ✅  
> **Dependents:** Flight 20 (Vault Graph & Naming Hygiene) ⏳, Flight 21 (Checkpoint Resumption Engine & Opus Deep Audit) ⏳  

---

## 🎯 Architectural Intent & Storyline

During live interactive triage, agents currently output multi-paragraph rationales, PCDA trade-offs, and next actions directly into the chat transcript. This creates two distinct problems:
1. **Chat Transcript Bloat**: Long-form triage text burns valuable context tokens and accelerates compaction amnesia.
2. **Context Fragmentation**: The human review surface is separated from the agent's explanation—the user looks at the board in the side pane, then has to scroll chat to read the agent's reasoning.

**Flight 19** introduces the **Per-Card Response Drawer**:
- **Rich In-Card Responses**: `MailItem` gains an optional `agent_reply: Optional[str] = None` field containing formatted Markdown.
- **Collapsible Glassmorphic Drawer**: When `agent_reply` is present, the card renders an accessible expandable drawer (`#drawer-<id>`) with smooth CSS height transitions, syntax-highlighted code blocks, and markdown formatting.
- **Chat-Minimal Pair Programming**: The agent writes full reasoning, PCDA matrices, and code snippets into `agent_reply` on the card, and emits only 1-2 lines in chat: *"Triaged to Backlog. Architectural breakdown committed to card drawer."*
- **Mobile-Ready Architecture**: When N'kai v0.3.0 mobile companion or browser tabs view the board, the complete decision log lives on the card itself without requiring chat connection.

---

## 📋 Phase Breakdown

### Phase 1: Data Model Expansion (`tools/models.py`)
- [x] In `tools/models.py`, add fields to `MailItem`:
  - `agent_reply: Optional[str] = None` (markdown body of agent response)
  - `agent_reply_at: Optional[str] = None` (ISO timestamp of reply)
  - `has_reply: bool = False` (computed property or validator setting `True` if `agent_reply` is non-empty)
- [x] In `tools/models.py`, update `RambleCard` with optional `agent_reply` passthrough to `to_mail_item()`.
- [x] Mirror models to `c:\Dev\nkai\tools/models.py` and `nkai/bundle/tools/models.py`.

### Phase 2: Response Drawer Markup & Styling (`master_template.html` & `sidecar.css`)
- [x] In `c:\Dev\nkai\core\master_template.html` (and `.agents/sidecar/master_template.html`):
  - On cards where `item.agent_reply` is present, render:
    ```html
    <div class="card-drawer-container" id="drawer-container-{{CARD_ID}}">
      <button class="card-drawer-toggle" id="btn-drawer-{{CARD_ID}}" onclick="toggleResponseDrawer('{{CARD_ID}}')" aria-expanded="false">
        <span class="drawer-toggle-icon">💬</span>
        <span class="drawer-toggle-label">Agent Assessment & Rationale</span>
        <span class="drawer-toggle-caret">▾</span>
      </button>
      <div class="card-drawer-content" id="drawer-{{CARD_ID}}" hidden>
        <div class="drawer-inner markdown-body">
          {{AGENT_REPLY_HTML}}
        </div>
      </div>
    </div>
    ```
- [x] In `c:\Dev\nkai\core\sidecar.css`:
  - Style `.card-drawer-container`, `.card-drawer-toggle`, and `.card-drawer-content`.
  - Add smooth transition, dark slate card contrast background, and subtle emerald/purple left border indicator.
  - Style `.drawer-inner` with clean typography, code block styling, and markdown spacing.

### Phase 3: Drawer Controller & CLI Reply Tooling (`sidecar.js` & `check_mail.py`)
- [x] In `c:\Dev\nkai\core\sidecar.js`:
  - Implement `toggleResponseDrawer(cardId)`:
    - Toggles `hidden` attribute and `.is-open` class.
    - Updates `aria-expanded` attribute.
    - Plays audio haptic tick (`playHaptic('tick')`).
- [x] In `tools/board_renderer.py`:
  - Convert `agent_reply` markdown to safe HTML using lightweight markdown-to-HTML parser.
  - Inject drawer markup conditionally.
- [x] In `tools/check_mail.py`:
  - Add CLI option `--reply-card <id> --text "<markdown>"` to record an agent reply on a card and regenerate the board in <50ms.
- [x] Add fixture with an `agent_reply` to `assets/fixtures/sandbox_mail_queue.json` (`sbx-02`).

### Phase 4: Automated Verification Gauntlet
- [x] Author `tools/tests/test_response_drawer.py`:
  - Asserts `MailItem` serializes and validates `agent_reply` and `agent_reply_at`.
  - Asserts `board_renderer.py` renders drawer container, toggle button, and content with deterministic IDs (`btn-drawer-<id>`, `drawer-<id>`).
  - Asserts `--reply-card` CLI updates the ledger and regenerates HTML cleanly.
- [x] Run test suites:
  - `python -m unittest tools/tests/test_response_drawer.py tools/tests/test_intake_v2.py tools/tests/test_board_hub.py tools/tests/test_sandbox_board.py` (assert 100% PASS).
- [x] Run Web QA linter `tools/audit_sidecar.py` across updated templates (assert 0 errors).
- [x] Run `python tools/verify_utf8.py --all --strict` (assert 100% compliance).
- [x] Run `python tools/verify_context_clues.py` (assert 100% pass).
- [x] Run `python tools/install_nkai_plugin.py` and verify extension assets.

### Phase 5: Design Spec, Documentation & Multi-Repo Commits
- [x] Author design spec `docs/specs/nkai_per_card_response_drawer.md`.
- [x] Register in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.
- [x] Update `docs/GLOSSARY.md` (§23: Per-Card Response Drawer) and `docs/CHEAT_SHEET.md`.
- [x] Mark Flight 19 complete in `docs/BACKLOG.md` and Card `rj-08` in `.agents/pipeline/mailbag/rejigger.json`.
- [x] Check off all `[x]` in `PLAN.md` and set `Status: COMPLETED ✅`.
- [x] Append execution receipt to `subagent_receipt.md` and set dispatch `STATUS: COMPLETED`.
- [x] Commit cleanly across `TheKlangSuite` and `nkai`.
