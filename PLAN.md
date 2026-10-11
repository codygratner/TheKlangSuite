# 🎭 The Grand Operatic Flight Plan: Symphony of New Klang City
## Multi-Plan Queue, Vault Studio Hub, Pydantic Engines & Deep Docs Gauntlet

> **TARGET:** TOOLING_HARNESS (New Klang City) 🚀  
> **Dramatis Personae & Execution Fleet:** Tier 2 Flash Builder Subagent (via `subagent_dispatch.md`) + Tier 1 Pro On-Demand Math/Taxonomy Specialists  
> **Status:** READY_FOR_EXECUTION (Awaiting Kickoff Confirmation) ⏳  
> **Target Branch:** `0.4.0-dev`  
> **Recommended Setting:** Gemini 3.8 Flash (Thinking: High) — 🟢 SUSTAINABLE  
> **Altitude & Stage:** 4 Operatic Acts across 16 Flat Sequenced Flights (Early Flights $\to$ Main Flights $\to$ Late Flights $\to$ Touchdown)  
> **Cognitive Model:** The Tasting Menu + Airport Departure Board (Sequential integers 1..16; ambient atmospheric altitude acts; 100% human cognitive offloading to AI)  
> **Boundary Box:** Strict 1-Week Tooling Sprint (guaranteeing we return to pure C++ synthesizer DSP immediately upon touchdown)  

---

## 🎼 The PCDA Dialectic Overture

```
                    ┌────────────────────────────────────────────────────────┐
                    │ THESIS [Pros] (Total Human Cognitive Offloading)       │
                    │ - AI manages the 16-step roadmap; human tastes sound   │
                    │ - Multi-Plan Queue prevents plan clobbering & loss     │
                    │ - Zonal READMEs stop token bloat & README cemeteries   │
                    │ - Global Pydantic eliminates virtualenv friction       │
                    │ - Field guide decoupling establishes world-class docs  │
                    │ - Workplace-relevant resume & technical talking points │
                    └───────────────────────────┬────────────────────────────┘
                                                │
                                                ▼
                    ┌────────────────────────────────────────────────────────┐
                    │ THESIS FRICTION [Cons] (Surface Overhead)              │
                    │ - Multi-repository choreography (TKS, TKV, TKR, NKAI)   │
                    │ - 4 distinct new tooling touchpoints (Queue, Mail, API)│
                    │ - Upfront visual planning takes ~2 hours before build  │
                    └───────────────────────────┬────────────────────────────┘
                                                │
                                                ▼
                    ┌────────────────────────────────────────────────────────┐
                    │ ANTITHESIS [Devil's Advocate] (The Distraction Trap)   │
                    │ - "Building infinite dev infrastructure instead of     │
                    │   finishing the C++ synthesizer UI release"            │
                    │ - The 3-Month Framework Odyssey risk                   │
                    └───────────────────────────┬────────────────────────────┘
                                                │
                                                ▼
                    ┌────────────────────────────────────────────────────────┐
                    │ SYNTHESIS [The 80/20 Operatic Resolution]              │
                    │ - Strict 1-Week Box: locked down upon Flight 16 landing│
                    │ - Plugin development is strictly "FOR ME" (zero rush)  │
                    │ - Upfront N'kai planning kills the non-linear trap     │
                    │ - Stateless Atomic Rollbacks provide 5ms reversibility │
                    │ - Event-driven glossary sync (only on terminology shift)│
                    └────────────────────────────────────────────────────────┘
```

---

## 🏛️ The Operatic Flight-Stage Architecture

