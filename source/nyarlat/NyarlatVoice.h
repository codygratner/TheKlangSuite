#pragma once



#include <cmath>

#include <juce_audio_basics/juce_audio_basics.h>

#include "NyarlatOscillator.h"

#include "NyarlatAnalogFilter.h"

#include "NyarlatChaosMatrix.h"

#include "../dsp/common/SteppedRandom.h"

#include "../hastur/HasturFilterUtils.h"

#include "../FastMath.h"



namespace Nyarlat {



// Stage 2: Slew-Smoothed Optical RMS Stem Leveller (-36 dBFS expansion floor)

struct VoiceOptoLeveller {

    float powerMs = 0.0f;

    float currentGain = 1.0f;

    float targetGain = 1.0f;

    float slewCoeff = 0.001f;

    float attCoeff = 0.005f;

    float relCoeff = 0.0003f;



    void prepare(double sampleRate) noexcept {

        double sr = (sampleRate > 0.0) ? sampleRate : 44100.0;

        attCoeff = static_cast<float>(1.0 - std::exp(-1.0 / (0.012 * sr)));

        relCoeff = static_cast<float>(1.0 - std::exp(-1.0 / (0.180 * sr)));

        slewCoeff = static_cast<float>(1.0 - std::exp(-1.0 / (0.025 * sr)));

        reset();

    }



    void reset() noexcept {

        powerMs = 0.0f;

        currentGain = 1.0f;

        targetGain = 1.0f;

    }



    void updateControlRate() noexcept {

        const float rms = std::sqrt(powerMs + 1e-9f);

        constexpr float kTargetRms = 0.20f;     // -14 dBFS

        constexpr float kFloorRms  = 0.01585f;  // -36 dBFS

        constexpr float kMinRms    = 0.007925f; // -42 dBFS



        if (rms <= kMinRms) {

            targetGain = 1.0f;

        } else if (rms < kFloorRms) {

            float u = (rms - kMinRms) / (kFloorRms - kMinRms);

            float s = u * u * (3.0f - 2.0f * u);

            float activeGain = std::sqrt(kTargetRms / kFloorRms);

            targetGain = 1.0f + (activeGain - 1.0f) * s;

        } else {

            float idealGain = std::sqrt(kTargetRms / rms);

            targetGain = juce::jlimit(0.501f, 1.412f, idealGain);

        }

    }



    inline void processSample(float& l, float& r) noexcept {

        const float p = 0.5f * (l * l + r * r);

        const float coeff = (p > powerMs) ? attCoeff : relCoeff;

        powerMs += coeff * (p - powerMs);

        currentGain += slewCoeff * (targetGain - currentGain);

        l *= currentGain;

        r *= currentGain;

    }

};



// Pillar 3: Vintage Hardware 18 kHz Analog Reconstruction Filter

struct OnePoleReconstructionLP {

    float yL = 0.0f;

    float yR = 0.0f;

    float a = 0.0950f;



    void prepare(double sampleRate) noexcept {

        double sr = (sampleRate > 0.0) ? sampleRate : 44100.0;

        a = static_cast<float>(std::exp(-6.28318530717958647692 * 18000.0 / sr));

        reset();

    }



    void reset() noexcept {

        yL = 0.0f;

        yR = 0.0f;

    }



    inline void process(float& l, float& r) noexcept {

        yL = yL + (1.0f - a) * (l - yL);

        yR = yR + (1.0f - a) * (r - yR);

        l = yL;

        r = yR;

    }

};



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

        optoLeveller_.prepare(sampleRate);

        reconstructionLp_.prepare(sampleRate);

        panSlewCoeff_ = static_cast<float>(1.0 - std::exp(-1.0 / (0.015 * (sampleRate > 0.0 ? sampleRate : 44100.0))));

        hammerDecayCoeff_ = static_cast<float>(std::exp(-1.0 / (0.0025 * (sampleRate > 0.0 ? sampleRate : 44100.0))));

        postFilterDcBlockerL_.prepare(sampleRate, 5.0);

