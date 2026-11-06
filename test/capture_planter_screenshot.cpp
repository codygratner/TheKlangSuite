#include <iostream>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include "PlanterProcessor.h"
#include "PlanterEditor.h"

#include "support/HeadlessX11.h"

int main(int argc, char* argv[]) {
    HeadlessX11::install();
    juce::ScopedJuceInitialiser_GUI guiInit;

    TheKlangPlanterAudioProcessor processor;
    TheKlangPlanterAudioProcessorEditor editor(processor);
    editor.setSize(1040, 680);

    auto saveImage = [](const juce::Image& img, const juce::String& filename) {
        juce::File file("C:/Dev/TheKlangFarmer/screenshots/" + filename);
        file.getParentDirectory().createDirectory();
        file.deleteFile();
        juce::FileOutputStream stream(file);
        if (stream.openedOk()) {
            juce::PNGImageFormat png;
            png.writeImageToStream(img, stream);
            std::cout << "SUCCESS: Snapshot rendered to " << file.getFullPathName().toStdString() << std::endl;
        }
    };

    // Trigger audio block so engine has active envelope and visual activity
    processor.getEngine().trigger(1.0f);
    juce::AudioBuffer<float> testBuf(2, 64);
    testBuf.clear();
    juce::MidiBuffer midi;
    processor.processBlock(testBuf, midi);

    editor.timerCallback();
    auto img = editor.createComponentSnapshot(editor.getLocalBounds());
    saveImage(img, "TheKlangPlanter_GUI.png");

    return 0;
}
