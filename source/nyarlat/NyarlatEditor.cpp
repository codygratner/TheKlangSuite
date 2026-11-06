#include "NyarlatEditor.h"

namespace Nyarlat {

NyarlatEditor::NyarlatEditor(NyarlatProcessor& proc)
    : AudioProcessorEditor(proc), processor_(proc)
{
    setLookAndFeel(&lnf_);

    // 1. 4:3 Aspect Ratio Lock for Host DAW & Renoise
    constrainer.setFixedAspectRatio(4.0 / 3.0);
    constrainer.setSizeLimits(juce::roundToInt(kBaseWidth  * rlyeh::ui::EditorScaleManager::kMinScale),
                              juce::roundToInt(kBaseHeight * rlyeh::ui::EditorScaleManager::kMinScale),
                              juce::roundToInt(kBaseWidth  * rlyeh::ui::EditorScaleManager::kMaxScale),
                              juce::roundToInt(kBaseHeight * rlyeh::ui::EditorScaleManager::kMaxScale));
    setConstrainer(&constrainer);
    setResizable(true, true);

    // 2. Preferred Scale Resolution
    const float sessionScale = processor_.getStoredEditorScale();
    effScale_ = rlyeh::ui::EditorScaleManager::getPreferredScale("nyarlat", sessionScale);
    expectedSize_ = { juce::roundToInt(kBaseWidth * effScale_), juce::roundToInt(kBaseHeight * effScale_) };

    // 3. Mount Content Container
    addAndMakeVisible(contentContainer);
    contentContainer.setBounds(0, 0, kBaseWidth, kBaseHeight);
    contentContainer.setInterceptsMouseClicks(false, true);
    contentContainer.onPaint = [this](juce::Graphics& g) { paintCanvas(g); };

    // Header Labels (Position A: x=14, y=4, w=140)
    titleLabel.setText("NYARLAT", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(juce::FontOptions(20.0f).withStyle("Bold")));
    titleLabel.setColour(juce::Label::textColourId, Colours::kOsc);
    contentContainer.addAndMakeVisible(titleLabel);

    subtitleLabel.setText("CRACKED CHIMES", juce::dontSendNotification);
    subtitleLabel.setFont(juce::Font(juce::FontOptions(9.5f).withStyle("Bold")));
    subtitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF8B949E));
    contentContainer.addAndMakeVisible(subtitleLabel);

    // Header Capsules (Positions B, C, D)
    presetStub_.setAccentColour(Colours::kOsc);
    contentContainer.addAndMakeVisible(presetStub_);
    contentContainer.addAndMakeVisible(headerInspector_);
    contentContainer.addAndMakeVisible(lifeModeToggle_);

    // Bind life_mode
    if (auto* p = processor_.getAPVTS().getParameter("life_mode")) {
        lifeModeAttachment_ = std::make_unique<juce::ParameterAttachment>(
            *p, [this](float v) {
                lifeModeToggle_.setIndex(static_cast<int>(v), juce::dontSendNotification);
            });
        lifeModeToggle_.onChange = [this](int idx) {
            if (lifeModeAttachment_) {
                lifeModeAttachment_->setValueAsCompleteGesture(static_cast<float>(idx));
            }
        };
        lifeModeAttachment_->sendInitialUpdate();
    }

    // 3D Vector Radar
    zCubeRadar_ = std::make_unique<ZCubeVisualizer>(proc);
    contentContainer.addAndMakeVisible(*zCubeRadar_);

    // Bitmask Waveform Visualizers (Osc 1, 2, 3)
    for (int i = 0; i < 3; ++i) {
        oscWaveViews_[i] = std::make_unique<BitmaskWaveVisualizer>(proc, i);
        contentContainer.addAndMakeVisible(*oscWaveViews_[i]);
    }

    // Vector ADSR Visualizer (Bay 5)
    vcaAdsrView_ = std::make_unique<VectorAdsrVisualizer>(proc);
    contentContainer.addAndMakeVisible(*vcaAdsrView_);

    auto makeKnob = [this](const juce::String& paramId, const juce::String& label, juce::Colour col) {
        auto item = std::make_unique<RotaryKnob>();
        setupKnob(*item, paramId, label, col);
        knobs_.push_back(std::move(item));
        return knobs_.back().get();
    };

    auto makeFader = [this](const juce::String& paramId, const juce::String& label, juce::Colour col) {
        auto item = std::make_unique<SliderFader>();
        setupFader(*item, paramId, label, col);
        faders_.push_back(std::move(item));
        return faders_.back().get();
    };

    auto makeCombo = [this](const juce::String& paramId, const juce::String& label) {
        auto item = std::make_unique<ComboBoxItem>();
        setupCombo(*item, paramId, label);
        combos_.push_back(std::move(item));
        return combos_.back().get();
    };

    auto makeButton = [this](const juce::String& paramId, const juce::String& label, juce::Colour col) {
        auto item = std::make_unique<ButtonItem>();
        setupButton(*item, paramId, label, col);
        buttons_.push_back(std::move(item));
        return buttons_.back().get();
    };

    namespace Ids = nyarlatParameters::Ids;

    // --- OSCILLATORS (Bay 1) ---
    // Osc 1
    makeCombo(Ids::osc1_bank, "O1 BANK");
    makeCombo(Ids::osc1_octave, "O1 OCT");
    makeButton(Ids::osc1_oct_lock, "LOCK", Colours::kOsc);
    makeKnob(Ids::osc1_wave, "WAVE", Colours::kOsc);
    makeKnob(Ids::osc1_mask, "MASK", Colours::kOsc);
    makeKnob(Ids::osc1_speed, "SPEED", Colours::kOsc);
    makeKnob(Ids::osc1_detune, "TUNE", Colours::kOsc);
    makeCombo(Ids::osc1_loop, "LOOP");

    // Osc 2
    makeCombo(Ids::osc2_bank, "O2 BANK");
    makeCombo(Ids::osc2_octave, "O2 OCT");
    makeButton(Ids::osc2_oct_lock, "LOCK", Colours::kOsc);
    makeButton(Ids::osc2_sync, "SYNC", Colours::kOsc);
    makeKnob(Ids::osc2_wave, "WAVE", Colours::kOsc);
    makeKnob(Ids::osc2_mask, "MASK", Colours::kOsc);
    makeKnob(Ids::osc2_speed, "SPEED", Colours::kOsc);
    makeKnob(Ids::osc2_detune, "TUNE", Colours::kOsc);
    makeCombo(Ids::osc2_loop, "LOOP");

    // Osc 3
    makeCombo(Ids::osc3_bank, "O3 BANK");
    makeCombo(Ids::osc3_octave, "O3 OCT");
    makeButton(Ids::osc3_oct_lock, "LOCK", Colours::kOsc);
    makeButton(Ids::osc3_sync, "SYNC", Colours::kOsc);
    makeKnob(Ids::osc3_wave, "WAVE", Colours::kOsc);
    makeKnob(Ids::osc3_mask, "MASK", Colours::kOsc);
    makeKnob(Ids::osc3_speed, "SPEED", Colours::kOsc);
    makeKnob(Ids::osc3_detune, "TUNE", Colours::kOsc);
    makeCombo(Ids::osc3_loop, "LOOP");

    // --- MIXER (Bay 2 Top) ---
    makeFader(Ids::osc1_level, "OSC 1", Colours::kMixer);
    makeFader(Ids::osc2_level, "OSC 2", Colours::kMixer);
    makeFader(Ids::osc3_level, "OSC 3", Colours::kMixer);
    makeFader(Ids::noise_level, "NOISE", Colours::kMixer);
    makeKnob(Ids::noise_tone, "TONE", Colours::kMixer);
    makeKnob(Ids::osc_spread, "SPREAD", Colours::kMixer);
    makeKnob(Ids::mixer_seethe, "WARMTH", Colours::kMixer);

    // --- CHAOS MASTER & ATTENUVERTERS (Bay 2 Bottom) ---
    makeKnob(Ids::chaos_rate, "RATE", Colours::kChaos);
    makeKnob(Ids::chaos_drift, "DRIFT", Colours::kChaos);
    makeKnob(Ids::chaos_poly_spread, "POLY", Colours::kChaos);

    // 15 Attenuverters
    makeKnob(Ids::rand_o1_wave, "O1 WAV", Colours::kChaos);
    makeKnob(Ids::rand_o1_mask, "O1 MSK", Colours::kChaos);
    makeKnob(Ids::rand_o1_speed, "O1 SPD", Colours::kChaos);

    makeKnob(Ids::rand_o2_wave, "O2 WAV", Colours::kChaos);
    makeKnob(Ids::rand_o2_mask, "O2 MSK", Colours::kChaos);
    makeKnob(Ids::rand_o2_speed, "O2 SPD", Colours::kChaos);

    makeKnob(Ids::rand_o3_wave, "O3 WAV", Colours::kChaos);
    makeKnob(Ids::rand_o3_mask, "O3 MSK", Colours::kChaos);
    makeKnob(Ids::rand_o3_speed, "O3 SPD", Colours::kChaos);

    makeKnob(Ids::rand_z_freq, "Z FREQ", Colours::kChaos);
    makeKnob(Ids::rand_z_morph, "Z MRPH", Colours::kChaos);
    makeKnob(Ids::rand_z_transform, "Z TRAN", Colours::kChaos);

    makeKnob(Ids::rand_seethe, "SEETHE", Colours::kChaos);
    makeKnob(Ids::rand_amp_decay, "AMP DEC", Colours::kChaos);
    makeKnob(Ids::rand_pan, "PAN", Colours::kChaos);

    // --- 14-POLE Z-FILTER (Bay 3) ---
    makeCombo(Ids::z_model, "MODEL");
    makeKnob(Ids::z_freq, "Z-FREQ", Colours::kFilter);
    makeKnob(Ids::z_morph, "Z-MORPH", Colours::kFilter);
    makeKnob(Ids::z_transform, "Z-TRANS", Colours::kFilter);

    makeKnob(Ids::filter_attack, "F-ATK", Colours::kFilter);
    makeKnob(Ids::filter_decay, "F-DEC", Colours::kFilter);
    makeKnob(Ids::filter_sustain, "F-SUS", Colours::kFilter);
    makeKnob(Ids::filter_release, "F-REL", Colours::kFilter);
    makeKnob(Ids::filter_env_depth, "ENV DEP", Colours::kFilter);
    makeKnob(Ids::filter_velocity, "VEL", Colours::kFilter);

    makeCombo(Ids::filter_lfo_shape, "LFO SHP");
    makeCombo(Ids::filter_lfo_dest, "LFO DST");
    makeKnob(Ids::filter_lfo_rate, "LFO SPD", Colours::kFilter);
    makeKnob(Ids::filter_lfo_depth, "LFO AMT", Colours::kFilter);
    makeKnob(Ids::filter_keytrack, "KEY TRK", Colours::kFilter);

    // --- VCA & VOICING (Bay 5) ---
    makeCombo(Ids::voice_mode, "VOICING");
    makeKnob(Ids::amp_attack, "ATK", Colours::kVca);
    makeKnob(Ids::amp_decay, "DEC", Colours::kVca);
    makeKnob(Ids::amp_sustain, "SUS", Colours::kVca);
    makeKnob(Ids::amp_release, "REL", Colours::kVca);
    makeKnob(Ids::amp_velocity, "VELOCITY", Colours::kVca);
    makeKnob(Ids::keytrack_rate, "KT RATE", Colours::kVca);
    makeKnob(Ids::glide_time, "GLIDE", Colours::kVca);
    makeKnob(Ids::mpe_slide_depth, "MPE SLD", Colours::kVca);
    makeKnob(Ids::mpe_pressure_depth, "MPE PRS", Colours::kVca);
    makeKnob(Ids::pitch_bend_range, "BEND", Colours::kVca);

    // --- MASTER & SPACE (Bay 6) ---
    makeKnob(Ids::bass_boost, "40Hz BASS", Colours::kMaster);
    makeKnob(Ids::treble_tame, "TREBLE", Colours::kMaster);
    makeKnob(Ids::reverb_time, "TIME", Colours::kMaster);
    makeKnob(Ids::reverb_damp, "DAMP", Colours::kMaster);
    makeKnob(Ids::reverb_mix, "REVERB", Colours::kMaster);
    makeKnob(Ids::alpha_chorus, "CHORUS", Colours::kMaster);
    makeKnob(Ids::osc_slop, "SLOP", Colours::kMaster);
    makeKnob(Ids::global_width, "WIDTH", Colours::kMaster);
    makeKnob(Ids::output_level, "VOLUME", Colours::kMaster);

    // Map components for unified Inspector & register mouse listeners
    contentContainer.addMouseListener(this, true);

    smoothedRandPips_.assign(knobs_.size(), 0.5f);

    // Lay out all 90 controls and visualizers into canonical 1160x870 coordinates
    layoutControls();

    // Unconditionally scale drag sensitivity for the initial scale factor
    applyDragSensitivity(effScale_);

    // Set initial size and trigger container scaling
    setSize(expectedSize_.x, expectedSize_.y);
    resized();

    startTimerHz(60);
}

