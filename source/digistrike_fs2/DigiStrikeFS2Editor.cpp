#include "DigiStrikeFS2Editor.h"
#include "Digistrike_fs2ParameterIds.h"

DigiStrikeFS2AudioProcessorEditor::DigiStrikeFS2AudioProcessorEditor(DigiStrikeFS2AudioProcessor& p)
    : AudioProcessorEditor(&p), processor_(p)
{
    using namespace digistrike_fs2Parameters::Ids;

    setSize(kBaseWidth, kBaseHeight);
    setLookAndFeel(&lnf_);
    startTimerHz(30);

    auto& apvts = processor_.getAPVTS();

    // =========================================================================
    // 1. Header Row
    // =========================================================================
    categoryBox_.addItemList(juce::StringArray{"KICK", "SNAR", "TOMS", "CLAP", "HATS", "CYMB", "PERC", "STAB"}, 1);
    addAndMakeVisible(categoryBox_);
    categoryAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, category, categoryBox_);

    lineageBox_.addItemList(juce::StringArray{"ALL", "CROWN", "APEX", "EMPYR", "FALCON", "K-WRK", "TOKYO", "GOTHN", "NIPPON", "CURIOS"}, 1);
    addAndMakeVisible(lineageBox_);
    lineageAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, lineage, lineageBox_);

    voiceModeBox_.addItemList(juce::StringArray{"MONO", "GLIDE", "POLY"}, 1);
    addAndMakeVisible(voiceModeBox_);
    voiceModeAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, voice_mode, voiceModeBox_);

    addAndMakeVisible(headerInspector_);

    lifeformRocker_.onSelect = [&apvts](int idx) {
        if (auto* param = apvts.getParameter(life_mode))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
    };
    if (auto* param = apvts.getRawParameterValue(life_mode))
        lifeformRocker_.setState(static_cast<int>(param->load()));
    addAndMakeVisible(lifeformRocker_);

    // =========================================================================
    // 2. Card 1 Knobs: Master Macro & Dynamics (Top-Left, 8)
    // 6-Domain Chromatic Standard: Movie Poster Orange (0xFFFF7B25)
    // =========================================================================
    const auto orangeCol = juce::Colour(0xFFFF7B25);
    setupKnob(macroXKnob_,     macro_x,     "X: SPECTRAL", "%", "macro spectral spread, sweep excursion and harmonics", orangeCol);
    setupKnob(macroYKnob_,     macro_y,     "Y: PUNCH",    "%", "macro transient attack spike and punch snap",         orangeCol);
    setupKnob(macroZKnob_,     macro_z,     "Z: TIMBRE",   "%", "macro harmonic waveshaping, fold & overtone",         orangeCol);
    setupKnob(macroTKnob_,     macro_t,     "T: TIME",     "%", "macro pitch envelope and body decay time",            orangeCol);
    setupKnob(velAggrKnob_,    vel_aggr,    "VEL->AGGR",   "%", "velocity to synthesis aggression & overdrive",       orangeCol);
    setupKnob(velTimbKnob_,    vel_timb,    "VEL->TIMB",   "%", "velocity to synthesis brightness & overtones",       orangeCol);
    setupKnob(panKnob_,        pan,         "PAN",         "%", "stereo panorama positioning",                        orangeCol, true);
    setupKnob(decayCurveKnob_, decay_curve, "DECAY CRV",   "",  "envelope discharge curve (exp to log)",              orangeCol, true);

    // =========================================================================
    // 3. Card 2 Knobs: Uniform Sound Lab (Top-Right, 8)
    // 6-Domain Chromatic Standard: Movie Poster Blue (0xFF38BDF8)
    // =========================================================================
    const auto blueCol = juce::Colour(0xFF38BDF8);
    setupKnob(pitchDecayKnob_, pitch_decay,   "P.DEC",   "ms", "pitch envelope decay duration (2ms..250ms)", blueCol);
    setupKnob(clickKnob_,      click_bleed,   "CLICK",   "%",  "transient crack & noise bleed level",        blueCol);
    setupKnob(startKnob_,      start_offset,  "START",   "ms", "initial sample playback offset (0..500ms)",   blueCol);
    setupKnob(timeKnob_,       traverse_rate, "TIME",    "x",  "varispeed / reverse traversal rate (-4x..+4x)", blueCol, true);
    setupKnob(punchKnob_,      pitch_dive,    "PUNCH",   "%",  "pitch dive depth (0%..100% => 0..36 semitones)", blueCol);
    setupKnob(compandKnob_,    dac_compand,   "COMPAND", "%",  "vintage DAC companding and bit ladder",      blueCol);
    setupKnob(lengthKnob_,     sample_length, "LENGTH",  "%",  "sample playback window / scan span",         blueCol);
    setupKnob(morphKnob_,      table_morph,   "MORPH",   "%",  "3D wavetable slice tilt and morph",          blueCol);

    // =========================================================================
    // 4. Card 3 Pod 1: Stepped Random (Bottom-Left, 8)
    // 6-Domain Chromatic Standard: Caustic Hastur Yellow (0xFFF2E10F)
    // =========================================================================
    const auto yellowCol = juce::Colour(0xFFF2E10F);
    setupKnob(randXKnob_,     rand_x,     "R-X",     "%", "per-hit Macro X S&H random depth",             yellowCol);
    setupKnob(randYKnob_,     rand_y,     "R-Y",     "%", "per-hit Macro Y S&H random depth",             yellowCol);
    setupKnob(randZKnob_,     rand_z,     "R-Z",     "%", "per-hit Macro Z S&H random depth",             yellowCol);
    setupKnob(randTKnob_,     rand_t,     "R-T",     "%", "per-hit Macro T S&H random depth",             yellowCol);
    setupKnob(randAggrKnob_,  rand_aggr,  "R-AGGR",  "%", "per-hit velocity aggression fluctuation depth", yellowCol);
    setupKnob(randTimbKnob_,  rand_timb,  "R-TIMB",  "%", "per-hit velocity timbre fluctuation depth",     yellowCol);
    setupKnob(randPanKnob_,   rand_pan,   "R-PAN",   "%", "per-hit stereo position modulation depth",      yellowCol);
    setupKnob(randDecayKnob_, rand_decay, "R-DECAY", "%", "per-hit amplitude decay modulation depth",     yellowCol);

    // =========================================================================
    // 5. Card 3 Pod 2: Performance Altar
    // =========================================================================
    setupKnob(macroDecayKnob_, macro_decay, "MACRO", "", "master exponential envelope decay scaling", yellowCol);
    macroDecayKnob_.slider.setDoubleClickReturnValue(true, 0.5);

    trigButton_.onTrigger = [this]() {
        processor_.triggerManual(0.95f);
    };
    addAndMakeVisible(trigButton_);

    // =========================================================================
    // 6. Card 3 Pod 3: VCF, Mojo & Output
    // =========================================================================
    // Row 0: CEM3320 ZDF VCF (Acid Green 0xFF00E676)
    const auto acidGreenCol = juce::Colour(0xFF00E676);
    setupKnob(cutoffKnob_,       cutoff,          "CUTOFF", "Hz", "Curtis CEM3320 filter cutoff frequency", acidGreenCol);
    setupKnob(resonanceKnob_,    resonance,       "RES",    "%",  "filter resonance / self-oscillation",    acidGreenCol);
    setupKnob(filterEnvAmtKnob_, filter_env_amt,  "FE-AMT", "%",  "filter envelope sweep amount (-100%..+100%)", acidGreenCol, true);
    setupKnob(filterDecayKnob_,  filter_decay,    "FE-DEC", "ms", "filter envelope decay time (5ms..5.0s)",  acidGreenCol);
    setupKnob(filterDriveKnob_,  pre_drive,        "DRIVE",  "dB", "pre-filter analog saturation drive",     acidGreenCol);

    // Row 1: Universal Hardware Mojo Strip (Sorcerous Purple 0xFFBC8CFF)
    const auto purpleCol = juce::Colour(0xFFBC8CFF);
    setupKnob(crushKnob_,   crush,         "CRUSH",   "Hz", "mojo sample rate decimation frequency",      purpleCol);
    setupKnob(shiftKnob_,   shift,         "SHIFT",   "Hz", "bode frequency shifter (+/-1000Hz)",         purpleCol, true);
    setupKnob(squashKnob_,  squash,        "SQUASH",  "%",  "bipolar dynamics: snappy vs huge",           purpleCol, true);
    setupKnob(widerKnob_,   wider,         "WIDER",   "%",  "mono-compatible stereo spatial widener",     purpleCol);
    setupKnob(sustainKnob_, sustain_level, "SUSTAIN", "%",  "gated hold level (0%=one-shot, >2%=synth)",  purpleCol);

    // Master Column: Limiter button, Volume knob, Amp Decay knob (Blistering Red 0xFFFF3B30)
    const auto redCol = juce::Colour(0xFFFF3B30);
    limiterAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, master_limiter, limiterButton_);
    addAndMakeVisible(limiterButton_);

    setupKnob(volumeKnob_,   volume,    "VOLUME",  "dB", "master output level into safety limiter", redCol);
    volumeKnob_.slider.setDoubleClickReturnValue(true, 0.5);

    setupKnob(ampDecayKnob_, amp_decay, "AMP-DEC", "s",  "master dynamic amp decay & gated release", redCol);
    ampDecayKnob_.slider.setDoubleClickReturnValue(true, 0.500);
}

