#pragma once

#include "FastMath.h"
#include "FormlessModMath.h"
#include "../dynamics/WiderStage.h"
#include "../dynamics/DrumLimiter.h"
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
    int category{9};         // 0=BD, 1=SN, 2=TM, 3=CP, 4=HH, 5=CY, 6=LN, 7=MC, 8=FX, 9=ST (Default: ST)
    int lineage{14};         // 0=CROWN .. 19=CURIOS (Default: 14=Fairlight)
    uint32_t lineageMask{1u << 14}; // 20-bit multi-select mask (bit 14=Fairlight default)
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
    float tableMorph{0.0f};  // 0.0 .. 100.0 % (MORPH, 3D Spectral Slice)
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

    float cutoff{20000.0f};  // 20.0 .. 20000.0 Hz
    float resonance{0.30f};  // 0.0 .. 1.0
    float filterEnvAmt{0.0f};// -1.0 .. +1.0
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

        reset();
    }

    [[nodiscard]] const std::array<float, 8>& getLatchedDraws() const noexcept { return latchedDraw_; }

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
            v.isFadingOut = false;
            v.fadeOutEnv = 1.0f;
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
        v.targetHz = noteHz;
        v.currentHz = (params_.voiceMode == 1 && activeVoiceCount_ > 0) ? v.currentHz : noteHz;

        const auto& rom = DigiStrikeFS2ROM::getInstance();
        const int cat = std::clamp(params_.category, 0, 9);
        const uint32_t mask = DigiStrikeFS2Fleet::sanitizeMask(params_.lineageMask);

        v.morph = std::clamp(params_.tableMorph * 0.01f, 0.0f, 1.0f);
        v.currentMorph = v.morph;
        v.isSustaining = false;
        v.noiseHpState = 0.0f;
        v.noiseRawPrev = 0.0f;

        const auto fleetPair = DigiStrikeFS2Fleet::resolveVoicePair(rom, cat, mask, v.morph);
        v.voiceA = fleetPair.voiceA;
        v.voiceB = fleetPair.voiceB;
        v.morphFrac = fleetPair.morphFrac;
        v.slotIdx = cat * 20 + std::clamp(params_.lineage, 0, 19);

        // Stepped Random per-trigger offsets
        for (int i = 0; i < 8; ++i) {
            latchedDraw_[static_cast<size_t>(i)] = nextFloat();
        }
        float rX = (latchedDraw_[0] - 0.5f) * 2.0f * (params_.randX * 0.01f);
        float rY = (latchedDraw_[1] - 0.5f) * 2.0f * (params_.randY * 0.01f);
        float rZ = (latchedDraw_[2] - 0.5f) * 2.0f * (params_.randZ * 0.01f);
        float rT = (latchedDraw_[3] - 0.5f) * 2.0f * (params_.randT * 0.01f);
        float rDecay = (latchedDraw_[7] - 0.5f) * 2.0f * (params_.randDecay * 0.01f);

        float effMacroX = std::clamp(params_.macroX + rX, 0.0f, 1.0f);
        float effMacroY = std::clamp(params_.macroY + rY, 0.0f, 1.0f);
        float effMacroZ = std::clamp(params_.macroZ + rZ, 0.0f, 1.0f);
        float effMacroT = std::clamp(params_.macroT + rT, 0.0f, 1.0f);

        // Effective one-shot duration derivation
        float ampDecaySeconds = params_.ampDecay * (0.6f + 0.8f * params_.macroDecay);
        ampDecaySeconds = std::clamp(ampDecaySeconds * (1.0f + rDecay * 0.35f), 0.005f, 5.0f);
        v.ampDecayRate = 1.0f / (ampDecaySeconds * sampleRate_);
        v.ampReleaseRate = 1.0f / (0.050f * sampleRate_);
        v.isGated = (params_.sustainLevel > 2.0f);

        // Durations & Native Sample Rates from Voice Meta
        const auto* metaA = rom.getVoice(v.voiceA);
        const auto* metaB = rom.getVoice(v.voiceB);
        double durA = (metaA != nullptr && metaA->sampleRate > 0) ? (static_cast<double>(metaA->sampleCount) / static_cast<double>(metaA->sampleRate)) : 0.5;
        double durB = (metaB != nullptr && metaB->sampleRate > 0) ? (static_cast<double>(metaB->sampleCount) / static_cast<double>(metaB->sampleRate)) : 0.5;
        double dEff = (1.0 - static_cast<double>(v.morphFrac)) * durA + static_cast<double>(v.morphFrac) * durB;
        double dMax = std::max(durA, durB);

        // Temporal Traversal Setup & 2-Grain OLA Initialization
        v.dEffSec = dEff;
        double startSec = std::min(static_cast<double>(params_.startOffset) * 0.001, dEff * 0.95);
        v.tStartSec = startSec;
        v.tStartSm = static_cast<float>(startSec);

        float lenNorm = std::clamp(params_.sampleLength * 0.01f, 0.0f, 1.0f);
        if (lenNorm <= 0.02f) {
            v.tEndSec = v.tStartSec; // single-cycle freeze mode
            v.tEndSm = static_cast<float>(v.tStartSec);
        } else {
            v.tEndSec = std::min(dEff, v.tStartSec + static_cast<double>(lenNorm) * (dEff - v.tStartSec));
            if (v.tEndSec <= v.tStartSec + 0.005) v.tEndSec = v.tStartSec + 0.005;
            v.tEndSm = static_cast<float>(v.tEndSec);
        }

        const float T = params_.traverseRate;
        const float absT = std::abs(T);
        v.isReverse = params_.reverse;
        v.tTotalSec = dMax * std::max(0.02f, absT);

        // Traversal Anchor & Grain Initialization
        v.u = v.isReverse ? v.tEndSec : v.tStartSec;
        v.grainPhase = 0.0;
        v.gAnchor[0] = v.u;
        v.gDir[0] = v.isReverse ? -1.0f : 1.0f;
        // Grain 1 initialized at virtual pre-history u - 0.5L for bit-accurate null invariant
        v.gAnchor[1] = v.u - 0.5 * static_cast<double>(grainLengthSec_);
        v.gDir[1] = v.gDir[0];

        // 15ms Pure-Impact Transient Gate (Bypassed on reverse so OLA sweeps smoothly backward to terminal crack)
        if (v.isReverse) {
            v.transientRemaining = 0;
            v.sampleCountTrigger = 0;
        } else {
            v.transientRemaining = static_cast<int>(0.015f * sampleRate_);
            v.sampleCountTrigger = v.transientRemaining;
        }

        v.playheadSec = v.u;
        v.oscPhase = 0.0;

        // Tuning & Frequency Tracking
        uint8_t rootMidi = (metaA != nullptr) ? metaA->rootMidi : 36;
        v.rootHz = 440.0f * FastMath::fastPow2((static_cast<float>(rootMidi) - 69.0f) * (1.0f / 12.0f));
        float semitones = static_cast<float>(noteNumber - rootMidi);
        v.staticPitchRatio = FastMath::fastPow2(std::clamp(semitones, -24.0f, 24.0f) * (1.0f / 12.0f));

        // Pitch Envelope: P.DEC & PUNCH
        float pDecayMs = params_.pitchDecay;
        v.pitchEnvRate = 1.0f / (pDecayMs * 0.001f * sampleRate_);
        v.pitchEnv = 1.0f;
        v.pitchDiveSemitones = (params_.pitchDive * 0.01f) * 36.0f * (0.5f + effMacroY);

        v.ampEnv = 1.0f;
        v.sp12ClockPhase = 0.0;
        v.zohSampledVal = 0.0f;

        v.ampDecaySeconds = ampDecaySeconds;
        v.effMacroT = effMacroT;
        v.loopMode = params_.loopMode;
        v.inReversePhase = false;
        v.ppgDirection = 1;
        v.prevTraverseRate = params_.traverseRate;
    }

    void triggerMasterFilterEnv(float velocity) noexcept {
        masterFilterEnv_ = velocity;
    }

    void renderVoice(VoiceSlot& v, float* accL, float* accR, int numSamples, float targetMorph, float invNumSamples) noexcept {
        float panL = 0.7071f;
        float panR = 0.7071f;
        float effectivePan = std::clamp(params_.pan + (latchedDraw_[6] - 0.5f) * 2.0f * (params_.randPan * 0.01f), -1.0f, 1.0f);
        panL = std::cos((effectivePan + 1.0f) * 0.25f * 3.14159265f);
        panR = std::sin((effectivePan + 1.0f) * 0.25f * 3.14159265f);

        const float glideCoeff = (params_.glideTime > 0.0005f)
            ? (1.0f - std::exp(-invSampleRate_ / std::max(0.001f, params_.glideTime)))
            : 1.0f;

        const float morphInc = (targetMorph - v.currentMorph) * invNumSamples;
        const auto& rom = DigiStrikeFS2ROM::getInstance();
        const auto* metaA = rom.getVoice(v.voiceA);
        const auto* metaB = rom.getVoice(v.voiceB);

        for (int i = 0; i < numSamples; ++i) {
            v.currentMorph += morphInc;            // 1. Dynamic Boundary Smoothing (Live Held-Note Tracking)
            const double dEff = v.dEffSec;
            const float aT = std::min(params_.startOffset * 0.001f, static_cast<float>(dEff * 0.95));
            const float ln = std::clamp(params_.sampleLength * 0.01f, 0.0f, 1.0f);
            const float bT = std::max(aT + 0.005f, aT + ln * static_cast<float>(dEff - aT));

            v.tStartSm += kEdgeCoef_ * (aT - v.tStartSm);
            v.tEndSm   += kEdgeCoef_ * (bT - v.tEndSm);
            const double a = v.tStartSm;
            const double b = v.tEndSm;
            const double S = std::max(0.005, b - a);

            // 2. Pitch Gliding & Pitch Ratio (100% Decoupled from Traversal!)
            if (params_.voiceMode == 1) {
                v.currentHz += (v.targetHz - v.currentHz) * glideCoeff;
            } else {
                v.currentHz = v.targetHz;
            }
            float pitchModSemitones = v.pitchDiveSemitones * (v.pitchEnv * v.pitchEnv);
            float punchRatio = FastMath::fastPow2(pitchModSemitones * (1.0f / 12.0f));
            float r = std::clamp(v.currentHz / std::max(10.0f, v.rootHz), 0.125f, 8.0f) * punchRatio;

            // 3. Traversal Speed & Direction (100% Decoupled from Pitch!)
            const float T = params_.traverseRate;
            const float absT = std::max(0.02f, std::abs(T));
            const float macroTMod = std::pow(2.0f, 2.0f * (v.effMacroT - 0.5f));
            // Unipolar duration scaling: 1.0x = nominal, 2.0x = takes twice as long (half speed)
            const double s = 1.0 / static_cast<double>(std::max(0.02f, absT * macroTMod));
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
                    if (d > 0.0 ? (v.u >= b + static_cast<double>(r * grainLengthSec_)) : (v.u <= a - static_cast<double>(r * grainLengthSec_))) {
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

            // 5. Audio Generation: Single-Cycle Freeze vs 15ms Transient Gate vs 2-Grain OLA
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
                    v.gDir[0] = (mode == 2) ? ((wrapFn(x, 2.0 * S) < S) ? 1.0f : -1.0f) : static_cast<float>(d);
                }
                if (phi1 < dPhi) {
                    v.gAnchor[1] = pos;
                    v.gDir[1] = (mode == 2) ? ((wrapFn(x, 2.0 * S) < S) ? 1.0f : -1.0f) : static_cast<float>(d);
                }

                // 15ms Pure-Impact Transient Gate:
                if (v.transientRemaining > 0) {
                    const int elapsed = v.sampleCountTrigger - v.transientRemaining;
                    v.transientRemaining--;
                    // Pure 1:1 playback at attack: zero comb filtering, 100% transient punch
                    double pTrans = a + static_cast<double>(r) * (static_cast<double>(elapsed) * invSampleRate_);
                    double pA = (metaA != nullptr) ? (pTrans * static_cast<double>(metaA->sampleRate)) : 0.0;
                    sampleVal = (metaA != nullptr) ? rom.sampleHermite(v.voiceA, pA) : 0.0f;
                    if (metaB != nullptr && v.morphFrac > 0.001f) {
                        double pB = pTrans * static_cast<double>(metaB->sampleRate);
                        float sB = rom.sampleHermite(v.voiceB, pB);
                        sampleVal = (1.0f - v.morphFrac) * sampleVal + v.morphFrac * sB;
                    }
                } else {
                    // Steady-state 2-Grain Hann Overlap-Add
                    float w0 = 0.5f * (1.0f - std::cos(static_cast<float>(phi0 * 6.283185307179586)));
                    float w1 = 1.0f - w0;

                    auto evalGrain = [&](int g, double phi) noexcept -> float {
                        double rawP = v.gAnchor[g] + static_cast<double>(v.gDir[g]) * static_cast<double>(r) * (phi * static_cast<double>(grainLengthSec_));
                        // Hardening: clamp grain read within loop window [a, b]
                        double clampedP = rawP;
                        if (mode == 0 || mode == 1) clampedP = a + wrapFn(rawP - a, S);
                        else if (mode == 2) clampedP = a + S - std::abs(wrapFn(rawP - a, 2.0 * S) - S);
                        else clampedP = std::clamp(rawP, a, b);

                        if (metaA == nullptr) return 0.0f;
                        double pA = clampedP * static_cast<double>(metaA->sampleRate);
                        float sA = rom.sampleHermite(v.voiceA, pA);
                        if (metaB == nullptr || v.morphFrac <= 0.001f) return sA;
                        double pB = clampedP * static_cast<double>(metaB->sampleRate);
                        float sB = rom.sampleHermite(v.voiceB, pB);
                        return (1.0f - v.morphFrac) * sA + v.morphFrac * sB;
                    };

                    sampleVal = w0 * evalGrain(0, phi0) + w1 * evalGrain(1, phi1);
                }
            }

            // 6. Lifeform Character Mojo
            switch (params_.lifeMode) {
                case 0: // Synthetic: Clean bypass (Null test <= -90 dBFS)
                    break;
                case 1: { // Cyborg: Authentic AMD Am6070 8-Bit Companding DAC
                    constexpr float kMu = 255.0f;
                    constexpr float kInvLnMu = 1.0f / 5.5451774f; // 1 / ln(256)
                    const float sgn = (sampleVal < 0.0f) ? -1.0f : 1.0f;
                    const float mag = std::min(1.0f, std::abs(sampleVal));
                    const float compressed = std::log1p(kMu * mag) * kInvLnMu;
                    const float quantized = std::round(compressed * 127.0f) * (1.0f / 127.0f);
                    const float expanded = (std::exp(quantized * 5.5451774f) - 1.0f) * (1.0f / kMu);
                    sampleVal = sgn * expanded;
                    break;
                }
                case 2: { // Organic: E-mu SP-1200 12-Bit Quantization + 26.04 kHz ZOH
                    constexpr float kScale = 2048.0f;
                    constexpr float kInvScale = 1.0f / 2048.0f;
                    sampleVal = std::clamp(std::round(sampleVal * kScale), -kScale, kScale - 1.0f) * kInvScale;

                    v.sp12ClockPhase += 26040.0 * invSampleRate_;
                    if (v.sp12ClockPhase >= 1.0) {
                        v.sp12ClockPhase -= 1.0;
                        v.zohSampledVal = sampleVal;
                    }
                    sampleVal = v.zohSampledVal;
                    break;
                }
            }

            // 7. Transient Impact Click Bleed & Upper Punch Dive Bleed
            float excessPunch = std::max(0.0f, pitchModSemitones - 7.0f);
            if (v.pitchEnv > 0.8f && (params_.clickBleed > 0.0f || excessPunch > 0.0f)) {
                float clickScale = (params_.clickBleed * 0.01f) + (excessPunch * (1.0f / 29.0f) * 0.25f);
                float click = (nextFloat() - 0.5f) * clickScale;
                sampleVal += click;
            }

            // 8. Enveloped Noise Mix
            if (params_.noiseMix > 0.001f) {
                float rawNoise = 2.0f * (nextFloat() - 0.5f);
                float hpNoise = rawNoise - v.noiseRawPrev + 0.85f * v.noiseHpState;
                v.noiseRawPrev = rawNoise;
                v.noiseHpState = hpNoise;
                if (std::abs(v.noiseHpState) < 1e-15f) v.noiseHpState = 0.0f;

                float envNoise = v.ampEnv * (0.35f + 0.65f * v.pitchEnv);
                sampleVal += hpNoise * envNoise * (params_.noiseMix * 0.01f);
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

            float gain = v.ampEnv * v.chokeEnv * v.velocity;
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

        const float effFilterDecaySec = std::clamp(params_.filterDecay, 0.005f, 5.0f);
        const float feDecayCoeff = FastMath::fastExp(-invSampleRate_ / effFilterDecaySec);

        // Empyrean-SSM Q-Compensated Saturated ZDF Filter
        const float res = std::clamp(params_.resonance, 0.0f, 0.98f);
        const float damping = 2.0f * (1.0f - res);
        const float gBassComp = 1.0f + 0.55f * res;

        constexpr int kSub = 16;
        for (int i0 = 0; i0 < numSamples; i0 += kSub) {
            const int n = std::min(kSub, numSamples - i0);

            // Dynamic Filter Envelope Modulation (+/-5.0 Octaves)
            const float envOctaves = std::clamp(params_.filterEnvAmt * 5.0f * masterFilterEnv_, -6.0f, 6.0f);
            float cutoffHz = params_.cutoff * FastMath::fastPow2(envOctaves + (params_.macroX - 0.5f) * 2.0f);
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

                if (std::abs(params_.shift) > 0.5f) {
                    filtL = freqShifter_.process(filtL, params_.shift);
                    filtR = freqShifter_.process(filtR, params_.shift);
                }

                if (params_.wider > 0.01f) {
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
    FreqShifter freqShifter_{};
    WiderStage widerStage_{};
    DrumLimiter drumLimiter_{};
    std::array<float, 8> latchedDraw_{{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};
};

} // namespace tkdsp
