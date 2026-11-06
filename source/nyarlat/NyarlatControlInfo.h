#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <string_view>
#include <array>
#include <unordered_map>
#include "NyarlatParameterIds.h"

namespace Nyarlat {

namespace Colours {
    static const juce::Colour kChassisBg  = juce::Colour(0xFF0D1117);
    static const juce::Colour kBayBg      = juce::Colour(0xFF161B22);
    static const juce::Colour kBorder     = juce::Colour(0xFF30363D);
    static const juce::Colour kOsc        = juce::Colour(0xFF00C8F0); // Cyan
    static const juce::Colour kMixer      = juce::Colour(0xFFFF3347); // Red
    static const juce::Colour kChaos      = juce::Colour(0xFFF2E10F); // Yellow
    static const juce::Colour kFilter     = juce::Colour(0xFF7DFF3B); // Acid Green
    static const juce::Colour kVca        = juce::Colour(0xFFFF7A1A); // Orange
    static const juce::Colour kMaster     = juce::Colour(0xFFB66DFF); // Purple
    static const juce::Colour kTextWhite  = juce::Colour(0xFFF0F6FC);

    // Standing Triad Sonic Canon Invariants
    static const juce::Colour kSynthetic  = juce::Colour(0xFF00C8FF); // Tron Blue
    static const juce::Colour kCyborg     = juce::Colour(0xFFE08C00); // Toxic Amber / Rad-Waste Yellow
    static const juce::Colour kOrganic    = juce::Colour(0xFF6BBF59); // Warm Bioluminescent Green
}

struct ControlInfo {
    const char* paramId;
    const char* name;
    const char* blurb;
    juce::Colour accent;
};

inline const std::vector<ControlInfo>& getNyarlatControlCatalog() {
    namespace Ids = nyarlatParameters::Ids;
    static const std::vector<ControlInfo> catalog = {
        // OSC 1
        { Ids::osc1_bank, "O1 BANK", "select AKWF ROM single-cycle waveform bank", Colours::kOsc },
        { Ids::osc1_wave, "O1 WAVE", "interpolate across waveforms in active bank", Colours::kOsc },
        { Ids::osc1_mask, "O1 MASK", "bitmask stator bitwise truncation aperture", Colours::kOsc },
        { Ids::osc1_speed, "O1 SPD", "bitmask stator pattern sweep speed (0-10 Hz)", Colours::kOsc },
        { Ids::osc1_loop, "O1 LOOP", "stator pattern sweep trajectory mode", Colours::kOsc },
        { Ids::osc1_octave, "O1 OCT", "coarse octave transposition offset", Colours::kOsc },
        { Ids::osc1_detune, "O1 TUNE", "fine cents micro-tuning detune", Colours::kOsc },
        { Ids::osc1_oct_lock, "O1 LOCK", "octave lock pitch stabilization table", Colours::kOsc },

        // OSC 2
        { Ids::osc2_bank, "O2 BANK", "select AKWF ROM single-cycle waveform bank", Colours::kOsc },
        { Ids::osc2_wave, "O2 WAVE", "interpolate across waveforms in active bank", Colours::kOsc },
        { Ids::osc2_mask, "O2 MASK", "bitmask stator bitwise truncation aperture", Colours::kOsc },
        { Ids::osc2_speed, "O2 SPD", "bitmask stator pattern sweep speed (0-10 Hz)", Colours::kOsc },
        { Ids::osc2_loop, "O2 LOOP", "stator pattern sweep trajectory mode", Colours::kOsc },
        { Ids::osc2_octave, "O2 OCT", "coarse octave transposition offset", Colours::kOsc },
        { Ids::osc2_detune, "O2 TUNE", "fine cents micro-tuning detune", Colours::kOsc },
        { Ids::osc2_oct_lock, "O2 LOCK", "octave lock pitch stabilization table", Colours::kOsc },
        { Ids::osc2_sync, "O2 SYNC", "hard sync to oscillator 1 master phase", Colours::kOsc },

        // OSC 3
        { Ids::osc3_bank, "O3 BANK", "select AKWF ROM single-cycle waveform bank", Colours::kOsc },
        { Ids::osc3_wave, "O3 WAVE", "interpolate across waveforms in active bank", Colours::kOsc },
        { Ids::osc3_mask, "O3 MASK", "bitmask stator bitwise truncation aperture", Colours::kOsc },
        { Ids::osc3_speed, "O3 SPD", "bitmask stator pattern sweep speed (0-10 Hz)", Colours::kOsc },
        { Ids::osc3_loop, "O3 LOOP", "stator pattern sweep trajectory mode", Colours::kOsc },
        { Ids::osc3_octave, "O3 OCT", "coarse octave transposition offset", Colours::kOsc },
        { Ids::osc3_detune, "O3 TUNE", "fine cents micro-tuning detune", Colours::kOsc },
        { Ids::osc3_oct_lock, "O3 LOCK", "octave lock pitch stabilization table", Colours::kOsc },
        { Ids::osc3_sync, "O3 SYNC", "hard sync to oscillator 1 master phase", Colours::kOsc },

        // MIXER
        { Ids::osc1_level, "OSC 1", "oscillator 1 stem level into console mixer", Colours::kMixer },
        { Ids::osc2_level, "OSC 2", "oscillator 2 stem level into console mixer", Colours::kMixer },
        { Ids::osc3_level, "OSC 3", "oscillator 3 stem level into console mixer", Colours::kMixer },
        { Ids::noise_level, "NOISE", "variable-rate sample & hold noise level", Colours::kMixer },
        { Ids::noise_tone, "TONE", "noise clock downsampling & decimation rate", Colours::kMixer },
        { Ids::osc_spread, "SPREAD", "stereo pan spread across oscillators 1, 2, 3", Colours::kMixer },
        { Ids::mixer_seethe, "WARMTH", "console seethe asymmetric soft saturation", Colours::kMixer },

        // CHAOS MASTER & ATTENUVERTERS
        { Ids::chaos_rate, "RATE", "master chaotic attractor evolution clock rate", Colours::kChaos },
        { Ids::chaos_drift, "DRIFT", "brownian random walk micro-pitch wandering", Colours::kChaos },
        { Ids::chaos_poly_spread, "POLY", "polyphonic voice seed chaos decorrelation", Colours::kChaos },

        { Ids::rand_o1_wave, "O1 WAV", "chaos modulation depth into osc 1 wave morph", Colours::kChaos },
        { Ids::rand_o1_mask, "O1 MSK", "chaos modulation depth into osc 1 mask", Colours::kChaos },
        { Ids::rand_o1_speed, "O1 SPD", "chaos modulation depth into osc 1 speed", Colours::kChaos },
        { Ids::rand_o2_wave, "O2 WAV", "chaos modulation depth into osc 2 wave morph", Colours::kChaos },
        { Ids::rand_o2_mask, "O2 MSK", "chaos modulation depth into osc 2 mask", Colours::kChaos },
        { Ids::rand_o2_speed, "O2 SPD", "chaos modulation depth into osc 2 speed", Colours::kChaos },
        { Ids::rand_o3_wave, "O3 WAV", "chaos modulation depth into osc 3 wave morph", Colours::kChaos },
        { Ids::rand_o3_mask, "O3 MSK", "chaos modulation depth into osc 3 mask", Colours::kChaos },
        { Ids::rand_o3_speed, "O3 SPD", "chaos modulation depth into osc 3 speed", Colours::kChaos },

        { Ids::rand_z_freq, "Z FREQ", "chaos modulation depth into Z-filter freq", Colours::kChaos },
        { Ids::rand_z_morph, "Z MRPH", "chaos modulation depth into Z-cube morph", Colours::kChaos },
        { Ids::rand_z_transform, "Z TRAN", "chaos modulation depth into Z transform", Colours::kChaos },
        { Ids::rand_seethe, "SEETHE", "chaos modulation depth into console warmth", Colours::kChaos },
        { Ids::rand_amp_decay, "AMP DEC", "chaos modulation depth into VCA decay", Colours::kChaos },
        { Ids::rand_spread, "RDM SPREAD", "chaos modulation depth into bipolar stereo spread swimming", Colours::kChaos },

        // 14-POLE Z-FILTER
        { Ids::z_model, "MODEL", "14-pole Z-domain parameter cube geometry model", Colours::kFilter },
        { Ids::z_freq, "Z-FREQ", "Z-domain resonant pole center frequency", Colours::kFilter },
        { Ids::z_morph, "Z-MORPH", "Y-axis parameter morph across cube corners", Colours::kFilter },
        { Ids::z_transform, "Z-TRANS", "Z-axis parameter transform & pole shift", Colours::kFilter },
        { Ids::filter_attack, "F-ATK", "filter envelope attack duration", Colours::kFilter },
        { Ids::filter_decay, "F-DEC", "filter envelope decay time", Colours::kFilter },
        { Ids::filter_sustain, "F-SUS", "filter envelope sustain plateau level", Colours::kFilter },
        { Ids::filter_release, "F-REL", "filter envelope release fade duration", Colours::kFilter },
        { Ids::filter_env_depth, "ENV DEP", "envelope modulation depth into Z-frequency", Colours::kFilter },
        { Ids::filter_velocity, "VEL", "keyboard velocity modulation of filter env", Colours::kFilter },
        { Ids::filter_lfo_shape, "LFO SHP", "secondary filter LFO waveform shape", Colours::kFilter },
        { Ids::filter_lfo_dest, "LFO DST", "secondary filter LFO routing destination", Colours::kFilter },
        { Ids::filter_lfo_rate, "LFO SPD", "Z-filter secondary LFO modulation rate", Colours::kFilter },
        { Ids::filter_lfo_depth, "LFO AMT", "Z-filter secondary LFO modulation depth", Colours::kFilter },
        { Ids::filter_keytrack, "KEY TRK", "keyboard pitch tracking into Z-frequency", Colours::kFilter },

        // VCA & VOICING
        { Ids::amp_attack, "ATTACK", "VCA amplitude envelope attack time", Colours::kVca },
        { Ids::amp_decay, "DECAY", "VCA amplitude envelope decay duration", Colours::kVca },
        { Ids::amp_sustain, "SUSTAIN", "VCA amplitude envelope sustain level", Colours::kVca },
        { Ids::amp_release, "RELEASE", "VCA amplitude envelope release tail time", Colours::kVca },
        { Ids::amp_velocity, "VELOCITY", "keyboard velocity dynamic response", Colours::kVca },
        { Ids::keytrack_rate, "KT RATE", "envelope decay keytrack scaling across keyboard", Colours::kVca },
        { Ids::glide_time, "GLIDE", "portamento polyphonic pitch glide time", Colours::kVca },
        { Ids::voice_mode, "VOICING", "polyphonic voice allocation mode", Colours::kVca },
        { Ids::mpe_slide_depth, "MPE SLD", "MPE Y-axis timbre slide modulation depth", Colours::kVca },
        { Ids::mpe_pressure_depth, "MPE PRS", "MPE channel pressure / aftertouch depth", Colours::kVca },
        { Ids::pitch_bend_range, "BEND", "pitch bend wheel semitone range", Colours::kVca },

        // MASTER & SPACE
        { Ids::bass_boost, "40Hz BASS", "tight resonant 40Hz sub-bass bandpass boost", Colours::kMaster },
        { Ids::treble_tame, "TREBLE", "one-pole high frequency tape roll-off damping", Colours::kMaster },
        { Ids::reverb_time, "TIME", "abyss algorithmic reverb decay time", Colours::kMaster },
        { Ids::reverb_damp, "DAMP", "high-frequency absorption damping in reverb tail", Colours::kMaster },
        { Ids::reverb_mix, "REVERB", "reverb wet/dry blend ratio into master", Colours::kMaster },
        { Ids::alpha_chorus, "CHORUS", "dual delay line BBD chorus rate & mix", Colours::kMaster },
        { Ids::osc_slop, "SLOP", "inter-voice thermal oscillator drift variance", Colours::kMaster },
        { Ids::global_width, "WIDTH", "mid/side stereo image expander / collapser", Colours::kMaster },
        { Ids::output_level, "VOLUME", "master output level with soft tanh saturation", Colours::kMaster },
        { Ids::life_mode, "Lifeform", "triad sonic canon character: synthetic, cyborg, or organic", Colours::kSynthetic }
    };
    return catalog;
}

inline const ControlInfo* findNyarlatControlInfo(const juce::String& paramId) {
    const auto& catalog = getNyarlatControlCatalog();
    for (const auto& c : catalog) {
        if (paramId == c.paramId) return &c;
    }
    return nullptr;
}

} // namespace Nyarlat
