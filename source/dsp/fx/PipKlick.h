#pragma once

#include "../common/AudioEffect.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace TbdAudio::FX {

/**
 * @brief PipKlick (`KlangFallout`) — WMD Geiger Counter & ShaperTable 256-Wavetable
 *        Amplitude Waveshaper, Bit-Mask Glitch & Sample Rate Decimator.
 *
 * Architecture:
 *   - Input Pre-Gain (-12 dB .. +36 dB via Profile E) driving instantaneous audio amplitude
 *     directly into a procedural 256-curve mathematical transfer function wavetable across 4 banks:
 *       Bank 0 (0..63):   Chebyshev Harmonic Polynomials (T2 .. T13)
 *       Bank 1 (64..127): West Coast Sine / Serge Multi-Stage Wavefolders
 *       Bank 2 (128..191): Geiger Counter Bitwise XOR / AND / Ring Glitch Curves
 *       Bank 3 (192..255): Quantized Staircase, Dead-Band Crossover & Rectifier Curves
 *   - Continuous wavetable morph (`MORPH / SCAN`) between adjacent curves.
 *   - Hardware Bit-Depth Quantizer (1..16 bits) + Bit-Mask Permutation & Zero-Order Hold
 *     Sample Rate Decimator (200 Hz .. 48 kHz).
 *   - Non-linear Slew Rate Limiter (`SLEW / SMOOTH`) & Post-Tone Filter (`MASK / TONE`).
 *
 * 8 Performance Knobs (params8[0..7]):
 *   [0] PRE-GAIN / DRIVE (-12 dB .. +36 dB input drive)
 *   [1] TABLE / WAVE     (Selects base transfer curve across 4 banks / 256 tables)
 *   [2] MORPH / SCAN     (Smooth interpolation scan across neighboring wavetable curves)
 *   [3] BIT DEPTH        (16 bits down to 1-bit comparator scream)
 *   [4] SAMPLE RATE      (48 kHz down to 200 Hz sample-and-hold decimation)
 *   [5] MASK / TONE      (Post-shaper CytomicSVF low-pass / bandpass tone sculpting)
 *   [6] SLEW / SMOOTH    (Non-linear dv/dt slew rate limiter rounding harsh digital spikes)
 *   [7] MIX / ∅          (Profile M Tri-Zone Mix: 0.0 = 100% Wet, 0.5 = 100% Dry, 1.0 = 50/50)
 */
class PipKlick final : public AudioEffect8 {
public:
    PipKlick() noexcept {
        setParam(0, 0.45f); // Pre-Gain
        setParam(1, 0.25f); // Table / Wave
        setParam(2, 0.30f); // Morph / Scan
        setParam(3, 0.20f); // Bit Depth (~12-bit)
        setParam(4, 0.15f); // Sample Rate
        setParam(5, 0.70f); // Mask / Tone
        setParam(6, 0.20f); // Slew / Smooth
        setParam(7, 0.00f); // Mix (100% Wet default per Profile M)
        reset();
    }

    ~PipKlick() override = default;

    void init(const BlockContext& ctx) override {
        prepare(ctx.sampleRate > 0.0f ? static_cast<double>(ctx.sampleRate) : 48000.0, 512);
    }

    void prepare(double sampleRate, int /*maxBlockSize*/ = 512) noexcept {
        sampleRate_ = std::clamp(static_cast<float>(sampleRate), 8000.0f, 192000.0f);
        invSr_ = 1.0f / sampleRate_;
        reset();
    }

    void reset() noexcept override {
        for (int c = 0; c < 2; ++c) {
            holdSample_[c] = 0.0f;
            slewState_[c] = 0.0f;
            toneFilter_[c].reset();
            dcBlocker_[c].init(sampleRate_);
            dcBlocker_[c].reset();
        }
        srrPhase_ = 0.0f;
    }

