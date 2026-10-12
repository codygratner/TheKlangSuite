# N'kai 3-Surface System & Frictionless Triage Ergonomics (Flight 20)

## 1. Architectural Intent & Storyline

As the N'kai multi-board ecosystem expanded, a proliferation of specialized boards risked board sprawl and cognitive fatigue. Developers faced unnecessary friction:
1. **Context Fragmentation**: Triage and capture required leaving the active IDE editor or sidecar to open Obsidian, run external shell commands, or manually edit text files.
2. **Delivery Disconnect**: High-level project roadmaps lived inside `docs/BACKLOG.md` markdown text, decoupled from the visual card triage workflow.
3. **Session Noise & Lab Sprawl**: Ephemeral architectural spikes, research sessions, and `/grill-me` interviews generated temporary boards that accumulated indefinitely without clear conclusion ceremonies or insight harvesting.

**Flight 20** establishes the **Unified 3-Surface Architecture**:
- **Surface 1 (Evergreen #1: The Mailbag — `morning_mail`)**: The daily human intake stream with an embedded, low-friction **Quick Capture Bar** and debounced **"Grab New Mail"** trigger.
- **Surface 2 (Evergreen #2: The Flight Runway — `runway`)**: The strategic delivery horizon mechanically generated from `docs/BACKLOG.md` Act III, providing cross-pillar roadmap visibility and status filtering (`[ 🚀 Active Flights ]`, `[ 📦 Shipped & Milestones ]`, `[ 📋 All ]`).
- **Surface 3 (Session Surface: The Lab — `lab`)**: A single, ephemeral active workspace slot for active research and architectural spikes, featuring a dynamic **"ALL CLEAR" Triumph Celebration** and automated harvest staging to `.agents/pipeline/staging/souls/`.
- **The Scratchpad Protocol**: A dedicated [`C:\_\Vault\Inbox\SCRATCHPAD.md`](file:///C:/_/Vault/Inbox/SCRATCHPAD.md) for transient reading notes, permanently excluded from mailbag harvesting and pinned in the ambient chat navigation footer.

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                       N'KAI 3-SURFACE SYSTEM TOPOLOGY                       │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   EVERGREEN SURFACE 1: THE MAILBAG (`morning_mail`)                         │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │ [ Quick Capture: "+ Quick Note..." ] [ 📬 Grab Mail ]                │   │
│   │ Daily triage docket · Vault Inbox notes · Catchall aggregation      │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│                                │                                            │
│                                ▼                                            │
│   EVERGREEN SURFACE 2: THE FLIGHT RUNWAY (`runway`)                         │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │ Filter Rails: [ 🚀 Active Flights ] [ 📦 Shipped ] [ 📋 All ]       │   │
│   │ Mapped directly from docs/BACKLOG.md Act III · Ecosystem Milestones │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│                                │                                            │
│                                ▼                                            │
│   SESSION SURFACE 3: THE LAB (`lab` — Ephemeral Active Slot)                │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │ Spikes · Research · Grill-Me · 0 Untriaged ➔ Triumph Celebration    │   │
│   │ [ 🏁 Conclude Lab & Stage Insights ] ➔ Soul Harvest Staging         │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 2. Component Specifications

### 2.1 In-Board Quick Capture & Daily Catchall (`morning_mail`)
- **Quick Capture Input Bar**: Rendered directly in `.header-tier-top` as `#quick-capture-container` (`#quick-capture-input` + `#btn-quick-capture`), active exclusively when `window.NKAI_EXTENSION_MODE === true`.
- **Catchall Aggregation**: Appends timestamped entries `- [<HH:MM>] <text>` to `C:\_\Vault\Inbox/<YYYY-MM-DD>_Quick_Notes.md`. If missing, the file is automatically initialized with compliant YAML frontmatter.
- **Debounced Mail Grab**: `#btn-grab-mail` triggers `POST /api/check-mail`, enforced by a 500ms debounce mutex in `sidecars/panel/main.mjs`, running `python tools/check_mail.py --no-browser` and auto-reloading without browser popups.

### 2.2 The Flight Runway Board (`runway_board.html`)
- **Generator**: Built mechanically via `tools/build_runway.py`.
- **Data Source**: Parses `docs/BACKLOG.md` Act III (Sections 15–18: Ecosystem Architecture & Hardening), extracting flight numbers, titles, targets, and model tiers.
- **Milestone Cards**: Ingests active milestone headers (e.g. `TKS v0.4.0`, `N'kai v0.3.5`, `TT v0.1.0`) with pillar badges and git-backed release tags.
- **Filter Rails**: Instant clientside filtering across three delivery views:
  - `[ 🚀 Active Flights ]`: In-progress and pending backlog flights.
  - `[ 📦 Shipped & Milestones ]`: Completed flights and milestone benchmarks.
  - `[ 📋 All ]`: Complete panoramic roadmap view.
- **Zero-Token Ordering**: Roadmap sequencing is modified strictly via chat commands (`/audit-reorder`, `/rejigger`), avoiding complex, error-prone UI drag-and-drop code.

### 2.3 Ephemeral Lab Board Lifecycle (`lab`)
- **Dynamic Board Registry**: `BoardRegistry` in `tools/models.py` tracks `active_lab_id: Optional[str]` and `staged_labs: List[str]`.
- **Triumph All-Clear Celebration**: When untriaged cards in an active lab reach 0, `#lab-all-clear-banner` renders an animated triumph card with `#btn-conclude-lab`.
- **Conclusion Ceremony (`tools/conclude_lab.py`)**:
  - Automatically compiles session takeaways into `.agents/pipeline/staging/souls/<YYYY-MM-DD>_<slug>_harvest.md`.
  - Transitions board state from active to staged in `.agents/pipeline/boards/registry.json`.
  - Clears `active_lab_id` from the ambient navigation footer.

### 2.4 The Scratchpad Protocol & Ambient Navigation Footer
- **Dedicated File**: `C:\_\Vault\Inbox\SCRATCHPAD.md`.
- **Scanner Exclusion**: Permanently excluded from `tools/check_mail.py` inbox harvesting to prevent stream-of-consciousness reading notes from polluting triage dockets.
- **Ambient Navigation Invariant**: Codified in `GEMINI.md`:
  ```markdown
  Focus: <Current Focus> | TKS-<ver> · TT-<ver> · NK-<ver> · TR-<ver> · [ 📝 Scratchpad ](file:///C:/_/Vault/Inbox/SCRATCHPAD.md) · [ 📬 Mailbag ](sidecar://nkai/panel/?board=morning_mail) · [ 🛫 Runway ](sidecar://nkai/panel/?board=runway)
  ```
  *(Appends `· [ 🔬 Lab: <topic> ](sidecar://nkai/panel/?board=lab)` when an active lab is pinned).*

---

## 3. Verification & Compliance
- **Test Gauntlet**: Covered by `tools/tests/test_runway_board.py` (parsing, schema validation, HTML generation) and `tools/tests/test_quick_capture.py` (catchall formatting, non-destructive appending).
- **Web QA**: Audited via `tools/audit_sidecar.py` with 100% pass across all HTML templates.
- **Anti-Cache Routing**: Registered explicit endpoints in `sidecars/panel/main.mjs` preventing 404s in Antigravity SidecarApp.
- **UTF-8 Safety**: Strict compliance with universal UTF-8 stream reconfigurations.
