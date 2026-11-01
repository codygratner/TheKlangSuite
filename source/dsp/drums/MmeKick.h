#pragma once

#include "../engines/MultiMachineEngineBase.h"

#include "../../FastMath.h"

#include <algorithm>

#include <array>

#include <cmath>

#include <cstdint>

namespace tkdsp {

/**

 * @brief MmeKick (MME-BD) — Multi-Machine Bass Drum Engine (Venti 32-Control).

 *

 * Implements 16 curated electronic bass drum synthesis models under the unified

 * role-based MME architecture:

 *

 *   00: MD-TRX-BD — Machinedrum TRX Classic 909/808 Sine Kick

 *   01: MD-TRX-B2 — Machinedrum TRX Hard Acoustic Kick with Tick

 *   02: MD-EFM-BD — Machinedrum EFM 2-Operator FM Metallic Kick

 *   03: MD-PI-BD  — Machinedrum P-I Physical Modeling Drum Membrane Kick

 *   04: TR-606-BD — Roland TR-606 Drumatix Bridged-T Bouncy Low Kick

 *   05: DR-110-BD — Boss DR-110 Dr. Rhythm Punchy Mid-Bass Thud

 *   06: TR-909-BD — Roland TR-909 Dual-Envelope Click & Tune Kick

 *   07: TR-808-BD — Roland TR-808 High-Q Bridged-T Sub-Bass Boom Kick

 *   08: PK-WAV-BD — Erica Pērkons HD-01 Wavefold / Overdrive Kick

 *   09: PK-FMS-BD — Erica Pērkons HD-01 FM Sub Drop Kick

 *   10: LXR-DIG-BD— Erica LXR-02 Multi-Op Digital FM Transient Kick

 *   11: MO-DRV-BD — Modor DR-2 Parallel Filter Overdrive Kick

 *   12: MO-SQR-BD — Modor DR-2 Square Sub-Heavy Resonant Kick

 *   13: VM-DRM-BD — Vermona DRM1 MKIV Discrete Bridged-T Kick

 *   14: VM-DNG-BD — Vermona drumDING Continuously Shaped VCO Kick

 *   15: SYN-LAS-BD— Pearl Syncussion SY-1 Mode A Linear/Exp Laser Sub Kick

 *

 * The Jam Card (Bank 0: Knobs 01–08):

 *   [01: MODEL] [02: PITCH] [03: DECAY] [04: PUNCH]

 *   [05: SWEEP] [06: SWP-T] [07: SUB  ] [08: BODY ]

 *

 * Real-Time Audio Thread Safety:

 *   - 100% zero heap allocations during trigger(), setParam(), or renderSampleStereo().

 *   - Zero CRT transcendental calls (uses TbdAudio::FastMath exclusively).

 *   - Guaranteed finite output with integrated DC blocking.

 */

class MmeKick final : public MultiMachineEngineBase {

public:

    enum Model : uint8_t {

        kMdTrxBd = 0,   ///< MD-TRX-BD: Machinedrum TRX Classic Sine Kick

        kMdTrxB2 = 1,   ///< MD-TRX-B2: Machinedrum TRX Hard Acoustic Kick with Tick

        kMdEfmBd = 2,   ///< MD-EFM-BD: Machinedrum EFM 2-Operator FM Metallic Kick

        kMdPiBd  = 3,   ///< MD-PI-BD: Machinedrum P-I Physical Modeling Membrane Kick

        kTr606Bd = 4,   ///< TR-606-BD: Roland TR-606 Drumatix Bouncy Low Kick

        kDr110Bd = 5,   ///< DR-110-BD: Boss DR-110 Dr. Rhythm Punchy Mid-Bass Thud

        kTr909Bd = 6,   ///< TR-909-BD: Roland TR-909 Dual-Envelope Click & Tune Kick

        kTr808Bd = 7,   ///< TR-808-BD: Roland TR-808 Bridged-T Deep Sub Boom Kick

        kPkWavBd = 8,   ///< PK-WAV-BD: Erica Pērkons HD-01 Wavefold / Overdrive Kick

        kPkFmsBd = 9,   ///< PK-FMS-BD: Erica Pērkons HD-01 FM Sub Drop Kick

