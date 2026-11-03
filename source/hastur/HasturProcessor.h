#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include "HasturVoice.h"
#include "../FastMath.h"
#include <atomic>
#include <array>

class HasturAudioProcessor : public juce::AudioProcessor {
public:
    HasturAudioProcessor();
    ~HasturAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Hastur"; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.5; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts_; }

    [[nodiscard]] bool isPllLocked() const noexcept { return voice_.isPllLocked(); }

    [[nodiscard]] float getStepRandAtomic(int i) const noexcept {
        return voice_.getStepRandAtomic(i);
    }

    std::atomic<int> uiLfoRateMode_{0};
    void setUiLfoRateMode(int mode) noexcept { uiLfoRateMode_.store(mode, std::memory_order_relaxed); }
    [[nodiscard]] int getUiLfoRateMode() const noexcept { return uiLfoRateMode_.load(std::memory_order_relaxed); }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::UndoManager undoManager_;
    juce::AudioProcessorValueTreeState apvts_;

    CrackedChimes::DSP::HasturVoice voice_;

    // Bay 1: DCO
    std::atomic<float>* osc1MultParam_{nullptr};
    std::atomic<float>* osc1ApfPitchTrackParam_{nullptr};
    std::atomic<float>* osc1DriveParam_{nullptr};
    std::atomic<float>* dcoBiteParam_{nullptr};
    std::atomic<float>* subApfDepthParam_{nullptr};
    std::atomic<float>* subSpitParam_{nullptr};
    std::atomic<float>* dcoMorphParam_{nullptr};
    std::atomic<float>* dcoWaveFamilyParam_{nullptr};
    std::atomic<float>* dcoChopperTopoParam_{nullptr};
    std::atomic<float>* subOctaveParam_{nullptr};
    std::atomic<float>* subPulseTrainParam_{nullptr};
    std::atomic<float>* subApfTapParam_{nullptr};

    // Bay 2: Stem Mixer
    std::atomic<float>* osc1LevelParam_{nullptr};
    std::atomic<float>* osc2LevelParam_{nullptr};
    std::atomic<float>* noiseLevelParam_{nullptr};
    std::atomic<float>* mixerDriveParam_{nullptr};
    // masterSpreadParam_ excised
    std::atomic<float>* oscSlopParam_{nullptr};

    // Bay 3: Dual SVF VCF
    std::atomic<float>* cutoffParam_{nullptr};
    std::atomic<float>* resonanceParam_{nullptr};
    std::atomic<float>* filterEnvDecayParam_{nullptr};
    std::atomic<float>* burbleDepthParam_{nullptr};
    std::atomic<float>* laserDriveParam_{nullptr};
    std::atomic<float>* filterWarpParam_{nullptr};
    std::atomic<float>* laserBiteParam_{nullptr};
    std::atomic<float>* filterOrbitParam_{nullptr};
    std::atomic<float>* burbleModeParam_{nullptr};
    std::atomic<float>* filterDeckParam_{nullptr};
    std::atomic<float>* envDepthParam_{nullptr};
    std::atomic<float>* filterVelocityParam_{nullptr};
    std::atomic<float>* keyTrackParam_{nullptr};
    std::atomic<float>* filterLfoDepthParam_{nullptr};
    std::atomic<float>* filterLfoDestParam_{nullptr};

    // Bay 4: LFO & Glide
    std::atomic<float>* lfoRateParam_{nullptr};
    std::atomic<float>* glideTimeParam_{nullptr};
    std::atomic<float>* lfoWaveParam_{nullptr};
    std::atomic<float>* lfoShapeParam_{nullptr};
    std::atomic<float>* lfoSyncParam_{nullptr};
    std::atomic<float>* pwLfoAmountParam_{nullptr};
    std::atomic<float>* pwLfoRateParam_{nullptr};
    std::atomic<float>* glideModeParam_{nullptr};
    std::atomic<float>* modASourceParam_{nullptr};
    std::atomic<float>* modAAmountParam_{nullptr};
    std::atomic<float>* modBSourceParam_{nullptr};
    std::atomic<float>* modBAmountParam_{nullptr};

    // Stepped Random Modulation Matrix (Bay 4)
    std::atomic<float>* randCoreWarpParam_{nullptr};
    std::atomic<float>* randCoreBiteParam_{nullptr};
    std::atomic<float>* randCoreMeltParam_{nullptr};
    std::atomic<float>* randRawStrangleParam_{nullptr};
    std::atomic<float>* randSubChewParam_{nullptr};
    std::atomic<float>* randSubSpitParam_{nullptr};
    std::atomic<float>* randFilterTwistParam_{nullptr};
    std::atomic<float>* randFilterStabParam_{nullptr};
    std::atomic<float>* randFilterOrbitParam_{nullptr};

    // Bay 5: VCA & Transients
    std::atomic<float>* ampGateModeParam_{nullptr};
    std::atomic<float>* ampEnvSnapParam_{nullptr};
    std::atomic<float>* ampEnvDecayParam_{nullptr};
    std::atomic<float>* pitchPunchDepthParam_{nullptr};
    std::atomic<float>* pitchPunchTimeParam_{nullptr};
    std::atomic<float>* ampEnvSustainParam_{nullptr};
    std::atomic<float>* ampEnvReleaseParam_{nullptr};
    std::atomic<float>* postAmpDriveParam_{nullptr};
    std::atomic<float>* ampVelocityParam_{nullptr};

    // Bay 6: Master Output
    std::atomic<float>* alphaChorusParam_{nullptr};
    std::atomic<float>* globalWidthParam_{nullptr};
    std::atomic<float>* outputLevelParam_{nullptr};
    std::atomic<float>* lifeModeParam_{nullptr};
    std::atomic<float>* triggerModeParam_{nullptr};
    std::atomic<float>* pitchBendRangeParam_{nullptr};
    std::atomic<float>* vibratoRateParam_{nullptr};
    std::atomic<float>* vibratoDepthParam_{nullptr};

    float pitchBendNorm_{0.0f};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HasturAudioProcessor)
};
