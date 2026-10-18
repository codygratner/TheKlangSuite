#pragma once

#include "SoundEngine.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace TbdAudio::Oscillators {

/**
 * @brief OSC-02: KlangAlpha (alias KlangJuno) — Dual DCO & Unison Synthesizer (14-Waveform
 *        Unipolar XOR DCO + 7-Voice Supersaw Stack + Variable PWM + Stereo Ensemble 80s BBD Chorus).
 *
 * 8 Performance Knobs (params8[0..7]):
 *   [0] WAVE 1            : 14 normalized Alpha shapes [0..13] via PolyBLEP::alphaWave14
 *   [1] OCTAVE            : Discrete [-4..+4] octaves via quantizeOctave4
 *   [2] WAVE 2            : 0 = Off, 1..14 = 14 Alpha shapes for Anchor/Sub DCO
 *   [3] SUPER             : 0..100% 7-voice detuned unison stack on Wave 1 with stereo pan spread
 *   [4] PWM AMOUNT        : 0..100% pulse width modulation depth
 *   [5] PWM SPEED         : 0.02 Hz .. 2500 Hz via Taper::profileC_ModSpeed
 *   [6] DETUNE            : -12..+12 semitones fine/coarse offset on Wave 2 + Super spread
 *   [7] RETRIGGER/CHORUS  : 0.00..0.33 = Free phase,
 *                           0.34..0.66 = Reset 0 deg on trigger,
 *                           0.67..1.00 = Stereo Ensemble 80s BBD Chorus I/II modulation
 *
 * Real-Time Safety Invariants:
 *   - Zero heap allocations.
 *   - Zero CRT transcendental calls (FastMath only).
 *   - Finite output clamped to [-4.0f, 4.0f].
 */
class AlphaKraft final : public SoundEngine8 {
public:
    enum ParamIndex : int {
        kWave1 = 0,
        kOctave = 1,
        kWave2 = 2,
        kSuper = 3,
        kPwmAmount = 4,
        kPwmSpeed = 5,
        kDetune = 6,
        kRetrigChorus = 7
    };

    AlphaKraft() noexcept {
        setParam8(kWave1, 0.0f);        // Shape 0 (Saw 1)
        setParam8(kOctave, 0.5f);       // 0 octaves
        setParam8(kWave2, 0.0f);        // Wave 2 Off
        setParam8(kSuper, 0.35f);       // Moderate Super stack
        setParam8(kPwmAmount, 0.40f);   // 40% PWM
        setParam8(kPwmSpeed, 0.25f);    // Classic LFO PWM speed
        setParam8(kDetune, 0.5f);       // 0 semitones center
        setParam8(kRetrigChorus, 0.80f);// Chorus I/II active
        reset();
    }

    ~AlphaKraft() override = default;

    void reset() noexcept override {
        for (int v = 0; v < 7; ++v) {
            phases1_[v] = kInitPhases[v];
        }
        phase2_ = 0.0f;
        pwmLfoPhase_ = 0.0f;
        chorusLfo1_ = 0.0f;
        chorusLfo2_ = 0.25f;
        chorusWriteIdx_ = 0;
        for (int c = 0; c < 2; ++c) {
            for (int i = 0; i < kChorusBufSize; ++i) {
                chorusBuf_[c][i] = 0.0f;
            }
        }
        bbdFilterL_.reset();
        bbdFilterR_.reset();
        dcL_.reset();
        dcR_.reset();
    }

protected:
    void onPrepare(float sampleRate) noexcept override {
        bbdFilterL_.setCutoff(std::min(9500.0f, sampleRate * 0.42f), invSr_);
        bbdFilterR_.setCutoff(std::min(9500.0f, sampleRate * 0.42f), invSr_);
        dcL_.init(sampleRate);
        dcR_.init(sampleRate);
    }

