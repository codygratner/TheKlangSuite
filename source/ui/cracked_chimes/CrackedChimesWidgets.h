#pragma once
// CrackedChimesWidgets.h - Unified widget suite for Cracked Chimes fleet.
// Encapsulates shared interactive UI components:
// - DiscreteParamAttachment (RAII parameter binder)
// - RockerToggle (2-way tactile rocker)
// - RockerToggle3 (3-way Lifeform Triad rocker: Synthetic / Cyborg / Organic)
// - SegmentedButtonBar (Horizontal/Vertical multi-button strip with vector glyph support)
// - PulseGridBar (3x2 grid with title cell and vector glyphs)
// - Grid2x2Bar (2x2 grid bar for filter types and mojo profiles)
// - WaveGrid2x2Bar (2x2 quadrant selector with vector glyphs)
// - TactileTriggerPad (Center altar drum trigger with tactile flash)
// - AuditionMenuButton (Capsule button with vector down-chevron)
// - WaveformPickerButton (Mini-preview + name + dropdown)
// - KnobTaperModal & WaveformGridModal (Full overlay modal dialogs)
// - HasturWaveUI vector waveform geometry helpers

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "CrackedChimesColours.h"
#include "CrackedChimesModMath.h"
#include "CrackedChimesFader.h"
#include <vector>
#include <array>
#include <memory>
#include <functional>
#include <optional>
#include <algorithm>
#include <cmath>

namespace CrackedChimes::UI
{

// ==============================================================================
// Waveform Geometry & Visual Vector Helpers
// ==============================================================================
namespace WaveUI
{
    inline juce::Path createWaveformPath(int idx, juce::Rectangle<float> r, bool isLfo = false)
    {
        juce::Path p;
        const float x = r.getX();
        const float y = r.getY();
        const float w = r.getWidth();
        const float h = r.getHeight();
        const float midY = y + h * 0.5f;

        if (isLfo)
        {
            switch (idx)
            {
                case 0: // Triangle
                    p.startNewSubPath(x, midY);
                    p.lineTo(x + w * 0.25f, y + 2.0f);
                    p.lineTo(x + w * 0.75f, y + h - 2.0f);
                    p.lineTo(x + w, midY);
                    break;
                case 1: // Square
                    p.startNewSubPath(x, y + 2.0f);
                    p.lineTo(x + w * 0.5f, y + 2.0f);
                    p.lineTo(x + w * 0.5f, y + h - 2.0f);
                    p.lineTo(x + w, y + h - 2.0f);
                    break;
                case 2: // Sine
                    p.startNewSubPath(x, midY);
                    for (int s = 1; s <= 24; ++s)
                    {
                        float t = s / 24.0f;
                        p.lineTo(x + w * t, midY - std::sin(t * 6.2831853f) * (h * 0.42f));
                    }
                    break;
                case 3: // Saw (Ramp Down)
                    p.startNewSubPath(x, y + 2.0f);
                    p.lineTo(x + w * 0.96f, y + h - 2.0f);
                    p.lineTo(x + w * 0.96f, y + 2.0f);
                    p.lineTo(x + w, y + 2.0f);
                    break;
                case 4: // Random (S&H)
                    p.startNewSubPath(x, midY - 6.0f);
                    p.lineTo(x + w * 0.2f, midY - 6.0f);
                    p.lineTo(x + w * 0.2f, y + 3.0f);
                    p.lineTo(x + w * 0.45f, y + 3.0f);
                    p.lineTo(x + w * 0.45f, y + h - 4.0f);
                    p.lineTo(x + w * 0.7f, y + h - 4.0f);
                    p.lineTo(x + w * 0.7f, midY + 4.0f);
                    p.lineTo(x + w, midY + 4.0f);
                    break;
                default:
                    p.startNewSubPath(x, midY);
                    p.lineTo(x + w, midY);
                    break;
            }
            return p;
        }

        // 14 Divided Logic DCO Waves
        switch (idx)
        {
            case 0: // Pulse 1: 50% Square
                p.startNewSubPath(x, y + 3.0f);
                p.lineTo(x + w * 0.5f, y + 3.0f);
                p.lineTo(x + w * 0.5f, y + h - 3.0f);
                p.lineTo(x + w, y + h - 3.0f);
                break;
            case 1: // Pulse 2: 25% Narrow Pulse
                p.startNewSubPath(x, y + 3.0f);
                p.lineTo(x + w * 0.25f, y + 3.0f);
                p.lineTo(x + w * 0.25f, y + h - 3.0f);
                p.lineTo(x + w, y + h - 3.0f);
                break;
            case 2: // Pulse 3: PWM Pulse
                p.startNewSubPath(x, y + 3.0f);
                p.lineTo(x + w * 0.5f, y + 3.0f);
                p.lineTo(x + w * 0.5f, y + h - 3.0f);
                p.lineTo(x + w, y + h - 3.0f);
                break;
            case 3: // Saw 1: Pure Rising Saw
                p.startNewSubPath(x, y + h - 3.0f);
                p.lineTo(x + w, y + 3.0f);
                p.lineTo(x + w, y + h - 3.0f);
                break;
            case 4: // Saw 2: Chopped Saw 2x
                p.startNewSubPath(x, y + h - 3.0f);
                p.lineTo(x + w * 0.25f, y + h * 0.5f);
                p.lineTo(x + w * 0.25f, y + h - 3.0f);
                p.lineTo(x + w * 0.50f, y + h - 3.0f);
                p.lineTo(x + w * 0.75f, y + 3.0f);
                p.lineTo(x + w * 0.75f, y + h - 3.0f);
                p.lineTo(x + w, y + h - 3.0f);
                break;
            case 5: // Saw 3: PWM Saw
                p.startNewSubPath(x, y + h - 3.0f);
                p.lineTo(x + w * 0.35f, y + h * 0.4f);
                p.lineTo(x + w * 0.35f, y + h - 3.0f);
                p.lineTo(x + w * 0.50f, y + h - 3.0f);
                p.lineTo(x + w * 0.85f, y + 3.0f);
                p.lineTo(x + w * 0.85f, y + h - 3.0f);
                p.lineTo(x + w, y + h - 3.0f);
                break;
            case 6: // Saw 4: Chopped Saw 8x
                p.startNewSubPath(x, y + h - 3.0f);
                for (int i = 0; i < 4; ++i)
                {
                    float sxMid = x + w * ((i + 0.5f) * 0.25f);
                    float sx1 = x + w * ((i + 1.0f) * 0.25f);
                    p.lineTo(sxMid, y + 3.0f + (3 - i) * 5.0f);
                    p.lineTo(sxMid, y + h - 3.0f);
                    p.lineTo(sx1, y + h - 3.0f);
                }
                break;
            case 7: // Saw 5: Hoover Saw
                p.startNewSubPath(x, y + h - 3.0f);
                p.lineTo(x + w * 0.125f, y + h * 0.7f);
                p.lineTo(x + w * 0.125f, y + h - 3.0f);
                p.lineTo(x + w * 0.25f, y + h * 0.5f);
                p.lineTo(x + w * 0.25f, y + h - 3.0f);
                p.lineTo(x + w * 0.50f, y + h - 3.0f);
                p.lineTo(x + w * 0.625f, y + h * 0.3f);
                p.lineTo(x + w * 0.625f, y + h - 3.0f);
                p.lineTo(x + w * 0.75f, y + 3.0f);
                p.lineTo(x + w * 0.75f, y + h - 3.0f);
                p.lineTo(x + w, y + h - 3.0f);
                break;
            case 8: // Sub 1: Square /2
                p.startNewSubPath(x, y + 3.0f);
                p.lineTo(x + w * 0.5f, y + 3.0f);
                p.lineTo(x + w * 0.5f, y + h - 3.0f);
                p.lineTo(x + w, y + h - 3.0f);
                break;
            case 9: // Sub 2: Pulse /2
                p.startNewSubPath(x, y + 3.0f);
                p.lineTo(x + w * 0.75f, y + 3.0f);
                p.lineTo(x + w * 0.75f, y + h - 3.0f);
                p.lineTo(x + w, y + h - 3.0f);
                break;
            case 10: // Sub 3: Sub Chopped 2x
                p.startNewSubPath(x, y + 3.0f);
                p.lineTo(x + w * 0.5f, y + 3.0f);
                p.lineTo(x + w * 0.5f, y + h - 3.0f);
                p.lineTo(x + w * 0.625f, y + h - 3.0f);
                p.lineTo(x + w * 0.625f, y + 3.0f);
                p.lineTo(x + w * 0.75f, y + 3.0f);
                p.lineTo(x + w * 0.75f, y + h - 3.0f);
                p.lineTo(x + w, y + h - 3.0f);
                break;
            case 11: // Sub 4: Sub Chopped 4x
                p.startNewSubPath(x, y + 3.0f);
                p.lineTo(x + w * 0.25f, y + 3.0f);
                p.lineTo(x + w * 0.25f, y + h - 3.0f);
                p.lineTo(x + w * 0.5f, y + h - 3.0f);
                p.lineTo(x + w * 0.5f, y + 3.0f);
                p.lineTo(x + w * 0.75f, y + 3.0f);
                p.lineTo(x + w * 0.75f, y + h - 3.0f);
                p.lineTo(x + w, y + h - 3.0f);
                break;
            case 12: // Sub 5: Square /4
                p.startNewSubPath(x, y + 3.0f);
                p.lineTo(x + w * 0.25f, y + 3.0f);
                p.lineTo(x + w * 0.25f, y + h - 3.0f);
                p.lineTo(x + w, y + h - 3.0f);
                break;
            case 13: // Sub 6: Pulse /4
                p.startNewSubPath(x, y + 3.0f);
                p.lineTo(x + w * 0.125f, y + 3.0f);
                p.lineTo(x + w * 0.125f, y + h - 3.0f);
                p.lineTo(x + w, y + h - 3.0f);
                break;
            default:
                p.startNewSubPath(x, midY);
                p.lineTo(x + w, midY);
                break;
        }
        return p;
    }

