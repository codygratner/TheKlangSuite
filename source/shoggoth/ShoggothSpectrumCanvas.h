#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <vector>

namespace Shoggoth {

class ShoggothProcessor;

class ShoggothSpectrumCanvas : public juce::Component {
public:
    explicit ShoggothSpectrumCanvas(ShoggothProcessor& proc);
    ~ShoggothSpectrumCanvas() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    void undo();
    void setSourceIndex(int srcIdx) { activeSource_ = srcIdx; repaint(); }

private:
    ShoggothProcessor& processor_;
    int activeSource_{0}; // 0 = Source 1, 1 = Source 2

    // Undo Ring Buffer (16 snapshots)
    struct SpectrumSnapshot {
        std::array<float, 63> s1;
        std::array<float, 63> s2;
    };
    std::vector<SpectrumSnapshot> undoBuffer_;
    size_t undoIndex_{0};

    void saveUndoSnapshot();
    void applyTemplate(const juce::String& templateName);
    void updateHarmonicFromMouse(int mouseX, int mouseY);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShoggothSpectrumCanvas)
};

} // namespace Shoggoth
