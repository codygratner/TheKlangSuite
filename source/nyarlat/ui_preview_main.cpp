#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "NyarlatProcessor.h"
#include "NyarlatEditor.h"
#include <iostream>
#include <vector>

int main(int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI guiInit;

    juce::File outputFile;
    for (int i = 1; i < argc; ++i) {
        juce::String arg(argv[i]);
        if (!arg.startsWith("--")) {
            if (juce::File::isAbsolutePath(arg)) {
                outputFile = juce::File(arg);
            } else {
                outputFile = juce::File::getCurrentWorkingDirectory().getChildFile(arg);
            }
        }
    }

    if (outputFile == juce::File()) {
        outputFile = juce::File("C:/Users/codyg/.gemini/antigravity/brain/8e191a0d-9f3e-4128-b565-5bb54f0edf9e/nyarlat_ui_proof.png");
    }

    std::cout << "[Nyarlat_Preview] Step 1: Initializing NyarlatProcessor..." << std::endl;
    std::unique_ptr<Nyarlat::NyarlatProcessor> processor;
    try {
        processor = std::make_unique<Nyarlat::NyarlatProcessor>();
    } catch (const std::exception& e) {
        std::cerr << "[Nyarlat_Preview] EXCEPTION in NyarlatProcessor: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "[Nyarlat_Preview] UNKNOWN EXCEPTION in NyarlatProcessor!" << std::endl;
        return 1;
    }
    std::cout << "[Nyarlat_Preview] Step 1 Complete: Processor allocated." << std::endl;

    std::cout << "[Nyarlat_Preview] Step 2: Creating NyarlatEditor..." << std::endl;
    std::unique_ptr<juce::AudioProcessorEditor> editor;
    try {
        editor.reset(processor->createEditorIfNeeded());
    } catch (const std::exception& e) {
        std::cerr << "[Nyarlat_Preview] EXCEPTION in createEditor: " << e.what() << std::endl;
        return 2;
    } catch (...) {
        std::cerr << "[Nyarlat_Preview] UNKNOWN EXCEPTION in createEditor!" << std::endl;
        return 2;
    }

    if (!editor) {
        std::cerr << "[Nyarlat_Preview] ERROR: createEditorIfNeeded() returned null!" << std::endl;
        return 3;
    }
    std::cout << "[Nyarlat_Preview] Step 2 Complete: Editor created." << std::endl;

    constexpr int kWidth = Nyarlat::NyarlatEditor::kBaseWidth;
    constexpr int kHeight = Nyarlat::NyarlatEditor::kBaseHeight;
    editor->setSize(kWidth, kHeight);
    editor->setVisible(true);

    std::cout << "[Nyarlat_Preview] Step 3: Editor visible at " << kWidth << "x" << kHeight << std::endl;

    juce::Array<juce::Component*> controls;
    for (auto* c : editor->getChildren()) {
        if (c->getNumChildComponents() > 0) {
            for (auto* sub : c->getChildren()) {
                controls.add(sub);
            }
        } else {
            controls.add(c);
        }
    }
    std::cout << "[Nyarlat_Preview] Scanned " << controls.size() << " active controls." << std::endl;

    int collisionCount = 0;
    for (int i = 0; i < controls.size(); ++i) {
        auto* c1 = controls[i];
        if (!c1->isVisible()) continue;
        auto b1 = c1->getBounds();

        for (int j = i + 1; j < controls.size(); ++j) {
            auto* c2 = controls[j];
            if (!c2->isVisible()) continue;
            auto b2 = c2->getBounds();

            if (b1.intersects(b2)) {
                auto intersection = b1.getIntersection(b2);
                if (intersection.getWidth() > 0 && intersection.getHeight() > 0) {
                    auto n1 = c1->getName().isNotEmpty() ? c1->getName() : juce::String("Item_") + juce::String(i);
                    auto n2 = c2->getName().isNotEmpty() ? c2->getName() : juce::String("Item_") + juce::String(j);
                    std::cout << "[RADAR] Overlap: " << n1 << " " << b1.toString()
                              << " overlaps " << n2 << " " << b2.toString()
                              << " (" << intersection.getWidth() << "x" << intersection.getHeight() << ")"
                              << std::endl;
                    collisionCount++;
                }
            }
        }
    }
    std::cout << "[Nyarlat_Preview] Collisions detected: " << collisionCount << std::endl;

    std::cout << "[Nyarlat_Preview] Step 4: Rendering offscreen via createComponentSnapshot..." << std::endl;
    juce::Image image = editor->createComponentSnapshot(editor->getLocalBounds(), true, 1.0f);
    std::cout << "[Nyarlat_Preview] Snapshot size: " << image.getWidth() << "x" << image.getHeight() << std::endl;

    outputFile.getParentDirectory().createDirectory();
    outputFile.deleteFile();

    juce::PNGImageFormat png;
    if (auto stream = outputFile.createOutputStream()) {
        if (png.writeImageToStream(image, *stream)) {
            std::cout << "[Nyarlat_Preview] SUCCESS: Rendered proof saved to " 
                      << outputFile.getFullPathName() << std::endl;
        } else {
            std::cerr << "[Nyarlat_Preview] ERROR: Failed to encode PNG" << std::endl;
            return 4;
        }
    } else {
        std::cerr << "[Nyarlat_Preview] ERROR: Failed to open output stream: " 
                  << outputFile.getFullPathName() << std::endl;
        return 5;
    }

    std::cout << "[Nyarlat_Preview] Complete." << std::endl;
    return 0;
}
