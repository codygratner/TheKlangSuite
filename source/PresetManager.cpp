#include "PresetManager.h"
#include <algorithm>
#include <cmath>

namespace RlyehSound {

PresetManager::PresetManager(const juce::String& pluginName)
    : targetPlugin(pluginName)
{
    getUserPresetsDirectory().createDirectory();
}

void PresetManager::setTargetPlugin(const juce::String& pluginName) {
    targetPlugin = pluginName;
    getUserPresetsDirectory().createDirectory();
    refreshPresetList();
}

juce::File PresetManager::getUserPresetsDirectory() const {
    juce::String folderName = targetPlugin;
    if (folderName.equalsIgnoreCase("TKF")) folderName = "TheKlangFarmer";

    return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("Cracked Chimes")
        .getChildFile(folderName)
        .getChildFile("Presets");
}

juce::File PresetManager::getFactoryPresetsDirectory() const {
    const juce::String sub = targetPlugin.toLowerCase();

    // 1. Check relative to VST3 bundle application file (Contents/Resources/presets/<sub or targetPlugin>)
    auto appBundle = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
    if (appBundle.isDirectory()) {
        auto bundlePresets = appBundle.getChildFile("Contents/Resources/presets").getChildFile(sub);
        if (bundlePresets.isDirectory()) return bundlePresets;
        auto bundlePresetsDirect = appBundle.getChildFile("Resources/presets").getChildFile(sub);
        if (bundlePresetsDirect.isDirectory()) return bundlePresetsDirect;
    }

    // 2. Check system VST3 directory (Common Files/VST3/<Plugin>.vst3/Contents/Resources/presets/<sub or targetPlugin>)
    auto commonFilesStr = juce::SystemStats::getEnvironmentVariable("CommonProgramFiles", "C:\\Program Files\\Common Files");
    auto commonFiles = juce::File(commonFilesStr);
    if (commonFiles.isDirectory()) {
        auto systemVst3 = commonFiles.getChildFile("VST3").getChildFile(targetPlugin + ".vst3")
            .getChildFile("Contents/Resources/presets").getChildFile(sub);
        if (systemVst3.isDirectory()) return systemVst3;
    }

    // 3. Check source tree relative paths
    auto devTree = juce::File("C:/Dev/TheKlangSuite/assets/presets").getChildFile(sub);
    if (devTree.isDirectory()) return devTree;

    // 4. Check relative to current executable
    auto exeDir = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory();
    auto exePluginPresets = exeDir.getChildFile("assets/presets").getChildFile(sub);
    if (exePluginPresets.isDirectory()) return exePluginPresets;

    auto p1 = exeDir.getParentDirectory().getParentDirectory().getChildFile("assets/presets").getChildFile(sub);
    if (p1.isDirectory()) return p1;

    // 5. Fallback to current working directory
    auto cwdPluginPresets = juce::File::getCurrentWorkingDirectory()
        .getChildFile("assets/presets").getChildFile(sub);
    if (cwdPluginPresets.isDirectory()) return cwdPluginPresets;

    // 6. Tier-4 host-safe fallback: user Documents/Cracked Chimes/<Plugin>/Presets
    return getUserPresetsDirectory();
}

bool PresetManager::applyParameterValue(juce::AudioProcessorValueTreeState& apvts,
                                        const juce::String& paramId, float engineeringValue) {
    auto* param = apvts.getParameter(paramId);
    if (param == nullptr) return false;   // unknown key -> ignored
    float norm = std::clamp(engineeringValue, 0.0f, 1.0f);
    if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(param))
        norm = ranged->convertTo0to1(engineeringValue);
    param->beginChangeGesture();
    param->setValueNotifyingHost(norm);
    param->endChangeGesture();
    return true;
}

