#include "FormlessEditor.h"
#include "formless_spawn/formless_spawnParameterIds.h"

FormlessSpawnAudioProcessorEditor::FormlessSpawnAudioProcessorEditor(FormlessSpawnAudioProcessor& p)
    : AudioProcessorEditor(&p), processor_(p)
{
    using namespace formless_spawnParameters::Ids;

    setSize(900, 560);
    setLookAndFeel(&lnf_);
    startTimerHz(30);

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
        updateRoleParamLabels(roleIdx);
    };
    roleGrid_.onHover = [this](int roleIdx) {
        HeaderInspector::Readout r;
        r.name = "ROLE: " + juce::String(tkdsp::FormlessSpawnVoice::kRoleNames[roleIdx]);
        r.value = "SEL";
        static const char* kRoleDescs[8] = {
            "Bass Drum / Sub thump, acoustic punch and analog kicks",
            "Snare Drum / Snappy wire, tonal shell and acoustic crack",
            "Tom / Resonant cylindrical shell and dynamic pitch dive toms",
            "Handclap / Multi-burst acoustic and electronic claps",
            "Hi-Hat / Inharmonic metallic cluster and sizzle hats",
            "Cymbal / Crash, ride, bell and atmospheric metal washes",
            "Percussion / Rimshot, cowbell, clave, conga, anvil & metallic scrap",
            "Special FX / Experimental FM warp, wavefold, laser dives and chaos"
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
    // 3. Setup Card 1 Knobs: Master Macro & Dynamics Lab (Top-Left, 8)
    // =========================================================================
    setupKnob(macroXKnob_,     macro_x,     "X: PITCH",  "%", "macro pitch, sweep excursion and root tuning", juce::Colour(0xFF00C8FF));
    setupKnob(macroYKnob_,     macro_y,     "Y: PUNCH",  "%", "macro transient attack spike and punch snap",  juce::Colour(0xFF00C8FF));
    setupKnob(macroZKnob_,     macro_z,     "Z: TIMBRE", "%", "macro harmonic waveshaping, fold & overtone",  juce::Colour(0xFF00C8FF));
    setupKnob(macroTKnob_,     macro_t,     "T: TIME",   "%", "macro pitch envelope and body decay time",     juce::Colour(0xFF00C8FF));
    setupKnob(velAggrKnob_,    vel_aggr,    "VEL->AGGR", "%", "velocity to synthesis aggression & overdrive",juce::Colour(0xFF00C8FF));
    setupKnob(velTimbKnob_,    vel_timb,    "VEL->TIMB", "%", "velocity to synthesis brightness & overtones",juce::Colour(0xFF00C8FF));
    setupKnob(panKnob_,        pan,         "PAN",       "%", "stereo panorama positioning",                 juce::Colour(0xFF8B949E), true);
    setupKnob(decayCurveKnob_, decay_curve, "DECAY CRV", "",  "envelope discharge curve (exp to log)",       juce::Colour(0xFF8B949E), true);

    // =========================================================================
    // 4. Setup Card 2 Knobs: Role Synthesis Lab (Top-Right, 8)
    // =========================================================================
    setupKnob(paramAKnob_, param_a, "PITCH",   "%", "role synthesis parameter A", juce::Colour(0xFFFFAA00));
    setupKnob(paramBKnob_, param_b, "P.DEC",   "%", "role synthesis parameter B", juce::Colour(0xFFFFAA00));
    setupKnob(paramCKnob_, param_c, "P.DEPTH", "%", "role synthesis parameter C", juce::Colour(0xFFFFAA00));
    setupKnob(paramDKnob_, param_d, "DECAY",   "%", "role synthesis parameter D", juce::Colour(0xFFFFAA00));
    setupKnob(paramEKnob_, param_e, "CLICK",   "%", "role synthesis parameter E", juce::Colour(0xFFFFAA00));
    setupKnob(paramFKnob_, param_f, "TIMBRE",  "%", "role synthesis parameter F", juce::Colour(0xFFFFAA00));
    setupKnob(paramGKnob_, param_g, "DRIVE",   "%", "role synthesis parameter G", juce::Colour(0xFFFFAA00));
    setupKnob(paramHKnob_, param_h, "PUNCH",   "%", "role synthesis parameter H", juce::Colour(0xFFFFAA00));

    const int initialRole = apvts.getRawParameterValue(role) ? static_cast<int>(apvts.getRawParameterValue(role)->load()) : 0;
    updateRoleParamLabels(initialRole);

    // =========================================================================
    // 5. Setup Card 3 Pod 1: Stepped Random (2x4 Grid of 8 Small Knobs, Bottom-Left)
    // =========================================================================
    setupKnob(randXKnob_,     rand_x,     "R-X",     "%", "per-hit Macro X (Pitch) S&H random depth",     juce::Colour(0xFFB374FF));
    setupKnob(randYKnob_,     rand_y,     "R-Y",     "%", "per-hit Macro Y (Punch) S&H random depth",     juce::Colour(0xFFB374FF));
    setupKnob(randZKnob_,     rand_z,     "R-Z",     "%", "per-hit Macro Z (Timbre) S&H random depth",    juce::Colour(0xFFB374FF));
    setupKnob(randTKnob_,     rand_t,     "R-T",     "%", "per-hit Macro T (Time) S&H random depth",      juce::Colour(0xFFB374FF));
    setupKnob(randAggrKnob_,  rand_aggr,  "R-AGGR",  "%", "per-hit velocity aggression fluctuation depth",juce::Colour(0xFFB374FF));
    setupKnob(randTimbKnob_,  rand_timb,  "R-TIMB",  "%", "per-hit velocity timbre fluctuation depth",    juce::Colour(0xFFB374FF));
    setupKnob(randPanKnob_,   rand_pan,   "R-PAN",   "%", "per-hit stereo position modulation depth",     juce::Colour(0xFFB374FF));
    setupKnob(randDecayKnob_, rand_decay, "R-DECAY", "%", "per-hit Master Macro Decay modulation depth",  juce::Colour(0xFFB374FF));

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
    setupKnob(volumeKnob_,    volume,       "VOLUME",    "%",  "master output stage level",                  juce::Colour(0xFFF0F6FC));

    filterTypeBar_.setAccentColour(juce::Colour(0xFF6BBF59));
    filterTypeBar_.onSelect = [&apvts](int idx) {
        if (auto* param = apvts.getParameter(filter_type))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
    };
    if (auto* param = apvts.getRawParameterValue(filter_type))
        filterTypeBar_.setState(static_cast<int>(param->load()));
    addAndMakeVisible(filterTypeBar_);

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
    stopTimer();
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
    item.label.setBorderSize(juce::BorderSize<int>(0));
    item.label.setInterceptsMouseClicks(false, false);
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

namespace {
const juce::Rectangle<int> kCard1{20, 104, 420, 198};
const juce::Rectangle<int> kCard2{460, 104, 420, 198};
const juce::Rectangle<int> kCard3{20, 310, 860, 240};
const juce::Rectangle<int> kPod1{28, 318, 404, 224};   // kCard1 inset 8 -> columns align 1:1
const juce::Rectangle<int> kPod2{440, 318, 96, 224};
const juce::Rectangle<int> kPod3{544, 318, 328, 224};

constexpr int kCellPitch = 98;   // 4 equal cells per 420 card (14px side inset)
constexpr int kCellInset = 14;
constexpr int kPod3Divider = 804;

// Centre of column c (0..3) for a card whose left edge is cardX.
inline int colCentre(int cardX, int c) { return cardX + kCellInset + kCellPitch * c + kCellPitch / 2; }
} // namespace

void FormlessSpawnAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xFF0D1117));

    g.setFont(juce::Font(juce::FontOptions(15.0f).withStyle("Bold")));
    g.setColour(juce::Colour(0xFFFFAA00));
    g.drawText("FORMLESS SPAWN", 20, 8, 140, 28, juce::Justification::centredLeft);

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

    // Card 0: Voice Navigator
    auto card0 = juce::Rectangle<float>(20.0f, 42.0f, 860.0f, 54.0f);
    g.setColour(juce::Colour(0xFF13171F));
    g.fillRoundedRectangle(card0, 6.0f);
    g.setColour(juce::Colour(0xFF262D38));
    g.drawRoundedRectangle(card0.reduced(0.5f), 6.0f, 1.0f);
    g.setColour(juce::Colour(0xFF21262D));
    g.drawVerticalLine(274, card0.getY() + 4.0f, card0.getBottom() - 4.0f);

    // Cards 1 & 2
    drawCard(kCard1);
    drawCardPill(kCard1, 250, "CARD 1: MASTER MACROS & DYNAMICS", juce::Colour(0xFF00C8FF));
    drawCard(kCard2);
    drawCardPill(kCard2, 240, "CARD 2: ROLE SYNTHESIS LAB (A..H)", juce::Colour(0xFFFFAA00));

    // Card 3 + pods
    drawCard(kCard3);
    drawPod(kPod1, 180, "STEPPED RANDOM HUMANIZER", juce::Colour(0xFFB374FF));
    drawPod(kPod2, kPod2.getWidth() - 16, "ALTAR", juce::Colour(0xFFFF9D00));
    drawPod(kPod3, 180, "VCF / MOJO / OUTPUT", juce::Colour(0xFFFF5252));

    // Pod 3 internals: VCF/Mojo row separator and master-column divider
    const int p3y = kPod3.getY();
    g.setColour(juce::Colour(0xFF1F2633));
    g.drawHorizontalLine(p3y + 113, (float)kPod3.getX() + 8.0f, 796.0f);
    g.drawVerticalLine((float)kPod3Divider, (float)p3y + 26.0f, (float)kPod3.getBottom() - 8.0f);
}

