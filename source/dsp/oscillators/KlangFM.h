#pragma once

#include "SoundEngine.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace TbdAudio::Oscillators {

/**
 * @brief KlangFM (also aliased as KlangTribeR) — 4-Operator Linear Phase Modulation &
 *        Korg ER-1 Cyclic Ring-Modulated Noise Percussion Synthesizer Engine.
 *
 * Implements 8 Algorithms:
 *   0: Cascade           4 -> 3 -> 2 -> 1
 *   1: Branched          (4 + 3) -> 2 -> 1
 *   2: Dual Pair         (4 -> 3) + (2 -> 1)
 *   3: Y-Stack           4 -> (3 + 2) -> 1
 *   4: 3-Mod Parallel    (4 + 3 + 2) -> 1
 *   5: Organ / Additive  1 + 2 + 3 + 4
 *   6: Cross-Feedback Percussion (Metallic Bell / Snare / Hat matrix)
 *   7: KlangTribe R      Cyclic Ring-Modulated Noise [Noise * osc(f_mod) FM/AM into Carrier]
 *
 * 8 Performance Knobs (params8[0..7]):
 *   0: ALGORITHM         (Selects 0..7 topology)
 *   1: MOD DEPTH / INDEX (0..1 phase modulation index)
 *   2: RATIO / SPEED     (Harmonic C:M ratios & ER-1 Mod Speed 0.1 Hz..5 kHz)
 *   3: FEEDBACK          (Operator 4 self-feedback & cyclic ring noise intensity)
 *   4: MOD DECAY         (Taper::profileA_Decay 10ms..10s modulator envelope)
 *   5: CARRIER DECAY     (Taper::profileA_Decay 10ms..10s carrier amp envelope)
 *   6: DETUNE / INHARM   (Operator frequency spread / metallic detune)
 *   7: WAVE / COLOR      (Operator waveform morph: Sine -> Half-Sine -> TX Camel -> Triangle)
 *
 * Real-Time Invariants:
 *   - Zero heap allocations.
 *   - Zero CRT transcendental calls (uses FastMath::fastSinNorm, fastPow2, fastExp, fastTanh).
 */
class KlangFM final : public SoundEngine8 {
public:
    KlangFM() noexcept {
        // Default musical patch: warm FM bell / ER-1 ready
        setParam(0, 0.0f);  // Algorithm 0 (Cascade)
        setParam(1, 0.45f); // Mod Depth
        setParam(2, 0.35f); // Ratio / Speed
        setParam(3, 0.20f); // Feedback
        setParam(4, 0.45f); // Mod Decay
        setParam(5, 0.65f); // Carrier Decay
        setParam(6, 0.15f); // Detune / Inharmonic
        setParam(7, 0.10f); // Wave / Color
        reset();
    }

    ~KlangFM() override = default;

    void reset() noexcept override {
        for (int i = 0; i < 4; ++i) {
            phase_[i] = 0.0f;
            prevOp_[i] = 0.0f;
        }
        modPhase_ = 0.0f;
        modEnv_ = 1.0f;
        carEnv_ = 1.0f;
        pitchEnv_ = 1.0f;
        autoTriggered_ = false;
        noise_.reset(0x2468ACE1u);
        dcL_.reset();
        dcR_.reset();
    }

protected:
    void onPrepare(float sampleRate) noexcept override {
        dcL_.init(sampleRate);
        dcR_.init(sampleRate);
        reset();
    }

