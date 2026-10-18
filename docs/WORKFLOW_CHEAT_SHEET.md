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

# 8. Context Clues & UTF-8 Hyperlink Integrity Validators
python tools/verify_context_clues.py
python tools/verify_utf8.py

# 9. Inter-Project Data Bus & Obsidian Vault Sync Engine
python C:\_\Vault\tools\bus.py status --for COSMOS
powershell -ExecutionPolicy Bypass -File .\tools\sync_obsidian_vault.ps1

# 10. Rhyl'al's Book Sovereign Vault Sync & Frontmatter Repair (/rhylals-book-vault)
python C:\Dev\rhylals-book-vault\sync_vault.py
```

---

## 🛫 5. Aviation Metaphors & The 4 Sovereign Runway Invariants

* **Flights (Sequential Integers `1..16`)**: Discrete tasting flights of work; flat integer index keeps cognitive load near zero.
* **The 4 Sovereign Runway Invariants**:
  1. **14-Day Mailbag Buffer**: Capture phone/voice notes into `Inbox/` anytime; batch-triage on a 14-day cadence.
  2. **Two Tabling Gates**: Every item passes *Triage Gate* (Mailbag &rarr; Agenda) and *Runway Gate* (Agenda &rarr; Active Flight) with Method B+ PCDA.
  3. **`1-Code + 1-Creative` Runway Cap**: At most 1 heavy engineering sprint and 1 creative/lore/music sprint active concurrently.
  4. **"Rejiggering Reality" Circuit Breaker**: Invoke `/audit-reorder` to prune scope and restore a linear 80/20 critical path whenever friction rises.
* **The 0-Turn Cold Boot**: Agents wake up and read `ACTIVE_CONTEXT.md` and `DEV_HISTORY.md` on Turn 1, seamlessly resuming work without asking "what should we do next?".

---

## 🎮 6. ToadTracker Widescreen (`640×360`) & Leverless Southpaw Quick Reference

* **Widescreen 16:9 Split-Pane (`v010_widescreen_nerdseq_ux.md`)**:
  * **Left Sequencer Pane (`X=0..421`)**: 8-track Song View, Phrase View with `T-1`/`T+1` adjacent track ghost previews (`X=264..420`), Chain, Table, Synth, Mixer.
  * **Right NerdSEQ Inspector Dock (`X=424..636`)**: Live SPSC Vector Oscilloscope, 4 Macro Gauges, Polyphonic Chord Detector, History Breadcrumb, and clickable `3×3` Spatial Minimap.
  * **Integer Scaling Hotkeys**: `Ctrl+1` (`640×360` 1×) · `Ctrl+2` (`1280×720` 2× Steam Deck) · `Ctrl+3` (`1920×1080` 3× FHD) · `Ctrl+4` (`2560×1440` 4× QHD).
* **Universal & Southpaw Input Standard (`universal_input_and_controls_spec.md` & `midi_and_mixbox_hardware_spec.md`)**:
  * **Edit / Confirm (`X` / `East`)**: Stamp note, `+Direction` value edit (`±1` fine / `±16` coarse), double-tap insert new chain/phrase.
  * **Alt / Option (`Z` / `South`)**: Copy value, `+Edit` (`Z+X`) cut/clear, `+Left Shift` (`Shift+Z`) cycle scope (`Track` &rarr; `Song` &rarr; `Live`).
  * **Nav (`Left Shift` / `West`)**: `+Arrows` 2D spatial matrix navigation (`Up: Song -> Chain -> Phrase -> Synth`, `Right: Table`), `+Edit` (`Shift+X`) paste/interpolate.
  * **Shoulder Superpowers**: `L1` (`Right Shift`) Context Menu & Block Highlight · `R1` (`Backspace`/`Delete`) Non-Destructive A/B Undo Toggle · `L1+R1+Left/Right` Linear History Scrub.
  * **"The Toad" Leverless Southpaw Hardware (`BACKLOG-15`)**: Reverse variant places the angled Kailh Choc arrow cluster under the **Right Hand** and modifiers (`Edit`, `Alt`, `Nav`, `Play`, `L1`, `R1`) under the **Left Hand** for 100% `m8.run` muscle-memory parity.
