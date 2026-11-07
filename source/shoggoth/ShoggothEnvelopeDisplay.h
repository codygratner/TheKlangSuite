#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>

namespace Shoggoth {

class ShoggothProcessor;

class ShoggothEnvelopeDisplay : public juce::Component {
public:
    explicit ShoggothEnvelopeDisplay(ShoggothProcessor& proc) : processor_(proc) {}
    ~ShoggothEnvelopeDisplay() override = default;

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat().reduced(4.0f);

        // Chassis background
        g.setColour(juce::Colour(0xFF0D1117));
        g.fillRoundedRectangle(bounds, 6.0f);
        g.setColour(juce::Colour(0xFF30363D));
        g.drawRoundedRectangle(bounds, 6.0f, 1.0f);

        // Grid lines
        g.setColour(juce::Colour(0x1AFFFFFF));
        for (int i = 1; i < 4; ++i) {
            float y = bounds.getY() + bounds.getHeight() * (static_cast<float>(i) / 4.0f);
            g.drawHorizontalLine(static_cast<int>(y), bounds.getX(), bounds.getRight());
        }

        // Draw 6-Stage Envelope Curve
        juce::Path p;
        p.startNewSubPath(bounds.getX(), bounds.getBottom());

        const float stageW = bounds.getWidth() / 6.0f;
        // Mock curve
        p.lineTo(bounds.getX() + stageW * 0.5f, bounds.getY() + bounds.getHeight() * 0.05f); // Peak
        p.lineTo(bounds.getX() + stageW * 2.0f, bounds.getY() + bounds.getHeight() * 0.25f); // Decay 1
        p.lineTo(bounds.getX() + stageW * 3.5f, bounds.getY() + bounds.getHeight() * 0.40f); // Decay 2
        p.lineTo(bounds.getX() + stageW * 4.5f, bounds.getY() + bounds.getHeight() * 0.45f); // Sustain
        p.lineTo(bounds.getRight(), bounds.getBottom()); // Release

        g.setColour(juce::Colour(0x3300FFCC));
        juce::Path strokePath = p;
        strokePath.lineTo(bounds.getRight(), bounds.getBottom());
        strokePath.lineTo(bounds.getX(), bounds.getBottom());
        strokePath.closeSubPath();
        g.fillPath(strokePath);

        g.setColour(juce::Colour(0xFF00FFCC));
        g.strokePath(p, juce::PathStrokeType(2.0f));
    }

    void resized() override {}

private:
    ShoggothProcessor& processor_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShoggothEnvelopeDisplay)
};

} // namespace Shoggoth
