#pragma once

#include <algorithm>
#include <cmath>
#include "../../FastMath.h"

namespace tkdsp {

namespace FastMath = TbdAudio::FastMath;

/**
 * DrumLimiter — Characterful Drum Channel Brickwall Limiter (Distressor Nuke Mode)
 * 
 * FET-fast 0.1ms attack, dual opto release (35ms / 380ms), odd-harmonic character drive,
 * and hard saturated ceiling at -0.3 dBFS (0.9660). Output guaranteed <= 0 dBFS at all times.
 */
class DrumLimiter {
public:
    void prepare(float sampleRate) noexcept {
        sampleRate_ = std::max(sampleRate, 4000.0f);
        invSampleRate_ = 1.0f / sampleRate_;

        attCoeff_ = 1.0f - std::exp(-invSampleRate_ / 0.0001f); // 0.1ms FET attack
        rel1Coeff_ = 1.0f - std::exp(-invSampleRate_ / 0.0350f); // 35ms fast hit recovery
        rel2Coeff_ = 1.0f - std::exp(-invSampleRate_ / 0.3800f); // 380ms opto sustain

        reset();
    }

    void reset() noexcept {
        env_ = 0.0f;
    }

    inline void process(float& xL, float& xR, bool enabled) noexcept {
        if (!enabled) return; // Passthrough

        constexpr float kThreshold = 0.630957f; // -4.0 dBFS
        constexpr float kCeiling   = 0.966051f; // -0.3 dBFS

        // Linked stereo peak detection
        const float peak = std::max(std::abs(xL), std::abs(xR));
        if (peak > env_) {
            env_ += (peak - env_) * attCoeff_;
        } else {
            const float rel = 0.65f * rel1Coeff_ + 0.35f * rel2Coeff_;
            env_ += (peak - env_) * rel;
        }

        float gr = 1.0f;
        if (env_ > kThreshold) {
            // Nuke limiting curve (infinite ratio above threshold)
            gr = kThreshold / env_;
        }

        // Apply gain reduction
        float yL = xL * gr;
        float yR = xR * gr;

        // Odd-harmonic character drive scaled by gain reduction
        const float grNorm = std::clamp(1.0f - gr, 0.0f, 1.0f);
        const float driveAmt = 0.02f + 0.05f * grNorm;
        yL += driveAmt * yL * std::abs(yL);
        yR += driveAmt * yR * std::abs(yR);

        // Brickwall ceiling soft-clip at -0.3 dBFS
        auto clipCeiling = [](float v) noexcept -> float {
            if (v > kCeiling) {
                float excess = v - kCeiling;
                return kCeiling + 0.033f * FastMath::fastTanh(excess * 10.0f);
            } else if (v < -kCeiling) {
                float excess = -v - kCeiling;
                return -(kCeiling + 0.033f * FastMath::fastTanh(excess * 10.0f));
            }
            return v;
        };

        xL = clipCeiling(yL);
        xR = clipCeiling(yR);
    }

private:
    float sampleRate_{48000.0f};
    float invSampleRate_{1.0f / 48000.0f};
    float attCoeff_{0.0f};
    float rel1Coeff_{0.0f};
    float rel2Coeff_{0.0f};
    float env_{0.0f};
};

} // namespace tkdsp
