# 🐸 Mayor Toad's Sidecar Embezzlement: N'kai v0.2.0 Native Extension & v0.3.0 Mobile Companion

> **ID:** plan-002  
> **TARGET:** TOOLING_HARNESS (New Klang City) 🚀  
> **Topic Target:** N'kai v0.2.0 (UI Extension & Native IPC Bridge) & N'kai v0.3.0 (Mobile Touch Companion & LAN Wi-Fi Pairing)  
> **Status:** QUEUED (Staged for Multi-Plan Queue Execution) ⏳  
> **Priority:** P1  
> **Target Branch:** `0.4.0-dev`  
> **Recommended Setting:** Gemini 3.8 Flash (Thinking: High) — 🟢 SUSTAINABLE  
> **Source Agreements:** `docs/BACKLOG.md` (Sections 13 & 14) & System Sanity Audit Topic 6.5, 6.17, 6.18  

---

## 🎯 Architectural Intent & Storyline

While the factory floor in Klang Industries is busy building DSP filters, Mayor Toad has quietly siphoned off municipal tooling resources to turn N'kai into his personal executive dashboard:
1. **v0.2.0: Official Google Antigravity UI Extension**:
   - Upgrades N'kai from an offline `file:///` sidecar into an officially registered Antigravity Plugin UI Extension using the built-in Sidecar SDK.
   - Replaces the temporary uneditable click-to-highlight prompt workaround with **true zero-copy native IPC** (`window.sidecar.agent.sendMessage()`).
   - Native host audio triggers for mechanical clicks and chime notifications.
2. **v0.3.0: Mobile Couch Companion & Zero-Config LAN Pairing**:
   - Enables Mayor Toad (and the developer) to conduct morning mail reviews and triage sessions directly from an iPhone or Android phone on the studio couch.
   - Touch ergonomics: 44px minimum tap targets, bottom-sheet thumb navigation, momentum scrolling.
   - Zero-config local LAN pairing via `python tools/nkai_serve.py --host 0.0.0.0` with auto-detected local IP and terminal ASCII QR code!

---

## 📋 Phase Breakdown

### Phase 1: Official Antigravity UI Extension Manifest & Packaging
- [ ] Create `c:\Dev\nkai\plugin.json` (or Antigravity plugin manifest structure):
  - Register `nkai` as a native UI extension with side-panel icon, title, and activation events.
  - Configure permissions: Webview host permissions, host clipboard access.
- [ ] Implement Sidecar SDK Bridge in `c:\Dev\nkai\core\sidecar.js`:
  - Detect `window.sidecar` presence.
  - Bind `dispatchNkaiAction()` directly to `window.sidecar.agent.sendMessage()` when in extension mode.
  - Retain `file:///` click-to-highlight fallback for air-gapped or non-Antigravity browsers.

### Phase 2: Native IPC Action Dispatch & Telemetry
- [ ] Author typed action contracts:
  - `ActionPayload`: `{ type: "TRIAGE_VERDICT" | "RUN_SKILL" | "EXPAND_RESEARCH", topicId: string, verdict?: string, prompt?: string }`.
- [ ] Wire 1-click execution:
  - Clicking `[ ✅ APPROVE ]` in extension mode immediately posts action to the agent without requiring manual chat pasting.
  - Floating toast HUD displays `⚡ Native IPC Dispatch: 200 OK`.
- [ ] Native audio trigger integration:
  - Connect haptic ticks and major triad chimes to native audio elements.

### Phase 3: Mobile Viewport & Touch Ergonomics (v0.3.0)
- [ ] In `c:\Dev\nkai\core\sidecar.css`:
  - Add responsive media queries for 320px–390px (iPhone/Android viewports).
  - Enforce minimum 44px touch targets on all verdict chips (`[ APPROVE ]`, `[ TABLE ]`, `[ KILL ]`, `[ PCDA ]`).
  - Implement bottom-sheet thumb drawer (`.mobile-action-bar`) anchored to viewport bottom.
  - Enable hardware momentum touch scrolling (`-webkit-overflow-scrolling: touch`).
- [ ] Verify tab navigation and allowance meters adapt gracefully without horizontal clipping.

### Phase 4: Zero-Config LAN Pairing & QR Code Server
- [ ] Update `tools/nkai_serve.py` (and mirror to `nkai/tools/`):
  - Add `--lan` / `--host 0.0.0.0` flag.
  - Auto-detect local network IP (e.g. `192.168.1.X`).
  - Output clickable LAN link and optional lightweight terminal ASCII QR code (or fallback URL display).
  - Display clear studio banner: `📱 Mobile Sidecar Online: Open http://192.168.1.X:3030 on your phone`.

### Phase 5: Verification & End-to-End Test Flight
- [ ] Run `python tools/audit_sidecar.py` across updated templates (100% tag balance, 0 errors).
- [ ] Test mobile responsiveness in Headless Chrome (`--window-size=375,812`).
- [ ] Test LAN server binding on local interface.
- [ ] Document release in `docs/BACKLOG.md` and commit cleanly.