        kLxrDigBd = 10, ///< LXR-DIG-BD: Erica LXR-02 Multi-Op Digital FM Transient Kick

        kMoDrvBd = 11,  ///< MO-DRV-BD: Modor DR-2 Parallel Filter Overdrive Kick

        kMoSqrBd = 12,  ///< MO-SQR-BD: Modor DR-2 Square Sub-Heavy Kick

        kVmDrmBd = 13,  ///< VM-DRM-BD: Vermona DRM1 MKIV Discrete Bridged-T Kick

        kVmDngBd = 14,  ///< VM-DNG-BD: Vermona drumDING Continuously Shaped VCO Kick

        kSynLasBd = 15  ///< SYN-LAS-BD: Pearl Syncussion SY-1 Mode A Laser Sub Kick

    };

    static constexpr uint8_t kTotalModels = 16;

    static constexpr const char* kModelNames[kTotalModels] = {

        "MD-TRX-BD", "MD-TRX-B2", "MD-EFM-BD", "MD-PI-BD",

        "TR-606-BD", "DR-110-BD", "TR-909-BD", "TR-808-BD",

        "PK-WAV-BD", "PK-FMS-BD", "LXR-DIG-BD", "MO-DRV-BD",

        "MO-SQR-BD", "VM-DRM-BD", "VM-DNG-BD", "SYN-LAS-BD"

    };

    MmeKick() noexcept {

        // Default Bank 0 Jam Card parameters

        setParam(0, 0.0f);  // K01: MODEL  (MD-TRX-BD)

        setParam(1, 0.35f); // K02: PITCH  (~55 Hz base)

        setParam(2, 0.45f); // K03: DECAY  (~400 ms)

        setParam(3, 0.50f); // K04: PUNCH  (balanced attack click)

        setParam(4, 0.60f); // K05: SWEEP  (punchy pitch drop)

        setParam(5, 0.30f); // K06: SWP-T  (~25 ms sweep time)

        setParam(6, 0.30f); // K07: SUB    (solid sub foundation)

        setParam(7, 0.25f); // K08: BODY   (subtle warm saturation)

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

        setParam(26, 0.00f); // K27: KEYTRACK (0 = drum mode)

        setParam(27, 0.50f); // K28: V-SENS

        setParam(28, 0.00f); // K29: V-DEST

        setParam(29, 0.00f); // K30: DISP

        setParam(30, 0.00f); // K31: FLAM

        setParam(31, 0.00f); // K32: MACRO

        reset();

    }

    ~MmeKick() override = default;

    [[nodiscard]] uint8_t numModels() const noexcept override { return kTotalModels; }

    [[nodiscard]] static const char* getModelName(uint8_t modelIndex) noexcept {

        return (modelIndex < kTotalModels) ? kModelNames[modelIndex] : "UNKNOWN";

    }

    void setSustain(bool s) noexcept {

        isSustained_ = s;

        if (s) releaseCoeff_ = 0.0f;

    }

    void noteOff(float releaseSec = 0.07f) noexcept {

        isSustained_ = false;

        releaseSec = std::clamp(releaseSec, 0.005f, 2.0f);

        releaseCoeff_ = FastMath::fastExp(-invSr_ / releaseSec);

    }

    [[nodiscard]] bool isSustained() const noexcept { return isSustained_; }

protected:

    void onPrepareSubModels(float sampleRate) noexcept override {

        sr_ = sampleRate;

        invSr_ = 1.0f / sr_;

        updateEnvelopes();

        reset();

    }

    void onSubModelTrigger(uint8_t /*modelIndex*/, float velocity) noexcept override {

        vel_ = std::clamp(velocity, 0.05f, 1.0f);

        ampEnv_ = vel_;

        pitchEnv_ = 1.0f;

        clickEnv_ = vel_;

        tickEnv_ = 1.0f;

        fmEnv_ = 1.0f;

        // Reset phases on trigger for maximum transient punch

        phase0_ = 0.0f;

        phase1_ = 0.0f;

        phase2_ = 0.0f;

        fmFeedback_ = 0.0f;

        // Reset modal membrane mode phases

        for (int i = 0; i < 4; ++i) {

            modalAmp_[i] = vel_ / (1.0f + static_cast<float>(i) * 0.4f);

            modalPhase_[i] = 0.0f;

        }

        // Reset filter states

        s0_ = 0.0f;

        s1_ = 0.0f;

        s2_ = 0.0f;

        s3_ = 0.0f;

    }

