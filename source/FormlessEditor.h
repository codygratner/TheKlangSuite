#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "FormlessProcessor.h"
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
        d.blurb = "sovereign 112-model percussion synthesizer";
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

        // Col 1: Parameter / System Name (x: 10, w: 105, 12.5pt bold)
        auto nameRect = juce::Rectangle<float>(bounds.getX() + 10.0f, bounds.getY(), 105.0f, bounds.getHeight());
        g.setFont(juce::Font(juce::FontOptions(12.5f).withStyle("Bold")));
        g.setColour(readout_.accent);
        g.drawText(readout_.name, nameRect, juce::Justification::centredLeft, true);

        // Col 2: Formatted Value (x: 118, w: 72, 12.5pt bold)
        auto valRect = juce::Rectangle<float>(bounds.getX() + 118.0f, bounds.getY(), 72.0f, bounds.getHeight());
        g.setFont(juce::Font(juce::FontOptions(12.5f).withStyle("Bold")));
        g.setColour(juce::Colour(0xFFF0F6FC));
        g.drawText(readout_.value, valRect, juce::Justification::centredLeft, true);

        // Col 3: Separator (x: 192, w: 12)
        auto sepRect = juce::Rectangle<float>(bounds.getX() + 192.0f, bounds.getY(), 12.0f, bounds.getHeight());
        g.setFont(juce::Font(juce::FontOptions(12.5f)));
        g.setColour(juce::Colour(0xFF8B949E));
        g.drawText(juce::CharPointer_UTF8("\xe2\x80\x94"), sepRect, juce::Justification::centred);

        // Col 4: Explanation blurb (x: 206, w: rest)
        float blurbW = std::max(50.0f, bounds.getWidth() - 214.0f);
        auto blurbRect = juce::Rectangle<float>(bounds.getX() + 206.0f, bounds.getY(), blurbW, bounds.getHeight());
        g.setFont(juce::Font(juce::FontOptions(12.0f)));
        g.setColour(readout_.isDefault ? juce::Colour(0xFF6E7681) : juce::Colour(0xFFC9D1D9));
        g.drawText(readout_.blurb, blurbRect, juce::Justification::centredLeft, true);
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

        g.setFont(juce::Font(juce::FontOptions(12.0f).withStyle("Bold")));
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
                                             static_cast<float>(width), static_cast<float>(height)).reduced(2.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        auto centre = bounds.getCentre();
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        const float sweep = rotaryEndAngle - rotaryStartAngle;

        const bool compact = radius < 26.0f; // Small-knob mode for 2x4 grid (44px knobs)
        const float trackRadius = radius - (compact ? 3.5f : 4.0f);
        const float trackStroke = compact ? 3.0f : 4.0f;

        // Background track ring
        g.setColour(juce::Colour(0xFF21262D));
        juce::Path trackPath;
        trackPath.addCentredArc(centre.x, centre.y, trackRadius, trackRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.strokePath(trackPath, juce::PathStrokeType(trackStroke, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

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

        // Dial body
        auto dialRadius = radius - (compact ? 7.0f : 8.0f);
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
        // VISUAL MODULATION RADAR TELEMETRY (Source-Color Arc Halos & Pips)
        // =====================================================================
        // 1. Macro Vector Modulation Halos (Movie Poster Orange 0xFFFF7B25)
        if (slider.getProperties().contains("modLo")) {
            const float lo = static_cast<float>(slider.getProperties()["modLo"]);
            const float hi = static_cast<float>(slider.getProperties()["modHi"]);
            if (hi - lo > 0.002f) {
                const float hr = radius - (compact ? 0.5f : 1.5f);
                const float glow = compact ? 3.0f : 4.5f;
                const float core = compact ? 1.6f : 2.0f;
                const auto orangeCol = juce::Colour(0xFFFF7B25); // Movie Poster Orange

                juce::Path arc;
                arc.addCentredArc(centre.x, centre.y, hr, hr, 0.0f,
                                  rotaryStartAngle + lo * sweep, rotaryStartAngle + hi * sweep, true);
                g.setColour(orangeCol.withAlpha(0.28f));
                g.strokePath(arc, juce::PathStrokeType(glow, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                g.setColour(orangeCol.withAlpha(0.95f));
                g.strokePath(arc, juce::PathStrokeType(core, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

                if (slider.getProperties().contains("modPip")) {
                    const float pipNorm = static_cast<float>(slider.getProperties()["modPip"]);
                    const float pipAngle = rotaryStartAngle + pipNorm * sweep;
                    const float px = centre.x + hr * std::sin(pipAngle);
                    const float py = centre.y - hr * std::cos(pipAngle);
                    const float pr = compact ? 2.0f : 2.5f;

                    g.setColour(juce::Colours::white);
                    g.fillEllipse(px - pr, py - pr, 2.0f * pr, 2.0f * pr);
                    g.setColour(orangeCol);
                    g.drawEllipse(px - pr - 1.0f, py - pr - 1.0f, 2.0f * pr + 2.0f, 2.0f * pr + 2.0f, 1.0f);
                }
            }
        }

        // 2. Stepped Random Humanization Halos (Caustic Hastur Yellow 0xFFF2E10F)
        if (slider.getProperties().contains("randLo")) {
            const float lo = static_cast<float>(slider.getProperties()["randLo"]);
            const float hi = static_cast<float>(slider.getProperties()["randHi"]);
            if (hi - lo > 0.002f) {
                const float hr = radius - (compact ? 0.5f : 1.5f);
                const float glow = compact ? 3.0f : 4.5f;
                const float core = compact ? 1.6f : 2.0f;
                const auto yellowCol = juce::Colour(0xFFF2E10F); // Caustic Hastur Yellow

                juce::Path arc;
                arc.addCentredArc(centre.x, centre.y, hr, hr, 0.0f,
                                  rotaryStartAngle + lo * sweep, rotaryStartAngle + hi * sweep, true);
                g.setColour(yellowCol.withAlpha(0.25f));
                g.strokePath(arc, juce::PathStrokeType(glow, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                g.setColour(yellowCol.withAlpha(0.95f));
                g.strokePath(arc, juce::PathStrokeType(core, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

                if (slider.getProperties().contains("randPip")) {
                    const float pipNorm = static_cast<float>(slider.getProperties()["randPip"]);
                    const float pipAngle = rotaryStartAngle + pipNorm * sweep;
                    const float px = centre.x + hr * std::sin(pipAngle);
                    const float py = centre.y - hr * std::cos(pipAngle);
                    const float pr = compact ? 2.0f : 2.5f;

                    g.setColour(juce::Colours::white);
                    g.fillEllipse(px - pr, py - pr, 2.0f * pr, 2.0f * pr);
                    g.setColour(yellowCol);
                    g.drawEllipse(px - pr - 1.0f, py - pr - 1.0f, 2.0f * pr + 2.0f, 2.0f * pr + 2.0f, 1.0f);
                }
            }
        }
    }
};

// ==============================================================================
// Formless Spawn AudioProcessorEditor
// ==============================================================================
class FormlessSpawnAudioProcessorEditor : public juce::AudioProcessorEditor,
                                          public juce::Timer {
public:
    explicit FormlessSpawnAudioProcessorEditor(FormlessSpawnAudioProcessor&);
    ~FormlessSpawnAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

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

    // Header Row 1
    PresetStubComponent presetStub_;
    HeaderInspector     headerInspector_;
    FormlessRocker3     lifeformRocker_{{"SYN", "CYB", "ORG"}};

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

    // Card 3 Pod 2: The Performance Altar (EXACTLY TWO CONTROLS)
    KnobItem          macroDecayKnob_;
    TactileTriggerPad trigButton_;

    // Card 3 Pod 3: VCF, Universal Mojo Strip & Master Output (6 knobs + 2 2x2 grids + master knob)
    KnobItem            cutoffKnob_;
    KnobItem            resonanceKnob_;
    KnobItem            preDriveKnob_;
    KnobItem            crushKnob_;
    KnobItem            bitsKnob_;
    KnobItem            tapeGlueKnob_;
    KnobItem            volumeKnob_;
    FormlessGrid2x2     filterTypeBar_{{"LP", "HP", "BP", "BR"}};
    FormlessGrid2x2     mojoProfileBar_{{"CLEAN", "MD-12", "SP-12", "IRON"}};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FormlessSpawnAudioProcessorEditor)
};
