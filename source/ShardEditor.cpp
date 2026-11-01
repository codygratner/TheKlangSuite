#include "ShardEditor.h"
#include "ParameterManager.h"
#include "FastMath.h"

// ==============================================================================
// 1. ShardRotarySlider Implementation
// ==============================================================================
ShardRotarySlider::ShardRotarySlider() {
    setSliderStyle(juce::Slider::RotaryVerticalDrag);
    setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    setScrollWheelEnabled(true);
}

void ShardRotarySlider::paint(juce::Graphics& g) {
    auto& lf = getLookAndFeel();
    lf.drawRotarySlider(g, 0, 0, getWidth(), getHeight(),
                        static_cast<float>(valueToProportionOfLength(getValue())),
                        getRotaryParameters().startAngleRadians,
                        getRotaryParameters().endAngleRadians,
                        *this);
}

// ==============================================================================
// 2. ShardKnobCell Implementation
// ==============================================================================
ShardKnobCell::ShardKnobCell(const juce::String& title,
                             juce::Colour accent,
                             bool bipolar,
                             double defaultVal,
                             std::function<juce::String(double)> formatFn,
                             std::function<double(const juce::String&)> parseFn)
{
    nameLabel.setText(title, juce::dontSendNotification);
    nameLabel.setFont(juce::Font(juce::FontOptions(9.5f, juce::Font::bold)));
    nameLabel.setColour(juce::Label::textColourId, juce::Colour(0xff94a3b8));
    nameLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(nameLabel);

    slider.isBipolar = bipolar;
    slider.setAccentColour(accent);
    slider.setColour(juce::Slider::rotarySliderFillColourId, accent);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff2d3748));
    slider.setDoubleClickReturnValue(true, defaultVal);
    slider.getDefaultValue = [defaultVal]() { return defaultVal; };

    if (formatFn) slider.customFormatText = formatFn;
    if (parseFn)  slider.customParseText  = parseFn;

    slider.onValueChange = [this]() {
        updateReadout();
    };
    addAndMakeVisible(slider);

    readoutLabel.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::bold)));
    readoutLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe2e8f0));
    readoutLabel.setColour(juce::Label::backgroundColourId, juce::Colour(0xff0d1117));
    readoutLabel.setColour(juce::Label::outlineColourId, juce::Colour(0xff232d3f));
    readoutLabel.setJustificationType(juce::Justification::centred);
    readoutLabel.setEditable(false, true, false);
    readoutLabel.onTextChange = [this]() {
        if (slider.customParseText) {
            slider.setValue(slider.customParseText(readoutLabel.getText()), juce::sendNotificationSync);
        } else {
            slider.setValue(readoutLabel.getText().getDoubleValue(), juce::sendNotificationSync);
        }
    };
    addAndMakeVisible(readoutLabel);
}

void ShardKnobCell::updateReadout() {
    readoutLabel.setText(slider.getTextFromValue(slider.getValue()), juce::dontSendNotification);
}

void ShardKnobCell::resized() {
    auto b = getLocalBounds();
    nameLabel.setBounds(b.removeFromTop(14));
    readoutLabel.setBounds(b.removeFromBottom(16).reduced(2, 0));
    b.reduce(1, 1);
    int sz = juce::jmin(b.getWidth(), b.getHeight());
    slider.setBounds(b.withSizeKeepingCentre(sz, sz));
}

// ==============================================================================
// 3. ShardCard Implementation
// ==============================================================================
ShardCard::ShardCard(const juce::String& numberAndTitle, const juce::String& typeBadge, juce::Colour accent)
    : title(numberAndTitle), badge(typeBadge), accentColour(accent) {}

void ShardCard::paint(juce::Graphics& g) {
    auto b = getLocalBounds().toFloat();

    // 1. Dark card background
    g.setColour(juce::Colour(0xff131722));
    g.fillRoundedRectangle(b, 4.0f);

    // 2. Card border
    g.setColour(juce::Colour(0xff222b3d));
    g.drawRoundedRectangle(b.reduced(0.5f), 4.0f, 1.0f);

    // 3. Top accent stripe
    auto topStripe = b.removeFromTop(3.0f);
    g.setColour(accentColour);
    g.fillRoundedRectangle(topStripe, 2.0f);

    // 4. Header background
    auto headerRect = b.removeFromTop(23.0f);
    g.setColour(juce::Colour(0xff181e2b));
    g.fillRect(headerRect);
    g.setColour(juce::Colour(0xff222b3d));
    g.drawHorizontalLine(static_cast<int>(headerRect.getBottom()), 0.0f, b.getWidth());

    // 5. Header text
    g.setFont(juce::Font(juce::FontOptions(10.5f, juce::Font::bold)));
    g.setColour(juce::Colours::white);
    g.drawText(title, headerRect.reduced(6.0f, 0.0f), juce::Justification::centredLeft);

    g.setFont(juce::Font(juce::FontOptions(8.5f, juce::Font::plain)));
    g.setColour(juce::Colour(0xff8896aa));
    g.drawText(badge, headerRect.reduced(6.0f, 0.0f), juce::Justification::centredRight);
}

