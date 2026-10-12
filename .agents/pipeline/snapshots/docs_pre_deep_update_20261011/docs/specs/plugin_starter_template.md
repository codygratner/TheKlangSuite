# The Klang Boilerplate: Production-Grade C++20 / JUCE 9 Audio Plugin Starter Template

**Target Milestone**: Post-v1.0.0 Refactor & Open-Source Tooling  
**Repository Name (Proposed)**: `the-klang-boilerplate` (or `juce-modern-plugin-template`)  
**License**: Free & Open-Source (GPLv3 / MIT Dual-Option)  
**Toolchain**: C++20, JUCE 9.0.3, CMake 3.24+, GitHub Actions  

---

## Executive Summary

During the development of *The Klang Farmer* and *The Klang Planter*, our architecture evolved through rigorous trials: transitioning from hardcoded JUCE prototypes to a battle-tested **Data-Driven JSON Architecture**, enforcing **Zero-Allocation Audio Thread Invariants**, implementing **Headless Dynamic Reflection GUI Testing (`gui_tests`)**, solving **JUCE 9.0.3 Timer Hygiene (`#1696`)**, and creating a dynamic **JSON-driven Theme System**.

This plan outlines the extraction of those core architectural breakthroughs into a standalone, pristine starter template repository. Any sound designer, DSP researcher, or audio engineer will be able to clone the repo, run a single setup command, and immediately begin authoring new synthesizers, drum machines, or boutique multi-effects with enterprise-grade stability out of the box.

```
                              ┌──────────────────────────────────┐
                              │     The Klang Boilerplate        │
                              │   GitHub Template Repository     │
                              └────────────────┬─────────────────┘
                                               │
             ┌─────────────────────────────────┼─────────────────────────────────┐
             ▼                                 ▼                                 ▼
┌───────────────────────────┐     ┌───────────────────────────┐     ┌───────────────────────────┐
│     Real-Time DSP Core    │     │   Data-Driven JSON Engine │     │   Headless Test Suite     │
│ - Zero-allocation loops   │     │ - assets/controls/ params │     │ - gui_tests reflection    │
│ - TbdAudio::FastMath      │     │ - assets/layouts/ cards   │     │ - dsp_tests audio safety  │
│ - Lock-free FIFOs         │     │ - assets/themes/ palettes │     │ - Zero-display CI runners │
└───────────────────────────┘     └───────────────────────────┘     └───────────────────────────┘
```

---

## Core Pillars & Architectural Features

### 1. Zero-Allocation Real-Time C++20 DSP Core
- **Audio Thread Guardrails**: Pre-allocated buffers in `prepareToPlay()`; strictly zero `new`, `malloc`, or dynamic vector resizing in `processBlock()`.
- **FastMath Transcendentals**: Integrated `TbdAudio::FastMath` library providing vectorized approximations for `pow`, `sin`, `cos`, `exp`, and `tanh` saturators.
- **Lock-Free Communication**: Lock-free single-producer single-consumer (SPSC) ring buffers and atomic state passing for UI visualizers, pitch meters, and oscilloscope feeds.
- **Decoupled Architecture**: Clean separation between pure C++ DSP blocks (`TbdAudio::DSPBlock`) and JUCE's `AudioProcessor` shell, making the audio code instantly portable to embedded platforms (Daisy Seed, Teensy, dadamachines tbd-16).

### 2. Data-Driven JSON Single Source of Truth
- **Zero Hardcoded Parameters**: APVTS parameter IDs, display names, ranges, skew factors, default values, quick-snap intervals, and units are parsed dynamically from modular JSON files (`assets/controls/`).
- **Declarative Grid Layouts**: Card-based and page-based UI layouts (`assets/layouts/`) defined in JSON. Modifying knob positions, card dimensions, or module order requires zero C++ recompilation.
- **Compile-Time Bundling + Hot-Reload**: Automatically compiles default JSON schemas into binary resources for standalone distribution, while supporting hot-reload during development.

### 3. Automated Headless Dynamic Reflection Testing
- **100% Parameter Sweep (`gui_tests`)**: Headless test suite that sweeps every registered APVTS parameter against the JSON schema, verifying bindings, range limits, text conversions, and snap intervals without requiring an active audio device or display server.
- **Audio Safety Static/Runtime Audits (`dsp_tests`)**: Automated verification of denormal prevention, NaN safety, and peak limiter clamping across buffer edge cases.
- **Continuous Integration Ready**: Runs seamlessly on headless Linux (`Xvfb`), macOS, and Windows GitHub Actions runners.

### 4. Boutique UI & Dynamic Theme Engine
- **JSON Theme Palettes**: Instant live theme switching across curated color schemes (e.g. *Cyberpunk Neon*, *Cykranosh Slate Navy*, *Dracula*, *Monokai Pro*, *Nord*).
- **Modern Vector UI**: Pure vector rendering with zero raster artifacts across 100% to 200% High-DPI scaling.
- **Tactile Depth Shading**: Procedural industrial drop shadows, recessed card bezels, and calibrated LED ballistics.
- **JUCE 9 Timer Hygiene**: Built-in destructors with mandatory `stopTimer()` protection to eliminate teardown crashes.

### 5. Multi-Platform Build & Packaging Pipeline
- **Clean CMake Scaffolding**: Structured for VST3, AU, CLAP, and Standalone targets.
- **Automated CI/CD**: Matrix builds on GitHub Actions testing Ubuntu, macOS (Universal `arm64` + `x86_64`), and Windows.
- **Installer Automation**: Scripts for Windows InnoSetup and macOS `.pkg` installer generation.