void FormlessSpawnAudioProcessorEditor::resized() {
    // Header: right edge lands on 880 (matches card right edge)
    presetStub_.setBounds(174, 8, 145, 28);
    headerInspector_.setBounds(326, 8, 354, 28);
    lifeformRocker_.setBounds(686, 8, 194, 28);

    // Card 0
    roleGrid_.setBounds(28, 46, 236, 48);
    modelGrid_.setBounds(282, 46, 588, 48);

    // Label sits directly above its slider (label h=14, centred on cx).
    auto place = [](KnobItem& k, int cx, int labelY, int labelW, int size, int sliderY) {
        k.label.setBounds(cx - labelW / 2, labelY, labelW, 14);
        k.slider.setBounds(cx - size / 2, sliderY, size, size);
    };

    // Cards 1 & 2: 2x4 grid, 56px knobs, 98px cells
    KnobItem* c1[2][4] = {
        {&macroXKnob_,    &macroYKnob_,    &macroZKnob_,  &macroTKnob_},
        {&velAggrKnob_,   &velTimbKnob_,   &panKnob_,     &decayCurveKnob_}};
    KnobItem* c2[2][4] = {
        {&paramAKnob_, &paramBKnob_, &paramCKnob_, &paramDKnob_},
        {&paramEKnob_, &paramFKnob_, &paramGKnob_, &paramHKnob_}};
    for (int r = 0; r < 2; ++r) {
        const int labelY = (r == 0) ? 30 : 114;
        for (int c = 0; c < 4; ++c) {
            place(*c1[r][c], colCentre(kCard1.getX(), c), kCard1.getY() + labelY, kCellPitch, 56, kCard1.getY() + labelY + 14);
            place(*c2[r][c], colCentre(kCard2.getX(), c), kCard2.getY() + labelY, kCellPitch, 56, kCard2.getY() + labelY + 14);
        }
    }

    // Pod 1: identical column centres to Card 1 (shared colCentre) -> exact 1:1 alignment
    KnobItem* p1[2][4] = {
        {&randXKnob_,    &randYKnob_,    &randZKnob_,   &randTKnob_},
        {&randAggrKnob_, &randTimbKnob_, &randPanKnob_, &randDecayKnob_}};
    for (int r = 0; r < 2; ++r) {
        const int labelY = kPod1.getY() + ((r == 0) ? 32 : 128);
        for (int c = 0; c < 4; ++c)
            place(*p1[r][c], colCentre(kCard1.getX(), c), labelY, kCellPitch, 56, labelY + 14);
    }

    // Pod 2: Performance Altar (hero knob + strike pad on the pod centreline)
    {
        const int cx = kPod2.getCentreX();
        const int py = kPod2.getY();
        place(macroDecayKnob_, cx, py + 30, kPod2.getWidth(), 72, py + 44);
        trigButton_.setBounds(cx - 36, py + 146, 72, 48);
    }

    // Pod 3: [3 knobs | 2x2 grid] x 2 rows, then master column behind the divider
    {
        const int py = kPod3.getY();
        const int x0 = kPod3.getX() + 8;          // 552
        const int pitch = 50;
        const int gridX = x0 + 3 * pitch + 6;     // 708
        KnobItem* p3[2][3] = {
            {&cutoffKnob_, &resonanceKnob_, &preDriveKnob_},
            {&crushKnob_,  &bitsKnob_,      &tapeGlueKnob_}};
        FormlessGrid2x2* grids[2] = {&filterTypeBar_, &mojoProfileBar_};

        for (int r = 0; r < 2; ++r) {
            const int labelY = py + ((r == 0) ? 32 : 128);
            const int sliderY = labelY + 18;      // 48px knob centred on the Pod 1 row centre
            for (int c = 0; c < 3; ++c)
                place(*p3[r][c], x0 + pitch / 2 + pitch * c, labelY, pitch, 48, sliderY);
            grids[r]->setBounds(gridX, sliderY, 88, 48);
        }

        // Master column: x = 810..864, centre 837, vertically centred between the two rows
        place(volumeKnob_, kPod3.getRight() - 8 - 27, py + 88, 54, 54, py + 102);
    }
}