    void onTrigger(float /*velocity*/) noexcept override {
        const float retrigMode = getParam(kRetrigChorus);
        if (retrigMode >= 0.34f && retrigMode <= 0.66f) {
            // Hard phase reset (0 deg) on trigger for tight punchy bass / pluck
            for (int v = 0; v < 7; ++v) {
                phases1_[v] = 0.0f;
            }
            phase2_ = 0.0f;
        }
    }

public:
    void renderSampleStereo(float baseFreqHz, float& outL, float& outR) noexcept override {
        // 1. Decode parameters
        const int shape1 = std::clamp(static_cast<int>(getParam(kWave1) * 13.99f), 0, 13);
        const int octOffset = quantizeOctave4(getParam(kOctave));
        const int wave2Raw = std::clamp(static_cast<int>(getParam(kWave2) * 14.99f), 0, 14);
        const float superAmt = std::clamp(getParam(kSuper), 0.0f, 1.0f);
        const float pwmDepth = std::clamp(getParam(kPwmAmount), 0.0f, 1.0f);
        const float pwmRateHz = Taper::profileC_ModSpeed(getParam(kPwmSpeed), 0.02f, 2500.0f);
        const float detuneNorm = getParam(kDetune); // 0..1 -> -12..+12 st
        const float detuneSemis = (detuneNorm - 0.5f) * 24.0f;
        const float retrigChorus = std::clamp(getParam(kRetrigChorus), 0.0f, 1.0f);

        // 2. Fundamental frequency with octave shift
        const float octMult = FastMath::fastPow2(static_cast<float>(octOffset));
        const float freq1Hz = std::clamp(baseFreqHz * octMult, 8.0f, sampleRate_ * 0.47f);
        const float dtCenter = freq1Hz * invSr_;

        // 3. Advance PWM LFO (triangle-sine hybrid for classic Roland DCO sweep)
        pwmLfoPhase_ = PolyBLEP::wrapPhase(pwmLfoPhase_ + pwmRateHz * invSr_);
        const float pwmMod = FastMath::fastSinNorm(pwmLfoPhase_) * pwmDepth;

        // 4. Render Wave 1 + 7-Voice JP-8000 Supersaw Unison Stack
        // Center voice (index 3) is always active; side voices (0..2, 4..6) scale in with superAmt
        const float spreadScale = 0.15f + 0.85f * superAmt + 0.25f * (std::abs(detuneSemis) * (1.0f / 12.0f));
        const float sideGain = superAmt * 0.42f;
        const float centerGain = 1.0f - 0.38f * superAmt;

        float sumL = 0.0f;
        float sumR = 0.0f;

        for (int v = 0; v < 7; ++v) {
            float voiceDt = dtCenter;
            if (v != 3) {
                const float detRatio = 1.0f + kJp8000DetuneOffsets[v] * spreadScale;
                voiceDt = std::clamp(dtCenter * detRatio, 1e-6f, 0.48f);
            }

            phases1_[v] = PolyBLEP::wrapPhase(phases1_[v] + voiceDt);

            // Evaluate Wave 1 only for active voices (skip side voices when superAmt is near zero)
            if (v == 3) {
                const float s = PolyBLEP::alphaWave14(shape1, phases1_[3], voiceDt, pwmMod);
                sumL += s * centerGain;
                sumR += s * centerGain;
            } else if (superAmt > 0.001f) {
                const float s = PolyBLEP::alphaWave14(shape1, phases1_[v], voiceDt, pwmMod);
                sumL += s * sideGain * kPanLeft[v];
                sumR += s * sideGain * kPanRight[v];
            }
        }

        // 5. Render Wave 2 (Anchor / Sub DCO: 0 = Off, 1..14 = shapes 0..13)
        if (wave2Raw > 0) {
            const int shape2 = wave2Raw - 1;
            // Fine/coarse pitch offset from DETUNE (-12..+12 semitones); defaults to -1 octave sub when centered near 0 if desired,
            // or tracks exact semitone offset (-12..+12 st) relative to Wave 1
            const float freq2Ratio = FastMath::fastPow2(detuneSemis * (1.0f / 12.0f));
            const float freq2Hz = std::clamp(freq1Hz * freq2Ratio, 8.0f, sampleRate_ * 0.47f);
            const float dt2 = freq2Hz * invSr_;
            phase2_ = PolyBLEP::wrapPhase(phase2_ + dt2);
            const float w2 = PolyBLEP::alphaWave14(shape2, phase2_, dt2, -pwmMod * 0.8f);

            sumL = 0.68f * sumL + 0.52f * w2;
            sumR = 0.68f * sumR + 0.52f * w2;
        }

        // 6. Stereo Ensemble 80s BBD Chorus I/II when Knob 7 is in [0.67 .. 1.0]
        // Always feed the circular buffer so transitions into Chorus are click-free
        const int wIdx = chorusWriteIdx_ & kChorusMask;
        chorusBuf_[0][wIdx] = sumL;
        chorusBuf_[1][wIdx] = sumR;
        chorusWriteIdx_ = (wIdx + 1) & kChorusMask;

        if (retrigChorus >= 0.67f) {
            // Morph from Chorus I (0.5 Hz) to Chorus I+II / Chorus II (0.8 Hz + fast vibrato)
            const float chorusNorm = (retrigChorus - 0.67f) * (1.0f / 0.33f);
            const float rate1Hz = 0.50f + 0.30f * chorusNorm;
            const float rate2Hz = 0.80f + 2.40f * chorusNorm;

            chorusLfo1_ = PolyBLEP::wrapPhase(chorusLfo1_ + rate1Hz * invSr_);
            chorusLfo2_ = PolyBLEP::wrapPhase(chorusLfo2_ + rate2Hz * invSr_);

            // Triangle LFOs as in classic 80s analog BBD clock driver
            const float tri1 = PolyBLEP::triangle(chorusLfo1_);
            const float tri2 = PolyBLEP::triangle(chorusLfo2_);
            const float modL = 0.7f * tri1 + 0.3f * tri2;
            const float modR = -0.7f * tri1 + 0.3f * tri2; // Complementary 180-deg phase for wide stereo

            // Nominal BBD center delay ~3.4 ms, depth ~1.8 ms
            const float centerSamples = 0.0034f * sampleRate_;
            const float depthSamples = (0.0014f + 0.0008f * chorusNorm) * sampleRate_;

            const float delayL = std::clamp(centerSamples + modL * depthSamples, 2.0f, static_cast<float>(kChorusBufSize - 4));
            const float delayR = std::clamp(centerSamples + modR * depthSamples, 2.0f, static_cast<float>(kChorusBufSize - 4));

            const float wetL = bbdFilterL_.processLP(readChorusInterp(0, wIdx, delayL));
            const float wetR = bbdFilterR_.processLP(readChorusInterp(1, wIdx, delayR));

            const float wetMix = 0.35f + 0.30f * chorusNorm;
            sumL = sumL * (1.0f - 0.3f * wetMix) + wetL * wetMix;
            sumR = sumR * (1.0f - 0.3f * wetMix) + wetR * wetMix;
        }

        // 7. Velocity scaling, soft saturation, DC blocking, and hard safety clamp
        const float velGain = 0.25f + 0.75f * velocity_;
        outL = dcL_.process(FastMath::fastTanh(sumL * velGain * 0.85f));
        outR = dcR_.process(FastMath::fastTanh(sumR * velGain * 0.85f));

        outL = std::clamp(std::isfinite(outL) ? outL : 0.0f, -4.0f, 4.0f);
        outR = std::clamp(std::isfinite(outR) ? outR : 0.0f, -4.0f, 4.0f);
    }

private:
    static constexpr int kChorusBufSize = 1024;
    static constexpr int kChorusMask = kChorusBufSize - 1;

