# The 4-Layer Declarative Data Schema Architecture

> **Authority:** New Klang City Architecture Standard  
> **Target:** `assets/controls/`, `assets/layouts/`, `assets/themes/`, `assets/text/`

---

## 1. Architectural Philosophy: Why Separate Concerns?

In traditional audio plugin codebases, parameter bounds, string labels, visual colors, layout coordinates, and tooltip copy are frequently intertwined inside monolithic C++ classes. This leads to severe architectural debt:
- Changing a button color requires recompiling C++.
- Adding localization requires touching core DSP parameter constructors.
- AI refactors accidentally invert parameter ranges or drop UI bindings.

To solve this, **The Klang Suite** enforces a **strict 4-layer declarative JSON schema separation**. Every piece of data belongs exclusively to one layer.

---

## 2. The 4 Distinct Schema Layers

```text
┌────────────────────────────────────────────────────────┐
│ Layer 4: TEXT & COPY                                   │
│ assets/text/strings.json                               │
│ (Parameter descriptions, tooltips, localized copy)     │
├────────────────────────────────────────────────────────┤
│ Layer 3: THEMES & STYLING                              │
│ assets/themes/theme.json & callouts.json               │
│ (Color palettes, module accents, typography, callouts) │
├────────────────────────────────────────────────────────┤
│ Layer 2: STRUCTURAL LAYOUT                             │
│ assets/layouts/*.json                                  │
│ (Chassis hierarchy: Pages -> Cards -> Slots -> IDs)    │
├────────────────────────────────────────────────────────┤
│ Layer 1: DSP & APVTS DATA CONTRACTS                    │
│ assets/controls/*.json                                 │
│ (Types, bounds, defaults, skew factors, choices)       │
└────────────────────────────────────────────────────────┘
```

---

### Layer 1: DSP & Parameter Contracts (`assets/controls/*.json`)
- **Responsibility:** Defines the immutable host DAW automation contract.
- **Allowed Keys:** `type` (`float`, `choice`, `bool`), `min`, `max`, `default`, `double_click`, `skew`, `format`, `snap_points`, `choices`.
- **STRICT PROHIBITION:** **ZERO visual styling, ZERO colors, ZERO descriptions, and ZERO pixel dimensions.** Leaking a color into a control definition causes `ParameterSchemaAuditTest` to hard-fail the build.

### Layer 2: Structural UI Hierarchy (`assets/layouts/*.json`)
- **Responsibility:** Defines the spatial arrangement of the synth chassis.
- **Hierarchy:** `Pages` $\to$ `Cards` (modules) $\to$ array of bound parameter IDs.
- **Separation:** Layout files only store IDs (e.g. `"id": "mod_decay"`). They do not store how the parameter behaves or what color it renders with.

### Layer 3: Visual Themes & Overlays (`assets/themes/*.json`)
- **Responsibility:** Visual styling, palettes, module identity accents, and callout geometry.
- **Files:**
  - `theme.json`: Color palettes (`cykranosh`, `nord`, `dracula`, `cyberpunk`), card border colors, meter hues, font profiles.
  - `callouts.json`: Floating popover dimensions, border radii, and secondary parameter groupings.
- **Live Switching:** Themes can be hot-swapped dynamically at runtime without restarting the DAW.

### Layer 4: Text Copy & Descriptions (`assets/text/strings.json`)
- **Responsibility:** User-facing descriptions, choice tooltips, and status bar feeds.
- **Namespaces:** Partitioned into `"shared"`, `"farmer"`, and `"planter"`.
- **Localization:** Allows translating 100% of user copy without touching DSP algorithms or layout schemas.

---

## 3. Automated Guardrail Tests

The 4-layer schema is validated by `ParameterSchemaAuditTest.h`:
1. **Schema Leak Sweep:** Asserts zero color codes (`#`, `0x`) or descriptions in `assets/controls/`.
2. **Contract Validity:** Asserts `min < max`, `min <= default <= max`, and `skew > 0`.
3. **Orphan Feature Audit:** Asserts 100% of registered APVTS parameters match JSON entries, and vice versa.
