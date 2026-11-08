#pragma once



#include <juce_audio_processors/juce_audio_processors.h>

#include <juce_audio_basics/juce_audio_basics.h>

#include <array>

#include <vector>

#include <atomic>

#include "NyarlatVoice.h"


#include "NyarlatParameterIds.h"

#include "NyarlatParameterLayout.h"

#include "../PresetManager.h"

#include "../hastur/HasturFilterUtils.h"



namespace Nyarlat {



struct SvfBandpass {

    float ic1 = 0.0f, ic2 = 0.0f;

    float g = 0.0f, k = 1.0f, a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;

    void set(float fc, float Q, float fs) noexcept {

        g = std::tan(juce::MathConstants<float>::pi * fc / std::max(1000.0f, fs));

        k = 1.0f / std::max(0.1f, Q);

        a1 = 1.0f / (1.0f + g * (g + k));

        a2 = g * a1;

        a3 = g * a2;

    }

    float process(float v) noexcept {

        const float v3 = v - ic2;

        const float v1 = a1 * ic1 + a2 * v3;

        const float v2 = ic2 + a2 * ic1 + a3 * v3;

        ic1 = 2.0f * v1 - ic1;

        ic2 = 2.0f * v2 - ic2;

        return k * v1; // unity peak bandpass

    }

    void reset() noexcept { ic1 = 0.0f; ic2 = 0.0f; }

};



// Dual 90-Degree Polyphase Hilbert Transform All-Pass Network for Continuous Alpha Chorus

struct HilbertNetwork {

    static constexpr std::array<float, 4> cI = {

        0.69238780f, 0.93606543f, 0.98822952f, 0.99874885f

    };

    static constexpr std::array<float, 4> cQ = {

        0.40219212f, 0.85617109f, 0.97229095f, 0.99528848f

    };

    std::array<float, 4> xI_{}, yI_{}, xQ_{}, yQ_{};

    float z1Q_ = 0.0f;



    void reset() noexcept {

        xI_.fill(0.0f); yI_.fill(0.0f);

        xQ_.fill(0.0f); yQ_.fill(0.0f);

        z1Q_ = 0.0f;

    }



    inline void process(float in, float& sigI, float& sigQ) noexcept {

        sigI = in;

        for (int s = 0; s < 4; ++s) {

            const float c = cI[s];

            const float y = c * (sigI + yI_[s]) - xI_[s];

            xI_[s] = sigI; yI_[s] = y; sigI = y;

        }

        sigQ = z1Q_;

        z1Q_ = in;

        for (int s = 0; s < 4; ++s) {

            const float c = cQ[s];

            const float y = c * (sigQ + yQ_[s]) - xQ_[s];

            xQ_[s] = sigQ; yQ_[s] = y; sigQ = y;

        }

    }

};



class NyarlatProcessor : public juce::AudioProcessor {

public:

    static constexpr int kNumVoices = 16;



    NyarlatProcessor();

    ~NyarlatProcessor() override = default;



    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    void releaseResources() override;

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;



    juce::AudioProcessorEditor* createEditor() override;

    bool hasEditor() const override { return true; }



    const juce::String getName() const override { return "Nyarlat"; }

    bool acceptsMidi() const override { return true; }

    bool producesMidi() const override { return false; }

    bool isMidiEffect() const override { return false; }

    double getTailLengthSeconds() const override { return 3.0; }



    int getNumPrograms() override { return 1; }

    int getCurrentProgram() override { return 0; }

    void setCurrentProgram(int) override {}

    const juce::String getProgramName(int) override { return {}; }

    void changeProgramName(int, const juce::String&) override {}



    void getStateInformation(juce::MemoryBlock& destData) override;

    void setStateInformation(const void* data, int sizeInBytes) override;



    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts_; }

    RlyehSound::PresetManager& getPresetManager() noexcept { return presetManager_; }



    std::atomic<float> storedEditorScale_{0.0f};

    void setStoredEditorScale(float s) noexcept { storedEditorScale_.store(s, std::memory_order_relaxed); }

    [[nodiscard]] float getStoredEditorScale() const noexcept { return storedEditorScale_.load(std::memory_order_relaxed); }



    // Telemetry for Stator Views and Modulation Arc Halos

    struct VoiceTelemetry {

        bool active = false;

