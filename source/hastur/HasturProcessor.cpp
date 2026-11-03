#include <algorithm>
#include <cmath>
[[nodiscard]] static constexpr float stepRandomNorm(float kNorm, float amount, float r) noexcept {
    return std::clamp((1.0f - amount) * kNorm + amount * r, 0.0f, 1.0f);
}

[[nodiscard]] static constexpr float stepRandomRange(float v, float lo, float hi, float amount, float r) noexcept {
    const float span = hi - lo;
    return (amount <= 0.0f || std::abs(span) < 1e-6f) ? v : lo + span * stepRandomNorm((v - lo) / span, amount, r);
}

#include "HasturProcessor.h"
#include "HasturEditor.h"
#include "HasturParameterLayout.h"

HasturAudioProcessor::HasturAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_(*this, &undoManager_, "HasturParams", createParameterLayout())
{
    voice_.seed(static_cast<uint64_t>(juce::Random::getSystemRandom().nextInt64()));
    // Bay 1: DCO
    osc1MultParam_          = apvts_.getRawParameterValue("osc1_mult");
    osc1ApfPitchTrackParam_ = apvts_.getRawParameterValue("osc1_apf_pitch_track");
    osc1DriveParam_         = apvts_.getRawParameterValue("osc1_drive");
    dcoBiteParam_           = apvts_.getRawParameterValue("dco_bite");
    subApfDepthParam_       = apvts_.getRawParameterValue("sub_apf_depth");
    subSpitParam_           = apvts_.getRawParameterValue("sub_spit");
    dcoMorphParam_          = apvts_.getRawParameterValue("dco_morph");
    dcoWaveFamilyParam_     = apvts_.getRawParameterValue("dco_wave_family");
    dcoChopperTopoParam_    = apvts_.getRawParameterValue("dco_chopper_topo");
    subOctaveParam_         = apvts_.getRawParameterValue("sub_octave");
    subPulseTrainParam_     = apvts_.getRawParameterValue("sub_pulse_train");
    subApfTapParam_         = apvts_.getRawParameterValue("sub_apf_tap");

    // Bay 2: Stem Mixer
    osc1LevelParam_         = apvts_.getRawParameterValue("osc1_level");
    osc2LevelParam_         = apvts_.getRawParameterValue("osc2_level");
    noiseLevelParam_        = apvts_.getRawParameterValue("noise_level");
    mixerDriveParam_        = apvts_.getRawParameterValue("mixer_drive");
    // master_spread excised
    oscSlopParam_           = apvts_.getRawParameterValue("osc_slop");

    // Bay 3: Dual IR3R05 VCF
    cutoffParam_            = apvts_.getRawParameterValue("cutoff");
    resonanceParam_         = apvts_.getRawParameterValue("resonance");
    filterEnvDecayParam_    = apvts_.getRawParameterValue("filter_env_decay");
    burbleDepthParam_       = apvts_.getRawParameterValue("burble_depth");
    laserDriveParam_        = apvts_.getRawParameterValue("laser_drive");
    filterWarpParam_        = apvts_.getRawParameterValue("filter_warp");
    laserBiteParam_         = apvts_.getRawParameterValue("laser_bite");
    filterOrbitParam_       = apvts_.getRawParameterValue("filter_orbit");
    burbleModeParam_        = apvts_.getRawParameterValue("burble_mode");
    filterDeckParam_        = apvts_.getRawParameterValue("filter_deck");
    envDepthParam_          = apvts_.getRawParameterValue("env_depth");
    filterVelocityParam_    = apvts_.getRawParameterValue("filter_velocity");
    keyTrackParam_          = apvts_.getRawParameterValue("keytrack");
    filterLfoDepthParam_    = apvts_.getRawParameterValue("filter_lfo_depth");
    filterLfoDestParam_     = apvts_.getRawParameterValue("filter_lfo_dest");

    // Bay 4: LFO & Glide
    lfoRateParam_           = apvts_.getRawParameterValue("lfo_rate");
    glideTimeParam_         = apvts_.getRawParameterValue("glide_time");
    lfoWaveParam_           = apvts_.getRawParameterValue("lfo_wave");
    lfoShapeParam_          = apvts_.getRawParameterValue("lfo_shape");
    lfoSyncParam_           = apvts_.getRawParameterValue("lfo_sync");
    pwLfoAmountParam_       = apvts_.getRawParameterValue("pw_lfo_amount");
    pwLfoRateParam_         = apvts_.getRawParameterValue("pw_lfo_rate");
    glideModeParam_         = apvts_.getRawParameterValue("glide_mode");
    modASourceParam_        = apvts_.getRawParameterValue("mod_a_source");
    modAAmountParam_        = apvts_.getRawParameterValue("mod_a_amount");
    modBSourceParam_        = apvts_.getRawParameterValue("mod_b_source");
    modBAmountParam_        = apvts_.getRawParameterValue("mod_b_amount");

    randCoreWarpParam_      = apvts_.getRawParameterValue("rand_core_warp");
    randCoreBiteParam_      = apvts_.getRawParameterValue("rand_core_bite");
    randCoreMeltParam_      = apvts_.getRawParameterValue("rand_core_melt");
    randRawStrangleParam_   = apvts_.getRawParameterValue("rand_raw_strangle");
    randSubChewParam_       = apvts_.getRawParameterValue("rand_sub_chew");
    randSubSpitParam_       = apvts_.getRawParameterValue("rand_sub_spit");
    randFilterTwistParam_   = apvts_.getRawParameterValue("rand_filter_twist");
    randFilterStabParam_    = apvts_.getRawParameterValue("rand_filter_stab");
    randFilterOrbitParam_   = apvts_.getRawParameterValue("rand_filter_orbit");

    // Bay 5: VCA & Transients
    ampGateModeParam_       = apvts_.getRawParameterValue("amp_gate_mode");
    ampEnvSnapParam_        = apvts_.getRawParameterValue("amp_env_snap");
    ampEnvDecayParam_       = apvts_.getRawParameterValue("amp_env_decay");
    pitchPunchDepthParam_   = apvts_.getRawParameterValue("pitch_punch_depth");
    pitchPunchTimeParam_    = apvts_.getRawParameterValue("pitch_punch_time");
    ampEnvSustainParam_     = apvts_.getRawParameterValue("amp_env_sustain");
    ampEnvReleaseParam_     = apvts_.getRawParameterValue("amp_env_release");
    postAmpDriveParam_      = apvts_.getRawParameterValue("post_amp_drive");
    ampVelocityParam_       = apvts_.getRawParameterValue("amp_velocity");

    // Bay 6: Master Output
    alphaChorusParam_       = apvts_.getRawParameterValue("alpha_chorus");
    globalWidthParam_       = apvts_.getRawParameterValue("global_width");
    outputLevelParam_       = apvts_.getRawParameterValue("output_level");
    lifeModeParam_          = apvts_.getRawParameterValue("life_mode");
    triggerModeParam_       = apvts_.getRawParameterValue("trigger_mode");
    pitchBendRangeParam_    = apvts_.getRawParameterValue("pitch_bend_range");
    vibratoRateParam_       = apvts_.getRawParameterValue("vibrato_rate");
    vibratoDepthParam_      = apvts_.getRawParameterValue("vibrato_depth");
}

