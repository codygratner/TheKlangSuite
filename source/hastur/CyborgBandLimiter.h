#pragma once

#include "dsp/oscillators/PolyBLEP.h"
#include <algorithm>
#include <cmath>

namespace CrackedChimes::DSP {

/**
 * @brief Vintage 1980s Hybrid DCO Band-Limiter Emulation.
 * Provides calibrated industrial foldover bite without harsh unmusical chirps.
 */
class CyborgBandLimiter {
public:
    // 65% correction factor: leaves -22 dB alias floor for authentic industrial bite
    static constexpr float kSawBlepDepth    = 0.65f;
    // 50% linear correction: tames 8x chopper chirp runaway while preserving bite
    static constexpr float kChopperBlepDepth = 0.50f;

    /**
     * @brief Band-limited Rising Saw with 1980s DCO analog discharge curve.
     */
    [[nodiscard]] static inline float processSaw(float phase, float dt, int lifeMode) noexcept {
        const float p = TbdAudio::Oscillators::PolyBLEP::wrapPhase(phase);
        float v = 2.0f * p - 1.0f;

        if (lifeMode == 1) { // CYBORG: 65% scaled BLEP (calibrated grit)
            v -= kSawBlepDepth * TbdAudio::Oscillators::PolyBLEP::blep(p, dt);
        } else { // SYNTHETIC / ORGANIC: Pristine 100% PolyBLEP
            v -= TbdAudio::Oscillators::PolyBLEP::blep(p, dt);
        }
        return v;
    }

    /**
     * @brief Linear Single-Point BLEP for Chopper edges in Cyborg mode.
     */
    [[nodiscard]] static inline float linearBlep(float t, float dt) noexcept {
        if (dt <= 1e-7f) return 0.0f;
        if (t < dt) {
            const float u = t / dt;
            return (1.0f - u) * (1.0f - u);
        }
        return 0.0f;
    }

    /**
     * @brief Apply chopper transition anti-aliasing based on LifeMode.
     */
    [[nodiscard]] static inline float processChopperEdge(
        float stepHeight, 
        float phaseDist, 
        float dt, 
        int lifeMode) noexcept 
    {
        if (lifeMode == 1) { // CYBORG: Scaled Linear BLEP
            return stepHeight * kChopperBlepDepth * linearBlep(phaseDist, dt);
        }
        // SYNTHETIC / ORGANIC: Pristine 2-point PolyBLEP
        return stepHeight * TbdAudio::Oscillators::PolyBLEP::blep(phaseDist, dt);
    }
};

/**
 * @brief Vintage 17.5 kHz Analog Summing Bus Emulation (Active strictly in Cyborg Mode).
 */
class CyborgAnalogBusFilter {
public:
    void prepare(double sampleRate) noexcept {
        // Discrete summing op-amp 1-pole rolloff at 17.5 kHz
        const float fc = 17500.0f;
        const float w = 2.0f * 3.14159265358979323846f * fc / static_cast<float>(sampleRate > 1000.0 ? sampleRate : 44100.0);
        alpha_ = w / (1.0f + w);
        reset();
    }

    void reset() noexcept { state_ = 0.0f; }

    [[nodiscard]] inline float process(float x, bool isCyborg) noexcept {
        if (!isCyborg) return x; // Transparent bypass in Synthetic / Organic
        state_ += alpha_ * (x - state_);
        return state_;
    }

private:
    float alpha_{0.5f};
    float state_{0.0f};
};

} // namespace CrackedChimes::DSP
