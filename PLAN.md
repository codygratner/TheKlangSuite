# Implementation Plan: DigiStrike FS2 Stuck Notes Elimination (Role & Lineage Switching)

## 🧭 Autoplan Frame: [TIER: MICRO] × [TRACK: DSP]
**Target:** `digistrike_fs2` (DigiStrike FS2 Sovereign Vintage PCM Workstation)  
**Origin:** Autoplan 3.0 ACF-5 Dual-Track Funnel (`00_frame.md` $\to$ `CUT` $\to$ `LOCK`)  
**Root Cause:** Rogue, unreleased `processor_.triggerVoice(60, 0.95f, 130.81f)` calls in `categoryGrid_.onSelect` and `lineageGrid_.onMaskChange`. These inject phantom Note-On events without Note-Off/Release, leaking gated sustain indefinitely, polluting the monophonic note stack (`noteStack_`), and violating thread safety across the JUCE message thread and audio thread.

---

### 📜 Build Execution Manifest

#### 1. Agentless Localization Scope (Xia et al.)
- **Primary Modification Target:**
  - `C:\Dev\TheKlangSuite\source\digistrike_fs2\DigiStrikeFS2Editor.cpp` (Lines 69, 98)
- **Read-Only Verification References:**
  - `C:\Dev\TheKlangSuite\contracts\digistrike_fs2.yaml`
  - `C:\Dev\TheKlangSuite\source\dsp\drums\DigiStrikeFS2Voice.h`
  - `C:\Dev\TheKlangSuite\source\digistrike_fs2\DigiStrikeFS2Processor.cpp`
  - `C:\Dev\TheKlangSuite\test\dsp_tests.cpp` (Suite 57)

#### 2. Immutable Symbol & Invariant Table (Plan-and-Solve+)
| Symbol / Location | Current Defect | Required Remediation | Locked Invariant ($I^*$) |
| :--- | :--- | :--- | :--- |
| `categoryGrid_.onSelect` (`DigiStrikeFS2Editor.cpp:66-70`) | Calls `processor_.triggerVoice(60, 0.95f, 130.81f)` | Remove `triggerVoice` call. Retain `param->setValueNotifyingHost(...)`. | Category button clicks MUST ONLY update APVTS; zero audio thread voice dispatch. |
| `lineageGrid_.onMaskChange` (`DigiStrikeFS2Editor.cpp:77-100`) | Calls `processor_.triggerVoice(60, 0.95f, 130.81f)` | Remove `triggerVoice` call. Retain lineage mask parameter notifications and inspector updates. | Lineage button toggles MUST ONLY update APVTS masks; zero audio thread voice dispatch. |
| `trigButton_` (`DigiStrikeFS2Editor.cpp:147-158`) | Paired manual trigger/release | Preserve as-is (`triggerManual` on click, `triggerVoiceRelease` on mouse-up). | Dedicated auditioning remains exclusively on the manual strike pad and incoming MIDI. |

---

### 🚀 Canonical 4-Phase Fast-Loop Sequence

### Phase 1: Contract & Parameter Audit (`40ms` Gate)
- [ ] Verify `contracts/digistrike_fs2.yaml` matches C++ APVTS parameter layouts with 0 drift.
- [ ] Run `python tools/context_bus_bridge.py fast-sweep` to assert codegen parity.

### Phase 2: Core DSP & Voice Dispatch Regression Verification (`~3s` Gate)
- [ ] Run native DSP test suite (`Suite 57: DigiStrikeFS2Voice Traversal Trinity & Choke Verification`) via `context_bus_bridge.py test`.
- [ ] Verify 100% pass across all 57 DSP test suites with zero regression in mono/glide note-stack priority, gated ADSR sustain, or hat choke.

### Phase 3: Surgical UI Fix & Spatial Verification (`~2s` Gate)
- [ ] Excise line 69 (`processor_.triggerVoice(60, 0.95f, 130.81f);`) from `categoryGrid_.onSelect` in `source/digistrike_fs2/DigiStrikeFS2Editor.cpp`.
- [ ] Excise line 98 (`processor_.triggerVoice(60, 0.95f, 130.81f);`) from `lineageGrid_.onMaskChange` in `source/digistrike_fs2/DigiStrikeFS2Editor.cpp`.
- [ ] Compile UI preview target: `python tools/context_bus_bridge.py build --target DigiStrikeFS2_Preview --dispatch`.
- [ ] Run headless spatial radar: `python tools/ui_gate.py --plugin digistrike_fs2 --pillars A,C`.
- [ ] Verify $\Omega = 0\text{px}$ non-intersection across all components and confirm visual proof generation.

### Phase 4: Final VST3 Packaging & Hot-Swap Deployment
- [ ] Compile release VST3 target: `python tools/context_bus_bridge.py build --target DigiStrikeFS2_VST3 --dispatch`.
- [ ] Deploy binary via `deploy_vst3.bat` to `C:\Program Files\Common Files\VST3\DigiStrike FS2.vst3`.
- [ ] Verify live window capture with `python tools/capture_ui.py "DigiStrike FS2"`.
- [ ] Execute `python tools/plan_manager.py finish` and update DevLog.
