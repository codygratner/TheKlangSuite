#pragma once

#include "dsp/oscillators/PolyBLEP.h"
#include <cmath>
#include <algorithm>

namespace CrackedChimes::DSP {

/**
 * @brief Dual Logic DCOs with Dual-Vortex Cross-Modulation & Metal Sync.
 * SSoT: Divided Logic DCO, Dual-Directional Soft Sync, and Pitch Strangle.
 */
class HasturDCO {
public:
    using AlphaJunoDCO = HasturDCO;

public:
    enum class SyncMode : uint8_t {
        OFF = 0,
        FORWARD_1_TO_2 = 1,
        REVERSE_2_TO_1 = 2,
        METAL_MUTUAL = 3
    };

    enum class ModType : uint8_t {
        OFF = 0,
        LINEAR_FM = 1,
        EXPONENTIAL_FM = 2,
        AM = 3,
        RM = 4
    };

    void prepare(double sampleRate) noexcept {
        sampleRate_ = (sampleRate > 1000.0) ? sampleRate : 44100.0;
        invSr_ = 1.0 / sampleRate_;
        reset();
    }

    void reset() noexcept {
        phase1_ = 0.0f;
        phase2_ = 0.0f;
        prevOut1_ = 0.0f;
        prevOut2_ = 0.0f;
        lastF1_ = 130.81f;
        lastF2_ = 130.81f;
    }

