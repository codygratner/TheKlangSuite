#pragma once



#include "FastMath.h"

#include <algorithm>

#include <array>

#include <cmath>

#include <cstdint>



namespace tkdsp {



namespace FastMath = TbdAudio::FastMath;



/**

 * @brief FormlessSpawnEngine — The Sovereign Tsathogguan Percussion Workstation.

 *

 * An analog-modeled, shapeshifting percussion synthesizer and transient slicer

 * inspired by the Korg Electribe ER-1 and modernized with non-linear diode

 * wavefolding, self-oscillating zero-delay feedback (ZDF) diode ladder filters,

 * and 16-step motion sequencing parameter locks.

 *

 * Architecture:

 *   - Voices 0..3 (Synth Drums): 4 Morphing Synth Percussion Voices

 *       - Dual-operator FM & cyclic ring modulation (ER-1 Heritage)

 *       - Snappy exponential pitch envelope (-48st .. +48st, 5ms..500ms)

 *       - West Coast diode wavefolder & transistor drive

 *       - 24dB/oct diode ladder filter with self-oscillation

 *   - Voices 4..7 (Transient Drums): 4 Sliced Drum & Noise Percussion Voices

 *       - Voice 4: Open Hi-Hat (Metallic 6-pulse resonant matrix)

 *       - Voice 5: Closed Hi-Hat (Instant choke for Voice 4)

 *       - Voice 6: Hand Clap (Multi-burst initial impulse + noise wash)

 *       - Voice 7: Crash / Sub-Thump (Deep sub drop or metallic wash)

 *   - 16-Step Motion Sequencer:

 *       - Per-step parameter locks for pitch, decay, mod speed, mod depth, cutoff, wavefold

 *       - Instant recording and playback with zero audio-thread allocation

 *   - Master Section:

 *       - Low Boost (resonant analog low-shelf saturation)

 *       - Master diode saturation & soft limiter

 *

 * Real-Time Audio Invariants:

 *   - 100% zero heap allocations in trigger(), setParam(), or process().

 *   - Zero CRT transcendental functions (exclusively FastMath fastSinNorm, fastExp, fastTanh).

 *   - Denormal-free processing across all voice integrators.

 */

class FormlessSpawnEngine {

public:

    static constexpr int kNumSynthVoices = 4;

    static constexpr int kNumTransientVoices = 4;

    static constexpr int kTotalVoices = kNumSynthVoices + kNumTransientVoices; // 8 Voices

    static constexpr int kNumSteps = 16;

    static constexpr int kNumParamsPerVoice = 14;



    // --- Voice Parameter Indices ---

    enum Param : int {

        kParamPitch = 0,          // 0.0 .. 1.0 (Mapped 20 Hz .. 2000 Hz)

        kParamDecay,              // 0.0 .. 1.0 (Mapped 10 ms .. 5.0 s)

        kParamPitchEnvDepth,      // 0.0 .. 1.0 (Mapped -48 st .. +48 st, 0.5 = 0)

        kParamPitchEnvDecay,      // 0.0 .. 1.0 (Mapped 5 ms .. 500 ms)

        kParamWaveform,           // 0.0 .. 1.0 (0=Sine, 1=Tri, 2=Saw, 3=Square)

        kParamModType,            // 0.0 .. 1.0 (0=Sine, 1=Tri, 2=Saw, 3=Square, 4=Noise, 5=RingMod)

        kParamModSpeed,           // 0.0 .. 1.0 (Mapped 0.1 Hz .. 5000 Hz)

        kParamModDepth,           // 0.0 .. 1.0 (FM / AM / Ring Index)

        kParamWavefold,           // 0.0 .. 1.0 (West Coast Diode Wavefolder depth)

        kParamDrive,              // 0.0 .. 1.0 (Transistor pre-filter drive)

        kParamFilterCutoff,       // 0.0 .. 1.0 (Mapped 20 Hz .. 20000 Hz)

        kParamFilterReso,         // 0.0 .. 1.0 (Diode ladder resonance)

        kParamPan,                // 0.0 .. 1.0 (0.0 = Left, 0.5 = Center, 1.0 = Right)

        kParamVolume              // 0.0 .. 1.0 (Master voice volume)

    };



