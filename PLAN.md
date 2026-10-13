# Flight 22: The Director's Walkie-Talkie (Mobile Intercom, iPhone File Uploads & Switcher Hub)

**TARGET:** TOOLING_HARNESS  
**Role:** New Klang City Architecture & Web Engineering  
**Recommended Model:** Gemini 3.8 Flash (Thinking: High) — Tier 2 (Balanced Engineering & UI)  
> **Status:** COMPLETED  

---

### Executive Overview & Architectural Dialectic
Following the 2-Round Heavy PCDA Crucible (`.agents/pipeline/plans/crucible_walkie_talkie_plan.json`), we are unifying the mobile companion experience:
1. **The Board Switcher is the Universal Walkie-Talkie**: Rather than polluting individual card surfaces or creating dual-input confusion, the universal Board Switcher modal across all boards is elevated into the **Director's Walkie-Talkie HUD** (Live agent state, current flight progress, scratchpad byte counter, 1-tap `[ Proceed ]` and `[ Read SP ]` action chips, followed by the lightning-fast board switching grid).
2. **iPhone File & Photo Uploads in the Chat Board**: Equipping the dedicated Chat Board (`chat_board.html`) with native mobile camera, photo library, and document attachments (`<input type="file" multiple>`), streaming directly to `tools/nkai_serve.py` via `POST /api/chat/upload`. Uploaded files are saved to `.agents/pipeline/uploads/` and mirrored to Brain artifacts `.user_uploaded/`, with markdown references automatically injected into the chat prompt turn.
3. **iOS Safe Area Inset Protection**: Hardening the bottom sticky prompt dock with `env(safe-area-inset-bottom)` to guarantee the iPhone home indicator bar never obstructs buttons.

---

### Phase 1: Backend File Upload & Multi-Part Ingest (tools/nkai_serve.py)
- [x] Implement `POST /api/chat/upload` endpoint in `tools/nkai_serve.py` supporting `multipart/form-data` and JSON base64 payloads.
- [x] Save uploaded files to `.agents/pipeline/uploads/<timestamp>_<safe_filename>`.
- [x] Mirror uploaded files to Brain artifacts `.user_uploaded/<timestamp>_<safe_filename>` for direct IDE and subagent visibility.
- [x] Return JSON receipt: `{"ok": true, "filename": str, "abs_path": str, "rel_path": str, "size": int, "mime_type": str}`.
- [x] Add unit test assertions in `tools/tests/test_chat_engine.py` for `/api/chat/upload`.

### Phase 2: Chat Board UI File Attachment & Media Staging (tools/chat_engine.py)
- [x] Add paperclip button `[ 📎 ]` to the sticky prompt bar in `generate_chat_board_html()`.
- [x] Add hidden `<input type="file" id="chat-file-input" multiple accept="image/*,video/*,audio/*,.pdf,.txt,.md,.json,.zip,.cpp,.h">`.
- [x] Implement client-side staged files tray (`#staged-files-tray`) displaying chips with image thumbnail or file icon, filename, size, and remove `✕` button.
- [x] Update `sendPrompt()` JavaScript: upload staged files via `/api/chat/upload`, append markdown links/embeds to prompt, and dispatch prompt turn.
- [x] Update transcript turn renderer to render inline image cards for `![...](file:///...)` and clickable file pills for `[📎 ...](file:///...)`.
- [x] Apply `padding-bottom: max(14px, env(safe-area-inset-bottom))` to prompt bar for iOS safe-area compliance.

### Phase 3: Universal Board Switcher Walkie-Talkie Upgrade (master_template.html & boards)
- [x] Update `c:\Dev\nkai\core\master_template.html`:
  - [x] Add top Walkie-Talkie HUD header to `#board-switcher-modal`:
    - Live Agent State pill (`[ 🟢 IDLE ]` / `[ 🔨 WORKING ]`).
    - Scratchpad status counter (`SP: <N> KB`).
    - 1-tap quick action buttons: `[ ⚡ Proceed ]` · `[ 📝 Read SP ]` · `[ 💬 Chat ]`.
  - [x] Bind real-time polling to `/api/bulletin` and `/api/scratchpad/status`.
- [x] Run `python tools/sync_templates.py sync --force` to propagate to all bundle templates.
- [x] Rebuild and mirror all boards (`morning_mail`, `sandbox`, `runway`, `tackle_first`, `chat`).

### Phase 4: Regression Gauntlet, Audits & Web QA
- [x] Run `tools/audit_sidecar.py` across all generated HTML boards (zero issues).
- [x] Run `tools/verify_utf8.py --strict` across all scripts (100% compliant).
- [x] Execute `python -m unittest discover tools/tests/` (100% pass rate).
- [x] Verify live endpoints on `http://127.0.0.1:3030`.
- [x] Commit all changes cleanly across `TheKlangSuite` and `nkai`.
