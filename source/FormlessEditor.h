#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "FormlessProcessor.h"
#include "formless_spawn/FormlessControlInfo.h"
#include <vector>
#include <array>
#include <memory>
#include <functional>

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
    juce::Colour accent_{0xFFFFAA00};
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
        juce::Colour accent{0xFFFFAA00};
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
        d.name = "FORMLESS SPAWN";
        d.value = "READY";
        d.blurb = "sovereign 64-model percussion synthesizer (8 roles x 8 models)";
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

        // Col 1: Parameter / System Name (x: 10, w: 100, 12.5pt bold)
        auto nameRect = juce::Rectangle<float>(bounds.getX() + 10.0f, bounds.getY(), 100.0f, bounds.getHeight());
        g.setFont(juce::Font(juce::FontOptions(12.5f).withStyle("Bold")));
        g.setColour(readout_.accent);
        g.drawText(readout_.name, nameRect, juce::Justification::centredLeft, true);

        // Col 2: Explanation blurb across the full remaining area (x: 116, w: bounds.getWidth() - 122.0f, 2 lines)
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

        g.setFont(juce::Font(juce::FontOptions(11.5f).withStyle("Bold")));
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
// FormlessRoleGrid2x4: 2x4 Role Matrix (8 Roles: BD, SD, TM, CP / HH, CY, PC, FX)
// ==============================================================================
class FormlessRoleGrid2x4 : public juce::Component {
public:
    FormlessRoleGrid2x4() = default;

    void setState(int roleIdx) {
        roleIdx = std::clamp(roleIdx, 0, tkdsp::FormlessSpawnVoice::kTotalRoles - 1);
        if (activeRole_ != roleIdx) {
            activeRole_ = roleIdx;
            repaint();
        }
    }

    int getState() const noexcept { return activeRole_; }
    std::function<void(int)> onSelect;
    std::function<void(int)> onHover;
    std::function<void()> onHoverExit;

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.isEmpty()) return;

        g.setColour(juce::Colour(0xFF161B22));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF30363D));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        const float colW = bounds.getWidth() / 4.0f;
        const float rowH = bounds.getHeight() / 2.0f;

        // Grid Dividers
        g.setColour(juce::Colour(0xFF262D38));
        g.drawHorizontalLine(static_cast<int>(std::round(bounds.getY() + rowH)), bounds.getX() + 3.0f, bounds.getRight() - 3.0f);
        for (int c = 1; c < 4; ++c) {
            float divX = bounds.getX() + c * colW;
            g.drawVerticalLine(static_cast<int>(std::round(divX)), bounds.getY() + 3.0f, bounds.getBottom() - 3.0f);
        }

        g.setFont(juce::Font(juce::FontOptions(13.0f).withStyle("Bold")));
        for (int r = 0; r < tkdsp::FormlessSpawnVoice::kTotalRoles; ++r) {
            int row = r / 4;
            int col = r % 4;
            auto cellRect = juce::Rectangle<float>(bounds.getX() + col * colW, bounds.getY() + row * rowH, colW, rowH);
            bool isActive = (r == activeRole_);

            if (isActive) {
                g.setColour(juce::Colour(0xFFFF9D00)); // Gold/Amber fill
                g.fillRoundedRectangle(cellRect.reduced(1.0f), 3.0f);
                g.setColour(juce::Colour(0xFF0D1117));
            } else {
                g.setColour(juce::Colour(0xFFF0F6FC));
            }

            g.drawText(tkdsp::FormlessSpawnVoice::kRoleNames[r], cellRect, juce::Justification::centred);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        int r = getRoleAt(e.position);
        if (r >= 0 && r != activeRole_) {
            activeRole_ = r;
            if (onSelect) onSelect(activeRole_);
            repaint();
        }
    }

    void mouseMove(const juce::MouseEvent& e) override {
        int r = getRoleAt(e.position);
        if (r >= 0) {
            if (onHover) onHover(r);
        } else {
            if (onHoverExit) onHoverExit();
        }
    }

    void mouseExit(const juce::MouseEvent&) override {
        if (onHoverExit) onHoverExit();
    }

