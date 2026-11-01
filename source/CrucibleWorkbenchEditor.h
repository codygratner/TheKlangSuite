#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "CrucibleWorkbenchProcessor.h"
#include <array>
#include <memory>

class CrucibleWorkbenchEditor : public juce::AudioProcessorEditor,
                                public juce::Timer {
public:
    explicit CrucibleWorkbenchEditor(CrucibleWorkbenchAudioProcessor&);
    ~CrucibleWorkbenchEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    CrucibleWorkbenchAudioProcessor& processor_;

    // Col 0: MmeKick 16-Control Flagship Tall Card
    struct KickCardUI {
        juce::Label titleLabel;
        juce::Label modelReadoutLabel;
        juce::TextButton trigButton{"TRIG"};
        std::array<juce::Slider, 16> knobs;
        std::array<juce::Label, 16> knobLabels;
        std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 16> attachments;
    };
    KickCardUI kickUI_;

    // Col 1: Snare & Clap Flagship Workstation (16 Knobs, 4 Trig Buttons, Dual Model Readouts)
    struct SnareClapCardUI {
        juce::Label titleLabel;
        juce::Label sdModelLabel;
        juce::Label cpModelLabel;
        juce::TextButton lilSdButton{"LIL SD"};
        juce::TextButton bigSdButton{"BIG SD"};
        juce::TextButton lilCpButton{"LIL CP"};
        juce::TextButton bigCpButton{"BIG CP"};
        std::array<juce::Slider, 16> knobs;
        std::array<juce::Label, 16> knobLabels;
        std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 16> attachments;
    };
    SnareClapCardUI snareClapUI_;

    // Col 2: Hi-Hat Flagship Workstation (16 Knobs, 2 Trig Buttons, 7-Stage Matrix)
    struct HatCardUI {
        juce::Label titleLabel;
        juce::Label infoLabel;
        juce::TextButton chTrigButton{"CH"};
        juce::TextButton ohTrigButton{"OH"};
        std::array<juce::Slider, 16> knobs;
        std::array<juce::Label, 16> knobLabels;
        std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 16> attachments;
    };
    HatCardUI hatUI_;

    // Standard Voice Cards (8 Knobs each) for Stacked Columns
    struct SlotUI {
        juce::Label titleLabel;
        juce::TextButton trigButton{"TRIG"};
        std::array<juce::Slider, 8> knobs;
        std::array<juce::Label, 8> knobLabels;
        std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 8> attachments;
    };

    // Col 3: Stacked Column (Top = Toms, Bottom = West Coast / Zap)
    SlotUI tomUI_;       // Processor Slot 2 (Toms / Perc)
    SlotUI westZapUI_;   // Processor Slot 3 (West Coast / FM Zap)

    // Col 4: Stacked Column (Top = Crash / Cymbals, Bottom = 808 Sub-Bass)
    SlotUI crashUI_;     // Processor Slot 7 (Crash / Cymbals)

    // Dedicated 808 Sub-Bass Card (Col 4 Bottom)
    struct Sub808CardUI {
        juce::Label titleLabel;
        juce::Label modelReadoutLabel;
        juce::TextButton trigButton{"TRIG"};
        std::array<juce::Slider, 8> knobs;
        std::array<juce::Label, 8> knobLabels;
        std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 8> attachments;
    };
    Sub808CardUI sub808UI_;

    // Master Console Card Controls (Bottom Bar)
    juce::Label masterTitleLabel_{{}, "STUDIO MASTER"};
    juce::Slider masterVolumeSlider_;
    juce::Slider masterDriveSlider_;
    juce::Slider masterLowBoostSlider_;
    juce::Label masterVolumeLabel_{{}, "VOLUME"};
    juce::Label masterDriveLabel_{{}, "DRIVE"};
    juce::Label masterLowBoostLabel_{{}, "LOW THUMP"};

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterVolAttach_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterDriveAttach_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterLowBoostAttach_;

    // Multi-Zone Keymapping Controls (Bottom Bar)
    juce::Slider subOctSlider_;
    juce::Label subOctLabel_{{}, "808 OCT"};
    juce::Slider tomOctSlider_;
    juce::Label tomOctLabel_{{}, "TOM OCT"};
    juce::Slider macroVarSlider_;
    juce::Label macroVarLabel_{{}, "VARIATION"};

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> subOctAttach_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tomOctAttach_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> macroVarAttach_;

    juce::ComboBox quantizeCombo_;
    int flashFrames_{0};

    void updateEngineLabels(int slot);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CrucibleWorkbenchEditor)
};