juce::Rectangle<int> ShardCard::getBodyBounds() const {
    return getLocalBounds().reduced(4, 4).withTrimmedTop(26);
}

// ==============================================================================
// 4. ShardContentComponent Implementation
// ==============================================================================
void ShardContentComponent::paint(juce::Graphics& g) {
    // Chassis faceplate background
    g.fillAll(juce::Colour(0xff0b0e14));

    // Top Cockpit Bar
    const int headerH = 36;
    g.setColour(juce::Colour(0xff121722));
    g.fillRect(0, 0, getWidth(), headerH);
    g.setColour(juce::Colour(0xff222b3d));
    g.drawHorizontalLine(headerH, 0.0f, static_cast<float>(getWidth()));

    // Diamond Logo & Brand Title
    g.setFont(juce::Font(juce::FontOptions(17.0f, juce::Font::bold)));
    g.setColour(juce::Colour(0xff00e5ff));
    g.drawText(juce::CharPointer_UTF8("\xe2\x97\x86"), 10, 0, 18, headerH, juce::Justification::centredLeft);

    g.setFont(juce::Font(juce::FontOptions(13.5f, juce::Font::bold)));
    g.drawText("THE SHARD", 32, 2, 90, 18, juce::Justification::centredLeft);

    g.setFont(juce::Font(juce::FontOptions(8.5f, juce::Font::plain)));
    g.setColour(juce::Colour(0xff7dd3fc));
    g.drawText("THE CULT OF KLANG", 33, 19, 120, 14, juce::Justification::centredLeft);

    // Canon Polarity Badges
    auto drawBadge = [&g](int x, int y, int w, int h, const juce::String& text, juce::Colour col) {
        g.setColour(juce::Colour(0xff181e2b));
        g.fillRoundedRectangle(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h), 2.0f);
        g.setColour(juce::Colour(0xff2d3748));
        g.drawRoundedRectangle(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h), 2.0f, 1.0f);
        g.setFont(juce::Font(juce::FontOptions(8.5f, juce::Font::bold)));
        g.setColour(col);
        g.drawText(text, x, y, w, h, juce::Justification::centred);
    };

    drawBadge(160, 7, 72, 22, "UNI: 6PM", juce::Colour(0xffffb300));
    drawBadge(236, 7, 70, 22, "BI: NOON", juce::Colour(0xff00d2ff));
    drawBadge(310, 7, 86, 22, "VOICE: MONO", juce::Colour(0xff10b981));
}

