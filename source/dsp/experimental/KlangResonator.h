#pragma once

#include "../common/AudioEffect.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace TbdAudio::Experimental {

/**
 * @brief KlangResonator (KlangComb) — 4-Voice Modal & Tuned Waveguide Resonator Bank.
 *
 * Implements 4 parallel fractional-delay waveguide resonators with cubic Lagrange interpolation,
 * per-voice TPT one-pole HF damping + anti-mud HPF, 1st-order all-pass stiffness dispersion,
 * and Padé fastTanh loop saturation.
 *
 * 8-Knob Performance Layout (params8[0..7]):
 *   [0] PITCH / TUNE          (30 Hz .. 4000 Hz)
 *   [1] FEEDBACK / DECAY      (0.30 .. 1.12 via profileF_Hyperbolic)
 *   [2] DAMPING / HF LOSS     (800 Hz .. 18000 Hz)
 *   [3] ENGINE                (0: Resonant Tube, 1: Tuned Steel, 2: Clatter/Bronze Bell, 3: Phrygian Modal Cluster)
 *   [4] STIFFNESS / INHARMONIC(All-pass dispersion coefficient)
 *   [5] DRIVE / LOOP SAT      (Waveguide feedback loop Padé fastTanh saturation)
 *   [6] STEREO SPREAD         (Odd/Even voice pan separation + micro-detune)
 *   [7] MIX / ∅               (Tri-Zone Mix via profileM_TriZoneMix)
 */
class KlangResonator final : public AudioEffect8 {
public:
    static constexpr int kNumVoices = 4;
    static constexpr int kMaxCombLen = 4096;
    static constexpr int kCombMask = kMaxCombLen - 1;

    KlangResonator() noexcept {
        params8[0] = 0.35f; // Pitch (~220 Hz)
        params8[1] = 0.65f; // Feedback / Decay
        params8[2] = 0.60f; // HF Damping
        params8[3] = 0.00f; // Engine 0: Resonant Tube
        params8[4] = 0.15f; // Stiffness
        params8[5] = 0.25f; // Drive / Loop Sat
        params8[6] = 0.50f; // Stereo Spread
        params8[7] = 0.50f; // Mix (50/50)
        reset();
    }

    ~KlangResonator() override = default;

    void init(const BlockContext& ctx) override {
        prepare(ctx.sampleRate > 0.0f ? static_cast<double>(ctx.sampleRate) : 48000.0, 512);
    }

    void prepare(double sampleRate, int /*maxBlockSize*/ = 512) noexcept {
        sampleRate_ = std::clamp(static_cast<float>(sampleRate), 8000.0f, 192000.0f);
        invSampleRate_ = 1.0f / sampleRate_;
        reset();
    }

