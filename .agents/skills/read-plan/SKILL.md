---
name: read-plan
description: Serves as Klang Industries' official Communiqué Dispatch Ingestor. Inspects .agents/pipeline/communique/plan_to_build.md and PLAN.md, transitions dispatch lifecycle to IN_PROGRESS, outputs the model verification badge, and readies execution. Triggers on `/readplan`, `/read-plan`, "read plan", "ingest plan", "take job", or "take plan".
---

# Communiqué Dispatch Ingestor (`read-plan`)

## Goal
Ingest architectural blueprints and signed dispatch contracts published by New Klang City in `.agents/pipeline/communique/plan_to_build.md` (or `PLAN.md`), verify the contract lifecycle state, transition it to `STATUS: IN_PROGRESS`, output model advisory verification, and ready the factory floor for execution—with zero arbitrary clipboard sniffing.

---

## Workflow

### 1. Inspect Communiqué Mailbox (`.agents/pipeline/communique/plan_to_build.md`)
1. Open and parse `.agents/pipeline/communique/plan_to_build.md` at project root.
2. **Facility Check (Domain Boundary Guard)**:
   - Inspect the `TARGET:` or `EXECUTOR:` header in `plan_to_build.md` and `PLAN.md`.
   - If `TARGET:` is `TOOLING_HARNESS`, `IVORY_TOWER`, or `NEW_KLANG_CITY`:
     - Output:
       > 📬 **Facility Stand-Down**: Active dispatch is routed to **New Klang City (Tooling Harness)**. Klang Industries is reserved strictly for native C++ DSP and remains idle 💤.
     - Halt execution immediately. Do NOT transition contract status to `IN_PROGRESS` or touch `build_to_plan.md`.
3. Check the `Status:` field:
   - **Case A: `STATUS: READY_FOR_EXECUTION` or `STATUS: PLAN_AMENDED` (Active or Amended Contract)**:
     - Extract **Task Name**, **Active Milestone**, **Recommended Model Tier**, **Strategic Objective**, and **Directives**.
     - If `STATUS: PLAN_AMENDED`, highlight the amendment changelog prominently in the terminal.
     - Verify that [`PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md) at root matches this task. If `PLAN.md` is empty or holds an older blueprint, synchronize the directives into `PLAN.md`.
     - Update `.agents/pipeline/communique/plan_to_build.md` setting:
       `Status: IN_PROGRESS` (with timestamp).
     - Update `.agents/pipeline/communique/build_to_plan.md` setting:
       `## Status: BUILDING 🔨 (Phase 1: <Phase Name>)` (ensuring New Klang City has real-time factory telemetry).
     - Proceed directly to **Section 3 (Briefing & Model Gate)**.
   - **Case B: `STATUS: COMPLETED` or `STATUS: DRAFTING` (No Pending Dispatch)**:
     - Check [`PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md) on disk.
     - If `PLAN.md` exists, contains active unchecked items (`- [ ]`), and is NOT designated for `TOOLING_HARNESS`, load `PLAN.md` and proceed to Section 3.
     - If both `plan_to_build.md` and `PLAN.md` are idle/completed or belong to tooling:
       - Proceed to **Section 2 (Workspace Fallback Discovery)**.

---

### 2. Workspace Fallback Discovery (If Mailbox is Idle)
If no active contract is waiting in `plan_to_build.md`:
1. Scan `docs/specs/` for unexecuted feature specs (`docs/specs/*.md`) and check top priorities in [`docs/BACKLOG.md`](file:///c:/Dev/TheKlangSuite/docs/BACKLOG.md).
2. Present a clean interactive picker via `ask_question`:
   - **Question:** *"No pending dispatch in `plan_to_build.md`. Which roadmap item would you like to review or stage?"*
   - **Options:** Formatted as direct user selections (e.g., `Load [Feature Spec] docs/specs/some_spec.md`, `Stage top item from docs/BACKLOG.md`, `Keep idle and await New Klang City dispatch`).
3. If an item is selected, load its contents into `PLAN.md`.

---

### 3. Dispatch Intake Briefing & Builder Model Gate
Display the ingested contract summary:

```markdown
# 📬 Blueprint Ingested from Communiqué

**Task:** <Task Name>  
**Milestone:** <Milestone>  
**Status:** IN_PROGRESS  

### Strategic Objective
<Objective Summary>

### Immediate Focus: Phase 1
- **Target Files:** <Files>
- **Verification Target:** <Unit tests / assertions>
```

#### Smart Model Verification & Non-Modal Gate:
1. **Self-Inspection**: Inspect active model and thinking configuration from session instructions.
2. **If Active Model Matches/Exceeds Recommended Tier**:
   - Output subtle verification badge:
     `✓ Model Verified: <Model> (<Thinking Level>) matches Tier <N>`
   - Prompt user in chat text:
     > Ready to construct. Reply **`proceed`** or **`engage`** to launch Phase 1.
3. **If Active Model is Below Recommended Tier**:
   - Render the prominent Model Advisory block:
     ```markdown
     > 🧠 **MODEL ADVISORY: Tier <N>**
     > - **Recommended Setting:** <Recommended Model & Thinking>
     > - **Quota Impact:** <Impact>
     > - **Active Model Check:** Please adjust your model dropdown in the IDE footer before proceeding!
     ```
   - **CRITICAL BUILDER GUARDRAIL (NO MODALS ON EXECUTION START):** Under NO circumstances invoke `ask_question` here (interactive modals freeze the IDE footer and prevent changing the model dropdown). Pause in regular chat text waiting for user to adjust model and reply `proceed`.

---

### 4. Execution Handoff
When the user replies `proceed`, `engage`, or `harvest & proceed`:
- Verify current branch safety (must not be `master` or `main`).
- Hand off execution directly to Phase 1 of the automated factory pipeline (`execute-task` / `step-verify`).
