#include "FormlessEditor.h"
#include "formless_spawn/FormlessControlInfo.h"
#include "formless_spawn/formless_spawnParameterIds.h"

FormlessSpawnAudioProcessorEditor::FormlessSpawnAudioProcessorEditor(FormlessSpawnAudioProcessor& p)
    : AudioProcessorEditor(&p), processor_(p)
{
    using namespace formless_spawnParameters::Ids;

    // 1. Mount content container
    addAndMakeVisible(contentContainer);
    contentContainer.setBounds(0, 0, kBaseWidth, kBaseHeight);
    contentContainer.setInterceptsMouseClicks(false, true);
    contentContainer.onPaint = [this](juce::Graphics& g) { paintCanvas(g); };
    contentContainer.addMouseListener(this, true);

    setLookAndFeel(&lnf_);
    startTimerHz(30);

    // 2. Configure constrainer & resizer
    constrainer.setFixedAspectRatio(double(kBaseWidth) / double(kBaseHeight));
    constrainer.setSizeLimits(juce::roundToInt(kBaseWidth  * rlyeh::ui::EditorScaleManager::kMinScale),
                              juce::roundToInt(kBaseHeight * rlyeh::ui::EditorScaleManager::kMinScale),
                              juce::roundToInt(kBaseWidth  * rlyeh::ui::EditorScaleManager::kMaxScale),
                              juce::roundToInt(kBaseHeight * rlyeh::ui::EditorScaleManager::kMaxScale));
    setConstrainer(&constrainer);
    setResizable(true, true);

    // 3. Resolve preferred scale (Session XML > Global JSON > Default)
    const float sessionScale = processor_.getStoredEditorScale();
    prefScale_ = rlyeh::ui::EditorScaleManager::getPreferredScale("formless_spawn", sessionScale);
    effScale_ = prefScale_;
    expectedSize_ = { juce::roundToInt(kBaseWidth * effScale_), juce::roundToInt(kBaseHeight * effScale_) };
    setSize(expectedSize_.x, expectedSize_.y);

    auto& apvts = processor_.getAPVTS();

    // =========================================================================
    // 1. Setup Header Row 1
    // =========================================================================
    contentContainer.addAndMakeVisible(presetStub_);
    contentContainer.addAndMakeVisible(headerInspector_);

    lifeformRocker_.onSelect = [&apvts](int idx) {
        if (auto* param = apvts.getParameter(life_mode))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
    };
    if (auto* param = apvts.getRawParameterValue(life_mode))
        lifeformRocker_.setState(static_cast<int>(param->load()));
    contentContainer.addAndMakeVisible(lifeformRocker_);

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
        r.blurb = juce::String(FormlessInfo::kRoleDescriptions[roleIdx].data(), FormlessInfo::kRoleDescriptions[roleIdx].size());
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
    contentContainer.addAndMakeVisible(roleGrid_);
    contentContainer.addAndMakeVisible(modelGrid_);

    // =========================================================================
    // 3. Setup Card 1 Knobs: Master Macro & Dynamics Lab (Top-Left, 8)
    // 6-Domain Chromatic Standard: Movie Poster Orange (0xFFFF7B25)
    // =========================================================================
    const auto orangeCol = juce::Colour(0xFFFF7B25);
    setupKnob(macroXKnob_,     macro_x,     "X: SPECTRAL","%", "macro spectral spread, sweep excursion and harmonics", orangeCol);
    setupKnob(macroYKnob_,     macro_y,     "Y: PUNCH",   "%", "macro transient attack spike and punch snap",         orangeCol);
    setupKnob(macroZKnob_,     macro_z,     "Z: TIMBRE",  "%", "macro harmonic waveshaping, fold & overtone",         orangeCol);
    setupKnob(macroTKnob_,     macro_t,     "T: TIME",    "%", "macro pitch envelope and body decay time",            orangeCol);
    setupKnob(velAggrKnob_,    vel_aggr,    "VEL->AGGR",  "%", "velocity to synthesis aggression & overdrive",       orangeCol);
    setupKnob(velTimbKnob_,    vel_timb,    "VEL->TIMB",  "%", "velocity to synthesis brightness & overtones",       orangeCol);
    setupKnob(panKnob_,        pan,         "PAN",        "%", "stereo panorama positioning",                        orangeCol, true);
    setupKnob(decayCurveKnob_, decay_curve, "DECAY CRV",  "",  "envelope discharge curve (exp to log)",              orangeCol, true);

    // =========================================================================
    // 4. Setup Card 2 Knobs: Role Synthesis Lab (Top-Right, 8)
    // 6-Domain Chromatic Standard: Movie Poster Blue (0xFF38BDF8)
    // =========================================================================
    const auto blueCol = juce::Colour(0xFF38BDF8);
    setupKnob(paramAKnob_, param_a, "P.CURVE", "%", "role synthesis parameter A", blueCol);
    setupKnob(paramBKnob_, param_b, "P.DEC",   "%", "role synthesis parameter B", blueCol);
    setupKnob(paramCKnob_, param_c, "SWEEP.D", "%", "role synthesis parameter C", blueCol);
    setupKnob(paramDKnob_, param_d, "DECAY",   "%", "role synthesis parameter D", blueCol);
    setupKnob(paramEKnob_, param_e, "CLICK",   "%", "role synthesis parameter E", blueCol);
    setupKnob(paramFKnob_, param_f, "TIMBRE",  "%", "role synthesis parameter F", blueCol);
    setupKnob(paramGKnob_, param_g, "DRIVE",   "%", "role synthesis parameter G", blueCol);
    setupKnob(paramHKnob_, param_h, "PUNCH",   "%", "role synthesis parameter H", blueCol);

    const int initialRole = apvts.getRawParameterValue(role) ? static_cast<int>(apvts.getRawParameterValue(role)->load()) : 0;
    updateRoleParamLabels(initialRole);

    // =========================================================================
    // 5. Setup Card 3 Pod 1: Stepped Random (2x4 Grid of 8 Small Knobs, Bottom-Left)
    // 6-Domain Chromatic Standard: Caustic Hastur Yellow (0xFFF2E10F)
    // =========================================================================
    const auto yellowCol = juce::Colour(0xFFF2E10F);
    setupKnob(randXKnob_,     rand_x,     "R-X",     "%", "per-hit Macro X S&H random depth",                     yellowCol);
    setupKnob(randYKnob_,     rand_y,     "R-Y",     "%", "per-hit Macro Y S&H random depth",                     yellowCol);
    setupKnob(randZKnob_,     rand_z,     "R-Z",     "%", "per-hit Macro Z S&H random depth",                     yellowCol);
    setupKnob(randTKnob_,     rand_t,     "R-T",     "%", "per-hit Macro T S&H random depth",                     yellowCol);
    setupKnob(randAggrKnob_,  rand_aggr,  "R-AGGR",  "%", "per-hit velocity aggression fluctuation depth",        yellowCol);
    setupKnob(randTimbKnob_,  rand_timb,  "R-TIMB",  "%", "per-hit velocity timbre fluctuation depth",            yellowCol);
    setupKnob(randPanKnob_,   rand_pan,   "R-PAN",   "%", "per-hit stereo position modulation depth",             yellowCol);
    setupKnob(randDecayKnob_, rand_decay, "R-DECAY", "%", "per-hit Master Macro Decay modulation depth",          yellowCol);

    // =========================================================================
    // 6. Setup Card 3 Pod 2: The Performance Altar (EXACTLY TWO CONTROLS)
    // 6-Domain Chromatic Standard: Caustic Hastur Yellow (0xFFF2E10F)
    // =========================================================================
    setupKnob(macroDecayKnob_, macro_decay, "MACRO DECAY", "", "master exponential envelope decay scaling", yellowCol);
    macroDecayKnob_.slider.setDoubleClickReturnValue(true, 0.5);

    trigButton_.onTrigger = [this]() {
        processor_.triggerManual(-1, -1.0f);
    };
    trigButton_.onRightTrigger = [this]() {
        processor_.triggerManual(60, 0.95f);
    };
    contentContainer.addAndMakeVisible(trigButton_);

    // =========================================================================
    // 7. Setup Card 3 Pod 3: VCF, Universal Mojo Strip & Master Output
    // =========================================================================
    // Row 1: Cytomic ZDF SVF & Dynamic Filter Envelope (Acid Green 0xFF00E676)
    const auto acidGreenCol = juce::Colour(0xFF00E676);
    setupKnob(cutoffKnob_,       cutoff,          "CUTOFF",    "Hz", "filter cutoff frequency",                    acidGreenCol);
    setupKnob(resonanceKnob_,    resonance,       "RES",       "%",  "filter resonance / self-oscillation",        acidGreenCol);
    setupKnob(filterEnvAmtKnob_, filter_env_amt,  "FE-AMT",    "%",  "filter envelope sweep amount (0%..100%, +10 octaves)", acidGreenCol, false);
    setupKnob(filterDecayKnob_,  filter_decay,    "FE-DEC",    "ms", "filter envelope decay time (5ms..5.0s)",      acidGreenCol);

    filterTypeBar_.setAccentColour(acidGreenCol);
    filterTypeBar_.onSelect = [this, &apvts](int idx) {
        if (auto* param = apvts.getParameter(filter_type))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
        updateFilterTypeUI(idx);
    };
    if (auto* param = apvts.getRawParameterValue(filter_type)) {
        const int curType = static_cast<int>(param->load());
        filterTypeBar_.setState(curType);
        updateFilterTypeUI(curType);
    }
    contentContainer.addAndMakeVisible(filterTypeBar_);

    // Row 2: Universal Mojo Strip (Sorcerous Purple 0xFFBC8CFF)
    const auto purpleCol = juce::Colour(0xFFBC8CFF);
    setupKnob(preDriveKnob_,     pre_drive,       "DRIVE",     "%",  "pre-filter analog saturation and drive",     purpleCol);
    setupKnob(crushKnob_,        crush,           "CRUSH",     "Hz", "mojo sample rate decimation frequency",      purpleCol);
    setupKnob(shiftKnob_,        shift,           "SHIFT",     "Hz", "bode frequency shifter (+/-1000Hz)",         purpleCol, true);
    setupKnob(squashKnob_,       squash,          "SQUASH",    "%",  "bipolar dynamics: thin/snappy vs thick/huge",purpleCol, true);
    setupKnob(widerKnob_,        wider,           "WIDER",     "%",  "bipolar widener: up=classic comb, down=hilbert phase",     purpleCol, true);
    widerKnob_.slider.textFromValueFunction = [](double val) -> juce::String {
        if (std::abs(val) < 0.01) return "OFF";
        const int pct = static_cast<int>(std::round(std::abs(val) * 100.0));
        return (val > 0.0) ? ("+" + juce::String(pct) + "%") : ("-" + juce::String(pct) + "%");
    };

    // Row 3: Master Output VCA & Distressor Limiter (Blistering Red 0xFFFF3B30)
    const auto redCol = juce::Colour(0xFFFF3B30);
    limiterAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, master_limiter, limiterButton_);
    contentContainer.addAndMakeVisible(limiterButton_);

    setupKnob(volumeKnob_,       volume,          "VOLUME",    "dB", "output level into the limiter (-inf..0..+12dB)", redCol);
    volumeKnob_.slider.setDoubleClickReturnValue(true, 0.5);

    setupKnob(ampDecayKnob_,     amp_decay,       "AMP-DEC",   "s",  "master dynamic amp decay [5ms .. engine length]", redCol);
    ampDecayKnob_.slider.setDoubleClickReturnValue(true, 1.0);

    setupKnob(velocityKnob_,     amp_velocity,    "VEL",       "%",  "master velocity response: 0%=flat, 50%=linear, 100%=exponential", redCol);
    velocityKnob_.slider.setDoubleClickReturnValue(true, 0.0);

    layoutControls();
}

