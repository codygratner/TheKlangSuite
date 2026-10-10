# Context Clues: Subagent Quick-Reference Index

> **Step 0 Bootstrapping**: Subagents waking up from `invoke_subagent` MUST read this document on Turn 1 to orient on active SemVer, task status, architectural memory, canonical taxonomy, and inviolable guardrails without prompt bloat.

---

## 📌 Active SemVer, Branch State & Roadmap Sidecar Deep Links
- **Current Git Branch:** `0.4.0-dev`
- **Ecosystem SemVer Matrix:**
  - `The Klang Suite (TKS)`: `v0.4.0-dev`
  - `ToadTracker (TT)`: `v0.1.0`
  - `N'kai (NK)`: `v0.1.0`
  - `The Klang Research (TKR)`: `v0.1.0`
- **Roadmap Sidecar Deep Links**:
  - [The Klang Suite v0.4.0 Roadmap](file:///c:/Dev/TheKlangSuite/.agents/sidecar/roadmap_sidecar.html#tks-v0.4.0)
  - [ToadTracker v0.1.0 Milestone](file:///c:/Dev/TheKlangSuite/.agents/sidecar/roadmap_sidecar.html#tt-v0.1.0)
  - [N'kai v0.1.0 Sidecar Framework](file:///c:/Dev/TheKlangSuite/.agents/sidecar/roadmap_sidecar.html#nk-v0.1.0)
  - [The Klang Research Central Hub](https://github.com/codygratner/TheKlangResearch)
- **Active Working Milestone:** `v0.4.0 "The Interface & Experience Update"` (🔨 In Factory Execution)

---

## 🎯 Immediate Return Agenda & Status
- **Current Objective:** Subagent Task Execution & Orientation
- **Operating Modes & Role Definitions**:
  - **`research` (`Model="pro"` / Tier 1 Sprint)**: Deep DSP differential equations, non-linear saturation proofs, lock-free ring-buffer concurrency, and complex multi-file architectural synthesis.
  - **`explore` (`Model="flash"` / Tier 2 Marathon)**: Broad UI/UX competitive benchmarking, community sentiment analysis (Reddit/KVR/Gearspace), preset browser taxonomy, and responsive layout sweeps.
  - **`contrarian` (`Model="flash"` / Tier 2 Advocate)**: The Devil's Advocate; stress-tests proposals, identifies hidden architectural traps, dependency bloat, and derives the pragmatic 80/20 middle ground.
  - **`factory implementation` (`Model="flash"` / Tier 2)**: Autonomous file modification, test execution, and compiler fix loops delegated from main chat.
- **Active Blueprint:** [`PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md)

---

## 🧠 Architectural Memory & "Unforgettables"
1. **Model Advisory & Quota Economics**:
   - Subagents run autonomously and return concise receipts to the parent agent.
   - Sprints default to `Model="flash"` unless deep DSP mathematics or concurrency proofs strictly demand `Model="pro"`.
2. **Data-Driven Separation of Concerns**:
   - Parameters strictly in `assets/controls/*.json`.
   - Layout strictly in `assets/layouts/*.json`.
   - Themes strictly in `assets/themes/*.json`.
   - Text strictly in `assets/text/*.json`.
3. **State & Persistence Standards (Topic 6.16)**:
   - Zero embedded binary databases (zero SQLite, zero LevelDB, zero RocksDB).
   - 3-Tier State: Frontmatter Markdown $\rightarrow$ Deterministic JSON session packages $\rightarrow$ Browser `localStorage`.
4. **Synthesis Engine Invariants**:
   - Continuous `_slope` controls exist natively on all envelopes (`ampenv_slope`, `filterenv_slope`).
   - Hardware Target: dadamachines TBD-16 (ESP32-P4 @ 400MHz, PSRAM budget).
5. **Obsidian Vault & Sync Guardrails**:
   - `TheKlangVault` is an independent Git repository housing the Mailbox, studio notes, and Pydantic data models.
   - Obsidian App ignores (search, graph, quick switcher) are defined in `TheKlangVault/.obsidian/app.json` (`userIgnoreFilters`).
   - Obsidian Sync natively ignores all dotfiles (`.git/`, `.gitignore`, `.venv/`, `.agents/`). Never place raw Python tools directly in root notes; keep scripts in `tools/` and excluded from sync.

---

## 📖 Canonical Taxonomy, Product Lineup & Essential Spec Links
- **Master Documentation Catalog**: [`docs/DOCS_CATALOG.json`](file:///c:/Dev/TheKlangSuite/docs/DOCS_CATALOG.json) (100% indexed documentation ledger).
- **System Architecture Map**: [`docs/SYSTEM_MAP.md`](file:///c:/Dev/TheKlangSuite/docs/SYSTEM_MAP.md) (central ecosystem index).
- **Architectural Taxonomy & Glossary**: [`docs/GLOSSARY.md`](file:///c:/Dev/TheKlangSuite/docs/GLOSSARY.md)
  - *Cards* = UI containers, *Modules* = DSP units, *Sliders* = meter bars, *Knobs* = rotary controls.
  - Acronym Registry: `TKS` (Suite), `TKF` (Farmer), `TKP` (Planter), `TKM` (Mill), `TKB` (Box/Hardware), `TKR` (R1), `TKC` (Cultivator).
- **Product Intent Matrix**: [`docs/architecture/product_lineup.md`](file:///c:/Dev/TheKlangSuite/docs/architecture/product_lineup.md) ("Stay in Your Lane" drift guardrail).
- **Universal Project Backlog**: [`docs/BACKLOG.md`](file:///c:/Dev/TheKlangSuite/docs/BACKLOG.md)
- **Active Technical Specs**:
  - [N'kai Canonical State & Persistence Architecture](file:///c:/Dev/TheKlangSuite/docs/specs/nkai_canonical_state_architecture.md)
  - [The Klang R1 Modular Drum Synthesizer](file:///c:/Dev/TheKlangSuite/docs/specs/spinoffs/the_klang_r1.md)
  - [The Klang Mill Pedalboard Multi-FX](file:///c:/Dev/TheKlangSuite/docs/specs/spinoffs/the_klang_mill.md)
  - [TBD-16 Klang Seed Hardware Integration](file:///c:/Dev/TheKlangSuite/docs/specs/tbd16_klang_seed_effects.md)

---

## ⚡ Audio Thread / Workflow Invariants & Handshake Gates
1. **Real-Time Audio Thread Invariants (CRITICAL)**:
   - **Zero Allocations**: Never call `new`, `malloc`, `free`, or resize dynamic containers (`std::vector::push_back`, `juce::Array::add`) in `processBlock()`, `processStereo()`, or per-sample render loops. Pre-allocate in `prepareToPlay()`.
   - **Zero Locks**: Never acquire `std::mutex`, `juce::CriticalSection`, or wait on thread synchronization primitives in the audio path.
   - **Zero Blocking I/O**: Never call filesystem operations, network APIs, or logging/console output (`std::cout`, `printf`, `DBG()`, `juce::Logger`) on the audio thread.
   - **SIMD & Fast Math**: Prefer `TbdAudio::FastMath` over standard CRT transcendentals (`std::pow`, `std::sin`, `std::tanh`) in hot audio loops.
2. **Web Access Policy & The `--no-web` Override**:
   - Subagents are pre-authorized to use `search_web` and `read_url_content` by default.
   - If the prompt specifies `--no-web` or `--offline`, strictly restrict research to local workspace files and `TheKlangResearch`.
3. **Subagent Delegation Protocol (The 20-Line / 1-File Rule)**:
   - *< 20 lines, 1 file*: Execute directly in-chat (Tier 2 Flash active).
   - *> 20 lines, multi-file, or running builds/tests*: Delegate to Flash subagent (`invoke_subagent(Model="flash")`).
   - *2-Strike Factory Escalation*: If a subagent fails 2 compilation/test attempts, pause and escalate to Pro (`Model="pro"`).
4. **Completion Receipt Format**:
   - Subagents return only a concise diff summary, verification status, and git state to the parent agent upon completion.
