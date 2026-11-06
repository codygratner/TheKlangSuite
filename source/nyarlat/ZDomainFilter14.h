#pragma once

#include <cmath>
#include <array>
#include <algorithm>
#include <immintrin.h>
#include <juce_core/juce_core.h>
#include "../FastMath.h"

namespace Nyarlat {

// Portable lane-0 extraction (MSVC / GCC / Clang compliant; zero member access)
static inline float lane0(__m128 v) noexcept { return _mm_cvtss_f32(v); }

// High-precision SIMD 2-Wide Stereo Transposed Direct Form II Biquad
// Evaluates both Left and Right audio channels simultaneously in 3 vector FMA/SSE instructions.
struct alignas(16) BiquadTDF2Stereo {
    __m128 s1 = _mm_setzero_ps();
    __m128 s2 = _mm_setzero_ps();

    // Active smoothed coefficients [c, c, 0, 0]
    __m128 b0 = _mm_set1_ps(1.0f);
    __m128 b1 = _mm_setzero_ps();
    __m128 b2 = _mm_setzero_ps();
    __m128 a1 = _mm_setzero_ps();
    __m128 a2 = _mm_setzero_ps();

    // Target coefficients
    __m128 tb0 = _mm_set1_ps(1.0f);
    __m128 tb1 = _mm_setzero_ps();
    __m128 tb2 = _mm_setzero_ps();
    __m128 ta1 = _mm_setzero_ps();
    __m128 ta2 = _mm_setzero_ps();

    // Sub-block linear delta slopes
    __m128 db0 = _mm_setzero_ps();
    __m128 db1 = _mm_setzero_ps();
    __m128 db2 = _mm_setzero_ps();
    __m128 da1 = _mm_setzero_ps();
    __m128 da2 = _mm_setzero_ps();
    int rampSteps = 0;

    void reset() noexcept {
        s1 = _mm_setzero_ps();
        s2 = _mm_setzero_ps();
        rampSteps = 0;
    }

    bool setTargetWithRamp(float inB0, float inB1, float inB2, float inA1, float inA2, float invSteps) noexcept {
        const bool changed = (lane0(tb0) != inB0) || (lane0(tb1) != inB1) ||
                             (lane0(tb2) != inB2) || (lane0(ta1) != inA1) ||
                             (lane0(ta2) != inA2);
        tb0 = _mm_set1_ps(inB0);
        tb1 = _mm_set1_ps(inB1);
        tb2 = _mm_set1_ps(inB2);
        ta1 = _mm_set1_ps(inA1);
        ta2 = _mm_set1_ps(inA2);

        if (changed) {
            if (invSteps <= 0.0f) {
                snap();
                return true;
            }
            __m128 vInv = _mm_set1_ps(invSteps);
            db0 = _mm_mul_ps(_mm_sub_ps(tb0, b0), vInv);
            db1 = _mm_mul_ps(_mm_sub_ps(tb1, b1), vInv);
            db2 = _mm_mul_ps(_mm_sub_ps(tb2, b2), vInv);
            da1 = _mm_mul_ps(_mm_sub_ps(ta1, a1), vInv);
            da2 = _mm_mul_ps(_mm_sub_ps(ta2, a2), vInv);
            rampSteps = static_cast<int>(std::lround(1.0f / invSteps));
        }
        return changed;
    }

    void snap() noexcept {
        b0 = tb0; b1 = tb1; b2 = tb2;
        a1 = ta1; a2 = ta2;
        rampSteps = 0;
    }

    inline void stepRamp() noexcept {
        if (rampSteps > 0) {
            b0 = _mm_add_ps(b0, db0);
            b1 = _mm_add_ps(b1, db1);
            b2 = _mm_add_ps(b2, db2);
            a1 = _mm_add_ps(a1, da1);
            a2 = _mm_add_ps(a2, da2);
            if (--rampSteps == 0) {
                b0 = tb0; b1 = tb1; b2 = tb2;
                a1 = ta1; a2 = ta2;
            }
        }
    }

