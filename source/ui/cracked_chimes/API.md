# CrackedChimesUI Core API (Wave 1-A)

Folder: `source/ui/cracked_chimes/`. Header-only, namespace `CrackedChimes::UI`. Include `CrackedChimesLookAndFeel.h` to get all four.

## CrackedChimesColours.h
- `Colours::kChassisBg #0D1117, kChassisDisc #161B22, kChassisRim #30363D, kBadgeBg #090D12, kBadgeBorder #262D38, kCavity #0F131A, kNeedle #F0F6FC, kText #C9D1D9` (`inline const juce::Colour`)
- `Colours::kOscCore #00C8FF, kOscAlt #FF7A1A, kModLfo #F2E10F, kModMacro #FF7B25, kFilterPrimary #7DFF3B, kOutput #FF3B3B, kNoise #B66DFF, kStepRand #FFD700`
- `Colours::kLifeformSynthetic #00C8FF, kLifeformCyborg #E08C00, kLifeformOrganic #6BBF59`
- `juce::Colour Colours::lifeformColour(int lifeform) noexcept`: 0 Synthetic, 1 Cyborg, 2 Organic, anything else falls back to Synthetic.
- `Typography::kLabelSize 12.0f, kLabelHeroSize 12.5f, kBadgeSize 12.5f, kBadgeCompactSize 11.0f, kPopupSize 13.5f, kComboSize 13.0f, kPopupItemHeight 26, kAutoColumnThreshold 18`
- `juce::Font Typography::makeFont(float height, bool bold = true)`: builds the font through FontOptions.

## CrackedChimesModMath.h (normalised [0,1] space)
- `struct NormRange { float lo, hi; constexpr float span() const; }`
- `constexpr float clampNorm(float)`, `constexpr float absf(float)`
- `constexpr NormRange makeRange(float a, float b)`: puts the ends in order and clamps them.
- `constexpr NormRange boundedBipolarRange(float centreNorm, float depthNorm)`: centre +/- |depth|.
- `constexpr NormRange directionalHeadroomRange(float baseNorm, float amount)`: amount in [-1,1]. Positive gives [b, b+a(1-b)], negative gives [b+a*b, b].
- `constexpr NormRange macroBipolarThrowRange(float baseNorm, float depthNorm, bool bipolarMacro)`: unipolar gives [b, b+d], bipolar gives [b-d, b+d].
- `constexpr float macroPipNorm(float baseNorm, float depthNorm, float macroValue)`: clamp(b + d*m).
- `constexpr NormRange steppedRandomWindow(float knobNorm, float amount)`: [k(1-a), k(1-a)+a].
- `constexpr float steppedRandomValue(float knobNorm, float amount, float r)`: k(1-a) + a*r.
- `float skewAwareOctaveShift(const juce::NormalisableRange<double>&, double baseValue, double octaves)`: normalised position of base*2^oct, taking the skew into account.
- `NormRange skewAwareOctaveRange(const juce::NormalisableRange<double>&, double baseValue, double octaveDepth)`: +/- octaves around the base.
- `enum class Unit { None, Hz, Ms, Seconds, dB, Percent, PercentNormalised, Semitones, Octaves }`
- `int engineeringDecimals(double)`: 2 decimals below 10, 1 below 100, otherwise 0.
- `juce::String formatNumber(double v, int decimals)`, `juce::String formatSigned(double v, int decimals)`: suppress "-0".
- `juce::String formatEngineeringValue(double value, Unit unit)`: Hz switches to kHz above 999. Ms switches to s at 1000 or more. Seconds switches to ms below 1. dB values of -100 or lower show as "-inf dB", other dB values are signed. Percent takes 0..100; PercentNormalised takes 0..1. st and Oct are signed. Never more than 2 decimals.

