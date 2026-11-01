#include "PlanterProcessor.h"
#include "PlanterEditor.h"
#include "ParameterManager.h"
#include "FastMath.h"
#include "DevLogger.h"

TheKlangPlanterAudioProcessor::TheKlangPlanterAudioProcessor() : KlangCoreProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true), "PARAMETERS", createParameterLayout()) {
    // Cache direct parameter pointers
    carrierTrackingParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("planter_carrier_tracking"));
    carrierPitchParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_carrier_pitch"));
    carrierShapeParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_carrier_shape"));
    carrierDepthParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_carrier_depth"));

    modTrackParam        = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("planter_mod_track"));
    modTypeParam         = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("planter_mod_type"));
    modShapeParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_mod_shape"));
    modSpeedParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_mod_speed"));

    pitchEnvTargetParam  = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("planter_pitchenv_target"));
    pitchEnvSlopeParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_pitchenv_slope"));
    pitchEnvDepthParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_pitchenv_depth"));
    pitchEnvDecayParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_pitchenv_decay"));

    noiseShRateParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_noise_sh_rate"));
    noiseFilterParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_noise_filter"));
    noiseDecayParam      = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_noise_decay"));
    noiseCrossfadeParam  = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_noise_crossfade"));

    filterTypeParam      = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("planter_filter_type"));
    filterSlopeParam     = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("planter_filter_slope"));
    filterCutoffParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_filter_cutoff"));
    filterResoParam      = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_filter_reso"));

    filterEnvSlopeParam  = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_filterenv_slope"));
    filterEnvDepthParam  = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_filterenv_depth"));
    filterEnvDecayParam  = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_filterenv_decay"));
    filterEnvDriveParam  = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_filterenv_drive"));

    ampDriveParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_amp_drive"));
    ampPanParam          = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_amp_pan"));
    ampVelSlopeParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_amp_vel_slope"));
    ampVelFloorParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_amp_vel_floor"));

    ampEnvClapsParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_ampenv_claps"));
    ampEnvClapSpeedParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_ampenv_clapspeed"));
    ampEnvSlopeParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_ampenv_slope"));
    ampEnvDecayParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_ampenv_decay"));
    limiterEnableParam   = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("planter_limiter_enable"));
    limiterGainParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_limiter_gain"));
    limiterThreshParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_limiter_thresh"));
    limiterReleaseParam  = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("planter_limiter_release"));
}

