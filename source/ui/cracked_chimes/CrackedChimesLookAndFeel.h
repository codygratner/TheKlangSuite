#pragma once
// CrackedChimesUI - fleet LookAndFeel. Generalises Hastur's cutoff knob
// (PLAN.md section 4 Concentric Ring Standard) to every CrackedChimesKnob.

#include <juce_gui_basics/juce_gui_basics.h>
#include "CrackedChimesColours.h"
#include "CrackedChimesModMath.h"
#include "CrackedChimesKnob.h"
#include "CrackedChimesFader.h"

namespace CrackedChimes::UI
{
class CrackedChimesLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CrackedChimesLookAndFeel()
    {
        using namespace Colours;
        setColour (juce::ResizableWindow::backgroundColourId,       kChassisBg);
        setColour (juce::Slider::rotarySliderFillColourId,          kOscCore);
        setColour (juce::Slider::rotarySliderOutlineColourId,       kChassisRim);
        setColour (juce::Slider::trackColourId,                     kOscCore);
        setColour (juce::Slider::thumbColourId,                     kNeedle);
        setColour (juce::Slider::backgroundColourId,                kCavity);
        setColour (juce::Label::textColourId,                       kText);
        setColour (juce::ComboBox::backgroundColourId,              kCavity);
        setColour (juce::ComboBox::outlineColourId,                 kBadgeBorder);
        setColour (juce::ComboBox::textColourId,                    kText);
        setColour (juce::ComboBox::arrowColourId,                   kText);
        setColour (juce::PopupMenu::backgroundColourId,             kChassisDisc);
        setColour (juce::PopupMenu::textColourId,                   kText);
        setColour (juce::PopupMenu::highlightedBackgroundColourId,  kChassisRim);
        setColour (juce::PopupMenu::highlightedTextColourId,        kNeedle);
        setColour (juce::TextEditor::backgroundColourId,            kBadgeBg);
        setColour (juce::TextEditor::textColourId,                  kNeedle);
        setColour (juce::TextEditor::outlineColourId,               kBadgeBorder);
    }

    // ---- Rotary -----------------------------------------------------------
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        using namespace Colours;
        const auto* knob = dynamic_cast<const CrackedChimesKnob*> (&slider);

        float outwardExtent = 2.0f;
        if (slider.getProperties().contains("ringExtent"))
            outwardExtent = static_cast<float>(slider.getProperties()["ringExtent"]);
        else if (knob != nullptr)
            outwardExtent = knob->getRingExtent();
        else if (slider.getProperties().contains("isCutoffDial"))
            outwardExtent = 6.0f; // Leaves headroom for live envelope arc & pips (Hastur R=34 parity @ 80px)
        else if (slider.getProperties().contains("modLo") || slider.getProperties().contains("randLo"))
            outwardExtent = 4.0f; // Single outward modulation ring

        const auto geo = computeDialGeometry (juce::Rectangle<int> (x, y, width, height).toFloat(), outwardExtent);
        const float cx = geo.centre.x;
        const float cy = geo.centre.y;
        const float a0 = rotaryStartAngle;
        const float a1 = rotaryEndAngle;
        auto angleAt = [a0, a1] (float n) noexcept { return a0 + clampNorm (n) * (a1 - a0); };

        const bool enabled = slider.isEnabled();
        if (! enabled)
            g.beginTransparencyLayer (0.4f);

        const auto fill = slider.findColour (juce::Slider::rotarySliderFillColourId);