## CrackedChimesKnob.h
- `enum class RingSource { Envelope, Lfo, Macro, SteppedRandom, Chaos, AudioRateXMod }`. AudioRateXMod draws its range only: no pip, no ghosts.
- `struct ModRingState { bool active=false; float loNorm=0, hiNorm=0; bool hasPip=false; float pipNorm=0; juce::Colour colour=kModLfo; RingSource source=Lfo; std::array<float,7> ghostPips{}; int numGhosts=0; }`
- Constants: `kMaxRings 4, kMaxGhostPips 7, kRingPitch 3.0f, kRingStroke 2.0f, kRingFirstOffset 1.0f, kRingOutwardExtent 14.0f, kRingDrawEpsilon 0.003f, kRingDirtyEpsilon 0.001f, kLivePipDiameter 5.0f, kLivePipGlow 7.0f, kGhostPipDiameter 3.0f, kGhostPipAlpha 0.35f, kHeroDialMin 64.0f`
- `struct DialGeometry { juce::Point<float> centre; float diameter, rCore, rTrack, rDisc; juce::Rectangle<float> badge; bool compact; }`
- `DialGeometry computeDialGeometry(juce::Rectangle<float> area) noexcept`: rCore = d/2 - 14, rTrack = rCore-4, rDisc = rCore-8. The badge is 19px tall (15.5px when d < 64), 0.72*d wide, and centred at cy + 0.86*rCore.
- `juce::Point<float> pointOnArc(juce::Point<float> c, float radius, float angle) noexcept`: uses the JUCE angle convention.
- `class CrackedChimesKnob : public juce::Slider, private juce::Timer`
  - `static constexpr int kBadgeHoldMs=350, kNormalDragPixels=250, kFineDragPixels=2500`
  - `CrackedChimesKnob()`, `explicit CrackedChimesKnob(const juce::String& name)`: RotaryHorizontalVerticalDrag, NoTextBox, rotary angles 1.25pi..2.75pi. Keep NoTextBox so the badge hit-test lines up.
  - `void setRing(int slot, const ModRingState&)`: slot equals priority (0 env, 1 lfo, 2 macro/chaos, 3 steprand/xmod). Repaints only when something moves more than 0.001.
  - `void clearRing(int slot)`, `void clearRings()`
  - `const ModRingState& getRing(int slot) const noexcept`, `const std::array<ModRingState,4>& getRings() const noexcept`
  - `void setBipolar(bool)` / `bool isBipolar()`, `void setHero(bool)` / `bool isHero()`, `void setUnit(Unit)` / `Unit getUnit()`
  - `void setDefaultValue(double)`, `double getResetValue() const`: if no default was set, bipolar resets to 0 and anything else to the minimum.
  - `bool isBadgeHot() const noexcept`: true while hovering or dragging, and for 350ms afterwards.
  - `juce::String getBadgeText() const`: formatEngineeringValue, or getTextFromValue when Unit::None.
  - `juce::Rectangle<float> getBadgeBounds() const`, `bool isEditingValue() const noexcept`
  - `double parseValueText(const juce::String&, bool& ok) const`: accepts input like "440 Hz", "1.2k", "-12 dB", "250 ms", "1.5 s", "7 st", "1 oct", "-inf". The result is clamped to the range.
  - `void showValueEditor()`, `void hideValueEditor(bool commit)`: commit goes through `setValue(v, sendNotificationSync)`, which the attachment wraps in a gesture.
  - `std::function<void(juce::PopupMenu&)> onBuildContextMenu`: right-click hook. An empty menu is not shown.
  - Mouse behaviour: a click on the badge opens the editor. Double-click on the body resets. Shift+drag is fine control (0.1x) and re-anchors with no jump. The wheel holds the badge hot.

## CrackedChimesLookAndFeel.h
- `class CrackedChimesLookAndFeel : public juce::LookAndFeel_V4`
  - `drawRotarySlider(...)`: draws the track (rTrack, 4px), the value arc (noon is skew-true when bipolar), the disc and needle, the rings, then the badge. The rings and badge are drawn only when the slider is a CrackedChimesKnob.
  - Rings: each active slot is packed outward at rCore + 1 + 3*p with a 2px stroke. Each ring gets a live pip (5px white dot over a 7px glow) except AudioRateXMod. Up to 7 ghost pips are drawn at 3px and 35% alpha.
  - `drawLinearSlider(...)`: 6px recessed groove and a 22x12 fader cap. Styles other than LinearVertical/LinearHorizontal fall back to V4.
  - `drawComboBox(...)`, `juce::Font getComboBoxFont(juce::ComboBox&)` (13px bold), `juce::Font getPopupMenuFont()` (13.5px bold), `getIdealPopupMenuItemSize(...)` (item height at least 26)
  - `static void drawTopLabel(juce::Graphics&, juce::Rectangle<int>, const juce::String&, bool hero=false, juce::Colour=kText)`
  - `static juce::PopupMenu::Options withAutoColumns(juce::PopupMenu::Options, int numItems)`: gives ceil(n/18) columns when there are more than 18 items.
