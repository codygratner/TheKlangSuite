// ==============================================================================
// SafetyMonitor.h — Real-Time Audio-Thread Safe LTL Safety Monitor
// ==============================================================================
// Part of Chassis 1: C++ DSP Titan Armor.
// Enforces LTL Safety: G ( finite(out) && |out| <= PeakBound )
// Invariants:
//   - Header-only, C++20 standard.
//   - Zero heap allocation (no new, malloc, vector resize).
//   - Zero locks / mutexes (uses relaxed std::atomic flags).
//   - Zero file or console I/O on the audio thread.
//   - Exponent bitwise masking survives -ffast-math optimization where
//     std::isfinite(x) can be folded to true.
// ==============================================================================

#pragma once

#include <atomic>
#include <bit>
#include <cstdint>
#include <algorithm>

namespace Klang::SQA {

class AudioSafetyMonitor {
public:
    explicit AudioSafetyMonitor(float peakBound = 1.0f) noexcept
        : peakBound_(peakBound) {}

    // Exponent bit-masking: In IEEE 754 float32, exponent is bits 23..30 (0x7F800000).
    // If all exponent bits are 1, the number is either NaN or Infinity.
    static constexpr bool isFiniteBits(float x) noexcept {
        return (std::bit_cast<std::uint32_t>(x) & 0x7F800000u) != 0x7F800000u;
    }

    // Audio-thread processing call
    void process(const float* buffer, int numSamples, std::uint32_t baseSampleIndex = 0) noexcept {
        if (!buffer || numSamples <= 0) return;

        std::uint32_t localBad = 0;
        for (int i = 0; i < numSamples; ++i) {
            const float x = buffer[i];
            const bool finite = isFiniteBits(x);
            const bool bounded = finite && (x <= peakBound_) && (x >= -peakBound_);

            if (!finite)  localBad |= 1u; // Bit 0: NaN or Inf
            if (!bounded) localBad |= 2u; // Bit 1: Peak bound exceeded

            if (localBad && firstBadSample_.load(std::memory_order_relaxed) == ~0u) {
                firstBadSample_.store(baseSampleIndex + static_cast<std::uint32_t>(i), std::memory_order_relaxed);
            }
        }

        if (localBad) {
            violations_.fetch_or(localBad, std::memory_order_relaxed);
        }
    }

    // Non-realtime query methods (called from test runners / GUI)
    [[nodiscard]] bool hasViolations() const noexcept {
        return violations_.load(std::memory_order_relaxed) != 0;
    }

    [[nodiscard]] bool hasNonFinite() const noexcept {
        return (violations_.load(std::memory_order_relaxed) & 1u) != 0;
    }

    [[nodiscard]] bool hasPeakExceeded() const noexcept {
        return (violations_.load(std::memory_order_relaxed) & 2u) != 0;
    }

    [[nodiscard]] std::uint32_t getFirstBadSample() const noexcept {
        return firstBadSample_.load(std::memory_order_relaxed);
    }

    void reset() noexcept {
        violations_.store(0, std::memory_order_relaxed);
        firstBadSample_.store(~0u, std::memory_order_relaxed);
    }

private:
    float peakBound_ = 1.0f;
    std::atomic<std::uint32_t> violations_{0};
    std::atomic<std::uint32_t> firstBadSample_{~0u};
};

} // namespace Klang::SQA