```
  ┌─────────────────────────────────────────────────────────────────────────────────────────┐
  │                                    FLIGHT ALTITUDE MAP                                  │
  ├─────────────────────────────────────────────────────────────────────────────────────────┤
  │                                                                                         │
  │   ACT I: Ground Control & Taxiway (Early Flights 1–4)                                   │
  │   [Queue Engine] ──> [Vault Git Sync] ──> [Zonal READMEs] ──> [Global Pydantic]        │
  │                                                                                         │
  │   ACT II: Ascent & Cruising Altitude (Main Flights 5–8)                                 │
  │   [Mailbag Pydantic] ──> [TKPL Preset Linter] ──> [FastAPI Bridge] ──> [Context Box]   │
  │                                                                                         │
  │   ACT III: High-Altitude Reconnaissance & Gauntlet (Late Flights 9–13)                  │
  │   [Field Guides Decoupled] ──> [Anti-Slop Manifesto] ──> [Cheat Sheet] ──> [Vault Sync] │
  │                                                                                         │
  │   ACT IV: Final Approach & Touchdown (Capstone Flights 14–16)                           │
  │   [System Map Refresh] ──> [Acceptance Gauntlet] ──> [Wheels Down / Tag v0.4.0]         │
  │                                                                                         │
  └─────────────────────────────────────────────────────────────────────────────────────────┘
```

---

## 📜 Contractual Data Schemas

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
      "title": "Grand Operatic Flight Plan: Symphony of New Klang City",
      "plan_file": "PLAN.md",
      "target": "TOOLING_HARNESS",
      "status": "IN_PROGRESS",
      "priority": "P0",
      "created_at": "2026-10-10T18:00:00Z"
    }
  ],
  "history": []
}
```

### 2. Pydantic Mail Queue Model (`TheKlangVault/tools/models.py`)
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

## 🎭 The Libretto: Operatic Phase-by-Phase Flight Itinerary

### 🛫 ACT I: GROUND CONTROL & TAXIWAY (Early Flights / Pre-Flight Infrastructure)

#### Scene 1 (Flight 1): The Multi-Plan Queue & Stateless Atomic Rollback Engine
- [ ] Author `.agents/pipeline/plans/queue.json` initialized with active plan pointer schema.
- [ ] Author `tools/plan_queue.py` (and mirror to `nkai/tools/`):
  - `add_plan(plan_path, priority, target)`: Appends draft plan to `queue.json`.
  - `promote_next()`: Snapshots active `PLAN.md` to `.agents/pipeline/plans/snapshots/PLAN_PREV.md`, promotes next queued draft to root `PLAN.md`, and advances execution pointer.
  - `rollback()`: Restores `.agents/pipeline/plans/snapshots/PLAN_PREV.md` to `PLAN.md` in 5ms with zero token burn.
- [ ] Update `.agents/skills/clean-plan/SKILL.md` to automatically invoke `plan_queue.py promote_next` upon archiving a completed plan.
- [ ] Register `/rollback-plan` shortcut in Antigravity skill definitions.

#### Scene 2 (Flight 2): The Klang Vault Private Studio Hub & Sync Hardening — ✅ COMPLETED
- [x] Verify `C:\Dev\TheKlangVault\.gitignore` (protecting `.obsidian/workspace.json`, `.obsidian/cache/`, `.trash/`).
- [x] Configure `C:\Dev\TheKlangVault\.obsidian\app.json` with `userIgnoreFilters` (excluding `.git/`, `tools/`, `*.py`, `*.pyc`).
- [x] Initialize Git repository and commit initial baseline in `TheKlangVault` (`525f070`).
- [x] Author `docs/architecture/the_klang_vault_studio_hub.md` detailing multi-repo separation.
- [x] Register sync safety rules and automated agent ownership in `docs/GLOSSARY.md`, `context_clues_subagent.md`, `docs/CHEAT_SHEET.md`, `docs/DOCS_CATALOG.json`, and `docs/SYSTEM_MAP.md`.

#### Scene 3 (Flight 3): The Architectural Zonal Boundary Standard (The 3-Zone READMEs) — ✅ COMPLETED
- [x] Author `Source/dsp/README.md` (Zone 1: Native Audio DSP):
  - Real-time audio thread invariants, zero allocations in `processBlock()`, zero locks, zero blocking I/O, SIMD `FastMath`, thread-safe lock-free SPSC ring buffers.
- [x] Author `assets/README.md` (Zone 2: Data Schemas & Assets):
  - 4-layer parameter separation of concerns (`controls/`, `layouts/`, `themes/`, `text/`), normalized ranges $[0.0, 1.0]$, APVTS parameter contracts.
- [x] Author `tools/README.md` (Zone 3: Tooling Harness):
  - Tooling isolation: Zero external dependencies in Python scripts (standard library only, UTF-8 stdlib), Node CLI wrappers, and clean bridge servers.
- [x] Codify the anti-pattern prohibition against scattering rogue per-folder blackboxes.

#### Scene 4 (Flight 4): Global Pydantic Environment Provisioning — ✅ COMPLETED
- [x] Execute `python -m pip install pydantic` in global Python.
- [x] Verify global installation: `python -c "import pydantic; print(pydantic.__version__)"` (verified Pydantic v2.14.0).
- [x] Validate zero-venv execution from both `TheKlangSuite` and `TheKlangVault` command lines.

---

### ✈️ ACT II: ASCENT & CRUISING ALTITUDE (Main Flights / Core Engine & Triage)

#### Scene 1 (Flight 5): Persistent Mailbag Session & Pydantic Validation
- [ ] Author `TheKlangVault/tools/models.py`:
  - Implements `MailItem`, `MailQueue`, `TriageStatus`, `PriorityLevel`, and `TransactionReceipt`.
- [ ] Refactor `tools/check_mail.py` to use Pydantic models for validation and serialization:
  - Enforces reverse-chronological sorting for `P0` notes.
  - Generates `.agents/pipeline/mailbag/mail_queue.json` ledger.
  - Implements atomic transaction receipts (`.agents/pipeline/staging/mail_tx_latest.json`) and `--rollback`.
- [ ] Update `morning_mail_inbox.html` with topological warning suggestion chips for dependent `P0` notes.

#### Scene 2 (Flight 6): TKPL — The Klang Preset Linter CLI
- [ ] Author `tools/preset_audit.py` (TKPL):
  - Validates all JSON files in `assets/presets/` against `PresetPayload` Pydantic model.
  - Validates parameter IDs exist in `assets/controls/master_control_schema.json`.
  - Asserts normalized parameter floats reside strictly in $[0.0, 1.0]$.
  - CLI commands: `--lint`, `--fix-metadata`, `--export-schema`.
- [ ] Generate `assets/presets/$schema/preset.schema.json` via Pydantic `.model_json_schema()`.

#### Scene 3 (Flight 7): N'kai Local FastAPI Sidecar Bridge (`localhost:8000`) — ✅ COMPLETED
- [x] Install FastAPI globally: `pip install fastapi uvicorn`.
- [x] Author `tools/nkai_api.py`:
  - Clean, lightweight FastAPI server running locally on `http://127.0.0.1:8000`.
  - Endpoints:
    - `GET /health`: Returns bridge status and active workspace.
    - `GET /api/mail`: Returns validated `MailQueue`.
    - `POST /api/action`: Dispatches validated `TriageAction` (`backlog`, `research`, `graveyard`, `pcda`).
    - `POST /api/plan/rollback`: Triggers 5ms plan queue rollback.
