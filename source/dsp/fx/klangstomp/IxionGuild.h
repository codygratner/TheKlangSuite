#pragma once

#include "../../common/AudioEffect.h"

namespace TbdAudio::KlangStomp {

/**
 * KlonCentaur ("SovereignCentaur")
 * Component-level model of the Finnegan Klon Centaur Professional Overdrive:
 * - Input buffer & sub-bass conditioning (120 Hz body / 400 Hz overdrive split)
 * - Dual-Ganged Gain Potentiometer Architecture (VR1a Overdrive / VR1b Clean Feed):
 *   - Clean path (VR1b): retains low-end weight and unclipped pick transients, crossfading down as drive increases.
 *   - Overdrive path (VR1a): 400 Hz high-pass pre-emphasis driven into 4x oversampled NOS 1N34A Germanium
 *     soft-clipping diodes (Vf ~ 0.35V with realistic forward-drop knee and subtle even-harmonic asymmetry).
 * - MAX1044 Charge Pump Headroom Stage (+18V/-9V rails -> 27V differential swing):
 *   - Controlled by `starveNorm` (0.0 = starved 4.5V sag, 0.5 = standard 9V, 1.0 = full +18V charge-pump headroom).
 * - Active Vocal Mid Body Bell at 1.0 kHz (`midNorm`) + Active Baxandall Treble Shelf at 2.0 kHz (`toneNorm`).
 */
class IxionGuild {
public:
    void init(float sampleRate) noexcept {
        sr = std::max(sampleRate, 8000.0f);
        invSr = 1.0f / sr;

        // Input coupling & dual-path frequency splitting
        cleanBassLPF.setCutoff(390.0f, invSr);
        cleanFeedHPF.setCutoff(65.0f, invSr);
        drivePreHPF.setCutoff(400.0f, invSr);
        driveAntiFizzLPF.setCutoff(4800.0f, invSr);
        postClipLPF.setCutoff(5200.0f, invSr);

        midBellFilter.setBell(1000.0f, 0.85f, 0.0f, invSr);
        trebleShelfFilter.setHighShelf(2000.0f, 0.707f, 0.0f, invSr);
        dcBlocker.init(sr);

        reset();
    }

    void reset() noexcept {
        cleanBassLPF.reset();
        cleanFeedHPF.reset();
        drivePreHPF.reset();
        driveAntiFizzLPF.reset();
        postClipLPF.reset();
        oversampler.reset();
        midBellFilter.reset();
        trebleShelfFilter.reset();
        dcBlocker.reset();
        envFollower = 0.0f;
    }

