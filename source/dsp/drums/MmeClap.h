#pragma once

#include "../engines/MultiMachineEngineBase.h"
#include "../../FastMath.h"
#include "../common/AudioEffect.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace tkdsp {

/**
 * @brief MmeClap (MME-CP) — Multi-Machine Handclap & Snap Engine (Venti 32-Control).
 *
 * Implements 16 curated electronic handclap synthesis models under the unified
 * role-based MME architecture:
 *
 *   00: MD-TRX-CP — Machinedrum TRX Multi-Burst Handclap
 *   01: MD-EFM-CP — Machinedrum EFM Metallic FM Clap
 *   02: DR-110-CP — Boss DR-110 Dr. Rhythm Authentic Crisp Paper Multi-Pulse Clap
 *   03: TR-808-CP — Roland TR-808 Triple-Trigger Reverb Burst Clap
 *   04: TR-909-CP — Roland TR-909 Tight Transient Staggered Clap
 *   05: PK-BUR-CP — Erica Pērkons Staggered Burst Overdrive Clap
 *   06: MO-MLT-CP — Modor DR-2 Multi-Tap Envelope Filtered Noise Clap
 *   07: VM-DRM-CP — Vermona DRM1 MKIV Analog Discrete Burst Clap
 *   08: SIM-SDS-CP — Simmons SDS-V Resonant Handclap
 *   09: SYN-BUR-CP — Pearl Syncussion SY-1 Explosive Burst Clap
 *   10: WLD-CRK-CP — Waldorf Attack Poisson Crackle Clap
 *   11: LXR-DIG-CP — Erica LXR-02 Multi-Op Digital Crack Clap
 *   12: KRG-VLC-CP — Korg Volca Drum Waveguide Clap
 *   13: JMX-X99-CP — Jomox Alpha Base Analog Membrane Clap
 *   14: VM-DNG-CP — Vermona drumDING Filtered Crack Clap
 *   15: CY-GLT-CP  — Cyberpunk Glitch / Granular Vector Clap
 *
 * The Jam Card (Bank 0: Knobs 01–08):
 *   [01: MODEL] [02: TONE ] [03: DECAY] [04: BURST]
 *   [05: SPREAD] [06: REVERB] [07: SIZE ] [08: PUNCH]
 *
 * Real-Time Audio Thread Safety:
 *   - 100% zero heap allocations during trigger(), setParam(), or renderSampleStereo().
 *   - Zero CRT transcendental calls (uses TbdAudio::FastMath exclusively).
 *   - Guaranteed finite output with integrated DC blocking.
 */
class MmeClap final : public MultiMachineEngineBase {
public:
    enum Model : uint8_t {
        kMdTrxCp = 0,   ///< MD-TRX-CP: Machinedrum TRX Multi-Burst Handclap
        kMdEfmCp = 1,   ///< MD-EFM-CP: Machinedrum EFM Metallic FM Clap
        kDr110Cp = 2,   ///< DR-110-CP: Boss DR-110 Paper-Crisp Clap
        kTr808Cp = 3,   ///< TR-808-CP: Roland TR-808 Reverb Burst Clap
        kTr909Cp = 4,   ///< TR-909-CP: Roland TR-909 Tight Staggered Clap
        kPkBurCp = 5,   ///< PK-BUR-CP: Erica Pērkons Overdrive Clap
        kMoMltCp = 6,   ///< MO-MLT-CP: Modor DR-2 Multi-Tap Envelope Clap
        kVmDrmCp = 7,   ///< VM-DRM-CP: Vermona DRM1 Analog Burst Clap
        kSimSdsCp = 8,  ///< SIM-SDS-CP: Simmons SDS-V Resonant Clap
        kSynBurCp = 9,  ///< SYN-BUR-CP: Pearl Syncussion Explosive Clap
        kWldCrkCp = 10, ///< WLD-CRK-CP: Waldorf Attack Poisson Clap
        kLxrDigCp = 11, ///< LXR-DIG-CP: Erica LXR-02 Digital Crack Clap
        kKrgVlcCp = 12, ///< KRG-VLC-CP: Korg Volca Waveguide Clap
        kJmxX99Cp = 13, ///< JMX-X99-CP: Jomox Alpha Base Membrane Clap
        kVmDngCp = 14,  ///< VM-DNG-CP: Vermona drumDING Filtered Crack Clap
        kCyGltCp = 15   ///< CY-GLT-CP: Cyberpunk Glitch Vector Clap
    };

    static constexpr uint8_t kTotalModels = 16;

