#pragma once
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <vector>
#include "../../FastMath.h"

namespace TbdAudio {

constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = 6.28318530717958647692f;

struct BlockContext {
    float sampleRate = 44100.0f;
    float invSr = 1.0f / 44100.0f;
    float bpm = 120.0f;
    // Shared contextual signals passed down the chain
    float triggerVelocity = 1.0f;
    bool isTriggered = false;
    float currentPitchHz = 65.4064f; // C2 default
    int currentMidiNote = 36;
    float carrier1PitchHz = 65.4064f;
    float carrier2PitchHz = 65.4064f;
    int pitchEnv1Target = 0; // 0=Off, 1=Carrier, 2=Modulator, 3=Both
    int pitchEnv2Target = 0; // 0=Off, 1=Carrier, 2=Modulator, 3=Both

    // Inter-block modulation buffers
    std::vector<float> mod1Signal;
    std::vector<float> mod2Signal;
    std::vector<float> pitchEnv1Signal;
    std::vector<float> pitchEnv2Signal;
    std::vector<float> filterEnv1Signal;
    std::vector<float> filterEnv2Signal;
    std::vector<float> filterEnv3Signal;
    std::vector<float> ampEnvSignal;

    // Velocity modulation state
    float curvedVelocity = 1.0f;
    float velDecayMod = 0.0f;
    float velDepthMod = 0.0f;
    float velVolumeGain = 1.0f;

    // Key tracking modulation state
    float curvedKeyNote = 0.5f;
    float keyDecayMod = 0.0f;
    float keyDepthMod = 0.0f;
    float keyVolumeGain = 1.0f;

    // Mod Envelope signals
    std::vector<float> modEnv1Signal;
    std::vector<float> modEnv2Signal;
    std::vector<float> modEnv3Signal;

    // Slop modulation state (independent stepped bipolar random offsets drawn on each trigger)
    // 1. Frequency (pitch and filters)
    float slopCarrier1Pitch = 0.0f;
    float slopCarrier2Pitch = 0.0f;
    float slopMod1Freq = 0.0f;
    float slopMod1Filter = 0.0f;
    float slopMod2Freq = 0.0f;
    float slopMod2Filter = 0.0f;
    float slopDriveFilter = 0.0f;
    float slopWaveFolderFilter = 0.0f;
    float slopNoiseShRate = 0.0f;
    float slopNoiseFilter = 0.0f;
    float slopFilter1Cutoff = 0.0f;
    float slopFilter2Cutoff = 0.0f;
    float slopFilter3Cutoff = 0.0f;
    float slopFXFilterCutoff = 0.0f;
    float slopRingModRate = 0.0f;
    float slopCombDamp = 0.0f;
    float slopCombCutoff = 0.0f;
    float slopPhaseSmearCutoff = 0.0f;
    float slopEQFreq = 0.0f;
    float slopEQFilter = 0.0f;

    // 2. Envelope Depths
    float slopPitchEnv1Depth = 0.0f;
    float slopPitchEnv2Depth = 0.0f;
    float slopFilterEnv1Depth = 0.0f;
    float slopFilterEnv2Depth = 0.0f;
    float slopFilterEnv3Depth = 0.0f;

    // 3. Envelope Decays
    float slopPitchEnv1Decay = 0.0f;
    float slopPitchEnv2Decay = 0.0f;
    float slopNoiseDecay = 0.0f;
    float slopFilterEnv1Decay = 0.0f;
    float slopFilterEnv2Decay = 0.0f;
    float slopFilterEnv3Decay = 0.0f;
    float slopAmpEnvDecay = 0.0f;

    // 4. Pan
    float slopAmpPan = 0.0f;
};

class DSPBlock {
public:
    virtual ~DSPBlock() = default;
    virtual void init(const BlockContext& ctx) = 0;
    virtual void trigger(float /*velocity*/) {}
    virtual void reset() {}
    
    // Set parameter 0..3 (normalized 0.0 to 1.0)
    virtual void setParam(int index, float value) {
        if (index >= 0 && index < 4) {
            baseParams[index] = value;
            params[index] = std::clamp(baseParams[index] + modParams[index], 0.0f, 1.0f);
        }
    }

    // Set modulation offset 0..3 (bipolar)
    virtual void setParamModulation(int index, float offset) {
        if (index >= 0 && index < 4) {
            modParams[index] = offset;
            params[index] = std::clamp(baseParams[index] + modParams[index], 0.0f, 1.0f);
        }
    }

    virtual float getParam(int index) const {
        return (index >= 0 && index < 4) ? baseParams[index] : 0.0f;
    }

    // Process mono buffer in-place
    virtual void process(float* buffer, int numSamples, BlockContext& ctx) = 0;

    // Process stereo buffer in-place
    virtual void processStereo(float* left, float* right, int numSamples, BlockContext& ctx) {
        process(left, numSamples, ctx);
        if (right != nullptr && right != left) {
            std::copy(left, left + numSamples, right);
        }
    }

protected:
    float baseParams[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float modParams[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float params[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
};

} // namespace TbdAudio
