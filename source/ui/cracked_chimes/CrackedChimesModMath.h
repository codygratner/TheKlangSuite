#pragma once
// CrackedChimesUI - modulation ring transfer functions + engineering formatter.
// PLAN.md section 4 (Concentric Ring Standard) and section 8 (anti-mojibake).
// All ring math is in NORMALISED slider-proportion space [0,1].

#include <juce_core/juce_core.h>
#include <cmath>

namespace CrackedChimes::UI
{
/** A normalised arc span, always lo <= hi, both clamped to [0,1]. */
struct NormRange
{
    float lo = 0.0f;
    float hi = 0.0f;
    constexpr float span() const noexcept { return hi - lo; }
};

constexpr float clampNorm (float v) noexcept { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }
constexpr float absf (float v) noexcept      { return v < 0.0f ? -v : v; }

/** Orders + clamps two normalised endpoints into a NormRange. */
constexpr NormRange makeRange (float a, float b) noexcept
{
    const float l = a < b ? a : b;
    const float h = a < b ? b : a;
    return NormRange { clampNorm (l), clampNorm (h) };
}

/** Symmetric +/- depth around centre (LFO / bipolar env). */
constexpr NormRange boundedBipolarRange (float centreNorm, float depthNorm) noexcept
{
    const float d = absf (depthNorm);
    return makeRange (centreNorm - d, centreNorm + d);
}

/** Unipolar push scaled by available headroom: amount in [-1,1].
    +a -> [base, base + a*(1-base)], -a -> [base + a*base, base]. */
constexpr NormRange directionalHeadroomRange (float baseNorm, float amount) noexcept
{
    const float b = clampNorm (baseNorm);
    const float a = amount < -1.0f ? -1.0f : (amount > 1.0f ? 1.0f : amount);
    return a >= 0.0f ? makeRange (b, b + a * (1.0f - b))
                     : makeRange (b + a * b, b);
}

/** Full sweep a macro can throw: unipolar macro -> [base, base+depth];
    bipolar macro -> [base-depth, base+depth]. depth is signed normalised. */
constexpr NormRange macroBipolarThrowRange (float baseNorm, float depthNorm, bool bipolarMacro) noexcept
{
    return bipolarMacro ? makeRange (baseNorm - depthNorm, baseNorm + depthNorm)
                        : makeRange (baseNorm, baseNorm + depthNorm);
}

/** Live macro position: macroValue in [0,1] (unipolar) or [-1,1] (bipolar). */
constexpr float macroPipNorm (float baseNorm, float depthNorm, float macroValue) noexcept
{
    return clampNorm (baseNorm + depthNorm * macroValue);
}

/** SteppedRandom window: value = k*(1-a) + a*r, r in [0,1].
    Window = [k*(1-a), k*(1-a) + a]. k = knob norm, a = amount [0,1]. */
constexpr NormRange steppedRandomWindow (float knobNorm, float amount) noexcept
{
    const float a = clampNorm (amount);
    const float base = clampNorm (knobNorm) * (1.0f - a);
    return makeRange (base, base + a);
}

/** Live SteppedRandom output for a held random r in [0,1]. */
constexpr float steppedRandomValue (float knobNorm, float amount, float r) noexcept
{
    const float a = clampNorm (amount);
    return clampNorm (clampNorm (knobNorm) * (1.0f - a) + a * clampNorm (r));
}

/** Shift a plain value by N octaves and return the SKEW-TRUE normalised
    position on the slider's range (e.g. cutoff ring under a log skew). */
inline float skewAwareOctaveShift (const juce::NormalisableRange<double>& range,
                                   double baseValue, double octaves)
{
    const double shifted = baseValue * std::pow (2.0, octaves);
    const double clamped = juce::jlimit (range.start, range.end, shifted);
    return clampNorm ((float) range.convertTo0to1 (clamped));
}

/** +/- octave span around baseValue, skew-true. */
inline NormRange skewAwareOctaveRange (const juce::NormalisableRange<double>& range,
                                       double baseValue, double octaveDepth)
{
    const double d = std::abs (octaveDepth);
    return makeRange (skewAwareOctaveShift (range, baseValue, -d),
                      skewAwareOctaveShift (range, baseValue,  d));
}

// ---------------------------------------------------------------------------
// Engineering value formatting (no String::formatted, ASCII units only).
// ---------------------------------------------------------------------------
enum class Unit
{
    None,              // caller falls back to Slider::getTextFromValue
    Hz,                // "440 Hz", "1.20 kHz" (above 999 Hz)
    Ms,                // "250 ms", "1.20 s" (>= 1000 ms)
    Seconds,           // "1.20 s", "250 ms" (< 1 s)
    dB,                // "-12.0 dB", "+3.00 dB", "-inf dB" (<= -100)
    Percent,           // value already 0..100 -> "50.0 %"
    PercentNormalised, // value 0..1 -> "50.0 %"
    Semitones,         // "+7 st", "-0.50 st"
    Octaves            // "+1.00 Oct"
};

/** 2 decimals below 10, 1 below 100, else 0. */
inline int engineeringDecimals (double v) noexcept
{
    const double a = std::abs (v);
    return a < 10.0 ? 2 : (a < 100.0 ? 1 : 0);
}

/** Fixed-decimal number with "-0.00" suppression. */
inline juce::String formatNumber (double v, int decimals)
{
    const double scale = std::pow (10.0, (double) decimals);
    if (std::abs (v) * scale < 0.5)
        v = 0.0;
    return decimals > 0 ? juce::String (v, decimals) : juce::String (juce::roundToInt (v));
}

inline juce::String formatSigned (double v, int decimals)
{
    const auto s = formatNumber (v, decimals);
    return (v > 0.0 && ! s.startsWithChar ('-') && s != formatNumber (0.0, decimals)) ? ("+" + s) : s;
}

/** Value in the unit's native scale -> display string, <= 2 decimals. */
inline juce::String formatEngineeringValue (double value, Unit unit)
{
    switch (unit)
    {
        case Unit::Hz:
            if (std::abs (value) > 999.0)
            {
                const double k = value / 1000.0;
                return formatNumber (k, std::abs (k) < 10.0 ? 2 : 1) + " kHz";
            }
            return formatNumber (value, engineeringDecimals (value)) + " Hz";

        case Unit::Ms:
            if (std::abs (value) >= 1000.0)
                return formatNumber (value / 1000.0, 2) + " s";
            return formatNumber (value, engineeringDecimals (value)) + " ms";

        case Unit::Seconds:
            if (std::abs (value) < 1.0)
            {
                const double ms = value * 1000.0;
                return formatNumber (ms, engineeringDecimals (ms)) + " ms";
            }
            return formatNumber (value, 2) + " s";

        case Unit::dB:
            if (value <= -100.0)
                return juce::String ("-inf dB");
            return formatSigned (value, std::abs (value) < 10.0 ? 2 : 1) + " dB";

        case Unit::Percent:
            return formatNumber (value, std::abs (value) < 100.0 ? 1 : 0) + " %";

        case Unit::PercentNormalised:
        {
            const double p = value * 100.0;
            return formatNumber (p, std::abs (p) < 100.0 ? 1 : 0) + " %";
        }

        case Unit::Semitones:
        {
            const bool whole = std::abs (value - std::round (value)) < 0.005;
            return formatSigned (value, whole ? 0 : 2) + " st";
        }

        case Unit::Octaves:
            return formatSigned (value, 2) + " Oct";

        case Unit::None:
        default:
            return formatNumber (value, engineeringDecimals (value));
    }
}
} // namespace CrackedChimes::UI
