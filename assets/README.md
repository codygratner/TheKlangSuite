# 📦 Zone 2: Data Schemas & Parameter Contracts
> **Architectural Zone:** Zone 2 (Data Schemas & Assets)  
> **Facility Owner:** Cross-Facility Data Layer (Klang Industries & New Klang City)  
> **Core References:** [`docs/GLOSSARY.md`](../docs/GLOSSARY.md) · [`docs/CHEAT_SHEET.md`](../docs/CHEAT_SHEET.md) · [`GEMINI.md`](../GEMINI.md)  

---

## 🏛️ The 4-Layer Separation of Concerns (CRITICAL)

To prevent parameter collisions, UI layout bugs, and translation fragmentation, all JSON files in `assets/` strictly adhere to their designated layer:

1. **`assets/controls/*.json` (APVTS Parameter Contracts)**:
   - Strictly DSP parameter definitions: parameter ID, parameter type (`float`, `choice`, `bool`), normalized ranges, default values, snap intervals, and choices.
   - **Strict Zero Rule**: ZERO visual styling, ZERO colors, and ZERO text descriptions or display labels.
2. **`assets/layouts/*.json` (Structural UI Surface Hierarchy)**:
   - Structural layout hierarchy: Pages $\to$ Cards $\to$ array of bound parameter IDs.
   - Declares which parameters appear on which card, without defining their internal DSP ranges or colors.
3. **`assets/themes/*.json` (Visual Styling & LookAndFeel)**:
   - Visual styling, color palettes, and typography (`theme.json`).
   - Floating overlay/callout geometry, background/border colors, and bound parameter arrays (`callouts.json`).
4. **`assets/text/*.json` (Copywriting & Localization)**:
   - Parameter descriptions, choice tooltips, and localized UI copy (`strings.json`).

---

## 🧭 Zonal Governance & Anti-Pattern Prohibition

* **No Hardcoded Values in C++**: Parameter IDs, ranges, default values, snap points, and labels must be defined here in JSON, never hardcoded in C++ source files unless technically mandatory.
* **No Rogue Per-Folder Blackboxes**: Schema definitions and directory guidelines stay centralized here in `assets/README.md`.
