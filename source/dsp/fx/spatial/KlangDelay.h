#pragma once

#include "../../common/AudioEffect.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace TbdAudio::Spatial {

/**
 * @brief KlangDelay — Mega-Effect 7: Stereo Echo, Delay & Repetition Lab.
 *        Houses 6 stereo delay engines with zero-allocation circular buffers:
 *          0: Digital Stereo Ping-Pong (Clean cross-bouncing stereo taps)
 *          1: Green Tape Echo (Roland Space Echo RE-201 multi-head flutter & tape saturation)
 *          2: Deluxe Memory BBD (Panasonic MN3005 dark analog bucket-brigade with vibrato/chorus)
 *          3: Reverse Delay Buffer (Dual-window Hann-crossfaded reverse read playhead)
 *          4: Granular Cloud Freeze (Multi-tap diffused micro-grain delay cluster)
 *          5: Oil Can Delay (Tel-Ray Adineko electrostatic dark fluid wobble)
 *
 * 8-Knob Parameter Topology:
 *   - Knob 0 (TIME / DIV)    : [0..1] 5 ms .. 1200 ms base delay time
 *   - Knob 1 (FEEDBACK)      : [0..1] 0% .. 115% Profile F Hyperbolic runaway (stabilized by fastTanh)
 *   - Knob 2 (TONE / DAMP)   : [0..1] Combined high-cut & low-cut feedback loop damping
 *   - Knob 3 (ALGORITHM)     : [0..1] Discrete 6-position delay engine selector (0..5)
 *   - Knob 4 (MOD / FLUTTER) : [0..1] Tape wow/flutter & BBD chorus depth/rate
 *   - Knob 5 (STEREO OFFSET) : [0..1] Ping-pong cross-bounce & L/R ratio offset
 *   - Knob 6 (DRIVE)         : [0..1] Analog tape / BBD preamp & loop saturation drive
 *   - Knob 7 (MIX / ∅)       : [0..1] Profile M Tri-Zone Mix (0.0 = Wet, 0.5 = Dry, 1.0 = 50/50)
 */
class KlangDelay final : public AudioEffect8 {
public:
    KlangDelay() noexcept {
        setParam(0, 0.35f); // TIME (~320 ms)
        setParam(1, 0.45f); // FEEDBACK
        setParam(2, 0.65f); // TONE / DAMP
        setParam(3, 0.0f);  // ALGORITHM 0 = Digital Stereo Ping-Pong
        setParam(4, 0.25f); // MOD / FLUTTER
        setParam(5, 0.50f); // STEREO OFFSET
        setParam(6, 0.20f); // DRIVE
        setParam(7, 0.0f);  // MIX = 100% Wet per Profile M
    }