    [[nodiscard]] float processSample(float x, float driveNorm, float toneNorm, float midNorm, float starveNorm) noexcept {
        if (!std::isfinite(x)) return 0.0f;

        const float d = std::clamp(driveNorm, 0.0f, 1.0f);
        const float t = std::clamp(toneNorm, 0.0f, 1.0f);
        const float m = std::clamp(midNorm, 0.0f, 1.0f);
        const float s = std::clamp(starveNorm, 0.0f, 1.0f);

        // --------------------------------------------------------------------
        // 1. Dual-Ganged Gain Potentiometer (VR1a / VR1b)
        // --------------------------------------------------------------------
        // Clean feed (VR1b): loud and full at low drive, tapers down as VR1a takes over,
        // while preserving the low-frequency foundation (< 390 Hz) that bypasses the clipper.
        const float cleanBroadbandGain = (1.0f - 0.78f * d * (0.65f + 0.35f * d)) * 1.15f;
        const float bassFoundation = cleanBassLPF.processLP(x);
        const float cleanFull = cleanFeedHPF.processHP(x);
        const float cleanPath = cleanFull * cleanBroadbandGain + bassFoundation * (0.45f + 0.55f * d);

        // --------------------------------------------------------------------
        // 2. Overdrive Path (VR1a): 400 Hz HPF + Pre-Clipping Gain Stage
        // --------------------------------------------------------------------
        // Profile E non-linear drive gain (up to +32 dB into 1N34A diode network)
        const float vr1aGain = (d <= 0.001f) ? 0.0f : (Taper::profileE_DriveGain(d, -6.0f, 32.0f) * d);
        float dirtyIn = drivePreHPF.processHP(x);
        dirtyIn = driveAntiFizzLPF.processLP(dirtyIn) * vr1aGain;

        // Track envelope for dynamic Germanium thermal/capacitance softness
        const float absIn = std::abs(dirtyIn);
        envFollower += (absIn > envFollower ? 0.02f : 0.0008f) * (absIn - envFollower);

        // --------------------------------------------------------------------
        // 3. 4x Oversampled NOS 1N34A Germanium Shunt Diode Clipper
        //    Forward voltage Vf ~ 0.35V, soft logarithmic/exponential knee,
        //    and subtle even-harmonic asymmetry.
        // --------------------------------------------------------------------
        constexpr float vfPos = 0.35f;
        constexpr float vfNeg = 0.385f; // slight junction asymmetry for warm 2nd harmonic
        constexpr float invVfPos = 1.0f / vfPos;
        constexpr float invVfNeg = 1.0f / vfNeg;
        const float dynamicKnee = 1.0f / (1.0f + 0.12f * std::min(envFollower, 4.0f));

        float clipped = oversampler.processSample(dirtyIn, [&](float osX) noexcept {
            // Op-amp feedback path + shunt 1N34A Germanium soft knee
            float biased = osX + 0.018f * dynamicKnee;
            if (biased >= 0.0f) {
                float u = biased * invVfPos;
                // Germanium soft-knee compression curve (blend of soft tanh and algebraic knee)
                float soft = FastMath::fastTanhPrecise(u * 0.85f);
                float knee = u / (1.0f + 0.65f * u);
                return vfPos * (0.65f * soft + 0.35f * knee) * 2.4f;
            } else {
                float u = -biased * invVfNeg;
                float soft = FastMath::fastTanhPrecise(u * 0.80f);
                float knee = u / (1.0f + 0.58f * u);
                return -vfNeg * (0.65f * soft + 0.35f * knee) * 2.4f;
            }
        });

        clipped = postClipLPF.processLP(clipped);

        // --------------------------------------------------------------------
        // 4. Summing Amplifier & MAX1044 +18V Charge Pump Headroom Stage
        //    `starveNorm` scales virtual supply headroom from starved 4.5V (0.0)
        //    through standard 9V (0.5) up to full 27V differential swing (1.0).
        // --------------------------------------------------------------------
        const float dirtyMixGain = 0.35f + 0.95f * std::sqrt(d);
        float summed = cleanPath + clipped * dirtyMixGain;

        // Virtual rail headroom in normalized signal units (0.55 at starved -> 3.6 at +18V rail)
        const float railHeadroom = 0.55f + 3.05f * (s * s * 0.65f + s * 0.35f);
        const float invRail = 1.0f / railHeadroom;
        // Asymmetric op-amp rail saturation when starved, transparent when charge pump is full
        const float railBias = (1.0f - s) * 0.08f;
        summed = railHeadroom * FastMath::fastTanhPrecise((summed + railBias) * invRail) - railBias * 0.8f;

        // --------------------------------------------------------------------
        // 5. Vocal Mid Body Bell (1.0 kHz) & Active Baxandall Treble Shelf (2.0 kHz)
        // --------------------------------------------------------------------
        // Centaur signature vocal midrange honk centered at 1.0 kHz (+/- 10 dB around nominal +2.5 dB push)
        const float midGainDb = 2.5f + Taper::profileB_Bipolar(m) * 9.5f;
        midBellFilter.setBell(1000.0f, 0.90f, midGainDb, invSr);
        float shaped = midBellFilter.process(summed);

        // Active 2.0 kHz Treble shelf (-12 dB to +12 dB)
        const float trebleGainDb = (t - 0.5f) * 24.0f;
        trebleShelfFilter.setHighShelf(2000.0f, 0.707f, trebleGainDb, invSr);
        shaped = trebleShelfFilter.process(shaped);

        midBellFilter.clampState();
        trebleShelfFilter.clampState();

        // Output level compensation & DC removal
        float out = dcBlocker.process(shaped * 0.92f);
        if (!std::isfinite(out)) return 0.0f;
        return std::clamp(out, -8.0f, 8.0f);
    }

private:
    float sr = 44100.0f;
    float invSr = 1.0f / 44100.0f;
    float envFollower = 0.0f;

    DSPCommon::OnePoleTPT cleanBassLPF;
    DSPCommon::OnePoleTPT cleanFeedHPF;
    DSPCommon::OnePoleTPT drivePreHPF;
    DSPCommon::OnePoleTPT driveAntiFizzLPF;
    DSPCommon::OnePoleTPT postClipLPF;

    DSPCommon::Oversampler4x oversampler;
    DSPCommon::CytomicSVF midBellFilter;
    DSPCommon::CytomicSVF trebleShelfFilter;
    DSPCommon::DCBlocker dcBlocker;
};



using SovereignCentaur = IxionGuild;
using KlonCentaur = IxionGuild;

} // namespace TbdAudio::KlangStomp