    void setParam8(int idx, float val) noexcept {
        setParam(idx, val);
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        if (!buffer || numSamples <= 0) return;
        FastMath::enableFTZDAZ();
        if (ctx.sampleRate > 0.0f && std::abs(ctx.sampleRate - sampleRate_) > 1.0f) {
            prepare(static_cast<double>(ctx.sampleRate), numSamples);
        }

        for (int i = 0; i < numSamples; ++i) {
            float L = buffer[i];
            float R = buffer[i];
            processSamplePair(L, R);
            buffer[i] = std::clamp(0.5f * (L + R), -4.0f, 4.0f);
        }
        FastMath::sanitizeBuffer(buffer, numSamples);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& ctx) override {
        if (!left || !right || numSamples <= 0) return;
        FastMath::enableFTZDAZ();
        if (ctx.sampleRate > 0.0f && std::abs(ctx.sampleRate - sampleRate_) > 1.0f) {
            prepare(static_cast<double>(ctx.sampleRate), numSamples);
        }

        for (int i = 0; i < numSamples; ++i) {
            float L = left[i];
            float R = right[i];
            processSamplePair(L, R);
            left[i] = std::clamp(L, -4.0f, 4.0f);
            right[i] = std::clamp(R, -4.0f, 4.0f);
        }
        FastMath::sanitizeBuffer(left, numSamples);
        FastMath::sanitizeBuffer(right, numSamples);
    }

private:
    /**
     * Evaluate one of the 256 transfer curves at normalized amplitude x in [-1.0, +1.0].
     */
    [[nodiscard]] static inline float evalTransferCurve(int tableIdx, float x) noexcept {
        const float u = std::clamp(x, -1.0f, 1.0f);
        const int idx = std::clamp(tableIdx, 0, 255);
        const int bank = idx >> 6;        // 0..3
        const int sub  = idx & 63;        // 0..63
        const float t  = static_cast<float>(sub) * (1.0f / 63.0f);

        switch (bank) {
            case 0: {
                // Bank 0: Chebyshev Polynomial Harmonics (T2, T3, T5, T7)
                const float u2 = u * u;
                const float t2 = 2.0f * u2 - 1.0f;
                const float t3 = u * (4.0f * u2 - 3.0f);
                const float t5 = u * (16.0f * u2 * u2 - 20.0f * u2 + 5.0f);
                const float cheb = (1.0f - t) * t3 + t * (0.6f * t5 + 0.4f * t2);
                return FastMath::fastTanh(0.45f * u + 0.85f * cheb);
            }
            case 1: {
                // Bank 1: West Coast Multi-Stage Sine / Serge Wavefolder
                const float folds = 1.0f + 5.5f * t;
                const float phase = std::clamp(u * folds * 0.5f + 0.25f * t, -4.0f, 4.0f);
                const float wrapped = phase - std::floor(phase);
                return FastMath::fastSinNorm(wrapped);
            }
            case 2: {
                // Bank 2: Geiger Counter Bitwise XOR / Ring Glitch Transfer Table
                const int qIn = std::clamp(static_cast<int>((u * 0.5f + 0.5f) * 255.0f), 0, 255);
                const int mask = (sub * 4 + 19) & 255;
                const int glitched = (qIn ^ mask) - 128;
                const float bitOut = static_cast<float>(glitched) * (1.0f / 128.0f);
                return (1.0f - 0.65f * t) * FastMath::fastTanh(u * 2.2f) + (0.65f * t) * bitOut;
            }
            case 3:
            default: {
                // Bank 3: Dead-Band Crossover, Full-Wave Rectifier & Staircase
                const float deadzone = 0.02f + 0.25f * t;
                float shaped = 0.0f;
                if (std::abs(u) > deadzone) {
                    const float sign = (u >= 0.0f) ? 1.0f : -1.0f;
                    shaped = sign * (std::abs(u) - deadzone) / (1.0f - deadzone);
                }
                const float rect = std::abs(u) * 2.0f - 1.0f;
                return FastMath::fastTanh(((1.0f - 0.5f * t) * shaped + 0.5f * t * rect) * 1.8f);
            }
        }
    }

