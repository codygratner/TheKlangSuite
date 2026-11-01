#pragma once

#include <juce_core/juce_core.h>
#include <unordered_map>
#include <cmath>
#include <algorithm>

namespace RlyehSound {

/**
 * StateMigrator
 * 
 * Provides universal, bidirectional preset compatibility and state migration across
 * The Klang Suite (TKF, TKP, TKR) and ToadTracker (TT / The R-1).
 * 
 * Detects target plugin origins and translates high-precision raw engine parameters
 * or APVTS floats directly into canonical TKF parameters, while injecting transparent
 * default values for missing controls to maintain 100% session backward-compatibility.
 */
class StateMigrator {
public:
    static bool isR1OrToadPreset(const juce::var& rootObj) {
        if (!rootObj.isObject()) return false;
        
        juce::String target = rootObj.getProperty("target_plugin", "").toString();
        if (target.equalsIgnoreCase("TKR") || target.equalsIgnoreCase("TKR1") || target.equalsIgnoreCase("TT"))
            return true;

        if (rootObj.hasProperty("raw_engine_state")) {
            auto rawState = rootObj["raw_engine_state"];
            if (rawState.isObject()) {
                auto eng = rawState.getProperty("engine_type", "").toString();
                if (eng.equalsIgnoreCase("THER1") || eng.equalsIgnoreCase("DRUM_KIT") || eng.equalsIgnoreCase("ALPHAKRAFT"))
                    return true;
            }
        }
        return false;
    }

