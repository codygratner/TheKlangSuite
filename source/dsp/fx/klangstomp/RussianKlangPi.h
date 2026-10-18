#pragma once

#include <algorithm>
#include <cmath>
#include "../../common/AudioEffect.h"

namespace TbdAudio::KlangStomp {

/**
 * BigMuffPi ("RussianMuff") — 4-Stage Cascaded BJT Feedback-Clipped Fuzz & Scooped Tone Stack
 *
 * Circuit Topology:
 * 1. Stage 1 — Input BJT Booster (+24 dB):
 *    - 100 Hz input coupling High-Pass Filter + common-emitter collector feedback saturation
 * 2. Stage 2 & Stage 3 — Cascaded Dual BJT Feedback Diode Clipping Stages (4x Oversampled):
 *    - Each stage features a 1nF local feedback capacitor (6.5 kHz OnePoleTPT low-pass)
 *      paired with shunt feedback clipping diodes, producing thick, singing sustain
 *      without fizzy intermodulation aliasing.
 *    - starveNorm controls BJT collector voltage sag and bias asymmetry.
 * 3. Stage 4 — Passive High/Low Summing Tone Stack + Output Recovery Booster:
 *    - Parallel low-pass branch (fc = 482 Hz) and high-pass branch (fc = 1808 Hz)
 *      crossfaded by toneNorm, naturally creating the famous -12 dB mid-notch at 1024 Hz.
 *    - midNorm provides a continuous mid-override (from deeper scoop through flat to boosted mids at 1024 Hz).
 */
class RussianKlangPi {
public:
    void init(float sampleRate) noexcept {
        sr = std::max(sampleRate, 8000.0f);
        invSr = 1.0f / sr;
        invSr4x = 0.25f * invSr;

        stage1HP100.setCutoff(100.0f, invSr);
        interStageHP.setCutoff(75.0f, invSr4x);
        stage2LP6500.setCutoff(6500.0f, invSr4x);
        stage3LP6500.setCutoff(6500.0f, invSr4x);

        // Passive High/Low Summing Tone Stack fixed corner frequencies
        toneStackLP482.setCutoff(482.0f, invSr);
        toneStackHP1808.setCutoff(1808.0f, invSr);
        midOverrideEQ.setBell(1024.0f, 0.85f, 0.0f, invSr);

        dcBlocker.init(sr);
        reset();
    }

    void reset() noexcept {
        stage1HP100.reset();
        interStageHP.reset();
        stage2LP6500.reset();
        stage3LP6500.reset();
        toneStackLP482.reset();
        toneStackHP1808.reset();
        midOverrideEQ.reset();
        oversampler.reset();
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

        // --- STAGE 1: Input BJT Booster (+24 dB -> 15.85x gain) with 100 Hz coupling HPF ---
        constexpr float stage1Gain = 15.8489319f; // +24 dB
        const float vCoupled = stage1HP100.processHP(x);
        const float stage1Bias = 0.06f * sNorm;
        const float vStage1 = FastMath::fastTanhPrecise(vCoupled * stage1Gain * 0.35f + stage1Bias)
                            - FastMath::fastTanhPrecise(stage1Bias);

        // Sustain potentiometer scales inter-stage drive into Stage 2 & Stage 3 (1.5 dB to 13 dB per stage -> 3 dB to 26 dB cascaded)
        const float stageGain = Taper::profileE_DriveGain(dNorm, 1.5f, 13.0f) * 2.4f;

        // Starve sags collector headroom and increases BJT emitter bias shift (gated fuzz character)
        const float headroom = 0.85f - 0.35f * sNorm;
        const float invHeadroom = 1.0f / headroom;
        const float biasShift = 0.14f * sNorm;

        // --- STAGE 2 & STAGE 3: Cascaded Dual BJT Feedback Diode Clipping Stages (4x Oversampled) ---
        float vFuzz = oversampler.processSample(vStage1, [&](float s) noexcept {
            // Stage 2: 1nF HF smoothing low-pass (6.5 kHz) + feedback diode soft/hard clipper
            const float s2Smoothed = stage2LP6500.processLP(s * stageGain);
            const float s2Clipped = headroom * (FastMath::fastTanhPrecise((s2Smoothed + biasShift) * invHeadroom)
                                              - FastMath::fastTanhPrecise(biasShift * invHeadroom));

            // Inter-stage coupling capacitor before Stage 3
            const float s3In = interStageHP.processHP(s2Clipped);

            // Stage 3: Second cascaded 1nF HF smoothing low-pass (6.5 kHz) + feedback diode clipper
            const float s3Smoothed = stage3LP6500.processLP(s3In * stageGain);
            const float s3Clipped = headroom * (FastMath::fastTanhPrecise((s3Smoothed - 0.7f * biasShift) * invHeadroom)
                                              + FastMath::fastTanhPrecise(0.7f * biasShift * invHeadroom));
            return s3Clipped;
        });

        vFuzz = dcBlocker.process(vFuzz);

        // --- STAGE 4: Passive High/Low Summing Tone Stack + Output Recovery Stage ---
        // Parallel low-pass branch (fc = 482 Hz) and high-pass branch (fc = 1808 Hz)
        // Crossfading between them naturally creates the signature -12 dB mid-notch at ~1024 Hz!
        const float lpBranch = toneStackLP482.processLP(vFuzz);
        const float hpBranch = toneStackHP1808.processHP(vFuzz);
        float vTone = (1.0f - tNorm) * lpBranch + tNorm * hpBranch;

        // Mid-notch override via midNorm:
        // At midNorm = 0.5, preserves classic scooped response; > 0.5 restores flat/boosted mids at 1024 Hz
        const float midDb = Taper::profileB_Bipolar(mNorm) * 14.0f;
        midOverrideEQ.setBell(1024.0f, 0.85f, midDb, invSr);
        vTone = midOverrideEQ.process(vTone);
        midOverrideEQ.clampState();

        // Stage 4 BJT recovery booster (+11 dB makeup with gentle collector rail limiting)
        float out = 1.6f * FastMath::fastTanhPrecise(vTone * 1.45f);

        if (!std::isfinite(out)) {
            return 0.0f;
        }
        return std::clamp(out, -8.0f, 8.0f);
    }

private:
    float sr = 44100.0f;
    float invSr = 1.0f / 44100.0f;
    float invSr4x = 1.0f / 176400.0f;

    DSPCommon::OnePoleTPT stage1HP100;
    DSPCommon::OnePoleTPT interStageHP;
    DSPCommon::OnePoleTPT stage2LP6500;
    DSPCommon::OnePoleTPT stage3LP6500;
    DSPCommon::OnePoleTPT toneStackLP482;
    DSPCommon::OnePoleTPT toneStackHP1808;
    DSPCommon::CytomicSVF midOverrideEQ;
    DSPCommon::Oversampler4x oversampler;
    DSPCommon::DCBlocker dcBlocker;
};



using RussianMuff = RussianKlangPi;
using BigMuffPi = RussianKlangPi;

} // namespace TbdAudio::KlangStomp