    void onSubModelReset(uint8_t modelIndex) noexcept override {
        // PRESERVE ampEnv_ and vel_ so sound never drops out on live model change!
        phase0_ = 0.0f;
        phase1_ = 0.0f;
        phase2_ = 0.0f;
        fmFeedback_ = 0.0f;
        s0_ = 0.0f;
        s1_ = 0.0f;
        s2_ = 0.0f;
        s3_ = 0.0f;

        // If modal membrane model, prime modes from active ampEnv_
        if (modelIndex == kMdPiBd) {
            for (int i = 0; i < 4; ++i) {
                modalPhase_[i] = 0.0f;
                modalAmp_[i] = std::max(modalAmp_[i], ampEnv_ * (1.0f / (1.0f + static_cast<float>(i))));
            }
        }
    }

    void onParamChanged(int index, float /*value*/) noexcept override {

        if (index == 2 || index == 5 || index == 21) {

            updateEnvelopes();

        }

    }

    void renderSubModelStereo(uint8_t modelIndex, float pitchHz, float& outL, float& outR) noexcept override {

        // Base pitch frequency from K02 (PITCH): at center (0.5), multiplier is EXACTLY 1.0f (pure concert pitch)

        const float pitchNorm = getParam(1);

        const float pitchMult = FastMath::fastPow2((pitchNorm - 0.5f) * 2.0f); // -12st .. +12st fine tune

        const float rootHz = std::clamp(pitchHz * pitchMult, 10.0f, 1500.0f);

        // Pitch envelope modulation: K05 (SWEEP) depth up to +4 octaves

        const float sweepDepthOct = getParam(4) * 4.0f;

        const float sweepMult = (pitchEnv_ > 0.001f && sweepDepthOct > 0.001f)

            ? FastMath::fastPow2(pitchEnv_ * sweepDepthOct)

            : 1.0f;

        const float instPitchHz = rootHz * sweepMult;

        float monoSample = 0.0f;

        switch (modelIndex) {

            case kMdTrxBd:   monoSample = renderMdTrxBd(instPitchHz); break;

            case kMdTrxB2:   monoSample = renderMdTrxB2(instPitchHz); break;

            case kMdEfmBd:   monoSample = renderMdEfmBd(instPitchHz); break;

            case kMdPiBd:    monoSample = renderMdPiBd(instPitchHz); break;

            case kTr606Bd:   monoSample = renderTr606Bd(instPitchHz); break;

            case kDr110Bd:   monoSample = renderDr110Bd(instPitchHz); break;

            case kTr909Bd:   monoSample = renderTr909Bd(instPitchHz); break;

            case kTr808Bd:   monoSample = renderTr808Bd(instPitchHz); break;

            case kPkWavBd:   monoSample = renderPkWavBd(instPitchHz); break;

            case kPkFmsBd:   monoSample = renderPkFmsBd(instPitchHz); break;

            case kLxrDigBd:  monoSample = renderLxrDigBd(instPitchHz); break;

            case kMoDrvBd:   monoSample = renderMoDrvBd(instPitchHz); break;

            case kMoSqrBd:   monoSample = renderMoSqrBd(instPitchHz); break;

            case kVmDrmBd:   monoSample = renderVmDrmBd(instPitchHz); break;

            case kVmDngBd:   monoSample = renderVmDngBd(instPitchHz); break;

            case kSynLasBd:  monoSample = renderSynLasBd(instPitchHz); break;

            default:         monoSample = renderMdTrxBd(instPitchHz); break;

        }

        // Apply amplitude envelope

        monoSample *= ampEnv_;

        // Advance envelopes

        stepEnvelopes();

        // High-pass DC Blocker: y[n] = x[n] - x[n-1] + R*y[n-1] with R = 0.9995

        const float dcOut = monoSample - dcL_ + 0.9995f * dcR_;

        dcL_ = monoSample;

        dcR_ = dcOut;

        // Space / Pan spread (K24 / SPACE)

        const float pan = getParam(23);

        outL = dcOut * (1.0f - 0.5f * pan);

        outR = dcOut * (0.5f + 0.5f * pan);

    }

private:

