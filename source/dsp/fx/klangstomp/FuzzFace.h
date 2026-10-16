#pragma once

#include "../../common/AudioEffect.h"

namespace TbdAudio::KlangStomp {

/**
 * FuzzFace ("VelcroFace")
 * Component-level model of the Dallas Arbiter Fuzz Face (NKT275 / AC128 Germanium & Starved Bias):
 * - Inductive Guitar Pickup Loading Simulation (`toneNorm`): models the low input impedance of Q1
 *   interacting with pickup inductance to roll off resonant highs or open into stinging bite.
 * - Direct-Coupled 2-Transistor BJT Cascade (Q1 Common-Emitter -> Q2 Common-Emitter) with
 *   global negative feedback from Q2 emitter back to Q1 base and C2 emitter bypass boost.
 * - Bias & Voltage Starvation ("Velcro" Sputtering Gate via `starveNorm`):
 *   - When `starveNorm >= 0.5`: warm, liquid, singing 4.5V collector bias (`Vc ~ 4.5V`, `Vs = 9V`) with
 *     iconic guitar-volume cleanup and rich octave-like intermodulation bloom.
 *   - When `starveNorm < 0.5`: collector bias shifts cold (`Vc > 6.5V` / supply sag `< 5V`), introducing
 *     a dynamic base-emitter deadzone threshold (`Vbe` cutoff starvation) where decaying notes and small
 *     signals sputter, pinch off, gate, and rip like tearing velcro!
 * - Midrange Voicing Contour (`midNorm`) + 4x Oversampled nonlinear BJT solver.
 */
class FuzzFace {
public:
    void init(float sampleRate) noexcept {
        sr = std::max(sampleRate, 8000.0f);
        invSr = 1.0f / sr;

        inputCouplingHPF.setCutoff(42.0f, invSr);
        emitterBypassHPF.setCutoff(150.0f, invSr);
        pickupLoadSVF.setLowpass(4500.0f, 1.35f, invSr);
        midContourSVF.setBell(900.0f, 0.85f, 0.0f, invSr);
        postCabRollLPF.setCutoff(6200.0f, invSr);
        dcBlocker.init(sr);

        reset();
    }

    void reset() noexcept {
        inputCouplingHPF.reset();
        emitterBypassHPF.reset();
        pickupLoadSVF.reset();
        midContourSVF.reset();
        postCabRollLPF.reset();
        oversampler.reset();
        dcBlocker.reset();
        q2EmitterFeedback = 0.0f;
        couplingCapState = 0.0f;
        envState = 0.0f;
    }

