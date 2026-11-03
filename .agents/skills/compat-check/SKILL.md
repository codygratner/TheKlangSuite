---
name: compat-check
description: Audits CMake configuration, JUCE 9.0.3 hygiene, and cross-platform compilation flags across macOS, Linux, and Windows. Verifies deployment targets, static runtime linking, timer destructor safety (JUCE 9.0.3 issue #1696), headless build definitions, and portable path formatting. Triggers on `/compatcheck`, `/compat-check`, "check compatibility", "cross-platform check", "host api check", or "audit build flags".
---

# Cross-Platform & Host API Compatibility Checker

## Goal
Perform a pre-build or pre-release compatibility audit of `CMakeLists.txt` and C++ source files to verify cross-platform portability across macOS (Apple Silicon + Intel), Linux (cross-distro dynamic linking), and Windows, and enforce JUCE 9.0.3 safety invariants.

## Operational Constraints
- **Multi-OS Portability:** Ensure the plugin can build and load seamlessly on macOS 11.0+ (Universal `arm64;x86_64`), generic Linux distributions (Ubuntu, Fedora, Arch via static C++ runtimes), and Windows 10/11.
- **JUCE 9.0.3 Safety:** Prevent known JUCE 9.0.3 host crashes (specifically issue `#1696` `TimerThread` race condition on plugin unload).
- **Read-Only / Diagnostic:** Report findings and provide exact recommended fixes without making unintended modifications unless requested.

## Workflow

### 0. Automated Deterministic Audit (Primary Execution)
Execute the fast Python compatibility linter first. This deterministic tool inspects `CMakeLists.txt` and all C++ headers and source files in <150ms without reading dozens of files into context:
```bash
python tools/compat_check.py --terse
```
- **If Clean:** Outputs 1 token-shield line (`✅ Cross-Platform & JUCE 9.0.3: 0 violations across CMakeLists.txt and N source files [Xms]`). Output the summary report directly.
- **If Violations Detected:** Returns exact line numbers and offending declarations for surgical remediation.
- **Full Report Mode:** Run `python tools/compat_check.py` to generate the complete publication-grade Markdown report automatically.

### 1. CMake Cross-Platform Configuration Audit
Inspect [`CMakeLists.txt`](file:///c:/Dev/TheKlangSuite/CMakeLists.txt) for critical cross-platform declarations:

1. **macOS Deployment Target Placement:**
   - Must be declared **BEFORE** `project(TheKlangSuite ...)`:
     ```cmake
     if (APPLE)
         set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS deployment version" FORCE)
         if (NOT CMAKE_OSX_ARCHITECTURES)
             set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Universal binary architectures" FORCE)
         endif()
     endif()
     ```
   - *Why:* If set after `project()`, Apple Clang initializes the host SDK triple instead of macOS 11.0, breaking backward compatibility on older Macs.

2. **Linux Static Standard Library Linking:**
   - Verify static linking flags on Linux:
     ```cmake
     if (UNIX AND NOT APPLE)
         target_link_options(${target_name} PRIVATE -static-libstdc++ -static-libgcc)
     endif()
     ```
   - *Why:* Prevents `GLIBCXX` version mismatch errors when loading the `.vst3` on distributions with different libstdc++ versions than the build host.

3. **Headless & Container Compile Definitions:**
   - Verify compile definitions:
     - `JUCE_WEB_BROWSER=0`
     - `JUCE_USE_CURL=0`
     - `JUCE_VST3_CAN_REPLACE_VST2=0`
   - *Why:* Eliminates heavyweight dependencies on GTK, WebKit2GTK, and libcurl in headless CI, Docker containers, and minimal Linux audio systems.

4. **VST3 Auto Manifest Flag:**
   - Verify `VST3_AUTO_MANIFEST FALSE` on all plugin targets (`TheKlangFarmer`, `TheKlangPlanter`).
   - *Why:* Avoids `juce_vst3_helper` hanging or crashing during headless Linux builds or pre-signed macOS packaging.

### 2. JUCE 9.0.3 Timer Destructor Hygiene (`#1696`)
Inspect all classes inheriting from `juce::Timer` in `source/`:
- `TheKlangFarmerAudioProcessorEditor`
- `PlanterEditor`
- `VisualizerComponent`
- Any custom UI timer classes in [`source/UIComponents.h`](file:///c:/Dev/TheKlangSuite/source/UIComponents.h) and [`source/UIComponents.cpp`](file:///c:/Dev/TheKlangSuite/source/UIComponents.cpp).

**Audit Criterion:**
The destructor of every `juce::Timer` subclass must explicitly call `stopTimer()` as its **very first line**:
```cpp
TheKlangFarmerAudioProcessorEditor::~TheKlangFarmerAudioProcessorEditor()
{
    stopTimer(); // Essential: Guards against JUCE 9.0.3 issue #1696 TimerThread crash
    // ...
}
```
*Why:* In JUCE 9.0.3, a race condition exists where `TimerThread` can trigger a callback while an editor is in the middle of teardown during plugin unload, causing DAW crashes.

### 3. Portable Path & Inclusion Audit
Scan `source/` and `CMakeLists.txt` for common portability gotchas:
1. **Windows Backslash Paths:**
   - Ensure all `#include` directives use forward slashes `/`.
   - Ensure all CMake file paths and file references use `/`.
2. **Case Sensitivity:**
   - Ensure header inclusion casing exactly matches filesystem casing (e.g. `#include "ModularBlocks.h"` not `#include "modularblocks.h"`).
3. **Compiler Extensions:**
   - Verify any MSVC-specific pragmas or intrinsics (e.g. `__forceinline`, `_mm_malloc`) are guarded by `#if defined(_MSC_VER)` or replaced with portable equivalents (`inline`, standard aligned allocations).

### 4. Generate Compatibility Audit Report
Output a diagnostic status report:

```markdown
### 🌐 Cross-Platform Compatibility Audit Report

#### 1. CMake & Platform Targets
- [x] macOS 11.0 Deployment Target before `project()`: ✅ PASS
- [x] macOS Universal Binary (`arm64;x86_64`): ✅ PASS
- [x] Linux Static Runtime Linking (`-static-libstdc++ -static-libgcc`): ✅ PASS
- [x] Headless Flags (`JUCE_WEB_BROWSER=0`, `JUCE_USE_CURL=0`): ✅ PASS
- [x] VST3 Auto Manifest Disabled (`VST3_AUTO_MANIFEST FALSE`): ✅ PASS

#### 2. JUCE 9.0.3 Hygiene (Issue #1696)
- [x] `TheKlangFarmerAudioProcessorEditor::~TheKlangFarmerAudioProcessorEditor()`: `stopTimer()` present on line 1 ✅ PASS
- [x] `PlanterEditor::~PlanterEditor()`: `stopTimer()` present on line 1 ✅ PASS
- [x] `UIComponents.h` Timers: All destructors guarded ✅ PASS

#### 3. Source Portability
- [x] Forward slashes in all `#include` directives: ✅ PASS
- [x] Case-sensitive filename matching: ✅ PASS
- [x] Non-standard compiler intrinsics guarded: ✅ PASS

**Overall Status:** 🟢 All Cross-Platform & JUCE 9.0.3 checks PASSED.
```