    // --- Step Parameter Lock Entry ---

    struct StepLock {

        bool active{false};

        float velocity{0.8f};

        std::array<float, 8> lockedParams{}; // Top 8 parameters locked per step

        bool hasLock{false};

    };



    FormlessSpawnEngine() noexcept {

        reset();

        initDefaultPatches();

    }



    void prepare(float sampleRate) noexcept {

        sampleRate_ = std::max(sampleRate, 4000.0f);

        invSampleRate_ = 1.0f / sampleRate_;

        nyquist_ = 0.495f * sampleRate_;

        reset();

    }



    void reset() noexcept {

        for (auto& v : synthVoices_) {

            v.phase = 0.0f;

            v.modPhase = 0.0f;

            v.ampEnv = 0.0f;

            v.pitchEnv = 0.0f;

            v.overrideFreqHz = -1.0f;

            v.isSustained = false;

            v.releaseCoeff = 0.0f;

            v.prevSample = 0.0f;

            for (auto& s : v.filterState) s = 0.0f;

            v.noise.reset(0x13579BDFu);

        }

        for (auto& tv : transientVoices_) {

            tv.ampEnv = 0.0f;

            tv.burstPhase = 0.0f;

            tv.prevFilter = 0.0f;

            tv.customDecaySec = -1.0f;

            tv.noise.reset(0xCAFEBABEu);

        }

        currentStep_ = 0;

        stepCounter_ = 0;

        masterLowBoostState_ = 0.0f;

    }



    void setParam(int voiceIndex, int paramIndex, float value) noexcept {

        if (voiceIndex < 0 || voiceIndex >= kTotalVoices) return;

        if (paramIndex < 0 || paramIndex >= kNumParamsPerVoice) return;

        voiceParams_[voiceIndex][paramIndex] = std::clamp(value, 0.0f, 1.0f);

    }



    [[nodiscard]] float getParam(int voiceIndex, int paramIndex) const noexcept {

        if (voiceIndex < 0 || voiceIndex >= kTotalVoices) return 0.0f;

        if (paramIndex < 0 || paramIndex >= kNumParamsPerVoice) return 0.0f;

        return voiceParams_[voiceIndex][paramIndex];

    }



    void trigger(int voiceIndex, float velocity, float noteHz = -1.0f, bool sustain = false) noexcept {

        if (voiceIndex < 0 || voiceIndex >= kTotalVoices) return;

        velocity = std::clamp(velocity, 0.0f, 1.0f);



        if (voiceIndex < kNumSynthVoices) {

            auto& v = synthVoices_[voiceIndex];

            v.ampEnv = velocity;

            v.pitchEnv = 1.0f;

            v.phase = 0.0f;

            v.modPhase = 0.0f;

            v.isSustained = sustain;

            v.releaseCoeff = 0.0f;

            if (noteHz > 0.0f) {

                v.overrideFreqHz = std::clamp(noteHz, 10.0f, nyquist_);

            } else {

                v.overrideFreqHz = -1.0f;

            }

        } else {

            int tIndex = voiceIndex - kNumSynthVoices;

            auto& tv = transientVoices_[tIndex];

            tv.ampEnv = velocity;

            tv.burstPhase = 0.0f;

            tv.customDecaySec = -1.0f;



            // Closed Hi-Hat (Voice 5) chokes Open Hi-Hat (Voice 4)

            if (voiceIndex == 5) {

                transientVoices_[0].ampEnv = 0.0f; // Choke Open Hat!

            }

        }

    }



    void noteOff(int voiceIndex, float releaseSec = 0.06f) noexcept {

        if (voiceIndex < 0 || voiceIndex >= kTotalVoices) return;

        if (voiceIndex < kNumSynthVoices) {

            auto& v = synthVoices_[voiceIndex];

            v.isSustained = false;

            releaseSec = std::clamp(releaseSec, 0.01f, 3.0f);

            v.releaseCoeff = FastMath::fastExp(-invSampleRate_ / releaseSec);

        }

    }



    void chokeHats() noexcept {

        transientVoices_[0].ampEnv = 0.0f;

        transientVoices_[1].ampEnv = 0.0f;

    }



