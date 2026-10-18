#pragma once

#include "../common/AudioEffect.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace TbdAudio::FX {

/**
 * @brief PluckFX — External Audio & Sidechain-Excited Physical Waveguide & Modal Resonator Effect.
 *
 * Companion audio effect to OSC-05 (`Pluck`), inspired by Mutable Instruments Rings' external
 * audio excitation input and Korg Volca Drum waveguide resonators. Any drum loop, synth lead,
 * or vocal fed into `PluckFX` directly excites a 4-voice sympathetic string waveguide + 16-mode
 * modal filter bank.
 *
 * 8-Knob Performance Layout (params8[0..7]):
 *   [0] PITCH / TUNE          (30 Hz .. 4000 Hz root fundamental)
 *   [1] DECAY / RING          (Taper::profileF_Hyperbolic 0.30 .. 1.08 waveguide & modal sustain)
 *   [2] DAMPING / BRIGHTNESS  (800 Hz .. 18000 Hz material HF absorption filter)
 *   [3] STRUCTURE / CHORD     (0: Unison String, 1: Fifths/Octave Sympathetic, 2: Modal Bell Plate, 3: Phrygian Cluster)
 *   [4] STIFFNESS / INHARMONIC(All-pass waveguide dispersion & modal Bessel stretch)
 *   [5] POSITION / DRIVE      (Strike position comb notch + Padé fastTanh resonator saturation)
 *   [6] STEREO SPREAD         (Odd/Even voice binaural pan separation + micro-detune)
 *   [7] MIX / ∅               (Profile M Tri-Zone Mix: 0.0 = 100% Wet, 0.5 = 100% Dry, 1.0 = 50/50 Parallel)
 */
class PluckFX final : public AudioEffect8 {
public:
    static constexpr int kNumVoices = 4;
    static constexpr int kMaxCombLen = 4096;
    static constexpr int kCombMask = kMaxCombLen - 1;

    PluckFX() noexcept {
        setParam(0, 0.35f); // Pitch (~220 Hz)
        setParam(1, 0.65f); // Decay / Ring
        setParam(2, 0.60f); // Damping / Brightness
        setParam(3, 0.25f); // Structure
        setParam(4, 0.20f); // Stiffness
        setParam(5, 0.30f); // Position / Drive
        setParam(6, 0.55f); // Stereo Spread
        setParam(7, 0.00f); // Mix (100% Wet default per Profile M)
        reset();
    }

