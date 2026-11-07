#pragma once
// ==============================================================================
// CrackedChimesHeaderInspector.h
// Universal 2-Column Monolithic OLED Telemetry Ribbon
// Header-only, namespace CrackedChimes::UI
// ==============================================================================

#include <juce_gui_basics/juce_gui_basics.h>
#include <algorithm>
#include "CrackedChimesColours.h"

namespace CrackedChimes::UI {

class CrackedChimesHeaderInspector : public juce::Component, private juce::Timer {
public:
    struct Readout {
        juce::String name;
        juce::String blurb;
        juce::Colour accent{Colours::kFilterPrimary};
        bool isDefault{false};
    };

    explicit CrackedChimesHeaderInspector(juce::String defaultPluginName = "CRACKED CHIMES",
                                          juce::String defaultBlurb = "sovereign metamorphic instrument",
                                          juce::Colour defaultAccent = Colours::kFilterPrimary)
        : defaultName_(std::move(defaultPluginName)),
          defaultBlurb_(std::move(defaultBlurb)),
          defaultAccent_(defaultAccent)
    {
        setInterceptsMouseClicks(false, false);
        setReadoutDefault();
    }

    ~CrackedChimesHeaderInspector() override {
        stopTimer();
    }

    void setDefaults(juce::String pluginName, juce::String blurb, juce::Colour accent = Colours::kFilterPrimary) {
        defaultName_ = std::move(pluginName);
        defaultBlurb_ = std::move(blurb);
        defaultAccent_ = accent;
        if (readout_.isDefault) {
            setReadoutDefault();
        }
    }

    void setReadout(Readout r) {
        stopTimer();
        if (readout_.name == r.name && readout_.blurb == r.blurb &&
            readout_.accent == r.accent && readout_.isDefault == r.isDefault)
            return;

        readout_ = std::move(r);
        repaint();
    }

    void setReadoutDefault() {
        stopTimer();
        Readout d;
        d.name = defaultName_;
        d.blurb = defaultBlurb_;
        d.accent = defaultAccent_;
        d.isDefault = true;
        setReadout(std::move(d));
    }

    void show(const juce::String& title,
              const juce::String& blurb,
              juce::Colour accent = Colours::kOscCore)
    {
        Readout r;
        r.name = title;
        r.blurb = blurb;
        r.accent = accent;
        r.isDefault = false;
        setReadout(std::move(r));
    }

    // Deprecated compatibility overload (drops obsolete valueText)
    void show(const juce::String& title,
              const juce::String& /*valueText*/,
              const juce::String& blurb,
              juce::Colour accent = Colours::kOscCore)
    {
        show(title, blurb, accent);
    }

    void setNameColumnWidth(float w) noexcept {
        nameColW_ = w;
        repaint();
    }

    float getNameColumnWidth() const noexcept { return nameColW_; }

    void setIdle(const juce::String& subtitle = {}) {
        if (subtitle.isNotEmpty()) {
            defaultBlurb_ = subtitle;
        }
        startTimer(1500);
    }

    void setIdleImmediate(const juce::String& subtitle = {}) {
        if (subtitle.isNotEmpty()) {
            defaultBlurb_ = subtitle;
        }
        setReadoutDefault();
    }

    const Readout& getReadout() const noexcept { return readout_; }

    void paint(juce::Graphics& g) override {
        const auto bounds = getLocalBounds().toFloat();
        if (bounds.isEmpty()) return;

        // 1. Recessed OLED display cavity
        g.setColour(Colours::kCavity);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(Colours::kBadgeBorder);
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        // 2. Col 1: Parameter / System Name (x: 10, w: nameColW_, 13.0f Bold, zero truncation)
        const float nameW = nameColW_;
        const auto nameRect = juce::Rectangle<float>(bounds.getX() + 10.0f, bounds.getY(), nameW, bounds.getHeight());
        g.setFont(Typography::makeFont(13.0f, true));
        g.setColour(readout_.accent);
        g.drawText(readout_.name, nameRect, juce::Justification::centredLeft, true);

        // 3. Subtle vertical separator
        g.setColour(Colours::kBadgeBorder.withAlpha(0.65f));
        g.drawVerticalLine(juce::roundToInt(bounds.getX() + nameW + 13.0f), bounds.getY() + 5.0f, bounds.getBottom() - 5.0f);

        // 4. Col 2: 2-line Gear Manual Blurb across remaining width (12.5f)
        const float blurbX = bounds.getX() + nameW + 20.0f;
        const float blurbW = std::max(40.0f, bounds.getRight() - blurbX - 8.0f);
        const auto blurbRect = juce::Rectangle<float>(blurbX, bounds.getY(), blurbW, bounds.getHeight());
        g.setFont(Typography::makeFont(12.5f, false));
        g.setColour(readout_.isDefault ? juce::Colour(0xFF6E7681) : Colours::kText);
        g.drawFittedText(readout_.blurb, blurbRect.toNearestInt(), juce::Justification::centredLeft, 2);
    }

private:
    void timerCallback() override {
        stopTimer();
        setReadoutDefault();
    }

    juce::String defaultName_{"CRACKED CHIMES"};
    juce::String defaultBlurb_{"sovereign metamorphic instrument"};
    juce::Colour defaultAccent_{Colours::kFilterPrimary};
    float nameColW_{115.0f};
    Readout readout_;
};

using HeaderInspector = CrackedChimesHeaderInspector;

} // namespace CrackedChimes::UI