void PresetManager::loadInit(juce::AudioProcessorValueTreeState& apvts) {
    for (auto* p : apvts.processor.getParameters()) {
        if (p == nullptr) continue;
        p->beginChangeGesture();
        p->setValueNotifyingHost(p->getDefaultValue());
        p->endChangeGesture();
    }
    currentIndex = -1;
    setActivePresetName("Init");
    if (onPresetLoaded) onPresetLoaded(activePresetName);
}

juce::Array<juce::File> PresetManager::getAvailablePresetFiles() const {
    juce::Array<juce::File> results;

    auto scanDir = [&](const juce::File& dir) {
        if (!dir.isDirectory()) return;
        juce::Array<juce::File> files;
        // Recursive scan to ingest themed bank subdirectories
        dir.findChildFiles(files, juce::File::findFiles, true, "*.json;*.tkp;*.tkpreset");
        for (const auto& f : files) {
            results.add(f);
        }
    };

    const auto factoryDir = getFactoryPresetsDirectory();
    const auto userDir = getUserPresetsDirectory();
    if (factoryDir != userDir) scanDir(factoryDir);   // tier-4 fallback may equal user dir
    scanDir(userDir);

    // Also check ToadTracker presets directory if applicable
    if (targetPlugin.equalsIgnoreCase("ToadTracker") || targetPlugin.equalsIgnoreCase("TKF")) {
        juce::File ttPresets("C:/Dev/ToadTracker/presets");
        if (ttPresets.isDirectory()) {
            scanDir(ttPresets);
        }
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
            info.targetPlugin = parsed.getProperty("target_plugin", targetPlugin).toString();

            // Default Bank / Category to immediate parent folder name
            juce::String parentName = f.getParentDirectory().getFileName();
            if (parentName.equalsIgnoreCase("Presets") || parentName.equalsIgnoreCase(targetPlugin)) {
                info.bank = "General";
                info.category = "General";
            } else {
                info.bank = parentName;
                info.category = parentName;
            }

            if (parsed.hasProperty("metadata") && parsed["metadata"].isObject()) {
                auto meta = parsed["metadata"];
                info.name = meta.getProperty("name", f.getFileNameWithoutExtension()).toString();
                info.author = meta.getProperty("author", "Cody Gratner").toString();
                info.category = meta.getProperty("category", info.category).toString();
                if (meta.hasProperty("tags") && meta["tags"].isArray()) {
                    if (auto* arr = meta["tags"].getArray()) {
                        for (const auto& t : *arr) info.tags.add(t.toString());
                    }
                }
            } else {
                info.name = parsed.getProperty("name", f.getFileNameWithoutExtension()).toString();
                info.author = parsed.getProperty("author", "Cracked Chimes").toString();
                info.bank = parsed.getProperty("bank", info.bank).toString();
                info.category = parsed.getProperty("category", info.category).toString();
                if (parsed.hasProperty("tags") && parsed["tags"].isArray()) {
                    if (auto* arr = parsed["tags"].getArray()) {
                        for (const auto& t : *arr) info.tags.add(t.toString());
                    }
                }
            }
            list.add(info);
        }
    }

    cachedPresets_ = list;
    return list;
}

void PresetManager::refreshPresetList() {
    getAvailablePresets();
}

int PresetManager::getNumPresets() const {
    if (cachedPresets_.isEmpty()) {
        getAvailablePresets();
    }
    return cachedPresets_.size();
}

bool PresetManager::loadPresetByIndex(int index, juce::AudioProcessorValueTreeState& apvts) {
    if (cachedPresets_.isEmpty()) {
        getAvailablePresets();
    }
    if (index < 0 || index >= cachedPresets_.size()) return false;

    currentIndex = index;
    return loadPreset(cachedPresets_[index].file, apvts);
}

bool PresetManager::nextPreset(juce::AudioProcessorValueTreeState& apvts) {
    if (cachedPresets_.isEmpty()) {
        getAvailablePresets();
    }
    if (cachedPresets_.isEmpty()) return false;

    int nextIdx = (currentIndex + 1) % cachedPresets_.size();
    return loadPresetByIndex(nextIdx, apvts);
}

