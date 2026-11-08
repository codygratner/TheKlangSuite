#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include "FastMath.h"

namespace tkdsp::common {

namespace FastMath = TbdAudio::FastMath;

/**
 * ==============================================================================
 * MasterTaper.h — Universal Master Volume Taper Canon (Anti-Hollow Standard)
 * ==============================================================================
 * Part of The Klang Suite SOCEP Engine. Copyright (c) 2026 Klang Industries.
 *
 * u in [0.0, 1.0]:
 *   u == 0.0 -> silence (-inf dB, linear 0.0f)
 *   u == 0.5 -> bit-exact unity (0.0 dB, linear 1.0f)
 *   u == 1.0 -> +18.0 dB clean boost (linear 7.94328235f)
 *
 * Derivative Continuity:
 *   Lower slope at 0.5- = 34.74 dB/unit
 *   Upper slope at 0.5+ = 36.00 dB/unit
 *   Ratio = 0.965 (near-C1 smooth, zero audible kink)
 * ==============================================================================
 */

inline constexpr float kMasterMaxDb   = 18.0f;
inline constexpr float kMasterMaxGain = 7.94328235f; // 10^(18/20)
inline constexpr float kEmergencyGuard = 31.622777f;  // +30.0 dBFS emergency guard

[[nodiscard]] inline float evalMasterVolumeGain(float u) noexcept {
    if (!(u > 0.0f)) return 0.0f;
    const float cu = std::min(u, 1.0f);
    if (cu == 0.5f) return 1.0f; // Bit-exact bypass (zero pow rounding error)
    if (cu < 0.5f) {
        const float t = 2.0f * cu;
        return t * t; // Quadratic fader taper (-inf .. 0 dB)
    }
    // Upper half: linear in dB (+18.0 dB at u=1.0)
    return FastMath::fastDbToGain(kMasterMaxDb * (2.0f * cu - 1.0f));
}

[[nodiscard]] inline float evalMasterVolumeDb(float u) noexcept {
    if (!(u > 0.0f)) return -std::numeric_limits<float>::infinity();
    const float cu = std::min(u, 1.0f);
    if (cu == 0.5f) return 0.0f;
    return (cu < 0.5f) ? 12.0411998f * FastMath::fastLog2(2.0f * cu) : kMasterMaxDb * (2.0f * cu - 1.0f);
}

/**
 * Emergency Float Guard: Transparent pass-through up to +30 dBFS.
 * Never soft-clips normal or driven signals (+18 dB + polyphony passes 100% bit-transparent).
 * Catches NaNs, Infs, or runaway numerical instability without hard ceilings at 0 dBFS.
 */
inline void applyEmergencyGuard(float& sample) noexcept {
    if (!(std::abs(sample) <= kEmergencyGuard)) {
        sample = std::isfinite(sample) ? std::copysign(kEmergencyGuard, sample) : 0.0f;
    }
}

inline void applyEmergencyGuardStereo(float& l, float& r) noexcept {
    applyEmergencyGuard(l);
    applyEmergencyGuard(r);
}

} // namespace tkdsp::common