FormlessSpawnAudioProcessorEditor::~FormlessSpawnAudioProcessorEditor() {
    setConstrainer(nullptr);
    stopTimer();
    contentContainer.removeMouseListener(this);
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
    contentContainer.addAndMakeVisible(item.slider);
    rotarySliders_.push_back(&item.slider);

    item.label.setText(name, juce::dontSendNotification);
    item.label.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
    item.label.setColour(juce::Label::textColourId, juce::Colour(0xFF8B949E));
    item.label.setJustificationType(juce::Justification::centred);
    item.label.setBorderSize(juce::BorderSize<int>(0));
    item.label.setInterceptsMouseClicks(false, false);
    contentContainer.addAndMakeVisible(item.label);

    item.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor_.getAPVTS(), paramId, item.slider);

    // Concise metric value formatter for In-Dial Kilohearts badge (<= 8 chars)
    item.slider.textFromValueFunction = [this, name, unit](double val) -> juce::String {
        if (name == "RES" || name == "GAIN") {
            const int fType = filterTypeBar_.getState();
            if (fType == 3) {
                const float gainDb = tkdsp::formless::evalPeakingGainDb(static_cast<float>(val));
                return (gainDb > 0.0f ? "+" : "") + juce::String(gainDb, 1) + " dB";
            }
            const float q = tkdsp::formless::evalCytomicQ(static_cast<float>(val));
            return "Q " + juce::String(q, 2);
        }
        if (name == "VOLUME") {
            const float u = static_cast<float>(val);
            if (u <= 0.0001f) return "-inf dB";
            float db = std::round(tkdsp::formless::evalMasterVolumeDb(u) * 10.0f) * 0.1f;
            if (std::abs(db) < 0.05f) db = 0.0f;
            return (db > 0.0f ? "+" : "") + juce::String(db, 1) + " dB";
        }
        if (name == "AMP-DEC") {
            const float u = static_cast<float>(val);
            const int curRole = std::clamp(roleGrid_.getState(), 0, tkdsp::FormlessSpawnVoice::kTotalRoles - 1);
            const int curModel = std::clamp(modelGrid_.getModel(), 0, tkdsp::FormlessSpawnVoice::kModelsPerRole - 1);
            const float macroDec = static_cast<float>(macroDecayKnob_.slider.getValue());
            const float macroT = static_cast<float>(macroTKnob_.slider.getValue());
            const float paramD = static_cast<float>(paramDKnob_.slider.getValue());
            const float baseSec = tkdsp::FormlessSpawnVoice::kModelBaseAmpDecaySec[curRole][curModel];
            const float bodyMult = tkdsp::formless::evalBodyDecayMult(paramD);
            const float macroMult = tkdsp::formless::evalMacroMultiplier(macroDec);
            const float tMult = tkdsp::formless::evalMacroMultiplier(macroT);
            const float lEngine = tkdsp::formless::evalEngineLengthSec(baseSec, bodyMult, macroMult, tMult);
            const float sec = tkdsp::formless::evalDynamicAmpDecaySec(u, lEngine);
            if (sec >= 1.0f) return juce::String(sec, 2) + " s";
            return juce::String(juce::roundToInt(sec * 1000.0f)) + " ms";
        }
        if (name == "FE-AMT") {
            const float pct = static_cast<float>(val) * 100.0f;
            if (std::abs(pct) < 0.5f) return "0%";
            if (pct > 0.0f) return "+" + juce::String(juce::roundToInt(pct)) + "%";
            return juce::String(juce::roundToInt(pct)) + "%";
        }
        if (name == "FE-DEC") {
            if (val >= 1.0) return juce::String(val, 2) + " s";
            return juce::String(juce::roundToInt(val * 1000.0)) + " ms";
        }
        if (name == "DRIVE") {
            const float db = tkdsp::formless::evalDriveDb(static_cast<float>(val));
            if (val <= 0.001f) return "0.0 dB";
            return "+" + juce::String(db, 1) + " dB";
        }
        if (name == "SQUASH") {
            if (std::abs(val) < 0.005) return "0%";
            if (val < 0.0) return juce::String(juce::roundToInt(val * 100.0)) + "%";
            return "+" + juce::String(juce::roundToInt(val * 100.0)) + "%";
        }
        if (name == "WIDER") {
            if (std::abs(val) < 0.01) return "OFF";
            const int pct = static_cast<int>(std::round(std::abs(val) * 100.0));
            return (val > 0.0) ? ("+" + juce::String(pct) + "%") : ("-" + juce::String(pct) + "%");
        }
        if (name == "PAN") {
            const int p = juce::roundToInt(val * 100.0);
            if (p == 0) return "C";
            if (p < 0) return juce::String(-p) + "% L";
            return juce::String(p) + "% R";
        }
        if (name == "DECAY-CRV") {
            if (std::abs(val) < 0.01) return "LIN 0.00";
            if (val < 0.0) return "EXP " + juce::String(val, 2);
            return "LOG +" + juce::String(val, 2);
        }
        if (name.startsWith("MACRO") || name.startsWith("VEL-")) {
            const float normVal = static_cast<float>(val);
            const float bipPercent = (normVal - 0.5f) * 200.0f;
            return (bipPercent > 0.0f ? "+" : "") + juce::String(juce::roundToInt(bipPercent)) + "%";
        }
        if (unit == "Hz") {
            if (val >= 1000.0) return juce::String(val * 0.001, 1) + " kHz";
            return juce::String(juce::roundToInt(val)) + " Hz";
        }
        if (unit == "ms") return juce::String(val, 1) + " ms";
        if (unit == "st") return (val > 0.0 ? "+" : "") + juce::String(val, 1) + " st";
        if (unit == "dB") return juce::String(val, 1) + " dB";
        if (unit == "%") return juce::String(juce::roundToInt(val * 100.0)) + "%";
        return juce::String(val, 2);
    };

    // Live Streaming Telemetry to HeaderInspector
    auto sendTelemetry = [this, &item, blurb, col](double val) {
        HeaderInspector::Readout r;
        r.name = item.label.getText();
        r.value = item.slider.textFromValueFunction ? item.slider.textFromValueFunction(val) : juce::String(val, 2);

        // Dynamically serve Card 2 blurbs & names per active role
        const int curRole = std::clamp(roleGrid_.getState(), 0, tkdsp::FormlessSpawnVoice::kTotalRoles - 1);
        if (&item == &paramAKnob_) { r.name = tkdsp::FormlessSpawnVoice::kRoleParamNames[curRole][0]; r.blurb = FormlessInfo::getRoleParamBlurb(curRole, 0); }
        else if (&item == &paramBKnob_) { r.name = tkdsp::FormlessSpawnVoice::kRoleParamNames[curRole][1]; r.blurb = FormlessInfo::getRoleParamBlurb(curRole, 1); }
        else if (&item == &paramCKnob_) { r.name = tkdsp::FormlessSpawnVoice::kRoleParamNames[curRole][2]; r.blurb = FormlessInfo::getRoleParamBlurb(curRole, 2); }
        else if (&item == &paramDKnob_) { r.name = tkdsp::FormlessSpawnVoice::kRoleParamNames[curRole][3]; r.blurb = FormlessInfo::getRoleParamBlurb(curRole, 3); }
        else if (&item == &paramEKnob_) { r.name = tkdsp::FormlessSpawnVoice::kRoleParamNames[curRole][4]; r.blurb = FormlessInfo::getRoleParamBlurb(curRole, 4); }
        else if (&item == &paramFKnob_) { r.name = tkdsp::FormlessSpawnVoice::kRoleParamNames[curRole][5]; r.blurb = FormlessInfo::getRoleParamBlurb(curRole, 5); }
        else if (&item == &paramGKnob_) { r.name = tkdsp::FormlessSpawnVoice::kRoleParamNames[curRole][6]; r.blurb = FormlessInfo::getRoleParamBlurb(curRole, 6); }
        else if (&item == &paramHKnob_) { r.name = tkdsp::FormlessSpawnVoice::kRoleParamNames[curRole][7]; r.blurb = FormlessInfo::getRoleParamBlurb(curRole, 7); }
        else {
            r.blurb = blurb;
        }

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
}

