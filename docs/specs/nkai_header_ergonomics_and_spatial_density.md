# N'kai Header Ergonomics & Spatial Density (Topics 6.9 & 6.10)

## 1. Architectural Intent & Scope

The **N'kai Asymmetric Sidecar Framework** (`nkai`) powers interactive visual canvases across the Four Pillars (`TheKlangSuite`, `ToadTracker`, `nkai`, `TheKlangResearch`). In IDE environments like Google Antigravity, Cursor, and VS Code, sidecars predominantly render inside an embedded side-panel webview constrained to a narrow viewport (typically 360px to 480px wide).

### The Problem: Header Bloat & Vertical Real Estate Collapse
In earlier iterations, the sidecar header (`.app-header`) utilized a single-row flex container that mixed operational mode buttons (`ACTIVE WORKSHOP`, `FULL POST-MORTEM & TRIAGE SLATES`), research category tabs (`LAYOUT TOPOLOGIES`, `UI / UX DESIGN`, `DSP ARCHITECTURE`, `BLUE SKY THINKING`, `TBD-16 ECOSYSTEM`), and utility controls (`[ ⌨️ Shortcuts ]`, `A- 100% A+`, `🔄 Force Refresh`) in a single wrapping layout.

In typical 380px–420px IDE side panels, this resulted in severe usability penalties:
1. **Vertical Space Squandering**: The single container wrapped across 3 to 5 clumsy lines, consuming 160px–220px of precious vertical height before the user could even see the active content canvas.
2. **Visual Hierarchy Confusion**: Operational modes (governing the high-level workflow engine) competed visually with category navigation filters and global utility controls.
3. **Utility Misplacement**: Critical accessibility tools (font zoom, refresh, keybinding aids) shifted position unpredictably depending on button label length and container wrapping.

### The Solution: Option B (Two-Tier Header & Hybrid Compact Tabs)
Formally approved under **Topics 6.9 and 6.10**, Flight 5 introduces a comprehensive redesign of the N'kai header ergonomics:
1. **Topic 6.9 — Two-Tier Header Geometry**: Establishes a permanent, structural separation between identity/utilities (Top Tier) and operational/category navigation (Bottom Tier).
2. **Topic 6.10 — Hybrid Compact Tab Density**: Condenses research and domain category tabs to high-density icon + index badges with hover tooltips, while keeping primary operational modes prominently labeled.
3. **Audio Haptics Protocol**: Integrates tactile synthesizer-grade audio feedback (`🔊` / `🔇`) via Web Audio API, paired with persistent user preference storage.
4. **Vertical Savings**: Reclaims 40px–60px (and up to 120px+ on narrow displays) of vertical height, locking the entire header to ~68px–72px total height across all screen widths down to 320px.

---

## 2. Two-Tier Header Geometry & Anatomy

The header uses a fixed, sticky container with glassmorphic backdrop-filter blurring, segmented into two distinct functional tiers:

```
┌─────────────────────────────────────────────────────────────────────────────────────────┐
│ .app-header (sticky top: 0, z-index: 1000, backdrop-filter: blur(10px))                  │
├─────────────────────────────────────────────────────────────────────────────────────────┤
│ Tier 1: .header-tier-top (Height: ~36px)                                                │
│ ┌──────────────────────┐               ┌──────────────────────────────────────────────┐ │
│ │ 🔬 N'KAI RESEARCH    │               │ [⌨️ Shortcuts] [A- 100% A+] [🔊] [🔄 Refresh]│ │
│ │ (Brand Title/Identity)│              │ (Persistent Utility Cluster)                 │ │
│ └──────────────────────┘               └──────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────────────────────────────────────────┤
│ Tier 2: .header-tier-bottom (Height: ~34px, Horizontal Scroll Rail)                     │
│ ┌───────────────────┐ ┌───────────────────┐  ┌───────┐ ┌───────┐ ┌───────┐ ┌───────┐    │
│ │ 🛠️ WORKSHOP       │ │ ⚖️ POST-MORTEM   │  │ 1 📐  │ │ 2 🎨  │ │ 3 🎛️  │ │ 4 🌌  │ ...│
│ │ (Named Mode)      │ │ (Named Mode)      │  │ (1.1) │ │ (2.1) │ │ (3.1) │ │ (4.1) │    │
│ └───────────────────┘ └───────────────────┘  └───────┘ └───────┘ └───────┘ └───────┘    │
└─────────────────────────────────────────────────────────────────────────────────────────┘
```

