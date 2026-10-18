#pragma once

#include "SoundEngine.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace TbdAudio::Oscillators {

/**
 * @brief Klang808 (alias KlangSlayer) — OSC-09: Roland TR-808 Bridged-T Sub-Bass
 *        Oscillator crossed with Mick Gordon (DOOM 2016 / Eternal) Recursive Sine
 *        Wavefolding & Pseudo-Acoustic Heterodyne Noise Ring Halo.
 *
 * 8-Knob Parameter Topology:
 *   - Knob 0 (OCTAVE)       : [-4..+4] Octaves tracked to MIDI pitch
 *   - Knob 1 (SUB DECAY)    : [0..1] Profile A Decay (10 ms .. 10.0 s + drone sustain at 1.0)
 *   - Knob 2 (LAYER 2 TYPE) : [0..1] 5 Transient/Attack layers (Click, Mallet, 2nd Harm, 3rd Harm, Dirt)
 *   - Knob 3 (LAYER 2 MIX)  : [0..1] Transient layer level + exponential pitch-drop punch depth
 *   - Knob 4 (SINE FOLD)    : [0..1] Mick Gordon recursive sine wavefolder drive
 *   - Knob 5 (NOISE RING)   : [0..1] Sub x Bandlimited Noise heterodyne harmonic halo
 *   - Knob 6 (SYMMETRY)     : [0..1] DC asymmetry bias before wavefolding (even 2nd harmonics)
 *   - Knob 7 (PHASE LOCK)   : [0..1] <=0.5: 0-deg Phase Lock on trigger; >0.5: Free phase + stereo upper spread
 */
class Klang808 final : public SoundEngine8 {
public:
    Klang808() noexcept {
        setParam(0, 0.5f);  // OCTAVE = 0
        setParam(1, 0.55f); // SUB DECAY (~1.6s)
        setParam(2, 0.0f);  // LAYER 2 TYPE = Analog Bridged-T Click
        setParam(3, 0.45f); // LAYER 2 MIX & Pitch Punch
        setParam(4, 0.30f); // SINE FOLD
        setParam(5, 0.20f); // NOISE RING
        setParam(6, 0.15f); // SYMMETRY
        setParam(7, 0.0f);  // PHASE LOCK (0 deg punch lock)
    }

    ~Klang808() override = default;

    void reset() noexcept override {
        subPhase_ = 0.0f;
        layer2Phase_ = 0.0f;
        ampEnv_ = 1.0f;
        pitchEnv_ = 1.0f;
        transEnv_ = 1.0f;
        noiseGen_.reset(0x8080D00Du);
        noiseBandL_.reset();
        noiseBandR_.reset();
        subKeepLow_.reset();
        dcBlockL_.reset();
        dcBlockR_.reset();
    }

protected:
    void onPrepare(float sampleRate) noexcept override {
        reset();
        dcBlockL_.init(sampleRate);
        dcBlockR_.init(sampleRate);
        noiseBandL_.setBandpass(650.0f, 1.4f, invSr_);
        noiseBandR_.setBandpass(780.0f, 1.4f, invSr_);
        subKeepLow_.setCutoff(140.0f, invSr_);
    }

