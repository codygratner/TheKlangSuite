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
    setSize(expectedSize_.x, expectedSize_.y);

    // 3. Mount Content Container
    addAndMakeVisible(contentContainer);
    contentContainer.setBounds(0, 0, kBaseWidth, kBaseHeight);
    contentContainer.setInterceptsMouseClicks(false, true);

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

    auto makeKnob = [this](const juce::String& paramId, const juce::String& label, juce::Colour col) {
        auto item = std::make_unique<RotaryKnob>();
        item->paramId = paramId;
        setupKnob(*item, paramId, label, col);
        knobs_.push_back(std::move(item));
        return knobs_.back().get();
    };

    auto makeFader = [this](const juce::String& paramId, const juce::String& label, juce::Colour col) {
        auto item = std::make_unique<SliderFader>();
        item->paramId = paramId;
        setupFader(*item, paramId, label, col);
        faders_.push_back(std::move(item));
        return faders_.back().get();
    };

    auto makeCombo = [this](const juce::String& paramId, const juce::String& label) {
        auto item = std::make_unique<ComboBoxItem>();
        item->paramId = paramId;
        setupCombo(*item, paramId, label);
        combos_.push_back(std::move(item));
        return combos_.back().get();
    };

    namespace Ids = nyarlatParameters::Ids;

    // --- OSCILLATORS (Bay 1) ---
    makeCombo(Ids::osc1_bank, "O1 BANK");
    makeKnob(Ids::osc1_wave, "WAVE", Colours::kOsc);
    makeKnob(Ids::osc1_mask, "MASK", Colours::kOsc);
    makeKnob(Ids::osc1_speed, "SPEED", Colours::kOsc);
    makeCombo(Ids::osc1_octave, "OCT");
    makeKnob(Ids::osc1_detune, "TUNE", Colours::kOsc);

    makeCombo(Ids::osc2_bank, "O2 BANK");
    makeKnob(Ids::osc2_wave, "WAVE", Colours::kOsc);
    makeKnob(Ids::osc2_mask, "MASK", Colours::kOsc);
    makeKnob(Ids::osc2_speed, "SPEED", Colours::kOsc);
    makeCombo(Ids::osc2_octave, "OCT");
    makeKnob(Ids::osc2_detune, "TUNE", Colours::kOsc);

    makeCombo(Ids::osc3_bank, "O3 BANK");
    makeKnob(Ids::osc3_wave, "WAVE", Colours::kOsc);
    makeKnob(Ids::osc3_mask, "MASK", Colours::kOsc);
    makeKnob(Ids::osc3_speed, "SPEED", Colours::kOsc);
    makeCombo(Ids::osc3_octave, "OCT");
    makeKnob(Ids::osc3_detune, "TUNE", Colours::kOsc);

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
    makeKnob(Ids::rand_o1_mask, "O1 MSK", Colours::kChaos);
    makeKnob(Ids::rand_o1_speed, "O1 SPD", Colours::kChaos);
    makeKnob(Ids::rand_o1_pitch, "O1 PCH", Colours::kChaos);

    makeKnob(Ids::rand_o2_mask, "O2 MSK", Colours::kChaos);
    makeKnob(Ids::rand_o2_speed, "O2 SPD", Colours::kChaos);
    makeKnob(Ids::rand_o2_pitch, "O2 PCH", Colours::kChaos);

    makeKnob(Ids::rand_o3_mask, "O3 MSK", Colours::kChaos);
    makeKnob(Ids::rand_o3_speed, "O3 SPD", Colours::kChaos);
    makeKnob(Ids::rand_o3_pitch, "O3 PCH", Colours::kChaos);

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
    makeKnob(Ids::filter_velocity, "ACCENT", Colours::kFilter);

    makeKnob(Ids::filter_lfo_rate, "LFO SPD", Colours::kFilter);
    makeKnob(Ids::filter_lfo_depth, "LFO AMT", Colours::kFilter);
    makeKnob(Ids::filter_keytrack, "KEY TRK", Colours::kFilter);

    // --- VCA & VOICING (Bay 5) ---
    makeKnob(Ids::amp_attack, "ATTACK", Colours::kVca);
    makeKnob(Ids::amp_decay, "DECAY", Colours::kVca);
    makeKnob(Ids::amp_sustain, "SUSTAIN", Colours::kVca);
    makeKnob(Ids::amp_release, "RELEASE", Colours::kVca);
    makeKnob(Ids::amp_velocity, "VELOCITY", Colours::kVca);
    makeKnob(Ids::keytrack_rate, "KT RATE", Colours::kVca);
    makeKnob(Ids::glide_time, "GLIDE", Colours::kVca);
    makeCombo(Ids::voice_mode, "VOICING");

    // --- MASTER & SPACE (Bay 6) ---
    makeKnob(Ids::bass_boost, "40Hz BASS", Colours::kMaster);
    makeKnob(Ids::treble_tame, "TREBLE", Colours::kMaster);
    makeKnob(Ids::reverb_time, "TIME", Colours::kMaster);
    makeKnob(Ids::reverb_mix, "REVERB", Colours::kMaster);
    makeKnob(Ids::alpha_chorus, "CHORUS", Colours::kMaster);
    makeKnob(Ids::osc_slop, "SLOP", Colours::kMaster);
    makeKnob(Ids::global_width, "WIDTH", Colours::kMaster);
    makeKnob(Ids::output_level, "VOLUME", Colours::kMaster);

    // Map components for unified Inspector & register mouse listeners
    contentContainer.addMouseListener(this, true);

    smoothedRandPips_.assign(knobs_.size(), 0.5f);

    startTimerHz(60);
}

