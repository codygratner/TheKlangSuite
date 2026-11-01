#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "KlangCoreEditor.h"
#include "PlanterProcessor.h"
#include "UIComponents.h"
#include "PlanterEditor.h"

// ==============================================================================
// 1. ShardRotarySlider
// Clean rotary slider delegating paint directly to RotaryKnobLookAndFeel
// to render unipolar (6pm -> Noon -> 6pm) and bipolar (Noon bidirectional) arcs.
// ==============================================================================
class ShardRotarySlider : public RotaryKnobSlider {
public:
    ShardRotarySlider();
    void paint(juce::Graphics& g) override;
};

// ==============================================================================
// 2. ShardKnobCell
// Unified control cell: Label above, ShardRotarySlider in middle, Readout below.
// ==============================================================================
class ShardKnobCell : public juce::Component {
public:
    ShardKnobCell(const juce::String& title,
                  juce::Colour accent,
                  bool bipolar,
                  double defaultVal,
                  std::function<juce::String(double)> formatFn = nullptr,
                  std::function<double(const juce::String&)> parseFn = nullptr);

    void updateReadout();
    void resized() override;

    ShardRotarySlider& getSlider() { return slider; }
    juce::Label& getNameLabel() { return nameLabel; }
    juce::Label& getReadoutLabel() { return readoutLabel; }

private:
    juce::Label nameLabel;
    ShardRotarySlider slider;
    juce::Label readoutLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShardKnobCell)
};

// ==============================================================================
// 3. ShardCard
// Brutalist hardware device card with colored top stripe and uppercase badge.
// ==============================================================================
class ShardCard : public juce::Component {
public:
    ShardCard(const juce::String& numberAndTitle, const juce::String& typeBadge, juce::Colour accent);
    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getBodyBounds() const;

private:
    juce::String title;
    juce::String badge;
    juce::Colour accentColour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShardCard)
};

// ==============================================================================
// 4. ShardContentComponent
// Fixed-dimension (1100 x 380) faceplate container that scales with aspect ratio.
// ==============================================================================
class ShardContentComponent : public juce::Component {
public:
    ShardContentComponent() = default;
    void paint(juce::Graphics& g) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShardContentComponent)
};

// ==============================================================================
// 5. TheShardAudioProcessorEditor
// Authentic brutalist 5-card single-window device chain editor for The Shard.
// ==============================================================================
class TheShardAudioProcessorEditor : public KlangCoreEditor, public juce::Timer {
public:
    explicit TheShardAudioProcessorEditor(TheKlangPlanterAudioProcessor&);
    ~TheShardAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    TheKlangPlanterAudioProcessor& audioProcessor;
    juce::ComponentBoundsConstrainer constrainer;
    ShardContentComponent content;

    PlanterHeaderVisualizer headerViz;
    juce::TextButton panicButton { "PANIC" };

    // 5 Device Cards
    std::unique_ptr<ShardCard> cardGen;
    std::unique_ptr<ShardCard> cardFilter;
    std::unique_ptr<ShardCard> cardTrans;
    std::unique_ptr<ShardCard> cardQuad;
    std::unique_ptr<ShardCard> cardAmp;

    // Selectors
    juce::ComboBox carrierTrackingBox;
    LedSelectorComponent carrierTrackingSelector;
    juce::ComboBox modTrackBox;
    LedSelectorComponent modTrackSelector;
    juce::ComboBox modTypeBox;
    LedSelectorComponent modTypeSelector;
    juce::ComboBox filterTypeBox;
    LedSelectorComponent filterTypeSelector;
    juce::ComboBox filterSlopeBox;
    LedSelectorComponent filterSlopeSelector;
    juce::ComboBox pitchEnvTargetBox;
    LedSelectorComponent pitchEnvTargetSelector;
    juce::ComboBox limiterEnableBox;
    LedSelectorComponent limiterEnableSelector;

    // Card 1: Generator Knob Cells
    std::unique_ptr<ShardKnobCell> carrierPitchKnob;
    std::unique_ptr<ShardKnobCell> carrierShapeKnob;
    std::unique_ptr<ShardKnobCell> carrierDepthKnob;
    std::unique_ptr<ShardKnobCell> modShapeKnob;
    std::unique_ptr<ShardKnobCell> modSpeedKnob;

    // Card 2: Filter Knob Cells
    std::unique_ptr<ShardKnobCell> filterCutoffKnob;
    std::unique_ptr<ShardKnobCell> filterResoKnob;
    std::unique_ptr<ShardKnobCell> filterDriveKnob;
    std::unique_ptr<ShardKnobCell> filterEnvDepthKnob;
    std::unique_ptr<ShardKnobCell> filterEnvDecayKnob;
    std::unique_ptr<ShardKnobCell> filterEnvSlopeKnob;

    // Card 3: Transient & Mod Knob Cells
    std::unique_ptr<ShardKnobCell> noiseShRateKnob;
    std::unique_ptr<ShardKnobCell> noiseFilterKnob;
    std::unique_ptr<ShardKnobCell> noiseDecayKnob;
    std::unique_ptr<ShardKnobCell> noiseCrossfadeKnob;

    // Card 4: QuadraMod Knob Cells
    std::unique_ptr<ShardKnobCell> pitchEnvDepthKnob;
    std::unique_ptr<ShardKnobCell> pitchEnvDecayKnob;
    std::unique_ptr<ShardKnobCell> pitchEnvSlopeKnob;

    // Card 5: Amp & Master Knob Cells
    std::unique_ptr<ShardKnobCell> ampDriveKnob;
    std::unique_ptr<ShardKnobCell> ampPanKnob;
    std::unique_ptr<ShardKnobCell> ampVelFloorKnob;
    std::unique_ptr<ShardKnobCell> ampVelSlopeKnob;
    std::unique_ptr<ShardKnobCell> ampEnvDecayKnob;
    std::unique_ptr<ShardKnobCell> ampEnvSlopeKnob;
    std::unique_ptr<ShardKnobCell> ampEnvClapsKnob;
    std::unique_ptr<ShardKnobCell> ampEnvClapSpeedKnob;
    std::unique_ptr<ShardKnobCell> limiterGainKnob;
    std::unique_ptr<ShardKnobCell> limiterThreshKnob;
    std::unique_ptr<ShardKnobCell> limiterReleaseKnob;

    // APVTS Attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<ComboBoxAttachment>> boxAttachments;

    std::vector<float> scopeBuffer;

    void layoutFaceplate();
    void bindKnob(std::unique_ptr<ShardKnobCell>& cell, const juce::String& paramId);
    void bindSelector(LedSelectorComponent& sel, juce::ComboBox& box, const juce::String& paramId,
                      const juce::StringArray& items, int columns = 2);
    int syncSelector(juce::ComboBox& box, LedSelectorComponent& sel, const juce::String& paramId, int& lastVal);

    void updateCarrierControls();
    void updateModControls();

    int lastCarrierTracking = -1;
    int lastModTrack = -1;
    int lastModType = -1;
    int lastPitchEnvTarget = -1;
    int lastFilterType = -1;
    int lastFilterSlope = -1;
    int lastLimiterEnable = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TheShardAudioProcessorEditor)
};
