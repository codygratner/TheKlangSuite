#pragma once

#if __has_include("../common/AudioEffect.h")
#include "../common/AudioEffect.h"
#else
#include "../../common/AudioEffect.h"
#endif

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace TbdAudio::Experimental {

/**
 * Bespoke Processor 5: KlangBode (Harald Bode Frequency Shifter & SSB Modulator)
 *
 * Implements a classic Harald Bode 1630 / Moog-style Single-Sideband (SSB) Frequency Shifter
 * using a Dual 90-Degree Polyphase Hilbert Transform All-Pass Network (4 cascaded 1st-order
 * all-pass dome sections per I/Q branch), quadrature carrier modulation for continuous
 * Lower Sideband (LSB) <-> Ring Mod (RM) <-> Upper Sideband (USB) morphing, antiphase
 * stereo dual-shift, and a static stereo feedback delay loop for barberpole pitch spirals.
 *
 * Real-Time Safety Invariants:
 * - Zero heap allocations (static constexpr kMaxDelay = 32768 per channel).
 * - Zero CRT transcendentals (uses FastMath::fastSinNorm, fastCosNorm, fastTanh, fastPow2, fastExp).
 * - Strict float literals in std::clamp and output clamping to [-4.0f, 4.0f].
 */
class KlangBode final : public AudioEffect8 {
public:
    static constexpr int kMaxDelay = 32768;
    static constexpr int kDelayMask = kMaxDelay - 1;

    static constexpr std::array<float, 4> cI = {
        0.69238780f, 0.93606543f, 0.98822952f, 0.99874885f
    };
    static constexpr std::array<float, 4> cQ = {
        0.40219212f, 0.85617109f, 0.97229095f, 0.99528848f
    };

    KlangBode() noexcept {
        // Default 8-knob voicing:
        // 0: SHIFT FREQ = 0.5 (0 Hz center)
        // 1: FINE TUNE = 0.5 (0 Hz center)
        // 2: SIDEBAND BLEND = 1.0 (Upper Sideband)
        // 3: FEEDBACK = 0.0 (0%)
        // 4: DELAY TIME = 0.25 (~162 ms spiral tap)
        // 5: STEREO DUAL = 0.5 (antiphase stereo spread)
        // 6: TONE / DAMP = 0.75 (warm high-cut feedback damping)
        // 7: MIX / ∅ = 0.0 (100% Wet in Profile M)
        setParam8(0, 0.5f);
        setParam8(1, 0.5f);
        setParam8(2, 1.0f);
        setParam8(3, 0.0f);
        setParam8(4, 0.25f);
        setParam8(5, 0.5f);
        setParam8(6, 0.75f);
        setParam8(7, 0.0f);
        reset();
    }

    ~KlangBode() override = default;

    void init(const BlockContext& ctx) override {
        prepare(ctx.sampleRate > 0.0 ? ctx.sampleRate : 48000.0, 512);
    }

    void prepare(double sampleRate, int /*maxBlockSize*/ = 512) noexcept {
        float sr = static_cast<float>(sampleRate);
        sr_ = std::clamp(sr, 8000.0f, 192000.0f);
        invSr_ = 1.0f / sr_;

        dcBlockL_.init(sr_);
        dcBlockR_.init(sr_);
        reset();
    }

    void reset() noexcept override {
        hilbertL_.reset();
        hilbertR_.reset();
        dampL_.reset();
        dampR_.reset();
        dcBlockL_.reset();
        dcBlockR_.reset();

        delayBufL_.fill(0.0f);
        delayBufR_.fill(0.0f);
        writePos_ = 0;

        phaseL_ = 0.0f;
        phaseR_ = 0.0f;
        fbStateL_ = 0.0f;
        fbStateR_ = 0.0f;

        smoothShiftHz_ = 0.0f;
        smoothDelaySamples_ = 1.0f;
        smoothBlend_ = params8[2];
        smoothFeedback_ = 0.0f;
    }

    void setParam8(int idx, float val) noexcept {
        setParam(idx, val);
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        if (buffer == nullptr || numSamples <= 0) {
            return;
        }
        FastMath::enableFTZDAZ();
        updateSampleRateFromContext(ctx);
        updateBlockTargets();

        float dryGain = 0.0f;
        float wetGain = 1.0f;
        Taper::profileM_TriZoneMix(params8[7], phaseInvert, dryGain, wetGain);

        for (int i = 0; i < numSamples; ++i) {
            stepSmoothers();

            const float dry = buffer[i];
            const float inL = dry + fbStateL_;

            float outI = 0.0f;
            float outQ = 0.0f;
            hilbertL_.process(inL, outI, outQ);

            const float cosCarrier = FastMath::fastCosNorm(phaseL_);
            const float sinCarrier = FastMath::fastSinNorm(phaseL_);

            // SSB Quadrature Modulation:
            // USB = I * cos - Q * sin
            // LSB = I * cos + Q * sin
            // RM  = USB + LSB = 2 * I * cos
            const float usb = outI * cosCarrier - outQ * sinCarrier;
            const float lsb = outI * cosCarrier + outQ * sinCarrier;
            const float wet = blendSidebands(lsb, usb, smoothBlend_);

            // Advance quadrature carrier phase in [0, 1)
            phaseL_ += smoothShiftHz_ * invSr_;
            phaseL_ -= std::floor(phaseL_);

            // Write shifted signal to spiral delay line and read delayed feedback
            delayBufL_[writePos_] = wet;
            const float delayed = readFractionalDelay(delayBufL_, writePos_, smoothDelaySamples_);
            writePos_ = (writePos_ + 1) & kDelayMask;

            const float damped = dampL_.processLP(dcBlockL_.process(delayed));
            fbStateL_ = FastMath::fastTanh(damped * smoothFeedback_);

            const float out = dry * dryGain + wet * wetGain;
            buffer[i] = std::clamp(out, -4.0f, 4.0f);
        }

        FastMath::sanitizeBuffer(buffer, numSamples);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& ctx) override {
        if (left == nullptr || right == nullptr || numSamples <= 0) {
            if (left != nullptr && numSamples > 0) {
                process(left, numSamples, ctx);
            }
            return;
        }
        FastMath::enableFTZDAZ();
        updateSampleRateFromContext(ctx);
        updateBlockTargets();

        float dryGain = 0.0f;
        float wetGain = 1.0f;
        Taper::profileM_TriZoneMix(params8[7], phaseInvert, dryGain, wetGain);

        const float stereoDual = std::clamp(params8[5], 0.0f, 1.0f);
        // Antiphase stereo dual shift:
        // stereoDual = 0.0 -> Right shifts same direction as Left (+df)
        // stereoDual = 1.0 -> Right shifts full antiphase (-df)
        const float rightShiftScale = 1.0f - 2.0f * stereoDual;

        for (int i = 0; i < numSamples; ++i) {
            stepSmoothers();

            const float dryL = left[i];
            const float dryR = right[i];

            const float inL = dryL + fbStateL_;
            const float inR = dryR + fbStateR_;

            float iL = 0.0f, qL = 0.0f;
            float iR = 0.0f, qR = 0.0f;
            hilbertL_.process(inL, iL, qL);
            hilbertR_.process(inR, iR, qR);

            const float cosL = FastMath::fastCosNorm(phaseL_);
            const float sinL = FastMath::fastSinNorm(phaseL_);
            const float usbL = iL * cosL - qL * sinL;
            const float lsbL = iL * cosL + qL * sinL;
            const float wetL = blendSidebands(lsbL, usbL, smoothBlend_);

            const float cosR = FastMath::fastCosNorm(phaseR_);
            const float sinR = FastMath::fastSinNorm(phaseR_);
            const float usbR = iR * cosR - qR * sinR;
            const float lsbR = iR * cosR + qR * sinR;
            const float wetR = blendSidebands(lsbR, usbR, smoothBlend_);

            const float shiftHzL = smoothShiftHz_;
            const float shiftHzR = smoothShiftHz_ * rightShiftScale;

            phaseL_ += shiftHzL * invSr_;
            phaseL_ -= std::floor(phaseL_);

            phaseR_ += shiftHzR * invSr_;
            phaseR_ -= std::floor(phaseR_);

            delayBufL_[writePos_] = wetL;
            delayBufR_[writePos_] = wetR;

            const float delL = readFractionalDelay(delayBufL_, writePos_, smoothDelaySamples_);
            const float delR = readFractionalDelay(delayBufR_, writePos_, smoothDelaySamples_);
            writePos_ = (writePos_ + 1) & kDelayMask;

            const float dampOutL = dampL_.processLP(dcBlockL_.process(delL));
            const float dampOutR = dampR_.processLP(dcBlockR_.process(delR));

            fbStateL_ = FastMath::fastTanh(dampOutL * smoothFeedback_);
            fbStateR_ = FastMath::fastTanh(dampOutR * smoothFeedback_);

            left[i]  = std::clamp(dryL * dryGain + wetL * wetGain, -4.0f, 4.0f);
            right[i] = std::clamp(dryR * dryGain + wetR * wetGain, -4.0f, 4.0f);
        }

        FastMath::sanitizeBuffer(left, numSamples);
        FastMath::sanitizeBuffer(right, numSamples);
    }

private:
    /**
     * Dual 90-Degree Polyphase Hilbert Transform All-Pass Network (Dome Filter)
     * 4 cascaded 1st-order all-pass sections per branch (I = 0 deg, Q = 90 deg).
     */
    struct HilbertNetwork {
        std::array<float, 4> xI{ 0.0f, 0.0f, 0.0f, 0.0f };
        std::array<float, 4> yI{ 0.0f, 0.0f, 0.0f, 0.0f };
        std::array<float, 4> xQ{ 0.0f, 0.0f, 0.0f, 0.0f };
        std::array<float, 4> yQ{ 0.0f, 0.0f, 0.0f, 0.0f };
        float z1Q = 0.0f;

        inline void reset() noexcept {
            xI.fill(0.0f);
            yI.fill(0.0f);
            xQ.fill(0.0f);
            yQ.fill(0.0f);
            z1Q = 0.0f;
        }

        inline void process(float in, float& outI, float& outQ) noexcept {
            // Branch I (0 deg reference phase path): 4 cascaded 1st-order all-pass filters
            float sigI = in;
            for (int s = 0; s < 4; ++s) {
                const float c = cI[s];
                const float y = c * (sigI + yI[s]) - xI[s];
                xI[s] = sigI;
                yI[s] = y;
                sigI = y;
            }

            // Branch Q (90 deg quadrature path): unit delay + 4 cascaded 1st-order all-pass filters
            float sigQ = z1Q;
            z1Q = in;
            for (int s = 0; s < 4; ++s) {
                const float c = cQ[s];
                const float y = c * (sigQ + yQ[s]) - xQ[s];
                xQ[s] = sigQ;
                yQ[s] = y;
                sigQ = y;
            }

            outI = sigI;
            outQ = sigQ;
        }
    };

    inline void updateSampleRateFromContext(const BlockContext& ctx) noexcept {
        if (ctx.sampleRate > 0.0) {
            const float newSr = std::clamp(static_cast<float>(ctx.sampleRate), 8000.0f, 192000.0f);
            if (std::abs(newSr - sr_) > 1.0f) {
                sr_ = newSr;
                invSr_ = 1.0f / sr_;
                dcBlockL_.init(sr_);
                dcBlockR_.init(sr_);
            }
        }
    }

    inline void updateBlockTargets() noexcept {
        // 0: SHIFT FREQ (-5000 Hz .. 0 Hz .. +5000 Hz via Profile B)
        const float coarseHz = Taper::profileB_Bipolar(params8[0]) * 5000.0f;

        // 1: FINE TUNE (-20 Hz .. +20 Hz barberpole phaser micro-shift)
        const float fineNorm = std::clamp(params8[1], 0.0f, 1.0f);
        const float fineHz = (2.0f * fineNorm - 1.0f) * 20.0f;

        targetShiftHz_ = std::clamp(coarseHz + fineHz, -5020.0f, 5020.0f);

        // 2: SIDEBAND BLEND (0.0 = LSB, 0.5 = Dual Ring Mod, 1.0 = USB)
        targetBlend_ = std::clamp(params8[2], 0.0f, 1.0f);

        // 3: FEEDBACK (0% .. 110% via Profile F Hyperbolic)
        targetFeedback_ = Taper::profileF_Hyperbolic(params8[3], 0.0f, 1.10f);

        // 4: DELAY TIME (0 ms .. 650 ms feedback spiral delay)
        const float delaySec = std::clamp(params8[4], 0.0f, 1.0f) * 0.650f;
        const float maxSamples = static_cast<float>(kMaxDelay - 4);
        targetDelaySamples_ = std::clamp(delaySec * sr_, 0.0f, maxSamples);

        // 6: TONE / DAMP (800 Hz .. 18 kHz feedback loop OnePoleTPT high-cut damping)
        const float dampCutoffHz = std::clamp(
            Taper::profileD_Cutoff(params8[6], 800.0f, 18000.0f),
            800.0f,
            std::min(18000.0f, 0.45f * sr_)
        );
        dampL_.setCutoff(dampCutoffHz, invSr_);
        dampR_.setCutoff(dampCutoffHz, invSr_);
    }

    inline void stepSmoothers() noexcept {
        constexpr float kSmoothCoeff = 0.004f;
        smoothShiftHz_ += kSmoothCoeff * (targetShiftHz_ - smoothShiftHz_);
        smoothBlend_ += kSmoothCoeff * (targetBlend_ - smoothBlend_);
        smoothFeedback_ += kSmoothCoeff * (targetFeedback_ - smoothFeedback_);
        smoothDelaySamples_ += 0.0015f * (targetDelaySamples_ - smoothDelaySamples_);
    }

    [[nodiscard]] static inline float blendSidebands(float lsb, float usb, float blend) noexcept {
        // 0.0 -> pure LSB
        // 0.5 -> Dual Ring Mod (RM = 0.5 * (USB + LSB) = I * cosCarrier)
        // 1.0 -> pure USB
        const float b = std::clamp(blend, 0.0f, 1.0f);
        const float rm = 0.5f * (usb + lsb);
        if (b <= 0.5f) {
            const float t = b * 2.0f;
            return lsb + t * (rm - lsb);
        } else {
            const float t = (b - 0.5f) * 2.0f;
            return rm + t * (usb - rm);
        }
    }

    [[nodiscard]] static inline float readFractionalDelay(
        const std::array<float, kMaxDelay>& buf,
        int writeIdx,
        float delaySamples) noexcept
    {
        if (delaySamples <= 0.001f) {
            return buf[writeIdx & kDelayMask];
        }
        const float readPos = static_cast<float>(writeIdx) - delaySamples;
        const int idxInt = static_cast<int>(std::floor(readPos));
        const float frac = readPos - static_cast<float>(idxInt);

        const int i0 = idxInt & kDelayMask;
        const int i1 = (idxInt + 1) & kDelayMask;
        return buf[i0] + frac * (buf[i1] - buf[i0]);
    }

    float sr_ = 48000.0f;
    float invSr_ = 1.0f / 48000.0f;

    HilbertNetwork hilbertL_;
    HilbertNetwork hilbertR_;

    DSPCommon::OnePoleTPT dampL_;
    DSPCommon::OnePoleTPT dampR_;
    DSPCommon::DCBlocker dcBlockL_;
    DSPCommon::DCBlocker dcBlockR_;

    std::array<float, kMaxDelay> delayBufL_{};
    std::array<float, kMaxDelay> delayBufR_{};
    int writePos_ = 0;

    float phaseL_ = 0.0f;
    float phaseR_ = 0.0f;
    float fbStateL_ = 0.0f;
    float fbStateR_ = 0.0f;

    float targetShiftHz_ = 0.0f;
    float smoothShiftHz_ = 0.0f;
    float targetBlend_ = 1.0f;
    float smoothBlend_ = 1.0f;
    float targetFeedback_ = 0.0f;
    float smoothFeedback_ = 0.0f;
    float targetDelaySamples_ = 1.0f;
    float smoothDelaySamples_ = 1.0f;
};

} // namespace TbdAudio::Experimental