juce::AudioProcessorValueTreeState::ParameterLayout TheKlangPlanterAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto makeFloatParam = [](const juce::String& id, const juce::String& name, float defaultVal) {
        if (auto* def = RlyehSound::ParameterManager::getInstance().getControlDef(id)) {
            return std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID(id, 1), def->name,
                juce::NormalisableRange<float>(def->min, def->max, def->step, def->skew),
                def->defaultFloat);
        }
        return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(id, 1), name, 0.0f, 1.0f, defaultVal);
    };

    auto makeChoiceParam = [](const juce::String& id, const juce::String& name, const juce::StringArray& choices, int defaultIndex) {
        if (auto* def = RlyehSound::ParameterManager::getInstance().getControlDef(id)) {
            return std::make_unique<juce::AudioParameterChoice>(
                juce::ParameterID(id, 1), def->name,
                def->choices, def->defaultChoice);
        }
        return std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID(id, 1), name, choices, defaultIndex);
    };

    // 1. Carrier
    layout.add(makeChoiceParam("planter_carrier_tracking", "Carrier: Tracking", juce::StringArray{ "MIDI", "Freq", "Note" }, 0));
    layout.add(makeFloatParam("planter_carrier_pitch", "Carrier: Pitch", 0.5f));
    layout.add(makeFloatParam("planter_carrier_shape", "Carrier: Shape", 0.0f));
    layout.add(makeFloatParam("planter_carrier_depth", "Carrier: Mod Depth", 0.5f));

    // 2. Modulator
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("planter_mod_track", 1), "Modulator: Tracking",
        juce::StringArray{ "Fixed", "Follow", "FM" }, 2));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("planter_mod_type", 1), "Modulator: Type",
        juce::StringArray{ "Osc", "Cyclic", "Noise" }, 0));
    layout.add(makeFloatParam("planter_mod_shape", "Modulator: Shape", 0.0f));
    layout.add(makeFloatParam("planter_mod_speed", "Modulator: Speed", 0.5f)); // 1:1 ratio

    // 3. Pitch Envelope
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("planter_pitchenv_target", 1), "PitchEnv: Target",
        juce::StringArray{ "Car", "Mod", "Both", "Opp" }, 0));
    layout.add(makeFloatParam("planter_pitchenv_slope", "PitchEnv: Slope", 0.5886f));
    layout.add(makeFloatParam("planter_pitchenv_depth", "PitchEnv: Depth", 0.5f));
    layout.add(makeFloatParam("planter_pitchenv_decay", "PitchEnv: Decay", 0.3806f));

    // 4. Noise Transient (S&H Rate, DJ Filter, Decay, FM/Noise Crossfader)
    layout.add(makeFloatParam("planter_noise_sh_rate", "Noise: S&H Rate", 1.0f));
    layout.add(makeFloatParam("planter_noise_filter", "Noise: DJ Filter", 0.5f));
    layout.add(makeFloatParam("planter_noise_decay", "Noise: Decay", 0.3078f));
    layout.add(makeFloatParam("planter_noise_crossfade", "Noise: FM / Noise", 1.0f)); // default 1.0 = +100% FM

    // 5. Filter
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("planter_filter_type", 1), "Filter: Type",
        juce::StringArray{ "LPF", "BPF", "HPF", "BRF" }, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("planter_filter_slope", 1), "Filter: Slope",
        juce::StringArray{ "6", "12", "18", "24", "36" }, 1));
    layout.add(makeFloatParam("planter_filter_cutoff", "Filter: Cutoff", 1.0f));
    layout.add(makeFloatParam("planter_filter_reso", "Filter: Resonance", 0.0f));

    // 6. Filter Envelope (Knob 4 is Pre-Filter Drive: -6dB to +24dB, 0dB at 0.5)
    layout.add(makeFloatParam("planter_filterenv_slope", "FilterEnv: Slope", 0.5886f));
    layout.add(makeFloatParam("planter_filterenv_depth", "FilterEnv: Depth", 0.5f));
    layout.add(makeFloatParam("planter_filterenv_decay", "FilterEnv: Decay", 0.3806f));
    layout.add(makeFloatParam("planter_filterenv_drive", "FilterEnv: Drive", 0.5f));

    // 7. Amplifier (Drive, Pan, Vel Slope, Vel Floor)
    layout.add(makeFloatParam("planter_amp_drive", "Amp: Drive", 0.5f));
    layout.add(makeFloatParam("planter_amp_pan", "Amp: Pan", 0.5f));
    layout.add(makeFloatParam("planter_amp_vel_slope", "Amp: Vel Slope", 0.75f));
    layout.add(makeFloatParam("planter_amp_vel_floor", "Amp: Velocity", 0.5f));

    // 8. Amp Envelope
    layout.add(makeFloatParam("planter_ampenv_claps", "AmpEnv: Claps", 0.0f));
    layout.add(makeFloatParam("planter_ampenv_clapspeed", "AmpEnv: Clap Speed", 2.0f / 14.0f));
    layout.add(makeFloatParam("planter_ampenv_slope", "AmpEnv: Slope", 0.5886f));
    layout.add(makeFloatParam("planter_ampenv_decay", "AmpEnv: Decay", 0.3806f));
    layout.add(makeChoiceParam("planter_limiter_enable", "Limiter: Enable", juce::StringArray{ "Off", "On" }, 1));
    layout.add(makeFloatParam("planter_limiter_gain", "Limiter: Gain", 12.0f / 36.0f));
    layout.add(makeFloatParam("planter_limiter_thresh", "Limiter: Threshold", 1.0f));
    layout.add(makeFloatParam("planter_limiter_release", "Limiter: Release", 0.6296f));

    return layout;
}

void TheKlangPlanterAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    ::RlyehSound::DevLogger::getInstance().registerAudioThread(std::this_thread::get_id());
    TKS_LOG_INFO("TheKlangPlanterAudioProcessor::prepareToPlay: sampleRate=" + juce::String(sampleRate) + ", samplesPerBlock=" + juce::String(samplesPerBlock));
    engine.init(static_cast<float>(sampleRate));
}

void TheKlangPlanterAudioProcessor::releaseResources() {
    TKS_LOG_INFO("TheKlangPlanterAudioProcessor::releaseResources");
    ::RlyehSound::DevLogger::getInstance().registerAudioThread(std::thread::id());
}

