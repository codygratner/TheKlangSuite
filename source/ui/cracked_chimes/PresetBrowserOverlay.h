#pragma once
// ==============================================================================
// PresetBrowserOverlay.h - Full In-Editor Modal Preset Vault & Smart Dice
// 680x440 modal card over dark scrim with search, category filtering,
// zero-alloc ListBox, section locks, Smart Dice mutation, and 16-step undo.
// ==============================================================================

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <functional>
#include <algorithm>
#include "PresetManager.h"
#include "CrackedChimesColours.h"

namespace CrackedChimes {

class PresetBrowserOverlay : public juce::Component,
                             public juce::ListBoxModel,
                             public juce::TextEditor::Listener
{
public:
    PresetBrowserOverlay(RlyehSound::PresetManager& pm,
                         juce::AudioProcessorValueTreeState& apvts,
                         juce::Colour accentColour = juce::Colour(0xFF00C8FF))
        : pm_(pm), apvts_(apvts), accentColour_(accentColour)
    {
        setInterceptsMouseClicks(true, true);

        // Snapshot initial preset name for Esc-revert
        initialPresetName_ = pm_.getActivePresetName();
        initialIndex_ = pm_.getCurrentIndex();

        // 1. Search Box
        searchEditor_.setTextToShowWhenEmpty("Search presets or tags...", juce::Colour(0xFF8B949E));
        searchEditor_.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xFF0D1117));
        searchEditor_.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xFF30363D));
        searchEditor_.setColour(juce::TextEditor::focusedOutlineColourId, accentColour_);
        searchEditor_.setColour(juce::TextEditor::textColourId, juce::Colour(0xFFF0F6FC));
        searchEditor_.addListener(this);
        addAndMakeVisible(searchEditor_);

        // 2. Preset ListBox
        listBox_.setModel(this);
        listBox_.setRowHeight(28);
        listBox_.setColour(juce::ListBox::backgroundColourId, juce::Colour(0xFF0D1117));
        listBox_.setColour(juce::ListBox::outlineColourId, juce::Colour(0xFF30363D));
        addAndMakeVisible(listBox_);

        // 3. Category Buttons
        setupCategoryButton(catAllBtn_, "ALL");
        setupCategoryButton(catFactoryBtn_, "FACTORY");
        setupCategoryButton(catUserBtn_, "USER");

        catAllBtn_.onClick = [this] { currentCategory_ = "ALL"; updateFilteredList(); };
        catFactoryBtn_.onClick = [this] { currentCategory_ = "FACTORY"; updateFilteredList(); };
        catUserBtn_.onClick = [this] { currentCategory_ = "USER"; updateFilteredList(); };

        // 4. Section Locks
        setupLockToggle(lockOscBtn_, "OSC", RlyehSound::LockOscCore);
        setupLockToggle(lockVcfBtn_, "VCF", RlyehSound::LockFilterVCF);
        setupLockToggle(lockEnvBtn_, "ENV", RlyehSound::LockEnvelopes);
        setupLockToggle(lockFxBtn_, "FX", RlyehSound::LockMasterFX);

        // 5. Smart Dice & Undo
        diceBtn_.setButtonText(juce::CharPointer_UTF8("\xf0\x9f\x8e\xb2 DICE")); // Die emoji / DICE
        diceBtn_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF238636));
        diceBtn_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFFFFFFFF));
        diceBtn_.onClick = [this] {
            pm_.mutateSmartDice(apvts_);
            listBox_.repaint();
        };
        addAndMakeVisible(diceBtn_);

        undoBtn_.setButtonText("UNDO");
        undoBtn_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF21262D));
        undoBtn_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF8B949E));
        undoBtn_.onClick = [this] {
            if (pm_.canUndoDice()) {
                pm_.undoDice(apvts_);
                listBox_.repaint();
            }
        };
        addAndMakeVisible(undoBtn_);

        // 6. Close / Commit Buttons
        closeButton_.setButtonText("Close");
        closeButton_.onClick = [this] { if (onClose) onClose(); };
        addAndMakeVisible(closeButton_);

        xButton_.setButtonText(juce::CharPointer_UTF8("\xc3\x97"));
        xButton_.onClick = [this] { if (onClose) onClose(); };
        addAndMakeVisible(xButton_);

        updateFilteredList();
    }

    void setOnClose(std::function<void()> cb) {
        onClose = std::move(cb);
    }

    bool keyPressed(const juce::KeyPress& key) override {
        if (key == juce::KeyPress::escapeKey) {
            // Revert on Esc
            if (initialIndex_ >= 0) {
                pm_.loadPresetByIndex(initialIndex_, apvts_);
            }
            if (onClose) onClose();
            return true;
        }
        if (key == juce::KeyPress::returnKey) {
            // Commit and close on Enter
            if (onClose) onClose();
            return true;
        }
        return false;
    }

    void textEditorTextChanged(juce::TextEditor&) override {
        updateFilteredList();
    }

    void updateFilteredList() {
        allPresets_ = pm_.getAvailablePresets();
        filteredIndices_.clear();

        juce::String filter = searchEditor_.getText().trim().toLowerCase();

        for (int i = 0; i < allPresets_.size(); ++i) {
            const auto& p = allPresets_[i];

            // Category filter
            if (currentCategory_ == "FACTORY" && p.bank.equalsIgnoreCase("User")) continue;
            if (currentCategory_ == "USER" && !p.bank.equalsIgnoreCase("User")) continue;

            // Search filter
            if (filter.isNotEmpty()) {
                bool match = p.name.toLowerCase().contains(filter) ||
                             p.category.toLowerCase().contains(filter) ||
                             p.bank.toLowerCase().contains(filter);
                if (!match) {
                    for (const auto& t : p.tags) {
                        if (t.toLowerCase().contains(filter)) { match = true; break; }
                    }
                }
                if (!match) continue;
            }

            filteredIndices_.add(i);
        }

        listBox_.updateContent();
        listBox_.repaint();
    }

    // ListBoxModel
    int getNumRows() override {
        return filteredIndices_.size();
    }

    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override {
        if (rowNumber < 0 || rowNumber >= filteredIndices_.size()) return;
        int presetIdx = filteredIndices_[rowNumber];
        const auto& p = allPresets_[presetIdx];

        bool isActive = (presetIdx == pm_.getCurrentIndex());

        if (rowIsSelected) {
            g.setColour(juce::Colour(0xFF21262D));
            g.fillAll();
        } else if (rowNumber % 2 == 1) {
            g.setColour(juce::Colour(0xFF11141A));
            g.fillAll();
        }

        auto r = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(8.0f, 0.0f);

        // Active indicator pip
        if (isActive) {
            g.setColour(accentColour_);
            g.fillEllipse(r.getX(), r.getCentreY() - 3.0f, 6.0f, 6.0f);
        }
        r.removeFromLeft(12.0f);

        // Preset Name
        g.setFont(juce::Font(juce::FontOptions(13.0f).withStyle(isActive ? "Bold" : "Plain")));
        g.setColour(isActive ? accentColour_ : juce::Colour(0xFFF0F6FC));
        g.drawText(p.name, r.removeFromLeft(width * 0.45f), juce::Justification::centredLeft, true);

        // Bank / Category
        g.setFont(juce::Font(juce::FontOptions(11.0f)));
        g.setColour(juce::Colour(0xFF8B949E));
        g.drawText(p.bank + " / " + p.category, r.removeFromLeft(width * 0.35f), juce::Justification::centredLeft, true);

        // Author
        g.setColour(juce::Colour(0xFF6E7681));
        g.drawText(p.author, r, juce::Justification::centredRight, true);
    }

    void selectedRowsChanged(int lastRowSelected) override {
        if (lastRowSelected >= 0 && lastRowSelected < filteredIndices_.size()) {
            int targetIdx = filteredIndices_[lastRowSelected];
            pm_.loadPresetByIndex(targetIdx, apvts_);
            listBox_.repaint();
        }
    }

    void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override {
        if (row >= 0 && row < filteredIndices_.size()) {
            int targetIdx = filteredIndices_[row];
            pm_.loadPresetByIndex(targetIdx, apvts_);
            if (onClose) onClose();
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        auto card = getCardBounds();
        if (!card.contains(e.getPosition().toFloat())) {
            if (onClose) onClose();
        }
    }

    void resized() override {
        auto card = getCardBounds();

        // Close 'X'
        xButton_.setBounds(static_cast<int>(card.getRight() - 32.0f),
                           static_cast<int>(card.getY() + 10.0f), 22, 22);

        // Top Search Bar
        searchEditor_.setBounds(static_cast<int>(card.getX() + 170.0f),
                                static_cast<int>(card.getY() + 16.0f),
                                static_cast<int>(card.getWidth() - 215.0f), 28);

        // Left Categories
        int catY = static_cast<int>(card.getY() + 60.0f);
        catAllBtn_.setBounds(static_cast<int>(card.getX() + 20.0f), catY, 130, 26);
        catFactoryBtn_.setBounds(static_cast<int>(card.getX() + 20.0f), catY + 32, 130, 26);
        catUserBtn_.setBounds(static_cast<int>(card.getX() + 20.0f), catY + 64, 130, 26);

        // ListBox
        listBox_.setBounds(static_cast<int>(card.getX() + 170.0f),
                           static_cast<int>(card.getY() + 60.0f),
                           static_cast<int>(card.getWidth() - 190.0f),
                           static_cast<int>(card.getHeight() - 125.0f));

        // Bottom Bar
        int botY = static_cast<int>(card.getBottom() - 44.0f);
        int lockX = static_cast<int>(card.getX() + 20.0f);
        lockOscBtn_.setBounds(lockX, botY, 48, 26);
        lockVcfBtn_.setBounds(lockX + 52, botY, 48, 26);
        lockEnvBtn_.setBounds(lockX + 104, botY, 48, 26);
        lockFxBtn_.setBounds(lockX + 156, botY, 48, 26);

        diceBtn_.setBounds(static_cast<int>(card.getX() + 220.0f), botY, 80, 26);
        undoBtn_.setBounds(static_cast<int>(card.getX() + 306.0f), botY, 65, 26);

        closeButton_.setBounds(static_cast<int>(card.getRight() - 100.0f), botY, 80, 26);
    }

    void paint(juce::Graphics& g) override {
        // 1. Scrim
        g.setColour(juce::Colour(0xDD0B0E14));
        g.fillAll();

        // 2. Chassis Card
        auto card = getCardBounds();
        g.setColour(juce::Colour(0xFF161B22));
        g.fillRoundedRectangle(card, 8.0f);
        g.setColour(accentColour_.withAlpha(0.6f));
        g.drawRoundedRectangle(card, 8.0f, 1.5f);

        // 3. Title
        g.setFont(juce::Font(juce::FontOptions(16.0f).withStyle("Bold")));
        g.setColour(accentColour_);
        auto titleArea = juce::Rectangle<float>(card.getX() + 20.0f, card.getY() + 16.0f, 180.0f, 28.0f);
        g.drawText(pm_.getTargetPlugin().toUpperCase() + " PRESET VAULT", titleArea, juce::Justification::centredLeft);

        // Sidebar separator
        g.setColour(juce::Colour(0xFF30363D));
        g.drawVerticalLine(static_cast<int>(card.getX() + 160.0f), static_cast<float>(card.getY() + 55.0f), static_cast<float>(card.getBottom() - 55.0f));

        // Bottom separator
        g.drawHorizontalLine(static_cast<int>(card.getBottom() - 55.0f), static_cast<float>(card.getX() + 16.0f), static_cast<float>(card.getRight() - 16.0f));
    }

    std::function<void()> onClose;

private:
    juce::Rectangle<float> getCardBounds() const {
        float w = juce::jmin(680.0f, static_cast<float>(getWidth()) - 32.0f);
        float h = juce::jmin(440.0f, static_cast<float>(getHeight()) - 32.0f);
        float x = (static_cast<float>(getWidth()) - w) * 0.5f;
        float y = (static_cast<float>(getHeight()) - h) * 0.5f;
        return {x, y, w, h};
    }

    void setupCategoryButton(juce::TextButton& btn, const juce::String& text) {
        btn.setButtonText(text);
        btn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF161B22));
        btn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF8B949E));
        addAndMakeVisible(btn);
    }

    void setupLockToggle(juce::ToggleButton& btn, const juce::String& text, uint32_t flag) {
        btn.setButtonText(text);
        btn.setToggleState(pm_.isSectionLocked(flag), juce::dontSendNotification);
        btn.onClick = [this, &btn, flag] {
            pm_.toggleSectionLock(flag);
            btn.setToggleState(pm_.isSectionLocked(flag), juce::dontSendNotification);
        };
        addAndMakeVisible(btn);
    }

    RlyehSound::PresetManager& pm_;
    juce::AudioProcessorValueTreeState& apvts_;
    juce::Colour accentColour_;

    juce::String initialPresetName_;
    int initialIndex_{-1};

    juce::TextEditor searchEditor_;
    juce::ListBox listBox_;

    juce::String currentCategory_{"ALL"};
    juce::TextButton catAllBtn_;
    juce::TextButton catFactoryBtn_;
    juce::TextButton catUserBtn_;

    juce::ToggleButton lockOscBtn_;
    juce::ToggleButton lockVcfBtn_;
    juce::ToggleButton lockEnvBtn_;
    juce::ToggleButton lockFxBtn_;

    juce::TextButton diceBtn_;
    juce::TextButton undoBtn_;
    juce::TextButton closeButton_;
    juce::TextButton xButton_;

    juce::Array<RlyehSound::PresetMetadataInfo> allPresets_;
    juce::Array<int> filteredIndices_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowserOverlay)
};

} // namespace CrackedChimes
