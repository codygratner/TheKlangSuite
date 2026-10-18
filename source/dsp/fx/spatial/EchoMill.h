#pragma once

#include "../../common/AudioEffect.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

namespace TbdAudio::Spatial {

/**
 * @brief EchoMill (`TKMD` / `KlangLoopMill`) — Recursive 2-Slot Insert-Rack Delay & Degradation Mill.
 *
 * Implements `docs/specs/spinoffs/the_klang_mill_delay.md`:
 *   - Stereo fractional delay line with 3rd-order Hermite spline interpolation, Cross-Coupled
 *     Ping-Pong bounce, and Haas micro-delay stereo offset.
 *   - Slot 1 Recursive Insert Engine (inside feedback loop):
 *       0: Clean Bypass
 *       1: FreakShifter (Harald Bode Hilbert SSB Frequency Shifter: spiraling +/-500 Hz echoes)
 *       2: Pitch Shifter (Dual-grain Hann-windowed pitch harmonizer staircase)
 *       3: Swedish KlangSaw (HM-2 dead-band zero-crossing distortion + 1050 Hz chainsaw peak)
 *       4: Buchla Phase-Fold (West Coast recursive sine wavefolder)
 *       5: Pipe & Wire Modal Resonator (Tuned sub-ms waveguide resonator inside feedback)
 *   - Slot 2 Recursive Insert Engine (inside feedback loop):
 *       0: Clean Bypass
 *       1: Analog Slop / Tape Wow (Asynchronous dual LFOs + Brownian tape drift)
 *       2: Tape Damping / HF Loss (OnePoleTPT dark dub low-pass filter)
 *       3: Bitcrush & Slew (ADC bit quantizer + sample-rate decimator)
 *       4: Juno BBD Chorus (Anti-phase stereo bucket-brigade modulation)
 *   - Hyperbolic Feedback (0..120% via Profile F) with Padé fastTanh limiter & Infinite Freeze Hold.
 *
 * 8 Performance Knobs (params8[0..7]):
 *   [0] TIME / RATE         (5 ms .. 1300 ms continuous Hermite delay time)
 *   [1] STEREO / PING-PONG  (-100% Ping-Pong Crossfeed .. 0% True Stereo .. +100% Haas Offset)
 *   [2] SLOT 1 ENGINE       (0: Bypass, 1: Bode Shift, 2: Pitch Shift, 3: Chainsaw, 4: Buchla Fold, 5: Resonator)
 *   [3] SLOT 1 INTENSITY    (0.0 .. 1.0 context-sensitive parameter for Slot 1)
 *   [4] SLOT 2 ENGINE       (0: Bypass, 1: Tape Wow, 2: HF Damping, 3: Bitcrush, 4: BBD Chorus)
 *   [5] SLOT 2 DEPTH        (0.0 .. 1.0 context-sensitive parameter for Slot 2)
 *   [6] FEEDBACK & FREEZE   (0% .. 120% Profile F Hyperbolic; >= 0.96 latches Infinite Freeze!)
 *   [7] MIX / ∅             (Profile M Tri-Zone Mix: 0.0 = 100% Wet, 0.5 = 100% Dry, 1.0 = 50/50)
 */
class EchoMill final : public AudioEffect8 {
public:
    static constexpr int kMaxDelaySamples = 65536;
    static constexpr int kDelayMask = kMaxDelaySamples - 1;
    static constexpr int kMiniBufLen = 2048;
    static constexpr int kMiniMask = kMiniBufLen - 1;

    EchoMill() noexcept {
        setParam(0, 0.30f); // Time (~280 ms)
        setParam(1, 0.25f); // Mild Ping-Pong
        setParam(2, 0.20f); // Slot 1: Bode Shifter
        setParam(3, 0.55f); // Slot 1 Intensity
        setParam(4, 0.35f); // Slot 2: Tape Wow / Damping
        setParam(5, 0.40f); // Slot 2 Depth
        setParam(6, 0.55f); // Feedback
        setParam(7, 0.00f); // Mix (100% Wet default per Profile M)
        reset();
    }

