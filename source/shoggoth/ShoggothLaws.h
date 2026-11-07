#pragma once

#include <cmath>
#include <algorithm>
#include "shoggoth_layout.h"
#include "FastMath.h"

namespace Shoggoth {

class Laws {
public:
    // ==============================================================================
    // 1. ADP1: Schroeder Dispersion & Energy-Matched Extension Gain
    // ==============================================================================
    static inline float computeSchroederPhaseTurns(int k, int kNyq, float occupancy) noexcept {
        if (k <= kMax1987Partials) return 0.0f;
        const float kExt = static_cast<float>(kNyq - kMax1987Partials);
        if (kExt <= 0.0f) return 0.0f;
        const float deltaK = static_cast<float>(k - kMax1987Partials);
        const float phaseTurns = occupancy * 0.5f * (deltaK * deltaK) / kExt;
        return phaseTurns - std::floor(phaseTurns); // Wrap to [0, 1)
    }

    static inline float computeExtensionGain(float e126, float eTot) noexcept {
        if (eTot <= 1e-12f) return 1.0f;
        const float ratio = e126 / eTot;
        const float g = std::sqrt(std::max(0.0f, ratio));
        return std::clamp(g, 0.25f, 1.0f);
    }

    // ==============================================================================
    // 2. GST1: Air-Absorption & Modal Release Velocity
    // ==============================================================================
    static inline float computeVelocityBrightnessWeight(float fHz, int midiNote, float velocity) noexcept {
        // v = 1 is neutral (weight = 1.0)
        const float v2 = velocity * velocity;
        const float vFactor = 1.0f - v2;
        if (vFactor <= 1e-6f) return 1.0f;

        const float fRatio = fHz / 6000.0f;
        const float lambda = 3.0f * TbdAudio::FastMath::fastPow2(0.25f * static_cast<float>(midiNote - 60) / 12.0f);
        const float exponent = -lambda * vFactor * (fRatio * fRatio);
        return TbdAudio::FastMath::fastPow2(exponent);
    }

    static inline float computeModalReleaseMultiplier(float fHz, float releaseVelocity) noexcept {
        const float rv = std::clamp(releaseVelocity, 0.0f, 127.0f);
        const float velTerm = TbdAudio::FastMath::fastPow2((64.0f - rv) / 32.0f);
        const float freqRatio = std::max(0.0f, fHz / 2000.0f);
        const float freqTerm = std::sqrt(1.0f + freqRatio);
        return velTerm * freqTerm;
    }

    // ==============================================================================
    // 3. SYN1: Wavefold Energy Conservation & Event Horizon
    // ==============================================================================
    static inline float foldFrequency(float fIn, float foldWallHz) noexcept {
        if (foldWallHz <= 100.0f || fIn <= foldWallHz) return fIn;
        const float period = 2.0f * foldWallHz;
        const float norm = fIn / period + 0.5f;
        const float frac = norm - std::floor(norm);
        return foldWallHz * std::abs(2.0f * frac - 1.0f);
    }

    static inline float computeWavefoldGainComp(float eDir, float eFold) noexcept {
        const float denom = std::max(eDir, 1e-12f);
        const float val = 1.0f + eFold / denom;
        return 1.0f / std::sqrt(val);
    }

    // ==============================================================================
    // 4. GOV1: Absolute Threshold of Hearing (ATH)
    // ==============================================================================
    static inline float computeATHdB(float fHz) noexcept {
        const float fKhz = std::clamp(fHz * 0.001f, 0.02f, 24.0f);
        const float term1 = 3.64f * std::pow(fKhz, -0.8f);
        const float d = fKhz - 3.3f;
        const float term2 = -6.5f * std::exp(-0.6f * d * d);
        const float term3 = 0.001f * (fKhz * fKhz * fKhz * fKhz);
        return term1 + term2 + term3; // in dB SPL
    }

    static inline float computeATHGain(float fHz, float poolLoadPercent) noexcept {
        // As load reaches 100%, threshold slides upward to aggressively cull masked partials
        const float baseAthDb = computeATHdB(fHz);
        const float dynamicOffset = (poolLoadPercent >= 1.0f) ? 40.0f : (poolLoadPercent * 40.0f);
        const float thresholdDb = -96.0f + baseAthDb * 0.7f + dynamicOffset;
        return TbdAudio::FastMath::fastDbToGain(thresholdDb);
    }
};

} // namespace Shoggoth