private:
    int getRoleAt(juce::Point<float> pos) const {
        const float colW = getWidth() / 4.0f;
        const float rowH = getHeight() / 2.0f;
        int col = std::clamp(static_cast<int>(pos.x / colW), 0, 3);
        int row = std::clamp(static_cast<int>(pos.y / rowH), 0, 1);
        return row * 4 + col;
    }

    int activeRole_{0};
};

// ==============================================================================
// FormlessModelGrid2x8: 2x4 Physical Model Matrix (8 Tactile Buttons, Zero Menus)
// ==============================================================================
class FormlessModelGrid2x8 : public juce::Component {
public:
    FormlessModelGrid2x8() = default;

    void setRoleAndModel(int roleIdx, int modelIdx) {
        roleIdx = std::clamp(roleIdx, 0, tkdsp::FormlessSpawnVoice::kTotalRoles - 1);
        modelIdx = std::clamp(modelIdx, 0, tkdsp::FormlessSpawnVoice::kModelsPerRole - 1);
        if (activeRole_ != roleIdx || activeModel_ != modelIdx) {
            activeRole_ = roleIdx;
            activeModel_ = modelIdx;
            repaint();
        }
    }

    void setModel(int modelIdx) {
        modelIdx = std::clamp(modelIdx, 0, tkdsp::FormlessSpawnVoice::kModelsPerRole - 1);
        if (activeModel_ != modelIdx) {
            activeModel_ = modelIdx;
            repaint();
        }
    }

    int getModel() const noexcept { return activeModel_; }
    std::function<void(int)> onSelect;
    std::function<void(int, const juce::String&)> onHover;
    std::function<void()> onHoverExit;

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.isEmpty()) return;

        g.setColour(juce::Colour(0xFF161B22));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF30363D));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        const float colW = bounds.getWidth() / 4.0f;
        const float rowH = bounds.getHeight() / 2.0f;

        // Dividers
        g.setColour(juce::Colour(0xFF262D38));
        g.drawHorizontalLine(static_cast<int>(std::round(bounds.getY() + rowH)), bounds.getX() + 3.0f, bounds.getRight() - 3.0f);
        for (int c = 1; c < 4; ++c) {
            float divX = bounds.getX() + c * colW;
            g.drawVerticalLine(static_cast<int>(std::round(divX)), bounds.getY() + 3.0f, bounds.getBottom() - 3.0f);
        }

        g.setFont(juce::Font(juce::FontOptions(12.0f).withStyle("Bold")));

        for (int m = 0; m < tkdsp::FormlessSpawnVoice::kModelsPerRole; ++m) {
            int row = m / 4;
            int col = m % 4;
            auto cellRect = juce::Rectangle<float>(bounds.getX() + col * colW, bounds.getY() + row * rowH, colW, rowH);
            bool isActive = (m == activeModel_);

            if (isActive) {
                g.setColour(juce::Colour(0xFF00C8FF)); // Cyan active
                g.fillRoundedRectangle(cellRect.reduced(1.0f), 3.0f);
                g.setColour(juce::Colour(0xFF0D1117));
            } else {
                g.setColour(juce::Colour(0xFFF0F6FC));
            }

            const char* name = tkdsp::FormlessSpawnVoice::kModelNames[activeRole_][m];
            g.drawText(name, cellRect, juce::Justification::centred);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        int m = getModelAt(e.position);
        if (m >= 0 && m != activeModel_) {
            activeModel_ = m;
            if (onSelect) onSelect(activeModel_);
            repaint();
        }
    }

    void mouseMove(const juce::MouseEvent& e) override {
        int m = getModelAt(e.position);
        if (m >= 0) {
            if (onHover) onHover(m, tkdsp::FormlessSpawnVoice::kModelNames[activeRole_][m]);
        } else {
            if (onHoverExit) onHoverExit();
        }
    }

    void mouseExit(const juce::MouseEvent&) override {
        if (onHoverExit) onHoverExit();
    }

