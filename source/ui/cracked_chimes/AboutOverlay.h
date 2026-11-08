#pragma once
// ==============================================================================
// AboutOverlay.h - In-Editor Modal About Overlay for Cracked Chimes Fleet
// Displays dynamic version (JucePlugin_VersionString), credits, attributions & links.
// Closes on Esc, clicking scrim, or clicking Close.
// ==============================================================================

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

namespace CrackedChimes {

class AboutOverlay : public juce::Component {
public:
    struct Attribution {
        juce::String label;
        juce::String url;
    };

    AboutOverlay(const juce::String& pluginName,
                 const juce::String& tagline,
                 juce::Colour accentColour,
                 const juce::Array<Attribution>& attributions = {})
        : pluginName_(pluginName),
          tagline_(tagline),
          accentColour_(accentColour),
          attributions_(attributions)
    {
        setInterceptsMouseClicks(true, true);

        closeButton_.setButtonText("Close");
        closeButton_.onClick = [this] { if (onClose) onClose(); };
        addAndMakeVisible(closeButton_);

        xButton_.setButtonText(juce::CharPointer_UTF8("\xc3\x97")); // Multiplication sign / X
        xButton_.onClick = [this] { if (onClose) onClose(); };
        addAndMakeVisible(xButton_);
    }

    void setOnClose(std::function<void()> cb) {
        onClose = std::move(cb);
    }

    bool keyPressed(const juce::KeyPress& key) override {
        if (key == juce::KeyPress::escapeKey) {
            if (onClose) onClose();
            return true;
        }
        return false;
    }

    void mouseDown(const juce::MouseEvent& e) override {
        auto cardBounds = getCardBounds();
        if (!cardBounds.contains(e.getPosition().toFloat())) {
            if (onClose) onClose();
        } else {
            // Check if user clicked an attribution link
            for (int i = 0; i < linkRects_.size(); ++i) {
                if (linkRects_[i].contains(e.getPosition().toFloat())) {
                    if (i < attributions_.size()) {
                        juce::URL(attributions_[i].url).launchInDefaultBrowser();
                    }
                    break;
                }
            }
        }
    }

