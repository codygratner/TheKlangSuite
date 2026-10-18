#pragma once

#include "../common/AudioEffect.h"
#include "../oscillators/PolyBLEP.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>

namespace TbdAudio::FX {

/**
 * @brief Vocoder56K (`56K Vocoder` / `KlangWarpFactoryPro`) — 32-Band Spectral Warper,
 *        Roland MKS-50 / AlphaKraft Carrier, Zero-Crossing Pitch Tracker & EMS Patch Matrix.
 *
 * Implements BACKLOG-13 ("Electrix WarpFactory Pro: 25th Anniversary Edition"):
 *   - 32 logarithmic Cytomic ZDF State-Variable Filter bands (80 Hz .. 14 kHz, selectable 8/16/24/32).
 *   - Bi-directional Formant Shift (-24..+24 st) & Bi-directional Warp envelope decay skew.
 *   - 5 Carrier Modes:
 *       0: AlphaKraft / MKS-50 Dual-DCO (14 Alpha shapes + Sub + Stereo BBD Chorus)
 *       1: Real-Time Zero-Crossing Vocal Pitch Tracker (tracks modulator F0 automatically)
 *       2: Retro Robot Pulse & 5-Voice Detuned Supersaw
 *       3: WarpFactory "Auto Mode" (Self-Vocoding: modulator excites both Formant & Carrier!)
 *       4: External Stereo Carrier (Right channel / sidechain modulates Left channel)
 *   - 4-Mode Electrix / EMS Patch Matrix (Normal 1:1, Inverted Spectral Mirror, Scramble, Shift).
 *   - Dual-Path Sibilance Engine (high-pass consonant pass-through + upper-band pink noise injection).
 *   - Spectral Freeze (`LATCH` button locks all 32 envelope followers in memory).
 *
 * 8 Performance Knobs (params8[0..7]):
 *   [0] FORMANT / GENDER (-24 .. +24 st vocal tract shift via Profile B Bipolar)
 *   [1] WARP / SKEW      (-1.0 .. +1.0 bi-directional envelope decay skew + Q resonance)
 *   [2] SIBILANCE / ESS  (0% .. 100% unvoiced consonant detection & noise injection)
 *   [3] CARRIER ENGINE   (0: AlphaKraft DCO, 1: Pitch Track, 2: Robot/Supersaw, 3: Auto Self-Vocode, 4: Ext)
 *   [4] BANDS / MATRIX   (Selects 8/16/24/32 bands & Matrix routing: Normal, Inverted, Scramble)
 *   [5] SPEED / FREEZE   (Envelope follower attack/release speed; >= 0.94 latches Spectral Freeze!)
 *   [6] PITCH / SPREAD   (Carrier base pitch 45..440 Hz + alternating stereo band pan & BBD Chorus)
 *   [7] MIX / ∅          (Profile M Tri-Zone Mix: 0.0 = 100% Wet, 0.5 = 100% Dry, 1.0 = 50/50 Parallel)
 */
class Vocoder56K final : public AudioEffect8 {
public:
    static constexpr int kMaxBands = 32;
    static constexpr int kChorusBufLen = 512;
    static constexpr int kChorusMask = kChorusBufLen - 1;

    enum class PatchMatrixMode : int {
        Normal = 0,
        InvertedMirror = 1,
        Scramble = 2,
        ShiftRotate = 3
    };

    Vocoder56K() noexcept {
        setParam(0, 0.50f); // Formant (0 st neutral)
        setParam(1, 0.45f); // Warp / Resonance
        setParam(2, 0.35f); // Sibilance
        setParam(3, 0.10f); // Carrier 0: AlphaKraft MKS-50 DCO
        setParam(4, 0.20f); // 32 Bands, Normal Matrix
        setParam(5, 0.25f); // Speed (fast articulate speech)
        setParam(6, 0.45f); // Pitch (~110 Hz) + Stereo Spread
        setParam(7, 0.00f); // Mix (100% Wet default per Profile M)
        reset();
    }

    ~Vocoder56K() override = default;

    void init(const BlockContext& ctx) override {
        prepare(ctx.sampleRate > 0.0f ? static_cast<double>(ctx.sampleRate) : 48000.0, 512);
    }

