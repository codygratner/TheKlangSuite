#include "HasturEditor.h"

// ==============================================================================
// Waveform Geometry & Visual Vector Helpers
// ==============================================================================
namespace HasturWaveUI {

juce::Path createWaveformPath(int idx, juce::Rectangle<float> r, bool isLfo) {
    juce::Path p;
    float x = r.getX();
    float y = r.getY();
    float w = r.getWidth();
    float h = r.getHeight();
    float midY = y + h * 0.5f;

    if (isLfo) {
        switch (idx) {
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
                for (int s = 1; s <= 24; ++s) {
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
    switch (idx) {
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
            for (int i = 0; i < 4; ++i) {
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

juce::Path createPwmExcursionPath(int idx, juce::Rectangle<float> r) {
    juce::Path p;
    float x = r.getX();
    float y = r.getY();
    float w = r.getWidth();
    float h = r.getHeight();

    if (idx == 2) {
        // Pulse 3 PWM excursion: 50% to 92%
        p.startNewSubPath(x + w * 0.5f, y + 3.0f);
        p.lineTo(x + w * 0.92f, y + 3.0f);
        p.lineTo(x + w * 0.92f, y + h - 3.0f);
    } else if (idx == 5) {
        // Saw 3 PWM excursion
        p.startNewSubPath(x + w * 0.35f, y + h * 0.4f);
        p.lineTo(x + w * 0.15f, y + h * 0.6f);
        p.lineTo(x + w * 0.15f, y + h - 3.0f);
    }
    return p;
}

} // namespace HasturWaveUI

// ==============================================================================
// Waveform Picker Button Implementation
// ==============================================================================
void WaveformPickerButton::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xFF161B22));
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(juce::Colour(0xFF30363D));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    auto previewArea = bounds.removeFromLeft(bounds.getHeight()).reduced(4.0f);
    auto path = HasturWaveUI::createWaveformPath(waveIndex, previewArea, isLfo);
    g.setColour(accentColour);
    g.strokePath(path, juce::PathStrokeType(1.8f));

    g.setFont(juce::Font(juce::FontOptions(11.5f).withStyle("Bold")));
    g.setColour(juce::Colour(0xFFF0F6FC));
    g.drawText(waveName, static_cast<int>(bounds.getX() + 6.0f), 0,
               static_cast<int>(bounds.getWidth() - 24.0f), static_cast<int>(bounds.getHeight()),
               juce::Justification::centredLeft);

    // Dropdown arrow indicator
    g.setColour(juce::Colour(0xFF8B949E));
    float arrX = bounds.getRight() - 14.0f;
    float arrY = bounds.getCentreY() - 2.0f;
    juce::Path arr;
    arr.startNewSubPath(arrX, arrY);
    arr.lineTo(arrX + 8.0f, arrY);
    arr.lineTo(arrX + 4.0f, arrY + 5.0f);
    arr.closeSubPath();
    g.fillPath(arr);
}

// ==============================================================================
// HasturLookAndFeel Implementation
// ==============================================================================
HasturLookAndFeel::HasturLookAndFeel() {
    setColour(juce::Slider::rotarySliderFillColourId, accentColour_);
    setColour(juce::Slider::thumbColourId, juce::Colour(0xFFF0F6FC));
}

void HasturLookAndFeel::setAccentColour(juce::Colour c) {
    accentColour_ = c;
    setColour(juce::Slider::rotarySliderFillColourId, c);
}

void HasturLookAndFeel::drawKiloheartsValueBadge(juce::Graphics& g, const juce::Rectangle<float>& bounds,
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

void HasturLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPosProportional, float rotaryStartAngle,
                                         float rotaryEndAngle, juce::Slider& slider) {
    auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                         static_cast<float>(width), static_cast<float>(height)).reduced(2.0f);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const bool isCutoff = slider.getProperties().contains("isCutoffDial");
    if (isCutoff) {
        radius = 34.0f; // Lock core dial and track radius to standard 72px dimensions
    }
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
    const bool isBipolar = (slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0);
    if (isBipolar) {
        const float zeroNorm = static_cast<float>(-slider.getMinimum() / (slider.getMaximum() - slider.getMinimum()));
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

    // =========================================================================
    // VISUAL MODULATION RADAR TELEMETRY
    // =========================================================================

    // 1. Stepped Random Modulation Window & Active Sample Pip
    if (slider.getProperties().contains("stepRandAmount")) {
        float randAmt = static_cast<float>(slider.getProperties().getWithDefault("stepRandAmount", 0.0));
        if (randAmt > 0.001f) {
            float kNorm = sliderPosProportional;
            float lowNorm = std::clamp(kNorm * (1.0f - randAmt), 0.0f, 1.0f);
            float highNorm = std::clamp(lowNorm + randAmt, 0.0f, 1.0f);

            float arcStart = rotaryStartAngle + lowNorm * sweep;
            float arcEnd   = rotaryStartAngle + highNorm * sweep;

            const auto yellowCol = HasturColours::kLfo; // 0xFFF2E10F

            juce::Path randArc;
            randArc.addCentredArc(centre.x, centre.y, radius - 1.5f, radius - 1.5f, 0.0f, arcStart, arcEnd, true);
            g.setColour(yellowCol.withAlpha(0.25f));
            g.strokePath(randArc, juce::PathStrokeType(4.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.setColour(yellowCol.withAlpha(0.95f));
            g.strokePath(randArc, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            if (slider.getProperties().contains("activeStepVal")) {
                float rVal = static_cast<float>(slider.getProperties().getWithDefault("activeStepVal", 0.0));
                float mNorm = std::clamp(kNorm + randAmt * (rVal - kNorm), 0.0f, 1.0f);
                float pipAngle = rotaryStartAngle + mNorm * sweep;
                float pipR = radius - 1.5f;
                float pipX = centre.x + pipR * std::sin(pipAngle);
                float pipY = centre.y - pipR * std::cos(pipAngle);

                g.setColour(juce::Colours::white);
                g.fillEllipse(pipX - 2.5f, pipY - 2.5f, 5.0f, 5.0f);
                g.setColour(yellowCol);
                g.drawEllipse(pipX - 3.5f, pipY - 3.5f, 7.0f, 7.0f, 1.0f);
            }
        }
    }

    // 2. DCO Morph LFO Arc (Bounded Headroom Shift Parity with HasturVoice.h:561)
    if (slider.getProperties().contains("morphLfoAmount")) {
        float lfoAmt = static_cast<float>(slider.getProperties().getWithDefault("morphLfoAmount", 0.0));
        if (lfoAmt > 0.001f) {
            float m = sliderPosProportional;
            float lo = std::clamp(m * (1.0f - lfoAmt), 0.0f, 1.0f);
            float hi = std::clamp(m + lfoAmt * (1.0f - m), 0.0f, 1.0f);

            float arcStart = rotaryStartAngle + lo * sweep;
            float arcEnd   = rotaryStartAngle + hi * sweep;

            const auto yellowCol = HasturColours::kLfo;
            juce::Path morphArc;
            morphArc.addCentredArc(centre.x, centre.y, radius - 1.5f, radius - 1.5f, 0.0f, arcStart, arcEnd, true);
            g.setColour(yellowCol.withAlpha(0.25f));
            g.strokePath(morphArc, juce::PathStrokeType(4.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.setColour(yellowCol.withAlpha(0.95f));
            g.strokePath(morphArc, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
    }

    // 3. Orbit LFO Arc (Bounded Headroom Shift Parity with HasturVoice.h:605)
    if (slider.getProperties().contains("orbitLfoAmount")) {
        float lfoAmt = static_cast<float>(slider.getProperties().getWithDefault("orbitLfoAmount", 0.0));
        if (lfoAmt > 0.001f) {
            float k = sliderPosProportional;
            float lo = std::clamp(k * (1.0f - lfoAmt), 0.0f, 1.0f);
            float hi = std::clamp(k + lfoAmt * (1.0f - k), 0.0f, 1.0f);

            float arcStart = rotaryStartAngle + lo * sweep;
            float arcEnd   = rotaryStartAngle + hi * sweep;

            const auto yellowCol = HasturColours::kLfo;
            juce::Path orbitArc;
            orbitArc.addCentredArc(centre.x, centre.y, radius - 1.5f, radius - 1.5f, 0.0f, arcStart, arcEnd, true);
            g.setColour(yellowCol.withAlpha(0.25f));
            g.strokePath(orbitArc, juce::PathStrokeType(4.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.setColour(yellowCol.withAlpha(0.95f));
            g.strokePath(orbitArc, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
    }

    // 4. Master Volume AM X-Mod Arc (Bounded Headroom Shift)
    if (slider.getProperties().contains("amXModAmount")) {
        float amAmt = static_cast<float>(slider.getProperties().getWithDefault("amXModAmount", 0.0));
        if (std::abs(amAmt) > 0.001f) {
            int amSrc = static_cast<int>(slider.getProperties().getWithDefault("amXModSource", 2));
            juce::Colour amCol = (amSrc == 2) ? HasturColours::kSub : HasturColours::kDco;

            float vNorm = sliderPosProportional;
            float loNorm = std::clamp(vNorm * (1.0f - amAmt), 0.0f, 1.0f);
            float hiNorm = std::clamp(vNorm + amAmt * (1.0f - vNorm), 0.0f, 1.0f);

            float a1 = rotaryStartAngle + loNorm * sweep;
            float a2 = rotaryStartAngle + hiNorm * sweep;

            juce::Path amArc;
            amArc.addCentredArc(centre.x, centre.y, radius - 1.5f, radius - 1.5f, 0.0f, a1, a2, true);
            g.setColour(amCol.withAlpha(0.25f));
            g.strokePath(amArc, juce::PathStrokeType(4.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.setColour(amCol.withAlpha(0.95f));
            g.strokePath(amArc, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
    }

    // 5. Filter Cutoff Multi-Ring Concentric Telemetry Arcs
    if (isCutoff) {
        float kNorm = sliderPosProportional;
        float thetaK = rotaryStartAngle + kNorm * sweep;

        // Ring 1: Env Depth (Inner, R = 32.0f)
        float envAmt = static_cast<float>(slider.getProperties().getWithDefault("cutoffEnvDepth", 0.0));
        if (envAmt > 0.001f) {
            float hiNorm = std::min(1.0f, kNorm + envAmt);
            float aEnd = rotaryStartAngle + hiNorm * sweep;
            if (aEnd > thetaK + 0.005f) {
                juce::Path arc;
                arc.addCentredArc(centre.x, centre.y, 32.0f, 32.0f, 0.0f, thetaK, aEnd, true);
                g.setColour(HasturColours::kVcf.withAlpha(0.90f)); // Acid Green
                g.strokePath(arc, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        }

        // Ring 2: LFO Depth (R = 35.0f, only if cutoffLfoDest == 0)
        int lfoDest = static_cast<int>(slider.getProperties().getWithDefault("cutoffLfoDest", 0));
        float lfoAmt = static_cast<float>(slider.getProperties().getWithDefault("cutoffLfoDepth", 0.0));
        if (lfoDest == 0 && lfoAmt > 0.001f) {
            float loNorm = std::clamp(kNorm * (1.0f - lfoAmt), 0.0f, 1.0f);
            float hiNorm = std::clamp(kNorm + lfoAmt * (1.0f - kNorm), 0.0f, 1.0f);
            float a1 = rotaryStartAngle + loNorm * sweep;
            float a2 = rotaryStartAngle + hiNorm * sweep;
            if (a2 > a1 + 0.005f) {
                juce::Path arc;
                arc.addCentredArc(centre.x, centre.y, 35.0f, 35.0f, 0.0f, a1, a2, true);
                g.setColour(HasturColours::kLfo.withAlpha(0.90f)); // Caustic Yellow
                g.strokePath(arc, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        }

        // Ring 3: Burble Depth (R = 38.0f)
        float burbleAmt = static_cast<float>(slider.getProperties().getWithDefault("cutoffBurbleDepth", 0.0));
        if (std::abs(burbleAmt) > 0.001f) {
            float tgtNorm = (burbleAmt >= 0.0f)
                ? std::clamp(kNorm + burbleAmt * (1.0f - kNorm), 0.0f, 1.0f)
                : std::clamp(kNorm + burbleAmt * kNorm, 0.0f, 1.0f);
            float aTgt = rotaryStartAngle + tgtNorm * sweep;
            float a1 = std::min(thetaK, aTgt);
            float a2 = std::max(thetaK, aTgt);
            if (a2 > a1 + 0.005f) {
                juce::Path arc;
                arc.addCentredArc(centre.x, centre.y, 38.0f, 38.0f, 0.0f, a1, a2, true);
                g.setColour(HasturColours::kSub.withAlpha(0.90f)); // Movie Poster Orange
                g.strokePath(arc, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        }

        // Ring 4: FM X-Mod Depth (R = 41.0f)
        float fmAmt = static_cast<float>(slider.getProperties().getWithDefault("cutoffFmDepth", 0.0));
        if (std::abs(fmAmt) > 0.001f) {
            int fmSrc = static_cast<int>(slider.getProperties().getWithDefault("cutoffFmSource", 0));
            juce::Colour fmCol = (fmSrc == 2) ? HasturColours::kNoise : HasturColours::kDco;

            float tgtNorm = (fmAmt >= 0.0f)
                ? std::clamp(kNorm + fmAmt * (1.0f - kNorm), 0.0f, 1.0f)
                : std::clamp(kNorm + fmAmt * kNorm, 0.0f, 1.0f);
            float aTgt = rotaryStartAngle + tgtNorm * sweep;
            float a1 = std::min(thetaK, aTgt);
            float a2 = std::max(thetaK, aTgt);
            if (a2 > a1 + 0.005f) {
                juce::Path arc;
                arc.addCentredArc(centre.x, centre.y, 41.0f, 41.0f, 0.0f, a1, a2, true);
                g.setColour(fmCol.withAlpha(0.90f)); // Blue or Purple
                g.strokePath(arc, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        }
    }

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
        drawKiloheartsValueBadge(g, bounds, text, fillCol);
    }
}

void HasturLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                                         const juce::Slider::SliderStyle style, juce::Slider& slider) {
    const bool isBipolar = (slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0);
    const auto trackCol = slider.isColourSpecified(juce::Slider::trackColourId)
        ? slider.findColour(juce::Slider::trackColourId) : accentColour_;
    const auto thumbCol = slider.isColourSpecified(juce::Slider::thumbColourId)
        ? slider.findColour(juce::Slider::thumbColourId) : accentColour_;

    if (style == juce::Slider::LinearHorizontal) {
        float trackY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;

        // Horizontal Track Groove
        g.setColour(juce::Colour(0xFF0F131A));
        g.fillRoundedRectangle(static_cast<float>(x), trackY - 3.0f, static_cast<float>(width), 6.0f, 3.0f);
        g.setColour(juce::Colour(0xFF262D38));
        g.drawRoundedRectangle(static_cast<float>(x), trackY - 3.0f, static_cast<float>(width), 6.0f, 3.0f, 1.0f);

        // Active fill (Bipolar from midX, Unipolar from leftX)
        float leftX = static_cast<float>(x);
        g.setColour(trackCol.withAlpha(0.75f));
        if (isBipolar) {
            float zeroNorm = static_cast<float>(-slider.getMinimum() / (slider.getMaximum() - slider.getMinimum()));
            float midX = leftX + zeroNorm * static_cast<float>(width);
            if (sliderPos > midX) {
                g.fillRoundedRectangle(midX, trackY - 2.0f, sliderPos - midX, 4.0f, 2.0f);
            } else if (sliderPos < midX) {
                g.fillRoundedRectangle(sliderPos, trackY - 2.0f, midX - sliderPos, 4.0f, 2.0f);
            }
        } else {
            if (sliderPos > leftX) {
                g.fillRoundedRectangle(leftX, trackY - 2.0f, sliderPos - leftX, 4.0f, 2.0f);
            }
        }

        // Horizontal Fader Cap / Thumb
        float capW = 12.0f;
        float capH = static_cast<float>(height) - 4.0f;
        auto capBounds = juce::Rectangle<float>(sliderPos - capW * 0.5f, trackY - capH * 0.5f, capW, capH);

        g.setColour(juce::Colour(0xFF21262D));
        g.fillRoundedRectangle(capBounds, 3.0f);
        g.setColour(juce::Colour(0xFF484F58));
        g.drawRoundedRectangle(capBounds, 3.0f, 1.0f);

        // Center vertical indicator line on fader cap
        g.setColour(thumbCol);
        g.fillRect(sliderPos - 1.0f, capBounds.getY() + 3.0f, 2.0f, capBounds.getHeight() - 6.0f);
        return;
    }

    // Vertical Fader (Stem Mixer)
    auto trackX = x + width * 0.5f;

    // Track Groove
    g.setColour(juce::Colour(0xFF0F131A));
    g.fillRoundedRectangle(trackX - 3.0f, static_cast<float>(y), 6.0f, static_cast<float>(height), 3.0f);
    g.setColour(juce::Colour(0xFF262D38));
    g.drawRoundedRectangle(trackX - 3.0f, static_cast<float>(y), 6.0f, static_cast<float>(height), 3.0f, 1.0f);

    // Active fill
    float bottomY = static_cast<float>(y + height);
    g.setColour(trackCol.withAlpha(0.75f));
    if (isBipolar) {
        float zeroNorm = static_cast<float>(-slider.getMinimum() / (slider.getMaximum() - slider.getMinimum()));
        float midY = static_cast<float>(y) + (1.0f - zeroNorm) * static_cast<float>(height);
        if (sliderPos < midY) {
            g.fillRoundedRectangle(trackX - 2.0f, sliderPos, 4.0f, midY - sliderPos, 2.0f);
        } else if (sliderPos > midY) {
            g.fillRoundedRectangle(trackX - 2.0f, midY, 4.0f, sliderPos - midY, 2.0f);
        }
    } else {
        if (sliderPos < bottomY) {
            g.fillRoundedRectangle(trackX - 2.0f, sliderPos, 4.0f, bottomY - sliderPos, 2.0f);
        }
    }

    // Vertical Fader Cap
    float capW = std::clamp(static_cast<float>(width) - 6.0f, 28.0f, 36.0f);
    float capH = 14.0f;
    auto capBounds = juce::Rectangle<float>(trackX - capW * 0.5f, sliderPos - capH * 0.5f, capW, capH);

    g.setColour(juce::Colour(0xFF21262D));
    g.fillRoundedRectangle(capBounds, 3.0f);
    g.setColour(juce::Colour(0xFF484F58));
    g.drawRoundedRectangle(capBounds, 3.0f, 1.0f);

    // Center indicator line on fader cap
    g.setColour(thumbCol);
    g.fillRect(capBounds.getX() + 3.0f, sliderPos - 1.0f, capBounds.getWidth() - 6.0f, 2.0f);
}

void HasturLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
                                     int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                                     juce::ComboBox& /*box*/) {
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    g.setColour(juce::Colour(0xFF161B22));
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(juce::Colour(0xFF30363D));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    // Dropdown arrow
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

// ==============================================================================
// KnobTaperModal Implementation
// ==============================================================================
KnobTaperModal::KnobTaperModal(juce::Slider& targetSlider, const juce::String& paramName, juce::Colour accent)
    : slider_(targetSlider), paramName_(paramName), accent_(accent)
{
    setAlwaysOnTop(true);

    titleLabel.setText("CALIBRATE KNOB TAPER: " + paramName, juce::dontSendNotification);
    titleLabel.setFont(juce::Font(juce::FontOptions(14.0f).withStyle("Bold")));
    titleLabel.setColour(juce::Label::textColourId, accent);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    auto setupRow = [this](juce::Label& lbl, const juce::String& txt, juce::TextEditor& ed) {
        lbl.setText(txt, juce::dontSendNotification);
        lbl.setFont(juce::Font(juce::FontOptions(12.0f).withStyle("Bold")));
        lbl.setColour(juce::Label::textColourId, juce::Colour(0xFF8B949E));
        addAndMakeVisible(lbl);

        ed.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xFF0F131A));
        ed.setColour(juce::TextEditor::textColourId, juce::Colour(0xFFF0F6FC));
        ed.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xFF30363D));
        addAndMakeVisible(ed);
    };

    setupRow(startLabel, "MINIMUM (0.0):", startEditor);
    setupRow(midLabel,   "CENTER (0.5):",  midEditor);
    setupRow(endLabel,   "MAXIMUM (1.0):", endEditor);

    startEditor.setText(juce::String(slider_.getMinimum()));
    endEditor.setText(juce::String(slider_.getMaximum()));
    midEditor.setText(juce::String((slider_.getMinimum() + slider_.getMaximum()) * 0.5));

    curveLabel.setText("RESPONSE CURVE:", juce::dontSendNotification);
    curveLabel.setFont(juce::Font(juce::FontOptions(12.0f).withStyle("Bold")));
    curveLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF8B949E));
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

void KnobTaperModal::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xB8080B10));

    auto dBox = juce::Rectangle<float>(getWidth() * 0.5f - 170.0f, getHeight() * 0.5f - 130.0f, 340.0f, 260.0f);
    g.setColour(juce::Colour(0xFF161B22));
    g.fillRoundedRectangle(dBox, 8.0f);
    g.setColour(accent_);
    g.drawRoundedRectangle(dBox, 8.0f, 1.5f);
}

void KnobTaperModal::resized() {
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

void KnobTaperModal::mouseDown(const juce::MouseEvent& e) {
    auto dBox = juce::Rectangle<int>(getWidth() / 2 - 170, getHeight() / 2 - 130, 340, 260);
    if (!dBox.contains(e.getPosition())) {
        if (onDismiss) onDismiss();
    }
}

// ==============================================================================
// WaveformGridModal Implementation
// ==============================================================================
WaveformGridModal::WaveformGridModal(bool isLfo, int currentSelection, juce::Colour accent,
                                     std::function<void(int)> onSelectWave)
    : isLfo_(isLfo), currentSelection_(currentSelection), accent_(accent), onSelectWave_(onSelectWave)
{
    setAlwaysOnTop(true);

    titleLabel.setText(isLfo ? "SELECT LFO WAVEFORM" : "SELECT OSCILLATOR WAVEFORM", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(juce::FontOptions(15.0f).withStyle("Bold")));
    titleLabel.setColour(juce::Label::textColourId, accent);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    closeButton.setButtonText(juce::CharPointer_UTF8("\xE2\x9C\x95"));
    closeButton.onClick = [this]() {
        if (onDismiss) onDismiss();
    };
    addAndMakeVisible(closeButton);
}

void WaveformGridModal::resized() {
    int w = isLfo_ ? 560 : 760;
    int h = isLfo_ ? 180 : 430;
    dialogBounds_ = juce::Rectangle<int>(getWidth() / 2 - w / 2, getHeight() / 2 - h / 2, w, h);

    titleLabel.setBounds(dialogBounds_.getX() + 20, dialogBounds_.getY() + 10, dialogBounds_.getWidth() - 60, 24);
    closeButton.setBounds(dialogBounds_.getRight() - 32, dialogBounds_.getY() + 8, 24, 24);
}

int WaveformGridModal::getTileIndexAt(juce::Point<int> pt) const {
    if (!dialogBounds_.contains(pt)) return -1;

    int cols = isLfo_ ? 5 : 5;
    int rows = isLfo_ ? 1 : 3;
    int total = isLfo_ ? 5 : 14;

    auto gridArea = dialogBounds_.reduced(16).withTrimmedTop(28);
    float tileW = gridArea.getWidth() / static_cast<float>(cols);
    float tileH = gridArea.getHeight() / static_cast<float>(rows);

    int col = static_cast<int>((pt.x - gridArea.getX()) / tileW);
    int row = static_cast<int>((pt.y - gridArea.getY()) / tileH);

    if (col >= 0 && col < cols && row >= 0 && row < rows) {
        int idx = row * cols + col;
        if (idx >= 0 && idx < total) return idx;
    }
    return -1;
}

void WaveformGridModal::mouseMove(const juce::MouseEvent& e) {
    int idx = getTileIndexAt(e.getPosition());
    if (idx != hoveredIdx_) {
        hoveredIdx_ = idx;
        repaint();
    }
}

void WaveformGridModal::mouseDown(const juce::MouseEvent& e) {
    if (!dialogBounds_.contains(e.getPosition())) {
        if (onDismiss) onDismiss();
        return;
    }

    int idx = getTileIndexAt(e.getPosition());
    if (idx >= 0) {
        if (onSelectWave_) onSelectWave_(idx);
        if (onDismiss) onDismiss();
    }
}

void WaveformGridModal::paint(juce::Graphics& g) {
    g.setColour(juce::Colour(0xB8080B10));
    g.fillRect(getLocalBounds());

    g.setColour(juce::Colour(0xFF161B22));
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

    int cols = isLfo_ ? 5 : 5;
    int rows = isLfo_ ? 1 : 3;
    int total = isLfo_ ? 5 : 14;

    auto gridArea = dialogBounds_.reduced(16).withTrimmedTop(28);
    float tileW = gridArea.getWidth() / static_cast<float>(cols);
    float tileH = gridArea.getHeight() / static_cast<float>(rows);

    for (int i = 0; i < total; ++i) {
        int r = i / cols;
        int c = i % cols;
        auto tileRect = juce::Rectangle<float>(gridArea.getX() + c * tileW, gridArea.getY() + r * tileH, tileW, tileH).reduced(4.0f);

        bool isSelected = (i == currentSelection_);
        bool isHovered = (i == hoveredIdx_);

        g.setColour(isSelected ? juce::Colour(0xFF21262D) : (isHovered ? juce::Colour(0xFF1B2028) : juce::Colour(0xFF0F131A)));
        g.fillRoundedRectangle(tileRect, 4.0f);

        if (isSelected) {
            g.setColour(accent_);
            g.drawRoundedRectangle(tileRect, 4.0f, 2.0f);
        } else if (isHovered) {
            g.setColour(accent_.withAlpha(0.6f));
            g.drawRoundedRectangle(tileRect, 4.0f, 1.0f);
        } else {
            g.setColour(juce::Colour(0xFF262D38));
            g.drawRoundedRectangle(tileRect, 4.0f, 1.0f);
        }

        auto waveCanvas = tileRect.reduced(6.0f).withTrimmedBottom(22.0f);
        auto path = HasturWaveUI::createWaveformPath(i, waveCanvas, isLfo_);
        g.setColour(isSelected ? accent_ : juce::Colour(0xFFD0D7DE));
        g.strokePath(path, juce::PathStrokeType(1.8f));

        if (!isLfo_) {
            auto pwmPath = HasturWaveUI::createPwmExcursionPath(i, waveCanvas);
            if (!pwmPath.isEmpty()) {
                juce::PathStrokeType dashedStroke(1.2f);
                const float dashes[] = { 3.0f, 3.0f };
                dashedStroke.createDashedStroke(pwmPath, pwmPath, dashes, 2);
                g.setColour(accent_.withAlpha(0.65f));
                g.strokePath(pwmPath, juce::PathStrokeType(1.2f));
            }
        }

        g.setFont(juce::Font(juce::FontOptions(10.5f).withStyle("Bold")));
        g.setColour(isSelected ? accent_ : juce::Colour(0xFFF0F6FC));
        juce::String name = isLfo_ ? kLfoWaves[i] : kAlphaWaves[i];
        g.drawText(name, static_cast<int>(tileRect.getX()), static_cast<int>(tileRect.getBottom() - 20.0f),
                   static_cast<int>(tileRect.getWidth()), 18, juce::Justification::centred);
    }
}

// ==============================================================================
// Master Hastur Audio Processor Editor Implementation
// ==============================================================================

HasturAudioProcessorEditor::HasturAudioProcessorEditor(HasturAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processor_(p)
{
    setLookAndFeel(&lnf_);

    // 1. Hardwire 4:3 Aspect Ratio Lock for Renoise & Host DAW
    constrainer.setFixedAspectRatio(4.0 / 3.0);
    constrainer.setSizeLimits(juce::roundToInt(kBaseWidth  * rlyeh::ui::EditorScaleManager::kMinScale),
                              juce::roundToInt(kBaseHeight * rlyeh::ui::EditorScaleManager::kMinScale),
                              juce::roundToInt(kBaseWidth  * rlyeh::ui::EditorScaleManager::kMaxScale),
                              juce::roundToInt(kBaseHeight * rlyeh::ui::EditorScaleManager::kMaxScale));
    setConstrainer(&constrainer);

    // 2. Arm Resizable Gripper (constrainer handles resize limits)
    setResizable(true, true);

    // 3. Resolve preferred scale (Session XML > Global JSON > Default)
    const float sessionScale = processor_.getStoredEditorScale();
    prefScale_ = rlyeh::ui::EditorScaleManager::getPreferredScale("hastur", sessionScale);
    effScale_ = prefScale_;
    expectedSize_ = { juce::roundToInt(kBaseWidth * effScale_), juce::roundToInt(kBaseHeight * effScale_) };
    setSize(expectedSize_.x, expectedSize_.y);

    // 4. Mount Content Container
    addAndMakeVisible(contentContainer);
    contentContainer.setBounds(0, 0, kBaseWidth, kBaseHeight);
    contentContainer.setInterceptsMouseClicks(false, true);
    contentContainer.onPaint = [this](juce::Graphics& g) {
        paintCanvas(g);
    };

    headerLabel.setText("HASTUR", juce::dontSendNotification);
    headerLabel.setFont(juce::Font(juce::FontOptions(28.0f).withStyle("Bold")));
    headerLabel.setJustificationType(juce::Justification::centredLeft);
    contentContainer.addAndMakeVisible(headerLabel);
    contentContainer.addAndMakeVisible(presetStub);
    contentContainer.addAndMakeVisible(headerInspector);
    contentContainer.addMouseListener(this, true);

    contentContainer.addAndMakeVisible(lifeModeToggle);
    headerLabel.setColour(juce::Label::textColourId, getAccentColour());
    presetStub.setAccentColour(getAccentColour());

    // =========================================================================
    // BAY 1: DCO & SUB-STATOR (x: 14, y: 44, w: 364, h: 490)
    // =========================================================================
    // =========================================================================
    // BAY 1: DCO & SUB-STATOR ATTACHMENTS & CONFIGURATION
    // =========================================================================
    dcoScope = std::make_unique<DcoWaveformVisualizer>(processor_);
    contentContainer.addAndMakeVisible(*dcoScope);
    subScope = std::make_unique<SubWaveformVisualizer>(processor_);
    contentContainer.addAndMakeVisible(*subScope);

    auto formatTime = [](double sec) -> juce::String {
        if (sec >= 1.0)
            return juce::String::formatted("%.2f s", sec);
        if (sec >= 0.1)
            return juce::String::formatted("%.0f ms", sec * 1000.0);
        return juce::String::formatted("%.1f ms", sec * 1000.0);
    };

    setupRotaryKnob(dcoMorphKnob, "dco_morph", "MORPH", HasturColours::kDco);
    dcoMorphKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%.0f%%", val * 100.0);
    };
    dcoMorphKnob.slider.updateText();

    setupRotaryKnob(pwLfoAmountKnob, "pw_lfo_amount", "AMT", HasturColours::kLfo);
    pwLfoAmountKnob.slider.setDoubleClickReturnValue(true, 0.0f);
    pwLfoAmountKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%.0f%%", val * 100.0);
    };
    pwLfoAmountKnob.slider.updateText();

    setupRotaryKnob(pwLfoRateKnob, "pw_lfo_rate", "RATE", HasturColours::kLfo);
    pwLfoRateKnob.slider.textFromValueFunction = [](double val) {
        if (val < 1.0)
            return juce::String::formatted("%.2f Hz", val);
        return juce::String::formatted("%.1f Hz", val);
    };
    pwLfoRateKnob.slider.updateText();

    setupButtonBar(dcoFamilyButtons, dcoFamilyLabel, "dco_wave_family", "WAVE", HasturColours::kDco);
    dcoFamilyButtons.setGlyphProviders({
        // Saw
        [](juce::Rectangle<float> r) {
            juce::Path p;
            p.startNewSubPath(r.getX(), r.getBottom() - 1.0f);
            p.lineTo(r.getRight() - 1.0f, r.getY() + 1.0f);
            p.lineTo(r.getRight() - 1.0f, r.getBottom() - 1.0f);
            return p;
        },
        // Pulse
        [](juce::Rectangle<float> r) {
            juce::Path p;
            float midX = r.getX() + r.getWidth() * 0.5f;
            p.startNewSubPath(r.getX(), r.getBottom() - 1.0f);
            p.lineTo(r.getX(), r.getY() + 1.0f);
            p.lineTo(midX, r.getY() + 1.0f);
            p.lineTo(midX, r.getBottom() - 1.0f);
            p.lineTo(r.getRight(), r.getBottom() - 1.0f);
            return p;
        },
        // Triangle: crisp centered /\ (one full period: rise to apex, fall to baseline)
        [](juce::Rectangle<float> r) {
            juce::Path p;
            p.startNewSubPath(r.getX(), r.getBottom() - 1.0f);
            p.lineTo(r.getCentreX(), r.getY() + 1.0f);
            p.lineTo(r.getRight(), r.getBottom() - 1.0f);
            return p;
        },
        // Sine: smooth full-period wave, crest and trough inside bounds
        [](juce::Rectangle<float> r) {
            juce::Path p;
            const float midY = r.getCentreY();
            const float amp  = r.getHeight() * 0.5f - 1.0f;
            constexpr int kSegs = 24;
            for (int i = 0; i <= kSegs; ++i) {
                const float t = static_cast<float>(i) / kSegs;
                const float px = r.getX() + t * r.getWidth();
                const float py = midY - std::sin(t * 6.2831853f) * amp;
                if (i == 0) p.startNewSubPath(px, py);
                else        p.lineTo(px, py);
            }
            return p;
        }
    });

    setupButtonBar(dcoTopoButtons, dcoTopoLabel, "dco_chopper_topo", "CHOP", HasturColours::kDco);
    // 5 Full Octaves of Pitch Strangle (1.0x to 32.0x)
    setupRotaryKnob(osc1MultKnob, "osc1_mult", "STRANGLE", HasturColours::kDco);
    osc1MultKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%.1fx", val);
    };
    osc1MultKnob.slider.updateText();

    setupRotaryKnob(osc1ApfKnob, "osc1_apf_pitch_track", "WARP", HasturColours::kDco);
    osc1ApfKnob.slider.setDoubleClickReturnValue(true, 0.0f);
    osc1ApfKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%+.2f Oct", val);
    };
    osc1ApfKnob.slider.updateText();

    setupRotaryKnob(osc1DriveKnob, "osc1_drive", "MELT", HasturColours::kDco);
    osc1DriveKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%.0f%%", val * 100.0);
    };
    osc1DriveKnob.slider.updateText();

    setupRotaryKnob(subApfDepthKnob, "sub_apf_depth", "CHEW", HasturColours::kSub);
    subApfDepthKnob.slider.setDoubleClickReturnValue(true, 0.0f);
    subApfDepthKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%+.2f Oct", val);
    };
    subApfDepthKnob.slider.updateText();

    setupRotaryKnob(subSpitKnob, "sub_spit", "SPIT", HasturColours::kSub);
    subSpitKnob.slider.setDoubleClickReturnValue(true, 0.0f);
    subSpitKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%+.0f%%", val * 100.0);
    };
    subSpitKnob.slider.updateText();

    setupRotaryKnob(heroBiteKnob, "dco_bite", "BITE", HasturColours::kDco);
    heroBiteKnob.slider.setDoubleClickReturnValue(true, 0.0f);
    heroBiteKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%+.2f", val);
    };
    heroBiteKnob.slider.updateText();

    setupButtonBar(subOctaveButtons, subOctaveLabel, "sub_octave", "SUB", HasturColours::kSub);
    setupButtonBar(subApfTapButtons, subApfTapLabel, "sub_apf_tap", "TAP", HasturColours::kSub);

    subTrainGrid.setAccentColour(HasturColours::kSub);
    subTrainGrid.setGlyphProviders({
        // SQR: 50% Duty Square
        [](juce::Rectangle<float> r) {
            juce::Path p;
            float midX = r.getX() + r.getWidth() * 0.5f;
            p.startNewSubPath(r.getX(), r.getBottom() - 1.0f);
            p.lineTo(r.getX(), r.getY() + 1.0f);
            p.lineTo(midX, r.getY() + 1.0f);
            p.lineTo(midX, r.getBottom() - 1.0f);
            p.lineTo(r.getRight(), r.getBottom() - 1.0f);
            return p;
        },
        // PLS: 25% Narrow Pulse
        [](juce::Rectangle<float> r) {
            juce::Path p;
            float pulseW = r.getWidth() * 0.28f;
            p.startNewSubPath(r.getX(), r.getBottom() - 1.0f);
            p.lineTo(r.getX(), r.getY() + 1.0f);
            p.lineTo(r.getX() + pulseW, r.getY() + 1.0f);
            p.lineTo(r.getX() + pulseW, r.getBottom() - 1.0f);
            p.lineTo(r.getRight(), r.getBottom() - 1.0f);
            return p;
        },
        // DUO: Two Narrow Pulses
        [](juce::Rectangle<float> r) {
            juce::Path p;
            float w = r.getWidth();
            float pW = w * 0.18f;
            p.startNewSubPath(r.getX(), r.getBottom() - 1.0f);
            p.lineTo(r.getX(), r.getY() + 1.0f);
            p.lineTo(r.getX() + pW, r.getY() + 1.0f);
            p.lineTo(r.getX() + pW, r.getBottom() - 1.0f);
            p.lineTo(r.getX() + w * 0.5f, r.getBottom() - 1.0f);
            p.lineTo(r.getX() + w * 0.5f, r.getY() + 1.0f);
            p.lineTo(r.getX() + w * 0.5f + pW, r.getY() + 1.0f);
            p.lineTo(r.getX() + w * 0.5f + pW, r.getBottom() - 1.0f);
            p.lineTo(r.getRight(), r.getBottom() - 1.0f);
            return p;
        },
        // TRI: Three Pulses
        [](juce::Rectangle<float> r) {
            juce::Path p;
            float w = r.getWidth();
            float pW = w * 0.14f;
            float gap = (w - 3.0f * pW) / 2.0f;
            for (int i = 0; i < 3; ++i) {
                float x0 = r.getX() + i * (pW + gap);
                p.startNewSubPath(x0, r.getBottom() - 1.0f);
                p.lineTo(x0, r.getY() + 1.0f);
                p.lineTo(x0 + pW, r.getY() + 1.0f);
                p.lineTo(x0 + pW, r.getBottom() - 1.0f);
            }
            return p;
        },
        // QUD: Four Pulses
        [](juce::Rectangle<float> r) {
            juce::Path p;
            float w = r.getWidth();
            float pW = w * 0.10f;
            float gap = (w - 4.0f * pW) / 3.0f;
            for (int i = 0; i < 4; ++i) {
                float x0 = r.getX() + i * (pW + gap);
                p.startNewSubPath(x0, r.getBottom() - 1.0f);
                p.lineTo(x0, r.getY() + 1.0f);
                p.lineTo(x0 + pW, r.getY() + 1.0f);
                p.lineTo(x0 + pW, r.getBottom() - 1.0f);
            }
            return p;
        }
    });
    contentContainer.addAndMakeVisible(subTrainGrid);

    // Bay 1 bindings to inspector
    bindInspector({&dcoFamilyButtons, &dcoFamilyLabel}, "dco_wave_family", HasturColours::kDco);
    bindInspector({&dcoTopoButtons, &dcoTopoLabel}, "dco_chopper_topo", HasturColours::kDco);
    bindInspector({&subOctaveButtons, &subOctaveLabel}, "sub_octave", HasturColours::kSub);
    bindInspector({&subApfTapButtons, &subApfTapLabel}, "sub_apf_tap", HasturColours::kSub);
    bindInspector({&subTrainGrid}, "sub_pulse_train", HasturColours::kSub);

    // Bay 1 bounds handled in layoutControls() SSoT


    // =========================================================================
    // BAY 2: STEM MIXER (Functional Color: Amber Gold)
    // =========================================================================
    setupMixerSlider(osc1LevelSlider, "osc1_level", "CORE", HasturColours::kDco);
    setupMixerSlider(subLevelSlider, "osc2_level", "SUB", HasturColours::kSub);
    setupMixerSlider(noiseLevelSlider, "noise_level", "NOISE", HasturColours::kNoise);
    setupMixerSlider(mixerDriveSlider, "mixer_drive", "SEETHE", HasturColours::kMixer);

    // =========================================================================
    // BAY 2 (lower half): AUDIO-RATE X-MOD
    // =========================================================================
    filterModSourceGrid.setAccentColour(HasturColours::kVcf);
    filterModSourceGrid.setItemColours({HasturColours::kDco, HasturColours::kDco, HasturColours::kNoise});
    contentContainer.addAndMakeVisible(filterModSourceGrid);
    bindInspector({&filterModSourceGrid}, "mod_a_source", HasturColours::kVcf);

    setupRotaryKnob(filterModAmountKnob, "mod_a_amount", "FM DEPTH", HasturColours::kVcf);
    filterModAmountKnob.slider.setDoubleClickReturnValue(true, 0.0);
    filterModAmountKnob.slider.textFromValueFunction = [](double v) {
        return juce::String::formatted("%+.0f%%", v * 100.0);
    };

    ampModSourceGrid.setAccentColour(HasturColours::kVca);
    ampModSourceGrid.setItemColours({HasturColours::kDco, HasturColours::kDco, HasturColours::kSub});
    contentContainer.addAndMakeVisible(ampModSourceGrid);
    bindInspector({&ampModSourceGrid}, "mod_b_source", HasturColours::kVca);

    setupRotaryKnob(ampModAmountKnob, "mod_b_amount", "AM DEPTH", HasturColours::kVca);
    ampModAmountKnob.slider.setDoubleClickReturnValue(true, 0.0);
    ampModAmountKnob.slider.textFromValueFunction = [](double v) {
        return juce::String::formatted("%+.0f%%", v * 100.0);
    };

    // =========================================================================
    // BAY 4: STEPPED RANDOM MODULATION MATRIX (9 KNOBS)
    // =========================================================================
    struct StepRandConfig {
        const char* paramId;
        const char* name;
        juce::Colour col;
    };
    const StepRandConfig randConfigs[9] = {
        // Row 1: DCO Core (Tron Blue)
        { "rand_core_warp",    "CORE WARP",    HasturColours::kDco },
        { "rand_core_bite",    "CORE BITE",    HasturColours::kDco },
        { "rand_core_melt",    "CORE MELT",    HasturColours::kDco },
        // Row 2: Sub-Stator & Mult (Orange & Blue)
        { "rand_sub_chew",     "SUB CHEW",     HasturColours::kSub },
        { "rand_sub_spit",     "SUB SPIT",     HasturColours::kSub },
        { "rand_raw_strangle", "RAW STRANGLE", HasturColours::kDco },
        // Row 3: Filter (Acid Green)
        { "rand_filter_twist", "FILTER TWIST", HasturColours::kVcf },
        { "rand_filter_stab",  "FILTER STAB",  HasturColours::kVcf },
        { "rand_filter_orbit", "FILTER ORBIT", HasturColours::kVcf }
    };

    for (int i = 0; i < 9; ++i) {
        setupRotaryKnob(stepRandKnobs[i], randConfigs[i].paramId, randConfigs[i].name, randConfigs[i].col);
        stepRandKnobs[i].slider.setDoubleClickReturnValue(true, 0.0f);
        stepRandKnobs[i].slider.textFromValueFunction = [](double val) {
            return juce::String::formatted("%.0f%%", val * 100.0);
        };
        stepRandKnobs[i].slider.updateText();
        bindInspector({&stepRandKnobs[i].slider, &stepRandKnobs[i].label}, randConfigs[i].paramId, randConfigs[i].col);
    }

    // Voice Trigger Mode
    setupButtonBar(trigModeButtons, trigModeLabel, "trigger_mode", "TRIG", HasturColours::kOutput);
    bindInspector({&trigModeButtons, &trigModeLabel}, "trigger_mode", HasturColours::kOutput);

// PWM LFO controls migrated to Bay 1 Morph Engine Pod

    // =========================================================================
    // BAY 3: DUAL SVF VCF (Functional Color: Crimson Coral)
    // =========================================================================
    // SOTA Top Acid Pentad (5 Linear 72px Hero Knobs: CUTOFF, RES, ENV AMT, DECAY, ACCENT)
    bindInspector({&filterDeckGrid}, "filter_deck", HasturColours::kVcf);
    bindInspector({&burbleModeButtons, &burbleModeLabel}, "burble_mode", HasturColours::kSub);
    bindInspector({&filterLfoDestButtons, &filterLfoDestLabel}, "filter_lfo_dest", HasturColours::kLfo);
    bindInspector({&lfoSyncButtons, &lfoSyncLabel}, "lfo_sync", HasturColours::kLfo);
    bindInspector({&lfoFamilyGrid}, "lfo_wave", HasturColours::kLfo);
    bindInspector({&ampGateGrid}, "amp_gate_mode", HasturColours::kVca);
    bindInspector({&glideModeButtons, &glideModeLabel}, "glide_mode", HasturColours::kOutput);
    bindInspector({&lifeModeToggle}, "life_mode", HasturColours::kOutput);

    setupRotaryKnob(cutoffKnob, "cutoff", "CUTOFF", HasturColours::kVcf);
    cutoffKnob.slider.getProperties().set("isCutoffDial", true);
    cutoffKnob.slider.setPaintingIsUnclipped(true);
    cutoffKnob.slider.textFromValueFunction = [](double val) {
        if (val >= 1000.0)
            return juce::String::formatted("%.1f kHz", val / 1000.0);
        return juce::String::formatted("%.0f Hz", val);
    };
    cutoffKnob.slider.updateText();

    setupRotaryKnob(resKnob, "resonance", "RES", HasturColours::kVcf);
    resKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%.0f%%", val * 100.0);
    };
    resKnob.slider.updateText();

    setupRotaryKnob(envDepthKnob, "env_depth", "ENV AMT", HasturColours::kVcf);
    envDepthKnob.slider.setDoubleClickReturnValue(true, 0.0f);
    envDepthKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%+.0f%%", val * 100.0);
    };
    envDepthKnob.slider.updateText();

    setupRotaryKnob(filterDecayKnob, "filter_env_decay", "DECAY", HasturColours::kVcf);
    filterDecayKnob.slider.textFromValueFunction = [formatTime](double val) {
        return formatTime(val);
    };
    filterDecayKnob.slider.updateText();

    setupRotaryKnob(accentKnob, "filter_velocity", "ACCENT", HasturColours::kVcf);
    accentKnob.slider.setDoubleClickReturnValue(true, 0.0f);
    accentKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%.0f%%", val * 100.0);
    };
    accentKnob.slider.updateText();

    // Secondary Row: TWIST, STAB, SLASH, ORBIT (Standard 52px)
    setupRotaryKnob(filterWarpKnob, "filter_warp", "TWIST", HasturColours::kVcf);
    filterWarpKnob.slider.setDoubleClickReturnValue(true, 0.0);
    filterWarpKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%.2f", val);
    };
    filterWarpKnob.slider.updateText();

    setupRotaryKnob(laserBiteKnob, "laser_bite", "STAB", HasturColours::kVcf);
    laserBiteKnob.slider.setDoubleClickReturnValue(true, 0.0);
    laserBiteKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%.2f", val);
    };
    laserBiteKnob.slider.updateText();

    setupRotaryKnob(laserDriveKnob, "laser_drive", "SLASH", HasturColours::kVcf);
    laserDriveKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%.2f", val);
    };
    laserDriveKnob.slider.updateText();

    setupRotaryKnob(filterOrbitKnob, "filter_orbit", "ORBIT", HasturColours::kVcf);
    filterOrbitKnob.slider.setDoubleClickReturnValue(true, 0.0f);
    filterOrbitKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%.2f", val);
    };
    filterOrbitKnob.slider.updateText();

    setupRotaryKnob(burbleDepthKnob, "burble_depth", "BURBLE", HasturColours::kSub);
    burbleDepthKnob.slider.setDoubleClickReturnValue(true, 0.0f);
    burbleDepthKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%+.0f%%", val * 100.0);
    };
    burbleDepthKnob.slider.updateText();

    setupButtonBar(burbleModeButtons, burbleModeLabel, "burble_mode", "MODE", HasturColours::kSub);
    burbleModeButtons.setVertical(true);

    setupRotaryKnob(keyTrackKnob, "keytrack", "KEYTRACK", HasturColours::kVcf);
    keyTrackKnob.slider.setDoubleClickReturnValue(true, 0.0f);
    keyTrackKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%+.0f%%", val * 100.0);
    };
    keyTrackKnob.slider.updateText();

    filterDeckGrid.setAccentColour(HasturColours::kVcf);
    contentContainer.addAndMakeVisible(filterDeckGrid);
    // Filter LFO Engine Pod Setups
    setupRotaryKnob(lfoAmountKnob, "filter_lfo_depth", "LFO AMT", HasturColours::kLfo);
    lfoAmountKnob.slider.setDoubleClickReturnValue(true, 0.0f);
    lfoAmountKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%.0f%%", val * 100.0);
    };
    lfoAmountKnob.slider.updateText();

