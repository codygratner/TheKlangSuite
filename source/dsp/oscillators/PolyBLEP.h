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
 * Evaluate the 14 authentic Roland Alpha Juno (MB87123) DCO single-cycle waveforms [0..13]:
 * - Group 1 (Pulse): 0: Pulse 1 (50% Square), 1: Pulse 2 (25% Narrow), 2: Pulse 3 (PWM Pulse)
 * - Group 2 (Saw):   3: Saw 1 (Pure Saw), 4: Saw 2 (Chopped 2x), 5: Saw 3 (PWM Saw), 6: Saw 4 (Chopped 4x), 7: Saw 5 (Hoover Saw)
 * - Group 3 (Sub):   8: Sub 1 (Square /2), 9: Sub 2 (Pulse /2), 10: Sub 3 (Sub Chopped 2x), 11: Sub 4 (Sub Chopped 4x), 12: Sub 5 (Square /4), 13: Sub 6 (Pulse /4)
 * 
 * SSoT: Roland Alpha Juno-1/2/MKS-50 Service Manuals, 12 MHz divided DCO XOR logic, and Cody's Vitaltable specs.
 */
[[nodiscard]] inline float alphaWave14(int shapeIdx, float phase, float dt, float pwm) noexcept {
    const int idx = std::clamp(shapeIdx, 0, 13);
    const float p = wrapPhase(phase);
    const float pw = std::clamp(0.5f + 0.45f * pwm, 0.05f, 0.95f);

    switch (idx) {
        // --- Group 1: Pulse Waveforms ---
        case 0: // Pulse 1: Symmetrical 50% Square
            return pulse(p, dt, 0.5f);

        case 1: // Pulse 2: Fixed Narrow 25% Pulse
            return pulse(p, dt, 0.25f);

        case 2: // Pulse 3: Variable PWM Pulse
            return pulse(p, dt, pw);

        // --- Group 2: Sawtooth Waveforms ---
        case 3: // Saw 1: Pure Linear Rising Sawtooth
            return saw(p, dt);

        case 4: { // Saw 2: Chopped Saw 2x (Rising Saw smashed to 0 by +1 oct square)
            const float s_uni = p;
            const float p2 = wrapPhase(p * 2.0f);
            const float u = (p2 < 0.5f) ? s_uni : 0.0f;
            constexpr float mean_dc = 3.0f / 16.0f; // 0.1875
            constexpr float scale = 16.0f / 9.0f;   // Peak normalized to 1.0
            float v = (u - mean_dc) * scale;
            // PolyBLEP anti-aliasing on step transitions
            v -= blep(p, dt) * (0.75f * scale);
            v -= blep(wrapPhase(p - 0.25f + 1.0f), dt) * (0.25f * scale);
            v += blep(wrapPhase(p - 0.50f + 1.0f), dt) * (0.50f * scale);
            v -= blep(wrapPhase(p - 0.75f + 1.0f), dt) * (0.75f * scale);
            return v;
        }

        case 5: { // Saw 3: PWM Saw (Authentic Roland Alpha Juno / TAL-Pha)
            const float s_uni = p;
            const float p2 = wrapPhase(p * 2.0f);
            const float u = (p2 < pw) ? s_uni : 0.0f;
            const float mean_dc = 0.25f * pw * (1.0f + pw);
            const float peak_dev = std::max(0.5f + pw * 0.5f - mean_dc, mean_dc);
            const float scale = 1.0f / std::max(0.01f, peak_dev);
            float v = (u - mean_dc) * scale;
            v -= blep(p, dt) * (0.5f * scale);
            v -= blep(wrapPhase(p - pw * 0.5f + 1.0f), dt) * (pw * 0.5f * scale);
            v += blep(wrapPhase(p - 0.50f + 1.0f), dt) * (0.5f * scale);
            v -= blep(wrapPhase(p - (0.50f + pw * 0.5f) + 1.0f), dt) * ((0.5f + pw * 0.5f) * scale);
            return v;
        }

        case 6: { // Saw 4: Chopped Saw 8x (Rising Saw smashed to 0 by +3 oct square)
            const float s_uni = p;
            const float p8 = wrapPhase(p * 8.0f);
            const float u = (p8 < 0.5f) ? s_uni : 0.0f;
            constexpr float mean_dc = 15.0f / 64.0f;
            constexpr float scale = 64.0f / 49.0f;
            float v = (u - mean_dc) * scale;
            v -= blep(p, dt) * (0.5f * scale);
            v -= blep(p8, std::min(dt * 8.0f, 0.48f)) * (0.5f * scale);
            v += blep(wrapPhase(p8 - 0.5f + 1.0f), std::min(dt * 8.0f, 0.48f)) * (0.5f * scale);
            return v;
        }

        case 7: { // Saw 5: Hoover Saw (Chopped by both +1 oct and +3 oct square)
            const float s_uni = p;
            const float p2 = wrapPhase(p * 2.0f);
            const float p8 = wrapPhase(p * 8.0f);
            const float u = (p2 < 0.5f && p8 < 0.5f) ? s_uni : 0.0f;
            constexpr float mean_dc = 11.0f / 128.0f;
            constexpr float scale = 128.0f / 85.0f;
            float v = (u - mean_dc) * scale;
            v -= blep(p, dt) * (0.5f * scale);
            v -= blep(p2, dt * 2.0f) * (0.3f * scale);
            v -= blep(p8, std::min(dt * 8.0f, 0.48f)) * (0.3f * scale);
            return v;
        }

        // --- Group 3: Sub-Oscillator Waveforms ---
        case 8: // Sub 1: Square /2 (-1 oct 50% square, Cody's Vitaltable pos 0)
            return pulse(p, dt, 0.5f);

        case 9: // Sub 2: Pulse /2 (-1 oct 75% pulse, Cody's Vitaltable pos 63)
            return pulse(p, dt, 0.75f);

        case 10: { // Sub 3: Sub Chopped 2x (Cody's Vitaltable pos 128)
            const float p4 = wrapPhase(p * 4.0f);
            const float u = (p < 0.5f) ? 1.0f : ((p4 >= 0.5f) ? 1.0f : 0.0f);
            constexpr float mean_dc = 0.75f;
            constexpr float scale = 1.0f / 0.75f;
            float v = (u - mean_dc) * scale;
            v += blep(p, dt);
            v -= blep(wrapPhase(p - 0.5f + 1.0f), dt);
            v += blep(p4, std::min(dt * 4.0f, 0.48f)) * 0.5f;
            return v;
        }

        case 11: { // Sub 4: Sub Chopped 4x (Cody's Vitaltable pos 256)
            const float p8 = wrapPhase(p * 8.0f);
            const float u = (p < 0.5f) ? 1.0f : ((p8 >= 0.5f) ? 1.0f : 0.0f);
            constexpr float mean_dc = 0.75f;
            constexpr float scale = 1.0f / 0.75f;
            float v = (u - mean_dc) * scale;
            v += blep(p, dt);
            v -= blep(wrapPhase(p - 0.5f + 1.0f), dt);
            v += blep(p8, std::min(dt * 8.0f, 0.48f)) * 0.5f;
            return v;
        }

        case 12: // Sub 5: Square /4 (-2 oct 50% square)
            return pulse(p, dt, 0.5f);

        case 13:
        default: // Sub 6: Pulse /4 (-2 oct 75% pulse)
            return pulse(p, dt, 0.75f);
    }
}

