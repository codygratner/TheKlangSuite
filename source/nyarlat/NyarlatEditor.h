#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "NyarlatProcessor.h"
#include "ZCubeVisualizer.h"
#include "NyarlatControlInfo.h"
#include "../ui/EditorScaleManager.h"
#include <vector>
#include <memory>
#include <array>
#include <unordered_map>
#include <functional>

namespace Nyarlat {

// ==============================================================================
// PresetStubComponent: Recessed Identity & Preset Capsule
// ==============================================================================
class PresetStubComponent : public juce::Component {
public:
    PresetStubComponent() {
        setInterceptsMouseClicks(false, false);
    }

    void setAccentColour(juce::Colour c) {
        if (accent_ != c) {
            accent_ = c;
            repaint();
        }
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xFF0F131A));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF262D38));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        g.setFont(juce::Font(juce::FontOptions(12.5f).withStyle("Bold")));
        g.setColour(accent_.withAlpha(0.92f));
        g.drawText("<  01: DEFAULT VOID  >", bounds, juce::Justification::centred);
    }

private:
    juce::Colour accent_{0xFF00C8F0};
};

// ==============================================================================
// HeaderInspector: Universal Header Mouse-Over & Live Drag Telemetry Panel
// ==============================================================================
class HeaderInspector : public juce::Component {
public:
    struct Readout {
        juce::String name;
        juce::String value;
        juce::String blurb;
        juce::Colour accent{0xFF00C8F0};
        bool isDefault{false};
    };

    HeaderInspector() {
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
        d.name = "NYARLAT";
        d.value = "READY";
        d.blurb = "16-voice metamorphic Z-domain synthesizer (3 bitmask oscs x 14-pole Z-cube)";
        d.accent = juce::Colour(0xFF00C8F0);
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

        // Col 1: Parameter / System Name
        auto nameRect = juce::Rectangle<float>(bounds.getX() + 10.0f, bounds.getY(), 100.0f, bounds.getHeight());
        g.setFont(juce::Font(juce::FontOptions(12.5f).withStyle("Bold")));
        g.setColour(readout_.accent);
        g.drawText(readout_.name, nameRect, juce::Justification::centredLeft, true);

        // Col 2: Explanation blurb across the full remaining area
        float blurbW = std::max(50.0f, bounds.getWidth() - 122.0f);
        auto blurbRect = juce::Rectangle<float>(bounds.getX() + 116.0f, bounds.getY(), blurbW, bounds.getHeight());
        g.setFont(juce::Font(juce::FontOptions(12.0f)));
        g.setColour(readout_.isDefault ? juce::Colour(0xFF6E7681) : juce::Colour(0xFFC9D1D9));
        g.drawFittedText(readout_.blurb, blurbRect.toNearestInt(), juce::Justification::centredLeft, 2);
    }

private:
    Readout readout_;
};

// ==============================================================================
// 3-Way Rocker Toggle (Lifeform Triad)
// ==============================================================================
class RockerToggle3 : public juce::Component {
public:
    RockerToggle3(const std::array<juce::String, 3>& labels) : labels_(labels) {}

    void setIndex(int idx, juce::NotificationType notif = juce::sendNotificationAsync) {
        idx = std::clamp(idx, 0, 2);
        if (activeIndex_ != idx) {
            activeIndex_ = idx;
            repaint();
            if (notif != juce::dontSendNotification && onChange) {
                onChange(activeIndex_);
            }
        }
    }

    int getIndex() const noexcept { return activeIndex_; }

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

        g.setFont(juce::Font(juce::FontOptions(9.5f).withStyle("Bold")));
        for (int i = 0; i < 3; ++i) {
            bool isActive = (i == activeIndex_);
            auto segRect = juce::Rectangle<float>(bounds.getX() + i * segW, bounds.getY(), segW, bounds.getHeight());
            g.setColour(isActive ? juce::Colour(0xFF0D1117) : juce::Colour(0xFF8B949E));
            g.drawFittedText(labels_[i], segRect.toNearestInt(), juce::Justification::centred, 1, 0.85f);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        const float segW = static_cast<float>(getWidth()) / 3.0f;
        int clickedIdx = std::clamp(static_cast<int>(e.position.x / segW), 0, 2);
        setIndex(clickedIdx);
    }