    setupRotaryKnob(lfoRateKnob, "lfo_rate", "RATE", HasturColours::kLfo);
    lfoRateKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%.2f Hz", val);
    };
    lfoRateKnob.slider.updateText();

    setupRotaryKnob(lfoShapeKnob, "lfo_shape", "SHAPE", HasturColours::kLfo);
    lfoShapeKnob.slider.setDoubleClickReturnValue(true, 0.5f);
    lfoShapeKnob.slider.textFromValueFunction = [this](double val) {
        int waveIdx = 0;
        if (auto* p = processor_.getAPVTS().getRawParameterValue("lfo_wave"))
            waveIdx = juce::roundToInt(p->load(std::memory_order_relaxed));
        if (waveIdx == 0 || waveIdx == 1) { // Tri / Pulse: Bipolar -100% to +100%
            const double bipVal = (val - 0.5) * 200.0;
            return juce::String::formatted("%+.0f%%", bipVal);
        }
        return juce::String::formatted("%.0f%%", val * 100.0);
    };
    lfoShapeKnob.slider.updateText();

    // LFO Wave Visualizer
    lfoWaveVisualizer = std::make_unique<LfoWaveformVisualizer>();
    lfoWaveVisualizer->setAccentColour(HasturColours::kLfo);
    contentContainer.addAndMakeVisible(*lfoWaveVisualizer);

    lfoWaveLabel.setText("WAVE", juce::dontSendNotification);
    lfoWaveLabel.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
    lfoWaveLabel.setJustificationType(juce::Justification::centred);
    lfoWaveLabel.setColour(juce::Label::textColourId, HasturColours::kTextWhite);
    contentContainer.addAndMakeVisible(lfoWaveLabel);

    // Rate Mode Vertical Selector (HZ / BPM)
    lfoRateModeLabel.setText("MODE", juce::dontSendNotification);
    lfoRateModeLabel.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
    lfoRateModeLabel.setJustificationType(juce::Justification::centred);
    lfoRateModeLabel.setColour(juce::Label::textColourId, HasturColours::kTextWhite);
    contentContainer.addAndMakeVisible(lfoRateModeLabel);

    lfoRateModeButtons.setVertical(true);
    lfoRateModeButtons.setAccentColour(HasturColours::kLfo);
    contentContainer.addAndMakeVisible(lfoRateModeButtons);
    lfoRateModeButtons.onSelect = [this](int idx) {
        processor_.setUiLfoRateMode(idx);
        if (idx == 0) {
            lfoRateKnob.slider.textFromValueFunction = [](double val) {
                return juce::String::formatted("%.2f Hz", val);
            };
        } else {
            lfoRateKnob.slider.textFromValueFunction = [](double val) {
                if (val < 0.25) return juce::String("4/1");
                if (val < 0.5) return juce::String("2/1");
                if (val < 1.0) return juce::String("1/1");
                if (val < 2.0) return juce::String("1/2");
                if (val < 4.0) return juce::String("1/4");
                if (val < 8.0) return juce::String("1/8");
                if (val < 16.0) return juce::String("1/16");
                return juce::String("1/32");
            };
        }
        lfoRateKnob.slider.updateText();
    };
    int initialLfoRateMode = processor_.getUiLfoRateMode();
    lfoRateModeButtons.setState(initialLfoRateMode);
    if (lfoRateModeButtons.onSelect) lfoRateModeButtons.onSelect(initialLfoRateMode);

    // Row 2 Selectors
    setupButtonBar(filterLfoDestButtons, filterLfoDestLabel, "filter_lfo_dest", "DEST", HasturColours::kLfo);
    filterLfoDestButtons.setVertical(true);

    setupButtonBar(lfoSyncButtons, lfoSyncLabel, "lfo_sync", "SYNC", HasturColours::kLfo);
    lfoSyncButtons.setVertical(true);

    lfoFamilyGrid.setAccentColour(HasturColours::kLfo);
    lfoFamilyGrid.setGlyphProviders({
        // 1. Saw / Tri
        [](juce::Rectangle<float> r) {
            juce::Path p;
            p.startNewSubPath(r.getX() + 2.0f, r.getBottom() - 2.0f);
            p.lineTo(r.getCentreX(), r.getY() + 2.0f);
            p.lineTo(r.getRight() - 2.0f, r.getBottom() - 2.0f);
            return p;
        },
        // 2. Square / Pulse
        [](juce::Rectangle<float> r) {
            juce::Path p;
            float splitX = r.getX() + r.getWidth() * 0.45f;
            p.startNewSubPath(r.getX() + 2.0f, r.getBottom() - 2.0f);
            p.lineTo(r.getX() + 2.0f, r.getY() + 2.0f);
            p.lineTo(splitX, r.getY() + 2.0f);
            p.lineTo(splitX, r.getBottom() - 2.0f);
            p.lineTo(r.getRight() - 2.0f, r.getBottom() - 2.0f);
            return p;
        },
        // 3. Sine (Infinifold)
        [](juce::Rectangle<float> r) {
            juce::Path p;
            p.startNewSubPath(r.getX() + 2.0f, r.getCentreY());
            p.cubicTo(r.getX() + r.getWidth() * 0.25f, r.getY() + 1.0f,
                      r.getX() + r.getWidth() * 0.25f, r.getY() + 1.0f,
                      r.getCentreX(), r.getCentreY());
            p.cubicTo(r.getX() + r.getWidth() * 0.75f, r.getBottom() - 1.0f,
                      r.getX() + r.getWidth() * 0.75f, r.getBottom() - 1.0f,
                      r.getRight() - 2.0f, r.getCentreY());
            return p;
        },
        // 4. Random S&H
        [](juce::Rectangle<float> r) {
            juce::Path p;
            float w = r.getWidth();
            p.startNewSubPath(r.getX() + 2.0f, r.getBottom() - 3.0f);
            p.lineTo(r.getX() + w * 0.33f, r.getBottom() - 3.0f);
            p.lineTo(r.getX() + w * 0.33f, r.getY() + 3.0f);
            p.lineTo(r.getX() + w * 0.66f, r.getY() + 3.0f);
            p.lineTo(r.getX() + w * 0.66f, r.getCentreY());
            p.lineTo(r.getRight() - 2.0f, r.getCentreY());
            return p;
        }
    });
    contentContainer.addAndMakeVisible(lfoFamilyGrid);

    // =========================================================================
    // BAY 5: VCA & TRANSIENTS (Functional Color: Blistering Red)
    // =========================================================================
    ampEnvVisualizer = std::make_unique<EnvelopeVisualizer>();
    ampEnvVisualizer->setAccentColour(HasturColours::kVca);
    ampEnvVisualizer->setSubStatorParams(0.0015f, 0.35f, 0.0f, 0.50f, 0.10f, 0.0f);
    contentContainer.addAndMakeVisible(*ampEnvVisualizer);

    setupRotaryKnob(ampSnapKnob, "amp_env_snap", "SNAP", HasturColours::kVca);
    ampSnapKnob.slider.textFromValueFunction = [formatTime](double val) {
        return formatTime(val);
    };
    ampSnapKnob.slider.updateText();

    setupRotaryKnob(ampDecayKnob, "amp_env_decay", "PLUNGE", HasturColours::kVca);
    ampDecayKnob.slider.textFromValueFunction = [formatTime](double val) {
        return formatTime(val);
    };
    ampDecayKnob.slider.updateText();

    setupRotaryKnob(ampSustainKnob, "amp_env_sustain", "LURK", HasturColours::kVca);
    ampSustainKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%.0f%%", val * 100.0);
    };
    ampSustainKnob.slider.updateText();

    setupRotaryKnob(ampReleaseKnob, "amp_env_release", "BLEED", HasturColours::kVca);
    ampReleaseKnob.slider.textFromValueFunction = [formatTime](double val) {
        return formatTime(val);
    };
    ampReleaseKnob.slider.updateText();

    ampGateGrid.setAccentColour(HasturColours::kVca);
    contentContainer.addAndMakeVisible(ampGateGrid);

    setupRotaryKnob(ampVelocityKnob, "amp_velocity", "VELOCITY", HasturColours::kVca);
    ampVelocityKnob.slider.setDoubleClickReturnValue(true, 0.0f);
    ampVelocityKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%.0f%%", val * 100.0);
    };
    ampVelocityKnob.slider.updateText();

    setupRotaryKnob(ampPunchKnob, "pitch_punch_depth", "PUNCH", HasturColours::kVca);
    ampPunchKnob.slider.setDoubleClickReturnValue(true, 0.0);
    ampPunchKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%.2f st", val);
    };
    ampPunchKnob.slider.updateText();

    setupRotaryKnob(ampTimeKnob, "pitch_punch_time", "TIME", HasturColours::kVca);
    ampTimeKnob.slider.textFromValueFunction = [formatTime](double val) {
        return formatTime(val);
    };
    ampTimeKnob.slider.updateText();

    // =========================================================================
    // BAY 6: MASTER OUTPUT (Functional Color: Amber Gold)
    // =========================================================================
    setupRotaryKnob(glideTimeKnob, "glide_time", "GLIDE", HasturColours::kOutput);
    glideTimeKnob.slider.textFromValueFunction = [formatTime](double val) {
        return formatTime(val);
    };
    glideTimeKnob.slider.updateText();

    setupButtonBar(glideModeButtons, glideModeLabel, "glide_mode", "MODE", HasturColours::kOutput);

    setupRotaryKnob(pitchBendRangeKnob, "pitch_bend_range", "BEND", HasturColours::kOutput);
    pitchBendRangeKnob.slider.setDoubleClickReturnValue(true, 2.0);
    pitchBendRangeKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%.0f st", val);
    };
    pitchBendRangeKnob.slider.updateText();

    setupRotaryKnob(vibRateKnob, "vibrato_rate", "RATE", HasturColours::kOutput);
    vibRateKnob.slider.setDoubleClickReturnValue(true, 5.5);
    vibRateKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%.1f Hz", val);
    };
    vibRateKnob.slider.updateText();

    setupRotaryKnob(vibDepthKnob, "vibrato_depth", "DEPTH", HasturColours::kOutput);
    vibDepthKnob.slider.setDoubleClickReturnValue(true, 0.0);
    vibDepthKnob.slider.textFromValueFunction = [](double val) {
        if (val <= 0.005) return juce::String("OFF");
        return juce::String::formatted("%.0f%%", val * 100.0);
    };
    vibDepthKnob.slider.updateText();

    setupRotaryKnob(alphaChorusKnob, "alpha_chorus", "CHORUS", HasturColours::kOutput);
    alphaChorusKnob.slider.textFromValueFunction = [](double val) {
        if (val <= 0.01) return juce::String("OFF");
        return juce::String::formatted("%.0f%%", val * 100.0);
    };
    alphaChorusKnob.slider.setDoubleClickReturnValue(true, 0.0f);
    alphaChorusKnob.slider.updateText();

    setupRotaryKnob(oscSlopKnob, "osc_slop", "SLOP", HasturColours::kOutput);
    oscSlopKnob.slider.setDoubleClickReturnValue(true, 0.0f);
    oscSlopKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%.2f", val);
    };
    oscSlopKnob.slider.updateText();

    setupRotaryKnob(postAmpDriveKnob, "post_amp_drive", "DRIVE", HasturColours::kOutput);
    postAmpDriveKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%.0f%%", val * 100.0);
    };
    postAmpDriveKnob.slider.updateText();

    setupRotaryKnob(globalWidthKnob, "global_width", "WIDTH", HasturColours::kOutput);
    globalWidthKnob.slider.setDoubleClickReturnValue(true, 0.0);
    globalWidthKnob.slider.textFromValueFunction = [](double val) {
        if (std::abs(val) < 0.005)
            return juce::String("STEREO (100%)");
        if (val < 0.0)
            return juce::String::formatted("MONO (%.0f%%)", -val * 100.0);
        return juce::String::formatted("CENTER (%.0f%%)", val * 100.0);
    };
    globalWidthKnob.slider.updateText();

    setupRotaryKnob(outputLevelKnob, "output_level", "VOLUME", HasturColours::kOutput);
    outputLevelKnob.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%.0f%%", val * 100.0);
    };
    outputLevelKnob.slider.updateText();

    // Bind all discrete controls with SOTA RAII Attachments
    bindDiscreteControls();

    // Layout all controls into canonical non-overlapping coordinates
    layoutControls();

    startTimerHz(30);
}

