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
    { hasturParameters::Ids::osc1_mult,            "Strangle",  "primary DCO pitch multiplier from sub-octave division to extreme harmonics" },
    { hasturParameters::Ids::osc1_apf_pitch_track, "Warp",      "4-stage allpass phase-dispersion ladder tracking oscillator pitch" },
    { hasturParameters::Ids::osc1_drive,           "Melt",      "asymmetric wavefolding and soft-saturation drive on primary core" },
    { hasturParameters::Ids::dco_bite,             "Bite",      "bipolar regenerative feedback across primary DCO allpass ladder" },
    { hasturParameters::Ids::sub_apf_depth,        "Chew",      "center frequency offset of the sub-oscillator allpass network" },
    { hasturParameters::Ids::sub_spit,             "Spit",      "bipolar regenerative feedback on sub allpass filter stage" },
    { hasturParameters::Ids::dco_morph,            "Morph",     "continuous geometric waveform morphing between core shapes" },
    { hasturParameters::Ids::dco_wave_family,      "Wave",      "discrete base waveform topology: saw, pulse, triangle, or sine" },
    { hasturParameters::Ids::dco_chopper_topo,     "Chop",      "phase-synchronous chopping and hard-sync harmonic multiplication" },

    // 2. Sub-Stator & Pulse Train (Bay 1 Lower)
    { hasturParameters::Ids::sub_octave,           "Sub",       "sub-oscillator frequency divider (-2, -1, unison, or +1 octave)", kSubOctave },
    { hasturParameters::Ids::sub_pulse_train,      "Pulse",     "sub-oscillator pulse-train duty cycle and harmonic spread" },
    { hasturParameters::Ids::sub_apf_tap,          "Tap",       "selectable allpass stage tap for sub phase-dispersion ladder",      kSubTap },

    // 3. Stem Mixer & Audio-Rate X-Mod (Bay 2)
    { hasturParameters::Ids::osc1_level,           "Core",      "primary DCO core level injected into the analog stem mixer" },
    { hasturParameters::Ids::osc2_level,           "Sub",       "sub-oscillator level injected into the analog stem mixer" },
    { hasturParameters::Ids::noise_level,          "Noise",     "analog white noise generator level for transient breath and grit" },
    { hasturParameters::Ids::mixer_drive,          "Seethe",    "overdrive saturation driving the dual cascaded VCF stages" },
    { hasturParameters::Ids::mod_a_source,         "FM Source", "modulation carrier source selecting raw DCO, shaped core, or noise", kModASource },
    { hasturParameters::Ids::mod_a_amount,         "FM Depth",  "bipolar audio-rate frequency modulation depth to VCF cutoff" },
    { hasturParameters::Ids::mod_b_source,         "AM Source", "modulation carrier source selecting raw DCO, shaped core, or sub",    kModBSource },
    { hasturParameters::Ids::mod_b_amount,         "AM Depth",  "bipolar audio-rate amplitude modulation depth to the VCA stage" },

    // 4. Dual Cascaded SVF VCF & Burble (Bay 3)
    { hasturParameters::Ids::cutoff,               "Cutoff",    "master corner cutoff frequency of the cascaded dual-SVF ladder" },
    { hasturParameters::Ids::resonance,            "Res",       "dual-SVF resonance peak with self-oscillation feedback" },
    { hasturParameters::Ids::filter_env_decay,     "Decay",     "filter envelope decay time from percussive pluck to long swell" },
    { hasturParameters::Ids::burble_depth,         "Burble",    "sub-oscillator to VCF cutoff frequency modulation depth" },
    { hasturParameters::Ids::laser_drive,          "Slash",     "pre-filter asymmetric wavefolding drive and odd-harmonic crunch" },
    { hasturParameters::Ids::filter_warp,          "Twist",     "filter feedback loop phase distortion and nonlinear resonance" },
    { hasturParameters::Ids::laser_bite,           "Stab",      "bipolar allpass feedback injecting sharp formant bite into filter" },
    { hasturParameters::Ids::filter_orbit,         "Orbit",     "stereo filter cutoff offset creating wide spatial rotation" },
    { hasturParameters::Ids::burble_mode,          "Burble",    "burble modulation algorithm: exponential FM vs linear FM",   kBurbleMode },
    { hasturParameters::Ids::filter_deck,          "Deck",      "dual-SVF routing topology: pre-24, duo-48, or post-24", kFilterDeck },
    { hasturParameters::Ids::env_depth,            "Env Amt",   "bipolar envelope modulation depth controlling filter cutoff" },
    { hasturParameters::Ids::filter_velocity,      "Accent",    "dynamic keyboard velocity sensitivity to filter envelope depth" },
    { hasturParameters::Ids::keytrack,             "Keytrack",  "1V/oct keyboard tracking ratio scaling filter cutoff with pitch" },

    // 5. Modulation & Phase 3 LFO Engine (Bay 3 Lower)
    { hasturParameters::Ids::filter_lfo_depth,     "LFO Amt",   "low frequency oscillator modulation depth to filter cutoff" },
    { hasturParameters::Ids::filter_lfo_dest,      "Dest",      "LFO modulation destination routing to cutoff or stereo orbit",  kFilterLfoDest },
    { hasturParameters::Ids::lfo_rate,             "Rate",      "primary LFO oscillation frequency in Hertz or beat divisions" },
    { hasturParameters::Ids::glide_time,           "Glide",     "portamento glide transition time between consecutive notes" },
    { hasturParameters::Ids::glide_mode,           "Glide Mode","portamento tracking behavior: continuous glide vs legato only", kGlideMode },
    { hasturParameters::Ids::lfo_wave,             "LFO Wave",  "low frequency oscillator waveform: triangle, pulse, sine, or random",            kLfoWave },
    { hasturParameters::Ids::lfo_shape,            "Shape",     "LFO symmetry skew, pulse width, wavefolding, or noise slew rate" },
    { hasturParameters::Ids::lfo_sync,             "Sync",      "LFO timebase mode: free-running Hertz vs host tempo sync", kLfoSync },
    { hasturParameters::Ids::pw_lfo_amount,        "Amt",       "LFO modulation depth to DCO waveform morph position" },
    { hasturParameters::Ids::pw_lfo_rate,          "Rate",      "dedicated morph LFO speed in Hertz or synced divisions" },

    // Stepped Random Modulation Matrix (Bay 4)
    { hasturParameters::Ids::rand_core_warp,       "Core Warp", "stepped random S&H modulation depth on primary DCO allpass warp" },
    { hasturParameters::Ids::rand_core_bite,       "Core Bite", "stepped random S&H modulation depth on DCO bite feedback" },
    { hasturParameters::Ids::rand_core_melt,       "Core Melt", "stepped random S&H modulation depth on core melt saturation" },
    { hasturParameters::Ids::rand_sub_chew,        "Sub Chew",  "stepped random S&H modulation depth on sub allpass chew offset" },
    { hasturParameters::Ids::rand_sub_spit,        "Sub Spit",  "stepped random S&H modulation depth on sub allpass spit feedback" },
    { hasturParameters::Ids::rand_raw_strangle,    "Strangle",  "stepped random S&H modulation depth on raw pulse strangle" },
    { hasturParameters::Ids::rand_filter_twist,    "Twist",     "stepped random S&H modulation depth on VCF filter warp" },
    { hasturParameters::Ids::rand_filter_stab,     "Stab",      "stepped random S&H modulation depth on laser bite filter stab" },
    { hasturParameters::Ids::rand_filter_orbit,    "Orbit",     "stepped random S&H modulation depth on stereo filter orbit" },

    // 6. Master VCA & Transient Dynamics (Bay 5)
    { hasturParameters::Ids::amp_gate_mode,        "Mode",      "VCA gate mode: ADSR envelope, instant gate, or infinite drone",              kAmpGateMode },
    { hasturParameters::Ids::amp_env_snap,         "Snap",      "amplitude envelope initial attack time and punch definition" },
    { hasturParameters::Ids::amp_env_decay,        "Plunge",    "amplitude envelope decay time falling to sustain plateau" },
    { hasturParameters::Ids::pitch_punch_depth,    "Punch",     "bipolar transient pitch envelope depth for initial kick impact" },
    { hasturParameters::Ids::pitch_punch_time,     "Time",      "pitch envelope transient decay duration in milliseconds" },
    { hasturParameters::Ids::amp_env_sustain,      "Lurk",      "amplitude envelope steady-state sustain level" },
    { hasturParameters::Ids::amp_env_release,      "Bleed",     "amplitude envelope final release time after note off" },
    { hasturParameters::Ids::amp_velocity,         "Velocity",  "dynamic keyboard velocity sensitivity to master output volume" },

    // 7. Output Stage, Chorus & Global Systems (Bay 6 & Header)
    { hasturParameters::Ids::osc_slop,             "Slop",      "analog circuit instability, pitch drift, and thermal fluctuations" },
    { hasturParameters::Ids::post_amp_drive,       "Drive",     "post-VCA analog circuit saturation and warm clipping" },
    { hasturParameters::Ids::alpha_chorus,         "Chorus",    "vintage bucket-brigade stereo chorus ensemble depth and rate" },
    { hasturParameters::Ids::global_width,         "Width",     "master stereo image width from mono center to hyper-wide" },
    { hasturParameters::Ids::output_level,         "Volume",    "master output attenuation before digital-to-analog converter" },
    { hasturParameters::Ids::life_mode,            "Lifeform",  "triad sonic canon character: synthetic, cyborg, or organic", kLifeMode },
    { hasturParameters::Ids::trigger_mode,         "Trigger",   "voice envelope and random trigger behavior", kTriggerMode },
    { hasturParameters::Ids::pitch_bend_range,     "Bend",      "master pitch wheel modulation range in semitones" },
    { hasturParameters::Ids::vibrato_rate,         "Vib Rate",  "global pitch vibrato LFO frequency in Hertz" },
    { hasturParameters::Ids::vibrato_depth,        "Vib Depth", "global pitch vibrato modulation depth in semitones" }
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
