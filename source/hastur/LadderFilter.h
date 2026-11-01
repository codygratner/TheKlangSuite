#pragma once

#include <cmath>
#include <algorithm>

namespace CrackedChimes::DSP {

/**
 * @brief 4-Pole (24 dB/oct) Non-Linear Resonant Ladder Filter (Moog / CEM3320 style).
 *        Features self-oscillation at high resonance and warm saturating feedback.
 */
class LadderFilter {
public:
    void prepare(double sampleRate) noexcept {
        sampleRate_ = (sampleRate > 1000.0) ? sampleRate : 44100.0;
        invSampleRate_ = 1.0 / sampleRate_;
        reset();
    }

    void reset() noexcept {
        s_[0] = s_[1] = s_[2] = s_[3] = 0.0f;
    }

    /**
     * @brief Process a single sample through the 4-pole ladder.
     * @param input Raw audio sample
     * @param cutoffHz Cutoff in Hz [20.0 .. 20000.0]
     * @param resonance Resonance [0.0 .. 1.0] (self-oscillates above 0.92)
     */
    [[nodiscard]] inline float process(float input, float cutoffHz, float resonance) noexcept {
        // Clamp parameters
        const float fc = std::clamp(cutoffHz, 20.0f, static_cast<float>(sampleRate_ * 0.45));
        const float res = std::clamp(resonance, 0.0f, 1.0f);

        // Calculate tuned pole frequency (trapezoidal integration)
        const float wd = 2.0f * 3.14159265f * fc;
        const float T = static_cast<float>(invSampleRate_);
        const float wa = (2.0f / T) * std::tan(wd * T * 0.5f);
        const float g = std::clamp(wa * T * 0.5f, 0.0f, 0.999f);
        const float h = g / (1.0f + g);

        // Feedback resonance factor (4.0 is theoretical self-oscillation boundary)
        const float k = res * 4.0f;

        // Feedback saturation (soft limiter to tame self-oscillation)
        const float fb = softSaturate(s_[3]);
        const float u = (input - k * fb) / (1.0f + k * h * h * h * h);

        // Cascade 4 one-pole lowpass stages
        float stageIn = u;
        for (int i = 0; i < 4; ++i) {
            const float v = (stageIn - s_[i]) * h;
            const float out = v + s_[i];
            s_[i] = out + v; // Update state for trapezoidal integration
            stageIn = out;
        }

        return stageIn;
    }

private:
    [[nodiscard]] static inline float softSaturate(float x) noexcept {
        // Fast cubic soft clipper
        const float xClamped = std::clamp(x, -1.5f, 1.5f);
        return xClamped - (xClamped * xClamped * xClamped) * (1.0f / 4.5f);
    }

    double sampleRate_{44100.0};
    double invSampleRate_{1.0 / 44100.0};
    float s_[4]{0.0f, 0.0f, 0.0f, 0.0f};
};

} // namespace CrackedChimes::DSP
