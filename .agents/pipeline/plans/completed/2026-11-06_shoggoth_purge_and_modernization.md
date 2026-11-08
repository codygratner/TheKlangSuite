# Implementation Plan — The Great Purge & Shoggoth Modernization

## Objectives
1. **The Great Purge (Source & Build System)**: Eradicate all pre-Hastur legacy code (`TheKlangFarmer`, `TheKlangPlanter`, `TheShard`, `TheKlangEditor`, `UIComponents.h/.cpp`, `ModularBlocks.h`, and `TkfAssets`) from `TheKlangSuite`.
2. **The VST3 Folder Cleanse**: Delete `Crucible Workbench.vst3`, `The Klang Farmer.vst3`, `The Klang Planter.vst3`, `The Shard.vst3`, and old `.bak` cruft from `C:\Program Files\Common Files\VST3\`, `current_build\VST3\`, and `artifacts\VST3\`.
3. **Shoggoth Modernization**: Upgrade `Shoggoth` to 100% Cracked Chimes fleet compliance:
   - Modern `CrackchimesLookAndFeel` (top-mounted labels, Kilohearts in-dial value badges, no legacy widgets).
   - `RockerToggle3` Triad Lifeform control (`SYNTHETIC` #00C8FF, `CYBORG` #E08C00, `ORGANIC` #6BBF59) bound to `life_mode`.
   - Universal VST3 Preset Discovery in `PresetManager.cpp` (resolving VST3 bundle resources in any DAW host).
   - `StepperAuditionBar` with 10 themed bank sub-menus and `<` / `>` patch cycling.
4. **Tower Build & Verification**: Build and test the clean modern fleet (**Hastur**, **Formless Spawn**, **DigiStrike FS2**, **Nyarlat**, **Shoggoth**).

---

## Phase 1: Pre-Purge Decoupling & Shoggoth Modernization
- [x] 1.1 In `source/PresetManager.cpp`:
  - Upgrade `getFactoryPresetsDirectory()` with multi-tier VST3 bundle, Common Files, and dev tree resolution.
- [x] 1.2 In `source/shoggoth/ShoggothEditor.h` & `ShoggothEditor.cpp`:
  - Remove `#include "UIComponents.h"` and `RotaryKnobSlider`.
  - Implement `ShoggothLookAndFeel` with `drawRotarySlider` and `drawKiloheartsValueBadge`.
  - Implement top-mounted bold labels for all rotary controls with clean engineering unit formatters.
  - Implement `RockerToggle3` for `life_mode` (`SYNTHETIC` #00C8FF, `CYBORG` #E08C00, `ORGANIC` #6BBF59).
  - Upgrade preset navigation to `StepperAuditionBar` with 10 themed bank sub-menus and `<` / `>` stepping.
- [x] 1.3 Move `source/DSPBlock.h` to `source/dsp/common/DSPBlock.h` and update `source/dsp/common/AudioEffect.h`.

## Phase 2: The Great Legacy Source Purge
- [x] 2.1 Delete legacy plugin source files:
  - Farmer: `source/FarmerProcessor.*`, `source/FarmerEditor.*`
  - Planter: `source/PlanterProcessor.*`, `source/PlanterEditor.*`, `source/PlanterEngine.h`
  - Editor: `source/KlangCoreProcessor.*`, `source/KlangCoreEditor.*`
  - Shard: `source/ShardEditor.*`, `source/ShardEngine.h`
- [x] 2.2 Delete legacy UI & block monoliths:
  - `source/UIComponents.h`, `source/UIComponents.cpp`
  - `source/ModularBlocks.h`
  - `source/ModulationEngine.h`
  - `source/ParameterManager.*`
  - `source/SettingsModal.*`
  - `source/DevLogger.h`
  - `source/legacy_aliases.h`
  - `source/StateMigrator.h`
  - `source/VersionChecker.*`
- [x] 2.3 Delete legacy assets:
  - `assets/controls/`, `assets/themes/`, `assets/text/`

## Phase 3: VST3 Installed Folder Cleanse
- [x] 3.1 From `C:\Program Files\Common Files\VST3\`:
  - Delete `Crucible Workbench.vst3`
  - Delete `The Klang Farmer.vst3`
  - Delete `The Klang Planter.vst3`
  - Delete `The Shard.vst3`
  - Delete `DigiStrike FS2.vst3.pre_klang_bak`
- [x] 3.2 From `C:\Dev\TheKlangSuite\current_build\VST3\` and `artifacts\VST3\`:
  - Clean out matching legacy binaries.

## Phase 4: CMakeLists.txt & dsp_tests.cpp Consolidation
- [x] 4.1 In `CMakeLists.txt`:
  - Remove targets `TheKlangFarmer`, `TheKlangPlanter`, `TheShard`, `TheKlangEditor`, `TkfAssets`, and `capture_planter_screenshot`.
  - Streamline project to build only the Active Fleet: `Hastur`, `FormlessSpawn`, `DigiStrikeFS2`, `Nyarlat`, `Shoggoth`, and test/preview runners.
- [x] 4.2 In `test/dsp_tests.cpp`:
  - Remove `#include "PlanterEngine.h"` and `#include "ModularBlocks.h"`.
  - Prune the legacy planter engine tests.
  - Add foreign-host VST3 preset discovery test to Suite 79.

## Phase 5: Tower Build, Packaging, Live UI Proof & Harvest
- [x] 5.1 Build `dsp_tests` on The Tower and verify all modern test suites pass 100%.
- [x] 5.2 Build `Shoggoth_All` and `Shoggoth_Preview` on The Tower.
- [x] 5.3 Deploy `Shoggoth.vst3` bundle (with 480 presets) to `C:\Program Files\Common Files\VST3\Shoggoth.vst3` and `current_build\VST3\Shoggoth.vst3`.
- [x] 5.4 Render live offscreen UI proof `shoggoth_ui_proof.png` and verify all 5 visual gates.
- [x] 5.5 Execute `plan_manager.py finish` and emit silent harvest confirmation receipt.