NyarlatEditor::~NyarlatEditor() {
    setConstrainer(nullptr);
    stopTimer();
    contentContainer.removeMouseListener(this);
    setLookAndFeel(nullptr);
}

void NyarlatEditor::setupKnob(RotaryKnob& item, const juce::String& paramId, const juce::String& label, juce::Colour col) {
    item.paramId = paramId;
    item.label.setText(label, juce::dontSendNotification);
    item.label.setFont(juce::Font(juce::FontOptions(10.5f).withStyle("Bold")));
    item.label.setJustificationType(juce::Justification::centred);
    item.label.setColour(juce::Label::textColourId, Colours::kTextWhite);
    item.label.setInterceptsMouseClicks(false, false);
    contentContainer.addAndMakeVisible(item.label);

    item.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    item.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    item.slider.setColour(juce::Slider::rotarySliderFillColourId, col);
    item.slider.setColour(juce::Slider::thumbColourId, col);
    contentContainer.addAndMakeVisible(item.slider);

    if (auto* p = processor_.getAPVTS().getParameter(paramId)) {
        float defaultVal = p->convertFrom0to1(p->getDefaultValue());
        item.slider.setDoubleClickReturnValue(true, defaultVal);
        item.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor_.getAPVTS(), paramId, item.slider);
    }

    if (auto* info = findNyarlatControlInfo(paramId)) {
        controlMap_[&item.slider] = info;
    }

    item.slider.onValueChange = [this, s = &item.slider]() {
        if (s->isMouseButtonDown()) {
            inspectComponent(s);
        }
    };
}