void FormlessSpawnAudioProcessorEditor::paintCanvas(juce::Graphics& g) {
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
    g.drawVerticalLine(kPod3Divider, (float)p3y + 26.0f, (float)kPod3.getBottom() - 8.0f);
}

void FormlessSpawnAudioProcessorEditor::layoutControls() {
    // Header: right edge lands on 880 (matches card right edge)
    presetStub_.setBounds(174, 8, 145, 28);
    headerInspector_.setBounds(326, 8, 314, 28);
    lifeformRocker_.setBounds(646, 8, 234, 28);

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

    // Pod 3: 5 columns (Row 0: VCF Strip, Row 1: Mojo Strip) + Master Column
    {
        const int py = kPod3.getY();
        const int x0 = kPod3.getX() + 8;          // 552
        const int pitch = 49;
        const int labelY0 = py + 32;
        const int sliderY0 = labelY0 + 18;
        const int labelY1 = py + 128;
        const int sliderY1 = labelY1 + 18;

        // Row 0 (VCF Strip): CUTOFF, RES, FE-AMT, FE-DEC, and filterTypeBar_ (48x48)
        place(cutoffKnob_,       x0 + pitch / 2 + pitch * 0, labelY0, pitch, 48, sliderY0);
        place(resonanceKnob_,    x0 + pitch / 2 + pitch * 1, labelY0, pitch, 48, sliderY0);
        place(filterEnvAmtKnob_, x0 + pitch / 2 + pitch * 2, labelY0, pitch, 48, sliderY0);
        place(filterDecayKnob_,  x0 + pitch / 2 + pitch * 3, labelY0, pitch, 48, sliderY0);
        filterTypeBar_.setBounds(x0 + pitch * 4 + 2, sliderY0, 48, 48);

        // Row 1 (Mojo Strip): DRIVE, CRUSH, BITS, SQUASH, and WIDER knob
        place(preDriveKnob_,     x0 + pitch / 2 + pitch * 0, labelY1, pitch, 48, sliderY1);
        place(crushKnob_,        x0 + pitch / 2 + pitch * 1, labelY1, pitch, 48, sliderY1);
        place(shiftKnob_,        x0 + pitch / 2 + pitch * 2, labelY1, pitch, 48, sliderY1);
        place(squashKnob_,       x0 + pitch / 2 + pitch * 3, labelY1, pitch, 48, sliderY1);
        place(widerKnob_,        x0 + pitch / 2 + pitch * 4, labelY1, pitch, 48, sliderY1);

        // Master Column: Limiter button, Volume knob, Amp Decay knob, Velocity knob (x=804..872)
        limiterButton_.setBounds(814, py + 8, 48, 20);
        place(volumeKnob_,   838, py + 32, 52, 40, py + 46);
        place(ampDecayKnob_, 838, py + 90, 52, 40, py + 104);
        place(velocityKnob_, 838, py + 148, 52, 40, py + 162);
    }
}

