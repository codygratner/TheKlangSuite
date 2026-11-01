#pragma once

#include <cmath>
#include <algorithm>

namespace CrackedChimes::DSP {

/**
 * @brief 4th-Order All-Pass Filter (APF) with Cutoff, Tanh Saturation, and Noise Engineering Infinifold.
 * SSoT: Roland IR3R05 phase dispersion + Noise Engineering wavefolding + 6-permutation topology.
 */
class AllPassFilter4 {
public:
    void prepare(double sampleRate) noexcept {
        sampleRate_ = (sampleRate > 1000.0) ? sampleRate : 44100.0;
        invSr_ = 1.0 / sampleRate_;
        reset();
    }

    void reset() noexcept {
        for (int i = 0; i < 8; ++i) {
            x1_[i] = 0.0f;
            y1_[i] = 0.0f;
            x1Inv_[i] = 0.0f;
            y1Inv_[i] = 0.0f;
        }
        for (int i = 0; i < 2; ++i) {
            bqX1_[i] = 0.0f; bqX2_[i] = 0.0f;
            bqY1_[i] = 0.0f; bqY2_[i] = 0.0f;
        }
        dcBlockerState_ = 0.0f;
    }

    /**
     * @brief 4-Pole All-Pass Filter core phase dispersion.
     */
    /**
     * @brief 4-Pole All-Pass Filter with Resonant Formant Feedback Loop.
     * @param input Audio sample.
     * @param cutoffHz Cutoff/break frequency in Hz.
     * @param feedback Resonant feedback [-0.95, +0.95]. 0.0 = pure allpass phase dispersion;
     *                 0.55 = vocal formant squelch / rubbery analog chew.
     */
    /**
     * @brief 4-Pole All-Pass Filter with Lifeform-specific non-linear feedback and dispersion.
     * @param input Audio sample.
     * @param cutoffHz Cutoff/break frequency in Hz.
     * @param feedback Resonant feedback [-0.95, +0.95].
     * @param lifeMode 0 = Synthetic, 1 = Cyborg, 2 = Organic.
     */
    [[nodiscard]] float processAPFLifeform(float input, float cutoffHz, float feedback, int lifeMode) noexcept {
        const float nyquist = static_cast<float>(sampleRate_ * 0.48);
        const float fc = std::clamp(cutoffHz, 20.0f, nyquist);

        const float wd = static_cast<float>(3.14159265358979323846 * fc * invSr_);
        const float tanWd = std::tan(std::min(wd, 1.52f));
        const float a = (tanWd - 1.0f) / (tanWd + 1.0f);
        const float fb = std::clamp(feedback, -0.95f, 0.95f);

        float inSample = input;
        if (lifeMode == 2) { // ORGANIC: Triode asymmetric feedback
            const float fbSignal = fb * y1_[7];
            const float satFb = (fbSignal + 0.15f * fbSignal * fbSignal) / (1.0f + 0.15f * std::abs(fbSignal));
            inSample = input - satFb;
        } else {
            inSample = input - std::tanh(fb * y1_[7]);
        }

        float sig = inSample;
        for (int i = 0; i < 8; ++i) {
            float y = a * sig + x1_[i] - a * y1_[i];
            x1_[i] = sig;

            if (lifeMode == 1) { // CYBORG: Starved op-amp rail foldback
                if (y > 1.1f) y = 2.2f - y;
                else if (y < -1.1f) y = -2.2f - y;
            }

            y1_[i] = y;
            sig = y;
        }

        if (lifeMode == 2) { // ORGANIC: Sample-rate normalized 6.0Hz leaky DC blocker
            const float alphaDc = static_cast<float>(6.2831853 * 6.0 * invSr_);
            const float out = sig - dcBlockerState_;
            dcBlockerState_ += alphaDc * out;
            return out;
        }

        return std::clamp(sig, -2.0f, 2.0f);
    }

