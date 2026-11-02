#include "FormlessProcessor.h"
#include "FormlessEditor.h"
#include "formless_spawn/formless_spawnParameterIds.h"
#include "formless_spawn/formless_spawnParameterLayout.h"

FormlessSpawnAudioProcessor::FormlessSpawnAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_(*this, &undoManager_, "PARAMS", createParameterLayout())
{
    using namespace formless_spawnParameters::Ids;

    pitchParam_      = apvts_.getRawParameterValue(pitch);
    pitchDepthParam_ = apvts_.getRawParameterValue(pitch_depth);
    pitchDecayParam_ = apvts_.getRawParameterValue(pitch_decay);
    pitchCurveParam_ = apvts_.getRawParameterValue(pitch_curve);
    clickTuneParam_  = apvts_.getRawParameterValue(click_tune);
    clickMixParam_   = apvts_.getRawParameterValue(click_mix);
    subLevelParam_   = apvts_.getRawParameterValue(sub_level);
    lowBoostParam_   = apvts_.getRawParameterValue(low_boost);

    timbreXParam_    = apvts_.getRawParameterValue(timbre_x);
    timbreYParam_    = apvts_.getRawParameterValue(timbre_y);
    timbreZParam_    = apvts_.getRawParameterValue(timbre_z);
    timbreTParam_    = apvts_.getRawParameterValue(timbre_t);
    fmMorphParam_    = apvts_.getRawParameterValue(fm_morph);
    dampParam_       = apvts_.getRawParameterValue(damp);
    panParam_        = apvts_.getRawParameterValue(pan);
    velSensParam_    = apvts_.getRawParameterValue(vel_sens);

    randPitchParam_  = apvts_.getRawParameterValue(rand_pitch);
    randDecayParam_  = apvts_.getRawParameterValue(rand_decay);
    randTimbreParam_ = apvts_.getRawParameterValue(rand_timbre);
    randScatterParam_= apvts_.getRawParameterValue(rand_scatter);
    randModeParam_   = apvts_.getRawParameterValue(rand_mode);
    macroDecayParam_ = apvts_.getRawParameterValue(macro_decay);
    volumeParam_     = apvts_.getRawParameterValue(volume);

    cutoffParam_     = apvts_.getRawParameterValue(cutoff);
    resonanceParam_  = apvts_.getRawParameterValue(resonance);
    vcfModeParam_    = apvts_.getRawParameterValue(vcf_mode);
    preDriveParam_   = apvts_.getRawParameterValue(pre_drive);
    crushParam_      = apvts_.getRawParameterValue(crush);
    bitsParam_       = apvts_.getRawParameterValue(bits);
    tapeGlueParam_   = apvts_.getRawParameterValue(tape_glue);
    mojoProfileParam_= apvts_.getRawParameterValue(mojo_profile);
    mojoMixParam_    = apvts_.getRawParameterValue(mojo_mix);

    roleParam_       = apvts_.getRawParameterValue(role);
    modelParam_      = apvts_.getRawParameterValue(model);
    lifeModeParam_   = apvts_.getRawParameterValue(life_mode);
    keyTrackParam_   = apvts_.getRawParameterValue(key_track);
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

void FormlessSpawnAudioProcessor::loadVoiceParams(tkdsp::FormlessSpawnVoice::Params& p) const noexcept {
    if (pitchParam_)      p.pitch = pitchParam_->load(std::memory_order_relaxed);
    if (pitchDepthParam_) p.pitchDepth = pitchDepthParam_->load(std::memory_order_relaxed);
    if (pitchDecayParam_) p.pitchDecay = pitchDecayParam_->load(std::memory_order_relaxed);
    if (pitchCurveParam_) p.pitchCurve = pitchCurveParam_->load(std::memory_order_relaxed);
    if (clickTuneParam_)  p.clickTune = clickTuneParam_->load(std::memory_order_relaxed);
    if (clickMixParam_)   p.clickMix = clickMixParam_->load(std::memory_order_relaxed);
    if (subLevelParam_)   p.subLevel = subLevelParam_->load(std::memory_order_relaxed);
    if (lowBoostParam_)   p.lowBoost = lowBoostParam_->load(std::memory_order_relaxed);

    if (timbreXParam_)    p.timbreX = timbreXParam_->load(std::memory_order_relaxed);
    if (timbreYParam_)    p.timbreY = timbreYParam_->load(std::memory_order_relaxed);
    if (timbreZParam_)    p.timbreZ = timbreZParam_->load(std::memory_order_relaxed);
    if (timbreTParam_)    p.timbreT = timbreTParam_->load(std::memory_order_relaxed);
    if (fmMorphParam_)    p.fmMorph = fmMorphParam_->load(std::memory_order_relaxed);
    if (dampParam_)       p.damp = dampParam_->load(std::memory_order_relaxed);
    if (panParam_)        p.pan = panParam_->load(std::memory_order_relaxed);
    if (velSensParam_)    p.velSens = velSensParam_->load(std::memory_order_relaxed);

    if (randPitchParam_)  p.randPitch = randPitchParam_->load(std::memory_order_relaxed);
    if (randDecayParam_)  p.randDecay = randDecayParam_->load(std::memory_order_relaxed);
    if (randTimbreParam_) p.randTimbre = randTimbreParam_->load(std::memory_order_relaxed);
    if (randScatterParam_)p.randScatter = randScatterParam_->load(std::memory_order_relaxed);
    if (randModeParam_)   p.randMode = static_cast<int>(randModeParam_->load(std::memory_order_relaxed));
    if (macroDecayParam_) p.macroDecay = macroDecayParam_->load(std::memory_order_relaxed);
    if (volumeParam_)     p.volume = volumeParam_->load(std::memory_order_relaxed);

    if (cutoffParam_)     p.cutoff = cutoffParam_->load(std::memory_order_relaxed);
    if (resonanceParam_)  p.resonance = resonanceParam_->load(std::memory_order_relaxed);
    if (vcfModeParam_)    p.vcfMode = static_cast<int>(vcfModeParam_->load(std::memory_order_relaxed));
    if (preDriveParam_)   p.preDrive = preDriveParam_->load(std::memory_order_relaxed);
    if (crushParam_)      p.crush = crushParam_->load(std::memory_order_relaxed);
    if (bitsParam_)       p.bits = bitsParam_->load(std::memory_order_relaxed);
    if (tapeGlueParam_)   p.tapeGlue = tapeGlueParam_->load(std::memory_order_relaxed);
    if (mojoProfileParam_)p.mojoProfile = static_cast<int>(mojoProfileParam_->load(std::memory_order_relaxed));
    if (mojoMixParam_)    p.mojoMix = mojoMixParam_->load(std::memory_order_relaxed);

    if (roleParam_)       p.role = static_cast<int>(roleParam_->load(std::memory_order_relaxed));
    if (modelParam_)      p.model = static_cast<int>(modelParam_->load(std::memory_order_relaxed));
    if (lifeModeParam_)   p.lifeMode = static_cast<int>(lifeModeParam_->load(std::memory_order_relaxed));
    if (keyTrackParam_)   p.keyTrack = static_cast<int>(keyTrackParam_->load(std::memory_order_relaxed));
}

void FormlessSpawnAudioProcessor::triggerVoice(float velocity, float noteHz) noexcept {
    tkdsp::FormlessSpawnVoice::Params p;
    loadVoiceParams(p);
    voice_.setModel(p.model);
    voice_.trigger(velocity, noteHz, p);
}

void FormlessSpawnAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();

    // 1. Gather Parameter Snapshot
    tkdsp::FormlessSpawnVoice::Params p;
    loadVoiceParams(p);
    voice_.setModel(p.model);

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