    ~EchoMill() override = default;

    void init(const BlockContext& ctx) override {
        prepare(ctx.sampleRate > 0.0f ? static_cast<double>(ctx.sampleRate) : 48000.0, 512);
    }

    void prepare(double sampleRate, int /*maxBlockSize*/ = 512) noexcept {
        sampleRate_ = std::clamp(static_cast<float>(sampleRate), 8000.0f, 192000.0f);
        invSr_ = 1.0f / sampleRate_;
        reset();
    }

    void reset() noexcept override {
        std::memset(delayBufL_, 0, sizeof(delayBufL_));
        std::memset(delayBufR_, 0, sizeof(delayBufR_));
        std::memset(miniBufL_, 0, sizeof(miniBufL_));
        std::memset(miniBufR_, 0, sizeof(miniBufR_));
        writePos_ = 0;
        miniWrite_ = 0;
        smoothDelayL_ = 12000.0f;
        smoothDelayR_ = 12000.0f;
        bodePhase_ = 0.0f;
        std::memset(hilbertI_, 0, sizeof(hilbertI_));
        std::memset(hilbertQ_, 0, sizeof(hilbertQ_));
        grainPhase_ = 0.0f;
        wowPhase1_ = 0.0f;
        wowPhase2_ = 0.25f;
        srrPhase_ = 0.0f;
        crushHoldL_ = 0.0f;
        crushHoldR_ = 0.0f;
        chainsawBellL_.reset();
        chainsawBellR_.reset();
        dampFilterL_.reset();
        dampFilterR_.reset();
        dcBlockL_.init(sampleRate_);
        dcBlockR_.init(sampleRate_);
        dcBlockL_.reset();
        dcBlockR_.reset();
    }

    void setParam8(int idx, float val) noexcept {
        setParam(idx, val);
    }

    void setFreeze(bool freeze) noexcept { manualFreeze_ = freeze; }
    [[nodiscard]] bool isFrozen() const noexcept { return manualFreeze_ || (params8[6] >= 0.96f); }

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
    [[nodiscard]] static inline float readHermite(const float* buf, int writePos, float delaySamples) noexcept {
        const float d = std::clamp(delaySamples, 2.0f, static_cast<float>(kMaxDelaySamples - 4));
        const float readPos = static_cast<float>(writePos) - d;
        const int i0 = static_cast<int>(std::floor(readPos));
        const float frac = readPos - static_cast<float>(i0);

        const float ym1 = buf[(i0 - 1) & kDelayMask];
        const float y0  = buf[(i0    ) & kDelayMask];
        const float y1  = buf[(i0 + 1) & kDelayMask];
        const float y2  = buf[(i0 + 2) & kDelayMask];

        const float c0 = y0;
        const float c1 = 0.5f * (y1 - ym1);
        const float c2 = ym1 - 2.5f * y0 + 2.0f * y1 - 0.5f * y2;
        const float c3 = 0.5f * (y2 - ym1) + 1.5f * (y0 - y1);
        return ((c3 * frac + c2) * frac + c1) * frac + c0;
    }