    void mouseMove(const juce::MouseEvent& e) override {
        bool overLink = false;
        for (const auto& r : linkRects_) {
            if (r.contains(e.getPosition().toFloat())) {
                overLink = true;
                break;
            }
        }
        setMouseCursor(overLink ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
    }

    void resized() override {
        auto card = getCardBounds();
        closeButton_.setBounds(static_cast<int>(card.getCentreX() - 50.0f),
                               static_cast<int>(card.getBottom() - 40.0f),
                               100, 28);
        xButton_.setBounds(static_cast<int>(card.getRight() - 32.0f),
                           static_cast<int>(card.getY() + 10.0f),
                           22, 22);

        // Precompute link rects
        linkRects_.clear();
        float linkY = card.getY() + 175.0f;
        for (int i = 0; i < attributions_.size(); ++i) {
            linkRects_.add(juce::Rectangle<float>(card.getX() + 30.0f, linkY, card.getWidth() - 60.0f, 20.0f));
            linkY += 24.0f;
        }
    }

    void paint(juce::Graphics& g) override {
        // 1. Dark semi-transparent scrim
        g.setColour(juce::Colour(0xDD0B0E14));
        g.fillAll();

        // 2. Centered obsidian chassis card
        auto card = getCardBounds();
        g.setColour(juce::Colour(0xFF161B22));
        g.fillRoundedRectangle(card, 8.0f);
        g.setColour(accentColour_.withAlpha(0.6f));
        g.drawRoundedRectangle(card, 8.0f, 1.5f);

        // 3. Hero Header
        auto headerArea = card.reduced(24.0f, 20.0f);
        g.setFont(juce::Font(juce::FontOptions(24.0f).withStyle("Bold")));
        g.setColour(accentColour_);
        g.drawText(pluginName_, headerArea.removeFromTop(30.0f), juce::Justification::centredLeft);

        // Version Pill
        juce::String versionStr = "v";
#ifdef JucePlugin_VersionString
        versionStr += JucePlugin_VersionString;
#else
        versionStr += "1.0.0";
#endif
        auto pillRect = juce::Rectangle<float>(card.getX() + 24.0f + 140.0f, card.getY() + 24.0f, 70.0f, 20.0f);
        g.setColour(juce::Colour(0xFF21262D));
        g.fillRoundedRectangle(pillRect, 4.0f);
        g.setColour(juce::Colour(0xFF8B949E));
        g.drawRoundedRectangle(pillRect, 4.0f, 1.0f);
        g.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
        g.setColour(juce::Colour(0xFFF0F6FC));
        g.drawText(versionStr, pillRect, juce::Justification::centred);

        // Tagline
        headerArea.removeFromTop(4.0f);
        g.setFont(juce::Font(juce::FontOptions(12.5f)));
        g.setColour(juce::Colour(0xFFC9D1D9));
        g.drawText(tagline_, headerArea.removeFromTop(20.0f), juce::Justification::centredLeft);

        // Subtitle / Author
        g.setFont(juce::Font(juce::FontOptions(11.0f)));
        g.setColour(juce::Colour(0xFF8B949E));
        g.drawText("Designed & Engineered by Cody Gratner \xc2\xb7 The Cult of Klang / The Klang Suite",
                   headerArea.removeFromTop(18.0f), juce::Justification::centredLeft);

        // Separator
        headerArea.removeFromTop(10.0f);
        g.setColour(juce::Colour(0xFF30363D));
        g.drawHorizontalLine(static_cast<int>(headerArea.getY()), card.getX() + 24.0f, card.getRight() - 24.0f);
        headerArea.removeFromTop(12.0f);

        // 4. Attributions & Architecture Lineage
        g.setFont(juce::Font(juce::FontOptions(12.0f).withStyle("Bold")));
        g.setColour(accentColour_);
        g.drawText("ARCHITECTURE & CORE ATTRIBUTIONS", headerArea.removeFromTop(20.0f), juce::Justification::centredLeft);

        g.setFont(juce::Font(juce::FontOptions(11.5f)));
        for (int i = 0; i < attributions_.size() && i < linkRects_.size(); ++i) {
            auto r = linkRects_[i];
            g.setColour(juce::Colour(0xFF58A6FF));
            g.drawText("\xe2\x86\x92 " + attributions_[i].label, r, juce::Justification::centredLeft);
        }

        // Host & Runtime Telemetry
        auto telemRect = juce::Rectangle<float>(card.getX() + 24.0f, card.getBottom() - 75.0f, card.getWidth() - 48.0f, 24.0f);
        g.setFont(juce::Font(juce::FontOptions(10.5f)));
        g.setColour(juce::Colour(0xFF6E7681));
        g.drawText("Format: VST3/AU/Standalone \xc2\xb7 64-bit Floating Point \xc2\xb7 Zero-CPU Sleep Active",
                   telemRect, juce::Justification::centred);
    }

    std::function<void()> onClose;

private:
    juce::Rectangle<float> getCardBounds() const {
        float w = juce::jmin(560.0f, static_cast<float>(getWidth()) - 32.0f);
        float h = juce::jmin(380.0f, static_cast<float>(getHeight()) - 32.0f);
        float x = (static_cast<float>(getWidth()) - w) * 0.5f;
        float y = (static_cast<float>(getHeight()) - h) * 0.5f;
        return {x, y, w, h};
    }

    juce::String pluginName_;
    juce::String tagline_;
    juce::Colour accentColour_;
    juce::Array<Attribution> attributions_;
    juce::Array<juce::Rectangle<float>> linkRects_;

    juce::TextButton closeButton_;
    juce::TextButton xButton_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AboutOverlay)
};

} // namespace CrackedChimes
