#pragma once
// CrackedChimesUI - fleet colour + typography tokens (header-only).
// PLAN.md sections 3 / 5. Pure ASCII source, compiles under /utf-8.

#include <juce_gui_basics/juce_gui_basics.h>

namespace CrackedChimes::UI
{
namespace Colours
{
    // ---- Chassis -------------------------------------------------------
    inline const juce::Colour kChassisBg     { 0xFF0D1117 }; // editor background
    inline const juce::Colour kChassisDisc   { 0xFF161B22 }; // knob disc / card
    inline const juce::Colour kChassisRim    { 0xFF30363D }; // disc rim, borders
    inline const juce::Colour kTrack         { 0xFF21262D }; // inactive value track (Hastur parity)
    inline const juce::Colour kBadgeBg       { 0xFF090D12 }; // value badge fill
    inline const juce::Colour kBadgeBorder   { 0xFF262D38 }; // value badge border
    inline const juce::Colour kCavity        { 0xFF0F131A }; // recessed grooves / combo wells
    inline const juce::Colour kNeedle        { 0xFFF0F6FC }; // needle + live pip core
    inline const juce::Colour kText          { 0xFFC9D1D9 }; // labels / badge text
    inline const juce::Colour kTextMuted     { 0xFF8B949E }; // muted labels / secondary text

    // ---- Semantic parts ------------------------------------------------
    inline const juce::Colour kOscCore       { 0xFF00C8FF };
    inline const juce::Colour kOscAlt        { 0xFFFF7A1A };
    inline const juce::Colour kSubCore       { 0xFFFF7A1A };
    inline const juce::Colour kModLfo        { 0xFFF2E10F };
    inline const juce::Colour kLfoCore       { 0xFFF2E10F };
    inline const juce::Colour kBurble        { 0xFFF2E10F };
    inline const juce::Colour kModMacro      { 0xFFFF7B25 };
    inline const juce::Colour kFilterPrimary { 0xFF7DFF3B };
    inline const juce::Colour kFilterCore    { 0xFF7DFF3B };
    inline const juce::Colour kOutput        { 0xFFFF3B3B };
    inline const juce::Colour kNoise         { 0xFFB66DFF };
    inline const juce::Colour kStepRand      { 0xFFFFD700 };

    // ---- Triad lifeforms (RockerToggle3) -------------------------------
    inline const juce::Colour kLifeformSynthetic { 0xFF00C8FF };
    inline const juce::Colour kLifeformCyborg    { 0xFFE08C00 };
    inline const juce::Colour kLifeformOrganic   { 0xFF6BBF59 };
    inline const juce::Colour kSynthetic         { 0xFF00C8FF };
    inline const juce::Colour kCyborg            { 0xFFE08C00 };
    inline const juce::Colour kOrganic           { 0xFF6BBF59 };

    /** 0 = Synthetic, 1 = Cyborg, 2 = Organic. Out-of-range -> Synthetic. */
    inline juce::Colour lifeformColour (int lifeform) noexcept
    {
        switch (lifeform)
        {
            case 1:  return kLifeformCyborg;
            case 2:  return kLifeformOrganic;
            default: return kLifeformSynthetic;
        }
    }
} // namespace Colours

namespace Typography
{
    inline constexpr float kLabelSize          = 12.0f;  // top-mounted label
    inline constexpr float kLabelHeroSize      = 12.5f;  // hero top label
    inline constexpr float kBadgeSize          = 12.5f;  // badge, dial >= 64 px
    inline constexpr float kBadgeCompactSize   = 11.0f;  // badge, dial 52..63 px
    inline constexpr float kPopupSize          = 13.5f;  // popup menu items
    inline constexpr float kComboSize          = 13.0f;  // combo box text
    inline constexpr int   kPopupItemHeight    = 26;     // min popup item height
    inline constexpr int   kAutoColumnThreshold = 18;    // items per popup column

    /** Bold-by-default font via FontOptions (no deprecated Font(float,int)). */
    inline juce::Font makeFont (float height, bool bold = true)
    {
        return bold ? juce::Font (juce::FontOptions (height).withStyle ("Bold")) : juce::Font (juce::FontOptions (height));
    }
} // namespace Typography
} // namespace CrackedChimes::UI
