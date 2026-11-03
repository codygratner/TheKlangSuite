---
name: step-verify
description: Enforces incremental engineering discipline between phases of PLAN.md. Audits code changes against active phase acceptance criteria, updates checklist checkmarks in PLAN.md, stages phase modifications in git, and pauses for user review before advancing. Triggers on `/stepverify`, `/step-verify`, "verify step", "verify phase", "next step", "progress check", or "audit phase".
---

# Step Verifier & Milestone Progression Check

## Goal
Audit changes made during the active phase of an implementation plan against its documented acceptance criteria in `PLAN.md`. Upon successful verification, update `PLAN.md` checkmarks, stage git changes for the completed milestone, and present a concise summary before proceeding to the subsequent phase.

## Operational Constraints
- **Do Not Skip Phases:** Phases must be executed and verified sequentially unless explicitly directed by the user.
- **Strict Verification Before Checkoff:** Never check off `[x]` in `PLAN.md` without verifying that the corresponding code changes and tests exist on disk.
- **Stage Progress Incrementally:** Stage changes milestone-by-milestone to maintain clean git diff boundaries.

## Workflow

### 1. Inspect Communiqué Mailbox for Mid-Flight Amendments
1. Open `.agents/pipeline/communique/plan_to_build.md`.
2. Check if `STATUS:` is `PLAN_AMENDED ⚠️`.
3. If amended: read the amendment changelog carefully, reset `plan_to_build.md` back to `STATUS: IN_PROGRESS` to acknowledge receipt, and ensure the updated `PLAN.md` is strictly followed.

### 2. Identify Active Phase in `PLAN.md`
1. Open and parse `PLAN.md` at the project root.
2. Locate the first phase containing unchecked boxes (`- [ ]`).
3. Extract:
   - Phase Title and Scope.
   - List of acceptance criteria and deliverables.
   - Target files and test requirements for this phase.

### 2. Audit Code Changes Against Phase Criteria
1. Inspect git modifications:
   ```powershell
   git status --short
   git diff --name-only
   ```
2. For each deliverable in the active phase:
   - Check if the targeted files contain the required implementation (classes, functions, macros, or configurations).
   - If audio code was touched, check for audio thread safety (or invoke `/audiothread-guard`).
   - If unit tests were specified, check that test cases exist in `test/dsp_tests.cpp` or equivalent test runners.

### 3. Run Targeted Phase Tests (When Build is Authorized)
If compilation/testing has been authorized by the user for this step:
- Run targeted tests for the phase (e.g. `dsp_tests` or specific test cases).
- If tests fail, halt progression and report the failure details. Do not mark the phase as complete.

### 4. Advance Phase & Check Off in `PLAN.md` (Mechanical Automation)
Execute the deterministic phase advancement CLI in `<5ms`:
```powershell
python tools/plan_manager.py advance
```
*What it executes automatically:*
- Flips `- [ ]` checkboxes to `- [x]` for the active phase in `PLAN.md` without LLM file clobbering.
- Automatically recalculates progress header (e.g. `Progress: [======>  ] 75%`).
- Updates `.agents/pipeline/communique/factory_receipt.md` (or `build_to_plan.md`) to `## Status: BUILDING 🔨 (Phase <N+1>)`.
- Saves ~3,500 tokens and 4 manual file read/edit tool calls!

### 5. Stage Git Changes for Completed Milestone
Stage the files modified during this phase:
```powershell
git add <file1> <file2> ...
```
*Note: Do not commit yet unless requested; staging establishes clean milestone checkpoints.*

### 6. Present Milestone Verification Card
Output a structured progression report to the user:

```markdown
### 🏁 Milestone Verified: Phase <N> — <Phase Name>

**Deliverables Checked:**
- [x] <Deliverable 1 summary>
- [x] <Deliverable 2 summary>

**Verification Findings:**
- **Code Audit:** Verified required symbols and data structures in [`source/FileName.cpp`](file:///path/to/source/FileName.cpp).
- **Tests:** <Test execution status or test case addition confirmation>.
- **Git Status:** Staged <N> modified files for Phase <N>.

---

#### ⏭️ Next Up: Phase <N+1> — <Next Phase Name>
- **Objective:** <Brief summary of what Phase N+1 covers>.
- **Target Files:** [`source/NextFile.h`](file:///path/to/source/NextFile.h)

*Ready to proceed to Phase <N+1>? Say "proceed" or provide any adjustments.*
```

### 8. Verification Progression & Handoff (SOCEP Hyperdrive v3.1)
- If all phase criteria verify cleanly, advance phase deterministically via `python tools/plan_manager.py advance`.
- If any deliverable or unit test assertion fails, return status `VERIFICATION_FAILED` to `execute-task` / `build-validate` to engage the SOCEP escalation ladder.
- **Strict Invariant**: `step-verify` NEVER invokes Claude directly and does not pause for manual model dropdown switches.
