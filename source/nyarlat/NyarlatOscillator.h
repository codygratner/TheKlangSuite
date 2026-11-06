#pragma once

#include <cmath>
#include <array>
#include <vector>
#include <cstring>
#include <cstdint>
#include <algorithm>
#include <juce_core/juce_core.h>
#include "NyarlatRom.h"

namespace Nyarlat {

class BitmaskOscillator {
public:
    static constexpr size_t kTableSize = 512;
    static constexpr size_t kZeroRunway = 8192; // 8KB buffer (512 wave + 7680 zeroes)
    static constexpr int kOversample = 16;

    BitmaskOscillator() {
        buffer8k_.assign(kZeroRunway, 0.0f);
    }

    void prepare(double sampleRate) noexcept {
        sampleRate_ = (sampleRate > 0.0) ? sampleRate : 44100.0;
        phase_ = 0;
        statorQ_ = 0;
        dcState_ = 0.0f;
        prevInput_ = 0.0f;
        syncFade_ = 0.0f;
        // Pre-warm static pitch compensation table outside real-time audio thread
        juce::ignoreUnused(getSharedPitchTable()[0]);
    }

    void reset() noexcept {
        phase_ = 0;
        statorQ_ = 0;
        dcState_ = 0.0f;
        prevInput_ = 0.0f;
        syncFade_ = 0.0f;
    }

    // Updates the morphed AKWF cycle in the first 512 samples of the 8KB buffer
    void updateTable(uint32_t bankIdx, float waveMorph) noexcept {
        AkwfRom::getInstance().interpolateWaveform(bankIdx, waveMorph, buffer8k_.data());
    }

    // Process a single audio sample (evaluates 16 sub-samples internally with Q32 Stator Sweep)
    float processSample(float freqHz, int rawMask, int maskStart, float maskSpeed,
                        int loopMode, bool octLock, bool hardSyncTrigger = false) noexcept {
        // Handle hard sync reset with 2-subsample anti-glitch
        if (hardSyncTrigger) {
            phase_ = 0;
            statorQ_ = 0;
            syncFade_ = 1.0f;
        }

        // 1. Calculate effective 9-bit table mask (bits strictly within 512-sample cycle)
        const uint32_t mask9 = static_cast<uint32_t>(juce::jlimit(0, 511, rawMask) * 16 + 15) & 0x1FFu;
        const bool maskIdentity = (mask9 == 0x1FFu);

        // 2. Octave Lock frequency compensation
        float effectiveFreq = freqHz;
        if (octLock && rawMask >= 0 && rawMask < 512) {
            effectiveFreq *= getSharedPitchTable()[rawMask];
        }

        // 3. Phase increment for 16x oversampling:
        // One period = 2^24 phase units = 512 table indices (512 * 2^15)
        // inc_per_substep = (freq * 2^24) / (sampleRate * 16)
        double incPerSubstep = (static_cast<double>(effectiveFreq) * 16777216.0) / (sampleRate_ * 16.0);
        uint32_t phaseInc = static_cast<uint32_t>(incPerSubstep);

        // 4. Stator -> smooth Casio CZ resonant phase-distortion depth
        const float sp = (maskSpeed > 0.0f && std::isfinite(maskSpeed)) ? std::fmin(maskSpeed, 50.0f) : 0.0f;
        const uint32_t statorInc = static_cast<uint32_t>(sp * (4294967296.0 / (sampleRate_ * 16.0)));
        const uint32_t base = static_cast<uint32_t>(maskStart + 512); // >= 257, no signed underflow
        const int effectiveLoop = (loopMode == 0 && sp > 0.0f) ? 1 : loopMode;

        auto S = [](float x) noexcept {
            x = juce::jlimit(0.0f, 1.0f, x);
            return x * x * (3.0f - 2.0f * x);
        };

        // 5. Evaluate 16 sub-samples and sum (boxcar decimation)
        float sum = 0.0f;
        float lastDepth = 0.0f;
        const float* tbl = buffer8k_.data();

        for (int sub = 0; sub < kOversample; ++sub) {
            uint32_t sweep = 0;
            if (effectiveLoop != 0) {
                statorQ_ += statorInc;
                const uint32_t p = statorQ_ >> 23; // 0..511 cyclic position
                sweep = (effectiveLoop == 1) ? p : (p < 256u ? p * 2u : (511u - p) * 2u);
            }
            const uint32_t off = (sweep + base) & 0x1FFu; // Stator position (0..511)
            lastStatorOffset_ = off;

            const float t = static_cast<float>(off) * (1.0f / 512.0f);
            const float depth = (effectiveLoop == 2)
                ? S(1.0f - std::fabs(2.0f * t - 1.0f))
                : (t <= 0.9f ? t : t * (1.0f - S((t - 0.9f) * 10.0f)));
            lastDepth = depth;
            const float r = 1.0f + 5.0f * depth;

            // Normalized cycle phase strictly in [0, 1), guaranteeing 100% pitch lock to f0
            const float phi = static_cast<float>(phase_ & 0x00FFFFFFu) * (1.0f / 16777216.0f);
            float x = phi * r;
            x -= std::floor(x);
            const float pos = x * 512.0f;
            const uint32_t i = static_cast<uint32_t>(pos);
            const uint32_t im = i & mask9;
            float s = tbl[im];
            if (maskIdentity) {
                const float frac = pos - static_cast<float>(i);
                s += frac * (tbl[(im + 1u) & 0x1FFu] - s);
            }
            // Cubic smooth window tapers resonant seam smoothly to zero at phi = 1 (no edge clicks)
            sum += s * (1.0f - depth * S(phi));
            phase_ += phaseInc;
        }

        // Soft linear gain scaling (1.0x .. 1.6x) keeping perceived volume uniform across resonant depth
        float decimated = sum * (1.0f / 16.0f) * (1.0f + 0.6f * lastDepth);

        // Smooth hard-sync transition if active
        if (syncFade_ > 0.0f) {
            decimated *= (1.0f - syncFade_ * 0.5f);
            syncFade_ -= 0.1f;
            if (syncFade_ < 0.0f) syncFade_ = 0.0f;
        }

        // 6. High-Pass Leaky DC Blocker (R = 0.999)
        // y[n] = x[n] - x[n-1] + R * y[n-1]
        float dcOut = decimated - prevInput_ + 0.999f * dcState_;
        prevInput_ = decimated;
        dcState_ = dcOut;

        return dcOut;
    }

    uint32_t getPhase() const noexcept { return phase_; }
    uint32_t getStatorOffset() const noexcept { return lastStatorOffset_; }

private:
    static const std::array<float, 512>& getSharedPitchTable() {
        static const auto table = []() {
            std::array<float, 512> t;
            for (int m = 0; m < 512; ++m) {
                int mask = m * 16 + 15;
                float ratio = 1.0f;
                if ((mask & 256) == 0 && (mask & 128) != 0) {
                    ratio = 0.5f;
                } else if ((mask & 128) == 0 && (mask & 64) != 0) {
                    ratio = 0.25f;
                }
                t[m] = ratio;
            }
            return t;
        }();
        return table;
    }

    double sampleRate_ = 44100.0;
    uint32_t phase_ = 0;
    uint32_t statorQ_ = 0;
    uint32_t lastStatorOffset_ = 0;
    float dcState_ = 0.0f;
    float prevInput_ = 0.0f;
    float syncFade_ = 0.0f;

    std::vector<float> buffer8k_;
};

} // namespace Nyarlat
