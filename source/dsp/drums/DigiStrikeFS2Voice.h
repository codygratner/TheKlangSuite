#pragma once

#include "FastMath.h"
#include "FormlessModMath.h"
#include "../dynamics/WiderStage.h"
#include "../dynamics/DrumLimiter.h"
#include "../mojo/FreqShifter.h"
#include "../../digistrike_fs2/DigiStrikeFS2Catalog.h"
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
    int category{0};         // 0=KICK, 1=SNAR, 2=TOMS, 3=CLAP, 4=HATS, 5=CYMB, 6=PERC, 7=STAB
    int lineage{0};          // 0=ALL, 1=CROWN, 2=APEX, 3=EMPYR, 4=FALCON, 5=K-WRK, 6=TOKYO, 7=GOTHN, 8=NIPPON, 9=CURIOS
    int voiceMode{0};        // 0=MONO, 1=GLIDE, 2=POLY
    int lifeMode{2};         // 0=Synthetic, 1=Cyborg, 2=Organic

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
    float dacCompand{50.0f}; // 0.0 .. 100.0 % (COMPAND, Linear -> u-Law -> Empyrean-12)
    float sampleLength{100.0f}; // 0.0 .. 100.0 % (LENGTH, 0% = single-cycle freeze)
    float tableMorph{0.0f};  // 0.0 .. 100.0 % (MORPH, 3D Spectral Slice)

    // Card 3 Pod 1: Stepped Random Humanization (8)
    float randX{0.0f};       // 0.0 .. 100.0 %
    float randY{0.0f};       // 0.0 .. 100.0 %
    float randZ{0.0f};       // 0.0 .. 100.0 %
    float randT{0.0f};       // 0.0 .. 100.0 %
    float randAggr{0.0f};    // 0.0 .. 100.0 %
    float randTimb{0.0f};    // 0.0 .. 100.0 %
    float randPan{0.0f};     // 0.0 .. 100.0 %
    float randDecay{0.0f};   // 0.0 .. 100.0 %

    // Card 3 Pod 2 & 3: Master Altar & Mojo Console (13)
    float macroDecay{0.5f};  // 0.0 .. 1.0 (Master Hero Knob)
    float volume{0.5f};      // 0.0 .. 1.0 (-inf .. 0dB .. +12dB)
    float sustainLevel{0.0f};// 0.0 .. 100.0 % (<=2% One-Shot, >2% Gated ADSR)
    float ampDecay{0.500f};  // 0.005 .. 5.0 s (skew 0.250s)

    float cutoff{1000.0f};   // 20.0 .. 20000.0 Hz
    float resonance{0.30f};  // 0.0 .. 1.0
    float filterEnvAmt{0.0f};// -1.0 .. +1.0
    float filterDecay{0.150f};// 0.005 .. 5.0 s
    int   filterType{0};     // 0=LP, 1=HP, 2=BP, 3=PK
    float preDrive{0.0f};    // 0.0 .. 1.0
    float crush{48000.0f};   // 500.0 .. 48000.0 Hz
    float shift{0.0f};      // -1000.0 .. +1000.0 Hz
    float squash{0.0f};      // -1.0 .. +1.0
    float wider{0.0f};       // 0.0 .. 2.0
    bool  masterLimiter{false};
};

class DigiStrikeFS2Voice {
public:
    static constexpr int kMaxVoices = 8;
    static constexpr size_t kSynthTableSize = 2048;

    DigiStrikeFS2Voice() noexcept {
        initSynthesisTable();
    }

    void prepare(float sampleRate) noexcept {
        sampleRate_ = (sampleRate > 1000.0f) ? sampleRate : 48000.0f;
        invSampleRate_ = 1.0f / sampleRate_;

        freqShifter_.prepare(sampleRate_);
        widerStage_.prepare(sampleRate_);
        drumLimiter_.prepare(sampleRate_);

        reset();
    }

    void reset() noexcept {
        for (auto& v : voices_) {
            v.active = false;
            v.phase = 0.0;
            v.ampEnv = 0.0f;
            v.pitchEnv = 0.0f;
            v.chokeEnv = 1.0f;
            v.chokeActive = false;
        }
        activeVoiceCount_ = 0;
        masterFilterState_.fill(0.0f);
        filterEnvState_ = 0.0f;
        filterEnvTarget_ = 0.0f;
        glideHz_ = 130.81f;

        freqShifter_.reset();
        widerStage_.reset();
        drumLimiter_.reset();
    }

