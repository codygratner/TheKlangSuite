---
name: automode
description: Executes N'kai Automode, a strictly bounded 2-pass iterative research sweep across queued triage items. Generates affirmative SOTA precedents, adversarial contrarian stress-tests, 4-lens PCDA matrices, and in-card multi-drawer dossiers with zero Pro model quota burn. Triggers on /automode, "run automode", "auto research", or "2-pass research".
---

# 🔬 N'kai Automode: 2-Pass Iterative Research Daemon (`/automode`)

> **Tooling Command:** `python tools/research_daemon.py`  
> **Model Tier:** Tier 2 Flash (High) — 🟢 SUSTAINABLE (Zero Pro Burn)  
> **Pillar:** N'kai Tooling Harness (`TARGET: TOOLING_HARNESS`)  
> **Specification:** [`docs/BACKLOG.md` (Flight 3: N'kai Automode)](#17-flight-3-nkai-automode-2-pass-iterative-research-daemon--multi-drawer-dossier-cards-p1--top-priority)  

---

## 🎯 Purpose & The "Rule of 2" Architecture

Unbounded autonomous research loops create severe failure modes: runaway token burn, split-brain state collisions, and compounding hallucinations. Multi-agent reflection research demonstrates that architectural gains steeply plateau after iteration 2; pass 3+ yields negative returns.

**N'kai Automode** enforces the **Rule of 2**:
1. **Pass 1 (Parallel Divergent Sweep — Tier 2 Flash)**:
   - *Explore Vector*: SOTA precedents, affirmative capabilities, and architectural leverage.
   - *Contrarian Vector*: Adversarial stress-testing using the **Method B+ Clause** (`Where I Disagree With The Prompt`), surfacing edge-case traps and anti-patterns.
2. **Pass 2 (Convergent Synthesis — Tier 2 Flash)**:
   - Reconciles Thesis and Antithesis into a 3-bullet decision card (🎯 Core Intent, 🛠️ Proposed Mechanism, 🚦 Action Route).
   - Generates a full 4-lens PCDA matrix (Ergonomics, Audio Software, Wider SOTA, Best Practices).
   - Stages a curated technical dossier with citations and platform gotchas.

---

## 🛡️ Ironclad Human-Gating Invariants

- **No Blind Vault Sweeps**: Automode NEVER autonomously touches raw notes in `TheKlangVault/Inbox/`. It operates strictly on items explicitly triaged with `[ 🔬 Research ]` in `morning_mail_inbox.html` or marked `status: "research"` in `mail_queue.json`.
- **Strict Batch Cap (`--limit 3`)**: Default execution batch size is strictly capped at 3 items. Operates as a finite, one-shot CLI job, NEVER as an infinite continuous background loop.
- **Zero Pro Burn**: Strictly prohibited from using Tier 1 Pro models. Runs 100% on Tier 2 Flash.

---

## 🚀 Usage & Commands

### 1. Batch Execution on Queued Research Items
Processes up to 3 queued items:
```powershell
python tools/research_daemon.py
```

### 2. Custom Batch Size
Process up to $N$ items:
```powershell
python tools/research_daemon.py --limit 5
```

### 3. Targeted Single Item Research
Research a specific topic, card ID, or note query:
```powershell
python tools/research_daemon.py --topic "Nkai Automode"
```

### 4. Safe Dry-Run Simulation
Inspect generated 2-pass dialectics without modifying ledgers or staging files:
```powershell
python tools/research_daemon.py --topic "Hardware Daisy Bridge" --dry-run
```
