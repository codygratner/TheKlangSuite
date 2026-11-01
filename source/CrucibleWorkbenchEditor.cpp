#include "CrucibleWorkbenchEditor.h"

// ==============================================================================
// THE CRUCIBLE WORKBENCH EDITOR IMPLEMENTATION
// ==============================================================================

CrucibleWorkbenchEditor::CrucibleWorkbenchEditor(CrucibleWorkbenchAudioProcessor& p)
    : AudioProcessorEditor(&p), processor_(p)
{
    // High-visibility flagship dimensions (1380 x 620)
    setSize(1380, 620);
    auto& apvts = processor_.getAPVTS();

    // --------------------------------------------------------------------------
    // 1. Setup Col 0: MmeKick 16-Control Flagship Tall Card
    // --------------------------------------------------------------------------
    {
        kickUI_.titleLabel.setText("S1: MME KICK", juce::dontSendNotification);
        kickUI_.titleLabel.setFont(juce::Font(14.5f, juce::Font::bold));
        kickUI_.titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xfff59e0b));
        kickUI_.titleLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(kickUI_.titleLabel);

        kickUI_.modelReadoutLabel.setText(processor_.getMmeKickModelName(), juce::dontSendNotification);
        kickUI_.modelReadoutLabel.setFont(juce::Font(13.0f, juce::Font::bold));
        kickUI_.modelReadoutLabel.setColour(juce::Label::textColourId, juce::Colour(0xfff59e0b));
        kickUI_.modelReadoutLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(kickUI_.modelReadoutLabel);

        kickUI_.trigButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
        kickUI_.trigButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        kickUI_.trigButton.onClick = [this]() {
            processor_.triggerSlot(0, 0.95f);
        };
        addAndMakeVisible(kickUI_.trigButton);

        static const char* kickParamLabels[16] = {
            "MODEL", "PITCH", "DECAY", "PUNCH",
            "SWEEP", "SWP-T", "SUB",   "BODY",
            "FLTF",  "FLTW",  "FLTQ",  "EQ-F",
            "EQ-G",  "SRR",   "BRR",   "FOLD"
        };

        for (int k = 0; k < 16; ++k) {
            auto& knob = kickUI_.knobs[k];
            knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            knob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xfff59e0b));
            knob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff334155));
            knob.setColour(juce::Slider::thumbColourId, juce::Colour(0xffffffff));
            addAndMakeVisible(knob);

            auto& lbl = kickUI_.knobLabels[k];
            lbl.setText(kickParamLabels[k], juce::dontSendNotification);
            lbl.setFont(juce::Font(13.0f, juce::Font::bold));
            lbl.setColour(juce::Label::textColourId, juce::Colour(0xfff8fafc));
            lbl.setJustificationType(juce::Justification::centred);
            addAndMakeVisible(lbl);

            juce::String paramID = "s0_p" + juce::String(k);
            kickUI_.attachments[k] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                apvts, paramID, knob
            );
        }
    }

    // --------------------------------------------------------------------------
    // 2. Setup Col 1: Snare & Clap Flagship Workstation (16 Knobs, 4 Trigs)
    // --------------------------------------------------------------------------
    {
        snareClapUI_.titleLabel.setText("SNARE & CLAP", juce::dontSendNotification);
        snareClapUI_.titleLabel.setFont(juce::Font(14.5f, juce::Font::bold));
        snareClapUI_.titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff38bdf8));
        snareClapUI_.titleLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(snareClapUI_.titleLabel);

        snareClapUI_.sdModelLabel.setText(processor_.getMmeSnareModelName(), juce::dontSendNotification);
        snareClapUI_.sdModelLabel.setFont(juce::Font(13.0f, juce::Font::bold));
        snareClapUI_.sdModelLabel.setColour(juce::Label::textColourId, juce::Colour(0xfff59e0b));
        snareClapUI_.sdModelLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(snareClapUI_.sdModelLabel);

        snareClapUI_.cpModelLabel.setText(processor_.getMmeClapModelName(), juce::dontSendNotification);
        snareClapUI_.cpModelLabel.setFont(juce::Font(13.0f, juce::Font::bold));
        snareClapUI_.cpModelLabel.setColour(juce::Label::textColourId, juce::Colour(0xff38bdf8));
        snareClapUI_.cpModelLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(snareClapUI_.cpModelLabel);

        auto setupTrigBtn = [this](juce::TextButton& btn, juce::Colour col, std::function<void()> cb) {
            btn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
            btn.setColour(juce::TextButton::textColourOffId, col);
            btn.onClick = cb;
            addAndMakeVisible(btn);
        };

        setupTrigBtn(snareClapUI_.lilSdButton, juce::Colour(0xfff59e0b), [this]() { processor_.triggerSnare(false, 0.95f); });
        setupTrigBtn(snareClapUI_.bigSdButton, juce::Colour(0xfff59e0b), [this]() { processor_.triggerSnare(true, 0.95f); });
        setupTrigBtn(snareClapUI_.lilCpButton, juce::Colour(0xff38bdf8), [this]() { processor_.triggerClap(false, 0.95f); });
        setupTrigBtn(snareClapUI_.bigCpButton, juce::Colour(0xff38bdf8), [this]() { processor_.triggerClap(true, 0.95f); });

        static const char* scParamNames[16] = {
            "sc_sd_model", "sc_sd_tune", "sc_sd_decay", "sc_sd_snap",
            "sc_sd_tone",  "sc_sd_wires", "sc_sd_body", "sc_sd_var",
            "sc_cp_model", "sc_cp_tone", "sc_cp_decay", "sc_cp_burst",
            "sc_cp_spread", "sc_cp_var", "sc_mojo_fltf", "sc_mojo_dist"
        };
        static const char* scParamLabels[16] = {
            "SD MOD",  "SD TUNE", "SD DEC",  "SD SNAP",
            "SD TONE", "SD WIRE", "SD BODY", "SD VAR",
            "CP MOD",  "CP TONE", "CP DEC",  "BURST",
            "SPREAD",  "CP VAR",  "SC FLT",  "DRIVE"
        };

        for (int k = 0; k < 16; ++k) {
            auto& knob = snareClapUI_.knobs[k];
            knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

            const juce::Colour col = (k < 8) ? juce::Colour(0xfff59e0b) : juce::Colour(0xff38bdf8);
            knob.setColour(juce::Slider::rotarySliderFillColourId, col);
            knob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff334155));
            knob.setColour(juce::Slider::thumbColourId, juce::Colour(0xffffffff));
            addAndMakeVisible(knob);

            auto& lbl = snareClapUI_.knobLabels[k];
            lbl.setText(scParamLabels[k], juce::dontSendNotification);
            lbl.setFont(juce::Font(13.0f, juce::Font::bold));
            lbl.setColour(juce::Label::textColourId, juce::Colour(0xfff8fafc));
            lbl.setJustificationType(juce::Justification::centred);
            addAndMakeVisible(lbl);

            snareClapUI_.attachments[k] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                apvts, scParamNames[k], knob
            );
        }
    }

    // --------------------------------------------------------------------------
    // 3. Setup Col 2: Hi-Hat Flagship Workstation (16 Knobs, 2 Trigs)
    // --------------------------------------------------------------------------
    {
        hatUI_.titleLabel.setText("HI-HAT WORKSTATION", juce::dontSendNotification);
        hatUI_.titleLabel.setFont(juce::Font(14.5f, juce::Font::bold));
        hatUI_.titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff06b6d4));
        hatUI_.titleLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(hatUI_.titleLabel);

        hatUI_.infoLabel.setText("7-STAGE MATRIX: 65..71", juce::dontSendNotification);
        hatUI_.infoLabel.setFont(juce::Font(13.0f, juce::Font::bold));
        hatUI_.infoLabel.setColour(juce::Label::textColourId, juce::Colour(0xff06b6d4));
        hatUI_.infoLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(hatUI_.infoLabel);

        hatUI_.chTrigButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
        hatUI_.chTrigButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff06b6d4));
        hatUI_.chTrigButton.onClick = [this]() { processor_.triggerHat(false, 0.95f); };
        addAndMakeVisible(hatUI_.chTrigButton);

        hatUI_.ohTrigButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
        hatUI_.ohTrigButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff38bdf8));
        hatUI_.ohTrigButton.onClick = [this]() { processor_.triggerHat(true, 0.95f); };
        addAndMakeVisible(hatUI_.ohTrigButton);

        static const char* hatParamNames[16] = {
            "hat_pitch", "hat_cl_decay", "hat_op_decay", "hat_metal",
            "hat_choke", "hat_bright",   "hat_pedal",    "hat_accent",
            "hat_fltf",  "hat_fltw",     "hat_fltq",     "hat_eqf",
            "hat_eqg",   "hat_srr",      "hat_brr",      "hat_drive"
        };
        static const char* hatParamLabels[16] = {
            "PITCH",  "CH DEC", "OH DEC", "METAL",
            "CHOKE",  "BRIGHT", "PEDAL",  "ACCENT",
            "FLTF",   "FLTW",   "FLTQ",   "EQ-F",
            "EQ-G",   "SRR",    "BRR",    "DRIVE"
        };

        for (int k = 0; k < 16; ++k) {
            auto& knob = hatUI_.knobs[k];
            knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

            const juce::Colour col = (k < 8) ? juce::Colour(0xff06b6d4) : juce::Colour(0xff38bdf8);
            knob.setColour(juce::Slider::rotarySliderFillColourId, col);
            knob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff334155));
            knob.setColour(juce::Slider::thumbColourId, juce::Colour(0xffffffff));
            addAndMakeVisible(knob);

            auto& lbl = hatUI_.knobLabels[k];
            lbl.setText(hatParamLabels[k], juce::dontSendNotification);
            lbl.setFont(juce::Font(13.0f, juce::Font::bold));
            lbl.setColour(juce::Label::textColourId, juce::Colour(0xfff8fafc));
            lbl.setJustificationType(juce::Justification::centred);
            addAndMakeVisible(lbl);

            hatUI_.attachments[k] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                apvts, hatParamNames[k], knob
            );
        }
    }

    // --------------------------------------------------------------------------
    // 4. Setup Stacked Slots (Col 3 & Col 4 Top)
    // --------------------------------------------------------------------------
    auto setupStandardSlot = [this, &apvts](SlotUI& ui, int slotIdx, const juce::String& defaultTitle, juce::Colour col) {
        ui.titleLabel.setText(defaultTitle, juce::dontSendNotification);
        ui.titleLabel.setFont(juce::Font(14.5f, juce::Font::bold));
        ui.titleLabel.setColour(juce::Label::textColourId, col);
        ui.titleLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(ui.titleLabel);

        ui.trigButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
        ui.trigButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        ui.trigButton.onClick = [this, slotIdx]() {
            processor_.triggerSlot(slotIdx, 0.95f);
        };
        addAndMakeVisible(ui.trigButton);

        for (int k = 0; k < 8; ++k) {
            auto& knob = ui.knobs[k];
            knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            knob.setColour(juce::Slider::rotarySliderFillColourId, col);
            knob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff334155));
            knob.setColour(juce::Slider::thumbColourId, juce::Colour(0xffffffff));
            addAndMakeVisible(knob);

            auto& lbl = ui.knobLabels[k];
            lbl.setFont(juce::Font(13.0f, juce::Font::bold));
            lbl.setColour(juce::Label::textColourId, juce::Colour(0xfff8fafc));
            lbl.setJustificationType(juce::Justification::centred);
            addAndMakeVisible(lbl);

            juce::String paramID = "s" + juce::String(slotIdx) + "_p" + juce::String(k);
            ui.attachments[k] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                apvts, paramID, knob
            );
        }
    };

    setupStandardSlot(tomUI_, 2, "S3: TOMS / PERC", juce::Colour(0xffa855f7));
    setupStandardSlot(westZapUI_, 3, "S4: WEST / FM ZAP", juce::Colour(0xff10b981));
    setupStandardSlot(crashUI_, 7, "S8: CRASH / CYM", juce::Colour(0xff38bdf8));

    updateEngineLabels(2);
    updateEngineLabels(3);
    updateEngineLabels(7);

    // --------------------------------------------------------------------------
    // 5. Setup Dedicated 808 Sub-Bass Card (Col 4 Bottom)
    // --------------------------------------------------------------------------
    {
        sub808UI_.titleLabel.setText("808 BASS", juce::dontSendNotification);
        sub808UI_.titleLabel.setFont(juce::Font(14.5f, juce::Font::bold));
        sub808UI_.titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xfff59e0b));
        sub808UI_.titleLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(sub808UI_.titleLabel);

        sub808UI_.modelReadoutLabel.setText(processor_.getSub808ModelName(), juce::dontSendNotification);
        sub808UI_.modelReadoutLabel.setFont(juce::Font(13.0f, juce::Font::bold));
        sub808UI_.modelReadoutLabel.setColour(juce::Label::textColourId, juce::Colour(0xfff59e0b));
        sub808UI_.modelReadoutLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(sub808UI_.modelReadoutLabel);

        sub808UI_.trigButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
        sub808UI_.trigButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        sub808UI_.trigButton.onClick = [this]() {
            processor_.triggerSub808(0.95f);
        };
        addAndMakeVisible(sub808UI_.trigButton);

        static const char* sub808ParamNames[8] = {
            "sub808_model", "sub808_tune", "sub808_decay", "sub808_punch",
            "sub808_sweep", "sub808_fold", "sub808_drive", "sub808_glide"
        };
        static const char* sub808Labels[8] = {
            "MODEL", "TUNE", "DECAY", "PUNCH",
            "SWEEP", "FOLD", "DRIVE", "GLIDE"
        };

        for (int k = 0; k < 8; ++k) {
            auto& knob = sub808UI_.knobs[k];
            knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            knob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xfff59e0b));
            knob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff334155));
            knob.setColour(juce::Slider::thumbColourId, juce::Colour(0xffffffff));
            addAndMakeVisible(knob);

            auto& lbl = sub808UI_.knobLabels[k];
            lbl.setText(sub808Labels[k], juce::dontSendNotification);
            lbl.setFont(juce::Font(13.0f, juce::Font::bold));
            lbl.setColour(juce::Label::textColourId, juce::Colour(0xfff8fafc));
            lbl.setJustificationType(juce::Justification::centred);
            addAndMakeVisible(lbl);

            sub808UI_.attachments[k] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                apvts, sub808ParamNames[k], knob
            );
        }
    }

    // --------------------------------------------------------------------------
    // 6. Setup Master Studio Strip Controls (Bottom Bar)
    // --------------------------------------------------------------------------
    auto setupMasterSlider = [this](juce::Slider& s, juce::Label& l, const juce::String& text, juce::Colour col) {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        s.setColour(juce::Slider::rotarySliderFillColourId, col);
        s.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff334155));
        s.setColour(juce::Slider::thumbColourId, juce::Colour(0xffffffff));
        addAndMakeVisible(s);

        l.setText(text, juce::dontSendNotification);
        l.setFont(juce::Font(13.0f, juce::Font::bold));
        l.setColour(juce::Label::textColourId, col);
        l.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(l);
    };

    setupMasterSlider(tomOctSlider_, tomOctLabel_, "TOM OCT", juce::Colour(0xffa855f7));
    setupMasterSlider(macroVarSlider_, macroVarLabel_, "VARIATION", juce::Colour(0xff06b6d4));
    setupMasterSlider(masterLowBoostSlider_, masterLowBoostLabel_, "LOW THUMP", juce::Colour(0xff10b981));
    setupMasterSlider(masterDriveSlider_, masterDriveLabel_, "DRIVE", juce::Colour(0xffef4444));
    setupMasterSlider(masterVolumeSlider_, masterVolumeLabel_, "VOLUME", juce::Colour(0xff38bdf8));

    tomOctAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "tom_octave", tomOctSlider_);
    macroVarAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "macro_variation", macroVarSlider_);
    masterLowBoostAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "master_low_boost", masterLowBoostSlider_);
    masterDriveAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "master_drive", masterDriveSlider_);
    masterVolAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "master_volume", masterVolumeSlider_);

    // --------------------------------------------------------------------------
    // 7. Quantize ComboBox & Timer
    // --------------------------------------------------------------------------
    quantizeCombo_.addItem("Loop Wrap Sync", 1);
    quantizeCombo_.addItem("Bar Sync", 2);
    quantizeCombo_.addItem("Immediate Swap", 3);
    quantizeCombo_.setSelectedId(processor_.getQuantizeMode() + 1, juce::dontSendNotification);
    quantizeCombo_.onChange = [this]() {
        processor_.setQuantizeMode(quantizeCombo_.getSelectedId() - 1);
    };
    quantizeCombo_.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff161b22));
    quantizeCombo_.setColour(juce::ComboBox::textColourId, juce::Colour(0xfff8fafc));
    quantizeCombo_.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff475569));
    addAndMakeVisible(quantizeCombo_);

    startTimerHz(30);
}

