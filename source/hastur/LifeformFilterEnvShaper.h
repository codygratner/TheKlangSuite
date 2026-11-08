#pragma once
#include <cmath>
#include <algorithm>

namespace Hastur {

/**
 * LifeformFilterEnvShaper: 36-byte L1-cache resident filter envelope shaper.
 * Models lifeform-specific analog CV circuit physics:
 * - Synthetic: Instant convex logarithmic attack p*(2-p), zero capacitive lag, 100% precision.
 * - Cyborg: Power rail starvation (up to 16% dip), Padé diode apex clamping at 0.82V, quadratic deadband below 70mV.
 * - Organic: CdS photoresistor deep-trap memory (+5..12% floating baseline), dual-exponential decay tail, power-law conductance warping.
 */
class LifeformFilterEnvShaper {
public:
    void prepare(double sampleRate) noexcept {
        sampleRate_ = sampleRate > 1000.0 ? sampleRate : 44100.0;
        const float dt = static_cast<float>(1.0 / sampleRate_);
        alphaWarm_ = 1.0f - std::exp(-dt / 0.140f);
        alphaCool_ = 1.0f - std::exp(-dt / 0.450f);
        alphaFast_ = 1.0f - std::exp(-dt / 0.028f);
        alphaSlow_ = 1.0f - std::exp(-dt / 0.380f);
        alphaAtt_  = 1.0f - std::exp(-dt / 0.0012f);
        reset();
    }

    void reset() noexcept {
        vactrolMem_  = 0.0f;
        vactrolTail_ = 0.0f;
        vactrolFast_ = 0.0f;
        prevRaw_     = 0.0f;
    }

    [[nodiscard]] float shapeEnvelope(float rawEnv, int lifeMode) noexcept {
        // Mode 0: Synthetic - Pure instant convex attack, zero memory lag
        if (lifeMode == 0) {
            const float p = std::clamp(rawEnv, 0.0f, 1.0f);
            return p * (2.0f - p);
        }

        // Mode 1: Cyborg - Power rail starvation + Diode apex clamp + Deadband
        if (lifeMode == 1) {
            const float dCV = rawEnv - prevRaw_;
            prevRaw_ = rawEnv;
            // Rail starvation dip on steep attack
            const float sag = (dCV > 0.0f) ? (0.16f * dCV * rawEnv) : 0.0f;
            const float sagged = std::clamp(rawEnv - sag, 0.0f, 1.0f);
            // Padé tanh soft-clip at diode knee Vknee = 0.82
            float clamped = sagged;
            if (clamped > 0.82f) {
                const float x = (clamped - 0.82f) / 0.18f;
                clamped = 0.82f + 0.18f * (x / (1.0f + 0.333f * x * x));
            }
            // Quadratic cutoff deadband below 70mV
            if (clamped < 0.070f) {
                const float norm = clamped / 0.070f;
                clamped = 0.070f * norm * norm;
            }
            return std::clamp(clamped, 0.0f, 1.0f);
        }

        // Mode 2: Organic - CdS photoresistor deep-trap accumulator + dual tail
        const float targetMem = rawEnv * rawEnv;
        if (targetMem > vactrolMem_) {
            vactrolMem_ += alphaWarm_ * (targetMem - vactrolMem_);
        } else {
            vactrolMem_ += alphaCool_ * (targetMem - vactrolMem_);
        }

        if (rawEnv > vactrolTail_) {
            vactrolTail_ += alphaAtt_ * (rawEnv - vactrolTail_);
            vactrolFast_ = vactrolTail_;
        } else {
            vactrolFast_ += alphaFast_ * (rawEnv - vactrolFast_);
            vactrolTail_ += alphaSlow_ * (rawEnv - vactrolTail_);
        }

        const float tail = 0.75f * vactrolFast_ + 0.25f * vactrolTail_;
        // Baseline float shift + power-law conductance warping
        const float s = std::clamp(tail + 0.12f * vactrolMem_, 0.0f, 1.0f);
        return s * (0.85f + 0.15f * s);
    }

private:
    double sampleRate_{44100.0};
    float alphaWarm_{0.001f};
    float alphaCool_{0.0003f};
    float alphaFast_{0.01f};
    float alphaSlow_{0.001f};
    float alphaAtt_{0.1f};
    float vactrolMem_{0.0f};
    float vactrolTail_{0.0f};
    float vactrolFast_{0.0f};
    float prevRaw_{0.0f};
};

} // namespace Hastur