    inline juce::Path createPwmExcursionPath(int idx, juce::Rectangle<float> r)
    {
        juce::Path p;
        const float x = r.getX();
        const float y = r.getY();
        const float w = r.getWidth();
        const float h = r.getHeight();

        if (idx == 2)
        {
            p.startNewSubPath(x + w * 0.5f, y + 3.0f);
            p.lineTo(x + w * 0.92f, y + 3.0f);
            p.lineTo(x + w * 0.92f, y + h - 3.0f);
        }
        else if (idx == 5)
        {
            p.startNewSubPath(x + w * 0.35f, y + h * 0.4f);
            p.lineTo(x + w * 0.15f, y + h * 0.6f);
            p.lineTo(x + w * 0.15f, y + h - 3.0f);
        }
        return p;
    }
} // namespace WaveUI

// Global compatibility alias
namespace HasturWaveUI = WaveUI;

// ==============================================================================
// CrackedChimesFaderItem: Compound Vertical Fader with Top Label & APVTS Binding
// ==============================================================================
struct CrackedChimesFaderItem
{
    CrackedChimesFader slider;
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};
using FaderItem = CrackedChimesFaderItem;

// ==============================================================================
// DiscreteParamAttachment: SOTA RAII Discrete Parameter Attachment
// ==============================================================================
class DiscreteParamAttachment
{
public:
    DiscreteParamAttachment(juce::RangedAudioParameter& param,
                            std::function<void(float)> uiUpdateCallback,
                            juce::UndoManager* undoManager = nullptr)
        : attachment_(param,
                      [cb = std::move(uiUpdateCallback)](float normVal) {
                          if (cb) cb(normVal);
                      },
                      undoManager)
    {
        attachment_.sendInitialUpdate();
    }

    void setValueAsCompleteGesture(float newValue)
    {
        attachment_.setValueAsCompleteGesture(newValue);
    }

private:
    juce::ParameterAttachment attachment_;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DiscreteParamAttachment)
};

// ==============================================================================
// RockerToggle: 2-Way Tactile Switch
// ==============================================================================
class RockerToggle : public juce::Component
{
public:
    RockerToggle(const juce::String& leftText, const juce::String& rightText)
        : leftLabel(leftText), rightLabel(rightText) {}

    void setAccentColour(juce::Colour c)
    {
        if (accentColour != c)
        {
            accentColour = c;
            repaint();
        }
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(Colours::kChassisRim);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(Colours::kBadgeBorder);
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        auto halfW = bounds.getWidth() * 0.5f;
        auto activeRect = isRight
            ? juce::Rectangle<float>(bounds.getX() + halfW, bounds.getY(), halfW, bounds.getHeight())
            : juce::Rectangle<float>(bounds.getX(), bounds.getY(), halfW, bounds.getHeight());

        g.setColour(accentColour);
        g.fillRoundedRectangle(activeRect.reduced(1.0f), 3.0f);

        g.setFont(Typography::makeFont(11.5f));
        g.setColour(isRight ? Colours::kTextMuted : Colours::kChassisBg);
        g.drawText(leftLabel, static_cast<int>(bounds.getX()), static_cast<int>(bounds.getY()),
                   static_cast<int>(halfW), static_cast<int>(bounds.getHeight()), juce::Justification::centred);

        g.setColour(isRight ? Colours::kChassisBg : Colours::kTextMuted);
        g.drawText(rightLabel, static_cast<int>(bounds.getX() + halfW), static_cast<int>(bounds.getY()),
                   static_cast<int>(halfW), static_cast<int>(bounds.getHeight()), juce::Justification::centred);
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        isRight = (e.position.x > getWidth() * 0.5f);
        if (onToggle) onToggle(isRight);
        repaint();
    }

    void setState(bool right)
    {
        if (isRight != right)
        {
            isRight = right;
            repaint();
        }
    }