        // 1. Background track (R_core - 4, 4 px).
        const juce::PathStrokeType trackStroke (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
        arcPath.clear();
        arcPath.addCentredArc (cx, cy, geo.rTrack, geo.rTrack, 0.0f, a0, a1, true);
        g.setColour (kTrack);
        g.strokePath (arcPath, trackStroke);

        // 2. Value arc - skew-true bipolar noon.
        float fromN = 0.0f;
        const bool hasCenterAtNoon = slider.getProperties().contains("centerAtNoon") && static_cast<bool>(slider.getProperties()["centerAtNoon"]);
        const bool isBipolar = (knob != nullptr && knob->isBipolar())
            || (slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0)
            || (slider.getProperties().contains("isBipolar") && static_cast<bool>(slider.getProperties()["isBipolar"]))
            || hasCenterAtNoon;
        if (hasCenterAtNoon)
        {
            fromN = 0.5f;
        }
        else if (isBipolar)
        {
            const double zero = juce::jlimit (slider.getMinimum(), slider.getMaximum(), 0.0);
            fromN = (float) slider.valueToProportionOfLength (zero);
        }
        const float vLo = juce::jmin (fromN, sliderPos);
        const float vHi = juce::jmax (fromN, sliderPos);
        if (vHi - vLo > kRingDrawEpsilon)
        {
            arcPath.clear();
            arcPath.addCentredArc (cx, cy, geo.rTrack, geo.rTrack, 0.0f, angleAt (vLo), angleAt (vHi), true);
            g.setColour (fill);
            g.strokePath (arcPath, trackStroke);
        }

        // 3. Disc (R_core - 8) + needle.
        g.setColour (kChassisDisc);
        g.fillEllipse (cx - geo.rDisc, cy - geo.rDisc, geo.rDisc * 2.0f, geo.rDisc * 2.0f);
        g.setColour (kChassisRim);
        g.drawEllipse (cx - geo.rDisc, cy - geo.rDisc, geo.rDisc * 2.0f, geo.rDisc * 2.0f, 1.2f);

        const float needleAngle = angleAt (sliderPos);
        const auto n1 = pointOnArc (geo.centre, geo.rDisc * 0.75f, needleAngle);
        needlePath.clear();
        needlePath.startNewSubPath (geo.centre);
        needlePath.lineTo (n1);
        g.setColour (kNeedle);
        g.strokePath (needlePath, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (fill);   // centre jewel pip (Hastur parity)
        g.fillEllipse (cx - 2.5f, cy - 2.5f, 5.0f, 5.0f);

        // 4. Outward concentric modulation rings (compact packing, priority order).
        if (knob != nullptr)
            drawModRings (g, geo, *knob, a0, a1);
        else
            drawPropertyModRings (g, geo, slider, a0, a1);

        // 5. Kilohearts value badge in the 7-to-5 o'clock dead zone.
        bool showBadge = false;
        juce::String badgeText;
        bool badgeHot = false;

        if (knob != nullptr)
        {
            if (! knob->isEditingValue())
            {
                showBadge = true;
                badgeText = knob->getBadgeText();
                badgeHot = knob->isBadgeHot();
            }
        }
        else
        {
            bool isInteracting = slider.isMouseButtonDown();
            if (! isInteracting && slider.getProperties().contains("badgeExpiry"))
            {
                auto expiry = static_cast<juce::int64>(static_cast<double>(slider.getProperties()["badgeExpiry"]));
                if (juce::Time::getMillisecondCounter() < expiry)
                    isInteracting = true;
            }
            if (isInteracting)
            {
                showBadge = true;
                badgeText = slider.textFromValueFunction
                    ? slider.textFromValueFunction(slider.getValue())
                    : slider.getTextFromValue(slider.getValue());
                badgeHot = slider.isMouseButtonDown();
            }
        }

        if (showBadge && badgeText.isNotEmpty())
        {
            const auto b = geo.badge;
            g.setColour (kBadgeBg);
            g.fillRoundedRectangle (b, 4.0f);
            g.setColour (badgeHot ? fill.withAlpha (0.9f) : kBadgeBorder);
            g.drawRoundedRectangle (b, 4.0f, 1.0f);
            g.setColour (kText);
            g.setFont (Typography::makeFont (geo.compact ? Typography::kBadgeCompactSize : Typography::kBadgeSize));
            g.drawFittedText (badgeText, b.toNearestInt(), juce::Justification::centred, 1, 0.8f);
        }

        if (! enabled)
            g.endTransparencyLayer();
    }

    // ---- Linear (fader) ---------------------------------------------------
    juce::Slider::SliderLayout getSliderLayout (juce::Slider& slider) override
    {
        auto layout = juce::LookAndFeel_V4::getSliderLayout (slider);
        if (slider.getSliderStyle() == juce::Slider::LinearVertical)
        {
            // Reserve bottom 22px so thumb travel is strictly above the recessed OLED badge
            layout.sliderBounds = slider.getLocalBounds().withTrimmedBottom (22);
        }
        return layout;
    }

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        using namespace Colours;
        if (style != juce::Slider::LinearVertical && style != juce::Slider::LinearHorizontal)
        {
            juce::LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos,
                                                    minSliderPos, maxSliderPos, style, slider);
            return;
        }

