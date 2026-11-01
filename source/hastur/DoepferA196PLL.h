#pragma once

#include <cmath>
#include <algorithm>
#include <cstdint>

namespace CrackedChimes::DSP {

/**
 * @brief Complete Eurorack Doepfer A-196 Phase-Locked Loop (PLL) Subsystem with 8x Multisampling.
 * 
 * Features:
 * - 8x Sub-Sample Oversampling for ultra-low jitter, sub-sample edge detection, and audio-rate PLL FM.
 * - Internal PLL VCO with 3 frequency ranges (Low: 2Hz-1kHz, Mid: 20Hz-10kHz, Hi: 200Hz-100kHz).
 * - 3 Phase Comparators (PC1: XOR, PC2: Edge-Triggered RS PFD with Lock LED, PC3: CD4046 Memory Network).
 * - Clock Divider (1..8) in the feedback loop before PC Input 1 (Frequency Multiplier mode).
 * - Multi-source Signal input (Osc 1, Osc 1 APF, Osc 2, Osc 2 APF, Osc 3, Osc 3 APF, PLL VCO, XOR 1/2, XOR 1/2/3).
 * - Multi-source Lock input (PLL VCO, Osc 3, XOR 3/V).
 * - Slew (Loop Filter cutoff 0.5 Hz to 2000 Hz).
 */
class DoepferA196PLL {
public:
    static constexpr int OVERSAMPLE_FACTOR = 8;

    enum class Range : uint8_t {
        LOW = 0, // 2 Hz .. 1 kHz
        MID = 1, // 20 Hz .. 10 kHz
        HI  = 2  // 200 Hz .. 100 kHz
    };

    enum class CompType : uint8_t {
        PC1_XOR    = 0, // XOR gate
        PC2_RS_PFD = 1, // Phase-Frequency Detector (Flip-Flop)
        PC3_MEMORY = 2  // CD4046 Digital Memory Network
    };

    enum class LockSource : uint8_t {
        PLL_VCO = 0,
        OSC_3   = 1,
        XOR_3_V = 2
    };

    enum class SignalSource : uint8_t {
        OSC_1       = 0,
        OSC_1_APF   = 1,
        OSC_2       = 2,
        OSC_2_APF   = 3,
        OSC_3       = 4,
        OSC_3_APF   = 5,
        PLL_VCO     = 6,
        XOR_1_2     = 7,
        XOR_1_2_3   = 8
    };

    void prepare(double sampleRate) noexcept {
        sampleRate_ = (sampleRate > 1000.0) ? sampleRate : 44100.0;
        invSampleRate_ = 1.0 / sampleRate_;
        subSampleRate_ = sampleRate_ * static_cast<double>(OVERSAMPLE_FACTOR);
        invSubSr_ = 1.0 / subSampleRate_;
        reset();
    }

    void reset() noexcept {
        vcoPhase_ = 0.0f;
        loopFilter_ = 0.0f;
        lockMetric_ = 0.5f;

        prevRawLock_ = false;
        prevDivLock_ = false;
        prevSqSig_ = false;

        dividerCount_ = 0;
        dividedState_ = false;

        pfdSig_ = false;
        pfdLock_ = false;
        pc3State_ = 0.0f;

        isLocked_ = false;
        vcoAudioOut_ = 0.0f;
    }

