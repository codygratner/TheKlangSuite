#pragma once

#include "SoundEngine.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace TbdAudio::Oscillators {

/**
 * @brief Pluck (OSC-05) — Merged Karplus-Strong Physical Waveguide, Mutable Instruments
 *        Rings Modal Resonator & Sympathetic String Synthesizer Engine.
 *
 * Unifies:
 *   1. Extended Karplus-Strong Waveguide (Lagrange 3rd-order fractional delay, pick-position
 *      comb filter, 1st-order all-pass stiffness dispersion, Padé fastTanh loop drive).
 *   2. MI Rings Modal Resonator Bank (8 tuned CytomicSVF bandpass modes per voice with
 *      inharmonic material stiffness dispersion: f_k = f_0 * k * sqrt(1 + beta * k^2)).
 *   3. Sympathetic String Coupling & 4-Voice Paraphonic Strumming (cycles across 4 voices
 *      on successive triggers so decaying strings ring out sympathetically!).
 *
 * 8 Performance Knobs (params8[0..7]):
 *   [0] OCTAVE        (-4..+4 quantized octaves)
 *   [1] DECAY         (Taper::profileA_Decay 10ms..10s RT60 feedback & modal decay)
 *   [2] DAMPING       (High-frequency brightness / material absorption loss)
 *   [3] EXCITER       (0: White Burst, 1: Pink Pluck, 2: Mallet Click, 3: Bow/Thump + Poly Mode)
 *   [4] POSITION      (0.05..0.50 Pick/Strike position comb notch & modal amplitude weighting)
 *   [5] STRUCTURE     (Inharmonic stiffness / material morph: Wood -> Nylon -> Steel -> Bell Plate)
 *   [6] BODY / MODAL  (Blend & resonance between Karplus Waveguide [0] and Modal Resonator + Sympathetic Strings [1])
 *   [7] DRIVE / POLY  (0.0..0.5 = Mono Slap/Waveguide Drive; 0.5..1.0 = 4-Voice Poly/Paraphonic Strum + Drive)
 */
class Pluck final : public SoundEngine8 {
public:
    static constexpr int kNumVoices = 4;
    static constexpr int kMaxDelay = 2048;
    static constexpr int kDelayMask = kMaxDelay - 1;
    static constexpr int kNumModes = 6;

    Pluck() noexcept {
        setParam(0, 0.50f); // Octave 0
        setParam(1, 0.65f); // Decay
        setParam(2, 0.35f); // Damping
        setParam(3, 0.30f); // Exciter
        setParam(4, 0.25f); // Pick / Strike Position
        setParam(5, 0.20f); // Structure / Stiffness
        setParam(6, 0.45f); // Body / Modal Blend
        setParam(7, 0.20f); // Drive / Poly Mode
        reset();
    }

    ~Pluck() override = default;

    void reset() noexcept override {
        for (int v = 0; v < kNumVoices; ++v) {
            std::fill(std::begin(voices_[v].delayBuf), std::end(voices_[v].delayBuf), 0.0f);
            std::fill(std::begin(voices_[v].pickBuf), std::end(voices_[v].pickBuf), 0.0f);
            voices_[v].writePos = 0;
            voices_[v].pickWritePos = 0;
            voices_[v].burstRemaining = 0;
            voices_[v].burstTotal = 1;
            voices_[v].thumpPhase = 0.0f;
            voices_[v].apX1 = 0.0f;
            voices_[v].apY1 = 0.0f;
            voices_[v].freqHz = 110.0f * (1.0f + 0.002f * static_cast<float>(v));
            voices_[v].velocity = 0.8f;
            voices_[v].dampingFilter.reset();
            voices_[v].dcBlocker.reset();
            for (int m = 0; m < kNumModes; ++m) {
                voices_[v].modalSvf[m].reset();
            }
        }
        activeVoice_ = 0;
        bodyFilterL_.reset();
        bodyFilterR_.reset();
        noise_.reset(0x9E3779B9u);
        needsAutoTrigger_ = true;
    }

    void setPolyMode(bool enabled) noexcept { forcePolyMode_ = enabled; }
    [[nodiscard]] bool isPolyMode() const noexcept { return forcePolyMode_ || (params8[7] >= 0.55f); }

protected:
    void onPrepare(float sampleRate) noexcept override {
        reset();
        for (int v = 0; v < kNumVoices; ++v) {
            voices_[v].dcBlocker.init(sampleRate);
        }
    }

