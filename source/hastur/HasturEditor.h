#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "HasturProcessor.h"
#include "WaveformPreviewEngine.h"
#include "HasturControlInfo.h"
#include <unordered_map>
#include <array>
#include <functional>
#include <memory>
#include <vector>

// ==============================================================================
// Waveform Geometry & Visual Vector Helpers
// ==============================================================================

#include <optional>

namespace HasturColours {
    static const juce::Colour kDco       = juce::Colour(0xFF00C8F0); // 2000s Movie Poster Blue
    static const juce::Colour kSub       = juce::Colour(0xFFFF7A1A); // 2000s Movie Poster Orange
    static const juce::Colour kVcf       = juce::Colour(0xFF7DFF3B); // TB-303 Acid Green
    static const juce::Colour kMixer     = juce::Colour(0xFFFF3347); // Blistering Red
    static const juce::Colour kOutput    = juce::Colour(0xFFFF3347); // Blistering Red
    static const juce::Colour kVca       = juce::Colour(0xFFFF3347); // Blistering Red
    static const juce::Colour kNoise     = juce::Colour(0xFFB66DFF); // Spectral Purple
    static const juce::Colour kLfo       = juce::Colour(0xFFF2E10F); // Caustic Hastur Yellow
    static const juce::Colour kTextWhite = juce::Colour(0xFFF0F6FC); // Roland Silkscreen White
    static const juce::Colour kChassisBg = juce::Colour(0xFF0D1117); // Carbon Chassis
    static const juce::Colour kBayBg     = juce::Colour(0xFF161B22); // Recessed Bay Deck

    // Lifeform SSoT Canon Palette
    static const juce::Colour kSynthetic = juce::Colour(0xFF00C8FF); // Tron Blue
    static const juce::Colour kCyborg    = juce::Colour(0xFFE08C00); // Rusted Construction Equipment Yellow
    static const juce::Colour kOrganic   = juce::Colour(0xFF6BBF59); // Warm Green
}

namespace HasturWaveUI {
    juce::Path createWaveformPath(int waveIndex, juce::Rectangle<float> r, bool isLfo = false);
    juce::Path createPwmExcursionPath(int waveIndex, juce::Rectangle<float> r);
}

// ==============================================================================
// Custom Lightweight Controls & SOTA RAII Attachments
// ==============================================================================

class DiscreteParamAttachment {
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

    void setValueAsCompleteGesture(float newValue) {
        attachment_.setValueAsCompleteGesture(newValue);
    }

private:
    juce::ParameterAttachment attachment_;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DiscreteParamAttachment)
};
class RockerToggle : public juce::Component {
public:
    RockerToggle(const juce::String& leftText, const juce::String& rightText)
        : leftLabel(leftText), rightLabel(rightText) {}

    void setAccentColour(juce::Colour c) {
        if (accentColour != c) {
            accentColour = c;
            repaint();
        }
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xFF161B22));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF30363D));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        auto halfW = bounds.getWidth() * 0.5f;
        auto activeRect = isRight
            ? juce::Rectangle<float>(bounds.getX() + halfW, bounds.getY(), halfW, bounds.getHeight())
            : juce::Rectangle<float>(bounds.getX(), bounds.getY(), halfW, bounds.getHeight());

        g.setColour(accentColour);
        g.fillRoundedRectangle(activeRect.reduced(1.0f), 3.0f);

        g.setFont(juce::Font(juce::FontOptions(11.5f).withStyle("Bold")));
        g.setColour(isRight ? juce::Colour(0xFF8B949E) : juce::Colour(0xFF0D1117));
        g.drawText(leftLabel, static_cast<int>(bounds.getX()), static_cast<int>(bounds.getY()),
                   static_cast<int>(halfW), static_cast<int>(bounds.getHeight()), juce::Justification::centred);

        g.setColour(isRight ? juce::Colour(0xFF0D1117) : juce::Colour(0xFF8B949E));
        g.drawText(rightLabel, static_cast<int>(bounds.getX() + halfW), static_cast<int>(bounds.getY()),
                   static_cast<int>(halfW), static_cast<int>(bounds.getHeight()), juce::Justification::centred);
    }

    void mouseDown(const juce::MouseEvent& e) override {
        isRight = (e.position.x > getWidth() * 0.5f);
        if (onToggle) onToggle(isRight);
        repaint();
    }

    void setState(bool right) {
        if (isRight != right) {
            isRight = right;
            repaint();
        }
    }

    bool getState() const noexcept { return isRight; }
    std::function<void(bool)> onToggle;

private:
    juce::String leftLabel, rightLabel;
    bool isRight{false};
    juce::Colour accentColour{0xFFE5A91E};
};

class RockerToggle3 : public juce::Component {
public:
    RockerToggle3(const std::array<juce::String, 3>& labels) : labels_(labels) {}

    void setAccentColour(juce::Colour c) {
        segmentColours_[std::clamp(activeIndex_, 0, 2)] = c;
        repaint();
    }

    void setSegmentColour(int idx, juce::Colour c) {
        if (idx >= 0 && idx < 3) {
            segmentColours_[idx] = c;
            repaint();
        }
    }

    juce::Colour getActiveColour() const noexcept {
        return segmentColours_[std::clamp(activeIndex_, 0, 2)];
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xFF161B22));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF30363D));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        float segW = bounds.getWidth() / 3.0f;
        auto activeRect = juce::Rectangle<float>(bounds.getX() + activeIndex_ * segW, bounds.getY(), segW, bounds.getHeight());

        g.setColour(segmentColours_[std::clamp(activeIndex_, 0, 2)]);
        g.fillRoundedRectangle(activeRect.reduced(1.0f), 3.0f);

        g.setFont(juce::Font(juce::FontOptions(11.5f).withStyle("Bold")));
        for (int i = 0; i < 3; ++i) {
            g.setColour((i == activeIndex_) ? juce::Colour(0xFF0D1117) : juce::Colour(0xFF8B949E));
            g.drawText(labels_[i], static_cast<int>(bounds.getX() + i * segW), static_cast<int>(bounds.getY()),
                       static_cast<int>(segW), static_cast<int>(bounds.getHeight()), juce::Justification::centred);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        float segW = static_cast<float>(getWidth()) / 3.0f;
        int idx = std::clamp(static_cast<int>(e.position.x / segW), 0, 2);
        if (idx != activeIndex_) {
            activeIndex_ = idx;
            if (onSelect) onSelect(activeIndex_);
            repaint();
        }
    }

    void setState(int idx) {
        idx = std::clamp(idx, 0, 2);
        if (activeIndex_ != idx) {
            activeIndex_ = idx;
            repaint();
        }
    }

    int getState() const noexcept { return activeIndex_; }
    std::function<void(int)> onSelect;

private:
    std::array<juce::String, 3> labels_;
    int activeIndex_{0};
    std::array<juce::Colour, 3> segmentColours_{
        juce::Colour(0xFF00C8FF), // Synthetic: Tron Blue
        juce::Colour(0xFFE08C00), // Cyborg: Rusted Construction Equipment Yellow
        juce::Colour(0xFF6BBF59)  // Organic: Warm Green
    };
};

class SegmentedButtonBar : public juce::Component {
public:
    using GlyphFn = std::function<juce::Path(juce::Rectangle<float>)>;

    SegmentedButtonBar(const std::vector<juce::String>& labels)
        : labels_(labels) {}

    void setVertical(bool isVertical) {
        if (isVertical_ != isVertical) {
            isVertical_ = isVertical;
            repaint();
        }
    }

    void setGlyphProviders(std::vector<GlyphFn> glyphs) {
        glyphProviders_ = std::move(glyphs);
        repaint();
    }

    void setAccentColour(juce::Colour c) {
        if (accentColour_ != c) {
            accentColour_ = c;
            repaint();
        }
    }

    void setState(int idx) {
        idx = std::clamp(idx, 0, static_cast<int>(labels_.size()) - 1);
        if (activeIndex_ != idx) {
            activeIndex_ = idx;
            repaint();
        }
    }

