#include "FormlessEditor.h"

FormlessSpawnAudioProcessorEditor::FormlessSpawnAudioProcessorEditor(FormlessSpawnAudioProcessor& p)
    : AudioProcessorEditor(&p), processor_(p)
{
    setSize(900, 560);
    setLookAndFeel(&lnf_);

    auto& apvts = processor_.getAPVTS();

    // 1. Setup Header Controls
    roleBar_.setAccentColour(juce::Colour(0xFFFF9D00));
    roleBar_.onSelect = [&apvts](int idx) {
        if (auto* param = apvts.getParameter("role"))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
    };
    if (auto* param = apvts.getRawParameterValue("role"))
        roleBar_.setState(static_cast<int>(param->load()));
    addAndMakeVisible(roleBar_);

    modelBar_.setAccentColour(juce::Colour(0xFFFF9D00));
    modelBar_.onSelect = [this, &apvts](int idx) {
        if (auto* param = apvts.getParameter("model"))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
        updateModelLabels(idx);
    };
    if (auto* param = apvts.getRawParameterValue("model")) {
        const int m = static_cast<int>(param->load());
        modelBar_.setState(m);
        updateModelLabels(m);
    }
    addAndMakeVisible(modelBar_);

    lifeformRocker_.onSelect = [&apvts](int idx) {
        if (auto* param = apvts.getParameter("life_mode"))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
    };
    if (auto* param = apvts.getRawParameterValue("life_mode"))
        lifeformRocker_.setState(static_cast<int>(param->load()));
    addAndMakeVisible(lifeformRocker_);

    keyTrackBar_.setAccentColour(juce::Colour(0xFF00C8FF));
    keyTrackBar_.onSelect = [&apvts](int idx) {
        if (auto* param = apvts.getParameter("key_track"))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
    };
    if (auto* param = apvts.getRawParameterValue("key_track"))
        keyTrackBar_.setState(static_cast<int>(param->load()));
    addAndMakeVisible(keyTrackBar_);

    trigButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF21262D));
    trigButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFFF0F6FC));
    trigButton_.onClick = [this]() {
        processor_.triggerManual(0.95f);
    };
    addAndMakeVisible(trigButton_);

    // 2. Setup The One Big Knob: MACRO DECAY (84px, Gold Accent)
    setupKnob(macroDecayKnob_, "macro_decay", "MACRO DECAY", juce::Colour(0xFFFF9D00));
    macroDecayKnob_.slider.setDoubleClickReturnValue(true, 0.5);

    // 3. Setup Card 1 Knobs
    setupKnob(pitchKnob_,      "pitch",       "PITCH",       juce::Colour(0xFF00C8FF));
    setupKnob(punchKnob_,      "punch",       "PUNCH",       juce::Colour(0xFF00C8FF));
    setupKnob(pitchDepthKnob_, "pitch_depth", "P-DEPTH",     juce::Colour(0xFF00C8FF), true);
    setupKnob(pitchDecayKnob_, "pitch_decay", "P-DECAY",     juce::Colour(0xFF00C8FF));
    setupKnob(timbreXKnob_,    "timbre_x",    "FM RATIO",    juce::Colour(0xFFE5A91E));
    setupKnob(timbreYKnob_,    "timbre_y",    "FM DEPTH",    juce::Colour(0xFFE5A91E));
    setupKnob(timbreZKnob_,    "timbre_z",    "WAVEFOLD",    juce::Colour(0xFFE5A91E));

    // 4. Setup Card 2 Knobs
    setupKnob(cutoffKnob_,     "cutoff",      "CUTOFF",      juce::Colour(0xFF6BBF59));
    setupKnob(resonanceKnob_,  "resonance",   "RESONANCE",   juce::Colour(0xFF6BBF59));
    setupKnob(preDriveKnob_,   "pre_drive",   "PRE-DRIVE",   juce::Colour(0xFFE08C00));
    setupKnob(lowBoostKnob_,   "low_boost",   "LOW BOOST",   juce::Colour(0xFFE08C00));
    setupKnob(saturateKnob_,   "saturate",    "SATURATE",    juce::Colour(0xFFE08C00));
    setupKnob(panKnob_,        "pan",         "PAN",         juce::Colour(0xFF8B949E), true);
    setupKnob(volumeKnob_,     "volume",      "VOLUME",      juce::Colour(0xFFF0F6FC));

    vcfModeBar_.setAccentColour(juce::Colour(0xFF6BBF59));
    vcfModeBar_.onSelect = [&apvts](int idx) {
        if (auto* param = apvts.getParameter("vcf_mode"))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
    };
    if (auto* param = apvts.getRawParameterValue("vcf_mode"))
        vcfModeBar_.setState(static_cast<int>(param->load()));
    addAndMakeVisible(vcfModeBar_);
}

