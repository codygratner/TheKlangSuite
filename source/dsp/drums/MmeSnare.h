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
 * @brief MmeSnare (MME-SD) — Multi-Machine Snare Drum Engine (Venti 32-Control).
 *
 * Implements 16 curated electronic snare drum synthesis models under the unified
 * role-based MME architecture:
 *
 *   00: MD-TRX-SD — Machinedrum TRX Dual-Sine Tone + Noise Snap
 *   01: MD-EFM-SD — Machinedrum EFM Enharmonic FM Snare
 *   02: MD-PI-SD  — Machinedrum P-I Acoustic Shell with Rattling Wires
 *   03: TR-606-SD — Roland TR-606 Drumatix Snappy Highpass Noise Snare
 *   04: DR-110-SD — Boss DR-110 Dr. Rhythm Paper-Crisp Analog Snap Snare
 *   05: TR-909-SD — Roland TR-909 Dual Bridged-T Shell + Shaped Noise Snare
 *   06: TR-808-SD — Roland TR-808 Twin-T Dual Tone + Sizzle Snare
 *   07: PK-NOI-SD — Erica Pērkons Analog Filter Noise Snare
 *   08: PK-KAR-SD — Erica Pērkons Karplus-Strong Physical Snare
 *   09: LXR-SUB-SD— Erica LXR-02 Subtractive Snappy Snare
 *   10: MO-BSC-SD — Modor DR-2 6-Sine FM Complex Snare
 *   11: MO-MRC-SD — Modor DR-2 Marching Band Ultra-Crisp Snare
 *   12: VM-DRM-SD — Vermona DRM1 MKIV Tonal Shell + Pseudo-Reverb Snare
 *   13: VM-DNG-SD — Vermona drumDING Filtered Noise + Crack Snare
 *   14: SYN-EXP-SD— Pearl Syncussion SY-1 Mode C Dual-Osc Explosive Snare
 *   15: WLD-ATK-SD— Waldorf Attack Poisson Crackle + Multi-Mode VCF Snare
 *
 * The Jam Card (Bank 0: Knobs 01–08):
 *   [01: MODEL] [02: PITCH] [03: DECAY] [04: SNAP ]
 *   [05: TONE ] [06: TUNE ] [07: WIRES] [08: BODY ]
 *
 * Real-Time Audio Thread Safety:
 *   - 100% zero heap allocations during trigger(), setParam(), or renderSampleStereo().
 *   - Zero CRT transcendental calls (uses TbdAudio::FastMath exclusively).
 *   - Guaranteed finite output with integrated DC blocking.
 */
class MmeSnare final : public MultiMachineEngineBase {
public:
    enum Model : uint8_t {
        kMdTrxSd = 0,   ///< MD-TRX-SD: Machinedrum TRX Dual-Sine Tone + Noise Snap
        kMdEfmSd = 1,   ///< MD-EFM-SD: Machinedrum EFM Enharmonic FM Snare
        kMdPiSd  = 2,   ///< MD-PI-SD: Machinedrum P-I Acoustic Shell with Wires
        kTr606Sd = 3,   ///< TR-606-SD: Roland TR-606 Drumatix Snappy Noise Snare
        kDr110Sd = 4,   ///< DR-110-SD: Boss DR-110 Dr. Rhythm Paper-Crisp Snare
        kTr909Sd = 5,   ///< TR-909-SD: Roland TR-909 Dual Bridged-T Snare
        kTr808Sd = 6,   ///< TR-808-SD: Roland TR-808 Twin-T Sizzle Snare
        kPkNoiSd = 7,   ///< PK-NOI-SD: Erica Pērkons Analog Filter Noise Snare
        kPkKarSd = 8,   ///< PK-KAR-SD: Erica Pērkons Karplus-Strong Snare
        kLxrSubSd = 9,  ///< LXR-SUB-SD: Erica LXR-02 Subtractive Snappy Snare
        kMoBscSd = 10,  ///< MO-BSC-SD: Modor DR-2 6-Sine FM Complex Snare
        kMoMrcSd = 11,  ///< MO-MRC-SD: Modor DR-2 Marching Band Ultra-Crisp Snare
        kVmDrmSd = 12,  ///< VM-DRM-SD: Vermona DRM1 MKIV Tonal Shell Snare
        kVmDngSd = 13,  ///< VM-DNG-SD: Vermona drumDING Filtered Crack Snare
        kSynExpSd = 14, ///< SYN-EXP-SD: Pearl Syncussion SY-1 Mode C Snare
        kWldAtkSd = 15  ///< WLD-ATK-SD: Waldorf Attack Poisson Crackle Snare
    };

