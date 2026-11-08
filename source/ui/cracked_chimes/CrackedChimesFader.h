#pragma once
// ==============================================================================
// CrackedChimesFader.h - Fleet Vertical Fader Standard with Recessed OLED Badge
// ==============================================================================
// Generalises Nyarlat's and Hastur's mixer faders into a unified, tactile
// vertical slider component mirroring CrackedChimesKnob.
//
// Key Features:
// - LinearVertical styling with NoTextBox (bounds dedicated to track + badge).
// - Sub-pixel Kilohearts OLED badge at base displaying formatted engineering units.
// - Direct click-to-edit value editor over badge.
// - Shift-drag 10x fine control (2500 px throw vs 250 px standard).
// - Double-click reset to default value.
// - Hover, drag, and 350 ms hold timer for tactile badge highlighting.
// ==============================================================================

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include <memory>
#include <algorithm>
#include "CrackedChimesColours.h"
#include "CrackedChimesModMath.h"

namespace CrackedChimes::UI
{

class CrackedChimesFader : public juce::Slider,
                           private juce::Timer
{
public:
    static constexpr int kBadgeHoldMs      = 350;
    static constexpr int kNormalDragPixels = 250;
    static constexpr int kFineDragPixels   = 2500; // Shift = 0.1x

    CrackedChimesFader() : CrackedChimesFader (juce::String()) {}

    explicit CrackedChimesFader (const juce::String& componentName)
        : juce::Slider (juce::Slider::LinearVertical, juce::Slider::NoTextBox)
    {
        setName (componentName);
        setMouseDragSensitivity (kNormalDragPixels);
        setDoubleClickReturnValue (false, 0.0); // handled in mouseDown
        setPopupMenuEnabled (false);
        setWantsKeyboardFocus (false);
    }

    ~CrackedChimesFader() override
    {
        stopTimer();
        if (editor != nullptr)
        {
            editor->onReturnKey = nullptr;
            editor->onEscapeKey = nullptr;
            editor->onFocusLost = nullptr;
        }
    }

    // ---- Presentation & Units ----------------------------------------------
    void setBipolar (bool b)            { if (bipolar != b) { bipolar = b; repaint(); } }
    bool isBipolar() const noexcept     { return bipolar; }

    void setUnit (Unit u)               { if (unit != u) { unit = u; repaint(); } }
    Unit getUnit() const noexcept       { return unit; }

    void setDefaultValue (double v)     { defaultValue = v; hasDefault = true; }
    double getResetValue() const
    {
        if (hasDefault)  return juce::jlimit (getMinimum(), getMaximum(), defaultValue);
        if (bipolar)     return juce::jlimit (getMinimum(), getMaximum(), 0.0);
        return getMinimum();
    }

    /** Badge highlighted while hovering, dragging, or within 350 ms after. */
    bool isBadgeHot() const noexcept
    {
        return hovering || isMouseButtonDown()
            || (juce::Time::getMillisecondCounter() - lastInteractionMs) < static_cast<juce::uint32> (kBadgeHoldMs);
    }

    juce::String getBadgeText() const
    {
        if (textFromValueFunction)
            return textFromValueFunction (getValue());
        return unit == Unit::None ? const_cast<CrackedChimesFader*> (this)->getTextFromValue (getValue())
                                  : formatEngineeringValue (getValue(), unit);
    }

    juce::Rectangle<float> getBadgeBounds() const noexcept
    {
        const auto area = getLocalBounds().toFloat();
        const float badgeH = 16.0f;
        const float badgeW = std::clamp (area.getWidth() - 4.0f, 26.0f, 44.0f);
        const float badgeX = area.getCentreX() - badgeW * 0.5f;
        const float badgeY = area.getBottom() - badgeH - 2.0f;
        return { badgeX, badgeY, badgeW, badgeH };
    }

    bool isEditingValue() const noexcept { return editing; }

    // ---- Value Editor ------------------------------------------------------
    double parseValueText (const juce::String& text, bool& ok) const
    {
        ok = false;
        auto t = text.trim().toLowerCase().removeCharacters (" ");
        if (t.isEmpty())
            return getValue();

        if (unit == Unit::None)
        {
            ok = true;
            return juce::jlimit (getMinimum(), getMaximum(), const_cast<CrackedChimesFader*> (this)->getValueFromText (text));
        }

        if (t.contains ("inf"))
        {
            ok = true;
            return t.startsWithChar ('-') ? getMinimum() : getMaximum();
        }

        int i = 0;
        const int len = t.length();
        while (i < len)
        {
            const auto c = t[i];
            if (juce::CharacterFunctions::isDigit (c) || c == '.' || ((c == '-' || c == '+') && i == 0))
                ++i;
            else
                break;
        }
        if (i == 0 || (i == 1 && ! juce::CharacterFunctions::isDigit (t[0])))
            return getValue();

        double v = t.substring (0, i).getDoubleValue();
        auto suffix = t.substring (i);

        if (suffix.startsWithChar ('k'))
        {
            v *= 1000.0;
            suffix = suffix.substring (1);
        }

        switch (unit)
        {
            case Unit::PercentNormalised:
                if (suffix.contains ("%")) v /= 100.0;
                else if (v > 1.0) v /= 100.0;
                break;
            case Unit::Percent:
                // Raw percentage [0..100]
                break;
            case Unit::dB:
                // Raw dB values
                break;
            case Unit::Ms:
                if (suffix == "s" || suffix.startsWith ("sec")) v *= 1000.0;
                break;
            case Unit::Seconds:
                if (suffix.startsWith ("ms")) v /= 1000.0;
                break;
            case Unit::Semitones:
                if (suffix.startsWith ("oct")) v *= 12.0;
                break;
            case Unit::Octaves:
                if (suffix.startsWith ("st")) v /= 12.0;
                break;
            case Unit::Hz:
            case Unit::None:
            default:
                break;
        }

        ok = true;
        return juce::jlimit (getMinimum(), getMaximum(), v);
    }

    void showValueEditor()
    {
        if (editing) return;
        if (editor == nullptr)
        {
            editor = std::make_unique<juce::TextEditor>();
            editor->setJustification (juce::Justification::centred);
            editor->setColour (juce::TextEditor::backgroundColourId, Colours::kBadgeBg);
            editor->setColour (juce::TextEditor::textColourId, Colours::kNeedle);
            editor->setColour (juce::TextEditor::outlineColourId, Colours::kBadgeBorder);
            editor->setColour (juce::TextEditor::focusedOutlineColourId, Colours::kOscCore);
            editor->setSelectAllWhenFocused (true);
            editor->setEscapeAndReturnKeysConsumed (true);

            editor->onReturnKey = [this] { hideValueEditor (true); };
            editor->onEscapeKey = [this] { hideValueEditor (false); };
            editor->onFocusLost = [this] { hideValueEditor (true); };

            addChildComponent (*editor);
        }

        const auto b = getBadgeBounds();
        editor->setFont (Typography::makeFont (Typography::kBadgeCompactSize));
        editor->setBounds (b.expanded (2.0f, 1.0f).toNearestInt().getIntersection (getLocalBounds()));
        editor->setText (getBadgeText(), false);
        editing = true;
        editor->setVisible (true);
        editor->grabKeyboardFocus();
        editor->selectAll();
        repaint();
    }

    void hideValueEditor (bool commit)
    {
        if (! editing || editor == nullptr)
            return;
        editing = false;

        if (commit)
        {
            bool ok = false;
            const double v = parseValueText (editor->getText(), ok);
            if (ok)
                setValue (v, juce::sendNotificationSync);
        }
        editor->setVisible (false);
        repaint();
    }

    std::function<void (juce::PopupMenu&)> onBuildContextMenu;

protected:
    void mouseDown (const juce::MouseEvent& e) override
    {
        noteInteraction();
        swallowGesture = false;
        pendingBadgeClick = false;

        if (e.mods.isPopupMenu())
        {
            swallowGesture = true;
            if (onBuildContextMenu)
            {
                juce::PopupMenu m;
                onBuildContextMenu (m);
                if (m.getNumItems() > 0)
                    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this));
            }
            return;
        }