HasturAudioProcessorEditor::~HasturAudioProcessorEditor() {
    setConstrainer(nullptr);
    stopTimer();
    contentContainer.removeMouseListener(this);
    setLookAndFeel(nullptr);
}

juce::Colour HasturAudioProcessorEditor::getAccentColour() const noexcept {
    switch (lastLifeMode_) {
        case 0: return HasturColours::kSynthetic; // Synthetic: Tron Blue (0xFF00C8FF)
        case 1: return HasturColours::kCyborg;    // Cyborg: Rusted Construction Equipment Yellow (0xFFE08C00)
        case 2: return HasturColours::kOrganic;   // Organic: Warm Green (0xFF6BBF59)
        default: return HasturColours::kSynthetic;
    }
}

void HasturAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(HasturColours::kChassisBg);
}

void HasturAudioProcessorEditor::paintCanvas(juce::Graphics& g) {
    g.fillAll(HasturColours::kChassisBg);

    // Top Accent Separator line dynamically matches the active Lifeform accent!
    g.setColour(getAccentColour());
    g.fillRect(8, 40, kBaseWidth - 16, 2);

    // Solid Section Header Badges (WCAG AAA/AA)
    auto drawBay = [&](int x, int y, int w, int h, const juce::String& title, juce::Colour badgeCol) {
        auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                             static_cast<float>(w), static_cast<float>(h));
        // Sub-chassis recessed container
        g.setColour(HasturColours::kBayBg);
        g.fillRoundedRectangle(bounds, 6.0f);
        g.setColour(juce::Colour(0xFF30363D));
        g.drawRoundedRectangle(bounds, 6.0f, 1.0f);

        // Solid Badge Pill centered at top of bay (only if title is not empty)
        if (title.isNotEmpty()) {
            int badgeW = 74;
            int badgeH = 20;
            int badgeX = x + (w - badgeW) / 2;
            int badgeY = y + 6;
            auto badgeRect = juce::Rectangle<float>(static_cast<float>(badgeX), static_cast<float>(badgeY),
                                                   static_cast<float>(badgeW), static_cast<float>(badgeH));

            // Pill fill with functional color
            g.setColour(badgeCol);
            g.fillRoundedRectangle(badgeRect, 4.0f);

            // High-contrast Obsidian ink on bright pill (WCAG AAA)
            g.setFont(juce::Font(juce::FontOptions(12.0f).withStyle("Bold")));
            g.setColour(juce::Colour(0xFF0D1117));
            g.drawText(title, badgeRect, juce::Justification::centred);

            // Header divider line (clears header from inner controls)
            g.setColour(juce::Colour(0xFF21262D));
            g.fillRect(x + 8, y + 30, w - 16, 1);
        }
    };

    // Subtle Recessed Pod Drawer Helper
    auto drawSubtlePod = [&](int px, int py, int pw, int ph) {
        auto podRect = juce::Rectangle<float>(static_cast<float>(px), static_cast<float>(py),
                                             static_cast<float>(pw), static_cast<float>(ph));
        g.setColour(juce::Colour(0xFF0B0F14)); // Recessed pod cavity
        g.fillRoundedRectangle(podRect, 4.0f);
        g.setColour(juce::Colour(0xFF38434F)); // Crisp legible pod border (WCAG AA)
        g.drawRoundedRectangle(podRect, 4.0f, 1.0f);
    };

    // =========================================================================
    // BAY 1: DCO FULL-HEIGHT MONOLITH TOWER (x: 14, y: 44, w: 277, h: 816)
    // =========================================================================
    drawBay(14, 44, 277, 816, "DCO", HasturColours::kDco);

    // Wave & Chop Stacked Pod (22, 176, 261, 58)
    drawSubtlePod(22, 176, 261, 58);

    // Monolithic Morph & Shaper Pod (22, 240, 261, 180)
    drawSubtlePod(22, 240, 261, 180);

    // Sub-Stator Card Divider & Badge in Bay 1 (Midpoint y: 452)
    g.setColour(juce::Colour(0xFF21262D));
    g.fillRect(14 + 8, 452, 277 - 16, 1);
    auto subPill = juce::Rectangle<float>(14.0f + (277.0f - 110.0f) * 0.5f, 443.0f, 110.0f, 18.0f);
    g.setColour(juce::Colour(0xFF161B22));
    g.fillRoundedRectangle(subPill, 3.0f);
    g.setColour(juce::Colour(0xFF30363D));
    g.drawRoundedRectangle(subPill, 3.0f, 1.0f);
    g.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
    g.setColour(HasturColours::kSub); // Movie Poster Orange
    g.drawText("SUB-STATOR", subPill, juce::Justification::centred);

    // Chew & Spit Hero Knobs Pod (22, 554, 261, 88)
    drawSubtlePod(22, 554, 261, 88);

    // Sub Octave Pod (22, 644, 261, 28)
    drawSubtlePod(22, 644, 261, 28);
    // Sub Tap Pod (22, 674, 261, 26)
    drawSubtlePod(22, 674, 261, 26);
    // Sub Pulse Train 3x2 Pod (22, 712, 261, 56) - rendered by PulseGridBar itself!

    // =========================================================================
    // BAY 2: STEM MIXER, AUDIO-RATE X-MOD & STEPPED RDM (x: 303, y: 44, w: 277, h: 816)
    // Full-height unified Titan Card mirroring Column 1
    // =========================================================================
    drawBay(303, 44, 277, 816, "", HasturColours::kMixer);

    // Top half: Mixer (y 74..244)
    drawSubtlePod(309, 74, 176, 170); // CORE, SUB, NOISE (Width 176, 8px margin to visible caps)
    drawSubtlePod(491, 74,  83, 170); // SEETHE (Width 83, centered on X = 528, right edge 574)

    // Half divider line
    g.setColour(juce::Colour(0xFF21262D));
    g.fillRect(303 + 8, 249, 277 - 16, 1);

    // Mid section: X-Mod (y 254..444)
    drawSubtlePod(309, 254, 265, 88); // CH1: FM to Filter
    drawSubtlePod(309, 346, 265, 96); // CH2: AM to Amp

    // Midpoint Divider & Badge in Column 2 (y: 452)
    g.setColour(juce::Colour(0xFF21262D));
    g.fillRect(303 + 8, 452, 277 - 16, 1);
    auto rdmPill = juce::Rectangle<float>(303.0f + (277.0f - 110.0f) * 0.5f, 443.0f, 110.0f, 18.0f);
    g.setColour(juce::Colour(0xFF161B22));
    g.fillRoundedRectangle(rdmPill, 3.0f);
    g.setColour(juce::Colour(0xFF30363D));
    g.drawRoundedRectangle(rdmPill, 3.0f, 1.0f);
    g.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
    g.setColour(HasturColours::kLfo); // Caustic Hastur Yellow
    g.drawText("STEPPED RDM", rdmPill, juce::Justification::centred);

    // Lower half: 3 Stepped Random Pod Drawers (Functional Sound Engine Partitioning)
    // Pod 1: DCO Core L-Pod (Row 0: 3 knobs + Row 1 Right: Raw Strangle)
    {
        juce::Path l;
        l.startNewSubPath(309.0f, 504.0f);
        l.lineTo(574.0f, 504.0f); // Top edge
        l.lineTo(574.0f, 700.0f); // Outer right edge
        l.lineTo(485.0f, 700.0f); // Bottom edge of stem
        l.lineTo(485.0f, 600.0f); // Inner left edge of stem
        l.lineTo(309.0f, 600.0f); // Bottom edge of Row 0
        l.closeSubPath();
        auto rounded = l.createPathWithRoundedCorners(4.0f);
        g.setColour(juce::Colour(0xFF0B0F14)); // Recessed pod cavity
        g.fillPath(rounded);
        g.setColour(juce::Colour(0xFF38434F)); // Crisp legible pod border (WCAG AA)
        g.strokePath(rounded, juce::PathStrokeType(1.0f));
    }
    // Pod 2: Sub-Stator Pod (Row 1 Left & Mid: Sub Chew, Sub Spit)
    drawSubtlePod(309, 604, 172, 96);

    // Pod 3: Dual SVF VCF Pod (Row 2: Filter Twist, Stab, Orbit)
    drawSubtlePod(309, 706, 265, 98);

    // =========================================================================
    // BAY 3: DUAL SVF VCF (x: 592, y: 44, w: 554, h: 410)
    // =========================================================================
    drawBay(592, 44, 554, 410, "VCF", HasturColours::kVcf);

    // Pod A: Resonance Core (Cutoff & Resonance, ends X = 790, Y = 178)
    drawSubtlePod(604, 74, 186, 104);

    // Pod B: Acid Envelope Triad (Env Depth, Filter Decay, Accent, ends X = 1136, Y = 178)
    drawSubtlePod(796, 74, 340, 104);

    // Pod C: Track & Orbit (Keytrack & Filter Orbit)
    drawSubtlePod(604, 178, 148, 72);

    // Pod D: Filter Deck (Duo-48, Pre-24, Post-24)
    drawSubtlePod(754, 178, 114, 72);

    // Pod E: Shapers Pod (Twist, Stab, Slash)
    drawSubtlePod(872, 178, 264, 72);

    // Row 3 Lower Modules: Burble Pod (Orange) & LFO Engine Pod (Yellow)
    drawSubtlePod(604, 258, 94, 186);  // Burble Pod
    drawSubtlePod(704, 258, 432, 186); // Filter LFO Engine Pod

    // =========================================================================
    // BAY 5: VCA & TRANSIENTS (x: 592, y: 462, w: 271, h: 398)
    // =========================================================================
    drawBay(592, 462, 271, 398, "VCA", HasturColours::kVca);

    // Pod 1: ADSR Pod (Snap, Plunge, Lurk, Bleed)
    drawSubtlePod(600, 566, 255, 144);

    // Pod 2: Gate Mode Pod (Env, Gate, Drone)
    drawSubtlePod(600, 714, 124, 64);

    // Pod 3: Standalone Velocity Pod (Velocity Sensitivity)
    drawSubtlePod(731, 710, 124, 68);

    // Pod 4: Pitch Dynamics Pod (PUNCH / TIME)
    drawSubtlePod(600, 786, 255, 68);

    // =========================================================================
    // BAY 6: MAIN MASTER CONTROL PANEL (x: 875, y: 462, w: 271, h: 398)
    // =========================================================================
    drawBay(875, 462, 271, 398, "MAIN", HasturColours::kOutput);

    // Tier 1 Dual Pods
    drawSubtlePod(883, 498, 126, 108); // Glide Pod
    drawSubtlePod(1013, 498, 125, 108); // Vibrato Pod

    // Vibrato Pod centered label
    g.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    g.setColour(HasturColours::kTextWhite);
    g.drawText("VIBRATO", 1013, 580, 125, 18, juce::Justification::centred, false);

    // Tier 4 Voice Trigger Pod
    drawSubtlePod(883, 814, 255, 38);  // Voice Trigger Pod
}

