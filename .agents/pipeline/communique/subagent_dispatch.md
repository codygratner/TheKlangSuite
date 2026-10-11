STATUS: DISPATCHED

# Subagent Dispatch Charter: Tooling Implementation
> *Target Pipeline:* Dual-Track Communiqué — Tooling Pipeline (`TARGET: TOOLING_HARNESS`)  
> *Active Branch:* `0.4.0-dev`  
> *Orchestrator:* Antigravity Main Chat (New Klang City)  
> *Executor:* Flash Builder Subagent  

---

## 1. Assignment Header
- **STATUS**: `DISPATCHED`
- **ROLE**: `Flash Tooling Builder`
- **MODEL TIER**: `Tier 2 Flash (High)`
- **TASK TYPE**: `TOOLING_IMPLEMENTATION`
- **TASK SLUG**: `nkai-v020-ui-extension`
- **ACTIVE BLUEPRINT**: [`c:\Dev\TheKlangSuite\PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md)
- **TARGET REPOSITORIES**:
  - [x] `TheKlangSuite` (`c:\Dev\TheKlangSuite`)
  - [x] `nkai` (`c:\Dev\nkai`)

---

## 2. Scope & Target Files
- Target Milestone: N'kai v0.2.0 (Official Antigravity UI Extension & Zero-Copy Native IPC).
- Phases to execute:
  - Phase 1: Native Plugin Manifest & Sidecar Infrastructure (`c:\Dev\nkai\plugin.json`, `c:\Dev\nkai\assets\logo.svg`, `c:\Dev\nkai\sidecars\panel\sidecar.json`, `c:\Dev\nkai\sidecars\panel\package.json`).
  - Phase 2: Node.js Sidecar Backend (`c:\Dev\nkai\sidecars\panel\main.mjs`) using `sidecar_sdk`.
  - Phase 3: Frontend Bridge Adapter (`c:\Dev\nkai\core\sidecar.js`) with `window.sidecar.agent.sendMessage()`.
  - Phase 4: Local Installation & Verification Harness (`tools/install_nkai_plugin.py` installing to `%USERPROFILE%\.gemini\config\plugins\nkai\`, `tools/audit_sidecar.py`, `tools/verify_context_clues.py`).
  - Phase 5: Closeout & Subagent Receipt.
