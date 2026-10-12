# Backlog & Roadmap for The Klang Farmer & The Klang Planter

> [!IMPORTANT]
> **NEXT SESSION KICKOFF REMINDER**:  
> When opening the next session, review the prioritized milestones below:
> 1. **v0.3.0 (Architecture)**: GUI Test Harness, The Klang Editor (TKE) & Snapshots, macOS .pkg Pipeline, GitHub Version Checker, Cruft Purge, & Parity Audit. — ✅ COMPLETED
> 2. **v0.3.1 (Editor Quality & Data Schema)**: Standalone Tree UX, Limiter Callout, Two-Line Status Bar, pluginval Runner. — ✅ COMPLETED
> 3. **v0.3.2 (Agent Infrastructure & Logging)**: Guardrails Audit, Unified Filterable Master Tree, Dedicated Text Schema, Developer Logging (TKS_LOG). — ✅ COMPLETED
> 4. **v0.4.0 (Interface & Experience)**: Complete Clean-Slate UX Overhaul, Neo-Slate Vector UI (Kilohearts/Vital/Pigments aesthetic), 4-Controls-Per-Card Architecture, Header Nav & Stereo Scope, Popover Callouts, 7-Theme Engine, & Desktop Sanity Testing. — 🔨 ACTIVE (Pre-Release Sanity & Research)
> 5. **v0.4.5 (The Klang Planter Refresh)**: Rebuilding The Klang Planter on TKF's Neo-Slate Foundation — 4-Controls-Per-Card, Header Nav, Vector Dice Buttons, Popover Callouts, & Universal Theming Parity. — ⏳ PLANNED
> 6. **v0.5.0 (Sound & Chaos + The Klang Hub)**: Unified "Mega" FX Modules (Drive w/ Tanh/Wavefolder/Faturator, Thickener w/ Chorus/Ensemble), FX Picker pre-selections, Alpha Juno oscillators, Mixer Cross-mod (ER-1 Mk2/Spectral), Preset Morphing, Spreadhseet Mod Matrix, Klang Hub, Dual Sample Players.
> 7. **v0.6.0 (Pro Workflow)**: Direct Render-to-Sample (Auto-load to Player/Wavetable), JSON Preset Browser, 2x/4x Oversampling, 4 TBD-16 Macros, Zero-Server GitHub Crash Reporting, One-Time Quick Tour, & Linux Headless CI.
> 8. **v0.9.0 (The Spring Cleaning Audit)**: Pencils down. Comprehensive Tech Debt Amnesty, code refactoring, and AI-Slop purge.
> 9. **v1.0.0 (General Availability)**: Multi-Platform Installers, Comprehensive User Manual, & Launch Demo Reel.
> 10. **v1.1.0 (The Klang Box Hardware Universe)**: The Klang Box (TKB) — dadamachines tbd-16, TKB-Daisy (Stereo), TKB-8 (Teensy Multi-Out), & Zynthian V5.
> 11. **Spin-Offs**: The Klang Mill (TKM 1x6 Pedalboard Rack), The Klang Boilerplate, & The Klang R1 (TKR-1).

> [!TIP]
> **CODE QUALITY STANDARD**: The C++ codebase currently maintains an A+ standard for defensive programming, descriptive `camelCase` variable naming, and explicit algorithmic comments (e.g., documenting DSP math curves directly above the function). All future contributions must rigidly match this level of in-line documentation and readability!

---

---

