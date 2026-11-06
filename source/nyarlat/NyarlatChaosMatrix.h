#pragma once

#include <cmath>
#include <array>
#include <random>
#include <juce_core/juce_core.h>

namespace Nyarlat {

// Generates smooth continuous chaos (Brownian flow) or discrete S&H per voice
class VoiceChaosEngine {
public:
    VoiceChaosEngine() {
        rng_.seed(std::random_device{}());
        for (int i = 0; i < 15; ++i) {
            currentValue_[i] = nextRandomFloat();
            targetValue_[i] = nextRandomFloat();
        }
    }

    void setVoiceSeed(int voiceIndex, float polySpread) noexcept {
        voiceIndex_ = voiceIndex;
        polySpread_ = polySpread;
        // Phase offset per voice for polyphonic decorrelation
        phaseOffset_ = static_cast<float>(voiceIndex) * 0.392699f * polySpread;
    }

    // Triggered on Note-On (rolls new stepped values)
    void triggerNoteOn() noexcept {
        for (int i = 0; i < 15; ++i) {
            steppedValue_[i] = nextRandomFloat();
        }
    }

    // Advanced at control-rate block (594 Hz)
    void updateBlock(float rateHz, float driftNorm, double blockDurationSec) noexcept {
        // driftNorm: 0.0 = pure stepped S&H, 1.0 = pure continuous chaos flow
        float stepSpeed = rateHz * static_cast<float>(blockDurationSec);
        phase_ += stepSpeed;

        for (int i = 0; i < 15; ++i) {
            // Smooth Brownian wander
            if (std::abs(currentValue_[i] - targetValue_[i]) < 0.05f) {
                targetValue_[i] = nextRandomFloat();
            }
            float slew = juce::jlimit(0.001f, 0.2f, stepSpeed * 2.0f);
            currentValue_[i] += slew * (targetValue_[i] - currentValue_[i]);

            // Blend between stepped S&H and continuous flow
            output_[i] = (1.0f - driftNorm) * steppedValue_[i] + driftNorm * currentValue_[i];
        }
    }

    // Get modulated offset for parameter index (0..14)
    float getModulation(int index, float attenuverterAmount) const noexcept {
        if (index < 0 || index >= 15) return 0.0f;
        // Output swings in [-1.0, 1.0] scaled by the attenuverter amount
        return (output_[index] * 2.0f - 1.0f) * attenuverterAmount;
    }

private:
    float nextRandomFloat() noexcept {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        return dist(rng_);
    }

    std::mt19937 rng_;
    int voiceIndex_ = 0;
    float polySpread_ = 0.5f;
    float phaseOffset_ = 0.0f;
    float phase_ = 0.0f;

    std::array<float, 15> steppedValue_{};
    std::array<float, 15> currentValue_{};
    std::array<float, 15> targetValue_{};
    std::array<float, 15> output_{};
};

} // namespace Nyarlat
