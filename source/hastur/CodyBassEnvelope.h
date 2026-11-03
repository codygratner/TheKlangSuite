#pragma once

#include "../FastMath.h"
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace CrackedChimes::DSP {

/**
 * @brief SubStator Bass Dynamics Engine (Cody Bass Envelope v2.0)
 * 
 * SSoT: Decoupled Snap-Body-Morph Architecture
 * - Snap: 0.1ms .. 25ms independent transient micro-stage. Never alters decay time.
 * - Decay: 20ms .. 8.0s logarithmic body duration (supports DAW tempo sync).
 * - Curvature: -1.0 (Acid Blip: alpha=6) .. 0.0 (Exponential Pluck: alpha=2) .. +1.0 (Parabolic Sub-Shelf: alpha=0.66).
 * - Sustain: 0.0 = One-Shot (808 Kick Mode), > 0.0 = Gated Synth Bass.
 * - Pitch Dive: Integrated transient pitch drop for kicks (0 .. +48 st).
 * - Velocity Matrix: Dynamic Snap compression, Dive scaling, and Amp headroom.
 * - Auto-Choke: 2ms acoustic beater choke on rapid retrigger, eliminating phase cancellation.
 * - Zero transcendentals in audio loop (1-pole recursive leaky integrators + FastMath polynomials).
 */
class CodyBassEnvelope {
public:
    enum class Stage : uint8_t {
        IDLE = 0,
        CHOKE,
        SNAP,
        DECAY,
        SUSTAIN,
        RELEASE
    };

    void prepare(double sampleRate) noexcept {
        sampleRate_ = (sampleRate > 1000.0) ? sampleRate : 44100.0;
        invSr_ = 1.0 / sampleRate_;
        reset();
    }

    void reset() noexcept {
        stage_ = Stage::IDLE;
        val_ = 0.0f;
        stagePhase_ = 0.0;
        snapStartVal_ = 0.0f;
        chokeStartVal_ = 0.0f;
        releaseStartVal_ = 0.0f;
        velocity_ = 1.0f;
        pitchStateSemi_ = 0.0f;
        sustainLevel_ = 0.0f;
        autoChoke_ = true;
        gate_ = false;
        currentFreqHz_ = 130.81f;
        targetFreqHz_ = 130.81f;
    }

    void noteOn(float velocity, bool autoChoke = true) noexcept {
        velocity_ = std::clamp(velocity, 0.01f, 1.0f);
        autoChoke_ = autoChoke;
        gate_ = true;

        if (autoChoke_ && (stage_ == Stage::DECAY || stage_ == Stage::RELEASE) && val_ > 0.01f) {
            // Rapid retrigger during active decay: 2ms acoustic beater choke!
            stage_ = Stage::CHOKE;
            stagePhase_ = 0.0;
            chokeStartVal_ = val_;
        } else {
            // Standard start or Capacitor Pickup
            stage_ = Stage::SNAP;
            stagePhase_ = 0.0;
            snapStartVal_ = (stage_ == Stage::IDLE) ? 0.0f : val_;
        }
    }

    // Overload for legacy/voice compatibility
    void noteOn(int midiNote, float velocity, float glideTimeSec) noexcept {
        targetFreqHz_ = 440.0f * std::pow(2.0f, (static_cast<float>(midiNote) - 69.0f) * (1.0f / 12.0f));
        if (stage_ == Stage::IDLE || glideTimeSec <= 0.0005f) {
            currentFreqHz_ = targetFreqHz_;
        }
        noteOn(velocity, true);
    }

    void noteOff() noexcept {
        gate_ = false;
        // In One-Shot mode (sustain <= 0.001f), note-off is completely ignored!
        // In Sustain mode, key-off initiates Release from the current level.
        if (sustainLevel_ > 0.001f && stage_ != Stage::IDLE && stage_ != Stage::RELEASE) {
            stage_ = Stage::RELEASE;
            stagePhase_ = 0.0;
            releaseStartVal_ = val_;
        }
    }

    /**
     * @brief Process one sample of the SubStator Bass Dynamics Engine.
     * @param snapSec Transient duration [0.0001s, 0.025s].
     * @param decaySec Master body duration [0.020s, 8.0s].
     * @param curvature Morph geometry [-1.0, +1.0] (Blip <-> Exponential <-> Sub-Shelf).
     * @param sustainLevel Gated plateau [0.0, 1.0] (0.0 = One-Shot Kick Mode).
     * @param releaseSec Key-off duration [0.005s, 3.0s].
     * @param pitchDiveSemi Instant pitch drop depth [0.0, 48.0 semitones].
     * @param diveTimeSec Pitch drop duration [0.002s, 0.080s].
     * @param velSens Velocity sensitivity depth [0.0, 1.0].
     * @param outPitchModSemitones Output pitch offset for voice oscillators.
     * @return Output amplitude envelope scalar [0.0, 1.0].
     */
    [[nodiscard]] float process(
        float snapSec,
        float decaySec,
        float curvature,
        float sustainLevel,
        float releaseSec,
        float pitchDiveSemi,
        float diveTimeSec,
        float velSens,
        float& outPitchModSemitones) noexcept
    {
        sustainLevel_ = sustainLevel;
        const bool isOneShot = (sustainLevel <= 0.001f);
        const float targetSus = isOneShot ? 0.0f : sustainLevel;

        // 1. Velocity Multi-Target Matrix
        const float vSens = std::clamp(velSens, 0.0f, 1.0f);
        const float vHeadroom = (1.0f - vSens * 0.7f) + (vSens * 0.7f) * velocity_;
        const float effSnapSec = std::max(0.0001f, snapSec * (1.0f - 0.75f * vSens * velocity_));
        const float effPitchDiveSemi = pitchDiveSemi * (0.3f + 0.7f * velocity_);

        // 2. Pitch Dive 1-Pole Leaky Integrator (Bipolar: +Kick Dive or -Sax Embouchure Scoop)
        if (stage_ == Stage::SNAP && stagePhase_ < invSr_ * 2.0) {
            pitchStateSemi_ = effPitchDiveSemi;
        }
        if (std::abs(pitchStateSemi_) > 0.001f) {
            const float tauDive = std::max(0.002f, diveTimeSec);
            const float kDive = static_cast<float>(1.0 - std::exp(-invSr_ / tauDive));
            pitchStateSemi_ += kDive * (0.0f - pitchStateSemi_);
        } else {
            pitchStateSemi_ = 0.0f;
        }
        outPitchModSemitones = pitchStateSemi_;

        // 3. Envelope State Machine
        switch (stage_) {
            case Stage::IDLE:
                val_ = 0.0f;
                break;

            case Stage::CHOKE: {
                // 2.0ms ultra-fast acoustic beater discharge
                stagePhase_ += invSr_ / 0.002;
                if (stagePhase_ >= 1.0) {
                    val_ = 0.0f;
                    stage_ = Stage::SNAP;
                    stagePhase_ = 0.0;
                    snapStartVal_ = 0.0f;
                } else {
                    const float p = static_cast<float>(stagePhase_);
                    val_ = chokeStartVal_ * std::max(0.0f, 1.0f - p);
                }
                break;
            }

            case Stage::SNAP: {
                stagePhase_ += invSr_ / effSnapSec;
                if (stagePhase_ >= 1.0) {
                    val_ = 1.0f;
                    stage_ = Stage::DECAY;
                    stagePhase_ = 0.0;
                } else {
                    const float p = static_cast<float>(stagePhase_);
                    val_ = snapStartVal_ + (1.0f - snapStartVal_) * p;
                }
                break;
            }

            case Stage::DECAY: {
                const float tDec = std::max(0.010f, decaySec);
                stagePhase_ += invSr_ / tDec;
                if (stagePhase_ >= 1.0) {
                    if (isOneShot) {
                        val_ = 0.0f;
                        stage_ = Stage::IDLE;
                    } else {
                        val_ = targetSus;
                        stage_ = Stage::SUSTAIN;
                    }
                    stagePhase_ = 0.0;
                } else {
                    const float p = static_cast<float>(stagePhase_);
                    const float u = std::clamp(1.0f - p, 0.0f, 1.0f);

                    // Continuous Curvature Transfer Function: alpha = 2.0 * 3.0^(-curvature)
                    // -1.0 -> alpha = 6.0  (Acid Blip: drops 90% in first 30% time)
                    //  0.0 -> alpha = 2.0  (Exponential Pluck: classic analog 1-pole RC capacitor)
                    // +1.0 -> alpha = 0.66 (Parabolic Sub-Shelf: holds >50% power past halfway mark)
                    const float c = std::clamp(curvature, -1.0f, 1.0f);
                    const float alpha = 2.0f * TbdAudio::FastMath::fastPow2(-c * 1.5849625f);
                    const float normDec = (u > 1e-5f)
                        ? TbdAudio::FastMath::fastPow2(alpha * TbdAudio::FastMath::fastLog2(u))
                        : 0.0f;

                    val_ = targetSus + (1.0f - targetSus) * normDec;
                }
                break;
            }

            case Stage::SUSTAIN:
                if (isOneShot) {
                    val_ = 0.0f;
                    stage_ = Stage::IDLE;
                } else {
                    val_ = sustainLevel;
                }
                break;

            case Stage::RELEASE: {
                const float tRel = std::max(0.005f, releaseSec);
                stagePhase_ += invSr_ / tRel;
                if (stagePhase_ >= 1.0 || releaseStartVal_ <= 1e-5f) {
                    val_ = 0.0f;
                    stage_ = Stage::IDLE;
                } else {
                    const float p = static_cast<float>(stagePhase_);
                    val_ = releaseStartVal_ * std::max(0.0f, 1.0f - p);
                }
                break;
            }
        }

        // Velocity scaling and denormal safety
        float out = val_ * vHeadroom;
        if (out < 1e-6f) out = 0.0f;
        return out;
    }

    /**
     * @brief Backward-compatible process overload for existing HasturVoice wiring.
     * Maps totalTimeSec and slope to the new decoupled SubStator math, fixing the broken seesaw!
     */
    [[nodiscard]] float process(
        float totalTimeSec,
        float slope,
        float sustainLevel,
        float releaseTimeSec,
        float glideTimeSec,
        float& outFreqHz) noexcept
    {
        // 1. Process Glide Slew
        if (std::abs(currentFreqHz_ - targetFreqHz_) > 0.01f) {
            const float glideSec = std::max(0.00025f, glideTimeSec);
            const float kGlide = static_cast<float>(1.0 - std::exp(-invSr_ / glideSec));
            currentFreqHz_ += kGlide * (targetFreqHz_ - currentFreqHz_);
        } else {
            currentFreqHz_ = targetFreqHz_;
        }
        outFreqHz = currentFreqHz_;

        // 2. Map Legacy Parameters to Fixed SubStator v2.0
        // Decoupled Snap: fixed 1.5ms punch (no more 990ms reverse attack!)
        const float snapSec = 0.0015f;
        const float decaySec = std::max(0.020f, totalTimeSec);
        const float curvature = std::clamp(slope, -1.0f, 1.0f);
        const float releaseSec = std::max(0.005f, releaseTimeSec);
        float dummyPitch = 0.0f;

        return process(snapSec, decaySec, curvature, sustainLevel, releaseSec, 0.0f, 0.015f, 0.5f, dummyPitch);
    }

    [[nodiscard]] bool isIdle() const noexcept { return stage_ == Stage::IDLE; }
    [[nodiscard]] Stage getStage() const noexcept { return stage_; }

private:
    double sampleRate_{44100.0};
    double invSr_{1.0 / 44100.0};
    Stage stage_{Stage::IDLE};
    double stagePhase_{0.0};
    float val_{0.0f};
    float snapStartVal_{0.0f};
    float chokeStartVal_{0.0f};
    float releaseStartVal_{0.0f};
    float velocity_{1.0f};
    float sustainLevel_{0.0f};
    bool autoChoke_{true};
    bool gate_{false};

    float pitchStateSemi_{0.0f};
    float currentFreqHz_{130.81f};
    float targetFreqHz_{130.81f};
};

} // namespace CrackedChimes::DSP
