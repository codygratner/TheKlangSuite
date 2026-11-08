#pragma once

#include <cmath>
#include <cstdint>
#include <algorithm>
#include <array>
#include "../../FastMath.h"
#include "DSPBlock.h"

namespace TbdAudio {

// ============================================================================
// 1. Canonical Perceptual Parameter Tapering Profiles (Profiles A – M)
//    Strictly zero CRT transcendentals (uses FastMath::fastPow2 / fastExp).
// ============================================================================
namespace Taper {

/**
 * Profile A: Temporal / Envelope Decay Taper (y = x^3 Cubic)
 */
[[nodiscard]] inline float profileA_Decay(float norm, float minSec = 0.010f, float maxSec = 60.0f) noexcept {
    float x = std::clamp(norm, 0.0f, 1.0f);
    return minSec + (maxSec - minSec) * (x * x * x);
}

/**
 * Profile B: Bipolar Sigmoidal S-Curve (Center plateau at 0.5, exponent 2.2)
 * Returns bipolar value in [-1.0f, +1.0f].
 */
[[nodiscard]] inline float profileB_Bipolar(float norm) noexcept {
    float x = std::clamp(norm, 0.0f, 1.0f);
    float u = 2.0f * (x - 0.5f);
    float absU = std::abs(u);
    if (absU < 1e-5f) return 0.0f;
    float sign = (u >= 0.0f) ? 1.0f : -1.0f;
    // |u|^2.2 via fastPow2(2.2 * fastLog2(|u|))
    return sign * FastMath::fastPow2(2.2f * FastMath::fastLog2(absU));
}

/**
 * Profile C: Wide-Range Modulator Speed (Dual-Regime Geometric Skew, 12:00 = 20 Hz)
 */
[[nodiscard]] inline float profileC_ModSpeed(float norm, float minHz = 0.1f, float maxHz = 5000.0f) noexcept {
    float x = std::clamp(norm, 0.0f, 1.0f);
    if (x <= 0.5f) {
        float t = x * 2.0f;
        float ratioLog2 = FastMath::fastLog2(20.0f / std::max(minHz, 0.001f));
        return minHz * FastMath::fastPow2(t * ratioLog2);
    } else {
        float t = (x - 0.5f) * 2.0f;
        float ratioLog2 = FastMath::fastLog2(std::max(maxHz, 20.0f) / 20.0f);
        return 20.0f * FastMath::fastPow2(t * ratioLog2);
    }
}

/**
 * Profile D: Equal-Octave Logarithmic Cutoff Frequency (20 Hz .. 20 kHz, 12:00 ~ 632 Hz)
 */
[[nodiscard]] inline float profileD_Cutoff(float norm, float minHz = 20.0f, float maxHz = 20000.0f) noexcept {
    float x = std::clamp(norm, 0.0f, 1.0f);
    float ratioLog2 = FastMath::fastLog2(std::max(maxHz, minHz) / std::max(minHz, 1.0f));
    return minHz * FastMath::fastPow2(x * ratioLog2);
}

/**
 * Profile E: Non-Linear Drive & Folding (Warmth Expansion y = x^1.8)
 * Returns drive in dB [0 .. maxDriveDb].
 */
[[nodiscard]] inline float profileE_DriveDb(float norm, float maxDriveDb = 36.0f) noexcept {
    float x = std::clamp(norm, 0.0f, 1.0f);
    if (x <= 1e-5f) return 0.0f;
    return maxDriveDb * FastMath::fastPow2(1.8f * FastMath::fastLog2(x));
}

/**
 * Profile E Linear Gain multiplier directly from normalized [0..1] knob.
 */
[[nodiscard]] inline float profileE_DriveGain(float norm, float minDb = 0.0f, float maxDb = 36.0f) noexcept {
    float x = std::clamp(norm, 0.0f, 1.0f);
    float shaped = (x <= 1e-5f) ? 0.0f : FastMath::fastPow2(1.8f * FastMath::fastLog2(x));
    float db = minDb + (maxDb - minDb) * shaped;
    return FastMath::fastDbToGain(db);
}

/**
 * Profile F: Runaway Feedback & Q-Factor (Hyperbolic End-Stop)
 */
[[nodiscard]] inline float profileF_Hyperbolic(float norm, float minVal = 0.0f, float maxVal = 1.15f) noexcept {
    float x = std::clamp(norm, 0.0f, 1.0f);
    constexpr float invDenom = 1.0f / (1.0f - 0.049787068f); // 1 / (1 - exp(-3))
    float num = 1.0f - FastMath::fastExp(-3.0f * x);
    return minVal + (maxVal - minVal) * (num * invDenom);
}

/**
 * Profile M: Universal Tri-Zone Parallel Mix Law & Phase Invert Protocol
 * - u = 0.0 (7:00):  Dry = 0.0, Wet = 1.0 (100% Pure Wet)
 * - u = 0.5 (12:00): Dry = 1.0, Wet = 0.0 (100% Pure Dry)
 * - u = 1.0 (5:00):  Dry = 1.0, Wet = 1.0 (100% Dry + 100% Parallel Wet)
 */
inline void profileM_TriZoneMix(float norm, bool phaseInvert, float& outDry, float& outWet) noexcept {
    float u = std::clamp(norm, 0.0f, 1.0f);
    float polarity = phaseInvert ? -1.0f : 1.0f;
    if (u <= 0.5f) {
        outDry = 2.0f * u;
        outWet = (1.0f - 2.0f * u) * polarity;
    } else {
        outDry = 1.0f;
        outWet = (2.0f * (u - 0.5f)) * polarity;
    }
}

} // namespace Taper

// ============================================================================
// 2. Real-Time Filter & Circuit Modeling Primitives (Zero-Allocation)
// ============================================================================
namespace DSPCommon {

/**
 * Fast tangent approximation for filter cutoff warping: tan(pi * fc / sr)
 * Clamped to Nyquist safe region [0.0001, 0.485] of sampleRate.
 */
[[nodiscard]] inline float fastTanPiNorm(float freqOverSr) noexcept {
    float f = std::clamp(freqOverSr, 0.00005f, 0.485f);
    float halfNorm = f * 0.5f; // Since fastSinNorm(1.0) = sin(2*pi), pi*f = 2*pi*(0.5*f)
    float s = FastMath::fastSinNorm(halfNorm);
    float c = FastMath::fastCosNorm(halfNorm);
    return s / std::max(c, 1e-5f);
}

/**
 * 1st-Order Zero-Delay Feedback (TPT) One-Pole Filter (LPF & HPF outputs).
 */
struct OnePoleTPT {
    float g = 0.01f;
    float s = 0.0f;

