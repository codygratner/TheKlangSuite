#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>
#include <algorithm>
#include "shoggoth_layout.h"
#include "ShoggothOscillator.h"
#include "ShoggothEnvelopes.h"
#include "ShoggothLaws.h"

namespace Shoggoth {

class Voice {
public:
    Voice() = default;

    void prepare(double sampleRate) noexcept {
        sampleRate_ = static_cast<float>(sampleRate);
        osc_.prepare(sampleRate);
        envs_.prepare(sampleRate);
        reset();
    }

    void reset() noexcept {
        state_.phase = VoicePhase::Idle;
        state_.activePartials = 126;
        state_.activeBlocks = 16;
        state_.voiceGain = 1.0f;
        state_.ageSamples = 0;
        state_.noteId = 0;
        stealFadeSamples_ = 0;
        stealFadeRemaining_ = 0;

        for (int k = 0; k < kMaxPartials; ++k) {
            state_.p.re[k] = 0.0f;
            state_.p.im[k] = 0.0f;
            state_.p.amp[k] = 0.0f;
            state_.p.ampTgt[k] = 0.0f;
            state_.p.baseHarmonic[k] = 0.0f;
            state_.p.ratio[k] = static_cast<float>(k + 1);
            state_.p.dOmega[k] = 0.0f;
            state_.p.envAssign[k] = static_cast<uint8_t>(k % 4);
        }
    }

    void noteOn(int midiNote, float velocity, float f0Hz,
                float pitchPunchDepthSt, bool modernMode, int cpuTier) noexcept {
        state_.midiNote = static_cast<uint8_t>(midiNote);
        state_.velocity = std::clamp(velocity, 0.01f, 1.0f);
        state_.f0Hz = f0Hz;
        state_.f0Tgt = f0Hz;
        state_.modernMode = modernMode;
        state_.phase = VoicePhase::Attack;
        state_.ageSamples = 0;
        stealFadeRemaining_ = 0;

        // Pitch punch: exponential drop
        state_.pitchPunchOffset = pitchPunchDepthSt;
        // ~15ms decay rate
        state_.pitchPunchDecay = std::exp(-1.0f / (0.015f * sampleRate_));

        // Set partial pool ceiling based on cpuTier
        int maxPool = 1024;
        if (cpuTier == 1) maxPool = 2048;
        else if (cpuTier == 2) maxPool = 2048;

        if (!modernMode) {
            state_.activePartials = 126;
        } else {
            // Adaptive Nyquist limit: f_k < 0.495 * fs
            const int nyqPartials = static_cast<int>((0.495f * sampleRate_) / std::max(20.0f, f0Hz));
            state_.activePartials = static_cast<uint32_t>(std::clamp(nyqPartials, 126, maxPool));
        }
        state_.activeBlocks = (state_.activePartials + 7) / 8;

        // Trigger DHG & AHDSR envelopes
        for (int e = 0; e < kNumDHGEnv; ++e) {
            envs_.trigger1987(state_.dhgRuntime[e], state_.dhgData[e]);
        }
        envs_.trigger1987(state_.ddaRuntime, state_.dhgData[0]);
        envs_.triggerAhdsr(state_.ampAhdsr);
        envs_.triggerAhdsr(state_.filterAhdsr);

        // Phase initialization: Schroeder turns for low crest factor
        for (int k = 0; k < static_cast<int>(state_.activePartials); ++k) {
            float phase = 0.0f;
            if (modernMode && k >= kMax1987Partials) {
                const float turns = Laws::computeSchroederPhaseTurns(k, state_.activePartials, 1.0f);
                phase = turns * Oscillator::kTwoPi;
            }
            state_.p.re[k] = 1e-4f * std::cos(phase);
            state_.p.im[k] = 1e-4f * std::sin(phase);
            state_.p.amp[k] = 1e-4f;
        }
    }

    void noteOff(float releaseVelocity) noexcept {
        if (state_.phase != VoicePhase::Idle) {
            state_.phase = VoicePhase::Release;
            for (int e = 0; e < kNumDHGEnv; ++e) {
                envs_.release1987(state_.dhgRuntime[e], state_.dhgData[e]);
            }
            envs_.release1987(state_.ddaRuntime, state_.dhgData[0]);
            envs_.releaseAhdsr(state_.ampAhdsr);
            envs_.releaseAhdsr(state_.filterAhdsr);
        }
    }