// ==============================================================================
// 5. TheShardAudioProcessorEditor Implementation
// ==============================================================================
TheShardAudioProcessorEditor::TheShardAudioProcessorEditor(TheKlangPlanterAudioProcessor& p)
    : KlangCoreEditor(p),
      audioProcessor(p),
      carrierTrackingSelector(juce::Colour(0xff00d2ff)),
      modTrackSelector(juce::Colour(0xff00d2ff)),
      modTypeSelector(juce::Colour(0xff00d2ff)),
      filterTypeSelector(juce::Colour(0xffffb300)),
      filterSlopeSelector(juce::Colour(0xffffb300)),
      pitchEnvTargetSelector(juce::Colour(0xffff7043)),
      limiterEnableSelector(juce::Colour(0xff10b981))
{
    setLookAndFeel(&knobLookAndFeel);
    statusBar.setVisible(false);
    scopeBuffer.resize(512, 0.0f);

    addAndMakeVisible(content);

    // Header Controls
    content.addAndMakeVisible(headerViz);

    triggerButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff00e5ff));
    triggerButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff0f1115));
    triggerButton.setTooltip("TRIGGER — Audition drum voice hit at full velocity.");
    triggerButton.onClick = [this]() {
        audioProcessor.getEngine().trigger(1.0f);
    };
    content.addAndMakeVisible(triggerButton);

    panicButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffe53935));
    panicButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    panicButton.setTooltip("PANIC — Instantly silence voice and reset synthesis state.");
    panicButton.onClick = [this]() {
        audioProcessor.panic();
    };
    content.addAndMakeVisible(panicButton);

    initButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e2535));
    initButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffc5d1e8));
    initButton.setTooltip("INIT — Reset all parameters to canonical defaults.");
    initButton.onClick = [this]() {
        audioProcessor.panic();
        audioProcessor.getEngine().setDefaultParameters();
        for (auto* param : audioProcessor.getParameters()) {
            if (auto* rp = dynamic_cast<juce::RangedAudioParameter*>(param)) {
                rp->setValueNotifyingHost(rp->getDefaultValue());
            }
        }
    };
    content.addAndMakeVisible(initButton);

    content.addAndMakeVisible(settingsButton);

    headerViz.onPanic = [this]() {
        audioProcessor.panic();
    };

    // Color Palette
    const juce::Colour colCyan   = juce::Colour(0xff00d2ff);
    const juce::Colour colAmber  = juce::Colour(0xffffb300);
    const juce::Colour colPurple = juce::Colour(0xffc084fc);
    const juce::Colour colCoral  = juce::Colour(0xffff7043);
    const juce::Colour colGreen  = juce::Colour(0xff10b981);

    // -------------------------------------------------------------------------
    // CARD 1: 1. GENERATOR (Cyan)
    // -------------------------------------------------------------------------
    cardGen = std::make_unique<ShardCard>("1. GENERATOR", "CARRIER + MOD", colCyan);
    content.addAndMakeVisible(cardGen.get());

    bindSelector(carrierTrackingSelector, carrierTrackingBox, "planter_carrier_tracking", { "MIDI", "Freq", "Note" }, 3);
    bindSelector(modTrackSelector, modTrackBox, "planter_mod_track", { "Fixed", "Follow", "FM" }, 3);
    bindSelector(modTypeSelector, modTypeBox, "planter_mod_type", { "Osc", "Cyclic", "Noise" }, 3);

    cardGen->addAndMakeVisible(carrierTrackingSelector);
    cardGen->addAndMakeVisible(modTrackSelector);
    cardGen->addAndMakeVisible(modTypeSelector);

    carrierTrackingBox.onChange = [this]() {
        carrierTrackingSelector.setSelectedIndex(carrierTrackingBox.getSelectedItemIndex(), juce::dontSendNotification);
        updateCarrierControls();
    };
    modTrackBox.onChange = [this]() {
        modTrackSelector.setSelectedIndex(modTrackBox.getSelectedItemIndex(), juce::dontSendNotification);
        updateModControls();
    };
    modTypeBox.onChange = [this]() {
        modTypeSelector.setSelectedIndex(modTypeBox.getSelectedItemIndex(), juce::dontSendNotification);
        updateModControls();
    };

    carrierPitchKnob = std::make_unique<ShardKnobCell>("PITCH", colCyan, true, 0.5,
        [this](double val) {
            int mode = carrierTrackingBox.getSelectedItemIndex();
            if (mode == 0) return formatSemi24(val);
            if (mode == 1) return formatCarrierFreqHz(val);
            return formatNoteDetail(val);
        },
        [this](const juce::String& text) {
            int mode = carrierTrackingBox.getSelectedItemIndex();
            if (mode == 0) return parseSemi24(text);
            if (mode == 1) return parseCarrierFreqHz(text);
            return parseNoteDetail(text);
        });

    carrierShapeKnob = std::make_unique<ShardKnobCell>("SHAPE", colCyan, false, 0.0, formatWaveshape, parseWaveshape);
    carrierDepthKnob = std::make_unique<ShardKnobCell>("MOD DEPTH", colCyan, true, 0.5, formatBipolarPercent, parseBipolarPercent);

    modShapeKnob = std::make_unique<ShardKnobCell>("M-SHAPE", colCyan, false, 0.0,
        [this](double val) {
            int t = modTypeBox.getSelectedItemIndex();
            if (t == 1 || t == 2) return formatBipolarPercent(val);
            return formatWaveshape(val);
        },
        [this](const juce::String& text) {
            int t = modTypeBox.getSelectedItemIndex();
            if (t == 1 || t == 2) return parseBipolarPercent(text);
            return parseWaveshape(text);
        });

    modSpeedKnob = std::make_unique<ShardKnobCell>("M-SPEED", colCyan, false, 0.5,
        [this](double val) {
            if (modTypeBox.getSelectedItemIndex() == 2) return formatFreqHz(val);
            if (modTrackBox.getSelectedItemIndex() == 2) return formatRatio(val);
            if (modTrackBox.getSelectedItemIndex() == 1) return formatSemi(val);
            return formatFreqHz(val);
        },
        [this](const juce::String& text) {
            if (modTypeBox.getSelectedItemIndex() == 2) return parseFreqHz(text);
            if (modTrackBox.getSelectedItemIndex() == 2) return parseRatio(text);
            if (modTrackBox.getSelectedItemIndex() == 1) return parseSemi(text);
            return parseFreqHz(text);
        });

    cardGen->addAndMakeVisible(carrierPitchKnob.get());
    cardGen->addAndMakeVisible(carrierShapeKnob.get());
    cardGen->addAndMakeVisible(carrierDepthKnob.get());
    cardGen->addAndMakeVisible(modShapeKnob.get());
    cardGen->addAndMakeVisible(modSpeedKnob.get());

    bindKnob(carrierPitchKnob, "planter_carrier_pitch");
    bindKnob(carrierShapeKnob, "planter_carrier_shape");
    bindKnob(carrierDepthKnob, "planter_carrier_depth");
    bindKnob(modShapeKnob,     "planter_mod_shape");
    bindKnob(modSpeedKnob,     "planter_mod_speed");

    // -------------------------------------------------------------------------
    // CARD 2: 2. 16-FILTER MATRIX (Amber)
    // -------------------------------------------------------------------------
    cardFilter = std::make_unique<ShardCard>("2. 16-FILTER MATRIX", "LADDER / SVF", colAmber);
    content.addAndMakeVisible(cardFilter.get());

    bindSelector(filterTypeSelector, filterTypeBox, "planter_filter_type", { "LPF", "BPF", "HPF", "BRF" }, 4);
    bindSelector(filterSlopeSelector, filterSlopeBox, "planter_filter_slope", { "6", "12", "18", "24", "36" }, 5);

    cardFilter->addAndMakeVisible(filterTypeSelector);
    cardFilter->addAndMakeVisible(filterSlopeSelector);

    filterCutoffKnob   = std::make_unique<ShardKnobCell>("CUTOFF", colAmber, false, 1.0, formatFreqHz, parseFreqHz);
    filterResoKnob     = std::make_unique<ShardKnobCell>("RESO", colAmber, false, 0.0, formatPercent, parsePercent);
    filterDriveKnob    = std::make_unique<ShardKnobCell>("PRE-DRIVE", colAmber, false, 0.5,
        [](double val) {
            float db = (val <= 0.5) ? static_cast<float>(-6.0 + val * 12.0)
                                    : static_cast<float>((val - 0.5) * 48.0);
            if (std::abs(db) < 0.05f) return juce::String("0.0 dB");
            return (db > 0.0f ? "+" : "") + juce::String(db, 1) + " dB";
        });
    filterEnvDepthKnob = std::make_unique<ShardKnobCell>("ENV AMT", colAmber, true, 0.5, formatFilterOctaves, parseFilterOctaves);
    filterEnvDecayKnob = std::make_unique<ShardKnobCell>("F-DECAY", colAmber, false, 0.3806, formatTimeMs, parseTimeMs);
    filterEnvSlopeKnob = std::make_unique<ShardKnobCell>("F-SLOPE", colAmber, false, 0.5886, formatSlope, parseSlope);

    cardFilter->addAndMakeVisible(filterCutoffKnob.get());
    cardFilter->addAndMakeVisible(filterResoKnob.get());
    cardFilter->addAndMakeVisible(filterDriveKnob.get());
    cardFilter->addAndMakeVisible(filterEnvDepthKnob.get());
    cardFilter->addAndMakeVisible(filterEnvDecayKnob.get());
    cardFilter->addAndMakeVisible(filterEnvSlopeKnob.get());

    bindKnob(filterCutoffKnob,   "planter_filter_cutoff");
    bindKnob(filterResoKnob,     "planter_filter_reso");
    bindKnob(filterDriveKnob,    "planter_filterenv_drive");
    bindKnob(filterEnvDepthKnob, "planter_filterenv_depth");
    bindKnob(filterEnvDecayKnob, "planter_filterenv_decay");
    bindKnob(filterEnvSlopeKnob, "planter_filterenv_slope");

    // -------------------------------------------------------------------------
    // CARD 3: 3. TRANSIENT & MOD (Purple)
    // -------------------------------------------------------------------------
    cardTrans = std::make_unique<ShardCard>("3. TRANSIENT & MOD", "NOISE / S&H", colPurple);
    content.addAndMakeVisible(cardTrans.get());

    noiseShRateKnob    = std::make_unique<ShardKnobCell>("S&H RATE", colPurple, false, 1.0, formatFreqHz, parseFreqHz);
    noiseFilterKnob    = std::make_unique<ShardKnobCell>("DJ FILTER", colPurple, true, 0.5, formatBipolarPercent, parseBipolarPercent);
    noiseDecayKnob     = std::make_unique<ShardKnobCell>("N-DECAY", colPurple, false, 0.3078, formatNoiseTimeMs, parseNoiseTimeMs);
    noiseCrossfadeKnob = std::make_unique<ShardKnobCell>("CROSSFADE", colPurple, true, 1.0, formatCrossfade, parseCrossfade);

    cardTrans->addAndMakeVisible(noiseShRateKnob.get());
    cardTrans->addAndMakeVisible(noiseFilterKnob.get());
    cardTrans->addAndMakeVisible(noiseDecayKnob.get());
    cardTrans->addAndMakeVisible(noiseCrossfadeKnob.get());

    bindKnob(noiseShRateKnob,    "planter_noise_sh_rate");
    bindKnob(noiseFilterKnob,    "planter_noise_filter");
    bindKnob(noiseDecayKnob,     "planter_noise_decay");
    bindKnob(noiseCrossfadeKnob, "planter_noise_crossfade");

    // -------------------------------------------------------------------------
    // CARD 4: 4. QUADRAMOD & PITCH (Coral)
    // -------------------------------------------------------------------------
    cardQuad = std::make_unique<ShardCard>("4. QUADRAMOD & PITCH", "PITCH ENV", colCoral);
    content.addAndMakeVisible(cardQuad.get());

    bindSelector(pitchEnvTargetSelector, pitchEnvTargetBox, "planter_pitchenv_target", { "Car", "Mod", "Both", "Opp" }, 4);
    cardQuad->addAndMakeVisible(pitchEnvTargetSelector);

    pitchEnvDepthKnob = std::make_unique<ShardKnobCell>("P-DEPTH", colCoral, true, 0.5, formatOctaves, parseOctaves);
    pitchEnvDecayKnob = std::make_unique<ShardKnobCell>("P-DECAY", colCoral, false, 0.3806, formatTimeMs, parseTimeMs);
    pitchEnvSlopeKnob = std::make_unique<ShardKnobCell>("P-SLOPE", colCoral, false, 0.5886, formatSlope, parseSlope);

    cardQuad->addAndMakeVisible(pitchEnvDepthKnob.get());
    cardQuad->addAndMakeVisible(pitchEnvDecayKnob.get());
    cardQuad->addAndMakeVisible(pitchEnvSlopeKnob.get());

    bindKnob(pitchEnvDepthKnob, "planter_pitchenv_depth");
    bindKnob(pitchEnvDecayKnob, "planter_pitchenv_decay");
    bindKnob(pitchEnvSlopeKnob, "planter_pitchenv_slope");

    // -------------------------------------------------------------------------
    // CARD 5: 5. AMP & MASTER (Green)
    // -------------------------------------------------------------------------
    cardAmp = std::make_unique<ShardCard>("5. AMP & MASTER", "VCA & LIMITER", colGreen);
    content.addAndMakeVisible(cardAmp.get());

    ampDriveKnob        = std::make_unique<ShardKnobCell>("DRIVE", colGreen, false, 0.5, formatAmpDriveDb, parseAmpDriveDb);
    ampPanKnob          = std::make_unique<ShardKnobCell>("PAN", colGreen, true, 0.5,
        [](double val) {
            int p = static_cast<int>(std::round((val - 0.5) * 200.0));
            if (p == 0) return juce::String("Center");
            return (p < 0 ? juce::String(-p) + "% L" : juce::String(p) + "% R");
        },
        parseBipolarPercent);
    ampVelFloorKnob     = std::make_unique<ShardKnobCell>("VELOCITY", colGreen, false, 0.5, formatVelocityFloor, parseVelocityFloor);
    ampVelSlopeKnob     = std::make_unique<ShardKnobCell>("V-SLOPE", colGreen, false, 0.75, formatSlope, parseSlope);

    ampEnvDecayKnob     = std::make_unique<ShardKnobCell>("A-DECAY", colGreen, false, 0.3806, formatTimeMs, parseTimeMs);
    ampEnvSlopeKnob     = std::make_unique<ShardKnobCell>("A-SLOPE", colGreen, false, 0.5886, formatSlope, parseSlope);
    ampEnvClapsKnob     = std::make_unique<ShardKnobCell>("CLAPS", colGreen, false, 0.0, formatClaps, parseClaps);
    ampEnvClapSpeedKnob = std::make_unique<ShardKnobCell>("CLAP SPD", colGreen, false, 2.0 / 14.0, formatClapSpeed, parseClapSpeed);

    bindSelector(limiterEnableSelector, limiterEnableBox, "planter_limiter_enable", { "Off", "On" }, 2);
    limiterGainKnob     = std::make_unique<ShardKnobCell>("GAIN", colGreen, false, 12.0 / 36.0, formatLimiterGain, parseLimiterGain);
    limiterThreshKnob   = std::make_unique<ShardKnobCell>("THRESH", colGreen, false, 1.0, formatLimiterThresh, parseLimiterThresh);
    limiterReleaseKnob  = std::make_unique<ShardKnobCell>("RELEASE", colGreen, false, 0.6296, formatLimiterRelease, parseLimiterRelease);

    cardAmp->addAndMakeVisible(ampDriveKnob.get());
    cardAmp->addAndMakeVisible(ampPanKnob.get());
    cardAmp->addAndMakeVisible(ampVelFloorKnob.get());
    cardAmp->addAndMakeVisible(ampVelSlopeKnob.get());
    cardAmp->addAndMakeVisible(ampEnvDecayKnob.get());
    cardAmp->addAndMakeVisible(ampEnvSlopeKnob.get());
    cardAmp->addAndMakeVisible(ampEnvClapsKnob.get());
    cardAmp->addAndMakeVisible(ampEnvClapSpeedKnob.get());
    cardAmp->addAndMakeVisible(limiterEnableSelector);
    cardAmp->addAndMakeVisible(limiterGainKnob.get());
    cardAmp->addAndMakeVisible(limiterThreshKnob.get());
    cardAmp->addAndMakeVisible(limiterReleaseKnob.get());

    bindKnob(ampDriveKnob,        "planter_amp_drive");
    bindKnob(ampPanKnob,          "planter_amp_pan");
    bindKnob(ampVelFloorKnob,     "planter_amp_vel_floor");
    bindKnob(ampVelSlopeKnob,     "planter_amp_vel_slope");
    bindKnob(ampEnvDecayKnob,     "planter_ampenv_decay");
    bindKnob(ampEnvSlopeKnob,     "planter_ampenv_slope");
    bindKnob(ampEnvClapsKnob,     "planter_ampenv_claps");
    bindKnob(ampEnvClapSpeedKnob, "planter_ampenv_clapspeed");
    bindKnob(limiterGainKnob,     "planter_limiter_gain");
    bindKnob(limiterThreshKnob,   "planter_limiter_thresh");
    bindKnob(limiterReleaseKnob,  "planter_limiter_release");

    updateCarrierControls();
    updateModControls();

    // Layout the fixed 1100x380 faceplate topography
    layoutFaceplate();

    // Lock aspect ratio to 1100:380 (2.8947) across resizing
    constrainer.setFixedAspectRatio(1100.0 / 380.0);
    constrainer.setSizeLimits(770, 266, 2750, 950);
    setConstrainer(&constrainer);

    // Initial default size
    setSize(1100, 380);
    setResizable(true, true);

    startTimerHz(30);
}

