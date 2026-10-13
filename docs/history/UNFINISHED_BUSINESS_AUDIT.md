# Cross-Ecosystem Unfinished Business Audit (rj-46)
**Date:** 2026-10-13  
**Auditor:** New Klang City Architecture & Tooling Engineering  
**Scope:** `TheKlangSuite`, `nkai`, `ToadTracker`, `PCDACore`, `TheKlangVault`  

---

## 1. Executive Summary

This audit establishes a single, ground-truth inventory of all in-progress, partially built, tabled, and queued items across the entire ecosystem. There is **zero phantom work**: all core codebases have clean, unblocked git trees, and every active inbox note has been synthesized into the canonical rejigger overlay (`rj-01` through `rj-54`).

```
Ecosystem Repository Health:
  ├── TheKlangSuite   🟢 CLEAN C++ CODE (Branch: 0.4.0-dev | 0 stashes)
  ├── nkai            🟢 100% CLEAN (Branch: main | Templates in sync)
  ├── ToadTracker     🟢 100% CLEAN (Branch: main | Bus integrity synced)
  ├── PCDACore        🟢 100% CLEAN (Branch: master | v1.0.0 shipped)
  └── TheKlangVault   🟡 READ-ONLY MIRROR (248 modified files awaiting batch docs push)
```

---

## 2. Immediate Open Loops & Actionable Findings

### 🔴 Finding A: Stranded Inter-Project Bus Dispatch (TKS ➔ TT)
- **ID:** `2026-10-12_tks_to_tt_the_klang_farmer_vst3_uid_and_`
- **Route:** `TKS` ➔ `TT` (Priority: P1)
- **Status:** `PENDING`
- **Topic:** The Klang Farmer VST3 UID and Parameter Contract Handoff for Renoise `.xrns` Project Exporter.
- **Action Required:** Run `python tools/bus.py ack 2026-10-12_tks_to_tt_the_klang_farmer_vst3_uid_and_` from ToadTracker to formalize the handshake for `BACKLOG-06`.

### 🟡 Finding B: Obsolete Plan Drafts in Pipeline Buffer
- Location: `.agents/pipeline/plans/drafts/`
- Four draft blueprints have already shipped their deliverables and should be archived from `drafts/` to `completed/`:
  1. `master_studio_hub_and_pydantic_data_infrastructure.md` (Topic 7 / Pydantic models shipped)
  2. `nkai_v020_ui_extension_heist.md` (N'kai v0.2.0 shipped)
  3. `persistent_mailbag_queue_and_rollback_architecture.md` (Flight 5 shipped)
  4. `rj22_six_way_da_benchmark.md` (Benchmark shipped)
- **Remaining Active Drafts:**
  - `2026-10-08_v041_ui_shells.md` (Queued for Factory Floor v0.4.1)
  - `agent_ecosystem_systemic_audit.md` (Queued for Method B+ systemic audit)
  - `post_milestone_deep_docs_gauntlet.md` (Queued for Act III `/update-docs --deep`)
  - `v0.5.0_visual_harness_architecture.md` (Queued for v0.5.0 umbrella)

### 🟡 Finding C: Shipped Notes Held on Workbench Shelf
- Location: `C:\_\Vault\Inbox\Staging/`
- 8 notes were completely shipped and resolved by `rj-45` in the Oct 12–13 night flight (`Nkai Mobile`, `PCDACore GitHub`, `PCDACore and lenses`, `NkaiCore`, `Staging area for inbox`, `Can we make a template...`, `Korg Logue SDK...`, and `normal vs heavy pcda`).
- **Action:** Can be moved to `C:\_\Vault\Archive/` to achieve Inbox/Staging Zero whenever desired.

### 🟢 Finding D: Two Staged Souls Ready for Act III Distillation
- Location: `.agents/pipeline/staging/souls/`
  1. `2026-10-12_antipattern_queue_vs_latch.md` (Hardware latch safety law)
  2. `2026-10-12_language_server_and_subagent_lifecycle.md` (LSP & subagent lifecycle gotchas)
- **Status:** Safely staged; scheduled for distillation into `TheKlangResearch` during Act III deep documentation sweeps.

---

## 3. Cross-Repo Backlog Status & Next Execution Steps

### 1. TheKlangSuite (`0.4.0-dev`)
- **Active Plan:** `rj-46` (Cross-Ecosystem Unfinished Business Audit).
- **Frozen Item:** Flight 21 (Checkpoint Resumption Engine & Opus Deep DSP Audit) — **FROZEN** until Sunday quota reset per user rule.
- **Top P1 Decisions Ready for User Approval:**
  - `rj-47`: Command Bridge Architecture (Cosmic Codex Central Hub vs Repository Pair-Programming).
  - `rj-52`: TKS v0.4 Architecture Crucible (Heavy PCDA on Core Assumptions).
  - `rj-54`: Mailbag Rejigger Intelligence & Multi-Source Card Provenance.

### 2. ToadTracker (`main`)
- **Working Tree:** 100% clean (`214ba03`).
- **Ready Feature Flights in Root `BACKLOG.md`:**
  - `BACKLOG-01`: Dedicated Project Screen (`VIEW_PROJECT`) & File Operations (P1 — Ready).
  - `BACKLOG-04`: Instantaneous Hold-to-View Navigation Modal (P1 — Ready).
  - `BACKLOG-05`: Steam Deck Target Harness & Direct SDL3/evdev Input Architecture (P1 — Ready).
  - `BACKLOG-06`: Native Renoise `.xrns` Project Exporter (P2 — Ready, awaiting TKS VST3 handshake).

### 3. N'kai (`main`)
- **Working Tree:** 100% clean (`569a271`).
- **Daemon Server:** Running live on port `3030` (`task-31012`) with LAN & mobile companion support.
- **Next Candidate Deliverable:** `rj-49` (Obsidian Webview Sidecar Tab & Quick-Dispatch Ribbon).

### 4. PCDACore (`master`)
- **Working Tree:** 100% clean (`21fae1f`).
- **Status:** v1.0.0 complete with canonical prompt rule, 4 domain lenses, and GPLv3 license.
- **Next Candidate Deliverable:** `rj-51` (PCDA Governance, Tiered Nomenclature, and Gauntlet Policy).

---

## 4. Immediate "To-Do Now" Action Checklist

- [x] **Audit Git working trees** across all 5 ecosystem repositories (All clean).
- [x] **Audit active plans and drafts** in `.agents/pipeline/plans/`.
- [x] **Audit cross-repo bus dispatches** (`C:\Dev\TheKlangVault\Dispatches`).
- [ ] **Acknowledge pending dispatch:** Ingest `TKS ➔ TT` VST3 UID handshake.
- [ ] **Archive 4 obsolete drafts:** Move shipped plans from `drafts/` to `completed/`.
- [ ] **Archive 8 shipped staging notes:** Move completed notes from `Inbox/Staging/` to `Archive/`.
- [ ] **Select next active flight:** Choose between ToadTracker `BACKLOG-01` (Project Screen) or TKS `rj-47` (Cosmic Codex Command Bridge).