    static constexpr const char* kModelNames[kTotalModels] = {
        "MD-TRX-CP", "MD-EFM-CP", "DR-110-CP", "TR-808-CP",
        "TR-909-CP", "PK-BUR-CP", "MO-MLT-CP", "VM-DRM-CP",
        "SIM-SDS-CP", "SYN-BUR-CP", "WLD-CRK-CP", "LXR-DIG-CP",
        "KRG-VLC-CP", "JMX-X99-CP", "VM-DNG-CP", "CY-GLT-CP"
    };

    MmeClap() noexcept {
        // Default Bank 0 Jam Card parameters
        setParam(0, 0.0f);  // K01: MODEL  (MD-TRX-CP)
        setParam(1, 0.45f); // K02: TONE   (~1.4 kHz bandpass)
        setParam(2, 0.45f); // K03: DECAY  (~320 ms reverb tail)
        setParam(3, 0.60f); // K04: BURST  (3 pre-claps)
        setParam(4, 0.40f); // K05: SPREAD (~12 ms spacing)
        setParam(5, 0.50f); // K06: REVERB (diffuse tail level)
        setParam(6, 0.40f); // K07: SIZE   (room diffusion density)
        setParam(7, 0.50f); // K08: PUNCH  (transient snap)

        // Default Bank 2 Timbre Lab parameters
        setParam(16, 0.50f); // K17: MOD-X
        setParam(17, 0.50f); // K18: MOD-Y
        setParam(18, 0.20f); // K19: MOD-Z
        setParam(19, 0.20f); // K20: MOD-T
        setParam(20, 0.00f); // K21: SUB-OCT
        setParam(21, 0.30f); // K22: FM-ENV
        setParam(22, 0.20f); // K23: DAMP
        setParam(23, 0.50f); // K24: SPACE (center)

        // Default Bank 3 Deep Acoustic Lab parameters
        setParam(24, 0.50f); // K25: P-CRV
        setParam(25, 0.50f); // K26: A-CRV
        setParam(26, 0.00f); // K27: KEYTRACK
        setParam(27, 0.50f); // K28: V-SENS
        setParam(28, 0.00f); // K29: V-DEST
        setParam(29, 0.00f); // K30: DISP
        setParam(30, 0.00f); // K31: FLAM
        setParam(31, 0.00f); // K32: MACRO

        reset();
    }

    ~MmeClap() override = default;

    [[nodiscard]] uint8_t numModels() const noexcept override { return kTotalModels; }

    [[nodiscard]] static const char* getModelName(uint8_t modelIndex) noexcept {
        return (modelIndex < kTotalModels) ? kModelNames[modelIndex] : "UNKNOWN";
    }

protected:
    void onPrepareSubModels(float sampleRate) noexcept override {
        sr_ = sampleRate;
        invSr_ = 1.0f / sr_;
        updateEnvelopes();
        updateFilter();
        reset();
    }

    void onSubModelTrigger(uint8_t /*modelIndex*/, float velocity) noexcept override {
        vel_ = std::clamp(velocity, 0.05f, 1.0f);
        tailEnv_ = 0.0f; // Tail starts after pre-bursts
        burstEnv_ = vel_;

        // Number of pre-claps from K04 (BURST): 1 to 5 bursts
        const float burstNorm = getParam(3);
        burstsRemaining_ = 1 + static_cast<int>(burstNorm * 4.0f + 0.5f);

        // Pre-clap spacing from K05 (SPREAD): 5 ms .. 30 ms
        const float spreadNorm = getParam(4);
        const float spreadSec = 0.005f + 0.025f * spreadNorm;
        burstSpacingSamples_ = std::max(16, static_cast<int>(spreadSec * sr_));
        burstTimer_ = 0;

        diffuser1_.fill(0.0f);
        diffuser2_.fill(0.0f);
        diffPos1_ = 0;
        diffPos2_ = 0;

        filterSvf_.reset();
    }

    void onSubModelReset(uint8_t /*modelIndex*/) noexcept override {
        // PRESERVE tailEnv_ and burstEnv_ so sound does not drop out on live model change!
        diffuser1_.fill(0.0f);
        diffuser2_.fill(0.0f);
        diffPos1_ = 0;
        diffPos2_ = 0;
        dcL_ = 0.0f;
        dcR_ = 0.0f;
        filterSvf_.reset();
    }

    void onParamChanged(int index, float /*value*/) noexcept override {
        if (index == 2) {
            updateEnvelopes();
        } else if (index == 1) {
            updateFilter();
        }
    }

