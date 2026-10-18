#pragma once

#include "../common/AudioEffect.h"
#include "../oscillators/PolyBLEP.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace TbdAudio::Experimental {

/**
 * @brief PhaseShriek (`KlangPLL`) — Doepfer A-196 Phase Locked Loop (PLL) Tracking
 *        Oscillator, Clock Divider Multiplier & Chaotic XOR Screamer.
 *
 * Clean-room mathematical model of the Doepfer A-196 PLL subsystem:
 *   1. Input Schmitt Trigger Comparator with adjustable tracking threshold hysteresis.
 *   2. 3 Selectable Phase Comparator Topologies:
 *        - PC 1 (XOR Gate): Locks to harmonics & subharmonics with jagged 2*f_in FM ripple.
 *        - PC 2 (RS Flip-Flop): Edge-triggered phase lock for singing tracking leads.
 *        - PC 3 (Tri-State Phase-Frequency Detector [PFD]): Wide-range clinical frequency capture.
 *   3. Clock Divider Feedback Multiplier (N = 1..8): Divides the VCO feedback by N so the
 *      VCO locks at N * f_in (octave/harmonic multiplier!).
 *   4. Low-Pass Loop Filter (0.5 Hz .. 2500 Hz): Controls loop damping from slow rubbery
 *      portamento glissando (< 15 Hz) to under-damped FM jitter and chaotic screaming (> 200 Hz).
 *   5. Band-Limited PolyBLEP VCO morphing between Square/Pulse, Saw, and Triangle.
 *
 * 8 Performance Knobs (params8[0..7]):
 *   [0] OFFSET        (20 Hz .. 4000 Hz free-running VCO resting frequency)
 *   [1] LPF FREQ      (0.5 Hz .. 2500 Hz loop filter cutoff: Glissando <-> FM Jitter/Chaos)
 *   [2] PC TYPE       (0: XOR Gate, 1: RS Flip-Flop, 2: Tri-State PFD)
 *   [3] DIVIDER       (1x .. 8x feedback clock divider harmonic multiplier)
 *   [4] TRACK SENS    (Input comparator hysteresis & pre-gain sensitivity)
 *   [5] FEEDBACK      (-100% .. +100% VCO self-modulation into input comparator)
 *   [6] WAVE SHAPE    (PolyBLEP Square -> Pulse -> Saw -> Triangle + Stereo Detune)
 *   [7] MIX / ∅       (Profile M Tri-Zone Mix: 0.0 = 100% Wet, 0.5 = 100% Dry, 1.0 = 50/50 Parallel)
 */
class PhaseShriek final : public AudioEffect8 {
public:
    PhaseShriek() noexcept {
        setParam(0, 0.30f); // Offset (~180 Hz)
        setParam(1, 0.35f); // LPF Cutoff (~35 Hz fast tracking with grit)
        setParam(2, 0.00f); // PC1: XOR Gate
        setParam(3, 0.00f); // Divider 1x
        setParam(4, 0.65f); // Track Sens
        setParam(5, 0.20f); // Feedback
        setParam(6, 0.25f); // Wave Shape
        setParam(7, 0.00f); // Mix (100% Wet default per Profile M)
        reset();
    }