TheShardAudioProcessorEditor::~TheShardAudioProcessorEditor() {
    stopTimer();
    setLookAndFeel(nullptr);
}

void TheShardAudioProcessorEditor::bindKnob(std::unique_ptr<ShardKnobCell>& cell, const juce::String& paramId) {
    if (cell) {
        sliderAttachments.push_back(
            std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                audioProcessor.apvts, paramId, cell->getSlider()));
        cell->updateReadout();
    }
}

void TheShardAudioProcessorEditor::bindSelector(LedSelectorComponent& sel, juce::ComboBox& box,
                                                 const juce::String& paramId, const juce::StringArray& items, int columns)
{
    sel.setParamId(paramId);
    sel.setItems(items, columns);

    box.clear();
    for (int i = 0; i < items.size(); ++i) {
        box.addItem(items[i], i + 1);
    }
    box.setVisible(false);
    addChildComponent(box);

    boxAttachments.push_back(
        std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            audioProcessor.apvts, paramId, box));

    sel.onChange = [this, &box, paramId](int idx) {
        if (box.getSelectedItemIndex() != idx) {
            box.setSelectedItemIndex(idx, juce::sendNotificationSync);
        }
    };
}

int TheShardAudioProcessorEditor::syncSelector(juce::ComboBox& box, LedSelectorComponent& sel,
                                               const juce::String& paramId, int& lastVal)
{
    if (auto* param = audioProcessor.apvts.getParameter(paramId)) {
        int curVal = static_cast<int>(std::round(param->getValue() * static_cast<float>(sel.getNumItems() - 1)));
        if (curVal != lastVal) {
            lastVal = curVal;
            sel.setSelectedIndex(curVal, juce::dontSendNotification);
            box.setSelectedItemIndex(curVal, juce::dontSendNotification);
            return curVal;
        }
    }
    return -1;
}

