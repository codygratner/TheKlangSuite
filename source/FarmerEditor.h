#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "KlangCoreEditor.h"
#include "FarmerProcessor.h"
#include "UIComponents.h"
#include <vector>
#include <memory>
#include <functional>

// Dynamic Card component representing an independent FX slot
class FXSlotCardComponent : public juce::Component,
                            public juce::SettableTooltipClient,
                            public juce::DragAndDropTarget {
public:
    FXSlotCardComponent(int slotIndex, bool isPostRack);
    void paint(juce::Graphics& g) override;
    void resized() override;

    void configureForType(int fxType);
    int getCurrentType() const { return currentType; }
    juce::String getTitle() const { return title; }

    RotaryKnobSlider& getKnob(int index) { return knobs[index]; }
    LedSelectorComponent& getSelector1() { return selector1; }
    LedSelectorComponent& getSelector2() { return selector2; }
    juce::ComboBox& getTypeSelector() { return typeSelector; }

    void updateDynamicControls();

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;

    // Drag and Drop Target implementation
    bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;
    void itemDragEnter(const SourceDetails& dragSourceDetails) override;
    void itemDragMove(const SourceDetails& dragSourceDetails) override;
    void itemDragExit(const SourceDetails& dragSourceDetails) override;
    void itemDropped(const SourceDetails& dragSourceDetails) override;

    std::function<void()> onCardClicked;
    std::function<void(bool srcIsPost, int srcSlot, bool dstIsPost, int dstSlot)> onSwapRequested;
    std::function<void(int newType)> onTypeChanged;

private:
    int slotIndex = 0;
    bool isPost = false;
    int currentType = 0;
    juce::String title;
    juce::Colour accent;
    bool isDragOver = false;

    juce::ComboBox typeSelector;
    juce::Label labels[4];
    RotaryKnobSlider knobs[4];
    LedSelectorComponent selector1;
    LedSelectorComponent selector2;
    int numActiveKnobs = 4;
    int lastSel1 = -1;
    int lastSel2 = -1;
};

// Hardware rack blank faceplate
class BlankPlateComponent : public juce::Component {
public:
    void paint(juce::Graphics& g) override;
};

// Permanent Slot 1: Navigation Block
class NavigationCardComponent : public juce::Component,
                                public juce::SettableTooltipClient {
public:
    NavigationCardComponent();
    void paint(juce::Graphics& g) override;
    void resized() override;
    void setSelectedPage(int pageIndex);
    int getSelectedPage() const { return selectedPage; }

    std::function<void(int)> onPageSelected;

private:
    int selectedPage = 0;
    juce::StringArray pageNames {
        "VOICE 1",
        "VOICE 2",
        "TRANSIENTS",
        "EFFECTS",
        "AMPLIFIER",
        "MODULATIONS"
    };
    std::vector<std::unique_ptr<juce::TextButton>> buttons;
};

// Slot 2: FX Picker Component (4 dropdown selectors)
class FXPickerCardComponent : public juce::Component {
public:
    FXPickerCardComponent(const juce::String& titleText, juce::Colour accentCol);
    void paint(juce::Graphics& g) override;
    void resized() override;
    juce::ComboBox& getBox(int index) { return boxes[index]; }

private:
    juce::String title;
    juce::Colour accent;
    juce::Label labels[4];
    juce::ComboBox boxes[4];
};

