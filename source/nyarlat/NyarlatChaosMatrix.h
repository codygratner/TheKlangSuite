#pragma once

#include <cmath>
#include <array>
#include <random>
#include <algorithm>
#include <juce_core/juce_core.h>

namespace Nyarlat {

/**
 * @brief 18-Channel Knot-Staggered Cubic Hermite Chaos Engine (10m to 1s Timebase)
 * 
 * Provides 18 channels of smooth Brownian wander or discrete S&H per voice,
 * mapping 1:1 to the 6 Timbral Controls across Oscillators 1, 2, 3:
 * - 0..2:   Morph (O1, O2, O3)
 * - 3..5:   Odd/Even (O1, O2, O3)
 * - 6..8:   Tilt (O1, O2, O3)
 * - 9..11:  Formant (O1, O2, O3)
 * - 12..14: Mask (O1, O2, O3)
 * - 15..17: Speed (O1, O2, O3)
 */
class VoiceChaosEngine {
public:
    static constexpr int kNumChannels = 21;

    VoiceChaosEngine() {
        rng_.seed(std::random_device{}());
        for (int i = 0; i < kNumChannels; ++i) {
            knotA_[i] = nextRandomFloat();
            knotB_[i] = nextRandomFloat();
            steppedValue_[i] = nextRandomFloat();
            currentValue_[i] = knotA_[i];
            output_[i] = knotA_[i];
            // Stagger channel phases so knot transitions never coincide
            channelPhase_[i] = static_cast<float>(i) * (1.0f / static_cast<float>(kNumChannels));
        }
    }

    void setVoiceSeed(int voiceIndex, float polySpread) noexcept {
        voiceIndex_ = voiceIndex;
        polySpread_ = juce::jlimit(0.0f, 1.0f, polySpread);
        // Phase decorrelation per voice (golden ratio distribution)
        voicePhaseOffset_ = static_cast<float>(voiceIndex) * 0.6180339887f * polySpread_;
    }

    // Triggered on Note-On (rolls new stepped values for S&H mode)
    void triggerNoteOn() noexcept {
        for (int i = 0; i < kNumChannels; ++i) {
            steppedValue_[i] = nextRandomFloat();
        }
    }

    // Advanced at control-rate block
    void updateBlock(float rateHz, float driftNorm, double blockDurationSec) noexcept {
        const float safeRate = std::clamp(rateHz, 0.001667f, 10.0f);
        const float stepSpeed = safeRate * static_cast<float>(blockDurationSec);
        const float drift = juce::jlimit(0.0f, 1.0f, driftNorm);

        for (int i = 0; i < kNumChannels; ++i) {
            channelPhase_[i] += stepSpeed;
            while (channelPhase_[i] >= 1.0f) {
                channelPhase_[i] -= 1.0f;
                knotA_[i] = knotB_[i];
                knotB_[i] = nextRandomFloat();
            }

            // Cubic Hermite smooth interpolation: S(t) = 3t^2 - 2t^3
            // Incorporates polyphonic voice phase offset
            float effPhase = channelPhase_[i] + voicePhaseOffset_;
            effPhase -= std::floor(effPhase);

            const float s = effPhase * effPhase * (3.0f - 2.0f * effPhase);
            currentValue_[i] = knotA_[i] + s * (knotB_[i] - knotA_[i]);

            // Blend between stepped S&H and continuous smooth flow
            output_[i] = (1.0f - drift) * steppedValue_[i] + drift * currentValue_[i];
        }
    }

    // Get modulated offset for parameter index (0..17)
    [[nodiscard]] inline float getModulation(int index, float amount) const noexcept {
        if (index < 0 || index >= kNumChannels) return 0.0f;
        // Unipolar modulation [0.0, amount]
        return output_[index] * std::clamp(amount, 0.0f, 1.0f);
    }

    void copyRawValues(float* dest, int count) const noexcept {
        int n = std::min(count, kNumChannels);
        for (int i = 0; i < n; ++i) dest[i] = output_[i];
    }

    [[nodiscard]] inline float getRawValue(int index) const noexcept {
        if (index < 0 || index >= kNumChannels) return 0.5f;
        return output_[index];
    }

private:
    float nextRandomFloat() noexcept {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        return dist(rng_);
    }

    std::mt19937 rng_;
    int voiceIndex_ = 0;
    float polySpread_ = 0.5f;
    float voicePhaseOffset_ = 0.0f;

    std::array<float, kNumChannels> knotA_{};
    std::array<float, kNumChannels> knotB_{};
    std::array<float, kNumChannels> channelPhase_{};
    std::array<float, kNumChannels> steppedValue_{};
    std::array<float, kNumChannels> currentValue_{};
    std::array<float, kNumChannels> output_{};
};

} // namespace Nyarlat
