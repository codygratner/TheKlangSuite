# 🧭 Flight 0: North Star Focus Doc & Ambient Status Footer (`rj-09`)

> **ID:** plan-007  
> **TARGET:** TOOLING_HARNESS (New Klang City) 🚀  
> **Topic Target:** Mailbag Card `rj-09` / Section 15 ("Priorities" + "Multiple Repos")  
> **Status:** COMPLETED ✅  
> **Priority:** P0 (Top Priority — First Flight After Mailbag Triage)  
> **Target Branch:** `0.4.0-dev`  
> **Recommended Setting:** Gemini 3.8 Flash (Thinking: Low/Medium) — ⚡ MINIMAL  
> **Source Note:** [`C:\Dev\TheKlangVault\Inbox\Archive\Priorities.md`](file:///C:/Dev/TheKlangVault/Inbox/Archive/Priorities.md) & [`C:\Dev\TheKlangVault\Inbox\Archive\Multiple Repos.md`](file:///C:/Dev/TheKlangVault/Inbox/Archive/Multiple%20Repos.md)  

---

## 🎯 Architectural Intent & Storyline

In the N'kai Morning Mailbag triage, Card `rj-09` identified that bouncing across three separate repositories (`TheKlangSuite`, `ToadTracker`, `nkai`) plus research (`TheKlangResearch`) and notes (`TheKlangVault`) is highly productive for a solo developer, provided exactly **one repository has active focus at any given moment**.

Without an explicit living North Star:
1. Context switches risk "amnesia drift" where sidequests bleed into unrelated milestones.
2. Remembering versions across 4 pillars creates cognitive friction.
3. Chat footers become cluttered with multiple disclaimers, links, and banners.

**Flight 0** delivers:
1. `docs/FOCUS.md`: A concise, living anchor declaring the active mission, the 4-pillar priority order (`N'kai solid` $\to$ `TKS 0.5.0-dev` $\to$ `Toad PoC`), and strict Negative Architecture (what we are *not* doing right now).
2. The Gentle Nudge Protocol: A 1-sentence non-blocking re-centering note after research detours (a nudge, never a gate).
3. The Unified Ambient Status Footer: A clean, 1-line footer replacing cognitive overhead with real-time clarity.

---

## 📋 Phase Breakdown

### Phase 1: Author `docs/FOCUS.md` (The Living North Star)
- [x] Author `docs/FOCUS.md`:
  - Active Mission & Current Phase.
  - The Priority Sequence: `1. N'kai Solidification` $\to$ `2. TKS v0.4.0 Release / v0.5.0-dev Sound & Chaos` $\to$ `3. ToadTracker PoC on Steam Deck / Desktop`.
  - The Four Pillars Matrix: Status, active branch, target milestone.
  - Negative Architecture: Explicitly documented boundaries (No mobile v0.3.0 before v0.2.0 is hardened; no DAW engine rewriting; no premature DSP before UI shell v0.4.1; no speculative moonshots).
  - The Gentle Re-Centering Nudge rule.

### Phase 2: Ambient Status Footer Line (`GEMINI.md`)
- [x] Update `GEMINI.md` under "Strict Model Advisory Protocol & Anti-Spam Gate":
  - Standardize the single-line ambient status footer format:
    `Focus: <Current Focus> | TKS-<ver> · TT-<ver> · NK-<ver> · TR-<ver>`
  - Codify dynamic board link rule: Append `· [ 📬 Open N'kai Board ](sidecar://nkai/panel/)` only when an N'kai board session is active; omit on routine engineering turns to keep transcripts clean.
  - Codify the Gentle Re-Centering Nudge for post-research transitions.

### Phase 3: Synchronize Canonical Documentation Invariants
- [x] Synchronize `docs/GLOSSARY.md` (Section 17: North Star & Focus Protocol).
- [x] Synchronize `docs/CHEAT_SHEET.md` (Footer & Focus reference).
- [x] Register `docs/FOCUS.md` in `docs/DOCS_CATALOG.json` under `core_governance`.
- [x] Register `docs/FOCUS.md` in `docs/SYSTEM_MAP.md` under Section 1 (Architecture & System Overview).

### Phase 4: Verification, Queue Sync & Multi-Repo Commits
- [x] Run `python tools/verify_context_clues.py` (assert 100% link resolution).
- [x] Update `docs/BACKLOG.md`: Mark Flight 0 as `[x] COMPLETED`.
- [x] Update `context_clues_plan.md` to reflect Flight 0 completion.
- [x] Commit changes cleanly in `TheKlangSuite`.
