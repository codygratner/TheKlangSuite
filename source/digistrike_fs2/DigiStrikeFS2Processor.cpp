#include "DigiStrikeFS2Processor.h"
#include "DigiStrikeFS2Editor.h"
#include "Digistrike_fs2ParameterIds.h"
#include "Digistrike_fs2ParameterLayout.h"

DigiStrikeFS2AudioProcessor::DigiStrikeFS2AudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_(*this, &undoManager_, "PARAMS", createParameterLayout())
{
    using namespace digistrike_fs2Parameters::Ids;

    categoryParam_      = apvts_.getRawParameterValue(category);
    lineageParam_       = apvts_.getRawParameterValue(lineage);
    voiceModeParam_     = apvts_.getRawParameterValue(voice_mode);
    lifeModeParam_      = apvts_.getRawParameterValue(life_mode);

    macroXParam_        = apvts_.getRawParameterValue(macro_x);
    macroYParam_        = apvts_.getRawParameterValue(macro_y);
    macroZParam_        = apvts_.getRawParameterValue(macro_z);
    macroTParam_        = apvts_.getRawParameterValue(macro_t);
    velAggrParam_       = apvts_.getRawParameterValue(vel_aggr);
    velTimbParam_       = apvts_.getRawParameterValue(vel_timb);
    panParam_           = apvts_.getRawParameterValue(pan);
    decayCurveParam_    = apvts_.getRawParameterValue(decay_curve);

    pitchDecayParam_    = apvts_.getRawParameterValue(pitch_decay);
    clickBleedParam_    = apvts_.getRawParameterValue(click_bleed);
    startOffsetParam_   = apvts_.getRawParameterValue(start_offset);
    traverseRateParam_  = apvts_.getRawParameterValue(traverse_rate);
    pitchDiveParam_     = apvts_.getRawParameterValue(pitch_dive);
    noiseMixParam_      = apvts_.getRawParameterValue(noise_mix);
    sampleLengthParam_  = apvts_.getRawParameterValue(sample_length);
    tableMorphParam_    = apvts_.getRawParameterValue(table_morph);
    loopModeParam_      = apvts_.getRawParameterValue(loop_mode);
    reverseParam_       = apvts_.getRawParameterValue(reverse);

    for (size_t i = 0; i < 20; ++i) {
        linParams_[i] = apvts_.getRawParameterValue(DigiStrikeFS2Info::kLineageParamIds[i]);
    }

    randXParam_         = apvts_.getRawParameterValue(rand_x);
    randYParam_         = apvts_.getRawParameterValue(rand_y);
    randZParam_         = apvts_.getRawParameterValue(rand_z);
    randTParam_         = apvts_.getRawParameterValue(rand_t);
    randAggrParam_      = apvts_.getRawParameterValue(rand_aggr);
    randTimbParam_      = apvts_.getRawParameterValue(rand_timb);
    randPanParam_       = apvts_.getRawParameterValue(rand_pan);
    randDecayParam_     = apvts_.getRawParameterValue(rand_decay);

    macroDecayParam_    = apvts_.getRawParameterValue(macro_decay);
    volumeParam_        = apvts_.getRawParameterValue(volume);
    sustainLevelParam_  = apvts_.getRawParameterValue(sustain_level);
    ampDecayParam_      = apvts_.getRawParameterValue(amp_decay);
    glideTimeParam_     = apvts_.getRawParameterValue(glide_time);

    cutoffParam_        = apvts_.getRawParameterValue(cutoff);
    resonanceParam_     = apvts_.getRawParameterValue(resonance);
    filterEnvAmtParam_  = apvts_.getRawParameterValue(filter_env_amt);
    filterDecayParam_   = apvts_.getRawParameterValue(filter_decay);
    filterTypeParam_    = apvts_.getRawParameterValue(filter_type);
    preDriveParam_      = apvts_.getRawParameterValue(pre_drive);
    crushParam_         = apvts_.getRawParameterValue(crush);
    shiftParam_         = apvts_.getRawParameterValue(shift);
    squashParam_        = apvts_.getRawParameterValue(squash);
    widerParam_         = apvts_.getRawParameterValue(wider);
    masterLimiterParam_ = apvts_.getRawParameterValue(master_limiter);
}