    /**
     * @brief Render both oscillators with cross-modulation and sync.
     * @param baseFreqHz Fundamental pitch (after glide/bend).
     * @param osc2PitchOffsetOct -1.0 to +5.0 octaves (in 1-cent steps).
     * @param sawMix1 Osc 1 Saw volume (0..1).
     * @param sawType1 Osc 1 Saw shape (0..4).
     * @param pulseMix1 Osc 1 Pulse volume (0..1).
     * @param pulseType1 Osc 1 Pulse shape (0..8).
     * @param sawMix2 Osc 2 Saw volume (0..1).
     * @param sawType2 Osc 2 Saw shape (0..4).
     * @param pulseMix2 Osc 2 Pulse volume (0..1).
     * @param pulseType2 Osc 2 Pulse shape (0..8).
     * @param syncMode Sync topology (Off, 1->2, 2->1, Metal).
     * @param syncAmount Hardness depth (0.0 to 1.0).
     * @param modTypeA Mod Type A (Off, LinFM, ExpFM, AM, RM).
     * @param modTypeB Mod Type B.
     * @param modDirection 0: Sunwise (1->A->2 & 2->B->1); 1: Widdershins (1->B->2 & 2->A->1).
     * @param modAmount Cross-modulation intensity (0.0 to 1.0).
     * @param effectivePwm Global pulse width modulation [-1.0, 1.0].
     * @param outOsc1 Audio output of Oscillator 1.
     * @param outOsc2 Audio output of Oscillator 2.
     */
    void process(
        float baseFreqHz,
        float osc2PitchOffsetOct,
        float sawMix1, int sawType1,
        float pulseMix1, int pulseType1,
        float sawMix2, int sawType2,
        float pulseMix2, int pulseType2,
        SyncMode syncMode, float syncAmount,
        ModType modTypeA, ModType modTypeB,
        int modDirection,
        float modAmount,
        float effectivePwm,
        float& outOsc1, float& outOsc2) noexcept
    {
        const float nyquist = static_cast<float>(sampleRate_ * 0.48);

        // 1. Determine base frequencies
        float f1 = std::clamp(baseFreqHz, 5.0f, nyquist);
        float f2 = std::clamp(baseFreqHz * std::pow(2.0f, osc2PitchOffsetOct), 5.0f, nyquist);

        const float modDepth = std::clamp(modAmount, 0.0f, 1.0f);

        // 2. Decode Cross-Modulation Types
        ModType mod1to2 = (modDirection == 0) ? modTypeA : modTypeB;
        ModType mod2to1 = (modDirection == 0) ? modTypeB : modTypeA;

        // Apply FM to Osc 1 from Osc 2 (using previous sample output)
        if (mod2to1 == ModType::LINEAR_FM) {
            f1 = std::clamp(f1 + prevOut2_ * (modDepth * 2400.0f), 5.0f, nyquist);
        } else if (mod2to1 == ModType::EXPONENTIAL_FM) {
            f1 = std::clamp(f1 * std::pow(2.0f, prevOut2_ * (modDepth * 3.5f)), 5.0f, nyquist);
        }

        // Apply FM to Osc 2 from Osc 1
        if (mod1to2 == ModType::LINEAR_FM) {
            f2 = std::clamp(f2 + prevOut1_ * (modDepth * 2400.0f), 5.0f, nyquist);
        } else if (mod1to2 == ModType::EXPONENTIAL_FM) {
            f2 = std::clamp(f2 * std::pow(2.0f, prevOut1_ * (modDepth * 3.5f)), 5.0f, nyquist);
        }

        lastF1_ = f1;
        lastF2_ = f2;

        const float dt1 = f1 * static_cast<float>(invSr_);
        const float dt2 = f2 * static_cast<float>(invSr_);

        // 3. Phase Accumulation & Continuous Hard Sync
        bool wrap1 = false;
        bool wrap2 = false;

        phase1_ += dt1;
        if (phase1_ >= 1.0f) {
            phase1_ -= 1.0f;
            wrap1 = true;
        }

        phase2_ += dt2;
        if (phase2_ >= 1.0f) {
            phase2_ -= 1.0f;
            wrap2 = true;
        }

        const float hardSync = std::clamp(syncAmount, 0.0f, 1.0f);
        if (syncMode != SyncMode::OFF && hardSync > 0.001f) {
            switch (syncMode) {
                case SyncMode::FORWARD_1_TO_2:
                    if (wrap1) {
                        phase2_ = phase2_ * (1.0f - hardSync);
                    }
                    break;

                case SyncMode::REVERSE_2_TO_1:
                    if (wrap2) {
                        phase1_ = phase1_ * (1.0f - hardSync);
                    }
                    break;

                case SyncMode::METAL_MUTUAL:
                    // Bi-directional mutual sync: wrap in either nudges the other
                    if (wrap1) {
                        phase2_ = phase2_ * (1.0f - hardSync);
                    }
                    if (wrap2) {
                        phase1_ = phase1_ * (1.0f - hardSync);
                    }
                    break;

                case SyncMode::OFF:
                default:
                    break;
            }
        }

        // 4. Generate Raw Waveforms using PolyBLEP alphaWave14
        const float pwm = std::clamp(effectivePwm, -0.92f, 0.92f);

        // Osc 1 Waves
        const int sShape1 = std::clamp(sawType1, 0, 4);
        const int pShape1 = std::clamp(pulseType1, 0, 8) + 5;
        const float sawVal1 = TbdAudio::Oscillators::PolyBLEP::alphaWave14(sShape1, phase1_, dt1, pwm);
        const float pulseVal1 = TbdAudio::Oscillators::PolyBLEP::alphaWave14(pShape1, phase1_, dt1, pwm);
        float raw1 = sawVal1 * sawMix1 + pulseVal1 * pulseMix1;

        // Osc 2 Waves
        const int sShape2 = std::clamp(sawType2, 0, 4);
        const int pShape2 = std::clamp(pulseType2, 0, 8) + 5;
        const float sawVal2 = TbdAudio::Oscillators::PolyBLEP::alphaWave14(sShape2, phase2_, dt2, pwm);
        const float pulseVal2 = TbdAudio::Oscillators::PolyBLEP::alphaWave14(pShape2, phase2_, dt2, pwm);
        float raw2 = sawVal2 * sawMix2 + pulseVal2 * pulseMix2;

        // 5. Apply AM / RM Cross-Modulation with modAmount depth
        // Mod 2 -> 1
        if (mod2to1 == ModType::AM) {
            raw1 *= std::clamp(1.0f + prevOut2_ * (modDepth * 1.5f), 0.0f, 2.5f);
        } else if (mod2to1 == ModType::RM) {
            raw1 = (1.0f - modDepth) * raw1 + modDepth * (raw1 * prevOut2_);
        }

        // Mod 1 -> 2
        if (mod1to2 == ModType::AM) {
            raw2 *= std::clamp(1.0f + prevOut1_ * (modDepth * 1.5f), 0.0f, 2.5f);
        } else if (mod1to2 == ModType::RM) {
            raw2 = (1.0f - modDepth) * raw2 + modDepth * (raw2 * prevOut1_);
        }

        // Store outputs for next-sample cross-mod
        prevOut1_ = raw1;
        prevOut2_ = raw2;

        outOsc1 = raw1;
        outOsc2 = raw2;
    }

    [[nodiscard]] float getPhase1() const noexcept { return phase1_; }
    [[nodiscard]] float getPhase2() const noexcept { return phase2_; }
    [[nodiscard]] bool getSquare1() const noexcept { return phase1_ < 0.5f; }
    [[nodiscard]] bool getSquare2() const noexcept { return phase2_ < 0.5f; }
    [[nodiscard]] float getFreq1() const noexcept { return lastF1_; }
    [[nodiscard]] float getFreq2() const noexcept { return lastF2_; }

private:
    double sampleRate_ = 44100.0;
    double invSr_ = 1.0 / 44100.0;

    float phase1_ = 0.0f;
    float phase2_ = 0.0f;
    float prevOut1_ = 0.0f;
    float prevOut2_ = 0.0f;
    float lastF1_ = 130.81f;
    float lastF2_ = 130.81f;
};

using AlphaJunoDCO = HasturDCO;
} // namespace CrackedChimes::DSP
