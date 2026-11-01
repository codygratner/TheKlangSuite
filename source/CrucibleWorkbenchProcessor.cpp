#include "CrucibleWorkbenchProcessor.h"
#include "CrucibleWorkbenchEditor.h"

CrucibleWorkbenchAudioProcessor::CrucibleWorkbenchAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_(*this, &undoManager_, "CRUCIBLE_WORKBENCH", createParameterLayout())
{
    // 1. Cache raw parameter pointers for lock-free audio thread reads
    for (int s = 0; s < kNumSlots; ++s) {
        for (int p = 0; p < kParamsPerSlot; ++p) {
            juce::String id = "s" + juce::String(s) + "_p" + juce::String(p);
            cachedParams_[s][p] = apvts_.getRawParameterValue(id);
        }
    }
    masterVolumeParam_ = apvts_.getRawParameterValue("master_volume");
    masterDriveParam_ = apvts_.getRawParameterValue("master_drive");
    masterLowBoostParam_ = apvts_.getRawParameterValue("master_low_boost");

    subOctParam_ = apvts_.getRawParameterValue("sub_octave");
    tomOctParam_ = apvts_.getRawParameterValue("tom_octave");
    macroVarParam_ = apvts_.getRawParameterValue("macro_variation");

    sub808ModelParam_  = apvts_.getRawParameterValue("sub808_model");
    sub808TuneParam_   = apvts_.getRawParameterValue("sub808_tune");
    sub808DecayParam_  = apvts_.getRawParameterValue("sub808_decay");
    sub808PunchParam_  = apvts_.getRawParameterValue("sub808_punch");
    sub808SweepParam_  = apvts_.getRawParameterValue("sub808_sweep");
    sub808FoldParam_   = apvts_.getRawParameterValue("sub808_fold");
    sub808DriveParam_  = apvts_.getRawParameterValue("sub808_drive");
    sub808CutoffParam_ = apvts_.getRawParameterValue("sub808_cutoff");
    sub808GlideParam_  = apvts_.getRawParameterValue("sub808_glide");

    // Snare & Clap Flagship Parameters (Col 1)
    scSdModelParam_    = apvts_.getRawParameterValue("sc_sd_model");
    scSdTuneParam_     = apvts_.getRawParameterValue("sc_sd_tune");
    scSdDecayParam_    = apvts_.getRawParameterValue("sc_sd_decay");
    scSdSnapParam_     = apvts_.getRawParameterValue("sc_sd_snap");
    scSdToneParam_     = apvts_.getRawParameterValue("sc_sd_tone");
    scSdWiresParam_    = apvts_.getRawParameterValue("sc_sd_wires");
    scSdBodyParam_     = apvts_.getRawParameterValue("sc_sd_body");
    scSdVarParam_      = apvts_.getRawParameterValue("sc_sd_var");

    scCpModelParam_    = apvts_.getRawParameterValue("sc_cp_model");
    scCpToneParam_     = apvts_.getRawParameterValue("sc_cp_tone");
    scCpDecayParam_    = apvts_.getRawParameterValue("sc_cp_decay");
    scCpBurstParam_    = apvts_.getRawParameterValue("sc_cp_burst");
    scCpSpreadParam_   = apvts_.getRawParameterValue("sc_cp_spread");
    scCpVarParam_      = apvts_.getRawParameterValue("sc_cp_var");
    scMojoFltfParam_   = apvts_.getRawParameterValue("sc_mojo_fltf");
    scMojoDistParam_   = apvts_.getRawParameterValue("sc_mojo_dist");

    // Hi-Hat Flagship Parameters (Col 2)
    static const char* kHatParamIDs[16] = {
        "hat_pitch", "hat_cl_decay", "hat_op_decay", "hat_metal",
        "hat_choke", "hat_bright", "hat_pedal", "hat_accent",
        "hat_fltf", "hat_fltw", "hat_fltq", "hat_eqf",
        "hat_eqg", "hat_srr", "hat_brr", "hat_drive"
    };
    for (int p = 0; p < 16; ++p) {
        hatParams_[p] = apvts_.getRawParameterValue(kHatParamIDs[p]);
    }

    // 2. Default Initial Slot Engine Assignments
    activeEngines_[0] = SlotEngineType::MME_KICK;
    activeEngines_[1] = SlotEngineType::FORMLESS_SNARE;
    activeEngines_[2] = SlotEngineType::FORMLESS_ZAP;
    activeEngines_[3] = SlotEngineType::FORMLESS_WEST;
    activeEngines_[4] = SlotEngineType::FORMLESS_OPEN_HAT;
    activeEngines_[5] = SlotEngineType::FORMLESS_CLOSED_HAT;
    activeEngines_[6] = SlotEngineType::FORMLESS_CLAP;
    activeEngines_[7] = SlotEngineType::FORMLESS_CRASH;

    stagedEngines_ = activeEngines_;
    ipcThread_ = nullptr;
}

CrucibleWorkbenchAudioProcessor::~CrucibleWorkbenchAudioProcessor() {
    if (ipcThread_ != nullptr) {
        ipcThread_->signalThreadShouldExit();
        ipcThread_->stopThread(500);
        ipcThread_.reset();
    }
}

void CrucibleWorkbenchAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/) {
    sampleRate_ = (sampleRate > 1000.0) ? sampleRate : 44100.0;
    mmeKick_.prepare(sampleRate_);
    sub808Engine_.prepare(sampleRate_);
    sub808Engine_.setParam(0, 7.0f / 15.0f); // Lock to Model 07 TR-808-BD
    sub808Engine_.setParam(9, 1.0f);          // Filter width open
    mmeSnare_.prepare(sampleRate_);
    mmeClap_.prepare(sampleRate_);
    scMojo_.prepare(static_cast<float>(sampleRate_));
    hiHatEngine_.prepare(sampleRate_);
    formlessEngine_.prepare(static_cast<float>(sampleRate_));

    for (int i = 0; i < kNumSlots; ++i) {
        slotStates_[i].active = false;
        slotStates_[i].isSustained = false;
        slotStates_[i].env = 0.0f;
        slotStates_[i].releaseCoeff = 0.0f;
        slotStates_[i].stereoSpread = 0.5f;
        slotStates_[i].burstDuration = 0.035f;
        slotStates_[i].noiseLfsr = 0x13579BDFu + i * 0x10001u;
    }
    heldSubNote_ = -1;
    currentKickPitchHz_ = 55.0f;
    masterLowBoostState_ = 0.0f;

    if (ipcThread_ == nullptr) {
        ipcThread_ = std::make_unique<IPCThread>(*this, udpPort_);
        ipcThread_->startThread();
    }
}

