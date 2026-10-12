# Headless GUI Testing & Synthetic Component Validation

> **Authority:** New Klang City Architecture Standard  
> **Target:** `test/gui_tests.cpp`, `test/PluginIntensiveTestSuite.h`, `test/HardeningSuites.h`

---

## 1. Executive Summary

Graphical user interfaces in C++ audio plugins are notoriously difficult to test automatically. Most audio teams rely on manual mouse clicking, leading to slow release cycles and frequent visual regressions. 

**The Klang Suite** features a **single-binary headless functional GUI testing harness (`gui_tests`)** that validates 100% of UI components, page layouts, modal dialogs, and APVTS bindings in milliseconds without opening physical desktop windows. In Linux CI/CD, it operates headlessly via a virtual framebuffer (`Xvfb`).

---

## 2. Testing Pillars

### 1. Dynamic Parameter Reflection (6-Pillar Reflection Suite)
- Sweeps 100% of `processor.getParameters()` for both plugins dynamically without hardcoded ID lists.
- **Normalization Roundtrip:** Asserts `convertTo0to1(convertFrom0to1(x)) == x` across $[0.0, 0.1, 0.25, 0.5, 0.75, 0.9, 1.0]$.
- **Boundary Clamping:** Tests $-1.0$ and $+2.0$ inputs to verify safe clamping with zero NaN leaks.
- **State Serialization:** XML/MemoryBlock save $\to$ parameter randomization $\to$ restore $\to$ assert 100% restoration parity.

### 2. Deep Component Binding Audit
- Scans the JUCE visual tree (`Component::findChildWithID`) for every registered card and control:
  - Asserts 100% of interactive sliders are bound to valid APVTS parameters.
  - Asserts 100% of controls display non-empty, localized tooltips.
  - Verifies that zero controls have collapsed or zero-width coordinate bounds.

### 3. Synthetic Event Simulation
- Simulates user gestures headlessly:
  - Clicks, double-clicks (reset to default), drags, and wheel events.
  - Right-click callout popover triggers and menu selections.
  - Page navigation and tab transitions.

### 4. Offscreen Paint Smoke Passes
- Renders the entire plugin UI offscreen using `Component::paintEntireComponent()` across multiple resolutions ($800\times 600$, $1000\times 750$, and $4\text{K}$).
- Verifies that zero divide-by-zero, clipping faults, or null pointer dereferences occur during paint operations.

### 5. Lifecycle Stress & Memory Safety
- Instantiates, initializes, renders, and destroys the graphical editor 10 times in a rapid loop.
- Catches dangling event listeners, unfreed memory, and asynchronous timer leaks before host unload.
