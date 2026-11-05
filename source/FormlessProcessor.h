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

    std::atomic<float> storedEditorScale_{0.0f};
    void setStoredEditorScale(float s) noexcept { storedEditorScale_.store(s, std::memory_order_relaxed); }
    [[nodiscard]] float getStoredEditorScale() const noexcept { return storedEditorScale_.load(std::memory_order_relaxed); }

    void triggerManual(int noteNumber = -1, float velocity = -1.0f) noexcept {
        manualTriggerNote_.store(noteNumber, std::memory_order_release);
        manualTriggerVel_.store(velocity, std::memory_order_release);
        manualTriggerPending_.store(true, std::memory_order_release);
    }

    struct TelemetrySnapshot {
        std::array<float, 8> randVal{{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};
        float lastVelocity{0.9f};
        int noteNumber{60};
    };

    TelemetrySnapshot getTelemetrySnapshot() const noexcept {
        const int idx = activeTelemetryIdx_.load(std::memory_order_acquire);
        return telemetryBuffers_[static_cast<size_t>(idx)];
    }

    float getLatchedDraw(int i) const noexcept {
        if (i >= 0 && i < 8) return latchedDraw_[i].load(std::memory_order_relaxed);
        return 0.0f;
    }

    [[nodiscard]] float getLimiterGR() const noexcept {
        return limiterGR_.load(std::memory_order_relaxed);
    }

private:
    void triggerVoice(float velocity, float noteHz) noexcept;
    void loadVoiceParams(tkdsp::FormlessSpawnVoice::Params& p) const noexcept;
    void publishTelemetry(float velocity, int noteNumber = 60) noexcept;

    std::array<TelemetrySnapshot, 3> telemetryBuffers_{};
    std::atomic<int> activeTelemetryIdx_{0};

    tkdsp::FormlessSpawnVoice voice_;
    juce::UndoManager undoManager_;
    juce::AudioProcessorValueTreeState apvts_;

    // Cached raw parameter pointers for lock-free audio thread access
    std::atomic<float>* macroXParam_{nullptr};
    std::atomic<float>* macroYParam_{nullptr};
    std::atomic<float>* macroZParam_{nullptr};
    std::atomic<float>* macroTParam_{nullptr};
    std::atomic<float>* velAggrParam_{nullptr};
    std::atomic<float>* velTimbParam_{nullptr};
    std::atomic<float>* panParam_{nullptr};
    std::atomic<float>* decayCurveParam_{nullptr};

    std::atomic<float>* paramAParam_{nullptr};
    std::atomic<float>* paramBParam_{nullptr};
    std::atomic<float>* paramCParam_{nullptr};
    std::atomic<float>* paramDParam_{nullptr};
    std::atomic<float>* paramEParam_{nullptr};
    std::atomic<float>* paramFParam_{nullptr};
    std::atomic<float>* paramGParam_{nullptr};
    std::atomic<float>* paramHParam_{nullptr};

    std::atomic<int> lastMidiNote_{60};
    std::atomic<float> lastMidiVelocity_{0.95f};
    std::atomic<int> manualTriggerNote_{-1};
    std::atomic<float> manualTriggerVel_{-1.0f};
    std::atomic<bool> manualTriggerPending_{false};
    std::array<std::atomic<float>, 8> latchedDraw_{};

    std::atomic<float>* randXParam_{nullptr};
    std::atomic<float>* randYParam_{nullptr};
    std::atomic<float>* randZParam_{nullptr};
    std::atomic<float>* randTParam_{nullptr};
    std::atomic<float>* randAggrParam_{nullptr};
    std::atomic<float>* randTimbParam_{nullptr};
    std::atomic<float>* randPanParam_{nullptr};
    std::atomic<float>* randDecayParam_{nullptr};
    std::atomic<float>* macroDecayParam_{nullptr};
    std::atomic<float>* volumeParam_{nullptr};
    std::atomic<float>* ampDecayParam_{nullptr};

    std::atomic<float>* cutoffParam_{nullptr};
    std::atomic<float>* resonanceParam_{nullptr};
    std::atomic<float>* filterEnvAmtParam_{nullptr};
    std::atomic<float>* filterDecayParam_{nullptr};
    std::atomic<float>* filterTypeParam_{nullptr};
    std::atomic<float>* preDriveParam_{nullptr};
    std::atomic<float>* crushParam_{nullptr};
    std::atomic<float>* shiftParam_{nullptr};
    std::atomic<float> limiterGR_{1.0f};
    std::atomic<float>* squashParam_{nullptr};
    std::atomic<float>* widerParam_{nullptr};
    std::atomic<float>* masterLimiterParam_{nullptr};

    std::atomic<float>* roleParam_{nullptr};
    std::atomic<float>* modelParam_{nullptr};
    std::atomic<float>* lifeModeParam_{nullptr};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FormlessSpawnAudioProcessor)
};