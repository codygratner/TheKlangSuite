#pragma once

#include <cmath>
#include <array>
#include <algorithm>
#include <juce_core/juce_core.h>
#include "../FastMath.h"

namespace Nyarlat {

// High-precision Portable Stereo Transposed Direct Form II Biquad
// Evaluates both Left and Right audio channels with scalar math (auto-vectorized by modern compilers).
struct BiquadTDF2Stereo {
    float s1L = 0.0f, s1R = 0.0f;
    float s2L = 0.0f, s2R = 0.0f;

    // Active smoothed coefficients
    float b0 = 1.0f;
    float b1 = 0.0f;
    float b2 = 0.0f;
    float a1 = 0.0f;
    float a2 = 0.0f;

    // Target coefficients
    float tb0 = 1.0f;
    float tb1 = 0.0f;
    float tb2 = 0.0f;
    float ta1 = 0.0f;
    float ta2 = 0.0f;

    // Sub-block linear delta slopes
    float db0 = 0.0f;
    float db1 = 0.0f;
    float db2 = 0.0f;
    float da1 = 0.0f;
    float da2 = 0.0f;
    int rampSteps = 0;

    void reset() noexcept {
        s1L = s1R = 0.0f;
        s2L = s2R = 0.0f;
        rampSteps = 0;
    }

    bool setTargetWithRamp(float inB0, float inB1, float inB2, float inA1, float inA2, float invSteps) noexcept {
        const bool changed = (tb0 != inB0) || (tb1 != inB1) ||
                             (tb2 != inB2) || (ta1 != inA1) ||
                             (ta2 != inA2);
        tb0 = inB0;
        tb1 = inB1;
        tb2 = inB2;
        ta1 = inA1;
        ta2 = inA2;

        if (changed) {
            if (invSteps <= 0.0f) {
                snap();
                return true;
            }
            db0 = (tb0 - b0) * invSteps;
            db1 = (tb1 - b1) * invSteps;
            db2 = (tb2 - b2) * invSteps;
            da1 = (ta1 - a1) * invSteps;
            da2 = (ta2 - a2) * invSteps;
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
            b0 += db0;
            b1 += db1;
            b2 += db2;
            a1 += da1;
            a2 += da2;
            if (--rampSteps == 0) {
                b0 = tb0; b1 = tb1; b2 = tb2;
                a1 = ta1; a2 = ta2;
            }
        }
    }

    inline void process(float& l, float& r) noexcept {
        // Left channel: TDF-II
        float outL = b0 * l + s1L;
        s1L = b1 * l - a1 * outL + s2L;
        s2L = b2 * l - a2 * outL;
        l = outL;

        // Right channel: TDF-II
        float outR = b0 * r + s1R;
        s1R = b1 * r - a1 * outR + s2R;
        s2R = b2 * r - a2 * outR;
        r = outR;
    }
};

// Branchless Padé soft-knee inter-stage limiter: clamps to [-2.0, +2.0]
static inline float softLimit(float v) noexcept {
    const float c = std::max(-2.0f, std::min(2.0f, v));
    return c - (c * c * c) * (1.0f / 12.0f);
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
            if (!std::isfinite(st.s1L + st.s1R + st.s2L + st.s2R) ||
                std::abs(st.s1L) > 1e4f || std::abs(st.s1R) > 1e4f ||
                std::abs(st.s2L) > 1e4f || std::abs(st.s2R) > 1e4f) {
                st.reset();
            }
        }
    }

    inline void processStereo(float& left, float& right) noexcept {
        float l = left;
        float r = right;

        for (int s = 0; s < kNumSections; ++s) {
            stages_[s].stepRamp();
            stages_[s].process(l, r);
            l = softLimit(l);
            r = softLimit(r);
        }

        left = l * activeModelNorm_;
        right = r * activeModelNorm_;
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
