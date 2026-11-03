#pragma once

#include <cmath>
#include <algorithm>

namespace CrackedChimes::DSP {

struct LeakyDCBlocker {
    float x1 = 0.0f;
    float y1 = 0.0f;
    float R = 0.9992f; // Defaults to ~10 Hz at 48 kHz
    inline void prepare(double fs, double fcHz = 5.0) noexcept {
        R = static_cast<float>(std::exp(-6.28318530717958647692 * fcHz / std::max(fs, 1000.0)));
    }
    inline float process(float x) noexcept {
        const float y = x - x1 + R * y1;
        x1 = x;
        y1 = y;
        return y;
    }
    inline void reset() noexcept { x1 = 0.0f; y1 = 0.0f; }
};

struct OnePoleHP {
    float x1 = 0.0f;
    float y1 = 0.0f;
    float a = 0.984f; // ~120 Hz at 48 kHz
    inline void setCutoff(float fc, float fs) noexcept {
        const float w = 6.28318530717958647692f * fc / std::max(fs, 1000.0f);
        a = std::exp(-w);
    }
    inline float process(float x) noexcept {
        const float y = x - x1 + a * y1;
        x1 = x;
        y1 = y;
        return y;
    }
    inline void reset() noexcept { x1 = 0.0f; y1 = 0.0f; }
};

struct OnePoleLP {
    float y1 = 0.0f;
    float a = 0.15f; // ~14.5 kHz damping at 48 kHz
    inline void setCutoff(float fc, float fs) noexcept {
        const float w = 6.28318530717958647692f * fc / std::max(fs, 1000.0f);
        a = std::exp(-w);
    }
    inline float process(float x) noexcept {
        y1 = (1.0f - a) * x + a * y1;
        return y1;
    }
    inline void reset() noexcept { y1 = 0.0f; }
};

} // namespace CrackedChimes::DSP
