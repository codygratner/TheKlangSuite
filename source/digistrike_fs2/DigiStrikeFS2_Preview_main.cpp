#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "DigiStrikeFS2Processor.h"
#include "DigiStrikeFS2Editor.h"
#include <iostream>

static void dumpTelemetryJson(const juce::File& file)
{
    auto* rootObj = new juce::DynamicObject();
    juce::var root(rootObj);
    file.getParentDirectory().createDirectory();
    file.replaceWithText(juce::JSON::toString(root, true));
    std::cout << "[DigiStrikeFS2_Preview] Dumped telemetry to " << file.getFullPathName() << std::endl;
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
                        .getParentDirectory().getChildFile("digistrike_fs2_ui_proof.png");
    }

    std::cout << "[DigiStrikeFS2_Preview] Initializing DigiStrikeFS2AudioProcessor..." << std::endl;
    DigiStrikeFS2AudioProcessor processor;

    std::cout << "[DigiStrikeFS2_Preview] Creating DigiStrikeFS2AudioProcessorEditor..." << std::endl;
    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditorIfNeeded());
    if (!editor) {
        std::cerr << "[DigiStrikeFS2_Preview] ERROR: Failed to create editor!" << std::endl;
        return 1;
    }

    editor->setSize(DigiStrikeFS2AudioProcessorEditor::kBaseWidth, DigiStrikeFS2AudioProcessorEditor::kBaseHeight);
    editor->setVisible(true);

    std::cout << "[DigiStrikeFS2_Preview] Rendering offscreen snapshot (" 
              << editor->getWidth() << "x" << editor->getHeight() << ")..." << std::endl;
    
    juce::Image image = editor->createComponentSnapshot(editor->getLocalBounds(), true, 1.0f);
    if (image.isNull() || image.getWidth() == 0 || image.getHeight() == 0) {
        std::cerr << "[DigiStrikeFS2_Preview] ERROR: Snapshot is null!" << std::endl;
        return 1;
    }

    outputFile.getParentDirectory().createDirectory();
    outputFile.deleteFile();

    juce::PNGImageFormat png;
    if (auto stream = outputFile.createOutputStream()) {
        if (png.writeImageToStream(image, *stream)) {
            std::cout << "[DigiStrikeFS2_Preview] Successfully saved bitmap proof to: "
                      << outputFile.getFullPathName() << std::endl;
            return 0;
        }
    }

    std::cerr << "[DigiStrikeFS2_Preview] ERROR: Failed to write image stream to: "
              << outputFile.getFullPathName() << std::endl;
    return 1;
}