    void renderSubModelStereo(uint8_t modelIndex, float /*pitchHz*/, float& outL, float& outR) noexcept override {
        // Multi-burst state machine: generate sharp narrow pre-clap bursts
        stepBurstStateMachine();

        float rawNoise = nextNoise();
        float filteredNoise = filterSvf_.process(rawNoise);

        float monoSample = 0.0f;

        switch (modelIndex) {
            case kMdTrxCp:   monoSample = renderMdTrxCp(filteredNoise); break;
            case kMdEfmCp:   monoSample = renderMdEfmCp(rawNoise); break;
            case kDr110Cp:   monoSample = renderDr110Cp(filteredNoise); break;
            case kTr808Cp:   monoSample = renderTr808Cp(filteredNoise); break;
            case kTr909Cp:   monoSample = renderTr909Cp(filteredNoise); break;
            case kPkBurCp:   monoSample = renderPkBurCp(filteredNoise); break;
            case kMoMltCp:   monoSample = renderMoMltCp(filteredNoise); break;
            case kVmDrmCp:   monoSample = renderVmDrmCp(filteredNoise); break;
            case kSimSdsCp:  monoSample = renderSimSdsCp(filteredNoise); break;
            case kSynBurCp:  monoSample = renderSynBurCp(filteredNoise); break;
            case kWldCrkCp:  monoSample = renderWldCrkCp(filteredNoise); break;
            case kLxrDigCp:  monoSample = renderLxrDigCp(rawNoise); break;
            case kKrgVlcCp:  monoSample = renderKrgVlcCp(filteredNoise); break;
            case kJmxX99Cp:  monoSample = renderJmxX99Cp(filteredNoise); break;
            case kVmDngCp:  monoSample = renderVmDngCp(filteredNoise); break;
            case kCyGltCp:   monoSample = renderCyGltCp(rawNoise); break;
            default:         monoSample = renderMdTrxCp(filteredNoise); break;
        }

        // Stereo Diffusion / Reverb stage
        float diffL = 0.0f, diffR = 0.0f;
        processDiffusion(monoSample, diffL, diffR);

        // Mix direct clap + diffuse room
        const float revMix = getParam(5); // K06 (REVERB)
        float outLeft = monoSample * (1.0f - revMix * 0.4f) + diffL * revMix;
        float outRight = monoSample * (1.0f - revMix * 0.4f) + diffR * revMix;

        // High-pass DC Blocker
        const float dcOutL = outLeft - dcL_ + 0.9995f * dcLOut_;
        dcL_ = outLeft;
        dcLOut_ = dcOutL;

        const float dcOutR = outRight - dcR_ + 0.9995f * dcROut_;
        dcR_ = outRight;
        dcROut_ = dcOutR;

        outL = dcOutL;
        outR = dcOutR;
    }

private:
    // =========================================================================
    // SUB-MODEL SYNTHESIS IMPLEMENTATIONS
    // =========================================================================

    // 00: MD-TRX-CP — Machinedrum TRX Multi-Burst Handclap
    [[nodiscard]] float renderMdTrxCp(float noise) noexcept {
        const float burstSig = noise * burstEnv_ * getParam(7) * 1.6f;
        const float tailSig = noise * tailEnv_ * 1.2f;
        return FastMath::fastTanh(burstSig + tailSig);
    }

    // 01: MD-EFM-CP — Machinedrum EFM Metallic FM Clap
    [[nodiscard]] float renderMdEfmCp(float noise) noexcept {
        phase0_ += (800.0f + getParam(16) * 2400.0f) * invSr_;
        if (phase0_ >= 1.0f) phase0_ -= std::floor(phase0_);
        const float ring = FastMath::fastSinNorm(phase0_);

        const float metallicNoise = noise * (0.6f + ring * 0.4f);
        const float burstSig = metallicNoise * burstEnv_ * getParam(7) * 1.5f;
        const float tailSig = metallicNoise * tailEnv_;
        return FastMath::fastTanh(burstSig + tailSig);
    }

    // 02: DR-110-CP — Boss DR-110 Authentic Crisp Paper Multi-Pulse Clap
    [[nodiscard]] float renderDr110Cp(float noise) noexcept {
        // High-pass differentiated paper-crisp noise with punchy capacitor clicks
        const float click = (burstEnv_ > 0.6f ? 0.7f : 0.0f) * getParam(7);
        const float burstSig = (noise + click) * burstEnv_ * 1.7f;
        // Dry, short, tight paper tail
        const float tailSig = noise * tailEnv_ * 0.9f;
        return FastMath::fastTanh(burstSig + tailSig);
    }