private:
    int getModelAt(juce::Point<float> pos) const {
        const float colW = getWidth() / 4.0f;
        const float rowH = getHeight() / 2.0f;
        int col = std::clamp(static_cast<int>(pos.x / colW), 0, 3);
        int row = std::clamp(static_cast<int>(pos.y / rowH), 0, 1);
        return row * 4 + col;
    }

    int activeRole_{0};
    int activeModel_{0};
};

// ==============================================================================
// FormlessVerticalBar: Vertical 2-Stack (DIODE / LPG)
// ==============================================================================
class FormlessVerticalBar : public juce::Component {
public:
    FormlessVerticalBar(const std::array<juce::String, 2>& labels) : labels_(labels) {}

    void setAccentColour(juce::Colour c) {
        accent_ = c;
        repaint();
    }

    void setState(int idx) {
        idx = std::clamp(idx, 0, 1);
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

        const float rowH = bounds.getHeight() * 0.5f;
        auto activeRect = juce::Rectangle<float>(bounds.getX(), bounds.getY() + activeIndex_ * rowH, bounds.getWidth(), rowH);
        g.setColour(accent_);
        g.fillRoundedRectangle(activeRect.reduced(1.0f), 3.0f);

        g.setColour(juce::Colour(0xFF262D38));
        g.drawHorizontalLine(static_cast<int>(std::round(bounds.getY() + rowH)), bounds.getX() + 3.0f, bounds.getRight() - 3.0f);

        g.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
        for (int i = 0; i < 2; ++i) {
            bool isActive = (i == activeIndex_);
            auto r = juce::Rectangle<float>(bounds.getX(), bounds.getY() + i * rowH, bounds.getWidth(), rowH);
            g.setColour(isActive ? juce::Colour(0xFF0D1117) : juce::Colour(0xFFF0F6FC));
            g.drawText(labels_[i], r, juce::Justification::centred);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        const float rowH = getHeight() * 0.5f;
        int clickedIdx = std::clamp(static_cast<int>(e.position.y / rowH), 0, 1);
        if (clickedIdx != activeIndex_) {
            activeIndex_ = clickedIdx;
            if (onSelect) onSelect(activeIndex_);
            repaint();
        }
    }

private:
    std::array<juce::String, 2> labels_;
    int activeIndex_{0};
    juce::Colour accent_{0xFF6BBF59};
};

// ==============================================================================
// FormlessGrid2x2: 2x2 Mojo Profile Selector (CLEAN, MD-12, SP-12, IRON)
// ==============================================================================
class FormlessGrid2x2 : public juce::Component {
public:
    FormlessGrid2x2(const std::array<juce::String, 4>& labels) : labels_(labels) {}

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
            auto cellRect = juce::Rectangle<float>(bounds.getX() + col * colW, bounds.getY() + row * rowH, colW, rowH);
            bool isActive = (i == activeIndex_);

            if (isActive) {
                g.setColour(accent_);
                g.fillRoundedRectangle(cellRect.reduced(1.0f), 3.0f);
                g.setColour(juce::Colour(0xFF0D1117));
            } else {
                g.setColour(juce::Colour(0xFFF0F6FC));
            }

            g.drawText(labels_[i], cellRect, juce::Justification::centred);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        const float colW = getWidth() * 0.5f;
        const float rowH = getHeight() * 0.5f;
        int col = std::clamp(static_cast<int>(e.position.x / colW), 0, 1);
        int row = std::clamp(static_cast<int>(e.position.y / rowH), 0, 1);
        int clickedIdx = row * 2 + col;
        if (clickedIdx != activeIndex_) {
            activeIndex_ = clickedIdx;
            if (onSelect) onSelect(activeIndex_);
            repaint();
        }
    }

private:
    std::array<juce::String, 4> labels_;
    int activeIndex_{0};
    juce::Colour accent_{0xFFFF5252};
};

// ==============================================================================
// TactileTriggerPad: Center Altar Drum Trigger with Tactile Gold Flash
// ==============================================================================
class TactileTriggerPad : public juce::Component {
public:
    TactileTriggerPad() = default;

