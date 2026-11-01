#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "HasturProcessor.h"
#include "HasturEditor.h"
#include "WaveformPreviewEngine.h"

static void dumpTelemetryJson(const juce::File& file)
{
    using namespace CrackedChimes::DSP;
    auto* rootObj = new juce::DynamicObject();
    juce::var root(rootObj);

    // 1. DCO Waves [family 0..3][morph_idx 0..4]
    auto* dcoObj = new juce::DynamicObject();
    const float morphSteps[5] = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };
    std::array<float, WaveformPreviewEngine::kCoreResolution> coreBuf;

    for (int fam = 0; fam < 4; ++fam) {
        juce::Array<juce::var> famList;
        for (float m : morphSteps) {
            WaveformPreviewEngine::generateCorePreview(coreBuf, fam, m, 0, 1.0f, 0.0f, 0.0f, 0.0f);
            juce::Array<juce::var> wave256;
            for (int i = 0; i < 256; ++i) {
                wave256.add(std::round(coreBuf[i * 4] * 10000.0f) / 10000.0f);
            }
            famList.add(wave256);
        }
        dcoObj->setProperty(juce::String(fam), famList);
    }
    rootObj->setProperty("dco_waves", juce::var(dcoObj));

    // 2. Sub Waves (-2, -1, 0, +1)
    auto* subObj = new juce::DynamicObject();
    const int octs[4] = { -2, -1, 0, 1 };
    std::array<float, WaveformPreviewEngine::kSubResolution> subBuf;

    for (int oct : octs) {
        WaveformPreviewEngine::generateSubPreview(subBuf, oct, 0, 0, 0.0f, 0.0f);
        juce::Array<juce::var> wave256;
        for (int i = 0; i < 256; ++i) {
            wave256.add(std::round(subBuf[i * 2] * 10000.0f) / 10000.0f);
        }
        subObj->setProperty(juce::String(oct), wave256);
    }
    rootObj->setProperty("sub_waves", juce::var(subObj));

    file.getParentDirectory().createDirectory();
    file.replaceWithText(juce::JSON::toString(root, true));
    std::cout << "[Hastur_Preview] Dumped C++ telemetry to " << file.getFullPathName() << std::endl;
}
#include <iostream>
#include <vector>

int main(int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI guiInit;

    juce::File outputFile;
    juce::File telemetryFile;

    int forceLifeMode = -1;
    for (int i = 1; i < argc; ++i) {
        juce::String arg(argv[i]);
        if (arg == "--dump-telemetry" && i + 1 < argc) {
            telemetryFile = juce::File(argv[++i]);
        } else if (arg == "--life-mode" && i + 1 < argc) {
            forceLifeMode = juce::String(argv[++i]).getIntValue();
        } else if (!arg.startsWith("--")) {
            if (juce::File::isAbsolutePath(arg)) {
                outputFile = juce::File(arg);
            } else {
                outputFile = juce::File::getCurrentWorkingDirectory().getChildFile(arg);
            }
        }
    }

    if (!outputFile.exists() && outputFile == juce::File()) {
        outputFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                        .getParentDirectory().getChildFile("hastur_ui_proof.png");
    }

    if (telemetryFile != juce::File()) {
        dumpTelemetryJson(telemetryFile);
    }

    std::cout << "[Hastur_Preview] Initializing HasturAudioProcessor..." << std::endl;
    HasturAudioProcessor processor;

    if (forceLifeMode >= 0) {
        if (auto* p = processor.getAPVTS().getParameter("life_mode")) {
            p->setValueNotifyingHost(p->convertTo0to1(static_cast<float>(forceLifeMode)));
        }
    }

    std::cout << "[Hastur_Preview] Creating HasturAudioProcessorEditor..." << std::endl;
    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditorIfNeeded());
    if (!editor) {
        std::cerr << "[Hastur_Preview] ERROR: createEditorIfNeeded() returned null!" << std::endl;
        return 1;
    }

    constexpr int kWidth = 1160;
    constexpr int kHeight = 870;
    editor->setSize(kWidth, kHeight);
    editor->setVisible(true);

    std::cout << "[Hastur_Preview] Running Spatial Radar & Component Introspection..." << std::endl;

    // Find the canvas container (contentContainer is typically child 0 of editor)
    juce::Component* canvas = editor.get();
    for (auto* child : editor->getChildren()) {
        if (child->getWidth() == kWidth && child->getHeight() == kHeight) {
            canvas = child;
            break;
        }
    }

    std::cout << "[Hastur_Preview] Editor visible: " << (editor->isVisible() ? "true" : "false")
              << " bounds: " << editor->getBounds().toString() << std::endl;
    std::cout << "[Hastur_Preview] Canvas visible: " << (canvas->isVisible() ? "true" : "false")
              << " bounds: " << canvas->getBounds().toString() << std::endl;

    auto& controls = canvas->getChildren();
    std::cout << "[Hastur_Preview] Scanned " << controls.size() << " children on canvas." << std::endl;

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
                    
                    std::cout << "[RADAR] Collision: " << n1 << " " << b1.toString()
                              << " overlaps " << n2 << " " << b2.toString()
                              << " (area: " << intersection.getWidth() << "x" << intersection.getHeight() << ")" 
                              << std::endl;
                    collisionCount++;
                }
            }
        }
    }

    std::cout << "[Hastur_Preview] Total collisions detected: " << collisionCount << std::endl;

    std::cout << "[Hastur_Preview] Rendering offscreen via createComponentSnapshot..." << std::endl;
    juce::Image image = canvas->createComponentSnapshot(canvas->getLocalBounds(), true, 1.0f);
    if (image.isNull() || image.getWidth() == 0 || image.getHeight() == 0) {
        std::cerr << "[Hastur_Preview] Canvas snapshot null, trying editor snapshot..." << std::endl;
        image = editor->createComponentSnapshot(editor->getLocalBounds(), true, 1.0f);
    }

    std::cout << "[Hastur_Preview] Snapshot size: " << image.getWidth() << "x" << image.getHeight() << std::endl;

    outputFile.getParentDirectory().createDirectory();
    outputFile.deleteFile();

    juce::PNGImageFormat png;
    if (auto stream = outputFile.createOutputStream()) {
        if (png.writeImageToStream(image, *stream)) {
            std::cout << "[Hastur_Preview] SUCCESS: Rendered proof saved to " 
                      << outputFile.getFullPathName() << std::endl;
        } else {
            std::cerr << "[Hastur_Preview] ERROR: Failed to encode PNG" << std::endl;
            return 2;
        }
    } else {
        std::cerr << "[Hastur_Preview] ERROR: Failed to open output stream: " 
                  << outputFile.getFullPathName() << std::endl;
        return 3;
    }

    std::cout << "[Hastur_Preview] Offscreen snapshot complete." << std::endl;
    return 0;
}
