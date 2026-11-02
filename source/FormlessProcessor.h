#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "dsp/drums/FormlessSpawnVoice.h"

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
    tkdsp::FormlessSpawnVoice& getVoice() { return voice_; }

    void triggerManual(float velocity = 0.95f) noexcept {
        triggerVoice(velocity, -1.0f);
    }

private:
    void triggerVoice(float velocity, float noteHz) noexcept;
    void loadVoiceParams(tkdsp::FormlessSpawnVoice::Params& p) const noexcept;

    tkdsp::FormlessSpawnVoice voice_;
    juce::UndoManager undoManager_;
    juce::AudioProcessorValueTreeState apvts_;

    // Cached raw parameter pointers for lock-free audio thread access
    std::atomic<float>* pitchParam_{nullptr};
    std::atomic<float>* pitchDepthParam_{nullptr};
    std::atomic<float>* pitchDecayParam_{nullptr};
    std::atomic<float>* pitchCurveParam_{nullptr};
    std::atomic<float>* clickTuneParam_{nullptr};
    std::atomic<float>* clickMixParam_{nullptr};
    std::atomic<float>* subLevelParam_{nullptr};
    std::atomic<float>* lowBoostParam_{nullptr};

    std::atomic<float>* timbreXParam_{nullptr};
    std::atomic<float>* timbreYParam_{nullptr};
    std::atomic<float>* timbreZParam_{nullptr};
    std::atomic<float>* timbreTParam_{nullptr};
    std::atomic<float>* fmMorphParam_{nullptr};
    std::atomic<float>* dampParam_{nullptr};
    std::atomic<float>* panParam_{nullptr};
    std::atomic<float>* velSensParam_{nullptr};

    std::atomic<float>* randPitchParam_{nullptr};
    std::atomic<float>* randDecayParam_{nullptr};
    std::atomic<float>* randTimbreParam_{nullptr};
    std::atomic<float>* randScatterParam_{nullptr};
    std::atomic<float>* randModeParam_{nullptr};
    std::atomic<float>* macroDecayParam_{nullptr};
    std::atomic<float>* volumeParam_{nullptr};

    std::atomic<float>* cutoffParam_{nullptr};
    std::atomic<float>* resonanceParam_{nullptr};
    std::atomic<float>* vcfModeParam_{nullptr};
    std::atomic<float>* preDriveParam_{nullptr};
    std::atomic<float>* crushParam_{nullptr};
    std::atomic<float>* bitsParam_{nullptr};
    std::atomic<float>* tapeGlueParam_{nullptr};
    std::atomic<float>* mojoProfileParam_{nullptr};
    std::atomic<float>* mojoMixParam_{nullptr};

    std::atomic<float>* roleParam_{nullptr};
    std::atomic<float>* modelParam_{nullptr};
    std::atomic<float>* lifeModeParam_{nullptr};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FormlessSpawnAudioProcessor)
};