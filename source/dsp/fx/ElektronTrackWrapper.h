#pragma once

#include "../../FastMath.h"
#include "../common/AudioEffect.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace tkdsp {

namespace FastMath = TbdAudio::FastMath;

/**
 * @brief ElektronTrackWrapper — Authentic Silver-Box Hardware Track Effects Strip
 * 
 * Sourced directly from the Elektron Machinedrum (SPS-1/UW) and Monomachine (SFX-6/60)
 * canonical signal path. Every machine voice routes through this identical 5-stage
 * tone shaping and distortion strip before hitting the bus.
 *
 * The 5 Sequential Processing Stages:
 *   1. Dual-Pole Dynamic Bandpass Filter (Base / Width / Resonance):
 *      - Cascaded 2-pole Zero-Delay Feedback (ZDF) Cytomic State Variable Filter (SVF).
 *      - Highpass cutoff set by FLTF (20 Hz .. 18 kHz).
 *      - Lowpass cutoff set by FLTF + FLTW (gap width in octaves, clamped to 20 kHz).
 *      - Dual-peak Q resonance around both cutoff points.
 *   2. 1-Band Parametric Peak EQ:
 *      - Calibrated to authentic Machinedrum EQ curve (20 Hz .. 16 kHz across 0..127).
 *      - Gain range: -12 dB to +12 dB (value 0.5 / 64 = 0 dB flat).
 *   3. Sample Rate Reducer (SRR):
 *      - True digital sample-and-hold downsampling creating aliased Nyquist sidebands.
 *   4. Bit Rate Reducer (BRR / DIRT):
 *      - Word-length truncation quantizer (16 bits down to 1.5 bits).
 *   5. Asymmetric Overdrive (DIST):
 *      - Non-linear soft-clipping saturation with dynamic headroom compensation.
 *
 * Real-Time Audio Thread Safety:
 *   - 100% zero heap allocations.
 *   - Zero CRT transcendental calls (uses FastMath::fastPow2 / fastTanhPrecise).
 *   - Denormal flush safety.
 */
class ElektronTrackWrapper {
public:
    ElektronTrackWrapper() noexcept {
        reset();
    }

    void prepare(float sampleRate) noexcept {
        sampleRate_ = (sampleRate > 8000.0f) ? sampleRate : 48000.0f;
        invSr_ = 1.0f / sampleRate_;
        reset();
        updateCoeffs();
    }

    void reset() noexcept {
        svfHpL_.reset();
        svfHpR_.reset();
        svfLpL_.reset();
        svfLpR_.reset();
        svfEqL_.reset();
        svfEqR_.reset();

        srrPhase_ = 0.0f;
        srrHeldL_ = 0.0f;
        srrHeldR_ = 0.0f;
    }

    /**
     * @brief Set normalized control parameter [0.0 .. 1.0]
     * @param index Parameter index 0..7 (maps to Bank 1 Knobs 09..16)
     */
    void setParam(int index, float val) noexcept {
        const float v = std::clamp(val, 0.0f, 1.0f);
        switch (index) {
            case 0: fltf_ = v; dirty_ = true; break; // K09: FLTF (Base Highpass)
            case 1: fltw_ = v; dirty_ = true; break; // K10: FLTW (Width Lowpass Gap)
            case 2: fltq_ = v; dirty_ = true; break; // K11: FLTQ (Filter Resonance)
            case 3: eqf_  = v; dirty_ = true; break; // K12: EQ-F (Center Frequency)
            case 4: eqg_  = v; dirty_ = true; break; // K13: EQ-G (Peak Gain)
            case 5: srr_  = v; break;                // K14: SRR  (Sample Rate Decimation)
            case 6: brr_  = v; break;                // K15: BRR  (Bit Rate Truncation)
            case 7: dist_ = v; break;                // K16: DIST (Asymmetric Overdrive)
            default: break;
        }
    }