    void onTrigger(float velocity) noexcept override {
        const float vel = std::clamp(velocity, 0.05f, 1.0f);
        modEnv_ = vel;
        carEnv_ = 0.25f + 0.75f * vel;
        pitchEnv_ = vel;
        autoTriggered_ = true;
    }

public:
    void renderSampleStereo(float baseFreqHz, float& outL, float& outR) noexcept override {
        if (!autoTriggered_) {
            onTrigger(velocity_);
        }

        const float sr = std::max(sampleRate_, 8000.0f);
        const float invSr = invSr_;
        const float nyquistLimit = 0.46f * sr;

        // 0: Algorithm [0..7]
        const int algo = std::clamp(static_cast<int>(params8[0] * 7.99f), 0, 7);

        // 1: Mod Depth / Index (quadratic perceptual curve up to 2.5 cycles = 5*pi rad)
        const float depthKnob = std::clamp(params8[1], 0.0f, 1.0f);
        const float modIndex = depthKnob * depthKnob * 2.2f;

        // 2: Ratio / Speed
        const float ratioKnob = std::clamp(params8[2], 0.0f, 1.0f);
        static constexpr float kHarmonicRatios[16] = {
            0.5f, 0.75f, 1.0f, 1.25f, 1.5f, 2.0f, 2.5f, 3.0f,
            3.5f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 12.0f
        };
        const int ratioIdx = std::clamp(static_cast<int>(ratioKnob * 15.99f), 0, 15);
        const float primaryRatio = kHarmonicRatios[ratioIdx];

        // 3: Feedback & Ring Noise Intensity
        const float fbKnob = std::clamp(params8[3], 0.0f, 1.0f);
        const float op4Feedback = fbKnob * fbKnob * 0.42f;

        // 4 & 5: Modulator and Carrier Envelopes (10ms..10s via Profile A)
        const float modDecaySec = Taper::profileA_Decay(params8[4], 0.010f, 10.0f);
        const float carDecaySec = Taper::profileA_Decay(params8[5], 0.010f, 10.0f);

        const float modCoef = FastMath::fastExp(-invSr / std::max(modDecaySec, 0.001f));
        const float carCoef = FastMath::fastExp(-invSr / std::max(carDecaySec, 0.001f));
        const float pitchCoef = FastMath::fastExp(-invSr / 0.018f);

        // Maintain small floor when Carrier Decay knob is high for continuous drone/test capability
        const float sustainFloor = (params8[5] > 0.92f) ? 0.35f : 0.025f;
        modEnv_ = modEnv_ * modCoef + (1.0f - modCoef) * (sustainFloor * 0.5f);
        carEnv_ = sustainFloor + (carEnv_ - sustainFloor) * carCoef;
        pitchEnv_ *= pitchCoef;

        // 6: Detune / Inharmonicity
        const float detuneKnob = std::clamp(params8[6], 0.0f, 1.0f);
        const float detuneQuad = detuneKnob * detuneKnob;

        // 7: Wave / Color [0..1]
        const float waveColor = std::clamp(params8[7], 0.0f, 1.0f);

        const float f0 = std::clamp(baseFreqHz, 12.0f, nyquistLimit);

        float sigL = 0.0f;
        float sigR = 0.0f;

        if (algo == 7) {
            // =========================================================================
            // Algorithm 7: OSC-01 KlangTribe R (Korg ER-1 Cyclic Ring-Modulated Noise)
            // =========================================================================
            const float modSpeedHz = std::min(Taper::profileC_ModSpeed(ratioKnob, 0.1f, 5000.0f), nyquistLimit);
            modPhase_ = PolyBLEP::wrapPhase(modPhase_ + modSpeedHz * invSr);
            const float cycSin = FastMath::fastSinNorm(modPhase_);

            const float white = noise_.nextWhite();
            const float pink = noise_.nextPink();
            const float noiseMix = white + fbKnob * (pink - white);

            // Cyclic Ring-Modulated Noise: Noise * sin(2*pi*f_mod*t) blended with pure cyclic sine
            const float ringNoise = cycSin * ((1.0f - fbKnob * 0.85f) + fbKnob * 1.35f * noiseMix);

            // Pitch transient punch (ER-1 style pitch bend envelope)
            const float punchMult = 1.0f + detuneQuad * 3.0f * pitchEnv_;
            const float carrierHz = std::min(f0 * punchMult, nyquistLimit);

            phase_[0] = PolyBLEP::wrapPhase(phase_[0] + carrierHz * invSr);
            // Sub/detuned stereo twin in phase_[1]
            const float stereoSpreadHz = std::min(carrierHz * (1.0f + 0.015f * detuneKnob), nyquistLimit);
            phase_[1] = PolyBLEP::wrapPhase(phase_[1] + stereoSpreadHz * invSr);

            const float fmMod = ringNoise * modIndex * (0.35f + 0.65f * modEnv_);
            const float carL = evalOperatorWave(phase_[0] + fmMod, waveColor);
            const float carR = evalOperatorWave(phase_[1] - fmMod * 0.85f, waveColor);

            // Combine FM carrier with direct AM ring-modulated noise burst
            const float amBurst = ringNoise * fbKnob * 0.45f * modEnv_;
            sigL = (carL + amBurst) * carEnv_;
            sigR = (carR - amBurst * 0.7f) * carEnv_;
        } else {
            // =========================================================================
            // Algorithms 0..6: 4-Operator Linear Phase Modulation
            // =========================================================================
            const float r1 = 1.0f;
            const float r2 = primaryRatio + detuneQuad * 0.4142135f;
            const float r3 = (primaryRatio * 1.5f) + detuneQuad * 1.7320508f;
            const float r4 = (primaryRatio * 2.0f) + detuneQuad * 2.4142135f;

            const float freqs[4] = {
                std::min(f0 * r1, nyquistLimit),
                std::min(f0 * (r2 + 0.003f * detuneKnob), nyquistLimit),
                std::min(f0 * (r3 - 0.004f * detuneKnob), nyquistLimit),
                std::min(f0 * (r4 + 0.006f * detuneKnob), nyquistLimit)
            };

            for (int i = 0; i < 4; ++i) {
                phase_[i] = PolyBLEP::wrapPhase(phase_[i] + freqs[i] * invSr);
            }

            const float envModIdx = modIndex * (0.15f + 0.85f * modEnv_);
            const float fb4 = op4Feedback * 0.5f * (prevOp_[3] + prevOp_[2] * (algo == 6 ? 0.6f : 0.0f));

            const float op4 = evalOperatorWave(phase_[3] + fb4, waveColor);
            prevOp_[3] = op4;

            float op3 = 0.0f;
            float op2 = 0.0f;
            float op1 = 0.0f;

            switch (algo) {
                case 0: { // 0: Cascade 4 -> 3 -> 2 -> 1
                    op3 = evalOperatorWave(phase_[2] + op4 * envModIdx * 0.85f, waveColor);
                    op2 = evalOperatorWave(phase_[1] + op3 * envModIdx * 0.95f, waveColor);
                    op1 = evalOperatorWave(phase_[0] + op2 * envModIdx, waveColor);
                    sigL = op1;
                    sigR = evalOperatorWave(phase_[0] + op2 * envModIdx * 0.92f + 0.02f * op3, waveColor);
                    break;
                }
                case 1: { // 1: Branched (4 + 3) -> 2 -> 1
                    op3 = evalOperatorWave(phase_[2] + prevOp_[2] * op4Feedback * 0.3f, waveColor);
                    op2 = evalOperatorWave(phase_[1] + (op4 + op3) * 0.5f * envModIdx, waveColor);
                    op1 = evalOperatorWave(phase_[0] + op2 * envModIdx, waveColor);
                    sigL = op1;
                    sigR = op1 * 0.85f + op2 * 0.15f;
                    break;
                }
                case 2: { // 2: Dual Pair (4 -> 3) + (2 -> 1)
                    op3 = evalOperatorWave(phase_[2] + op4 * envModIdx, waveColor);
                    op2 = evalOperatorWave(phase_[1], waveColor);
                    op1 = evalOperatorWave(phase_[0] + op2 * envModIdx, waveColor);
                    sigL = 0.55f * op1 + 0.45f * op3 * modEnv_;
                    sigR = 0.55f * op1 - 0.45f * op3 * modEnv_;
                    break;
                }
                case 3: { // 3: Y-Stack 4 -> (3 + 2) -> 1
                    op3 = evalOperatorWave(phase_[2] + op4 * envModIdx * 0.8f, waveColor);
                    op2 = evalOperatorWave(phase_[1] + op4 * envModIdx * 0.8f, waveColor);
                    op1 = evalOperatorWave(phase_[0] + 0.5f * (op3 + op2) * envModIdx, waveColor);
                    sigL = op1 + 0.15f * op2;
                    sigR = op1 + 0.15f * op3;
                    break;
                }
                case 4: { // 4: 3-Mod Parallel (4 + 3 + 2) -> 1
                    op3 = evalOperatorWave(phase_[2], waveColor);
                    op2 = evalOperatorWave(phase_[1], waveColor);
                    const float modSum = (op4 + op3 + op2) * (1.0f / 3.0f) * envModIdx;
                    op1 = evalOperatorWave(phase_[0] + modSum, waveColor);
                    sigL = op1;
                    sigR = evalOperatorWave(phase_[0] + modSum * 0.94f, waveColor);
                    break;
                }
                case 5: { // 5: Organ / Additive 1 + 2 + 3 + 4
                    op3 = evalOperatorWave(phase_[2] + op4 * envModIdx * 0.15f, waveColor);
                    op2 = evalOperatorWave(phase_[1] + op3 * envModIdx * 0.15f, waveColor);
                    op1 = evalOperatorWave(phase_[0] + op2 * envModIdx * 0.15f, waveColor);
                    const float mixEnv = 0.35f + 0.65f * modEnv_;
                    sigL = 0.40f * op1 + 0.25f * op2 * mixEnv + 0.20f * op3 * mixEnv + 0.15f * op4 * mixEnv;
                    sigR = 0.40f * op1 + 0.20f * op2 * mixEnv + 0.25f * op3 * mixEnv - 0.15f * op4 * mixEnv;
                    break;
                }
                case 6:
                default: { // 6: Cross-Feedback Percussion (Metallic Bell / Cym / Snare)
                    const float n = noise_.nextWhite() * fbKnob * 0.35f * modEnv_;
                    op3 = evalOperatorWave(phase_[2] + (op4 + prevOp_[0] * op4Feedback) * envModIdx + n, waveColor);
                    op2 = evalOperatorWave(phase_[1] + (op3 - prevOp_[1] * op4Feedback) * envModIdx, waveColor);
                    op1 = evalOperatorWave(phase_[0] + (op2 + op4 * 0.5f) * envModIdx, waveColor);
                    sigL = (0.65f * op1 + 0.35f * op3) * (1.0f + 0.25f * pitchEnv_);
                    sigR = (0.65f * op1 + 0.35f * op2) * (1.0f + 0.25f * pitchEnv_);
                    break;
                }
            }

            prevOp_[2] = op3;
            prevOp_[1] = op2;
            prevOp_[0] = op1;

            sigL *= carEnv_;
            sigR *= carEnv_;
        }

        // DC block, gentle saturation, and finite clamping
        sigL = dcL_.process(FastMath::fastTanh(sigL * 1.15f));
        sigR = dcR_.process(FastMath::fastTanh(sigR * 1.15f));

        if (phaseInvert) {
            sigL = -sigL;
            sigR = -sigR;
        }

        outL = std::clamp(std::isfinite(sigL) ? sigL : 0.0f, -4.0f, 4.0f);
        outR = std::clamp(std::isfinite(sigR) ? sigR : 0.0f, -4.0f, 4.0f);
    }

private:
    /**
     * Evaluates Yamaha TX81Z / OPL-inspired continuous waveform morph across [0..1]:
     *   0.00 .. 0.33: Pure Sine -> Half-Rectified Sine (OPL Wave 1)
     *   0.33 .. 0.66: Half-Sine -> Double-Frequency Camel Sine (TX Wave 3)
     *   0.66 .. 1.00: Camel Sine -> Band-limited Triangle
     */
    [[nodiscard]] static inline float evalOperatorWave(float normPhase, float morph) noexcept {
        const float p = PolyBLEP::wrapPhase(normPhase);
        const float s1 = FastMath::fastSinNorm(p);
        if (morph <= 0.001f) {
            return s1;
        }

        // Half-sine (zero on second half-cycle, DC-centered)
        const float halfSin = (p < 0.5f) ? (s1 - 0.31830988f) * 1.35f : (-0.31830988f * 1.35f);

        if (morph < 0.333333f) {
            const float t = morph * 3.0f;
            return s1 + t * (halfSin - s1);
        }

        // TX Camel wave: abs(sin(2*pi*p)) * sign(sin) + 2nd harmonic
        const float s2 = FastMath::fastSinNorm(PolyBLEP::wrapPhase(p * 2.0f));
        const float camel = (p < 0.5f) ? std::abs(s2) : -std::abs(s2);

        if (morph < 0.666667f) {
            const float t = (morph - 0.333333f) * 3.0f;
            return halfSin + t * (camel - halfSin);
        }

        const float tri = PolyBLEP::triangle(p);
        const float t = (morph - 0.666667f) * 3.0f;
        return camel + t * (tri - camel);
    }

    float phase_[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float prevOp_[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float modPhase_ = 0.0f;
    float modEnv_ = 1.0f;
    float carEnv_ = 1.0f;
    float pitchEnv_ = 1.0f;
    bool autoTriggered_ = false;
    PolyBLEP::NoiseGenerator noise_;
    DSPCommon::DCBlocker dcL_;
    DSPCommon::DCBlocker dcR_;
};

using KlangTribeR = KlangFM;
using TheR1 = KlangFM;
using TheKlangR1 = KlangFM;

} // namespace TbdAudio::Oscillators