    void reset() noexcept override {
        std::memset(combBuf_, 0, sizeof(combBuf_));
        for (int v = 0; v < kNumVoices; ++v) {
            writePos_[v] = 0;
            lpState_[v] = 0.0f;
            hpState_[v] = 0.0f;
            apStateIn_[v] = 0.0f;
            apStateOut_[v] = 0.0f;
            dcX1_[v] = 0.0f;
            dcY1_[v] = 0.0f;
        }
        smoothPitchHz_ = 220.0f;
        smoothFeedback_ = 0.75f;
        smoothDampCoeff_ = 0.5f;
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
    static constexpr float kModalRatios[4][4] = {
        { 1.000000f, 1.500000f, 2.000000f, 3.000000f }, // 0: Resonant Tube (Inverted -fb odd-harmonic pipe)
        { 1.000000f, 2.000000f, 3.000000f, 4.000000f }, // 1: Tuned Steel (Positive +fb integer harmonics)
        { 1.000000f, 2.756000f, 5.404000f, 8.933000f }, // 2: Clatter / Bronze Bell (Inharmonic Bessel modes)
        { 1.000000f, 1.059463f, 1.498307f, 2.000000f }  // 3: Phrygian Modal Cluster (Root, b2, 5th, Oct)
    };

    inline float readLagrange3(int voice, float delaySamples) const noexcept {
        const float clampedDelay = std::clamp(delaySamples, 2.0f, static_cast<float>(kMaxCombLen - 4));
        const float readIdx = static_cast<float>(writePos_[voice]) - clampedDelay;
        int idx0 = static_cast<int>( std::floor(readIdx) );
        const float frac = readIdx - static_cast<float>(idx0);

        const int im1 = (idx0 - 1) & kCombMask;
        const int i0  = (idx0    ) & kCombMask;
        const int i1  = (idx0 + 1) & kCombMask;
        const int i2  = (idx0 + 2) & kCombMask;

        const float ym1 = combBuf_[voice][im1];
        const float y0  = combBuf_[voice][i0];
        const float y1  = combBuf_[voice][i1];
        const float y2  = combBuf_[voice][i2];

        const float c0 = y0;
        const float c1 = y1 - (1.0f / 3.0f) * ym1 - 0.5f * y0 - (1.0f / 6.0f) * y2;
        const float c2 = 0.5f * (ym1 + y1) - y0;
        const float c3 = (1.0f / 6.0f) * (y2 - ym1) + 0.5f * (y0 - y1);

        return ((c3 * frac + c2) * frac + c1) * frac + c0;
    }

    inline void processSamplePair(float& ioL, float& ioR) noexcept {
        const float dryL = ioL;
        const float dryR = ioR;
        const float monoIn = 0.5f * (dryL + dryR);

        // 0: PITCH / TUNE (30 Hz .. 4000 Hz exponential mapping via fastPow2)
        const float pPitch = std::clamp(params8[0], 0.0f, 1.0f);
        const float targetPitchHz = 30.0f * FastMath::fastPow2(pPitch * 7.0588937f); // 30 * 2^7.05889 = 4000 Hz
        smoothPitchHz_ += 0.005f * (targetPitchHz - smoothPitchHz_);

        // 1: FEEDBACK / DECAY (0.30 .. 1.12 via profileF_Hyperbolic)
        const float fbTaper = Taper::profileF_Hyperbolic(std::clamp(params8[1], 0.0f, 1.0f));
        const float targetFb = 0.30f + fbTaper * 0.82f; // 0.30 .. 1.12
        smoothFeedback_ += 0.005f * (targetFb - smoothFeedback_);

        // 2: DAMPING / HF LOSS (800 Hz .. 18000 Hz)
        const float dampRaw = Taper::profileD_Cutoff(std::clamp(params8[2], 0.0f, 1.0f));
        const float dampHz = (dampRaw > 1.5f)
            ? std::clamp(dampRaw, 800.0f, 18000.0f)
            : (800.0f + dampRaw * 17200.0f);
        const float gDamp = std::clamp(3.14159265f * dampHz * invSampleRate_, 0.01f, 0.95f);
        const float targetDampCoeff = gDamp / (1.0f + gDamp);
        smoothDampCoeff_ += 0.01f * (targetDampCoeff - smoothDampCoeff_);

        // Anti-mud HPF at ~35 Hz
        const float gHp = std::clamp(3.14159265f * 35.0f * invSampleRate_, 0.0005f, 0.1f);
        const float hpCoeff = gHp / (1.0f + gHp);

        // 3: ENGINE [0..3]
        const int engineIdx = std::clamp(static_cast<int>(params8[3] * 3.99f), 0, 3);
        const float fbPolarity = (engineIdx == 0) ? -1.0f : 1.0f;

        // 4: STIFFNESS / INHARMONIC (1st-order all-pass coefficient -0.65 .. +0.65)
        const float stiffness = std::clamp(params8[4], 0.0f, 1.0f);
        const float apCoeff = (stiffness - 0.5f) * 1.30f;

        // 5: DRIVE / LOOP SAT (1.0x .. 6.0x loop gain into Padé fastTanh)
        const float driveNorm = std::clamp(params8[5], 0.0f, 1.0f);
        const float loopDrive = 1.0f + driveNorm * 5.0f;
        const float invLoopDrive = 1.0f / FastMath::fastTanh(loopDrive);

        // 6: STEREO SPREAD (Pan & micro-detune across the 4 voices)
        const float spread = std::clamp(params8[6], 0.0f, 1.0f);

        float wetL = 0.0f;
        float wetR = 0.0f;

        for (int v = 0; v < kNumVoices; ++v) {
            const float detuneSign = ((v & 1) == 0) ? -1.0f : 1.0f;
            const float microDetune = 1.0f + detuneSign * spread * 0.006f * static_cast<float>(v + 1);
            const float ratio = kModalRatios[engineIdx][v] * microDetune;
            const float voiceHz = std::clamp(smoothPitchHz_ * ratio, 20.0f, sampleRate_ * 0.45f);
            const float delaySamples = sampleRate_ / voiceHz;

            const float delayed = readLagrange3(v, delaySamples);

            // Per-voice TPT One-Pole HF Damping LPF
            const float vLp = (delayed - lpState_[v]) * smoothDampCoeff_;
            const float lpOut = vLp + lpState_[v];
            lpState_[v] = lpOut + vLp;

            // Per-voice TPT One-Pole Anti-Mud HPF
            const float vHp = (lpOut - hpState_[v]) * hpCoeff;
            const float hpLp = vHp + hpState_[v];
            hpState_[v] = hpLp + vHp;
            const float filtered = lpOut - hpLp;

            // 1st-Order All-Pass Stiffness Dispersion Filter
            const float apOut = apCoeff * filtered + apStateIn_[v] - apCoeff * apStateOut_[v];
            apStateIn_[v] = filtered;
            apStateOut_[v] = apOut;

            // Waveguide feedback with Padé fastTanh loop saturation
            const float excited = monoIn * 0.35f + fbPolarity * smoothFeedback_ * apOut;
            const float saturated = FastMath::fastTanh(excited * loopDrive) * invLoopDrive;

            // Subtle DC blocker inside loop
            const float dcOut = saturated - dcX1_[v] + 0.995f * dcY1_[v];
            dcX1_[v] = saturated;
            dcY1_[v] = dcOut;

            combBuf_[v][writePos_[v]] = dcOut;
            writePos_[v] = (writePos_[v] + 1) & kCombMask;

            // Odd voices (v=0,2 -> 1st,3rd) lean L, Even voices (v=1,3 -> 2nd,4th) lean R
            const float voicePan = ((v & 1) == 0) ? (0.5f - 0.45f * spread) : (0.5f + 0.45f * spread);
            wetL += dcOut * (1.0f - voicePan);
            wetR += dcOut * voicePan;
        }

        wetL *= 0.55f;
        wetR *= 0.55f;

        float dryGain = 1.0f;
        float wetGain = 0.0f;
        Taper::profileM_TriZoneMix(params8[7], phaseInvert, dryGain, wetGain);

        ioL = dryL * dryGain + wetL * wetGain;
        ioR = dryR * dryGain + wetR * wetGain;
    }

    float combBuf_[kNumVoices][kMaxCombLen]{};
    int writePos_[kNumVoices]{};
    float lpState_[kNumVoices]{};
    float hpState_[kNumVoices]{};
    float apStateIn_[kNumVoices]{};
    float apStateOut_[kNumVoices]{};
    float dcX1_[kNumVoices]{};
    float dcY1_[kNumVoices]{};

    float sampleRate_ = 48000.0f;
    float invSampleRate_ = 1.0f / 48000.0f;
    float smoothPitchHz_ = 220.0f;
    float smoothFeedback_ = 0.75f;
    float smoothDampCoeff_ = 0.5f;
};

using KlangComb = KlangResonator;
using PipeAndWire = KlangResonator;

} // namespace TbdAudio::Experimental