    int getState() const noexcept { return activeIndex_; }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.isEmpty() || labels_.empty()) return;

        g.setColour(juce::Colour(0xFF161B22));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF30363D));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        const int numSegs = static_cast<int>(labels_.size());

        if (isVertical_) {
            const float segH = bounds.getHeight() / static_cast<float>(numSegs);

            // Active Segment Highlight
            auto activeRect = juce::Rectangle<float>(bounds.getX(), bounds.getY() + activeIndex_ * segH, bounds.getWidth(), segH);
            g.setColour(getActiveColour());
            g.fillRoundedRectangle(activeRect.reduced(1.0f), 3.0f);

            // Inner Dividers between inactive segments
            g.setColour(juce::Colour(0xFF262D38));
            for (int i = 1; i < numSegs; ++i) {
                if (i != activeIndex_ && i != activeIndex_ + 1) {
                    float divY = bounds.getY() + i * segH;
                    g.drawHorizontalLine(static_cast<int>(std::round(divY)), bounds.getX() + 3.0f, bounds.getRight() - 3.0f);
                }
            }

            // Segment Labels or Waveform Glyphs
            g.setFont(juce::Font(juce::FontOptions(11.5f).withStyle("Bold")));
            for (int i = 0; i < numSegs; ++i) {
                bool isActive = (i == activeIndex_);
                auto segRect = juce::Rectangle<float>(bounds.getX(), bounds.getY() + i * segH, bounds.getWidth(), segH);
                
                if (i < static_cast<int>(glyphProviders_.size()) && glyphProviders_[i] != nullptr) {
                    auto glyphBounds = segRect.reduced(bounds.getWidth() * 0.18f, segH * 0.22f);
                    auto glyphPath = glyphProviders_[i](glyphBounds);
                    float strokeW = isActive ? 2.0f : 1.5f;
                    g.setColour(isActive ? juce::Colour(0xFF0D1117) : juce::Colour(0xFF8B949E));
                    g.strokePath(glyphPath, juce::PathStrokeType(strokeW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                } else {
                    g.setColour(isActive ? juce::Colour(0xFF0D1117) : juce::Colour(0xFFF0F6FC));
                    g.drawText(labels_[i], static_cast<int>(segRect.getX()), static_cast<int>(segRect.getY()),
                               static_cast<int>(segRect.getWidth()), static_cast<int>(segRect.getHeight()),
                               juce::Justification::centred);
                }
            }
        } else {
            const float segW = bounds.getWidth() / static_cast<float>(numSegs);

            // Active Segment Highlight
            auto activeRect = juce::Rectangle<float>(bounds.getX() + activeIndex_ * segW, bounds.getY(), segW, bounds.getHeight());
            g.setColour(getActiveColour());
            g.fillRoundedRectangle(activeRect.reduced(1.0f), 3.0f);

            // Inner Dividers between inactive segments
            g.setColour(juce::Colour(0xFF262D38));
            for (int i = 1; i < numSegs; ++i) {
                if (i != activeIndex_ && i != activeIndex_ + 1) {
                    float divX = bounds.getX() + i * segW;
                    g.drawVerticalLine(static_cast<int>(std::round(divX)), bounds.getY() + 3.0f, bounds.getBottom() - 3.0f);
                }
            }

            // Segment Labels or Waveform Glyphs
            g.setFont(juce::Font(juce::FontOptions(12.5f).withStyle("Bold")));
            for (int i = 0; i < numSegs; ++i) {
                bool isActive = (i == activeIndex_);
                auto segRect = juce::Rectangle<float>(bounds.getX() + i * segW, bounds.getY(), segW, bounds.getHeight());
                
                if (i < static_cast<int>(glyphProviders_.size()) && glyphProviders_[i] != nullptr) {
                    // Render Vector Waveform Glyph (TAL-Pha SOTA)
                    auto glyphBounds = segRect.reduced(segW * 0.18f, bounds.getHeight() * 0.22f);
                    auto glyphPath = glyphProviders_[i](glyphBounds);
                    float strokeW = isActive ? 2.0f : 1.5f;
                    g.setColour(isActive ? juce::Colour(0xFF0D1117) : juce::Colour(0xFF8B949E));
                    g.strokePath(glyphPath, juce::PathStrokeType(strokeW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                } else {
                    g.setColour(isActive ? juce::Colour(0xFF0D1117) : juce::Colour(0xFFF0F6FC));
                    g.drawText(labels_[i], static_cast<int>(segRect.getX()), static_cast<int>(segRect.getY()),
                               static_cast<int>(segRect.getWidth()), static_cast<int>(segRect.getHeight()),
                               juce::Justification::centred);
                }
            }
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        if (labels_.empty()) return;
        const int numSegs = static_cast<int>(labels_.size());
        int idx = 0;
        if (isVertical_) {
            const float segH = static_cast<float>(getHeight()) / static_cast<float>(numSegs);
            idx = std::clamp(static_cast<int>(e.position.y / segH), 0, numSegs - 1);
        } else {
            const float segW = static_cast<float>(getWidth()) / static_cast<float>(numSegs);
            idx = std::clamp(static_cast<int>(e.position.x / segW), 0, numSegs - 1);
        }
        if (idx != activeIndex_) {
            activeIndex_ = idx;
            if (onSelect) onSelect(activeIndex_);
            repaint();
        }
    }

    std::function<void(int)> onSelect;

    void setSegmentColour(int idx, juce::Colour c) {
        if (idx < 0) return;
        if (segmentColours_.size() <= static_cast<size_t>(idx))
            segmentColours_.resize(static_cast<size_t>(idx) + 1);
        segmentColours_[static_cast<size_t>(idx)] = c;
        if (idx == activeIndex_) repaint();
    }

    juce::Colour getActiveColour() const noexcept {
        if (activeIndex_ >= 0 && static_cast<size_t>(activeIndex_) < segmentColours_.size() && segmentColours_[static_cast<size_t>(activeIndex_)].has_value())
            return *segmentColours_[static_cast<size_t>(activeIndex_)];
        return accentColour_;
    }

private:
    std::vector<juce::String> labels_;
    std::vector<GlyphFn> glyphProviders_;
    std::vector<std::optional<juce::Colour>> segmentColours_;
    int activeIndex_{0};
    juce::Colour accentColour_{0xFFE5A91E};
    bool isVertical_{false};
};
class PulseGridBar : public juce::Component {
public:
    using GlyphFn = std::function<juce::Path(juce::Rectangle<float>)>;

    PulseGridBar(const juce::String& title, const std::vector<juce::String>& optionLabels)
        : title_(title), labels_(optionLabels) {}

    void setGlyphProviders(std::vector<GlyphFn> glyphs) {
        glyphProviders_ = std::move(glyphs);
        repaint();
    }

    void setAccentColour(juce::Colour c) {
        if (accentColour_ != c) {
            accentColour_ = c;
            repaint();
        }
    }

    void setState(int idx) {
        idx = std::clamp(idx, 0, 4);
        if (activeIndex_ != idx) {
            activeIndex_ = idx;
            repaint();
        }
    }

    int getState() const noexcept { return activeIndex_; }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.isEmpty()) return;

        g.setColour(juce::Colour(0xFF161B22));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF30363D));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        const float colW = bounds.getWidth() / 3.0f;
        const float rowH = bounds.getHeight() / 2.0f;

        // Title Cell (0, 0)
        auto titleRect = juce::Rectangle<float>(bounds.getX(), bounds.getY(), colW, rowH);
        g.setColour(juce::Colour(0xFF0D1117));
        g.fillRoundedRectangle(titleRect.reduced(1.0f), 3.0f);
        g.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
        g.setColour(accentColour_);
        g.drawText(title_, titleRect, juce::Justification::centred);

        // Dividers
        g.setColour(juce::Colour(0xFF262D38));
        g.drawHorizontalLine(static_cast<int>(std::round(bounds.getY() + rowH)), bounds.getX() + 3.0f, bounds.getRight() - 3.0f);
        g.drawVerticalLine(static_cast<int>(std::round(bounds.getX() + colW)), bounds.getY() + 3.0f, bounds.getBottom() - 3.0f);
        g.drawVerticalLine(static_cast<int>(std::round(bounds.getX() + colW * 2.0f)), bounds.getY() + 3.0f, bounds.getBottom() - 3.0f);

        // Option Mapping: 0 -> (0,1), 1 -> (0,2), 2 -> (1,0), 3 -> (1,1), 4 -> (1,2)
        static const int gridPos[5][2] = { {0, 1}, {0, 2}, {1, 0}, {1, 1}, {1, 2} };

        g.setFont(juce::Font(juce::FontOptions(12.0f).withStyle("Bold")));
        for (int i = 0; i < 5; ++i) {
            int r = gridPos[i][0];
            int c = gridPos[i][1];
            auto cellRect = juce::Rectangle<float>(bounds.getX() + c * colW, bounds.getY() + r * rowH, colW, rowH);
            bool isActive = (i == activeIndex_);

            if (isActive) {
                g.setColour(accentColour_);
                g.fillRoundedRectangle(cellRect.reduced(1.5f), 3.0f);
            }

            if (i < static_cast<int>(glyphProviders_.size()) && glyphProviders_[i] != nullptr) {
                auto glyphBounds = cellRect.reduced(colW * 0.16f, rowH * 0.20f);
                auto glyphPath = glyphProviders_[i](glyphBounds);
                float strokeW = isActive ? 2.0f : 1.5f;
                g.setColour(isActive ? juce::Colour(0xFF0D1117) : juce::Colour(0xFF8B949E));
                g.strokePath(glyphPath, juce::PathStrokeType(strokeW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            } else if (i < static_cast<int>(labels_.size())) {
                g.setColour(isActive ? juce::Colour(0xFF0D1117) : juce::Colour(0xFFF0F6FC));
                g.drawText(labels_[i], cellRect, juce::Justification::centred);
            }
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
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

        if (clickedIdx >= 0 && clickedIdx != activeIndex_) {
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
    juce::Colour accentColour_{0xFFFF7A1A};
};

class Grid2x2Bar : public juce::Component {
public:
    Grid2x2Bar(const juce::String& title, const std::vector<juce::String>& optionLabels)
        : title_(title), labels_(optionLabels) {}

    void setAccentColour(juce::Colour c) {
        if (accentColour_ != c) {
            accentColour_ = c;
            repaint();
        }
    }

    void setItemColours(const std::vector<juce::Colour>& c) {
        itemColours_ = c;
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

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.isEmpty()) return;

        g.setColour(juce::Colour(0xFF161B22));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF30363D));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        const float colW = bounds.getWidth() * 0.5f;
        const float rowH = bounds.getHeight() * 0.5f;

        // Title Cell (0, 0)
        auto titleRect = juce::Rectangle<float>(bounds.getX(), bounds.getY(), colW, rowH);
        g.setColour(juce::Colour(0xFF0D1117));
        g.fillRoundedRectangle(titleRect.reduced(1.0f), 3.0f);
        g.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
        g.setColour(accentColour_);
        g.drawText(title_, titleRect, juce::Justification::centred);

        // Dividers
        g.setColour(juce::Colour(0xFF262D38));
        g.drawHorizontalLine(static_cast<int>(std::round(bounds.getY() + rowH)), bounds.getX() + 3.0f, bounds.getRight() - 3.0f);
        g.drawVerticalLine(static_cast<int>(std::round(bounds.getX() + colW)), bounds.getY() + 3.0f, bounds.getBottom() - 3.0f);

        // 3 Options Mapping: 0 -> (0, 1), 1 -> (1, 0), 2 -> (1, 1)
        static const int gridPos[3][2] = { {0, 1}, {1, 0}, {1, 1} };

        g.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
        for (int i = 0; i < 3 && i < static_cast<int>(labels_.size()); ++i) {
            int r = gridPos[i][0];
            int c = gridPos[i][1];
            auto cellRect = juce::Rectangle<float>(bounds.getX() + c * colW, bounds.getY() + r * rowH, colW, rowH);
            bool isActive = (i == activeIndex_);

            juce::Colour cellCol = (i < static_cast<int>(itemColours_.size())) ? itemColours_[i] : accentColour_;

            if (isActive) {
                g.setColour(cellCol);
                g.fillRoundedRectangle(cellRect.reduced(1.5f), 3.0f);
                g.setColour(juce::Colour(0xFF0D1117));
            } else {
                g.setColour(juce::Colour(0xFF1E2530));
                g.fillRoundedRectangle(cellRect.reduced(1.5f), 3.0f);
                g.setColour(cellCol.withAlpha(0.75f));
            }
            g.drawText(labels_[i], cellRect, juce::Justification::centred);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        const float colW = static_cast<float>(getWidth()) * 0.5f;
        const float rowH = static_cast<float>(getHeight()) * 0.5f;
        int c = std::clamp(static_cast<int>(e.position.x / colW), 0, 1);
        int r = std::clamp(static_cast<int>(e.position.y / rowH), 0, 1);

        int clickedIdx = -1;
        if (r == 0 && c == 1) clickedIdx = 0;
        else if (r == 1 && c == 0) clickedIdx = 1;
        else if (r == 1 && c == 1) clickedIdx = 2;

        if (clickedIdx >= 0 && clickedIdx != activeIndex_) {
            activeIndex_ = clickedIdx;
            if (onSelect) onSelect(activeIndex_);
            repaint();
        }
    }

    std::function<void(int)> onSelect;

private:
    juce::String title_;
    std::vector<juce::String> labels_;
    std::vector<juce::Colour> itemColours_;
    int activeIndex_{0};
    juce::Colour accentColour_{0xFF7DFF3B};
};



class EnvelopeVisualizer : public juce::Component {
public:
    void setTitle(const juce::String& t) {
        title_ = t;
        repaint();
    }

    void setAccentColour(juce::Colour c) {
        if (accentColour != c) {
            accentColour = c;
            repaint();
        }
    }

    void setVortexParams(float attackSec, float snap, float decaySec, float bloom, float aftershock, float sustainLevel, float releaseSec) {
        isSubStator_ = false;
        atkMs_ = attackSec * 1000.0f;
        snap_ = snap;
        decMs_ = decaySec * 1000.0f;
        bloom_ = bloom;
        aftershock_ = aftershock;
        sustainLevel_ = sustainLevel;
        relMs_ = releaseSec * 1000.0f;
        repaint();
    }

    void setSubStatorParams(float snapSec, float decaySec, float curvature, float sustainLevel, float releaseSec, float biteDepth) {
        isSubStator_ = true;
        snapMs_ = snapSec * 1000.0f;
        decMs_ = decaySec * 1000.0f;
        curvature_ = curvature;
        sustainLevel_ = sustainLevel;
        relMs_ = releaseSec * 1000.0f;
        biteDepth_ = biteDepth;
        repaint();
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xFF0F131A));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF262D38));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        float headerH = isSubStator_ ? 0.0f : 18.0f;
        auto plotBounds = bounds.withTrimmedTop(headerH).reduced(4.0f);

        // Grid lines
        g.setColour(juce::Colour(0xFF1E2530));
        g.drawHorizontalLine(static_cast<int>(plotBounds.getCentreY()), plotBounds.getX(), plotBounds.getRight());
        g.drawVerticalLine(static_cast<int>(plotBounds.getCentreX()), plotBounds.getY(), plotBounds.getBottom());

        juce::Path p;
        float x0 = plotBounds.getX();
        float y0 = plotBounds.getBottom();
        p.startNewSubPath(x0, y0);

        float w = plotBounds.getWidth();
        float h = plotBounds.getHeight();

        if (!isSubStator_) {
            // Proportional Vortex Curve (Logarithmic Stage Weighting W = t^0.45)
            float wAtk = std::pow(std::clamp(atkMs_, 1.0f, 5000.0f), 0.45f);
            float wDec = std::pow(std::clamp(decMs_, 5.0f, 8000.0f), 0.45f);
            float wSus = 10.0f; // Visible sustain shelf
            float wRel = std::pow(std::clamp(relMs_, 5.0f, 10000.0f), 0.45f);
            float wTotal = wAtk + wDec + wSus + wRel;

            float pAtkX = x0 + w * (wAtk / wTotal);
            float pPeakY = plotBounds.getY() + h * (1.0f - std::clamp(snap_, 0.1f, 1.0f));
            p.lineTo(pAtkX, pPeakY);

            float pDecMidX = pAtkX + w * (wDec * 0.45f / wTotal);
            float breakNorm = std::clamp(sustainLevel_ + bloom_ * 0.40f, 0.0f, 1.0f);
            float pBreakY = plotBounds.getBottom() - h * breakNorm;
            p.lineTo(pDecMidX, pBreakY);

            float pDecEndX = pAtkX + w * (wDec / wTotal);
            float afterNorm = std::clamp(sustainLevel_ + aftershock_ * 0.40f, 0.0f, 1.0f);
            float pAfterY = plotBounds.getBottom() - h * afterNorm;
            p.lineTo(pDecEndX, pAfterY);

            float pSusX = pDecEndX + w * (wSus / wTotal);
            float pSusY = plotBounds.getBottom() - h * std::clamp(sustainLevel_, 0.0f, 1.0f);
            p.lineTo(pSusX, pSusY);

            float pRelX = plotBounds.getRight();
            p.lineTo(pRelX, plotBounds.getBottom());

            g.setColour(accentColour);
            g.strokePath(p, juce::PathStrokeType(2.0f));

            g.setFont(juce::Font(juce::FontOptions(10.5f).withStyle("Bold")));
            g.setColour(accentColour);
            juce::String labelPrefix = title_.isNotEmpty() ? title_ : "VORTEX";
            juce::String timeStr = juce::String::formatted("%s A:%dms D:%dms", labelPrefix.toRawUTF8(), (int)std::round(atkMs_), (int)std::round(decMs_));
            g.drawText(timeStr, static_cast<int>(bounds.getX() + 4.0f), static_cast<int>(bounds.getY() + 1.0f),
                       130, 16, juce::Justification::left);

            juce::String susStr = juce::String::formatted("R:%dms S:%d%%", (int)std::round(relMs_), (int)std::round(sustainLevel_ * 100.0f));
            g.setColour(juce::Colour(0xFF8B949E));
            g.drawText(susStr, static_cast<int>(bounds.getRight() - 110.0f), static_cast<int>(bounds.getY() + 1.0f),
                       106, 16, juce::Justification::right);
        } else {
            // Proportional SubStator Curve (Full OLED Canvas without Header)
            float wSnap = std::pow(std::clamp(snapMs_, 0.5f, 50.0f), 0.45f);
            float wDec  = std::pow(std::clamp(decMs_, 10.0f, 5000.0f), 0.45f);
            float wSus  = 8.0f;
            float wRel  = std::pow(std::clamp(relMs_, 5.0f, 8000.0f), 0.45f);
            float wTotal = wSnap + wDec + wSus + wRel;

            float pSnapX = x0 + w * (wSnap / wTotal);
            p.lineTo(pSnapX, plotBounds.getY() + 2.0f);

            float pDecEndX = pSnapX + w * (wDec / wTotal);
            const float susNorm = std::clamp(sustainLevel_, 0.0f, 1.0f);
            const float susY = plotBounds.getBottom() - h * susNorm;

            // 32-point exact continuous alpha-power decay curve: alpha = 2.0 * 3^(-curvature)
            const float alpha = 2.0f * std::pow(3.0f, -curvature_);
            constexpr int kDecayPoints = 32;
            for (int i = 1; i <= kDecayPoints; ++i) {
                const float pFrac = static_cast<float>(i) / static_cast<float>(kDecayPoints);
                const float currX = pSnapX + (pDecEndX - pSnapX) * pFrac;
                const float curveVal = susNorm + (1.0f - susNorm) * std::pow(1.0f - pFrac, alpha);
                const float currY = plotBounds.getBottom() - h * std::clamp(curveVal, 0.0f, 1.0f);
                p.lineTo(currX, currY);
            }

            float pSusX = pDecEndX + w * (wSus / wTotal);
            p.lineTo(pSusX, susY);

            // Release stage: exponential release to bottom
            constexpr int kReleasePoints = 16;
            float pRelX = plotBounds.getRight();
            for (int i = 1; i <= kReleasePoints; ++i) {
                const float rFrac = static_cast<float>(i) / static_cast<float>(kReleasePoints);
                const float currX = pSusX + (pRelX - pSusX) * rFrac;
                const float relVal = susNorm * std::pow(1.0f - rFrac, 1.8f);
                const float currY = plotBounds.getBottom() - h * std::clamp(relVal, 0.0f, 1.0f);
                p.lineTo(currX, currY);
            }

            g.setColour(accentColour);
            g.strokePath(p, juce::PathStrokeType(2.0f));
        }
    }

