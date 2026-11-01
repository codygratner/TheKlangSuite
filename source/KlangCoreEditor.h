#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <map>
#include "KlangCoreProcessor.h"
#include "UIComponents.h"
#include "SettingsModal.h"
#include "VersionChecker.h"

class KlangCoreEditor : public juce::AudioProcessorEditor
{
public:
    explicit KlangCoreEditor(KlangCoreProcessor& p);
    ~KlangCoreEditor() override;

protected:
    KlangCoreProcessor& coreProcessor;
    RotaryKnobLookAndFeel knobLookAndFeel;
    std::unique_ptr<juce::TooltipWindow> tooltipWindow;
    bool tooltipsEnabled = false;

    // Header buttons (shared)
    GearButton settingsButton;
    UpdateBadgeButton updateBadgeButton;
    juce::TextButton tooltipsButton;
    juce::TextButton presetButton;
    juce::TextButton initButton;
    juce::TextButton triggerButton;
    juce::TextButton guideButton;

    std::unique_ptr<SettingsModalComponent> settingsModal;
    StatusBarComponent statusBar;

public:
    StatusBarComponent& getStatusBar() { return statusBar; }
    juce::TextButton& getTooltipsButton() { return tooltipsButton; }
    juce::TextButton& getPresetButton() { return presetButton; }
    juce::TextButton& getInitButton() { return initButton; }
    juce::TextButton& getTriggerButton() { return triggerButton; }
    juce::TextButton& getGuideButton() { return guideButton; }
    GearButton& getSettingsButton() { return settingsButton; }

    void simulateHeaderHover(juce::Component* comp);
    void simulateHeaderExit(juce::Component* comp);

    void wireHeaderHover(juce::Component& comp,
                         const juce::String& name,
                         std::function<juce::String()> getValue,
                         const juce::String& desc,
                         const juce::String& clickHint,
                         const juce::String& rightClickHint = {});

    void wireHeaderHover(juce::Component& comp,
                         const juce::String& name,
                         const juce::String& value,
                         const juce::String& desc,
                         const juce::String& clickHint,
                         const juce::String& rightClickHint = {});

    void unwireHeaderHover(juce::Component& comp);

protected:
    void setTooltipsEnabled(bool enabled);

private:
    struct HeaderHoverInfo {
        juce::Component::SafePointer<juce::Component> comp;
        juce::String name;
        std::function<juce::String()> getValue;
        juce::String desc;
        juce::String clickHint;
        juce::String rightClickHint;
    };

    struct HeaderHoverListener : public juce::MouseListener {
        KlangCoreEditor& owner;
        std::map<juce::Component*, HeaderHoverInfo> infoMap;

        explicit HeaderHoverListener(KlangCoreEditor& o) : owner(o) {}

        void mouseEnter(const juce::MouseEvent& e) override;
        void mouseExit(const juce::MouseEvent& e) override;
    };

    HeaderHoverListener hoverListener { *this };
};
