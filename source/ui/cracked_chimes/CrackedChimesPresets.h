#pragma once
// ==============================================================================
// CrackedChimesPresets.h - shared fleet preset browser (CCUI Wave 2-P)
// [ < ] [ Bank / Patch v ] [ > ]  - 168x28 baseline, scales with bounds.
// Depends only on RlyehSound::PresetManager + APVTS (no processor types).
// No String::formatted; ASCII-only UI strings.
// ==============================================================================

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <functional>
#include <map>
#include <memory>
#include "PresetManager.h"

namespace CrackedChimes {

class StepperAuditionBar : public juce::Component {
public:
    static constexpr int kBaseWidth  = 168;
    static constexpr int kBaseHeight = 28;

    StepperAuditionBar(RlyehSound::PresetManager& pm,
                       juce::AudioProcessorValueTreeState& apvts,
                       std::function<void()> onPresetChanged = {})
        : pm_(pm), apvts_(apvts), onPresetChanged_(std::move(onPresetChanged))
    {
        prevBtn_.setButtonText("<");
        nextBtn_.setButtonText(">");
        menuBtn_.setTriggeredOnMouseDown(true);
        for (auto* b : { &prevBtn_, &nextBtn_, &menuBtn_ }) {
            b->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF161B22));
            b->setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFF21262D));
            b->setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF00FFCC));
            b->setColour(juce::TextButton::textColourOnId, juce::Colour(0xFFF0F6FC));
            addAndMakeVisible(b);
        }
        prevBtn_.onClick = [this] { step(-1); };
        nextBtn_.onClick = [this] { step(+1); };
        menuBtn_.onClick = [this] { showPresetMenu(); };
        setSize(kBaseWidth, kBaseHeight);
        updateDisplay();
    }

    void step(int dir) {
        if (dir > 0) pm_.nextPreset(apvts_); else pm_.prevPreset(apvts_);
        changed();
    }

    void updateDisplay() {
        juce::String name = pm_.getActivePresetName();
        if (name.isEmpty()) name = "Init";
        juce::String label = name;
        const int idx = pm_.getCurrentIndex();
        if (idx >= 0 && idx < pm_.getNumPresets()) {
            auto list = pm_.getAvailablePresets();
            const auto bank = list[idx].bank;
            if (bank.isNotEmpty() && ! bank.equalsIgnoreCase("General"))
                label = bank + " / " + name;
        }
        menuBtn_.setButtonText(label + " v");
        menuBtn_.setTooltip(label);
        repaint();
    }

    void resized() override {
        auto r = getLocalBounds();
        const int btnW = juce::jmax(18, juce::roundToInt(r.getHeight() * 0.8f));
        prevBtn_.setBounds(r.removeFromLeft(btnW));
        r.removeFromLeft(2);
        nextBtn_.setBounds(r.removeFromRight(btnW));
        r.removeFromRight(2);
        menuBtn_.setBounds(r);
    }

    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& wheel) override {
        if (wheel.deltaY > 0.01f) step(+1);
        else if (wheel.deltaY < -0.01f) step(-1);
    }

    void showPresetMenu() {
        auto presets = pm_.getAvailablePresets();
        std::map<juce::String, juce::Array<int>> bankMap;
        for (int i = 0; i < presets.size(); ++i) {
            auto bank = presets[i].bank.isNotEmpty() ? presets[i].bank : juce::String("General");
            bankMap[bank].add(i);
        }

        constexpr int kInitId = 1, kSaveId = 2, kBrowseId = 3, kPresetBase = 100;
        juce::PopupMenu m;
        m.addItem(kBrowseId, "Open Preset Vault...");
        m.addSeparator();
        m.addItem(kInitId, "Init", true, pm_.getCurrentIndex() < 0
                                         && pm_.getActivePresetName() == "Init");
        m.addSeparator();
        for (const auto& [bank, idxs] : bankMap) {
            juce::PopupMenu sub;
            for (int i : idxs)
                sub.addItem(kPresetBase + i, presets[i].name, true, i == pm_.getCurrentIndex());
            if (bankMap.size() == 1) { m.addSectionHeader(bank); m = mergeInto(m, sub); }
            else m.addSubMenu(bank, sub);
        }
        m.addSeparator();
        m.addItem(kSaveId, "Save As...");

        const int maxRows = juce::jmax(12, (getTopLevelComponent() != nullptr
                                             ? getTopLevelComponent()->getHeight() : 600) / 24);
        auto opts = juce::PopupMenu::Options().withTargetComponent(&menuBtn_)
                                              .withMaximumNumColumns(6)
                                              .withMinimumNumColumns(1);
        (void) maxRows; // JUCE auto-columns long lists to screen height
        juce::Component::SafePointer<StepperAuditionBar> safe(this);
        m.showMenuAsync(opts, [safe](int result) {
            if (safe == nullptr || result == 0) return;
            if (result == kBrowseId) { if (safe->onOpenBrowser) safe->onOpenBrowser(); }
            else if (result == kInitId) { safe->pm_.loadInit(safe->apvts_); safe->changed(); }
            else if (result == kSaveId) safe->showSaveAs();
            else if (result >= kPresetBase) {
                safe->pm_.loadPresetByIndex(result - kPresetBase, safe->apvts_);
                safe->changed();
            }
        });
    }

    void showSaveAs() {
        auto dir = pm_.getUserPresetsDirectory();
        dir.createDirectory();
        chooser_ = std::make_unique<juce::FileChooser>("Save Preset As...",
                       dir.getChildFile(pm_.getActivePresetName() + ".json"), "*.json");
        juce::Component::SafePointer<StepperAuditionBar> safe(this);
        chooser_->launchAsync(juce::FileBrowserComponent::saveMode
                              | juce::FileBrowserComponent::canSelectFiles
                              | juce::FileBrowserComponent::warnAboutOverwriting,
            [safe](const juce::FileChooser& fc) {
                if (safe == nullptr) return;
                auto f = fc.getResult();
                if (f == juce::File()) return;
                f = f.withFileExtension("json");
                if (safe->pm_.savePreset(f, safe->apvts_, f.getFileNameWithoutExtension(),
                                         "User", "User"))
                    safe->changed();
            });
    }

    void setOnOpenBrowser(std::function<void()> cb) { onOpenBrowser = std::move(cb); }
    std::function<void()> onOpenBrowser;

private:
    static juce::PopupMenu mergeInto(juce::PopupMenu dst, const juce::PopupMenu& src) {
        for (juce::PopupMenu::MenuItemIterator it(src); it.next();)
            dst.addItem(it.getItem());
        return dst;
    }

    void changed() {
        updateDisplay();
        if (onPresetChanged_) onPresetChanged_();
    }

    RlyehSound::PresetManager& pm_;
    juce::AudioProcessorValueTreeState& apvts_;
    std::function<void()> onPresetChanged_;
    juce::TextButton prevBtn_, nextBtn_, menuBtn_;
    std::unique_ptr<juce::FileChooser> chooser_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StepperAuditionBar)
};

// Compat shim: legacy editors that used a PresetStubComponent compile unchanged
// until migrated to StepperAuditionBar.
using PresetStubComponent = StepperAuditionBar;

} // namespace CrackedChimes