CrucibleWorkbenchEditor::~CrucibleWorkbenchEditor() {
    stopTimer();
    for (int k = 0; k < 16; ++k) {
        kickUI_.attachments[k].reset();
        snareClapUI_.attachments[k].reset();
        hatUI_.attachments[k].reset();
    }
    for (int k = 0; k < 8; ++k) {
        tomUI_.attachments[k].reset();
        westZapUI_.attachments[k].reset();
        crashUI_.attachments[k].reset();
        sub808UI_.attachments[k].reset();
    }
}

void CrucibleWorkbenchEditor::updateEngineLabels(int slot) {
    if (slot == 0) {
        kickUI_.modelReadoutLabel.setText(processor_.getMmeKickModelName(), juce::dontSendNotification);
        return;
    }

    SlotUI* targetUI = nullptr;
    if (slot == 2) targetUI = &tomUI_;
    else if (slot == 3) targetUI = &westZapUI_;
    else if (slot == 7) targetUI = &crashUI_;
    if (!targetUI) return;

    auto engine = processor_.getSlotEngine(slot);
    juce::Colour col;
    if (static_cast<int>(engine) < 4) {
        col = juce::Colour(0xfff59e0b); // Amber for Formless Synth
    } else if (static_cast<int>(engine) < 8) {
        col = juce::Colour(0xff38bdf8); // Cyan for Formless Transient
    } else {
        col = juce::Colour(0xff10b981); // Emerald Acid Green for DigiStrike
    }

    targetUI->titleLabel.setText("S" + juce::String(slot + 1) + ": " + getSlotEngineName(engine), juce::dontSendNotification);
    targetUI->titleLabel.setColour(juce::Label::textColourId, col);

    static const char* defaultLabels[8] = { "PITCH", "DECAY", "MOD SPD", "MOD DPT", "FOLD", "CUTOFF", "RESO", "DRIVE" };
    static const char* sdLabels[8]      = { "TONE", "DECAY", "NOISE", "SNAP", "WIRE", "CUTOFF", "RESO", "DRIVE" };
    static const char* prcLabels[8]     = { "FM PITCH", "DECAY", "FM MOD", "PING", "BELL", "CUTOFF", "RESO", "METAL" };

    const char** activeSet = defaultLabels;
    if (engine == SlotEngineType::FORMLESS_SNARE || engine == SlotEngineType::DIGISTRIKE_SD) activeSet = sdLabels;
    else if (engine == SlotEngineType::FORMLESS_ZAP || engine == SlotEngineType::DIGISTRIKE_PRC) activeSet = prcLabels;

    for (int k = 0; k < 8; ++k) {
        targetUI->knobs[k].setColour(juce::Slider::rotarySliderFillColourId, col);
        targetUI->knobLabels[k].setText(activeSet[k], juce::dontSendNotification);
        targetUI->knobLabels[k].setColour(juce::Label::textColourId, juce::Colour(0xfff8fafc));
    }
}

