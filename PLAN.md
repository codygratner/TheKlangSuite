# Plan: rj-46 — Cross-Ecosystem Unfinished Business Audit

**TARGET:** TOOLING_HARNESS  
**Role:** Systems & Tooling Auditor  
**Recommended Model:** Gemini 3.8 Flash (Thinking: High) — Tier 2 (Balanced Engineering & UI)  
> **Status:** COMPLETED ✅  

---

### Executive Overview & Objective
Conduct a comprehensive, zero-speculation audit across all 5 repositories (`TheKlangSuite`, `nkai`, `ToadTracker`, `PCDACore`, `TheKlangVault`) to catalog every in-progress feature, unbuilt plan phase, stranded quick-note, git stash, and open backlog item into a single actionable immediate "to-do now" checklist.

---

### Phase 1: Multi-Repository Git & Working Tree Health Scan
- [x] Scan `git status -s`, uncommitted modifications, and `git stash list` across:
  - `c:\Dev\TheKlangSuite` (audio engine, DSP, tools)
  - `c:\Dev\nkai` (sidecar UI, templates, server)
  - `c:\Dev\ToadTracker` (SteamOS tracker)
  - `c:\Dev\PCDACore` (standalone package)
  - `C:\_\Vault` (Obsidian knowledge base)
- [x] Identify any uncommitted work, unmerged stashes, or diverged branches.
  - Result: All repos clean; ToadTracker `tools/bus.py` synced and committed (`214ba03`); 0 stashes.

### Phase 2: Active Plans & Pipeline Staging Audit
- [x] Inspect `.agents/pipeline/` across repos:
  - Staged souls (`.agents/pipeline/staging/souls/`): 2 souls held for Act III.
  - Staged inbox notes (`C:\_\Vault\Inbox\Staging/`): 8 shipped notes resolved by `rj-45`.
  - Tabled notes (`C:\_\Vault\Inbox\Tabled/`): 1 note (`Live_Reading_Notes_Protocol.md` / `rj-43`).
  - Bus dispatches: Found 1 unacknowledged dispatch (`TKS ➔ TT`); acknowledged. All 14 dispatches clear.
- [x] Check active and completed plans in `.agents/pipeline/plans/`:
  - Archived 4 obsolete drafts from `drafts/` to `completed/`. 4 active drafts remain.

### Phase 3: Backlog Reconcile & Cross-Repo Open Loops
- [x] Audit `docs/BACKLOG.md` across `TheKlangSuite` and `ToadTracker`:
  - TKS: Act III flights 0, 1–8, 14, 20 completed; Flight 21 frozen until Sunday quota reset; 4 queued feature flights (M8 bridge, Component presets, Snap heap, CyDrums).
  - ToadTracker: 6 prioritized ready features in root `BACKLOG.md` (`BACKLOG-01` Project Screen is next).
- [x] Detect orphaned or stranded quick-notes across `QuickNotes/`:
  - 5 synthetic test notes accounted for and closed by `rj-45`.

### Phase 4: Consolidated "Unfinished Business Docket" Report
- [x] Synthesize findings into a single structured report:
  - Published to `docs/history/UNFINISHED_BUSINESS_AUDIT.md`.
  - Dispatches acknowledged and pipeline drafts cleaned up.