        const auto* cf = dynamic_cast<const CrackedChimesFader*> (&slider);
        const bool isBipolar = (cf != nullptr && cf->isBipolar())
            || (slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0)
            || (slider.getProperties().contains ("isBipolar") && static_cast<bool> (slider.getProperties()["isBipolar"]));

        const auto trackCol = slider.isColourSpecified (juce::Slider::trackColourId)
            ? slider.findColour (juce::Slider::trackColourId) : accentColour_;
        const auto thumbCol = slider.isColourSpecified (juce::Slider::thumbColourId)
            ? slider.findColour (juce::Slider::thumbColourId) : trackCol;

        if (style == juce::Slider::LinearHorizontal)
        {
            const float trackY = static_cast<float> (y) + static_cast<float> (height) * 0.5f;
            const float groove = 6.0f;

            // Horizontal Track Groove
            g.setColour (kCavity);
            g.fillRoundedRectangle (static_cast<float> (x), trackY - 3.0f, static_cast<float> (width), groove, 3.0f);
            g.setColour (kBadgeBorder);
            g.drawRoundedRectangle (static_cast<float> (x), trackY - 3.0f, static_cast<float> (width), groove, 3.0f, 1.0f);

            // Active fill
            const float leftX = static_cast<float> (x);
            g.setColour (trackCol.withAlpha (0.75f));
            if (isBipolar)
            {
                const float zeroNorm = static_cast<float> (-slider.getMinimum() / (slider.getMaximum() - slider.getMinimum()));
                const float midX = leftX + zeroNorm * static_cast<float> (width);
                if (sliderPos > midX)
                    g.fillRoundedRectangle (midX, trackY - 2.0f, sliderPos - midX, 4.0f, 2.0f);
                else if (sliderPos < midX)
                    g.fillRoundedRectangle (sliderPos, trackY - 2.0f, midX - sliderPos, 4.0f, 2.0f);
            }
            else
            {
                if (sliderPos > leftX)
                    g.fillRoundedRectangle (leftX, trackY - 2.0f, sliderPos - leftX, 4.0f, 2.0f);
            }

            // Horizontal Fader Cap
            const float capW = 12.0f;
            const float capH = static_cast<float> (height) - 4.0f;
            const auto capBounds = juce::Rectangle<float> (sliderPos - capW * 0.5f, trackY - capH * 0.5f, capW, capH);

            g.setColour (kChassisDisc);
            g.fillRoundedRectangle (capBounds, 2.5f);
            g.setColour (kChassisRim);
            g.drawRoundedRectangle (capBounds, 2.5f, 1.0f);

            g.setColour (kNeedle);
            g.drawLine (capBounds.getCentreX(), capBounds.getY() + 3.0f, capBounds.getCentreX(), capBounds.getBottom() - 3.0f, 1.5f);
            return;
        }

        // LinearVertical Fader
        const auto area = juce::Rectangle<int> (x, y, width, height).toFloat();

        // 1. Recessed Bottom Kilohearts Value Badge Bounds
        const float badgeH = 16.0f;
        const float badgeW = std::clamp (area.getWidth() - 4.0f, 26.0f, 44.0f);
        const float badgeX = area.getCentreX() - badgeW * 0.5f;
        const float badgeY = area.getBottom() - badgeH - 2.0f;
        const auto badgeRect = juce::Rectangle<float> (badgeX, badgeY, badgeW, badgeH);

        // 2. Fader Track Bounds (restricted to travel above badge)
        const float trackTop = area.getY() + 4.0f;
        const float trackBottom = badgeRect.getY() - 4.0f;
        const float trackHeight = trackBottom - trackTop;
        const float trackX = area.getCentreX();
        const float grooveW = 6.0f;
        const auto track = juce::Rectangle<float> (trackX - grooveW * 0.5f, trackTop, grooveW, trackHeight);

        // Track Cavity & Border
        g.setColour (kCavity);
        g.fillRoundedRectangle (track, 3.0f);
        g.setColour (kBadgeBorder);
        g.drawRoundedRectangle (track, 3.0f, 1.0f);

