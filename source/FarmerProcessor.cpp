#include "FarmerProcessor.h"
#include "FarmerEditor.h"
#include "ParameterManager.h"
#include "FastMath.h"
#include "DevLogger.h"

TheKlangFarmerAudioProcessor::TheKlangFarmerAudioProcessor()
    : KlangCoreProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true).withOutput("Output", juce::AudioChannelSet::stereo(), true), "Parameters", createParameterLayout())
{


    // Retrieve direct raw parameter pointers
    // 1. Carrier 1
    carrier1TrackingParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("carrier1_tracking"));
    carrier1PitchParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("carrier1_pitch"));
    carrier1ShapeParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("carrier1_shape"));
    carrier1DepthParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("carrier1_depth"));

    // 2. Modulator 1
    mod1TrackParam        = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("mod1_track"));
    mod1TypeParam         = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("mod1_type"));
    mod1ShapeParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mod1_shape"));
    mod1SpeedParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mod1_speed"));

    // 3. Pitch Envelope 1
    pitchEnv1TargetParam  = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("pitchenv1_target"));
    pitchEnv1SlopeParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("pitchenv1_slope"));
    pitchEnv1DepthParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("pitchenv1_depth"));
    pitchEnv1DecayParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("pitchenv1_decay"));

    // 4. Carrier 2
    carrier2TrackingParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("carrier2_tracking"));
    carrier2PitchParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("carrier2_pitch"));
    carrier2ShapeParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("carrier2_shape"));
    carrier2DepthParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("carrier2_depth"));

    // 5. Modulator 2
    mod2TrackParam        = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("mod2_track"));
    mod2TypeParam         = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("mod2_type"));
    mod2ShapeParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mod2_shape"));
    mod2SpeedParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mod2_speed"));

    // 6. Pitch Envelope 2
    pitchEnv2TargetParam  = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("pitchenv2_target"));
    pitchEnv2SlopeParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("pitchenv2_slope"));
    pitchEnv2DepthParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("pitchenv2_depth"));
    pitchEnv2DecayParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("pitchenv2_decay"));

    // 7. Noise Transient
    noiseShRateParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("noise_sh_rate"));
    noiseFilterParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("noise_filter"));
    noiseDriveParam      = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("noise_drive"));
    noiseDecayParam      = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("noise_decay"));

    // 8. Mixer
    mixerCarrier1LevelParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mixer_carrier1_level"));
    mixerCarrier2LevelParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mixer_carrier2_level"));
    mixerRingModParam       = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mixer_ringmod"));
    mixerNoiseLevelParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mixer_noise_level"));

    // 9. Drive

    // Voice 1 Filter & Env
    filter1TypeParam      = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("filter1_type"));
    filter1SlopeParam     = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("filter1_slope"));
    filter1CutoffParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filter1_cutoff"));
    filter1ResonanceParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filter1_resonance"));

    filterEnv1SlopeParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv1_slope"));
    filterEnv1DepthParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv1_depth"));
    filterEnv1DecayParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv1_decay"));
    filterEnv1PostDriveParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv1_postdrive"));

    // Voice 2 Filter & Env
    filter2TypeParam      = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("filter2_type"));
    filter2SlopeParam     = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("filter2_slope"));
    filter2CutoffParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filter2_cutoff"));
    filter2ResonanceParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filter2_resonance"));

    filterEnv2SlopeParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv2_slope"));
    filterEnv2DepthParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv2_depth"));
    filterEnv2DecayParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv2_decay"));
    filterEnv2PostDriveParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv2_postdrive"));

    // Transients Filter & Env
    filter3TypeParam      = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("filter3_type"));
    filter3SlopeParam     = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("filter3_slope"));
    filter3CutoffParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filter3_cutoff"));
    filter3ResonanceParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filter3_resonance"));

    filterEnv3SlopeParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv3_slope"));
    filterEnv3DepthParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv3_depth"));
    filterEnv3DecayParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv3_decay"));
    filterEnv3PostDriveParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv3_postdrive"));

    // Standalone FX Filter

    // Limiters
    preLimiterEnableParam   = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("pre_limiter_enable"));
    preLimiterGainParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("pre_limiter_gain"));
    preLimiterThreshParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("pre_limiter_thresh"));
    preLimiterReleaseParam  = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("pre_limiter_release"));

    postLimiterEnableParam  = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("post_limiter_enable"));
    postLimiterGainParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("post_limiter_gain"));
    postLimiterThreshParam  = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("post_limiter_thresh"));
    postLimiterReleaseParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("post_limiter_release"));

    // FX Pickers
    preFX1TypeParam  = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("pre_fx_1_type"));
    preFX2TypeParam  = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("pre_fx_2_type"));
    preFX3TypeParam  = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("pre_fx_3_type"));
    preFX4TypeParam  = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("pre_fx_4_type"));

    postFX1TypeParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("post_fx_1_type"));
    postFX2TypeParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("post_fx_2_type"));
    postFX3TypeParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("post_fx_3_type"));
    postFX4TypeParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("post_fx_4_type"));

    // 12. Wave Folder

    // 13. RingMod

    // 14. Frequency Shifter

    // 15. Grit FX

    // 16. Comb Filter

    // 17. PhaseSmear

    // 18. EQ (bell EQ)

    // 19. Amp
    ampLevelParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("amp_level"));
    ampPanParam          = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("amp_pan"));
    ampDriveParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("amp_drive"));
    ampLimiterParam      = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("amp_limiter"));

    // 20. Amp Envelope
    ampEnvClapsParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("ampenv_claps"));
    ampEnvClapSpeedParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("ampenv_clapspeed"));
    ampEnvSlopeParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("ampenv_slope"));
    ampEnvDecayParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("ampenv_decay"));

    // 21. Velocity
    velSlopeParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("vel_slope"));
    velDepthParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("vel_depth"));
    velDecayParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("vel_decay"));
    velVolumeParam       = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("vel_volume"));

    // 22. Key Tracking
    keySlopeParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("key_slope"));
    keyDepthParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("key_depth"));
    keyDecayParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("key_decay"));
    keyVolumeParam       = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("key_volume"));

    // 23. Slop
    slopFreqParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("slop_freq"));
    slopDepthParam       = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("slop_depth"));
    slopDecayParam       = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("slop_decay"));
    slopPanParam         = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("slop_pan"));

    // 24. Mod Envelopes 1..3
    modEnv1SlopeParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("modenv1_slope"));
    modEnv1DepthParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("modenv1_depth"));
    modEnv1DecayParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("modenv1_decay"));
    modEnv1TargetParam   = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("modenv1_target"));

    modEnv2SlopeParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("modenv2_slope"));
    modEnv2DepthParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("modenv2_depth"));
    modEnv2DecayParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("modenv2_decay"));
    modEnv2TargetParam   = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("modenv2_target"));

    modEnv3SlopeParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("modenv3_slope"));
    modEnv3DepthParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("modenv3_depth"));
    modEnv3DecayParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("modenv3_decay"));
    modEnv3TargetParam   = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("modenv3_target"));

    for (int s = 0; s < 4; ++s) {
        for (int p = 0; p < 4; ++p) {
            preFXParam[s][p] = dynamic_cast<juce::AudioParameterFloat*>(
                apvts.getParameter("pre_fx_" + juce::String(s + 1) + "_p" + juce::String(p + 1)));
            postFXParam[s][p] = dynamic_cast<juce::AudioParameterFloat*>(
                apvts.getParameter("post_fx_" + juce::String(s + 1) + "_p" + juce::String(p + 1)));
        }
    }

    for (int i = 0; i < 4; ++i) {
        macroParams[i] = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("macro_" + juce::String(i + 1)));
    }

    continuousParams.clear();
    continuousParams.reserve(getModDestinations().size() + 4);
    for (const auto& d : getModDestinations()) {
        continuousParams.push_back(dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(d.id)));
    }
    for (int i = 0; i < 4; ++i) {
        if (macroParams[i]) continuousParams.push_back(macroParams[i]);
    }
}

void TheKlangFarmerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    ::RlyehSound::DevLogger::getInstance().registerAudioThread(std::this_thread::get_id());
    TKS_LOG_INFO("TheKlangFarmerAudioProcessor::prepareToPlay: sampleRate=" + juce::String(sampleRate) + ", samplesPerBlock=" + juce::String(samplesPerBlock));
    engine.init(static_cast<float>(sampleRate), samplesPerBlock);
}

void TheKlangFarmerAudioProcessor::releaseResources() {
    TKS_LOG_INFO("TheKlangFarmerAudioProcessor::releaseResources");
    ::RlyehSound::DevLogger::getInstance().registerAudioThread(std::thread::id());
}

