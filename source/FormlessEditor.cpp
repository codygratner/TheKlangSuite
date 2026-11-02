#include "FormlessEditor.h"
#include "formless_spawn/formless_spawnParameterIds.h"

FormlessSpawnAudioProcessorEditor::FormlessSpawnAudioProcessorEditor(FormlessSpawnAudioProcessor& p)
    : AudioProcessorEditor(&p), processor_(p)
{
    using namespace formless_spawnParameters::Ids;

    setSize(900, 620);
    setLookAndFeel(&lnf_);

    auto& apvts = processor_.getAPVTS();

    // =========================================================================
    // 1. Setup Header Controls
    // =========================================================================
    roleBar_.setAccentColour(juce::Colour(0xFFFF9D00));
    roleBar_.onSelect = [&apvts](int idx) {
        if (auto* param = apvts.getParameter(role))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
    };
    if (auto* param = apvts.getRawParameterValue(role))
        roleBar_.setState(static_cast<int>(param->load()));
    addAndMakeVisible(roleBar_);

    modelBar_.setAccentColour(juce::Colour(0xFFFF9D00));
    modelBar_.onSelect = [this, &apvts](int idx) {
        if (auto* param = apvts.getParameter(model))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
        updateModelLabels(idx);
    };
    if (auto* param = apvts.getRawParameterValue(model)) {
        const int m = static_cast<int>(param->load());
        modelBar_.setState(m);
        updateModelLabels(m);
    }
    addAndMakeVisible(modelBar_);

    lifeformRocker_.onSelect = [&apvts](int idx) {
        if (auto* param = apvts.getParameter(life_mode))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
    };
    if (auto* param = apvts.getRawParameterValue(life_mode))
        lifeformRocker_.setState(static_cast<int>(param->load()));
    addAndMakeVisible(lifeformRocker_);

    keyTrackBar_.setAccentColour(juce::Colour(0xFF00C8FF));
    keyTrackBar_.onSelect = [&apvts](int idx) {
        if (auto* param = apvts.getParameter(key_track))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
    };
    if (auto* param = apvts.getRawParameterValue(key_track))
        keyTrackBar_.setState(static_cast<int>(param->load()));
    addAndMakeVisible(keyTrackBar_);

    trigButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF21262D));
    trigButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFFF0F6FC));
    trigButton_.onClick = [this]() {
        processor_.triggerManual(0.95f);
    };
    addAndMakeVisible(trigButton_);

    // =========================================================================
    // 2. Setup Card 1 Knobs: Drum Core & Pitch Lab (8)
    // =========================================================================
    setupKnob(pitchKnob_,      pitch,       "PITCH",     juce::Colour(0xFF00C8FF));
    setupKnob(pitchDepthKnob_, pitch_depth, "P-DEPTH",   juce::Colour(0xFF00C8FF), true);
    setupKnob(pitchDecayKnob_, pitch_decay, "P-DECAY",   juce::Colour(0xFF00C8FF));
    setupKnob(pitchCurveKnob_, pitch_curve, "P-CURVE",   juce::Colour(0xFF00C8FF), true);
    setupKnob(clickTuneKnob_,  click_tune,  "CLK-TUNE",  juce::Colour(0xFF00C8FF));
    clickTuneKnob_.slider.textFromValueFunction = [](double v) { return juce::String(juce::roundToInt(v)); };
    setupKnob(clickMixKnob_,   click_mix,   "CLK-MIX",   juce::Colour(0xFF00C8FF));
    setupKnob(subLevelKnob_,   sub_level,   "SUB-LVL",   juce::Colour(0xFF00C8FF));
    setupKnob(lowBoostKnob_,   low_boost,   "LOW-BST",   juce::Colour(0xFFE08C00));

    // =========================================================================
    // 3. Setup Card 2 Knobs: Model Timbre & Acoustic Lab (8)
    // =========================================================================
    setupKnob(timbreXKnob_,    timbre_x,    "TIMBRE-X",  juce::Colour(0xFFE5A91E));
    setupKnob(timbreYKnob_,    timbre_y,    "TIMBRE-Y",  juce::Colour(0xFFE5A91E));
    setupKnob(timbreZKnob_,    timbre_z,    "TIMBRE-Z",  juce::Colour(0xFFE5A91E));
    setupKnob(timbreTKnob_,    timbre_t,    "TIMBRE-T",  juce::Colour(0xFFE5A91E));
    setupKnob(fmMorphKnob_,    fm_morph,    "FM-MORPH",  juce::Colour(0xFFE5A91E));
    setupKnob(dampKnob_,       damp,        "DAMP",      juce::Colour(0xFFE5A91E));
    setupKnob(panKnob_,        pan,         "PAN",       juce::Colour(0xFF8B949E), true);
    setupKnob(velSensKnob_,    vel_sens,    "VEL-SENS",  juce::Colour(0xFF8B949E));

    // =========================================================================
    // 4. Setup Card 3 Left Pod: Stepped Random (4 knobs + 1 button)
    // =========================================================================
    setupKnob(randPitchKnob_,   rand_pitch,   "R-PITCH",  juce::Colour(0xFFB374FF));
    setupKnob(randDecayKnob_,   rand_decay,   "R-DECAY",  juce::Colour(0xFFB374FF));
    setupKnob(randTimbreKnob_,  rand_timbre,  "R-TIMBRE", juce::Colour(0xFFB374FF));
    setupKnob(randScatterKnob_, rand_scatter, "R-SCAT",   juce::Colour(0xFFB374FF));

    randModeBar_.setAccentColour(juce::Colour(0xFFB374FF));
    randModeBar_.onSelect = [&apvts](int idx) {
        if (auto* param = apvts.getParameter(rand_mode))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
    };
    if (auto* param = apvts.getRawParameterValue(rand_mode))
        randModeBar_.setState(static_cast<int>(param->load()));
    addAndMakeVisible(randModeBar_);

    // =========================================================================
    // 5. Setup Card 3 Center Altar: The One Big Knob & Master Out
    // =========================================================================
    setupKnob(macroDecayKnob_, macro_decay, "MACRO DECAY", juce::Colour(0xFFFF9D00));
    macroDecayKnob_.slider.setDoubleClickReturnValue(true, 0.5);

    setupKnob(volumeKnob_, volume, "VOLUME", juce::Colour(0xFFF0F6FC));

    // =========================================================================
    // 6. Setup Card 3 Right Pod: VCF & Universal Mojo Strip (7 knobs + 2 buttons)
    // =========================================================================
    setupKnob(cutoffKnob_,    cutoff,       "CUTOFF",    juce::Colour(0xFF6BBF59));
    cutoffKnob_.slider.textFromValueFunction = [](double v) { return juce::String(juce::roundToInt(v)); };
    setupKnob(resonanceKnob_, resonance,    "RES",       juce::Colour(0xFF6BBF59));
    setupKnob(preDriveKnob_,  pre_drive,    "DRIVE",     juce::Colour(0xFFE08C00));
    setupKnob(crushKnob_,     crush,        "CRUSH",     juce::Colour(0xFFFF5252));
    crushKnob_.slider.textFromValueFunction = [](double v) { return juce::String(juce::roundToInt(v)); };
    setupKnob(bitsKnob_,      bits,         "BITS",      juce::Colour(0xFFFF5252));
    setupKnob(tapeGlueKnob_,  tape_glue,    "TAPE",      juce::Colour(0xFFFF5252));
    setupKnob(mojoMixKnob_,   mojo_mix,     "MOJO-MIX",  juce::Colour(0xFFFF5252));

    vcfModeBar_.setAccentColour(juce::Colour(0xFF6BBF59));
    vcfModeBar_.onSelect = [&apvts](int idx) {
        if (auto* param = apvts.getParameter(vcf_mode))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
    };
    if (auto* param = apvts.getRawParameterValue(vcf_mode))
        vcfModeBar_.setState(static_cast<int>(param->load()));
    addAndMakeVisible(vcfModeBar_);

    mojoProfileBar_.setAccentColour(juce::Colour(0xFFFF5252));
    mojoProfileBar_.onSelect = [&apvts](int idx) {
        if (auto* param = apvts.getParameter(mojo_profile))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
    };
    if (auto* param = apvts.getRawParameterValue(mojo_profile))
        mojoProfileBar_.setState(static_cast<int>(param->load()));
    addAndMakeVisible(mojoProfileBar_);
}

