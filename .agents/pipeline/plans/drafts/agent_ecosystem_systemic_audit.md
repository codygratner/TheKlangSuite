# Blueprint: Systemic Audit & Method-B Adversarial Stress-Test of Agent Tasks & Workflows

> **TARGET:** TOOLING_HARNESS · Staged Draft Buffer  
> **Topic:** Comprehensive Ecosystem Audit (Communication, Queues, Latches, Mailbox, Labs, Verification, Memory)  
> **Recommended Model Tier:** Tier 2 Flash High (Orchestrator) + Tier 1 Pro High (Targeted Method-B Adversarial Subagents)  
> **Status:** DRAFT / PROPOSED 📋  

---

## 1. Executive Summary & Objective

With the recent launch of **The Cosmos** (`C:\_\Vault`) and a dedicated Antigravity project managed by the **Cosmic Codex** agent, our ecosystem now operates as a true multi-agent, multi-repository network:
- **Project A (`TheKlangSuite`)**: Native C++ audio DSP and JUCE factory floor.
- **Project B (`The Cosmos`)**: Personal knowledge zettelkasten, TTRPG game design, Simplenote sync, and custom Obsidian tooling.
- **Project C (`ToadTracker`)**: Hardware groovebox / tracker for SteamOS and embedded microcontrollers.
- **Project D (`nkai`)**: Standalone sidecar UI extension framework.
- **Project E (`TheKlangResearch`)**: Centralized RFC and architectural knowledge hub.

Over the past two weeks, New Klang City engineered an extensive suite of bespoke tools: 21 agent skills, 29 Python utilities, 2 multi-stage queue/overlay systems, dual-track communiqués, sidecar HTML labs, and automated verification gauntlets.

**The Mission:** Execute an exhaustive, adversarial system-wide audit of all agent-level mechanisms. Apply the proven **Method B Devil's Advocate Protocol** (Thesis $\rightarrow$ Friction $\rightarrow$ Searing Antithesis $\rightarrow$ 80/20 Synthesis + Mandatory *"Where I Disagree With The Prompt"*) to each cluster, uncovering hidden bloat, redundant plumbing, and multi-AGY concurrency hazards, culminating in a definitive **Keep / Refactor / Merge / Kill** roadmap.

---

## 2. The 4 Audit Clusters & Method B Dialectics

```
+----------------------------------------------------------------------------------------------------+
|                                    THE 4 AUDIT CLUSTERS                                            |
+-----------------------------------+----------------------------------------------------------------+
| Cluster 1: Cross-Project Concurrency| Inter-Project Data Bus vs Intra-Repo Communiqué vs Junction     |
| Cluster 2: Capture & Triage Flow   | Vault Inbox vs Simplenote vs check_mail vs rejigger vs overflow|
| Cluster 3: Interactive Sidecar Labs| Morning Mail vs Sandbox vs Board Hub vs Visual Grill-Me vs Research|
| Cluster 4: Execution & Memory Flush| Factory Gauntlet vs 4-Step Pre-Compaction Flush vs Soul Harvest|
+-----------------------------------+----------------------------------------------------------------+
```

### Cluster 1: Inter-Agent & Cross-Project Messaging
- **Target Mechanisms**:
  - The Inter-Project Data Bus (`Dispatches/`, `tools/bus.py`).
  - The Dual-Track Communiqué:
    - Factory Pipeline: `plan_to_build.md` $\leftrightarrow$ `build_to_plan.md` (`TARGET: FACTORY_FLOOR`).
    - Tooling Pipeline: `subagent_dispatch.md` $\leftrightarrow$ `subagent_receipt.md` (`TARGET: TOOLING_HARNESS`).
  - The NTFS Directory Junction bridge (`C:\Dev\TheKlangVault` $\longleftrightarrow$ `C:\_\Vault\New Klang City`).
- **Core Dilemma to Stress-Test**:
  - Now that Cosmic Codex and New Klang City run concurrently in separate Antigravity IDE instances, what are the exact concurrency hazards?
  - Does having two communication layers (external bus + internal communiqué) cause cognitive friction and naming debt, or is it an indispensable boundary law?
- **Method B Adversarial Prompting Clause**:
  - *Must include "Where I Disagree With The Prompt" challenging the premise that unified queues are superior to local semaphores.*

### Cluster 2: Human Capture, Mailbag & Triage Pipeline
- **Target Mechanisms**:
  - Raw Vault `Inbox/` & `Inbox/Tabled/` (agent tabling protocol).
  - `tools/check_mail.py` & `tools/models.py`.
  - The Pro Rejigger overlay (`rejigger.json` — 70 raw notes $\rightarrow$ 37 curated cards).
  - Overflow queue (`tools/ramble_ingest.py` $\rightarrow$ `overflow.json`).
  - Simplenote sync engine (`tools/sync_simplenote.py`).
  - Position-splicing CLI (`tools/mail_add.py`).