void FormlessSpawnAudioProcessorEditor::timerCallback() {
    if (activeBadgeSlider_ != nullptr) {
        if (juce::Time::getMillisecondCounter() >= activeBadgeExpiry_) {
            activeBadgeSlider_->getProperties().remove("badgeExpiry");
            activeBadgeSlider_->repaint();
            activeBadgeSlider_ = nullptr;
        }
    }
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

    // Update Limiter 3-State Telemetry LED (with GUI optical decay)
    const float rawGr = processor_.getLimiterGR();
    if (rawGr < guiLimiterGr_) {
        guiLimiterGr_ = rawGr; // Instant attack on transient
    } else {
        guiLimiterGr_ += (1.0f - guiLimiterGr_) * 0.22f; // ~90ms optical release
    }
    limiterButton_.updateTelemetry(guiLimiterGr_);

    const auto tele = processor_.getTelemetrySnapshot();

    using namespace formless_spawnParameters::Ids;
    if (auto* param = processor_.getAPVTS().getRawParameterValue(life_mode)) {
        lifeformRocker_.setState(static_cast<int>(param->load()));
    }

    // =========================================================================
    // 1. Tier 2: Stepped Random Halos on Macro Dials (Caustic Hastur Yellow)
    // =========================================================================
    // 1. Macro X (0..1)
    const float depthX = static_cast<float>(randXKnob_.slider.getValue()) * 0.01f;
    const float mX = static_cast<float>(macroXKnob_.slider.getValue());
    const auto haloX = CrackedChimes::DSP::computeStepRandHalo(mX, depthX, tele.randVal[0]);
    if (haloX.active) {
        updateProp(macroXKnob_.slider, "randLo", haloX.lo);
        updateProp(macroXKnob_.slider, "randHi", haloX.hi);
        updateProp(macroXKnob_.slider, "randPip", haloX.pip);
    } else {
        clearProp(macroXKnob_.slider);
    }

    // 2. Macro Y (0..1)
    const float depthY = static_cast<float>(randYKnob_.slider.getValue()) * 0.01f;
    const float mY = static_cast<float>(macroYKnob_.slider.getValue());
    const auto haloY = CrackedChimes::DSP::computeStepRandHalo(mY, depthY, tele.randVal[1]);
    if (haloY.active) {
        updateProp(macroYKnob_.slider, "randLo", haloY.lo);
        updateProp(macroYKnob_.slider, "randHi", haloY.hi);
        updateProp(macroYKnob_.slider, "randPip", haloY.pip);
    } else {
        clearProp(macroYKnob_.slider);
    }

    // 3. Macro Z (0..1)
    const float depthZ = static_cast<float>(randZKnob_.slider.getValue()) * 0.01f;
    const float mZ = static_cast<float>(macroZKnob_.slider.getValue());
    const auto haloZ = CrackedChimes::DSP::computeStepRandHalo(mZ, depthZ, tele.randVal[2]);
    if (haloZ.active) {
        updateProp(macroZKnob_.slider, "randLo", haloZ.lo);
        updateProp(macroZKnob_.slider, "randHi", haloZ.hi);
        updateProp(macroZKnob_.slider, "randPip", haloZ.pip);
    } else {
        clearProp(macroZKnob_.slider);
    }

    // 4. Macro T (0..1)
    const float depthT = static_cast<float>(randTKnob_.slider.getValue()) * 0.01f;
    const float mT = static_cast<float>(macroTKnob_.slider.getValue());
    const auto haloT = CrackedChimes::DSP::computeStepRandHalo(mT, depthT, tele.randVal[3]);
    if (haloT.active) {
        updateProp(macroTKnob_.slider, "randLo", haloT.lo);
        updateProp(macroTKnob_.slider, "randHi", haloT.hi);
        updateProp(macroTKnob_.slider, "randPip", haloT.pip);
    } else {
        clearProp(macroTKnob_.slider);
    }

    // 5. Vel -> Aggr (0..1)
    const float depthAggr = static_cast<float>(randAggrKnob_.slider.getValue()) * 0.01f;
    const float baseAggr = static_cast<float>(velAggrKnob_.slider.getValue());
    const auto haloAggr = CrackedChimes::DSP::computeStepRandHalo(baseAggr, depthAggr, tele.randVal[4]);
    if (haloAggr.active) {
        updateProp(velAggrKnob_.slider, "randLo", haloAggr.lo);
        updateProp(velAggrKnob_.slider, "randHi", haloAggr.hi);
        updateProp(velAggrKnob_.slider, "randPip", haloAggr.pip);
    } else {
        clearProp(velAggrKnob_.slider);
    }

    // 6. Vel -> Timb (0..1)
    const float depthTimb = static_cast<float>(randTimbKnob_.slider.getValue()) * 0.01f;
    const float baseTimb = static_cast<float>(velTimbKnob_.slider.getValue());
    const auto haloTimb = CrackedChimes::DSP::computeStepRandHalo(baseTimb, depthTimb, tele.randVal[5]);
    if (haloTimb.active) {
        updateProp(velTimbKnob_.slider, "randLo", haloTimb.lo);
        updateProp(velTimbKnob_.slider, "randHi", haloTimb.hi);
        updateProp(velTimbKnob_.slider, "randPip", haloTimb.pip);
    } else {
        clearProp(velTimbKnob_.slider);
    }

    // 7. Pan (-1..+1)
    const float depthPan = static_cast<float>(randPanKnob_.slider.getValue()) * 0.01f;
    const float basePan = static_cast<float>(panKnob_.slider.getValue());
    const float normPan = static_cast<float>(panKnob_.slider.valueToProportionOfLength(basePan));
    const auto haloPan = CrackedChimes::DSP::computeStepRandHalo(normPan, depthPan, tele.randVal[6]);
    if (haloPan.active) {
        updateProp(panKnob_.slider, "randLo", haloPan.lo);
        updateProp(panKnob_.slider, "randHi", haloPan.hi);
        updateProp(panKnob_.slider, "randPip", haloPan.pip);
    } else {
        clearProp(panKnob_.slider);
    }

    // 8. Macro Decay (0..1)
    const float depthDecay = static_cast<float>(randDecayKnob_.slider.getValue()) * 0.01f;
    const float baseDecay = static_cast<float>(macroDecayKnob_.slider.getValue());
    const auto haloDecay = CrackedChimes::DSP::computeStepRandHalo(baseDecay, depthDecay, tele.randVal[7]);
    if (haloDecay.active) {
        updateProp(macroDecayKnob_.slider, "randLo", haloDecay.lo);
        updateProp(macroDecayKnob_.slider, "randHi", haloDecay.hi);
        updateProp(macroDecayKnob_.slider, "randPip", haloDecay.pip);
    } else {
        clearProp(macroDecayKnob_.slider);
    }

    // =========================================================================
    // 2. Tier 3: Card 2 Role Synthesis Lab (Knobs A through H)
    // Trickle-Down Ecoknobics: Concentric Inner Orange & Outer Yellow Halos
    // =========================================================================
    auto updateModProp = [](juce::Slider& s, const tkdsp::formless::HaloRange& r) {
        if (r.active) {
            s.getProperties().set("modLo", r.lo);
            s.getProperties().set("modHi", r.hi);
            s.getProperties().set("modPip", r.pip);
        } else if (s.getProperties().contains("modLo")) {
            s.getProperties().remove("modLo");
            s.getProperties().remove("modHi");
            s.getProperties().remove("modPip");
        }
        s.repaint();
    };

    auto updateRandProp = [](juce::Slider& s, const tkdsp::formless::HaloRange& r) {
        if (r.active) {
            s.getProperties().set("randLo", r.lo);
            s.getProperties().set("randHi", r.hi);
            s.getProperties().set("randPip", r.pip);
        } else if (s.getProperties().contains("randLo")) {
            s.getProperties().remove("randLo");
            s.getProperties().remove("randHi");
            s.getProperties().remove("randPip");
        }
        s.repaint();
    };

    const float valA = static_cast<float>(paramAKnob_.slider.getValue());
    const float valB = static_cast<float>(paramBKnob_.slider.getValue());
    const float valC = static_cast<float>(paramCKnob_.slider.getValue());
    const float valD = static_cast<float>(paramDKnob_.slider.getValue());
    const float valE = static_cast<float>(paramEKnob_.slider.getValue());
    const float valF = static_cast<float>(paramFKnob_.slider.getValue());
    const float valG = static_cast<float>(paramGKnob_.slider.getValue());
    const float valH = static_cast<float>(paramHKnob_.slider.getValue());

    // --- Inner Ring: Macro Modulation Halos (Movie Poster Orange 0xFFFF7B25) ---
    // Perfect 4x2 Macro Matrix Symmetry across ALL 8 Roles:
    updateModProp(paramAKnob_.slider, tkdsp::formless::computeMacroHalo(valA, mX, 0.85f));
    updateModProp(paramBKnob_.slider, tkdsp::formless::computeMacroHalo(valB, mT, 0.70f));
    updateModProp(paramCKnob_.slider, tkdsp::formless::computeMacroHalo(valC, mX, 0.75f));
    updateModProp(paramDKnob_.slider, tkdsp::formless::computeMacroHalo(valD, mT, 0.70f));
    updateModProp(paramEKnob_.slider, tkdsp::formless::computeMacroHalo(valE, mY, 0.80f));
    updateModProp(paramFKnob_.slider, tkdsp::formless::computeMacroHalo(valF, mZ, 0.70f));
    updateModProp(paramGKnob_.slider, tkdsp::formless::computeMacroHalo(valG, mZ, 0.65f));
    updateModProp(paramHKnob_.slider, tkdsp::formless::computeMacroHalo(valH, mY, 0.75f));

    // --- Outer Ring: Cascaded Stepped Random & Velocity Halos (Caustic Hastur Yellow 0xFFF2E10F) ---
    // Knobs A & C <- Macro X Cascaded Stepped Random
    updateRandProp(paramAKnob_.slider, tkdsp::formless::trickleDownHeadroom(valA, haloX.lo, haloX.hi, haloX.pip, 0.85f));
    updateRandProp(paramCKnob_.slider, tkdsp::formless::trickleDownHeadroom(valC, haloX.lo, haloX.hi, haloX.pip, 0.75f));

    // Knobs B & D <- Macro Decay + Macro T Cascaded Compound Decay
    updateRandProp(paramBKnob_.slider, tkdsp::formless::trickleDownDecay(valB, haloDecay.lo, haloDecay.hi, haloDecay.pip, haloT.lo, haloT.hi, haloT.pip, 0.70f));
    updateRandProp(paramDKnob_.slider, tkdsp::formless::trickleDownDecay(valD, haloDecay.lo, haloDecay.hi, haloDecay.pip, haloT.lo, haloT.hi, haloT.pip, 0.70f));

    // Knobs E & H <- Macro Y + Velocity Dynamics (Vel->Aggr / Vel->Timb)
    updateRandProp(paramEKnob_.slider, tkdsp::formless::trickleDownArbitrate(valE, haloY.lo, haloY.hi, haloY.pip, 0.80f,
                                                                             haloTimb.lo, haloTimb.hi, haloTimb.pip, 0.65f, tele.lastVelocity));
    updateRandProp(paramHKnob_.slider, tkdsp::formless::trickleDownArbitrate(valH, haloY.lo, haloY.hi, haloY.pip, 0.75f,
                                                                             haloAggr.lo, haloAggr.hi, haloAggr.pip, 0.75f, tele.lastVelocity));

    // Knobs F & G <- Macro Z + Velocity Dynamics (Vel->Aggr / Vel->Timb)
    updateRandProp(paramFKnob_.slider, tkdsp::formless::trickleDownArbitrate(valF, haloZ.lo, haloZ.hi, haloZ.pip, 0.70f,
                                                                             haloTimb.lo, haloTimb.hi, haloTimb.pip, 0.70f, tele.lastVelocity));
    updateRandProp(paramGKnob_.slider, tkdsp::formless::trickleDownArbitrate(valG, haloZ.lo, haloZ.hi, haloZ.pip, 0.65f,
                                                                             haloAggr.lo, haloAggr.hi, haloAggr.pip, 0.70f, tele.lastVelocity));

    // Master Amp Decay Knob Halos: Outer Yellow Stepped Random Ring (Macro Decay decoupled)
    const float valAmpDecay = static_cast<float>(ampDecayKnob_.slider.getValue());
    const auto haloAmpDecay = CrackedChimes::DSP::computeStepRandHalo(valAmpDecay, depthDecay, tele.randVal[7]);
    if (haloAmpDecay.active) {
        updateProp(ampDecayKnob_.slider, "randLo", haloAmpDecay.lo);
        updateProp(ampDecayKnob_.slider, "randHi", haloAmpDecay.hi);
        updateProp(ampDecayKnob_.slider, "randPip", haloAmpDecay.pip);
    } else {
        clearProp(ampDecayKnob_.slider);
    }

    // 3. Card 3: Cutoff Filter Envelope Arc Halo (Electric Cyan Inner Arc)
    auto& cutSlider = cutoffKnob_.slider;
    const float feAmt = static_cast<float>(filterEnvAmtKnob_.slider.getValue());
    const double curCutoffHz = cutSlider.getValue();
    const float normBase = static_cast<float>(cutSlider.valueToProportionOfLength(curCutoffHz));

    tkdsp::formless::HaloRange cutHalo;
    if (feAmt > 0.005f) {
        // Dynamic Filter Envelope throw (0..+10.0 Octaves at full velocity 1.0)
        const float peakOctaves = std::clamp(feAmt * 10.0f, 0.0f, 10.5f);
        const double peakCutoffHz = std::clamp(curCutoffHz * std::pow(2.0, static_cast<double>(peakOctaves)), 20.0, 20000.0);
        const float normPeak = static_cast<float>(cutSlider.valueToProportionOfLength(peakCutoffHz));

        cutHalo.lo = normBase;
        cutHalo.hi = normPeak;
        cutHalo.pip = normPeak;
        cutHalo.active = (cutHalo.hi - cutHalo.lo > 0.002f);
    } else {
        cutHalo.active = false;
    }
    updateModProp(cutSlider, cutHalo);
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

void FormlessSpawnAudioProcessorEditor::updateFilterTypeUI(int filterIdx) {
    if (filterIdx == 3) {
        // PK (Peaking Bell EQ)
        resonanceKnob_.label.setText("GAIN", juce::dontSendNotification);
        resonanceKnob_.slider.getProperties().set("isBipolar", true);
        resonanceKnob_.slider.setDoubleClickReturnValue(true, 0.5);
    } else {
        // LP, HP, BP (Resonant Q)
        resonanceKnob_.label.setText("RES", juce::dontSendNotification);
        resonanceKnob_.slider.getProperties().set("isBipolar", false);
        resonanceKnob_.slider.setDoubleClickReturnValue(true, 0.0);
    }
    resonanceKnob_.slider.repaint();
}

void FormlessSpawnAudioProcessorEditor::mouseEnter(const juce::MouseEvent& e) {
    if (auto* s = dynamic_cast<juce::Slider*>(e.eventComponent)) {
        if (s->onValueChange) s->onValueChange();
    }
}

void FormlessSpawnAudioProcessorEditor::mouseExit(const juce::MouseEvent& e) {
    juce::ignoreUnused(e);
    if (!juce::ModifierKeys::currentModifiers.isAnyMouseButtonDown()) {
        headerInspector_.setReadoutDefault();
    }
}

void FormlessSpawnAudioProcessorEditor::mouseDown(const juce::MouseEvent& e) {
    if (auto* s = dynamic_cast<juce::Slider*>(e.eventComponent)) {
        activeBadgeSlider_ = s;
        activeBadgeExpiry_ = juce::Time::getMillisecondCounter() + 350;
        s->getProperties().set("badgeExpiry", static_cast<double>(activeBadgeExpiry_));
        s->repaint();
    }
}

void FormlessSpawnAudioProcessorEditor::mouseDrag(const juce::MouseEvent& e) {
    if (auto* s = dynamic_cast<juce::Slider*>(e.eventComponent)) {
        activeBadgeSlider_ = s;
        activeBadgeExpiry_ = juce::Time::getMillisecondCounter() + 350;
        s->getProperties().set("badgeExpiry", static_cast<double>(activeBadgeExpiry_));
        s->repaint();
    }
}

void FormlessSpawnAudioProcessorEditor::mouseUp(const juce::MouseEvent& e) {
    if (auto* s = dynamic_cast<juce::Slider*>(e.eventComponent)) {
        activeBadgeSlider_ = s;
        activeBadgeExpiry_ = juce::Time::getMillisecondCounter() + 350;
        s->getProperties().set("badgeExpiry", static_cast<double>(activeBadgeExpiry_));
        s->repaint();
    }
    if (e.eventComponent != nullptr && !e.eventComponent->isMouseOver()) {
        headerInspector_.setReadoutDefault();
    }
}

void FormlessSpawnAudioProcessorEditor::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) {
    juce::ignoreUnused(wheel);
    if (auto* s = dynamic_cast<juce::Slider*>(e.eventComponent)) {
        activeBadgeSlider_ = s;
        activeBadgeExpiry_ = juce::Time::getMillisecondCounter() + 350;
        s->getProperties().set("badgeExpiry", static_cast<double>(activeBadgeExpiry_));
        s->repaint();
    }
}