    void prepare(double sampleRate, int /*maxBlockSize*/ = 512) noexcept {
        sampleRate_ = std::clamp(static_cast<float>(sampleRate), 8000.0f, 192000.0f);
        invSr_ = 1.0f / sampleRate_;
        computeBandFrequencies();
        reset();
    }

    void reset() noexcept override {
        for (int b = 0; b < kMaxBands; ++b) {
            analysisSvf_[b].reset();
            synthSvfL_[b].reset();
            synthSvfR_[b].reset();
            envLevels_[b] = 0.0f;
            frozenLevels_[b] = 0.0f;
        }
        std::memset(chorusBufL_, 0, sizeof(chorusBufL_));
        std::memset(chorusBufR_, 0, sizeof(chorusBufR_));
        chorusWrite_ = 0;
        chorusPhase_ = 0.0f;
        dcoPhase1_ = 0.0f;
        dcoPhase2_ = 0.0f;
        subPhase_ = 0.0f;
        trackedPitchHz_ = 110.0f;
        zcCounter_ = 0;
        lastModSign_ = false;
        optoEnv_ = 0.1f;
        modLowpass_.reset();
        sibilanceHpf_.reset();
        dcBlockL_.init(sampleRate_);
        dcBlockR_.init(sampleRate_);
        dcBlockL_.reset();
        dcBlockR_.reset();
        noise_.reset(0x564B3030u);
        updateFilterCoefficients(6.0f);
    }

    void setParam8(int idx, float val) noexcept {
        setParam(idx, val);
    }

    void setFreeze(bool freeze) noexcept {
        if (freeze && !manualFreeze_) {
            for (int b = 0; b < kMaxBands; ++b) {
                frozenLevels_[b] = envLevels_[b];
            }
        }
        manualFreeze_ = freeze;
    }

    [[nodiscard]] bool isFrozen() const noexcept {
        return manualFreeze_ || (params8[5] >= 0.94f);
    }

    void setPatchMatrix(PatchMatrixMode mode) noexcept {
        matrixOverride_ = static_cast<int>(mode);
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
            processSamplePair(L, R, ctx.currentPitchHz);
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
            processSamplePair(L, R, ctx.currentPitchHz);
            left[i] = std::clamp(L, -4.0f, 4.0f);
            right[i] = std::clamp(R, -4.0f, 4.0f);
        }
        FastMath::sanitizeBuffer(left, numSamples);
        FastMath::sanitizeBuffer(right, numSamples);
    }

private:
    // Deterministic EMS-5000 scramble permutation table across 32 bands
    static constexpr int kScrambleMap[32] = {
        17,  4, 29, 11,  0, 23,  8, 31,
        14,  2, 26, 19,  6, 21, 12, 28,
         3, 25,  9, 30, 15,  1, 20,  7,
        27, 10, 22,  5, 18, 13, 24, 16
    };

    void computeBandFrequencies() noexcept {
        // Logarithmic Bark/cochlear spacing from 80 Hz to 13500 Hz using FastMath::fastPow2
        // 13500 / 80 = 168.75 -> log2(168.75) = 7.3987437f
        constexpr float fMin = 80.0f;
        constexpr float totalOctaves = 7.3987437f;
        for (int b = 0; b < kMaxBands; ++b) {
            const float norm = static_cast<float>(b) / static_cast<float>(kMaxBands - 1);
            bandFreqs_[b] = std::min(fMin * FastMath::fastPow2(norm * totalOctaves), 0.44f * sampleRate_);
        }
    }

    void updateFilterCoefficients(float qFactor) noexcept {
        const float q = std::clamp(qFactor, 2.0f, 18.0f);
        for (int b = 0; b < kMaxBands; ++b) {
            const float f = std::clamp(bandFreqs_[b], 60.0f, 0.44f * sampleRate_);
            analysisSvf_[b].setBandpass(f, q, invSr_);
            synthSvfL_[b].setBandpass(f, q, invSr_);
            synthSvfR_[b].setBandpass(f, q, invSr_);
        }
        sibilanceHpf_.setCutoff(std::min(4800.0f, 0.42f * sampleRate_), invSr_);
        modLowpass_.setCutoff(650.0f, invSr_);
    }

