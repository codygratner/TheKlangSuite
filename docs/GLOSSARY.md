# The Klang Suite: Official Architectural Glossary & Taxonomy

> [!NOTE]  
> This glossary is the single source of truth for terminology across UI design, synthesis audio architecture, and Git release lifecycles in The Klang Suite. All documentation, code reviews, and JSON data schemas must adhere to these definitions.

---

## 1. UI Surface & Spatial Hierarchy

| Term | Scope | Definition & Industry Standard | The Klang Suite Implementation |
| :--- | :--- | :--- | :--- |
| **Chassis** | Structural | The master application window/frame housing global headers, footers, visualizers, and the main canvas. | The top-level `FarmerEditor` / `PlanterEditor` window (800x600 to 4K resizable). |
| **Page** | View | A full-canvas surface swap. Swapping pages replaces the displayed cards while maintaining global headers and audio playback. | `FarmerEditor` Page 1 ("Main Synth"), Page 2 ("FX Rack 1-4"), Page 3 ("FX Rack 5-8"). |
| **Card** | Visual UI | A rectangular visual UI container with an accent color, title bar, background styling, and slot grid. | Cards 1–8 in Farmer; Cards 1–8 in Planter (defined structurally in `assets/layouts/*.json`). |
| **Module** | Audio DSP | The functional audio-processing unit. A Module generates or alters sound; a Card is the visual UI that exposes it. | S&H Filter, Carrier 1, Master Limiter. (One Card controls one Module; Callouts can also control Modules). |
| **Slot** | Layout | A discrete coordinate inside a Card's grid where a control widget resides. | Row/column cells defined in `tkf_layout.json` (e.g. `row: 0, col: 1`). |
| **CalloutBox / Popover** | Transient UI | A temporary floating mini-panel anchored to a specific trigger badge or control, launched by click or right-click. | `juce::CallOutBox` used for Planter Master Limiter, Snap Point menus, and Color Pickers. |

---

## 2. Interactive Controls & Widgets

| Term | Visual Style | Interaction & Behavior | The Klang Suite Implementation |
| :--- | :--- | :--- | :--- |
| **Slider** *(Meter Slider)* | Horizontal recessed trough with filled color bar | Dragging horizontally or vertically fills the bar; embedded label on left, value on right. | Primary signature widget across Cards 1–8 (arcade HP meter style). Under the hood: `RotaryKnobSlider` class. |
| **Knob** | Circular rotary dial | Vertical drag rotates pointer/indicator around pivot. | Used in floating Callout mini-cards (Planter Limiter) and hardware-inspired controls. |
| **Fader** | Vertical linear slide potentiometer | Vertical axis drag exclusively reserved for mixing channel levels and summing. | Reserved for mixer channel strips (Card 8 channel levels). |
| **Selector** | Text badge / pill stepper | Clicking opens dropdown or cycles through discrete choice options. | Waveform shapes, MIDI tracking modes, filter types (`DiscreteSelector`). |
| **Toggle** | 2-state latching button / LED | Binary click toggles state between on/off (`0.0` or `1.0`). | Header `TOOLTIPS`, `BYPASS` switches, modal toggles. |

---

## 3. Synthesis & Audio DSP Taxonomy

| Term | Domain | Definition & Industry Standard | The Klang Suite Implementation |
| :--- | :--- | :--- | :--- |
| **Carrier** | FM Audio | An operator whose audio output is **heard directly** in the mix. | Carrier 1 & Carrier 2 (audible sound generators feeding the mixer). |
| **Modulator** | FM Audio | An operator whose audio output is **inaudible directly**, but modulates the phase/frequency of a Carrier. | Modulator 1 & Modulator 2 (generating metallic harmonic sidebands). |
| **Operator** | FM Audio | The complete FM synthesis building block: Oscillator + Dedicated Envelope Generator + Level Scaling. | Chowning / Yamaha DX7 standard architecture. |
| **Engine** | Master Processor | The master audio container managing voice allocation, MIDI event parsing, and audio bus routing. | `TheKlangFarmerAudioProcessor`, `TheKlangPlanterAudioProcessor`. |
| **Voice** | Audio Instance | A single independent sound execution instance that renders one drum hit (tracks active phase, envelope state, filter memory). | Individual polyphonic / monophonic voice renderers in DSP code. |
| **DSP Block** | Algorithm | The C++ class implementing a specific mathematical audio routine. | `juce::dsp::LadderFilter`, `TbdAudio::FastMath`, `SvfFilter`. |
| **Klang-Brain** | Generative Engine | 4-mode algorithmic sequencer & modulation generator (Fugue Counterpoint, Matriceal Polymeter, Stage Accumulators, Turing LFSR) with Continuous and DSI Pro 2 Triggered Key-Advance modes. | Deployed as rhythm trigger engine in TKR-1 and multi-lane stepped/continuous modulation source in TKF. |

