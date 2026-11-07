#pragma once

#include <cmath>
#include <algorithm>
#include <array>
#include "shoggoth_layout.h"
#include "FastMath.h"

namespace Shoggoth {

class Envelopes {
public:
    Envelopes() {
        initJanssenLUT();
    }

    void prepare(double sampleRate) noexcept {
        sampleRate_ = static_cast<float>(sampleRate > 1000.0 ? sampleRate : 48000.0);
    }

    // ==============================================================================
    // 1. Trigger 1987 Mode 6-Stage Envelope
    // ==============================================================================
    void trigger1987(EnvRuntimeState& state, const DHGEnvelopeData& data) noexcept {
        state.stageIndex = 0;
        state.currentLevel = 0.0f;
        state.targetLevel = data.stages[0].level;
        state.isFinished = false;
        state.isReleased = false;
        setupRateDelta(state, data.stages[0].rate);
    }

    void release1987(EnvRuntimeState& state, const DHGEnvelopeData& data) noexcept {
        state.isReleased = true;
        // Jump to release stage (Stage 5)
        state.stageIndex = 5;
        state.targetLevel = 0.0f;
        setupRateDelta(state, data.stages[5].rate);
    }

    // Advance by numSamples (typically 16 samples)
    float advance1987(EnvRuntimeState& state, const DHGEnvelopeData& data, int numSamples) noexcept {
        if (state.isFinished) return janssenGain(state.currentLevel);

        if (!state.isReleased && state.stageIndex == 4 && std::abs(state.currentLevel - state.targetLevel) < 1e-4f) {
            return janssenGain(state.currentLevel);
        }

        state.currentLevel += state.rateDelta * static_cast<float>(numSamples);

        // Check if reached target
        bool reached = false;
        if (state.rateDelta >= 0.0f && state.currentLevel >= state.targetLevel) {
            state.currentLevel = state.targetLevel;
            reached = true;
        } else if (state.rateDelta < 0.0f && state.currentLevel <= state.targetLevel) {
            state.currentLevel = state.targetLevel;
            reached = true;
        }

        if (reached) {
            advanceStage(state, data);
        }

        return janssenGain(state.currentLevel);
    }

    // ==============================================================================
    // 2. Monosynth Snappy AHDSR
    // ==============================================================================
    void triggerAhdsr(AhdsrState& s) noexcept {
        s.phase = AhdsrState::Phase::Attack;
        s.currentLevel = 0.0f;
        s.holdTimerSec = 0.0f;
    }

    void releaseAhdsr(AhdsrState& s) noexcept {
        s.phase = AhdsrState::Phase::Release;
    }

    float advanceAhdsr(AhdsrState& s,
                       float attackSec, float holdSec, float decaySec,
                       float sustainNorm, float releaseSec,
                       int numSamples, bool analogSnap) noexcept {
        const float dt = static_cast<float>(numSamples) / sampleRate_;

        switch (s.phase) {
            case AhdsrState::Phase::Idle:
                s.currentLevel = 0.0f;
                break;

            case AhdsrState::Phase::Attack: {
                const float aTime = std::max(0.0005f, attackSec);
                s.currentLevel += dt / aTime;
                if (s.currentLevel >= 1.0f) {
                    s.currentLevel = 1.0f;
                    s.phase = (holdSec > 0.001f) ? AhdsrState::Phase::Hold : AhdsrState::Phase::Decay;
                    s.holdTimerSec = holdSec;
                }
                break;
            }

            case AhdsrState::Phase::Hold:
                s.holdTimerSec -= dt;
                s.currentLevel = 1.0f;
                if (s.holdTimerSec <= 0.0f) {
                    s.phase = AhdsrState::Phase::Decay;
                }
                break;

            case AhdsrState::Phase::Decay: {
                const float dTime = std::max(0.002f, decaySec);
                const float rate = dt / dTime;
                s.currentLevel += (sustainNorm - s.currentLevel) * rate * 3.0f;
                if (std::abs(s.currentLevel - sustainNorm) < 0.001f) {
                    s.currentLevel = sustainNorm;
                    s.phase = AhdsrState::Phase::Sustain;
                }
                break;
            }

            case AhdsrState::Phase::Sustain:
                s.currentLevel = sustainNorm;
                break;

            case AhdsrState::Phase::Release: {
                const float rTime = std::max(0.002f, releaseSec);
                const float rate = dt / rTime;
                s.currentLevel -= s.currentLevel * rate * 3.0f;
                if (s.currentLevel <= 0.0001f) {
                    s.currentLevel = 0.0f;
                    s.phase = AhdsrState::Phase::Idle;
                }
                break;
            }
        }

        float out = std::clamp(s.currentLevel, 0.0f, 1.0f);
        if (analogSnap && s.phase == AhdsrState::Phase::Attack) {
            out = std::min(1.0f, out * 1.25f);
        }
        return out;
    }

private:
    float sampleRate_{48000.0f};
    std::array<float, 1024> janssenGainLUT_{};

    void initJanssenLUT() noexcept {
        constexpr float gamma = 1.6f;
        for (int q = 0; q < 1024; ++q) {
            // Herbert Janssen's 10-bit logarithmic dB curve
            const float normQ = static_cast<float>(q) / 1023.0f;
            const float db = -96.0f * (1.0f - normQ);
            janssenGainLUT_[q] = TbdAudio::FastMath::fastDbToGain(db);
        }
    }

    float janssenGain(float level99) const noexcept {
        const float clamped = std::clamp(level99, 0.0f, 99.0f);
        const float norm = clamped / 99.0f;
        constexpr float gamma = 1.6f;
        const int q = std::clamp(static_cast<int>(std::round(1023.0f * std::pow(norm, gamma))), 0, 1023);
        return janssenGainLUT_[q];
    }

    void setupRateDelta(EnvRuntimeState& state, float rate99) noexcept {
        const float r = std::clamp(rate99, 0.0f, 99.0f);
        // Rate maps exponentially: 0 = 30 seconds, 99 = 2 milliseconds
        const float timeSec = 30.0f * std::pow(0.000066f, r / 99.0f);
        const float totalSamples = timeSec * sampleRate_;
        const float deltaLvl = state.targetLevel - state.currentLevel;
        state.rateDelta = (totalSamples > 1.0f) ? (deltaLvl / totalSamples) : deltaLvl;
    }

    void advanceStage(EnvRuntimeState& state, const DHGEnvelopeData& data) noexcept {
        if (state.stageIndex >= 5) {
            state.isFinished = true;
            state.currentLevel = state.targetLevel;
            state.rateDelta = 0.0f;
            return;
        }

        // Stage 4 is the authentic sustain point while key is held!
        if (!state.isReleased && state.stageIndex == 4) {
            state.currentLevel = state.targetLevel;
            state.rateDelta = 0.0f;
            return;
        }

        // Loop 3-4 handling
        if (data.loop34 && state.stageIndex == 3 && !state.isReleased) {
            state.stageIndex = 2; // Loop back to Stage 3
        } else {
            state.stageIndex++;
        }

        if (!state.isReleased && state.stageIndex == 4) {
            // Arrived at sustain stage
            state.targetLevel = data.stages[4].level;
            setupRateDelta(state, data.stages[4].rate);
        } else if (state.stageIndex < 5) {
            state.targetLevel = data.stages[state.stageIndex].level;
            setupRateDelta(state, data.stages[state.stageIndex].rate);
        } else {
            // Stage 5 Release
            state.targetLevel = 0.0f;
            setupRateDelta(state, data.stages[5].rate);
        }
    }
};

} // namespace Shoggoth