    /**
     * @brief Process one audio-rate sample using 8x sub-sample resolution.
     */
    [[nodiscard]] float process(
        float phase1Start, float dt1,
        float phase2Start, float dt2,
        float phase3Start, float dt3,
        bool osc1ApfSq, bool osc2ApfSq, bool osc3ApfSq,
        Range range,
        float vcoOffset,
        CompType compType,
        LockSource lockSource,
        SignalSource sigSource,
        int dividerRatio,
        float slew) noexcept
    {
        // 1. Determine Base Frequency Range for PLL VCO
        float minF = 20.0f;
        float maxF = 10000.0f;
        switch (range) {
            case Range::LOW:
                minF = 2.0f;
                maxF = 1000.0f;
                break;
            case Range::MID:
                minF = 20.0f;
                maxF = 10000.0f;
                break;
            case Range::HI:
                minF = 200.0f;
                maxF = 100000.0f;
                break;
        }

        const float f0 = minF * std::pow(maxF / minF, std::clamp(vcoOffset, 0.0f, 1.0f));
        const int divRatio = std::clamp(dividerRatio, 1, 8);

        // Loop Filter (Slew) Cutoff mapped exponentially (0.5 Hz to 2000 Hz)
        const float slewCutoff = 0.5f * std::pow(4000.0f, std::clamp(slew, 0.0f, 1.0f));
        const float slewW = static_cast<float>(2.0 * 3.14159265358979323846 * slewCutoff * invSubSr_);
        const float slewG = std::clamp(slewW, 0.00001f, 0.45f);

        float vcoAudioAccum = 0.0f;

        // 2. Execute 8x Multisampling Loop
        for (int sub = 0; sub < OVERSAMPLE_FACTOR; ++sub) {
            const float frac = (static_cast<float>(sub) + 0.5f) * (1.0f / static_cast<float>(OVERSAMPLE_FACTOR));

            // High-precision sub-sample phase interpolation for incoming oscillators
            float p1 = phase1Start + dt1 * frac;
            p1 -= static_cast<float>(static_cast<int>(p1));
            if (p1 < 0.0f) p1 += 1.0f;

            float p2 = phase2Start + dt2 * frac;
            p2 -= static_cast<float>(static_cast<int>(p2));
            if (p2 < 0.0f) p2 += 1.0f;

            float p3 = phase3Start + dt3 * frac;
            p3 -= static_cast<float>(static_cast<int>(p3));
            if (p3 < 0.0f) p3 += 1.0f;

            const bool sq1 = (p1 < 0.5f);
            const bool sq2 = (p2 < 0.5f);
            const bool sq3 = (p3 < 0.5f);

            // Compute PLL VCO Sub-Sample Frequency modulated by Loop Filter
            const float vcoFreq = std::clamp(
                f0 * std::pow(2.0f, loopFilter_ * 4.0f),
                1.0f,
                static_cast<float>(subSampleRate_ * 0.48));

            const float dtVco = vcoFreq * static_cast<float>(invSubSr_);
            vcoPhase_ += dtVco;
            if (vcoPhase_ >= 1.0f) vcoPhase_ -= 1.0f;
            const bool sqVco = (vcoPhase_ < 0.5f);

            // Select PC Input 1 (Feedback/Lock)
            bool rawLock = sqVco;
            switch (lockSource) {
                case LockSource::PLL_VCO: rawLock = sqVco; break;
                case LockSource::OSC_3:   rawLock = sq3;   break;
                case LockSource::XOR_3_V: rawLock = sq3 ^ sqVco; break;
            }

            // Apply Clock Divider before PC Input 1
            bool divLock = rawLock;
            if (divRatio > 1) {
                const bool rawRise = (rawLock && !prevRawLock_);
                if (rawRise) {
                    dividerCount_++;
                    if (dividerCount_ >= divRatio) {
                        dividerCount_ = 0;
                        dividedState_ = !dividedState_;
                    }
                }
                divLock = dividedState_;
            }
            prevRawLock_ = rawLock;

            // Select PC Input 2 (Signal Reference)
            bool sqSig = sq1;
            switch (sigSource) {
                case SignalSource::OSC_1:     sqSig = sq1; break;
                case SignalSource::OSC_1_APF: sqSig = osc1ApfSq; break;
                case SignalSource::OSC_2:     sqSig = sq2; break;
                case SignalSource::OSC_2_APF: sqSig = osc2ApfSq; break;
                case SignalSource::OSC_3:     sqSig = sq3; break;
                case SignalSource::OSC_3_APF: sqSig = osc3ApfSq; break;
                case SignalSource::PLL_VCO:   sqSig = sqVco; break;
                case SignalSource::XOR_1_2:   sqSig = sq1 ^ sq2; break;
                case SignalSource::XOR_1_2_3: sqSig = sq1 ^ sq2 ^ sq3; break;
            }

            // Edge detection for PC2 and PC3
            const bool sigRise  = (sqSig && !prevSqSig_);
            const bool lockRise = (divLock && !prevDivLock_);
            prevSqSig_  = sqSig;
            prevDivLock_ = divLock;

            // PC 1: XOR
            const float pc1 = (divLock ^ sqSig) ? 1.0f : -1.0f;

            // PC 2: Edge-Triggered Phase-Frequency Detector (RS Flip-Flop)
            if (sigRise)  pfdSig_  = true;
            if (lockRise) pfdLock_ = true;

            float pc2 = 0.0f;
            if (pfdSig_ && !pfdLock_) {
                pc2 = 1.0f; // Signal leading, pump up
            } else if (pfdLock_ && !pfdSig_) {
                pc2 = -1.0f; // Lock leading, pump down
            } else if (pfdSig_ && pfdLock_) {
                // Both arrived: reset
                pfdSig_  = false;
                pfdLock_ = false;
                pc2 = 0.0f;
            }
            lockMetric_ += 0.0008f * (std::abs(pc2) - lockMetric_);

            // PC 3: CD4046 Digital Memory Network
            if (sigRise)  pc3State_ = 1.0f;
            if (lockRise) pc3State_ = -1.0f;
            const float pc3 = pc3State_;

            // Select active comparator error
            float rawError = pc1;
            switch (compType) {
                case CompType::PC1_XOR:    rawError = pc1; break;
                case CompType::PC2_RS_PFD: rawError = pc2; break;
                case CompType::PC3_MEMORY: rawError = pc3; break;
            }

            // Loop Filter (Slew) integration
            loopFilter_ += slewG * (rawError - loopFilter_);

            // Accumulate band-limited or raw sub-sample audio
            const float vcoSub = sqVco ? 1.0f : -1.0f;
            vcoAudioAccum += vcoSub;
        }

        // 3. Post-Oversampling Decimation & Status
        vcoAudioOut_ = vcoAudioAccum * (1.0f / static_cast<float>(OVERSAMPLE_FACTOR));
        isLocked_ = (lockMetric_ < 0.22f);

        // Denormal protection
        if (std::abs(loopFilter_) < 1e-9f) loopFilter_ = 0.0f;

        return loopFilter_;
    }

    [[nodiscard]] float getLoopFilter() const noexcept { return loopFilter_; }
    [[nodiscard]] float getVcoAudio() const noexcept { return vcoAudioOut_; }
    [[nodiscard]] bool getVcoSquare() const noexcept { return vcoPhase_ < 0.5f; }
    [[nodiscard]] bool isLocked() const noexcept { return isLocked_; }

private:
    double sampleRate_ = 44100.0;
    double invSampleRate_ = 1.0 / 44100.0;
    double subSampleRate_ = 352800.0;
    double invSubSr_ = 1.0 / 352800.0;

    float vcoPhase_ = 0.0f;
    float loopFilter_ = 0.0f;
    float lockMetric_ = 0.5f;
    bool isLocked_ = false;
    float vcoAudioOut_ = 0.0f;

    bool prevRawLock_ = false;
    bool prevDivLock_ = false;
    bool prevSqSig_   = false;

    int dividerCount_ = 0;
    bool dividedState_ = false;

    bool pfdSig_  = false;
    bool pfdLock_ = false;
    float pc3State_ = 0.0f;
};

} // namespace CrackedChimes::DSP