---

## 4. DevOps, Git & Release Lifecycles

| Term | Mechanism | Analogy | The Klang Suite Protocol |
| :--- | :--- | :--- | :--- |
| **Commit** | Local Git record | Checkpoint save on local drive | Atomic, descriptive commit (`feat:`, `fix:`, `docs:`) created via `git commit`. |
| **Push** | Network upload | Syncing local save files to cloud | `git push origin <branch>` to publish local commits to GitHub. |
| **Branch** | Isolated timeline | Parallel universe for experimentation | `main` (stable production), `<version>-dev` (e.g. `0.3.2-dev` active milestone). |
| **Tag** | Immutable pointer | Stamping an official serial number on a build | Annotated Git tag (`git tag -a v0.3.1 -m "Release v0.3.1"`) frozen on one commit forever. |
| **Version (SemVer)** | Numerical standard | Vehicle model year & trim | `MAJOR.MINOR.PATCH` (`v0.3.2`): Breaking changes = Major; Features = Minor; Fixes = Patch. |
| **Milestone** | Project container | The sprint checklist | Roadmap container in `docs/BACKLOG.md` grouping features and fixes targeting a version. |
| **Release** | Public distribution | Placing boxed product on retail shelves | Published GitHub Release bundled with installer artifacts (`.vst3`, `.exe`, `.pkg`) and changelogs. |

> [!TIP]
> **The "Silent Git Tag" Policy**: When a milestone consists strictly of internal architectural cleanup, developer tooling, or schema refactors with zero user-facing sound/UI changes (e.g. `v0.3.2`), we stamp and push an annotated Git Tag (`git tag -a vX.Y.Z`) and merge to `main`, but do not publish a public GitHub Release entry. This preserves clean SemVer and reproducible builds while preventing spurious "Update Available" notifications in end-user DAWs.

---

## 5. Ecosystem Product Acronym Registry & Collision Guardrails

To prevent identity collisions across codebases, logging tags (`TKS_LOG`), asset files, and user documentation, every software plugin, standalone app, and hardware spin-off in The Klang Suite is assigned a strict, immutable 3-letter acronym (`TK<X>`):

| Acronym | Product Name | Category | Scope / Role |
| :--- | :--- | :--- | :--- |
| **`TKS`** | **The Klang Suite** | **Umbrella / Suite** | Master project repository, shared utility libraries, and developer logging (`TKS_LOG`). *RESERVED EXCLUSIVELY FOR THE SUITE.* |
| **`TKF`** | **The Klang Farmer** | Instrument Plugin | Dual-FM percussion and bass synthesizer. |
| **`TKP`** | **The Klang Planter** | Instrument Plugin | Multi-layer percussive sound machine. |
| **`TKM`** | **The Klang Mill** | FX Plugin / Sub-Rack | 1x6 modular multi-effects pedalboard rack. |
| **`TKE`** | **The Klang Editor** | Standalone Desktop App | JSON schema, theme calibration, and curve inspection workstation. |
| **`TKB`** | **The Klang Box** | Embedded Hardware | Standalone hardware synthesizer (Daisy Seed / Teensy 4.1 / dadamachines TBD-16). |
| **`TKR`** | **The Klang R1** | Instrument Plugin | 7-voice Electribe ER-1 tribute rhythm synthesizer. |
| **`TKC`** | **The Klang Cultivator** | Utility Plugin | Standalone MIDI CC / CV / MPE modulation generator rack. |

