# N'kai Canonical State & Persistence Architecture (Topic 6.16)

## 1. Architectural Intent & Scope

The **N'kai Asymmetric Sidecar Framework** (`nkai`) powers rich, visual development boards, triage autopsies, interactive interview labs (`visual-grill-me`), and real-time project monitors across the Four Pillars (`TheKlangSuite`, `ToadTracker`, `nkai`, `TheKlangResearch`) and `TheKlangVault`.

### The Core Problem
Traditional developer tooling often reaches for embedded binary databases (such as SQLite, DuckDB, or LevelDB) or centralized stateful server daemons. While convenient for rapid prototyping, binary databases introduce critical failure modes in AI pair-programming and Git workflows:
1. **Opaque Binary Bloat**: Binary database files (`*.sqlite`, `*.db`) destroy Git diff fidelity, make branch merges impossible, and cause silent repository bloat.
2. **Locking & Concurrency Hazards**: File locks on database files cause crashes when IDE webviews, background terminal tasks, and CLI commands access them concurrently.
3. **Daemon Dependency**: Requiring a running local background server (e.g. Node.js or Python daemon) breaks zero-configuration offline usage and IDE webview compatibility (`file:///` protocol).
4. **Agent Inspection Friction**: LLM agents cannot easily inspect, grep, or edit raw binary databases without specialized external tooling.

### The Solution: Option C (Hybrid 3-Tier State Model)
Formally approved under **Topic 6.16**, N'kai adopts a clean, deterministic, 3-tier persistence architecture combining **Human-Readable Markdown Frontmatter**, **Deterministic JSON Session Packages**, and **Browser `localStorage`**.

---

## 2. The 3-Tier Canonical State Hierarchy

```
┌─────────────────────────────────────────────────────────────────────────┐
│               TIER 1: Canonical Knowledge & RFC Autopsies               │
│      Location: TheKlangResearch/, TheKlangVault/, docs/specs/*.md       │
│      Format: Plaintext GFM Markdown with YAML Frontmatter               │
│      Role: Eternal source of truth, human-editable, Git-tracked        │
└────────────────────────────────────┬────────────────────────────────────┘
                                     │ Hydrates / Ingests
                                     ▼
┌─────────────────────────────────────────────────────────────────────────┐
│              TIER 2: Intermediate Session Packages & Catalogs           │
│      Location: .agents/sidecar/packages/<pkg_id>.json, DOCS_CATALOG.json│
│      Format: Deterministic Plain JSON Schemas                           │
│      Role: Offline data exchange between agent CLI and HTML sidecar     │
└────────────────────────────────────┬────────────────────────────────────┘
                                     │ Renders into Webview
                                     ▼
┌─────────────────────────────────────────────────────────────────────────┐
│               TIER 3: Ephemeral Client-Side UI & Settings               │
│      Location: Browser / Chromium Webview window.localStorage           │
│      Format: Key-Value Local Webview Storage                            │
│      Role: Zoom scale, active tabs, filter chips, composer drafts       │
└─────────────────────────────────────────────────────────────────────────┘
```

---

### Tier 1: Canonical Human-Readable Knowledge (Markdown Frontmatter)
- **Primary Locations**:
  - `TheKlangResearch/RFC/` & `TheKlangResearch/Autopsies/`: High-level architectural RFCs, DSP derivations, and post-mortem findings.
  - `TheKlangVault/Inbox/` & `TheKlangVault/Research/`: Obsidian-compatible research vault notes.
  - `docs/specs/*.md`: Codebase implementation specifications.
- **Format**: Plaintext Markdown with structured YAML frontmatter:
  ```yaml
  ---
  id: RFC-0042
  title: "Hydra Modulation Matrix & Cable Visualizer"
  status: APPROVED
  author: "New Klang City"
  date: 2026-10-09
  category: dsp_architecture
  tags: [modulation, hydra, dsp, matrix]
  decisions:
    - topic: "Cable Rendering"
      choice: "Cubic Catmull-Rom Splines"
  ---
  ```
- **Guarantees**:
  - 100% human-readable and plain-text diffable in Git.
  - Compatible with Obsidian, GitHub, VS Code, and standard markdown viewers.
  - Directly greppable and parseable by standard CLI tools (`grep`, `rg`, `Select-String`, Python).

---

### Tier 2: Intermediate Session Packages & Catalogs (JSON)
- **Primary Locations**:
  - `.agents/sidecar/packages/<package_id>.json`: Staged research boards, triage matrices, option cards, and PCDA evaluations.
  - `docs/DOCS_CATALOG.json`: Master navigation catalog of all architectural documentation and specifications.
  - `assets/themes/theme.json`: Curated studio color palettes and chassis skins.
