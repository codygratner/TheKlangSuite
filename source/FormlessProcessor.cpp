#include "FormlessProcessor.h"
#include "FormlessEditor.h"
#include "formless_spawn/formless_spawnParameterIds.h"
#include "formless_spawn/formless_spawnParameterLayout.h"

FormlessSpawnAudioProcessor::FormlessSpawnAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_(*this, &undoManager_, "PARAMS", createParameterLayout())
{
    using namespace formless_spawnParameters::Ids;

    macroXParam_     = apvts_.getRawParameterValue(macro_x);
    macroYParam_     = apvts_.getRawParameterValue(macro_y);
    macroZParam_     = apvts_.getRawParameterValue(macro_z);
    macroTParam_     = apvts_.getRawParameterValue(macro_t);
    velAggrParam_    = apvts_.getRawParameterValue(vel_aggr);
    velTimbParam_    = apvts_.getRawParameterValue(vel_timb);
    panParam_        = apvts_.getRawParameterValue(pan);
    decayCurveParam_ = apvts_.getRawParameterValue(decay_curve);

    paramAParam_     = apvts_.getRawParameterValue(param_a);
    paramBParam_     = apvts_.getRawParameterValue(param_b);
    paramCParam_     = apvts_.getRawParameterValue(param_c);
    paramDParam_     = apvts_.getRawParameterValue(param_d);
    paramEParam_     = apvts_.getRawParameterValue(param_e);
    paramFParam_     = apvts_.getRawParameterValue(param_f);
    paramGParam_     = apvts_.getRawParameterValue(param_g);
    paramHParam_     = apvts_.getRawParameterValue(param_h);

    randXParam_      = apvts_.getRawParameterValue(rand_x);
    randYParam_      = apvts_.getRawParameterValue(rand_y);
    randZParam_      = apvts_.getRawParameterValue(rand_z);
    randTParam_      = apvts_.getRawParameterValue(rand_t);
    randAggrParam_   = apvts_.getRawParameterValue(rand_aggr);
    randTimbParam_   = apvts_.getRawParameterValue(rand_timb);
    randPanParam_    = apvts_.getRawParameterValue(rand_pan);
    randDecayParam_  = apvts_.getRawParameterValue(rand_decay);
    macroDecayParam_ = apvts_.getRawParameterValue(macro_decay);
    volumeParam_     = apvts_.getRawParameterValue(volume);
    ampDecayParam_   = apvts_.getRawParameterValue(amp_decay);

    cutoffParam_       = apvts_.getRawParameterValue(cutoff);
    resonanceParam_    = apvts_.getRawParameterValue(resonance);
    filterEnvAmtParam_ = apvts_.getRawParameterValue(filter_env_amt);
    filterDecayParam_  = apvts_.getRawParameterValue(filter_decay);
    filterTypeParam_   = apvts_.getRawParameterValue(filter_type);
    preDriveParam_   = apvts_.getRawParameterValue(pre_drive);
    crushParam_      = apvts_.getRawParameterValue(crush);
    shiftParam_      = apvts_.getRawParameterValue(shift);
    squashParam_     = apvts_.getRawParameterValue(squash);
    widerParam_      = apvts_.getRawParameterValue(wider);
    masterLimiterParam_ = apvts_.getRawParameterValue(master_limiter);

    roleParam_       = apvts_.getRawParameterValue(role);
    modelParam_      = apvts_.getRawParameterValue(model);
    lifeModeParam_   = apvts_.getRawParameterValue(life_mode);
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
    if (macroXParam_)     p.macroX = macroXParam_->load(std::memory_order_relaxed);
    if (macroYParam_)     p.macroY = macroYParam_->load(std::memory_order_relaxed);
    if (macroZParam_)     p.macroZ = macroZParam_->load(std::memory_order_relaxed);
    if (macroTParam_)     p.macroT = macroTParam_->load(std::memory_order_relaxed);
    if (velAggrParam_)    p.velAggr = velAggrParam_->load(std::memory_order_relaxed);
    if (velTimbParam_)    p.velTimb = velTimbParam_->load(std::memory_order_relaxed);
    if (panParam_)        p.pan = panParam_->load(std::memory_order_relaxed);
    if (decayCurveParam_) p.decayCurve = decayCurveParam_->load(std::memory_order_relaxed);

    if (paramAParam_)     p.paramA = paramAParam_->load(std::memory_order_relaxed);
    if (paramBParam_)     p.paramB = paramBParam_->load(std::memory_order_relaxed);
    if (paramCParam_)     p.paramC = paramCParam_->load(std::memory_order_relaxed);
    if (paramDParam_)     p.paramD = paramDParam_->load(std::memory_order_relaxed);
    if (paramEParam_)     p.paramE = paramEParam_->load(std::memory_order_relaxed);
    if (paramFParam_)     p.paramF = paramFParam_->load(std::memory_order_relaxed);
    if (paramGParam_)     p.paramG = paramGParam_->load(std::memory_order_relaxed);
    if (paramHParam_)     p.paramH = paramHParam_->load(std::memory_order_relaxed);

    if (randXParam_)      p.randX = randXParam_->load(std::memory_order_relaxed);
    if (randYParam_)      p.randY = randYParam_->load(std::memory_order_relaxed);
    if (randZParam_)      p.randZ = randZParam_->load(std::memory_order_relaxed);
    if (randTParam_)      p.randT = randTParam_->load(std::memory_order_relaxed);
    if (randAggrParam_)   p.randAggr = randAggrParam_->load(std::memory_order_relaxed);
    if (randTimbParam_)   p.randTimb = randTimbParam_->load(std::memory_order_relaxed);
    if (randPanParam_)    p.randPan = randPanParam_->load(std::memory_order_relaxed);
    if (randDecayParam_)  p.randDecay = randDecayParam_->load(std::memory_order_relaxed);
    if (macroDecayParam_) p.macroDecay = macroDecayParam_->load(std::memory_order_relaxed);
    if (volumeParam_)     p.volume = volumeParam_->load(std::memory_order_relaxed);
    if (ampDecayParam_)   p.ampDecay = ampDecayParam_->load(std::memory_order_relaxed);

    if (cutoffParam_)       p.cutoff = cutoffParam_->load(std::memory_order_relaxed);
    if (resonanceParam_)    p.resonance = resonanceParam_->load(std::memory_order_relaxed);
    if (filterEnvAmtParam_) p.filterEnvAmt = filterEnvAmtParam_->load(std::memory_order_relaxed);
    if (filterDecayParam_)  p.filterDecay = filterDecayParam_->load(std::memory_order_relaxed);
    if (filterTypeParam_)   p.filterType = static_cast<int>(filterTypeParam_->load(std::memory_order_relaxed));
    if (preDriveParam_)   p.preDrive = preDriveParam_->load(std::memory_order_relaxed);
    if (crushParam_)      p.crush = crushParam_->load(std::memory_order_relaxed);
    if (shiftParam_)      p.shift = shiftParam_->load(std::memory_order_relaxed);
    if (squashParam_)     p.squash = squashParam_->load(std::memory_order_relaxed);
    if (widerParam_)      p.wider = widerParam_->load(std::memory_order_relaxed);
    if (masterLimiterParam_) p.masterLimiter = masterLimiterParam_->load(std::memory_order_relaxed) > 0.5f;

    if (roleParam_)       p.role = static_cast<int>(roleParam_->load(std::memory_order_relaxed));
    if (modelParam_)      p.model = static_cast<int>(modelParam_->load(std::memory_order_relaxed));
    if (lifeModeParam_)   p.lifeMode = static_cast<int>(lifeModeParam_->load(std::memory_order_relaxed));
}

