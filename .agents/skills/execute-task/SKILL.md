---
name: execute-task
description: Executes the automated factory engineering pipeline (Code Edits -> audiothread-guard -> build-validate -> step-verify -> task-finish) on demand for an active plan or backlog item. Sourced strictly from PLAN.md or docs/BACKLOG.md. Triggers on `/executetask`, `/execute-task`, `/buildtask`, `/build-task`, `/runbacklog`, `/run-backlog`, `/run-plan`, or `/execute-plan`.
---

# Factory Task Runner & Pipeline Orchestrator (`execute-task`)

## Goal
Execute the automated factory engineering pipeline on demand for the active blueprint in `PLAN.md` or a prioritized backlog item from `docs/BACKLOG.md`, enforcing audio-thread safety, compiler sanity, and milestone staging with zero external tracker dependencies.

---

## Operational Constraints
- **Preserve Deterministic Pipeline Chain:** Every implementation phase must strictly follow the deterministic pipeline:
  `Code Changes` ➔ `audiothread-guard` (Real-Time Safety Audit) ➔ `build-validate` (Compiler Sanitizer & Native Tests) ➔ `step-verify` (Checklist & Git Stage) ➔ `task-finish` (Conventional Commit & Plan Archive).
- **Domain Routing Protocol (`TARGET: [FACTORY_FLOOR | TOOLING_HARNESS]`):**
  - **`TARGET: FACTORY_FLOOR`**: Sourced via `plan_to_build.md` ➔ `build_to_plan.md` to Klang Industries (native C++ DSP, JUCE GUI, CMake).
  - **`TARGET: TOOLING_HARNESS`**: Sourced via `subagent_dispatch.md` ➔ `subagent_receipt.md` to Flash Builder Subagent (Python/PowerShell scripts, docs, HTML sidecars, agent skills).
  - *Anti-Snatching Guard*: Klang Industries NEVER ingests tickets marked `TARGET: TOOLING_HARNESS`. Tooling subagents NEVER touch real-time C++ audio rendering loops.
- **Strict Audio-Thread Rules:** Never bypass real-time safety checks (`audiothread-guard`) when modifying DSP code.
- **Single Source of Truth:** Tasks and backlog items must originate exclusively from local workspace files (`PLAN.md` or `docs/BACKLOG.md`). Zero clipboard sniffing.
- **Phase Boundaries:** Complete and verify one phase at a time unless `--all` is explicitly passed.
- **Subagent Factory Delegation (`--subagent` or Research Trigger):** When invoked during an active research/triage session (or when `--subagent` is passed), `execute-task` delegates the pipeline (`Code Edits -> audiothread-guard -> build-validate -> step-verify`) to a dedicated `Factory Implementation Subagent` (`invoke_subagent(Model="flash")`) to protect main context. Escalates to `Model="pro"` on 2 strikes.

---

## Workflow

### 1. Ingestion & Task Targeting
Inspect the invocation arguments:

#### Case A: No Arguments Given (`/execute-task` or `/build-task`)
1. Inspect [`PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md) at project root:
   - If `PLAN.md` contains active unchecked phases (`- [ ]`), extract the active phase, target files, and verification targets.
   - Proceed to **Section 2 (Builder Pause Gate)**.
2. If `PLAN.md` is idle or empty:
   - Parse top prioritized roadmap items from [`docs/BACKLOG.md`](file:///c:/Dev/TheKlangSuite/docs/BACKLOG.md).
   - Present interactive selection via `ask_question`:
     - **Question:** *"No active phase in PLAN.md. Which backlog item would you like to build?"*
     - **Options:** Clickable list of prioritized backlog items.
   - Once selected, arm `PLAN.md` with the feature's architecture spec.

#### Case B: Direct Backlog Item Given (e.g. `/build-task "Item 1"` or `/build-task "Where-Used"`)
- Locate the item in [`docs/BACKLOG.md`](file:///c:/Dev/TheKlangSuite/docs/BACKLOG.md).
- Arm `PLAN.md` and proceed to Section 2.

---

### 2. Builder Pause Gate & Model Advisory
Before executing Phase 1 or calling any editing/compilation tool:
1. Output a concise briefing card with the active phase, target files, and verification criteria.
2. Render the Model Advisory banner at the very bottom of the response.
3. **CRITICAL GUARDRAIL (NO MODAL ON EXECUTION START):** Under NO circumstances invoke `ask_question` here (interactive modals freeze the IDE interface and prevent changing the model dropdown in the IDE footer).
4. Pause in regular chat text:
   > Please verify or adjust your model dropdown in the IDE footer to match the advisory above, then reply **`proceed`** to begin Phase 1.

---

### 3. Automated Pipeline Execution Loop (Per Phase)
When the user replies `proceed`, `engage`, or if `--all` was passed:

1. **Step 1: Code Implementation**
   - Apply edits to the target source files for the phase.
2. **Step 2: Real-Time Audio Audit (`audiothread-guard`)**
   - Run audit on modified DSP files in `source/`.
   - Ensure zero heap allocations, zero blocking locks, zero console I/O, and fast math in hot loops.
3. **Step 3: Compilation & Native Tests (`build-validate`)**
   - Run CMake build for target (`Release`):
     ```powershell
     cmake --build build --config Release --parallel --target <target>
     ```
   - Run `dsp_tests.exe` and `gui_tests.exe`.
   - Enforce the **2-Strike Factory Floor Escalation**: allow up to 2 targeted fix attempts before hard-pausing and requesting user guidance / tier escalation.
4. **Step 4: Check-off & Stage (`step-verify`)**
   - Check off phase criteria in `PLAN.md` (`- [x]`).
   - Stage modified files in git (`git add <files>`).
5. **Step 5: Transition Check**
   - If more phases remain in `PLAN.md`:
     - If `--all` was set: proceed immediately to next phase.
     - Otherwise: present phase verification report and wait for user to reply `proceed` for Phase `[N+1]`.
   - If all phases are complete:
     - Invoke `task-finish` to generate conventional commit, update `build_to_plan.md` and `plan_to_build.md`, and auto-archive `PLAN.md`.
