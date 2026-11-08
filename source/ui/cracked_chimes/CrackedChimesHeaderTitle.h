#pragma once
// ==============================================================================
// CrackedChimesHeaderTitle.h - Uniform Header Title Block for Cracked Chimes Fleet
// Fixed geometry: (14, 4, 140, 36)
// Mouse click opens the AboutOverlay dialog.
// ==============================================================================

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

namespace CrackedChimes {

class HeaderTitleBlock : public juce::Component {
public:
    HeaderTitleBlock(const juce::String& title, juce::Colour accentColour)
        : title_(title), accentColour_(accentColour)
    {
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
    }

    void setAccentColour(juce::Colour c) {
        accentColour_ = c;
        repaint();
    }

    void setTitle(const juce::String& title) {
        title_ = title;
        repaint();
    }

    void mouseEnter(const juce::MouseEvent&) override {
        isHovered_ = true;
        repaint();
    }

    void mouseExit(const juce::MouseEvent&) override {
        isHovered_ = false;
        repaint();
    }

    void mouseUp(const juce::MouseEvent& e) override {
        if (e.mods.isLeftButtonDown() || e.mouseWasClicked()) {
            if (onClick) onClick();
        }
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();

        // 1. Primary Title Line (y: 0 .. 22)
        auto titleRect = bounds.removeFromTop(22.0f);
        g.setFont(juce::Font(juce::FontOptions(19.0f).withStyle("Bold")));
        
        juce::Colour c = accentColour_;
        if (isHovered_) {
            c = c.brighter(0.2f);
        }
        g.setColour(c);
        g.drawText(title_, titleRect, juce::Justification::centredLeft, true);

        // 2. Subtitle Line (y: 22 .. 34)
        g.setFont(juce::Font(juce::FontOptions(9.0f).withStyle("Bold")));
        g.setColour(juce::Colour(0xFF8B949E));
        g.drawText("CRACKED CHIMES", bounds, juce::Justification::centredLeft, true);
    }

    std::function<void()> onClick;

private:
    juce::String title_;
    juce::Colour accentColour_;
    bool isHovered_{false};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderTitleBlock)
};

} // namespace CrackedChimes