void HasturAudioProcessorEditor::bindDiscreteControls() {
    discreteAttachments_.clear();

    // 1. Bay 1: DCO & Sub-Stator
    bindDiscrete(dcoFamilyButtons,     "dco_wave_family");
    bindDiscrete(dcoTopoButtons,       "dco_chopper_topo");
    bindDiscrete(subOctaveButtons,     "sub_octave");
    bindDiscrete(subTrainGrid,         "sub_pulse_train");
    bindDiscrete(subApfTapButtons,     "sub_apf_tap");

    // 2. Bay 2: Audio-Rate X-Mod Sources
    bindDiscrete(filterModSourceGrid,  "mod_a_source");
    bindDiscrete(ampModSourceGrid,     "mod_b_source");

    // 3. Bay 3: Filter & Burble
    auto deckRemap = [](int x) { return (x == 1) ? 0 : (x == 0) ? 1 : 2; };
    bindDiscrete(filterDeckGrid,       "filter_deck", deckRemap, deckRemap);
    bindDiscrete(burbleModeButtons,    "burble_mode");

    // 4. Bay 4: Filter LFO & Modulation
    bindDiscrete(filterLfoDestButtons, "filter_lfo_dest");
    bindDiscrete(lfoSyncButtons,       "lfo_sync");
    bindDiscrete(lfoFamilyGrid,        "lfo_wave");

    // 5. Bay 5: VCA & Voice Trigger
    bindDiscrete(ampGateGrid,          "amp_gate_mode");
    bindDiscrete(trigModeButtons,      "trigger_mode");
    bindDiscrete(glideModeButtons,     "glide_mode");

    // 6. Header: Lifeform Mode
    bindDiscrete(lifeModeToggle,       "life_mode", nullptr, nullptr, [this](int lm) {
        lastLifeMode_ = lm;
        updateColours();
        repaint();
    });
}

