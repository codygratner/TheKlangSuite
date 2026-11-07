#pragma once

#include <cmath>
#include <algorithm>
#include <array>
#include "shoggoth_layout.h"
#include "ShoggothLaws.h"
#include "FastMath.h"

namespace Shoggoth {

class Oscillator {
public:
    static constexpr float kTwoPi = 6.28318530717958647692f;

    Oscillator() = default;

    void prepare(double sampleRate) noexcept {
        sampleRate_ = static_cast<float>(sampleRate > 1000.0 ? sampleRate : 48000.0);
        invSampleRate_ = 1.0f / sampleRate_;
    }

    // ==============================================================================
    // 1. Kuramoto Stencil Evaluation (Detune Phase psi_k)
    // ==============================================================================
    void updateKuramoto(ShoggothVoiceState& voice, float K) noexcept {
        if (std::abs(K) < 1e-5f) {
            for (int k = 0; k < kMaxPartials; ++k) {
                voice.p.dOmega[k] = 0.0f;
            }
            return;
        }

        const float maxDeltaOmega = (kTwoPi * 5.0f) * invSampleRate_; // Max +/-5 Hz drift
        const auto& psi = voice.p.psi;
        const auto& kAmp = voice.p.kAmp;

        // Active blocks evaluated strictly
        const int activeLimit = std::min(static_cast<int>(voice.activePartials), kMaxPartials);

        for (int k = 0; k < activeLimit; ++k) {
            const int idx = k + kGuard; // Offset into guarded array
            const float psi_k = psi[idx];

            // 5-tap stencil: j in {-2, -1, 1, 2}
            const float d1 = psi[idx - 1] - psi_k;
            const float d2 = psi[idx + 1] - psi_k;
            const float d3 = psi[idx - 2] - psi_k;
            const float d4 = psi[idx + 2] - psi_k;

            const float w1 = kAmp[idx - 1] * 1.0f;
            const float w2 = kAmp[idx + 1] * 1.0f;
            const float w3 = kAmp[idx - 2] * 0.5f;
            const float w4 = kAmp[idx + 2] * 0.5f;

            const float sumSin = w1 * std::sin(d1) + w2 * std::sin(d2) + w3 * std::sin(d3) + w4 * std::sin(d4);
            const float sumW = w1 + w2 + w3 + w4 + 1e-12f;

            float deltaOmega = (K / sumW) * sumSin;
            voice.p.dOmega[k] = std::clamp(deltaOmega, -maxDeltaOmega, maxDeltaOmega);
        }
    }

    // ==============================================================================
    // 2. Prepare Rotator Step Vectors (16-sample subblock)
    // ==============================================================================
    void prepareSubblockVectors(ShoggothVoiceState& voice, float f0, float foldWallHz) noexcept {
        const int numBlocks = voice.activeBlocks;
        const int numPartials = std::min(numBlocks * kLanes, kMaxPartials);
        const float baseRad = kTwoPi * invSampleRate_;

        for (int k = 0; k < numPartials; ++k) {
            // Fold frequency at event horizon
            const float rawFk = f0 * voice.p.ratio[k];
            const float fk = Laws::foldFrequency(rawFk, foldWallHz);
            const float omega_k = fk * baseRad + voice.p.dOmega[k];

            const float a0 = voice.p.amp[k];
            const float a1 = voice.p.ampTgt[k];

            // Defect F1 Fix: Fresh onset seeding & bounded exponential transition
            const float a0_clamped = std::max(a0, 1e-5f);
            const float a1_clamped = std::max(a1, 1e-5f);

            if (a0 < 2e-4f && a1 >= 1e-5f) {
                const float curMag = std::sqrt(voice.p.re[k] * voice.p.re[k] + voice.p.im[k] * voice.p.im[k]);
                if (curMag < 1e-6f) {
                    voice.p.re[k] = a1;
                    voice.p.im[k] = 0.0f;
                } else {
                    const float scale = a1 / curMag;
                    voice.p.re[k] *= scale;
                    voice.p.im[k] *= scale;
                }
                voice.p.amp[k] = a1;
            }

            const float ratio = a1_clamped / std::max(voice.p.amp[k], 1e-5f);
            const float clampedRatio = std::clamp(ratio, 0.001f, 1000.0f);
            const float g = std::pow(clampedRatio, 0.0625f);
            voice.p.dAmp[k] = 0.0f;

            // Folded step: w = g * exp(j * omega)
            voice.p.wRe[k] = g * std::cos(omega_k);
            voice.p.wIm[k] = g * std::sin(omega_k);

            // Update Kuramoto detune phase tracker: psi_k = theta_k - r_k * Theta_0
            voice.p.psi[k + kGuard] = std::atan2(voice.p.im[k], voice.p.re[k]) - voice.p.ratio[k] * voice.masterPhase;
            voice.p.kAmp[k + kGuard] = a1;
        }

        voice.masterPhase += 16.0f * (f0 * baseRad);
        while (voice.masterPhase >= kTwoPi) voice.masterPhase -= kTwoPi;
    }

