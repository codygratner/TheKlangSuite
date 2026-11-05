#include "DigiStrikeFS2Editor.h"
#include "Digistrike_fs2ParameterIds.h"

using namespace digistrike_fs2Parameters::Ids;

DigiStrikeFS2AudioProcessorEditor::DigiStrikeFS2AudioProcessorEditor(DigiStrikeFS2AudioProcessor& p)
    : AudioProcessorEditor(&p), processor_(p)
{

    setSize(kBaseWidth, kBaseHeight);
    setLookAndFeel(&lnf_);
    addMouseListener(this, true);
    startTimerHz(30);

    auto& apvts = processor_.getAPVTS();

    // =========================================================================
    // 1. Header Row
    // =========================================================================
    presetStub_.setAccentColour(juce::Colour(0xFFFFAA00));
    addAndMakeVisible(presetStub_);

    addAndMakeVisible(headerInspector_);

    lifeformRocker_.onSelect = [&apvts](int idx) {
        if (auto* param = apvts.getParameter(life_mode))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
    };
    if (auto* param = apvts.getRawParameterValue(life_mode))
        lifeformRocker_.setState(static_cast<int>(param->load()));
    addAndMakeVisible(lifeformRocker_);
    registerInspector(&lifeformRocker_, life_mode, juce::Colour(0xFF6BBF59));

    // =========================================================================
    // 2. Card 1 Knobs: Master Macro & Dynamics (Top-Left, 8)
    // 6-Domain Chromatic Standard: Movie Poster Orange (0xFFFF7B25)
    // =========================================================================
    const auto orangeCol = juce::Colour(0xFFFF7B25);
    setupKnob(macroXKnob_,     macro_x,     "X: SPECTRAL", "%", "macro spectral spread, sweep excursion and harmonics", orangeCol, true);
    setupKnob(macroYKnob_,     macro_y,     "Y: PUNCH",    "%", "macro transient attack spike and punch snap",         orangeCol, true);
    setupKnob(macroZKnob_,     macro_z,     "Z: TIMBRE",   "%", "macro harmonic waveshaping, fold & overtone",         orangeCol, true);
    setupKnob(macroTKnob_,     macro_t,     "T: TIME",     "%", "macro pitch envelope and body decay time",            orangeCol, true);
    setupKnob(velAggrKnob_,    vel_aggr,    "VEL->AGGR",   "%", "velocity to synthesis aggression & overdrive",       orangeCol, true);
    setupKnob(velTimbKnob_,    vel_timb,    "VEL->TIMB",   "%", "velocity to synthesis brightness & overtones",       orangeCol, true);
    setupKnob(panKnob_,        pan,         "PAN",         "%", "stereo panorama positioning",                        orangeCol, true);
    setupKnob(decayCurveKnob_, decay_curve, "DECAY CRV",   "",  "envelope discharge curve (exp to log)",              orangeCol, true);

    // =========================================================================
    // 3. Card 2: Uniform Sound Lab (Top-Right)
    // 6-Domain Chromatic Standard: Movie Poster Blue (0xFF38BDF8)
    // =========================================================================
    const auto blueCol = juce::Colour(0xFF38BDF8);
    // Upper 8 Knobs
    setupKnob(pitchDecayKnob_, pitch_decay,   "P.DEC",   "ms", "pitch envelope decay duration (2ms..250ms)", blueCol);
    setupKnob(clickKnob_,      click_bleed,   "CLICK",   "%",  "transient crack & noise bleed level",        blueCol);
    setupKnob(startKnob_,      start_offset,  "START",   "ms", "initial sample playback offset (0..500ms)",   blueCol);
    setupKnob(timeKnob_,       traverse_rate, "TIME",    "x",  "duration scaling & reverse traversal (-4x..+4x, +/-1x=normal, 0x=freeze)", blueCol, true);
    timeKnob_.slider.setDoubleClickReturnValue(true, 1.0);
    setupKnob(punchKnob_,      pitch_dive,    "PUNCH",   "%",  "pitch dive depth (0%..100% => 0..36 semitones)", blueCol);
    setupKnob(noiseKnob_,      noise_mix,     "NOISE",   "%",  "enveloped 1-pole highpass noise mix (35% amp + 65% pitch attack)", blueCol);
    setupKnob(lengthKnob_,     sample_length, "LENGTH",  "%",  "sample playback window / scan span",         blueCol);
    setupKnob(morphKnob_,      table_morph,   "MORPH",   "%",  "3D wavetable slice tilt and morph",          blueCol);

    // Loop Mode 2x2 Switch (ONCE, HOLD, LOOP, PING)
    loopModeBar_.setAccentColour(blueCol);
    loopModeBar_.onSelect = [this, &apvts](int idx) {
        int choice = (idx == -1) ? 0 : idx;
        loopModeBar_.setState(choice);
        if (auto* param = apvts.getParameter(loop_mode)) {
            param->beginChangeGesture();
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(choice)));
            param->endChangeGesture();
        }
    };
    if (auto* param = apvts.getRawParameterValue(loop_mode)) {
        int val = static_cast<int>(std::lround(param->load()));
        loopModeBar_.setState(std::clamp(val, 0, 3));
    }
    addAndMakeVisible(loopModeBar_);
    registerInspector(&loopModeBar_, loop_mode, blueCol);

    // REV Latching Button
    revAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, reverse, revButton_);
    addAndMakeVisible(revButton_);
    registerInspector(&revButton_, reverse, blueCol);

    // Voice Selector Capsule Setup
    voiceSelectorCapsule_.onStepPrev = [this, &apvts]() {
        const auto& rom = tkdsp::DigiStrikeFS2ROM::getInstance();
        int cat = categoryGrid_.getState();
        uint32_t mask = lineageGrid_.getMask();
        int N = DigiStrikeFS2Fleet::getActiveFleetCount(rom, cat, mask);
        if (N <= 1) return;

        if (auto* p = apvts.getRawParameterValue(table_morph)) {
            float norm = std::clamp(p->load() * 0.01f, 0.0f, 1.0f);
            int curG = DigiStrikeFS2Fleet::indexForMorph(norm, N);
            int newG = (curG > 0) ? (curG - 1) : (N - 1);
            float newMorph = DigiStrikeFS2Fleet::morphForIndex(newG, N) * 100.0f;
            if (auto* param = apvts.getParameter(table_morph)) {
                param->beginChangeGesture();
                param->setValueNotifyingHost(param->convertTo0to1(newMorph));
                param->endChangeGesture();
            }
        }
    };

    voiceSelectorCapsule_.onStepNext = [this, &apvts]() {
        const auto& rom = tkdsp::DigiStrikeFS2ROM::getInstance();
        int cat = categoryGrid_.getState();
        uint32_t mask = lineageGrid_.getMask();
        int N = DigiStrikeFS2Fleet::getActiveFleetCount(rom, cat, mask);
        if (N <= 1) return;

        if (auto* p = apvts.getRawParameterValue(table_morph)) {
            float norm = std::clamp(p->load() * 0.01f, 0.0f, 1.0f);
            int curG = DigiStrikeFS2Fleet::indexForMorph(norm, N);
            int newG = (curG + 1 < N) ? (curG + 1) : 0;
            float newMorph = DigiStrikeFS2Fleet::morphForIndex(newG, N) * 100.0f;
            if (auto* param = apvts.getParameter(table_morph)) {
                param->beginChangeGesture();
                param->setValueNotifyingHost(param->convertTo0to1(newMorph));
                param->endChangeGesture();
            }
        }
    };

    voiceSelectorCapsule_.onClickCenter = [this, &apvts]() {
        const auto& rom = tkdsp::DigiStrikeFS2ROM::getInstance();
        int cat = categoryGrid_.getState();
        uint32_t mask = lineageGrid_.getMask();
        int N = DigiStrikeFS2Fleet::getActiveFleetCount(rom, cat, mask);
        if (N <= 0) return;

        float norm = 0.0f;
        if (auto* p = apvts.getRawParameterValue(table_morph)) norm = std::clamp(p->load() * 0.01f, 0.0f, 1.0f);
        int curG = DigiStrikeFS2Fleet::indexForMorph(norm, N);

        int activeLinCount = 0;
        for (int i = 0; i < 20; ++i) if ((mask & (1u << i)) != 0) ++activeLinCount;

        auto* menu = new juce::PopupMenu();
        if (activeLinCount <= 1) {
            // Flat list for single active lineage
            for (int g = 0; g < N; ++g) {
                DigiStrikeFS2Fleet::VoiceRef ref;
                if (DigiStrikeFS2Fleet::resolveFleetIndex(rom, cat, mask, g, ref)) {
                    const char* vName = rom.getVoiceName(ref.romIndex);
                    juce::String numStr = juce::String(ref.localIndex + 1);
                    if (numStr.length() < 2) numStr = " " + numStr;
                    menu->addItem(g + 1, numStr + ": " + juce::String::fromUTF8(vName), true, (g == curG));
                }
            }
        } else {
            // Hierarchical submenus per active lineage
            int gAccum = 0;
            for (int lin = 0; lin < 20; ++lin) {
                if ((mask & (1u << lin)) != 0) {
                    auto pr = rom.getPoolRange(cat * 20 + lin);
                    const int count = static_cast<int>(pr.voiceCount);
                    if (count > 0) {
                        juce::PopupMenu sub;
                        for (int i = 0; i < count; ++i) {
                            const int g = gAccum + i;
                            const char* vName = rom.getVoiceName(pr.firstVoiceIndex + static_cast<uint32_t>(i));
                            juce::String numStr = juce::String(i + 1);
                            if (numStr.length() < 2) numStr = " " + numStr;
                            sub.addItem(g + 1, numStr + ": " + juce::String::fromUTF8(vName), true, (g == curG));
                        }
                        const juce::String linName = juce::String(DigiStrikeFS2Fleet::kLineageNames[static_cast<size_t>(lin)]);
                        const bool hasActive = (curG >= gAccum && curG < gAccum + count);
                        menu->addSubMenu(linName + " (" + juce::String(count) + ")", sub, true, nullptr, hasActive);
                        gAccum += count;
                    }
                }
            }
        }

        menu->showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&voiceSelectorCapsule_).withMaximumNumColumns(1),
            [this, N, &apvts, menu](int result) {
                if (result > 0 && N > 0) {
                    int pickedG = result - 1;
                    float newMorph = DigiStrikeFS2Fleet::morphForIndex(pickedG, N) * 100.0f;
                    if (auto* param = apvts.getParameter(table_morph)) {
                        param->beginChangeGesture();
                        param->setValueNotifyingHost(param->convertTo0to1(newMorph));
                        param->endChangeGesture();
                    }
                }
                delete menu;
            });
    };
    addAndMakeVisible(voiceSelectorCapsule_);
    registerInspector(&voiceSelectorCapsule_, table_morph, blueCol);

    loopLabel_.setText("LOOP", juce::dontSendNotification);
    loopLabel_.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
    loopLabel_.setColour(juce::Label::textColourId, juce::Colour(0xFF8B949E));
    loopLabel_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(loopLabel_);

    // Lower Tactile Button Matrix: Category & Lineage
    categoryGrid_.setAccentColour(blueCol);
    categoryGrid_.onSelect = [this, &apvts](int idx) {
        if (auto* param = apvts.getParameter(category))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
    };
    if (auto* param = apvts.getRawParameterValue(category))
        categoryGrid_.setState(static_cast<int>(param->load()));
    addAndMakeVisible(categoryGrid_);
    registerInspector(&categoryGrid_, category, blueCol);

    lineageGrid_.setAccentColour(blueCol);
    lineageGrid_.onMaskChange = [this, &apvts](uint32_t newMask) {
        for (size_t i = 0; i < 20; ++i) {
            if (auto* param = apvts.getParameter(DigiStrikeFS2Info::kLineageParamIds[i]))
                param->setValueNotifyingHost(((newMask & (1u << i)) != 0) ? 1.0f : 0.0f);
        }

        if (auto* param = apvts.getParameter(lineage)) {
            int firstLin = 14; // Default Fairlight
            for (int i = 0; i < 20; ++i) {
                if ((newMask & (1u << i)) != 0) { firstLin = i; break; }
            }
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(firstLin)));
        }
        updateInspectorForComponent(&lineageGrid_);
        repaint();
    };
    lineageGrid_.onHover = [this](int hoveredIdx) {
        juce::ignoreUnused(hoveredIdx);
        updateInspectorForComponent(&lineageGrid_);
        repaint();
    };
    lineageGrid_.onBudgetLimit = [this](const juce::String& msg) {
        DigiStrikeHeaderInspector::Readout r;
        r.name = "BUDGET";
        r.value = "64/64 MAX";
        r.blurb = msg;
        r.accent = juce::Colour(0xFFFF5555);
        headerInspector_.setReadout(r);
    };

    uint32_t initMask = 0;
    for (size_t i = 0; i < 20; ++i) {
        if (auto* paramVal = apvts.getRawParameterValue(DigiStrikeFS2Info::kLineageParamIds[i])) {
            if (paramVal->load() > 0.5f) initMask |= (1u << i);
        } else {
            initMask |= (1u << i); // Default all lineages enabled
        }
    }
    lineageGrid_.setMask(initMask != 0 ? initMask : 0xFFFFFu);

    addAndMakeVisible(lineageGrid_);
    registerInspector(&lineageGrid_, lineage, blueCol);

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
        processor_.triggerManual();
    };
    trigButton_.onRelease = [this]() {
        processor_.triggerVoiceRelease();
    };
    trigButton_.onRightTrigger = [this]() {
        processor_.triggerManual(60, 100.0f / 127.0f);
    };
    trigButton_.onRightRelease = [this]() {
        processor_.triggerVoiceRelease(60);
    };
    addAndMakeVisible(trigButton_);
    registerInspector(&trigButton_, "", juce::Colour(0xFFFFAA00), nullptr,
                      "MANUAL STRIKE PAD", "Left-Click: replay last incoming MIDI note/vel. Right-Click: reference C3.");

    // =========================================================================
    // 6. Card 3 Pod 3: VCF, Mojo & Output
    // =========================================================================
    // Row 0: CEM3320 ZDF VCF (Acid Green 0xFF00E676)
    const auto acidGreenCol = juce::Colour(0xFF00E676);
    setupKnob(cutoffKnob_,       cutoff,          "CUTOFF", "Hz", "Curtis CEM3320 filter cutoff frequency", acidGreenCol);
    setupKnob(resonanceKnob_,    resonance,       "RES",    "%",  "filter resonance / self-oscillation",    acidGreenCol);
    setupKnob(filterEnvAmtKnob_, filter_env_amt,  "FE-AMT", "%",  "filter envelope sweep amount (-100%..+100%)", acidGreenCol, true);
    setupKnob(filterDecayKnob_,  filter_decay,    "FE-DEC", "ms", "filter envelope decay time (5ms..5.0s)",  acidGreenCol);

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
    addAndMakeVisible(filterTypeBar_);
    registerInspector(&filterTypeBar_, filter_type, acidGreenCol);

    // Row 1: Universal Hardware Mojo Strip (Sorcerous Purple 0xFFBC8CFF)
    const auto purpleCol = juce::Colour(0xFFBC8CFF);
    setupKnob(filterDriveKnob_,  pre_drive,        "DRIVE",  "%",  "pre-filter analog saturation and drive", purpleCol);
    setupKnob(crushKnob_,        crush,            "CRUSH",  "Hz", "mojo sample rate decimation frequency",  purpleCol);
    setupKnob(shiftKnob_,        shift,            "SHIFT",  "Hz", "bode frequency shifter (+/-1000Hz)",     purpleCol, true);
    setupKnob(squashKnob_,       squash,           "SQUASH", "%",  "bipolar dynamics: snappy vs huge",       purpleCol, true);
    setupKnob(widerKnob_,        wider,            "WIDER",  "%",  "bipolar widener: up=classic comb, down=hilbert phase", purpleCol, true);
    widerKnob_.slider.textFromValueFunction = [](double val) -> juce::String {
        if (std::abs(val) < 0.01) return "OFF";
        const int pct = static_cast<int>(std::round(std::abs(val) * 100.0));
        return (val > 0.0) ? ("+" + juce::String(pct) + "%") : ("-" + juce::String(pct) + "%");
    };

    // Master Output Bay: Sustain, Amp Decay, Glide Time, Limiter, Volume & Voice Mode (Blistering Red 0xFFFF3B30)
    const auto redCol = juce::Colour(0xFFFF3B30);
    setupKnob(sustainKnob_, sustain_level, "SUSTAIN", "%",  "gated hold level (0%=one-shot, >2%=synth)", redCol);
    limiterAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, master_limiter, limiterButton_);
    addAndMakeVisible(limiterButton_);
    registerInspector(&limiterButton_, master_limiter, redCol);

    setupKnob(volumeKnob_,   volume,    "VOLUME",  "dB", "master output level into safety limiter", redCol);
    volumeKnob_.slider.setDoubleClickReturnValue(true, 0.5);

    setupKnob(ampDecayKnob_, amp_decay, "AMP-DEC", "s",  "master dynamic amp decay & gated release", redCol);
    ampDecayKnob_.slider.setDoubleClickReturnValue(true, 0.500);

    setupKnob(glideTimeKnob_, glide_time, "GLIDE", "s", "portamento glide duration for authentic legato synth basslines", redCol);
    glideTimeKnob_.slider.setDoubleClickReturnValue(true, 0.050);

    voiceModeRocker_.setColours({juce::Colour(0xFFFFAA00), juce::Colour(0xFF00C8FF), juce::Colour(0xFF6BBF59)});
    voiceModeRocker_.onSelect = [&apvts](int idx) {
        if (auto* param = apvts.getParameter(voice_mode))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
    };
    if (auto* param = apvts.getRawParameterValue(voice_mode))
        voiceModeRocker_.setState(static_cast<int>(param->load()));
    addAndMakeVisible(voiceModeRocker_);
    registerInspector(&voiceModeRocker_, voice_mode, juce::Colour(0xFF00C8FF));
}

