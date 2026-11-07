#pragma once
// CrackedChimesEditorHarness.h -- editor scale lifecycle (PLAN.md section 7) and the
// per-editor widget harness (PLAN.md section 13). Header-only, namespace CrackedChimes::UI.

#include <juce_gui_basics/juce_gui_basics.h>
#include <algorithm>
#include <functional>
#include <memory>
#include <vector>
#include "../EditorScaleManager.h"
#include "CrackedChimesColours.h"
#include "CrackedChimesKnob.h"
#include "CrackedChimesOledCanvas.h"
#include "CrackedChimesHeaderInspector.h"

namespace CrackedChimes::UI
{

//==============================================================================
/** Scale lifecycle, part 1 (call from the editor constructor).
    Fix vs the PLAN sketch: getPreferredScale never returns <= 0 (its fallback is
    kDefaultScale), so the display-fit scale is applied as a cap rather than as a fallback. */
template <typename EditorT>
inline void initEditorScaling (EditorT& editor,
                               juce::ComponentBoundsConstrainer& constrainer,
                               int baseW, int baseH,
                               const juce::String& slug,
                               float sessionScale = 0.0f)
{
    using SM = rlyeh::ui::EditorScaleManager;
    constrainer.setFixedAspectRatio ((double) baseW / (double) baseH);
    constrainer.setSizeLimits (juce::roundToInt ((float) baseW * SM::kMinScale),
                               juce::roundToInt ((float) baseH * SM::kMinScale),
                               juce::roundToInt ((float) baseW * SM::kMaxScale),
                               juce::roundToInt ((float) baseH * SM::kMaxScale));
    editor.setConstrainer (&constrainer);
    editor.setResizable (true, true);

    float effScale = SM::getPreferredScale (slug, sessionScale);
    const float fit = SM::getDisplayFitScale (editor, baseW, baseH);
    effScale = juce::jlimit (SM::kMinScale, SM::kMaxScale, juce::jmin (effScale, fit));
    editor.setSize (juce::roundToInt ((float) baseW * effScale), juce::roundToInt ((float) baseH * effScale));
}

/** Scale lifecycle, part 2 (call from the editor resized()). Returns the applied scale. */
inline float applyEditorResized (juce::Component& contentContainer,
                                 rlyeh::ui::EditorScaleManager::Writer& scaleWriter,
                                 int w, int h, int baseW, int baseH,
                                 const juce::String& slug)
{
    using SM = rlyeh::ui::EditorScaleManager;
    float s = juce::jmin ((float) w / (float) baseW, (float) h / (float) baseH);
    s = juce::jlimit (SM::kMinScale, SM::kMaxScale, s);

    const float ox = (float) (w - juce::roundToInt ((float) baseW * s)) * 0.5f;
    const float oy = (float) (h - juce::roundToInt ((float) baseH * s)) * 0.5f;

    contentContainer.setBounds (0, 0, baseW, baseH);
    contentContainer.setTransform (juce::AffineTransform::scale (s).translated (ox, oy));

    scaleWriter.note (slug, s);
    return s;
}

//==============================================================================
/** A helper the editor owns. It binds knobs to the HeaderInspector (hover -> show,
    exit -> idle), runs modal overlays, and suspends canvas timers.
    Badge expiry: CrackedChimesKnob already handles this itself (its private timer
    and kBadgeHoldMs = 350), so the harness adds no badge timer of its own.
    Declare the harness AFTER the knobs, canvases and inspector it references, so
    that it is destroyed first. */
class CrackedChimesEditorHarness : private juce::MouseListener,
                                   private juce::Slider::Listener
{
public:
    CrackedChimesEditorHarness() = default;

    ~CrackedChimesEditorHarness() override
    {
        dismissModal();
        for (auto& b : bindings)
            if (b.knob != nullptr)
            {
                b.knob->removeMouseListener (this);
                b.knob->removeListener (this);
            }
    }

    CrackedChimesEditorHarness (const CrackedChimesEditorHarness&) = delete;
    CrackedChimesEditorHarness& operator= (const CrackedChimesEditorHarness&) = delete;

    //==============================================================================
    /** The inspector this harness drives. The harness only ever calls
        show(title, valueText, blurb, accent) and setIdle(subtitle) on it. */
    void attachInspector (CrackedChimesHeaderInspector* inspectorToUse, juce::String idleSubtitleText = {})
    {
        inspector = inspectorToUse;
        idleSubtitle = std::move (idleSubtitleText);
        goIdle();
    }

    void setIdleSubtitle (juce::String s)   { idleSubtitle = std::move (s); if (hovered == nullptr) goIdle(); }

    /** Registers a knob for inspector binding. The paramId is kept for lookup only
        and is never renamed. */
    void registerKnob (CrackedChimesKnob& knob, juce::String paramId, juce::String title,
                       juce::String blurb, juce::Colour accent = Colours::kOscCore)
    {
        if (auto* existing = findBinding (&knob))
        {
            existing->paramId = std::move (paramId);
            existing->title = std::move (title);
            existing->blurb = std::move (blurb);
            existing->accent = accent;
            return;
        }
        bindings.push_back ({ &knob, std::move (paramId), std::move (title), std::move (blurb), accent });
        knob.addMouseListener (this, false);
        knob.addListener (this);
    }

    void unregisterKnob (CrackedChimesKnob& knob)
    {
        knob.removeMouseListener (this);
        knob.removeListener (this);
        if (hovered == &knob) { hovered = nullptr; goIdle(); }
        bindings.erase (std::remove_if (bindings.begin(), bindings.end(),
                                        [&knob] (const Binding& b) { return b.knob == &knob; }),
                        bindings.end());
    }

    CrackedChimesKnob* findKnob (const juce::String& paramId) const
    {
        for (auto& b : bindings)
            if (b.paramId == paramId) return b.knob;
        return nullptr;
    }

    //==============================================================================
    /** Canvases whose timers follow setTimersSuspended(). */
    void registerCanvas (CrackedChimesOledCanvas& canvas)
    {
        if (std::find (canvases.begin(), canvases.end(), &canvas) == canvases.end())
            canvases.push_back (&canvas);
        canvas.setSuspended (timersSuspended);
    }

    void unregisterCanvas (CrackedChimesOledCanvas& canvas)
    {
        canvases.erase (std::remove (canvases.begin(), canvases.end(), &canvas), canvases.end());
    }

    /** Timer suspension hook: covers the registered canvases, plus the user callback
        for editor-owned timers. Showing a modal suspends timers automatically. */
    void setTimersSuspended (bool shouldSuspend)
    {
        if (timersSuspended == shouldSuspend) return;
        timersSuspended = shouldSuspend;
        for (auto* c : canvases) c->setSuspended (shouldSuspend);
        if (onTimersSuspended) onTimersSuspended (shouldSuspend);
    }

    bool areTimersSuspended() const noexcept { return timersSuspended; }

    std::function<void (bool suspended)> onTimersSuspended;

    //==============================================================================
    /** The component that modals cover (usually the editor content container). */
    void setModalHost (juce::Component* host) { modalHost = host; }

    /** Shows content centred over a dimmed scrim. A click on the scrim or the Escape
        key dismisses it. Replaces any modal that is already open. */
    void showModal (std::unique_ptr<juce::Component> content, bool suspendTimersWhileOpen = true)
    {
        dismissModal();
        if (modalHost == nullptr || content == nullptr) return;
        modal = std::make_unique<ModalScrim> (*this, std::move (content));
        modalHost->addAndMakeVisible (*modal);
        modal->setBounds (modalHost->getLocalBounds());
        modal->toFront (true);
        modal->grabKeyboardFocus();
        suspendedByModal = suspendTimersWhileOpen && ! timersSuspended;
        if (suspendedByModal) setTimersSuspended (true);
    }

    void dismissModal()
    {
        if (modal == nullptr) return;
        auto doomed = std::move (modal);
        if (auto* p = doomed->getParentComponent()) p->removeChildComponent (doomed.get());
        doomed.reset();
        if (suspendedByModal) { suspendedByModal = false; setTimersSuspended (false); }
        if (onModalDismissed) onModalDismissed();
    }

    bool isModalShowing() const noexcept { return modal != nullptr; }
    juce::Component* getModalContent() const noexcept { return modal != nullptr ? modal->content.get() : nullptr; }

    std::function<void()> onModalDismissed;

    /** Call from the editor resized() so an open modal follows the host bounds. */
    void layoutModal()
    {
        if (modal != nullptr && modalHost != nullptr) modal->setBounds (modalHost->getLocalBounds());
    }

private:
    struct Binding
    {
        CrackedChimesKnob* knob = nullptr;
        juce::String paramId, title, blurb;
        juce::Colour accent;
    };

    //==============================================================================
    class ModalScrim : public juce::Component
    {
    public:
        ModalScrim (CrackedChimesEditorHarness& o, std::unique_ptr<juce::Component> c)
            : owner (o), content (std::move (c))
        {
            setWantsKeyboardFocus (true);
            addAndMakeVisible (*content);
        }

        void paint (juce::Graphics& g) override { g.fillAll (Colours::kChassisBg.withAlpha (0.72f)); }

        void resized() override
        {
            auto b = content->getBounds();
            if (b.isEmpty()) b = getLocalBounds().reduced (getWidth() / 6, getHeight() / 6);
            content->setBounds (b.withCentre (getLocalBounds().getCentre()).constrainedWithin (getLocalBounds()));
        }

        void mouseDown (const juce::MouseEvent& e) override
        {
            if (e.eventComponent == this) juce::MessageManager::callAsync (
                [safe = juce::Component::SafePointer<ModalScrim> (this)]
                { if (safe != nullptr) safe->owner.dismissModal(); });
        }

        bool keyPressed (const juce::KeyPress& k) override
        {
            if (k == juce::KeyPress::escapeKey)
            {
                juce::MessageManager::callAsync (
                    [safe = juce::Component::SafePointer<ModalScrim> (this)]
                    { if (safe != nullptr) safe->owner.dismissModal(); });
                return true;
            }
            return false;
        }

        CrackedChimesEditorHarness& owner;
        std::unique_ptr<juce::Component> content;
    };

    //==============================================================================
    Binding* findBinding (const juce::Component* c)
    {
        for (auto& b : bindings)
            if (b.knob == c) return &b;
        return nullptr;
    }

    void showFor (const Binding& b)
    {
        if (inspector != nullptr)
            inspector->show (b.title, b.blurb, b.accent);
    }

    void goIdle()
    {
        if (inspector != nullptr) inspector->setIdle (idleSubtitle);
    }

    // juce::MouseListener
    void mouseEnter (const juce::MouseEvent& e) override
    {
        if (auto* b = findBinding (e.eventComponent)) { hovered = b->knob; showFor (*b); }
    }

    void mouseExit (const juce::MouseEvent& e) override
    {
        auto* b = findBinding (e.eventComponent);
        if (b == nullptr || b->knob != hovered) return;
        if (b->knob->isMouseButtonDown()) return;   // keep showing while dragging
        hovered = nullptr;
        goIdle();
    }

    void mouseUp (const juce::MouseEvent& e) override
    {
        if (auto* b = findBinding (e.eventComponent))
            if (! b->knob->isMouseOver (true) && hovered == b->knob) { hovered = nullptr; goIdle(); }
    }

    // juce::Slider::Listener: live value text while hovering or dragging
    void sliderValueChanged (juce::Slider* s) override
    {
        if (auto* b = findBinding (s))
            if (hovered == b->knob || s->isMouseButtonDown()) showFor (*b);
    }

    std::vector<Binding> bindings;
    std::vector<CrackedChimesOledCanvas*> canvases;
    CrackedChimesHeaderInspector* inspector = nullptr;
    CrackedChimesKnob* hovered = nullptr;
    juce::String idleSubtitle;
    juce::Component* modalHost = nullptr;
    std::unique_ptr<ModalScrim> modal;
    bool timersSuspended = false, suspendedByModal = false;
};

} // namespace CrackedChimes::UI
