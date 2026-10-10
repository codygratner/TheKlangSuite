# Implementation Plan: Multi-Plan Queue, Vault Studio Hub, Pydantic Infrastructure & TKPL

> **TARGET:** TOOLING_HARNESS (New Klang City) 🚀  
> **Topic Target:** Multi-Plan Queue, Topic 6.11 (Mailbag & Studio Hub), Topic 6.16 (Data Persistence), TKPL & Local FastAPI Bridge  
> **Status:** READY_FOR_EXECUTION (Awaiting Weekly Allowance Refresh Kickoff) ⏳  
> **Target Branch:** `0.4.0-dev`  
> **Recommended Model:** Gemini 3.8 Flash (Thinking: High) — 🟢 SUSTAINABLE  
> **Execution Strategy:** Delegated to Tier 2 Flash Builder Subagent via `subagent_dispatch.md` once weekly allowance refreshes  

---

## 🎯 Architectural Intent & PCDA Synthesis

### The Core Vision
This master implementation plan establishes an enterprise-grade, multi-repository tooling and data infrastructure for The Klang Suite ecosystem, solving five foundational needs:
1. **The Multi-Plan Queue & Rollback Engine**: Implemented *first*, creating an automated, stateful queue (`.agents/pipeline/plans/queue.json`) and 0-token rollback mechanism so plans can be queued, tracked, and reverted safely without risk of accidental clobbering.
2. **The Klang Vault as a Private Studio Hub**: Establishes `TheKlangVault` as an independent, private Git-tracked studio repository (`git init` + `.gitignore` protecting Obsidian sync cache) housing interoffice communication, raw mobile thoughts, and triage state away from the C++ audio factory.
3. **Global Pydantic Data Infrastructure**: Leverages a global Python install (`pip install pydantic`) to eliminate virtualenv friction, providing runtime data validation, automatic JSON schema generation, and workplace-ready muscle memory.
4. **TKPL (The Klang Preset Linter)**: A standalone Pydantic CLI validating `assets/presets/*.json` against APVTS parameter contracts and DSP ranges.
5. **N'kai Local FastAPI Sidecar Bridge (`localhost:8000`)**: A high-performance, lightweight microservice bridge replacing the temporary loopback server with production-grade FastAPI endpoints and Pydantic request models.

---

### PCDA Dialectic Breakdown

```
                    ┌──────────────────────────────────────────────┐
                    │ THESIS (Pros)                                │
                    │ - First-class Multi-Plan Queue management    │
                    │ - C++ repo stays 100% pure audio code        │
                    │ - Zero venv friction via global Pydantic     │
                    │ - High-value workplace resume talking points │
                    └──────────────────────┬───────────────────────┘
                                           │
                                           ▼
                    ┌──────────────────────────────────────────────┐
                    │ THESIS FRICTION (Cons)                       │
                    │ - 3 new Pydantic touchpoints (Mail, TKPL, API)│
                    │ - Multi-repo workspace context coordination  │
                    └──────────────────────┬───────────────────────┘
                                           │
                                           ▼
                    ┌──────────────────────────────────────────────┐
                    │ ANTITHESIS (Devil's Advocate)                │
                    │ - "Over-engineering the studio tooling"      │
                    │ - Procrastinating on v0.4.0 C++ UI release   │
                    └──────────────────────┬───────────────────────┘
                                           │
                                           ▼
                    ┌──────────────────────────────────────────────┐
                    │ SYNTHESIS (The 80/20 Resolution)             │
                    │ - Build Multi-Plan Queue FIRST to govern all │
                    │ - Global pip install pydantic (0 setup lag)  │
                    │ - Clean separation: TKS (C++) vs TKV (Python)│
                    └──────────────────────────────────────────────┘
```

---

## 📑 Data Contracts & Schemas

