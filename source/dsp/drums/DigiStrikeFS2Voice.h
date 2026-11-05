#pragma once

#include "FastMath.h"
#include "FormlessModMath.h"
#include "../dynamics/WiderStage.h"
#include "../dynamics/DrumLimiter.h"
#include "../dynamics/SquashStage.h"
#include "../mojo/FreqShifter.h"
#include "../../digistrike_fs2/DigiStrikeFS2Catalog.h"
#include "../../digistrike_fs2/DigiStrikeFS2ROM.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

// ==============================================================================
// DigiStrikeFS2Voice.h — Sovereign Vintage PCM Workstation Voice Engine (8-Voice Poly)
// ==============================================================================
// Part of The Klang Suite SOCEP Engine. Copyright (c) 2026 Klang Industries.
// Clean-Room IP Compliance: Zero commercial trademarks.
// ==============================================================================

namespace tkdsp {

namespace FastMath = TbdAudio::FastMath;

struct DigiStrikeFS2Params {
    // Header Controls (4)
    int category{1};         // 0=BD, 1=SN, 2=TM, 3=CP, 4=HH, 5=CY, 6=LN, 7=MC, 8=FX, 9=ST, 10=BS (Default: SN)
    int lineage{0};          // 0=CROWN .. 19=CURIOS
    uint32_t lineageMask{0xFFFFFu}; // 20-bit multi-select mask (all 20 lineages default)
    int voiceMode{2};        // 0=MONO, 1=GLIDE, 2=POLY (Default: POLY)
    int lifeMode{1};         // 0=Synthetic, 1=Cyborg, 2=Organic (Default: Cyborg)

    // Card 1: Master Macro & Dynamics Lab (8)
    float macroX{0.5f};      // 0.0 .. 1.0 (Spectral Macro)
    float macroY{0.5f};      // 0.0 .. 1.0 (Punch Macro)
    float macroZ{0.5f};      // 0.0 .. 1.0 (Timbre Macro)
    float macroT{0.5f};      // 0.0 .. 1.0 (Time Macro)
    float velAggr{0.5f};     // 0.0 .. 1.0 (Vel -> Aggression)
    float velTimb{0.5f};     // 0.0 .. 1.0 (Vel -> Timbre)
    float pan{0.0f};         // -1.0 .. +1.0
    float decayCurve{0.0f};  // -1.0 .. +1.0

    // Card 2: 4-Column Functional Pods (8)
    float pitchDecay{15.0f}; // 2.0 .. 250.0 ms (P.DEC, skew 30ms)
    float clickBleed{0.0f};  // 0.0 .. 100.0 % (CLICK)
    float startOffset{0.0f}; // 0.0 .. 500.0 ms (START, skew 20ms)
    float traverseRate{1.0f};// 0.25 .. 4.0 x (TIME, 1.0 nominal, higher = longer duration)
    float pitchDive{0.0f};   // 0.0 .. 100.0 % (PUNCH, 0..36 semitones)
    float noiseMix{0.0f};    // 0.0 .. 100.0 % (NOISE, Enveloped Noise Mix)
    float sampleLength{100.0f}; // 0.0 .. 100.0 % (LENGTH, 0% = single-cycle freeze)
    float tableMorph{8.74317f}; // 0.0 .. 100.0 % (MORPH, 3D Spectral Slice, Default: Snare 49/550)
    int   loopMode{0};        // 0=ONCE, 1=HOLD, 2=LOOP, 3=PING
    bool  reverse{false};     // false=forward, true=reverse

    // Card 3 Pod 1: Stepped Random Humanization (8)
    float randX{0.0f};       // 0.0 .. 100.0 %
    float randY{0.0f};       // 0.0 .. 100.0 %
    float randZ{0.0f};       // 0.0 .. 100.0 %
    float randT{0.0f};       // 0.0 .. 100.0 %
    float randAggr{0.0f};    // 0.0 .. 100.0 %
    float randTimb{0.0f};    // 0.0 .. 100.0 %
    float randPan{0.0f};     // 0.0 .. 100.0 %
    float randDecay{0.0f};   // 0.0 .. 100.0 %

    // Card 3 Pod 2 & 3: Master Altar & Mojo Console (14)
    float macroDecay{0.5f};  // 0.0 .. 1.0 (Master Hero Knob)
    float volume{0.5f};      // 0.0 .. 1.0 (-inf .. 0dB .. +12dB)
    float sustainLevel{0.0f};// 0.0 .. 100.0 % (<=2% One-Shot, >2% Gated ADSR)
    float ampDecay{1.000f};  // 0.005 .. 5.0 s (skew 0.250s)
    float glideTime{0.050f}; // 0.001 .. 2.0 s (skew 0.050s, Portamento Glide)
    float ampVelocity{50.0f};// 0.0 .. 100.0 % (0%=Flat, 50%=Linear, 100%=Exponential)

    float cutoff{20000.0f};  // 20.0 .. 20000.0 Hz
    float resonance{0.30f};  // 0.0 .. 1.0
    float filterEnvAmt{0.0f};// 0.0 .. 1.0 (unipolar 0 .. +10 Octaves)
    float filterDecay{0.150f};// 0.005 .. 5.0 s
    int   filterType{0};     // 0=LP, 1=HP, 2=BP, 3=PK
    float preDrive{0.0f};    // 0.0 .. 1.0
    float crush{48000.0f};   // 500.0 .. 48000.0 Hz
    float shift{0.0f};       // -1000.0 .. +1000.0 Hz
    float squash{0.0f};      // -1.0 .. +1.0
    float wider{0.0f};       // 0.0 .. 2.0
    bool  masterLimiter{false};
};

class DigiStrikeFS2Voice {
public:
    static constexpr int kMaxVoices = 8;
    static constexpr size_t kSynthTableSize = 2048;

    struct NoteEntry {
        int noteNumber{0};
        float velocity{0.0f};
    };

    DigiStrikeFS2Voice() noexcept = default;

    void prepare(float sampleRate) noexcept {
        sampleRate_ = (sampleRate > 1000.0f) ? sampleRate : 48000.0f;
        invSampleRate_ = 1.0f / sampleRate_;

        grainLengthSec_ = 0.030f;
        grainLengthSamples_ = static_cast<double>(grainLengthSec_ * sampleRate_);
        invGrainLengthSamples_ = 1.0 / std::max(1.0, grainLengthSamples_);
        kEdgeCoef_ = 1.0f - std::exp(-invSampleRate_ / 0.005f);

        freqShifter_.prepare(sampleRate_);
        widerStage_.prepare(sampleRate_);
        drumLimiter_.prepare(sampleRate_);
        squashStage_.prepare(sampleRate_);
        lpgAttCoeff_ = 1.0f - std::exp(-invSampleRate_ / 0.003f); // 3ms strike attack
        lpgRelCoeff_ = 1.0f - std::exp(-invSampleRate_ / 0.060f); // 60ms sluggish droop

        reset();
    }