FormlessSpawnAudioProcessorEditor::~FormlessSpawnAudioProcessorEditor() {
    setLookAndFeel(nullptr);
}

void FormlessSpawnAudioProcessorEditor::setupKnob(KnobItem& item, const juce::String& paramId,
                                                  const juce::String& name, juce::Colour col, bool isBipolar) {
    item.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    item.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 54, 14);
    item.slider.setColour(juce::Slider::rotarySliderFillColourId, col);
    if (isBipolar) {
        item.slider.setDoubleClickReturnValue(true, 0.0);
    }
    addAndMakeVisible(item.slider);

    item.label.setText(name, juce::dontSendNotification);
    item.label.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
    item.label.setColour(juce::Label::textColourId, juce::Colour(0xFF8B949E));
    item.label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(item.label);

    item.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor_.getAPVTS(), paramId, item.slider);
}

void FormlessSpawnAudioProcessorEditor::updateModelLabels(int modelIndex) {
    if (modelIndex == 0) {
        timbreXKnob_.label.setText("FM RATIO", juce::dontSendNotification);
        timbreYKnob_.label.setText("FM DEPTH", juce::dontSendNotification);
        timbreZKnob_.label.setText("WAVEFOLD", juce::dontSendNotification);
    } else if (modelIndex == 1) {
        timbreXKnob_.label.setText("RING Q", juce::dontSendNotification);
        timbreYKnob_.label.setText("PULSE W", juce::dontSendNotification);
        timbreZKnob_.label.setText("ASYM", juce::dontSendNotification);
    } else {
        timbreXKnob_.label.setText("FOLD DEPTH", juce::dontSendNotification);
        timbreYKnob_.label.setText("SYMMETRY", juce::dontSendNotification);
        timbreZKnob_.label.setText("STAGES", juce::dontSendNotification);
    }
}

void FormlessSpawnAudioProcessorEditor::paint(juce::Graphics& g) {
    // 1. Master Chassis Background
    g.fillAll(juce::Colour(0xFF0D1117));

    // 2. Header Deck Capsule
    auto headerRect = juce::Rectangle<float>(14.0f, 10.0f, getWidth() - 28.0f, 44.0f);
    g.setColour(juce::Colour(0xFF161B22));
    g.fillRoundedRectangle(headerRect, 5.0f);
    g.setColour(juce::Colour(0xFF30363D));
    g.drawRoundedRectangle(headerRect, 5.0f, 1.0f);

    // Title Stencil
    g.setFont(juce::Font(juce::FontOptions(14.0f).withStyle("Bold")));
    g.setColour(juce::Colour(0xFFFF9D00));
    g.drawText("FORMLESS SPAWN", 24, 10, 160, 44, juce::Justification::centredLeft);

    // 3. Card 1 Chassis (Left Deck: Drum Core & Timbre Lab)
    auto card1 = juce::Rectangle<float>(14.0f, 64.0f, (getWidth() - 36.0f) * 0.5f, getHeight() - 78.0f);
    g.setColour(juce::Colour(0xFF13171F));
    g.fillRoundedRectangle(card1, 6.0f);
    g.setColour(juce::Colour(0xFF262D38));
    g.drawRoundedRectangle(card1, 6.0f, 1.0f);

    // Card 1 Header Pill
    auto pill1 = juce::Rectangle<float>(card1.getX() + 12.0f, card1.getY() + 10.0f, 210.0f, 20.0f);
    g.setColour(juce::Colour(0xFF1F242E));
    g.fillRoundedRectangle(pill1, 3.0f);
    g.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
    g.setColour(juce::Colour(0xFF00C8FF));
    g.drawText("CARD 1: DRUM CORE & TIMBRE LAB", pill1, juce::Justification::centred);

    // 4. Card 2 Chassis (Right Deck: VCF / LPG & Output)
    auto card2 = juce::Rectangle<float>(card1.getRight() + 8.0f, 64.0f, (getWidth() - 36.0f) * 0.5f, getHeight() - 78.0f);
    g.setColour(juce::Colour(0xFF13171F));
    g.fillRoundedRectangle(card2, 6.0f);
    g.setColour(juce::Colour(0xFF262D38));
    g.drawRoundedRectangle(card2, 6.0f, 1.0f);

    // Card 2 Header Pill
    auto pill2 = juce::Rectangle<float>(card2.getX() + 12.0f, card2.getY() + 10.0f, 190.0f, 20.0f);
    g.setColour(juce::Colour(0xFF1F242E));
    g.fillRoundedRectangle(pill2, 3.0f);
    g.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
    g.setColour(juce::Colour(0xFF6BBF59));
    g.drawText("CARD 2: VCF / LPG & OUTPUT", pill2, juce::Justification::centred);
}

