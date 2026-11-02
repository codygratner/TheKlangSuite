#include "FormlessProcessor.h"
#include "FormlessEditor.h"
#include "formless_spawn/formless_spawnParameterIds.h"
#include "formless_spawn/formless_spawnParameterLayout.h"

FormlessSpawnAudioProcessor::FormlessSpawnAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_(*this, &undoManager_, "PARAMS", createParameterLayout())
{
    pitchParam_      = apvts_.getRawParameterValue("pitch");
    macroDecayParam_ = apvts_.getRawParameterValue("macro_decay");
    punchParam_      = apvts_.getRawParameterValue("punch");
    pitchDepthParam_ = apvts_.getRawParameterValue("pitch_depth");
    pitchDecayParam_ = apvts_.getRawParameterValue("pitch_decay");
    timbreXParam_    = apvts_.getRawParameterValue("timbre_x");
    timbreYParam_    = apvts_.getRawParameterValue("timbre_y");
    timbreZParam_    = apvts_.getRawParameterValue("timbre_z");

    cutoffParam_     = apvts_.getRawParameterValue("cutoff");
    resonanceParam_  = apvts_.getRawParameterValue("resonance");
    vcfModeParam_    = apvts_.getRawParameterValue("vcf_mode");
    preDriveParam_   = apvts_.getRawParameterValue("pre_drive");
    lowBoostParam_   = apvts_.getRawParameterValue("low_boost");
    saturateParam_   = apvts_.getRawParameterValue("saturate");
    panParam_        = apvts_.getRawParameterValue("pan");
    volumeParam_     = apvts_.getRawParameterValue("volume");

    roleParam_       = apvts_.getRawParameterValue("role");
    modelParam_      = apvts_.getRawParameterValue("model");
    lifeModeParam_   = apvts_.getRawParameterValue("life_mode");
    keyTrackParam_   = apvts_.getRawParameterValue("key_track");
}

void FormlessSpawnAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/) {
    voice_.prepare(static_cast<float>(sampleRate));
}

void FormlessSpawnAudioProcessor::releaseResources() {
    voice_.reset();
}

bool FormlessSpawnAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void FormlessSpawnAudioProcessor::triggerVoice(float velocity, float noteHz) noexcept {
    tkdsp::FormlessSpawnVoice::Params p;
    if (pitchParam_)      p.pitch = pitchParam_->load(std::memory_order_relaxed);
    if (macroDecayParam_) p.macroDecay = macroDecayParam_->load(std::memory_order_relaxed);
    if (punchParam_)      p.punch = punchParam_->load(std::memory_order_relaxed);
    if (pitchDepthParam_) p.pitchDepth = pitchDepthParam_->load(std::memory_order_relaxed);
    if (pitchDecayParam_) p.pitchDecay = pitchDecayParam_->load(std::memory_order_relaxed);
    if (timbreXParam_)    p.timbreX = timbreXParam_->load(std::memory_order_relaxed);
    if (timbreYParam_)    p.timbreY = timbreYParam_->load(std::memory_order_relaxed);
    if (timbreZParam_)    p.timbreZ = timbreZParam_->load(std::memory_order_relaxed);
    if (cutoffParam_)     p.cutoff = cutoffParam_->load(std::memory_order_relaxed);
    if (resonanceParam_)  p.resonance = resonanceParam_->load(std::memory_order_relaxed);
    if (vcfModeParam_)    p.vcfMode = static_cast<int>(vcfModeParam_->load(std::memory_order_relaxed));
    if (preDriveParam_)   p.preDrive = preDriveParam_->load(std::memory_order_relaxed);
    if (lowBoostParam_)   p.lowBoost = lowBoostParam_->load(std::memory_order_relaxed);
    if (saturateParam_)   p.saturate = saturateParam_->load(std::memory_order_relaxed);
    if (panParam_)        p.pan = panParam_->load(std::memory_order_relaxed);
    if (volumeParam_)     p.volume = volumeParam_->load(std::memory_order_relaxed);
    if (roleParam_)       p.role = static_cast<int>(roleParam_->load(std::memory_order_relaxed));
    if (modelParam_) {
        p.model = static_cast<int>(modelParam_->load(std::memory_order_relaxed));
        voice_.setModel(p.model);
    }
    if (lifeModeParam_)   p.lifeMode = static_cast<int>(lifeModeParam_->load(std::memory_order_relaxed));
    if (keyTrackParam_)   p.keyTrack = static_cast<int>(keyTrackParam_->load(std::memory_order_relaxed));

    voice_.trigger(velocity, noteHz, p);
}

void FormlessSpawnAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();

    // 1. Gather Parameter Snapshot
    tkdsp::FormlessSpawnVoice::Params p;
    if (pitchParam_)      p.pitch = pitchParam_->load(std::memory_order_relaxed);
    if (macroDecayParam_) p.macroDecay = macroDecayParam_->load(std::memory_order_relaxed);
    if (punchParam_)      p.punch = punchParam_->load(std::memory_order_relaxed);
    if (pitchDepthParam_) p.pitchDepth = pitchDepthParam_->load(std::memory_order_relaxed);
    if (pitchDecayParam_) p.pitchDecay = pitchDecayParam_->load(std::memory_order_relaxed);
    if (timbreXParam_)    p.timbreX = timbreXParam_->load(std::memory_order_relaxed);
    if (timbreYParam_)    p.timbreY = timbreYParam_->load(std::memory_order_relaxed);
    if (timbreZParam_)    p.timbreZ = timbreZParam_->load(std::memory_order_relaxed);
    if (cutoffParam_)     p.cutoff = cutoffParam_->load(std::memory_order_relaxed);
    if (resonanceParam_)  p.resonance = resonanceParam_->load(std::memory_order_relaxed);
    if (vcfModeParam_)    p.vcfMode = static_cast<int>(vcfModeParam_->load(std::memory_order_relaxed));
    if (preDriveParam_)   p.preDrive = preDriveParam_->load(std::memory_order_relaxed);
    if (lowBoostParam_)   p.lowBoost = lowBoostParam_->load(std::memory_order_relaxed);
    if (saturateParam_)   p.saturate = saturateParam_->load(std::memory_order_relaxed);
    if (panParam_)        p.pan = panParam_->load(std::memory_order_relaxed);
    if (volumeParam_)     p.volume = volumeParam_->load(std::memory_order_relaxed);
    if (roleParam_)       p.role = static_cast<int>(roleParam_->load(std::memory_order_relaxed));
    if (modelParam_) {
        p.model = static_cast<int>(modelParam_->load(std::memory_order_relaxed));
        voice_.setModel(p.model);
    }
    if (lifeModeParam_)   p.lifeMode = static_cast<int>(lifeModeParam_->load(std::memory_order_relaxed));
    if (keyTrackParam_)   p.keyTrack = static_cast<int>(keyTrackParam_->load(std::memory_order_relaxed));

    // 2. Process MIDI Note Events
    for (const auto metadata : midiMessages) {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn()) {
            const int note = msg.getNoteNumber();
            const float vel = msg.getFloatVelocity();
            const float noteHz = static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(note));
            voice_.trigger(vel, noteHz, p);
        }
    }

    // 3. Render Audio
    auto* channelL = buffer.getWritePointer(0);
    auto* channelR = (buffer.getNumChannels() > 1) ? buffer.getWritePointer(1) : channelL;

    voice_.processBlockStereo(channelL, channelR, numSamples, p);
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
    return formless_spawnParameters::createLayout();
}

// Juce plugin entry point
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new FormlessSpawnAudioProcessor();
}