    inline __m128 process(__m128 in) noexcept {
        // y[n] = b0 * x[n] + s1
        __m128 out = _mm_add_ps(_mm_mul_ps(b0, in), s1);
        // s1 = b1 * x[n] - a1 * y[n] + s2
        s1 = _mm_add_ps(_mm_sub_ps(_mm_mul_ps(b1, in), _mm_mul_ps(a1, out)), s2);
        // s2 = b2 * x[n] - a2 * y[n]
        s2 = _mm_sub_ps(_mm_mul_ps(b2, in), _mm_mul_ps(a2, out));
        return out;
    }
};

// Branchless Padé soft-knee inter-stage limiter: clamps to [-2.0, +2.0]
static inline __m128 softLimit2Wide(__m128 v) noexcept {
    const __m128 vMin = _mm_set1_ps(-2.0f);
    const __m128 vMax = _mm_set1_ps(2.0f);
    const __m128 vInv12 = _mm_set1_ps(1.0f / 12.0f);
    __m128 c = _mm_max_ps(vMin, _mm_min_ps(vMax, v));
    __m128 c2 = _mm_mul_ps(c, c);
    __m128 c3 = _mm_mul_ps(c2, c);
    return _mm_sub_ps(c, _mm_mul_ps(c3, vInv12));
}

// Exact rational quadratic peak power solver for H(z) = (b0 + b1 z^-1 + b2 z^-2)/(1 + a1 z^-1 + a2 z^-2)
// Identifies all interior critical points in c = cos(omega) in (-1, 1) and asserts peak power across full spectrum.
static inline double sectionPeakPower(double b1, double b2, double a1, double a2) noexcept {
    const double n2 = 4.0 * b2;
    const double n1 = 2.0 * b1 * (1.0 + b2);
    const double n0 = (1.0 - b2) * (1.0 - b2) + b1 * b1;

    const double d2 = 4.0 * a2;
    const double d1 = 2.0 * a1 * (1.0 + a2);
    const double d0 = (1.0 - a2) * (1.0 - a2) + a1 * a1;

    auto R = [&](double c) noexcept {
        const double num = n2 * c * c + n1 * c + n0;
        const double den = std::max(d2 * c * c + d1 * c + d0, 1e-12);
        return num / den;
    };

    double peak = std::max(R(1.0), R(-1.0));
    const double qa = n2 * d1 - n1 * d2;
    const double qb = n2 * d0 - n0 * d2;
    const double qc = n1 * d0 - n0 * d1;

    auto tryRoot = [&](double c) noexcept {
        if (c > -1.0 && c < 1.0) peak = std::max(peak, R(c));
    };

    if (std::abs(qa) > 1e-12) {
        const double disc = qb * qb - qa * qc;
        if (disc >= 0.0) {
            const double s = std::sqrt(disc);
            tryRoot((-qb + s) / qa);
            tryRoot((-qb - s) / qa);
        }
    } else if (std::abs(qb) > 1e-12) {
        tryRoot(-qc / (2.0 * qb));
    }
    return peak;
}

// Legacy single-channel scalar biquad for compatibility
struct BiquadTDF2 {
    float s1 = 0.0f;
    float s2 = 0.0f;
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f;
    float tb0 = 1.0f, tb1 = 0.0f, tb2 = 0.0f;
    float ta1 = 0.0f, ta2 = 0.0f;

    void reset() noexcept { s1 = s2 = 0.0f; }
    bool setTarget(float inB0, float inB1, float inB2, float inA1, float inA2) noexcept {
        const bool changed = (tb0 != inB0) || (tb1 != inB1) || (tb2 != inB2) || (ta1 != inA1) || (ta2 != inA2);
        tb0 = inB0; tb1 = inB1; tb2 = inB2; ta1 = inA1; ta2 = inA2;
        return changed;
    }
    void snap() noexcept { b0 = tb0; b1 = tb1; b2 = tb2; a1 = ta1; a2 = ta2; }
    inline float process(float in) noexcept {
        float out = b0 * in + s1;
        s1 = b1 * in - a1 * out + s2;
        s2 = b2 * in - a2 * out;
        return out;
    }
};

// Represents one corner in the 8-vertex 3D parameter cube
struct ZCubeCorner {
    struct Section {
        float pole_rho;   // 0.0 .. 0.995 (strictly < 1.0)
        float pole_theta; // 0.0 .. pi (frequency in radians)
        float zero_rho;   // 0.0 .. 1.0
        float zero_theta; // 0.0 .. pi
    };
    std::array<Section, 7> sections;
    float gain_db = 0.0f; // Passband gain correction
};

// A complete 14-pole Z-Plane model (8 corners defining 3D parameter cube)
struct ZFilterModel {
    const char* name;
    std::array<ZCubeCorner, 8> corners;
    float modelNorm = 0.35f; // Headroom normalization factor
};

class ZDomainFilter14 {
public:
    static constexpr int kNumSections = 7;

