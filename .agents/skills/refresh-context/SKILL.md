---
name: refresh-context
description: Executes the Rolling Blackbox Context Compaction flush (harvests to docs/history/DEV_HISTORY.md, updates context clues, stages souls, runs link verification), and readies /clear. Triggers on `/refresh-context`.
---

# Rolling Blackbox Context Refresh Workflow

## 1. Scope & Purpose
The `/refresh-context` workflow executes the **4-Step Pre-Compaction Memory Flush** to guarantee that architectural state, flight progress, and key decisions are permanently recorded before chat context compaction or truncation.

## 2. The Automated 4-Step Pre-Compaction Flush

The entire memory flush protocol is automated via `tools/flush_memory.py` at zero token cost:
```powershell
python tools/flush_memory.py --role "[Role]" --summary "[Objective]"
```

### What It Executes Mechanically:
1. **Dev History Ingestion**: Appends structured summary, recent git commits, and active plan target to [`docs/history/DEV_HISTORY.md`](file:///c:/Dev/TheKlangSuite/docs/history/DEV_HISTORY.md).
2. **Soul Harvest Staging Audit**: Scans `.agents/pipeline/staging/souls/` for pending soul captures.
3. **Workspace State Synchronization**: Runs `tools/state_manager.py` to regenerate [`.agents/pipeline/context_state.json`](file:///c:/Dev/TheKlangSuite/.agents/pipeline/context_state.json) and [`ACTIVE_CONTEXT.md`](file:///c:/Dev/TheKlangSuite/ACTIVE_CONTEXT.md) (<600 bytes).
4. **Workspace State & Link Verification**: Audits that communiqués, active targets, and links are 100% consistent and intact.

*(Optional)* To archive a completed plan simultaneously:
```powershell
python tools/flush_memory.py --role "[Role]" --summary "[Objective]" --archive-plan <slug>
```

---

## 3. The 0-Turn Cold Boot Invariant
Following `/refresh-context` and `/clear`, when a fresh session boots (or if the user says "resume" / "continue"):
The agent MUST read [`ACTIVE_CONTEXT.md`](file:///c:/Dev/TheKlangSuite/ACTIVE_CONTEXT.md) on Turn 1 and immediately resume operations in **0 turns**, with zero orientation questions or repetitive prompts.