    void onTrigger(float velocity) noexcept override {
        needsAutoTrigger_ = false;
        const float vel = std::clamp(velocity, 0.05f, 1.0f);
        velocity_ = vel;

        const bool poly = isPolyMode();
        if (poly) {
            activeVoice_ = (activeVoice_ + 1) & (kNumVoices - 1);
        } else {
            activeVoice_ = 0;
        }

        VoiceState& v = voices_[activeVoice_];
        const int oct = quantizeOctave4(params8[0]);
        const float octMult = FastMath::fastPow2(static_cast<float>(oct));
        v.freqHz = std::clamp(basePitchHz_ * octMult, 18.0f, std::min(8000.0f, 0.42f * sampleRate_));
        v.velocity = vel;

        const float exciterKnob = std::clamp(params8[3], 0.0f, 1.0f);
        const int excIdx = std::clamp(static_cast<int>(exciterKnob * 3.99f), 0, 3);
        const float subPos = (exciterKnob * 4.0f) - static_cast<float>(excIdx);
        const float burstMs = 0.0025f + 0.0145f * std::clamp(subPos, 0.0f, 1.0f);
        v.burstTotal = std::clamp(static_cast<int>(burstMs * sampleRate_), 8, kMaxDelay / 2);
        v.burstRemaining = v.burstTotal;
        v.thumpPhase = 0.0f;

        // In poly/paraphonic mode, tune idle voices to sympathetic chord intervals (Root, 5th, Oct, Maj/Min 10th)
        if (poly && !hasTriggeredOnce_) {
            hasTriggeredOnce_ = true;
            constexpr float kChordRatios[4] = { 1.0f, 1.498307f, 2.0f, 2.519842f };
            for (int i = 0; i < kNumVoices; ++i) {
                voices_[i].freqHz = std::clamp(v.freqHz * kChordRatios[i], 18.0f, 0.42f * sampleRate_);
            }
        }
    }

public:
    void renderSampleStereo(float baseFreqHz, float& outL, float& outR) noexcept override {
        if (needsAutoTrigger_) {
            basePitchHz_ = (baseFreqHz > 15.0f) ? baseFreqHz : 110.0f;
            onTrigger(velocity_);
        }

        const float sr = std::max(sampleRate_, 8000.0f);
        const float invSr = invSr_;
        const int oct = quantizeOctave4(params8[0]);
        const float octMult = FastMath::fastPow2(static_cast<float>(oct));
        const float currentTargetHz = std::clamp(baseFreqHz * octMult, 18.0f, std::min(8000.0f, 0.42f * sr));

        const bool poly = isPolyMode();
        if (!poly) {
            voices_[0].freqHz = currentTargetHz;
        } else {
            voices_[activeVoice_].freqHz = currentTargetHz;
        }

        // 1: DECAY (RT60 in [0.015s .. 10.0s])
        const float rt60Sec = Taper::profileA_Decay(params8[1], 0.015f, 10.0f);
        const float dampKnob = std::clamp(params8[2], 0.0f, 1.0f);
        const int exciterType = std::clamp(static_cast<int>(params8[3] * 3.99f), 0, 3);
        const float pickRatio = 0.05f + 0.45f * std::clamp(params8[4], 0.0f, 1.0f);
        const float structureKnob = std::clamp(params8[5], 0.0f, 1.0f);
        const float modalMix = std::clamp(params8[6], 0.0f, 1.0f);

        // Extract drive from Knob 7 (0..0.5 in mono, 0.5..1.0 in poly)
        const float rawDrive = std::clamp(params8[7], 0.0f, 1.0f);
        const float driveKnob = poly ? ((rawDrive - 0.55f) * 2.22f) : (rawDrive * 1.8f);
        const float clampedDrive = std::clamp(driveKnob, 0.0f, 1.0f);

        const int numActive = poly ? kNumVoices : 1;
        float sumL = 0.0f;
        float sumR = 0.0f;

        // Calculate sympathetic coupling energy from previous sample
        const float sympCoupling = sympatheticState_ * (0.018f * modalMix);
        float nextSymp = 0.0f;

        for (int vIdx = 0; vIdx < numActive; ++vIdx) {
            VoiceState& v = voices_[vIdx];
            const float f0 = std::clamp(v.freqHz, 18.0f, 0.42f * sr);
            const float totalDelaySamples = std::clamp(sr / f0, 2.5f, static_cast<float>(kMaxDelay - 8));

            const float periodsPerDecay = std::max(f0 * rt60Sec, 0.25f);
            const float fbGain = std::clamp(FastMath::fastPow2(-9.96578428f / periodsPerDecay), 0.55f, 0.99985f);

            const float dampCutoffHz = std::clamp(
                Taper::profileD_Cutoff(1.0f - dampKnob * 0.90f, f0 * 1.25f, std::min(18000.0f, 0.46f * sr)),
                60.0f, 0.46f * sr);
            v.dampingFilter.setCutoff(dampCutoffHz, invSr);

            // Exciter generation
            float exciterOut = 0.0f;
            if (v.burstRemaining > 0) {
                const float env = static_cast<float>(v.burstRemaining) / static_cast<float>(std::max(v.burstTotal, 1));
                const float envWin = env * env;
                switch (exciterType) {
                    case 0: exciterOut = noise_.nextWhite() * envWin; break;
                    case 1: exciterOut = noise_.nextPink() * envWin * 1.35f; break;
                    case 2: {
                        const float spike = (v.burstRemaining > v.burstTotal - 4) ? 1.0f : -0.25f;
                        exciterOut = (0.75f * spike + 0.25f * noise_.nextWhite()) * envWin;
                        break;
                    }
                    case 3:
                    default: {
                        const float sweepHz = f0 * (1.0f + 2.5f * envWin);
                        v.thumpPhase = PolyBLEP::wrapPhase(v.thumpPhase + sweepHz * invSr);
                        exciterOut = FastMath::fastTanh((FastMath::fastSinNorm(v.thumpPhase) * 1.6f + 0.3f * noise_.nextPink()) * envWin);
                        break;
                    }
                }
                exciterOut *= (0.35f + 0.65f * v.velocity);
                --v.burstRemaining;
            }

            // Pick position comb filter
            const int pickDelay = std::clamp(static_cast<int>(totalDelaySamples * pickRatio + 0.5f), 1, kMaxDelay - 2);
            v.pickBuf[v.pickWritePos & kDelayMask] = exciterOut;
            const float delayedPick = v.pickBuf[(v.pickWritePos - pickDelay) & kDelayMask];
            ++v.pickWritePos;
            const float combedExcitation = 0.75f * (exciterOut - delayedPick) + sympCoupling;

            // All-pass stiffness dispersion inside waveguide
            const float apCoeff = -0.68f * structureKnob;
            const float apPhaseDelay = (1.0f - std::abs(apCoeff)) / (1.0f + std::abs(apCoeff) + 1e-5f);
            const float effectiveDelay = std::clamp(totalDelaySamples - apPhaseDelay - 0.5f, 2.0f, static_cast<float>(kMaxDelay - 6));

            const float loopRead = readLagrange3(v, effectiveDelay);
            float loopSig = v.dampingFilter.processLP(loopRead) * fbGain;

            const float apOut = apCoeff * loopSig + v.apX1 - apCoeff * v.apY1;
            v.apX1 = loopSig;
            v.apY1 = apOut;
            loopSig = apOut;

            if (clampedDrive > 0.01f) {
                const float driveGain = 1.0f + clampedDrive * clampedDrive * 3.0f;
                const float normGain = 1.0f / FastMath::fastTanh(driveGain);
                loopSig = FastMath::fastTanh(loopSig * driveGain) * (0.985f * std::min(normGain, 1.25f));
            }

            const float waveguideOut = v.dcBlocker.process(combedExcitation + loopSig);
            v.delayBuf[v.writePos & kDelayMask] = std::clamp(std::isfinite(waveguideOut) ? waveguideOut : 0.0f, -4.0f, 4.0f);
            ++v.writePos;

            // MI Rings-Style Modal Resonator Bank (6 modes with inharmonic stiffness dispersion)
            float modalSum = 0.0f;
            if (modalMix > 0.02f) {
                const float beta = structureKnob * structureKnob * 0.085f;
                const float baseQ = (12.0f + 160.0f * (1.0f - dampKnob * 0.75f)) * (0.5f + 0.5f * fbGain);
                for (int m = 0; m < kNumModes; ++m) {
                    const float k = static_cast<float>(m + 1);
                    // Inharmonic modal dispersion: f_k = f_0 * k * sqrt(1 + beta * k^2)
                    const float disp = 1.0f + 0.5f * beta * k * k;
                    const float modeHz = f0 * k * disp;
                    if (modeHz < 0.44f * sr) {
                        const float modeQ = std::clamp(baseQ / (1.0f + 0.35f * k * dampKnob), 2.0f, 30.0f);
                        v.modalSvf[m].setBandpass(modeHz, modeQ, invSr);
                        // Strike position spatial weighting: sin(k * pi * pickRatio)
                        const float weight = FastMath::fastSinNorm(0.5f * k * pickRatio) / (1.0f + 0.25f * k);
                        modalSum += v.modalSvf[m].process(combedExcitation) * weight * 2.2f;
                        v.modalSvf[m].clampState();
                    }
                }
            }

            const float voiceOut = (1.0f - 0.55f * modalMix) * waveguideOut + (0.85f * modalMix) * modalSum;
            nextSymp += voiceOut;

            const float pan = poly ? (0.20f + 0.60f * (static_cast<float>(vIdx) / 3.0f)) : 0.5f;
            sumL += voiceOut * (1.0f - pan) * 1.35f;
            sumR += voiceOut * pan * 1.35f;
        }

        sympatheticState_ = std::clamp(nextSymp * 0.25f, -2.0f, 2.0f);

        // Wooden Body Cavity Resonance Filter
        const float bodyFreqL = std::clamp(currentTargetHz * 2.1f + modalMix * 380.0f, 85.0f, 0.42f * sr);
        const float bodyFreqR = std::clamp(bodyFreqL * 1.15f, 95.0f, 0.42f * sr);
        const float bodyQ = 0.85f + 3.5f * modalMix * modalMix;
        const float bodyGainDb = 1.5f + 9.0f * modalMix;

        bodyFilterL_.setBell(bodyFreqL, bodyQ, bodyGainDb, invSr);
        bodyFilterR_.setBell(bodyFreqR, bodyQ, bodyGainDb, invSr);

        float sigL = bodyFilterL_.process(sumL);
        float sigR = bodyFilterR_.process(sumR);
        bodyFilterL_.clampState();
        bodyFilterR_.clampState();

        if (phaseInvert) {
            sigL = -sigL;
            sigR = -sigR;
        }

        outL = std::clamp(std::isfinite(sigL) ? sigL : 0.0f, -4.0f, 4.0f);
        outR = std::clamp(std::isfinite(sigR) ? sigR : 0.0f, -4.0f, 4.0f);
    }

private:
    struct VoiceState {
        float delayBuf[kMaxDelay]{};
        float pickBuf[kMaxDelay]{};
        int writePos = 0;
        int pickWritePos = 0;
        int burstRemaining = 0;
        int burstTotal = 1;
        float thumpPhase = 0.0f;
        float apX1 = 0.0f;
        float apY1 = 0.0f;
        float freqHz = 110.0f;
        float velocity = 0.8f;
        DSPCommon::OnePoleTPT dampingFilter;
        DSPCommon::DCBlocker dcBlocker;
        DSPCommon::CytomicSVF modalSvf[kNumModes];
    };

