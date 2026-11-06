#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <atomic>
#include "NyarlatVoice.h"
#include "AbyssReverb.h"
#include "NyarlatParameterIds.h"
#include "NyarlatParameterLayout.h"
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

    std::atomic<float> storedEditorScale_{0.0f};
    void setStoredEditorScale(float s) noexcept { storedEditorScale_.store(s, std::memory_order_relaxed); }
    [[nodiscard]] float getStoredEditorScale() const noexcept { return storedEditorScale_.load(std::memory_order_relaxed); }

    // Telemetry for Z-Cube Visualizer
    struct VoiceTelemetry {
        bool active = false;
        int note = 60;
        float x = 0.5f;
        float y = 0.0f;
        float z = 0.0f;
    };
    std::array<VoiceTelemetry, kNumVoices> getVoiceTelemetry() const noexcept {
        return telemetry_;
    }

private:
    void handleMidiEvent(const juce::MidiMessage& msg) noexcept;
    void allocateVoice(int midiNote, float velocity) noexcept;
    void releaseVoice(int midiNote) noexcept;

    juce::AudioProcessorValueTreeState apvts_;

    std::array<NyarlatVoice, kNumVoices> voices_;
    AbyssReverb reverb_;

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

    // Alpha Chorus State
    std::vector<float> chorusBufferL_;
    std::vector<float> chorusBufferR_;
    int chorusPtr_ = 0;
    float chorusPhase_ = 0.0f;

    // MIDI / MPE Expression State
    float globalPitchWheel_ = 0.0f;
    float globalModWheel_ = 0.0f;
    float globalChannelAftertouch_ = 0.0f;
    int nextVoiceIndex_ = 0;

    std::array<VoiceTelemetry, kNumVoices> telemetry_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NyarlatProcessor)
};

} // namespace Nyarlat