### 1. Multi-Plan Queue Schema (`.agents/pipeline/plans/queue.json`)
```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "version": "1.0.0",
  "active_plan": "PLAN.md",
  "active_target": "TOOLING_HARNESS",
  "execution_pointer": 0,
  "queue": [
    {
      "id": "plan-001",
      "title": "Multi-Plan Queue, Vault Studio Hub & Pydantic Infrastructure",
      "plan_file": "PLAN.md",
      "target": "TOOLING_HARNESS",
      "status": "IN_PROGRESS",
      "priority": "P0",
      "created_at": "2026-10-10T18:00:00Z"
    },
    {
      "id": "plan-002",
      "title": "Post-Milestone Deep Docs & Field Guides Separation Gauntlet",
      "plan_file": ".agents/pipeline/plans/drafts/post_milestone_deep_docs_gauntlet.md",
      "target": "TOOLING_HARNESS",
      "status": "QUEUED",
      "priority": "P1",
      "created_at": "2026-10-09T18:00:00Z"
    }
  ],
  "history": []
}
```

### 2. Pydantic Mail Queue Model (`mail_queue.json`)
```python
from enum import Enum
from pydantic import BaseModel, Field
from typing import List, Optional
import datetime

class TriageStatus(str, Enum):
    PENDING = "PENDING"
    BACKLOG = "BACKLOG"
    RESEARCH = "RESEARCH"
    GRAVEYARD = "GRAVEYARD"
    PCDA = "PCDA"

class PriorityLevel(str, Enum):
    P0 = "P0"
    P1 = "P1"
    P2 = "P2"
    P3 = "P3"

class MailItem(BaseModel):
    id: str
    source_file: str
    title: str
    date_captured: datetime.date
    priority: PriorityLevel = PriorityLevel.P1
    status: TriageStatus = TriageStatus.PENDING
    tags: List[str] = Field(default_factory=list)
    word_count: int = 0
    excerpt: str = ""
    prerequisites: List[str] = Field(default_factory=list)
    archived_to: Optional[str] = None
```

### 3. TKPL Preset Validation Model (`preset.schema.json`)
```python
from pydantic import BaseModel, Field
from typing import Dict, List, Optional

class PresetMetadata(BaseModel):
    name: str = Field(min_length=1, max_length=64)
    author: str = "Cody Gratner"
    category: str
    tags: List[str] = Field(default_factory=list)
    theme_hint: Optional[str] = None

class PresetPayload(BaseModel):
    schema_version: str = "1.0.0"
    target_plugin: str = Field(pattern="^(TKF|TKP|TKM|TKR)$")
    metadata: PresetMetadata
    parameters: Dict[str, float]  # Validates parameter ID -> float range (0.0 to 1.0)
```

---

## 📋 Phase-by-Phase Execution Plan

### Phase 1: Persistent Multi-Plan Queue & Plan Rollback Engine
- [ ] Initialize `.agents/pipeline/plans/queue.json` with active plan tracking schema.
- [ ] Implement `tools/plan_queue.py` (and mirror to `nkai`):
  - `add_plan(plan_path, priority, target)`: Appends draft plan to `queue.json`.
  - `promote_next()`: Snapshots active `PLAN.md` to `.agents/pipeline/plans/snapshots/PLAN_PREV.md`, promotes next queued draft to root `PLAN.md`, and advances execution pointer.
  - `rollback()`: Restores `.agents/pipeline/plans/snapshots/PLAN_PREV.md` to `PLAN.md` in 5ms with zero token burn.
- [ ] Update `.agents/skills/clean-plan/SKILL.md` to automatically invoke `plan_queue.py promote_next` upon archiving a completed plan.
- [ ] Register `/rollback-plan` shortcut in Antigravity skill definitions.

### Phase 2: The Klang Vault Private Studio Hub & Sync Hardening — ✅ COMPLETED
- [x] Verify `C:\Dev\TheKlangVault\.gitignore` (protecting `.obsidian/workspace.json`, `.obsidian/cache/`, `.trash/`).
- [x] Configure `C:\Dev\TheKlangVault\.obsidian\app.json` with `userIgnoreFilters` (excluding `.git/`, `tools/`, `*.py`, `*.pyc`).
- [x] Initialize Git repository and commit initial baseline in `TheKlangVault` (`525f070`).
- [x] Author `docs/architecture/the_klang_vault_studio_hub.md` detailing multi-repo separation.
- [x] Register sync safety rules and automated agent ownership in `docs/GLOSSARY.md`, `context_clues_subagent.md`, `docs/CHEAT_SHEET.md`, `docs/DOCS_CATALOG.json`, and `docs/SYSTEM_MAP.md`.

