#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "DigiStrikeFS2Processor.h"
#include "DigiStrikeFS2ControlInfo.h"
#include <vector>
#include <array>
#include <memory>
#include <functional>

// ==============================================================================
// DigiStrikeFS2Editor.h — Tactical Console GUI for DigiStrike FS2
// ==============================================================================
// Sovereign Tactical Console matching Formless Spawn visual & tactile standards.
// Part of The Klang Suite SOCEP Engine. Copyright (c) 2026 Cracked Chimes.
// ==============================================================================

// ==============================================================================
// HeaderInspector: Universal Header Mouse-Over & Live Drag Telemetry Panel
// ==============================================================================
class DigiStrikeHeaderInspector : public juce::Component {
public:
    struct Readout {
        juce::String name;
        juce::String value;
        juce::String blurb;
        juce::Colour accent{0xFFFFAA00};
        bool isDefault{false};
    };

    DigiStrikeHeaderInspector() {
        setInterceptsMouseClicks(false, false);
        setReadoutDefault();
    }

    void setReadout(Readout r) {
        if (readout_.name == r.name && readout_.value == r.value &&
            readout_.blurb == r.blurb && readout_.accent == r.accent &&
            readout_.isDefault == r.isDefault)
            return;

        readout_ = std::move(r);
        repaint();
    }

    void setReadoutDefault() {
        Readout d;
        d.name = "DIGISTRIKE FS2";
        d.value = "READY";
        d.blurb = "sovereign vintage PCM workstation (2,069 voices across 20 manufacturer lineages)";
        d.accent = juce::Colour(0xFFFFAA00);
        d.isDefault = true;
        setReadout(d);
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();

        // Recessed OLED display cavity
        g.setColour(juce::Colour(0xFF0F131A));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF262D38));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        // Col 1: Parameter Name (x: 10, w: 90, 12pt bold)
        auto nameRect = juce::Rectangle<float>(bounds.getX() + 10.0f, bounds.getY(), 90.0f, bounds.getHeight());
        g.setFont(juce::Font(juce::FontOptions(12.0f).withStyle("Bold")));
        g.setColour(readout_.accent);
        g.drawText(readout_.name, nameRect, juce::Justification::centredLeft, true);

        // Col 2: Explanation blurb across remaining area (2 lines)
        float blurbW = std::max(40.0f, bounds.getWidth() - 106.0f);
        auto blurbRect = juce::Rectangle<float>(bounds.getX() + 102.0f, bounds.getY(), blurbW, bounds.getHeight());
        g.setFont(juce::Font(juce::FontOptions(11.0f)));
        g.setColour(readout_.isDefault ? juce::Colour(0xFF6E7681) : juce::Colour(0xFFC9D1D9));
        g.drawFittedText(readout_.blurb, blurbRect.toNearestInt(), juce::Justification::centredLeft, 2);
    }

private:
    Readout readout_;
};

// ==============================================================================
// 3-Way Rocker Toggle (Lifeform Triad)
// ==============================================================================
class DigiStrikeRocker3 : public juce::Component {
public:
    DigiStrikeRocker3(const std::array<juce::String, 3>& labels) : labels_(labels) {}

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
// TactileTriggerPad: Center Altar Drum Trigger with Tactile Gold Flash
// ==============================================================================
class DigiStrikeTriggerPad : public juce::Component {
public:
    DigiStrikeTriggerPad() = default;

    std::function<void()> onTrigger;

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        if (isDown_) {
            g.setColour(juce::Colour(0xFFFFAA00)); // Gold flash on strike
            g.fillRoundedRectangle(bounds, 5.0f);
            g.setFont(juce::Font(juce::FontOptions(13.0f).withStyle("Bold")));
            g.setColour(juce::Colour(0xFF0D1117));
            g.drawText("TRIG", bounds, juce::Justification::centred);
        } else {
            g.setColour(juce::Colour(0xFF21262D));
            g.fillRoundedRectangle(bounds, 5.0f);
            g.setColour(juce::Colour(0xFF30363D));
            g.drawRoundedRectangle(bounds, 5.0f, 1.2f);
            g.setFont(juce::Font(juce::FontOptions(13.0f).withStyle("Bold")));
            g.setColour(juce::Colour(0xFFF0F6FC));
            g.drawText("TRIG", bounds, juce::Justification::centred);
        }
    }

    void mouseDown(const juce::MouseEvent&) override {
        isDown_ = true;
        repaint();
        if (onTrigger) onTrigger();
    }

    void mouseUp(const juce::MouseEvent&) override {
        isDown_ = false;
        repaint();
    }

private:
    bool isDown_{false};
};

// ==============================================================================
// LimiterTactileButton: Tactile Limit Button with 3-State Hardware Jewel LED Pip
// ==============================================================================
class DigiStrikeLimiterButton : public juce::Button {
public:
    DigiStrikeLimiterButton() : juce::Button("LIMIT") {
        setClickingTogglesState(true);
    }

