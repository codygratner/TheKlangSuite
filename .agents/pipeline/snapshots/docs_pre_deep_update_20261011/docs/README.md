# 🏛️ The Klang Suite Documentation Wiki

Welcome to the internal engineering documentation and architectural knowledge base for **The Klang Suite** (*The Klang Farmer*, *The Klang Planter*, and *The Klang Editor*).

This directory serves as the **Master Map of Content (MOC)**. It is mirrored directly to **The Klang Vault** for mobile reading via Obsidian Sync, and is authored with standard relative Markdown links to guarantee 100% clickable navigation on GitHub.com.

---

## 🗺️ Master Roadmaps & Living Standards

- **[Documentation Catalog](DOCS_CATALOG.json)**: Master index and navigation manifest for all documentation, specs, research archives, and sidecars.
- **[Master Product Backlog & Roadmap](BACKLOG.md)**: Prioritized milestones, feature lists, and upcoming releases.
- **[Architectural Taxonomy (Glossary)](GLOSSARY.md)**: Canonical terminology (*Chassis*, *Cards*, *Modules*, *Sliders*, *Knobs*).
- **[Third-Party Licenses & Attribution](THIRD_PARTY_LICENSES.md)**: FOSS licensing, provenance, and GPLv3 compliance ledger.
- **[Project Root README](../README.md)**: Public project showcase, screenshots, and build instructions.
- **[Changelog](../CHANGELOG.md)**: SemVer release notes and version history.

---

## 📐 Technical Architecture (`docs/architecture/`)

Deep technical design standards and system contracts:

- **[Product Lineup & Core Intent Matrix](architecture/product_lineup.md)**: Master ecosystem taxonomy, instrument identities (TKF, TKP, TKE, TKM, TKR-1, TKB), and hardware synergies.
- **[Real-Time Audio Thread Invariants](architecture/audio_thread_invariants.md)**: Zero-allocation, zero-lock, and zero-blocking I/O rules for real-time DSP stability.
- **[4-Layer Declarative Data Schema](architecture/data_schema_layers.md)**: Strict separation of concerns across Controls, Layouts, Themes, and Text/Localization.
- **[Headless GUI Testing & Synthetic Validation](architecture/headless_gui_testing.md)**: Headless component inspection, 6-pillar VST3 parameter normalization, and offscreen rendering.
- **[Developer Logging Subsystem (TKS_LOG)](architecture/dev_logger_subsystem.md)**: Rotating diagnostic logging with audio-thread assertions and zero release overhead.
- **[Obsidian Asymmetric Sync Bridge](architecture/obsidian_sync_bridge.md)**: Conflict-free partition architecture connecting mobile Obsidian Sync with Git.

---

## 🚀 Active & Future Specifications (`docs/specs/`)

Comprehensive design blueprints for upcoming milestones and sound design capabilities:

### Milestone v0.4.0: Interface & Experience
- **[Neo-Slate Vector UI & 4-Controls-Per-Card](specs/v040_neo_slate_ux.md)**: Clean-slate vector overhaul, 1x4 meter sliders, and chassis header navigation.
- **[SQA Automation Hardening](specs/sqa_automation_hardening.md)**: Dual timeouts (5m/30s), wait-fail locators, and automated failure screenshots.

### Milestone v0.5.0: Sound & Chaos
- **[Voice Articulation, Gated Bass & Glide](specs/gated_bass_glide.md)**: Staccato gated release, semitone pitch slew, and portamento curves.
- **[FX Processors Catalog Expansion (14–26)](specs/fx_catalog_expansion.md)**: 13 new studio effects, universal dual-mode mix, and 5-column browser.
- **[Audio & Workflow Expansion (Undo/Redo & MIDI Learn)](specs/pre_v1_sound_and_workflow_expansion.md)**: Non-destructive A/B comparison and dynamic velocity curves.

### Milestone v0.6.0: Pro Workflow
- **[JSON Preset Management System](specs/preset_system.md)**: Dual-column tag browser and automated patch migration.
- **[Dual Transient Sample Players](BACKLOG.md#item-62-dual-transient-sample-players-transient-page)**: Percussive sample playback and choke groups on the transient page.
- **[WAV & SoundFont (SF2) Drag-and-Drop Export](specs/wav_sf2_export.md)**: One-click export for hardware grooveboxes and external samplers.
- **[Advanced Typography Engine](specs/typography_engine.md)**: Embedded binary typography and CSS-style font profiles.
- **[Zero-Server GitHub Crash Reporting](specs/crash_reporting.md)**: Privacy-preserving crash dump export.
- **[One-Time Quick Tour Onboarding](specs/one_time_quick_tour.md)**: "Right-Click is the Way" interactive feature walkthrough.

### Spinoff Hardware & Companion Tools (`docs/specs/spinoffs/`)
- **[The Klang Mill (TKM 1x6 Pedalboard Rack)](specs/spinoffs/the_klang_mill.md)**: Dedicated 6-slot modular multi-effects host.
- **[The Klang R1 (TKR-1 Synthesizer)](specs/spinoffs/the_klang_r1.md)**: Handheld hardware synth instrument.
- **[Zynthian OS V5 Platform Port](specs/spinoffs/zynthian_port.md)**: Headless LV2/VST3 engine for Raspberry Pi Linux audio boxes.
- **[ToadTracker Retro Migration](specs/spinoffs/toadtracker_migration.md)**: 8-bit chip chiptune engine interoperability.

---

## 📜 Development History & Archives (`docs/history/`)

Institutional memory and milestone summaries:

- **[The Vibe-Coder's Field Guide](history/vibe_coding_field_guide.md)**: Autonomous pair-programming workflows, agent pipelines, and the Strategy Graveyard.
- **[The Audio DSP & C++ Field Guide](history/audio_dsp_field_guide.md)**: C++ audio engineering, DSP thread invariants, and advice for transitioning developers.
- **[Deep Research Soul Harvest](https://github.com/codygratner/TheKlangResearch/blob/main/subagents/2026-10-08_Subagent_Deep_Research_Harvest.md)**: Permanent archive of DSP equations, architectures, and proofs from Subagent research.
- **[Development Journal (DEV_HISTORY.md)](history/DEV_HISTORY.md)**: Living chronological record of architecture decisions and milestones.
- **[Historical Backlog Archive](history/BACKLOG_ARCHIVE.md)**: Pre-v0.3.0 tasks and legacy prototype notes.
- **[v0.3.0 Release History](history/archives/DEV_HISTORY_v0.3.0.md)**: Complete record of the v0.3.0 Architecture Update.

---

## 🎨 Sidecar Infrastructure (`.agents/sidecar/`)

- **[Sidecar Template](../.agents/sidecar/template.html)**: Standardized interactive HTML sidecar shell (dark theme, font zoom, state sync).
- **[Active Grill Lab Sidecar](../.agents/sidecar/visual_grill_me.html)**: Visual design and architectural interview canvas.

---

## 🎙️ Consultation & Stakeholder Briefings (`docs/briefings/`)

- **[SQA Advisory Briefing with Tom (October 7, 2026)](briefings/2026-10-07_sqa_meeting.md)**: High-velocity vibe coding QA, timeouts, and automated visual defect reproduction.
