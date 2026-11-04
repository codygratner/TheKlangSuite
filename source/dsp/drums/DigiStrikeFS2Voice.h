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
    int category{7};         // 0=KICK, 1=SNAR, 2=TOMS, 3=CLAP, 4=HATS, 5=CYMB, 6=PERC, 7=STAB (Default: STAB)
    int lineage{4};          // 0=CROWN .. 19=CURIOS (Default: 4=K-WRK Kawai)
    uint32_t lineageMask{0x00000010}; // 20-bit multi-select mask (bit 4=K-WRK Kawai default)
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
    float traverseRate{1.0f};// -4.0 .. +4.0 x (TIME, 1.0 neutral, negative reverse)
    float pitchDive{0.0f};   // 0.0 .. 100.0 % (PUNCH, 0..36 semitones)
    float noiseMix{0.0f};    // 0.0 .. 100.0 % (NOISE, Enveloped Noise Mix)
    float sampleLength{100.0f}; // 0.0 .. 100.0 % (LENGTH, 0% = single-cycle freeze)
    float tableMorph{0.0f};  // 0.0 .. 100.0 % (MORPH, 3D Spectral Slice)
    int   loopMode{3};        // 0=FWD, 1=REV, 2=PPG, 3=OFF (Default: OFF)

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
            v.framePos = 0.0;
            v.frameInc = 0.0;
            v.ampEnv = 0.0f;
            v.pitchEnv = 0.0f;
            v.chokeEnv = 1.0f;
            v.chokeActive = false;
            v.loopMode = 3;
            v.inReversePhase = false;
            v.ppgDirection = 1;
            v.prevTraverseRate = 1.0f;
        }
        activeVoiceCount_ = 0;
        masterFilterState_.fill(0.0f);
        filterEnvState_ = 0.0f;
        filterEnvTarget_ = 0.0f;
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
        bool isSecretMode{false};
        uint8_t activeCount{0};
        std::array<DigiStrikeFS2Catalog::WaveCoord, 64> activeWaves{};
        float currentMorph{0.0f};

        bool isSustaining{false};
        float noiseHpState{0.0f};
        float noiseRawPrev{0.0f};

        int noteNumber{60};
        int category{0};
        int slotIdx{0}; // category * 8 + lineage (0..63)
        float morph{0.0f}; // 0.0 .. 1.0
        float velocity{0.9f};
        float targetHz{130.81f};
        float currentHz{130.81f};

        // Intra-frame oscillator phase accumulator [0.0 .. 2047.999]
        double oscPhase{0.0};

        // Temporal wavetable frame traversal playhead [0.0 .. 3.0]
        double framePos{0.0};
        double frameInc{0.0};
        float startFrame{0.0f};
        float endFrame{3.0f};
        bool isReverse{false};

        float ampEnv{1.0f};
        float ampDecayRate{0.001f};
        float ampReleaseRate{0.005f};

        float pitchEnv{0.0f};
        float pitchEnvRate{0.01f};
        float pitchDiveSemitones{0.0f};

        float chokeEnv{1.0f};
        float chokeRate{0.05f};

        float zohSlewState{0.0f};

        int loopMode{3};
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
        v.noteNumber = noteNumber;
        v.category = params_.category;
        v.velocity = velocity;
        v.targetHz = noteHz;
        v.currentHz = (params_.voiceMode == 1 && activeVoiceCount_ > 0) ? v.currentHz : noteHz;

        // Map Category (0..7) and Lineage (0..19) to slotIdx (0..159)
        const int cat = std::clamp(params_.category, 0, 7);
        const int lin = std::clamp(params_.lineage, 0, 19);
        v.slotIdx = cat * 20 + lin;
        v.morph = std::clamp(params_.tableMorph * 0.01f, 0.0f, 1.0f);
        v.currentMorph = v.morph;
        v.isSustaining = false;
        v.noiseHpState = 0.0f;
        v.noiseRawPrev = 0.0f;

        // Multi-Lineage Active Wave Gathering & Secret Mode Detection:
        const uint32_t mask = params_.lineageMask;
        v.activeCount = 0;
        if (mask == 0) {
            v.isSecretMode = true;
        } else {
            v.isSecretMode = false;
            for (size_t i = 0; i < DigiStrikeFS2Catalog::kMusicalOrder[static_cast<size_t>(cat)].size(); ++i) {
                const auto coord = DigiStrikeFS2Catalog::kMusicalOrder[static_cast<size_t>(cat)][i];
                if ((mask & (1u << coord.lineage)) != 0 && v.activeCount < 64) {
                    v.activeWaves[v.activeCount++] = coord;
                }
            }
        }

        for (int i = 0; i < 8; ++i) {
            latchedDraw_[static_cast<size_t>(i)] = nextFloat();
        }

        // Stepped Random per-trigger offsets
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

        // Temporal Traversal Setup:
        // START: 0..500 ms -> maps to frame 0.0 .. 2.5
        float startNorm = std::clamp(params_.startOffset / 500.0f, 0.0f, 1.0f);
        v.startFrame = startNorm * 2.5f;

        // LENGTH: 0% = single-cycle freeze (length <= 2%), 100% = full 3.0 span
        float lenNorm = std::clamp(params_.sampleLength * 0.01f, 0.0f, 1.0f);
        if (lenNorm <= 0.02f) {
            v.endFrame = v.startFrame; // single-cycle frame freeze!
        } else {
            v.endFrame = std::clamp(v.startFrame + lenNorm * (3.0f - v.startFrame), v.startFrame + 0.1f, 3.0f);
        }

        // TIME: Inverted Duration Space [-8.0x .. +8.0x]
        // 8x takes 8x as long; values close to zero take less than a second;
        // at zero: 0 ms instant traversal, sits at destination frame immediately!
        const float T = params_.traverseRate;
        const float absT = std::abs(T);
        v.isReverse = (T < -0.01f);

        if (absT <= 0.02f) {
            // 0.0x: Instantaneous Traversal (0 ms)
            v.framePos = v.isReverse ? v.startFrame : v.endFrame;
            v.frameInc = 0.0;
        } else {
            const float macroTMod = std::pow(2.0f, 2.0f * (effMacroT - 0.5f));
            const double scanSeconds = std::max(0.001, static_cast<double>(ampDecaySeconds * absT * macroTMod));
            const double span = std::max(0.05, static_cast<double>(v.endFrame - v.startFrame));
            const double frameRate = span / (scanSeconds * sampleRate_);

            if (v.isReverse) {
                v.framePos = v.endFrame;
                v.frameInc = -frameRate;
            } else {
                v.framePos = v.startFrame;
                v.frameInc = frameRate;
            }
        }

        v.oscPhase = 0.0;

        // Pitch Envelope: P.DEC & PUNCH
        float pDecayMs = params_.pitchDecay;
        v.pitchEnvRate = 1.0f / (pDecayMs * 0.001f * sampleRate_);
        v.pitchEnv = 1.0f;
        v.pitchDiveSemitones = (params_.pitchDive * 0.01f) * 36.0f * (0.5f + effMacroY);

        v.ampEnv = 1.0f;
        v.zohSlewState = 0.0f;

        v.ampDecaySeconds = ampDecaySeconds;
        v.effMacroT = effMacroT;
        v.loopMode = params_.loopMode;
        v.inReversePhase = false;
        v.ppgDirection = 1;
        v.prevTraverseRate = params_.traverseRate;
    }

    void triggerMasterFilterEnv(float velocity) noexcept {
        filterEnvTarget_ = velocity * std::abs(params_.filterEnvAmt);
        filterEnvSlew_ = 0.0f;
    }

    void renderVoice(VoiceSlot& v, float* accL, float* accR, int numSamples, float targetMorph, float invNumSamples) noexcept {
        float panL = 0.7071f;
        float panR = 0.7071f;
        float effectivePan = std::clamp(params_.pan + (latchedDraw_[6] - 0.5f) * 2.0f * (params_.randPan * 0.01f), -1.0f, 1.0f);
        panL = std::cos((effectivePan + 1.0f) * 0.25f * 3.14159265f);
        panR = std::sin((effectivePan + 1.0f) * 0.25f * 3.14159265f);

        // Exponential portamento glide coefficient
        const float glideCoeff = (params_.glideTime > 0.0005f)
            ? (1.0f - std::exp(-invSampleRate_ / std::max(0.001f, params_.glideTime)))
            : 1.0f;

        const float morphInc = (targetMorph - v.currentMorph) * invNumSamples;
        const auto& rom = DigiStrikeFS2ROM::getInstance();

        for (int i = 0; i < numSamples; ++i) {
            v.currentMorph += morphInc;

            // 1. Pitch Gliding
            if (params_.voiceMode == 1) {
                v.currentHz += (v.targetHz - v.currentHz) * glideCoeff;
            } else {
                v.currentHz = v.targetHz;
            }

            // 2. Pitch Envelope & Intra-Frame Oscillator Accumulator
            // (100% DECOUPLED FROM TIME TRAVERSAL!)
            float pitchModSemitones = v.pitchDiveSemitones * (v.pitchEnv * v.pitchEnv);
            float pitchRatio = FastMath::fastPow2(pitchModSemitones * (1.0f / 12.0f));
            float freqHz = v.currentHz * pitchRatio;

            // Pitch phase advances at true audio rate: Delta theta = 2048 * (f / fs)
            double oscInc = (static_cast<double>(freqHz) * 2048.0) * invSampleRate_;
            v.oscPhase += oscInc;
            if (v.oscPhase >= 2048.0) {
                v.oscPhase = std::fmod(v.oscPhase, 2048.0);
            }

            // 3. Temporal Wavetable Frame Traversal Playhead
            const float liveStartNorm = std::clamp(params_.startOffset / 500.0f, 0.0f, 1.0f);
            v.startFrame = liveStartNorm * 2.5f;

            const float liveLenNorm = std::clamp(params_.sampleLength * 0.01f, 0.0f, 1.0f);
            v.endFrame = (liveLenNorm <= 0.02f)
                ? v.startFrame
                : std::clamp(v.startFrame + liveLenNorm * (3.0f - v.startFrame), v.startFrame + 0.05f, 3.0f);
            const double loopSpan = std::max(0.02, static_cast<double>(v.endFrame - v.startFrame));

            const float T = params_.traverseRate;
            const float absT = std::abs(T);
            const bool isTimeReversed = (T < -0.01f);

            // Detect live sign flip of TIME parameter:
            if ((T < -0.01f && v.prevTraverseRate >= -0.01f) || (T > 0.01f && v.prevTraverseRate <= 0.01f)) {
                if (v.loopMode == 2) {
                    v.ppgDirection = -v.ppgDirection;
                } else if (v.loopMode == 1) {
                    v.inReversePhase = !v.inReversePhase;
                }
            }
            v.prevTraverseRate = T;
            v.loopMode = params_.loopMode;

            double stepInc = 0.0;
            if (absT > 0.02f && params_.sampleLength > 2.0f) {
                const float macroTMod = std::pow(2.0f, 2.0f * (v.effMacroT - 0.5f));
                const double scanSeconds = std::max(0.001, static_cast<double>(v.ampDecaySeconds * absT * macroTMod));
                stepInc = loopSpan / (scanSeconds * sampleRate_);
            }

            if (params_.sampleLength <= 2.0f) {
                v.framePos = v.startFrame;
            } else if (absT <= 0.02f) {
                v.framePos = isTimeReversed ? v.startFrame : v.endFrame;
            } else {
                switch (v.loopMode) {
                    case 0: // FWD (Forward Looped)
                        if (!isTimeReversed) {
                            v.framePos += stepInc;
                            if (v.framePos >= v.endFrame) {
                                v.framePos = v.startFrame + std::fmod(v.framePos - v.startFrame, loopSpan);
                            }
                        } else {
                            v.framePos -= stepInc;
                            if (v.framePos <= v.startFrame) {
                                v.framePos = v.endFrame - std::fmod(v.endFrame - v.framePos, loopSpan);
                            }
                        }
                        break;

                    case 1: // REV (Reverse Looped: play initial direction to end, reverse and loop in reverse)
                        if (!isTimeReversed) {
                            if (!v.inReversePhase) {
                                v.framePos += stepInc;
                                if (v.framePos >= v.endFrame) {
                                    v.inReversePhase = true;
                                    v.framePos = v.endFrame - (v.framePos - v.endFrame);
                                }
                            } else {
                                v.framePos -= stepInc;
                                if (v.framePos <= v.startFrame) {
                                    v.framePos = v.endFrame - std::fmod(v.startFrame - v.framePos, loopSpan);
                                }
                            }
                        } else {
                            if (!v.inReversePhase) {
                                v.framePos -= stepInc;
                                if (v.framePos <= v.startFrame) {
                                    v.inReversePhase = true;
                                    v.framePos = v.startFrame + (v.startFrame - v.framePos);
                                }
                            } else {
                                v.framePos += stepInc;
                                if (v.framePos >= v.endFrame) {
                                    v.framePos = v.startFrame + std::fmod(v.framePos - v.endFrame, loopSpan);
                                }
                            }
                        }
                        break;

                    case 2: // PPG (Ping-Pong)
                        {
                            const double effectiveInc = stepInc * (isTimeReversed ? -v.ppgDirection : v.ppgDirection);
                            v.framePos += effectiveInc;
                            if (v.framePos >= v.endFrame) {
                                v.framePos = v.endFrame - (v.framePos - v.endFrame);
                                v.ppgDirection = -1;
                            } else if (v.framePos <= v.startFrame) {
                                v.framePos = v.startFrame + (v.startFrame - v.framePos);
                                v.ppgDirection = 1;
                            }
                            v.framePos = std::clamp(v.framePos, static_cast<double>(v.startFrame), static_cast<double>(v.endFrame));
                        }
                        break;

                    case 3: // OFF (One-shot, stuck at final frame)
                    default:
                        if (!isTimeReversed) {
                            v.framePos += stepInc;
                            if (v.framePos >= v.endFrame) {
                                v.framePos = v.endFrame;
                                v.isSustaining = true;
                            }
                        } else {
                            v.framePos -= stepInc;
                            if (v.framePos <= v.startFrame) {
                                v.framePos = v.startFrame;
                                v.isSustaining = true;
                            }
                        }
                        break;
                }
            }

            // 4. Sample ROM Evaluation (Continuous 3D Interpolation)
            float sampleVal = 0.0f;
            if (v.isSecretMode) {
                // Secret Core: Slot 160 (Sine -> Tube Warmth -> 16-Saw -> 32-Lanczos Hyper-Saw)
                sampleVal = rom.readSample(DigiStrikeFS2ROM::kSecretCoreSlot, v.currentMorph, static_cast<float>(v.framePos), v.oscPhase);
            } else if (v.activeCount == 1) {
                const auto c = v.activeWaves[0];
                const int slot = v.category * 20 + c.lineage;
                const float m = static_cast<float>(c.morphStep) * (1.0f / 3.0f);
                sampleVal = rom.readSample(slot, m, static_cast<float>(v.framePos), v.oscPhase);
            } else if (v.activeCount > 1) {
                const float span = static_cast<float>(v.activeCount - 1);
                const float pos = std::clamp(v.currentMorph * span, 0.0f, span);
                const int i0 = static_cast<int>(pos);
                const int i1 = std::min(i0 + 1, static_cast<int>(v.activeCount - 1));
                const float frac = pos - static_cast<float>(i0);
                // C^1 Hermite Smoothstep Crossfade (Zero boundary velocity, anti-click SOTA!)
                const float smoothFrac = frac * frac * (3.0f - 2.0f * frac);

                const auto c0 = v.activeWaves[static_cast<size_t>(i0)];
                const auto c1 = v.activeWaves[static_cast<size_t>(i1)];
                const int slot0 = v.category * 20 + c0.lineage;
                const int slot1 = v.category * 20 + c1.lineage;
                const float m0 = static_cast<float>(c0.morphStep) * (1.0f / 3.0f);
                const float m1 = static_cast<float>(c1.morphStep) * (1.0f / 3.0f);

                const float s0 = rom.readSample(slot0, m0, static_cast<float>(v.framePos), v.oscPhase);
                const float s1 = rom.readSample(slot1, m1, static_cast<float>(v.framePos), v.oscPhase);
                sampleVal = s0 + smoothFrac * (s1 - s0);
            } else {
                sampleVal = rom.readSample(v.slotIdx, v.currentMorph, static_cast<float>(v.framePos), v.oscPhase);
            }

            // 5. Lifeform Character Mojo
            if (params_.lifeMode == 0) {
                // Synthetic: Clean 32-bit linear interpolation
            } else if (params_.lifeMode == 1) {
                // Cyborg: Am6070 u-law 8-bit companding curve
                float sgn = (sampleVal < 0.0f) ? -1.0f : 1.0f;
                float mag = std::abs(sampleVal);
                float comp = std::log(1.0f + 255.0f * mag) * (1.0f / 5.5451774f); // ln(256)
                sampleVal = sgn * comp;
            } else {
                // Organic: 12-bit ZOH slew / drop-sample warmth
                v.zohSlewState += (sampleVal - v.zohSlewState) * 0.55f;
                sampleVal = v.zohSlewState;
            }

            // 6. Transient Impact Click Bleed
            if (v.pitchEnv > 0.8f && params_.clickBleed > 0.0f) {
                float click = (nextFloat() - 0.5f) * (params_.clickBleed * 0.01f);
                sampleVal += click;
            }

            // 6.5. Enveloped Noise Mix (K06)
            if (params_.noiseMix > 0.001f) {
                float rawNoise = 2.0f * (nextFloat() - 0.5f);
                float hpNoise = rawNoise - v.noiseRawPrev + 0.85f * v.noiseHpState;
                v.noiseRawPrev = rawNoise;
                v.noiseHpState = hpNoise;
                if (std::abs(v.noiseHpState) < 1e-15f) v.noiseHpState = 0.0f;

                float envNoise = v.ampEnv * (0.35f + 0.65f * v.pitchEnv);
                sampleVal += hpNoise * envNoise * (params_.noiseMix * 0.01f);
            }

            // 7. Amplitude Gain Scaling
            float gain = v.ampEnv * v.chokeEnv * v.velocity;
            float outSample = sampleVal * gain;

            accL[i] += outSample * panL;
            accR[i] += outSample * panR;

            // 8. Envelope Updates
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

        float feDecayRate = 1.0f / (params_.filterDecay * sampleRate_);
        filterEnvState_ += (filterEnvTarget_ - filterEnvState_) * feDecayRate;
        float feMod = (filterEnvState_ * params_.filterEnvAmt) * 4.0f;

        float cutoffHz = params_.cutoff * std::pow(2.0f, feMod + (params_.macroX - 0.5f) * 2.0f);
        cutoffHz = std::clamp(cutoffHz, 20.0f, 20000.0f);

        // Empyrean-SSM Q-Compensated Saturated ZDF Filter
        float res = std::clamp(params_.resonance, 0.0f, 0.98f);
        float damping = 2.0f * (1.0f - res);
        float gBassComp = 1.0f + 0.55f * res;

        float w0 = 2.0f * 3.14159265f * cutoffHz * invSampleRate_;
        float g = std::tan(w0 * 0.5f);
        float denom = 1.0f / (1.0f + damping * g + g * g);

        for (int i = 0; i < numSamples; ++i) {
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
    }

    float sampleRate_{48000.0f};
    float invSampleRate_{1.0f / 48000.0f};
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
    float filterEnvState_{0.0f};
    float filterEnvTarget_{0.0f};
    float filterEnvSlew_{0.0f};

    uint32_t rngState_{0xDEADBEEFu};
    FreqShifter freqShifter_{};
    WiderStage widerStage_{};
    DrumLimiter drumLimiter_{};
    std::array<float, 8> latchedDraw_{{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};
};

} // namespace tkdsp