juce::String TheKlangFarmerAudioProcessor::getFXParamDisplayName(bool isPost, int slotIndex, int fxType, int paramIndex) {
    juce::String prefix = isPost ? "Post FX " : "Pre FX ";
    prefix += juce::String(slotIndex + 1);

    struct FXInfo {
        const char* name;
        const char* p[4];
    };
    static const FXInfo infos[14] = {
        { "Empty",        { "Empty", "Empty", "Empty", "Empty" } },
        { "Bell EQ",      { "Freq", "Width", "Gain", "DJ Filter" } },
        { "Chorus",       { "Rate", "Depth", "Feedback", "Mix" } },
        { "Comb Filter",  { "Dampening", "Cutoff", "Resonance", "Mix" } },
        { "Phase Smear",  { "Order", "Amount", "Cutoff", "Resonance" } },
        { "Drive",        { "Drive", "Bias", "Filter", "Limiter" } },
        { "FX Filter",    { "Type", "Slope", "Cutoff", "Resonance" } },
        { "Flanger",      { "Rate", "Depth", "Feedback", "Mix" } },
        { "Freq Shift",   { "Shift", "Range", "Blend", "Width" } },
        { "Grit FX",      { "Bit Rate", "Sample Rate", "Low", "High" } },
        { "Phaser",       { "Rate", "Depth", "Feedback", "Mix" } },
        { "RingMod",      { "Waveform", "Rate", "Amount", "Width" } },
        { "Tempo Delay",  { "Division", "Feedback", "Tone", "Mix" } },
        { "Wave Folder",  { "Enable", "Fold", "Bias", "Filter" } }
    };

    int typeIdx = (fxType >= 0 && fxType <= 13) ? fxType : 0;
    int pIdx = (paramIndex >= 0 && paramIndex < 4) ? paramIndex : 0;
    const auto& info = infos[typeIdx];

    return prefix + " [" + info.name + "]: Param " + juce::String(pIdx + 1) + " [" + info.p[pIdx] + "]";
}

const std::vector<TheKlangFarmerAudioProcessor::ModDestDescriptor>& TheKlangFarmerAudioProcessor::getModDestinations() {
    using MDE = TbdAudio::ModularDrumEngine;
    static const std::vector<ModDestDescriptor> destinations = []() {
        std::vector<ModDestDescriptor> list = {
            // Voice 1 (14 params)
            { "carrier1_pitch", "Carrier 1: Pitch / Freq", ModTargetType::PageBlock, MDE::BLK_CARRIER1, 1 },
            { "carrier1_shape", "Carrier 1: Shape", ModTargetType::PageBlock, MDE::BLK_CARRIER1, 2 },
            { "carrier1_depth", "Carrier 1: Modulation Depth", ModTargetType::PageBlock, MDE::BLK_CARRIER1, 3 },
            { "mod1_shape", "Modulator 1: Shape", ModTargetType::PageBlock, MDE::BLK_MODULATOR1, 2 },
            { "mod1_speed", "Modulator 1: Speed", ModTargetType::PageBlock, MDE::BLK_MODULATOR1, 3 },
            { "pitchenv1_slope", "PitchEnv 1: Slope", ModTargetType::PageBlock, MDE::BLK_PITCHENV1, 0 },
            { "pitchenv1_depth", "PitchEnv 1: Depth", ModTargetType::PageBlock, MDE::BLK_PITCHENV1, 1 },
            { "pitchenv1_decay", "PitchEnv 1: Decay", ModTargetType::PageBlock, MDE::BLK_PITCHENV1, 2 },
            { "filter1_cutoff", "Filter 1: Cutoff", ModTargetType::PageBlock, MDE::BLK_FILTER1, 2 },
            { "filter1_resonance", "Filter 1: Resonance", ModTargetType::PageBlock, MDE::BLK_FILTER1, 3 },
            { "filterenv1_slope", "FilterEnv 1: Slope", ModTargetType::PageBlock, MDE::BLK_FILTERENV1, 0 },
            { "filterenv1_depth", "FilterEnv 1: Depth", ModTargetType::PageBlock, MDE::BLK_FILTERENV1, 1 },
            { "filterenv1_decay", "FilterEnv 1: Decay", ModTargetType::PageBlock, MDE::BLK_FILTERENV1, 2 },
            { "filterenv1_postdrive", "FilterEnv 1: Post-Drive", ModTargetType::PageBlock, MDE::BLK_FILTERENV1, 3 },

            // Voice 2 (14 params)
            { "carrier2_pitch", "Carrier 2: Pitch / Freq", ModTargetType::PageBlock, MDE::BLK_CARRIER2, 1 },
            { "carrier2_shape", "Carrier 2: Shape", ModTargetType::PageBlock, MDE::BLK_CARRIER2, 2 },
            { "carrier2_depth", "Carrier 2: Modulation Depth", ModTargetType::PageBlock, MDE::BLK_CARRIER2, 3 },
            { "mod2_shape", "Modulator 2: Shape", ModTargetType::PageBlock, MDE::BLK_MODULATOR2, 2 },
            { "mod2_speed", "Modulator 2: Speed", ModTargetType::PageBlock, MDE::BLK_MODULATOR2, 3 },
            { "pitchenv2_slope", "PitchEnv 2: Slope", ModTargetType::PageBlock, MDE::BLK_PITCHENV2, 0 },
            { "pitchenv2_depth", "PitchEnv 2: Depth", ModTargetType::PageBlock, MDE::BLK_PITCHENV2, 1 },
            { "pitchenv2_decay", "PitchEnv 2: Decay", ModTargetType::PageBlock, MDE::BLK_PITCHENV2, 2 },
            { "filter2_cutoff", "Filter 2: Cutoff", ModTargetType::PageBlock, MDE::BLK_FILTER2, 2 },
            { "filter2_resonance", "Filter 2: Resonance", ModTargetType::PageBlock, MDE::BLK_FILTER2, 3 },
            { "filterenv2_slope", "FilterEnv 2: Slope", ModTargetType::PageBlock, MDE::BLK_FILTERENV2, 0 },
            { "filterenv2_depth", "FilterEnv 2: Depth", ModTargetType::PageBlock, MDE::BLK_FILTERENV2, 1 },
            { "filterenv2_decay", "FilterEnv 2: Decay", ModTargetType::PageBlock, MDE::BLK_FILTERENV2, 2 },
            { "filterenv2_postdrive", "FilterEnv 2: Post-Drive", ModTargetType::PageBlock, MDE::BLK_FILTERENV2, 3 },

            // Transients & Mixer (14 params)
            { "noise_sh_rate", "Noise: S&H Rate", ModTargetType::PageBlock, MDE::BLK_NOISE, 0 },
            { "noise_filter", "Noise: Filter", ModTargetType::PageBlock, MDE::BLK_NOISE, 1 },
            { "noise_drive", "Noise: Drive", ModTargetType::PageBlock, MDE::BLK_NOISE, 2 },
            { "noise_decay", "Noise: Decay", ModTargetType::PageBlock, MDE::BLK_NOISE, 3 },
            { "filter3_cutoff", "Filter 3: Cutoff", ModTargetType::PageBlock, MDE::BLK_FILTER3, 2 },
            { "filter3_resonance", "Filter 3: Resonance", ModTargetType::PageBlock, MDE::BLK_FILTER3, 3 },
            { "filterenv3_slope", "FilterEnv 3: Slope", ModTargetType::PageBlock, MDE::BLK_FILTERENV3, 0 },
            { "filterenv3_depth", "FilterEnv 3: Depth", ModTargetType::PageBlock, MDE::BLK_FILTERENV3, 1 },
            { "filterenv3_decay", "FilterEnv 3: Decay", ModTargetType::PageBlock, MDE::BLK_FILTERENV3, 2 },
            { "filterenv3_postdrive", "FilterEnv 3: Post-Drive", ModTargetType::PageBlock, MDE::BLK_FILTERENV3, 3 },
            { "mixer_carrier1_level", "Mixer: Carrier 1 Level", ModTargetType::PageBlock, MDE::BLK_MIXER, 0 },
            { "mixer_carrier2_level", "Mixer: Carrier 2 Level", ModTargetType::PageBlock, MDE::BLK_MIXER, 1 },
            { "mixer_ringmod", "Mixer: Ring Mod Level", ModTargetType::PageBlock, MDE::BLK_MIXER, 2 },
            { "mixer_noise_level", "Mixer: Noise Level", ModTargetType::PageBlock, MDE::BLK_MIXER, 3 },

            // Limiters (6 params)
            { "pre_limiter_gain", "Pre-Limiter: Input Gain", ModTargetType::PageBlock, MDE::BLK_PRE_LIMITER, 1 },
            { "pre_limiter_thresh", "Pre-Limiter: Threshold", ModTargetType::PageBlock, MDE::BLK_PRE_LIMITER, 2 },
            { "pre_limiter_release", "Pre-Limiter: Release", ModTargetType::PageBlock, MDE::BLK_PRE_LIMITER, 3 },
            { "post_limiter_gain", "Post-Limiter: Input Gain", ModTargetType::PageBlock, MDE::BLK_POST_LIMITER, 1 },
            { "post_limiter_thresh", "Post-Limiter: Threshold", ModTargetType::PageBlock, MDE::BLK_POST_LIMITER, 2 },
            { "post_limiter_release", "Post-Limiter: Release", ModTargetType::PageBlock, MDE::BLK_POST_LIMITER, 3 }
        };

        // 16 Pre FX parameters
        static const char* preIds[4][4] = {
            { "pre_fx_1_p1", "pre_fx_1_p2", "pre_fx_1_p3", "pre_fx_1_p4" },
            { "pre_fx_2_p1", "pre_fx_2_p2", "pre_fx_2_p3", "pre_fx_2_p4" },
            { "pre_fx_3_p1", "pre_fx_3_p2", "pre_fx_3_p3", "pre_fx_3_p4" },
            { "pre_fx_4_p1", "pre_fx_4_p2", "pre_fx_4_p3", "pre_fx_4_p4" }
        };
        static const char* preNames[4][4] = {
            { "Pre FX 1: Param 1", "Pre FX 1: Param 2", "Pre FX 1: Param 3", "Pre FX 1: Param 4" },
            { "Pre FX 2: Param 1", "Pre FX 2: Param 2", "Pre FX 2: Param 3", "Pre FX 2: Param 4" },
            { "Pre FX 3: Param 1", "Pre FX 3: Param 2", "Pre FX 3: Param 3", "Pre FX 3: Param 4" },
            { "Pre FX 4: Param 1", "Pre FX 4: Param 2", "Pre FX 4: Param 3", "Pre FX 4: Param 4" }
        };
        for (int s = 0; s < 4; ++s) {
            for (int p = 0; p < 4; ++p) {
                list.push_back({ preIds[s][p], preNames[s][p], ModTargetType::PreFX, s, p });
            }
        }

        // 16 Post FX parameters
        static const char* postIds[4][4] = {
            { "post_fx_1_p1", "post_fx_1_p2", "post_fx_1_p3", "post_fx_1_p4" },
            { "post_fx_2_p1", "post_fx_2_p2", "post_fx_2_p3", "post_fx_2_p4" },
            { "post_fx_3_p1", "post_fx_3_p2", "post_fx_3_p3", "post_fx_3_p4" },
            { "post_fx_4_p1", "post_fx_4_p2", "post_fx_4_p3", "post_fx_4_p4" }
        };
        static const char* postNames[4][4] = {
            { "Post FX 1: Param 1", "Post FX 1: Param 2", "Post FX 1: Param 3", "Post FX 1: Param 4" },
            { "Post FX 2: Param 1", "Post FX 2: Param 2", "Post FX 2: Param 3", "Post FX 2: Param 4" },
            { "Post FX 3: Param 1", "Post FX 3: Param 2", "Post FX 3: Param 3", "Post FX 3: Param 4" },
            { "Post FX 4: Param 1", "Post FX 4: Param 2", "Post FX 4: Param 3", "Post FX 4: Param 4" }
        };
        for (int s = 0; s < 4; ++s) {
            for (int p = 0; p < 4; ++p) {
                list.push_back({ postIds[s][p], postNames[s][p], ModTargetType::PostFX, s, p });
            }
        }

        // Amplifier & Modulations (28 params)
        std::vector<ModDestDescriptor> remaining = {
            { "amp_level", "Amp: Level", ModTargetType::PageBlock, MDE::BLK_AMP, 0 },
            { "amp_pan", "Amp: Pan", ModTargetType::PageBlock, MDE::BLK_AMP, 1 },
            { "amp_drive", "Amp: Drive", ModTargetType::PageBlock, MDE::BLK_AMP, 2 },
            { "ampenv_claps", "AmpEnv: Claps", ModTargetType::PageBlock, MDE::BLK_AMPENV, 0 },
            { "ampenv_clapspeed", "AmpEnv: Clap Speed", ModTargetType::PageBlock, MDE::BLK_AMPENV, 1 },
            { "ampenv_slope", "AmpEnv: Slope", ModTargetType::PageBlock, MDE::BLK_AMPENV, 2 },
            { "ampenv_decay", "AmpEnv: Decay", ModTargetType::PageBlock, MDE::BLK_AMPENV, 3 },
            { "vel_slope", "Velocity: Slope", ModTargetType::PageBlock, MDE::BLK_VELOCITY, 0 },
            { "vel_depth", "Velocity: Depth", ModTargetType::PageBlock, MDE::BLK_VELOCITY, 1 },
            { "vel_decay", "Velocity: Decay", ModTargetType::PageBlock, MDE::BLK_VELOCITY, 2 },
            { "vel_volume", "Velocity: Volume", ModTargetType::PageBlock, MDE::BLK_VELOCITY, 3 },
            { "key_slope", "KeyTrack: Slope", ModTargetType::PageBlock, MDE::BLK_KEYTRACK, 0 },
            { "key_depth", "KeyTrack: Depth", ModTargetType::PageBlock, MDE::BLK_KEYTRACK, 1 },
            { "key_decay", "KeyTrack: Decay", ModTargetType::PageBlock, MDE::BLK_KEYTRACK, 2 },
            { "key_volume", "KeyTrack: Volume", ModTargetType::PageBlock, MDE::BLK_KEYTRACK, 3 },
            { "slop_freq", "Slop: Frequency", ModTargetType::PageBlock, MDE::BLK_SLOP, 0 },
            { "slop_depth", "Slop: Depth", ModTargetType::PageBlock, MDE::BLK_SLOP, 1 },
            { "slop_decay", "Slop: Decay", ModTargetType::PageBlock, MDE::BLK_SLOP, 2 },
            { "slop_pan", "Slop: Pan", ModTargetType::PageBlock, MDE::BLK_SLOP, 3 },
            { "modenv1_slope", "ModEnv 1: Slope", ModTargetType::PageBlock, MDE::BLK_MODENV1, 0 },
            { "modenv1_depth", "ModEnv 1: Depth", ModTargetType::PageBlock, MDE::BLK_MODENV1, 1 },
            { "modenv1_decay", "ModEnv 1: Decay", ModTargetType::PageBlock, MDE::BLK_MODENV1, 2 },
            { "modenv2_slope", "ModEnv 2: Slope", ModTargetType::PageBlock, MDE::BLK_MODENV2, 0 },
            { "modenv2_depth", "ModEnv 2: Depth", ModTargetType::PageBlock, MDE::BLK_MODENV2, 1 },
            { "modenv2_decay", "ModEnv 2: Decay", ModTargetType::PageBlock, MDE::BLK_MODENV2, 2 },
            { "modenv3_slope", "ModEnv 3: Slope", ModTargetType::PageBlock, MDE::BLK_MODENV3, 0 },
            { "modenv3_depth", "ModEnv 3: Depth", ModTargetType::PageBlock, MDE::BLK_MODENV3, 1 },
            { "modenv3_decay", "ModEnv 3: Decay", ModTargetType::PageBlock, MDE::BLK_MODENV3, 2 }
        };

        for (auto& item : remaining) list.push_back(item);
        return list;
    }();
    return destinations;
}

