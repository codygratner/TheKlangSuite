#pragma once

#include "AlphaOscillator.h"
#include "SampleHoldNoise.h"
#include "DualSvfFilter.h"
#include "AllPassFilter4.h"
#include "CodyBassEnvelope.h"
#include "VortexEnvelope.h"
#include "VactrolLPG.h"
#include "HalfBandOversampler2x.h"
#include "CyborgBandLimiter.h"
#include "ModMatrix.h"
#include "../FastMath.h"
#include <cmath>
#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>

namespace CrackedChimes::DSP {
namespace FastMath = TbdAudio::FastMath;


/**
 * @brief Cracked Chimes Hastur Monosynth Voice Engine (Workhorse Edition).
 * 
 * SOTA Single-Folio Acid & Bass Synthesizer:
 * - Dual Divided Logic DCO Cores (Osc 1 Core & Osc 2 Sync/Offset) with 18 PolyBLEP shapes.
 * - Dual Pitch-Tracking 4-Pole All-Pass Filters with S-Curve Octave Dispersion (-1..0..+2 Oct) & Tanh saturation.
 * - Formant Sync Modes between Osc 1 and Osc 2 (Hard, Soft, Metal, APF Hard, APF Soft, APF Metal).
 * - Dedicated 5-Lane Mixer Block (Osc 1 Raw, Osc 1 APF, Osc 2 Raw, Osc 2 APF, Noise S&H) with Tanh Summing Saturation.
 * - Extended Dual-SVF VCF with 4 Pole Topologies (1/3, 2/2, 3/1, 4-Loop) and Mid-Filter APF+Tanh.
 * - Dedicated Amp Block with 3 Gate Modes (Envelope, Gate, Drone) and Master Post-Amp Saturation.
 * - Universal Dual-Channel Audio-Rate Cross-Modulation Matrix (Mod A & Mod B).
 * - Cody's Precision Bass Envelope (Staccato 1:99 to 99:1 Boom) and Global Sine/Noise LFOs.
 * - Fleet of Meng Qi DPLPG Optocoupler Cells and 3-Tier Life Modes (Synthetic, Cyborg, Organic).
 */
class HasturVoice {
public:
    [[nodiscard]] static constexpr float slidingWindowShift(float k, float lo, float hi, float amount, float s) noexcept {
        const float span = hi - lo;
        const float w = std::clamp(amount, 0.0f, 1.0f) * span;
        const float a = std::clamp(k - 0.5f * w, lo, hi - w);
        const float u = 0.5f + 0.5f * std::clamp(s, -1.0f, 1.0f);
        return a + w * u;
    }

    [[nodiscard]] static constexpr float boundedShift(float k, float lo, float hi, float amount, float s) noexcept {
        const float a = std::clamp(amount, 0.0f, 1.0f);
        const float x = a * std::clamp(s, -1.0f, 1.0f);
        return k + x * (x >= 0.0f ? (hi - k) : (k - lo));
    }

public:
    enum class LifeMode : uint8_t {
        SYNTHETIC = 0,
        CYBORG    = 1,
        ORGANIC   = 2
    };

    enum class LfoWave : uint8_t {
        TRIANGLE = 0,
        PULSE    = 1,
        SINE     = 2,
        RANDOM   = 3
    };

    enum class LfoTriggerMode : uint8_t {
        FREE_RUNNING  = 0,
        ALL_NEW_NOTES = 1,
        LEGATO        = 2,
        ONE_SHOT      = 3,
        HALF_SHOT     = 4
    };

    enum class AmpGateMode : uint8_t {
        ENVELOPE = 0,
        GATE     = 1,
        DRONE    = 2
    };

    void prepare(double sampleRate) noexcept {
        hostSampleRate_ = (sampleRate > 1000.0) ? sampleRate : 44100.0;
        sampleRate_ = hostSampleRate_;
        invSampleRate_ = 1.0 / sampleRate_;

        const double voiceCore4xSr = hostSampleRate_ * 4.0;
        const double voiceFilter2xSr = hostSampleRate_ * 2.0;

        // DCO prepared at 4x rate (192 kHz)
        osc1_.prepare(voiceCore4xSr);
        osc2_.prepare(voiceCore4xSr);

        // VCF & Master prepared at 2x rate (96 kHz)
        filter_.prepare(voiceFilter2xSr);
        filterR_.prepare(voiceFilter2xSr);

        // Fleet of Meng Qi DPLPG Cells
        osc1Lpg_.prepare(hostSampleRate_);
        osc2Lpg_.prepare(hostSampleRate_);
        noiseLpg_.prepare(hostSampleRate_);
        masterLpg_.prepare(voiceFilter2xSr);
        masterLpgR_.prepare(voiceFilter2xSr);

        // Multi-Rate Decimators & Cyborg Bus Filters
        decimator4xTo2x_.reset();
        decimator_.reset();
        cyborgBusFilterL_.prepare(voiceFilter2xSr);
        cyborgBusFilterR_.prepare(voiceFilter2xSr);

        noiseGen_.prepare(hostSampleRate_);
        masterSideHp_.setCutoff(120.0f, static_cast<float>(hostSampleRate_));
        apfChorusL_.prepare(hostSampleRate_);
        apfChorusR_.prepare(hostSampleRate_);
        vortexEnv_.prepare(hostSampleRate_);
        ampEnv_.prepare(hostSampleRate_);

        // Virtual Vactrol CV Slewers (Organic Mode)
        cutoffCv_.prepare(hostSampleRate_);
        resonanceCv_.prepare(hostSampleRate_);
        widthCv_.prepare(hostSampleRate_);
        osc1OffsetCv_.prepare(hostSampleRate_);
        osc2OffsetCv_.prepare(hostSampleRate_);
        slewCv_.prepare(hostSampleRate_);
        mixerDriveCv_.prepare(hostSampleRate_);
        bendCoeff_ = 1.0f - std::exp(-1.0f / (0.005f * static_cast<float>(hostSampleRate_)));
        slopLeak_ = std::exp(-static_cast<float>(invSampleRate_) / 0.30f);
        slopInnov_ = std::sqrt(std::max(0.0f, 1.0f - slopLeak_ * slopLeak_)) * 0.198f;

        reset();
    }

    void reset() noexcept {
        decimator4xTo2x_.reset();
        decimator_.reset();
        osc1_.resetSubPhase();
        cyborgBusFilterL_.reset();
        cyborgBusFilterR_.reset();
        adaaAmpL_ = 0.0f;
        adaaAmpR_ = 0.0f;
        adaaDiodeL_ = 0.0f;
        adaaDiodeR_ = 0.0f;
        prevAmpGateSig_ = 0.0f;
        lfoPhase_ = 0.0f;
        lfoRandomVal_ = 0.0f;
        lfoRandomPrev_ = 0.0f;
        lfoRunning_ = true;

        for (int i = 0; i < 9; ++i) {
            stepRand_[i] = random01();
            stepRandAtom_[i].store(stepRand_[i], std::memory_order_relaxed);
        }

        pwLfoPhase_ = 0.0f;
        effMasterSpread_ = 0.0f;
        bbdSampleCounter_ = false;
        bbdHoldL_ = 0.0f;
        bbdHoldR_ = 0.0f;

        slopWander1_ = 0.0f;
        slopWander2_ = 0.0f;
        slopPrng_ = 0x5EED1234u;
        bendTargetSemi_ = 0.0f;
        bendSemi_ = 0.0f;
        vibPhase_ = 0.0f;

        osc1Lpg_.reset();
        osc2Lpg_.reset();
        noiseLpg_.reset();
        masterLpg_.reset();

        osc1_.reset();
        osc2_.reset();
        noiseGen_.reset();
        filter_.reset();
        vortexEnv_.reset();
        ampEnv_.reset();
        currentPitchHz_ = 130.81f;
        targetPitchHz_ = 130.81f;

        cutoffCv_.reset(1200.0f);
        resonanceCv_.reset(0.25f);
        widthCv_.reset(0.5f);
        osc1OffsetCv_.reset(0.0f);
        osc2OffsetCv_.reset(0.0f);
        slewCv_.reset(0.3f);
        mixerDriveCv_.reset(0.0f);

        prevOsc1_ = 0.0f;
        prevOsc2_ = 0.0f;
        prevOsc1Trigger_ = false;
        prevOsc2Trigger_ = false;
        prevOsc1SyncFrac_ = 0.0f;
        prevOsc2SyncFrac_ = 0.0f;
        prevNoise_ = 0.0f;
        prevPreFilter_ = 0.0f;
        prevPostFilter_ = 0.0f;

        filterR_.reset();
        masterLpgR_.reset();
        masterSideHp_.reset();
        apfChorusL_.reset();
        apfChorusR_.reset();
        chorusLfoPhase_ = 0.0f;
        subCrossoverLpL_ = 0.0f;
        subCrossoverLpR_ = 0.0f;
        fbDelayBuffer_.fill(0.0f);
        fbDelayWriteIdx_ = 0;
        fbFilterState_ = 0.0f;
        fbDcStateX_ = 0.0f;
        fbDcStateY_ = 0.0f;
        noteStackSize_ = 0;
        for (auto& n : noteStack_) n = {};

        activeNoteCount_ = 0;
        gate_ = false;
        currentSampleCount_ = 0;
        lastNoteOffSample_ = 0;
        lastPitchHz_ = 130.81f;
    }

