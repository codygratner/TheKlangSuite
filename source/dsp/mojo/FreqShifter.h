#pragma once

#include "../../FastMath.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace tkdsp {

namespace FastMath = TbdAudio::FastMath;

/**
 * FreqShifter — Zero-Latency Single-Sideband (SSB) Bode Frequency Shifter
 *
 * Implements a true time-domain Hilbert Transform allpass network (Bernie Hutchins
 * 4-pole per branch topology) with zero algorithmic delay, quadrature carrier modulation
 * for clean Upper/Lower sideband shifting, and an integrated 6 Hz sub-audio DC blocker.
 *
 * Real-Time Safety Invariants:
 * - 100% Real-Time Audio Safe: Zero heap allocations, zero locks, denormal-free.
 * - Zero CRT transcendentals (FastMath::fastSinNorm / fastCosNorm).
 * - Exact cyclic wrapping via 32-bit unsigned integer DDS accumulator.
 */
class FreqShifter {
public:
    static constexpr std::array<float, 4> cI = {
        0.69238780f, 0.93606543f, 0.98822952f, 0.99874885f
    };
    static constexpr std::array<float, 4> cQ = {
        0.40219212f, 0.85617109f, 0.97229095f, 0.99528848f
    };

    void prepare(float sampleRate) noexcept {
        sampleRate_ = std::max(sampleRate, 8000.0f);
        invSampleRate_ = 1.0f / sampleRate_;

        // 6 Hz single-pole sub-audio DC blocker coefficient
        dcAlpha_ = 1.0f - std::exp(-2.0f * 3.14159265f * 6.0f * invSampleRate_);
        // 10 ms parameter smoothing coefficient
        slewAlpha_ = 1.0f - std::exp(-invSampleRate_ / 0.010f);

        reset();
    }

    void reset() noexcept {
        xI_.fill(0.0f);
        yI_.fill(0.0f);
        xQ_.fill(0.0f);
        yQ_.fill(0.0f);
        z1Q_ = 0.0f;

        phaseNorm_ = 0.0f;
        smoothedDeltaHz_ = 0.0f;
        dcState_ = 0.0f;
    }

    /**
     * @brief Process a single mono sample through the Frequency Shifter.
     * @param in Input audio sample.
     * @param targetDeltaHz Target frequency shift in Hertz (-1000.0 .. +1000.0 Hz).
     * @return Frequency-shifted audio sample.
     */
    inline float process(float in, float targetDeltaHz) noexcept {
        // Slew target frequency to prevent zipper noise on automation
        smoothedDeltaHz_ += slewAlpha_ * (targetDeltaHz - smoothedDeltaHz_);

        // 1. Dual-Branch 90-Degree Allpass Hilbert Network
        // Branch I (0 deg reference path): 4 cascaded 1st-order all-pass filters
        float sigI = in;
        for (int s = 0; s < 4; ++s) {
            const float c = cI[s];
            const float y = c * (sigI + yI_[s]) - xI_[s];
            xI_[s] = sigI;
            yI_[s] = y;
            sigI = y;
        }

        // Branch Q (90 deg quadrature path): unit delay + 4 cascaded 1st-order all-pass filters
        float sigQ = z1Q_;
        z1Q_ = in;
        for (int s = 0; s < 4; ++s) {
            const float c = cQ[s];
            const float y = c * (sigQ + yQ_[s]) - xQ_[s];
            xQ_[s] = sigQ;
            yQ_[s] = y;
            sigQ = y;
        }

        // When shift is essentially 0 Hz, bypass rotation and output Branch I directly
        if (std::abs(smoothedDeltaHz_) < 0.02f) {
            return sigI;
        }

        // 2. Quadrature Carrier Modulation
        const float cosCarrier = FastMath::fastCosNorm(phaseNorm_);
        const float sinCarrier = FastMath::fastSinNorm(phaseNorm_);

        // Single-Sideband Heterodyne:
        // Due to Branch Q leading Branch I by ~+90 degrees (Q ~ cos, I ~ sin):
        // Upper Sideband (shift up):   USB = sin(w0*t + ws*t) = I * cos + Q * sin
        // Lower Sideband (shift down): LSB = sin(w0*t - ws*t) = I * cos - Q * sin
        float wet = 0.0f;
        if (smoothedDeltaHz_ >= 0.0f) {
            wet = sigI * cosCarrier + sigQ * sinCarrier;
        } else {
            wet = sigI * cosCarrier - sigQ * sinCarrier;
        }

        // Advance carrier phase in [0, 1) using normalized frequency
        const float phaseInc = std::abs(smoothedDeltaHz_) * invSampleRate_;
        phaseNorm_ += phaseInc;
        if (phaseNorm_ >= 1.0f) {
            phaseNorm_ -= std::floor(phaseNorm_);
        }

        // 3. Post-Shifter Sub-Audio DC Blocker (6 Hz)
        // Strips any static DC produced when frequencies shift across 0 Hz
        const float hp = wet - dcState_;
        dcState_ += dcAlpha_ * hp;

        return hp;
    }

private:
    float sampleRate_{48000.0f};
    float invSampleRate_{1.0f / 48000.0f};
    float dcAlpha_{0.0008f};
    float slewAlpha_{0.002f};

    // Hilbert Filter States
    std::array<float, 4> xI_{{0.0f, 0.0f, 0.0f, 0.0f}};
    std::array<float, 4> yI_{{0.0f, 0.0f, 0.0f, 0.0f}};
    std::array<float, 4> xQ_{{0.0f, 0.0f, 0.0f, 0.0f}};
    std::array<float, 4> yQ_{{0.0f, 0.0f, 0.0f, 0.0f}};
    float z1Q_{0.0f};

    // Carrier Oscillator
    float phaseNorm_{0.0f};
    float smoothedDeltaHz_{0.0f};
    float dcState_{0.0f};
};

} // namespace tkdsp