juce::StringArray TheKlangFarmerAudioProcessor::getModDestinationChoices() {
    juce::StringArray choices;
    choices.add("None");
    for (const auto& d : getModDestinations()) {
        if (d.type == ModTargetType::PreFX) {
            static const int defPre[4] = { 5, 13, 11, 8 };
            choices.add(getFXParamDisplayName(false, d.blockOrSlot, defPre[d.blockOrSlot], d.paramIndex));
        } else if (d.type == ModTargetType::PostFX) {
            static const int defPost[4] = { 9, 3, 4, 1 };
            choices.add(getFXParamDisplayName(true, d.blockOrSlot, defPost[d.blockOrSlot], d.paramIndex));
        } else {
            choices.add(d.name);
        }
    }
    return choices;
}

bool TheKlangFarmerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void TheKlangFarmerAudioProcessor::applyBaseParameters() {
    auto getNorm = [](juce::AudioParameterFloat* p) {
        return p ? p->range.convertTo0to1(p->get()) : 0.0f;
    };

    // 1. Carrier 1
    if (carrier1TrackingParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER1, 0, static_cast<float>(carrier1TrackingParam->getIndex()) / 2.0f);
    if (carrier1PitchParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER1, 1, getNorm(carrier1PitchParam));
    if (carrier1ShapeParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER1, 2, getNorm(carrier1ShapeParam));
    if (carrier1DepthParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER1, 3, getNorm(carrier1DepthParam));

    // 2. Modulator 1
    if (mod1TrackParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODULATOR1, 0, static_cast<float>(mod1TrackParam->getIndex()) / 2.0f);
    if (mod1TypeParam)         engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODULATOR1, 1, static_cast<float>(mod1TypeParam->getIndex()) / 2.0f);
    if (mod1ShapeParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODULATOR1, 2, getNorm(mod1ShapeParam));
    if (mod1SpeedParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODULATOR1, 3, getNorm(mod1SpeedParam));

    // 3. Pitch Envelope 1
    if (pitchEnv1SlopeParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV1, 0, getNorm(pitchEnv1SlopeParam));
    if (pitchEnv1DepthParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV1, 1, getNorm(pitchEnv1DepthParam));
    if (pitchEnv1DecayParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV1, 2, getNorm(pitchEnv1DecayParam));
    if (pitchEnv1TargetParam)  engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV1, 3, static_cast<float>(pitchEnv1TargetParam->getIndex()) / 3.0f);

    // 4. Carrier 2
    if (carrier2TrackingParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER2, 0, static_cast<float>(carrier2TrackingParam->getIndex()) / 2.0f);
    if (carrier2PitchParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER2, 1, getNorm(carrier2PitchParam));
    if (carrier2ShapeParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER2, 2, getNorm(carrier2ShapeParam));
    if (carrier2DepthParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER2, 3, getNorm(carrier2DepthParam));

    // 5. Modulator 2
    if (mod2TrackParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODULATOR2, 0, static_cast<float>(mod2TrackParam->getIndex()) / 2.0f);
    if (mod2TypeParam)         engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODULATOR2, 1, static_cast<float>(mod2TypeParam->getIndex()) / 2.0f);
    if (mod2ShapeParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODULATOR2, 2, getNorm(mod2ShapeParam));
    if (mod2SpeedParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODULATOR2, 3, getNorm(mod2SpeedParam));

    // 6. Pitch Envelope 2
    if (pitchEnv2SlopeParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV2, 0, getNorm(pitchEnv2SlopeParam));
    if (pitchEnv2DepthParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV2, 1, getNorm(pitchEnv2DepthParam));
    if (pitchEnv2DecayParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV2, 2, getNorm(pitchEnv2DecayParam));
    if (pitchEnv2TargetParam)  engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV2, 3, static_cast<float>(pitchEnv2TargetParam->getIndex()) / 3.0f);

    // 7. Noise Transient
    if (noiseShRateParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_NOISE, 0, getNorm(noiseShRateParam));
    if (noiseFilterParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_NOISE, 1, getNorm(noiseFilterParam));
    if (noiseDriveParam)      engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_NOISE, 2, getNorm(noiseDriveParam));
    if (noiseDecayParam)      engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_NOISE, 3, getNorm(noiseDecayParam));

    // 8. Mixer
    if (mixerCarrier1LevelParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 0, getNorm(mixerCarrier1LevelParam));
    if (mixerCarrier2LevelParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 1, getNorm(mixerCarrier2LevelParam));
    if (mixerRingModParam)       engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 2, getNorm(mixerRingModParam));
    if (mixerNoiseLevelParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 3, getNorm(mixerNoiseLevelParam));

    // 9. Drive

    // Voice 1 Filter & Env
    if (filter1TypeParam)      engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER1, 0, static_cast<float>(filter1TypeParam->getIndex()) / 3.0f);
    if (filter1SlopeParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER1, 1, static_cast<float>(filter1SlopeParam->getIndex()) / 4.0f);
    if (filter1CutoffParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER1, 2, getNorm(filter1CutoffParam));
    if (filter1ResonanceParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER1, 3, getNorm(filter1ResonanceParam));

    if (filterEnv1SlopeParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV1, 0, getNorm(filterEnv1SlopeParam));
    if (filterEnv1DepthParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV1, 1, getNorm(filterEnv1DepthParam));
    if (filterEnv1DecayParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV1, 2, getNorm(filterEnv1DecayParam));
    if (filterEnv1PostDriveParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV1, 3, getNorm(filterEnv1PostDriveParam));

    // Voice 2 Filter & Env
    if (filter2TypeParam)      engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER2, 0, static_cast<float>(filter2TypeParam->getIndex()) / 3.0f);
    if (filter2SlopeParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER2, 1, static_cast<float>(filter2SlopeParam->getIndex()) / 4.0f);
    if (filter2CutoffParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER2, 2, getNorm(filter2CutoffParam));
    if (filter2ResonanceParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER2, 3, getNorm(filter2ResonanceParam));

    if (filterEnv2SlopeParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV2, 0, getNorm(filterEnv2SlopeParam));
    if (filterEnv2DepthParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV2, 1, getNorm(filterEnv2DepthParam));
    if (filterEnv2DecayParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV2, 2, getNorm(filterEnv2DecayParam));
    if (filterEnv2PostDriveParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV2, 3, getNorm(filterEnv2PostDriveParam));

    // Transients Filter & Env
    if (filter3TypeParam)      engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER3, 0, static_cast<float>(filter3TypeParam->getIndex()) / 3.0f);
    if (filter3SlopeParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER3, 1, static_cast<float>(filter3SlopeParam->getIndex()) / 4.0f);
    if (filter3CutoffParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER3, 2, getNorm(filter3CutoffParam));
    if (filter3ResonanceParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER3, 3, getNorm(filter3ResonanceParam));

    if (filterEnv3SlopeParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV3, 0, getNorm(filterEnv3SlopeParam));
    if (filterEnv3DepthParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV3, 1, getNorm(filterEnv3DepthParam));
    if (filterEnv3DecayParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV3, 2, getNorm(filterEnv3DecayParam));
    if (filterEnv3PostDriveParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV3, 3, getNorm(filterEnv3PostDriveParam));

    // Standalone FX Filter

    // Limiters
    if (preLimiterEnableParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PRE_LIMITER, 0, static_cast<float>(preLimiterEnableParam->getIndex()));
    if (preLimiterGainParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PRE_LIMITER, 1, getNorm(preLimiterGainParam));
    if (preLimiterThreshParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PRE_LIMITER, 2, getNorm(preLimiterThreshParam));
    if (preLimiterReleaseParam)  engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PRE_LIMITER, 3, getNorm(preLimiterReleaseParam));

    if (postLimiterEnableParam)  engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_POST_LIMITER, 0, static_cast<float>(postLimiterEnableParam->getIndex()));
    if (postLimiterGainParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_POST_LIMITER, 1, getNorm(postLimiterGainParam));
    if (postLimiterThreshParam)  engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_POST_LIMITER, 2, getNorm(postLimiterThreshParam));
    if (postLimiterReleaseParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_POST_LIMITER, 3, getNorm(postLimiterReleaseParam));

    // FX Pickers
    if (preFX1TypeParam)  engine.setPreFXType(0, preFX1TypeParam->getIndex());
    if (preFX2TypeParam)  engine.setPreFXType(1, preFX2TypeParam->getIndex());
    if (preFX3TypeParam)  engine.setPreFXType(2, preFX3TypeParam->getIndex());
    if (preFX4TypeParam)  engine.setPreFXType(3, preFX4TypeParam->getIndex());

    if (postFX1TypeParam) engine.setPostFXType(0, postFX1TypeParam->getIndex());
    if (postFX2TypeParam) engine.setPostFXType(1, postFX2TypeParam->getIndex());
    if (postFX3TypeParam) engine.setPostFXType(2, postFX3TypeParam->getIndex());
    if (postFX4TypeParam) engine.setPostFXType(3, postFX4TypeParam->getIndex());

    for (int s = 0; s < 4; ++s) {
        for (int p = 0; p < 4; ++p) {
            if (preFXParam[s][p])  engine.setPreFXParam(s, p, getNorm(preFXParam[s][p]));
            if (postFXParam[s][p]) engine.setPostFXParam(s, p, getNorm(postFXParam[s][p]));
        }
    }

    // 12. Wave Folder

    // 13. RingMod

    // 14. Frequency Shifter

    // 15. Grit FX

    // 16. Comb Filter

    // 17. PhaseSmear

    // 18. EQ (bell EQ)

    // 19. Amp
    if (ampLevelParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMP, 0, getNorm(ampLevelParam));
    if (ampPanParam)          engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMP, 1, getNorm(ampPanParam));
    if (ampDriveParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMP, 2, getNorm(ampDriveParam));
    if (ampLimiterParam)      engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMP, 3, static_cast<float>(ampLimiterParam->getIndex()));

    // 20. Amp Envelope
    if (ampEnvClapsParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMPENV, 0, getNorm(ampEnvClapsParam));
    if (ampEnvClapSpeedParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMPENV, 1, getNorm(ampEnvClapSpeedParam));
    if (ampEnvSlopeParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMPENV, 2, getNorm(ampEnvSlopeParam));
    if (ampEnvDecayParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMPENV, 3, getNorm(ampEnvDecayParam));

    // 21. Velocity
    if (velSlopeParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 0, getNorm(velSlopeParam));
    if (velDepthParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 1, getNorm(velDepthParam));
    if (velDecayParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 2, getNorm(velDecayParam));
    if (velVolumeParam)       engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 3, getNorm(velVolumeParam));

    // 22. Key Tracking
    if (keySlopeParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_KEYTRACK, 0, getNorm(keySlopeParam));
    if (keyDepthParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_KEYTRACK, 1, getNorm(keyDepthParam));
    if (keyDecayParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_KEYTRACK, 2, getNorm(keyDecayParam));
    if (keyVolumeParam)       engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_KEYTRACK, 3, getNorm(keyVolumeParam));

    // 23. Slop
    if (slopFreqParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_SLOP, 0, getNorm(slopFreqParam));
    if (slopDepthParam)       engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_SLOP, 1, getNorm(slopDepthParam));
    if (slopDecayParam)       engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_SLOP, 2, getNorm(slopDecayParam));
    if (slopPanParam)         engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_SLOP, 3, getNorm(slopPanParam));

    // 24. Mod Envelopes 1..3
    if (modEnv1SlopeParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODENV1, 0, getNorm(modEnv1SlopeParam));
    if (modEnv1DepthParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODENV1, 1, getNorm(modEnv1DepthParam));
    if (modEnv1DecayParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODENV1, 2, getNorm(modEnv1DecayParam));
    if (modEnv1TargetParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODENV1, 3, static_cast<float>(modEnv1TargetParam->getIndex()));

    if (modEnv2SlopeParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODENV2, 0, getNorm(modEnv2SlopeParam));
    if (modEnv2DepthParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODENV2, 1, getNorm(modEnv2DepthParam));
    if (modEnv2DecayParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODENV2, 2, getNorm(modEnv2DecayParam));
    if (modEnv2TargetParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODENV2, 3, static_cast<float>(modEnv2TargetParam->getIndex()));

    if (modEnv3SlopeParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODENV3, 0, getNorm(modEnv3SlopeParam));
    if (modEnv3DepthParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODENV3, 1, getNorm(modEnv3DepthParam));
    if (modEnv3DecayParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODENV3, 2, getNorm(modEnv3DecayParam));
    if (modEnv3TargetParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODENV3, 3, static_cast<float>(modEnv3TargetParam->getIndex()));

    // Performance Macros
    for (int i = 0; i < 4; ++i) {
        if (macroParams[i]) {
            modMatrix.setMacro(i, getNorm(macroParams[i]));
        }
    }
}

