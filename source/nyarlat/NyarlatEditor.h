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
#include <cmath>

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
    int activeIndex_{0};
    std::array<juce::Colour, 3> colours_{
        Colours::kSynthetic, // 0xFF00C8FF (Tron Blue)
        Colours::kCyborg,    // 0xFFE08C00 (Toxic Amber)
        Colours::kOrganic    // 0xFF6BBF59 (Warm Green)
    };
};

// ==============================================================================
// BitmaskWaveVisualizer: Real-Time Single-Cycle AKWF Stator Display
// ==============================================================================
class BitmaskWaveVisualizer : public juce::Component {
public:
    BitmaskWaveVisualizer(NyarlatProcessor& proc, int oscIndex)
        : proc_(proc), oscIndex_(oscIndex) {
        setInterceptsMouseClicks(false, false);
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xFF0D1117));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF262D38));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        // Center zero-crossing baseline
        g.setColour(juce::Colour(0xFF161B22));
        float midY = bounds.getCentreY();
        g.drawHorizontalLine(static_cast<int>(midY), bounds.getX() + 2.0f, bounds.getRight() - 2.0f);

        namespace Ids = nyarlatParameters::Ids;
        float waveVal = 0.0f;
        int maskVal = 24;
        int startVal = 0;
        float speedVal = 0.0f;
        int loopVal = 1;
        uint32_t bankVal = 0;

        if (oscIndex_ == 0) {
            bankVal = static_cast<uint32_t>(*proc_.getAPVTS().getRawParameterValue(Ids::osc1_bank));
            waveVal = *proc_.getAPVTS().getRawParameterValue(Ids::osc1_wave);
            maskVal = static_cast<int>(*proc_.getAPVTS().getRawParameterValue(Ids::osc1_mask));
            startVal = static_cast<int>(*proc_.getAPVTS().getRawParameterValue(Ids::osc1_start));
            speedVal = *proc_.getAPVTS().getRawParameterValue(Ids::osc1_speed);
            loopVal = static_cast<int>(*proc_.getAPVTS().getRawParameterValue(Ids::osc1_loop));
        } else if (oscIndex_ == 1) {
            bankVal = static_cast<uint32_t>(*proc_.getAPVTS().getRawParameterValue(Ids::osc2_bank));
            waveVal = *proc_.getAPVTS().getRawParameterValue(Ids::osc2_wave);
            maskVal = static_cast<int>(*proc_.getAPVTS().getRawParameterValue(Ids::osc2_mask));
            startVal = static_cast<int>(*proc_.getAPVTS().getRawParameterValue(Ids::osc2_start));
            speedVal = *proc_.getAPVTS().getRawParameterValue(Ids::osc2_speed);
            loopVal = static_cast<int>(*proc_.getAPVTS().getRawParameterValue(Ids::osc2_loop));
        } else {
            bankVal = static_cast<uint32_t>(*proc_.getAPVTS().getRawParameterValue(Ids::osc3_bank));
            waveVal = *proc_.getAPVTS().getRawParameterValue(Ids::osc3_wave);
            maskVal = static_cast<int>(*proc_.getAPVTS().getRawParameterValue(Ids::osc3_mask));
            startVal = static_cast<int>(*proc_.getAPVTS().getRawParameterValue(Ids::osc3_start));
            speedVal = *proc_.getAPVTS().getRawParameterValue(Ids::osc3_speed);
            loopVal = static_cast<int>(*proc_.getAPVTS().getRawParameterValue(Ids::osc3_loop));
        }

        std::array<float, 512> waveTable;
        AkwfRom::getInstance().interpolateWaveform(bankVal, waveVal, waveTable.data());

        int effectiveMask = juce::jlimit(1, 8191, (maskVal * 16) + 15);
        uint32_t statorOff = static_cast<uint32_t>((startVal + 512) & 0x1FF);

        bool activeVoiceFound = false;
        auto tele = proc_.getVoiceTelemetry();
        for (const auto& vt : tele) {
            if (vt.active) {
                statorOff = vt.statorOffsets[oscIndex_];
                activeVoiceFound = true;
                break;
            }
        }

        if (!activeVoiceFound && speedVal > 0.0f) {
            const int effLoop = (loopVal == 0) ? 1 : loopVal;
            previewPhase_ += speedVal * 0.033f; // ~30 Hz timer delta
            if (previewPhase_ >= 1.0f) previewPhase_ -= std::floor(previewPhase_);
            uint32_t p = static_cast<uint32_t>(previewPhase_ * 512.0f) & 0x1FFu;
            uint32_t sweep = (effLoop == 1) ? p : (p < 256u ? p * 2u : (511u - p) * 2u);
            statorOff = (sweep + static_cast<uint32_t>(startVal + 512)) & 0x1FFu;
        }

        // Apply pitch-invariant k-scaling (duty-cycle formant squeeze matching DSP engine)
        const uint32_t k = 16u + ((statorOff * 15u) >> 5);

        juce::Path p;
        float w = bounds.getWidth() - 8.0f;
        float h = bounds.getHeight() - 8.0f;
        float startX = bounds.getX() + 4.0f;
        float startY = bounds.getY() + 4.0f;

        const int numPts = 128;
        for (int i = 0; i < numPts; ++i) {
            uint32_t phaseIdx = static_cast<uint32_t>((i * 512) / numPts);
            uint32_t m = phaseIdx & static_cast<uint32_t>(effectiveMask);
            uint32_t tableIdx = (((m & 0x1FFu) * k) >> 4) | (m & ~0x1FFu);
            float samp = (tableIdx < 512) ? waveTable[tableIdx] : 0.0f;

            float px = startX + (static_cast<float>(i) / static_cast<float>(numPts - 1)) * w;
            float py = startY + (0.5f - samp * 0.44f) * h;

            if (i == 0) p.startNewSubPath(px, py);
            else p.lineTo(px, py);
        }

        g.setColour(Colours::kOsc.withAlpha(0.25f));
        g.strokePath(p, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(Colours::kOsc);
        g.strokePath(p, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Draw vertical stator cursor line showing sweep head position
        float cursorNorm = static_cast<float>(statorOff) / 512.0f;
        float cursorX = startX + cursorNorm * w;
        g.setColour(Colours::kOsc.withAlpha(0.45f));
        g.drawVerticalLine(static_cast<int>(cursorX), startY + 2.0f, startY + h - 2.0f);
        g.setColour(Colours::kOsc);
        g.fillEllipse(cursorX - 2.0f, startY + h - 4.0f, 4.0f, 4.0f);
    }

private:
    NyarlatProcessor& proc_;
    int oscIndex_ = 0;
    float previewPhase_ = 0.0f;
};

