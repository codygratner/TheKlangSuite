#include "FormlessEditor.h"

FormlessSpawnAudioProcessorEditor::FormlessSpawnAudioProcessorEditor(FormlessSpawnAudioProcessor& p)
    : AudioProcessorEditor(&p), processor_(p)
{
    setSize(980, 560);

    const juce::String voiceNames[8] = {
        "V1: KICK", "V2: SNARE", "V3: ZAP", "V4: BONGO",
        "V5: OPEN HAT", "V6: CLOSED HAT", "V7: CLAP", "V8: CRASH"
    };

    auto& apvts = processor_.getAPVTS();

    for (int i = 0; i < 8; ++i) {
        auto& vg = voiceGroups_[i];

        vg.titleLabel.setText(voiceNames[i], juce::dontSendNotification);
        vg.titleLabel.setFont(juce::Font(13.0f, juce::Font::bold));
        vg.titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff9d00));
        vg.titleLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(vg.titleLabel);

        vg.triggerButton.setButtonText("TRIG");
        vg.triggerButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e232a));
        vg.triggerButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe2e8f0));
        vg.triggerButton.onClick = [this, i]() {
            processor_.getEngine().trigger(i, 0.95f);
        };
        addAndMakeVisible(vg.triggerButton);

        auto setupSlider = [this](juce::Slider& s, juce::Label& l, const juce::String& text) {
            s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
            s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffff9d00));
            s.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff2d3748));
            s.setColour(juce::Slider::thumbColourId, juce::Colour(0xffe2e8f0));
            addAndMakeVisible(s);

            l.setText(text, juce::dontSendNotification);
            l.setFont(juce::Font(10.0f, juce::Font::plain));
            l.setColour(juce::Label::textColourId, juce::Colour(0xffa0aec0));
            l.setJustificationType(juce::Justification::centred);
            addAndMakeVisible(l);
        };

        setupSlider(vg.pitchSlider, vg.pitchLabel, "PITCH");
        setupSlider(vg.decaySlider, vg.decayLabel, "DECAY");
        setupSlider(vg.modSlider, vg.modLabel, (i < 4) ? "MOD" : "TONE");
        setupSlider(vg.foldSlider, vg.foldLabel, (i < 4) ? "FOLD" : "LEVEL");

        vg.pitchAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, "v" + juce::String(i) + "_p0", vg.pitchSlider);
        vg.decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, "v" + juce::String(i) + "_p1", vg.decaySlider);
        vg.modAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, "v" + juce::String(i) + "_p7", vg.modSlider);
        vg.foldAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, "v" + juce::String(i) + "_p8", vg.foldSlider);
    }

    // Master Section Setup
    auto setupMasterSlider = [this](juce::Slider& s, juce::Label& l, const juce::String& text, juce::Colour col) {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 55, 18);
        s.setColour(juce::Slider::rotarySliderFillColourId, col);
        s.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff2d3748));
        s.setColour(juce::Slider::thumbColourId, juce::Colour(0xffffffff));
        addAndMakeVisible(s);

        l.setText(text, juce::dontSendNotification);
        l.setFont(juce::Font(11.0f, juce::Font::bold));
        l.setColour(juce::Label::textColourId, col);
        l.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(l);
    };

    setupMasterSlider(masterLowBoostSlider_, masterLowBoostLabel_, "LOW BOOST", juce::Colour(0xff10b981));
    setupMasterSlider(masterDriveSlider_, masterDriveLabel_, "DRIVE", juce::Colour(0xffef4444));

    masterLowBoostAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "master_low_boost", masterLowBoostSlider_);
    masterDriveAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "master_drive", masterDriveSlider_);
}

void FormlessSpawnAudioProcessorEditor::paint(juce::Graphics& g) {
    // Deep Tsathogguan Basalt Obsidian Background
    g.fillAll(juce::Colour(0xff0d1015));

    // Top Title Bar
    g.setColour(juce::Colour(0xff161b22));
    g.fillRect(0, 0, getWidth(), 45);

    g.setColour(juce::Colour(0xffff9d00));
    g.setFont(juce::Font(18.0f, juce::Font::bold));
    g.drawText("FORMLESS SPAWN", 20, 0, 300, 45, juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xff718096));
    g.setFont(juce::Font(12.0f, juce::Font::plain));
    g.drawText("TSATHOGGUAN PERCUSSION WORKSTATION  //  CULT OF KLANG", 220, 2, 500, 45, juce::Justification::centredLeft);

    // Section Dividers
    g.setColour(juce::Colour(0xff21262d));
    g.drawHorizontalLine(45, 0.0f, static_cast<float>(getWidth()));
    g.drawHorizontalLine(465, 0.0f, static_cast<float>(getWidth()));

    // Voice card backgrounds
    for (int i = 0; i < 8; ++i) {
        int col = i % 4;
        int row = i / 4;
        int x = 20 + col * 235;
        int y = 55 + row * 200;
        int w = 225;
        int h = 190;

        g.setColour(juce::Colour(0xff161b22));
        g.fillRoundedRectangle(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h), 6.0f);
        g.setColour(juce::Colour(0xff21262d));
        g.drawRoundedRectangle(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h), 6.0f, 1.0f);
    }
}

void FormlessSpawnAudioProcessorEditor::resized() {
    for (int i = 0; i < 8; ++i) {
        int col = i % 4;
        int row = i / 4;
        int cardX = 20 + col * 235;
        int cardY = 55 + row * 200;

        auto& vg = voiceGroups_[i];
        vg.titleLabel.setBounds(cardX + 10, cardY + 8, 130, 22);
        vg.triggerButton.setBounds(cardX + 155, cardY + 8, 60, 22);

        // 4 knobs in 2x2 grid inside card
        int kSize = 54;
        // Pitch
        vg.pitchSlider.setBounds(cardX + 15, cardY + 40, kSize, kSize + 15);
        vg.pitchLabel.setBounds(cardX + 15, cardY + 98, kSize, 14);

        // Decay
        vg.decaySlider.setBounds(cardX + 120, cardY + 40, kSize, kSize + 15);
        vg.decayLabel.setBounds(cardX + 120, cardY + 98, kSize, 14);

        // Mod / Tone
        vg.modSlider.setBounds(cardX + 15, cardY + 115, kSize, kSize + 15);
        vg.modLabel.setBounds(cardX + 15, cardY + 170, kSize, 14);

        // Fold / Level
        vg.foldSlider.setBounds(cardX + 120, cardY + 115, kSize, kSize + 15);
        vg.foldLabel.setBounds(cardX + 120, cardY + 170, kSize, 14);
    }

    // Master Controls at bottom bar
    masterLowBoostSlider_.setBounds(730, 475, 75, 70);
    masterLowBoostLabel_.setBounds(720, 535, 95, 18);

    masterDriveSlider_.setBounds(840, 475, 75, 70);
    masterDriveLabel_.setBounds(830, 535, 95, 18);
}
