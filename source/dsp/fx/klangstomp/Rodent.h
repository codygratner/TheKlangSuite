#pragma once

#include <algorithm>
#include <cmath>
#include "../../common/AudioEffect.h"

namespace TbdAudio::KlangStomp {

/**
 * ProCoRat ("Rodent") — Vintage LM308 Op-Amp Slew-Limited Hard-Clipping Distortion
 *
 * Circuit Topology:
 * 1. Dual RC High-Pass Input Network:
 *    - 60 Hz Coupling High-Pass Filter (bass tightening before distortion)
 *    - 1.54 kHz Treble Pre-Emphasis Shelf / High-Pass branch (4.7k + 2.2uF & 560R + 47nF)
 * 2. LM308 Op-Amp Non-Inverting Gain + Slew-Rate Limiter (4x Oversampled):
 *    - Gain scaled up to +60 dB via Profile E
 *    - Slew-rate limiter restricts maximum dv/dt (maxDeltaPerSample at 4x SR, ~0.3 V/us)
 *      before shunt diodes, transforming harsh high-frequency transients into singing,
 *      violin-like triangular ramps.
 * 3. Symmetrical Hard-Clipping Shunt Diodes to Ground (1N914 Silicon -> Red LED Turbo):
 *    - Headroom threshold scaled by starveNorm from 0.65V (Silicon) to 1.80V (Turbo LED)
 *      with subtle thermal asymmetry when starved.
 * 4. Passive Reverse "Filter" Tone Control + Mid Presence Sculptor:
 *    - OnePoleTPT Low-Pass filter sweeping 480 Hz to 16 kHz via toneNorm
 *    - CytomicSVF Bell at 1.1 kHz controlled by midNorm for extra cut or scoop
 */
class Rodent {
public:
    void init(float sampleRate) noexcept {
        sr = std::max(sampleRate, 8000.0f);
        invSr = 1.0f / sr;
        invSr4x = 0.25f * invSr;

        hp60.setCutoff(60.0f, invSr);
        hp1540.setCutoff(1540.0f, invSr);
        opAmpCompLP.setCutoff(14000.0f, invSr4x);
        toneLP.setCutoff(3200.0f, invSr);
        midBell.setBell(1100.0f, 0.85f, 0.0f, invSr);
        dcBlocker.init(sr);
        reset();
    }

    void reset() noexcept {
        hp60.reset();
        hp1540.reset();
        opAmpCompLP.reset();
        toneLP.reset();
        midBell.reset();
        oversampler.reset();
        dcBlocker.reset();
        slewState = 0.0f;
    }

    [[nodiscard]] float processSample(float x, float driveNorm, float toneNorm, float midNorm, float starveNorm) noexcept {
        if (!std::isfinite(x)) {
            return 0.0f;
        }

        const float dNorm = std::clamp(driveNorm, 0.0f, 1.0f);
        const float tNorm = std::clamp(toneNorm, 0.0f, 1.0f);
        const float mNorm = std::clamp(midNorm, 0.0f, 1.0f);
        const float sNorm = std::clamp(starveNorm, 0.0f, 1.0f);

        // 1. Dual RC High-Pass Network before clipping:
        //    60 Hz base HPF + 1.54 kHz treble pre-emphasis boost proportional to drive
        const float v60 = hp60.processHP(x);
        const float v1540 = hp1540.processHP(v60);
        const float preEmphGain = 1.0f + dNorm * 5.5f;
        const float preIn = v60 + v1540 * preEmphGain;

        // 2. Drive gain (up to +54 dB in active region) & Starve/Diode headroom scaling
        const float driveGain = Taper::profileE_DriveGain(dNorm, 6.0f, 54.0f);

        // Headroom scales between 0.65V (1N914 Silicon) and 1.80V (Turbo LED),
        // while high starveNorm can also tighten slew rate like a sagged LM308 rail
        const float headroom = 0.65f + sNorm * 1.15f;
        const float invHeadroom = 1.0f / headroom;

        // LM308 Slew Rate Limit: ~0.3 V/us nominal, modulated by drive and rail starve
        // At 4x oversampling, maxDeltaPerSample = slewRateVoltsPerSec / (4 * sr)
        const float slewVoltsPerSec = (240000.0f - 110000.0f * dNorm) * (0.65f + 0.55f * (1.0f - 0.5f * sNorm));
        const float maxDeltaPerSample = std::max(slewVoltsPerSec * invSr4x, 0.005f);
        const float dcBiasAsym = 0.045f * sNorm;

        // 3. 4x Oversampled LM308 Slew-Rate Limiter + Shunt Hard-Clipping Diodes
        float clipped = oversampler.processSample(preIn, [&](float s) noexcept {
            const float target = s * driveGain + dcBiasAsym;

            // LM308 Op-Amp Slew-Rate Limiting (dv/dt clamp -> triangular ramp on steep edges)
            const float delta = std::clamp(target - slewState, -maxDeltaPerSample, maxDeltaPerSample);
            slewState += delta;

            // 30pF compensation cap HF roll-off inside LM308
            const float slewed = opAmpCompLP.processLP(slewState);

            // Symmetrical hard-clipping shunt diodes to ground (scaled by headroom)
            return headroom * FastMath::fastTanhPrecise(slewed * invHeadroom);
        });

        clipped = dcBlocker.process(clipped);

        // 4. Passive Reverse 'Filter' Tone Control (480 Hz to 16 kHz)
        //    Higher toneNorm opens the filter from 480 Hz to 16,000 Hz
        const float cutoffHz = Taper::profileD_Cutoff(tNorm, 480.0f, 16000.0f);
        toneLP.setCutoff(cutoffHz, invSr);
        float out = toneLP.processLP(clipped);

        // 5. Mid-Presence Control (midNorm: -9 dB scoop to +9 dB bark at 1.1 kHz)
        const float midDb = Taper::profileB_Bipolar(mNorm) * 9.0f;
        midBell.setBell(1100.0f, 0.9f, midDb, invSr);
        out = midBell.process(out);
        midBell.clampState();

        // Output makeup normalization (algebraic approximation to 0.85 / sqrt(headroom))
        const float makeup = 1.15f / (0.65f + 0.65f * headroom);
        out *= makeup;

        if (!std::isfinite(out)) {
            return 0.0f;
        }
        return std::clamp(out, -8.0f, 8.0f);
    }

private:
    float sr = 44100.0f;
    float invSr = 1.0f / 44100.0f;
    float invSr4x = 1.0f / 176400.0f;
    float slewState = 0.0f;

    DSPCommon::OnePoleTPT hp60;
    DSPCommon::OnePoleTPT hp1540;
    DSPCommon::OnePoleTPT opAmpCompLP;
    DSPCommon::OnePoleTPT toneLP;
    DSPCommon::CytomicSVF midBell;
    DSPCommon::Oversampler4x oversampler;
    DSPCommon::DCBlocker dcBlocker;
};



using ProCoRat = Rodent;

} // namespace TbdAudio::KlangStomp
