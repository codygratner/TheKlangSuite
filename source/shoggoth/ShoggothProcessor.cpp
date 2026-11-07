#include "ShoggothProcessor.h"
#include "ShoggothEditor.h"
#include "FastMath.h"

namespace Shoggoth {

ShoggothProcessor::ShoggothProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_(*this, nullptr, "Parameters", ShoggothParameters::createLayout())
{
    initDefaultHarmonics();

    presetManager_.onCustomStateLoad = [this](const juce::var& v) { loadCustomPresetState(v); };
    presetManager_.onCustomStateSave = [this]() { return saveCustomPresetState(); };

    auto presets = presetManager_.getAvailablePresets();
    if (!presets.isEmpty()) {
        presetManager_.loadPresetByIndex(0, apvts_);
    }
}

void ShoggothProcessor::initDefaultHarmonics() noexcept {
    // Default: Warm analog sawtooth harmonic falloff (1/k)
    for (int k = 0; k < 63; ++k) {
        const float harm = static_cast<float>(k + 1);
        s1Harmonics_[k] = std::clamp(99.0f / harm, 0.0f, 99.0f);
        s2Harmonics_[k] = std::clamp(99.0f / (harm * harm), 0.0f, 99.0f); // S2 = softer triangle-like
        harmonicEnvAssign_[k] = static_cast<uint8_t>(k % 4);
        harmonicEnvAssign_[63 + k] = static_cast<uint8_t>((k + 1) % 4);
    }

    // Default 1987 DHG Envelopes
    for (int e = 0; e < kNumDHGEnv; ++e) {
        dhgEnvelopes_[e].stages[0] = { 85.0f, 99.0f }; // Fast attack
        dhgEnvelopes_[e].stages[1] = { 60.0f, 80.0f }; // Decay 1
        dhgEnvelopes_[e].stages[2] = { 40.0f, 70.0f }; // Decay 2
        dhgEnvelopes_[e].stages[3] = { 30.0f, 60.0f }; // Sustain
        dhgEnvelopes_[e].stages[4] = { 20.0f, 60.0f };
        dhgEnvelopes_[e].stages[5] = { 50.0f,  0.0f }; // Release
        dhgEnvelopes_[e].loop34 = false;
    }
}

void ShoggothProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/) {
    for (auto& v : voices_) {
        v.prepare(sampleRate);
    }
}

void ShoggothProcessor::releaseResources() {
    for (auto& v : voices_) {
        v.reset();
    }
}

bool ShoggothProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

int ShoggothProcessor::allocateVoice(int midiNote) noexcept {
    // 1. Check for idle voice
    for (size_t i = 0; i < voices_.size(); ++i) {
        if (!voices_[i].isVoiceActive()) {
            return static_cast<int>(i);
        }
    }

    // 2. Check for released voice with lowest energy
    int bestReleased = -1;
    uint64_t oldestAge = 0;
    for (size_t i = 0; i < voices_.size(); ++i) {
        if (voices_[i].getPhase() == VoicePhase::Release) {
            if (voices_[i].getAge() > oldestAge) {
                oldestAge = voices_[i].getAge();
                bestReleased = static_cast<int>(i);
            }
        }
    }
    if (bestReleased >= 0) {
        voices_[bestReleased].startStealFade();
        return bestReleased;
    }

    // 3. Fallback: steal oldest active voice
    int oldestIdx = 0;
    oldestAge = 0;
    for (size_t i = 0; i < voices_.size(); ++i) {
        if (voices_[i].getAge() > oldestAge) {
            oldestAge = voices_[i].getAge();
            oldestIdx = static_cast<int>(i);
        }
    }
    voices_[oldestIdx].startStealFade();
    return oldestIdx;
}

void ShoggothProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    TbdAudio::FastMath::enableFTZDAZ();

    buffer.clear();
    const int numSamples = buffer.getNumSamples();

    // Read APVTS atomic parameter values
    const float filterCutoff = apvts_.getRawParameterValue(ShoggothParameters::Ids::filter_cutoff)->load();
    const float filterReso   = apvts_.getRawParameterValue(ShoggothParameters::Ids::filter_resonance)->load();
    const float spectralTilt = apvts_.getRawParameterValue(ShoggothParameters::Ids::spectral_tilt)->load();
    const float sourceMorph  = apvts_.getRawParameterValue(ShoggothParameters::Ids::source_morph)->load();
    const float oddEvenBal   = apvts_.getRawParameterValue(ShoggothParameters::Ids::odd_even_bal)->load();
    const float disWarp      = apvts_.getRawParameterValue(ShoggothParameters::Ids::dissonance_warp)->load();
    const float pitchPunch   = apvts_.getRawParameterValue(ShoggothParameters::Ids::pitch_punch_depth)->load();
    const int   chorusMode   = static_cast<int>(apvts_.getRawParameterValue(ShoggothParameters::Ids::alpha_chorus)->load());
    const float waterbed     = apvts_.getRawParameterValue(ShoggothParameters::Ids::waterbed_depth)->load();
    const float swarmK       = apvts_.getRawParameterValue(ShoggothParameters::Ids::swarm_coupling)->load();
    const float foldWall     = apvts_.getRawParameterValue(ShoggothParameters::Ids::fold_wall)->load();
    const float seethe       = apvts_.getRawParameterValue(ShoggothParameters::Ids::mixer_seethe)->load();
    const float masterGainDb = apvts_.getRawParameterValue(ShoggothParameters::Ids::master_gain_db)->load();
    const float oscSlop      = apvts_.getRawParameterValue(ShoggothParameters::Ids::osc_slop)->load();
    const float velBright    = apvts_.getRawParameterValue(ShoggothParameters::Ids::vel_brightness)->load();
    const float modalRel     = apvts_.getRawParameterValue(ShoggothParameters::Ids::modal_release)->load();
    const int   engineMode   = static_cast<int>(apvts_.getRawParameterValue(ShoggothParameters::Ids::vintage_engine_mode)->load());
    const int   cpuTier      = static_cast<int>(apvts_.getRawParameterValue(ShoggothParameters::Ids::cpu_tier)->load());

    const float ampAttack    = apvts_.getRawParameterValue(ShoggothParameters::Ids::amp_attack)->load();
    const float ampDecay     = apvts_.getRawParameterValue(ShoggothParameters::Ids::amp_decay)->load();
    const float ampSustain   = apvts_.getRawParameterValue(ShoggothParameters::Ids::amp_sustain)->load();
    const float ampRelease   = apvts_.getRawParameterValue(ShoggothParameters::Ids::amp_release)->load();
    const float filterEnvDepth = apvts_.getRawParameterValue(ShoggothParameters::Ids::filter_env_depth)->load();

    const bool modernMode = (engineMode != 1); // 1 = 1987 Stepped DDF

    // Parse MIDI Events
    for (const auto metadata : midiMessages) {
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn()) {
            const int note = msg.getNoteNumber();
            const float vel = msg.getFloatVelocity();
            const float f0 = static_cast<float>(msg.getMidiNoteInHertz(note));

            const int vIdx = allocateVoice(note);
            auto& voice = voices_[vIdx];

            // Copy master harmonic amplitudes into voice
            for (int k = 0; k < 63; ++k) {
                // Morph between S1 and S2
                const float s1 = s1Harmonics_[k] / 99.0f;
                const float s2 = s2Harmonics_[k] / 99.0f;
                const float morphed = std::sqrt((1.0f - sourceMorph) * (s1 * s1) + sourceMorph * (s2 * s2));

                voice.getState().p.baseHarmonic[k] = morphed;
                voice.getState().p.kAmp[k + kGuard] = morphed;
                voice.getState().p.envAssign[k] = harmonicEnvAssign_[k];
            }

            if (modernMode) {
                const float lastAmp = voice.getState().p.baseHarmonic[62];
                for (int k = 63; k < static_cast<int>(voice.getState().activePartials); ++k) {
                    const float extAmp = lastAmp * (63.0f / static_cast<float>(k + 1));
                    voice.getState().p.baseHarmonic[k] = extAmp;
                    voice.getState().p.kAmp[k + kGuard] = extAmp;
                    voice.getState().p.envAssign[k] = static_cast<uint8_t>(k % 4);
                }
            } else {
                for (int k = 63; k < kMaxPartials; ++k) {
                    voice.getState().p.baseHarmonic[k] = 0.0f;
                    voice.getState().p.kAmp[k + kGuard] = 0.0f;
                }
            }

            // Copy DHG envelopes
            for (int e = 0; e < kNumDHGEnv; ++e) {
                voice.getState().dhgData[e] = dhgEnvelopes_[e];
            }

            voice.noteOn(note, vel, f0, pitchPunch, modernMode, cpuTier);

        } else if (msg.isNoteOff()) {
            const int note = msg.getNoteNumber();
            const float relVel = static_cast<float>(msg.getVelocity());
            for (auto& v : voices_) {
                if (v.isVoiceActive() && v.getMidiNote() == note) {
                    v.noteOff(relVel);
                }
            }
        } else if (msg.isPitchWheel()) {
            // 14-bit pitch bend mapped to +/- 2 semitones
            const int rawPb = msg.getPitchWheelValue();
            pitchBendSemitones_ = (static_cast<float>(rawPb - 8192) / 8192.0f) * 2.0f;
        } else if (msg.isController()) {
            if (msg.getControllerNumber() == 1) { // Mod Wheel
                modWheelNorm_ = static_cast<float>(msg.getControllerValue()) / 127.0f;
            } else if (msg.getControllerNumber() == 64) { // Sustain Pedal
                sustainPedalDown_ = (msg.getControllerValue() >= 64);
            }
        }
    }

    // Render Voices
    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);

    for (auto& v : voices_) {
        if (v.isVoiceActive()) {
            v.renderBlock(leftChannel, rightChannel, numSamples,
                          filterCutoff, filterReso, spectralTilt,
                          sourceMorph, oddEvenBal, disWarp,
                          foldWall, swarmK, chorusMode,
                          waterbed, oscSlop, velBright, modalRel,
                          ampAttack, ampDecay, ampSustain, ampRelease,
                          filterEnvDepth);
        }
    }

    // Mixer Seethe (Asymmetric console drive) & Master Gain
    const float masterGain = TbdAudio::FastMath::fastDbToGain(masterGainDb);

    for (int i = 0; i < numSamples; ++i) {
        float l = leftChannel[i];
        float r = rightChannel[i];

        if (seethe > 0.001f) {
            // Asymmetric op-amp / transformer saturation curve
            const float drive = 1.0f + seethe * 2.0f;
            l = (l * drive) - 0.15f * (l * l * drive * drive) - 0.05f * (l * l * l);
            r = (r * drive) - 0.15f * (r * r * drive * drive) - 0.05f * (r * r * r);
        }

        leftChannel[i]  = l * masterGain;
        rightChannel[i] = r * masterGain;
    }

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        TbdAudio::FastMath::sanitizeBuffer(buffer.getWritePointer(ch), buffer.getNumSamples());
    }
}