void DigiStrikeFS2AudioProcessorEditor::updateFilterTypeUI(int filterIdx) {
    juce::ignoreUnused(filterIdx);
}

DigiStrikeFS2AudioProcessorEditor::~DigiStrikeFS2AudioProcessorEditor() {
    removeMouseListener(this);
    stopTimer();
    setLookAndFeel(nullptr);
}

void DigiStrikeFS2AudioProcessorEditor::registerInspector(juce::Component* comp, const juce::String& paramId,
                                                          juce::Colour accent, juce::Slider* s,
                                                          const juce::String& cName, const juce::String& cBlurb) {
    if (comp != nullptr) {
        inspectorBindings_[comp] = InspectorBinding{paramId, accent, s, cName, cBlurb};
    }
}

juce::Component* DigiStrikeFS2AudioProcessorEditor::resolveInspectorComponent(juce::Component* comp) {
    while (comp != nullptr && comp != this) {
        if (inspectorBindings_.find(comp) != inspectorBindings_.end())
            return comp;
        comp = comp->getParentComponent();
    }
    return nullptr;
}

void DigiStrikeFS2AudioProcessorEditor::updateInspectorForComponent(juce::Component* comp) {
    if (comp == nullptr) return;
    auto it = inspectorBindings_.find(comp);
    if (it == inspectorBindings_.end()) return;

    const auto& binding = it->second;
    const auto* info = DigiStrikeFS2Info::find(binding.paramId.toStdString());

    DigiStrikeHeaderInspector::Readout r;
    r.accent = binding.accent;
    r.isDefault = false;

    if (!binding.customName.isEmpty()) {
        r.name = binding.customName;
        r.blurb = binding.customBlurb;
        r.value = "TRIG";
    } else if (info != nullptr) {
        r.name = juce::String(info->name.data(), info->name.size());
        r.blurb = juce::String(info->blurb.data(), info->blurb.size());

        if (binding.slider != nullptr) {
            if (binding.slider->textFromValueFunction)
                r.value = binding.slider->textFromValueFunction(binding.slider->getValue());
            else
                r.value = juce::String(binding.slider->getValue(), 2);
        } else if (binding.paramId == "category") {
            const int cat = std::clamp(categoryGrid_.getState(), 0, 10);
            const juce::StringArray catNames{"BD", "SN", "TM", "CP", "HH", "CY", "LN", "MC", "FX", "ST", "BS"};
            r.value = catNames[cat];
            r.blurb = juce::String(DigiStrikeFS2Info::kCategoryDescriptions[cat].data(), DigiStrikeFS2Info::kCategoryDescriptions[cat].size());
        } else if (binding.paramId == "lineage") {
            const uint32_t mask = lineageGrid_.getMask();
            int hovered = lineageGrid_.getHoveredIndex();
            int lin = (hovered >= 0) ? hovered : 0;
            if (hovered < 0) {
                for (int i = 0; i < 20; ++i) {
                    if ((mask & (1u << i)) != 0) { lin = i; break; }
                }
            }
            const juce::StringArray linNames{
                "Crown", "Apex", "Empyrean", "Falcon", "K-Works", "Osaka", "Tokyo", "Gothenburg", "Circuits", "Shibuya",
                "Hamamatsu", "Keio", "Nippon", "Adios", "Sydney", "Dartmouth", "Malvern", "Hexcess", "Berlin", "Curios"
            };
            bool isActive = (mask & (1u << lin)) != 0;
            int activeCount = 0;
            for (int i = 0; i < 20; ++i) {
                if ((mask & (1u << i)) != 0) ++activeCount;
            }
            r.name = "LINEAGE: " + linNames[lin];
            r.value = isActive ? (juce::String(activeCount * 4) + "/80 WAVES (" + juce::String(activeCount) + " LIN)") : "MUTED";
            if (lin + 1 < static_cast<int>(std::size(DigiStrikeFS2Info::kLineageDescriptions))) {
                r.blurb = juce::String(DigiStrikeFS2Info::kLineageDescriptions[lin + 1].data(), DigiStrikeFS2Info::kLineageDescriptions[lin + 1].size())
                         + " (Click: toggle, R-Click/Alt: solo, 2x-Click: Fairlight reset)";
            } else {
                r.blurb = "PCM hardware lineage drum profile (Click: toggle, R-Click/Alt: solo)";
            }
        } else if (binding.paramId == "voice_mode") {
            const int mode = std::clamp(voiceModeRocker_.getState(), 0, 2);
            r.value = (mode == 0 ? "MONO" : (mode == 1 ? "GLIDE" : "POLY"));
            r.blurb = juce::String(DigiStrikeFS2Info::kVoiceModeDescriptions[mode].data(), DigiStrikeFS2Info::kVoiceModeDescriptions[mode].size());
        } else if (binding.paramId == "life_mode") {
            const int life = std::clamp(lifeformRocker_.getState(), 0, 2);
            r.value = (life == 0 ? "SYNTHETIC" : (life == 1 ? "CYBORG" : "ORGANIC"));
            r.blurb = juce::String(DigiStrikeFS2Info::kLifeformDescriptions[life].data(), DigiStrikeFS2Info::kLifeformDescriptions[life].size());
        } else if (binding.paramId == "loop_mode") {
            const int mode = std::clamp(loopModeBar_.getState(), 0, 3);
            const juce::StringArray loopNames{"ONCE (ONE-SHOT)", "HOLD (SUSTAIN FREEZE)", "LOOP (FORWARD LOOP)", "PING (PING-PONG)"};
            r.value = loopNames[mode];
            r.blurb = "wavetable frame traversal boundary behavior: one-shot release, sustain freeze, continuous loop, or ping-pong";
        } else if (binding.paramId == "reverse") {
            const bool on = revButton_.getToggleState();
            r.value = on ? "REVERSE (B -> A)" : "FORWARD (A -> B)";
            r.blurb = "playback traversal direction: forward from start offset or reverse sweeping toward transient crack";
        }
    }

    if (!r.name.isEmpty()) {
        headerInspector_.setReadout(r);
    }
}