    void triggerNote(int noteNumber, float velocity, float noteHz) noexcept {
        velocity = std::clamp(velocity, 0.01f, 1.0f);
        lastNoteNumber_ = noteNumber;
        lastVelocity_ = velocity;

        // Mode 0: MONO (instant 2ms choke on existing voices)
        // Mode 1: GLIDE (legato bass mode with portamento)
        // Mode 2: POLY (8-voice polyphonic allocation)
        if (params_.voiceMode == 0) {
            for (auto& v : voices_) {
                if (v.active) chokeVoice(v, velocity);
            }
            allocateVoice(0, noteNumber, velocity, noteHz);
        } else if (params_.voiceMode == 1) {
            glideHz_ = (activeVoiceCount_ > 0) ? glideHz_ : noteHz;
            allocateVoice(0, noteNumber, velocity, noteHz);
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

        // Paraphonic soft-retriggering of master bus filter envelope
        triggerMasterFilterEnv(velocity);
    }

    void releaseNote(int noteNumber) noexcept {
        for (auto& v : voices_) {
            if (v.active && v.noteNumber == noteNumber) {
                if (v.isGated) {
                    v.released = true;
                }
            }
        }
    }

    void setParams(const DigiStrikeFS2Params& p) noexcept {
        params_ = p;
    }

    void renderBlock(float* outL, float* outR, int numSamples) noexcept {
        if (numSamples <= 0) return;

        std::fill_n(outL, numSamples, 0.0f);
        std::fill_n(outR, numSamples, 0.0f);

        int activeCount = 0;
        for (auto& v : voices_) {
            if (v.active) {
                renderVoice(v, outL, outR, numSamples);
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
        bool chokeActive{false};
        int noteNumber{60};
        int category{0};
        float velocity{0.9f};
        float targetHz{130.81f};
        float currentHz{130.81f};

        double phase{0.0};
        double startPos{0.0};
        double endPos{kSynthTableSize - 1.0};
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
    };

    inline float nextFloat() noexcept {
        rngState_ += 0x9E3779B9u;
        uint32_t z = rngState_;
        z = (z ^ (z >> 16)) * 0x85EBCA6Bu;
        z = (z ^ (z >> 13)) * 0xC2B2AE35u;
        z ^= (z >> 16);
        return static_cast<float>((z >> 8) & 0x00FFFFFF) * (1.0f / 16777216.0f);
    }

    void initSynthesisTable() noexcept {
        for (size_t i = 0; i < kSynthTableSize; ++i) {
            double ph = 2.0 * 3.14159265358979323846 * static_cast<double>(i) / static_cast<double>(kSynthTableSize);
            double s = 0.55 * std::sin(ph)
                     + 0.25 * std::sin(2.0 * ph)
                     + 0.12 * std::sin(3.0 * ph)
                     + 0.06 * std::sin(4.0 * ph)
                     + 0.04 * std::sin(7.0 * ph);
            s = std::tanh(s * 1.35);
            synthTable_[i] = static_cast<float>(s);
        }
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
        v.currentHz = (params_.voiceMode == 1) ? glideHz_ : noteHz;

        // Stepped Random per-trigger offsets
        float rX = (nextFloat() - 0.5f) * 2.0f * (params_.randX * 0.01f);
        float rY = (nextFloat() - 0.5f) * 2.0f * (params_.randY * 0.01f);
        float rZ = (nextFloat() - 0.5f) * 2.0f * (params_.randZ * 0.01f);
        float rT = (nextFloat() - 0.5f) * 2.0f * (params_.randT * 0.01f);
        float rDecay = (nextFloat() - 0.5f) * 2.0f * (params_.randDecay * 0.01f);

        float effMacroX = std::clamp(params_.macroX + rX, 0.0f, 1.0f);
        float effMacroY = std::clamp(params_.macroY + rY, 0.0f, 1.0f);
        float effMacroZ = std::clamp(params_.macroZ + rZ, 0.0f, 1.0f);
        float effMacroT = std::clamp(params_.macroT + rT, 0.0f, 1.0f);

        // Traversal Trinity: START (0..500 ms)
        float startNorm = std::clamp(params_.startOffset / 500.0f, 0.0f, 1.0f);
        double maxFrames = static_cast<double>(kSynthTableSize - 1);
        v.startPos = startNorm * (maxFrames * 0.85);

        // LENGTH: 0% = single-cycle 2048 loop, 100% = full
        float lenNorm = std::clamp(params_.sampleLength * 0.01f, 0.0f, 1.0f);
        if (lenNorm <= 0.001f) {
            v.startPos = std::clamp(v.startPos, 0.0, maxFrames - 2048.0 > 0.0 ? maxFrames - 2048.0 : 0.0);
            v.endPos = v.startPos + 2047.0;
        } else {
            v.endPos = std::clamp(v.startPos + lenNorm * (maxFrames - v.startPos), v.startPos + 16.0, maxFrames);
        }

        // TIME: -4.0x .. +4.0x
        float rTime = params_.traverseRate * std::pow(2.0f, 2.0f * (effMacroT - 0.5f));
        v.isReverse = (rTime < -0.05f);

        // Effective one-shot duration derivation (FLAW-DSP-01 fix)
        float ampDecaySeconds = params_.ampDecay * (0.6f + 0.8f * params_.macroDecay);
        ampDecaySeconds = std::clamp(ampDecaySeconds * (1.0f + rDecay * 0.35f), 0.005f, 5.0f);
        v.ampDecayRate = 1.0f / (ampDecaySeconds * sampleRate_);
        v.ampReleaseRate = 1.0f / (0.050f * sampleRate_);

        v.isGated = (params_.sustainLevel > 2.0f);

        if (v.isReverse) {
            float effRate = std::abs(rTime);
            double effectiveSpanSamples = (v.endPos - v.startPos);
            if (!v.isGated) {
                double timeSpanSamples = static_cast<double>(ampDecaySeconds * 3.5f * sampleRate_ * effRate);
                effectiveSpanSamples = std::min(effectiveSpanSamples, timeSpanSamples);
            }
            v.phase = std::min(v.endPos, v.startPos + effectiveSpanSamples);
        } else {
            v.phase = v.startPos;
        }

        // Pitch Envelope: P.DEC & PUNCH
        float pDecayMs = params_.pitchDecay;
        v.pitchEnvRate = 1.0f / (pDecayMs * 0.001f * sampleRate_);
        v.pitchEnv = 1.0f;
        v.pitchDiveSemitones = (params_.pitchDive * 0.01f) * 36.0f * (0.5f + effMacroY);

        v.ampEnv = 1.0f;
        v.zohSlewState = 0.0f;
    }

    void triggerMasterFilterEnv(float velocity) noexcept {
        filterEnvTarget_ = velocity * std::abs(params_.filterEnvAmt);
        filterEnvSlew_ = 0.0f;
    }

    void renderVoice(VoiceSlot& v, float* accL, float* accR, int numSamples) noexcept {
        const double tableLen = static_cast<double>(kSynthTableSize);
        const float baseHz = sampleRate_ / static_cast<float>(kSynthTableSize);

        float panL = 0.7071f;
        float panR = 0.7071f;
        float effectivePan = std::clamp(params_.pan + (nextFloat() - 0.5f) * 2.0f * (params_.randPan * 0.01f), -1.0f, 1.0f);
        panL = std::cos((effectivePan + 1.0f) * 0.25f * 3.14159265f);
        panR = std::sin((effectivePan + 1.0f) * 0.25f * 3.14159265f);

        for (int i = 0; i < numSamples; ++i) {
            if (params_.voiceMode == 1) {
                v.currentHz += (v.targetHz - v.currentHz) * 0.015f;
            } else {
                v.currentHz = v.targetHz;
            }

            float pitchModSemitones = v.pitchDiveSemitones * (v.pitchEnv * v.pitchEnv);
            float pitchRatio = std::pow(2.0f, pitchModSemitones / 12.0f);
            float freqHz = v.currentHz * pitchRatio;

            double inc = (freqHz / baseHz);
            if (std::abs(params_.traverseRate) <= 0.05f) {
                inc = 0.0;
            } else if (v.isReverse) {
                inc = -std::abs(inc) * std::abs(params_.traverseRate);
            } else {
                inc = inc * params_.traverseRate;
            }

            double curPos = v.phase;
            if (curPos < 0.0) curPos = 0.0;
            if (curPos >= tableLen - 1.0) curPos = tableLen - 1.0;

            int idx0 = static_cast<int>(curPos);
            float frac = static_cast<float>(curPos - static_cast<double>(idx0));

            float sampleVal = 0.0f;
            if (params_.lifeMode == 0) {
                int idx1 = std::min(idx0 + 1, static_cast<int>(kSynthTableSize - 1));
                sampleVal = synthTable_[static_cast<size_t>(idx0)] * (1.0f - frac) + synthTable_[static_cast<size_t>(idx1)] * frac;
            } else if (params_.lifeMode == 1) {
                float raw = synthTable_[static_cast<size_t>(idx0)];
                float sgn = (raw < 0.0f) ? -1.0f : 1.0f;
                float mag = std::abs(raw);
                float comp = std::log(1.0f + 255.0f * mag) / std::log(256.0f);
                sampleVal = sgn * comp;
            } else {
                float raw = synthTable_[static_cast<size_t>(idx0)];
                v.zohSlewState += (raw - v.zohSlewState) * 0.55f;
                sampleVal = v.zohSlewState;
            }

            if (v.pitchEnv > 0.8f && params_.clickBleed > 0.0f) {
                float click = (nextFloat() - 0.5f) * (params_.clickBleed * 0.01f);
                sampleVal += click;
            }

            float gain = v.ampEnv * v.chokeEnv * v.velocity;
            float outSample = sampleVal * gain;

            accL[i] += outSample * panL;
            accR[i] += outSample * panR;

            v.phase += inc;
            if (v.isReverse) {
                if (v.phase <= v.startPos) {
                    if (v.isGated && !v.released) {
                        v.phase = v.endPos;
                    } else {
                        v.active = false;
                        break;
                    }
                }
            } else {
                if (v.phase >= v.endPos) {
                    if (v.isGated && !v.released) {
                        v.phase = v.startPos;
                    } else {
                        v.active = false;
                        break;
                    }
                }
            }

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
                if (v.released) {
                    v.ampEnv -= v.ampReleaseRate;
                    if (v.ampEnv <= 0.0f) {
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
        float masterGain = 1.0f;
        if (params_.volume <= 0.001f) {
            masterGain = 0.0f;
        } else {
            float dB = (params_.volume - 0.5f) * 24.0f;
            masterGain = std::pow(10.0f, dB / 20.0f);
        }

        float cutoffHz = std::clamp(params_.cutoff, 20.0f, 20000.0f);
        float q = std::clamp(params_.resonance * 0.95f, 0.01f, 0.95f);
        float w0 = 2.0f * 3.14159265f * cutoffHz * invSampleRate_;
        float g = std::tan(w0 * 0.5f);

        for (int i = 0; i < numSamples; ++i) {
            float inL = outL[i];
            float inR = outR[i];

            if (params_.preDrive > 0.01f) {
                float driveGain = 1.0f + params_.preDrive * 4.0f;
                inL = std::tanh(inL * driveGain);
                inR = std::tanh(inR * driveGain);
            }

            float hpL = (inL - (2.0f * (1.0f - q) + g) * masterFilterState_[0] - masterFilterState_[1]) / (1.0f + 2.0f * (1.0f - q) * g + g * g);
            float bpL = g * hpL + masterFilterState_[0];
            float lpL = g * bpL + masterFilterState_[1];
            masterFilterState_[0] = g * hpL + bpL;
            masterFilterState_[1] = g * bpL + lpL;

            float hpR = (inR - (2.0f * (1.0f - q) + g) * masterFilterState_[2] - masterFilterState_[3]) / (1.0f + 2.0f * (1.0f - q) * g + g * g);
            float bpR = g * hpR + masterFilterState_[2];
            float lpR = g * bpR + masterFilterState_[3];
            masterFilterState_[2] = g * hpR + bpR;
            masterFilterState_[3] = g * bpR + lpR;

            float filtL = lpL;
            float filtR = lpR;
            if (params_.filterType == 1) { filtL = hpL; filtR = hpR; }
            else if (params_.filterType == 2) { filtL = bpL; filtR = bpR; }

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

    std::array<float, kSynthTableSize> synthTable_{};
    std::array<float, 4> masterFilterState_{{0.0f, 0.0f, 0.0f, 0.0f}};
    float filterEnvState_{0.0f};
    float filterEnvTarget_{0.0f};
    float filterEnvSlew_{0.0f};

    uint32_t rngState_{0xDEADBEEFu};
    FreqShifter freqShifter_{};
    WiderStage widerStage_{};
    DrumLimiter drumLimiter_{};
};

} // namespace tkdsp