void CrucibleWorkbenchEditor::paint(juce::Graphics& g) {
    // Deep Tsathogguan Basalt Obsidian Background
    g.fillAll(juce::Colour(0xff0d1015));

    // Top Title Bar
    g.setColour(juce::Colour(0xff161b22));
    g.fillRect(0, 0, getWidth(), 48);

    g.setColour(juce::Colour(0xff10b981)); // Emerald accent
    g.setFont(juce::Font(20.0f, juce::Font::bold));
    g.drawText("CRUCIBLE WORKBENCH", 18, 0, 270, 48, juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xffcbd5e1));
    g.setFont(juce::Font(13.5f, juce::Font::bold));
    g.drawText("POLYMORPHIC FLAGSHIP // 3 TALL WORKSTATIONS + 808 SUB // UDP :9123", 295, 2, 600, 48, juce::Justification::centredLeft);

    // Swap Pending Indicator
    if (processor_.isSwapPending()) {
        g.setColour(juce::Colour(0xfff59e0b));
        g.setFont(juce::Font(13.5f, juce::Font::bold));
        g.drawText("HOT-SWAP PENDING...", 960, 0, 400, 48, juce::Justification::centredRight);
    } else if (flashFrames_ > 0) {
        g.setColour(juce::Colour(0xff10b981));
        g.setFont(juce::Font(13.5f, juce::Font::bold));
        g.drawText("SWAP COMMITTED!", 960, 0, 400, 48, juce::Justification::centredRight);
    }

    // Dividers
    g.setColour(juce::Colour(0xff21262d));
    g.drawHorizontalLine(48, 0.0f, static_cast<float>(getWidth()));
    g.drawHorizontalLine(508, 0.0f, static_cast<float>(getWidth()));

    // --------------------------------------------------------------------------
    // Draw 3 Tall Card Backgrounds (Cols 0, 1, 2)
    // --------------------------------------------------------------------------
    const int startX = 16;
    const int cardW = 260;
    const int gutter = 12;
    const int cardY = 54;
    const int tallCardH = 450;

    auto drawTallCardFrame = [&g](int x, int y, int w, int h, juce::Colour col, const char* bank0, const char* bank1) {
        g.setColour(juce::Colour(0xff161b22));
        g.fillRoundedRectangle(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h), 6.0f);

        g.setColour(col.withAlpha(0.60f));
        g.drawRoundedRectangle(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h), 6.0f, 1.4f);

        g.setColour(col.withAlpha(0.20f));
        g.fillRoundedRectangle(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), 30.0f, 6.0f);

        g.setFont(juce::Font(13.0f, juce::Font::bold));
        g.setColour(col);
        g.drawText(bank0, x + 8, y + 32, w - 16, 16, juce::Justification::centredLeft);

        g.setColour(juce::Colour(0xff21262d));
        g.drawHorizontalLine(y + 196, static_cast<float>(x + 4), static_cast<float>(x + w - 4));

        g.setColour(col);
        g.drawText(bank1, x + 8, y + 200, w - 16, 16, juce::Justification::centredLeft);
    };

    // Col 0: MME Kick
    drawTallCardFrame(startX, cardY, cardW, tallCardH, juce::Colour(0xfff59e0b), "BANK 0: JAM SYNTHESIS", "BANK 1: MOJO / WAVEFOLD");
    {
        // Bottom Telemetry Info Box
        g.setColour(juce::Colour(0xff0d1015));
        g.fillRoundedRectangle(static_cast<float>(startX + 8), static_cast<float>(cardY + 368), static_cast<float>(cardW - 16), 74.0f, 4.0f);
        g.setColour(juce::Colour(0xff334155));
        g.drawRoundedRectangle(static_cast<float>(startX + 8), static_cast<float>(cardY + 368), static_cast<float>(cardW - 16), 74.0f, 4.0f, 1.0f);

        g.setFont(juce::Font(13.0f, juce::Font::bold));
        g.setColour(juce::Colour(0xfff59e0b));
        g.drawText("MME BASS DRUM ENGINE", startX + 14, cardY + 372, cardW - 28, 18, juce::Justification::centredLeft);

        g.setFont(juce::Font(13.0f, juce::Font::plain));
        g.setColour(juce::Colour(0xffcbd5e1));
        g.drawText("16 Analog & Digital BD Models", startX + 14, cardY + 394, cardW - 28, 18, juce::Justification::centredLeft);

        g.setFont(juce::Font(13.0f, juce::Font::bold));
        g.setColour(juce::Colour(0xff10b981));
        g.drawText("Knob 16: Diode Wavefold", startX + 14, cardY + 416, cardW - 28, 18, juce::Justification::centredLeft);
    }

    // Col 1: Snare & Clap Workstation
    const int col1X = startX + cardW + gutter;
    drawTallCardFrame(col1X, cardY, cardW, tallCardH, juce::Colour(0xff38bdf8), "BANK 0: MME SNARE (16 MODELS)", "BANK 1: MME CLAP & MOJO");
    {
        // Bottom Telemetry Info Box
        g.setColour(juce::Colour(0xff0d1015));
        g.fillRoundedRectangle(static_cast<float>(col1X + 8), static_cast<float>(cardY + 368), static_cast<float>(cardW - 16), 74.0f, 4.0f);
        g.setColour(juce::Colour(0xff334155));
        g.drawRoundedRectangle(static_cast<float>(col1X + 8), static_cast<float>(cardY + 368), static_cast<float>(cardW - 16), 74.0f, 4.0f, 1.0f);

        g.setFont(juce::Font(13.0f, juce::Font::bold));
        g.setColour(juce::Colour(0xff38bdf8));
        g.drawText("4 MACRO NOTES: 61..64", col1X + 14, cardY + 372, cardW - 28, 18, juce::Justification::centredLeft);

        g.setFont(juce::Font(13.0f, juce::Font::plain));
        g.setColour(juce::Colour(0xffcbd5e1));
        g.drawText("61:Lil CP | 62:Lil SD | 63:Big CP | 64:Big SD", col1X + 14, cardY + 394, cardW - 28, 18, juce::Justification::centredLeft);

        g.setFont(juce::Font(13.0f, juce::Font::bold));
        g.setColour(juce::Colour(0xff10b981));
        g.drawText("Dual Variation + Analog Mojo Strip", col1X + 14, cardY + 416, cardW - 28, 18, juce::Justification::centredLeft);
    }

    // Col 2: Hi-Hat Workstation
    const int col2X = col1X + cardW + gutter;
    drawTallCardFrame(col2X, cardY, cardW, tallCardH, juce::Colour(0xff06b6d4), "BANK 0: HAT CORE SYNTHESIS", "BANK 1: ELEKTRON MOJO STRIP");
    {
        // Bottom Telemetry Info Box
        g.setColour(juce::Colour(0xff0d1015));
        g.fillRoundedRectangle(static_cast<float>(col2X + 8), static_cast<float>(cardY + 368), static_cast<float>(cardW - 16), 74.0f, 4.0f);
        g.setColour(juce::Colour(0xff334155));
        g.drawRoundedRectangle(static_cast<float>(col2X + 8), static_cast<float>(cardY + 368), static_cast<float>(cardW - 16), 74.0f, 4.0f, 1.0f);

        g.setFont(juce::Font(13.0f, juce::Font::bold));
        g.setColour(juce::Colour(0xff06b6d4));
        g.drawText("7 ARTICULATIONS: 65..71", col2X + 14, cardY + 372, cardW - 28, 18, juce::Justification::centredLeft);

        g.setFont(juce::Font(13.0f, juce::Font::plain));
        g.setColour(juce::Colour(0xffcbd5e1));
        g.drawText("Tight->Norm->Loose->Semi->Open->Sizzle->Choke", col2X + 14, cardY + 394, cardW - 28, 18, juce::Justification::centredLeft);

        g.setFont(juce::Font(13.0f, juce::Font::bold));
        g.setColour(juce::Colour(0xff10b981));
        g.drawText("Mutual Choking + 6-Osc Cluster", col2X + 14, cardY + 416, cardW - 28, 18, juce::Justification::centredLeft);
    }

    // --------------------------------------------------------------------------
    // Draw 4 Stacked Card Backgrounds (Cols 3 & 4)
    // --------------------------------------------------------------------------
    const int col3X = col2X + cardW + gutter;
    const int col4X = col3X + cardW + gutter;
    const int stackH = 220;
    const int stackY1 = 54;
    const int stackY2 = 284;

    auto drawStackedFrame = [&g](int x, int y, int w, int h, juce::Colour col) {
        g.setColour(juce::Colour(0xff161b22));
        g.fillRoundedRectangle(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h), 6.0f);

        g.setColour(col.withAlpha(0.50f));
        g.drawRoundedRectangle(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h), 6.0f, 1.2f);

        g.setColour(col.withAlpha(0.18f));
        g.fillRoundedRectangle(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), 30.0f, 6.0f);
    };

    // Col 3 Top: Toms / Perc
    drawStackedFrame(col3X, stackY1, cardW, stackH, juce::Colour(0xffa855f7));
    // Col 3 Bottom: West Coast / Zap
    drawStackedFrame(col3X, stackY2, cardW, stackH, juce::Colour(0xff10b981));

    // Col 4 Top: Crash / Cymbal
    drawStackedFrame(col4X, stackY1, cardW, stackH, juce::Colour(0xff38bdf8));
    // Col 4 Bottom: 808 Sub-Bass
    drawStackedFrame(col4X, stackY2, cardW, stackH, juce::Colour(0xfff59e0b));

    // --------------------------------------------------------------------------
    // Bottom Console Bar: Zone Labels, Transport & Framed Master Studio Box
    // --------------------------------------------------------------------------
    g.setColour(juce::Colour(0xff94a3b8));
    g.setFont(juce::Font(13.5f, juce::Font::bold));
    g.drawText("SYNC:", 18, 526, 50, 28, juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xffcbd5e1));
    g.setFont(juce::Font(13.5f, juce::Font::bold));
    g.drawText("Z1: 808 SUB (0..59)  |  Z2: 1-SHOT KIT (60..83)  |  Z3: TOMS (84..127)", 230, 518, 480, 20, juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xff10b981));
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    g.drawText("UDP: 127.0.0.1:9123  |  CRUCIBLE WORKBENCH FLAGSHIP v2.0", 230, 544, 480, 18, juce::Justification::centredLeft);

    // Framed Master Studio Box on bottom right
    {
        g.setColour(juce::Colour(0xff161b22));
        g.fillRoundedRectangle(726.0f, 512.0f, 638.0f, 96.0f, 4.0f);
        g.setColour(juce::Colour(0xff334155));
        g.drawRoundedRectangle(726.0f, 512.0f, 638.0f, 96.0f, 4.0f, 1.0f);

        g.setFont(juce::Font(13.0f, juce::Font::bold));
        g.setColour(juce::Colour(0xff38bdf8));
        g.drawText("STUDIO MASTER & MACROS", 736, 514, 200, 16, juce::Justification::centredLeft);
    }
}