    // =========================================================================

    // SUB-MODEL SYNTHESIS IMPLEMENTATIONS

    // =========================================================================

    // 00: MD-TRX-BD — Machinedrum TRX Classic Sine Kick

    [[nodiscard]] float renderMdTrxBd(float pitchHz) noexcept {

        advancePhase(phase0_, pitchHz);

        float core = FastMath::fastSinNorm(phase0_);

        // Sub reinforcement (K07)

        if (getParam(6) > 0.01f) {

            advancePhase(phase1_, pitchHz * 0.5f);

            core += FastMath::fastSinNorm(phase1_) * getParam(6) * 0.6f;

        }

        // Punch click transient (K04)

        const float click = clickEnv_ * getParam(3) * 0.4f * FastMath::fastSinNorm(phase0_ * 8.0f);

        core += click;

        // Body saturation (K08)

        const float drive = 1.0f + getParam(7) * 2.5f;

        return FastMath::fastTanh(core * drive);

    }

    // 01: MD-TRX-B2 — Machinedrum TRX Hard Acoustic Kick with Tick

    [[nodiscard]] float renderMdTrxB2(float pitchHz) noexcept {

        // High frequency initial tick transient

        const float tickHz = std::clamp(pitchHz + tickEnv_ * 1800.0f, 20.0f, 16000.0f);

        advancePhase(phase0_, tickHz);

        float core = FastMath::fastSinNorm(phase0_);

        // Second harmonic acoustic shell tone

        advancePhase(phase1_, pitchHz * 2.0f);

        core += FastMath::fastSinNorm(phase1_) * (getParam(7) * 0.35f);

        // Acoustic click snap

        const float snap = clickEnv_ * getParam(3) * 0.5f * nextNoise();

        core += snap;

        const float drive = 1.0f + getParam(7) * 2.0f;

        return FastMath::fastTanh(core * drive);

    }

    // 02: MD-EFM-BD — Machinedrum EFM 2-Operator FM Metallic Kick

    [[nodiscard]] float renderMdEfmBd(float pitchHz) noexcept {

        // Modulator ratio mapped from MOD-X (K17): 0.5 .. 8.0

        const float ratio = 0.5f + getParam(16) * 7.5f;

        const float modHz = pitchHz * ratio;

        advancePhase(phase1_, modHz);

        // Self-feedback from MOD-Z (K19)

        const float fbAmount = getParam(18) * 0.45f;

        const float modSignal = FastMath::fastSinNorm(phase1_ + fmFeedback_ * fbAmount);

        fmFeedback_ = modSignal;

        // Carrier modulated by modulator * fmEnv * MOD-Y (K18)

        const float fmIndex = getParam(17) * 3.5f * fmEnv_;

        advancePhase(phase0_, pitchHz);

        float core = FastMath::fastSinNorm(phase0_ + modSignal * fmIndex * (1.0f / 6.2831853f));

        // Sub addition (K07)

        if (getParam(6) > 0.01f) {

            advancePhase(phase2_, pitchHz * 0.5f);

            core += FastMath::fastSinNorm(phase2_) * getParam(6) * 0.5f;

        }

        const float drive = 1.0f + getParam(7) * 2.0f;

        return FastMath::fastTanh(core * drive);

    }

    // 03: MD-PI-BD — Machinedrum P-I Physical Modeling Drum Membrane Kick

