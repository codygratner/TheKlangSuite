#pragma once

#include <cmath>
#include <vector>
#include <juce_core/juce_core.h>
#include "../hastur/HasturFilterUtils.h"

namespace Nyarlat {

// A figure-eight algorithmic stereo plate reverb for dark, infinite pad decays
class AbyssReverb {
public:
    void prepare(double sampleRate) noexcept {
        sampleRate_ = (sampleRate > 0.0) ? sampleRate : 44100.0;
        initDelayLines();
        loopDcBlockerL_.prepare(sampleRate_, 10.0);
        loopDcBlockerR_.prepare(sampleRate_, 10.0);
        reset();
    }

    void reset() noexcept {
        for (auto& line : delayLines_) std::fill(line.begin(), line.end(), 0.0f);
        dampStateL_ = 0.0f;
        dampStateR_ = 0.0f;
        loopDcBlockerL_.reset();
        loopDcBlockerR_.reset();
    }

    void processStereo(float& left, float& right, float timeSec, float dampNorm, float mixNorm) noexcept {
        if (mixNorm <= 0.001f) return;

        float inL = left;
        float inR = right;

        // Feedback gain proportional to timeSec (0.2s .. 20s)
        float feedback = juce::jlimit(0.2f, 0.985f, 1.0f - (1.0f / (timeSec * 2.5f + 1.0f)));
        float damp = juce::jlimit(0.05f, 0.95f, 1.0f - dampNorm * 0.75f);

        // Figure-eight loop A & B with in-loop 10Hz DC blocking
        float tapA = loopDcBlockerL_.process(readDelay(0, idxA_));
        float tapB = loopDcBlockerR_.process(readDelay(1, idxB_));

        // Lowpass damping inside loops
        dampStateL_ += damp * (tapA - dampStateL_);
        dampStateR_ += damp * (tapB - dampStateR_);

        // Feed forward into canonical allpass diffusers
        float apL = processAllpass(0, inL + dampStateR_ * feedback, 0.55f);
        float apR = processAllpass(1, inR + dampStateL_ * feedback, 0.55f);

        // Fault guard: catches NaN (comparison false) and sudden runaway
        if (!(std::abs(apL) < 16.0f && std::abs(apR) < 16.0f)) {
            reset();
            return;
        }

        writeDelay(0, idxA_, apL);
        writeDelay(1, idxB_, apR);

        idxA_ = (idxA_ + 1) % lineLengths_[0];
        idxB_ = (idxB_ + 1) % lineLengths_[1];

        float wetL = tapA;
        float wetR = tapB;

        left = left * (1.0f - mixNorm) + wetL * mixNorm;
        right = right * (1.0f - mixNorm) + wetR * mixNorm;
    }

private:
    float readDelay(int lineIdx, int ptr) const noexcept {
        return delayLines_[lineIdx][ptr];
    }

    void writeDelay(int lineIdx, int ptr, float val) noexcept {
        delayLines_[lineIdx][ptr] = val;
    }

    // Canonical Schroeder single-delay-line allpass: |H(e^jw)| == 1.0000 strictly
    float processAllpass(int apIdx, float in, float coeff) noexcept {
        int ptr = apPtrs_[apIdx];
        float delayed = apLines_[apIdx][ptr];
        float v = in + coeff * delayed;
        float out = delayed - coeff * v;
        apLines_[apIdx][ptr] = v;
        apPtrs_[apIdx] = (ptr + 1) % apLengths_[apIdx];
        return out;
    }

    void initDelayLines() {
        // Base lengths scaled for sample rate
        float sScale = static_cast<float>(sampleRate_ / 44100.0);
        lineLengths_[0] = static_cast<int>(4410.0f * sScale);
        lineLengths_[1] = static_cast<int>(5120.0f * sScale);
        delayLines_[0].assign(lineLengths_[0], 0.0f);
        delayLines_[1].assign(lineLengths_[1], 0.0f);

        apLengths_[0] = static_cast<int>(556.0f * sScale);
        apLengths_[1] = static_cast<int>(443.0f * sScale); // Detuned to 443 (coprime to 4410)
        apLines_[0].assign(apLengths_[0], 0.0f);
        apLines_[1].assign(apLengths_[1], 0.0f);
        apPtrs_[0] = 0;
        apPtrs_[1] = 0;
        idxA_ = 0;
        idxB_ = 0;
    }

    double sampleRate_ = 44100.0;
    std::vector<float> delayLines_[2];
    int lineLengths_[2]{ 4410, 5120 };
    int idxA_ = 0;
    int idxB_ = 0;

    std::vector<float> apLines_[2];
    int apLengths_[2]{ 556, 443 };
    int apPtrs_[2]{ 0, 0 };

    float dampStateL_ = 0.0f;
    float dampStateR_ = 0.0f;

    CrackedChimes::DSP::LeakyDCBlocker loopDcBlockerL_;
    CrackedChimes::DSP::LeakyDCBlocker loopDcBlockerR_;
};

} // namespace Nyarlat