    static constexpr uint8_t kTotalModels = 16;

    static constexpr const char* kModelNames[kTotalModels] = {
        "MD-TRX-SD", "MD-EFM-SD", "MD-PI-SD", "TR-606-SD",
        "DR-110-SD", "TR-909-SD", "TR-808-SD", "PK-NOI-SD",
        "PK-KAR-SD", "LXR-SUB-SD", "MO-BSC-SD", "MO-MRC-SD",
        "VM-DRM-SD", "VM-DNG-SD", "SYN-EXP-SD", "WLD-ATK-SD"
    };

    MmeSnare() noexcept {
        // Default Bank 0 Jam Card parameters
        setParam(0, 0.0f);  // K01: MODEL  (MD-TRX-SD)
        setParam(1, 0.40f); // K02: PITCH  (~180 Hz shell root)
        setParam(2, 0.45f); // K03: DECAY  (~280 ms)
        setParam(3, 0.55f); // K04: SNAP   (balanced noise snap vs body)
        setParam(4, 0.50f); // K05: TONE   (crisp 2.5 kHz bandpass noise)
        setParam(5, 0.35f); // K06: TUNE   (dual-osc detune interval)
        setParam(6, 0.40f); // K07: WIRES  (wire rattle decay / tension)
        setParam(7, 0.30f); // K08: BODY   (tonal shell presence)

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

    ~MmeSnare() override = default;

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
        toneEnv_ = vel_;
        noiseEnv_ = vel_;
        wireEnv_ = vel_;
        snapEnv_ = vel_;

        phase0_ = 0.0f;
        phase1_ = 0.0f;
        phase2_ = 0.0f;
        fmFeedback_ = 0.0f;

        // Reset Karplus delay line
        karplusRead_ = 0;
        karplusWrite_ = 0;
        karplusBuffer_.fill(0.0f);
        // Excite Karplus buffer with initial burst
        for (size_t i = 0; i < 64; ++i) {
            karplusBuffer_[i] = nextNoise() * vel_;
        }

        toneSvf_.reset();
    }

    void onSubModelReset(uint8_t /*modelIndex*/) noexcept override {
        // PRESERVE envelopes so sound does not drop out on live model change!
        phase0_ = 0.0f;
        phase1_ = 0.0f;
        phase2_ = 0.0f;
        fmFeedback_ = 0.0f;
        dcL_ = 0.0f;
        dcR_ = 0.0f;
        karplusBuffer_.fill(0.0f);
        toneSvf_.reset();
    }

    void onParamChanged(int index, float /*value*/) noexcept override {
        if (index == 2 || index == 6) {
            updateEnvelopes();
        } else if (index == 4) {
            updateFilter();
        }
    }