    inline void processSlot1(int engine, float intensity, float& sigL, float& sigR) noexcept {
        const float u = std::clamp(intensity, 0.0f, 1.0f);
        switch (engine) {
            case 0: // Clean Bypass
                return;
            case 1: { // 1: FreakShifter (Bode Hilbert SSB Shifter -250 Hz .. +250 Hz)
                const float shiftHz = Taper::profileB_Bipolar(u) * 250.0f;
                bodePhase_ += shiftHz * invSr_;
                bodePhase_ -= std::floor(bodePhase_);
                const float cosC = FastMath::fastCosNorm(bodePhase_);
                const float sinC = FastMath::fastSinNorm(bodePhase_);

                constexpr float cI[2] = { 0.6923878f, 0.9882295f };
                constexpr float cQ[2] = { 0.4021921f, 0.9722909f };
                float inArr[2] = { sigL, sigR };
                for (int c = 0; c < 2; ++c) {
                    float xI = inArr[c], xQ = inArr[c];
                    for (int s = 0; s < 2; ++s) {
                        const float yI = cI[s] * (xI - hilbertI_[c][s][1]) + hilbertI_[c][s][0];
                        hilbertI_[c][s][0] = xI; hilbertI_[c][s][1] = yI; xI = yI;
                        const float yQ = cQ[s] * (xQ - hilbertQ_[c][s][1]) + hilbertQ_[c][s][0];
                        hilbertQ_[c][s][0] = xQ; hilbertQ_[c][s][1] = yQ; xQ = yQ;
                    }
                    inArr[c] = xI * cosC - ((c == 0) ? (xQ * sinC) : (-xQ * sinC));
                }
                sigL = inArr[0];
                sigR = inArr[1];
                return;
            }
            case 2: { // 2: Granular Pitch Shifter (-12 .. +12 st)
                miniBufL_[miniWrite_ & kMiniMask] = sigL;
                miniBufR_[miniWrite_ & kMiniMask] = sigR;
                const float semitones = std::round((u - 0.5f) * 24.0f);
                const float ratio = FastMath::fastPow2(semitones * (1.0f / 12.0f));
                const float rate = (1.0f - ratio) / 1024.0f;
                grainPhase_ += rate;
                grainPhase_ -= std::floor(grainPhase_);
                const float p2 = grainPhase_ + 0.5f - std::floor(grainPhase_ + 0.5f);
                const float w1 = 0.5f * (1.0f - FastMath::fastCosNorm(grainPhase_));
                const float w2 = 0.5f * (1.0f - FastMath::fastCosNorm(p2));
                const int d1 = static_cast<int>(grainPhase_ * 1024.0f) + 4;
                const int d2 = static_cast<int>(p2 * 1024.0f) + 4;
                sigL = w1 * miniBufL_[(miniWrite_ - d1) & kMiniMask] + w2 * miniBufL_[(miniWrite_ - d2) & kMiniMask];
                sigR = w1 * miniBufR_[(miniWrite_ - d1) & kMiniMask] + w2 * miniBufR_[(miniWrite_ - d2) & kMiniMask];
                return;
            }
            case 3: { // 3: Swedish KlangSaw (HM-2 Dead-Band + 1050 Hz Gyrator)
                const float drive = 1.0f + u * 8.0f;
                const float vth = 0.08f * u;
                auto hm2Clip = [drive, vth](float x) noexcept {
                    float v = FastMath::fastTanh(x * drive);
                    if (std::abs(v) < vth) return 0.0f;
                    return (v > 0.0f) ? (v - vth) : (v + vth);
                };
                chainsawBellL_.setBell(1050.0f, 2.4f, 4.0f + 12.0f * u, invSr_);
                chainsawBellR_.setBell(1050.0f, 2.4f, 4.0f + 12.0f * u, invSr_);
                sigL = chainsawBellL_.process(hm2Clip(sigL)) * 0.7f;
                sigR = chainsawBellR_.process(hm2Clip(sigR)) * 0.7f;
                chainsawBellL_.clampState();
                chainsawBellR_.clampState();
                return;
            }
            case 4: { // 4: Buchla Phase-Fold (Recursive Sine Wavefolder)
                const float folds = 1.0f + 4.5f * u * u;
                const float pL = sigL * folds * 0.35f;
                const float pR = sigR * folds * 0.35f;
                sigL = FastMath::fastSinNorm(pL - std::floor(pL)) * 0.85f;
                sigR = FastMath::fastSinNorm(pR - std::floor(pR)) * 0.85f;
                return;
            }
            case 5:
            default: { // 5: Pipe & Wire Modal Resonator
                const float resHz = Taper::profileD_Cutoff(u, 60.0f, 1600.0f);
                const int combDelay = std::clamp(static_cast<int>(sampleRate_ / resHz), 8, kMiniBufLen - 8);
                const float fbL = miniBufL_[(miniWrite_ - combDelay) & kMiniMask];
                const float fbR = miniBufR_[(miniWrite_ - combDelay) & kMiniMask];
                sigL = FastMath::fastTanh(sigL + 0.68f * fbL);
                sigR = FastMath::fastTanh(sigR + 0.68f * fbR);
                miniBufL_[miniWrite_ & kMiniMask] = sigL;
                miniBufR_[miniWrite_ & kMiniMask] = sigR;
                return;
            }
        }
    }

