#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "FormlessProcessor.h"
#include <vector>
#include <array>
#include <memory>
#include <functional>

// ==============================================================================
// Formless LookAndFeel (Vector Dial Mechanics matching Hastur)
// ==============================================================================
class FormlessLookAndFeel : public juce::LookAndFeel_V4 {
public:
    FormlessLookAndFeel() {
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xFF0D1117));
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override {
        auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                             static_cast<float>(width), static_cast<float>(height)).reduced(2.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        auto centre = bounds.getCentre();
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        const float sweep = rotaryEndAngle - rotaryStartAngle;

        // Background track ring
        g.setColour(juce::Colour(0xFF21262D));
        juce::Path trackPath;
        trackPath.addCentredArc(centre.x, centre.y, radius - 4.0f, radius - 4.0f, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.strokePath(trackPath, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Active modulation arc with Bipolar Mid-Point Glow
        auto fillCol = slider.findColour(juce::Slider::rotarySliderFillColourId);
        g.setColour(fillCol);
        juce::Path activePath;
        const bool isBipolar = (slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0);

        if (isBipolar) {
            const float zeroNorm = static_cast<float>(-slider.getMinimum() / (slider.getMaximum() - slider.getMinimum()));
            const float midAngle = rotaryStartAngle + zeroNorm * sweep;
            if (std::abs(angle - midAngle) > 0.005f) {
                if (angle > midAngle) {
                    activePath.addCentredArc(centre.x, centre.y, radius - 4.0f, radius - 4.0f, 0.0f, midAngle, angle, true);
                } else {
                    activePath.addCentredArc(centre.x, centre.y, radius - 4.0f, radius - 4.0f, 0.0f, angle, midAngle, true);
                }
                g.strokePath(activePath, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        } else {
            if (sliderPosProportional > 0.001f) {
                activePath.addCentredArc(centre.x, centre.y, radius - 4.0f, radius - 4.0f, 0.0f, rotaryStartAngle, angle, true);
                g.strokePath(activePath, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        }

        // Dial body
        auto dialRadius = radius - 8.0f;
        g.setColour(juce::Colour(0xFF161B22));
        g.fillEllipse(centre.x - dialRadius, centre.y - dialRadius, dialRadius * 2.0f, dialRadius * 2.0f);
        g.setColour(juce::Colour(0xFF30363D));
        g.drawEllipse(centre.x - dialRadius, centre.y - dialRadius, dialRadius * 2.0f, dialRadius * 2.0f, 1.2f);

        // Position pointer needle
        juce::Path p;
        auto pointerLen = dialRadius * 0.75f;
        p.startNewSubPath(centre.x, centre.y);
        p.lineTo(centre.x + pointerLen * std::sin(angle), centre.y - pointerLen * std::cos(angle));
        g.setColour(juce::Colour(0xFFF0F6FC));
        g.strokePath(p, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Center jewel pip
        g.setColour(fillCol);
        g.fillEllipse(centre.x - 2.5f, centre.y - 2.5f, 5.0f, 5.0f);
    }
};

// ==============================================================================
// Segmented Button Bar Component
// ==============================================================================
class FormlessSegmentedBar : public juce::Component {
public:
    FormlessSegmentedBar(const std::vector<juce::String>& labels) : labels_(labels) {}

    void setAccentColour(juce::Colour c) {
        accent_ = c;
        repaint();
    }

    void setState(int idx) {
        idx = std::clamp(idx, 0, static_cast<int>(labels_.size()) - 1);
        if (activeIndex_ != idx) {
            activeIndex_ = idx;
            repaint();
        }
    }

    int getState() const noexcept { return activeIndex_; }
    std::function<void(int)> onSelect;

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.isEmpty() || labels_.empty()) return;

        g.setColour(juce::Colour(0xFF161B22));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF30363D));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        const int numSegs = static_cast<int>(labels_.size());
        const float segW = bounds.getWidth() / static_cast<float>(numSegs);

        auto activeRect = juce::Rectangle<float>(bounds.getX() + activeIndex_ * segW, bounds.getY(), segW, bounds.getHeight());
        g.setColour(accent_);
        g.fillRoundedRectangle(activeRect.reduced(1.0f), 3.0f);

        g.setColour(juce::Colour(0xFF262D38));
        for (int i = 1; i < numSegs; ++i) {
            if (i != activeIndex_ && i != activeIndex_ + 1) {
                float divX = bounds.getX() + i * segW;
                g.drawVerticalLine(static_cast<int>(std::round(divX)), bounds.getY() + 3.0f, bounds.getBottom() - 3.0f);
            }
        }

        g.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
        for (int i = 0; i < numSegs; ++i) {
            bool isActive = (i == activeIndex_);
            auto segRect = juce::Rectangle<float>(bounds.getX() + i * segW, bounds.getY(), segW, bounds.getHeight());
            g.setColour(isActive ? juce::Colour(0xFF0D1117) : juce::Colour(0xFFF0F6FC));
            g.drawText(labels_[i], segRect, juce::Justification::centred);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        const float segW = static_cast<float>(getWidth()) / static_cast<float>(labels_.size());
        int clickedIdx = std::clamp(static_cast<int>(e.position.x / segW), 0, static_cast<int>(labels_.size()) - 1);
        if (clickedIdx != activeIndex_) {
            activeIndex_ = clickedIdx;
            if (onSelect) onSelect(activeIndex_);
            repaint();
        }
    }

private:
    std::vector<juce::String> labels_;
    int activeIndex_{0};
    juce::Colour accent_{0xFF00C8FF};
};

// ==============================================================================
// 3-Way Rocker Toggle (Lifeform Triad)
// ==============================================================================
class FormlessRocker3 : public juce::Component {
public:
    FormlessRocker3(const std::array<juce::String, 3>& labels) : labels_(labels) {}

    void setState(int idx) {
        idx = std::clamp(idx, 0, 2);
        if (activeIndex_ != idx) {
            activeIndex_ = idx;
            repaint();
        }
    }

    int getState() const noexcept { return activeIndex_; }
    std::function<void(int)> onSelect;

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xFF161B22));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF30363D));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        const float segW = bounds.getWidth() / 3.0f;
        auto activeRect = juce::Rectangle<float>(bounds.getX() + activeIndex_ * segW, bounds.getY(), segW, bounds.getHeight());
        g.setColour(colours_[activeIndex_]);
        g.fillRoundedRectangle(activeRect.reduced(1.0f), 3.0f);

        g.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
        for (int i = 0; i < 3; ++i) {
            bool isActive = (i == activeIndex_);
            auto segRect = juce::Rectangle<float>(bounds.getX() + i * segW, bounds.getY(), segW, bounds.getHeight());
            g.setColour(isActive ? juce::Colour(0xFF0D1117) : juce::Colour(0xFFF0F6FC));
            g.drawText(labels_[i], segRect, juce::Justification::centred);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        const float segW = static_cast<float>(getWidth()) / 3.0f;
        int clickedIdx = std::clamp(static_cast<int>(e.position.x / segW), 0, 2);
        if (clickedIdx != activeIndex_) {
            activeIndex_ = clickedIdx;
            if (onSelect) onSelect(activeIndex_);
            repaint();
        }
    }

