#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "DigiStrikeFS2Processor.h"
#include "DigiStrikeFS2ControlInfo.h"
#include "../dsp/common/SteppedRandom.h"
#include "../dsp/drums/FormlessModMath.h"
#include <vector>
#include <array>
#include <memory>
#include <functional>
#include <unordered_map>

// ==============================================================================
// DigiStrikeFS2Editor.h — Tactical Console GUI for DigiStrike FS2
// ==============================================================================
// Sovereign Tactical Console matching Formless Spawn visual & tactile standards.
// Part of The Klang Suite SOCEP Engine. Copyright (c) 2026 Cracked Chimes.
// ==============================================================================

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
        g.setColour(juce::Colour(0xFF0F131A));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF262D38));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        g.setFont(juce::Font(juce::FontOptions(12.0f).withStyle("Bold")));
        g.setColour(accent_.withAlpha(0.90f));
        g.drawText("<  01: DEFAULT VOID  >", bounds, juce::Justification::centred);
    }

private:
    juce::Colour accent_{0xFFFFAA00};
};

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
// 3-Way Rocker Toggle (Lifeform Triad / Voice Mode)
// ==============================================================================
class DigiStrikeRocker3 : public juce::Component {
public:
    DigiStrikeRocker3(const std::array<juce::String, 3>& labels) : labels_(labels) {}

    void setColours(const std::array<juce::Colour, 3>& c) {
        colours_ = c;
        repaint();
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

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xFF161B22));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF30363D));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        const float segW = bounds.getWidth() / 3.0f;
        auto activeRect = juce::Rectangle<float>(bounds.getX() + activeIndex_ * segW, bounds.getY(), segW, bounds.getHeight());
        g.setColour(colours_[static_cast<size_t>(activeIndex_)]);
        g.fillRoundedRectangle(activeRect.reduced(1.0f), 3.0f);

        g.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
        for (int i = 0; i < 3; ++i) {
            bool isActive = (i == activeIndex_);
            auto segRect = juce::Rectangle<float>(bounds.getX() + static_cast<float>(i) * segW, bounds.getY(), segW, bounds.getHeight());
            g.setColour(isActive ? juce::Colour(0xFF0D1117) : juce::Colour(0xFFF0F6FC));
            g.drawText(labels_[static_cast<size_t>(i)], segRect, juce::Justification::centred);
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
    int activeIndex_{0};
    std::array<juce::Colour, 3> colours_{
        juce::Colour(0xFF00C8FF), // Synthetic
        juce::Colour(0xFFE08C00), // Cyborg
        juce::Colour(0xFF6BBF59)  // Organic
    };
};

// ==============================================================================
// DigiStrikeSegmentedBar: Multi-Segment Tactile Button Matrix Strip
// ==============================================================================
class DigiStrikeSegmentedBar : public juce::Component {
public:
    DigiStrikeSegmentedBar(const std::vector<juce::String>& labels) : labels_(labels) {}

    void setState(int idx) {
        idx = std::clamp(idx, 0, static_cast<int>(labels_.size()) - 1);
        if (activeIndex_ != idx) {
            activeIndex_ = idx;
            repaint();
        }
    }

    int getState() const noexcept { return activeIndex_; }
    std::function<void(int)> onSelect;