    [[nodiscard]] static inline float readLagrange3(const VoiceState& v, float delaySamples) noexcept {
        const float clampedDelay = std::clamp(delaySamples, 1.5f, static_cast<float>(kMaxDelay - 5));
        int dInt = static_cast<int>(clampedDelay);
        if (dInt < 1) dInt = 1;
        const float frac = clampedDelay - static_cast<float>(dInt);

        const int idx0 = (v.writePos - dInt + 1) & kDelayMask;
        const int idx1 = (v.writePos - dInt)     & kDelayMask;
        const int idx2 = (v.writePos - dInt - 1) & kDelayMask;
        const int idx3 = (v.writePos - dInt - 2) & kDelayMask;

        const float y0 = v.delayBuf[idx0];
        const float y1 = v.delayBuf[idx1];
        const float y2 = v.delayBuf[idx2];
        const float y3 = v.delayBuf[idx3];

        const float d0 = frac;
        const float d1 = frac - 1.0f;
        const float d2 = frac - 2.0f;
        const float d3 = frac + 1.0f;

        const float c0 = -d0 * d1 * d2 * (1.0f / 6.0f);
        const float c1 =  d3 * d1 * d2 * 0.5f;
        const float c2 = -d3 * d0 * d2 * 0.5f;
        const float c3 =  d3 * d0 * d1 * (1.0f / 6.0f);

        return c0 * y0 + c1 * y1 + c2 * y2 + c3 * y3;
    }

    VoiceState voices_[kNumVoices];
    int activeVoice_ = 0;
    float sympatheticState_ = 0.0f;
    bool forcePolyMode_ = false;
    bool hasTriggeredOnce_ = false;
    bool needsAutoTrigger_ = true;

    DSPCommon::CytomicSVF bodyFilterL_;
    DSPCommon::CytomicSVF bodyFilterR_;
    PolyBLEP::NoiseGenerator noise_;
};

using KlangPluckPoly = Pluck;

} // namespace TbdAudio::Oscillators