void NyarlatEditor::setupFader(SliderFader& item, const juce::String& paramId, const juce::String& label, juce::Colour col) {
    item.paramId = paramId;
    item.label.setText(label, juce::dontSendNotification);
    item.label.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
    item.label.setJustificationType(juce::Justification::centred);
    item.label.setColour(juce::Label::textColourId, Colours::kTextWhite);
    item.label.setInterceptsMouseClicks(false, false);
    contentContainer.addAndMakeVisible(item.label);

    item.slider.setSliderStyle(juce::Slider::LinearVertical);
    item.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    item.slider.setColour(juce::Slider::trackColourId, col);
    item.slider.setColour(juce::Slider::thumbColourId, col);
    contentContainer.addAndMakeVisible(item.slider);

    if (auto* p = processor_.getAPVTS().getParameter(paramId)) {
        float defaultVal = p->convertFrom0to1(p->getDefaultValue());
        item.slider.setDoubleClickReturnValue(true, defaultVal);
        item.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor_.getAPVTS(), paramId, item.slider);
    }

    if (auto* info = findNyarlatControlInfo(paramId)) {
        controlMap_[&item.slider] = info;
    }

    item.slider.onValueChange = [this, s = &item.slider]() {
        if (s->isMouseButtonDown()) {
            inspectComponent(s);
        }
    };
}

