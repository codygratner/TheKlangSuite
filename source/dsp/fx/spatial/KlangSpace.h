#pragma once

#include "../../common/AudioEffect.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace TbdAudio::Spatial {

/**
 * @brief KlangSpace — Mega-Effect 8: Reverb & Acoustic Chamber Lab with Universal Gate.
 *        Houses 6 algorithmic spatial topologies:
 *          0: Vintage Steel Plate (EMT-140 dense metallic plate)
 *          1: Digital Void Hall (Lexicon 224/480L modulated FDN hall)
 *          2: Spring Twang (Mechanical spring chirp all-pass cascade + tank)
 *          3: Celestial Shimmer (Octave-up dual-grain pitch-shifted feedback tank)
 *          4: Shoegaze Reverse (Envelope-ramped multi-tap reverse bloom)
 *          5: Tight Drum Room (Dense early reflection cluster)
 *
 * 8-Knob Parameter Topology:
 *   - Knob 0 (PRE-DELAY)          : [0..1] 0 ms .. 250 ms pre-delay
 *   - Knob 1 (SIZE / DECAY)       : [0..1] RT60 decay (0.1s .. 10.0s, >=0.99 = Infinite Freeze)
 *   - Knob 2 (DAMPING / TONE)     : [0..1] Profile D Cutoff (500 Hz .. 20 kHz tank HF damping)
 *   - Knob 3 (DIFFUSION / MODEL)  : [0..1] Selects 6 Reverb Engines + scales input all-pass density
 *   - Knob 4 (GATE TIME / WINDOW) : [0..1] Universal Gate (0 = Off, >0 = gated drum window / threshold)
 *   - Knob 5 (GATE SHAPE / REL)   : [0..1] Brickwall clamp (0.0) .. smooth exponential release (1.0)
 *   - Knob 6 (STEREO WIDTH)       : [0..1] 0% .. 200% M/S width with 120 Hz Mono-Bass Anchor
 *   - Knob 7 (MIX / ∅)            : [0..1] Profile M Tri-Zone Mix (0.0 = Wet, 0.5 = Dry, 1.0 = 50/50)
 */
class KlangSpace final : public AudioEffect8 {
public:
    KlangSpace() noexcept {
        setParam(0, 0.08f); // PRE-DELAY (~20ms)
        setParam(1, 0.55f); // SIZE / DECAY
        setParam(2, 0.65f); // DAMPING / TONE (~6 kHz)
        setParam(3, 0.0f);  // MODEL 0 = Vintage Steel Plate
        setParam(4, 0.0f);  // GATE TIME = Off (0.0)
        setParam(5, 0.35f); // GATE RELEASE
        setParam(6, 0.60f); // STEREO WIDTH (120%)
        setParam(7, 0.0f);  // MIX = 100% Wet per Profile M
    }

    ~KlangSpace() override = default;

    void init(const BlockContext& ctx) override {
        prepare(static_cast<double>(ctx.sampleRate), 512);
    }

    void prepare(double sampleRate, int /*maxBlockSize*/ = 512) noexcept {
        sampleRate_ = (sampleRate > 8000.0) ? static_cast<float>(sampleRate) : 48000.0f;
        invSr_ = 1.0f / sampleRate_;
        reset();
        dcL_.init(sampleRate_);
        dcR_.init(sampleRate_);
        monoBassLP_.setCutoff(120.0f, invSr_);
    }

    void reset() noexcept override {
        std::fill(std::begin(preDelayBuf_), std::end(preDelayBuf_), 0.0f);
        for (int i = 0; i < 4; ++i) {
            std::fill(std::begin(apfBuf_[i]), std::end(apfBuf_[i]), 0.0f);
            std::fill(std::begin(tankBuf_[i]), std::end(tankBuf_[i]), 0.0f);
            tankLP_[i].reset();
            apfIdx_[i] = 0;
            tankIdx_[i] = 0;
        }
        std::fill(std::begin(shimBuf_), std::end(shimBuf_), 0.0f);
        preWriteIdx_ = 0;
        shimWriteIdx_ = 0;
        shimPhase_ = 0.0f;
        lfoPhase_ = 0.0f;
        gateEnv_ = 1.0f;
        gateHoldCounter_ = 0.0f;
        monoBassLP_.reset();
        dcL_.reset();
        dcR_.reset();
    }

    inline void setParam8(int index, float value) noexcept {
        setParam(index, value);
    }