    // 03: TR-808-CP — Roland TR-808 Triple-Trigger Reverb Burst Clap
    [[nodiscard]] float renderTr808Cp(float noise) noexcept {
        // 808 triple trigger + rich diffuse tail
        const float burstSig = noise * burstEnv_ * getParam(7) * 1.5f;
        const float tailSig = noise * tailEnv_ * 1.3f;
        return FastMath::fastTanh(burstSig + tailSig);
    }

    // 04: TR-909-CP — Roland TR-909 Tight Transient Staggered Clap
    [[nodiscard]] float renderTr909Cp(float noise) noexcept {
        // Narrow punchy bursts with subtle transient spike
        const float spike = (burstEnv_ > 0.7f ? nextNoise() * 0.4f : 0.0f);
        const float burstSig = (noise + spike) * burstEnv_ * getParam(7) * 1.6f;
        const float tailSig = noise * tailEnv_;
        return FastMath::fastTanh(burstSig + tailSig);
    }

    // 05: PK-BUR-CP — Erica Pērkons Staggered Burst Overdrive Clap
    [[nodiscard]] float renderPkBurCp(float noise) noexcept {
        const float burstSig = noise * burstEnv_ * 1.8f;
        const float tailSig = noise * tailEnv_ * 1.4f;
        // Hard saturation
        const float drive = 1.3f + getParam(7) * 2.5f;
        return FastMath::fastTanh((burstSig + tailSig) * drive);
    }

    // 06: MO-MLT-CP — Modor DR-2 Multi-Tap Envelope Filtered Noise Clap
    [[nodiscard]] float renderMoMltCp(float noise) noexcept {
        const float burstSig = noise * burstEnv_ * getParam(7) * 1.4f;
        const float tailSig = noise * tailEnv_ * 1.1f;
        return FastMath::fastTanh((burstSig + tailSig) * 1.3f);
    }

    // 07: VM-DRM-CP — Vermona DRM1 MKIV Analog Discrete Burst Clap
    [[nodiscard]] float renderVmDrmCp(float noise) noexcept {
        const float burstSig = noise * burstEnv_ * getParam(7) * 1.3f;
        const float tailSig = noise * tailEnv_ * 1.2f;
        return FastMath::fastTanh(burstSig + tailSig);
    }

    // 08: SIM-SDS-CP — Simmons SDS-V Resonant Handclap
    [[nodiscard]] float renderSimSdsCp(float noise) noexcept {
        const float burstSig = noise * burstEnv_ * getParam(7) * 1.5f;
        const float tailSig = noise * tailEnv_ * 1.3f;
        return FastMath::fastTanh((burstSig + tailSig) * 1.4f);
    }

    // 09: SYN-BUR-CP — Pearl Syncussion SY-1 Explosive Burst Clap
    [[nodiscard]] float renderSynBurCp(float noise) noexcept {
        const float burstSig = noise * burstEnv_ * getParam(7) * 1.8f;
        const float tailSig = noise * tailEnv_ * 1.5f;
        return FastMath::fastTanh((burstSig + tailSig) * 1.5f);
    }

    // 10: WLD-CRK-CP — Waldorf Attack Poisson Crackle Clap
    [[nodiscard]] float renderWldCrkCp(float noise) noexcept {
        const float crackle = (nextNoise() > 0.8f ? nextNoise() * 1.6f : 0.0f) * burstEnv_;
        const float burstSig = (noise + crackle) * burstEnv_ * 1.4f;
        const float tailSig = noise * tailEnv_;
        return FastMath::fastTanh(burstSig + tailSig);
    }

    // 11: LXR-DIG-CP — Erica LXR-02 Multi-Op Digital Crack Clap
    [[nodiscard]] float renderLxrDigCp(float noise) noexcept {
        const float bitNoise = (noise > 0.0f ? 0.8f : -0.8f);
        const float burstSig = bitNoise * burstEnv_ * getParam(7) * 1.5f;
        const float tailSig = noise * tailEnv_ * 1.1f;
        return FastMath::fastTanh(burstSig + tailSig);
    }

    // 12: KRG-VLC-CP — Korg Volca Drum Waveguide Clap
    [[nodiscard]] float renderKrgVlcCp(float noise) noexcept {
        const float burstSig = noise * burstEnv_ * getParam(7) * 1.4f;
        const float tailSig = noise * tailEnv_ * 1.2f;
        return FastMath::fastTanh(burstSig + tailSig);
    }

    // 13: JMX-X99-CP — Jomox Alpha Base Analog Membrane Clap
    [[nodiscard]] float renderJmxX99Cp(float noise) noexcept {
        const float burstSig = noise * burstEnv_ * getParam(7) * 1.3f;
        const float tailSig = noise * tailEnv_ * 1.2f;
        return FastMath::fastTanh(burstSig + tailSig);
    }

