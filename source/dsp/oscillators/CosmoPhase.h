#pragma once

#include "SoundEngine.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace TbdAudio::Oscillators {

/**
 * @brief OSC-03: KlangCZ (KlangCZPhaseX) — Casio CZ-101 / VZ-1 Twin-Line Phase Distortion
 *        Synthesizer with Dual-Slope Inflection Transfer, Windowed Resonance Sync, and 7 Interaction Modes.
 *
 * 8 Waveforms (0..7):
 *   0: Saw            (Dual-slope inflection at d = 0.5 * (1 - 0.96 * dcw))
 *   1: Square         (Symmetric dual-plateau phase warp)
 *   2: Pulse          (Narrow spike phase concentration)
 *   3: DoubleSine     (1x -> 2x harmonic phase doubling inside cycle)
 *   4: SawPulse       (Hybrid asymmetric half-saw / half-pulse transfer)
 *   5: Reso1_SawSync  (Hard-synced resonant carrier multiplied by linear decay window)
 *   6: Reso2_TriSync  (Hard-synced resonant carrier multiplied by triangle window)
 *   7: Reso3_TrapSync (Hard-synced resonant carrier multiplied by trapezoidal window)
 *
 * 7 Interaction Modes (0..6):
 *   0: Normal         (L1 + L2)
 *   1: RingMod        (L1 * L2)
 *   2: PhaseDistort   (theta2 += k * L1 prior to Line 2 PD transfer)
 *   3: CrossMod       (Mutual bi-directional phase modulation between Line 1 & Line 2)
 *   4: HardSync       (Line 1 master wrap resets Line 2 phase)
 *   5: LogicXOR       (Bitwise sign/comparator XOR emulation between L1 and L2)
 *   6: MinMaxWavefold (Non-linear min/max reflection wavefolder on L1 & L2)
 *
 * 8 Performance Knobs (params8[0..7]):
 *   [0] WAVE 1        : Line 1 waveform [0..7]
 *   [1] PD 1 (DCW 1)  : Line 1 Digital Controlled Waveform depth [0..1]
 *   [2] WAVE 2        : Line 2 waveform [0..7]
 *   [3] PD 2 (DCW 2)  : Line 2 Digital Controlled Waveform depth [0..1]
 *   [4] OCTAVE        : Master octave shift [-4..+4]
 *   [5] DETUNE 2      : Line 2 pitch offset [-1..+5] octaves (continuous semitone/octave sweep)
 *   [6] AMP MIX       : Line 1 vs Line 2 balance [0..1]
 *   [7] INTERACTION   : Twin-Line interaction topology [0..6]
 *
 * Real-Time Safety Invariants:
 *   - Zero heap allocations.
 *   - Zero CRT transcendental calls (FastMath::fastCosNorm / fastPow2 / fastTanh only).
 *   - Finite output clamped to [-4.0f, 4.0f].
 */
class CosmoPhase final : public SoundEngine8 {
public:
    enum ParamIndex : int {
        kWave1 = 0,
        kPd1 = 1,
        kWave2 = 2,
        kPd2 = 3,
        kOctave = 4,
        kDetune2 = 5,
        kAmpMix = 6,
        kInteraction = 7
    };

    enum Waveform : int {
        kSaw = 0,
        kSquare = 1,
        kPulse = 2,
        kDoubleSine = 3,
        kSawPulse = 4,
        kReso1_SawSync = 5,
        kReso2_TriSync = 6,
        kReso3_TrapSync = 7
    };

    enum InteractionMode : int {
        kModeNormal = 0,
        kModeRingMod = 1,
        kModePhaseDistort = 2,
        kModeCrossMod = 3,
        kModeHardSync = 4,
        kModeLogicXOR = 5,
        kModeMinMaxWavefold = 6
    };

    CosmoPhase() noexcept {
        setParam8(kWave1, 0.0f);        // Saw
        setParam8(kPd1, 0.65f);         // Crisp CZ brass/bass DCW
        setParam8(kWave2, 0.72f);       // Reso1_SawSync
        setParam8(kPd2, 0.55f);         // Resonant peak
        setParam8(kOctave, 0.5f);       // 0 octaves
        setParam8(kDetune2, 0.166667f); // 0 octaves (since -1..+5 maps 0 at 1/6)
        setParam8(kAmpMix, 0.5f);       // Equal balance
        setParam8(kInteraction, 0.0f);  // Normal L1 + L2
        reset();
    }

