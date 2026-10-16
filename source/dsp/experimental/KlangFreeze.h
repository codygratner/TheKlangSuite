#pragma once

#include "../common/AudioEffect.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

namespace TbdAudio::Experimental {

/**
 * @brief KlangFreeze (KlangShift) — Granular Cloud Freeze & Scale-Quantized Pitch Harmonizer.
 *
 * Implements a stereo circular grain buffer (32768 samples per channel) with 4 overlapping
 * Hann-windowed grains (phases offset by 0.00, 0.25, 0.50, 0.75), scale-quantized pitch
 * shifting (-24..+24 semitones), shimmer feedback loop, and infinite Freeze sustain when
 * Knob 3 (SCALE SNAP / FREEZE) exceeds 0.90.
 *
 * 8-Knob Performance Layout (params8[0..7]):
 *   [0] PITCH / SHIFT         (-24 .. +24 semitones)
 *   [1] FINE DETUNE           (-50 .. +50 cents)
 *   [2] GRAIN SIZE            (10 ms .. 250 ms)
 *   [3] SCALE SNAP / FREEZE   (0: Cont, 1: Chrom, 2: Phrygian, 3: Phryg Dom, 4: Harm Min, >0.90: FREEZE)
 *   [4] TEXTURE / DENSITY     (Grain window contour & micro-jitter density)
 *   [5] STEREO SPREAD         (Alternating stereo grain pan & pitch spread)
 *   [6] FEEDBACK / SHIMMER    (0 .. 85% pitch-regenerated shimmer feedback)
 *   [7] MIX / ∅               (Tri-Zone Mix via profileM_TriZoneMix)
 */
class KlangFreeze final : public AudioEffect8 {
public:
    static constexpr int kNumGrains = 4;
    static constexpr int kGrainBufLen = 32768;
    static constexpr int kGrainBufMask = kGrainBufLen - 1;

    KlangFreeze() noexcept {
        params8[0] = 0.50f; // Pitch Shift (0 st)
        params8[1] = 0.50f; // Fine Detune (0 cents)
        params8[2] = 0.35f; // Grain Size (~90 ms)
        params8[3] = 0.00f; // Scale Snap (Continuous, un-frozen)
        params8[4] = 0.40f; // Texture / Density
        params8[5] = 0.50f; // Stereo Spread
        params8[6] = 0.20f; // Shimmer Feedback
        params8[7] = 0.50f; // Mix (50/50)
        reset();
    }

    ~KlangFreeze() override = default;

    void init(const BlockContext& ctx) override {
        prepare(ctx.sampleRate > 0.0f ? static_cast<double>(ctx.sampleRate) : 48000.0, 512);
    }

    void prepare(double sampleRate, int /*maxBlockSize*/ = 512) noexcept {
        sampleRate_ = std::clamp(static_cast<float>(sampleRate), 8000.0f, 192000.0f);
        invSampleRate_ = 1.0f / sampleRate_;
        reset();
    }

    void reset() noexcept override {
        std::memset(bufL_, 0, sizeof(bufL_));
        std::memset(bufR_, 0, sizeof(bufR_));
        writePos_ = 0;
        for (int g = 0; g < kNumGrains; ++g) {
            phases_[g] = 0.25f * static_cast<float>(g);
            grainJitter_[g] = 0.0f;
        }
        fbStateL_ = 0.0f;
        fbStateR_ = 0.0f;
        lpFbL_ = 0.0f;
        lpFbR_ = 0.0f;
        dcX1L_ = 0.0f;
        dcY1L_ = 0.0f;
        dcX1R_ = 0.0f;
        dcY1R_ = 0.0f;
        rngState_ = 0x13579BDFu;
    }