void NyarlatEditor::setupCombo(ComboBoxItem& item, const juce::String& paramId, const juce::String& label) {
    item.paramId = paramId;
    item.label.setText(label, juce::dontSendNotification);
    item.label.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
    item.label.setJustificationType(juce::Justification::centredLeft);
    item.label.setColour(juce::Label::textColourId, Colours::kTextWhite);
    item.label.setInterceptsMouseClicks(false, false);
    contentContainer.addAndMakeVisible(item.label);

    item.combo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF161B22));
    item.combo.setColour(juce::ComboBox::outlineColourId, Colours::kBorder);
    item.combo.setColour(juce::ComboBox::textColourId, Colours::kTextWhite);
    contentContainer.addAndMakeVisible(item.combo);

    if (auto* p = processor_.getAPVTS().getParameter(paramId)) {
        if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(p)) {
            for (int i = 0; i < choiceParam->choices.size(); ++i) {
                item.combo.addItem(choiceParam->choices[i], i + 1);
            }
        }
        item.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            processor_.getAPVTS(), paramId, item.combo);
    }

    if (auto* info = findNyarlatControlInfo(paramId)) {
        controlMap_[&item.combo] = info;
    }
}

void NyarlatEditor::setupButton(ButtonItem& item, const juce::String& paramId, const juce::String& label, juce::Colour col) {
    item.paramId = paramId;
    item.button.setButtonText(label);
    item.button.setClickingTogglesState(true);
    item.button.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF161B22));
    item.button.setColour(juce::TextButton::buttonOnColourId, col.withAlpha(0.85f));
    item.button.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF8B949E));
    item.button.setColour(juce::TextButton::textColourOnId, juce::Colour(0xFF0D1117));
    contentContainer.addAndMakeVisible(item.button);

    if (auto* p = processor_.getAPVTS().getParameter(paramId)) {
        item.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            processor_.getAPVTS(), paramId, item.button);
    }

    if (auto* info = findNyarlatControlInfo(paramId)) {
        controlMap_[&item.button] = info;
    }

    item.button.onClick = [this, b = &item.button]() {
        inspectComponent(b);
    };
}

void NyarlatEditor::inspectComponent(juce::Component* comp) {
    if (comp == nullptr) return;
    auto it = controlMap_.find(comp);
    if (it != controlMap_.end() && it->second != nullptr) {
        const auto* info = it->second;
        HeaderInspector::Readout r;
        r.name = info->name;
        r.blurb = info->blurb;
        r.accent = info->accent;
        r.isDefault = false;

        if (auto* s = dynamic_cast<juce::Slider*>(comp)) {
            r.value = s->textFromValueFunction
                ? s->textFromValueFunction(s->getValue())
                : s->getTextFromValue(s->getValue());
        } else if (auto* cb = dynamic_cast<juce::ComboBox*>(comp)) {
            r.value = cb->getText();
        } else if (auto* tb = dynamic_cast<juce::TextButton*>(comp)) {
            r.value = tb->getToggleState() ? "ON" : "OFF";
        }
        headerInspector_.setReadout(r);
    }
}