    void renderSubModelStereo(uint8_t modelIndex, float pitchHz, float& outL, float& outR) noexcept override {
        // Base shell frequency: 100 Hz .. 450 Hz
        const float pitchNorm = getParam(1);
        const float shellRootHz = std::clamp(pitchHz * (0.8f + pitchNorm * 2.2f), 80.0f, 650.0f);

        // Secondary shell oscillator interval detune: 1.2 .. 2.2 (K06 / TUNE)
        const float detuneRatio = 1.25f + getParam(5) * 0.95f;
        const float shellSecondHz = shellRootHz * detuneRatio;

        float monoSample = 0.0f;

        switch (modelIndex) {
            case kMdTrxSd:   monoSample = renderMdTrxSd(shellRootHz, shellSecondHz); break;
            case kMdEfmSd:   monoSample = renderMdEfmSd(shellRootHz); break;
            case kMdPiSd:    monoSample = renderMdPiSd(shellRootHz, shellSecondHz); break;
            case kTr606Sd:   monoSample = renderTr606Sd(shellRootHz); break;
            case kDr110Sd:   monoSample = renderDr110Sd(shellRootHz); break;
            case kTr909Sd:   monoSample = renderTr909Sd(shellRootHz, shellSecondHz); break;
            case kTr808Sd:   monoSample = renderTr808Sd(shellRootHz, shellSecondHz); break;
            case kPkNoiSd:   monoSample = renderPkNoiSd(shellRootHz); break;
            case kPkKarSd:   monoSample = renderPkKarSd(shellRootHz); break;
            case kLxrSubSd:  monoSample = renderLxrSubSd(shellRootHz); break;
            case kMoBscSd:   monoSample = renderMoBscSd(shellRootHz); break;
            case kMoMrcSd:   monoSample = renderMoMrcSd(shellRootHz); break;
            case kVmDrmSd:   monoSample = renderVmDrmSd(shellRootHz, shellSecondHz); break;
            case kVmDngSd:   monoSample = renderVmDngSd(shellRootHz); break;
            case kSynExpSd:  monoSample = renderSynExpSd(shellRootHz, shellSecondHz); break;
            case kWldAtkSd:  monoSample = renderWldAtkSd(shellRootHz); break;
            default:         monoSample = renderMdTrxSd(shellRootHz, shellSecondHz); break;
        }

        stepEnvelopes();

        // High-pass DC Blocker
        const float dcOut = monoSample - dcL_ + 0.9995f * dcR_;
        dcL_ = monoSample;
        dcR_ = dcOut;

        const float pan = getParam(23);
        outL = dcOut * (1.0f - 0.5f * pan);
        outR = dcOut * (0.5f + 0.5f * pan);
    }

private:
    // =========================================================================
    // SUB-MODEL SYNTHESIS IMPLEMENTATIONS
    // =========================================================================

    // 00: MD-TRX-SD — Machinedrum TRX Dual-Sine Tone + Noise Snap
    [[nodiscard]] float renderMdTrxSd(float rootHz, float secHz) noexcept {
        advancePhase(phase0_, rootHz);
        advancePhase(phase1_, secHz);
        const float tone = (FastMath::fastSinNorm(phase0_) + FastMath::fastSinNorm(phase1_) * 0.7f) * toneEnv_ * getParam(7);

        // Filtered white noise snap
        const float rawNoise = nextNoise();
        const float filtNoise = toneSvf_.process(rawNoise);
        const float snap = filtNoise * noiseEnv_ * getParam(3) * 1.4f;

        // Wire rattle
        const float rattle = nextNoise() * wireEnv_ * getParam(6) * 0.4f;

        return FastMath::fastTanh((tone + snap + rattle) * (1.0f + getParam(7) * 1.5f));
    }

    // 01: MD-EFM-SD — Machinedrum EFM Enharmonic FM Snare
    [[nodiscard]] float renderMdEfmSd(float rootHz) noexcept {
        const float ratio = 1.75f + getParam(16) * 4.25f;
        advancePhase(phase1_, rootHz * ratio);
        const float mod = FastMath::fastSinNorm(phase1_) * getParam(17) * 2.8f * toneEnv_;

        advancePhase(phase0_, rootHz);
        const float carrier = FastMath::fastSinNorm(phase0_ + mod * 0.15915494f) * toneEnv_ * getParam(7);

        const float noise = toneSvf_.process(nextNoise()) * noiseEnv_ * getParam(3) * 1.3f;
        return FastMath::fastTanh((carrier + noise) * 1.5f);
    }

    // 02: MD-PI-SD — Machinedrum P-I Acoustic Shell with Wires
    [[nodiscard]] float renderMdPiSd(float rootHz, float secHz) noexcept {
        advancePhase(phase0_, rootHz);
        advancePhase(phase1_, secHz);
        const float membrane = (FastMath::fastSinNorm(phase0_) * 0.6f + FastMath::fastSinNorm(phase1_) * 0.4f) * toneEnv_;

        // Wire buzz (non-linear contact bounce against bottom head)
        const float contact = std::max(0.0f, membrane) * nextNoise() * wireEnv_ * getParam(6) * 1.5f;
        const float snap = toneSvf_.process(nextNoise()) * noiseEnv_ * getParam(3);

        return FastMath::fastTanh((membrane * getParam(7) + contact + snap) * 1.4f);
    }