- **Core Dilemma to Stress-Test**:
  - Are we spending more engineering time maintaining triage tools than actually reviewing notes?
  - Is the `rejigger.json` static overlay fragile when new notes arrive?
  - How does Simplenote sync interact with raw phone rambles and N'kai mobile web triage?
- **Method B Adversarial Prompting Clause**:
  - *Must include "Where I Disagree With The Prompt" challenging whether complex priority ranking algorithms are just sophisticated procrastination.*

### Cluster 3: Interactive Sidecar Labs & Exploration Engines
- **Target Mechanisms**:
  - N'kai Panel Sidecar (`morning_mail_inbox.html`, `sandbox_mail_inbox.html`, `cli_board.html`).
  - Multi-Board Hub & Session Manager (`tools/board_registry.py`, `main.mjs` cache busting).
  - Per-Card Response Drawer (`agent_reply` markdown rendering).
  - Mobile Companion (`tools/nkai_serve.py --lan`, terminal QR code, touch layout).
  - Visual Grill-Me Lab (`visual-grill-me`, live HTML sidecar + design interview).
  - Deep Research Lab (`deep-research`, standalone interactive research boards).
  - Audit-Reorder skill (`audit-reorder`, 2-Pass Convergence Gate).
- **Core Dilemma to Stress-Test**:
  - Does maintaining 5+ distinct interactive HTML boards create browser cache drift, template duplication across repos, and Electron rendering overhead?
  - How can we achieve an 80/20 consolidation into a single unified sidecar surface without losing offline portability?
- **Method B Adversarial Prompting Clause**:
  - *Must include "Where I Disagree With The Prompt" evaluating whether standalone HTML sidecars are truly superior to simple terminal CLIs or markdown.*

### Cluster 4: Execution, Verification & Institutional Memory
- **Target Mechanisms**:
  - Factory Lifecycle: `execute-task` $\rightarrow$ `step-verify` $\rightarrow$ `task-finish`.
  - Safety Gauntlet: `audiothread-guard`, `build-validate`, `compat-check`, `web-qa`, `cut-release`.
  - Rolling Blackbox Context Compaction & 4-Step Pre-Compaction Flush (`docs/history/DEV_HISTORY.md`, `context_clues_*.md` 0-turn cold boot).
  - Telescoping Soul Harvest Standard (`staging/souls/` $\rightarrow$ `TheKlangResearch` RFCs $\rightarrow$ `vibe_coding_field_guide.md`).
- **Core Dilemma to Stress-Test**:
  - Does the 4-step flush and context clues ritual add unacceptable friction to routine turns, or is it our only insurance against LLM amnesia and chat context compaction?
  - Can we automate context clues updates so human/agent manual synchronization is minimized?
- **Method B Adversarial Prompting Clause**:
  - *Must include "Where I Disagree With The Prompt" auditing whether our memory rituals are over-engineered for solo vibe coding.*

---

## 3. Execution Phases

### Phase 1: Landscape Inventory & External SOTA Benchmarking
- Catalog all 21 skills, 29 tools, and 8 pipeline directories with exact lines of code, last modified dates, and active callers.
- Benchmark modern 2026 multi-agent orchestration architectures (e.g. Anthropic multi-agent patterns, local-first agent protocols) via web search.

### Phase 2: Targeted Pro Adversarial Subagents (Method B Sprints)
- Spawn targeted Pro research subagents (`Model="pro"`) executing blind or scoped Method B teardowns across Clusters 1, 2, 3, and 4.
- Each subagent produces a structured PCDA report ending with `"Where I Disagree With The Prompt"`.

### Phase 3: The Grand Synthesis & Scorecard
- Consolidate subagent findings into an ecosystem-wide master report:
  `C:\Dev\Research\agentic\2026-10_agent_ecosystem_systemic_audit.md`.
- Produce the definitive **Keep / Refactor / Merge / Kill Matrix** for all tools and workflows.
- Formulate concrete simplification proposals (e.g. renaming `plan_to_build`/`build_to_plan` to `factory_dispatch`/`factory_receipt`, consolidating redundant sync scripts).

### Phase 4: Documentation & Protocol Hardening
- Register the audit report in `docs/DOCS_CATALOG.json` and `Master_Research_Index.md`.
- Update `GEMINI.md` and `docs/GLOSSARY.md` with multi-AGY concurrency rules governing New Klang City and Cosmic Codex.
- Stage action items in `docs/BACKLOG.md` (or execute lean quick-wins).