## 📑 Index
- [v0.3.0 "The Architecture Update"](#-milestone-v030-the-architecture-update) (Completed)
- [v0.3.1 "Editor Quality & Data Schema"](#-milestone-v031-editor-quality--data-schema) (Completed)
- [v0.3.2 "Agent Infrastructure & Guardrails Audit"](#-milestone-v032-agent-infrastructure--guardrails-audit) (Completed)
- [v0.3.3 "The Hardening Gauntlet"](#-milestone-v033-the-hardening-gauntlet) (Completed)
- [v0.4.0 "The Interface & Experience Update"](#-milestone-v040-the-interface--experience-update) (Completed)
- [v0.4.5 "The Klang Planter Refresh"](#-milestone-v045-the-klang-planter-refresh) (Planned)
- [v0.5.0 "The Sound & Chaos Update"](#-milestone-v050-the-sound--chaos-update)
- [v0.6.0 "The Pro Workflow Update"](#-milestone-v060-the-pro-workflow-update)
- [v0.7.0 "The Visual Polish & UI Mastery Update"](#-milestone-v070-the-visual-polish--ui-mastery-update)
- [v1.0.0 "The General Availability Launch"](#-milestone-v100-the-general-availability-launch)
- [v1.1.0 "The Klang Box Hardware Universe (Post-1.0)"](#-milestone-v110-the-klang-box-hardware-universe-post-10)
- [N'kai (NKAI) Asymmetric Sidecar Framework](#-nkai-nkai-asymmetric-sidecar-framework)
  - [N'kai v0.2.0 Milestone (UI Extension & Native Bridge)](#13-nkai-v020-milestone-the-ui-extension--native-bridge-update---completed)
  - [N'kai v0.3.0 Milestone (Mobile Companion & Touch Ergonomics)](#14-nkai-v030-milestone-the-mobile-companion--touch-ergonomics-update---deferred)
  - [N'kai Sidecar Architecture Roadmap (Act III)](#15-nkai-sidecar-architecture-roadmap-the-rejiggered-runway--act-iii)
  - [Flight 2: N'kai to High Reasoning Prompt Compiler (P1 — TOP ACTIVE PRIORITY)](#16-flight-2-nkai-to-high-reasoning-prompt-compiler-p1--top-active-priority)
  - [Flight 3: N'kai Automode & Multi-Drawer Dossier Cards (P1 — TOP PRIORITY)](#17-flight-3-nkai-automode-2-pass-iterative-research-daemon--multi-drawer-dossier-cards-p1--top-priority)
  - [Flight 4: General Research Assistant & Configurable Lens Profiles (P2)](#18-flight-4-general-research-assistant--configurable-lens-profiles-p2)
- [Spin-Off Products & Explorations](#-spin-off-products--explorations)
  - [1. The Klang Mill (Standalone VST)](#1-the-klang-mill-standalone-vst--industrial-1x6-multi-fx-pedalboard-rack)
  - [2. The Klang Boilerplate](#2-the-klang-boilerplate--modern-c20--juce-9-foss-plugin-starter-template)
  - [3. ToadTracker Core Migration](#3-toadtracker-core-migration--architectural-port)
  - [4. The Klang R1 (TKR-1)](#4-the-klang-r1-tkr-1--7-voice-rhythm-synthesizer-electribe-er-1-tribute)
  - [5. The Klang Cultivator (TKC)](#5-the-klang-cultivator-tkc--standalone-midi-cc--cv--mpe-modulation-generator-rack)
  - [6. N'kai Web Audio Haptics](#6-nkai-sidecar-harness-web-audio-haptics--webview-audio-pipeline-backlog)
  - [7. Obsidian Vault Inbox & Addons Triage](#7-obsidian-vault-inbox--addons-triage-next-nkai-session)
  - [8. Post-Milestone Deep Docs Gauntlet](#8-post-milestone-deep-docs--field-guides-separation-gauntlet---queued)
  - [9. Tri-State Model Advisory Architecture](#9-tri-state-model-advisory-architecture-topic-4---completed)
  - [10. Thematic Milestone Governance](#10-thematic-milestone-governance--lightweight-anti-willy-nilly-guardrails-topic-10---queued)
  - [11. Persistent Mailbag Session & Dynamic Priority Queue](#11-persistent-mailbag-session--dynamic-priority-queue---queued)
  - [12. Multi-Tier Rollback Architecture Across Agent Skills](#12-multi-tier-rollback-architecture-across-agent-skills---active)
  - [13. The Operatic Runway & Tooling Flights (Acts I & II)](#13-the-operatic-runway--tooling-flights-acts-i--ii---completed)
  - [14. Tooling Maintenance: Git for Windows Upgrade](#14-tooling-maintenance-git-for-windows-upgrade---queued)
  - [15. Open Source Anti-Slop Hardening & CI Safety Gauntlet](#15-open-source-anti-slop-hardening--ci-safety-gauntlet---completed)
- [The Strategy Graveyard](#-the-strategy-graveyard)

---
*Focus: Tooling, Data-Driven Architecture, and 1:1 Legacy Parity.*

### 1. Automated GUI Test Harness (Guardrail) — ✅ COMPLETED
*Detailed Plan: [`docs/../.agents/pipeline/plans/completed/2026-10-06_Universal_Automated_GUI_Test_Harness.md`](../.agents/pipeline/plans/completed/2026-10-06_Universal_Automated_GUI_Test_Harness.md)*
Comprehensive, single-binary C++ functional GUI testing harness (`gui_tests`) testing The Klang Farmer, The Klang Planter, and The Klang Editor with synthetic mouse event simulation, APVTS parameter sync, page navigation, offscreen smoke paint checks, and dynamic reflection audit.

### 2. Standalone JSON Data & Theme Editor (TheKlangEditor) — Phase 2: Controls, Typography & Snapshots — ✅ COMPLETED
*Detailed Plan: [`docs/../.agents/pipeline/plans/completed/2026-10-05_TheKlangEditor.md`](../.agents/pipeline/plans/completed/2026-10-05_TheKlangEditor.md)*
Dedicated JUCE GUI editor with card preview, controls inspector, JSON snapshot export/import/factory restore, and automated commit tracking.

### 3. Automated C++ Linting & Formatting (`clang-format`) — ✅ COMPLETED
`.clang-format` configured and active, enforcing 4-space indentation and clean C++ formatting.

### 4. Comprehensive Codebase Cruft Purge — ✅ COMPLETED
*Detailed Plan: [`docs/../.agents/pipeline/plans/completed/2026-10-06_Cruft_Purge_Execution_Plan.md`](../.agents/pipeline/plans/completed/2026-10-06_Cruft_Purge_Execution_Plan.md)*
Purged legacy standalone FX blocks, orphaned APVTS parameters, and bypassed routing in commit `4001296`, cleanly migrating all effects to the dynamic 8-slot multi-instance architecture.

### 5. Automated v0.2.0 Parity Audit — ✅ COMPLETED
*Detailed Plan: [`docs/../.agents/pipeline/plans/completed/2026-10-06_v020_parity_audit_plan.md`](../.agents/pipeline/plans/completed/2026-10-06_v020_parity_audit_plan.md)*
*Audit Report:* [`docs/parity_audit/tkf_parity_audit.html`](parity_audit/tkf_parity_audit.html) | [`docs/parity_audit/tkf_parity_audit.pdf`](parity_audit/tkf_parity_audit.pdf)
Comprehensive automated audit cross-referencing all 204 legacy v0.2.0 parameters against current v0.3.0 JSON controls across The Klang Farmer and The Klang Planter. All calibrated legacy defaults, string formatters, and colors verified with 0 discrepancies (160 exact matches, 44 intentional multi-instance FX slot migrations, and 30 safe additions). Slide-deck printable PDF report generated.

### 6. Zero-Cost macOS FOSS Distribution Pipeline (.pkg + Quarantine Stripper + Visual Guide) — ✅ COMPLETED
*Goal: Ensure the v0.3.0 Mac release installs and upgrades with zero friction or Gatekeeper blocks.*
- **Automated `.pkg` Installer Generator**: GitHub Actions runner uses macOS native `pkgbuild` & `productbuild` to generate a standard installer.
- **Automated Post-Install Quarantine Stripper**: Installer runs an automated `postinstall` script (`xattr -rd com.apple.quarantine /Library/Audio/Plug-Ins/...`) that strips the internet quarantine flag so DAWs scan the VST3/AU immediately with zero Gatekeeper warnings!
- **In-Place Seamless Upgrades**: Overwrites older `v0.2.0` bundles cleanly while preserving all user presets and DAW project compatibility.
- **Manual Portable DMG & Helper Script**:
  - Packages a stylized `.dmg` with drag-and-drop symlinks to `/Library/Audio/Plug-Ins/`.
  - Includes a double-clickable `Fix_Mac_Permissions.command` helper script.
  - Includes an illustrated `macOS_Install_Guide.html` showing the 2-step bypass in System Settings -> Privacy & Security.

### 7. Automated GitHub Release Version Checker & Settings Modal — ✅ COMPLETED
*Goal: Provide seamless, non-intrusive notification of new releases directly inside the plugin so v0.3.0 users automatically know when v0.4.0 and beyond drop.*
- **Non-Blocking Background Worker**: Async background thread querying GitHub release API on plugin load (`VersionChecker`).
- **Header Notification Badge**: Subtle, glowing 'Update Available' tag next to version text (`UpdateBadgeButton`). Clicking opens release page.
- **Settings & About Modal**: Gear icon in header (`GearButton`) exposing 'Check for updates on launch' toggle, manual 'Check Now' button, and build metadata (`SettingsModalComponent`).

### 8. GitHub CLI Integration & Repository Tagging — ✅ COMPLETED
*Goal: Improve repository discoverability for audio-plugin developers and the vibe coding community.*
- Installed GitHub CLI (`gh`) via `winget` and authenticated with user credentials.
- Curated repository topics applied: `vst3`, `juce-framework`, `drum-machine`, `fm-synthesis`, `vibe-coding`, `agentic-coding`, `audio-plugin`, `synthesizer`, `dsp`, `c-plus-plus`.

### 9. Post-v0.3.0 Tagged Release, Knowledge Distillation & Chat Archival
*Detailed Plan: [`docs/../.agents/pipeline/plans/completed/2026-10-06_post_v030_release_and_archive_plan.md`](../.agents/pipeline/plans/completed/2026-10-06_post_v030_release_and_archive_plan.md)*  
*Goal: Consolidate institutional memory across all 14+ chat sessions into a permanent Git-versioned Markdown knowledge base, tag and publish the v0.3.0 release, and safely close all active chats with zero lost knowledge.*
- **Automated Transcript Harvester**:
  - Python harvester script scans all `transcript.jsonl` files in `~/.gemini/antigravity/brain/*/` across all project chat sessions.
  - Distills prompts, architectural decisions, solved bugs, and created artifacts into a structured, chronological `docs/DEV_HISTORY.md`.
- **Executive Institutional Memory Index**:
  - Curated cheat-sheet summarizing core DSP invariants, JUCE 9.0.3 hygiene, build heuristics, and data-driven design patterns at the top of `docs/DEV_HISTORY.md`.
- **Git Tagging & GitHub Release**:
  - Create and push Git tag `v0.3.0`.
  - Author comprehensive release notes and publish GitHub Release with bundled artifacts (`.pkg`, `.vst3`, `.exe`).
- **Primary Branch Migration (`master` &rarr; `main`)**:
  - Fast-forward remote `main` with all 100+ commits from `master`.
  - Switch default repository branch to `main` on GitHub (via `gh repo edit --default-branch main`).
  - Update `CMakeLists.txt` comments and cleanly retire/delete obsolete remote `master` branch.
- **Repository Umbrella Rename & Description Polish**:
  - Rename GitHub repository from `TheKlangFarmer` &rarr; `TheKlangSuite` (GitHub automatically redirects all web traffic, Git clones, and release assets).
  - Update repository About description: `"FM drum synthesizers, multi-effects, and sound design tools (VST3/AU). Pair-programmed and vibe-coded with Google Gemini."`
  - Update local remote origin: `git remote set-url origin https://github.com/codygratner/TheKlangSuite.git`.
- **Post-Release Housekeeping & Chat Purge**:
  - Archive all completed v0.3.0 plan files into `docs/../.agents/pipeline/plans/completed/`.
  - Update `CHANGELOG.md` with final v0.3.0 diff.
  - Safe signal to close/kill all accumulated chat sessions in the Antigravity UI for a clean, lightning-fast v0.4.0 kickoff.

---


## ?? Milestone: v0.3.1 "Editor Quality & Data Schema"
*Focus: Expanding the Editor's GUI tests, Tree View UX, and upgrading the JSON data schema for rigorous parameter definitions.*

### 1. Editor GUI Test Suite Expansion & Tree View UX — ✅ COMPLETED
*Detailed Plan: [`docs/../.agents/pipeline/plans/completed/2026-10-06_editor_tree_ux.md`](../.agents/pipeline/plans/completed/2026-10-06_editor_tree_ux.md)*
*Goal: Expand `gui_tests` to fully validate `The Klang Editor` through headless component testing, and improve the Tree View's user experience.*
- **Headless Validation**: Added `juce::UnitTest` module simulating 100% parameter tree node selection with property manager synchronization.
- **Global Tree Controls**: Added mini-toolbar with `Expand All` and `Collapse All` icon buttons.
- **Contextual Tree Controls**: Added right-click context menu to tree items with `Collapse Others`, `Expand All`, and `Collapse All`.

### 2. Extract Hardcoded C++ Parameter Metadata into JSON (Parity Preservation) — ✅ COMPLETED
*Detailed Plan: [`docs/../.agents/pipeline/plans/completed/2026-10-06_parameter_metadata_extraction.md`](../.agents/pipeline/plans/completed/2026-10-06_parameter_metadata_extraction.md)*
*Goal: Pull all hardcoded parameter descriptions and bipolar flags out of `FarmerEditor.cpp` and populate them into `assets/controls/*.json` to match `PlanterEditor`'s modern `ControlDef` binding pattern, while maintaining 100% exact parity.*
- **Extract Legacy Boilerplate**: Migrated ~120 lines from `getFarmerParamDescription()` into JSON asset schemas.
- **Modernize `bindSlider`**: Refactored `FarmerEditor::bindSlider` to read `def->description`, `def->isBipolar`, `def->doubleClickValue`, and `def->snapPoints` from `ControlDef`.
- **Zero Parity Breakage**: 84/84 tests passing with zero regressions.

### 3. Intensive GUI Test Suite for Plugins & Standalone (Farmer & Planter) — ✅ COMPLETED
*Detailed Plan: [`docs/../.agents/pipeline/plans/completed/2026-10-06_intensive_gui_tests.md`](../.agents/pipeline/plans/completed/2026-10-06_intensive_gui_tests.md)*
*Goal: Model intensive GUI testing after The Klang Editor's test harness, expanding `gui_tests` to comprehensively validate component trees, page navigation, modal popups, and offscreen rendering for both The Klang Farmer and The Klang Planter.*
- **Headless Component & Parameter Sweep**: Programmatically verified 100% of cards, sliders, and selectors bind correctly to APVTS parameters and display non-empty tooltips (fixed 3 missing tooltips on Planter limiter).
- **Page Navigation & Paint Smoke Test**: Cycled through all page views across 800x600, 1000x750, and 4K dimensions with offscreen paint passes (`paintEntireComponent()`). Zero crashes, zero division-by-zero.
- **Modal Lifecycle Test**: Simulated opening and closing all modals (Settings, About, Quickstart) with zero timer leaks.
- **Verification Metric**: 117 / 117 `gui_tests` passed successfully with 100% assertion pass rate.

### 4. Planter Header Interactions: VU Meter Panic & Limiter CalloutBox — ✅ COMPLETED
*Detailed Plan: [`docs/../.agents/pipeline/plans/completed/2026-10-06_planter_header_and_status_bar.md`](../.agents/pipeline/plans/completed/2026-10-06_planter_header_and_status_bar.md)*
*Goal: Transform The Klang Planter's header visualizer into an interactive control center with dedicated mouse targets for master panic and instant limiter adjustment.*
- **Limiter Right-Click CalloutBox**:
  - Right-clicking the center `LIMIT` badge launches a floating mini-card `juce::CalloutBox`.
  - Houses an Enable toggle (`planter_limiter_enable`) and 3 mini rotary knobs for Gain (`planter_limiter_gain`), Ceiling/Threshold (`planter_limiter_thresh`), and Release (`planter_limiter_release`).
  - Styled to match Card 6's Doepfer silver & red chassis theme (`0xffe53935`).
- **Peak VU Meter Panic**:
  - Clicking the stereo peak meters flushes all active voice and noise envelope timings, resets the S&H DJ filter, and clears master peak levels.
  - Features a crisp 150ms visual flash on the meter bars upon panic trigger.
- **Parity Safety**:
  - Non-destructive: Binds directly to existing APVTS parameters without altering presets, audio DSP math, or Card 6.

### 5. Interactive Two-Line Status Bar (Values, Mouse Shortcuts & Tooltip Feed) — ✅ COMPLETED
*Detailed Plan: [`docs/../.agents/pipeline/plans/completed/2026-10-06_planter_header_and_status_bar.md`](../.agents/pipeline/plans/completed/2026-10-06_planter_header_and_status_bar.md)*
*Goal: Implement a Kilohearts/Ableton style 36px bottom status bar across The Klang Farmer and The Klang Planter, providing permanent value readouts, mouse shortcut badges, and a full-width tooltip feed.*
- **Line 1 (Top Bar - Permanent)**:
  - Left: Control Name and formatted Parameter Value in bold (e.g., `Carrier 1: Pitch  +12.0 st [440 Hz]`).
  - Right: Contextual Mouse Shortcuts in subtle pill badges (e.g., `Right-Click: Snap Points | Double-Click: Reset (0.5)`). Always visible even if tooltips are toggled off.
- **Line 2 (Bottom Bar - Tooltip Feed)**:
  - Full-width parameter description and functional explanation.
  - Toggled dynamically by the header `TOOLTIPS` button (when off, Line 2 is quiet or shows engine status).
- **Verification Metric**: 164 / 164 `gui_tests` passed successfully with 100% assertion pass rate across all limiter controls and status bar hover callbacks.

### 6. Restore Two-Line Status Bar Visibility & Universal Hover Feed — ✅ COMPLETED
*Detailed Plan: [`docs/../.agents/pipeline/plans/completed/2026-10-06_two_line_status_bar_visibility_and_universal_hover.md`](../.agents/pipeline/plans/completed/2026-10-06_two_line_status_bar_visibility_and_universal_hover.md)*
*Goal: Diagnose why the interactive two-line bottom status bar was low-contrast or appearing missing in The Klang Planter / The Klang Farmer standalone and plugin windows, and restore it to full visibility and responsiveness.*
- **Visual Elevation**: Elevated chassis background (`0xff121622`) with a crisp 1.5px top demarcation border (`0xff2a3449`), green status LED (`● READY`), subtle version badge (`The Klang Suite v0.3.1`), and high-contrast Line 2 text luminance.
- **Universal Header Hover Feeds**: Connected hover callbacks on `initButton`, `triggerButton`, `tooltipsButton`, `settingsButton`, `guideButton`, and `headerViz` so the entire interface feeds live parameter details into the status bar.
- **Z-Order Assurance**: Enforced `statusBar.toFront(false)` in `resized()` across all editors.
### 7. Automated VST3 Parameter Validation Suite & Headless `pluginval` Runner — ✅ COMPLETED
*Detailed Plan: [`docs/../.agents/pipeline/plans/completed/2026-10-06_vst3_parameter_validation_and_pluginval.md`](../.agents/pipeline/plans/completed/2026-10-06_vst3_parameter_validation_and_pluginval.md)*
*Goal: Systematically validate 100% of registered VST3 parameters across The Klang Farmer and The Klang Planter through an in-engine 6-pillar reflection suite, backed by a portable headless pluginval runner.*
- **In-Engine 6-Pillar Parameter Suite (`test/PluginIntensiveTestSuite.h`)**:
  1. **Dynamic Reflection & Identity**: Recursively sweeps 100% of `processor.getParameters()` for both plugins without hardcoded lists, verifying non-empty IDs and names.
  2. **Normalization Roundtrip**: Asserts `convertTo0to1(convertFrom0to1(x)) == x` across `[0.0, 0.1, 0.25, 0.5, 0.75, 0.9, 1.0]` with step-aware tolerances.
  3. **Boundary Clamping & Safety**: Tests out-of-bounds input (-1.0, 2.0) to mathematically verify parameters clamp safely without underflow, overflow, or NaN leaks.
  4. **JSON Schema Parity**: Verifies all registered parameter IDs exist in `assets/controls/*.json` with matching defaults, min/max ranges, and skew factors.
  5. **State Serialization Roundtrip**: Saves APVTS state to XML/MemoryBlock &rarr; randomizes all parameters &rarr; restores state &rarr; asserts 100% restoration parity.
  6. **DSP Audio Smoke Pass**: Sweeps parameters from 0.0 to 1.0 while pumping audio through `processBlock()` to prove zero NaNs, Infs, or divisions by zero under rapid host automation.
- **Headless `pluginval` Fallback Runner (`tools/run_pluginval.ps1`)**:
  - Authored portable runner script discovering `pluginval.exe` in `tools\` or system PATH at configurable strictness (default level 5).
- **Verification Metric**: 207 / 207 `gui_tests` passed successfully with 100% assertion pass rate.

### 8. Planter Header Visualizer Optimization & Latency Remediation — ✅ COMPLETED
*Detailed Plan: [`docs/../.agents/pipeline/plans/completed/2026-10-06_optimize_planter_gui_rendering.md`](../.agents/pipeline/plans/completed/2026-10-06_optimize_planter_gui_rendering.md)*  
*Goal: Eliminate UI thread input latency and sluggishness in The Klang Planter by optimizing visualizer repaint cascades and message thread overhead.*
- **Opaque Visualizer (`setOpaque(true)`)**: Prevents JUCE from invalidating and repainting the entire parent window background during real-time oscilloscope animations.
- **30 Hz Timer Frequency Alignment**: Aligned `PlanterEditor` timer rate with `FarmerEditor` (30 Hz down from 60 Hz), cutting message thread repaint events in half.
- **Scope Spline & Point Optimization**: Downsampled oscilloscope resolution from 128 points to 64 points with smooth rounded stroke joints, dramatically reducing CPU Bézier calculation time during software rendering.
- **String & Glyph Layout Caching**: Cached static title, subtitle, and version strings and computed widths at initialization, eliminating per-frame JSON dictionary lookups and glyph arrangement loops in `paint()`.
- **Idle Silence Bypass**: Added signal gate checking for zero amplitude before queuing repaints when the synthesizer is silent.
- **Verification Metric**: 215 / 215 `gui_tests` passed successfully with 100% assertion pass rate.

### 9. Schema Separation of Concerns & Cross-Reference "Where Used" Inspector — ✅ COMPLETED
*Archived Plan: [`.agents/pipeline/plans/completed/2026-10-07_schema_separation_and_where_used_inspector.md`](../.agents/pipeline/plans/completed/2026-10-07_schema_separation_and_where_used_inspector.md)*  
*Goal: Enforce strict separation of concerns across JSON data layers and eliminate parameter discoverability gaps in The Klang Editor by introducing a pre-indexed Cross-Reference ("Where Used") navigation panel.*
- **Strict Schema Separation**:
  - `assets/controls/*.json`: Strictly DSP/APVTS parameter definitions. Zero visual styling, zero colors, zero pixel dimensions.
  - `assets/layouts/*.json`: Structural UI hierarchy (Pages -> Cards -> parameter ID bindings).
  - `assets/themes/*.json`: Created `assets/themes/` containing `theme.json` (color palettes, module accents) and `callouts.json` (floating overlay geometry, colors, and bound parameter arrays).
- **Planter Master Limiter Discovery**: Added `[Callout] Master Limiter` under The Klang Planter in `assets/layouts/tkp_layout.json` so all 9 modules (8 surface cards + 1 callout) and 100% of APVTS parameters are discoverable in one place.
- **Cross-Reference ("Where Used") Panel**:
  - Added a lower resizable list panel beneath the Tree View in The Klang Editor.
  - Pre-computes a two-way index map at startup (`< 2ms`) with zero runtime filesystem crawling.
  - Selecting any parameter, card, or callout displays its bidirectional associations.
  - Double-clicking any reference item automatically focuses, expands, and selects it in the tree and property form.
- **Automated Schema Audit**: Updated `ParameterSchemaAuditTest` to hard-fail if any visual styling or color properties leak into `assets/controls/`.
- **Verification Metric**: 223 / 223 `gui_tests` passed successfully with 100% assertion pass rate. All visual properties cleanly isolated into `assets/themes/`, and `ParameterSchemaAuditTest` hard-fails if visual keys appear in `assets/controls/`.

---

## 🚀 Milestone: v0.3.2 "Agent Infrastructure & Guardrails Audit"
*Focus: Post-release optimization of GEMINI.md guardrails, custom skills, inter-chat handoffs, and agent telemetry.*

### 1. Comprehensive Guardrails & Skills Optimization Audit — ✅ COMPLETED
*Goal: Systematically audit GEMINI.md, system rules, custom skills, and inter-chat communiques to eliminate cruft, reduce context bloat, streamline execution pipelines, and expand missing capabilities.*
- **Guardrails Audit (`GEMINI.md`)**:
  - Removed all references to retired `/pasteplan`, `/paste-plan`, and `/strict-plan`.
  - Fixed typos (`\x08uild-validate` -> `build-validate`).
  - Aligned 2-Strike Factory Floor Escalation Protocol across `GEMINI.md` and `build-validate/SKILL.md`.
  - Codified the Strict Clipboard & External Link Ingestion Guardrail to prevent accidental OS clipboard sniffing or URL scraping.
  - Codified the Strict Guardrail & Skills Governance Gate, restricting `GEMINI.md`, system rules, and skills editing exclusively to New Klang City (Ivory Tower) unless explicitly bypassed by the user with `"just do it"`.
- **Skills Modernization & 100% Directory Parity**:
  - Retired and deleted obsolete skills (`strict-plan`, `paste-plan`) across both workspace (`.agents/skills/`) and global (`C:\Users\codyg\.gemini\config\skills\`) directories.
  - Modernized `read-plan` as the official Communiqué Dispatch Ingestor (`docs/communique/plan_to_build.md` -> `PLAN.md`).
  - Modernized `execute-task` and `task-finish` to purge all Linear.app references and dead backlog paths, sourcing strictly from `docs/BACKLOG.md`.
  - Synchronized workspace (`.agents/skills/`) and global (`~/.gemini/config/skills/`) in 100% exact parity across all 12 active skills.
- **Inter-Chat Communique Tuning (`docs/communique/`)**:
  - Added `STATUS: COMPLETED` as a formal terminal state in `plan_to_build.md` to prevent stale dispatch pickups.
  - Mandated dual-mailbox closure in `task-finish` (updating both `build_to_plan.md` and `plan_to_build.md`).

### 2. Unified Filterable Master Tree & Dedicated Text/Localization Schema — ✅ COMPLETED
*Detailed Plan: [`docs/../.agents/pipeline/plans/completed/2026-10-07_unified_tree_text_schema.md`](../.agents/pipeline/plans/completed/2026-10-07_unified_tree_text_schema.md)*  
*Goal: Redesign The Klang Editor's navigation tree into a unified master tree with multi-state layer filters, extract text/tooltips into a dedicated data schema, and standardize DSP block file naming.*
- **Unified Master Tree with Layer Filters**:
  - Replaced the dual tabs (`CONTROLS` and `LAYOUTS`) with a single unified `masterTree`.
  - Added 3 toggle filter buttons above the tree: `[Controls]`, `[Layout]`, and `[Theme]`, with Smart Minimum enforcement.
  - Implemented simultaneous property editing: selecting a parameter node in `[Controls]` allows editing DSP limits and text descriptions side-by-side with smart routing to respective JSON files on save.
- **Dedicated Text & Tooltip Schema (`assets/text/strings.json`)**:
  - Extracted 100% of parameter descriptions and choice tooltips out of `assets/controls/*.json` into `assets/text/strings.json`.
  - Organized under clean modular namespaces (`"shared"`, `"farmer"`, `"planter"`).
  - Maintained parameter `"name"` in `assets/controls/` as the immutable DAW/Host automation contract.
  - Implemented seamless startup text merge in `ParameterManager` for zero C++ call-site breakage.
- **DSP Block File Naming Standardization (`assets/controls/*.json`)**:
  - Renamed legacy plural files: `modulator.json`, `filter.json`, `envelope.json`. Ruthlessly purged plural files.
- **Verification Metric**: 275 / 275 `gui_tests` passed successfully with 100% assertion pass rate across all 11 test suites; 100% `dsp_tests` passed. Binaries deployed to `current_build/` and system VST3 directories.

### 3. Developer Logging Subsystem (`TKS_LOG`) & Diagnostics Engine — ✅ COMPLETED
*Detailed Plan: [`docs/../.agents/pipeline/plans/completed/2026-10-07_dev_logger_subsystem.md`](../.agents/pipeline/plans/completed/2026-10-07_dev_logger_subsystem.md)*  
*Goal: Provide structured, leveled developer logging for UI lifecycles, asset loading, and DAW diagnostics in debug builds, strictly guarded against real-time audio thread abuse and stripped completely in release builds.*
- **Dual-Mode Output**:
  - Debug builds pipe timestamped entries to system debugger (`OutputDebugString` / `DBG`) and write to a rotating `%LOCALAPPDATA%/TheKlangSuite/dev.log`.
  - File retention is capped at 5 MB with a maximum of 2 rolled backup files.
- **Leveled Diagnostics**:
  - `TKS_LOG_INFO`: Normal lifecycle events (JSON asset loading, window resize, preset init).
  - `TKS_LOG_WARN`: Non-fatal anomalies (missing optional property, fallback styling used).
  - `TKS_LOG_ERROR`: Critical errors (JSON syntax parse failure, missing APVTS binding).
- **Strict Audio Thread Safety & Static Guardrail**:
  - `TKS_LOG` asserts in Debug builds if invoked on audio threads, with safe early return before memory allocation.
  - `audiothread-guard` static analysis rules flag any `TKS_LOG*` invocations inside audio loops (`processBlock`, `renderVoice`) at build time.
- **Zero Release Overhead**:
  - Compiled out completely to empty no-ops (`do {} while (false)`) when `JUCE_DEBUG` is not defined. Zero binary strings, zero allocations, zero CPU overhead.
- **Verification Metric**: 294 / 294 `gui_tests` passed successfully with 100% assertion pass rate across all 12 test suites in both Debug and Release configurations; 100% `dsp_tests` passed. Binaries deployed to `current_build/` and system VST3 directories.

---

## 🛡️ Milestone: v0.3.3 "The Hardening Gauntlet"
*Focus: Runtime audio safety, concurrency memory safety, and schema fuzzing.*

### 1. Hardening Gauntlet Execution — ✅ COMPLETED
*Archived Plan: [`.agents/pipeline/plans/completed/2026-10-07_hardening_gauntlet.md`](../.agents/pipeline/plans/completed/2026-10-07_hardening_gauntlet.md)*
- CI Toolchain Lockdown & Sanitizer Integration (-Werror)
- The "Monitor Saver" Protocol (DSP NaN/Inf Failsafe)
- Asynchronous DAW Automation Defense
- Data-Driven "Poison Pill" Fuzzing

---

## 🚀 Milestone: v0.4.0 "The Interface & Experience Update"
*Focus: Complete Clean-Slate UX Overhaul, Modern Neo-Slate Vector UI (Kilohearts/Vital/Pigments aesthetic), 4-Controls-Per-Card Architecture, Header Nav & Stereo Scope, & Breaking Prototype Parity.*

### 1. Obsidian Knowledge Base & Asymmetric Sync Bridge — ✅ COMPLETED
*Detailed Guide: [`docs/OBSIDIAN_INTEGRATION.md`](architecture/obsidian_sync_bridge.md)*  
*Archived Plan: [`docs/../.agents/pipeline/plans/completed/2026-10-07_obsidian_vault_asymmetric_sync_bridge.md`](../.agents/pipeline/plans/completed/2026-10-07_obsidian_vault_asymmetric_sync_bridge.md)*  
*Goal: Create a decoupled, conflict-free Obsidian knowledge base (`C:\Dev\TheKlangVault`) backed by an automated PowerShell sync bridge (`tools/sync_obsidian_vault.ps1`), enabling mobile note capture via Obsidian Sync without Git merge collisions, offline docs reading, and live build/error telemetry.*
- **Decoupled Vault & Partitioned Ownership**:
  - `Inbox/` (Vault $\to$ Repo): Frictionless mobile idea capture automatically mirrored to `TheKlangSuite/docs/inbox/`.
  - `Docs/` (Repo $\to$ Vault): Official repository documentation mirrored to the vault for offline reading on mobile/tablet.
  - `Telemetry/` (Script $\to$ Vault): Live mobile dashboards (`Dashboard.md`, `Active_Errors.md`) tracking Git branch, recent commits, and `dev.log` runtime errors.
  - `Canvas/`: Visual workspace for signal flow graphs, FM modulation routing, and card mockups.
- **Asymmetric Sync Engine (`tools/sync_obsidian_vault.ps1`)**:
  - Fast, idempotent sync script supporting on-demand single execution and continuous background loop (`-Watch`).
  - Generates live heartbeat note (`_sync_heartbeat.md`) in the vault.
- **Verification Metric**: End-to-end smoke test passed (61 docs mirrored in ~600ms, test note ingested from Vault Inbox to repo docs inbox in ~450ms, telemetry parsed clean).

### 2. SQA Automation Hardening: Timeout Guardrails, Failure Snapshots & Metric Profiling — ✅ COMPLETED
*Origin: SQA Advisory Consultation (Tom) — Local Vault Inbox*  
*Context Briefing: [`docs/briefings/sqa_meeting_briefing.md`](specs/sqa_automation_hardening.md)*  
*Archived Plan: [`.agents/pipeline/plans/completed/2026-10-07_sqa_automation_hardening.md`](../.agents/pipeline/plans/completed/2026-10-07_sqa_automation_hardening.md)*  
*Goal: Harden the automated testing infrastructure across `gui_tests` and `dsp_tests` based on senior SQA recommendations: enforce global (5m) and local (30s) timeout guardrails, capture automated offscreen UI failure screenshots to `test_screenshots/`, pull failure summaries to the top of test reports, and log granular step duration metrics.*
- **Dual Timeout Architecture**:
  - Global Timeout Threshold: 5-minute watchdog limit across entire test suites (`gui_tests`, `dsp_tests`) backed by `std::jthread`, preventing hung runner processes from burning CPU or CI budgets.
  - Local Timeout Threshold: 30-second individual test step timeouts. Fail fast, log failure state, and cleanly advance to next independent test without cascade aborts.
- **The "Wait-Fail" Component Locator Pattern**:
  - In asynchronous UI environments (modal popups, page transitions, callout animations), replace instant brittle assertions with a timeout-bounded try/wait locator (`waitForComponent<T>(parent, id, timeoutMs)`).
  - Pumps the JUCE dispatch loop in 20ms slices during async transitions while keeping the Watchdog heartbeat alive.
- **Automated & Deduplicated Failure Snapshots (`test_screenshots/`)**:
  - Offscreen Component Rendering: When an assertion fails or a wait-timeout triggers, captures the active window/card hierarchy via `juce::Component::createComponentSnapshot(getLocalBounds())` and writes to a timestamped PNG (`test_screenshots/<TestTag>_<Timestamp>.png`).
  - Snapshot Deduplication & Rate Limiting: Inspects recent captures to prevent looping assertions from flooding disk space with redundant PNGs within the same 10-second window.
  - Instant Diagnostic Linkage: Outputs clickable `file:///` URIs directly to the console for instant inspection.
- **Failure-First Reporting & Profiling Metrics**:
  - High-visibility `🚨 CRITICAL FAILURE SUMMARY` printed at the end of runs listing failure messages and clickable screenshot paths.
  - `⏱️ EXECUTION PROFILING LEADERBOARD` sorts test suites by duration and reports total execution time.
- **Seeded & Replayable Chaos Monkey (`--chaos`)**:
  - Implemented `ChaosMonkeySuite` delivering randomized click, drag, and resize bursts across Farmer and Planter editors with deterministic seed logging (`gui_tests --chaos --seed=<SEED>`).
- **Verification Metric**: 300 / 300 `gui_tests` passed (0 failures, exit code 0) in both Debug and Release configurations. 100% `dsp_tests` passed. Chaos suite survived 1,230,640 randomized events in 3000ms with zero crashes. Deploy synced cleanly via `deploy.ps1`.

### 3. Dynamic Modulation Matrix Engine & Hydra 1-to-Many Architecture — ✅ COMPLETED
*Archived Plan: [`.agents/pipeline/plans/completed/2026-10-08_v040_modulation_matrix_and_regression.md`](../.agents/pipeline/plans/completed/2026-10-08_v040_modulation_matrix_and_regression.md)*
*Goal: Build a lock-free, zero-allocation modular modulation engine in The Klang Farmer inspired by Vital, Phase Plant, and Renoise Hydra.*
- **16 Fixed Pre-Allocated Sources**: 4 LFOs (sync, free, poly/voice/mono retrigger), 4 Envelopes (2 Voice + 2 Aux), 2 Random (Stepped S&H + Smooth Noise), Velocity, Key Tracking, Global Slop, and 4 Hydra Macros.
- **Dynamic Routing Matrix**: Up to 64 active connections routing any source to ANY parameter, including other modulators' rates/decays.
- **Secondary 'Via' Modulation**: Matrix routes support auxiliary depth scaling (e.g. Velocity scales LFO 1 depth to Filter Cutoff).
- **Hydra Macro Hub**: Macros act as standard sources in the Matrix, plus clicking a Macro knob opens a dedicated 'Hydra Fan-Out' popover with individual destination min/max bounds and inverted curves.
- **Verification Metric**: 100% pass across all modular drum DSP verification tests (`dsp_tests.exe`), zero audio-thread allocations, and 352/352 GUI test assertions passed.

### 4. Single-Tab Drag-and-Drop FX Rack & Pre-Amp Console Strip — ✅ COMPLETED
*Archived Plan: [`.agents/pipeline/plans/completed/2026-10-08_v040_modulation_matrix_and_regression.md`](../.agents/pipeline/plans/completed/2026-10-08_v040_modulation_matrix_and_regression.md)*
*Goal: Consolidate Pre-Amp and Post-Amp FX into a single unified EFFECTS tab with tactile drag-and-drop card reordering.*
- **Two Horizontal Signal Lanes**:
  - Top Lane (Pre-Amp): Slots 1–4 -> Immutable Anchor 5: [PRE-AMP DRIVE & COLOR].
  - Bottom Lane (Post-Amp): Slots 5–8 -> Immutable Anchor 5: [MASTER LIMITER / OUT].
- **Tactile Drag-and-Drop Swap**: Free card dragging with clean parameter/algorithm swapping within and across lanes.
- **Console Pre-Amp Anchor**: Immutable 5th card exposing Input Gain, Drive Curve, Tone, and Output Level.
- **Verification Metric**: Seamless tactile reordering verified under automated GUI tests with ghost/target drag feedback.

### 5. The Neo-Slate Vector Design System & Bundled JetBrains Mono — ✅ COMPLETED
*Archived Plan: [`.agents/pipeline/plans/completed/2026-10-08_v040_modulation_matrix_and_regression.md`](../.agents/pipeline/plans/completed/2026-10-08_v040_modulation_matrix_and_regression.md)*
*Goal: Overhaul the overall plugin window chassis with a sleek, modern, non-skeuomorphic vector design inspired by Kilohearts Phase Plant, Vital, and Arturia Pigments.*
- **Header-Integrated Navigation & Visualizer**:
  - Evict the legacy navigation card and visualizer card from the module rack grid.
  - Implement sleek horizontal Page Navigation tabs directly in the top Chassis Header (`[VOICE 1]`, `[VOICE 2]`, `[TRANSIENTS]`, `[EFFECTS]`, `[AMPLIFIER]`, `[MOD]`).
  - Integrate a unified real-time stereo oscilloscope, peak VU meters, and double-click Panic flush into the header.
- **Neo-Slate Visual Aesthetic**:
  - Dark matte slate surfaces (`0xff0d1117`), dark titanium card bodies (`0xff161b22`), crisp 1px borders (`0xff283141`), JetBrains Mono typography, and vibrant neon accent highlights.
  - Zero faux-vintage screws, zero fake drop shadows, zero 3D skeuomorphism. Clean, futuristic, responsive, and distraction-free.
- **Verification Metric**: 100% theme parity verified across Cykranosh, Cyberpunk Neon, Dracula, and Monokai Pro.

### 6. Interactive Parameter & Curve Audit Tool in The Klang Editor — ✅ COMPLETED
*Detailed Plan: [`docs/v040_ux_overhaul_plan.md`](specs/v040_neo_slate_ux.md)*  
*Goal: Integrate an interactive curve calibration workspace in The Klang Editor to systematically tune the tactile response, snap points, logarithmic slider slopes, and ergonomic double-click defaults of the new curated 4-control parameter set.*
- Live interactive slider evaluation, tactile response tuning, and real-time visualization of parameter skew factor curves.
- Test and calibrate discrete musical snap points live within the editor before persisting to `assets/controls/*.json`.

### 7. Switchable Studio Theme Engine (`Cykranosh`, `Nord`, `Dracula`, `Cyberpunk`) — ✅ COMPLETED
*Detailed Plan: [`docs/v040_ux_overhaul_plan.md`](specs/v040_neo_slate_ux.md)*  
*Goal: Provide distinctive, switchable visual flavors for different studio environments, featuring the creator's signature Cykranosh theme as the flagship look.*
- **Curated Multi-Palette Schema (`assets/themes/theme.json`)**:
  - `cykranosh` (Default / Creator's Signature): Deep slate navy (`#161B22` / `#1A202C`), muted deep blue cards, eerie ghostly teal (`#4EBEB1`), arctic ice blue, and starlight silver indicators (engineered for zero eye fatigue during marathon sessions).
  - `nord`: Arctic frost blue-grey (`#2E3440`) with icy cyan and pastel aurora highlights.
  - `dracula`: Iconic vampire purple (`#282A36`) with electric cyan, hot pink, and lime green accents.
  - `cyberpunk`: Deep obsidian black (`#090C12`) with glowing electric cyan and hot magenta.
- **Live Non-Destructive Theme Switching**:
  - Live theme switcher dropdown in both the Settings & About modal and The Klang Editor.
  - Instantly re-skins the UI without restarting the DAW and persists in user properties.
- **Zen Mode (Visual Ergonomics & Anti-Fatigue)**:
  - User preference in Settings & About modal (SettingsModal.h) toggling between [Fluid & Dynamic] and [Zen Mode (Minimal)].
  - Dims non-essential visual eye candy, freezes decorative scope wobbles, disables flashing modulation halos, and converts meters into steady, functional informational displays for fatigue-free marathon sessions.

### 8. Desktop Feedback, Interaction Remediation & Theme Polish Patch — ✅ COMPLETED
*Detailed Plan: [`.agents/pipeline/plans/completed/2026-10-08_v040_interaction_and_ui_polish.md`](../.agents/pipeline/plans/completed/2026-10-08_v040_interaction_and_ui_polish.md)*  
*Goal: Remediate first-round desktop testing feedback on The Klang Farmer before cutting release:*
- **Interaction Contract Fixes**:
  - Double-clicking a slider opens the Smart Value text editor without resetting parameter value.
  - Alt-clicking cleanly resets parameter value to default.
- **Audition `TRIGGER` Button**:
  - Wired to lock-free audio thread trigger queue with verified audio output (>0.05 peak).
- **Tactile Vector Dice Randomizer**:
  - Replaced text "d6" buttons with custom vector 6-sided die icon (`DiceButton`) with high-contrast pips across cards and header.
- **Floating Callout Popovers**:
  - Replaced OS-style popup menu on right-click with themed `juce::CallOutBox` popovers on parameters and module cards.
- **Extended 7-Theme Engine & Settings Quick-Change**:
  - Curated 7 distinct palettes in `assets/themes/theme.json` (`cyberpunk`, `cykranosh`, `dexciyan`, `boring`, `matrix_green`, `amber_crt`, `tracker_ft2`).
  - Added live theme selector dropdown and quick-change background/accent color swatch pills in `SettingsModal`.
- **Text Truncation Audit**:
  - Measured bounding box clearances across custom sliders, card titles, and status bar across all DPI display scales; verified 0 truncations across 3 resolutions and 4 DPI scales.
- **Dual-Configuration Regression Testing & Deployment**:
  - 100% test passes across `dsp_tests.exe` and `gui_tests.exe` in both Debug and Release configurations (Release: 375/375, Debug: 374/374).
  - Deployed cleanly via `deploy.ps1` to `current_build/` and system `C:\Program Files\Common Files\VST3\`.

---

## 🚀 Milestone: v0.4.1 "The Complete Control Surface & UI Shells"
> **Design Blueprint & Sidecar:** [`.agents/sidecar/ui_ux_deep_research.html`](../.agents/sidecar/ui_ux_deep_research.html)  
> **Staged Factory Plan:** [`.agents/pipeline/plans/drafts/2026-10-08_v041_ui_shells.md`](../.agents/pipeline/plans/drafts/2026-10-08_v041_ui_shells.md)  
*Focus: Implement the complete suite-wide front-end UI framework, control shells, navigation containers, and interaction contracts on The Klang Farmer. This delivers all UI/UX surface paradigms as functional, test-covered UI shells prior to the v0.4.5 Planter refresh and v0.5.0 DSP expansion.*

### 1. Persistent Lower Modulator Dock Strip (`ModulatorDock`)
*Goal: Provide instant, one-click visual feedback and drag-and-drop modulation assignment across all active pages without menu diving.*
- **16-Tile Unified Dock:** 4 LFOs, 4 Envelopes, 4 Macro Knobs, and 4 Random/Chaos sources housed in a persistent, collapsible lower chassis strip.
- **Micro-Visualizers & Caching:** Static tile backgrounds cached via `setBufferedToImage(true)`, with animated waveforms rendered at 30 Hz via lightweight dirty-rect updates.
- **Drag-and-Drop Drag Pill:** Dragging a modulation tile's drag pill reveals an interactive glowing target halo around compatible sliders.
- **Z-Order Bézier Overlay Layer:** Transparent overlay (`ModulationOverlayLayer`) that draws dynamic Bézier splines connecting modulation sources to targets with dirty-rect invalidation (`repaint(unionBounds)`).

### 2. Alt+Click Flip Cards (`FlipCardContainer`)
*Goal: Solve the 4-control surface limit for deep parameter modulation without cluttering the primary sound-design workspace.*
- **Tactile Card Flip State Machine:** Holding `Alt` and clicking any parameter slider smoothly flips the containing 4-control card into its `ModulationInjectorView`.
- **4-Fader Depth Injector:** The 4 encoders/sliders dynamically re-map to bipolar modulation depth controls for the 4 most prominent sources (e.g. LFO1, LFO2, ENV1, MACRO1).
- **Zero-Allocation APVTS Binding:** Inactive view is hidden (`setVisible(false)`) while preserving APVTS slider attachments, avoiding tear-down allocations.
- **Accessibility & Contrast:** Colorblind-friendly shape-coded indicators (solid for envelopes, dotted for LFOs, dashed for randomizers).

### 3. 3-Column Slide-Over Preset Browser (`PresetBrowserOverlay`)
*Goal: Overhaul preset management from antiquated OS dropdowns to a modern, tag-based sound discovery engine.*
- **Column 1 (Taxonomy & Tag Filters):** Factory vs. User banks, sonic categories (Bass, Kick, Snare, Drone, Glitch), mood tags, and author filters.
- **Column 2 (Preset List & Live Audition):** Search-filtered list with instant Spacebar / Play icon (`▶`) audition triggering the lock-free audio thread trigger queue.
- **Column 3 (Preset Details & Playable Macros):** Author notes, creation date, star rating, and 4 interactive macro knobs allowing immediate performance auditioning before loading.
- **Smooth Drawer Animation:** Slide-over panel docking from the right edge with backdrop blur/dimming over the inactive rack.

### 4. Fullscreen Modulation Matrix Table (`ModMatrixModal`)
*Goal: Provide power users with an exhaustive, spreadsheet-style routing table for complex multi-source modulation inspection.*
- **Dense High-Contrast Table:** Scrollable JUCE `TableListBox` showing Source, Target, Bipolar Depth slider, Smoothing/Curve, and Mute/Solo toggles.
- **Target Filtering:** Search filter to isolate all modulators routed to a specific block (e.g., `filter_*`).
- **One-Click Unlink:** Tactile clear/bypass buttons per routing row for rapid cleanup during mixing.

### 5. Macro & Performance "Play View" Page
*Goal: Stage and live-performance mode optimized for high-visibility tactile control and macro morphing.*
- **Giant 4-Macro Stage Interface:** 4 massive rotary macro controls with large, high-legibility JetBrains Mono value readouts.
- **Integrated Live Oscilloscope & Reactive Ring:** Audio-reactive glow around macros reflecting active modulation energy.
- **Single-Click Switcher:** Accessible directly from top header page tabs (`[PLAY]`).

---

## 🚀 Milestone: v0.4.5 "The Klang Planter Refresh"
*Focus: Bringing The Klang Planter to 100% architectural, visual, and interaction parity with The Klang Farmer's modernized v0.4.0 Neo-Slate foundation.*

### 1. Planter 4-Controls-Per-Card & Chassis Architecture
- Re-architect `PlanterEditor` to follow the standardized 4-controls-per-card modular layout.
- Eliminate legacy monolithic panel groupings in favor of clean, swappable synth module cards.
- Implement header navigation tabs with integrated oscilloscope and master limiter callout.

### 2. Neo-Slate Vector UI & Theming Parity
- Apply the dark matte titanium and JetBrains Mono vector design language to Planter.
- Connect Planter to the unified 7-palette theme engine (`ParameterManager::loadTheme`, quick-change swatches).
- Replace all legacy controls with Arcade Meter Sliders and vector `DiceButton` randomizers.

### 3. Popover Callouts & Interaction Scheme
- Adopt the Horizontal Action-Bar popover on right-click for all Planter sliders.
- Enforce the universal interaction contract: Double-click = Smart Text Entry, Alt-click = Default Reset, Right-click = Popover Callout.

### 4. Regression & Parity Validation
- Expand `gui_tests` to achieve 100% parameter reflection and component coverage across the refreshed Planter.
- Verify dual-config Debug and Release passes with zero regressions.

---

## 🚀 Milestone: v0.5.0 "The Sound & Chaos Update"
> **Tracking & Research:** [Alpha Juno & Wavefolder Spec](specs/fx_catalog_expansion.md) | [2026 Agentic Architecture Deep Research](https://github.com/codygratner/TheKlangResearch/blob/main/agentic/2026-10-08_Agentic_Architecture_Deep_Research.md)
*Focus: Sonic Expansion, Workflow Disruption, and Modulation.*

### 1. DSP Modulation Engine & Real-Time Matrix Routing
*Goal: Wire the real-time DSP modulation summing engine to the v0.4.1 Flip Card shells and Mod Matrix table.*
- **Lock-Free Modulation Accumulators:** Audio-thread per-block modulation accumulation for 4 LFOs, 4 Envelopes, 4 Macros, and 4 Random sources into target parameters with zero allocations.
- **Dynamic Re-Mapping:** Encoders control real-time DSP modulation depth indices for LFO1, LFO2, ENV1, and ENV2.
- **Real-Time Shape-Based Ring Animation:** Animate target knob rings with active modulation energy at 30 Hz via lock-free atomics.

### 2. Transparent Quota Telemetry & Language Server Probe (`/quota` Skill)
*Goal: Provide instant, transparent visibility into Antigravity model quotas (5-hour rolling bucket, weekly tier allowances) without diving deep into IDE settings menus.*
- **Investigation & Probing**:
  - Probe the local running `language_server.exe` gRPC/HTTP bridge (`localhost:61440/61441`) and Google Cloud Code endpoint to determine if quota/bucket metrics are accessible via a lightweight local socket call.
  - Evaluate creating a custom slash command skill (`/quota`) or status bar widget that displays active tier allowances on demand in <50ms without network roundtrips.
- **Guardrails**:
  - Strictly on-demand execution (never polled automatically during every model evaluation to prevent latency, token bloat, and rate-limiting).

### 0.6. Rigorous GPLv3 Codebase Audit & Copyleft Hygiene Gauntlet ("FOSS Forever")
*Charter & Standard: [`docs/THIRD_PARTY_LICENSES.md`](THIRD_PARTY_LICENSES.md)*  
*Research & Provenance: [`docs/https://github.com/codygratner/TheKlangResearch/blob/main/subagents/2026-10-08_Subagent_Deep_Research_Harvest.md`](https://github.com/codygratner/TheKlangResearch/blob/main/subagents/2026-10-08_Subagent_Deep_Research_Harvest.md)*  
*Goal: Conduct an exhaustive codebase-wide licensing and provenance audit to guarantee strict GNU General Public License v3.0 (GPLv3) compliance, clean copyleft reciprocity, and respectful attribution to all upstream open-source authors.*
- **Universal SPDX License Headers**:
  - Audit 100% of `.h`, `.cpp`, and `.cmake` files across `source/`, `test/`, and build scripts.
  - Enforce standard header tag: `// SPDX-License-Identifier: GPL-3.0-or-later` and `// Copyright (C) 2026 Cody Gratner & The Klang Suite Contributors`.
- **Third-Party DSP Provenance & Inline Adaptation Docblocks**:
  - Perform a deep code audit across `ModularBlocks.h`, `FastMath.h`, and DSP modules to identify all algorithms adapted or inspired by external open-source work (Mutable Instruments, ChowDSP, EarLevel, DAFX, MusicDSP).
  - Annotate each adapted block with a standardized provenance docblock documenting:
    1. Original author and project name.
    2. Upstream repository URL / publication reference.
    3. Original permissive license (MIT, BSD-3-Clause, Apache 2.0, Public Domain).
    4. Exact modifications made for The Klang Suite (vectorization, ZDF conversion, 4-knob parameter adaptation).
- **Copyleft Compatibility & Incompatible License Sweep**:
  - Verify that every third-party component ingested into the repository is 100% compatible with GPLv3 copyleft terms.
  - Mathematically verify zero inclusion of non-commercial restrictions (e.g., CC-BY-NC), advertising clauses, or proprietary NDA SDKs.
- **JUCE 9 GPLv3 Compliance Verification**:
  - Verify `CMakeLists.txt` build configurations strictly adhere to the JUCE 9.0.3 GPLv3 open-source requirements.
- **Automated Licensing Linter (`gui_tests` or pre-commit)**:
  - Add an automated regression assertion to `gui_tests` (or a pre-flight CI script) that scans all files in `source/` and hard-fails the build if any C++ file lacks an SPDX tag or provenance block.
- **Attribution Ledger Parity**:
  - Cross-verify that 100% of external algorithms documented in code are cataloged in [`docs/THIRD_PARTY_LICENSES.md`](THIRD_PARTY_LICENSES.md) and surfaced in the upcoming v0.6.0 in-app About credits modal.

### 0.7. Tier 1 SIMD Voice Summation & Branchless FM Phase Accumulators
*Goal: Optimize real-time FM operator phase modulation and polyphonic voice summation using JUCE 9 SIMD wrappers and branchless bitwise math.*
- **SIMD Voice Summation (`juce::dsp::SIMDRegister<float>`)**:
  - Migrate polyphonic voice summation from sequential loops to hardware-abstracted 64-byte aligned SIMD registers (4-lane SSE / 8-lane AVX2).
  - Adopt a Structure-of-Arrays (SoA) layout for active voice synthesis buffers to eliminate cache-line thrashing.
- **Branchless Power-of-Two FM Phase Wrapping**:
  - Replace conditional phase wrapping with 32-bit fixed-point integer phase accumulators (`uint32_t`) and power-of-two lookup table indexing with bitwise masking (`& 4095`).
  - Completely eliminates CPU branch mispredictions and CRT transcendentals in hot FM feedback and cross-modulation loops.

### 0.9. The Klang Editor CalloutBox Preview & Live Theming Harness — ✅ COMPLETED
*Goal: Bring full parity and visual inspection capability for all modal CalloutBoxes directly into The Klang Editor, backed by automated GUI test coverage.*
- **Editor Callout Inspection Toolbar**:
  - Add dedicated preview triggers in The Klang Editor's property inspector for floating callout components:
    1. `PlanterLimiterCalloutComponent` (Master Limiter mini-card).
    2. `SliderCalloutComponent` (Snap points, modulation routing, numeric entry).
    3. `SelectorCalloutComponent` (Discrete selector default reset menu).
- **Data-Driven Theming Integration**:
  - Expose callout chassis dimensions, border radii, accent colors, and typography profiles in `assets/controls/global_ui.json` under `"callout_styles"`.
  - The Editor allows live adjustment of callout padding, knob width, and typography with real-time visual preview.
- **Automated Test Coverage (`test/gui_tests.cpp`)**:
  - Expand the Editor functional test suite to programmatically open, render, and dismiss every registered CalloutBox variant with zero leaks and 100% paint assertion success.

### 0. Automated Version Bump Guardrail (`/cut-release` Skill) — ✅ COMPLETED
*Skill Definition: [`C:\Users\codyg\.gemini\config\skills\cut-release\SKILL.md`](file:///C:/Users/codyg/.gemini/config/skills/cut-release/SKILL.md)*  
*Goal: Formalize the "Version Bump = Clean Slate" workflow by building a dedicated AGY slash command to execute the 5-stage pre-release regression gauntlet safely.*
- **Phase 1 (Safe Cruft Sweep)**: Auto-cleans ephemeral scratch scripts (`*.tmp`, `temp_*.txt`, `update_*.py`); halts if uncommitted edits to tracked C++/JSON files exist.
- **Phase 2 (Dual-Config Regression Gauntlet)**: Compiles and runs `Debug` (asserts & memory checks) and `Release` (100% `dsp_tests` & `gui_tests` passes, plus `pluginval` host validation). If any failure occurs, halts and rolls back to Bug Squashing Mode.
- **Phase 3 (Version Bump & Tagging)**: Updates `CMakeLists.txt` and `source/VersionChecker.h`, runs `deploy.ps1`, commits `chore(release): bump version to vX.X.X`, and creates annotated Git tag.
- **Phase 4 (Desktop Sanity & GitHub Remote Push)**: Prompts user to verify deployed binaries, then prompts to push active branch and release tags to GitHub (`git push origin <branch> --tags`).
- **Phase 5 (Summon the Harvester)**: Interactive modal prompt to archive institutional memory into `docs/DEV_HISTORY.md`, slice previous releases to `docs/archives/`, wipe chat transcripts, and reset context clues for a clean slate kickoff.

### 0.8. Smart Model Auto-Detection & Verification Badging (Frictionless Kickoff Guardrail) — ✅ COMPLETED
*Rule Reference: [`GEMINI.md`](../GEMINI.md)*  
*Goal: Eliminate redundant execution pause gates and modal deadlocks when the active AI agent model and thinking level already match or exceed the task's required complexity tier.*
- **Contextual Self-Inspection**: The agent checks its active model identity and extended thinking level from session context instructions.
- **Matched Tier Auto-Proceed**: If active configuration matches the required tier (e.g. Flash 3.8 High on Tier 2), outputs a subtle non-blocking verification badge (`✓ Model Verified: <Model> (<Thinking>) matches Tier <N>`) and begins immediately without pausing.
- **Mismatched Tier Quota Defense**:
  - If lower than required (e.g. Flash on Tier 1 DSP math): Full advisory banner + mandatory hard-pause to switch models.
  - If higher than required (e.g. Pro 3.1 on Tier 2/3 task): Advisory banner + 5-minute downgrade timer (Tier 1 on 2) or hard pause (Tier 1 on 3) to prevent accidental Pro burn.
- **Modal-Free Builder Gate**: Builder never pops up blocking `ask_question` modals during task initialization, ensuring the IDE model picker in the footer is never locked.

### 0.10. Threshold-Aware Factory Clean Slate & Soul Harvest Protocol — ✅ COMPLETED
*Rule Reference: [`GEMINI.md`](../GEMINI.md)*  
*Goal: Balance pristine context hygiene with working memory retention in Klang Industries, preventing model hallucinations while avoiding unnecessary agent amnesia on rapid iterations.*
- **Zero-Friction Young Sessions (< 12 turns)**: Automatically proceeds without prompting to harvest, preserving warmed-up compiler insights, recent file state, and rapid iteration speed.
- **Context-Aware Mature Sessions (> 15 turns)**: In mature sessions with heavy build/test logs, appends a subtle non-blocking 1-line note offering a clean slate (`💡 Factory Context Notice: ~N turns accumulated. Reply 'proceed' to build, or 'harvest & proceed' for a clean slate`).
- **Power-User Override Flag**: Supports explicit `harvest & proceed` / `proceed --harvest` at any time, instantly triggering the `/refresh-context` workflow into `docs/DEV_HISTORY.md` and `context_clues_build.md` before compiling.
- **Modal-Free Safety**: Never uses interactive modals on plan kickoff, keeping the IDE model picker in the footer accessible.

### 1. Parameter Randomization Engine (d6) — ⚡ PULLED FORWARD INTO v0.4.0 — ✅ COMPLETED
*Archived Plan: [`.agents/pipeline/plans/completed/2026-10-08_v040_modulation_matrix_and_regression.md`](../.agents/pipeline/plans/completed/2026-10-08_v040_modulation_matrix_and_regression.md)*
Add a fully JSON-driven contextual randomization system:
- **d6 Icon UI**: Placed on the top-right of every Card (randomizes card), every Page (randomizes page), the global Header (randomizes synth), and the FX selection card (randomizes FX selectors).
- **Right-Click Modal**: Sets the 'Depth' (5%, 15%, 25%, 50%, 75%, 100%).
- **JSON Driven**: The 6 depth strings and 6 visual colors (e.g. Green to Red gradient) are explicitly stored in `global_ui.json` under `"global_strings"` and `"global_colors"`.
- **Randomization Logic**:
  - **Discrete Selectors**: Probability Flip (Depth percentage defines the literal chance that the selector randomly flips to a new choice).
  - **Continuous Sliders**: Incremental Jitter (Slider randomly shifts up to $\pm$Depth% away from its *current* position).
- **Verification Metric**: Contextual d6 buttons wired on 100% of card headers, pushing reversible undo actions into `juce::UndoManager`.

### 2. New Effects Processors Catalog Expansion (Effects 14–27), Universal Mix, & 5-Column Browser Modal
*Detailed Plan: [`docs/new_effects_plan.md`](specs/fx_catalog_expansion.md)*  
  Expand the FX catalog from 13 to 27 algorithms (including Algorithm 15: The Thresher transfer-function waveshaper and Algorithm 27: The Baler 3-band upward/downward compressor) and bundle the Kilohearts-style 5-column categorized modal browser:
    - **Zero-Trademark Acoustic Descriptions**: Never use third-party trademark names (e.g. Geiger, OTT, Snap Heap) in user-facing UI labels, pickers, or tooltips. Use pure functional acoustic terminology: THRESHER (Polynomial & Transfer Function Waveshaper), BALER (3-Band Upward/Downward Dynamics), IRRIGATOR (1-to-8 Modulation Manifold), KLANG MILL (Nested Multi-FX Sub-Rack).
    - **Algorithm 28: The Klang Mill Container**: Embeds an entire 6-slot Klang Mill pedalboard inside an FX slot with 4 front-panel macros and floating window editing ('Mill-ception').
- **Phase 1: Universal Dual-Mode Mix Helper & Core Enums**:
  - Implement shared `computeDualModeMix(float normParam, float& dryGain, float& wetGain)` in `source/DSPBlock.h` (-100% wet crossfade $\to$ 0% pure dry $\to$ +100% parallel additive blend).
  - Update `createFXBlock()` factory and `BlockType` enum with: TransientShaper (14), CustomWaveshaper (15), ChannelMixer (16), StereoEnhancer (17), SpatialAudio (18 - Haas Delay & Binaural HRTF), GatedReverb (19), JunoChorus (20), WaveguideResonator (21), SubGenerator (22), TapeWarmth (23), DynamicFilter (24), PitchTransposer (25), StutterGate (26), Baler (27), and TheMist (29 - Granular Particle Cloud & Freeze) - see [`THIRD_PARTY_LICENSES.md`](THIRD_PARTY_LICENSES.md) and [`2026-10-08_Subagent_Deep_Research_Harvest.md`](https://github.com/codygratner/TheKlangResearch/blob/main/subagents/2026-10-08_Subagent_Deep_Research_Harvest.md).
- **Phase 2: DSP Implementations (`source/ModularBlocks.h`)**:
  - TransientShaperBlock (14), CustomWaveshaperBlock (15), ChannelMixerBlock (16), StereoEnhancerBlock (17), SpatialAudioBlock (18: Unified Mega-Module containing Haas micro-delay widening and Binaural HRTF 3D psychoacoustic positioning; both listed individually in the FX picker with auto-selection), GatedReverbBlock (19), JunoChorusBlock (20), WaveguideResonatorBlock (21), SubGeneratorBlock (22), TapeWarmthBlock (23), DynamicFilterBlock (24), PitchTransposerBlock (25), StutterGateBlock (26), BalerCompressorBlock (27), and TheMistGranularBlock (29: Real-time circular buffer particle cloud with Scrub, Density, Grain Size, and Spray jitter controls; Tier 3 Master/Send effect; inspired by Stephen King's eerie particulate atmosphere - see [`THIRD_PARTY_LICENSES.md`](THIRD_PARTY_LICENSES.md) and [`2026-10-08_Subagent_Deep_Research_Harvest.md`](https://github.com/codygratner/TheKlangResearch/blob/main/subagents/2026-10-08_Subagent_Deep_Research_Harvest.md)).
- **Phase 3: Standardize Existing FX Mix Knobs**:
  - Migrate Chorus, Comb, Flanger, Phaser, Tempo Delay, and Drive to use `computeDualModeMix`.
- **Phase 4: Categorized FX Selection Modal (Kilohearts-Style Browser)**:
  - 5-Column Categorized Modal: Dynamics & Gain, Filters & Tone, Modulation & Pitch, Delay & Space, Lo-Fi & Character.
  - Left-click on FX card header launches browser modal; `<` / `>` steppers continue to cycle sequentially.
  - Zero-allocation verification suite testing dual-mode mix curves and stereo imaging.
- **Phase Smear (Disperser) Enhancements (Algorithm 9)**:
  - *True Zero-DSP Bypass*: Ensure Amount set to 0 strictly bypasses all allpass stages (`if (apfStages == 0) return;`).
  - *Order Switch Replacement*: Retire the subtle 2nd vs 4th order toggle in favor of a post-dispersion **Bipolar Drive** knob (`-100%` hard diode clip $\leftrightarrow$ `0%` clean $\leftrightarrow$ `+100%` warm saturating $\tanh$ drive) with automated gain compensation, turning Phase Smear into a lethal bass and transient sculpting tool.

### 2.5. FX Routing Taxonomy & Micro-Benchmark Gauntlet: Voice, Pre-FX Mono, & Master-Only Tiering
*Goal: Benchmark all 27 DSP effects to scientifically establish optimal signal routing across per-voice cards, monophonic pre-amp stages, and stereo master summing buses.*
- **Automated DSP Micro-Benchmarking Suite (`dsp_benchmarks`)**:
  - Measures cycle counts, memory access patterns, and nanos-per-sample across every algorithm in both 1-channel mono and 2-channel stereo.
  - Profiles cache locality on desktop (x86_64 / ARM64 AVX/Neon) and assesses feasibility on embedded targets (ESP32-P4 RISC-V SIMD).
- **3-Tier Routing & Placement Taxonomy**:
  1. **Tier 1: Universal / Polyphonic (Everywhere)**:
     - Ultra-low CPU, zero phase bloat, safe for per-voice execution (SVF Filter, Tanh Drive, Bitcrusher, Ladder Filter, Classic Chorus, Flanger, Phaser).
  2. **Tier 2: Pre-FX / Pre-Amp (Strict Mono Mode)**:
     - Designed for raw wave shaping and transient sculpting before stereo spatialization and voice summing: Pre-APF Phase Smear, Asymmetric Wavefolder, Tuned Comb Filter / Karplus-Strong, Transient Dirac Click, Ring Modulator.
  3. **Tier 3: Master Bus / Send-Only (Master-Only)**:
     - High CPU footprint, dense buffer allocations, stereo phase smearing, or dynamic bus mastering: Baler 3-Band Compressor, Granular Particle Cloud Scrambler, SOFA/KEMAR Convolution HRTF, Gated Reverb Tank, and Master Brickwall Limiter.
- **Context-Aware FX Picker Indicators**:
  - In the 5-column FX browser modal, subtle tags (`[MONO]`, `[POLY]`, `[MASTER]`) guide sound designers toward acoustically optimal and CPU-friendly choices without artificially locking out creative experimentation.

### 3. Modular 3-Slot Transient Engine (Noise, Sample Players & Impulse Clicks) with Sub-Mixer
  *Goal: Transform the static noise page into a fully modular 3-slot transient layering powerhouse with dedicated sub-mixing before the Pre-Amp Console.*
  - **3 Swappable Modular Slots**: Each slot can independently load one of three transient engines:
    1. **Analog Noise Generator**: White, Pink, Metallic, Velvet, and Vinyl Crackle with dedicated tilt filter and decay envelope.
    2. **Sample One-Shot Player**: Drag-and-drop .wav sample playback with reverse, pitch transposition (-24 to +24 st), and decay.
    3. **Synthetic Impulse / Click**: Ultra-short Dirac delta acoustic click generator (Plastic, Wood, Metal, Glass) with tuning for sharp percussive punch.
  - **Dedicated Transient Sub-Mixer**: Each slot has Level, Pan, and Filter controls, summing into a dedicated Transient bus before hitting the main Voice 1 & 2 mixer.
  - **Choke Groups**: Configurable choking between transient slots (e.g., open vs closed hi-hats or muting clicks).

### 3.5. Custom Wavetable, Vintage Chiptune & Expanded Oscillator Topologies (Mailbag `rj-20`)
*Goal: Expand Voice 1 & 2 synthesis topologies with vintage chiptune engines, modern bit-masking, M8-style wave manipulation, and CyDrums-style sound structures.*
- **CyDrums & Multi-Engine Topologies**: Pre-configured synthesis topologies on Voice 1 & 2 ([2-Op FM], [Wavetable FM], [Ring Mod], [Hard Sync], [Wavetable Morph]) keeping the 4 front knobs invariant.
- **Vintage Computer & Console Oscillator Cores**:
  - **Game Boy / NES (APU + VRC6/FDS)**: Variable pulse width (12.5%, 25%, 50%, 75%), authentic NES triangle stepped staircase, VRC6 saw, and FDS 64-step wavetable accumulator.
  - **Commodore 64 (SID 6581/8580)**: Combined multi-waveforms (pulse+saw, tri+saw) and authentic ring modulation/sync.
  - **PC-Engine (HuC6280)**: 32-step 5-bit wavetable looping registers for lo-fi crunchy retro lead/bass textures.
- **Modern Macro & Wave-Shaping Models**:
  - **M8-Style Wave Synth**: Multiplier, fold/squeeze, and waveform rearranging macros.
  - **Kodamo Mask1 Bit-Masking Engine**: Bitwise masking of oscillator phase to generate gritty, vocal-like harmonic overtones.
  - **Plaits / Braids Macro Models**: Single-knob macro-timbral sound models (Virtual Analog, Additive, Formant, Harmonic Strings).
- **Popover Option & Serum/Vital Standard**:
  - Right-click Carrier/Modulator card popover reveals Oscillator Mode selector.
  - Drag-and-drop loading of standard 2048-sample single-cycle or 256-frame `.wav` wavetables directly onto the card.
  - Bandlimited anti-aliased mip-mapped wavetables in memory.
- **CyDrums Structural Research Spike (Mailbag `rj-21`)**: Prior to active DSP implementation, execute an explore subagent research spike to ingest the CyDrums manual/schematics, mapping its percussion voice models directly into JSON parameter schemas (`assets/controls/*.json`) and C++ DSP signal graphs.
### 4. Advanced Typography Engine (JUCE 9) & Aesthetic Direction (Mailbag `rj-19`)
*Detailed Plan: [`docs/typography_engine_plan.md`](specs/typography_engine.md)*
Implement a JSON-driven, CSS-class style typography system utilizing JUCE 9's advanced text rendering pipeline.
- **Embedded Binary Assets & OFL Invariant**: `.ttf`/`.otf` files are baked into `BinaryData` for 100% cross-platform consistency. Strictly enforce SIL Open Font License (OFL) typefaces (JetBrains Mono for monospace meters/values, Inter or Satoshi for UI copy) to eliminate proprietary font licensing exposure.
- **CSS-Style JSON Classes**: Define global text profiles (e.g., `HeaderStyle`, `TooltipStyle`) in the layout JSON, exposing Font Family, Size, Weight, Tracking (letter-spacing), and Justification.
- **Automated Text-Truncation Test Suite (`gui_tests`)**: Automated regression tests measuring string layout bounds against card boundaries across 100%, 125%, 150%, and 200% DPI display scales to mathematically prove zero string clipping (`bounds.getWidth() >= textLayout.getWidth()`).
- **Unified Module Visual Grammar**: Harmonize card chrome, pill headers, slider gutters, and corner bevels across Voice, FX, and Modulation cards into a single cohesive visual language.
- **Editor Integration**: The JSON Editor tool provides sliders/fields to instantly visualize tracking and weight changes across the UI.

### 5. Continuous Fuzz Testing (DSP Stability)
*Goal: Guarantee absolute DSP stability during extreme generative parameter changes.*
- Implement an automated fuzzing harness that blasts the `processBlock` and `apvts` with randomized, out-of-bounds, and extreme NaN garbage data to mathematically ensure the synth will never crash a host DAW.

### 6. Selector Right-Click Callout 'Reset to Default' Action
*Goal: Provide instant, discoverable default reset capability for discrete selector buttons.*
- Add a top/bottom action button `[Default: <Preset/Mode>]` inside `SelectorCalloutComponent` / right-click menu.
- Clicking the button instantly restores the selector parameter to its JSON-defined default value.

### 7. Voice & Articulation Engine: Gated Staccato Bass, Note-Off Release & Portamento Glide
*Detailed Plan: [`docs/gated_bass_note_off_plan.md`](specs/gated_bass_glide.md)*  
Transform the dual-FM drum synthesizer into a dual-threat drum and bass machine capable of tight, punchy, articulate staccato basslines, sustained drones, and fluid portamento slides:
- **Header Front-Panel Badge (`VOICE / ARTICULATION`)**:
  - `ONE-SHOT` (Default): Traditional drum-machine behavior; ignores MIDI Note-Offs so envelopes decay naturally.
  - `GATED`: MIDI Note-Off immediately cuts the voice with a smooth, pop-free release ramp.
  - `GLIDE ~`: Shows animated glide indicator when portamento is active (`GLIDE: LEGATO` or `GLIDE: ALWAYS`).
  - Left-click toggles One-Shot vs Gated mode; right-click launches `VoiceArticulationCalloutComponent`.
- **Unified Modal Callout (`VoiceArticulationCalloutComponent`)**:
  - **Trigger Mode**: `[One-Shot]` / `[Gated]`.
  - **Note-Off Release**: `1.0 ms` to `30.0 ms` (default `5.0 ms`, logarithmic skew).
  - **Glide Mode**: `[Off]` / `[Legato]` (glides on overlapping notes) / `[Always]` (glides between all notes).
  - **Glide Time / Sync**: Free milliseconds (`5.0 ms` to `2000.0 ms`) vs Tempo Sync (`1/64` to `1/2 bar`).
  - **Glide Slope**: Slew curve control: `Exponential (0.0)` (analog RC curve) &rarr; `Linear (0.5)` &rarr; `Logarithmic (1.0)`.
  - **Legato Retrigger**: `Off (Continuous)` for fluid acid slides vs `On (Punchy)` for modern trap 808 re-striking slides.
- **Smart Defaults + Envelope Popover Overrides (Noise Engineering Gate-Hold)**:
  - 1-click header switch defaults to musical bass behavior: Amp Envelope automatically holds on key gate (decay acts as release tail on note-off); Pitch Envelopes decay immediately (preserving punchy 808 transient click).
  - Deep per-envelope overrides inside right-click Inspector Popovers: `Gate Behavior: [ Auto (Default) | Always Decay | Force Sustain ]`.
- **Planter Voice Modes & Dual-Oscillator Hard Sync Engine**:
  - **Mode Selector**: `[Percussion]` (traditional 2-op FM with fast percussive pitch envelopes) $\to$ `[Bass FM]` (dedicated FM bass engine with sustain, glide, and tighter keyboard tracking) $\to$ `[Dual Osc Sync]` (Carrier and Modulator act as twin free-running oscillators with classic hard-sync phase resets from Osc 1 to Osc 2, detune, and harmonic richness).
  - Integrates with the Bipolar Drive on Phase Smear and gated releases for lethal, heavy analog and FM bass synthesis.
- **Click-Free Semitone-Space Pitch Slew & Release DSP**:
  - Slews pitch in musical semitone space so 1-octave bass slides match 1-octave lead slides identically.
  - Exponential amplitude release ramp via `TbdAudio::FastMath::fastExp`.
  - Zero heap allocations, zero mutexes, and zero DC pops on the audio thread.


### 7.5. [SLOPE 1..4] Make Noise Maths-Style Looping Function Generators
  *Goal: Integrate Eurorack Make Noise Maths / Serge DUSG-style dual slope function generators into the modulation engine.*
  - **Dedicated Modulator Category**: Adds [SLOPE 1..4] alongside LFOs, Envelopes, and Irrigators.
  - **4 Front-Panel Controls**: [Rise] (Attack time 0.5ms - 10s), [Fall] (Decay/Release time 1ms - 20s), [Curve] (Logarithmic <-> Linear <-> Exponential continuous curve morph), and [Cycle] (Looping LFO / VCO toggle).
  - **End-of-Fall (EOF) / End-of-Rise (EOR) Trigger Pulses**: When the envelope finishes its fall phase, it emits a discrete single-sample trigger pulse that can fire Voice 1, Voice 2, or re-trigger another Slope for cascading generative rhythms, polyrhythmic bursts, and ratchets.
  - **Slew Limiter Mode**: Popover setting allowing the slope to act as a portamento/lag processor smoothing incoming discrete modulations.

### 7.6. The Klang-Brain Algorithmic Modulation Generator (Continuous & DSI Pro 2 Triggered Advance)
*Spec Reference: [`docs/specs/spinoffs/the_klang_r1.md`](specs/spinoffs/the_klang_r1.md)*  
*Goal: Provide TKF with deep generative, polymetric, and alternative modulation curves beyond standard LFOs.*
- **5 Algorithmic Modes**:
  1. *Fugue Counterpoint*: 4 playheads reading 1 pattern at varied clock dividers & directions.
  2. *Matriceal Polymeter*: Decoupled parameter loop lengths (Triggers, Velocity, Octave, Gate).
  3. *Stage Pulses & Accumulators*: Metropolix-style variable pulse repeats + M8 sub-tick micro-tables.
  4. *Turing Machine (LFSR)*: Shift-register generative loops with controlled bit-flip mutation probability.
  5. *Axon Neural Network*: Audio Damage-style leaky integrate-and-fire interconnected biological neurons with excitatory/inhibitory synaptic feedback loops.
- **Advance Modes (Continuous vs. DSI Pro 2 Triggered Key-Advance)**:
  - *Continuous Mode*: Advances locked to the DAW host transport and tempo clock.
  - *Triggered / Key-Advance Mode*: Sequencer lanes hold stationary until an incoming MIDI Note-On or gate trigger is received, advancing by exactly 1 step per keypress (Dave Smith Instruments Pro 2 gated polymetric sequencer).
  - *Polymetric Modulation*: Each keypress steps independent parameter lanes of different lengths, allowing filter cutoffs, FM ratios, and wavefolding to morph continuously with live keyboard playing.

### 8. Sound Design Safety: Undo / Redo & A/B State Comparison — ⚡ PULLED FORWARD INTO v0.4.0 — ✅ COMPLETED
*Archived Plan: [`.agents/pipeline/plans/completed/2026-10-08_v040_modulation_matrix_and_regression.md`](../.agents/pipeline/plans/completed/2026-10-08_v040_modulation_matrix_and_regression.md)*  
*Goal: Provide full sound design safety and non-destructive experimentation, essential when rolling the d6 Randomizer.*
- **Header Controls & Keyboard Shortcuts**:
  - Subtle `↶` (Undo) and `↷` (Redo) buttons and a tactile `[ A | B ]` toggle button in the header bar.
  - Global hotkeys: `Ctrl+Z` / `Cmd+Z` (Undo) and `Ctrl+Y` / `Cmd+Shift+Z` (Redo).
  - Right-click context menu on `[ A | B ]`: `Copy State A to B` / `Copy State B to A`.
- **APVTS & Randomizer Transactions**:
  - Integrates `juce::UndoManager` into `KlangCoreProcessor` and APVTS slider gestures.
  - Every d6 randomizer roll pushes a named transaction (e.g., "Randomize Pitch Card", "Randomize Synth") so accidental overwrites can be instantly undone.
- **Verification Metric**: Verified under automated GUI test assertions with seamless state buffer preservation.

### 9. Velocity Sensitivity Curves & MIDI CC Learn — ⚡ PULLED FORWARD INTO v0.4.0 — ✅ COMPLETED
*Archived Plan: [`.agents/pipeline/plans/completed/2026-10-08_v040_modulation_matrix_and_regression.md`](../.agents/pipeline/plans/completed/2026-10-08_v040_modulation_matrix_and_regression.md)*  
*Goal: Calibrate dynamic response for external drum pads/keys and enable instant hardware MIDI controller mapping.*
- **Dynamic Velocity Scaling (Voice & Articulation Modal)**:
  - Selectable response curves: `Linear`, `Exponential` (soft touch / wide dynamics), `Logarithmic` (hard touch), and `Fixed (127)` (essential for uniform electronic/techno drum hits).
  - Velocity Depth slider (`0%` = velocity immune &rarr; `100%` = full dynamic range).
- **Right-Click MIDI CC Learn**:
  - Right-click any parameter knob or slider &rarr; `MIDI Learn` (captures next incoming hardware CC) or `Clear MIDI CC`.
  - Mappings stored in user config and persistent across sessions.
- **Verification Metric**: 100% pass across DSP velocity response tests (`dsp_tests.exe`).

### 10. Panic / Kill Audio (Emergency Silence & DSP Flush) — ⚡ PULLED FORWARD INTO v0.4.0 — ✅ COMPLETED
*Archived Plan: [`.agents/pipeline/plans/completed/2026-10-08_v040_modulation_matrix_and_regression.md`](../.agents/pipeline/plans/completed/2026-10-08_v040_modulation_matrix_and_regression.md)*  
*Goal: Instant safety shutoff protecting ears and studio monitors from runaway delay/reverb feedback or stuck MIDI notes.*
- **Header Trigger & MIDI CC Integration**:
  - Double-clicking the Master Peak Meter / CPU indicator instantly cuts all audio.
  - Also triggers on standard incoming MIDI CC 120 (All Sound Off) and CC 123 (All Notes Off).
- **Pop-Free DSP Buffer Flush**:
  - Applies a sub-millisecond (1ms) exponential fade-out to prevent speaker pops.
  - Flushes all internal delay lines, reverb tanks, and comb filter feedback buffers to zero.
  - Resets active MIDI voice tracking and legato gate memory.
- **Verification Metric**: Verified zero audio pops, clean decay reset, and 100% test pass in `dsp_tests.exe`.

### 8. Evaluate Agentic Workflow Strategy (Context Wipes & Strict Chat Roles)
*Goal: After completing v0.4.0, review how well the two-chat workflow held up against prompt drift and task bleeding.*
- Did the `/clear` command with split `context_clues.md` files sufficiently protect against hidden state?
- Did the strict "Planner vs Builder" guardrail successfully prevent task bleeding and keep architecture decisions centralized?
- **Granular Hybrid Decomposition & 2-Strike Escalation**: Codified Flash 3.8 High as the universal default baseline for both planning and building. Pro High is strictly an exception reserved for complex DSP math/concurrency, and when used for planning, must decompose `PLAN.md` into granular drop-in C++ blueprints so execution drops back to Flash High.
### 11. "The Klang Blip" (MeeBlip SE Standalone Edition) Palette-Cleanser Sidequest (Mailbag `rj-25`)
*Goal: Build a bit-accurate, zero-feature-creep software recreation of the iconic open-source MeeBlip SE hybrid synthesizer as a dedicated standalone VST3/AU/CLAP plugin, packaged as a fun inter-milestone palette cleanser.*
- **Zero-Feature-Creep "Iron Panel" Constraint**:
  - Exact 1:1 hardware front-panel parameter parity (Dual Digital Oscillators, Pulse Width, Sweep, Sub-Osc, Cutoff, Resonance, Decay, Envelope Routing switches).
  - Strictly zero sub-menus, zero secondary tabs, zero hidden mod matrices.
- **TKS Preset Browser Addition**:
  - Embeds The Klang Suite's JSON preset management system (`assets/presets/`), adding full patch saving, loading, and randomizer capability (the one feature the original hardware lacked).
- **GPLv3 Clean Open-Source Provenance**:
  - Ingests the open-source C/assembly firmware algorithms released by James Surine and Peter Kirn (CDM) under GNU General Public License v3.0.
  - Zero Delay Feedback (ZDF) analog Twin-T resonant filter model with non-linear saturation.
- **Sibling Build Target**:
  - Compiles as a lightweight sibling CMake target inside `c:\Dev\TheKlangSuite`, reusing `Source/dsp/` and `Source/ui/` without creating a rogue repository.

---

## 🚀 Milestone: v0.6.0 "The Pro Workflow Update"
> **Tracking & Research:** [TBD-16 Decoupling Spec](specs/spinoffs/the_klang_mill.md) | [2026 Agentic Architecture Deep Research](https://github.com/codygratner/TheKlangResearch/blob/main/agentic/2026-10-08_Agentic_Architecture_Deep_Research.md)
*Focus: Professional DAW Integration, File Management, Preset Library, and Export.*

### 1. JSON Preset Browser, Tagging & State Migration
*Detailed Plan: [`docs/preset_system_plan.md`](specs/preset_system.md)*  
Implement a professional, tag-based preset management system utilizing JSON files for storage.
- **Phase 1: JSON Schema & StateMigrator (`source/PresetManager.h`, `source/StateMigrator.h`)**:
  - Background scanner to instantly build a database from metadata headers without loading full state.
  - Intercept older patches via StateMigrator to inject missing default values.
- **Phase 2: UI Browser Overlay (`source/PresetBrowserComponent.h`)**:
  - Dual-column UI (Tags on Left, Results on Right) with fuzzy text search.
  - "Save As" modal with text inputs for name, author, and tokenized tags.
- **Phase 3: Header Integration & Automated Tests**:
  - LCD-style preset display and `<` `>` stepper buttons in the main header.

### 2. Curated Factory Preset Library & Sound Design Pack (64–128 Patches)
*Goal: Provide professional out-of-the-box sounds showcasing the expanded DSP and sample players.*
- Author 64–128 production-ready drum patches categorised across:
  - **Kicks**: Sub-heavy 808s, punchy acoustic-style kicks, hardstyle industrial distortion kicks.
  - **Snares & Claps**: Metallic FM snares, 80s gated reverb claps, organic transient layers.
  - **Toms & Percs**: Physical modeling resonant tubes, FM bells, alien zaps, and cowbells.
  - **Hi-Hats & Cymbals**: Transient sample-layered hats, choked pairs, and FM metallic cymbal washes.

### 3. One-Click Preset Bank Sharing (`.tkfbank` / `.zip` Import/Export)
*Goal: Zero-friction sharing of user presets and community expansion packs.*
- **Export Bank**: Bundles selected presets, tags, and custom transient sample files into a single compressed `.tkfbank` archive.
- **Import Bank**: Drag-and-drop `.tkfbank` file onto the preset browser to automatically install, categorize, and rebuild the tag cache.

### 4. WAV Render, Multi-Sample & SF2 Export Dialog + Instant DAW Drag 'n' Drop (Features #16 & #17)
*Detailed Plan: [`docs/wav_render_sf2_export_dragndrop_plan.md`](specs/wav_sf2_export.md)*  
Comprehensive offline audio bounce, multi-sample SoundFont 2 (.sf2) bank generation, and zero-friction DAW integration:
- **Phase 1: Offline Render Pipeline & SF2 Builder**:
  - Zero-dependency RIFF sfbk v2.01 binary builder with L/R linked sample headers.
  - Non-blocking atomic note/velocity tracking.
- **Phase 2: Instant DAW Drag 'n' Drop ("Tekno-Style" Header Badge)**:
  - InstantDragBadgeComponent in header displaying miniature waveform preview of the last hit.
- **Phase 3: WAV Render & Export Modal Dialog**:
  - Target mode selection: Last Auditioned Note vs Multi-Sample Range.
  - Format selection: WAV files folder, SoundFont 2 (.sf2) bank, or both.
- **Phase 4: Header Integration & Automated Unit Tests**:
  - Add renderButton and dragBadge to plugin headers.

### 5. Headless Linux CLAP / VST3 Automated CI/CD Runner
*Goal: Ensure multi-platform stability and continuous validation for Linux audio.*
- Add an Ubuntu `aarch64` / `x86_64` container to GitHub Actions building headless Linux CLAP/VST3 binaries on every commit.

### 6. Dual-Tier 2x / 4x Oversampling Engine (Anti-Aliasing)
*Detailed Plan: [`docs/pre_v1_sound_and_workflow_expansion_plan.md`](specs/pre_v1_sound_and_workflow_expansion.md)*  
*Goal: Eliminate FM modulation sideband foldback and non-linear saturation aliasing in both realtime and offline render paths.*
- **Dual-Tier Quality Strategy**:
  - **Realtime / Live**: Selectable `[Off (1x) | 2x | 4x]`. Minimum-phase IIR filters guarantee zero monitoring latency for live finger-drumming and tracking.
  - **Render / Export**: Selectable up to `8x` oversampling for maximum offline fidelity during WAV/SF2 bouncing.
- **DSP Engine Wrapping**:
  - `juce::dsp::Oversampling<float>` wraps the core voice and non-linear effects path in `processBlock()`.
  - Zero allocation audio-thread invariant strictly preserved by pre-allocating oversamplers in `prepareToPlay()`.

### 7. 4 Performance Macro Knobs (TBD-16 Hardware Aligned) — ⚡ PULLED FORWARD INTO v0.4.0
*Detailed Plan: [`docs/pre_v1_sound_and_workflow_expansion_plan.md`](specs/pre_v1_sound_and_workflow_expansion.md)*  
*Goal: Instant front-panel performance tweaking mapped 1:1 to Page 1 of the dadamachines TBD-16 hardware.*
- **Global Front-Panel Access**:
  - 4 persistent macro knobs accessible from the plugin header across all pages.
  - Aligned 1:1 to the 4 physical endless push-encoders on **Page 1 of the dadamachines TBD-16** hardware groovebox.
- **Right-Click Modulation Assignment**:
  - Right-click any parameter knob or slider &rarr; `Assign to Macro 1–4`.
  - Configurable bipolar modulation depth (`-100%` to `+100%`).
  - Macro assignments and positions serialized directly into JSON preset files (`PresetManager.h`).

### 8. Zero-Server GitHub Crash Reporting Engine
*Detailed Plan: [`docs/github_crash_reporting_plan.md`](specs/crash_reporting.md)*  
*Goal: Capture real-world crash logs and stack traces from beta testers directly into GitHub Issues with zero server infrastructure, zero hosting costs, and zero secret token leaks.*
- **Pre-Filled GitHub Issue URL Generator**:
  - When an unexpected termination is detected, prompts: *"The Klang Farmer encountered an unexpected shutdown. Submit report to GitHub?"*
  - Clicking launches the user's default browser to a pre-filled GitHub Issue URL with markdown callstack and environment info.
- **Hybrid Crash Detection**:
  - **In-Process Scoped Exception Handling**: Structured exception catching (`__try / __except` / signal traps) around top-level DSP and UI callbacks generates immediate stack traces and mutes audio before host DAW crash.
  - **Heartbeat Session Lockfile**: `%APPDATA%/TheKlangFarmer/sessions/session_active.lock` detects abnormal host DAW crashes on next launch.
- **Full Scrubbed Diagnostics**:
  - Gathers OS, host DAW name/version, buffer size, sample rate, Git commit hash, active preset/FX, and demangled C++ call stack.
  - Automatically scrubs local usernames from paths (e.g. `C:\Users\<redacted>\...` &rarr; `<UserPath>`) to protect privacy.

### 9. One-Time Quick Tour & Gesture Revelation ("Right-Click is the Way")
*Detailed Plan: [`docs/one_time_quick_tour_plan.md`](specs/one_time_quick_tour.md)*  
*Goal: Provide a sleek, non-intrusive first-launch onboarding card that introduces users to the tactile power of right-click quick snaps, randomizer menus, voice articulation callouts, and double-click resets.*
- **Unobtrusive Single-Screen Overlay**:
  - Automatically pops up on first launch only; persistent state stored in `%APPDATA%/TheKlangFarmer/settings.json`.
  - Dismissible with a single click outside the card, hitting `ESC`, or clicking `[ GOT IT, LET'S PLAY ]`.
  - Can be reopened anytime via the header `[ ? ]` button or Settings gear menu.
- **The Core Message**:
  - Visual gesture breakdown emphasizing:
    1. Right-click knobs & sliders for Quick-Snap intervals and MIDI CC Learn.
    2. Right-click selectors for default resets and full dropdown lists.
    3. Right-click card/page headers for the d6 Randomizer depth menu.
    4. Right-click the Voice badge for Gated Bass & Glide, and right-click `[A|B]` to copy states.
    5. Double-click any parameter to reset to factory default.
- **100% JSON-Driven Copy**:
  - All headings, icons, descriptions, and button labels parsed from `assets/controls/global_ui.json` under `"quick_tour"`.

### 10. Comprehensive FOSS & POSS Attribution, License Ledger & About Modal Expansion
*Goal: Provide rigorous, ethical, and transparent attribution to open-source DSP pioneers, papers, and libraries powering The Klang Suite.*
- **Third-Party FOSS/POSS Ledger (`docs/THIRD_PARTY_LICENSES.md`)**:
  - Detailed directory indexing all upstream inspirations, open-source algorithms, mathematical papers, and authors:
    - **Émilie Gillet (Mutable Instruments)**: Algorithmic inspiration for granular texture processing (Clouds/Beads), modal resonators (Rings), and macro-oscillators (Plaits/Braids) under MIT / CC-BY-SA, strictly honoring trademark naming protections.
    - **Jatin Chowdhury (ChowDSP)**: Non-linear diode saturation, Wave Digital Filters (WDF), and FastMath foundations.
    - **Will Pirkle & Vadim Zavalishin**: Zero-Delay Feedback (ZDF) and Virtual Analog filter topologies.
    - **Paul Kellett & Nigel Redmon (EarLevel Engineering)**: Biquad and polyphase half-band decimation algorithms.
    - **Sean Costello (Valhalla DSP)**: Feedback delay networks and diffuse reverberation structures.
    - **Tom Whitwell (Music Thing Modular)**: Turing Machine 16-bit LFSR generative shift register topology.
    - **dadamachines**: Open-source CTAG TBD-16 groovebox hardware architecture.
  - Documents license types (MIT, BSD-3, Apache-2.0, GPL-3.0, CC-BY-SA), copyright notices, and upstream repository hyperlinks.
- **Settings & About Modal UI Expansion (`SettingsModalComponent`)**:
  - Add a dedicated clickable `[ Credits & Open-Source Licenses ]` button inside the About dialog.
  - Displays a clean, scrollable in-app attribution viewer with clickable hyperlinks to author websites and repositories.

---

## 🚀 Milestone: v0.7.0 "The Visual Polish & UI Mastery Update"
*Focus: Professional Boutique Aesthetics, High-DPI Scaling, 60 FPS Visualizers, and Tactile Industrial Hardware Styling.*

### 1. Dynamic UI Scaling (100% to 200%) & High-DPI Vector Crispness
*Goal: Guarantee razor-sharp visuals on 4K, 5K, and Retina displays.*
- **Interactive Free Resizing**: Bottom-right corner drag handle allowing smooth proportional scaling with fixed aspect ratio.
- **Header Zoom Presets**: Stepped zoom selector (100%, 125%, 150%, 175%, 200%) persisted in global config JSON.
- **100% Vector Rendering**: Replace any remaining raster graphics with pure JUCE vector paths and procedural glyphs so lines never blur at high scaling factors.

### 2. Tactile Industrial Chassis Shading & Depth (Baby Audio / Elektron Aesthetic)
*Goal: Transform flat 2D cards into a rich, tactile piece of boutique hardware.*
- **Subtle Drop Shadows & Recessed Bezels**: Procedural soft drop shadows behind cards and sunken, beveled card slots.
- **Chassis Texturing**: Powder-coated matte chassis background rendering with brushed aluminum card borders.
- **Tactile Button Physics**: Depressed click animations with shadow shifts on buttons and selector steppers.

### 3. 60 FPS Smooth Visualizers, Peak Meters & Realistic LED Glow
*Goal: Bring the plugin to life with fluid, responsive feedback.*
- **Tear-Free 60 FPS Oscilloscope**: Lock-free circular FIFO streaming audio waveform points to `MiniOscilloscopeComponent` rendering at a solid 60 FPS with zero audio-thread overhead.
- **Analog-Style Meter Ballistics**: Smooth peak meters with calibrated attack and gentle exponential decay.
- **Realistic LED Bloom**: Soft radial bloom/glow shaders for active LEDs, indicators, and visualizer traces.

### 4. Curated Theme Palette Presets (JSON Driven)
*Goal: Provide distinctive, switchable visual flavors for different studio moods.*
- Author curated, pre-made theme palettes in `assets/themes/`:
  - **Cyberpunk Neon (Default — High-Impact Showcase)**: Deep obsidian black chassis with glowing electric cyan, hot magenta, and radioactive green LEDs (engineered for maximum visual punch in videos, thumbnails, and screenshots).
  - **Cykranosh (Creator's Signature)**: Deep desaturated slate navy chassis (\#161B22\ / \#1A202C\), muted deep blue card surfaces, with eerie ghostly teal (\#4EBEB1\), ice blue, and starlight silver indicators (custom tuned for zero eye fatigue during live sets and marathon studio sessions).
  - **Industrial Carbon**: Dark matte charcoal chassis with warm amber & muted industrial orange accents.
  - **Vintage Hardware (80s Cream)**: Retro off-white chassis with classic slate blue & brick red hardware buttons.
  - **High-Contrast Dark Studio**: Ultra-clean monochrome palette for minimal distraction.
    - **The 'Vibe Coder' IDE Essentials Pack**:
      - **Dracula**: Iconic vampire dark purple background (`#282A36`) with bright cyan, hot pink, and lime green LED accents.
      - **Monokai Pro**: Classic code editor dark grey (`#2D2A2E`) with warm peach, yellow, and vibrant green controls.
      - **Nord**: Arctic frost blue-grey palette with icy cyan and pastel aurora highlights.
      - **Solarized Dark**: Precision teal-grey background with soft amber and cyan highlights.
      - **Solarized Light (The Cursed Mode)**: Warm cream/beige background with high-contrast slate text and warm pastel LEDs for daylight studio sessions.
- Instant, non-destructive live theme switching from the Settings & About modal without restarting the DAW.

---

## 🚀 Milestone: v1.0.0 "The General Availability Launch"
*Focus: Official Public FOSS (GPLv3) Release of The Klang Farmer & The Klang Planter.*

### 1. Multi-Platform Automated Installers & Distribution Packaging
*Goal: Provide zero-friction, professional installers across all major operating systems.*
- **Windows InnoSetup Installer**:
  - Automatically installs VST3 binaries to `C:\Program Files\Common Files\VST3\`.
  - Installs Standalone executables, factory preset library, and documentation.
  - Registers uninstaller in Windows Control Panel / Settings.
- **macOS Signed & Notarized `.pkg` Installer**:
  - Automatically deploys VST3 to `/Library/Audio/Plug-Ins/VST3/` and AU component to `/Library/Audio/Plug-Ins/Components/`.
  - Signed with Apple Developer ID and notarized via `notarytool`.

### 2. Comprehensive User Manual & Interactive Guide
*Goal: Empower sound designers and music producers to master the engine.*
- Author an illustrated, searchable HTML and PDF documentation manual:
  - Deep-dive diagrams explaining the dual FM carrier/modulator phase architecture.
  - Complete 4-parameter reference guide for all 26 DSP effects in the catalog.
  - Keyboard shortcuts, right-click quick-snap intervals, and MIDI CC mapping guide.

### 3. Launch Demo Reel & Audio Showcase
*Goal: Showcase the sonic versatility of the engine on GitHub and social media.*
- Produce high-fidelity audio stems and video demos across multiple musical genres:
  - Industrial Techno, Cyberpunk, 80s Gated Retro Synthwave, and Punchy Modern Trap/Hip-Hop.

---

## 🚀 Milestone: v1.1.0 "The Klang Box Hardware Universe (Post-1.0)"
*Focus: Standalone Hardware Synthesizer (The Klang Box / TKB), dadamachines tbd-16 Integration, and Zynthian V5 Linux Port.*

### 1. dadamachines tbd-16 Groovebox Integration (TKB-TBD)
*Detailed Plan: [`docs/specs/tbd16_klang_seed_effects.md`](specs/tbd16_klang_seed_effects.md)*  
*Official Reference Links:*
- **TBD-16 Hardware Docs**: `https://docs.dadamachines.com/tbd-16/`
- **TBD Platform Architecture**: `https://docs.dadamachines.com/tbd-platform/`
- **CTAG TBD C++ SDK & Synth Framework**: `https://dadamachines.github.io/ctag-tbd/index.html`

Deploy the pure C++ DSP engine onto the open-source **dadamachines tbd-16** platform:
- **Architecture**: Dual-core **ESP32-P4 RISC-V @ 400 MHz** (Audio DSP) + **RP2350B @ 150 MHz** (UI, Sequencer, 2.4" OLED, 30 RGB buttons) + **ESP32-C6** (Wi-Fi/Ableton Link).
- **Native 4-Encoder Mapping**: The unit features **4 endless push-encoders**; each 4-knob card and 4-knob FX slot in our engine maps directly to one 4-encoder screen page on its 2.4" OLED!
- **2.4" OLED Vector Engine**: Render an ultra-crisp 1-bit monochrome vector layout displaying 4 horizontal meter bars stacked vertically, matching the Neo-Slate UI.
- **30-Button Grid Navigation**: Direct card jump buttons (Buttons 1–8 for Cards 1–8, Buttons 9–12 for FX Slots 1–4, dedicated triggers for `[INIT]`, `[TRIGGER]`, and `[BYPASS]`).
- **Lock-Free Multi-Core IPC**: Stream encoder updates from RP2350B across the internal bus to ESP32-P4 `ParameterManager` without blocking the audio render callback.
- **CTAG Audio Callback Adapter**: Zero-overhead wrapper (`Tbd16AudioDriver.cpp`) feeding `TheKlangFarmer` or `TheKlangPlanter` core DSP directly into the CTAG DMA buffer stream.

### 2. The Klang Box: Daisy Edition (TKB-Daisy) — Stereo Desktop & Eurorack Hardware
*Detailed Plan: [`docs/embedded_dsp_and_hardware_port_plan.md`](specs/tbd16_klang_seed_effects.md)*
A self-contained, portable stereo FM drum synthesizer and Eurorack module built on the **Electro-Smith Daisy Seed**:
- **Processor & Memory**: STM32H750 ARM Cortex-M7 @ 480 MHz with **64 MB high-speed SDRAM** for immense reverb/delay buffers.
- **Onboard Codec**: Integrated AK4556 24-bit 96 kHz stereo audio DAC/ADC.
- **Hardware Build Complexity**: Low/Moderate. Simple breakout PCB housing Daisy Seed, 4 rotary encoders, 128x64 OLED screen, MIDI TRS/DIN, and 1/4" stereo outputs. Perfect for rapid hardware prototyping!

### 3. The Klang Box: Studio Edition (TKB-8) — Teensy 4.1 8-Voice Multi-Output Drum Machine
*Detailed Plan: [`docs/embedded_dsp_and_hardware_port_plan.md`](specs/tbd16_klang_seed_effects.md)*
A flagship studio drum machine built on **PJRC Teensy 4.1** featuring discrete individual analog voice routing:
- **Processor**: NXP i.MX RT1062 ARM Cortex-M7 @ 600 MHz running 8 mono drum voices (~14.7% CPU load).
- **Multi-Channel DAC**: Cirrus Logic **CS42448 8-Channel 24-bit 192 kHz Codec** driven via TDM.
- **8 Discrete Analog Outputs**: 8 individual 1/4" phone jacks plus Master Stereo L/R. Switched normalled jacks automatically remove a voice from the master stereo mix when an external cable is plugged in, allowing each drum voice to be processed through separate outboard preamps, compressors, and mixing consoles!
- **Hardware Build Complexity**: Advanced. Custom PCB housing Teensy 4.1, CS42448 daughterboard, 10 switched phone jacks, 4 encoders, and OLED screen.

### 4. Zynthian V5 / V4 Standalone Hardware Port (TENTATIVE)
*Detailed Plan: [`docs/zynthian_port_plan.md`](specs/spinoffs/zynthian_port.md)*
Deploy headless Linux LV2 / CLAP plugins onto the open-source Zynthian hardware ecosystem:
- **Compute**: Raspberry Pi 5 (Quad-core ARM Cortex-A76 @ 2.4 GHz) running 64-bit ZynthianOS.
- **Zero GUI Overhead**: Pure headless real-time DSP without X11/OpenGL overhead.
- **1:1 4-Encoder Page Mapping**: Maps 1:1 onto Zynthian V5's 4 physical optical push-encoders and 800x480 touchscreen.

---

## 🌌 N'kai (NKAI) Asymmetric Sidecar Framework
*Focus: Asymmetric Sidecar Framework, Visual Grill Lab, and Triage Workspace.*

### 1. The 9-Flight Consolidated Itinerary
*Status: 🏆 COMPLETED (100% Consensus Reached: 17 Approved, 2 Tabled for v0.2.0, 0 Killed, 0 Pending)*
Consolidated 19 fragmented system sanity audit topics into 9 cohesive flights across 4 phases:
- **Phase 1: Foundations (State & IPC)** $ightarrow$ F1 (Canonical State: 6.16+6.7) & F2 (Execution Bridge: 6.17+6.18)
- **Phase 2: Orchestration (Skills & Portability)** $ightarrow$ F3 (Skill Taxonomy: 6.4+6.14) & F4 (Harness Portability: 6.13)
- **Phase 3: Topology & Ergonomics (UI/UX)** $ightarrow$ F5 (Header & Tab Density: 6.9+6.10) & F6 (Triage Views: 6.8+6.11)
- **Phase 4: Meta & Governance** $\rightarrow$ F7 (PCDA Governance: 6.1+6.2+6.15), F8 (Token Ledger: 6.19+6.3), & F9 (Bridge & Concurrency: 6.17+6.18)

### 2. Flight 1: Canonical State & Factory Handoff (Topics 6.16 & 6.7) — ✅ COMPLETED
- **Topic 6.16: Backend Persistence & State Management Architecture**: ✅ APPROVED (Option C: Hybrid Markdown Frontmatter + Client-Side LocalStorage + JSON Catalog). Bedrock state architecture finalized with zero heavy binary dependencies and clean Git diffs.
- **Topic 6.7: Context Clues Architecture & Factory Handshake Gate**: ✅ APPROVED (Option B: Structured Context Clues — dedicated context clues files seeded with canonical links to `DOCS_CATALOG.json` and `SYSTEM_MAP.md`).

### 3. Flight 2: Distribution Packaging & Key Ergonomics (Topics 6.12 & 6.6) — ✅ COMPLETED
- **Topic 6.12: The "All-in-One" N'kai Distribution Bundle**: ✅ APPROVED (Option B: Monolithic All-in-One Bundle — bundle canonical skills, templates, guardrail rules, and sidecar runner inside `nkai/bundle/` with a 1-command installer script `npx nkai init` / `./install_nkai_bundle.ps1`, featuring a universal pairing core with an optional `--audio` flag for audio/C++ specifics).
- **Topic 6.6: AGY Keyboard Shortcuts & Power Tools Integration**: ✅ APPROVED (Option B: Dynamic Shortcut Bar — subtly displaying core AGY shortcuts in the N'kai sidecar footer/tooltips: `Ctrl+K` Quick Prompt, `Ctrl+Shift+P` Command Palette, `Ctrl+Enter` Execute, and slash commands to reinforce developer muscle memory with zero token burn).

### 4. Flight 3: Universal AI Harness Portability (Topic 6.13) — ✅ COMPLETED
- **Topic 6.13: Universal AI Harness Portability**: ✅ APPROVED (Option B: Decoupled Interface Layer — zero-dependency standalone HTML `file:///` for IDE webviews and lightweight `npx nkai serve` / `tools/nkai_serve.py` for terminal CLIs).

### 5. Flight 4: Skill Taxonomy & Invocation Defaults (Topics 6.4 & 6.14) — ✅ COMPLETED
- **Topic 6.4: Agent Skills Organization: Micro-Skills vs. Macro-Suites**: ✅ APPROVED (Option C: Layered Hybrid — atomic micro-skills serve as the composable underlying engine while unified macro-commands like `/execute-task` and `qa` chain them deterministically with zero loss of granularity and minimal token burn).
- **Topic 6.14: N'kai-First Skill Default & --chat Fallback Flag**: ✅ APPROVED (Option B: N'kai-First with `--chat` fallback — all visual/interview skills like `/grill-me`, `/deep-research`, and `/postmortem` default to generating the rich N'kai sidecar canvas automatically, with `--chat` or "in chat" providing an instant, clean terminal/chat escape hatch).

### 6. Flight 5: Header Ergonomics & Spatial Density (Topics 6.9 & 6.10) — ✅ COMPLETED
- **Topic 6.9: Header Layout & Utility Ergonomics**: ✅ APPROVED (Option B: Two-Tier Header w/ Bridge Status LED & Utility Cluster — top bar hosts brand and persistent utility cluster with live bridge LED, roadmap/sessions drawer toggles, font zoom, sound haptics, and refresh; bottom bar hosts dedicated view tabs).
- **Topic 6.10: Tab Header Density & Spatial Conservation**: ✅ APPROVED (Option B: Hybrid Compact w/ Hover Tooltips — primary operational modes named like Workshop and Autopsy, while category tabs condense to `1 🗺️`, `2 📦`, `3 🚀` with instant full-text tooltips on hover, saving 40–60px of vertical header space in narrow IDE panels).

### 7. Flight 6: Interactive Triage Views (Topics 6.8 & 6.11) — ✅ COMPLETED
- **Topic 6.8: "Ongoing Autopsy": Live Triage Feed & Filterable Scoreboard**: ✅ APPROVED (Option B: Dedicated Autopsy Tab w/ Filter Chips — Slot 2 header item providing a continuous, filterable stream of all active/tabled/killed decisions with direct 1-click jumps to full cards, automatically morphing into the final Post-Mortem report upon milestone completion).
- **Topic 6.11: "Check the Mail" (Vault Inbox) Workflow & Sidecar Board Default**: ✅ APPROVED (Option B: N'kai Inbox Board Default — checking the Obsidian Vault `Inbox/` defaults to generating a dedicated interactive N'kai research board with 1 section per note and triage chips, with `--chat` flag preserved for pure terminal/chat dumps).

### 8. Phase 3 Capstone: N'kai Ergonomics (Topic 6.5) — ⏳ TABLED FOR v0.2.0
*Status: ⏳ TABLED FOR v0.2.0 (Phase 3: Surfaces & Viewports)*
- **Topic 6.5: N'kai Ergonomics: Asymmetric Chat Split & Responsive Layout**: ⏳ TABLED FOR v0.2.0 (Tabled alongside the Bidirectional Bridge `:4040` for the v0.2.0 UI Extension release, decoupling local `file:///` sidecar development from runtime bridge dependencies and eliminating premature IPC complexity).

### 9. Flight 7: PCDA Dialectic Governance & SOTA Benchmarks (Topics 6.1, 6.2 & 6.15) — ✅ COMPLETED
- **Topic 6.1: Post-Mortem Architecture: Continuous Live Triage vs Retrospective Batch**: ✅ APPROVED (Option B: Continuous In-Flight Triage — live post-mortem compilation during active research).
- **Topic 6.2: Pros & Cons Strategy: On-Demand SOTA vs Always-Grabbed**: ✅ APPROVED (Option B: On-Demand SOTA — punchy conversational chat turns by default, with 4-Lens PCDA matrices generated strictly on demand via sidecar actions).
- **Topic 6.15: Industry Workflow Precedents & Competitive Teardowns**: ✅ APPROVED (Option C: Hybrid File-URI + Micro-Bridge Model — zero-bundle `file:///` instant loading combined with modular bridge extension pattern).

### 10. Flight 8: Cumulative Allowance & Token Impact Ledger (Topics 6.3 & 6.19) — ✅ COMPLETED
- **Topic 6.3: Topic Organization Strategy: Deep-Linked Monolithic Board vs Fragmented Mini-Canvases**: ✅ APPROVED (Option B: Deep-Linked Monolithic Board — single, highly structured living research canvas with hash-anchor deep linking).
- **Topic 6.19: Cumulative Allowance & Token Impact Ledger in Post-Mortem**: ✅ APPROVED (Option C: Live Cumulative Cost Meter & Budget Ledger — interactive meter and ledger in the Post-Mortem & Workshop header showing total estimated token burn across all approved items, broken down by Tier 1 Pro High vs Tier 2 Flash High vs Tier 3 Minimal, with proactive budget warnings before code execution begins).

### 11. Flight 9: Execution Bridge & Concurrency (Topics 6.17 & 6.18) — ⏳ TABLED FOR v0.2.0
*Status: ⏳ TABLED FOR v0.2.0*
- **Topic 6.17: The Bidirectional Click-to-Agent Bridge (Zero-Copy Sidecar IPC)**: ⏳ TABLED FOR v0.2.0 (Decoupled `file:///` local sidecars from runtime localhost bridge dependencies; reserved for official Antigravity UI Extension release).
- **Topic 6.18: Parallel Subagent Delegation for Sidecar Actions**: ✅ APPROVED (Option B: Dedicated Parallel Flash Subagents — background `invoke_subagent(Model="flash")` executes asynchronously, writes results to the sidecar canvas or `.agents/sidecar/`, and alerts user when ready, zero main chat stall, minimal quota impact).

### 12. Bidirectional Micro-Bridge, Programmatic Clipboard & Modal Popups — ⏳ TABLED FOR v0.2.0
*Status: ⏳ TABLED (Post-v0.1.0 / v0.2.0 Plugin Release)*
- **Decision**: Tabled autonomous background loopback socket dispatch (`:4040`), programmatic clipboard manipulation (`navigator.clipboard.writeText` / `document.execCommand`), and fallback modal popups for v0.2.0.
- **Rationale**: In Chromium / Electron webviews without explicit `clipboard-write` sandbox permissions, `file:///` URLs have `origin: "null"`, causing silent permission denials. Fallback modal popups create jarring UI interruptions and still fail to copy reliably in restricted webviews.
- **v0.1.0 Standard**: Standardized on **uneditable (readonly) click-to-highlight text boxes** directly embedded on every card (`#ws-prompt-input` and `#audit-prompt-input-${tid}`). The user clicks or double-clicks once to select/highlight all text, then manually copies (`Ctrl+C`) and pastes into chat (`Ctrl+V`). Zero modal popups, zero sandbox permission issues, zero silent failures.
- **v0.2.0 Path**: Fully implement bidirectional agent messaging (`window.sidecar.agent.sendMessage()`) and native clipboard APIs when packaging N'kai as an official Antigravity UI Extension plugin.

### 13. N'kai v0.2.0 Milestone ("The UI Extension & Native Bridge Update") — ✅ COMPLETED
*Goal: Integrate N'kai directly into Google Antigravity as an official UI Extension with a true native bidirectional IPC bridge and clipboard engine.*
- **Official Antigravity UI Extension Packaging**:
  - Packaged N'kai as an official Antigravity UI Extension plugin (`%USERPROFILE%\.gemini\config\plugins\nkai\`) using the built-in Sidecar SDK (`sidecar_sdk`), supervised `main.mjs`, and native `plugin.json` + `sidecar.json`.
  - Registered extension panel in IDE auxiliary side pane (`SIDECAR_UI_ENTRYPOINT_AUX_PANE`) with glowing emerald vector glyph logo (`assets/logo.svg`).
- **Native IPC Bridge & Direct Agent Dispatch**:
  - True zero-copy native IPC agent messaging via `window.sidecar.agent.sendMessage()` in `core/sidecar.js` and global `window.dispatchNkaiAction()`.
  - Direct 1-click agent action dispatch with persistent HUD toast and mechanical audio chime haptics.
  - Native clipboard write and host IDE event integration, eliminating `file:///` sandbox restrictions and origin null CORS barriers.
- **Preserved Decoupled Offline Mode**:
  - Maintained 100% standalone offline `file:///` fallback and CLI runner parity for non-Antigravity environments.

### 14. N'kai v0.3.0 Milestone ("The Mobile Companion & Touch Ergonomics Update") — ⏳ QUEUED (TOP ACTIVE IMMEDIATE FOCUS)
*(Un-deferred per user burning desire & Pro roadmap stress-test; strictly bound by 1-day kill-gate).*
*Goal: Evolve N'kai from a desktop IDE sidecar into a multi-device, interactive AI pair-programming companion with mobile ergonomics and zero-config LAN pairing.*
- **Mobile Formatting & Touch Ergonomics**:
  - **Phone-Optimized Layouts**: Responsive breakpoints down to 320px–390px (iPhone & Android mobile viewports).
  - **Touch Target Ergonomics**: Minimum 44px tap targets for all interactive elements (`[ APPROVE ]`, `[ TABLE ]`, `[ KILL ]`, tab pills, filter chips) preventing accidental mis-taps.
  - **Bottom-Sheet & Thumb Navigation**: Sticky bottom action bar / drawer for one-handed mobile triage, scorecard review, and prompt copying while holding the phone.
  - **Touch-Scroll Optimization**: Smooth hardware-accelerated momentum scrolling (`-webkit-overflow-scrolling: touch`), overscroll boundary containment, and sticky top headers.
- **Mobile Testing & LAN Pairing**:
  - Zero-config mobile pairing via `python tools/nkai_serve.py --host 0.0.0.0` or `npx nkai serve --host 0.0.0.0` (with local network IP output and optional terminal QR code) to instantly load and test N'kai triage sessions on phone over local Wi-Fi from the studio or the couch!
- **Strict 1-Day Kill-Gate (Enforced by Pro Roadmap Stress-Test)**:
  - Vanilla CSS media queries, zero heavy PWA frameworks, simple Python/Node HTTP LAN server, and terminal QR code. If mobile styling exceeds 8 hours, freeze and advance to C++ DSP.

### 15. N'kai Sidecar Architecture Roadmap (The Rejiggered Runway — Act III)
> **🛫 Post-Mailbag Execution Order (set 2026-10-11):** finish mailbag triage first, then fly **Flight 0 (FOCUS) → 18 (UTF-8) → 16+17 (Sandbox) → 15 (Template) → 14 (Hub)**, with 19 (Response Drawer) and 7 (Intake v2) slotting in after 16+17.

- [x] **Flight 0: North Star Focus Doc + Status Footer (Mailbag `rj-09`, P0 — FIRST FLIGHT AFTER MAILBAG TRIAGE)** — ✅ COMPLETED: Authored living [`docs/FOCUS.md`](FOCUS.md) (current mission precedence: N'kai solid → TKS 0.5.0-dev → Toad PoC; negative architecture; gentle nudge protocol). Codified unified ambient status line (`Focus: N'kai hardening | TKS-0.4.0 · TT-0.1.0 · NK-0.2.0 · TR-Active`) and dynamic board link rule in `GEMINI.md`, `GLOSSARY.md`, `CHEAT_SHEET.md`, `DOCS_CATALOG.json`, and `SYSTEM_MAP.md`.
- [x] **Flight 1: The "Rambles" Ingestion Engine (Topic 2, P0)** — ✅ COMPLETED: Zero-dependency stream-of-consciousness ramble ingestor CLI (`tools/ramble_ingest.py`), Pydantic models (`RambleCard`, `RambleExtractionResult`, `to_mail_item()`), 3-bullet decision card decomposition, and `/ramble` micro-skill (`.agents/skills/ramble-ingest/SKILL.md`).
- [ ] **Flight 2: N'kai to High Reasoning Prompt Compiler (Topic 5, P1 — TOP ACTIVE PRIORITY)** — ⏳ QUEUED: Automated compilation of research cards, user notes, dialectical theses, and contrarian failure modes into a dense master system prompt payload for external LLM reasoning sessions.
- [ ] **Flight 3: N'kai Automode: 2-Pass Iterative Research Daemon & Multi-Drawer Dossier Cards (Topic 6, P1 — TOP PRIORITY)** — ⏳ QUEUED: Background research daemon automating iterative exploration paired with the multi-tab dossier card layout (Source Note, PCDA, Research, and Mobile Bottom Sheet); next execution target upon weekly quota reset.
- [ ] **Flight 4: General Research Assistant & Configurable Lens Profiles (Topic 9, P2)** — ⏳ QUEUED: Declarative lens profiles (`tools/lens_profiles.json`) with auto-detection tags (`#studio`, `#hardware`, `#ttrpg`, `#writing`) and zero product forking.
- [x] **Flight 5: Spun-Up Flash Low Agent for N'kai-to-Chat Button Routing (Topic 16, P1)** — 🪦 GRAVEYARD: Rejected in favor of Antigravity's native UI Extension IPC (`window.sidecar.agent.sendMessage()`), avoiding unnecessary LLM inference latency and token overhead for simple string routing.
- [x] **Flight 7: Mailbag Intake & Triage v2 — Overflow, Quick Notes, Tabled Folder, Attention Tags & Position Splice (Topic 18 + Mailbag `rj-07` & `rj-17`, P1)** — ✅ COMPLETED 2026-10-11: Implemented ramble overflow queue (top 3 active, remaining to `overflow.json` with `split_group`, `.badge-overflow` chip, and Rail 2 `Overflow` filter pill), filename-only quick notes (0-byte/whitespace notes take title from stem, tagged `quick-note`), dedicated `TheKlangVault/Inbox/Tabled/` agent folder with `🗂 Tabled by agent` chip and tabling protocol in `GEMINI.md`, attention marker extraction (`!IMPORTANT`, `!BLOATED`, `!INCOMPLETE`, `!SPIKE`) with interactive session launcher buttons (`[ 🔬 Grill-Lab ]`, `[ 🧠 Deep Research ]`, `[ 🔨 Draft Plan ]`), and `/mail-add` position splice CLI (`tools/mail_add.py`) with contiguous rank renumbering and inbox note generation. Verified with 16 unit tests, 0 Web QA errors, 100% UTF-8 compliance, and design spec `docs/specs/nkai_mailbag_intake_and_triage_v2.md`.
- [ ] **Flight 9: Dirtywave M8 Telemetry Capture & Bridge (Topic 10, P2)** — ⏳ QUEUED: Bidirectional telemetry and pattern synchronization bridge for Dirtywave M8 tracker hardware.
- [ ] **Flight 10: Sub-Module & Component Presets Architecture (Topic 14, P1)** — ⏳ QUEUED: Sub-component preset schemas and serialization across synth modules.
- [ ] **Flight 11: 4-Macro Mega Modules & Snap Heap Popovers (Topic 12, P1)** — ⏳ QUEUED: 4-encoder mega module abstractions with contextual popover settings callouts.
- [ ] **Flight 12: Sonicware LIVEN CyDrums Super-Engine (Topic 15, P1)** — ⏳ QUEUED: Drum voice modeling and synthesis engine inspired by CyDrums.
- [x] **Flight 13: Consolidated Filter-Counter Pills & Spatial Header Optimization (Topic 19, P2)** — ✅ COMPLETED: Merged category/status filter buttons with count badges into unified interactive pill chips (`[ 📋 All Mail (68) ]`, `[ ⚪ Untriaged (61) ]`, `[ 🚨 P0 (9) ]`, `[ 📥 Backlog (3) ]`, `[ ✅ Completed (4) ]`), eliminated redundant `.scoreboard-banner` to reclaim ~60px of vertical height, established Two-Tier Consolidated Rails (Rail 1 Platform Pillars + Rail 2 Status Filters with `All` first and `Untriaged` default), and enabled 1-click card pillar badge filtering across `tools/check_mail.py` and N'kai UI extension.
- [x] **Flight 14: Multi-Board Hub & Session Manager (Topic 20 + "archived boards" + session-manager wish → Mailbag `rj-05`, P0)** — ✅ COMPLETED: Implemented `#btn-board-switcher` in Top-Tier Header and `#board-switcher-modal` with real-time text search and Active vs Archived grouping, codified Pydantic `BoardRegistry` and `BoardMetadata` models, established canonical `.agents/pipeline/boards/registry.json`, injected strict anti-cache HTTP response headers (`Cache-Control: no-store, no-cache, must-revalidate, max-age=0`, `Pragma: no-cache`, `Expires: 0`) and `?board=<id>` routing in `sidecars/panel/main.mjs`, authored `tools/board_registry.py` CLI (`list`, `register`, `rescan`), added automated test suite (`tools/tests/test_board_hub.py`), and authored design spec `docs/specs/nkai_multi_board_hub_and_session_manager.md`.
- [x] **Flight 15: Unified Master Template & Schema-Driven Boards (Topic 21 + "data-driven N'kai" note → Mailbag `rj-04`, P0)** — ✅ COMPLETED: Consolidated canonical standalone shell `c:\Dev\nkai\core\master_template.html` (keeping Two-Tier header, live allowance meter, audio haptics, optional hero stage, and in-canvas 4-lens accordions while excising dead loopback bridges), codified declarative `BoardSchema` in `tools/models.py` and `c:\Dev\nkai\core\schema.json`, built zero-HTML-edit generator `tools/board_renderer.py`, refactored `tools/check_mail.py` eliminating ~1,500 lines of monolithic HTML string, expanded automated test suite (5/5 PASS), and authored design spec `docs/specs/nkai_unified_master_template_and_schema.md`.
- [x] **Flight 16+17: QA Sandbox Board & Sandbox/Production Mode (Topics 22 & 23 merged → Mailbag `rj-03`, P0)** — ✅ COMPLETED: Implemented synthetic Lovecraftian test fixture (`assets/fixtures/sandbox_mail_queue.json`, 7 cards), `--sandbox` runtime mode in `tools/check_mail.py` rendering `.agents/sidecar/sandbox_mail_inbox.html` with persistent amber banner and `[SBX]` title, echo-only mock action dispatch with `[QA_TEST_MOCK]` prefix, deterministic DOM IDs (`btn-<action>-<id>`, `pill-<rail>-<slug>`), storage isolation (`nkai_sbx_mail_triage_actions`), automated verification test suite (`tools/tests/test_sandbox_board.py`), and design spec `docs/specs/nkai_sandbox_and_multistage_environments.md`.
- [x] **Flight 18: Universal UTF-8 Invariant (Mailbag `rj-02`, P0 — QUICK WIN)** — ✅ COMPLETED: Built automated linter and enforcer CLI [`tools/verify_utf8.py`](../tools/verify_utf8.py). Hardened 100% of Python tools (42 files) across `TheKlangSuite`, `nkai`, and `bundle/` with `sys.stdout/stderr.reconfigure(encoding='utf-8')` and explicit `encoding='utf-8'` on all file I/O (safe `errors='replace'`, zero `latin-1`). Added `$env:PYTHONUTF8 = '1'` to PowerShell harnesses. Codified universal invariant in `GEMINI.md`, `GLOSSARY.md`, `CHEAT_SHEET.md`, `DOCS_CATALOG.json`, and `SYSTEM_MAP.md`.
- [x] **Flight 19: Per-Card Response Drawer (Mailbag `rj-08` split, P1 — SMALL, CAN GO EARLY)** — ✅ COMPLETED: Implemented `agent_reply`, `agent_reply_at`, and `has_reply` in `MailItem` and `RambleCard` (`tools/models.py`), collapsible glassmorphic drawer markup and styling in `master_template.html` and `sidecar.css`, `toggleResponseDrawer` with procedural Web Audio detent haptics in `sidecar.js`, `--reply-card` CLI in `tools/check_mail.py` (<50ms board regen), comprehensive Lovecraftian QA fixture with phoney PCDA dialectic matrices, research briefs, autopsies, and backlog notes, full test suite `tools/tests/test_response_drawer.py`, and design spec `docs/specs/nkai_per_card_response_drawer.md`.
- [x] **Spike: Fullscreen N'kai Sidecar (Mailbag `rj-08` split)** — ✅ RESOLVED 2026-10-11: Antigravity UI extensions document only `SIDECAR_UI_ENTRYPOINT_AUX_PANE` (side pane); no fullscreen / editor-tab / pop-out entrypoint. Options: drag-widen the side pane (keeps native one-click send), or `python tools/nkai_serve.py` in a browser tab for fullscreen/second monitor (buttons fall back to clipboard copy — `window.sidecar` only exists in the pane). Revisit if Antigravity adds a new entrypoint.
- HUD for a second monitor (Mailbag `rj-08` split) — 🗂 TABLED until a second monitor exists; folded into Mailbag `rj-24` (M8 Telemetry Bridge & Second-Monitor Sidecar).
- [ ] **Flight 20: Vault Graph & Naming Hygiene (Mailbag `rj-10c` PCDA, P2, Tier 3)** — ✅ APPROVED 2026-10-11. One-way vault mirror rule already codified in `GEMINI.md` / GLOSSARY §15 / CHEAT_SHEET.
  1. **Sync-time link rewrite (c1)** in `tools/sync_obsidian_vault.ps1`: convert `file:///` links that target mirrored files into vault-relative links (~87 dead-end links join the graph/backlinks); leave links to non-mirrored files (`.cpp`, assets) untouched; strip `#L123-L145` line anchors, keep heading anchors; decode `%20` defensively. Header comment restating the one-way rule.
  2. **Underscore naming invariant**: all agent-created files use `snake_case`/underscores, never spaces. One-time migration of the 18 TKS + ~4 non-Inbox vault files (and vault folders like `The Klang Suite/` via the sync mapping) with link rewrites. Inbox notes keep phone-typed names on capture and are renamed to underscores by `check_mail.py` **on archive**; title derivation converts `_` back to spaces so title-matching (`rejigger.json` sources) keeps working. **Exceptions:** shipped product names (`The Klang Farmer.vst3`, `The Klang Planter.vst3`, `current_build/` artifacts).
  3. **`related:` captured at birth (c2-lite)**: when an agent creates a spec / backlog entry / research doc from a mailbag card, it writes `related:` frontmatter (source note, card id, spec, RFC) while the links are already in context — near-zero marginal cost. No retroactive LLM footer sweeps; Obsidian backlinks + local graph cover the rest.
  4. **Resolve `docs/inbox/`** (12 copies of vault Inbox notes inside TKS): conflicts with the one-way rule — confirm with user, then delete and drop the `docs/inbox` fallback from `check_mail.py`.
- [ ] **Flight 21: Checkpoint Resumption Engine & Opus Deep Audit (Mailbag `rj-11`, P1, Tier 1 Targeted Opus Swap)** — ✅ APPROVED 2026-10-11: Build `tools/audit_checkpoint.py` (per-repo progress persistence in `audit_checkpoint.json`), schedule first Deep Opus Audit across TKS, TT, NK after N'kai P0s land. Results surface on an N'kai review board (depends on Flight 14 Multi-Board Hub). Cadence: light audit at every patch tag; deep audit once per minor release (`v0.4.0`, `v0.5.0`) or before weekly quota reset. Codified in `GEMINI.md` / GLOSSARY §16 / CHEAT_SHEET.
- [x] **Flight 22: Six-Way Devil's Advocate Benchmark (Mailbag `rj-22`, P0)** — ✅ COMPLETED 2026-10-11: Completed exhaustive 6-report DA benchmark (Flash-A/B, Pro-A/B, Opus-A/B) evaluating the real-world DAW audio backend trap. Codified the **Method B Adversarial Clause** (`Where I Disagree With The Prompt`), proving +1.44 quality leap on Flash and +1.06 on Pro. Sealed corpus in `TheKlangResearch/agentic/rj22_da_benchmark/` and published canonical report `TheKlangResearch/agentic/da_methodology_six_way_benchmark_2026-10.md` with Kendall's tau-b of +0.8667.
- [ ] **Card `rj-26`: N'kai Minimal: Headless Zero-Dependency CLI & Chat-Only Distribution (P1, Tier 2)** — ⏳ QUEUED: Package N'kai core skills, guardrails, stream-of-consciousness mailbag, PCDA dialectic, and transaction ledgers for pure CLI / chat pair-programming without requiring the Electron sidecar webview panel or Node.js runtime.
- [ ] **Card `rj-27`: Selective Dialectic Gate: On-Demand Quick PCDA via Card Launcher & Attention Tags (P2, Tier 2)** — ⏳ QUEUED: Killed blanket batch auto-PCDA on all inbox items (avoids token churn and artificial dialectics on clear-cut notes). Instead, implement selective on-demand quick PCDA triggered via board card chips or `!DILEMMA` attention tags.
- [x] **Card `rj-28` / `rj-40`: Act III Deep Docs Update: Flash High Execution Engine with Atomic Git Quarantine & Snapshot Rollback (P1, Tier 2)** — 🔨 IN PROGRESS: Comprehensive documentation update across all repositories using sustainable Gemini 3.8 Flash High, isolated on a dedicated git quarantine branch (`docs/deep-update-20261011`) with pre-flight snapshot rollback.
- [ ] **Card `rj-29`: PicoTracker (LGPT) Architectural Teardown (P2, Tier 2)** — ⏳ QUEUED: Clone and analyze the picotracker (LittleGPTracker fork) codebase into `Research/external/` as architectural reference for ToadTracker pattern memory layouts, gamepad debouncing, and embedded playback loops.
- [ ] **Card `rj-30`: Batch Vault Note YAML Frontmatter Enrichment (P2, Tier 3)** — ⏳ QUEUED: Automate the batch injection of standardized YAML frontmatter into ~70 archived human notes in `TheKlangVault/Inbox/Archive/` using a dedicated Flash subagent script `tools/enrich_archive_notes.py`.

### 16. Flight 2: N'kai to High Reasoning Prompt Compiler (P1 — TOP ACTIVE PRIORITY)
*Source Note:* [`TheKlangVault/Inbox/Archive/Nkai to High Reasoning Prompt.md`](file:///C:/Dev/TheKlangVault/Inbox/Archive/Nkai%20to%20High%20Reasoning%20Prompt.md)  
*Status: ⏳ QUEUED (Top Active Priority for Act III Runway)*  
*Priority: P1 — TOP ACTIVE PRIORITY*  
*Target Pillar: N'kai Sidecar Architecture Roadmap*  
*Goal: Automate the compilation of raw research cards, user notes, dialectical theses, and adversarial contrarian friction metrics into a dense, high-signal master system prompt payload for high-reasoning flagship models (Gemini 3.1 Pro High, Claude 3.7 Sonnet Thinking).*

#### 1. Core Value Proposition & The "Master Architect" Principle
- **Data Miner & Structural Filter**: Passing unstructured rambles or fragmented chat histories directly to expensive high-reasoning models wastes valuable context window capacity and token allowance on basic noise reduction. The local N'kai harness and Tier 2 Flash orchestrator pre-mine data and apply structural filtering (Thesis + Antithesis) locally at near-zero token cost.
- **Zero-Waste Synthesis**: When the high-reasoning model receives the compiled dossier, it bypasses orientation and baseline discovery entirely. Its deep-thinking reasoning budget is 100% focused on resolving architectural tensions, reconciling trade-offs, and generating definitive software blueprints adhering to the "Sleepy Architecture" invariant (achieving MAXIMUM feature leverage with MINIMAL coding execution overhead).

#### 2. The 80/20 Dual-Harness Architecture (Synthesis)
- **Mode A: In-IDE Native Subagent Dispatch (Context Shield)**:
  - Compiles the structured dialectical payload directly into an autonomous `invoke_subagent(Model="pro")` dispatch charter ([`.agents/pipeline/communique/subagent_dispatch.md`](file:///c:/Dev/TheKlangSuite/.agents/pipeline/communique/subagent_dispatch.md)).
  - Keeps the main chat session permanently lean on Tier 2 Flash 3.8 High (protecting user allowance and transcript hygiene) while spinning up on-demand Pro thinking subagents for heavy synthesis sprints.
- **Mode B: External Web & Clipboard Exporter (CLI Tool `tools/compile_prompt.py`)**:
  - Standalone, zero-dependency Python CLI: `python tools/compile_prompt.py --topic <id> --clipboard` (with `--out <file>`, `--chat`, `--vault` flags).
  - Automatically aggregates reference nodes from `TheKlangResearch`, active notes from `TheKlangVault`, and contrarian failure modes.
  - Injects formatted tags: `[THESIS: CORE INTENT]`, `[ANTITHESIS: ADVERSARIAL STRESS-TEST]`, `[CONSTRAINTS & TARGETS]`, and `[SYNTHESIS MANDATE]`.
  - Populates the OS clipboard via cross-platform bindings (Win32 `ctypes`, macOS `pbcopy`, Linux `xclip`) for instantaneous 1-click pasting into external web interfaces (e.g. Claude Web, ChatGPT Pro, Gemini Web).

#### 3. Structured Data Models (`tools/models.py`)
- **`PromptHandoffPayload`**: Pydantic v2 data model capturing:
  - `thread_id`: Unique session identifier.
  - `reconciliation_protocol`: Dialectical reconciliation schema (e.g., `Standard`, `Strict Audio DSP`, `UI/UX Ergonomics`).
  - `thesis`: Core intent and user goals (`ThesisBlock`).
  - `antithesis`: Reviewer persona, friction matrix, identified structural vulnerabilities, and orthogonal vectors (`AntithesisBlock`).
  - `targets`: Materialization targets, blueprint context, effort/impact metrics, and mayor clearance status (`SynthesisTargetConfig`).
  - `prompt_mandate`: Explicit instruction set directing the reasoning model to synthesize the final blueprint.
- **Export Adapters**: Methods `to_markdown()`, `to_clipboard()`, and `to_vault_file()`.

#### 4. Fallback Boundaries (Zero-Bridge Reliability)
- **Pure File-Based Sync (Obsidian Vault Drop)**: If OS clipboard APIs are restricted in sandboxed environments, writes compiled payload directly to `TheKlangVault/TsathogguaHandoffs/thread_<id>.md` for manual copying or archival review.
- **CLI Shell Redirection**: Supports piping directly to terminal utilities: `python tools/compile_prompt.py --topic 5 | clip` (Windows) or `pbcopy` (macOS).

#### 5. Sidecar & Webview Integration
- **1-Click Action Chip**: Integrates `[ 🧠 Compile Reasoning Prompt ]` chip into `morning_mail_inbox.html` and N'kai sidecars.
- **Readonly Prompt Composer**: Pre-renders the compiled payload into a selectable, highlighted container with dedicated `[ 📋 Copy ]` button, eliminating webview clipboard sandbox permission traps.

### 17. Flight 3: N'kai Automode: 2-Pass Iterative Research Daemon & Multi-Drawer Dossier Cards (P1 — TOP PRIORITY)
*Source Note:* [`TheKlangVault/Inbox/Archive/Nkai Automode.md`](file:///C:/Dev/TheKlangVault/Inbox/Archive/Nkai%20Automode.md) ("Should there be a skill to auto-research all the items twice using the idea about loops and going through twice and not more?")  
*Status: ⏳ QUEUED (Top Immediate Priority for Execution upon Weekly Quota Reset)*  
*Priority: P1 — TOP PRIORITY*  
*Target Pillar: N'kai Sidecar Architecture Roadmap*  
*Related Architecture:* [`audit-reorder` Skill (2-Pass Convergence Gate)](file:///c:/Dev/TheKlangSuite/.agents/skills/audit-reorder/SKILL.md)  
*Goal: Automate 2-pass iterative research sweeps across queued triage items without stalling the chat session, while strictly bounding loops, preventing hallucination compounding or quota incinerations, and integrating a unified multi-tab dossier card layout for both desktop and mobile.*

#### 1. The "Rule of 2" Bounded Convergence Architecture
- **Strict Finite Bound (`max_iterations = 2`)**: Multi-agent reflection research demonstrates that gains steeply plateau after iteration 2. Pass 3+ yields negative returns (semantic churn, hallucination compounding, and token waste). Automode strictly enforces the 2-Pass law:
  - **Pass 1 (Parallel Divergent Sweep — Tier 2 Flash)**: Spawns parallel `explore` (SOTA precedents & affirmative thesis) and `contrarian` (failure modes & architectural stress-testing) subagents with a 3-minute hard watchdog.
  - **Pass 2 (Convergent Synthesis — Tier 2 Flash)**: Spawns a synthesis subagent to reconcile Thesis and Antithesis into standard 3-bullet decision cards and complete 4-lens PCDA matrices (Real-World Ergonomics, Audio Software, Wider SOTA, Best Practices).
- **Pro Model Prohibition (Zero Pro Burn)**: Automode runs 100% on Tier 2 Flash 3.8 High. Tier 1 Pro models are strictly prohibited, reserving critical high-reasoning quotas exclusively for Flight 2 Prompt Compiler synthesis.

#### 2. The Human Triage Gate & Bounded Execution Invariants
- **No Blind Vault Sweeps**: Automode NEVER autonomously ingests or researches raw notes in [`TheKlangVault/Inbox/`](file:///C:/Dev/TheKlangVault/Inbox). It executes strictly on items that the user has explicitly triaged with `[ 🔬 Research ]` in [`morning_mail_inbox.html`](file:///c:/Dev/TheKlangSuite/.agents/sidecar/morning_mail_inbox.html) or marked `status: "research"` in `.agents/pipeline/mailbag/mail_queue.json`.
- **Strict Batch Cap (`--limit 3`)**: Default execution batch size is strictly capped at 3 items (configurable via `--limit N`). Operates as a finite, one-shot CLI job (`tools/research_daemon.py` or `/automode --limit 3`), NEVER as an infinite continuous loop daemon (`while True:`).
- **Isolated Staging Area**: Researched outputs write to `.agents/pipeline/mailbag/researched/`, keeping active Git working trees and tracked C++ codebase completely clean. Emits an ambient completion chime upon batch finish.

#### 3. In-Card Dossier Integration (The "Hero Front + 3-Tab Drawer" Standard)
- **Front Panel Hero (Always Visible)**: The 3-bullet synthesis (🎯 Core Intent, 🛠️ Proposed Mechanism, 🚦 Action Route) remains permanently visible on the front face of each card for instantaneous 15-second scanning. Never hidden behind an accordion.
- **Single Unified Dossier Drawer (`.mail-dossier-drawer`)**: Replaces multi-accordion vertical stacks with ONE clean drawer toggle: `[ 📂 View Full Dossier (3) ]`.
- **3-Segmented Tab Strip Inside Drawer**:
  - `[ 📄 Source Note ]`: Renders original raw/rendered Markdown note from the Obsidian vault.
  - `[ ⚖️ PCDA Dialectic ]`: Renders the 4-part Dialectic Triad (Pros, Cons, Devil's Advocate, 80/20 Resolution) with 4-lens metrics.
  - `[ 🔬 Research Dossier ]`: Renders curated technical citations, benchmarks, and upstream architectural precedents.
- **On-Demand Progressive Population**: Unresearched tabs display a clean empty state with a 1-click trigger (`[ ⚡ Generate PCDA in Sidecar ]`). When Automode completes, the dossier tabs are populated in-place in the sidecar with zero chat transcript bloat.
- **Mobile-Ready Architecture (v0.3.0 Bottom-Sheet Bridge)**: On viewports $\le 640\text{px}$, toggling the dossier slides up an 85vh swipeable bottom sheet with sticky segmented tabs, completely eliminating nested accordion scroll traps on touch devices.

### 18. Flight 4: General Research Assistant & Configurable Lens Profiles (P2)
*Source Note:* [`TheKlangVault/Inbox/Archive/Nkai Use Cases.md`](file:///C:/Dev/TheKlangVault/Inbox/Archive/Nkai%20Use%20Cases.md) ("Right now it’s mostly for dev. But, what about a general research assistant use case? What if I want to research a TTRPG project, studio rewiring, Eurorack layout, or write lyrics?")  
*Status: ⏳ QUEUED*  
*Priority: P2*  
*Target Pillar: N'kai Sidecar Architecture Roadmap*  
*Goal: Expand N'kai into a versatile research companion beyond C++ audio plugin development by decoupling the 4-Lens evaluation matrix into declarative JSON profiles, enabling specialized research workflows (studio hardware, Eurorack, game design, writing) with zero product forking and zero yak shaving.*

#### 1. Core Architecture & Anti-Yak-Shaving Invariant
- **Single Unified Codebase (Never Fork the Product)**: N'kai remains ONE unified sidecar tool. Rejects building separate spin-off apps or multi-tenant plugin engines ("The Second-System Effect") to avoid derailment from core audio products (*The Klang Farmer*, *ToadTracker*).
- **Invariant Dialectic Core**: The underlying Hegelian Dialectic Triad (Thesis $\to$ Antithesis $\to$ Synthesis) and the 3-Bullet Decision Card (🎯 Core Intent, 🛠️ Proposed Mechanism, 🚦 Action Route) remain permanently invariant across all domains. Only the 4 domain-specific evaluation lenses adapt.

#### 2. Declarative Lens Dictionary (`tools/lens_profiles.json`)
- Move hardcoded prompt strings out of Python scripts into a clean, lightweight JSON schema:
  ```json
  {
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "version": "1.0.0",
    "default_profile": "audio_dev",
    "profiles": {
      "audio_dev": {
        "display_name": "Audio Plugin & DSP Engineering",
        "tags": ["audio", "dsp", "vst", "synth", "juce", "tks"],
        "lenses": [
          { "id": "ergonomics", "name": "Real-World / Daily Ergonomics", "prompt": "Practical day-to-day developer friction, velocity, cognitive load, and maintenance realities." },
          { "id": "industry", "name": "Audio Software Industry", "prompt": "How top synth/plugin developers (Vital, Kilohearts, FabFilter, Elektron, Bitwig, Reaper) solve it." },
          { "id": "sota", "name": "Wider Software Industry / SOTA", "prompt": "Modern systems, agent harnesses, and web architectures (Anthropic, OpenAI, GitHub, Linux)." },
          { "id": "best_practices", "name": "Best Practices & Sustainability", "prompt": "Audio thread safety, zero-allocations, token economics, and avoiding premature optimization." }
        ]
      },
      "studio_hardware": {
        "display_name": "Studio, Hardware & Eurorack Ergonomics",
        "tags": ["studio", "hardware", "eurorack", "patchbay", "wiring", "m8", "tbd-16"],
        "lenses": [
          { "id": "signal_flow", "name": "Signal Flow, Latency & Impedance", "prompt": "Acoustic signal path, conversion stages, normalled patchbay routing, and monitoring latency." },
          { "id": "physical_ergonomics", "name": "Physical Ergonomics & Patchability", "prompt": "Reachable knob/slider layouts, cable obstruction, ModularGrid 3U/1U spacing, and tactile flow." },
          { "id": "power_noise", "name": "Power Draw, Heat & Noise Floor", "prompt": "mA load on +12V/-12V rails, ground loops, analog noise bleed, and thermal dissipation." },
          { "id": "bom_budget", "name": "Modularity, Sourcing & Budget", "prompt": "Bill of materials, DIY feasibility, component availability, and 80/20 cost efficiency." }
        ]
      },
      "creative_general": {
        "display_name": "General Research, TTRPG & Creative Writing",
        "tags": ["ttrpg", "writing", "lyrics", "gamedesign", "general"],
        "lenses": [
          { "id": "intent_narrative", "name": "Creative Intent & Narrative Resonance", "prompt": "Emotional core, thematic consistency, and poetic or fictional impact." },
          { "id": "user_experience", "name": "User / Player Experience", "prompt": "Player agency, DM cognitive burden, lyric vocal rhythm, and intuitive clarity." },
          { "id": "feasibility", "name": "Structural / Technical Feasibility", "prompt": "Ruleset coherence, systemic math balance, or practical execution constraints." },
          { "id": "elegance", "name": "Elegance & The 80/20 Rule", "prompt": "Maximizing evocative depth while eliminating unnecessary mechanical crunch or clunky prose." }
        ]
      }
    }
  }
  ```

#### 3. Zero-Click Auto-Detection via Note Tags
- **Frictionless Intake**: When capturing quick voice memos or rambles into Obsidian, adding a tag automatically maps the card to its optimal lens profile:
  - Tags containing `#studio`, `#hardware`, `#eurorack`, `#wiring` $\to$ Activates `studio_hardware`.
  - Tags containing `#ttrpg`, `#writing`, `#lyrics`, `#gamedesign` $\to$ Activates `creative_general`.
  - Tags containing `#tks`, `#dsp`, `#audio`, or untagged notes $\to$ Defaults to `audio_dev`.
- **Sidecar & CLI Overrides**:
  - In `morning_mail_inbox.html`: A subtle dropdown on the PCDA tab allows instantaneous manual switching between profiles.
  - In CLI: `python tools/check_mail.py --profile studio_hardware` or `tools/pcda_format.py --profile creative_general`.

---

## 🔮 Spin-Off Products & Explorations

### 1. The Klang Mill (Standalone VST) — Industrial 1x6 Multi-FX Pedalboard Rack
*Detailed Plan: [`docs/the_klang_mill_plan.md`](specs/spinoffs/the_klang_mill.md)*
Create a standalone multi-effects VST3 plugin styled after vintage studio rackmounts and boutique pedalboards (e.g., Soundtoys Effect Rack):
- **1x6 Horizontal Chassis**: Input/Slop $\to$ 4 Serial Multi-FX Pedal Slots (26 algorithms) $\to$ Master Limiter & Output.
- **Immediate & Tactile**: Zero routing matrices or drag-and-drop clutter; dedicated stomp bypasses per slot.
- **Global Slop**: Injects organic, non-linear analog drift across all 4 pedals for instant vintage character.
- **Codebase Integration**: Built as a sibling build target (`TheKlangMill_VST3`) inheriting directly from `KlangCoreProcessor` and `KlangCoreEditor`.

### 2. The Klang Boilerplate — Modern C++20 / JUCE 9 FOSS Plugin Starter Template
*Detailed Plan: [`docs/plugin_starter_template_repo_plan.md`](specs/plugin_starter_template.md)*  
Extract a clean, standalone GitHub Template Repository incorporating all lessons learned from *The Klang Farmer* to accelerate new audio plugin development:
- **Zero-Allocation DSP Core**: Strict audio-thread invariants, vectorized `TbdAudio::FastMath`, and lock-free SPSC FIFO queues.
- **Data-Driven JSON Architecture**: APVTS parameter registration, quick-snap intervals, and declarative card/page layouts authored 100% in JSON (`assets/controls/`, `assets/layouts/`).
- **Automated Headless Reflection Testing (`gui_tests`)**: Sweeps 100% of APVTS parameters and JSON assets headlessly in CI without audio hardware or display servers.
- **Boutique UI & Theme System**: JSON theme palettes (*Cyberpunk Neon*, *Cykranosh*, *Dracula*, *Monokai Pro*), vector LookAndFeel, high-DPI scaling, and JUCE 9.0.3 timer hygiene.
- **Developer Onboarding CLI (`init_plugin.py`)**: One-command wizard to rename targets, bundle IDs, C++ namespaces, and parameter prefixes in seconds.

### 3. ToadTracker (`TT` v0.1.0) PoC Pivot & Architecture Roadmap
*Detailed Specifications: [`C:\Dev\ToadTracker\TRACKER_SPEC.md`](file:///c:/Dev/ToadTracker/TRACKER_SPEC.md) & [`docs/specs/toadtracker_gamepad_latency_and_renoise_spec.md`](file:///c:/Dev/Research/hardware/2026-10-11_toadtracker_gamepad_latency_and_renoise_spec.md)*  
*Priority: P1 — Active Horizon Target (Stage 5 PoC Pivot immediately following TKS v0.5.0)*
*Goal: Stand up the core headless tracker engine (`ToadCore`), 500Hz SteamOS / Linux gamepad input pipeline, and Renoise `.xrns` project export referencing TKS plugins.*
- **Strict GPLv3 "FOSS Forever" Invariant**:
  - Full copyleft reciprocity across all engine code (`core/`, `hal/`, `apps/`). Zero closed-source or GPL-incompatible dependencies.
  - Universal SPDX headers: `// SPDX-License-Identifier: GPL-3.0-or-later` and `// Copyright (C) 2026 Cody Gratner & ToadTracker Contributors`.
  - Mandatory inline provenance docblocks for any algorithms adapted from open-source references (PicoTracker/LGPT, Renoise specs, libremidi).
  - Third-party ledger maintained at `ToadTracker/docs/THIRD_PARTY_LICENSES.md`.
  - Strict zero-trademark policy for tracker commands and FX.
- **500Hz SteamOS / Linux Gamepad Input Pipeline**:
  - Direct low-latency SDL3 / Linux `evdev` event polling loop running on a dedicated high-priority input thread.
  - Zero-jitter SPSC lock-free ring buffer feeding the tracker state machine, timestamping events against audio buffer boundaries.
  - Ergonomic mapping centered on D-pad + ABXY + L1/R1, with L4/R4 back paddles for octave/track shifts.
- **Renoise `.xrns` Project File Export**:
  - Clean C++ zip compression and streaming XML serialization for `Song.xml`.
  - Exports tracks, pattern matrices, and `<PluginDevice>` blocks targeting *The Klang Farmer* and *The Klang Planter* VST3 plugins with v0.5.0 DSP sound presets.
- **Enforced Kill-Gate (Pro Roadmap Stress-Test)**:
  - Headless CLI PoC first: SDL3 reads Gamepad ➔ updates tracker state ➔ writes `.xrns` ➔ exits.
  - Zero premature UI rendering until input timing and project serialization are verified 100% stable.

### 4. The Klang R1 (TKR-1) — 7-Voice Rhythm Synthesizer (Electribe ER-1 Tribute)
*Detailed Plan: [`docs/specs/spinoffs/the_klang_r1.md`](specs/spinoffs/the_klang_r1.md)*  
*Goal: Provide a stripped-down, tactile, zero-tab 7-voice drum synthesizer inspired by the iconic Korg Electribe ER-1 with white-key octave-invariant triggering and DAW multi-out routing.*
- **7-Voice Hybrid Architecture**:
  - **Voices 1–4 (Pure Synth)**: Kicks, sub-bass, snares, toms, and resonant FM zaps.
  - **Voices 5–7 (Percussion & Metallic)**: Closed Hat, Open Hat (auto-choked by Voice 5), and Cymbal / Crash.
- **Octave-Invariant White Key Triggering**:
  - White keys in any octave map to Voices 1–7 (`C` = Voice 1 &rarr; `B` = Voice 7).
  - Pitches are fixed to front-panel knobs for authentic drum-machine operation (zero pitch tracking).
  - Black keys (`C#`, `D#`, `F#`, `G#`, `A#`) are unassigned for foolproof live finger drumming anywhere on the keybed.
- **DAW Multi-Out Bus Architecture**:
  - 8 Stereo output pairs: Master Mix + 7 Individual Voice stems (`Voice 1` through `Voice 7`).
  - Auto-mute routing: Routing a voice to an aux track removes it from Master Mix (with parallel toggle).
- **All-in-One Console UI (Zero Tabs)**:
  - 7 vertical mixer-style voice strips with `Pitch`, `Decay`, `Mod Type`, `Mod Speed`, `Mod Depth`, `Pan`, `Level`, and `[DELAY SEND]`.
  - Master section featuring classic ER-1 **Low Boost** sub-punch knob, host-synced **Tempo Delay**, and **Ring Mod** cross-modulation (`Voice 1 × Voice 2`).
- **"Klang-Brain" Generative Sequencing Engine**:
  - Incorporates 5 selectable alternative generative modes adapted into our 4-Control Mega Module paradigm:
    1. *Fugue Counterpoint*: 4 playheads reading 1 pattern at varied clock dividers & directions.
    2. *Matriceal Polymeter*: Decoupled parameter loop lengths (Triggers, Velocity, Octave, Gate).
    3. *Stage Pulses & Accumulators*: Metropolix-style variable pulse repeats + M8 sub-tick micro-tables.
    4. *Turing Machine (LFSR)*: Shift-register generative loops with controlled bit-flip mutation probability.
    5. *Axon Neural Network*: Audio Damage-style 7-neuron leaky integrate-and-fire network mapped 1:1 to TKR-1's 7 voices, generating organic, biologically interdependent polyrhythmic grooves.
  - **Dual Playback Modes (DSI Pro 2 Heritage)**:
    - *Continuous Mode*: Advances automatically with the DAW host playhead and tempo clock.
    - *Triggered / Key-Advance Mode*: Sequencer lanes hold stationary until an incoming MIDI note or gate trigger is received, advancing by 1 step per keypress (emulating the classic Dave Smith Instruments Pro 2 gated polymetric sequencer).
  - Dual utility: acts as internal drum trigger generator for TKR-1 and continuous/stepped modulation source for TKF!
  - **Tabled Algorithmic Engines (Deep Research Lab Scheduled)**: Conway's Game of Life (Cellular Automata), Wolfram Rule 30/110, Markov chains, and L-systems are formally tabled for a dedicated Deep Research Lab session upon active kickoff of the TKR-1 milestone.
  - Confirmed via `ui_ux_research_board.html` Post-Mortem.

---

### 5. The Klang Cultivator (TKC) — Standalone MIDI CC / CV / MPE Modulation Generator Rack
  *Goal: Extract our decoupled C++ ModulationEngine into a dedicated MIDI effect plugin that hosts our full 16-source modulation suite to control external DAW tracks and hardware synths.*
  - **Universal MIDI CC / CV Dispatch**: Assign any modulator (LFO, Env, Irrigator, Slope, Random, Slop) to an outgoing MIDI CC number, Channel Aftertouch, Pitch Bend, or high-resolution MPE pressure/slide.
  - **Eurorack CV Output**: Compatible with DC-coupled audio interfaces (sending low-frequency control voltages directly into Eurorack modular gear).
  - **8-Macro "Meta-Modulator" Parity**: The standalone plugin will feature 8 Top-Level Macros to ensure 1:1 preset compatibility when its patches are loaded as a nested "Cultivator Mod Block" inside The Klang Farmer's Mod Matrix.
  - **Architectural Guardrails**: Employs the "Macro Firewall" pattern. APVTS only sees the 8 Macros; internal Mod routing is serialized privately to JSON. Max nesting depth is strictly 1 to prevent infinite graph recursion and test-suite failures.
  - **Shared Codebase Heritage**: Inherits 100% of its DSP routing, modulation math, and 2x8 card UI directly from The Klang Suite core.

---

### 6. N'kai Sidecar Harness: Web Audio Haptics & WebView Audio Pipeline (Backlog)
*Goal: Resolve Chromium/Electron webview audio driver restrictions to enable physical hardware clicks and approval chimes in the N'kai sidecar.*
- **Context**: In Google Antigravity / Electron side-panel WebViews, `AudioContext` and HTML5 audio playback can be completely suspended or routed to inaccessible audio devices depending on host sandbox policies and Windows audio driver exclusivity (ASIO/WASAPI).
- **Deliverables**:
  - Investigate Antigravity Webview audio permissions and IPC audio dispatch (e.g. firing system audio alerts via Node.js or `window.sidecar` rather than browser audio).
  - Provide complete fallback and default-silent state with toggleable sound effects.
  - Sourced from N'kai Deep Research harvest ([`TheKlangResearch`](https://github.com/codygratner/TheKlangResearch/blob/main/subagents/2026-10-09_Nkai_Dynamic_Loaders_and_Blue_Sky_Harvest.md)).

---

### 7. Obsidian Vault Inbox & Addons Triage (Next N'kai Session)
*Goal: Process raw notes accumulated in `TheKlangVault/Inbox/` (including Obsidian Addons evaluations and mobile scratchpad dumps) in a dedicated N'kai research board with 1 section per note.*
- **Scope**: Evaluate suggested community addons (Dataview, Omnisearch, Excalidraw, Canvas), vault sync ergonomics, and mobile tag hygiene.
- **Workflow**: Automated generation of an interactive N'kai triage board directly from inbox markdown files (defaulting to sidecar board unless `--chat` flag is explicitly requested).

---

### 8. Post-Milestone Deep Docs & Field Guides Separation Gauntlet — ⏳ QUEUED
*Detailed Plan: [`.agents/pipeline/plans/drafts/post_milestone_deep_docs_gauntlet.md`](../.agents/pipeline/plans/drafts/post_milestone_deep_docs_gauntlet.md)*
*Goal: Execute during the post-milestone `/update-docs --deep` pass to decouple universal AI pair-programming lore from domain-specific C++ audio DSP engineering.*
- **Decouple `vibe_coding_field_guide.md`**: Strip out audio-specific C++ and JUCE parameter code blocks; elevate into the 100% stack-agnostic Manifesto for autonomous AI pair-programming across any tech stack.
- **Elevate `audio_dsp_field_guide.md`**: Expand from 33-line stub into the definitive real-time C++ audio and DSP engineering manual (audio thread invariants, SIMD Padé FastMath, filter topologies, anti-aliasing, and headless testing suites).
- **Five-Pillar Link Audit**: Recompile Vault Master Index (`tools/sync_obsidian_vault.ps1`) and verify 100% link integrity across all 5 ecosystem repositories.

---

### 9. Tri-State Model Advisory Architecture (Topic 4) — ✅ COMPLETED
*Goal: Streamline model selection protocol in `GEMINI.md` to eliminate banner fatigue and interactive pauses during routine turns while strictly maintaining safety tripwires for audio DSP and factory build failures.*
- **State 0 (95% Happy Path)**: Permanent Flash 3.8 High orchestrator daily driver. Routine turns, phase advances, and plan kickoffs run without multi-line advisory blocks or dropdown-check pauses, displaying only an ambient single-line footer badge (`[ ⚡ Engine: Flash High | Pro Subagents on Demand ]`).
- **State 1 (Internal Guardrail - No Direct DSP)**: The Flash orchestrator is strictly forbidden from writing differential equations, non-linear saturation curves, SIMD DSP loops, or lock-free atomics directly in-session; it MUST delegate Tier 1 math tasks to `invoke_subagent(Model="pro")`.
- **State 2 (Emergency Tripwire)**: A loud blocking advisory banner and hard-pause in chat text is strictly reserved for true emergency escalations: 2-Strike Factory Build Failures, Milestone Capstone Sanity Gates, and Release Gauntlets (`/cut-release`).
- **Status**: Implemented and codified in `GEMINI.md`, `docs/GLOSSARY.md`, and `docs/CHEAT_SHEET.md`.

---

### 10. Thematic Milestone Governance & Lightweight Anti-Willy-Nilly Guardrails (Topic 10) — ⏳ QUEUED
*Goal: Prevent random feature sprawl into unrelated milestones without adding bureaucratic overhead to solo developer vibe coding.*
- **Thematic Milestone Contracts**: Define explicit theme scope for upcoming releases (e.g. `v0.4.0` = Interface & Experience, `v0.5.0` = Sound & Chaos, `v0.6.0` = Pro Workflow) in `BACKLOG.md`.
- **Lightweight Sanity Tripwire**: When planning or backlogging new ideas, the agent checks the proposed milestone's theme. If an idea mismatches (e.g., proposing a new distortion algorithm during a UI polish milestone), the agent gently prompts with an 80/20 alternative: *"This feature belongs in v0.5.0 (Sound & Chaos). Would you like to backlog it there instead?"*
- **Decouple Backlog Capture from Execution Scope**: The developer can capture any idea instantly without friction, while execution scope remains tightly focused on the active milestone theme.
- **Sourced from**: Topic 10 PCDA & Dialectic Analysis ([`TheKlangResearch`](https://github.com/codygratner/TheKlangResearch)).

---

### 11. Persistent Mailbag Session & Dynamic Priority Queue — ⏳ QUEUED
*Goal: Evolve the morning mail triage workflow from an ephemeral snapshot into a persistent, stateful queue (`mail_queue.json`) decoupled from physical file movements.*
- **Persistent State Ledger (`.agents/pipeline/mailbag/mail_queue.json`)**: Tracks UUID, filename, timestamp, priority level (`P0` through `P3`), triage status (`pending`, `backlog`, `research`, `graveyard`, `docket`), and topic linkages across sessions.
- **Reverse-Chronological P0 Sorting**: Urgent and freshly captured notes (`P0` / `TOP PRIORITY`) dynamically sort to the top of the Mailbag in reverse-chronological order (newest first).
- **Topological Rejigger Intercept**: If a high-priority note depends on an unbuilt prerequisite (e.g., requesting a UI feature before the underlying IPC engine is built), the Mailbag renders a non-blocking suggestion chip: `[ 🔄 Slot into Topological Order ]` vs `[ ⚡ Keep at Top ]`.
- **Physical Archival Decoupling**: Archiving notes in `TheKlangVault/Inbox/Archive/` retains full session state, review history, and links in the Mailbag canvas without losing context.

---

### 12. Multi-Tier Rollback Architecture Across Agent Skills — 🔨 ACTIVE
*Goal: Establish stateless, instant atomic rollback paths across critical agent workflows to recover from mistaken operations, bad runs, or accidental file movements with zero token burn.*
- **Persistent Plans Queue & Plan Rollback (`clean-plan --rollback` / `/rollback-plan`)** — ✅ COMPLETED: Manages multi-plan queue in `.agents/pipeline/plans/queue.json` and snapshots active `PLAN.md` to `PLAN_PREV.md` before overwriting or promoting from `drafts/`. Executes 5ms stateless rollback.
- **Mailbox Archive Rollback (`tools/check_mail.py --rollback`)**: Emits atomic transaction receipts (`.agents/pipeline/staging/mail_tx_latest.json`) before moving files to `Archive/`. Running `--rollback` restores files in 5ms without LLM overhead.
- **Topological Itinerary Rollback (`tools/audit_reorder.py --rollback`)**: Snapshots the prior flight sequence (`itinerary_snapshot_prev.json`) before applying re-sequencing, allowing instant reversion if topological reordering causes friction.
- **Factory & Verification Rollback (Future Discussion)**: Staged rollback hooks for `execute-task` (git stash/checkout on 2-strike build failures), `step-verify` (un-staging failed phase checkpoints), and `update-docs` (restoring docs catalog snapshots on link integrity failures).

---

### 13. The Operatic Runway & Tooling Flights (Acts I & II) — ✅ COMPLETED
*The foundational infrastructure and developer tooling engine delivered in October 2026 across Acts I & II of the Operatic Flight Plan (`PLAN.md`).*
- **Flight 1 (Multi-Plan Queue & Rollback)**: Added `tools/plan_queue.py`, `.agents/pipeline/plans/queue.json`, and `/rollback-plan` skill for 5ms zero-token plan restoration.
- **Flight 2 (The Klang Vault Studio Hub)**: Hardened Obsidian configuration (`app.json`, `userIgnoreFilters`), initialized git repository, and authored multi-repo studio architecture spec.
- **Flight 3 (Architectural Zonal Boundaries)**: Authored 3-Zone READMEs (`Source/dsp/README.md`, `assets/README.md`, `tools/README.md`) prohibiting rogue per-folder blackboxes.
- **Flight 4 (Global Pydantic v2.14.0)**: Provisioned global Pydantic environment and unified data models (`tools/models.py`) mirrored across `TheKlangSuite`, `TheKlangVault`, and `nkai`.
- **Flight 5 (Mailbag Session & Pydantic Validation)**: Validated mailbag queues and Pydantic models for structured note triage.
- **Flight 6 (TKPL Preset Linter CLI)**: Authored `tools/preset_audit.py`, compiled `master_control_schema.json` (all 202 DSP parameters), and generated `preset.schema.json`.
- **Flight 7 (N'kai FastAPI Local Bridge)**: Deployed `tools/nkai_api.py` on `localhost:8000` with universal CORS, PNA preflight, and dynamic sidecar detection.
- **Flight 8 (Rolling Blackbox Context Compaction)**: Formalized context compaction tripwires, 4-step memory flush protocol, and 0-turn cold boot invariant in `GEMINI.md`, `docs/GLOSSARY.md`, and `docs/CHEAT_SHEET.md`.
- **Flight 9 (Subagent Soul Harvest)**: Vacuumed staged souls from `.agents/pipeline/staging/souls/` into `TheKlangResearch/subagents/`, achieving true Inbox Zero.
- **Flight 10 (Act III, Flight 1: The "Rambles" Ingestion Engine - Topic 2, P0)**: ✅ COMPLETED — Zero-dependency ramble ingestor CLI (`tools/ramble_ingest.py`), Pydantic models (`RambleCard`, `RambleExtractionResult`, `to_mail_item()`), 3-bullet decision card decomposition, and `/ramble` micro-skill (`.agents/skills/ramble-ingest/SKILL.md`).

---

### 14. Tooling Maintenance: Git for Windows Upgrade — ⏳ QUEUED
*Goal: Upgrade Git for Windows CLI to the latest release (`git update-git-for-windows` or `winget upgrade Git.Git`) during an idle maintenance window.*
- **Scope**: Perform clean CLI upgrade, verify credentials/SSH keys, and validate git subprocess operations across all 5 ecosystem repositories.
- **Trigger**: Run between feature plans when no active compilation or subagent git commits are running.

---

### 15. Open Source Anti-Slop Hardening & CI Safety Gauntlet — ✅ COMPLETED
*Goal: Harden public-facing open source posture and CI regression pipeline inspired by the Anti-Slop Audit (opensourceprojects.dev/post/anti-slop).*
- [x] **Audio-Thread Pre-Allocation Hardening (`Source/ModularBlocks.h`)**: Eliminated real-time vector resizes in `processStereo()` (lines 505, 608, 1716, 2219, 2401) by pre-allocating fixed scratch buffers in `prepareToPlay()` and bounding loops. `audit_audiothread.py --strict` passes with 0 critical violations.
- [x] **CI Matrix Schema & UI Reflection Enforcement**: Added `gui_tests` to `.github/workflows/ci.yml` (on Windows and on Linux under `xvfb-run -a`) to enforce automated C++ schema separation assertions (`ParameterSchemaAuditTest.h`) on external PRs.
- [x] **Audio Thread CI Linter Step**: Integrated `python tools/audit_audiothread.py --strict` into `.github/workflows/ci.yml` to automatically reject PRs introducing allocations, locks, or I/O into DSP render paths.
- [x] **Strict Pluginval Compliance**: Removed `|| true` masking from Linux and macOS `pluginval` runs in GitHub Actions.
- [x] **Cross-Pillar Governance Sync**: Authored universal `CONTRIBUTING.md` and `.github/PULL_REQUEST_TEMPLATE.md` across `TheKlangSuite`, `ToadTracker`, and `nkai`.
- [x] **10/10 EXCEED Audit Scorecard**: Re-evaluated all 10 criteria against `https://www.opensourceprojects.dev/post/anti-slop`, achieving 10/10 EXCEED / SOTA ratings across the ecosystem.

---

## 🪦 The Strategy Graveyard
*Architectural proposals, prototypes, and technical paths evaluated and deliberately rejected, documenting the strategic autopsy and rationale to prevent revisiting dead ends.*

### 1. Tracktion Engine as Scaffolding for ToadTracker (Topic 13 / Flight 8) — 🪦 KILLED / GRAVEYARD
- **Date:** 2026-10-11 | **Proposed Route:** Use Tracktion Engine as temporary desktop DAW scaffolding while developing ToadTracker.
- **Strategic Verdict:** **REJECTED (KILLED)**.
- **Autopsy & Failure Modes:**
  1. *Paradigm Collision*: Tracktion Engine is an object-heavy linear DAW engine (`te::Edit`, `te::Track`, `te::Clip`). Trackers are discrete, integer-based step sequencers and tick accumulators.
  2. *The Parallel Engine Debt*: Maintaining a Tracktion desktop version alongside a bare-metal microcontroller C++ engine doubles development debt and bug surface area.
  3. *Leaky Abstractions*: Temporary scaffolding inevitably creates hidden dependencies on DAW-style async streaming, making eventual microcontroller migration a catastrophic rewrite.
- **Winning Synthesis (The 80/20 Resolution)**: Adopt the **Dirtywave M8 / SunVox Model** — author a pure, allocation-free C++20 tracker core (`ToadCore`) that compiles identically on Desktop and TBD-16, wrapped in a lightweight ~100-line JUCE audio device host on Desktop. 100% hardware parity on Day 1, zero throwaway code, zero Tracktion bloat.

---

### 2. Spun-Up Flash Low Agent for N'kai Button Routing (Topic 16 / Flight 5) — 🪦 KILLED / GRAVEYARD
- **Date:** 2026-10-11 | **Proposed Route:** Spawn an autonomous background Flash Low subagent (`invoke_subagent(Model="flash_lite")`) to poll for webview button clicks and route action directives to the main chat session.
- **Strategic Verdict:** **REJECTED (KILLED)**.
- **Autopsy & Failure Modes:**
  1. *Using an LLM for Plumbing (AI Slop Anti-Pattern)*: Software engineering has possessed instant, deterministic, zero-cost IPC mechanisms (sockets, pipes, DOM events, native APIs) for 50 years. Using a multi-billion-parameter neural network as a dumb string relay between a webview and a chat window is textbook AI over-engineering.
  2. *Lifecycle & Deadlock Trap*: In Antigravity, subagents cannot be spawned by DOM button clicks or external scripts—only an active LLM turn can call `invoke_subagent`. An agent cannot run an infinite polling loop without hitting turn/timer limits.
  3. *The Double-Dip Quota Multiplier*: When a subagent calls `send_message()`, the main chat wakes up and executes a full turn. That means 1 button click = 2 separate LLM inferences, burning quota needlessly.
- **Winning Synthesis (The 80/20 Resolution)**: Rely on Antigravity's **Native UI Extension IPC (`window.sidecar.agent.sendMessage()`)**. When N'kai runs in its official home in the Antigravity Extension Pane, clicking any button dispatches directly to the active chat in 0ms with zero token burn. For standalone browser views, maintain the honest 1-click clipboard composer (`Ctrl+V`).

---

---

## 📁 Completed & Archived Milestones
All completed tasks, architectural decisions, and release summaries are archived in:
👉 **[`docs/BACKLOG_ARCHIVE.md`](history/BACKLOG_ARCHIVE.md)**  
*(Individual phase execution plans are preserved in `docs/../.agents/pipeline/plans/completed/`)*