void CrucibleWorkbenchAudioProcessor::releaseResources() {
    if (ipcThread_ != nullptr) {
        ipcThread_->signalThreadShouldExit();
        ipcThread_->stopThread(500);
        ipcThread_.reset();
    }
    mmeKick_.reset();
    sub808Engine_.reset();
    mmeSnare_.reset();
    mmeClap_.reset();
    scMojo_.reset();
    hiHatEngine_.reset();
    formlessEngine_.reset();
}

bool CrucibleWorkbenchAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void CrucibleWorkbenchAudioProcessor::commitStagedSlots() noexcept {
    activeEngines_ = stagedEngines_;
    swapPending_.store(false, std::memory_order_release);
    lastSwapCommitted_.store(true, std::memory_order_release);
}

// ==============================================================================
// ROLE LOOKUPS & MULTI-ZONE DISPATCHER
// ==============================================================================

int CrucibleWorkbenchAudioProcessor::findSlotForRole(SlotEngineType primary, SlotEngineType secondary, int fallback) const noexcept {
    for (int i = 0; i < kNumSlots; ++i) {
        if (activeEngines_[i] == primary || activeEngines_[i] == secondary) return i;
    }
    return fallback;
}

int CrucibleWorkbenchAudioProcessor::findKickSlot() const noexcept {
    return 0; // Slot 0 is dedicated MmeKick Flagship
}

int CrucibleWorkbenchAudioProcessor::findSnareSlot() const noexcept {
    return findSlotForRole(SlotEngineType::FORMLESS_SNARE, SlotEngineType::DIGISTRIKE_SD, 1);
}

int CrucibleWorkbenchAudioProcessor::findClapSlot() const noexcept {
    return findSlotForRole(SlotEngineType::FORMLESS_CLAP, SlotEngineType::DIGISTRIKE_CLP, 6);
}

int CrucibleWorkbenchAudioProcessor::findHatSlot() const noexcept {
    for (int i = 0; i < kNumSlots; ++i) {
        if (activeEngines_[i] == SlotEngineType::FORMLESS_CLOSED_HAT ||
            activeEngines_[i] == SlotEngineType::FORMLESS_OPEN_HAT ||
            activeEngines_[i] == SlotEngineType::DIGISTRIKE_HH) return i;
    }
    return 5;
}

int CrucibleWorkbenchAudioProcessor::findCymbalSlot() const noexcept {
    return findSlotForRole(SlotEngineType::FORMLESS_CRASH, SlotEngineType::DIGISTRIKE_CYM, 7);
}

int CrucibleWorkbenchAudioProcessor::findTomSlot() const noexcept {
    for (int i = 0; i < kNumSlots; ++i) {
        if (activeEngines_[i] == SlotEngineType::DIGISTRIKE_TOM ||
            activeEngines_[i] == SlotEngineType::FORMLESS_WEST ||
            activeEngines_[i] == SlotEngineType::FORMLESS_ZAP) return i;
    }
    return 2;
}

void CrucibleWorkbenchAudioProcessor::triggerSub808(float velocity) noexcept {
    velocity = std::clamp(velocity, 0.05f, 1.0f);
    sub808CurrentPitchHz_ = 55.0f;
    sub808TargetPitchHz_ = 55.0f;
    sub808Engine_.setSustain(false);
    sub808Engine_.trigger(velocity);
}

void CrucibleWorkbenchAudioProcessor::triggerSnare(bool isBig, float velocity) noexcept {
    velocity = std::clamp(velocity, 0.05f, 1.0f);
    const float varDepth = scSdVarParam_ ? scSdVarParam_->load(std::memory_order_relaxed) : 0.5f;

    if (scSdModelParam_) mmeSnare_.setParam(0, scSdModelParam_->load(std::memory_order_relaxed));
    if (scSdToneParam_)  mmeSnare_.setParam(4, scSdToneParam_->load(std::memory_order_relaxed));
    if (scSdBodyParam_)  mmeSnare_.setParam(7, scSdBodyParam_->load(std::memory_order_relaxed));

    const float basePitch = scSdTuneParam_ ? scSdTuneParam_->load(std::memory_order_relaxed) : 0.40f;
    const float baseDecay = scSdDecayParam_ ? scSdDecayParam_->load(std::memory_order_relaxed) : 0.45f;
    const float baseSnap  = scSdSnapParam_ ? scSdSnapParam_->load(std::memory_order_relaxed) : 0.55f;
    const float baseWires = scSdWiresParam_ ? scSdWiresParam_->load(std::memory_order_relaxed) : 0.40f;

    if (!isBig) {
        // Little Snare: higher shell pitch, crisper snap, shorter decay
        mmeSnare_.setParam(1, std::clamp(basePitch + 0.15f * varDepth, 0.0f, 1.0f));
        mmeSnare_.setParam(2, std::clamp(baseDecay * (1.0f - 0.40f * varDepth), 0.05f, 1.0f));
        mmeSnare_.setParam(3, std::clamp(baseSnap + 0.20f * varDepth, 0.0f, 1.0f));
        mmeSnare_.setParam(6, std::clamp(baseWires * (1.0f - 0.25f * varDepth), 0.05f, 1.0f));
    } else {
        // Big Snare: lower pitch, heavy shell rattle, longer decay
        mmeSnare_.setParam(1, std::clamp(basePitch - 0.15f * varDepth, 0.0f, 1.0f));
        mmeSnare_.setParam(2, std::clamp(baseDecay * (1.0f + 0.50f * varDepth), 0.05f, 1.0f));
        mmeSnare_.setParam(3, baseSnap);
        mmeSnare_.setParam(6, std::clamp(baseWires + 0.30f * varDepth, 0.0f, 1.0f));
    }

    mmeSnare_.trigger(velocity);
}