void TheKlangFarmerAudioProcessor::applyModulationTargets() {
    const auto& destinations = getModDestinations();
    int numDests = static_cast<int>(destinations.size());
    if (numDests <= 0) return;

    // We need to zero out the modulation offsets first
    // Since some parameters might not be actively modulated anymore, we must reset all
    // Or we can just zero all active modulation destinations.
    // To be perfectly safe, we clear all active modulation offsets by calling setPageParameterModulation to 0.0f
    // But it's faster to just apply 0 to all destinations!
    // We can maintain a static array of accumulators
    float accumulators[512] = {0.0f};

    // Evaluate block-rate modulations
    modMatrix.evaluateBlockModulations(accumulators, numDests);

    // Evaluate audio-rate Hydras
    for (int h = 0; h < 4; ++h) {
        auto& hub = modMatrix.getHydra(h);
        if (hub.isAudioRate()) {
            float inVal = engine.getAudioRateSourceSample(hub.inputSource);
            // Convert bipolar oscillator output (-1 to 1) to unipolar (0 to 1) for the curve mapping
            float normIn = std::clamp((inVal * 0.5f) + 0.5f, 0.0f, 1.0f);
            
            for (int d = 0; d < 8; ++d) {
                const auto& dest = hub.destinations[d];
                if (dest.active && dest.targetParamId >= 0 && dest.targetParamId < numDests) {
                    accumulators[dest.targetParamId] += dest.mapValue(normIn);
                }
            }
        }
    }

    // Apply all modulation offsets (even 0.0f to reset unused ones)
    for (int i = 0; i < numDests; ++i) {
        const auto& d = destinations[i];
        if (d.type == ModTargetType::PageBlock) {
            engine.setPageParameterModulation(static_cast<TbdAudio::ModularDrumEngine::BlockID>(d.blockOrSlot), d.paramIndex, accumulators[i]);
        } else if (d.type == ModTargetType::PreFX) {
            if (auto* fx = engine.getPreFXBlock(d.blockOrSlot)) {
                fx->setParamModulation(d.paramIndex, accumulators[i]);
            }
        } else if (d.type == ModTargetType::PostFX) {
            if (auto* fx = engine.getPostFXBlock(d.blockOrSlot)) {
                fx->setParamModulation(d.paramIndex, accumulators[i]);
            }
        }
    }
}

