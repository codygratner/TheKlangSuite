#pragma once

#include <algorithm>
#include <cmath>
#include "../common/AudioEffect.h"
#include "../../FastMath.h"

namespace tkdsp {

namespace FastMath = TbdAudio::FastMath;

/**
 * SquashStage — Bipolar Dual-Personality Dynamics Manifold
 * 
 * Left (-1.0 .. 0.0): "Thinner & Snappier" (Transient expansion + 180Hz mud shelf cut).
 * Center (0.0): Bit-exact clean bypass wire.
 * Right (0.0 .. +1.0): "Thicker & Huge!" (Distressor 10:1..20:1 Nuke + tape saturation + 80Hz head bump).
 */
class SquashStage {
public:
    void prepare(float sampleRate) noexcept {
        sampleRate_ = std::max(sampleRate, 4000.0f);
        invSampleRate_ = 1.0f / sampleRate_;

        // Snappy envelope coefficients
        fastAtt_ = 1.0f - std::exp(-invSampleRate_ / 0.0005f); // 0.5ms
        fastRel_ = 1.0f - std::exp(-invSampleRate_ / 0.0050f); // 5ms
        slowAtt_ = 1.0f - std::exp(-invSampleRate_ / 0.0200f); // 20ms
        slowRel_ = 1.0f - std::exp(-invSampleRate_ / 0.1200f); // 120ms

        // Distressor envelope coefficients
        distAtt_ = 1.0f - std::exp(-invSampleRate_ / 0.0060f); // 6ms
        distRel1_ = 1.0f - std::exp(-invSampleRate_ / 0.0600f); // 60ms
        distRel2_ = 1.0f - std::exp(-invSampleRate_ / 0.2500f); // 250ms

        reset();
    }

    void reset() noexcept {
        fastEnvL_ = fastEnvR_ = 0.0f;
        slowEnvL_ = slowEnvR_ = 0.0f;
        distEnvL_ = distEnvR_ = 0.0f;
        mudLpL_.reset();
        mudLpR_.reset();
        headBumpLpL_.reset();
        headBumpLpR_.reset();
        warmLpL_.reset();
        warmLpR_.reset();
    }

