#include "NyarlatEditor.h"

#include "../dsp/common/MasterTaper.h"



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



    // Header Capsules: Universal StepperAuditionBar (replaces stub), Inspector, Lifeform Toggle

    stepperAuditionBar_.setAccentColour(Colours::kOsc);

    contentContainer.addAndMakeVisible(stepperAuditionBar_);

    contentContainer.addAndMakeVisible(headerInspector_);

    contentContainer.addAndMakeVisible(lifeModeToggle_);



    auto updatePresetDisplay = [this]() {

        auto& pm = processor_.getPresetManager();

        auto presets = pm.getAvailablePresets();

        int idx = pm.getCurrentIndex();

        juce::String name = pm.getActivePresetName();

        if (name.isEmpty()) name = (idx >= 0 && idx < presets.size()) ? presets[idx].name : "INIT VOID";

        stepperAuditionBar_.setItem(idx, name, presets.size());

    };



    stepperAuditionBar_.onSelectIndex = [this, updatePresetDisplay](int idx) {

        processor_.getPresetManager().loadPresetByIndex(idx, processor_.getAPVTS());

        updatePresetDisplay();

    };



    stepperAuditionBar_.onBuildMenu = [this](juce::PopupMenu& menu) {

        auto& pm = processor_.getPresetManager();

        auto presets = pm.getAvailablePresets();

        std::map<juce::String, juce::Array<std::pair<int, juce::String>>> bankMap;

        for (int i = 0; i < presets.size(); ++i) {

            juce::String b = presets[i].bank.isNotEmpty() ? presets[i].bank : "General";

            bankMap[b].add({i, presets[i].name});

        }

        for (const auto& [bName, list] : bankMap) {

            juce::PopupMenu sub;

            for (const auto& [pIdx, pName] : list) {

                sub.addItem(pIdx + 1, pName, true, pIdx == pm.getCurrentIndex());

            }

            menu.addSubMenu(bName, sub);

        }

    };



    updatePresetDisplay();



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



    // Additive Waveform Visualizers (Osc 1, 2, 3)

    for (int i = 0; i < 3; ++i) {

        oscWaveViews_[i] = std::make_unique<BitmaskWaveVisualizer>(proc, i);

        contentContainer.addAndMakeVisible(*oscWaveViews_[i]);

    }



    // Vector ADSR Visualizer (Bay 2)

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



    auto makeStepper = [this](const juce::String& paramId, const juce::String& label, juce::Colour col) {

        auto item = std::make_unique<StepperItem>();

        setupStepper(*item, paramId, label, col);

        steppers_.push_back(std::move(item));

        return steppers_.back().get();

    };



    auto makeButton = [this](const juce::String& paramId, const juce::String& label, juce::Colour col) {

        auto item = std::make_unique<ButtonItem>();

        setupButton(*item, paramId, label, col);

        buttons_.push_back(std::move(item));

        return buttons_.back().get();

    };



    auto makeRadio = [this](const juce::String& paramId, juce::Colour col, float fontSize = 10.0f) {

        auto item = std::make_unique<RadioItem>();

        setupRadio(*item, paramId, col, fontSize);

        radios_.push_back(std::move(item));

        return &radios_.back()->radio;

    };



    namespace Ids = nyarlatParameters::Ids;



    // --- OSCILLATORS (Bay 1) ---

    // loopTags_ retired per UI overhaul (Lock button replaces LOOP label at x=28)



    osc1MasterButton_.setButtonText("MSTR");

    osc1MasterButton_.setEnabled(false);

    osc1MasterButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF161B22));

    osc1MasterButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF6E7681));

    contentContainer.addAndMakeVisible(osc1MasterButton_);



    // Osc 1

    makeStepper(Ids::osc1_bank, "O1 BANK", Colours::kOsc);

    makeRadio(Ids::osc1_octave, Colours::kOsc, 12.5f);

    makeButton(Ids::osc1_speed_lock, "LOCK", juce::Colour(0xFFFFD700));

    makeKnob(Ids::osc1_morph, "MORPH", Colours::kOsc);

    makeKnob(Ids::osc1_odd_even, "ODD/EV", Colours::kOsc);

    makeKnob(Ids::osc1_tilt, "TILT", Colours::kOsc);

    makeKnob(Ids::osc1_formant, "FORM", Colours::kOsc);

    makeKnob(Ids::osc1_mask, "MASK", Colours::kOsc);

    makeKnob(Ids::osc1_speed, "SPEED", Colours::kOsc);

    for (int o = 0; o < 3; ++o) {

        const juce::String spId = (o == 0) ? Ids::osc1_speed : (o == 1 ? Ids::osc2_speed : Ids::osc3_speed);

        const juce::String lockId = (o == 0) ? Ids::osc1_speed_lock : (o == 1 ? Ids::osc2_speed_lock : Ids::osc3_speed_lock);

        for (auto& k : knobs_) {

            if (k->paramId == spId) {

                k->slider.textFromValueFunction = [this, lockId](double val) {

                    const bool locked = *processor_.getAPVTS().getRawParameterValue(lockId) > 0.5f;

                    if (locked) {

                        const float u = juce::jlimit(0.0f, 1.0f, static_cast<float>(val) * 0.1f);

                        const int n = 1 + static_cast<int>(std::round(63.0f * u));

                        return juce::String(n) + "/16";

                    }

                    return juce::String::formatted("%.2f Hz", val);

                };

                k->slider.updateText();

                break;

            }

        }

    }

    makeKnob(Ids::osc1_detune, "TUNE", Colours::kOsc);

    makeRadio(Ids::osc1_loop, Colours::kOsc);



    // Osc 2

    makeStepper(Ids::osc2_bank, "O2 BANK", Colours::kOsc);

    makeRadio(Ids::osc2_octave, Colours::kOsc, 12.5f);

    makeButton(Ids::osc2_speed_lock, "LOCK", juce::Colour(0xFFFFD700));

    makeButton(Ids::osc2_sync, "SYNC", Colours::kOsc);

    makeKnob(Ids::osc2_morph, "MORPH", Colours::kOsc);

    makeKnob(Ids::osc2_odd_even, "ODD/EV", Colours::kOsc);

    makeKnob(Ids::osc2_tilt, "TILT", Colours::kOsc);

    makeKnob(Ids::osc2_formant, "FORM", Colours::kOsc);

    makeKnob(Ids::osc2_mask, "MASK", Colours::kOsc);

    makeKnob(Ids::osc2_speed, "SPEED", Colours::kOsc);

    makeKnob(Ids::osc2_detune, "TUNE", Colours::kOsc);

    makeRadio(Ids::osc2_loop, Colours::kOsc);



    // Osc 3

    makeStepper(Ids::osc3_bank, "O3 BANK", Colours::kOsc);

    makeRadio(Ids::osc3_octave, Colours::kOsc, 12.5f);

    makeButton(Ids::osc3_speed_lock, "LOCK", juce::Colour(0xFFFFD700));

    makeButton(Ids::osc3_sync, "SYNC", Colours::kOsc);

    makeKnob(Ids::osc3_morph, "MORPH", Colours::kOsc);

    makeKnob(Ids::osc3_odd_even, "ODD/EV", Colours::kOsc);

    makeKnob(Ids::osc3_tilt, "TILT", Colours::kOsc);

    makeKnob(Ids::osc3_formant, "FORM", Colours::kOsc);

    makeKnob(Ids::osc3_mask, "MASK", Colours::kOsc);

    makeKnob(Ids::osc3_speed, "SPEED", Colours::kOsc);

    makeKnob(Ids::osc3_detune, "TUNE", Colours::kOsc);

    makeRadio(Ids::osc3_loop, Colours::kOsc);



    // --- BAY 2: SILKY ANALOG FILTER & VOICING ---

    makeCombo(Ids::filter_slope, "SLOPE");

    makeKnob(Ids::filter_cutoff, "CUTOFF", Colours::kFilter);

    makeKnob(Ids::filter_resonance, "RESO", Colours::kFilter);

    makeKnob(Ids::filter_drive, "DRIVE", Colours::kFilter);

    makeKnob(Ids::filter_attack, "F-ATK", Colours::kFilter);

    makeKnob(Ids::filter_decay, "F-DEC", Colours::kFilter);

    makeKnob(Ids::filter_sustain, "F-SUS", Colours::kFilter);

    makeKnob(Ids::filter_release, "F-REL", Colours::kFilter);

    makeKnob(Ids::filter_env_depth, "ENV DEP", Colours::kFilter);

    makeKnob(Ids::filter_velocity, "VEL", Colours::kFilter);

    makeKnob(Ids::filter_keytrack, "KEY TRK", Colours::kFilter);

    makeKnob(Ids::noise_hammer_snap, "SNAP", Colours::kFilter);



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



    // --- BAY 3: MIXER, 18-CH CHAOS & MASTER DECK ---

    makeFader(Ids::osc1_level, "OSC 1", Colours::kMixer);

    makeFader(Ids::osc2_level, "OSC 2", Colours::kMixer);

    makeFader(Ids::osc3_level, "OSC 3", Colours::kMixer);

    makeFader(Ids::noise_level, "NOISE", Colours::kMixer);

    makeKnob(Ids::noise_tone, "TONE", Colours::kMixer);

    makeKnob(Ids::osc_spread, "SPREAD", Colours::kMixer);

    makeKnob(Ids::mixer_seethe, "WARMTH", Colours::kMixer);



    // Master Chaos Deck

    makeKnob(Ids::chaos_rate, "RATE", Colours::kChaos);

    makeKnob(Ids::chaos_drift, "DRIFT", Colours::kChaos);

    makeKnob(Ids::chaos_poly_spread, "POLY SPRD", Colours::kChaos);

    makeKnob(Ids::rand_spread, "CHAOS SPRD", Colours::kChaos);



    // 18 Attenuverters (6 Timbral controls x 3 Oscillators)

    makeKnob(Ids::rand_o1_morph, "O1 MRP", Colours::kChaos);

    makeKnob(Ids::rand_o1_odd_even, "O1 O/E", Colours::kChaos);

    makeKnob(Ids::rand_o1_tilt, "O1 TLT", Colours::kChaos);

    makeKnob(Ids::rand_o1_formant, "O1 FRM", Colours::kChaos);

    makeKnob(Ids::rand_o1_mask, "O1 MSK", Colours::kChaos);

    makeKnob(Ids::rand_o1_speed, "O1 SPD", Colours::kChaos);



    makeKnob(Ids::rand_o2_morph, "O2 MRP", Colours::kChaos);

    makeKnob(Ids::rand_o2_odd_even, "O2 O/E", Colours::kChaos);

    makeKnob(Ids::rand_o2_tilt, "O2 TLT", Colours::kChaos);

    makeKnob(Ids::rand_o2_formant, "O2 FRM", Colours::kChaos);

    makeKnob(Ids::rand_o2_mask, "O2 MSK", Colours::kChaos);

    makeKnob(Ids::rand_o2_speed, "O2 SPD", Colours::kChaos);



    makeKnob(Ids::rand_o3_morph, "O3 MRP", Colours::kChaos);

    makeKnob(Ids::rand_o3_odd_even, "O3 O/E", Colours::kChaos);

    makeKnob(Ids::rand_o3_tilt, "O3 TLT", Colours::kChaos);

    makeKnob(Ids::rand_o3_formant, "O3 FRM", Colours::kChaos);

    makeKnob(Ids::rand_o3_mask, "O3 MSK", Colours::kChaos);

    makeKnob(Ids::rand_o3_speed, "O3 SPD", Colours::kChaos);



    // Master & Space

    makeKnob(Ids::bass_boost, "40Hz BASS", Colours::kMaster);

    makeKnob(Ids::treble_tame, "TREBLE", Colours::kMaster);

    makeKnob(Ids::alpha_chorus, "CHORUS", Colours::kMaster);

    makeKnob(Ids::reverb_time, "TIME", Colours::kMaster);

    makeKnob(Ids::reverb_damp, "DAMP", Colours::kMaster);

    makeKnob(Ids::reverb_mix, "REVERB", Colours::kMaster);

    makeKnob(Ids::osc_slop, "SLOP", Colours::kMaster);

    makeKnob(Ids::global_width, "WIDTH", Colours::kMaster);

    makeKnob(Ids::output_level, "VOLUME", Colours::kMaster);



    contentContainer.addMouseListener(this, true);



    layoutControls();



    setSize(expectedSize_.x, expectedSize_.y);

    applyDragSensitivity(effScale_);



    smoothedRandPips_.assign(knobs_.size(), 0.5f);

    startTimerHz(30);

}