/**
 * Closed-form DC-normalized multi-octave chopped Hoover sawtooth.
 * M = 1 (1x fundamental), 2 (2x twin-tooth), 4 (4x quad-tooth), 8 (8x octo-tooth).
 * pw in [0.05, 0.95].
 */
[[nodiscard]] inline float choppedSawM(float p, float dt, int M, float pw) noexcept {
    const int effM = std::max(M, 1);
    const float fM = static_cast<float>(effM);
    const float effPw = std::clamp(pw, 0.05f, 0.95f);
    const float pM = wrapPhase(p * fM);
    const float u = (pM < effPw) ? p : 0.0f;
    const float mean = (effPw / (2.0f * fM)) * (fM - 1.0f + effPw);
    const float uMax = (fM - 1.0f + effPw) / fM;
    const float peakDev = std::max(0.05f, uMax - mean);
    const float scale = 1.0f / peakDev;
    const float h = 0.5f * scale; // PolyBLEP spans peak-to-peak of 2.0

    float v = (u - mean) * scale;

    // No wrap correction at p=0 when pw < 1.0: u(1) == 0 and u(0) == 0 (continuous across wrap).
    // All teeth drops and rises are handled exactly by the loop below:
    for (int k = 0; k < effM; ++k) {
        // Drop at (k + pw) / M
        const float tDrop = (static_cast<float>(k) + effPw) / fM;
        v -= blep(wrapPhase(p - tDrop + 1.0f), dt) * (tDrop * h);

        // Rise at (k + 1) / M if k < M - 1
        if (k < effM - 1) {
            const float tRise = static_cast<float>(k + 1) / fM;
            v += blep(wrapPhase(p - tRise + 1.0f), dt) * (tRise * h);
        }
    }
    return std::clamp(v, -1.8f, 1.8f);
}

/**
 * Evaluate the 16 authentic Roland & 1985 SOTA DCO 2.0 Waveforms [0..15]:
 * - Bank 0 (Hoover Saws): 0: 1x PWM Saw, 1: 2x Twin Hoover, 2: 4x Quad Hoover, 3: 8x Octo Mega Hoover
 * - Bank 1 (Pulses):      4: 50% Square, 5: 25% Narrow Pulse, 6: 12.5% Needle Pulse, 7: Var PWM Pulse
 * - Bank 2 (Triangles):   8: Linear Triangle, 9: Skewed Sharktooth, 10: Diamond M, 11: Crown W
 * - Bank 3 (Exotic/PD):   12: System-100M Diff Sine, 13: Casio CZ-101 PD Res Sine, 14: Detroit /3 Stair, 15: CMOS Ring Sub
 */