void TheKlangFarmerAudioProcessor::triggerPanic() {
    panicRequested.store(true, std::memory_order_release);
}

void TheKlangFarmerAudioProcessor::triggerAudition(float velocity, int midiNote) {
    auditionVelocity.store(velocity, std::memory_order_relaxed);
    auditionNote.store(midiNote, std::memory_order_relaxed);
    auditionTriggerRequested.store(true, std::memory_order_release);
}

void TheKlangFarmerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    TbdAudio::FastMath::enableFTZDAZ();

    // Check for CC 120 (All Sound Off) or CC 123 (All Notes Off)
    for (const auto metadata : midiMessages) {
        auto msg = metadata.getMessage();
        if (msg.isController() && (msg.getControllerNumber() == 120 || msg.getControllerNumber() == 123)) {
            panicRequested.store(true, std::memory_order_release);
        }
    }

    if (panicRequested.exchange(false, std::memory_order_acq_rel)) {
        float* left = buffer.getNumChannels() > 0 ? buffer.getWritePointer(0) : nullptr;
        float* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : left;
        engine.triggerPanic(left, right, buffer.getNumSamples(), static_cast<float>(getSampleRate()));
        modMatrix.reset();
        midiMessages.clear();
        return;
    }

    if (auto* playHead = getPlayHead()) {
        if (auto posOpt = playHead->getPosition()) {
            if (auto bpmOpt = posOpt->getBpm()) {
                engine.setBpm(static_cast<float>(*bpmOpt));
            }
        }
    }

    applyBaseParameters();

    bool hasModTargets = (modMatrix.getNumActiveRoutes() > 0);
    bool requiresAudioRate = false;
    for (int h = 0; h < 4; ++h) {
        bool hasActiveDest = false;
        for (int d = 0; d < 8; ++d) {
            if (modMatrix.getHydra(h).destinations[d].active) {
                hasActiveDest = true;
                break;
            }
        }
        if (hasActiveDest) {
            hasModTargets = true;
            if (modMatrix.getHydra(h).isAudioRate()) {
                requiresAudioRate = true;
            }
        }
    }

    int numSamples = buffer.getNumSamples();
    float* left = buffer.getNumChannels() > 0 ? buffer.getWritePointer(0) : nullptr;
    float* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : left;

    auto renderSlice = [&](int startSample, int numSliceSamples) {
        if (numSliceSamples <= 0) return;

        if (!hasModTargets) {
            float* l = left ? left + startSample : nullptr;
            float* r = right ? right + startSample : nullptr;
            engine.processStereo(l, r, numSliceSamples);
            return;
        }

        constexpr int SUB_CHUNK = 32;
        int chunk = requiresAudioRate ? 1 : SUB_CHUNK;
        int slicePos = 0;
        
        while (slicePos < numSliceSamples) {
            int curChunk = std::min(chunk, numSliceSamples - slicePos);
            float* l = left ? left + startSample + slicePos : nullptr;
            float* r = right ? right + startSample + slicePos : nullptr;

            // Update modulation matrix sources (block-rate sources)
            modMatrix.setSourceValue(TbdAudio::ModSource::Env1, engine.getModEnvValue(0));
            modMatrix.setSourceValue(TbdAudio::ModSource::Env2, engine.getModEnvValue(1));
            modMatrix.setSourceValue(TbdAudio::ModSource::Env3, engine.getModEnvValue(2));
            modMatrix.setSourceValue(TbdAudio::ModSource::Velocity, engine.getContext().curvedVelocity);
            modMatrix.setSourceValue(TbdAudio::ModSource::KeyTrack, engine.getContext().curvedKeyNote);

            applyModulationTargets();
            engine.processStereo(l, r, curChunk);
            slicePos += curChunk;
        }
    };

    int currentSample = 0;

    if (auditionTriggerRequested.exchange(false, std::memory_order_acq_rel)) {
        engine.setMidiPitch(auditionNote.load(std::memory_order_relaxed));
        engine.trigger(auditionVelocity.load(std::memory_order_relaxed));
        if (hasModTargets) {
            applyModulationTargets();
        }
    }

    for (const auto metadata : midiMessages) {
        auto msg = metadata.getMessage();
        int eventSample = static_cast<int>(metadata.samplePosition);

        if (eventSample < currentSample) eventSample = currentSample;
        if (eventSample > numSamples) eventSample = numSamples;

        if (eventSample > currentSample) {
            renderSlice(currentSample, eventSample - currentSample);
            currentSample = eventSample;
        }

        if (msg.isNoteOn()) {
            engine.setMidiPitch(msg.getNoteNumber());
            engine.trigger(msg.getFloatVelocity());
            if (hasModTargets) {
                applyModulationTargets();
            }
        }
    }

    if (currentSample < numSamples) {
        renderSlice(currentSample, numSamples - currentSample);
    }

    // Monitor Saver Protocol: Silences any NaN/Inf corrupted frames
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        TbdAudio::FastMath::sanitizeBuffer(buffer.getWritePointer(ch), numSamples);
    }
}

juce::AudioProcessorEditor* TheKlangFarmerAudioProcessor::createEditor() {
    return new TheKlangFarmerAudioProcessorEditor(*this);
}

bool TheKlangFarmerAudioProcessor::hasEditor() const {
    return true;
}

const juce::String TheKlangFarmerAudioProcessor::getName() const {
    return "The Klang Farmer";
}