private:
    bool isSubStator_{true};
    float atkMs_{10.0f};
    float snap_{1.0f};
    float decMs_{400.0f};
    float bloom_{0.0f};
    float aftershock_{0.0f};
    float sustainLevel_{0.30f};
    float relMs_{500.0f};

    float snapMs_{1.5f};
    float curvature_{0.0f};
    float biteDepth_{0.0f};
    float punchDepth_{0.0f};
    juce::Colour accentColour{0xFFE5A91E};
    juce::String title_;
};

// ==============================================================================
// LFO Waveform Mini-Visualizer (Live Vector Morphing Display)
// ==============================================================================
class LfoWaveformVisualizer : public juce::Component {
public:
    void setParams(int waveIndex, float shape) {
        if (waveIndex_ != waveIndex || std::abs(shape_ - shape) > 0.002f) {
            waveIndex_ = waveIndex;
            shape_ = shape;
            repaint();
        }
    }

    void setAccentColour(juce::Colour c) {
        if (accentColour_ != c) {
            accentColour_ = c;
            repaint();
        }
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.isEmpty()) return;

        // OLED chassis
        g.setColour(juce::Colour(0xFF0F131A));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF262D38));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        auto plotBounds = bounds.reduced(5.0f);
        float w = plotBounds.getWidth();
        float h = plotBounds.getHeight();
        float centreY = plotBounds.getCentreY();

        // Center line
        g.setColour(juce::Colour(0xFF1E2530));
        g.drawHorizontalLine(static_cast<int>(centreY), plotBounds.getX(), plotBounds.getRight());

        juce::Path p;
        const int steps = 48;
        for (int i = 0; i <= steps; ++i) {
            float normX = static_cast<float>(i) / static_cast<float>(steps);
            float px = plotBounds.getX() + normX * w;
            float val = 0.0f;
            switch (waveIndex_) {
                case 0: { // TRIANGLE / SAW
                    const float c = std::clamp(shape_, 0.0005f, 0.9995f);
                    val = (normX < c) ? (-1.0f + 2.0f * (normX / c))
                                      : ( 1.0f - 2.0f * ((normX - c) / (1.0f - c)));
                    break;
                }
                case 1: { // PULSE
                    const float d = std::clamp(0.01f + 0.98f * shape_, 0.01f, 0.99f);
                    val = (normX < d) ? 1.0f : -1.0f;
                    break;
                }
                case 2: { // SINE (Infinifold)
                    const float k = 1.0f + 15.0f * shape_ * shape_;
                    const float inner = std::sin(normX * 6.28318530718f);
                    val = std::sin(0.25f * k * inner * 6.28318530718f);
                    break;
                }
                case 3:
                default: { // RANDOM
                    static const float previewPts[5] = { -0.7f, 0.8f, -0.3f, 0.9f, -0.6f };
                    float seg = normX * 4.0f;
                    int segIdx = std::clamp(static_cast<int>(seg), 0, 3);
                    float segFrac = seg - static_cast<float>(segIdx);
                    float cur = previewPts[segIdx];
                    float next = previewPts[segIdx + 1];
                    const float smooth = shape_;
                    float u = segFrac;
                    u = u * u * (3.0f - 2.0f * u);
                    val = (1.0f - smooth) * cur + smooth * ((1.0f - u) * cur + u * next);
                    break;
                }
            }
            float py = centreY - val * (h * 0.40f);
            if (i == 0) p.startNewSubPath(px, py);
            else p.lineTo(px, py);
        }

        g.setColour(accentColour_);
        g.strokePath(p, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

private:
    int waveIndex_{0};
    float shape_{0.5f};
    juce::Colour accentColour_{0xFFE5C02A};
};