    void setAccentColour(juce::Colour c) {
        if (accentColour_ != c) {
            accentColour_ = c;
            repaint();
        }
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xFF161B22));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF30363D));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        const int numSegs = static_cast<int>(labels_.size());
        if (numSegs <= 0) return;

        const float segW = bounds.getWidth() / static_cast<float>(numSegs);
        auto activeRect = juce::Rectangle<float>(bounds.getX() + static_cast<float>(activeIndex_) * segW, bounds.getY(), segW, bounds.getHeight());
        g.setColour(accentColour_);
        g.fillRoundedRectangle(activeRect.reduced(1.0f), 3.0f);

        // Inner vertical dividers between inactive segments
        g.setColour(juce::Colour(0xFF262D38));
        for (int i = 1; i < numSegs; ++i) {
            if (i != activeIndex_ && i != activeIndex_ + 1) {
                float divX = bounds.getX() + static_cast<float>(i) * segW;
                g.drawVerticalLine(static_cast<int>(std::round(divX)), bounds.getY() + 3.0f, bounds.getBottom() - 3.0f);
            }
        }

        g.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
        for (int i = 0; i < numSegs; ++i) {
            bool isActive = (i == activeIndex_);
            auto segRect = juce::Rectangle<float>(bounds.getX() + static_cast<float>(i) * segW, bounds.getY(), segW, bounds.getHeight());
            g.setColour(isActive ? juce::Colour(0xFF0D1117) : juce::Colour(0xFF8B949E));
            g.drawText(labels_[static_cast<size_t>(i)], segRect, juce::Justification::centred);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        const int numSegs = static_cast<int>(labels_.size());
        if (numSegs <= 0) return;
        const float segW = static_cast<float>(getWidth()) / static_cast<float>(numSegs);
        int clickedIdx = std::clamp(static_cast<int>(e.position.x / segW), 0, numSegs - 1);
        activeIndex_ = clickedIdx;
        if (onSelect) onSelect(activeIndex_);
        repaint();
    }

private:
    std::vector<juce::String> labels_;
    int activeIndex_{0};
    juce::Colour accentColour_{0xFFFFAA00};
};

// ==============================================================================
// DigiStrikeMultiSelectBar: Sovereign Independent Lineage Multi-Select Strip (2x10 Grid)
// ==============================================================================
class DigiStrikeMultiSelectBar : public juce::Component {
public:
    DigiStrikeMultiSelectBar(const std::vector<juce::String>& labels) : labels_(labels) {}

    void setMask(uint32_t mask) {
        if (mask_ != mask) {
            mask_ = mask;
            repaint();
        }
    }

    uint32_t getMask() const noexcept { return mask_; }

    void setAccentColour(juce::Colour c) {
        if (accentColour_ != c) {
            accentColour_ = c;
            repaint();
        }
    }

    void updateMorphDNA(uint32_t mask, int category, float morphNorm) {
        morphNorm = std::clamp(morphNorm, 0.0f, 1.0f);
        category = std::clamp(category, 0, 7);

        int lin0 = -1;
        int lin1 = -1;
        float w0 = 0.0f;
        float w1 = 0.0f;

        if (mask != 0) {
            std::array<DigiStrikeFS2Catalog::WaveCoord, 64> activeWaves{};
            int activeCount = 0;
            for (int i = 0; i < 80; ++i) {
                const auto coord = DigiStrikeFS2Catalog::kMusicalOrder[static_cast<size_t>(category)][static_cast<size_t>(i)];
                if ((mask & (1u << coord.lineage)) != 0) {
                    if (activeCount < 64) {
                        activeWaves[static_cast<size_t>(activeCount++)] = coord;
                    }
                }
            }

            if (activeCount == 1) {
                lin0 = activeWaves[0].lineage;
                w0 = 1.0f;
            } else if (activeCount > 1) {
                const float span = static_cast<float>(activeCount - 1);
                const float pos = std::clamp(morphNorm * span, 0.0f, span);
                const int i0 = static_cast<int>(pos);
                const int i1 = std::min(i0 + 1, activeCount - 1);
                const float frac = pos - static_cast<float>(i0);
                const float smoothFrac = frac * frac * (3.0f - 2.0f * frac);

                lin0 = activeWaves[static_cast<size_t>(i0)].lineage;
                lin1 = activeWaves[static_cast<size_t>(i1)].lineage;
                w0 = 1.0f - smoothFrac;
                w1 = smoothFrac;
            }
        }

        if (morphLin0_ != lin0 || morphLin1_ != lin1 ||
            std::abs(morphWeight0_ - w0) > 0.01f || std::abs(morphWeight1_ - w1) > 0.01f) {
            morphLin0_ = lin0;
            morphLin1_ = lin1;
            morphWeight0_ = w0;
            morphWeight1_ = w1;
            repaint();
        }
    }