void FormlessSpawnAudioProcessorEditor::visibilityChanged() {
    if (isShowing()) requestRefit();
}

void FormlessSpawnAudioProcessorEditor::resized() {
    const int w = getWidth();
    const int h = getHeight();
    if (w <= 0 || h <= 0) return;

    const float s = juce::jmin(static_cast<float>(w) / static_cast<float>(kBaseWidth),
                               static_cast<float>(h) / static_cast<float>(kBaseHeight));
    const int ox = (w - juce::roundToInt(kBaseWidth  * s)) / 2;
    const int oy = (h - juce::roundToInt(kBaseHeight * s)) / 2;

    contentContainer.setBounds(0, 0, kBaseWidth, kBaseHeight);
    contentContainer.setTransform(juce::AffineTransform::scale(s).translated(static_cast<float>(ox), static_cast<float>(oy)));

    if (std::abs(s - effScale_) > 1.0e-4f) {
        effScale_ = s;
        const int sens = juce::roundToInt(250.0f / s);
        for (auto* sl : rotarySliders_) {
            if (sl != nullptr) sl->setMouseDragSensitivity(sens);
        }
    }

    if (getPeer() != nullptr && juce::Point<int>{ w, h } != expectedSize_) {
        commitUserScale(s);
    }
}

void FormlessSpawnAudioProcessorEditor::applyEffectiveScale(float s) {
    s = juce::jlimit(rlyeh::ui::EditorScaleManager::kMinScale, rlyeh::ui::EditorScaleManager::kMaxScale, s);
    expectedSize_ = { juce::roundToInt(kBaseWidth * s), juce::roundToInt(kBaseHeight * s) };
    setSize(expectedSize_.x, expectedSize_.y);
}

