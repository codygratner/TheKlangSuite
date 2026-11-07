#pragma once



#include <juce_gui_basics/juce_gui_basics.h>

#include <juce_audio_processors/juce_audio_processors.h>

#include "NyarlatProcessor.h"

#include "NyarlatAdditiveOscillator.h"

#include "NyarlatControlInfo.h"

#include "NyarlatZPlaneRom.h"

#include "NyarlatRom.h"

#include "../ui/EditorScaleManager.h"

#include "../ui/CrackedChimesUI.h"

#include "../dsp/common/SteppedRandom.h"

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
        juce::String blurb;

        juce::Colour accent{0xFF00C8F0};

        bool isDefault{false};

    };



    HeaderInspector() {

        setInterceptsMouseClicks(false, false);

        setReadoutDefault();

    }



    void setReadout(Readout r) {

        if (readout_.name == r.name &&

            readout_.blurb == r.blurb && readout_.accent == r.accent &&

            readout_.isDefault == r.isDefault)

            return;



        readout_ = std::move(r);

        repaint();

    }



    void setReadoutDefault() {

        Readout d;

        d.name = "NYARLAT";

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
        auto nameRect = juce::Rectangle<float>(bounds.getX() + 10.0f, bounds.getY(), 110.0f, bounds.getHeight());
        g.setFont(juce::Font(juce::FontOptions(13.0f).withStyle("Bold")));
        g.setColour(readout_.accent);
        g.drawText(readout_.name, nameRect, juce::Justification::centredLeft, true);

        // Subtle separator
        g.setColour(juce::Colour(0xFF262D38).withAlpha(0.65f));
        g.drawVerticalLine(juce::roundToInt(bounds.getX() + 124.0f), bounds.getY() + 5.0f, bounds.getBottom() - 5.0f);

        // Col 2: Explanation blurb across the full remaining area (2 lines)
        float blurbW = std::max(50.0f, bounds.getWidth() - 138.0f);
        auto blurbRect = juce::Rectangle<float>(bounds.getX() + 130.0f, bounds.getY(), blurbW, bounds.getHeight());

        g.setFont(juce::Font(juce::FontOptions(11.5f)));

        g.setColour(readout_.isDefault ? juce::Colour(0xFF6E7681) : juce::Colour(0xFFC9D1D9));

        g.drawFittedText(readout_.blurb, blurbRect.toNearestInt(), juce::Justification::centredLeft, 2);

    }



private:

    Readout readout_;

};



// ==============================================================================

// SegmentedRadio: Generic N-segment radio control with ParameterAttachment

// ==============================================================================

class SegmentedRadio : public juce::Component {

public:

    void configure(const juce::StringArray& labels, juce::Colour accent, int seamPx = 2, float fontSize = 10.0f) {

        labels_ = labels;

        accent_ = accent;

        seam_ = seamPx;

        fontSize_ = fontSize;

        repaint();

    }



    void setIndex(int idx, juce::NotificationType n = juce::sendNotificationSync) {

        idx = std::clamp(idx, 0, juce::jmax(0, labels_.size() - 1));

        if (idx == active_) return;

        active_ = idx;

        repaint();

        if (n != juce::dontSendNotification && onChange) onChange(active_);

    }



    int getIndex() const noexcept { return active_; }



    juce::String getValueText() const {

        return valueText ? valueText(active_) : (active_ >= 0 && active_ < labels_.size() ? labels_[active_] : "");

    }



    juce::Rectangle<int> segment(int i) const {

        const int n = labels_.size();

        if (n <= 0) return {};

        const int usable = getWidth() - (n - 1) * seam_;

        const int x0 = (i * usable) / n + i * seam_;

        const int x1 = ((i + 1) * usable) / n + i * seam_;

        return { x0, 0, x1 - x0, getHeight() };

    }



    void paint(juce::Graphics& g) override {

        const auto font = juce::Font(juce::FontOptions(fontSize_).withStyle("Bold"));

        g.setFont(font);

        for (int i = 0; i < labels_.size(); ++i) {

            const auto seg = segment(i);

            const bool on = (i == active_);

            g.setColour(on ? accent_ : juce::Colour(0xFF161B22));

            g.fillRoundedRectangle(seg.toFloat(), 3.0f);

            g.setColour(juce::Colour(0xFF30363D));

            g.drawRoundedRectangle(seg.toFloat().reduced(0.5f), 3.0f, 1.0f);

            g.setColour(on ? juce::Colour(0xFF0D1117) : juce::Colour(0xFF8B949E));

            g.drawFittedText(labels_[i], seg.reduced(3, 0), juce::Justification::centred, 1, 0.85f);

        }

    }



