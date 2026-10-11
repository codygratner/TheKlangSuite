# ⚡ The Klang Suite: Developer Workflow Cheat Sheet
> **A 1-Page High-Density Operational Reference for Human Developers & AI Agents**  
> *Companion:* See [`docs/CHEAT_SHEET.md`](CHEAT_SHEET.md) for ecosystem glossary and [`docs/GLOSSARY.md`](GLOSSARY.md) for UI taxonomy.

---

## 🏛️ 1. Facility Spheres & Communiqué Routing

The workspace is strictly partitioned into two operational spheres to prevent context contamination:

| Facility Sphere | Domain & Mandate | Pipeline Files | Target Flag |
| :--- | :--- | :--- | :--- |
| **New Klang City** *(Ivory Tower / Tooling)* | Architectural planning, data schemas (`assets/`), documentation, Python/Node dev scripts, and N'kai HTML sidecars. | `subagent_dispatch.md` &rarr; `subagent_receipt.md` | `TARGET: TOOLING_HARNESS` |
| **Klang Industries** *(Factory Floor / C++)* | Native C++20 DSP engines, JUCE UI components, CMake/MSBuild targets, APVTS bindings, and real-time audio thread safety audits. | `plan_to_build.md` &rarr; `build_to_plan.md` | `TARGET: FACTORY_FLOOR` |

> [!IMPORTANT]
> **Anti-Snatching Guard**: Klang Industries never ingests tickets marked `TARGET: TOOLING_HARNESS`. New Klang City delegates mechanical file writing to Flash builder subagents (`invoke_subagent(Model="flash")`) to protect main chat context.

---

## 🧠 2. Model Tier Matrix & Subagent Swarm Roles

The main chat session stays permanently on **Tier 2 Flash High** (sustainable daily driver, near-zero burn). Deep math and architectural proofs are delegated autonomously on demand:

| Tier / Role | Recommended Setting | Model / Invocation | Primary Scope |
| :--- | :--- | :--- | :--- |
| **Tier 1 (Critical DSP & Architecture)** | Gemini 3.1 Pro (Thinking: High) | `invoke_subagent(Model="pro")` | Differential equations, non-linear saturation, lock-free concurrency, polyphony allocation, and milestone capstone sanity gates. |
| **Tier 2 (Universal Default)** | Gemini 3.8 Flash (Thinking: High) | Main Orchestrator Session | Standard JUCE components, APVTS bindings, unit test expansions, N'kai sidecars, and standard phase execution. |
| **Tier 3 (Rapid Iteration & Data)** | Gemini 3.8 Flash (Thinking: Low) | Subagent / Inline | JSON schemas, documentation/backlog updates, git operations, and mechanical find-and-replace. |
| **`research` Subagent** | Tier 1 Pro High Sprint | `invoke_subagent(Role="research")` | 1–3 short, surgical, high-density turns for deep DSP math and multi-file architectural synthesis. |
| **`explore` Subagent** | Tier 2 Flash High Marathon | `invoke_subagent(Role="explore")` | Autonomous sweeps: UX benchmarking, forum surveys, preset taxonomies, and competitive teardowns. |
| **`contrarian` Subagent** | Tier 2 Flash High Adversarial | `invoke_subagent(Role="contrarian")` | Devil's Advocate stress-testing: Exposes edge-case traps, dependency rot, and extracts the pragmatic 80/20 resolution. |

---

## 📋 3. The Standard 5-Phase Implementation Blueprint

Every flight or feature plan in `PLAN.md` executes through five disciplined phases:

```text
  Phase 1: Specification & Schema Registration
    │      ↳ Author docs/specs/*.md, register in DOCS_CATALOG.json & SYSTEM_MAP.md.
    ▼
  Phase 2: Core Architecture & Components
    │      ↳ C++ DSP/UI or core N'kai CSS/JS implementation.
    ▼
  Phase 3: Tooling, Automation & CLIs
    │      ↳ Python/Node developer utilities and offline validators.
    ▼
  Phase 4: Verification & Acceptance Gauntlet
    │      ↳ Automated tests (dsp_tests, gui_tests, web-qa, verify_context_clues).
    ▼
  Phase 5: Communiqué Handshake & Closeout
           ↳ Check off PLAN.md, write subagent receipt, conventional commit.
```

---

## 🛠️ 4. N'kai & Ecosystem CLI Quick Reference

All Python developer tools run cleanly from workspace root under Windows UTF-8 stdlib:

```powershell
# 1. N'kai Local Sidecar Server (Zero-dependency HTTP server with auto-browser launch)
python tools/nkai_serve.py .agents/sidecar/morning_mail_inbox.html --port 3030

# 2. N'kai Local FastAPI Bridge (:8000 bridge with universal CORS and PNA)
python tools/nkai_api.py

# 3. Multi-Plan Queue & 5ms Atomic Rollback Engine
python tools/plan_queue.py list
python tools/plan_queue.py promote-next
python tools/plan_queue.py rollback

# 4. Morning Mail Vault Inbox Harvester
python tools/check_mail.py --dir C:\Dev\TheKlangVault\Inbox --chat

# 5. TKPL: The Klang Preset Linter CLI
python tools/preset_audit.py --lint
python tools/preset_audit.py --export-schema assets/presets/$schema/preset.schema.json

# 6. PCDA Dialectic Validator & Formatter
python tools/pcda_format.py --validate docs/specs/nkai_pcda_governance_and_sota_benchmarks.md

# 7. Allowance Ledger & Token Burn Estimator
python tools/nkai_ledger.py --audit .agents/sidecar/nkai_deep_research.html --chat

# 8. Context Clues Hyperlink Integrity Validator
python tools/verify_context_clues.py

# 9. Obsidian Vault Sync Engine
powershell -ExecutionPolicy Bypass -File .\tools\sync_obsidian_vault.ps1
```

---

## 🛫 5. Aviation Metaphors & The Staged Flights PCDA

* **Flights (Sequential Integers `1..16`)**: Discrete tasting flights of work; flat integer index keeps cognitive load near zero.
* **Acts (Altitude Bands)**: Act I (Ground Control & Taxiway) &rarr; Act II (Ascent & Cruising Altitude) &rarr; Act III (High-Altitude Reconnaissance & Gauntlet) &rarr; Act IV (Final Approach & Touchdown).
* **The 80/20 Synthesis**: Upfront N'kai visual planning (1–2 hrs) stress-tests dependencies *before* compiling, eliminating 3-week non-linear detours and keeping the factory floor on high-velocity cruise.
* **The 0-Turn Cold Boot**: Agents wake up and read `context_clues_*.md` on Turn 1, seamlessly resuming work without asking "what should we do next?".