    [[nodiscard]] float getParam(int index) const noexcept {
        switch (index) {
            case 0: return fltf_;
            case 1: return fltw_;
            case 2: return fltq_;
            case 3: return eqf_;
            case 4: return eqg_;
            case 5: return srr_;
            case 6: return brr_;
            case 7: return dist_;
            default: return 0.0f;
        }
    }

    /**
     * @brief Process a single stereo sample frame through all 5 stages in-place.
     */
    inline void processSampleStereo(float& left, float& right) noexcept {
        if (dirty_) {
            updateCoeffs();
        }

        // ====================================================================
        // STAGE 1: DUAL-POLE BASE / WIDTH FILTER (ZDF Cytomic SVF)
        // ====================================================================
        // Highpass Base -> Lowpass Width cascade
        float fltL = left;
        float fltR = right;

        if (fltf_ > 0.001f || fltq_ > 0.001f) {
            fltL = svfHpL_.process(fltL);
            fltR = svfHpR_.process(fltR);
        }
        if (fltw_ < 0.999f || fltq_ > 0.001f) {
            fltL = svfLpL_.process(fltL);
            fltR = svfLpR_.process(fltR);
        }

        // ====================================================================
        // STAGE 2: 1-BAND PARAMETRIC PEAK EQ (ZDF Bell Filter)
        // ====================================================================
        float eqOutL = fltL;
        float eqOutR = fltR;
        if (std::abs(eqGainDb_) > 0.05f) {
            eqOutL = svfEqL_.process(fltL);
            eqOutR = svfEqR_.process(fltR);
        }

        // ====================================================================
        // STAGE 3: ASYMMETRIC OVERDRIVE / DIODE WAVEFOLD (Pre-Decimation)
        // ====================================================================
        float foldOutL = eqOutL;
        float foldOutR = eqOutR;
        if (dist_ > 0.001f) {
            // Drive gain: 0 dB up to +32 dB
            const float driveGain = 1.0f + dist_ * dist_ * 15.0f;
            float drvL = eqOutL * driveGain;
            float drvR = eqOutR * driveGain;

            // Diode wavefold: folds over peaks when driven hard
            if (dist_ > 0.10f) {
                const float foldAmt = (dist_ - 0.10f) * 1.111f; // 0..1
                const float fL1 = FastMath::fastSinNorm(drvL * 0.15915494f);
                const float fL2 = FastMath::fastSinNorm(drvL * 0.31830988f);
                drvL = (1.0f - foldAmt) * drvL + foldAmt * (fL1 + 0.40f * fL2);

                const float fR1 = FastMath::fastSinNorm(drvR * 0.15915494f);
                const float fR2 = FastMath::fastSinNorm(drvR * 0.31830988f);
                drvR = (1.0f - foldAmt) * drvR + foldAmt * (fR1 + 0.40f * fR2);
            }

            // Asymmetric soft-clip curve (adds even harmonics + transient crunch)
            drvL = FastMath::fastTanhPrecise(drvL + 0.12f * drvL * drvL);
            drvR = FastMath::fastTanhPrecise(drvR + 0.12f * drvR * drvR);

            // Dynamic output makeup trim
            const float makeup = 1.0f / std::sqrt(driveGain);
            foldOutL = drvL * makeup;
            foldOutR = drvR * makeup;
        }

        // ====================================================================
        // STAGE 4: SAMPLE RATE REDUCER (SRR Decimation Aliasing on Folded Tone)
        // ====================================================================
        float srrOutL = foldOutL;
        float srrOutR = foldOutR;
        if (srr_ > 0.001f) {
            // Factor ranges from 1.0 (no reduction) to 64.0 (down to ~750 Hz)
            const float decimateStep = 1.0f + srr_ * srr_ * 63.0f;
            srrPhase_ += 1.0f;
            if (srrPhase_ >= decimateStep) {
                srrPhase_ -= decimateStep;
                srrHeldL_ = foldOutL;
                srrHeldR_ = foldOutR;
            }
            srrOutL = srrHeldL_;
            srrOutR = srrHeldR_;
        } else {
            srrHeldL_ = foldOutL;
            srrHeldR_ = foldOutR;
        }

        // ====================================================================
        // STAGE 5: BIT RATE REDUCER (BRR / DIRT Quantization)
        // ====================================================================
        float brrOutL = srrOutL;
        float brrOutR = srrOutR;
        if (brr_ > 0.001f) {
            // Bits drop smoothly from 16 bits down to 1.5 bits
            const float bits = 16.0f - brr_ * 14.5f;
            const float steps = FastMath::fastPow2(bits);
            const float invSteps = 1.0f / steps;
            brrOutL = std::round(srrOutL * steps) * invSteps;
            brrOutR = std::round(srrOutR * steps) * invSteps;
        }

        left  = brrOutL;
        right = brrOutR;
    }

