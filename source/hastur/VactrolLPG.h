#pragma once

#include <cmath>
#include <algorithm>
#include <cstdint>

namespace CrackedChimes::DSP {

/**
 * @brief Antiderivative Anti-Aliasing (ADAA1) Numerical Utilities.
 * SSoT: Julian Parker / Stefan Bilbao continuous piecewise antiderivative evaluation.
 */
struct ADAA {
    // 1st-Order Antiderivative Anti-Aliasing for tanh(g * x)
    [[nodiscard]] static inline float tanh(float x, float& x1, float g = 1.0f) noexcept {
        const float diff = x - x1;
        float y = 0.0f;
        if (std::abs(diff) > 1e-5f) {
            auto F = [g](float v) noexcept {
                const float gv = std::abs(g * v);
                return (gv + std::log1p(std::exp(-2.0f * gv)) - 0.69314718056f) / g;
            };
            y = (F(x) - F(x1)) / diff;
        } else {
            y = std::tanh(g * 0.5f * (x + x1));
        }
        x1 = x;
        return y;
    }

    // Piecewise-quadratic ADAA1 for asymmetric clamp [minVal, maxVal]
    [[nodiscard]] static inline float clamp(float x, float& x1, float minVal, float maxVal) noexcept {
        const float diff = x - x1;
        auto F = [minVal, maxVal](float v) noexcept {
            if (v < minVal) return minVal * v - 0.5f * minVal * minVal;
            if (v > maxVal) return maxVal * v - 0.5f * maxVal * maxVal;
            return 0.5f * v * v;
        };
        float y = 0.0f;
        if (std::abs(diff) > 1e-5f) {
            y = (F(x) - F(x1)) / diff;
        } else {
            y = std::clamp(0.5f * (x + x1), minVal, maxVal);
        }
        x1 = x;
        return y;
    }
};

/**
 * @brief The 3-Tier Life Mode Hierarchy (One Spec Sheet, Three High-End Teams).
 */
enum class LifeMode : uint8_t {
    SYNTHETIC = 0, // Clean & Precise (Modern High-End Digital Lab)
    CYBORG    = 1, // Gross Abomination (Modern Industrial Hybrid / Tetsuo: The Iron Man)
    ORGANIC   = 2  // Sexy & Droppy (Modern Boutique Analog Artisans / Liquid Vactrol LPG)
};

/**
 * @brief Virtual Vactrol CV Slew Cell (0V..5V knob conditioning with physical LED attack and CdS decay).
 */
class VactrolCV {
public:
    void prepare(double sampleRate) noexcept {
        const double sr = (sampleRate > 1000.0) ? sampleRate : 44100.0;
        const double invSr = 1.0 / sr;

        // Sub-millisecond LED attack (0.15ms)
        alphaAttack_ = static_cast<float>(1.0 - std::exp(-invSr / 0.00015));

        // Dual-exponential photoresistive release
        decayFastCoeff_ = static_cast<float>(std::exp(-invSr / 0.028)); // 28ms
        decaySlowCoeff_ = static_cast<float>(std::exp(-invSr / 0.380)); // 380ms
        state_ = 0.0f;
    }

    void reset(float initialVal = 0.0f) noexcept {
        state_ = initialVal;
    }

    [[nodiscard]] float process(float targetVal, bool organicMode) noexcept {
        if (!organicMode) {
            state_ = targetVal;
            return targetVal;
        }

        if (targetVal > state_) {
            // Fast LED rise
            state_ += alphaAttack_ * (targetVal - state_);
        } else {
            // Non-linear dual-exponential relaxation
            const float delta = state_ - targetVal;
            state_ = targetVal + delta * (0.72f * decayFastCoeff_ + 0.28f * decaySlowCoeff_);
        }
        return state_;
    }

    [[nodiscard]] float getState() const noexcept { return state_; }

private:
    float alphaAttack_{0.1f};
    float decayFastCoeff_{0.999f};
    float decaySlowCoeff_{0.9999f};
    float state_{0.0f};
};

/**
 * @brief Vactrol Low Pass Gate & Lifeform VCA Engine (VTL5C3/VTL5C4 optocoupler model + Tetsuo Hybrid).
 * 
 * Paradigms:
 * - SYNTHETIC (Clean & Precise): Precision Transconductance (OTA) VCA with 4th-order algebraic soft headroom.
 * - CYBORG (Gross Abomination - Tetsuo): Overdriven Discrete OTA -> Leaky Passive Vactrol LPG (-38.4 dB bleed floor).
 * - ORGANIC (Sexy & Droppy): Active Tube-Based Resonant Low-Pass Gate (Q = 1.25, 12AX7 triode saturation, 4 Hz DC decoupling).
 */
class VactrolLPG {
public:
    enum class BackendMode : uint8_t {
        VCA     = 0, // Clean Precision OTA VCA with algebraic headroom (SYNTHETIC)
        LPG     = 1, // Resonant Active Tube Low Pass Gate (ORGANIC)
        CYBORG  = 2  // Overdriven MS-10 OTA + Leaky Passive Vactrol LPG (CYBORG)
    };

