#pragma once

#include <algorithm>
#include <cmath>
#include "../../FastMath.h"

namespace tkdsp {

namespace FastMath = TbdAudio::FastMath;

/**
 * DrumLimiter — SOTA Drum Channel Brickwall Limiter (Distressor Nuke Mode)
 * 
 * Features:
 * - 0.1ms FET attack with 11.0 ms detector peak hold (eliminates 40-50 Hz sub ripple).
 * - Two-state program-dependent adaptive release (30ms hit recovery to 300ms opto sustain).
 * - Automatic makeup gain (G_makeup = kCeiling / kThreshold = 1.531082x = +3.70 dB).
 * - Headroom-scaled character drive (odd harmonic saturation).
 * - Linked stereo ceiling soft-clipper with continuous C^1 Hermite knee.
 * - Lock-free ballistic gain reduction telemetry.
 * - Output guaranteed <= -0.3 dBFS (0.9660) at all times with 0 samples latency.
 */
class DrumLimiter {
public:
    void prepare(float sampleRate) noexcept {
        sampleRate_ = std::max(sampleRate, 4000.0f);
        invSampleRate_ = 1.0f / sampleRate_;

        attCoeff_      = 1.0f - std::exp(-invSampleRate_ / 0.0001f); // 0.1ms FET attack
        relFastCoeff_  = 1.0f - std::exp(-invSampleRate_ / 0.0300f); // 30ms fast hit recovery
        relSlowCoeff_  = 1.0f - std::exp(-invSampleRate_ / 0.3000f); // 300ms opto sustain
        densCoeff_     = 1.0f - std::exp(-invSampleRate_ / 0.1200f); // 120ms density tracker
        holdSamples_   = std::max(4, static_cast<int>(0.011f * sampleRate_)); // 11ms hold

        reset();
    }

    void reset() noexcept {
        env_ = 0.0f;
        dens_ = 0.0f;
        holdCtr_ = 0;
        latestGr_ = 1.0f;
    }

    inline void process(float& xL, float& xR, bool enabled) noexcept {
        if (!enabled) {
            latestGr_ = 1.0f;
            return; // Passthrough
        }

        constexpr float kThreshold = 0.630957f; // -4.0 dBFS
        constexpr float kCeiling   = 0.966051f; // -0.3 dBFS
        constexpr float kMakeup    = 1.531082f; // kCeiling / kThreshold (+3.70 dB)
        constexpr float kKneeStart = 0.812836f; // -1.5 dB below ceiling

        // 1. Linked stereo peak detection with 11.0 ms Peak Hold
        const float peak = std::max(std::abs(xL), std::abs(xR));
        if (peak >= env_) {
            env_ += (peak - env_) * attCoeff_;
            holdCtr_ = holdSamples_;
        } else if (holdCtr_ > 0) {
            --holdCtr_; // Hold across deep 40-50 Hz sub-bass half-cycles
        } else {
            // Adaptive release: interpolate between 30ms fast and 300ms slow opto
            const float rel = relFastCoeff_ + dens_ * (relSlowCoeff_ - relFastCoeff_);
            env_ += (peak - env_) * rel;
        }

        // Flush CPU denormals
        if (env_ < 1.0e-15f) env_ = 0.0f;
        if (dens_ < 1.0e-15f) dens_ = 0.0f;

        // 2. Gain Reduction & Density Tracking
        float gr = 1.0f;
        if (env_ > kThreshold) {
            gr = kThreshold / env_;
        }
        latestGr_ = gr;

        const float grNorm = std::clamp(1.0f - gr, 0.0f, 1.0f);
        dens_ += (grNorm - dens_) * densCoeff_;

        // 3. Apply Gain Reduction & Auto-Makeup
        float yL = (xL * gr) * kMakeup;
        float yR = (xR * gr) * kMakeup;

        // 4. Headroom-Scaled Odd-Harmonic Character Drive
        const float driveAmt = 0.015f + 0.035f * grNorm;
        yL += driveAmt * yL * std::abs(yL) * (1.0f - driveAmt);
        yR += driveAmt * yR * std::abs(yR) * (1.0f - driveAmt);

        // 5. Linked Stereo Soft-Clipper with C^1 Hermite Knee at -0.3 dBFS
        const float maxPeak = std::max(std::abs(yL), std::abs(yR));
        if (maxPeak > kKneeStart) {
            const float excess = maxPeak - kKneeStart;
            constexpr float kKneeWidth = kCeiling - kKneeStart; // 0.153215f
            const float clippedPeak = kKneeStart + kKneeWidth * FastMath::fastTanh(excess / kKneeWidth);
            const float scale = clippedPeak / maxPeak;
            xL = yL * scale;
            xR = yR * scale;
        } else {
            xL = yL;
            xR = yR;
        }
    }

    [[nodiscard]] inline float getGainReduction() const noexcept {
        return latestGr_;
    }

private:
    float sampleRate_{48000.0f};
    float invSampleRate_{1.0f / 48000.0f};
    float attCoeff_{0.0f};
    float relFastCoeff_{0.0f};
    float relSlowCoeff_{0.0f};
    float densCoeff_{0.0f};
    int   holdSamples_{0};
    int   holdCtr_{0};

    float env_{0.0f};
    float dens_{0.0f};
    float latestGr_{1.0f};
};

} // namespace tkdsp