NyarlatEditor::~NyarlatEditor() {

    stopTimer();

    setLookAndFeel(nullptr);

}



void NyarlatEditor::setupKnob(RotaryKnob& item, const juce::String& paramId, const juce::String& label, juce::Colour col) {

    item.paramId = paramId;

    item.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);

    item.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    item.slider.setColour(juce::Slider::rotarySliderFillColourId, col);

    item.slider.setColour(juce::Slider::thumbColourId, col);



    // In-Dial Custom Value Badge Formatters

    namespace Ids = nyarlatParameters::Ids;

    if (paramId == Ids::chaos_rate) {

        item.slider.textFromValueFunction = [](double v) {

            if (v <= 0.0) return juce::String("0 Hz");

            double periodSec = 1.0 / v;

            if (periodSec >= 60.0) {

                int mins = static_cast<int>(periodSec / 60.0);

                int secs = static_cast<int>(periodSec) % 60;

                return juce::String::formatted("%dm %02ds", mins, secs);

            } else if (periodSec >= 1.0) {

                return juce::String(periodSec, 1) + " s";

            }

            return juce::String(v, 1) + " Hz";

        };

    } else if (paramId == Ids::alpha_chorus) {

        item.slider.textFromValueFunction = [](double v) {

            if (v <= 0.001) return juce::String("OFF");

            float rateHz = 0.2f + static_cast<float>(v) * 5.8f;

            return juce::String(rateHz, 1) + " Hz";

        };

    } else if (paramId == Ids::output_level) {

        item.slider.textFromValueFunction = [](double v) {

            float gain = tkdsp::common::evalMasterVolumeGain(static_cast<float>(v));

            float db = (gain > 1e-4f) ? 20.0f * std::log10(gain) : -60.0f;

            return juce::String(db, 1) + " dB";

        };

    }



    item.label.setText(label, juce::dontSendNotification);

    item.label.setFont(juce::Font(juce::FontOptions(11.5f).withStyle("Bold")));

    item.label.setJustificationType(juce::Justification::centred);

    item.label.setColour(juce::Label::textColourId, col);



    contentContainer.addAndMakeVisible(item.slider);

    contentContainer.addAndMakeVisible(item.label);



    item.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(

        processor_.getAPVTS(), paramId, item.slider);



    for (const auto& ci : getNyarlatControlCatalog()) {

        if (paramId == ci.paramId) {

            controlMap_[&item.slider] = &ci;

            break;

        }

    }

}