void DigiStrikeFS2AudioProcessorEditor::mouseEnter(const juce::MouseEvent& e) {
    if (lockedComponent_ == nullptr) {
        if (auto* c = resolveInspectorComponent(e.eventComponent)) {
            updateInspectorForComponent(c);
        }
    }
}

void DigiStrikeFS2AudioProcessorEditor::mouseMove(const juce::MouseEvent& e) {
    if (lockedComponent_ == nullptr) {
        if (auto* c = resolveInspectorComponent(e.eventComponent)) {
            updateInspectorForComponent(c);
        }
    }
}

void DigiStrikeFS2AudioProcessorEditor::mouseDown(const juce::MouseEvent& e) {
    if (auto* s = dynamic_cast<juce::Slider*>(e.eventComponent)) {
        activeBadgeSlider_ = s;
        activeBadgeExpiry_ = juce::Time::getMillisecondCounter() + 350;
        s->getProperties().set("badgeExpiry", static_cast<double>(activeBadgeExpiry_));
        s->repaint();
    }
    if (auto* c = resolveInspectorComponent(e.eventComponent)) {
        lockedComponent_ = c;
        updateInspectorForComponent(c);
    }
}

void DigiStrikeFS2AudioProcessorEditor::mouseDrag(const juce::MouseEvent& e) {
    juce::ignoreUnused(e);
    if (auto* s = dynamic_cast<juce::Slider*>(e.eventComponent)) {
        activeBadgeSlider_ = s;
        activeBadgeExpiry_ = juce::Time::getMillisecondCounter() + 350;
        s->getProperties().set("badgeExpiry", static_cast<double>(activeBadgeExpiry_));
        s->repaint();
    }
    if (lockedComponent_ != nullptr) {
        updateInspectorForComponent(lockedComponent_.getComponent());
    }
}