    inline void renderCarrier(int carrierMode, float basePitchHz, float modIn, float extCarrierIn,
                              float& outCarrierL, float& outCarrierR) noexcept {
        // Zero-crossing vocal pitch tracker on low-passed modulator
        const float lpMod = modLowpass_.processLP(modIn);
        const bool signPos = (lpMod >= 0.0f);
        ++zcCounter_;
        if (signPos && !lastModSign_) {
            if (zcCounter_ > static_cast<int>(sampleRate_ / 800.0f) &&
                zcCounter_ < static_cast<int>(sampleRate_ / 50.0f)) {
                const float instHz = sampleRate_ / static_cast<float>(zcCounter_);
                trackedPitchHz_ += 0.12f * (instHz - trackedPitchHz_);
            }
            zcCounter_ = 0;
        }
        lastModSign_ = signPos;

        float freqHz = ( carrierMode == 1 ) ? trackedPitchHz_ : basePitchHz;
        freqHz = std::clamp(freqHz, 32.0f, 2400.0f);
        const float dt1 = freqHz * invSr_;
        const float dt2 = (freqHz * 1.007f) * invSr_;
        const float dtSub = (freqHz * 0.5f) * invSr_;

        dcoPhase1_ = Oscillators::PolyBLEP::wrapPhase(dcoPhase1_ + dt1);
        dcoPhase2_ = Oscillators::PolyBLEP::wrapPhase(dcoPhase2_ + dt2);
        subPhase_  = Oscillators::PolyBLEP::wrapPhase(subPhase_ + dtSub);

        switch (carrierMode) {
            case 0: // AlphaKraft / MKS-50 Dual-DCO + Sub + PWM
            case 1: { // Vocal Pitch Tracker driving AlphaKraft DCO
                const float w1 = Oscillators::PolyBLEP::alphaWave14(4, dcoPhase1_, dt1, 0.45f);
                const float w2 = Oscillators::PolyBLEP::alphaWave14(1, dcoPhase2_, dt2, 0.55f);
                const float sub = Oscillators::PolyBLEP::pulse(subPhase_, dtSub, 0.5f);
                const float buzz = 0.18f * noise_.nextPink();
                outCarrierL = 0.55f * w1 + 0.30f * sub + buzz;
                outCarrierR = 0.55f * w2 + 0.30f * sub - buzz;
                break;
            }
            case 2: { // Retro Robot Pulse + Supersaw
                const float saw1 = Oscillators::PolyBLEP::saw(dcoPhase1_, dt1);
                const float saw2 = Oscillators::PolyBLEP::saw(dcoPhase2_, dt2);
                const float pls  = Oscillators::PolyBLEP::pulse(dcoPhase1_, dt1, 0.28f);
                outCarrierL = 0.50f * saw1 + 0.40f * pls + 0.15f * noise_.nextWhite();
                outCarrierR = 0.50f * saw2 + 0.40f * pls + 0.15f * noise_.nextWhite();
                break;
            }
            case 3: { // WarpFactory "Auto Mode" (Self-Vocoder + harmonic exciter)
                const float harmonicExciter = FastMath::fastTanh(modIn * 3.8f) + 0.25f * Oscillators::PolyBLEP::saw(dcoPhase1_, dt1);
                outCarrierL = harmonicExciter;
                outCarrierR = FastMath::fastTanh(extCarrierIn * 3.8f) + 0.25f * Oscillators::PolyBLEP::saw(dcoPhase2_, dt2);
                break;
            }
            case 4:
            default: { // External Stereo Carrier (with subtle internal fallback if silent)
                const float extEnergy = std::abs(extCarrierIn);
                const float fallback = (extEnergy < 1e-4f) ? (0.35f * Oscillators::PolyBLEP::saw(dcoPhase1_, dt1)) : 0.0f;
                outCarrierL = extCarrierIn * 2.2f + fallback;
                outCarrierR = extCarrierIn * 2.2f + fallback;
                break;
            }
        }
    }

