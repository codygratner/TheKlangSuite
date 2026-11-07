#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "ShoggothProcessor.h"
#include "ShoggothEditor.h"
#include <iostream>

int main(int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI guiInit;

    juce::File outputFile;
    for (int i = 1; i < argc; ++i) {
        juce::String arg(argv[i]);
        if (arg == "--dump-telemetry") {
            ++i; // skip next argument (telemetry path)
            continue;
        }
        if (!arg.startsWith("--")) {
            if (juce::File::isAbsolutePath(arg)) {
                outputFile = juce::File(arg);
            } else {
                outputFile = juce::File::getCurrentWorkingDirectory().getChildFile(arg);
            }
        }
    }

    std::cout << "[Shoggoth_Preview] Initializing headless processor..." << std::endl;
    Shoggoth::ShoggothProcessor processor;
    processor.prepareToPlay(48000.0, 512);

    std::cout << "[Shoggoth_Preview] Creating editor..." << std::endl;
    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());

    if (!editor) {
        std::cerr << "[Shoggoth_Preview] FAILED to create editor!" << std::endl;
        return 1;
    }

    constexpr int kWidth = 1160;
    constexpr int kHeight = 680;
    editor->setSize(kWidth, kHeight);
    editor->setVisible(true);

    std::cout << "[Shoggoth_Preview] Editor size: " << editor->getWidth() << "x" << editor->getHeight() << std::endl;

    auto& pm = processor.getPresetManager();
    auto presets = pm.getAvailablePresets();
    std::cout << "[Shoggoth_Preview] Presets discovered: " << presets.size() << std::endl;
    if (!presets.isEmpty()) {
        pm.loadPresetByIndex(0, processor.getAPVTS());
        std::cout << "[Shoggoth_Preview] Loaded initial preset: " << pm.getActivePresetName() 
                  << " from: " << presets[0].file.getFullPathName() << std::endl;
        std::cout << "[Shoggoth_Preview] filter_cutoff parameter value: " 
                  << processor.getAPVTS().getRawParameterValue("filter_cutoff")->load() << std::endl;
    }

    if (outputFile != juce::File()) {
        std::cout << "[Shoggoth_Preview] Rendering offscreen snapshot to: " << outputFile.getFullPathName() << std::endl;
        juce::Image image = editor->createComponentSnapshot(editor->getLocalBounds(), true, 1.0f);
        if (!image.isNull() && image.getWidth() > 0 && image.getHeight() > 0) {
            outputFile.getParentDirectory().createDirectory();
            outputFile.deleteFile();
            juce::PNGImageFormat png;
            if (auto stream = outputFile.createOutputStream()) {
                if (png.writeImageToStream(image, *stream)) {
                    std::cout << "[Shoggoth_Preview] SUCCESS: Rendered proof saved." << std::endl;
                } else {
                    std::cerr << "[Shoggoth_Preview] ERROR: Failed to encode PNG" << std::endl;
                    return 2;
                }
            } else {
                std::cerr << "[Shoggoth_Preview] ERROR: Failed to create output stream" << std::endl;
                return 3;
            }
        }
    }

    // ==============================================================================
    // End-to-End Audio DSP Verification Gauntlet
    // ==============================================================================
    std::cout << "[Shoggoth_Preview] Running end-to-end audio DSP verification..." << std::endl;
    juce::AudioBuffer<float> testBuffer(2, 512);
    juce::MidiBuffer midiMessages;

    // Send Note 60 On (vel 0.8)
    midiMessages.addEvent(juce::MidiMessage::noteOn(1, 60, 0.8f), 0);

    float peakAudio = 0.0f;
    for (int b = 0; b < 20; ++b) {
        testBuffer.clear();
        processor.processBlock(testBuffer, midiMessages);
        midiMessages.clear();

        for (int ch = 0; ch < 2; ++ch) {
            const float* p = testBuffer.getReadPointer(ch);
            for (int i = 0; i < 512; ++i) {
                peakAudio = std::max(peakAudio, std::abs(p[i]));
            }
        }
    }

    std::cout << "[Shoggoth_Preview] Note 60 Peak Sustain Audio: " << peakAudio
              << " (" << (20.0f * std::log10(std::max(1e-5f, peakAudio))) << " dBFS)" << std::endl;

    if (peakAudio < 0.05f) {
        std::cerr << "[Shoggoth_Preview] FATAL: Audio output is SILENT or severely attenuated! Peak = " << peakAudio << std::endl;
        return 4;
    }
    if (peakAudio > 2.0f) {
        std::cerr << "[Shoggoth_Preview] FATAL: Audio output is severely clipping/exploding! Peak = " << peakAudio << std::endl;
        return 5;
    }

    // Send Note 60 Off
    midiMessages.addEvent(juce::MidiMessage::noteOff(1, 60, 0.5f), 0);
    float peakRelease = 0.0f;
    for (int b = 0; b < 40; ++b) {
        testBuffer.clear();
        processor.processBlock(testBuffer, midiMessages);
        midiMessages.clear();

        if (b < 5) {
            std::cout << "[DBG testBuffer] b=" << b << " s[0]=" << testBuffer.getSample(0, 0) 
                      << " s[10]=" << testBuffer.getSample(0, 10) << std::endl;
        }
        for (int ch = 0; ch < 2; ++ch) {
            const float* p = testBuffer.getReadPointer(ch);
            for (int i = 0; i < 512; ++i) {
                peakRelease = std::max(peakRelease, std::abs(p[i]));
            }
        }
    }
    std::cout << "[Shoggoth_Preview] Release Tail Audio Peak: " << peakRelease << std::endl;

    std::cout << "[Shoggoth_Preview] Verification SUCCESS." << std::endl;
    return 0;
}
