#pragma once

#include "HasturFilterUtils.h"
#include "AllPassFilter4.h"
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace CrackedChimes::DSP {


/**
 * @brief Dual Cascaded SVF Extended VCF Engine:
 * - Dual-SVF Analog Silicon Emulation (Two 2-pole ZDF SVFs per chip = 4x 2-pole stages total).
 * - Zero passband bass loss: authentic dual-SVF 0dB DC gain response.
 * - Mid-Deck 8th-Order Laser Sandwich (8-Pole APF -> Normalized Tanh -> 8-Pole Inv-APF).
 * - Resonant Comb Recirculation Loop across the sandwich.
 * - 3 Deck Routing Modes: Pre-24dB (Bite -> Chip A), Dual-48dB Monolith (Chip A -> Bite -> Chip B), Post-24dB (Chip A -> Bite).
 * - Authentic 4-Position Stepped Analog HPF Deck (0: Flat, 1: 80Hz Bass Bump, 2: 150Hz Cut, 3: 300Hz Cut).
 */
class DualSvfFilter {
public:
    // Backward compatibility alias
    using AlphaJunoFilter = DualSvfFilter;

public:
    enum DeckMode : uint8_t {
        DECK_PRE_24  = 0, // Sandwich -> Chip A (24dB)
        DECK_DUAL_48 = 1, // Chip A -> Sandwich -> Chip B (48dB Monolith)
        DECK_POST_24 = 2  // Chip A (24dB) -> Sandwich
    };

    enum PoleMode : uint8_t {
        POLE_1_3 = 0,
        POLE_2_2 = 1,
        POLE_3_1 = 2,
        POLE_4_LOOP = 1
    };

    enum HpfMode : uint8_t {
        HPF_OFF       = 0, // Flat DC bypass
        HPF_BASS_BUMP = 1, // 80Hz resonant bass boost (+3dB peak at 85Hz)
        HPF_150HZ     = 2, // 150Hz Butterworth highpass
        HPF_300HZ     = 3  // 300Hz Butterworth highpass
    };

    void prepare(double sampleRate) noexcept {
        sampleRate_ = (sampleRate > 1000.0) ? sampleRate : 44100.0;
        invSr_ = 1.0 / sampleRate_;
        midApf_.prepare(sampleRate_);
        biteHfFilter_.setCutoff(14500.0f, static_cast<float>(sampleRate_));
        reset();
    }

    void reset() noexcept {
        s1_1_ = 0.0f; s2_1_ = 0.0f;
        s1_2_ = 0.0f; s2_2_ = 0.0f;
        s1_3_ = 0.0f; s2_3_ = 0.0f;
        s1_4_ = 0.0f; s2_4_ = 0.0f;
        invApfState_ = 0.0f;
        prevSandwichCore_ = 0.0f;
        hpS1_ = 0.0f; hpS2_ = 0.0f;
        midApf_.reset();
        biteDcFilter_.reset();
        biteHfFilter_.reset();
    }

    /**
     * @brief Process audio sample through the Dual-SVF VCF Engine.
     * 
     * @param input Raw audio sample from the mixer.
     * @param cutoffHz Master cutoff frequency (10 Hz to 22000 Hz).
     * @param resonance Amount of resonance feedback (0.0 to 1.0).
     * @param deckMode Deck routing (0: Pre-24dB, 1: Dual-48dB, 2: Post-24dB).
     * @param laserBite 8th-order APF phase dispersion & recirculation depth [0.0, 1.0].
     * @param laserDrive Normalized Tanh saturation drive [0.0, 1.0].
     * @param hpfMode 4-position analog HPF (0: Off, 1: 80Hz Bass Bump, 2: 150Hz, 3: 300Hz).
     * @return Filtered output sample.
     */
    [[nodiscard]] float process(
        float input,
        float cutoffHz,
        float resonance,
        float hpCutoffHz,
        int deckMode = 1,
        float midApfCutoff = 1000.0f,
        bool midApfEnabled = false,
        float midDrive = 0.0f) noexcept
    {
        (void)midApfCutoff;
        const int hpfMode = (hpCutoffHz > 200.0f) ? 3 : ((hpCutoffHz > 50.0f) ? 2 : 1);
        const float laserBite = midApfEnabled ? 0.5f : 0.0f;
        return process(input, cutoffHz, resonance, deckMode, laserBite, midDrive, hpfMode);
    }

    // 7-parameter compatibility overload (omitting filterWarp)
    [[nodiscard]] float process(
        float input,
        float cutoffHz,
        float resonance,
        int deckMode,
        float laserBite,
        float laserDrive,
        int hpfMode) noexcept
    {
        return process(input, cutoffHz, resonance, deckMode, 0.0f, laserBite, laserDrive, hpfMode);
    }

    [[nodiscard]] float process(
        float input,
        float cutoffHz,
        float resonance,
        int deckMode = 1,
        float filterWarp = 0.0f,
        float laserBite = 0.0f,
        float laserDrive = 0.0f,
        int hpfMode = 1) noexcept
    {
        const float nyquist = static_cast<float>(sampleRate_ * 0.48);
        const float fc = std::clamp(cutoffHz, 10.0f, nyquist);

        // Pre-warped frequency coefficient g = tan(pi * fc / fs) with Padé fast approximation
        const float wd = static_cast<float>(3.14159265358979323846 * fc * invSr_);
        auto fastTanPade = [](float x) noexcept -> float {
            const float x2 = x * x;
            return x * (15.0f - x2) / (15.0f - 6.0f * x2);
        };
        const float g = (wd < 0.75f) ? fastTanPade(wd) : std::tan(std::min(wd, 1.52f));

        // Authentic Dual-SVF Local Damping:
        // In 24dB mode: kDamp in [0.05, 2.0] (Q up to 20.0, liquid resonance).
        // In Dual-48dB mode: scale damping so 4-stage composite Q^4 <= 36 (+32dB max) to prevent unusable screech cliff.
        const float resNorm = std::clamp(resonance, 0.0f, 1.0f);
        const float resCurve = std::pow(resNorm, 1.5f);
        const float kDampScale = (deckMode == DECK_DUAL_48) ? 1.58f : 1.78f;
        const float kDamp = 2.0f - kDampScale * resCurve;

        // 2-pole ZDF SVF coefficients
        const float a1 = 1.0f / (1.0f + g * (g + kDamp));
        const float a2 = g * a1;
        const float a3 = g * a2;

        float out = 0.0f;
        switch (deckMode) {
            case 0: { // Pre-24dB: Laser Sandwich -> Chip A
                float s = processSandwich(input, fc, filterWarp, laserBite, laserDrive);
                out = processChipA(s, a1, a2, a3);
                break;
            }
            case 2: { // Post-24dB: Chip A -> Laser Sandwich
                float a = processChipA(input, a1, a2, a3);
                out = processSandwich(a, fc, filterWarp, laserBite, laserDrive);
                break;
            }
            case 1:
            default: { // Dual-48dB Monolith: Chip A -> Laser Sandwich -> Chip B
                float a = processChipA(input, a1, a2, a3);
                float s = processSandwich(a, fc, filterWarp, laserBite, laserDrive);
                out = processChipB(s, a1, a2, a3);
                break;
            }
        }

        // Apply 4-position Analog High-Pass Filter
        out = processHPF(out, hpfMode);

        // Denormal protection
        if (std::abs(s1_1_) < 1e-9f) s1_1_ = 0.0f;
        if (std::abs(s2_1_) < 1e-9f) s2_1_ = 0.0f;
        if (std::abs(s1_2_) < 1e-9f) s1_2_ = 0.0f;
        if (std::abs(s2_2_) < 1e-9f) s2_2_ = 0.0f;
        if (std::abs(s1_3_) < 1e-9f) s1_3_ = 0.0f;
        if (std::abs(s2_3_) < 1e-9f) s2_3_ = 0.0f;
        if (std::abs(s1_4_) < 1e-9f) s1_4_ = 0.0f;
        if (std::abs(s2_4_) < 1e-9f) s2_4_ = 0.0f;
        if (std::abs(invApfState_) < 1e-9f) invApfState_ = 0.0f;
        if (std::abs(hpS1_) < 1e-9f) hpS1_ = 0.0f;
        if (std::abs(hpS2_) < 1e-9f) hpS2_ = 0.0f;

        return out;
    }

private:
    /**
     * @brief Chip A: Dual-SVF 24dB/oct lowpass (dual 2-pole SVF cascade).
     */
    [[nodiscard]] inline float processChipA(float x, float a1, float a2, float a3) noexcept {
        // Stage 1: 2-pole SVF
        const float v1 = a1 * s1_1_ + a2 * (x - s2_1_);
        const float lp1 = s2_1_ + a2 * s1_1_ + a3 * (x - s2_1_);
        s1_1_ = 2.0f * v1 - s1_1_;
        s2_1_ = 2.0f * lp1 - s2_1_;

        // Soft transconductance saturation on Stage 1
        const float stage1Out = std::tanh(lp1 * 1.1f);

        // Stage 2: 2-pole SVF
        const float v2 = a1 * s1_2_ + a2 * (stage1Out - s2_2_);
        const float lp2 = s2_2_ + a2 * s1_2_ + a3 * (stage1Out - s2_2_);
        s1_2_ = 2.0f * v2 - s1_2_;
        s2_2_ = 2.0f * lp2 - s2_2_;

        return std::tanh(lp2 * 1.0f);
    }

    /**
     * @brief Chip B: Second Dual-SVF 24dB/oct lowpass (dual 2-pole SVF cascade).
     */
    [[nodiscard]] inline float processChipB(float x, float a1, float a2, float a3) noexcept {
        // Stage 3: 2-pole SVF
        const float v3 = a1 * s1_3_ + a2 * (x - s2_3_);
        const float lp3 = s2_3_ + a2 * s1_3_ + a3 * (x - s2_3_);
        s1_3_ = 2.0f * v3 - s1_3_;
        s2_3_ = 2.0f * lp3 - s2_3_;

        // Soft transconductance saturation on Stage 3
        const float stage3Out = std::tanh(lp3 * 1.1f);

        // Stage 4: 2-pole SVF
        const float v4 = a1 * s1_4_ + a2 * (stage3Out - s2_4_);
        const float lp4 = s2_4_ + a2 * s1_4_ + a3 * (stage3Out - s2_4_);
        s1_4_ = 2.0f * v4 - s1_4_;
        s2_4_ = 2.0f * lp4 - s2_4_;

        return std::tanh(lp4 * 1.0f);
    }

    /**
     * @brief Mid-Deck 8th-Order Laser Sandwich with Bipolar Recirculation Daughterboard.
     * Inherits the exact same daughterboard design as the DCO:
     * - APF base frequency tracks unmodulated cutoff knob * 2^warpOct.
     * - Bipolar BITE: Clockwise = Upper Bite (In-phase), Counter-Clockwise = Lower Bite (Anti-phase).
     * - 14.5kHz 1-pole HF damping + DC block in feedback loop prevents ultrasonic limit-cycle squeals.
     */
    [[nodiscard]] inline float processSandwich(float sig, float baseCutoffHz, float warpOct, float bite, float drive) noexcept {
        if (std::abs(warpOct) <= 0.001f && std::abs(bite) <= 0.001f && drive <= 0.001f) {
            return sig; // Transparent identity bypass
        }

        const float nyquist = static_cast<float>(sampleRate_ * 0.48);
        const float warpMult = std::exp2(std::clamp(warpOct, -2.0f, 2.0f));
        const float apfFc = std::clamp(baseCutoffHz * warpMult, 20.0f, nyquist);

        // Bipolar BITE loop with 14.5kHz HF damping and leaky DC blocker
        float inWithFb = sig;
        if (std::abs(bite) > 0.001f) {
            const float effBite = bite * 0.65f;
            const float satFb = std::tanh(prevSandwichCore_ * (1.0f + std::abs(effBite))) * 0.75f;
            inWithFb = sig - (effBite * 0.85f) * satFb;
        }

        // 1. Forward 8-pole APF (Phase Dispersion)
        const float apfOut = midApf_.processAPF(inWithFb, apfFc, 0.0f);

        // Feedback state with HF roll-off and DC rejection
        prevSandwichCore_ = biteHfFilter_.process(biteDcFilter_.process(apfOut));

        // 2. Normalized Punchy Tanh Overdrive in the middle
        const float driveGain = std::clamp(drive, 0.0f, 1.0f);
        const float G = 1.0f + driveGain * 4.0f;
        float tanhOut = std::tanh(apfOut * G);
        if (G > 1.01f) {
            tanhOut /= std::tanh(G); // Zero volume sag
        }

        // 3. Inverse 8-pole APF (Phase Unwinding)
        const float invOut = midApf_.processInvertedAPF(tanhOut, apfFc);
        invApfState_ = invOut;

        return invOut;
    }

    /**
     * @brief 4-Position Authentic Analog High-Pass Filter Deck.
     */
    [[nodiscard]] inline float processHPF(float sig, int mode) noexcept {
        switch (mode) {
            case 1: { // 80Hz Resonant Bass Bump (Q = 1.35)
                const float hpWd = static_cast<float>(2.0 * 3.14159265358979323846 * 78.0 * invSr_);
                const float hpG = std::tan(hpWd);
                const float hpK = 1.0f / 1.35f;
                const float hpA1 = 1.0f / (1.0f + hpG * (hpG + hpK));
                const float v = hpA1 * (sig - hpS1_ * hpK - hpS2_);
                const float bp = v * hpG + hpS1_;
                const float lp = bp * hpG + hpS2_;
                const float hp = sig - hpK * bp - lp;
                hpS1_ = 2.0f * bp - hpS1_;
                hpS2_ = 2.0f * lp - hpS2_;
                return hp;
            }
            case 2: { // 150Hz Cut (Butterworth Q = 0.707)
                const float hpWd = static_cast<float>(2.0 * 3.14159265358979323846 * 150.0 * invSr_);
                const float hpG = std::tan(hpWd);
                const float hpK = 1.41421356f;
                const float hpA1 = 1.0f / (1.0f + hpG * (hpG + hpK));
                const float v = hpA1 * (sig - hpS1_ * hpK - hpS2_);
                const float bp = v * hpG + hpS1_;
                const float lp = bp * hpG + hpS2_;
                const float hp = sig - hpK * bp - lp;
                hpS1_ = 2.0f * bp - hpS1_;
                hpS2_ = 2.0f * lp - hpS2_;
                return hp;
            }
            case 3: { // 300Hz Cut (Butterworth Q = 0.707)
                const float hpWd = static_cast<float>(2.0 * 3.14159265358979323846 * 300.0 * invSr_);
                const float hpG = std::tan(hpWd);
                const float hpK = 1.41421356f;
                const float hpA1 = 1.0f / (1.0f + hpG * (hpG + hpK));
                const float v = hpA1 * (sig - hpS1_ * hpK - hpS2_);
                const float bp = v * hpG + hpS1_;
                const float lp = bp * hpG + hpS2_;
                const float hp = sig - hpK * bp - lp;
                hpS1_ = 2.0f * bp - hpS1_;
                hpS2_ = 2.0f * lp - hpS2_;
                return hp;
            }
            case 0:
            default:
                return sig; // Flat DC bypass
        }
    }

    double sampleRate_ = 44100.0;
    double invSr_ = 1.0 / 44100.0;

    // Chip A: Stage 1 & 2 SVF state variables
    float s1_1_ = 0.0f;
    float s2_1_ = 0.0f;
    float s1_2_ = 0.0f;
    float s2_2_ = 0.0f;

    // Chip B: Stage 3 & 4 SVF state variables
    float s1_3_ = 0.0f;
    float s2_3_ = 0.0f;
    float s1_4_ = 0.0f;
    float s2_4_ = 0.0f;

    // Mid-Deck 8th-order APF
    AllPassFilter4 midApf_;
    float invApfState_ = 0.0f;
    float prevSandwichCore_ = 0.0f;
    LeakyDCBlocker biteDcFilter_;
    OnePoleLP biteHfFilter_;

    // Stepped HPF state variables
    float hpS1_ = 0.0f;
    float hpS2_ = 0.0f;
};

using AlphaJunoFilter = DualSvfFilter;
} // namespace CrackedChimes::DSP
