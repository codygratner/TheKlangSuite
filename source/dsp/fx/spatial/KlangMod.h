#pragma once

#include <cmath>
#include <cstdint>
#include <algorithm>
#include <cstring>
#include "../../common/AudioEffect.h"

namespace TbdAudio::Spatial {

/**
 * Mega-Effect 5: KlangMod (Chorus, Ensemble & Modulation Lab)
 *
 * 8-Engine Stereo Modulation Processor:
 *   0: Clean Digital Stereo Chorus (Quadrature L/R sine LFOs)
 *   1: Vintage BBD Chorus (Ensemble 60 Modes I, II, I+II anti-phase triangle LFOs + MN3009 fastTanh warmth)
 *   2: Swept Hoover Chorus (Dual-rate stereo swept chorus)
 *   3: 16-Voice Hyper-Ensemble (Multi-tap detuned stereo chorus voices, 8 per channel)
 *   4: Spatial Dimension BBD (Stereo Dimension cross-channel high-passed phase-inverted spatial expansion)
 *   5: Through-Zero Flanger (Fixed 5ms reference tap + modulated tap sweeping 0ms..10ms through zero)
 *   6: 8-Stage Optical Phaser (8 cascaded 1st-order all-pass stages per channel swept by LFO + feedback)
 *   7: Harmonic Tremolo (500 Hz OnePoleTPT low/high crossover split modulated 180 degrees out of phase)
 *
 * Performance Knobs (params8[0..7]):
 *   0: RATE         (0.05 Hz .. 20.0 Hz)
 *   1: DEPTH        (0% .. 100%)
 *   2: FEEDBACK     (-95% .. +95% with fastTanh saturation clamp)
 *   3: ENGINE       (0..7 discrete selector)
 *   4: STEREO WIDTH (0% .. 200% Mid/Side width matrix on wet signal)
 *   5: TONE / COLOR (800 Hz .. 18 kHz BBD low-pass damping filter via OnePoleTPT)
 *   6: PRE-DELAY    (0 ms .. 20 ms base delay offset)
 *   7: MIX / ∅      (Taper::profileM_TriZoneMix: 0.0=100% Wet, 0.5=100% Dry, 1.0=100% Dry+Wet)
 */
class KlangMod final : public AudioEffect8 {
public:
    static constexpr int kMaxModBuf = 4096;
    static constexpr int kBufferMask = kMaxModBuf - 1;
    static constexpr int kNumPhaserStages = 8;

    KlangMod() noexcept {
        // Default knob states: Mix = 0.0f (100% Wet per Profile M default)
        setParam8(0, 0.25f); // Rate
        setParam8(1, 0.55f); // Depth
        setParam8(2, 0.50f); // Feedback (0% at center)
        setParam8(3, 0.00f); // Engine 0
        setParam8(4, 0.50f); // Stereo Width (100% normal stereo at 0.5)
        setParam8(5, 0.75f); // Tone / Color
        setParam8(6, 0.15f); // Pre-Delay
        setParam8(7, 0.00f); // Mix (0.0 = 100% Wet)
        reset();
    }

    ~KlangMod() override = default;

    void init(const BlockContext& ctx) override {
        prepare(ctx.sampleRate > 0.0 ? ctx.sampleRate : 44100.0, 512);
    }

    void prepare(double sampleRate, int /*maxBlockSize*/ = 512) noexcept {
        sr_ = static_cast<float>(std::clamp(sampleRate, 8000.0, 192000.0));
        invSr_ = 1.0f / sr_;
        reset();
        updateFilters();
    }

    void reset() noexcept override {
        std::memset(delayL_, 0, sizeof(delayL_));
        std::memset(delayR_, 0, sizeof(delayR_));
        writePos_ = 0;

        lfoPhase1_ = 0.0f;
        lfoPhase2_ = 0.37f;
        lfoPhase3_ = 0.71f;

        fbStateL_ = 0.0f;
        fbStateR_ = 0.0f;

        toneFilterL_.reset();
        toneFilterR_.reset();
        dimHpL_.reset();
        dimHpR_.reset();
        tremFilterL_.reset();
        tremFilterR_.reset();
        dcBlockL_.init(sr_);
        dcBlockR_.init(sr_);
        dcBlockL_.reset();
        dcBlockR_.reset();

        for (int i = 0; i < kNumPhaserStages; ++i) {
            apStateL_[i] = 0.0f;
            apStateR_[i] = 0.0f;
        }
    }