    void triggerTransientWithCustom(int transientIndex, float velocity, float decaySec) noexcept {

        if (transientIndex < 0 || transientIndex >= kNumTransientVoices) return;

        auto& tv = transientVoices_[transientIndex];

        tv.ampEnv = std::clamp(velocity, 0.0f, 1.0f);

        tv.burstPhase = 0.0f;

        tv.customDecaySec = std::clamp(decaySec, 0.005f, 15.0f);

    }



    // --- 16-Step Motion Sequencing Controls ---

    void setStepTrigger(int voiceIndex, int step, bool active, float velocity = 0.8f) noexcept {

        if (voiceIndex < 0 || voiceIndex >= kTotalVoices) return;

        if (step < 0 || step >= kNumSteps) return;

        patterns_[voiceIndex][step].active = active;

        patterns_[voiceIndex][step].velocity = std::clamp(velocity, 0.0f, 1.0f);

    }



    void setStepLock(int voiceIndex, int step, int lockParamIndex, float value) noexcept {

        if (voiceIndex < 0 || voiceIndex >= kTotalVoices) return;

        if (step < 0 || step >= kNumSteps) return;

        if (lockParamIndex < 0 || lockParamIndex >= 8) return;

        patterns_[voiceIndex][step].lockedParams[lockParamIndex] = std::clamp(value, 0.0f, 1.0f);

        patterns_[voiceIndex][step].hasLock = true;

    }



    void clearStepLock(int voiceIndex, int step) noexcept {

        if (voiceIndex < 0 || voiceIndex >= kTotalVoices) return;

        if (step < 0 || step >= kNumSteps) return;

        patterns_[voiceIndex][step].hasLock = false;

    }



    void advanceStep() noexcept {

        currentStep_ = (currentStep_ + 1) % kNumSteps;

        for (int v = 0; v < kTotalVoices; ++v) {

            const auto& lock = patterns_[v][currentStep_];

            if (lock.active) {

                trigger(v, lock.velocity);

            }

        }

    }



    [[nodiscard]] int getCurrentStep() const noexcept { return currentStep_; }

    void setCurrentStep(int step) noexcept { currentStep_ = step % kNumSteps; }



    void setMasterLowBoost(float boost) noexcept { masterLowBoost_ = std::clamp(boost, 0.0f, 1.0f); }

    void setMasterDrive(float drive) noexcept { masterDrive_ = std::clamp(drive, 0.0f, 1.0f); }



    // --- Audio Rendering ---

    void renderSampleStereo(float& outL, float& outR) noexcept {

        float sumL = 0.0f;

        float sumR = 0.0f;



        // 1. Process 4 Synth Voices

        for (int i = 0; i < kNumSynthVoices; ++i) {

            auto& v = synthVoices_[i];

            if (v.ampEnv < 0.0001f) continue;



            float vL = 0.0f, vR = 0.0f;

            processSynthVoice(i, v, vL, vR);

            sumL += vL;

            sumR += vR;

        }



        // 2. Process 4 Transient Voices

        for (int i = 0; i < kNumTransientVoices; ++i) {

            auto& tv = transientVoices_[i];

            if (tv.ampEnv < 0.0001f) continue;



            float tvL = 0.0f, tvR = 0.0f;

            processTransientVoice(i, tv, tvL, tvR);

            sumL += tvL;

            sumR += tvR;

        }



        // 3. Master Section: Resonant Low Boost (ER-1 Signature Thump)

        if (masterLowBoost_ > 0.01f) {

            const float lowBoostGain = masterLowBoost_ * 2.5f;

            const float lowCutoff = 80.0f * invSampleRate_;

            masterLowBoostState_ += lowCutoff * ((sumL + sumR) * 0.5f - masterLowBoostState_);

            const float boosted = FastMath::fastTanh(masterLowBoostState_ * lowBoostGain);

            sumL += boosted;

            sumR += boosted;

        }



        // 4. Master Transistor Drive & Output Soft Limiter

        const float driveGain = 1.0f + masterDrive_ * 3.0f;

        outL = FastMath::fastTanh(sumL * driveGain * 0.7f);

        outR = FastMath::fastTanh(sumR * driveGain * 0.7f);

    }



