# Contributing to The Klang Suite 🎛️

Thank you for your interest in contributing to **The Klang Suite**! We build high-performance, polyphonic synthesizer plugins (VST3/AU) with JUCE 9.0.3 and C++20.

We take engineering rigor, real-time safety, and code elegance extremely seriously. Whether you are a human engineer, an AI-assisted developer, or an autonomous agent, **all contributions are held to the exact same uncompromising standards.**

---

## 🚫 Zero-Tolerance for AI Slop & The Boy Scout Rule

We practice **Negative Architecture**: removing cognitive noise, ungrounded abstractions, and boilerplate slop.
1. **The Boy Scout Rule**: Always leave a file cleaner, faster, and more readable than you found it.
2. **Zero Decorative Banners**: Never use decorative ASCII box-drawing banners (e.g. `/* ================== SECTION ================== */`) or emojis inside C++ source files.
3. **No Tautological Comments**: Never write comments that merely restate code (e.g., `// Returns the cutoff frequency` above `getCutoffFrequency()`). Comments must explain **why**, the mathematical rationale, or non-obvious lifecycle constraints.
4. **No Dead or Commented-Out Code**: Ruthlessly delete obsolete functions, commented-out blocks, and legacy experiments. *Git is our archive.*
5. **No Fake Metrics**: Never claim unverified performance stats or latency figures. Performance must be grounded in cycle-accurate DSP benchmarks (`test/benchmark_dsp.cpp`).

---

## ⚡ Real-Time Audio Thread Invariants (CRITICAL)

The DSP render thread is non-negotiable. Code executed inside `processBlock()`, `processStereo()`, or sample-rate render loops must obey:

1. **Zero Allocations**: Never call `new`, `delete`, `malloc`, `free`, or resize dynamic containers (`std::vector::push_back`, `juce::Array::add`, `std::string` concatenation) in the audio path. All memory buffers must be pre-allocated in `prepareToPlay()`.
2. **Zero Locks**: Never acquire a `std::mutex`, `std::lock_guard`, `juce::CriticalSection`, or wait on thread synchronization primitives in the audio path. Use lock-free atomics (`std::atomic`) or bounded FIFO queues for audio-to-UI messaging.
3. **Zero Blocking I/O**: Never call filesystem operations, network APIs, or logging/console output (`std::cout`, `printf`, `DBG()`, `juce::Logger`) on the audio thread.
4. **SIMD & Fast Math**: Prefer `TbdAudio::FastMath` over standard CRT transcendentals (`std::pow`, `std::sin`, `std::tanh`) in hot audio loops.

*Automated verification: Run `python tools/audit_audiothread.py` before submitting.*

---

## 🏗️ Strict 4-Layer Schema Separation

Our UI and DSP parameter architectures are 100% data-driven. Never hardcode APVTS parameters or styling in C++. All JSON files must strictly adhere to their layer:

| Layer | Directory | Strict Scope | Prohibitions |
| :--- | :--- | :--- | :--- |
| **1. DSP Contracts** | `assets/controls/*.json` | Parameter IDs, min/max ranges, defaults, snap points, choices. | ❌ ZERO visual styling, colors, widths, or text copy. |
| **2. UI Layout** | `assets/layouts/*.json` | Surface hierarchy (Pages ➔ Cards ➔ Parameter ID bindings). | ❌ ZERO parameter bounds or colors. |
| **3. Visual Themes** | `assets/themes/*.json` | Palettes, typography, callout geometry (`theme.json`). | ❌ ZERO parameter ranges or labels. |
| **4. UI Text & Copy** | `assets/text/*.json` | Parameter descriptions, choice tooltips (`strings.json`). | ❌ ZERO DSP bounds or color definitions. |

*Automated verification: `test/ParameterSchemaAuditTest.h` (executed in `gui_tests`) asserts zero schema leakage and will hard-fail the build if violated.*

---

## 📜 FOSS Forever & Algorithmic Provenance

*The Klang Suite* is licensed under the **GNU General Public License v3.0 (GPLv3)**.
- **Copyleft Reciprocity**: Under no circumstances may proprietary, non-commercial (CC-BY-NC), or GPL-incompatible code be introduced.
- **Mandatory Provenance Docblocks**: Every adapted DSP algorithm, biquad filter formula, or third-party utility MUST include an inline provenance docblock:
  ```cpp
  /**
   * @brief [Algorithm Name / Functionality Description]
   * Adapted from: [Author / Project Name]
   * Source:        [Upstream URL / Paper / Repository]
   * Original License: [MIT / BSD-3-Clause / Apache 2.0 / Public Domain]
   * Modifications: [e.g. Vectorized with FastMath, adapted to 4-control interface]
   */
  ```
- **Universal SPDX Headers**: 100% of `.h`, `.cpp`, and `.cmake` files must begin with:
  `// SPDX-License-Identifier: GPL-3.0-or-later`
  `// Copyright (C) 2026 Cody Gratner & The Klang Suite Contributors`
- **Zero Third-Party Trademarks**: Never use proprietary trademark names in UI parameter labels, FX selectors, preset titles, or documentation. Use original agrarian or evocative descriptors (e.g. `THE MIST`, `MODAL RESONATOR`).

---

## 🧪 Testing & Verification Gauntlet

Every PR must pass the local regression gauntlet before review:
1. `build/bin/Release/dsp_tests.exe`: 100% audio thread safety, FastMath bounds, and SIMD tests.
2. `build/bin/Release/gui_tests.exe`: 100% APVTS dynamic parameter reflection and schema isolation tests.
3. `python tools/audit_audiothread.py`: Static analysis passes with 0 warnings.
4. `python tools/verify_context_clues.py`: All documentation hyperlinks resolve.

---

## 🌿 Git Commit & PR Workflow

- **Branching**: Never commit directly to `main` or `master`. Create a feature branch: `git checkout -b feat/<descriptive-name>`.
- **Conventional Commits**: Format commit messages strictly as:
  `<type>(<optional-scope>): <description>`  
  Valid types: `feat:`, `fix:`, `refactor:`, `perf:`, `test:`, `docs:`, `chore:`.
- **Pull Requests**: Open a PR using our [.github/PULL_REQUEST_TEMPLATE.md](.github/PULL_REQUEST_TEMPLATE.md) and ensure all checklist items are checked.
