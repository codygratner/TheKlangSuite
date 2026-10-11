---
name: refresh-context
description: Executes the Rolling Blackbox Context Compaction flush (harvests to docs/history/DEV_HISTORY.md, updates context clues, stages souls, runs link verification), and readies /clear. Triggers on `/refresh-context`.
---

# Rolling Blackbox Context Refresh Workflow

## 1. Scope & Purpose
The `/refresh-context` workflow executes the **4-Step Pre-Compaction Memory Flush** to guarantee that architectural state, flight progress, and key decisions are permanently recorded before chat context compaction or truncation.

## 2. The 4-Step Pre-Compaction Flush Protocol

### Step 1: Dev History Ingestion
Append a structured summary of the session's accomplishments to [`docs/history/DEV_HISTORY.md`](file:///c:/Dev/TheKlangSuite/docs/history/DEV_HISTORY.md) using the canonical layout:
```markdown
### Session: YYYY-MM-DD HH:MM (<conversation_id>)
- **Chat Role:** [Strategic Planning (New Klang City) | Factory Floor (Klang Industries) | Tooling Builder]
- **Primary Objectives:** <Summary of tasks tackled>
- **Key Decisions & Deliverables:** <Concrete deliverables, architectural breakthroughs, files touched>
```

### Step 2: Soul Harvest Staging
If any non-trivial platform gotchas, bug autopsies, or novel PCDA dialectic resolutions occurred during the session, stage a raw capture file into `.agents/pipeline/staging/souls/<YYYY-MM-DD>_<slug>.md`.

### Step 3: Context Clues Hardening
Update the role's context clues reference at the workspace root:
- For Planning Chats (New Klang City): update [`context_clues_plan.md`](file:///c:/Dev/TheKlangSuite/context_clues_plan.md).
- For Factory Chats (Klang Industries): update [`context_clues_build.md`](file:///c:/Dev/TheKlangSuite/context_clues_build.md).
- For Subagents: update [`context_clues_subagent.md`](file:///c:/Dev/TheKlangSuite/context_clues_subagent.md).

Ensure the following 4 sections are completely accurate:
1. **Active SemVer & Branch State** (branch name, active milestone).
2. **Immediate Return Agenda & Status** (exact current flight/phase and immediate next command).
3. **Architectural Memory & "Unforgettables"** (critical invariants).
4. **Active Specs & Links**.

### Step 4: Link Integrity Verification
Execute `python tools/verify_context_clues.py` to assert 100% link resolution and 0 broken anchors.

---

## 3. The 0-Turn Cold Boot Invariant
Following `/refresh-context` and `/clear`, when the fresh session starts (or if the user says "resume" / "continue"):
The agent MUST read the appropriate `context_clues_*.md` file on Turn 1 and immediately resume operations in **0 turns**, with zero orientation questions or repetitive prompts.