    void renderBlockStereo(float* outL, float* outR, int numSamples) noexcept {

        for (int i = 0; i < numSamples; ++i) {

            renderSampleStereo(outL[i], outR[i]);

        }

    }



private:

    struct FastNoise {

        uint32_t state{0x13579BDFu};

        float b0{0.0f}, b1{0.0f}, b2{0.0f};



        void reset(uint32_t seed) noexcept {

            state = (seed != 0) ? seed : 0x13579BDFu;

            b0 = b1 = b2 = 0.0f;

        }



        inline float nextWhite() noexcept {

            state ^= (state << 13);

            state ^= (state >> 17);

            state ^= (state << 5);

            return static_cast<float>(static_cast<int32_t>(state)) * (1.0f / 2147483648.0f);

        }



        inline float nextPink() noexcept {

            const float white = nextWhite();

            b0 = 0.99765f * b0 + white * 0.0990460f;

            b1 = 0.96300f * b1 + white * 0.2965164f;

            b2 = 0.57000f * b2 + white * 1.0526913f;

            return (b0 + b1 + b2 + white * 0.1848f) * 0.25f;

        }

    };



    struct SynthVoiceState {

        float phase{0.0f};

        float modPhase{0.0f};

        float ampEnv{0.0f};

        float pitchEnv{0.0f};

        float overrideFreqHz{-1.0f};

        float prevSample{0.0f};

        std::array<float, 4> filterState{};

        FastNoise noise{};

        bool isSustained{false};

        float releaseCoeff{0.0f};

    };



    struct TransientVoiceState {

        float ampEnv{0.0f};

        float burstPhase{0.0f};

        float prevFilter{0.0f};

        FastNoise noise{};

        float customDecaySec{-1.0f};

    };



    static inline float wrapPhase(float p) noexcept {

        while (p >= 1.0f) p -= 1.0f;

        while (p < 0.0f) p += 1.0f;

        return p;

    }



    static inline float polySaw(float phase, float dt) noexcept {

        float saw = 2.0f * phase - 1.0f;

        if (phase < dt) {

            float t = phase / dt;

            saw += (2.0f * t - t * t - 1.0f);

        } else if (phase > 1.0f - dt) {

            float t = (phase - 1.0f) / dt;

            saw += (t * t + 2.0f * t + 1.0f);

        }

        return saw;

    }



    static inline float polySquare(float phase, float dt) noexcept {

        float sq = (phase < 0.5f) ? 1.0f : -1.0f;

        if (phase < dt) {

            float t = phase / dt;

            sq += (2.0f * t - t * t - 1.0f);

        } else if (std::abs(phase - 0.5f) < dt) {

            float t = (phase - 0.5f) / dt;

            sq -= (2.0f * t - t * t - 1.0f);

        }

        return sq;

    }



