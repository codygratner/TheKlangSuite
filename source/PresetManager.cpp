#include "PresetManager.h"
#include <cmath>

namespace RlyehSound {

PresetManager::PresetManager() {
    getUserPresetsDirectory().createDirectory();
}

juce::File PresetManager::getUserPresetsDirectory() const {
    return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("TbdAudio")
        .getChildFile("TheKlangFarmer")
        .getChildFile("Presets");
}

juce::File PresetManager::getFactoryPresetsDirectory() const {
    // 1. Check relative to current working directory
    auto cwdPresets = juce::File::getCurrentWorkingDirectory().getChildFile("assets/presets");
    if (cwdPresets.isDirectory()) return cwdPresets;

    // 2. Check relative to executable
    auto exePresets = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
        .getParentDirectory().getChildFile("assets/presets");
    if (exePresets.isDirectory()) return exePresets;

    // 3. Fallback: parent dirs
    auto p1 = juce::File::getCurrentWorkingDirectory().getParentDirectory().getChildFile("assets/presets");
    if (p1.isDirectory()) return p1;

    return cwdPresets;
}

juce::Array<juce::File> PresetManager::getAvailablePresetFiles() const {
    juce::Array<juce::File> results;

    auto scanDir = [&](const juce::File& dir) {
        if (!dir.isDirectory()) return;
        juce::Array<juce::File> files;
        dir.findChildFiles(files, juce::File::findFiles, false, "*.json;*.tkp");
        for (const auto& f : files) {
            results.add(f);
        }
    };

    scanDir(getFactoryPresetsDirectory());
    scanDir(getUserPresetsDirectory());

    // Also check ToadTracker presets directory if present
    juce::File ttPresets("C:/Dev/ToadTracker/presets");
    if (ttPresets.isDirectory()) {
        scanDir(ttPresets);
    }

    return results;
}

juce::Array<PresetMetadataInfo> PresetManager::getAvailablePresets() const {
    juce::Array<PresetMetadataInfo> list;
    auto files = getAvailablePresetFiles();

    for (const auto& f : files) {
        auto jsonStr = f.loadFileAsString();
        auto parsed = juce::JSON::parse(jsonStr);
        if (parsed.isObject()) {
            PresetMetadataInfo info;
            info.file = f;
            info.targetPlugin = parsed.getProperty("target_plugin", "TKF").toString();

            if (parsed.hasProperty("metadata") && parsed["metadata"].isObject()) {
                auto meta = parsed["metadata"];
                info.name = meta.getProperty("name", f.getFileNameWithoutExtension()).toString();
                info.author = meta.getProperty("author", "Cody Gratner").toString();
                info.category = meta.getProperty("category", "General").toString();
                if (meta.hasProperty("tags") && meta["tags"].isArray()) {
                    if (auto* arr = meta["tags"].getArray()) {
                        for (const auto& t : *arr) info.tags.add(t.toString());
                    }
                }
            } else {
                info.name = f.getFileNameWithoutExtension();
                info.category = "General";
            }
            list.add(info);
        }
    }
    return list;
}

bool PresetManager::loadPresetFromJson(const juce::String& jsonContent, juce::AudioProcessorValueTreeState& apvts) {
    auto parsed = juce::JSON::parse(jsonContent);
    if (!parsed.isObject()) return false;

    std::unordered_map<juce::String, float> migratedParams;
    StateMigrator::migrateToTKF(parsed, migratedParams);

    // Apply parameters into APVTS
    for (const auto& [paramId, val] : migratedParams) {
        if (auto* param = apvts.getParameter(paramId)) {
            param->setValueNotifyingHost(std::clamp(val, 0.0f, 1.0f));
        }
    }

    if (parsed.hasProperty("metadata") && parsed["metadata"].isObject()) {
        auto name = parsed["metadata"].getProperty("name", "").toString();
        if (name.isNotEmpty()) {
            setActivePresetName(name);
        }
    }

    if (onPresetLoaded) {
        onPresetLoaded(activePresetName);
    }

    return true;
}

bool PresetManager::loadPreset(const juce::File& file, juce::AudioProcessorValueTreeState& apvts) {
    if (!file.existsAsFile()) return false;
    juce::String content = file.loadFileAsString();
    if (content.isEmpty()) return false;

    bool ok = loadPresetFromJson(content, apvts);
    if (ok) {
        setActivePresetName(file.getFileNameWithoutExtension());
    }
    return ok;
}

bool PresetManager::savePreset(const juce::File& file,
                               const juce::AudioProcessorValueTreeState& apvts,
                               const juce::String& name,
                               const juce::String& author,
                               const juce::String& category,
                               const juce::StringArray& tags) {
    auto rootObj = std::make_unique<juce::DynamicObject>();
    rootObj->setProperty("schema_version", "1.0.0");
    rootObj->setProperty("target_plugin", "TKF");

    auto metaObj = std::make_unique<juce::DynamicObject>();
    juce::String pName = name.isNotEmpty() ? name : file.getFileNameWithoutExtension();
    metaObj->setProperty("name", pName);
    metaObj->setProperty("author", author.isNotEmpty() ? author : "Cody Gratner");
    metaObj->setProperty("category", category.isNotEmpty() ? category : "Bass");

    juce::Array<juce::var> tagArray;
    if (tags.isEmpty()) {
        tagArray.add("User");
        tagArray.add("tkf");
    } else {
        for (const auto& t : tags) tagArray.add(t);
    }
    metaObj->setProperty("tags", tagArray);
    metaObj->setProperty("theme_hint", "dark_terminal");
    rootObj->setProperty("metadata", juce::var(metaObj.release()));

    auto paramsObj = std::make_unique<juce::DynamicObject>();
    for (auto* p : apvts.processor.getParameters()) {
        if (auto* paramWithID = dynamic_cast<juce::AudioProcessorParameterWithID*>(p)) {
            float val = paramWithID->getValue();
            val = std::round(val * 10000.0f) / 10000.0f;
            paramsObj->setProperty(paramWithID->paramID, static_cast<double>(val));
        }
    }
    rootObj->setProperty("parameters", juce::var(paramsObj.release()));

    juce::var rootVar(rootObj.release());
    juce::String jsonStr = juce::JSON::toString(rootVar);

    file.getParentDirectory().createDirectory();
    if (!file.replaceWithText(jsonStr)) return false;

    setActivePresetName(pName);
    return true;
}

} // namespace RlyehSound
