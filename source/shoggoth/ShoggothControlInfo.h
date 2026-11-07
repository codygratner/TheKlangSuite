#pragma once

#include <string_view>
#include <span>
#include <array>
#include "ShoggothParameterIds.h"

namespace ShoggothInfo {

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

inline constexpr Option kAlphaChorusOptions[] = {
    {"OFF",         "In-synthesis chorus bypassed"},
    {"CHORUS I",    "Slow 0.4 Hz vintage ensemble swirl"},
    {"CHORUS II",   "Deuterated 0.6 Hz rich stereo shimmer"},
    {"CHORUS I+II", "Fast 8.0 Hz rotating vibrato wash"}
};

inline constexpr Option kLifeModeOptions[] = {
    {"SYNTHETIC", "Ultra-clean virtual analog (>140 dB SNR)"},
    {"ORGANIC",   "Warm tube-backed LPG and optical response"},
    {"CYBORG",    "Nuclear reactor saturation and digital grit"}
};

inline constexpr Option kVintageEngineModeOptions[] = {
    {"MODERN ZDF",      "Zero-delay feedback 4-pole clean lowpass"},
    {"1987 STEPPED DDF","Authentic 1987 phase smear and DAC character"},
    {"BYPASS",          "Pure frequency-domain zero-phase additive summation"}
};

inline constexpr Option kVoiceModeOptions[] = {
    {"POLY 64",   "64-voice dynamic allocation pool"},
    {"MONO",      "Monophonic note priority with retrigger"},
    {"LEGATO",    "Continuous phase gliding legato monosynth"},
    {"8x UNISON", "8-voice mono unison cluster (16,384 partials)"}
};

inline constexpr Option kCpuTierOptions[] = {
    {"ECO 1024",     "Up to 1024 partials with aggressive culling"},
    {"STANDARD 2048","2048 partials desktop standard ceiling"},
    {"TURBO 4096",   "Uncapped high-density production ceiling"}
};

// ==============================================================================
// Canonical 32 Controls Telemetry
// ==============================================================================

inline constexpr Control kControls[] = {
    // --- TIER 1: PRIMARY TIMBRE & FILTERS ---
    {ShoggothParameters::Ids::filter_cutoff,     "Filter Cutoff",     "Frequency-domain additive cutoff frequency", {}},
    {ShoggothParameters::Ids::filter_resonance,  "Filter Reso",       "Additive formant resonance Q factor", {}},
    {ShoggothParameters::Ids::spectral_tilt,     "Spectral Tilt",     "Harmonic amplitude slope in dB/octave", {}},
    {ShoggothParameters::Ids::source_morph,      "Source Morph",      "Equal-power S1 to S2 protoplasmic crossfade", {}},
    {ShoggothParameters::Ids::odd_even_bal,      "Odd / Even Bal",    "Balance between odd (-1) and even (+1) partials", {}},
    {ShoggothParameters::Ids::dissonance_warp,   "Dissonance Warp",   "Inharmonic partial exponent: f_k = f_0 * k^alpha", {}},
    {ShoggothParameters::Ids::post_ddf_cutoff,   "Post DDF Cutoff",   "Stage 2 post-summation lowpass cutoff", {}},
    {ShoggothParameters::Ids::post_ddf_reso,     "Post DDF Reso",     "Stage 2 filter resonance feedback", {}},

    // --- TIER 2: ENVELOPES & DYNAMICS ---
    {ShoggothParameters::Ids::amp_attack,        "Amp Attack",        "Master envelope attack time in seconds", {}},
    {ShoggothParameters::Ids::amp_decay,         "Amp Decay",         "Master envelope decay time in seconds", {}},
    {ShoggothParameters::Ids::amp_sustain,       "Amp Sustain",       "Master envelope sustain level", {}},
    {ShoggothParameters::Ids::amp_release,       "Amp Release",       "Master envelope release time in seconds", {}},
    {ShoggothParameters::Ids::filter_env_depth,  "Filter Env Depth",  "Modulation depth of envelope to filter cutoff", {}},
    {ShoggothParameters::Ids::pitch_punch_depth, "Pitch Punch",       "Instant attack transient knock depth in semitones", {}},
    {ShoggothParameters::Ids::glide_time,        "Glide Time",        "Legato portamento slew time in milliseconds", {}},
    {ShoggothParameters::Ids::chaos_amount,      "Chaos Amount",      "15-channel Stepped Random / Chaos Matrix depth", {}},

    // --- TIER 3: SPACE, COLOR & DIMENSION ---
    {ShoggothParameters::Ids::alpha_chorus,      "Alpha Chorus",      "In-Synthesis Quadrature Chorus mode", kAlphaChorusOptions},
    {ShoggothParameters::Ids::waterbed_depth,    "Waterbed Depth",    "Stereo phase diffusion and spatial spread", {}},
    {ShoggothParameters::Ids::swarm_coupling,    "Swarm Coupling",    "Kuramoto detune-phase coupling strength", {}},
    {ShoggothParameters::Ids::fold_wall,         "Fold Wall",         "Nyquist event horizon triangle wavefold ceiling", {}},
    {ShoggothParameters::Ids::mixer_seethe,      "Mixer Seethe",      "Asymmetric console bus drive and transformer glue", {}},
    {ShoggothParameters::Ids::master_gain_db,    "Master Gain",       "Master output trim in decibels", {}},
    {ShoggothParameters::Ids::mpe_slide_depth,   "MPE Slide Depth",   "CC74 slide modulation to formant / tilt", {}},
    {ShoggothParameters::Ids::mpe_pressure_depth,"MPE Press Depth",   "Poly-Pressure aftertouch to brightness / fold", {}},

    // --- TIER 4: PRECISION TRIMPOTS & DIGITAL SWITCHES ---
    {ShoggothParameters::Ids::osc_slop,          "Osc Slop",          "Runge-Kutta thermal wander and power drift", {}},
    {ShoggothParameters::Ids::air_absorption,    "Air Absorption",    "High-frequency acoustic distance loss damping", {}},
    {ShoggothParameters::Ids::vel_brightness,    "Vel Brightness",    "Velocity-to-spectral-brightness sensitivity", {}},
    {ShoggothParameters::Ids::modal_release,     "Modal Release",     "Frequency-dependent acoustic high-partial damping", {}},
    {ShoggothParameters::Ids::life_mode,         "Life Mode",         "Tripartite tone signature profile", kLifeModeOptions},
    {ShoggothParameters::Ids::vintage_engine_mode,"Engine Mode",      "Filter topology and historical recreation switch", kVintageEngineModeOptions},
    {ShoggothParameters::Ids::voice_mode,        "Voice Mode",        "Voice allocation and unison stacking mode", kVoiceModeOptions},
    {ShoggothParameters::Ids::cpu_tier,          "CPU Tier",          "Active partial pool ceiling selector", kCpuTierOptions}
};

inline const Control* findControl(std::string_view paramId) {
    for (const auto& c : kControls) {
        if (c.paramId == paramId)
            return &c;
    }
    return nullptr;
}

} // namespace ShoggothInfo
