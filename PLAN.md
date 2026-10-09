# Implementation Plan: Flight 9 — Distribution Packaging & Key Ergonomics
> **Flight Target:** Topics 6.12 & 6.6 (Phase 4: Packaging & Key Ergonomics)  
> **Status:** COMPLETED ✅  
> **Target Branch:** `0.4.0-dev`  
> **Recommended Model:** Gemini 3.8 Flash (Thinking: High) — 🟢 SUSTAINABLE  
> **Authority:** New Klang City (Ivory Tower) ➔ Klang Industries (Factory Floor)  

---

## 🎯 Architectural Intent & Scope

Flight 9 packages N'kai into a portable, production-grade distribution bundle and integrates key developer ergonomics into the sidecar interface:
1. **Topic 6.12 (Approved Option B — Monolithic All-in-One Bundle):**
   - Assembles `c:\Dev\nkai\bundle\` containing the canonical pairing core: standardized skills (`visual-grill-me`, `deep-research`, `post-mortem`, `web-qa`), templates (`template.html`, `context_clues_template.md`), guidelines, and verification tools.
   - Author `install_nkai_bundle.ps1` to enable 1-command installation into any target project (`TheKlangSuite`, `ToadTracker`, or any external repo), featuring a universal pairing core with an optional `-Audio` flag for audio thread guardrails.
   - Authors `docs/specs/nkai_distribution_bundle_architecture.md` formalizing the bundle structure and lifecycle.
2. **Topic 6.6 (Approved Option B — Dynamic Shortcut Bar & Power Tools):**
   - Updates the canonical sidecar templates (`c:\Dev\nkai\dist\standalone-template.html`, `c:\Dev\nkai\bundle\templates\template.html`, and `c:\Dev\TheKlangSuite\.agents\sidecar\template.html`).
   - Implements a subtle, non-intrusive Dynamic Shortcut Helper (`[ ⌨️ Shortcuts ]`) highlighting Antigravity developer shortcuts (`Ctrl+K`, `Ctrl+Shift+P`, `Ctrl+Enter`) and core slash commands (`/plan`, `/grill-me`, `/execute-task`, `/postmortem`) to build muscle memory without chat token burn.

---

## 📋 Phase-by-Phase Execution Plan

### Phase 1: Distribution Bundle Architecture Specification (Topic 6.12)
- [x] Author `docs/specs/nkai_distribution_bundle_architecture.md`:
  - Define bundle directory topology (`bundle/skills/`, `bundle/templates/`, `bundle/rules/`, `bundle/tools/`).
  - Document the Universal Pairing Core vs Audio Add-on separation (`-Audio` switch).
  - Document the single-command deployment contract (`./install_nkai_bundle.ps1 -TargetDir <path> [-Audio]`).
  - Document the Git-ignore boundary (`.agents/sidecar/*.html` ignored, `!template.html` preserved).
- [x] Register the new spec in `docs/DOCS_CATALOG.json` under `specifications`.
- [x] Add direct link in `docs/SYSTEM_MAP.md` under Section 7 (Design Specs).

### Phase 2: Assemble Canonical `nkai/bundle/` Core & Audio Add-on
- [x] Create `c:\Dev\nkai\bundle\` structure:
  - `bundle/templates/`:
    - Seed `bundle/templates/template.html` from `c:\Dev\TheKlangSuite\.agents\sidecar\template.html`.
    - Seed `bundle/templates/context_clues_template.md` from `c:\Dev\TheKlangSuite\docs\templates\context_clues_template.md`.
  - `bundle/skills/`:
    - Copy canonical skills: `visual-grill-me`, `deep-research`, `post-mortem`, `web-qa`.
  - `bundle/rules/`:
    - Author `bundle/rules/nkai_guidelines.md` (Asymmetric Sidecar Split, Sidecar Ready Handshake, uneditable click-to-highlight prompt standard, 2-line footer).
    - Author `bundle/rules/audio_guardrails_addon.md` (Audio thread invariants: zero dynamic allocation, zero locks, zero blocking I/O, SIMD FastMath).
  - `bundle/tools/`:
    - Copy `audit_sidecar.py` and `verify_context_clues.py`.

### Phase 3: Universal Installer Script (`install_nkai_bundle.ps1`)
- [x] Author `c:\Dev\nkai\install_nkai_bundle.ps1`:
  - Support `-TargetDir <string>` (defaults to `$PWD`).
  - Support `[switch]$Audio` (installs audio guardrail rules when specified).
  - Support `[switch]$DryRun` (previews actions without modifying disk).
  - Automate directory creation (`.agents/skills/`, `.agents/sidecar/`, `docs/templates/`, `tools/`).
  - Copy skills, templates, tools, and applicable rules.
  - Automatically verify/update target `.gitignore` to protect sidecar artifacts.
  - Output high-density visual pass report.
- [x] Provide convenience copy in `c:\Dev\TheKlangSuite\tools\install_nkai_bundle.ps1`.

### Phase 4: Dynamic AGY Keyboard Shortcuts Bar & Power Tool Prompters (Topic 6.6)
- [x] Update `c:\Dev\nkai\core\sidecar.css` and `c:\Dev\nkai\core\sidecar.js`:
  - Add styles for `.shortcut-pill` and `.shortcut-popover`.
  - Provide toggleable shortcut helper HUD:
    - `Ctrl+K`: Quick Chat / Focus
    - `Ctrl+Shift+P`: Command Palette
    - `Ctrl+Enter`: Submit Prompt
    - `/plan`, `/grill-me`, `/execute-task`, `/postmortem`
  - Ensure zero collision with narrow IDE viewports (360px+).
- [x] Synchronize shortcut helper markup to:
  - `c:\Dev\nkai\dist\standalone-template.html`
  - `c:\Dev\nkai\bundle\templates\template.html`
  - `c:\Dev\TheKlangSuite\.agents\sidecar\template.html`

### Phase 5: Automated Verification & Ecosystem Gauntlet
- [x] Run `python tools/audit_sidecar.py` on all updated HTML templates to assert 100% tag balance and 0 syntax errors.
- [x] Run `python tools/verify_context_clues.py` to assert 100% link resolution.
- [x] Test `install_nkai_bundle.ps1 -TargetDir <scratchDir> -Audio` on a test directory to assert perfect installation.
- [x] Run `powershell -ExecutionPolicy Bypass -File .\tools\sync_obsidian_vault.ps1` to recompile vault indices.
- [x] Stage and commit changes with conventional commits: `feat(distribution): implement Flight 9 all-in-one bundle and dynamic shortcuts`.

---

## 🛡️ Verification & Acceptance Criteria
1. `c:\Dev\nkai\bundle\` contains complete, canonical `skills/`, `templates/`, `rules/`, and `tools/`.
2. `install_nkai_bundle.ps1` successfully deploys bundle to target directory with exit code 0.
3. `-Audio` flag selectively installs audio guardrails without polluting non-audio projects.
4. `template.html` features working Dynamic Shortcut helper (`[ ⌨️ Shortcuts ]`).
5. `audit_sidecar.py` and `verify_context_clues.py` pass cleanly.