void NyarlatEditor::mouseEnter(const juce::MouseEvent& e) {
    inspectComponent(e.eventComponent);
}

void NyarlatEditor::mouseExit(const juce::MouseEvent&) {
    headerInspector_.setReadoutDefault();
}

void NyarlatEditor::mouseDrag(const juce::MouseEvent& e) {
    inspectComponent(e.eventComponent);
}

void NyarlatEditor::paint(juce::Graphics& g) {
    g.fillAll(Colours::kChassisBg);
}

void NyarlatEditor::paintCanvas(juce::Graphics& g) {
    // Horizontal accent rule directly under header
    g.setColour(Colours::kOsc);
    g.fillRect(8, 38, kBaseWidth - 16, 2);

    auto drawBay = [&](int x, int y, int w, int h, const juce::String& title, juce::Colour badgeCol, int badgeWidth = 90) {
        auto r = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h));
        g.setColour(Colours::kBayBg);
        g.fillRoundedRectangle(r, 6.0f);
        g.setColour(Colours::kBorder);
        g.drawRoundedRectangle(r, 6.0f, 1.0f);

        if (title.isNotEmpty()) {
            float bw = static_cast<float>(badgeWidth);
            float bh = 18.0f;
            auto badge = juce::Rectangle<float>(static_cast<float>(x) + (static_cast<float>(w) - bw) * 0.5f, static_cast<float>(y + 6), bw, bh);
            g.setColour(badgeCol);
            g.fillRoundedRectangle(badge, 4.0f);
            g.setFont(juce::Font(juce::FontOptions(10.5f).withStyle("Bold")));
            g.setColour(juce::Colour(0xFF0D1117));
            g.drawText(title, badge, juce::Justification::centred);
        }
    };

    auto drawSubtlePod = [&](int x, int y, int w, int h, const juce::String& label = {}) {
        auto r = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h));
        g.setColour(juce::Colour(0xFF0B0F14));
        g.fillRoundedRectangle(r, 4.0f);
        g.setColour(juce::Colour(0xFF262D38));
        g.drawRoundedRectangle(r, 4.0f, 1.0f);
        if (label.isNotEmpty()) {
            g.setFont(juce::Font(juce::FontOptions(9.0f).withStyle("Bold")));
            g.setColour(juce::Colour(0xFF6E7681));
            g.drawText(label, x + 8, y + 2, w - 16, 12, juce::Justification::centredLeft);
        }
    };

    // 5 Primary Bays
    drawBay(14,  44, 277, 816, "OSCILLATORS", Colours::kOsc, 100);
    drawBay(303, 44, 277, 816, "MIX & CHAOS", Colours::kMixer, 100);
    drawBay(592, 44, 554, 410, "14-POLE Z-FILTER", Colours::kFilter, 134);
    drawBay(592, 462, 271, 398, "VCA & VOICING", Colours::kVca, 110);
    drawBay(875, 462, 271, 398, "MASTER & SPACE", Colours::kMaster, 124);

    // Subtle Pods inside Bay 1 (Oscillators)
    drawSubtlePod(20, 68,  265, 256);
    drawSubtlePod(20, 330, 265, 256);
    drawSubtlePod(20, 592, 265, 256);

    // Subtle Pods inside Bay 2 (Mix & Chaos)
    drawSubtlePod(309, 48,  265, 178, "SUB-MIXER");
    drawSubtlePod(309, 232, 265, 126, "MASTER ATTRACTOR");
    drawSubtlePod(309, 364, 265, 246, "OSC CHAOS MATRIX");
    drawSubtlePod(309, 616, 265, 238, "FILTER / SPACE CHAOS");
}

void NyarlatEditor::applyDragSensitivity(float s) {
    const int sens = juce::roundToInt(250.0f / s);
    for (auto& k : knobs_) {
        if (k) k->slider.setMouseDragSensitivity(sens);
    }
    for (auto& f : faders_) {
        if (f) f->slider.setMouseDragSensitivity(sens);
    }
}

void NyarlatEditor::resized() {
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
        applyDragSensitivity(s);
    }

    if (getPeer() != nullptr && juce::Point<int>{ w, h } != expectedSize_) {
        expectedSize_ = { w, h };
        scaleWriter_.note("nyarlat", s);
        processor_.setStoredEditorScale(s);
    }
}

