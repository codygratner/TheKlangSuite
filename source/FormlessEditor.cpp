#include "FormlessEditor.h"
#include "formless_spawn/formless_spawnParameterIds.h"

FormlessSpawnAudioProcessorEditor::FormlessSpawnAudioProcessorEditor(FormlessSpawnAudioProcessor& p)
    : AudioProcessorEditor(&p), processor_(p)
{
    using namespace formless_spawnParameters::Ids;

    setSize(900, 680);
    setLookAndFeel(&lnf_);

    auto& apvts = processor_.getAPVTS();

    // =========================================================================
    // 1. Setup Header Row 1
    // =========================================================================
    addAndMakeVisible(presetStub_);
    addAndMakeVisible(headerInspector_);

    lifeformRocker_.onSelect = [&apvts](int idx) {
        if (auto* param = apvts.getParameter(life_mode))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
    };
    if (auto* param = apvts.getRawParameterValue(life_mode))
        lifeformRocker_.setState(static_cast<int>(param->load()));
    addAndMakeVisible(lifeformRocker_);

    // =========================================================================
    // 2. Setup Card 0: Voice Navigator & Model Matrix
    // =========================================================================
    roleGrid_.onSelect = [this, &apvts](int roleIdx) {
        if (auto* param = apvts.getParameter(role))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(roleIdx)));
        modelGrid_.setRoleAndModel(roleIdx, modelGrid_.getModel());
    };
    roleGrid_.onHover = [this](int roleIdx) {
        HeaderInspector::Readout r;
        r.name = "ROLE: " + juce::String(tkdsp::FormlessSpawnVoice::kRoleNames[roleIdx]);
        r.value = "SEL";
        static const char* kRoleDescs[7] = {
            "Bass Drum / Sub-heavy acoustic and electronic kicks",
            "Tom / Acoustic, tuned resonant and FM percussion",
            "Snare / Crisp, snappy noise and acoustic shells",
            "Handclap / Multi-burst acoustic and electronic claps",
            "Hi-Hat / 6-osc inharmonic metallic cluster and sizzle hats",
            "Cymbal / Crash, ride, bell and atmospheric metal washes",
            "Special FX / Experimental FM warp, wavefold and laser dives"
        };
        r.blurb = kRoleDescs[roleIdx];
        r.accent = juce::Colour(0xFFFF9D00);
        r.isDefault = false;
        headerInspector_.setReadout(r);
    };
    roleGrid_.onHoverExit = [this]() {
        headerInspector_.setReadoutDefault();
    };

    modelGrid_.onSelect = [&apvts](int modelIdx) {
        if (auto* param = apvts.getParameter(model))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(modelIdx)));
    };
    modelGrid_.onHover = [this](int modelIdx, const juce::String& modelName) {
        HeaderInspector::Readout r;
        r.name = "MODEL: " + modelName;
        r.value = juce::String::formatted("#%02d", modelIdx);
        r.blurb = "active drum synthesis topology and transient core";
        r.accent = juce::Colour(0xFF00C8FF);
        r.isDefault = false;
        headerInspector_.setReadout(r);
    };
    modelGrid_.onHoverExit = [this]() {
        headerInspector_.setReadoutDefault();
    };

    if (auto* rParam = apvts.getRawParameterValue(role)) {
        const int r = static_cast<int>(rParam->load());
        roleGrid_.setState(r);
        const int m = apvts.getRawParameterValue(model) ? static_cast<int>(apvts.getRawParameterValue(model)->load()) : 0;
        modelGrid_.setRoleAndModel(r, m);
    }
    addAndMakeVisible(roleGrid_);
    addAndMakeVisible(modelGrid_);

    // =========================================================================
    // 3. Setup Card 1 Knobs: Drum Core & Pitch Lab (8)
    // =========================================================================
    setupKnob(pitchKnob_,      pitch,       "PITCH",    "Hz", "base fundamental tuning of oscillator core", juce::Colour(0xFF00C8FF));
    setupKnob(pitchDepthKnob_, pitch_depth, "P-DEPTH",  "st", "pitch envelope sweep modulation depth",      juce::Colour(0xFF00C8FF), true);
    setupKnob(pitchDecayKnob_, pitch_decay, "P-DECAY",  "ms", "exponential pitch dive decay time",           juce::Colour(0xFF00C8FF));
    setupKnob(pitchCurveKnob_, pitch_curve, "P-CURVE",  "",   "pitch envelope bend curve (log to exp)",      juce::Colour(0xFF00C8FF), true);
    setupKnob(clickTuneKnob_,  click_tune,  "CLK-TUNE", "Hz", "transient beater impact frequency",          juce::Colour(0xFF00C8FF));
    setupKnob(clickMixKnob_,   click_mix,   "CLK-MIX",  "%",  "initial transient beater click balance",      juce::Colour(0xFF00C8FF));
    setupKnob(subLevelKnob_,   sub_level,   "SUB-LVL",  "%",  "reinforcing pure sub-bass sine level",        juce::Colour(0xFF00C8FF));
    setupKnob(lowBoostKnob_,   low_boost,   "LOW-BST",  "dB", "resonant 80Hz low-end thump boost",           juce::Colour(0xFFE08C00));

    // =========================================================================
    // 4. Setup Card 2 Knobs: Model Timbre & Acoustic Lab (8)
    // =========================================================================
    setupKnob(timbreXKnob_,    timbre_x,    "TIMBRE-X", "",   "model ratio / timbre primary axis",           juce::Colour(0xFFE5A91E));
    setupKnob(timbreYKnob_,    timbre_y,    "TIMBRE-Y", "",   "model FM depth / secondary modulation axis",  juce::Colour(0xFFE5A91E));
    setupKnob(timbreZKnob_,    timbre_z,    "TIMBRE-Z", "",   "diode wavefolding & saturation depth",        juce::Colour(0xFFE5A91E));
    setupKnob(timbreTKnob_,    timbre_t,    "TIMBRE-T", "ms", "timbre settling & modulation decay time",    juce::Colour(0xFFE5A91E));
    setupKnob(fmMorphKnob_,    fm_morph,    "FM-MORPH", "",   "modulator waveform morph (sine to noise)",    juce::Colour(0xFFE5A91E));
    setupKnob(dampKnob_,       damp,        "DAMP",     "%",  "dynamic high-frequency damping",              juce::Colour(0xFFE5A91E));
    setupKnob(panKnob_,        pan,         "PAN",      "",   "stereo panorama positioning",                 juce::Colour(0xFF8B949E), true);
    setupKnob(velSensKnob_,    vel_sens,    "VEL-SENS", "%",  "velocity response dynamic scaling",           juce::Colour(0xFF8B949E));

    // =========================================================================
    // 5. Setup Card 3 Pod 1: Stepped Random (4 knobs + 1 bar)
    // =========================================================================
    setupKnob(randPitchKnob_,   rand_pitch,   "R-PITCH",  "st", "per-hit randomized pitch drift",            juce::Colour(0xFFB374FF));
    setupKnob(randDecayKnob_,   rand_decay,   "R-DECAY",  "%",  "per-hit decay time jitter",                 juce::Colour(0xFFB374FF));
    setupKnob(randTimbreKnob_,  rand_timbre,  "R-TIMBRE", "%",  "per-hit timbre modulation offset",          juce::Colour(0xFFB374FF));
    setupKnob(randScatterKnob_, rand_scatter, "R-SCAT",   "%",  "per-hit velocity and dynamics scatter",     juce::Colour(0xFFB374FF));

    randModeBar_.setAccentColour(juce::Colour(0xFFB374FF));
    randModeBar_.onSelect = [&apvts](int idx) {
        if (auto* param = apvts.getParameter(rand_mode))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
    };
    if (auto* param = apvts.getRawParameterValue(rand_mode))
        randModeBar_.setState(static_cast<int>(param->load()));
    addAndMakeVisible(randModeBar_);

    // =========================================================================
    // 6. Setup Card 3 Pod 2: The Performance Altar (EXACTLY TWO CONTROLS)
    // =========================================================================
    setupKnob(macroDecayKnob_, macro_decay, "MACRO DECAY", "", "master exponential envelope decay scaling", juce::Colour(0xFFFF9D00));
    macroDecayKnob_.slider.setDoubleClickReturnValue(true, 0.5);

    trigButton_.onTrigger = [this]() {
        processor_.triggerManual(0.95f);
    };
    addAndMakeVisible(trigButton_);

    // =========================================================================
    // 7. Setup Card 3 Pod 3: VCF, Universal Mojo Strip & Master Output
    // =========================================================================
    setupKnob(cutoffKnob_,    cutoff,       "CUTOFF",    "Hz", "filter cutoff frequency",                    juce::Colour(0xFF6BBF59));
    setupKnob(resonanceKnob_, resonance,    "RES",       "%",  "filter resonance / self-oscillation",        juce::Colour(0xFF6BBF59));
    setupKnob(preDriveKnob_,  pre_drive,    "DRIVE",     "%",  "pre-filter analog saturation and drive",     juce::Colour(0xFFE08C00));
    setupKnob(crushKnob_,     crush,        "CRUSH",     "Hz", "mojo sample rate decimation frequency",      juce::Colour(0xFFFF5252));
    setupKnob(bitsKnob_,      bits,         "BITS",      "bit","mojo word-length bit reduction",             juce::Colour(0xFFFF5252));
    setupKnob(tapeGlueKnob_,  tape_glue,    "TAPE",      "%",  "analog tape glue saturation and warmth",     juce::Colour(0xFFFF5252));
    setupKnob(mojoMixKnob_,   mojo_mix,     "MOJO-MIX",  "%",  "delta crossover hardware mojo wet/dry mix",  juce::Colour(0xFFFF5252));
    setupKnob(volumeKnob_,    volume,       "VOLUME",    "%",  "master output stage level",                  juce::Colour(0xFFF0F6FC));

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
                                                  const juce::String& name, const juce::String& unit,
                                                  const juce::String& blurb, juce::Colour col, bool isBipolar) {
    item.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    item.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
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

    // Live Streaming Telemetry to HeaderInspector
    auto sendTelemetry = [this, name, unit, blurb, col](double val) {
        HeaderInspector::Readout r;
        r.name = name;
        if (unit == "Hz") {
            if (val >= 1000.0)
                r.value = juce::String(val * 0.001, 2) + " kHz";
            else
                r.value = juce::String(juce::roundToInt(val)) + " Hz";
        } else if (unit == "ms") {
            r.value = juce::String(val, 1) + " ms";
        } else if (unit == "st") {
            r.value = (val > 0.0 ? "+" : "") + juce::String(val, 1) + " st";
        } else if (unit == "dB") {
            r.value = juce::String(val, 1) + " dB";
        } else if (unit == "%") {
            r.value = juce::String(juce::roundToInt(val * 100.0)) + " %";
        } else if (unit == "bit") {
            r.value = juce::String(val, 1) + " bit";
        } else {
            r.value = juce::String(val, 2);
        }
        r.blurb = blurb;
        r.accent = col;
        r.isDefault = false;
        headerInspector_.setReadout(r);
    };

    item.slider.onValueChange = [sendTelemetry, &item]() {
        sendTelemetry(item.slider.getValue());
    };

    item.slider.addMouseListener(this, false);
}