    std::function<void(uint32_t)> onMaskChange;
    std::function<void(int hoveredIdx)> onHover;
    std::function<void(const juce::String&)> onBudgetLimit;

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.isEmpty()) return;

        const int totalButtons = static_cast<int>(labels_.size());
        if (totalButtons <= 0) return;

        const bool isSecret = (mask_ == 0);

        if (isSecret) {
            // Secret Mode: Glowing Neon Magenta Chasm
            g.setColour(juce::Colour(0xFF190A1A));
            g.fillRoundedRectangle(bounds, 4.0f);

            // Pulsing Neon Magenta Border (0xFFFF007F)
            g.setColour(juce::Colour(0xFFFF007F));
            g.drawRoundedRectangle(bounds, 4.0f, 1.5f);

            // Secret Core Glow Text
            g.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
            g.setColour(juce::Colour(0xFFFF55B2));
            g.drawText(juce::CharPointer_UTF8("\xe2\x98\x85  SECRET CORE ENGAGED: PURE SINE -> HYPER-SAW  [CLICK ANY BUTTON TO EXIT]  \xe2\x98\x85"),
                       bounds, juce::Justification::centred);
            return;
        }

        // Count active lineages to determine 64-wave budget limit (max 16 lineages)
        int activeLineageCount = 0;
        for (int i = 0; i < totalButtons; ++i) {
            if ((mask_ & (1u << i)) != 0) ++activeLineageCount;
        }
        const bool isBudgetFull = (activeLineageCount >= 16);

        // Render 2x10 Grid (80x22px buttons, 4px gaps)
        const int cols = 10;
        const int rows = 2;
        const float gapX = 4.0f;
        const float gapY = 4.0f;
        const float btnW = (bounds.getWidth() - static_cast<float>(cols - 1) * gapX) / static_cast<float>(cols);
        const float btnH = (bounds.getHeight() - static_cast<float>(rows - 1) * gapY) / static_cast<float>(rows);

        for (int i = 0; i < totalButtons; ++i) {
            const int r = i / cols;
            const int c = i % cols;
            const auto btnRect = juce::Rectangle<float>(
                bounds.getX() + static_cast<float>(c) * (btnW + gapX),
                bounds.getY() + static_cast<float>(r) * (btnH + gapY),
                btnW, btnH);

            const bool isOn = (mask_ & (1u << i)) != 0;
            const bool isGreyedOut = !isOn && isBudgetFull;

            if (isOn) {
                // Active button pill
                g.setColour(accentColour_);
                g.fillRoundedRectangle(btnRect, 3.0f);

                // Morph DNA Halo highlight
                if (i == morphLin0_ || i == morphLin1_) {
                    float w = (i == morphLin0_) ? morphWeight0_ : morphWeight1_;
                    auto haloBar = btnRect.reduced(2.0f, 1.0f).removeFromTop(3.0f);
                    g.setColour(juce::Colour(0xFF00FF88).withAlpha(0.6f + 0.4f * w));
                    g.fillRoundedRectangle(haloBar, 1.5f);

                    g.setColour(juce::Colour(0xFFFFFFFF).withAlpha(0.4f * w));
                    g.drawRoundedRectangle(btnRect.reduced(0.5f), 3.0f, 1.0f);
                }

                g.setFont(juce::Font(juce::FontOptions(9.5f).withStyle("Bold")));
                g.setColour(juce::Colour(0xFF0D1117));
                g.drawText(labels_[static_cast<size_t>(i)], btnRect, juce::Justification::centred);
            } else if (isGreyedOut) {
                // Greyed out / Budget locked button
                g.setColour(juce::Colour(0xFF0F1218));
                g.fillRoundedRectangle(btnRect, 3.0f);
                g.setColour(juce::Colour(0xFF1F2633));
                g.drawRoundedRectangle(btnRect.reduced(0.5f), 3.0f, 0.8f);

                g.setFont(juce::Font(juce::FontOptions(9.5f)));
                g.setColour(juce::Colour(0xFF484F58));
                g.drawText(labels_[static_cast<size_t>(i)], btnRect, juce::Justification::centred);
            } else {
                // Normal inactive button
                g.setColour(juce::Colour(0xFF161B22));
                g.fillRoundedRectangle(btnRect, 3.0f);
                g.setColour(juce::Colour(0xFF30363D));
                g.drawRoundedRectangle(btnRect.reduced(0.5f), 3.0f, 0.8f);

                g.setFont(juce::Font(juce::FontOptions(9.5f).withStyle("Bold")));
                g.setColour(juce::Colour(0xFF8B949E));
                g.drawText(labels_[static_cast<size_t>(i)], btnRect, juce::Justification::centred);
            }
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        const int totalButtons = static_cast<int>(labels_.size());
        if (totalButtons <= 0) return;

        auto bounds = getLocalBounds().toFloat();
        const int cols = 10;
        const int rows = 2;
        const float gapX = 4.0f;
        const float gapY = 4.0f;
        const float btnW = (bounds.getWidth() - static_cast<float>(cols - 1) * gapX) / static_cast<float>(cols);
        const float btnH = (bounds.getHeight() - static_cast<float>(rows - 1) * gapY) / static_cast<float>(rows);

        int clickedIdx = -1;
        for (int i = 0; i < totalButtons; ++i) {
            const int r = i / cols;
            const int c = i % cols;
            const auto btnRect = juce::Rectangle<float>(
                bounds.getX() + static_cast<float>(c) * (btnW + gapX),
                bounds.getY() + static_cast<float>(r) * (btnH + gapY),
                btnW, btnH);
            if (btnRect.contains(e.position)) {
                clickedIdx = i;
                break;
            }
        }
        if (clickedIdx < 0 || clickedIdx >= totalButtons) return;

        int activeLineageCount = 0;
        for (int i = 0; i < totalButtons; ++i) {
            if ((mask_ & (1u << i)) != 0) ++activeLineageCount;
        }

        if (e.getNumberOfClicks() >= 2) {
            // Double-Click: Reset to default K-Works (Kawai)
            mask_ = 0x00000010;
        } else if (e.mods.isPopupMenu() || e.mods.isAltDown()) {
            // Solo clicked lineage (or reset to K-Works if already soloed)
            uint32_t soloMask = (1u << clickedIdx);
            if (mask_ == soloMask) {
                mask_ = 0x00000010; // Back to K-Works
            } else {
                mask_ = soloMask;
            }
        } else {
            // Standard click toggle
            if (mask_ == 0) {
                mask_ = (1u << clickedIdx);
            } else {
                bool wasOn = (mask_ & (1u << clickedIdx)) != 0;
                if (wasOn) {
                    mask_ &= ~(1u << clickedIdx);
                } else {
                    if (activeLineageCount < 16) {
                        mask_ |= (1u << clickedIdx);
                    } else {
                        if (onBudgetLimit) onBudgetLimit("64/64 WAVES LATCHED (DESELECT TO SWAP)");
                        return;
                    }
                }
            }
        }

        if (onMaskChange) onMaskChange(mask_);
        repaint();
    }

    void mouseMove(const juce::MouseEvent& e) override {
        const int totalButtons = static_cast<int>(labels_.size());
        if (totalButtons <= 0) return;
        auto bounds = getLocalBounds().toFloat();
        const int cols = 10;
        const int rows = 2;
        const float gapX = 4.0f;
        const float gapY = 4.0f;
        const float btnW = (bounds.getWidth() - static_cast<float>(cols - 1) * gapX) / static_cast<float>(cols);
        const float btnH = (bounds.getHeight() - static_cast<float>(rows - 1) * gapY) / static_cast<float>(rows);

        int foundIdx = -1;
        for (int i = 0; i < totalButtons; ++i) {
            const int r = i / cols;
            const int c = i % cols;
            const auto btnRect = juce::Rectangle<float>(
                bounds.getX() + static_cast<float>(c) * (btnW + gapX),
                bounds.getY() + static_cast<float>(r) * (btnH + gapY),
                btnW, btnH);
            if (btnRect.contains(e.position)) {
                foundIdx = i;
                break;
            }
        }
        if (foundIdx != hoveredIdx_) {
            hoveredIdx_ = foundIdx;
            if (onHover) onHover(hoveredIdx_);
        }
    }

    void mouseExit(const juce::MouseEvent& /*e*/) override {
        hoveredIdx_ = -1;
        if (onHover) onHover(-1);
    }

    int getHoveredIndex() const noexcept { return hoveredIdx_; }

