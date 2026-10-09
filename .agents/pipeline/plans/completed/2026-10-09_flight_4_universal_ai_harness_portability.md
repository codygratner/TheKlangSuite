# Implementation Plan: Flight 4 — Universal AI Harness Portability
> **TARGET:** TOOLING_HARNESS (New Klang City) 🚀  
> **Flight Target:** Topic 6.13 (Universal AI Harness Portability)  
> **Status:** COMPLETED ✅  
> **Target Branch:** `0.4.0-dev`  
> **Recommended Model:** Gemini 3.8 Flash (Thinking: High) — 🟢 SUSTAINABLE  
> **Execution Strategy:** Delegated to Tier 2 Flash Builder Subagent via `subagent_dispatch.md`  

---

## 🎯 Architectural Intent & Scope

Flight 4 establishes universal cross-harness portability for N'kai (NKAI):
1. **Topic 6.13 (Approved Option B — Decoupled Interface Layer):**
   - Enables N'kai to run seamlessly across all major AI harnesses:
     - **Google Antigravity IDE**: Offline `file:///` sidecar artifact & native Sidecar SDK UI Extension (`window.sidecar`).
     - **Terminal CLIs (Claude Code CLI, OpenAI Codex CLI, Aider, terminal)**: Zero-dependency local dev server (`python tools/nkai_serve.py` or `npx nkai serve [file]`) that auto-launches the system browser and hot-reloads on file changes.
     - **Desktop Editors (Cursor, Windsurf, VS Code)**: Universal split-window rendering with zero CORS breakage.
   - Authors `docs/specs/nkai_universal_harness_portability.md` specifying the tri-mode environment detection protocol (`Extension` vs `LocalFile` vs `HttpServer`).
   - Implements `tools/nkai_serve.py` in `c:\Dev\nkai\` and mirrors to `TheKlangSuite/tools/`.
   - Packages `c:\Dev\nkai\package.json` and `bin/nkai.js` for lightweight `npx nkai serve` CLI execution.
   - Updates `c:\Dev\nkai\core\sidecar.js` with the Tri-Mode Environment Adapter.

---

## 📋 Phase-by-Phase Execution Plan

### Phase 1: Universal Portability Architecture Specification (Topic 6.13)
- [x] Author `docs/specs/nkai_universal_harness_portability.md`:
  - Define the Tri-Mode Environment Detection hierarchy (`Antigravity Extension` $\to$ `Local File:///` $\to$ `HTTP Server`).
  - Document CLI serving architecture (local Python/Node zero-dependency HTTP server, port negotiation, MIME types, auto-browser launch).
  - Document terminal agent pairing patterns (Claude Code CLI, Cursor, Windsurf, Aider).
- [x] Register spec in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.

### Phase 2: Tri-Mode Environment Adapter in `core/sidecar.js`
- [x] Update `c:\Dev\nkai\core\sidecar.js`:
  - Add `NkaiEnv.detect()` returning `'extension' | 'file' | 'server'`.
  - Add unified IPC / messaging abstraction:
    - If `extension`: route through `window.sidecar.agent.sendMessage()`.
    - If `file`: route through uneditable click-to-highlight prompt boxes.
    - If `server`: support optional HTTP SSE / POST loopback for terminal CLI agents.
- [x] Update `c:\Dev\nkai\dist\standalone-template.html` and `c:\Dev\nkai\bundle\templates\template.html`.

### Phase 3: Zero-Dependency CLI Server (`tools/nkai_serve.py` & `bin/nkai.js`)
- [x] Author `c:\Dev\nkai\tools\nkai_serve.py`:
  - Lightweight Python standard library HTTP server (`http.server`, `threading`, `webbrowser`).
  - Support `python tools/nkai_serve.py <html-file-or-dir> [--port 3030] [--no-browser]`.
  - Proper MIME handling for `.html`, `.css`, `.js`, `.json`, `.svg`, `.png`, `.wav`.
  - Mirror to `c:\Dev\TheKlangSuite\tools\nkai_serve.py`.
- [x] Author `c:\Dev\nkai\package.json` and `c:\Dev\nkai\bin\nkai.js`:
  - Standard npm `bin` definition pointing to a clean, zero-dependency Node.js HTTP server.
  - Enables `npx nkai serve [path]` without global installation.

### Phase 4: Bundle Synchronization & Verification
- [x] Update `c:\Dev\nkai\bundle\tools\` with `nkai_serve.py`.
- [x] Update `c:\Dev\nkai\bundle\templates\` with tri-mode adapted `template.html`.
- [x] Test `python tools/nkai_serve.py --help` and validate server startup/shutdown.
- [x] Run `python tools/audit_sidecar.py` across updated templates (100% tag balance, 0 errors).
- [x] Run `python tools/verify_context_clues.py` to ensure link integrity.
- [x] Commit changes cleanly across `TheKlangSuite` and `nkai`.

---

## 🛡️ Verification & Acceptance Criteria
1. `docs/specs/nkai_universal_harness_portability.md` is valid and indexed in `DOCS_CATALOG.json`.
2. `sidecar.js` dynamically detects and adapts to `extension`, `file:///`, and `http://` runtimes.
3. `tools/nkai_serve.py` successfully serves local HTML sidecars with proper MIME types.
4. `package.json` and `bin/nkai.js` enable `npx nkai serve` without third-party dependencies.
5. `audit_sidecar.py` and `verify_context_clues.py` exit with 0 errors.