// ==============================================================================
// VectorAdsrVisualizer: Glowing Vector ADSR Envelope Curve Display
// ==============================================================================
class VectorAdsrVisualizer : public juce::Component {
public:
    VectorAdsrVisualizer(NyarlatProcessor& proc) : proc_(proc) {
        setInterceptsMouseClicks(false, false);
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xFF0D1117));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF262D38));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        namespace Ids = nyarlatParameters::Ids;
        float a = *proc_.getAPVTS().getRawParameterValue(Ids::amp_attack);
        float d = *proc_.getAPVTS().getRawParameterValue(Ids::amp_decay);
        float s = *proc_.getAPVTS().getRawParameterValue(Ids::amp_sustain);
        float r = *proc_.getAPVTS().getRawParameterValue(Ids::amp_release);

        float pad = 6.0f;
        float w = bounds.getWidth() - pad * 2.0f;
        float h = bounds.getHeight() - pad * 2.0f;
        float x0 = bounds.getX() + pad;
        float y0 = bounds.getBottom() - pad;

        float aW = std::clamp(a / 1.5f, 0.05f, 1.0f) * (w * 0.25f);
        float dW = std::clamp(d / 2.5f, 0.05f, 1.0f) * (w * 0.25f);
        float sW = w * 0.25f;
        float rW = std::clamp(r / 4.0f, 0.05f, 1.0f) * (w * 0.25f);

        float susY = y0 - (s * h * 0.88f);
        float peakY = y0 - (h * 0.88f);

        juce::Path curve;
        curve.startNewSubPath(x0, y0);
        curve.lineTo(x0 + aW, peakY);
        curve.lineTo(x0 + aW + dW, susY);
        curve.lineTo(x0 + aW + dW + sW, susY);
        curve.lineTo(x0 + aW + dW + sW + rW, y0);

        juce::Path fillPath(curve);
        fillPath.lineTo(x0, y0);
        fillPath.closeSubPath();

        juce::ColourGradient grad(Colours::kVca.withAlpha(0.28f), x0, peakY,
                                  Colours::kVca.withAlpha(0.02f), x0, y0, false);
        g.setGradientFill(grad);
        g.fillPath(fillPath);

        g.setColour(Colours::kVca);
        g.strokePath(curve, juce::PathStrokeType(1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

private:
    NyarlatProcessor& proc_;
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
        // Positioned strictly at bottom rim of dial track
        const float badgeY = bounds.getBottom() - badgeH - 1.0f;
        auto badgeRect = juce::Rectangle<float>(badgeX, badgeY, badgeW, badgeH);

        g.setColour(juce::Colour(0xFA090D12));
        g.fillRoundedRectangle(badgeRect, 3.5f);
        g.setColour(accentColour.withAlpha(0.65f));
        g.drawRoundedRectangle(badgeRect, 3.5f, 1.0f);

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

        g.setColour(juce::Colour(0xFF0F131A));
        g.fillRoundedRectangle(trackX - 3.0f, static_cast<float>(y), 6.0f, static_cast<float>(height), 3.0f);
        g.setColour(juce::Colour(0xFF262D38));
        g.drawRoundedRectangle(trackX - 3.0f, static_cast<float>(y), 6.0f, static_cast<float>(height), 3.0f, 1.0f);

        float bottomY = static_cast<float>(y + height);
        g.setColour(trackCol.withAlpha(0.75f));
        if (sliderPos < bottomY) {
            g.fillRoundedRectangle(trackX - 2.0f, sliderPos, 4.0f, bottomY - sliderPos, 2.0f);
        }

        float capW = std::clamp(static_cast<float>(width) - 6.0f, 26.0f, 32.0f);
        float capH = 14.0f;
        auto capBounds = juce::Rectangle<float>(trackX - capW * 0.5f, sliderPos - capH * 0.5f, capW, capH);

        g.setColour(juce::Colour(0xFF21262D));
        g.fillRoundedRectangle(capBounds, 3.0f);
        g.setColour(juce::Colour(0xFF484F58));
        g.drawRoundedRectangle(capBounds, 3.0f, 1.0f);

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

// ==============================================================================
// NyarlatEditor
// ==============================================================================
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

    struct ButtonItem {
        juce::TextButton button;
        juce::String paramId;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
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
    void setupButton(ButtonItem& item, const juce::String& paramId, const juce::String& label, juce::Colour col);

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
    std::array<std::unique_ptr<BitmaskWaveVisualizer>, 3> oscWaveViews_;
    std::unique_ptr<VectorAdsrVisualizer> vcaAdsrView_;

    // All parameter controls
    std::vector<std::unique_ptr<RotaryKnob>> knobs_;
    std::vector<std::unique_ptr<SliderFader>> faders_;
    std::vector<std::unique_ptr<ComboBoxItem>> combos_;
    std::vector<std::unique_ptr<ButtonItem>> buttons_;

    std::unordered_map<juce::Component*, const ControlInfo*> controlMap_;

    // Moving lerp tracking
    std::vector<float> smoothedRandPips_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NyarlatEditor)
};

} // namespace Nyarlat