---

## Proposed Project Directory Layout

```text
the-klang-boilerplate/
├── .github/
│   └── workflows/
│       ├── build-and-test.yml       # Headless CI: builds all targets & executes test suites
│       └── release.yml              # Tagged release builder: creates installers & zips
├── assets/
│   ├── controls/                    # Data-driven parameter definitions
│   │   ├── global_settings.json
│   │   └── module_synth.json
│   ├── layouts/                     # Declarative UI schemas (cards, grids, pages)
│   │   └── default_layout.json
│   └── themes/                      # Curated color palettes
│       ├── cyberpunk_neon.json
│       ├── cykranosh.json
│       └── dracula.json
├── cmake/
│   ├── CompilerWarnings.cmake       # Strict warnings (-Wall -Wextra -Werror)
│   ├── JuceConfiguration.cmake      # JUCE 9.0.3 fetch & configuration
│   └── Sanitizers.cmake             # ASan / TSan / UBSan configs
├── docs/
│   ├── ARCHITECTURE.md              # Detailed guide on JSON + DSP patterns
│   └── GETTING_STARTED.md           # Developer onboarding & quickstart
├── source/
│   ├── dsp/
│   │   ├── FastMath.h               # Vectorized transcendental math
│   │   ├── SynthVoice.h             # Clean polyphonic/monophonic voice archetype
│   │   └── SafeAudioRingBuffer.h    # Lock-free SPSC queue
│   ├── parameters/
│   │   ├── ParameterManager.h       # JSON-to-APVTS reflection parser
│   │   └── ParameterTypes.h         # Strongly-typed parameter descriptors
│   ├── ui/
│   │   ├── components/              # Tactile knobs, cards, switches, meters
│   │   ├── theme/                   # Dynamic ThemeManager & color lookup
│   │   ├── BoilerplateEditor.h      # Generic grid/card host editor
│   │   └── BoilerplateEditor.cpp
│   ├── BoilerplateProcessor.h       # JUCE AudioProcessor shell
│   ├── BoilerplateProcessor.cpp
│   └── Main.cpp
├── test/
│   ├── dsp_tests.cpp                # Real-time safety & math validation
│   ├── gui_tests.cpp                # Dynamic reflection & parameter binding tests
│   └── test_helpers.h
├── tools/
│   └── init_plugin.py               # Wizard to customize name, company, bundle ID
├── .gitignore
├── CMakeLists.txt
├── LICENSE                          # GPLv3 / Permissive Dual License
└── README.md
```

---

## Detailed Implementation Roadmap (Post-1.0)

### Phase 1: Repository Scaffolding & Core Extraction
- [ ] Create repository `the-klang-boilerplate` on GitHub.
- [ ] Extract clean, generalized copies of:
  - `source/dsp/FastMath.h`
  - `source/parameters/ParameterManager.cpp/.h` (JSON parser & dynamic APVTS registrar)
  - `source/ui/theme/ThemeManager.cpp/.h`
  - `source/ui/components/TactileKnobComponent.cpp/.h`
  - `source/ui/components/ModuleCardComponent.cpp/.h`
- [ ] Author a minimal demonstration synthesizer/effect (e.g. A dual-oscillator resonant filter module with LFO).

### Phase 2: Headless Test Suite & Reflection Parity
- [ ] Extract `test/gui_tests.cpp` to run headless reflection sweeps on any valid JSON directory.
- [ ] Extract `test/dsp_tests.cpp` with standard checks for audio safety, denormals, and zero-allocation compliance.
- [ ] Configure CTest integration for one-command test execution (`ctest --output-on-failure`).

### Phase 3: Developer Onboarding Wizard (`init_plugin.py`)
- [ ] Author an interactive Python/Bash setup wizard:
  ```bash
  python tools/init_plugin.py \
    --name "AetherVerb" \
    --company "KlangWorks" \
    --code "AthV" \
    --type "Effect" \
    --prefix "AV"
  ```
- [ ] Automatically rename:
  - CMake project and target names.
  - C++ classes (`BoilerplateProcessor` $\to$ `AetherVerbProcessor`).
  - Bundle identifiers (`com.KlangWorks.AetherVerb`).
  - JSON parameter namespaces.

### Phase 4: CI/CD Workflows & Multi-Platform Packaging
- [ ] Implement GitHub Actions matrix workflow (Windows, macOS, Ubuntu).
- [ ] Configure headless test validation in CI before allowing release builds.
- [ ] Add basic InnoSetup (Windows) and `.pkg` (macOS) workflow templates.

### Phase 5: Documentation & Public Release
- [ ] Write comprehensive `README.md` with GIFs and architecture diagrams.
- [ ] Author `ARCHITECTURE.md` explaining the "Data-Driven JSON + Zero-Allocation C++20" philosophy.
- [ ] Release under GitHub's "Template Repository" feature for instant 1-click project initialization.

---

## Guardrails & Non-Goals

> [!IMPORTANT]
> - **Post-1.0 Priority**: Under no circumstances will this extraction take precedence over reaching the v1.0.0 General Availability release of *The Klang Farmer* and *The Klang Planter*.
> - **Read-Only / No Premature Refactoring**: No production source code in the main repository will be gutted, broken, or destabilized to build the template ahead of schedule.
> - **Self-Contained**: The template repository will have zero proprietary or machine-specific dependencies, relying solely on JUCE 9 and modern CMake.
