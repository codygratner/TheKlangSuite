#include "FormlessProcessor.h"
#include "FormlessEditor.h"

FormlessSpawnAudioProcessor::FormlessSpawnAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_(*this, &undoManager_, "PARAMS", createParameterLayout())
{
    // Cache raw parameter pointers for lock-free audio thread reads
    for (int v = 0; v < tkdsp::FormlessSpawnEngine::kTotalVoices; ++v) {
        for (int p = 0; p < tkdsp::FormlessSpawnEngine::kNumParamsPerVoice; ++p) {
            juce::String id = "v" + juce::String(v) + "_p" + juce::String(p);
            cachedParams_[v][p] = apvts_.getRawParameterValue(id);
        }
    }
    masterLowBoostParam_ = apvts_.getRawParameterValue("master_low_boost");
    masterDriveParam_ = apvts_.getRawParameterValue("master_drive");
}

void FormlessSpawnAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/) {
    engine_.prepare(static_cast<float>(sampleRate));
}

void FormlessSpawnAudioProcessor::releaseResources() {
    engine_.reset();
}

bool FormlessSpawnAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void FormlessSpawnAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();

    // 1. Process MIDI Note Events
    for (const auto metadata : midiMessages) {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn()) {
            const int note = msg.getNoteNumber();
            const float vel = msg.getFloatVelocity();

            // General MIDI standard drum mapping:
            switch (note) {
                case 35: // Acoustic Bass Drum
                case 36: // Electric Bass Drum
                    engine_.trigger(0, vel);
                    break;
                case 38: // Acoustic Snare
                case 40: // Electric Snare
                    engine_.trigger(1, vel);
                    break;
                case 37: // Side Stick
                case 39: // Hand Clap
                    engine_.trigger(6, vel); // Voice 6: Clap
                    break;
                case 42: // Closed Hi-Hat
                case 44: // Pedal Hi-Hat
                    engine_.trigger(5, vel); // Voice 5: Closed Hat (Chokes Voice 4)
                    break;
                case 46: // Open Hi-Hat
                    engine_.trigger(4, vel); // Voice 4: Open Hat
                    break;
                case 41: // Low Floor Tom
                case 43: // High Floor Tom
                case 45: // Low Tom
                case 47: // Low-Mid Tom
                    engine_.trigger(2, vel); // Voice 2: Metallic Zap / Low Tom
                    break;
                case 48: // Hi-Mid Tom
                case 50: // High Tom
                    engine_.trigger(3, vel); // Voice 3: Cyber Bongo / Percussion
                    break;
                case 49: // Crash Cymbal 1
                case 51: // Ride Cymbal 1
                case 57: // Crash Cymbal 2
                    engine_.trigger(7, vel); // Voice 7: Crash / Sub Wash
                    break;
                default: {
                    // Chromatic playback on Voice 0 if outside standard drum octave
                    const float freqHz = static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(note));
                    engine_.trigger(0, vel, freqHz);
                    break;
                }
            }
        }
    }

    // 2. Synchronize APVTS Parameters to Engine
    for (int v = 0; v < tkdsp::FormlessSpawnEngine::kTotalVoices; ++v) {
        for (int p = 0; p < tkdsp::FormlessSpawnEngine::kNumParamsPerVoice; ++p) {
            if (cachedParams_[v][p] != nullptr) {
                engine_.setParam(v, p, cachedParams_[v][p]->load(std::memory_order_relaxed));
            }
        }
    }
    if (masterLowBoostParam_ != nullptr) engine_.setMasterLowBoost(masterLowBoostParam_->load(std::memory_order_relaxed));
    if (masterDriveParam_ != nullptr) engine_.setMasterDrive(masterDriveParam_->load(std::memory_order_relaxed));

    // 3. Clear Extra Channels
    for (int i = 2; i < buffer.getNumChannels(); ++i) {
        buffer.clear(i, 0, numSamples);
    }

    // 4. Render Stereo Audio Block
    float* channelL = buffer.getWritePointer(0);
    float* channelR = (buffer.getNumChannels() > 1) ? buffer.getWritePointer(1) : channelL;

    engine_.renderBlockStereo(channelL, channelR, numSamples);
}

