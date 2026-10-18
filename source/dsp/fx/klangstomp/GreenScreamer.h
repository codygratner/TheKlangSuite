#pragma once

#include <algorithm>
#include <cmath>
#include "../../common/AudioEffect.h"

namespace TbdAudio::KlangStomp {

/**
 * IbanezTS9 ("Screamer808") — Classic JRC4558D Op-Amp Feedback Soft-Clipper
 *
 * Circuit Topology:
 * 1. Input Emitter-Follower Buffer & Coupling HPF (~35 Hz)
 * 2. Non-Inverting Op-Amp Feedback Loop with Signature 720.5 Hz High-Pass Filter:
 *    - 4.7k resistor + 0.047uF capacitor to ground in the negative feedback path
 *      sets fc = 1 / (2 * pi * 4700 * 47e-9) = 720.48 Hz.
 *    - Frequencies below 720.5 Hz bypass amplification/clipping and remain clean (unity),
 *      while frequencies above 720.5 Hz are amplified and soft-clipped by anti-parallel
 *      MA150 silicon diodes in the feedback loop:
 *      Vout = Vin_clean + softClip(V_hp * gain)
 * 3. Post-Clipping Passive 1k/0.22uF Low-Pass Filter (~723.4 Hz roll-off) +
 *    Active Op-Amp Gyrator Tone Stage sweeping 723 Hz to 4.5 kHz via toneNorm,
 *    plus adjustable mid-hump Q/boost at 800 Hz via midNorm.
 */
class GreenScreamer {
public:
    void init(float sampleRate) noexcept {
        sr = std::max(sampleRate, 8000.0f);
        invSr = 1.0f / sr;
        invSr4x = 0.25f * invSr;

        inputHP.setCutoff(35.0f, invSr);
        feedbackHP720.setCutoff(720.5f, invSr4x);
        feedbackLP.setCutoff(5800.0f, invSr4x);
        toneLP.setCutoff(1800.0f, invSr);
        midHumpEQ.setBell(800.0f, 1.0f, 3.0f, invSr);
        dcBlocker.init(sr);
        reset();
    }

    void reset() noexcept {
        inputHP.reset();
        feedbackHP720.reset();
        feedbackLP.reset();
        toneLP.reset();
        midHumpEQ.reset();
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

        // 1. Input buffer coupling
        const float vIn = inputHP.processHP(x);

        // 2. Non-inverting op-amp feedback loop gain (51k + 500k pot over 4.7k -> 1 + R_fb / 4.7k)
        //    Scaled via Profile E from +12 dB (min drive) to +41 dB (max drive)
        const float loopGain = Taper::profileE_DriveGain(dNorm, 12.0f, 41.0f);

        // Starve controls diode forward drop / asymmetrical warm sag (0.55V to 1.15V)
        const float diodeVt = 0.95f - 0.38f * sNorm;
        const float invVt = 1.0f / diodeVt;
        const float asymOffset = 0.08f * sNorm;

        // 3. 4x Oversampled Non-Inverting Op-Amp Feedback Soft-Clipper
        //    Vout = Vin_clean + softClip(V_hp * gain)
        float vClipped = oversampler.processSample(vIn, [&](float s) noexcept {
            // 720.5 Hz HPF in feedback path: bass (<720.5 Hz) stays at unity clean gain!
            const float vHp = feedbackHP720.processHP(s);
            // 51pF feedback cap smooths ultrasonic harshness
            const float vDrive = feedbackLP.processLP(vHp * loopGain);

            // Soft-clipping silicon diodes in feedback loop (with subtle asymmetry from starveNorm)
            const float shaped = diodeVt * (FastMath::fastTanhPrecise((vDrive + asymOffset) * invVt)
                                          - FastMath::fastTanhPrecise(asymOffset * invVt));

            // Non-inverting op-amp equation retains dynamic dry signal sum
            return s + shaped;
        });

        vClipped = dcBlocker.process(vClipped);

        // 4. Adjustable 800 Hz Mid-Hump Q/Boost via midNorm
        const float midBipolar = Taper::profileB_Bipolar(mNorm);
        const float midBoostDb = 3.5f + midBipolar * 8.5f; // Classic mid-hump centered at +3.5 dB, adjustable [-5, +12] dB
        const float midQ = 0.75f + 0.95f * mNorm;
        midHumpEQ.setBell(800.0f, midQ, midBoostDb, invSr);
        float vMid = midHumpEQ.process(vClipped);
        midHumpEQ.clampState();

        // 5. Active Low-Pass Tone Stage (723 Hz to 4.5 kHz via toneNorm)
        const float toneCutoffHz = Taper::profileD_Cutoff(tNorm, 723.0f, 4500.0f);
        toneLP.setCutoff(toneCutoffHz, invSr);
        float out = toneLP.processLP(vMid);

        // Makeup level alignment
        out *= 0.65f;

        if (!std::isfinite(out)) {
            return 0.0f;
        }
        return std::clamp(out, -8.0f, 8.0f);
    }

private:
    float sr = 44100.0f;
    float invSr = 1.0f / 44100.0f;
    float invSr4x = 1.0f / 176400.0f;

    DSPCommon::OnePoleTPT inputHP;
    DSPCommon::OnePoleTPT feedbackHP720;
    DSPCommon::OnePoleTPT feedbackLP;
    DSPCommon::OnePoleTPT toneLP;
    DSPCommon::CytomicSVF midHumpEQ;
    DSPCommon::Oversampler4x oversampler;
    DSPCommon::DCBlocker dcBlocker;
};



using Screamer808 = GreenScreamer;
using IbanezTS9 = GreenScreamer;

} // namespace TbdAudio::KlangStomp