DigiStrikeFS2AudioProcessorEditor::~DigiStrikeFS2AudioProcessorEditor() {
    stopTimer();
    setLookAndFeel(nullptr);
}

void DigiStrikeFS2AudioProcessorEditor::setupKnob(KnobItem& item, const juce::String& paramId,
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
    item.label.setBorderSize(juce::BorderSize<int>(0));
    item.label.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(item.label);

    item.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor_.getAPVTS(), paramId, item.slider);

    item.slider.textFromValueFunction = [name, unit](double val) -> juce::String {
        if (name == "VOLUME") {
            const float u = static_cast<float>(val);
            if (u <= 0.0001f) return "-inf dB";
            float db = (u <= 0.5f) ? (-60.0f + (u / 0.5f) * 60.0f) : ((u - 0.5f) / 0.5f * 12.0f);
            return (db > 0.0f ? "+" : "") + juce::String(db, 1) + " dB";
        }
        if (name == "AMP-DEC" || name == "FE-DEC") {
            if (val >= 1.0) return juce::String(val, 2) + " s";
            return juce::String(juce::roundToInt(val * 1000.0)) + " ms";
        }
        if (name == "TIME") {
            if (std::abs(val) <= 0.05) return "FREEZE";
            if (val < 0.0) return "REV " + juce::String(-val, 2) + "x";
            return "+" + juce::String(val, 2) + "x";
        }
        if (name == "SUSTAIN") {
            if (val <= 2.0) return "1-SHOT";
            return "GATE " + juce::String(juce::roundToInt(val)) + "%";
        }
        if (name == "PUNCH") {
            return juce::String(juce::roundToInt(val * 0.36f)) + " st";
        }
        if (unit == "Hz") {
            if (val >= 1000.0) return juce::String(val * 0.001, 1) + " kHz";
            return juce::String(juce::roundToInt(val)) + " Hz";
        }
        if (unit == "ms") return juce::String(val, 1) + " ms";
        if (unit == "dB") return (val > 0.0 ? "+" : "") + juce::String(val, 1) + " dB";
        if (unit == "%") return juce::String(juce::roundToInt(val)) + "%";
        return juce::String(val, 2);
    };

    auto sendTelemetry = [this, &item, blurb, col](double val) {
        DigiStrikeHeaderInspector::Readout r;
        r.name = item.label.getText();
        r.value = item.slider.textFromValueFunction ? item.slider.textFromValueFunction(val) : juce::String(val, 2);
        r.blurb = blurb;
        r.accent = col;
        r.isDefault = false;
        headerInspector_.setReadout(r);
    };

    item.slider.onValueChange = [sendTelemetry, &item]() {
        sendTelemetry(item.slider.getValue());
    };
}

