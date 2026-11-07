#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <map>
#include <array>
#include <functional>
#include "../ui/CrackedChimesUI.h"
#include "../ui/EditorScaleManager.h"
#include "ShoggothProcessor.h"
#include "ShoggothSpectrumCanvas.h"
#include "ShoggothEnvelopeDisplay.h"
#include "ShoggothControlInfo.h"
#include "PresetManager.h"

namespace Shoggoth {

// ==============================================================================
// 3-Way Rocker Toggle (Lifeform Triad: Synthetic, Cyborg, Organic)
// ==============================================================================
class RockerToggle3 : public juce::Component {
public:
    explicit RockerToggle3(const std::array<juce::String, 3>& labels) : labels_(labels) {}

    void setIndex(int idx, juce::NotificationType notif = juce::sendNotificationAsync) {
        idx = std::clamp(idx, 0, 2);
        if (activeIndex_ != idx) {
            activeIndex_ = idx;
            repaint();
            if (notif != juce::dontSendNotification && onChange) {
                onChange(activeIndex_);
            }
        }
    }

    int getIndex() const noexcept { return activeIndex_; }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xFF161B22));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF30363D));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        const float segW = bounds.getWidth() / 3.0f;
        auto activeRect = juce::Rectangle<float>(bounds.getX() + static_cast<float>(activeIndex_) * segW, bounds.getY(), segW, bounds.getHeight());
        g.setColour(colours_[static_cast<size_t>(activeIndex_)]);
        g.fillRoundedRectangle(activeRect.reduced(1.0f), 3.0f);

        g.setFont(juce::Font(juce::FontOptions(9.5f).withStyle("Bold")));
        for (int i = 0; i < 3; ++i) {
            bool isActive = (i == activeIndex_);
            auto segRect = juce::Rectangle<float>(bounds.getX() + static_cast<float>(i) * segW, bounds.getY(), segW, bounds.getHeight());
            g.setColour(isActive ? juce::Colour(0xFF0D1117) : juce::Colour(0xFF8B949E));
            g.drawFittedText(labels_[static_cast<size_t>(i)], segRect.toNearestInt(), juce::Justification::centred, 1, 0.85f);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        const float segW = static_cast<float>(getWidth()) / 3.0f;
        int clickedIdx = std::clamp(static_cast<int>(e.position.x / segW), 0, 2);
        setIndex(clickedIdx);
    }

    std::function<void(int)> onChange;

private:
    std::array<juce::String, 3> labels_;
    int activeIndex_{0};
    std::array<juce::Colour, 3> colours_{
        juce::Colour(0xFF00C8FF), // Synthetic (Tron Blue)
        juce::Colour(0xFFE08C00), // Cyborg (Toxic Amber)
        juce::Colour(0xFF6BBF59)  // Organic (Warm Green)
    };
};

// ==============================================================================
// AuditionMenuButton: Clean dark capsule button with vector down-chevron
// ==============================================================================
class AuditionMenuButton : public juce::TextButton {
public:
    void setAccentColour(juce::Colour c) { accentCol_ = c; repaint(); }

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.isEmpty()) return;

        juce::Colour bg = shouldDrawButtonAsDown ? juce::Colour(0xFF262D38)
                        : (shouldDrawButtonAsHighlighted ? juce::Colour(0xFF21262D) : juce::Colour(0xFF161B22));
        g.setColour(bg);
        g.fillRect(bounds);

        g.setColour(juce::Colour(0xFF30363D));
        g.drawRect(bounds, 1.0f);

        // Vector down-chevron at right edge
        const float chevronX = bounds.getRight() - 12.0f;
        const float chevronY = bounds.getCentreY();
        juce::Path p;
        p.startNewSubPath(chevronX - 3.5f, chevronY - 2.0f);
        p.lineTo(chevronX, chevronY + 2.0f);
        p.lineTo(chevronX + 3.5f, chevronY - 2.0f);
        g.setColour(accentCol_.withAlpha(0.85f));
        g.strokePath(p, juce::PathStrokeType(1.5f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));

        // Text area
        auto textRect = bounds.withTrimmedRight(18.0f).withTrimmedLeft(6.0f);
        g.setColour(shouldDrawButtonAsHighlighted ? juce::Colours::white : accentCol_.withAlpha(0.95f));
        auto font = juce::Font(juce::FontOptions(11.0f).withStyle("Bold"));
        g.setFont(font);
        g.drawFittedText(getButtonText(), textRect.toNearestInt(), juce::Justification::centred, 1);
    }

private:
    juce::Colour accentCol_{0xFF00FFCC};
};