void CrucibleWorkbenchEditor::resized() {
    // Quantize combo
    quantizeCombo_.setBounds(72, 526, 145, 28);

    const int startX = 16;
    const int cardW = 260;
    const int gutter = 12;
    const int cardY = 54;
    const int kColW = 62;
    const int kSize = 42;

    // --------------------------------------------------------------------------
    // 1. Position Col 0: MmeKick 16-Control Tall Card
    // --------------------------------------------------------------------------
    {
        const int x = startX;
        kickUI_.titleLabel.setBounds(x + 6, cardY + 3, 105, 24);
        kickUI_.modelReadoutLabel.setBounds(x + 112, cardY + 3, 98, 24);
        kickUI_.trigButton.setBounds(x + 212, cardY + 4, 42, 22);

        const int xOff = x + 6;
        const int rowY[4] = { cardY + 50, cardY + 118, cardY + 218, cardY + 286 };

        for (int k = 0; k < 16; ++k) {
            const int col = k % 4;
            const int row = k / 4;
            const int kX = xOff + col * kColW + (kColW - kSize) / 2;
            const int kY = rowY[row];

            kickUI_.knobs[k].setBounds(kX, kY, kSize, kSize);
            kickUI_.knobLabels[k].setBounds(xOff + col * kColW, kY + kSize + 2, kColW, 18);
        }
    }

    // --------------------------------------------------------------------------
    // 2. Position Col 1: Snare & Clap Flagship Workstation
    // --------------------------------------------------------------------------
    {
        const int x = startX + cardW + gutter;
        snareClapUI_.titleLabel.setBounds(x + 6, cardY + 3, 100, 24);

        // 4 Trig Buttons across the top header
        snareClapUI_.lilSdButton.setBounds(x + 108, cardY + 4, 34, 22);
        snareClapUI_.bigSdButton.setBounds(x + 144, cardY + 4, 34, 22);
        snareClapUI_.lilCpButton.setBounds(x + 180, cardY + 4, 34, 22);
        snareClapUI_.bigCpButton.setBounds(x + 216, cardY + 4, 38, 22);

        // Model readouts inside the bank header dividers
        snareClapUI_.sdModelLabel.setBounds(x + 150, cardY + 30, 104, 18);
        snareClapUI_.cpModelLabel.setBounds(x + 150, cardY + 198, 104, 18);

        const int xOff = x + 6;
        const int rowY[4] = { cardY + 50, cardY + 118, cardY + 218, cardY + 286 };

        for (int k = 0; k < 16; ++k) {
            const int col = k % 4;
            const int row = k / 4;
            const int kX = xOff + col * kColW + (kColW - kSize) / 2;
            const int kY = rowY[row];

            snareClapUI_.knobs[k].setBounds(kX, kY, kSize, kSize);
            snareClapUI_.knobLabels[k].setBounds(xOff + col * kColW, kY + kSize + 2, kColW, 18);
        }
    }

    // --------------------------------------------------------------------------
    // 3. Position Col 2: Hi-Hat Flagship Workstation
    // --------------------------------------------------------------------------
    {
        const int x = startX + 2 * (cardW + gutter);
        hatUI_.titleLabel.setBounds(x + 6, cardY + 3, 140, 24);
        hatUI_.chTrigButton.setBounds(x + 168, cardY + 4, 40, 22);
        hatUI_.ohTrigButton.setBounds(x + 212, cardY + 4, 42, 22);

        hatUI_.infoLabel.setBounds(x + 130, cardY + 30, 124, 18);

        const int xOff = x + 6;
        const int rowY[4] = { cardY + 50, cardY + 118, cardY + 218, cardY + 286 };

        for (int k = 0; k < 16; ++k) {
            const int col = k % 4;
            const int row = k / 4;
            const int kX = xOff + col * kColW + (kColW - kSize) / 2;
            const int kY = rowY[row];

            hatUI_.knobs[k].setBounds(kX, kY, kSize, kSize);
            hatUI_.knobLabels[k].setBounds(xOff + col * kColW, kY + kSize + 2, kColW, 18);
        }
    }

    // --------------------------------------------------------------------------
    // 4. Position Col 3: Stacked Slots (Toms Top, West/Zap Bottom)
    // --------------------------------------------------------------------------
    auto layoutStandardSlot = [&](SlotUI& ui, int x, int y) {
        ui.titleLabel.setBounds(x + 8, y + 3, 180, 24);
        ui.trigButton.setBounds(x + 198, y + 4, 52, 22);

        const int xOff = x + 6;
        for (int k = 0; k < 8; ++k) {
            const int col = k % 4;
            const int row = k / 4;
            const int kX = xOff + col * kColW + (kColW - kSize) / 2;
            const int kY = y + 44 + row * 82;

            ui.knobs[k].setBounds(kX, kY, kSize, kSize);
            ui.knobLabels[k].setBounds(xOff + col * kColW, kY + kSize + 2, kColW, 18);
        }
    };

    const int col3X = startX + 3 * (cardW + gutter);
    layoutStandardSlot(tomUI_, col3X, 54);
    layoutStandardSlot(westZapUI_, col3X, 284);

    // --------------------------------------------------------------------------
    // 5. Position Col 4: Stacked Slots (Crash Top, 808 Sub-Bass Bottom)
    // --------------------------------------------------------------------------
    const int col4X = startX + 4 * (cardW + gutter);
    layoutStandardSlot(crashUI_, col4X, 54);

    {
        const int y = 284;
        sub808UI_.titleLabel.setBounds(col4X + 8, y + 3, 90, 24);
        sub808UI_.modelReadoutLabel.setBounds(col4X + 100, y + 3, 98, 24);
        sub808UI_.trigButton.setBounds(col4X + 204, y + 4, 46, 22);

        const int xOff = col4X + 6;
        for (int k = 0; k < 8; ++k) {
            const int col = k % 4;
            const int row = k / 4;
            const int kX = xOff + col * kColW + (kColW - kSize) / 2;
            const int kY = y + 44 + row * 82;

            sub808UI_.knobs[k].setBounds(kX, kY, kSize, kSize);
            sub808UI_.knobLabels[k].setBounds(xOff + col * kColW, kY + kSize + 2, kColW, 18);
        }
    }

    // --------------------------------------------------------------------------
    // 6. Position Master Studio Controls in Bottom Strip
    // --------------------------------------------------------------------------
    {
        const int masterStartX = 736;
        const int colW = 124;
        const int kY = 532;
        const int mSize = 44;

        auto placeMasterKnob = [&](juce::Slider& s, juce::Label& l, int idx) {
            const int kX = masterStartX + idx * colW + (colW - mSize) / 2;
            s.setBounds(kX, kY, mSize, mSize);
            l.setBounds(masterStartX + idx * colW, kY + mSize + 2, colW, 18);
        };

        placeMasterKnob(tomOctSlider_, tomOctLabel_, 0);
        placeMasterKnob(macroVarSlider_, macroVarLabel_, 1);
        placeMasterKnob(masterLowBoostSlider_, masterLowBoostLabel_, 2);
        placeMasterKnob(masterDriveSlider_, masterDriveLabel_, 3);
        placeMasterKnob(masterVolumeSlider_, masterVolumeLabel_, 4);
    }
}

