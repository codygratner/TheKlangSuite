#pragma once

#include "../common/AudioEffect.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace TbdAudio::Experimental {

/**
 * @brief KlangVortex — Bespoke Processor 8: Phase-Fold Chaos Resonator.
 *        Crosses a West Coast multi-stage Buchla recursive wavefolder with a multi-order
 *        All-Pass Filter (APF) phase dispersion network in a self-modulating feedback loop.
 *
 * 3 Routing Modes (selected via Knob 2: APF STAGES & ROUTING):
 *   - Mode A [ FOLD -> APF ]  (0.00 .. 0.33): Wavefolds first for dense harmonics, then phase-disperses into laser drops.
 *   - Mode B [ APF -> FOLD ]  (0.34 .. 0.66): Disperses phase first so different frequencies hit folder thresholds at staggered times.
 *   - Mode C [ CHAOS LOOP ]   (0.67 .. 1.00): Dual Recirculating Chaos Loop — both Folder and APF live inside the Cross-FM feedback loop!
 *
 * 8-Knob Parameter Topology:
 *   - Knob 0 (FOLDS)            : [0..1] 1 .. 8 West Coast Buchla recursive sine fold stages
 *   - Knob 1 (SMEAR FREQ)       : [0..1] Profile D Cutoff (20 Hz .. 20 kHz APF center pitch)
 *   - Knob 2 (STAGES & ROUTING) : [0..1] Selects Mode A / B / C & scales active APF stages (4 .. 16 poles)
 *   - Knob 3 (FEEDBACK)         : [0..1] -115% .. +115% Bipolar Profile F Hyperbolic feedback loop
 *   - Knob 4 (SYMMETRY / BIAS)  : [0..1] DC offset asymmetry generating rich even harmonics before folding
 *   - Knob 5 (CROSS-FM)         : [0..1] 0% .. 100% Audio-rate wavefolder output modulates APF cutoff frequency!
 *   - Knob 6 (DAMPING)          : [0..1] 1 kHz .. 20 kHz feedback loop HF damping filter
 *   - Knob 7 (MIX / ∅)          : [0..1] Profile M Tri-Zone Mix (0.0 = Wet, 0.5 = Dry, 1.0 = 50/50)
 */
class KlangVortex final : public AudioEffect8 {
public:
    KlangVortex() noexcept {
        setParam(0, 0.45f); // FOLDS (~4 stages)
        setParam(1, 0.40f); // SMEAR FREQ (~500 Hz)
        setParam(2, 0.80f); // Mode C (Dual Recirculating Chaos Loop) + 12 APF stages
        setParam(3, 0.72f); // FEEDBACK (+55%)
        setParam(4, 0.55f); // SYMMETRY / BIAS
        setParam(5, 0.35f); // CROSS-FM
        setParam(6, 0.75f); // DAMPING (~10 kHz)
        setParam(7, 0.0f);  // MIX = 100% Wet per Profile M
    }

    ~KlangVortex() override = default;

    void init(const BlockContext& ctx) override {
        prepare(static_cast<double>(ctx.sampleRate), 512);
    }

    void prepare(double sampleRate, int /*maxBlockSize*/ = 512) noexcept {
        sampleRate_ = (sampleRate > 8000.0) ? static_cast<float>(sampleRate) : 48000.0f;
        invSr_ = 1.0f / sampleRate_;
        reset();
        dcL_.init(sampleRate_);
        dcR_.init(sampleRate_);
    }

    void reset() noexcept override {
        for (int ch = 0; ch < 2; ++ch) {
            for (int s = 0; s < kMaxApfStages; ++s) {
                apfState_[ch][s] = 0.0f;
            }
            fbState_[ch] = 0.0f;
            loopLP_[ch].reset();
        }
        dcL_.reset();
        dcR_.reset();
    }

    inline void setParam8(int index, float value) noexcept {
        setParam(index, value);
    }

