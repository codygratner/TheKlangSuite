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
- **TASK SLUG**: `flight-3-skill-taxonomy-and-defaults`
- **ACTIVE BLUEPRINT**: [`c:\Dev\TheKlangSuite\PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md)
- **TARGET REPOSITORIES**:
  - [x] `TheKlangSuite` (`c:\Dev\TheKlangSuite`)
  - [x] `nkai` (`c:\Dev\nkai`)

---

## 2. Scope & Target Files
- **Primary Objective**:
  1. Author `docs/specs/nkai_skill_taxonomy_and_invocation_architecture.md` (3-layer hierarchy: atomic micro-skills, macro orchestrators, interactive visual experiences + `--chat` fallback) and register in `DOCS_CATALOG.json` and `SYSTEM_MAP.md`.
  2. Update visual skills (`visual-grill-me`, `deep-research`, `post-mortem`) in `.agents/skills/` to codify N'kai-first default canvas creation and document `--chat` / `--text-only` fallback.
  3. Update macro skills (`execute-task`, `web-qa`) with domain routing and toolchain sequencing.
  4. Synchronize updated skills to `c:\Dev\nkai\bundle\skills/`.
  5. Validate with `audit_sidecar.py` and `verify_context_clues.py`.
  6. Commit changes cleanly across `TheKlangSuite` and `nkai`.
- **Target Files / Boundaries**:
  - `docs/specs/nkai_skill_taxonomy_and_invocation_architecture.md`
  - `docs/DOCS_CATALOG.json`
  - `docs/SYSTEM_MAP.md`
  - `.agents/skills/visual-grill-me/SKILL.md`
  - `.agents/skills/deep-research/SKILL.md`
  - `.agents/skills/post-mortem/SKILL.md`
  - `.agents/skills/execute-task/SKILL.md`
  - `.agents/skills/web-qa/SKILL.md`
  - `c:\Dev\nkai\bundle\skills/`
  - `.agents/pipeline/communique/subagent_receipt.md`

---

## 3. Mandatory Invariants & Constraints
- **Zero Allocations & Audio Safety**: This is a tooling task; do not touch native C++ audio loops.
- **Context Protection**: Complete all file writing and verification inside subagent context. Return only a concise 3-line receipt.
- **Boy Scout Cleanliness**: Zero AI slop, clean formatting, accurate relative paths.
