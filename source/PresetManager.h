#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <unordered_map>
#include <vector>
#include "StateMigrator.h"

namespace RlyehSound {

struct PresetMetadataInfo {
    juce::String name;
    juce::String author;
    juce::String category;
    juce::StringArray tags;
    juce::String targetPlugin;
    juce::File file;
};

class PresetManager {
public:
    static PresetManager& getInstance() {
        static PresetManager instance;
        return instance;
    }

    // Preset loading
    bool loadPreset(const juce::File& file, juce::AudioProcessorValueTreeState& apvts);
    bool loadPresetFromJson(const juce::String& jsonContent, juce::AudioProcessorValueTreeState& apvts);

    // Preset saving
    bool savePreset(const juce::File& file,
                    const juce::AudioProcessorValueTreeState& apvts,
                    const juce::String& name,
                    const juce::String& author = "Cody Gratner",
                    const juce::String& category = "Bass",
                    const juce::StringArray& tags = {});

    // Preset location queries
    juce::File getUserPresetsDirectory() const;
    juce::File getFactoryPresetsDirectory() const;
    juce::Array<juce::File> getAvailablePresetFiles() const;
    juce::Array<PresetMetadataInfo> getAvailablePresets() const;

    // Active preset tracking
    juce::String getActivePresetName() const { return activePresetName; }
    void setActivePresetName(const juce::String& name) { activePresetName = name; }

    std::function<void(const juce::String&)> onPresetLoaded;

private:
    PresetManager();
    ~PresetManager() = default;

    PresetManager(const PresetManager&) = delete;
    PresetManager& operator=(const PresetManager&) = delete;

    juce::String activePresetName { "Default" };
};

} // namespace RlyehSound