void TheShardAudioProcessorEditor::updateCarrierControls() {
    int mode = carrierTrackingBox.getSelectedItemIndex();
    if (carrierPitchKnob) {
        if (mode == 0) { // MIDI
            carrierPitchKnob->getNameLabel().setText("OFFSET", juce::dontSendNotification);
            carrierPitchKnob->getSlider().isBipolar = true;
            carrierPitchKnob->getSlider().getDefaultValue = []() { return 0.5; };
        } else if (mode == 1) { // Freq
            carrierPitchKnob->getNameLabel().setText("FREQ", juce::dontSendNotification);
            carrierPitchKnob->getSlider().isBipolar = false;
            carrierPitchKnob->getSlider().getDefaultValue = []() {
                return static_cast<double>(std::log(55.0f / 20.0f) / std::log(24000.0f / 20.0f));
            };
        } else { // Note
            carrierPitchKnob->getNameLabel().setText("NOTE", juce::dontSendNotification);
            carrierPitchKnob->getSlider().isBipolar = false;
            carrierPitchKnob->getSlider().getDefaultValue = []() { return 33.0 / 127.0; };
        }
        carrierPitchKnob->getSlider().repaint();
        carrierPitchKnob->updateReadout();
    }
}

void TheShardAudioProcessorEditor::updateModControls() {
    int curType = modTypeBox.getSelectedItemIndex();
    int curTrack = modTrackBox.getSelectedItemIndex();

    if (modShapeKnob) {
        if (curType == 0) {
            modShapeKnob->getNameLabel().setText("SHAPE", juce::dontSendNotification);
            modShapeKnob->getSlider().isBipolar = false;
            modShapeKnob->getSlider().getDefaultValue = []() { return 0.0; };
        } else {
            modShapeKnob->getNameLabel().setText("DJ FILTER", juce::dontSendNotification);
            modShapeKnob->getSlider().isBipolar = true;
            modShapeKnob->getSlider().getDefaultValue = []() { return 0.5; };
        }
        modShapeKnob->getSlider().repaint();
        modShapeKnob->updateReadout();
    }

    if (modSpeedKnob) {
        if (curType == 2) {
            modSpeedKnob->getNameLabel().setText("S&H RATE", juce::dontSendNotification);
            modSpeedKnob->getSlider().getDefaultValue = []() { return 1.0; };
        } else {
            modSpeedKnob->getNameLabel().setText("SPEED", juce::dontSendNotification);
            if (curTrack == 0) {
                modSpeedKnob->getSlider().getDefaultValue = []() { return 0.50934; };
            } else {
                modSpeedKnob->getSlider().getDefaultValue = []() { return 0.5; };
            }
        }
        modSpeedKnob->getSlider().repaint();
        modSpeedKnob->updateReadout();
    }
}

