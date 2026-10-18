#pragma once

#include "../../common/AudioEffect.h"

namespace TbdAudio::KlangStomp {

/**
 * DOD250 ("YellowPreamp250")
 * Component-level model of the classic grey/yellow DOD Overdrive Preamp 250:
 * - Non-inverting LM741 single op-amp gain stage with variable bass rolloff in the negative feedback
 *   ground return (720 Hz high-pass contour at higher gain settings, tightening low end as drive rises).
 * - Authentic LM741 0.5 V/us slew-rate limiting and dynamic op-amp rail asymmetry.
 * - 4x Oversampled passive shunt clipping diodes to ground:
 *   - Continuously morphs between vintage 1N270 Germanium (warm, soft-knee asymmetric compression)
 *     and 1N4148 Silicon (crisp, open, aggressive crunch) via `midNorm` / `starveNorm`.
 * - Post-clipping tone tilt EQ (`toneNorm`) + midrange presence contour (`midNorm`) + DC blocker.
 */
class DOD250 {
public:
    void init(float sampleRate) noexcept {
        sr = std::max(sampleRate, 8000.0f);
        invSr = 1.0f / sr;

        inputCouplingHPF.setCutoff(35.0f, invSr);
        feedbackReturnHPF.setCutoff(720.0f, invSr);
        lm741BandwidthLPF.setCutoff(7500.0f, invSr);
        toneLPF.setCutoff(1800.0f, invSr);
        toneHPF.setCutoff(1200.0f, invSr);
        midContourSVF.setBell(850.0f, 0.85f, 0.0f, invSr);
        dcBlocker.init(sr);

        reset();
    }

    void reset() noexcept {
        inputCouplingHPF.reset();
        feedbackReturnHPF.reset();
        lm741BandwidthLPF.reset();
        toneLPF.reset();
        toneHPF.reset();
        midContourSVF.reset();
        oversampler.reset();
        dcBlocker.reset();
        slewState = 0.0f;
    }