// Permanent Slot 8: Visualizations Block with auto-switching, module title, and lock icon
class VisualizationCardComponent : public juce::Component,
                                   public juce::SettableTooltipClient {
public:
    VisualizationCardComponent(juce::Colour accentColour);
    void paint(juce::Graphics& g) override;
    void resized() override;

    juce::String getTooltip() override;

    void setVisualizedBlock(int blockIndex, const juce::String& blockName);
    int getCurrentBlockIndex() const { return currentBlockIndex; }
    juce::String getCurrentBlockName() const { return currentBlockName; }

    void setLocked(bool locked) { isLocked = locked; repaint(); }
    bool getIsLocked() const { return isLocked; }

    void setIsOff(bool off);
    bool getIsOff() const { return isOff; }

    MiniOscilloscopeComponent& getOscilloscope() { return oscilloscope; }

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

    juce::Rectangle<int> getLockBounds() const;
    juce::Rectangle<int> getOffBounds() const;

    std::function<void()> onPanicTriggered;

private:
    juce::Colour accent;
    int currentBlockIndex = 0;
    juce::String currentBlockName { "CARRIER 1" };
    bool isLocked = false;
    bool isLockHovered = false;
    bool isOff = false;
    bool isOffHovered = false;

    MiniOscilloscopeComponent oscilloscope;
};

// Non-scrollable full-page quickstart reference modal dialog
class QuickstartGuideModalComponent : public juce::Component {
public:
    QuickstartGuideModalComponent();
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    bool keyPressed(const juce::KeyPress& key) override;

    juce::Rectangle<int> getCardBounds() const;

private:
    juce::TextButton closeButton { "✕" };
};

// Table component listing active modulation routes on Page 5
class ModulationMatrixTableComponent : public juce::Component,
                                       public juce::TableListBoxModel {
public:
    explicit ModulationMatrixTableComponent(TheKlangFarmerAudioProcessor& proc);
    ~ModulationMatrixTableComponent() override { table.setModel(nullptr); }

    void paint(juce::Graphics& g) override;
    void resized() override;

    int getNumRows() override;
    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;

    void refresh();

private:
    TheKlangFarmerAudioProcessor& processor;
    juce::TableListBox table;
    struct RouteDisplay {
        juce::String source;
        juce::String target;
        float depth = 0.0f;
        juce::String viaSource;
        float viaDepth = 0.0f;
    };
    std::vector<RouteDisplay> activeRoutes;
};

class TheKlangFarmerAudioProcessorEditor : public KlangCoreEditor,
                                           public juce::DragAndDropContainer,
                                           private juce::Timer {
public:
    explicit TheKlangFarmerAudioProcessorEditor(TheKlangFarmerAudioProcessor&);
    ~TheKlangFarmerAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void updateDynamicControls();
    void bindSelector(LedSelectorComponent& selector, juce::ComboBox& box,
                      const juce::String& paramId, const juce::StringArray& items, int numColumns = 1);

    void setPage(int pageIndex);
    void updatePageLayout();
    void handleFXSwap(bool srcIsPost, int srcSlot, bool dstIsPost, int dstSlot);

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& d) override;
    bool keyPressed(const juce::KeyPress& key) override;
    void handleCardInteraction(juce::Component* comp);
    void resetToDefaults(bool cleanFX = false);

    void rollGlobalDice();
    void openInspectorForSource(int sourceIndex);
    void updateTracerForFocusedKnob(RotaryKnobSlider* knob);
    void updateTracerForInspector(int sourceIndex);

    // Header & Modulator Strip accessors
    juce::TextButton& getUndoButton() { return undoButton; }
    juce::TextButton& getRedoButton() { return redoButton; }
    juce::TextButton& getAbButton() { return abButton; }
    DiceButton& getGlobalDiceButton() { return globalDiceButton; }
    HeaderMeterComponent& getHeaderMeter() { return headerMeter; }
    LowerModulatorStripComponent& getModStrip() { return modStrip; }
    RotaryKnobSlider& getMacroKnob(int idx) { return macroKnobs[idx]; }
    ModulationTracerOverlay& getTracerOverlay() { return tracerOverlay; }
    void openInspector(int sourceIndex) { openInspectorForSource(sourceIndex); }
    ModulationInspectorPopover* getInspectorPopover() { return inspectorPopover.get(); }
    ModulationMatrixTableComponent* getModMatrixTable() { return modMatrixTable.get(); }