    std::function<void()> onTrigger;
    std::function<void()> onRightTrigger;

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

    void mouseUp(const juce::MouseEvent&) override {
        isDown_ = false;
        isRightClick_ = false;
        repaint();
    }

    void mouseExit(const juce::MouseEvent&) override {
        if (isDown_) {
            isDown_ = false;
            isRightClick_ = false;
            repaint();
        }
    }

private:
    bool isDown_{false};
    bool isRightClick_{false};
};

// ==============================================================================
// Segmented Button Bar Component (for Rand Mode)
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

        g.setFont(juce::Font(juce::FontOptions(11.5f).withStyle("Bold")));
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
                                             static_cast<float>(width), static_cast<float>(height)).reduced(3.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        auto centre = bounds.getCentre();
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        const float sweep = rotaryEndAngle - rotaryStartAngle;

        const bool compact = radius < 26.0f; // Small-knob mode for 2x4 grid (44px knobs)
        const bool hasMod = slider.getProperties().contains("modLo");
        const bool hasRand = slider.getProperties().contains("randLo");
        const bool dualRadar = hasMod && hasRand;

        // Concentric Dual-Radar Radial Offsets
        float hrYellow, hrOrange, trackRadius, dialRadius;
        float yellowGlow, yellowCore, yellowPipR;
        float orangeGlow, orangeCore, orangePipR;
        float trackStroke;

        if (compact) {
            if (dualRadar) {
                hrYellow    = radius - 1.0f;
                yellowGlow  = 2.5f; yellowCore = 1.4f; yellowPipR = 1.8f;
                hrOrange    = radius - 3.2f;
                orangeGlow  = 2.2f; orangeCore = 1.4f; orangePipR = 1.6f;
                trackRadius = radius - 5.5f;
                trackStroke = 2.5f;
                dialRadius  = radius - 7.5f;
            } else {
                hrYellow    = radius - 1.0f;
                yellowGlow  = 3.0f; yellowCore = 1.6f; yellowPipR = 2.0f;
                hrOrange    = hrYellow;
                orangeGlow  = yellowGlow; orangeCore = yellowCore; orangePipR = yellowPipR;
                trackRadius = radius - 3.5f;
                trackStroke = 3.0f;
                dialRadius  = radius - 7.0f;
            }
        } else {
            if (dualRadar) {
                hrYellow    = radius - 1.2f;
                yellowGlow  = 4.0f; yellowCore = 2.0f; yellowPipR = 2.2f;
                hrOrange    = radius - 4.2f;
                orangeGlow  = 3.5f; orangeCore = 1.8f; orangePipR = 2.0f;
                trackRadius = radius - 7.2f;
                trackStroke = 3.5f;
                dialRadius  = radius - 9.8f;
            } else {
                hrYellow    = radius - 1.5f;
                yellowGlow  = 4.5f; yellowCore = 2.0f; yellowPipR = 2.5f;
                hrOrange    = hrYellow;
                orangeGlow  = yellowGlow; orangeCore = yellowCore; orangePipR = yellowPipR;
                trackRadius = radius - 4.0f;
                trackStroke = 4.0f;
                dialRadius  = radius - 8.0f;
            }
        }

        // 1. Background track ring
        g.setColour(juce::Colour(0xFF21262D));
        juce::Path trackPath;
        trackPath.addCentredArc(centre.x, centre.y, trackRadius, trackRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.strokePath(trackPath, juce::PathStrokeType(trackStroke, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // 2. Active modulation arc with Bipolar Mid-Point Glow
        auto fillCol = slider.findColour(juce::Slider::rotarySliderFillColourId);
        g.setColour(fillCol);
        juce::Path activePath;
        const bool propBipolar = slider.getProperties().contains("isBipolar") && static_cast<bool>(slider.getProperties()["isBipolar"]);
        const bool rangeBipolar = (slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0);
        const bool isBipolar = rangeBipolar || propBipolar;

        if (isBipolar) {
            float zeroNorm = 0.5f;
            if (rangeBipolar) {
                zeroNorm = static_cast<float>(-slider.getMinimum() / (slider.getMaximum() - slider.getMinimum()));
            }
            const float midAngle = rotaryStartAngle + zeroNorm * sweep;
            if (std::abs(angle - midAngle) > 0.005f) {
                if (angle > midAngle) {
                    activePath.addCentredArc(centre.x, centre.y, trackRadius, trackRadius, 0.0f, midAngle, angle, true);
                } else {
                    activePath.addCentredArc(centre.x, centre.y, trackRadius, trackRadius, 0.0f, angle, midAngle, true);
                }
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

        // =====================================================================
        // VISUAL MODULATION RADAR TELEMETRY (Concentric Source-Color Arc Halos & Pips)
        // =====================================================================
        // Inner Ring: Macro Vector Modulation Halos (Movie Poster Orange 0xFFFF7B25)
        if (hasMod) {
            const float lo = static_cast<float>(slider.getProperties()["modLo"]);
            const float hi = static_cast<float>(slider.getProperties()["modHi"]);
            if (hi - lo > 0.002f) {
                const auto orangeCol = juce::Colour(0xFFFF7B25); // Movie Poster Orange

                juce::Path arc;
                arc.addCentredArc(centre.x, centre.y, hrOrange, hrOrange, 0.0f,
                                  rotaryStartAngle + lo * sweep, rotaryStartAngle + hi * sweep, true);
                g.setColour(orangeCol.withAlpha(0.28f));
                g.strokePath(arc, juce::PathStrokeType(orangeGlow, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                g.setColour(orangeCol.withAlpha(0.95f));
                g.strokePath(arc, juce::PathStrokeType(orangeCore, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

                if (slider.getProperties().contains("modPip")) {
                    const float pipNorm = static_cast<float>(slider.getProperties()["modPip"]);
                    const float pipAngle = rotaryStartAngle + pipNorm * sweep;
                    const float px = centre.x + hrOrange * std::sin(pipAngle);
                    const float py = centre.y - hrOrange * std::cos(pipAngle);

                    g.setColour(juce::Colours::white);
                    g.fillEllipse(px - orangePipR, py - orangePipR, 2.0f * orangePipR, 2.0f * orangePipR);
                    g.setColour(orangeCol);
                    g.drawEllipse(px - orangePipR - 1.0f, py - orangePipR - 1.0f, 2.0f * orangePipR + 2.0f, 2.0f * orangePipR + 2.0f, 1.0f);
                }
            }
        }

        // Outer Ring: Stepped Random & Velocity Dynamic Halos (Caustic Hastur Yellow 0xFFF2E10F)
        if (hasRand) {
            const float lo = static_cast<float>(slider.getProperties()["randLo"]);
            const float hi = static_cast<float>(slider.getProperties()["randHi"]);
            if (hi - lo > 0.002f) {
                const auto yellowCol = juce::Colour(0xFFF2E10F); // Caustic Hastur Yellow

                juce::Path arc;
                arc.addCentredArc(centre.x, centre.y, hrYellow, hrYellow, 0.0f,
                                  rotaryStartAngle + lo * sweep, rotaryStartAngle + hi * sweep, true);
                g.setColour(yellowCol.withAlpha(0.25f));
                g.strokePath(arc, juce::PathStrokeType(yellowGlow, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                g.setColour(yellowCol.withAlpha(0.95f));
                g.strokePath(arc, juce::PathStrokeType(yellowCore, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

                if (slider.getProperties().contains("randPip")) {
                    const float pipNorm = static_cast<float>(slider.getProperties()["randPip"]);
                    const float pipAngle = rotaryStartAngle + pipNorm * sweep;
                    const float px = centre.x + hrYellow * std::sin(pipAngle);
                    const float py = centre.y - hrYellow * std::cos(pipAngle);

                    g.setColour(juce::Colours::white);
                    g.fillEllipse(px - yellowPipR, py - yellowPipR, 2.0f * yellowPipR, 2.0f * yellowPipR);
                    g.setColour(yellowCol);
                    g.drawEllipse(px - yellowPipR - 1.0f, py - yellowPipR - 1.0f, 2.0f * yellowPipR + 2.0f, 2.0f * yellowPipR + 2.0f, 1.0f);
                }
            }
        }

        // =====================================================================
        // KILOHEARTS IN-DIAL VALUE BADGE (Ephemerally Rendered On Interaction)
        // =====================================================================
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

    static void drawKiloheartsValueBadge(juce::Graphics& g, const juce::Rectangle<float>& bounds,
                                         const juce::String& text, juce::Colour accentColour) {
        if (text.isEmpty()) return;
        const auto font = juce::Font(juce::FontOptions(10.5f).withStyle("Bold"));
        const float textW = juce::GlyphArrangement::getStringWidth(font, text);
        const float badgeW = std::clamp(textW + 10.0f, 26.0f, bounds.getWidth() - 6.0f);
        const float badgeH = 14.0f;
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
// Formless Spawn AudioProcessorEditor
// ==============================================================================
// ==============================================================================
// LimiterTactileButton: Tactile Limit Button with 3-State Hardware Jewel LED Pip
// ==============================================================================
class LimiterTactileButton : public juce::Button {
public:
    LimiterTactileButton() : juce::Button("LIMIT") {
        setClickingTogglesState(true);
    }

    void updateTelemetry(float gr) noexcept {
        if (std::abs(gr - currentGr_) > 0.004f) {
            currentGr_ = gr;
            repaint();
        }
    }

    void paintButton(juce::Graphics& g, bool /*shouldDrawButtonAsHighlighted*/, bool /*shouldDrawButtonAsDown*/) override {
        auto bounds = getLocalBounds().toFloat();
        const bool on = getToggleState();

        // 1. Tactile Bezel / Chassis
        const auto bgCol = on ? juce::Colour(0xFF21262D) : juce::Colour(0xFF161B22);
        g.setColour(bgCol);
        g.fillRoundedRectangle(bounds, 3.0f);

        // Border outline
        const auto borderCol = on ? juce::Colour(0xFF484F58) : juce::Colour(0xFF30363D);
        g.setColour(borderCol);
        g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.0f);

        // 2. Hardware Jewel LED Pip (5px circular lens at x=7, centered vertically)
        const float ledX = 7.0f;
        const float ledY = bounds.getCentreY() - 2.5f;
        const juce::Rectangle<float> ledRect(ledX, ledY, 5.0f, 5.0f);

        // Outer cathode ring
        g.setColour(juce::Colour(0xFF0D1117));
        g.fillEllipse(ledRect.expanded(1.0f));

        if (!on) {
            // Unlit dark cathode
            g.setColour(juce::Colour(0xFF21262D));
            g.fillEllipse(ledRect);
        } else {
            // Active Reactive Emitter:
            // Green: GR == 0 dB (gr > 0.9441)
            // Yellow: 0.5 dB <= GR < 4 dB (0.6310 <= gr <= 0.9441)
            // Red: GR >= 4 dB (gr < 0.6310)
            juce::Colour ledCol;
            juce::Colour glowCol;
            if (currentGr_ > 0.9441f) {
                ledCol = juce::Colour(0xFF39D353); // Bright Green
                glowCol = juce::Colour(0x662EA043);
            } else if (currentGr_ >= 0.6310f) {
                ledCol = juce::Colour(0xFFF3CA3E); // Amber / Yellow
                glowCol = juce::Colour(0x66D29922);
            } else {
                ledCol = juce::Colour(0xFFFF3B30); // Blistering Red
                glowCol = juce::Colour(0x88DA3633);
            }

            // Radial glow halo
            g.setColour(glowCol);
            g.fillEllipse(ledRect.expanded(2.5f));

            // Core bright emitter lens
            g.setColour(ledCol);
            g.fillEllipse(ledRect);

            // Specular micro-pip
            g.setColour(juce::Colours::white.withAlpha(0.6f));
            g.fillEllipse(juce::Rectangle<float>(ledX + 1.0f, ledY + 1.0f, 1.5f, 1.5f));
        }

        // 3. Typography Label "LIMIT"
        const auto textCol = on ? juce::Colours::white : juce::Colour(0xFF8B949E);
        g.setColour(textCol);
        g.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
        const juce::Rectangle<float> labelArea(15.0f, 0.0f, bounds.getWidth() - 17.0f, bounds.getHeight());
        g.drawText("LIMIT", labelArea, juce::Justification::centred, false);
    }

private:
    float currentGr_{1.0f};
};

class FormlessSpawnAudioProcessorEditor : public juce::AudioProcessorEditor,
                                          public juce::Timer {
public:
    explicit FormlessSpawnAudioProcessorEditor(FormlessSpawnAudioProcessor&);
    ~FormlessSpawnAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;

private:
    struct KnobItem {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    void setupKnob(KnobItem& item, const juce::String& paramId, const juce::String& name,
                   const juce::String& unit, const juce::String& blurb,
                   juce::Colour col, bool isBipolar = false);

    FormlessSpawnAudioProcessor& processor_;
    FormlessLookAndFeel lnf_;

    juce::Component::SafePointer<juce::Slider> activeBadgeSlider_;
    juce::int64 activeBadgeExpiry_{0};

    // Header Row 1
    PresetStubComponent presetStub_;
    HeaderInspector     headerInspector_;
    FormlessRocker3     lifeformRocker_{{"SYNTHETIC", "CYBORG", "ORGANIC"}};

    // Card 0: Voice Navigator & Model Matrix
    FormlessRoleGrid2x4  roleGrid_;
    FormlessModelGrid2x8 modelGrid_;

    // Card 1 Knobs: Master Macro & Dynamics Lab (Top-Left, 8)
    KnobItem macroXKnob_;
    KnobItem macroYKnob_;
    KnobItem macroZKnob_;
    KnobItem macroTKnob_;
    KnobItem velAggrKnob_;
    KnobItem velTimbKnob_;
    KnobItem panKnob_;
    KnobItem decayCurveKnob_;

    // Card 2 Knobs: Role Synthesis Lab (Top-Right, 8)
    KnobItem paramAKnob_;
    KnobItem paramBKnob_;
    KnobItem paramCKnob_;
    KnobItem paramDKnob_;
    KnobItem paramEKnob_;
    KnobItem paramFKnob_;
    KnobItem paramGKnob_;
    KnobItem paramHKnob_;

    // Card 3 Pod 1: Stepped Random (2x4 Grid of 8 Small Knobs, Bottom-Left)
    KnobItem randXKnob_;
    KnobItem randYKnob_;
    KnobItem randZKnob_;
    KnobItem randTKnob_;
    KnobItem randAggrKnob_;
    KnobItem randTimbKnob_;
    KnobItem randPanKnob_;
    KnobItem randDecayKnob_;

    void updateRoleParamLabels(int roleIdx);
    void updateFilterTypeUI(int filterIdx);

    // Card 3 Pod 2: The Performance Altar (EXACTLY TWO CONTROLS)
    KnobItem          macroDecayKnob_;
    TactileTriggerPad trigButton_;

    // Card 3 Pod 3: VCF, Universal Mojo Strip & Master Output (10 knobs + 1 2x2 grid + limiter toggle + volume)
    KnobItem            cutoffKnob_;
    KnobItem            resonanceKnob_;
    KnobItem            filterEnvAmtKnob_;
    KnobItem            filterDecayKnob_;
    KnobItem            preDriveKnob_;
    KnobItem            crushKnob_;
    KnobItem            shiftKnob_;
    KnobItem            squashKnob_;
    KnobItem            widerKnob_;
    LimiterTactileButton limiterButton_;
    float               guiLimiterGr_{1.0f};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> limiterAttachment_;
    KnobItem            volumeKnob_;
    KnobItem            ampDecayKnob_;
    FormlessGrid2x2     filterTypeBar_{{"LP", "HP", "BP", "PK"}};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FormlessSpawnAudioProcessorEditor)
};