void DigiStrikeFS2AudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/) {
    voice_.prepare(static_cast<float>(sampleRate));
}

void DigiStrikeFS2AudioProcessor::releaseResources() {
    voice_.reset();
}

bool DigiStrikeFS2AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) {
        return false;
    }
    return true;
}

juce::AudioProcessorValueTreeState::ParameterLayout DigiStrikeFS2AudioProcessor::createParameterLayout() {
    return digistrike_fs2Parameters::createLayout();
}

void DigiStrikeFS2AudioProcessor::loadVoiceParams(tkdsp::DigiStrikeFS2Params& p) const noexcept {
    auto load = [](std::atomic<float>* ptr, float defVal) noexcept {
        return ptr != nullptr ? ptr->load(std::memory_order_relaxed) : defVal;
    };

    p.category      = static_cast<int>(load(categoryParam_, 9.0f));
    p.lineage       = static_cast<int>(load(lineageParam_, 14.0f));
    p.voiceMode     = static_cast<int>(load(voiceModeParam_, 2.0f));
    p.lifeMode      = static_cast<int>(load(lifeModeParam_, 1.0f));

    p.macroX        = load(macroXParam_, 0.5f);
    p.macroY        = load(macroYParam_, 0.5f);
    p.macroZ        = load(macroZParam_, 0.5f);
    p.macroT        = load(macroTParam_, 0.5f);
    p.velAggr       = load(velAggrParam_, 0.5f);
    p.velTimb       = load(velTimbParam_, 0.5f);
    p.pan           = load(panParam_, 0.0f);
    p.decayCurve    = load(decayCurveParam_, 0.0f);

    p.pitchDecay    = load(pitchDecayParam_, 15.0f);
    p.clickBleed    = load(clickBleedParam_, 0.0f);
    p.startOffset   = load(startOffsetParam_, 0.0f);
    p.traverseRate  = load(traverseRateParam_, 1.0f);
    p.pitchDive     = load(pitchDiveParam_, 0.0f);
    p.noiseMix      = load(noiseMixParam_, 0.0f);
    p.sampleLength  = load(sampleLengthParam_, 100.0f);
    p.tableMorph    = load(tableMorphParam_, 0.0f);
    p.loopMode      = static_cast<int>(load(loopModeParam_, 0.0f));
    p.reverse       = (load(reverseParam_, 0.0f) > 0.5f);

    uint32_t mask = 0;
    for (size_t i = 0; i < 20; ++i) {
        if (load(linParams_[i], i == 14 ? 1.0f : 0.0f) > 0.5f) {
            mask |= (1u << i);
        }
    }
    p.lineageMask   = DigiStrikeFS2Fleet::sanitizeMask(mask);

    p.randX         = load(randXParam_, 0.0f);
    p.randY         = load(randYParam_, 0.0f);
    p.randZ         = load(randZParam_, 0.0f);
    p.randT         = load(randTParam_, 0.0f);
    p.randAggr      = load(randAggrParam_, 0.0f);
    p.randTimb      = load(randTimbParam_, 0.0f);
    p.randPan       = load(randPanParam_, 0.0f);
    p.randDecay     = load(randDecayParam_, 0.0f);

    p.macroDecay    = load(macroDecayParam_, 0.5f);
    p.volume        = load(volumeParam_, 0.5f);
    p.sustainLevel  = load(sustainLevelParam_, 0.0f);
    p.ampDecay      = load(ampDecayParam_, 1.000f);
    p.glideTime     = load(glideTimeParam_, 0.050f);

    p.cutoff        = load(cutoffParam_, 20000.0f);
    p.resonance     = load(resonanceParam_, 0.30f);
    p.filterEnvAmt  = load(filterEnvAmtParam_, 0.0f);
    p.filterDecay   = load(filterDecayParam_, 0.150f);
    p.filterType    = static_cast<int>(load(filterTypeParam_, 0.0f));
    p.preDrive      = load(preDriveParam_, 0.0f);
    p.crush         = load(crushParam_, 48000.0f);
    p.shift         = load(shiftParam_, 0.0f);
    p.squash        = load(squashParam_, 0.0f);
    p.wider         = load(widerParam_, 0.0f);
    p.masterLimiter = (load(masterLimiterParam_, 0.0f) > 0.5f);
}

