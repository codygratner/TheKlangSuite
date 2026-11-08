#pragma once
// CrackedChimesUI - CrackedChimesKnob: rotary slider carrying up to 4 outward
// modulation rings (PLAN.md section 4) + Kilohearts value badge (section 3).
// Message-thread only. setRing() is meant to be fed from a UI Timer that
// reads audio telemetry atomics; it never allocates.

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <functional>
#include <memory>
#include "CrackedChimesColours.h"
#include "CrackedChimesModMath.h"

namespace CrackedChimes::UI
{
/** Ring slot priority == slot index: 0 Envelope, 1 Lfo, 2 Macro/Chaos
    (burble/drift), 3 SteppedRandom/AudioRateXMod. */
enum class RingSource
{
    Envelope,
    Lfo,
    Macro,
    SteppedRandom,
    Chaos,
    AudioRateXMod   // range only - never draws a live pip (audio-rate)
};

struct ModRingState
{
    bool        active   = false;
    float       loNorm   = 0.0f;   // normalised range start [0,1]
    float       hiNorm   = 0.0f;   // normalised range end   [0,1]
    bool        hasPip   = false;  // draw live pip at pipNorm
    float       pipNorm  = 0.0f;
    juce::Colour colour  { Colours::kModLfo };
    RingSource  source   = RingSource::Lfo;
    std::array<float, 7> ghostPips {};  // polyphonic ghost voices (normalised)
    int         numGhosts = 0;          // 0..7
};

// ---- Geometry constants (shared by knob hit-testing and LookAndFeel) ------
inline constexpr int   kMaxRings          = 4;
inline constexpr int   kMaxGhostPips      = 7;
inline constexpr float kRingPitch         = 3.0f;   // 2 px stroke + 1 px airgap
inline constexpr float kRingStroke        = 2.0f;
inline constexpr float kRingFirstOffset   = -2.0f;  // ring 0 centreline = rCore - 2 (Hastur R=32 @ rCore 34)
inline constexpr float kRingOutwardExtent = 11.0f;  // outer ring rCore+7 + 3.5 pip glow (+2 Hastur reduce folded in)
inline constexpr float kRingDrawEpsilon   = 0.003f; // min span / delta to draw
inline constexpr float kRingDirtyEpsilon  = 0.001f; // min change to repaint
inline constexpr float kLivePipDiameter   = 5.0f;
inline constexpr float kLivePipGlow       = 7.0f;   // Hastur: 7 px ring outline, 1 px stroke
inline constexpr float kGhostPipDiameter  = 3.0f;
inline constexpr float kGhostPipAlpha     = 0.35f;
inline constexpr float kHeroDialMin       = 64.0f;  // >= 64 px -> full badge

struct DialGeometry
{
    juce::Point<float>     centre;
    float                  diameter = 0.0f;
    float                  rCore    = 0.0f;  // invariant core radius
    float                  rTrack   = 0.0f;  // rCore - 4, 4 px stroke
    float                  rDisc    = 0.0f;  // rCore - 8
    juce::Rectangle<float> badge;            // 7-to-5 o'clock dead zone
    bool                   compact  = false; // diameter < 64 px
};

/** Single source of truth for dial layout (LookAndFeel + knob hit-test). */
inline DialGeometry computeDialGeometry (juce::Rectangle<float> area, float outwardExtent = 2.0f) noexcept
{
    DialGeometry g;
    g.diameter = juce::jmin (area.getWidth(), area.getHeight());
    g.centre   = area.getCentre();
    g.rCore    = juce::jmax (10.0f, g.diameter * 0.5f - outwardExtent);
    g.rTrack   = g.rCore - 4.0f;
    g.rDisc    = juce::jmax (2.0f, g.rCore - 8.0f);
    g.compact  = g.diameter < kHeroDialMin;

    const float badgeH = g.compact ? 13.5f : 15.0f;
    const float badgeW = std::clamp(g.diameter * 0.72f, 26.0f, g.diameter - 4.0f);
    const float badgeY = area.getBottom() - badgeH - 1.5f;
    g.badge = juce::Rectangle<float> (g.centre.x - badgeW * 0.5f, badgeY, badgeW, badgeH);
    return g;
}

/** Polar helper: JUCE arc convention (0 rad = 12 o'clock, clockwise). */
inline juce::Point<float> pointOnArc (juce::Point<float> c, float radius, float angle) noexcept
{
    return { c.x + radius * std::sin (angle), c.y - radius * std::cos (angle) };
}

class CrackedChimesKnob : public juce::Slider,
                          private juce::Timer
{
public:
    static constexpr int kBadgeHoldMs      = 350;
    static constexpr int kNormalDragPixels = 250;
    static constexpr int kFineDragPixels   = 2500;  // Shift = 0.1x

    CrackedChimesKnob() : CrackedChimesKnob (juce::String()) {}

    explicit CrackedChimesKnob (const juce::String& componentName)
        : juce::Slider (juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox)
    {
        setName (componentName);
        setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                             juce::MathConstants<float>::pi * 2.75f, true);
        setMouseDragSensitivity (kNormalDragPixels);
        setDoubleClickReturnValue (false, 0.0);   // handled in mouseDown
        setPopupMenuEnabled (false);
        setWantsKeyboardFocus (false);
    }

