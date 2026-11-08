#include "PresetManager.h"
#include <algorithm>
#include <cmath>

namespace RlyehSound {

PresetManager::PresetManager(const juce::String& pluginName)
    : targetPlugin(pluginName)
{
    getUserPresetsDirectory().createDirectory();

    // Migrate from legacy Documents folder if AppData folder is empty
    auto userDir = getUserPresetsDirectory();
    if (userDir.isDirectory() && userDir.findChildFiles(juce::File::findFiles, false, "*.json").isEmpty()) {
        auto legacyDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                            .getChildFile("Cracked Chimes")
                            .getChildFile(pluginName)
                            .getChildFile("Presets");
        if (legacyDir.isDirectory()) {
            auto legacyFiles = legacyDir.findChildFiles(juce::File::findFiles, true, "*.json");
            for (const auto& lf : legacyFiles) {
                lf.copyFileTo(userDir.getChildFile(lf.getFileName()));
            }
        }
    }
}

void PresetManager::setTargetPlugin(const juce::String& pluginName) {
    targetPlugin = pluginName;
    getUserPresetsDirectory().createDirectory();
    refreshPresetList();
}

juce::File PresetManager::getUserPresetsDirectory() const {
    juce::String folderName = targetPlugin;
    if (folderName.equalsIgnoreCase("TKF")) folderName = "TheKlangFarmer";

    // Primary: AppData/CrackedChimes/<Plugin>/Presets
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("CrackedChimes")
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

    // 6. Tier-4 host-safe fallback: user presets directory
    return getUserPresetsDirectory();
}

bool PresetManager::applyParameterValue(juce::AudioProcessorValueTreeState& apvts,
                                        const juce::String& paramId, float engineeringValue) {
    auto* param = apvts.getParameter(paramId);
    if (param == nullptr) return false;
    float norm = std::clamp(engineeringValue, 0.0f, 1.0f);
    if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(param))
        norm = ranged->convertTo0to1(engineeringValue);
    param->beginChangeGesture();
    param->setValueNotifyingHost(norm);
    param->endChangeGesture();
    return true;
}

uint32_t PresetManager::classifyParameterSection(const juce::String& paramId) const {
    juce::String lower = paramId.toLowerCase();

    if (lower.contains("filter") || lower.contains("cutoff") || lower.contains("res") ||
        lower.contains("drive") || lower.contains("burble") || lower.contains("laser") ||
        lower.contains("vcf") || lower.contains("orbit") || lower.contains("deck") ||
        lower.contains("svf") || lower.contains("tilt") || lower.contains("tame")) {
        return LockFilterVCF;
    }

    if (lower.contains("env") || lower.contains("decay") || lower.contains("attack") ||
        lower.contains("sustain") || lower.contains("release") || lower.contains("snap") ||
        lower.contains("punch") || lower.contains("gate")) {
        return LockEnvelopes;
    }

    if (lower.contains("chorus") || lower.contains("width") || lower.contains("reverb") ||
        lower.contains("delay") || lower.contains("squash") || lower.contains("wider") ||
        lower.contains("space") || lower.contains("stomp") || lower.contains("output")) {
        return LockMasterFX;
    }

    if (lower.contains("lfo") || lower.contains("mod") || lower.contains("rand") ||
        lower.contains("glide") || lower.contains("vibrato")) {
        return LockModMatrix;
    }

    if (lower.contains("osc") || lower.contains("dco") || lower.contains("sub") ||
        lower.contains("pitch") || lower.contains("wave") || lower.contains("bite") ||
        lower.contains("spit") || lower.contains("chopper") || lower.contains("harmonic") ||
        lower.contains("additive") || lower.contains("sample") || lower.contains("drum") ||
        lower.contains("timbre") || lower.contains("slop")) {
        return LockOscCore;
    }

    return LockNone;
}

bool PresetManager::isLoadSkip(const juce::String& paramId) const {
    juce::String lower = paramId.toLowerCase();
    return (lower == "output_level" || lower == "master_volume" ||
            lower == "editor_scale" || lower == "ui_scale" ||
            lower == "masterspread");
}

bool PresetManager::isDiceDeny(const juce::String& paramId) const {
    if (isLoadSkip(paramId)) return true;
    juce::String lower = paramId.toLowerCase();
    return (lower.contains("life") || lower.contains("trigger") ||
            lower.contains("voice") || lower.contains("bend") ||
            lower.contains("sync"));
}

void PresetManager::loadInit(juce::AudioProcessorValueTreeState& apvts) {
    for (auto* p : apvts.processor.getParameters()) {
        if (p == nullptr) continue;
        if (auto* paramWithId = dynamic_cast<juce::AudioProcessorParameterWithID*>(p)) {
            if (isLoadSkip(paramWithId->paramID)) continue;
            if (isSectionLocked(classifyParameterSection(paramWithId->paramID))) continue;
        }
        p->beginChangeGesture();
        p->setValueNotifyingHost(p->getDefaultValue());
        p->endChangeGesture();
    }
    currentIndex = -1;
    setActivePresetName("Init");
    if (onPresetLoaded) onPresetLoaded(activePresetName);
}

