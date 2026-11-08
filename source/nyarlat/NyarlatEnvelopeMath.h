#pragma once

#include <cmath>
#include <algorithm>

namespace Nyarlat::EnvMath {

    constexpr double kLn4 = 1.3862943611198906188;
    constexpr double kRail = 1.0 / 3.0; // Target overshoot/undershoot ratio (4/3 attack, s - (1-s)/3 decay)
    constexpr double kReleaseK = 9.210340371976184; // -80 dB release floor (1e-4)

    // Closed-form inverse progress u in [0, 1] given current level L
    inline float attackProgress(float L) noexcept {
        const float norm = L / (1.0f + static_cast<float>(kRail));
        return -std::log(std::max(1e-5f, 1.0f - norm)) / static_cast<float>(kLn4);
    }

    inline float decayProgress(float L, float s) noexcept {
        const float t = s - (1.0f - s) * static_cast<float>(kRail);
        if (std::abs(L - t) < 1e-5f) return 1.0f;
        return std::log(std::max(1e-5f, (1.0f - t) / (std::max(L, s) - t))) / static_cast<float>(kLn4);
    }

    inline float releaseProgress(float L, float Lrel) noexcept {
        const float denom = std::max(Lrel, 1e-4f);
        if (L <= 1e-5f) return 1.0f;
        return -std::log(std::clamp(L / denom, 1e-4f, 1.0f)) / static_cast<float>(kReleaseK);
    }

} // namespace Nyarlat::EnvMath