private:
    TheKlangFarmerAudioProcessor& audioProcessor;

    // Header controls
    juce::TextButton undoButton { juce::CharPointer_UTF8("\xe2\x86\xb6") }; // ↶
    juce::TextButton redoButton { juce::CharPointer_UTF8("\xe2\x86\xb7") }; // ↷
    juce::TextButton abButton { "A | B" };
    DiceButton globalDiceButton { "globalDiceButton" };
    HeaderMeterComponent headerMeter;

    // Header Performance Macro Knobs
    RotaryKnobSlider macroKnobs[4];
    float globalD6Depth = 0.25f;

    // Modulation Tracer Overlay & Inspector Popover
    ModulationTracerOverlay tracerOverlay;
    std::unique_ptr<ModulationInspectorPopover> inspectorPopover;
    std::unique_ptr<ModulationMatrixTableComponent> modMatrixTable;

    // Lower Modulator Strip
    LowerModulatorStripComponent modStrip;

    // Quickstart Guide overlay
    QuickstartGuideModalComponent quickstartGuide;

    // Preset File Chooser
    std::unique_ptr<juce::FileChooser> fileChooser;

    // Permanent blocks (Slot 1 and Slot 8)
    NavigationCardComponent navCard;
    VisualizationCardComponent vizCard;
    int currentPage = 0;

    // Blank plates for empty rack slots
    BlankPlateComponent blankPlates[6];

    // FX Pickers
    std::unique_ptr<FXPickerCardComponent> preFXPickerCard;
    std::unique_ptr<FXPickerCardComponent> postFXPickerCard;

    // Module Cards
    std::unique_ptr<ModuleCardComponent> cardCarrier1;
    std::unique_ptr<ModuleCardComponent> cardMod1;
    std::unique_ptr<ModuleCardComponent> cardPitchEnv1;
    std::unique_ptr<ModuleCardComponent> cardFilter1;
    std::unique_ptr<ModuleCardComponent> cardFilterEnv1;

    std::unique_ptr<ModuleCardComponent> cardCarrier2;
    std::unique_ptr<ModuleCardComponent> cardMod2;
    std::unique_ptr<ModuleCardComponent> cardPitchEnv2;
    std::unique_ptr<ModuleCardComponent> cardFilter2;
    std::unique_ptr<ModuleCardComponent> cardFilterEnv2;

    std::unique_ptr<ModuleCardComponent> cardNoise;
    std::unique_ptr<ModuleCardComponent> cardFilter3;
    std::unique_ptr<ModuleCardComponent> cardFilterEnv3;

    std::unique_ptr<ModuleCardComponent> cardMixer;

    // Multi-Instance FX Slot Cards
    std::unique_ptr<FXSlotCardComponent> preFXCards[4];
    std::unique_ptr<FXSlotCardComponent> postFXCards[4];
    void setFXSlotDefaults(int slot, bool isPost, int fxType);

    std::unique_ptr<ModuleCardComponent> cardAmp;
    std::unique_ptr<ModuleCardComponent> cardAmpEnv;

    std::unique_ptr<ModuleCardComponent> cardPreLimiter;
    std::unique_ptr<ModuleCardComponent> cardPostLimiter;

    std::unique_ptr<ModuleCardComponent> cardVelocity;
    std::unique_ptr<ModuleCardComponent> cardKeyTrack;
    std::unique_ptr<ModuleCardComponent> cardSlop;
    std::unique_ptr<ModuleCardComponent> cardModEnv1;
    std::unique_ptr<ModuleCardComponent> cardModEnv2;
    std::unique_ptr<ModuleCardComponent> cardModEnv3;

    // --- Controls ---
    // Voice 1
    juce::ComboBox carrier1TrackingBox;
    LedSelectorComponent carrier1TrackingSelector;
    RotaryKnobSlider carrier1PitchSlider;
    RotaryKnobSlider carrier1ShapeSlider;
    RotaryKnobSlider carrier1DepthSlider;

    juce::ComboBox mod1TrackBox;
    LedSelectorComponent mod1TrackSelector;
    juce::ComboBox mod1TypeBox;
    LedSelectorComponent mod1TypeSelector;
    RotaryKnobSlider mod1ShapeSlider;
    RotaryKnobSlider mod1SpeedSlider;

    juce::ComboBox pitchEnv1TargetBox;
    LedSelectorComponent pitchEnv1TargetSelector;
    RotaryKnobSlider pitchEnv1SlopeSlider;
    RotaryKnobSlider pitchEnv1DepthSlider;
    RotaryKnobSlider pitchEnv1DecaySlider;

    juce::ComboBox filter1TypeBox;
    LedSelectorComponent filter1TypeSelector;
    juce::ComboBox filter1SlopeBox;
    LedSelectorComponent filter1SlopeSelector;
    RotaryKnobSlider filter1CutoffSlider;
    RotaryKnobSlider filter1ResonanceSlider;

    RotaryKnobSlider filterEnv1SlopeSlider;
    RotaryKnobSlider filterEnv1DepthSlider;
    RotaryKnobSlider filterEnv1DecaySlider;
    RotaryKnobSlider filterEnv1PostDriveSlider;

    // Voice 2
    juce::ComboBox carrier2TrackingBox;
    LedSelectorComponent carrier2TrackingSelector;
    RotaryKnobSlider carrier2PitchSlider;
    RotaryKnobSlider carrier2ShapeSlider;
    RotaryKnobSlider carrier2DepthSlider;

    juce::ComboBox mod2TrackBox;
    LedSelectorComponent mod2TrackSelector;
    juce::ComboBox mod2TypeBox;
    LedSelectorComponent mod2TypeSelector;
    RotaryKnobSlider mod2ShapeSlider;
    RotaryKnobSlider mod2SpeedSlider;

    juce::ComboBox pitchEnv2TargetBox;
    LedSelectorComponent pitchEnv2TargetSelector;
    RotaryKnobSlider pitchEnv2SlopeSlider;
    RotaryKnobSlider pitchEnv2DepthSlider;
    RotaryKnobSlider pitchEnv2DecaySlider;

    juce::ComboBox filter2TypeBox;
    LedSelectorComponent filter2TypeSelector;
    juce::ComboBox filter2SlopeBox;
    LedSelectorComponent filter2SlopeSelector;
    RotaryKnobSlider filter2CutoffSlider;
    RotaryKnobSlider filter2ResonanceSlider;

    RotaryKnobSlider filterEnv2SlopeSlider;
    RotaryKnobSlider filterEnv2DepthSlider;
    RotaryKnobSlider filterEnv2DecaySlider;
    RotaryKnobSlider filterEnv2PostDriveSlider;

    // Transients
    RotaryKnobSlider noiseShRateSlider;
    RotaryKnobSlider noiseFilterSlider;
    RotaryKnobSlider noiseDriveSlider;
    RotaryKnobSlider noiseDecaySlider;

    juce::ComboBox filter3TypeBox;
    LedSelectorComponent filter3TypeSelector;
    juce::ComboBox filter3SlopeBox;
    LedSelectorComponent filter3SlopeSelector;
    RotaryKnobSlider filter3CutoffSlider;
    RotaryKnobSlider filter3ResonanceSlider;

    RotaryKnobSlider filterEnv3SlopeSlider;
    RotaryKnobSlider filterEnv3DepthSlider;
    RotaryKnobSlider filterEnv3DecaySlider;
    RotaryKnobSlider filterEnv3PostDriveSlider;

    // Mixer
    RotaryKnobSlider mixerCarrier1LevelSlider;
    RotaryKnobSlider mixerCarrier2LevelSlider;
    RotaryKnobSlider mixerRingModSlider;
    RotaryKnobSlider mixerNoiseLevelSlider;

    // Amp
    RotaryKnobSlider ampPanSlider;
    RotaryKnobSlider ampLevelSlider;
    RotaryKnobSlider ampDriveSlider;
    juce::ComboBox ampLimiterBox;
    LedSelectorComponent ampLimiterSelector;

    // Amp Envelope
    RotaryKnobSlider ampEnvClapsSlider;
    RotaryKnobSlider ampEnvClapSpeedSlider;
    RotaryKnobSlider ampEnvSlopeSlider;
    RotaryKnobSlider ampEnvDecaySlider;

    // Pre-Amp Limiter
    juce::ComboBox preLimiterEnableBox;
    LedSelectorComponent preLimiterEnableSelector;
    RotaryKnobSlider preLimiterGainSlider;
    RotaryKnobSlider preLimiterThreshSlider;
    RotaryKnobSlider preLimiterReleaseSlider;

    // Post-Amp Limiter (Master Limiter)
    juce::ComboBox postLimiterEnableBox;
    LedSelectorComponent postLimiterEnableSelector;
    RotaryKnobSlider postLimiterGainSlider;
    RotaryKnobSlider postLimiterThreshSlider;
    RotaryKnobSlider postLimiterReleaseSlider;

    // Velocity
    RotaryKnobSlider velSlopeSlider;
    RotaryKnobSlider velDepthSlider;
    RotaryKnobSlider velDecaySlider;
    RotaryKnobSlider velVolumeSlider;

    // Key Tracking
    RotaryKnobSlider keySlopeSlider;
    RotaryKnobSlider keyDepthSlider;
    RotaryKnobSlider keyDecaySlider;
    RotaryKnobSlider keyVolumeSlider;

    // Slop
    RotaryKnobSlider slopFreqSlider;
    RotaryKnobSlider slopDepthSlider;
    RotaryKnobSlider slopDecaySlider;
    RotaryKnobSlider slopPanSlider;

    // Mod Envelopes 1..3
    RotaryKnobSlider modEnv1SlopeSlider;
    RotaryKnobSlider modEnv1DepthSlider;
    RotaryKnobSlider modEnv1DecaySlider;
    juce::ComboBox   modEnv1TargetBox;

    RotaryKnobSlider modEnv2SlopeSlider;
    RotaryKnobSlider modEnv2DepthSlider;
    RotaryKnobSlider modEnv2DecaySlider;
    juce::ComboBox   modEnv2TargetBox;

    RotaryKnobSlider modEnv3SlopeSlider;
    RotaryKnobSlider modEnv3DepthSlider;
    RotaryKnobSlider modEnv3DecaySlider;
    juce::ComboBox   modEnv3TargetBox;

    // APVTS Attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<ComboBoxAttachment>> boxAttachments;

    void setupKnob(RotaryKnobSlider& slider, juce::Colour trackColour, bool isBipolar = false, double defaultVal = 0.5);
    void setupBox(juce::ComboBox& box);
    void updateCarrier1Controls();
    void updateCarrier2Controls();

    // Tracking states
    int lastCarrier1Track = -1;
    int lastMod1Track = -1;
    int lastMod1Type = -1;
    int lastPitchEnv1Target = -1;
    int lastFilter1Type = -1;
    int lastFilter1Slope = -1;

    int lastCarrier2Track = -1;
    int lastMod2Track = -1;
    int lastMod2Type = -1;
    int lastPitchEnv2Target = -1;
    int lastFilter2Type = -1;
    int lastFilter2Slope = -1;

    int lastFilter3Type = -1;
    int lastFilter3Slope = -1;

    int lastAmpLimiter = -1;
    int lastPreLimiterEnable = -1;
    int lastPostLimiterEnable = -1;

    int lastPreFXTypes[4] = { -1, -1, -1, -1 };
    int lastPostFXTypes[4] = { -1, -1, -1, -1 };
    void updateModTargetBoxItems();

    std::vector<float> scopeBuffer;
    std::vector<RotaryKnobSlider*> registeredSliders;
    void bindSlider(const juce::String& paramId, RotaryKnobSlider& slider);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TheKlangFarmerAudioProcessorEditor)
};