void DigiStrikeFS2AudioProcessorEditor::timerCallback() {
    using namespace digistrike_fs2Parameters::Ids;
    if (auto* param = processor_.getAPVTS().getRawParameterValue(life_mode)) {
        lifeformRocker_.setState(static_cast<int>(param->load()));
    }
}

void DigiStrikeFS2AudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xFF0D1117));

    // Header Title (Matching Formless Spawn brand title placement)
    g.setFont(juce::Font(juce::FontOptions(15.0f).withStyle("Bold")));
    g.setColour(juce::Colour(0xFFFFAA00));
    g.drawText("DIGISTRIKE FS2", 20, 8, 130, 28, juce::Justification::centredLeft);

    auto drawCard = [&g](juce::Rectangle<int> r) {
        g.setColour(juce::Colour(0xFF13171F));
        g.fillRoundedRectangle(r.toFloat(), 6.0f);
        g.setColour(juce::Colour(0xFF262D38));
        g.drawRoundedRectangle(r.toFloat().reduced(0.5f), 6.0f, 1.0f);
    };

    auto drawCardPill = [&g](juce::Rectangle<int> card, int w, const char* text, juce::Colour accent) {
        auto pill = juce::Rectangle<float>((float)card.getX() + 10.0f, (float)card.getY() + 6.0f, (float)w, 16.0f);
        g.setColour(juce::Colour(0xFF1F242E));
        g.fillRoundedRectangle(pill, 3.0f);
        g.setFont(juce::Font(juce::FontOptions(10.5f).withStyle("Bold")));
        g.setColour(accent);
        g.drawText(text, pill, juce::Justification::centred);
    };

    auto drawPod = [&g](juce::Rectangle<int> pod, int pillW, const char* text, juce::Colour accent) {
        g.setColour(juce::Colour(0xFF0E121A));
        g.fillRoundedRectangle(pod.toFloat(), 5.0f);
        g.setColour(juce::Colour(0xFF1F2633));
        g.drawRoundedRectangle(pod.toFloat().reduced(0.5f), 5.0f, 1.0f);

        auto pill = juce::Rectangle<float>((float)pod.getX() + 8.0f, (float)pod.getY() + 5.0f, (float)pillW, 15.0f);
        g.setColour(juce::Colour(0xFF181C26));
        g.fillRoundedRectangle(pill, 3.0f);
        g.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
        g.setColour(accent);
        g.drawText(text, pill, juce::Justification::centred);
    };

    // Cards 1 & 2
    const juce::Rectangle<int> kCard1{20, 44, 424, 258};
    const juce::Rectangle<int> kCard2{456, 44, 424, 258};
    drawCard(kCard1);
    drawCardPill(kCard1, 240, "CARD 1: MASTER MACRO & DYNAMICS", juce::Colour(0xFF00C8FF));
    drawCard(kCard2);
    drawCardPill(kCard2, 220, "CARD 2: UNIFORM SOUND LAB", juce::Colour(0xFFFFAA00));

    // Card 3 + Pods
    const juce::Rectangle<int> kCard3{20, 312, 860, 232};
    const juce::Rectangle<int> kPod1{28, 320, 408, 216};
    const juce::Rectangle<int> kPod2{444, 320, 92, 216};
    const juce::Rectangle<int> kPod3{544, 320, 328, 216};

    drawCard(kCard3);
    drawPod(kPod1, 190, "STEPPED RANDOM HUMANIZER", juce::Colour(0xFFB374FF));
    drawPod(kPod2, kPod2.getWidth() - 16, "ALTAR", juce::Colour(0xFFFF9D00));
    drawPod(kPod3, 190, "VCF / MOJO / OUTPUT", juce::Colour(0xFFFF5252));

    // Pod 3 Internals: Mojo Row separator & Master-Column divider
    const int p3y = kPod3.getY();
    g.setColour(juce::Colour(0xFF1F2633));
    g.drawHorizontalLine(p3y + 104, (float)kPod3.getX() + 8.0f, (float)kPod3.getX() + 252.0f);
    g.drawVerticalLine(kPod3.getX() + 258, (float)p3y + 24.0f, (float)kPod3.getBottom() - 8.0f);
}