// ==============================================================================
// StepperAuditionBar: Categorized PopupMenu + Stepper Buttons [ < ] [ Label v ] [ > ]
// ==============================================================================
class StepperAuditionBar : public juce::Component {
public:
    explicit StepperAuditionBar(ShoggothProcessor& proc, std::function<void()> onPresetChanged)
        : processor_(proc), onPresetChanged_(std::move(onPresetChanged))
    {
        prevBtn_.setButtonText("<");
        nextBtn_.setButtonText(">");
        menuBtn_.setTriggeredOnMouseDown(true);

        for (auto* b : { &prevBtn_, &nextBtn_ }) {
            b->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF161B22));
            b->setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFF21262D));
            b->setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF00FFCC));
            b->setColour(juce::TextButton::textColourOnId, juce::Colour(0xFFF0F6FC));
            addAndMakeVisible(b);
        }
        addAndMakeVisible(menuBtn_);

        prevBtn_.onClick = [this]() {
            processor_.getPresetManager().prevPreset(processor_.getAPVTS());
            updateDisplay();
            if (onPresetChanged_) onPresetChanged_();
        };

        nextBtn_.onClick = [this]() {
            processor_.getPresetManager().nextPreset(processor_.getAPVTS());
            updateDisplay();
            if (onPresetChanged_) onPresetChanged_();
        };

        menuBtn_.onClick = [this]() { showPresetMenu(); };
        updateDisplay();
    }

    void updateDisplay() {
        auto& pm = processor_.getPresetManager();
        auto name = pm.getActivePresetName();
        if (name.isEmpty()) name = "INIT PATCH";
        int idx = pm.getCurrentIndex();
        juce::String label;
        if (idx >= 0) {
            label = juce::String(idx + 1) + ": " + name;
        } else {
            label = name;
        }
        menuBtn_.setButtonText(label);
        repaint();
    }

    void resized() override {
        auto r = getLocalBounds();
        const int btnW = 22;
        prevBtn_.setBounds(r.removeFromLeft(btnW));
        r.removeFromLeft(2);
        nextBtn_.setBounds(r.removeFromRight(btnW));
        r.removeFromRight(2);
        menuBtn_.setBounds(r);
    }

    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& wheel) override {
        if (wheel.deltaY > 0.01f) {
            processor_.getPresetManager().nextPreset(processor_.getAPVTS());
            updateDisplay();
            if (onPresetChanged_) onPresetChanged_();
        } else if (wheel.deltaY < -0.01f) {
            processor_.getPresetManager().prevPreset(processor_.getAPVTS());
            updateDisplay();
            if (onPresetChanged_) onPresetChanged_();
        }
    }

    void showPresetMenu() {
        auto& pm = processor_.getPresetManager();
        auto presets = pm.getAvailablePresets();

        juce::PopupMenu m;
        std::map<juce::String, juce::Array<std::pair<int, juce::String>>> bankMap;
        for (int i = 0; i < presets.size(); ++i) {
            juce::String bankName = presets[i].bank.isNotEmpty() ? presets[i].bank : "General";
            bankMap[bankName].add({ i, presets[i].name });
        }

        int itemBase = 100;
        std::map<int, int> menuIdToPresetIndex;

        for (const auto& [bankName, patchList] : bankMap) {
            juce::PopupMenu sub;
            for (const auto& [origIdx, pName] : patchList) {
                int menuId = itemBase++;
                menuIdToPresetIndex[menuId] = origIdx;
                bool isTicked = (origIdx == pm.getCurrentIndex());
                sub.addItem(menuId, pName, true, isTicked);
            }
            m.addSubMenu(bankName, sub);
        }

        m.addSeparator();
        m.addItem(1, "Save Preset As...");

        m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&menuBtn_).withStandardItemHeight(24),
            [this, &pm, menuIdToPresetIndex](int res) {
                if (res >= 100) {
                    auto it = menuIdToPresetIndex.find(res);
                    if (it != menuIdToPresetIndex.end()) {
                        pm.loadPresetByIndex(it->second, processor_.getAPVTS());
                        updateDisplay();
                        if (onPresetChanged_) onPresetChanged_();
                    }
                } else if (res == 1) {
                    auto userDir = pm.getUserPresetsDirectory();
                    auto file = userDir.getChildFile(pm.getActivePresetName() + ".json");
                    pm.savePreset(file, processor_.getAPVTS(), pm.getActivePresetName());
                    updateDisplay();
                }
            });
    }

private:
    ShoggothProcessor& processor_;
    std::function<void()> onPresetChanged_;
    juce::TextButton prevBtn_;
    AuditionMenuButton menuBtn_;
    juce::TextButton nextBtn_;
};

