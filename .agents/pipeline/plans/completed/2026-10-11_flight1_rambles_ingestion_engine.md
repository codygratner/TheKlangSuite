# 🎙️ Flight 1: The "Rambles" Ingestion Engine (P0 — Topic 2)

> **ID:** plan-005  
> **TARGET:** TOOLING_HARNESS (New Klang City) 🚀  
> **Topic Target:** Flight 1 (The "Rambles" Ingestion Engine — Topic 2)  
> **Status:** COMPLETED ✅  
> **Priority:** P0 (Top Priority)  
> **Target Branch:** `0.4.0-dev`  
> **Recommended Setting:** Gemini 3.8 Flash (Thinking: High) — 🟢 SUSTAINABLE  
> **Source Note:** [`C:\Dev\TheKlangVault\Inbox\Archive\Rambles.md`](file:///C:/Dev/TheKlangVault/Inbox/Archive/Rambles.md)  
> **Source Agreements:** Morning Mail Rejiggered Runway & System Sanity Audit Topic 6.11  

---

## 🎯 Architectural Intent & Storyline

Creative musicians and developers don't author structured Jira tickets while inspiration strikes; they dump voice memos, stream-of-consciousness phone notes, and chaotic midnight thoughts into Obsidian (`TheKlangVault/Inbox/`). 

Currently, raw rambling notes require manual interpretation, risking context bloat or premature backlog pollution. The **"Rambles" Ingestion Engine** turns unformatted stream-of-consciousness prose into atomic, structured 3-bullet decision cards:
1. 🎯 **Core Intent**: 1-sentence synthesis of the creative goal.
2. 🛠️ **Proposed Mechanism**: Target repo, hardware, or module architecture (`TKS`, `TT`, `NK`, `TBD-16`).
3. 🚦 **Action Route**: Suggested triage recommendation (`[ 📥 Backlog ]`, `[ 🔬 Research ]`, `[ 🪦 Graveyard ]`, `[ ⚖️ PCDA ]`).

### Strict Safety & Governance Invariants:
- **Zero Premature Mutation**: Extracted cards NEVER directly edit `docs/BACKLOG.md` or `PLAN.md` without an explicit human triage confirmation button.
- **Multi-Idea Decomposition**: A single long ramble discussing multiple distinct topics (e.g. note size vs ingestion pipeline in `Rambles.md`) is intelligently decomposed into separate, atomic cards.
- **Inbox Zero Stewardship**: Processing notes with `--archive` safely moves them into `TheKlangVault/Inbox/Archive/`, preserving full provenance.

---

## 📋 Phase Breakdown

### Phase 1: Specification & Architectural Catalog Registration
- [x] Author `docs/specs/nkai_rambles_ingestion_engine.md`:
  - Document the 3-bullet card format (Core Intent, Proposed Mechanism, Action Route).
  - Document the multi-idea decomposition heuristic for stream-of-consciousness text.
  - Document CLI flags (`--file`, `--inbox`, `--scan-archive`, `--chat`, `--json`, `--archive`, `--add-to-queue`, `--dry-run`).
  - Document triage dispatch integration with N'kai sidecars and `tools/check_mail.py`.
- [x] Register spec in `docs/DOCS_CATALOG.json` under `specifications`.
- [x] Register spec in `docs/SYSTEM_MAP.md` under Section 7 (Design Specs).

### Phase 2: Pydantic Data Models (`tools/models.py`)
- [x] Author `RambleCard` and `RambleExtractionResult` models in `tools/models.py`:
  - `source_note`: Originating file name or identifier.
  - `idea_index`: Integer index for multi-idea notes (1-indexed).
  - `title`: Synthesized crisp title (max 64 chars).
  - `core_intent`: 1-sentence creative goal summary.
  - `proposed_mechanism`: Affected repo, module, or hardware target.
  - `action_route`: Triage action enum (`BACKLOG`, `RESEARCH`, `GRAVEYARD`, `PCDA`).
  - `target_pillar`: Target repo/domain (`TKS`, `TT`, `NK`, `TKR`, `TBD-16`).
  - `tags`: List of searchable tags (`#ramble`, `#dsp`, `#ui`, etc.).
  - `raw_snippet`: Supporting excerpt from the source text.
  - Method `to_mail_item()`: Converts `RambleCard` to a standardized `MailItem` bearing the `[ 🎙️ RAMBLE ]` badge and formatted body for instant consumption in `morning_mail_inbox.html`.

### Phase 3: Zero-Dependency Ramble Ingestor CLI (`tools/ramble_ingest.py`)
- [x] Implement `tools/ramble_ingest.py`:
  - Standard Python library only + `tools/models.py` (Pydantic v2).
  - Heuristic-driven stream-of-consciousness parser:
    - Splits text on paragraph breaks, rhetorical transition phrases ("also", "another thought", "what about", "on the other hand"), and numbered lists.
    - Categorizes target pillar (`TKS`, `TT`, `NK`, `TKR`, `TBD-16`) based on domain keyword heuristics.
    - Determines suggested action route based on question vs concrete proposal cues.
  - CLI capabilities:
    - `--file <path>`: Ingest specific markdown ramble.
    - `--inbox`: Scan all notes in `TheKlangVault/Inbox/`.
    - `--scan-archive`: Scan notes in `TheKlangVault/Inbox/Archive/`.
    - `--chat` / `--text-only`: Output clean Markdown cards directly in console.
    - `--json`: Output serialized JSON.
    - `--add-to-queue`: Append extracted cards to `.agents/pipeline/mailbag/mail_queue.json`.
    - `--archive`: Move processed notes from `Inbox/` to `Inbox/Archive/`.
    - `--dry-run`: Preview extraction without disk modifications.
- [x] Mirror `tools/ramble_ingest.py` to `c:\Dev\nkai\tools\ramble_ingest.py` and `c:\Dev\nkai\bundle\tools\ramble_ingest.py`.

### Phase 4: Micro-Skill Registration (`.agents/skills/ramble-ingest/SKILL.md`)
- [x] Author `c:\Dev\TheKlangSuite\.agents\skills\ramble-ingest\SKILL.md`:
  - Name: `ramble-ingest` (triggers `/ramble`, `/ramble-ingest`, `/ingest-ramble`, "ingest ramble", "ramble ingest").
  - Clear usage guidance, flag references, and output format examples.
- [x] Mirror skill to `c:\Dev\nkai\bundle\skills\ramble-ingest\SKILL.md`.

### Phase 5: Verification Gauntlet & Multi-Repo Commits
- [x] Run `python tools/ramble_ingest.py --file "C:\Dev\TheKlangVault\Inbox\Archive\Rambles.md" --chat`:
  - Assert that both ideas (Ramble Ingestion Workflow + Note Granularity) are decomposed into crisp 3-bullet cards.
- [x] Run test on complex note: `python tools/ramble_ingest.py --file "C:\Dev\TheKlangVault\Inbox\Archive\Effects and Oscillators and More.md" --chat`.
- [x] Run `python tools/verify_context_clues.py` in `c:\Dev\TheKlangSuite` (confirm 100% link resolution).
- [x] Update `docs/BACKLOG.md`: Mark Flight 1 (Topic 2: Rambles Ingestion Engine) as completed.
- [x] In `PLAN.md`: Mark all phases completed `[x]` and update Status to `COMPLETED ✅`.
- [x] Append execution report to `.agents/pipeline/communique/subagent_receipt.md`.
- [x] Commit changes cleanly in `c:\Dev\TheKlangSuite` and `c:\Dev\nkai`.
