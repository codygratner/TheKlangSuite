STATUS: IN_PROGRESS

# Subagent Dispatch Charter: Tooling Implementation
> *Target Pipeline:* Dual-Track Communiqué — Tooling Pipeline (`TARGET: TOOLING_HARNESS`)  
> *Active Branch:* `0.4.0-dev`  
> *Orchestrator:* Antigravity Main Chat (New Klang City)  
> *Executor:* Flash Tooling Builder Subagent  

---

## 1. Assignment Header
- **STATUS**: `IN_PROGRESS`
- **ROLE**: `Flash Tooling Builder`
- **MODEL TIER**: `Tier 2 Flash (High)`
- **TASK TYPE**: `TOOLING_IMPLEMENTATION`
- **TASK SLUG**: `flight-7-pcda-dialectic-and-sota-benchmarks`
- **ACTIVE BLUEPRINT**: [`c:\Dev\TheKlangSuite\PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md)
- **TARGET REPOSITORIES**:
  - [x] `TheKlangSuite` (`c:\Dev\TheKlangSuite`)
  - [x] `nkai` (`c:\Dev\nkai`)

---

## 2. Scope & Target Files
- **Primary Objective**:
  1. Author `docs/specs/nkai_pcda_governance_and_sota_benchmarks.md` (Topics 6.1, 6.2 & 6.15: Continuous Live Triage, 4-Part PCDA Dialectic Triad, On-Demand SOTA Progressive Disclosure, SOTA Precedent Teardowns) and index in `DOCS_CATALOG.json` and `SYSTEM_MAP.md`.
  2. Implement PCDA components and 4-lens layout styling in `c:\Dev\nkai\core\sidecar.css` and `sidecar.js` (`.pcda-container`, `.pcda-pro`, `.pcda-con`, `.pcda-advocate`, `.pcda-synthesis`, `.badge-sota`, `togglePcdaBlock`, `copyPcdaMarkdown`).
  3. Author `tools/pcda_format.py` in `c:\Dev\nkai\tools/` and `c:\Dev\TheKlangSuite\tools/` (zero-dependency validator, Markdown-to-HTML converter, and `--chat` formatter).
  4. Synchronize canonical templates: `c:\Dev\nkai\dist\standalone-template.html`, `c:\Dev\nkai\bundle\templates\template.html`, and `c:\Dev\TheKlangSuite\.agents\sidecar\template.html`.
  5. Run `python tools/audit_sidecar.py` across updated templates (100% tag balance) and test `python tools/pcda_format.py --help`.
  6. Commit changes cleanly across `TheKlangSuite` and `nkai` with conventional commits: `feat(pcda): implement Flight 7 PCDA dialectic governance and SOTA benchmarks`.
- **Target Files / Boundaries**:
  - `docs/specs/nkai_pcda_governance_and_sota_benchmarks.md`
  - `docs/DOCS_CATALOG.json`
  - `docs/SYSTEM_MAP.md`
  - `c:\Dev\nkai\core\sidecar.css`
  - `c:\Dev\nkai\core\sidecar.js`
  - `c:\Dev\nkai\tools\pcda_format.py`
  - `c:\Dev\TheKlangSuite\tools\pcda_format.py`
  - `c:\Dev\nkai\dist\standalone-template.html`
  - `c:\Dev\nkai\bundle\templates\template.html`
  - `c:\Dev\TheKlangSuite\.agents\sidecar\template.html`
  - `.agents/pipeline/communique/subagent_receipt.md`

---

## 3. Mandatory Invariants & Constraints
- **Zero Allocations & Audio Safety**: Tooling implementation; zero native C++ audio thread code touched.
- **Context Protection**: Complete all file writing, script creation, and verification inside subagent context. Return only a concise 3-line receipt.
- **Boy Scout Cleanliness**: Zero external dependencies in `pcda_format.py` (standard Python library only). 100% valid HTML in generated components.