    inline void reset() noexcept { s = 0.0f; }

    inline void setCutoff(float cutoffHz, float invSr) noexcept {
        float wd = fastTanPiNorm(cutoffHz * invSr);
        g = wd / (1.0f + wd);
    }

    [[nodiscard]] inline float processLP(float x) noexcept {
        float v = (x - s) * g;
        float y = v + s;
        s = y + v;
        return y;
    }

    [[nodiscard]] inline float processHP(float x) noexcept {
        float v = (x - s) * g;
        float lp = v + s;
        s = lp + v;
        return x - lp;
    }
};

/**
 * 5 Hz DC Blocking High-Pass Filter.
 */
struct DCBlocker {
    float x1 = 0.0f;
    float y1 = 0.0f;
    float R = 0.9993f;

    inline void reset() noexcept { x1 = 0.0f; y1 = 0.0f; }

    inline void init(float sampleRate) noexcept {
        R = 1.0f - (FastMath::TWO_PI * 5.0f / std::max(sampleRate, 8000.0f));
        R = std::clamp(R, 0.98f, 0.9998f);
    }

    [[nodiscard]] inline float process(float x) noexcept {
        float y = x - x1 + R * y1;
        x1 = x;
        y1 = y;
        return y;
    }
};

/**
 * Cytomic Zero-Delay Feedback State Variable Filter (SVF) & Gyrator Peaking EQ.
 * Unconditionally stable under rapid modulation; zero CRT trig calls.
 */
struct CytomicSVF {
    float ic1eq = 0.0f;
    float ic2eq = 0.0f;
    float a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;
    float m0 = 0.0f, m1 = 0.0f, m2 = 0.0f;

    inline void reset() noexcept {
        ic1eq = 0.0f;
        ic2eq = 0.0f;
    }

    inline void clampState() noexcept {
        if (!std::isfinite(ic1eq) || std::abs(ic1eq) > 16.0f) ic1eq = std::clamp(ic1eq, -16.0f, 16.0f);
        if (!std::isfinite(ic2eq) || std::abs(ic2eq) > 16.0f) ic2eq = std::clamp(ic2eq, -16.0f, 16.0f);
    }