void DigiStrikeFS2AudioProcessorEditor::mouseUp(const juce::MouseEvent& e) {
    if (auto* s = dynamic_cast<juce::Slider*>(e.eventComponent)) {
        activeBadgeSlider_ = s;
        activeBadgeExpiry_ = juce::Time::getMillisecondCounter() + 350;
        s->getProperties().set("badgeExpiry", static_cast<double>(activeBadgeExpiry_));
        s->repaint();
    }
    lockedComponent_ = nullptr;
    auto* c = resolveInspectorComponent(getComponentAt(e.getEventRelativeTo(this).getPosition()));
    if (c != nullptr) {
        updateInspectorForComponent(c);
    } else {
        headerInspector_.setReadoutDefault();
    }
}

void DigiStrikeFS2AudioProcessorEditor::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) {
    juce::ignoreUnused(wheel);
    if (auto* s = dynamic_cast<juce::Slider*>(e.eventComponent)) {
        activeBadgeSlider_ = s;
        activeBadgeExpiry_ = juce::Time::getMillisecondCounter() + 350;
        s->getProperties().set("badgeExpiry", static_cast<double>(activeBadgeExpiry_));
        s->repaint();
    }
}

void DigiStrikeFS2AudioProcessorEditor::mouseExit(const juce::MouseEvent& e) {
    juce::ignoreUnused(e);
    if (lockedComponent_ == nullptr) {
        headerInspector_.setReadoutDefault();
    }
}

