#pragma once

#include "../common/AudioEffect.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace TbdAudio::Oscillators {

/**
 * @brief Band-limited Polynomial Transition (PolyBLEP & PolyBLAMP) and deterministic
 *        PRNG noise utilities for alias-free real-time synthesis.
 *        Zero heap allocations, zero CRT transcendental calls.
 */
namespace PolyBLEP {

/**
 * 2-point Polynomial Band-Limited Step (PolyBLEP) residual around discontinuity at t = 0 / 1.
 * @param t   Normalized phase in [0.0f, 1.0f)
 * @param dt  Normalized phase increment (freqHz * invSr) in (0.0f, 0.5f)
 */
[[nodiscard]] inline float blep(float t, float dt) noexcept {
    if (dt <= 1e-7f) return 0.0f;
    if (t < dt) {
        const float u = t / dt;
        return u + u - u * u - 1.0f;
    }
    if (t > 1.0f - dt) {
        const float u = (t - 1.0f) / dt;
        return u * u + u + u + 1.0f;
    }
    return 0.0f;
}

/**
 * Wrap phase into [0.0f, 1.0f) without std::floor.
 */
[[nodiscard]] inline float wrapPhase(float p) noexcept {
    p -= static_cast<float>(static_cast<int>(p));
    if (p < 0.0f) p += 1.0f;
    if (p >= 1.0f) p -= 1.0f;
    return p;
}

/**
 * Band-limited Rising Sawtooth in [-1.0f, +1.0f].
 */
[[nodiscard]] inline float saw(float phase, float dt) noexcept {
    const float p = wrapPhase(phase);
    float v = 2.0f * p - 1.0f;
    v -= blep(p, dt);
    return v;
}

/**
 * Band-limited Variable-Duty Pulse wave in [-1.0f, +1.0f].
 * @param duty Pulse width in [0.05f, 0.95f]
 */
[[nodiscard]] inline float pulse(float phase, float dt, float duty = 0.5f) noexcept {
    const float p = wrapPhase(phase);
    const float pw = std::clamp(duty, 0.03f, 0.97f);
    float v = (p < pw) ? 1.0f : -1.0f;
    v += blep(p, dt);
    v -= blep(wrapPhase(p - pw + 1.0f), dt);
    return v;
}

/**
 * Band-limited Triangle wave in [-1.0f, +1.0f].
 */
[[nodiscard]] inline float triangle(float phase) noexcept {
    const float p = wrapPhase(phase);
    return 1.0f - 4.0f * std::abs(p - 0.5f);
}

/**
 * Evaluate the 14 normalized Alpha DCO single-cycle waveforms [0..13].
 * All 14 shapes are normalized to a single fundamental period [0.0, 1.0) with PolyBLEP anti-aliasing.
 */
[[nodiscard]] inline float alphaWave14(int shapeIdx, float phase, float dt, float pwm) noexcept {
    const int idx = std::clamp(shapeIdx, 0, 13);
    const float p = wrapPhase(phase);
    const float pw = std::clamp(0.5f + 0.42f * pwm, 0.05f, 0.95f);

    switch (idx) {
        case 0: // Saw 1: Classic Linear PolyBLEP Saw
            return saw(p, dt);

        case 1: { // Saw 2: PWM-Windowed Saw (saw gated by pulse window)
            const float s = saw(p, dt);
            const float g = 0.5f * (pulse(p, dt, pw) + 1.0f);
            return s * g * 1.4f - 0.35f * (1.0f - g);
        }

        case 2: { // Saw 3: Stepped 2x XOR Saw (saw + octave square notch)
            const float s1 = saw(p, dt);
            const float sq2 = pulse(wrapPhase(p * 2.0f), dt * 2.0f, 0.5f);
            return 0.65f * s1 + 0.35f * sq2;
        }

        case 3: { // Saw 4: Stepped 4x Comb Saw
            const float s1 = saw(p, dt);
            const float sq4 = pulse(wrapPhase(p * 4.0f), std::min(dt * 4.0f, 0.48f), pw);
            return 0.6f * s1 + 0.4f * sq4;
        }

        case 4: { // Saw 5: Double-Saw (Alpha Hoover core: fundamental + phase-shifted 2x saw)
            const float s1 = saw(p, dt);
            const float s2 = saw(wrapPhase(p * 2.0f + 0.5f * pwm), std::min(dt * 2.0f, 0.48f));
            return 0.55f * s1 + 0.45f * s2;
        }

        case 5: // Square 1: Symmetrical 50% Square
            return pulse(p, dt, 0.5f);

        case 6: // Square 2: Variable PWM Pulse
            return pulse(p, dt, pw);

        case 7: // Square 3: Narrow Spike Pulse (clavinet / nasal reed)
            return pulse(p, dt, std::clamp(0.12f + 0.25f * pwm, 0.04f, 0.45f));

        case 8: { // Shape 9: Sub-Octave Divided Square + Fundamental
            const float sq1 = pulse(p, dt, 0.5f);
            const float sq2 = pulse(wrapPhase(p * 2.0f), std::min(dt * 2.0f, 0.48f), pw);
            return 0.55f * sq1 + 0.45f * sq2;
        }

        case 9: { // Shape 10: Sub PWM Notch
            const float sq1 = pulse(p, dt, pw);
            const float sq2 = pulse(wrapPhase(p * 2.0f), std::min(dt * 2.0f, 0.48f), 0.5f);
            return sq1 * sq2; // Unipolar XOR equivalent in [-1, +1]
        }

        case 10: { // Shape 11: Asymmetric Notch Pulse
            const float p1 = pulse(p, dt, 0.25f);
            const float p2 = pulse(wrapPhase(p + pw * 0.5f), dt, 0.25f);
            return 0.5f * (p1 - p2);
        }

        case 11: { // Shape 12: Dual Unequal Pulse
            const float p1 = pulse(p, dt, pw);
            const float p2 = pulse(wrapPhase(p * 3.0f), std::min(dt * 3.0f, 0.48f), 0.5f);
            return 0.6f * p1 + 0.4f * p2;
        }

        case 12: { // Shape 13: Stepped Staircase Pulse 1
            const float tri = triangle(p);
            const float sq2 = pulse(wrapPhase(p * 2.0f), std::min(dt * 2.0f, 0.48f), pw);
            return 0.55f * tri + 0.45f * sq2;
        }

        case 13:
        default: { // Shape 14: Stepped Staircase Pulse 2 (Metallic XOR)
            const float s1 = saw(p, dt);
            const float p2 = pulse(wrapPhase(p * 2.0f), std::min(dt * 2.0f, 0.48f), pw);
            const float p4 = pulse(wrapPhase(p * 4.0f), std::min(dt * 4.0f, 0.48f), 0.5f);
            return 0.45f * s1 + 0.35f * p2 + 0.20f * p4;
        }
    }
}

/**
 * Deterministic, lock-free Xorshift32 White & Pink Noise Generator for audio thread use.
 */
struct NoiseGenerator {
    uint32_t state = 0x13579BDFu;
    float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f;

    inline void reset(uint32_t seed = 0x13579BDFu) noexcept {
        state = (seed != 0u) ? seed : 0x13579BDFu;
        b0 = b1 = b2 = 0.0f;
    }

    /**
     * White noise in [-1.0f, +1.0f].
     */
    [[nodiscard]] inline float nextWhite() noexcept {
        uint32_t x = state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        state = x;
        return static_cast<float>(static_cast<int32_t>(x)) * (1.0f / 2147483648.0f);
    }

    /**
     * 3-stage Paul Kellet Pink Noise approximation in [-1.0f, +1.0f].
     */
    [[nodiscard]] inline float nextPink() noexcept {
        const float w = nextWhite();
        b0 = 0.99765f * b0 + w * 0.0990460f;
        b1 = 0.96300f * b1 + w * 0.2965164f;
        b2 = 0.57000f * b2 + w * 1.0526913f;
        const float pink = (b0 + b1 + b2 + w * 0.1848f) * 0.22f;
        return std::clamp(pink, -1.0f, 1.0f);
    }
};

} // namespace PolyBLEP
} // namespace TbdAudio::Oscillators
