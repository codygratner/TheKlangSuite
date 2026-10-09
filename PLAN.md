# Implementation Plan: Flight 5 — Header Ergonomics & Spatial Density
> **TARGET:** TOOLING_HARNESS (New Klang City) 🚀  
> **Flight Target:** Topics 6.9 & 6.10 (Header Layout & Spatial Density)  
> **Status:** COMPLETED ✅  
> **Target Branch:** `0.4.0-dev`  
> **Recommended Model:** Gemini 3.8 Flash (Thinking: High) — 🟢 SUSTAINABLE  
> **Execution Strategy:** Delegated to Tier 2 Flash Builder Subagent via `subagent_dispatch.md`  

---

## 🎯 Architectural Intent & Scope

Flight 5 optimizes the vertical real estate and spatial density of the N'kai sidecar header:
1. **Topic 6.9 (Approved Option B — Two-Tier Header w/ Persistent Utility Cluster):**
   - Implements a structured two-tier header layout:
     - **Top Tier (Brand & Utilities)**: Brand title on left; persistent utility cluster on right (Font Zoom scaler `A-`/`100%`/`A+`, Audio Haptics toggle `🔊`, Refresh button `🔄`, and `[ ⌨️ Shortcuts ]` pill).
     - **Bottom Tier (Navigation & Tabs)**: Clean horizontal scrollable tab rail hosting operational modes and category filters.
2. **Topic 6.10 (Approved Option B — Hybrid Compact Tab Density w/ Hover Tooltips):**
   - Conserves 40–60px of vertical header space in narrow IDE viewports (360px–480px):
     - Primary operational modes remain clearly labeled text tabs (`🔬 WORKSHOP`, `⚖️ POST-MORTEM`).
     - Category/research domain tabs condense to clean icon + index badges (e.g. `1 🗺️`, `2 📦`, `3 🚀`) with instant full-text CSS hover tooltips (`title` attribute + accessible tooltip styling).
   - Authors `docs/specs/nkai_header_ergonomics_and_spatial_density.md` formalizing header geometry, breakpoint behaviors, and tooltip accessibility.
   - Updates `core/sidecar.css`, `core/sidecar.js`, and canonical templates in `dist/`, `bundle/`, and `TheKlangSuite`.

---

## 📋 Phase-by-Phase Execution Plan

### Phase 1: Header Architecture & Spatial Density Specification (Topics 6.9 & 6.10)
- [x] Author `docs/specs/nkai_header_ergonomics_and_spatial_density.md`:
  - Detail Two-Tier Header geometry, sticky behavior, and z-index layering.
  - Document the Hybrid Compact Tab standard (named operational modes vs condensed emoji category chips).
  - Document CSS tooltip positioning and touch/narrow viewport responsiveness (down to 320px).
- [x] Register spec in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.

### Phase 2: Update Core CSS & JS in `c:\Dev\nkai\core/`
- [x] In `c:\Dev\nkai\core\sidecar.css`:
  - Implement `.header-tier-top` (brand + utility cluster) and `.header-tier-bottom` (tab strip).
  - Implement `.tab-compact` with `.tab-tooltip` hover styling and smooth fade transitions.
  - Add responsive media queries ensuring zero clipping or wrap distortion on narrow (360px) IDE side panels.
- [x] In `c:\Dev\nkai\core\sidecar.js`:
  - Ensure tab switching (`switchTab`) cleanly supports compact tab badges and activates corresponding viewports without layout jitter.

### Phase 3: Synchronize Canonical Templates
- [x] Update `c:\Dev\nkai\dist\standalone-template.html`.
- [x] Update `c:\Dev\nkai\bundle\templates\template.html`.
- [x] Update `c:\Dev\TheKlangSuite\.agents\sidecar\template.html`.
- [x] Update `c:\Dev\TheKlangSuite\.agents\sidecar\roadmap_sidecar.html` (if applicable) for layout consistency.

### Phase 4: Verification Gauntlet & Closeout
- [x] Run `python tools/audit_sidecar.py` across updated templates (100% tag balance, zero syntax errors).
- [x] Run `python tools/verify_context_clues.py` in `c:\Dev\TheKlangSuite`.
- [x] Commit changes cleanly in `TheKlangSuite` and `nkai` with conventional commits: `feat(header): implement Flight 5 two-tier header and compact tab density`.

---

## 🛡️ Verification & Acceptance Criteria
1. `docs/specs/nkai_header_ergonomics_and_spatial_density.md` is valid and indexed in `DOCS_CATALOG.json`.
2. Header cleanly organizes brand & utilities on Tier 1 and navigation tabs on Tier 2.
3. Compact tabs render cleanly with hover tooltips and zero horizontal overflow on 360px viewports.
4. `audit_sidecar.py` and `verify_context_clues.py` exit with 0 errors.
