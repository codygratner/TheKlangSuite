#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <atomic>
#include "shoggoth_layout.h"
#include "ShoggothParameterIds.h"
#include "ShoggothParameterLayout.h"
#include "ShoggothControlInfo.h"
#include "ShoggothVoice.h"
#include "ShoggothSysExParser.h"
#include "ShoggothAudioResynthesizer.h"
#include "PresetManager.h"

namespace Shoggoth {

struct ShoggothModVisual {
    bool isModulated = false;
    float rangeMinNorm = 0.0f;
    float rangeMaxNorm = 0.0f;
    float currentNorm = 0.0f;
};

class ShoggothProcessor : public juce::AudioProcessor {
public:
    ShoggothProcessor();
    ~ShoggothProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Shoggoth"; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Telemetry & Modulation Halos
    ShoggothModVisual getParamModulationInfo(const juce::String& paramId) const noexcept;

    // File Ingestion API
    bool loadSysExFile(const juce::File& file);
    bool loadAudioFile(const juce::File& file);

    // Spectrum Data Accessors for GUI Canvas
    float getHarmonicLevel(int sourceIdx, int harmonicIdx) const noexcept;
    void setHarmonicLevel(int sourceIdx, int harmonicIdx, float level99) noexcept;
    uint8_t getHarmonicEnvAssign(int harmonicIdx) const noexcept;
    void setHarmonicEnvAssign(int harmonicIdx, uint8_t envIdx) noexcept;

    // Universal Preset Manager & State Hooks
    RlyehSound::PresetManager& getPresetManager() noexcept { return presetManager_; }
    void loadCustomPresetState(const juce::var& customState);
    juce::var saveCustomPresetState() const;

    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts_; }
    std::atomic<float>& getStoredEditorScale() noexcept { return storedEditorScale_; }

private:
    juce::AudioProcessorValueTreeState apvts_;
    RlyehSound::PresetManager presetManager_{"Shoggoth"};
    std::array<Voice, kMaxVoices> voices_;

    // Master Additive Spectrum State (S1 = 1..63, S2 = 1..63)
    std::array<float, 63> s1Harmonics_{};
    std::array<float, 63> s2Harmonics_{};
    std::array<uint8_t, 126> harmonicEnvAssign_{};
    std::array<DHGEnvelopeData, kNumDHGEnv> dhgEnvelopes_{};

    // Voice Allocation & Stealing
    uint32_t nextNoteId_{1};
    int allocateVoice(int midiNote, int voiceMode) noexcept;
    void setupVoiceHarmonics(Voice& voice, int activeLimit, bool modernMode) noexcept;

    // UI Zoom
    std::atomic<float> storedEditorScale_{1.0f};

    // MIDI & Modulation State
    float pitchBendSemitones_{0.0f};
    float modWheelNorm_{0.0f};
    float mpeSlideNorm_{0.0f};
    float mpePressureNorm_{0.0f};
    float lastNoteF0_{440.0f};
    bool sustainPedalDown_{false};

    void initDefaultHarmonics() noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShoggothProcessor)
};

} // namespace Shoggoth