FormlessSpawnAudioProcessorEditor::~FormlessSpawnAudioProcessorEditor() {
    setLookAndFeel(nullptr);
}

void FormlessSpawnAudioProcessorEditor::setupKnob(KnobItem& item, const juce::String& paramId,
                                                  const juce::String& name, juce::Colour col, bool isBipolar) {
    item.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    item.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 58, 14);
    item.slider.setColour(juce::Slider::rotarySliderFillColourId, col);
    if (isBipolar) {
        item.slider.setDoubleClickReturnValue(true, 0.0);
    }
    addAndMakeVisible(item.slider);

    item.label.setText(name, juce::dontSendNotification);
    item.label.setFont(juce::Font(juce::FontOptions(10.5f).withStyle("Bold")));
    item.label.setColour(juce::Label::textColourId, juce::Colour(0xFF8B949E));
    item.label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(item.label);

    item.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor_.getAPVTS(), paramId, item.slider);
}

void FormlessSpawnAudioProcessorEditor::updateModelLabels(int modelIndex) {
    if (modelIndex == 0) {
        timbreXKnob_.label.setText("RATIO", juce::dontSendNotification);
        timbreYKnob_.label.setText("FM-DEPTH", juce::dontSendNotification);
        timbreZKnob_.label.setText("WAVEFOLD", juce::dontSendNotification);
    } else if (modelIndex == 1) {
        timbreXKnob_.label.setText("RING-Q", juce::dontSendNotification);
        timbreYKnob_.label.setText("PULSE-W", juce::dontSendNotification);
        timbreZKnob_.label.setText("ASYM", juce::dontSendNotification);
    } else {
        timbreXKnob_.label.setText("FOLD-DEP", juce::dontSendNotification);
        timbreYKnob_.label.setText("SYMMETRY", juce::dontSendNotification);
        timbreZKnob_.label.setText("STAGES", juce::dontSendNotification);
    }
}