- [x] Add CORS headers allowing requests from `file:///` and `http://localhost:*`.
- [x] Update N'kai sidecars to detect `:8000` FastAPI bridge with zero-latency HTTP responses.


#### Scene 4 (Flight 8): Rolling Blackbox Context Compaction & Institutional Memory (Topic #7) — ✅ COMPLETED
- [x] Formalize Context Compaction Tripwires in `GEMINI.md`:
  - Enforce automated `/refresh-context` when active chat transcript approaches compaction thresholds.
  - Automate updating `context_clues_subagent.md` and `docs/history/DEV_HISTORY.md` before compacting.


---

### 🛰️ ACT III: HIGH-ALTITUDE RECONNAISSANCE & THE GAUNTLET (Late Flights / Deep Docs)

#### Scene 1 (Flight 9): Subagent Soul Harvest & Autopsy Ingestion (Inbox Zero) — ✅ COMPLETED
- [x] Ingest `.agents/pipeline/staging/souls/*.md` (including `2026-10-10_soul_operatic_flights_pcda.md`) into `TheKlangResearch/subagents/`.
- [x] Verify `TheKlangResearch/qa_autopsies/` contains all verified post-mortems and platform autopsies.
- [x] Reset `.agents/pipeline/communique/subagent_receipt.md` to `STATUS: IDLE`.
- [x] Purge processed raw files in `.agents/pipeline/staging/souls/` to achieve Inbox Zero.