- **Format**: Strictly structured JSON with clear schema separation:
  ```json
  {
    "package_id": "pkg_triage_milestone_040",
    "timestamp": "2026-10-09T12:00:00Z",
    "milestone": "v0.4.0",
    "topics": [
      {
        "id": "topic_6_16",
        "title": "Backend Persistence & State Management",
        "status": "APPROVED",
        "recommendation": "Option C (Hybrid 3-Tier)",
        "pcda": {
          "thesis": "Zero binary locks, clean git diffs",
          "antithesis": "Requires client-side JSON serialization",
          "synthesis": "Use localStorage for UI and JSON packages for payloads"
        }
      }
    ]
  }
  ```
- **Guarantees**:
  - Bridges agent executions and interactive visual HTML sidecars without requiring live IPC sockets or running daemons.
  - Deterministic serialization ensures predictable Git diffs if checked into version control.

---

### Tier 3: Ephemeral Client-Side UI State (Browser `localStorage`)
- **Primary Location**: Chromium Webview / Browser `window.localStorage` sandbox.
- **Scope & Keys**:
  | Key | Purpose | Default | Fallback |
  | :--- | :--- | :--- | :--- |
  | `nkai_font_scale` | Persistent sidecar typography zoom factor (`A-` / `100%` / `A+`) | `"1.0"` | `"1.0"` |
  | `nkai_active_tab` | Currently active operational view tab (`workshop`, `autopsy`, `roadmap`) | `"workshop"` | `"workshop"` |
  | `nkai_filter_status` | Active triage filter chip (`all`, `approved`, `tabled`, `killed`) | `"all"` | `"all"` |
  | `nkai_collapsed_sections` | JSON array of collapsed section IDs in the active canvas | `[]` | `[]` |
  | `nkai_composer_draft` | Pending Decision Composer scratch text in the side panel | `""` | `""` |
- **Guarantees**:
  - Instant hydration when opening local HTML sidecars via `file:///` URI.
  - Survives browser page refreshes and window resizing.
  - Complete zero-latency UI interactions with zero network round-trips.

---

## 3. Strict Invariants & Operational Guardrails

### Invariant 1: The Zero-Binary Database Invariant (CRITICAL)
- **Zero Binary Blobs**: Under NO circumstances may SQLite (`*.sqlite`, `*.db`), LevelDB, RocksDB, or custom binary databases be created, committed, or required for persistence in the repository.
- **Zero Runtime Daemons**: Sidecars must function as autonomous, self-contained HTML/CSS/JS bundles executable directly via `file:///` without requiring a local Node.js or Python background server.

### Invariant 2: The Git-Diff Preservation Standard
- All JSON files must be formatted with 2-space indentation and sorted or deterministic key ordering.
- Markdown frontmatter must follow strict YAML formatting without multiline escapes that disrupt diff tools.
- Never write transient timestamps or session counters into version-controlled files unless updating an explicit changelog or dev history.

### Invariant 3: Offline `file:///` First Principle
- All interactive HTML canvases must bundle their required CSS and inline SVG icons.
- JavaScript logic must rely on native browser Web APIs (`localStorage`, `DOMParser`, `CanvasRenderingContext2D`, `Web Audio API`) without unpkg/CDN dependencies that fail when working offline.

---

## 4. Hydration & State Reconciliation Flow

When a user or agent opens an N'kai sidecar:
1. **Initial Boot (Tier 3 Hydration)**:
   - JavaScript reads `localStorage` to restore user font scaling, dark theme settings, and active tab index.
2. **Payload Ingestion (Tier 2 Loading)**:
   - Sidecar reads the structured JSON package (embedded in the HTML or loaded from `.agents/sidecar/packages/`).
   - Renders topic cards, PCDA dialectics, and action chips into the DOM.
3. **Interactive Decision Capture (Tier 3 Staging)**:
   - When the user selects choices or composes decisions in the Decision Composer, the draft is mirrored in `localStorage` to prevent accidental data loss.
4. **Handoff & Canonical Persistence (Tier 1 Archival)**:
   - Clicking `[ Copy Decision to Chat ]` formats the choice as clean Markdown text for the IDE chat.
   - Upon task completion, Klang Industries commits the verified result to Git, and `tools/sync_obsidian_vault.ps1` synchronizes the Markdown record into `TheKlangVault/Research/`.

---

## 5. Verification & Validation Harness

- **Integrity Audit**: Verified programmatically by `tools/verify_context_clues.py`.
- **Schema Audit**: Enforced by `test/ParameterSchemaAuditTest.h` and the Dynamic Reflection test harness.
