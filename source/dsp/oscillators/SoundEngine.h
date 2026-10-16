#pragma once

#include "../common/AudioEffect.h"
#include "PolyBLEP.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace TbdAudio::Oscillators {

/**
 * @brief SoundEngine8 — Universal 8-Control Synthesizer Voice & Oscillator Base Class.
 *        Inherits from TbdAudio::AudioEffect8 (and TbdAudio::DSPBlock) so every sound engine
 *        can be hosted standalone, inside ModularDrumEngine/Planter, or inside ToadTracker.
 *
 * Real-Time Invariants:
 *   - Zero heap allocations in trigger(), process(), processStereo(), or renderSampleStereo().
 *   - Zero CRT transcendentals (uses FastMath::fastPow2, fastSinNorm, fastExp, fastTanh).
 */
class SoundEngine8 : public AudioEffect8 {
public:
    SoundEngine8() noexcept = default;
    ~SoundEngine8() override = default;

    void init(const BlockContext& ctx) override {
        sampleRate_ = (ctx.sampleRate > 8000.0f) ? ctx.sampleRate : 48000.0f;
        invSr_ = 1.0f / sampleRate_;
        basePitchHz_ = (ctx.currentPitchHz > 10.0f) ? ctx.currentPitchHz : 65.4064f;
        midiNote_ = ctx.currentMidiNote;
        onPrepare(sampleRate_);
    }

    void prepare(double sampleRate, int /*maxBlockSize*/ = 512) noexcept {
        sampleRate_ = (sampleRate > 8000.0) ? static_cast<float>(sampleRate) : 48000.0f;
        invSr_ = 1.0f / sampleRate_;
        onPrepare(sampleRate_);
    }

    inline void setParam8(int index, float value) noexcept {
        setParam(index, value);
    }

    /**
     * Convert MIDI note + semitone offset to frequency (Hz) using FastMath::fastPow2.
     */
    [[nodiscard]] static inline float midiNoteToHz(float midiNote, float semitoneOffset = 0.0f) noexcept {
        const float note = std::clamp(midiNote + semitoneOffset, 0.0f, 135.0f);
        return 440.0f * FastMath::fastPow2((note - 69.0f) * (1.0f / 12.0f));
    }

    /**
     * Quantize a normalized [0..1] octave knob into discrete octave offsets [-4..+4] (or semitones).
     */
    [[nodiscard]] static inline int quantizeOctave4(float norm) noexcept {
        const float clamped = std::clamp(norm, 0.0f, 1.0f);
        return std::clamp(static_cast<int>(clamped * 8.0f + 0.5f) - 4, -4, 4);
    }

    void setPitchHz(float hz) noexcept {
        basePitchHz_ = std::clamp(hz, 8.0f, 20000.0f);
    }

    void setMidiNote(int note, float velocity = 1.0f) noexcept {
        midiNote_ = std::clamp(note, 0, 127);
        basePitchHz_ = midiNoteToHz(static_cast<float>(midiNote_));
        trigger(velocity);
    }

    void trigger(float velocity = 1.0f) override {
        velocity_ = std::clamp(velocity, 0.0f, 1.0f);
        onTrigger(velocity_);
    }

    /**
     * Per-sample stereo synthesis hook implemented by each concrete SoundEngine8 subclass.
     */
    virtual void renderSampleStereo(float baseFreqHz, float& outL, float& outR) noexcept = 0;

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        FastMath::enableFTZDAZ();
        if (!buffer || numSamples <= 0) return;
        if (ctx.currentPitchHz > 10.0f) {
            basePitchHz_ = ctx.currentPitchHz;
        }
        if (ctx.isTriggered) {
            trigger(ctx.triggerVelocity);
        }
        for (int i = 0; i < numSamples; ++i) {
            float l = 0.0f, r = 0.0f;
            renderSampleStereo(basePitchHz_, l, r);
            buffer[i] = 0.5f * (l + r);
        }
        FastMath::sanitizeBuffer(buffer, numSamples);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& ctx) override {
        FastMath::enableFTZDAZ();
        if (!left || numSamples <= 0) return;
        if (ctx.currentPitchHz > 10.0f) {
            basePitchHz_ = ctx.currentPitchHz;
        }
        if (ctx.isTriggered) {
            trigger(ctx.triggerVelocity);
        }
        if (!right || right == left) {
            for (int i = 0; i < numSamples; ++i) {
                float l = 0.0f, r = 0.0f;
                renderSampleStereo(basePitchHz_, l, r);
                left[i] = 0.5f * (l + r);
            }
            FastMath::sanitizeBuffer(left, numSamples);
            return;
        }
        for (int i = 0; i < numSamples; ++i) {
            float l = 0.0f, r = 0.0f;
            renderSampleStereo(basePitchHz_, l, r);
            left[i]  = l;
            right[i] = r;
        }
        FastMath::sanitizeBuffer(left, numSamples);
        FastMath::sanitizeBuffer(right, numSamples);
    }

protected:
    virtual void onPrepare(float /*sampleRate*/) noexcept {}
    virtual void onTrigger(float /*velocity*/) noexcept {}

    float sampleRate_ = 48000.0f;
    float invSr_ = 1.0f / 48000.0f;
    float basePitchHz_ = 110.0f;
    int midiNote_ = 45;
    float velocity_ = 1.0f;
};

} // namespace TbdAudio::Oscillators
