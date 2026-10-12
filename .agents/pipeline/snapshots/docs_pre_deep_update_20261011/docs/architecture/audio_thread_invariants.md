# Real-Time Audio Thread Invariants

> **Authority:** New Klang City Architecture Standard  
> **Target:** `TheKlangFarmer`, `TheKlangPlanter`, `DSPBlock`, `ModularBlocks`

---

## 1. Executive Summary

In high-performance digital signal processing (DSP) and professional audio plugin development, the **audio rendering thread** is a hard real-time operating system thread. If the audio thread fails to deliver a block of audio samples before the hardware sound card buffer deadline (e.g. within $1.3\,\text{ms}$ at 64 samples @ $48\,\text{kHz}$), the operating system experiences a **buffer underrun**, heard by musicians and producers as an audible glitch, click, or pop.

To guarantee zero dropouts across all DAWs and sample rates, the codebase enforces five strict invariants.

---

## 2. The 5 Core Invariants

### Invariant 1: Zero Heap Allocations
- **Rule:** Never invoke `new`, `delete`, `malloc`, `free`, or resize dynamic containers inside `processBlock()`, `processStereo()`, or per-sample render loops.
- **Forbidden Calls:**
  - `std::vector::push_back`, `std::vector::resize`, `std::vector::insert`
  - `juce::Array::add`, `juce::String::operator+`, string formatting
  - `std::make_unique`, `std::make_shared`
- **Enforcement:** Pre-allocate all buffers, delay lines, and history states during `prepareToPlay()` / `init()`. Size buffers for worst-case parameters (e.g. maximum sample rate of $96\,\text{kHz}$).

### Invariant 2: Zero Blocking Locks & Thread Synchronization
- **Rule:** Never acquire a mutex, critical section, or wait on thread synchronization primitives in the audio path.
- **Forbidden Calls:**
  - `std::mutex::lock`, `std::lock_guard`, `std::unique_lock`
  - `juce::CriticalSection`, `juce::SpinLock`
  - `std::condition_variable::wait`
- **Enforcement:** Audio-to-UI communication must utilize lock-free atomics (`std::atomic<float>`), single-reader single-writer (SRSW) bounded FIFOs, or `juce::AsyncUpdater`.

### Invariant 3: Zero Blocking I/O & Logging
- **Rule:** Never perform filesystem operations, network requests, or console logging on the audio thread.
- **Forbidden Calls:**
  - `std::cout`, `printf`, `DBG()`, `juce::Logger::writeToLog`
  - File reading/writing (`juce::File`, `std::ifstream`, `std::ofstream`)
  - `TKS_LOG_*` (assertions in Debug builds prevent this)
- **Enforcement:** Diagnostics and parameter snapshots must be recorded on the message/UI thread or offloaded via a lock-free ring buffer to a worker thread.

### Invariant 4: SIMD & FastMath Acceleration
- **Rule:** Hot audio calculation loops must avoid standard CRT transcendentals and prefer hardware-accelerated FastMath.
- **Standard CRT Costs:**
  - `std::pow()`, `std::sin()`, `std::cos()`, `std::tanh()` can take 50–120 CPU cycles per call.
- **Approved FastMath Replacements:**
  - `TbdAudio::FastMath::fastTanh(x)` (Rational Padé approximation)
  - `TbdAudio::FastMath::fastSin(x)` / `fastCos(x)` (Parabolic polynomial approximation)
  - `TbdAudio::FastMath::fastExp(x)` / `fastPow2(x)` (Bitwise IEEE-754 mantissa manipulation)
  - JUCE 9 SIMD wrappers: `juce::dsp::SIMDRegister<float>`

### Invariant 5: Strict Thread Separation (Audio vs UI)
- **Rule:** Never trigger GUI repaints or manipulate UI component states directly from the audio thread.
- **Forbidden Calls:**
  - `component->repaint()`, `slider->setValue()`, `label->setText()`
- **Enforcement:** Components poll atomic parameters or visualizer peak values at a fixed display refresh rate (e.g., 30 Hz timer on the message thread).

---

## 3. Automated Guardrail Verification

Real-time audio invariants are continuously validated by our automated test harness:
1. **Static Analysis (`/audiothread-guard`):** Scans all git diffs and modified files in `source/` for allocations, locks, and I/O before builds.
2. **DSP Unit Tests (`test/dsp_tests.cpp`):** Asserts zero NaN/Inf leaks, parameter sweeps, and buffer clearing under intensive stress.