        // Active fill
        const float clampedPos = std::clamp (sliderPos, trackTop, trackBottom);
        g.setColour (trackCol.withAlpha (0.85f));
        if (isBipolar)
        {
            const float zeroNorm = static_cast<float> (-slider.getMinimum() / (slider.getMaximum() - slider.getMinimum()));
            const float midY = trackTop + (1.0f - zeroNorm) * trackHeight;
            if (clampedPos < midY)
                g.fillRoundedRectangle (trackX - 2.0f, clampedPos, 4.0f, midY - clampedPos, 2.0f);
            else if (clampedPos > midY)
                g.fillRoundedRectangle (trackX - 2.0f, midY, 4.0f, clampedPos - midY, 2.0f);
        }
        else
        {
            if (clampedPos < trackBottom)
            {
                auto filled = track.withTop (clampedPos);
                g.fillRoundedRectangle (filled.reduced (1.0f), 2.0f);
            }
        }

        // 3. Tactile Fader Cap
        const float capW = std::clamp (area.getWidth() - 6.0f, 26.0f, 34.0f);
        const float capH = 13.0f;
        const auto capBounds = juce::Rectangle<float> (trackX - capW * 0.5f, clampedPos - capH * 0.5f, capW, capH);

        g.setColour (kChassisDisc);
        g.fillRoundedRectangle (capBounds, 2.5f);
        g.setColour (kChassisRim);
        g.drawRoundedRectangle (capBounds, 2.5f, 1.0f);
        g.setColour (kNeedle);
        g.drawLine (capBounds.getX() + 3.0f, capBounds.getCentreY(), capBounds.getRight() - 3.0f, capBounds.getCentreY(), 1.5f);

        // 4. Kilohearts Badge Value Rendering
        juce::String badgeText;
        bool badgeHot = false;

        if (cf != nullptr)
        {
            if (! cf->isEditingValue())
            {
                badgeText = cf->getBadgeText();
                badgeHot = cf->isBadgeHot();
            }
        }
        else
        {
            badgeText = slider.textFromValueFunction
                ? slider.textFromValueFunction (slider.getValue())
                : slider.getTextFromValue (slider.getValue());
            badgeHot = slider.isMouseButtonDown();
        }