HasturAudioProcessor::~HasturAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout HasturAudioProcessor::createParameterLayout() {
    return hasturParameters::createLayout();
}

void HasturAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/) {
    voice_.prepare(sampleRate);
}

void HasturAudioProcessor::releaseResources() {
    voice_.reset();
}

bool HasturAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void HasturAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    auto* left = (numChannels > 0) ? buffer.getWritePointer(0) : nullptr;
    auto* right = (numChannels > 1) ? buffer.getWritePointer(1) : nullptr;

    // 1. Fetch Parameters once at block start
    const auto lifeMode = static_cast<CrackedChimes::DSP::HasturVoice::LifeMode>(
        std::clamp(static_cast<int>(lifeModeParam_ ? lifeModeParam_->load(std::memory_order_relaxed) : 2), 0, 2));
    voice_.setLifeMode(lifeMode);

    const float bendRange = pitchBendRangeParam_ ? pitchBendRangeParam_->load(std::memory_order_relaxed) : 2.0f;
    const float vibRate   = vibratoRateParam_  ? vibratoRateParam_->load(std::memory_order_relaxed)  : 5.5f;
    const float vibDepth  = vibratoDepthParam_ ? vibratoDepthParam_->load(std::memory_order_relaxed) : 0.0f;
    voice_.setVibrato(vibRate, vibDepth);

    const auto ampGateMode = static_cast<CrackedChimes::DSP::HasturVoice::AmpGateMode>(
        std::clamp(static_cast<int>(ampGateModeParam_ ? ampGateModeParam_->load(std::memory_order_relaxed) : 0), 0, 2));
    const float postAmpDrive = postAmpDriveParam_ ? postAmpDriveParam_->load(std::memory_order_relaxed) : 0.0f;
    const float outputLevel  = outputLevelParam_  ? outputLevelParam_->load(std::memory_order_relaxed) : 0.8f;

    // Bay 1: DCO
    const float osc1Mult         = osc1MultParam_ ? osc1MultParam_->load(std::memory_order_relaxed) : 1.0f;
    const float osc1ApfOct       = osc1ApfPitchTrackParam_ ? osc1ApfPitchTrackParam_->load(std::memory_order_relaxed) : 0.0f;
    const float osc1Drive        = osc1DriveParam_ ? osc1DriveParam_->load(std::memory_order_relaxed) : 0.0f;
    const float dcoBite          = dcoBiteParam_ ? dcoBiteParam_->load(std::memory_order_relaxed) : 0.0f;
    const float subApfDepth      = subApfDepthParam_ ? subApfDepthParam_->load(std::memory_order_relaxed) : 0.0f;
    const float subSpit          = subSpitParam_ ? subSpitParam_->load(std::memory_order_relaxed) : 0.0f;
    const float dcoMorph         = dcoMorphParam_ ? dcoMorphParam_->load(std::memory_order_relaxed) : 0.0f;
    const int   dcoFamily        = dcoWaveFamilyParam_ ? static_cast<int>(dcoWaveFamilyParam_->load(std::memory_order_relaxed)) : 0;
    const int   dcoTopo          = dcoChopperTopoParam_ ? static_cast<int>(dcoChopperTopoParam_->load(std::memory_order_relaxed)) : 0;
    const int   subOctave        = subOctaveParam_ ? (static_cast<int>(subOctaveParam_->load(std::memory_order_relaxed)) - 2) : -1;
    const int   subPulseTrain    = subPulseTrainParam_ ? static_cast<int>(subPulseTrainParam_->load(std::memory_order_relaxed)) : 0;
    const int   subApfTap        = subApfTapParam_ ? static_cast<int>(subApfTapParam_->load(std::memory_order_relaxed)) : 0;

    // Bay 2: Mixer Block
    const float osc1Level        = osc1LevelParam_ ? osc1LevelParam_->load(std::memory_order_relaxed) : 1.0f;
    const float osc2Level        = osc2LevelParam_ ? osc2LevelParam_->load(std::memory_order_relaxed) : 0.8f;
    const float noiseLevel       = noiseLevelParam_ ? noiseLevelParam_->load(std::memory_order_relaxed) : 0.0f;
    const float mixerDrive       = mixerDriveParam_ ? mixerDriveParam_->load(std::memory_order_relaxed) : 0.0f;
    const float masterSpread     = globalWidthParam_ ? globalWidthParam_->load(std::memory_order_relaxed) : 0.0f;
    const float oscSlop          = oscSlopParam_ ? oscSlopParam_->load(std::memory_order_relaxed) : 0.0f;

    // Bay 3: VCF Block & VCF Dynamics
    const float cutoff           = cutoffParam_ ? cutoffParam_->load(std::memory_order_relaxed) : 1200.0f;
    const float resonance        = resonanceParam_ ? resonanceParam_->load(std::memory_order_relaxed) : 0.0f;
    const float filterDecay      = filterEnvDecayParam_ ? filterEnvDecayParam_->load(std::memory_order_relaxed) : 0.400f;
    const float burbleDepth      = burbleDepthParam_ ? burbleDepthParam_->load(std::memory_order_relaxed) : 0.0f;
    const int   burbleMode       = burbleModeParam_ ? static_cast<int>(burbleModeParam_->load(std::memory_order_relaxed)) : 0;
    const float laserDrive       = laserDriveParam_ ? laserDriveParam_->load(std::memory_order_relaxed) : 0.0f;
    const float filterWarp       = filterWarpParam_ ? filterWarpParam_->load(std::memory_order_relaxed) : 0.0f;
    const float laserBite        = laserBiteParam_ ? laserBiteParam_->load(std::memory_order_relaxed) : 0.0f;
    const float filterOrbit      = filterOrbitParam_ ? filterOrbitParam_->load(std::memory_order_relaxed) : 0.0f;
    const int   filterDeck       = filterDeckParam_ ? static_cast<int>(filterDeckParam_->load(std::memory_order_relaxed)) : 0;
    const float envDepth         = envDepthParam_ ? envDepthParam_->load(std::memory_order_relaxed) : 0.5f;
    const float filterVelocity   = filterVelocityParam_ ? filterVelocityParam_->load(std::memory_order_relaxed) : 0.0f;
    const float keyTrack         = keyTrackParam_ ? keyTrackParam_->load(std::memory_order_relaxed) : 0.0f;
    const float filterLfoDepth   = filterLfoDepthParam_ ? filterLfoDepthParam_->load(std::memory_order_relaxed) : 0.0f;
    const float lfoShape         = lfoShapeParam_ ? lfoShapeParam_->load(std::memory_order_relaxed) : 0.5f;
    const int   lfoSync          = lfoSyncParam_ ? static_cast<int>(lfoSyncParam_->load(std::memory_order_relaxed)) : 0;

    // Hardwired Acid Bass Envelopes (Zero Clutter)
    const float filterAttack     = 0.001f;
    const float filterSnap       = 1.0f;
    const float filterBloom      = 0.0f;
    const float filterAftershock = 0.0f;
    const float filterSustain    = 0.0f;
    const float filterRelease    = filterDecay;

    // Bay 4: Global Modulation & Mod Matrix
    const float globalWidth      = masterSpread;
    const float pwLfoRate        = pwLfoRateParam_ ? pwLfoRateParam_->load(std::memory_order_relaxed) : 0.85f;
    const float pwLfoAmount      = pwLfoAmountParam_ ? pwLfoAmountParam_->load(std::memory_order_relaxed) : 0.0f;
    const float widthEnv         = 0.0f;
    const float ampVelocity      = ampVelocityParam_ ? ampVelocityParam_->load(std::memory_order_relaxed) : 0.0f;
    const int   filterLfoDest    = filterLfoDestParam_ ? static_cast<int>(filterLfoDestParam_->load(std::memory_order_relaxed)) : 0;
    const float lfoRate          = lfoRateParam_ ? lfoRateParam_->load(std::memory_order_relaxed) : 2.5f;
    const auto  lfoWave          = static_cast<CrackedChimes::DSP::HasturVoice::LfoWave>(
        std::clamp(static_cast<int>(lfoWaveParam_ ? lfoWaveParam_->load(std::memory_order_relaxed) : 0), 0, 3));
    const auto  lfoTrig          = (lfoSync > 0)
        ? CrackedChimes::DSP::HasturVoice::LfoTriggerMode::ALL_NEW_NOTES
        : CrackedChimes::DSP::HasturVoice::LfoTriggerMode::FREE_RUNNING;
    const float glideSec         = glideTimeParam_ ? glideTimeParam_->load(std::memory_order_relaxed) : 0.005f;
    const auto  glideMode        = (glideModeParam_ && glideModeParam_->load(std::memory_order_relaxed) > 0.5f)
        ? CrackedChimes::DSP::HasturVoice::GlideMode::LEGATO
        : CrackedChimes::DSP::HasturVoice::GlideMode::ALWAYS;

    const int tmIdx = triggerModeParam_ ? static_cast<int>(std::round(triggerModeParam_->load(std::memory_order_relaxed))) : 2;
    CrackedChimes::DSP::HasturVoice::TriggerMode trigMode = (tmIdx == 0) ? CrackedChimes::DSP::HasturVoice::TriggerMode::RETRIG
                                                         : (tmIdx == 1) ? CrackedChimes::DSP::HasturVoice::TriggerMode::LEGATO
                                                                        : CrackedChimes::DSP::HasturVoice::TriggerMode::AUTO;

    int modASrc = modASourceParam_ ? juce::roundToInt(modASourceParam_->load(std::memory_order_relaxed)) : 0;
    CrackedChimes::DSP::ModMatrix::ChannelConfig modA;
    modA.source      = (modASrc == 2) ? CrackedChimes::DSP::ModMatrix::Source::NOISE
                     : (modASrc == 1) ? CrackedChimes::DSP::ModMatrix::Source::CORE
                                      : CrackedChimes::DSP::ModMatrix::Source::RAW;
    modA.type        = CrackedChimes::DSP::ModMatrix::Type::LINEAR_FM;
    modA.amount      = modAAmountParam_ ? modAAmountParam_->load(std::memory_order_relaxed) : 0.0f;
    modA.destination = CrackedChimes::DSP::ModMatrix::Destination::FILTER_CUTOFF;

    int modBSrc = modBSourceParam_ ? juce::roundToInt(modBSourceParam_->load(std::memory_order_relaxed)) : 2;
    CrackedChimes::DSP::ModMatrix::ChannelConfig modB;
    modB.source      = (modBSrc == 2) ? CrackedChimes::DSP::ModMatrix::Source::SUB
                     : (modBSrc == 1) ? CrackedChimes::DSP::ModMatrix::Source::CORE
                                      : CrackedChimes::DSP::ModMatrix::Source::RAW;
    modB.type        = CrackedChimes::DSP::ModMatrix::Type::AM;
    modB.amount      = modBAmountParam_ ? modBAmountParam_->load(std::memory_order_relaxed) : 0.0f;
    modB.destination = CrackedChimes::DSP::ModMatrix::Destination::POST_FILTER_LEVEL;

    // Bay 5: VCA & Transients
    const float ampSnap          = ampEnvSnapParam_ ? ampEnvSnapParam_->load(std::memory_order_relaxed) : 0.0015f;
    const float ampDecay         = ampEnvDecayParam_ ? ampEnvDecayParam_->load(std::memory_order_relaxed) : 0.350f;
    const float ampCurvature     = 0.0f;
    const float ampSustain       = ampEnvSustainParam_ ? ampEnvSustainParam_->load(std::memory_order_relaxed) : 0.50f;
    const float ampRelease       = ampEnvReleaseParam_ ? ampEnvReleaseParam_->load(std::memory_order_relaxed) : 0.100f;
    const float pitchPunchDepth  = pitchPunchDepthParam_ ? pitchPunchDepthParam_->load(std::memory_order_relaxed) : 0.0f;
    const float pitchPunchTime   = pitchPunchTimeParam_ ? pitchPunchTimeParam_->load(std::memory_order_relaxed) : 0.020f;
    const float alphaChorus      = alphaChorusParam_ ? alphaChorusParam_->load(std::memory_order_relaxed) : 0.0f;

    auto amt = [](std::atomic<float>* p) noexcept { return p ? p->load(std::memory_order_relaxed) : 0.0f; };
    const float mOsc1Warp  = stepRandomRange(osc1ApfOct,  -2.0f,  2.0f, amt(randCoreWarpParam_),    voice_.getStepRand(0));
    const float mBite      = stepRandomRange(dcoBite,     -1.0f,  1.0f, amt(randCoreBiteParam_),    voice_.getStepRand(1));
    const float mMelt      = stepRandomRange(osc1Drive,    0.0f,  1.0f, amt(randCoreMeltParam_),    voice_.getStepRand(2));
    const float mChew      = stepRandomRange(subApfDepth, -2.0f,  2.0f, amt(randSubChewParam_),     voice_.getStepRand(3));
    const float mSpit      = stepRandomRange(subSpit,     -1.0f,  1.0f, amt(randSubSpitParam_),     voice_.getStepRand(4));
    const float mStrangle  = stepRandomRange(osc1Mult,     1.0f, 32.0f, amt(randRawStrangleParam_), voice_.getStepRand(5));
    const float mTwist     = stepRandomRange(filterWarp,  -2.0f,  2.0f, amt(randFilterTwistParam_), voice_.getStepRand(6));
    const float mStab      = stepRandomRange(laserBite,   -1.0f,  1.0f, amt(randFilterStabParam_),  voice_.getStepRand(7));
    const float mOrbit     = stepRandomRange(filterOrbit, -1.0f,  1.0f, amt(randFilterOrbitParam_), voice_.getStepRand(8));

    // 2. Factor out MIDI message handling (Sample-Accurate Slicing)
    auto handleMidi = [&](const juce::MidiMessage& msg) noexcept {
        if (msg.isNoteOn()) {
            voice_.noteOn(msg.getNoteNumber(), msg.getFloatVelocity(), glideSec, lfoTrig, true, glideMode, trigMode);
        } else if (msg.isNoteOff()) {
            voice_.noteOff(msg.getNoteNumber());
        } else if (msg.isAllSoundOff()) {
            voice_.reset();
            pitchBendNorm_ = 0.0f;
            voice_.setPitchBend(0.0f, bendRange);
        } else if (msg.isAllNotesOff()) {
            voice_.releaseAll();
        } else if (msg.isPitchWheel()) {
            const int v = msg.getPitchWheelValue();
            pitchBendNorm_ = (v >= 8192) ? static_cast<float>(v - 8192) / 8191.0f : static_cast<float>(v - 8192) / 8192.0f;
            voice_.setPitchBend(pitchBendNorm_, bendRange);
        }
    };

    // 3. Render sub-blocks up to MIDI timestamps
    int samplePos = 0;
    auto renderTo = [&](int targetSample) noexcept {
        for (; samplePos < targetSample; ++samplePos) {
            float sL = 0.0f, sR = 0.0f;
            voice_.processStereoSample(
                dcoFamily, dcoTopo, dcoMorph,
                mStrangle, mOsc1Warp, mMelt,
                mBite,
                0.0f,
                subOctave, subPulseTrain,
                osc1Level, osc2Level, 0.0f, noiseLevel, mixerDrive,
                globalWidth, pwLfoRate, pwLfoAmount, widthEnv,
                lfoRate, lfoWave, lfoTrig, glideSec, oscSlop,
                modA, modB,
                cutoff, resonance, filterDeck, mTwist, mStab, laserDrive,
                mOrbit, 0, envDepth, filterVelocity, keyTrack, filterLfoDepth, 0.0f,
                filterAttack, filterSnap, filterDecay, filterBloom,
                filterAftershock, filterSustain, filterRelease, 0,
                ampSnap, ampDecay, ampCurvature, ampSustain, ampRelease,
                pitchPunchDepth, pitchPunchTime,
                ampGateMode, postAmpDrive, masterSpread, outputLevel,
                sL, sR,
                mChew, subApfTap, burbleDepth, burbleMode, alphaChorus,
                mSpit,
                ampVelocity,
                filterLfoDest,
                lfoShape,
                lfoSync
            );

            if (left != nullptr) left[samplePos] = sL;
            if (right != nullptr) right[samplePos] = sR;
        }
    };

    for (const auto metadata : midiMessages) {
        const int msgPos = std::clamp(metadata.samplePosition, 0, numSamples);
        renderTo(msgPos);
        handleMidi(metadata.getMessage());
    }
    renderTo(numSamples);

    // 4. Sanitize both output channels and auto-heal voice state from NaN blowup
    const bool badL = (left != nullptr) && TbdAudio::FastMath::sanitizeBuffer(left, numSamples);
    const bool badR = (right != nullptr) && TbdAudio::FastMath::sanitizeBuffer(right, numSamples);
    if (badL || badR) {
        voice_.reset();
    }
}