void DigiStrikeFS2AudioProcessorEditor::resized() {
    // 1. Header Layout
    categoryBox_.setBounds(154, 8, 80, 28);
    lineageBox_.setBounds(238, 8, 84, 28);
    voiceModeBox_.setBounds(326, 8, 76, 28);
    headerInspector_.setBounds(406, 8, 280, 28);
    lifeformRocker_.setBounds(690, 8, 190, 28);

    // Helper: place label directly above slider
    auto place = [](KnobItem& k, int cx, int labelY, int labelW, int size, int sliderY) {
        k.label.setBounds(cx - labelW / 2, labelY, labelW, 14);
        k.slider.setBounds(cx - size / 2, sliderY, size, size);
    };

    auto colCentre = [](int cardX, int col, int cellPitch) {
        return cardX + 16 + cellPitch / 2 + col * cellPitch;
    };

    // Cards 1 & 2: 2x4 grid, 56px knobs, 98px cells
    const int kCard1X = 20;
    const int kCard2X = 456;
    const int kCellPitch = 98;

    KnobItem* c1[2][4] = {
        {&macroXKnob_,  &macroYKnob_,  &macroZKnob_, &macroTKnob_},
        {&velAggrKnob_, &velTimbKnob_, &panKnob_,    &decayCurveKnob_}
    };

    KnobItem* c2[2][4] = {
        {&pitchDecayKnob_, &clickKnob_,   &startKnob_,  &timeKnob_},
        {&punchKnob_,      &compandKnob_, &lengthKnob_, &morphKnob_}
    };

    for (int r = 0; r < 2; ++r) {
        const int labelY = (r == 0) ? 72 : 166;
        const int sliderY = labelY + 16;
        for (int c = 0; c < 4; ++c) {
            place(*c1[r][c], colCentre(kCard1X, c, kCellPitch), labelY, kCellPitch, 56, sliderY);
            place(*c2[r][c], colCentre(kCard2X, c, kCellPitch), labelY, kCellPitch, 56, sliderY);
        }
    }

    // Card 3 Pod 1: Stepped Random (2x4 grid, 48px knobs, 96px pitch)
    const int kPod1X = 28;
    const int kRandPitch = 96;
    KnobItem* p1[2][4] = {
        {&randXKnob_,    &randYKnob_,    &randZKnob_,   &randTKnob_},
        {&randAggrKnob_, &randTimbKnob_, &randPanKnob_, &randDecayKnob_}
    };

    for (int r = 0; r < 2; ++r) {
        const int labelY = (r == 0) ? 346 : 434;
        const int sliderY = labelY + 15;
        for (int c = 0; c < 4; ++c) {
            place(*p1[r][c], colCentre(kPod1X, c, kRandPitch), labelY, kRandPitch, 48, sliderY);
        }
    }

    // Card 3 Pod 2: Performance Altar
    {
        const int cx = 444 + 92 / 2; // 490
        place(macroDecayKnob_, cx, 340, 88, 72, 356);
        trigButton_.setBounds(cx - 36, 452, 72, 48);
    }

    // Card 3 Pod 3: VCF, Mojo & Output
    {
        const int x0 = 552;
        const int pitch = 48;
        const int labelY0 = 346;
        const int sliderY0 = labelY0 + 15;
        const int labelY1 = 434;
        const int sliderY1 = labelY1 + 15;

        // Row 0 (VCF Strip): CUTOFF, RES, FE-AMT, FE-DEC, DRIVE
        place(cutoffKnob_,       x0 + pitch / 2 + pitch * 0, labelY0, pitch, 44, sliderY0);
        place(resonanceKnob_,    x0 + pitch / 2 + pitch * 1, labelY0, pitch, 44, sliderY0);
        place(filterEnvAmtKnob_, x0 + pitch / 2 + pitch * 2, labelY0, pitch, 44, sliderY0);
        place(filterDecayKnob_,  x0 + pitch / 2 + pitch * 3, labelY0, pitch, 44, sliderY0);
        place(filterDriveKnob_,  x0 + pitch / 2 + pitch * 4, labelY0, pitch, 44, sliderY0);

        // Row 1 (Mojo Strip): CRUSH, SHIFT, SQUASH, WIDER, SUSTAIN
        place(crushKnob_,   x0 + pitch / 2 + pitch * 0, labelY1, pitch, 44, sliderY1);
        place(shiftKnob_,   x0 + pitch / 2 + pitch * 1, labelY1, pitch, 44, sliderY1);
        place(squashKnob_,  x0 + pitch / 2 + pitch * 2, labelY1, pitch, 44, sliderY1);
        place(widerKnob_,   x0 + pitch / 2 + pitch * 3, labelY1, pitch, 44, sliderY1);
        place(sustainKnob_, x0 + pitch / 2 + pitch * 4, labelY1, pitch, 44, sliderY1);

        // Master Column: LIMIT button, VOLUME, AMP-DEC
        limiterButton_.setBounds(814, 328, 48, 20);
        place(volumeKnob_,   838, 356, 52, 46, 372);
        place(ampDecayKnob_, 838, 440, 52, 46, 456);
    }
}