    void onTrigger(float velocity) noexcept override {
        const float phaseLock = std::clamp(params8[7], 0.0f, 1.0f);
        if (phaseLock <= 0.5f) {
            subPhase_ = 0.0f;
            layer2Phase_ = 0.0f;
        }
        ampEnv_ = std::max(0.2f, velocity);
        pitchEnv_ = 1.0f;
        transEnv_ = 1.0f;
    }

public:
    void renderSampleStereo(float baseFreqHz, float& outL, float& outR) noexcept override {
        const int oct = quantizeOctave4(params8[0]);
        const float decayNorm  = std::clamp(params8[1], 0.0f, 1.0f);
        const float layer2Type = std::clamp(params8[2], 0.0f, 1.0f);
        const float layer2Mix  = std::clamp(params8[3], 0.0f, 1.0f);
        const float foldNorm   = std::clamp(params8[4], 0.0f, 1.0f);
        const float noiseRing  = std::clamp(params8[5], 0.0f, 1.0f);
        const float symmetry   = std::clamp(params8[6], 0.0f, 1.0f);
        const float phaseLock  = std::clamp(params8[7], 0.0f, 1.0f);

        // Octave-shifted root sub frequency clamped to musical sub/bass range
        const float octMul = FastMath::fastPow2(static_cast<float>(oct));
        const float rootHz = std::clamp(baseFreqHz * octMul, 16.0f, 4000.0f);

        // Exponential Bridged-T pitch drop envelope (fast 18ms punch sweep up to +3.5 octaves)
        const float pitchDropOct = pitchEnv_ * (0.5f + 3.0f * layer2Mix);
        const float instHz = std::clamp(rootHz * FastMath::fastPow2(pitchDropOct), 16.0f, sampleRate_ * 0.45f);

        // Envelope decay coefficients (zero std::exp, uses FastMath::fastExp)
        const float subDecaySec = Taper::profileA_Decay(decayNorm, false);
        const float ampDecayCoeff = (decayNorm > 0.98f)
            ? 0.999995f
            : FastMath::fastExp(-4.60517f / std::max(1.0f, subDecaySec * sampleRate_));
        const float pitchDecayCoeff = FastMath::fastExp(-1.0f / std::max(1.0f, 0.016f * sampleRate_));
        const float transDecayCoeff = FastMath::fastExp(-1.0f / std::max(1.0f, 0.008f * sampleRate_));

        // Maintain a small floor when rendering continuous un-triggered test blocks
        ampEnv_ = std::max(ampEnv_ * ampDecayCoeff, 0.15f);
        pitchEnv_ *= pitchDecayCoeff;
        transEnv_ *= transDecayCoeff;

        // Advance phase-locked fundamental sub oscillator
        const float dt = instHz * invSr_;
        subPhase_ = PolyBLEP::wrapPhase(subPhase_ + dt);
        layer2Phase_ = PolyBLEP::wrapPhase(layer2Phase_ + dt * 2.0f);

        // Core 1: Pure Bridged-T fundamental sine + subtle 808 collector sag
        const float cleanSub = FastMath::fastSinNorm(subPhase_);

        // Core 2: 5 Selectable Attack / Harmonic Transient Layers
        const int typeIdx = std::clamp(static_cast<int>(layer2Type * 4.0f + 0.5f), 0, 4);
        const float wNoiseL = noiseGen_.nextWhite();
        const float wNoiseR = noiseGen_.nextWhite();
        float layer2Sig = 0.0f;
        switch (typeIdx) {
            case 0: // Analog Bridged-T Click Impulse
                layer2Sig = transEnv_ * (1.2f * FastMath::fastCosNorm(subPhase_) + 0.4f * wNoiseL);
                break;
            case 1: // Acoustic Kick Mallet (160 Hz damped woody thump)
                layer2Sig = transEnv_ * FastMath::fastSinNorm(PolyBLEP::wrapPhase(subPhase_ * 2.6f));
                break;
            case 2: // Continuous 2nd Harmonic Punch
                layer2Sig = (0.35f + 0.65f * transEnv_) * FastMath::fastSinNorm(layer2Phase_);
                break;
            case 3: // Continuous 3rd Harmonic Growl
                layer2Sig = (0.35f + 0.65f * transEnv_) * FastMath::fastSinNorm(PolyBLEP::wrapPhase(subPhase_ * 3.0f));
                break;
            case 4:
            default: // Filtered Dirt Noise Burst
                layer2Sig = (0.20f + 0.80f * transEnv_) * FastMath::fastTanh(wNoiseL * 2.5f);
                break;
        }

        const float preFold = cleanSub + layer2Mix * 0.75f * layer2Sig;

        // Mick Gordon Recursive Sine Wavefolder with DC Symmetry Bias
        // Preserves a rock-solid clean sub foundation via subKeepLow_ while folding upper harmonics
        const float lowSubAnchor = subKeepLow_.processLP(cleanSub);
        const float foldDrive = 0.25f + foldNorm * 2.25f; // normalized cycles for fastSinNorm
        const float symOffset = (symmetry - 0.5f) * 0.45f;

        const float stage1 = FastMath::fastSinNorm(PolyBLEP::wrapPhase(preFold * foldDrive + symOffset));
        const float stage2 = FastMath::fastSinNorm(PolyBLEP::wrapPhase(stage1 * (0.35f + 0.95f * foldNorm) - symOffset * 0.5f));
        const float foldedMono = (1.0f - 0.75f * foldNorm) * preFold
                               + foldNorm * (0.65f * stage1 + 0.35f * stage2);

        // Pseudo-Acoustic Heterodyne Noise Ring-Modulation (Sub * Bandlimited Noise)
        const float bandNoiseL = noiseBandL_.process(wNoiseL);
        const float bandNoiseR = noiseBandR_.process(wNoiseR);
        const float ringHaloL = cleanSub * bandNoiseL * (noiseRing * 2.2f);
        const float ringHaloR = cleanSub * bandNoiseR * (noiseRing * 2.2f);

        // Stereo upper-harmonic spread when PHASE LOCK > 0.5 (keeping sub <140Hz strictly mono)
        const float stereoSpread = (phaseLock > 0.5f) ? ((phaseLock - 0.5f) * 2.0f) : 0.0f;
        const float upperHarm = foldedMono - lowSubAnchor;
        const float upperL = upperHarm * (1.0f + 0.25f * stereoSpread) + ringHaloL;
        const float upperR = (stereoSpread > 0.01f)
            ? (FastMath::fastTanh(upperHarm * (1.0f + 0.4f * stereoSpread)) - ringHaloR)
            : (upperHarm + ringHaloR);

        const float rawL = (0.75f * lowSubAnchor + 0.85f * upperL) * ampEnv_;
        const float rawR = (0.75f * lowSubAnchor + 0.85f * upperR) * ampEnv_;

        outL = std::clamp(dcBlockL_.process(rawL), -4.0f, 4.0f);
        outR = std::clamp(dcBlockR_.process(rawR), -4.0f, 4.0f);
    }

private:
    float subPhase_ = 0.0f;
    float layer2Phase_ = 0.0f;
    float ampEnv_ = 1.0f;
    float pitchEnv_ = 1.0f;
    float transEnv_ = 1.0f;

    PolyBLEP::NoiseGenerator noiseGen_{};
    DSPCommon::CytomicSVF noiseBandL_{};
    DSPCommon::CytomicSVF noiseBandR_{};
    DSPCommon::OnePoleTPT subKeepLow_{};
    DSPCommon::DCBlocker dcBlockL_{};
    DSPCommon::DCBlocker dcBlockR_{};
};

using KlangSlayer = Klang808;
using ArgentSlayer = Klang808;

} // namespace TbdAudio::Oscillators
