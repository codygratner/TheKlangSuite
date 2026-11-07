#pragma once







#include <juce_core/juce_core.h>



#include <juce_graphics/juce_graphics.h>



#include <string_view>



#include <array>



#include <vector>



#include <unordered_map>



#include "NyarlatParameterIds.h"







namespace Nyarlat {







namespace Colours {



    static const juce::Colour kChassisBg  = juce::Colour(0xFF0D1117);



    static const juce::Colour kBayBg      = juce::Colour(0xFF161B22);



    static const juce::Colour kBorder     = juce::Colour(0xFF30363D);



    static const juce::Colour kOsc        = juce::Colour(0xFF00C8F0); // Cyan

    static const juce::Colour kOsc1       = juce::Colour(0xFF00C8F0); // Movie Poster Blue

    static const juce::Colour kOsc2       = juce::Colour(0xFFFF7A1A); // Movie Poster Orange

    static const juce::Colour kOsc3       = juce::Colour(0xFFB66DFF); // Sorcerous Purple



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



        { Ids::filter_cutoff, "Filter Cutoff", "analog filter cutoff frequency", Colours::kFilter },



        { Ids::filter_resonance, "Filter Resonance", "filter liquid resonance and peak emphasis", Colours::kFilter },



        { Ids::filter_slope, "Filter Slope", "filter slope and topology mode", Colours::kFilter },



        { Ids::filter_drive, "Filter Drive", "analog tube warmth and saturating drive", Colours::kFilter },



        { Ids::filter_attack, "Filter Attack", "filter envelope attack duration", Colours::kFilter },



        { Ids::filter_decay, "Filter Decay", "filter envelope decay time", Colours::kFilter },



        { Ids::filter_sustain, "Filter Sustain", "filter envelope sustain plateau level", Colours::kFilter },



        { Ids::filter_release, "Filter Release", "filter envelope release fade duration", Colours::kFilter },



        { Ids::filter_env_depth, "Filter Env Depth", "envelope modulation depth into filter cutoff", Colours::kFilter },



        { Ids::filter_velocity, "Filter Velocity", "keyboard velocity modulation of filter env", Colours::kFilter },



        { Ids::filter_keytrack, "Filter Key Track", "keyboard pitch tracking into filter cutoff", Colours::kFilter },



        { Ids::osc1_morph, "Osc 1 Morph", "additive seed spectrum crossfade", Colours::kOsc },



        { Ids::osc1_odd_even, "Osc 1 Odd / Even", "hollow odd harmonics vs warm even harmonics balance", Colours::kOsc },



        { Ids::osc1_tilt, "Osc 1 Tilt", "additive spectral slope dB/octave", Colours::kOsc },



        { Ids::osc1_formant, "Osc 1 Formant", "additive harmonic peak formant cluster", Colours::kOsc },



        { Ids::osc1_mask, "Osc 1 Mask", "bitmask stator bitwise truncation aperture", Colours::kOsc },



        { Ids::osc1_speed, "Osc 1 Speed", "Casio CZ resonant stator sweep speed (0-10 Hz)", Colours::kOsc },






        { Ids::osc1_octave, "Osc 1 Octave", "osc 1 octave parameter", Colours::kOsc },



        { Ids::osc1_speed_lock, "Osc 1 Speed Lock", "lock stator sweep to 1/16th note tempo grid", Colours::kChaos },



        { Ids::osc1_loop, "Osc 1 Mask Loop", "osc 1 mask loop parameter", Colours::kOsc },



        { Ids::osc1_bank, "Osc 1 Seed Bank", "osc 1 seed bank parameter", Colours::kOsc },



        { Ids::osc1_level, "Osc 1 Level", "osc 1 level parameter", Colours::kMixer },



        { Ids::osc2_morph, "Osc 2 Morph", "additive seed spectrum crossfade", Colours::kOsc },



        { Ids::osc2_odd_even, "Osc 2 Odd / Even", "hollow odd harmonics vs warm even harmonics balance", Colours::kOsc },



        { Ids::osc2_tilt, "Osc 2 Tilt", "additive spectral slope dB/octave", Colours::kOsc },



        { Ids::osc2_formant, "Osc 2 Formant", "additive harmonic peak formant cluster", Colours::kOsc },



        { Ids::osc2_mask, "Osc 2 Mask", "bitmask stator bitwise truncation aperture", Colours::kOsc },



        { Ids::osc2_speed, "Osc 2 Speed", "Casio CZ resonant stator sweep speed (0-10 Hz)", Colours::kOsc },






        { Ids::osc2_octave, "Osc 2 Octave", "osc 2 octave parameter", Colours::kOsc },



        { Ids::osc2_speed_lock, "Osc 2 Speed Lock", "lock stator sweep to 1/16th note tempo grid", Colours::kChaos },






        { Ids::osc2_loop, "Osc 2 Mask Loop", "osc 2 mask loop parameter", Colours::kOsc },



        { Ids::osc2_bank, "Osc 2 Seed Bank", "osc 2 seed bank parameter", Colours::kOsc },



        { Ids::osc2_level, "Osc 2 Level", "osc 2 level parameter", Colours::kMixer },



        { Ids::osc3_morph, "Osc 3 Morph", "additive seed spectrum crossfade", Colours::kOsc },



        { Ids::osc3_odd_even, "Osc 3 Odd / Even", "hollow odd harmonics vs warm even harmonics balance", Colours::kOsc },



        { Ids::osc3_tilt, "Osc 3 Tilt", "additive spectral slope dB/octave", Colours::kOsc },



