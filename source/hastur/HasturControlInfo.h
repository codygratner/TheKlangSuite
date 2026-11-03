#pragma once

#include <string_view>
#include <span>
#include <array>
#include "HasturParameterIds.h"

namespace HasturInfo {

struct Option {
    std::string_view display;
    std::string_view blurb;
};

struct Control {
    std::string_view paramId;
    std::string_view name;
    std::string_view blurb;
    std::span<const Option> options{};
};

// ==============================================================================
// Discrete Sub-Option Tables
// ==============================================================================

inline constexpr Option kSubOctave[] = {
    {"-2", "two octaves down"},
    {"-1", "one octave down"},
    {"0",  "unison with core"},
    {"+1", "one octave up"}
};

inline constexpr Option kSubTap[] = {
    {"OFF", "allpass stages bypassed"},
    {"4P",  "four pole allpass tap"},
    {"8P",  "eight pole allpass tap"},
    {"16P", "sixteen pole allpass tap"}
};

inline constexpr Option kModASource[] = {
    {"RAW",   "raw oscillator tap"},
    {"CORE",  "shaped dco core"},
    {"NOISE", "analog noise source"}
};

inline constexpr Option kModBSource[] = {
    {"RAW",  "raw oscillator tap"},
    {"CORE", "shaped dco core"},
    {"SUB",  "sub oscillator source"}
};

inline constexpr Option kBurbleMode[] = {
    {"XFM", "exponential fm to cutoff"},
    {"LFM", "linear fm to cutoff"}
};

// filter_deck APVTS parameter value: 0 = Pre-24, 1 = Duo-48, 2 = Post-24
inline constexpr Option kFilterDeck[] = {
    {"PRE-24",  "4-pole filter before apf"},
    {"DUO-48",  "apf between two 4-pole filters"},
    {"POST-24", "4-pole filter after apf"}
};

inline constexpr Option kFilterLfoDest[] = {
    {"CUTOFF", "modulates filter cutoff frequency"},
    {"ORBIT",  "modulates stereo filter orbit offset"}
};

inline constexpr Option kLfoSync[] = {
    {"HZ",  "free running frequency in hertz"},
    {"BPM", "synced to host tempo divisions"}
};

inline constexpr Option kLfoWave[] = {
    {"TRI",    "triangle waveform"},
    {"PULSE",  "variable pulse waveform"},
    {"SINE",   "sine with wavefolder"},
    {"RANDOM", "S&H noise with slew"}
};

inline constexpr Option kAmpGateMode[] = {
    {"ENV",   "standard 4-stage envelope generator"},
    {"GATE",  "instant on off gate"},
    {"DRONE", "continuous infinite drone"}
};

inline constexpr Option kGlideMode[] = {
    {"ON",   "continuous portamento on all notes"},
    {"LGTO", "legato only portamento"}
};

inline constexpr Option kLifeMode[] = {
    {"SYNTHETIC", "clean wide surgical precision"},
    {"CYBORG",    "gritty overdriven analog character"},
    {"ORGANIC",   "warm drifted vintage circuit"}
};

inline constexpr Option kTriggerMode[] = {
    {"RETRIG", "every note retriggers envelopes and resamples random"},
    {"LEGATO", "legato phrases hold envelopes and random values"},
    {"AUTO",   "single-column auto-legato with 15ms grace window"}
};

// ==============================================================================
// Master 57-Parameter Control SSoT Metadata Table
// ==============================================================================

inline constexpr Control kControls[] = {
    // 1. DCO & Core Synthesis (Bay 1)
    { hasturParameters::Ids::osc1_mult,            "Strangle",  "octave multiplier for primary dco" },
    { hasturParameters::Ids::osc1_apf_pitch_track, "Warp",      "APF stack around chopper" },
    { hasturParameters::Ids::osc1_drive,           "Melt",      "wavefolder and tanh saturation after APF stages" },
    { hasturParameters::Ids::dco_bite,             "Bite",      "bipolar feedback for APFs" },
    { hasturParameters::Ids::sub_apf_depth,        "Chew",      "sub APF frequency offset" },
    { hasturParameters::Ids::sub_spit,             "Spit",      "bipolar feedback for sub APF" },
    { hasturParameters::Ids::dco_morph,            "Morph",     "alpha waveform morph position" },
    { hasturParameters::Ids::dco_wave_family,      "Wave",      "saw pulse tri or sine core" },
    { hasturParameters::Ids::dco_chopper_topo,     "Chop",      "phase chopping multiplier" },

    // 2. Sub-Stator & Pulse Train (Bay 1 Lower)
    { hasturParameters::Ids::sub_octave,           "Sub",       "sub oscillator octave divider", kSubOctave },
    { hasturParameters::Ids::sub_pulse_train,      "Pulse",     "sub oscillator pulse train topology" },
    { hasturParameters::Ids::sub_apf_tap,          "Tap",       "allpass filter stage tap",      kSubTap },

    // 3. Stem Mixer & Audio-Rate X-Mod (Bay 2)
    { hasturParameters::Ids::osc1_level,           "Core",      "primary oscillator level" },
    { hasturParameters::Ids::osc2_level,           "Sub",       "sub oscillator level" },
    { hasturParameters::Ids::noise_level,          "Noise",     "white noise level" },
    { hasturParameters::Ids::mixer_drive,          "Seethe",    "tanh saturation after mixer" },
    { hasturParameters::Ids::mod_a_source,         "FM Source", "modulation source for filter fm", kModASource },
    { hasturParameters::Ids::mod_a_amount,         "FM Depth",  "bipolar audio rate filter fm depth" },
    { hasturParameters::Ids::mod_b_source,         "AM Source", "modulation source for vca am",    kModBSource },
    { hasturParameters::Ids::mod_b_amount,         "AM Depth",  "bipolar audio rate vca am depth" },

    // 4. Dual Cascaded SVF VCF & Burble (Bay 3)
    { hasturParameters::Ids::cutoff,               "Cutoff",    "cascaded dual-SVF filter cutoff frequency" },
    { hasturParameters::Ids::resonance,            "Res",       "filter resonance peak" },
    { hasturParameters::Ids::filter_env_decay,     "Decay",     "filter envelope decay" },
    { hasturParameters::Ids::burble_depth,         "Burble",    "sub to cutoff fm" },
    { hasturParameters::Ids::laser_drive,          "Slash",     "pre filter asymmetric wavefolding drive" },
    { hasturParameters::Ids::filter_warp,          "Twist",     "filter feedback phase distortion" },
    { hasturParameters::Ids::laser_bite,           "Stab",      "bipolar feedback for filter APF" },
    { hasturParameters::Ids::filter_orbit,         "Orbit",     "stereo filter offset" },
    { hasturParameters::Ids::burble_mode,          "Burble",    "burble modulation algorithm",   kBurbleMode },
    { hasturParameters::Ids::filter_deck,          "Deck",      "dual filter routing topology", kFilterDeck },
    { hasturParameters::Ids::env_depth,            "Env Amt",   "bipolar filter envelope depth" },
    { hasturParameters::Ids::filter_velocity,      "Accent",    "velocity sensitivity to filter cutoff" },
    { hasturParameters::Ids::keytrack,             "Keytrack",  "keyboard tracking for filter cutoff" },

    // 5. Modulation & Phase 3 LFO Engine (Bay 3 Lower)
    { hasturParameters::Ids::filter_lfo_depth,     "LFO Amt",   "filter lfo modulation depth" },
    { hasturParameters::Ids::filter_lfo_dest,      "Dest",      "route lfo to cutoff or orbit",  kFilterLfoDest },
    { hasturParameters::Ids::lfo_rate,             "Rate",      "lfo oscillation frequency" },
    { hasturParameters::Ids::glide_time,           "Glide",     "portamento transition time" },
    { hasturParameters::Ids::glide_mode,           "Glide Mode","continuous or legato portamento", kGlideMode },
    { hasturParameters::Ids::lfo_wave,             "LFO Wave",  "lfo waveform shape",            kLfoWave },
    { hasturParameters::Ids::lfo_shape,            "Shape",     "lfo symmetry or wavefolding" },
    { hasturParameters::Ids::lfo_sync,             "Sync",      "free running hz or tempo sync", kLfoSync },
    { hasturParameters::Ids::pw_lfo_amount,        "Amt",       "morph lfo modulation depth" },
    { hasturParameters::Ids::pw_lfo_rate,          "Rate",      "morph lfo speed" },

    // Stepped Random Modulation Matrix (Bay 4)
    { hasturParameters::Ids::rand_core_warp,       "Core Warp", "stepped random modulation on core apf warp" },
    { hasturParameters::Ids::rand_core_bite,       "Core Bite", "stepped random modulation on dco bite" },
    { hasturParameters::Ids::rand_core_melt,       "Core Melt", "stepped random modulation on core melt drive" },
    { hasturParameters::Ids::rand_sub_chew,        "Sub Chew",  "stepped random modulation on sub apf chew depth" },
    { hasturParameters::Ids::rand_sub_spit,        "Sub Spit",  "stepped random modulation on sub apf spit feedback" },
    { hasturParameters::Ids::rand_raw_strangle,    "Strangle",  "stepped random modulation on raw pulse strangle" },
    { hasturParameters::Ids::rand_filter_twist,    "Twist",     "stepped random modulation on vcf filter twist" },
    { hasturParameters::Ids::rand_filter_stab,     "Stab",      "stepped random modulation on laser bite filter stab" },
    { hasturParameters::Ids::rand_filter_orbit,    "Orbit",     "stepped random modulation on filter allpass orbit" },

    // 6. Master VCA & Transient Dynamics (Bay 5)
    { hasturParameters::Ids::amp_gate_mode,        "Mode",      "vca trigger mode",              kAmpGateMode },
    { hasturParameters::Ids::amp_env_snap,         "Snap",      "vca envelope attack" },
    { hasturParameters::Ids::amp_env_decay,        "Plunge",    "amp env decay" },
    { hasturParameters::Ids::pitch_punch_depth,    "Punch",     "pitch envelope transient depth" },
    { hasturParameters::Ids::pitch_punch_time,     "Time",      "pitch envelope transient decay" },
    { hasturParameters::Ids::amp_env_sustain,      "Lurk",      "vca sustain level" },
    { hasturParameters::Ids::amp_env_release,      "Bleed",     "vca envelope release" },
    { hasturParameters::Ids::amp_velocity,         "Velocity",  "velocity sensitivity to amp volume" },

    // 7. Output Stage, Chorus & Global Systems (Bay 6 & Header)
    { hasturParameters::Ids::osc_slop,             "Slop",      "analog oscillator drift and instability" },
    { hasturParameters::Ids::post_amp_drive,       "Drive",     "analog saturation on final mix" },
    { hasturParameters::Ids::alpha_chorus,         "Chorus",    "chorus amount and speed" },
    { hasturParameters::Ids::global_width,         "Width",     "master stereo field width" },
    { hasturParameters::Ids::output_level,         "Volume",    "master output attenuation" },
    { hasturParameters::Ids::life_mode,            "Lifeform",  "synthetic cyborg or organic character", kLifeMode },
    { hasturParameters::Ids::trigger_mode,         "Trigger",   "voice envelope and random trigger behavior", kTriggerMode },
    { hasturParameters::Ids::pitch_bend_range,     "Bend",      "master pitch wheel bend range in semitones" },
    { hasturParameters::Ids::vibrato_rate,         "Vib Rate",  "global sine vibrato frequency" },
    { hasturParameters::Ids::vibrato_depth,        "Vib Depth", "global sine vibrato modulation depth" }
};

static_assert(sizeof(kControls) / sizeof(kControls[0]) == 70,
              "HasturInfo::kControls must cover exactly all 70 parameters in HasturParameterIds.h!");

inline const Control* find(std::string_view paramId) noexcept {
    for (const auto& c : kControls) {
        if (c.paramId == paramId)
            return &c;
    }
    return nullptr;
}

// Dynamic context-sensitive resolution for LFO Shape based on current LFO Waveform
inline std::string_view getDynamicShapeBlurb(int lfoWaveIndex) noexcept {
    switch (lfoWaveIndex) {
        case 0: return "saw - triangle - reverse saw";
        case 1: return "pulse width";
        case 2: return "infinifold";
        case 3: return "slew rate";
        default: return "lfo symmetry or wavefolding";
    }
}

} // namespace HasturInfo