void PresetManager::mutateSmartDice(juce::AudioProcessorValueTreeState& apvts, float intensity) {
    // 1. Snapshot current parameters for undo
    std::unordered_map<juce::String, float> snapshot;
    for (auto* p : apvts.processor.getParameters()) {
        if (auto* pWithId = dynamic_cast<juce::AudioProcessorParameterWithID*>(p)) {
            snapshot[pWithId->paramID] = p->getValue();
        }
    }
    if (diceHistory_.size() >= kMaxDiceHistory) {
        diceHistory_.erase(diceHistory_.begin());
    }
    diceHistory_.push_back(std::move(snapshot));

    // 2. Mutate non-denied, non-locked parameters
    auto& rng = juce::Random::getSystemRandom();
    intensity = std::clamp(intensity, 0.05f, 0.80f);

    for (auto* p : apvts.processor.getParameters()) {
        auto* pWithId = dynamic_cast<juce::AudioProcessorParameterWithID*>(p);
        if (pWithId == nullptr) continue;

        const auto& id = pWithId->paramID;
        if (isDiceDeny(id)) continue;
        if (isSectionLocked(classifyParameterSection(id))) continue;

        float currentVal = p->getValue();
        float delta = (rng.nextFloat() * 2.0f - 1.0f) * intensity;
        float newVal = currentVal + delta;

        // Reflection barrier at [0.0, 1.0]
        if (newVal < 0.0f) newVal = -newVal;
        if (newVal > 1.0f) newVal = 2.0f - newVal;
        newVal = std::clamp(newVal, 0.0f, 1.0f);

        p->beginChangeGesture();
        p->setValueNotifyingHost(newVal);
        p->endChangeGesture();
    }

    setActivePresetName("Smart Dice");
    if (onPresetLoaded) onPresetLoaded(activePresetName);
}

void PresetManager::undoDice(juce::AudioProcessorValueTreeState& apvts) {
    if (diceHistory_.empty()) return;

    auto lastState = diceHistory_.back();
    diceHistory_.pop_back();

    for (const auto& [paramId, val] : lastState) {
        if (auto* param = apvts.getParameter(paramId)) {
            param->beginChangeGesture();
            param->setValueNotifyingHost(val);
            param->endChangeGesture();
        }
    }

    setActivePresetName("Undo Dice");
    if (onPresetLoaded) onPresetLoaded(activePresetName);
}

juce::Array<juce::File> PresetManager::getAvailablePresetFiles() const {
    juce::Array<juce::File> results;

    auto scanDir = [&](const juce::File& dir) {
        if (!dir.isDirectory()) return;
        auto files = dir.findChildFiles(juce::File::findFiles, true, "*.json");
        for (const auto& f : files) {
            results.add(f);
        }
    };

    scanDir(getFactoryPresetsDirectory());
    scanDir(getUserPresetsDirectory());

    return results;
}

juce::Array<PresetMetadataInfo> PresetManager::getAvailablePresets() const {
    juce::Array<PresetMetadataInfo> list;
    auto files = getAvailablePresetFiles();

    for (const auto& f : files) {
        auto parsed = juce::JSON::parse(f);
        if (parsed.isObject()) {
            PresetMetadataInfo info;
            info.file = f;
            info.name = f.getFileNameWithoutExtension();
            info.category = "General";
            info.bank = f.getParentDirectory().getFileName();
            if (info.bank == targetPlugin.toLowerCase() || info.bank == "Presets") {
                info.bank = "Factory";
            }

            if (parsed.hasProperty("metadata") && parsed["metadata"].isObject()) {
                auto meta = parsed["metadata"];
                info.name = meta.getProperty("name", info.name).toString();
                info.author = meta.getProperty("author", "Cracked Chimes").toString();
                info.category = meta.getProperty("category", info.category).toString();
                info.bank = meta.getProperty("bank", info.bank).toString();
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

    // 1. Direct APVTS Parameter Ingestion (respecting section locks & LoadSkip)
    if (parsed.hasProperty("parameters") && parsed["parameters"].isObject()) {
        if (auto* pObj = parsed["parameters"].getDynamicObject()) {
            for (const auto& prop : pObj->getProperties()) {
                juce::String paramId = prop.name.toString();
                if (isLoadSkip(paramId)) continue;
                if (isSectionLocked(classifyParameterSection(paramId))) continue;

                applyParameterValue(apvts, paramId, static_cast<float>(prop.value));
            }
        }
    }

    // 2. Custom Engine State Hook
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
    bool res = loadPresetFromJson(content, apvts);
    if (res && activePresetName.isEmpty()) {
        setActivePresetName(file.getFileNameWithoutExtension());
    }
    return res;
}

bool PresetManager::savePreset(const juce::File& file,
                               const juce::AudioProcessorValueTreeState& apvts,
                               const juce::String& name,
                               const juce::String& author,
                               const juce::String& category,
                               const juce::StringArray& tags) {
    juce::DynamicObject::Ptr root = new juce::DynamicObject();
    root->setProperty("schema_version", "1.0.0");
    root->setProperty("target_plugin", targetPlugin);

    juce::DynamicObject::Ptr meta = new juce::DynamicObject();
    meta->setProperty("name", name);
    meta->setProperty("author", author);
    meta->setProperty("category", category);

    juce::Array<juce::var> tagVars;
    for (const auto& t : tags) tagVars.add(t);
    meta->setProperty("tags", tagVars);
    root->setProperty("metadata", juce::var(meta.get()));

    juce::DynamicObject::Ptr params = new juce::DynamicObject();
    for (auto* p : apvts.processor.getParameters()) {
        if (auto* paramWithId = dynamic_cast<juce::AudioProcessorParameterWithID*>(p)) {
            if (isLoadSkip(paramWithId->paramID)) continue;
            float rawVal = p->getValue();
            if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(p))
                rawVal = ranged->convertFrom0to1(rawVal);
            params->setProperty(paramWithId->paramID, rawVal);
        }
    }
    root->setProperty("parameters", juce::var(params.get()));

    if (onCustomStateSave) {
        root->setProperty("custom_state", onCustomStateSave());
    }

    juce::String json = juce::JSON::toString(juce::var(root.get()), true);
    bool ok = file.replaceWithText(json);
    if (ok) {
        setActivePresetName(name);
        refreshPresetList();
    }
    return ok;
}

} // namespace RlyehSound