    static void migrateToTKF(const juce::var& rootObj, std::unordered_map<juce::String, float>& outParams) {
        if (!rootObj.isObject()) return;

        // 1. Ingest base parameters block (either "parameters" or legacy "state")
        juce::var paramVar;
        if (rootObj.hasProperty("parameters") && rootObj["parameters"].isObject()) {
            paramVar = rootObj["parameters"];
        } else if (rootObj.hasProperty("state") && rootObj["state"].isObject()) {
            paramVar = rootObj["state"];
        }

        if (paramVar.isObject()) {
            if (auto* dyn = paramVar.getDynamicObject()) {
                for (const auto& prop : dyn->getProperties()) {
                    outParams[prop.name.toString()] = static_cast<float>(prop.value);
                }
            }
        }

        // 2. Cross-plugin migration: The Klang Planter (TKP) -> TKF
        juce::String target = rootObj.getProperty("target_plugin", "").toString();
        if (target.equalsIgnoreCase("TKP")) {
            auto mapTKP = [&](const juce::String& tkpKey, const juce::String& tkfKey) {
                auto it = outParams.find(tkpKey);
                if (it != outParams.end()) {
                    outParams[tkfKey] = it->second;
                }
            };
            mapTKP("planter_filter_cutoff", "filter1_cutoff");
            mapTKP("planter_pitchenv_decay", "pitchenv1_decay");
            mapTKP("planter_noise_filter", "noise_filter");
            mapTKP("planter_mod_speed", "mod1_speed");
            mapTKP("planter_drive", "drive_amount");
        }

        // 3. The R-1 / ToadTracker Instrument Migration
        if (isR1OrToadPreset(rootObj)) {
            if (rootObj.hasProperty("raw_engine_state")) {
                auto rawState = rootObj["raw_engine_state"];
                if (rawState.isObject()) {
                    juce::String engType = rawState.getProperty("engine_type", "THER1").toString();

                    if (rawState.hasProperty("params8") && rawState["params8"].isArray()) {
                        auto* arr = rawState["params8"].getArray();
                        if (arr && arr->size() >= 8) {
                            if (engType.equalsIgnoreCase("THER1")) {
                                float algo         = static_cast<float>((*arr)[0]);
                                float modDepth     = static_cast<float>((*arr)[1]);
                                float modSpeed     = static_cast<float>((*arr)[2]);
                                float feedback     = static_cast<float>((*arr)[3]);
                                float modDecay     = static_cast<float>((*arr)[4]);
                                float carrierDecay = static_cast<float>((*arr)[5]);
                                float detune       = static_cast<float>((*arr)[6]);
                                float waveColor    = static_cast<float>((*arr)[7]);

                                outParams["carrier1_shape"]  = std::clamp(waveColor, 0.0f, 1.0f);
                                outParams["carrier1_depth"]  = std::clamp(modDepth, 0.0f, 1.0f);
                                outParams["mod1_speed"]      = std::clamp(modSpeed, 0.0f, 1.0f);
                                outParams["pitchenv1_decay"] = std::clamp(modDecay, 0.0f, 1.0f);
                                outParams["ampenv_decay"]    = std::clamp(carrierDecay, 0.0f, 1.0f);
                                outParams["carrier2_pitch"]  = std::clamp(detune, 0.0f, 1.0f);

                                int algoIdx = std::clamp(static_cast<int>(algo * 7.99f), 0, 7);
                                if (algoIdx == 7) {
                                    outParams["mixer_ringmod"] = 1.0f;
                                    outParams["mixer_noise_level"] = std::clamp(0.5f + 0.5f * feedback, 0.0f, 1.0f);
                                } else {
                                    outParams["mixer_carrier1_level"] = 1.0f;
                                    outParams["wavefolder_fold"] = std::clamp(feedback, 0.0f, 1.0f);
                                }
                            } else if (engType.equalsIgnoreCase("ALPHAKRAFT")) {
                                outParams["carrier1_shape"]   = std::clamp(static_cast<float>((*arr)[0]), 0.0f, 1.0f);
                                outParams["carrier2_pitch"]   = std::clamp(static_cast<float>((*arr)[1]), 0.0f, 1.0f);
                                outParams["ampenv_decay"]     = std::clamp(static_cast<float>((*arr)[4]), 0.0f, 1.0f);
                                outParams["filterenv1_depth"] = std::clamp(static_cast<float>((*arr)[5]), 0.0f, 1.0f);
                                outParams["chorus_mix"]       = std::clamp(static_cast<float>((*arr)[6]), 0.0f, 1.0f);
                                outParams["mixer_carrier1_level"] = 0.8f;
                                outParams["mixer_carrier2_level"] = 0.8f;
                            }
                        }
                    }

                    if (rawState.hasProperty("filter_cutoff_hz")) {
                        float hz = static_cast<float>(rawState["filter_cutoff_hz"]);
                        float logMin = std::log(20.0f);
                        float logMax = std::log(20000.0f);
                        float clampedHz = std::clamp(hz, 20.0f, 20000.0f);
                        outParams["filter1_cutoff"] = std::clamp((std::log(clampedHz) - logMin) / (logMax - logMin), 0.0f, 1.0f);
                    }
                    if (rawState.hasProperty("filter_resonance")) {
                        outParams["filter1_resonance"] = std::clamp(static_cast<float>(rawState["filter_resonance"]), 0.0f, 1.0f);
                    }
                    if (rawState.hasProperty("drive_amount")) {
                        outParams["drive_amount"] = std::clamp(static_cast<float>(rawState["drive_amount"]), 0.0f, 1.0f);
                    }
                }
            }
        }

        // 4. The Klang Mill (TKM) / EchoMill FX Rack Migration
        if (target.equalsIgnoreCase("TKM") || target.equalsIgnoreCase("TKMD") || target.equalsIgnoreCase("EchoMill")) {
            auto mapTKM = [&](const juce::String& tkmKey, const juce::String& tkfKey) {
                auto it = outParams.find(tkmKey);
                if (it != outParams.end()) {
                    outParams[tkfKey] = it->second;
                }
            };
            mapTKM("delay_time", "delay_div");
            mapTKM("feedback", "delay_feedback");
            mapTKM("mix", "delay_mix");
            mapTKM("slot2_depth", "delay_tone");
            if (rootObj.hasProperty("raw_fx_state")) {
                auto rawState = rootObj["raw_fx_state"];
                if (rawState.isObject()) {
                    if (rawState.hasProperty("delay_time"))
                        outParams["delay_div"] = static_cast<float>(rawState["delay_time"]);
                    if (rawState.hasProperty("feedback"))
                        outParams["delay_feedback"] = static_cast<float>(rawState["feedback"]);
                    if (rawState.hasProperty("slot2_depth"))
                        outParams["delay_tone"] = static_cast<float>(rawState["slot2_depth"]);
                    if (rawState.hasProperty("mix"))
                        outParams["delay_mix"] = static_cast<float>(rawState["mix"]);
                }
            }
        }

        // 5. Inject transparent defaults for essential routing & levels
        injectMissingDefaults(outParams);
    }

    static void injectMissingDefaults(std::unordered_map<juce::String, float>& params) {
        auto setDefaultIfMissing = [&](const juce::String& key, float defaultVal) {
            if (params.find(key) == params.end()) {
                params[key] = defaultVal;
            }
        };

        setDefaultIfMissing("mixer_carrier1_level", 1.0f);
        setDefaultIfMissing("amp_level", 0.8f);
        setDefaultIfMissing("carrier1_pitch", 0.5f);
        setDefaultIfMissing("filter1_cutoff", 1.0f);
        setDefaultIfMissing("filter1_resonance", 0.0f);
        setDefaultIfMissing("drive_amount", 0.0f);
    }
};

} // namespace RlyehSound