void FormlessSpawnAudioProcessorEditor::paint(juce::Graphics& g) {
    // 1. Master Chassis Background
    g.fillAll(juce::Colour(0xFF0D1117));

    // 2. Header Row 1 Wordmark
    g.setFont(juce::Font(juce::FontOptions(15.0f).withStyle("Bold")));
    g.setColour(juce::Colour(0xFFFFAA00));
    g.drawText("FORMLESS SPAWN", 16, 8, 150, 28, juce::Justification::centredLeft);

    // 3. Card 0 Chassis: Voice Navigator & Model Matrix
    auto card0 = juce::Rectangle<float>(20.0f, 44.0f, 860.0f, 68.0f);
    g.setColour(juce::Colour(0xFF13171F));
    g.fillRoundedRectangle(card0, 6.0f);
    g.setColour(juce::Colour(0xFF262D38));
    g.drawRoundedRectangle(card0, 6.0f, 1.0f);

    // Role / Model Divider
    g.setColour(juce::Colour(0xFF21262D));
    g.drawVerticalLine(274, card0.getY() + 6.0f, card0.getBottom() - 6.0f);

    // 4. Card 1 Chassis: Drum Core & Pitch Lab
    auto card1 = juce::Rectangle<float>(20.0f, 124.0f, 420.0f, 264.0f);
    g.setColour(juce::Colour(0xFF13171F));
    g.fillRoundedRectangle(card1, 6.0f);
    g.setColour(juce::Colour(0xFF262D38));
    g.drawRoundedRectangle(card1, 6.0f, 1.0f);

    auto pill1 = juce::Rectangle<float>(card1.getX() + 10.0f, card1.getY() + 8.0f, 210.0f, 18.0f);
    g.setColour(juce::Colour(0xFF1F242E));
    g.fillRoundedRectangle(pill1, 3.0f);
    g.setFont(juce::Font(juce::FontOptions(10.5f).withStyle("Bold")));
    g.setColour(juce::Colour(0xFF00C8FF));
    g.drawText("CARD 1: DRUM CORE & PITCH LAB", pill1, juce::Justification::centred);

    // 5. Card 2 Chassis: Model Timbre & Acoustic Lab
    auto card2 = juce::Rectangle<float>(460.0f, 124.0f, 420.0f, 264.0f);
    g.setColour(juce::Colour(0xFF13171F));
    g.fillRoundedRectangle(card2, 6.0f);
    g.setColour(juce::Colour(0xFF262D38));
    g.drawRoundedRectangle(card2, 6.0f, 1.0f);

    auto pill2 = juce::Rectangle<float>(card2.getX() + 10.0f, card2.getY() + 8.0f, 230.0f, 18.0f);
    g.setColour(juce::Colour(0xFF1F242E));
    g.fillRoundedRectangle(pill2, 3.0f);
    g.setFont(juce::Font(juce::FontOptions(10.5f).withStyle("Bold")));
    g.setColour(juce::Colour(0xFFE5A91E));
    g.drawText("CARD 2: MODEL TIMBRE & ACOUSTIC LAB", pill2, juce::Justification::centred);

    // 6. Card 3 Panoramic Deck
    auto card3 = juce::Rectangle<float>(20.0f, 400.0f, 860.0f, 264.0f);
    g.setColour(juce::Colour(0xFF13171F));
    g.fillRoundedRectangle(card3, 6.0f);
    g.setColour(juce::Colour(0xFF262D38));
    g.drawRoundedRectangle(card3, 6.0f, 1.0f);

    // Pod 1: Stepped Random Pod
    auto pod1 = juce::Rectangle<float>(28.0f, 408.0f, 300.0f, 248.0f);
    g.setColour(juce::Colour(0xFF0E121A));
    g.fillRoundedRectangle(pod1, 5.0f);
    g.setColour(juce::Colour(0xFF1F2633));
    g.drawRoundedRectangle(pod1, 5.0f, 1.0f);

    auto pillPod1 = juce::Rectangle<float>(pod1.getX() + 8.0f, pod1.getY() + 6.0f, 150.0f, 16.0f);
    g.setColour(juce::Colour(0xFF181C26));
    g.fillRoundedRectangle(pillPod1, 3.0f);
    g.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
    g.setColour(juce::Colour(0xFFB374FF));
    g.drawText("STEPPED RANDOM (HASTUR)", pillPod1, juce::Justification::centred);

    // Pod 2: The Performance Altar
    auto pod2 = juce::Rectangle<float>(338.0f, 408.0f, 200.0f, 248.0f);
    g.setColour(juce::Colour(0xFF0E121A));
    g.fillRoundedRectangle(pod2, 5.0f);
    g.setColour(juce::Colour(0xFF1F2633));
    g.drawRoundedRectangle(pod2, 5.0f, 1.0f);

    auto pillPod2 = juce::Rectangle<float>(pod2.getX() + 8.0f, pod2.getY() + 6.0f, 184.0f, 16.0f);
    g.setColour(juce::Colour(0xFF181C26));
    g.fillRoundedRectangle(pillPod2, 3.0f);
    g.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
    g.setColour(juce::Colour(0xFFFF9D00));
    g.drawText("THE PERFORMANCE ALTAR", pillPod2, juce::Justification::centred);

    // Pod 3: VCF & Mojo Strip Pod
    auto pod3 = juce::Rectangle<float>(548.0f, 408.0f, 324.0f, 248.0f);
    g.setColour(juce::Colour(0xFF0E121A));
    g.fillRoundedRectangle(pod3, 5.0f);
    g.setColour(juce::Colour(0xFF1F2633));
    g.drawRoundedRectangle(pod3, 5.0f, 1.0f);

    auto pillPod3 = juce::Rectangle<float>(pod3.getX() + 8.0f, pod3.getY() + 6.0f, 180.0f, 16.0f);
    g.setColour(juce::Colour(0xFF181C26));
    g.fillRoundedRectangle(pillPod3, 3.0f);
    g.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
    g.setColour(juce::Colour(0xFFFF5252));
    g.drawText("VCF & MOJO STRIP (100Hz HP)", pillPod3, juce::Justification::centred);
}