    [[nodiscard]] float renderMdPiBd(float pitchHz) noexcept {

        // Bessel circular membrane modal ratios: (0,1)=1.0, (1,1)=1.593, (2,1)=2.135, (0,2)=2.295

        constexpr float kModeRatios[4] = { 1.000f, 1.593f, 2.135f, 2.295f };

        const float dampKnob = getParam(22);

        float membrane = 0.0f;

        for (int i = 0; i < 4; ++i) {

            const float modeFreq = std::clamp(pitchHz * kModeRatios[i], 20.0f, sr_ * 0.48f);

            advancePhase(modalPhase_[i], modeFreq);

            membrane += FastMath::fastSinNorm(modalPhase_[i]) * modalAmp_[i];

            if (isSustained_ && i == 0) { modalAmp_[0] = std::max(modalAmp_[0], vel_); }
            // Higher modes decay faster

            const float modeDampFactor = 1.0f + static_cast<float>(i) * (1.5f + dampKnob * 4.0f);

            modalAmp_[i] *= FastMath::fastExp(-invSr_ * modeDampFactor * (1.0f / std::max(0.01f, getParam(2) * 1.5f)));

        }

        // Stick attack excitation impulse (K04)

        const float stick = clickEnv_ * getParam(3) * 0.4f * FastMath::fastSinNorm(phase0_ * 12.0f);

        advancePhase(phase0_, pitchHz * 6.0f);

        membrane += stick;

        const float drive = 1.0f + getParam(7) * 2.0f;

        return FastMath::fastTanh(membrane * drive);

    }

    // 04: TR-606-BD — Roland TR-606 Drumatix Bouncy Low Kick

    [[nodiscard]] float renderTr606Bd(float pitchHz) noexcept {

        // Bridged-T active resonator: slightly lower Q, warm bounce, soft click

        advancePhase(phase0_, pitchHz);

        float core = FastMath::fastSinNorm(phase0_);

        // 606 soft pulse discharge click (~1.5 ms)

        const float softClick = clickEnv_ * getParam(3) * 0.28f * (1.0f - phase0_);

        core += softClick;

        // Sub reinforcement

        if (getParam(6) > 0.01f) {

            advancePhase(phase1_, pitchHz * 0.5f);

            core += FastMath::fastSinNorm(phase1_) * getParam(6) * 0.4f;

        }

        // Spongy analog warmth (soft saturation with slight odd harmonics)

        const float drive = 1.0f + getParam(7) * 1.5f;

        const float x = core * drive;

        return x / (1.0f + std::abs(x) * 0.4f);

    }

    // 05: DR-110-BD — Boss DR-110 Dr. Rhythm Punchy Mid-Bass Thud

    [[nodiscard]] float renderDr110Bd(float pitchHz) noexcept {

        // Tighter tuning, distinctive capacitor snap, punchy indie mid-bass

        advancePhase(phase0_, pitchHz * 1.15f);

        float core = FastMath::fastSinNorm(phase0_);

        // Sharp capacitor discharge click (K04)

        const float capSnap = (clickEnv_ > 0.5f ? 1.0f : -0.5f) * clickEnv_ * getParam(3) * 0.5f;

        core += capSnap;

        // Boxy mid punch saturation

        const float drive = 1.2f + getParam(7) * 2.2f;

        return FastMath::fastTanh(core * drive);

    }

    // 06: TR-909-BD — Roland TR-909 Dual-Envelope Click & Tune Kick

    [[nodiscard]] float renderTr909Bd(float pitchHz) noexcept {

        // Fast attack tune dive + body

        advancePhase(phase0_, pitchHz);

        float core = FastMath::fastSinNorm(phase0_);

        // Dedicated 909 square/pulse click generator

        const float click = (clickEnv_ > 0.3f ? 0.6f : 0.0f) * getParam(3) * nextNoise();

        core += click;

        // Sub reinforcement (K07)

        if (getParam(6) > 0.01f) {

            advancePhase(phase1_, pitchHz * 0.5f);

            core += FastMath::fastSinNorm(phase1_) * getParam(6) * 0.5f;

        }

        // Asymmetric diode clipping curve

        const float drive = 1.0f + getParam(7) * 2.8f;

        float x = core * drive;

        x = (x > 0.0f) ? FastMath::fastTanh(x) : (FastMath::fastTanh(x * 1.25f) * (1.0f / 1.25f));

        return x;

    }

    // 07: TR-808-BD — Roland TR-808 High-Q Bridged-T Sub-Bass Boom Kick