    // 03: TR-606-SD — Roland TR-606 Drumatix Snappy Noise Snare
    [[nodiscard]] float renderTr606Sd(float rootHz) noexcept {
        // Highpass-heavy snappy noise burst + soft bridged-T thud
        advancePhase(phase0_, rootHz);
        const float tone = FastMath::fastSinNorm(phase0_) * toneEnv_ * getParam(7) * 0.7f;

        const float noise = (nextNoise() - nextNoise() * 0.3f) * noiseEnv_ * getParam(3) * 1.6f;
        return FastMath::fastTanh((tone + noise) * 1.3f);
    }

    // 04: DR-110-SD — Boss DR-110 Dr. Rhythm Paper-Crisp Snare
    [[nodiscard]] float renderDr110Sd(float rootHz) noexcept {
        // Distinctive paper-crisp high-passed snap + tight thud
        advancePhase(phase0_, rootHz * 1.1f);
        const float tone = FastMath::fastSinNorm(phase0_) * toneEnv_ * getParam(7) * 0.6f;

        // Bright differentiated noise
        const float n = nextNoise();
        const float diffNoise = (n - lastNoise_) * noiseEnv_ * getParam(3) * 1.8f;
        lastNoise_ = n;

        return FastMath::fastTanh((tone + diffNoise) * 1.5f);
    }

    // 05: TR-909-SD — Roland TR-909 Dual Bridged-T Snare
    [[nodiscard]] float renderTr909Sd(float rootHz, float secHz) noexcept {
        // Authentic 909: Two bridged-T resonant tone generators + shaped noise
        advancePhase(phase0_, rootHz);
        advancePhase(phase1_, secHz);
        const float tone = (FastMath::fastSinNorm(phase0_) * 0.6f + FastMath::fastSinNorm(phase1_) * 0.4f) * toneEnv_ * getParam(7);

        const float snap = toneSvf_.process(nextNoise()) * noiseEnv_ * getParam(3) * 1.5f;
        const float initialClick = snapEnv_ * getParam(3) * 0.5f * nextNoise();

        return FastMath::fastTanh((tone + snap + initialClick) * 1.4f);
    }

    // 06: TR-808-SD — Roland TR-808 Twin-T Sizzle Snare
    [[nodiscard]] float renderTr808Sd(float rootHz, float secHz) noexcept {
        // 808 twin-T oscillators + highpass noise sizzle
        advancePhase(phase0_, rootHz);
        advancePhase(phase1_, secHz);
        const float tone = (FastMath::fastSinNorm(phase0_) + FastMath::fastSinNorm(phase1_) * 0.5f) * toneEnv_ * getParam(7) * 0.8f;

        const float sizzle = toneSvf_.process(nextNoise()) * noiseEnv_ * getParam(3) * 1.5f;
        return FastMath::fastTanh((tone + sizzle) * 1.3f);
    }

    // 07: PK-NOI-SD — Erica Pērkons Analog Filter Noise Snare
    [[nodiscard]] float renderPkNoiSd(float rootHz) noexcept {
        advancePhase(phase0_, rootHz);
        const float body = FastMath::fastSinNorm(phase0_) * toneEnv_ * getParam(7);

        // Crushed harsh noise
        const float raw = nextNoise();
        const float crushed = (raw > 0.1f) ? 0.7f : ((raw < -0.1f) ? -0.7f : 0.0f);
        const float noise = crushed * noiseEnv_ * getParam(3) * 1.6f;

        return FastMath::fastTanh((body + noise) * (1.2f + getParam(7) * 2.0f));
    }

    // 08: PK-KAR-SD — Erica Pērkons Karplus-Strong Snare
    [[nodiscard]] float renderPkKarSd(float rootHz) noexcept {
        // Karplus-Strong string/spring loop
        const size_t delayLength = std::clamp(static_cast<size_t>(sr_ / std::max(60.0f, rootHz)), size_t(16), size_t(255));
        const float readVal = karplusBuffer_[karplusRead_];

        // Lowpass feedback loop
        const float damp = 0.45f + getParam(22) * 0.5f;
        const float feedback = readVal * (1.0f - damp);
        karplusBuffer_[karplusWrite_] = feedback;

        karplusRead_ = (karplusRead_ + 1) % delayLength;
        karplusWrite_ = (karplusWrite_ + 1) % delayLength;

        const float snap = toneSvf_.process(nextNoise()) * noiseEnv_ * getParam(3);
        return FastMath::fastTanh((readVal * getParam(7) + snap) * 1.5f);
    }