juce::AudioProcessorEditor* HasturAudioProcessor::createEditor() {
    return new HasturAudioProcessorEditor(*this);
}

void HasturAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts_.copyState();
    state.setProperty("ui_lfo_rate_mode", uiLfoRateMode_.load(std::memory_order_relaxed), nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void HasturAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts_.state.getType())) {
        auto tree = juce::ValueTree::fromXml(*xmlState);
        if (tree.hasProperty("ui_lfo_rate_mode")) {
            uiLfoRateMode_.store(static_cast<int>(tree.getProperty("ui_lfo_rate_mode", 0)), std::memory_order_relaxed);
        }

        // Legacy Preset Migration Shim for Sub-Oscillator (consume once, strip from tree):
        if (auto legacy = tree.getChildWithProperty("id", "sub_mode"); legacy.isValid()) {
            const int legacyMode = static_cast<int>(legacy.getProperty("value", -1));
            tree.removeChild(legacy, nullptr);
            auto setChoice = [&](const char* id, int val) {
                if (auto c = tree.getChildWithProperty("id", id); c.isValid()) {
                    c.setProperty("value", val, nullptr);
                } else {
                    juce::ValueTree p("PARAM");
                    p.setProperty("id", id, nullptr);
                    p.setProperty("value", val, nullptr);
                    tree.appendChild(p, nullptr);
                }
            };
            switch (legacyMode) {
                case 0: setChoice("sub_octave", 1); setChoice("sub_pulse_train", 0); break;
                case 1: setChoice("sub_octave", 1); setChoice("sub_pulse_train", 1); break;
                case 2: setChoice("sub_octave", 1); setChoice("sub_pulse_train", 2); break;
                case 3: setChoice("sub_octave", 1); setChoice("sub_pulse_train", 4); break;
                case 4: setChoice("sub_octave", 0); setChoice("sub_pulse_train", 0); break;
                case 5: setChoice("sub_octave", 0); setChoice("sub_pulse_train", 1); break;
                default: break;
            }
        }
        apvts_.replaceState(tree);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new HasturAudioProcessor();
}