    [[nodiscard]] float renderTr808Bd(float pitchHz) noexcept {

        // Pure resonant sine ringing in self-oscillation

        advancePhase(phase0_, pitchHz);

        float core = FastMath::fastSinNorm(phase0_);

        // Bridged-T initial thud impulse

        const float thud = clickEnv_ * getParam(3) * 0.25f;

        core += thud;

        // 808 purity: very subtle drive

        const float drive = 1.0f + getParam(7) * 1.2f;

        return FastMath::fastTanh(core * drive);

    }

    // 08: PK-WAV-BD — Erica Pērkons HD-01 Wavefold / Overdrive Kick

    [[nodiscard]] float renderPkWavBd(float pitchHz) noexcept {

        advancePhase(phase0_, pitchHz);

        const float raw = FastMath::fastSinNorm(phase0_);

        // Multi-stage wavefolding: sin(x * drive) + fold * sin(2 * x * drive)

        const float foldDrive = 1.0f + std::max(getParam(16), getParam(15)) * 4.0f; // MOD-X or K16 FOLD

        const float x = raw * foldDrive;

        float folded = FastMath::fastSinNorm(x * 0.15915494f) + 0.45f * FastMath::fastSinNorm(x * 0.31830988f);

        // Click punch

        folded += clickEnv_ * getParam(3) * 0.4f * nextNoise();

        // Master overdrive (K08)

        const float distDrive = 1.0f + getParam(7) * 3.5f;

        return FastMath::fastTanh(folded * distDrive);

    }

    // 09: PK-FMS-BD — Erica Pērkons HD-01 FM Sub Drop Kick

    [[nodiscard]] float renderPkFmsBd(float pitchHz) noexcept {

        // Deep FM plunge with harsh digital edge

        advancePhase(phase1_, pitchHz * (1.0f + getParam(16) * 3.0f));

        const float mod = FastMath::fastSinNorm(phase1_);

        const float fmDepth = getParam(17) * 4.0f * fmEnv_;

        advancePhase(phase0_, pitchHz);

        float core = FastMath::fastSinNorm(phase0_ + mod * fmDepth * 0.15915494f);

        // Sub octave grit

        advancePhase(phase2_, pitchHz * 0.5f);

        core += (FastMath::fastSinNorm(phase2_) > 0.0f ? 0.3f : -0.3f) * getParam(6);

        const float drive = 1.0f + getParam(7) * 3.0f;

        return FastMath::fastTanh(core * drive);

    }

    // 10: LXR-DIG-BD — Erica LXR-02 Multi-Op Digital FM Transient Kick

    [[nodiscard]] float renderLxrDigBd(float pitchHz) noexcept {

        // 3-Op Phase modulation stack

        advancePhase(phase2_, pitchHz * 3.0f);

        const float op3 = FastMath::fastSinNorm(phase2_) * getParam(18) * 2.0f;

        advancePhase(phase1_, pitchHz * 1.5f);

        const float op2 = FastMath::fastSinNorm(phase1_ + op3 * 0.15915494f) * getParam(16) * 2.5f;

        advancePhase(phase0_, pitchHz);

        float core = FastMath::fastSinNorm(phase0_ + op2 * fmEnv_ * 0.15915494f);

        // Crisp digital click

        core += clickEnv_ * getParam(3) * 0.5f * (nextNoise() > 0.0f ? 1.0f : -1.0f);

        const float drive = 1.0f + getParam(7) * 2.5f;

        return FastMath::fastTanh(core * drive);

    }

    // 11: MO-DRV-BD — Modor DR-2 Parallel Filter Overdrive Kick

    [[nodiscard]] float renderMoDrvBd(float pitchHz) noexcept {

        // Impulse excitation into parallel resonant lowpass & bandpass filters

        advancePhase(phase0_, pitchHz);

        const float osc = FastMath::fastSinNorm(phase0_);

        // Parallel filter emulation (simple 2-pole state variable)

        const float f = std::clamp(pitchHz * 2.0f * invSr_, 0.001f, 0.45f);

        const float q = 0.85f + getParam(18) * 3.0f;

        s0_ += f * s1_;

        s1_ += f * (osc - s0_ - q * s1_);
        s0_ = std::clamp(s0_, -3.0f, 3.0f);
        s1_ = std::clamp(s1_, -3.0f, 3.0f);

        const float lp = s0_;

        const float bp = s1_;

        float summed = lp + bp * 0.6f;

        summed += clickEnv_ * getParam(3) * 0.4f * nextNoise();

        // Modor parallel overdrive

        const float drive = 1.0f + getParam(7) * 3.5f;

        return FastMath::fastTanh(summed * drive);

    }

