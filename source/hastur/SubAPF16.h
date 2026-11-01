#pragma once

#include <array>
#include <cstdint>
#include <cmath>
#include <algorithm>

namespace CrackedChimes::DSP {

/**
 * @brief 16th-Order Tapped All-Pass Sub-Resonator & Liquid Burble Engine.
 * 
 * Features:
 * - 16 cascaded 1-pole all-pass filter sections tracking sub fundamental frequency.
 * - Dynamic octave tracking: fc_i = clamp(fSub * 2^chewOct * 20^(i/15), 20.0, 0.45*fs).
 * - Flat magnitude response |H(e^jw)| = 1.0 (0 dB) across all open-loop frequencies.
 * - Multi-tap feedback from Stage 4 (4-Pole), Stage 8 (8-Pole), or Stage 16 (16-Pole) with
 *   dual 5Hz DC blocking and Padé tanh soft saturation.
 * - Fast tanh burble modulation output for audio-rate VCF Cutoff modulation.
 * - Audio-thread safe: zero dynamic allocations, no locks, branchless cascade loop.
 */
class SubAPF16 {
public:
    static constexpr int kNumStages = 16;
    static constexpr double kFb = 0.5;

    struct Output {
        float audioOut;
        float burbleMod;
    };

    /**
     * @brief Calculate all-pass pole coefficient a_i for stage i tracking fSub and chewOct.
     */
    static inline double poleCoeff(int i, double fSubHz, double chewOct, double sr) noexcept {
        const double fSubClamped = (fSubHz > 5.0) ? fSubHz : 55.0;
        const double ratio = std::pow(20.0, static_cast<double>(i) / 15.0);
        const double fc = std::clamp(fSubClamped * std::exp2(chewOct) * ratio, 20.0, sr * 0.45);
        const double wd = 3.14159265358979323846 * fc / sr;
        const double tanWd = std::tan(std::min(wd, 1.52));
        return (tanWd - 1.0) / (tanWd + 1.0);
    }

    void prepare(double sampleRate) noexcept {
        sampleRate_ = (sampleRate > 1000.0) ? sampleRate : 44100.0;
        invSr_ = 1.0 / sampleRate_;

        // Initialize targetA_ with default 55 Hz sub, chewOct = 0.0
        for (int i = 0; i < kNumStages; ++i) {
            targetA_[i] = poleCoeff(i, 55.0, 0.0, sampleRate_);
        }

        // DC Blocker pole for ~5 Hz cutoff: R = 1.0 - (2 * pi * 5 / fs)
        dcR_ = std::clamp(1.0 - (2.0 * 3.14159265358979323846 * 5.0 * invSr_), 0.990, 0.9999);

        reset();
    }

    void setSpit(float spit) noexcept {
        spit_ = std::clamp(spit, -1.0f, 1.0f);
    }
    [[nodiscard]] float getSpit() const noexcept { return spit_; }

    void reset() noexcept {
        s_.fill(0.0);
        fb_ = 0.0;
        spitSmoothed_ = spit_;
        dcPreX_ = 0.0;  dcPreY_ = 0.0;
        dcPostX_ = 0.0; dcPostY_ = 0.0;
        dcModX_ = 0.0;  dcModY_ = 0.0;
        updateCtr_ = 0;
    }

    /**
     * @brief Dynamic pitch tracking update rate-limited to once every 16 samples.
     */
    inline void setTracking(double fSubHz, double chewOct) noexcept {
        if ((updateCtr_++ & 15) == 0) {
            for (int i = 0; i < kNumStages; ++i) {
                targetA_[i] = poleCoeff(i, fSubHz, chewOct, sampleRate_);
            }
        }
    }