    bool getState() const noexcept { return isRight; }
    std::function<void(bool)> onToggle;

private:
    juce::String leftLabel, rightLabel;
    bool isRight{false};
    juce::Colour accentColour{Colours::kSubCore};
};

// ==============================================================================
// RockerToggle3: 3-Way Lifeform Triad Switch
// ==============================================================================
class RockerToggle3 : public juce::Component
{
public:
    explicit RockerToggle3(const std::array<juce::String, 3>& labels) : labels_(labels) {}

    void setAccentColour(juce::Colour c)
    {
        segmentColours_[std::clamp(activeIndex_, 0, 2)] = c;
        repaint();
    }

    void setSegmentColour(int idx, juce::Colour c)
    {
        if (idx >= 0 && idx < 3)
        {
            segmentColours_[idx] = c;
            repaint();
        }
    }

    juce::Colour getActiveColour() const noexcept
    {
        return segmentColours_[std::clamp(activeIndex_, 0, 2)];
    }

    void setIndex(int idx, juce::NotificationType notif = juce::sendNotificationAsync)
    {
        idx = std::clamp(idx, 0, 2);
        if (activeIndex_ != idx)
        {
            activeIndex_ = idx;
            repaint();
            if (notif != juce::dontSendNotification)
            {
                if (onChange) onChange(activeIndex_);
                if (onSelect) onSelect(activeIndex_);
            }
        }
    }

    void setState(int idx) { setIndex(idx, juce::dontSendNotification); }
    int getIndex() const noexcept { return activeIndex_; }
    int getState() const noexcept { return activeIndex_; }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(Colours::kChassisRim);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(Colours::kBadgeBorder);
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        float segW = bounds.getWidth() / 3.0f;
        auto activeRect = juce::Rectangle<float>(bounds.getX() + activeIndex_ * segW, bounds.getY(), segW, bounds.getHeight());

        g.setColour(segmentColours_[std::clamp(activeIndex_, 0, 2)]);
        g.fillRoundedRectangle(activeRect.reduced(1.0f), 3.0f);

        g.setFont(Typography::makeFont(11.0f));
        for (int i = 0; i < 3; ++i)
        {
            g.setColour((i == activeIndex_) ? Colours::kChassisBg : Colours::kTextMuted);
            g.drawFittedText(labels_[i],
                             juce::Rectangle<float>(bounds.getX() + i * segW, bounds.getY(), segW, bounds.getHeight()).toNearestInt(),
                             juce::Justification::centred, 1, 0.85f);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        float segW = static_cast<float>(getWidth()) / 3.0f;
        int idx = std::clamp(static_cast<int>(e.position.x / segW), 0, 2);
        setIndex(idx, juce::sendNotificationAsync);
    }

    std::function<void(int)> onChange;
    std::function<void(int)> onSelect;

private:
    std::array<juce::String, 3> labels_;
    int activeIndex_{0};
    std::array<juce::Colour, 3> segmentColours_{
        Colours::kSynthetic, // Tron Blue
        Colours::kCyborg,    // Construction Equipment Yellow
        Colours::kOrganic    // Warm Green
    };
};

// Aliases for per-plugin compatibility
using FormlessRocker3 = RockerToggle3;
using DigiStrikeRocker3 = RockerToggle3;

// ==============================================================================
// SegmentedButtonBar: Multi-Segment Button Strip (Horizontal or Vertical)
// ==============================================================================
class SegmentedButtonBar : public juce::Component
{
public:
    using GlyphFn = std::function<juce::Path(juce::Rectangle<float>)>;

    explicit SegmentedButtonBar(const std::vector<juce::String>& labels)
        : labels_(labels) {}

    void setVertical(bool isVertical)
    {
        if (isVertical_ != isVertical)
        {
            isVertical_ = isVertical;
            repaint();
        }
    }

    void setGlyphProviders(std::vector<GlyphFn> glyphs)
    {
        glyphProviders_ = std::move(glyphs);
        repaint();
    }

    void setAccentColour(juce::Colour c)
    {
        if (accentColour_ != c)
        {
            accentColour_ = c;
            repaint();
        }
    }

    void setSegmentColour(int idx, juce::Colour c)
    {
        if (idx < 0) return;
        if (segmentColours_.size() <= static_cast<size_t>(idx))
            segmentColours_.resize(static_cast<size_t>(idx) + 1);
        segmentColours_[static_cast<size_t>(idx)] = c;
        if (idx == activeIndex_) repaint();
    }

    juce::Colour getActiveColour() const noexcept
    {
        if (activeIndex_ >= 0 && static_cast<size_t>(activeIndex_) < segmentColours_.size()
            && segmentColours_[static_cast<size_t>(activeIndex_)].has_value())
            return *segmentColours_[static_cast<size_t>(activeIndex_)];
        return accentColour_;
    }

    void setState(int idx)
    {
        idx = std::clamp(idx, 0, static_cast<int>(labels_.size()) - 1);
        if (activeIndex_ != idx)
        {
            activeIndex_ = idx;
            repaint();
        }
    }

