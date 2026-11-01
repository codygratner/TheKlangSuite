#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "dsp/drums/FormlessSpawnEngine.h"

class FormlessSpawnAudioProcessor : public juce::AudioProcessor {
public:
    FormlessSpawnAudioProcessor();
    ~FormlessSpawnAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override { return "Formless Spawn"; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts_; }
    tkdsp::FormlessSpawnEngine& getEngine() { return engine_; }

private:
    tkdsp::FormlessSpawnEngine engine_;
    juce::UndoManager undoManager_;
    juce::AudioProcessorValueTreeState apvts_;

    // Cached raw parameter pointers for lock-free audio thread access
    std::array<std::array<std::atomic<float>*, tkdsp::FormlessSpawnEngine::kNumParamsPerVoice>, tkdsp::FormlessSpawnEngine::kTotalVoices> cachedParams_{};
    std::atomic<float>* masterLowBoostParam_{nullptr};
    std::atomic<float>* masterDriveParam_{nullptr};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FormlessSpawnAudioProcessor)
};