void HasturAudioProcessorEditor::layoutControls() {
    // Header (Position A: Title -> Preset Stub -> Unified Inspector -> Lifeform Toggle)
    headerLabel.setBounds(14, 4, 140, 36);
    presetStub.setBounds(162, 8, 168, 28);
    headerInspector.setBounds(338, 8, 566, 28);
    lifeModeToggle.setBounds(912, 8, 234, 28);

    // =========================================================================
    // BAY 1: DCO FULL-HEIGHT MONOLITH TOWER (x: 14, y: 44, w: 277, h: 816)
    // =========================================================================
    // 1. Core DCO Scope (261x92px)
    if (dcoScope != nullptr) {
        dcoScope->setBounds(22, 78, 261, 92);
    }

    // WAVE and CHOP Stacked Vertically in Pod (22, 176, 261, 58)
    // Format 1: Segmented Ribbons (h: 22px)
    dcoFamilyLabel.setBounds(26, 180, 42, 22);
    dcoFamilyButtons.setBounds(72, 180, 207, 22);

    dcoTopoLabel.setBounds(26, 206, 42, 22);
    dcoTopoButtons.setBounds(72, 206, 207, 22);

    // Shaper Row 1: Morph, PWM Amount, Strangle (Centerline Y = 290)
    // Primary (64px): Morph, Strangle | Secondary (48px): PW Amount
    dcoMorphKnob.label.setBounds(26, 244, 70, 12);
    dcoMorphKnob.slider.setBounds(29, 258, 64, 64);

    pwLfoAmountKnob.label.setBounds(99, 244, 56, 12);
    pwLfoAmountKnob.slider.setBounds(103, 266, 48, 48); // Centered Y=290

    osc1MultKnob.label.setBounds(166, 244, 70, 12);
    osc1MultKnob.slider.setBounds(169, 258, 64, 64);

    // Shaper Row 2: PW Rate, Warp, Bite, Melt (Centerline Y = 384)
    // All 4 knobs Secondary (48px)
    pwLfoRateKnob.label.setBounds(26, 346, 52, 12);
    pwLfoRateKnob.slider.setBounds(28, 360, 48, 48);

    osc1ApfKnob.label.setBounds(86, 346, 52, 12);
    osc1ApfKnob.slider.setBounds(88, 360, 48, 48);

    heroBiteKnob.label.setBounds(146, 346, 52, 12);
    heroBiteKnob.slider.setBounds(148, 360, 48, 48);

    osc1DriveKnob.label.setBounds(206, 346, 52, 12);
    osc1DriveKnob.slider.setBounds(208, 360, 48, 48);

    // 2. Sub-Stator Scope (261x80px)
    if (subScope != nullptr) {
        subScope->setBounds(22, 470, 261, 80);
    }

    // Sub Hero Knobs: CHEW & SPIT (Dual Primary 64px, Centerline Y = 606)
    subApfDepthKnob.label.setBounds(45, 560, 84, 12);
    subApfDepthKnob.slider.setBounds(55, 574, 64, 64);

    subSpitKnob.label.setBounds(176, 560, 84, 12);
    subSpitKnob.slider.setBounds(186, 574, 64, 64);

    // Sub Octave & Tap: Format 1 Ribbons (h: 22px)
    subOctaveLabel.setBounds(26, 648, 38, 22);
    subOctaveButtons.setBounds(68, 648, 211, 22);

    subApfTapLabel.setBounds(26, 676, 38, 22);
    subApfTapButtons.setBounds(68, 676, 211, 22);

    // Sub Pulse Train: Format 2 Grid (h: 56px, 2 rows of 28px)
    subTrainGrid.setBounds(22, 712, 261, 56);

    // =========================================================================
    // BAY 2: STEM MIXER, top half (y 74..244)
    // =========================================================================
    const int faderY = 96;
    const int faderW = 40;
    const int faderH = 130;

    // Stem faders aligned so visible caps (34px) hit X-Mod selector grid (X = 317..477, center = 397)
    osc1LevelSlider.label.setBounds(311, 78, 46, 14);
    osc1LevelSlider.slider.setBounds(314, faderY, faderW, faderH);

    subLevelSlider.label.setBounds(374, 78, 46, 14);
    subLevelSlider.slider.setBounds(377, faderY, faderW, faderH);

    noiseLevelSlider.label.setBounds(437, 78, 46, 14);
    noiseLevelSlider.slider.setBounds(440, faderY, faderW, faderH);

    // Seethe fader aligned so visible cap center hits FM/AM depth knobs centerline (X = 528)
    mixerDriveSlider.label.setBounds(491, 78, 74, 14);
    mixerDriveSlider.slider.setBounds(508, faderY, faderW, faderH);

    // =========================================================================
    // BAY 2: AUDIO-RATE X-MOD (Centerlines Y = 306.0 and Y = 392.0)
    // =========================================================================
    // CH1: FM to Filter (Grid 56px + Primary Knob 64px, Centerline Y = 306.0)
    filterModSourceGrid.setBounds(317, 278, 160, 56);
    filterModAmountKnob.label.setBounds(491, 260, 74, 12);
    filterModAmountKnob.slider.setBounds(496, 274, 64, 64);

    // CH2: AM to Amp (Grid 56px + Primary Knob 64px, Centerline Y = 392.0)
    ampModSourceGrid.setBounds(317, 364, 160, 56);
    ampModAmountKnob.label.setBounds(491, 346, 74, 12);
    ampModAmountKnob.slider.setBounds(496, 360, 64, 64);

    // =========================================================================
    // BAY 4: STEPPED RANDOM MODULATION MATRIX (9 PRIMARY 64px KNOBS)
    // Locked to Datums 4, 5, 6 (Centerlines Y = 548.0, 660.0, 764.0)
    // =========================================================================
    const int resColX[3] = {319, 403, 487};
    const int resRowY[3] = {508, 612, 716};

    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            int idx = row * 3 + col;
            int cx = resColX[col];
            int cy = resRowY[row];
            stepRandKnobs[idx].label.setBounds(cx, cy, 76, 14);
            stepRandKnobs[idx].slider.setBounds(cx + 6, cy + 16, 64, 64);
        }
    }

    // =========================================================================
    // BAY 3: DUAL SVF VCF (x: 592, y: 44, w: 554, h: 410)
    // =========================================================================
    // --- ROW 1: THE MASTER VCF FILTER PENTAD (Centerline Y = 136.0) ---
    // ONE BIG KNOB (80px): Cutoff
    cutoffKnob.label.setBounds(618, 78, 80, 14);
    cutoffKnob.slider.setBounds(618, 96, 80, 80);

    // Primary Knobs (64px): Resonance, Env Depth, Decay, Accent
    resKnob.label.setBounds(710, 86, 68, 14);
    resKnob.slider.setBounds(712, 104, 64, 64);

    envDepthKnob.label.setBounds(831, 86, 68, 14);
    envDepthKnob.slider.setBounds(833, 104, 64, 64);

    filterDecayKnob.label.setBounds(932, 86, 68, 14);
    filterDecayKnob.slider.setBounds(934, 104, 64, 64);

    accentKnob.label.setBounds(1033, 86, 68, 14);
    accentKnob.slider.setBounds(1035, 104, 64, 64);

    // --- ROW 2: SECONDARY CONTROLS & DECK (Centerline Y = 222.0) ---
    // Pod C: Track & Orbit (Secondary 48px)
    keyTrackKnob.label.setBounds(610, 184, 56, 12);
    keyTrackKnob.slider.setBounds(614, 198, 48, 48);

    filterOrbitKnob.label.setBounds(678, 184, 56, 12);
    filterOrbitKnob.slider.setBounds(682, 198, 48, 48);

    // Pod D: Filter Deck Grid (Format 2: 56px height, 2 rows of 28px)
    filterDeckGrid.setBounds(756, 194, 106, 56);

    // Pod E: Shapers Pod (Secondary 48px: Twist, Stab, Slash)
    filterWarpKnob.label.setBounds(884, 184, 60, 12);
    filterWarpKnob.slider.setBounds(890, 198, 48, 48);

    laserBiteKnob.label.setBounds(968, 184, 60, 12);
    laserBiteKnob.slider.setBounds(974, 198, 48, 48);

    laserDriveKnob.label.setBounds(1052, 184, 60, 12);
    laserDriveKnob.slider.setBounds(1058, 198, 48, 48);

    // --- ROW 3: LOWER MODULES (y: 258..444) ---
    // Left: Burble Module (48px Knob + Format 2 56px Vertical Bar)
    burbleDepthKnob.label.setBounds(610, 266, 82, 12);
    burbleDepthKnob.slider.setBounds(627, 282, 48, 48);

    burbleModeLabel.setBounds(610, 348, 82, 12);
    burbleModeButtons.setBounds(615, 364, 72, 56);

    // Right: Filter LFO Engine
    // Row 3A: Amount, Shape, Visualizer, Rate (Secondary 48px, Centerline Y = 306.0)
    lfoAmountKnob.label.setBounds(712, 266, 56, 12);
    lfoAmountKnob.slider.setBounds(716, 282, 48, 48);

    lfoShapeKnob.label.setBounds(784, 266, 56, 12);
    lfoShapeKnob.slider.setBounds(788, 282, 48, 48);

    lfoWaveLabel.setBounds(856, 266, 58, 12);
    if (lfoWaveVisualizer != nullptr) {
        lfoWaveVisualizer->setBounds(856, 282, 58, 48);
    }

    lfoRateKnob.label.setBounds(934, 266, 56, 12);
    lfoRateKnob.slider.setBounds(938, 282, 48, 48);

    lfoRateModeLabel.setBounds(1014, 266, 72, 12);
    lfoRateModeButtons.setBounds(1014, 282, 72, 56);

    // Row 3B: Destination, Wave Grid, Sync (All Format 2: 56px height, Centerline Y = 392.0)
    filterLfoDestButtons.setBounds(716, 364, 82, 56);
    lfoFamilyGrid.setBounds(822, 364, 184, 56);
    lfoSyncButtons.setBounds(1028, 364, 72, 56);

    // =========================================================================
    // BAY 5: VCA & TRANSIENTS (x: 592, y: 462, w: 271, h: 398)
    // =========================================================================
    // Row 1: Full-Width OLED Amp Envelope Visualizer (600, 494, 255, 64)
    if (ampEnvVisualizer != nullptr) {
        ampEnvVisualizer->setBounds(600, 494, 255, 64);
    }

    // Pod 1: ADSR Pod (All 4 Secondary 48px)
    // Row 2A (Attack & Decay: SNAP & PLUNGE, Centerline Y = 610.0)
    ampSnapKnob.label.setBounds(610, 572, 100, 12);
    ampSnapKnob.slider.setBounds(636, 586, 48, 48);

    ampDecayKnob.label.setBounds(744, 572, 100, 12);
    ampDecayKnob.slider.setBounds(770, 586, 48, 48);

    // Row 2B (Sustain & Release: LURK & BLEED, Centerline Y = 680.0)
    ampSustainKnob.label.setBounds(610, 642, 100, 12);
    ampSustainKnob.slider.setBounds(636, 656, 48, 48);

    ampReleaseKnob.label.setBounds(744, 642, 100, 12);
    ampReleaseKnob.slider.setBounds(770, 656, 48, 48);

    // Pod 2 & 3: Gate Mode (Grid 56px) & Velocity Sensitivity (Secondary 48px)
    ampGateGrid.setBounds(605, 718, 114, 56);

    ampVelocityKnob.label.setBounds(744, 712, 100, 12);
    ampVelocityKnob.slider.setBounds(770, 726, 48, 48);

    // Pod 4: Pitch Dynamics (Both Secondary 48px, Centerline Y = 828.0)
    ampPunchKnob.label.setBounds(610, 790, 100, 12);
    ampPunchKnob.slider.setBounds(636, 804, 48, 48);

    ampTimeKnob.label.setBounds(744, 790, 100, 12);
    ampTimeKnob.slider.setBounds(770, 804, 48, 48);

    // =========================================================================
    // BAY 6: MAIN MASTER CONTROL PANEL (x: 875, y: 462, w: 271, h: 398)
    // =========================================================================
    // Tier 1: Dual Pods (Glide & Vibrato, Centerline Y = 542.0)
    // Glide Pod: Secondary Knob 48px + Format 1 Ribbon 22px
    glideTimeKnob.label.setBounds(889, 504, 114, 12);
    glideTimeKnob.slider.setBounds(922, 518, 48, 48);
    glideModeButtons.setBounds(893, 574, 106, 22);

    // Vibrato Pod: Dual Secondary Knobs 48px
    vibRateKnob.label.setBounds(1015, 504, 58, 12);
    vibRateKnob.slider.setBounds(1020, 518, 48, 48);

    vibDepthKnob.label.setBounds(1074, 504, 58, 12);
    vibDepthKnob.slider.setBounds(1078, 518, 48, 48);

    // Tier 2: Row B Pitch & Saturation (Primary 64px, Centerline Y = 660.0)
    // Collinear with Bay 4 Row 1 (Stepped Random Row 1)
    pitchBendRangeKnob.label.setBounds(883, 612, 85, 14);
    pitchBendRangeKnob.slider.setBounds(893, 628, 64, 64);

    oscSlopKnob.label.setBounds(968, 612, 85, 14);
    oscSlopKnob.slider.setBounds(978, 628, 64, 64);

    postAmpDriveKnob.label.setBounds(1053, 612, 85, 14);
    postAmpDriveKnob.slider.setBounds(1063, 628, 64, 64);

    // Tier 3: Row C Stereo & Space (Primary 64px, Centerline Y = 764.0)
    // Collinear with Bay 4 Row 2 (Stepped Random Row 2)
    alphaChorusKnob.label.setBounds(883, 716, 85, 14);
    alphaChorusKnob.slider.setBounds(893, 732, 64, 64);

    globalWidthKnob.label.setBounds(968, 716, 85, 14);
    globalWidthKnob.slider.setBounds(978, 732, 64, 64);

    outputLevelKnob.label.setBounds(1053, 716, 85, 14);
    outputLevelKnob.slider.setBounds(1063, 732, 64, 64);

    // Tier 4: Voice Trigger Mode (Format 1 Ribbon 22px)
    trigModeLabel.setBounds(895, 822, 50, 22);
    trigModeButtons.setBounds(955, 822, 175, 22);
}

