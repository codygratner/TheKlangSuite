#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include "../common/AudioEffect.h"
#include "../drums/FormlessModMath.h"

namespace tkdsp {

/**
 * WiderStage — 100% Mono-Compatible Stereo Widener with Role-Adaptive Low-End Crossover.
 * 
 * Invariant: L = inL + g * s, R = inR - g * s  ==> L + R = inL + inR (bit-exact mono cancellation).
 * Low frequencies below role cutoff (BD: 300Hz, SD: 200Hz, etc.) are kept 100% mono.
 */
class WiderStage {
public:
    WiderStage() noexcept {
        updateAllpassFreqs();
    }

    void prepare(float sampleRate) noexcept {
        sampleRate_ = std::max(sampleRate, 4000.0f);
        invSampleRate_ = 1.0f / sampleRate_;
        updateAllpassFreqs();
        reset();
    }

    void reset() noexcept {
        crossoverLp_.reset();
        for (auto& ap : apChain_) {
            ap.reset();
        }
        dlIdx_ = 0;
        dlBuffer_.fill(0.0f);
    }

    /**
     * Process stereo frame (in-place).
     * @param inOutL Left channel sample
     * @param inOutR Right channel sample
     * @param widerAmount 0.0 .. 2.0 (0% .. 200%)
     * @param roleIdx 0..7 (active drum role for bass cutoff frequency)
     */
    inline void process(float& inOutL, float& inOutR, float widerAmount, int roleIdx) noexcept {
        if (widerAmount <= 0.001f) return; // Clean bypass wire

        const float mid = 0.5f * (inOutL + inOutR);
        const float fc = formless::getRoleWiderBassCutoffHz(roleIdx);

        // Update crossover lowpass filter at role cutoff
        crossoverLp_.setLowpass(fc, 0.7071f, invSampleRate_);
        const float lowBand = crossoverLp_.process(mid);
        crossoverLp_.clampState();

        // High band subject to spatial phase decorrelation
        const float highBand = mid - lowBand;

        // 4-stage all-pass phase network + fractional delay dispersion
        float s = highBand;
        for (size_t i = 0; i < apChain_.size(); ++i) {
            s = apChain_[i].process(s);
        }

        // Fractional delay line with feedback (1.8ms)
        const int delaySamples = std::clamp(static_cast<int>(0.0018f * sampleRate_), 4, static_cast<int>(kMaxDelay - 1));
        int readIdx = dlIdx_ - delaySamples;
        if (readIdx < 0) readIdx += kMaxDelay;
        const float delayed = dlBuffer_[readIdx];
        dlBuffer_[dlIdx_] = s + 0.25f * delayed;
        if (++dlIdx_ >= kMaxDelay) dlIdx_ = 0;

        // Composite side signal
        const float sideSignal = 0.6f * s + 0.4f * delayed;

        // Gain scaling: 0..200% -> g = 0.5 * widerAmount
        const float g = 0.5f * widerAmount;

        // Complementary summation: L = inOutL + g * s, R = inOutR - g * s
        // Notice: (inOutL + g*s) + (inOutR - g*s) = inOutL + inOutR!
        inOutL += g * sideSignal;
        inOutR -= g * sideSignal;
    }

private:
    struct SimpleAP1 {
        float a{0.5f};
        float z1{0.0f};

        void setFreq(float freqHz, float invSr) noexcept {
            float w = std::tan(3.14159265f * std::clamp(freqHz * invSr, 0.001f, 0.49f));
            a = (w - 1.0f) / (w + 1.0f);
        }

        void reset() noexcept { z1 = 0.0f; }

        inline float process(float x) noexcept {
            float y = a * x + z1;
            z1 = x - a * y;
            return y;
        }
    };

    void updateAllpassFreqs() noexcept {
        // Staggered frequencies for dense spatial decorrelation
        apChain_[0].setFreq(650.0f, invSampleRate_);
        apChain_[1].setFreq(1400.0f, invSampleRate_);
        apChain_[2].setFreq(3100.0f, invSampleRate_);
        apChain_[3].setFreq(6800.0f, invSampleRate_);
    }

    static constexpr int kMaxDelay = 512;
    float sampleRate_{48000.0f};
    float invSampleRate_{1.0f / 48000.0f};

    TbdAudio::DSPCommon::CytomicSVF crossoverLp_{};
    std::array<SimpleAP1, 4> apChain_{};
    std::array<float, kMaxDelay> dlBuffer_{};
    int dlIdx_{0};
};

} // namespace tkdsp