void NyarlatEditor::setupFader(SliderFader& item, const juce::String& paramId, const juce::String& label, juce::Colour col) {

    item.paramId = paramId;

    item.slider.setSliderStyle(juce::Slider::LinearVertical);

    item.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    item.slider.setColour(juce::Slider::thumbColourId, col);

    item.slider.setColour(juce::Slider::trackColourId, juce::Colour(0xFF262D38));



    item.label.setText(label, juce::dontSendNotification);

    item.label.setFont(juce::Font(juce::FontOptions(11.5f).withStyle("Bold")));

    item.label.setJustificationType(juce::Justification::centred);

    item.label.setColour(juce::Label::textColourId, col);



    contentContainer.addAndMakeVisible(item.slider);

    contentContainer.addAndMakeVisible(item.label);



    item.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(

        processor_.getAPVTS(), paramId, item.slider);



    for (const auto& ci : getNyarlatControlCatalog()) {

        if (paramId == ci.paramId) {

            controlMap_[&item.slider] = &ci;

            break;

        }

    }

}



void NyarlatEditor::setupCombo(ComboBoxItem& item, const juce::String& paramId, const juce::String& label) {

    item.paramId = paramId;

    item.label.setText(label, juce::dontSendNotification);

    item.label.setFont(juce::Font(juce::FontOptions(11.5f).withStyle("Bold")));

    item.label.setJustificationType(juce::Justification::centredLeft);

    item.label.setColour(juce::Label::textColourId, juce::Colour(0xFF8B949E));



    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(processor_.getAPVTS().getParameter(paramId))) {

        item.combo.addItemList(choiceParam->choices, 1);

    }



    contentContainer.addAndMakeVisible(item.combo);

    contentContainer.addAndMakeVisible(item.label);



    item.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(

        processor_.getAPVTS(), paramId, item.combo);



    for (const auto& ci : getNyarlatControlCatalog()) {

        if (paramId == ci.paramId) {

            controlMap_[&item.combo] = &ci;

            break;

        }

    }

}



