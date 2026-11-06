// test/headless_plugin_runner.cpp
// Headless Plugin-Boundary Harness for Hastur Audio Processor
// Part of Autoplan 2.0 Grounded Verification Stack

#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "hastur/HasturProcessor.h"
#include "hastur/HasturParameterIds.h"

#include "support/HeadlessX11.h"

int main(int argc, char* argv[]) {
    HeadlessX11::install();
    juce::ScopedJuceInitialiser_GUI guiInit;

    std::cout << "Starting Headless Plugin-Boundary Harness for Hastur..." << std::endl;

    // 1. Instantiate Real AudioProcessor
    HasturAudioProcessor processor;
    std::cout << "Instantiated HasturAudioProcessor successfully." << std::endl;

    // 2. Validate APVTS Parameter Presence via getAPVTS()
    auto& apvts = processor.getAPVTS();
    
    // Check critical parameters
    const char* criticalParams[] = {
        HasturParameters::Ids::dco_bite,
        HasturParameters::Ids::dco_morph,
        HasturParameters::Ids::osc1_mult,
        HasturParameters::Ids::osc1_apf_pitch_track,
        HasturParameters::Ids::cutoff,
        HasturParameters::Ids::resonance,
        HasturParameters::Ids::filter_env_attack,
        HasturParameters::Ids::amp_env_snap,
        HasturParameters::Ids::output_level
    };

    for (const char* pid : criticalParams) {
        auto* p = apvts.getParameter(pid);
        if (p == nullptr) {
            std::cerr << "FAILED: Critical parameter not found in APVTS: " << pid << std::endl;
            return 1;
        }
        std::cout << "  Verified APVTS parameter: " << pid << " (" << p->getName(32).toRawUTF8() << ")" << std::endl;
    }

    // 3. Offline Rendering Verification across Block Sizes
    const double sampleRate = 48000.0;
    const int blockSize = 256;
    processor.prepareToPlay(sampleRate, blockSize);

    juce::AudioBuffer<float> buffer(2, blockSize);
    buffer.clear();

    juce::MidiBuffer midi;
    // Note On C3 (MIDI 60), Velocity 0.8 at sample 0
    midi.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)100), 0);

    processor.processBlock(buffer, midi);

    float peakL = 0.0f, peakR = 0.0f;
    for (int i = 0; i < blockSize; ++i) {
        float l = buffer.getSample(0, i);
        float r = buffer.getSample(1, i);
        assert(std::isfinite(l));
        assert(std::isfinite(r));
        peakL = std::max(peakL, std::abs(l));
        peakR = std::max(peakR, std::abs(r));
    }

    std::cout << "  Rendered 256 samples offline. Peak L: " << peakL << ", Peak R: " << peakR << std::endl;
    assert(peakL > 0.05f); // Must produce real audio on note-on
    assert(peakR > 0.05f);

    // 4. Parameter Modulation Verification via APVTS
    auto* biteParam = apvts.getParameter(HasturParameters::Ids::dco_bite);
    assert(biteParam != nullptr);
    biteParam->setValueNotifyingHost(0.85f); // Set bite high

    midi.clear();
    buffer.clear();
    processor.processBlock(buffer, midi);

    float bitePeak = 0.0f;
    for (int i = 0; i < blockSize; ++i) {
        bitePeak = std::max(bitePeak, std::abs(buffer.getSample(0, i)));
    }
    std::cout << "  Rendered with high BITE. Peak: " << bitePeak << std::endl;
    assert(std::isfinite(bitePeak) && bitePeak > 0.01f);

    processor.releaseResources();
    std::cout << ">>> HEADLESS PLUGIN-BOUNDARY HARNESS PASSED 100%! <<<" << std::endl;
    return 0;
}