void CrucibleWorkbenchAudioProcessor::triggerClap(bool isBig, float velocity) noexcept {
    velocity = std::clamp(velocity, 0.05f, 1.0f);
    const float varDepth = scCpVarParam_ ? scCpVarParam_->load(std::memory_order_relaxed) : 0.5f;

    if (scCpModelParam_) mmeClap_.setParam(0, scCpModelParam_->load(std::memory_order_relaxed));
    if (scCpToneParam_)  mmeClap_.setParam(1, scCpToneParam_->load(std::memory_order_relaxed));

    const float baseDecay  = scCpDecayParam_ ? scCpDecayParam_->load(std::memory_order_relaxed) : 0.45f;
    const float baseBurst  = scCpBurstParam_ ? scCpBurstParam_->load(std::memory_order_relaxed) : 0.60f;
    const float baseSpread = scCpSpreadParam_ ? scCpSpreadParam_->load(std::memory_order_relaxed) : 0.40f;

    if (!isBig) {
        // Little Clap: tight single/dual snap, narrow spread, tight tail
        mmeClap_.setParam(2, std::clamp(baseDecay * (1.0f - 0.45f * varDepth), 0.05f, 1.0f));
        mmeClap_.setParam(3, std::clamp(baseBurst * (1.0f - 0.50f * varDepth), 0.05f, 1.0f));
        mmeClap_.setParam(4, std::clamp(baseSpread * (1.0f - 0.40f * varDepth), 0.05f, 1.0f));
    } else {
        // Big Clap: multi-burst flam, wide stereo spread, long tail
        mmeClap_.setParam(2, std::clamp(baseDecay * (1.0f + 0.60f * varDepth), 0.05f, 1.0f));
        mmeClap_.setParam(3, std::clamp(baseBurst + 0.30f * varDepth, 0.0f, 1.0f));
        mmeClap_.setParam(4, std::clamp(baseSpread + 0.35f * varDepth, 0.0f, 1.0f));
    }

    mmeClap_.trigger(velocity);
}

void CrucibleWorkbenchAudioProcessor::triggerHat(bool isOpen, float velocity) noexcept {
    velocity = std::clamp(velocity, 0.05f, 1.0f);
    if (isOpen) {
        hiHatEngine_.triggerOpen(velocity);
    } else {
        hiHatEngine_.triggerClosed(velocity);
    }
}

void CrucibleWorkbenchAudioProcessor::triggerSlot(int slotIndex, float velocity) noexcept {
    if (slotIndex < 0 || slotIndex >= kNumSlots) return;
    velocity = std::clamp(velocity, 0.05f, 1.0f);

    if (slotIndex == 0) {
        currentKickPitchHz_ = 55.0f;
        mmeKick_.setSustain(false);
        mmeKick_.trigger(velocity);
        return;
    }

    auto engine = activeEngines_[slotIndex];
    auto& s = slotStates_[slotIndex];

    if (static_cast<int>(engine) < 8) {
        // Trigger Formless Spawn Engine Voice
        formlessEngine_.trigger(static_cast<int>(engine), velocity, -1.0f, false);
    } else {
        // Trigger DigiStrike Voice
        s.active = true;
        s.isSustained = false;
        s.env = velocity;
        s.phase = 0.0f;
        s.phase2 = 0.0f;
        s.filterState = 0.0f;
        s.burstTimer = 0.0f;
        s.burstDuration = 0.035f;
        s.stereoSpread = 0.5f;

        const float decayKnob = cachedParams_[slotIndex][1] ? cachedParams_[slotIndex][1]->load(std::memory_order_relaxed) : 0.3f;
        const float decaySec = std::clamp(0.01f + decayKnob * decayKnob * 2.0f, 0.01f, 3.0f);
        s.decayCoeff = std::exp(-1.0f / (static_cast<float>(sampleRate_) * decaySec));
        s.releaseCoeff = 0.0f;

        const float pitchKnob = cachedParams_[slotIndex][0] ? cachedParams_[slotIndex][0]->load(std::memory_order_relaxed) : 0.5f;
        const float pitchMult = std::pow(2.0f, (pitchKnob - 0.5f) * 4.0f);

        switch (engine) {
            case SlotEngineType::DIGISTRIKE_SD:
                s.phaseInc = (180.0f * pitchMult) / static_cast<float>(sampleRate_);
                s.phaseInc2 = (330.0f * pitchMult) / static_cast<float>(sampleRate_);
                break;
            case SlotEngineType::DIGISTRIKE_HH:
                formlessEngine_.chokeHats();
                break;
            case SlotEngineType::DIGISTRIKE_CLP:
                s.burstTimer = 0.0f;
                break;
            case SlotEngineType::DIGISTRIKE_TOM:
                s.targetPitch = 70.0f * pitchMult;
                s.pitchCur = 140.0f * pitchMult;
                s.pitchDrop = std::exp(-1.0f / (static_cast<float>(sampleRate_) * 0.08f));
                s.phaseInc = s.pitchCur / static_cast<float>(sampleRate_);
                break;
            case SlotEngineType::DIGISTRIKE_PRC:
                s.phaseInc = (640.0f * pitchMult) / static_cast<float>(sampleRate_);
                s.phaseInc2 = (1120.0f * pitchMult) / static_cast<float>(sampleRate_);
                break;
            case SlotEngineType::DIGISTRIKE_CYM:
                break;
            default:
                break;
        }
    }
}