### Phase 3: Global Pydantic Setup & Persistent Mailbag Session
- [ ] Run `pip install pydantic` into global Python.
- [ ] Author `TheKlangVault/tools/models.py` (or `tools/mail_models.py`):
  - Defines `MailItem`, `MailQueue`, `TriageStatus`, and `TransactionReceipt`.
- [ ] Refactor `tools/check_mail.py` to use Pydantic for validation and serialization:
  - Enforce reverse-chronological sorting for `P0` notes.
  - Generate `.agents/pipeline/mailbag/mail_queue.json` ledger.
  - Implement atomic transaction receipts (`.agents/pipeline/staging/mail_tx_latest.json`) and `--rollback`.
- [ ] Update `morning_mail_inbox.html` with topological warning suggestion chips for dependent `P0` notes.

### Phase 4: TKPL — The Klang Preset Linter CLI
- [ ] Run `pip install pydantic` verification.
- [ ] Author `tools/preset_audit.py` (TKPL):
  - Validates all JSON files in `assets/presets/` against `PresetPayload` model.
  - Validates that parameter IDs exist in `assets/controls/master_control_schema.json`.
  - Asserts normalized parameter floats reside strictly in $[0.0, 1.0]$.
  - CLI commands: `--lint`, `--fix-metadata`, `--export-schema`.
- [ ] Generate `assets/presets/$schema/preset.schema.json` via Pydantic `.model_json_schema()`.

### Phase 5: N'kai Local FastAPI Sidecar Bridge (`localhost:8000`)
- [ ] Run `pip install fastapi uvicorn` into global Python.
- [ ] Author `tools/nkai_api.py`:
  - 80-line lightweight FastAPI server running locally on `http://127.0.0.1:8000`.
  - Pydantic endpoints:
    - `GET /health`: Returns bridge status and active workspace.
    - `GET /api/mail`: Returns validated `MailQueue`.
    - `POST /api/action`: Dispatches validated `TriageAction` (`backlog`, `research`, `graveyard`, `pcda`).
    - `POST /api/plan/rollback`: Triggers 5ms plan queue rollback.
- [ ] Add CORS headers allowing requests from `file:///` and `http://localhost:*`.
- [ ] Update N'kai sidecars to detect `:8000` FastAPI bridge with zero-latency HTTP responses.

### Phase 6: Rolling Blackbox Context Compaction & Institutional Memory (Topic #7)
- [ ] Formalize Context Compaction Tripwires in `GEMINI.md`:
  - Enforce automated `/refresh-context` when active chat transcript approaches compaction thresholds.
  - Automate updating `context_clues_subagent.md` and `docs/history/DEV_HISTORY.md` before compacting.

### Phase 7: Catalog Registration & Verification Gauntlet
- [ ] Register new specs in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.
- [ ] Run `python tools/preset_audit.py --lint`.
- [ ] Run `python tools/audit_sidecar.py .agents/sidecar/morning_mail_inbox.html`.
- [ ] Run `python tools/verify_context_clues.py`.
- [ ] Commit all changes cleanly with conventional commits.

---

## 🧪 Verification & Acceptance Gauntlet

1. **Multi-Plan Queue Test**:
   - Queue a test plan `tools/plan_queue.py add-plan test.md`.
   - Promote next plan and verify `PLAN.md` updates.
   - Run `tools/plan_queue.py rollback` and assert original `PLAN.md` is restored in 5ms.
2. **Pydantic Validation Test**:
   - Inject an invalid field or type into `mail_queue.json`.
   - Run `python tools/check_mail.py`.
   - Assert Pydantic catches and logs a structured `ValidationError`.
3. **TKPL Preset Linting Test**:
   - Run `python tools/preset_audit.py --lint`.
   - Assert all factory presets pass schema validation.
4. **FastAPI Bridge Test**:
   - Start `python tools/nkai_api.py` and test `GET http://127.0.0.1:8000/health`.
   - Assert HTTP 200 with valid JSON response.