#### Scene 2 (Flight 10): Ground-State Spec Reconciliation & Catalog Registration — ✅ COMPLETED
- [x] Register new tools (`preset_audit.py`, `nkai_api.py`), schemas (`master_control_schema.json`, `preset.schema.json`), and specs in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.
- [x] Reconcile `c:\Dev\TheKlangSuite\docs\BACKLOG.md` with recent Act I & II completions.
- [x] Verify Section 8 (Aviation Metaphors), Section 9 (Zonal Boundaries), and Section 10 (Documentation Invariant) in `docs/GLOSSARY.md` and `docs/CHEAT_SHEET.md`.

#### Scene 3 (Flight 11): Field Guide Separation of Concerns (Universal vs. Audio DSP — Field Guides Always Last) — ✅ COMPLETED
- [x] **`TheKlangResearch/guides/vibe_coding_field_guide.md` (Universal Vibe-Coding Manifesto)**:
  - Strip out audio-specific C++ parameter blocks, JUCE audio thread invariants, and sample-rate math.
  - Generalize into a 100% stack-agnostic manual for autonomous AI pair-programming (Python, Rust, TypeScript, C++, web, embedded).
  - Emphasize: Dual-Chat division of labor (Ivory Tower vs Factory Floor), Model Tier economics, Asymmetric Sidecar Split, Tri-Engine subagent swarms (`explore`, `contrarian`, `research`), Strategy Graveyards, Negative Architecture, and bounded sprints vs runaway loops.
- [x] **`TheKlangResearch/guides/audio_dsp_field_guide.md` (Real-Time C++ & Audio DSP Bible)**:
  - Elevate from current stub into comprehensive, definitive manual for modern AI-assisted audio development.
  - Ingest all audio invariants: Zero allocations in `processBlock`, zero locks, zero blocking I/O, lock-free SPSC FIFOs, atomics for audio-to-UI telemetry.
  - Document SIMD `FastMath` Padé approximations, rational polynomial waveshapers, and cycle budgets.
  - Ingest filter topologies (SVF, ZDF, Ladder, Comb/Karplus-Strong) and anti-aliasing protocols (PolyBLEP, Half-Band FIR/IIR upsampling, MIP-mapped wavetables).
  - Codify modern C++20 standards for audio (RAII, zero raw ownership, JUCE 9.0.3 timer hygiene).
  - Document headless testing harness patterns (`dsp_tests`, `gui_tests`, dynamic reflection, `pluginval`).
- [x] **Local Pointers Synchronization**:
  - Update `c:\Dev\TheKlangSuite\docs\history\vibe_coding_field_guide.md` and `audio_dsp_field_guide.md` to reflect updated domain split.