    static inline float hsum256(__m256 v) noexcept {
        const __m128 lo = _mm256_castps256_ps128(v);
        const __m128 hi = _mm256_extractf128_ps(v, 1);
        const __m128 sum128 = _mm_add_ps(lo, hi);
        const __m128 shuf = _mm_movehl_ps(sum128, sum128);
        const __m128 sums = _mm_add_ps(sum128, shuf);
        const __m128 shuf2 = _mm_shuffle_ps(sums, sums, 1);
        return _mm_cvtss_f32(_mm_add_ss(sums, shuf2));
    }

    // ==============================================================================
    // 3. Render 16 Samples of Summed Partials (Quadrature Buses)
    // ==============================================================================
    void renderSubblock(ShoggothVoiceState& voice, float* outSRe, float* outSIm) noexcept {
        // Zero output buses
        for (int i = 0; i < kSub; ++i) {
            outSRe[i] = 0.0f;
            outSIm[i] = 0.0f;
        }

        const int numBlocks = voice.activeBlocks;

        for (int b = 0; b < numBlocks; ++b) {
            const int offset = b * kLanes;

            // Software Prefetch next block into L1 cache
            _mm_prefetch(reinterpret_cast<const char*>(&voice.p.re[offset + 16]), _MM_HINT_T0);
            _mm_prefetch(reinterpret_cast<const char*>(&voice.p.im[offset + 16]), _MM_HINT_T0);

            __m256 re = _mm256_load_ps(&voice.p.re[offset]);
            __m256 im = _mm256_load_ps(&voice.p.im[offset]);
            const __m256 wRe = _mm256_load_ps(&voice.p.wRe[offset]);
            const __m256 wIm = _mm256_load_ps(&voice.p.wIm[offset]);

            // Process 16 samples
            for (int n = 0; n < kSub; ++n) {
                // Complex rotation:
                // nextRe = re * wRe - im * wIm
                // nextIm = re * wIm + im * wRe
                const __m256 nextRe = _mm256_sub_ps(_mm256_mul_ps(re, wRe), _mm256_mul_ps(im, wIm));
                const __m256 nextIm = _mm256_add_ps(_mm256_mul_ps(re, wIm), _mm256_mul_ps(im, wRe));
                re = nextRe;
                im = nextIm;

                outSRe[n] += hsum256(re);
                outSIm[n] += hsum256(im);
            }

            // Store back state
            _mm256_store_ps(&voice.p.re[offset], re);
            _mm256_store_ps(&voice.p.im[offset], im);
        }

        // Protected Rotator Renormalization at subblock boundary
        for (int k = 0; k < numBlocks * kLanes; ++k) {
            const float targetAmp = voice.p.ampTgt[k];
            if (targetAmp < 1e-5f) {
                // Smooth exponential decay towards zero rather than discontinuous hard clipping
                voice.p.re[k] *= 0.5f;
                voice.p.im[k] *= 0.5f;
                if (std::abs(voice.p.re[k]) < 1e-7f) voice.p.re[k] = 0.0f;
                if (std::abs(voice.p.im[k]) < 1e-7f) voice.p.im[k] = 0.0f;
                voice.p.amp[k] = 0.0f;
            } else {
                const float currentMag2 = voice.p.re[k] * voice.p.re[k] + voice.p.im[k] * voice.p.im[k];
                const float targetMag2 = targetAmp * targetAmp;
                float factor = 1.0f;
                if (currentMag2 > 1e-12f) {
                    const float ratio2 = currentMag2 / targetMag2;
                    if (ratio2 >= 0.25f && ratio2 <= 2.25f) {
                        factor = 1.5f - 0.5f * ratio2;
                    } else {
                        factor = targetAmp / std::sqrt(currentMag2);
                    }
                } else {
                    voice.p.re[k] = targetAmp;
                    voice.p.im[k] = 0.0f;
                    factor = 1.0f;
                }
                voice.p.re[k] *= factor;
                voice.p.im[k] *= factor;
                voice.p.amp[k] = targetAmp;
            }
        }
    }