### Top Tier (`.header-tier-top`)
- **Brand Title (`.header-brand`)**: Positioned on the far left. Displays the project glyph, name, and optional SemVer badge (`🔬 N'KAI RESEARCH` or `🎹 THE KLANG SUITE`). On narrow viewports (<360px), gracefully collapses to icon-only.
- **Persistent Utility Cluster (`.header-utilities`)**: Positioned on the far right as a flex-aligned group:
  1. `[ ⌨️ Shortcuts ]` (`.shortcut-container`): Toggles the popover drawer for Antigravity keybindings and slash commands.
  2. Font Zoom Scaler (`.zoom-ctrl`): Provides `A-`, active percentage (`100%`), and `A+` controls adhering to accessible dynamic sizing (`--zoom` or root `font-size`).
  3. Audio Haptics (`.haptics-btn`): Toggles tactile audio feedback (`🔊` active, `🔇` muted).
  4. Force Refresh (`.refresh-btn`): Executes cache-busting canvas reload bypassing Chromium `file:///` memory cache traps.

### Bottom Tier (`.header-tier-bottom`)
- **Horizontal Navigation Rail**: A single-line, non-wrapping scroll rail (`overflow-x: auto; white-space: nowrap`) hosting all primary and secondary navigation controls.
- **Scroll Hygiene**: Features thin custom scrollbars (`scrollbar-width: thin`), momentum scrolling on touch/wheel, and zero vertical clipping.

---

## 3. Hybrid Compact Tab Standard & Tooltip Accessibility

### Distinction Between Operational Modes and Category Filters
In N'kai sidecars, tabs serve two fundamentally distinct purposes:
1. **Primary Operational Modes**: Switch entire workflow engines (e.g., Active Workshop triage vs Full Post-Mortem summary table). These remain **fully named text buttons** (`🛠️ WORKSHOP`, `⚖️ POST-MORTEM`) with distinct accent colors.
2. **Category / Domain Tabs**: Filter or deep-link into specific sections of the research or canvas (e.g., Layouts, UI, DSP, Blue Sky, Hardware). In earlier versions, verbose names like `LAYOUT TOPOLOGIES` consumed massive horizontal space. Under Flight 5, these are condensed into **Hybrid Compact Tabs** (`.tab-compact`).

### Compact Tab Geometry & Anatomy
Each compact tab represents a domain section as an index badge and thematic emoji:
- `1 📐` — Layout Topologies & Monolithic Faceplates
- `2 🎨` — UI / UX Design & Vector Glyphs
- `3 🎛️` — DSP Architecture & Anti-Aliasing
- `4 🌌` — Blue Sky Thinking & Cellular Automata
- `5 📦` — TBD-16 Hardware Ecosystem

```html
<button class="mode-btn tab-compact" id="btn-layouts" onclick="switchMode('layouts')" title="1 📐 Layout Topologies & Faceplates">
  <span class="tab-index">1</span>
  <span class="tab-icon">📐</span>
  <span class="tab-tooltip">1 📐 Layout Topologies</span>
</button>
```

### Dual-Layer Accessible Hover Tooltips
To ensure zero friction and complete accessibility across all operating systems and viewports, N'kai employs a dual-layer tooltip architecture:
1. **Layer 1: Native OS Tooltip (`title` attribute)**:
   - Guaranteed 100% reliability; rendered directly by the operating system / window manager.
   - Never clipped by container overflow, scroll rails, or iframe boundaries.
