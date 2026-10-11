# Agent Guidelines & Guardrails

## Audio Thread Invariants (CRITICAL)
- **Zero Allocations**: Never call `new`, `malloc`, `free`, or resize dynamic containers (`std::vector::push_back`, `juce::Array::add`, `std::string` concatenation) inside `processBlock()`, `processStereo()`, or per-sample render loops. Pre-allocate in `prepareToPlay()`.
- **Zero Locks**: Never acquire a `std::mutex`, `std::lock_guard`, `juce::CriticalSection`, or wait on thread synchronization primitives in the audio path. Use lock-free atomics (`std::atomic`) or bounded FIFO queues for audio-to-UI messaging.
- **Zero Blocking I/O**: Never call filesystem operations, network APIs, or logging/console output (`std::cout`, `printf`, `DBG()`, `juce::Logger`) on the audio thread.
- **SIMD & Fast Math**: Prefer `TbdAudio::FastMath` over standard CRT transcendentals (`std::pow`, `std::sin`, `std::tanh`) in hot audio loops.

## Strict Model Advisory Protocol & Anti-Spam Gate
- **Display Only on Kickoff / First Presentation**: The Model Advisory block MUST ONLY be shown when an agent initially presents a new architectural plan or prepares to execute Phase 1 of a task before the user says "proceed". Do NOT repeat or spam the banner on routine conversational turns or within the same tier.
  > 🧠 **MODEL ADVISORY: Tier [1 | 2 | 3]**
  > - **Recommended Setting**: [Claude 5.5 Opus (Thinking: High) / Gemini 4 Argon (upon GA) | Gemini 3.8 Flash (Thinking: High) | Gemini 3.8 Flash (Low/Medium)]
  > - **Quota Impact**: [⚠️ HIGH IMPACT | 🟢 SUSTAINABLE | ⚡ MINIMAL]
  > - **Active Model Check**: Please verify your model dropdown in the IDE footer matches this tier before proceeding!

- **Complexity Tiers**:
  | Tier | Scope | Recommended Model | Quota Impact |
  | :--- | :--- | :--- | :--- |
  | **Tier 1 (High Reasoning / Critical DSP)** | Differential equations, non-linear saturation, polyphonic voice allocation, SIMD FastMath, lock-free concurrency, deep architecture. | Claude 5.5 Opus (Thinking: High) *(or Gemini 4 Argon upon GA)* | ⚠️ HIGH IMPACT (Reserve strictly for deep math/DSP/architecture) |
  | **Tier 2 (Balanced Engineering & UI)** *(Universal Default)* | Standard JUCE UI components, modal dialogs, APVTS bindings, expanding `gui_tests` or `dsp_tests`, standard phase execution. | Gemini 3.8 Flash (Thinking: High) | 🟢 SUSTAINABLE (Fast, highly capable daily driver) |
  | **Tier 3 (Rapid Iteration & Data)** | JSON schemas in `assets/`, documentation/backlog updates, git operations, mechanical find-and-replace, CMake tweaks. | Gemini 3.8 Flash (Thinking: Low/Medium) | ⚡ MINIMAL (Near-zero burn rate) |

