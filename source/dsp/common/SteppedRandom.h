#pragma once

#include <algorithm>
#include <cmath>

namespace CrackedChimes::DSP {

/**
 * Destination-anchored stepped random lerp for normalized [0, 1] parameter space.
 * v_eff = lerp(k, r, a) = (1 - a) * k + a * r
 * Window bounds: [k * (1 - a), k * (1 - a) + a]
 * Span is always strictly invariant: Delta = a.
 */
inline constexpr float stepRandomNorm(float kNorm, float amount, float r) noexcept {
    kNorm = std::clamp(kNorm, 0.0f, 1.0f);
    amount = std::clamp(amount, 0.0f, 1.0f);
    r = std::clamp(r, 0.0f, 1.0f);
    return (1.0f - amount) * kNorm + amount * r;
}

/**
 * Destination-anchored stepped random lerp for unnormalized physical parameter ranges [lo, hi].
 */
inline constexpr float stepRandomRange(float v, float lo, float hi, float amount, float r) noexcept {
    if (hi <= lo) return lo;
    const float kNorm = std::clamp((v - lo) / (hi - lo), 0.0f, 1.0f);
    const float effNorm = stepRandomNorm(kNorm, amount, r);
    return lo + effNorm * (hi - lo);
}

/**
 * GUI Radar Halo computation for destination-anchored stepped random.
 */
struct StepRandHalo {
    float lo{0.0f};
    float hi{0.0f};
    float pip{0.0f};
    bool active{false};
};

inline constexpr StepRandHalo computeStepRandHalo(float kNorm, float amount, float r) noexcept {
    kNorm = std::clamp(kNorm, 0.0f, 1.0f);
    amount = std::clamp(amount, 0.0f, 1.0f);
    r = std::clamp(r, 0.0f, 1.0f);

    StepRandHalo h;
    h.lo = kNorm * (1.0f - amount);
    h.hi = h.lo + amount;
    h.pip = (1.0f - amount) * kNorm + amount * r;
    h.active = (amount > 0.002f);
    return h;
}

} // namespace CrackedChimes::DSP