    // 12: MO-SQR-BD — Modor DR-2 Square Sub-Heavy Kick

    [[nodiscard]] float renderMoSqrBd(float pitchHz) noexcept {

        advancePhase(phase0_, pitchHz);

        // Variable pulse width from MOD-X (K17)

        const float pw = 0.3f + getParam(16) * 0.4f;

        const float sqr = (phase0_ < pw) ? 0.8f : -0.8f;

        // Resonant lowpass filter sweep

        const float cutoff = std::clamp(pitchHz * (1.5f + pitchEnv_ * 4.0f) * invSr_, 0.001f, 0.45f);

        s0_ += cutoff * s1_;

        s1_ += cutoff * (sqr - s0_ - 1.2f * s1_);
        s0_ = std::clamp(s0_, -3.0f, 3.0f);
        s1_ = std::clamp(s1_, -3.0f, 3.0f);

        float core = s0_;

        core += clickEnv_ * getParam(3) * 0.4f;

        const float drive = 1.0f + getParam(7) * 2.5f;

        return FastMath::fastTanh(core * drive);

    }

    // 13: VM-DRM-BD — Vermona DRM1 MKIV Discrete Bridged-T Kick

    [[nodiscard]] float renderVmDrmBd(float pitchHz) noexcept {

        // Discrete analog bridged-T resonator with continuous pitch bend

        advancePhase(phase0_, pitchHz);

        float core = FastMath::fastSinNorm(phase0_);

        // Discrete transistor click

        const float click = clickEnv_ * getParam(3) * 0.35f * FastMath::fastCosNorm(phase0_ * 4.0f);

        core += click;

        // German discrete transistor warmth

        const float drive = 1.1f + getParam(7) * 2.0f;

        return FastMath::fastTanh(core * drive);

    }

    // 14: VM-DNG-BD — Vermona drumDING Continuously Shaped VCO Kick

    [[nodiscard]] float renderVmDngBd(float pitchHz) noexcept {

        advancePhase(phase0_, pitchHz);

        const float shape = getParam(16); // MOD-X: 0 = Sine, 0.5 = Triangle, 1.0 = Saw

        float vco = 0.0f;

        if (shape <= 0.5f) {

            const float t = shape * 2.0f;

            const float sine = FastMath::fastSinNorm(phase0_);

            const float tri = (phase0_ < 0.5f) ? (4.0f * phase0_ - 1.0f) : (3.0f - 4.0f * phase0_);

            vco = sine * (1.0f - t) + tri * t;

        } else {

            const float t = (shape - 0.5f) * 2.0f;

            const float tri = (phase0_ < 0.5f) ? (4.0f * phase0_ - 1.0f) : (3.0f - 4.0f * phase0_);

            const float saw = 2.0f * phase0_ - 1.0f;

            vco = tri * (1.0f - t) + saw * t;

        }

        // Sub addition

        if (getParam(6) > 0.01f) {

            advancePhase(phase1_, pitchHz * 0.5f);

            vco += FastMath::fastSinNorm(phase1_) * getParam(6) * 0.5f;

        }

        const float drive = 1.0f + getParam(7) * 2.2f;

        return FastMath::fastTanh(vco * drive);

    }

    // 15: SYN-LAS-BD — Pearl Syncussion SY-1 Mode A Laser Sub Kick

    [[nodiscard]] float renderSynLasBd(float pitchHz) noexcept {

        // Mode A linear/exponential descending laser sweep

        advancePhase(phase0_, pitchHz);

        float core = FastMath::fastSinNorm(phase0_);

        // Dual-oscillator detune (MOD-X)

        const float detune = 1.0f + getParam(16) * 0.08f;

        advancePhase(phase1_, pitchHz * detune);

        core += FastMath::fastSinNorm(phase1_) * 0.7f;

        // Vintage dub snap

        core += clickEnv_ * getParam(3) * 0.4f * FastMath::fastSinNorm(phase0_ * 6.0f);

        const float drive = 1.1f + getParam(7) * 2.2f;

        return FastMath::fastTanh(core * drive);

    }