    // Authentic JP-8000 non-linear asymmetric detune ratios
    static constexpr float kJp8000DetuneOffsets[7] = {
        -0.11002313f, -0.06288439f, -0.01952356f,
         0.0f,
         0.01991221f,  0.06216538f,  0.10745242f
    };

    // Stereo pan weights for the 7 unison voices (constant-power-like spread)
    static constexpr float kPanLeft[7] = {
        0.95f, 0.78f, 0.62f, 0.50f, 0.38f, 0.22f, 0.05f
    };
    static constexpr float kPanRight[7] = {
        0.05f, 0.22f, 0.38f, 0.50f, 0.62f, 0.78f, 0.95f
    };

    static constexpr float kInitPhases[7] = {
        0.0f, 0.142857f, 0.285714f, 0.428571f, 0.571428f, 0.714285f, 0.857142f
    };

    [[nodiscard]] inline float readChorusInterp(int ch, int writeIdx, float delaySamples) const noexcept {
        float readPos = static_cast<float>(writeIdx) - delaySamples;
        while (readPos < 0.0f) {
            readPos += static_cast<float>(kChorusBufSize);
        }
        const int idx0 = static_cast<int>(readPos) & kChorusMask;
        const int idx1 = (idx0 + 1) & kChorusMask;
        const float frac = readPos - static_cast<float>(static_cast<int>(readPos));
        return chorusBuf_[ch][idx0] + frac * (chorusBuf_[ch][idx1] - chorusBuf_[ch][idx0]);
    }

    float phases1_[7] = { 0.0f, 0.142857f, 0.285714f, 0.428571f, 0.571428f, 0.714285f, 0.857142f };
    float phase2_ = 0.0f;
    float pwmLfoPhase_ = 0.0f;
    float chorusLfo1_ = 0.0f;
    float chorusLfo2_ = 0.25f;
    int chorusWriteIdx_ = 0;
    float chorusBuf_[2][kChorusBufSize] = {};

    DSPCommon::OnePoleTPT bbdFilterL_;
    DSPCommon::OnePoleTPT bbdFilterR_;
    DSPCommon::DCBlocker dcL_;
    DSPCommon::DCBlocker dcR_;
};



using KlangAlpha = AlphaKraft;
using KlangJuno = AlphaKraft;

} // namespace TbdAudio::Oscillators