    void setParam8(int idx, float val) noexcept {
        setParam(idx, val);
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        if (!buffer || numSamples <= 0) return;
        FastMath::enableFTZDAZ();
        if (ctx.sampleRate > 0.0f && std::abs(ctx.sampleRate - sr_) > 1.0f) {
            sr_ = std::clamp(ctx.sampleRate, 8000.0f, 192000.0f);
            invSr_ = 1.0f / sr_;
            dcBlockL_.init(sr_);
            dcBlockR_.init(sr_);
        }

        updateParameters();

        for (int i = 0; i < numSamples; ++i) {
            float inL = buffer[i];
            float inR = buffer[i];
            float outL = 0.0f;
            float outR = 0.0f;
            processSamplePair(inL, inR, outL, outR);
            buffer[i] = std::clamp(0.5f * (outL + outR), -4.0f, 4.0f);
        }

        FastMath::sanitizeBuffer(buffer, numSamples);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& ctx) override {
        if (!left || !right || numSamples <= 0) return;
        FastMath::enableFTZDAZ();
        if (ctx.sampleRate > 0.0f && std::abs(ctx.sampleRate - sr_) > 1.0f) {
            sr_ = std::clamp(ctx.sampleRate, 8000.0f, 192000.0f);
            invSr_ = 1.0f / sr_;
            dcBlockL_.init(sr_);
            dcBlockR_.init(sr_);
        }

        updateParameters();

        for (int i = 0; i < numSamples; ++i) {
            float outL = 0.0f;
            float outR = 0.0f;
            processSamplePair(left[i], right[i], outL, outR);
            left[i]  = std::clamp(outL, -4.0f, 4.0f);
            right[i] = std::clamp(outR, -4.0f, 4.0f);
        }

        FastMath::sanitizeBuffer(left, numSamples);
        FastMath::sanitizeBuffer(right, numSamples);
    }

private:
    float delayL_[kMaxModBuf]{};
    float delayR_[kMaxModBuf]{};
    int writePos_ = 0;

    float sr_ = 44100.0f;
    float invSr_ = 1.0f / 44100.0f;

    // LFO phases in [0, 1)
    float lfoPhase1_ = 0.0f;
    float lfoPhase2_ = 0.37f;
    float lfoPhase3_ = 0.71f;

    // Cached parameters
    float rateHz_ = 0.8f;
    float depth_ = 0.55f;
    float feedback_ = 0.0f;
    int engine_ = 0;
    float stereoWidth_ = 1.0f;
    float toneHz_ = 8000.0f;
    float preDelayMs_ = 2.0f;
    float dryGain_ = 0.0f;
    float wetGain_ = 1.0f;

    // Feedback & filter states
    float fbStateL_ = 0.0f;
    float fbStateR_ = 0.0f;
    DSPCommon::OnePoleTPT toneFilterL_;
    DSPCommon::OnePoleTPT toneFilterR_;
    DSPCommon::OnePoleTPT dimHpL_;
    DSPCommon::OnePoleTPT dimHpR_;
    DSPCommon::OnePoleTPT tremFilterL_;
    DSPCommon::OnePoleTPT tremFilterR_;
    DSPCommon::DCBlocker dcBlockL_;
    DSPCommon::DCBlocker dcBlockR_;

    float apStateL_[kNumPhaserStages]{};
    float apStateR_[kNumPhaserStages]{};

    [[nodiscard]] static inline float wrapPhase(float p) noexcept {
        p -= static_cast<float>(static_cast<int>(p));
        if (p < 0.0f) p += 1.0f;
        return p;
    }

    // Bipolar triangle LFO [-1.0, +1.0] from normalized phase [0, 1)
    [[nodiscard]] static inline float triangleWave(float phase) noexcept {
        float p = wrapPhase(phase);
        return 1.0f - 4.0f * std::abs(p - 0.5f);
    }

    // Linear fractional delay read helper
    [[nodiscard]] inline float readDelayLinear(const float* buf, float delaySamples) const noexcept {
        float d = std::clamp(delaySamples, 0.0f, static_cast<float>(kMaxModBuf - 4));
        float readIdx = static_cast<float>(writePos_) - d;
        while (readIdx < 0.0f) {
            readIdx += static_cast<float>(kMaxModBuf);
        }
        int idx0 = static_cast<int>(readIdx) & kBufferMask;
        int idx1 = (idx0 + 1) & kBufferMask;
        float frac = readIdx - static_cast<float>(static_cast<int>(readIdx));
        return buf[idx0] + frac * (buf[idx1] - buf[idx0]);
    }