void FormlessSpawnAudioProcessorEditor::resized() {
    // =========================================================================
    // 1. Header Row 1 Geometry
    // =========================================================================
    presetStub_.setBounds(174, 8, 160, 28);
    headerInspector_.setBounds(342, 8, 340, 28);
    lifeformRocker_.setBounds(690, 8, 194, 28);

    // =========================================================================
    // 2. Card 0 Geometry: Voice Navigator & Model Matrix
    // =========================================================================
    roleGrid_.setBounds(28, 52, 236, 52);
    modelGrid_.setBounds(282, 52, 588, 52);

    // =========================================================================
    // 3. Card 1 Geometry: Drum Core & Pitch Lab (w=420, y=124..388)
    // =========================================================================
    const int c1X = 20;
    const int c1Y = 124;
    const int c1Cols[4] = { c1X + 26, c1X + 124, c1X + 222, c1X + 320 };

    // Row 1: PITCH, P-DEPTH, P-DECAY, P-CURVE
    pitchKnob_.label.setBounds(c1Cols[0] - 6, c1Y + 34, 70, 14);
    pitchKnob_.slider.setBounds(c1Cols[0], c1Y + 50, 58, 58);

    pitchDepthKnob_.label.setBounds(c1Cols[1] - 6, c1Y + 34, 70, 14);
    pitchDepthKnob_.slider.setBounds(c1Cols[1], c1Y + 50, 58, 58);

    pitchDecayKnob_.label.setBounds(c1Cols[2] - 6, c1Y + 34, 70, 14);
    pitchDecayKnob_.slider.setBounds(c1Cols[2], c1Y + 50, 58, 58);

    pitchCurveKnob_.label.setBounds(c1Cols[3] - 6, c1Y + 34, 70, 14);
    pitchCurveKnob_.slider.setBounds(c1Cols[3], c1Y + 50, 58, 58);

    // Row 2: CLK-TUNE, CLK-MIX, SUB-LVL, LOW-BST
    clickTuneKnob_.label.setBounds(c1Cols[0] - 6, c1Y + 140, 70, 14);
    clickTuneKnob_.slider.setBounds(c1Cols[0], c1Y + 156, 58, 58);

    clickMixKnob_.label.setBounds(c1Cols[1] - 6, c1Y + 140, 70, 14);
    clickMixKnob_.slider.setBounds(c1Cols[1], c1Y + 156, 58, 58);

    subLevelKnob_.label.setBounds(c1Cols[2] - 6, c1Y + 140, 70, 14);
    subLevelKnob_.slider.setBounds(c1Cols[2], c1Y + 156, 58, 58);

    lowBoostKnob_.label.setBounds(c1Cols[3] - 6, c1Y + 140, 70, 14);
    lowBoostKnob_.slider.setBounds(c1Cols[3], c1Y + 156, 58, 58);

    // =========================================================================
    // 4. Card 2 Geometry: Model Timbre & Acoustic Lab (w=420, y=124..388)
    // =========================================================================
    const int c2X = 460;
    const int c2Y = 124;
    const int c2Cols[4] = { c2X + 26, c2X + 124, c2X + 222, c2X + 320 };

    // Row 1: TIMBRE-X, TIMBRE-Y, TIMBRE-Z, TIMBRE-T
    timbreXKnob_.label.setBounds(c2Cols[0] - 6, c2Y + 34, 70, 14);
    timbreXKnob_.slider.setBounds(c2Cols[0], c2Y + 50, 58, 58);

    timbreYKnob_.label.setBounds(c2Cols[1] - 6, c2Y + 34, 70, 14);
    timbreYKnob_.slider.setBounds(c2Cols[1], c2Y + 50, 58, 58);

    timbreZKnob_.label.setBounds(c2Cols[2] - 6, c2Y + 34, 70, 14);
    timbreZKnob_.slider.setBounds(c2Cols[2], c2Y + 50, 58, 58);

    timbreTKnob_.label.setBounds(c2Cols[3] - 6, c2Y + 34, 70, 14);
    timbreTKnob_.slider.setBounds(c2Cols[3], c2Y + 50, 58, 58);

    // Row 2: FM-MORPH, DAMP, PAN, VEL-SENS
    fmMorphKnob_.label.setBounds(c2Cols[0] - 6, c2Y + 140, 70, 14);
    fmMorphKnob_.slider.setBounds(c2Cols[0], c2Y + 156, 58, 58);

    dampKnob_.label.setBounds(c2Cols[1] - 6, c2Y + 140, 70, 14);
    dampKnob_.slider.setBounds(c2Cols[1], c2Y + 156, 58, 58);

    panKnob_.label.setBounds(c2Cols[2] - 6, c2Y + 140, 70, 14);
    panKnob_.slider.setBounds(c2Cols[2], c2Y + 156, 58, 58);

    velSensKnob_.label.setBounds(c2Cols[3] - 6, c2Y + 140, 70, 14);
    velSensKnob_.slider.setBounds(c2Cols[3], c2Y + 156, 58, 58);

    // =========================================================================
    // 5. Card 3 Pod 1: Stepped Random (w=300, y=408..656)
    // =========================================================================
    const int pod1X = 28;
    const int pod1Y = 408;

    // Row 1: R-PITCH, R-DECAY
    randPitchKnob_.label.setBounds(pod1X + 44, pod1Y + 28, 64, 14);
    randPitchKnob_.slider.setBounds(pod1X + 47, pod1Y + 44, 58, 58);

    randDecayKnob_.label.setBounds(pod1X + 184, pod1Y + 28, 64, 14);
    randDecayKnob_.slider.setBounds(pod1X + 187, pod1Y + 44, 58, 58);

    // Row 2: R-TIMBRE, R-SCAT
    randTimbreKnob_.label.setBounds(pod1X + 44, pod1Y + 116, 64, 14);
    randTimbreKnob_.slider.setBounds(pod1X + 47, pod1Y + 132, 58, 58);

    randScatterKnob_.label.setBounds(pod1X + 184, pod1Y + 116, 64, 14);
    randScatterKnob_.slider.setBounds(pod1X + 187, pod1Y + 132, 58, 58);

    // Row 3: RAND MODE (FREE / LOOP16)
    randModeBar_.setBounds(pod1X + 30, pod1Y + 206, 240, 26);

    // =========================================================================
    // 6. Card 3 Pod 2: The Performance Altar (EXACTLY TWO CONTROLS)
    // =========================================================================
    const int pod2X = 338;
    const int pod2Y = 408;

    macroDecayKnob_.label.setBounds(pod2X, pod2Y + 24, 200, 16);
    macroDecayKnob_.slider.setBounds(pod2X + 48, pod2Y + 44, 104, 104);

    trigButton_.setBounds(pod2X + 55, pod2Y + 186, 90, 36);

    // =========================================================================
    // 7. Card 3 Pod 3: VCF, Mojo Strip & Master Output (w=324, y=408..656)
    // =========================================================================
    const int pod3X = 548;
    const int pod3Y = 408;

    // Row 1: CUTOFF (Hero 68), RES (56), VCF MODE (Vert 62x50), DRIVE (56)
    cutoffKnob_.label.setBounds(pod3X + 10, pod3Y + 22, 68, 14);
    cutoffKnob_.slider.setBounds(pod3X + 10, pod3Y + 38, 68, 68);

    resonanceKnob_.label.setBounds(pod3X + 88, pod3Y + 28, 56, 14);
    resonanceKnob_.slider.setBounds(pod3X + 88, pod3Y + 44, 56, 56);

    vcfModeBar_.setBounds(pod3X + 156, pod3Y + 46, 62, 52);

    preDriveKnob_.label.setBounds(pod3X + 232, pod3Y + 28, 56, 14);
    preDriveKnob_.slider.setBounds(pod3X + 232, pod3Y + 44, 56, 56);

    // Row 2: CRUSH (56), [Spacer], BITS (56), TAPE (56)
    crushKnob_.label.setBounds(pod3X + 16, pod3Y + 116, 56, 14);
    crushKnob_.slider.setBounds(pod3X + 16, pod3Y + 132, 56, 56);

    bitsKnob_.label.setBounds(pod3X + 159, pod3Y + 116, 56, 14);
    bitsKnob_.slider.setBounds(pod3X + 159, pod3Y + 132, 56, 56);

    tapeGlueKnob_.label.setBounds(pod3X + 232, pod3Y + 116, 56, 14);
    tapeGlueKnob_.slider.setBounds(pod3X + 232, pod3Y + 132, 56, 56);

    // Row 3: MOJO-MIX (52), PROFILE 2x2 (118x48), VOLUME (Hero 60)
    mojoMixKnob_.label.setBounds(pod3X + 16, pod3Y + 196, 56, 12);
    mojoMixKnob_.slider.setBounds(pod3X + 18, pod3Y + 208, 52, 40);

    mojoProfileBar_.setBounds(pod3X + 88, pod3Y + 200, 118, 48);

    volumeKnob_.label.setBounds(pod3X + 230, pod3Y + 194, 60, 12);
    volumeKnob_.slider.setBounds(pod3X + 230, pod3Y + 206, 60, 44);
}