    ZDomainFilter14() = default;

    void prepare(double sampleRate) noexcept {
        sampleRate_ = (sampleRate > 0.0) ? sampleRate : 44100.0;
        reset();
    }

    void reset() noexcept {
        for (auto& s : stages_) s.reset();
        lastModel_ = -1;
        lastX_ = -1.0f; lastY_ = -1.0f; lastZ_ = -1.0f;
    }

    // Evaluate 3D Trilinear interpolation across active model's 8 corners
    // X = Freq (0..1), Y = Morph (0..1), Z = Transform (0..1)
    void updateCube(int modelIdx, float xNorm, float yNorm, float zNorm, float invSteps = (1.0f / 64.0f)) noexcept {
        modelIdx = juce::jlimit(0, 19, modelIdx);
        if (modelIdx == lastModel_ && xNorm == lastX_ && yNorm == lastY_ && zNorm == lastZ_) return;
        lastModel_ = modelIdx; lastX_ = xNorm; lastY_ = yNorm; lastZ_ = zNorm;
        const auto& model = getSharedModels()[modelIdx];

        float cx = juce::jlimit(0.0f, 1.0f, xNorm);
        float cy = juce::jlimit(0.0f, 1.0f, yNorm);
        float cz = juce::jlimit(0.0f, 1.0f, zNorm);

        // Trilinear corner weights
        float w0 = (1.0f - cx) * (1.0f - cy) * (1.0f - cz);
        float w1 = cx * (1.0f - cy) * (1.0f - cz);
        float w2 = (1.0f - cx) * cy * (1.0f - cz);
        float w3 = cx * cy * (1.0f - cz);
        float w4 = (1.0f - cx) * (1.0f - cy) * cz;
        float w5 = cx * (1.0f - cy) * cz;
        float w6 = (1.0f - cx) * cy * cz;
        float w7 = cx * cy * cz;

        for (int s = 0; s < kNumSections; ++s) {
            float pr = 0.0f, pt = 0.0f, zr = 0.0f, zt = 0.0f;
            const auto& c = model.corners;

            pr = w0*c[0].sections[s].pole_rho + w1*c[1].sections[s].pole_rho +
                 w2*c[2].sections[s].pole_rho + w3*c[3].sections[s].pole_rho +
                 w4*c[4].sections[s].pole_rho + w5*c[5].sections[s].pole_rho +
                 w6*c[6].sections[s].pole_rho + w7*c[7].sections[s].pole_rho;

            pt = w0*c[0].sections[s].pole_theta + w1*c[1].sections[s].pole_theta +
                 w2*c[2].sections[s].pole_theta + w3*c[3].sections[s].pole_theta +
                 w4*c[4].sections[s].pole_theta + w5*c[5].sections[s].pole_theta +
                 w6*c[6].sections[s].pole_theta + w7*c[7].sections[s].pole_theta;

            zr = w0*c[0].sections[s].zero_rho + w1*c[1].sections[s].zero_rho +
                 w2*c[2].sections[s].zero_rho + w3*c[3].sections[s].zero_rho +
                 w4*c[4].sections[s].zero_rho + w5*c[5].sections[s].zero_rho +
                 w6*c[6].sections[s].zero_rho + w7*c[7].sections[s].zero_rho;

            zt = w0*c[0].sections[s].zero_theta + w1*c[1].sections[s].zero_theta +
                 w2*c[2].sections[s].zero_theta + w3*c[3].sections[s].zero_theta +
                 w4*c[4].sections[s].zero_theta + w5*c[5].sections[s].zero_theta +
                 w6*c[6].sections[s].zero_theta + w7*c[7].sections[s].zero_theta;

            // Safe pole radius clamping (strictly <= 0.970 for guaranteed 7-stage cascade stability)
            pr = juce::jlimit(0.0f, 0.970f, pr);
            zr = juce::jlimit(0.0f, 0.980f, zr);

            // Fast minimax cosine
            const float cp = TbdAudio::FastMath::fastCos(pt);
            const float czAngle = TbdAudio::FastMath::fastCos(zt);

            // Polar to biquad polynomial coefficients
            float a1 = -2.0f * pr * cp;
            float a2 = pr * pr;

            // Unnormalized numerator coefficients
            float rawB1 = -2.0f * zr * czAngle;
            float rawB2 = zr * zr;

            // Exact rational quadratic peak power normalizer (guarantees max |H(w)| <= 0 dB at all frequencies)
            const double peakPwr = sectionPeakPower(rawB1, rawB2, a1, a2);
            const float g = static_cast<float>(0.95 / std::sqrt(std::max(1.0, peakPwr)));

            float b0 = g;
            float b1 = rawB1 * g;
            float b2 = rawB2 * g;

            stages_[s].setTargetWithRamp(b0, b1, b2, a1, a2, invSteps);
        }

        activeModelNorm_ = model.modelNorm;
    }