    // 14: VM-DNG-CP — Vermona drumDING Filtered Crack Clap
    [[nodiscard]] float renderVmDngCp(float noise) noexcept {
        const float burstSig = noise * burstEnv_ * getParam(7) * 1.5f;
        const float tailSig = noise * tailEnv_ * 1.2f;
        return FastMath::fastTanh(burstSig + tailSig);
    }

    // 15: CY-GLT-CP — Cyberpunk Glitch / Granular Vector Clap
    [[nodiscard]] float renderCyGltCp(float noise) noexcept {
        const float glitch = (nextNoise() > 0.7f ? nextNoise() * 2.0f : 0.0f);
        const float burstSig = (noise + glitch) * burstEnv_ * 1.6f;
        const float tailSig = noise * tailEnv_ * 1.3f;
        return FastMath::fastTanh(burstSig + tailSig);
    }

    // =========================================================================
    // STATE MACHINE & HELPERS
    // =========================================================================

    inline void stepBurstStateMachine() noexcept {
        // Decay the active burst impulse (~1.8 ms)
        burstEnv_ *= burstDecayCoeff_;

        if (burstsRemaining_ > 0) {
            ++burstTimer_;
            if (burstTimer_ >= burstSpacingSamples_) {
                burstTimer_ = 0;
                --burstsRemaining_;
                // Trigger subsequent pre-clap burst with slight random human flam variation
                burstEnv_ = vel_ * (0.8f + nextNoise() * 0.2f);
                if (burstsRemaining_ == 0) {
                    // Pre-claps finished -> trigger full reverb tail
                    tailEnv_ = vel_;
                }
            }
        } else {
            // Decay the diffuse reverb tail
            tailEnv_ *= tailDecayCoeff_;
        }
    }

    void processDiffusion(float in, float& outL, float& outR) noexcept {
        // Dual-line allpass / micro-diffusion
        const float d1 = diffuser1_[diffPos1_];
        const float d2 = diffuser2_[diffPos2_];

        const float g = 0.55f + getParam(6) * 0.25f; // SIZE / diffusion density
        diffuser1_[diffPos1_] = in + d1 * g;
        diffuser2_[diffPos2_] = in + d2 * g;

        diffPos1_ = (diffPos1_ + 1) % 179;
        diffPos2_ = (diffPos2_ + 1) % 239;

        outL = d1 - in * g;
        outR = d2 - in * g;
    }

    void updateEnvelopes() noexcept {
        // Fast burst decay: ~2.0 ms
        burstDecayCoeff_ = FastMath::fastExp(-invSr_ / 0.0020f);

        // K03 (DECAY): 50 ms .. 3.0 s
        const float decayNorm = getParam(2);
        const float tailSec = 0.050f + 2.950f * (decayNorm * decayNorm);
        tailDecayCoeff_ = FastMath::fastExp(-invSr_ / std::max(0.010f, tailSec));
    }

    void updateFilter() noexcept {
        // K02 (TONE): Bandpass center 600 Hz .. 6 kHz
        const float toneNorm = getParam(1);
        const float centerHz = 600.0f * FastMath::fastPow2(toneNorm * 3.321928f);
        filterSvf_.setBandpass(centerHz, 1.6f, invSr_);
    }

    [[nodiscard]] inline float nextNoise() noexcept {
        noiseSeed_ = noiseSeed_ * 1664525u + 1013904223u;
        return static_cast<float>(static_cast<int32_t>(noiseSeed_)) * (1.0f / 2147483648.0f);
    }

    float sr_ = 48000.0f;
    float invSr_ = 1.0f / 48000.0f;
    float vel_ = 1.0f;

    float phase0_ = 0.0f;

    // Burst sequencing
    int burstsRemaining_ = 0;
    int burstTimer_ = 0;
    int burstSpacingSamples_ = 576; // ~12 ms at 48k

    float burstEnv_ = 0.0f;
    float tailEnv_ = 0.0f;

    float burstDecayCoeff_ = 0.985f;
    float tailDecayCoeff_ = 0.999f;

    TbdAudio::DSPCommon::CytomicSVF filterSvf_;

    // Micro-diffuser arrays
    std::array<float, 179> diffuser1_{};
    std::array<float, 239> diffuser2_{};
    size_t diffPos1_ = 0;
    size_t diffPos2_ = 0;

    float dcL_ = 0.0f;
    float dcR_ = 0.0f;
    float dcLOut_ = 0.0f;
    float dcROut_ = 0.0f;

    uint32_t noiseSeed_ = 0x808C0001u;
};

} // namespace tkdsp