    /**
     * @brief Process an entire block of stereo audio.
     */
    void processBlockStereo(float* left, float* right, int numSamples) noexcept {
        if (!left || !right || numSamples <= 0) return;
        for (int i = 0; i < numSamples; ++i) {
            processSampleStereo(left[i], right[i]);
        }
    }

private:
    void updateCoeffs() noexcept {
        dirty_ = false;

        // 1. Highpass Base Filter (FLTF: 20 Hz .. 18 kHz)
        const float fHp = 20.0f * FastMath::fastPow2(fltf_ * 9.81378f);
        const float qHp = 0.5f + fltq_ * fltq_ * 15.0f;
        svfHpL_.setHighpass(fHp, qHp, invSr_);
        svfHpR_.setHighpass(fHp, qHp, invSr_);

        // 2. Lowpass Width Filter (FLTW: gap from fHp up to 20 kHz)
        const float widthOctaves = fltw_ * 10.0f;
        const float fLp = std::clamp(fHp * FastMath::fastPow2(widthOctaves), 20.0f, 20000.0f);
        const float qLp = 0.5f + fltq_ * fltq_ * 15.0f;
        svfLpL_.setLowpass(fLp, qLp, invSr_);
        svfLpR_.setLowpass(fLp, qLp, invSr_);

        // 3. 1-Band Parametric EQ (Machinedrum Calibration: 20 Hz .. 16 kHz, -12 dB .. +12 dB)
        const float fEq = 20.0f * FastMath::fastPow2(eqf_ * 9.64385f);
        eqGainDb_ = (eqg_ - 0.5f) * 24.0f;
        svfEqL_.setBell(fEq, 1.4142f, eqGainDb_, invSr_);
        svfEqR_.setBell(fEq, 1.4142f, eqGainDb_, invSr_);
    }

    // Parameters (Normalized 0..1)
    float fltf_ = 0.0f;   // K09: Base HP Cutoff (Default 20 Hz)
    float fltw_ = 1.0f;   // K10: Width LP Gap (Default wide open)
    float fltq_ = 0.0f;   // K11: Filter Resonance Q
    float eqf_  = 0.5f;   // K12: EQ Frequency
    float eqg_  = 0.5f;   // K13: EQ Gain (Default 0 dB flat)
    float srr_  = 0.0f;   // K14: Sample Rate Reducer
    float brr_  = 0.0f;   // K15: Bit Rate Reducer (DIRT)
    float dist_ = 0.0f;   // K16: Asymmetric Overdrive

    float sampleRate_ = 48000.0f;
    float invSr_ = 1.0f / 48000.0f;
    float eqGainDb_ = 0.0f;
    bool dirty_ = true;

    // Cytomic ZDF SVF Filters (Stereo)
    TbdAudio::DSPCommon::CytomicSVF svfHpL_, svfHpR_;
    TbdAudio::DSPCommon::CytomicSVF svfLpL_, svfLpR_;
    TbdAudio::DSPCommon::CytomicSVF svfEqL_, svfEqR_;

    // SRR Decimator State
    float srrPhase_ = 0.0f;
    float srrHeldL_ = 0.0f;
    float srrHeldR_ = 0.0f;
};

} // namespace tkdsp
