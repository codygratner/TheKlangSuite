#pragma once

#include <cmath>
#include <juce_audio_basics/juce_audio_basics.h>
#include "NyarlatOscillator.h"
#include "ZDomainFilter14.h"
#include "NyarlatChaosMatrix.h"
#include "../hastur/HasturFilterUtils.h"
#include "../FastMath.h"

namespace Nyarlat {

class NyarlatVoice {
public:
    enum class EnvStage { Idle, Attack, Decay, Sustain, Release };

    struct Envelope {
        EnvStage stage = EnvStage::Idle;
        float level = 0.0f;
        float rate = 0.0f;
        float attackRate = 0.01f;
        float decayRate = 0.005f;
        float sustainLevel = 0.7f;
        float releaseRate = 0.005f;

        void setADSR(float aSec, float dSec, float sNorm, float rSec, double sampleRate, float rateScale = 1.0f) noexcept {
            sustainLevel = juce::jlimit(0.0f, 1.0f, sNorm);
            double sr = sampleRate > 0.0 ? sampleRate : 44100.0;
            // Scale attack/decay by rateScale for acoustic clavinet keytrack
            double scaledA = std::max(0.0005, static_cast<double>(aSec) * rateScale);
            double scaledD = std::max(0.005, static_cast<double>(dSec) * rateScale);
            double scaledR = std::max(0.005, static_cast<double>(rSec));

            attackRate = static_cast<float>(1.0 / (scaledA * sr));
            decayRate = static_cast<float>(1.0 / (scaledD * sr));
            // Exact one-pole coefficient: level falls 1.0 -> 1e-4 (-80 dB, the idle cutoff) in scaledR seconds
            releaseRate = static_cast<float>(-std::expm1(-9.210340371976184 / (scaledR * sr)));
        }

        void noteOn() noexcept {
            stage = EnvStage::Attack;
        }

        void noteOff() noexcept {
            if (stage != EnvStage::Idle)
                stage = EnvStage::Release;
        }

        inline float process() noexcept {
            switch (stage) {
                case EnvStage::Attack:
                    level += attackRate;
                    if (level >= 1.0f) {
                        level = 1.0f;
                        stage = EnvStage::Decay;
                    }
                    break;
                case EnvStage::Decay:
                    level -= (level - sustainLevel) * decayRate;
                    if (level <= sustainLevel + 0.001f) {
                        level = sustainLevel;
                        stage = EnvStage::Sustain;
                    }
                    break;
                case EnvStage::Sustain:
                    level = sustainLevel;
                    break;
                case EnvStage::Release:
                    level -= level * releaseRate;
                    if (level <= 0.0001f) {
                        level = 0.0f;
                        stage = EnvStage::Idle;
                    }
                    break;
                case EnvStage::Idle:
                default:
                    level = 0.0f;
                    break;
            }
            return level;
        }

        bool isActive() const noexcept { return stage != EnvStage::Idle; }

        bool shouldCull(int activeCount) const noexcept {
            if (stage != EnvStage::Release) return false;
            if (activeCount > 12) return level < 0.0010f; // -60 dB
            if (activeCount > 8)  return level < 0.0005f; // -66 dB
            return level <= 0.0001f;                      // -80 dB
        }
    };

    NyarlatVoice(int voiceIndex = 0) : voiceIndex_(voiceIndex) {
        chaos_.setVoiceSeed(voiceIndex, 0.5f);
    }

    void setVoiceIndex(int voiceIndex) noexcept {
        voiceIndex_ = voiceIndex;
        chaos_.setVoiceSeed(voiceIndex, 0.5f);
    }

    void prepare(double sampleRate) noexcept {
        sampleRate_ = sampleRate;
        for (auto& osc : oscs_) osc.prepare(sampleRate);
        filter_.prepare(sampleRate);
        mixerDcBlockerL_.prepare(sampleRate, 5.0);
        mixerDcBlockerR_.prepare(sampleRate, 5.0);
        postFilterDcBlockerL_.prepare(sampleRate, 5.0);
        postFilterDcBlockerR_.prepare(sampleRate, 5.0);
        reset();
    }