void DigiStrikeFS2AudioProcessor::publishTelemetry(float velocity, int noteNumber) noexcept {
    const auto& draws = voice_.getLatchedDraws();
    const int curIdx = activeTelemetryIdx_.load(std::memory_order_relaxed);
    const int nextIdx = (curIdx + 1) % 3;
    auto& buf = telemetryBuffers_[static_cast<size_t>(nextIdx)];
    buf.randVal = draws;
    buf.lastVelocity = velocity;
    buf.noteNumber = noteNumber;
    activeTelemetryIdx_.store(nextIdx, std::memory_order_release);
}

void DigiStrikeFS2AudioProcessor::triggerVoice(int noteNumber, float velocity, float noteHz) noexcept {
    voice_.triggerNote(noteNumber, velocity, noteHz);
    publishTelemetry(velocity, noteNumber);
}

void DigiStrikeFS2AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    if (numSamples <= 0) return;

    tkdsp::DigiStrikeFS2Params p;
    loadVoiceParams(p);
    voice_.setParams(p);

    // Process manual UI strike pad trigger & release
    const float manVel = manualTriggerVel_.exchange(0.0f, std::memory_order_acq_rel);
    const int manNote = manualTriggerNote_.exchange(-1, std::memory_order_acq_rel);
    if (manVel > 0.001f || manNote >= 0) {
        const int n = (manNote >= 0) ? manNote : lastMidiNote_.load(std::memory_order_relaxed);
        const float v = (manVel > 0.001f) ? manVel : lastMidiVelocity_.load(std::memory_order_relaxed);
        const float noteHz = static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(n));
        triggerVoice(n, v, noteHz);
    }

    const int relNote = manualReleaseNote_.exchange(-1, std::memory_order_acq_rel);
    if (relNote >= 0) {
        voice_.releaseNote(relNote);
    } else if (relNote == -2) {
        voice_.releaseNote(lastMidiNote_.load(std::memory_order_relaxed));
    }

    // Process MIDI messages
    for (const auto metadata : midiMessages) {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOff() || (msg.isNoteOn() && msg.getVelocity() == 0)) {
            voice_.releaseNote(msg.getNoteNumber());
        } else if (msg.isNoteOn()) {
            const int noteNumber = msg.getNoteNumber();
            const float velocity = msg.getFloatVelocity();
            lastMidiNote_.store(noteNumber, std::memory_order_relaxed);
            lastMidiVelocity_.store(velocity, std::memory_order_relaxed);
            const float noteHz = static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(noteNumber));
            triggerVoice(noteNumber, velocity, noteHz);
        } else if (msg.isAllNotesOff() || msg.isAllSoundOff()) {
            voice_.reset();
        } else if (msg.isController()) {
            const int cc = msg.getControllerNumber();
            if (cc == 64) {
                voice_.setSustainPedal(msg.getControllerValue() >= 64);
            } else if (cc == 120 || cc == 123) {
                voice_.reset();
            }
        }
    }

    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = (buffer.getNumChannels() > 1) ? buffer.getWritePointer(1) : leftChannel;

    voice_.renderBlock(leftChannel, rightChannel, numSamples);
}

juce::AudioProcessorEditor* DigiStrikeFS2AudioProcessor::createEditor() {
    return new DigiStrikeFS2AudioProcessorEditor(*this);
}

bool DigiStrikeFS2AudioProcessor::hasEditor() const {
    return true;
}

void DigiStrikeFS2AudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts_.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void DigiStrikeFS2AudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr && xmlState->hasTagName(apvts_.state.getType())) {
        apvts_.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

// Juce plugin entry point
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new DigiStrikeFS2AudioProcessor();
}