void NyarlatEditor::layoutControls() {
    // 4-Block Header Strip Geometry (1160px canvas, Omega = 0px)
    titleLabel.setBounds      (14,  4, 140, 20);
    subtitleLabel.setBounds   (14, 24, 140, 12);
    presetStub_.setBounds     (162, 8, 168, 28);
    headerInspector_.setBounds(338, 8, 566, 28);
    lifeModeToggle_.setBounds (912, 8, 234, 28);

    auto findKnob = [this](const juce::String& id) -> RotaryKnob* {
        for (auto& k : knobs_) if (k->paramId == id) return k.get();
        return nullptr;
    };
    auto findFader = [this](const juce::String& id) -> SliderFader* {
        for (auto& f : faders_) if (f->paramId == id) return f.get();
        return nullptr;
    };
    auto findCombo = [this](const juce::String& id) -> ComboBoxItem* {
        for (auto& c : combos_) if (c->paramId == id) return c.get();
        return nullptr;
    };
    auto findButton = [this](const juce::String& id) -> ButtonItem* {
        for (auto& b : buttons_) if (b->paramId == id) return b.get();
        return nullptr;
    };

    auto placeKnob = [&](const juce::String& id, int x, int y, int size, int labelW = 0, int labelH = 12) {
        auto* k = findKnob(id);
        if (k == nullptr) return;
        if (labelW <= 0) labelW = size + 10;
        k->label.setBounds(x - (labelW - size) / 2, y, labelW, labelH);
        k->slider.setBounds(x, y + labelH + 2, size, size);
    };

    auto placeFader = [&](const juce::String& id, int x, int y, int w, int h, int labelH = 12) {
        auto* f = findFader(id);
        if (f == nullptr) return;
        f->label.setBounds(x - 4, y, w + 8, labelH);
        f->slider.setBounds(x, y + labelH + 2, w, h);
    };

    auto placeCombo = [&](const juce::String& id, int x, int y, int w, int h, int labelH = 12, bool labelAbove = false) {
        auto* c = findCombo(id);
        if (c == nullptr) return;
        if (labelAbove) {
            c->label.setBounds(x, y, w, labelH);
            c->combo.setBounds(x, y + labelH + 2, w, h);
        } else {
            c->combo.setBounds(x, y, w, h);
            c->label.setBounds(x, y - labelH, w, labelH);
        }
    };

    namespace Ids = nyarlatParameters::Ids;

    // ==============================================================================
    // Bay 1: Oscillators (14, 44, 277, 816)
    // ==============================================================================
    const std::array<int, 3> oscBaseY = { 74, 336, 598 };

    for (int o = 0; o < 3; ++o) {
        const int by = oscBaseY[o];
        const juce::String oPrefix = (o == 0) ? "osc1_" : (o == 1 ? "osc2_" : "osc3_");

        // Header Row: Bank Combo + Octave Combo + Oct Lock Button
        placeCombo(oPrefix + "bank", 24, by, 136, 22, 0, false);
        placeCombo(oPrefix + "octave", 164, by, 68, 22, 0, false);
        if (auto* b = findButton(oPrefix + "oct_lock")) {
            b->button.setBounds(236, by, 42, 22);
        }

        // Bitmask Waveform Preview
        if (oscWaveViews_[o] != nullptr) {
            oscWaveViews_[o]->setBounds(24, by + 26, 254, 58);
        }

        // Morph Row 1 (3 Knobs, 52px, overhead labels at by + 88)
        placeKnob(oPrefix + "wave",  24,  by + 88, 52);
        placeKnob(oPrefix + "mask",  125, by + 88, 52);
        placeKnob(oPrefix + "speed", 226, by + 88, 52);

        // Mod Row 2: Tune Knob + Loop Combo + Sync Button (Osc 2/3)
        placeKnob(oPrefix + "detune", 26, by + 154, 48);

        if (o == 0) {
            // Osc 1 Loop Combo (w=120)
            placeCombo(oPrefix + "loop", 96, by + 168, 160, 24, 12, false);
        } else {
            // Osc 2/3 Loop Combo + Sync Button
            placeCombo(oPrefix + "loop", 88, by + 168, 104, 24, 12, false);
            if (auto* b = findButton(oPrefix + "sync")) {
                b->button.setBounds(198, by + 168, 76, 24);
            }
        }
    }

    // ==============================================================================
    // Bay 2: Sub-Mixer & Chaos Matrix (303, 44, 277, 816)
    // ==============================================================================
    // Deck 2A: Sub-Mixer (4 Faders + Tone + Spread + Seethe Hero)
    const std::array<int, 4> faderX = { 314, 350, 386, 422 };
    placeFader(Ids::osc1_level,  faderX[0], 56, 26, 112);
    placeFader(Ids::osc2_level,  faderX[1], 56, 26, 112);
    placeFader(Ids::osc3_level,  faderX[2], 56, 26, 112);
    placeFader(Ids::noise_level, faderX[3], 56, 26, 112);

    placeKnob(Ids::noise_tone,  458, 56,  46);
    placeKnob(Ids::osc_spread,  518, 56,  46);
    placeKnob(Ids::mixer_seethe, 484, 124, 56, 68, 14); // Warmth Hero Knob

    // Deck 2B: Chaos Master (3 Knobs 56px)
    placeKnob(Ids::chaos_rate,        314, 246, 56, 60, 14);
    placeKnob(Ids::chaos_drift,       404, 246, 56, 60, 14);
    placeKnob(Ids::chaos_poly_spread, 494, 246, 56, 60, 14);

    // Chaos Matrix: 15 Attenuverters (5 rows of 3 knobs, 46px)
    // Pod 1 (Osc Mod): Rows 0, 1, 2
    const std::array<int, 3> chaosColX = { 314, 400, 486 };

    // Row 0: Osc 1
    placeKnob(Ids::rand_o1_wave,  chaosColX[0], 376, 46);
    placeKnob(Ids::rand_o1_mask,  chaosColX[1], 376, 46);
    placeKnob(Ids::rand_o1_speed, chaosColX[2], 376, 46);

    // Row 1: Osc 2
    placeKnob(Ids::rand_o2_wave,  chaosColX[0], 452, 46);
    placeKnob(Ids::rand_o2_mask,  chaosColX[1], 452, 46);
    placeKnob(Ids::rand_o2_speed, chaosColX[2], 452, 46);

    // Row 2: Osc 3
    placeKnob(Ids::rand_o3_wave,  chaosColX[0], 528, 46);
    placeKnob(Ids::rand_o3_mask,  chaosColX[1], 528, 46);
    placeKnob(Ids::rand_o3_speed, chaosColX[2], 528, 46);

    // Pod 2 (Filter/Space Mod): Rows 3, 4
    // Row 3: Z-Filter
    placeKnob(Ids::rand_z_freq,      chaosColX[0], 632, 46);
    placeKnob(Ids::rand_z_morph,     chaosColX[1], 632, 46);
    placeKnob(Ids::rand_z_transform, chaosColX[2], 632, 46);

    // Row 4: Space & VCA
    placeKnob(Ids::rand_seethe,    chaosColX[0], 708, 46);
    placeKnob(Ids::rand_amp_decay, chaosColX[1], 708, 46);
    placeKnob(Ids::rand_pan,       chaosColX[2], 708, 46);

    // ==============================================================================
    // Bay 3: 14-Pole Z-Filter (592, 44, 554, 410)
    // ==============================================================================
    placeCombo(Ids::z_model, 604, 68, 194, 24, 12, false);

    // 3 Primary Hero Knobs (64px)
    placeKnob(Ids::z_freq,      604, 102, 64, 62, 14);
    placeKnob(Ids::z_morph,     672, 102, 64, 62, 14);
    placeKnob(Ids::z_transform, 740, 102, 64, 62, 14);

    if (zCubeRadar_ != nullptr) {
        zCubeRadar_->setBounds(818, 56, 316, 156);
    }

    // Filter Envelope: 6 Knobs (50px, overhead labels at y = 230)
    placeKnob(Ids::filter_attack,    604,  230, 50);
    placeKnob(Ids::filter_decay,     690,  230, 50);
    placeKnob(Ids::filter_sustain,   776,  230, 50);
    placeKnob(Ids::filter_release,   862,  230, 50);
    placeKnob(Ids::filter_env_depth, 948,  230, 50);
    placeKnob(Ids::filter_velocity,  1034, 230, 50);

    // Filter Modulation: 2 Combos + 3 Knobs (50px)
    placeCombo(Ids::filter_lfo_shape, 604, 326, 114, 24, 12, false);
    placeCombo(Ids::filter_lfo_dest,  728, 326, 114, 24, 12, false);

    placeKnob(Ids::filter_lfo_rate,  862,  312, 50);
    placeKnob(Ids::filter_lfo_depth, 948,  312, 50);
    placeKnob(Ids::filter_keytrack,  1034, 312, 50);

    // ==============================================================================
    // Bay 5: VCA & Voicing (592, 462, 271, 398)
    // ==============================================================================
    placeCombo(Ids::voice_mode, 604, 482, 130, 22, 12, false);

    if (vcaAdsrView_ != nullptr) {
        vcaAdsrView_->setBounds(604, 514, 246, 64);
    }

    // Row 1: Amp Envelope (4 Knobs 46px, overhead labels at y = 590)
    placeKnob(Ids::amp_attack,  604, 590, 46);
    placeKnob(Ids::amp_decay,   666, 590, 46);
    placeKnob(Ids::amp_sustain, 728, 590, 46);
    placeKnob(Ids::amp_release, 790, 590, 46);

    // Row 2: Dynamics & Keytrack (3 Knobs 48px, overhead labels at y = 662)
    placeKnob(Ids::amp_velocity, 604, 662, 48);
    placeKnob(Ids::keytrack_rate, 686, 662, 48);
    placeKnob(Ids::glide_time,   768, 662, 48);

    // Row 3: MPE & Bend (3 Knobs 48px, overhead labels at y = 736)
    placeKnob(Ids::mpe_slide_depth,    604, 736, 48);
    placeKnob(Ids::mpe_pressure_depth, 686, 736, 48);
    placeKnob(Ids::pitch_bend_range,   768, 736, 48);

    // ==============================================================================
    // Bay 6: Master & Space (875, 462, 271, 398)
    // ==============================================================================
    // Row 1 (3 Knobs 52px, overhead labels at y = 488)
    placeKnob(Ids::bass_boost,  887,  488, 52);
    placeKnob(Ids::treble_tame, 970,  488, 52);
    placeKnob(Ids::reverb_time, 1053, 488, 52);

    // Row 2 (3 Knobs 52px, overhead labels at y = 574)
    placeKnob(Ids::reverb_damp,  887,  574, 52);
    placeKnob(Ids::reverb_mix,   970,  574, 52);
    placeKnob(Ids::alpha_chorus, 1053, 574, 52);

    // Row 3 (2 Knobs 52px + Volume Hero Knob 60px, overhead labels at y = 660)
    placeKnob(Ids::osc_slop,     887,  660, 52);
    placeKnob(Ids::global_width, 970,  660, 52);
    placeKnob(Ids::output_level, 1053, 656, 60, 68, 14);

    // ==============================================================================
    // Permanent Regression Guards (Claude Audit Invariant)
    // ==============================================================================
    for (auto& k : knobs_) {
        jassert(k && !k->slider.getBounds().isEmpty());
    }
    for (auto& f : faders_) {
        jassert(f && !f->slider.getBounds().isEmpty());
    }
    for (auto& c : combos_) {
        jassert(c && !c->combo.getBounds().isEmpty());
    }
    for (auto& b : buttons_) {
        jassert(b && !b->button.getBounds().isEmpty());
    }
    if (zCubeRadar_ != nullptr) {
        jassert(!zCubeRadar_->getBounds().isEmpty());
    }
    for (int o = 0; o < 3; ++o) {
        if (oscWaveViews_[o] != nullptr) {
            jassert(!oscWaveViews_[o]->getBounds().isEmpty());
        }
    }
    if (vcaAdsrView_ != nullptr) {
        jassert(!vcaAdsrView_->getBounds().isEmpty());
    }
}

