---
name: rollback-plan
description: Instantly restores prior PLAN.md from snapshots/PLAN_PREV.md in 5ms. Triggers on /rollback-plan, /rollbackplan, or "rollback plan".
---

# Stateless Atomic Plan Rollback Engine

## Goal
Instantly restore the prior `PLAN.md` from `.agents/pipeline/plans/snapshots/PLAN_PREV.md` in ~5ms with zero token burn, reverting the execution pointer in `.agents/pipeline/plans/queue.json` without requiring complex context reconstruction.

## Trigger Conditions
- Triggered by `/rollback-plan`, `/rollbackplan`, or natural language phrases like "rollback plan", "revert plan", or "undo plan promotion".
- Used when a newly promoted plan needs to be reversed immediately, or when an aborted task requires returning to the preceding flight plan.

## Workflow

### 1. Execute Rollback Script
Run the stateless rollback CLI command:
```powershell
python tools/plan_queue.py rollback
```

### 2. Verify Output & State
The command automatically:
1. Restores `.agents/pipeline/plans/snapshots/PLAN_PREV.md` back to root `PLAN.md`.
2. Decrements `execution_pointer` in `.agents/pipeline/plans/queue.json` (if pointer > 0).
3. Reverts the current plan's status back to `QUEUED` and prior plan back to `IN_PROGRESS`.
4. Updates top-level `active_target` to match the restored plan.

### 3. Report Status to User
Confirm the restored plan to the user:
```markdown
⚡ **Plan Rollback Complete (5ms)**
- **Active Plan**: [<Restored Plan Title>](file:///c:/Dev/TheKlangSuite/PLAN.md)
- **Target Pipeline**: `TOOLING_HARNESS` | `FACTORY_FLOOR`
- **Queue Pointer**: `<index>`
```