    void setParam8(int idx, float val) noexcept {
        if (idx >= 0 && idx < 8) {
            params8[idx] = std::clamp(val, 0.0f, 1.0f);
        }
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        if (!buffer || numSamples <= 0) return;
        FastMath::enableFTZDAZ();

        if (ctx.sampleRate > 0.0 && std::fabs(static_cast<float>(ctx.sampleRate) - sampleRate_) > 1.0f) {
            sampleRate_ = std::clamp(static_cast<float>(ctx.sampleRate), 8000.0f, 192000.0f);
            invSampleRate_ = 1.0f / sampleRate_;
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

        if (ctx.sampleRate > 0.0 && std::fabs(static_cast<float>(ctx.sampleRate) - sampleRate_) > 1.0f) {
            sampleRate_ = std::clamp(static_cast<float>(ctx.sampleRate), 8000.0f, 192000.0f);
            invSampleRate_ = 1.0f / sampleRate_;
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
    inline float nextRandBipolar() noexcept {
        rngState_ ^= rngState_ << 13;
        rngState_ ^= rngState_ >> 17;
        rngState_ ^= rngState_ << 5;
        return static_cast<float>(static_cast<int32_t>(rngState_)) * (1.0f / 2147483648.0f);
    }

    static inline float quantizeSemitones(float rawSemitones, int scaleMode) noexcept {
        if (scaleMode <= 0) {
            return rawSemitones; // 0: Continuous
        }
        const int rounded = static_cast<int>(std::round(rawSemitones));
        if (scaleMode == 1) {
            return static_cast<float>(rounded); // 1: Chromatic
        }

        // Scale degree tables (7-note masks inside 12-TET octave)
        static constexpr int kScaleDegrees[3][7] = {
            { 0, 1, 3, 5, 7, 8, 10 }, // 2: Phrygian
            { 0, 1, 4, 5, 7, 8, 10 }, // 3: Phrygian Dominant
            { 0, 2, 3, 5, 7, 8, 11 }  // 4: Harmonic Minor
        };
        const int tableIdx = std::clamp(scaleMode - 2, 0, 2);

        int octave = (rounded >= 0) ? (rounded / 12) : ((rounded - 11) / 12);
        int pc = rounded - octave * 12;
        if (pc < 0) pc += 12;

        int bestPc = kScaleDegrees[tableIdx][0];
        int minDist = 99;
        for (int i = 0; i < 7; ++i) {
            const int deg = kScaleDegrees[tableIdx][i];
            const int dist = (pc > deg) ? (pc - deg) : (deg - pc);
            if (dist < minDist) {
                minDist = dist;
                bestPc = deg;
            }
        }
        return static_cast<float>(octave * 12 + bestPc);
    }

    inline float readLinear(const float* buf, float readIdx) const noexcept {
        int i0 = static_cast<int>(std::floor(readIdx));
        const float frac = readIdx - static_cast<float>(i0);
        const int idx0 = i0 & kGrainBufMask;
        const int idx1 = (i0 + 1) & kGrainBufMask;
        return buf[idx0] + frac * (buf[idx1] - buf[idx0]);
    }

    inline void processSamplePair(float& ioL, float& ioR) noexcept {
        const float dryL = ioL;
        const float dryR = ioR;

        // 3: SCALE SNAP / FREEZE (> 0.90 engages Freeze Mode & Harmonic Minor or Phrygian snap)
        const float snapParam = std::clamp(params8[3], 0.0f, 1.0f);
        const bool freezeActive = (snapParam > 0.90f);
        const int scaleMode = freezeActive
            ? 4
            : std::clamp(static_cast<int>((snapParam / 0.90f) * 4.99f), 0, 4);

        // 0: PITCH / SHIFT (-24 .. +24 st) & 1: FINE DETUNE (-50 .. +50 cents)
        const float rawSt = (std::clamp(params8[0], 0.0f, 1.0f) * 48.0f) - 24.0f;
        const float quantSt = quantizeSemitones(rawSt, scaleMode);
        const float fineSt = (std::clamp(params8[1], 0.0f, 1.0f) - 0.5f); // -0.5 .. +0.5 st (-50..+50 cents)
        const float totalSt = quantSt + fineSt;
        const float pitchRatio = FastMath::fastPow2(totalSt * (1.0f / 12.0f));

        // 2: GRAIN SIZE (10 ms .. 250 ms)
        const float grainMs = 10.0f + std::clamp(params8[2], 0.0f, 1.0f) * 240.0f;
        const float grainSamples = std::clamp(grainMs * 0.001f * sampleRate_, 64.0f, static_cast<float>(kGrainBufLen - 512));

        // 4: TEXTURE / DENSITY & 5: STEREO SPREAD & 6: FEEDBACK / SHIMMER (0 .. 85%)
        const float texture = std::clamp(params8[4], 0.0f, 1.0f);
        const float spread = std::clamp(params8[5], 0.0f, 1.0f);
        const float feedback = std::clamp(params8[6], 0.0f, 1.0f) * 0.85f;

        // Write incoming + shimmer feedback into circular buffer unless FREEZE is active
        if (!freezeActive) {
            const float inL = FastMath::fastTanh(dryL + fbStateL_ * feedback);
            const float inR = FastMath::fastTanh(dryR + fbStateR_ * feedback);
            bufL_[writePos_] = inL;
            bufR_[writePos_] = inR;
            writePos_ = (writePos_ + 1) & kGrainBufMask;
        }

        // Advance grain phases: in Freeze mode, scrub continuously at pitchRatio speed;
        // in live harmonizer mode, step relative to (1.0 - pitchRatio) / grainSamples
        float phaseStep = freezeActive
            ? (pitchRatio / grainSamples)
            : ((1.0f - pitchRatio) / grainSamples);
        if ( std::fabs(phaseStep) < (0.15f / grainSamples) && !freezeActive ) {
            // Keep gentle motion when pitch shift is near unison so texture/spread remains alive
            phaseStep = (0.15f * texture) / grainSamples;
        }

        float wetL = 0.0f;
        float wetR = 0.0f;

        for (int g = 0; g < kNumGrains; ++g) {
            phases_[g] += phaseStep;
            if (phases_[g] >= 1.0f) {
                phases_[g] -= std::floor(phases_[g]);
                grainJitter_[g] = nextRandBipolar() * texture * 0.35f * grainSamples;
            } else if (phases_[g] < 0.0f) {
                phases_[g] -= std::floor(phases_[g]);
                grainJitter_[g] = nextRandBipolar() * texture * 0.35f * grainSamples;
            }

            const float ph = std::clamp(phases_[g], 0.0f, 1.0f);

            // Hann window: 0.5 * (1 - cos(2*pi*ph)) via FastMath::fastCosNorm
            float win = 0.5f * (1.0f - FastMath::fastCosNorm(ph));
            // Texture shapes window fullness smoothly
            win = win * (1.0f - 0.35f * texture) + FastMath::fastSinNorm(0.5f * ph) * (0.35f * texture);

            const float delayOffset = ph * grainSamples + std::fabs(grainJitter_[g]) + 4.0f;
            const float readPos = static_cast<float>(writePos_) - std::clamp(delayOffset, 2.0f, static_cast<float>(kGrainBufLen - 4));

            const float gL = readLinear(bufL_, readPos) * win;
            const float gR = readLinear(bufR_, readPos) * win;

            const float panOffset = ((g & 1) == 0 ? -0.45f : 0.45f) * spread;
            const float gainL = 0.5f - panOffset;
            const float gainR = 0.5f + panOffset;

            wetL += gL * gainL + gR * (1.0f - gainL) * 0.35f * spread;
            wetR += gR * gainR + gL * (1.0f - gainR) * 0.35f * spread;
        }

        // Normalize 4 overlapping Hann grains (sum of 4 quarter-spaced Hann windows = 2.0)
        wetL *= 0.7071f;
        wetR *= 0.7071f;

        // Shimmer feedback HF damping & DC blocking
        const float dampG = 0.35f;
        lpFbL_ += dampG * (wetL - lpFbL_);
        lpFbR_ += dampG * (wetR - lpFbR_);

        const float dcL = lpFbL_ - dcX1L_ + 0.995f * dcY1L_;
        dcX1L_ = lpFbL_;
        dcY1L_ = dcL;

        const float dcR = lpFbR_ - dcX1R_ + 0.995f * dcY1R_;
        dcX1R_ = lpFbR_;
        dcY1R_ = dcR;

        fbStateL_ = dcL;
        fbStateR_ = dcR;

        float dryGain = 1.0f;
        float wetGain = 0.0f;
        Taper::profileM_TriZoneMix(params8[7], phaseInvert, dryGain, wetGain);

        ioL = dryL * dryGain + wetL * wetGain;
        ioR = dryR * dryGain + wetR * wetGain;
    }

    float bufL_[kGrainBufLen]{};
    float bufR_[kGrainBufLen]{};
    int writePos_ = 0;
    float phases_[kNumGrains]{ 0.0f, 0.25f, 0.50f, 0.75f };
    float grainJitter_[kNumGrains]{};

    float sampleRate_ = 48000.0f;
    float invSampleRate_ = 1.0f / 48000.0f;
    float fbStateL_ = 0.0f;
    float fbStateR_ = 0.0f;
    float lpFbL_ = 0.0f;
    float lpFbR_ = 0.0f;
    float dcX1L_ = 0.0f;
    float dcY1L_ = 0.0f;
    float dcX1R_ = 0.0f;
    float dcY1R_ = 0.0f;
    uint32_t rngState_ = 0x13579BDFu;
};

using KlangShift = KlangFreeze;

} // namespace TbdAudio::Experimental