    void setBackendMode(VactrolLPG::BackendMode mode) noexcept {
        backendMode_ = mode;
        osc1Lpg_.setMode(mode);
        osc2Lpg_.setMode(mode);
        noiseLpg_.setMode(mode);
        masterLpg_.setMode(mode);
        masterLpgR_.setMode(mode);
    }

    void setLifeMode(LifeMode mode) noexcept {
        lifeMode_ = mode;
        const auto bMode = (mode == LifeMode::SYNTHETIC)
            ? VactrolLPG::BackendMode::VCA
            : ((mode == LifeMode::CYBORG) ? VactrolLPG::BackendMode::CYBORG : VactrolLPG::BackendMode::LPG);
        setBackendMode(bMode);
        const auto dspLife = static_cast<CrackedChimes::DSP::LifeMode>(mode);
        masterLpg_.setLifeMode(dspLife);
        masterLpgR_.setLifeMode(dspLife);
        osc1Lpg_.setLifeMode(dspLife);
        osc2Lpg_.setLifeMode(dspLife);
        noiseLpg_.setLifeMode(dspLife);
        osc1_.setLifeMode(static_cast<int>(mode));
        osc2_.setLifeMode(static_cast<int>(mode));
    }

    enum class GlideMode : uint8_t {
        ALWAYS = 0,
        LEGATO = 1
    };

    enum class TriggerMode : uint8_t {
        RETRIG = 0,
        LEGATO = 1,
        AUTO   = 2
    };

    [[nodiscard]] static float shapeLfo(LfoWave fam, float p, float s, float randVal, float randPrev) noexcept {
        switch (fam) {
            case LfoWave::TRIANGLE: { // Saw/Tri morphing: 0 = rev saw, 0.5 = tri, 1 = rising saw
                const float c = std::clamp(s, 0.0005f, 0.9995f);
                return (p < c) ? (-1.0f + 2.0f * (p / c))
                               : ( 1.0f - 2.0f * ((p - c) / (1.0f - c)));
            }
            case LfoWave::PULSE: { // Square/PWM morphing: 0 = 1% pulse, 0.5 = 50% square, 1 = 99% pulse
                const float d = std::clamp(0.01f + 0.98f * s, 0.01f, 0.99f);
                return (p < d) ? 1.0f : -1.0f;
            }
            case LfoWave::SINE: { // Sine with Infinifold: 0 = pure sine, 1 = deep multi-fold
                const float k = 1.0f + 15.0f * s * s;
                const float inner = FastMath::fastSinNorm(p);
                return FastMath::fastSinNorm(0.25f * k * inner);
            }
            case LfoWave::RANDOM:
            default: { // S&H Random: 0 = stepped S&H, 1 = smoothstep continuous random walk
                const float w = std::max(s, 1e-3f);
                float u = std::min(p / w, 1.0f);
                u = u * u * (3.0f - 2.0f * u); // smoothstep
                return randPrev + (randVal - randPrev) * u;
            }
        }
    }

    void snapLfoRandom() noexcept {
        lfoPrng_ = lfoPrng_ * 1664525u + 1013904223u;
        lfoRandomVal_  = static_cast<float>(static_cast<int32_t>(lfoPrng_)) * 4.6566129e-10f;
        lfoRandomPrev_ = lfoRandomVal_;
    }

    void setPitchBend(float norm, float rangeSemi) noexcept {
        bendTargetSemi_ = std::clamp(norm, -1.0f, 1.0f) * std::clamp(rangeSemi, 0.0f, 24.0f);
    }

    void setVibrato(float rateHz, float depth) noexcept {
        vibRateHz_ = std::clamp(rateHz, 0.1f, 12.0f);
        vibDepth_  = std::clamp(depth, 0.0f, 1.0f);
    }

    void noteOn(int midiNote, float velocity, float glideSec = 0.005f,
                LfoTriggerMode lfoTrig = LfoTriggerMode::ALL_NEW_NOTES, bool autoChoke = true,
                GlideMode glideMode = GlideMode::ALWAYS,
                TriggerMode trigMode = TriggerMode::AUTO) noexcept {
        const bool isHolding = (activeNoteCount_ > 0);
        activeNoteCount_++;

        // Renoise Single-Column Auto-Legato: 15ms grace window
        const double msSinceOff = (sampleRate_ > 0.0)
            ? (static_cast<double>(currentSampleCount_ - lastNoteOffSample_) * 1000.0 / sampleRate_)
            : 999.0;
        const bool autoLegatoEligible = (trigMode == TriggerMode::AUTO && !isHolding && msSinceOff <= 15.0 && lastPitchHz_ > 20.0f);
        const bool isLegato = isHolding || autoLegatoEligible;
        
        // Update mono note stack
        bool inStack = false;
        for (int i = 0; i < noteStackSize_; ++i) {
            if (noteStack_[i].noteNumber == midiNote) {
                inStack = true;
                NoteEntry e = noteStack_[i];
                e.velocity = velocity;
                for (int j = i; j < noteStackSize_ - 1; ++j) noteStack_[j] = noteStack_[j + 1];
                noteStack_[noteStackSize_ - 1] = e;
                break;
            }
        }
        if (!inStack && noteStackSize_ < 16) {
            noteStack_[noteStackSize_++] = NoteEntry{midiNote, velocity};
        }

        currentNote_ = midiNote;
        tgtSemi_ = static_cast<float>(midiNote);
        targetPitchHz_ = 440.0f * FastMath::fastPow2((tgtSemi_ - 69.0f) * (1.0f / 12.0f));
        bool shouldGlide = (glideSec > 0.0005f);
        if (glideMode == GlideMode::LEGATO && !isLegato) {
            shouldGlide = false; // LGTO mode: only glide if already holding a note
        }
        if (!shouldGlide) {
            curSemi_ = tgtSemi_;
            currentPitchHz_ = targetPitchHz_;
        }
        velocity_ = std::clamp(velocity, 0.01f, 1.0f);
        gate_ = true;

        const auto ampStage = ampEnv_.getStage();
        const bool envReleased = ampEnv_.isIdle() || ampStage == CodyBassEnvelope::Stage::RELEASE;
        const bool shouldRetrig = (trigMode == TriggerMode::RETRIG) || !isLegato || (!isHolding && envReleased);
        if (shouldRetrig) {
            vortexEnv_.noteOn(midiNote, velocity_);
            ampEnv_.noteOn(velocity_, autoChoke);
            if (ampEnv_.isIdle() || ampStage == CodyBassEnvelope::Stage::CHOKE) {
                osc1_.resetSubPhase();
            }

            // Sample fresh 9-element stepped random vector
            for (int i = 0; i < 9; ++i) {
                stepRand_[i] = random01();
                stepRandAtom_[i].store(stepRand_[i], std::memory_order_relaxed);
            }
        }

        // Handle LFO retrigger modes
        switch (lfoTrig) {
            case LfoTriggerMode::ALL_NEW_NOTES:
                lfoPhase_ = 0.0f;
                snapLfoRandom();
                lfoRunning_ = true;
                break;
            case LfoTriggerMode::LEGATO:
                if (!isLegato) {
                    lfoPhase_ = 0.0f;
                    snapLfoRandom();
                }
                lfoRunning_ = true;
                break;
            case LfoTriggerMode::ONE_SHOT:
            case LfoTriggerMode::HALF_SHOT:
                lfoPhase_ = 0.0f;
                snapLfoRandom();
                lfoRunning_ = true;
                break;
            case LfoTriggerMode::FREE_RUNNING:
            default:
                break;
        }
    }

    void noteOff(int midiNote = -1) noexcept {
        lastNoteOffSample_ = currentSampleCount_;
        lastPitchHz_ = currentPitchHz_;

        if (midiNote >= 0) {
            for (int i = 0; i < noteStackSize_; ++i) {
                if (noteStack_[i].noteNumber == midiNote) {
                    for (int j = i; j < noteStackSize_ - 1; ++j) {
                        noteStack_[j] = noteStack_[j + 1];
                    }
                    noteStackSize_--;
                    break;
                }
            }
        } else if (noteStackSize_ > 0) {
            noteStackSize_--;
        }

        if (activeNoteCount_ > 0) activeNoteCount_--;

        if (noteStackSize_ > 0 && activeNoteCount_ > 0) {
            currentNote_ = noteStack_[noteStackSize_ - 1].noteNumber;
            tgtSemi_ = static_cast<float>(currentNote_);
            targetPitchHz_ = 440.0f * FastMath::fastPow2((tgtSemi_ - 69.0f) * (1.0f / 12.0f));
        } else {
            gate_ = false;
            noteStackSize_ = 0;
            activeNoteCount_ = 0;
            vortexEnv_.noteOff();
            ampEnv_.noteOff();
        }
    }

