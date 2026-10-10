# Implementation Plan: Persistent Mailbag Session, Dynamic Priority Queue, and Multi-Tier Rollback Architecture

> **TARGET:** TOOLING_HARNESS (New Klang City) 🚀  
> **Topic Target:** Topic 6.11 (Check the Mail Harvester), Topic 6.16 (Data Persistence), & System-Wide Rollback Architecture  
> **Status:** DRAFT / STAGED (Awaiting Weekly Allowance Refresh) ⏳  
> **Target Branch:** `0.4.0-dev`  
> **Recommended Model:** Gemini 3.8 Flash (Thinking: High) — 🟢 SUSTAINABLE  
> **Execution Strategy:** Delegated to Tier 2 Flash Builder Subagent via `subagent_dispatch.md` once weekly allowance refreshes  

---

## 🎯 Architectural Intent & PCDA Synthesis

### The Core Problem
1. **Ephemeral Mailbox Scraping vs. Stateful Triage**:
   Currently, `tools/check_mail.py` scans `C:\Dev\TheKlangVault\Inbox\` on demand. When notes are archived (`C:\Dev\TheKlangVault\Inbox\Archive\`), their status inside the Mailbag is lost unless re-scanned with `--archive`. Moving files in Obsidian physically mutates triage state, creating cognitive friction.
2. **Reverse-Chronological Urgency vs. Topological Sequence**:
   Freshly captured high-priority notes (`P0` / `TOP PRIORITY`) need immediate visibility at the top of the Mailbag, but sometimes depend on lower-level unbuilt prerequisites. If an agent naively acts on them out of order, architectural chaos ensues.
3. **Accidental Mutation & Zero-Token Recovery**:
   When files are archived, plans promoted, or itineraries re-sequenced, there is currently no 1-command atomic rollback. Recovering from an accidental action requires manually moving files or re-running LLM prompts, wasting precious weekly token allowance.

---

### PCDA Dialectic Breakdown

```
                    ┌──────────────────────────────────────────────┐
                    │ THESIS (Pros)                                │
                    │ - Instant stateful resume across sessions     │
                    │ - Zero-token 5ms atomic rollbacks            │
                    │ - Clear Mailbox (files) vs Mailbag (session) │
                    └──────────────────────┬───────────────────────┘
                                           │
                                           ▼
                    ┌──────────────────────────────────────────────┐
                    │ THESIS FRICTION (Cons)                       │
                    │ - Extra JSON ledger files on disk            │
                    │ - Potential state desync if files deleted    │
                    └──────────────────────┬───────────────────────┘
                                           │
                                           ▼
                    ┌──────────────────────────────────────────────┐
                    │ ANTITHESIS (Devil's Advocate)                │
                    │ - "SQLite or full database is over-kill"     │
                    │ - Risk of building a brittle state machine   │
                    │ - Token burn during recovery                 │
                    └──────────────────────┬───────────────────────┘
                                           │
                                           ▼
                    ┌──────────────────────────────────────────────┐
                    │ SYNTHESIS (The 80/20 Resolution)             │
                    │ - Plain JSON ledgers + immutable receipts    │
                    │ - Non-blocking topological suggestion chips  │
                    │ - Zero-dependency Python CLI rollback hooks   │
                    └──────────────────────────────────────────────┘
```

1. **Pros [Thesis]**:
   - Decouples physical file movement from triage review state. Notes can be cleanly archived in Obsidian without disappearing from active N'kai review.
   - Provides instant 5ms atomic rollback (`--rollback`) restoring physical files and session state without calling LLMs.
   - Dynamic P0 sorting ensures critical ideas never get lost in a 30-item inbox.
2. **Cons [Thesis Friction]**:
   - Introduces `.agents/pipeline/mailbag/mail_queue.json` and `.agents/pipeline/plans/queue.json` schema maintenance.
   - If files are deleted externally in Obsidian, the queue ledger must gracefully prune orphaned records.
3. **Devil's Advocate [Antithesis]**:
   - Are we building an operating system instead of an audio suite? A heavy database (SQLite) or complex event-sourcing engine would be classic developer procrastinatory over-engineering.
4. **The 80/20 Resolution [Synthesis]**:
   - Lightweight, human-readable JSON ledgers (`mail_queue.json`, `queue.json`).
   - Atomic pre-transaction snapshots (`mail_tx_latest.json`, `itinerary_snapshot_prev.json`, `plan_snapshot_prev.json`).
   - Pure Python standard library with zero external dependencies.

---

## 📑 Data Contracts & Schemas

### 1. Persistent Mailbag Queue Schema (`.agents/pipeline/mailbag/mail_queue.json`)
```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "version": "1.0.0",
  "last_synced": "2026-10-10T17:30:00Z",
  "active_session_id": "session-2026-10-10-001",
  "items": [
    {
      "id": "mail-7a8b9c",
      "source_file": "Mailbag Queue.md",
      "relative_path": "Inbox/Mailbag Queue.md",
      "title": "Mailbag Queue & Rollback Architecture",
      "date_captured": "2026-10-10",
      "priority": "P0",
      "status": "APPROVED",
      "destination": "BACKLOG",
      "prerequisites": ["TIK-4", "Topic-6.16"],
      "token_tier": "Tier 2",
      "estimated_tokens": 10000,
      "archived_to": "Inbox/Archive/Mailbag Queue.md"
    }
  ]
}
```

### 2. Transaction Receipt Schema (`.agents/pipeline/staging/mail_tx_latest.json`)
```json
{
  "transaction_id": "tx-20261010-173000",
  "timestamp": "2026-10-10T17:30:00Z",
  "command": "check_mail.py --archive",
  "actions": [
    {
      "type": "MOVE_FILE",
      "source": "C:\\Dev\\TheKlangVault\\Inbox\\Mailbag Queue.md",
      "destination": "C:\\Dev\\TheKlangVault\\Inbox\\Archive\\Mailbag Queue.md"
    },
    {
      "type": "UPDATE_LEDGER",
      "ledger": ".agents/pipeline/mailbag/mail_queue.json",
      "item_id": "mail-7a8b9c",
      "previous_status": "PENDING",
      "new_status": "APPROVED"
    }
  ]
}
```

### 3. Persistent Plans Queue Schema (`.agents/pipeline/plans/queue.json`)
```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "version": "1.0.0",
  "active_plan": "persistent_mailbag_queue_and_rollback_architecture.md",
  "active_target": "TOOLING_HARNESS",
  "execution_pointer": 0,
  "queue": [
    {
      "plan_file": ".agents/pipeline/plans/drafts/persistent_mailbag_queue_and_rollback_architecture.md",
      "title": "Persistent Mailbag Session & Rollback Architecture",
      "priority": "P0",
      "target": "TOOLING_HARNESS",
      "status": "QUEUED",
      "created_at": "2026-10-10T17:30:00Z"
    },
    {
      "plan_file": ".agents/pipeline/plans/drafts/post_milestone_deep_docs_gauntlet.md",
      "title": "Post-Milestone Deep Docs & Field Guides Separation",
      "priority": "P1",
      "target": "TOOLING_HARNESS",
      "status": "QUEUED",
      "created_at": "2026-10-09T18:00:00Z"
    }
  ],
  "history": [
    {
      "plan_file": ".agents/pipeline/plans/completed/2026-10-09_Flight8_Cumulative_Allowance_Ledger.md",
      "completed_at": "2026-10-09T17:50:00Z",
      "status": "COMPLETED"
    }
  ]
}
```

---

## 📋 Phase-by-Phase Execution Plan

### Phase 1: Glossary Formalization & Documentation Sync
- [ ] Verify Section 7 in `docs/GLOSSARY.md` (Mailbox vs. Mailbag, Queue & Rollback terminology).
- [ ] Author `docs/specs/nkai_mailbag_queue_and_rollback_architecture.md` detailing the persistence protocol and transaction rollback invariants.
- [ ] Register spec in `docs/DOCS_CATALOG.json` under `specifications`.
- [ ] Register spec in `docs/SYSTEM_MAP.md` under Section 7 (Design Specs).

### Phase 2: Persistent Mailbag Session Manager in `tools/check_mail.py`
- [ ] Implement `MailbagSession` class in `tools/check_mail.py`:
  - `load_queue(queue_path)`: Loads `.agents/pipeline/mailbag/mail_queue.json` (auto-creates if missing).
  - `sync_with_inbox(inbox_dir, archive_dir)`: Detects new notes in `TheKlangVault/Inbox/`, computes content hash / UUID, registers new entries, and reconciles moved files.
  - `save_queue(queue_path)`: Atomically writes formatted JSON with timestamp.
  - Graceful orphan handler: If a note was deleted outside the system, marks status as `ORPHANED` rather than crashing.
- [ ] Add CLI flag `--session`: Launches the Mailbag with persistent state rather than purely scanning raw directory mtimes.
- [ ] Sync `tools/check_mail.py` with `c:\Dev\nkai\tools\check_mail.py`.

### Phase 3: Dynamic Reverse-Chronological P0 Sorting & Topological Suggestion Chips
- [ ] Update sorting algorithm in `tools/check_mail.py`:
  - Primary sort key: Priority (`P0` > `P1` > `P2` > `P3`).
  - Secondary sort key for `P0`: Reverse-chronological capture timestamp (newest notes first).
- [ ] Implement Topological Intercept logic:
  - If a `P0` note declares or is analyzed to have unmet prerequisites in `mail_queue.json` (e.g. `prerequisites: ["TIK-4"]` where `TIK-4` is pending), inject a dynamic banner / suggestion chip in the card:
    ```html
    <div class="topological-warning-chip">
      <span class="chip-icon">💡</span>
      <span class="chip-text">Dependency Detected: Prerequisite <strong>TIK-4</strong> is pending.</span>
      <div class="chip-actions">
        <button class="chip-btn chip-btn-reorder" onclick="resequenceMailItem('mail-7a8b9c')">🔄 Slot into Sequence</button>
        <button class="chip-btn chip-btn-keep" onclick="dismissDependencyAlert('mail-7a8b9c')">⚡ Keep at Top</button>
      </div>
    </div>
    ```
- [ ] Update CSS in `c:\Dev\nkai\core\sidecar.css` and `c:\Dev\TheKlangSuite\.agents\sidecar\morning_mail_inbox.html` to style `.topological-warning-chip`.

### Phase 4: Stateless Atomic Rollback Implementation (`--rollback`)
- [ ] Implement Atomic Transaction Receipts in `tools/check_mail.py`:
  - When archiving notes (`--archive-all` or individual item archival), write `.agents/pipeline/staging/mail_tx_latest.json` BEFORE executing file operations.
  - Implement `tools/check_mail.py --rollback`:
    - Reads `.agents/pipeline/staging/mail_tx_latest.json`.
    - Inverts each action (`MOVE_FILE` back to original location, reverts ledger status).
    - Prints instant 5ms recovery summary to terminal.
- [ ] Implement Itinerary Snapshot Rollback in `tools/audit_reorder.py`:
  - Before applying a re-ordered flight sequence, writes `.agents/pipeline/staging/itinerary_snapshot_prev.json`.
  - Supports `tools/audit_reorder.py --rollback` to immediately restore previous flight ordering.

### Phase 5: Persistent Plans Queue & Plan Rollback Architecture
- [ ] Create `.agents/pipeline/plans/queue.json` initializing active plan pointers.
- [ ] Enhance `.agents/skills/clean-plan/SKILL.md` and `tools/clean_plan.py` (or PowerShell wrapper):
  - When archiving or replacing `PLAN.md`, snapshot the file to `.agents/pipeline/plans/snapshots/PLAN_PREV.md`.
  - Update `queue.json` state transitions (`QUEUED` $\to$ `IN_PROGRESS` $\to$ `COMPLETED`).
  - Add `/rollback-plan` or `clean-plan --rollback`:
    - Restores `.agents/pipeline/plans/snapshots/PLAN_PREV.md` back to `PLAN.md`.
    - Updates `queue.json` resetting the pointer to the previous active plan.
    - Zero token burn, 100% deterministic local recovery.

### Phase 6: Cross-Skill Rollback Matrix (Staged Discussion & Backlog Cataloging)
- [ ] Document future rollback paths for remaining agent skills:
  - `execute-task`: Git checkout / stash rollback on 2-strike factory build failure.
  - `step-verify`: Git un-stage checkpoint rollback if phase acceptance criteria fail.
  - `update-docs`: Documentation catalog snapshot restoration if link integrity fails.
  - `cut-release`: Pre-release tag and staging branch abort.
- [ ] Run `python tools/audit_sidecar.py` across updated templates.
- [ ] Run `python tools/verify_context_clues.py` in `c:\Dev\TheKlangSuite`.

---

## 🧪 Verification & Acceptance Gauntlet

1. **Transaction Integrity Test**:
   - Run `python tools/check_mail.py --archive-note "Test Note.md"`.
   - Verify note moved to `Archive/` and `mail_tx_latest.json` created.
   - Run `python tools/check_mail.py --rollback`.
   - Assert note returned to `TheKlangVault/Inbox/` with 100% bitwise parity.
2. **Persistent Queue Idempotency Test**:
   - Load `morning_mail_inbox.html` in browser.
   - Change a status badge.
   - Re-run `check_mail.py --session`.
   - Assert modified status badge is preserved from `mail_queue.json`.
3. **Plan Rollback Test**:
   - Snapshot `PLAN.md`.
   - Overwrite `PLAN.md` with dummy content.
   - Run plan rollback command.
   - Assert original `PLAN.md` restored.
4. **Sidecar Audit**:
   - Run `python tools/audit_sidecar.py .agents/sidecar/morning_mail_inbox.html`.
   - Assert 100% tag balance and zero console syntax errors.