    [[nodiscard]] float processAPF(float input, float cutoffHz, float feedback = 0.55f) noexcept {
        const float nyquist = static_cast<float>(sampleRate_ * 0.48);
        const float fc = std::clamp(cutoffHz, 20.0f, nyquist);

        // 1-pole all-pass coefficient: a = (tan(pi*fc/fs) - 1) / (tan(pi*fc/fs) + 1)
        const float wd = static_cast<float>(3.14159265358979323846 * fc * invSr_);
        const float tanWd = std::tan(std::min(wd, 1.52f));
        const float a = (tanWd - 1.0f) / (tanWd + 1.0f);

        // Resonant feedback loop from 4th stage output with soft analog saturation
        const float fb = std::clamp(feedback, -0.95f, 0.95f);
        const float inWithFb = input - std::tanh(fb * y1_[7]);

        float sig = inWithFb;
        for (int i = 0; i < 8; ++i) {
            const float y = a * sig + x1_[i] - a * y1_[i];
            x1_[i] = sig;
            y1_[i] = y;
            sig = y;
        }
        return sig;
    }

    /**
     * @brief Non-linear Tanh saturation.
     * @param amt Saturation depth [0.0, 1.0].
     */
    [[nodiscard]] static inline float applyTanh(float sig, float amt) noexcept {
        if (amt <= 0.001f) return sig;
        const float gain = 1.0f + amt * 5.0f;
        return std::tanh(sig * gain);
    }

    /**
     * @brief Noise Engineering Infinifold continuous multi-stage wavefolding.
     * Progressively folds the waveform back onto itself as foldAmt increases:
     * - At foldAmt == 0.0f: 100% linear identity bypass.
     * - From 0.01 to 1.0: Progressive, immediate wavefolding yielding 1 to 4 folds (gain: 1.0 -> 9.0).
     * - Output remains naturally bounded within [-1.0, 1.0] without aggressive post-tanh crushing.
     * 
     * @param sig Audio sample nominally in [-1.0, 1.0].
     * @param foldAmt Folding intensity [0.0, 1.0].
     */
    [[nodiscard]] static inline float applyInfinifold(float sig, float foldAmt) noexcept {
        if (foldAmt <= 0.001f) return sig;

        // Progressive drive gain: 1.0 (0 folds) to 9.0 (4 complete folds)
        const float gain = 1.0f + foldAmt * 8.0f;
        float v = sig * gain;

        // Multi-stage continuous iterative wavefolding (up to 4 complete folds / 6 reflections)
        for (int i = 0; i < 6; ++i) {
            if (v > 1.0f) {
                v = 2.0f - v;
            } else if (v < -1.0f) {
                v = -2.0f - v;
            } else {
                break;
            }
        }

        return std::clamp(v, -1.0f, 1.0f);
    }

    /**
     * @brief Process sample through APF, Tanh, and Infinifold with selectable topology order.
     * Progressive immediate wavefolding across the full 0.0..1.0 range:
     * - At drive == 0.0: 100% linear identity bypass for drive stages.
     * - 0.01..1.0: Progressive Infinifold wavefolding (1 to 4 folds).
     * - Tanh stage provides gentle analog warmth without aggressive post-tanh fold crushing.
     * 
     * Topologies (Top 3 Sweet-Spot Permutations):
     * 0: APF -> Tanh -> Infinifold
     * 1: APF -> Infinifold -> Tanh
     * 2: Tanh -> APF -> Infinifold
     */
    [[nodiscard]] float processTopology(
        float input,
        float cutoffHz,
        float drive,
        int topology = 0,
        float feedback = 0.55f) noexcept
    {
        const float d = std::clamp(drive, 0.0f, 1.0f);
        if (d <= 0.001f) {
            return processAPF(input, cutoffHz, feedback);
        }

        const float infiniAmt = d;
        // In Topology 1 (where Tanh follows Infinifold), keep Tanh subtle to prevent crushing folds.
        // In Topologies 0 and 2 (where Tanh precedes Infinifold), gentle analog warmth feeds the folder.
        const float tanhAmt = (topology == 1) ? (d * 0.15f) : (d * 0.35f);

        auto doApf = [this, cutoffHz, feedback](float s) { return processAPF(s, cutoffHz, feedback); };
        auto doTanh = [tanhAmt](float s) { return applyTanh(s, tanhAmt); };
        auto doInfini = [infiniAmt](float s) { return applyInfinifold(s, infiniAmt); };

        float sig = input;
        switch (std::clamp(topology, 0, 2)) {
            case 0: // APF -> Tanh -> Infinifold
                sig = doApf(sig);
                sig = doTanh(sig);
                sig = doInfini(sig);
                break;
            case 1: // APF -> Infinifold -> Tanh
                sig = doApf(sig);
                sig = doInfini(sig);
                sig = doTanh(sig);
                break;
            case 2: // Tanh -> APF -> Infinifold
            default:
                sig = doTanh(sig);
                sig = doApf(sig);
                sig = doInfini(sig);
                break;
        }
        return sig;
    }

