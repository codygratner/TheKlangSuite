#include <juce_core/juce_core.h>
#pragma once

#include <string_view>
#include <span>
#include <array>

namespace FormlessInfo {

struct Control {
    std::string_view paramId;
    std::string_view name;
    std::string_view blurb;
};

// ==============================================================================
// 1. Role Descriptions (8 Canonical Roles)
// ==============================================================================
inline constexpr std::string_view kRoleDescriptions[8] = {
    "Bass Drum: sub thump, punch transient, and pitch sweep body",
    "Snare Drum: acoustic shell resonance and dual-wire snappy rattle",
    "Tom: acoustic drumhead tension, stick rebound, and pitch bend",
    "Handclap: multi-hand flam dispersion, room burst, and acoustic crack",
    "Hi-Hat: metallic cymbal clusters, sizzle, and optocoupler choke",
    "Cymbal: shimmering alloy wash, bell strike, and long sustain",
    "Percussion: modal resonators, rimshots, woodblocks, and metallic pings",
    "Special Effects: sci-fi zaps, chaos modulations, lasers, and drone drops"
};

// ==============================================================================
// 2. Lifeform Triad Canon Descriptions
// ==============================================================================
inline constexpr std::string_view kLifeformDescriptions[3] = {
    "surgical drum punch: pure mathematical pitch dives & linear-phase snap",
    "germanium diode clipping, aggressive transient drive & industrial bite",
    "acoustic drumhead warmth, thermal pitch sag & spongy opto-vactrol LPG"
};

// ==============================================================================
// 3. Card 2: 64 Role Parameter Controls (8 Roles x 8 Knobs)
// Sourced directly from tkdsp::FormlessSpawnVoice::kRoleParamNames[8][8]
// ==============================================================================
inline constexpr Control kRoleControls[8][8] = {
    // Role 0: BD
    {
        { "param_a", "P.CURVE",  "pitch envelope curve shape from exponential plunge to linear sweep" },
        { "param_b", "P.DEC",    "pitch envelope drop time for initial transient thump" },
        { "param_c", "SWEEP.D",  "pitch envelope modulation excursion depth in semitones" },
        { "param_d", "DECAY",    "tail decay time for low-end bass resonance" },
        { "param_e", "CLICK",    "beater transient attack click and high-frequency spike level" },
        { "param_f", "TIMBRE",   "oscillator wavefolding and harmonic overtone saturation" },
        { "param_g", "DRIVE",    "analog core saturation and asymmetric soft-clipping" },
        { "param_h", "PUNCH",    "mid-band transient punch and sub-harmonic thump weight" }
    },
    // Role 1: SD
    {
        { "param_a", "SHELL",    "wooden/metal acoustic drumhead shell fundamental pitch" },
        { "param_b", "P.DEC",    "body transient pitch drop decay time" },
        { "param_c", "SNAPPY",   "underside snare wire tension and rattle response" },
        { "param_d", "S.DECAY",  "snare rattle decay and acoustic ring duration" },
        { "param_e", "ATTACK",   "stick impact transient attack sharpness and pop" },
        { "param_f", "TONE",     "dual bandpass acoustic resonance and shell body color" },
        { "param_g", "NOISE",    "analog white noise generator level for snare buzz" },
        { "param_h", "SNAP",     "initial transient crack and rimshot click level" }
    },
    // Role 2: TM
    {
        { "param_a", "P.CURVE",  "tom pitch drop curve from steep dive to gentle bend" },
        { "param_b", "BEND.T",   "pitch bend duration and membrane rebound time" },
        { "param_c", "INTERVAL", "pitch sweep interval depth across the drum head" },
        { "param_d", "DECAY",    "resonant membrane vibration sustain and decay time" },
        { "param_e", "STICK",    "mallet/stick impact transient definition and bite" },
        { "param_f", "TENSION",  "drumhead skin tension and harmonic modal spread" },
        { "param_g", "RING",     "rim and kettle acoustic resonance ring" },
        { "param_h", "BODY",     "deep kettle cavity low-frequency body resonance" }
    },
    // Role 3: CP
    {
        { "param_a", "PRE-TIME", "micro-delay time between clustered handclaps" },
        { "param_b", "SPREAD",   "stereo and time dispersion of group handclaps" },
        { "param_c", "BURST",    "number and density of individual clapping impulses" },
        { "param_d", "TAIL",     "reverberant acoustic room tail decay time" },
        { "param_e", "FLAM",     "initial leading hand flam transient offset" },
        { "param_f", "COLOR",    "spectral tone and bandpass filtering of clap burst" },
        { "param_g", "DIFFUSE",  "stereo diffusion and phase randomization of hand impacts" },
        { "param_h", "CRACK",    "high-frequency palm slap impact transient crack" }
    },
    // Role 4: HH
    {
        { "param_a", "CLUSTER",  "metallic square-wave oscillator cluster dissonance spread" },
        { "param_b", "CHOKE",    "optocoupler vactrol closed-hat choke damping speed" },
        { "param_c", "METRIC",   "harmonic ratio and phase relationship between metallic cores" },
        { "param_d", "DECAY",    "cymbal ring and shimmer decay time" },
        { "param_e", "METALLIC", "high-frequency metallic inharmonic overtone density" },
        { "param_f", "BRIGHT",   "top-end high-shelf brightness and airy sizzle" },
        { "param_g", "SIZZLE",   "noise floor texture and bronze raspiness" },
        { "param_h", "BITE",     "initial pedal/stick strike impact transient edge" }
    },
    // Role 5: CY
    {
        { "param_a", "ALLOY",    "bronze alloy formulation and inharmonic modal frequency matrix" },
        { "param_b", "SHIMMER",  "high-frequency phase modulation and shimmer sparkle" },
        { "param_c", "DAMP",     "edge damping and acoustic decay choke rate" },
        { "param_d", "SUSTAIN",  "total wash sustain time and energy decay envelope" },
        { "param_e", "MALLET",   "felt mallet vs wooden tip stick impact hardness" },
        { "param_f", "DENSITY",  "modal resonance density and spectral bell cluster complexity" },
        { "param_g", "BELL",     "center cup bell strike emphasis and fundamental ping" },
        { "param_h", "CRASH",    "outer edge crash explosion wash and sizzle level" }
    },
    // Role 6: PC
    {
        { "param_a", "DISPERSE", "allpass phase dispersion and transient spreading" },
        { "param_b", "SWEEP",    "pitch glide and frequency modulation sweep depth" },
        { "param_c", "RATIO",    "frequency modulation harmonic modulator-to-carrier ratio" },
        { "param_d", "DECAY",    "modal resonator and acoustic body decay time" },
        { "param_e", "IMPACT",   "hard mallet impact transient click and attack snap" },
        { "param_f", "MATERIAL", "resonator material selection from wood to glass and metal" },
        { "param_g", "RING",     "inharmonic metallic bell ring and resonance feedback" },
        { "param_h", "POP",      "hollow membrane pop and cavity resonance thump" }
    },
    // Role 7: FX
    {
        { "param_a", "WARP",     "bipolar phase distortion and waveshaping asymmetry" },
        { "param_b", "SPEED",    "internal modulation and audio-rate sweep velocity" },
        { "param_c", "SWEEP",    "bipolar frequency trajectory and pitch dive excursion" },
        { "param_d", "DECAY",    "total envelope release and tail decay duration" },
        { "param_e", "TRIGGER",  "transient burst trigger rate and impulse density" },
        { "param_f", "INDEX",    "frequency modulation index and cross-modulation depth" },
        { "param_g", "CHAOS",    "nonlinear chaotic oscillator feedback and drift amount" },
        { "param_h", "EDGE",     "harmonic distortion drive, bite, and high-frequency edge" }
    }
};

// ==============================================================================
// 4. Master Controls for Cards 1 & 3 (28 Controls)
// ==============================================================================
inline constexpr Control kMasterControls[] = {
    // Card 1: Master Macro & Dynamics Lab (8)
    { "macro_x",     "X: SPECTRAL", "macro spectral spread, sweep excursion and harmonics" },
    { "macro_y",     "Y: PUNCH",    "macro transient attack spike and punch snap" },
    { "macro_z",     "Z: TIMBRE",   "macro harmonic waveshaping, fold & overtone" },
    { "macro_t",     "T: TIME",     "macro pitch envelope and body decay time" },
    { "vel_aggr",    "VEL->AGGR",   "velocity to synthesis aggression & overdrive" },
    { "vel_timb",    "VEL->TIMB",   "velocity to synthesis brightness & overtones" },
    { "pan",         "PAN",         "stereo panorama positioning" },
    { "decay_curve", "DECAY CRV",   "envelope discharge curve (exp to log)" },

    // Card 3 Pod 1: Stepped Random Humanization (8)
    { "rand_x",      "R-X",         "per-hit Macro X S&H random depth" },
    { "rand_y",      "R-Y",         "per-hit Macro Y S&H random depth" },
    { "rand_z",      "R-Z",         "per-hit Macro Z S&H random depth" },
    { "rand_t",      "R-T",         "per-hit Macro T S&H random depth" },
    { "rand_aggr",   "R-AGGR",      "per-hit velocity aggression fluctuation depth" },
    { "rand_timb",   "R-TIMB",      "per-hit velocity timbre fluctuation depth" },
    { "rand_pan",    "R-PAN",       "per-hit stereo position modulation depth" },
    { "rand_decay",  "R-DECAY",     "per-hit Master Macro Decay modulation depth" },

    // Card 3 Pod 2 & 3: Master Altar, Mojo & Output (12)
    { "macro_decay", "MACRO DECAY", "master exponential envelope decay scaling" },
    { "cutoff",      "CUTOFF",      "filter cutoff frequency" },
    { "resonance",   "RES",         "filter resonance / self-oscillation" },
    { "filter_env_amt", "FE-AMT",   "unipolar filter envelope modulation depth across 0..+10 octaves" },
    { "filter_decay", "FE-DEC",     "filter envelope decay time (5ms..5.0s)" },
    { "pre_drive",   "DRIVE",       "pre-filter analog saturation and drive" },
    { "crush",       "CRUSH",       "mojo sample rate decimation frequency" },
    { "shift",       "SHIFT",       "bode frequency shifter (+/-1000Hz)" },
    { "squash",      "SQUASH",      "bipolar dynamics: thin/snappy vs thick/huge" },
    { "wider",       "WIDER",       "mono-compatible stereo spatial widener" },
    { "volume",      "VOLUME",      "output level into the limiter (-inf..0..+12dB)" },
    { "amp_decay",   "AMP-DEC",     "master dynamic amp decay [5ms .. engine length]" },
    { "amp_velocity","VEL",         "master velocity dynamic response: 0%=flat punch, 50%=linear, 100%=exponential" }
};

static_assert(sizeof(kMasterControls) / sizeof(kMasterControls[0]) == 29,
              "kMasterControls must contain exactly 29 controls!");

inline juce::String getRoleParamBlurb(int roleIdx, int knobIdx) {
    if (roleIdx >= 0 && roleIdx < 8 && knobIdx >= 0 && knobIdx < 8) {
        auto sv = kRoleControls[roleIdx][knobIdx].blurb;
        return juce::String(sv.data(), sv.size());
    }
    return "role synthesis parameter";
}

inline const Control* findMasterControl(std::string_view paramId) noexcept {
    for (const auto& c : kMasterControls) {
        if (c.paramId == paramId)
            return &c;
    }
    return nullptr;
}

} // namespace FormlessInfo
