#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include "dsp/drums/MmeKick.h"
#include "dsp/drums/MmeSnare.h"
#include "dsp/drums/MmeClap.h"
#include "dsp/drums/HiHatEngine.h"
#include "dsp/fx/ElektronTrackWrapper.h"
#include "dsp/drums/FormlessSpawnEngine.h"
#include <atomic>
#include <array>

// ==============================================================================
// THE CRUCIBLE WORKBENCH - UNIVERSAL 8-SLOT / 128-PARAMETER TESTBED VST3
// ==============================================================================

enum class SlotEngineType : uint8_t {
    MME_KICK = 0,             // Flagship MME-BD 16-Model Synthesizer
    FORMLESS_SNARE = 1,
    FORMLESS_ZAP = 2,
    FORMLESS_WEST = 3,
    FORMLESS_OPEN_HAT = 4,
    FORMLESS_CLOSED_HAT = 5,
    FORMLESS_CLAP = 6,
    FORMLESS_CRASH = 7,
    DIGISTRIKE_BD = 8,
    DIGISTRIKE_SD = 9,
    DIGISTRIKE_HH = 10,
    DIGISTRIKE_CLP = 11,
    DIGISTRIKE_TOM = 12,
    DIGISTRIKE_PRC = 13,
    DIGISTRIKE_CYM = 14
};

inline const char* getSlotEngineName(SlotEngineType t) noexcept {
    switch (t) {
        case SlotEngineType::MME_KICK:           return "MME Kick / 808";
        case SlotEngineType::FORMLESS_SNARE:     return "Formless Snare";
        case SlotEngineType::FORMLESS_ZAP:       return "Formless Zap";
        case SlotEngineType::FORMLESS_WEST:      return "Formless West";
        case SlotEngineType::FORMLESS_OPEN_HAT:  return "Formless Open Hat";
        case SlotEngineType::FORMLESS_CLOSED_HAT:return "Formless Closed Hat";
        case SlotEngineType::FORMLESS_CLAP:      return "Formless Clap";
        case SlotEngineType::FORMLESS_CRASH:     return "Formless Crash";
        case SlotEngineType::DIGISTRIKE_BD:      return "DigiStrike BD";
        case SlotEngineType::DIGISTRIKE_SD:      return "DigiStrike SD";
        case SlotEngineType::DIGISTRIKE_HH:      return "DigiStrike HH";
        case SlotEngineType::DIGISTRIKE_CLP:     return "DigiStrike CLP";
        case SlotEngineType::DIGISTRIKE_TOM:     return "DigiStrike TOM";
        case SlotEngineType::DIGISTRIKE_PRC:     return "DigiStrike PRC";
        case SlotEngineType::DIGISTRIKE_CYM:     return "DigiStrike CYM";
        default: return "Unknown";
    }
}

struct SlotState {
    SlotEngineType engineType{SlotEngineType::MME_KICK};
    bool active{false};
    float env{0.0f};
    float decayCoeff{0.999f};
    float phase{0.0f};
    float phaseInc{0.0f};
    float phase2{0.0f};
    float phaseInc2{0.0f};
    float pitchCur{0.0f};
    float pitchDrop{0.0f};
    float filterState{0.0f};
    float burstTimer{0.0f};
    uint32_t noiseLfsr{0x13579BDFu};

    // Expressive MIDI Mapping & Multi-Zone Fields
    bool isSustained{false};
    float targetPitch{55.0f};
    float releaseCoeff{0.0f};
    float stereoSpread{0.5f};
    float burstDuration{0.035f};

    inline float getNoise() noexcept {
        noiseLfsr = (noiseLfsr >> 1) ^ ((noiseLfsr & 1u) ? 0xD0000001u : 0u);
        return static_cast<float>(static_cast<int32_t>(noiseLfsr)) * (1.0f / 2147483648.0f);
    }
};

class CrucibleWorkbenchAudioProcessor : public juce::AudioProcessor {
public:
    static constexpr int kNumSlots = 8;
    static constexpr int kParamsPerSlot = 16;
    static constexpr int kTotalSlotParams = kNumSlots * kParamsPerSlot; // 128 Parameters

    CrucibleWorkbenchAudioProcessor();
    ~CrucibleWorkbenchAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override { return "Crucible Workbench"; }

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

    void triggerSlot(int slotIndex, float velocity = 0.9f) noexcept;
    void triggerSub808(float velocity = 0.9f) noexcept;
    void triggerSnare(bool isBig, float velocity = 0.9f) noexcept;
    void triggerClap(bool isBig, float velocity = 0.9f) noexcept;
    void triggerHat(bool isOpen, float velocity = 0.9f) noexcept;

    // Multi-Zone Expressive MIDI Handlers
    void handleNoteOn(int note, float velocity) noexcept;
    void handleNoteOff(int note) noexcept;

    int findSlotForRole(SlotEngineType primary, SlotEngineType secondary, int fallback) const noexcept;
    int findKickSlot() const noexcept;
    int findSnareSlot() const noexcept;
    int findClapSlot() const noexcept;
    int findHatSlot() const noexcept;
    int findCymbalSlot() const noexcept;
    int findTomSlot() const noexcept;

    SlotEngineType getSlotEngine(int slot) const noexcept;
    void setSlotEngine(int slot, SlotEngineType type, bool immediate = true) noexcept;

    bool isSwapPending() const noexcept { return swapPending_.load(std::memory_order_relaxed); }
    bool lastSwapCommitted() const noexcept { return lastSwapCommitted_.load(std::memory_order_relaxed); }
    void clearCommitFlag() noexcept { lastSwapCommitted_.store(false, std::memory_order_relaxed); }