        { Ids::osc3_formant, "Osc 3 Formant", "additive harmonic peak formant cluster", Colours::kOsc },



        { Ids::osc3_mask, "Osc 3 Mask", "bitmask stator bitwise truncation aperture", Colours::kOsc },



        { Ids::osc3_speed, "Osc 3 Speed", "Casio CZ resonant stator sweep speed (0-10 Hz)", Colours::kOsc },






        { Ids::osc3_octave, "Osc 3 Octave", "osc 3 octave parameter", Colours::kOsc },



        { Ids::osc3_speed_lock, "Osc 3 Speed Lock", "lock stator sweep to 1/16th note tempo grid", Colours::kChaos },






        { Ids::osc3_loop, "Osc 3 Mask Loop", "osc 3 mask loop parameter", Colours::kOsc },



        { Ids::osc3_bank, "Osc 3 Seed Bank", "osc 3 seed bank parameter", Colours::kOsc },



        { Ids::osc3_level, "Osc 3 Level", "osc 3 level parameter", Colours::kMixer },



        { Ids::noise_level, "Noise Level", "noise level parameter", Colours::kMixer },



        { Ids::noise_tone, "Noise Tone", "noise tone parameter", Colours::kMixer },



        { Ids::noise_hammer_snap, "Clav Hammer Snap", "percussive clavinet string hammer strike click", Colours::kMixer },



        { Ids::osc_spread, "Osc Pan Spread", "osc pan spread parameter", Colours::kMixer },



        { Ids::mixer_seethe, "Console Warmth", "console warmth parameter", Colours::kMixer },



        { Ids::chaos_rate, "Chaos Rate", "master chaotic attractor evolution clock rate (10m to 1s)", Colours::kChaos },



        { Ids::chaos_drift, "Chaos Drift", "blend from stepped S&H to continuous brownian wander", Colours::kChaos },



        { Ids::chaos_poly_spread, "Poly Chaos Spread", "polyphonic voice seed chaos decorrelation", Colours::kChaos },



        { Ids::rand_spread, "Spread Chaos", "chaos modulation depth into stereo spread swimming", Colours::kChaos },

        { Ids::rand_tune, "Chaos Detune", "symmetric opposite chaos pitch modulation for Osc 2 and Osc 3", Colours::kChaos },

        { Ids::rand_chorus_rate, "Chaos Chorus Rate", "Hermite chaos frequency modulation depth into Alpha Chorus LFO rate", Colours::kChaos },

        { Ids::rand_morph, "Chaos Morph", "additive seed spectrum chaos crossfade depth across all oscillators", Colours::kChaos },
        { Ids::rand_odd_even, "Chaos Odd/Even", "hollow odd harmonics vs warm even harmonics chaos balance", Colours::kChaos },
        { Ids::rand_tilt, "Chaos Tilt", "additive spectral slope dB/octave chaos modulation depth", Colours::kChaos },
        { Ids::rand_formant, "Chaos Formant", "additive harmonic peak formant cluster chaos modulation depth", Colours::kChaos },
        { Ids::rand_mask, "Chaos Mask", "unipolar chaos modulation depth into bitmask stator across all oscillators", Colours::kChaos },
        { Ids::rand_speed, "Chaos Speed", "unipolar chaos modulation depth into wavetable speed across all oscillators", Colours::kChaos },

        { Ids::amp_attack, "Amp Attack", "amp attack parameter", Colours::kVca },



        { Ids::amp_decay, "Amp Decay", "amp decay parameter", Colours::kVca },



        { Ids::amp_sustain, "Amp Sustain", "amp sustain parameter", Colours::kVca },



        { Ids::amp_release, "Amp Release", "amp release parameter", Colours::kVca },



        { Ids::amp_velocity, "Amp Velocity", "amp velocity parameter", Colours::kVca },



        { Ids::keytrack_rate, "Keytrack Rate", "keytrack rate parameter", Colours::kVca },



        { Ids::glide_time, "Glide Time", "glide time parameter", Colours::kVca },



        { Ids::mpe_slide_depth, "MPE Slide (Y) Depth", "mpe slide (y) depth parameter", Colours::kVca },



        { Ids::mpe_pressure_depth, "MPE Press (Z) Depth", "mpe press (z) depth parameter", Colours::kVca },



        { Ids::pitch_bend_range, "Pitch Bend Range", "pitch bend range parameter", Colours::kVca },



        { Ids::bass_boost, "40Hz Bass Boost", "40hz bass boost parameter", Colours::kMaster },



        { Ids::treble_tame, "Treble Tame", "treble tame parameter", Colours::kMaster },



        { Ids::alpha_chorus, "Chorus Amount", "continuous zero-latency quadrature phase chorus depth and wet mix", Colours::kMaster },
        { Ids::chorus_rate, "Chorus Rate", "quadrature phase chorus LFO modulation speed (0.1 to 8.0 Hz)", Colours::kMaster },

        { Ids::osc_slop, "Voice Drift / Slop", "voice drift / slop parameter", Colours::kMaster },



        { Ids::global_width, "Stereo Width", "stereo width parameter", Colours::kMaster },



        { Ids::output_level, "Master Volume", "master volume parameter", Colours::kMaster },



        { Ids::voice_mode, "Voicing Mode", "voicing mode parameter", Colours::kVca },



        { Ids::life_mode, "Life Mode", "triad sonic canon character: synthetic, cyborg, or organic", Colours::kSynthetic },



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