    [[nodiscard]] float processChannelSample(int ch, float dryIn) noexcept {
        const int c = (ch & 1);
        const float foldsNorm   = std::clamp(params8[0], 0.0f, 1.0f);
        const float freqNorm    = std::clamp(params8[1], 0.0f, 1.0f);
        const float routeNorm   = std::clamp(params8[2], 0.0f, 1.0f);
        const float fbNorm      = std::clamp(params8[3], 0.0f, 1.0f);
        const float symNorm     = std::clamp(params8[4], 0.0f, 1.0f);
        const float crossFmNorm = std::clamp(params8[5], 0.0f, 1.0f);
        const float dampNorm    = std::clamp(params8[6], 0.0f, 1.0f);
        const float mixNorm     = std::clamp(params8[7], 0.0f, 1.0f);

        float dryGain = 0.0f, wetGain = 1.0f;
        Taper::profileM_TriZoneMix(mixNorm, phaseInvert, dryGain, wetGain);
        if (wetGain == 0.0f) {
            return dryIn * dryGain;
        }

        // Determine Routing Mode (0 = Fold->APF, 1 = APF->Fold, 2 = Recirculating Chaos Loop)
        int mode = 0;
        float subStageNorm = routeNorm * 3.0f;
        if (routeNorm >= 0.666f) {
            mode = 2;
            subStageNorm = (routeNorm - 0.666f) * 3.0f;
        } else if (routeNorm >= 0.333f) {
            mode = 1;
            subStageNorm = (routeNorm - 0.333f) * 3.0f;
        }
        const int activeStages = std::clamp(4 + static_cast<int>(subStageNorm * 12.0f + 0.5f), 4, kMaxApfStages);

        // Bipolar feedback in [-1.15, +1.15] using Profile B + Profile F scaling
        const float bipolarFb = (fbNorm - 0.5f) * 2.0f;
        const float fbMag = Taper::profileF_Hyperbolic(std::abs(bipolarFb), 0.0f, 1.15f);
        const float signedFb = (bipolarFb >= 0.0f) ? fbMag : -fbMag;

        // Loop HF Damping
        const float dampHz = Taper::profileD_Cutoff(dampNorm, 1000.0f, 19000.0f);
        loopLP_[c].setCutoff(dampHz, invSr_);

        // Slight stereo offset on right channel APF center frequency for 3D neurofunk width
        const float baseFreqHz = Taper::profileD_Cutoff(freqNorm, 25.0f, 16000.0f) * ((c == 1) ? 1.07f : 1.0f);
        const float symBias = (symNorm - 0.5f) * 0.65f;
        const float foldDrive = 1.0f + foldsNorm * 6.5f;

        auto applyBuchlaFold = [&](float x) noexcept -> float {
            const float biased = x * foldDrive + symBias;
            // Recursive Buchla 259 style sine-folding + soft saturation blend
            float norm1 = biased * 0.25f;
            norm1 -= static_cast<float>(static_cast<int>(norm1));
            const float s1 = FastMath::fastSinNorm(norm1);
            float norm2 = (biased * 0.5f + 0.35f * s1);
            norm2 -= static_cast<float>(static_cast<int>(norm2));
            const float s2 = FastMath::fastSinNorm(norm2);
            const float folded = (1.0f - 0.8f * foldsNorm) * FastMath::fastTanh(biased)
                               + (0.8f * foldsNorm) * (0.65f * s1 + 0.35f * s2);
            return folded;
        };

        auto applyApfCascade = [&](float x, float fmModSignal) noexcept -> float {
            const float modOctaves = fmModSignal * crossFmNorm * 3.0f;
            const float modHz = std::clamp(baseFreqHz * FastMath::fastPow2(modOctaves), 20.0f, sampleRate_ * 0.46f);
            const float g = DSPCommon::fastTanPiNorm(modHz * invSr_);
            const float apfCoeff = (g - 1.0f) / (g + 1.0f);

            float v = x;
            for (int s = 0; s < activeStages; ++s) {
                const float prev = apfState_[c][s];
                const float y = apfCoeff * v + prev;
                apfState_[c][s] = v - apfCoeff * y;
                v = y;
            }
            return v;
        };

        const float loopIn = dryIn + fbState_[c] * signedFb;
        float wetOut = 0.0f;

        if (mode == 0) {
            // Mode A: [ FOLD -> APF ]
            const float folded = applyBuchlaFold(loopIn);
            wetOut = applyApfCascade(folded, folded);
            fbState_[c] = FastMath::fastTanh(loopLP_[c].processLP(wetOut));
        } else if (mode == 1) {
            // Mode B: [ APF -> FOLD ]
            const float smeared = applyApfCascade(loopIn, fbState_[c]);
            wetOut = applyBuchlaFold(smeared);
            fbState_[c] = FastMath::fastTanh(loopLP_[c].processLP(wetOut));
        } else {
            // Mode C: [ DUAL RECIRCULATING CHAOS LOOP ]
            const float folded = applyBuchlaFold(loopIn);
            const float smeared = applyApfCascade(folded, folded - fbState_[c]);
            wetOut = FastMath::fastTanh(0.6f * smeared + 0.4f * applyBuchlaFold(smeared * 0.7f));
            fbState_[c] = FastMath::fastTanh(loopLP_[c].processLP(wetOut));
        }

        const float dcClean = (c == 0) ? dcL_.process(wetOut) : dcR_.process(wetOut);
        return std::clamp(dryIn * dryGain + dcClean * wetGain, -4.0f, 4.0f);
    }

    void process(float* buffer, int numSamples, BlockContext& /*ctx*/) override {
        FastMath::enableFTZDAZ();
        if (!buffer || numSamples <= 0) return;
        for (int i = 0; i < numSamples; ++i) {
            buffer[i] = processChannelSample(0, buffer[i]);
        }
        FastMath::sanitizeBuffer(buffer, numSamples);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& /*ctx*/) override {
        FastMath::enableFTZDAZ();
        if (!left || numSamples <= 0) return;
        if (!right || right == left) {
            for (int i = 0; i < numSamples; ++i) {
                left[i] = processChannelSample(0, left[i]);
            }
            FastMath::sanitizeBuffer(left, numSamples);
            return;
        }
        for (int i = 0; i < numSamples; ++i) {
            left[i]  = processChannelSample(0, left[i]);
            right[i] = processChannelSample(1, right[i]);
        }
        FastMath::sanitizeBuffer(left, numSamples);
        FastMath::sanitizeBuffer(right, numSamples);
    }

private:
    static constexpr int kMaxApfStages = 16;

    float sampleRate_ = 48000.0f;
    float invSr_ = 1.0f / 48000.0f;

    float apfState_[2][kMaxApfStages]{};
    float fbState_[2]{};
    DSPCommon::OnePoleTPT loopLP_[2]{};
    DSPCommon::DCBlocker dcL_{};
    DSPCommon::DCBlocker dcR_{};
};

} // namespace TbdAudio::Experimental
