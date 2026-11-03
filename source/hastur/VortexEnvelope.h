#pragma once

#include "../FastMath.h"
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace CrackedChimes::DSP {

/**
 * @brief Vortex Multi-Stage Filter & Timbre Modulation Engine (Multi-Stage Metamorphic Envelope Engine)
 * 
 * SSoT: 5+2 Macro Model with Emancipated Directional Inversions
 * - Attack & Snap: Rise duration and initial transient strike.
 * - Bloom (-100% .. +100%): Biphasic Pluck <-> Monotonic ADSR <-> Reverse Bass Suck / Hoover Swell.
 * - Aftershock (0% .. 100%): Secondary mid-note detonation (L2 up to 150% for screaming filter overdrive).
 * - Decay: Master deterministic time budget (T2 + T3) with DAW tempo-sync.
 * - Sustain & Release: Holding level and key-off tail duration with capacitor pickup.
 * - KeyRateTrack & Function Generator Looping: High-register scaling and cyclic LFO mode.
 * - Soft Saturation Output Guard: Prevents Nyquist blowup during extreme aftershock boosts.
 */
class VortexEnvelope {
public:
    enum class Stage : uint8_t {
        IDLE = 0,
        ATTACK,      // T1 -> L1
        BREAK,       // T2 -> L2 (Valley on Reverse Suck, Peak on Aftershock)
        DECAY,       // T3 -> L3 (Swell to Sustain or secondary drop)
        SUSTAIN,     // Hold at L3
        RELEASE      // T4 -> 0.0
    };

    enum class LoopMode : uint8_t {
        OFF = 0,
        FREE,
        TEMPO_SYNC
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
        attackStartVal_ = 0.0f;
        releaseStartVal_ = 0.0f;
        velocity_ = 1.0f;
        midiNote_ = 60;
        gate_ = false;
    }

    void noteOn(int midiNote, float velocity) noexcept {
        midiNote_ = midiNote;
        velocity_ = std::clamp(velocity, 0.01f, 1.0f);
        gate_ = true;
        stage_ = Stage::ATTACK;
        stagePhase_ = 0.0;
        attackStartVal_ = (stage_ == Stage::IDLE) ? 0.0f : val_; // Smooth capacitor pickup
    }

    void noteOff() noexcept {
        gate_ = false;
        if (stage_ != Stage::IDLE && stage_ != Stage::RELEASE) {
            stage_ = Stage::RELEASE;
            stagePhase_ = 0.0;
            releaseStartVal_ = val_; // Slew-bounded release pickup
        }
    }

    /**
     * @brief Process one sample of the Vortex Envelope.
     * @param attackSec Attack rise duration [0.0005s, 10.0s].
     * @param snap Peak amplitude scalar [0.0, 1.0].
     * @param decaySec Master decay budget [0.010s, 15.0s].
     * @param bloom Contour morph [-1.0, +1.0] (Pluck <-> ADSR <-> Reverse Suck).
     * @param aftershock Secondary detonation boost [0.0, 1.0] (L2 up to 150%).
     * @param sustainLevel Holding plateau [0.0, 1.0].
     * @param releaseSec Release duration [0.002s, 15.0s].
     * @param keyTrack Keyboard rate scaling [0.0, 1.0].
     * @param loopMode Looping behavior (Off, Free, Tempo Sync).
     * @return Output envelope scalar [0.0, 1.5] (soft-saturated).
     */
    [[nodiscard]] float process(
        float attackSec,
        float snap,
        float decaySec,
        float bloom,
        float aftershock,
        float sustainLevel,
        float releaseSec,
        float keyTrack,
        LoopMode loopMode, float velSens = 1.0f) noexcept
    {
        // 1. Keyboard Rate Tracking: higher notes decay faster
        const float noteOffset = static_cast<float>(midiNote_ - 60);
        const float kTrack = std::clamp(keyTrack, 0.0f, 1.0f);
        const float rateMult = TbdAudio::FastMath::fastPow2(-kTrack * noteOffset * (1.0f / 12.0f));

        const float tAtk = std::max(0.0002f, attackSec * rateMult);
        const float tDecayTotal = std::max(0.005f, decaySec * rateMult);
        const float tRel = std::max(0.002f, releaseSec * rateMult);

        const float effVel = (1.0f - velSens) + velSens * velocity_;
        const float L1 = std::clamp(snap * effVel, 0.05f, 1.0f);
        const float L3 = std::clamp(sustainLevel, 0.0f, 1.0f);
        const float b = std::clamp(bloom, -1.0f, 1.0f);
        const float ashock = std::clamp(aftershock, 0.0f, 1.0f);

        // 2. Closed-Form Macro Coordinate Mapping
        float t2 = 0.35f * tDecayTotal;
        float L2 = L3 + (L1 - L3) * 0.37f;

        if (b > 0.0f) {
            // Hoover Swell / Reverse Suck
            t2 = tDecayTotal * (0.35f - 0.20f * b);
            // Valley target: drops deep
            const float valleyBase = (L3 > 0.05f) ? (L3 * (1.0f - 0.85f * b)) : (L1 * 0.15f * (1.0f - b));
            L2 = std::max(0.02f, valleyBase);
        } else if (b < 0.0f) {
            // Acoustic Pluck (fast initial drop)
            t2 = tDecayTotal * (0.35f + 0.20f * b);
            L2 = L3 + (L1 - L3) * (0.37f + 0.30f * b);
        }

        // Apply Aftershock boost to break level L2
        if (ashock > 0.001f) {
            L2 = std::min(1.5f, L2 + ashock * 0.65f);
        }

        const float t3 = std::max(0.002f, tDecayTotal - t2);

        // 3. State Machine
        switch (stage_) {
            case Stage::IDLE:
                val_ = 0.0f;
                break;

            case Stage::ATTACK: {
                stagePhase_ += invSr_ / tAtk;
                if (stagePhase_ >= 1.0) {
                    val_ = L1;
                    stage_ = Stage::BREAK;
                    stagePhase_ = 0.0;
                } else {
                    const float p = static_cast<float>(stagePhase_);
                    val_ = attackStartVal_ + (L1 - attackStartVal_) * p;
                }
                break;
            }

            case Stage::BREAK: {
                stagePhase_ += invSr_ / t2;
                if (stagePhase_ >= 1.0) {
                    val_ = L2;
                    stage_ = Stage::DECAY;
                    stagePhase_ = 0.0;
                } else {
                    const float p = static_cast<float>(stagePhase_);
                    val_ = L1 + (L2 - L1) * p;
                }
                break;
            }

            case Stage::DECAY: {
                stagePhase_ += invSr_ / t3;
                if (stagePhase_ >= 1.0) {
                    val_ = L3;
                    if (loopMode != LoopMode::OFF) {
                        // Function generator cyclic loop
                        stage_ = Stage::ATTACK;
                        stagePhase_ = 0.0;
                        attackStartVal_ = val_;
                    } else if (L3 <= 0.001f) {
                        val_ = 0.0f;
                        stage_ = Stage::IDLE;
                    } else {
                        stage_ = Stage::SUSTAIN;
                    }
                    stagePhase_ = 0.0;
                } else {
                    const float p = static_cast<float>(stagePhase_);
                    val_ = L2 + (L3 - L2) * p;
                }
                break;
            }

            case Stage::SUSTAIN:
                val_ = L3;
                break;

            case Stage::RELEASE: {
                stagePhase_ += invSr_ / tRel;
                if (stagePhase_ >= 1.0 || releaseStartVal_ <= 1e-5f) {
                    val_ = 0.0f;
                    if (loopMode != LoopMode::OFF) {
                        stage_ = Stage::ATTACK;
                        stagePhase_ = 0.0;
                        attackStartVal_ = 0.0f;
                    } else {
                        stage_ = Stage::IDLE;
                    }
                } else {
                    const float p = static_cast<float>(stagePhase_);
                    val_ = releaseStartVal_ * std::max(0.0f, 1.0f - p);
                }
                break;
            }
        }

        // 4. Soft Saturation Ceiling to protect filter from extreme aftershock blowup
        float out = val_;
        if (out > 1.0f) {
            // Smoothly compress overshoots between 1.0 and 1.5
            const float over = out - 1.0f;
            out = 1.0f + TbdAudio::FastMath::fastTanh(over * 1.5f) * (1.0f / 1.5f);
        }

        if (out < 1e-6f) out = 0.0f;
        return out;
    }

    [[nodiscard]] bool isIdle() const noexcept { return stage_ == Stage::IDLE; }
    [[nodiscard]] Stage getStage() const noexcept { return stage_; }

private:
    double sampleRate_{44100.0};
    double invSr_{1.0 / 44100.0};
    Stage stage_{Stage::IDLE};
    double stagePhase_{0.0};
    float val_{0.0f};
    float attackStartVal_{0.0f};
    float releaseStartVal_{0.0f};
    float velocity_{1.0f};
    int midiNote_{60};
    bool gate_{false};
};

} // namespace CrackedChimes::DSP