void NyarlatEditor::setupStepper(StepperItem& item, const juce::String& paramId, const juce::String& label, juce::Colour col) {

    item.paramId = paramId;

    item.bar.setAccentColour(col);

    item.label.setText(label, juce::dontSendNotification);

    item.label.setFont(juce::Font(juce::FontOptions(11.5f).withStyle("Bold")));

    item.label.setJustificationType(juce::Justification::centred);

    item.label.setColour(juce::Label::textColourId, col);



    contentContainer.addAndMakeVisible(item.bar);

    contentContainer.addAndMakeVisible(item.label);



    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(processor_.getAPVTS().getParameter(paramId))) {

        auto choices = choiceParam->choices;

        const int count = choices.size();

        auto updateText = [&item, choices, count](int idx) {

            idx = std::clamp(idx, 0, count - 1);

            item.bar.setItem(idx, choices[idx], count);

        };



        item.attachment = std::make_unique<juce::ParameterAttachment>(

            *choiceParam, [updateText](float v) {

                updateText(static_cast<int>(v));

            });



        item.bar.onSelectIndex = [this, paramId](int idx) {

            if (auto* p = processor_.getAPVTS().getParameter(paramId)) {

                p->setValueNotifyingHost(p->convertTo0to1(static_cast<float>(idx)));

            }

        };



        item.bar.onBuildMenu = [choices, count, &item](juce::PopupMenu& m) {

            for (int i = 0; i < count; ++i) {

                m.addItem(i + 1, choices[i], true, i == item.bar.getCurrentIndex());

            }

        };



        item.attachment->sendInitialUpdate();

    }



    for (const auto& ci : getNyarlatControlCatalog()) {

        if (paramId == ci.paramId) {

            controlMap_[&item.bar] = &ci;

            break;

        }

    }

}



void NyarlatEditor::setupButton(ButtonItem& item, const juce::String& paramId, const juce::String& label, juce::Colour col) {

    item.paramId = paramId;

    item.button.setButtonText(label);

    item.button.setClickingTogglesState(true);

    item.button.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF161B22));

    item.button.setColour(juce::TextButton::buttonOnColourId, col);

    item.button.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF8B949E));

    item.button.setColour(juce::TextButton::textColourOnId, juce::Colour(0xFF0D1117));



    contentContainer.addAndMakeVisible(item.button);



    item.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(

        processor_.getAPVTS(), paramId, item.button);



    for (const auto& ci : getNyarlatControlCatalog()) {

        if (paramId == ci.paramId) {

            controlMap_[&item.button] = &ci;

            break;

        }

    }

}



void NyarlatEditor::setupRadio(RadioItem& item, const juce::String& paramId, juce::Colour col, float fontSize) {

    item.paramId = paramId;

    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(processor_.getAPVTS().getParameter(paramId))) {

        item.radio.configure(choiceParam->choices, col, 2, fontSize);

        item.attachment = std::make_unique<juce::ParameterAttachment>(

            *choiceParam, [&item](float v) {

                item.radio.setIndex(static_cast<int>(v), juce::dontSendNotification);

            });

        item.radio.onChange = [this, paramId](int idx) {

            if (auto* p = processor_.getAPVTS().getParameter(paramId)) {

                p->setValueNotifyingHost(p->convertTo0to1(static_cast<float>(idx)));

            }

        };

        item.attachment->sendInitialUpdate();

    }



    contentContainer.addAndMakeVisible(item.radio);



    for (const auto& ci : getNyarlatControlCatalog()) {

        if (paramId == ci.paramId) {

            controlMap_[&item.radio] = &ci;

            break;

        }

    }

}



SegmentedRadio* NyarlatEditor::findRadio(const juce::String& paramId) {

    for (auto& r : radios_) if (r->paramId == paramId) return &r->radio;

    return nullptr;

}



NyarlatEditor::StepperItem* NyarlatEditor::findStepper(const juce::String& paramId) {

    for (auto& s : steppers_) if (s->paramId == paramId) return s.get();

    return nullptr;

}



void NyarlatEditor::paint(juce::Graphics& g) {

    g.fillAll(Colours::kChassisBg);

}