// ==============================================================================
// Wave Grid 2x2 Bar (4 Quadrants with Vector Glyphs)
// ==============================================================================
class WaveGrid2x2Bar : public juce::Component {
public:
    using GlyphFn = std::function<juce::Path(juce::Rectangle<float>)>;

    WaveGrid2x2Bar(const std::vector<juce::String>& labels)
        : labels_(labels) {}

    void setGlyphProviders(std::vector<GlyphFn> glyphs) {
        glyphProviders_ = std::move(glyphs);
        repaint();
    }

    void setAccentColour(juce::Colour c) {
        if (accentColour_ != c) {
            accentColour_ = c;
            repaint();
        }
    }

    void setState(int idx) {
        idx = std::clamp(idx, 0, static_cast<int>(labels_.size()) - 1);
        if (activeIndex_ != idx) {
            activeIndex_ = idx;
            repaint();
        }
    }

    int getState() const noexcept { return activeIndex_; }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.isEmpty() || labels_.empty()) return;

        g.setColour(juce::Colour(0xFF161B22));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF30363D));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        const float colW = bounds.getWidth() * 0.5f;
        const float rowH = bounds.getHeight() * 0.5f;

        // Inner Dividers
        g.setColour(juce::Colour(0xFF262D38));
        g.drawHorizontalLine(static_cast<int>(std::round(bounds.getY() + rowH)), bounds.getX() + 3.0f, bounds.getRight() - 3.0f);
        g.drawVerticalLine(static_cast<int>(std::round(bounds.getX() + colW)), bounds.getY() + 3.0f, bounds.getBottom() - 3.0f);

        g.setFont(juce::Font(juce::FontOptions(10.5f).withStyle("Bold")));
        for (int i = 0; i < 4 && i < static_cast<int>(labels_.size()); ++i) {
            int r = i / 2;
            int c = i % 2;
            auto cellRect = juce::Rectangle<float>(bounds.getX() + c * colW, bounds.getY() + r * rowH, colW, rowH);
            bool isActive = (i == activeIndex_);

            if (isActive) {
                g.setColour(accentColour_);
                g.fillRoundedRectangle(cellRect.reduced(1.5f), 3.0f);
            }

            if (i < static_cast<int>(glyphProviders_.size()) && glyphProviders_[i] != nullptr) {
                auto glyphBounds = cellRect.reduced(colW * 0.18f, rowH * 0.20f);
                auto glyphPath = glyphProviders_[i](glyphBounds);
                float strokeW = isActive ? 2.0f : 1.5f;
                g.setColour(isActive ? juce::Colour(0xFF0D1117) : juce::Colour(0xFF8B949E));
                g.strokePath(glyphPath, juce::PathStrokeType(strokeW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            } else {
                g.setColour(isActive ? juce::Colour(0xFF0D1117) : juce::Colour(0xFFF0F6FC));
                g.drawText(labels_[i], cellRect, juce::Justification::centred);
            }
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        const float colW = static_cast<float>(getWidth()) * 0.5f;
        const float rowH = static_cast<float>(getHeight()) * 0.5f;
        int c = std::clamp(static_cast<int>(e.position.x / colW), 0, 1);
        int r = std::clamp(static_cast<int>(e.position.y / rowH), 0, 1);
        int clickedIdx = r * 2 + c;
        if (clickedIdx >= 0 && clickedIdx < static_cast<int>(labels_.size()) && clickedIdx != activeIndex_) {
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
    juce::Colour accentColour_{0xFFE5C02A};
};

// ==============================================================================
// Waveform Picker Button (Mini preview + Wave Name + Dropdown Click)
// ==============================================================================
class WaveformPickerButton : public juce::Component {
public:
    int waveIndex{0};
    bool isLfo{false};
    juce::String waveName;
    std::function<void()> onClick;
    juce::Colour accentColour{0xFFE5A91E};

    void setWaveIndex(int idx) {
        waveIndex = idx;
        repaint();
    }

    void setAccentColour(juce::Colour c) {
        accentColour = c;
        repaint();
    }

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent&) override {
        if (onClick) onClick();
    }
};



// ==============================================================================
// PresetStubComponent: Header Preset Selector Stub Capsule
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
        // Recessed capsule
        g.setColour(juce::Colour(0xFF0F131A));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF262D38));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        // Display text: < 01: ACID PRIME >
        g.setFont(juce::Font(juce::FontOptions(12.5f).withStyle("Bold")));
        g.setColour(accent_.withAlpha(0.90f));
        g.drawText("<  01: ACID PRIME  >", bounds, juce::Justification::centred);
    }