    /**
     * @brief Process a single audio sample through the complete Hastur workhorse voice.
     */
    /**
     * @brief Process a single stereo audio sample with DCO 2.0, Dual-Control Sub, 5-Lane Mixer and Master SPREAD.
     */
    void processStereoSample(
        int waveFamily, int chopperTopo, float morph,
        float dcoMult, float dcoWarpOct, float dcoMelt,
        float dcoBite,
        float dcoSwarm,
        int subOctave, int subPulseTrain,
        float coreLevel, float subLevel, float swarmLevel, float noiseLevel, float mixerDrive,
        float globalWidth, float pwLfoRateHz, float pwLfoAmount, float widthEnvDepth,
        float lfoRateHz, LfoWave lfoWave, LfoTriggerMode lfoTrig,
        float glideTimeSec, float oscSlop,
        const ModMatrix::ChannelConfig& modA,
        const ModMatrix::ChannelConfig& modB,
        float cutoffHz, float resonance,
        int filterDeck, float filterWarp, float laserBite, float laserDrive,
        float filterOrbit, int hpfMode,
        float envDepth, float filterVelocity, float keyTrack, float filterLfoDepth, float modDepth,
        // Vortex Filter Envelope
        float filterAttack, float filterSnap, float filterDecay, float filterBloom,
        float filterAftershock, float filterSustain, float filterRelease, int filterLoop,
        // SubStator Amp Envelope + Embouchure Bite
        float ampSnap, float ampDecay, float ampCurvature, float ampSustain, float ampRelease,
        float pitchPunchDepth, float pitchPunchTime,
        AmpGateMode ampGateMode, float postAmpDrive, float masterSpread, float outputLevel,
        float& outLeft, float& outRight,
        float subApfDepth = 0.0f,
        int subApfTap = 0,
        float burbleDepth = 0.0f,
        int burbleMode = 1,
        float alphaChorus = 0.0f,
        float subSpit = 0.0f,
        float ampVelocity = 0.0f,
        int filterLfoDest = 0,
        float lfoShape = 0.5f,
        int lfoSync = 0) noexcept
    {
        currentSampleCount_++;
        const float nyquist = static_cast<float>(sampleRate_ * 0.48);

        // 10ms one-pole smoother across all life modes for click-free width sweeps
        const float widthSmoothCoeff = static_cast<float>(1.0 - std::exp(-invSampleRate_ / 0.010));
        effMasterSpread_ += widthSmoothCoeff * (masterSpread - effMasterSpread_);
        const float W = std::clamp(effMasterSpread_, -1.0f, 1.0f);
        const float spreadS = 1.0f - std::abs(W);
        const float orbitScaleO = (W < 0.0f) ? (1.0f + W) : 1.0f;

        // 1. Organic Vactrol CV Slew Smoothing (Organic Life Mode only)
        const bool isOrganic = (lifeMode_ == LifeMode::ORGANIC);
        const float effCutoffHz   = cutoffCv_.process(cutoffHz, isOrganic);
        const float effRes        = resonanceCv_.process(resonance, isOrganic);
        const float effWidth      = widthCv_.process(globalWidth, isOrganic);
        const float effMixerDrive = mixerDriveCv_.process(mixerDrive, isOrganic);

        // 2. Evaluate Vortex Filter Envelope -> Dual-SVF VCF Cutoff
        const auto loopMode = static_cast<VortexEnvelope::LoopMode>(std::clamp(filterLoop, 0, 2));
        // Acid Accent Detection & Modulation
        float accentFactor = 0.0f;
        if (velocity_ >= (100.0f / 127.0f)) {
            accentFactor = std::clamp((velocity_ - (100.0f / 127.0f)) / (27.0f / 127.0f), 0.0f, 1.0f);
        }
        const float resBump = accentFactor * filterVelocity * 0.15f;
        const float totalRes = std::clamp(effRes + resBump, 0.0f, 0.985f);
        const float envAccentBoost = 1.0f + (accentFactor * filterVelocity * 0.70f);

        const float filterEnvVal = vortexEnv_.process(
            filterAttack, filterSnap, filterDecay, filterBloom,
            filterAftershock, filterSustain, filterRelease, keyTrack, loopMode, filterVelocity);
        const float effEnvDepth = std::pow(std::clamp(envDepth, 0.0f, 1.0f), 1.25f);
        const float cutoffModSemitones = filterEnvVal * (effEnvDepth * envAccentBoost) * 96.0f;

        // 3. Evaluate SubStator Amp Envelope -> Master VCA & Pitch Punch
        float punchPitchSemi = 0.0f;
        const float ampEnvVal = ampEnv_.process(
            ampSnap, ampDecay, ampCurvature, ampSustain, ampRelease,
            pitchPunchDepth, pitchPunchTime, 0.5f, punchPitchSemi);

        // 4. Frequency Glide Slew towards target pitch (integrated in semitone domain)
        if (glideTimeSec <= 0.0005f || std::abs(tgtSemi_ - curSemi_) < 1e-4f) {
            curSemi_ = tgtSemi_;
        } else {
            const float T = std::max(0.0005f, glideTimeSec);
            const float diffSemi = tgtSemi_ - curSemi_;
            const float glideK = 1.0f - std::exp(-static_cast<float>(invSampleRate_) / (0.25f * T));
            const float maxStepSemi = (12.0f / T) * static_cast<float>(invSampleRate_);
            const float step = std::clamp(diffSemi * glideK, -maxStepSemi, maxStepSemi);
            curSemi_ += step;
            if (std::abs(tgtSemi_ - curSemi_) < 0.005f) {
                curSemi_ = tgtSemi_;
            }
        }
        currentPitchHz_ = 440.0f * FastMath::fastPow2((curSemi_ - 69.0f) * (1.0f / 12.0f));

        // 5. Compute Analog Slop Wander (Brownian Random Walk with differential Core/Sub detune)
        if (oscSlop > 0.001f) {
            slopPrng_ = slopPrng_ * 1664525u + 1013904223u;
            const float white1 = static_cast<float>(static_cast<int32_t>(slopPrng_)) * 4.6566129e-10f;
            slopWander1_ = slopWander1_ * slopLeak_ + white1 * slopInnov_;
            slopWander1_ = std::clamp(slopWander1_, -1.0f, 1.0f);
        } else {
            slopWander1_ = 0.0f;
        }

        // 4b. Smooth pitch bend & free-running global sine vibrato
        bendSemi_ += bendCoeff_ * (bendTargetSemi_ - bendSemi_);
        vibPhase_ += vibRateHz_ * static_cast<float>(invSampleRate_);
        if (vibPhase_ >= 1.0f) vibPhase_ -= std::floor(vibPhase_);
        constexpr float kVibMaxSemi = 2.0f; // full depth = +/- 2 semitones peak
        const float vibSemi = FastMath::fastSinNorm(vibPhase_) * (vibDepth_ * kVibMaxSemi);

        // Maximum drift: +/- 0.35 semitones (+/- 35 cents) at oscSlop = 1.0
        const float slopSemitones = slopWander1_ * (oscSlop * 0.35f);
        const float totalPitchOffsetSemi = slopSemitones + punchPitchSemi + bendSemi_ + vibSemi;
        float f1 = std::clamp(currentPitchHz_ * FastMath::fastPow2(totalPitchOffsetSemi * (1.0f / 12.0f)), 0.0001f, nyquist);

        // Differential Sub Slop: Sub drifts in the opposite direction
        osc1_.setSubDetuneSemitones(-2.0f * slopSemitones);

        // 5. Evaluate Global LFO
        float lfoVal = 0.0f;
        if (lfoRunning_) {
            const float lfoDt = std::clamp(lfoRateHz, 0.01f, 50.0f) * static_cast<float>(invSampleRate_);
            lfoPhase_ += lfoDt;
            if (lfoPhase_ >= 1.0f) {
                if (lfoTrig == LfoTriggerMode::ONE_SHOT) {
                    lfoRunning_ = false;
                    lfoPhase_ = 1.0f;
                } else {
                    lfoPhase_ -= std::floor(lfoPhase_);
                    lfoRandomPrev_ = lfoRandomVal_;
                    lfoPrng_ = lfoPrng_ * 1664525u + 1013904223u;
                    lfoRandomVal_ = static_cast<float>(static_cast<int32_t>(lfoPrng_)) * 4.6566129e-10f;
                }
            } else if (lfoTrig == LfoTriggerMode::HALF_SHOT && lfoPhase_ >= 0.5f) {
                lfoRunning_ = false;
            }
            lfoVal = shapeLfo(lfoWave, lfoPhase_, lfoShape, lfoRandomVal_, lfoRandomPrev_);
        }

        // 6. Evaluate Dedicated Sine Morph LFO
        const float pwLfoDt = std::clamp(pwLfoRateHz, 0.01f, 50.0f) * static_cast<float>(invSampleRate_);
        pwLfoPhase_ += pwLfoDt;
        if (pwLfoPhase_ >= 1.0f) pwLfoPhase_ -= std::floor(pwLfoPhase_);
        const float pwLfoVal = std::sin(pwLfoPhase_ * 6.28318530717958647692f);

        // 7. Modulated Morph with Bounded Headroom Shift (No Seams, Full Span)
        const float effMorph = slidingWindowShift(std::clamp(morph, 0.0f, 1.0f), 0.0f, 1.0f, pwLfoAmount, pwLfoVal);
        const float effPwm = 0.0f;

        // 8. Mod Matrix
        ModMatrix::Sources modSrc;
        modSrc.raw         = prevRaw_;
        modSrc.core        = prevCore_;
        modSrc.sub         = prevSub_;
        modSrc.osc1        = prevOsc1_;
        modSrc.osc2        = prevOsc2_;
        modSrc.noise       = prevNoise_;
        modSrc.preFilter   = prevPreFilter_;
        modSrc.postFilter  = prevPostFilter_;

        const auto modRes = ModMatrix::compute(modA, modB, modSrc);
        f1 = std::clamp((f1 + modRes.linFmOsc1Hz) * modRes.expFmOsc1Mult, 1.0f, static_cast<float>(sampleRate_ * 1.92f));

        // 9. DCO 2.0 Timbre Core & 5-Lane Stem Mixer Summation
        osc1_.setWaveFamily(waveFamily);
        osc1_.setChopperTopology(chopperTopo);
        osc1_.setMorph(effMorph);
        osc1_.setBite(dcoBite);
        osc1_.setMult(dcoMult);
        osc1_.setApfOctaveOffset(dcoWarpOct);
        osc1_.setSubOctave(subOctave);
        osc1_.setSubPulseTrain(subPulseTrain);
        osc1_.setSubApfDepth(subApfDepth);
        osc1_.setSubApfTap(subApfTap);
        osc1_.setSubSpit(subSpit);
        osc1_.setSwarmDepth(dcoSwarm);
        osc1_.setSuperSawSpread(dcoSwarm);
        osc1_.setCoreLevel(coreLevel);
        osc1_.setSubLevel(subLevel);
        osc1_.setSuperSawLevel(swarmLevel);
        osc1_.setNoiseLevel(noiseLevel);
        osc1_.setMixerDrive(effMixerDrive);
        osc1_.setMasterSpread(1.0f); // Pre-filter stereo preserved, Master Spread applied at Card 4

        // 10. Filter Frequency Modulation & Keytracking (FastMath Vectorized)
        const float keyTrackSemitones = (static_cast<float>(currentNote_) - 60.0f) * keyTrack;
        const float keyTrackMult = FastMath::fastPow2(keyTrackSemitones * (1.0f / 12.0f));
        float lfoCutoffSemi = 0.0f;
        float effOrbit = filterOrbit;
        if (filterLfoDest == 1) {
            effOrbit = boundedShift(filterOrbit, -1.0f, 1.0f, filterLfoDepth, lfoVal);
        } else {
            // Clamped Asymmetric Semitone Headroom [10Hz, 0.45*fs] with +/-60st musical cap
            const float fMin = 10.0f;
            const float fMax = static_cast<float>(hostSampleRate_ * 0.45);
            const float fBase = std::clamp(effCutoffHz * keyTrackMult * FastMath::fastPow2(cutoffModSemitones * (1.0f / 12.0f)), fMin, fMax);
            const float logBase = std::log2(fBase);
            const float hiSt = std::min(60.0f, 12.0f * (std::log2(fMax) - logBase));
            const float loSt = std::min(60.0f, 12.0f * (logBase - std::log2(fMin)));
            const float x = std::clamp(lfoVal * filterLfoDepth, -1.0f, 1.0f);
            lfoCutoffSemi = (x >= 0.0f) ? (x * hiSt) : (x * loSt);
        }
        // Scale orbit by bipolar width orbit factor O:
        // W < 0 => collapses to 0 for even filters (Coherent True Mono)
        // W > 0 => enforces minimum dual-peak split (kDualPeakMin = 0.30f)
        if (W < 0.0f) {
            effOrbit *= (1.0f + W);
        } else {
            constexpr float kDualPeakMin = 0.30f; // ~ +/-0.6 oct at W = +1
            const float sgn = (effOrbit < 0.0f) ? -1.0f : 1.0f;
            effOrbit = sgn * std::max(std::abs(effOrbit), W * kDualPeakMin);
        }
        const float totalModMult = FastMath::fastPow2((cutoffModSemitones + lfoCutoffSemi) * (1.0f / 12.0f));

        float filteredL = 0.0f, filteredR = 0.0f;
        float mixL = 0.0f, mixR = 0.0f;

        // 11. Multi-Rate Centaur Architecture: 4x DCO Core -> 2x VCF -> 1x Host Output
        const float coreNyquist4x = static_cast<float>(hostSampleRate_ * 1.92);
        const float effOsc1ApfCutoff = std::clamp(std::abs(f1) * std::pow(2.0f, dcoWarpOct), 10.0f, coreNyquist4x);
        const float dt1_4x = std::clamp(f1 * static_cast<float>(invSampleRate_ * 0.25), 1e-6f, 0.48f);

        // Sub-sample linear interpolation of Amp Gate signal across 2x ticks
        float ampGateSig = ampEnvVal;
        switch (ampGateMode) {
            case AmpGateMode::GATE:  ampGateSig = gate_ ? velocity_ : 0.0f; break;
            case AmpGateMode::DRONE: ampGateSig = 1.0f; break;
            case AmpGateMode::ENVELOPE:
            default: ampGateSig = ampEnvVal; break;
        }
        const float effAmpVelocity = (1.0f - ampVelocity) + ampVelocity * velocity_;
        ampGateSig *= effAmpVelocity;

        const float ampGateStep = (ampGateSig - prevAmpGateSig_) * 0.5f;
        const float ampGateTick[2] = { prevAmpGateSig_ + ampGateStep, ampGateSig };
        prevAmpGateSig_ = ampGateSig;

        // Stage 1: Run DCO 2.0 Core at 4x rate (4 ticks per sample)
        float dco4xL[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        float dco4xR[4] = {0.0f, 0.0f, 0.0f, 0.0f};

        for (int k = 0; k < 4; ++k) {
            osc1_.processStereo(f1, dt1_4x, effPwm, dcoMelt, 0, effOsc1ApfCutoff, dco4xL[k], dco4xR[k]);

            dco4xL[k] *= modRes.amRmOsc1;
            dco4xR[k] *= modRes.amRmOsc1;

            if (lifeMode_ == LifeMode::CYBORG) {
                // Asymmetric Diode Seam with ADAA1
                auto applyCyborgSeamADAA = [](float s, float& sPrev) noexcept {
                    const float sAbs = std::abs(s);
                    if (sAbs < 0.05f) {
                        s *= (sAbs * 20.0f); // soften crossover notch
                    }
                    return ADAA::clamp(s, sPrev, -0.72f, 0.32f);
                };
                dco4xL[k] = applyCyborgSeamADAA(dco4xL[k], adaaDiodeL_);
                dco4xR[k] = applyCyborgSeamADAA(dco4xR[k], adaaDiodeR_);
            } else if (lifeMode_ == LifeMode::ORGANIC) {
                dco4xL[k] = dco4xL[k] / (1.0f + 0.15f * std::abs(dco4xL[k]));
                dco4xR[k] = dco4xR[k] / (1.0f + 0.15f * std::abs(dco4xR[k]));
            }
        }

        // Stage 2: Decimate 4x DCO Stereo Stream down to 2x Pre-Filter Bus (>96 dB Rejection)
        float pre2xL[2] = {0.0f, 0.0f};
        float pre2xR[2] = {0.0f, 0.0f};
        decimator4xTo2x_.downsample(
            dco4xL[0], dco4xL[1], dco4xL[2], dco4xL[3],
            dco4xR[0], dco4xR[1], dco4xR[2], dco4xR[3],
            pre2xL[0], pre2xL[1], pre2xR[0], pre2xR[1]
        );

        // Stage 3: Run Dual-SVF VCF, Master LPG, and Post-Amp Saturation at 2x rate (2 ticks)
        const float coreNyquist2x = static_cast<float>(hostSampleRate_ * 0.96);
        float sub2xL[2] = {0.0f, 0.0f};
        float sub2xR[2] = {0.0f, 0.0f};
        float subFiltTapL[2] = {0.0f, 0.0f};
        float subFiltTapR[2] = {0.0f, 0.0f};

        float lastBurbleMod = 0.0f;
        for (int k = 0; k < 2; ++k) {
            // Sub-tick Burble Mod evaluation
            const float burbleMod = osc1_.getBurbleMod();
            lastBurbleMod = burbleMod;
            float burbleMult = 1.0f;
            if (std::abs(burbleDepth) > 0.0001f) {
                const float effBurble = std::copysign(std::pow(std::abs(burbleDepth), 2.2f), burbleDepth);
                switch (burbleMode) {
                    case 0:
                        burbleMult = FastMath::fastPow2(effBurble * 4.0f * burbleMod);
                        break;
                    case 1:
                    {
                        const float kb = (effBurble * 4000.0f * burbleMod) / std::max(20.0f, effCutoffHz);
                        burbleMult = (kb >= 0.0f) ? (1.0f + kb) : (1.0f / (1.0f - kb));
                        break;
                    }
                    default:
                        burbleMult = FastMath::fastPow2(effBurble * 4.0f * burbleMod);
                        break;
                }
            }

            const float subEffectiveCutoffHz = std::clamp(
                effCutoffHz * keyTrackMult * totalModMult * modRes.cutoffMult * burbleMult,
                10.0f,
                coreNyquist2x);

            float subCutoffL = subEffectiveCutoffHz;
            float subCutoffR = subEffectiveCutoffHz;
            if (std::abs(effOrbit) > 0.001f) {
                const float orbitMult = FastMath::fastPow2(std::clamp(effOrbit, -1.0f, 1.0f) * 2.0f);
                const float minCutoff = std::min(120.0f, subEffectiveCutoffHz);
                subCutoffL = std::clamp(subEffectiveCutoffHz / orbitMult, minCutoff, coreNyquist2x);
                subCutoffR = std::clamp(subEffectiveCutoffHz * orbitMult, minCutoff, coreNyquist2x);
            }

            float subFiltL = filter_.process(pre2xL[k], subCutoffL, totalRes, filterDeck, filterWarp, laserBite, laserDrive, hpfMode);
            float subFiltR = filterR_.process(pre2xR[k], subCutoffR, totalRes, filterDeck, filterWarp, laserBite, laserDrive, hpfMode);

            subFiltL *= modRes.postFilterMult;
            subFiltR *= modRes.postFilterMult;

            if (lifeMode_ == LifeMode::CYBORG) {
                subFiltL = cyborgBusFilterL_.process(subFiltL, true);
                subFiltR = cyborgBusFilterR_.process(subFiltR, true);
            }

            subFiltTapL[k] = subFiltL;
            subFiltTapR[k] = subFiltR;

            // Pre-LPG Saturation with ADAA1 at 2x rate (Singing Tail Invariant)
            if (postAmpDrive > 0.001f) {
                const float driveGain = 1.0f + postAmpDrive * 4.0f;
                subFiltL = ADAA::tanh(subFiltL * driveGain, adaaAmpL_, 1.0f);
                subFiltR = ADAA::tanh(subFiltR * driveGain, adaaAmpR_, 1.0f);
            } else {
                adaaAmpL_ = subFiltL;
                adaaAmpR_ = subFiltR;
            }

            // Master LPG Processing at 2x rate
            float lpgOutL = masterLpg_.process(subFiltL, ampGateTick[k]);
            float lpgOutR = masterLpgR_.process(subFiltR, ampGateTick[k]);

            sub2xL[k] = lpgOutL;
            sub2xR[k] = lpgOutR;
        }

        // Stage 4: Downsample 2x Master Stream to Host Rate
        float outSigL = 0.0f, outSigR = 0.0f;
        decimator_.downsample(sub2xL[0], sub2xL[1], sub2xR[0], sub2xR[1], outSigL, outSigR);

        // Cross-Modulation History Buffers
        mixL = 0.25f * (dco4xL[0] + dco4xL[1] + dco4xL[2] + dco4xL[3]);
        mixR = 0.25f * (dco4xR[0] + dco4xR[1] + dco4xR[2] + dco4xR[3]);
        filteredL = 0.5f * (subFiltTapL[0] + subFiltTapL[1]);
        filteredR = 0.5f * (subFiltTapR[0] + subFiltTapR[1]);

        // 13. Master Output: Alpha APF Stereo Dimension & 100% Mono-Sub Invariant (<160Hz)
        const float crossFreq = 160.0f;
        const float crossOmega = static_cast<float>(2.0 * 3.141592653589793 * crossFreq * invSampleRate_);
        const float crossCoeff = crossOmega / (1.0f + crossOmega);

        subCrossoverLpL_ += crossCoeff * (outSigL - subCrossoverLpL_);
        subCrossoverLpR_ += crossCoeff * (outSigR - subCrossoverLpR_);

        const float subL = subCrossoverLpL_;
        const float subR = subCrossoverLpR_;
        const float midL = outSigL - subL;
        const float midR = outSigR - subR;

        // 100% Mono-Sub Foundation
        const float monoSub = 0.5f * (subL + subR);

        // Mid/High Channel Phase Dispersion
        float finalMidL = midL;
        float finalMidR = midR;

        if (alphaChorus > 0.001f) {
            const float chorusRate = (alphaChorus <= 0.5f)
                ? 0.65f
                : 0.65f + (alphaChorus - 0.5f) * 1.2f;

            chorusLfoPhase_ += chorusRate * static_cast<float>(invSampleRate_);
            if (chorusLfoPhase_ >= 1.0f) chorusLfoPhase_ -= 1.0f;

            const float lfoSinL = std::sin(chorusLfoPhase_ * 6.283185307179586f);
            const float phaseOffset = (alphaChorus <= 0.5f) ? 0.25f : (0.25f + (alphaChorus - 0.5f) * 0.5f);
            const float lfoSinR = std::sin((chorusLfoPhase_ + phaseOffset) * 6.283185307179586f);

            const float apfBaseCutoff = 1200.0f;
            const float apfDepth = alphaChorus * 1.0f;
            const float apfCutoffL = std::clamp(apfBaseCutoff * FastMath::fastPow2(lfoSinL * apfDepth), 100.0f, nyquist);
            const float apfCutoffR = std::clamp(apfBaseCutoff * FastMath::fastPow2(lfoSinR * apfDepth), 100.0f, nyquist);

            float apfOutL = apfChorusL_.processAPF(midL, apfCutoffL, 0.2f);
            float apfOutR = apfChorusR_.processAPF(midR, apfCutoffR, 0.2f);

            if (lifeMode_ == LifeMode::CYBORG) {
                bbdSampleCounter_ = !bbdSampleCounter_;
                if (bbdSampleCounter_) {
                    bbdHoldL_ = apfOutL;
                    bbdHoldR_ = apfOutR;
                } else {
                    apfOutL = bbdHoldL_;
                    apfOutR = bbdHoldR_;
                }
            }

            const float wet = std::min(1.0f, alphaChorus * 1.5f);
            const float th = 1.5707963f * std::clamp(-W, 0.0f, 1.0f); // 7:00 side
            const float cT = std::cos(th), sT = std::sin(th);
            const float dS = 0.5f * (apfOutL - apfOutR); // decorrelated side
            const float dM = 0.5f * (apfOutL + apfOutR); // in-phase mid
            const float comb = std::clamp(W, 0.0f, 1.0f); // 5:00 side: comb/flange allowed
            const float midWet = 0.5f * (sT + comb) * dM;
            const float sideWet = cT * dS;
            finalMidL = midL + wet * (midWet + sideWet);
            finalMidR = midR + wet * (midWet - sideWet);
        }

        const float midSum = 0.5f * (finalMidL + finalMidR);
        float sideDiff = 0.5f * (finalMidL - finalMidR);
        sideDiff = masterSideHp_.process(sideDiff);

        if (spreadS <= 0.00001f) {
            outLeft  = (monoSub + midSum) * outputLevel;
            outRight = (monoSub + midSum) * outputLevel;
        } else {
            outLeft  = (monoSub + midSum + sideDiff * spreadS) * outputLevel;
            outRight = (monoSub + midSum - sideDiff * spreadS) * outputLevel;
        }

        // 14. History Buffers
        prevRaw_        = osc1_.getRaw();
        prevCore_       = osc1_.getCore();
        prevSub_        = osc1_.getSub();
        prevOsc1_       = mixL;
        prevOsc2_       = osc1_.getSub();
        prevNoise_      = osc1_.getNoise();
        prevPreFilter_  = 0.5f * (mixL + mixR);
        prevPostFilter_ = 0.5f * (filteredL + filteredR);

        lastFilterEnvVal_.store(filterEnvVal, std::memory_order_relaxed);
        lastGlobalLfoVal_.store(lfoVal, std::memory_order_relaxed);
        lastPwLfoVal_.store(pwLfoVal, std::memory_order_relaxed);
        lastBurbleVal_.store(lastBurbleMod, std::memory_order_relaxed);
        lastModAFmVal_.store(modRes.cutoffMult, std::memory_order_relaxed);
        lastModBAmVal_.store(modRes.postFilterMult, std::memory_order_relaxed);
    }

    void processStereoSample(
        int waveFamily, int chopperTopo, float morph,
        float dcoMult, float dcoWarpOct, float dcoMelt,
        float dcoBite,
        float dcoSwarm,
        int subOctave, int subPulseTrain,
        float coreLevel, float subLevel, float swarmLevel, float noiseLevel, float mixerDrive,
        float globalWidth, float pwLfoRateHz, float pwLfoAmount, float widthEnvDepth,
        float lfoRateHz, LfoWave lfoWave, LfoTriggerMode lfoTrig,
        float glideTimeSec, float oscSlop,
        const ModMatrix::ChannelConfig& modA,
        const ModMatrix::ChannelConfig& modB,
        float cutoffHz, float resonance,
        int filterDeck, float laserBite, float laserDrive,
        float filterOrbit, int hpfMode,
        float envDepth, float filterVelocity, float keyTrack, float filterLfoDepth, float modDepth,
        float filterAttack, float filterSnap, float filterDecay, float filterBloom,
        float filterAftershock, float filterSustain, float filterRelease, int filterLoop,
        float ampSnap, float ampDecay, float ampCurvature, float ampSustain, float ampRelease,
        float pitchPunchDepth, float pitchPunchTime,
        AmpGateMode ampGateMode, float postAmpDrive, float masterSpread, float outputLevel,
        float& outLeft, float& outRight,
        float subApfDepth = 0.0f,
        int subApfTap = 0,
        float burbleDepth = 0.0f,
        int burbleMode = 1,
        float alphaChorus = 0.0f,
        float subSpit = 0.0f,
        float ampVelocity = 0.0f,
        int filterLfoDest = 0,
        float lfoShape = 0.5f,
        int lfoSync = 0) noexcept
    {
        processStereoSample(
            waveFamily, chopperTopo, morph,
            dcoMult, dcoWarpOct, dcoMelt, dcoBite, dcoSwarm,
            subOctave, subPulseTrain,
            coreLevel, subLevel, swarmLevel, noiseLevel, mixerDrive,
            globalWidth, pwLfoRateHz, pwLfoAmount, widthEnvDepth,
            lfoRateHz, lfoWave, lfoTrig, glideTimeSec, oscSlop,
            modA, modB,
            cutoffHz, resonance, filterDeck, 0.0f, laserBite, laserDrive,
            filterOrbit, hpfMode, envDepth, filterVelocity, keyTrack, filterLfoDepth, modDepth,
            filterAttack, filterSnap, filterDecay, filterBloom,
            filterAftershock, filterSustain, filterRelease, filterLoop,
            ampSnap, ampDecay, ampCurvature, ampSustain, ampRelease,
            pitchPunchDepth, pitchPunchTime,
            ampGateMode, postAmpDrive, masterSpread, outputLevel,
            outLeft, outRight,
            subApfDepth, subApfTap, burbleDepth, burbleMode, alphaChorus, subSpit,
            ampVelocity, filterLfoDest, lfoShape, lfoSync
        );
    }

    /**
     * @brief Backwards-compatible legacy overload for existing tests and harnesses.
     */
    void processStereoSample(
        int waveFamily, int chopperTopo, float morph,
        float dcoMult, float dcoWarpOct, float dcoMelt,
        float dcoBite,
        float dcoSwarm,
        int subOctave, int subPulseTrain,
        float coreLevel, float subLevel, float swarmLevel, float noiseLevel, float mixerDrive,
        float globalWidth, float pwLfoRateHz, float pwLfoAmount, float widthEnvDepth,
        float lfoRateHz, LfoWave lfoWave, LfoTriggerMode lfoTrig,
        float glideTimeSec, float oscSlop,
        const ModMatrix::ChannelConfig& modA,
        const ModMatrix::ChannelConfig& modB,
        float cutoffHz, float resonance,
        DualSvfFilter::PoleMode poleMode,
        float midApfCutoff, bool midApfEnabled,
        float midDrive,
        float hpCutoffHz, float envDepth, float keyTrack, float modDepth,
        float filterAttack, float filterSnap, float filterDecay, float filterBloom,
        float filterAftershock, float filterSustain, float filterRelease, int filterLoop,
        float ampSnap, float ampDecay, float ampCurvature, float ampSustain, float ampRelease,
        float pitchPunchDepth, float pitchPunchTime,
        AmpGateMode ampGateMode, float postAmpDrive, float masterSpread, float outputLevel,
        float& outLeft, float& outRight,
        float subApfDepth = 0.0f,
        int subApfTap = 0,
        float burbleDepth = 0.0f,
        int burbleMode = 1,
        float alphaChorus = 0.0f,
        float subSpit = 0.0f,
        float ampVelocity = 0.0f,
        int filterLfoDest = 0,
        float lfoShape = 0.5f,
        int lfoSync = 0) noexcept
    {
        const int hpfMode = (hpCutoffHz > 200.0f) ? 3 : ((hpCutoffHz > 50.0f) ? 2 : 1);
        const float laserBite = midApfEnabled ? 0.5f : 0.0f;
        processStereoSample(
            waveFamily, chopperTopo, morph,
            dcoMult, dcoWarpOct, dcoMelt, dcoBite, dcoSwarm,
            subOctave, subPulseTrain,
            coreLevel, subLevel, swarmLevel, noiseLevel, mixerDrive,
            globalWidth, pwLfoRateHz, pwLfoAmount, widthEnvDepth,
            lfoRateHz, lfoWave, lfoTrig, glideTimeSec, oscSlop,
            modA, modB,
            cutoffHz, resonance, static_cast<int>(poleMode), laserBite, midDrive,
            0.0f, hpfMode, envDepth, 0.5f, keyTrack, modDepth, modDepth,
            filterAttack, filterSnap, filterDecay, filterBloom,
            filterAftershock, filterSustain, filterRelease, filterLoop,
            ampSnap, ampDecay, ampCurvature, ampSustain, ampRelease,
            pitchPunchDepth, pitchPunchTime,
            ampGateMode, postAmpDrive, masterSpread, outputLevel,
            outLeft, outRight,
            subApfDepth, subApfTap, burbleDepth, burbleMode, alphaChorus, subSpit,
            ampVelocity, filterLfoDest, lfoShape, lfoSync
        );
    }


    [[nodiscard]] float processSample(
        // Card 1: Osc 1 (Pitch Offset, Mult Self-Sync & Pitch-Tracking APF)
        int osc1Wave, float osc1PitchOffsetSemi, float osc1Mult, float osc1ApfPitchTrackOct, float osc1Drive,
        // In-between Card: Metalizer (Audio-Rate Post-APF Metal Sync)
        float metalizerAmount,
        // Card 2: Osc 2 (Pitch Offset, Mult Self-Sync & Pitch-Tracking APF)
        int osc2Wave, float osc2PitchOffsetSemi, float osc2Mult, float osc2ApfPitchTrackOct, float osc2Drive,
        // Drive Topology Permutation
        int driveTopology,
        // Card 3: Dedicated 4-Lane Full-Height Mixer Block
        float osc1Level,
        float osc2Level,
        float noiseLevel,
        float mixerDrive,
        // Card 5: Dedicated Global Modulation Block
        float globalWidth, float pwLfoRateHz, float pwLfoAmount, float widthEnvDepth,
        float lfoRateHz, LfoWave lfoWave, LfoTriggerMode lfoTrig,
        float glideTimeSec, float oscSlop,
        // Card 6: Universal Mod A & Mod B Matrix
        const ModMatrix::ChannelConfig& modA,
        const ModMatrix::ChannelConfig& modB,
        // Card 7: Extended Dual-SVF VCF Block
        float cutoffHz, float resonance,
        DualSvfFilter::PoleMode poleMode,
        float midApfCutoff, bool midApfEnabled,
        float midDrive,
        float hpCutoffHz, float envDepth, float keyTrack, float modDepth,
        // Card 8: Cody's Bass Envelope Controls
        float totalTimeSec, float slope, float sustainLevel, float releaseTimeSec,
        // Card 4: Dedicated Amp Block
        AmpGateMode ampGateMode, float postAmpDrive, float outputLevel) noexcept
    {
        const float nyquist = static_cast<float>(sampleRate_ * 0.48);

        // 1. Organic Vactrol CV Slew Smoothing (Organic Life Mode only)
        const bool isOrganic = (lifeMode_ == LifeMode::ORGANIC);
        const float effCutoffHz   = cutoffCv_.process(cutoffHz, isOrganic);
        const float effRes        = resonanceCv_.process(resonance, isOrganic);
        const float effWidth      = widthCv_.process(globalWidth, isOrganic);
        const float effOsc1Offset = osc1OffsetCv_.process(osc1PitchOffsetSemi, isOrganic);
        const float effOsc2Offset = osc2OffsetCv_.process(osc2PitchOffsetSemi, isOrganic);
        const float effMixerDrive = mixerDriveCv_.process(mixerDrive, isOrganic);

        // 2. Evaluate Cody's Bass Envelope
        float basePitchHz = 130.81f;
        float biteDummy = 0.0f;
        const float envVal = ampEnv_.process(0.0015f, totalTimeSec, slope, sustainLevel, releaseTimeSec, 0.0f, 0.020f, 0.5f, biteDummy);

        // 3. Compute Analog Slop Wander
        if (oscSlop > 0.001f) {
            slopPrng_ = slopPrng_ * 1664525u + 1013904223u;
            const float white1 = static_cast<float>(static_cast<int32_t>(slopPrng_)) * 4.6566129e-10f;
            slopPrng_ = slopPrng_ * 1664525u + 1013904223u;
            const float white2 = static_cast<float>(static_cast<int32_t>(slopPrng_)) * 4.6566129e-10f;

            slopWander1_ += (white1 - slopWander1_) * 0.0003f;
            slopWander2_ += (white2 - slopWander2_) * 0.0003f;
        } else {
            slopWander1_ = 0.0f;
            slopWander2_ = 0.0f;
        }

        // 4. Compute Base Oscillator Frequencies
        const float slopSemitones1 = slopWander1_ * (oscSlop * 0.35f);
        const float slopSemitones2 = slopWander2_ * (oscSlop * 0.35f);

        float f1 = std::clamp(basePitchHz * std::pow(2.0f, (effOsc1Offset + slopSemitones1) * (1.0f / 12.0f)), 0.0001f, nyquist);
        float f2 = std::clamp(basePitchHz * std::pow(2.0f, (effOsc2Offset + slopSemitones2) * (1.0f / 12.0f)), 0.0001f, nyquist);

        // 5. Evaluate Global LFO
        float lfoVal = 0.0f;
        if (lfoRunning_) {
            const float lfoDt = std::clamp(lfoRateHz, 0.01f, 50.0f) * static_cast<float>(invSampleRate_);
            lfoPhase_ += lfoDt;

            if (lfoTrig == LfoTriggerMode::ONE_SHOT && lfoPhase_ >= 1.0f) {
                lfoRunning_ = false;
                lfoPhase_ = 1.0f;
            } else if (lfoTrig == LfoTriggerMode::HALF_SHOT && lfoPhase_ >= 0.5f) {
                lfoRunning_ = false;
                lfoPhase_ = 0.5f;
            } else if (lfoPhase_ >= 1.0f) {
                lfoPhase_ -= std::floor(lfoPhase_);
                slopPrng_ = slopPrng_ * 1664525u + 1013904223u;
                lfoRandomVal_ = static_cast<float>(static_cast<int32_t>(slopPrng_)) * 4.6566129e-10f;
            }

            lfoVal = shapeLfo(lfoWave, lfoPhase_, 0.5f, lfoRandomVal_, lfoRandomPrev_);
        }

        // 6. Evaluate Dedicated Sine PWM LFO
        const float pwLfoDt = std::clamp(pwLfoRateHz, 0.01f, 50.0f) * static_cast<float>(invSampleRate_);
        pwLfoPhase_ += pwLfoDt;
        if (pwLfoPhase_ >= 1.0f) pwLfoPhase_ -= std::floor(pwLfoPhase_);
        const float pwLfoVal = std::sin(pwLfoPhase_ * 6.28318530717958647692f);

        // 7. Compute Modulated Pulse Width
        const float modPw = std::clamp(
            effWidth + pwLfoVal * (pwLfoAmount * 0.5f) + envVal * (widthEnvDepth * 0.5f),
            0.01f, 0.99f);
        const float effPwm = (modPw - 0.5f) * 2.0f; // [-0.98, +0.98]

        // 8. Evaluate Mod Matrix from Previous Sample (Mod A & Mod B)
        ModMatrix::Sources modSrc;
        modSrc.osc1        = prevOsc1_;
        modSrc.osc2        = prevOsc2_;
        modSrc.noise       = prevNoise_;
        modSrc.preFilter   = prevPreFilter_;
        modSrc.postFilter  = prevPostFilter_;

        const auto modRes = ModMatrix::compute(modA, modB, modSrc);

        // Apply Frequency Modulations (Through-Zero linear FM permitted)
        f1 = std::clamp((f1 + modRes.linFmOsc1Hz) * modRes.expFmOsc1Mult, 1.0f, static_cast<float>(sampleRate_ * 1.92f));
        f2 = (f2 + modRes.linFmOsc2Hz) * modRes.expFmOsc2Mult;

        // 9. Pitch-Following APF Cutoffs (-1..0..+2 Octave Tracking with abs for TZFM)
        const float effOsc1ApfCutoff = std::clamp(std::abs(f1) * std::pow(2.0f, osc1ApfPitchTrackOct), 10.0f, nyquist);
        const float effOsc2ApfCutoff = std::clamp(std::abs(f2) * std::pow(2.0f, osc2ApfPitchTrackOct), 10.0f, nyquist);

        // 10. Metalizer: Mutual Audio-Rate Metal Sync (1 -> 2 AND 2 -> 1)
        // Listens to post-MULT + post-APF/Tanh saturated output both ways!
        // Linear mapping to Bresenham positive-edge counter across full 0.0 -> 1.0 range
        const float effMetalizer = std::clamp(metalizerAmount, 0.0f, 1.0f);
        const bool syncOsc1 = prevOsc2Trigger_ && (effMetalizer > 0.0001f);
        const bool syncOsc2 = prevOsc1Trigger_ && (effMetalizer > 0.0001f);

        // 11. Render Osc 1 with MULT Self-Hard-Sync & Metalizer Sync from Osc 2 on 12 MHz clock
        const float osc1Out = osc1_.process(f1, osc1Wave, effPwm, effOsc1ApfCutoff, osc1Drive, osc1Mult, driveTopology, syncOsc1, effMetalizer, prevOsc2SyncFrac_);

        // 12. Render Osc 2 with MULT Self-Hard-Sync & Metalizer Sync from Osc 1 on 12 MHz clock
        const float osc2Out = osc2_.process(f2, osc2Wave, effPwm, effOsc2ApfCutoff, osc2Drive, osc2Mult, driveTopology, syncOsc2, effMetalizer, prevOsc1SyncFrac_);

        // Update rising zero-crossing comparator states & sub-sample timestamps on full osc block outputs
        const bool osc1Fired = (prevOsc1_ <= 0.0f && osc1Out > 0.0f);
        const bool osc2Fired = (prevOsc2_ <= 0.0f && osc2Out > 0.0f);
        if (osc1Fired) {
            prevOsc1SyncFrac_ = std::clamp(-prevOsc1_ / (osc1Out - prevOsc1_ + 1e-9f), 0.0f, 1.0f);
        }
        if (osc2Fired) {
            prevOsc2SyncFrac_ = std::clamp(-prevOsc2_ / (osc2Out - prevOsc2_ + 1e-9f), 0.0f, 1.0f);
        }
        prevOsc1Trigger_ = osc1Fired;
        prevOsc2Trigger_ = osc2Fired;

        // Unified full osc block outputs for mixer (Osc -> Mult -> [Disperse-Fold])
        float osc1Tap = osc1Out;
        float osc2Tap = osc2Out;

        // Apply AM/RM Mod Matrix Scaling
        osc1Tap *= modRes.amRmOsc1;
        osc2Tap *= modRes.amRmOsc2;

        // 13. Evaluate Pure White Noise
        const float noiseRaw = noiseGen_.process();

        // 14. Pass through Meng Qi DPLPG Cells for 4-Lane Dedicated Mixer
        const float osc1Gated = osc1Lpg_.process(osc1Tap, osc1Level);
        const float osc2Gated = osc2Lpg_.process(osc2Tap, osc2Level);
        const float noiseGated = noiseLpg_.process(noiseRaw, noiseLevel);

        // 15. Sum Dedicated Mixer Bus with Mixer Tanh Saturation
        float preFilterMix = osc1Gated + osc2Gated + noiseGated;
        if (effMixerDrive > 0.001f) {
            preFilterMix = std::tanh(preFilterMix * (1.0f + effMixerDrive * 4.0f));
        }

        // 15. Filter Frequency Modulation & Keytracking
        const float keyTrackSemitones = (static_cast<float>(currentNote_) - 60.0f) * keyTrack;
        const float keyTrackMult = std::pow(2.0f, keyTrackSemitones * (1.0f / 12.0f));

        const float envModSemitones = envVal * envDepth * 48.0f;
        const float lfoModSemitones = lfoVal * modDepth * 24.0f;
        const float totalModMult = std::pow(2.0f, (envModSemitones + lfoModSemitones) * (1.0f / 12.0f));

        const float effectiveCutoffHz = std::clamp(
            effCutoffHz * keyTrackMult * totalModMult * modRes.cutoffMult,
            10.0f,
            22000.0f);

        // 16. Extended Dual-SVF VCF with Mid-Filter APF and 4 Pole Modes
        const float effMidApfCutoff = std::clamp(midApfCutoff, 20.0f, nyquist);

        float filtered = filter_.process(
            preFilterMix,
            effectiveCutoffHz,
            effRes,
            hpCutoffHz,
            poleMode,
            effMidApfCutoff,
            midApfEnabled,
            midDrive);

        // Apply Post-Filter Mod Matrix
        filtered *= modRes.postFilterMult;

        // 17. Dedicated Amp Block (Envelope, Gate, Drone) + Post-Amp Saturation
        float ampGateSig = envVal;
        switch (ampGateMode) {
            case AmpGateMode::GATE:
                ampGateSig = gate_ ? velocity_ : 0.0f;
                break;
            case AmpGateMode::DRONE:
                ampGateSig = 1.0f;
                break;
            case AmpGateMode::ENVELOPE:
            default:
                ampGateSig = envVal;
                break;
        }

        if (postAmpDrive > 0.001f) {
            filtered = std::tanh(filtered * (1.0f + postAmpDrive * 4.0f));
        }

        float outSignal = masterLpg_.process(filtered, ampGateSig);

        // 18. Store Cross-Modulation History Buffers
        prevOsc1_      = osc1Out;
        prevOsc2_      = osc2Out;
        prevNoise_     = noiseRaw;
        prevPreFilter_ = preFilterMix;
        prevPostFilter_= filtered;

        return outSignal * outputLevel;
    }

    [[nodiscard]] bool isActive() const noexcept {
        return gate_ || !vortexEnv_.isIdle() || !ampEnv_.isIdle() || masterLpg_.isRinging();
    }

    [[nodiscard]] bool isPllLocked() const noexcept {
        return false;
    }

private:
    double sampleRate_ = 44100.0;
    double invSampleRate_ = 1.0 / 44100.0;
    double hostSampleRate_ = 44100.0;

    StereoHalfBandDecimator4xTo2x decimator4xTo2x_;
    StereoHalfBandDecimator2x decimator_;
    CyborgAnalogBusFilter cyborgBusFilterL_;
    CyborgAnalogBusFilter cyborgBusFilterR_;

    // ADAA1 History States & Sub-Sample Slew
    float adaaAmpL_{0.0f};
    float adaaAmpR_{0.0f};
    float adaaDiodeL_{0.0f};
    float adaaDiodeR_{0.0f};
    float prevAmpGateSig_{0.0f};

    VactrolLPG::BackendMode backendMode_ = VactrolLPG::BackendMode::LPG;
    LifeMode lifeMode_{LifeMode::CYBORG};

    // Fleet of 6 Meng Qi DPLPG Cells (5-lane mixer + master amp)
    VactrolLPG osc1Lpg_;
    VactrolLPG osc2Lpg_;
    VactrolLPG noiseLpg_;
    VactrolLPG masterLpg_;

    // Virtual Vactrol CV Slewers (Organic Life Mode)
    VactrolCV cutoffCv_;
    VactrolCV resonanceCv_;
    VactrolCV widthCv_;
    VactrolCV osc1OffsetCv_;
    VactrolCV osc2OffsetCv_;
    VactrolCV slewCv_;
    VactrolCV mixerDriveCv_;

    // Core Synthesizer Subsystems
    AlphaOscillator osc1_;
    AlphaOscillator osc2_;
    SampleHoldNoise noiseGen_;
    DualSvfFilter filter_;
    DualSvfFilter filterR_;
    VactrolLPG masterLpgR_;
    OnePoleHP masterSideHp_;
    VortexEnvelope vortexEnv_;
    CodyBassEnvelope ampEnv_;
    // SOTA Note Stack & Priority
    struct NoteEntry {
        int noteNumber = -1;
        float velocity = 0.0f;
    };
    std::array<NoteEntry, 16> noteStack_{};
    int noteStackSize_ = 0;

    // Nonlinear Tuned Resonant Feedback
    static constexpr size_t kFbDelaySize = 4096;
    std::array<float, kFbDelaySize> fbDelayBuffer_{};
    size_t fbDelayWriteIdx_ = 0;
    float fbFilterState_ = 0.0f;
    float fbDcStateX_ = 0.0f;
    float fbDcStateY_ = 0.0f;

    // Alpha APF Stereo Dimension
    AllPassFilter4 apfChorusL_;
    AllPassFilter4 apfChorusR_;
    float chorusLfoPhase_ = 0.0f;
    bool  bbdSampleCounter_ = false;
    float bbdHoldL_ = 0.0f;
    float bbdHoldR_ = 0.0f;
    float subCrossoverLpL_ = 0.0f;
    float subCrossoverLpR_ = 0.0f;
    float currentPitchHz_{130.81f};
    float targetPitchHz_{130.81f};

    // Cross-Modulation History Buffers
    float prevRaw_ = 0.0f;
    float prevCore_ = 0.0f;
    float prevSub_ = 0.0f;
    float prevOsc1_ = 0.0f;
    float prevOsc2_ = 0.0f;
    bool  prevOsc1Trigger_ = false;
    bool  prevOsc2Trigger_ = false;
    float prevOsc1SyncFrac_ = 0.0f;
    float prevOsc2SyncFrac_ = 0.0f;
    float prevNoise_ = 0.0f;
    float prevPreFilter_ = 0.0f;
    float prevPostFilter_ = 0.0f;

    // Global Modulation & Slop State
    float lfoPhase_ = 0.0f;
    float lfoRandomVal_ = 0.0f;
    float lfoRandomPrev_ = 0.0f;
    uint32_t lfoPrng_ = 0xC0DEC0DEu;
    bool  lfoRunning_ = true;
    float pwLfoPhase_ = 0.0f;
    float effMasterSpread_{0.0f};

    float curSemi_{60.0f};
    float tgtSemi_{60.0f};

    float slopWander1_ = 0.0f;
    float slopWander2_ = 0.0f;
    float slopLeak_{0.99992f};
    float slopInnov_{0.0025f};
    uint32_t slopPrng_ = 0x5EED1234u;
    uint64_t baseSeed_{0x85432107C0DEC0DEull};

    float bendTargetSemi_{0.0f};
    float bendSemi_{0.0f};
    float bendCoeff_{0.005f};
    float vibPhase_{0.0f};
    float vibRateHz_{5.5f};
    float vibDepth_{0.0f};

    int activeNoteCount_ = 0;
    int currentNote_ = 60;
    float velocity_ = 1.0f;
    bool gate_ = false;

    // Stepped Random Modulation State (9 Target Channels)
    uint64_t currentSampleCount_{0};
    uint64_t lastNoteOffSample_{0};
    float lastPitchHz_{130.81f};

    uint32_t stepRandSeed_{0x85432107u};
    std::array<float, 9> stepRand_{};
    std::array<std::atomic<float>, 9> stepRandAtom_{};
    std::atomic<float> lastFilterEnvVal_{0.0f};
    std::atomic<float> lastGlobalLfoVal_{0.0f};
    std::atomic<float> lastPwLfoVal_{0.0f};
    std::atomic<float> lastBurbleVal_{0.0f};
    std::atomic<float> lastModAFmVal_{1.0f};
    std::atomic<float> lastModBAmVal_{1.0f};

public:
    void seed(uint64_t s) noexcept {
        baseSeed_ = s;
        auto mix = [](uint64_t k) {
            k += 0x9E3779B97F4A7C15ull;
            k = (k ^ (k >> 30)) * 0xBF58476D1CE4E5B9ull;
            k = (k ^ (k >> 27)) * 0x94D049BB133111EBull;
            return static_cast<uint32_t>(k ^ (k >> 31)) | 1u;
        };
        stepRandSeed_ = mix(s);
        lfoPrng_ = mix(s + 1);
        slopPrng_ = mix(s + 2);
    }

    void releaseAll() noexcept {
        noteStackSize_ = 0;
        activeNoteCount_ = 0;
        gate_ = false;
        ampEnv_.noteOff();
        vortexEnv_.noteOff();
    }

    [[nodiscard]] float random01() noexcept {
        stepRandSeed_ ^= (stepRandSeed_ << 13);
        stepRandSeed_ ^= (stepRandSeed_ >> 17);
        stepRandSeed_ ^= (stepRandSeed_ << 5);
        return static_cast<float>(stepRandSeed_ & 0x00FFFFFF) * (1.0f / 16777216.0f);
    }

    [[nodiscard]] float getStepRand(int i) const noexcept {
        return (i >= 0 && i < 9) ? stepRand_[i] : 0.0f;
    }

    [[nodiscard]] float getStepRandAtomic(int i) const noexcept {
        return (i >= 0 && i < 9) ? stepRandAtom_[i].load(std::memory_order_relaxed) : 0.0f;
    }

    [[nodiscard]] float getFilterEnvAtomic() const noexcept { return lastFilterEnvVal_.load(std::memory_order_relaxed); }
    [[nodiscard]] float getGlobalLfoAtomic() const noexcept { return lastGlobalLfoVal_.load(std::memory_order_relaxed); }
    [[nodiscard]] float getPwLfoAtomic() const noexcept { return lastPwLfoVal_.load(std::memory_order_relaxed); }
    [[nodiscard]] float getBurbleAtomic() const noexcept { return lastBurbleVal_.load(std::memory_order_relaxed); }
    [[nodiscard]] float getModAFmAtomic() const noexcept { return lastModAFmVal_.load(std::memory_order_relaxed); }
    [[nodiscard]] float getModBAmAtomic() const noexcept { return lastModBAmVal_.load(std::memory_order_relaxed); }
};

} // namespace CrackedChimes::DSP
