#pragma once

#include <cmath>
#include <array>
#include <juce_core/juce_core.h>

namespace Nyarlat {

// A single 2nd-order section using Transposed Direct Form II (TDF-II)
// Superior numerical behavior for time-varying filters
struct BiquadTDF2 {
    float s1 = 0.0f;
    float s2 = 0.0f;

    // Active smoothed coefficients
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f;

    // Target coefficients for slew smoothing
    float tb0 = 1.0f, tb1 = 0.0f, tb2 = 0.0f;
    float ta1 = 0.0f, ta2 = 0.0f;

    void reset() noexcept {
        s1 = 0.0f;
        s2 = 0.0f;
    }

    void setTarget(float inB0, float inB1, float inB2, float inA1, float inA2) noexcept {
        tb0 = inB0; tb1 = inB1; tb2 = inB2;
        ta1 = inA1; ta2 = inA2;
    }

    void snap() noexcept {
        b0 = tb0; b1 = tb1; b2 = tb2;
        a1 = ta1; a2 = ta2;
    }

    void smooth(float lambda) noexcept {
        b0 += lambda * (tb0 - b0);
        b1 += lambda * (tb1 - b1);
        b2 += lambda * (tb2 - b2);
        a1 += lambda * (ta1 - a1);
        a2 += lambda * (ta2 - a2);
    }

    inline float process(float in) noexcept {
        // y[n] = b0 * x[n] + s1
        float out = b0 * in + s1;
        // s1 = b1 * x[n] - a1 * y[n] + s2
        s1 = b1 * in - a1 * out + s2;
        // s2 = b2 * x[n] - a2 * y[n]
        s2 = b2 * in - a2 * out;
        return out;
    }
};

// Represents one corner in the 8-vertex 3D parameter cube
// 7 pole-zero biquad sections in polar form (rho, theta)
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
        for (auto& b : leftStages_) b.reset();
        for (auto& b : rightStages_) b.reset();
    }

    // Evaluate 3D Trilinear interpolation across active model's 8 corners
    // X = Freq (0..1), Y = Morph (0..1), Z = Transform (0..1)
    void updateCube(int modelIdx, float xNorm, float yNorm, float zNorm) noexcept {
        modelIdx = juce::jlimit(0, 19, modelIdx);
        const auto& model = getSharedModels()[modelIdx];

        float cx = juce::jlimit(0.0f, 1.0f, xNorm);
        float cy = juce::jlimit(0.0f, 1.0f, yNorm);
        float cz = juce::jlimit(0.0f, 1.0f, zNorm);

        // Trilinear corner weights
        // Corner 0: (0,0,0), 1: (1,0,0), 2: (0,1,0), 3: (1,1,0)
        // Corner 4: (0,0,1), 5: (1,0,1), 6: (0,1,1), 7: (1,1,1)
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

            // Clamp radius strictly inside unit circle for guaranteed stability
            pr = juce::jlimit(0.0f, 0.990f, pr);
            zr = juce::jlimit(0.0f, 1.0f, zr);

            // Convert polar to polynomial biquad coefficients
            // a1 = -2 * rho * cos(theta), a2 = rho^2
            // b0 = 1.0, b1 = -2 * z_rho * cos(z_theta), b2 = z_rho^2
            float a1 = -2.0f * pr * std::cos(pt);
            float a2 = pr * pr;

            // |H(1)| = N/D (cancellation-free). Attenuate-only: a DC null (N->0) never divides.
            const float sp = std::sin(0.5f * pt);
            const float sz = std::sin(0.5f * zt);
            const float D = (1.0f - pr) * (1.0f - pr) + 4.0f * pr * sp * sp; // >= 1e-4 (pr <= 0.990)
            const float N = (1.0f - zr) * (1.0f - zr) + 4.0f * zr * sz * sz;
            const float g = (N > D) ? (D / N) : 1.0f;

            float b0 = g;
            float b1 = -2.0f * zr * std::cos(zt) * g;
            float b2 = zr * zr * g;

            leftStages_[s].setTarget(b0, b1, b2, a1, a2);
            rightStages_[s].setTarget(b0, b1, b2, a1, a2);
        }

        activeModelNorm_ = model.modelNorm;
    }

    void sanitize() noexcept {
        for (int s = 0; s < kNumSections; ++s) {
            auto& bl = leftStages_[s];
            if (!std::isfinite(bl.s1 + bl.s2) || std::abs(bl.s1) > 1e4f || std::abs(bl.s2) > 1e4f) {
                bl.reset();
            }
            auto& br = rightStages_[s];
            if (!std::isfinite(br.s1 + br.s2) || std::abs(br.s1) > 1e4f || std::abs(br.s2) > 1e4f) {
                br.reset();
            }
        }
    }

    void processStereo(float& left, float& right) noexcept {
        constexpr float kLambda = 0.05f; // Fast 1-pole coefficient smoothing (~0.8ms)

        float l = left;
        float r = right;

        for (int s = 0; s < kNumSections; ++s) {
            leftStages_[s].smooth(kLambda);
            rightStages_[s].smooth(kLambda);
            l = leftStages_[s].process(l);
            r = rightStages_[s].process(r);
        }

        // Apply dynamic model headroom normalization
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

    std::array<BiquadTDF2, kNumSections> leftStages_;
    std::array<BiquadTDF2, kNumSections> rightStages_;
};

} // namespace Nyarlat