    void startStealFade() noexcept {
        state_.phase = VoicePhase::Steal;
        // 5ms cosine fade-out (240 samples at 48kHz)
        stealFadeSamples_ = static_cast<int>(0.005f * sampleRate_);
        stealFadeRemaining_ = stealFadeSamples_;
    }

    bool isVoiceActive() const noexcept {
        return state_.phase != VoicePhase::Idle;
    }

    uint64_t getAge() const noexcept { return state_.ageSamples; }
    int getMidiNote() const noexcept { return state_.midiNote; }
    VoicePhase getPhase() const noexcept { return state_.phase; }

    ShoggothVoiceState& getState() noexcept { return state_; }
    const ShoggothVoiceState& getState() const noexcept { return state_; }

    // ==============================================================================
    // Render Voice Block
    // ==============================================================================
    void renderBlock(float* outL, float* outR, int numSamples,
                     float filterCutoffHz, float filterReso, float spectralTilt,
                     float sourceMorph, float oddEvenBal, float inharmonicWarp,
                     float foldWallHz, float swarmK, int chorusMode,
                     float waterbedDepth, float oscSlop, float velBrightness,
                     float modalRelease,
                     float ampAttack, float ampDecay, float ampSustain, float ampRelease,
                     float filterEnvDepth) noexcept {
        if (state_.phase == VoicePhase::Idle) return;

        state_.ageSamples += numSamples;

        // Subblock processing loop (16 samples per iteration)
        int samplesRemaining = numSamples;
        int sampleOffset = 0;

        alignas(64) float sRe[kSub];
        alignas(64) float sIm[kSub];
        alignas(64) float subL[kSub];
        alignas(64) float subR[kSub];

        while (samplesRemaining > 0) {
            const int chunkSize = std::min(samplesRemaining, kSub);

            // 1. Advance Envelopes (Modern AHDSR vs Vintage 1987 DHG/DDA)
            float dhgEnvOut[kNumDHGEnv];
            float ampEnv = 0.0f;
            float filterEnv = 0.0f;

            if (state_.modernMode) {
                ampEnv = envs_.advanceAhdsr(state_.ampAhdsr,
                                            ampAttack, 0.0f, ampDecay,
                                            ampSustain, ampRelease,
                                            chunkSize, true);
                filterEnv = envs_.advanceAhdsr(state_.filterAhdsr,
                                               ampAttack, 0.0f, ampDecay,
                                               ampSustain, ampRelease,
                                               chunkSize, false);
                for (int e = 0; e < kNumDHGEnv; ++e) {
                    dhgEnvOut[e] = ampEnv;
                }
            } else {
                for (int e = 0; e < kNumDHGEnv; ++e) {
                    dhgEnvOut[e] = envs_.advance1987(state_.dhgRuntime[e], state_.dhgData[e], chunkSize);
                }
                ampEnv = envs_.advance1987(state_.ddaRuntime, state_.dhgData[0], chunkSize);
                filterEnv = dhgEnvOut[0];
            }

            if (state_.phase == VoicePhase::Release && ampEnv <= 0.0001f) {
                state_.phase = VoicePhase::Idle;
                break;
            }

            // 2. Evaluate Effective Fundamental f0 with Pitch Punch & Slop
            float effF0 = state_.f0Hz;
            if (std::abs(state_.pitchPunchOffset) > 0.01f) {
                effF0 *= std::pow(2.0f, state_.pitchPunchOffset / 12.0f);
                state_.pitchPunchOffset *= state_.pitchPunchDecay;
            }
            if (oscSlop > 0.001f) {
                effF0 *= (1.0f + (state_.p.psi[kGuard] * 0.0005f * oscSlop));
            }

            // 3. Compute Target Amplitudes for Active Partials
            float e126 = 0.0f;
            float eTot = 0.0f;

            const int activeLimit = std::min(static_cast<int>(state_.activePartials), kMaxPartials);
            for (int k = 0; k < activeLimit; ++k) {
                const float harmNum = static_cast<float>(k + 1);

                // Inharmonic warp: f_k = f0 * k^alpha
                state_.p.ratio[k] = (std::abs(inharmonicWarp - 1.0f) > 0.001f)
                    ? std::pow(harmNum, inharmonicWarp) : harmNum;

                const float fk = effF0 * state_.p.ratio[k];

                // Envelope modulation
                const uint8_t envIdx = state_.p.envAssign[k] & 0x03;
                const float envGain = dhgEnvOut[envIdx];

                // Odd / Even balance
                const bool isOdd = ((k + 1) % 2 != 0);
                float balGain = 1.0f;
                if (oddEvenBal < 0.0f && !isOdd) {
                    balGain = 1.0f + oddEvenBal;
                } else if (oddEvenBal > 0.0f && isOdd) {
                    balGain = 1.0f - oddEvenBal;
                }

                // Spectral Tilt: dB slope per octave
                const float oct = std::log2(std::max(1.0f, harmNum));
                const float tiltGain = TbdAudio::FastMath::fastDbToGain(spectralTilt * oct);

                // Additive Filter (Modern Mode): Frequency-domain cutoff with envelope modulation
                float effCutoff = filterCutoffHz;
                if (std::abs(filterEnvDepth) > 0.01f) {
                    effCutoff = std::clamp(filterCutoffHz * std::pow(2.0f, filterEnvDepth * filterEnv * 4.0f),
                                           20.0f, 0.495f * sampleRate_);
                }

                float filterGain = 1.0f;
                if (state_.modernMode && fk > effCutoff) {
                    const float ratio = fk / std::max(20.0f, effCutoff);
                    filterGain = 1.0f / (1.0f + std::pow(ratio, 4.0f));
                }

                float baseAmp = state_.p.baseHarmonic[k] * envGain * balGain * tiltGain * filterGain;

                // GST1: Velocity brightness
                if (velBrightness > 0.01f) {
                    baseAmp *= Laws::computeVelocityBrightnessWeight(fk, state_.midiNote, state_.velocity);
                }

                // GST1: Modal release damping
                if (state_.phase == VoicePhase::Release && modalRelease > 0.01f) {
                    baseAmp *= Laws::computeModalReleaseMultiplier(fk, 64.0f);
                }

                state_.p.ampTgt[k] = baseAmp * ampEnv * state_.velocity;

                const float amp2 = state_.p.ampTgt[k] * state_.p.ampTgt[k];
                if (k < kMax1987Partials) e126 += amp2;
                eTot += amp2;
            }

            // ADP1: Energy-matched extension gain for k > 126
            if (state_.modernMode && eTot > e126) {
                const float gExt = Laws::computeExtensionGain(e126, eTot);
                for (int k = kMax1987Partials; k < activeLimit; ++k) {
                    state_.p.ampTgt[k] *= gExt;
                }
            }

            // 4. Update Kuramoto coupling & rotator vectors
            osc_.updateKuramoto(state_, swarmK);
            osc_.prepareSubblockVectors(state_, effF0, foldWallHz);

            // 5. Render 16 samples
            osc_.renderSubblock(state_, sRe, sIm);

            // 6. In-Synthesis Alpha Chorus & Stereo Projection
            osc_.projectStereo(sRe, sIm, subL, subR, chunkSize, chorusMode, waterbedDepth, 0.0f);

            // 7. Accumulate into output with anti-click stealing fade if needed
            for (int i = 0; i < chunkSize; ++i) {
                float gain = 1.0f;
                if (stealFadeRemaining_ > 0) {
                    // Cosine window fade-out
                    const float progress = static_cast<float>(stealFadeRemaining_) / static_cast<float>(stealFadeSamples_);
                    gain = 0.5f * (1.0f - std::cos(3.14159265f * progress));
                    stealFadeRemaining_--;
                    if (stealFadeRemaining_ <= 0) {
                        state_.phase = VoicePhase::Idle;
                    }
                }
                outL[sampleOffset + i] += subL[i] * gain;
                outR[sampleOffset + i] += subR[i] * gain;
            }

            samplesRemaining -= chunkSize;
            sampleOffset += chunkSize;
        }
    }

private:
    float sampleRate_{48000.0f};
    int stealFadeSamples_{0};
    int stealFadeRemaining_{0};

    ShoggothVoiceState state_;
    Oscillator osc_;
    Envelopes envs_;
};

} // namespace Shoggoth
