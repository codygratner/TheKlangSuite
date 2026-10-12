# Factory Blueprint: Full Rebuild, Automated Test Suite & Local VST3 Deployment

> **TARGET:** FACTORY_FLOOR · Task `factory-rebuild-and-local-deploy`  
> **Active Model Setting:** Gemini 3.8 Flash (Thinking: High) — 🟢 SUSTAINABLE  
> **Execution Strategy:** CMake Release Build $\rightarrow$ Automated Test Execution $\rightarrow$ Local VST3 Deployment  
> **Status:** COMPLETED ✅  

## Goal
Execute a clean, synchronized multi-target Release build of The Klang Suite, verify 100% test passing across `dsp_tests` and `gui_tests`, deploy fresh VST3 artifacts to `C:\Program Files\Common Files\VST3\` and `current_build/`, and update build telemetry.

---

## 📋 Phase Breakdown

### Phase 1: CMake Reconfigure & Multi-Target Release Build
- [x] Run CMake reconfigure to catch any recent asset/schema updates:
  ```powershell
  cmake -B build
  ```
- [x] Execute parallel Release build across all factory targets:
  ```powershell
  cmake --build build --config Release --parallel --target TheKlangFarmer_VST3 TheKlangPlanter_VST3 TheKlangEditor dsp_tests gui_tests TheKlangFarmer_Standalone TheKlangPlanter_Standalone
  ```
- [x] Verify 0 compilation errors and 0 linker errors.

### Phase 2: Automated Test Gauntlet Verification
- [x] Execute native DSP unit test suite:
  ```powershell
  .\build\Release\dsp_tests.exe
  ```
  *(Verify 100% pass across audio thread safety, SIMD FastMath, and parameter bounds).*
- [x] Execute native GUI unit test suite:
  ```powershell
  .\build\Release\gui_tests.exe
  ```
  *(Verify 100% pass across APVTS bindings, layout JSON parsing, and component hierarchies).*

### Phase 3: Local Artifact Deployment & System Installation
- [x] Execute root deployment script:
  ```powershell
  powershell -ExecutionPolicy Bypass -File .\deploy.ps1
  ```
- [x] Verify binaries copied to system VST3 directory:
  - `C:\Program Files\Common Files\VST3\The Klang Farmer.vst3`
  - `C:\Program Files\Common Files\VST3\The Klang Planter.vst3`
- [x] Verify binaries copied to repository `current_build/`:
  - `current_build/VST3/The Klang Farmer.vst3`
  - `current_build/VST3/The Klang Planter.vst3`
  - `current_build/Standalone/The Klang Farmer.exe`
  - `current_build/Standalone/The Klang Planter.exe`
  - `current_build/Editor/The Klang Editor.exe`

### Phase 4: Factory Telemetry & Completion Handshake
- [x] Update `.agents/pipeline/communique/build_to_plan.md` to `Status: COMPLETED ✅` with test metrics and timestamps.
- [x] Update `.agents/pipeline/communique/plan_to_build.md` to `STATUS: COMPLETED`.
- [x] Sound the "JOB'S DONE" chime for New Klang City.