    ~PhaseShriek() override = default;

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
            ch_[c].vcoPhase = 0.0f;
            ch_[c].divCounter = 0;
            ch_[c].divState = false;
            ch_[c].lastInHigh = false;
            ch_[c].lastVcoHigh = false;
            ch_[c].rsState = 0.0f;
            ch_[c].pfdState = 0.0f;
            ch_[c].lastVcoOut = 0.0f;
            ch_[c].loopLpf.reset();
            ch_[c].dcBlocker.init(sampleRate_);
            ch_[c].dcBlocker.reset();
        }
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
    struct ChannelPLL {
        float vcoPhase = 0.0f;
        int divCounter = 0;
        bool divState = false;
        bool lastInHigh = false;
        bool lastVcoHigh = false;
        float rsState = 0.0f;
        float pfdState = 0.0f;
        float lastVcoOut = 0.0f;
        DSPCommon::OnePoleTPT loopLpf;
        DSPCommon::DCBlocker dcBlocker;
    };

    inline float processSingleChannel(ChannelPLL& st, float inSig, float offsetHz,
                                      float lpfCutoffHz, int pcType, int dividerRatio,
                                      float hystThresh, float fbAmount, float waveMorph) noexcept {
        // 1. Input Schmitt Trigger Comparator with feedback injection
        const float compIn = inSig + fbAmount * st.lastVcoOut;
        bool inHigh = st.lastInHigh;
        if (compIn > hystThresh) {
            inHigh = true;
        } else if (compIn < -hystThresh) {
            inHigh = false;
        }

        const bool vcoHigh = (dividerRatio <= 1) ? (st.vcoPhase < 0.5f) : st.divState;

        // 2. Phase Comparator (PC1: XOR, PC2: RS Flip-Flop, PC3: Tri-State PFD)
        float pcError = 0.0f;
        if (pcType == 0) {
            // PC 1: Exclusive OR Gate
            pcError = (inHigh != vcoHigh) ? 1.0f : -1.0f;
        } else if (pcType == 1) {
            // PC 2: Edge-Triggered RS Flip-Flop
            if (inHigh && !st.lastInHigh) st.rsState = 1.0f;
            if (vcoHigh && !st.lastVcoHigh) st.rsState = -1.0f;
            pcError = st.rsState;
        } else {
            // PC 3: Phase-Frequency Detector (Tri-State Charge Pump)
            if (inHigh && !st.lastInHigh) st.pfdState = std::min(st.pfdState + 1.0f, 1.0f);
            if (vcoHigh && !st.lastVcoHigh) st.pfdState = std::max(st.pfdState - 1.0f, -1.0f);
            pcError = st.pfdState;
            st.pfdState *= 0.9992f; // charge pump bleed
        }
        st.lastInHigh = inHigh;
        st.lastVcoHigh = vcoHigh;

        // 3. Low-Pass Loop Filter
        st.loopLpf.setCutoff(lpfCutoffHz, invSr_);
        const float smoothedCV = st.loopLpf.processLP(pcError);

        // 4. Linear / Exponential Hybrid VCO Frequency Update
        // Allows wide capture range across 20 Hz .. 12 kHz
        const float cvOctaves = smoothedCV * 3.2f;
        const float linearPull = smoothedCV * offsetHz * 1.4f;
        const float vcoFreq = std::clamp(
            offsetHz * FastMath::fastPow2(cvOctaves) + linearPull,
            8.0f, 0.45f * sampleRate_);

        const float dt = vcoFreq * invSr_;
        const float prevPhase = st.vcoPhase;
        st.vcoPhase = Oscillators::PolyBLEP::wrapPhase(st.vcoPhase + dt);

        // Update feedback clock divider on VCO wrap
        if (st.vcoPhase < prevPhase) {
            ++st.divCounter;
            if (st.divCounter >= dividerRatio) {
                st.divCounter = 0;
                st.divState = !st.divState;
            }
        }

        // 5. Band-Limited PolyBLEP VCO Waveform Morph (Square -> Pulse -> Saw -> Triangle)
        float rawWave = 0.0f;
        if (waveMorph < 0.34f) {
            const float pw = 0.50f - (waveMorph * 3.0f) * 0.32f; // 50% down to 18% pulse
            rawWave = Oscillators::PolyBLEP::pulse(st.vcoPhase, dt, pw);
        } else if (waveMorph < 0.67f) {
            const float t = (waveMorph - 0.34f) * 3.03f;
            const float pls = Oscillators::PolyBLEP::pulse(st.vcoPhase, dt, 0.20f);
            const float saw = Oscillators::PolyBLEP::saw(st.vcoPhase, dt);
            rawWave = (1.0f - t) * pls + t * saw;
        } else {
            const float t = (waveMorph - 0.67f) * 3.03f;
            const float saw = Oscillators::PolyBLEP::saw(st.vcoPhase, dt);
            const float tri = Oscillators::PolyBLEP::triangle(st.vcoPhase);
            rawWave = (1.0f - t) * saw + t * tri;
        }

        st.lastVcoOut = rawWave;
        return st.dcBlocker.process(rawWave * 0.80f);
    }

    inline void processSamplePair(float& ioL, float& ioR) noexcept {
        const float dryL = ioL;
        const float dryR = ioR;

        // 0: OFFSET (20 Hz .. 4000 Hz)
        const float offsetHz = Taper::profileD_Cutoff(params8[0], 20.0f, 4000.0f);

        // 1: LPF FREQ (0.5 Hz .. 2500 Hz via Profile C ModSpeed)
        const float lpfCutoffHz = Taper::profileC_ModSpeed(params8[1], 0.5f, 2500.0f);

        // 2: PC TYPE (0: XOR, 1: RS-FF, 2: PFD)
        const int pcType = std::clamp(static_cast<int>(params8[2] * 2.99f), 0, 2);

        // 3: DIVIDER (1 .. 8)
        const int divider = std::clamp(1 + static_cast<int>(params8[3] * 7.99f), 1, 8);

        // 4: TRACK SENS (0..1 -> hysteresis threshold 0.18 down to 0.002)
        const float sens = std::clamp(params8[4], 0.0f, 1.0f);
        const float hyst = 0.002f + (1.0f - sens) * (1.0f - sens) * 0.18f;
        const float preGain = 1.0f + sens * 6.0f;

        // 5: FEEDBACK (-0.65 .. +0.65)
        const float fb = (std::clamp(params8[5], 0.0f, 1.0f) - 0.5f) * 1.30f;

        // 6: WAVE SHAPE
        const float waveMorph = std::clamp(params8[6], 0.0f, 1.0f);

        // Slight stereo offset on R channel for wide analog PLL scream
        const float offsetR = offsetHz * (1.0f + 0.008f * waveMorph);

        const float wetL = processSingleChannel(ch_[0], dryL * preGain, offsetHz, lpfCutoffHz, pcType, divider, hyst, fb, waveMorph);
        const float wetR = processSingleChannel(ch_[1], dryR * preGain, offsetR,  lpfCutoffHz, pcType, divider, hyst, -fb, waveMorph);

        float dryGain = 1.0f;
        float wetGain = 0.0f;
        Taper::profileM_TriZoneMix(params8[7], phaseInvert, dryGain, wetGain);

        ioL = dryL * dryGain + wetL * wetGain;
        ioR = dryR * dryGain + wetR * wetGain;
    }

    float sampleRate_ = 48000.0f;
    float invSr_ = 1.0f / 48000.0f;
    ChannelPLL ch_[2];
};

using KlangPLL = PhaseShriek;

} // namespace TbdAudio::Experimental