void CrucibleWorkbenchEditor::timerCallback() {
    // Keep model readouts up to date
    juce::String currentKickModel = processor_.getMmeKickModelName();
    if (kickUI_.modelReadoutLabel.getText() != currentKickModel) {
        kickUI_.modelReadoutLabel.setText(currentKickModel, juce::dontSendNotification);
    }

    juce::String currentSnareModel = processor_.getMmeSnareModelName();
    if (snareClapUI_.sdModelLabel.getText() != currentSnareModel) {
        snareClapUI_.sdModelLabel.setText(currentSnareModel, juce::dontSendNotification);
    }

    juce::String currentClapModel = processor_.getMmeClapModelName();
    if (snareClapUI_.cpModelLabel.getText() != currentClapModel) {
        snareClapUI_.cpModelLabel.setText(currentClapModel, juce::dontSendNotification);
    }

    juce::String currentSubModel = processor_.getSub808ModelName();
    if (sub808UI_.modelReadoutLabel.getText() != currentSubModel) {
        sub808UI_.modelReadoutLabel.setText(currentSubModel, juce::dontSendNotification);
    }

    if (processor_.lastSwapCommitted()) {
        processor_.clearCommitFlag();
        updateEngineLabels(2);
        updateEngineLabels(3);
        updateEngineLabels(7);
        flashFrames_ = 10;
        repaint();
    } else if (flashFrames_ > 0) {
        --flashFrames_;
        if (flashFrames_ == 0) {
            repaint();
        }
    }

    if (processor_.isSwapPending()) {
        repaint(950, 0, 420, 48);
    }
}
