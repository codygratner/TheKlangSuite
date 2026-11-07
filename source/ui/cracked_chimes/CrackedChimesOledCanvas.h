#pragma once
// CrackedChimesOledCanvas.h -- Pure OLED Display Standard (PLAN.md section 10).
// Header-only, namespace CrackedChimes::UI.
//
// THE PURE OLED LAW: this base class provides NO text-drawing helpers, and
// subclasses must never call drawText / drawFittedText / drawSingleLineText.
// The chassis silkscreen, knob badges and HeaderInspector carry all text.

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <atomic>
#include <cstdint>
#include <functional>
#include <vector>
#include <cmath>
#include "CrackedChimesColours.h"

namespace CrackedChimes::UI
{

//==============================================================================
/** Recessed OLED cavity base: #0F131A glass, #262D38 border, reticle grid,
    dual-pass neon glow helpers. Timer auto-throttles while not showing. */
class CrackedChimesOledCanvas : public juce::Component,
                                private juce::Timer
{
public:
    static constexpr float kCornerRadius = 4.0f;
    static constexpr float kBorderWidth  = 1.0f;
    static constexpr float kPadding      = 5.0f;   // 4-6 px edge padding
    static constexpr float kGlowWidth    = 3.5f;
    static constexpr float kGlowAlpha    = 0.25f;
    static constexpr float kCoreWidth    = 1.6f;

    CrackedChimesOledCanvas()
    {
        setOpaque (false);
        setInterceptsMouseClicks (false, false);
    }

    ~CrackedChimesOledCanvas() override { stopTimer(); }

    //==============================================================================
    void setAccentColour (juce::Colour c)   { if (c != accent) { accent = c; repaint(); } }
    juce::Colour getAccentColour() const noexcept { return accent; }

    void setGridColour (juce::Colour c)     { if (c != grid) { grid = c; repaint(); } }
    void setGridDivisions (int cols, int rows) { gridCols = juce::jmax (0, cols); gridRows = juce::jmax (0, rows); repaint(); }

    /** Refresh rate while shown; 0 disables the animation timer. */
    void setRefreshHz (int hz)              { refreshHz = juce::jmax (0, hz); updateTimerState(); }
    int  getRefreshHz() const noexcept      { return refreshHz; }

    /** External suspension hook (e.g. CrackedChimesEditorHarness). */
    void setSuspended (bool shouldSuspend)  { suspended = shouldSuspend; updateTimerState(); }
    bool isSuspended() const noexcept       { return suspended; }

    bool isAnimating() const noexcept       { return isTimerRunning(); }

    //==============================================================================
    void paint (juce::Graphics& g) override
    {
        const auto outer = getLocalBounds().toFloat().reduced (0.5f);
        g.setColour (Colours::kCavity);
        g.fillRoundedRectangle (outer, kCornerRadius);

        const auto screen = getScreenArea();
        {
            juce::Graphics::ScopedSaveState sss (g);
            g.reduceClipRegion (outer.reduced (kBorderWidth).toNearestInt());
            drawReticle (g, screen);
            paintScreen (g, screen);
        }

        g.setColour (Colours::kBadgeBorder);   // #262D38
        g.drawRoundedRectangle (outer, kCornerRadius, kBorderWidth);
    }

    /** Plot area (cavity minus edge padding). */
    juce::Rectangle<float> getScreenArea() const
    {
        return getLocalBounds().toFloat().reduced (kBorderWidth + kPadding);
    }

protected:
    /** Subclasses draw curves here. NO TEXT. */
    virtual void paintScreen (juce::Graphics&, juce::Rectangle<float> /*screen*/) {}

    /** Called at refreshHz while showing (and not suspended). Default repaints. */
    virtual void onFrame() { repaint(); }

    //==============================================================================
    /** Dual-pass neon phosphor stroke: 3.5px @ 0.25 bloom, then crisp core beam. */
    static void strokeGlow (juce::Graphics& g, const juce::Path& path, juce::Colour colour,
                            float coreWidth = kCoreWidth)
    {
        g.setColour (colour.withAlpha (kGlowAlpha));
        g.strokePath (path, juce::PathStrokeType (kGlowWidth, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
        g.setColour (colour);
        g.strokePath (path, juce::PathStrokeType (coreWidth, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    }

    /** Translucent under-curve fill: alpha 0.25 at topY -> 0.01 at baselineY.
        `closedPath` must already be closed down to the baseline. */
    static void fillUnderCurve (juce::Graphics& g, const juce::Path& closedPath, juce::Colour colour,
                                float topY, float baselineY)
    {
        juce::ColourGradient grad (colour.withAlpha (0.25f), 0.0f, topY,
                                   colour.withAlpha (0.01f), 0.0f, baselineY, false);
        g.setGradientFill (grad);
        g.fillPath (closedPath);
    }

    /** Glowing dot (playhead / phase marker). */
    static void drawGlowDot (juce::Graphics& g, juce::Point<float> p, juce::Colour colour,
                             float diameter = 5.0f)
    {
        g.setColour (colour.withAlpha (kGlowAlpha));
        g.fillEllipse (juce::Rectangle<float> (diameter + 4.0f, diameter + 4.0f).withCentre (p));
        g.setColour (Colours::kNeedle);
        g.fillEllipse (juce::Rectangle<float> (diameter, diameter).withCentre (p));
    }

    void visibilityChanged() override       { updateTimerState(); }
    void parentHierarchyChanged() override  { updateTimerState(); }

private:
    void drawReticle (juce::Graphics& g, juce::Rectangle<float> r) const
    {
        g.setColour (grid);
        for (int i = 1; i < gridCols; ++i)
        {
            const float x = std::round (r.getX() + r.getWidth() * (float) i / (float) gridCols) + 0.5f;
            g.drawVerticalLine ((int) x, r.getY(), r.getBottom());
        }
        for (int j = 1; j < gridRows; ++j)
        {
            const float y = std::round (r.getY() + r.getHeight() * (float) j / (float) gridRows) + 0.5f;
            g.drawHorizontalLine ((int) y, r.getX(), r.getRight());
        }
    }

    void updateTimerState()
    {
        const bool shouldRun = refreshHz > 0 && ! suspended && isShowing();
        if (shouldRun)
        {
            if (! isTimerRunning() || runningHz != refreshHz)
            {
                runningHz = refreshHz;
                startTimerHz (refreshHz);
            }
        }
        else if (isTimerRunning())
        {
            stopTimer();
        }
    }

    void timerCallback() override
    {
        if (! isShowing()) { stopTimer(); return; }   // belt-and-braces throttle
        onFrame();
    }

    juce::Colour accent { Colours::kOscCore };
    juce::Colour grid   { juce::Colour (0xFF1E2530) };
    int gridCols = 4, gridRows = 2;
    int refreshHz = 0, runningHz = 0;
    bool suspended = false;
};

//==============================================================================
/** ADSR envelope display with optional live playhead. Pure OLED: no text. */
class CrackedChimesEnvelopeVisualizer : public CrackedChimesOledCanvas
{
public:
    CrackedChimesEnvelopeVisualizer() { setAccentColour (Colours::kFilterPrimary); }

    /** a/d/r in seconds (any consistent unit), s in [0,1], curve in [-1,1]
        (0 linear, >0 exponential-style bow, <0 logarithmic-style bow). */
    void setADSR (float a, float d, float s, float r, float curve = 0.0f)
    {
        a = juce::jmax (0.0f, a); d = juce::jmax (0.0f, d); r = juce::jmax (0.0f, r);
        s = juce::jlimit (0.0f, 1.0f, s); curve = juce::jlimit (-1.0f, 1.0f, curve);
        if (a == atk && d == dec && s == sus && r == rel && curve == crv) return;
        atk = a; dec = d; sus = s; rel = r; crv = curve;
        repaint();
    }

    /** Live playhead across the whole drawn envelope, [0,1]; < 0 hides it.
        Safe to call from the message thread; for audio telemetry, write to
        an atomic and enable setRefreshHz so onFrame polls it. */
    void setPlayheadNorm (float norm)       { playhead.store (norm, std::memory_order_relaxed); }
    std::atomic<float>& playheadSource() noexcept { return playhead; }

protected:
    void paintScreen (juce::Graphics& g, juce::Rectangle<float> r) override
    {
        const float sustainHold = 0.25f * juce::jmax (0.05f, atk + dec + rel);
        const float total = juce::jmax (1.0e-6f, atk + dec + sustainHold + rel);
        const float x0 = r.getX(), w = r.getWidth(), top = r.getY(), base = r.getBottom(), h = r.getHeight();
        auto xAt = [&] (float t) { return x0 + w * (t / total); };
        auto yAt = [&] (float lvl) { return base - h * lvl; };

        curvePath.clear();
        curvePath.startNewSubPath (x0, base);
        appendSegment (xAt (0.0f), 0.0f, xAt (atk), 1.0f, yAt, true);
        appendSegment (xAt (atk), 1.0f, xAt (atk + dec), sus, yAt, false);
        curvePath.lineTo (xAt (atk + dec + sustainHold), yAt (sus));
        appendSegment (xAt (atk + dec + sustainHold), sus, xAt (total), 0.0f, yAt, false);

        fillPath = curvePath;
        fillPath.lineTo (xAt (total), base);
        fillPath.closeSubPath();

        const auto c = getAccentColour();
        fillUnderCurve (g, fillPath, c, top, base);
        strokeGlow (g, curvePath, c);

        const float ph = playhead.load (std::memory_order_relaxed);
        if (ph >= 0.0f && ph <= 1.0f)
        {
            const float px = x0 + w * ph;
            g.setColour (c.withAlpha (0.35f));
            g.drawVerticalLine ((int) std::round (px), top, base);
            drawGlowDot (g, { px, yAt (levelAt (ph * total, sustainHold)) }, c);
        }
    }

    void onFrame() override
    {
        const float ph = playhead.load (std::memory_order_relaxed);
        if (std::abs (ph - lastDrawnPlayhead) > 0.001f) { lastDrawnPlayhead = ph; repaint(); }
    }

private:
    static constexpr int kSegSteps = 24;

    template <typename YFn>
    void appendSegment (float xA, float lA, float xB, float lB, YFn yAt, bool rising)
    {
        for (int i = 1; i <= kSegSteps; ++i)
        {
            const float t = (float) i / (float) kSegSteps;
            const float lvl = lA + (lB - lA) * shapeFor (t, rising);
            curvePath.lineTo (xA + (xB - xA) * t, yAt (lvl));
        }
    }

    /** Rising (attack) bows concave-down for curve > 0; decays bow concave-up. */
    float shapeFor (float t, bool rising) const
    {
        if (std::abs (crv) < 1.0e-3f) return t;
        const float k = crv * 4.0f;
        const float e = (1.0f - std::exp (-k * t)) / (1.0f - std::exp (-k));
        juce::ignoreUnused (rising);   // same normalised shape; lA->lB direction handles rise/fall
        return e;
    }

    float levelAt (float t, float sustainHold) const
    {
        if (t < atk)               return atk > 0.0f ? shapeFor (t / atk, true) : 1.0f;
        t -= atk;
        if (t < dec)               return 1.0f + (sus - 1.0f) * (dec > 0.0f ? shapeFor (t / dec, false) : 1.0f);
        t -= dec;
        if (t < sustainHold)       return sus;
        t -= sustainHold;
        if (rel <= 0.0f)           return 0.0f;
        return sus * (1.0f - shapeFor (juce::jmin (1.0f, t / rel), false));
    }

    float atk = 0.01f, dec = 0.2f, sus = 0.7f, rel = 0.3f, crv = 0.0f;
    std::atomic<float> playhead { -1.0f };
    float lastDrawnPlayhead = -1.0f;
    juce::Path curvePath, fillPath;
};

//==============================================================================
/** Single-cycle waveform display. Pure OLED: no text. */
class CrackedChimesWaveformVisualizer : public CrackedChimesOledCanvas
{
public:
    static constexpr int kMaxPoints = 512;

    CrackedChimesWaveformVisualizer() { setAccentColour (Colours::kOscCore); }

    /** Analytic waveform, phase in [0,1) -> value in [-1,1]. Message thread. */
    void setWaveFn (std::function<float (float)> fn)
    {
        waveFn = std::move (fn);
        useSamples = false;
        rebuildCache();
    }

    /** Copies up to kMaxPoints samples (decimated). Message thread. */
    void setSamples (const float* data, int num)
    {
        useSamples = true;
        numCached = 0;
        if (data != nullptr && num > 0)
        {
            const int n = juce::jmin (num, kMaxPoints);
            for (int i = 0; i < n; ++i)
            {
                const int src = (num <= kMaxPoints) ? i : (int) ((int64_t) i * num / n);
                cache[(size_t) i] = juce::jlimit (-1.0f, 1.0f, data[src]);
            }
            numCached = n;
        }
        repaint();
    }

    void setFilled (bool shouldFill) { filled = shouldFill; repaint(); }

protected:
    void paintScreen (juce::Graphics& g, juce::Rectangle<float> r) override
    {
        if (numCached < 2) return;
        const float midY = r.getCentreY(), halfH = r.getHeight() * 0.5f;
        wavePath.clear();
        for (int i = 0; i < numCached; ++i)
        {
            const float x = r.getX() + r.getWidth() * (float) i / (float) (numCached - 1);
            const float y = midY - halfH * cache[(size_t) i];
            if (i == 0) wavePath.startNewSubPath (x, y); else wavePath.lineTo (x, y);
        }
        const auto c = getAccentColour();
        if (filled)
        {
            fillPath = wavePath;
            fillPath.lineTo (r.getRight(), midY);
            fillPath.lineTo (r.getX(), midY);
            fillPath.closeSubPath();
            fillUnderCurve (g, fillPath, c, r.getY(), midY);
        }
        strokeGlow (g, wavePath, c);
    }

    void resized() override { if (! useSamples) rebuildCache(); }

    /** Value at normalised x for subclasses (LFO dot). */
    float valueAtNorm (float x) const
    {
        if (numCached < 2) return 0.0f;
        const float fi = juce::jlimit (0.0f, 1.0f, x) * (float) (numCached - 1);
        const int i0 = (int) fi;
        const int i1 = juce::jmin (i0 + 1, numCached - 1);
        const float fr = fi - (float) i0;
        return cache[(size_t) i0] + (cache[(size_t) i1] - cache[(size_t) i0]) * fr;
    }

private:
    void rebuildCache()
    {
        numCached = 0;
        if (waveFn)
        {
            const int n = juce::jlimit (32, kMaxPoints, (int) getScreenArea().getWidth());
            for (int i = 0; i < n; ++i)
                cache[(size_t) i] = juce::jlimit (-1.0f, 1.0f, waveFn ((float) i / (float) (n - 1) * 0.999999f));
            numCached = n;
        }
        repaint();
    }

    std::function<float (float)> waveFn;
    std::array<float, (size_t) kMaxPoints> cache {};
    int numCached = 0;
    bool useSamples = false, filled = false;
    juce::Path wavePath, fillPath;
};

//==============================================================================
/** LFO monitor: waveform + live phase dot. Pure OLED: no text.
    Audio/UI telemetry: write phase into phaseSource() (atomic) and call
    setRefreshHz (30..60) so onFrame polls it. */
class CrackedChimesLfoVisualizer : public CrackedChimesWaveformVisualizer
{
public:
    CrackedChimesLfoVisualizer()
    {
        setAccentColour (Colours::kModLfo);
        setRefreshHz (30);
    }

    void setPhase (float phase01)                { phase.store (phase01, std::memory_order_relaxed); }
    std::atomic<float>& phaseSource() noexcept   { return phase; }
    void setShowPhaseDot (bool show)             { showDot = show; repaint(); }

protected:
    void paintScreen (juce::Graphics& g, juce::Rectangle<float> r) override
    {
        CrackedChimesWaveformVisualizer::paintScreen (g, r);
        if (! showDot) return;
        float ph = phase.load (std::memory_order_relaxed);
        if (! std::isfinite (ph) || ph < 0.0f) return;
        ph = ph - std::floor (ph);
        const float x = r.getX() + r.getWidth() * ph;
        const float y = r.getCentreY() - r.getHeight() * 0.5f * valueAtNorm (ph);
        drawGlowDot (g, { x, y }, getAccentColour());
    }

    void onFrame() override
    {
        const float ph = phase.load (std::memory_order_relaxed);
        if (std::abs (ph - lastDrawn) > 0.0005f) { lastDrawn = ph; repaint(); }
    }

private:
    std::atomic<float> phase { -1.0f };
    float lastDrawn = -2.0f;
    bool showDot = true;
};

} // namespace CrackedChimes::UI