    [[nodiscard]] const std::array<float, 8>& getLatchedDraws() const noexcept { return latchedDraw_; }
    [[nodiscard]] const std::array<float, 8>& getVoiceRandVal(size_t slot) const noexcept {
        return (slot < voices_.size()) ? voices_[slot].randVal : latchedDraw_;
    }
    [[nodiscard]] float getVoicePanL(size_t slot) const noexcept {
        return (slot < voices_.size()) ? voices_[slot].panL : 0.7071f;
    }
    [[nodiscard]] float getVoicePanR(size_t slot) const noexcept {
        return (slot < voices_.size()) ? voices_[slot].panR : 0.7071f;
    }
    [[nodiscard]] float getVoiceLoopSpanMult(size_t slot) const noexcept {
        return (slot < voices_.size()) ? voices_[slot].loopSpanMult : 1.0f;
    }
    [[nodiscard]] float getVoiceTravPitchScale(size_t slot) const noexcept {
        return (slot < voices_.size()) ? voices_[slot].travPitchScale : 1.0f;
    }

    void reset() noexcept {
        for (auto& v : voices_) {
            v.active = false;
            v.oscPhase = 0.0;
            v.playheadSec = 0.0;
            v.tStartSec = 0.0;
            v.tEndSec = 0.0;
            v.tTotalSec = 0.0;
            v.ampEnv = 0.0f;
            v.pitchEnv = 0.0f;
            v.chokeEnv = 1.0f;
            v.chokeActive = false;
            v.loopMode = 4;
            v.inReversePhase = false;
            v.ppgDirection = 1;
            v.prevTraverseRate = 1.0f;
            v.sp12ClockPhase = 0.0;
            v.zohSampledVal = 0.0f;
            v.cyborgChargeSpike = 0.0f;
            v.lpgState = 0.0f;
            v.isFadingOut = false;
            v.fadeOutEnv = 1.0f;
            v.loopSpanMult = 1.0f;
            v.randLen = 0.0f;
            v.travPitchScale = 1.0f;
            v.tSm = 1.0f;
        }
        activeVoiceCount_ = 0;
        masterFilterState_.fill(0.0f);
        masterFilterEnv_ = 0.0f;
        glideHz_ = 130.81f;

        noteStackSize_ = 0;
        activeNoteCount_ = 0;
        lastNoteOffSample_ = 0;
        currentSampleCount_ = 0;
        lastPitchHz_ = 130.81f;
        for (auto& n : noteStack_) n = {};

        freqShifter_.reset();
        widerStage_.reset();
        drumLimiter_.reset();
        squashStage_.reset();
        crushPhase_ = 0.0f;
        crushedSampleL_ = 0.0f;
        crushedSampleR_ = 0.0f;
        panPhase_ = 0.0f;
        loopPhase_ = 0.0f;
    }

    [[nodiscard]] int getActiveVoiceCount() const noexcept { return activeVoiceCount_; }
    [[nodiscard]] double getVoiceU(size_t slot = 0) const noexcept { return (slot < voices_.size()) ? voices_[slot].u : 0.0; }

    void triggerNote(int noteNumber, float velocity, float noteHz) noexcept {
        velocity = std::clamp(velocity, 0.01f, 1.0f);
        lastNoteNumber_ = noteNumber;
        lastVelocity_ = velocity;

        const bool isHolding = (activeNoteCount_ > 0);
        activeNoteCount_++;

        // Tracker Auto-Legato: 15ms grace window across consecutive note-off -> note-on events
        const double msSinceOff = (sampleRate_ > 0.0f)
            ? (static_cast<double>(currentSampleCount_ - lastNoteOffSample_) * 1000.0 / sampleRate_)
            : 999.0;
        const bool autoLegatoEligible = (!isHolding && msSinceOff <= 15.0 && lastPitchHz_ > 20.0f);
        const bool isLegato = isHolding || autoLegatoEligible;

        // Update mono/glide note stack (move-to-top if already held)
        bool inStack = false;
        for (int i = 0; i < noteStackSize_; ++i) {
            if (noteStack_[i].noteNumber == noteNumber) {
                inStack = true;
                NoteEntry e = noteStack_[i];
                e.velocity = velocity;
                for (int j = i; j < noteStackSize_ - 1; ++j) noteStack_[j] = noteStack_[j + 1];
                noteStack_[noteStackSize_ - 1] = e;
                break;
            }
        }
        if (!inStack && noteStackSize_ < 16) {
            noteStack_[static_cast<size_t>(noteStackSize_++)] = NoteEntry{noteNumber, velocity};
        }
        activeNoteCount_ = noteStackSize_;

        // Mode 0: MONO (instant 2ms choke on existing voices)
        // Mode 1: GLIDE (legato bass mode with portamento)
        // Mode 2: POLY (8-voice polyphonic allocation)
        if (params_.voiceMode == 0) {
            for (auto& v : voices_) {
                if (v.active) chokeVoice(v, velocity);
            }
            allocateVoice(0, noteNumber, velocity, noteHz);
        } else if (params_.voiceMode == 1) {
            if (isLegato && voices_[0].active) {
                // Legato Bass Glide: Repoint target pitch WITHOUT retriggering amplitude envelope!
                voices_[0].targetHz = noteHz;
                voices_[0].noteNumber = noteNumber;
                voices_[0].velocity = velocity;
                voices_[0].effVelocityGain = CrackedChimes::DSP::calculateVelocityGain(velocity, params_.ampVelocity * 0.01f);
            } else {
                // Fresh note attack
                for (auto& v : voices_) {
                    if (v.active) chokeVoice(v, velocity);
                }
                const float startHz = (activeVoiceCount_ > 0 && voices_[0].currentHz > 20.0f) ? voices_[0].currentHz : noteHz;
                allocateVoice(0, noteNumber, velocity, noteHz);
                voices_[0].currentHz = startHz;
            }
        } else {
            // Poly 8: Choke hat group if applicable (cat 4 = hats)
            if (params_.category == 4) {
                for (auto& v : voices_) {
                    if (v.active && v.category == 4) chokeVoice(v, velocity);
                }
            }
            int slot = findVoiceSlot();
            allocateVoice(slot, noteNumber, velocity, noteHz);
        }

        triggerMasterFilterEnv(velocity);
    }

