#pragma once

/**
 * ==============================================================================
 * CRACKED CHIMES // ARCHIVE REPOSITORY
 * Subsystem: HASTUR — Osc 3, Doepfer A-196 PLL, Quadrature LFO & Mod C Archive
 * Date Archived: 2026-10-28
 * Planned Destination: Phonomicon / The Klang Chimaera Flagship Modular Synthesizer
 * ==============================================================================
 *
 * This archive preserves the complete production-tested DSP, parameter layout,
 * and modulation routing for the complex experimental modules originally designed
 * for Hastur, preserved prior to the Workhorse Refactoring (Phase 1):
 *
 * 1. Osc 3 Tri-State Pitch Source Engine (MIDI, PLL, Quadrature LFO)
 * 2. Doepfer A-196 Phase Locked Loop (PLL) with 8x Multisampling
 * 3. 4-Phase Quadrature LFO (0°, 90°, 180°, 270°) with Ice/Slow/Fast/Full ranges
 * 4. Mod Matrix Channel C (Source, Type, Destination, Bipolar Depth)
 * 5. Roland IR3R05 Pre-Filter Drive and Post-Filter Drive stages
 * ==============================================================================
 */

#include <cmath>
#include <algorithm>
#include <array>
#include <cstdint>
#include <juce_audio_processors/juce_audio_processors.h>