juce::AudioProcessorValueTreeState::ParameterLayout TheKlangFarmerAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto makeFloatParam = [](const juce::String& id, const juce::String& name, float defaultVal) {
        if (auto* def = RlyehSound::ParameterManager::getInstance().getControlDef(id)) {
            return std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID(id, 1), def->name,
                juce::NormalisableRange<float>(def->min, def->max, def->step, def->skew),
                def->defaultFloat);
        }
        return std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(id, 1), name,
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0005f), defaultVal);
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

    // --- 1. CARRIER 1 ---
    layout.add(makeChoiceParam("carrier1_tracking", "Carrier 1: Tracking", juce::StringArray{ "MIDI", "Freq", "Note" }, 0));
    layout.add(makeFloatParam("carrier1_pitch", "Carrier 1: Pitch / Freq", 0.5f));
    layout.add(makeFloatParam("carrier1_shape", "Carrier 1: Shape", 0.0f));         // Sine (0%)
    layout.add(makeFloatParam("carrier1_depth", "Carrier 1: Modulation Depth", 0.5f)); // 0% Depth (-200% to +200%)

    // --- 2. MODULATOR 1 ---
    layout.add(makeChoiceParam("mod1_track", "Modulator 1: Pitch Tracking", juce::StringArray{ "Fixed", "Follow", "FM" }, 0));
    layout.add(makeChoiceParam("mod1_type", "Modulator 1: Type", juce::StringArray{ "Osc", "Cyclic", "Noise" }, 0));
    layout.add(makeFloatParam("mod1_shape", "Modulator 1: Shape", 0.0f));          // Sine (0%)
    layout.add(makeFloatParam("mod1_speed", "Modulator 1: Speed", 0.5f));          // 1:1 ratio (0.5)

    // --- 3. PITCH ENVELOPE 1 ---
    layout.add(makeFloatParam("pitchenv1_slope", "PitchEnv 1: Slope", 0.5886f));   // Exponential (def)
    layout.add(makeFloatParam("pitchenv1_depth", "PitchEnv 1: Depth", 0.5f));      // 0%
    layout.add(makeFloatParam("pitchenv1_decay", "PitchEnv 1: Decay", 0.3806f));   // 333 ms
    layout.add(makeChoiceParam("pitchenv1_target", "PitchEnv 1: Target", juce::StringArray{ "Car", "Mod", "Both", "Opp" }, 0));

    // --- 4. CARRIER 2 ---
    layout.add(makeChoiceParam("carrier2_tracking", "Carrier 2: Tracking", juce::StringArray{ "MIDI", "Freq", "Note"  }, 0));
    layout.add(makeFloatParam("carrier2_pitch", "Carrier 2: Pitch / Freq", 0.5f));
    layout.add(makeFloatParam("carrier2_shape", "Carrier 2: Shape", 0.0f));         // Sine (0%)
    layout.add(makeFloatParam("carrier2_depth", "Carrier 2: Modulation Depth", 0.5f)); // 0% Depth (-200% to +200%)

    // --- 5. MODULATOR 2 ---
    layout.add(makeChoiceParam("mod2_track", "Modulator 2: Pitch Tracking", juce::StringArray{ "Fixed", "Follow", "FM"  }, 0));
    layout.add(makeChoiceParam("mod2_type", "Modulator 2: Type", juce::StringArray{ "Osc", "Cyclic", "Noise"  }, 0));
    layout.add(makeFloatParam("mod2_shape", "Modulator 2: Shape", 0.0f));          // Sine (0%)
    layout.add(makeFloatParam("mod2_speed", "Modulator 2: Speed", 0.5f));          // 1:1 ratio (0.5)

    // --- 6. PITCH ENVELOPE 2 ---
    layout.add(makeFloatParam("pitchenv2_slope", "PitchEnv 2: Slope", 0.5886f));   // Exponential (def)
    layout.add(makeFloatParam("pitchenv2_depth", "PitchEnv 2: Depth", 0.5f));      // 0%
    layout.add(makeFloatParam("pitchenv2_decay", "PitchEnv 2: Decay", 0.3806f));   // 333 ms
    layout.add(makeChoiceParam("pitchenv2_target", "PitchEnv 2: Target", juce::StringArray{ "Car", "Mod", "Both", "Opp"  }, 0));

    // --- 7. NOISE TRANSIENT ---
    layout.add(makeFloatParam("noise_sh_rate", "Noise: S&H Rate", 1.0f));       // 24 kHz
    layout.add(makeFloatParam("noise_filter", "Noise: Filter", 0.5f));          // Flat (50%)
    layout.add(makeFloatParam("noise_drive", "Noise: Drive", 0.5f));            // 0 dB (default)
    layout.add(makeFloatParam("noise_decay", "Noise: Decay", 0.3078f));         // 100 ms

    // --- 8. MIXER ---
    layout.add(makeFloatParam("mixer_carrier1_level", "Mixer: Carrier 1 Level", 0.5f)); // 100%
    layout.add(makeFloatParam("mixer_carrier2_level", "Mixer: Carrier 2 Level", 0.0f)); // 0% (double click returns 100%)
    layout.add(makeFloatParam("mixer_ringmod", "Mixer: Ring Mod Level", 0.0f));        // 0% (double click returns 100%)
    layout.add(makeFloatParam("mixer_noise_level", "Mixer: Noise Level", 0.0f));       // 0% (double click returns 100%)

    // --- 9. DRIVE ---

    // --- VOICE 1 FILTER & FILTER ENVELOPE ---
    layout.add(makeChoiceParam("filter1_type", "Filter 1: Type", juce::StringArray{ "LPF", "BPF", "HPF", "BRF"  }, 0));
    layout.add(makeChoiceParam("filter1_slope", "Filter 1: Slope", juce::StringArray{ "6", "12", "18", "24", "36"  }, 1));
    layout.add(makeFloatParam("filter1_cutoff", "Filter 1: Cutoff", 1.0f));
    layout.add(makeFloatParam("filter1_resonance", "Filter 1: Resonance", 0.0f));

    layout.add(makeFloatParam("filterenv1_slope", "FilterEnv 1: Slope", 0.5886f));
    layout.add(makeFloatParam("filterenv1_depth", "FilterEnv 1: Depth", 0.5f));
    layout.add(makeFloatParam("filterenv1_decay", "FilterEnv 1: Decay", 0.3806f));
    layout.add(makeFloatParam("filterenv1_postdrive", "FilterEnv 1: Post-Drive", 0.5f));

    // --- VOICE 2 FILTER & FILTER ENVELOPE ---
    layout.add(makeChoiceParam("filter2_type", "Filter 2: Type", juce::StringArray{ "LPF", "BPF", "HPF", "BRF"  }, 0));
    layout.add(makeChoiceParam("filter2_slope", "Filter 2: Slope", juce::StringArray{ "6", "12", "18", "24", "36"  }, 1));
    layout.add(makeFloatParam("filter2_cutoff", "Filter 2: Cutoff", 1.0f));
    layout.add(makeFloatParam("filter2_resonance", "Filter 2: Resonance", 0.0f));

    layout.add(makeFloatParam("filterenv2_slope", "FilterEnv 2: Slope", 0.5886f));
    layout.add(makeFloatParam("filterenv2_depth", "FilterEnv 2: Depth", 0.5f));
    layout.add(makeFloatParam("filterenv2_decay", "FilterEnv 2: Decay", 0.3806f));
    layout.add(makeFloatParam("filterenv2_postdrive", "FilterEnv 2: Post-Drive", 0.5f));

    // --- TRANSIENTS FILTER & FILTER ENVELOPE ---
    layout.add(makeChoiceParam("filter3_type", "Filter 3: Type", juce::StringArray{ "LPF", "BPF", "HPF", "BRF"  }, 0));
    layout.add(makeChoiceParam("filter3_slope", "Filter 3: Slope", juce::StringArray{ "6", "12", "18", "24", "36"  }, 1));
    layout.add(makeFloatParam("filter3_cutoff", "Filter 3: Cutoff", 1.0f));
    layout.add(makeFloatParam("filter3_resonance", "Filter 3: Resonance", 0.0f));

    layout.add(makeFloatParam("filterenv3_slope", "FilterEnv 3: Slope", 0.5886f));
    layout.add(makeFloatParam("filterenv3_depth", "FilterEnv 3: Depth", 0.5f));
    layout.add(makeFloatParam("filterenv3_decay", "FilterEnv 3: Decay", 0.3078f));
    layout.add(makeFloatParam("filterenv3_postdrive", "FilterEnv 3: Post-Drive", 0.5f));

    // --- STANDALONE FX FILTER ---
    layout.add(makeChoiceParam("pre_limiter_enable", "Pre-Limiter: Enable", juce::StringArray{ "Off", "On"  }, 1));
    layout.add(makeFloatParam("pre_limiter_gain", "Pre-Limiter: Input Gain", 12.0f / 36.0f)); // 0 dB
    layout.add(makeFloatParam("pre_limiter_thresh", "Pre-Limiter: Threshold", 1.0f));          // 0 dB
    layout.add(makeFloatParam("pre_limiter_release", "Pre-Limiter: Release", 0.6296f));       // 50 ms

    layout.add(makeChoiceParam("post_limiter_enable", "Post-Limiter: Enable", juce::StringArray{ "Off", "On"  }, 1));
    layout.add(makeFloatParam("post_limiter_gain", "Post-Limiter: Input Gain", 12.0f / 36.0f)); // 0 dB
    layout.add(makeFloatParam("post_limiter_thresh", "Post-Limiter: Threshold", 1.0f));          // 0 dB
    layout.add(makeFloatParam("post_limiter_release", "Post-Limiter: Release", 0.6296f));       // 50 ms

    // --- FX PICKERS ---
    const juce::StringArray fxChoices {
        "None",              // 0
        "Bell EQ",           // 1
        "Chorus",            // 2
        "Comb Filter",       // 3
        "Drive",             // 4
        "Filter",            // 5
        "Flanger",           // 6
        "Frequency Shifter", // 7
        "Grit FX",           // 8
        "Phase Smear",       // 9
        "Phaser",            // 10
        "RingMod",           // 11
        "Tempo Delay",       // 12
        "Wave Folder"        // 13
    };
    layout.add(makeChoiceParam("pre_fx_1_type", "Pre FX 1: Type", fxChoices, 4)); // Drive
    layout.add(makeChoiceParam("pre_fx_2_type", "Pre FX 2: Type", fxChoices, 13)); // Wave Folder
    layout.add(makeChoiceParam("pre_fx_3_type", "Pre FX 3: Type", fxChoices, 11)); // RingMod
    layout.add(makeChoiceParam("pre_fx_4_type", "Pre FX 4: Type", fxChoices, 7)); // Frequency Shifter

    layout.add(makeChoiceParam("post_fx_1_type", "Post FX 1: Type", fxChoices, 8)); // Grit FX
    layout.add(makeChoiceParam("post_fx_2_type", "Post FX 2: Type", fxChoices, 3)); // Comb Filter
    layout.add(makeChoiceParam("post_fx_3_type", "Post FX 3: Type", fxChoices, 9)); // Phase Smear
    layout.add(makeChoiceParam("post_fx_4_type", "Post FX 4: Type", fxChoices, 1)); // Bell EQ

    // --- 32 MULTI-INSTANCE FX SLOT PARAMETERS ---
    for (int s = 0; s < 4; ++s) {
        for (int p = 0; p < 4; ++p) {
            juce::String preId = "pre_fx_" + juce::String(s + 1) + "_p" + juce::String(p + 1);
            juce::String preName = "Pre FX " + juce::String(s + 1) + ": Param " + juce::String(p + 1);
            float defPre = 0.5f;
            if (s == 0) { const float d[4] = { 0.4f, 0.5f, 0.5f, 1.0f }; defPre = d[p]; } // Drive: +6dB (0.4)
            else if (s == 1) { const float d[4] = { 0.0f, 0.0f, 0.5f, 0.5f }; defPre = d[p]; }
            else if (s == 2) { const float d[4] = { 0.0f, 0.50934f, 0.0f, 0.5f }; defPre = d[p]; }
            else if (s == 3) { const float d[4] = { 0.5f, TbdAudio::rangeHzToNorm(3.0f), 0.75f, 0.5f }; defPre = d[p]; } // FreqShift: Blend +50%:50% (0.75)
            layout.add(makeFloatParam(preId, preName, defPre));

            juce::String postId = "post_fx_" + juce::String(s + 1) + "_p" + juce::String(p + 1);
            juce::String postName = "Post FX " + juce::String(s + 1) + ": Param " + juce::String(p + 1);
            float defPost = 0.5f;
            if (s == 0) { const float d[4] = { 1.0f, 1.0f, 0.5f, 0.5f }; defPost = d[p]; }
            else if (s == 1) { const float d[4] = { 1.0f, 1.0f, 0.5f, 0.75f }; defPost = d[p]; } // Comb: Damp 24k, Cut 24k, Res 0%, Mix +50%:50%
            else if (s == 2) { const float d[4] = { 0.0f, 4.0f / 32.0f, 0.62124f, 0.5f }; defPost = d[p]; } // Phase Smear: 2nd Order (0)
            else if (s == 3) { const float d[4] = { 1.0f, 0.0f, 0.5f, 0.5f }; defPost = d[p]; }
            layout.add(makeFloatParam(postId, postName, defPost));
        }
    }

    // --- 12. WAVE FOLDER ---

    // --- 13. RING MOD ---

    // --- 14. FREQUENCY SHIFTER ---

    // --- 15. GRIT FX ---

    // --- 16. COMB FILTER ---

    // --- 17. PHASE SMEAR ---

    // --- 18. EQ (bell EQ) ---

    // --- 19. AMP ---
    layout.add(makeFloatParam("amp_level", "Amp: Level", 1.0f));                // 100%
    layout.add(makeFloatParam("amp_pan", "Amp: Pan", 0.5f));                    // Center
    layout.add(makeFloatParam("amp_drive", "Amp: Drive", 0.5f));                // 0 dB
    layout.add(makeChoiceParam("amp_limiter", "Amp: Limiter", juce::StringArray{ "Off", "On"  }, 1));                                  // On

    // --- 20. AMP ENVELOPE ---
    layout.add(makeFloatParam("ampenv_claps", "AmpEnv: Claps", 0.0f));          // 0 claps
    layout.add(makeFloatParam("ampenv_clapspeed", "AmpEnv: Clap Speed", 0.1429f)); // 3 ms
    layout.add(makeFloatParam("ampenv_slope", "AmpEnv: Slope", 0.5886f));       // Exponential (def)
    layout.add(makeFloatParam("ampenv_decay", "AmpEnv: Decay", 0.3806f));       // 333 ms

    // --- 21. VELOCITY ---
    layout.add(makeFloatParam("vel_slope", "Velocity: Slope", 0.5886f));          // Exponential (def)
    layout.add(makeFloatParam("vel_depth", "Velocity: Depth", 0.5f));            // 0% (bipolar center)
    layout.add(makeFloatParam("vel_decay", "Velocity: Decay", 0.5f));            // 0% (bipolar center)
    layout.add(makeFloatParam("vel_volume", "Velocity: Volume", 0.0f));          // 0% (unipolar min)

    // --- 22. KEY TRACKING ---
    layout.add(makeFloatParam("key_slope", "KeyTrack: Slope", 0.5886f));          // Exponential (def)
    layout.add(makeFloatParam("key_depth", "KeyTrack: Depth", 0.5f));            // 0% (bipolar center)
    layout.add(makeFloatParam("key_decay", "KeyTrack: Decay", 0.5f));            // 0% (bipolar center)
    layout.add(makeFloatParam("key_volume", "KeyTrack: Volume", 0.0f));          // 0% (unipolar min)

    // --- 23. SLOP ---
    layout.add(makeFloatParam("slop_freq", "Slop: Frequency", 0.0f));           // 0% (unipolar min)
    layout.add(makeFloatParam("slop_depth", "Slop: Depth", 0.0f));              // 0% (unipolar min)
    layout.add(makeFloatParam("slop_decay", "Slop: Decay", 0.0f));              // 0% (unipolar min)
    layout.add(makeFloatParam("slop_pan", "Slop: Pan", 0.0f));                  // 0% (unipolar min)

    // --- 24. MOD ENVELOPE 1 ---
    const auto modChoices = getModDestinationChoices();
    layout.add(makeFloatParam("modenv1_slope", "ModEnv 1: Slope", 0.5886f));     // Exponential (def)
    layout.add(makeFloatParam("modenv1_depth", "ModEnv 1: Depth", 0.5f));        // 0% (bipolar center)
    layout.add(makeFloatParam("modenv1_decay", "ModEnv 1: Decay", 0.3806f));     // 333 ms
    layout.add(makeChoiceParam("modenv1_target", "ModEnv 1: Destination", modChoices, 0));

    // --- 25. MOD ENVELOPE 2 ---
    layout.add(makeFloatParam("modenv2_slope", "ModEnv 2: Slope", 0.5886f));     // Exponential (def)
    layout.add(makeFloatParam("modenv2_depth", "ModEnv 2: Depth", 0.5f));        // 0% (bipolar center)
    layout.add(makeFloatParam("modenv2_decay", "ModEnv 2: Decay", 0.3806f));     // 333 ms
    layout.add(makeChoiceParam("modenv2_target", "ModEnv 2: Destination", modChoices, 0));

    // --- 26. MOD ENVELOPE 3 ---
    layout.add(makeFloatParam("modenv3_slope", "ModEnv 3: Slope", 0.5886f));     // Exponential (def)
    layout.add(makeFloatParam("modenv3_depth", "ModEnv 3: Depth", 0.5f));        // 0% (bipolar center)
    layout.add(makeFloatParam("modenv3_decay", "ModEnv 3: Decay", 0.3806f));     // 333 ms
    layout.add(makeChoiceParam("modenv3_target", "ModEnv 3: Destination", modChoices, 0));

    // --- 27. PERFORMANCE MACROS ---
    layout.add(makeFloatParam("macro_1", "Macro 1", 0.0f));
    layout.add(makeFloatParam("macro_2", "Macro 2", 0.0f));
    layout.add(makeFloatParam("macro_3", "Macro 3", 0.0f));
    layout.add(makeFloatParam("macro_4", "Macro 4", 0.0f));

    return layout;
}