void FormlessSpawnAudioProcessorEditor::timerCallback() {
    auto updateProp = [](juce::Slider& s, const juce::Identifier& id, float val) {
        if (!s.getProperties().contains(id) || std::abs(static_cast<float>(s.getProperties()[id]) - val) > 0.001f) {
            s.getProperties().set(id, val);
            s.repaint();
        }
    };
    auto clearProp = [](juce::Slider& s) {
        if (s.getProperties().contains("randLo")) {
            s.getProperties().remove("randLo");
            s.getProperties().remove("randHi");
            s.getProperties().remove("randPip");
            s.repaint();
        }
    };

    // 1. Macro X (0..1)
    const float depthX = static_cast<float>(randXKnob_.slider.getValue()) * 0.01f;
    if (depthX > 0.001f) {
        const float baseVal = static_cast<float>(macroXKnob_.slider.getValue());
        const float lo = std::clamp(baseVal - depthX, 0.0f, 1.0f);
        const float hi = std::clamp(baseVal + depthX, 0.0f, 1.0f);
        const float pip = std::clamp(baseVal + processor_.getLatchedDraw(0) * depthX, 0.0f, 1.0f);
        updateProp(macroXKnob_.slider, "randLo", lo);
        updateProp(macroXKnob_.slider, "randHi", hi);
        updateProp(macroXKnob_.slider, "randPip", pip);
    } else {
        clearProp(macroXKnob_.slider);
    }

    // 2. Macro Y (0..1)
    const float depthY = static_cast<float>(randYKnob_.slider.getValue()) * 0.01f;
    if (depthY > 0.001f) {
        const float baseVal = static_cast<float>(macroYKnob_.slider.getValue());
        const float lo = std::clamp(baseVal - depthY, 0.0f, 1.0f);
        const float hi = std::clamp(baseVal + depthY, 0.0f, 1.0f);
        const float pip = std::clamp(baseVal + processor_.getLatchedDraw(1) * depthY, 0.0f, 1.0f);
        updateProp(macroYKnob_.slider, "randLo", lo);
        updateProp(macroYKnob_.slider, "randHi", hi);
        updateProp(macroYKnob_.slider, "randPip", pip);
    } else {
        clearProp(macroYKnob_.slider);
    }

    // 3. Macro Z (0..1)
    const float depthZ = static_cast<float>(randZKnob_.slider.getValue()) * 0.01f;
    if (depthZ > 0.001f) {
        const float baseVal = static_cast<float>(macroZKnob_.slider.getValue());
        const float lo = std::clamp(baseVal - depthZ, 0.0f, 1.0f);
        const float hi = std::clamp(baseVal + depthZ, 0.0f, 1.0f);
        const float pip = std::clamp(baseVal + processor_.getLatchedDraw(2) * depthZ, 0.0f, 1.0f);
        updateProp(macroZKnob_.slider, "randLo", lo);
        updateProp(macroZKnob_.slider, "randHi", hi);
        updateProp(macroZKnob_.slider, "randPip", pip);
    } else {
        clearProp(macroZKnob_.slider);
    }

    // 4. Macro T (0..1)
    const float depthT = static_cast<float>(randTKnob_.slider.getValue()) * 0.01f;
    if (depthT > 0.001f) {
        const float baseVal = static_cast<float>(macroTKnob_.slider.getValue());
        const float lo = std::clamp(baseVal - depthT, 0.0f, 1.0f);
        const float hi = std::clamp(baseVal + depthT, 0.0f, 1.0f);
        const float pip = std::clamp(baseVal + processor_.getLatchedDraw(3) * depthT, 0.0f, 1.0f);
        updateProp(macroTKnob_.slider, "randLo", lo);
        updateProp(macroTKnob_.slider, "randHi", hi);
        updateProp(macroTKnob_.slider, "randPip", pip);
    } else {
        clearProp(macroTKnob_.slider);
    }

    // 5. Vel -> Aggr (0..1)
    const float depthAggr = static_cast<float>(randAggrKnob_.slider.getValue()) * 0.01f;
    if (depthAggr > 0.001f) {
        const float baseVal = static_cast<float>(velAggrKnob_.slider.getValue());
        const float lo = std::clamp(baseVal - depthAggr, 0.0f, 1.0f);
        const float hi = std::clamp(baseVal + depthAggr, 0.0f, 1.0f);
        const float pip = std::clamp(baseVal + processor_.getLatchedDraw(4) * depthAggr, 0.0f, 1.0f);
        updateProp(velAggrKnob_.slider, "randLo", lo);
        updateProp(velAggrKnob_.slider, "randHi", hi);
        updateProp(velAggrKnob_.slider, "randPip", pip);
    } else {
        clearProp(velAggrKnob_.slider);
    }

    // 6. Vel -> Timb (0..1)
    const float depthTimb = static_cast<float>(randTimbKnob_.slider.getValue()) * 0.01f;
    if (depthTimb > 0.001f) {
        const float baseVal = static_cast<float>(velTimbKnob_.slider.getValue());
        const float lo = std::clamp(baseVal - depthTimb, 0.0f, 1.0f);
        const float hi = std::clamp(baseVal + depthTimb, 0.0f, 1.0f);
        const float pip = std::clamp(baseVal + processor_.getLatchedDraw(5) * depthTimb, 0.0f, 1.0f);
        updateProp(velTimbKnob_.slider, "randLo", lo);
        updateProp(velTimbKnob_.slider, "randHi", hi);
        updateProp(velTimbKnob_.slider, "randPip", pip);
    } else {
        clearProp(velTimbKnob_.slider);
    }

    // 7. Pan (-1..+1)
    const float depthPan = static_cast<float>(randPanKnob_.slider.getValue()) * 0.01f;
    if (depthPan > 0.001f) {
        const float basePan = static_cast<float>(panKnob_.slider.getValue());
        const float valLo = std::clamp(basePan - depthPan, -1.0f, 1.0f);
        const float valHi = std::clamp(basePan + depthPan, -1.0f, 1.0f);
        const float valPip = std::clamp(basePan + processor_.getLatchedDraw(6) * depthPan, -1.0f, 1.0f);
        updateProp(panKnob_.slider, "randLo", static_cast<float>(panKnob_.slider.valueToProportionOfLength(valLo)));
        updateProp(panKnob_.slider, "randHi", static_cast<float>(panKnob_.slider.valueToProportionOfLength(valHi)));
        updateProp(panKnob_.slider, "randPip", static_cast<float>(panKnob_.slider.valueToProportionOfLength(valPip)));
    } else {
        clearProp(panKnob_.slider);
    }

    // 8. Macro Decay (0..1)
    const float depthDecay = static_cast<float>(randDecayKnob_.slider.getValue()) * 0.01f;
    if (depthDecay > 0.001f) {
        const float baseDecay = static_cast<float>(macroDecayKnob_.slider.getValue());
        const float lo = std::clamp(baseDecay - depthDecay * 0.5f, 0.0f, 1.0f);
        const float hi = std::clamp(baseDecay + depthDecay * 0.5f, 0.0f, 1.0f);
        const float pip = std::clamp(baseDecay + processor_.getLatchedDraw(7) * depthDecay * 0.5f, 0.0f, 1.0f);
        updateProp(macroDecayKnob_.slider, "randLo", lo);
        updateProp(macroDecayKnob_.slider, "randHi", hi);
        updateProp(macroDecayKnob_.slider, "randPip", pip);
    } else {
        clearProp(macroDecayKnob_.slider);
    }
}