2. **Layer 2: Instant CSS Tooltip (`.tab-tooltip`)**:
   - Styled to match the dark slate theme (`#1f242c`, border `#30363d`, font mono 11px).
   - Positioned above or below the tab with smooth opacity and vertical micro-translation.
   - Set to `pointer-events: none` to prevent click trapping or hovering glitches.

---

## 4. Audio Haptics Protocol (`🔊` / `🔇`)

To evoke the tactile, physical hardware experience of high-end synthesizers and grooveboxes (e.g., Elektron, Teenage Engineering), N'kai sidecars provide synthetic click feedback for navigation events.

### Web Audio Invariants & Sandboxing
- **Zero Allocations in Audio Path**: In the sidecar web context, clicks are synthesized on-demand via standard Web Audio API oscillators.
- **User Gesture Requirement**: `AudioContext` is lazily instantiated upon the first user click to strictly adhere to Chromium and WebKit autoplay sandboxing policies.
- **Persistent Mute State**: The mute state is saved under `localStorage.getItem('nkai_audio_muted')`. When muted, zero audio nodes are created or processed.

### Haptic Feedback Signatures
- **Tab / Navigation Switch (`'tab'`)**: Quick 40ms triangle-wave pitch chirp (440Hz $\to$ 880Hz) with exponential volume decay.
- **Primary Action / Unmute (`'action'`)**: 60ms dual-tone harmonic pulse (D5 587Hz $\to$ A5 880Hz).
- **Standard Button Click (`'click'`)**: Subtle 20ms sine transient pip (800Hz $\to$ 400Hz, peak gain 0.03).

---

## 5. Responsive Breakpoint Specification

The header layout dynamically adapts across four distinct viewport widths:

| Viewport Width | Tier 1 (Top) Behavior | Tier 2 (Bottom) Behavior | Total Height |
| :--- | :--- | :--- | :--- |
| **Desktop / Wide (`> 800px`)** | Full brand title (`🔬 N'KAI RESEARCH & BRAINSTORMING`), full utility buttons with text. | Operational modes and all compact category tabs displayed inline with generous spacing. | ~70px |
| **Medium Side-Panel (`481px – 800px`)** | Shortened brand title (`🔬 N'KAI RESEARCH`), utility cluster fully visible. | Operational modes + compact tabs fit horizontally on a single line. | ~68px |
| **Narrow Side-Panel (`361px – 480px`)** *(Antigravity Default)* | Compact brand title, iconized utilities (`[⌨️]`, `A- 100% A+`, `[🔊]`, `[🔄]`). | Operational modes (`🛠️ WORKSHOP`, `⚖️ POST-MORTEM`) + compact tabs (`1 📐` ... `5 📦`) in smooth single row. | ~68px |
| **Ultra-Narrow (`320px – 360px`)** | Brand glyph only (`🔬`), tightly packed utilities. | Horizontal scroll rail with smooth touch panning; zero wrapping or clipping. | ~66px |

---

## 6. DOM Contract & CSS Class Reference