    ~CrackedChimesKnob() override
    {
        stopTimer();
        if (editor != nullptr)
        {
            editor->onReturnKey = nullptr;
            editor->onEscapeKey = nullptr;
            editor->onFocusLost = nullptr;
        }
    }

    // ---- Rings ------------------------------------------------------------
    /** Store ring state; repaints only if something moved > kRingDirtyEpsilon. */
    void setRing (int slot, const ModRingState& s)
    {
        if (! juce::isPositiveAndBelow (slot, kMaxRings))
            return;
        if (ringDiffers (rings[(size_t) slot], s))
        {
            rings[(size_t) slot] = s;
            rings[(size_t) slot].numGhosts = juce::jlimit (0, kMaxGhostPips, s.numGhosts);
            repaint();
        }
    }

    void clearRing (int slot)
    {
        if (juce::isPositiveAndBelow (slot, kMaxRings) && rings[(size_t) slot].active)
        {
            rings[(size_t) slot] = ModRingState();
            repaint();
        }
    }

    void clearRings()
    {
        for (int i = 0; i < kMaxRings; ++i)
            clearRing (i);
    }

    const ModRingState& getRing (int slot) const noexcept { return rings[(size_t) juce::jlimit (0, kMaxRings - 1, slot)]; }
    const std::array<ModRingState, kMaxRings>& getRings() const noexcept { return rings; }

    float getRingExtent() const noexcept
    {
        int activeRings = 0;
        for (int i = 0; i < kMaxRings; ++i)
            if (rings[(size_t) i].active)
                activeRings = i + 1;
        if (activeRings == 0) return 2.0f;
        return 2.0f + 3.0f * static_cast<float>(activeRings - 1) + 2.0f;
    }

    // ---- Presentation -----------------------------------------------------
    void setBipolar (bool b)            { if (bipolar != b) { bipolar = b; repaint(); } }
    bool isBipolar() const noexcept     { return bipolar; }
    void setHero (bool h)               { if (hero != h) { hero = h; repaint(); } }
    bool isHero() const noexcept        { return hero; }
    void setUnit (Unit u)               { if (unit != u) { unit = u; repaint(); } }
    Unit getUnit() const noexcept       { return unit; }

    /** Double-click target. If never set: bipolar -> 0 (clamped), else minimum. */
    void setDefaultValue (double v)     { defaultValue = v; hasDefault = true; }
    double getResetValue() const
    {
        if (hasDefault)  return juce::jlimit (getMinimum(), getMaximum(), defaultValue);
        if (bipolar)     return juce::jlimit (getMinimum(), getMaximum(), 0.0);
        return getMinimum();
    }

    /** Badge highlighted while hovering/dragging and for 350 ms after. */
    bool isBadgeHot() const noexcept
    {
        return hovering || isMouseButtonDown()
            || (juce::Time::getMillisecondCounter() - lastInteractionMs) < (juce::uint32) kBadgeHoldMs;
    }

    juce::String getBadgeText() const
    {
        return unit == Unit::None ? const_cast<CrackedChimesKnob*> (this)->getTextFromValue (getValue())
                                  : formatEngineeringValue (getValue(), unit);
    }

    juce::Rectangle<float> getBadgeBounds() const { return computeDialGeometry (getLocalBounds().toFloat(), getRingExtent()).badge; }
    bool isEditingValue() const noexcept { return editing; }

    /** Parses '440 Hz', '1.2k', '1.2 kHz', '-12 dB', '250 ms', '1.5 s', '50 %',
        '7 st', '1 oct'. Result clamped to [min,max]; ok=false if unparsable. */
    double parseValueText (const juce::String& text, bool& ok) const
    {
        ok = false;
        auto t = text.trim().toLowerCase().removeCharacters (" ");
        if (t.isEmpty())
            return getValue();

        if (unit == Unit::None)
        {
            ok = true;
            return juce::jlimit (getMinimum(), getMaximum(), const_cast<CrackedChimesKnob*> (this)->getValueFromText (text));
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
            case Unit::Ms:                if (suffix == "s" || suffix.startsWith ("sec")) v *= 1000.0; break;
            case Unit::Seconds:           if (suffix.startsWith ("ms")) v /= 1000.0; break;
            case Unit::PercentNormalised: v /= 100.0; break;
            case Unit::Semitones:         if (suffix.startsWith ("oct")) v *= 12.0; break;
            case Unit::Octaves:           if (suffix.startsWith ("st")) v /= 12.0; break;
            case Unit::Hz: case Unit::dB: case Unit::Percent: case Unit::None: default: break;
        }

        ok = true;
        return juce::jlimit (getMinimum(), getMaximum(), v);
    }