    void updateTelemetry(float gr) noexcept {
        if (std::abs(gr - currentGr_) > 0.004f) {
            currentGr_ = gr;
            repaint();
        }
    }

    void paintButton(juce::Graphics& g, bool, bool) override {
        auto bounds = getLocalBounds().toFloat();
        const bool on = getToggleState();

        const auto bgCol = on ? juce::Colour(0xFF21262D) : juce::Colour(0xFF161B22);
        g.setColour(bgCol);
        g.fillRoundedRectangle(bounds, 3.0f);

        const auto borderCol = on ? juce::Colour(0xFF484F58) : juce::Colour(0xFF30363D);
        g.setColour(borderCol);
        g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.0f);

        // Hardware Jewel LED Pip
        const float ledX = 7.0f;
        const float ledY = bounds.getCentreY() - 2.5f;
        const juce::Rectangle<float> ledRect(ledX, ledY, 5.0f, 5.0f);

        g.setColour(juce::Colour(0xFF0D1117));
        g.fillEllipse(ledRect.expanded(1.0f));

        if (!on) {
            g.setColour(juce::Colour(0xFF21262D));
            g.fillEllipse(ledRect);
        } else {
            juce::Colour ledCol = juce::Colour(0xFF39D353);
            juce::Colour glowCol = juce::Colour(0x662EA043);
            if (currentGr_ < 0.6310f) {
                ledCol = juce::Colour(0xFFFF3B30);
                glowCol = juce::Colour(0x88DA3633);
            } else if (currentGr_ < 0.9441f) {
                ledCol = juce::Colour(0xFFF3CA3E);
                glowCol = juce::Colour(0x66D29922);
            }

            g.setColour(glowCol);
            g.fillEllipse(ledRect.expanded(2.5f));
            g.setColour(ledCol);
            g.fillEllipse(ledRect);
            g.setColour(juce::Colours::white.withAlpha(0.6f));
            g.fillEllipse(juce::Rectangle<float>(ledX + 1.0f, ledY + 1.0f, 1.5f, 1.5f));
        }

        const auto textCol = on ? juce::Colours::white : juce::Colour(0xFF8B949E);
        g.setColour(textCol);
        g.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
        const juce::Rectangle<float> labelArea(15.0f, 0.0f, bounds.getWidth() - 17.0f, bounds.getHeight());
        g.drawText("LIMIT", labelArea, juce::Justification::centred, false);
    }

private:
    float currentGr_{1.0f};
};