    void releaseNote(int noteNumber) noexcept {
        lastNoteOffSample_ = currentSampleCount_;
        if (voices_[0].active) {
            lastPitchHz_ = voices_[0].currentHz;
        }

        // Remove released note from note stack
        if (noteNumber >= 0) {
            for (int i = 0; i < noteStackSize_; ++i) {
                if (noteStack_[i].noteNumber == noteNumber) {
                    for (int j = i; j < noteStackSize_ - 1; ++j) {
                        noteStack_[j] = noteStack_[j + 1];
                    }
                    noteStackSize_--;
                    break;
                }
            }
        } else if (noteStackSize_ > 0) {
            noteStackSize_--;
        }

        activeNoteCount_ = noteStackSize_;

        if (params_.voiceMode == 0 || params_.voiceMode == 1) {
            // MONO / GLIDE Last-Note Priority:
            if (noteStackSize_ > 0 && activeNoteCount_ > 0) {
                // Releasing upper key while still holding root key!
                // Return target pitch to top remaining held note, KEEPING GATE OPEN!
                const int heldNote = noteStack_[noteStackSize_ - 1].noteNumber;
                const float heldHz = 440.0f * FastMath::fastPow2((static_cast<float>(heldNote) - 69.0f) * (1.0f / 12.0f));
                if (voices_[0].active) {
                    voices_[0].targetHz = heldHz;
                    voices_[0].noteNumber = heldNote;
                    // Note remains unreleased & active!
                }
            } else {
                // All held notes released: voice enters release phase
                for (auto& v : voices_) {
                    if (v.active) {
                        if (sustainPedal_) v.pendingPedalRelease = true;
                        else v.released = true;
                    }
                }
                noteStackSize_ = 0;
                activeNoteCount_ = 0;
            }
        } else {
            // POLY: Normal per-note release
            for (auto& v : voices_) {
                if (v.active && (v.noteNumber == noteNumber || noteNumber < 0)) {
                    if (v.isGated) {
                        if (sustainPedal_) v.pendingPedalRelease = true;
                        else v.released = true;
                    }
                }
            }
        }
    }

    void setSustainPedal(bool isDown) noexcept {
        if (sustainPedal_ == isDown) return;
        sustainPedal_ = isDown;
        if (!sustainPedal_) {
            // Pedal lifted: release all voices that were held by sustain pedal
            for (auto& v : voices_) {
                if (v.active && v.pendingPedalRelease) {
                    v.pendingPedalRelease = false;
                    v.released = true;
                }
            }
        }
    }

    void setParams(const DigiStrikeFS2Params& p) noexcept {
        if ((p.voiceMode == 0 || p.voiceMode == 1) && params_.voiceMode == 2) {
            // Mode switch from POLY to MONO/GLIDE: choke voices 1..7 immediately
            for (size_t i = 1; i < voices_.size(); ++i) {
                if (voices_[i].active) {
                    voices_[i].chokeActive = true;
                    voices_[i].chokeRate = 1.0f / (0.010f * sampleRate_);
                }
            }
        }
        params_ = p;
    }

    void renderBlock(float* outL, float* outR, int numSamples) noexcept {
        if (numSamples <= 0) return;
        currentSampleCount_ += numSamples;

        std::fill_n(outL, numSamples, 0.0f);
        std::fill_n(outR, numSamples, 0.0f);

        const float targetMorph = std::clamp(params_.tableMorph * 0.01f, 0.0f, 1.0f);
        const float invNumSamples = 1.0f / static_cast<float>(numSamples);

        int activeCount = 0;
        for (auto& v : voices_) {
            if (v.active) {
                renderVoice(v, outL, outR, numSamples, targetMorph, invNumSamples);
                if (v.active) ++activeCount;
            }
        }
        activeVoiceCount_ = activeCount;

        processMasterBus(outL, outR, numSamples);
    }

private:
    struct VoiceSlot {
        bool active{false};
        bool isGated{false};
        bool released{false};
        bool pendingPedalRelease{false};
        bool chokeActive{false};
        uint8_t activeCount{0};
        std::array<DigiStrikeFS2Catalog::WaveCoord, 80> activeWaves{};
        float currentMorph{0.0f};

        bool isSustaining{false};
        float noiseHpState{0.0f};
        float noiseRawPrev{0.0f};

        int noteNumber{60};
        int category{7};
        int slotIdx{154};
        float morph{0.0f}; // 0.0 .. 1.0
        float velocity{0.9f};
        float effVelocityGain{1.0f};
        float targetHz{261.63f};
        float currentHz{261.63f};
        float staticPitchRatio{1.0f};

        // Intra-frame oscillator phase accumulator (for single-cycle freeze mode)
        double oscPhase{0.0};

        // Dual-domain PCM & 2-Grain OLA Traversal State
        uint32_t voiceA{0};
        uint32_t voiceB{0};
        float morphFrac{0.0f};
        double playheadSec{0.0};
        double tStartSec{0.0};
        double tEndSec{0.0};
        double tTotalSec{0.0};
        bool isReverse{false};

        double u{0.0};                     // Traversal position in source seconds
        double grainPhase{0.0};            // phi0 in [0.0, 1.0)
        double gAnchor[2]{0.0, 0.0};       // Latched anchors for Grain 0 & 1
        float  gDir[2]{1.0f, 1.0f};        // Latched direction (+1 / -1)
        float  tStartSm{0.0f};             // Dynamically smoothed start boundary (5ms 1-pole)
        float  tEndSm{0.0f};               // Dynamically smoothed end boundary (5ms 1-pole)
        double dEffSec{0.5};               // Effective voice duration
        float  rootHz{261.63f};            // Root note frequency in Hz
        int    transientRemaining{0};      // 15ms Pure-Impact Transient Gate samples
        int    sampleCountTrigger{0};      // Initial transient gate counter

        float ampEnv{1.0f};
        float ampDecayRate{0.001f};
        float ampReleaseRate{0.005f};

        float pitchEnv{0.0f};
        float pitchEnvRate{0.01f};
        float pitchDiveSemitones{0.0f};

        float chokeEnv{1.0f};
        float chokeRate{0.05f};

        // Vintage Lifeform state
        double sp12ClockPhase{0.0};
        float zohSampledVal{0.0f};
        float cyborgChargeSpike{0.0f};
        float lpgState{0.0f};

        // Anti-click fadeout
        bool isFadingOut{false};
        float fadeOutEnv{1.0f};
        float fadeOutRate{0.005f};

        int loopMode{4};
        bool inReversePhase{false};
        int ppgDirection{1};
        float prevTraverseRate{1.0f};
        float ampDecaySeconds{1.0f};
        float effMacroT{0.5f};
        float effMacroX{0.5f};
        float effMacroZ{0.5f};
        float effTimb{0.5f};
        float effClickBleed{0.0f};
        float effNoiseMix{0.0f};
        bool  decayCurveActive{false};
        float decayCurveExp{1.0f};