void CrucibleWorkbenchAudioProcessor::handleNoteOn(int note, float velocity) noexcept {
    velocity = std::clamp(velocity, 0.05f, 1.0f);
    const float varDepth = macroVarParam_ ? macroVarParam_->load(std::memory_order_relaxed) : 0.5f;

    // =========================================================================
    // ZONE 1: SUSTAINED TUNED 808 SUB BASS (Notes 0 .. 59)
    // =========================================================================
    if (note < 60) {
        const float tuneVal = sub808TuneParam_ ? sub808TuneParam_->load(std::memory_order_relaxed) : 0.5f;
        const int subOct = subOctParam_ ? static_cast<int>(std::round(subOctParam_->load(std::memory_order_relaxed))) : 0;
        const float fineSemi = (tuneVal - 0.5f) * 24.0f; // -12 .. +12 semitones
        const float effNote = static_cast<float>(note + 24 + (subOct * 12)) + fineSemi;
        float targetFreq = 440.0f * std::pow(2.0f, (effNote - 69.0f) / 12.0f);
        targetFreq = std::clamp(targetFreq, 10.0f, 3500.0f);

        heldSubNote_ = note;
        sub808TargetPitchHz_ = targetFreq;

        const float glideVal = sub808GlideParam_ ? sub808GlideParam_->load(std::memory_order_relaxed) : 0.0f;
        if (glideVal < 0.01f || sub808CurrentPitchHz_ < 5.0f) {
            sub808CurrentPitchHz_ = targetFreq;
        }

        sub808Engine_.setSustain(true);
        sub808Engine_.trigger(velocity);
        return;
    }

    // =========================================================================
    // ZONE 2: CORE DRUM KIT & MACRO ARTICULATIONS (Notes 60 .. 83)
    // =========================================================================
    if (note == 60) {
        // Center C: 1-Shot Punch Kick
        currentKickPitchHz_ = 55.0f;
        mmeKick_.setSustain(false);
        mmeKick_.trigger(velocity);
        return;
    }

    if (note == 61) {
        // Lil Clap (Tight, narrow, crisp)
        triggerClap(false, velocity);
        return;
    }

    if (note == 62) {
        // Lil Snare (High crack, snappy, tight decay)
        triggerSnare(false, velocity);
        return;
    }

    if (note == 63) {
        // Big Clap (Wide flam, staggered bursts, stereo diffuse)
        triggerClap(true, velocity);
        return;
    }

    if (note == 64) {
        // Big Snare (Deep tone, rattling wires, heavy shell)
        triggerSnare(true, velocity);
        return;
    }

    if (note >= 65 && note <= 71) {
        // Hi-Hat Flagship Workstation: 7-Stage Articulations + Mutual Choke Group!
        hiHatEngine_.handleArticulationNote(note, velocity);
        return;
    }

    if (note >= 72 && note <= 83) {
        // The Cymbals: 4 Classes x 3 Articulations
        int cymIdx = note - 72;
        int cymClass = cymIdx / 3;
        int hitType = cymIdx % 3;

        int cymSlot = findCymbalSlot();
        auto engine = activeEngines_[cymSlot];

        static const float cymDecays[4][3] = {
            { 0.15f, 0.40f, 0.75f },
            { 0.35f, 1.20f, 3.20f },
            { 0.30f, 1.00f, 5.00f },
            { 0.25f, 0.85f, 3.80f }
        };

        const float decayKnob = cachedParams_[cymSlot][1] ? cachedParams_[cymSlot][1]->load(std::memory_order_relaxed) : 0.5f;
        float decaySec = cymDecays[cymClass][hitType] * (0.4f + decayKnob * 1.4f);

        if (static_cast<int>(engine) < 8) {
            formlessEngine_.triggerTransientWithCustom(3, velocity, decaySec);
        } else {
            auto& s = slotStates_[cymSlot];
            s.active = true;
            s.isSustained = false;
            s.env = velocity;
            s.filterState = 0.0f;
            s.decayCoeff = std::exp(-1.0f / (static_cast<float>(sampleRate_) * decaySec));
            s.releaseCoeff = 0.0f;
        }
        return;
    }

    // =========================================================================
    // ZONE 3: CHROMATIC TUNED TOMS (Notes 84 .. 127)
    // =========================================================================
    if (note >= 84 && note <= 127) {
        const int tomOct = tomOctParam_ ? static_cast<int>(std::round(tomOctParam_->load(std::memory_order_relaxed))) : -2;
        int effTomNote = note + (tomOct * 12);
        float tomFreq = 440.0f * std::pow(2.0f, (effTomNote - 69.0f) / 12.0f);
        tomFreq = std::clamp(tomFreq, 20.0f, 4000.0f);

        int tomSlot = findTomSlot();
        auto engine = activeEngines_[tomSlot];

        if (static_cast<int>(engine) < 8) {
            int v = (static_cast<int>(engine) < 4) ? static_cast<int>(engine) : 2;
            formlessEngine_.trigger(v, velocity, tomFreq, false);
        } else {
            auto& s = slotStates_[tomSlot];
            s.active = true;
            s.isSustained = false;
            s.env = velocity;
            s.phase = 0.0f;
            s.targetPitch = tomFreq;
            s.pitchCur = tomFreq * 2.2f;
            s.pitchDrop = std::exp(-1.0f / (static_cast<float>(sampleRate_) * 0.045f));
            s.phaseInc = s.pitchCur / static_cast<float>(sampleRate_);

            const float decayKnob = cachedParams_[tomSlot][1] ? cachedParams_[tomSlot][1]->load(std::memory_order_relaxed) : 0.35f;
            const float decaySec = std::clamp(0.02f + decayKnob * decayKnob * 2.2f, 0.02f, 3.0f);
            s.decayCoeff = std::exp(-1.0f / (static_cast<float>(sampleRate_) * decaySec));
            s.releaseCoeff = 0.0f;
        }
        return;
    }
}

