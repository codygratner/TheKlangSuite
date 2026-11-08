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







// ==============================================================================
// Lifeform Triad Canon Descriptions (The Writer - Gear Manual Hat)
// ==============================================================================
inline constexpr std::string_view kLifeformDescriptions[3] = {
    "pure additive partials: zero phase jitter & pristine harmonic clarity",
    "bitwise phase-mask tearing, odd-harmonic fold & gritty digital crunch",
    "brownian chaotic pitch drift, warm even harmonics & vintage BBD chorus"
};

inline const std::vector<ControlInfo>& getNyarlatControlCatalog() {
    namespace Ids = nyarlatParameters::Ids;
    static const std::vector<ControlInfo> catalog = {
        // 1. Cascaded Analog Filter (Bay 3)
        { Ids::filter_cutoff,     "Cutoff",        "corner cutoff frequency of the 4-pole cascaded ladder",            Colours::kFilter },
        { Ids::filter_resonance,  "Resonance",     "resonant peak sharpness with liquid self-oscillation howl",        Colours::kFilter },
        { Ids::filter_slope,      "Slope",         "filter pole steepness: 12dB 2-pole vs 24dB 4-pole slope",          Colours::kFilter },
        { Ids::filter_drive,      "Drive",         "pre-filter asymmetric saturation and analog tube-style drive",     Colours::kFilter },
        { Ids::filter_attack,     "F.Attack",      "filter envelope rise time from instant bite to slow swell",        Colours::kFilter },
        { Ids::filter_decay,      "F.Decay",       "filter cutoff decay time falling to sustain plateau",              Colours::kFilter },
        { Ids::filter_sustain,    "F.Sustain",     "filter envelope steady-state held cutoff frequency level",         Colours::kFilter },
        { Ids::filter_release,    "F.Release",     "filter cutoff close duration after note release",                  Colours::kFilter },
        { Ids::filter_env_depth,  "Env Depth",     "bipolar envelope modulation amount to filter cutoff",              Colours::kFilter },
        { Ids::filter_velocity,   "Velocity",      "dynamic keyboard velocity scaling of filter envelope depth",       Colours::kFilter },
        { Ids::filter_keytrack,   "Key Track",     "1V/oct keyboard tracking ratio scaling cutoff with note pitch",    Colours::kFilter },

        // 2. Tri-Oscillator Additive Wavetables (Bays 1 & 2)
        // Oscillator 1
        { Ids::osc1_morph,        "Osc 1 Morph",   "continuous wavetable morphing across harmonic seed spectra",       Colours::kOsc1 },
        { Ids::osc1_odd_even,     "Osc 1 Even",    "hollow square odd harmonics vs warm saw even harmonics balance",   Colours::kOsc1 },
        { Ids::osc1_tilt,         "Osc 1 Tilt",    "spectral tilt: dark low-end warmth vs bright treble presence",     Colours::kOsc1 },
        { Ids::osc1_formant,      "Osc 1 Vowel",   "vocal formant resonance peak sweep across upper harmonics",        Colours::kOsc1 },
        { Ids::osc1_mask,         "Osc 1 Mask",    "bitwise phase-masking aperture creating gritty digital crunch",    Colours::kOsc1 },
        { Ids::osc1_speed,        "Osc 1 Speed",   "wavetable sweep cycle speed across harmonic spectra (0-10 Hz)",   Colours::kOsc1 },
        { Ids::osc1_octave,       "Osc 1 Oct",     "coarse pitch transposition: -2, -1, unison, or +1 octave",         Colours::kOsc1 },
        { Ids::osc1_speed_lock,   "Speed Lock",    "syncs wavetable sweep speed to host 16th-note tempo grid",         Colours::kChaos },
        { Ids::osc1_loop,         "Sweep Loop",    "wavetable sweep direction: saw up, triangle, or saw down",         Colours::kOsc1 },
        { Ids::osc1_bank,         "Seed Bank",     "harmonic seed bank: clean VA, vowel formants, metallic, or bell",  Colours::kOsc1 },
        { Ids::osc1_level,        "Osc 1 Level",   "oscillator output level injected into the console mixer",          Colours::kMixer },

        // Oscillator 2
        { Ids::osc2_morph,        "Osc 2 Morph",   "continuous wavetable morphing across harmonic seed spectra",       Colours::kOsc2 },
        { Ids::osc2_odd_even,     "Osc 2 Even",    "hollow square odd harmonics vs warm saw even harmonics balance",   Colours::kOsc2 },
        { Ids::osc2_tilt,         "Osc 2 Tilt",    "spectral tilt: dark low-end warmth vs bright treble presence",     Colours::kOsc2 },
        { Ids::osc2_formant,      "Osc 2 Vowel",   "vocal formant resonance peak sweep across upper harmonics",        Colours::kOsc2 },
        { Ids::osc2_mask,         "Osc 2 Mask",    "bitwise phase-masking aperture creating gritty digital crunch",    Colours::kOsc2 },
        { Ids::osc2_speed,        "Osc 2 Speed",   "wavetable sweep cycle speed across harmonic spectra (0-10 Hz)",   Colours::kOsc2 },
        { Ids::osc2_octave,       "Osc 2 Oct",     "coarse pitch transposition: -2, -1, unison, or +1 octave",         Colours::kOsc2 },
        { Ids::osc2_speed_lock,   "Speed Lock",    "syncs wavetable sweep speed to host 16th-note tempo grid",         Colours::kChaos },
        { Ids::osc2_loop,         "Sweep Loop",    "wavetable sweep direction: saw up, triangle, or saw down",         Colours::kOsc2 },
        { Ids::osc2_bank,         "Seed Bank",     "harmonic seed bank: clean VA, vowel formants, metallic, or bell",  Colours::kOsc2 },
        { Ids::osc2_level,        "Osc 2 Level",   "oscillator output level injected into the console mixer",          Colours::kMixer },

        // Oscillator 3
        { Ids::osc3_morph,        "Osc 3 Morph",   "continuous wavetable morphing across harmonic seed spectra",       Colours::kOsc3 },
        { Ids::osc3_odd_even,     "Osc 3 Even",    "hollow square odd harmonics vs warm saw even harmonics balance",   Colours::kOsc3 },
        { Ids::osc3_tilt,         "Osc 3 Tilt",    "spectral tilt: dark low-end warmth vs bright treble presence",     Colours::kOsc3 },
        { Ids::osc3_formant,      "Osc 3 Vowel",   "vocal formant resonance peak sweep across upper harmonics",        Colours::kOsc3 },
        { Ids::osc3_mask,         "Osc 3 Mask",    "bitwise phase-masking aperture creating gritty digital crunch",    Colours::kOsc3 },
        { Ids::osc3_speed,        "Osc 3 Speed",   "wavetable sweep cycle speed across harmonic spectra (0-10 Hz)",   Colours::kOsc3 },
        { Ids::osc3_octave,       "Osc 3 Oct",     "coarse pitch transposition: -2, -1, unison, or +1 octave",         Colours::kOsc3 },
        { Ids::osc3_speed_lock,   "Speed Lock",    "syncs wavetable sweep speed to host 16th-note tempo grid",         Colours::kChaos },
        { Ids::osc3_loop,         "Sweep Loop",    "wavetable sweep direction: saw up, triangle, or saw down",         Colours::kOsc3 },
        { Ids::osc3_bank,         "Seed Bank",     "harmonic seed bank: clean VA, vowel formants, metallic, or bell",  Colours::kOsc3 },
        { Ids::osc3_level,        "Osc 3 Level",   "oscillator output level injected into the console mixer",          Colours::kMixer },

        // 3. Stem Mixer & Noise (Bay 2 Lower)
        { Ids::noise_level,       "Noise Level",   "analog white noise generator level for transient breath and grit", Colours::kMixer },
        { Ids::noise_tone,        "Noise Tone",    "noise color tilt: dark low-rumble red to bright hissing pink",     Colours::kMixer },
        { Ids::noise_hammer_snap, "Hammer Snap",   "percussive clavinet hammer strike transient impact click",         Colours::kMixer },
        { Ids::osc_spread,        "Pan Spread",    "stereo pan dispersion fanning the three oscillators across field", Colours::kMixer },
        { Ids::mixer_seethe,      "Console Sat",   "mixing console saturation and asymmetric harmonic warmth",         Colours::kMixer },

        // 4. Chaotic Attractor Modulation Matrix (Bay 4)
        { Ids::chaos_rate,        "Chaos Rate",    "chaotic attractor evolution speed from slow drift to audio rate",  Colours::kChaos },
        { Ids::chaos_drift,       "Chaos Drift",   "smoothness: stepped sample & hold vs fluid brownian motion",       Colours::kChaos },
        { Ids::chaos_poly_spread, "Poly Spread",   "decorrelates chaotic attractor paths independently across voices",  Colours::kChaos },
        { Ids::rand_spread,       "Chaos Pan",     "chaotic stereo width modulation for wandering soundstage",         Colours::kChaos },
        { Ids::rand_tune,         "Chaos Detune",  "chaotic pitch drift spreading Osc 2 & 3 in opposite directions",   Colours::kChaos },
        { Ids::rand_chorus_rate,  "Chaos Chorus",  "chaotic modulation of chorus rate for liquid rotary flutter",      Colours::kChaos },
        { Ids::rand_morph,        "Chaos Morph",   "chaotic wavetable position wandering across all 3 oscillators",    Colours::kChaos },
        { Ids::rand_odd_even,     "Chaos Even",    "chaotic fluctuation between hollow odd and warm even harmonics",   Colours::kChaos },
        { Ids::rand_tilt,         "Chaos Tilt",    "chaotic dark/bright spectral tilt animation across the spectrum",  Colours::kChaos },
        { Ids::rand_formant,      "Chaos Formant", "chaotic vocal formant filter sweeps across upper partials",       Colours::kChaos },
        { Ids::rand_mask,         "Chaos Mask",    "chaotic bitmask modulation introducing sporadic digital crunch",   Colours::kChaos },
        { Ids::rand_speed,        "Chaos Speed",   "chaotic speed variations across the wavetable sweep cycle",        Colours::kChaos },

        // 5. Amplifier, Envelopes & MPE Expression (Bay 5)
        { Ids::amp_attack,        "Attack",        "amplifier envelope attack time from instant click to swell",       Colours::kVca },
        { Ids::amp_decay,         "Decay",         "amplifier envelope decay time falling to sustain plateau",         Colours::kVca },
        { Ids::amp_sustain,       "Sustain",       "amplifier envelope steady-state level held during note sustain",   Colours::kVca },
        { Ids::amp_release,       "Release",       "amplifier envelope release fade time after key release",           Colours::kVca },
        { Ids::amp_velocity,      "Velocity",      "dynamic velocity sensitivity scaling overall amplifier volume",    Colours::kVca },
        { Ids::keytrack_rate,     "Key Track",     "scales envelope speed with pitch: higher notes decay faster",      Colours::kVca },
        { Ids::glide_time,        "Glide",         "portamento pitch glide transition time between notes",             Colours::kVca },
        { Ids::mpe_slide_depth,   "MPE Slide",     "MPE Y-axis vertical finger slide modulation sensitivity",          Colours::kVca },
        { Ids::mpe_pressure_depth,"MPE Press",     "MPE Z-axis continuous finger aftertouch pressure sensitivity",     Colours::kVca },
        { Ids::pitch_bend_range,  "Bend Range",    "pitch wheel maximum bend depth in semitones (1 to 24 st)",         Colours::kVca },
        { Ids::voice_mode,        "Voice Mode",    "polyphonic allocation: 16-voice poly, mono, or legato glide",      Colours::kVca },

        // 6. Master Output & Stereo Space (Bay 6)
        { Ids::bass_boost,        "Bass Boost",    "solid-state sub-bass EQ boost centered at 40Hz for low-end thump",Colours::kMaster },
        { Ids::treble_tame,       "Treble Tame",   "gentle high-frequency shelving filter to smooth harsh overtones",  Colours::kMaster },
        { Ids::alpha_chorus,      "Chorus",        "vintage BBD analog chorus depth with wide quadrature stereo spread",Colours::kMaster },
        { Ids::chorus_rate,       "Chorus Rate",   "chorus LFO modulation rate from slow swirl to rapid vibrato",      Colours::kMaster },
        { Ids::osc_slop,          "Voice Drift",   "analog oscillator pitch and phase drift between voices",           Colours::kMaster },
        { Ids::global_width,      "Width",         "frequency-dependent width: mono sub with wide upper harmonics",    Colours::kMaster },
        { Ids::output_level,      "Volume",        "final master output level driving the DAW channel strip",          Colours::kMaster },
        { Ids::life_mode,         "Life Mode",     "triad sonic canon character: synthetic, cyborg, or organic",      Colours::kSynthetic }
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