        std::array<float, 8> randVal{{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};
        std::array<float, 8> randBip{{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};
        float panL{0.7071f};
        float panR{0.7071f};
        float targetPanL{0.7071f};
        float targetPanR{0.7071f};
        float loopSpanMult{1.0f};
        float randLen{0.0f};
        float travPitchScale{1.0f};
        float tSm{1.0f};
    };

    inline float nextFloat() noexcept {
        rngState_ += 0x9E3779B9u;
        uint32_t z = rngState_;
        z = (z ^ (z >> 16)) * 0x85EBCA6Bu;
        z = (z ^ (z >> 13)) * 0xC2B2AE35u;
        z ^= (z >> 16);
        return static_cast<float>((z >> 8) & 0x00FFFFFF) * (1.0f / 16777216.0f);
    }

    int findVoiceSlot() noexcept {
        for (int i = 0; i < kMaxVoices; ++i) {
            if (!voices_[static_cast<size_t>(i)].active) return i;
        }
        int lowestIdx = 0;
        float lowestAmp = 100.0f;
        for (int i = 0; i < kMaxVoices; ++i) {
            if (voices_[static_cast<size_t>(i)].ampEnv < lowestAmp) {
                lowestAmp = voices_[static_cast<size_t>(i)].ampEnv;
                lowestIdx = i;
            }
        }
        return lowestIdx;
    }

    void chokeVoice(VoiceSlot& v, float chokerVelocity) noexcept {
        v.chokeActive = true;
        float chokeMs = 15.0f - 7.0f * chokerVelocity;
        v.chokeRate = 1.0f / (chokeMs * 0.001f * sampleRate_);
    }

    void allocateVoice(int slot, int noteNumber, float velocity, float noteHz) noexcept {
        if (slot < 0 || slot >= kMaxVoices) return;
        auto& v = voices_[static_cast<size_t>(slot)];

        v.active = true;
        v.released = false;
        v.chokeActive = false;
        v.chokeEnv = 1.0f;
        v.isFadingOut = false;
        v.fadeOutEnv = 1.0f;
        v.fadeOutRate = 1.0f / (0.002f * sampleRate_); // 2ms fadeout
        v.noteNumber = noteNumber;
        v.category = params_.category;
        v.velocity = velocity;
        v.effVelocityGain = CrackedChimes::DSP::calculateVelocityGain(velocity, params_.ampVelocity * 0.01f);
        v.targetHz = noteHz;
        v.currentHz = (params_.voiceMode == 1 && activeVoiceCount_ > 0) ? v.currentHz : noteHz;
        v.cyborgChargeSpike = 1.0f;
        v.lpgState = 0.0f;

        const auto& rom = DigiStrikeFS2ROM::getInstance();
        const int cat = std::clamp(params_.category, 0, 10);
        const uint32_t mask = DigiStrikeFS2Fleet::sanitizeMask(params_.lineageMask);

        // 1. Stepped Random per-trigger offsets
        const bool isLegatoGlide = (params_.voiceMode == 1 && activeVoiceCount_ > 0 && v.active && !v.released);
        if (!isLegatoGlide) {
            for (int i = 0; i < 8; ++i) {
                v.randVal[static_cast<size_t>(i)] = nextFloat();
            }
            if (params_.voiceMode == 2) {
                // Golden-ratio quasi-Monte Carlo stratification for Poly chords (prevents stereo clumping)
                panPhase_ = std::fmod(panPhase_ + 0.6180339887f, 1.0f);
                v.randVal[6] = panPhase_;

                // Layer 1: Golden-ratio loop span stratification for Poly chords (prevents ping-pong lockstep)
                loopPhase_ = std::fmod(loopPhase_ + 0.6180339887f, 1.0f);
                const float loopSpread = (loopPhase_ - 0.5f) * 2.0f; // [-1.0, 1.0]
                const float jitterDepth = 0.15f + 0.15f * (params_.randT * 0.01f);
                v.loopSpanMult = 1.0f + jitterDepth * loopSpread;
            } else {
                v.loopSpanMult = 1.0f;
            }
            // Layer 2: Dedicated independent random draw for LENGTH (breaks headroom cancellation with START)
            v.randLen = (nextFloat() - 0.5f) * 2.0f;

            for (size_t i = 0; i < 8; ++i) {
                v.randBip[i] = (v.randVal[i] - 0.5f) * 2.0f;
            }
            latchedDraw_ = v.randVal;
        }

        const float effX    = CrackedChimes::DSP::stepRandomNorm(params_.macroX, params_.randX * 0.01f, v.randVal[0]);
        const float effY    = CrackedChimes::DSP::stepRandomNorm(params_.macroY, params_.randY * 0.01f, v.randVal[1]);
        const float effZ    = CrackedChimes::DSP::stepRandomNorm(params_.macroZ, params_.randZ * 0.01f, v.randVal[2]);
        const float effT    = CrackedChimes::DSP::stepRandomNorm(params_.macroT, params_.randT * 0.01f, v.randVal[3]);
        const float effAggr = CrackedChimes::DSP::stepRandomNorm(params_.velAggr, params_.randAggr * 0.01f, v.randVal[4]);
        const float effTimb = CrackedChimes::DSP::stepRandomNorm(params_.velTimb, params_.randTimb * 0.01f, v.randVal[5]);
        const float rDecay  = v.randBip[7] * (params_.randDecay * 0.01f);

        v.effMacroX = effX;
        v.effMacroT = effT;
        v.effMacroZ = effZ;
        v.effTimb = effTimb;

        const float initPan = std::clamp(params_.pan + v.randBip[6] * (params_.randPan * 0.01f), -1.0f, 1.0f);
        const float initAngle = (initPan + 1.0f) * 0.25f * 3.14159265f;
        v.panL = std::cos(initAngle);
        v.panR = std::sin(initAngle);
        v.targetPanL = v.panL;
        v.targetPanR = v.panR;

        // 2. Col 4: Wavetable Morph & Noise (Macro Z + Vel Timbre/Aggr)
        const float normMorph = std::clamp(params_.tableMorph * 0.01f, 0.0f, 1.0f);
        const float effMorph = tkdsp::formless::sequentialArbitrate(normMorph, effZ, 0.75f, effTimb, 0.70f, velocity);
        v.morph = effMorph;
        v.currentMorph = effMorph;
        v.isSustaining = false;
        v.noiseHpState = 0.0f;
        v.noiseRawPrev = 0.0f;

        const float normNoise = std::clamp(params_.noiseMix * 0.01f, 0.0f, 1.0f);
        v.effNoiseMix = tkdsp::formless::sequentialArbitrate(normNoise, effZ, 0.70f, effAggr, 0.70f, velocity);

        const auto fleetPair = DigiStrikeFS2Fleet::resolveVoicePair(rom, cat, mask, v.morph);
        v.voiceA = fleetPair.voiceA;
        v.voiceB = fleetPair.voiceB;
        v.morphFrac = fleetPair.morphFrac;
        v.slotIdx = cat * 20 + std::clamp(params_.lineage, 0, 19);

        // 3. Amplitude Envelope Decay & Curvature (Master Macro Decay & R-DECAY Fan-Out)
        const float effDecay = CrackedChimes::DSP::stepRandomNorm(params_.macroDecay, params_.randDecay * 0.01f, v.randVal[7]);
        const float macroDecayMult = tkdsp::formless::evalMacroMultiplier(effDecay);

        float ampDecaySeconds = std::clamp(params_.ampDecay * macroDecayMult, 0.005f, 5.0f);
        v.ampDecaySeconds = ampDecaySeconds;
        v.ampDecayRate = 1.0f / (ampDecaySeconds * sampleRate_);
        v.ampReleaseRate = 1.0f / (std::clamp(0.050f * macroDecayMult, 0.002f, 2.0f) * sampleRate_);
        v.isGated = (params_.sustainLevel > 2.0f);

        v.decayCurveActive = (std::abs(params_.decayCurve) > 0.01f);
        v.decayCurveExp = v.decayCurveActive ? FastMath::fastPow2(-params_.decayCurve * 1.5f) : 1.0f;

        // Durations & Native Sample Rates from Voice Meta
        const auto* metaA = rom.getVoice(v.voiceA);
        const auto* metaB = rom.getVoice(v.voiceB);
        double durA = (metaA != nullptr && metaA->sampleRate > 0) ? (static_cast<double>(metaA->sampleCount) / static_cast<double>(metaA->sampleRate)) : 0.5;
        double durB = (metaB != nullptr && metaB->sampleRate > 0) ? (static_cast<double>(metaB->sampleCount) / static_cast<double>(metaB->sampleRate)) : 0.5;
        double dEff = (1.0 - static_cast<double>(v.morphFrac)) * durA + static_cast<double>(v.morphFrac) * durB;
        double dMax = std::max(durA, durB);

        // 4. Col 3: START & LENGTH (Headroom-Proportional via effX)
        v.dEffSec = dEff;
        const float normStart = std::clamp(params_.startOffset / 500.0f, 0.0f, 1.0f);
        const float effStartNorm = tkdsp::formless::headroomLerp(normStart, effX, 0.85f);
        double startSec = std::min(static_cast<double>(effStartNorm * 500.0f) * 0.001, dEff * 0.95);
        v.tStartSec = startSec;
        v.tStartSm = static_cast<float>(startSec);

        const float normLen = std::clamp(params_.sampleLength * 0.01f, 0.0f, 1.0f);
        const float effLenNormBase = tkdsp::formless::headroomLerp(normLen, effX, 0.75f);
        // Layer 2: Break headroom cancellation with independent LENGTH S&H throw proportional to randX
        const float lenRandThrow = v.randLen * (params_.randX * 0.01f) * 0.35f;
        const float effLenNorm = std::clamp(effLenNormBase + lenRandThrow, 0.02f, 1.0f);
        if (effLenNorm <= 0.02f) {
            v.tEndSec = v.tStartSec; // single-cycle freeze mode
            v.tEndSm = static_cast<float>(v.tStartSec);
        } else {
            v.tEndSec = std::min(dEff, v.tStartSec + static_cast<double>(effLenNorm) * (dEff - v.tStartSec));
            if (v.tEndSec <= v.tStartSec + 0.005) v.tEndSec = v.tStartSec + 0.005;
            v.tEndSm = static_cast<float>(v.tEndSec);
        }

        const float T = params_.traverseRate;
        const float absT = std::abs(T);
        v.isReverse = params_.reverse;
        v.tTotalSec = dMax * std::max(0.02f, absT);

        // Traversal Anchor & Grain Initialization (100% Decoupled from MIDI Pitch!)
        const float targetTInit = std::max(0.02f, absT) * FastMath::fastPow2(2.0f * (effT - 0.5f));
        v.tSm = targetTInit;
        v.u = v.isReverse ? v.tEndSec : v.tStartSec;
        v.grainPhase = 0.5;
        double initDir = v.isReverse ? -1.0 : 1.0;
        v.gDir[0] = static_cast<float>(initDir);
        v.gDir[1] = static_cast<float>(initDir);
        v.gAnchor[0] = v.u - initDir * 0.5 * static_cast<double>(grainLengthSec_) * static_cast<double>(v.staticPitchRatio);
        v.gAnchor[1] = v.u;
        v.transientRemaining = 0;
        v.sampleCountTrigger = 0;

        v.playheadSec = v.u;
        v.oscPhase = 0.0;

        // Tuning & Frequency Tracking
        uint8_t rootMidi = (metaA != nullptr) ? metaA->rootMidi : 36;
        v.rootHz = 440.0f * FastMath::fastPow2((static_cast<float>(rootMidi) - 69.0f) * (1.0f / 12.0f));
        float semitones = static_cast<float>(noteNumber - rootMidi);
        v.staticPitchRatio = FastMath::fastPow2(std::clamp(semitones, -24.0f, 24.0f) * (1.0f / 12.0f));

        // Layer 3: Lifeform Invariant (Strict 1.0f Pitch-Decoupled Traversal)
        // Across Synthetic, Cyborg, and Organic, traversal clock is 100% decoupled from MIDI pitch.
        // Vintage character is delivered authentically via AMD Am6070 non-linear companding,
        // SP-1200 12-bit linear DAC + 26.04 kHz ZOH clocking, and vactrol LPG slew.
        v.travPitchScale = 1.0f;

        // 5. Col 1: Pitch Envelope P.DEC (Headroom Lerp via effT & Master Macro Decay scaling)
        const float normPDec = std::clamp((params_.pitchDecay - 2.0f) / (250.0f - 2.0f), 0.0f, 1.0f);
        const float effPDecNorm = tkdsp::formless::headroomLerp(normPDec, effT, 0.70f);
        const float basePDecMs = 2.0f + effPDecNorm * (250.0f - 2.0f);
        const float pDecayMs = std::clamp(basePDecMs * std::sqrt(macroDecayMult), 1.0f, 500.0f);
        v.pitchEnvRate = 1.0f / (pDecayMs * 0.001f * sampleRate_);
        v.pitchEnv = 1.0f;

        // 6. Col 2: PUNCH & CLICK (Sequential Arbitrate via effY)
        const float normDive = std::clamp(params_.pitchDive * 0.01f, 0.0f, 1.0f);
        const float effDive = tkdsp::formless::sequentialArbitrate(normDive, effY, 0.80f, effAggr, 0.75f, velocity);
        v.pitchDiveSemitones = effDive * 36.0f;

        const float normClick = std::clamp(params_.clickBleed * 0.01f, 0.0f, 1.0f);
        v.effClickBleed = tkdsp::formless::sequentialArbitrate(normClick, effY, 0.75f, effTimb, 0.65f, velocity);

        v.ampEnv = 1.0f;
        v.sp12ClockPhase = 0.0;
        v.zohSampledVal = 0.0f;

        v.ampDecaySeconds = ampDecaySeconds;
        v.effMacroT = effT;
        v.loopMode = params_.loopMode;
        v.inReversePhase = false;
        v.ppgDirection = 1;
        v.prevTraverseRate = params_.traverseRate;
    }

    void triggerMasterFilterEnv(float velocity) noexcept {
        masterFilterEnv_ = std::max(masterFilterEnv_, velocity);
    }

    void renderVoice(VoiceSlot& v, float* accL, float* accR, int numSamples, float targetMorph, float invNumSamples) noexcept {
        const float effectivePan = std::clamp(params_.pan + v.randBip[6] * (params_.randPan * 0.01f), -1.0f, 1.0f);
        const float panAngle = (effectivePan + 1.0f) * 0.25f * 3.14159265f;
        v.targetPanL = std::cos(panAngle);
        v.targetPanR = std::sin(panAngle);
        v.panL += (v.targetPanL - v.panL) * 0.2f;
        v.panR += (v.targetPanR - v.panR) * 0.2f;
        const float panL = v.panL;
        const float panR = v.panR;

        const float glideCoeff = (params_.glideTime > 0.0005f)
            ? (1.0f - std::exp(-invSampleRate_ / std::max(0.001f, params_.glideTime)))
            : 1.0f;

        const float morphInc = (targetMorph - v.currentMorph) * invNumSamples;
        const auto& rom = DigiStrikeFS2ROM::getInstance();
        const auto* metaA = rom.getVoice(v.voiceA);
        const auto* metaB = rom.getVoice(v.voiceB);

        // Dynamic held-note amplitude decay tracking (Master Macro Decay live sweep)
        const float liveMacroMult = tkdsp::formless::evalMacroMultiplier(params_.macroDecay);
        const float targetDecaySec = std::clamp(params_.ampDecay * liveMacroMult, 0.005f, 5.0f);
        const float targetAmpRate = 1.0f / (targetDecaySec * sampleRate_);
        v.ampDecayRate += 0.05f * (targetAmpRate - v.ampDecayRate);

        for (int i = 0; i < numSamples; ++i) {
            v.currentMorph += morphInc;            // 1. Dynamic Boundary Smoothing (Live Held-Note Tracking)
            const double dEff = v.dEffSec;
            const float normStart = std::clamp(params_.startOffset / 500.0f, 0.0f, 1.0f);
            const float effStartNorm = tkdsp::formless::headroomLerp(normStart, v.effMacroX, 0.85f);
            const float aT = std::min(static_cast<float>(effStartNorm * 500.0f * 0.001f), static_cast<float>(dEff * 0.95));

            const float normLen = std::clamp(params_.sampleLength * 0.01f, 0.0f, 1.0f);
            const float effLenNormBase = tkdsp::formless::headroomLerp(normLen, v.effMacroX, 0.75f);
            const float lenRandThrow = v.randLen * (params_.randX * 0.01f) * 0.35f;
            const float effLenNorm = std::clamp(effLenNormBase + lenRandThrow, 0.02f, 1.0f);
            const float bT = (effLenNorm <= 0.02f) ? aT : std::max(aT + 0.005f, aT + effLenNorm * static_cast<float>(dEff - aT));

            v.tStartSm += kEdgeCoef_ * (aT - v.tStartSm);
            v.tEndSm   += kEdgeCoef_ * (bT - v.tEndSm);
            const double a = v.tStartSm;
            const double b = v.tEndSm;
            const double baseS = std::max(0.005, b - a);
            const double S = std::clamp(baseS * static_cast<double>(v.loopSpanMult), 0.005, std::max(0.005, dEff - a));

            // 2. Pitch Gliding & Pitch Ratio (100% Decoupled from Traversal!)
            if (params_.voiceMode == 1) {
                v.currentHz += (v.targetHz - v.currentHz) * glideCoeff;
            } else {
                v.currentHz = v.targetHz;
            }
            float pitchModSemitones = v.pitchDiveSemitones * (v.pitchEnv * v.pitchEnv);
            float punchRatio = FastMath::fastPow2(pitchModSemitones * (1.0f / 12.0f));
            float r = std::clamp(v.currentHz / std::max(10.0f, v.rootHz), 0.0078125f, 128.0f) * punchRatio;

            // 3. Traversal Speed & Direction (100% Decoupled from Audio Pitch Ratio!)
            const float T = params_.traverseRate;
            const float absT = std::max(0.02f, std::abs(T));
            const float macroTMod = std::pow(2.0f, 2.0f * (v.effMacroT - 0.5f));
            const float targetT = absT * macroTMod;
            v.tSm += kEdgeCoef_ * (targetT - v.tSm);

            // Decoupled Traversal Rate Invariant:
            // Traversal speed s is governed SOLELY by TIME (T) and MACRO-T.
            // Unipolar duration scaling: 1.0x = nominal sample speed (1s source = 1s wall-clock).
            // 2.0x = takes twice as long (half traversal speed: s = 0.5).
            // 0.5x = takes half as long (double traversal speed: s = 2.0).
            // ZERO dependency on audio pitch ratio r or MIDI note!
            const double s = 1.0 / static_cast<double>(std::max(0.02f, v.tSm));
            const int mode = params_.loopMode;

            double d = params_.reverse ? -1.0 : 1.0;
            v.u += d * s * invSampleRate_;

            // 4. Stateless Loop Evaluation for Traversal Coordinate pos(u)
            auto wrapFn = [](double val, double span) noexcept {
                return val - span * std::floor(val / span);
            };
            const double x = v.u - a;
            double pos = a;
            switch (mode) {
                case 0: // ONCE (True One-Shot Clean Release)
                default:
                    pos = v.u;
                    if (d > 0.0 ? (v.u >= b) : (v.u <= a)) {
                        v.isFadingOut = true;
                    }
                    break;
                case 1: // HOLD (Last Frame Freeze / Sustain)
                    pos = std::clamp(v.u, a, b);
                    v.isSustaining = true;
                    break;
                case 2: // LOOP (Forward Looped)
                    pos = a + wrapFn(x, S);
                    break;
                case 3: // PING (Ping-Pong Looped)
                    pos = a + S - std::abs(wrapFn(x, 2.0 * S) - S);
                    break;
            }

            // 5. Audio Generation: Single-Cycle Freeze vs 2-Grain Unified OLA
            float sampleVal = 0.0f;
            if (params_.sampleLength <= 2.0f) {
                // Single-cycle freeze mode: audio oscillator
                double oscInc = static_cast<double>(v.currentHz * punchRatio) * invSampleRate_;
                v.oscPhase += oscInc;
                if (v.oscPhase >= 1.0) v.oscPhase = std::fmod(v.oscPhase, 1.0);

                uint32_t cycleA = (metaA != nullptr) ? std::min(2048u, metaA->sampleCount) : 2048u;
                double pA = v.oscPhase * static_cast<double>(cycleA);
                sampleVal = (metaA != nullptr) ? rom.sampleHermite(v.voiceA, pA) : 0.0f;
            } else {
                // Advance grain phase
                double dPhi = invGrainLengthSamples_;
                v.grainPhase += dPhi;
                if (v.grainPhase >= 1.0) v.grainPhase -= 1.0;

                const double phi0 = v.grainPhase;
                const double phi1 = std::fmod(v.grainPhase + 0.5, 1.0);

                // Latch anchors at wraps
                if (phi0 < dPhi) {
                    v.gAnchor[0] = pos;
                    v.gDir[0] = (mode == 3) ? ((wrapFn(x, 2.0 * S) < S) ? 1.0f : -1.0f) : static_cast<float>(d);
                }
                if (phi1 < dPhi) {
                    v.gAnchor[1] = pos;
                    v.gDir[1] = (mode == 3) ? ((wrapFn(x, 2.0 * S) < S) ? 1.0f : -1.0f) : static_cast<float>(d);
                }

                // Partition-of-unity Hann Overlap-Add (w0 + w1 ≡ 1.0)
                float w0 = 0.5f * (1.0f - std::cos(static_cast<float>(phi0 * 6.283185307179586)));
                float w1 = 1.0f - w0;

                // Red Team Invariant: Source-Domain Tail Fade (Anti-Buzz & Anti-Click)
                const double F = 0.002 * static_cast<double>(r); // 2ms fade window in source seconds

                auto evalGrain = [&](int g, double phi) noexcept -> float {
                    double rawP = v.gAnchor[g] + static_cast<double>(v.gDir[g]) * static_cast<double>(r) * (phi * static_cast<double>(grainLengthSec_));
                    float grainFade = 1.0f;
                    double clampedP = rawP;

                    if (mode == 2) {
                        clampedP = a + wrapFn(rawP - a, S); // LOOP (Forward Looped)
                    } else if (mode == 3) {
                        clampedP = a + S - std::abs(wrapFn(rawP - a, 2.0 * S) - S); // PING (Ping-Pong)
                    } else {
                        // ONCE (0) or HOLD (1)
                        if (rawP >= b) {
                            return 0.0f; // Beyond end, fully faded out
                        } else if (rawP > b - F && F > 1e-6) {
                            float normFade = static_cast<float>((rawP - (b - F)) / F);
                            grainFade = 0.5f * (1.0f + std::cos(normFade * 3.141592653589793f));
                        }
                        if (rawP <= a) {
                            if (d < 0.0) return 0.0f;
                            clampedP = a;
                        } else {
                            clampedP = rawP;
                        }
                    }

                    if (metaA == nullptr) return 0.0f;
                    double pA = clampedP * static_cast<double>(metaA->sampleRate);
                    float sA = rom.sampleHermite(v.voiceA, pA) * grainFade;
                    if (metaB == nullptr || v.morphFrac <= 0.001f) return sA;
                    double pB = clampedP * static_cast<double>(metaB->sampleRate);
                    float sB = rom.sampleHermite(v.voiceB, pB) * grainFade;
                    return (1.0f - v.morphFrac) * sA + v.morphFrac * sB;
                };

                sampleVal = w0 * evalGrain(0, phi0) + w1 * evalGrain(1, phi1);
            }

            // 6. Lifeform Character Mojo (The Triad Sonic Canon)
            switch (params_.lifeMode) {
                case 0: // Synthetic: Clean Virtual Analog Bypass (Null test <= -90 dBFS)
                    break;
                case 1: { // Cyborg: Nuclear Reactor Waste (8-bit linear DAC + diode foldback + charge spike)
                    if (v.cyborgChargeSpike > 0.001f) {
                        sampleVal += v.cyborgChargeSpike * (nextFloat() - 0.5f) * 0.15f;
                        v.cyborgChargeSpike *= 0.96f;
                    }
                    sampleVal = std::clamp(std::round(sampleVal * 128.0f), -128.0f, 127.0f) * (1.0f / 128.0f);
                    if (sampleVal > 0.85f) {
                        sampleVal = 0.85f - (sampleVal - 0.85f) * 0.5f;
                    } else if (sampleVal < -0.85f) {
                        sampleVal = -0.85f - (sampleVal + 0.85f) * 0.5f;
                    }
                    break;
                }
                case 2: { // Organic: Sloshy Tube-Back Warmth with VTL5C3 LPG droop
                    const float x = sampleVal * 1.35f;
                    sampleVal = (x + 0.28f * x * x) / (1.0f + 0.28f * std::abs(x));
                    const float lpgCoeff = (std::abs(sampleVal) > std::abs(v.lpgState)) ? lpgAttCoeff_ : lpgRelCoeff_;
                    v.lpgState += lpgCoeff * (sampleVal - v.lpgState);
                    sampleVal = v.lpgState;
                    break;
                }
            }

            // 7. Transient Impact Click Bleed & Upper Punch Dive Bleed
            float excessPunch = std::max(0.0f, pitchModSemitones - 7.0f);
            if (v.pitchEnv > 0.8f && (v.effClickBleed > 0.001f || excessPunch > 0.0f)) {
                float clickScale = v.effClickBleed + (excessPunch * (1.0f / 29.0f) * 0.25f);
                float click = (nextFloat() - 0.5f) * clickScale;
                sampleVal += click;
            }

            // 8. Enveloped Noise Mix (Dynamic Arbitrated Throw)
            if (v.effNoiseMix > 0.001f) {
                float rawNoise = 2.0f * (nextFloat() - 0.5f);
                float hpNoise = rawNoise - v.noiseRawPrev + 0.85f * v.noiseHpState;
                v.noiseRawPrev = rawNoise;
                v.noiseHpState = hpNoise;
                if (std::abs(v.noiseHpState) < 1e-15f) v.noiseHpState = 0.0f;

                float envNoise = v.ampEnv * (0.35f + 0.65f * v.pitchEnv);
                sampleVal += hpNoise * envNoise * v.effNoiseMix;
            }

            // 9. Amplitude Gain Scaling & Anti-Click Fadeout
            if (v.isFadingOut) {
                v.fadeOutEnv -= v.fadeOutRate;
                if (v.fadeOutEnv <= 0.0f) {
                    v.active = false;
                    break;
                }
                sampleVal *= v.fadeOutEnv;
            }

            // Decay Curvature Shaping on Amplitude
            float shapedAmp = v.ampEnv;
            if (v.decayCurveActive && v.ampEnv > 0.0001f) {
                shapedAmp = FastMath::fastPow2(FastMath::fastLog2(std::max(v.ampEnv, 1e-6f)) * v.decayCurveExp);
            }
            float gain = shapedAmp * v.chokeEnv * v.effVelocityGain;
            float outSample = sampleVal * gain;

            accL[i] += outSample * panL;
            accR[i] += outSample * panR;

            // 10. Envelope Updates
            if (v.pitchEnv > 0.0f) {
                v.pitchEnv -= v.pitchEnvRate;
                if (v.pitchEnv < 0.0f) v.pitchEnv = 0.0f;
            }

            if (v.chokeActive) {
                v.chokeEnv -= v.chokeRate;
                if (v.chokeEnv <= 0.0f) {
                    v.active = false;
                    break;
                }
            }

            if (v.isGated) {
                const float targetAmp = params_.sustainLevel * 0.01f;
                if (!v.released && v.ampEnv > targetAmp) {
                    v.ampEnv = std::max(targetAmp, v.ampEnv - v.ampDecayRate);
                }
                if (v.released) {
                    v.ampEnv -= v.ampReleaseRate;
                    if (v.ampEnv <= 0.0005f) {
                        v.active = false;
                        break;
                    }
                }
            } else {
                v.ampEnv -= v.ampDecayRate;
                if (v.ampEnv <= 0.0f) {
                    v.active = false;
                    break;
                }
            }
        }
    }

    void processMasterBus(float* outL, float* outR, int numSamples) noexcept {
        float masterGain = params_.volume * (1.0f + params_.macroY * 0.25f);
        if (params_.volume <= 0.0001f) masterGain = 0.0f;

        const float macroDecayMultBus = tkdsp::formless::evalMacroMultiplier(params_.macroDecay);
        const float effFilterDecaySec = std::clamp(params_.filterDecay * macroDecayMultBus, 0.005f, 5.0f);
        const float feDecayCoeff = FastMath::fastExp(-invSampleRate_ / effFilterDecaySec);

        // Empyrean-SSM Q-Compensated Saturated ZDF Filter
        const float res = std::clamp(params_.resonance, 0.0f, 0.98f);
        const float damping = 2.0f * (1.0f - res);
        const float gBassComp = 1.0f + 0.55f * res;

        constexpr int kSub = 16;
        for (int i0 = 0; i0 < numSamples; i0 += kSub) {
            const int n = std::min(kSub, numSamples - i0);

            // Dynamic Filter Envelope Modulation (0 .. +10.0 Octaves) with Macro X & Rand X
            const float envOctaves = std::clamp(params_.filterEnvAmt * 10.0f * masterFilterEnv_, 0.0f, 10.5f);
            const float effCutoffMacroX = std::clamp(params_.macroX + (latchedDraw_[0] - 0.5f) * 2.0f * (params_.randX * 0.01f), 0.0f, 1.0f);
            float cutoffHz = params_.cutoff * FastMath::fastPow2(envOctaves + (effCutoffMacroX - 0.5f) * 2.0f);
            cutoffHz = std::clamp(cutoffHz, 20.0f, 20000.0f);

            const float w0 = 2.0f * 3.14159265f * cutoffHz * invSampleRate_;
            const float g = std::tan(w0 * 0.5f);
            const float denom = 1.0f / (1.0f + damping * g + g * g);

            for (int i = i0; i < i0 + n; ++i) {
                float inL = outL[i];
                float inR = outR[i];

                if (params_.preDrive > 0.01f) {
                    float driveGain = 1.0f + params_.preDrive * 4.0f;
                    inL = std::tanh(inL * driveGain);
                    inR = std::tanh(inR * driveGain);
                }

                // Left Channel
                float s1L = masterFilterState_[0];
                float s2L = masterFilterState_[1];
                if (res > 0.05f) {
                    s1L = std::tanh(s1L * 1.5f) * (1.0f / 1.5f);
                }
                float hpL = (inL - (damping + g) * s1L - s2L) * denom;
                float bpL = g * hpL + s1L;
                if (res > 0.05f) {
                    bpL = std::tanh(bpL * 1.5f) * (1.0f / 1.5f);
                }
                float lpL = g * bpL + s2L;
                masterFilterState_[0] = g * hpL + bpL;
                masterFilterState_[1] = g * bpL + lpL;

                // Right Channel
                float s1R = masterFilterState_[2];
                float s2R = masterFilterState_[3];
                if (res > 0.05f) {
                    s1R = std::tanh(s1R * 1.5f) * (1.0f / 1.5f);
                }
                float hpR = (inR - (damping + g) * s1R - s2R) * denom;
                float bpR = g * hpR + s1R;
                if (res > 0.05f) {
                    bpR = std::tanh(bpR * 1.5f) * (1.0f / 1.5f);
                }
                float lpR = g * bpR + s2R;
                masterFilterState_[2] = g * hpR + bpR;
                masterFilterState_[3] = g * bpR + lpR;

                // 4-Mode Output Matrix (LP, HP, BP, PK)
                float filtL = lpL * gBassComp;
                float filtR = lpR * gBassComp;
                if (params_.filterType == 1) {
                    filtL = hpL;
                    filtR = hpR;
                } else if (params_.filterType == 2) {
                    filtL = bpL * (1.0f + res);
                    filtR = bpR * (1.0f + res);
                } else if (params_.filterType == 3) {
                    filtL = (lpL - 2.0f * bpL + hpL) * 0.707f;
                    filtR = (lpR - 2.0f * bpR + hpR) * 0.707f;
                }

                filtL *= masterGain;
                filtR *= masterGain;

                // Crush: Machinedrum S&H Clock Decimation
                if (params_.crush < 47500.0f) {
                    crushPhase_ += params_.crush * invSampleRate_;
                    if (crushPhase_ >= 1.0f) {
                        crushPhase_ -= std::floor(crushPhase_);
                        crushedSampleL_ = filtL;
                        crushedSampleR_ = filtR;
                    }
                    filtL = crushedSampleL_;
                    filtR = crushedSampleR_;
                }

                if (std::abs(params_.shift) > 0.5f) {
                    filtL = freqShifter_.process(filtL, params_.shift);
                    filtR = freqShifter_.process(filtR, params_.shift);
                }

                // Squash: Bipolar Dynamics Manifold (-1 snappy expansion .. 0 bypass .. +1 distressor nuke)
                if (std::abs(params_.squash) > 0.005f) {
                    squashStage_.process(filtL, filtR, params_.squash);
                }

                if (std::abs(params_.wider) > 0.005f) {
                    widerStage_.process(filtL, filtR, params_.wider, params_.category);
                }

                if (params_.masterLimiter) {
                    drumLimiter_.process(filtL, filtR, true);
                }

                outL[i] = filtL;
                outR[i] = filtR;
            }

            // Sub-block envelope decay advance
            masterFilterEnv_ *= std::pow(feDecayCoeff, static_cast<float>(n));
            if (masterFilterEnv_ < 1e-7f) masterFilterEnv_ = 0.0f;
        }
    }

    float sampleRate_{48000.0f};
    float invSampleRate_{1.0f / 48000.0f};
    float grainLengthSec_{0.030f};
    double grainLengthSamples_{1440.0};
    double invGrainLengthSamples_{1.0 / 1440.0};
    float kEdgeCoef_{0.004f};
    DigiStrikeFS2Params params_{};

    std::array<VoiceSlot, kMaxVoices> voices_{};
    int activeVoiceCount_{0};
    int lastNoteNumber_{60};
    float lastVelocity_{0.9f};
    float glideHz_{130.81f};

    // Hastur Note-Priority Stack Architecture
    std::array<NoteEntry, 16> noteStack_{};
    int noteStackSize_{0};
    int activeNoteCount_{0};
    int64_t currentSampleCount_{0};
    int64_t lastNoteOffSample_{0};
    float lastPitchHz_{130.81f};
    bool sustainPedal_{false};

    std::array<float, 4> masterFilterState_{{0.0f, 0.0f, 0.0f, 0.0f}};
    float masterFilterEnv_{0.0f};

    uint32_t rngState_{0xDEADBEEFu};
    float panPhase_{0.0f};
    float loopPhase_{0.0f};
    FreqShifter freqShifter_{};
    WiderStage widerStage_{};
    DrumLimiter drumLimiter_{};
    SquashStage squashStage_{};
    float crushPhase_{0.0f};
    float crushedSampleL_{0.0f};
    float crushedSampleR_{0.0f};
    float lpgAttCoeff_{0.0f};
    float lpgRelCoeff_{0.0f};
    std::array<float, 8> latchedDraw_{{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};
};

} // namespace tkdsp