    inline void process(float& xL, float& xR, float squash) noexcept {
        if (std::abs(squash) < 0.005f) return; // Clean bypass wire

        if (squash < 0.0f) {
            // =================================================================
            // LEFT THROW: "Thinner & Snappier"
            // =================================================================
            const float b = -squash; // 0.0 .. 1.0

            // Left channel transient comparator
            const float absL = std::abs(xL);
            fastEnvL_ += (absL > fastEnvL_) ? (absL - fastEnvL_) * fastAtt_ : (absL - fastEnvL_) * fastRel_;
            slowEnvL_ += (absL > slowEnvL_) ? (absL - slowEnvL_) * slowAtt_ : (absL - slowEnvL_) * slowRel_;
            const float diffL = fastEnvL_ - slowEnvL_;
            float gL = 1.0f;
            if (diffL > 0.0f) {
                // Boost attack transient by up to +6b dB (factor ~2.0)
                gL += b * std::min(diffL * 4.0f, 1.0f);
            } else {
                // Attenuate tail by up to -9b dB (factor down to ~0.35)
                gL -= b * std::min(-diffL * 3.0f, 0.65f);
            }

            // Right channel transient comparator
            const float absR = std::abs(xR);
            fastEnvR_ += (absR > fastEnvR_) ? (absR - fastEnvR_) * fastAtt_ : (absR - fastEnvR_) * fastRel_;
            slowEnvR_ += (absR > slowEnvR_) ? (absR - slowEnvR_) * slowAtt_ : (absR - slowEnvR_) * slowRel_;
            const float diffR = fastEnvR_ - slowEnvR_;
            float gR = 1.0f;
            if (diffR > 0.0f) {
                gR += b * std::min(diffR * 4.0f, 1.0f);
            } else {
                gR -= b * std::min(-diffR * 3.0f, 0.65f);
            }

            // Subtractive 180Hz mud shelf cut
            mudLpL_.setLowpass(180.0f, 0.7071f, invSampleRate_);
            mudLpR_.setLowpass(180.0f, 0.7071f, invSampleRate_);
            const float mudL = mudLpL_.process(xL);
            const float mudR = mudLpR_.process(xR);
            mudLpL_.clampState();
            mudLpR_.clampState();

            xL = (xL * gL - 0.5f * b * mudL) / (1.0f + 0.25f * b);
            xR = (xR * gR - 0.5f * b * mudR) / (1.0f + 0.25f * b);

        } else {
            // =================================================================
            // RIGHT THROW: "Thicker & Huge!"
            // =================================================================
            const float a = squash; // 0.0 .. 1.0

            // Distressor heavy compression: threshold down to -30dB (0.0316), ratio 10:1..20:1
            const float thresh = 0.50f - a * 0.4684f; // 0.50 down to 0.0316
            const float ratio = 10.0f + a * 10.0f;     // 10:1 to 20:1 Nuke
            const float slope = 1.0f - (1.0f / ratio);

            // Linked stereo detector
            const float peak = std::max(std::abs(xL), std::abs(xR));
            const float relCoeff = (peak < distEnvL_) ? (0.7f * distRel1_ + 0.3f * distRel2_) : distAtt_;
            distEnvL_ += (peak - distEnvL_) * relCoeff;

            float gr = 1.0f;
            if (distEnvL_ > thresh) {
                const float overDb = 20.0f * std::log10(distEnvL_ / thresh);
                const float grDb = overDb * slope;
                gr = std::pow(10.0f, -grDb / 20.0f);
            }

            // Auto-makeup gain
            const float makeup = 1.0f + a * (1.0f / std::max(gr, 0.2f) - 1.0f) * 0.75f;
            float yL = xL * gr * makeup;
            float yR = xR * gr * makeup;

            // Tape Hysteresis Saturation: fastTanh with d = 1.0 + 2.5 * a^1.5
            const float d = 1.0f + 2.5f * std::pow(a, 1.5f);
            const float invTanhD = 1.0f / FastMath::fastTanh(d);
            yL = FastMath::fastTanh(yL * d) * invTanhD;
            yR = FastMath::fastTanh(yR * d) * invTanhD;

            // 80Hz Analog Head Bump
            headBumpLpL_.setLowpass(80.0f, 0.7071f, invSampleRate_);
            headBumpLpR_.setLowpass(80.0f, 0.7071f, invSampleRate_);
            const float bumpL = headBumpLpL_.process(yL);
            const float bumpR = headBumpLpR_.process(yR);
            headBumpLpL_.clampState();
            headBumpLpR_.clampState();

            yL += 0.25f * a * bumpL;
            yR += 0.25f * a * bumpR;

            // 12kHz Tape Top-End Warmth
            warmLpL_.setLowpass(12000.0f, 0.7071f, invSampleRate_);
            warmLpR_.setLowpass(12000.0f, 0.7071f, invSampleRate_);
            yL = warmLpL_.process(yL);
            yR = warmLpR_.process(yR);
            warmLpL_.clampState();
            warmLpR_.clampState();

            xL = yL;
            xR = yR;
        }
    }

private:
    float sampleRate_{48000.0f};
    float invSampleRate_{1.0f / 48000.0f};

    float fastAtt_{0.0f}, fastRel_{0.0f};
    float slowAtt_{0.0f}, slowRel_{0.0f};
    float distAtt_{0.0f}, distRel1_{0.0f}, distRel2_{0.0f};

    float fastEnvL_{0.0f}, fastEnvR_{0.0f};
    float slowEnvL_{0.0f}, slowEnvR_{0.0f};
    float distEnvL_{0.0f}, distEnvR_{0.0f};

    TbdAudio::DSPCommon::CytomicSVF mudLpL_{}, mudLpR_{};
    TbdAudio::DSPCommon::CytomicSVF headBumpLpL_{}, headBumpLpR_{};
    TbdAudio::DSPCommon::CytomicSVF warmLpL_{}, warmLpR_{};
};

} // namespace tkdsp