private:
    std::vector<juce::String> labels_;
    uint32_t mask_{0x00000010};
    juce::Colour accentColour_{0xFF00C8FF};

    int morphLin0_{-1};
    int morphLin1_{-1};
    float morphWeight0_{0.0f};
    float morphWeight1_{0.0f};
    int hoveredIdx_{-1};
};

// ==============================================================================
// TactileTriggerPad: Center Altar Drum Trigger with Tactile Gold Flash
// ==============================================================================
class DigiStrikeTriggerPad : public juce::Component {
public:
    DigiStrikeTriggerPad() = default;

    std::function<void()> onTrigger;
    std::function<void()> onRelease;
    std::function<void()> onRightTrigger;
    std::function<void()> onRightRelease;

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        if (isDown_) {
            g.setColour(isRightClick_ ? juce::Colour(0xFF00E676) : juce::Colour(0xFFFFAA00));
            g.fillRoundedRectangle(bounds, 5.0f);
            g.setFont(juce::Font(juce::FontOptions(13.0f).withStyle("Bold")));
            g.setColour(juce::Colour(0xFF0D1117));
            g.drawText(isRightClick_ ? "C3" : "TRIG", bounds, juce::Justification::centred);
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

    void mouseDown(const juce::MouseEvent& e) override {
        isDown_ = true;
        isRightClick_ = e.mods.isRightButtonDown();
        repaint();
        if (isRightClick_) {
            if (onRightTrigger) onRightTrigger();
        } else {
            if (onTrigger) onTrigger();
        }
    }

    void mouseUp(const juce::MouseEvent& e) override {
        juce::ignoreUnused(e);
        if (isDown_) {
            isDown_ = false;
            repaint();
            if (isRightClick_) {
                if (onRightRelease) onRightRelease();
            } else {
                if (onRelease) onRelease();
            }
            isRightClick_ = false;
        }
    }

    void mouseExit(const juce::MouseEvent& e) override {
        juce::ignoreUnused(e);
        if (isDown_) {
            isDown_ = false;
            repaint();
            if (isRightClick_) {
                if (onRightRelease) onRightRelease();
            } else {
                if (onRelease) onRelease();
            }
            isRightClick_ = false;
        }
    }

private:
    bool isDown_{false};
    bool isRightClick_{false};
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
// DigiStrikeGrid2x2: 2x2 Filter Type Selector (LP, HP, BP, PK)
// ==============================================================================
class DigiStrikeGrid2x2 : public juce::Component {
public:
    DigiStrikeGrid2x2(const std::array<juce::String, 4>& labels) : labels_(labels) {}

    void setAccentColour(juce::Colour c) {
        accent_ = c;
        repaint();
    }

    void setState(int idx) {
        idx = std::clamp(idx, 0, 3);
        if (activeIndex_ != idx) {
            activeIndex_ = idx;
            repaint();
        }
    }

    int getState() const noexcept { return activeIndex_; }
    std::function<void(int)> onSelect;

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.isEmpty()) return;

        g.setColour(juce::Colour(0xFF161B22));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF30363D));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        const float colW = bounds.getWidth() * 0.5f;
        const float rowH = bounds.getHeight() * 0.5f;

        // Dividers
        g.setColour(juce::Colour(0xFF262D38));
        g.drawHorizontalLine(static_cast<int>(std::round(bounds.getY() + rowH)), bounds.getX() + 3.0f, bounds.getRight() - 3.0f);
        g.drawVerticalLine(static_cast<int>(std::round(bounds.getX() + colW)), bounds.getY() + 3.0f, bounds.getBottom() - 3.0f);

        g.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
        for (int i = 0; i < 4; ++i) {
            int row = i / 2;
            int col = i % 2;
            auto cellRect = juce::Rectangle<float>(bounds.getX() + static_cast<float>(col) * colW, bounds.getY() + static_cast<float>(row) * rowH, colW, rowH);
            bool isActive = (i == activeIndex_);

            if (isActive) {
                g.setColour(accent_);
                g.fillRoundedRectangle(cellRect.reduced(1.0f), 3.0f);
                g.setColour(juce::Colour(0xFF0D1117));
            } else {
                g.setColour(juce::Colour(0xFFF0F6FC));
            }

            g.drawText(labels_[static_cast<size_t>(i)], cellRect, juce::Justification::centred);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        const float colW = static_cast<float>(getWidth()) * 0.5f;
        const float rowH = static_cast<float>(getHeight()) * 0.5f;
        int col = std::clamp(static_cast<int>(e.position.x / colW), 0, 1);
        int row = std::clamp(static_cast<int>(e.position.y / rowH), 0, 1);
        int idx = row * 2 + col;
        setState(idx);
        if (onSelect) onSelect(idx);
    }