void FormlessSpawnAudioProcessorEditor::paint(juce::Graphics& g) {
    // 1. Master Chassis Background
    g.fillAll(juce::Colour(0xFF0D1117));

    // 2. Header Deck Capsule
    auto headerRect = juce::Rectangle<float>(12.0f, 10.0f, getWidth() - 24.0f, 36.0f);
    g.setColour(juce::Colour(0xFF161B22));
    g.fillRoundedRectangle(headerRect, 5.0f);
    g.setColour(juce::Colour(0xFF30363D));
    g.drawRoundedRectangle(headerRect, 5.0f, 1.0f);

    // Title Stencil
    g.setFont(juce::Font(juce::FontOptions(13.0f).withStyle("Bold")));
    g.setColour(juce::Colour(0xFFFF9D00));
    g.drawText("FORMLESS SPAWN", 20, 10, 130, 36, juce::Justification::centredLeft);

    // 3. Card 1 Chassis (Top Left: Drum Core & Pitch Lab)
    auto card1 = juce::Rectangle<float>(12.0f, 52.0f, 430.0f, 260.0f);
    g.setColour(juce::Colour(0xFF13171F));
    g.fillRoundedRectangle(card1, 6.0f);
    g.setColour(juce::Colour(0xFF262D38));
    g.drawRoundedRectangle(card1, 6.0f, 1.0f);

    auto pill1 = juce::Rectangle<float>(card1.getX() + 10.0f, card1.getY() + 8.0f, 190.0f, 18.0f);
    g.setColour(juce::Colour(0xFF1F242E));
    g.fillRoundedRectangle(pill1, 3.0f);
    g.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
    g.setColour(juce::Colour(0xFF00C8FF));
    g.drawText("CARD 1: DRUM CORE & PITCH LAB", pill1, juce::Justification::centred);

    // 4. Card 2 Chassis (Top Right: Model Timbre & Acoustic Lab)
    auto card2 = juce::Rectangle<float>(458.0f, 52.0f, 430.0f, 260.0f);
    g.setColour(juce::Colour(0xFF13171F));
    g.fillRoundedRectangle(card2, 6.0f);
    g.setColour(juce::Colour(0xFF262D38));
    g.drawRoundedRectangle(card2, 6.0f, 1.0f);

    auto pill2 = juce::Rectangle<float>(card2.getX() + 10.0f, card2.getY() + 8.0f, 210.0f, 18.0f);
    g.setColour(juce::Colour(0xFF1F242E));
    g.fillRoundedRectangle(pill2, 3.0f);
    g.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
    g.setColour(juce::Colour(0xFFE5A91E));
    g.drawText("CARD 2: MODEL TIMBRE & ACOUSTIC LAB", pill2, juce::Justification::centred);

    // 5. Card 3 Panoramic Deck (Performance Altar & Mojo Console)
    auto card3 = juce::Rectangle<float>(12.0f, 320.0f, 876.0f, 288.0f);
    g.setColour(juce::Colour(0xFF13171F));
    g.fillRoundedRectangle(card3, 6.0f);
    g.setColour(juce::Colour(0xFF262D38));
    g.drawRoundedRectangle(card3, 6.0f, 1.0f);

    // Sub-pod 1: Stepped Random Pod
    auto pod1 = juce::Rectangle<float>(20.0f, 328.0f, 310.0f, 272.0f);
    g.setColour(juce::Colour(0xFF0E121A));
    g.fillRoundedRectangle(pod1, 5.0f);
    g.setColour(juce::Colour(0xFF1F2633));
    g.drawRoundedRectangle(pod1, 5.0f, 1.0f);

    auto pillPod1 = juce::Rectangle<float>(pod1.getX() + 8.0f, pod1.getY() + 6.0f, 150.0f, 16.0f);
    g.setColour(juce::Colour(0xFF181C26));
    g.fillRoundedRectangle(pillPod1, 3.0f);
    g.setFont(juce::Font(juce::FontOptions(9.5f).withStyle("Bold")));
    g.setColour(juce::Colour(0xFFB374FF));
    g.drawText("STEPPED RANDOM (HASTUR)", pillPod1, juce::Justification::centred);

    // Sub-pod 2: Center Altar Pod
    auto pod2 = juce::Rectangle<float>(338.0f, 328.0f, 200.0f, 272.0f);
    g.setColour(juce::Colour(0xFF0E121A));
    g.fillRoundedRectangle(pod2, 5.0f);
    g.setColour(juce::Colour(0xFF1F2633));
    g.drawRoundedRectangle(pod2, 5.0f, 1.0f);

    auto pillPod2 = juce::Rectangle<float>(pod2.getX() + 8.0f, pod2.getY() + 6.0f, 184.0f, 16.0f);
    g.setColour(juce::Colour(0xFF181C26));
    g.fillRoundedRectangle(pillPod2, 3.0f);
    g.setFont(juce::Font(juce::FontOptions(9.5f).withStyle("Bold")));
    g.setColour(juce::Colour(0xFFFF9D00));
    g.drawText("PERFORMANCE ALTAR", pillPod2, juce::Justification::centred);

    // Sub-pod 3: VCF & Mojo Strip Pod
    auto pod3 = juce::Rectangle<float>(546.0f, 328.0f, 334.0f, 272.0f);
    g.setColour(juce::Colour(0xFF0E121A));
    g.fillRoundedRectangle(pod3, 5.0f);
    g.setColour(juce::Colour(0xFF1F2633));
    g.drawRoundedRectangle(pod3, 5.0f, 1.0f);

    auto pillPod3 = juce::Rectangle<float>(pod3.getX() + 8.0f, pod3.getY() + 6.0f, 170.0f, 16.0f);
    g.setColour(juce::Colour(0xFF181C26));
    g.fillRoundedRectangle(pillPod3, 3.0f);
    g.setFont(juce::Font(juce::FontOptions(9.5f).withStyle("Bold")));
    g.setColour(juce::Colour(0xFFFF5252));
    g.drawText("VCF & MOJO STRIP (100Hz HP)", pillPod3, juce::Justification::centred);
}