    inline void processSamplePair(float& ioL, float& ioR) noexcept {
        const float dryL = ioL;
        const float dryR = ioR;

        // 0: PRE-GAIN / DRIVE (-6 dB .. +36 dB via Profile E)
        const float driveGain = Taper::profileE_DriveGain(params8[0], -6.0f, 36.0f);

        // 1 & 2: TABLE / WAVE + MORPH / SCAN across 256 curves
        const float basePos = std::clamp(params8[1], 0.0f, 1.0f) * 220.0f;
        const float morphOffset = std::clamp(params8[2], 0.0f, 1.0f) * 35.0f;
        const float exactTable = std::clamp(basePos + morphOffset, 0.0f, 254.99f);
        const int idxA = static_cast<int>(exactTable);
        const int idxB = std::min(idxA + 1, 255);
        const float frac = exactTable - static_cast<float>(idxA);

        // 3: BIT DEPTH (0.0 = 16-bit clean, 1.0 = 1.5-bit brutal Geiger crush)
        const float crushNorm = std::clamp(params8[3], 0.0f, 1.0f);
        const float bits = 16.0f - crushNorm * 14.5f;
        const float quantSteps = std::max(FastMath::fastPow2(bits), 2.0f);
        const float invQuantSteps = 1.0f / quantSteps;

        // 4: SAMPLE RATE DECIMATOR (0.0 = Full SR, 1.0 = 250 Hz)
        const float srrNorm = std::clamp(params8[4], 0.0f, 1.0f);
        const float targetSr = sampleRate_ * FastMath::fastPow2(-srrNorm * 7.5f);
        const float srrStep = std::clamp(targetSr * invSr_, 0.004f, 1.0f);

        srrPhase_ += srrStep;
        if (srrPhase_ >= 1.0f) {
            srrPhase_ -= 1.0f;
            float inDrivenL = FastMath::fastTanh(dryL * driveGain * 0.5f);
            float inDrivenR = FastMath::fastTanh(dryR * driveGain * 0.5f);

            // Quantize amplitude before lookup
            inDrivenL = std::round(inDrivenL * quantSteps) * invQuantSteps;
            inDrivenR = std::round(inDrivenR * quantSteps) * invQuantSteps;

            const float shapedL = (1.0f - frac) * evalTransferCurve(idxA, inDrivenL)
                                + frac * evalTransferCurve(idxB, inDrivenL);
            const float shapedR = (1.0f - frac) * evalTransferCurve(idxA, inDrivenR)
                                + frac * evalTransferCurve(idxB, inDrivenR);

            holdSample_[0] = shapedL;
            holdSample_[1] = shapedR;
        }

        // 6: SLEW / SMOOTH (Non-linear dv/dt slew rate limiter)
        const float slewKnob = std::clamp(params8[6], 0.0f, 1.0f);
        const float maxDelta = 2.0f * FastMath::fastPow2(-slewKnob * 6.5f);
        for (int c = 0; c < 2; ++c) {
            const float diff = holdSample_[c] - slewState_[c];
            slewState_[c] += std::clamp(diff, -maxDelta, maxDelta);
        }

        // 5: MASK / TONE (Post-shaper CytomicSVF low-pass filter)
        const float toneCutoff = Taper::profileD_Cutoff(params8[5], 250.0f, std::min(18500.0f, 0.45f * sampleRate_));
        const float toneQ = 0.707f + 2.0f * crushNorm;
        toneFilter_[0].setLowpass(toneCutoff, toneQ, invSr_);
        toneFilter_[1].setLowpass(toneCutoff, toneQ, invSr_);

        const float wetL = dcBlocker_[0].process(toneFilter_[0].process(slewState_[0]));
        const float wetR = dcBlocker_[1].process(toneFilter_[1].process(slewState_[1]));
        toneFilter_[0].clampState();
        toneFilter_[1].clampState();

        float dryGain = 1.0f;
        float wetGain = 0.0f;
        Taper::profileM_TriZoneMix(params8[7], phaseInvert, dryGain, wetGain);

        ioL = dryL * dryGain + wetL * wetGain;
        ioR = dryR * dryGain + wetR * wetGain;
    }

    float sampleRate_ = 48000.0f;
    float invSr_ = 1.0f / 48000.0f;
    float srrPhase_ = 0.0f;
    float holdSample_[2]{};
    float slewState_[2]{};
    DSPCommon::CytomicSVF toneFilter_[2];
    DSPCommon::DCBlocker dcBlocker_[2];
};

using KlangFallout = PipKlick;

} // namespace TbdAudio::FX