private:
    juce::Colour accent_{0xFF7DFF3B};
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
        juce::Colour accent{0xFF7DFF3B}; // Default Acid Green
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
        d.name = "HASTUR";
        d.value = "READY";
        d.blurb = "dual-ir3r05 metamorphic analog bass synthesizer";
        d.accent = juce::Colour(0xFF7DFF3B);
        d.isDefault = true;
        setReadout(d);
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();

        // 1. Recessed OLED display cavity
        g.setColour(juce::Colour(0xFF0F131A));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF262D38));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        // Unified Monolithic OLED Ribbon (w: 566px, h: 28px, >= 13pt typography, zero nested box)
        // Col 1: Name (x: 10, w: 105, h: 28, 13pt bold)
        auto nameRect = juce::Rectangle<float>(10.0f, 0.0f, 105.0f, bounds.getHeight());
        g.setFont(juce::Font(juce::FontOptions(13.0f).withStyle("Bold")));
        g.setColour(readout_.accent);
        g.drawText(readout_.name, nameRect, juce::Justification::centredLeft, true);

        // Col 2: Value (x: 118, w: 90, h: 28, 13pt bold)
        auto valRect = juce::Rectangle<float>(118.0f, 0.0f, 90.0f, bounds.getHeight());
        g.setFont(juce::Font(juce::FontOptions(13.0f).withStyle("Bold")));
        g.setColour(HasturColours::kTextWhite);
        g.drawText(readout_.value, valRect, juce::Justification::centredLeft, true);

        // Col 3: Separator (x: 210, w: 12, h: 28)
        auto sepRect = juce::Rectangle<float>(210.0f, 0.0f, 12.0f, bounds.getHeight());
        g.setFont(juce::Font(juce::FontOptions(13.0f)));
        g.setColour(juce::Colour(0xFF8B949E));
        g.drawText(juce::CharPointer_UTF8("\xe2\x80\x94"), sepRect, juce::Justification::centred);

        // Col 4: Explanation blurb (x: 226, w: 330, h: 28, 13pt)
        auto blurbRect = juce::Rectangle<float>(226.0f, 0.0f, 330.0f, bounds.getHeight());
        g.setFont(juce::Font(juce::FontOptions(13.0f)));
        g.setColour(readout_.isDefault ? juce::Colour(0xFF6E7681) : juce::Colour(0xFFC9D1D9));
        g.drawText(readout_.blurb, blurbRect, juce::Justification::centredLeft, true);
    }

private:
    Readout readout_;
};

// ==============================================================================
// Custom Rotary & Linear LookAndFeel with Dynamic Accent Tinting
// ==============================================================================
class HasturLookAndFeel : public juce::LookAndFeel_V4 {
public:
    HasturLookAndFeel();
    void setAccentColour(juce::Colour c);
    juce::Colour getAccentColour() const noexcept { return accentColour_; }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override;

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider& slider) override;

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override;

private:
    juce::Colour accentColour_{0xFFE5A91E};
};

// ==============================================================================
// Right-Click Knob Range & Curve Modal Overlay
// ==============================================================================
class KnobTaperModal : public juce::Component {
public:
    KnobTaperModal(juce::Slider& targetSlider, const juce::String& paramName, juce::Colour accent);
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;

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
// Waveform Grid Modal Overlay
// ==============================================================================
class WaveformGridModal : public juce::Component {
public:
    WaveformGridModal(bool isLfo, int currentSelection, juce::Colour accent,
                      std::function<void(int)> onSelectWave);
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;

    std::function<void()> onDismiss;

private:
    bool isLfo_{false};
    int currentSelection_{0};
    int hoveredIdx_{-1};
    juce::Colour accent_;
    std::function<void(int)> onSelectWave_;

    juce::Label titleLabel;
    juce::TextButton closeButton;
    juce::Rectangle<int> dialogBounds_;

    int getTileIndexAt(juce::Point<int> p) const;
};

// ==============================================================================
// Master Hastur Audio Processor Editor
// ==============================================================================

// ==============================================================================
// Dual SOTA Oscilloscopes: Core DCO (Scope 1 Cyan) & Sub-Stator (Scope 2 Amber)
// ==============================================================================
class DcoWaveformVisualizer : public juce::Component, private juce::Timer {
public:
    explicit DcoWaveformVisualizer(HasturAudioProcessor& processor)
        : processor_(processor), apvts_(processor.getAPVTS())
    {
        setOpaque(true);
        startTimerHz(30);
    }

    ~DcoWaveformVisualizer() override { stopTimer(); }

    void visibilityChanged() override { updateTimerState(); }
    void parentHierarchyChanged() override { updateTimerState(); }

    void updateTimerState() {
        if (isShowing()) {
            lastTickMs_ = 0.0;
            startTimerHz(animating_ ? 60 : 30);
        } else {
            stopTimer();
        }
    }