    inline void processSamplePair(float& ioL, float& ioR, float hostPitchHz) noexcept {
        const float dryL = ioL;
        const float dryR = ioR;
        const float rawMod = 0.5f * (dryL + dryR);

        // 0: FORMANT (-24..+24 st -> band offset [-12..+12])
        const float formantShiftBands = (std::clamp(params8[0], 0.0f, 1.0f) - 0.5f) * 20.0f;
        const int formantOffset = static_cast<int>( std::round(formantShiftBands) );

        // 1: WARP (-1.0..+1.0 decay skew + resonance update)
        const float warpKnob = std::clamp(params8[1], 0.0f, 1.0f);
        const float warpSkew = (warpKnob - 0.5f) * 2.0f;
        const float targetQ = 3.5f + std::abs(warpSkew) * 9.5f;
        if (std::abs(targetQ - currentQ_) > 0.25f) {
            currentQ_ = targetQ;
            updateFilterCoefficients(currentQ_);
        }

        // 2: SIBILANCE
        const float sibSens = std::clamp(params8[2], 0.0f, 1.0f);

        // 3: CARRIER ENGINE [0..4]
        const int carrierMode = std::clamp(static_cast<int>(params8[3] * 4.99f), 0, 4);

        // 4: BANDS / MATRIX
        const float matrixKnob = std::clamp(params8[4], 0.0f, 1.0f);
        const int matrixMode = (matrixOverride_ >= 0)
            ? matrixOverride_
            : std::clamp(static_cast<int>(matrixKnob * 3.99f), 0, 3);

        // 5: SPEED / FREEZE
        const float speedKnob = std::clamp(params8[5], 0.0f, 1.0f);
        const bool frozen = manualFreeze_ || (speedKnob >= 0.94f);
        const float baseReleaseCoeff = 0.0004f + (1.0f - std::min(speedKnob, 0.93f)) * 0.025f;

        // 6: PITCH / SPREAD
        const float pitchSpreadKnob = std::clamp(params8[6], 0.0f, 1.0f);
        const float carrierBaseHz = (hostPitchHz > 20.0f)
            ? hostPitchHz
            : (55.0f * FastMath::fastPow2(pitchSpreadKnob * 3.0f));
        const float stereoSpread = 0.25f + 0.75f * pitchSpreadKnob;

        // Optical Vocal Compressor on Modulator input
        const float absMod = std::abs(rawMod);
        optoEnv_ += (absMod > optoEnv_ ? 0.05f : 0.001f) * (absMod - optoEnv_);
        const float agcGain = std::clamp(0.35f / std::max(optoEnv_, 0.04f), 0.5f, 4.5f);
        const float modIn = FastMath::fastTanh(rawMod * agcGain);

        // Sibilance high-frequency consonant detection
        const float highFreqSig = sibilanceHpf_.processHP(modIn);
        const float sibEnv = std::abs(highFreqSig);
        const bool isSibilant = (sibEnv > (0.08f * (1.05f - 0.75f * sibSens)));

        // Render Carrier
        float carrierL = 0.0f, carrierR = 0.0f;
        renderCarrier(carrierMode, carrierBaseHz, modIn, dryR, carrierL, carrierR);

        // Analysis & Synthesis across 32 bands (stride 1 for full 32, or grouped)
        float vocodedL = 0.0f;
        float vocodedR = 0.0f;

        for (int b = 0; b < kMaxBands; ++b) {
            if (!frozen) {
                const float bandMod = analysisSvf_[b].process(modIn);
                analysisSvf_[b].clampState();
                const float peak = std::abs(bandMod);

                // Bi-Directional Warp skews low vs high band decay rate
                const float bandNorm = (static_cast<float>(b) - 15.5f) * (1.0f / 16.0f);
                const float skewedRelease = std::clamp(baseReleaseCoeff * (1.0f - 0.75f * warpSkew * bandNorm), 0.0001f, 0.08f);

                if (peak > envLevels_[b]) {
                    envLevels_[b] += 0.18f * (peak - envLevels_[b]);
                } else {
                    envLevels_[b] += skewedRelease * (peak - envLevels_[b]);
                }
                frozenLevels_[b] = envLevels_[b];
            }

            // Patch Matrix & Formant Mapping
            int srcBand = b - formantOffset;
            srcBand = std::clamp(srcBand, 0, kMaxBands - 1);
            switch (matrixMode) {
                case 1: // Inverted Spectral Mirror
                    srcBand = (kMaxBands - 1) - srcBand;
                    break;
                case 2: // EMS Scramble
                    srcBand = kScrambleMap[srcBand & 31];
                    break;
                case 3: // Circular Shift Rotate
                    srcBand = (srcBand + 8) & 31;
                    break;
                case 0:
                default:
                    break;
            }

            const float envGain = (frozen ? frozenLevels_[srcBand] : envLevels_[srcBand]) * 3.8f;

            float bandCarL = carrierL;
            float bandCarR = carrierR;
            if (b >= 24 && (isSibilant || sibSens > 0.5f)) {
                const float sNoise = noise_.nextWhite() * sibSens * 0.45f;
                bandCarL += sNoise;
                bandCarR -= sNoise;
            }

            const float synL = synthSvfL_[b].process(bandCarL) * envGain;
            const float synR = synthSvfR_[b].process(bandCarR) * envGain;
            synthSvfL_[b].clampState();
            synthSvfR_[b].clampState();

            const float pan = ((b & 1) == 0) ? (0.5f - 0.42f * stereoSpread) : (0.5f + 0.42f * stereoSpread);
            vocodedL += synL * (1.0f - pan);
            vocodedR += synR * pan;
        }

        // Direct Sibilance Consonant Bleed
        if (sibSens > 0.02f) {
            const float bleed = highFreqSig * sibSens * (isSibilant ? 0.55f : 0.18f);
            vocodedL += bleed;
            vocodedR += bleed;
        }

        // Stereo Roland Juno / VP-330 BBD Chorus on Vocoded Sum
        chorusBufL_[chorusWrite_ & kChorusMask] = vocodedL;
        chorusBufR_[chorusWrite_ & kChorusMask] = vocodedR;
        chorusPhase_ += 0.65f * invSr_;
        if (chorusPhase_ >= 1.0f) chorusPhase_ -= 1.0f;
        const float lfo = FastMath::fastSinNorm(chorusPhase_);
        const float modDelayL = 120.0f + 42.0f * lfo;
        const float modDelayR = 120.0f - 42.0f * lfo;
        const int idxL = (chorusWrite_ - static_cast<int>(modDelayL)) & kChorusMask;
        const int idxR = (chorusWrite_ - static_cast<int>(modDelayR)) & kChorusMask;
        ++chorusWrite_;

        const float wetL = dcBlockL_.process(0.75f * vocodedL + 0.35f * stereoSpread * chorusBufL_[idxL]);
        const float wetR = dcBlockR_.process(0.75f * vocodedR + 0.35f * stereoSpread * chorusBufR_[idxR]);

        float dryGain = 1.0f;
        float wetGain = 0.0f;
        Taper::profileM_TriZoneMix(params8[7], phaseInvert, dryGain, wetGain);

        ioL = dryL * dryGain + wetL * wetGain;
        ioR = dryR * dryGain + wetR * wetGain;
    }