juce::AudioProcessorEditor* FormlessSpawnAudioProcessor::createEditor() {
    return new FormlessSpawnAudioProcessorEditor(*this);
}

bool FormlessSpawnAudioProcessor::hasEditor() const {
    return true;
}

void FormlessSpawnAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts_.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void FormlessSpawnAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts_.state.getType())) {
        apvts_.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout FormlessSpawnAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    const char* paramNames[tkdsp::FormlessSpawnEngine::kNumParamsPerVoice] = {
        "Pitch", "Decay", "Pitch Env Depth", "Pitch Env Decay",
        "Waveform", "Mod Type", "Mod Speed", "Mod Depth",
        "Wavefold", "Drive", "Cutoff", "Resonance",
        "Pan", "Volume"
    };

    float defaultValues[tkdsp::FormlessSpawnEngine::kTotalVoices][tkdsp::FormlessSpawnEngine::kNumParamsPerVoice] = {
        // V0: Kick
        { 0.15f, 0.35f, 0.85f, 0.10f, 0.0f, 0.0f, 0.20f, 0.15f, 0.25f, 0.30f, 0.70f, 0.15f, 0.5f, 0.95f },
        // V1: Snare
        { 0.35f, 0.28f, 0.65f, 0.08f, 0.25f, 0.80f, 0.50f, 0.65f, 0.20f, 0.20f, 0.85f, 0.25f, 0.5f, 0.85f },
        // V2: Ring Zap
        { 0.45f, 0.22f, 0.75f, 0.05f, 0.0f, 1.0f, 0.65f, 0.75f, 0.40f, 0.25f, 0.80f, 0.35f, 0.4f, 0.80f },
        // V3: Bongo
        { 0.55f, 0.18f, 0.50f, 0.05f, 0.5f, 0.40f, 0.30f, 0.45f, 0.50f, 0.35f, 0.60f, 0.60f, 0.6f, 0.75f },
        // V4: Open Hat
        { 0.70f, 0.50f, 0.50f, 0.05f, 0.0f, 0.0f, 0.50f, 0.0f, 0.0f, 0.0f, 0.85f, 0.20f, 0.6f, 0.80f },
        // V5: Closed Hat
        { 0.75f, 0.15f, 0.50f, 0.05f, 0.0f, 0.0f, 0.50f, 0.0f, 0.0f, 0.0f, 0.85f, 0.20f, 0.4f, 0.80f },
        // V6: Clap
        { 0.50f, 0.25f, 0.50f, 0.05f, 0.0f, 0.0f, 0.50f, 0.0f, 0.0f, 0.0f, 0.80f, 0.15f, 0.5f, 0.85f },
        // V7: Crash
        { 0.40f, 0.65f, 0.50f, 0.05f, 0.0f, 0.0f, 0.50f, 0.0f, 0.0f, 0.0f, 0.75f, 0.10f, 0.5f, 0.75f }
    };

    for (int v = 0; v < tkdsp::FormlessSpawnEngine::kTotalVoices; ++v) {
        for (int p = 0; p < tkdsp::FormlessSpawnEngine::kNumParamsPerVoice; ++p) {
            juce::String id = "v" + juce::String(v) + "_p" + juce::String(p);
            juce::String name = "V" + juce::String(v + 1) + " " + paramNames[p];
            float defVal = defaultValues[v][p];
            layout.add(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID(id, 1), name, juce::NormalisableRange<float>(0.0f, 1.0f), defVal
            ));
        }
    }

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("master_low_boost", 1), "Master Low Boost", juce::NormalisableRange<float>(0.0f, 1.0f), 0.45f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("master_drive", 1), "Master Drive", juce::NormalisableRange<float>(0.0f, 1.0f), 0.15f
    ));

    return layout;
}


juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new FormlessSpawnAudioProcessor();
}
