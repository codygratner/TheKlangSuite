#include "KlangCoreEditor.h"
#include "ParameterManager.h"

KlangCoreEditor::KlangCoreEditor(KlangCoreProcessor& p)
    : AudioProcessorEditor(p), coreProcessor(p)
{
    initButton.setButtonText(RlyehSound::ParameterManager::getInstance().getGlobalString("btn_init", "INIT"));
    initButton.setName("INIT");
    triggerButton.setButtonText(RlyehSound::ParameterManager::getInstance().getGlobalString("btn_trigger", "TRIGGER"));
    triggerButton.setName("TRIGGER");
    tooltipsButton.setButtonText(RlyehSound::ParameterManager::getInstance().getGlobalString("btn_tooltips_off", "TIPS: OFF"));
    tooltipsButton.setName("TIPS");
    guideButton.setButtonText("GUIDE");
    guideButton.setName("GUIDE");
    setLookAndFeel(&knobLookAndFeel);

    // Default tooltips window
    tooltipWindow = std::make_unique<juce::TooltipWindow>(this, 700);
    tooltipWindow->setOpaque(false);
    tooltipWindow->setMillisecondsBeforeTipAppears(400);

    // Settings Modal & Button
    settingsModal = std::make_unique<SettingsModalComponent>(coreProcessor.getName());
    addChildComponent(*settingsModal);

    settingsButton.onClick = [this] {
        if (settingsModal) {
            settingsModal->setVisible(true);
            settingsModal->toFront(true);
        }
    };
    addAndMakeVisible(settingsButton);

    // Update Badge Button (hidden until an update is discovered)
    addChildComponent(updateBadgeButton);

    // Setup buttons
    tooltipsButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff181a20));
    tooltipsButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff8b99a6));
    tooltipsButton.onClick = [this] {
        setTooltipsEnabled(!tooltipsEnabled);
    };
    addAndMakeVisible(tooltipsButton);

    initButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff181a20));
    initButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff8b99a6));
    addAndMakeVisible(initButton);

    presetButton.setButtonText("PRESET");
    presetButton.setName("PRESET");
    presetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff181a20));
    presetButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff8b99a6));
    addAndMakeVisible(presetButton);

    triggerButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff181a20));
    triggerButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff8b99a6));
    addAndMakeVisible(triggerButton);

    addAndMakeVisible(statusBar);

    // Wire universal hover guidance feeds for shared header buttons
    wireHeaderHover(presetButton, "PRESET", "Preset Manager",
                    "Browse, load, and save universal .json / .tkp presets.",
                    "Click: Open Menu");
    wireHeaderHover(initButton, "INIT", "Patch Reset",
                    "Emergency panic audio flush & restore factory default parameters.",
                    "Click: Reset");
    wireHeaderHover(triggerButton, "TRIGGER", "Audition Hit",
                    "Fire manual audition drum strike at full 1.0 velocity.",
                    "Click: Trigger");
    wireHeaderHover(tooltipsButton, "TOOLTIPS",
                    [this] { return tooltipsEnabled ? "ON" : "OFF"; },
                    "Toggle live dynamic two-line parameter guidance feed.",
                    "Click: Toggle");
    wireHeaderHover(settingsButton, "SETTINGS", "Configuration",
                    "Check for GitHub updates and view system telemetry.",
                    "Click: Open");
    wireHeaderHover(guideButton, "QUICKSTART", "Interactive Guide",
                    "Display modal walkthrough and workflow shortcuts.",
                    "Click: Open");

    // Trigger update check on launch if enabled
    if (VersionChecker::isCheckOnLaunchEnabled())
        VersionChecker::getInstance().checkForUpdates(false);
}

KlangCoreEditor::~KlangCoreEditor()
{
    for (auto& pair : hoverListener.infoMap) {
        if (pair.second.comp != nullptr) pair.second.comp->removeMouseListener(&hoverListener);
    }
    hoverListener.infoMap.clear();
}

void KlangCoreEditor::unwireHeaderHover(juce::Component& comp)
{
    comp.removeMouseListener(&hoverListener);
    hoverListener.infoMap.erase(&comp);
}

void KlangCoreEditor::wireHeaderHover(juce::Component& comp,
                                      const juce::String& name,
                                      std::function<juce::String()> getValue,
                                      const juce::String& desc,
                                      const juce::String& clickHint,
                                      const juce::String& rightClickHint)
{
    hoverListener.infoMap[&comp] = { &comp, name, std::move(getValue), desc, clickHint, rightClickHint };
    comp.addMouseListener(&hoverListener, false);
}

void KlangCoreEditor::wireHeaderHover(juce::Component& comp,
                                      const juce::String& name,
                                      const juce::String& value,
                                      const juce::String& desc,
                                      const juce::String& clickHint,
                                      const juce::String& rightClickHint)
{
    wireHeaderHover(comp, name, [value] { return value; }, desc, clickHint, rightClickHint);
}

void KlangCoreEditor::simulateHeaderHover(juce::Component* comp)
{
    if (!comp) return;
    auto it = hoverListener.infoMap.find(comp);
    if (it != hoverListener.infoMap.end()) {
        auto val = it->second.getValue ? it->second.getValue() : juce::String();
        statusBar.setHoveredControl(it->second.name, val, it->second.desc,
                                    it->second.rightClickHint, it->second.clickHint);
    }
}

void KlangCoreEditor::simulateHeaderExit(juce::Component* comp)
{
    if (!comp) return;
    if (hoverListener.infoMap.find(comp) != hoverListener.infoMap.end()) {
        statusBar.clearHoveredControl();
    }
}

void KlangCoreEditor::HeaderHoverListener::mouseEnter(const juce::MouseEvent& e)
{
    auto it = infoMap.find(e.eventComponent);
    if (it != infoMap.end()) {
        auto val = it->second.getValue ? it->second.getValue() : juce::String();
        owner.getStatusBar().setHoveredControl(it->second.name, val, it->second.desc,
                                              it->second.rightClickHint, it->second.clickHint);
    }
}

void KlangCoreEditor::HeaderHoverListener::mouseExit(const juce::MouseEvent& e)
{
    if (infoMap.find(e.eventComponent) != infoMap.end()) {
        owner.getStatusBar().clearHoveredControl();
    }
}

void KlangCoreEditor::setTooltipsEnabled(bool enabled)
{
    tooltipsEnabled = enabled;
    statusBar.setTooltipsEnabled(enabled);
    if (enabled) {
        tooltipsButton.setButtonText(RlyehSound::ParameterManager::getInstance().getGlobalString("btn_tooltips_on", "TIPS: ON"));
        tooltipsButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff33ccff));
        tooltipWindow->setMillisecondsBeforeTipAppears(400);
    } else {
        tooltipsButton.setButtonText(RlyehSound::ParameterManager::getInstance().getGlobalString("btn_tooltips_off", "TIPS: OFF"));
        tooltipsButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff8b99a6));
        tooltipWindow->setMillisecondsBeforeTipAppears(9999999);
        tooltipWindow->hideTip();
    }
}
