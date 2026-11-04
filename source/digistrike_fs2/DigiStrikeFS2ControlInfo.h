#pragma once

#include <string_view>
#include <array>
#include <span>

// ==============================================================================
// DigiStrikeFS2ControlInfo.h — Single Source of Truth for Gear Manual Inspector Blurbs
// ==============================================================================
// Part of The Klang Suite SOCEP Engine. Copyright (c) 2026 Klang Industries.
// Written by The Writer wearing his Gear Manual Hat.
// ==============================================================================

namespace DigiStrikeFS2Info {

struct Control {
    std::string_view paramId;
    std::string_view name;
    std::string_view blurb;
};

// ==============================================================================
// 1. Header Selectors & Lifeform Descriptions
// ==============================================================================
inline constexpr std::string_view kCategoryDescriptions[8] = {
    "Bass Drum: vintage punch, 555 pitch clock dive, and deep sub resonance",
    "Snare Drum: acoustic shell body, gated digital rattle, and snappy wire crack",
    "Tom: electronic drumhead tension, Simmons hexagonal bite, and pitch sweep",
    "Handclap: multi-sample flam cluster, 8-bit digital snap, and claptrap burst",
    "Hi-Hat: sizzling metallic clusters, variable clock sizzle, and choked decays",
    "Cymbal: shimmering bronze alloy wash, bell strikes, and long ambient decays",
    "Percussion: Latin cowbells, congas, rimshots, woodblocks, and metallic pings",
    "Stabs & Bass: 12-bit orchestral hits, rave stabs, slap bass, and digital synth growl"
};

inline constexpr std::string_view kLineageDescriptions[21] = {
    "All Lineages: complete 2,069-voice library from all 20 sovereign manufacturers",
    "Crown Audio: vintage LM-1, LinnDrum, and 9000 companded studio voices",
    "Apex Electronics: 8-bit companded DMX and DX punch with discrete EPROM grit",
    "Empyrean Systems: 12-bit SP-1200 drop-sample slew and Drumulator crunch",
    "Red Falcon: 16-bit 44.1kHz MPC60, XR-10, and XE8 tight studio punch",
    "K-Works: 12-bit K-Beat and K-Electro 50 filtered hits and iconic slap basses",
    "Compact Line: 12-bit DR-110, DR-220, and DR-550 punchy street grooveboxes",
    "Tokyo Beat: linear matrix rhythm voices from Matrix-70, Hybrid-90, and HumanRhythm-8",
    "Gothenburg Lab: Swedish algorithmic digital percussion and 12-bit E12 crunch",
    "Circuits Core: Sequential Drumtraks, TOM, and Studio 440 vintage EPROM slam",
    "Phase Tone: Casio RZ-1, SK-1, and FZ-1 lo-fi sampling and PCM crunch",
    "Studio Line: Alesis HR-16, HR-16B, and SR-16 iconic 90s studio drum machines",
    "Keio Systems: Korg DDD-1, DDD-5, and S3 dynamic velocity multi-card PCM",
    "Nippon Sound: 12-bit waveform memory from RX5, RX11, and dynamic RY30 filters",
    "Adios Engine: SAM3308B multi-timbral PCM percussion and dark synth growls",
    "Fairlight Series: CMI Series IIx/III iconic 8-bit page-R orchestral hits and snares",
    "Synclavier Tech: NED Synclavier II high-precision direct-to-disk digital PCM",
    "Ensoniq Matrix: ASR-10 and EPS-16+ gritty variable-rate drop-sample drums",
    "Simmons Hex: SDS-V, SDS-7, and SDS-9 analog noise click and hexagonal bite",
    "Jomox Analog: XBase 09 and AirBase 99 punchy German hybrid kick and hats",
    "Curio Fleet: Bizarre custom EPROMs, modified romplers, and circuit-bent classics"
};

inline constexpr std::array<const char*, 20> kLineageParamIds = {
    "lin_crown", "lin_apex", "lin_empyr", "lin_falcon", "lin_kwrk",
    "lin_compact", "lin_tokyo", "lin_gothn", "lin_circuits", "lin_phase",
    "lin_studio", "lin_keio", "lin_nippon", "lin_adios", "lin_fairlight",
    "lin_synclavier", "lin_ensoniq", "lin_simmons", "lin_jomox", "lin_curios"
};

inline constexpr std::string_view kVoiceModeDescriptions[3] = {
    "Mono Retrig: classic drum choke with 2ms anti-pop crossfade on re-trigger",
    "Mono Glide: legato bass mode with log-pitch exponential portamento glide",
    "Poly 8: 8-voice polyphony for orchestral stabs, rave chords, and ringing cymbals"
};

inline constexpr std::string_view kLifeformDescriptions[3] = {
    "Synthetic: pristine 32-bit float interpolation with linear phase and crisp highs",
    "Cyborg: Am6070 u-law 8-bit companding combined with Curtis CEM3320 analog warmth",
    "Organic: SP-1200 26.04kHz drop-sampling with analog 555-timer jitter and thermal drift"
};

// ==============================================================================
// 2. Card 1: Master Macro & Dynamics Lab (8 Controls)
// ==============================================================================
inline constexpr Control kMacroControls[8] = {
    { "macro_x",    "MACRO-X",   "spectral morph sweeping variable clock pitch and Curtis CEM3320 cutoff" },
    { "macro_y",    "MACRO-Y",   "punch macro intensifying transient pitch dive depth and pre-filter drive" },
    { "macro_z",    "MACRO-Z",   "timbre macro sweeping 3D wavetable position and vintage DAC companding depth" },
    { "macro_t",    "MACRO-T",   "time macro modulating wavetable frame traverse rate and master amp decay" },
    { "vel_aggr",   "VEL-AGGR",  "dynamic velocity scaling routed to transient pitch dive and saturation drive" },
    { "vel_timb",   "VEL-TIMB",  "dynamic velocity scaling routed to 3D wavetable morph and filter cutoff" },
    { "pan",        "PAN",       "bipolar stereo voice panorama positioning across the master mix" },
    { "decay_curve","DECAY-CRV", "exponential-to-logarithmic curvature skew for amplitude envelope decay" }
};

// ==============================================================================
// 3. Card 2: Uniform Sound Lab (8 Controls in 4 Functional Pods)
// Col 1: P.DEC (K01) / PUNCH (K05)   - Pitch Envelope Pod
// Col 2: CLICK (K02) / COMPAND (K06) - Transient Attack & DAC Mojo Pod
// Col 3: START (K03) / LENGTH (K07)  - Wavetable / Sample Window Pod
// Col 4: TIME (K04)  / MORPH (K08)   - Traversal Motion & Spectral Slice Pod
// ==============================================================================
inline constexpr Control kCard2Controls[8] = {
    // Row 1 (K01..K04)
    { "pitch_decay",   "P.DEC",   "pitch dive decay duration from snappy beater click to laser tom sweep" },
    { "click_bleed",   "CLICK",   "beater impact transient click accent and analog hardware white noise bleed" },
    { "start_offset",  "START",   "transient skip offset isolating warm body and fundamental tone" },
    { "traverse_rate", "TIME",    "bidirectional frame traverse rate from reverse swell to varispeed forward" },
    // Row 2 (K05..K08)
    { "pitch_dive",    "PUNCH",   "quadratic transient pitch dive depth for authentic 80s drum knock" },
    { "noise_mix",     "NOISE",   "enveloped 1-pole 800Hz highpass noise mix (35% amp env + 65% pitch attack)" },
    { "sample_length", "LENGTH",  "playback window and scan span from single-cycle frame to full sample" },
    { "table_morph",   "MORPH",   "3D wavetable timbre slice position with smoothed inter-frame morphing" }
};

// ==============================================================================
// 4. Card 3: Stepped Random Humanization (8 Controls)
// ==============================================================================
inline constexpr Control kRandomControls[8] = {
    { "rand_x",     "R-X",     "per-trigger latched pseudo-random wander applied to Spectral Macro X" },
    { "rand_y",     "R-Y",     "per-trigger latched pseudo-random wander applied to Punch Macro Y" },
    { "rand_z",     "R-Z",     "per-trigger latched pseudo-random wander applied to Timbre Macro Z" },
    { "rand_t",     "R-T",     "per-trigger latched pseudo-random wander applied to Time Macro T" },
    { "rand_aggr",  "R-AGGR",  "per-trigger random variation applied to velocity aggression depth" },
    { "rand_timb",  "R-TIMB",  "per-trigger random variation applied to velocity timbre sensitivity" },
    { "rand_pan",   "R-PAN",   "per-trigger pseudo-random spatial stereo field wandering" },
    { "rand_decay", "R-DECAY", "per-trigger micro-slop applied to amplitude envelope decay duration" }
};

// ==============================================================================
// 5. Card 3: Master Altar & Master Output Strip (4 Controls)
// ==============================================================================
inline constexpr Control kAltarControls[1] = {
    { "macro_decay", "MACRO",   "master multiplicative scaling factor across all dynamic envelopes" }
};

inline constexpr Control kMasterControls[4] = {
    { "volume",        "VOLUME",  "master logarithmic output level with authentic analog headroom trim" },
    { "sustain_level", "SUSTAIN", "gated sustain level with equal-power ping-pong micro-grain looping" },
    { "amp_decay",     "AMP-DEC", "master amplitude decay duration and gated note release decay time" },
    { "glide_time",    "GLIDE",   "exponential portamento glide duration for authentic legato synth basslines" }
};

// ==============================================================================
// 6. Card 3: Master Bus Mojo Console (11 Controls)
// ==============================================================================
inline constexpr Control kMojoControls[11] = {
    { "cutoff",         "CUTOFF",  "Curtis CEM3320 24dB/oct Zero-Delay Feedback resonant reconstruction VCF" },
    { "resonance",      "RES",     "CEM3320 filter resonance peak and acoustic drumhead cavity ring" },
    { "filter_env_amt", "FE-AMT",  "bipolar filter envelope modulation depth across +/-5 octaves" },
    { "filter_decay",   "FE-DEC",  "exponential decay duration of the dedicated filter envelope" },
    { "filter_type",    "TYPE",    "4-pole filter topology selection: Lowpass, Highpass, Bandpass, or Peak" },
    { "pre_drive",      "DRIVE",   "non-linear hyperbolic tangent pre-filter saturator and overdrive warmth" },
    { "crush",          "CRUSH",   "Machinedrum sample-and-hold audio decimation independent of pitch clock" },
    { "shift",          "SHIFT",   "Bode analog frequency shifter introducing inharmonic metallic sidebands" },
    { "squash",         "SQUASH",  "bipolar VCA dynamics processor from punchy expansion to brickwall compression" },
    { "wider",          "WIDER",   "Haas inter-aural time difference and comb stereo widening stage" },
    { "master_limiter", "LIMIT",   "true-peak master output safety limiter preventing digital overs" }
};

[[nodiscard]] inline const Control* find(std::string_view paramId) noexcept {
    for (const auto& c : kMacroControls)  if (c.paramId == paramId) return &c;
    for (const auto& c : kCard2Controls)  if (c.paramId == paramId) return &c;
    for (const auto& c : kRandomControls) if (c.paramId == paramId) return &c;
    for (const auto& c : kAltarControls)  if (c.paramId == paramId) return &c;
    for (const auto& c : kMasterControls) if (c.paramId == paramId) return &c;
    for (const auto& c : kMojoControls)   if (c.paramId == paramId) return &c;
    return nullptr;
}

} // namespace DigiStrikeFS2Info
