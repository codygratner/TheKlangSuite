#pragma once

#include <algorithm>
#include <cmath>

namespace tkdsp::formless {

/**
 * Headroom-Proportional Bipolar Scaling
 * Maps macro [0..1] with 0.5 neutral onto base parameter [0..1].
 * When m >= 0: lerps base toward 1.0 using remaining upper headroom.
 * When m < 0:  lerps base toward 0.0 using remaining lower headroom.
 * Eliminates clipping dead-zones when base is near extremes.
 */
inline constexpr float headroomLerp(float base, float macroNorm, float depth = 1.0f) noexcept {
    base = std::clamp(base, 0.0f, 1.0f);
    macroNorm = std::clamp(macroNorm, 0.0f, 1.0f);
    depth = std::clamp(depth, 0.0f, 1.0f);

    const float m = std::clamp(2.0f * (macroNorm - 0.5f), -1.0f, 1.0f) * depth;
    if (m >= 0.0f) {
        return base + (1.0f - base) * m;
    } else {
        return base + base * m; // m is negative: base * (1 + m)
    }
}

/**
 * Sequential Arbitration:
 * Stage 1: Macro scales base to patch operating point (basePrime).
 * Stage 2: Velocity dynamically modulates basePrime per hit.
 */
inline constexpr float sequentialArbitrate(float base, float macroNorm, float macroDepth,
                                          float velAmount, float velDepth, float hitVelocity) noexcept {
    const float basePrime = headroomLerp(base, macroNorm, macroDepth);
    if (velDepth <= 0.0001f || hitVelocity <= 0.0001f) return basePrime;

    const float vThrow = (velAmount - 0.5f) * 2.0f * std::clamp(hitVelocity, 0.0f, 1.0f) * velDepth;
    if (vThrow >= 0.0f) {
        return basePrime + (1.0f - basePrime) * vThrow;
    } else {
        return basePrime + basePrime * vThrow;
    }
}

/**
 * Range structure for GUI Arc Halos
 */
struct HaloRange {
    float lo{0.0f};
    float hi{0.0f};
    float pip{0.0f};
    bool active{false};
};

/**
 * Computes the arc halo bounds for a knob driven by a macro.
 */
inline HaloRange computeMacroHalo(float baseVal, float macroNorm, float depth = 1.0f) noexcept {
    HaloRange r;
    const float eff = headroomLerp(baseVal, macroNorm, depth);
    r.lo = std::min(baseVal, eff);
    r.hi = std::max(baseVal, eff);
    r.pip = eff;
    const float diff = eff - baseVal;
    r.active = ((diff < 0.0f ? -diff : diff) > 0.002f);
    return r;
}

} // namespace tkdsp::formless
