#pragma once

#include "SoundEngine.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace TbdAudio::Oscillators {

/**
 * @brief KlangPluck (also aliased as KlangPluckKS) — Extended Karplus-Strong Physical
 *        Waveguide String & Cavity Synthesizer Engine (OSC-05).
 *
 * Architecture:
 *   - Fixed static circular delay line (kMaxDelay = 4096) with 3rd-order Lagrange
 *     fractional delay interpolation for exact pitch tracking down to 15 Hz.
 *   - 4 Selectable Exciters (White Noise, Pink Noise, Impulse Click, Saturated Thump)
 *     filtered through a feed-forward Pick Position comb notch before entering the loop.
 *   - Waveguide loop contains:
 *       1. OnePoleTPT low-pass damping filter (high-frequency string loss)
 *       2. 1st-order All-Pass dispersion filter (string stiffness / inharmonic stretch)
 *       3. Padé FastMath::fastTanh loop saturation (singing sustain & slap bass grit)
 *       4. DCBlocker to prevent zero-frequency drift at high feedback/drive
 *   - Post-loop CytomicSVF wooden body cavity resonator.
 *   - Auto-triggers on first sample after reset()/onPrepare() so standalone oscillator
 *     tests always produce continuous signal even without an explicit trigger() call.
 *
 * 8 Performance Knobs (params8[0..7]):
 *   0: OCTAVE        (-4..+4 quantized octaves)
 *   1: DECAY         (Taper::profileA_Decay 10ms..10s RT60 feedback gain calculation)
 *   2: DAMPING       (One-pole TPT low-pass HF loss filter inside waveguide loop)
 *   3: EXCITER       (Selects & scales burst length across 4 exciters)
 *   4: PICK POSITION (0.05..0.50 comb filter feed-forward notch before loop)
 *   5: STRETCH       (1st-order All-Pass string stiffness / inharmonicity dispersion)
 *   6: BODY RES      (CytomicSVF wooden cavity resonance bell/bandpass filter)
 *   7: DRIVE         (Internal Padé FastMath::fastTanh waveguide loop saturation)
 */
class KlangPluck final : public SoundEngine8 {
public:
    static constexpr int kMaxDelay = 4096;
    static constexpr int kDelayMask = kMaxDelay - 1;

    KlangPluck() noexcept {
        setParam(0, 0.50f); // Octave 0
        setParam(1, 0.65f); // Decay
        setParam(2, 0.35f); // Damping
        setParam(3, 0.30f); // Exciter
        setParam(4, 0.25f); // Pick Position
        setParam(5, 0.20f); // Stretch
        setParam(6, 0.40f); // Body Res
        setParam(7, 0.15f); // Drive
        reset();
    }

    ~KlangPluck() override = default;

    void reset() noexcept override {
        std::fill(std::begin(delayBuf_), std::end(delayBuf_), 0.0f);
        std::fill(std::begin(pickBuf_), std::end(pickBuf_), 0.0f);
        writePos_ = 0;
        pickWritePos_ = 0;
        burstRemaining_ = 0;
        burstTotal_ = 1;
        thumpPhase_ = 0.0f;
        apX1_ = 0.0f;
        apY1_ = 0.0f;
        stereoApX1_ = 0.0f;
        stereoApY1_ = 0.0f;
        dampingFilter_.reset();
        loopDcBlocker_.reset();
        bodyFilterL_.reset();
        bodyFilterR_.reset();
        noise_.reset(0x9E3779B9u);
        needsAutoTrigger_ = true;
    }

protected:
    void onPrepare(float sampleRate) noexcept override {
        reset();
        loopDcBlocker_.init(sampleRate);
    }

