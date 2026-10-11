# 🐸 Mayor Toad's Sidecar Embezzlement: N'kai v0.2.0 Native Extension & Zero-Copy IPC

> **ID:** plan-002  
> **TARGET:** TOOLING_HARNESS (New Klang City) 🚀  
> **Topic Target:** N'kai v0.2.0 (Official Antigravity UI Extension & Zero-Copy Native IPC)  
> **Status:** COMPLETED ✅  
> **Priority:** P1  
> **Target Branch:** `0.4.0-dev`  
> **Recommended Setting:** Gemini 3.8 Flash (Thinking: High) — 🟢 SUSTAINABLE  
> **Source Agreements:** `docs/BACKLOG.md` (Section 13) & System Sanity Audit Topics 6.5, 6.17, 6.18  
> **Mobile Deferral Invariant:** Mobile Touch Companion & LAN Wi-Fi Pairing (v0.3.0) are formally deferred to the backlog to protect the 1-Week Bounded Tooling Box and unblock the return to native C++ DSP.

---

## 🎯 Architectural Intent & Storyline

While the factory floor in Klang Industries prepares to forge DSP filter shells, Mayor Toad has quietly siphoned off municipal tooling resources to turn N'kai into an official Google Antigravity UI Extension:
1. **Official Antigravity Plugin Packaging**:
   - Packages N'kai under `%USERPROFILE%\.gemini\config\plugins\nkai\` with `plugin.json`, `assets/logo.svg`, and `sidecars/panel/sidecar.json`.
   - Uses the built-in Sidecar SDK (`sidecar_sdk`) running a supervised Node.js backend (`main.mjs`).
2. **True Zero-Copy Native IPC**:
   - Upgrades `dispatchNkaiAction()` to use `window.sidecar.agent.sendMessage()` when in extension mode.
   - Clicking `[ ✅ APPROVE ]`, `[ 🔬 Research ]`, or `[ ⚖️ PCDA ]` sends actionable prompts directly to the active conversation loop without requiring manual copy-paste.
   - Seamlessly maintains the read-only click-to-highlight fallback when running in standalone offline `file:///` mode or external browsers.
3. **Native Audio Alerts**:
   - Triggers clean, physical audio haptics (30ms mechanical click and major triad chime) in the side panel.

---

## 📋 Phase Breakdown

### Phase 1: Native Plugin Manifest & Sidecar Infrastructure
- [x] Create `c:\Dev\nkai\plugin.json`:
  - Identifier `"name": "nkai"`, description, `"logo": "assets/logo.svg"`.
- [x] Create `c:\Dev\nkai\assets\logo.svg`:
  - 64x64 vector logo with dark slate rounded square background and glowing emerald N'kai glyph.
- [x] Create `c:\Dev\nkai\sidecars\panel\sidecar.json`:
  - Process command `"node"`, args `["main.mjs"]`, restart policy `"always"`.
  - `"has_web_ui": true`, `"entrypoint": "SIDECAR_UI_ENTRYPOINT_AUX_PANE"`, title `"N'kai Triage Lab"`.
- [x] Create `c:\Dev\nkai\sidecars\panel\package.json`:
  - `{ "type": "module" }`.

### Phase 2: Node.js Sidecar Backend (`sidecars/panel/main.mjs`)
- [x] Implement `main.mjs` using `sidecar_sdk`:
  - Serves static assets (`core/sidecar.css`, `core/sidecar.js`, sound effects).
  - Serves canonical HTML template on `/`.
  - Implements lightweight JSON API endpoints:
    - `POST /api/action`: Records triage verdicts and telemetries.
    - `POST /api/echo`: Health check and status ping.
  - Automatically listens on `ANTIGRAVITY_SIDECAR_WEB_PORT` via `app.run()`.

### Phase 3: Frontend Bridge Adapter (`core/sidecar.js`)
- [x] Update `c:\Dev\nkai\core\sidecar.js`:
  - Detect `window.sidecar` and `window.sidecar.agent`.
  - When running as an Antigravity UI Extension:
    - Route `dispatchNkaiAction(actionData)` directly to `window.sidecar.agent.sendMessage(actionData.prompt)`.
    - Display persistent floating toast HUD (`⚡ Sent to Agent!`).
    - Trigger haptic chime sound.
  - When running under standalone `file:///` or HTTP CLI server:
    - Gracefully fallback to highlighting the readonly prompt box for manual `Ctrl+C` / `Ctrl+V`.
- [x] Synchronize changes to `c:\Dev\TheKlangSuite\.agents\sidecar\template.html` and `c:\Dev\nkai\dist\standalone-template.html`.

### Phase 4: Local Installation & Verification Harness
- [x] Author `c:\Dev\TheKlangSuite\tools\install_nkai_plugin.py` (and mirror to `nkai/tools/`):
  - Copies or links `c:\Dev\nkai` into `%USERPROFILE%\.gemini\config\plugins\nkai\`.
  - Asserts all required files exist (`plugin.json`, `logo.svg`, `sidecar.json`, `main.mjs`).
  - Verifies Node.js syntax and `sidecar.json` schema validity.
- [x] Execute `python tools/audit_sidecar.py` across all N'kai templates (confirm 100% tag balance, 0 errors).
- [x] Run `python tools/verify_context_clues.py` in `c:\Dev\TheKlangSuite` (confirm 100% pass).

### Phase 5: Closeout, Backlog Sync & Multi-Repo Commits
- [x] Update `docs/BACKLOG.md`: Mark N'kai v0.2.0 as complete and reaffirm v0.3.0 (Mobile) as deferred.
- [x] In `PLAN.md`: Mark all phases completed `[x]` and update Status to `COMPLETED ✅`.
- [x] Commit changes cleanly in `c:\Dev\TheKlangSuite` and `c:\Dev\nkai`.