        int note = 60;

        float x = 0.5f;

        float y = 0.0f;

        float z = 0.0f;

        uint32_t statorOffsets[3] = {0, 0, 0};

        float oscEffWave[3] = {0.0f, 0.0f, 0.0f};

        int oscEffMask[3] = {511, 511, 511};

        float oscEffSpeed[3] = {0.0f, 0.0f, 0.0f};

        float effSpread = 0.0f;

        float filterEnv = 0.0f;

        float rawChaos[19] = {0.0f};

    };

    std::array<VoiceTelemetry, kNumVoices> getVoiceTelemetry() const noexcept {

        return telemetry_;

    }

    int getNewestVoice() const noexcept {

        return newestVoice_.load(std::memory_order_relaxed);

    }

    float getBpm() const noexcept {

        return currentBpm_.load(std::memory_order_relaxed);

    }

    [[nodiscard]] float getMasterChorusChaos() const noexcept {
        return masterChorusChaos_.load(std::memory_order_relaxed);
    }



private:

    struct VoiceParams {

        float oscBanks[3];

        float oscMorphs[3];

        float oscOddEvens[3];

        float oscTilts[3];

        float oscFormants[3];

        int oscMasks[3];

        int oscOctaves[3];

        bool oscSpeedLocks[3];

        float bpm = 120.0f;

        float oscSpeeds[3];

        int oscLoops[3];

        float osc1Lvl, osc2Lvl, osc3Lvl;

        float noiseLvl, noiseTone, noiseHammerSnap;

        float oscSpread, mixerSeethe;
        float width = 0.5f;

        float chaosRate, chaosDrift, chaosPoly;

        float randChaos[6] = {0.0f};
        float randAttenuverters[18] = {0.0f};
        float randSpread = 0.0f;
        float randTune = 0.0f;
        float randChorusRate = 0.0f;

        float filterCutoff, filterResonance;

        int filterSlope;

        float filterDrive;

        float filterEnvDepth, filterVelocity, filterKeytrack;

        float ampVel, globalBendSemi;

        int lifeMode;

        float oscSlop;

    };



    void handleMidiEvent(const juce::MidiMessage& msg) noexcept;

    void allocateVoice(int midiNote, float velocity) noexcept;

    void releaseVoice(int midiNote) noexcept;

    void renderSegment(float* left, float* right, int numSamples, const VoiceParams& params, int activeVoiceCount) noexcept;



    juce::AudioProcessorValueTreeState apvts_;

    RlyehSound::PresetManager presetManager_{"Nyarlat"};



    std::array<NyarlatVoice, kNumVoices> voices_;




    // Master 40Hz Bass Bandpass State

    SvfBandpass bassBpL_;

    SvfBandpass bassBpR_;

    CrackedChimes::DSP::LeakyDCBlocker masterDcBlockerL_;

    CrackedChimes::DSP::LeakyDCBlocker masterDcBlockerR_;

    CrackedChimes::DSP::LeakyDCBlocker postDcBlockerL_;

    CrackedChimes::DSP::LeakyDCBlocker postDcBlockerR_;



    // Treble Tame State

    float trebleStateL_ = 0.0f;

    float trebleStateR_ = 0.0f;



    // Continuous Zero-Latency Hilbert Alpha Chorus

    HilbertNetwork hilbertL_;

    HilbertNetwork hilbertR_;

    float chorusPhase_ = 0.0f;
    Nyarlat::VoiceChaosEngine masterChaos_;
    float smoothedChorusRate_ = 1.2f;



    // Note stack for Mono / Legato / Unison modes

    std::vector<std::pair<int, float>> heldNotes_;



    // MIDI / MPE Expression State

    float globalPitchWheel_ = 0.0f;

    float globalModWheel_ = 0.0f;

    float globalChannelAftertouch_ = 0.0f;

    int nextVoiceIndex_ = 0;

    uint64_t noteCounter_ = 0;

    int masterSilenceSamples_ = 0;

    bool masterSleeping_ = false;



    std::array<VoiceTelemetry, kNumVoices> telemetry_;

    std::atomic<int> newestVoice_{-1};

    std::atomic<float> currentBpm_{120.0f};
    std::atomic<float> masterChorusChaos_{0.5f};



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NyarlatProcessor)

};



} // namespace Nyarlat