    void timerCallback() override {
        auto* famP     = apvts_.getRawParameterValue("dco_wave_family");
        auto* morphP   = apvts_.getRawParameterValue("dco_morph");
        auto* topoP    = apvts_.getRawParameterValue("dco_chopper_topo");
        auto* strangP  = apvts_.getRawParameterValue("osc1_mult");
        auto* warpP    = apvts_.getRawParameterValue("osc1_apf_pitch_track");
        auto* driveP   = apvts_.getRawParameterValue("osc1_drive");
        auto* biteP    = apvts_.getRawParameterValue("dco_bite");
        auto* lfoAmtP  = apvts_.getRawParameterValue("pw_lfo_amount");
        auto* lfoRateP = apvts_.getRawParameterValue("pw_lfo_rate");

        auto* rWarpP   = apvts_.getRawParameterValue("rand_core_warp");
        auto* rBiteP   = apvts_.getRawParameterValue("rand_core_bite");
        auto* rMeltP   = apvts_.getRawParameterValue("rand_core_melt");
        auto* rStrangP = apvts_.getRawParameterValue("rand_raw_strangle");

        const int   family   = famP    ? static_cast<int>(famP->load(std::memory_order_relaxed)) : 0;
        const float morph    = morphP  ? morphP->load(std::memory_order_relaxed)                 : 0.0f;
        const int   topo     = topoP   ? static_cast<int>(topoP->load(std::memory_order_relaxed)) : 0;
        const float strangle = strangP ? strangP->load(std::memory_order_relaxed)                : 0.0f;
        const float warp     = warpP   ? warpP->load(std::memory_order_relaxed)                  : 0.0f;
        const float drive    = driveP  ? driveP->load(std::memory_order_relaxed)                 : 0.0f;
        const float bite     = biteP   ? biteP->load(std::memory_order_relaxed)                  : 0.0f;
        const float lfoAmt   = lfoAmtP ? lfoAmtP->load(std::memory_order_relaxed)                : 0.0f;
        const float lfoRate  = lfoRateP ? lfoRateP->load(std::memory_order_relaxed)              : 0.85f;

        const float rWarpAmt   = rWarpP   ? rWarpP->load(std::memory_order_relaxed)   : 0.0f;
        const float rBiteAmt   = rBiteP   ? rBiteP->load(std::memory_order_relaxed)   : 0.0f;
        const float rMeltAmt   = rMeltP   ? rMeltP->load(std::memory_order_relaxed)   : 0.0f;
        const float rStrangAmt = rStrangP ? rStrangP->load(std::memory_order_relaxed) : 0.0f;

        const float rWarpVal   = processor_.getStepRandAtomic(0);
        const float rBiteVal   = processor_.getStepRandAtomic(1);
        const float rMeltVal   = processor_.getStepRandAtomic(2);
        const float rStrangVal = processor_.getStepRandAtomic(5);

        const float effWarp = (rWarpAmt > 0.0f)
            ? (-2.0f + 4.0f * std::clamp((1.0f - rWarpAmt) * ((warp + 2.0f) * 0.25f) + rWarpAmt * rWarpVal, 0.0f, 1.0f))
            : warp;
        const float effBite = (rBiteAmt > 0.0f)
            ? (-1.0f + 2.0f * std::clamp((1.0f - rBiteAmt) * ((bite + 1.0f) * 0.5f) + rBiteAmt * rBiteVal, 0.0f, 1.0f))
            : bite;
        const float effDrive = (rMeltAmt > 0.0f)
            ? std::clamp((1.0f - rMeltAmt) * drive + rMeltAmt * rMeltVal, 0.0f, 1.0f)
            : drive;
        const float effStrangle = (rStrangAmt > 0.0f)
            ? (1.0f + 31.0f * std::clamp((1.0f - rStrangAmt) * ((strangle - 1.0f) / 31.0f) + rStrangAmt * rStrangVal, 0.0f, 1.0f))
            : strangle;

        const bool animate = (lfoAmt > 0.001f);

        if (animate) {
            const double nowMs = juce::Time::getMillisecondCounterHiRes();
            double dt = (lastTickMs_ > 0.0) ? (nowMs - lastTickMs_) * 0.001 : 0.0;
            lastTickMs_ = nowMs;
            dt = std::min(dt, 0.1);

            const double hz = std::clamp(static_cast<double>(lfoRate), 0.01, 50.0);
            lfoPhase_ += hz * dt;
            lfoPhase_ -= std::floor(lfoPhase_);

            // Parity with HasturVoice.h:561 (Bounded Headroom Shift)
            const float lfoVal = std::sin(static_cast<float>(lfoPhase_) * 6.28318530717958647692f);
            const float effMorph = CrackedChimes::DSP::HasturVoice::boundedShift(
                std::clamp(morph, 0.0f, 1.0f), 0.0f, 1.0f, lfoAmt, lfoVal);

            family_   = family;
            topo_     = topo;
            strangle_ = effStrangle;
            warp_     = effWarp;
            drive_    = effDrive;
            bite_     = effBite;
            morph_    = effMorph;
            lastHash_ = 0xFFFFFFFFu;
            updateWavePath();
            repaint();
        } else {
            lastTickMs_ = 0.0;
            const bool wasAnimating = animating_;

            uint32_t hash = static_cast<uint32_t>(family) * 1000003
                          ^ static_cast<uint32_t>(topo) * 500009
                          ^ static_cast<uint32_t>(std::round(morph * 100.0f)) * 31
                          ^ static_cast<uint32_t>(std::round(effStrangle * 100.0f)) * 43
                          ^ static_cast<uint32_t>(static_cast<int32_t>(std::round(effWarp * 100.0f))) * 53
                          ^ static_cast<uint32_t>(std::round(effDrive * 100.0f)) * 67
                          ^ static_cast<uint32_t>(std::round((effBite + 1.0f) * 100.0f));

            if (hash != lastHash_ || wasAnimating) {
                lastHash_ = hash;
                family_   = family;
                morph_    = morph;
                topo_     = topo;
                strangle_ = effStrangle;
                warp_     = effWarp;
                drive_    = effDrive;
                bite_     = effBite;
                updateWavePath();
                repaint();
            }
        }

        if (animate != animating_) {
            animating_ = animate;
            if (isShowing()) {
                startTimerHz(animating_ ? 60 : 30);
            }
        }
    }

    void resized() override {
        updateWavePath();
    }

    void updateWavePath() {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.isEmpty()) return;

        CrackedChimes::DSP::WaveformPreviewEngine::generateCorePreview(
            buffer_, family_, morph_, topo_, strangle_, warp_, drive_, bite_);

        wavePath_.clear();
        wavePath_.preallocateSpace(256);

        const float plotTop = bounds.getY() + 6.0f;
        const float plotH   = bounds.getHeight() - 12.0f;
        const float midY    = bounds.getCentreY();
        const float plotW   = bounds.getWidth() - 16.0f;
        const float leftX   = bounds.getX() + 8.0f;

        const int numPts = static_cast<int>(std::min(plotW, 160.0f));
        const float step = static_cast<float>(CrackedChimes::DSP::WaveformPreviewEngine::kCoreResolution - 1) / static_cast<float>(numPts - 1);

        for (int i = 0; i < numPts; ++i) {
            int srcIdx = static_cast<int>(i * step);
            srcIdx = std::clamp(srcIdx, 0, CrackedChimes::DSP::WaveformPreviewEngine::kCoreResolution - 1);
            float yVal = buffer_[srcIdx];
            float py = midY - yVal * (plotH * 0.40f);
            py = juce::jlimit(plotTop + 1.0f, plotTop + plotH - 1.0f, py);
            float px = leftX + (static_cast<float>(i) / static_cast<float>(numPts - 1)) * plotW;

            if (i == 0) wavePath_.startNewSubPath(px, py);
            else wavePath_.lineTo(px, py);
        }
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();

        // 1. OLED chassis
        g.setColour(juce::Colour(0xFF080C10));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(HasturColours::kDco.withAlpha(0.6f));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        // 2. Reticle grid lines
        g.setColour(juce::Colour(0xFF16202A));
        g.drawHorizontalLine(static_cast<int>(bounds.getCentreY()), bounds.getX() + 6.0f, bounds.getRight() - 6.0f);
        g.drawVerticalLine(static_cast<int>(bounds.getCentreX()), bounds.getY() + 6.0f, bounds.getBottom() - 6.0f);

