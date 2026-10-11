STATUS: ACTIVE

# Subagent Dispatch Charter: Tooling Implementation
> *Target Pipeline:* Dual-Track Communiqué — Tooling Pipeline (`TARGET: TOOLING_HARNESS`)  
> *Active Branch:* `0.4.0-dev`  
> *Orchestrator:* Antigravity Main Chat (New Klang City)  
> *Executor:* Flash Tooling Builder  

---

## 1. Assignment Header
- **STATUS**: `ACTIVE`
- **ROLE**: `Flash Tooling Builder`
- **MODEL TIER**: `Tier 2 Flash (High)`
- **TASK TYPE**: `TOOLING_IMPLEMENTATION`
- **TASK SLUG**: `flight1-multiplan-queue`
- **ACTIVE BLUEPRINT**: [`c:\Dev\TheKlangSuite\PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md)
- **TARGET REPOSITORIES**:
  - [x] `TheKlangSuite` (`c:\Dev\TheKlangSuite`)
  - [x] `nkai` (`c:\Dev\nkai`)

---

## 2. Scope & Target Files
Execute **Flight 1: The Multi-Plan Queue & Stateless Atomic Rollback Engine** following `PLAN.md`:

1. **Initialize `.agents/pipeline/plans/queue.json`**:
   - Schema version `1.0.0`, `active_plan: "PLAN.md"`, `active_target: "TOOLING_HARNESS"`, `execution_pointer: 0`.
   - Pre-populated with the 4 plans:
     - `plan-001` (Active, P0): `PLAN.md` (The Grand Operatic Flight Plan: Symphony of New Klang City)
     - `plan-002` (Queued, P1): `.agents/pipeline/plans/drafts/nkai_v020_ui_extension_heist.md` (Mayor Toad's Sidecar Embezzlement)
     - `plan-003` (Queued, P2): `.agents/pipeline/plans/drafts/2026-10-08_v041_ui_shells.md` (Milestone v0.4.1 Control Surface & UI Shells)
     - `plan-004` (Queued, P3): `.agents/pipeline/plans/drafts/v0.5.0_visual_harness_architecture.md` (Milestone v0.5.0 The Klang Hub)

2. **Author `tools/plan_queue.py` (and mirror to `nkai/tools/plan_queue.py`)**:
   - Python standard library with optional Pydantic validation fallback.
   - Hardened UTF-8 stdout (`sys.stdout.reconfigure(encoding='utf-8')`).
   - Commands:
     - `list` / `--list`: Pretty-print terminal table of active plan, queued plans, targets, priorities, and statuses.
     - `add <plan_file> [--priority P1] [--target TOOLING_HARNESS]`: Appends draft plan to `queue.json`.
     - `promote-next [--dry-run]`:
       - Snapshots active `PLAN.md` to `.agents/pipeline/plans/snapshots/PLAN_PREV.md`.
       - Advances execution pointer in `queue.json`.
       - Copies next queued plan into `PLAN.md`.
     - `rollback`:
       - Restores `.agents/pipeline/plans/snapshots/PLAN_PREV.md` back to `PLAN.md` in 5ms.
       - Reverts pointer in `queue.json`.
     - `--status`: Return structured JSON status.

3. **Register `/rollback-plan` Skill**:
   - Author `.agents/skills/rollback-plan/SKILL.md` documenting the 5-ms rollback tool and shortcut `/rollback-plan`.

4. **Update `.agents/skills/clean-plan/SKILL.md`**:
   - Integrate `python tools/plan_queue.py promote-next` as an optional workflow step when archiving a completed plan.

5. **Test Gauntlet**:
   - `python tools/plan_queue.py list`
   - `python tools/plan_queue.py promote-next --dry-run`
   - `python tools/plan_queue.py promote-next` (promote Plan 2)
   - Verify `PLAN.md` contains Plan 2
   - `python tools/plan_queue.py rollback` (restore Plan 1)
   - Verify `PLAN.md` contains Plan 1
   - `python tools/verify_context_clues.py`

6. **Closeout**:
   - In `PLAN.md`: Mark Flight 1 `[x] COMPLETED ✅`.
   - In `.agents/pipeline/communique/subagent_receipt.md`: Append structured receipt.
   - Commit changes cleanly across `TheKlangSuite` and `nkai`.