    void reset() noexcept {
        for (auto& osc : oscs_) osc.reset();
        filter_.reset();
        mixerDcBlockerL_.reset();
        mixerDcBlockerR_.reset();
        postFilterDcBlockerL_.reset();
        postFilterDcBlockerR_.reset();
        ampEnv_.stage = EnvStage::Idle;
        ampEnv_.level = 0.0f;
        filterEnv_.stage = EnvStage::Idle;
        filterEnv_.level = 0.0f;
        currentPitchHz_ = 440.0f;
        targetPitchHz_ = 440.0f;
        mpePitchBendSemitones_ = 0.0f;
        mpeSlide_ = 0.0f;
        mpePressure_ = 0.0f;
        noiseHoldCounter_ = 0;
        noiseSample_ = 0.0f;
        ctrlCounter_ = 0;
        cachedOctMult_[0] = 1.0f; cachedOctMult_[1] = 1.0f; cachedOctMult_[2] = 1.0f;
        cachedBendMult_ = 1.0f;
        effWave_[0] = 0.0f; effWave_[1] = 0.0f; effWave_[2] = 0.0f;
        cachedEffSpeed_[0] = 0.0f; cachedEffSpeed_[1] = 0.0f; cachedEffSpeed_[2] = 0.0f;
        cachedEffMask_[0] = 511; cachedEffMask_[1] = 511; cachedEffMask_[2] = 511;
        lastSemis_[0] = lastSemis_[1] = lastSemis_[2] = 1e9f;
        lastTotalBend_ = 1e9f;
    }

    void sanitize() noexcept {
        filter_.sanitize();
    }

    void noteOn(int midiNote, float velocity, float glideTimeSec) noexcept {
        midiNote_ = midiNote;
        velocity_ = velocity;
        targetPitchHz_ = 440.0f * std::pow(2.0f, (midiNote - 69.0f) / 12.0f);

        if (glideTimeSec <= 0.001f || !ampEnv_.isActive()) {
            currentPitchHz_ = targetPitchHz_;
        }
        glideSpeed_ = (glideTimeSec > 0.001f)
            ? static_cast<float>(1.0 / (glideTimeSec * sampleRate_))
            : 1.0f;

        chaos_.triggerNoteOn();
        ampEnv_.noteOn();
        filterEnv_.noteOn();
        ctrlCounter_ = 0;
    }

    void noteOff() noexcept {
        ampEnv_.noteOff();
        filterEnv_.noteOff();
    }

    bool isActive() const noexcept {
        return ampEnv_.isActive();
    }

    int getMidiNote() const noexcept { return midiNote_; }
    uint32_t getStatorOffset(int osc) const noexcept {
        return (osc >= 0 && osc < 3) ? oscs_[osc].getStatorOffset() : 0;
    }
    float getEffWave(int osc) const noexcept {
        return (osc >= 0 && osc < 3) ? effWave_[osc] : 0.0f;
    }
    int getEffMask(int osc) const noexcept {
        return (osc >= 0 && osc < 3) ? cachedEffMask_[osc] : 511;
    }
    float getEffSpeed(int osc) const noexcept {
        return (osc >= 0 && osc < 3) ? cachedEffSpeed_[osc] : 0.0f;
    }

    // Update MPE per-note controllers
    void setMpePitchBend(float semitones) noexcept { mpePitchBendSemitones_ = semitones; }
    void setMpeSlide(float slide) noexcept { mpeSlide_ = slide; }
    void setMpePressure(float pressure) noexcept { mpePressure_ = pressure; }