        // 3. Cyan Glow Bloom + Crisp Neon Core
        g.setColour(HasturColours::kDco.withAlpha(0.25f));
        g.strokePath(wavePath_, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(HasturColours::kDco);
        g.strokePath(wavePath_, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

private:
    HasturAudioProcessor& processor_;
    juce::AudioProcessorValueTreeState& apvts_;
    std::array<float, CrackedChimes::DSP::WaveformPreviewEngine::kCoreResolution> buffer_{};
    juce::Path wavePath_;
    uint32_t lastHash_{0xFFFFFFFF};
    int   family_{0};
    float morph_{0.0f};
    int   topo_{0};
    float strangle_{0.0f};
    float warp_{0.0f};
    float drive_{0.0f};
    float bite_{0.0f};
    double lfoPhase_{0.0};
    double lastTickMs_{0.0};
    bool   animating_{false};
};

class SubWaveformVisualizer : public juce::Component, private juce::Timer {
public:
    explicit SubWaveformVisualizer(HasturAudioProcessor& processor)
        : processor_(processor), apvts_(processor.getAPVTS())
    {
        setOpaque(true);
        startTimerHz(30);
    }

    ~SubWaveformVisualizer() override { stopTimer(); }

    void visibilityChanged() override { updateTimerState(); }
    void parentHierarchyChanged() override { updateTimerState(); }

    void updateTimerState() {
        if (isShowing()) {
            startTimerHz(30);
        } else {
            stopTimer();
        }
    }

    void timerCallback() override {
        auto* octP   = apvts_.getRawParameterValue("sub_octave");
        auto* trainP = apvts_.getRawParameterValue("sub_pulse_train");
        auto* tapP   = apvts_.getRawParameterValue("sub_apf_tap");
        auto* chewP  = apvts_.getRawParameterValue("sub_apf_depth");
        auto* spitP  = apvts_.getRawParameterValue("sub_spit");

        auto* rChewP = apvts_.getRawParameterValue("rand_sub_chew");
        auto* rSpitP = apvts_.getRawParameterValue("rand_sub_spit");

        int   oct   = octP   ? static_cast<int>(octP->load(std::memory_order_relaxed))   : -1;
        int   train = trainP ? static_cast<int>(trainP->load(std::memory_order_relaxed)) : 0;
        int   tap   = tapP   ? static_cast<int>(tapP->load(std::memory_order_relaxed))   : 0;
        float chew  = chewP  ? chewP->load(std::memory_order_relaxed)                    : 0.0f;
        float spit  = spitP  ? spitP->load(std::memory_order_relaxed)                    : 0.0f;

        const float rChewAmt = rChewP ? rChewP->load(std::memory_order_relaxed) : 0.0f;
        const float rSpitAmt = rSpitP ? rSpitP->load(std::memory_order_relaxed) : 0.0f;
        const float rChewVal = processor_.getStepRandAtomic(3);
        const float rSpitVal = processor_.getStepRandAtomic(4);

        const float effChew = (rChewAmt > 0.0f)
            ? (-2.0f + 4.0f * std::clamp((1.0f - rChewAmt) * ((chew + 2.0f) * 0.25f) + rChewAmt * rChewVal, 0.0f, 1.0f))
            : chew;
        const float effSpit = (rSpitAmt > 0.0f)
            ? (-1.0f + 2.0f * std::clamp((1.0f - rSpitAmt) * ((spit + 1.0f) * 0.5f) + rSpitAmt * rSpitVal, 0.0f, 1.0f))
            : spit;

        uint32_t hash = static_cast<uint32_t>(oct + 2) * 1000003
                      ^ static_cast<uint32_t>(train) * 500009
                      ^ static_cast<uint32_t>(tap) * 31
                      ^ static_cast<uint32_t>(static_cast<int32_t>(std::round(effChew * 100.0f))) * 67
                      ^ static_cast<uint32_t>(static_cast<int32_t>(std::round(effSpit * 100.0f))) * 131;

        if (hash != lastHash_) {
            lastHash_ = hash;
            oct_      = oct;
            train_    = train;
            tap_      = tap;
            chew_     = effChew;
            spit_     = effSpit;
            updateWavePath();
            repaint();
        }
    }

    void resized() override {
        updateWavePath();
    }

    void updateWavePath() {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.isEmpty()) return;

        CrackedChimes::DSP::WaveformPreviewEngine::generateSubPreview(
            buffer_, oct_, train_, tap_, chew_, spit_);

        wavePath_.clear();
        wavePath_.preallocateSpace(256);

        const float plotTop = bounds.getY() + 6.0f;
        const float plotH   = bounds.getHeight() - 12.0f;
        const float midY    = bounds.getCentreY();
        const float plotW   = bounds.getWidth() - 16.0f;
        const float leftX   = bounds.getX() + 8.0f;

        const int numPts = static_cast<int>(std::min(plotW, 160.0f));
        const float step = static_cast<float>(CrackedChimes::DSP::WaveformPreviewEngine::kSubResolution - 1) / static_cast<float>(numPts - 1);

        for (int i = 0; i < numPts; ++i) {
            int srcIdx = static_cast<int>(i * step);
            srcIdx = std::clamp(srcIdx, 0, CrackedChimes::DSP::WaveformPreviewEngine::kSubResolution - 1);
            float yVal = buffer_[srcIdx];
            float py = midY - yVal * (plotH * 0.40f);
            py = juce::jlimit(plotTop + 1.0f, plotTop + plotH - 1.0f, py);
            float px = leftX + (static_cast<float>(i) / static_cast<float>(numPts - 1)) * plotW;

            if (i == 0) wavePath_.startNewSubPath(px, py);
            else wavePath_.lineTo(px, py);
        }
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();

        // 1. OLED chassis
        g.setColour(juce::Colour(0xFF080C10));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(HasturColours::kSub.withAlpha(0.6f));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        // 2. Reticle grid lines
        g.setColour(juce::Colour(0xFF221A10));
        g.drawHorizontalLine(static_cast<int>(bounds.getCentreY()), bounds.getX() + 6.0f, bounds.getRight() - 6.0f);
        g.drawVerticalLine(static_cast<int>(bounds.getCentreX()), bounds.getY() + 6.0f, bounds.getBottom() - 6.0f);

        // 3. Tangerine Glow Bloom + Crisp Core
        g.setColour(HasturColours::kSub.withAlpha(0.25f));
        g.strokePath(wavePath_, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(HasturColours::kSub);
        g.strokePath(wavePath_, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

private:
    HasturAudioProcessor& processor_;
    juce::AudioProcessorValueTreeState& apvts_;
    std::array<float, CrackedChimes::DSP::WaveformPreviewEngine::kSubResolution> buffer_{};
    juce::Path wavePath_;
    uint32_t lastHash_{0xFFFFFFFF};
    int   oct_{-1};
    int   train_{0};
    int   tap_{0};
    float chew_{0.0f};
    float spit_{0.0f};
};

class HasturAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit HasturAudioProcessorEditor(HasturAudioProcessor&);
    ~HasturAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    juce::Colour getAccentColour() const noexcept;
    void updateColours();

    static constexpr int kBaseWidth   = 1160;
    static constexpr int kBaseHeight  = 870;
    static constexpr int kMinWidth    = 580;   // 0.50x
    static constexpr int kMinHeight   = 435;
    static constexpr int kMaxWidth    = 1740;  // 1.50x
    static constexpr int kMaxHeight   = 1305;

    class ContentCanvas : public juce::Component {
    public:
        std::function<void(juce::Graphics&)> onPaint;
        void paint(juce::Graphics& g) override {
            if (onPaint) onPaint(g);
        }
    };

private:
    void timerCallback() override;
    void paintCanvas(juce::Graphics& g);
    void layoutControls();

    HasturAudioProcessor& processor_;
    HasturLookAndFeel lnf_;
    juce::ComponentBoundsConstrainer constrainer;
    ContentCanvas contentContainer;

    // Master Header & Backend Toggle
    juce::Label headerLabel;
    juce::Label subheaderLabel;
    PresetStubComponent presetStub;
    RockerToggle3 lifeModeToggle{{"Synthetic", "Cyborg", "Organic"}};

    struct SliderItem {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    struct ComboItem {
        juce::ComboBox combo;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
    };

    // Card 1: DCO & Sub Matrix
    std::unique_ptr<DcoWaveformVisualizer> dcoScope;
    std::unique_ptr<SubWaveformVisualizer> subScope;
    juce::Label dcoFamilyLabel;
    SegmentedButtonBar dcoFamilyButtons{{"SAW", "PULSE", "TRI", "SINE"}};

    juce::Label dcoTopoLabel;
    SegmentedButtonBar dcoTopoButtons{{"OFF", "2x", "4x", "8x"}};

    SliderItem dcoMorphKnob;

    juce::Label subOctaveLabel;
    SegmentedButtonBar subOctaveButtons{{"-2", "-1", "0", "+1"}};

    PulseGridBar subTrainGrid{"PULSE", {"SQR", "PLS", "DUO", "TRI", "QUD"}};

    // Card 2: DCO Hero Engine (Morph, Strangle, Warp, Melt, Bite, Feedback)
    SliderItem osc1MultKnob;  // STRANGLE
    SliderItem osc1ApfKnob;   // WARP
    SliderItem osc1DriveKnob; // MELT
    SliderItem heroBiteKnob;  // BITE
    SliderItem subApfDepthKnob;  // CHEW (Hero 72)
    SliderItem subSpitKnob;      // SPIT (Hero 72)
    juce::Label subApfTapLabel;
    SegmentedButtonBar subApfTapButtons{{"OFF", "4P", "8P", "16P"}};

    SliderItem burbleDepthKnob;  // BURBLE (Hero 72)
    juce::Label burbleModeLabel;
    SegmentedButtonBar burbleModeButtons{{"XFM", "LFM"}};

    // Card 3: Dedicated 4-Lane Vertical Stem Mixer Strip
    SliderItem osc1LevelSlider;   // CORE
    SliderItem subLevelSlider;    // SUB
    SliderItem noiseLevelSlider;  // NOISE
    SliderItem mixerDriveSlider;  // DRIVE

    // Card 4: Dedicated Global Modulation Block
    SliderItem globalWidthKnob;
    SliderItem pwLfoRateKnob;
    SliderItem pwLfoAmountKnob;
    SliderItem lfoAmountKnob; // LFO AMT in Bay 3
    SliderItem lfoRateKnob;
    SliderItem lfoShapeKnob;
    std::unique_ptr<LfoWaveformVisualizer> lfoWaveVisualizer;
    juce::Label lfoWaveLabel;
    SegmentedButtonBar lfoRateModeButtons{{"HZ", "BPM"}};
    juce::Label lfoRateModeLabel;
    SegmentedButtonBar lfoSyncButtons{{"FREE", "RESET"}};
    WaveGrid2x2Bar lfoFamilyGrid{{"TRI", "PULSE", "SINE", "RANDOM"}};
    juce::Label lfoSyncLabel;
    WaveformPickerButton lfoWavePicker;
    SliderItem glideTimeKnob;
    juce::Label glideModeLabel;
    SegmentedButtonBar glideModeButtons{{"ON", "LGTO"}};
    SliderItem oscSlopKnob;

    // Bay 2 (Lower 50%): Dedicated Audio-Rate Modulation Matrix (Filter Mod & Amp Mod)
    Grid2x2Bar filterModSourceGrid{"FM", {"RAW", "CORE", "NOISE"}};
    SliderItem filterModAmountKnob;

    Grid2x2Bar ampModSourceGrid{"AM", {"RAW", "CORE", "SUB"}};
    SliderItem ampModAmountKnob;

    // Bay 4: Reserved Space 3x3 Placeholder Grid (9 Knobs)
    std::array<SliderItem, 9> stepRandKnobs;

    juce::Label trigModeLabel;
    SegmentedButtonBar trigModeButtons{{"RETRIG", "LEGATO", "AUTO"}};
    int lastLfoWaveIdx_{-1};

    // VCF Dynamics & Mod
    SliderItem envDepthKnob;    // Hero 72
    SliderItem accentKnob;      // Hero 72 (filter_velocity)
    SliderItem keyTrackKnob;
    juce::Label filterLfoDestLabel;
    SegmentedButtonBar filterLfoDestButtons{{"CUTOFF", "ORBIT"}};

    // VCF Controls (Dual Roland IR3R05 Extended VCF)
    Grid2x2Bar filterDeckGrid{"DECK", {"DUO-48", "PRE-24", "POST-24"}};
    SliderItem cutoffKnob;     // Hero 72
    SliderItem resKnob;        // Hero 72
    SliderItem filterDecayKnob;// Hero 72 (The Acid Pinch)
    SliderItem filterWarpKnob; // Standard 52
    SliderItem laserBiteKnob;  // Standard 52
    SliderItem laserDriveKnob; // Standard 52
    SliderItem filterOrbitKnob;// Standard 52

    // Bay 6: Master Output Stage ("MAIN")
    SliderItem         pitchBendRangeKnob;
    SliderItem         vibRateKnob;
    SliderItem         vibDepthKnob;
    SliderItem         alphaChorusKnob; // Hero 72 Continuous Macro [0.0, 1.0]
    SliderItem         outputLevelKnob; // Hero 72

    // Bay 5: VCA & Transients
    SliderItem ampVelocityKnob; // Small 52 (Double-clicks to 0.0)
    SliderItem ampSnapKnob;
    SliderItem ampDecayKnob;
    SliderItem ampSustainKnob;
    SliderItem ampReleaseKnob;
    SliderItem ampPunchKnob;
    SliderItem ampTimeKnob;



    // Master Amp Controls
    Grid2x2Bar ampGateGrid{"MODE", {"ENV", "GATE", "DRONE"}};
    std::unique_ptr<EnvelopeVisualizer> ampEnvVisualizer;
    SliderItem postAmpDriveKnob; // Hero 72

    // Active Modal Overlays
    std::unique_ptr<WaveformGridModal> activeWaveModal_;
    std::unique_ptr<KnobTaperModal> activeKnobModal_;

    void setupMixerSlider(SliderItem& item, const juce::String& paramId, const juce::String& name, juce::Colour col);
    void setupRotaryKnob(SliderItem& item, const juce::String& paramId, const juce::String& name, juce::Colour col);
    void setupCombo(ComboItem& item, const juce::String& paramId, const juce::String& name, const juce::StringArray& choices);
    void setupButtonBar(SegmentedButtonBar& bar, juce::Label& label, const juce::String& paramId, const juce::String& name, juce::Colour col);
    void setupButtonBar(SegmentedButtonBar& bar, const juce::String& paramId, juce::Colour col);
    void setupHorizontalSlider(SliderItem& item, const juce::String& paramId, const juce::String& name);
    void setupWaveformPicker(WaveformPickerButton& button, const juce::String& paramId, bool isLfo);

    void openWaveformModal(WaveformPickerButton& button, const juce::String& paramId, bool isLfo);
    void openKnobTaperModal(juce::Slider& slider, const juce::String& paramName);

    int lastLifeMode_{-1};

    // Universal Header Mouse-Over Telemetry Inspector
    HeaderInspector headerInspector;

    struct InspectorBinding {
        juce::String paramId;
        juce::Colour accent;
        juce::Slider* slider{nullptr};
    };
    std::unordered_map<juce::Component*, InspectorBinding> inspectorBindings_;
    juce::Component::SafePointer<juce::Component> lockedComponent_{nullptr};

    void bindInspector(std::initializer_list<juce::Component*> comps, const juce::String& paramId, juce::Colour accent, juce::Slider* s = nullptr);
    juce::Component* resolveInspectorComponent(juce::Component* comp);
    void updateInspectorForComponent(juce::Component* comp, juce::Point<int> localPos);

    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    template <typename ControlType>
    void bindDiscrete(ControlType& control, const juce::String& paramId) {
        if (auto* param = processor_.getAPVTS().getParameter(paramId)) {
            auto att = std::make_unique<DiscreteParamAttachment>(
                *param,
                [&control](float val) {
                    control.setState(juce::roundToInt(val));
                }
            );
            auto* rawAtt = att.get();
            control.onSelect = [rawAtt](int idx) {
                rawAtt->setValueAsCompleteGesture(static_cast<float>(idx));
            };
            discreteAttachments_.push_back(std::move(att));
        }
    }

    template <typename ControlType>
    void bindDiscrete(ControlType& control, const juce::String& paramId,
                      std::function<int(int)> uiToParam,
                      std::function<int(int)> paramToUi,
                      std::function<void(int)> extraCallback = nullptr) {
        if (auto* param = processor_.getAPVTS().getParameter(paramId)) {
            auto att = std::make_unique<DiscreteParamAttachment>(
                *param,
                [&control, paramToUi, extraCallback](float val) {
                    int paramVal = juce::roundToInt(val);
                    int uiVal = paramToUi ? paramToUi(paramVal) : paramVal;
                    control.setState(uiVal);
                    if (extraCallback) extraCallback(paramVal);
                }
            );
            auto* rawAtt = att.get();
            control.onSelect = [rawAtt, uiToParam, extraCallback](int idx) {
                int paramVal = uiToParam ? uiToParam(idx) : idx;
                rawAtt->setValueAsCompleteGesture(static_cast<float>(paramVal));
                if (extraCallback) extraCallback(paramVal);
            };
            discreteAttachments_.push_back(std::move(att));
        }
    }

    void bindDiscreteControls();

    std::vector<std::unique_ptr<DiscreteParamAttachment>> discreteAttachments_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HasturAudioProcessorEditor)
};