void FormlessSpawnAudioProcessorEditor::resized() {
    // =========================================================================
    // 1. Header Layout
    // =========================================================================
    int hX = 156;
    roleBar_.setBounds(hX, 15, 140, 26);
    hX += 146;
    modelBar_.setBounds(hX, 15, 150, 26);
    hX += 156;
    lifeformRocker_.setBounds(hX, 15, 150, 26);
    hX += 156;
    keyTrackBar_.setBounds(hX, 15, 80, 26);
    hX += 86;
    trigButton_.setBounds(hX, 15, 54, 26);

    // =========================================================================
    // 2. Card 1 Geometry (4 cols x 102px, w=430)
    // =========================================================================
    const int c1X = 12;
    const int c1Y = 52;

    // Row 1: PITCH (Hero 68), P-DEPTH, P-DECAY, P-CURVE (50)
    pitchKnob_.label.setBounds(c1X + 16, c1Y + 32, 68, 14);
    pitchKnob_.slider.setBounds(c1X + 16, c1Y + 46, 68, 68);

    pitchDepthKnob_.label.setBounds(c1X + 116, c1Y + 40, 60, 14);
    pitchDepthKnob_.slider.setBounds(c1X + 120, c1Y + 54, 52, 52);

    pitchDecayKnob_.label.setBounds(c1X + 218, c1Y + 40, 60, 14);
    pitchDecayKnob_.slider.setBounds(c1X + 222, c1Y + 54, 52, 52);

    pitchCurveKnob_.label.setBounds(c1X + 320, c1Y + 40, 60, 14);
    pitchCurveKnob_.slider.setBounds(c1X + 324, c1Y + 54, 52, 52);

    // Row 2: CLK-TUNE, CLK-MIX, SUB-LVL, LOW-BST (50)
    clickTuneKnob_.label.setBounds(c1X + 20, c1Y + 140, 60, 14);
    clickTuneKnob_.slider.setBounds(c1X + 24, c1Y + 154, 52, 52);

    clickMixKnob_.label.setBounds(c1X + 116, c1Y + 140, 60, 14);
    clickMixKnob_.slider.setBounds(c1X + 120, c1Y + 154, 52, 52);

    subLevelKnob_.label.setBounds(c1X + 218, c1Y + 140, 60, 14);
    subLevelKnob_.slider.setBounds(c1X + 222, c1Y + 154, 52, 52);

    lowBoostKnob_.label.setBounds(c1X + 320, c1Y + 140, 60, 14);
    lowBoostKnob_.slider.setBounds(c1X + 324, c1Y + 154, 52, 52);

    // =========================================================================
    // 3. Card 2 Geometry (4 cols x 102px, w=430)
    // =========================================================================
    const int c2X = 458;
    const int c2Y = 52;

    // Row 1: TIMBRE-X, TIMBRE-Y, TIMBRE-Z, TIMBRE-T (50)
    timbreXKnob_.label.setBounds(c2X + 20, c2Y + 40, 60, 14);
    timbreXKnob_.slider.setBounds(c2X + 24, c2Y + 54, 52, 52);

    timbreYKnob_.label.setBounds(c2X + 116, c2Y + 40, 60, 14);
    timbreYKnob_.slider.setBounds(c2X + 120, c2Y + 54, 52, 52);

    timbreZKnob_.label.setBounds(c2X + 218, c2Y + 40, 60, 14);
    timbreZKnob_.slider.setBounds(c2X + 222, c2Y + 54, 52, 52);

    timbreTKnob_.label.setBounds(c2X + 320, c2Y + 40, 60, 14);
    timbreTKnob_.slider.setBounds(c2X + 324, c2Y + 54, 52, 52);

    // Row 2: FM-MORPH, DAMP, PAN, VEL-SENS (50)
    fmMorphKnob_.label.setBounds(c2X + 20, c2Y + 140, 60, 14);
    fmMorphKnob_.slider.setBounds(c2X + 24, c2Y + 154, 52, 52);

    dampKnob_.label.setBounds(c2X + 116, c2Y + 140, 60, 14);
    dampKnob_.slider.setBounds(c2X + 120, c2Y + 154, 52, 52);

    panKnob_.label.setBounds(c2X + 218, c2Y + 140, 60, 14);
    panKnob_.slider.setBounds(c2X + 222, c2Y + 154, 52, 52);

    velSensKnob_.label.setBounds(c2X + 320, c2Y + 140, 60, 14);
    velSensKnob_.slider.setBounds(c2X + 324, c2Y + 154, 52, 52);

    // =========================================================================
    // 4. Card 3: Left Pod (Stepped Random, w=310)
    // =========================================================================
    const int pod1X = 20;
    const int pod1Y = 328;

    // Row 1: R-PITCH, R-DECAY
    randPitchKnob_.label.setBounds(pod1X + 36, pod1Y + 30, 60, 14);
    randPitchKnob_.slider.setBounds(pod1X + 40, pod1Y + 44, 52, 52);

    randDecayKnob_.label.setBounds(pod1X + 186, pod1Y + 30, 60, 14);
    randDecayKnob_.slider.setBounds(pod1X + 190, pod1Y + 44, 52, 52);

    // Row 2: R-TIMBRE, R-SCAT
    randTimbreKnob_.label.setBounds(pod1X + 36, pod1Y + 122, 60, 14);
    randTimbreKnob_.slider.setBounds(pod1X + 40, pod1Y + 136, 52, 52);

    randScatterKnob_.label.setBounds(pod1X + 186, pod1Y + 122, 60, 14);
    randScatterKnob_.slider.setBounds(pod1X + 190, pod1Y + 136, 52, 52);

    // Row 3: RAND MODE (FREE / LOOP16)
    randModeBar_.setBounds(pod1X + 65, pod1Y + 222, 180, 24);

    // =========================================================================
    // 5. Card 3: Center Altar (Macro Decay 84px + Volume 50px)
    // =========================================================================
    const int pod2X = 338;
    const int pod2Y = 328;

    macroDecayKnob_.label.setBounds(pod2X, pod2Y + 26, 200, 16);
    macroDecayKnob_.slider.setBounds(pod2X + 58, pod2Y + 44, 84, 84);

    volumeKnob_.label.setBounds(pod2X, pod2Y + 160, 200, 14);
    volumeKnob_.slider.setBounds(pod2X + 74, pod2Y + 176, 52, 52);

    // =========================================================================
    // 6. Card 3: Right Pod (VCF & Mojo Strip, w=334)
    // =========================================================================
    const int pod3X = 546;
    const int pod3Y = 328;

    // Row 1: CUTOFF (Hero 68), RES (50), DRIVE (50)
    cutoffKnob_.label.setBounds(pod3X + 14, pod3Y + 30, 68, 14);
    cutoffKnob_.slider.setBounds(pod3X + 14, pod3Y + 44, 68, 68);

    resonanceKnob_.label.setBounds(pod3X + 122, pod3Y + 38, 60, 14);
    resonanceKnob_.slider.setBounds(pod3X + 126, pod3Y + 52, 52, 52);

    preDriveKnob_.label.setBounds(pod3X + 228, pod3Y + 38, 60, 14);
    preDriveKnob_.slider.setBounds(pod3X + 232, pod3Y + 52, 52, 52);

    // Row 2: CRUSH (50), BITS (50), TAPE (50)
    crushKnob_.label.setBounds(pod3X + 18, pod3Y + 130, 60, 14);
    crushKnob_.slider.setBounds(pod3X + 18, pod3Y + 144, 60, 52);

    bitsKnob_.label.setBounds(pod3X + 122, pod3Y + 130, 60, 14);
    bitsKnob_.slider.setBounds(pod3X + 126, pod3Y + 144, 52, 52);

    tapeGlueKnob_.label.setBounds(pod3X + 228, pod3Y + 130, 60, 14);
    tapeGlueKnob_.slider.setBounds(pod3X + 232, pod3Y + 144, 52, 52);

    // Row 3: MOJO-MIX (50), VCF-MODE (Button), PROFILE (Button)
    mojoMixKnob_.label.setBounds(pod3X + 18, pod3Y + 208, 60, 14);
    mojoMixKnob_.slider.setBounds(pod3X + 22, pod3Y + 222, 52, 52);

    vcfModeBar_.setBounds(pod3X + 90, pod3Y + 230, 76, 22);
    mojoProfileBar_.setBounds(pod3X + 172, pod3Y + 230, 150, 22);
}