void FormlessSpawnAudioProcessorEditor::updateRoleParamLabels(int roleIdx) {
    roleIdx = std::clamp(roleIdx, 0, tkdsp::FormlessSpawnVoice::kTotalRoles - 1);
    paramAKnob_.label.setText(tkdsp::FormlessSpawnVoice::kRoleParamNames[roleIdx][0], juce::dontSendNotification);
    paramBKnob_.label.setText(tkdsp::FormlessSpawnVoice::kRoleParamNames[roleIdx][1], juce::dontSendNotification);
    paramCKnob_.label.setText(tkdsp::FormlessSpawnVoice::kRoleParamNames[roleIdx][2], juce::dontSendNotification);
    paramDKnob_.label.setText(tkdsp::FormlessSpawnVoice::kRoleParamNames[roleIdx][3], juce::dontSendNotification);
    paramEKnob_.label.setText(tkdsp::FormlessSpawnVoice::kRoleParamNames[roleIdx][4], juce::dontSendNotification);
    paramFKnob_.label.setText(tkdsp::FormlessSpawnVoice::kRoleParamNames[roleIdx][5], juce::dontSendNotification);
    paramGKnob_.label.setText(tkdsp::FormlessSpawnVoice::kRoleParamNames[roleIdx][6], juce::dontSendNotification);
    paramHKnob_.label.setText(tkdsp::FormlessSpawnVoice::kRoleParamNames[roleIdx][7], juce::dontSendNotification);
}