    // Block-level voice rendering accumulating directly into output buffers (L1 cache pinned)
    void renderBlockAdd(float* outL, float* outR, int numSamples, int activeVoiceCount,
                        // Global parameter snapshots
                        const float* oscBankIdx, const float* oscWaveMorph,
                        const int* oscMask, const float* oscDetune, const int* oscOctave,
                        const bool* oscOctLock, const bool* oscSync,
                        const float* oscSpeed, const int* oscLoop,
                        float osc1Level, float osc2Level, float osc3Level,
                        float noiseLevel, float noiseTone, float oscSpread, float mixerSeethe,
                        // Chaos & Random Attenuverters
                        float chaosRate, float chaosDrift, float chaosPolySpread,
                        const float* randAttenuverters,
                        // Z-Filter
                        int zModel, float zFreq, float zMorph, float zTransform,
                        float filterEnvDepth, float filterVelocity,
                        // Voicing & Dynamics
                        float ampVelocity, float globalPitchBendSemitones,
                        float globalModWheel, float globalChannelAftertouch) noexcept {

        juce::ignoreUnused(oscBankIdx, oscWaveMorph, chaosRate, chaosDrift, chaosPolySpread, filterVelocity);

        if (!ampEnv_.isActive()) return;

        const float velScale = 1.0f - ampVelocity + (ampVelocity * velocity_ * velocity_);
        const float panSpread = oscSpread * 0.5f;
        const int noiseHoldMax = 1 + static_cast<int>(noiseTone * 126.0f);
        const float driveGain = 1.0f + mixerSeethe * 4.0f;

        for (int s = 0; s < numSamples; ++s) {
            // Adaptive polyphonic culling: drop quiet release tails under high voice density
            if (ampEnv_.shouldCull(activeVoiceCount)) {
                ampEnv_.stage = EnvStage::Idle;
                ampEnv_.level = 0.0f;
                return;
            }

            // 1. Advance Glide toward target pitch
            if (currentPitchHz_ != targetPitchHz_) {
                currentPitchHz_ += (targetPitchHz_ - currentPitchHz_) * glideSpeed_;
            }

            // 2. Advance Envelopes
            float ampGain = ampEnv_.process();
            float filtEnvVal = filterEnv_.process();

            // Control-rate decimation (every 64 samples, ~750 Hz at 48k)
            if ((ctrlCounter_++ & 63) == 0) {
                // Total pitch with Pitch Bend (MPE + Global)
                float totalBend = mpePitchBendSemitones_ + globalPitchBendSemitones;
                if (totalBend != lastTotalBend_) {
                    lastTotalBend_ = totalBend;
                    cachedBendMult_ = TbdAudio::FastMath::fastPow2(totalBend / 12.0f);
                }

                // Process 3 Oscillator Pitch Ratios (detune scaled from cents to semitones: 100 ct = 1 st)
                for (int i = 0; i < 3; ++i) {
                    float semis = (oscOctave[i] - 2) * 12.0f + (oscDetune[i] * 0.01f);
                    if (semis != lastSemis_[i]) {
                        lastSemis_[i] = semis;
                        cachedOctMult_[i] = TbdAudio::FastMath::fastPow2(semis / 12.0f);
                    }
                }

                // Modulate stator speed and mask aperture with chaos attenuverters
                for (int i = 0; i < 3; ++i) {
                    cachedEffMask_[i] = juce::jlimit(0, 511,
                        oscMask[i] + static_cast<int>(chaos_.getModulation(i * 3 + 1, randAttenuverters[i * 3 + 1]) * 255.0f));
                    cachedEffSpeed_[i] = juce::jlimit(0.0f, 10.0f,
                        oscSpeed[i] + chaos_.getModulation(i * 3 + 2, randAttenuverters[i * 3 + 2]) * 2.5f);
                }

                // Z-Plane Filter Expression & Target Coefficients Update (linear ramp across 64 steps)
                float effMorph = juce::jlimit(0.0f, 1.0f, zMorph + mpeSlide_ + globalModWheel);
                float effTransform = juce::jlimit(0.0f, 1.0f, zTransform + std::max(mpePressure_, globalChannelAftertouch));
                float effFreq = juce::jlimit(20.0f, 20000.0f, zFreq + (filtEnvVal * filterEnvDepth * 8000.0f));
                float xNorm = TbdAudio::FastMath::fastLog2(effFreq / 20.0f) * 0.10034333f;

                filter_.updateCube(zModel, xNorm, effMorph, effTransform, 1.0f / 64.0f);
            }

            float basePitch = currentPitchHz_ * cachedBendMult_;

            // 3. Process 3 Oscillators with Hard Sync & Q32 Stator Sweep
            const uint32_t prevP1 = oscs_[0].getPhase() & 0x00FFFFFFu;
            float sig1 = oscs_[0].processSample(basePitch * cachedOctMult_[0], cachedEffMask_[0], cachedEffSpeed_[0], oscLoop[0], oscOctLock[0]);
            bool syncTrigger = ((oscs_[0].getPhase() & 0x00FFFFFFu) < prevP1);

            float sig2 = oscs_[1].processSample(basePitch * cachedOctMult_[1], cachedEffMask_[1], cachedEffSpeed_[1], oscLoop[1], oscOctLock[1], oscSync[1] && syncTrigger);
            float sig3 = oscs_[2].processSample(basePitch * cachedOctMult_[2], cachedEffMask_[2], cachedEffSpeed_[2], oscLoop[2], oscOctLock[2], oscSync[2] && syncTrigger);

            // 4. Variable-rate Sample-and-Hold Noise
            if (++noiseHoldCounter_ >= noiseHoldMax) {
                noiseHoldCounter_ = 0;
                noiseSeed_ = 214013 * noiseSeed_ + 2531011;
                noiseSample_ = static_cast<float>((noiseSeed_ >> 16) & 0x7FFF) / 16384.0f - 1.0f;
            }

            // 5. Pre-Filter Stereo Stem Mixer with Osc Spread
            float mixL = (sig1 * osc1Level * 0.707f + sig2 * osc2Level * (0.707f + panSpread) + sig3 * osc3Level * (0.707f - panSpread) + noiseSample_ * noiseLevel) * 0.333f;
            float mixR = (sig1 * osc1Level * 0.707f + sig2 * osc2Level * (0.707f - panSpread) + sig3 * osc3Level * (0.707f + panSpread) + noiseSample_ * noiseLevel) * 0.333f;

            // 6. Per-Voice Console Seethe Drive
            if (mixerSeethe > 0.001f) {
                mixL = TbdAudio::FastMath::fastTanh(mixL * driveGain);
                mixR = TbdAudio::FastMath::fastTanh(mixR * driveGain);
            }

            // Pre-Filter DC Blocker
            mixL = mixerDcBlockerL_.process(mixL);
            mixR = mixerDcBlockerR_.process(mixR);

            // 7. Process Z-Plane Filter (SIMD 2-wide with stepRamp and softLimit2Wide)
            filter_.processStereo(mixL, mixR);

            // Post-Filter DC Blocker (nullifies any resonant/transient DC offset)
            mixL = postFilterDcBlockerL_.process(mixL);
            mixR = postFilterDcBlockerR_.process(mixR);

            // 8. VCA Stage with Velocity Scaling
            float totalGain = ampGain * velScale;
            outL[s] += mixL * totalGain;
            outR[s] += mixR * totalGain;
        }
    }