    void onTrigger(float velocity) noexcept override {
        needsAutoTrigger_ = false;
        const float vel = std::clamp(velocity, 0.05f, 1.0f);
        velocity_ = vel;

        const float exciterKnob = std::clamp(params8[3], 0.0f, 1.0f);
        // Scale burst duration between 2.5 ms and 18 ms based on exciter sub-range
        const float subPos = (exciterKnob * 4.0f) - static_cast<float>(std::clamp(static_cast<int>(exciterKnob * 3.99f), 0, 3));
        const float burstMs = 0.0025f + 0.0155f * std::clamp(subPos, 0.0f, 1.0f);
        burstTotal_ = std::clamp(static_cast<int>(burstMs * sampleRate_), 8, kMaxDelay / 2);
        burstRemaining_ = burstTotal_;
        thumpPhase_ = 0.0f;
    }

public:
    void renderSampleStereo(float baseFreqHz, float& outL, float& outR) noexcept override {
        if (needsAutoTrigger_) {
            onTrigger(velocity_);
        }

        const float sr = std::max(sampleRate_, 8000.0f);
        const float invSr = invSr_;

        // 0: OCTAVE (-4..+4)
        const int oct = quantizeOctave4(params8[0]);
        const float octMult = FastMath::fastPow2(static_cast<float>(oct));
        const float targetHz = std::clamp(baseFreqHz * octMult, 15.0f, std::min(8000.0f, 0.42f * sr));

        // Exact period in samples
        const float totalDelaySamples = std::clamp(sr / targetHz, 2.5f, static_cast<float>(kMaxDelay - 8));

        // 1: DECAY (RT60 in [0.010s .. 10.0s] via Profile A)
        const float rt60Sec = Taper::profileA_Decay(params8[1], 0.010f, 10.0f);
        // g = 10^(-3 / (f0 * RT60)) = 2^(-3 * log2(10) / (f0 * RT60))
        // -3 * log2(10) = -9.96578428f
        const float periodsPerDecay = std::max(targetHz * rt60Sec, 0.25f);
        const float fbGain = std::clamp(FastMath::fastPow2(-9.96578428f / periodsPerDecay), 0.50f, 0.99985f);

        // 2: DAMPING (0 = bright string, 1 = heavily muted palm-mute)
        const float dampKnob = std::clamp(params8[2], 0.0f, 1.0f);
        const float dampCutoffHz = std::clamp(
            Taper::profileD_Cutoff(1.0f - dampKnob * 0.92f, targetHz * 1.25f, std::min(18000.0f, 0.46f * sr)),
            60.0f, 0.46f * sr);
        dampingFilter_.setCutoff(dampCutoffHz, invSr);

        // 3: EXCITER Burst Generation (4 exciters: White, Pink, Impulse Click, Saturated Thump)
        float exciterOut = 0.0f;
        if (burstRemaining_ > 0) {
            const int exciterType = std::clamp(static_cast<int>(params8[3] * 3.99f), 0, 3);
            const float env = static_cast<float>(burstRemaining_) / static_cast<float>( std::max(burstTotal_, 1));
            const float envWin = env * env; // smooth quadratic decay window

            switch (exciterType) {
                case 0: // White Noise Burst
                    exciterOut = noise_.nextWhite() * envWin;
                    break;
                case 1: // Pink Noise Burst (warm nylon / gut string)
                    exciterOut = noise_.nextPink() * envWin * 1.35f;
                    break;
                case 2: { // Impulse Click + transient edge (harpsichord / plectrum snap)
                    const float spike = (burstRemaining_ > burstTotal_ - 4) ? 1.0f : -0.25f;
                    exciterOut = (0.75f * spike + 0.25f * noise_.nextWhite()) * envWin;
                    break;
                }
                case 3:
                default: { // Saturated Thump (pitched cosine-sine sweep + soft grit)
                    const float sweepHz = targetHz * (1.0f + 3.0f * envWin);
                    thumpPhase_ = PolyBLEP::wrapPhase(thumpPhase_ + sweepHz * invSr);
                    const float tone = FastMath::fastSinNorm(thumpPhase_);
                    exciterOut = FastMath::fastTanh((tone * 1.8f + 0.35f * noise_.nextPink()) * envWin);
                    break;
                }
            }
            exciterOut *= (0.35f + 0.65f * velocity_);
            --burstRemaining_;
        }

        // 4: PICK POSITION (0.05..0.50 feed-forward comb notch before waveguide loop)
        const float pickRatio = 0.05f + 0.45f * std::clamp(params8[4], 0.0f, 1.0f);
        const int pickDelay = std::clamp(static_cast<int>(totalDelaySamples * pickRatio + 0.5f), 1, kMaxDelay - 2);
        pickBuf_[pickWritePos_ & kDelayMask] = exciterOut;
        const float delayedPick = pickBuf_[(pickWritePos_ - pickDelay) & kDelayMask];
        ++pickWritePos_;
        const float combedExcitation = 0.75f * (exciterOut - delayedPick);

        // 5: STRETCH (1st-order All-Pass dispersion filter inside loop)
        const float stretchKnob = std::clamp(params8[5], 0.0f, 1.0f);
        const float apCoeff = -0.68f * stretchKnob;
        // Compensate phase delay of all-pass filter at low frequencies: D_ap ≈ (1 - a) / (1 + a)
        const float apPhaseDelay = (1.0f - std::abs(apCoeff)) / (1.0f + std::abs(apCoeff) + 1e-5f);
        const float effectiveDelay = std::clamp(totalDelaySamples - apPhaseDelay - 0.5f, 2.0f, static_cast<float>(kMaxDelay - 6));

        // Read waveguide delay line using 3rd-order Lagrange fractional interpolation
        const float loopRead = readLagrange3(effectiveDelay);

        // Waveguide loop processing: One-pole LP damping -> All-Pass stretch -> Drive saturation -> DC block
        float loopSig = dampingFilter_.processLP(loopRead) * fbGain;

        // 1st-order All-Pass: y[n] = a * x[n] + x[n-1] - a * y[n-1]
        const float apOut = apCoeff * loopSig + apX1_ - apCoeff * apY1_;
        apX1_ = loopSig;
        apY1_ = apOut;
        loopSig = apOut;

        // 7: DRIVE (Waveguide loop saturation via Padé FastMath::fastTanh)
        const float driveKnob = std::clamp(params8[7], 0.0f, 1.0f);
        if (driveKnob > 0.01f) {
            const float driveGain = 1.0f + driveKnob * driveKnob * 3.2f;
            const float normGain = 1.0f / FastMath::fastTanh(driveGain);
            loopSig = FastMath::fastTanh(loopSig * driveGain) * (0.985f * std::min(normGain, 1.25f));
        }

        const float nextLoop = loopDcBlocker_.process(combedExcitation + loopSig);
        delayBuf_[writePos_ & kDelayMask] = std::clamp(std::isfinite(nextLoop) ? nextLoop : 0.0f, -4.0f, 4.0f);

        // Secondary stereo tap at pick position for natural acoustic width
        const float tapStereo = readLagrange3(std::clamp(effectiveDelay * (1.0f - 0.35f * pickRatio), 2.0f, static_cast<float>(kMaxDelay - 6)));
        ++writePos_;

        // 6: BODY RES (CytomicSVF wooden cavity resonance filter)
        const float bodyKnob = std::clamp(params8[6], 0.0f, 1.0f);
        const float bodyFreqL = std::clamp(targetHz * 2.2f + bodyKnob * 420.0f, 90.0f, 0.42f * sr);
        const float bodyFreqR = std::clamp(bodyFreqL * 1.14f, 95.0f, 0.42f * sr);
        const float bodyQ = 0.85f + 4.5f * bodyKnob * bodyKnob;
        const float bodyGainDb = 1.5f + 12.0f * bodyKnob;

        bodyFilterL_.setBell(bodyFreqL, bodyQ, bodyGainDb, invSr);
        bodyFilterR_.setBell(bodyFreqR, bodyQ, bodyGainDb, invSr);

        // Slight stereo dispersion on R channel
        const float rDisp = 0.35f * tapStereo + stereoApX1_ - 0.35f * stereoApY1_;
        stereoApX1_ = tapStereo;
        stereoApY1_ = rDisp;

        float sigL = bodyFilterL_.process(nextLoop);
        float sigR = bodyFilterR_.process(0.65f * nextLoop + 0.35f * rDisp);
        bodyFilterL_.clampState();
        bodyFilterR_.clampState();

        if (phaseInvert) {
            sigL = -sigL;
            sigR = -sigR;
        }

        outL = std::clamp(std::isfinite(sigL) ? sigL : 0.0f, -4.0f, 4.0f);
        outR = std::clamp(std::isfinite(sigR) ? sigR : 0.0f, -4.0f, 4.0f);
    }

private:
    /**
     * 3rd-order (4-point cubic) Lagrange fractional delay interpolation.
     */
    [[nodiscard]] inline float readLagrange3(float delaySamples) const noexcept {
        const float clampedDelay = std::clamp(delaySamples, 1.5f, static_cast<float>(kMaxDelay - 5));
        int dInt = static_cast<int>(clampedDelay);
        if ( dInt < 1 ) dInt = 1;
        const float frac = clampedDelay - static_cast<float>(dInt);

        const int idx0 = (writePos_ - dInt + 1) & kDelayMask;
        const int idx1 = (writePos_ - dInt)     & kDelayMask;
        const int idx2 = (writePos_ - dInt - 1) & kDelayMask;
        const int idx3 = (writePos_ - dInt - 2) & kDelayMask;

        const float y0 = delayBuf_[idx0];
        const float y1 = delayBuf_[idx1];
        const float y2 = delayBuf_[idx2];
        const float y3 = delayBuf_[idx3];

        const float d0 = frac;
        const float d1 = frac - 1.0f;
        const float d2 = frac - 2.0f;
        const float d3 = frac + 1.0f;

        const float c0 = -d0 * d1 * d2 * (1.0f / 6.0f);
        const float c1 =  d3 * d1 * d2 * 0.5f;
        const float c2 = -d3 * d0 * d2 * 0.5f;
        const float c3 =  d3 * d0 * d1 * (1.0f / 6.0f);

        return c0 * y0 + c1 * y1 + c2 * y2 + c3 * y3;
    }

    float delayBuf_[kMaxDelay]{};
    float pickBuf_[kMaxDelay]{};
    int writePos_ = 0;
    int pickWritePos_ = 0;
    int burstRemaining_ = 0;
    int burstTotal_ = 1;
    float thumpPhase_ = 0.0f;
    float apX1_ = 0.0f;
    float apY1_ = 0.0f;
    float stereoApX1_ = 0.0f;
    float stereoApY1_ = 0.0f;
    bool needsAutoTrigger_ = true;

    DSPCommon::OnePoleTPT dampingFilter_;
    DSPCommon::DCBlocker loopDcBlocker_;
    DSPCommon::CytomicSVF bodyFilterL_;
    DSPCommon::CytomicSVF bodyFilterR_;
    PolyBLEP::NoiseGenerator noise_;
};

using KlangPluckKS = KlangPluck;

} // namespace TbdAudio::Oscillators