    /**
     * @brief Process a single sample through the 16th-order tapped APF.
     * @param x Input audio sample (dry sub-oscillator pulse train).
     * @param tapChoice Tap selector: 0 = Bypass/Wire, 1 = 4-Pole, 2 = 8-Pole, 3 = 16-Pole.
     * @return Output containing audioOut and burbleMod.
     */
    [[nodiscard]] inline Output process(float x, int tapChoice) noexcept {
        const bool wire = (tapChoice <= 0); // Audio wire bypass while burble cascade keeps running warm
        const double w4  = (tapChoice == 1) ? 1.0 : 0.0;
        const double w8  = (tapChoice == 2) ? 1.0 : 0.0;
        const double w16 = wire ? 1.0 : ((tapChoice == 3) ? 1.0 : 0.0);

        double v = static_cast<double>(x) + (wire ? 0.0 : fb_);
        double t4 = 0.0, t8 = 0.0;

        // 16-Stage 1-Multiplier All-Pass Cascade: y = a*v + s; s = v - a*y
        for (int i = 0; i < kNumStages; ++i) {
            const double a = targetA_[i];
            const double y = a * v + s_[i];
            s_[i] = v - a * y;
            v = y;

            if (i == 3) t4 = v;
            if (i == 7) t8 = v;
        }

        const double tap = w4 * t4 + w8 * t8 + w16 * v;

        // Pre-tanh DC Block (~5 Hz) to center asymmetrical pulse trains
        const double dcPreIn = tap;
        const double dcPreOut = dcPreIn - dcPreX_ + dcR_ * dcPreY_;
        dcPreX_ = dcPreIn;
        dcPreY_ = dcPreOut;

        // Padé / Soft Tanh Saturation in feedback loop
        const double sat = fastTanh(dcPreOut * 2.0);

        // Post-tanh DC Block to reject saturation DC bias
        const double dcPostIn = sat;
        const double dcPostOut = dcPostIn - dcPostX_ + dcR_ * dcPostY_;
        dcPostX_ = dcPostIn;
        dcPostY_ = dcPostOut;

        // Dynamic Spit Squelch feedback with 1-pole parameter slew capped at 0.45 * spit
        spitSmoothed_ += 0.005f * (spit_ - spitSmoothed_);
        fb_ = wire ? 0.0 : (static_cast<double>(spitSmoothed_ * 0.45f) * dcPostOut);

        // Burble tap follows the sub's tap setting: OFF = raw sub, 1/2/3 = 4P/8P/16P
        const double dcModIn = (tapChoice <= 0) ? static_cast<double>(x)
                             : (tapChoice == 1) ? t4
                             : (tapChoice == 2) ? t8
                             :                    v;
        const double dcModOut = dcModIn - dcModX_ + dcR_ * dcModY_;
        dcModX_ = dcModIn;
        dcModY_ = dcModOut;

        const float burbleMod = static_cast<float>(fastTanh(0.85 * dcModOut));

        const float audioOut = wire ? x : static_cast<float>(tap);
        return { audioOut, burbleMod };
    }

    /**
     * @brief Backward-compatible overload ignoring deprecated depth argument.
     */
    [[nodiscard]] inline Output process(float x, float /*deprecatedDepth*/, int tapChoice) noexcept {
        return process(x, tapChoice);
    }

private:
    [[nodiscard]] static inline double fastTanh(double x) noexcept {
        if (x < -3.0) return -1.0;
        if (x > 3.0)  return 1.0;
        const double x2 = x * x;
        return x * (27.0 + x2) / (27.0 + 9.0 * x2);
    }

    double sampleRate_ = 44100.0;
    double invSr_ = 1.0 / 44100.0;
    double dcR_ = 0.9993;

    float spit_ = 0.0f;
    float spitSmoothed_ = 0.0f;

    std::array<double, kNumStages> targetA_{};
    std::array<double, kNumStages> s_{};
    double fb_ = 0.0;

    double dcPreX_ = 0.0,  dcPreY_ = 0.0;
    double dcPostX_ = 0.0, dcPostY_ = 0.0;
    double dcModX_ = 0.0,  dcModY_ = 0.0;

    uint32_t updateCtr_ = 0;
};

} // namespace CrackedChimes::DSP