- **Tier Transition, Targeted DSP Swap & Verification Rules**:
  - **Flash 3.8 High is the Universal Default**: All planning in New Klang City and building in Klang Industries defaults to Tier 2. Tier 1 (Opus 5.5 High) is strictly an on-demand escalation for heavy DSP math, complex concurrency, or 2-strike factory escalations.
  - **The Targeted Opus DSP Swap Protocol**: The user remains in Gemini 3.8 Flash High for 95% of daily engineering. When entering a heavy DSP phase or 2-strike factory escalation, the agent emits the `🧠 MODEL ADVISORY: Tier 1` banner prompting the user to manually swap to `Claude 5.5 Opus (Thinking: High)` in the IDE footer.
  - **Mandatory Downgrade Chime (`🟢 TASK CONCLUDED`)**: To prevent the "Forgetful Downgrade Trap" (burning expensive Opus allowance on routine chores), the agent MUST emit a prominent completion banner immediately upon completing a Tier 1 phase or during `task-finish`:
    > 🟢 **MODEL ADVISORY: Tier 1 Concluded — Safe to Downgrade**
    > - **Recommended Setting**: Gemini 3.8 Flash (Thinking: High)
    > - **Allowance Protection**: Please swap back to Flash in your IDE footer to preserve Opus quota for future DSP!
  - **Gemini 4 Argon GA Reconvene Trigger**: As of October 2026, Gemini 4 Argon is restricted to the Fairwind Program with no public GA date. When Argon achieves public General Availability in the IDE dropdown, immediately reconvene to benchmark Argon vs. Opus 5.5 High on C++ DSP to determine the primary Tier 1 engine.
  - **Granular Blueprinting (Pro/Opus Plans, Flash Builds)**: When Tier 1 architects a plan, it MUST decompose `PLAN.md` into exact drop-in method signatures, JSON keys, and test assertion lines so Klang Industries executes immediately on Tier 2 (Flash High).
  - **2-Strike Factory Escalation**: Klang Industries has up to 2 attempts to resolve a compilation or test failure on Tier 2. If still failing, it hard-pauses, recommends a Tier 1 upgrade to Opus 5.5 High, and waits for user confirmation.
  - **Tier Upgrade**: Display banner and **hard-pause** for explicit user "proceed". Never auto-proceed into a higher tier.
  - **1-Level Downgrade (Tier 1 ➔ 2, Tier 2 ➔ 3)**: Display banner, schedule a 5-minute grace timer (`schedule(DurationSeconds=300)`), and pause in chat. If timer expires without reply, auto-proceed on existing tier.
  - **2-Level Downgrade (Tier 1 ➔ 3)**: Display banner and **hard-pause indefinitely** without timer to protect critical Pro quota.
  - **Matched Tier Auto-Proceed**: If active model matches or exceeds the recommended tier, output subtle badge `✓ Model Verified: <Model> (<Thinking>) matches Tier <N>` and proceed immediately.
  - **The "Flash Orchestrator + Subagent" Engine**: The main chat session stays permanently on Tier 2 Flash 3.8 High (sustainable daily driver). For targeted background deep dives, the agent autonomously delegates to on-demand subagents (`invoke_subagent`), slashing quota burn and keeping the main chat transcript lean. Manually switching the IDE footer dropdown to Opus 5.5 High is strictly reserved for the Targeted DSP Swap Protocol or 2-strike factory escalations.
  - **Subagent Policy & Naming Convention (`research` vs `explore` vs `contrarian`)**:
    - **`research` (`Model="pro"`)**: Targeted Tier 1 Sprint mode reserved strictly for high-reasoning tasks (DSP differential equations, non-linear saturation proofs, lock-free concurrency proofs, and complex multi-file architectural synthesis).
    - **`explore` (`Model="flash"`)**: Autonomous Tier 2 Marathon mode for broad, goal-oriented sweeps (UI/UX layout explorations, community sentiment analysis across Reddit/KVR/Gearspace, preset browser taxonomy, and broad competitive benchmarking).
    - **`contrarian` (`Model="flash"`)**: The Devil's Advocate adversarial mode paired with exploratory sprints to stress-test proposals, expose edge-case failure modes, and derive the pragmatic 80/20 middle ground.
    - **Subagent Pre-Authorized Web Access & The `--no-web` Override**: By default, research, explore, and web-qa subagents are pre-authorized to use web access tools (`search_web`, `read_url_content`) to query upstream browser standards, developer forums, and competitor architectures without interactive prompts. Appending `--no-web` or `--offline` strictly enforces an air-gapped run restricted to local workspace docs and `TheKlangResearch`.
  - **Session Footer Link**: Chat responses during active sessions must include the clickable N'kai board launcher in the footer: `[ 📬 Open N'kai Board ](sidecar://nkai/panel/)` (replacing legacy subagent text to provide 1-click side pane access).

## Target Toolchain & Standards
- **C++ Standard**: C++20 (`CMAKE_CXX_STANDARD 20`).
- **Framework**: JUCE 9.0.3.
- **Timer Hygiene**: Always call `stopTimer()` as the first line of destructors in all `juce::Timer` subclasses to prevent `#1696` unload crashes.
- **macOS Deployment Target**: Set `CMAKE_OSX_DEPLOYMENT_TARGET="11.0"` and `CMAKE_OSX_ARCHITECTURES="arm64;x86_64"` before `project()` in `CMakeLists.txt`.
- **Linux Portability**: Use `-static-libstdc++ -static-libgcc` and set `VST3_AUTO_MANIFEST FALSE`.

## Strict TODO & Backlog Handling
- **Record Only, Do Not Implement**: When the user mentions items "for the todo", "on the backlog", "for later", or lists future features/fixes, you must **ONLY** document them into `docs/BACKLOG.md`.
- **No Premature Implementation**: Under no circumstances should you edit source code, run compilers, trigger builds, or execute tests for backlog items until the user explicitly directs implementation.
- **Single Source of Truth**: The *only* valid backlog file is `docs/BACKLOG.md`. Never create rogue backlog files (`TODO.md`, `future_backlog.md`, etc.).

## Strict Domain Division: Tooling Harness vs. Native Audio Factory
- **Klang Industries (Factory Floor)**: Reserved strictly for native C++ DSP, JUCE component architecture, CMake/MSBuild targets, APVTS parameter schemas (`assets/controls/`), and real-time audio safety audits (`audiothread-guard`). Never dispatch pure HTML/CSS sidecars, documentation sweeps, or PowerShell/Python dev scripts to the C++ factory.
- **New Klang City (Tooling Harness)**: Owns ecosystem architecture, N'kai sidecars, Python/PowerShell automation, and documentation.
- **The Context Shield Rule**: When New Klang City executes tooling implementation, it must delegate mechanical file-writing and testing to a dedicated Flash subagent (`invoke_subagent(Model="flash")`) to prevent chat transcript bloat and protect user allowance.
- **Dual-Track Communiqué Routing (Anti-Snatching Guard)**:
  - **Factory Pipeline (C++/DSP)**: Sourced through `plan_to_build.md` ➔ `build_to_plan.md` ➔ Klang Industries (`TARGET: FACTORY_FLOOR`).
  - **Tooling Pipeline (Web/Docs/Scripts)**: Sourced through `subagent_dispatch.md` ➔ `subagent_receipt.md` ➔ Flash Builder Subagent (`TARGET: TOOLING_HARNESS`).
  - Under no circumstances should Klang Industries ingest tickets marked `TARGET: TOOLING_HARNESS`.
  - Root `PLAN.md` must declare `TARGET: [FACTORY_FLOOR | TOOLING_HARNESS]` to prevent concurrent file clobbering.

## Strict VST3 Deployment Directory
- **Program Files Only**: When deploying or copying built VST3 plugins on Windows, you must **ONLY** install to `C:\Program Files\Common Files\VST3\The Klang Farmer.vst3` and `C:\Program Files\Common Files\VST3\The Klang Planter.vst3`.
- **Never Use AppData**: Under no circumstances should you copy, deploy, or fallback to `%LOCALAPPDATA%\Programs\Common\VST3` or any local user directories.

## Strict Data-Driven Architecture (CRITICAL)
- **JSON First Priority**: Always default to data-driven solutions for new features, UI layouts, colors, and DSP parameters.
- **No Hardcoded Values (Unless Mandatory)**: Under no circumstances should APVTS parameter IDs, default values, min/max ranges, string labels, snap points, or tooltips be hardcoded in C++ source files unless technically mandatory.
- **Strict Schema Separation of Concerns**: All JSON files must rigidly adhere to their designated layer:
  1. `assets/controls/*.json`: Strictly DSP & APVTS parameter contracts (type, min/max ranges, default, format, snap_points, choices). ZERO visual styling, ZERO colors, and ZERO text descriptions.
  2. `assets/layouts/*.json`: Structural UI surface hierarchy (Pages -> Cards -> array of bound parameter IDs).
  3. `assets/themes/*.json`: Visual styling, color palettes, typography (`theme.json`), and floating overlay/callout geometry, background/border colors, and bound parameter arrays (`callouts.json`).
  4. `assets/text/*.json`: Parameter descriptions, choice tooltips, and localized UI copy (`strings.json`).
- **Architectural Taxonomy**: Adhere strictly to `docs/GLOSSARY.md` (*Cards* = UI containers, *Modules* = DSP units, *Sliders* = meter bars, *Knobs* = rotary controls). Consult the glossary on-demand whenever planning new features or resolving naming ambiguities.

## Permanent Documentation Invariant (Glossary & Cheat Sheet Sync)
- **Mandatory Dual-File Synchronization**: Whenever terminology, workflow metaphors (such as aviation flight stages), agent skills, or system guardrails are created, renamed, or modified, the agent MUST proactively update both [`docs/GLOSSARY.md`](file:///c:/Dev/TheKlangSuite/docs/GLOSSARY.md) and [`docs/CHEAT_SHEET.md`](file:///c:/Dev/TheKlangSuite/docs/CHEAT_SHEET.md) in the exact same commit pass.
- **The 3-Zone Architectural Standard**: Follow the 3-Zone README standard (`Source/dsp/README.md`, `assets/README.md`, `tools/README.md`). Never create rogue per-folder blackboxes.
- **Telescoping Soul Harvest Standard (Capture vs. Distill)**:
  - *Post-Flight Opportunistic Staging*: When a flight or subagent uncovers a non-trivial insight, platform gotcha, bug autopsy, or novel PCDA dialectic, immediately drop raw notes into `.agents/pipeline/staging/souls/<YYYY-MM-DD>_<slug>.md`. Zero token cost; immunizes insights against chat context truncation. Never harvest routine mechanical noise.
  - *Post-Plan Batch Distillation (Inbox Zero)*: During deep docs passes (`/update-docs --deep` or Flight 9/13), vacuum all staged souls, formalize them into permanent RFCs/autopsies in `TheKlangResearch`, compile `Master_Research_Index.md`, and wipe staging clean.
- **The "Field Guides Always Last" Invariant**: In all deep documentation passes (`/update-docs --deep` or Act III), raw soul harvests, bug autopsies, backlogs, and technical specs MUST be ingested and reconciled *first*. High-level Field Guides (`vibe_coding_field_guide.md`, `audio_dsp_field_guide.md`, `WORKFLOW_CHEAT_SHEET.md`, and manifestos) MUST be synthesized **LAST**. Having the complete panoramic picture allows the author to boldly synthesize new pedagogical mental models, diagrams, and frameworks rather than appending fragmented bullet points. Git and Obsidian sync guarantee complete historical versioning, eliminating the fear of radical refactoring.
- **Obsidian Vault Archive Index & Asynchronous Enrichment Standard**:
  - *Mechanical Lean Index*: `TheKlangVault/Inbox/Archive/INDEX.md` is maintained mechanically by Python (`tools/check_mail.py`) at zero token cost. The table format is restricted to: `Title`, `Date`, `Priority`, `Pillar`, and `File Link`, keeping the catalog under 200 lines (<3 KB) for instantaneous $O(1)$ agent lookup.
  - *Zero Live Note Rewriting (Curator's Trap Defense)*: During active chat pair-programming and N'kai triage board interactions, agents MUST NEVER edit, rewrite, or inject metadata into archived raw notes on-the-fly. Triage verdicts are committed in <5ms strictly to `.agents/pipeline/mailbag/mail_queue.json` and local transaction receipts.
  - *Deferred Note Enrichment (Batch under `/update-docs --deep`)*: Enriching original archived notes with resolved status badges, target pillar crosslinks, backlog links, and PCDA summaries is deferred strictly to deep documentation sweeps (`/update-docs --deep`) via dedicated Flash subagents, preserving main session token allowance and pair-programming flow.
  - *Single Source of Truth*: `docs/BACKLOG.md` remains the sole canonical roadmap; `TheKlangVault/Inbox/Archive/` is purely historical provenance.
  - *One-Way Vault Mirror (ABSOLUTE — NEVER VIOLATE)*: `TheKlangVault` is a **generated, read-only mirror** of the repos. Content flows **repo → vault ONLY**. Under NO circumstances may any agent, script, or skill copy, sync, merge, or write vault content back into any repository (TheKlangSuite, nkai, ToadTracker, TheKlangResearch). Vault-side edits outside `Inbox/` are disposable and are overwritten on the next sync. **Sole exception — `Inbox/`**: human-authored notes are *read* by intake tools (`tools/check_mail.py`, `tools/ramble_ingest.py`) and converted into ledger/backlog entries in the repo; the note files themselves are never copied into repos. Any future need for vault → repo flow requires an explicit user decision and a rule change first.

## Rolling Blackbox Context Compaction & Institutional Memory (Topic #7)
- **The Amnesia Prevention Tripwire**:
  - Deep agentic engineering sessions produce rich architectural context that risks being lost if the chat window truncates or compacts unexpectedly.
  - **Tripwire Triggers**:
    1. *Act / Flight Completion*: Immediately upon completing a major flight or full Act in `PLAN.md`.
    2. *Context Weight Tripwire*: When the active chat session approaches ~150,000 tokens or when the user invokes `/refresh-context`.
    3. *Pre-Clear Flush Gate*: Under NO circumstances should `/clear` be executed without first performing the 4-Step Pre-Compaction Memory Flush.
- **The 4-Step Pre-Compaction Flush Protocol**:
  1. **Dev History Ingestion**: Append structured session achievements to `docs/history/DEV_HISTORY.md` using the canonical session template.
  2. **Soul Harvest Staging**: Stage any unexpected platform gotchas, bug autopsies, or PCDA dialectic resolutions into `.agents/pipeline/staging/souls/<YYYY-MM-DD>_<slug>.md`.
  3. **Context Clues Hardening**: Update the role-specific context clues index (`context_clues_plan.md`, `context_clues_build.md`, or `context_clues_subagent.md`) with the active flight, target repository, uncommitted state, and immediate next command.
  4. **Link Integrity Gate**: Run `python tools/verify_context_clues.py` to confirm 100% link resolution.
- **The 0-Turn Cold Boot Invariant**:
  - Following a context refresh, `/clear`, or fresh conversation boot, an agent reading `context_clues_*.md` MUST resume operations in exactly **0 turns**, seamlessly picking up the next immediate task without asking redundant orientation questions ("What should we work on?") or forcing the user to repeat prior context.


## Strict Planning Guardrails
- **No Spontaneous Implementation**: When planning commands (`/plan`, `/read-plan`, `/execute-task`) are invoked, NEVER automatically start writing C++ code, compiling, or modifying source files.
- **Mandatory Confirmation**: Always stop, summarize the loaded plan, and explicitly ask the user for permission to begin implementation.
- **The "Pro Sanity & Standards Gate" (Flash Drafts, Pro Audits, Flash Builds)**:
  - **Scope**: For major architectural features, new schema layers, multi-module UI overhauls, or milestone kickoffs, New Klang City may recommend a targeted 1-turn Pro High sanity audit before dispatching to the factory floor.
  - **Workflow**:
    1. *Drafting (Tier 2 Flash High)*: Conducts `/grill-me`, codebase research, and drafts the initial `PLAN.md` at near-zero quota cost.
    2. *Sanity Gate (Tier 1 Pro High)*: User temporarily swaps to Pro High to stress-test the draft for DAW automation compatibility, edge cases, industry standards, and daily developer ergonomics (the "insanity check").
    3. *Dispatch & Build (Tier 2 Flash High)*: User swaps back to Flash High so Klang Industries executes the refined plan on sustainable quota.
  - **Routine Exemption**: Skip the Pro Sanity Gate for routine bug fixes, test suite expansions, CMake tweaks, and mechanical chores.
- **The Staged Planning Buffer (`.agents/pipeline/plans/drafts/`)**:
  - While Klang Industries is executing an active task, New Klang City MUST author all new plans, brainstorming, and `/grill-me` blueprints into `.agents/pipeline/plans/drafts/<task_slug>.md`.
  - Under no circumstances should New Klang City overwrite the active `PLAN.md` or set `READY_FOR_EXECUTION` while Klang Industries is in progress (`STATUS: IN_PROGRESS`).
  - **The Factory-Idle Promotion Gate**: Only after Klang Industries concludes its task, sounds the "JOB'S DONE" chime, and archives the previous plan, may New Klang City promote the approved draft to `PLAN.md`, remove it from `.agents/pipeline/plans/drafts/` (acting as a clean inbox queue), and dispatch via `plan_to_build.md`.

## Strict Background Task Etiquette & Watchdog Timer Policy
- **No Polling or Pinging**: When a command goes to the background, NEVER use `manage_task` to poll its status in a loop or spam the chat.
- **No Task Log Peeking**: NEVER inspect running task logs via `cat`, `Get-Content`, `type`, `head`, `tail`, or `view_file` on `.system_generated/tasks/task-*.log` while actively running.
- **Two-Stage Watchdog Timer Protocol**:
  | Task Type | T = 0 (Launch & Yield) | T = Checkpoint 1 | T = Hard Timeout |
  | :--- | :--- | :--- | :--- |
  | **Quick Tasks / Single Targets** (`dsp_tests`, incremental builds, git) | Schedule 5m watchdog (`TimerCondition="<task-id>"`) | T=5m: check status once silently via `manage_task`; schedule 10m timeout | T=15m: terminate task, inspect logs, alert user |
  | **Full Rebuilds / Multi-Target Suites** (full `/build-validate`, `--clean-first`) | Schedule 10m watchdog (`TimerCondition="<task-id>"`) | T=10m: check status once silently; schedule 10m timeout | T=20m: terminate task, inspect logs, alert user |
  | **Remote GitHub CI Workflows** (Linux, macOS, Windows matrix, auval) | Schedule 10m watchdog | T=10m, 20m, 30m: query GitHub API once, output markdown status table | T=50m: auto-cancel remote run via API, fetch failure logs, alert user |

## Strict C++ Formatting, Style & Memory Safety
- **Indentation & Naming**: Exactly 4 spaces. No tabs. `camelCase` for variables and methods. `PascalCase` for classes and structs.
- **Braces**: Allman style (opening brace on new line) for functions and classes; K&R style (opening brace on same line) for control flow (`if`, `for`, `while`). Omit braces for single-line `if`.
- **Pointers & References**: Attach asterisk/ampersand to type (`float* myPtr`, `const juce::String& text`). Modern C++: generous `const`, obvious `auto`.
- **Memory Safety**: No raw ownership (`new`/`delete`). Dynamic object ownership must use `std::unique_ptr` and `std::make_unique`. Raw pointers (`T*`) and references (`T&`) are for non-owning observation only.
- **Thread Separation**: Never call UI methods (`repaint()`, `setValue()`) from `processBlock()` or `processStereo()`. Audio-to-UI communication must use lock-free atomics/FIFOs or `juce::AsyncUpdater`.

## Strict Refactoring & "Boy Scout" Code Cleanliness
- **Zero-Tolerance for "AI Slop"**: Never leave behind messy, undocumented, or spaghetti code. The user relies on this codebase remaining clean, readable, and highly optimized.
- **The Boy Scout Rule**: Always leave a C++ file cleaner than you found it. If you are modifying a function to add a feature, take an extra 30 seconds to clean up its formatting, improve variable names, and ensure the DSP math is documented.
- **Ruthless Deletion (Git is the Archive)**: Ruthlessly delete obsolete source code and files. Never comment out dead code blocks or create old_Component.cpp/legacy/ folders.
## Strict Git Commit & Branch Safety Gate
- **Conventional Commits**: Format strictly as `<type>(<optional scope>): <description>` (`feat:`, `fix:`, `refactor:`, `chore:`, `docs:`).
- **NEVER Work Directly on Master/Main**: Check active branch (`git branch --show-current`). If on `master` or `main`, immediately create and switch to a descriptive branch (`git checkout -b feat/<slug>`). Direct commits to `master`/`main` are allowed ONLY by explicit user order.
- **Silent Git Tags for Architectural Refactors**: When a milestone consists strictly of internal architectural cleanup, developer tooling, or data schema refactoring with zero user-facing sound/UI changes, create and push an annotated Git Tag (`git tag -a vX.Y.Z -m "..."`) and merge to `main`, but do NOT publish a public GitHub Release entry. This preserves clean SemVer and reproducible build history while preventing spurious 'Update Available' prompts in end-user DAWs.

## Strict Debugging Heuristics
- **Initialization Order First**: When encountering garbage data, parse failures, or unexplained crashes during asset loading, ALWAYS verify C++ object lifecycles and static initialization order *before* investigating encoding or unicode issues.
- **JSON Merge Priority (Overwrite Trap)**: Base/Master JSON files MUST be loaded *first*, followed by specific modules/effects. Loading out of order will silently overwrite module data.
- **Trust, But Verify (Pre-Flight Grounding Rule)**: Before adopting any premise stated in a prompt or recalled from memory (framework versions, model names/vendors, file locations, "X already exists"), verify it: **local first** (`CMakeLists.txt`, `GEMINI.md`, version headers, the file itself), **web second** for model/vendor/upstream claims. If the premise is wrong, open with a polite one-line reality check before answering. Never build on an unverified premise.

## Interactive Decision Modals & Wrap-Up
- **Interactive Modal Required for Design Menus**: When presenting design options (at the end of `/grill-me`, `/plan`, or architectural branching forks), NEVER output raw numbered text lists forcing the user to type "3". ALWAYS invoke `ask_question`.
- **CRITICAL EXCEPTIONS (NEVER USE `ask_question`)**:
  1. **Post-Mortem & Triage Lab (`/postmortem`, `/triage`)**: Under NO circumstances use `ask_question` during post-mortem triage. All option cards, trade-off breakdowns, and triage choices MUST live inside the Sidecar HTML Canvas (with a copy-to-chat Decision Composer). The user reviews the sidecar and simply types back in natural chat text.
  2. **Builder Execution Start Gate (`/read-plan`, `/execute-task`)**: Modals freeze the IDE model dropdown. Pause in chat text.
- **Wrap-Up Housekeeping**: Document interview conclusions in Backlog or Plan.

## Strict "4-Lens Pros & Cons" & PCDA Dialectic Evaluation Standard (On-Demand SOTA)
- **On-Demand Progressive Disclosure**: Routine chat responses default to ultra-terse, punchy summaries (1–3 sentences). Full multi-lens tradeoff tables are generated **strictly on-demand** when the user explicitly requests "pros and cons", "PCDA", or triggers an action chip (`[ ⚖️ Request Pros & Cons ]`) from the sidecar.
- **PCDA & Hegelian Dialectic Protocol**: When the user requests a "PCDA" (Pros, Cons, Devil's Advocate) or asks for trade-offs, agents respond using the structured dialectic triad:
  1. **Pros [Thesis]**: Affirmative benefits, user value, and immediate engineering velocity.
  2. **Cons [Thesis Friction]**: Direct implementation costs, surface friction, and known tradeoffs.
  3. **Devil's Advocate [Antithesis]**: Structural failure modes, hidden maintenance debts, worst-case production traps, and token bloat.
  4. **The 80/20 Resolution [Synthesis]**: The battle-tested pragmatic architecture extracting 80% of the value with 20% of the complexity.
- **Acronym Disambiguation**: Use **PCDA** rather than PCA to prevent collision with **Principal Component Analysis** (DSP/ML dimensionality reduction) and **Printed Circuit Assembly** (embedded hardware BOMs).
- **The 4 Lenses Framework**: When triggered for full architectural reviews, agents analyze systematically across:
  1. 🌍 **Real-World / Daily Ergonomics**: Practical day-to-day developer friction, speed, cognitive load, and maintenance realities.
  2. 🎛️ **Audio Software Industry**: How top synth/plugin developers (Vital, Kilohearts, FabFilter, Elektron, Bitwig, Reaper) solve it. *(May be omitted during pure N'kai / web tooling sessions to preserve token allowance).*
  3. 🏛️ **Wider Software Industry / SOTA**: How modern systems, AI agent frameworks, and web architectures (Anthropic, OpenAI, GitHub, Linux) solve it.
  4. 🏆 **Best Practices & Sustainability**: Clean code, zero technical debt, token economics, avoiding premature optimization.
- **On-Demand Deep Dive Research Prompter**: When the user requests external validation or clicks `[ 🔬 Research Pros & Cons Online ]`, the agent leverages pre-authorized web access to sweep developer forums, bug tracks, and upstream documentation, returning concrete industry citations.

## Multi-Repo SemVer Footer Tracking & Deep-Linked Roadmap
- Chat responses during active research/planning sessions must include the SemVer status badge and the N'kai board link on separate lines without wrapping links in backticks (to allow clean clickable rendering and prevent mangled wrapping):
  🏷️ [TKS: v0.4.0-dev](file:///<artifactDir>/roadmap_sidecar.html#tks-v0.4.0) · [TT: v0.1.0](file:///<artifactDir>/roadmap_sidecar.html#tt-v0.1.0) · [NK: v0.1.0](file:///<artifactDir>/roadmap_sidecar.html#nk-v0.1.0) · [TKR: Research Hub](https://github.com/codygratner/TheKlangResearch)
  📬 [Open N'kai Board](sidecar://nkai/panel/)

## The Centralized Research Hub (`TheKlangResearch`) & Hybrid 80/20 Standard
- **The Five Pillars**:
  1. `TheKlangSuite` (`c:\Dev\TheKlangSuite`): Desktop audio plugin suite (VST3/AU).
  2. `ToadTracker` (`c:\Dev\ToadTracker`): Embedded hardware tracker & groovebox engine.
  3. `nkai` (`c:\Dev\nkai`): Asymmetric sidecar framework & interactive triage lab.
  4. `TheKlangResearch` (`c:\Dev\Research` $\leftrightarrow$ `github.com/codygratner/TheKlangResearch`): Centralized RFCs, eternal DSP mathematics, cross-cutting microcontroller teardowns (TBD-16 ESP32-P4), interactive HTML research boards, and subagent soul harvests. Governed as a Continuous Living Archive (zero software SemVer, zero manual CHANGELOG.md; Git commit history is the single source of truth).
  5. `TheKlangVault` (`C:\Dev\TheKlangVault`): Private Git-backed Obsidian studio hub for quick mobile thoughts, triage ledger, and global Pydantic data pipelines.
- **The Hybrid 80/20 Invariant**:
  - Active implementation specs (`docs/specs/*.md`) and APVTS contracts stay co-located in the code repository to preserve atomic Git commits, PR review fidelity, and `git bisect` history.
  - Large 300KB+ HTML research boards, subagent transcript harvests, and cross-project hardware specs live in `TheKlangResearch` to prevent repository bloat and token-burning agent scans.
  - `tools/sync_obsidian_vault.ps1` compiles `TheKlangVault/Research/Master_Research_Index.md` on every sync pass. Zero Dataview plugin dependency; 100% portable plain Markdown.

## Strict Sidecar Architecture & Ready Handshake Gate
- **Dual-Storage Sidecar Architecture**: All interactive HTML sidecar tools (`visual-grill-me`, research boards, interactive canvas labs) MUST write to BOTH:
  1. The Antigravity Artifact directory: `<appDataDir>\brain\<conversation-id>\<tool_name>.html` (for live side-panel webview rendering).
  2. The gitignored repository directory: `c:\Dev\TheKlangSuite\.agents\sidecar/<tool_name>.html` (for local persistence).
- **The "Sidecar Ready Handshake" (MANDATORY ARTIFACT URI)**: Whenever an agent initiates a workflow paired with an interactive sidecar:
  1. **Mandatory Artifact Link**: The agent MUST output the prominent, clickable file URI pointing to the **Artifact Directory** in chat:
     `👉 **[Open <Tool Name> Canvas](file:///<appDataDir>/brain/<conversation-id>/<name>.html)**`
     *(CRITICAL: Antigravity treats links inside the workspace repository `c:\Dev\TheKlangSuite\...` as source code and opens them in the text editor. Links pointing to the Artifact Directory are intercepted by the IDE and rendered in the rich, interactive side-panel Webview. Never output workspace repo paths as the primary chat link).*
  2. The agent MUST pause and prompt the user to open the file in their side panel.
  3. The agent must wait for the user to confirm ("ready", "open", or direct input) BEFORE presenting complex question blocks or options, guaranteeing the visual experience is active.
- **The Asymmetric Sidecar Split (Terse Chat, Rich Sidecar)**: Once the sidecar is confirmed active, chat responses MUST be ultra-terse, snappy, and conversational (1–3 sentences or quick prompts). All verbose prose, comparative tradeoff tables, architectural diagrams, and option mockups belong exclusively inside the sidecar HTML canvas. This eliminates redundant reading and preserves chat context.
- **The Universal Reusable Sidecar Template (`.agents/sidecar/template.html`)**: All sidecar-enabled tools (`visual-grill-me`, `deep-research`, future interactive labs) MUST instantiate from the standardized repository template. Features include the Antigravity Dark Blue theme, sticky header, persistent font size scaler (`A-` / `100%` / `A+` persisted in `localStorage`), refresh button, Web Worker state sync, and decision composer. Template updates automatically propagate across all sidecar workflows.

## Strict Scratch Script Hygiene
- **Instant Cleanup**: When temporary scripts (`update_*.py`, `temp_*.py`) are created for refactors or tests, ALWAYS delete them immediately in the terminal command chain (e.g., `python script.py ; rm script.py`). Never leave temporary scripts in the tree.

## Strict Test-Driven Guardrail & Build Validation
- **Zero Orphaned Features**: No new DSP algorithm, parameter, UI card, page, modal, or editor tool may be merged without test coverage in `test/dsp_tests.cpp` and `test/gui_tests.cpp`.
- **Dynamic Reflection Compliance**: `gui_tests` dynamically sweeps 100% of registered APVTS parameters and JSON assets. Missing UI bindings or definitions will hard-fail the build.
- **Mandatory Planning Test Phase**: All plans (`/plan`) MUST include an explicit Test Suite Update phase.
- **Always Deploy**: After building successfully, run `deploy.ps1` to sync `.exe` and `.vst3` artifacts into `current_build/` and system directories.

## Strict Context Management & Soul Harvest Protocol
- **Proactive Context Refreshing**: When a conversation crosses ~40 turns or approaches high token density, suggest running `/refresh-context` (or clicking "Replace with New" in the sidebar).
- **Threshold-Aware Factory Clean Slate**:
  - Fresh builder sessions (< 12 turns): Instant engage on `proceed`.
  - Mature builder sessions (> 15 turns): Output subtle 1-line notice: `💡 Factory Context Notice: ~N turns accumulated. Reply 'proceed' to build, or 'harvest & proceed' for a clean slate.`
  - Power-User Flag (`harvest & proceed`): Harvests session into `docs/DEV_HISTORY.md`, updates `context_clues_build.md`, and prompts refresh.
- **"Step 0: Pre-Flight Context Clues" & Link-Only Indexing**: `context_clues_plan.md`, `context_clues_build.md`, and `context_clues_subagent.md` maintain lightweight (< 150 token) Quick-Reference Indexes with clickable `file://` links to `docs/GLOSSARY.md`, `docs/DOCS_CATALOG.json`, `docs/architecture/product_lineup.md`, sibling specs, and active `docs/BACKLOG.md` milestones. Fresh agents and spawned subagents waking up from `/clear`, context refresh, or `invoke_subagent` MUST execute a deterministic "Step 0" check on turn 1 to index the active vision and glossary without dumping static document contents into the prompt.

## Strict Chat Role Enforcement (Planner vs. Builder)
- **Role Identification**: Determine role by checking the user's initial prompt or reading `context_clues_plan.md` vs `context_clues_build.md`.
- **New Klang City (Planner / Ivory Tower)**: Strictly Read-Only for C++ source files. May ONLY draft `PLAN.md`, update `docs/BACKLOG.md`, write documentation, and manage agent skills. Live factory radar (`git status --short`, `git diff --stat`) encouraged.
- **Klang Industries (Builder / Factory Floor)**: Strictly Read-Only for overarching architecture. Strictly forbidden from modifying `docs/BACKLOG.md` or drafting core `PLAN.md` features. Refuses architectural planning requests.
- **Strict Release Authority Gate (Ivory Tower Only)**:
  - Klang Industries is strictly forbidden from executing `/cut-release`, modifying release version strings, creating/moving `git tag`s, or pushing tags. Its job ends after tests pass, `deploy.ps1` runs, changes are committed, and the "JOB'S DONE" chime sounds in `build_to_plan.md`.
  - New Klang City is the sole release authority (reviews diffs, verifies desktop testing, updates changelogs, tags Git releases, and pushes to GitHub).
- **Strict Guardrail & Skills Governance Gate (Ivory Tower Only)**:
  - Klang Industries is strictly forbidden from modifying `GEMINI.md`, system rules, or authoring/editing agent Skills (`.agents/skills/`, `.gemini/config/skills/`). It must refuse rule changes and direct the user to New Klang City.
  - **The "Just Do It" Emergency Override**: Klang Industries may ONLY bypass this restriction if the user explicitly includes the exact phrase `"just do it"` in their prompt.
  - New Klang City is the sole authority for system architecture, `GEMINI.md` guardrails, agent workflows, skill definitions, and project backlogs.
  - **Living Field Guide Maintenance**: Whenever agent skills, system guardrails, or workflow pipelines are updated or upgraded, New Klang City must document the rationale, operational lessons learned, and failure modes into `docs/history/vibe_coding_field_guide.md` to preserve institutional memory.

## Mobile-Aware Autopilot Factory Pipeline
- When the user indicates mobile (*"I'm on mobile"*, *"run full task"*), Klang Industries executes all plan phases continuously end-to-end (`Code Edits ➔ audiothread-guard ➔ cmake build ➔ gui_tests ➔ deploy.ps1 ➔ archive PLAN.md ➔ update build_to_plan.md`) without halting for intermediate chat approvals.

## Strict Inter-Chat Communique Protocol & "Job's Done" Handshake
- **Mailbox Protocol (`.agents/pipeline/communique/`)**:
  - `plan_to_build.md`: Authored strictly by New Klang City. Declares dispatch state machine:
    `STATUS: DRAFTING` ➔ `STATUS: READY_FOR_EXECUTION` ➔ `STATUS: IN_PROGRESS` ➔ `STATUS: PLAN_AMENDED ⚠️` ➔ `STATUS: COMPLETED ✅`.
    *(When New Klang City injects mid-flight additions, it sets `STATUS: PLAN_AMENDED ⚠️` with a brief changelog so the factory re-ingests the plan at its next `step-verify` checkpoint).*
  - `build_to_plan.md`: Authored strictly by Klang Industries. Declares factory execution state machine:
    `STATUS: IDLE 💤` ➔ `STATUS: BUILDING 🔨 (Phase <N>: <Name>)` ➔ `STATUS: COMPLETE ✅`.
    *(Klang Industries updates `build_to_plan.md` to `STATUS: BUILDING 🔨` immediately upon `/read-plan` intake and at each `/step-verify` progression, giving the Ivory Tower real-time factory telemetry).*
- **Zero-Polling Discipline**: No terminal status loops waiting for the other chat. Handoffs are event-driven.
- **The "Job's Done" Chime & Dual Mailbox Closure**: When Klang Industries finishes compilation, test passes, and artifact deployment, it MUST update BOTH `build_to_plan.md` (to `Status: COMPLETE ✅`) and `plan_to_build.md` (to `Status: COMPLETED ✅`), archive `PLAN.md` to `.agents/pipeline/plans/completed/`, and conclude its turn with this prominent handoff chime:
  > 🔔 **JOB'S DONE!** `<Task Name>` is fully built, tested, and deployed. Switch to New Klang City to review and advance!
- **Single-Read Ingest**: Returning to New Klang City triggers **exactly ONE read** of `build_to_plan.md` to confirm completion.
- **Mandatory Backlog Reconciliation upon Ingestion**: When New Klang City ingests a `Status: COMPLETE ✅` report from `build_to_plan.md`, its FIRST mandatory action before discussing or drafting new features is to update `docs/BACKLOG.md` (marking the matching task `— ✅ COMPLETED`, updating any `PLAN.md` references to the permanent archive path in `.agents/pipeline/plans/completed/`, and documenting verification metrics).
- **Auto-Archive & Reset on Task Finish**: Klang Industries must immediately archive `PLAN.md` to `.agents/pipeline/plans/completed/<YYYY-MM-DD>_<task_slug>.md` and reset `PLAN.md` to an idle state (`# No Active Plan`).

## Strict Semantic Versioning & Milestone Progression Protocol
- **Major/Minor Feature Milestones (X.Y.0)**: Reserved strictly for massive architectural overhauls, breaking UI paradigm shifts, or entirely new product deployments.
- **Backend & Hardening Patches (X.Y.Z)**: Reserved exclusively for DSP runtime safety, automated testing gauntlets, CI/CD pipeline lockdowns, and bug fixes.
- **The "Pre-Flight Capstone" Rule**: Before bumping the CMakeLists.txt version to a new X.Y.0-dev milestone to begin destructive feature work, the pipeline MUST execute a final .Z hardening patch milestone to mathematically prove the legacy foundation is secure, warning-free, and locked down. Never build new UI on top of unhardened DSP.
## Pre-Release "Clean Slate & Regression Gauntlet" Guardrail
- Mandatory pre-flight checklist before any version bump, git release tagging, or `/cut-release`:
  1. **Hardened Cruft Sweep (4 Tiers)**: Recursive scratch purge (no sub-tree scripts), pipeline/inbox cleanliness (`PLAN.md` idle, inbox zero, active milestone backlog items 100% reconciled with zero stale `PLAN.md` links), diagnostic disk hygiene (pruned `test_screenshots/`), and static code audit (zero debug console leaks, JUCE 9.0.3 timer hygiene).
  2. **Dual-Configuration Parity**: Both `Debug` and `Release` compile cleanly with zero errors.
  3. **Universal 100% Test Pass**: `dsp_tests.exe` (100% audio invariants, SIMD, FastMath) and `gui_tests.exe` (100% parameter reflection, component bindings, modal lifecycles, offscreen paint smoke passes).
  4. **Strict Schema Parity**: 100% of APVTS parameters registered in both plugins have matching `ControlDef` entries in `assets/controls/*.json`.
  5. **JUCE 9.0.3 Timer Hygiene**: 100% of `juce::Timer` subclasses call `stopTimer()` as first line of destructor.
  6. **Pro High Documentation Polish & Wiki Sync (Phase 2.5)**: Review and update `CHANGELOG.md`, `docs/history/DEV_HISTORY.md`, cross-link integrity, and mirror to `TheKlangVault/Docs/` in Tier 1 Pro High before tagging.

## Strict Web Access & Clipboard Guardrails
- **Zero Arbitrary Clipboard Sniffing**: Agents must NEVER inspect or read the host system clipboard (`Get-Clipboard`). Blueprints, tasks, and code must originate exclusively from local workspace files or direct chat input.
- **The Web Access State Machine**: To prevent prompt-fatigue while preserving privacy, agents must adhere to a strict Web Access state machine for `search_web`, `read_url_content`, and `curl`:
  1. **RESTRICTED (Default)**: Web access is strictly prohibited. The agent must pause and ask for explicit permission before any external query.
  2. **QUERY_ONLY**: User replies "ok" / "yes". Access is granted for the immediate action, then instantly reverts to RESTRICTED.
  3. **SKILL_SESSION**: User replies "ok for this session" / "for this skill". Access is granted for the duration of the active skill (e.g., `/deep-research`). It reverts to RESTRICTED upon skill completion or archival.
  4. **GLOBAL_TEMPORARY**: User replies "ok globally". Access is granted indefinitely for the duration of the current context window. Reverts to RESTRICTED upon `/clear`.
- **Mandatory Web Security Footer**: If the state machine is elevated to `SKILL_SESSION` or `GLOBAL_TEMPORARY`, the agent MUST append a highly visible security badge to the bottom of EVERY chat response so the user does not forget the agent is live on the internet.
  > `[ 🌐 Web Access: ACTIVE (<Scope>) | 🧠 Model: <Tier> ]`
- **Local-Only Plan Priority**: All plans, tasks, and communique documents MUST come exclusively from local workspace files (e.g. `PLAN.md`, `.agents/pipeline/communique/plan_to_build.md`) or direct text provided by user.
- **Privacy & Context Boundary**: The host system clipboard may contain private, out-of-band user data from other applications (notes, meeting links, passwords, tokens). Respect context boundaries at all times.

## Strict FOSS Attribution & External Code Hygiene Guardrail ("FOSS Forever")
- **Copyleft Reciprocity First (GPLv3)**: *The Klang Suite* is licensed under the GNU General Public License v3.0 (GPLv3). Under NO circumstances may proprietary, non-commercial (e.g., CC-BY-NC), or GPL-incompatible code (e.g., 4-clause BSD with advertising clause, commercial NDA SDKs) be copied, adapted, or introduced into the repository.
- **Permissive Open-Source & Public Domain Only**: External algorithms, DSP blocks, and utilities may ONLY be ingested from compatible open-source licenses (GPLv3, LGPLv3, MIT, BSD-2/3-Clause, Apache 2.0, ISC, zlib, Boost) or verifiable Public Domain works (CC0, Unlicense, academic algorithm descriptions).
- **Mandatory Inline Provenance Docblocks**: Every adapted DSP function, mathematical formula, or external utility in C++ source files MUST be preceded by a standardized provenance docblock:
  ```cpp
  /**
   * @brief [Algorithm Name / Functionality Description]
   * Adapted from: [Author / Project Name]
   * Source:        [Upstream URL / Paper / Repository]
   * Original License: [MIT / BSD-3-Clause / Apache 2.0 / Public Domain]
   * Modifications: [e.g. Vectorized with FastMath, adapted to 4-control interface]
   */
  ```
- **Universal Public Domain & POSS Attribution**: Even when code or mathematical formulations originate from the Public Domain (e.g., MusicDSP source archives, academic whitepapers, Nigel Redmon / EarLevel biquad formulas, or Paul Kellett filters), explicit author attribution and source URLs MUST be documented. "Public Domain" does not mean anonymous.
- **Universal SPDX License Headers**: 100% of `.h`, `.cpp`, and `.cmake` files must begin with the standard SPDX tag:
  `// SPDX-License-Identifier: GPL-3.0-or-later`
  `// Copyright (C) 2026 Cody Gratner & The Klang Suite Contributors`
- **Central Ledger Synchronization**: All ingested or adapted external code MUST be recorded in `docs/THIRD_PARTY_LICENSES.md` and surfaced in the in-app Settings/About credits viewer (Milestone v0.6.0).
- **Strict Zero-Trademark Policy**: When adapting DSP architectures inspired by iconic hardware or software (e.g. Mutable Instruments, Dave Smith Instruments, Roland, Noise Engineering), NEVER use third-party trademark names in UI parameter names, FX selectors, preset titles, or documentation. Use original agrarian or evocative descriptive terminology (e.g., `THE MIST`, `MODAL RESONATOR`).