    void mouseDown(const juce::MouseEvent& e) override {

        if (labels_.isEmpty()) return;

        const int n = labels_.size();

        for (int i = 0; i < n; ++i) {

            if (segment(i).contains(e.getPosition())) {

                setIndex(i);

                break;

            }

        }

    }



    std::function<void(int)> onChange;

    std::function<juce::String(int)> valueText;



private:

    juce::StringArray labels_;

    juce::Colour accent_{0xFF00C8F0};

    int seam_{2};

    int active_{0};

    float fontSize_{10.0f};

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
// Icons: Vector glyphs for 2x2 oscillator grid (Padlock, Triangle, Saw Up/Down)
// ==============================================================================
namespace Icons {

inline juce::Path sawUp() {
    juce::Path p;
    p.startNewSubPath(0.12f, 0.78f);
    p.lineTo(0.50f, 0.22f);
    p.lineTo(0.50f, 0.78f);
    p.lineTo(0.88f, 0.22f);
    return p;
}

inline juce::Path triangle() {
    juce::Path p;
    p.startNewSubPath(0.10f, 0.78f);
    p.lineTo(0.30f, 0.22f);
    p.lineTo(0.50f, 0.78f);
    p.lineTo(0.70f, 0.22f);
    p.lineTo(0.90f, 0.78f);
    return p;
}

inline juce::Path sawDown() {
    juce::Path p;
    p.startNewSubPath(0.12f, 0.22f);
    p.lineTo(0.50f, 0.78f);
    p.lineTo(0.50f, 0.22f);
    p.lineTo(0.88f, 0.78f);
    return p;
}

inline juce::Path padlockBody() {
    juce::Path p;
    p.addRoundedRectangle(0.20f, 0.44f, 0.60f, 0.46f, 0.08f);
    return p;
}

inline juce::Path shackleClosed() {
    juce::Path p;
    p.startNewSubPath(0.32f, 0.44f);
    p.lineTo(0.32f, 0.28f);
    p.cubicTo(0.32f, 0.08f, 0.68f, 0.08f, 0.68f, 0.28f);
    p.lineTo(0.68f, 0.44f);
    return p;
}

inline juce::Path shackleOpen() {
    juce::Path p;
    p.startNewSubPath(0.32f, 0.44f);
    p.lineTo(0.32f, 0.26f);
    p.cubicTo(0.32f, 0.04f, 0.68f, 0.04f, 0.68f, 0.18f);
    p.lineTo(0.68f, 0.24f);
    return p;
}

} // namespace Icons

// ==============================================================================
// VectorIconButton: Large tactile square button rendering crisp vector iconography
// ==============================================================================
class VectorIconButton : public juce::Button {
public:
    enum class IconType {
        Lock,
        Triangle,
        SawUp,
        SawDown
    };

    VectorIconButton(const juce::String& name, IconType type, juce::Colour accentCol)
        : juce::Button(name), type_(type), accentCol_(accentCol) {
        setClickingTogglesState(true);
    }

    void setAccentColour(juce::Colour c) { accentCol_ = c; repaint(); }

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
        auto bounds = getLocalBounds().toFloat().reduced(0.5f);
        if (bounds.isEmpty()) return;

        const bool on = getToggleState();