void FormlessSpawnAudioProcessorEditor::commitUserScale(float s) {
    prefScale_ = std::round(juce::jlimit(rlyeh::ui::EditorScaleManager::kMinScale,
                                         rlyeh::ui::EditorScaleManager::kMaxScale, s) * 100.0f) / 100.0f;
    expectedSize_ = { getWidth(), getHeight() };
    scaleWriter_.note("formless_spawn", prefScale_);
    processor_.setStoredEditorScale(prefScale_);
}

float FormlessSpawnAudioProcessorEditor::fitScale() {
    return rlyeh::ui::EditorScaleManager::getDisplayFitScale(*this, kBaseWidth, kBaseHeight);
}

void FormlessSpawnAudioProcessorEditor::requestRefit() {
    if (refitPending_) return;
    refitPending_ = true;
    juce::Component::SafePointer<FormlessSpawnAudioProcessorEditor> safe(this);
    juce::MessageManager::callAsync([safe] {
        if (safe == nullptr) return;
        safe->refitPending_ = false;

        if (juce::ModifierKeys::currentModifiers.isAnyMouseButtonDown()) {
            juce::Timer::callAfterDelay(150, [safe] { if (safe) safe->requestRefit(); });
            return;
        }

        if (safe->inRefit_ || !safe->isShowing()) return;
        const juce::ScopedValueSetter<bool> guard(safe->inRefit_, true);
        const float target = juce::jmin(safe->prefScale_, safe->fitScale());
        if (std::abs(target - safe->effScale_) > 0.01f) {
            safe->applyEffectiveScale(target);
        }
    });
}