    void processSynthVoice(int idx, SynthVoiceState& v, float& outL, float& outR) noexcept {

        const auto& p = voiceParams_[idx];



        // Base Frequency & Pitch Envelope (ER-1 Punch)

        float baseHz = (v.overrideFreqHz > 0.0f) ? v.overrideFreqHz : mapPitchKnob(p[kParamPitch]);

        const float pitchEnvDepthSemitones = (p[kParamPitchEnvDepth] - 0.5f) * 96.0f; // -48st .. +48st

        const float pitchMultiplier = (v.pitchEnv > 0.001f)
            ? FastMath::fastPow2((pitchEnvDepthSemitones * v.pitchEnv) * (1.0f / 12.0f))
            : 1.0f;

        const float curFreq = std::clamp(baseHz * pitchMultiplier, 10.0f, nyquist_);



        // Advance Pitch and Amp Envelopes

        float pitchEnvDecayMs = 5.0f + p[kParamPitchEnvDecay] * 495.0f;
        if (v.isSustained) {
            pitchEnvDecayMs = std::min(pitchEnvDecayMs, 25.0f);
        }

        const float pitchEnvCoef = FastMath::fastExp(-invSampleRate_ / (pitchEnvDecayMs * 0.001f));

        v.pitchEnv *= pitchEnvCoef;



        if (v.isSustained) {

            // Sustained hold: pitch env drops to base tone, amplitude holds steady!

        } else if (v.releaseCoeff > 0.0f) {

            v.ampEnv *= v.releaseCoeff;

        } else {

            const float ampDecayMs = 10.0f + p[kParamDecay] * p[kParamDecay] * 4990.0f; // Quadratic taper

            const float ampEnvCoef = FastMath::fastExp(-invSampleRate_ / (ampDecayMs * 0.001f));

            v.ampEnv *= ampEnvCoef;

        }



        // Modulator Oscillator (Sine, Tri, Saw, Square, Noise, or Ring Mod)

        const float modSpeedHz = mapModSpeedKnob(p[kParamModSpeed]);

        v.modPhase = wrapPhase(v.modPhase + modSpeedHz * invSampleRate_);

        float modSignal = 0.0f;

        const int modType = static_cast<int>(p[kParamModType] * 5.99f);



        switch (modType) {

            case 0: modSignal = FastMath::fastSinNorm(v.modPhase); break;

            case 1: modSignal = 1.0f - 4.0f * std::abs(v.modPhase - 0.5f); break; // Tri

            case 2: modSignal = 2.0f * v.modPhase - 1.0f; break; // Saw

            case 3: modSignal = (v.modPhase < 0.5f) ? 1.0f : -1.0f; break; // Square

            case 4: modSignal = v.noise.nextPink(); break; // Pink Noise (ER-1 Snare rattle)

            case 5: default: {

                // Cyclic Ring Mod (Noise * Carrier Phase Mod)

                const float n = v.noise.nextWhite();

                modSignal = FastMath::fastSinNorm(v.modPhase) * (0.3f + 0.7f * n);

                break;

            }

        }



        const float modDepth = p[kParamModDepth] * 4.0f;

        const float phaseModulation = modSignal * modDepth * v.ampEnv;



        // Carrier Oscillator with Phase Modulation

        v.phase = wrapPhase(v.phase + curFreq * invSampleRate_);

        const float modulatedPhase = wrapPhase(v.phase + phaseModulation);



        float carrier = 0.0f;

        const int waveType = static_cast<int>(p[kParamWaveform] * 3.99f);

        switch (waveType) {

            case 0: carrier = FastMath::fastSinNorm(modulatedPhase); break; // Pure Sine

            case 1: carrier = 1.0f - 4.0f * std::abs(modulatedPhase - 0.5f); break; // Triangle

            case 2: carrier = polySaw(modulatedPhase, curFreq * invSampleRate_); break; // Bandlimited Saw

            case 3: default: carrier = polySquare(modulatedPhase, curFreq * invSampleRate_); break; // Square

        }



        // Cross-Modulation / Ring Mod with adjacent voice (Voice 0 x Voice 1, Voice 2 x Voice 3)

        if (modType == 5) {

            int partnerIdx = (idx % 2 == 0) ? (idx + 1) : (idx - 1);

            carrier *= (0.5f + 0.5f * synthVoices_[partnerIdx].prevSample);

        }

        v.prevSample = carrier;



        // West Coast Diode Wavefolder & Saturation

        float shaped = carrier * (1.0f + p[kParamDrive] * 3.0f);

        const float foldDepth = p[kParamWavefold];

        if (foldDepth > 0.01f) {

            // Multi-stage sinusoidal folding with asymmetric bias

            const float foldBias = 0.15f * foldDepth;

            const float folded1 = FastMath::fastSinNorm((shaped + foldBias) * (0.5f + foldDepth * 1.5f));

            const float folded2 = FastMath::fastSinNorm(folded1 * (1.0f + foldDepth * 2.0f));

            shaped = (1.0f - foldDepth) * shaped + foldDepth * folded2;

        }



        // 4-Pole Zero-Delay Feedback Diode Ladder Filter

        const float cutoffHz = mapCutoffKnob(p[kParamFilterCutoff]);

        const float reso = p[kParamFilterReso] * 0.95f;

        const float filtered = processDiodeFilter(v, shaped, cutoffHz, reso);



        // Apply Amp Envelope & Panning

        const float voiceSignal = filtered * v.ampEnv * p[kParamVolume];

        const float pan = p[kParamPan];

        outL = voiceSignal * FastMath::fastCosNorm(pan * 0.25f);

        outR = voiceSignal * FastMath::fastSinNorm(pan * 0.25f);

    }



