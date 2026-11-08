#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <unordered_map>
#include <vector>
#include <functional>
#include <atomic>
#include <algorithm>

namespace RlyehSound {

struct PresetMetadataInfo {
    juce::String name;
    juce::String author;
    juce::String category;
    juce::String bank;
    juce::StringArray tags;
    juce::String targetPlugin;
    juce::File file;
};

// Section Locks Bitmask
enum SectionLockFlags : uint32_t {
    LockNone      = 0,
    LockOscCore   = 1 << 0,
    LockFilterVCF = 1 << 1,
    LockEnvelopes = 1 << 2,
    LockMasterFX  = 1 << 3,
    LockModMatrix = 1 << 4
};

// ==============================================================================
// DuckGate: Lock-Free Audio-Thread Sleep-Bypass Watchdog
// ==============================================================================
struct DuckGate {
    std::atomic<uint64_t> processBlockCounter{0};
    std::atomic<bool> isDucking{false};
    std::atomic<bool> isSilent{false};
    std::atomic<float> duckGain{1.0f};

    void tickAudio(float* const* channelData, int numChannels, int numSamples, bool isSleeping) noexcept {
        processBlockCounter.fetch_add(1, std::memory_order_relaxed);
        if (isSleeping) {
            duckGain.store(1.0f, std::memory_order_relaxed);
            isSilent.store(true, std::memory_order_release);
            return;
        }

        if (isDucking.load(std::memory_order_acquire)) {
            float currentGain = duckGain.load(std::memory_order_relaxed);
            constexpr float kRampStep = 1.0f / (0.008f * 44100.0f);
            for (int s = 0; s < numSamples; ++s) {
                currentGain = std::max(0.0f, currentGain - kRampStep);
                for (int ch = 0; ch < numChannels; ++ch) {
                    if (channelData[ch]) channelData[ch][s] *= currentGain;
                }
            }
            duckGain.store(currentGain, std::memory_order_relaxed);
            if (currentGain <= 0.001f) {
                isSilent.store(true, std::memory_order_release);
            }
        } else {
            float currentGain = duckGain.load(std::memory_order_relaxed);
            if (currentGain < 1.0f) {
                constexpr float kRampStep = 1.0f / (0.008f * 44100.0f);
                for (int s = 0; s < numSamples; ++s) {
                    currentGain = std::min(1.0f, currentGain + kRampStep);
                    for (int ch = 0; ch < numChannels; ++ch) {
                        if (channelData[ch]) channelData[ch][s] *= currentGain;
                    }
                }
                duckGain.store(currentGain, std::memory_order_relaxed);
            }
            isSilent.store(false, std::memory_order_release);
        }
    }
};

class PresetManager {
public:
    explicit PresetManager(const juce::String& pluginName = "Hastur");
    ~PresetManager() = default;

    void setTargetPlugin(const juce::String& pluginName);
    juce::String getTargetPlugin() const { return targetPlugin; }

    // Preset loading
    bool loadPreset(const juce::File& file, juce::AudioProcessorValueTreeState& apvts);
    bool loadPresetFromJson(const juce::String& jsonContent, juce::AudioProcessorValueTreeState& apvts);

    // Preset saving
    bool savePreset(const juce::File& file,
                    const juce::AudioProcessorValueTreeState& apvts,
                    const juce::String& name,
                    const juce::String& author = "Cody Gratner",
                    const juce::String& category = "General",
                    const juce::StringArray& tags = {});

    // Preset location queries
    juce::File getUserPresetsDirectory() const;
    juce::File getFactoryPresetsDirectory() const;
    juce::Array<juce::File> getAvailablePresetFiles() const;
    juce::Array<PresetMetadataInfo> getAvailablePresets() const;

    // Linear navigation & fast switching
    void refreshPresetList();
    int getNumPresets() const;
    int getCurrentIndex() const { return currentIndex; }
    bool loadPresetByIndex(int index, juce::AudioProcessorValueTreeState& apvts);
    bool nextPreset(juce::AudioProcessorValueTreeState& apvts);
    bool prevPreset(juce::AudioProcessorValueTreeState& apvts);

    // 'Init' entry: resets every non-locked parameter to its default (gesture-wrapped).
    void loadInit(juce::AudioProcessorValueTreeState& apvts);

    // Applies one value (engineering units) by string ID, gesture-wrapped.
    static bool applyParameterValue(juce::AudioProcessorValueTreeState& apvts,
                                    const juce::String& paramId, float engineeringValue);

    // Active preset tracking
    juce::String getActivePresetName() const { return activePresetName; }
    void setActivePresetName(const juce::String& name) { activePresetName = name; }

    // Section Locks
    uint32_t getSectionLocks() const noexcept { return sectionLocks_; }
    void setSectionLocks(uint32_t mask) noexcept { sectionLocks_ = mask; }
    void toggleSectionLock(uint32_t flag) noexcept { sectionLocks_ ^= flag; }
    bool isSectionLocked(uint32_t flag) const noexcept { return (sectionLocks_ & flag) != 0; }
    uint32_t classifyParameterSection(const juce::String& paramId) const;

    // Smart Dice & 16-Step Undo Ring
    void mutateSmartDice(juce::AudioProcessorValueTreeState& apvts, float intensity = 0.25f);
    bool canUndoDice() const noexcept { return !diceHistory_.empty(); }
    void undoDice(juce::AudioProcessorValueTreeState& apvts);
    void clearDiceHistory() noexcept { diceHistory_.clear(); }

    // DuckGate access
    DuckGate& getDuckGate() noexcept { return duckGate_; }

    // Parameter exclusion queries
    bool isLoadSkip(const juce::String& paramId) const;
    bool isDiceDeny(const juce::String& paramId) const;

    // Callbacks & Custom State Hooks
    std::function<void(const juce::String&)> onPresetLoaded;
    std::function<void(const juce::var&)> onCustomStateLoad;
    std::function<juce::var()> onCustomStateSave;

private:
    juce::String targetPlugin{"Hastur"};
    juce::String activePresetName{"Default"};
    mutable juce::Array<PresetMetadataInfo> cachedPresets_;
    int currentIndex{-1};

    uint32_t sectionLocks_{LockNone};
    DuckGate duckGate_;

    // 16-state undo history ring buffer for Smart Dice
    std::vector<std::unordered_map<juce::String, float>> diceHistory_;
    static constexpr size_t kMaxDiceHistory = 16;
};

} // namespace RlyehSound
