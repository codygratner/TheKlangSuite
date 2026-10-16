#pragma once

#include "../../common/AudioEffect.h"

namespace TbdAudio::KlangStomp {

/**
 * BossHM2 ("Swedish Chainsaw")
 *
 * Authentic component-level DSP topology of the Japanese Boss HM-2 Heavy Metal circuit:
 * 1. 720 Hz High-Pass pre-emphasis (OnePoleTPT) stripping low-end mud before clipping.
 * 2. 4x Oversampled 3-Stage Non-Linear Cascade:
 *    - Stage 1: Soft feedback op-amp saturation (FastMath::fastTanh).
 *    - Stage 2: Hard shunt silicon diode clipper scaled by voltage rail headroom (starveNorm).
 *    - Stage 3: Unbiased Complementary BJT Crossover Dead-Band Stage (|v| < Vth smooth dead-zone)
 *      generating grinding high-order odd harmonics across zero crossings even on decaying tails.
 * 3. Dual Gyrator Active Tone Stack (CytomicSVF Bell stages):
 *    - Low Gyrator at 98 Hz (Q = 1.8), gain driven by toneNorm (-12 dB to +18 dB).
 *    - High Chainsaw Gyrator at 1050 Hz (Q = 2.8), gain driven by midNorm (-6 dB to +20 dB).
 * 4. Post-EQ 4.5 kHz de-emphasis lowpass and 5 Hz DC blocker.
 */
class BossHM2 {
public:
    void init(float sampleRate) noexcept {
        sr = std::max(sampleRate, 8000.0f);
        invSr = 1.0f / sr;

        preHPF.setCutoff(720.0f, invSr);
        postLPF.setCutoff(4500.0f, invSr);
        dcBlocker.init(sr);

        cachedTone = -1.0f;
        cachedMid = -1.0f;
        updateGyrators(0.5f, 0.5f);
        reset();
    }

    void reset() noexcept {
        preHPF.reset();
        oversampler.reset();
        gyratorLow.reset();
        gyratorHigh.reset();
        postLPF.reset();
        dcBlocker.reset();
    }

    [[nodiscard]] float processSample(float x, float driveNorm, float toneNorm, float midNorm, float starveNorm) noexcept {
        if (!std::isfinite(x)) {
            return 0.0f;
        }

        const float dNorm = std::clamp(driveNorm, 0.0f, 1.0f);
        const float tNorm = std::clamp(toneNorm, 0.0f, 1.0f);
        const float mNorm = std::clamp(midNorm, 0.0f, 1.0f);
        const float sNorm = std::clamp(starveNorm, 0.0f, 1.0f);

        // Update active gyrator EQ coefficients when parameters move
        if (std::abs(tNorm - cachedTone) > 1e-4f || std::abs(mNorm - cachedMid) > 1e-4f) {
            updateGyrators(tNorm, mNorm);
        }

        // 1. 720 Hz High-Pass Pre-Emphasis (blend dry floor + steep mid-high pre-emphasis)
        const float hp = preHPF.processHP(x);
        const float preEmph = 0.20f * x + 0.80f * hp;

        // Drive gain mapping: +6 dB to +42 dB into the non-linear cascade
        const float driveGain = Taper::profileE_DriveGain(dNorm, 6.0f, 42.0f);

        // Rail voltage headroom scaling: 0.0 -> 3V sag, 0.5 -> 9V nominal, 1.0 -> 18V expanded
        const float railScale = 0.35f + sNorm * (0.65f + 0.70f * sNorm); // ~0.35 at 0, ~0.85 at 0.5, ~1.70 at 1.0
        const float asymBias = (1.0f - sNorm) * 0.14f;                   // Asymmetric starve shift at low voltage
        const float posClip = 0.68f * railScale;
        const float negClip = (0.68f * railScale) * (1.0f - 0.25f * (1.0f - sNorm));
        const float invPosClip = 1.0f / std::max(posClip, 0.05f);
        const float invNegClip = 1.0f / std::max(negClip, 0.05f);

        // Unbiased BJT crossover dead-band threshold (wider under starved bias, tighter at 18V)
        const float vth = 0.055f + 0.045f * (1.0f - 0.5f * sNorm);

        // 2. 4x Oversampled 3-Stage Non-Linear Clipping Cascade
        float clipped = oversampler.processSample(preEmph, [&](float s) noexcept -> float {
            // Stage 1: Soft feedback op-amp saturation
            float v1 = FastMath::fastTanh(s * driveGain * 0.35f + asymBias) - (asymBias * 0.5f);

            // Stage 2: Hard shunt diode clip scaled by voltage rail headroom
            float v2Drive = v1 * (1.8f + 2.2f * dNorm);
            float v2 = (v2Drive >= 0.0f)
                ? posClip * FastMath::fastTanhPrecise(v2Drive * invPosClip)
                : negClip * FastMath::fastTanhPrecise(v2Drive * invNegClip);

            // Stage 3: Unbiased Complementary BJT Crossover Dead-Band Stage
            // Smooth cubic dead-zone around |v| < Vth: u * (u^2 / (u^2 + Vth^2))
            float u = v2 * 1.35f;
            float u2 = u * u;
            float deadBand = u * (u2 / (u2 + vth * vth + 1e-6f));
            return FastMath::fastTanh(deadBand * 1.4f);
        });

        // Makeup level compensation for rail scaling
        const float makeup = 0.72f / std::sqrt(std::max(railScale, 0.25f));
        float sig = clipped * makeup;

        // 3. Dual Gyrator Active EQ
        sig = gyratorLow.process(sig);
        sig = gyratorHigh.process(sig);
        gyratorLow.clampState();
        gyratorHigh.clampState();

        // 4. Post-EQ 4.5 kHz de-emphasis lowpass & DC blocker
        sig = postLPF.processLP(sig);
        sig = dcBlocker.process(sig);

        if (!std::isfinite(sig)) {
            reset();
            return 0.0f;
        }
        return std::clamp(sig, -8.0f, 8.0f);
    }

private:
    void updateGyrators(float toneNorm, float midNorm) noexcept {
        cachedTone = toneNorm;
        cachedMid = midNorm;

        // Low Gyrator at 98 Hz, Q = 1.8, gain driven by toneNorm (-12 dB to +18 dB)
        const float lowGainDb = -12.0f + 30.0f * toneNorm;
        gyratorLow.setBell(98.0f, 1.8f, lowGainDb, invSr);

        // High Chainsaw Gyrator at 1050 Hz, Q = 2.8, gain driven by midNorm (-6 dB to +20 dB)
        const float highGainDb = -6.0f + 26.0f * midNorm;
        gyratorHigh.setBell(1050.0f, 2.8f, highGainDb, invSr);
    }

    float sr = 44100.0f;
    float invSr = 1.0f / 44100.0f;
    float cachedTone = -1.0f;
    float cachedMid = -1.0f;

    DSPCommon::OnePoleTPT preHPF;
    DSPCommon::Oversampler4x oversampler;
    DSPCommon::CytomicSVF gyratorLow;
    DSPCommon::CytomicSVF gyratorHigh;
    DSPCommon::OnePoleTPT postLPF;
    DSPCommon::DCBlocker dcBlocker;
};

using SwedishChainsaw = BossHM2;

} // namespace TbdAudio::KlangStomp