    void sanitize() noexcept {
        for (int s = 0; s < kNumSections; ++s) {
            auto& st = stages_[s];
            alignas(16) float a[4], b[4];
            _mm_store_ps(a, st.s1);
            _mm_store_ps(b, st.s2);
            if (!std::isfinite(a[0] + a[1] + b[0] + b[1]) ||
                std::abs(a[0]) > 1e4f || std::abs(a[1]) > 1e4f ||
                std::abs(b[0]) > 1e4f || std::abs(b[1]) > 1e4f) {
                st.reset();
            }
        }
    }

    inline void processStereo(float& left, float& right) noexcept {
        // Pack Left and Right into 2-wide SIMD vector: [left, right, 0, 0]
        __m128 in = _mm_setr_ps(left, right, 0.0f, 0.0f);

        for (int s = 0; s < kNumSections; ++s) {
            stages_[s].stepRamp();
            in = stages_[s].process(in);
            in = softLimit2Wide(in);
        }

        alignas(16) float out[4];
        _mm_store_ps(out, in);

        left = out[0] * activeModelNorm_;
        right = out[1] * activeModelNorm_;
    }

private:
    static const std::array<ZFilterModel, 20>& getSharedModels() {
        static const auto sharedModels = []() {
            std::array<ZFilterModel, 20> mods;
            for (int m = 0; m < 20; ++m) {
                auto& mod = mods[m];
                mod.modelNorm = 0.35f;

                float baseF = 0.05f + static_cast<float>(m) * 0.04f;

                for (int corner = 0; corner < 8; ++corner) {
                    auto& c = mod.corners[corner];
                    float cornerX = (corner & 1) ? 1.0f : 0.0f;
                    float cornerY = (corner & 2) ? 1.0f : 0.0f;
                    float cornerZ = (corner & 4) ? 1.0f : 0.0f;

                    for (int s = 0; s < 7; ++s) {
                        float sFactor = 1.0f + static_cast<float>(s) * 0.45f;
                        float theta = (baseF * sFactor) * (1.0f + cornerX * 1.5f + cornerY * 0.8f);
                        theta = juce::jlimit(0.01f, 3.10f, theta);

                        float rho = 0.85f + cornerZ * 0.14f - static_cast<float>(s) * 0.015f;
                        rho = juce::jlimit(0.5f, 0.995f, rho);

                        c.sections[s].pole_rho = rho;
                        c.sections[s].pole_theta = theta;

                        c.sections[s].zero_rho = 0.90f - cornerZ * 0.2f;
                        c.sections[s].zero_theta = theta * (0.85f + cornerY * 0.3f);
                    }
                }
            }
            return mods;
        }();
        return sharedModels;
    }

    double sampleRate_ = 44100.0;
    float activeModelNorm_ = 0.35f;

    int lastModel_ = -1;
    float lastX_ = -1.0f, lastY_ = -1.0f, lastZ_ = -1.0f;

    std::array<BiquadTDF2Stereo, kNumSections> stages_;
};

} // namespace Nyarlat