#### Scene 4 (Flight 12): The "Anti-Slop" Vibe-Coding Executive Summary & Workflow Cheat Sheet — ✅ COMPLETED
- [x] **Author `TheKlangResearch/guides/anti_slop_vibe_coding_manifesto.md`**:
  - **Reading Time Target**: 2 to 3 minutes (~600 words, high-density, punchy, practical).
  - **Target Audience**: Professional software engineers and SQA practitioners skeptical of AI code generation and frustrated by "AI slop", hallucinations, broken whitespace, and silent regressions.
  - **Strict Tone & Style Invariant (ZERO INTERNAL LORE & ZERO PRODUCT NAMES)**:
    - **100% Zero-Lore & Zero-Product Mandate**: Strictly ZERO references to project mythology, Mayor Toad, fictional factions, inside jokes, or creative worldbuilding. Furthermore, strictly ZERO mentions of specific proprietary product names (no *The Klang Farmer*, *The Klang Planter*, *The Klang Suite*, *ToadTracker*, *N'kai*, or author names).
    - **"Process, Not Products"**: Objective, stack-agnostic software engineering methodology whitepaper.
    - **Professional Industry Standards**: Written strictly in industry-standard software engineering and SQA terminology (sandboxing, bounded context, static analysis, invariant assertions, deterministic verification gauntlets, AST codemods).
  - **Core Controls Highlighted**:
    1. Context Shield & Flash/Pro Subagent Slicing.
    2. Strict Domain Division & Facility Gates.
    3. Granular Blueprinting (Pro Plans, Flash Builds).
    4. Strict Architectural Invariants & Automated Auditing (`audiothread-guard`).
    5. The Boy Scout Rule & Ruthless Deletion.
    6. PCDA & The Adversarial Contrarian.
    7. Deterministic Test Gauntlets.
  - Index link in `TheKlangResearch/README.md` and `TheKlangVault/Research/Master_Research_Index.md`.
- [x] **Author `c:\Dev\TheKlangSuite\docs\WORKFLOW_CHEAT_SHEET.md`**:
  - Distill staged soul harvest `.agents/pipeline/staging/souls/2026-10-10_soul_operatic_flights_pcda.md`.
  - Concise, 1-page high-density reference sheet for human developers.
  - Facility Spheres, Model Tiers, Subagent Swarm roles, Communiqué flow, 5-Phase standard, N'kai CLI reference.

#### Scene 5 (Flight 13): Five-Pillar Hyperlink Integrity Gauntlet & Central Vault Recompilation
- [ ] Programmatic verification across all markdown documents in `TheKlangSuite`, `ToadTracker`, `nkai`, `TheKlangResearch`, and `TheKlangVault`.
- [ ] Run `powershell -ExecutionPolicy Bypass -File .\tools\sync_obsidian_vault.ps1`.
- [ ] Verify `TheKlangVault/Research/Master_Research_Index.md` compiles cleanly with 0 errors.


---

### 🛬 ACT IV: FINAL APPROACH & TOUCHDOWN (Pre-Release Capstone)

#### Scene 1 (Flight 14): Catalog Registration & Ecosystem System Map Synchronization
- [ ] Register all new specs, guides, and tools in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.
- [ ] Verify Section 8 (Aviation Metaphors), Section 9 (Zonal Boundaries), and Section 10 (Documentation Invariant) in `docs/GLOSSARY.md` and `docs/CHEAT_SHEET.md`.

#### Scene 2 (Flight 15): The Full Acceptance Gauntlet
- [ ] **Multi-Plan Queue Test**:
  - Queue test plan via `python tools/plan_queue.py add-plan test.md`.
  - Promote next plan and verify `PLAN.md` updates.
  - Run `python tools/plan_queue.py rollback` and assert original `PLAN.md` is restored in 5ms.
- [ ] **Pydantic Validation Test**:
  - Inject invalid type into `mail_queue.json`.
  - Run `python tools/check_mail.py`.
  - Assert Pydantic logs structured `ValidationError`.
- [ ] **TKPL Preset Linting Test**:
  - Run `python tools/preset_audit.py --lint`.
  - Assert all presets pass schema validation.
- [ ] **FastAPI Bridge Test**:
  - Start `python tools/nkai_api.py` and query `GET http://127.0.0.1:8000/health`.
  - Assert HTTP 200 with valid JSON response.
- [ ] **Context Clues Verification**:
  - Run `python tools/verify_context_clues.py`.
  - Assert 100% pass rate.

#### Scene 3 (Flight 16): Grand Touchdown & v0.4.0 Test-Drive Docket Review
- [ ] Commit all changes cleanly with conventional commits across affected repositories.
- [ ] Push clean commits to `TheKlangResearch` and `TheKlangVault`.
- [ ] Present comprehensive Operatic Touchdown report in chat with interactive test-drive docket.