    inline void updateFilters() noexcept {
        toneFilterL_.setCutoff(toneHz_, invSr_);
        toneFilterR_.setCutoff(toneHz_, invSr_);
        dimHpL_.setCutoff(280.0f, invSr_);
        dimHpR_.setCutoff(280.0f, invSr_);
        tremFilterL_.setCutoff(500.0f, invSr_);
        tremFilterR_.setCutoff(500.0f, invSr_);
    }

    inline void updateParameters() noexcept {
        // 0: RATE (0.05 Hz .. 20.0 Hz)
        rateHz_ = Taper::profileD_Cutoff(params8[0], 0.05f, 20.0f);
        rateHz_ = std::clamp(rateHz_, 0.05f, 20.0f);

        // 1: DEPTH (0% .. 100%)
        depth_ = std::clamp(params8[1], 0.0f, 1.0f);

        // 2: FEEDBACK (-95% .. +95%)
        feedback_ = std::clamp(Taper::profileB_Bipolar(params8[2]) * 0.95f, -0.95f, 0.95f);

        // 3: ENGINE (0..7 discrete selector)
        engine_ = std::clamp(static_cast<int>(params8[3] * 7.999f), 0, 7);

        // 4: STEREO WIDTH (0% .. 200%)
        stereoWidth_ = std::clamp(params8[4] * 2.0f, 0.0f, 2.0f);

        // 5: TONE / COLOR (800 Hz .. 18 kHz)
        toneHz_ = std::clamp(Taper::profileD_Cutoff(params8[5], 800.0f, 18000.0f), 800.0f, 18000.0f);

        // 6: PRE-DELAY (0 ms .. 20 ms)
        preDelayMs_ = std::clamp(params8[6] * 20.0f, 0.0f, 20.0f);

        // 7: MIX / ∅ (Profile M Tri-Zone Mix)
        Taper::profileM_TriZoneMix(params8[7], phaseInvert, dryGain_, wetGain_);

        updateFilters();
    }

