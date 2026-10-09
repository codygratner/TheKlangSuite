STATUS: COMPLETED

# Subagent Dispatch Charter: Tooling Implementation
> *Target Pipeline:* Dual-Track Communiqué — Tooling Pipeline (`TARGET: TOOLING_HARNESS`)  
> *Active Branch:* `0.4.0-dev`  
> *Orchestrator:* Antigravity Main Chat (New Klang City)  
> *Executor:* Flash Builder Subagent (`invoke_subagent(Model="flash")`)  

---

## 1. Assignment Header
- **STATUS**: `COMPLETED`
- **ROLE**: `Flash Builder Subagent`
- **MODEL TIER**: `Tier 2 Flash (High)`
- **TASK TYPE**: `TOOLING_IMPLEMENTATION`
- **TASK SLUG**: `flight-4-universal-portability`
- **ACTIVE BLUEPRINT**: [`c:\Dev\TheKlangSuite\PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md)
- **TARGET REPOSITORIES**:
  - [x] `TheKlangSuite` (`c:\Dev\TheKlangSuite`)
  - [x] `nkai` (`c:\Dev\nkai`)

---

## 2. Scope & Target Files
- **Primary Objective**:
  1. Author `docs/specs/nkai_universal_harness_portability.md` defining Tri-Mode Environment Detection (`extension`, `file:///`, `server`) and register in `DOCS_CATALOG.json` and `SYSTEM_MAP.md`.
  2. Update `c:\Dev\nkai\core\sidecar.js` with `NkaiEnv.detect()` tri-mode adapter.
  3. Author `c:\Dev\nkai\tools\nkai_serve.py` (lightweight standard-library HTTP server) and mirror to `TheKlangSuite/tools/`.
  4. Author `c:\Dev\nkai\package.json` and `c:\Dev\nkai\bin\nkai.js` enabling `npx nkai serve [path]`.
  5. Synchronize templates and tools to `c:\Dev\nkai\bundle/`.
  6. Run `audit_sidecar.py` and `verify_context_clues.py` to assert 100% compliance.
  7. Commit changes cleanly across `TheKlangSuite` and `nkai`.
- **Target Files / Boundaries**:
  - `docs/specs/nkai_universal_harness_portability.md`
  - `docs/DOCS_CATALOG.json`
  - `docs/SYSTEM_MAP.md`
  - `c:\Dev\nkai\core\sidecar.js`
  - `c:\Dev\nkai\tools\nkai_serve.py`
  - `c:\Dev\nkai\package.json`
  - `c:\Dev\nkai\bin\nkai.js`
  - `c:\Dev\nkai\bundle/`
  - `.agents/pipeline/communique/subagent_receipt.md`

---

## 3. Mandatory Invariants & Constraints
- **Zero Allocations & Audio Safety**: This is a tooling task; do not touch native C++ audio rendering loops.
- **Context Protection**: Complete all file writing and verification inside subagent context. Return only a concise 3-line receipt.
- **Boy Scout Cleanliness**: Zero AI slop, strict formatting, and zero external dependencies (pure Python standard library and Node.js standard library).