void TheShardAudioProcessorEditor::timerCallback() {
    int curCarTrack = syncSelector(carrierTrackingBox, carrierTrackingSelector, "planter_carrier_tracking", lastCarrierTracking);
    if (curCarTrack >= 0) updateCarrierControls();

    int curModTrack = syncSelector(modTrackBox, modTrackSelector, "planter_mod_track", lastModTrack);
    int curModType  = syncSelector(modTypeBox, modTypeSelector, "planter_mod_type", lastModType);
    if (curModType >= 0 || curModTrack >= 0) updateModControls();

    syncSelector(filterTypeBox, filterTypeSelector, "planter_filter_type", lastFilterType);
    syncSelector(filterSlopeBox, filterSlopeSelector, "planter_filter_slope", lastFilterSlope);
    syncSelector(pitchEnvTargetBox, pitchEnvTargetSelector, "planter_pitchenv_target", lastPitchEnvTarget);
    syncSelector(limiterEnableBox, limiterEnableSelector, "planter_limiter_enable", lastLimiterEnable);

    // Fetch live audio oscilloscope and peak telemetry
    audioProcessor.getEngine().getScopeData(scopeBuffer.data(), static_cast<int>(scopeBuffer.size()));
    float peakL = audioProcessor.getEngine().getPeakL();
    float peakR = audioProcessor.getEngine().getPeakR();
    float limAct = audioProcessor.getLimiterActivity();

    headerViz.updateData(scopeBuffer.data(), static_cast<int>(scopeBuffer.size()), peakL, peakR, limAct);

    if (settingsModal && settingsModal->isVisible()) {
        settingsModal->setBounds(getLocalBounds());
    }
}

