# Context Clues: The Klang Tower (Implementation & Build)

> **Domain:** The Klang Tower (`C:\Dev\TheKlangSuite`) & The Astral Abyss (`C:\_\Vault`)  
> **Session Refreshed:** 2026-10-14 17:26  
> **Active Milestone:** N'kai v0.4.0 Sovereign Command Deck  
> **Step 0 Bootstrapping**: Tower builders waking up from a context refresh or starting an implementation phase MUST read this document on Turn 1 to orient on active build directives, return agenda, architectural memory, canonical taxonomy, and tower invariants without prompt bloat.

---

## 📌 Active SemVer, Branch State & Roadmap Sidecar Deep Links
- **Current Git Branch:** `0.4.0-dev`
- **Ecosystem SemVer Matrix:**
  - `The Klang Suite (TKS)`: `v0.4.0-dev`
  - `ToadTracker (TT)`: `v0.1.0`
  - `N'kai (NK)`: `v0.4.0`
  - `The Klang Research (TKR)`: Continuous Living Archive (Zero SemVer)
- **Roadmap Sidecar Deep Links**:
  - [The Klang Suite v0.4.0 Roadmap](file:///c:/Dev/TheKlangSuite/.agents/sidecar/roadmap_sidecar.html#tks-v0.4.0)
  - [ToadTracker v0.1.0 Milestone](file:///c:/Dev/TheKlangSuite/.agents/sidecar/roadmap_sidecar.html#tt-v0.1.0)
  - [N'kai v0.1.0 Sidecar Framework](file:///c:/Dev/TheKlangSuite/.agents/sidecar/roadmap_sidecar.html#nk-v0.1.0)
- **Active Working Milestone:** N'kai v0.4.0 Sovereign Command Deck
- **Verified Systems**:
  - `tools/chat_engine.py`: Turn parsing with consecutive user message folding and immutable `turn_id: step_index`.
  - `tools/nkai_serve.py`: SSE streaming bridge, silent `agentapi` injection, `/api/check-mail` harvest, Apple Web Push.
  - `cockpit.html`: Multi-pane grid, mobile swipe gestures, Get Mail button.
  - `tools/webpush_service.py`: APNs VAPID notifications active.
  - `Agenda.md`: Queued tomorrow's items (Systray upgrades & Swipe threshold tuning).

---

## 🎯 Immediate Return Agenda & Status
- **Current Objective:** Completed all 7 Quick Wins from the Ecosystem-Wide Tooling & Skills Audit (plan_manager, bus fast-path, global skills, compat_check linter, agenda shadow cache, tool drift, context_extract & extract_clues). Ready for Flight 33 (DSP Efficiency & Saturation Core) or next milestone flight.
- **TKS Return Directive (Generative UI Spikes)**: When pivoting to TKS v0.4.1 / v0.5.0, immediately invoke `generative_ui` to prototype interactive, side-by-side HTML/CSS mockups of the 8-knob card layouts, dual-deck front/back flip, and TBD-16 encoder ergonomics before writing C++ JUCE components!
- **Execution State:** `IDLE / READY`
- **Active Blueprint:** [`PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md)
- **Flight Horizon**:
  - All 7 Tooling & Skills Quick Wins 100% complete and verified across all repositories.
  - Advance to Flight 33 (DSP Saturation Core) or user-directed priority.

---

## 🧠 Architectural Memory & "Unforgettables"
1. **Model Advisory & Quota Discipline**:
   - Universal default: **Gemini 3.8 Flash (Thinking: High)** for near-zero quota burn and rapid iteration.
   - Pro Tier 1 (`Model="pro"`) reserved strictly for high-reasoning DSP differential equations, non-linear saturation proofs, and complex architectural synthesis.
2. **Mobile & Chat Invariants**:
   - Folded rapid consecutive user messages into a single conversational turn with stacked message bubbles.
   - Bound DOM turn card IDs to immutable `step_index` instead of dynamic array slices.
   - Re-established EventSource on iOS Safari wake (`visibilitychange`) + added active 2.5s execution fallback polling to eliminate WebKit zombie sockets.
3. **Data-Driven Architecture (CRITICAL)**:
   - DSP parameters live exclusively in `assets/controls/*.json`.
   - Structural UI hierarchy lives in `assets/layouts/*.json`.
   - Visual styling and color schemes live in `assets/themes/*.json`.
   - Text, tooltips, and copy live in `assets/text/*.json`.

---

## 📖 Canonical Taxonomy, Product Lineup & Essential Spec Links
- **Master Documentation Catalog**: [`docs/DOCS_CATALOG.json`](file:///c:/Dev/TheKlangSuite/docs/DOCS_CATALOG.json) (100% indexed documentation ledger).
- **System Architecture Map**: [`docs/SYSTEM_MAP.md`](file:///c:/Dev/TheKlangSuite/docs/SYSTEM_MAP.md) (high-level cross-repo index).
- **Architectural Taxonomy & Glossary**: [`docs/GLOSSARY.md`](file:///c:/Dev/TheKlangSuite/docs/GLOSSARY.md)
- **Universal Project Backlog**: [`docs/BACKLOG.md`](file:///c:/Dev/TheKlangSuite/docs/BACKLOG.md) (single source of truth for all planned work).

---

## ⚡ Audio Thread / Workflow Invariants & Handshake Gates
1. **Audio Thread Invariants (CRITICAL)**:
   - **Zero Allocations**: Never call `new`, `malloc`, `free`, or resize dynamic containers in `processBlock()`.
   - **Zero Locks**: Never acquire `std::mutex`, `juce::CriticalSection`, or sleep on thread synchronization in the audio path.
   - **Zero Blocking I/O**: Zero filesystem, network, or console output on the audio thread.
   - **FastMath**: Prefer `TbdAudio::FastMath` over CRT transcendentals.
2. **Mandatory Ambient Status Footer Template (`Agent Rules/footer.md`)**:
   Conclude chat responses with:
   ```text
   Focus: <Current Focus><br>
   [ 📬 Mailbag ](sidecar://nkai/panel/?board=morning_mail) · [ 🧭 Agenda ](sidecar://nkai/panel/?board=agenda) · [ 🛫 Runway ](sidecar://nkai/panel/?board=runway) · [ 📝 Scratchpad ](file:///C:/_/Vault/SCRATCHPAD.md) · [ 📖 Cheat Sheet ](file:///c:/Dev/TheKlangSuite/docs/CHEAT_SHEET.md)<br>
   Ecosystem: TKS-v0.4.0 · TT-v0.1.0 · NK-v0.1.0 · PCDA-v1.1.2
   ```
