# Preset Manager, Browser UI & State Migration

## Goal Description
Implement a professional, tag-based preset management system for `TheKlangFarmer`. The system will utilize JSON files to store preset data, complete with author metadata, tagging, and versioning. It includes a dedicated `StateMigrator` to guarantee backward compatibility of presets as the plugin's DSP architecture evolves, and a standalone UI browser for searching, filtering, and saving patches.

## User Review Required
> [!IMPORTANT]
> **Preset Storage Location**
> By standard convention, user presets should NOT be stored in the VST3 installation folder (which requires Admin rights). 
> They will be stored in OS-specific user directories:
> - **Windows:** `C:\Users\<User>\Documents\TbdAudio\TheKlangFarmer\Presets\`
> - **macOS:** `~/Music/Audio Music Apps/TbdAudio/TheKlangFarmer/Presets/` (or `~/Documents/...`)

## Proposed Changes

### 1. JSON Preset Schema & State Migration
Decouple preset saving from JUCE's raw XML `ValueTree` state, moving to a structured JSON format.

#### [NEW] Example Preset (`808_Sub.json`)
```json
{
  "metadata": {
    "name": "808 Sub Kick",
    "author": "Cody",
    "plugin_version": "0.3.0",
    "tags": ["Kick", "Bass", "Sub", "Clean"]
  },
  "state": {
    "carrier1_pitch": 0.25,
    "filter_cutoff": 0.40
    // ... all active parameters
  }
}
```

#### [NEW] `source/PresetManager.h` & `.cpp`
- **Background Scanner**: A `juce::TimeSliceThread` that recursively scans the preset directory, reading *only* the `"metadata"` block of each file to build a fast, searchable database (without loading the heavy APVTS state into memory).
- **Save/Load Logic**: Functions to serialize/deserialize APVTS normalized floats to/from the `"state"` block.

#### [NEW] `source/StateMigrator.h`
- A utility class that intercepts a loaded JSON state *before* it hits the APVTS.
- Compares `"plugin_version"` against the current version.
- **Migration Logic**: If loading an old preset that lacks newly added parameters (e.g., a newly added Drive knob), the Migrator injects the exact default value required to make the old patch sound identical to how it originally did, ensuring 100% backward compatibility for DAW sessions.

### 2. Preset Browser UI
A full-screen or modal overlay browser for patch selection.

#### [NEW] `source/PresetBrowserComponent.h` & `.cpp`
- **Layout**:
  - **Top Bar**: Search text input (fuzzy filtering).
  - **Left Column (Tags)**: A list of all unique tags found in the database (e.g., `Kick`, `Snare`, `FX`). Clicking a tag filters the right column.
  - **Right Column (Results)**: A `juce::ListBox` displaying matching presets, author names, and versions.
- **Interaction**: Single-click loads the preset (audition), double-click loads and closes the browser.

#### [NEW] `source/PresetSaveModal.h`
- A popup form with text fields for `Preset Name`, `Author`, and a tokenized input field for adding `Tags`.
- "Save" button writes the new JSON file to the user's OS directory and updates the `PresetManager` database.

### 3. Header Integration
Provide access to the browser from the main plugin UI.

#### [MODIFY] `source/UIComponents.cpp` & `source/PluginEditor.cpp`
- Add a central LCD-style `PresetNameDisplay` in the top header bar showing the currently loaded preset.
- Add `<` and `>` stepper buttons for quick next/prev preset loading.
- Clicking the LCD text opens the `PresetBrowserComponent` overlay.
- Add a `[Save]` disk icon to trigger the `PresetSaveModal`.

---

## Verification Plan

### Automated Tests
1. **Migration Unit Tests (`test/dsp_tests.cpp`)**: 
   - Construct a mock JSON preset mimicking an older version of the plugin (missing parameters).
   - Pass it through `StateMigrator`.
   - Assert that the newly added parameters default to "transparent" audio values, not APVTS defaults.

### Manual Verification
1. Open the standalone plugin.
2. Dial in a sound, click Save, add tags `["Test", "Percussion"]`, and verify it appears in the OS Documents folder as a JSON file.
3. Open the Browser, click the `Percussion` tag, and ensure the preset list filters correctly.
4. Type in the search bar and verify fuzzy text matching works on preset names.
