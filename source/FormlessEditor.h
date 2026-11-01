#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "FormlessProcessor.h"
#include "UIComponents.h"

class FormlessSpawnAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
    explicit FormlessSpawnAudioProcessorEditor(FormlessSpawnAudioProcessor&);
    ~FormlessSpawnAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    FormlessSpawnAudioProcessor& processor_;

    struct VoiceControlGroup {
        juce::TextButton triggerButton;
        juce::Label titleLabel;

        juce::Slider pitchSlider;
        juce::Slider decaySlider;
        juce::Slider modSlider;
        juce::Slider foldSlider;

        juce::Label pitchLabel;
        juce::Label decayLabel;
        juce::Label modLabel;
        juce::Label foldLabel;

        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pitchAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> foldAttachment;
    };

    std::array<VoiceControlGroup, 8> voiceGroups_;

    // Master Controls
    juce::Slider masterLowBoostSlider_;
    juce::Slider masterDriveSlider_;
    juce::Label masterLowBoostLabel_;
    juce::Label masterDriveLabel_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterLowBoostAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterDriveAttachment_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FormlessSpawnAudioProcessorEditor)
};