[[nodiscard]] inline float alphaWave16(int shapeIdx, float phase, float dt, float pwm) noexcept {
    const int idx = std::clamp(shapeIdx, 0, 15);
    const float p = wrapPhase(phase);
    const float pw = std::clamp(0.5f + 0.45f * pwm, 0.05f, 0.95f);

    switch (idx) {
        // --- Bank 0: Multi-Octave Dynamic PWM Hoover Saws ---
        case 0: return choppedSawM(p, dt, 1, pw);
        case 1: return choppedSawM(p, dt, 2, pw);
        case 2: return choppedSawM(p, dt, 4, pw);
        case 3: return choppedSawM(p, dt, 8, pw);

        // --- Bank 1: Symmetrical & Asymmetrical Pulses ---
        case 4: return pulse(p, dt, 0.5f);
        case 5: return pulse(p, dt, 0.25f);
        case 6: return pulse(p, dt, 0.125f);
        case 7: return pulse(p, dt, pw);

        // --- Bank 2: Op-Amp Triangle & Morphologies ---
        case 8: return triangle(p);
        case 9: { // Skewed Sharktooth Triangle with Normalized Makeup Gain
            const float S = pw;
            const float u = (p < S) ? (p / S) : ((1.0f - p) / (1.0f - S));
            const float normGain = 1.0f / std::sqrt(std::max(0.01f, 4.0f * S * (1.0f - S)));
            const float v = (2.0f * u - 1.0f) * std::min(2.0f, normGain);
            return std::clamp(v, -1.8f, 1.8f);
        }
        case 10: { // Diamond M-Triangle (Dual-Peak Folded Triangle)
            const float tri2 = triangle(wrapPhase(p * 2.0f));
            return 2.0f * std::abs(tri2) - 1.0f;
        }
        case 11: { // Crown W-Triangle (Triple-Peak Folded Triangle)
            const float tri3 = triangle(wrapPhase(p * 3.0f));
            return 2.0f * std::abs(tri3) - 1.0f;
        }

        // --- Bank 3: 1985 Exotic / Complex / Phase Distortion ---
        case 12: { // Roland System-100M Diff-Pair Saturated Sine
            const float rawSin = std::sin(6.28318530717958647692f * p);
            return std::tanh(3.5f * rawSin) * 1.05f;
        }
        case 13: { // Casio CZ-101 Resonant Phase Distortion Sine
            const float nRes = 2.0f + 6.0f * pw;
            const float carrier = std::sin(6.28318530717958647692f * p * nRes);
            const float window = 0.5f * (1.0f + std::cos(3.14159265358979323846f * p));
            return carrier * window * 1.4f;
        }
        case 14: { // Detroit /3 Subharmonic Stair Saw
            const float saw1 = saw(p, dt);
            const float saw3 = saw(wrapPhase(p * 3.0f), std::min(dt * 3.0f, 0.48f));
            const float stair3 = (saw1 - saw3 * (1.0f / 3.0f)) * 1.5f;
            return 0.5f * saw1 + 0.5f * stair3;
        }
        case 15:
        default: { // CMOS Ring-Mod Bit Sub (PolyBLEP XOR Product)
            const float s0 = pulse(p, dt, pw);
            const float s1 = pulse(wrapPhase(p * 0.5f), dt * 0.5f, 0.5f);
            return s0 * s1;
        }
    }
}

/**
 * SuperSaw-85 3-Voice Staircase Generator (PolyBLEP Identity):
 * Uses 8253 timer channels driving CD4024 ripple counters + 5-bit R-2R ladder networks.
 * Closed-form band-limited: staircase_8(phi) = saw(phi) - 1/8 * saw(8*phi).
 */
[[nodiscard]] inline float superSawStaircase(
    float phase,
    float dt,
    float detuneA,
    float detuneB) noexcept
{
    const float p = wrapPhase(phase);
    const float dtA = dt * (1.0f + detuneA);
    const float dtB = dt * (1.0f - detuneB);
    const float pA = wrapPhase(phase * (1.0f + detuneA));
    const float pB = wrapPhase(phase * (1.0f - detuneB));

    // Center Voice: Pure Band-Limited Saw
    const float sMain = saw(p, dt);

    // Voice A: Band-Limited 8-Step R-2R Staircase (via PolyBLEP Identity)
    const float sA = saw(pA, dtA) - 0.125f * saw(wrapPhase(pA * 8.0f), std::min(dtA * 8.0f, 0.48f));

    // Voice B: Band-Limited 8-Step R-2R Staircase
    const float sB = saw(pB, dtB) - 0.125f * saw(wrapPhase(pB * 8.0f), std::min(dtB * 8.0f, 0.48f));

    // 3-Voice Sum normalized
    return (sMain + sA + sB) * 0.36f;
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