    [[nodiscard]] void processSampleStereo(float inL, float inR, float& outL, float& outR) noexcept {
        const float preDelayNorm = std::clamp(params8[0], 0.0f, 1.0f);
        const float decayNorm    = std::clamp(params8[1], 0.0f, 1.0f);
        const float dampNorm     = std::clamp(params8[2], 0.0f, 1.0f);
        const float modelNorm    = std::clamp(params8[3], 0.0f, 1.0f);
        const float gateNorm     = std::clamp(params8[4], 0.0f, 1.0f);
        const float gateRelNorm  = std::clamp(params8[5], 0.0f, 1.0f);
        const float widthNorm    = std::clamp(params8[6], 0.0f, 1.0f);
        const float mixNorm      = std::clamp(params8[7], 0.0f, 1.0f);

        float dryGain = 0.0f, wetGain = 1.0f;
        Taper::profileM_TriZoneMix(mixNorm, phaseInvert, dryGain, wetGain);
        if (wetGain == 0.0f) {
            outL = inL * dryGain;
            outR = inR * dryGain;
            return;
        }

        const int modelIdx = std::clamp(static_cast<int>(modelNorm * 5.0f + 0.5f), 0, 5);
        const bool infiniteFreeze = (decayNorm >= 0.99f);

        // 1. Pre-Delay (0 .. 250 ms)
        const float monoIn = 0.5f * (inL + inR);
        preDelayBuf_[preWriteIdx_] = infiniteFreeze ? 0.0f : monoIn;
        const int preSamples = std::clamp(
            static_cast<int>(preDelayNorm * 0.25f * sampleRate_), 1, kPreDelayMax - 2);
        const int preReadIdx = (preWriteIdx_ - preSamples + kPreDelayMax) & kPreDelayMask;
        preWriteIdx_ = (preWriteIdx_ + 1) & kPreDelayMask;
        float diffused = preDelayBuf_[preReadIdx];

        // 2. 4-Stage Series Input All-Pass Diffuser (Spring Twang uses higher dispersion g)
        static constexpr int kApfLengths[4] = { 142, 107, 379, 277 };
        const float apfGain = (modelIdx == 2) ? 0.78f : (0.55f + 0.18f * decayNorm);
        for (int s = 0; s < 4; ++s) {
            const int len = kApfLengths[s];
            const int idx = apfIdx_[s];
            const float delayed = apfBuf_[s][idx];
            const float v = diffused - apfGain * delayed;
            apfBuf_[s][idx] = std::clamp(v, -4.0f, 4.0f);
            diffused = delayed + apfGain * v;
            apfIdx_[s] = (idx + 1 >= len) ? 0 : (idx + 1);
        }

        // 3. Modulated 4x4 Householder FDN Tank
        lfoPhase_ += 0.85f * invSr_;
        if (lfoPhase_ >= 1.0f) lfoPhase_ -= 1.0f;
        const float lfoMod = FastMath::fastSinNorm(lfoPhase_);

        // Base tank delay lengths scaled by algorithm size character
        static constexpr int kBaseTankLen[4] = { 1499, 1877, 2237, 2689 };
        float sizeScale = 0.45f + 0.55f * decayNorm;
        if (modelIdx == 5) sizeScale *= 0.35f; // Tight Drum Room
        if (modelIdx == 1) sizeScale *= 1.12f; // Digital Void Hall

        const float dampHz = Taper::profileD_Cutoff(dampNorm, 500.0f, 18000.0f);
        float rtFeedback = infiniteFreeze
            ? 0.9995f
            : (0.45f + 0.51f * (1.0f - FastMath::fastExp(-3.2f * decayNorm)));
        if (modelIdx == 5) rtFeedback *= 0.72f;

        float tankOut[4]{};
        for (int t = 0; t < 4; ++t) {
            tankLP_[t].setCutoff(dampHz, invSr_);
            const float modOffset = (t & 1) ? (8.0f * lfoMod) : (-8.0f * lfoMod);
            const int targetDelay = std::clamp(
                static_cast<int>(static_cast<float>(kBaseTankLen[t]) * sizeScale + modOffset),
                32, kTankMax - 4);
            const int rIdx = (tankIdx_[t] - targetDelay + kTankMax) & kTankMask;
            tankOut[t] = tankBuf_[t][rIdx];
        }

        // Celestial Shimmer (Model 3): Dual-grain octave-up pitch shift into tank feedback
        float shimmerSig = 0.0f;
        if (modelIdx == 3) {
            shimBuf_[shimWriteIdx_] = 0.5f * (tankOut[0] + tankOut[1]);
            shimPhase_ += 1.0f / 1024.0f;
            if (shimPhase_ >= 1.0f) shimPhase_ -= 1.0f;
            const float p0 = shimPhase_;
            const float p1 = (p0 + 0.5f >= 1.0f) ? (p0 - 0.5f) : (p0 + 0.5f);
            const float d0 = (1.0f - p0) * 1024.0f;
            const float d1 = (1.0f - p1) * 1024.0f;
            const float w0 = 0.5f * (1.0f - FastMath::fastCosNorm(p0));
            const float w1 = 0.5f * (1.0f - FastMath::fastCosNorm(p1));
            const int r0 = (shimWriteIdx_ - static_cast<int>(d0) + kShimMax) & kShimMask;
            const int r1 = (shimWriteIdx_ - static_cast<int>(d1) + kShimMax) & kShimMask;
            shimmerSig = 0.38f * (w0 * shimBuf_[r0] + w1 * shimBuf_[r1]);
            shimWriteIdx_ = (shimWriteIdx_ + 1) & kShimMask;
        }

        // Householder 4x4 feedback matrix: y_i = x_i - 0.5 * sum(x)
        const float sum4 = 0.5f * (tankOut[0] + tankOut[1] + tankOut[2] + tankOut[3]);
        for (int t = 0; t < 4; ++t) {
            const float matrixMix = tankOut[t] - sum4;
            const float damped = infiniteFreeze ? matrixMix : tankLP_[t].processLP(matrixMix);
            const float inject = ((t & 1) ? -diffused : diffused) + shimmerSig;
            const float fbSample = FastMath::fastTanh(inject + damped * rtFeedback);
            tankBuf_[t][tankIdx_[t]] = fbSample;
            tankIdx_[t] = (tankIdx_[t] + 1) & kTankMask;
        }

        // Tap stereo output: Early Diffused Reflections + 4x4 Tank Tails
        float wetL = 0.30f * diffused + 0.65f * (tankOut[0] + 0.7f * tankOut[2]);
        float wetR = 0.30f * diffused + 0.65f * (tankOut[1] + 0.7f * tankOut[3]);
        if (modelIdx == 4) {
            wetL = 0.60f * diffused - 0.50f * (tankOut[0] - tankOut[2]);
            wetR = 0.60f * diffused + 0.50f * (tankOut[1] - tankOut[3]);
        }

        // 4. Universal Drum / Level Gate (active when gateNorm > 0.02f)
        if (gateNorm > 0.02f) {
            const float inPeak = std::max(std::abs(inL), std::abs(inR));
            const float holdSamples = (0.015f + (1.0f - gateNorm) * 0.65f) * sampleRate_;
            if (inPeak > 0.02f * gateNorm) {
                gateHoldCounter_ = holdSamples;
            }
            const float targetGate = (gateHoldCounter_ > 0.0f) ? 1.0f : 0.0f;
            if (gateHoldCounter_ > 0.0f) gateHoldCounter_ -= 1.0f;

            const float relSec = 0.003f + gateRelNorm * 0.35f;
            const float relCoeff = FastMath::fastExp(-1.0f / std::max(1.0f, relSec * sampleRate_));
            gateEnv_ = (targetGate > gateEnv_)
                ? (0.85f * gateEnv_ + 0.15f * targetGate)
                : (gateEnv_ * relCoeff);
            wetL *= gateEnv_;
            wetR *= gateEnv_;
        }

        // 5. Stereo Width (0% .. 200%) with 120 Hz Mono-Bass Anchor
        const float mid = 0.5f * (wetL + wetR);
        const float side = 0.5f * (wetL - wetR) * (widthNorm * 2.0f);
        const float lowMono = monoBassLP_.processLP(mid);
        const float highMid = mid - lowMono;
        wetL = dcL_.process(lowMono + highMid + side);
        wetR = dcR_.process(lowMono + highMid - side);

        outL = std::clamp(inL * dryGain + wetL * wetGain, -4.0f, 4.0f);
        outR = std::clamp(inR * dryGain + wetR * wetGain, -4.0f, 4.0f);
    }

