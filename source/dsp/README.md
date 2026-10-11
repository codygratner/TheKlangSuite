# 🎛️ Zone 1: Native Audio DSP & Real-Time Engine
> **Architectural Zone:** Zone 1 (Native Audio DSP)  
> **Facility Owner:** Klang Industries (Factory Floor) 🏭  
> **Target Standard:** C++20 / JUCE 9.0.3  
> **Core References:** [`docs/GLOSSARY.md`](../../docs/GLOSSARY.md) · [`docs/CHEAT_SHEET.md`](../../docs/CHEAT_SHEET.md) · [`GEMINI.md`](../../GEMINI.md)  

---

## ⚡ Real-Time Audio Thread Invariants (CRITICAL)

All code executing in `Source/dsp/`, `processBlock()`, `processStereo()`, or per-sample voice render loops MUST rigidly adhere to these non-negotiable real-time safety rules:

1. **Zero Allocations**:
   - Never call `new`, `malloc`, `free`, or resize dynamic containers (`std::vector::push_back`, `juce::Array::add`, `std::string` concatenation) inside audio render paths.
   - All buffers, wavetables, voice structures, and delay lines MUST be pre-allocated in `prepareToPlay()`.
2. **Zero Locks**:
   - Never acquire a `std::mutex`, `std::lock_guard`, `juce::CriticalSection`, or wait on thread synchronization primitives in the audio path.
   - Use lock-free atomics (`std::atomic<float>`, `std::atomic<bool>`) or bounded Single-Producer Single-Consumer (SPSC) ring-buffer FIFOs for audio-to-UI messaging.
3. **Zero Blocking I/O**:
   - Never call filesystem operations, network APIs, or logging/console output (`std::cout`, `printf`, `DBG()`, `juce::Logger`) on the audio thread.
4. **SIMD & Fast Math**:
   - Prefer `TbdAudio::FastMath` over standard CRT transcendentals (`std::pow`, `std::sin`, `std::tanh`) in hot audio loops.

---

## 🧭 Zonal Governance & Anti-Pattern Prohibition

* **No Rogue Per-Folder Blackboxes**: Real-time DSP contracts are defined here at the root of `Source/dsp/`. Do not create scattered, nested READMEs inside sub-folders.
* **Separation from Tooling**: Pure dev scripts, HTML sidecars, and Python runners belong strictly in Zone 3 (`tools/`). Never import audio DSP classes into tooling scripts.
