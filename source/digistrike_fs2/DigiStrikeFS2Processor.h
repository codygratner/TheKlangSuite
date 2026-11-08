#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "../dsp/drums/DigiStrikeFS2Voice.h"
#include "../PresetManager.h"
#include "DigiStrikeFS2ControlInfo.h"
#include <atomic>
#include <array>

// ==============================================================================
// DigiStrikeFS2Processor.h — AudioProcessor for DigiStrike FS2
// ==============================================================================
// Part of The Klang Suite SOCEP Engine. Copyright (c) 2026 Klang Industries.
// Clean-Room IP Compliance: Zero commercial trademarks.
// ==============================================================================

class DigiStrikeFS2AudioProcessor : public juce::AudioProcessor {
public:
    DigiStrikeFS2AudioProcessor();
    ~DigiStrikeFS2AudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override { return "DigiStrike FS2"; }

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
    tkdsp::DigiStrikeFS2Voice& getVoice() { return voice_; }
    RlyehSound::PresetManager& getPresetManager() noexcept { return presetManager_; }
    [[nodiscard]] bool isSleeping() const noexcept { return false; }

    std::atomic<float> storedEditorScale_{0.0f};
    void setStoredEditorScale(float s) noexcept { storedEditorScale_.store(s, std::memory_order_relaxed); }
    [[nodiscard]] float getStoredEditorScale() const noexcept { return storedEditorScale_.load(std::memory_order_relaxed); }

    struct TelemetrySnapshot {
        std::array<float, 8> randVal{{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};
        float lastVelocity{0.9f};
        int noteNumber{60};
    };

    TelemetrySnapshot getTelemetrySnapshot() const noexcept {
        const int idx = activeTelemetryIdx_.load(std::memory_order_acquire);
        return telemetryBuffers_[static_cast<size_t>(idx)];
    }

    void triggerManual(int note = -1, float velocity = -1.0f) noexcept {
        manualTriggerNote_.store(note, std::memory_order_release);
        manualTriggerVel_.store(velocity, std::memory_order_release);
        manualTriggerPending_.store(true, std::memory_order_release);
    }

    void triggerVoiceRelease(int note = -1) noexcept {
        manualReleaseNote_.store(note >= 0 ? note : -2, std::memory_order_release);
    }

    int getLastMidiNote() const noexcept {
        return lastMidiNote_.load(std::memory_order_relaxed);
    }

    float getLastMidiVelocity() const noexcept {
        return lastMidiVelocity_.load(std::memory_order_relaxed);
    }

    float exchangeLimiterGrMin(float resetVal = 1.0f) noexcept {
        return limiterGrMin_.exchange(resetVal, std::memory_order_relaxed);
    }

    void triggerVoice(int noteNumber, float velocity, float noteHz) noexcept;

private:
    void loadVoiceParams(tkdsp::DigiStrikeFS2Params& p) const noexcept;
    void publishTelemetry(float velocity, int noteNumber = 60) noexcept;

    std::array<TelemetrySnapshot, 3> telemetryBuffers_{};
    std::atomic<int> activeTelemetryIdx_{0};

    tkdsp::DigiStrikeFS2Voice voice_;
    juce::UndoManager undoManager_;
    juce::AudioProcessorValueTreeState apvts_;

    // Cached raw parameter pointers for lock-free audio thread access
    std::atomic<float>* categoryParam_{nullptr};
    std::atomic<float>* lineageParam_{nullptr};
    std::atomic<float>* voiceModeParam_{nullptr};
    std::atomic<float>* lifeModeParam_{nullptr};

    std::atomic<float>* macroXParam_{nullptr};
    std::atomic<float>* macroYParam_{nullptr};
    std::atomic<float>* macroZParam_{nullptr};
    std::atomic<float>* macroTParam_{nullptr};
    std::atomic<float>* velAggrParam_{nullptr};
    std::atomic<float>* velTimbParam_{nullptr};
    std::atomic<float>* panParam_{nullptr};
    std::atomic<float>* decayCurveParam_{nullptr};

    std::atomic<float>* pitchDecayParam_{nullptr};
    std::atomic<float>* clickBleedParam_{nullptr};
    std::atomic<float>* startOffsetParam_{nullptr};
    std::atomic<float>* traverseRateParam_{nullptr};
    std::atomic<float>* pitchDiveParam_{nullptr};
    std::atomic<float>* klangMorphParam_{nullptr};
    std::atomic<float>* sampleLengthParam_{nullptr};
    std::atomic<float>* tableMorphParam_{nullptr};
    std::atomic<float>* loopModeParam_{nullptr};
    std::atomic<float>* reverseParam_{nullptr};

    std::array<std::atomic<float>*, 20> linParams_{};

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
    std::atomic<float>* sustainLevelParam_{nullptr};
    std::atomic<float>* ampDecayParam_{nullptr};
    std::atomic<float>* glideTimeParam_{nullptr};
    std::atomic<float>* ampVelocityParam_{nullptr};

    std::atomic<float>* cutoffParam_{nullptr};
    std::atomic<float>* resonanceParam_{nullptr};
    std::atomic<float>* filterEnvAmtParam_{nullptr};
    std::atomic<float>* filterDecayParam_{nullptr};
    std::atomic<float>* filterTypeParam_{nullptr};
    std::atomic<float>* preDriveParam_{nullptr};
    std::atomic<float>* crushParam_{nullptr};
    std::atomic<float>* shiftParam_{nullptr};
    std::atomic<float>* squashParam_{nullptr};
    std::atomic<float>* widerParam_{nullptr};
    std::atomic<float>* masterLimiterParam_{nullptr};

    std::atomic<int> lastMidiNote_{60};
    std::atomic<float> lastMidiVelocity_{100.0f / 127.0f};
    std::atomic<int> manualTriggerNote_{-1};
    std::atomic<float> manualTriggerVel_{0.0f};
    std::atomic<bool> manualTriggerPending_{false};
    std::atomic<int> manualReleaseNote_{-1};

    bool telemetryDirty_{false};
    float pendingTelemetryVel_{100.0f / 127.0f};
    int pendingTelemetryNote_{60};

    std::atomic<float> limiterGrMin_{1.0f};
    RlyehSound::PresetManager presetManager_{"DigiStrike FS2"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DigiStrikeFS2AudioProcessor)
};