private:
    std::array<juce::String, 4> labels_;
    int activeIndex_{0};
    juce::Colour accent_{0xFF00E676};
};

// ==============================================================================
// DigiStrike LookAndFeel (Concentric Dual-Radar Halo Mechanics matching Formless Spawn)
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
                                             static_cast<float>(width), static_cast<float>(height)).reduced(3.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        auto centre = bounds.getCentre();
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        const float sweep = rotaryEndAngle - rotaryStartAngle;

        // Background Track Cavity
        juce::Path bgTrack;
        bgTrack.addCentredArc(centre.x, centre.y, radius - 3.0f, radius - 3.0f, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colour(0xFF1F242E));
        g.strokePath(bgTrack, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // 1. Trickle-Down Ecoknobics Halo Arc (Inner Track Arc - Electric Cyan)
        if (slider.getProperties().contains("modLo") && slider.getProperties().contains("modHi")) {
            const float mLo = std::clamp(static_cast<float>(slider.getProperties()["modLo"]), 0.0f, 1.0f);
            const float mHi = std::clamp(static_cast<float>(slider.getProperties()["modHi"]), 0.0f, 1.0f);
            if (mHi > mLo + 0.001f) {
                juce::Path modArc;
                const float aLo = rotaryStartAngle + mLo * sweep;
                const float aHi = rotaryStartAngle + mHi * sweep;
                modArc.addCentredArc(centre.x, centre.y, radius - 3.0f, radius - 3.0f, 0.0f, aLo, aHi, true);
                g.setColour(juce::Colour(0xFF00E5FF).withAlpha(0.55f));
                g.strokePath(modArc, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
            if (slider.getProperties().contains("modPip")) {
                const float mPip = std::clamp(static_cast<float>(slider.getProperties()["modPip"]), 0.0f, 1.0f);
                const float aPip = rotaryStartAngle + mPip * sweep;
                const float pipR = radius - 3.0f;
                const float pipX = centre.x + pipR * std::sin(aPip);
                const float pipY = centre.y - pipR * std::cos(aPip);
                g.setColour(juce::Colour(0xFF00E5FF));
                g.fillEllipse(pipX - 2.5f, pipY - 2.5f, 5.0f, 5.0f);
            }
        }

        // 2. Stepped Random Range Halo (Outer Track Arc - Caustic Yellow)
        if (slider.getProperties().contains("randLo") && slider.getProperties().contains("randHi")) {
            const float rLo = static_cast<float>(slider.getProperties()["randLo"]);
            const float rHi = static_cast<float>(slider.getProperties()["randHi"]);
            if (rHi > rLo + 0.001f) {
                juce::Path randArc;
                const float aLo = rotaryStartAngle + rLo * sweep;
                const float aHi = rotaryStartAngle + rHi * sweep;
                randArc.addCentredArc(centre.x, centre.y, radius + 2.5f, radius + 2.5f, 0.0f, aLo, aHi, true);
                g.setColour(juce::Colour(0xFFF2E10F).withAlpha(0.40f));
                g.strokePath(randArc, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        }

        // Active Value Arc
        juce::Colour baseColour = slider.findColour(juce::Slider::rotarySliderFillColourId);
        juce::Path valArc;
        valArc.addCentredArc(centre.x, centre.y, radius - 3.0f, radius - 3.0f, 0.0f, rotaryStartAngle, angle, true);
        g.setColour(baseColour);
        g.strokePath(valArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Knob Dial Faceplate
        const float knobRadius = radius - 8.0f;
        g.setColour(juce::Colour(0xFF13171F));
        g.fillEllipse(centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);
        g.setColour(juce::Colour(0xFF262D38));
        g.drawEllipse(centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.2f);

        // Indicator Needle
        juce::Path needle;
        needle.startNewSubPath(centre.x, centre.y);
        needle.lineTo(centre.x + (knobRadius - 2.0f) * std::sin(angle), centre.y - (knobRadius - 2.0f) * std::cos(angle));
        g.setColour(baseColour.brighter(0.2f));
        g.strokePath(needle, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // =========================================================================
        // KILOHEARTS IN-DIAL VALUE BADGE (Ephemerally Rendered On Interaction)
        // =========================================================================
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
            drawKiloheartsValueBadge(g, bounds, text, baseColour);
        }
    }

    static void drawKiloheartsValueBadge(juce::Graphics& g, const juce::Rectangle<float>& bounds,
                                         const juce::String& text, juce::Colour accentColour) {
        if (text.isEmpty()) return;
        const bool isCompact = bounds.getWidth() < 50.0f;
        const auto font = juce::Font(juce::FontOptions(isCompact ? 9.0f : 10.5f).withStyle("Bold"));
        const float textW = juce::GlyphArrangement::getStringWidth(font, text);
        const float badgeW = std::clamp(textW + (isCompact ? 6.0f : 10.0f), 22.0f, bounds.getWidth() - 4.0f);
        const float badgeH = isCompact ? 12.0f : 14.0f;
        const float badgeX = bounds.getCentreX() - badgeW * 0.5f;
        const float badgeY = bounds.getBottom() - badgeH - 1.0f;
        const auto badgeRect = juce::Rectangle<float>(badgeX, badgeY, badgeW, badgeH);

        // Obsidian Badge Background
        g.setColour(juce::Colour(0xFA090D12));
        g.fillRoundedRectangle(badgeRect, 3.5f);

        // Subtle Accent-tinted Border
        g.setColour(accentColour.withAlpha(0.65f));
        g.drawRoundedRectangle(badgeRect, 3.5f, 1.0f);

        // Crisp White Text
        g.setFont(font);
        g.setColour(juce::Colour(0xFFF0F6FC));
        g.drawFittedText(text, badgeRect.toNearestInt(), juce::Justification::centred, 1);
    }
};

// ==============================================================================
// DigiStrikeFS2AudioProcessorEditor: Master Workstation Console
// ==============================================================================
class DigiStrikeFS2AudioProcessorEditor : public juce::AudioProcessorEditor,
                                          private juce::Timer
{
public:
    static constexpr int kBaseWidth = 900;
    static constexpr int kBaseHeight = 564;

    explicit DigiStrikeFS2AudioProcessorEditor(DigiStrikeFS2AudioProcessor&);
    ~DigiStrikeFS2AudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;

    struct KnobItem {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    struct InspectorBinding {
        juce::String paramId;
        juce::Colour accent;
        juce::Slider* slider{nullptr};
        juce::String customName{};
        juce::String customBlurb{};
    };

private:
    void setupKnob(KnobItem& item, const juce::String& paramId, const juce::String& name,
                   const juce::String& unit, const juce::String& blurb,
                   juce::Colour col, bool isBipolar = false);

    void registerInspector(juce::Component* comp, const juce::String& paramId, juce::Colour accent,
                           juce::Slider* s = nullptr, const juce::String& cName = {}, const juce::String& cBlurb = {});
    juce::Component* resolveInspectorComponent(juce::Component* comp);
    void updateInspectorForComponent(juce::Component* comp);

    DigiStrikeFS2AudioProcessor& processor_;
    DigiStrikeLookAndFeel lnf_;

    std::unordered_map<juce::Component*, InspectorBinding> inspectorBindings_;
    juce::Component::SafePointer<juce::Component> lockedComponent_;
    juce::Component::SafePointer<juce::Slider> activeBadgeSlider_;
    juce::int64 activeBadgeExpiry_{0};

    // Header Row
    PresetStubComponent presetStub_;
    DigiStrikeHeaderInspector headerInspector_;
    DigiStrikeRocker3 lifeformRocker_{{"SYNTHETIC", "CYBORG", "ORGANIC"}};

    // Card 1 Knobs: Master Macro & Dynamics (Top-Left, 8)
    KnobItem macroXKnob_;
    KnobItem macroYKnob_;
    KnobItem macroZKnob_;
    KnobItem macroTKnob_;
    KnobItem velAggrKnob_;
    KnobItem velTimbKnob_;
    KnobItem panKnob_;
    KnobItem decayCurveKnob_;

    // Card 2: Uniform Sound Lab (Top-Right)
    // 8 Knobs (Upper Bay)
    KnobItem pitchDecayKnob_;
    KnobItem clickKnob_;
    KnobItem startKnob_;
    KnobItem timeKnob_;
    KnobItem punchKnob_;
    KnobItem noiseKnob_;
    KnobItem lengthKnob_;
    KnobItem morphKnob_;

    // Bottom Tactile Matrix (Lower Bay)
    DigiStrikeSegmentedBar categoryGrid_{{"KICK", "SNAR", "TOMS", "CLAP", "HATS", "CYMB", "PERC", "STAB"}};
    DigiStrikeMultiSelectBar lineageGrid_{{
        "Crown", "Apex", "Empyrean", "Falcon", "K-Works", "Compact", "Tokyo", "Gothenburg", "Circuits", "Phase",
        "Studio", "Keio", "Nippon", "Adios", "Fairlight", "Synclavier", "Ensoniq", "Simmons", "Jomox", "Curios"
    }};

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
    DigiStrikeGrid2x2 filterTypeBar_{{"LP", "HP", "BP", "PK"}};
    void updateFilterTypeUI(int filterIdx);

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
    KnobItem glideTimeKnob_;
    DigiStrikeRocker3 voiceModeRocker_{{"MONO", "GLIDE", "POLY"}};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DigiStrikeFS2AudioProcessorEditor)
};