// ==============================================================================
// State Serialization (<AdditiveSpectrum> Recall Law, INVARIANT-ST1)
// ==============================================================================
void ShoggothProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts_.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());

    const float s = storedEditorScale_.load(std::memory_order_relaxed);
    if (s > 0.0f) xml->setAttribute("editorScale", static_cast<double>(s));

    // Child XML for 126 harmonic amplitudes & DHG envelopes
    auto* specXml = xml->createNewChildElement("AdditiveSpectrum");

    juce::String s1Str, s2Str, assignStr;
    for (int k = 0; k < 63; ++k) {
        s1Str << s1Harmonics_[k] << (k < 62 ? " " : "");
        s2Str << s2Harmonics_[k] << (k < 62 ? " " : "");
        assignStr << static_cast<int>(harmonicEnvAssign_[k]) << " ";
    }
    specXml->setAttribute("s1", s1Str);
    specXml->setAttribute("s2", s2Str);
    specXml->setAttribute("assign", assignStr);

    copyXmlToBinary(*xml, destData);
}

void ShoggothProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts_.state.getType())) {
        if (xmlState->hasAttribute("editorScale")) {
            storedEditorScale_.store(static_cast<float>(xmlState->getDoubleAttribute("editorScale", 1.0)), std::memory_order_relaxed);
        }

        if (auto* specXml = xmlState->getChildByName("AdditiveSpectrum")) {
            auto s1Tokens = juce::StringArray::fromTokens(specXml->getStringAttribute("s1"), " ", "");
            auto s2Tokens = juce::StringArray::fromTokens(specXml->getStringAttribute("s2"), " ", "");
            auto assignTokens = juce::StringArray::fromTokens(specXml->getStringAttribute("assign"), " ", "");

            for (int k = 0; k < 63; ++k) {
                if (k < s1Tokens.size()) s1Harmonics_[k] = s1Tokens[k].getFloatValue();
                if (k < s2Tokens.size()) s2Harmonics_[k] = s2Tokens[k].getFloatValue();
                if (k < assignTokens.size()) harmonicEnvAssign_[k] = static_cast<uint8_t>(assignTokens[k].getIntValue());
            }
        }

        apvts_.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

// ==============================================================================
// Preset Custom State Ingestion / Serialization
// ==============================================================================
void ShoggothProcessor::loadCustomPresetState(const juce::var& customState) {
    if (!customState.isObject()) return;
    auto* obj = customState.getDynamicObject();
    if (!obj) return;

    if (obj->hasProperty("s1_harmonics") && obj->getProperty("s1_harmonics").isArray()) {
        auto* arr = obj->getProperty("s1_harmonics").getArray();
        for (int k = 0; k < 63 && k < arr->size(); ++k) {
            s1Harmonics_[k] = static_cast<float>((*arr)[k]);
        }
    }
    if (obj->hasProperty("s2_harmonics") && obj->getProperty("s2_harmonics").isArray()) {
        auto* arr = obj->getProperty("s2_harmonics").getArray();
        for (int k = 0; k < 63 && k < arr->size(); ++k) {
            s2Harmonics_[k] = static_cast<float>((*arr)[k]);
        }
    }
    if (obj->hasProperty("harmonic_env_assign") && obj->getProperty("harmonic_env_assign").isArray()) {
        auto* arr = obj->getProperty("harmonic_env_assign").getArray();
        for (int k = 0; k < 126 && k < arr->size(); ++k) {
            harmonicEnvAssign_[k] = static_cast<uint8_t>(static_cast<int>((*arr)[k]) & 0x03);
        }
    }
    if (obj->hasProperty("dhg_envelopes") && obj->getProperty("dhg_envelopes").isArray()) {
        auto* arr = obj->getProperty("dhg_envelopes").getArray();
        for (int e = 0; e < kNumDHGEnv && e < arr->size(); ++e) {
            if (auto* eObj = (*arr)[e].getDynamicObject()) {
                dhgEnvelopes_[e].loop34 = static_cast<bool>(eObj->getProperty("loop34"));
                if (eObj->hasProperty("stages") && eObj->getProperty("stages").isArray()) {
                    auto* stagesArr = eObj->getProperty("stages").getArray();
                    for (int s = 0; s < 6 && s < stagesArr->size(); ++s) {
                        if (auto* sObj = (*stagesArr)[s].getDynamicObject()) {
                            dhgEnvelopes_[e].stages[s].rate = static_cast<float>(sObj->getProperty("rate"));
                            dhgEnvelopes_[e].stages[s].level = static_cast<float>(sObj->getProperty("level"));
                        }
                    }
                }
            }
        }
    }
}

juce::var ShoggothProcessor::saveCustomPresetState() const {
    auto* obj = new juce::DynamicObject();
    juce::var v(obj);
    obj->setProperty("num_harmonics", 63);

    juce::Array<juce::var> s1Arr, s2Arr, assignArr, envsArr;
    for (int k = 0; k < 63; ++k) {
        s1Arr.add(static_cast<double>(s1Harmonics_[k]));
        s2Arr.add(static_cast<double>(s2Harmonics_[k]));
    }
    for (int k = 0; k < 126; ++k) {
        assignArr.add(static_cast<int>(harmonicEnvAssign_[k]));
    }
    for (int e = 0; e < kNumDHGEnv; ++e) {
        auto* eObj = new juce::DynamicObject();
        eObj->setProperty("loop34", dhgEnvelopes_[e].loop34);
        juce::Array<juce::var> stagesArr;
        for (int s = 0; s < 6; ++s) {
            auto* sObj = new juce::DynamicObject();
            sObj->setProperty("rate", static_cast<double>(dhgEnvelopes_[e].stages[s].rate));
            sObj->setProperty("level", static_cast<double>(dhgEnvelopes_[e].stages[s].level));
            stagesArr.add(juce::var(sObj));
        }
        eObj->setProperty("stages", stagesArr);
        envsArr.add(juce::var(eObj));
    }
    obj->setProperty("s1_harmonics", s1Arr);
    obj->setProperty("s2_harmonics", s2Arr);
    obj->setProperty("harmonic_env_assign", assignArr);
    obj->setProperty("dhg_envelopes", envsArr);
    return v;
}

// ==============================================================================
// Telemetry & Modulation Halos
// ==============================================================================
ShoggothModVisual ShoggothProcessor::getParamModulationInfo(const juce::String& paramId) const noexcept {
    ShoggothModVisual mv;
    mv.isModulated = false;

    // Report active modulation from mod wheel / chaos matrix
    if (paramId == ShoggothParameters::Ids::filter_cutoff && modWheelNorm_ > 0.001f) {
        mv.isModulated = true;
        mv.rangeMinNorm = 0.0f;
        mv.rangeMaxNorm = modWheelNorm_;
        mv.currentNorm = modWheelNorm_;
    }
    return mv;
}

// ==============================================================================
// File Ingestion API
// ==============================================================================
bool ShoggothProcessor::loadSysExFile(const juce::File& file) {
    juce::MemoryBlock mb;
    if (!file.loadFileAsData(mb)) return false;

    DecodedSinglePatch patch;
    if (SysExParser::parseSysExFile(static_cast<const uint8_t*>(mb.getData()), mb.getSize(), patch)) {
        for (int k = 0; k < 63; ++k) {
            s1Harmonics_[k] = patch.s1Harmonics[k];
            s2Harmonics_[k] = patch.s2Harmonics[k];
            harmonicEnvAssign_[k] = patch.s1EnvAssign[k];
        }
        for (int e = 0; e < kNumDHGEnv; ++e) {
            dhgEnvelopes_[e] = patch.dhgEnvs[e];
        }
        return true;
    }
    return false;
}

bool ShoggothProcessor::loadAudioFile(const juce::File& file) {
    auto res = AudioResynthesizer::resynthesizeAudioFile(file);
    if (res.valid) {
        for (int k = 0; k < 63; ++k) {
            s1Harmonics_[k] = res.s1Harmonics[k];
            s2Harmonics_[k] = res.s2Harmonics[k];
        }
        return true;
    }
    return false;
}

float ShoggothProcessor::getHarmonicLevel(int sourceIdx, int harmonicIdx) const noexcept {
    if (harmonicIdx < 0 || harmonicIdx >= 63) return 0.0f;
    return (sourceIdx == 0) ? s1Harmonics_[harmonicIdx] : s2Harmonics_[harmonicIdx];
}

void ShoggothProcessor::setHarmonicLevel(int sourceIdx, int harmonicIdx, float level99) noexcept {
    if (harmonicIdx < 0 || harmonicIdx >= 63) return;
    const float clamped = std::clamp(level99, 0.0f, 99.0f);
    if (sourceIdx == 0) s1Harmonics_[harmonicIdx] = clamped;
    else s2Harmonics_[harmonicIdx] = clamped;
}

uint8_t ShoggothProcessor::getHarmonicEnvAssign(int harmonicIdx) const noexcept {
    if (harmonicIdx < 0 || harmonicIdx >= 126) return 0;
    return harmonicEnvAssign_[harmonicIdx];
}

void ShoggothProcessor::setHarmonicEnvAssign(int harmonicIdx, uint8_t envIdx) noexcept {
    if (harmonicIdx < 0 || harmonicIdx >= 126) return;
    harmonicEnvAssign_[harmonicIdx] = envIdx & 0x03;
}

juce::AudioProcessorEditor* ShoggothProcessor::createEditor() {
    return new ShoggothEditor(*this);
}

} // namespace Shoggoth

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new Shoggoth::ShoggothProcessor();
}
