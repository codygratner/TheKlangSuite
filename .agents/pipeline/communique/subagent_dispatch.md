STATUS: COMPLETED

# Subagent Dispatch Charter: Tooling Implementation
> *Target Pipeline:* Dual-Track Communiqué — Tooling Pipeline (`TARGET: TOOLING_HARNESS`)  
> *Active Branch:* `0.4.0-dev`  
> *Orchestrator:* Antigravity Main Chat (New Klang City)  
> *Executor:* Flash Tooling Builder Subagent  

---

## 1. Assignment Header
- **STATUS**: `COMPLETED`
- **ROLE**: `Flash Tooling Builder`
- **MODEL TIER**: `Tier 2 Flash (High)`
- **TASK TYPE**: `TOOLING_IMPLEMENTATION`
- **TASK SLUG**: `flight-8-allowance-ledger-and-token-governance`
- **ACTIVE BLUEPRINT**: [`c:\Dev\TheKlangSuite\PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md)
- **TARGET REPOSITORIES**:
  - [x] `TheKlangSuite` (`c:\Dev\TheKlangSuite`)
  - [x] `nkai` (`c:\Dev\nkai`)

---

## 2. Scope & Target Files
- **Primary Objective**:
  1. Author `docs/specs/nkai_allowance_ledger_and_token_governance.md` (Topics 6.3 & 6.19: Deep-Linked Monolithic Board, Model Tier Token Heuristics, Cumulative Allowance Ledger) and index in `DOCS_CATALOG.json` and `SYSTEM_MAP.md`.
  2. Implement allowance meter and deep-link styling in `c:\Dev\nkai\core\sidecar.css` and `sidecar.js` (`.allowance-meter`, `.meter-bar-fill`, `updateAllowanceLedger`, hash-change router, `.anchor-highlight`).
  3. Author `tools/nkai_ledger.py` in `c:\Dev\nkai\tools/` and `c:\Dev\TheKlangSuite\tools/` (zero-dependency CLI audit and allowance calculator).
  4. Synchronize canonical templates: `c:\Dev\nkai\dist\standalone-template.html`, `c:\Dev\nkai\bundle\templates\template.html`, and `c:\Dev\TheKlangSuite\.agents\sidecar\template.html`.
  5. Run `python tools/audit_sidecar.py` across updated templates (100% tag balance) and test `python tools/nkai_ledger.py --help`.
  6. Commit changes cleanly across `TheKlangSuite` and `nkai` with conventional commits: `feat(ledger): implement Flight 8 cumulative allowance ledger and token governance`.
- **Target Files / Boundaries**:
  - `docs/specs/nkai_allowance_ledger_and_token_governance.md`
  - `docs/DOCS_CATALOG.json`
  - `docs/SYSTEM_MAP.md`
  - `c:\Dev\nkai\core\sidecar.css`
  - `c:\Dev\nkai\core\sidecar.js`
  - `c:\Dev\nkai\tools\nkai_ledger.py`
  - `c:\Dev\TheKlangSuite\tools\nkai_ledger.py`
  - `c:\Dev\nkai\dist\standalone-template.html`
  - `c:\Dev\nkai\bundle\templates\template.html`
  - `c:\Dev\TheKlangSuite\.agents\sidecar\template.html`
  - `.agents/pipeline/communique/subagent_receipt.md`

---

## 3. Mandatory Invariants & Constraints
- **Zero Allocations & Audio Safety**: Tooling implementation; zero native C++ audio thread code touched.
- **Context Protection**: Complete all file writing, script creation, and verification inside subagent context. Return only a concise 3-line receipt.
- **Boy Scout Cleanliness**: Zero external dependencies in `nkai_ledger.py` (standard Python library only). 100% valid HTML in generated components.