    // 09: LXR-SUB-SD — Erica LXR-02 Subtractive Snappy Snare
    [[nodiscard]] float renderLxrSubSd(float rootHz) noexcept {
        advancePhase(phase0_, rootHz);
        const float tri = (phase0_ < 0.5f) ? (4.0f * phase0_ - 1.0f) : (3.0f - 4.0f * phase0_);
        const float body = tri * toneEnv_ * getParam(7);

        const float noise = toneSvf_.process(nextNoise()) * noiseEnv_ * getParam(3) * 1.6f;
        return FastMath::fastTanh((body + noise) * 1.5f);
    }

    // 10: MO-BSC-SD — Modor DR-2 6-Sine FM Complex Snare
    [[nodiscard]] float renderMoBscSd(float rootHz) noexcept {
        advancePhase(phase0_, rootHz);
        advancePhase(phase1_, rootHz * 2.14f);
        advancePhase(phase2_, rootHz * 3.47f);

        const float tone = (FastMath::fastSinNorm(phase0_) + FastMath::fastSinNorm(phase1_) * 0.6f + FastMath::fastSinNorm(phase2_) * 0.3f) * toneEnv_ * getParam(7);
        const float noise = toneSvf_.process(nextNoise()) * noiseEnv_ * getParam(3) * 1.4f;

        return FastMath::fastTanh((tone + noise) * 1.4f);
    }

    // 11: MO-MRC-SD — Modor DR-2 Marching Band Ultra-Crisp Snare
    [[nodiscard]] float renderMoMrcSd(float rootHz) noexcept {
        advancePhase(phase0_, rootHz * 1.3f);
        const float rimTone = FastMath::fastSinNorm(phase0_) * toneEnv_ * getParam(7) * 0.5f;

        // Tight high-density marching snare wire rattle
        const float wires = toneSvf_.process(nextNoise()) * wireEnv_ * getParam(6) * 1.7f;
        const float snap = nextNoise() * noiseEnv_ * getParam(3) * 0.8f;

        return FastMath::fastTanh((rimTone + wires + snap) * 1.6f);
    }

    // 12: VM-DRM-SD — Vermona DRM1 MKIV Tonal Shell Snare
    [[nodiscard]] float renderVmDrmSd(float rootHz, float secHz) noexcept {
        advancePhase(phase0_, rootHz);
        advancePhase(phase1_, secHz);
        const float tone = (FastMath::fastSinNorm(phase0_) + FastMath::fastSinNorm(phase1_) * 0.5f) * toneEnv_ * getParam(7);

        // Analog pseudo-reverb noise decay
        const float noise = toneSvf_.process(nextNoise()) * noiseEnv_ * getParam(3) * 1.4f;
        return FastMath::fastTanh((tone + noise) * 1.3f);
    }

    // 13: VM-DNG-SD — Vermona drumDING Filtered Crack Snare
    [[nodiscard]] float renderVmDngSd(float rootHz) noexcept {
        advancePhase(phase0_, rootHz);
        const float core = FastMath::fastSinNorm(phase0_) * toneEnv_ * getParam(7) * 0.7f;

        // Sharp crack transient
        const float crack = (snapEnv_ > 0.4f ? nextNoise() * 1.5f : 0.0f) * getParam(3);
        const float noise = toneSvf_.process(nextNoise()) * noiseEnv_ * getParam(3) * 1.2f;

        return FastMath::fastTanh((core + crack + noise) * 1.4f);
    }

    // 14: SYN-EXP-SD — Pearl Syncussion SY-1 Mode C Snare
    [[nodiscard]] float renderSynExpSd(float rootHz, float secHz) noexcept {
        advancePhase(phase0_, rootHz);
        advancePhase(phase1_, secHz);
        const float osc = (FastMath::fastSinNorm(phase0_) + FastMath::fastSinNorm(phase1_)) * toneEnv_ * getParam(7);

        // Syncussion explosive noise mix
        const float noise = nextNoise() * noiseEnv_ * getParam(3) * 1.8f;
        return FastMath::fastTanh((osc + noise) * 1.5f);
    }