void CrucibleWorkbenchAudioProcessor::handleNoteOff(int note) noexcept {
    if (note < 60 && note == heldSubNote_) {
        heldSubNote_ = -1;
        const float decayKnob = sub808DecayParam_ ? sub808DecayParam_->load(std::memory_order_relaxed) : 0.65f;
        const float releaseSec = std::clamp(0.04f + decayKnob * 0.25f, 0.02f, 0.5f);
        sub808Engine_.noteOff(releaseSec);
    }
}

// ==============================================================================
// AUDIO PROCESSING BLOCK
// ==============================================================================

void CrucibleWorkbenchAudioProcessor::processDigiStrikeSlot(int slotIndex, SlotState& s, float& outL, float& outR, float invSampleRate) noexcept {
    if (!s.active) return;

    float raw = 0.0f;
    switch (activeEngines_[slotIndex]) {
        case SlotEngineType::DIGISTRIKE_SD: {
            float body = 0.6f * std::sin(s.phase * 6.2831853f) + 0.4f * std::sin(s.phase2 * 6.2831853f);
            s.phase += s.phaseInc;
            if (s.phase >= 1.0f) s.phase -= 1.0f;
            s.phase2 += s.phaseInc2;
            if (s.phase2 >= 1.0f) s.phase2 -= 1.0f;

            float n = s.getNoise();
            s.filterState += 0.4f * (n - s.filterState);
            float snareWire = n - s.filterState;
            raw = body * 0.6f + snareWire * 0.75f;
            break;
        }
        case SlotEngineType::DIGISTRIKE_HH: {
            float n = s.getNoise();
            s.filterState += 0.55f * (n - s.filterState);
            raw = (n - s.filterState) * 0.85f;
            break;
        }
        case SlotEngineType::DIGISTRIKE_CLP: {
            s.burstTimer += invSampleRate;
            float burstAmp = 1.0f;
            if (s.burstTimer < s.burstDuration) {
                float cycle = std::fmod(s.burstTimer, 0.011f);
                burstAmp = (cycle < 0.003f) ? 1.0f : 0.15f;
            }
            float n = s.getNoise();
            s.filterState += 0.35f * (n - s.filterState);
            raw = (n - s.filterState) * burstAmp * 0.95f;
            break;
        }
        case SlotEngineType::DIGISTRIKE_TOM: {
            raw = std::sin(s.phase * 6.2831853f);
            s.phase += s.phaseInc;
            if (s.phase >= 1.0f) s.phase -= 1.0f;
            s.pitchCur = s.targetPitch + (s.pitchCur - s.targetPitch) * s.pitchDrop;
            s.phaseInc = s.pitchCur * invSampleRate;
            break;
        }
        case SlotEngineType::DIGISTRIKE_PRC: {
            float mod = std::sin(s.phase2 * 6.2831853f) * 0.5f;
            raw = std::sin((s.phase + mod) * 6.2831853f);
            s.phase += s.phaseInc;
            if (s.phase >= 1.0f) s.phase -= 1.0f;
            s.phase2 += s.phaseInc2;
            if (s.phase2 >= 1.0f) s.phase2 -= 1.0f;
            break;
        }
        case SlotEngineType::DIGISTRIKE_CYM: {
            float n = s.getNoise();
            s.filterState += 0.65f * (n - s.filterState);
            raw = (n - s.filterState) * 0.8f;
            break;
        }
        default:
            break;
    }

    raw *= s.env;
    if (s.isSustained) {
        // Hold envelope steady while key is pressed!
    } else {
        if (s.releaseCoeff > 0.0f) {
            s.env *= s.releaseCoeff;
        } else {
            s.env *= s.decayCoeff;
        }
        if (s.env < 0.0001f) {
            s.active = false;
        }
    }

    const float pan = (s.stereoSpread != 0.5f) ? s.stereoSpread
        : (cachedParams_[slotIndex][8] ? cachedParams_[slotIndex][8]->load(std::memory_order_relaxed) : 0.5f);
    const float vol = cachedParams_[slotIndex][9] ? cachedParams_[slotIndex][9]->load(std::memory_order_relaxed) : 0.8f;
    const float lAmp = std::cos(pan * 1.5707963f) * vol;
    const float rAmp = std::sin(pan * 1.5707963f) * vol;

    outL += raw * lAmp;
    outR += raw * rAmp;
}

void CrucibleWorkbenchAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    const float invSampleRate = 1.0f / static_cast<float>(sampleRate_);

    // 1. Evaluate PlayHead for Loop-Wrap / Bar Boundary Commit
    if (auto* playHead = getPlayHead()) {
        if (auto pos = playHead->getPosition()) {
            const bool isPlaying = pos->getIsPlaying();
            const double ppq = pos->getPpqPosition().orFallback(0.0);
            const int bar = pos->getBarCount().orFallback(0);

            bool shouldSwap = false;
            const int qMode = quantizeMode_.load(std::memory_order_relaxed);

            if (qMode == 0) { // Loop Wrap
                if (isPlaying && ppq < (lastPpq_ - 0.05)) {
                    shouldSwap = true;
                }
            } else if (qMode == 1) { // Bar Boundary
                if (isPlaying && bar != lastBar_) {
                    shouldSwap = true;
                }
            } else { // Immediate
                shouldSwap = true;
            }

            if (shouldSwap && swapPending_.load(std::memory_order_acquire)) {
                commitStagedSlots();
            }

            lastPpq_ = ppq;
            lastBar_ = bar;
        }
    }

    // 2. Process MIDI Note Events via Expressive Multi-Zone Handlers
    for (const auto metadata : midiMessages) {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn()) {
            handleNoteOn(msg.getNoteNumber(), msg.getFloatVelocity());
        } else if (msg.isNoteOff()) {
            handleNoteOff(msg.getNoteNumber());
        }
    }

    // 3. Update MmeKick parameters from Slot 0 (all 16 controls!)
    for (int p = 0; p < kParamsPerSlot; ++p) {
        if (cachedParams_[0][p]) {
            mmeKick_.setParam(p, cachedParams_[0][p]->load(std::memory_order_relaxed));
        }
    }

    // 3b. Update Dedicated 808 Sub-Bass parameters
    if (sub808ModelParam_)   sub808Engine_.setParam(0, sub808ModelParam_->load(std::memory_order_relaxed));
    if (sub808DecayParam_)   sub808Engine_.setParam(2, sub808DecayParam_->load(std::memory_order_relaxed));
    if (sub808PunchParam_)   sub808Engine_.setParam(3, sub808PunchParam_->load(std::memory_order_relaxed));
    if (sub808SweepParam_)   sub808Engine_.setParam(4, sub808SweepParam_->load(std::memory_order_relaxed));
    if (sub808DriveParam_)   sub808Engine_.setParam(7, sub808DriveParam_->load(std::memory_order_relaxed));
    if (sub808CutoffParam_)  sub808Engine_.setParam(9, sub808CutoffParam_->load(std::memory_order_relaxed));
    if (sub808FoldParam_)    sub808Engine_.setParam(15, sub808FoldParam_->load(std::memory_order_relaxed));

    const float glideVal = sub808GlideParam_ ? sub808GlideParam_->load(std::memory_order_relaxed) : 0.0f;
    const float glideCoeff = (glideVal > 0.005f) ? (1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate_) * (glideVal * 0.25f)))) : 1.0f;

    // 3c. Update MmeSnare & MmeClap & Mojo parameters (Col 1)
    if (scSdModelParam_) mmeSnare_.setParam(0, scSdModelParam_->load(std::memory_order_relaxed));
    if (scSdTuneParam_)  mmeSnare_.setParam(1, scSdTuneParam_->load(std::memory_order_relaxed));
    if (scSdDecayParam_) mmeSnare_.setParam(2, scSdDecayParam_->load(std::memory_order_relaxed));
    if (scSdSnapParam_)  mmeSnare_.setParam(3, scSdSnapParam_->load(std::memory_order_relaxed));
    if (scSdToneParam_)  mmeSnare_.setParam(4, scSdToneParam_->load(std::memory_order_relaxed));
    if (scSdWiresParam_) mmeSnare_.setParam(6, scSdWiresParam_->load(std::memory_order_relaxed));
    if (scSdBodyParam_)  mmeSnare_.setParam(7, scSdBodyParam_->load(std::memory_order_relaxed));

    if (scCpModelParam_)  mmeClap_.setParam(0, scCpModelParam_->load(std::memory_order_relaxed));
    if (scCpToneParam_)   mmeClap_.setParam(1, scCpToneParam_->load(std::memory_order_relaxed));
    if (scCpDecayParam_)  mmeClap_.setParam(2, scCpDecayParam_->load(std::memory_order_relaxed));
    if (scCpBurstParam_)  mmeClap_.setParam(3, scCpBurstParam_->load(std::memory_order_relaxed));
    if (scCpSpreadParam_) mmeClap_.setParam(4, scCpSpreadParam_->load(std::memory_order_relaxed));

    if (scMojoFltfParam_) scMojo_.setParam(0, scMojoFltfParam_->load(std::memory_order_relaxed));
    if (scMojoDistParam_) scMojo_.setParam(7, scMojoDistParam_->load(std::memory_order_relaxed));

    // 3d. Update Hi-Hat Flagship parameters (Col 2)
    for (int p = 0; p < 16; ++p) {
        if (hatParams_[p]) {
            hiHatEngine_.setParam(p, hatParams_[p]->load(std::memory_order_relaxed));
        }
    }

    // 4. Update Formless Spawn Parameters from Active Slots (Slots 1..7)
    for (int s = 1; s < kNumSlots; ++s) {
        auto eng = activeEngines_[s];
        if (static_cast<int>(eng) < 8) {
            int v = static_cast<int>(eng);
            for (int p = 0; p < tkdsp::FormlessSpawnEngine::kNumParamsPerVoice; ++p) {
                if (cachedParams_[s][p]) {
                    formlessEngine_.setParam(v, p, cachedParams_[s][p]->load(std::memory_order_relaxed));
                }
            }
        }
    }

    const float masterVol = masterVolumeParam_ ? masterVolumeParam_->load(std::memory_order_relaxed) : 0.8f;
    const float masterDrive = masterDriveParam_ ? masterDriveParam_->load(std::memory_order_relaxed) : 0.0f;
    const float masterLowBoost = masterLowBoostParam_ ? masterLowBoostParam_->load(std::memory_order_relaxed) : 0.0f;

    formlessEngine_.setMasterLowBoost(masterLowBoost);
    formlessEngine_.setMasterDrive(masterDrive);

    float* outL = buffer.getWritePointer(0);
    float* outR = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : outL;

    // 5. Render Sample-by-Sample
    for (int i = 0; i < numSamples; ++i) {
        // Render MmeKick Flagship
        float kickL = 0.0f, kickR = 0.0f;
        mmeKick_.renderSampleStereo(currentKickPitchHz_, kickL, kickR);

        // 808 Sub Glide
        if (glideCoeff < 0.999f) {
            sub808CurrentPitchHz_ += (sub808TargetPitchHz_ - sub808CurrentPitchHz_) * glideCoeff;
        } else {
            sub808CurrentPitchHz_ = sub808TargetPitchHz_;
        }

        // Render Dedicated 808 Sub-Bass Voice
        float subL = 0.0f, subR = 0.0f;
        sub808Engine_.renderSampleStereo(sub808CurrentPitchHz_, subL, subR);

        // Render Snare & Clap Flagship Workstation (Col 1)
        float sdL = 0.0f, sdR = 0.0f;
        mmeSnare_.renderSampleStereo(200.0f, sdL, sdR);
        float cpL = 0.0f, cpR = 0.0f;
        mmeClap_.renderSampleStereo(200.0f, cpL, cpR);
        float scL = sdL + cpL;
        float scR = sdR + cpR;
        scMojo_.processSampleStereo(scL, scR);

        // Render Hi-Hat Flagship Workstation (Col 2)
        float hatL = 0.0f, hatR = 0.0f;
        hiHatEngine_.renderSampleStereo(hatL, hatR);

        // Render Formless Engine
        float fL = 0.0f, fR = 0.0f;
        formlessEngine_.renderSampleStereo(fL, fR);

        // Render DigiStrike Voices
        float dsL = 0.0f, dsR = 0.0f;
        for (int s = 1; s < kNumSlots; ++s) {
            if (static_cast<int>(activeEngines_[s]) >= 8) {
                processDigiStrikeSlot(s, slotStates_[s], dsL, dsR, invSampleRate);
            }
        }

        float mixL = (kickL + subL + scL + hatL + fL + dsL) * masterVol;
        float mixR = (kickR + subR + scR + hatR + fR + dsR) * masterVol;

        outL[i] = tkdsp::FastMath::fastTanh(mixL * 0.85f);
        outR[i] = tkdsp::FastMath::fastTanh(mixR * 0.85f);
    }
}

