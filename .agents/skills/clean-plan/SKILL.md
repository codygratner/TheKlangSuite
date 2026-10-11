---
name: clean-plan
description: Cleans up temporary planning artifacts (context_snapshot*.md, PLAN_BACKUP*.md) and archives completed PLAN.md files to .agents/pipeline/plans/completed/ to keep the workspace clean. Triggers on `/cleanplan`, `/clean-plan`, `/snapshot-clean`, `/cleansnapshot`, or "clean plan".
---

# Planning Hygiene & Snapshot Cleaner

## Goal
Purge temporary planning snapshots (`context_snapshot*.md`) and scratchpad backups, and archive completed `PLAN.md` files into `.agents/pipeline/plans/completed/` to maintain a pristine, clutter-free repository root.

## Operational Constraints
- **Preserve Unfinished Work:** Do not delete an uncompleted `PLAN.md` without explicit user confirmation.
- **Permanent Archival:** Always archive completed plans to `.agents/pipeline/plans/completed/` rather than hard-deleting them so historical engineering context is preserved.

## Workflow

### 1. Detect Planning Artifacts
Inspect the project root for planning files:
- `context_snapshot*.md`
- `PLAN_BACKUP*.md`
- `PLAN.md`

### 2. Purge Temporary Snapshots & Backups
If temporary snapshots or backup files exist, delete them:
```powershell
Remove-Item -Path "context_snapshot*.md", "PLAN_BACKUP*.md" -Force -ErrorAction SilentlyContinue
```

### 3. Archive or Clean `PLAN.md`
If `PLAN.md` exists at the project root:

1. **Check Completion Status:**
   - Scan for unchecked checklist items (`- [ ]`).
   - If unchecked items exist:
     - If invoked manually with an explicit intent to reset, prompt the user:
       > *"PLAN.md still contains unchecked items. Would you like to archive it anyway or keep it in place?"*
     - If invoked as part of a post-task cleanup after all phases passed: proceed to archive.

2. **Archive Completed Plan:**
   - Extract the plan title and ticket identifier (e.g., `THE-6`, `THE-7`, or the primary heading).
   - Get the current date in `YYYY-MM-DD` format.
   - Target directory: `.agents/pipeline/plans/completed/` (create if it does not exist).
   - Copy or move `PLAN.md` to:
     `.agents/pipeline/plans/completed/<YYYY-MM-DD>_<IDENTIFIER_OR_TITLE>.md`
   - Reset `PLAN.md` at project root:
     ```powershell
     Set-Content -Path "PLAN.md" -Value "# No Active Plan`n"
     ```
   <!-- [Strategy Experiment: Multi-Chat Communiqué Awareness] -->
   - **Communiqué Sync (If Present):** If `.agents/pipeline/communique/` exists:
     - Update `.agents/pipeline/communique/build_to_plan.md` setting `Status: COMPLETE ✅` with link to archived plan and timestamp.
     - Update `.agents/pipeline/communique/plan_to_build.md` setting `Status: COMPLETED ✅`.
   - **Vault Inbox Archival (If Applicable):** If `PLAN.md` references an inbox note (`Source Note: <note_name>.md`) or an associated note exists in `C:\Dev\TheKlangVault\Inbox\`:
     ```powershell
     powershell -ExecutionPolicy Bypass -File .\tools\sync_obsidian_vault.ps1 -ArchiveNotes "<note_name>"
     ```

3. **Promote Next Queued Plan (Optional / Pipeline Next):**
   - If a multi-plan queue is active in `.agents/pipeline/plans/queue.json`, the user or agent can automatically advance the queue to promote the next queued draft into `PLAN.md`:
     ```powershell
     python tools/plan_queue.py promote-next
     ```
   - This snapshots `PLAN.md` to `.agents/pipeline/plans/snapshots/PLAN_PREV.md`, advances the execution pointer in `queue.json`, and loads the next blueprint into root `PLAN.md`.

### 4. Output Cleanliness Report
Display a concise summary:

```markdown
### 🧹 Workspace Cleaned

- **Temporary Snapshots Removed:** `context_snapshot.md` deleted.
- **Plan Archival:** `PLAN.md` archived to [`.agents/pipeline/plans/completed/<archive_name>.md`](file:///.agents/pipeline/plans/completed/<archive_name>.md).
- **Repository Root:** Clean (zero untracked scratchpads).
```