    inline void processSamplePair(float inL, float inR, float& outL, float& outR) noexcept {
        // Advance primary and secondary LFO phases
        float inc1 = rateHz_ * invSr_;
        lfoPhase1_ = wrapPhase(lfoPhase1_ + inc1);
        lfoPhase2_ = wrapPhase(lfoPhase2_ + inc1 * 3.15f + 0.4f * invSr_);
        lfoPhase3_ = wrapPhase(lfoPhase3_ + inc1 * 0.618f);

        // Write input + saturated feedback into circular delay buffers
        float fbL = FastMath::fastTanh(fbStateL_ * feedback_);
        float fbR = FastMath::fastTanh(fbStateR_ * feedback_);

        delayL_[writePos_] = std::clamp(inL + fbL, -4.0f, 4.0f);
        delayR_[writePos_] = std::clamp(inR + fbR, -4.0f, 4.0f);

        float msToSamples = sr_ * 0.001f;
        float preDelaySamps = preDelayMs_ * msToSamples;

        float wetL = 0.0f;
        float wetR = 0.0f;
        float dryRefL = inL;
        float dryRefR = inR;

        switch (engine_) {
            case 0: {
                // 0: Clean Digital Stereo Chorus (Quadrature L/R sine LFOs)
                float modL = FastMath::fastSinNorm(lfoPhase1_);
                float modR = FastMath::fastCosNorm(lfoPhase1_);
                float baseMs = 7.0f + preDelayMs_;
                float depthMs = 5.0f * depth_;
                float dL = (baseMs + depthMs * modL) * msToSamples;
                float dR = (baseMs + depthMs * modR) * msToSamples;
                wetL = readDelayLinear(delayL_, dL);
                wetR = readDelayLinear(delayR_, dR);
                break;
            }

            case 1: {
                // 1: Vintage BBD Chorus (Ensemble 60 Modes I, II, I+II anti-phase triangle LFOs + MN3009 warmth)
                float tri1 = triangleWave(lfoPhase1_);
                float tri2 = triangleWave(lfoPhase2_);
                // Blend Mode I (slow), Mode II (faster), or Mode I+II based on depth
                float lfoL = (depth_ < 0.65f) ? tri1 : (0.65f * tri1 + 0.35f * tri2);
                float lfoR = (depth_ < 0.65f) ? -tri1 : (-0.65f * tri1 + 0.35f * tri2);
                float baseMs = 3.4f + preDelayMs_ * 0.5f;
                float depthMs = 2.2f * depth_;
                float dL = (baseMs + depthMs * lfoL) * msToSamples;
                float dR = (baseMs + depthMs * lfoR) * msToSamples;
                // MN3009 BBD soft saturation warmth
                wetL = FastMath::fastTanh(readDelayLinear(delayL_, dL) * 1.25f) * 0.85f;
                wetR = FastMath::fastTanh(readDelayLinear(delayR_, dR) * 1.25f) * 0.85f;
                break;
            }

            case 2: {
                // 2: Swept Hoover Chorus (Dual-rate stereo swept chorus)
                float slowL = FastMath::fastSinNorm(lfoPhase1_);
                float slowR = FastMath::fastSinNorm(wrapPhase(lfoPhase1_ + 0.5f));
                float fastL = FastMath::fastSinNorm(lfoPhase2_);
                float fastR = FastMath::fastCosNorm(lfoPhase2_);
                float baseMs = 5.0f + preDelayMs_;
                float depthSlow = 4.5f * depth_;
                float depthFast = 2.2f * depth_;
                float dL1 = (baseMs + depthSlow * slowL) * msToSamples;
                float dL2 = (baseMs * 1.3f + depthFast * fastL) * msToSamples;
                float dR1 = (baseMs + depthSlow * slowR) * msToSamples;
                float dR2 = (baseMs * 1.3f + depthFast * fastR) * msToSamples;
                wetL = 0.6f * (readDelayLinear(delayL_, dL1) + readDelayLinear(delayL_, dL2));
                wetR = 0.6f * (readDelayLinear(delayR_, dR1) + readDelayLinear(delayR_, dR2));
                break;
            }

            case 3: {
                // 3: 16-Voice Hyper-Ensemble (8 detuned taps per channel = 16 stereo voices)
                static constexpr float kPhaseOffsets[8] = {
                    0.000f, 0.125f, 0.250f, 0.375f, 0.500f, 0.625f, 0.750f, 0.875f
                };
                static constexpr float kBaseTapMs[8] = {
                    2.7f, 4.1f, 5.8f, 7.3f, 9.1f, 11.2f, 13.5f, 16.0f
                };
                float sumL = 0.0f;
                float sumR = 0.0f;
                float sweepMs = 3.2f * depth_;
                for (int v = 0; v < 8; ++v) {
                    float pL = wrapPhase(lfoPhase1_ + kPhaseOffsets[v] + 0.15f * FastMath::fastSinNorm(lfoPhase3_ + kPhaseOffsets[v]));
                    float pR = wrapPhase(lfoPhase2_ + kPhaseOffsets[7 - v] + 0.15f * FastMath::fastCosNorm(lfoPhase3_ + kPhaseOffsets[v]));
                    float modL = FastMath::fastSinNorm(pL);
                    float modR = FastMath::fastCosNorm(pR);
                    float tapL = (preDelayMs_ + kBaseTapMs[v] + sweepMs * modL) * msToSamples;
                    float tapR = (preDelayMs_ + kBaseTapMs[v] + sweepMs * modR) * msToSamples;
                    sumL += readDelayLinear(delayL_, tapL);
                    sumR += readDelayLinear(delayR_, tapR);
                }
                wetL = sumL * 0.28f;
                wetR = sumR * 0.28f;
                break;
            }

            case 4: {
                // 4: Spatial Dimension BBD (Stereo Dimension cross-channel high-passed phase-inverted spatial expansion)
                float triL = triangleWave(lfoPhase1_);
                float triR = -triL;
                float baseMs = 4.2f + preDelayMs_ * 0.5f;
                float depthMs = 2.6f * depth_;
                float dL = (baseMs + depthMs * triL) * msToSamples;
                float dR = (baseMs + depthMs * triR) * msToSamples;
                float bbdL = FastMath::fastTanh(readDelayLinear(delayL_, dL));
                float bbdR = FastMath::fastTanh(readDelayLinear(delayR_, dR));
                float hpL = dimHpL_.processHP(bbdL);
                float hpR = dimHpR_.processHP(bbdR);
                // Cross-channel phase-inverted high-passed injection
                wetL = bbdL - 0.65f * hpR;
                wetR = bbdR - 0.65f * hpL;
                break;
            }

            case 5: {
                // 5: Through-Zero Flanger
                // Fixed reference dry delay tap at 5ms + modulated tap sweeping across 0ms..10ms
                float refDelaySamps = (5.0f + preDelayMs_ * 0.25f) * msToSamples;
                dryRefL = readDelayLinear(delayL_, refDelaySamps);
                dryRefR = readDelayLinear(delayR_, refDelaySamps);

                float triL = triangleWave(lfoPhase1_);
                float triR = triangleWave(wrapPhase(lfoPhase1_ + 0.25f));
                // Center swept tap at refDelaySamps and sweep +/- 5ms * depth across zero point
                float sweepSamps = 5.0f * depth_ * msToSamples;
                float modDelayL = std::max(0.0f, refDelaySamps + sweepSamps * triL);
                float modDelayR = std::max(0.0f, refDelaySamps + sweepSamps * triR);
                wetL = readDelayLinear(delayL_, modDelayL);
                wetR = readDelayLinear(delayR_, modDelayR);
                break;
            }

            case 6: {
                // 6: 8-Stage Optical Phaser (8 cascaded 1st-order all-pass stages per channel swept by LFO + feedback)
                float lfoL = 0.5f + 0.5f * FastMath::fastSinNorm(lfoPhase1_);
                float lfoR = 0.5f + 0.5f * FastMath::fastCosNorm(lfoPhase1_);
                float minHz = 200.0f;
                float maxHz = 200.0f + 4200.0f * depth_;
                float fcL = Taper::profileD_Cutoff(lfoL, minHz, maxHz);
                float fcR = Taper::profileD_Cutoff(lfoR, minHz, maxHz);

                float wdL = DSPCommon::fastTanPiNorm(fcL * invSr_);
                float wdR = DSPCommon::fastTanPiNorm(fcR * invSr_);
                float gL = wdL / (1.0f + wdL);
                float gR = wdR / (1.0f + wdR);

                float sigL = readDelayLinear(delayL_, preDelaySamps);
                float sigR = readDelayLinear(delayR_, preDelaySamps);

                for (int s = 0; s < kNumPhaserStages; ++s) {
                    float vL = (sigL - apStateL_[s]) * gL;
                    float lpL = vL + apStateL_[s];
                    apStateL_[s] = lpL + vL;
                    sigL = 2.0f * lpL - sigL;

                    float vR = (sigR - apStateR_[s]) * gR;
                    float lpR = vR + apStateR_[s];
                    apStateR_[s] = lpR + vR;
                    sigR = 2.0f * lpR - sigR;
                }
                wetL = sigL;
                wetR = sigR;
                break;
            }

            case 7:
            default: {
                // 7: Harmonic Tremolo (500 Hz OnePoleTPT low/high crossover split modulated 180 degrees out of phase)
                float srcL = readDelayLinear(delayL_, preDelaySamps);
                float srcR = readDelayLinear(delayR_, preDelaySamps);

                float lowL = tremFilterL_.processLP(srcL);
                float highL = srcL - lowL;
                float lowR = tremFilterR_.processLP(srcR);
                float highR = srcR - lowR;

                float modL = depth_ * FastMath::fastSinNorm(lfoPhase1_);
                float modR = depth_ * FastMath::fastSinNorm(wrapPhase(lfoPhase1_ + 0.25f * stereoWidth_));

                float gainLowL  = 0.5f * (1.0f + modL);
                float gainHighL = 0.5f * (1.0f - modL);
                float gainLowR  = 0.5f * (1.0f + modR);
                float gainHighR = 0.5f * (1.0f - modR);

                wetL = lowL * gainLowL + highL * gainHighL;
                wetR = lowR * gainLowR + highR * gainHighR;
                break;
            }
        }

        // Apply Tone / Color BBD low-pass damping filter & DC blocker
        wetL = dcBlockL_.process(toneFilterL_.processLP(wetL));
        wetR = dcBlockR_.process(toneFilterR_.processLP(wetR));

        // Update feedback states
        fbStateL_ = wetL;
        fbStateR_ = wetR;

        // Apply Mid/Side Stereo Width matrix (0% .. 200%) on wet signal
        float mid  = 0.5f * (wetL + wetR);
        float side = 0.5f * (wetL - wetR) * stereoWidth_;
        wetL = mid + side;
        wetR = mid - side;

        // Advance circular buffer write pointer
        writePos_ = (writePos_ + 1) & kBufferMask;

        // Apply Profile M Tri-Zone Parallel Mix
        outL = dryRefL * dryGain_ + wetL * wetGain_;
        outR = dryRefR * dryGain_ + wetR * wetGain_;
    }
};

} // namespace TbdAudio::Spatial
