# The Klang Suite — Claude Engineering & DSP Directives

## Workspace Identity & Mission
You are operating inside **The Klang Suite** (`C:\Dev\TheKlangSuite`), the flagship professional C++20 audio plugin suite, DSP library (`TheKlangDSP`), and tracker ecosystem (`ToadTracker`).

## Absolute DSP & Audio Thread Invariants
1. **Zero Heap Allocation**:
   - Strictly ZERO dynamic heap allocations (`new`, `malloc`, `std::make_shared`, `std::vector` push/resize, dynamic `std::string`) inside any audio callback (`processBlock`, `render`, `processVoice`).
   - All buffers must be pre-allocated during `prepareToPlay` or statically sized.
2. **Zero Blocking Locks**:
   - Strictly ZERO mutexes, spinlocks, or blocking synchronization primitives on the audio thread. Use lock-free atomics, single-producer single-consumer (SPSC) ring buffers, or double-buffered parameter states.
3. **Zero I/O & Diagnostics**:
   - NO file I/O, network sockets, or console printing (`std::cout`, `printf`, `DBG`, `Logger`) on the audio path.
4. **Denormal & Transcendental Protection**:
   - Flush-to-zero / denormals-are-zero (FTZ/DAZ) must be enabled or handled with dc offset/denormal prevention.
   - Transcendentals (`std::sin`, `std::exp`, `std::tanh`) must be evaluated with vectorized SIMD approximations (`FastMath`) or wavetable lookup where appropriate.
5. **Universal Portability**:
   - Strictly portable C++20 standard across Windows (MSVC), macOS (Clang), Linux (GCC), Steam Deck, and ARM/Raspberry Pi.
   - Wrap architecture-specific intrinsics (AVX2, NEON) inside `TheKlangDSP/simd/` abstractions.

## Coding Style & Workflow
- Use JUCE 9.0.3 standards where applicable.
- Keep comments strictly relevant to signal flow, transfer functions, and mathematical invariants.
- Prefer surgical diffs over entire file rewrites.