    inline void processSlot2(int engine, float depth, float& sigL, float& sigR, float& outWowOffsetSamples) noexcept {
        outWowOffsetSamples = 0.0f;
        const float u = std::clamp(depth, 0.0f, 1.0f);
        switch (engine) {
            case 0: // Clean Bypass
                return;
            case 1: { // 1: Analog Slop / Tape Wow & Flutter
                wowPhase1_ += 0.55f * invSr_;
                if (wowPhase1_ >= 1.0f) wowPhase1_ -= 1.0f;
                wowPhase2_ += 2.35f * invSr_;
                if (wowPhase2_ >= 1.0f) wowPhase2_ -= 1.0f;
                const float wow = FastMath::fastSinNorm(wowPhase1_) + 0.35f * FastMath::fastSinNorm(wowPhase2_);
                outWowOffsetSamples = wow * (u * u * 85.0f);
                return;
            }
            case 2: { // 2: Tape Damping / HF Loss (18 kHz down to 400 Hz)
                const float cutoffHz = Taper::profileD_Cutoff(1.0f - u * 0.92f, 400.0f, 18000.0f);
                dampFilterL_.setCutoff(cutoffHz, invSr_);
                dampFilterR_.setCutoff(cutoffHz, invSr_);
                sigL = dampFilterL_.processLP(sigL);
                sigR = dampFilterR_.processLP(sigR);
                return;
            }
            case 3: { // 3: Bitcrush & Slew Decimator
                const float step = FastMath::fastPow2(-u * 4.0f);
                srrPhase_ += step;
                if (srrPhase_ >= 1.0f) {
                    srrPhase_ -= 1.0f;
                    const float qSteps = FastMath::fastPow2(15.0f - u * 12.5f);
                    const float invQ = 1.0f / qSteps;
                    crushHoldL_ = std::round(sigL * qSteps) * invQ;
                    crushHoldR_ = std::round(sigR * qSteps) * invQ;
                }
                sigL = crushHoldL_;
                sigR = crushHoldR_;
                return;
            }
            case 4:
            default: { // 4: Juno BBD Stereo Chorus
                wowPhase1_ += 0.65f * invSr_;
                if (wowPhase1_ >= 1.0f) wowPhase1_ -= 1.0f;
                outWowOffsetSamples = FastMath::fastSinNorm(wowPhase1_) * (u * 45.0f);
                return;
            }
        }
    }

