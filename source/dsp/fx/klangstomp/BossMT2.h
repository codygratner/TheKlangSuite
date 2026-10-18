#pragma once

#include "../../common/AudioEffect.h"

namespace TbdAudio::KlangStomp {

/**
 * BossMT2 ("Industrial Zone")
 *
 * Authentic component-level DSP topology of the Boss MT-2 Metal Zone circuit:
 * 1. Pre-distortion band-limiting: 180 Hz HPF + 1.6 kHz Bandpass emphasis to sharpen pick attack.
 * 2. 4x Oversampled dual-stage NJM4558 soft feedback clip + hard silicon diode clip (±0.65V scaled by starveNorm).
 * 3. 3-Band Active Gyrator EQ:
 *    - Low Shelf at 95 Hz, High Shelf at 4.0 kHz (governed by toneNorm).
 *    - Parametric Mid Gyrator (CytomicSVF::setBell) with Q = 2.2, frequency swept from
 *      250 Hz to 4500 Hz by midNorm with musical boost/scoop contouring.
 * 4. Post-EQ steep reconstruction lowpass at 4.2 kHz (CytomicSVF::setLowpass) + 5 Hz DC blocker.
 */
class BossMT2 {
public:
    void init(float sampleRate) noexcept {
        sr = std::max(sampleRate, 8000.0f);
        invSr = 1.0f / sr;

        preHPF.setCutoff(180.0f, invSr);
        preAttackBPF.setBandpass(1600.0f, 1.6f, invSr);
        postCabLPF.setLowpass(4200.0f, 0.85f, invSr);
        dcBlocker.init(sr);

        cachedTone = -1.0f;
        cachedMid = -1.0f;
        updateEQ(0.5f, 0.5f);
        reset();
    }

    void reset() noexcept {
        preHPF.reset();
        preAttackBPF.reset();
        oversampler.reset();
        lowShelf.reset();
        highShelf.reset();
        midGyrator.reset();
        postCabLPF.reset();
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

        // Update 3-Band Active Gyrator EQ when tone or mid parameters change
        if (std::abs(tNorm - cachedTone) > 1e-4f || std::abs(mNorm - cachedMid) > 1e-4f) {
            updateEQ(tNorm, mNorm);
        }

        // 1. Pre-distortion band-limiting: 180 Hz HPF + 1.6 kHz Bandpass pick-attack emphasis
        const float hp = preHPF.processHP(x);
        const float bp = preAttackBPF.process(hp);
        preAttackBPF.clampState();
        const float preFocused = hp + 1.85f * bp;

        // Dual-stage gain staging (+8 dB to +45 dB total cascade gain)
        const float totalDrive = Taper::profileE_DriveGain(dNorm, 8.0f, 45.0f);
        const float stage1Gain = 1.0f + 0.28f * totalDrive;
        const float stage2Gain = 1.2f + 0.18f * totalDrive;

        // Rail voltage headroom scaling around ±0.65V silicon diode threshold
        // 0.0 = 3V starved sag, 0.5 = 9V nominal (±0.65V), 1.0 = 18V expanded headroom
        const float railScale = 0.34f + sNorm * (1.32f + 0.68f * (sNorm - 0.5f));
        const float vDiodePos = 0.65f * railScale;
        const float vDiodeNeg = 0.65f * railScale * (1.0f - 0.22f * (1.0f - sNorm));
        const float invDiodePos = 1.0f / std::max(vDiodePos, 0.05f);
        const float invDiodeNeg = 1.0f / std::max(vDiodeNeg, 0.05f);
        const float starveBias = (1.0f - sNorm) * 0.12f;

        // 2. 4x Oversampled Dual-Stage NJM4558 Soft Feedback + Hard Silicon Diode Clipper
        float clipped = oversampler.processSample(preFocused, [&](float s) noexcept -> float {
            // Stage 1: NJM4558 op-amp soft feedback loop saturation
            float v1 = FastMath::fastTanh(s * stage1Gain + starveBias) - (starveBias * 0.5f);

            // Stage 2: Hard silicon diode shunt clip (±0.65V nominal scaled by starveNorm)
            float v2In = v1 * stage2Gain;
            float hardClip = std::clamp(v2In, -vDiodeNeg * 1.35f, vDiodePos * 1.35f);
            float diodeOut = (hardClip >= 0.0f)
                ? vDiodePos * FastMath::fastTanhPrecise(hardClip * invDiodePos)
                : vDiodeNeg * FastMath::fastTanhPrecise(hardClip * invDiodeNeg);

            return diodeOut;
        });

        // Compensate output level for diode headroom scaling
        const float makeup = 0.85f / std::sqrt(std::max(railScale, 0.25f));
        float sig = clipped * makeup;

        // 3. 3-Band Active Gyrator EQ
        sig = lowShelf.process(sig);
        sig = highShelf.process(sig);
        sig = midGyrator.process(sig);
        lowShelf.clampState();
        highShelf.clampState();
        midGyrator.clampState();

        // 4. Post-EQ steep reconstruction lowpass at 4.2 kHz + DC blocker
        sig = postCabLPF.process(sig);
        postCabLPF.clampState();
        sig = dcBlocker.process(sig);

        if (!std::isfinite(sig)) {
            reset();
            return 0.0f;
        }
        return std::clamp(sig, -8.0f, 8.0f);
    }

private:
    void updateEQ(float toneNorm, float midNorm) noexcept {
        cachedTone = toneNorm;
        cachedMid = midNorm;

        // Low Shelf at 95 Hz & High Shelf at 4.0 kHz governed by toneNorm
        // Balanced industrial punch across the sweep with tight low-end + articulate top
        const float tilt = Taper::profileB_Bipolar(toneNorm); // [-1..+1]
        const float lowGainDb = 4.0f - 8.0f * tilt;           // +12 dB warm weight down to -4 dB tight cut
        const float highGainDb = 11.0f * tilt;                // -11 dB dark to +11 dB searing industrial bite
        lowShelf.setLowShelf(95.0f, 0.707f, lowGainDb, invSr);
        highShelf.setHighShelf(4000.0f, 0.707f, highGainDb, invSr);

        // Parametric Mid Gyrator (Q = 2.2), frequency swept 250 Hz to 4500 Hz by midNorm
        const float midFreqHz = Taper::profileD_Cutoff(midNorm, 250.0f, 4500.0f);
        // Lower-half sweeps deep industrial scoop (-14 dB..0 dB), upper-half sweeps cutting mid boost (0..+15 dB)
        const float midGainDb = (midNorm < 0.5f)
            ? -14.0f * (1.0f - 2.0f * midNorm)
            : 15.0f * (2.0f * (midNorm - 0.5f));
        midGyrator.setBell(midFreqHz, 2.2f, midGainDb, invSr);
    }

    float sr = 44100.0f;
    float invSr = 1.0f / 44100.0f;
    float cachedTone = -1.0f;
    float cachedMid = -1.0f;

    DSPCommon::OnePoleTPT preHPF;
    DSPCommon::CytomicSVF preAttackBPF;
    DSPCommon::Oversampler4x oversampler;
    DSPCommon::CytomicSVF lowShelf;
    DSPCommon::CytomicSVF highShelf;
    DSPCommon::CytomicSVF midGyrator;
    DSPCommon::CytomicSVF postCabLPF;
    DSPCommon::DCBlocker dcBlocker;
};

using IndustrialZone = BossMT2;
using KlangZone = BossMT2;

} // namespace TbdAudio::KlangStomp