    int getState() const noexcept { return activeIndex_; }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.isEmpty() || labels_.empty()) return;

        g.setColour(Colours::kChassisRim);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(Colours::kBadgeBorder);
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        const int numSegs = static_cast<int>(labels_.size());

        if (isVertical_)
        {
            const float segH = bounds.getHeight() / static_cast<float>(numSegs);
            auto activeRect = juce::Rectangle<float>(bounds.getX(), bounds.getY() + activeIndex_ * segH, bounds.getWidth(), segH);
            g.setColour(getActiveColour());
            g.fillRoundedRectangle(activeRect.reduced(1.0f), 3.0f);

            g.setColour(Colours::kCavity);
            for (int i = 1; i < numSegs; ++i)
            {
                if (i != activeIndex_ && i != activeIndex_ + 1)
                {
                    float divY = bounds.getY() + i * segH;
                    g.drawHorizontalLine(static_cast<int>(std::round(divY)), bounds.getX() + 3.0f, bounds.getRight() - 3.0f);
                }
            }

            g.setFont(Typography::makeFont(11.5f));
            for (int i = 0; i < numSegs; ++i)
            {
                bool isActive = (i == activeIndex_);
                auto segRect = juce::Rectangle<float>(bounds.getX(), bounds.getY() + i * segH, bounds.getWidth(), segH);
                if (i < static_cast<int>(glyphProviders_.size()) && glyphProviders_[i] != nullptr)
                {
                    auto glyphBounds = segRect.reduced(bounds.getWidth() * 0.18f, segH * 0.22f);
                    auto glyphPath = glyphProviders_[i](glyphBounds);
                    g.setColour(isActive ? Colours::kChassisBg : Colours::kTextMuted);
                    g.strokePath(glyphPath, juce::PathStrokeType(isActive ? 2.0f : 1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                }
                else
                {
                    g.setColour(isActive ? Colours::kChassisBg : Colours::kText);
                    g.drawFittedText(labels_[i], segRect.toNearestInt(), juce::Justification::centred, 1);
                }
            }
        }
        else
        {
            const float segW = bounds.getWidth() / static_cast<float>(numSegs);
            auto activeRect = juce::Rectangle<float>(bounds.getX() + activeIndex_ * segW, bounds.getY(), segW, bounds.getHeight());
            g.setColour(getActiveColour());
            g.fillRoundedRectangle(activeRect.reduced(1.0f), 3.0f);

            g.setColour(Colours::kCavity);
            for (int i = 1; i < numSegs; ++i)
            {
                if (i != activeIndex_ && i != activeIndex_ + 1)
                {
                    float divX = bounds.getX() + i * segW;
                    g.drawVerticalLine(static_cast<int>(std::round(divX)), bounds.getY() + 3.0f, bounds.getBottom() - 3.0f);
                }
            }

            g.setFont(Typography::makeFont(11.5f));
            for (int i = 0; i < numSegs; ++i)
            {
                bool isActive = (i == activeIndex_);
                auto segRect = juce::Rectangle<float>(bounds.getX() + i * segW, bounds.getY(), segW, bounds.getHeight());
                if (i < static_cast<int>(glyphProviders_.size()) && glyphProviders_[i] != nullptr)
                {
                    auto glyphBounds = segRect.reduced(segW * 0.18f, bounds.getHeight() * 0.22f);
                    auto glyphPath = glyphProviders_[i](glyphBounds);
                    g.setColour(isActive ? Colours::kChassisBg : Colours::kTextMuted);
                    g.strokePath(glyphPath, juce::PathStrokeType(isActive ? 2.0f : 1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                }
                else
                {
                    g.setColour(isActive ? Colours::kChassisBg : Colours::kText);
                    g.drawFittedText(labels_[i], segRect.toNearestInt(), juce::Justification::centred, 1);
                }
            }
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (labels_.empty()) return;
        const int numSegs = static_cast<int>(labels_.size());
        int idx = 0;
        if (isVertical_)
        {
            const float segH = static_cast<float>(getHeight()) / static_cast<float>(numSegs);
            idx = std::clamp(static_cast<int>(e.position.y / segH), 0, numSegs - 1);
        }
        else
        {
            const float segW = static_cast<float>(getWidth()) / static_cast<float>(numSegs);
            idx = std::clamp(static_cast<int>(e.position.x / segW), 0, numSegs - 1);
        }
        if (idx != activeIndex_)
        {
            activeIndex_ = idx;
            if (onSelect) onSelect(activeIndex_);
            repaint();
        }
    }

    std::function<void(int)> onSelect;

private:
    std::vector<juce::String> labels_;
    std::vector<GlyphFn> glyphProviders_;
    std::vector<std::optional<juce::Colour>> segmentColours_;
    int activeIndex_{0};
    juce::Colour accentColour_{Colours::kSubCore};
    bool isVertical_{false};
};

using FormlessSegmentedBar = SegmentedButtonBar;
using DigiStrikeSegmentedBar = SegmentedButtonBar;

// ==============================================================================
// PulseGridBar: 3x2 Grid Bar with Title Cell at (0, 0)
// ==============================================================================
class PulseGridBar : public juce::Component
{
public:
    using GlyphFn = std::function<juce::Path(juce::Rectangle<float>)>;

    PulseGridBar(const juce::String& title, const std::vector<juce::String>& optionLabels)
        : title_(title), labels_(optionLabels) {}

    void setGlyphProviders(std::vector<GlyphFn> glyphs)
    {
        glyphProviders_ = std::move(glyphs);
        repaint();
    }

    void setAccentColour(juce::Colour c)
    {
        if (accentColour_ != c)
        {
            accentColour_ = c;
            repaint();
        }
    }

    void setState(int idx)
    {
        idx = std::clamp(idx, 0, 4);
        if (activeIndex_ != idx)
        {
            activeIndex_ = idx;
            repaint();
        }
    }

    int getState() const noexcept { return activeIndex_; }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.isEmpty()) return;

        g.setColour(Colours::kChassisRim);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(Colours::kBadgeBorder);
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        const float colW = bounds.getWidth() / 3.0f;
        const float rowH = bounds.getHeight() / 2.0f;

        // Title Cell (0, 0)
        auto titleRect = juce::Rectangle<float>(bounds.getX(), bounds.getY(), colW, rowH);
        g.setColour(Colours::kChassisBg);
        g.fillRoundedRectangle(titleRect.reduced(1.0f), 3.0f);
        g.setFont(Typography::makeFont(11.0f));
        g.setColour(accentColour_);
        g.drawText(title_, titleRect, juce::Justification::centred);

        // Dividers
        g.setColour(Colours::kCavity);
        g.drawHorizontalLine(static_cast<int>(std::round(bounds.getY() + rowH)), bounds.getX() + 3.0f, bounds.getRight() - 3.0f);
        g.drawVerticalLine(static_cast<int>(std::round(bounds.getX() + colW)), bounds.getY() + 3.0f, bounds.getBottom() - 3.0f);
        g.drawVerticalLine(static_cast<int>(std::round(bounds.getX() + colW * 2.0f)), bounds.getY() + 3.0f, bounds.getBottom() - 3.0f);

        static const int gridPos[5][2] = { {0, 1}, {0, 2}, {1, 0}, {1, 1}, {1, 2} };

        g.setFont(Typography::makeFont(11.5f));
        for (int i = 0; i < 5; ++i)
        {
            int r = gridPos[i][0];
            int c = gridPos[i][1];
            auto cellRect = juce::Rectangle<float>(bounds.getX() + c * colW, bounds.getY() + r * rowH, colW, rowH);
            bool isActive = (i == activeIndex_);

            if (isActive)
            {
                g.setColour(accentColour_);
                g.fillRoundedRectangle(cellRect.reduced(1.5f), 3.0f);
            }

            if (i < static_cast<int>(glyphProviders_.size()) && glyphProviders_[i] != nullptr)
            {
                auto glyphBounds = cellRect.reduced(colW * 0.16f, rowH * 0.20f);
                auto glyphPath = glyphProviders_[i](glyphBounds);
                g.setColour(isActive ? Colours::kChassisBg : Colours::kTextMuted);
                g.strokePath(glyphPath, juce::PathStrokeType(isActive ? 2.0f : 1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
            else if (i < static_cast<int>(labels_.size()))
            {
                g.setColour(isActive ? Colours::kChassisBg : Colours::kText);
                g.drawText(labels_[i], cellRect, juce::Justification::centred);
            }
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        const float colW = static_cast<float>(getWidth()) / 3.0f;
        const float rowH = static_cast<float>(getHeight()) / 2.0f;
        int c = std::clamp(static_cast<int>(e.position.x / colW), 0, 2);
        int r = std::clamp(static_cast<int>(e.position.y / rowH), 0, 1);

        int clickedIdx = -1;
        if (r == 0 && c == 1) clickedIdx = 0;
        else if (r == 0 && c == 2) clickedIdx = 1;
        else if (r == 1 && c == 0) clickedIdx = 2;
        else if (r == 1 && c == 1) clickedIdx = 3;
        else if (r == 1 && c == 2) clickedIdx = 4;

        if (clickedIdx >= 0 && clickedIdx != activeIndex_)
        {
            activeIndex_ = clickedIdx;
            if (onSelect) onSelect(activeIndex_);
            repaint();
        }
    }

    std::function<void(int)> onSelect;

private:
    juce::String title_;
    std::vector<juce::String> labels_;
    std::vector<GlyphFn> glyphProviders_;
    int activeIndex_{0};
    juce::Colour accentColour_{Colours::kSubCore};
};

// ==============================================================================
// Grid2x2Bar: 2x2 Grid Selector (With or without title cell)
// ==============================================================================
class Grid2x2Bar : public juce::Component
{
public:
    Grid2x2Bar(const juce::String& title, const std::vector<juce::String>& optionLabels)
        : title_(title), labels_(optionLabels), hasTitle_(true) {}

    explicit Grid2x2Bar(const std::array<juce::String, 4>& labels)
        : labels_(labels.begin(), labels.end()), hasTitle_(false) {}

    void setAccentColour(juce::Colour c)
    {
        if (accentColour_ != c)
        {
            accentColour_ = c;
            repaint();
        }
    }

    void setItemColours(const std::vector<juce::Colour>& c)
    {
        itemColours_ = c;
        repaint();
    }

    void setState(int idx)
    {
        int maxIdx = hasTitle_ ? static_cast<int>(labels_.size()) - 1 : 3;
        idx = std::clamp(idx, -1, maxIdx);
        if (activeIndex_ != idx)
        {
            activeIndex_ = idx;
            repaint();
        }
    }

    int getState() const noexcept { return activeIndex_; }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.isEmpty()) return;

        g.setColour(Colours::kChassisRim);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(Colours::kBadgeBorder);
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        const float colW = bounds.getWidth() * 0.5f;
        const float rowH = bounds.getHeight() * 0.5f;

        g.setColour(Colours::kCavity);
        g.drawHorizontalLine(static_cast<int>(std::round(bounds.getY() + rowH)), bounds.getX() + 3.0f, bounds.getRight() - 3.0f);
        g.drawVerticalLine(static_cast<int>(std::round(bounds.getX() + colW)), bounds.getY() + 3.0f, bounds.getBottom() - 3.0f);

        if (hasTitle_)
        {
            // Title Cell (0, 0)
            auto titleRect = juce::Rectangle<float>(bounds.getX(), bounds.getY(), colW, rowH);
            g.setColour(Colours::kChassisBg);
            g.fillRoundedRectangle(titleRect.reduced(1.0f), 3.0f);
            g.setFont(Typography::makeFont(11.0f));
            g.setColour(accentColour_);
            g.drawText(title_, titleRect, juce::Justification::centred);

            static const int gridPos[3][2] = { {0, 1}, {1, 0}, {1, 1} };
            g.setFont(Typography::makeFont(10.5f));
            for (int i = 0; i < 3 && i < static_cast<int>(labels_.size()); ++i)
            {
                int r = gridPos[i][0];
                int c = gridPos[i][1];
                auto cellRect = juce::Rectangle<float>(bounds.getX() + c * colW, bounds.getY() + r * rowH, colW, rowH);
                bool isActive = (i == activeIndex_);
                juce::Colour cellCol = (i < static_cast<int>(itemColours_.size())) ? itemColours_[i] : accentColour_;

                if (isActive)
                {
                    g.setColour(cellCol);
                    g.fillRoundedRectangle(cellRect.reduced(1.5f), 3.0f);
                    g.setColour(Colours::kChassisBg);
                }
                else
                {
                    g.setColour(Colours::kBadgeBg);
                    g.fillRoundedRectangle(cellRect.reduced(1.5f), 3.0f);
                    g.setColour(cellCol.withAlpha(0.75f));
                }
                g.drawText(labels_[i], cellRect, juce::Justification::centred);
            }
        }
        else
        {
            // 4 options in 2x2
            g.setFont(Typography::makeFont(10.5f));
            for (int i = 0; i < 4 && i < static_cast<int>(labels_.size()); ++i)
            {
                int r = i / 2;
                int c = i % 2;
                auto cellRect = juce::Rectangle<float>(bounds.getX() + c * colW, bounds.getY() + r * rowH, colW, rowH);
                bool isActive = (i == activeIndex_);
                juce::Colour cellCol = (i < static_cast<int>(itemColours_.size())) ? itemColours_[i] : accentColour_;

                if (isActive)
                {
                    g.setColour(cellCol);
                    g.fillRoundedRectangle(cellRect.reduced(1.0f), 3.0f);
                    g.setColour(Colours::kChassisBg);
                }
                else
                {
                    g.setColour(Colours::kText);
                }
                g.drawFittedText(labels_[i], cellRect.toNearestInt(), juce::Justification::centred, 1, 0.75f);
            }
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        const float colW = static_cast<float>(getWidth()) * 0.5f;
        const float rowH = static_cast<float>(getHeight()) * 0.5f;
        int c = std::clamp(static_cast<int>(e.position.x / colW), 0, 1);
        int r = std::clamp(static_cast<int>(e.position.y / rowH), 0, 1);

        if (hasTitle_)
        {
            int clickedIdx = -1;
            if (r == 0 && c == 1) clickedIdx = 0;
            else if (r == 1 && c == 0) clickedIdx = 1;
            else if (r == 1 && c == 1) clickedIdx = 2;

            if (clickedIdx >= 0 && clickedIdx != activeIndex_)
            {
                activeIndex_ = clickedIdx;
                if (onSelect) onSelect(activeIndex_);
                repaint();
            }
        }
        else
        {
            int idx = r * 2 + c;
            if (activeIndex_ == idx)
            {
                setState(-1);
                if (onSelect) onSelect(-1);
            }
            else
            {
                setState(idx);
                if (onSelect) onSelect(idx);
            }
        }
    }

    std::function<void(int)> onSelect;

private:
    juce::String title_;
    std::vector<juce::String> labels_;
    std::vector<juce::Colour> itemColours_;
    int activeIndex_{0};
    juce::Colour accentColour_{Colours::kFilterCore};
    bool hasTitle_{true};
};

using DigiStrikeGrid2x2 = Grid2x2Bar;
using FormlessGrid2x2 = Grid2x2Bar;

// ==============================================================================
// WaveGrid2x2Bar: 2x2 Quadrant Grid with Vector Wave Glyphs
// ==============================================================================
class WaveGrid2x2Bar : public juce::Component
{
public:
    using GlyphFn = std::function<juce::Path(juce::Rectangle<float>)>;

    explicit WaveGrid2x2Bar(const std::vector<juce::String>& labels)
        : labels_(labels) {}

    void setGlyphProviders(std::vector<GlyphFn> glyphs)
    {
        glyphProviders_ = std::move(glyphs);
        repaint();
    }

    void setAccentColour(juce::Colour c)
    {
        if (accentColour_ != c)
        {
            accentColour_ = c;
            repaint();
        }
    }

    void setState(int idx)
    {
        idx = std::clamp(idx, 0, static_cast<int>(labels_.size()) - 1);
        if (activeIndex_ != idx)
        {
            activeIndex_ = idx;
            repaint();
        }
    }

    int getState() const noexcept { return activeIndex_; }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.isEmpty() || labels_.empty()) return;

        g.setColour(Colours::kChassisRim);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(Colours::kBadgeBorder);
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        const float colW = bounds.getWidth() * 0.5f;
        const float rowH = bounds.getHeight() * 0.5f;

        g.setColour(Colours::kCavity);
        g.drawHorizontalLine(static_cast<int>(std::round(bounds.getY() + rowH)), bounds.getX() + 3.0f, bounds.getRight() - 3.0f);
        g.drawVerticalLine(static_cast<int>(std::round(bounds.getX() + colW)), bounds.getY() + 3.0f, bounds.getBottom() - 3.0f);

        g.setFont(Typography::makeFont(10.5f));
        for (int i = 0; i < 4 && i < static_cast<int>(labels_.size()); ++i)
        {
            int r = i / 2;
            int c = i % 2;
            auto cellRect = juce::Rectangle<float>(bounds.getX() + c * colW, bounds.getY() + r * rowH, colW, rowH);
            bool isActive = (i == activeIndex_);

            if (isActive)
            {
                g.setColour(accentColour_);
                g.fillRoundedRectangle(cellRect.reduced(1.5f), 3.0f);
            }

            if (i < static_cast<int>(glyphProviders_.size()) && glyphProviders_[i] != nullptr)
            {
                auto glyphBounds = cellRect.reduced(colW * 0.18f, rowH * 0.20f);
                auto glyphPath = glyphProviders_[i](glyphBounds);
                g.setColour(isActive ? Colours::kChassisBg : Colours::kTextMuted);
                g.strokePath(glyphPath, juce::PathStrokeType(isActive ? 2.0f : 1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
            else
            {
                g.setColour(isActive ? Colours::kChassisBg : Colours::kText);
                g.drawText(labels_[i], cellRect, juce::Justification::centred);
            }
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        const float colW = static_cast<float>(getWidth()) * 0.5f;
        const float rowH = static_cast<float>(getHeight()) * 0.5f;
        int c = std::clamp(static_cast<int>(e.position.x / colW), 0, 1);
        int r = std::clamp(static_cast<int>(e.position.y / rowH), 0, 1);
        int clickedIdx = r * 2 + c;
        if (clickedIdx >= 0 && clickedIdx < static_cast<int>(labels_.size()) && clickedIdx != activeIndex_)
        {
            activeIndex_ = clickedIdx;
            if (onSelect) onSelect(activeIndex_);
            repaint();
        }
    }

    std::function<void(int)> onSelect;

private:
    std::vector<juce::String> labels_;
    std::vector<GlyphFn> glyphProviders_;
    int activeIndex_{0};
    juce::Colour accentColour_{Colours::kSubCore};
};

// ==============================================================================
// TactileTriggerPad: Center Altar Drum Trigger with Tactile Gold/Green Flash
// ==============================================================================
class TactileTriggerPad : public juce::Component
{
public:
    TactileTriggerPad() = default;

    std::function<void()> onTrigger;
    std::function<void()> onRelease;
    std::function<void()> onRightTrigger;
    std::function<void()> onRightRelease;

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        if (isDown_)
        {
            g.setColour(isRightClick_ ? juce::Colour(0xFF00E676) : Colours::kSubCore);
            g.fillRoundedRectangle(bounds, 5.0f);
            g.setFont(Typography::makeFont(13.0f));
            g.setColour(Colours::kChassisBg);
            g.drawText(isRightClick_ ? "C3" : "TRIG", bounds, juce::Justification::centred);
        }
        else
        {
            g.setColour(Colours::kChassisRim);
            g.fillRoundedRectangle(bounds, 5.0f);
            g.setColour(Colours::kBadgeBorder);
            g.drawRoundedRectangle(bounds, 5.0f, 1.2f);
            g.setFont(Typography::makeFont(13.0f));
            g.setColour(Colours::kText);
            g.drawText("TRIG", bounds, juce::Justification::centred);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        isDown_ = true;
        isRightClick_ = e.mods.isRightButtonDown();
        repaint();
        if (isRightClick_)
        {
            if (onRightTrigger) onRightTrigger();
        }
        else
        {
            if (onTrigger) onTrigger();
        }
    }

    void mouseUp(const juce::MouseEvent&) override
    {
        if (isDown_)
        {
            isDown_ = false;
            repaint();
            if (isRightClick_)
            {
                if (onRightRelease) onRightRelease();
            }
            else
            {
                if (onRelease) onRelease();
            }
            isRightClick_ = false;
        }
    }

    void mouseExit(const juce::MouseEvent&) override
    {
        if (isDown_)
        {
            isDown_ = false;
            repaint();
            if (isRightClick_)
            {
                if (onRightRelease) onRightRelease();
            }
            else
            {
                if (onRelease) onRelease();
            }
            isRightClick_ = false;
        }
    }

private:
    bool isDown_{false};
    bool isRightClick_{false};
};

using DigiStrikeTriggerPad = TactileTriggerPad;

// ==============================================================================
// AuditionMenuButton: Capsule Button with Vector Down-Chevron
// ==============================================================================
class AuditionMenuButton : public juce::TextButton
{
public:
    void setAccentColour(juce::Colour c) { accentCol_ = c; repaint(); }

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.isEmpty()) return;

        juce::Colour bg = shouldDrawButtonAsDown ? juce::Colour(0xFF262D38)
                        : (shouldDrawButtonAsHighlighted ? juce::Colour(0xFF21262D) : juce::Colour(0xFF161B22));
        g.setColour(bg);
        g.fillRoundedRectangle(bounds, 4.0f);

        g.setColour(Colours::kBadgeBorder);
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        const float chevronX = bounds.getRight() - 12.0f;
        const float chevronY = bounds.getCentreY();
        juce::Path p;
        p.startNewSubPath(chevronX - 3.5f, chevronY - 2.0f);
        p.lineTo(chevronX, chevronY + 2.0f);
        p.lineTo(chevronX + 3.5f, chevronY - 2.0f);
        g.setColour(accentCol_.withAlpha(0.85f));
        g.strokePath(p, juce::PathStrokeType(1.5f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));

        auto textRect = bounds.withTrimmedRight(18.0f).withTrimmedLeft(6.0f);
        g.setColour(shouldDrawButtonAsHighlighted ? Colours::kText : accentCol_.withAlpha(0.95f));
        g.setFont(Typography::makeFont(11.5f));
        g.drawFittedText(getButtonText(), textRect.toNearestInt(), juce::Justification::centred, 1);
    }

private:
    juce::Colour accentCol_{Colours::kOscCore};
};

// ==============================================================================
// WaveformPickerButton: Waveform Capsule Button with Mini Vector Preview
// ==============================================================================
class WaveformPickerButton : public juce::Component
{
public:
    int waveIndex{0};
    bool isLfo{false};
    juce::String waveName;
    std::function<void()> onClick;
    juce::Colour accentColour{Colours::kSubCore};

    void setWaveIndex(int idx)
    {
        waveIndex = idx;
        repaint();
    }

    void setAccentColour(juce::Colour c)
    {
        accentColour = c;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(Colours::kChassisRim);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(Colours::kBadgeBorder);
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        auto previewArea = bounds.removeFromLeft(bounds.getHeight()).reduced(4.0f);
        auto path = WaveUI::createWaveformPath(waveIndex, previewArea, isLfo);
        g.setColour(accentColour);
        g.strokePath(path, juce::PathStrokeType(1.8f));

        g.setFont(Typography::makeFont(11.5f));
        g.setColour(Colours::kText);
        g.drawText(waveName, static_cast<int>(bounds.getX() + 6.0f), 0,
                   static_cast<int>(bounds.getWidth() - 24.0f), static_cast<int>(bounds.getHeight()),
                   juce::Justification::centredLeft);

        g.setColour(Colours::kTextMuted);
        float arrX = bounds.getRight() - 14.0f;
        float arrY = bounds.getCentreY() - 2.0f;
        juce::Path arr;
        arr.startNewSubPath(arrX, arrY);
        arr.lineTo(arrX + 8.0f, arrY);
        arr.lineTo(arrX + 4.0f, arrY + 5.0f);
        arr.closeSubPath();
        g.fillPath(arr);
    }

    void mouseDown(const juce::MouseEvent&) override
    {
        if (onClick) onClick();
    }
};

// ==============================================================================
// KnobTaperModal: Right-Click Knob Range & Curve Modal Overlay
// ==============================================================================
class KnobTaperModal : public juce::Component
{
public:
    KnobTaperModal(juce::Slider& targetSlider, const juce::String& paramName, juce::Colour accent)
        : slider_(targetSlider), paramName_(paramName), accent_(accent)
    {
        setAlwaysOnTop(true);

        titleLabel.setText("CALIBRATE KNOB TAPER: " + paramName, juce::dontSendNotification);
        titleLabel.setFont(Typography::makeFont(13.5f));
        titleLabel.setColour(juce::Label::textColourId, accent);
        titleLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(titleLabel);

        auto setupRow = [this](juce::Label& lbl, const juce::String& txt, juce::TextEditor& ed) {
            lbl.setText(txt, juce::dontSendNotification);
            lbl.setFont(Typography::makeFont(11.5f));
            lbl.setColour(juce::Label::textColourId, Colours::kTextMuted);
            addAndMakeVisible(lbl);

            ed.setColour(juce::TextEditor::backgroundColourId, Colours::kBadgeBg);
            ed.setColour(juce::TextEditor::textColourId, Colours::kText);
            ed.setColour(juce::TextEditor::outlineColourId, Colours::kBadgeBorder);
            addAndMakeVisible(ed);
        };

        setupRow(startLabel, "MINIMUM (0.0):", startEditor);
        setupRow(midLabel,   "CENTER (0.5):",  midEditor);
        setupRow(endLabel,   "MAXIMUM (1.0):", endEditor);

        startEditor.setText(juce::String(slider_.getMinimum()));
        endEditor.setText(juce::String(slider_.getMaximum()));
        midEditor.setText(juce::String((slider_.getMinimum() + slider_.getMaximum()) * 0.5));

        curveLabel.setText("RESPONSE CURVE:", juce::dontSendNotification);
        curveLabel.setFont(Typography::makeFont(11.5f));
        curveLabel.setColour(juce::Label::textColourId, Colours::kTextMuted);
        addAndMakeVisible(curveLabel);

        curveCombo.addItem("Linear", 1);
        curveCombo.addItem("Audio Logarithmic", 2);
        curveCombo.addItem("Pitch S-Curve", 3);
        curveCombo.setSelectedId(1);
        addAndMakeVisible(curveCombo);

        applyButton.setButtonText("APPLY");
        applyButton.onClick = [this]() {
            double mn = startEditor.getText().getDoubleValue();
            double mx = endEditor.getText().getDoubleValue();
            if (mx > mn) {
                slider_.setRange(mn, mx);
            }
            if (onDismiss) onDismiss();
        };
        addAndMakeVisible(applyButton);

        cancelButton.setButtonText("CANCEL");
        cancelButton.onClick = [this]() {
            if (onDismiss) onDismiss();
        };
        addAndMakeVisible(cancelButton);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xB8080B10));

        auto dBox = juce::Rectangle<float>(getWidth() * 0.5f - 170.0f, getHeight() * 0.5f - 130.0f, 340.0f, 260.0f);
        g.setColour(Colours::kChassisRim);
        g.fillRoundedRectangle(dBox, 8.0f);
        g.setColour(accent_);
        g.drawRoundedRectangle(dBox, 8.0f, 1.5f);
    }

    void resized() override
    {
        int bx = getWidth() / 2 - 170;
        int by = getHeight() / 2 - 130;

        titleLabel.setBounds(bx + 10, by + 12, 320, 24);

        int rowY = by + 46;
        int rowH = 24;
        int sp = 34;

        startLabel.setBounds(bx + 20, rowY, 120, rowH);
        startEditor.setBounds(bx + 150, rowY, 160, rowH);

        rowY += sp;
        midLabel.setBounds(bx + 20, rowY, 120, rowH);
        midEditor.setBounds(bx + 150, rowY, 160, rowH);

        rowY += sp;
        endLabel.setBounds(bx + 20, rowY, 120, rowH);
        endEditor.setBounds(bx + 150, rowY, 160, rowH);

        rowY += sp;
        curveLabel.setBounds(bx + 20, rowY, 120, rowH);
        curveCombo.setBounds(bx + 150, rowY, 160, rowH);

        applyButton.setBounds(bx + 40, by + 214, 110, 28);
        cancelButton.setBounds(bx + 190, by + 214, 110, 28);
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        auto dBox = juce::Rectangle<int>(getWidth() / 2 - 170, getHeight() / 2 - 130, 340, 260);
        if (!dBox.contains(e.getPosition())) {
            if (onDismiss) onDismiss();
        }
    }

    std::function<void()> onDismiss;

private:
    juce::Slider& slider_;
    juce::String paramName_;
    juce::Colour accent_;

    juce::Label titleLabel;
    juce::Label startLabel, midLabel, endLabel, curveLabel;
    juce::TextEditor startEditor, midEditor, endEditor;
    juce::ComboBox curveCombo;
    juce::TextButton applyButton, cancelButton;
};

// ==============================================================================
// WaveformGridModal: Full Waveform Matrix Selection Overlay
// ==============================================================================
class WaveformGridModal : public juce::Component
{
public:
    WaveformGridModal(bool isLfo, int currentSelection, juce::Colour accent,
                      std::function<void(int)> onSelectWave)
        : isLfo_(isLfo), currentSelection_(currentSelection), accent_(accent), onSelectWave_(std::move(onSelectWave))
    {
        setAlwaysOnTop(true);

        titleLabel.setText(isLfo ? "SELECT LFO WAVEFORM" : "SELECT OSCILLATOR WAVEFORM", juce::dontSendNotification);
        titleLabel.setFont(Typography::makeFont(14.5f));
        titleLabel.setColour(juce::Label::textColourId, accent);
        titleLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(titleLabel);

        closeButton.setButtonText(juce::CharPointer_UTF8("\xE2\x9C\x95"));
        closeButton.onClick = [this]() {
            if (onDismiss) onDismiss();
        };
        addAndMakeVisible(closeButton);
    }

    void resized() override
    {
        int w = isLfo_ ? 560 : 760;
        int h = isLfo_ ? 180 : 430;
        dialogBounds_ = juce::Rectangle<int>(getWidth() / 2 - w / 2, getHeight() / 2 - h / 2, w, h);

        titleLabel.setBounds(dialogBounds_.getX() + 20, dialogBounds_.getY() + 10, dialogBounds_.getWidth() - 60, 24);
        closeButton.setBounds(dialogBounds_.getRight() - 32, dialogBounds_.getY() + 8, 24, 24);
    }

    void mouseMove(const juce::MouseEvent& e) override
    {
        int idx = getTileIndexAt(e.getPosition());
        if (idx != hoveredIdx_)
        {
            hoveredIdx_ = idx;
            repaint();
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (!dialogBounds_.contains(e.getPosition()))
        {
            if (onDismiss) onDismiss();
            return;
        }

        int idx = getTileIndexAt(e.getPosition());
        if (idx >= 0)
        {
            if (onSelectWave_) onSelectWave_(idx);
            if (onDismiss) onDismiss();
        }
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colour(0xB8080B10));
        g.fillRect(getLocalBounds());

        g.setColour(Colours::kChassisRim);
        g.fillRoundedRectangle(dialogBounds_.toFloat(), 8.0f);
        g.setColour(accent_);
        g.drawRoundedRectangle(dialogBounds_.toFloat(), 8.0f, 1.5f);

        static const juce::StringArray kAlphaWaves{
            "Pulse 1 (Square)", "Pulse 2 (Pulse)", "Pulse 3 (PWM Pulse)",
            "Saw 1 (Saw)", "Saw 2 (Chopped 2x)", "Saw 3 (PWM Saw)", "Saw 4 (Chopped 4x)", "Saw 5 (Hoover)",
            "Sub 1 (Sq /2)", "Sub 2 (Pls /2)", "Sub 3 (Sub 2x)", "Sub 4 (Sub 4x)", "Sub 5 (Sq /4)", "Sub 6 (Pls /4)"
        };

        static const juce::StringArray kLfoWaves{
            "Triangle", "Square", "Sine", "Saw", "Random (S&H)"
        };

        int cols = 5;
        int rows = isLfo_ ? 1 : 3;
        int total = isLfo_ ? 5 : 14;

        auto gridArea = dialogBounds_.reduced(16).withTrimmedTop(28);
        float tileW = gridArea.getWidth() / static_cast<float>(cols);
        float tileH = gridArea.getHeight() / static_cast<float>(rows);

        for (int i = 0; i < total; ++i)
        {
            int r = i / cols;
            int c = i % cols;
            auto tileRect = juce::Rectangle<float>(gridArea.getX() + c * tileW, gridArea.getY() + r * tileH, tileW, tileH).reduced(4.0f);

            bool isSelected = (i == currentSelection_);
            bool isHovered = (i == hoveredIdx_);

            g.setColour(isSelected ? juce::Colour(0xFF21262D) : (isHovered ? juce::Colour(0xFF1B2028) : Colours::kBadgeBg));
            g.fillRoundedRectangle(tileRect, 4.0f);

            if (isSelected)
            {
                g.setColour(accent_);
                g.drawRoundedRectangle(tileRect, 4.0f, 2.0f);
            }
            else if (isHovered)
            {
                g.setColour(accent_.withAlpha(0.6f));
                g.drawRoundedRectangle(tileRect, 4.0f, 1.0f);
            }
            else
            {
                g.setColour(Colours::kBadgeBorder);
                g.drawRoundedRectangle(tileRect, 4.0f, 1.0f);
            }

            auto waveCanvas = tileRect.reduced(6.0f).withTrimmedBottom(22.0f);
            auto path = WaveUI::createWaveformPath(i, waveCanvas, isLfo_);
            g.setColour(isSelected ? accent_ : Colours::kTextMuted);
            g.strokePath(path, juce::PathStrokeType(1.8f));

            if (!isLfo_)
            {
                auto pwmPath = WaveUI::createPwmExcursionPath(i, waveCanvas);
                if (!pwmPath.isEmpty())
                {
                    juce::PathStrokeType dashedStroke(1.2f);
                    const float dashes[] = { 3.0f, 3.0f };
                    dashedStroke.createDashedStroke(pwmPath, pwmPath, dashes, 2);
                    g.setColour(accent_.withAlpha(0.65f));
                    g.strokePath(pwmPath, juce::PathStrokeType(1.2f));
                }
            }

            g.setFont(Typography::makeFont(10.5f));
            g.setColour(isSelected ? accent_ : Colours::kText);
            juce::String name = isLfo_ ? kLfoWaves[i] : kAlphaWaves[i];
            g.drawText(name, static_cast<int>(tileRect.getX()), static_cast<int>(tileRect.getBottom() - 20.0f),
                       static_cast<int>(tileRect.getWidth()), 18, juce::Justification::centred);
        }
    }

    std::function<void()> onDismiss;

private:
    int getTileIndexAt(juce::Point<int> pt) const
    {
        if (!dialogBounds_.contains(pt)) return -1;
        int cols = 5;
        int rows = isLfo_ ? 1 : 3;
        int total = isLfo_ ? 5 : 14;

        auto gridArea = dialogBounds_.reduced(16).withTrimmedTop(28);
        float tileW = gridArea.getWidth() / static_cast<float>(cols);
        float tileH = gridArea.getHeight() / static_cast<float>(rows);

        int col = static_cast<int>((pt.x - gridArea.getX()) / tileW);
        int row = static_cast<int>((pt.y - gridArea.getY()) / tileH);

        if (col >= 0 && col < cols && row >= 0 && row < rows)
        {
            int idx = row * cols + col;
            if (idx >= 0 && idx < total) return idx;
        }
        return -1;
    }

    bool isLfo_{false};
    int currentSelection_{0};
    int hoveredIdx_{-1};
    juce::Colour accent_;
    std::function<void(int)> onSelectWave_;

    juce::Label titleLabel;
    juce::TextButton closeButton;
    juce::Rectangle<int> dialogBounds_;
};

} // namespace CrackedChimes::UI
