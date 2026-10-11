## 📝 Pull Request Description
<!-- Provide a clear, concise summary of the changes and the architectural rationale. -->

### 🎯 Intent & Motivation
- **What problem does this PR solve?**
- **Affected Subsystems:** `[ ] DSP` `[ ] UI` `[ ] Assets/JSON` `[ ] Tooling` `[ ] Docs`

---

## 🛡️ Anti-Slop & Quality Gate Checklist
*Please confirm and check every box below before requesting review:*

### ⚡ Real-Time Audio Safety (If modifying DSP / `Source/dsp/`)
- [ ] **Zero Allocs**: No `new`, `malloc`, `std::vector::push_back`, or dynamic string formatting in `processBlock()` / per-sample loops.
- [ ] **Zero Locks**: No `std::mutex`, `std::lock_guard`, `juce::CriticalSection`, or sleep calls in audio path.
- [ ] **Zero Blocking I/O**: No `std::cout`, `printf`, `DBG()`, or filesystem access in audio path.
- [ ] **SIMD & FastMath**: CRT transcendentals replaced with `TbdAudio::FastMath`.

### 🏗️ Schema & Architecture Isolation (If modifying `assets/`)
- [ ] **DSP Schema Purity**: No colors, styles, widths, or copy added to `assets/controls/*.json`.
- [ ] **No Hardcoded Parameters**: APVTS parameters match JSON schema contracts 1:1.
- [ ] **Dynamic Reflection**: New controls are registered in layouts and bound in `Editor.cpp`.

### 🧹 Code Cleanliness & Anti-Slop
- [ ] **The Boy Scout Rule**: Touched files are cleaner than before.
- [ ] **Zero AI Slop**: No decorative comment banners (`/* ===== */`) or emojis in C++ source files.
- [ ] **No Tautological Comments**: Comments explain *why*, math, or lifecycles—not obvious syntax.
- [ ] **No Dead Code**: Legacy or commented-out code blocks have been purged.

### 📜 Legal & Provenance
- [ ] **Provenance Docblocks**: Included for any adapted external algorithms with upstream URLs and licenses.
- [ ] **Universal SPDX Header**: `// SPDX-License-Identifier: GPL-3.0-or-later` present on all new source files.

### 🧪 Verification
- [ ] `Release/dsp_tests.exe` passes 100% locally.
- [ ] `Release/gui_tests.exe` passes 100% locally (parameter reflection & schema audit).
- [ ] `python tools/audit_audiothread.py` passes with 0 warnings.