        if (type_ == IconType::Lock) {
            juce::Colour bg = shouldDrawButtonAsDown ? juce::Colour(0xFF262D38)
                            : (shouldDrawButtonAsHighlighted ? juce::Colour(0xFF21262D) : juce::Colour(0xFF161B22));
            g.setColour(bg);
            g.fillRoundedRectangle(bounds, 4.0f);

            juce::Colour borderCol = on ? juce::Colour(0xFFFFD700)
                                   : (shouldDrawButtonAsHighlighted ? juce::Colour(0xFF484F58) : juce::Colour(0xFF30363D));
            g.setColour(borderCol);
            g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

            auto iconArea = bounds.reduced(5.0f);
            juce::Colour glyphCol = on ? juce::Colour(0xFFFFD700) : juce::Colour(0xFF8B949E);
            g.setColour(glyphCol);

            auto body = Icons::padlockBody();
            body.applyTransform(juce::AffineTransform::scale(iconArea.getWidth(), iconArea.getHeight())
                                    .translated(iconArea.getX(), iconArea.getY()));
            g.fillPath(body);

            auto shackle = on ? Icons::shackleClosed() : Icons::shackleOpen();
            shackle.applyTransform(juce::AffineTransform::scale(iconArea.getWidth(), iconArea.getHeight())
                                       .translated(iconArea.getX(), iconArea.getY()));
            g.strokePath(shackle, juce::PathStrokeType(1.75f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        } else {
            juce::Colour bg = on ? accentCol_
                            : (shouldDrawButtonAsDown ? juce::Colour(0xFF262D38)
                            : (shouldDrawButtonAsHighlighted ? juce::Colour(0xFF21262D) : juce::Colour(0xFF161B22)));
            g.setColour(bg);
            g.fillRoundedRectangle(bounds, 4.0f);

            juce::Colour borderCol = on ? accentCol_
                                   : (shouldDrawButtonAsHighlighted ? juce::Colour(0xFF484F58) : juce::Colour(0xFF30363D));
            g.setColour(borderCol);
            g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

            auto iconArea = bounds.reduced(5.0f);
            juce::Colour iconCol = on ? juce::Colour(0xFF0D1117)
                                 : (shouldDrawButtonAsHighlighted ? juce::Colours::white : accentCol_.withAlpha(0.85f));
            g.setColour(iconCol);

            juce::Path shapePath;
            if (type_ == IconType::SawUp) shapePath = Icons::sawUp();
            else if (type_ == IconType::Triangle) shapePath = Icons::triangle();
            else if (type_ == IconType::SawDown) shapePath = Icons::sawDown();

            shapePath.applyTransform(juce::AffineTransform::scale(iconArea.getWidth(), iconArea.getHeight())
                                         .translated(iconArea.getX(), iconArea.getY()));
            g.strokePath(shapePath, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
    }

private:
    IconType type_;
    juce::Colour accentCol_;
};

// ==============================================================================

// AuditionMenuButton: Clean dark capsule button with vector down-chevron

// ==============================================================================

class AuditionMenuButton : public juce::TextButton {

public:

    void setAccentColour(juce::Colour c) { accentCol_ = c; repaint(); }



    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {

        auto bounds = getLocalBounds().toFloat();

        if (bounds.isEmpty()) return;



        juce::Colour bg = shouldDrawButtonAsDown ? juce::Colour(0xFF262D38)

                        : (shouldDrawButtonAsHighlighted ? juce::Colour(0xFF21262D) : juce::Colour(0xFF161B22));

        g.setColour(bg);

        g.fillRect(bounds);



        g.setColour(juce::Colour(0xFF30363D));

        g.drawRect(bounds, 1.0f);



        // Vector down-chevron at right edge

        const float chevronX = bounds.getRight() - 12.0f;

        const float chevronY = bounds.getCentreY();

        juce::Path p;

        p.startNewSubPath(chevronX - 3.5f, chevronY - 2.0f);

        p.lineTo(chevronX, chevronY + 2.0f);

        p.lineTo(chevronX + 3.5f, chevronY - 2.0f);

        g.setColour(accentCol_.withAlpha(0.85f));

        g.strokePath(p, juce::PathStrokeType(1.5f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));



        // Text area

        auto textRect = bounds.withTrimmedRight(18.0f).withTrimmedLeft(6.0f);

        g.setColour(shouldDrawButtonAsHighlighted ? juce::Colours::white : accentCol_.withAlpha(0.95f));

        auto font = juce::Font(juce::FontOptions(11.5f).withStyle("Bold"));

        g.setFont(font);

        g.drawFittedText(getButtonText(), textRect.toNearestInt(), juce::Justification::centred, 1);

    }



private:

    juce::Colour accentCol_{0xFF00C8F0};

};



// ==============================================================================

// StepperAuditionBar: Categorized PopupMenu + Stepper Buttons [ < ] [ Label v ] [ > ]

// ==============================================================================

class StepperAuditionBar : public juce::Component {

public:

    StepperAuditionBar() {

        prevBtn_.setButtonText("<");

        nextBtn_.setButtonText(">");

        menuBtn_.setTriggeredOnMouseDown(true);



        for (auto* b : { &prevBtn_, &nextBtn_ }) {

            b->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF161B22));

            b->setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFF21262D));

            b->setColour(juce::TextButton::textColourOffId, juce::Colour(0xFFC9D1D9));

            b->setColour(juce::TextButton::textColourOnId, juce::Colour(0xFFF0F6FC));

            addAndMakeVisible(b);

        }

        addAndMakeVisible(menuBtn_);



