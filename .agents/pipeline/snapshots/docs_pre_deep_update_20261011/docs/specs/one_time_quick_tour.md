# Implementation Plan: One-Time Quick Tour ("Right-Click is the Way")
**Target Milestone**: v0.5.0 ("The Pro Workflow Update")  
**Objective**: Introduce a sleek, unobtrusive, single-screen onboarding card on first plugin launch highlighting the tactile power of right-click gestures, snap callouts, randomizer controls, and hidden shortcuts.

---

## 1. Goal Description
The Klang Farmer suite features powerful right-click context menus, quick-snap intervals, selector menus, voice articulation callouts, and macro assignment workflows that transform the plugin into a fluid hardware experience. However, users unfamiliar with modern boutique UI conventions often treat knobs like standard 1D DAW sliders and miss out on these features.

This feature introduces a non-intrusive, single-screen **First Launch Quick Tour** card that opens automatically on the first boot of the plugin (and can be recalled anytime from the header `[ ? ]` button or Settings modal). It visually hammers home that **right-clicking is the way and the light**.

---

## 2. User Review Required
> [!IMPORTANT]
> **One-Time Persistence**: The tour status will be saved to the user's local persistent configuration file (`%APPDATA%/TheKlangFarmer/settings.json` on Windows, `~/Library/Application Support/TheKlangFarmer/settings.json` on macOS) under `"has_seen_quick_tour": true`. It will never interrupt an existing session or DAW project reload.

> [!TIP]
> **Zero Friction Dismissal**: The card can be dismissed with a single click anywhere outside the modal, pressing `ESC`, or clicking `[ Got it, Let's Play ]`.

---

## 3. Proposed Changes

### Component 1: Data-Driven Content Schema
Author the tour layout, headers, and bullet copy into `assets/controls/global_ui.json` under `"quick_tour"`:
```json
"quick_tour": {
  "title": "WELCOME TO THE KLANG FARMER",
  "subtitle": "Pro Tip: Right-clicking on controls is the way and the light!",
  "panels": [
    {
      "icon": "🖱️",
      "heading": "Right-Click Knobs & Sliders",
      "body": "Opens Quick-Snap intervals (octaves, fifths, 0%, 50%, 100%) and hardware MIDI CC Learn."
    },
    {
      "icon": "⚡",
      "heading": "Right-Click Selectors & Cards",
      "body": "Opens full dropdown menus, default resets, and the d6 Randomizer depth menu (5% jitter to 100% chaos)."
    },
    {
      "icon": "🎛️",
      "heading": "Right-Click Header Badges",
      "body": "Right-click the Voice badge for Gated Bass & Glide; right-click [A|B] to copy states."
    },
    {
      "icon": "🎯",
      "heading": "Double-Click Anywhere",
      "body": "Double-clicking any parameter resets it to its factory default value instantly."
    }
  ],
  "checkbox_text": "Don't show this tour automatically on launch",
  "dismiss_button": "GOT IT, LET'S PLAY"
}
```

### Component 2: `QuickTourModalComponent` (`source/QuickTourModal.h` & `.cpp`)
Create a shared, lightweight JUCE component inherited from `juce::Component`:
- **Chassis & Styling**: Soft darkened vignette backdrop (`Colour(0xd00a0d14)`), sleek beveled card border matching active theme palette (`Cykranosh`, `Dracula`, `Cyberpunk`), and subtle glowing accent banner.
- **Interactive Checkbox**: `juce::ToggleButton dontShowAgainToggle;` (pre-checked).
- **Dismiss Button**: High-contrast, tactile `[ GOT IT, LET'S PLAY ]` button.
- **Keyboard Handling**: Pressing `ESC` or `Enter` closes the modal immediately.

### Component 3: `KlangCoreEditor` Integration (`source/KlangCoreEditor.h` & `.cpp`)
- Add `std::unique_ptr<QuickTourModalComponent> quickTourModal;` to `KlangCoreEditor`.
- In `KlangCoreEditor` constructor:
  - Check `SettingsManager::getInstance().hasSeenQuickTour()`.
  - If `false`, display `quickTourModal` and bring to front.
- Wire header `guideButton` (`[ ? ]` icon) to toggle `quickTourModal->setVisible(true)`.

---

## 4. Verification Plan

### Automated Tests
- Run `gui_tests` to verify synthetic mouse clicks on `[ GOT IT, LET'S PLAY ]` and background dismissal close the modal cleanly.
- Verify `SettingsManager` unit test toggles `has_seen_quick_tour` flag on disk.

### Manual Verification
1. Launch Standalone or VST3 in DAW for the first time: Quick Tour modal appears smoothly.
2. Verify clicking outside or hitting `ESC` dismisses the card.
3. Reload plugin: Verify the tour does NOT appear on second launch.
4. Click `[ ? ]` in header: Verify the tour reopens instantly for review.