> [!WARNING]
> **Acronym Collision Guardrail**: No new plugin or spin-off product may adopt an acronym that collides with registered letters (`S`, `F`, `P`, `M`, `E`, `B`, `R`, `C`). `TKS` is strictly forbidden for individual plugins.
> 
> **The Klang Box (`TKB`) vs. The Klang-Brain**: `TKB` is officially reserved for **The Klang Box** embedded hardware platform. The generative algorithmic modulation and sequencing engine is designated **The Klang-Brain** (internal engine code `TKG` - *The Klang Generator / Genome* if an acronym is required), preserving `TKB` for hardware.

---

## 6. Decision & Engineering Review Frameworks

| Framework / Term | Category | Definition & Classical Foundation | The Klang Suite Implementation & Protocol |
| :--- | :--- | :--- | :--- |
| **`PCDA`** | Review Protocol | **Pros, Cons, Devil's Advocate**. A 3-dimensional evaluation framework designed to counter confirmation bias by pairing positive/negative tradeoffs with active contrarian stress-testing. | Invoked when the user or agent requests a rapid decision breakdown (`"give me the PCDA on this"`). Structured with an explicit Devil's Advocate / Antithesis breakdown. *Deliberately designated PCDA rather than PCA to prevent collision with Principal Component Analysis (DSP/ML) and Printed Circuit Assembly (hardware).* |
| **`Dialectic`** *(Hegelian Dialectic)* | Decision Model | The classical philosophical method of investigating truth through the opposition of contradictory assertions: **Thesis $\rightarrow$ Antithesis $\rightarrow$ Synthesis**. | Applied systematically during architectural forks, RFCs, and triage sprints: <br>1. **Thesis** = The core proposal / Option (Pros & intended benefits). <br>2. **Antithesis** = The Devil's Advocate (Cons, failure modes, real-world precedent, maintenance debt). <br>3. **Synthesis** = The Pragmatic 80/20 Middle Ground (extracting 80% of the value with 20% of the complexity). |
| **`Thesis`** | Dialectic Component | The foundational proposal, design hypothesis, or architectural direction. | Presented as Option A/B/C in RFCs, PLANs, or Active Workshop cards. |
| **`Devil's Advocate [Antithesis]`** | Dialectic Component | The adversarial counter-argument tasked with exposing unstated assumptions, fatal edge cases, token bloat, and cognitive debt. | Rendered by the `contrarian` subagent or formatted under the heading `Devil's Advocate [Antithesis]` in chat and sidecar reviews. |
| **`The 80/20 Resolution [Synthesis]`** | Dialectic Component | The pragmatic, battle-tested middle ground reconciling the Thesis with the Antithesis. | The mandatory conclusion of all PCDA reviews, preventing endless philosophical debate and driving decisive factory execution. |
| **`Re-sequence / Rejigger`** | Workflow Action | Topologically auditing, consolidating, and re-ordering roadmap and triage topics based on prerequisite dependencies. | Triggered via `/audit-reorder`, `/rejigger`, `/re-sequence`, or the `[ 🔄 Re-sequence Flights ]` header button in N'kai sidecars. |
| **`2-Pass Convergence Gate`** | Execution Protocol | Multi-agent iterative reflection pattern enforcing a strict 2-pass cap (Pass 1: Flash PCDA exploration $\rightarrow$ Pass 2: Pro topological synthesis). | Prevents AI echo chambers, infinite review looping, and quota exhaustion while ensuring 100% prerequisite alignment before factory execution. |
| **`Subagent Swarm`** | Multi-Agent Pattern | A tripartite delegation pattern where the main chat orchestrator remains permanently in Tier 2 Flash High and delegates specialized investigations to targeted subagents. | Defined by the triad: `research` (Tier 1 Pro sprint for DSP math & multi-file synthesis), `explore` (Tier 2 Flash marathon for UI/UX sweeps), and `contrarian` (Tier 2 Flash adversarial devil's advocate). |
| **`Targeted Opus DSP Swap Protocol`** | Model Governance | A human-in-the-loop surgical model switching workflow where the user pairs with Gemini 3.8 Flash High for 95% of daily engineering and temporarily swaps the IDE footer dropdown to `Claude 5.5 Opus (Thinking: High)` *strictly* when prompted by a Tier 1 Model Advisory banner for heavy C++ DSP algorithms or 2-strike factory escalations. | Codified in `GEMINI.md` to prevent "Ferrari in a traffic jam" token burn while guaranteeing frontier reasoning for real-time differential equations and SIMD FastMath. |
| **`Mandatory Downgrade Chime`** | Quota Safeguard | A prominent completion banner (`🟢 TASK CONCLUDED: Downgrade to Flash High`) emitted immediately upon finishing a Tier 1 DSP phase or during `task-finish`. | Immunizes the user against the "Forgetful Downgrade Trap", preventing accidental burning of premium Opus allowance on routine documentation, JSON schemas, or CMake edits. |
| **`Argon Reconvene Trigger`** | Roadmap Trigger | An architectural reconvene milestone event scheduled to fire when Google's **Gemini 4 Argon** reaches public General Availability in the IDE dropdown. | Formally initiates a head-to-head C++ audio DSP benchmark between Gemini 4 Argon and Claude 5.5 Opus High to determine the primary Tier 1 engine. |
| **`Lean Chat Footer`** | Ambient Telemetry | A clean, single-line status footer providing direct 1-click side pane access via `[ 📬 Open N'kai Board ](sidecar://nkai/panel/)`, omitting legacy repetitive subagent labels while preserving the SemVer roadmap track. | Appended to active chat responses to provide 1-click sidecar opening without noisy visual clutter. |

> [!TIP]
> **PCDA Conversational Format**: When the user requests a "PCDA" on any topic, agents format the response systematically:
> 1. **Pros [Thesis]**: What works, immediate velocity, and user value.
> 2. **Cons [Thesis Friction]**: Known drawbacks, implementation effort, and direct costs.
> 3. **Devil's Advocate [Antithesis]**: Structural failure modes, hidden maintenance debts, and historical production disasters.
> 4. **The 80/20 Resolution [Synthesis]**: The recommended pragmatic architecture extracting maximum value with minimum complexity.

---

## 7. Note Triage, Workflows & Queue Management (The Mailbox vs. The Mailbag)

| Term | Domain | Definition & Purpose | The Klang Suite Implementation |
| :--- | :--- | :--- | :--- |
| **`Mailbox`** 📮 | Physical Storage | The physical Obsidian directory where raw, unorganized quick notes are dumped from mobile or desktop. | `TheKlangVault/Inbox/` (and archived to `TheKlangVault/Inbox/Archive/`). |
| **`Mailbag`** 🎒 | Interactive Session | The persistent N'kai interactive triage session board and historical ledger. | `morning_mail_inbox.html` & `.agents/pipeline/mailbag/mail_queue.json`. |
| **`Persistent Mailbag Queue`** | Session Ledger | A decoupled state ledger tracking note triage statuses, priority levels, and metadata independently of physical file movements. | `.agents/pipeline/mailbag/mail_queue.json`, ensuring archiving in Obsidian does not wipe active sidecar state. |
| **`Topological Rejigger Intercept`** | Dependency Gate | Dynamic dependency-checking logic that flags when a high-priority (`P0`) note depends on an unbuilt prerequisite. | Suggestion chip rendered on Mailbag cards: `[ 🔄 Slot into Topological Order ]` vs `[ ⚡ Keep at Top ]`. |
| **`Plans Queue`** | Agent Orchestration | A persistent queue managing multi-plan staging, execution sequences, and active execution pointer. | `.agents/pipeline/plans/queue.json`, coordinating plan transitions from `drafts/` to root `PLAN.md` to `completed/`. |
| **`Stateless Atomic Rollback`** | Fault Recovery | A multi-tier rollback mechanism restoring previous states via immutable pre-transaction receipts. | Implemented in `check_mail.py --rollback`, `audit_reorder.py --rollback`, and `clean-plan --rollback`. |
| **`Living Archive Standard`** 🏛️ | Knowledge Governance | A continuous, non-versioned research hub where Git commit history is the single source of truth. | Applied strictly to `TheKlangResearch` (zero software SemVer, zero manual `CHANGELOG.md`; RFCs, whitepapers, and soul harvests tracked via Git SHAs and timestamps). |

---

## 8. Aviation Metaphors & Execution Terminology (Flight Plans, Runways & Test Flights)

To replace confusing corporate Agile jargon (*Sprints*, *Epics*, *Story Points*) with intuitive, spatial engineering concepts, The Klang Suite employs an aerospace flight-deck metaphor across all planning, triage, and building pipelines:

| Aviation Term | Engineering Equivalent | Definition & Purpose | The Klang Suite Protocol |
| :--- | :--- | :--- | :--- |
| **`Flight`** 🛫 | Incremental Step | A single discrete, bounded implementation phase or milestone increment (e.g. `Flight 1` through `Flight 8`). | Decoupled from topic ID; represents the linear integer execution order (`Flight N`). |
| **`Flight Plan`** 📋 | Implementation Plan | The synthesized, topologically ordered blueprint detailing exact dependencies, schemas, code diffs, and verification steps. | Formatted strictly in root `PLAN.md` (or staged in `.agents/pipeline/plans/drafts/`). |
| **`Runway`** 🛫 | Prerequisite Sequence | The foundational sequence of infrastructure flights required before a major feature can take off safely without crash landing. | e.g. The 8-Flight N'kai Runway establishing IPC, schemas, and headers before building views. |
| **`Test Flight` / `Test-Drive`** 🧪 | Verification Run | Taking an interactive sidecar, UI harness, or new tool out for a spin using realistic data packages before full production deployment. | e.g. The 12-Topic Test-Drive Docket loaded into `morning_mail_inbox.html`. |
| **`Docket`** 📑 | Pre-Flight Queue | The curated queue of active topics, notes, or feature proposals staged and ready for review in an interactive triage lab. | Sourced from `morning_mail_topics.md` and parsed into the Mailbag canvas. |
| **`Holding Pattern`** ⏳ | Staged Buffer | Keeping draft plans or brainstorms parked in a clean holding state without mutating the active flight deck. | Stored in `.agents/pipeline/plans/drafts/` until the factory is idle and allowance is refreshed. |
| **`Re-sequence / Rejigger`** 🔄 | Topological Sorting | Auditing and re-ordering flights based on strict prerequisite dependencies so foundations precede surfaces. | Executed via `/audit-reorder`, `/rejigger`, or the `[ 🔄 Re-sequence Flights ]` sidecar button. |
| **`Wheels Up`** 🚀 | Kickoff Execution | The formal transition of an approved Flight Plan into active factory implementation (`STATUS: IN_PROGRESS`). | Triggered by user saying "proceed" or running `/execute-task`. |
| **`Touchdown`** 🛬 | Task Completion | Concluding an active flight plan, passing all acceptance tests, committing cleanly, and archiving the plan. | Executed via `/task-finish` or `task-finish` skill. |

### The Operatic Flight-Stage Taxonomy (Acts & Flight Tiers)

When complex, multi-milestone initiatives are blueprinted, flight plans are structured operatically into **Acts** divided into discrete **Scenes (Flights)** across four altitude stages:

1. **Act I: Ground Control & Taxiway (Early Flights)** 🛫:
   - Foundational runway preparation, multi-plan queues, zonal boundaries, git baseline setup, and environment verification.
   - Clears the flight deck before any heavy code or data pipelines take off.
2. **Act II: Ascent & Cruising Altitude (Main Flights)** ✈️:
   - Core operational engines, Pydantic data pipelines, linters, and APIs running at full cruising altitude and speed.
   - Heavy data lifting, real-time validations, and interactive triage workflows.
3. **Act III: High-Altitude Reconnaissance & The Gauntlet (Late Flights)** 🛰️:
   - Comprehensive cross-repo synchronization, field guide decoupling (Universal Vibe-Coding Manifesto vs. Real-Time C++ Audio DSP Bible), and subagent soul harvesting.
   - Ensures zero institutional knowledge or architectural breakthroughs are lost across sessions.
4. **Act IV: Final Approach & Touchdown (Pre-Release Capstone)** 🛬:
   - Pre-release verification gauntlet, catalog registration, SemVer tagging, and test-drive docket review before formal deployment.

---

## 9. Architectural Zonal Boundaries (The 3-Zone README Standard)

To eliminate token bloat, maintenance rot, and synchronization drift caused by placing blackbox files in every subfolder (the "README Cemetery" anti-pattern), The Klang Suite enforces the **3-Zone Architectural Standard**:

| Zone | Primary Boundary | Directory Root | Core Invariants & Governance |
| :--- | :--- | :--- | :--- |
| **Zone 1: Native Audio DSP** | Audio Thread & DSP Engine | `Source/dsp/README.md` | Strict real-time safety: zero heap allocations in `processBlock()`, zero mutex locks, zero blocking I/O, SIMD `FastMath`, thread-safe SPSC ring buffers. |
| **Zone 2: Data Schemas & Assets** | 4-Layer Parameter Contracts | `assets/README.md` | Strict data separation: `controls/` (APVTS bounds), `layouts/` (UI geometry), `themes/` (visuals), `text/` (strings). Normalized $[0.0, 1.0]$ ranges. |
| **Zone 3: Tooling Harness** | Dev Scripts & Sidecars | `tools/README.md` | Tooling isolation: Zero external dependencies in Python scripts (standard library only, UTF-8 stdlib), Node CLI wrappers, and clean bridge servers. |

---

## 10. Permanent Documentation Invariant (Glossary & Cheat Sheet Sync)

* **Mandatory Dual-File Synchronization**: Whenever terminology, workflow metaphors (such as aviation flight stages), agent skills, or system guardrails are created, renamed, or modified, the agent MUST proactively update both [`docs/GLOSSARY.md`](file:///c:/Dev/TheKlangSuite/docs/GLOSSARY.md) and [`docs/CHEAT_SHEET.md`](file:///c:/Dev/TheKlangSuite/docs/CHEAT_SHEET.md) in the exact same commit pass.
* **Secondary Directory Pointers**: Core directories point back to the centralized Glossary and Cheat Sheet rather than authoring rogue, fragmented dictionaries.

---

## 11. Telescoping Soul Harvest Standard (Staging vs. Distillation)

* **Opportunistic Post-Flight Staging (The Digital Shoebox)**: Dropping unvarnished notes, mathematical derivations, or bug autopsies into `.agents/pipeline/staging/souls/` immediately upon flight completion. Captures fresh context at 0 token cost before chat compaction.
* **Batch Post-Plan Distillation (Inbox Zero)**: Vacuuming staged souls during milestone capstone deep docs passes (`/update-docs --deep`), formalizing them into canonical RFCs in `TheKlangResearch`, indexing into `Master_Research_Index.md`, and purging staging to empty.
* **The "Don't Harvest Noise" Rule**: Routine mechanical tasks (dependency installs, routine refactors) are never staged; only genuinely novel architecture or non-obvious gotchas qualify.

---

## 12. The "Field Guides Always Last" Invariant

* **Topological Synthesis Sequence**: In all deep documentation passes (`/update-docs --deep` or Act III), low-level raw soul harvests, bug autopsies, backlogs, and technical specs MUST be ingested and cataloged *first*. High-level Field Guides (`vibe_coding_field_guide.md`, `audio_dsp_field_guide.md`, `WORKFLOW_CHEAT_SHEET.md`, and manifestos) MUST be synthesized **LAST**.
* **Panoramic Pedagogical Synthesis**: Ingesting code diffs, gotchas, and raw notes first provides the author with the complete, panoramic picture. The agent can then boldly synthesize new pedagogical models, unify metaphors, and refactor explanations for maximum clarity rather than appending fragmented, disjointed bullet points.
* **Git & Obsidian History Safety**: Because Git and Obsidian sync provide complete, immutable historical versioning across all repositories, authors and agents have total license to radically refactor and elevate field guides without fear of losing previous iterations.

---

## 13. Rolling Blackbox Context Compaction & Institutional Memory (Topic #7)

* **The Amnesia Prevention Tripwire**: Deep multi-flight pair programming generates nuanced architectural context. To prevent catastrophic amnesia when a chat session reaches context limits or prior to running `/clear`, agents trigger the 4-step memory flush.
* **The 4-Step Pre-Compaction Flush Protocol**:
  1. *Dev History Ingestion*: Append session accomplishments to `docs/history/DEV_HISTORY.md`.
  2. *Soul Harvest Staging*: Drop unexpected gotchas, autopsies, and PCDA dialectics into `.agents/pipeline/staging/souls/<YYYY-MM-DD>_<slug>.md`.
  3. *Context Clues Hardening*: Update role-specific context clues index (`context_clues_plan.md`, `context_clues_build.md`, or `context_clues_subagent.md`) with active flights, branches, targets, and uncommitted files.
  4. *Link Integrity Gate*: Execute `python tools/verify_context_clues.py` to ensure 100% link resolution.
* **The 0-Turn Cold Boot Invariant**: Following `/clear` or starting a fresh session, reading `context_clues_*.md` allows the agent to boot in **0 turns**, seamlessly resuming the active task without asking redundant orientation questions ("What should we work on?").
* **Obsidian Vault Archive Index & Asynchronous Enrichment Standard**: The knowledge management pattern where `TheKlangVault/Inbox/Archive/INDEX.md` is maintained mechanically by zero-token Python automation (`tools/check_mail.py`), paired with zero-rewrite live triage and deferred batch enrichment under `/update-docs --deep`.
* **The Curator's Trap**: The agentic failure mode of pausing active engineering/triage sessions to re-read, rewrite, and format raw historical notes one-by-one in real-time, burning expensive LLM tokens instead of deferring to zero-cost mechanical tools or async batch sweeps.

## 14. Trust, But Verify (Pre-Flight Grounding Rule)

* **Pre-Flight Grounding**: Before adopting a premise (framework version, model name/vendor, file location, "this already exists"), the agent verifies it — local files first (`CMakeLists.txt`, `GEMINI.md`, version headers), web second for model/vendor/upstream claims.
* **Reality Check**: When a premise is wrong, the agent opens with a polite one-line correction before answering, instead of silently building on it. Origin: the "JUCE 8 vs 9" and "Pro 3.8 is Argon" whoopsies (Mailbag `rj-06`, 2026-10-11).

## 15. One-Way Vault Mirror (Absolute Invariant)

* **One-Way Vault Mirror**: `TheKlangVault` is a generated, read-only mirror. Content flows **repo → vault only**; nothing in the vault is ever synced, copied, or merged back into a repo. Vault-side edits outside `Inbox/` are disposable.
* **Inbox Exception**: `Inbox/` notes are *read* by intake tools (`check_mail.py`, `ramble_ingest.py`) and turned into repo ledger/backlog entries; the note files themselves never enter a repo. Changing this requires an explicit user decision ("burn that bridge when we get there").

## 16. Audit Cadence & Checkpoint Resumption Protocol

* **Dual-Tier Cadence**: Light milestone audits (Flash High) run at every patch release tag; Deep Opus audits (Claude 5.5 Opus High) run once per minor version (`v0.4.0`, `v0.5.0`) or before weekly quota reset.
* **Checkpoint Resumption**: Deep cross-repo sweeps save progress to `audit_checkpoint.json` per repo, allowing 0-turn resumption if interrupted by rate/token limits.
* **Findings Board**: Audit findings populate an N'kai board rather than flooding chat history (Mailbag `rj-11`).

---

## 17. The North Star Focus & Ambient Status Protocol (Mailbag `rj-09`)

* **The North Star Focus Anchor (`docs/FOCUS.md`)**: The living compass document that declares the active mission, the 4-pillar precedence order (`N'kai Solidification` → `TKS v0.4.0 UI Shells / v0.5.0-dev Sound & Chaos` → `ToadTracker PoC`), and strict Negative Architecture (what we are *not* doing right now).
* **The Gentle Re-Centering Nudge**: The conversational habit where an agent wrapping up a deep research spike, dialectic teardown, or creative tangent appends a single-sentence compass check to re-orient onto the active milestone without blocking execution (`🧭 North Star Check: Returning from [Detour] to active focus: [Target]`). Nudge, not a gate.
* **Ambient Status Footer Line**: The unified, single-line footer appended to responses:
  `Focus: <Current Focus> | TKS-<ver> · TT-<ver> · NK-<ver> · TR-<ver>`
  providing ambient system orientation across multi-repo solo vibe-coding sessions.