    [[nodiscard]] float processSample(float x, float driveNorm, float toneNorm, float midNorm, float starveNorm) noexcept {
        if (!std::isfinite(x)) return 0.0f;

        const float d = std::clamp(driveNorm, 0.0f, 1.0f);
        const float t = std::clamp(toneNorm, 0.0f, 1.0f);
        const float m = std::clamp(midNorm, 0.0f, 1.0f);
        const float s = std::clamp(starveNorm, 0.0f, 1.0f);

        // --------------------------------------------------------------------
        // 1. Input Coupling & Non-Inverting Feedback Ground Return (720 Hz HPF)
        // --------------------------------------------------------------------
        // In the DOD 250 schematic, the 4.7uF + resistor network in the inverting feedback leg
        // shifts the bass rolloff toward ~720 Hz as the Gain pot increases, keeping low gain full
        // and tightening high gain so bass never farts out.
        float in = inputCouplingHPF.processHP(x);
        const float hp720 = feedbackReturnHPF.processHP(in);
        const float tightBlend = 0.25f + 0.70f * (d * d);
        float preAmpIn = in * (1.0f - tightBlend) + hp720 * tightBlend;

        // Non-inverting LM741 voltage gain (1 + Rf/Rg, up to +38 dB)
        const float opAmpGain = Taper::profileE_DriveGain(d, 2.0f, 38.0f);
        float opAmpOut = preAmpIn * opAmpGain;

        // --------------------------------------------------------------------
        // 2. LM741 Slew-Rate Limiter (0.5 V/us nominal, modulated by starveNorm)
        // --------------------------------------------------------------------
        // Normalized to +/-1.0 = ~4.5V peak swing -> 0.5 V/us corresponds to ~111,000 norm-units/sec.
        // Starving the supply reduces tail current in the LM741 differential pair, slowing slew rate further.
        const float slewScale = 0.35f + 0.95f * s;
        const float maxDeltaPerSample = (28000.0f * slewScale) * invSr;
        const float delta = std::clamp(opAmpOut - slewState, -maxDeltaPerSample, maxDeltaPerSample);
        slewState += delta;
        opAmpOut = lm741BandwidthLPF.processLP(slewState);

        // Op-amp internal rail clipping before passive shunt diodes (starved 9V sag vs full rail)
        const float opAmpRail = 1.15f + 1.85f * s;
        const float invOpAmpRail = 1.0f / opAmpRail;
        const float opAmpAsym = 0.06f * (1.0f - 0.5f * s);
        opAmpOut = opAmpRail * FastMath::fastTanhPrecise((opAmpOut + opAmpAsym) * invOpAmpRail) - opAmpAsym;

        // --------------------------------------------------------------------
        // 3. 4x Oversampled Shunt Diode Clipper to Ground:
        //    Morph between 1N270 Germanium (soft asymmetric, Vf ~ 0.32V)
        //    and 1N4148 Silicon (crisp, open crunch, Vf ~ 0.68V)
        // --------------------------------------------------------------------
        // Diode morph: controlled by combination of starveNorm and midNorm
        // (low starve / warm mid -> 1N270 Germanium; higher starve / crisp -> 1N4148 Silicon)
        const float siliconMorph = std::clamp(0.55f * s + 0.45f * m, 0.0f, 1.0f);

        float clipped = oversampler.processSample(opAmpOut, [&](float v) noexcept {
            // 1N270 Germanium branch (asymmetric 2-diode + 1-diode vintage mod feel, soft knee)
            constexpr float gePosVf = 0.34f;
            constexpr float geNegVf = 0.46f;
            float geOut;
            if (v >= 0.0f) {
                float u = v * (1.0f / gePosVf);
                geOut = gePosVf * (0.60f * FastMath::fastTanhPrecise(u * 0.75f) + 0.40f * (u / (1.0f + 0.55f * u))) * 2.1f;
            } else {
                float u = -v * (1.0f / geNegVf);
                geOut = -geNegVf * (0.65f * FastMath::fastTanhPrecise(u * 0.80f) + 0.35f * (u / (1.0f + 0.50f * u))) * 2.1f;
            }

            // 1N4148 Silicon branch (higher forward voltage threshold, firmer knee, open transient)
            constexpr float siPosVf = 0.68f;
            constexpr float siNegVf = 0.74f;
            float siOut;
            if (v >= 0.0f) {
                float u = v * (1.0f / siPosVf);
                // Cubic hard-knee transition into tanh saturation
                float uClamped = std::min(u, 2.5f);
                float firm = FastMath::fastTanhPrecise(u * (1.0f + 0.18f * uClamped * uClamped));
                siOut = siPosVf * firm * 1.65f;
            } else {
                float u = -v * (1.0f / siNegVf);
                float uClamped = std::min(u, 2.5f);
                float firm = FastMath::fastTanhPrecise(u * (1.0f + 0.15f * uClamped * uClamped));
                siOut = -siNegVf * firm * 1.65f;
            }

            return geOut + siliconMorph * (siOut - geOut);
        });

        // --------------------------------------------------------------------
        // 4. Midrange Body Contour (`midNorm`) & Post-Clipping Tone Tilt (`toneNorm`)
        // --------------------------------------------------------------------
        const float midDb = Taper::profileB_Bipolar(m) * 9.0f;
        midContourSVF.setBell(850.0f, 0.85f, midDb, invSr);
        float voiced = midContourSVF.process(clipped);
        midContourSVF.clampState();

        // Passive tilt network around 1.4 kHz
        const float lowTilt = toneLPF.processLP(voiced);
        const float highTilt = toneHPF.processHP(voiced);
        const float tilt = (t - 0.5f) * 2.0f; // [-1, +1]
        float tilted = voiced + (tilt < 0.0f ? (-tilt * 0.75f * (lowTilt - highTilt))
                                             : (tilt * 0.85f * (highTilt - lowTilt * 0.35f)));

        // Makeup level & DC block
        float out = dcBlocker.process(tilted * 0.95f);
        if (!std::isfinite(out)) return 0.0f;
        return std::clamp(out, -8.0f, 8.0f);
    }

private:
    float sr = 44100.0f;
    float invSr = 1.0f / 44100.0f;
    float slewState = 0.0f;

    DSPCommon::OnePoleTPT inputCouplingHPF;
    DSPCommon::OnePoleTPT feedbackReturnHPF;
    DSPCommon::OnePoleTPT lm741BandwidthLPF;
    DSPCommon::OnePoleTPT toneLPF;
    DSPCommon::OnePoleTPT toneHPF;
    DSPCommon::CytomicSVF midContourSVF;
    DSPCommon::Oversampler4x oversampler;
    DSPCommon::DCBlocker dcBlocker;
};

using YellowPreamp250 = DOD250;
using Yellow250 = DOD250;

} // namespace TbdAudio::KlangStomp