// ==============================================================================
// HeaderInspector: Universal Header Telemetry Readout
// ==============================================================================
class HeaderInspector : public juce::Component {
public:
    struct Readout {
        juce::String name{"SHOGGOTH"};
        juce::String value{"READY"};
        juce::String blurb{"Multidimensional clean-room additive synthesizer (AVX2 complex rotators)"};
        juce::Colour accent{0xFF00FFCC};
        bool isDefault{true};
    };

    HeaderInspector() {
        setInterceptsMouseClicks(false, false);
        setReadoutDefault();
    }

    void setReadout(Readout r) {
        readout_ = std::move(r);
        repaint();
    }

    void setReadoutDefault() {
        Readout d;
        d.name = "SHOGGOTH";
        d.value = "READY";
        d.blurb = "Multidimensional clean-room additive synthesizer (AVX2 complex rotators)";
        d.accent = juce::Colour(0xFF00FFCC);
        d.isDefault = true;
        setReadout(d);
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xFF0F131A));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF262D38));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        auto nameRect = juce::Rectangle<float>(bounds.getX() + 8.0f, bounds.getY(), 95.0f, bounds.getHeight());
        g.setFont(juce::Font(juce::FontOptions(11.5f).withStyle("Bold")));
        g.setColour(readout_.accent);
        g.drawText(readout_.name, nameRect, juce::Justification::centredLeft, true);

        auto valueRect = juce::Rectangle<float>(bounds.getX() + 105.0f, bounds.getY(), 85.0f, bounds.getHeight());
        g.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
        g.setColour(juce::Colour(0xFFF0F6FC));
        g.drawText(readout_.value, valueRect, juce::Justification::centredLeft, true);

        float blurbW = std::max(40.0f, bounds.getWidth() - 195.0f);
        auto blurbRect = juce::Rectangle<float>(bounds.getX() + 192.0f, bounds.getY(), blurbW, bounds.getHeight());
        g.setFont(juce::Font(juce::FontOptions(11.0f)));
        g.setColour(readout_.isDefault ? juce::Colour(0xFF6E7681) : juce::Colour(0xFFC9D1D9));
        g.drawFittedText(readout_.blurb, blurbRect.toNearestInt(), juce::Justification::centredLeft, 1);
    }

private:
    Readout readout_;
};

// ==============================================================================
// ShoggothLookAndFeel: Modern Cracked Chimes Fleet Standard via CrackedChimesUI
// ==============================================================================
class ShoggothLookAndFeel : public CrackedChimes::UI::CrackedChimesLookAndFeel {
public:
    ShoggothLookAndFeel() {
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xFF0D1117));
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xFF0D1117));
        setColour(juce::PopupMenu::textColourId, juce::Colour(0xFFF0F6FC));
        setColour(juce::PopupMenu::headerTextColourId, juce::Colour(0xFF8B949E));
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xFF21262D));
        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
        accentColour_ = juce::Colour(0xFF00FFCC);
        modRingDefaultColour_ = juce::Colour(0xFFFF7B25); // Movie Poster Orange
        randRingDefaultColour_ = juce::Colour(0xFFF2E10F); // Caustic Yellow
    }

    void setAccentColour(juce::Colour c) { accentColour_ = c; }
};

// ==============================================================================
// ShoggothEditor: Master UI Component
// ==============================================================================
class ShoggothEditor : public juce::AudioProcessorEditor,
                       public juce::FileDragAndDropTarget,
                       public juce::Timer {
public:
    explicit ShoggothEditor(ShoggothProcessor&);
    ~ShoggothEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    // FileDragAndDropTarget
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    struct RotaryKnob {
        juce::String paramId;
        juce::Label label;
        juce::Slider slider;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

private:
    ShoggothProcessor& audioProcessor;
    ShoggothLookAndFeel lnf_;

    // Master Header Strip
    juce::Label titleLabel;
    juce::Label subtitleLabel;
    StepperAuditionBar presetSelector_;
    RockerToggle3 lifeModeToggle_;
    HeaderInspector headerInspector_;

    // Visualizers
    ShoggothSpectrumCanvas spectrumCanvas_;
    ShoggothEnvelopeDisplay envelopeDisplay_;

    // 32 Rotary Knobs
    std::vector<std::unique_ptr<RotaryKnob>> knobs_;

    bool isDraggingFile_{false};

    void bindControls();
    void setupKnob(RotaryKnob& item, const juce::String& paramId, const juce::String& label, juce::Colour col);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShoggothEditor)
};

} // namespace Shoggoth