void HasturAudioProcessorEditor::visibilityChanged() {
    if (isShowing()) requestRefit();
}

void HasturAudioProcessorEditor::resized() {
    const int w = getWidth();
    const int h = getHeight();
    if (w <= 0 || h <= 0) return;

    const float s = juce::jmin(static_cast<float>(w) / static_cast<float>(kBaseWidth),
                               static_cast<float>(h) / static_cast<float>(kBaseHeight));
    const int ox = (w - juce::roundToInt(kBaseWidth  * s)) / 2;
    const int oy = (h - juce::roundToInt(kBaseHeight * s)) / 2;

    contentContainer.setBounds(0, 0, kBaseWidth, kBaseHeight);
    contentContainer.setTransform(juce::AffineTransform::scale(s).translated(static_cast<float>(ox), static_cast<float>(oy)));

    if (std::abs(s - effScale_) > 1.0e-4f) {
        effScale_ = s;
        const int sens = juce::roundToInt(250.0f / s);
        for (auto* sl : rotarySliders_) {
            if (sl != nullptr) sl->setMouseDragSensitivity(sens);
        }
    }

    if (getPeer() != nullptr && juce::Point<int>{ w, h } != expectedSize_) {
        commitUserScale(s);
    }
}

void HasturAudioProcessorEditor::applyEffectiveScale(float s) {
    s = juce::jlimit(rlyeh::ui::EditorScaleManager::kMinScale, rlyeh::ui::EditorScaleManager::kMaxScale, s);
    expectedSize_ = { juce::roundToInt(kBaseWidth * s), juce::roundToInt(kBaseHeight * s) };
    setSize(expectedSize_.x, expectedSize_.y);
}

