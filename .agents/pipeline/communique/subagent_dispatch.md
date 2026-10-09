STATUS: READY_FOR_EXECUTION

# Subagent Dispatch Charter: Tooling Implementation
> *Target Pipeline:* Dual-Track Communiqué — Tooling Pipeline (`TARGET: TOOLING_HARNESS`)  
> *Active Branch:* `0.4.0-dev`  
> *Orchestrator:* Antigravity Main Chat (New Klang City)  
> *Executor:* Flash Builder Subagent (`invoke_subagent(Model="flash")`)  

---

## 1. Assignment Header
- **STATUS**: `READY_FOR_EXECUTION`
- **ROLE**: `Flash Builder Subagent`
- **MODEL TIER**: `Tier 2 Flash (High)`
- **TASK TYPE**: `TOOLING_IMPLEMENTATION`
- **TASK SLUG**: `flight-5-header-ergonomics-and-density`
- **ACTIVE BLUEPRINT**: [`c:\Dev\TheKlangSuite\PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md)
- **TARGET REPOSITORIES**:
  - [x] `TheKlangSuite` (`c:\Dev\TheKlangSuite`)
  - [x] `nkai` (`c:\Dev\nkai`)

---

## 2. Scope & Target Files
- **Primary Objective**:
  1. Author `docs/specs/nkai_header_ergonomics_and_spatial_density.md` (Topics 6.9 & 6.10: Two-tier header geometry, utility cluster, hybrid compact tabs with hover tooltips) and register in `DOCS_CATALOG.json` and `SYSTEM_MAP.md`.
  2. Update `c:\Dev\nkai\core\sidecar.css` and `sidecar.js` with `.header-tier-top`, `.header-tier-bottom`, and `.tab-compact` with hover tooltips and narrow-viewport (360px+) media queries.
  3. Synchronize canonical templates: `c:\Dev\nkai\dist\standalone-template.html`, `c:\Dev\nkai\bundle\templates\template.html`, and `c:\Dev\TheKlangSuite\.agents\sidecar\template.html`.
  4. Run `python tools/audit_sidecar.py` and `python tools/verify_context_clues.py`.
  5. Commit changes cleanly across `TheKlangSuite` and `nkai`.
- **Target Files / Boundaries**:
  - `docs/specs/nkai_header_ergonomics_and_spatial_density.md`
  - `docs/DOCS_CATALOG.json`
  - `docs/SYSTEM_MAP.md`
  - `c:\Dev\nkai\core\sidecar.css`
  - `c:\Dev\nkai\core\sidecar.js`
  - `c:\Dev\nkai\dist\standalone-template.html`
  - `c:\Dev\nkai\bundle\templates\template.html`
  - `c:\Dev\TheKlangSuite\.agents\sidecar\template.html`
  - `.agents/pipeline/communique/subagent_receipt.md`

---

## 3. Mandatory Invariants & Constraints
- **Zero Allocations & Audio Safety**: This is a tooling task; do not touch native C++ audio loops.
- **Context Protection**: Complete all file writing and verification inside subagent context. Return only a concise 3-line receipt.
- **Boy Scout Cleanliness**: Zero AI slop, clean CSS without hardcoded viewport glitches, 100% responsive.