void NyarlatEditor::timerCallback() {
    // 1. Repaint animated Bitmask Waveform previews
    for (auto& wv : oscWaveViews_) {
        if (wv != nullptr) wv->repaint();
    }
    // 2. Repaint Vector ADSR curve preview
    if (vcaAdsrView_ != nullptr) {
        vcaAdsrView_->repaint();
    }

    // 3. Dynamic modulation halos for Z-Cube hero knobs and chaos parameters
    auto tele = processor_.getVoiceTelemetry();
    bool anyVoiceActive = false;
    float activeX = 0.5f, activeY = 0.0f, activeZ = 0.0f;

    for (const auto& vt : tele) {
        if (vt.active) {
            anyVoiceActive = true;
            activeX = vt.x;
            activeY = vt.y;
            activeZ = vt.z;
            break;
        }
    }

    for (size_t i = 0; i < knobs_.size(); ++i) {
        auto* k = knobs_[i].get();
        if (!k) continue;

        if (k->paramId == nyarlatParameters::Ids::z_freq) {
            float targetPip = activeX;
            float& curPip = smoothedRandPips_[i];
            curPip += 0.25f * (targetPip - curPip);

            k->slider.getProperties().set("randLo", std::max(0.0f, curPip - 0.15f));
            k->slider.getProperties().set("randHi", std::min(1.0f, curPip + 0.15f));
            k->slider.getProperties().set("randPip", curPip);
            k->slider.repaint();
        } else if (k->paramId == nyarlatParameters::Ids::z_morph) {
            float targetPip = activeY;
            float& curPip = smoothedRandPips_[i];
            curPip += 0.25f * (targetPip - curPip);

            k->slider.getProperties().set("modLo", std::max(0.0f, curPip - 0.12f));
            k->slider.getProperties().set("modHi", std::min(1.0f, curPip + 0.12f));
            k->slider.getProperties().set("modPip", curPip);
            k->slider.repaint();
        } else if (k->paramId == nyarlatParameters::Ids::z_transform) {
            float targetPip = activeZ;
            float& curPip = smoothedRandPips_[i];
            curPip += 0.25f * (targetPip - curPip);

            k->slider.getProperties().set("modLo", std::max(0.0f, curPip - 0.12f));
            k->slider.getProperties().set("modHi", std::min(1.0f, curPip + 0.12f));
            k->slider.getProperties().set("modPip", curPip);
            k->slider.repaint();
        }
    }
}

} // namespace Nyarlat