void HasturAudioProcessorEditor::commitUserScale(float s) {
    prefScale_ = std::round(juce::jlimit(rlyeh::ui::EditorScaleManager::kMinScale,
                                         rlyeh::ui::EditorScaleManager::kMaxScale, s) * 100.0f) / 100.0f;
    expectedSize_ = { getWidth(), getHeight() };
    scaleWriter_.note("hastur", prefScale_);
    processor_.setStoredEditorScale(prefScale_);
}

float HasturAudioProcessorEditor::fitScale() {
    return rlyeh::ui::EditorScaleManager::getDisplayFitScale(*this, kBaseWidth, kBaseHeight);
}

void HasturAudioProcessorEditor::requestRefit() {
    if (refitPending_) return;
    refitPending_ = true;
    juce::Component::SafePointer<HasturAudioProcessorEditor> safe(this);
    juce::MessageManager::callAsync([safe] {
        if (safe == nullptr) return;
        safe->refitPending_ = false;

        if (juce::ModifierKeys::currentModifiers.isAnyMouseButtonDown()) {
            juce::Timer::callAfterDelay(150, [safe] { if (safe) safe->requestRefit(); });
            return;
        }

        if (safe->inRefit_ || !safe->isShowing()) return;
        const juce::ScopedValueSetter<bool> guard(safe->inRefit_, true);
        const float target = juce::jmin(safe->prefScale_, safe->fitScale());
        if (std::abs(target - safe->effScale_) > 0.01f) {
            safe->applyEffectiveScale(target);
        }
    });
}

void HasturAudioProcessorEditor::timerCallback() {
    if (activeBadgeSlider_ != nullptr) {
        if (juce::Time::getMillisecondCounter() >= activeBadgeExpiry_) {
            activeBadgeSlider_->getProperties().remove("badgeExpiry");
            activeBadgeSlider_->repaint();
            activeBadgeSlider_ = nullptr;
        }
    }
    if (lockedComponent_ != nullptr) {
        if (!juce::ModifierKeys::currentModifiers.isAnyMouseButtonDown()) {
            lockedComponent_ = nullptr;
        } else {
            updateInspectorForComponent(lockedComponent_.getComponent(), {});
        }
    }
    // Note: Discrete controls now handled event-driven by DiscreteParamAttachment
    if (lfoWaveVisualizer != nullptr) {
        int wIdx = 0;
        float shp = 0.5f;
        if (auto* p = processor_.getAPVTS().getRawParameterValue("lfo_wave"))
            wIdx = juce::roundToInt(p->load(std::memory_order_relaxed));
        if (auto* p = processor_.getAPVTS().getRawParameterValue("lfo_shape"))
            shp = p->load(std::memory_order_relaxed);
        lfoWaveVisualizer->setParams(wIdx, shp);
        if (wIdx != lastLfoWaveIdx_) {
            lastLfoWaveIdx_ = wIdx;
            lfoShapeKnob.slider.updateText();
        }
    }

    if (ampEnvVisualizer != nullptr) {
        float snapSec  = 0.01f;
        float decaySec = 0.40f;
        float susLevel = 0.30f;
        float relSec   = 0.50f;
        if (auto* p = processor_.getAPVTS().getRawParameterValue("amp_env_snap"))
            snapSec = p->load(std::memory_order_relaxed);
        if (auto* p = processor_.getAPVTS().getRawParameterValue("amp_env_decay"))
            decaySec = p->load(std::memory_order_relaxed);
        if (auto* p = processor_.getAPVTS().getRawParameterValue("amp_env_sustain"))
            susLevel = p->load(std::memory_order_relaxed);
        if (auto* p = processor_.getAPVTS().getRawParameterValue("amp_env_release"))
            relSec = p->load(std::memory_order_relaxed);
        ampEnvVisualizer->setSubStatorParams(snapSec, decaySec, 0.0f, susLevel, relSec, 0.0f);
    }

    // Note: life_mode is event-driven by DiscreteParamAttachment

    // ---- Modulation Telemetry Radar (Message Thread; Relaxed Atomics Only) ----
    auto& apvts = processor_.getAPVTS();
    auto getParam = [&apvts](const char* id, float def = 0.0f) noexcept {
        auto* p = apvts.getRawParameterValue(id);
        return p ? p->load(std::memory_order_relaxed) : def;
    };
    auto putProp = [](SliderItem& it, const char* key, double v) {
        if (it.slider.getProperties().set(key, v)) {
            it.slider.repaint();
            return true;
        }
        return false;
    };

    // 1. Stepped Random Targets: knob order == voice step index
    static constexpr const char* kRandIds[9] = {
        "rand_core_warp", "rand_core_bite", "rand_core_melt",
        "rand_sub_chew", "rand_sub_spit", "rand_raw_strangle",
        "rand_filter_twist", "rand_filter_stab", "rand_filter_orbit"
    };
    SliderItem* const randTargets[9] = {
        &osc1ApfKnob, &heroBiteKnob, &osc1DriveKnob,
        &subApfDepthKnob, &subSpitKnob, &osc1MultKnob,
        &filterWarpKnob, &laserBiteKnob, &filterOrbitKnob
    };
    for (int i = 0; i < 9; ++i) {
        putProp(*randTargets[i], "stepRandAmount", getParam(kRandIds[i]));
        putProp(*randTargets[i], "activeStepVal",  processor_.getStepRandAtomic(i));
    }

    // 2. Morph / Orbit LFO Arcs & Live Moving Pips
    const int lfoDest = juce::roundToInt(getParam("filter_lfo_dest"));
    const float lfoDepth = getParam("filter_lfo_depth");
    const float pwLfoAmt = getParam("pw_lfo_amount");
    putProp(dcoMorphKnob, "morphLfoAmount", pwLfoAmt);
    if (pwLfoAmt > 0.001f) {
        const float morphVal = getParam("dco_morph");
        const float pwLfoVal = processor_.getPwLfoAtomic();
        const float w = pwLfoAmt;
        const float a = std::clamp(morphVal - 0.5f * w, 0.0f, 1.0f - w);
        const float b = a + w;
        const float effMorph = CrackedChimes::DSP::HasturVoice::slidingWindowShift(
            std::clamp(morphVal, 0.0f, 1.0f), 0.0f, 1.0f, pwLfoAmt, pwLfoVal);
        putProp(dcoMorphKnob, "modLo", a);
        putProp(dcoMorphKnob, "modHi", b);
        putProp(dcoMorphKnob, "modPip", effMorph);
    } else {
        dcoMorphKnob.slider.getProperties().remove("modLo");
        dcoMorphKnob.slider.getProperties().remove("modHi");
        dcoMorphKnob.slider.getProperties().remove("modPip");
    }

    const float orbitLfoAmt = (lfoDest == 1) ? lfoDepth : 0.0f;
    putProp(filterOrbitKnob, "orbitLfoAmount", orbitLfoAmt);
    if (orbitLfoAmt > 0.001f) {
        const float orbitVal = getParam("filter_orbit");
        const float lfoVal = processor_.getGlobalLfoAtomic();
        const float effOrbit = CrackedChimes::DSP::HasturVoice::boundedShift(
            std::clamp((orbitVal + 1.0f) * 0.5f, 0.0f, 1.0f), 0.0f, 1.0f, orbitLfoAmt, lfoVal);
        putProp(filterOrbitKnob, "modLo", std::clamp(((orbitVal + 1.0f) * 0.5f) * (1.0f - orbitLfoAmt), 0.0f, 1.0f));
        putProp(filterOrbitKnob, "modHi", std::clamp(((orbitVal + 1.0f) * 0.5f) + orbitLfoAmt * (1.0f - (orbitVal + 1.0f) * 0.5f), 0.0f, 1.0f));
        putProp(filterOrbitKnob, "modPip", effOrbit);
    } else {
        filterOrbitKnob.slider.getProperties().remove("modLo");
        filterOrbitKnob.slider.getProperties().remove("modHi");
        filterOrbitKnob.slider.getProperties().remove("modPip");
    }

    // 3. AM X-Mod on Master Output Level (Audio-rate -> Range Arc Only, No Pip)
    putProp(outputLevelKnob, "amXModAmount", getParam("mod_b_amount"));
    putProp(outputLevelKnob, "amXModSource", juce::roundToInt(getParam("mod_b_source", 2.0f)));

    // 4. Cutoff Concentric Rings & Live Sample Pips (Computed in Slider's Proportional Space)
    auto& cs = cutoffKnob.slider;
    const double hz = juce::jlimit(cs.getMinimum(), cs.getMaximum(), static_cast<double>(getParam("cutoff", 1200.0f)));
    auto prop = [&cs](double f) {
        return cs.valueToProportionOfLength(juce::jlimit(cs.getMinimum(), cs.getMaximum(), f));
    };
    const double k = prop(hz);
    bool cutoffDirty = false;

    // Ring 0: Env
    const float effEnvDepth = std::pow(getParam("env_depth"), 1.25f);
    const double envDepthNorm = juce::jmax(0.0, prop(hz * std::exp2(8.0 * effEnvDepth)) - k);
    cutoffDirty |= putProp(cutoffKnob, "cutoffEnvDepth", envDepthNorm);
    if (envDepthNorm > 0.001) {
        const float envVal = processor_.getFilterEnvAtomic();
        const double envCurHz = hz * std::exp2(8.0 * effEnvDepth * envVal);
        cutoffDirty |= putProp(cutoffKnob, "cutoffEnvPip", prop(envCurHz));
    } else {
        cutoffKnob.slider.getProperties().remove("cutoffEnvPip");
    }

    // Ring 1: LFO
    cutoffDirty |= putProp(cutoffKnob, "cutoffLfoDest", lfoDest);
    const float lfoCutoffDepth = (lfoDest == 0) ? lfoDepth : 0.0f;
    cutoffDirty |= putProp(cutoffKnob, "cutoffLfoDepth", lfoCutoffDepth);
    if (lfoCutoffDepth > 0.001f) {
        const float lfoVal = processor_.getGlobalLfoAtomic();
        const double loNorm = std::clamp(k * (1.0f - lfoCutoffDepth), 0.0, 1.0);
        const double hiNorm = std::clamp(k + lfoCutoffDepth * (1.0f - k), 0.0, 1.0);
        const double lfoCurProp = std::clamp(loNorm + (hiNorm - loNorm) * (lfoVal * 0.5f + 0.5f), 0.0, 1.0);
        cutoffDirty |= putProp(cutoffKnob, "cutoffLfoPip", lfoCurProp);
    } else {
        cutoffKnob.slider.getProperties().remove("cutoffLfoPip");
    }

    // Ring 2: Burble
    const float bDepth = getParam("burble_depth");
    const float effB = std::copysign(std::pow(std::abs(bDepth), 2.2f), bDepth);
    cutoffDirty |= putProp(cutoffKnob, "cutoffBurbleDepth", effB);
    if (std::abs(effB) > 0.001f) {
        const float burbleVal = processor_.getBurbleAtomic();
        const double tgtNorm = (effB >= 0.0f)
            ? std::clamp(k + effB * (1.0 - k), 0.0, 1.0)
            : std::clamp(k + effB * k, 0.0, 1.0);
        const double burbleCurProp = std::clamp(k + (tgtNorm - k) * (burbleVal * 0.5f + 0.5f), 0.0, 1.0);
        cutoffDirty |= putProp(cutoffKnob, "cutoffBurblePip", burbleCurProp);
    } else {
        cutoffKnob.slider.getProperties().remove("cutoffBurblePip");
    }

    // Ring 3: FM X-Mod (Audio-rate -> Range Arc Only, No Pip)
    cutoffDirty |= putProp(cutoffKnob, "cutoffFmDepth",  getParam("mod_a_amount"));
    cutoffDirty |= putProp(cutoffKnob, "cutoffFmSource", juce::roundToInt(getParam("mod_a_source")));
    if (cutoffDirty) {
        contentContainer.repaint(cs.getBounds().expanded(14));
    }
}