    std::function<void(int)> onChange;

private:
    std::array<juce::String, 3> labels_;
    int activeIndex_{0}; // Default: Synthetic
    std::array<juce::Colour, 3> colours_{
        Colours::kSynthetic, // 0xFF00C8FF (Tron Blue)
        Colours::kCyborg,    // 0xFFE08C00 (Toxic Amber)
        Colours::kOrganic    // 0xFF6BBF59 (Warm Green)
    };
};

// ==============================================================================
// NyarlatLookAndFeel: Crackchimes Fleet Standard LookAndFeel
// ==============================================================================
class NyarlatLookAndFeel : public juce::LookAndFeel_V4 {
public:
    NyarlatLookAndFeel() {
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xFF0D1117));
    }

    void setAccentColour(juce::Colour c) { accentColour_ = c; }
    juce::Colour getAccentColour() const noexcept { return accentColour_; }

    static void drawKiloheartsValueBadge(juce::Graphics& g, const juce::Rectangle<float>& bounds,
                                         const juce::String& text, juce::Colour accentColour) {
        if (text.isEmpty()) return;
        const auto font = juce::Font(juce::FontOptions(10.5f).withStyle("Bold"));
        const float textW = juce::GlyphArrangement::getStringWidth(font, text);
        const float badgeW = std::clamp(textW + 10.0f, 26.0f, bounds.getWidth() - 6.0f);
        const float badgeH = 14.0f;
        const float badgeX = bounds.getCentreX() - badgeW * 0.5f;
        const float badgeY = bounds.getCentreY() - badgeH * 0.5f;
        auto badgeRect = juce::Rectangle<float>(badgeX, badgeY, badgeW, badgeH);

        g.setColour(juce::Colour(0xFF0D1117).withAlpha(0.92f));
        g.fillRoundedRectangle(badgeRect, 3.0f);
        g.setColour(accentColour.withAlpha(0.85f));
        g.drawRoundedRectangle(badgeRect, 3.0f, 1.0f);

        g.setFont(font);
        g.setColour(juce::Colour(0xFFF0F6FC));
        g.drawFittedText(text, badgeRect.toNearestInt(), juce::Justification::centred, 1);
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
        if (!fillCol.isOpaque()) fillCol = accentColour_;
        g.setColour(fillCol);
        juce::Path activePath;
        const bool isBipolar = (slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0) ||
                               (slider.getProperties().contains("isBipolar") && static_cast<bool>(slider.getProperties()["isBipolar"]));
        if (isBipolar) {
            float zeroNorm = 0.5f;
            if (slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0) {
                zeroNorm = static_cast<float>(-slider.getMinimum() / (slider.getMaximum() - slider.getMinimum()));
            }
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

        // Concentric Dual-Radar Telemetry Arc Halos
        // 1. Outer Ring: Stepped Random & Chaos (Caustic Yellow 0xFFF2E10F)
        if (slider.getProperties().contains("randLo") && slider.getProperties().contains("randHi")) {
            const float lo = static_cast<float>(slider.getProperties()["randLo"]);
            const float hi = static_cast<float>(slider.getProperties()["randHi"]);
            if (hi - lo > 0.002f) {
                const auto yellowCol = juce::Colour(0xFFF2E10F);
                juce::Path randArc;
                randArc.addCentredArc(centre.x, centre.y, radius - 1.5f, radius - 1.5f, 0.0f,
                                      rotaryStartAngle + lo * sweep, rotaryStartAngle + hi * sweep, true);
                g.setColour(yellowCol.withAlpha(0.25f));
                g.strokePath(randArc, juce::PathStrokeType(4.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                g.setColour(yellowCol.withAlpha(0.95f));
                g.strokePath(randArc, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

                if (slider.getProperties().contains("randPip")) {
                    const float pipNorm = static_cast<float>(slider.getProperties()["randPip"]);
                    const float pipAngle = rotaryStartAngle + pipNorm * sweep;
                    const float px = centre.x + (radius - 1.5f) * std::sin(pipAngle);
                    const float py = centre.y - (radius - 1.5f) * std::cos(pipAngle);

                    g.setColour(juce::Colours::white);
                    g.fillEllipse(px - 2.5f, py - 2.5f, 5.0f, 5.0f);
                    g.setColour(yellowCol);
                    g.drawEllipse(px - 3.5f, py - 3.5f, 7.0f, 7.0f, 1.0f);
                }
            }
        }

        // 2. Inner Ring: Macro Vector Modulation (Movie Poster Orange 0xFFFF7B25)
        if (slider.getProperties().contains("modLo") && slider.getProperties().contains("modHi")) {
            const float lo = static_cast<float>(slider.getProperties()["modLo"]);
            const float hi = static_cast<float>(slider.getProperties()["modHi"]);
            if (hi - lo > 0.002f) {
                const auto orangeCol = juce::Colour(0xFFFF7B25);
                juce::Path modArc;
                modArc.addCentredArc(centre.x, centre.y, radius - 4.5f, radius - 4.5f, 0.0f,
                                     rotaryStartAngle + lo * sweep, rotaryStartAngle + hi * sweep, true);
                g.setColour(orangeCol.withAlpha(0.25f));
                g.strokePath(modArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                g.setColour(orangeCol.withAlpha(0.95f));
                g.strokePath(modArc, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

                if (slider.getProperties().contains("modPip")) {
                    const float pipNorm = static_cast<float>(slider.getProperties()["modPip"]);
                    const float pipAngle = rotaryStartAngle + pipNorm * sweep;
                    const float px = centre.x + (radius - 4.5f) * std::sin(pipAngle);
                    const float py = centre.y - (radius - 4.5f) * std::cos(pipAngle);

                    g.setColour(juce::Colours::white);
                    g.fillEllipse(px - 2.0f, py - 2.0f, 4.0f, 4.0f);
                    g.setColour(orangeCol);
                    g.drawEllipse(px - 3.0f, py - 3.0f, 6.0f, 6.0f, 1.0f);
                }
            }
        }

        // Kilohearts In-Dial Value Badge
        bool isInteracting = slider.isMouseButtonDown();
        if (!isInteracting && slider.getProperties().contains("badgeExpiry")) {
            auto expiry = static_cast<juce::int64>(static_cast<double>(slider.getProperties()["badgeExpiry"]));
            if (juce::Time::getMillisecondCounter() < expiry) {
                isInteracting = true;
            }
        }
        if (isInteracting) {
            juce::String text = slider.textFromValueFunction
                ? slider.textFromValueFunction(slider.getValue())
                : slider.getTextFromValue(slider.getValue());
            drawKiloheartsValueBadge(g, bounds, text, fillCol);
        }
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float, float,
                          const juce::Slider::SliderStyle, juce::Slider& slider) override {
        const auto trackCol = slider.isColourSpecified(juce::Slider::trackColourId)
            ? slider.findColour(juce::Slider::trackColourId) : accentColour_;
        const auto thumbCol = slider.isColourSpecified(juce::Slider::thumbColourId)
            ? slider.findColour(juce::Slider::thumbColourId) : accentColour_;

        auto trackX = x + width * 0.5f;

        // Track Groove
        g.setColour(juce::Colour(0xFF0F131A));
        g.fillRoundedRectangle(trackX - 3.0f, static_cast<float>(y), 6.0f, static_cast<float>(height), 3.0f);
        g.setColour(juce::Colour(0xFF262D38));
        g.drawRoundedRectangle(trackX - 3.0f, static_cast<float>(y), 6.0f, static_cast<float>(height), 3.0f, 1.0f);

        // Active fill
        float bottomY = static_cast<float>(y + height);
        g.setColour(trackCol.withAlpha(0.75f));
        if (sliderPos < bottomY) {
            g.fillRoundedRectangle(trackX - 2.0f, sliderPos, 4.0f, bottomY - sliderPos, 2.0f);
        }

        // Fader Cap
        float capW = std::clamp(static_cast<float>(width) - 6.0f, 28.0f, 36.0f);
        float capH = 14.0f;
        auto capBounds = juce::Rectangle<float>(trackX - capW * 0.5f, sliderPos - capH * 0.5f, capW, capH);

        g.setColour(juce::Colour(0xFF21262D));
        g.fillRoundedRectangle(capBounds, 3.0f);
        g.setColour(juce::Colour(0xFF484F58));
        g.drawRoundedRectangle(capBounds, 3.0f, 1.0f);

        // Center indicator line
        g.setColour(thumbCol);
        g.fillRect(capBounds.getX() + 3.0f, sliderPos - 1.0f, capBounds.getWidth() - 6.0f, 2.0f);
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox&) override {
        auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
        g.setColour(juce::Colour(0xFF161B22));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF30363D));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        float arrX = static_cast<float>(width) - 14.0f;
        float arrY = static_cast<float>(height) * 0.5f - 2.0f;
        juce::Path arr;
        arr.startNewSubPath(arrX, arrY);
        arr.lineTo(arrX + 6.0f, arrY);
        arr.lineTo(arrX + 3.0f, arrY + 4.0f);
        arr.closeSubPath();
        g.setColour(juce::Colour(0xFF8B949E));
        g.fillPath(arr);
    }

private:
    juce::Colour accentColour_{0xFF00C8F0};
};

class NyarlatEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    static constexpr int kBaseWidth = 1160;
    static constexpr int kBaseHeight = 870;

    NyarlatEditor(NyarlatProcessor& proc);
    ~NyarlatEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;

private:
    void timerCallback() override;
    void inspectComponent(juce::Component* comp);

    struct RotaryKnob {
        juce::Slider slider;
        juce::Label label;
        juce::String paramId;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    struct SliderFader {
        juce::Slider slider;
        juce::Label label;
        juce::String paramId;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    struct ComboBoxItem {
        juce::ComboBox combo;
        juce::Label label;
        juce::String paramId;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
    };

    class ContentCanvas : public juce::Component {
    public:
        std::function<void(juce::Graphics&)> onPaint;
        void paint(juce::Graphics& g) override {
            if (onPaint) onPaint(g);
        }
    };

    void setupKnob(RotaryKnob& item, const juce::String& paramId, const juce::String& label, juce::Colour col);
    void setupFader(SliderFader& item, const juce::String& paramId, const juce::String& label, juce::Colour col);
    void setupCombo(ComboBoxItem& item, const juce::String& paramId, const juce::String& label);

    void layoutControls();
    void applyDragSensitivity(float s);
    void paintCanvas(juce::Graphics& g);

    NyarlatProcessor& processor_;
    NyarlatLookAndFeel lnf_;

    // Resizability Container & Constrainer
    ContentCanvas contentContainer;
    juce::ComponentBoundsConstrainer constrainer;
    rlyeh::ui::EditorScaleManager::Writer scaleWriter_;
    float effScale_ = 1.0f;
    juce::Point<int> expectedSize_{ kBaseWidth, kBaseHeight };

    // Master Header Components
    juce::Label titleLabel;
    juce::Label subtitleLabel;
    PresetStubComponent presetStub_;
    HeaderInspector headerInspector_;
    RockerToggle3 lifeModeToggle_{{"SYNTHETIC", "CYBORG", "ORGANIC"}};
    std::unique_ptr<juce::ParameterAttachment> lifeModeAttachment_;

    // Visualizers
    std::unique_ptr<ZCubeVisualizer> zCubeRadar_;

    // All parameter controls
    std::vector<std::unique_ptr<RotaryKnob>> knobs_;
    std::vector<std::unique_ptr<SliderFader>> faders_;
    std::vector<std::unique_ptr<ComboBoxItem>> combos_;

    std::unordered_map<juce::Component*, const ControlInfo*> controlMap_;

    // Moving lerp tracking
    std::vector<float> smoothedRandPips_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NyarlatEditor)
};

} // namespace Nyarlat
