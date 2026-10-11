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
- **TASK SLUG**: `flight-1-ramble-ingest`
- **ACTIVE BLUEPRINT**: [`c:\Dev\TheKlangSuite\PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md)
- **TARGET REPOSITORIES**:
  - [x] `TheKlangSuite` (`c:\Dev\TheKlangSuite`)
  - [x] `nkai` (`c:\Dev\nkai`)

---

## 2. Scope & Target Files
- Target Milestone: Flight 1: The "Rambles" Ingestion Engine (`P0 — Topic 2`).
- Phases to execute:
  - Phase 1: Architectural Specification & Catalog Registration (`docs/specs/nkai_rambles_ingestion_engine.md`, `docs/DOCS_CATALOG.json`, `docs/SYSTEM_MAP.md`).
  - Phase 2: Pydantic Data Models (`tools/models.py`: `RambleCard`, `RambleExtractionResult`, `to_mail_item()`).
  - Phase 3: Zero-Dependency Ramble Ingestor CLI (`tools/ramble_ingest.py`, mirrored to `c:\Dev\nkai\tools\ramble_ingest.py` and `c:\Dev\nkai\bundle\tools\ramble_ingest.py`).
  - Phase 4: Micro-Skill Registration (`.agents/skills/ramble-ingest/SKILL.md`, mirrored to `c:\Dev\nkai\bundle\skills\ramble-ingest\SKILL.md`).
  - Phase 5: Verification Gauntlet, Backlog Sync, and Clean Commits across `TheKlangSuite` and `nkai`.