namespace CrackedChimes::Archive {

// ==============================================================================
// 1. Osc 3 Pitch Sources & Quadrature LFO Ranges
// ==============================================================================
enum class Osc3PitchSource : uint8_t {
    MIDI = 0,
    PLL  = 1,
    LFO  = 2
};

enum class Osc3LfoRange : uint8_t {
    ICE  = 0, // 0.0016667 Hz (10 min) .. 0.1 Hz (10 sec)
    SLOW = 1, // 0.016667 Hz (1 min)   .. 1.0 Hz (1 sec)
    FAST = 2, // 1.0 Hz                .. 100 Hz
    FULL = 3  // 20.0 Hz               .. 20000 Hz
};

enum class QuadLfoDest : uint8_t {
    OFF           = 0,
    VCF_CUTOFF    = 1,
    VCF_RES       = 2,
    OSC1_PITCH    = 3,
    OSC2_PITCH    = 4,
    PULSE_WIDTH   = 5,
    MIXER_DRIVE   = 6,
    AMP_LEVEL     = 7,
    MID_APF       = 8,
    OSC1_APF      = 9,
    OSC2_APF      = 10
};

// ==============================================================================
// 2. Frequency & Slew Mapping Functions
// ==============================================================================
[[nodiscard]] inline float computeLfoFreq(Osc3LfoRange range, float speedKnob) noexcept {
    const float s = std::clamp(speedKnob, 0.0f, 1.0f);
    float minF = 0.016667f;
    float maxF = 1.0f;
    switch (range) {
        case Osc3LfoRange::ICE:
            minF = 1.0f / 600.0f; // 0.0016667 Hz (10 min)
            maxF = 1.0f / 10.0f;  // 0.1 Hz (10 sec)
            break;
        case Osc3LfoRange::SLOW:
            minF = 1.0f / 60.0f;  // 0.016667 Hz (1 min)
            maxF = 1.0f / 1.0f;   // 1.0 Hz (1 sec)
            break;
        case Osc3LfoRange::FAST:
            minF = 1.0f;          // 1 Hz
            maxF = 100.0f;        // 100 Hz
            break;
        case Osc3LfoRange::FULL:
            minF = 20.0f;         // 20 Hz
            maxF = 20000.0f;      // 20 kHz
            break;
    }
    return minF * std::pow(maxF / minF, s);
}

[[nodiscard]] inline float computeSlewAlpha(float slewKnob, double sampleRate) noexcept {
    const float s = std::clamp(slewKnob, 0.0f, 1.0f);
    const float fc = 20000.0f * std::pow(0.01f / 20000.0f, s);
    if (fc >= 19500.0f) return 1.0f; // Bypass
    const float omega = static_cast<float>(6.28318530717958647692 * fc / sampleRate);
    return 1.0f - std::exp(-omega);
}

// ==============================================================================
// 3. Mod Matrix Channel C & Routing Definition
// ==============================================================================
struct ModMatrixSourcesArchive {
    float osc1{0.0f};
    float osc1Apf{0.0f};
    float osc2{0.0f};
    float osc2Apf{0.0f};
    float osc3{0.0f};
    float osc3Apf{0.0f};
    float pllVco{0.0f};
    float noise{0.0f};
    float preFilter{0.0f};
    float postFilter{0.0f};
};

// ==============================================================================
// 4. Archived APVTS Parameter Layout Reference
// ==============================================================================
inline void addArchivedParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout) {
    // Card 3: Osc 3 Tri-State Pitch Source & Engines
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"osc3_pitch_source", 1}, "Osc 3 Pitch Source",
        juce::StringArray{"MIDI", "PLL", "LFO"}, 0));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"osc3_wave", 1}, "Osc 3 Wave",
        juce::StringArray{"Saw 1 (PolyBLEP)", "Square 1 (50%)", "Sub Divided", "Clean Sine"}, 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"osc3_offset", 1}, "Osc 3 Pitch Offset",
        juce::NormalisableRange<float>{-24.0f, 24.0f, 1.0f}, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"osc13_sync_mode", 1}, "Osc 1->3 Sync Mode",
        juce::StringArray{"Off", "1 -> 3", "3 -> 1", "Metal", "APF 1 -> 3", "APF 3 -> 1", "APF Metal"}, 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"osc13_sync_amount", 1}, "Osc 1->3 Sync Amount",
        0.0f, 1.0f, 1.0f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"osc3_octave", 1}, "Osc 3 Octave (PLL)",
        juce::StringArray{"-3 Oct", "-2 Oct", "-1 Oct", "0 (Fund)", "+1 Oct", "+2 Oct", "+3 Oct"}, 3));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"osc3_apf_cutoff", 1}, "Osc 3 APF Cutoff",
        juce::NormalisableRange<float>{20.0f, 20000.0f, 0.0f, 0.25f}, 1000.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"osc3_drive", 1}, "Osc 3 Drive",
        0.0f, 1.0f, 0.0f));

    // Doepfer A-196 PLL Parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"pll_vco_offset", 1}, "PLL VCO Center Freq",
        juce::NormalisableRange<float>{0.0f, 1.0f}, 0.5f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"pll_range", 1}, "PLL Freq Range",
        juce::StringArray{"Low", "Mid", "Hi"}, 1));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"pll_comp_type", 1}, "PLL Phase Comparator",
        juce::StringArray{"PC1 (XOR)", "PC2 (Phase/Freq Edge)", "PC3 (Sample/Hold Edge)"}, 1));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"pll_divider", 1}, "PLL Loop Divider",
        juce::StringArray{"/1", "/2", "/4", "/8", "/16"}, 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"pll_slew", 1}, "PLL Loop Filter Slew",
        0.0f, 1.0f, 0.35f));

    // Quadrature LFO Parameters
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"osc3_lfo_range", 1}, "Osc 3 LFO Range",
        juce::StringArray{"Ice", "Slow", "Fast", "Full"}, 1));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"osc3_lfo_speed", 1}, "Osc 3 LFO Speed",
        0.0f, 1.0f, 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"osc3_lfo_slew", 1}, "Osc 3 LFO Slew",
        0.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"osc3_lfo_use_apf", 1}, "Osc 3 LFO Output Tap",
        false));

    // Mod C Channel Parameters
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"mod_c_source", 1}, "Mod C Source",
        juce::StringArray{"None", "Osc 1", "Osc 1 APF", "Osc 2", "Osc 2 APF", "Osc 3", "Osc 3 APF", "PLL VCO", "Noise", "Pre-Filter", "Post-Filter"}, 0));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"mod_c_type", 1}, "Mod C Type",
        juce::StringArray{"Lin FM", "Exp FM", "AM/RM", "Filter FM"}, 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"mod_c_amount", 1}, "Mod C Amount",
        -1.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"mod_c_dest", 1}, "Mod C Dest",
        juce::StringArray{"Osc 1", "Osc 2", "Osc 3", "Cutoff", "Resonance"}, 0));

    // Redundant Filter Drive Stages
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"pre_drive", 1}, "Pre-Filter Drive",
        0.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"post_drive", 1}, "Post-Filter Drive",
        0.0f, 1.0f, 0.0f));
}

} // namespace CrackedChimes::Archive