    /**
     * @brief Inverted 4-Pole APF where group delay is maximum at high frequencies.
     * High frequencies lag behind, smearing sharp chopper edges into dispersive laser chirps.
     */
    [[nodiscard]] float processInvertedAPF(float input, float cutoffHz) noexcept {
        const float nyquist = static_cast<float>(sampleRate_ * 0.48);
        const float fc = std::clamp(cutoffHz, 20.0f, nyquist);

        const float wd = static_cast<float>(3.14159265358979323846 * fc * invSr_);
        const float tanWd = std::tan(std::min(wd, 1.52f));
        const float a = (tanWd - 1.0f) / (tanWd + 1.0f);
        const float aInv = -a; // Mirror pole near Nyquist

        float sig = input;
        for (int i = 0; i < 8; ++i) {
            const float y = aInv * sig + x1Inv_[i] - aInv * y1Inv_[i];
            x1Inv_[i] = sig;
            y1Inv_[i] = y;
            sig = y;
        }
        return sig;
    }

    /**
     * @brief 2-Stage Cascaded 2nd-Order Biquad APF Chirp Disperser.
     * Group delay peaks sharply around fChirpHz (e.g. 4.5 kHz), producing an authentic metallic "pew".
     */
    [[nodiscard]] float processBiquadChirp(float input, float fChirpHz = 4500.0f, float Q = 3.2f) noexcept {
        const float nyq = static_cast<float>(sampleRate_ * 0.48);
        const float fc = std::clamp(fChirpHz, 100.0f, nyq);
        const float w0 = static_cast<float>(6.28318530717958647692 * fc * invSr_);
        const float alpha = std::sin(w0) / (2.0f * Q);

        const float b0 = 1.0f - alpha;
        const float b1 = -2.0f * std::cos(w0);
        const float b2 = 1.0f + alpha;
        const float a0 = 1.0f + alpha;
        const float a1 = -2.0f * std::cos(w0);
        const float a2 = 1.0f - alpha;

        const float invA0 = 1.0f / a0;
        const float cB0 = b0 * invA0, cB1 = b1 * invA0, cB2 = b2 * invA0;
        const float cA1 = a1 * invA0, cA2 = a2 * invA0;

        float sig = input;
        for (int i = 0; i < 2; ++i) {
            const float y = cB0 * sig + cB1 * bqX1_[i] + cB2 * bqX2_[i] - cA1 * bqY1_[i] - cA2 * bqY2_[i];
            bqX2_[i] = bqX1_[i];
            bqX1_[i] = sig;
            bqY2_[i] = bqY1_[i];
            bqY1_[i] = y;
            sig = y;
        }
        return sig;
    }

    /**
     * @brief Legacy compatibility helper.
     */
    [[nodiscard]] float process(float input, float cutoffHz, bool enabled, float drive, float feedback = 0.55f) noexcept {
        if (!enabled) return applyTanh(input, drive);
        return processTopology(input, cutoffHz, drive, 0, feedback);
    }

private:
    double sampleRate_ = 44100.0;
    double invSr_ = 1.0 / 44100.0;

    static constexpr int kNumStages = 8;
    float x1_[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float y1_[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float x1Inv_[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float y1Inv_[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float bqX1_[2] = {0.0f, 0.0f};
    float bqX2_[2] = {0.0f, 0.0f};
    float bqY1_[2] = {0.0f, 0.0f};
    float bqY2_[2] = {0.0f, 0.0f};
    float dcBlockerState_ = 0.0f;
};

} // namespace CrackedChimes::DSP