    inline void setLowpass(float cutoffHz, float q, float invSr) noexcept {
        float g = fastTanPiNorm(cutoffHz * invSr);
        float k = 1.0f / std::clamp(q, 0.1f, 30.0f);
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
        m0 = 0.0f; m1 = 0.0f; m2 = 1.0f;
    }

    inline void setHighpass(float cutoffHz, float q, float invSr) noexcept {
        float g = fastTanPiNorm(cutoffHz * invSr);
        float k = 1.0f / std::clamp(q, 0.1f, 30.0f);
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
        m0 = 1.0f; m1 = -k; m2 = -1.0f;
    }

    inline void setBandpass(float cutoffHz, float q, float invSr) noexcept {
        float g = fastTanPiNorm(cutoffHz * invSr);
        float k = 1.0f / std::clamp(q, 0.1f, 30.0f);
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
        m0 = 0.0f; m1 = 1.0f; m2 = 0.0f;
    }

    inline void setNotch(float cutoffHz, float q, float invSr) noexcept {
        float g = fastTanPiNorm(cutoffHz * invSr);
        float k = 1.0f / std::clamp(q, 0.1f, 30.0f);
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
        m0 = 1.0f; m1 = -k; m2 = 0.0f;
    }

    /**
     * Gyrator / Bell Peaking EQ stage (gainDb in [-24, +24]).
     */
    inline void setBell(float centerHz, float q, float gainDb, float invSr) noexcept {
        float A = FastMath::fastPow2(gainDb * (FastMath::DB_TO_LOG2 * 0.5f)); // 10^(gainDb / 40)
        float g = fastTanPiNorm(centerHz * invSr);
        float k = 1.0f / (std::clamp(q, 0.1f, 30.0f) * A);
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
        m0 = 1.0f;
        m1 = k * (A * A - 1.0f);
        m2 = 0.0f;
    }

    /**
     * Active Low Shelf filter (gainDb in [-24, +24]).
     */
    inline void setLowShelf(float cornerHz, float q, float gainDb, float invSr) noexcept {
        float A = FastMath::fastPow2(gainDb * (FastMath::DB_TO_LOG2 * 0.5f));
        float sqrtA = std::sqrt(std::max(A, 1e-4f));
        float g = fastTanPiNorm(cornerHz * invSr) / sqrtA;
        float k = 1.0f / std::clamp(q, 0.1f, 20.0f);
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
        m0 = 1.0f;
        m1 = k * (A - 1.0f);
        m2 = A * A - 1.0f;
    }

    /**
     * Active High Shelf filter (gainDb in [-24, +24]).
     */
    inline void setHighShelf(float cornerHz, float q, float gainDb, float invSr) noexcept {
        float A = FastMath::fastPow2(gainDb * (FastMath::DB_TO_LOG2 * 0.5f));
        float sqrtA = std::sqrt(std::max(A, 1e-4f));
        float g = fastTanPiNorm(cornerHz * invSr) * sqrtA;
        float k = 1.0f / std::clamp(q, 0.1f, 20.0f);
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
        m0 = A * A;
        m1 = k * (1.0f - A) * A;
        m2 = 1.0f - A * A;
    }

    [[nodiscard]] inline float process(float v0) noexcept {
        float v3 = v0 - ic2eq;
        float v1 = a1 * ic1eq + a2 * v3;
        float v2 = ic2eq + a2 * ic1eq + a3 * v3;
        ic1eq = 2.0f * v1 - ic1eq;
        ic2eq = 2.0f * v2 - ic2eq;
        return m0 * v0 + m1 * v1 + m2 * v2;
    }
};

/**
 * Zero-Allocation 4x Polyphase All-Pass IIR Half-Band Oversampler.
 * Essential for alias-free high-gain clipping and HM-2 crossover dead-band stages.
 */
struct Oversampler4x {
    struct HalfBandStage {
        float a0 = 0.0f, a1 = 0.0f;
        float b0 = 0.0f, b1 = 0.0f;
        float prevOut = 0.0f;

        inline void reset() noexcept {
            a0 = a1 = b0 = b1 = prevOut = 0.0f;
        }