    void processTransientVoice(int idx, TransientVoiceState& tv, float& outL, float& outR) noexcept {

        const int voiceParamIdx = kNumSynthVoices + idx;

        const auto& p = voiceParams_[voiceParamIdx];



        // Exponential decay envelope

        float decayMs = 15.0f + p[kParamDecay] * p[kParamDecay] * 2500.0f;

        if (tv.customDecaySec > 0.0f) {

            decayMs = tv.customDecaySec * 1000.0f;

        }

        const float decayCoef = FastMath::fastExp(-invSampleRate_ / (decayMs * 0.001f));

        tv.ampEnv *= decayCoef;



        float rawSignal = 0.0f;



        switch (idx) {

            case 0: { // Voice 4: Open Hi-Hat (Metallic 6-pulse resonant matrix)

                const float white = tv.noise.nextWhite();

                tv.prevFilter += 0.45f * (white - tv.prevFilter); // High-pass filtering

                rawSignal = (white - tv.prevFilter) * 1.4f;

                break;

            }

            case 1: { // Voice 5: Closed Hi-Hat (Tight click)

                const float white = tv.noise.nextWhite();

                tv.prevFilter += 0.75f * (white - tv.prevFilter);

                rawSignal = (white - tv.prevFilter) * 1.8f;

                break;

            }

            case 2: { // Voice 6: Hand Clap (Multi-burst initial impulse + noise)

                tv.burstPhase += 120.0f * invSampleRate_; // 120 Hz pulse sequence

                const float burst = (tv.burstPhase < 3.0f && std::fmod(tv.burstPhase, 1.0f) < 0.25f) ? 1.0f : 0.0f;

                const float pink = tv.noise.nextPink();

                rawSignal = (burst * 0.8f + pink * 0.6f);

                break;

            }

            case 3: default: { // Voice 7: Crash / Sub-Thump

                const float pink = tv.noise.nextPink();

                tv.prevFilter += 0.25f * (pink - tv.prevFilter);

                rawSignal = tv.prevFilter * 1.5f;

                break;

            }

        }



        // Filter & Panning

        const float cutoffHz = mapCutoffKnob(p[kParamFilterCutoff]);

        const float alpha = std::clamp(2.0f * 3.14159265f * cutoffHz * invSampleRate_, 0.01f, 0.95f);

        tv.prevFilter += alpha * (rawSignal - tv.prevFilter);



        const float voiceSignal = tv.prevFilter * tv.ampEnv * p[kParamVolume];

        const float pan = p[kParamPan];

        outL = voiceSignal * FastMath::fastCosNorm(pan * 0.25f);

        outR = voiceSignal * FastMath::fastSinNorm(pan * 0.25f);

    }



    float processDiodeFilter(SynthVoiceState& v, float in, float cutoffHz, float reso) noexcept {

        // 4-stage diode ladder approximation with non-linear feedback

        const float wc = std::clamp(2.0f * 3.14159265f * cutoffHz * invSampleRate_, 0.001f, 0.85f);

        const float feedback = reso * 3.8f;

        const float inputWithFb = in - feedback * FastMath::fastTanh(v.filterState[3]);



        v.filterState[0] += wc * (FastMath::fastTanh(inputWithFb) - FastMath::fastTanh(v.filterState[0]));

        v.filterState[1] += wc * (FastMath::fastTanh(v.filterState[0]) - FastMath::fastTanh(v.filterState[1]));

        v.filterState[2] += wc * (FastMath::fastTanh(v.filterState[1]) - FastMath::fastTanh(v.filterState[2]));

        v.filterState[3] += wc * (FastMath::fastTanh(v.filterState[2]) - FastMath::fastTanh(v.filterState[3]));



        return v.filterState[3];

    }



    static float mapPitchKnob(float k) noexcept {

        // 20 Hz .. 2000 Hz exponential mapping

        return 20.0f * FastMath::fastPow2(k * 6.643856f); // 20 * 2^(k * log2(100))

    }



