#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <unordered_map>
#include <vector>
#include <functional>

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

class PresetManager {
public:
    // NOTE: the old static getInstance() singleton (hard-coded to "Hastur") was
    // removed in CCUI Wave 2-P. Each processor owns PresetManager(pluginName).

    explicit PresetManager(const juce::String& pluginName = "TheKlangFarmer");
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

    // 'Init' entry: resets every parameter to its default (gesture-wrapped).
    void loadInit(juce::AudioProcessorValueTreeState& apvts);

    // Applies one value (engineering units) by string ID, gesture-wrapped.
    // Returns false (and does nothing) for unknown IDs.
    static bool applyParameterValue(juce::AudioProcessorValueTreeState& apvts,
                                    const juce::String& paramId, float engineeringValue);

    // Active preset tracking
    juce::String getActivePresetName() const { return activePresetName; }
    void setActivePresetName(const juce::String& name) { activePresetName = name; }

    // Callbacks & Custom State Hooks
    std::function<void(const juce::String&)> onPresetLoaded;
    std::function<void(const juce::var&)> onCustomStateLoad;
    std::function<juce::var()> onCustomStateSave;

private:
    juce::String targetPlugin{"TheKlangFarmer"};
    juce::String activePresetName{"Default"};
    mutable juce::Array<PresetMetadataInfo> cachedPresets_;
    int currentIndex{-1};
};

} // namespace RlyehSound