    // 15: WLD-ATK-SD — Waldorf Attack Poisson Crackle Snare
    [[nodiscard]] float renderWldAtkSd(float rootHz) noexcept {
        advancePhase(phase0_, rootHz);
        const float body = FastMath::fastSinNorm(phase0_) * toneEnv_ * getParam(7);

        // Poisson crackle: intermittent dense impulses
        const float pNoise = (nextNoise() > 0.85f ? nextNoise() * 2.0f : 0.0f) * wireEnv_ * getParam(6);
        const float noise = toneSvf_.process(nextNoise()) * noiseEnv_ * getParam(3) * 1.3f;

        return FastMath::fastTanh((body + pNoise + noise) * 1.4f);
    }

    // =========================================================================
    // HELPERS
    // =========================================================================

    inline void advancePhase(float& phase, float freqHz) const noexcept {
        phase += freqHz * invSr_;
        if (phase >= 1.0f) phase -= std::floor(phase);
    }

    inline void stepEnvelopes() noexcept {
        toneEnv_  *= toneDecayCoeff_;
        noiseEnv_ *= noiseDecayCoeff_;
        wireEnv_  *= wireDecayCoeff_;
        snapEnv_  *= snapDecayCoeff_;
    }

    void updateEnvelopes() noexcept {
        // K03 (DECAY): 30 ms .. 2.5 s
        const float decayNorm = getParam(2);
        const float decaySec = 0.030f + 2.470f * (decayNorm * decayNorm);
        toneDecayCoeff_ = FastMath::fastExp(-invSr_ / std::max(0.010f, decaySec * 0.7f));
        noiseDecayCoeff_ = FastMath::fastExp(-invSr_ / std::max(0.010f, decaySec));

        // K07 (WIRES): 20 ms .. 1.2 s
        const float wireNorm = getParam(6);
        const float wireSec = 0.020f + 1.180f * (wireNorm * wireNorm);
        wireDecayCoeff_ = FastMath::fastExp(-invSr_ / std::max(0.010f, wireSec));

        // Fast initial snap: ~4 ms
        snapDecayCoeff_ = FastMath::fastExp(-invSr_ / 0.004f);
    }

    void updateFilter() noexcept {
        // K05 (TONE): Noise bandpass cutoff 800 Hz .. 8 kHz
        const float toneNorm = getParam(4);
        const float cutoffHz = 800.0f * FastMath::fastPow2(toneNorm * 3.321928f);
        toneSvf_.setBandpass(cutoffHz, 1.8f, invSr_);
    }

    [[nodiscard]] inline float nextNoise() noexcept {
        noiseSeed_ = noiseSeed_ * 1664525u + 1013904223u;
        return static_cast<float>(static_cast<int32_t>(noiseSeed_)) * (1.0f / 2147483648.0f);
    }

    float sr_ = 48000.0f;
    float invSr_ = 1.0f / 48000.0f;
    float vel_ = 1.0f;

    float phase0_ = 0.0f;
    float phase1_ = 0.0f;
    float phase2_ = 0.0f;
    float fmFeedback_ = 0.0f;

    float toneEnv_ = 0.0f;
    float noiseEnv_ = 0.0f;
    float wireEnv_ = 0.0f;
    float snapEnv_ = 0.0f;

    float toneDecayCoeff_ = 0.999f;
    float noiseDecayCoeff_ = 0.999f;
    float wireDecayCoeff_ = 0.998f;
    float snapDecayCoeff_ = 0.95f;

    TbdAudio::DSPCommon::CytomicSVF toneSvf_;

    // Karplus Delay Buffer
    std::array<float, 256> karplusBuffer_{};
    size_t karplusRead_ = 0;
    size_t karplusWrite_ = 0;

    float lastNoise_ = 0.0f;
    float dcL_ = 0.0f;
    float dcR_ = 0.0f;

    uint32_t noiseSeed_ = 0x9095D001u;
};

} // namespace tkdsp