// ==============================================================================
// DigiStrike LookAndFeel (Vector Dial Mechanics matching Formless Spawn)
// ==============================================================================
class DigiStrikeLookAndFeel : public juce::LookAndFeel_V4 {
public:
    DigiStrikeLookAndFeel() {
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

        const bool compact = radius < 26.0f;
        float trackRadius = compact ? radius - 3.5f : radius - 4.5f;
        float trackStroke = compact ? 3.0f : 3.5f;
        float dialRadius  = compact ? radius - 7.0f : radius - 8.5f;

        // 1. Background track ring
        g.setColour(juce::Colour(0xFF21262D));
        juce::Path trackPath;
        trackPath.addCentredArc(centre.x, centre.y, trackRadius, trackRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.strokePath(trackPath, juce::PathStrokeType(trackStroke, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // 2. Active modulation arc
        auto fillCol = slider.findColour(juce::Slider::rotarySliderFillColourId);
        g.setColour(fillCol);
        juce::Path activePath;
        const bool rangeBipolar = (slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0);
        const bool propBipolar = slider.getProperties().contains("isBipolar") && static_cast<bool>(slider.getProperties()["isBipolar"]);
        const bool isBipolar = rangeBipolar || propBipolar;

        if (isBipolar) {
            float zeroNorm = 0.5f;
            if (rangeBipolar)
                zeroNorm = static_cast<float>(-slider.getMinimum() / (slider.getMaximum() - slider.getMinimum()));
            const float midAngle = rotaryStartAngle + zeroNorm * sweep;
            if (std::abs(angle - midAngle) > 0.005f) {
                if (angle > midAngle)
                    activePath.addCentredArc(centre.x, centre.y, trackRadius, trackRadius, 0.0f, midAngle, angle, true);
                else
                    activePath.addCentredArc(centre.x, centre.y, trackRadius, trackRadius, 0.0f, angle, midAngle, true);
                g.strokePath(activePath, juce::PathStrokeType(trackStroke, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        } else {
            if (sliderPosProportional > 0.001f) {
                activePath.addCentredArc(centre.x, centre.y, trackRadius, trackRadius, 0.0f, rotaryStartAngle, angle, true);
                g.strokePath(activePath, juce::PathStrokeType(trackStroke, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        }

        // 3. Dial body
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
        g.strokePath(p, juce::PathStrokeType(compact ? 2.0f : 2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Center jewel pip
        g.setColour(fillCol);
        const float pipD = compact ? 4.0f : 5.0f;
        g.fillEllipse(centre.x - pipD * 0.5f, centre.y - pipD * 0.5f, pipD, pipD);

        // Ephemeral in-dial value badge on mouse interaction
        if (slider.isMouseButtonDown()) {
            juce::String text = slider.textFromValueFunction
                ? slider.textFromValueFunction(slider.getValue())
                : slider.getTextFromValue(slider.getValue());
            if (text.isNotEmpty()) {
                const auto font = juce::Font(juce::FontOptions(10.0f).withStyle("Bold"));
                const float textW = juce::GlyphArrangement::getStringWidth(font, text);
                const float badgeW = std::clamp(textW + 10.0f, 26.0f, bounds.getWidth() - 4.0f);
                const float badgeH = 14.0f;
                const float badgeX = bounds.getCentreX() - badgeW * 0.5f;
                const float badgeY = bounds.getBottom() - badgeH - 1.0f;
                const auto badgeRect = juce::Rectangle<float>(badgeX, badgeY, badgeW, badgeH);

                g.setColour(juce::Colour(0xFA090D12));
                g.fillRoundedRectangle(badgeRect, 3.5f);
                g.setColour(fillCol.withAlpha(0.65f));
                g.drawRoundedRectangle(badgeRect, 3.5f, 1.0f);
                g.setFont(font);
                g.setColour(juce::Colour(0xFFF0F6FC));
                g.drawFittedText(text, badgeRect.toNearestInt(), juce::Justification::centred, 1);
            }
        }
    }
};

// ==============================================================================
// DigiStrikeFS2AudioProcessorEditor
// ==============================================================================
class DigiStrikeFS2AudioProcessorEditor : public juce::AudioProcessorEditor,
                                          public juce::Timer {
public:
    static constexpr int kBaseWidth = 900;
    static constexpr int kBaseHeight = 560;

    explicit DigiStrikeFS2AudioProcessorEditor(DigiStrikeFS2AudioProcessor&);
    ~DigiStrikeFS2AudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    struct KnobItem {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

private:
    void setupKnob(KnobItem& item, const juce::String& paramId, const juce::String& name,
                   const juce::String& unit, const juce::String& blurb,
                   juce::Colour col, bool isBipolar = false);

    DigiStrikeFS2AudioProcessor& processor_;
    DigiStrikeLookAndFeel lnf_;

    // Header Row
    DigiStrikeHeaderInspector headerInspector_;
    juce::ComboBox categoryBox_;
    juce::ComboBox lineageBox_;
    juce::ComboBox voiceModeBox_;
    DigiStrikeRocker3 lifeformRocker_{{"SYNTHETIC", "CYBORG", "ORGANIC"}};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> categoryAttach_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lineageAttach_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> voiceModeAttach_;

    // Card 1 Knobs: Master Macro & Dynamics (Top-Left, 8)
    KnobItem macroXKnob_;
    KnobItem macroYKnob_;
    KnobItem macroZKnob_;
    KnobItem macroTKnob_;
    KnobItem velAggrKnob_;
    KnobItem velTimbKnob_;
    KnobItem panKnob_;
    KnobItem decayCurveKnob_;

    // Card 2 Knobs: Uniform Sound Lab (Top-Right, 8)
    KnobItem pitchDecayKnob_;
    KnobItem clickKnob_;
    KnobItem startKnob_;
    KnobItem timeKnob_;
    KnobItem punchKnob_;
    KnobItem compandKnob_;
    KnobItem lengthKnob_;
    KnobItem morphKnob_;

    // Card 3 Pod 1: Stepped Random (Bottom-Left, 8)
    KnobItem randXKnob_;
    KnobItem randYKnob_;
    KnobItem randZKnob_;
    KnobItem randTKnob_;
    KnobItem randAggrKnob_;
    KnobItem randTimbKnob_;
    KnobItem randPanKnob_;
    KnobItem randDecayKnob_;

    // Card 3 Pod 2: Performance Altar
    KnobItem macroDecayKnob_;
    DigiStrikeTriggerPad trigButton_;

    // Card 3 Pod 3: VCF, Mojo & Output
    KnobItem cutoffKnob_;
    KnobItem resonanceKnob_;
    KnobItem filterEnvAmtKnob_;
    KnobItem filterDecayKnob_;
    KnobItem filterDriveKnob_;

    KnobItem crushKnob_;
    KnobItem shiftKnob_;
    KnobItem squashKnob_;
    KnobItem widerKnob_;
    KnobItem sustainKnob_;

    // Master Output Strip
    DigiStrikeLimiterButton limiterButton_;
    float guiLimiterGr_{1.0f};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> limiterAttachment_;
    KnobItem volumeKnob_;
    KnobItem ampDecayKnob_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DigiStrikeFS2AudioProcessorEditor)
};
