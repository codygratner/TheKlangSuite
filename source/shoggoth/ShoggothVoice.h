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
        state_.masterPhase = 0.0f;
        stealFadeSamples_ = 0;
        stealFadeRemaining_ = 0;
        glideCoeff_ = 0.0f;
        lfoPhase_ = 0.0f;

        state_.ddf.z1 = 0.0f;
        state_.ddf.z2 = 0.0f;
        state_.ddfR.z1 = 0.0f;
        state_.ddfR.z2 = 0.0f;

        for (int k = 0; k < kMaxPartials; ++k) {
            state_.p.re[k] = 0.0f;
            state_.p.im[k] = 0.0f;
            state_.p.amp[k] = 0.0f;
            state_.p.ampTgt[k] = 0.0f;
            state_.p.baseHarmonic[k] = 0.0f;
            state_.p.s1Harmonic[k] = 0.0f;
            state_.p.s2Harmonic[k] = 0.0f;
            state_.p.ratio[k] = static_cast<float>(k + 1);
            state_.p.dOmega[k] = 0.0f;
            state_.p.envAssign[k] = static_cast<uint8_t>(k % 4);
        }

        for (int i = 0; i < kGuarded; ++i) {
            state_.p.psi[i] = 0.0f;
            state_.p.kAmp[i] = 0.0f;
        }
    }

    void noteOn(int midiNote, float velocity, float f0Hz,
                float pitchPunchDepthSt, bool modernMode, int cpuTier,
                float glideTimeMs = 0.0f, float prevF0Hz = 0.0f) noexcept {
        state_.midiNote = static_cast<uint8_t>(midiNote);
        state_.velocity = std::clamp(velocity, 0.01f, 1.0f);
        state_.f0Tgt = f0Hz;
        if (glideTimeMs > 0.001f && prevF0Hz > 20.0f) {
            state_.f0Hz = prevF0Hz;
            glideCoeff_ = std::exp(-1.0f / (std::max(0.001f, glideTimeMs * 0.001f) * sampleRate_ / static_cast<float>(kSub)));
        } else {
            state_.f0Hz = f0Hz;
            glideCoeff_ = 0.0f;
        }
        state_.modernMode = modernMode;
        state_.phase = VoicePhase::Attack;
        state_.ageSamples = 0;
        stealFadeRemaining_ = 0;

        // Reset filter biquad delays
        state_.ddf.z1 = 0.0f;
        state_.ddf.z2 = 0.0f;
        state_.ddfR.z1 = 0.0f;
        state_.ddfR.z2 = 0.0f;

        // Pitch punch: exponential drop
        state_.pitchPunchOffset = pitchPunchDepthSt;
        // ~15ms decay rate
        state_.pitchPunchDecay = std::exp(-1.0f / (0.015f * sampleRate_));

        // Set partial pool ceiling based on cpuTier
        int maxPool = 1024;
        if (cpuTier == 1) maxPool = 2048;
        else if (cpuTier == 2) maxPool = kMaxPartials;

        if (!modernMode) {
            state_.activePartials = 126;
        } else {
            // Adaptive Nyquist limit: f_k < 0.495 * fs
            const int nyqPartials = static_cast<int>((0.495f * sampleRate_) / std::max(20.0f, f0Hz));
            state_.activePartials = static_cast<uint32_t>(std::clamp(nyqPartials, 1, maxPool));
        }
        state_.activeBlocks = (state_.activePartials + 7) / 8;
        state_.masterPhase = 0.0f;

        // Trigger DHG & AHDSR envelopes
        for (int e = 0; e < kNumDHGEnv; ++e) {
            envs_.trigger1987(state_.dhgRuntime[e], state_.dhgData[e]);
        }
        envs_.trigger1987(state_.ddaRuntime, state_.dhgData[0]);
        envs_.triggerAhdsr(state_.ampAhdsr);
        envs_.triggerAhdsr(state_.filterAhdsr);

        // Phase initialization: Schroeder turns for low crest factor
        for (int k = 0; k < kMaxPartials; ++k) {
            if (k < static_cast<int>(state_.activePartials)) {
                float phase = 0.0f;
                if (modernMode && k >= kMax1987Partials) {
                    const float turns = Laws::computeSchroederPhaseTurns(k, state_.activePartials, 1.0f);
                    phase = turns * Oscillator::kTwoPi;
                }
                state_.p.re[k] = 1e-4f * std::cos(phase);
                state_.p.im[k] = 1e-4f * std::sin(phase);
                state_.p.amp[k] = 1e-4f;
            } else {
                state_.p.re[k] = 0.0f;
                state_.p.im[k] = 0.0f;
                state_.p.amp[k] = 0.0f;
                state_.p.ampTgt[k] = 0.0f;
                state_.p.baseHarmonic[k] = 0.0f;
            }
        }
    }

    void setPitchTarget(float f0Hz, float glideTimeMs = 0.0f) noexcept {
        state_.f0Tgt = f0Hz;
        if (glideTimeMs > 0.001f) {
            glideCoeff_ = std::exp(-1.0f / (std::max(0.001f, glideTimeMs * 0.001f) * sampleRate_ / static_cast<float>(kSub)));
        } else {
            state_.f0Hz = f0Hz;
            glideCoeff_ = 0.0f;
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
                     float filterEnvDepth,
                     float postDdfCutoff = 20000.0f, float postDdfReso = 0.0f,
                     float chaosAmount = 0.0f, float airAbsorption = 0.0f,
                     float mpeSlide = 0.0f, float mpePressure = 0.0f) noexcept {
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

            // 1. Advance Envelopes (Universal DHG animation in Modern & Vintage)
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
                    dhgEnvOut[e] = envs_.advance1987(state_.dhgRuntime[e], state_.dhgData[e], chunkSize);
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

            // 2. Portamento Glide Slewing (Subblock)
            if (glideCoeff_ > 0.0f && std::abs(state_.f0Hz - state_.f0Tgt) > 0.01f) {
                state_.f0Hz = state_.f0Tgt + (state_.f0Hz - state_.f0Tgt) * glideCoeff_;
            }

            // 3. Evaluate Effective Fundamental f0 with Pitch Punch & Slop
            float effF0 = state_.f0Hz;
            if (std::abs(state_.pitchPunchOffset) > 0.01f) {
                effF0 *= std::pow(2.0f, state_.pitchPunchOffset / 12.0f);
                state_.pitchPunchOffset *= state_.pitchPunchDecay;
            }
            if (oscSlop > 0.001f) {
                effF0 *= (1.0f + (state_.p.psi[kGuard] * 0.0005f * oscSlop));
            }

            // 4. Filter Cutoff with Envelope Depth & MPE Slide Modulation
            float effCutoff = filterCutoffHz;
            if (std::abs(filterEnvDepth) > 0.001f) {
                effCutoff = std::clamp(filterCutoffHz * std::pow(2.0f, filterEnvDepth * filterEnv * 4.0f),
                                       20.0f, 0.495f * sampleRate_);
            }
            if (std::abs(mpeSlide) > 0.001f) {
                effCutoff = std::clamp(effCutoff * std::pow(2.0f, mpeSlide * 2.0f),
                                       20.0f, 0.495f * sampleRate_);
            }

            const float Q = 1.0f + 8.0f * std::clamp(filterReso, 0.0f, 1.0f);
            const float invCutoff = 1.0f / std::max(20.0f, effCutoff);

            // 5. Compute Target Amplitudes for Active Partials
            float e126 = 0.0f;
            float eTot = 0.0f;

            const int activeLimit = std::min(static_cast<int>(state_.activePartials), kMaxPartials);
            for (int k = 0; k < activeLimit; ++k) {
                const float harmNum = static_cast<float>(k + 1);

                // Inharmonic warp: f_k = f0 * k^alpha
                state_.p.ratio[k] = (std::abs(inharmonicWarp - 1.0f) > 0.001f)
                    ? std::pow(harmNum, inharmonicWarp) : harmNum;

                const float fk = effF0 * state_.p.ratio[k];

                // Dynamic S1/S2 Subblock Morphing
                const float s1 = state_.p.s1Harmonic[k];
                const float s2 = state_.p.s2Harmonic[k];
                const float morphed = std::sqrt((1.0f - sourceMorph) * (s1 * s1) + sourceMorph * (s2 * s2));
                state_.p.baseHarmonic[k] = morphed;

                // Universal DHG Envelope Modulation
                const uint8_t envIdx = state_.p.envAssign[k] & 0x03;
                float envGain = dhgEnvOut[envIdx];
                if (state_.modernMode && envGain < 0.001f) {
                    envGain = 1.0f;
                }

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

                // Resonant Additive Filter (Modern & Vintage modes)
                float filterGain = 1.0f;
                const float x = fk * invCutoff;
                const float x2 = x * x;
                const float denom = std::sqrt((1.0f - x2) * (1.0f - x2) + (x2 / (Q * Q)) + 1e-6f);
                filterGain = std::min(4.0f, 1.0f / (denom * (1.0f + 0.5f * x2)));

                float baseAmp = state_.p.baseHarmonic[k] * envGain * balGain * tiltGain * filterGain;

                // Air Absorption: frequency-squared damping
                if (airAbsorption > 0.001f) {
                    const float fRatio = fk / 12000.0f;
                    baseAmp *= std::exp(-airAbsorption * fRatio * fRatio);
                }

                // GST1: Velocity brightness
                if (velBrightness > 0.01f) {
                    baseAmp *= Laws::computeVelocityBrightnessWeight(fk, state_.midiNote, state_.velocity);
                }

                // GST1: Modal release damping
                if (state_.phase == VoicePhase::Release && modalRelease > 0.01f) {
                    baseAmp *= Laws::computeModalReleaseMultiplier(fk, 64.0f);
                }

                const float pressGain = (mpePressure > 0.001f) ? (1.0f + mpePressure * 0.5f) : 1.0f;
                state_.p.ampTgt[k] = baseAmp * ampEnv * state_.velocity * pressGain;

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

            // 6. Update Kuramoto coupling, Chaos wander & rotator vectors
            osc_.updateKuramoto(state_, swarmK, chaosAmount);
            osc_.prepareSubblockVectors(state_, effF0, foldWallHz);

            // 7. Render 16 samples
            osc_.renderSubblock(state_, sRe, sIm);

            // 8. In-Synthesis Alpha Chorus & Stereo Projection
            osc_.projectStereo(sRe, sIm, subL, subR, chunkSize, chorusMode, waterbedDepth, 0.0f);

            // 9. Post-DDF Biquad Filter (Controls 7 & 8)
            if (postDdfCutoff < 19500.0f || postDdfReso > 0.01f) {
                const float w0 = Oscillator::kTwoPi * std::clamp(postDdfCutoff, 20.0f, 0.49f * sampleRate_) / sampleRate_;
                const float cosW = std::cos(w0);
                const float sinW = std::sin(w0);
                const float qPost = 0.7071f + 7.0f * std::clamp(postDdfReso, 0.0f, 1.0f);
                const float alpha = sinW / (2.0f * qPost);
                const float b0 = (1.0f - cosW) * 0.5f;
                const float b1 = 1.0f - cosW;
                const float b2 = (1.0f - cosW) * 0.5f;
                const float a0 = 1.0f + alpha;
                const float a1 = -2.0f * cosW;
                const float a2 = 1.0f - alpha;

                const float invA0 = 1.0f / a0;
                const float nb0 = b0 * invA0;
                const float nb1 = b1 * invA0;
                const float nb2 = b2 * invA0;
                const float na1 = a1 * invA0;
                const float na2 = a2 * invA0;

                for (int i = 0; i < chunkSize; ++i) {
                    const float inL = subL[i];
                    const float yL = nb0 * inL + state_.ddf.z1;
                    state_.ddf.z1 = nb1 * inL - na1 * yL + state_.ddf.z2;
                    state_.ddf.z2 = nb2 * inL - na2 * yL;
                    subL[i] = yL;

                    const float inR = subR[i];
                    const float yR = nb0 * inR + state_.ddfR.z1;
                    state_.ddfR.z1 = nb1 * inR - na1 * yR + state_.ddfR.z2;
                    state_.ddfR.z2 = nb2 * inR - na2 * yR;
                    subR[i] = yR;
                }
            }

            // 10. Accumulate into output with calibrated voice scaling & anti-click stealing fade
            constexpr float kVoiceScale = 0.354813f; // Calibrated headroom scaling for additive sum (-9 dB, +3 dB authority)
            for (int i = 0; i < chunkSize; ++i) {
                float gain = kVoiceScale;
                if (stealFadeRemaining_ > 0) {
                    // Cosine window fade-out
                    const float progress = static_cast<float>(stealFadeRemaining_) / static_cast<float>(stealFadeSamples_);
                    gain *= 0.5f * (1.0f - std::cos(3.14159265f * progress));
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
    float glideCoeff_{0.0f};
    float lfoPhase_{0.0f};

    ShoggothVoiceState state_;
    Oscillator osc_;
    Envelopes envs_;
};

} // namespace Shoggoth
