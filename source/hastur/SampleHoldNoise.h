#pragma once

#include <cmath>
#include <algorithm>
#include <cstdint>

namespace CrackedChimes::DSP {

/**
 * @brief High-quality Anti-Aliased White Noise Generator.
 */
class SampleHoldNoise {
public:
    void prepare(double sampleRate) noexcept {
        sampleRate_ = (sampleRate > 1000.0) ? sampleRate : 44100.0;
        reset();
    }

    void reset() noexcept {
        prng_ = 0x5EEDACE1u;
    }

    /**
     * @brief Process one sample of pure full-spectrum white noise.
     * @return Uniform random sample in [-1.0, 1.0].
     */
    [[nodiscard]] float process() noexcept {
        prng_ = prng_ * 1664525u + 1013904223u;
        return static_cast<float>(static_cast<int32_t>(prng_)) * 4.6566129e-10f;
    }

    // Overload for backwards compatibility
    [[nodiscard]] float process(float /*rateHz*/) noexcept {
        return process();
    }

private:
    double sampleRate_ = 44100.0;
    uint32_t prng_ = 0x5EEDACE1u;
};

} // namespace CrackedChimes::DSP