        if (badgeText.isNotEmpty())
        {
            g.setColour (kBadgeBg);
            g.fillRoundedRectangle (badgeRect, 3.5f);
            g.setColour (badgeHot ? thumbCol.withAlpha (0.9f) : kBadgeBorder);
            g.drawRoundedRectangle (badgeRect, 3.5f, 1.0f);

            g.setFont (Typography::makeFont (Typography::kBadgeCompactSize));
            g.setColour (kText);
            g.drawFittedText (badgeText, badgeRect.toNearestInt(), juce::Justification::centred, 1, 0.85f);
        }
    }

    // ---- Combo box / popup -------------------------------------------------
    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int, int, int, int, juce::ComboBox& box) override
    {
        using namespace Colours;
        const auto r = juce::Rectangle<int> (0, 0, width, height).toFloat().reduced (0.5f);
        g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
        g.fillRoundedRectangle (r, 4.0f);
        g.setColour (isButtonDown || box.hasKeyboardFocus (true) ? kChassisRim.brighter (0.3f)
                                                                 : box.findColour (juce::ComboBox::outlineColourId));
        g.drawRoundedRectangle (r, 4.0f, 1.0f);

        const float ax = (float) width - 14.0f;
        const float ay = (float) height * 0.5f;
        arrowPath.clear();
        arrowPath.startNewSubPath (ax - 4.0f, ay - 2.0f);
        arrowPath.lineTo (ax, ay + 2.5f);
        arrowPath.lineTo (ax + 4.0f, ay - 2.0f);
        g.setColour (box.findColour (juce::ComboBox::arrowColourId).withAlpha (box.isEnabled() ? 0.9f : 0.3f));
        g.strokePath (arrowPath, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    juce::Font getComboBoxFont (juce::ComboBox&) override { return Typography::makeFont (Typography::kComboSize); }
    juce::Font getPopupMenuFont() override                { return Typography::makeFont (Typography::kPopupSize); }

    void getIdealPopupMenuItemSize (const juce::String& text, bool isSeparator, int standardMenuItemHeight,
                                    int& idealWidth, int& idealHeight) override
    {
        juce::LookAndFeel_V4::getIdealPopupMenuItemSize (text, isSeparator, standardMenuItemHeight, idealWidth, idealHeight);
        if (! isSeparator)
            idealHeight = juce::jmax (idealHeight, Typography::kPopupItemHeight);
    }

    // ---- Helpers ------------------------------------------------------------
    void setAccentColour (juce::Colour c) { accentColour_ = c; }
    juce::Colour getAccentColour() const noexcept { return accentColour_; }

    static void drawKiloheartsValueBadge (juce::Graphics& g, const juce::Rectangle<float>& bounds,
                                          const juce::String& text, juce::Colour accentColour)
    {
        if (text.isEmpty()) return;
        const auto font = Typography::makeFont (Typography::kBadgeSize);
        const float textW = juce::GlyphArrangement::getStringWidth (font, text);
        const float badgeW = std::clamp (textW + 10.0f, 26.0f, bounds.getWidth() - 6.0f);
        const float badgeH = 16.0f;
        const float badgeX = bounds.getCentreX() - badgeW * 0.5f;
        const float badgeY = bounds.getBottom() - badgeH - 1.0f;
        const auto badgeRect = juce::Rectangle<float> (badgeX, badgeY, badgeW, badgeH);

        g.setColour (Colours::kBadgeBg);
        g.fillRoundedRectangle (badgeRect, 3.5f);
        g.setColour (accentColour.withAlpha (0.65f));
        g.drawRoundedRectangle (badgeRect, 3.5f, 1.0f);

        g.setFont (font);
        g.setColour (Colours::kText);
        g.drawFittedText (text, badgeRect.toNearestInt(), juce::Justification::centred, 1);
    }

    /** Top-mounted label: 12.0f bold (12.5f hero), centred, never ellipsised. */
    static void drawTopLabel (juce::Graphics& g, juce::Rectangle<int> area, const juce::String& text,
                              bool hero = false, juce::Colour colour = Colours::kText)
    {
        g.setColour (colour);
        g.setFont (Typography::makeFont (hero ? Typography::kLabelHeroSize : Typography::kLabelSize));
        g.drawFittedText (text, area, juce::Justification::centred, 1, 0.85f);
    }

    /** Splits menus with > 18 items into ceil(n/18) columns. */
    static juce::PopupMenu::Options withAutoColumns (juce::PopupMenu::Options options, int numItems)
    {
        if (numItems > Typography::kAutoColumnThreshold)
        {
            const int cols = (numItems + Typography::kAutoColumnThreshold - 1) / Typography::kAutoColumnThreshold;
            return options.withMinimumNumColumns (cols).withMaximumNumColumns (cols);
        }
        return options;
    }

private:
    void drawModRings (juce::Graphics& g, const DialGeometry& geo, const CrackedChimesKnob& knob, float a0, float a1)
    {
        auto angleAt = [a0, a1] (float n) noexcept { return a0 + clampNorm (n) * (a1 - a0); };
        const juce::PathStrokeType ringStroke (kRingStroke, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);

        int packed = 0;
        for (int slot = 0; slot < kMaxRings; ++slot)
        {
            const auto& r = knob.getRing (slot);
            if (! r.active)
                continue;

            const float lo = clampNorm (juce::jmin (r.loNorm, r.hiNorm));
            const float hi = clampNorm (juce::jmax (r.loNorm, r.hiNorm));
            const bool hasSpan = (hi - lo) > kRingDrawEpsilon;
            const bool drawPip = r.hasPip && r.source != RingSource::AudioRateXMod;
            const int ghosts = r.source == RingSource::AudioRateXMod ? 0 : juce::jlimit (0, kMaxGhostPips, r.numGhosts);
            if (! hasSpan && ! drawPip && ghosts == 0)
                continue;

            const float rad = geo.rCore + kRingFirstOffset + kRingPitch * (float) packed;
            ++packed;

            if (hasSpan)
            {
                arcPath.clear();
                arcPath.addCentredArc (geo.centre.x, geo.centre.y, rad, rad, 0.0f, angleAt (lo), angleAt (hi), true);
                g.setColour (r.colour.withMultipliedAlpha (0.90f));
                g.strokePath (arcPath, ringStroke);
            }

            for (int i = 0; i < ghosts; ++i)
            {
                const auto p = pointOnArc (geo.centre, rad, angleAt (r.ghostPips[(size_t) i]));
                g.setColour (r.colour.withAlpha (kGhostPipAlpha));
                g.fillEllipse (juce::Rectangle<float> (kGhostPipDiameter, kGhostPipDiameter).withCentre (p));
            }

            if (drawPip)
            {
                const auto p = pointOnArc (geo.centre, rad, angleAt (r.pipNorm));
                g.setColour (juce::Colours::white);
                g.fillEllipse (juce::Rectangle<float> (kLivePipDiameter, kLivePipDiameter).withCentre (p));
                g.setColour (r.colour);
                g.drawEllipse (juce::Rectangle<float> (kLivePipGlow, kLivePipGlow).withCentre (p), 1.0f);
            }
        }
    }

    void drawPropertyModRings (juce::Graphics& g, const DialGeometry& geo, juce::Slider& slider, float a0, float a1)
    {
        auto angleAt = [a0, a1] (float n) noexcept { return a0 + clampNorm (n) * (a1 - a0); };
        const juce::PathStrokeType ringStroke (kRingStroke, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
        int packed = 0;

        auto drawArcAndPip = [&] (float loN, float hiN, float pipN, bool hasPip, juce::Colour col, const juce::Array<juce::var>* ghosts = nullptr) {
            const float lo = clampNorm (juce::jmin (loN, hiN));
            const float hi = clampNorm (juce::jmax (loN, hiN));
            const bool hasSpan = (hi - lo) > kRingDrawEpsilon;
            if (! hasSpan && ! hasPip && (ghosts == nullptr || ghosts->isEmpty())) return;

            const float rad = geo.rCore + kRingFirstOffset + kRingPitch * (float) packed;
            ++packed;

            if (hasSpan)
            {
                arcPath.clear();
                arcPath.addCentredArc (geo.centre.x, geo.centre.y, rad, rad, 0.0f, angleAt (lo), angleAt (hi), true);
                g.setColour (col.withMultipliedAlpha (0.90f));
                g.strokePath (arcPath, ringStroke);
            }

            if (ghosts != nullptr)
            {
                const int numG = juce::jmin (kMaxGhostPips, ghosts->size());
                for (int gi = 0; gi < numG; ++gi)
                {
                    float gn = clampNorm ((float) (double) (*ghosts)[gi]);
                    const auto gp = pointOnArc (geo.centre, rad, angleAt (gn));
                    g.setColour (col.withAlpha (kGhostPipAlpha));
                    g.fillEllipse (juce::Rectangle<float> (kGhostPipDiameter, kGhostPipDiameter).withCentre (gp));
                }
            }

            if (hasPip)
            {
                const auto p = pointOnArc (geo.centre, rad, angleAt (pipN));
                g.setColour (juce::Colours::white);
                g.fillEllipse (juce::Rectangle<float> (kLivePipDiameter, kLivePipDiameter).withCentre (p));
                g.setColour (col);
                g.drawEllipse (juce::Rectangle<float> (kLivePipGlow, kLivePipGlow).withCentre (p), 1.0f);
            }
        };

        const auto& props = slider.getProperties();

        // 1. Cutoff dial 4 concentric rings (Hastur / Nyarlat)
        if (props.contains ("isCutoffDial"))
        {
            const float kNorm = (float) slider.valueToProportionOfLength (slider.getValue());
            // Ring 0: Env
            if (props.contains ("cutoffEnvDepth"))
            {
                float env = (float) (double) props["cutoffEnvDepth"];
                if (std::abs (env) > 0.001f)
                {
                    float hi = std::clamp (kNorm + env, 0.0f, 1.0f);
                    float pip = hi;
                    bool hasPip = false;
                    if (props.contains ("cutoffEnvPip")) { pip = (float) (double) props["cutoffEnvPip"]; hasPip = true; }
                    drawArcAndPip (kNorm, hi, pip, hasPip, Colours::kFilterCore);
                }
            }
            // Ring 1: LFO
            if (props.contains ("cutoffLfoDepth"))
            {
                float lfo = (float) (double) props["cutoffLfoDepth"];
                int dest = props.contains ("cutoffLfoDest") ? (int) props["cutoffLfoDest"] : 0;
                if (dest == 0 && std::abs (lfo) > 0.001f)
                {
                    float lo = std::clamp (kNorm * (1.0f - lfo), 0.0f, 1.0f);
                    float hi = std::clamp (kNorm + lfo * (1.0f - kNorm), 0.0f, 1.0f);
                    float pip = kNorm;
                    bool hasPip = false;
                    if (props.contains ("cutoffLfoPip")) { pip = (float) (double) props["cutoffLfoPip"]; hasPip = true; }
                    drawArcAndPip (lo, hi, pip, hasPip, Colours::kLfoCore);
                }
            }
            // Ring 2: Burble
            if (props.contains ("cutoffBurbleDepth"))
            {
                float b = (float) (double) props["cutoffBurbleDepth"];
                if (std::abs (b) > 0.001f)
                {
                    float tgt = (b >= 0.0f) ? std::clamp (kNorm + b * (1.0f - kNorm), 0.0f, 1.0f)
                                           : std::clamp (kNorm + b * kNorm, 0.0f, 1.0f);
                    float pip = tgt;
                    bool hasPip = false;
                    if (props.contains ("cutoffBurblePip")) { pip = (float) (double) props["cutoffBurblePip"]; hasPip = true; }
                    drawArcAndPip (std::min (kNorm, tgt), std::max (kNorm, tgt), pip, hasPip, Colours::kBurble);
                }
            }
            // Ring 3: FM (Audio-rate -> no pip per Turn 15 rule)
            if (props.contains ("cutoffFmDepth"))
            {
                float fm = (float) (double) props["cutoffFmDepth"];
                if (std::abs (fm) > 0.001f)
                {
                    float tgt = (fm >= 0.0f) ? std::clamp (kNorm + fm * (1.0f - kNorm), 0.0f, 1.0f)
                                            : std::clamp (kNorm + fm * kNorm, 0.0f, 1.0f);
                    drawArcAndPip (std::min (kNorm, tgt), std::max (kNorm, tgt), 0.0f, false, Colours::kOscCore);
                }
            }
        }

        // 2. modLo / modHi / modPip (Macro / LFO)
        if (props.contains ("modLo") && props.contains ("modHi"))
        {
            float lo = (float) (double) props["modLo"];
            float hi = (float) (double) props["modHi"];
            float pip = props.contains ("modPip") ? (float) (double) props["modPip"] : lo;
            bool hasPip = props.contains ("modPip");
            juce::Colour c = props.contains ("modColour")
                ? juce::Colour ((juce::uint32) (juce::int64) props["modColour"])
                : modRingDefaultColour_;
            drawArcAndPip (lo, hi, pip, hasPip, c);
        }

        // 3. randLo / randHi / randPip (Stepped Random / Chaos)
        if (props.contains ("randLo") && props.contains ("randHi"))
        {
            float lo = (float) (double) props["randLo"];
            float hi = (float) (double) props["randHi"];
            float pip = props.contains ("randPip") ? (float) (double) props["randPip"] : lo;
            bool hasPip = props.contains ("randPip");
            juce::Colour c = props.contains ("randColour")
                ? juce::Colour ((juce::uint32) (juce::int64) props["randColour"])
                : randRingDefaultColour_;
            const auto* ghosts = props.contains ("randGhostPips") ? props["randGhostPips"].getArray() : nullptr;
            drawArcAndPip (lo, hi, pip, hasPip, c, ghosts);
        }

        // 4. stepRandAmount / activeStepVal
        if (props.contains ("stepRandAmount"))
        {
            float amt = (float) (double) props["stepRandAmount"];
            if (amt > 0.001f)
            {
                float kNorm = (float) slider.valueToProportionOfLength (slider.getValue());
                float lo = std::clamp (kNorm * (1.0f - amt), 0.0f, 1.0f);
                float hi = std::clamp (lo + amt, 0.0f, 1.0f);
                float pip = kNorm;
                bool hasPip = false;
                if (props.contains ("activeStepVal"))
                {
                    float rVal = (float) (double) props["activeStepVal"];
                    pip = std::clamp (kNorm + amt * (rVal - kNorm), 0.0f, 1.0f);
                    hasPip = true;
                }
                drawArcAndPip (lo, hi, pip, hasPip, Colours::kBurble);
            }
        }
    }

public:
    juce::Colour accentColour_{Colours::kOscCore};
    juce::Colour modRingDefaultColour_{Colours::kModMacro};
    juce::Colour randRingDefaultColour_{Colours::kBurble};

protected:
    juce::Path arcPath;    // reused every paint - no per-frame Path allocation growth
    juce::Path arrowPath;
    juce::Path needlePath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CrackedChimesLookAndFeel)
};
} // namespace CrackedChimes::UI
