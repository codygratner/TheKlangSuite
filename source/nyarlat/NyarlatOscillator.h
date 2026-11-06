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

        // 1. Calculate effective mask
        int mask = (rawMask * 16) + 15;
        // Clamp to 1..8191
        mask = juce::jlimit(1, 8191, mask);

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

        // 4. Q32 Stator Sweep Setup
        const float sp = (maskSpeed > 0.0f && std::isfinite(maskSpeed)) ? std::fmin(maskSpeed, 50.0f) : 0.0f;
        const uint32_t statorInc = static_cast<uint32_t>(sp * (4294967296.0 / (sampleRate_ * 16.0)));
        const uint32_t base = static_cast<uint32_t>(maskStart + 512); // >= 257, no signed underflow

        // 5. Evaluate 16 sub-samples and sum (boxcar decimation)
        // Auto-promote loopMode == 0 (Off) to Saw (1) when speed > 0 so SPEED is never silently dead
        const int effectiveLoop = (loopMode == 0 && sp > 0.0f) ? 1 : loopMode;
        float sum = 0.0f;
        for (int sub = 0; sub < kOversample; ++sub) {
            uint32_t sweep = 0;
            if (effectiveLoop != 0) {
                statorQ_ += statorInc;
                const uint32_t p = statorQ_ >> 23; // 0..511 cyclic position
                // Saw (1): linear ramp | Ping-Pong (2): triangle bounce
                sweep = (effectiveLoop == 1) ? p : (p < 256u ? p * 2u : (511u - p) * 2u);
            }
            const uint32_t off = (sweep + base) & 0x1FFu; // Circular mod-512 offset
            lastStatorOffset_ = off;
            const uint32_t tableIdx = ((phase_ >> 15) + off) & static_cast<uint32_t>(mask);

            sum += buffer8k_[tableIdx & 0x1FFFu];
            phase_ += phaseInc;
        }

        float decimated = sum * (1.0f / 16.0f);

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
                } else if ((mask & 512) != 0) {
                    ratio = 2.0f;
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
