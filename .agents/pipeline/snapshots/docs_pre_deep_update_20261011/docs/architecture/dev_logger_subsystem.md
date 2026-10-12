# Developer Logging Subsystem (`TKS_LOG`) & Diagnostics

> **Authority:** New Klang City Architecture Standard  
> **Target:** `source/DevLogger.h`, `source/DevLogger.cpp`

---

## 1. Executive Summary

Debugging complex DSP applications and asynchronous UI lifecycles requires structured logging that provides granular diagnostics in development, zero CPU overhead in production, and guaranteed safety against real-time audio thread dropouts.

The `TKS_LOG` subsystem provides leveled developer logging in Debug builds, strictly audited against audio thread abuse and stripped completely in Release builds.

---

## 2. Core Architecture

### 1. Leveled Diagnostics
- `TKS_LOG_INFO(category, message)`: Normal lifecycle events (JSON asset loading, window resizing, preset initialization).
- `TKS_LOG_WARN(category, message)`: Non-fatal anomalies (missing optional JSON key, fallback styling applied).
- `TKS_LOG_ERROR(category, message)`: Critical failures (JSON syntax parse errors, missing APVTS bindings, unexpected host state).

### 2. Dual-Mode Output Sinks
In Debug builds (`JUCE_DEBUG=1`):
1. **System Debugger:** Pipes timestamped entries to system debug output (`OutputDebugString` on Windows / standard console).
2. **Rotating Disk Sink:** Writes timestamped entries to `%LOCALAPPDATA%\TheKlangSuite\dev.log`.
   - **Retention Cap:** File size is strictly capped at 5 MB.
   - **Rotation:** Automatically rolls over up to 2 backup files (`dev.log.1`, `dev.log.2`). Stale logs beyond the cap are purged.

### 3. Strict Real-Time Audio Thread Guardrails
- **Hard Debug Assertion:** `TKS_LOG` asserts in Debug builds if invoked from the audio rendering thread (`AudioIODeviceCallback::audioDeviceIOCallback` or `juce::AudioProcessor::processBlock`).
- **Safe Fallback:** If inadvertently invoked on an audio thread, it executes a safe early return before performing string formatting or acquiring thread locks.
- **Static Analysis:** `/audiothread-guard` scans source files to ensure no `TKS_LOG` macros exist in audio processing loops.

### 4. Zero-Cost Release Stripping
When compiling for Release (`NDEBUG` / `JUCE_DEBUG=0`):
- All `TKS_LOG_*` macros compile out completely to empty no-ops:
  ```cpp
  #define TKS_LOG_INFO(category, msg) do {} while (false)
  #define TKS_LOG_WARN(category, msg) do {} while (false)
  #define TKS_LOG_ERROR(category, msg) do {} while (false)
  ```
- **Zero Binary Bloat:** Strings are completely eliminated by the compiler.
- **Zero Overhead:** Zero CPU cycles, zero memory allocations.