    [[nodiscard]] float processSample(float x, float driveNorm, float toneNorm, float midNorm, float starveNorm) noexcept {
        if (!std::isfinite(x)) return 0.0f;

        const float d = std::clamp(driveNorm, 0.0f, 1.0f);
        const float t = std::clamp(toneNorm, 0.0f, 1.0f);
        const float m = std::clamp(midNorm, 0.0f, 1.0f);
        const float s = std::clamp(starveNorm, 0.0f, 1.0f);

        // --------------------------------------------------------------------
        // 1. Inductive Pickup Loading Simulation (`toneNorm`)
        //    Fuzz Face has a notoriously low ~5k-10k input impedance that loads passive pickups.
        //    `toneNorm` sweeps the pickup resonant loading cutoff from dark/wooly (1.4 kHz)
        //    to biting/unloaded (9.5 kHz) with subtle resonant Q peak.
        // --------------------------------------------------------------------
        const float pickupCutoff = Taper::profileD_Cutoff(t, 1400.0f, 9500.0f);
        const float pickupQ = 0.75f + 0.85f * t;
        pickupLoadSVF.setLowpass(pickupCutoff, pickupQ, invSr);
        float in = pickupLoadSVF.process(inputCouplingHPF.processHP(x));
        pickupLoadSVF.clampState();

        // 20uF Emitter Bypass Capacitor on Q2 (boosts frequencies > 150 Hz as Fuzz pot increases)
        const float emitterBoost = emitterBypassHPF.processHP(in);
        in = in * 0.55f + emitterBoost * (0.45f + 1.15f * d);

        // Input Fuzz drive gain (models the 1k Fuzz pot splitting Q2 emitter signal + input drive)
        const float fuzzGain = Taper::profileE_DriveGain(d, 6.0f, 42.0f);

        // --------------------------------------------------------------------
        // 2. Bias & Voltage Starvation Parameters (`starveNorm`)
        //    - s < 0.5: Starved / Misbiased Velcro regime (Vc > 6.5V, supply < 5V)
        //    - s >= 0.5: Ideal 4.5V NKT275 Germanium bias -> hot liquid sustain at 1.0
        // --------------------------------------------------------------------
        const float velcroAmount = (s < 0.5f) ? ((0.5f - s) * 2.0f) : 0.0f; // 1.0 at s=0, 0.0 at s>=0.5
        const float hotBiasAmount = (s > 0.5f) ? ((s - 0.5f) * 2.0f) : 0.0f;

        // Virtual battery rail sag (drops from 1.0 down to 0.38 under extreme starvation)
        const float supplyRail = 0.38f + 0.62f * (1.0f - 0.75f * velcroAmount) + 0.25f * hotBiasAmount;
        const float invSupplyRail = 1.0f / supplyRail;

        // Deadzone threshold for cold collector bias (Vc > 6.5V pinches Q2 base-emitter below Vbe)
        const float deadzoneThreshold = velcroAmount * velcroAmount * 0.42f;
        // Charge-storage blocking distortion shift (couples envelope into Q2 base bias)
        const float blockingShiftGain = 0.15f + 0.85f * velcroAmount;

        // --------------------------------------------------------------------
        // 3. 4x Oversampled Direct-Coupled Q1 -> Q2 BJT Cascade with Global Shunt Feedback
        // --------------------------------------------------------------------
        float fuzzOut = oversampler.processSample(in, [&](float osIn) noexcept {
            // Global negative feedback from Q2 emitter to Q1 base (100k feedback resistor)
            // Reduced feedback at high Fuzz settings or high starvation causes steeper square edges.
            const float fbAmount = (0.18f - 0.10f * d) * (1.0f - 0.55f * velcroAmount);
            float q1Base = osIn * fuzzGain - q2EmitterFeedback * fbAmount;

            // --- Stage 1: Q1 Common-Emitter BJT (asymmetric collector saturation / cutoff) ---
            const float q1Bias = -0.08f - 0.18f * velcroAmount + 0.06f * hotBiasAmount;
            float v1 = q1Base + q1Bias;
            // Asymmetric BJT collector curve: sharp cutoff on one polarity, soft saturation on the other
            float q1Collector;
            if (v1 >= 0.0f) {
                q1Collector = -FastMath::fastTanhPrecise(v1 * 1.35f);
            } else {
                q1Collector = -0.78f * FastMath::fastTanhPrecise(v1 * 0.90f);
            }

            // Dynamic charge accumulation on base-collector junction (causes sputtering recovery on peaks)
            float rect = std::abs(q1Collector);
            envState += (rect > envState ? 0.08f : 0.0012f) * (rect - envState);
            couplingCapState += 0.004f * (q1Collector - couplingCapState);

            // --- Stage 2: Q2 Direct-Coupled BJT with "Velcro" Starved-Bias Deadzone ---
            float q2Base = (q1Collector - 0.35f * couplingCapState * blockingShiftGain) * (2.2f + 2.8f * d);
            // Dynamic bias shift from envelope sag
            q2Base -= velcroAmount * (0.28f + 0.35f * envState);

            // Base-Emitter Deadzone Gate (when starved, signals below Vbe cutoff pinch off abruptly!)
            float q2Conducting = 0.0f;
            const float absBase = std::abs(q2Base);
            if (absBase > deadzoneThreshold) {
                const float sign = (q2Base >= 0.0f) ? 1.0f : -1.0f;
                const float excess = absBase - deadzoneThreshold;
                // Ripping velcro edge: steep transconductance turn-on once Vbe threshold is crossed
                const float turnOnGain = 1.6f + 3.2f * velcroAmount;
                q2Conducting = sign * (excess * turnOnGain);
            } else if (velcroAmount < 0.99f) {
                // Micro-leakage in normal/warm germanium mode so zero-crossings stay liquid when not starved
                const float leakage = (1.0f - velcroAmount) * (1.0f - velcroAmount);
                q2Conducting = q2Base * leakage * 1.4f;
            }

            // Q2 Collector saturation against virtual supply rail (Vs < 5V when starved, 9V+ when full)
            const float q2Asym = 0.14f + 0.22f * velcroAmount;
            float q2Collector = supplyRail * FastMath::fastTanhPrecise((q2Conducting + q2Asym) * invSupplyRail)
                              - supplyRail * FastMath::fastTanhPrecise(q2Asym * invSupplyRail);

            // Update Q2 emitter state for next sample's global negative feedback to Q1 base
            q2EmitterFeedback = std::clamp(q2Collector * 0.65f, -2.5f, 2.5f);
            return q2Collector;
        });

        // --------------------------------------------------------------------
        // 4. Midrange Contour (`midNorm`) & High-Frequency Smoothing
        // --------------------------------------------------------------------
        // Allows sculpting from scooped vintage fuzz to thick mid-forward lead tone
        const float midDb = Taper::profileB_Bipolar(m) * 10.5f;
        midContourSVF.setBell(900.0f, 0.85f, midDb, invSr);
        float voiced = midContourSVF.process(fuzzOut);
        midContourSVF.clampState();

        // Smooth out harsh ultrasonic square edges while preserving velcro rip
        voiced = postCabRollLPF.processLP(voiced);

        // Output makeup gain (compensates for lower rail voltage when starved)
        const float makeup = (1.15f / (0.55f + 0.45f * supplyRail));
        float out = dcBlocker.process(voiced * makeup);
        if (!std::isfinite(out)) return 0.0f;
        return std::clamp(out, -8.0f, 8.0f);
    }

private:
    float sr = 44100.0f;
    float invSr = 1.0f / 44100.0f;
    float q2EmitterFeedback = 0.0f;
    float couplingCapState = 0.0f;
    float envState = 0.0f;

    DSPCommon::OnePoleTPT inputCouplingHPF;
    DSPCommon::OnePoleTPT emitterBypassHPF;
    DSPCommon::CytomicSVF pickupLoadSVF;
    DSPCommon::CytomicSVF midContourSVF;
    DSPCommon::OnePoleTPT postCabRollLPF;
    DSPCommon::Oversampler4x oversampler;
    DSPCommon::DCBlocker dcBlocker;
};

using VelcroFace = FuzzFace;

} // namespace TbdAudio::KlangStomp