    ~CosmoPhase() override = default;

    void reset() noexcept override {
        phase1_ = 0.0f;
        phase2_ = 0.0f;
        prevL1_ = 0.0f;
        prevL2_ = 0.0f;
        dcL_.reset();
        dcR_.reset();
    }

protected:
    void onPrepare(float sampleRate) noexcept override {
        dcL_.init(sampleRate);
        dcR_.init(sampleRate);
    }

    void onTrigger(float /*velocity*/) noexcept override {
        // Authentic CZ phase-locked attack: both lines start at zero crossing (cos(0) inverted -> -1)
        phase1_ = 0.0f;
        phase2_ = 0.0f;
    }

public:
    /**
     * Dual-slope piecewise linear phase distortion transfer function phi' = distortPhase(phi, dcw).
     * Maps normalized phase phi in [0, 1) to distorted phase in [0, 1) with inflection point d.
     */
    [[nodiscard]] static inline float distortPhase(float phi, float dcw) noexcept {
        const float p = PolyBLEP::wrapPhase(phi);
        const float d = std::clamp(0.5f * (1.0f - 0.96f * std::clamp(dcw, 0.0f, 1.0f)), 0.02f, 0.5f);
        if (p < d) {
            return 0.5f * (p / d);
        }
        return 0.5f + 0.5f * ((p - d) / (1.0f - d));
    }

    /**
     * Evaluate one of the 8 Casio CZ Phase Distortion waveforms at normalized phase [0..1) and DCW [0..1].
     * Uses -FastMath::fastCosNorm(phi') so phi'=0 starts at -1, peaking at +1 at phi'=0.5, returning to -1 at 1.0.
     */
    [[nodiscard]] static inline float evalCZWave(int waveIdx, float phi, float dcw, float dt) noexcept {
        const int w = std::clamp(waveIdx, 0, 7);
        const float p = PolyBLEP::wrapPhase(phi);
        // Dynamic Nyquist anti-aliasing protection: gentle roll-off of extreme DCW at high fundamental frequencies
        const float maxSafeDcw = std::clamp(1.0f - 1.5f * dt, 0.1f, 1.0f);
        const float cDcw = std::clamp(dcw, 0.0f, 1.0f) * maxSafeDcw;

        switch (w) {
            case kSaw: {
                const float pd = distortPhase(p, cDcw);
                return -FastMath::fastCosNorm(pd);
            }

            case kSquare: {
                // Half-cycle dual plateau phase distortion for hollow CZ square
                const float half = (p < 0.5f) ? (p * 2.0f) : ((p - 0.5f) * 2.0f);
                const float d = std::clamp(0.5f * (1.0f - 0.94f * cDcw), 0.03f, 0.5f);
                float subPhi = 0.0f;
                if (half < d) {
                    subPhi = 0.5f * (half / d);
                } else {
                    subPhi = 0.5f;
                }
                const float fullPhi = (p < 0.5f) ? subPhi : (0.5f + subPhi);
                return -FastMath::fastCosNorm(fullPhi);
            }

            case kPulse: {
                // Narrow pulse concentration around phase 0..wDuty
                const float width = std::clamp(1.0f - 0.88f * cDcw, 0.08f, 1.0f);
                if (p < width) {
                    return -FastMath::fastCosNorm(p / width);
                }
                return -1.0f;
            }

            case kDoubleSine: {
                // Warps 1-cycle cosine into 2-cycle double-sine harmonic burst
                const float pd = distortPhase(p, cDcw);
                const float twoCycle = PolyBLEP::wrapPhase(pd * (1.0f + cDcw));
                return -FastMath::fastCosNorm(twoCycle);
            }

            case kSawPulse: {
                // First half is distorted sawtooth, second half is compressed pulse
                if (p < 0.5f) {
                    const float pd = distortPhase(p * 2.0f, cDcw);
                    return -FastMath::fastCosNorm(pd * 0.5f);
                }
                const float p2 = (p - 0.5f) * 2.0f;
                const float width = std::clamp(1.0f - 0.85f * cDcw, 0.10f, 1.0f);
                if (p2 < width) {
                    return -FastMath::fastCosNorm(0.5f + 0.5f * (p2 / width));
                }
                return -1.0f;
            }

            case kReso1_SawSync:
            case kReso2_TriSync:
            case kReso3_TrapSync:
            default: {
                // Casio CZ Windowed Resonance (Reso 1..3):
                // Resonant frequency multiplier 1x .. 16x controlled by DCW, capped below Nyquist
                const float maxMult = std::clamp(0.42f / std::max(dt, 1e-5f), 1.0f, 16.0f);
                const float resoMult = 1.0f + cDcw * std::min(15.0f, maxMult - 1.0f);
                const float carrierPhase = PolyBLEP::wrapPhase(p * resoMult);
                // Carrier starts at 0 at p=0 (1 - cos(2*pi*carrierPhase)) * 0.5 or bipolar sine
                const float carrier = -FastMath::fastCosNorm(carrierPhase);

                float window = 1.0f - p; // kReso1_SawSync: Falling linear sawtooth envelope
                if (w == kReso2_TriSync) {
                    window = (p < 0.5f) ? (2.0f * p) : (2.0f * (1.0f - p));
                } else if (w == kReso3_TrapSync) {
                    window = (p < 0.5f) ? 1.0f : (2.0f * (1.0f - p));
                }
                return carrier * window;
            }
        }
    }