private:
    std::array<juce::String, 3> labels_;
    int activeIndex_{2}; // Default: Organic
    std::array<juce::Colour, 3> colours_{
        juce::Colour(0xFF00C8FF), // Synthetic
        juce::Colour(0xFFE08C00), // Cyborg
        juce::Colour(0xFF6BBF59)  // Organic
    };
};

// ==============================================================================
// Formless Spawn AudioProcessorEditor
// ==============================================================================
class FormlessSpawnAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
    explicit FormlessSpawnAudioProcessorEditor(FormlessSpawnAudioProcessor&);
    ~FormlessSpawnAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    struct KnobItem {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    void setupKnob(KnobItem& item, const juce::String& paramId, const juce::String& name, juce::Colour col, bool isBipolar = false);
    void updateModelLabels(int modelIndex);

    FormlessSpawnAudioProcessor& processor_;
    FormlessLookAndFeel lnf_;

    // Header Selectors
    FormlessSegmentedBar roleBar_{{"BD", "SD", "HH", "PERC", "FX"}};
    FormlessSegmentedBar modelBar_{{"ER-1", "909", "FOLD"}};
    FormlessRocker3      lifeformRocker_{{"SYN", "CYB", "ORG"}};
    FormlessSegmentedBar keyTrackBar_{{"FIXED", "KEY"}};
    juce::TextButton     trigButton_{"TRIG"};

    // Card 1 Knobs: Drum Core & Pitch Lab (8)
    KnobItem pitchKnob_;        // Hero 72
    KnobItem pitchDepthKnob_;   // Standard 52 (bipolar)
    KnobItem pitchDecayKnob_;   // Standard 52
    KnobItem pitchCurveKnob_;   // Standard 52 (bipolar)
    KnobItem clickTuneKnob_;    // Standard 52
    KnobItem clickMixKnob_;     // Standard 52
    KnobItem subLevelKnob_;     // Standard 52
    KnobItem lowBoostKnob_;     // Standard 52

    // Card 2 Knobs: Model Timbre & Acoustic Lab (8)
    KnobItem timbreXKnob_;      // Standard 52
    KnobItem timbreYKnob_;      // Standard 52
    KnobItem timbreZKnob_;      // Standard 52
    KnobItem timbreTKnob_;      // Standard 52 (Timbre Settle)
    KnobItem fmMorphKnob_;      // Standard 52
    KnobItem dampKnob_;         // Standard 52
    KnobItem panKnob_;          // Standard 52 (bipolar)
    KnobItem velSensKnob_;      // Standard 52

    // Card 3 Left Pod: Stepped Random (4 knobs + 1 button)
    KnobItem randPitchKnob_;    // Standard 52
    KnobItem randDecayKnob_;    // Standard 52
    KnobItem randTimbreKnob_;   // Standard 52
    KnobItem randScatterKnob_;  // Standard 52
    FormlessSegmentedBar randModeBar_{{"FREE", "LOOP16"}};

    // Card 3 Center Altar: The One Big Knob (1 Big + 1 Standard)
    KnobItem macroDecayKnob_;   // Giant 92
    KnobItem volumeKnob_;       // Standard 52

    // Card 3 Right Pod: VCF & Universal Mojo Strip (7 knobs + 2 buttons)
    KnobItem cutoffKnob_;       // Hero 72
    KnobItem resonanceKnob_;    // Standard 52
    KnobItem preDriveKnob_;     // Standard 52
    KnobItem crushKnob_;        // Standard 52
    KnobItem bitsKnob_;         // Standard 52
    KnobItem tapeGlueKnob_;     // Standard 52
    KnobItem mojoMixKnob_;      // Standard 52
    FormlessSegmentedBar vcfModeBar_{{"DIODE", "LPG"}};
    FormlessSegmentedBar mojoProfileBar_{{"CLEAN", "MD-12", "SP-12", "IRON"}};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FormlessSpawnAudioProcessorEditor)
};