void NyarlatEditor::paintCanvas(juce::Graphics& g) {

    // Horizontal accent rule directly under header

    g.setColour(Colours::kOsc);

    g.fillRect(8, 38, kBaseWidth - 16, 2);



    auto drawBay = [&](int x, int y, int w, int h, const juce::String& title, juce::Colour badgeCol, int badgeWidth = 100) {

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



    // 3 Primary Master Bays (1160 x 870 4:3 Ratio)

    drawBay(14,  44, 360, 816, "ADDITIVE OSCILLATORS", Colours::kOsc, 160);

    drawBay(386, 44, 380, 816, "SILKY ANALOG & VOICING", Colours::kFilter, 170);

    drawBay(778, 44, 368, 816, "CHAOS & MASTER DECK", Colours::kChaos, 160);



    // Subtle Pods inside Bay 1 (3 Oscillators)

    drawSubtlePod(20, 68,  348, 260);

    drawSubtlePod(20, 336, 348, 260);

    drawSubtlePod(20, 604, 348, 260);



    // Subtle Pods inside Bay 2 (Filter & VCA)

    drawSubtlePod(392, 68,  368, 370, "SILKY ZERO-DELAY FEEDBACK FILTER");

    drawSubtlePod(392, 450, 368, 402, "VOICING & VCA ENVELOPE");



    // Subtle Pods inside Bay 3 (Mixer, Chaos Matrix, Master)

    drawSubtlePod(784, 68,  356, 186, "SUB-MIXER");

    drawSubtlePod(784, 264, 356, 320, "18-CH CHAOS TIMBRAL MATRIX");

    drawSubtlePod(784, 594, 356, 258, "MASTER & SPACE");

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

    // Header Strip Geometry

    titleLabel.setBounds          (14,  4, 140, 20);

    subtitleLabel.setBounds       (14, 24, 140, 12);

    stepperAuditionBar_.setBounds  (162, 8, 168, 28);

    headerInspector_.setBounds    (338, 8, 566, 28);

    lifeModeToggle_.setBounds     (912, 8, 234, 28);



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



    auto placeKnob = [&](const juce::String& id, int x, int y, int size, int labelW = 0, int labelH = 13) {

        auto* k = findKnob(id);

        if (k == nullptr) return;

        if (labelW <= 0) labelW = size + 6;

        k->label.setBounds(x - (labelW - size) / 2, y, labelW, labelH);

        k->slider.setBounds(x, y + labelH + 2, size, size);

    };



    auto placeFader = [&](const juce::String& id, int x, int y, int w, int h, int labelH = 13) {

        auto* f = findFader(id);

        if (f == nullptr) return;

        f->label.setBounds(x - 4, y, w + 8, labelH);

        f->slider.setBounds(x, y + labelH + 2, w, h);

    };



    auto placeCombo = [&](const juce::String& id, int x, int y, int w, int h, int labelH = 13, bool labelAbove = false) {

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



    auto placeStepper = [&](const juce::String& id, int x, int y, int w, int h, int labelH = 0, bool labelAbove = false) {

        auto* s = findStepper(id);

        if (s == nullptr) return;

        if (labelH > 0) {

            if (labelAbove) {

                s->label.setBounds(x, y, w, labelH);

                s->bar.setBounds(x, y + labelH + 2, w, h);

                return;

            } else {

                s->label.setBounds(x, y - labelH, w, labelH);

            }

        }

        s->bar.setBounds(x, y, w, h);

    };



    namespace Ids = nyarlatParameters::Ids;



    // ==============================================================================

    // Bay 1: Additive Oscillators (14, 44, 360, 816)

    // 3 Pods: 348 x 260px each (Base Y: 66, 332, 598)

    // ==============================================================================

    const std::array<int, 3> oscBaseY = { 66, 332, 598 };



    for (int o = 0; o < 3; ++o) {

        const int by = oscBaseY[o];

        const juce::String oPrefix = (o == 0) ? "osc1_" : (o == 1 ? "osc2_" : "osc3_");



        // Row 1: Bank Stepper Audition Bar

        placeStepper(oPrefix + "bank", 28, by + 2, 332, 20);



        // Row 2: Expanded Octave Radio (252px) + Sync/MSTR (284px)

        if (auto* r = findRadio(oPrefix + "octave")) {

            r->setBounds(28, by + 24, 252, 20);

        }

        if (o == 0) {

            osc1MasterButton_.setBounds(284, by + 24, 66, 20);

        } else if (auto* b = findButton(oPrefix + "sync")) {

            b->button.setBounds(284, by + 24, 66, 20);

        }



        // Row 3: Additive Waveform Preview flanked by 52px Detune Knob

        if (oscWaveViews_[o] != nullptr) {

            oscWaveViews_[o]->setBounds(28, by + 46, 236, 36);

        }

        placeKnob(oPrefix + "detune", 284, by + 44, 52, 58, 12);



        // Row 4: Speed Lock Button (28px, 46x20) + Loop Radio (78px, 186x20) -> right edge = 264px

        if (auto* b = findButton(oPrefix + "speed_lock")) {

            b->button.setBounds(28, by + 84, 46, 20);

        }

        if (auto* r = findRadio(oPrefix + "loop")) {

            r->setBounds(78, by + 84, 186, 20);

        }



        // Row 5a: Morph, Odd/Even, Tilt (3 knobs, 52px!)

        placeKnob(oPrefix + "morph",    38,  by + 116, 52, 56, 12);

        placeKnob(oPrefix + "odd_even", 150, by + 116, 52, 56, 12);

        placeKnob(oPrefix + "tilt",     262, by + 116, 52, 56, 12);



        // Row 5b: Formant, Mask, Speed (3 knobs, 52px!)

        placeKnob(oPrefix + "formant", 38,  by + 188, 52, 56, 12);

        placeKnob(oPrefix + "mask",    150, by + 188, 52, 56, 12);

        placeKnob(oPrefix + "speed",   262, by + 188, 52, 56, 12);

    }



    // ==============================================================================

    // Bay 2: Silky Analog Filter & Voicing (386, 44, 380, 816)

    // ==============================================================================

    // Pod 2A: Silky Analog Filter (392, 68, 368, 370)

    placeCombo(Ids::filter_slope, 404, 78, 150, 24, 12, false);



    // Hero Knobs: Cutoff & Resonance (72px!), Drive (56px), Hammer Snap (52px)

    placeKnob(Ids::filter_cutoff,     412, 114, 72, 76, 13);

    placeKnob(Ids::filter_resonance,  498, 114, 72, 76, 13);

    placeKnob(Ids::filter_drive,      582, 122, 56, 60, 13);

    placeKnob(Ids::noise_hammer_snap, 654, 124, 52, 56, 12);



    // Filter Envelope (4 Knobs, 54px!)

    placeKnob(Ids::filter_attack,  404, 216, 54, 58, 12);

    placeKnob(Ids::filter_decay,   486, 216, 54, 58, 12);

    placeKnob(Ids::filter_sustain, 568, 216, 54, 58, 12);

    placeKnob(Ids::filter_release, 650, 216, 54, 58, 12);



    // Filter Modulation (3 Knobs, 56px!)

    placeKnob(Ids::filter_env_depth, 422, 302, 56, 60, 12);

    placeKnob(Ids::filter_velocity,  516, 302, 56, 60, 12);

    placeKnob(Ids::filter_keytrack,  610, 302, 56, 60, 12);



    // Pod 2B: Voicing & VCA Envelope (392, 450, 368, 402)

    placeCombo(Ids::voice_mode, 404, 460, 160, 24, 12, false);



    if (vcaAdsrView_ != nullptr) {

        vcaAdsrView_->setBounds(404, 490, 344, 58);

    }



    // Amp Envelope (4 Knobs, 54px!)

    placeKnob(Ids::amp_attack,  404, 560, 54, 58, 12);

    placeKnob(Ids::amp_decay,   486, 560, 54, 58, 12);

    placeKnob(Ids::amp_sustain, 568, 560, 54, 58, 12);

    placeKnob(Ids::amp_release, 650, 560, 54, 58, 12);



    // Dynamics & Glide (3 Knobs, 56px!)

    placeKnob(Ids::amp_velocity, 422, 642, 56, 60, 12);

    placeKnob(Ids::keytrack_rate, 516, 642, 56, 60, 12);

    placeKnob(Ids::glide_time,    610, 642, 56, 60, 12);



    // Expression & Bend (3 Knobs, 56px!)

    placeKnob(Ids::mpe_slide_depth,    422, 726, 56, 60, 12);

    placeKnob(Ids::mpe_pressure_depth, 516, 726, 56, 60, 12);

    placeKnob(Ids::pitch_bend_range,   610, 726, 56, 60, 12);



    // ==============================================================================

    // Bay 3: Mixer, 18-CH Chaos & Master Deck (778, 44, 368, 816)

    // ==============================================================================

    // Pod 3A: Sub-Mixer (784, 68, 356, 186)

    const std::array<int, 4> faderX = { 794, 830, 866, 902 };

    placeFader(Ids::osc1_level,  faderX[0], 78, 26, 112);

    placeFader(Ids::osc2_level,  faderX[1], 78, 26, 112);

    placeFader(Ids::osc3_level,  faderX[2], 78, 26, 112);

    placeFader(Ids::noise_level, faderX[3], 78, 26, 112);



    placeKnob(Ids::noise_tone,   948, 78, 52, 56, 12);

    placeKnob(Ids::osc_spread,  1018, 78, 52, 56, 12);

    placeKnob(Ids::mixer_seethe, 980, 142, 64, 68, 13);



    // Pod 3B: Master Chaos & 18-CH Matrix (784, 264, 356, 320)

    // 4 Master Knobs across top row (Rate, Drift, Poly, Spread Chaos)

    placeKnob(Ids::chaos_rate,        792,  274, 56, 60, 13);

    placeKnob(Ids::chaos_drift,       872,  276, 52, 56, 12);

    placeKnob(Ids::chaos_poly_spread, 952,  276, 52, 56, 12);

    placeKnob(Ids::rand_spread,       1032, 276, 52, 56, 12);



    // 18 Chaos Depth Knobs (6 columns x 3 rows, 52px each, 57.3px pitch)

    // Exactly 5.3px physical airgap between columns -> Omega = 0px collision-free!

    const std::array<int, 6> cX = { 792, 849, 906, 963, 1020, 1077 };



    // Row 0: Osc 1

    placeKnob(Ids::rand_o1_morph,    cX[0], 352, 52, 52, 12);

    placeKnob(Ids::rand_o1_odd_even, cX[1], 352, 52, 52, 12);

    placeKnob(Ids::rand_o1_tilt,     cX[2], 352, 52, 52, 12);

    placeKnob(Ids::rand_o1_formant,  cX[3], 352, 52, 52, 12);

    placeKnob(Ids::rand_o1_mask,     cX[4], 352, 52, 52, 12);

    placeKnob(Ids::rand_o1_speed,    cX[5], 352, 52, 52, 12);



    // Row 1: Osc 2

    placeKnob(Ids::rand_o2_morph,    cX[0], 428, 52, 52, 12);

    placeKnob(Ids::rand_o2_odd_even, cX[1], 428, 52, 52, 12);

    placeKnob(Ids::rand_o2_tilt,     cX[2], 428, 52, 52, 12);

    placeKnob(Ids::rand_o2_formant,  cX[3], 428, 52, 52, 12);

    placeKnob(Ids::rand_o2_mask,     cX[4], 428, 52, 52, 12);

    placeKnob(Ids::rand_o2_speed,    cX[5], 428, 52, 52, 12);



    // Row 2: Osc 3

    placeKnob(Ids::rand_o3_morph,    cX[0], 504, 52, 52, 12);

    placeKnob(Ids::rand_o3_odd_even, cX[1], 504, 52, 52, 12);

    placeKnob(Ids::rand_o3_tilt,     cX[2], 504, 52, 52, 12);

    placeKnob(Ids::rand_o3_formant,  cX[3], 504, 52, 52, 12);

    placeKnob(Ids::rand_o3_mask,     cX[4], 504, 52, 52, 12);

    placeKnob(Ids::rand_o3_speed,    cX[5], 504, 52, 52, 12);



    // Pod 3C: Master & Space Deck (784, 594, 356, 258)

    placeKnob(Ids::bass_boost,   796, 604, 52, 56, 12);

    placeKnob(Ids::treble_tame,  880, 604, 52, 56, 12);

    placeKnob(Ids::alpha_chorus, 964, 604, 52, 56, 12);



    placeKnob(Ids::reverb_time,  796, 676, 52, 56, 12);

    placeKnob(Ids::reverb_damp,  880, 676, 52, 56, 12);

    placeKnob(Ids::reverb_mix,   964, 676, 52, 56, 12);



    placeKnob(Ids::osc_slop,     796, 746, 52, 56, 12);

    placeKnob(Ids::global_width, 880, 746, 52, 56, 12);

    placeKnob(Ids::output_level, 964, 738, 72, 76, 13);



#if JUCE_DEBUG

    // Regression Safety Guards

    for (auto& k : knobs_) {

        jassert(k && !k->slider.getBounds().isEmpty());

    }

    for (auto& f : faders_) {

        jassert(f && !f->slider.getBounds().isEmpty());

    }

    for (auto& c : combos_) {

        jassert(c && !c->combo.getBounds().isEmpty());

    }

    for (auto& s : steppers_) {

        jassert(s && !s->bar.getBounds().isEmpty());

    }

    for (auto& b : buttons_) {

        jassert(b && !b->button.getBounds().isEmpty());

    }

    for (int o = 0; o < 3; ++o) {

        if (oscWaveViews_[o] != nullptr) {

            jassert(!oscWaveViews_[o]->getBounds().isEmpty());

        }

    }

    if (vcaAdsrView_ != nullptr) {

        jassert(!vcaAdsrView_->getBounds().isEmpty());

    }

#endif

}



void NyarlatEditor::timerCallback() {

    auto tele = processor_.getVoiceTelemetry();

    int nv = processor_.getNewestVoice();

    bool hasLiveVoice = (nv >= 0 && nv < NyarlatProcessor::kNumVoices && tele[nv].active);



    // 1. Repaint Waveform Visualizers with live aperture & sweep

    for (int o = 0; o < 3; ++o) {

        if (oscWaveViews_[o] != nullptr) {

            if (hasLiveVoice) {

                oscWaveViews_[o]->setLive(true, tele[nv].oscEffWave[o], tele[nv].oscEffMask[o], tele[nv].oscEffSpeed[o]);

            } else {

                oscWaveViews_[o]->setLive(false, 0.0f, 511, 0.0f);

            }

            oscWaveViews_[o]->repaint();

        }

    }



    // 2. Repaint Vector ADSR Preview

    if (vcaAdsrView_ != nullptr) {

        vcaAdsrView_->repaint();

    }



    // 3. Dynamic Cracked Chimes Arc Halos

    namespace Ids = nyarlatParameters::Ids;

    auto& apvts = processor_.getAPVTS();



    auto findKnob = [this](const juce::String& id) -> RotaryKnob* {

        for (auto& k : knobs_) if (k->paramId == id) return k.get();

        return nullptr;

    };



    // 3A. Filter Cutoff Envelope Arc Halo

    if (auto* kCutoff = findKnob(Ids::filter_cutoff)) {

        float fEnvDepth = *apvts.getRawParameterValue(Ids::filter_env_depth);

        float kNorm = static_cast<float>(kCutoff->slider.valueToProportionOfLength(kCutoff->slider.getValue()));

        auto& sp = kCutoff->slider.getProperties();

        if (std::abs(fEnvDepth) > 0.002f) {

            float liveEnv = hasLiveVoice ? tele[nv].filterEnv : 0.0f;

            float tgt = (fEnvDepth >= 0.0f)

                ? std::clamp(kNorm + fEnvDepth * (1.0f - kNorm), 0.0f, 1.0f)

                : std::clamp(kNorm + fEnvDepth * kNorm, 0.0f, 1.0f);

            float pip = (fEnvDepth >= 0.0f)

                ? std::clamp(kNorm + fEnvDepth * liveEnv * (1.0f - kNorm), 0.0f, 1.0f)

                : std::clamp(kNorm + fEnvDepth * liveEnv * kNorm, 0.0f, 1.0f);



            float lo = std::min(kNorm, tgt);

            float hi = std::max(kNorm, tgt);

            float oldPip = sp.getWithDefault("modPip", -1.0f);

            if (std::abs(oldPip - pip) > 0.002f) {

                sp.set("modLo", lo);

                sp.set("modHi", hi);

                sp.set("modPip", pip);

                sp.set("modColour", static_cast<juce::int64>(Colours::kFilter.getARGB()));

                kCutoff->slider.repaint();

            }

        } else if (sp.contains("modLo")) {

            sp.remove("modLo");

            sp.remove("modHi");

            sp.remove("modPip");

            kCutoff->slider.repaint();

        }

    }



    // 3B. Stereo Spread Chaos Arc Halo

    if (auto* kSpread = findKnob(Ids::osc_spread)) {

        float randSpread = *apvts.getRawParameterValue(Ids::rand_spread);

        auto& sp = kSpread->slider.getProperties();

        if (randSpread > 0.002f) {

            float kNorm = static_cast<float>(kSpread->slider.valueToProportionOfLength(kSpread->slider.getValue()));

            float rVal = hasLiveVoice ? tele[nv].rawChaos[18] : 0.5f;

            auto halo = CrackedChimes::DSP::computeStepRandHalo(kNorm, randSpread, rVal);

            float oldPip = sp.getWithDefault("randPip", -1.0f);

            if (std::abs(oldPip - halo.pip) > 0.002f) {

                sp.set("randLo", halo.lo);

                sp.set("randHi", halo.hi);

                sp.set("randPip", halo.pip);

                sp.set("randColour", static_cast<juce::int64>(Colours::kChaos.getARGB()));

                kSpread->slider.repaint();

            }

        } else if (sp.contains("randLo")) {

            sp.remove("randLo");

            sp.remove("randHi");

            sp.remove("randPip");

            kSpread->slider.repaint();

        }

    }



    // 3C. 18 Additive Oscillator Destination-Anchored Chaos Halos

    struct OscModTarget {

        const char* baseId;

        const char* randId;

        int chaosIdx;

    };

    static const OscModTarget kTargets[18] = {

        // Osc 1

        { Ids::osc1_morph,    Ids::rand_o1_morph,    0 },

        { Ids::osc1_odd_even, Ids::rand_o1_odd_even, 3 },

        { Ids::osc1_tilt,     Ids::rand_o1_tilt,     6 },

        { Ids::osc1_formant,  Ids::rand_o1_formant,  9 },

        { Ids::osc1_mask,     Ids::rand_o1_mask,     12 },

        { Ids::osc1_speed,    Ids::rand_o1_speed,    15 },

        // Osc 2

        { Ids::osc2_morph,    Ids::rand_o2_morph,    1 },

        { Ids::osc2_odd_even, Ids::rand_o2_odd_even, 4 },

        { Ids::osc2_tilt,     Ids::rand_o2_tilt,     7 },

        { Ids::osc2_formant,  Ids::rand_o2_formant,  10 },

        { Ids::osc2_mask,     Ids::rand_o2_mask,     13 },

        { Ids::osc2_speed,    Ids::rand_o2_speed,    16 },

        // Osc 3

        { Ids::osc3_morph,    Ids::rand_o3_morph,    2 },

        { Ids::osc3_odd_even, Ids::rand_o3_odd_even, 5 },

        { Ids::osc3_tilt,     Ids::rand_o3_tilt,     8 },

        { Ids::osc3_formant,  Ids::rand_o3_formant,  11 },

        { Ids::osc3_mask,     Ids::rand_o3_mask,     14 },

        { Ids::osc3_speed,    Ids::rand_o3_speed,    17 }

    };



    for (const auto& t : kTargets) {

        auto* k = findKnob(t.baseId);

        if (k == nullptr) continue;

        float amt = *apvts.getRawParameterValue(t.randId);

        auto& sp = k->slider.getProperties();



        if (amt > 0.002f) {

            float kNorm = static_cast<float>(k->slider.valueToProportionOfLength(k->slider.getValue()));

            float rVal = hasLiveVoice ? tele[nv].rawChaos[t.chaosIdx] : 0.5f;

            auto halo = CrackedChimes::DSP::computeStepRandHalo(kNorm, amt, rVal);



            float oldPip = sp.getWithDefault("randPip", -1.0f);

            if (std::abs(oldPip - halo.pip) > 0.002f) {

                sp.set("randLo", halo.lo);

                sp.set("randHi", halo.hi);

                sp.set("randPip", halo.pip);

                sp.set("randColour", static_cast<juce::int64>(Colours::kChaos.getARGB()));

                k->slider.repaint();

            }

        } else if (sp.contains("randLo")) {

            sp.remove("randLo");

            sp.remove("randHi");

            sp.remove("randPip");

            k->slider.repaint();

        }

    }

}



void NyarlatEditor::inspectComponent(juce::Component* comp) {

    if (comp == nullptr) {

        headerInspector_.setReadoutDefault();

        return;

    }



    auto it = controlMap_.find(comp);

    if (it != controlMap_.end() && it->second != nullptr) {

        const auto* info = it->second;

        HeaderInspector::Readout r;

        r.name = info->name;

        r.blurb = info->blurb;

        r.accent = info->accent;

        r.isDefault = false;



        if (auto* s = dynamic_cast<juce::Slider*>(comp)) {

            r.value = s->getTextFromValue(s->getValue());

        } else if (auto* c = dynamic_cast<juce::ComboBox*>(comp)) {

            r.value = c->getText();

        } else if (auto* b = dynamic_cast<juce::TextButton*>(comp)) {

            r.value = b->getToggleState() ? "ON" : "OFF";

        } else if (auto* rad = dynamic_cast<SegmentedRadio*>(comp)) {

            r.value = rad->getValueText();

        } else if (auto* step = dynamic_cast<StepperAuditionBar*>(comp)) {

            r.value = juce::String(step->getCurrentIndex());

        }

        headerInspector_.setReadout(r);

        return;

    }



    if (comp->getParentComponent() != nullptr && comp->getParentComponent() != &contentContainer) {

        inspectComponent(comp->getParentComponent());

        return;

    }



    headerInspector_.setReadoutDefault();

}



void NyarlatEditor::mouseEnter(const juce::MouseEvent& e) {

    inspectComponent(e.eventComponent);

}



void NyarlatEditor::mouseExit(const juce::MouseEvent& e) {

    juce::ignoreUnused(e);

    headerInspector_.setReadoutDefault();

}



void NyarlatEditor::mouseDrag(const juce::MouseEvent& e) {

    inspectComponent(e.eventComponent);

}



} // namespace Nyarlat