TheKlangFarmerAudioProcessor::ParamModulationInfo TheKlangFarmerAudioProcessor::getParamModulationInfo(const juce::String& paramId) const {
    ParamModulationInfo info;
    auto* param = apvts.getParameter(paramId);
    if (!param) return info;

    float baseNorm = param->getValue();
    float totalPeakOffset = 0.0f;
    float totalCurrOffset = 0.0f;

    // 1. Mod Envelopes 1..3
    const auto& destinations = getModDestinations();
    auto checkModEnv = [&](int envIdx, juce::AudioParameterChoice* targetParam, juce::AudioParameterFloat* depthParam, float envVal) {
        if (!targetParam || !depthParam) return;
        int t = targetParam->getIndex();
        if (t <= 0 || t > static_cast<int>(destinations.size())) return;
        const auto& dest = destinations[t - 1];
        if (paramId == dest.id) {
            float d = (depthParam->get() - 0.5f) * 2.0f;
            if (std::abs(d) >= 0.005f) {
                totalPeakOffset += d;
                totalCurrOffset += envVal;
                int pct = static_cast<int>(std::round(d * 100.0f));
                info.sources.push_back({ "Mod Env " + juce::String(envIdx + 1), (pct > 0 ? "+" : "") + juce::String(pct) + "%" });
            }
        }
    };

    checkModEnv(0, modEnv1TargetParam, modEnv1DepthParam, engine.getModEnvValue(0));
    checkModEnv(1, modEnv2TargetParam, modEnv2DepthParam, engine.getModEnvValue(1));
    checkModEnv(2, modEnv3TargetParam, modEnv3DepthParam, engine.getModEnvValue(2));

    // 2. Filter Envelopes 1..3
    constexpr float filterOctToNorm = 0.559513f; // 10.0 / log2(240000)
    if (paramId == "filter1_cutoff" && filterEnv1DepthParam) {
        float d = (filterEnv1DepthParam->get() - 0.5f) * 2.0f;
        if (std::abs(d) >= 0.005f) {
            totalPeakOffset += d * filterOctToNorm;
            totalCurrOffset += engine.getFilterEnvValue(1) * filterOctToNorm;
            float oct = d * 10.0f;
            info.sources.push_back({ "Filter Env 1", (oct > 0 ? "+" : "") + juce::String(oct, 1) + " oct" });
        }
    } else if (paramId == "filter2_cutoff" && filterEnv2DepthParam) {
        float d = (filterEnv2DepthParam->get() - 0.5f) * 2.0f;
        if (std::abs(d) >= 0.005f) {
            totalPeakOffset += d * filterOctToNorm;
            totalCurrOffset += engine.getFilterEnvValue(2) * filterOctToNorm;
            float oct = d * 10.0f;
            info.sources.push_back({ "Filter Env 2", (oct > 0 ? "+" : "") + juce::String(oct, 1) + " oct" });
        }
    } else if (paramId == "filter3_cutoff" && filterEnv3DepthParam) {
        float d = (filterEnv3DepthParam->get() - 0.5f) * 2.0f;
        if (std::abs(d) >= 0.005f) {
            totalPeakOffset += d * filterOctToNorm;
            totalCurrOffset += engine.getFilterEnvValue(3) * filterOctToNorm;
            float oct = d * 10.0f;
            info.sources.push_back({ "Filter Env 3", (oct > 0 ? "+" : "") + juce::String(oct, 1) + " oct" });
        }
    }

    // 3. Pitch Envelopes 1..2
    constexpr float pitchOctToNorm = 0.34363f; // 5.0 / log2(24000)
    float fmPeakSpan = 0.0f;
    if (paramId == "carrier1_pitch") {
        info.showNeedle = false;
        if (pitchEnv1DepthParam && pitchEnv1TargetParam) {
            int t = pitchEnv1TargetParam->getIndex();
            if (t == 0 || t == 2 || t == 3) {
                float d = (pitchEnv1DepthParam->get() - 0.5f) * 2.0f;
                if (std::abs(d) >= 0.005f) {
                    totalPeakOffset += d * pitchOctToNorm;
                    totalCurrOffset += engine.getPitchEnvValue(1) * pitchOctToNorm;
                    float oct = d * 5.0f;
                    info.sources.push_back({ "Pitch Env 1", (oct > 0 ? "+" : "") + juce::String(oct, 1) + " oct" });
                }
            }
        }
        if (carrier1DepthParam) {
            float d = (carrier1DepthParam->get() - 0.5f) * 2.0f;
            float absD = std::abs(d);
            if (absD >= 0.005f) {
                float fmSpanNorm = absD * (4.0f / 10.2288f);
                float oct = absD * 4.0f;
                float pct = absD * 100.0f;
                info.sources.push_back({ "Mod Depth", juce::String(pct, 0) + "% (\xc2\xb1" + juce::String(oct, 1) + " oct)" });
                fmPeakSpan = std::max(fmPeakSpan, fmSpanNorm);
            }
        }
    } else if (paramId == "mod1_speed" && pitchEnv1DepthParam && pitchEnv1TargetParam) {
        int t = pitchEnv1TargetParam->getIndex();
        if (t == 1 || t == 2 || t == 3) {
            float sign = (t == 3) ? -1.0f : 1.0f;
            float d = (pitchEnv1DepthParam->get() - 0.5f) * 2.0f * sign;
            if (std::abs(d) >= 0.005f) {
                totalPeakOffset += d * pitchOctToNorm;
                totalCurrOffset += engine.getPitchEnvValue(1) * sign * pitchOctToNorm;
                float oct = d * 5.0f;
                info.sources.push_back({ "Pitch Env 1", (oct > 0 ? "+" : "") + juce::String(oct, 1) + " oct" });
            }
        }
    } else if (paramId == "carrier2_pitch") {
        info.showNeedle = false;
        if (pitchEnv2DepthParam && pitchEnv2TargetParam) {
            int t = pitchEnv2TargetParam->getIndex();
            if (t == 0 || t == 2 || t == 3) {
                float d = (pitchEnv2DepthParam->get() - 0.5f) * 2.0f;
                if (std::abs(d) >= 0.005f) {
                    totalPeakOffset += d * pitchOctToNorm;
                    totalCurrOffset += engine.getPitchEnvValue(2) * pitchOctToNorm;
                    float oct = d * 5.0f;
                    info.sources.push_back({ "Pitch Env 2", (oct > 0 ? "+" : "") + juce::String(oct, 1) + " oct" });
                }
            }
        }
        if (carrier2DepthParam) {
            float d = (carrier2DepthParam->get() - 0.5f) * 2.0f;
            float absD = std::abs(d);
            if (absD >= 0.005f) {
                float fmSpanNorm = absD * (4.0f / 10.2288f);
                float oct = absD * 4.0f;
                float pct = absD * 100.0f;
                info.sources.push_back({ "Mod Depth", juce::String(pct, 0) + "% (\xc2\xb1" + juce::String(oct, 1) + " oct)" });
                fmPeakSpan = std::max(fmPeakSpan, fmSpanNorm);
            }
        }
    } else if (paramId == "mod2_speed" && pitchEnv2DepthParam && pitchEnv2TargetParam) {
        int t = pitchEnv2TargetParam->getIndex();
        if (t == 1 || t == 2 || t == 3) {
            float sign = (t == 3) ? -1.0f : 1.0f;
            float d = (pitchEnv2DepthParam->get() - 0.5f) * 2.0f * sign;
            if (std::abs(d) >= 0.005f) {
                totalPeakOffset += d * pitchOctToNorm;
                totalCurrOffset += engine.getPitchEnvValue(2) * sign * pitchOctToNorm;
                float oct = d * 5.0f;
                info.sources.push_back({ "Pitch Env 2", (oct > 0 ? "+" : "") + juce::String(oct, 1) + " oct" });
            }
        }
    }

    if (info.sources.empty() || (std::abs(totalPeakOffset) < 0.002f && fmPeakSpan < 0.002f)) {
        info.isModulated = false;
        return info;
    }

    info.isModulated = true;
    float peakNorm = std::clamp(baseNorm + totalPeakOffset, 0.0f, 1.0f);
    float currNorm = std::clamp(baseNorm + totalCurrOffset, 0.0f, 1.0f);
    float minN = std::min(baseNorm, peakNorm) - fmPeakSpan;
    float maxN = std::max(baseNorm, peakNorm) + fmPeakSpan;
    info.rangeMinNorm = std::clamp(minN, 0.0f, 1.0f);
    info.rangeMaxNorm = std::clamp(maxN, 0.0f, 1.0f);
    info.currentNorm = currNorm;
    info.rangeText = param->getText(info.rangeMinNorm, 16) + " " + juce::String(juce::CharPointer_UTF8("\xe2\x86\x92")) + " " + param->getText(info.rangeMaxNorm, 16);
    info.liveValueText = param->getText(currNorm, 16);

    return info;
}

#ifndef TKF_GUI_TESTS
// JUCE plugin entry point factory
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TheKlangFarmerAudioProcessor();
}
#endif