    // =========================================================================

    // ENVELOPE & PHASE HELPERS

    // =========================================================================

    inline void advancePhase(float& phase, float freqHz) const noexcept {

        phase += freqHz * invSr_;

        if (phase >= 1.0f) phase -= std::floor(phase);

    }

    inline void stepEnvelopes() noexcept {

        if (isSustained_) {

            // Sustained hold: amplitude remains steady while key is pressed!

        } else if (releaseCoeff_ > 0.0f) {

            ampEnv_ *= releaseCoeff_;

        } else {

            ampEnv_ *= ampDecayCoeff_;

        }

        pitchEnv_ *= pitchDecayCoeff_;

        clickEnv_ *= clickDecayCoeff_;

        tickEnv_  *= tickDecayCoeff_;

        fmEnv_    *= fmDecayCoeff_;

    }

    void updateEnvelopes() noexcept {

        // K03 (DECAY): 20 ms .. 4.0 s (Profile A cubic response)

        const float decayNorm = getParam(2);

        const float decaySec = 0.020f + 3.980f * (decayNorm * decayNorm * decayNorm);

        ampDecayCoeff_ = FastMath::fastExp(-invSr_ / std::max(0.005f, decaySec));

        // K06 (SWP-T): 2 ms .. 150 ms

        const float swpTNorm = getParam(5);

        const float swpTSec = 0.002f + 0.148f * (swpTNorm * swpTNorm);

        pitchDecayCoeff_ = FastMath::fastExp(-invSr_ / std::max(0.001f, swpTSec));

        // Fixed click envelope: ~2.5 ms

        clickDecayCoeff_ = FastMath::fastExp(-invSr_ / 0.0025f);

        // Fixed tick envelope: ~5 ms

        tickDecayCoeff_ = FastMath::fastExp(-invSr_ / 0.0050f);

        // K22 (FM-ENV): 5 ms .. 500 ms

        const float fmEnvNorm = getParam(21);

        const float fmEnvSec = 0.005f + 0.495f * (fmEnvNorm * fmEnvNorm);

        fmDecayCoeff_ = FastMath::fastExp(-invSr_ / std::max(0.002f, fmEnvSec));

    }

    [[nodiscard]] inline float nextNoise() noexcept {

        noiseSeed_ = noiseSeed_ * 1664525u + 1013904223u;

        return static_cast<float>(static_cast<int32_t>(noiseSeed_)) * (1.0f / 2147483648.0f);

    }

    // State Variables

    float sr_ = 48000.0f;

    float invSr_ = 1.0f / 48000.0f;

    float vel_ = 1.0f;

    // Phases

    float phase0_ = 0.0f;

    float phase1_ = 0.0f;

    float phase2_ = 0.0f;

    float fmFeedback_ = 0.0f;

    // Sustain & Release

    bool isSustained_ = false;

    float releaseCoeff_ = 0.0f;

    // Envelopes

    float ampEnv_ = 0.0f;

    float pitchEnv_ = 0.0f;

    float clickEnv_ = 0.0f;

    float tickEnv_ = 0.0f;

    float fmEnv_ = 0.0f;

    // Decay Coefficients

    float ampDecayCoeff_ = 0.999f;

    float pitchDecayCoeff_ = 0.99f;

    float clickDecayCoeff_ = 0.95f;

    float tickDecayCoeff_ = 0.90f;

    float fmDecayCoeff_ = 0.98f;

    // Modal Membrane Modes

    std::array<float, 4> modalPhase_{};

    std::array<float, 4> modalAmp_{};

    // Filter states

    float s0_ = 0.0f;

    float s1_ = 0.0f;

    float s2_ = 0.0f;

    float s3_ = 0.0f;

    // DC Blocker states

    float dcL_ = 0.0f;

    float dcR_ = 0.0f;

    // PRNG seed

    uint32_t noiseSeed_ = 0x808BD001u;

};

} // namespace tkdsp

