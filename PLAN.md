# Implementation Plan: Flight 3 — Skill Taxonomy & Invocation Defaults
> **TARGET:** TOOLING_HARNESS (New Klang City) 🚀  
> **Flight Target:** Topics 6.4 & 6.14 (Skill Taxonomy & Invocation Defaults)  
> **Status:** COMPLETED ✅  
> **Target Branch:** `0.4.0-dev`  
> **Recommended Model:** Gemini 3.8 Flash (Thinking: High) — 🟢 SUSTAINABLE  
> **Execution Strategy:** Delegated to Tier 2 Flash Builder Subagent via `subagent_dispatch.md`  

---

## 🎯 Architectural Intent & Scope

Flight 3 standardizes the ecosystem's agent skill taxonomy and formalizes N'kai-first visual workflows:
1. **Topic 6.4 (Approved Option C — Layered Hybrid Skills):**
   - Preserves discrete, atomic micro-skills (`audiothread-guard`, `build-validate`, `step-verify`, `web-qa`, `audit-reorder`) for precision developer intervention.
   - Formalizes high-level macro orchestrators (`/execute-task`, `qa`, `/check-mail`) that deterministically sequence underlying micro-skills.
   - Authors `docs/specs/nkai_skill_taxonomy_and_invocation_architecture.md` cataloging the layered hierarchy and lifecycle.
2. **Topic 6.14 (Approved Option B — N'kai-First with `--chat` Fallback):**
   - Establishes that all visual, interview, and research skills (`visual-grill-me`, `deep-research`, `post-mortem`) default to launching the interactive N'kai sidecar canvas.
   - Formalizes the `--chat` / `--no-sidecar` escape hatch, allowing users on terminal CLIs or low-bandwidth chats to force a pure text/Markdown inline experience.
   - Updates `SKILL.md` files in `.agents/skills/` and `c:\Dev\nkai\bundle\skills/`.

---

## 📋 Phase-by-Phase Execution Plan

### Phase 1: Skill Taxonomy & Invocation Architecture Specification (Topics 6.4 & 6.14)
- [x] Author `docs/specs/nkai_skill_taxonomy_and_invocation_architecture.md`:
  - Detail the 3-Layer Hierarchy:
    - Layer 1: Atomic Micro-Skills (Specialized single-purpose tools: `audiothread-guard`, `build-validate`, `web-qa`, `step-verify`).
    - Layer 2: Macro Orchestrators (Composed deterministic workflows: `/execute-task`, `qa`, `/check-mail`).
    - Layer 3: Interactive Visual Experiences (N'kai sidecar canvases: `/grill-me`, `/deep-research`, `/postmortem`).
  - Document the N'kai-First invocation standard and `--chat` terminal fallback flag.
- [x] Register spec in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.

### Phase 2: Update Core Visual Skills with `--chat` Fallback Standard
- [x] Update `.agents/skills/visual-grill-me/SKILL.md`:
  - Explicitly document default N'kai sidecar canvas generation (`file:///<artifactDir>/visual_grill_me.html`).
  - Add `--chat` / `--text-only` flag handling: switches to interactive chat interview with option numbers.
- [x] Update `.agents/skills/deep-research/SKILL.md`:
  - Default to generating interactive research board (`file:///<artifactDir>/<slug>_research_board.html`).
  - Add `--chat` flag handling: outputs summary report directly in chat.
- [x] Update `.agents/skills/post-mortem/SKILL.md`:
  - Default to generating interactive triage canvas with Decision Composer.
  - Add `--chat` flag handling: executes inline terminal interview.

### Phase 3: Update Macro Orchestrator (`execute-task` and `qa`) Conventions
- [x] Update `.agents/skills/execute-task/SKILL.md`:
  - Document the deterministic chain: `audiothread-guard` ➔ `build-validate` ➔ `step-verify` ➔ `task-finish`.
  - Document the domain routing: C++ targets dispatch to Klang Industries; tooling targets dispatch to Flash subagent.
- [x] Update `.agents/skills/web-qa/SKILL.md`:
  - Clarify dispatch routing: `qa` during C++ DSP invokes `audiothread-guard` + `build-validate`; during sidecar/tooling invokes `web-qa`.

### Phase 4: Synchronize to `nkai/bundle/` and Verification Gauntlet
- [x] Synchronize updated skills to `c:\Dev\nkai\bundle\skills/` (`visual-grill-me`, `deep-research`, `post-mortem`, `web-qa`).
- [x] Run `python tools/audit_sidecar.py` and `python tools/verify_context_clues.py`.
- [x] Stage and commit changes cleanly in `TheKlangSuite` and `nkai` with conventional commits: `feat(skills): implement Flight 3 layered skill taxonomy and --chat fallback`.

---

## 🛡️ Verification & Acceptance Criteria
1. `docs/specs/nkai_skill_taxonomy_and_invocation_architecture.md` is valid and indexed in `DOCS_CATALOG.json`.
2. `visual-grill-me`, `deep-research`, and `post-mortem` specify N'kai-first default with `--chat` fallback.
3. `c:\Dev\nkai\bundle\skills/` is 100% in sync with workspace skills.
4. `verify_context_clues.py` exits with code 0 (100% link resolution).