        postFilterDcBlockerR_.prepare(sampleRate, 5.0);

        reset();

    }



    void reset() noexcept {

        for (auto& osc : oscs_) osc.reset();

        filter_.reset();

        optoLeveller_.reset();

        reconstructionLp_.reset();

        curPan2L_ = 0.7071068f;

        curPan2R_ = 0.7071068f;

        targetPan2L_ = 0.7071068f;

        targetPan2R_ = 0.7071068f;

        postFilterDcBlockerL_.reset();

        postFilterDcBlockerR_.reset();

        ampEnv_.stage = EnvStage::Idle;

        ampEnv_.level = 0.0f;

        filterEnv_.stage = EnvStage::Idle;

        filterEnv_.level = 0.0f;

        currentPitchHz_ = 440.0f;

        targetPitchHz_ = 440.0f;

        glideSpeed_ = 1.0f;

        mpePitchBendSemitones_ = 0.0f;

        mpeSlide_ = 0.0f;

        mpePressure_ = 0.0f;

        noiseHoldCounter_ = 0;

        noiseSample_ = 0.0f;

        hammerSnapEnv_ = 0.0f;

        ctrlCounter_ = 0;

        cachedOctMult_[0] = 1.0f; cachedOctMult_[1] = 1.0f; cachedOctMult_[2] = 1.0f;

        cachedBendMult_ = 1.0f;

        effMorph_[0] = 0.0f; effMorph_[1] = 0.0f; effMorph_[2] = 0.0f;

        cachedEffSpeed_[0] = 0.0f; cachedEffSpeed_[1] = 0.0f; cachedEffSpeed_[2] = 0.0f;

        cachedEffMask_[0] = 511; cachedEffMask_[1] = 511; cachedEffMask_[2] = 511;

        lastSemis_[0] = lastSemis_[1] = lastSemis_[2] = 1e9f;

        lastTotalBend_ = 1e9f;

    }



    void sanitize() noexcept {

        filter_.sanitize();

    }



    void noteOn(int midiNote, float velocity, float glideTimeSec, float hammerSnap = 0.0f, float targetHz = -1.0f) noexcept {

        midiNote_ = midiNote;

        velocity_ = velocity;

        targetPitchHz_ = (targetHz > 0.0f) ? targetHz : 440.0f * std::pow(2.0f, (midiNote - 69.0f) / 12.0f);



        if (glideTimeSec <= 0.001f || !ampEnv_.isActive()) {

            currentPitchHz_ = targetPitchHz_;

        }

        glideSpeed_ = (glideTimeSec > 0.001f)

            ? static_cast<float>(1.0 / (glideTimeSec * sampleRate_))

            : 1.0f;



        hammerSnapEnv_ = hammerSnap * velocity;

        chaos_.triggerNoteOn();

        ampEnv_.noteOn();

        filterEnv_.noteOn();

        ctrlCounter_ = 0;

    }



    void setPitchTarget(float targetHz, float glideTimeSec) noexcept {

        targetPitchHz_ = targetHz;

        glideSpeed_ = (glideTimeSec > 0.001f)

            ? static_cast<float>(1.0 / (glideTimeSec * sampleRate_))

            : 1.0f;

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

    float getEffMorph(int osc) const noexcept {

        return (osc >= 0 && osc < 3) ? effMorph_[osc] : 0.0f;

    }

    float getEffWave(int osc) const noexcept {

        return getEffMorph(osc);

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



    void renderBlockAdd(float* outL, float* outR, int numSamples, int activeVoiceCount,

                        // 3x Additive Oscillators parameters

                        const float* oscBankIdx, const float* oscMorph, const float* oscOddEven,

                        const float* oscTilt, const float* oscFormant, const int* oscMask,

                        const float* oscDetune, const int* oscOctave, const bool* oscSpeedLock,

                        const bool* oscSync, const float* oscSpeed, const int* oscLoop,

                        float osc1Level, float osc2Level, float osc3Level,

                        // Mixer & Noise

                        float noiseLevel, float noiseTone, float noiseHammerSnap,

                        float oscSpread, float mixerSeethe,

                        // 18-Channel Chaos Attenuverters & Spread Chaos

                        const float* randAttenuverters, float randSpread,

                        // Silky Analog Filter

                        float filterCutoff, float filterResonance, int filterSlope, float filterDrive,

                        float filterEnvDepth, float filterVelocity, float filterKeytrack,

                        // Dynamics & Global

                        float ampVelocity, float globalPitchBendSemitones,

                        float globalModWheel, float globalChannelAftertouch,

                        float hostBpm = 120.0f) noexcept {



        juce::ignoreUnused(oscBankIdx, oscMorph, oscOddEven, oscTilt, oscFormant, noiseHammerSnap);



        if (!ampEnv_.isActive()) return;



        const float velScale = 1.0f - ampVelocity + (ampVelocity * velocity_ * velocity_);

        const int noiseHoldMax = 1 + static_cast<int>(noiseTone * 126.0f);

        const float keytrackOffset = (static_cast<float>(midiNote_ - 60) / 12.0f);



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



            // Control-rate decimation staggered across voices (every 32 samples)

            if (((ctrlCounter_++ + static_cast<uint32_t>(voiceIndex_ * 2)) & 31) == 0) {

                // Total pitch with Pitch Bend

                float totalBend = mpePitchBendSemitones_ + globalPitchBendSemitones;

                if (totalBend != lastTotalBend_) {

                    lastTotalBend_ = totalBend;

                    cachedBendMult_ = TbdAudio::FastMath::fastPow2(totalBend / 12.0f);

                }



                // Process 3 Oscillator Pitch Ratios (detune scaled: 100 ct = 1 st)

                for (int i = 0; i < 3; ++i) {

                    float semis = (oscOctave[i] - 2) * 12.0f + (oscDetune[i] * 0.01f);

                    if (semis != lastSemis_[i]) {

                        lastSemis_[i] = semis;

                        cachedOctMult_[i] = TbdAudio::FastMath::fastPow2(semis / 12.0f);

                    }

                }



                // Destination-anchored stator speed and mask aperture with chaos channels (12..17)

                const float safeBpm = (hostBpm > 20.0f && hostBpm < 999.0f && std::isfinite(hostBpm)) ? hostBpm : 120.0f;

                for (int i = 0; i < 3; ++i) {

                    cachedEffMask_[i] = static_cast<int>(CrackedChimes::DSP::stepRandomRange(

                        static_cast<float>(oscMask[i]), 0.0f, 511.0f, randAttenuverters[12 + i], chaos_.getRawValue(12 + i)));



                    if (oscSpeedLock[i]) {

                        // Discrete 1/16th note tempo grid (1/16 to 64/16 in 1/16th steps)

                        const float u = juce::jlimit(0.0f, 1.0f, oscSpeed[i] * 0.1f);

                        const int dialedN = 1 + static_cast<int>(std::round(63.0f * u));



                        // Modulate N via Chaos Channel (15 + i)

                        const float chaosRaw = chaos_.getRawValue(15 + i);

                        const float chaosDepth = randAttenuverters[15 + i];

                        const int deltaN = static_cast<int>(std::round(chaosDepth * (chaosRaw * 63.0f)));



                        const int effN = std::clamp(dialedN + deltaN, 1, 64);

                        // Frequency for N sixteenth notes: f = B / (15 * N) Hz

                        cachedEffSpeed_[i] = safeBpm / (15.0f * static_cast<float>(effN));

                    } else {

                        cachedEffSpeed_[i] = CrackedChimes::DSP::stepRandomRange(

                            oscSpeed[i], 0.0f, 10.0f, randAttenuverters[15 + i], chaos_.getRawValue(15 + i));

                    }

                }



                // Stage 2: Update Opto Leveller

                optoLeveller_.updateControlRate();



                // Destination-anchored Bipolar Spread (Channel 18)

                const float effSpread = CrackedChimes::DSP::stepRandomRange(

                    oscSpread, -1.0f, 1.0f, randSpread, chaos_.getRawValue(18));

                lastEffSpread_ = effSpread;

                targetPan2L_ = std::sqrt(0.5f * (1.0f - effSpread));

                targetPan2R_ = std::sqrt(0.5f * (1.0f + effSpread));

            }



            // Slew pan gains towards target

            curPan2L_ += (targetPan2L_ - curPan2L_) * panSlewCoeff_;

            curPan2R_ += (targetPan2R_ - curPan2R_) * panSlewCoeff_;



            float basePitch = currentPitchHz_ * cachedBendMult_;



            // 3. Process 3 Oscillators with Hard Sync & Q32 Stator Sweep

            const uint32_t prevP1 = oscs_[0].getPhase() & 0x00FFFFFFu;

            float sig1 = oscs_[0].processSample(basePitch * cachedOctMult_[0], cachedEffMask_[0], cachedEffSpeed_[0], oscLoop[0], false);

            bool syncTrigger = ((oscs_[0].getPhase() & 0x00FFFFFFu) < prevP1);



            float sig2 = oscs_[1].processSample(basePitch * cachedOctMult_[1], cachedEffMask_[1], cachedEffSpeed_[1], oscLoop[1], false, oscSync[1] && syncTrigger);

            float sig3 = oscs_[2].processSample(basePitch * cachedOctMult_[2], cachedEffMask_[2], cachedEffSpeed_[2], oscLoop[2], false, oscSync[2] && syncTrigger);



            // 4. Variable-rate Sample-and-Hold Noise + Clavinet Hammer Snap

            if (++noiseHoldCounter_ >= noiseHoldMax) {

                noiseHoldCounter_ = 0;

                noiseSeed_ = 214013 * noiseSeed_ + 2531011;

                noiseSample_ = static_cast<float>((noiseSeed_ >> 16) & 0x7FFF) / 16384.0f - 1.0f;

            }



            hammerSnapEnv_ *= hammerDecayCoeff_;

            const float totalNoise = noiseSample_ * (noiseLevel + hammerSnapEnv_ * 0.7f);



            // 5. Pre-Filter Stereo Stem Mixer with Constant-Power Bipolar Spread

            float mixL = (sig1 * osc1Level * 0.7071068f + sig2 * osc2Level * curPan2L_ + sig3 * osc3Level * curPan2R_ + totalNoise) * 0.333f;

            float mixR = (sig1 * osc1Level * 0.7071068f + sig2 * osc2Level * curPan2R_ + sig3 * osc3Level * curPan2L_ + totalNoise) * 0.333f;



            // --- STAGE 2: Slew-Smoothed Opto RMS Leveller (-36 dBFS floor) ---

            optoLeveller_.processSample(mixL, mixR);



            // --- PILLAR 3: 18 kHz Vintage Analog Reconstruction Filter ---

            reconstructionLp_.process(mixL, mixR);



            // --- STAGE 3: Console Seethe Drive & Hard Ceiling ---

            if (mixerSeethe > 0.001f) {

                const float drive = 1.0f + mixerSeethe * 3.0f;

                const float makeup = 1.0f / std::sqrt(1.0f + 0.6f * mixerSeethe);

                mixL = TbdAudio::FastMath::fastTanh(mixL * drive) * makeup;

                mixR = TbdAudio::FastMath::fastTanh(mixR * drive) * makeup;

            }



            // 7. Process Silky Analog ZDF Filter

            const float effCutoff = juce::jlimit(15.0f, 20000.0f,

                filterCutoff * std::pow(2.0f, filtEnvVal * filterEnvDepth * 4.0f +

                                              (velocity_ - 0.5f) * filterVelocity * 2.0f +

                                              keytrackOffset * filterKeytrack +

                                              mpeSlide_ * 2.0f +

                                              std::max(mpePressure_, globalChannelAftertouch) * 2.0f));



            filter_.processStereo(mixL, mixR, effCutoff, filterResonance, filterSlope, filterDrive);



            // Post-Filter DC Blocker

            mixL = postFilterDcBlockerL_.process(mixL);

            mixR = postFilterDcBlockerR_.process(mixR);



            // 8. VCA Stage with -12 dB Headroom Calibration

            constexpr float kVoiceHeadroom = 0.25f; // -12 dB calibrated polyphonic headroom

            float totalGain = ampGain * velScale * kVoiceHeadroom;

            outL[s] += mixL * totalGain;

            outR[s] += mixR * totalGain;

        }

    }



    void updateWavetables(const uint32_t* bankIdx, const float* oscMorph, const float* oscOddEven,

                          const float* oscTilt, const float* oscFormant, const float* randAttenuverters,

                          int lifeMode) noexcept {

        for (int i = 0; i < 3; ++i) {

            // Destination-anchored unipolar chaos modulation

            const float effMorph = CrackedChimes::DSP::stepRandomNorm(

                oscMorph[i], randAttenuverters[i], chaos_.getRawValue(i));

            const float effOddEven = CrackedChimes::DSP::stepRandomRange(

                oscOddEven[i], -1.0f, 1.0f, randAttenuverters[3 + i], chaos_.getRawValue(3 + i));

            const float effTilt = CrackedChimes::DSP::stepRandomRange(

                oscTilt[i], -12.0f, 6.0f, randAttenuverters[6 + i], chaos_.getRawValue(6 + i));

            const float effFormant = CrackedChimes::DSP::stepRandomNorm(

                oscFormant[i], randAttenuverters[9 + i], chaos_.getRawValue(9 + i));



            effMorph_[i] = effMorph;

            oscs_[i].updateTable(bankIdx[i], effMorph, effOddEven, effTilt, effFormant, lifeMode);

        }

    }



    void advanceChaos(float rate, float drift, double blockSec) noexcept {

        chaos_.updateBlock(rate, drift, blockSec);

    }



    void setPolyChaosSpread(float spread) noexcept {

        chaos_.setVoiceSeed(voiceIndex_, spread);

    }



    void getChaosTelemetry(float* dest, int count) const noexcept {

        chaos_.copyRawValues(dest, count);

    }



    Envelope& getAmpEnv() noexcept { return ampEnv_; }

    Envelope& getFilterEnv() noexcept { return filterEnv_; }

    float getEffSpread() const noexcept { return lastEffSpread_; }



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

    float hammerSnapEnv_ = 0.0f;

    float hammerDecayCoeff_ = 0.99f;



    uint32_t ctrlCounter_ = 0;

    float cachedOctMult_[3] = { 1.0f, 1.0f, 1.0f };

    float cachedBendMult_ = 1.0f;

    float effMorph_[3] = { 0.0f, 0.0f, 0.0f };

    float cachedEffSpeed_[3] = { 0.0f, 0.0f, 0.0f };

    int cachedEffMask_[3] = { 511, 511, 511 };

    float lastSemis_[3] = { 1e9f, 1e9f, 1e9f };

    float lastTotalBend_ = 1e9f;



    std::array<BitmaskOscillator, 3> oscs_;

    NyarlatAnalogFilter filter_;

    VoiceOptoLeveller optoLeveller_;

    OnePoleReconstructionLP reconstructionLp_;

    float curPan2L_ = 0.7071068f;

    float curPan2R_ = 0.7071068f;

    float targetPan2L_ = 0.7071068f;

    float targetPan2R_ = 0.7071068f;

    float panSlewCoeff_ = 0.0015f;

    CrackedChimes::DSP::LeakyDCBlocker postFilterDcBlockerL_;

    CrackedChimes::DSP::LeakyDCBlocker postFilterDcBlockerR_;

    VoiceChaosEngine chaos_;

    Envelope ampEnv_;

    Envelope filterEnv_;

    float lastEffSpread_ = 0.0f;

};



} // namespace Nyarlat