    int getQuantizeMode() const noexcept { return quantizeMode_.load(std::memory_order_relaxed); }
    void setQuantizeMode(int m) noexcept { quantizeMode_.store(m, std::memory_order_relaxed); }

    int getUdpPort() const noexcept { return udpPort_; }

    void commitStagedSlots() noexcept;

    tkdsp::MmeKick& getMmeKick() noexcept { return mmeKick_; }
    const char* getMmeKickModelName() const noexcept {
        uint8_t m = mmeKick_.activeModel();
        return tkdsp::MmeKick::getModelName(m);
    }
    const char* getSub808ModelName() const noexcept {
        uint8_t m = sub808Engine_.activeModel();
        return tkdsp::MmeKick::getModelName(m);
    }
    const char* getMmeSnareModelName() const noexcept {
        uint8_t m = mmeSnare_.activeModel();
        return tkdsp::MmeSnare::kModelNames[m < 16 ? m : 0];
    }
    const char* getMmeClapModelName() const noexcept {
        uint8_t m = mmeClap_.activeModel();
        return tkdsp::MmeClap::kModelNames[m < 16 ? m : 0];
    }

private:
    void processDigiStrikeSlot(int slot, SlotState& s, float& outL, float& outR, float invSampleRate) noexcept;

    juce::UndoManager undoManager_;
    juce::AudioProcessorValueTreeState apvts_;

    // 128 Raw Parameter Pointers
    std::array<std::array<std::atomic<float>*, kParamsPerSlot>, kNumSlots> cachedParams_{};
    std::atomic<float>* masterVolumeParam_{nullptr};
    std::atomic<float>* masterDriveParam_{nullptr};
    std::atomic<float>* masterLowBoostParam_{nullptr};

    // Multi-Zone Tuning & Macro Parameters
    std::atomic<float>* subOctParam_{nullptr};
    std::atomic<float>* tomOctParam_{nullptr};
    std::atomic<float>* macroVarParam_{nullptr};
    int heldSubNote_{-1};

    // DSP Engines
    tkdsp::MmeKick mmeKick_{};
    float currentKickPitchHz_{55.0f};

    // Dedicated 808 Sub-Bass Voice (Zone 1)
    tkdsp::MmeKick sub808Engine_{};
    float sub808TargetPitchHz_{55.0f};
    float sub808CurrentPitchHz_{55.0f};
    std::atomic<float>* sub808ModelParam_{nullptr};
    std::atomic<float>* sub808TuneParam_{nullptr};
    std::atomic<float>* sub808DecayParam_{nullptr};
    std::atomic<float>* sub808PunchParam_{nullptr};
    std::atomic<float>* sub808SweepParam_{nullptr};
    std::atomic<float>* sub808FoldParam_{nullptr};
    std::atomic<float>* sub808DriveParam_{nullptr};
    std::atomic<float>* sub808CutoffParam_{nullptr};
    std::atomic<float>* sub808GlideParam_{nullptr};

    // Snare & Clap Flagship Workstation (Col 1)
    tkdsp::MmeSnare mmeSnare_{};
    tkdsp::MmeClap mmeClap_{};
    tkdsp::ElektronTrackWrapper scMojo_{};
    std::atomic<float>* scSdModelParam_{nullptr};
    std::atomic<float>* scSdTuneParam_{nullptr};
    std::atomic<float>* scSdDecayParam_{nullptr};
    std::atomic<float>* scSdSnapParam_{nullptr};
    std::atomic<float>* scSdToneParam_{nullptr};
    std::atomic<float>* scSdWiresParam_{nullptr};
    std::atomic<float>* scSdBodyParam_{nullptr};
    std::atomic<float>* scSdVarParam_{nullptr};
    std::atomic<float>* scCpModelParam_{nullptr};
    std::atomic<float>* scCpToneParam_{nullptr};
    std::atomic<float>* scCpDecayParam_{nullptr};
    std::atomic<float>* scCpBurstParam_{nullptr};
    std::atomic<float>* scCpSpreadParam_{nullptr};
    std::atomic<float>* scCpVarParam_{nullptr};
    std::atomic<float>* scMojoFltfParam_{nullptr};
    std::atomic<float>* scMojoDistParam_{nullptr};

    // Hi-Hat Flagship Workstation (Col 2)
    tkdsp::HiHatEngine hiHatEngine_{};
    std::array<std::atomic<float>*, 16> hatParams_{};

    tkdsp::FormlessSpawnEngine formlessEngine_{};
    std::array<SlotState, kNumSlots> slotStates_{};

    // Double-Buffered Slot Configuration
    std::array<SlotEngineType, kNumSlots> activeEngines_{};
    std::array<SlotEngineType, kNumSlots> stagedEngines_{};

    std::atomic<bool> swapPending_{false};
    std::atomic<bool> lastSwapCommitted_{false};
    std::atomic<int> quantizeMode_{0}; // 0 = Loop Wrap, 1 = Bar, 2 = Immediate

    double sampleRate_{44100.0};
    double lastPpq_{0.0};
    int lastBar_{0};

    // Master DSP State
    float masterLowBoostState_{0.0f};

    // UDP IPC Thread
    class IPCThread : public juce::Thread {
    public:
        IPCThread(CrucibleWorkbenchAudioProcessor& owner, int port = 9123);
        ~IPCThread() override;
        void run() override;
    private:
        CrucibleWorkbenchAudioProcessor& owner_;
        int port_{9123};
        std::unique_ptr<juce::DatagramSocket> socket_;
    };

    int udpPort_{9123};
    std::unique_ptr<IPCThread> ipcThread_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CrucibleWorkbenchAudioProcessor)
};