    // ==============================================================================
    // 4. In-Synthesis Alpha Chorus & Stereo Projection (0 Delay Buffers, 0 Latency)
    // ==============================================================================
    void projectStereo(const float* sRe, const float* sIm,
                       float* outL, float* outR,
                       int numSamples,
                       int chorusMode, float waterbedDepth, float pan) noexcept {
        // Roland Alpha Chorus Calibrated Rates
        float rateHz = 0.0f;
        float depth = 0.0f;
        if (chorusMode == 1) {      // Chorus I: 0.4 Hz slow swirl
            rateHz = 0.4f; depth = 0.5f;
        } else if (chorusMode == 2) { // Chorus II: 0.6 Hz deep shimmer
            rateHz = 0.6f; depth = 0.8f;
        } else if (chorusMode == 3) { // Chorus I+II: 8.0 Hz fast rotary
            rateHz = 8.0f; depth = 1.0f;
        }

        const float lfoStep = (kTwoPi * rateHz) * invSampleRate_;
        const float wSpread = waterbedDepth * 0.35f;

        // Equal-power stereo panning
        const float normPan = std::clamp(pan, -1.0f, 1.0f) * 0.5f + 0.5f;
        const float panL = std::cos(1.57079632679f * normPan);
        const float panR = std::sin(1.57079632679f * normPan);

        for (int n = 0; n < numSamples; ++n) {
            chorusPhase_ += lfoStep;
            if (chorusPhase_ >= kTwoPi) chorusPhase_ -= kTwoPi;

            const float lfoL = std::sin(chorusPhase_);
            const float lfoR = std::cos(chorusPhase_);

            // Quadrature phase modulation
            const float phaseL = depth * lfoL - wSpread;
            const float phaseR = depth * lfoR + wSpread;

            const float cL_re = std::cos(phaseL);
            const float cL_im = std::sin(phaseL);
            const float cR_re = std::cos(phaseR);
            const float cR_im = std::sin(phaseR);

            // In-Synthesis Projection:
            // L = Re(c_L) * S_re - Im(c_L) * S_im
            // R = Re(c_R) * S_re - Im(c_R) * S_im
            outL[n] = panL * (cL_re * sRe[n] - cL_im * sIm[n]);
            outR[n] = panR * (cR_re * sRe[n] - cR_im * sIm[n]);
        }
    }

private:
    float sampleRate_{48000.0f};
    float invSampleRate_{1.0f / 48000.0f};
    float chorusPhase_{0.0f};
};

} // namespace Shoggoth
