#pragma once

#include "dsp/oscillators/PolyBLEP.h"
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace CrackedChimes::DSP {

/**
 * @brief Doepfer A-196 Style Phase-Locked Loop Sub-Oscillator with Instability & XOR Phase Comparator.
 */
class PllSubOsc {
public:
    enum class Source : uint8_t {
        OSC_1 = 0,
        OSC_1_AND_2_XOR = 1,
        OSC_2 = 2
    };

    void prepare(double sampleRate) noexcept {
        sampleRate_ = (sampleRate > 1000.0) ? sampleRate : 44100.0;
        invSr_ = 1.0 / sampleRate_;
        reset();
    }

    void reset() noexcept {
        pllPhase_ = 0.0f;
        trackedFreqHz_ = 65.4f;
        loopFilter_ = 0.0f;
        jitterLfsr_ = 0x9182u;
    }

    /**
     * @brief Process one sample of the PLL Sub-Oscillator.
     * @param osc1Square Clean square wave state from Osc 1.
     * @param osc2Square Clean square wave state from Osc 2.
     * @param freq1Hz Osc 1 fundamental frequency (Hz).
     * @param freq2Hz Osc 2 fundamental frequency (Hz, with offset).
     * @param source Tracking source (Osc 1, Osc 1&2 XOR, Osc 2).
     * @param octaveOffset Octave shift (+1, 0, -1, -2).
     * @param shapeIdx One of the 14 Alpha Juno shapes (0..13).
     * @param instability Doepfer A-196 tracking lag, phase jitter, and cycle-slip depth [0.0, 1.0].
     * @param pwm Pulse width modulation [-1.0, 1.0].
     * @return Generated sub-oscillator sample.
     */
    [[nodiscard]] float process(
        bool osc1Square,
        bool osc2Square,
        float freq1Hz,
        float freq2Hz,
        Source source,
        int octaveOffset,
        int shapeIdx,
        float instability,
        float pwm) noexcept
    {
        const float nyquist = static_cast<float>(sampleRate_ * 0.48);
        const float octMult = std::pow(2.0f, static_cast<float>(octaveOffset));

        // 1. Calculate Target Frequency Based on Source
        float nominalFreq = freq1Hz * octMult;
        bool compSig = osc1Square;

        switch (source) {
            case Source::OSC_1:
                nominalFreq = freq1Hz * octMult;
                compSig = osc1Square;
                break;

            case Source::OSC_2:
                nominalFreq = freq2Hz * octMult;
                compSig = osc2Square;
                break;

            case Source::OSC_1_AND_2_XOR: {
                compSig = (osc1Square ^ osc2Square);
                // When Osc 1 and Osc 2 are at different pitches, XOR composite contains
                // sum and difference components. Track the active composite rate:
                const float diff = std::abs(freq1Hz - freq2Hz);
                if (diff > 12.0f) {
                    nominalFreq = diff * octMult;
                } else {
                    nominalFreq = ((freq1Hz + freq2Hz) * 0.5f) * octMult;
                }
                break;
            }
        }

        nominalFreq = std::clamp(nominalFreq, 5.0f, nyquist * 0.85f);

        // 2. Phase Comparator (Type 1 XOR logic vs Sub VCO square output)
        const bool subSquare = (pllPhase_ < 0.5f);
        const float phaseError = (compSig ^ subSquare) ? 1.0f : -1.0f;

        // 3. Loop Filter with variable time constant & instability
        const float inst = std::clamp(instability, 0.0f, 1.0f);
        // At 0 instability: instantaneous loop response; at high instability: heavy slew lag
        const float filterCoeff = std::clamp(0.005f + (1.0f - inst) * 0.35f, 0.002f, 0.45f);
        loopFilter_ += filterCoeff * (phaseError - loopFilter_);

        // 4. Tracking Slew & Instability Wander
        jitterLfsr_ = (jitterLfsr_ >> 1) ^ (-(static_cast<int32_t>(jitterLfsr_ & 1u)) & 0xB400u);
        const float jitterNoise = (static_cast<float>(jitterLfsr_ & 0xFFFF) / 32768.0f - 1.0f);

        const float trackingSlew = 1.0f - std::exp(-filterCoeff * (35.0f + 50.0f * (1.0f - inst)));
        trackedFreqHz_ += trackingSlew * (nominalFreq - trackedFreqHz_);

        // Frequency modulation from phase error + analog slew jitter
        const float errorMod = loopFilter_ * (inst * 150.0f + 25.0f);
        const float jitterMod = (inst * inst) * 0.35f * jitterNoise * trackedFreqHz_;
        const float effectiveFreq = std::clamp(trackedFreqHz_ + errorMod + jitterMod, 5.0f, nyquist);

        const float dt = effectiveFreq * static_cast<float>(invSr_);

        // 5. Accumulate Phase with Cycle-Slipping Chaos
        pllPhase_ += dt;
        if (pllPhase_ >= 1.0f) {
            pllPhase_ -= 1.0f;
            // High instability occasionally causes cycle slip (sub drops a fraction of a cycle)
            if (inst > 0.60f && (jitterLfsr_ & 0x7u) == 0) {
                pllPhase_ += 0.20f * (inst - 0.60f);
                if (pllPhase_ >= 1.0f) pllPhase_ -= 1.0f;
            }
        }

        // 6. Generate Selected Alpha Shape
        const int shape = std::clamp(shapeIdx, 0, 13);
        return TbdAudio::Oscillators::PolyBLEP::alphaWave14(shape, pllPhase_, dt, pwm);
    }

private:
    double sampleRate_ = 44100.0;
    double invSr_ = 1.0 / 44100.0;

    float pllPhase_ = 0.0f;
    float trackedFreqHz_ = 65.4f;
    float loopFilter_ = 0.0f;
    uint32_t jitterLfsr_ = 0x9182u;
};

} // namespace CrackedChimes::DSP