    void prepare(double sampleRate) noexcept {
        sampleRate_ = (sampleRate > 1000.0) ? sampleRate : 44100.0;
        invSampleRate_ = 1.0 / sampleRate_;
        updateCoeffs();
        reset();
    }

    void setSampleRate(double sampleRate) noexcept {
        sampleRate_ = (sampleRate > 1000.0) ? sampleRate : 44100.0;
        invSampleRate_ = 1.0 / sampleRate_;
        updateCoeffs();
    }

    void updateCoeffs() noexcept {
        // Sub-millisecond LED attack (0.15ms)
        const double tauAttack = 0.00015;
        alphaAttack_ = static_cast<float>(1.0 - std::exp(-invSampleRate_ / tauAttack));

        // Rate-invariant dual-exponential release: c(2x) = sqrt(c_ref(1x))
        const double hostInvSr = (sampleRate_ > 60000.0) ? (invSampleRate_ * 2.0) : invSampleRate_;
        const double cRef = 0.72 * std::exp(-hostInvSr / 0.028) + 0.28 * std::exp(-hostInvSr / 0.380);
        decayCoeff_ = (sampleRate_ > 60000.0) ? static_cast<float>(std::sqrt(cRef)) : static_cast<float>(cRef);

        decayFastCoeff_ = static_cast<float>(std::exp(-invSampleRate_ / 0.028));
        decaySlowCoeff_ = static_cast<float>(std::exp(-invSampleRate_ / 0.380));

        // Cyborg snappy optical slew (2ms attack / dual-rate 28ms fast + 120ms tail release)
        cyborgAlphaAttack_ = static_cast<float>(1.0 - std::exp(-invSampleRate_ / 0.0020));
        cyborgDecayFast_   = static_cast<float>(std::exp(-invSampleRate_ / 0.028));
        cyborgDecayTail_   = static_cast<float>(std::exp(-invSampleRate_ / 0.120));

        // Exact exponential DC blocker alphas: 4 Hz (Organic) and 5 Hz (Cyborg)
        dcBlockAlphaOrganic_ = static_cast<float>(1.0 - std::exp(-2.0 * 3.14159265358979323846 * 4.0 * invSampleRate_));
        dcBlockAlphaCyborg_  = static_cast<float>(1.0 - std::exp(-2.0 * 3.14159265358979323846 * 5.0 * invSampleRate_));
    }

    void reset() noexcept {
        vactrolState_ = 0.0f;
        svfS1_ = 0.0f;
        svfS2_ = 0.0f;
        cyborgVactrolState_ = 0.0f;
        cyborgFilterState_ = 0.0f;
        dcBlockerState_ = 0.0f;
        adaaOtaW1_ = 0.0f;
    }

    void setMode(BackendMode mode) noexcept {
        mode_ = mode;
    }

    void setLifeMode(LifeMode lifeMode) noexcept {
        switch (lifeMode) {
            case LifeMode::SYNTHETIC: mode_ = BackendMode::VCA; break;
            case LifeMode::CYBORG:    mode_ = BackendMode::CYBORG; break;
            case LifeMode::ORGANIC:
            default:                  mode_ = BackendMode::LPG; break;
        }
    }

