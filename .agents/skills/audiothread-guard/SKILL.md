---
name: audiothread-guard
description: Performs a strict real-time audio-thread safety audit on DSP processing code, scanning modified files for heap allocations (new, malloc, dynamic vectors), blocking locks (std::mutex, CriticalSection), file/console I/O (std::cout, DBG), and unvectorized transcendentals. Triggers on `/audiothreadguard`, `/audiothread-guard`, "check audio safety", "audio thread audit", or "audit real-time safety".
---

# Audio-Thread Safety Guard & Real-Time Audit

## Goal
Audit audio-thread code paths (`process()`, `processBlock()`, `processStereo()`, and per-sample processing loops) across modified files or specific DSP modules to guarantee strict deterministic execution, preventing dropouts, priority inversions, and audio glitches.

## Operational Constraints
- **Zero Tolerance for Non-Determinism:** The audio thread must never allocate memory, take blocking locks, perform I/O, or execute unbounded loops.
- **Inspect Hot Paths Only:** Distinguish between initialization code (`init()`, `prepareToPlay()`, `trigger()`) and real-time processing code (`process()`, `processBlock()`, per-sample loops).

## Workflow

### 1. Identify Target Files
- If explicit files or directories are provided (e.g. `/audiothreadguard source/ModularBlocks.h`), audit those files.
- Otherwise, inspect modified files in the working directory using `git diff --name-only HEAD`. Filter for C++ files in `source/` that inherit from or implement `DSPBlock` or `juce::AudioProcessor`.

### 2. Scan for Real-Time Violations in Processing Loops
Search the inner methods (`process`, `processBlock`, `processStereo`, `applyEnvelopeSlope`, `evaluateWaveform`, etc.) for the following violation categories:

#### Category A: Dynamic Heap Allocations
- `new`, `delete`, `malloc`, `free`, `calloc`, `realloc`
- `std::make_unique`, `std::make_shared`
- `std::vector::push_back`, `std::vector::insert`, `std::vector::resize` (inside process loops)
- String manipulations: `std::string` concatenation, `juce::String` creation, `std::stringstream`
- Dynamic JUCE containers: `juce::Array::add`, `juce::OwnedArray::add`

#### Category B: Blocking Synchronization & OS Primitives
- `std::mutex`, `std::recursive_mutex`, `std::timed_mutex`
- `std::lock_guard`, `std::unique_lock`, `std::scoped_lock`
- `juce::CriticalSection`, `juce::ScopedLock`
- Thread sleep or wait calls: `std::this_thread::sleep_for`, `juce::Thread::sleep`
- Atomic operations with `std::memory_order_seq_cst` inside heavy per-sample inner loops (prefer `relaxed` or `acquire`/`release` where appropriate).

#### Category C: File, Network, or Console I/O
- Output streams: `std::cout`, `std::cerr`, `printf`, `fprintf`
- File operations: `std::ifstream`, `std::ofstream`, `FILE*`, `juce::File`
- Logging & developer diagnostics: `DBG(...)`, `juce::Logger::writeToLog`, `TKS_LOG`, `TKS_LOG_INFO`, `TKS_LOG_WARN`, `TKS_LOG_ERROR` inside per-sample processing.

#### Category D: SIMD Vectorization & Transcendental Bottlenecks
- Flag heavy CRT transcendental math calls in inner sample loops:
  - `std::pow(2.0f, x)` $\to$ recommend replacing with `TbdAudio::FastMath::fastPow2(x)`
  - `std::sin(phase * TWO_PI)` $\to$ recommend replacing with `TbdAudio::FastMath::fastSin(phase)`
  - `std::tanh(x)` $\to$ recommend replacing with `TbdAudio::FastMath::fastTanh(x)`
  - `phase - std::floor(phase)` $\to$ recommend branchless `PhaseAccumulator32`

### 3. Generate Audit Report
Present a clear, structured report:

```markdown
### 🛡️ Audio-Thread Safety Audit Report

**Files Audited:**
- `source/ModularBlocks.h`
- `source/PlanterEngine.h`

#### Status: ✅ PASSED (Zero Real-Time Violations)
- **Heap Allocations:** 0 detected in processing paths.
- **Blocking Locks:** 0 detected.
- **I/O & Logging:** 0 detected.
- **Optimization Suggestions:** 2 calls to `std::pow` detected in `CarrierBlock::processStereo` (can be accelerated via `fastPow2`).
If violations are found, detail the exact file, line number, offending code, and proposed non-allocating fix.

---

### ⚡ Pre-AST Token Shield Protocol (MANDATORY INVARIANT)
To prevent burning tens of thousands of LLM tokens manually reading 1,000+ lines of C++ files, agents **MUST NEVER** inspect full `.cpp` or `.h` files directly to audit real-time safety.

Instead, always execute the mechanical CLI linter with `--terse`:
```bash
python tools/audiothread_guard.py --terse
```
- **If Clean**: Emits a single line (`✅ Audio-Thread Safety: 0 critical violations across N files`) consuming ~10 tokens. Proceed immediately without reading source code.
- **If Violations Detected**: The CLI emits ONLY the exact file path, line number, category, and offending snippet (<100 tokens total). Read and edit **ONLY** the specific reported lines to remediate.
