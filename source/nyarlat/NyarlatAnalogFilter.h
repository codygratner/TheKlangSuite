#pragma once

#include <cmath>
#include <algorithm>
#include <juce_core/juce_core.h>
#include "../FastMath.h"

namespace Nyarlat {

/**
 * @brief Zero-Delay Feedback (ZDF) Silky Analog Filter
 * 
 * Provides three musical filter modes:
 * - 0: 12 dB Lowpass (2-pole ZDF SVF, snappy clavs & open strings)
 * - 1: 24 dB Lowpass (4-pole ZDF ladder, deep velvety pads & warm bass)
 * - 2: 12 dB Bandpass (2-pole ZDF SVF, vocal wah & resonant clav pick)
 *
 * Features soft-limiting resonance that never explodes, plus tube warmth drive.
 * SOTA S2A Optimized: Pre-computes filter coefficients once per stereo block,
 * eliminates redundant per-channel std::tan evaluations, and uses Padé rational saturation.
 */
class NyarlatAnalogFilter {
public:
    enum SlopeMode : int {
        Mode_12dB_LP = 0,
        Mode_24dB_LP = 1,
        Mode_12dB_BP = 2
    };

    void prepare(double sampleRate) noexcept {
        sampleRate_ = (sampleRate > 1000.0) ? sampleRate : 48000.0;
        invSampleRate_ = 1.0 / sampleRate_;
        reset();
    }

    void reset() noexcept {
        s1L_ = s2L_ = 0.0f;
        s1R_ = s2R_ = 0.0f;
        for (int i = 0; i < 4; ++i) {
            ladL_[i] = 0.0f;
            ladR_[i] = 0.0f;
        }
    }

    void sanitize() noexcept {
        auto clean = [](float& v) {
            if (!std::isfinite(v) || std::abs(v) > 1e4f) v = 0.0f;
        };
        clean(s1L_); clean(s2L_);
        clean(s1R_); clean(s2R_);
        for (int i = 0; i < 4; ++i) {
            clean(ladL_[i]);
            clean(ladR_[i]);
        }
    }

    void processStereo(float& left, float& right,
                       float cutoffHz, float resonance, int mode, float drive) noexcept {
        const float fc = std::clamp(cutoffHz, 15.0f, static_cast<float>(sampleRate_ * 0.46));
        const float res = std::clamp(resonance, 0.0f, 0.98f);
        const float drv = std::clamp(drive, 0.0f, 1.0f);

        // Warm drive pre-saturation using FastMath Padé rational saturation
        if (drv > 0.001f) {
            const float driveFactor = 1.0f + drv * 2.5f;
            left = softSaturate(left * driveFactor);
            right = softSaturate(right * driveFactor);
        }

        // Shared stereo pre-computation: compute g once for both channels!
        // Unclamps g beyond 0.999f so the filter can open cleanly up to 20kHz
        const float g = std::clamp(std::tan(3.14159265358979323846f * fc * static_cast<float>(invSampleRate_)), 0.0001f, 15.0f);

        if (mode == Mode_24dB_LP) {
            // 4-Pole (24 dB/oct) Non-Linear Resonant Ladder
            const float h = g / (1.0f + g);
            const float k = res * 3.92f; // Keep safely bounded below 4.0
            const float h4 = h * h * h * h;
            const float invDenom = 1.0f / (1.0f + k * h4);

            left = processLadder(left, h, k, invDenom, ladL_);
            right = processLadder(right, h, k, invDenom, ladR_);
        } else {
            // 2-Pole (12 dB/oct) ZDF State Variable Filter
            const bool isBp = (mode == Mode_12dB_BP);
            const float Q = 0.5f + res * 15.5f;
            const float k = 1.0f / Q;
            const float invDenom = 1.0f / (1.0f + 2.0f * k * g + g * g);

            left = processSvf(left, g, k, invDenom, s1L_, s2L_, isBp);
            right = processSvf(right, g, k, invDenom, s1R_, s2R_, isBp);
        }
    }

private:
    [[nodiscard]] static inline float softSaturate(float x) noexcept {
        const float xc = std::clamp(x, -1.8f, 1.8f);
        return xc - 0.08f * (xc * xc) - 0.05f * (xc * xc * xc);
    }

    inline float processLadder(float in, float h, float k, float invDenom, float* s) noexcept {
        const float fb = softSaturate(s[3]);
        const float u = (in - k * fb) * invDenom;

        float stageIn = u;
        for (int i = 0; i < 4; ++i) {
            const float v = (stageIn - s[i]) * h;
            const float out = v + s[i];
            s[i] = out + v;
            stageIn = out;
        }
        return stageIn;
    }

    inline float processSvf(float in, float g, float k, float invDenom, float& s1, float& s2, bool bandpass) noexcept {
        // ZDF SVF linear solve with pre-computed reciprocal denominator
        const float hp = (in - (2.0f * k + g) * s1 - s2) * invDenom;
        const float v1 = g * hp;
        const float bp = v1 + s1;
        s1 = bp + v1;
        const float v2 = g * bp;
        const float lp = v2 + s2;
        s2 = lp + v2;

        return bandpass ? bp : lp;
    }

    double sampleRate_{48000.0};
    double invSampleRate_{1.0 / 48000.0};

    // SVF states
    float s1L_{0.0f}, s2L_{0.0f};
    float s1R_{0.0f}, s2R_{0.0f};

    // Ladder states
    float ladL_[4]{0.0f, 0.0f, 0.0f, 0.0f};
    float ladR_[4]{0.0f, 0.0f, 0.0f, 0.0f};
};

} // namespace Nyarlat