    /**
     * @brief Process a single sample through the active Lifeform VCA/LPG stage.
     * @param audioIn Input audio sample.
     * @param controlLevel Control signal (envelope, gate, or fader) in [0.0, 1.0].
     * @return Output sample shaped by the active engineering realization.
     */
    [[nodiscard]] float process(float audioIn, float controlLevel) noexcept {
        const float target = std::clamp(controlLevel, 0.0f, 1.0f);

        // =====================================================================
        // 1. SYNTHETIC: Clean Precision Transconductance (OTA) VCA
        // Linear control response + 4th-order algebraic soft headroom saturation
        // =====================================================================
        if (mode_ == BackendMode::VCA) {
            const float vLin = audioIn * target;
            const float v4 = vLin * vLin * vLin * vLin;
            return vLin / std::sqrt(std::sqrt(1.0f + 0.3164f * v4));
        }

        // =====================================================================
        // 2. CYBORG: Overdriven Discrete OTA -> Leaky Passive Vactrol LPG
        // - Defective dark optical bleed floor: 0.012 (-38.4 dB)
        // - Sluggish fatigued phosphor optical slew (55ms / 920ms)
        // - MS-10 OTA differential pair saturation: tanh(1.75*x + 0.22*x^2)
        // - Dynamic optical curtain roll-off with high-frequency drive tear
        // - Integrated 5 Hz leaky DC blocker
        // =====================================================================
        if (mode_ == BackendMode::CYBORG) {
            // A. Snappy optical slew with dual-rate release
            if (target > cyborgVactrolState_) {
                cyborgVactrolState_ += cyborgAlphaAttack_ * (target - cyborgVactrolState_);
            } else {
                const float decayCoeff = (cyborgVactrolState_ > 0.25f) ? cyborgDecayFast_ : cyborgDecayTail_;
                cyborgVactrolState_ += (1.0f - decayCoeff) * (target - cyborgVactrolState_);
            }

            // B. Effective control with activity-gated optical bleed floor
            constexpr float kBleedFloor = 0.012f;
            const float bleed = kBleedFloor * std::min(1.0f, cyborgVactrolState_ * 50.0f);
            const float cEff = std::clamp(cyborgVactrolState_ + bleed, 0.0f, 1.0f);

            // C. MS-10 OTA Differential Saturation with ADAA1
            const float xIn = audioIn * cEff;
            const float wOta = 1.75f * xIn + 0.22f * xIn * std::abs(xIn);
            const float vOta = ADAA::tanh(wOta, adaaOtaW1_, 1.0f);

            // D. Dynamic Optical Curtain (1-pole passive roll-off)
            const float fc = 40.0f + (cEff * cEff) * (0.40f * static_cast<float>(sampleRate_) - 40.0f);
            const float wc = 2.0f * 3.14159265358979323846f * fc * static_cast<float>(invSampleRate_);
            const float alphaCurtain = std::clamp(wc / (1.0f + wc), 0.001f, 0.999f);
            cyborgFilterState_ += alphaCurtain * (vOta - cyborgFilterState_);

            // High-frequency edge tear with C1-continuous smoothstep centered on 0.75
            const float absIn = std::abs(audioIn);
            float tearWeight = 0.0f;
            if (absIn > 0.70f) {
                const float u = std::min(1.0f, (absIn - 0.70f) * 10.0f);
                tearWeight = u * u * (3.0f - 2.0f * u);
            }
            const float edgeTear = (audioIn - cyborgFilterState_) * (0.08f * cEff * tearWeight);
            const float out = cyborgFilterState_ + edgeTear;

            // E. Integrated 5 Hz Leaky DC Blocker
            const float hpOut = out - dcBlockerState_;
            dcBlockerState_ += dcBlockAlphaCyborg_ * hpOut;
            return hpOut;
        }

        // =====================================================================
        // 3. ORGANIC: Active Tube-Based Resonant Low-Pass Gate
        // - VTL5C3 dual-exponential optical slew (0.15ms attack / 28ms & 380ms release)
        // - Active 2-pole TPT SVF with dynamic resonant bump (Q = 1.25, +2.5 dB)
        // - Asymmetric 12AX7 triode tube saturation: v + 0.18*v^2 - 0.05*v^3
        // - Integrated 4 Hz DC decoupling
        // =====================================================================
        // A. LED Response & CdS Photoresistive Decay (Rate-Invariant)
        if (target > vactrolState_) {
            vactrolState_ += alphaAttack_ * (target - vactrolState_);
        } else {
            const float diff = vactrolState_ - target;
            vactrolState_ = target + diff * decayCoeff_;
        }

        const float s = std::clamp(vactrolState_, 0.0f, 1.0f);
        const float conductance = s * s; // Cadmium-selenide square-law response

        // B. Dynamic 2-pole TPT SVF with Q = 1.25 (R = 0.40, 2R = 0.80)
        const float minCutoff = 18.0f;
        const float maxCutoff = static_cast<float>(sampleRate_ * 0.42);
        const float cutoffHz = minCutoff + conductance * (maxCutoff - minCutoff);

        const float g = std::clamp(
            std::tan(3.14159265358979323846f * cutoffHz * static_cast<float>(invSampleRate_)),
            0.0001f, 100.0f);
        constexpr float k2R = 0.80f; // Q = 1.25 => 2R = 1/1.25 = 0.80

        const float hp = (audioIn - (k2R + g) * svfS1_ - svfS2_) / (1.0f + k2R * g + g * g);
        const float bp = g * hp + svfS1_;
        const float lp = g * bp + svfS2_;
        svfS1_ = g * hp + bp;
        svfS2_ = g * bp + lp;

        // C. Asymmetric 12AX7 Triode Tube Saturation on Resonant Output
        const float vTube = lp * 1.25f;
        const float yTube = std::tanh((vTube + 0.28f * vTube * vTube) / (1.0f + 0.28f * std::abs(vTube)));

        // D. Optical Conductance Gating with Organic Makeup & Soft Ceiling
        const float gated = yTube * std::clamp(s, 0.0f, 1.0f) * 1.15f;
        auto softCeil = [](float x) noexcept {
            const float ax = std::abs(x);
            if (ax <= 0.90f) return x;
            const float excess = ax - 0.90f;
            const float compressed = 0.90f + 0.10f * std::tanh(excess * 10.0f);
            return std::copysign(compressed, x);
        };
        const float out = softCeil(gated);

        // E. Integrated 4 Hz DC Decoupling
        const float hpOut = out - dcBlockerState_;
        dcBlockerState_ += dcBlockAlphaOrganic_ * hpOut;
        return hpOut;
    }