        if (getBadgeBounds().contains (e.position))
        {
            pendingBadgeClick = true;
            return;
        }

        if (e.getNumberOfClicks() >= 2)
        {
            swallowGesture = true;
            setValue (getResetValue(), juce::sendNotificationSync);
            return;
        }

        fineMode = e.mods.isShiftDown();
        setMouseDragSensitivity (fineMode ? kFineDragPixels : kNormalDragPixels);
        juce::Slider::mouseDown (e);
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (pendingBadgeClick || swallowGesture)
            return;

        if (e.mods.isShiftDown() != fineMode)
        {
            juce::Slider::mouseUp (e);
            fineMode = e.mods.isShiftDown();
            setMouseDragSensitivity (fineMode ? kFineDragPixels : kNormalDragPixels);
            juce::Slider::mouseDown (e);
        }

        noteInteraction();
        juce::Slider::mouseDrag (e);
    }

    void mouseUp (const juce::MouseEvent& e) override
    {
        if (pendingBadgeClick)
        {
            pendingBadgeClick = false;
            if (getBadgeBounds().contains (e.position))
                showValueEditor();
            return;
        }

        if (swallowGesture)
        {
            swallowGesture = false;
            return;
        }

        noteInteraction();
        juce::Slider::mouseUp (e);
    }

    void mouseEnter (const juce::MouseEvent&) override
    {
        hovering = true;
        repaint();
    }

    void mouseExit (const juce::MouseEvent&) override
    {
        hovering = false;
        if (! isBadgeHot())
            repaint();
    }

    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override
    {
        noteInteraction();
        juce::Slider::mouseWheelMove (e, wheel);
    }

    void timerCallback() override
    {
        if (! isBadgeHot())
        {
            stopTimer();
            repaint();
        }
    }

private:
    void noteInteraction() noexcept
    {
        lastInteractionMs = juce::Time::getMillisecondCounter();
        if (! isTimerRunning())
            startTimer (100);
        repaint();
    }

    Unit unit = Unit::None;
    bool bipolar = false;
    bool hasDefault = false;
    double defaultValue = 0.0;
    bool hovering = false;
    bool fineMode = false;
    bool editing = false;
    bool pendingBadgeClick = false;
    bool swallowGesture = false;
    juce::uint32 lastInteractionMs = 0;
    std::unique_ptr<juce::TextEditor> editor;
};

} // namespace CrackedChimes::UI