### DOM Structure
```html
<header class="app-header">
  <!-- Tier 1: Brand & Utilities -->
  <div class="header-tier-top">
    <div class="header-brand">
      <span class="brand-logo">🔬</span>
      <span class="brand-name">N'kai Research</span>
      <span class="brand-badge">v1.0.0</span>
    </div>
    <div class="header-utilities">
      <div class="shortcut-container">
        <button class="shortcut-pill" id="btn-shortcuts" onclick="toggleShortcuts()" title="Keyboard Shortcuts">
          <span>⌨️</span> <span class="util-label">Shortcuts</span>
        </button>
        <!-- Popover content -->
      </div>
      <div class="zoom-ctrl">
        <button class="zoom-btn" onclick="changeFontSize(-1)">A-</button>
        <span class="zoom-level" id="font-zoom-level">100%</span>
        <button class="zoom-btn" onclick="changeFontSize(1)">A+</button>
      </div>
      <button class="haptics-btn" id="btn-haptics" onclick="toggleAudioHaptics()" title="Audio Haptics (Sound Feedback)">
        <span id="haptics-icon">🔊</span>
      </button>
      <button class="refresh-btn" onclick="forceReloadCanvas()" title="Force Refresh Canvas">
        <span>🔄</span> <span class="util-label">Refresh</span>
      </button>
    </div>
  </div>

  <!-- Tier 2: Modes & Compact Tabs -->
  <div class="header-tier-bottom">
    <!-- Operational Modes -->
    <button class="mode-btn workshop active" id="btn-workshop" onclick="switchMode('workshop')">
      <span>🛠️</span> WORKSHOP
    </button>
    <button class="mode-btn pm" id="btn-pm" onclick="switchMode('pm')">
      <span>⚖️</span> POST-MORTEM
    </button>

    <div class="header-divider-v"></div>

    <!-- Compact Category Tabs -->
    <button class="mode-btn tab-compact" id="btn-layouts" onclick="switchMode('layouts')" title="1 📐 Layout Topologies">
      <span class="tab-index">1</span>
      <span class="tab-icon">📐</span>
      <span class="tab-tooltip">1 📐 Layout Topologies</span>
    </button>
    <button class="mode-btn tab-compact" id="btn-ui" onclick="switchMode('ui')" title="2 🎨 UI / UX Design">
      <span class="tab-index">2</span>
      <span class="tab-icon">🎨</span>
      <span class="tab-tooltip">2 🎨 UI / UX Design</span>
    </button>
    <button class="mode-btn tab-compact" id="btn-dsp" onclick="switchMode('dsp')" title="3 🎛️ DSP Architecture">
      <span class="tab-index">3</span>
      <span class="tab-icon">🎛️</span>
      <span class="tab-tooltip">3 🎛️ DSP Architecture</span>
    </button>
    <button class="mode-btn tab-compact" id="btn-blue" onclick="switchMode('blue')" title="4 🌌 Blue Sky Thinking">
      <span class="tab-index">4</span>
      <span class="tab-icon">🌌</span>
      <span class="tab-tooltip">4 🌌 Blue Sky Thinking</span>
    </button>
    <button class="mode-btn tab-compact" id="btn-tbd" onclick="switchMode('tbd')" title="5 📦 TBD-16 Ecosystem">
      <span class="tab-index">5</span>
      <span class="tab-icon">📦</span>
      <span class="tab-tooltip">5 📦 TBD-16 Ecosystem</span>
    </button>
  </div>
</header>
```

### Core CSS Tokens
- `.header-tier-top`: `display: flex; align-items: center; justify-content: space-between; width: 100%; padding: 6px 16px; border-bottom: 1px solid rgba(48, 54, 61, 0.4);`
- `.header-tier-bottom`: `display: flex; align-items: center; width: 100%; padding: 6px 16px; overflow-x: auto; white-space: nowrap; gap: 8px; scrollbar-width: thin;`
- `.tab-compact`: High-density `padding: 5px 10px; font-size: 11px; min-width: 44px; display: inline-flex; align-items: center; gap: 4px;`
- `.tab-tooltip`: Absolute CSS hover badge positioned above/below tab with `opacity: 0` transitioning to `1` on hover.

---

## 7. Synchronization & Verification Gauntlet

1. **Core Library Updates**: Synchronized across `c:\Dev\nkai\core\sidecar.css` and `c:\Dev\nkai\core\sidecar.js`.
2. **Canonical Templates**:
   - `c:\Dev\nkai\dist\standalone-template.html`
   - `c:\Dev\nkai\bundle\templates\template.html`
   - `c:\Dev\TheKlangSuite\.agents\sidecar\template.html`
3. **Auditing**: Validated against `tools/audit_sidecar.py` ensuring:
   - 100% HTML tag balance.
   - Clean `forceReloadCanvas()` cache-busting implementation.
   - Complete `localStorage.getItem('nkai_audio_muted')` audio hygiene contract.
   - Headless runtime execution with 0 console errors.