    // Scalar fallback for single-sample processing
    void processSample(float& outL, float& outR,
                       // Global parameter snapshots
                       const float* oscBankIdx, const float* oscWaveMorph,
                       const int* oscMask, const float* oscDetune, const int* oscOctave,
                       const bool* oscOctLock, const bool* oscSync,
                       const float* oscSpeed, const int* oscLoop,
                       float osc1Level, float osc2Level, float osc3Level,
                       float noiseLevel, float noiseTone, float oscSpread, float mixerSeethe,
                       // Chaos & Random Attenuverters
                       float chaosRate, float chaosDrift, float chaosPolySpread,
                       const float* randAttenuverters,
                       // Z-Filter
                       int zModel, float zFreq, float zMorph, float zTransform,
                       float filterEnvDepth, float filterVelocity,
                       // Voicing & Dynamics
                       float ampVelocity, float globalPitchBendSemitones,
                       float globalModWheel, float globalChannelAftertouch) noexcept {
        float bL = 0.0f, bR = 0.0f;
        renderBlockAdd(&bL, &bR, 1, 1,
                       oscBankIdx, oscWaveMorph, oscMask, oscDetune, oscOctave,
                       oscOctLock, oscSync, oscSpeed, oscLoop,
                       osc1Level, osc2Level, osc3Level, noiseLevel, noiseTone, oscSpread, mixerSeethe,
                       chaosRate, chaosDrift, chaosPolySpread, randAttenuverters,
                       zModel, zFreq, zMorph, zTransform, filterEnvDepth, filterVelocity,
                       ampVelocity, globalPitchBendSemitones, globalModWheel, globalChannelAftertouch);
        outL = bL;
        outR = bR;
    }