void DigiStrikeFS2AudioProcessorEditor::setupKnob(KnobItem& item, const juce::String& paramId,
                                                   const juce::String& name, const juce::String& unit,
                                                   const juce::String& blurb, juce::Colour col, bool isBipolar) {
    juce::ignoreUnused(blurb);
    item.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    item.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    item.slider.setColour(juce::Slider::rotarySliderFillColourId, col);
    if (isBipolar) {
        item.slider.getProperties().set("centerAtNoon", true);
        const double dblVal = (paramId == pan || paramId == decay_curve || paramId == filter_env_amt || paramId == shift || paramId == squash || paramId == wider) ? 0.0 : 0.5;
        item.slider.setDoubleClickReturnValue(true, dblVal);
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
        if (name.startsWith("X:") || name.startsWith("Y:") || name.startsWith("Z:") || name.startsWith("T:")
            || name.startsWith("VEL->")) {
            const float normVal = static_cast<float>(val);
            const float bipPercent = (normVal - 0.5f) * 200.0f;
            return (bipPercent > 0.0f ? "+" : "") + juce::String(juce::roundToInt(bipPercent)) + "%";
        }
        if (name == "PAN") {
            const int p = juce::roundToInt(val * 100.0);
            if (p == 0) return "C";
            if (p < 0) return juce::String(-p) + "% L";
            return juce::String(p) + "% R";
        }
        if (name == "DECAY CRV") {
            if (val <= -0.05) return "EXP " + juce::String(val, 2);
            if (val >= 0.05) return "LOG +" + juce::String(val, 2);
            return "LIN 0.00";
        }
        if (name == "VOLUME") {
            const float u = static_cast<float>(val);
            if (u <= 0.0001f) return "-inf dB";
            float db = (u <= 0.5f) ? (-60.0f + (u / 0.5f) * 60.0f) : ((u - 0.5f) / 0.5f * 12.0f);
            return (db > 0.0f ? "+" : "") + juce::String(db, 1) + " dB";
        }
        if (name == "AMP-DEC" || name == "FE-DEC" || name == "GLIDE") {
            if (val >= 1.0) return juce::String(val, 2) + " s";
            return juce::String(juce::roundToInt(val * 1000.0)) + " ms";
        }
        if (name == "TIME") {
            if (std::abs(val) <= 0.02) return "INSTANT";
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

    registerInspector(&item.slider, paramId, col, &item.slider);
    registerInspector(&item.label, paramId, col, &item.slider);

    item.slider.onValueChange = [this, &item]() {
        if (lockedComponent_ == &item.slider || lockedComponent_ == &item.label) {
            updateInspectorForComponent(&item.slider);
        }
    };
}

void DigiStrikeFS2AudioProcessorEditor::timerCallback() {
    if (activeBadgeSlider_ != nullptr) {
        if (juce::Time::getMillisecondCounter() >= activeBadgeExpiry_) {
            activeBadgeSlider_->getProperties().remove("badgeExpiry");
            activeBadgeSlider_->repaint();
            activeBadgeSlider_ = nullptr;
        }
    }

    auto& apvts = processor_.getAPVTS();

    // Sync state buttons with APVTS
    if (auto* p = apvts.getRawParameterValue("category")) {
        categoryGrid_.setState(static_cast<int>(p->load()));
    }
    uint32_t curMask = 0;
    for (size_t i = 0; i < 20; ++i) {
        if (auto* p = apvts.getRawParameterValue(DigiStrikeFS2Info::kLineageParamIds[i])) {
            if (p->load(std::memory_order_relaxed) > 0.5f) curMask |= (1u << i);
        }
    }
    lineageGrid_.setMask(curMask);

    int catIdx = 0;
    if (auto* pCat = apvts.getRawParameterValue(category)) {
        catIdx = static_cast<int>(pCat->load());
    }
    float morphNorm = 0.0f;
    if (auto* pMorph = apvts.getRawParameterValue(table_morph)) {
        morphNorm = pMorph->load() * 0.01f;
    }
    lineageGrid_.updateMorphDNA(curMask, catIdx, morphNorm);
    if (auto* p = apvts.getRawParameterValue("voice_mode")) {
        voiceModeRocker_.setState(static_cast<int>(p->load()));
    }
    if (auto* p = apvts.getRawParameterValue("life_mode")) {
        lifeformRocker_.setState(static_cast<int>(p->load()));
    }
    if (auto* p = apvts.getRawParameterValue("loop_mode")) {
        int val = static_cast<int>(std::lround(p->load()));
        loopModeBar_.setState(std::clamp(val, 0, 3));
    }

    // Update Voice Selector Capsule
    {
        const auto& rom = tkdsp::DigiStrikeFS2ROM::getInstance();
        int curCat = categoryGrid_.getState();
        uint32_t mask = lineageGrid_.getMask();
        float morphNorm = 0.0f;
        if (auto* p = apvts.getRawParameterValue("table_morph"))
            morphNorm = std::clamp(p->load() * 0.01f, 0.0f, 1.0f);

        const int N = DigiStrikeFS2Fleet::getActiveFleetCount(rom, curCat, mask);
        DigiStrikeFS2Fleet::VoiceRef ref;
        if (N > 0 && DigiStrikeFS2Fleet::resolveFleetIndex(rom, curCat, mask, DigiStrikeFS2Fleet::indexForMorph(morphNorm, N), ref)) {
            const char* vName = rom.getVoiceName(ref.romIndex);
            int activeLins = 0;
            for (int i = 0; i < 20; ++i) if ((mask & (1u << i)) != 0) ++activeLins;
            if (activeLins > 1) {
                voiceSelectorCapsule_.setText(juce::String::fromUTF8(vName) + " (" + juce::String(ref.globalIndex + 1) + "/" + juce::String(ref.totalVoices) + ") \xc2\xb7 " + juce::String(DigiStrikeFS2Fleet::kLineageNames[ref.lineage]));
            } else {
                voiceSelectorCapsule_.setText(juce::String::fromUTF8(vName) + " (" + juce::String(ref.localIndex + 1) + "/" + juce::String(ref.localCount) + ")");
            }
        } else {
            voiceSelectorCapsule_.setText("No Voices in Pool");
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

    const auto tele = processor_.getTelemetrySnapshot();

    // 1. Card 1 Knobs: Macro Halo Calculations
    const float mX = static_cast<float>(macroXKnob_.slider.getValue());
    const float mY = static_cast<float>(macroYKnob_.slider.getValue());
    const float mZ = static_cast<float>(macroZKnob_.slider.getValue());
    const float mT = static_cast<float>(macroTKnob_.slider.getValue());

    const float depthX = static_cast<float>(randXKnob_.slider.getValue()) * 0.01f;
    const float depthY = static_cast<float>(randYKnob_.slider.getValue()) * 0.01f;
    const float depthZ = static_cast<float>(randZKnob_.slider.getValue()) * 0.01f;
    const float depthT = static_cast<float>(randTKnob_.slider.getValue()) * 0.01f;

    const auto haloX = CrackedChimes::DSP::computeStepRandHalo(mX, depthX, tele.randVal[0]);
    if (haloX.active) {
        updateProp(macroXKnob_.slider, "randLo", haloX.lo);
        updateProp(macroXKnob_.slider, "randHi", haloX.hi);
        updateProp(macroXKnob_.slider, "randPip", haloX.pip);
    } else {
        clearProp(macroXKnob_.slider);
    }

    const auto haloY = CrackedChimes::DSP::computeStepRandHalo(mY, depthY, tele.randVal[1]);
    if (haloY.active) {
        updateProp(macroYKnob_.slider, "randLo", haloY.lo);
        updateProp(macroYKnob_.slider, "randHi", haloY.hi);
        updateProp(macroYKnob_.slider, "randPip", haloY.pip);
    } else {
        clearProp(macroYKnob_.slider);
    }

    const auto haloZ = CrackedChimes::DSP::computeStepRandHalo(mZ, depthZ, tele.randVal[2]);
    if (haloZ.active) {
        updateProp(macroZKnob_.slider, "randLo", haloZ.lo);
        updateProp(macroZKnob_.slider, "randHi", haloZ.hi);
        updateProp(macroZKnob_.slider, "randPip", haloZ.pip);
    } else {
        clearProp(macroZKnob_.slider);
    }

    const auto haloT = CrackedChimes::DSP::computeStepRandHalo(mT, depthT, tele.randVal[3]);
    if (haloT.active) {
        updateProp(macroTKnob_.slider, "randLo", haloT.lo);
        updateProp(macroTKnob_.slider, "randHi", haloT.hi);
        updateProp(macroTKnob_.slider, "randPip", haloT.pip);
    } else {
        clearProp(macroTKnob_.slider);
    }

    // Velocity Aggression & Timbre
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

    // Pan & Decay Curve
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

    // 2. Card 2: Inner Orange & Outer Yellow Halos
    const float normPDec = static_cast<float>(pitchDecayKnob_.slider.valueToProportionOfLength(pitchDecayKnob_.slider.getValue()));
    updateModProp(pitchDecayKnob_.slider, tkdsp::formless::computeMacroHalo(normPDec, mT, 0.70f));
    updateRandProp(pitchDecayKnob_.slider, tkdsp::formless::trickleDownHeadroom(normPDec, haloT.lo, haloT.hi, haloT.pip, 0.70f));

    const float normPunch = static_cast<float>(punchKnob_.slider.valueToProportionOfLength(punchKnob_.slider.getValue()));
    updateModProp(punchKnob_.slider, tkdsp::formless::computeMacroHalo(normPunch, mY, 0.80f));
    updateRandProp(punchKnob_.slider, tkdsp::formless::trickleDownArbitrate(normPunch, haloY.lo, haloY.hi, haloY.pip, 0.80f,
                                                                           haloAggr.lo, haloAggr.hi, haloAggr.pip, 0.75f, tele.lastVelocity));

    const float normClick = static_cast<float>(clickKnob_.slider.valueToProportionOfLength(clickKnob_.slider.getValue()));
    updateModProp(clickKnob_.slider, tkdsp::formless::computeMacroHalo(normClick, mY, 0.75f));
    updateRandProp(clickKnob_.slider, tkdsp::formless::trickleDownArbitrate(normClick, haloY.lo, haloY.hi, haloY.pip, 0.75f,
                                                                           haloTimb.lo, haloTimb.hi, haloTimb.pip, 0.65f, tele.lastVelocity));

    const float normNoise = static_cast<float>(noiseKnob_.slider.valueToProportionOfLength(noiseKnob_.slider.getValue()));
    updateModProp(noiseKnob_.slider, tkdsp::formless::computeMacroHalo(normNoise, mZ, 0.70f));
    updateRandProp(noiseKnob_.slider, tkdsp::formless::trickleDownArbitrate(normNoise, haloZ.lo, haloZ.hi, haloZ.pip, 0.70f,
                                                                            haloAggr.lo, haloAggr.hi, haloAggr.pip, 0.70f, tele.lastVelocity));

    const float normStart = static_cast<float>(startKnob_.slider.valueToProportionOfLength(startKnob_.slider.getValue()));
    updateModProp(startKnob_.slider, tkdsp::formless::computeMacroHalo(normStart, mX, 0.85f));
    updateRandProp(startKnob_.slider, tkdsp::formless::trickleDownHeadroom(normStart, haloX.lo, haloX.hi, haloX.pip, 0.85f));

    const float normLen = static_cast<float>(lengthKnob_.slider.valueToProportionOfLength(lengthKnob_.slider.getValue()));
    updateModProp(lengthKnob_.slider, tkdsp::formless::computeMacroHalo(normLen, mX, 0.75f));
    updateRandProp(lengthKnob_.slider, tkdsp::formless::trickleDownHeadroom(normLen, haloX.lo, haloX.hi, haloX.pip, 0.75f));

    const float normTime = static_cast<float>(timeKnob_.slider.valueToProportionOfLength(timeKnob_.slider.getValue()));
    updateModProp(timeKnob_.slider, tkdsp::formless::computeMacroHalo(normTime, mT, 0.80f));
    updateRandProp(timeKnob_.slider, tkdsp::formless::trickleDownDecay(normTime, haloDecay.lo, haloDecay.hi, haloDecay.pip, haloT.lo, haloT.hi, haloT.pip, 0.70f));

    const float normMorph = static_cast<float>(morphKnob_.slider.valueToProportionOfLength(morphKnob_.slider.getValue()));
    updateModProp(morphKnob_.slider, tkdsp::formless::computeMacroHalo(normMorph, mZ, 0.75f));
    updateRandProp(morphKnob_.slider, tkdsp::formless::trickleDownArbitrate(normMorph, haloZ.lo, haloZ.hi, haloZ.pip, 0.75f,
                                                                            haloTimb.lo, haloTimb.hi, haloTimb.pip, 0.70f, tele.lastVelocity));

    // 3. Card 3: Cutoff Filter Envelope Arc Halo (Electric Cyan Inner Arc)
    auto& cutSlider = cutoffKnob_.slider;
    const float feAmt = static_cast<float>(filterEnvAmtKnob_.slider.getValue());
    const double curCutoffHz = cutSlider.getValue();
    const float normBase = static_cast<float>(cutSlider.valueToProportionOfLength(curCutoffHz));

    tkdsp::formless::HaloRange cutHalo;
    if (std::abs(feAmt) > 0.005f) {
        // Dynamic Filter Envelope throw (+/-5.0 Octaves at full velocity 1.0)
        const float peakOctaves = std::clamp(feAmt * 5.0f, -6.0f, 6.0f);
        const double peakCutoffHz = std::clamp(curCutoffHz * std::pow(2.0, static_cast<double>(peakOctaves)), 20.0, 20000.0);
        const float normPeak = static_cast<float>(cutSlider.valueToProportionOfLength(peakCutoffHz));

        cutHalo.lo = std::min(normBase, normPeak);
        cutHalo.hi = std::max(normBase, normPeak);
        cutHalo.pip = normPeak;
        cutHalo.active = (cutHalo.hi - cutHalo.lo > 0.002f);
    } else {
        cutHalo.active = false;
    }
    updateModProp(cutSlider, cutHalo);
}

void DigiStrikeFS2AudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xFF0D1117));

    // Header Title
    g.setFont(juce::Font(juce::FontOptions(15.0f).withStyle("Bold")));
    g.setColour(juce::Colour(0xFFFFAA00));
    g.drawText("DIGISTRIKE FS2", 20, 8, 140, 28, juce::Justification::centredLeft);

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
    const juce::Rectangle<int> kCard1{20, 44, 424, 174};
    const juce::Rectangle<int> kCard2{456, 44, 424, 174};
    drawCard(kCard1);
    drawCardPill(kCard1, 240, "CARD 1: MASTER MACRO & DYNAMICS", juce::Colour(0xFF00C8FF));
    drawCard(kCard2);

    // Dedicated Pod: Sovereign Lineages (20 Families)
    const juce::Rectangle<int> kPodLineage{20, 226, 860, 76};
    drawPod(kPodLineage, 270, "SOVEREIGN LINEAGE MATRIX (20 FAMILIES)", juce::Colour(0xFF00E5FF));

    // Sovereign Lineage Status Badge (Option B: N LIN · X / 64 WAVES)
    {
        const uint32_t linMask = lineageGrid_.getMask();
        int activeLinCount = 0;
        for (int i = 0; i < 20; ++i) {
            if ((linMask & (1u << i)) != 0) ++activeLinCount;
        }
        const int activeWaves = std::min(64, activeLinCount * 4);

        auto linBadge = juce::Rectangle<float>(716.0f, 231.0f, 154.0f, 15.0f);
        g.setColour(juce::Colour(0xFF181C26));
        g.fillRoundedRectangle(linBadge, 3.0f);
        g.setColour(juce::Colour(0xFF262D38));
        g.drawRoundedRectangle(linBadge, 3.0f, 0.8f);

        g.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
        if (activeLinCount >= 20) {
            g.setColour(juce::Colour(0xFFFFAA00));
            g.drawText("20 LIN \xc2\xb7 80/80 FULL", linBadge, juce::Justification::centred);
        } else {
            g.setColour(juce::Colour(0xFF00E5FF));
            g.drawText(juce::String(activeLinCount) + " LIN \xc2\xb7 " + juce::String(activeLinCount * 4) + "/80 WAVES", linBadge, juce::Justification::centred);
        }
    }

    // Card 3 + Pods
    const juce::Rectangle<int> kCard3{20, 312, 860, 232};
    const juce::Rectangle<int> kPod1{28, 320, 296, 216};
    const juce::Rectangle<int> kPod2{332, 320, 92, 216};
    const juce::Rectangle<int> kPod3{432, 320, 440, 216};

    drawCard(kCard3);
    drawPod(kPod1, 190, "STEPPED RANDOM HUMANIZER", juce::Colour(0xFFB374FF));
    drawPod(kPod2, kPod2.getWidth() - 16, "ALTAR", juce::Colour(0xFFFF9D00));
    drawPod(kPod3, 190, "VCF / MOJO / OUTPUT", juce::Colour(0xFFFF5252));

    // Pod 3 Internals: Mojo Row separator & Master-Column divider
    const int p3y = kPod3.getY();
    g.setColour(juce::Colour(0xFF1F2633));
    g.drawHorizontalLine(p3y + 104, (float)kPod3.getX() + 8.0f, (float)kPod3.getX() + 280.0f);
    g.drawVerticalLine(kPod3.getX() + 288, (float)p3y + 24.0f, (float)kPod3.getBottom() - 8.0f);
}

void DigiStrikeFS2AudioProcessorEditor::resized() {
    // 1. Header Layout (Restored Preset Stub at 174)
    presetStub_.setBounds(174, 8, 145, 28);
    headerInspector_.setBounds(326, 8, 314, 28);
    lifeformRocker_.setBounds(646, 8, 234, 28);

    // Helper: place label directly above slider
    auto place = [](KnobItem& k, int cx, int labelY, int labelW, int size, int sliderY) {
        k.label.setBounds(cx - labelW / 2, labelY, labelW, 14);
        k.slider.setBounds(cx - size / 2, sliderY, size, size);
    };

    auto colCentre = [](int cardX, int col, int cellPitch) {
        return cardX + 16 + cellPitch / 2 + col * cellPitch;
    };

    // Cards 1 & 2:
    const int kCard1X = 20;
    const int kCard2X = 456;
    const int kCellPitch = 98;

    KnobItem* c1[2][4] = {
        {&macroXKnob_,  &macroYKnob_,  &macroZKnob_, &macroTKnob_},
        {&velAggrKnob_, &velTimbKnob_, &panKnob_,    &decayCurveKnob_}
    };

    // Card 1 Knobs: Symmetrical 2x4 grid (52px knobs, cleared below header pill)
    for (int r = 0; r < 2; ++r) {
        const int labelY = (r == 0) ? 76 : 148;
        const int sliderY = labelY + 16;
        for (int c = 0; c < 4; ++c) {
            place(*c1[r][c], colCentre(kCard1X, c, kCellPitch), labelY, kCellPitch, 52, sliderY);
        }
    }

    // Card 2 Header: Category Role 1x8 button strip
    categoryGrid_.setBounds(468, 46, 400, 20);

    // Voice Selector Capsule
    voiceSelectorCapsule_.setBounds(468, 68, 318, 18);

    // Card 2 Knobs: 4 columns with 76px pitch centered at cx in {496, 572, 648, 724}
    KnobItem* c2[2][4] = {
        {&pitchDecayKnob_, &clickKnob_, &startKnob_,  &timeKnob_},
        {&punchKnob_,      &noiseKnob_, &lengthKnob_, &morphKnob_}
    };

    for (int r = 0; r < 2; ++r) {
        const int labelY = (r == 0) ? 88 : 158;
        const int sliderY = (r == 0) ? 104 : 172;
        for (int c = 0; c < 4; ++c) {
            place(*c2[r][c], 496 + c * 76, labelY, 76, 52, sliderY);
        }
    }

    // Card 2 Right Bay: LOOP 2x2 Switch & REV Button
    loopLabel_.setBounds(782, 90, 76, 14);
    loopModeBar_.setBounds(782, 108, 76, 52);
    revButton_.setBounds(782, 166, 76, 22);

    // Dedicated Full-Width Pod: Sovereign Lineage 2x10 Button Matrix
    lineageGrid_.setBounds(32, 246, 836, 48);

    // Card 3 Pod 1: Stepped Random (2x4 grid, 44px knobs, 70px pitch, width 296px)
    const int kPod1X = 28;
    const int kRandPitch = 70;
    KnobItem* p1[2][4] = {
        {&randXKnob_,    &randYKnob_,    &randZKnob_,   &randTKnob_},
        {&randAggrKnob_, &randTimbKnob_, &randPanKnob_, &randDecayKnob_}
    };

    for (int r = 0; r < 2; ++r) {
        const int labelY = (r == 0) ? 346 : 434;
        const int sliderY = labelY + 15;
        for (int c = 0; c < 4; ++c) {
            place(*p1[r][c], kPod1X + 8 + kRandPitch / 2 + c * kRandPitch, labelY, kRandPitch, 44, sliderY);
        }
    }

    // Card 3 Pod 2: Performance Altar
    {
        const int cx = 332 + 92 / 2; // 378
        place(macroDecayKnob_, cx, 340, 88, 72, 356);
        trigButton_.setBounds(cx - 36, 452, 72, 48);
    }

    // Card 3 Pod 3: VCF, Mojo & Output
    {
        const int x0 = 438;
        const int pitch = 52;
        const int labelY0 = 346;
        const int sliderY0 = labelY0 + 15;
        const int labelY1 = 434;
        const int sliderY1 = labelY1 + 15;

        // Row 0 (VCF Strip): CUTOFF, RES, FE-AMT, FE-DEC, and filterTypeBar_ (48x48)
        place(cutoffKnob_,       x0 + pitch / 2 + pitch * 0, labelY0, pitch, 44, sliderY0);
        place(resonanceKnob_,    x0 + pitch / 2 + pitch * 1, labelY0, pitch, 44, sliderY0);
        place(filterEnvAmtKnob_, x0 + pitch / 2 + pitch * 2, labelY0, pitch, 44, sliderY0);
        place(filterDecayKnob_,  x0 + pitch / 2 + pitch * 3, labelY0, pitch, 44, sliderY0);
        filterTypeBar_.setBounds(x0 + pitch * 4 + 4, sliderY0 - 3, 48, 48);

        // Row 1 (Mojo Strip): DRIVE, CRUSH, SHIFT, SQUASH, WIDER
        place(filterDriveKnob_,  x0 + pitch / 2 + pitch * 0, labelY1, pitch, 44, sliderY1);
        place(crushKnob_,        x0 + pitch / 2 + pitch * 1, labelY1, pitch, 44, sliderY1);
        place(shiftKnob_,        x0 + pitch / 2 + pitch * 2, labelY1, pitch, 44, sliderY1);
        place(squashKnob_,       x0 + pitch / 2 + pitch * 3, labelY1, pitch, 44, sliderY1);
        place(widerKnob_,        x0 + pitch / 2 + pitch * 4, labelY1, pitch, 44, sliderY1);

        // Master Output Bay:
        // Col AX (754): SUSTAIN, AMP-DEC, GLIDE
        // Col BX (822): LIMIT, VOLUME
        // Bottom: [ MONO | GLIDE | POLY ] rocker directly below amp knobs
        const int colAX = 754;
        const int colBX = 822;

        // Envelope Column (Col AX)
        place(sustainKnob_,   colAX, 342, 48, 40, 356);
        place(ampDecayKnob_,  colAX, 398, 48, 40, 412);
        place(glideTimeKnob_, colAX, 454, 48, 40, 468);

        // Master Level Column (Col BX)
        limiterButton_.setBounds(colBX - 26, 356, 52, 22);
        place(volumeKnob_,    colBX, 398, 48, 46, 412);

        // Voice Mode Selector (Directly below amp knobs in Pod 3)
        voiceModeRocker_.setBounds(738, 510, 116, 22);
    }
}
