#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "FormlessProcessor.h"
#include "FormlessEditor.h"
#include <iostream>

static void dumpTelemetryJson(const juce::File& file)
{
    auto* rootObj = new juce::DynamicObject();
    juce::var root(rootObj);
    file.getParentDirectory().createDirectory();
    file.replaceWithText(juce::JSON::toString(root, true));
    std::cout << "[FormlessSpawn_Preview] Dumped C++ telemetry to " << file.getFullPathName() << std::endl;
}

int main(int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI guiInit;

    juce::File outputFile;
    juce::File telemetryFile;

    for (int i = 1; i < argc; ++i) {
        juce::String arg(argv[i]);
        if (arg == "--dump-telemetry" && i + 1 < argc) {
            telemetryFile = juce::File(argv[++i]);
        } else if (!arg.startsWith("--")) {
            if (juce::File::isAbsolutePath(arg)) {
                outputFile = juce::File(arg);
            } else {
                outputFile = juce::File::getCurrentWorkingDirectory().getChildFile(arg);
            }
        }
    }

    if (telemetryFile != juce::File()) {
        dumpTelemetryJson(telemetryFile);
    }

    if (outputFile == juce::File()) {
        outputFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                        .getParentDirectory().getChildFile("formless_ui_proof.png");
    }

    std::cout << "[FormlessSpawn_Preview] Initializing FormlessSpawnAudioProcessor..." << std::endl;
    FormlessSpawnAudioProcessor processor;

    std::cout << "[FormlessSpawn_Preview] Creating FormlessSpawnAudioProcessorEditor..." << std::endl;
    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditorIfNeeded());
    if (!editor) {
        std::cerr << "[FormlessSpawn_Preview] ERROR: createEditorIfNeeded() returned null!" << std::endl;
        return 1;
    }

    constexpr int kWidth = 900;
    constexpr int kHeight = 560;
    editor->setSize(kWidth, kHeight);
    editor->setVisible(true);

    // Render snapshot via createComponentSnapshot
    juce::Image image = editor->createComponentSnapshot(editor->getLocalBounds(), true, 1.0f);
    if (image.isNull() || image.getWidth() == 0 || image.getHeight() == 0) {
        std::cerr << "[FormlessSpawn_Preview] Snapshot null!" << std::endl;
        return 1;
    }

    outputFile.getParentDirectory().createDirectory();
    outputFile.deleteFile();

    juce::PNGImageFormat png;
    if (auto stream = outputFile.createOutputStream()) {
        if (png.writeImageToStream(image, *stream)) {
            std::cout << "[FormlessSpawn_Preview] Saved offscreen proof to " << outputFile.getFullPathName() << std::endl;
            return 0;
        }
    }

    std::cerr << "[FormlessSpawn_Preview] Failed to write image stream!" << std::endl;
    return 1;
}