    [[nodiscard]] bool isRinging() const noexcept {
        if (mode_ == BackendMode::LPG) return vactrolState_ > 3.2e-6f;
        if (mode_ == BackendMode::CYBORG) return cyborgVactrolState_ > 3.2e-6f;
        return false;
    }

private:
    BackendMode mode_{BackendMode::VCA};
    double sampleRate_{44100.0};
    double invSampleRate_{1.0 / 44100.0};

    // Organic (VTL5C3) coefficients
    float alphaAttack_{0.1f};
    float decayFastCoeff_{0.999f};
    float decaySlowCoeff_{0.9999f};
    float decayCoeff_{0.999f};
    float dcBlockAlphaOrganic_{0.00057f};

    // Organic state
    float vactrolState_{0.0f};
    float svfS1_{0.0f};
    float svfS2_{0.0f};

    // Cyborg coefficients & state
    float cyborgAlphaAttack_{0.01f};
    float cyborgDecayFast_{0.999f};
    float cyborgDecayTail_{0.999f};
    float dcBlockAlphaCyborg_{0.00071f};
    float cyborgVactrolState_{0.0f};
    float cyborgFilterState_{0.0f};
    float adaaOtaW1_{0.0f};

    // DC blocker state
    float dcBlockerState_{0.0f};
};

/**
 * @brief Universal Control Slew Engine across 3 Lifeforms.
 * - Synthetic: 4ms symmetric linear slew.
 * - Organic: Dual-exponential optical vactrol CdS lag on negative movement.
 * - Cyborg: 6-bit DAC ladder quantization (64 levels) with 1-pole smoothing.
 */
class UniversalControlSlew {
public:
    void prepare(double sampleRate) noexcept {
        sr_ = (sampleRate > 1000.0) ? sampleRate : 44100.0;
        vactrol_.prepare(sr_);
        alphaSynthetic_ = static_cast<float>(1.0 - std::exp(-1.0 / (sr_ * 0.004))); // 4ms
        state_ = 0.0f;
    }

    void reset(float initial = 0.0f) noexcept {
        vactrol_.reset(initial);
        state_ = initial;
    }

    [[nodiscard]] float process(float target, LifeMode mode) noexcept {
        switch (mode) {
            case LifeMode::SYNTHETIC:
                state_ += alphaSynthetic_ * (target - state_);
                return state_;

            case LifeMode::ORGANIC:
                return vactrol_.process(target, true);

            case LifeMode::CYBORG: {
                // Authentic 6-bit DAC ladder quantization (64 steps) with 1-pole smoothing
                const float stepped = std::round(target * 64.0f) / 64.0f;
                state_ += 0.08f * (stepped - state_);
                return std::clamp(state_, 0.0f, 1.0f);
            }
        }
        return target;
    }

private:
    double sr_{44100.0};
    VactrolCV vactrol_;
    float alphaSynthetic_{0.05f};
    float state_{0.0f};
};

} // namespace CrackedChimes::DSP