bool PresetManager::prevPreset(juce::AudioProcessorValueTreeState& apvts) {
    if (cachedPresets_.isEmpty()) {
        getAvailablePresets();
    }
    if (cachedPresets_.isEmpty()) return false;

    int prevIdx = (currentIndex - 1 + cachedPresets_.size()) % cachedPresets_.size();
    return loadPresetByIndex(prevIdx, apvts);
}

bool PresetManager::loadPresetFromJson(const juce::String& jsonContent, juce::AudioProcessorValueTreeState& apvts) {
    auto parsed = juce::JSON::parse(jsonContent);
    if (!parsed.isObject()) return false;

    // 1. Direct APVTS Parameter Ingestion (Clean Modern Schema)
    if (parsed.hasProperty("parameters") && parsed["parameters"].isObject()) {
        if (auto* pObj = parsed["parameters"].getDynamicObject()) {
            for (const auto& prop : pObj->getProperties()) {
                // String-ID only; unknown keys ignored, missing params keep current values.
                applyParameterValue(apvts, prop.name.toString(), static_cast<float>(prop.value));
            }
        }
    }

    // 2. Custom Engine State Hook (e.g. Shoggoth's 126 Harmonics + DHG Envelopes)
    if (onCustomStateLoad && parsed.hasProperty("custom_state")) {
        onCustomStateLoad(parsed["custom_state"]);
    }

    // 3. Metadata tracking
    juce::String pName;
    if (parsed.hasProperty("metadata") && parsed["metadata"].isObject()) {
        pName = parsed["metadata"].getProperty("name", "").toString();
    } else {
        pName = parsed.getProperty("name", "").toString();
    }
    if (pName.isNotEmpty()) {
        setActivePresetName(pName);
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
    rootObj->setProperty("schema_version", 1);
    rootObj->setProperty("target_plugin", targetPlugin);
    rootObj->setProperty("plugin_id", targetPlugin);

    auto metaObj = std::make_unique<juce::DynamicObject>();
    juce::String pName = name.isNotEmpty() ? name : file.getFileNameWithoutExtension();
    metaObj->setProperty("name", pName);
    metaObj->setProperty("author", author.isNotEmpty() ? author : "Cracked Chimes");
    metaObj->setProperty("category", category.isNotEmpty() ? category : "General");

    juce::Array<juce::var> tagArray;
    if (tags.isEmpty()) {
        tagArray.add("User");
        tagArray.add(targetPlugin.toLowerCase());
    } else {
        for (const auto& t : tags) tagArray.add(t);
    }
    metaObj->setProperty("tags", tagArray);
    rootObj->setProperty("metadata", juce::var(metaObj.release()));
    rootObj->setProperty("name", pName);

    // APVTS Parameters (stored in engineering units)
    auto paramsObj = std::make_unique<juce::DynamicObject>();
    for (auto* p : apvts.processor.getParameters()) {
        if (auto* paramWithID = dynamic_cast<juce::AudioProcessorParameterWithID*>(p)) {
            float val = paramWithID->getValue();
            if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(p)) {
                val = ranged->convertFrom0to1(val);
            }
            val = std::round(val * 10000.0f) / 10000.0f;
            paramsObj->setProperty(paramWithID->paramID, static_cast<double>(val));
        }
    }
    rootObj->setProperty("parameters", juce::var(paramsObj.release()));

    // Polymorphic Custom State
    if (onCustomStateSave) {
        rootObj->setProperty("custom_state", onCustomStateSave());
    }

    juce::var rootVar(rootObj.release());
    juce::String jsonStr = juce::JSON::toString(rootVar, true);

    file.getParentDirectory().createDirectory();
    if (!file.replaceWithText(jsonStr)) return false;

    setActivePresetName(pName);
    refreshPresetList();
    return true;
}

} // namespace RlyehSound