void FormlessSpawnAudioProcessor::publishTelemetry(float velocity, int noteNumber) noexcept {
    const auto& draws = voice_.getLatchedDraws();
    for (int i = 0; i < 8; ++i) {
        latchedDraw_[i].store(draws[i], std::memory_order_relaxed);
    }
    const int curIdx = activeTelemetryIdx_.load(std::memory_order_relaxed);
    const int nextIdx = (curIdx + 1) % 3;
    auto& buf = telemetryBuffers_[static_cast<size_t>(nextIdx)];
    buf.randVal = draws;
    buf.lastVelocity = velocity;
    buf.noteNumber = noteNumber;
    activeTelemetryIdx_.store(nextIdx, std::memory_order_release);
}

void FormlessSpawnAudioProcessor::triggerVoice(float velocity, float noteHz) noexcept {
    tkdsp::FormlessSpawnVoice::Params p;
    loadVoiceParams(p);
    voice_.setRole(p.role);
    voice_.setModel(p.model);
    voice_.trigger(velocity, noteHz, p);
    publishTelemetry(velocity, 60);
}

void FormlessSpawnAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();

    // 1. Gather Parameter Snapshot
    tkdsp::FormlessSpawnVoice::Params p;
    loadVoiceParams(p);
    voice_.setRole(p.role);
    voice_.setModel(p.model);

    // 2. Check Lock-Free Manual Trigger Mailbox
    if (manualTriggerPending_.exchange(false, std::memory_order_acq_rel)) {
        const float manVel = manualTriggerVel_.exchange(-1.0f, std::memory_order_acq_rel);
        const int manNote = manualTriggerNote_.exchange(-1, std::memory_order_acq_rel);
        const int n = (manNote >= 0) ? manNote : lastMidiNote_.load(std::memory_order_relaxed);
        const float v = (manVel > 0.001f) ? manVel : lastMidiVelocity_.load(std::memory_order_relaxed);
        const float noteHz = static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(n));
        voice_.trigger(v, noteHz, p);
        publishTelemetry(v, n);
    }

    // 3. Process MIDI Note Events
    for (const auto metadata : midiMessages) {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn()) {
            const int note = msg.getNoteNumber();
            const float vel = msg.getFloatVelocity();
            const float noteHz = static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(note));
            lastMidiNote_.store(note, std::memory_order_relaxed);
            lastMidiVelocity_.store(vel, std::memory_order_relaxed);
            voice_.trigger(vel, noteHz, p);
            publishTelemetry(vel, note);
        }
    }

    // 3. Render Audio
    auto* channelL = buffer.getWritePointer(0);
    auto* channelR = (buffer.getNumChannels() > 1) ? buffer.getWritePointer(1) : channelL;

    voice_.processBlockStereo(channelL, channelR, numSamples, p);
    limiterGR_.store(voice_.getLimiterGainReduction(), std::memory_order_relaxed);
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