    void renderSampleStereo(float baseFreqHz, float& outL, float& outR) noexcept override {
        // 1. Decode parameters
        const int wave1 = std::clamp(static_cast<int>(getParam(kWave1) * 7.99f), 0, 7);
        const float pd1 = std::clamp(getParam(kPd1), 0.0f, 1.0f);
        const int wave2 = std::clamp(static_cast<int>(getParam(kWave2) * 7.99f), 0, 7);
        const float pd2 = std::clamp(getParam(kPd2), 0.0f, 1.0f);
        const int octOffset = quantizeOctave4(getParam(kOctave));
        // Knob 5: DETUNE 2 in [-1.0 .. +5.0] octaves
        const float detune2Oct = -1.0f + getParam(kDetune2) * 6.0f;
        const float ampMix = std::clamp(getParam(kAmpMix), 0.0f, 1.0f);
        const int mode = std::clamp(static_cast<int>(getParam(kInteraction) * 6.99f), 0, 6);

        // 2. Compute Line 1 & Line 2 frequencies
        const float freq1Hz = std::clamp(baseFreqHz * FastMath::fastPow2(static_cast<float>(octOffset)),
                                         8.0f, sampleRate_ * 0.47f);
        const float freq2Hz = std::clamp(freq1Hz * FastMath::fastPow2(detune2Oct),
                                         8.0f, sampleRate_ * 0.47f);

        const float dt1 = freq1Hz * invSr_;
        const float dt2 = freq2Hz * invSr_;

        // 3. Advance master phases & detect Line 1 wrap for HardSync mode
        const float nextP1 = phase1_ + dt1;
        const bool line1Wrapped = (nextP1 >= 1.0f);
        phase1_ = PolyBLEP::wrapPhase(nextP1);
        phase2_ = PolyBLEP::wrapPhase(phase2_ + dt2);

        if (mode == kModeHardSync && line1Wrapped) {
            // Sub-sample fractional reset of Line 2 phase
            const float frac = (nextP1 - 1.0f) / std::max(dt1, 1e-6f);
            phase2_ = PolyBLEP::wrapPhase(frac * dt2);
        }

        // 4. Evaluate Line 1 & Line 2 with interaction-specific phase offsets
        float effPhase1 = phase1_;
        float effPhase2 = phase2_;

        if (mode == kModeCrossMod) {
            const float xmodDepth = 0.12f + 0.38f * pd1;
            effPhase1 = PolyBLEP::wrapPhase(phase1_ + prevL2_ * xmodDepth);
        }

        const float l1 = evalCZWave(wave1, effPhase1, pd1, dt1);

        if (mode == kModePhaseDistort) {
            // Line 1 directly modulates Line 2 phase prior to Line 2 PD transfer
            const float pdModDepth = 0.20f + 0.65f * pd1;
            effPhase2 = PolyBLEP::wrapPhase(phase2_ + l1 * pdModDepth);
        } else if (mode == kModeCrossMod) {
            const float xmodDepth2 = 0.15f + 0.45f * pd2;
            effPhase2 = PolyBLEP::wrapPhase(phase2_ + l1 * xmodDepth2);
        }

        const float l2 = evalCZWave(wave2, effPhase2, pd2, dt2);
        prevL1_ = l1;
        prevL2_ = l2;

        // 5. Combine Line 1 & Line 2 via the selected Interaction Mode and AMP MIX
        const float g1 = 1.0f - ampMix;
        const float g2 = ampMix;
        const float linearMix = g1 * l1 + g2 * l2;

        float sigL = linearMix;
        float sigR = linearMix;

        switch (mode) {
            case kModeNormal: {
                // Slight stereo pan spread between Line 1 and Line 2 for rich twin-line width
                sigL = (g1 * 1.08f) * l1 + (g2 * 0.92f) * l2;
                sigR = (g1 * 0.92f) * l1 + (g2 * 1.08f) * l2;
                break;
            }

            case kModeRingMod: {
                // Casio CZ Ring Modulation: blends Line 1 with L1 * L2 balanced by AMP MIX
                const float ring = l1 * l2;
                sigL = g1 * l1 + g2 * ring;
                sigR = g1 * l2 + g2 * ring;
                break;
            }

            case kModePhaseDistort: {
                sigL = g1 * l1 + g2 * l2;
                sigR = g1 * l1 + g2 * evalCZWave(wave2, PolyBLEP::wrapPhase(effPhase2 + 0.015f), pd2, dt2);
                break;
            }

            case kModeCrossMod: {
                sigL = g1 * l1 + g2 * l2;
                sigR = g1 * l2 + g2 * l1;
                break;
            }

            case kModeHardSync: {
                // In HardSync mode, AMP MIX blends master Line 1 with synced slave Line 2
                sigL = g1 * l1 + g2 * l2;
                sigR = sigL;
                break;
            }

            case kModeLogicXOR: {
                // Smooth + comparative digital XOR emulation between Line 1 and Line 2
                const float s1 = (l1 >= 0.0f) ? 1.0f : -1.0f;
                const float s2 = (l2 >= 0.0f) ? 1.0f : -1.0f;
                const float hardXor = -(s1 * s2);
                const float softXor = l1 + l2 - 1.5f * (l1 * l2);
                const float xorSig = 0.55f * softXor + 0.45f * hardXor;
                sigL = g1 * l1 + g2 * xorSig;
                sigR = g1 * l2 + g2 * xorSig;
                break;
            }

            case kModeMinMaxWavefold:
            default: {
                // Non-linear Min/Max reflection wavefolder driven by PD1 + PD2
                const float minV = std::min(l1, l2);
                const float maxV = std::max(l1, l2);
                const float drive = 1.2f + 2.5f * (0.5f * (pd1 + pd2));
                const float foldedL = FastMath::fastSinNorm((g1 * minV + g2 * maxV) * drive * 0.35f);
                const float foldedR = FastMath::fastSinNorm((g1 * maxV + g2 * minV) * drive * 0.35f);
                sigL = foldedL;
                sigR = foldedR;
                break;
            }
        }

        // 6. Velocity scaling, gentle analog output stage warmth, DC block, and safety clamp
        const float velGain = 0.25f + 0.75f * velocity_;
        outL = dcL_.process(FastMath::fastTanh(sigL * velGain * 0.95f));
        outR = dcR_.process(FastMath::fastTanh(sigR * velGain * 0.95f));

        outL = std::clamp(std::isfinite(outL) ? outL : 0.0f, -4.0f, 4.0f);
        outR = std::clamp(std::isfinite(outR) ? outR : 0.0f, -4.0f, 4.0f);
    }

private:
    float phase1_ = 0.0f;
    float phase2_ = 0.0f;
    float prevL1_ = 0.0f;
    float prevL2_ = 0.0f;
    DSPCommon::DCBlocker dcL_;
    DSPCommon::DCBlocker dcR_;
};



using KlangCZPhaseX = CosmoPhase;
using KlangCZ = CosmoPhase;

} // namespace TbdAudio::Oscillators