SlotEngineType CrucibleWorkbenchAudioProcessor::getSlotEngine(int slot) const noexcept {
    if (slot >= 0 && slot < kNumSlots) {
        return activeEngines_[slot];
    }
    return SlotEngineType::MME_KICK;
}

void CrucibleWorkbenchAudioProcessor::setSlotEngine(int slot, SlotEngineType type, bool immediate) noexcept {
    if (slot >= 0 && slot < kNumSlots) {
        stagedEngines_[slot] = type;
        if (immediate) {
            commitStagedSlots();
        } else {
            swapPending_.store(true, std::memory_order_release);
        }
    }
}

void CrucibleWorkbenchAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts_.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void CrucibleWorkbenchAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts_.state.getType())) {
        apvts_.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout CrucibleWorkbenchAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Slot 0: MmeKick 16-Control Dedicated Venti Parameters
    const char* kickParamNames[16] = {
        "Kick Model", "Kick Pitch", "Kick Decay", "Kick Punch",
        "Kick Sweep", "Kick Swp-T", "Kick Sub", "Kick Body",
        "Kick Filter Cutoff", "Kick Filter Width", "Kick Filter Reso", "Kick EQ Freq",
        "Kick EQ Gain", "Kick SRR Decimate", "Kick BRR Bitcrush", "Kick Wavefold Drive"
    };

    float kickDefaults[16] = {
        0.467f, // Model 7: TR-808-BD (7/15)
        0.50f,  // Pitch 1:1 concert pitch
        0.55f,  // Decay
        0.40f,  // Punch
        0.25f,  // Sweep
        0.20f,  // Swp-T
        0.65f,  // Sub boost
        0.35f,  // Body
        0.00f,  // FLTF
        1.00f,  // FLTW
        0.00f,  // FLTQ
        0.50f,  // EQ-F
        0.50f,  // EQ-G
        0.00f,  // SRR
        0.00f,  // BRR
        0.00f   // DIST / Wavefold
    };

    for (int p = 0; p < kParamsPerSlot; ++p) {
        juce::String id = "s0_p" + juce::String(p);
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(id, 1), kickParamNames[p],
            juce::NormalisableRange<float>(0.0f, 1.0f), kickDefaults[p]
        ));
    }

    // Slots 1..7 (Snare, Tom, West, Open Hat, Closed Hat, Clap, Crash)
    for (int s = 1; s < kNumSlots; ++s) {
        for (int p = 0; p < kParamsPerSlot; ++p) {
            juce::String id = "s" + juce::String(s) + "_p" + juce::String(p);
            juce::String name = "Slot " + juce::String(s + 1) + " P" + juce::String(p + 1);
            float defVal = 0.5f;
            if (p == 1) defVal = 0.3f;
            if (p == 9) defVal = 0.85f;
            layout.add(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID(id, 1), name,
                juce::NormalisableRange<float>(0.0f, 1.0f), defVal
            ));
        }
    }

    // Master Controls
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("master_volume", 1), "Master Volume",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.85f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("master_drive", 1), "Master Drive",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("master_low_boost", 1), "Master Low Boost",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f
    ));

    // Multi-Zone Tuning & Macro Parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sub_octave", 1), "808 Sub Octave",
        juce::NormalisableRange<float>(-2.0f, 2.0f, 1.0f), 0.0f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("tom_octave", 1), "Tom Octave",
        juce::NormalisableRange<float>(-3.0f, 1.0f, 1.0f), -2.0f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("macro_variation", 1), "Macro Variation Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f
    ));

    // Dedicated 808 Sub-Bass Voice Parameters (Zone 1)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sub808_model", 1), "808 Model",
        juce::NormalisableRange<float>(0.0f, 1.0f), 7.0f / 15.0f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sub808_tune", 1), "808 Sub Tune",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sub808_decay", 1), "808 Sub Decay",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.65f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sub808_punch", 1), "808 Sub Punch",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.35f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sub808_sweep", 1), "808 Sub Sweep",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.25f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sub808_fold", 1), "808 Sub Wavefold Drive",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sub808_drive", 1), "808 Sub Drive",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.15f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sub808_cutoff", 1), "808 Sub Cutoff",
        juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sub808_glide", 1), "808 Sub Glide",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f
    ));

    // Snare & Clap Flagship Workstation Parameters (Col 1)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sc_sd_model", 1), "Snare Model",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sc_sd_tune", 1), "Snare Tune",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.40f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sc_sd_decay", 1), "Snare Decay",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.45f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sc_sd_snap", 1), "Snare Snap",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.55f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sc_sd_tone", 1), "Snare Tone",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.50f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sc_sd_wires", 1), "Snare Wires",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.40f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sc_sd_body", 1), "Snare Body",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.30f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sc_sd_var", 1), "Snare Variation Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.50f
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sc_cp_model", 1), "Clap Model",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sc_cp_tone", 1), "Clap Tone",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.45f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sc_cp_decay", 1), "Clap Decay",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.45f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sc_cp_burst", 1), "Clap Burst Count",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.60f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sc_cp_spread", 1), "Clap Spread",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.40f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sc_cp_var", 1), "Clap Variation Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.50f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sc_mojo_fltf", 1), "Snare/Clap Cutoff",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sc_mojo_dist", 1), "Snare/Clap Drive",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f
    ));

    // Hi-Hat Flagship Workstation Parameters (Col 2)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("hat_pitch", 1), "Hat Pitch",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.50f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("hat_cl_decay", 1), "Closed Hat Decay",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.30f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("hat_op_decay", 1), "Open Hat Decay",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.65f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("hat_metal", 1), "Hat Metal Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.70f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("hat_choke", 1), "Hat Choke Speed",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.20f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("hat_bright", 1), "Hat Brightness",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.65f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("hat_pedal", 1), "Hat Pedal Thud",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.30f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("hat_accent", 1), "Hat Accent Dynamic",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.50f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("hat_fltf", 1), "Hat Filter HP",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.00f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("hat_fltw", 1), "Hat Filter Width",
        juce::NormalisableRange<float>(0.0f, 1.0f), 1.00f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("hat_fltq", 1), "Hat Filter Reso",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.00f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("hat_eqf", 1), "Hat EQ Freq",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.50f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("hat_eqg", 1), "Hat EQ Gain",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.50f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("hat_srr", 1), "Hat SRR Decimate",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.00f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("hat_brr", 1), "Hat BRR Bitcrush",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.00f
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("hat_drive", 1), "Hat Drive / Fold",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.00f
    ));

    return layout;
}