    void showValueEditor()
    {
        if (editor == nullptr)
        {
            editor = std::make_unique<juce::TextEditor>();
            editor->setJustification (juce::Justification::centred);
            editor->setColour (juce::TextEditor::backgroundColourId,     Colours::kBadgeBg);
            editor->setColour (juce::TextEditor::textColourId,           Colours::kNeedle);
            editor->setColour (juce::TextEditor::outlineColourId,        Colours::kBadgeBorder);
            editor->setColour (juce::TextEditor::focusedOutlineColourId, findColour (juce::Slider::rotarySliderFillColourId));
            editor->setColour (juce::TextEditor::highlightColourId,      Colours::kChassisRim);
            editor->onReturnKey = [this] { hideValueEditor (true); };
            editor->onEscapeKey = [this] { hideValueEditor (false); };
            editor->onFocusLost = [this] { hideValueEditor (true); };
            addChildComponent (*editor);
        }

        const auto geo = computeDialGeometry (getLocalBounds().toFloat(), getRingExtent());
        editor->setFont (Typography::makeFont (geo.compact ? Typography::kBadgeCompactSize : Typography::kBadgeSize));
        editor->setBounds (geo.badge.expanded (4.0f, 1.0f).toNearestInt().getIntersection (getLocalBounds()));
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
        editing = false;   // set first: setVisible(false) re-enters via onFocusLost

        if (commit)
        {
            bool ok = false;
            const double v = parseValueText (editor->getText(), ok);
            if (ok)
                setValue (v, juce::sendNotificationSync);  // attachment wraps the gesture
        }
        editor->setVisible (false);
        repaint();
    }

    /** Right-click hook: populate the menu; empty menu -> nothing shown. */
    std::function<void (juce::PopupMenu&)> onBuildContextMenu;

    // ---- Mouse ------------------------------------------------------------
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
            // Re-anchor so the sensitivity switch never jumps the value.
            juce::Slider::mouseUp (e);
            fineMode = e.mods.isShiftDown();
            setMouseDragSensitivity (fineMode ? kFineDragPixels : kNormalDragPixels);
            juce::Slider::mouseDown (e);
        }
        juce::Slider::mouseDrag (e);
        noteInteraction();
    }

    void mouseUp (const juce::MouseEvent& e) override
    {
        if (pendingBadgeClick)
        {
            pendingBadgeClick = false;
            if (e.mouseWasClicked())
                showValueEditor();
            return;
        }
        if (swallowGesture)
        {
            swallowGesture = false;
            return;
        }
        juce::Slider::mouseUp (e);
        noteInteraction();
        startTimer (kBadgeHoldMs + 10);
    }

    void mouseEnter (const juce::MouseEvent& e) override
    {
        hovering = true;
        juce::Slider::mouseEnter (e);
        repaint();
    }

    void mouseExit (const juce::MouseEvent& e) override
    {
        hovering = false;
        noteInteraction();
        juce::Slider::mouseExit (e);
        startTimer (kBadgeHoldMs + 10);
    }

    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& w) override
    {
        juce::Slider::mouseWheelMove (e, w);
        noteInteraction();
        startTimer (kBadgeHoldMs + 10);
    }

    void resized() override
    {
        juce::Slider::resized();
        if (editing && editor != nullptr)
            editor->setBounds (getBadgeBounds().expanded (4.0f, 1.0f).toNearestInt().getIntersection (getLocalBounds()));
    }

private:
    void timerCallback() override
    {
        stopTimer();
        repaint();
    }

    void noteInteraction() noexcept { lastInteractionMs = juce::Time::getMillisecondCounter(); }

    static bool ringDiffers (const ModRingState& a, const ModRingState& b) noexcept
    {
        if (a.active != b.active || a.hasPip != b.hasPip || a.source != b.source
            || a.numGhosts != b.numGhosts || a.colour != b.colour)
            return true;
        if (absf (a.loNorm - b.loNorm) > kRingDirtyEpsilon || absf (a.hiNorm - b.hiNorm) > kRingDirtyEpsilon)
            return true;
        if (b.hasPip && absf (a.pipNorm - b.pipNorm) > kRingDirtyEpsilon)
            return true;
        const int n = juce::jlimit (0, kMaxGhostPips, b.numGhosts);
        for (int i = 0; i < n; ++i)
            if (absf (a.ghostPips[(size_t) i] - b.ghostPips[(size_t) i]) > kRingDirtyEpsilon)
                return true;
        return false;
    }

    std::array<ModRingState, kMaxRings> rings {};
    std::unique_ptr<juce::TextEditor> editor;
    Unit   unit = Unit::None;
    double defaultValue = 0.0;
    bool   hasDefault = false;
    bool   bipolar = false;
    bool   hero = false;
    bool   hovering = false;
    bool   fineMode = false;
    bool   editing = false;
    bool   pendingBadgeClick = false;
    bool   swallowGesture = false;
    juce::uint32 lastInteractionMs = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CrackedChimesKnob)
};
} // namespace CrackedChimes::UI