void HasturAudioProcessorEditor::updateColours() {
    headerLabel.setColour(juce::Label::textColourId, getAccentColour());
    presetStub.setAccentColour(getAccentColour());
    contentContainer.repaint();
}

void HasturAudioProcessorEditor::setupRotaryKnob(SliderItem& item, const juce::String& paramId, const juce::String& labelText, juce::Colour col) {
    item.label.setText(labelText, juce::dontSendNotification);
    item.label.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
    item.label.setJustificationType(juce::Justification::centred);
    item.label.setColour(juce::Label::textColourId, HasturColours::kTextWhite);
    contentContainer.addAndMakeVisible(item.label);

    item.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    item.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    item.slider.setColour(juce::Slider::rotarySliderFillColourId, col);
    item.slider.setColour(juce::Slider::thumbColourId, col);
    contentContainer.addAndMakeVisible(item.slider);
    rotarySliders_.push_back(&item.slider);

    if (auto* param = processor_.getAPVTS().getParameter(paramId)) {
        item.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor_.getAPVTS(), paramId, item.slider);
    }

    bindInspector({&item.slider, &item.label}, paramId, col, &item.slider);
}

void HasturAudioProcessorEditor::setupMixerSlider(SliderItem& item, const juce::String& paramId, const juce::String& labelText, juce::Colour col) {
    item.label.setText(labelText, juce::dontSendNotification);
    item.label.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
    item.label.setJustificationType(juce::Justification::centred);
    item.label.setColour(juce::Label::textColourId, HasturColours::kTextWhite);
    item.label.setBorderSize(juce::BorderSize<int>(1, 0, 1, 0));
    contentContainer.addAndMakeVisible(item.label);

    item.slider.setSliderStyle(juce::Slider::LinearVertical);
    item.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    item.slider.setColour(juce::Slider::trackColourId, juce::Colour(0xFF21262D));
    item.slider.setColour(juce::Slider::thumbColourId, col);
    contentContainer.addAndMakeVisible(item.slider);

    item.slider.textFromValueFunction = [](double val) {
        return juce::String::formatted("%.0f%%", val * 100.0);
    };

    if (auto* param = processor_.getAPVTS().getParameter(paramId)) {
        item.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor_.getAPVTS(), paramId, item.slider);
    }

    bindInspector({&item.slider, &item.label}, paramId, col, &item.slider);
}

void HasturAudioProcessorEditor::setupButtonBar(SegmentedButtonBar& bar, const juce::String& paramId, juce::Colour col) {
    bar.setAccentColour(col);
    contentContainer.addAndMakeVisible(bar);
    bindInspector({&bar}, paramId, col);
}

void HasturAudioProcessorEditor::setupButtonBar(SegmentedButtonBar& bar, juce::Label& label, const juce::String& paramId, const juce::String& labelText, juce::Colour col) {
    label.setText(labelText, juce::dontSendNotification);
    label.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
    label.setJustificationType(juce::Justification::left);
    label.setColour(juce::Label::textColourId, HasturColours::kTextWhite);
    contentContainer.addAndMakeVisible(label);

    bar.setAccentColour(col);
    contentContainer.addAndMakeVisible(bar);
    bindInspector({&bar, &label}, paramId, col);
}



void HasturAudioProcessorEditor::setupWaveformPicker(WaveformPickerButton& button, const juce::String& paramId, bool isLfo) {
    button.isLfo = isLfo;
    button.setAccentColour(HasturColours::kLfo);
    contentContainer.addAndMakeVisible(button);

    button.onClick = [this, &button, paramId, isLfo] {
        openWaveformModal(button, paramId, isLfo);
    };
}

void HasturAudioProcessorEditor::openWaveformModal(WaveformPickerButton& button, const juce::String& paramId, bool isLfo) {
    if (activeWaveModal_ != nullptr) {
        contentContainer.removeChildComponent(activeWaveModal_.get());
        activeWaveModal_.reset();
    }

    int curSel = 0;
    if (auto* p = processor_.getAPVTS().getRawParameterValue(paramId)) {
        curSel = juce::roundToInt(p->load(std::memory_order_relaxed));
    }

    activeWaveModal_ = std::make_unique<WaveformGridModal>(
        isLfo, curSel, HasturColours::kLfo,
        [this, paramId, &button](int waveIdx) {
            if (auto* param = processor_.getAPVTS().getParameter(paramId)) {
                param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(waveIdx)));
            }
            button.setWaveIndex(waveIdx);
        }
    );
    activeWaveModal_->onDismiss = [this] {
        juce::MessageManager::callAsync([safeThis = juce::Component::SafePointer<HasturAudioProcessorEditor>(this)] {
            if (safeThis != nullptr && safeThis->activeWaveModal_ != nullptr) {
                safeThis->contentContainer.removeChildComponent(safeThis->activeWaveModal_.get());
                safeThis->activeWaveModal_.reset();
            }
        });
    };

    activeWaveModal_->setBounds(contentContainer.getLocalBounds());
    contentContainer.addAndMakeVisible(activeWaveModal_.get());
}

// ==============================================================================
// Universal Header Telemetry Inspector & Deep Mouse Event Implementations
// ==============================================================================

void HasturAudioProcessorEditor::bindInspector(std::initializer_list<juce::Component*> comps,
                                               const juce::String& paramId,
                                               juce::Colour accent,
                                               juce::Slider* s) {
    for (auto* c : comps) {
        if (c != nullptr) {
            inspectorBindings_[c] = InspectorBinding{paramId, accent, s};
        }
    }
}

juce::Component* HasturAudioProcessorEditor::resolveInspectorComponent(juce::Component* comp) {
    while (comp != nullptr && comp != &contentContainer) {
        if (inspectorBindings_.find(comp) != inspectorBindings_.end())
            return comp;
        comp = comp->getParentComponent();
    }
    return nullptr;
}

void HasturAudioProcessorEditor::updateInspectorForComponent(juce::Component* comp, juce::Point<int> localPos) {
    juce::ignoreUnused(localPos);
    if (comp == nullptr) return;
    auto it = inspectorBindings_.find(comp);
    if (it == inspectorBindings_.end()) return;

    const auto& binding = it->second;
    const auto* info = HasturInfo::find(binding.paramId.toStdString());

    HeaderInspector::Readout r;
    r.accent = binding.accent;

    if (info != nullptr) {
        r.name = juce::String(info->name.data(), info->name.size());
        r.blurb = juce::String(info->blurb.data(), info->blurb.size());

        // Dynamic context-sensitive blurb for Shape
        if (binding.paramId == hasturParameters::Ids::lfo_shape) {
            int waveIdx = 0;
            if (auto* p = processor_.getAPVTS().getRawParameterValue("lfo_wave"))
                waveIdx = static_cast<int>(p->load());
            auto dynamicBlurb = HasturInfo::getDynamicShapeBlurb(waveIdx);
            r.blurb = juce::String(dynamicBlurb.data(), dynamicBlurb.size());
        }

        // Live value formatting
        if (binding.slider != nullptr) {
            if (binding.slider->textFromValueFunction)
                r.value = binding.slider->textFromValueFunction(binding.slider->getValue());
            else
                r.value = binding.slider->getTextFromValue(binding.slider->getValue());
        } else if (auto* param = processor_.getAPVTS().getParameter(binding.paramId)) {
            if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param)) {
                r.value = choiceParam->getCurrentChoiceName();
                int idx = choiceParam->getIndex();
                if (!info->options.empty() && idx >= 0 && idx < static_cast<int>(info->options.size())) {
                    r.blurb = juce::String(info->options[idx].blurb.data(), info->options[idx].blurb.size());
                }
            } else {
                r.value = param->getCurrentValueAsText();
            }
        }
    }

    if (!r.name.isEmpty()) {
        headerInspector.setReadout(std::move(r));
    }
}

void HasturAudioProcessorEditor::mouseEnter(const juce::MouseEvent& e) {
    if (lockedComponent_ == nullptr) {
        if (auto* c = resolveInspectorComponent(e.eventComponent)) {
            updateInspectorForComponent(c, e.getPosition());
        }
    }
}

void HasturAudioProcessorEditor::mouseMove(const juce::MouseEvent& e) {
    if (lockedComponent_ == nullptr) {
        if (auto* c = resolveInspectorComponent(e.eventComponent)) {
            updateInspectorForComponent(c, e.getPosition());
        }
    }
}

void HasturAudioProcessorEditor::mouseDown(const juce::MouseEvent& e) {
    if (auto* c = resolveInspectorComponent(e.eventComponent)) {
        lockedComponent_ = c;
        if (auto* s = dynamic_cast<juce::Slider*>(c)) {
            activeBadgeSlider_ = s;
            activeBadgeExpiry_ = juce::Time::getMillisecondCounter() + 350;
            s->getProperties().set("badgeExpiry", static_cast<double>(activeBadgeExpiry_));
            s->repaint();
        }
        updateInspectorForComponent(c, e.getPosition());
    }
}

void HasturAudioProcessorEditor::mouseDrag(const juce::MouseEvent& e) {
    if (lockedComponent_ != nullptr) {
        if (auto* s = dynamic_cast<juce::Slider*>(lockedComponent_.getComponent())) {
            activeBadgeSlider_ = s;
            activeBadgeExpiry_ = juce::Time::getMillisecondCounter() + 350;
            s->getProperties().set("badgeExpiry", static_cast<double>(activeBadgeExpiry_));
            s->repaint();
        }
        updateInspectorForComponent(lockedComponent_.getComponent(), e.getPosition());
    }
}

void HasturAudioProcessorEditor::mouseUp(const juce::MouseEvent& e) {
    if (lockedComponent_ != nullptr) {
        if (auto* s = dynamic_cast<juce::Slider*>(lockedComponent_.getComponent())) {
            activeBadgeSlider_ = s;
            activeBadgeExpiry_ = juce::Time::getMillisecondCounter() + 350;
            s->getProperties().set("badgeExpiry", static_cast<double>(activeBadgeExpiry_));
            s->repaint();
        }
    }
    lockedComponent_ = nullptr;
    auto eventInContainer = e.getEventRelativeTo(&contentContainer);
    if (auto* c = resolveInspectorComponent(contentContainer.getComponentAt(eventInContainer.getPosition()))) {
        updateInspectorForComponent(c, eventInContainer.getPosition());
    }
}

void HasturAudioProcessorEditor::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) {
    juce::ignoreUnused(wheel);
    if (auto* c = resolveInspectorComponent(e.eventComponent)) {
        if (auto* s = dynamic_cast<juce::Slider*>(c)) {
            activeBadgeSlider_ = s;
            activeBadgeExpiry_ = juce::Time::getMillisecondCounter() + 350;
            s->getProperties().set("badgeExpiry", static_cast<double>(activeBadgeExpiry_));
            s->repaint();
        }
        updateInspectorForComponent(c, e.getPosition());
    }
}

void HasturAudioProcessorEditor::mouseExit(const juce::MouseEvent& e) {
    juce::ignoreUnused(e);
}