bool TheKlangPlanterAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void TheKlangPlanterAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    TbdAudio::FastMath::enableFTZDAZ();
    const int numSamples = buffer.getNumSamples();

    // 1. Sync DSP parameters
    // Carrier
    if (carrierTrackingParam) engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_CARRIER, 0, static_cast<float>(carrierTrackingParam->getIndex()) / 2.0f);
    if (carrierPitchParam)    engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_CARRIER, 1, carrierPitchParam->get());
    if (carrierShapeParam)    engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_CARRIER, 2, carrierShapeParam->get());
    if (carrierDepthParam)    engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_CARRIER, 3, carrierDepthParam->get());

    // Modulator
    if (modTrackParam) engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_MODULATOR, 0, static_cast<float>(modTrackParam->getIndex()) / 2.0f);
    if (modTypeParam)  engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_MODULATOR, 1, static_cast<float>(modTypeParam->getIndex()) / 2.0f);
    if (modShapeParam) engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_MODULATOR, 2, modShapeParam->get());
    if (modSpeedParam) engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_MODULATOR, 3, modSpeedParam->get());

    // Pitch Envelope
    if (pitchEnvTargetParam) engine.setPitchEnvTarget(pitchEnvTargetParam->getIndex());
    if (pitchEnvSlopeParam)  engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_PITCHENV, 0, pitchEnvSlopeParam->get());
    if (pitchEnvDepthParam)  engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_PITCHENV, 1, pitchEnvDepthParam->get());
    if (pitchEnvDecayParam)  engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_PITCHENV, 2, pitchEnvDecayParam->get());

    // Noise Transient (S&H, DJ Filter, Decay, Crossfade)
    if (noiseShRateParam)    engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_NOISE, 0, noiseShRateParam->get());
    if (noiseFilterParam)    engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_NOISE, 1, noiseFilterParam->get());
    if (noiseDecayParam)     engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_NOISE, 2, noiseDecayParam->get());
    if (noiseCrossfadeParam) engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_NOISE, 3, noiseCrossfadeParam->get());

    // Filter
    if (filterTypeParam)   engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_FILTER, 0, static_cast<float>(filterTypeParam->getIndex()) / 3.0f);
    if (filterSlopeParam)  engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_FILTER, 1, static_cast<float>(filterSlopeParam->getIndex()) / 4.0f);
    if (filterCutoffParam) engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_FILTER, 2, filterCutoffParam->get());
    if (filterResoParam)   engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_FILTER, 3, filterResoParam->get());

    // Filter Envelope (with Pre-Filter Drive at param 3)
    if (filterEnvSlopeParam) engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_FILTERENV, 0, filterEnvSlopeParam->get());
    if (filterEnvDepthParam) engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_FILTERENV, 1, filterEnvDepthParam->get());
    if (filterEnvDecayParam) engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_FILTERENV, 2, filterEnvDecayParam->get());
    if (filterEnvDriveParam) engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_FILTERENV, 3, filterEnvDriveParam->get());

    // Amp (Drive, Pan, Vel Slope, Vel Floor)
    if (ampDriveParam)    engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_AMP, 0, ampDriveParam->get());
    if (ampPanParam)      engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_AMP, 1, ampPanParam->get());
    if (ampVelSlopeParam) engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_AMP, 2, ampVelSlopeParam->get());
    if (ampVelFloorParam) engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_AMP, 3, ampVelFloorParam->get());

    // Amp Envelope
    if (ampEnvClapsParam)     engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_AMPENV, 0, ampEnvClapsParam->get());
    if (ampEnvClapSpeedParam) engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_AMPENV, 1, ampEnvClapSpeedParam->get());
    if (ampEnvSlopeParam)     engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_AMPENV, 2, ampEnvSlopeParam->get());
        if (ampEnvDecayParam)     engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_AMPENV, 3, ampEnvDecayParam->get());

    // Limiter
    if (limiterEnableParam)   engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_LIMITER, 0, static_cast<float>(limiterEnableParam->getIndex()));
    if (limiterGainParam)     engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_LIMITER, 1, limiterGainParam->get());
    if (limiterThreshParam)   engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_LIMITER, 2, limiterThreshParam->get());
    if (limiterReleaseParam)  engine.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_LIMITER, 3, limiterReleaseParam->get());

    // 2. Process MIDI events
    for (const auto metadata : midiMessages) {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn()) {
            engine.noteOn(msg.getNoteNumber(), msg.getFloatVelocity());
        } else if (msg.isNoteOff()) {
            engine.noteOff(msg.getNoteNumber());
        }
    }

    // 3. Process Audio
    float* left  = buffer.getNumChannels() > 0 ? buffer.getWritePointer(0) : nullptr;
    float* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : left;

    engine.processStereo(left, right, numSamples);

    // Monitor Saver Protocol: Silences any NaN/Inf corrupted frames
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        TbdAudio::FastMath::sanitizeBuffer(buffer.getWritePointer(ch), numSamples);
    }
}

TheKlangPlanterAudioProcessor::ParamModulationInfo TheKlangPlanterAudioProcessor::getParamModulationInfo(const juce::String& /*paramId*/) const {
    ParamModulationInfo info;
    info.isModulated = false;
    return info;
}

#if TK_IS_THE_SHARD
#include "ShardEditor.h"
juce::AudioProcessorEditor* TheKlangPlanterAudioProcessor::createEditor() {
    return new TheShardAudioProcessorEditor(*this);
}
#else
juce::AudioProcessorEditor* TheKlangPlanterAudioProcessor::createEditor() {
    return new TheKlangPlanterAudioProcessorEditor(*this);
}
#endif

bool TheKlangPlanterAudioProcessor::hasEditor() const {
    return true;
}

// JUCE Plugin Entry Point for The Klang Planter
#ifndef TKF_GUI_TESTS
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new TheKlangPlanterAudioProcessor();
}
#endif