    inline void processSamplePair(float& ioL, float& ioR) noexcept {
        const float dryL = ioL;
        const float dryR = ioR;

        // 0: TIME / RATE (5 ms .. 1300 ms)
        const float timeKnob = std::clamp(params8[0], 0.0f, 1.0f);
        const float baseDelayMs = 5.0f + timeKnob * timeKnob * 1295.0f;
        const float baseSamples = baseDelayMs * 0.001f * sampleRate_;

        // 1: STEREO / PING-PONG (-1.0 .. +1.0)
        const float stereoBipolar = (std::clamp(params8[1], 0.0f, 1.0f) - 0.5f) * 2.0f;
        const float pingPongCross = (stereoBipolar < 0.0f) ? (-stereoBipolar) : 0.0f;
        const float haasOffsetSamples = (stereoBipolar > 0.0f) ? (stereoBipolar * 0.020f * sampleRate_) : 0.0f;

        smoothDelayL_ += 0.002f * (baseSamples - smoothDelayL_);
        smoothDelayR_ += 0.002f * ((baseSamples + haasOffsetSamples) - smoothDelayR_);

        // 2 & 3: SLOT 1 ENGINE & INTENSITY
        const int slot1Engine = std::clamp(static_cast<int>(params8[2] * 5.99f), 0, 5);
        const float slot1Intensity = params8[3];

        // 4 & 5: SLOT 2 ENGINE & DEPTH
        const int slot2Engine = std::clamp(static_cast<int>(params8[4] * 4.99f), 0, 4);
        const float slot2Depth = params8[5];

        // 6: FEEDBACK & FREEZE
        const float fbKnob = std::clamp(params8[6], 0.0f, 1.0f);
        const bool frozen = manualFreeze_ || (fbKnob >= 0.96f);
        const float fbGain = frozen ? 1.0f : Taper::profileF_Hyperbolic(fbKnob, 0.0f, 1.15f);

        // Read delay taps
        const float tapL = readHermite(delayBufL_, writePos_, smoothDelayL_ + lastWowOffset_);
        const float tapR = readHermite(delayBufR_, writePos_, smoothDelayR_ - lastWowOffset_);

        // Pass feedback taps through Slot 1 -> Slot 2 Recursive Insert Mill
        float loopL = tapL;
        float loopR = tapR;
        processSlot1(slot1Engine, slot1Intensity, loopL, loopR);
        processSlot2(slot2Engine, slot2Depth, loopL, loopR, lastWowOffset_);

        // Padé fastTanh Loop Limiter & DC Blocker
        loopL = dcBlockL_.process(FastMath::fastTanh(loopL * fbGain));
        loopR = dcBlockR_.process(FastMath::fastTanh(loopR * fbGain));

        // Ping-Pong Crossfeed Matrix
        const float nextL = (frozen ? 0.0f : dryL) + (1.0f - pingPongCross) * loopL + pingPongCross * loopR;
        const float nextR = (frozen ? 0.0f : dryR) + (1.0f - pingPongCross) * loopR + pingPongCross * loopL;

        delayBufL_[writePos_ & kDelayMask] = std::clamp(std::isfinite(nextL) ? nextL : 0.0f, -4.0f, 4.0f);
        delayBufR_[writePos_ & kDelayMask] = std::clamp(std::isfinite(nextR) ? nextR : 0.0f, -4.0f, 4.0f);
        ++writePos_;
        ++miniWrite_;

        float dryGain = 1.0f;
        float wetGain = 0.0f;
        Taper::profileM_TriZoneMix(params8[7], phaseInvert, dryGain, wetGain);

        ioL = dryL * dryGain + tapL * wetGain;
        ioR = dryR * dryGain + tapR * wetGain;
    }

    float delayBufL_[kMaxDelaySamples]{};
    float delayBufR_[kMaxDelaySamples]{};
    float miniBufL_[kMiniBufLen]{};
    float miniBufR_[kMiniBufLen]{};
    int writePos_ = 0;
    int miniWrite_ = 0;

    float sampleRate_ = 48000.0f;
    float invSr_ = 1.0f / 48000.0f;
    float smoothDelayL_ = 12000.0f;
    float smoothDelayR_ = 12000.0f;
    float lastWowOffset_ = 0.0f;
    bool manualFreeze_ = false;

    float bodePhase_ = 0.0f;
    float hilbertI_[2][2][2]{};
    float hilbertQ_[2][2][2]{};
    float grainPhase_ = 0.0f;
    float wowPhase1_ = 0.0f;
    float wowPhase2_ = 0.25f;
    float srrPhase_ = 0.0f;
    float crushHoldL_ = 0.0f;
    float crushHoldR_ = 0.0f;

    DSPCommon::CytomicSVF chainsawBellL_;
    DSPCommon::CytomicSVF chainsawBellR_;
    DSPCommon::OnePoleTPT dampFilterL_;
    DSPCommon::OnePoleTPT dampFilterR_;
    DSPCommon::DCBlocker dcBlockL_;
    DSPCommon::DCBlocker dcBlockR_;
};

using KlangLoopMill = EchoMill;
using TKMD = EchoMill;

} // namespace TbdAudio::Spatial