NyarlatEditor::~NyarlatEditor() {
    stopTimer();
    contentContainer.removeMouseListener(this);
    setLookAndFeel(nullptr);
}

void NyarlatEditor::setupKnob(RotaryKnob& item, const juce::String& paramId, const juce::String& label, juce::Colour col) {
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
    paintCanvas(g);
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

    drawBay(14,  44, 277, 816, "OSCILLATORS", Colours::kOsc, 100);
    drawBay(303, 44, 277, 816, "MIX & CHAOS", Colours::kMixer, 100);
    drawBay(592, 44, 554, 410, "14-POLE Z-FILTER", Colours::kFilter, 134);
    drawBay(592, 462, 271, 398, "VCA & VOICING", Colours::kVca, 110);
    drawBay(875, 462, 271, 398, "MASTER & SPACE", Colours::kMaster, 124);
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
        const int sens = juce::roundToInt(250.0f / s);
        for (auto& k : knobs_) {
            if (k) k->slider.setMouseDragSensitivity(sens);
        }
        for (auto& f : faders_) {
            if (f) f->slider.setMouseDragSensitivity(sens);
        }
    }

    if (getPeer() != nullptr && juce::Point<int>{ w, h } != expectedSize_) {
        expectedSize_ = { w, h };
        scaleWriter_.note("nyarlat", s);
        processor_.setStoredEditorScale(s);
    }

    // 4-Block Header Strip Geometry (1160px canvas, Omega = 0px)
    titleLabel.setBounds      (14,  4, 140, 20);
    subtitleLabel.setBounds   (14, 24, 140, 12);
    presetStub_.setBounds     (162, 8, 168, 28);
    headerInspector_.setBounds(338, 8, 566, 28);
    lifeModeToggle_.setBounds (912, 8, 234, 28);

    int comboIdx = 0;
    int knobIdx = 0;
    int faderIdx = 0;

    auto placeKnob = [&](int x, int y, int size, int labelW = 0, int labelH = 14) {
        if (knobIdx >= static_cast<int>(knobs_.size())) return;
        if (labelW <= 0) labelW = size + 8;
        knobs_[knobIdx]->slider.setBounds(x, y, size, size);
        knobs_[knobIdx]->label.setBounds(x - (labelW - size) / 2, y + size, labelW, labelH);
        knobIdx++;
    };

    auto placeFader = [&](int x, int y, int w, int h, int labelH = 14) {
        if (faderIdx >= static_cast<int>(faders_.size())) return;
        faders_[faderIdx]->slider.setBounds(x, y, w, h);
        faders_[faderIdx]->label.setBounds(x - 4, y + h + 2, w + 8, labelH);
        faderIdx++;
    };

    auto placeCombo = [&](int x, int y, int w, int h, int labelH = 16) {
        if (comboIdx >= static_cast<int>(combos_.size())) return;
        combos_[comboIdx]->combo.setBounds(x, y, w, h);
        combos_[comboIdx]->label.setBounds(x, y - labelH, w, labelH);
        comboIdx++;
    };

    // --- Bay 1: Oscillators (14, 44, 277, 816) ---
    int oscY = 74;
    int oscH = 260;
    for (int o = 0; o < 3; ++o) {
        int by = oscY + o * oscH;
        placeCombo(24, by, 160, 22);
        placeKnob(28, by + 36, 54);
        placeKnob(92, by + 36, 54);
        placeKnob(156, by + 36, 54);
        placeCombo(194, by, 86, 22);
        placeKnob(220, by + 36, 54);
    }

    // --- Bay 2 Top: Mixer (303, 44, 277, 816) ---
    int fx = 314;
    for (int f = 0; f < 4; ++f) {
        placeFader(fx + f * 42, 74, 34, 110);
    }
    placeKnob(484, 74, 42, 42);  // Noise Tone
    placeKnob(534, 74, 42, 42);  // Osc Spread
    placeKnob(510, 134, 52); // Seethe

    // --- Bay 2 Bottom: Chaos Master & 15 Attenuverters ---
    int cx = 314;
    int cy = 230;
    placeKnob(cx, cy, 54);       // Rate
    placeKnob(cx + 90, cy, 54);  // Drift
    placeKnob(cx + 180, cy, 54); // Poly Spread

    // 15 Attenuverter matrix (5 pods x 3 knobs)
    int rdmY = 324;
    int rdmRowH = 70;
    for (int r = 0; r < 5; ++r) {
        int ry = rdmY + r * rdmRowH;
        for (int c = 0; c < 3; ++c) {
            placeKnob(cx + c * 86, ry, 48);
        }
    }

    // --- Bay 3: 14-Pole Z-Filter (592, 44, 554, 410) ---
    placeCombo(604, 74, 180, 24); // Z-Model

    // 3 Primary Hero knobs: Freq, Morph, Transform
    placeKnob(604, 114, 62, 60); // Z-Freq
    placeKnob(670, 114, 62, 60); // Z-Morph
    placeKnob(736, 114, 62, 60); // Z-Transform

    if (zCubeRadar_ != nullptr) {
        zCubeRadar_->setBounds(810, 60, 324, 160);
    }

    // Filter Envelope & Modulation: 9 knobs
    int fey = 240;
    int fex = 604;
    for (int k = 0; k < 9; ++k) {
        int col = k % 5;
        int row = k / 5;
        int kx = fex + col * 105;
        int ky = fey + row * 76;
        placeKnob(kx, ky, 52);
    }

    // --- Bay 5: VCA & Voicing (592, 462, 271, 398) ---
    int vy = 500;
    for (int k = 0; k < 7; ++k) {
        int col = k % 3;
        int row = k / 3;
        int kx = 604 + col * 86;
        int ky = vy + 38 + row * 72;
        placeKnob(kx, ky, 52);
    }
    placeCombo(604, vy, 120, 22); // Voicing mode

    // --- Bay 6: Master & Space (875, 462, 271, 398) ---
    int my = 500;
    for (int k = 0; k < 8; ++k) {
        int col = k % 3;
        int row = k / 3;
        int mx = 886 + col * 86;
        int ky = my + row * 74;
        placeKnob(mx, ky, 54);
    }
}

void NyarlatEditor::timerCallback() {
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

    // Dynamic modulation halos for Z-Cube hero knobs and chaos parameters
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