        prevBtn_.onClick = [this]() { step(-1); };

        nextBtn_.onClick = [this]() { step(+1); };

        menuBtn_.onClick = [this]() { showPopup(); };

    }



    void setAccentColour(juce::Colour c) {

        accentCol_ = c;

        menuBtn_.setAccentColour(c);

        repaint();

    }



    void setItem(int index, const juce::String& text, int totalCount) {

        currentIndex_ = index;

        totalCount_ = totalCount;

        currentText_ = text;

        menuBtn_.setButtonText(text);

        repaint();

    }



    int getCurrentIndex() const noexcept { return currentIndex_; }



    void resized() override {

        auto r = getLocalBounds();

        const int btnW = 24;

        prevBtn_.setBounds(r.removeFromLeft(btnW));

        nextBtn_.setBounds(r.removeFromRight(btnW));

        menuBtn_.setBounds(r);

    }



    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& wheel) override {

        if (wheel.deltaY > 0.01f) step(+1);

        else if (wheel.deltaY < -0.01f) step(-1);

    }



    std::function<void(int)> onSelectIndex;

    std::function<void(juce::PopupMenu&)> onBuildMenu;



private:

    void step(int delta) {

        if (totalCount_ <= 0) return;

        int next = (currentIndex_ + delta + totalCount_) % totalCount_;

        if (onSelectIndex) onSelectIndex(next);

    }



    void showPopup() {

        if (!onBuildMenu) return;

        juce::PopupMenu menu;

        onBuildMenu(menu);



        auto opts = juce::PopupMenu::Options().withTargetComponent(&menuBtn_).withStandardItemHeight(26);

        const auto* d = juce::Desktop::getInstance().getDisplays().getDisplayForRect(menuBtn_.getScreenBounds());

        const int dispH = d ? d->userArea.getHeight() : 720;

        const int fit = juce::jmax(8, (dispH - 64) / 26);

        const int cols = juce::jlimit(1, 3, juce::jmax((totalCount_ + fit - 1) / fit, totalCount_ > 18 ? 2 : 1));

        opts = opts.withMinimumNumColumns(cols).withMaximumNumColumns(cols);



        juce::Component::SafePointer<StepperAuditionBar> safeThis(this);

        menu.showMenuAsync(opts,

            [safeThis](int result) {

                if (safeThis == nullptr || result <= 0) return;

                int selectedIndex = result - 1; // 1-based index

                if (safeThis->onSelectIndex) {

                    safeThis->onSelectIndex(selectedIndex);

                }

            });

    }



    int currentIndex_ = 0;

    int totalCount_ = 0;

    juce::String currentText_;

    juce::TextButton prevBtn_;

    AuditionMenuButton menuBtn_;

    juce::TextButton nextBtn_;

    juce::Colour accentCol_{0xFF00C8F0};

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



    void setLive(bool on, float wave, int mask, float speed) {

        if (on == hasLive_ && std::abs(wave - liveWave_) < 1e-3f && mask == liveMask_ && std::abs(speed - liveSpeed_) < 1e-3f) return;

        hasLive_ = on;

        liveWave_ = wave;

        liveMask_ = mask;

        liveSpeed_ = speed;

        repaint();

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

        float morphVal = 0.0f;

        float oddEvenVal = 0.0f;

        float tiltVal = 0.0f;

        float formantVal = 0.5f;

        int maskVal = 511;

        float speedVal = 0.0f;

        int loopVal = 1;

        uint32_t bankVal = 0;



        if (oscIndex_ == 0) {

            bankVal = static_cast<uint32_t>(*proc_.getAPVTS().getRawParameterValue(Ids::osc1_bank));

            morphVal = hasLive_ ? liveWave_ : static_cast<float>(*proc_.getAPVTS().getRawParameterValue(Ids::osc1_morph));

            oddEvenVal = static_cast<float>(*proc_.getAPVTS().getRawParameterValue(Ids::osc1_odd_even));

            tiltVal = static_cast<float>(*proc_.getAPVTS().getRawParameterValue(Ids::osc1_tilt));

            formantVal = static_cast<float>(*proc_.getAPVTS().getRawParameterValue(Ids::osc1_formant));

            maskVal = hasLive_ ? liveMask_ : static_cast<int>(*proc_.getAPVTS().getRawParameterValue(Ids::osc1_mask));

            speedVal = hasLive_ ? liveSpeed_ : static_cast<float>(*proc_.getAPVTS().getRawParameterValue(Ids::osc1_speed));

            loopVal = static_cast<int>(*proc_.getAPVTS().getRawParameterValue(Ids::osc1_loop));

        } else if (oscIndex_ == 1) {

            bankVal = static_cast<uint32_t>(*proc_.getAPVTS().getRawParameterValue(Ids::osc2_bank));

            morphVal = hasLive_ ? liveWave_ : static_cast<float>(*proc_.getAPVTS().getRawParameterValue(Ids::osc2_morph));

            oddEvenVal = static_cast<float>(*proc_.getAPVTS().getRawParameterValue(Ids::osc2_odd_even));

            tiltVal = static_cast<float>(*proc_.getAPVTS().getRawParameterValue(Ids::osc2_tilt));

            formantVal = static_cast<float>(*proc_.getAPVTS().getRawParameterValue(Ids::osc2_formant));

            maskVal = hasLive_ ? liveMask_ : static_cast<int>(*proc_.getAPVTS().getRawParameterValue(Ids::osc2_mask));

            speedVal = hasLive_ ? liveSpeed_ : static_cast<float>(*proc_.getAPVTS().getRawParameterValue(Ids::osc2_speed));

            loopVal = static_cast<int>(*proc_.getAPVTS().getRawParameterValue(Ids::osc2_loop));

        } else {

            bankVal = static_cast<uint32_t>(*proc_.getAPVTS().getRawParameterValue(Ids::osc3_bank));

            morphVal = hasLive_ ? liveWave_ : static_cast<float>(*proc_.getAPVTS().getRawParameterValue(Ids::osc3_morph));

            oddEvenVal = static_cast<float>(*proc_.getAPVTS().getRawParameterValue(Ids::osc3_odd_even));

            tiltVal = static_cast<float>(*proc_.getAPVTS().getRawParameterValue(Ids::osc3_tilt));

            formantVal = static_cast<float>(*proc_.getAPVTS().getRawParameterValue(Ids::osc3_formant));

            maskVal = hasLive_ ? liveMask_ : static_cast<int>(*proc_.getAPVTS().getRawParameterValue(Ids::osc3_mask));

            speedVal = hasLive_ ? liveSpeed_ : static_cast<float>(*proc_.getAPVTS().getRawParameterValue(Ids::osc3_speed));

            loopVal = static_cast<int>(*proc_.getAPVTS().getRawParameterValue(Ids::osc3_loop));

        }



        std::array<float, 512> waveTable;

        int lifeMode = static_cast<int>(*proc_.getAPVTS().getRawParameterValue(Ids::life_mode));

        NyarlatAdditiveEngine::getInstance().synthesizeCycle(

            waveTable.data(), bankVal, morphVal, oddEvenVal, tiltVal, formantVal, lifeMode);



        const uint32_t mask9 = static_cast<uint32_t>(juce::jlimit(0, 511, maskVal) * 16 + 15) & 0x1FFu;

        const bool maskIdentity = (mask9 == 0x1FFu);

        uint32_t statorOff = 0;



        bool activeVoiceFound = false;

        auto tele = proc_.getVoiceTelemetry();

        for (const auto& vt : tele) {

            if (vt.active) {

                statorOff = vt.statorOffsets[oscIndex_];

                activeVoiceFound = true;

                break;

            }

        }



        const int effLoop = (loopVal == 0) ? 1 : loopVal;

        if (!activeVoiceFound && speedVal > 0.0f) {

            previewPhase_ += speedVal * 0.033f; // ~30 Hz timer delta

            if (previewPhase_ >= 1.0f) previewPhase_ -= std::floor(previewPhase_);

            uint32_t p = static_cast<uint32_t>(previewPhase_ * 512.0f) & 0x1FFu;

            uint32_t sweep = (effLoop == 1) ? p : (p < 256u ? p * 2u : (511u - p) * 2u);

            statorOff = sweep & 0x1FFu;

        }



        auto S = [](float x) noexcept {

            x = juce::jlimit(0.0f, 1.0f, x);

            return x * x * (3.0f - 2.0f * x);

        };



        const float t = static_cast<float>(statorOff) * (1.0f / 512.0f);

        const float depth = (effLoop == 2)

            ? S(1.0f - std::fabs(2.0f * t - 1.0f))

            : (t <= 0.9f ? t : t * (1.0f - S((t - 0.9f) * 10.0f)));

        const float r = 1.0f + 5.0f * depth;



        juce::Path p;

        float w = bounds.getWidth() - 8.0f;

        float h = bounds.getHeight() - 8.0f;

        float startX = bounds.getX() + 4.0f;

        float startY = bounds.getY() + 4.0f;



        const int numPts = 128;

        for (int pt = 0; pt < numPts; ++pt) {

            const float phi = static_cast<float>(pt) / static_cast<float>(numPts - 1);

            float x = phi * r;

            x -= std::floor(x);

            const float pos = x * 512.0f;

            const uint32_t i = static_cast<uint32_t>(pos);

            const uint32_t im = i & mask9;

            float s = waveTable[im];

            if (maskIdentity) {

                const float frac = pos - static_cast<float>(i);

                s += frac * (waveTable[(im + 1u) & 0x1FFu] - s);

            }

            s *= (1.0f - depth * S(phi));



            float px = startX + phi * w;

            float py = startY + (0.5f - s * 0.44f) * h;



            if (pt == 0) p.startNewSubPath(px, py);

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

    bool hasLive_ = false;

    float liveWave_ = 0.0f;

    int liveMask_ = 511;

    float liveSpeed_ = 0.0f;

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

// NyarlatLookAndFeel: Crackchimes Fleet Standard LookAndFeel via CrackedChimesUI

// ==============================================================================

class NyarlatLookAndFeel : public CrackedChimes::UI::CrackedChimesLookAndFeel {

public:

    NyarlatLookAndFeel() {

        setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xFF0D1117));

        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xFF0D1117));

        setColour(juce::PopupMenu::textColourId, juce::Colour(0xFFF0F6FC));

        setColour(juce::PopupMenu::headerTextColourId, juce::Colour(0xFF8B949E));

        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xFF21262D));

        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);

        accentColour_ = juce::Colour(0xFF00C8F0);

        modRingDefaultColour_ = juce::Colour(0xFFFF7B25); // Movie Poster Orange

        randRingDefaultColour_ = juce::Colour(0xFFF2E10F); // Caustic Yellow

    }



    void setAccentColour(juce::Colour c) { accentColour_ = c; }

    juce::Colour getAccentColour() const noexcept { return accentColour_; }

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



    struct RadioItem {

        SegmentedRadio radio;

        juce::String paramId;

        std::unique_ptr<juce::ParameterAttachment> attachment;

    };



    struct StepperItem {

        StepperAuditionBar bar;

        juce::Label label;

        juce::String paramId;

        std::unique_ptr<juce::ParameterAttachment> attachment;

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

    void setupStepper(StepperItem& item, const juce::String& paramId, const juce::String& label, juce::Colour col);

    void setupButton(ButtonItem& item, const juce::String& paramId, const juce::String& label, juce::Colour col);

    void setupRadio(RadioItem& item, const juce::String& paramId, juce::Colour col, float fontSize = 10.0f);

    SegmentedRadio* findRadio(const juce::String& paramId);

    StepperItem* findStepper(const juce::String& paramId);



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

    StepperAuditionBar stepperAuditionBar_;

    HeaderInspector headerInspector_;

    RockerToggle3 lifeModeToggle_{{"SYNTHETIC", "CYBORG", "ORGANIC"}};

    std::unique_ptr<juce::ParameterAttachment> lifeModeAttachment_;



    // Visualizers

    std::array<std::unique_ptr<BitmaskWaveVisualizer>, 3> oscWaveViews_;

    std::unique_ptr<VectorAdsrVisualizer> vcaAdsrView_;



    // All parameter controls

    std::vector<std::unique_ptr<RotaryKnob>> knobs_;

    std::vector<std::unique_ptr<SliderFader>> faders_;

    std::vector<std::unique_ptr<ComboBoxItem>> combos_;

    std::vector<std::unique_ptr<StepperItem>> steppers_;

    std::vector<std::unique_ptr<ButtonItem>> buttons_;

    std::vector<std::unique_ptr<RadioItem>> radios_;

    std::array<juce::Label, 3> loopTags_;

    struct OscShapeGrid {
        std::unique_ptr<VectorIconButton> lockBtn;
        std::unique_ptr<VectorIconButton> triBtn;
        std::unique_ptr<VectorIconButton> upBtn;
        std::unique_ptr<VectorIconButton> downBtn;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lockAttachment;
        std::unique_ptr<juce::ParameterAttachment> loopAttachment;
    };
    std::array<OscShapeGrid, 3> oscShapeGrids_;



    std::unordered_map<juce::Component*, const ControlInfo*> controlMap_;



    // Moving lerp tracking

    std::vector<float> smoothedRandPips_;



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NyarlatEditor)

};



} // namespace Nyarlat