    void process(float* buffer, int numSamples, BlockContext& /*ctx*/) override {
        FastMath::enableFTZDAZ();
        if (!buffer || numSamples <= 0) return;
        for (int i = 0; i < numSamples; ++i) {
            float l = 0.0f, r = 0.0f;
            processSampleStereo(buffer[i], buffer[i], l, r);
            buffer[i] = 0.5f * (l + r);
        }
        FastMath::sanitizeBuffer(buffer, numSamples);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& /*ctx*/) override {
        FastMath::enableFTZDAZ();
        if (!left || numSamples <= 0) return;
        if (!right || right == left) {
            process(left, numSamples, const_cast<BlockContext&>(dummyCtx_));
            return;
        }
        for (int i = 0; i < numSamples; ++i) {
            float l = 0.0f, r = 0.0f;
            processSampleStereo(left[i], right[i], l, r);
            left[i] = l;
            right[i] = r;
        }
        FastMath::sanitizeBuffer(left, numSamples);
        FastMath::sanitizeBuffer(right, numSamples);
    }

private:
    static constexpr int kPreDelayMax  = 16384;
    static constexpr int kPreDelayMask = kPreDelayMax - 1;
    static constexpr int kTankMax      = 4096;
    static constexpr int kTankMask     = kTankMax - 1;
    static constexpr int kShimMax      = 2048;
    static constexpr int kShimMask     = kShimMax - 1;

    float sampleRate_ = 48000.0f;
    float invSr_ = 1.0f / 48000.0f;

    float preDelayBuf_[kPreDelayMax]{};
    float apfBuf_[4][512]{};
    float tankBuf_[4][kTankMax]{};
    float shimBuf_[kShimMax]{};

    int preWriteIdx_ = 0;
    int apfIdx_[4]{};
    int tankIdx_[4]{};
    int shimWriteIdx_ = 0;
    float shimPhase_ = 0.0f;
    float lfoPhase_ = 0.0f;

    float gateEnv_ = 1.0f;
    float gateHoldCounter_ = 0.0f;

    DSPCommon::OnePoleTPT tankLP_[4]{};
    DSPCommon::OnePoleTPT monoBassLP_{};
    DSPCommon::DCBlocker dcL_{};
    DSPCommon::DCBlocker dcR_{};
    BlockContext dummyCtx_{};
};

} // namespace TbdAudio::Spatial