void FormlessSpawnAudioProcessorEditor::resized() {
    // 1. Header Layout
    int hX = 190;
    roleBar_.setBounds(hX, 18, 140, 28);
    hX += 148;
    modelBar_.setBounds(hX, 18, 200, 28);
    hX += 208;
    lifeformRocker_.setBounds(hX, 18, 180, 28);
    hX += 188;
    keyTrackBar_.setBounds(hX, 18, 80, 28);
    hX += 88;
    trigButton_.setBounds(hX, 18, 56, 28);

    // 2. Card 1 Geometry
    const int c1X = 14;
    const int c1Y = 64;

    // Row 1: PITCH (Hero 72), MACRO DECAY (Big Knob 84), PUNCH (52)
    pitchKnob_.label.setBounds(c1X + 24, c1Y + 44, 76, 14);
    pitchKnob_.slider.setBounds(c1X + 24, c1Y + 60, 76, 76);

    macroDecayKnob_.label.setBounds(c1X + 160, c1Y + 36, 100, 14);
    macroDecayKnob_.slider.setBounds(c1X + 168, c1Y + 54, 84, 84);

    punchKnob_.label.setBounds(c1X + 310, c1Y + 54, 64, 14);
    punchKnob_.slider.setBounds(c1X + 312, c1Y + 70, 60, 60);

    // Row 2: PITCH DEPTH (Hero 72) and PITCH DECAY (52)
    pitchDepthKnob_.label.setBounds(c1X + 60, c1Y + 168, 76, 14);
    pitchDepthKnob_.slider.setBounds(c1X + 60, c1Y + 184, 76, 76);

    pitchDecayKnob_.label.setBounds(c1X + 240, c1Y + 178, 64, 14);
    pitchDecayKnob_.slider.setBounds(c1X + 242, c1Y + 194, 60, 60);

    // Row 3: TIMBRE LAB (X, Y, Z - 52px each)
    timbreXKnob_.label.setBounds(c1X + 40, c1Y + 300, 80, 14);
    timbreXKnob_.slider.setBounds(c1X + 50, c1Y + 316, 60, 60);

    timbreYKnob_.label.setBounds(c1X + 170, c1Y + 300, 80, 14);
    timbreYKnob_.slider.setBounds(c1X + 180, c1Y + 316, 60, 60);

    timbreZKnob_.label.setBounds(c1X + 300, c1Y + 300, 80, 14);
    timbreZKnob_.slider.setBounds(c1X + 310, c1Y + 316, 60, 60);

    // 3. Card 2 Geometry
    const int c2X = c1X + 434;
    const int c2Y = 64;

    // Row 1: CUTOFF (Hero 72), RESONANCE (52), VCF MODE (Button Bar)
    cutoffKnob_.label.setBounds(c2X + 24, c2Y + 44, 76, 14);
    cutoffKnob_.slider.setBounds(c2X + 24, c2Y + 60, 76, 76);

    resonanceKnob_.label.setBounds(c2X + 160, c2Y + 54, 64, 14);
    resonanceKnob_.slider.setBounds(c2X + 162, c2Y + 70, 60, 60);

    vcfModeBar_.setBounds(c2X + 280, c2Y + 78, 100, 26);

    // Row 2: PRE-DRIVE (52), LOW BOOST (Hero 72), SATURATE (52)
    preDriveKnob_.label.setBounds(c2X + 40, c2Y + 178, 64, 14);
    preDriveKnob_.slider.setBounds(c2X + 42, c2Y + 194, 60, 60);

    lowBoostKnob_.label.setBounds(c2X + 160, c2Y + 168, 76, 14);
    lowBoostKnob_.slider.setBounds(c2X + 160, c2Y + 184, 76, 76);

    saturateKnob_.label.setBounds(c2X + 290, c2Y + 178, 64, 14);
    saturateKnob_.slider.setBounds(c2X + 292, c2Y + 194, 60, 60);

    // Row 3: PAN (52) and VOLUME (Hero 72)
    panKnob_.label.setBounds(c2X + 90, c2Y + 300, 64, 14);
    panKnob_.slider.setBounds(c2X + 92, c2Y + 316, 60, 60);

    volumeKnob_.label.setBounds(c2X + 240, c2Y + 292, 76, 14);
    volumeKnob_.slider.setBounds(c2X + 240, c2Y + 308, 76, 76);
}