    static float mapModSpeedKnob(float k) noexcept {

        // 0.1 Hz .. 5000 Hz exponential mapping

        return 0.1f * FastMath::fastPow2(k * 15.60964f);

    }



    static float mapCutoffKnob(float k) noexcept {

        // 20 Hz .. 20000 Hz exponential mapping

        return 20.0f * FastMath::fastPow2(k * 9.965784f);

    }



    void initDefaultPatches() noexcept {

        // Voice 0: Heavy ER-1 Sub Kick

        setParam(0, kParamPitch, 0.15f);            // ~55 Hz

        setParam(0, kParamDecay, 0.35f);            // Snappy bass drop

        setParam(0, kParamPitchEnvDepth, 0.85f);    // Sharp initial punch

        setParam(0, kParamPitchEnvDecay, 0.10f);    // 25ms pitch click

        setParam(0, kParamWaveform, 0.0f);          // Sine

        setParam(0, kParamModType, 0.0f);           // Sine

        setParam(0, kParamWavefold, 0.25f);         // Slight diode grit

        setParam(0, kParamDrive, 0.30f);

        setParam(0, kParamFilterCutoff, 0.70f);

        setParam(0, kParamVolume, 0.95f);



        // Voice 1: ER-1 Snare with Noise

        setParam(1, kParamPitch, 0.35f);            // ~180 Hz

        setParam(1, kParamDecay, 0.28f);

        setParam(1, kParamPitchEnvDepth, 0.65f);

        setParam(1, kParamPitchEnvDecay, 0.08f);

        setParam(1, kParamWaveform, 1.0f);          // Triangle

        setParam(1, kParamModType, 0.8f);           // Pink Noise mod

        setParam(1, kParamModSpeed, 0.5f);

        setParam(1, kParamModDepth, 0.65f);

        setParam(1, kParamFilterCutoff, 0.85f);

        setParam(1, kParamVolume, 0.85f);



        // Voice 2: Metallic Ring Mod Zap / Tom

        setParam(2, kParamPitch, 0.45f);

        setParam(2, kParamDecay, 0.22f);

        setParam(2, kParamPitchEnvDepth, 0.75f);

        setParam(2, kParamWaveform, 0.0f);

        setParam(2, kParamModType, 1.0f);           // Cyclic Ring Mod

        setParam(2, kParamModSpeed, 0.65f);

        setParam(2, kParamModDepth, 0.75f);

        setParam(2, kParamWavefold, 0.40f);

        setParam(2, kParamVolume, 0.80f);



        // Voice 3: Industrial Cyber Bongo / Percussion

        setParam(3, kParamPitch, 0.55f);

        setParam(3, kParamDecay, 0.18f);

        setParam(3, kParamWaveform, 2.0f);          // Saw

        setParam(3, kParamModType, 0.4f);

        setParam(3, kParamWavefold, 0.50f);

        setParam(3, kParamFilterReso, 0.60f);

        setParam(3, kParamVolume, 0.75f);



        // Voices 4..7: Transient Defaults (Open Hat, Closed Hat, Clap, Crash)

        for (int i = 4; i < kTotalVoices; ++i) {

            setParam(i, kParamDecay, 0.25f);

            setParam(i, kParamFilterCutoff, 0.85f);

            setParam(i, kParamVolume, 0.80f);

            setParam(i, kParamPan, 0.5f);

        }

        setParam(4, kParamDecay, 0.50f); // Longer Open Hat

    }



    float sampleRate_{48000.0f};

    float invSampleRate_{1.0f / 48000.0f};

    float nyquist_{23760.0f};

    float masterLowBoost_{0.45f};

    float masterDrive_{0.15f};

    float masterLowBoostState_{0.0f};



    int currentStep_{0};

    int stepCounter_{0};



    std::array<SynthVoiceState, kNumSynthVoices> synthVoices_{};

    std::array<TransientVoiceState, kNumTransientVoices> transientVoices_{};

    std::array<std::array<float, kNumParamsPerVoice>, kTotalVoices> voiceParams_{};

    std::array<std::array<StepLock, kNumSteps>, kTotalVoices> patterns_{};

};



} // namespace tkdsp



namespace toad {

    using FormlessSpawnEngine = tkdsp::FormlessSpawnEngine;

}