    ~KlangDelay() override = default;

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
        std::fill(std::begin(bufL_), std::end(bufL_), 0.0f);
        std::fill(std::begin(bufR_), std::end(bufR_), 0.0f);
        writeIdx_ = 0;
        wowPhase_ = 0.0f;
        flutterPhase_ = 0.0f;
        revPhase_ = 0.0f;
        lpL_.reset();
        lpR_.reset();
        hpL_.reset();
        hpR_.reset();
        oilResL_.reset();
        oilResR_.reset();
        dcL_.reset();
        dcR_.reset();
    }

    inline void setParam8(int index, float value) noexcept {
        setParam(index, value);
    }

    void processSampleStereo(float inL, float inR, float& outL, float& outR) noexcept {
        const float timeNorm   = std::clamp(params8[0], 0.0f, 1.0f);
        const float fbNorm     = std::clamp(params8[1], 0.0f, 1.0f);
        const float toneNorm   = std::clamp(params8[2], 0.0f, 1.0f);
        const float algoNorm   = std::clamp(params8[3], 0.0f, 1.0f);
        const float modNorm    = std::clamp(params8[4], 0.0f, 1.0f);
        const float offsetNorm = std::clamp(params8[5], 0.0f, 1.0f);
        const float driveNorm  = std::clamp(params8[6], 0.0f, 1.0f);
        const float mixNorm    = std::clamp(params8[7], 0.0f, 1.0f);

        float dryGain = 0.0f, wetGain = 1.0f;
        Taper::profileM_TriZoneMix(mixNorm, phaseInvert, dryGain, wetGain);
        if (wetGain == 0.0f) {
            outL = inL * dryGain;
            outR = inR * dryGain;
            return;
        }

        const int algo = std::clamp(static_cast<int>(algoNorm * 5.0f + 0.5f), 0, 5);

        // Delay time (5 ms .. 1200 ms with quadratic curve for fine slapback resolution)
        const float baseMs = 5.0f + 1195.0f * (timeNorm * timeNorm);
        const float baseSamples = std::clamp(baseMs * 0.001f * sampleRate_, 4.0f, static_cast<float>(kMaxDelayBuf - 256));

        // Stereo offset ratio (1.0 at 0.0 -> 0.75 dotted-8th at 0.5 -> 0.618 golden ratio at 1.0)
        const float ratioR = 1.0f - 0.382f * offsetNorm;
        const float delaySamplesL = baseSamples;
        const float delaySamplesR = std::max(4.0f, baseSamples * ratioR);

        // Dual LFOs: slow Wow (0.4 .. 2.2 Hz) and fast Flutter (4.5 .. 14 Hz)
        const float wowRate = 0.4f + 1.8f * modNorm;
        const float flutterRate = 4.5f + 9.5f * modNorm;
        wowPhase_ += wowRate * invSr_;
        if (wowPhase_ >= 1.0f) wowPhase_ -= 1.0f;
        flutterPhase_ += flutterRate * invSr_;
        if (flutterPhase_ >= 1.0f) flutterPhase_ -= 1.0f;

        const float wowL = FastMath::fastSinNorm(wowPhase_);
        const float wowR = FastMath::fastCosNorm(wowPhase_);
        const float flut = FastMath::fastSinNorm(flutterPhase_);
        const float modDepthSamples = (modNorm * modNorm) * (0.0035f * sampleRate_);
        const float modOffsetL = modDepthSamples * (0.75f * wowL + 0.25f * flut);
        const float modOffsetR = modDepthSamples * (0.75f * wowR - 0.25f * flut);

        // Read taps according to selected Delay Engine
        float tapL = 0.0f;
        float tapR = 0.0f;

        switch (algo) {
            case 0: { // Digital Stereo Ping-Pong
                tapL = readFractional(bufL_, delaySamplesL + modOffsetL * 0.25f);
                tapR = readFractional(bufR_, delaySamplesR + modOffsetR * 0.25f);
                break;
            }
            case 1: { // Green Tape Echo (RE-201 3-head multi-tap + wow/flutter)
                const float d1 = delaySamplesL + modOffsetL;
                const float d2 = delaySamplesL * 0.666f + modOffsetR;
                const float d3 = delaySamplesR * 0.333f - modOffsetL * 0.5f;
                tapL = 0.55f * readFractional(bufL_, d1) + 0.30f * readFractional(bufL_, d2) + 0.15f * readFractional(bufR_, d3);
                tapR = 0.55f * readFractional(bufR_, delaySamplesR + modOffsetR) + 0.30f * readFractional(bufR_, d2) + 0.15f * readFractional(bufL_, d3);
                break;
            }
            case 2: { // Deluxe Memory BBD (MN3005 dark analog bucket-brigade)
                const float bbdModL = modOffsetL * 1.6f;
                const float bbdModR = modOffsetR * 1.6f;
                tapL = readFractional(bufL_, delaySamplesL + bbdModL);
                tapR = readFractional(bufR_, delaySamplesR + bbdModR);
                break;
            }
            case 3: { // Reverse Delay Buffer (Dual-window Hann-crossfaded reverse playhead)
                const float winLen = std::clamp(delaySamplesL, 128.0f, 32768.0f);
                revPhase_ += 1.0f / winLen;
                if (revPhase_ >= 1.0f) revPhase_ -= 1.0f;
                const float p0 = revPhase_;
                const float p1 = (p0 + 0.5f >= 1.0f) ? (p0 - 0.5f) : (p0 + 0.5f);
                // Reading backwards means delay increases at 2x rate relative to write head
                const float revDelay0 = 4.0f + p0 * winLen;
                const float revDelay1 = 4.0f + p1 * winLen;
                const float w0 = 0.5f * (1.0f - FastMath::fastCosNorm(p0));
                const float w1 = 0.5f * (1.0f - FastMath::fastCosNorm(p1));
                tapL = w0 * readFractional(bufL_, revDelay0) + w1 * readFractional(bufL_, revDelay1);
                tapR = w0 * readFractional(bufR_, revDelay0 * ratioR) + w1 * readFractional(bufR_, revDelay1 * ratioR);
                break;
            }
            case 4: { // Granular Cloud Freeze (4 diffused micro-grain taps)
                const float g1 = delaySamplesL * 0.41f + modOffsetL * 1.4f;
                const float g2 = delaySamplesL * 0.73f - modOffsetR * 1.2f;
                const float g3 = delaySamplesR * 0.89f + modOffsetR * 1.5f;
                const float g4 = delaySamplesL + modOffsetL;
                tapL = 0.35f * readFractional(bufL_, g4) + 0.25f * readFractional(bufR_, g1)
                     + 0.25f * readFractional(bufL_, g2) + 0.15f * readFractional(bufR_, g3);
                tapR = 0.35f * readFractional(bufR_, g3) + 0.25f * readFractional(bufL_, g1)
                     + 0.25f * readFractional(bufR_, g2) + 0.15f * readFractional(bufL_, g4);
                break;
            }
            case 5:
            default: { // Oil Can Delay (Tel-Ray Adineko dark fluid warble)
                const float oilModL = modDepthSamples * 2.2f * (wowL + 0.4f * flut);
                const float oilModR = modDepthSamples * 2.2f * (wowR - 0.4f * flut);
                oilResL_.setBandpass(750.0f + 450.0f * toneNorm, 1.3f, invSr_);
                oilResR_.setBandpass(820.0f + 450.0f * toneNorm, 1.3f, invSr_);
                tapL = oilResL_.process(readFractional(bufL_, delaySamplesL + oilModL));
                tapR = oilResR_.process(readFractional(bufR_, delaySamplesR + oilModR));
                break;
            }
        }

        // Loop Tone / Damping (BBD & Tape naturally darken highs and roll off sub mud)
        float lpCutoff = Taper::profileD_Cutoff(toneNorm, 650.0f, 16000.0f);
        if (algo == 2) lpCutoff = std::min(lpCutoff, 3800.0f); // MN3005 BBD anti-aliasing filter
        const float hpCutoff = (algo == 1 || algo == 5) ? 110.0f : (30.0f + 180.0f * (1.0f - toneNorm));
        lpL_.setCutoff(lpCutoff, invSr_);
        lpR_.setCutoff(lpCutoff, invSr_);
        hpL_.setCutoff(hpCutoff, invSr_);
        hpR_.setCutoff(hpCutoff, invSr_);

        const float dampedL = hpL_.processHP(lpL_.processLP(tapL));
        const float dampedR = hpR_.processHP(lpR_.processLP(tapR));

        // Feedback with Profile F Hyperbolic end-stop (0% .. 115% runaway) + Ping-Pong Crossfeed
        const float fbGain = Taper::profileF_Hyperbolic(fbNorm, 0.0f, 1.15f);
        const float cross = 0.65f * offsetNorm;
        const float fbMixL = (1.0f - cross) * dampedL + cross * dampedR;
        const float fbMixR = (1.0f - cross) * dampedR + cross * dampedL;

        // Analog Preamp & Loop Saturation Drive
        const float driveGain = 1.0f + driveNorm * 3.5f;
        const float nextL = FastMath::fastTanh((inL + fbMixL * fbGain) * driveGain);
        const float nextR = FastMath::fastTanh((inR * (1.0f - 0.5f * cross) + fbMixR * fbGain) * driveGain);

        bufL_[writeIdx_] = dcL_.process(nextL);
        bufR_[writeIdx_] = dcR_.process(nextR);
        writeIdx_ = (writeIdx_ + 1) & kDelayMask;

        outL = std::clamp(inL * dryGain + dampedL * wetGain, -4.0f, 4.0f);
        outR = std::clamp(inR * dryGain + dampedR * wetGain, -4.0f, 4.0f);
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
    static constexpr int kMaxDelayBuf = 65536;
    static constexpr int kDelayMask   = kMaxDelayBuf - 1;

    [[nodiscard]] inline float readFractional(const float* buf, float delaySamples) const noexcept {
        const float clamped = std::clamp(delaySamples, 1.0f, static_cast<float>(kMaxDelayBuf - 4));
        const int intPart = static_cast<int>(clamped);
        const float frac = clamped - static_cast<float>(intPart);
        const int idx0 = (writeIdx_ - intPart + kMaxDelayBuf) & kDelayMask;
        const int idx1 = (idx0 - 1 + kMaxDelayBuf) & kDelayMask;
        return buf[idx0] + frac * (buf[idx1] - buf[idx0]);
    }

    float sampleRate_ = 48000.0f;
    float invSr_ = 1.0f / 48000.0f;

    float bufL_[kMaxDelayBuf]{};
    float bufR_[kMaxDelayBuf]{};
    int writeIdx_ = 0;

    float wowPhase_ = 0.0f;
    float flutterPhase_ = 0.0f;
    float revPhase_ = 0.0f;

    DSPCommon::OnePoleTPT lpL_{};
    DSPCommon::OnePoleTPT lpR_{};
    DSPCommon::OnePoleTPT hpL_{};
    DSPCommon::OnePoleTPT hpR_{};
    DSPCommon::CytomicSVF oilResL_{};
    DSPCommon::CytomicSVF oilResR_{};
    DSPCommon::DCBlocker dcL_{};
    DSPCommon::DCBlocker dcR_{};
    BlockContext dummyCtx_{};
};

} // namespace TbdAudio::Spatial