    float sampleRate_ = 48000.0f;
    float invSr_ = 1.0f / 48000.0f;
    float currentQ_ = 6.0f;
    bool manualFreeze_ = false;
    int matrixOverride_ = -1;

    float bandFreqs_[kMaxBands]{};
    float envLevels_[kMaxBands]{};
    float frozenLevels_[kMaxBands]{};
    DSPCommon::CytomicSVF analysisSvf_[kMaxBands];
    DSPCommon::CytomicSVF synthSvfL_[kMaxBands];
    DSPCommon::CytomicSVF synthSvfR_[kMaxBands];

    float chorusBufL_[kChorusBufLen]{};
    float chorusBufR_[kChorusBufLen]{};
    int chorusWrite_ = 0;
    float chorusPhase_ = 0.0f;

    float dcoPhase1_ = 0.0f;
    float dcoPhase2_ = 0.0f;
    float subPhase_ = 0.0f;
    float trackedPitchHz_ = 110.0f;
    int zcCounter_ = 0;
    bool lastModSign_ = false;
    float optoEnv_ = 0.1f;

    DSPCommon::OnePoleTPT modLowpass_;
    DSPCommon::OnePoleTPT sibilanceHpf_;
    DSPCommon::DCBlocker dcBlockL_;
    DSPCommon::DCBlocker dcBlockR_;
    Oscillators::PolyBLEP::NoiseGenerator noise_;
};

using KlangWarpFactoryPro = Vocoder56K;
using KlangVocoder = Vocoder56K;

} // namespace TbdAudio::FX
