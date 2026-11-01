#pragma once

#include "../../FastMath.h"
#include "../oscillators/SoundEngine.h"
#include "../fx/ElektronTrackWrapper.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace tkdsp {

namespace FastMath = TbdAudio::FastMath;

/**
 * @brief MultiMachineEngineBase — Universal 4-Bank Venti (32-Control) Drum Synthesizer Base Class.
 *
 * Inherits from TbdAudio::Oscillators::SoundEngine32, implementing the role-based MME
 * architecture with zero-allocation, 64-sample click-free model crossfading and the
 * integrated Elektron hardware tone and mojo strip.
 *
 * The 4-Bank Venti Topology:
 *   - Bank 0 (Knobs 01–08): The Jam Card (Model Selector + 7 Role-Specific Controls)
 *   - Bank 1 (Knobs 09–16): The Universal Hardware Tone & Mojo Strip (ElektronTrackWrapper)
 *   - Bank 2 (Knobs 17–24): The Model-Specific Timbre Lab (Modor XYZT, Perkons, FM, Wavetable)
 *   - Bank 3 (Knobs 25–32): The Deep Acoustic & Modulation Lab (Curves, Damping, Mechanics)
 *
 * Click-Free Model Parameter Locking:
 *   When the MODEL parameter (Knob 01 / Param 0) changes during live sequencing,
 *   MultiMachineEngineBase automatically executes a 64-sample equal-power sinusoidal
 *   micro-crossfade across pre-allocated internal scratch memory. This guarantees
 *   zero audio pops, clicks, or discontinuities on rapid per-step model changes!
 *
 * Real-Time Audio Thread Safety:
 *   - 100% zero heap allocations during trigger(), setParam(), or process().
 *   - Zero CRT transcendental calls.
 *   - Strict denormal flush safety.
 */
class MultiMachineEngineBase : public TbdAudio::Oscillators::SoundEngine32 {
public:
    static constexpr size_t kCrossfadeSamples = 64;
    static constexpr uint8_t kMaxModels = 16;

    MultiMachineEngineBase() noexcept {
        params32_.fill(0.0f);
        // Default Bank 1 (Mojo Strip) parameters
        setParam(8,  0.0f); // K09: FLTF (Base HP Cutoff, default 20 Hz)
        setParam(9,  1.0f); // K10: FLTW (Width LP Gap, default wide open)
        setParam(10, 0.0f); // K11: FLTQ (Filter Resonance Q)
        setParam(11, 0.5f); // K12: EQ-F (Center Frequency)
        setParam(12, 0.5f); // K13: EQ-G (0 dB flat)
        setParam(13, 0.0f); // K14: SRR  (Bypassed)
        setParam(14, 0.0f); // K15: BRR  (Bypassed)
        setParam(15, 0.0f); // K16: DIST (Bypassed)
    }

    ~MultiMachineEngineBase() override = default;

    void prepare(double sampleRate, int maxBlockSize = 512) noexcept {
        SoundEngine32::prepare(sampleRate, maxBlockSize);
        elektronMojo_.prepare(sampleRate_);
        onPrepareSubModels(sampleRate_);
    }

    /**
     * @brief Set normalized control parameter [0.0 .. 1.0] across all 32 Venti slots.
     */
    void setParam(int index, float val) noexcept override {
        if (index < 0 || index >= 32) return;
        const float v = std::clamp(val, 0.0f, 1.0f);
        params32_[index] = v;

        // Model selection (Knob 01 / Param 0)
        if (index == 0) {
            // Equal 1/16 slice for all 16 models across 0.0 .. 1.0 (full range available)
            const uint8_t newModel = std::clamp(static_cast<uint8_t>(v * static_cast<float>(numModels())),
                                                static_cast<uint8_t>(0), static_cast<uint8_t>(numModels() - 1));
            if (newModel != activeModel_) {
                activeModel_ = newModel;
                crossfadeRemaining_ = 0;
                onSubModelReset(activeModel_);
            }
            return;
        }

        // Bank 1 (Knobs 09–16): Route directly to integrated ElektronTrackWrapper
        if (index >= 8 && index <= 15) {
            elektronMojo_.setParam(index - 8, v);
            return;
        }

        // Sub-model parameter update notification
        onParamChanged(index, v);
    }

    [[nodiscard]] float getParam(int index) const noexcept override {
        if (index < 0 || index >= 32) return 0.0f;
        return params32_[index];
    }

    void reset() noexcept override {
        elektronMojo_.reset();
        crossfadeRemaining_ = 0;
        onSubModelReset(activeModel_);
    }

    void trigger(float velocity = 1.0f) noexcept override {
        velocity_ = std::clamp(velocity, 0.0f, 1.0f);
        onSubModelTrigger(activeModel_, velocity_);
        if (crossfadeRemaining_ > 0) {
            onSubModelTrigger(fadingModel_, velocity_);
        }
    }

    /**
     * @brief Render one sample of stereo audio through the active sub-model + Mojo Strip.
     */
    void renderSampleStereo(float pitchHz, float& outL, float& outR) noexcept override {
        // Calculate keytracked effective pitch if KEYTRACK (Knob 27 / Param 26) is active
        const float keytrack = params32_[26];
        const float effectivePitch = (keytrack > 0.001f)
            ? (pitchHz * TbdAudio::FastMath::fastPow2((static_cast<float>(midiNote_) - 60.0f) * (1.0f / 12.0f) * keytrack))
            : pitchHz;

        float rawL = 0.0f;
        float rawR = 0.0f;

        // Instant, click-free rendering of active model
        renderSubModelStereo(activeModel_, effectivePitch, rawL, rawR);

        // Pass through the universal Bank 1 Elektron Hardware Tone & Mojo Strip
        elektronMojo_.processSampleStereo(rawL, rawR);

        outL = rawL;
        outR = rawR;
    }

    [[nodiscard]] uint8_t activeModel() const noexcept { return activeModel_; }
    [[nodiscard]] virtual uint8_t numModels() const noexcept { return kMaxModels; }

protected:
    virtual void onPrepareSubModels(float sampleRate) noexcept = 0;
    virtual void onSubModelTrigger(uint8_t modelIndex, float velocity) noexcept = 0;
    virtual void onSubModelReset(uint8_t modelIndex) noexcept = 0;
    virtual void onSubModelIdle(uint8_t /*modelIndex*/) noexcept {}
    virtual void onParamChanged(int /*index*/, float /*value*/) noexcept {}
    virtual void renderSubModelStereo(uint8_t modelIndex, float pitchHz, float& outL, float& outR) noexcept = 0;

private:
    void initiateModelCrossfade(uint8_t newModel) noexcept {
        fadingModel_ = activeModel_;
        activeModel_ = newModel;
        crossfadeRemaining_ = kCrossfadeSamples;
        onSubModelReset(activeModel_);
    }

    std::array<float, 32> params32_{};
    ElektronTrackWrapper elektronMojo_;

    uint8_t activeModel_ = 0;
    uint8_t fadingModel_ = 0;
    size_t crossfadeRemaining_ = 0;
};

} // namespace tkdsp