void TheShardAudioProcessorEditor::paint(juce::Graphics& g) {
    // Fill surround letterbox area with deep obsidian
    g.fillAll(juce::Colour(0xff07090e));
}

void TheShardAudioProcessorEditor::layoutFaceplate() {
    const int margin = 6;
    const int gap = 6;
    const int headerH = 36;
    const int cardY = headerH + margin;
    const int cardH = 380 - cardY - margin; // Fixed 332px height at 1100x380

    // Header buttons
    headerViz.setBounds(1100 - 480, 5, 210, 26);
    triggerButton.setBounds(1100 - 264, 5, 68, 26);
    panicButton.setBounds(1100 - 192, 5, 52, 26);
    initButton.setBounds(1100 - 136, 5, 46, 26);
    settingsButton.setBounds(1100 - 42, 5, 26, 26);

    // 5 Device Card Layout (Widths: 230, 230, 165, 165, 274 = 1064px + 24px gaps = 1088px)
    const int wGen = 230;
    const int wFlt = 230;
    const int wTra = 165;
    const int wQua = 165;
    const int wAmp = 274;

    int x = margin;
    if (cardGen)    { cardGen->setBounds(x, cardY, wGen, cardH);    x += wGen + gap; }
    if (cardFilter) { cardFilter->setBounds(x, cardY, wFlt, cardH); x += wFlt + gap; }
    if (cardTrans)  { cardTrans->setBounds(x, cardY, wTra, cardH);  x += wTra + gap; }
    if (cardQuad)   { cardQuad->setBounds(x, cardY, wQua, cardH);   x += wQua + gap; }
    if (cardAmp)    { cardAmp->setBounds(x, cardY, wAmp, cardH); }

    // -------------------------------------------------------------------------
    // CARD 1 INTERIOR
    // -------------------------------------------------------------------------
    if (cardGen) {
        auto b = cardGen->getBodyBounds();
        int topY = b.getY() + 4;
        int selW = (b.getWidth() - 6) / 3;
        carrierTrackingSelector.setBounds(b.getX(), topY, selW, 22);
        modTrackSelector.setBounds(b.getX() + selW + 3, topY, selW, 22);
        modTypeSelector.setBounds(b.getX() + (selW + 3) * 2, topY, selW, 22);

        int row1Y = topY + 28;
        int k3W = b.getWidth() / 3;
        if (carrierPitchKnob) carrierPitchKnob->setBounds(b.getX(), row1Y, k3W, 110);
        if (carrierShapeKnob) carrierShapeKnob->setBounds(b.getX() + k3W, row1Y, k3W, 110);
        if (carrierDepthKnob) carrierDepthKnob->setBounds(b.getX() + k3W * 2, row1Y, k3W, 110);

        int row2Y = row1Y + 116;
        int k2W = b.getWidth() / 2;
        if (modShapeKnob) modShapeKnob->setBounds(b.getX() + 6, row2Y, k2W - 12, 110);
        if (modSpeedKnob) modSpeedKnob->setBounds(b.getX() + k2W + 6, row2Y, k2W - 12, 110);
    }

    // -------------------------------------------------------------------------
    // CARD 2 INTERIOR
    // -------------------------------------------------------------------------
    if (cardFilter) {
        auto b = cardFilter->getBodyBounds();
        int topY = b.getY() + 4;
        int selW = (b.getWidth() - 4) / 2;
        filterTypeSelector.setBounds(b.getX(), topY, selW, 22);
        filterSlopeSelector.setBounds(b.getX() + selW + 4, topY, selW, 22);

        int row1Y = topY + 28;
        int k3W = b.getWidth() / 3;
        if (filterCutoffKnob) filterCutoffKnob->setBounds(b.getX(), row1Y, k3W, 110);
        if (filterResoKnob)   filterResoKnob->setBounds(b.getX() + k3W, row1Y, k3W, 110);
        if (filterDriveKnob)  filterDriveKnob->setBounds(b.getX() + k3W * 2, row1Y, k3W, 110);

        int row2Y = row1Y + 116;
        if (filterEnvDepthKnob) filterEnvDepthKnob->setBounds(b.getX(), row2Y, k3W, 110);
        if (filterEnvDecayKnob) filterEnvDecayKnob->setBounds(b.getX() + k3W, row2Y, k3W, 110);
        if (filterEnvSlopeKnob) filterEnvSlopeKnob->setBounds(b.getX() + k3W * 2, row2Y, k3W, 110);
    }

    // -------------------------------------------------------------------------
    // CARD 3 INTERIOR
    // -------------------------------------------------------------------------
    if (cardTrans) {
        auto b = cardTrans->getBodyBounds();
        int row1Y = b.getY() + 18;
        int k2W = b.getWidth() / 2;
        if (noiseShRateKnob)    noiseShRateKnob->setBounds(b.getX(), row1Y, k2W, 115);
        if (noiseFilterKnob)    noiseFilterKnob->setBounds(b.getX() + k2W, row1Y, k2W, 115);

        int row2Y = row1Y + 125;
        if (noiseDecayKnob)     noiseDecayKnob->setBounds(b.getX(), row2Y, k2W, 115);
        if (noiseCrossfadeKnob) noiseCrossfadeKnob->setBounds(b.getX() + k2W, row2Y, k2W, 115);
    }

    // -------------------------------------------------------------------------
    // CARD 4 INTERIOR
    // -------------------------------------------------------------------------
    if (cardQuad) {
        auto b = cardQuad->getBodyBounds();
        int topY = b.getY() + 4;
        pitchEnvTargetSelector.setBounds(b.getX(), topY, b.getWidth(), 22);

        int row1Y = topY + 28;
        if (pitchEnvDepthKnob) {
            int ctrW = 76;
            pitchEnvDepthKnob->setBounds(b.getX() + (b.getWidth() - ctrW) / 2, row1Y, ctrW, 110);
        }

        int row2Y = row1Y + 116;
        int k2W = b.getWidth() / 2;
        if (pitchEnvDecayKnob) pitchEnvDecayKnob->setBounds(b.getX(), row2Y, k2W, 110);
        if (pitchEnvSlopeKnob) pitchEnvSlopeKnob->setBounds(b.getX() + k2W, row2Y, k2W, 110);
    }

    // -------------------------------------------------------------------------
    // CARD 5 INTERIOR
    // -------------------------------------------------------------------------
    if (cardAmp) {
        auto b = cardAmp->getBodyBounds();
        int row1Y = b.getY() + 2;
        int k4W = b.getWidth() / 4;
        if (ampDriveKnob)    ampDriveKnob->setBounds(b.getX(), row1Y, k4W, 86);
        if (ampPanKnob)      ampPanKnob->setBounds(b.getX() + k4W, row1Y, k4W, 86);
        if (ampVelFloorKnob) ampVelFloorKnob->setBounds(b.getX() + k4W * 2, row1Y, k4W, 86);
        if (ampVelSlopeKnob) ampVelSlopeKnob->setBounds(b.getX() + k4W * 3, row1Y, k4W, 86);

        int row2Y = row1Y + 92;
        if (ampEnvDecayKnob)     ampEnvDecayKnob->setBounds(b.getX(), row2Y, k4W, 86);
        if (ampEnvSlopeKnob)     ampEnvSlopeKnob->setBounds(b.getX() + k4W, row2Y, k4W, 86);
        if (ampEnvClapsKnob)     ampEnvClapsKnob->setBounds(b.getX() + k4W * 2, row2Y, k4W, 86);
        if (ampEnvClapSpeedKnob) ampEnvClapSpeedKnob->setBounds(b.getX() + k4W * 3, row2Y, k4W, 86);

        int row3Y = row2Y + 94;
        int limBtnW = 56;
        limiterEnableSelector.setBounds(b.getX() + 2, row3Y + 16, limBtnW, 22);

        int limK3W = (b.getWidth() - limBtnW - 8) / 3;
        int limKX = b.getX() + limBtnW + 6;
        if (limiterGainKnob)    limiterGainKnob->setBounds(limKX, row3Y, limK3W, 76);
        if (limiterThreshKnob)  limiterThreshKnob->setBounds(limKX + limK3W, row3Y, limK3W, 76);
        if (limiterReleaseKnob) limiterReleaseKnob->setBounds(limKX + limK3W * 2, row3Y, limK3W, 76);
    }
}

void TheShardAudioProcessorEditor::resized() {
    float scale = std::min(static_cast<float>(getWidth()) / 1100.0f,
                           static_cast<float>(getHeight()) / 380.0f);
    scale = std::clamp(scale, 0.4f, 4.0f);

    float scaledW = 1100.0f * scale;
    float scaledH = 380.0f * scale;

    float offsetX = (static_cast<float>(getWidth()) - scaledW) * 0.5f;
    float offsetY = (static_cast<float>(getHeight()) - scaledH) * 0.5f;

    content.setBounds(0, 0, 1100, 380);
    content.setTransform(juce::AffineTransform::scale(scale).translated(offsetX, offsetY));

    if (settingsModal) {
        settingsModal->setBounds(getLocalBounds());
    }
}