    void updateWavetables(const uint32_t* bankIdx, const float* waveMorph, const float* randAttenuverters,
                          const std::array<std::array<float, 512>, 3>* sharedTables = nullptr) noexcept {
        for (int i = 0; i < 3; ++i) {
            float effWave = juce::jlimit(0.0f, 1.0f,
                waveMorph[i] + chaos_.getModulation(i * 3 + 0, randAttenuverters[i * 3 + 0]));
            effWave_[i] = effWave;
            if (sharedTables != nullptr && randAttenuverters[i * 3 + 0] == 0.0f) {
                oscs_[i].copyTable((*sharedTables)[i].data(), bankIdx[i], waveMorph[i]);
            } else {
                oscs_[i].updateTable(bankIdx[i], effWave);
            }
        }
    }

    void advanceChaos(float rate, float drift, double blockSec) noexcept {
        chaos_.updateBlock(rate, drift, blockSec);
    }

    Envelope& getAmpEnv() noexcept { return ampEnv_; }
    Envelope& getFilterEnv() noexcept { return filterEnv_; }

private:
    int voiceIndex_ = 0;
    double sampleRate_ = 44100.0;
    int midiNote_ = 60;
    float velocity_ = 0.8f;
    float currentPitchHz_ = 440.0f;
    float targetPitchHz_ = 440.0f;
    float glideSpeed_ = 1.0f;

    float mpePitchBendSemitones_ = 0.0f;
    float mpeSlide_ = 0.0f;
    float mpePressure_ = 0.0f;

    uint32_t noiseSeed_ = 12345;
    int noiseHoldCounter_ = 0;
    float noiseSample_ = 0.0f;

    uint32_t ctrlCounter_ = 0;
    float cachedOctMult_[3] = { 1.0f, 1.0f, 1.0f };
    float cachedBendMult_ = 1.0f;
    float effWave_[3] = { 0.0f, 0.0f, 0.0f };
    float cachedEffSpeed_[3] = { 0.0f, 0.0f, 0.0f };
    int cachedEffMask_[3] = { 511, 511, 511 };
    float lastSemis_[3] = { 1e9f, 1e9f, 1e9f };
    float lastTotalBend_ = 1e9f;

    std::array<BitmaskOscillator, 3> oscs_;
    ZDomainFilter14 filter_;
    CrackedChimes::DSP::LeakyDCBlocker mixerDcBlockerL_;
    CrackedChimes::DSP::LeakyDCBlocker mixerDcBlockerR_;
    CrackedChimes::DSP::LeakyDCBlocker postFilterDcBlockerL_;
    CrackedChimes::DSP::LeakyDCBlocker postFilterDcBlockerR_;
    VoiceChaosEngine chaos_;
    Envelope ampEnv_;
    Envelope filterEnv_;
};

} // namespace Nyarlat