    ~PluckFX() override = default;

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
            modalFilter_[v][0].reset();
            modalFilter_[v][1].reset();
        }
        smoothPitchHz_ = 220.0f;
        smoothFeedback_ = 0.75f;
        smoothDampCoeff_ = 0.5f;
    }

    void setParam8(int idx, float val) noexcept {
        setParam(idx, val);
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        if (!buffer || numSamples <= 0) return;
        FastMath::enableFTZDAZ();

        if (ctx.sampleRate > 0.0f && std::abs(ctx.sampleRate - sampleRate_) > 1.0f) {
            sampleRate_ = std::clamp(ctx.sampleRate, 8000.0f, 192000.0f);
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

        if (ctx.sampleRate > 0.0f && std::abs(ctx.sampleRate - sampleRate_) > 1.0f) {
            sampleRate_ = std::clamp(ctx.sampleRate, 8000.0f, 192000.0f);
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
    static constexpr float kStructureRatios[4][4] = {
        { 1.000000f, 1.002000f, 1.998000f, 2.003000f }, // 0: Unison / Octave Doubled Strings
        { 1.000000f, 1.498307f, 2.000000f, 2.996614f }, // 1: Sympathetic Fifths & Octaves
        { 1.000000f, 2.756000f, 5.404000f, 8.933000f }, // 2: Modal Bell Plate (Bessel modes)
        { 1.000000f, 1.059463f, 1.498307f, 2.000000f }  // 3: Phrygian Sympathetic Cluster
    };

    [[nodiscard]] inline float readLagrange3(int voice, float delaySamples) const noexcept {
        const float clampedDelay = std::clamp(delaySamples, 2.0f, static_cast<float>(kMaxCombLen - 4));
        const float readIdx = static_cast<float>(writePos_[voice]) - clampedDelay;
        int idx0 = static_cast<int>(std::floor(readIdx));
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

        // 0: PITCH / TUNE (30 Hz .. 4000 Hz)
        const float pPitch = std::clamp(params8[0], 0.0f, 1.0f);
        const float targetPitchHz = 30.0f * FastMath::fastPow2(pPitch * 7.0588937f);
        smoothPitchHz_ += 0.005f * (targetPitchHz - smoothPitchHz_);

        // 1: DECAY / RING (0.35 .. 1.08 via profileF_Hyperbolic)
        const float fbTaper = Taper::profileF_Hyperbolic(std::clamp(params8[1], 0.0f, 1.0f), 0.35f, 1.08f);
        smoothFeedback_ += 0.005f * (fbTaper - smoothFeedback_);

        // 2: DAMPING / BRIGHTNESS (800 Hz .. 18000 Hz)
        const float dampHz = Taper::profileD_Cutoff(std::clamp(params8[2], 0.0f, 1.0f), 800.0f, 18000.0f);
        const float gDamp = std::clamp(3.14159265f * dampHz * invSampleRate_, 0.01f, 0.95f);
        const float targetDampCoeff = gDamp / (1.0f + gDamp);
        smoothDampCoeff_ += 0.01f * (targetDampCoeff - smoothDampCoeff_);

        const float gHp = std::clamp(3.14159265f * 35.0f * invSampleRate_, 0.0005f, 0.1f);
        const float hpCoeff = gHp / (1.0f + gHp);

        // 3: STRUCTURE [0..3]
        const int structIdx = std::clamp(static_cast<int>(params8[3] * 3.99f), 0, 3);

        // 4: STIFFNESS / INHARMONIC
        const float stiffness = std::clamp(params8[4], 0.0f, 1.0f);
        const float apCoeff = (stiffness - 0.5f) * 1.30f;

        // 5: POSITION / DRIVE
        const float driveNorm = std::clamp(params8[5], 0.0f, 1.0f);
        const float loopDrive = 1.0f + driveNorm * 4.5f;
        const float invLoopDrive = 1.0f / FastMath::fastTanh(loopDrive);

        // 6: STEREO SPREAD
        const float spread = std::clamp(params8[6], 0.0f, 1.0f);

        float wetL = 0.0f;
        float wetR = 0.0f;

        for (int v = 0; v < kNumVoices; ++v) {
            const float detuneSign = ((v & 1) == 0) ? -1.0f : 1.0f;
            const float microDetune = 1.0f + detuneSign * spread * 0.005f * static_cast<float>(v + 1);
            const float ratio = kStructureRatios[structIdx][v] * microDetune;
            const float voiceHz = std::clamp(smoothPitchHz_ * ratio, 20.0f, sampleRate_ * 0.44f);
            const float delaySamples = sampleRate_ / voiceHz;

            const float delayed = readLagrange3(v, delaySamples);

            // One-pole LP damping
            const float vLp = (delayed - lpState_[v]) * smoothDampCoeff_;
            const float lpOut = vLp + lpState_[v];
            lpState_[v] = lpOut + vLp;

            // One-pole HP anti-mud
            const float vHp = (lpOut - hpState_[v]) * hpCoeff;
            const float hpLp = vHp + hpState_[v];
            hpState_[v] = hpLp + vHp;
            const float filtered = lpOut - hpLp;

            // All-pass stiffness dispersion
            const float apOut = apCoeff * filtered + apStateIn_[v] - apCoeff * apStateOut_[v];
            apStateIn_[v] = filtered;
            apStateOut_[v] = apOut;

            // Parallel Modal SVF Resonator pair for Rings-style metallic/wooden shimmer
            const float modalQ = 6.0f + 18.0f * (1.0f - stiffness * 0.4f);
            modalFilter_[v][0].setBandpass(voiceHz, modalQ, invSampleRate_);
            const float mode2Hz = std::min(voiceHz * (2.0f + 0.75f * stiffness), sampleRate_ * 0.44f);
            modalFilter_[v][1].setBandpass(mode2Hz, modalQ * 0.8f, invSampleRate_);
            const float modalOut = (modalFilter_[v][0].process(monoIn) + 0.5f * modalFilter_[v][1].process(monoIn)) * 0.45f;
            modalFilter_[v][0].clampState();
            modalFilter_[v][1].clampState();

            // Waveguide feedback + saturation
            const float excited = monoIn * 0.35f + smoothFeedback_ * apOut;
            const float saturated = FastMath::fastTanh(excited * loopDrive) * invLoopDrive;

            const float dcOut = saturated - dcX1_[v] + 0.995f * dcY1_[v];
            dcX1_[v] = saturated;
            dcY1_[v] = dcOut;

            combBuf_[v][writePos_[v]] = dcOut;
            writePos_[v] = (writePos_[v] + 1) & kCombMask;

            const float combinedVoice = 0.75f * dcOut + 0.55f * modalOut;
            const float voicePan = ((v & 1) == 0) ? (0.5f - 0.45f * spread) : (0.5f + 0.45f * spread);
            wetL += combinedVoice * (1.0f - voicePan);
            wetR += combinedVoice * voicePan;
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
    DSPCommon::CytomicSVF modalFilter_[kNumVoices][2];

    float sampleRate_ = 48000.0f;
    float invSampleRate_ = 1.0f / 48000.0f;
    float smoothPitchHz_ = 220.0f;
    float smoothFeedback_ = 0.75f;
    float smoothDampCoeff_ = 0.5f;
};

using KlangPluckFX = PluckFX;

} // namespace TbdAudio::FX
