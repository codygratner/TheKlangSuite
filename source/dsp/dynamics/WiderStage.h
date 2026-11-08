#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include "../common/AudioEffect.h"
#include "../drums/FormlessModMath.h"

namespace tkdsp {

/**
 * WiderStage — 100% Mono-Compatible Bipolar Stereo Widener (-2.0 .. 0.0 .. +2.0).
 * 
 * - Noon (0.0): OFF (Clean Wire Bypass, bit-exact mono dry signal).
 * - Up (> 0.0): Classic Wider (Multi-Tap Allpass Comb Diffusor with h[0] == 0,
 *               alternating prime delays, and balanced critical-band energy).
 * - Down (< 0.0): Hilbert Wider (31-Tap Linear-Phase Anti-Symmetric FIR Quadrature
 *                 Phase Widener with 0.00 dB tilt and flat magnitude response).
 * 
 * Invariant: (inOutL + inOutR) cancels side signal identically ==> bit-exact mono cancellation.
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

        // Calculate prime-staggered delay samples for Classic Wider
        delay1_ = std::clamp(static_cast<int>(0.0053f * sampleRate_), 4, kMaxDelay - 1);
        delay2_ = std::clamp(static_cast<int>(0.0089f * sampleRate_), 4, kMaxDelay - 1);
        delay3_ = std::clamp(static_cast<int>(0.0131f * sampleRate_), 4, kMaxDelay - 1);

        reset();
    }

    void reset() noexcept {
        crossoverLp_.reset();
        for (auto& ap : apChain_) {
            ap.reset();
        }
        dlIdx_ = 0;
        dlBuffer_.fill(0.0f);

        firIdx_ = 0;
        firBuffer_.fill(0.0f);
        lowIdx_ = 0;
        lowBuffer_.fill(0.0f);
    }

    /**
     * Process stereo frame (in-place).
     * @param inOutL Left channel sample
     * @param inOutR Right channel sample
     * @param widerAmount -2.0 .. +2.0 (Down: Hilbert -200% .. Noon: OFF .. Up: Classic +200%)
     * @param roleIdx 0..7 (active drum role for bass cutoff frequency)
     */
    inline void process(float& inOutL, float& inOutR, float widerAmount, int roleIdx) noexcept {
        if (std::abs(widerAmount) <= 0.001f) return; // Clean bypass wire at noon (OFF)

        const float mid = 0.5f * (inOutL + inOutR);

        // Update crossover lowpass filter at role cutoff (cached to prevent transcendental per-sample recomputation)
        if (roleIdx != cachedRoleIdx_ || invSampleRate_ != cachedInvSr_) {
            cachedRoleIdx_ = roleIdx;
            cachedInvSr_ = invSampleRate_;
            const float fc = formless::getRoleWiderBassCutoffHz(roleIdx);
            crossoverLp_.setLowpass(fc, 0.7071f, invSampleRate_);
        }
        const float lowBand = crossoverLp_.process(mid);
        crossoverLp_.clampState();

        // High band subject to spatial phase decorrelation
        const float highBand = mid - lowBand;

        if (widerAmount > 0.0f) {
            // ========================================================
            // UP (> 0): Classic Wider (Multi-Tap Allpass Comb Diffusor)
            // ========================================================
            float s = highBand;
            for (size_t i = 0; i < apChain_.size(); ++i) {
                s = apChain_[i].process(s);
            }

            dlBuffer_[dlIdx_] = s;

            // Delayed multi-tap read with alternating polarities (h[0] == 0 invariant)
            const float tap1 = readDelay(delay1_);
            const float tap2 = readDelay(delay2_);
            const float tap3 = readDelay(delay3_);

            if (++dlIdx_ >= kMaxDelay) dlIdx_ = 0;

            const float sideSignal = 0.50f * tap1 - 0.35f * tap2 + 0.15f * tap3;
            const float g = 0.5f * widerAmount;

            inOutL += g * sideSignal;
            inOutR -= g * sideSignal;
        } else {
            // ========================================================
            // DOWN (< 0): Hilbert Wider (31-Tap FIR Quadrature Phase)
            // ========================================================
            firBuffer_[firIdx_] = highBand;
            lowBuffer_[lowIdx_] = lowBand;

            // Center tap (delay M = 15 samples)
            const float dryMid = firBuffer_[(firIdx_ - 15) & 31];
            const float delayedLow = lowBuffer_[(lowIdx_ - 15) & 31];

            // 31-tap anti-symmetric FIR filter (h[15-k] = -h[15+k] for odd k, even k == 0)
            static constexpr float kH[8] = {
                0.63384584f,  // k = 1  (taps 14 & 16)
                0.20401733f,  // k = 3  (taps 12 & 18)
                0.11398417f,  // k = 5  (taps 10 & 20)
                0.07296282f,  // k = 7  (taps 8 & 22)
                0.04875590f,  // k = 9  (taps 6 & 24)
                0.03265204f,  // k = 11 (taps 4 & 26)
                0.02134400f,  // k = 13 (taps 2 & 28)
                0.02050168f   // k = 15 (taps 0 & 30)
            };

            float quad = 0.0f;
            for (int i = 0; i < 8; ++i) {
                const int k = 1 + 2 * i;
                const float xEarly = firBuffer_[(firIdx_ - (15 - k)) & 31];
                const float xLate  = firBuffer_[(firIdx_ - (15 + k)) & 31];
                quad += kH[i] * (xEarly - xLate);
            }

            firIdx_ = (firIdx_ + 1) & 31;
            lowIdx_ = (lowIdx_ + 1) & 31;

            const float g = 0.5f * (-widerAmount); // widerAmount is negative

            inOutL = delayedLow + dryMid + g * quad;
            inOutR = delayedLow + dryMid - g * quad;
        }
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
        apChain_[0].setFreq(800.0f, invSampleRate_);
        apChain_[1].setFreq(1800.0f, invSampleRate_);
        apChain_[2].setFreq(3800.0f, invSampleRate_);
        apChain_[3].setFreq(7200.0f, invSampleRate_);
    }

    inline float readDelay(int delaySamples) const noexcept {
        int idx = dlIdx_ - delaySamples;
        if (idx < 0) idx += kMaxDelay;
        return dlBuffer_[idx];
    }

    static constexpr int kMaxDelay = 4096;
    float sampleRate_{48000.0f};
    float invSampleRate_{1.0f / 48000.0f};
    int cachedRoleIdx_{-1};
    float cachedInvSr_{0.0f};

    int delay1_{254};
    int delay2_{427};
    int delay3_{629};

    TbdAudio::DSPCommon::CytomicSVF crossoverLp_{};
    std::array<SimpleAP1, 4> apChain_{};
    std::array<float, kMaxDelay> dlBuffer_{};
    int dlIdx_{0};

    // 31-Tap FIR Hilbert Ring Buffer (power of two 32 for fast bitmask)
    std::array<float, 32> firBuffer_{};
    int firIdx_{0};
    std::array<float, 32> lowBuffer_{};
    int lowIdx_{0};
};

} // namespace tkdsp