juce::AudioProcessorEditor* CrucibleWorkbenchAudioProcessor::createEditor() {
    return new CrucibleWorkbenchEditor(*this);
}

bool CrucibleWorkbenchAudioProcessor::hasEditor() const {
    return true;
}

// ==============================================================================
// UDP IPC THREAD (Port 9123)
// ==============================================================================
CrucibleWorkbenchAudioProcessor::IPCThread::IPCThread(CrucibleWorkbenchAudioProcessor& owner, int port)
    : juce::Thread("CrucibleWorkbench_IPC"), owner_(owner), port_(port)
{
    socket_ = std::make_unique<juce::DatagramSocket>(false);
    socket_->bindToPort(port_);
}

CrucibleWorkbenchAudioProcessor::IPCThread::~IPCThread() {
    signalThreadShouldExit();
    if (socket_ != nullptr) {
        socket_->shutdown();
    }
    stopThread(500);
}

void CrucibleWorkbenchAudioProcessor::IPCThread::run() {
    char buffer[2048];
    while (!threadShouldExit()) {
        if (socket_ == nullptr) {
            wait(50);
            continue;
        }

        int ready = socket_->waitUntilReady(true, 50);
        if (ready == 1) {
            juce::String senderIP;
            int senderPort = 0;
            int bytesRead = socket_->read(buffer, sizeof(buffer) - 1, false, senderIP, senderPort);
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                juce::String msg = juce::String::fromUTF8(buffer);
                auto json = juce::JSON::parse(msg);

                juce::DynamicObject* response = new juce::DynamicObject();
                response->setProperty("success", true);

                if (json.isObject()) {
                    juce::String cmd = json["cmd"].toString();

                    if (cmd == "status") {
                        juce::Array<juce::var> slotsArr;
                        for (int i = 0; i < kNumSlots; ++i) {
                            juce::DynamicObject* slotObj = new juce::DynamicObject();
                            slotObj->setProperty("slot", i);
                            slotObj->setProperty("engine_id", static_cast<int>(owner_.activeEngines_[i]));
                            slotObj->setProperty("engine_name", getSlotEngineName(owner_.activeEngines_[i]));
                            slotsArr.add(slotObj);
                        }
                        response->setProperty("slots", slotsArr);
                        response->setProperty("swap_pending", owner_.isSwapPending());
                        response->setProperty("quantize_mode", owner_.getQuantizeMode());
                        response->setProperty("udp_port", owner_.getUdpPort());
                        response->setProperty("mme_kick_model", owner_.getMmeKickModelName());
                    }
                    else if (cmd == "reassign" || cmd == "stage") {
                        bool immediate = (json["quantize"].toString() == "IMMEDIATE");
                        auto slotsVar = json["slots"];
                        if (slotsVar.isObject()) {
                            for (auto prop : slotsVar.getDynamicObject()->getProperties()) {
                                int slotIdx = prop.name.toString().getIntValue();
                                int engineIdx = prop.value;
                                if (slotIdx >= 0 && slotIdx < kNumSlots && engineIdx >= 0 && engineIdx <= 14) {
                                    owner_.stagedEngines_[slotIdx] = static_cast<SlotEngineType>(engineIdx);
                                }
                            }
                        }
                        if (immediate) {
                            owner_.commitStagedSlots();
                            response->setProperty("action", "committed_immediately");
                        } else {
                            owner_.swapPending_.store(true, std::memory_order_release);
                            response->setProperty("action", "staged_for_loop_wrap");
                        }
                    }
                    else if (cmd == "set_param") {
                        int slot = json["slot"];
                        int param = json["param"];
                        float val = static_cast<float>(json["val"]);
                        if (slot >= 0 && slot < kNumSlots && param >= 0 && param < kParamsPerSlot) {
                            if (owner_.cachedParams_[slot][param]) {
                                owner_.cachedParams_[slot][param]->store(val, std::memory_order_relaxed);
                                response->setProperty("param_set", true);
                            }
                        }
                    }
                }

                juce::String jsonStr = juce::JSON::toString(juce::var(response));
                socket_->write(senderIP, senderPort, jsonStr.toRawUTF8(), static_cast<int>(jsonStr.getNumBytesAsUTF8()));
            }
        } else if (ready < 0) {
            wait(50);
        }
    }
}

// JUCE Plugin Filter Factory Entry Point
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new CrucibleWorkbenchAudioProcessor();
}