        // 2-branch polyphase all-pass coefficients (steep transition band)
        static constexpr float cA0 = 0.0798664262f;
        static constexpr float cA1 = 0.5453536511f;
        static constexpr float cB0 = 0.2838293449f;
        static constexpr float cB1 = 0.8344118915f;

        inline void upsample(float x, float& y0, float& y1) noexcept {
            // Branch A
            float aTmp0 = cA0 * (x - a0) + prevOut; // simplified first-order allpass cascade
            float aOut0 = cA0 * x + a0 - cA0 * aTmp0;
            a0 = x;
            float aOut1 = cA1 * aOut0 + a1 - cA1 * a1;
            a1 = aOut0;

            // Branch B
            float bOut0 = cB0 * x + b0 - cB0 * b0;
            b0 = x;
            float bOut1 = cB1 * bOut0 + b1 - cB1 * b1;
            b1 = bOut0;

            y0 = aOut1;
            y1 = bOut1;
        }

        [[nodiscard]] inline float downsample(float x0, float x1) noexcept {
            float aOut0 = cA0 * x0 + a0 - cA0 * a0;
            a0 = x0;
            float aOut1 = cA1 * aOut0 + a1 - cA1 * a1;
            a1 = aOut0;

            float bOut0 = cB0 * x1 + b0 - cB0 * b0;
            b0 = x1;
            float bOut1 = cB1 * bOut0 + b1 - cB1 * b1;
            b1 = bOut0;

            return 0.5f * (aOut1 + bOut1);
        }
    };

    HalfBandStage up1, up2a, up2b;
    HalfBandStage down2a, down2b, down1;

    inline void reset() noexcept {
        up1.reset(); up2a.reset(); up2b.reset();
        down2a.reset(); down2b.reset(); down1.reset();
    }

    template <typename Callback>
    [[nodiscard]] inline float processSample(float x, Callback&& nonLinearFn) noexcept {
        float u0, u1;
        up1.upsample(x, u0, u1);
        float s0, s1, s2, s3;
        up2a.upsample(u0, s0, s1);
        up2b.upsample(u1, s2, s3);

        s0 = nonLinearFn(s0);
        s1 = nonLinearFn(s1);
        s2 = nonLinearFn(s2);
        s3 = nonLinearFn(s3);

        float d0 = down2a.downsample(s0, s1);
        float d1 = down2b.downsample(s2, s3);
        return down1.downsample(d0, d1);
    }
};

} // namespace DSPCommon

// ============================================================================
// 3. Universal 8-Control Audio Effect Base Class (AudioEffect8)
//    Extends TbdAudio::DSPBlock to support the 8-knob Universal Card standard
//    and Profile M Tri-Zone Parallel Mix + Phase Invert.
// ============================================================================
class AudioEffect8 : public DSPBlock {
public:
    static constexpr int NUM_PARAMS = 8;

    AudioEffect8() noexcept {
        for (int i = 0; i < NUM_PARAMS; ++i) {
            baseParams8[i] = 0.5f;
            modParams8[i] = 0.0f;
            params8[i] = 0.5f;
        }
    }

    ~AudioEffect8() override = default;

    void setParam(int index, float value) override {
        if (index >= 0 && index < NUM_PARAMS) {
            baseParams8[index] = std::clamp(value, 0.0f, 1.0f);
            params8[index] = std::clamp(baseParams8[index] + modParams8[index], 0.0f, 1.0f);
            if (index < 4) {
                DSPBlock::setParam(index, value);
            }
        }
    }

    void setParamModulation(int index, float offset) override {
        if (index >= 0 && index < NUM_PARAMS) {
            modParams8[index] = offset;
            params8[index] = std::clamp(baseParams8[index] + modParams8[index], 0.0f, 1.0f);
            if (index < 4) {
                DSPBlock::setParamModulation(index, offset);
            }
        }
    }

    float getParam(int index) const override {
        return (index >= 0 && index < NUM_PARAMS) ? baseParams8[index] : 0.0f;
    }

    inline void setPhaseInvert(bool invert) noexcept { phaseInvert = invert; }
    [[nodiscard]] inline bool getPhaseInvert() const noexcept { return phaseInvert; }

protected:
    float baseParams8[NUM_PARAMS] = { 0.5f, 0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.0f };
    float modParams8[NUM_PARAMS]  = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    float params8[NUM_PARAMS]     = { 0.5f, 0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.0f };
    bool phaseInvert = false;
};

} // namespace TbdAudio
