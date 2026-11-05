#pragma once

#include "FastMath.h"
#include "FormlessModMath.h"
#include "../common/SteppedRandom.h"
#include "../common/AudioEffect.h"
#include "../dynamics/WiderStage.h"
#include "../dynamics/SquashStage.h"
#include "../dynamics/DrumLimiter.h"
#include "../mojo/FreqShifter.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace tkdsp {

namespace FastMath = TbdAudio::FastMath;

struct FormlessSpawnParams {
    // Header Controls (3)
    int role{0};             // 0=BD, 1=SD, 2=TM, 3=CP, 4=HH, 5=CY, 6=PC, 7=FX
    int model{0};            // 0..7
    int lifeMode{2};         // 0=Synthetic, 1=Cyborg, 2=Organic

    // Card 1: Master Macro & Dynamics Lab (8)
    float macroX{0.5f};          // 0.0 .. 1.0 (Pitch Macro)
    float macroY{0.5f};          // 0.0 .. 1.0 (Punch Macro)
    float macroZ{0.5f};          // 0.0 .. 1.0 (Timbre Macro)
    float macroT{0.5f};          // 0.0 .. 1.0 (Time Macro)
    float velAggr{0.5f};         // 0.0 .. 1.0 (Vel -> Aggression)
    float velTimb{0.5f};         // 0.0 .. 1.0 (Vel -> Timbre)
    float pan{0.0f};             // -1.0 .. +1.0
    float decayCurve{0.0f};      // -1.0 .. +1.0

    // Card 2: Role Synthesis Lab (8)
    float paramA{0.5f};          // 0.0 .. 1.0
    float paramB{0.5f};          // 0.0 .. 1.0
    float paramC{0.5f};          // 0.0 .. 1.0
    float paramD{0.5f};          // 0.0 .. 1.0
    float paramE{0.5f};          // 0.0 .. 1.0
    float paramF{0.5f};          // 0.0 .. 1.0
    float paramG{0.5f};          // 0.0 .. 1.0
    float paramH{0.5f};          // 0.0 .. 1.0

    // Card 3 Pod 1: Stepped Random Humanization (8)
    float randX{0.0f};           // 0.0 .. 100.0 %
    float randY{0.0f};           // 0.0 .. 100.0 %
    float randZ{0.0f};           // 0.0 .. 100.0 %
    float randT{0.0f};           // 0.0 .. 100.0 %
    float randAggr{0.0f};        // 0.0 .. 100.0 %
    float randTimb{0.0f};        // 0.0 .. 100.0 %
    float randPan{0.0f};         // 0.0 .. 100.0 %
    float randDecay{0.0f};       // 0.0 .. 100.0 % (targets macroDecay)

    // Card 3 Pod 2 & 3: Master Altar & Mojo Console (13)
    float macroDecay{0.5f};      // 0.0 .. 1.0 (Master Macro)
    float volume{0.5f};          // 0.0 .. 1.0 (0=-inf, 0.5=0dB, 1=+12dB)
    float ampDecay{1.0f};        // 0.0 .. 1.0 (Uniform Role Logarithmic Decay)
    float cutoff{1000.0f};       // 20.0 .. 20000.0 Hz
    float resonance{0.30f};      // 0.0 .. 1.0 (Q in LP/HP/BP, +/-dB in PK)
    float filterEnvAmt{0.0f};    // -1.0 .. +1.0 (+/-5 Octaves)
    float filterDecay{0.150f};   // 0.005 .. 5.0 s
    int   filterType{0};         // 0=LP, 1=HP, 2=BP, 3=PK
    float preDrive{0.0f};        // 0.0 .. 1.0 (Tanh Drive: 0=clean, 0.5=+6dB, 1.0=+36dB)
    float crush{48000.0f};       // 500.0 .. 48000.0 Hz
    float shift{0.0f};          // -1000.0 .. +1000.0 Hz (Bode Frequency Shifter)
    float squash{0.0f};          // -1.0 .. +1.0 (Bipolar Dynamics)
    float wider{0.0f};           // 0.0 .. 2.0 (Mojo Stereo Widener)
    bool  masterLimiter{false};  // Master Drum Limiter
};


/**
 * @brief FormlessSpawnVoice — Sovereign 112-Model Percussion Synthesizer Workstation.
 *
 * Implements the 7 canonical percussion roles with 16 synthesis models each (112 models total),
 * adhering to the Venti 32-Control Specification and Triad Sonic Canon:
 *
 *   - 7 Canonical Roles: BD, TM, SD, CP, HH, CY, FX
 *   - 112 Sovereign Trademark-Stripped Synthesis Models (16 per role)
 *   - Hardwired Permanent 1V/Oct Chromatic Key Tracking
 *   - Dual-Slot Anti-Pop Ping-Pong Voice Core (1.5ms linear de-click crossfade)
 *   - 32-Bit Latched PRNG Humanization Engine (Free Analog Drift vs. Locked Loop16)
 *   - Multiplicative Log-Domain PCHIP Hermite Spline Macro Decay (g(0.5)=1.0 Neutral)
 *   - Dedicated Sub-Bass Sine Layering & Resonant 80Hz Low-Boost Thump
 *   - 4-Axis Timbre Lab (Ratio X, FM Index Y, Wavefold Z, Timbre Settle T)
 *   - Zero-Latency Delta Crossover Mojo Strip with Sub-Bass Defense (LR4 100Hz HP)
 *   - 100% Real-Time Audio Safe: Zero heap allocations, zero locks, denormal-free.
 */
class FormlessSpawnVoice {
public:
    enum class Role : int {
        BD = 0,
        SD = 1,
        TM = 2,
        CP = 3,
        HH = 4,
        CY = 5,
        PC = 6,
        FX = 7
    };

    enum class LifeMode : int {
        Synthetic = 0,
        Cyborg    = 1,
        Organic   = 2
    };

    static constexpr int kTotalRoles = 8;
    static constexpr int kModelsPerRole = 8;
    static constexpr int kTotalModels = 64;

    static constexpr const char* kRoleNames[kTotalRoles] = {
        "BD", "SD", "TM", "CP", "HH", "CY", "PC", "FX"
    };

    static constexpr const char* kModelNames[kTotalRoles][kModelsPerRole] = {
        // BD (0..7)
        {"ANLG-9", "BOOM", "FM-2", "W-FOLD", "MODAL", "CHIRP", "DUAL-V", "GABBER"},
        // SD (0..7)
        {"ANLG-9", "ANLG-8", "FM-SD", "MODAL", "VCF-S", "DIGI-S", "SHELL", "EXPLOD"},
        // TM (0..7)
        {"ANLG-9", "ANLG-8", "HEXCESS", "MODAL", "PING-T", "FM-TOM", "W-FOLD", "FORMNT"},
        // CP (0..7)
        {"ANLG-9", "ANLG-8", "HANDS", "CP-BRST", "FM-CLAP", "W-CLP", "GRAIN", "COMB"},
        // HH (0..7)
        {"SCHMITT", "DIODE", "FM-GLS", "FM-GRT", "PLATE", "CHIME", "NOISE", "RING-M"},
        // CY (0..7)
        {"SCHMITT", "DIODE", "FM-CR", "FM-RD", "BRONZE", "GONG", "DUST", "CHIME"},
        // PC (0..7)
        {"RIM-ST", "ANLG-CB", "CLAVE", "SHAKR", "CONGA", "ANVIL", "FM-PC", "SCRAP"},
        // FX (0..7)
        {"LASER", "SUB-DP", "FM-MRP", "W-FOLD", "RING-M", "PD-ZAP", "CHAOS", "FLANGE"}
    };

    static constexpr const char* kRoleParamNames[kTotalRoles][8] = {
        // BD
        {"P.CURVE", "P.DEC", "SWEEP.D", "DECAY", "CLICK", "TIMBRE", "DRIVE", "PUNCH"},
        // SD
        {"SHELL", "P.DEC", "SNAPPY", "S.DECAY", "ATTACK", "TONE", "NOISE", "SNAP"},
        // TM
        {"P.CURVE", "BEND.T", "INTERVAL", "DECAY", "STICK", "TENSION", "RING", "BODY"},
        // CP
        {"PRE-TIME", "SPREAD", "BURST", "TAIL", "FLAM", "COLOR", "DIFFUSE", "CRACK"},
        // HH
        {"CLUSTER", "CHOKE", "METRIC", "DECAY", "METALLIC", "BRIGHT", "SIZZLE", "BITE"},
        // CY
        {"ALLOY", "SHIMMER", "DAMP", "SUSTAIN", "MALLET", "DENSITY", "BELL", "CRASH"},
        // PC
        {"DISPERSE", "SWEEP", "RATIO", "DECAY", "IMPACT", "MATERIAL", "RING", "POP"},
        // FX
        {"WARP", "SPEED", "SWEEP", "DECAY", "TRIGGER", "INDEX", "CHAOS", "EDGE"}
    };

    static constexpr float kRoleDefaultHz[kTotalRoles] = {
        55.0f,   // BD
        180.0f,  // SD
        110.0f,  // TM
        800.0f,  // CP
        6000.0f, // HH
        4000.0f, // CY
        440.0f,  // PC
        220.0f   // FX
    };

    // =========================================================================
    // 64-Model Calibrated Macro Decay Matrix (ACF-5 Calibrated)
    // =========================================================================
    static constexpr float kModelBaseAmpDecaySec[kTotalRoles][kModelsPerRole] = {
        // BD: ANLG-9 BOOM   FM-2   W-FOLD MODAL  CHIRP  DUAL-V GABBER
        {      0.110f,0.280f,0.095f,0.100f,0.140f,0.060f,0.150f,0.130f },
        // SD: ANLG-9 ANLG-8 FM-SD  MODAL  VCF-S  DIGI-S SHELL  EXPLOD
        {      0.030f,0.045f,0.035f,0.040f,0.038f,0.032f,0.048f,0.055f },
        // TM: ANLG-9 ANLG-8 HEXCES MODAL  PING-T FM-TOM W-FOLD FORMNT
        {      0.300f,0.260f,0.240f,0.420f,0.380f,0.220f,0.280f,0.320f },
        // CP: ANLG-9 ANLG-8 HANDS  CP-BRS FM-CLP W-CLP  GRAIN  COMB
        {      0.140f,0.180f,0.220f,0.260f,0.120f,0.150f,0.240f,0.200f },
        // HH: SCHMIT DIODE  FM-GLS FM-GRT PLATE  CHIME  NOISE  RING-M
        {      0.050f,0.070f,0.080f,0.040f,0.120f,0.150f,0.060f,0.090f },
        // CY: SCHMIT DIODE  FM-CR  FM-RD  BRONZE GONG   DUST   CHIME
        {      1.100f,1.600f,1.200f,1.000f,1.400f,1.800f,0.700f,1.500f },
        // PC: RIM-ST ANLG-C CLAVE  SHAKR  CONGA  ANVIL  FM-PC  SCRAP
        {      0.060f,0.180f,0.080f,0.120f,0.250f,0.400f,0.140f,0.220f },
        // FX: LASER  SUB-DP FM-MRP W-FOLD RING-M PD-ZAP CHAOS  FLANGE
        {      0.200f,0.600f,0.300f,0.250f,0.320f,0.140f,0.550f,0.420f }
    };

    static constexpr float kModelPitchDecayResponse[kTotalRoles][kModelsPerRole] = {
        // BD
        {      0.08f, 0.12f, 0.10f, 0.08f, 0.15f, 0.00f, 0.10f, 0.05f },
        // SD
        {      0.10f, 0.05f, 0.15f, 0.15f, 0.10f, 0.05f, 0.08f, 0.35f },
        // TM
        {      0.30f, 0.30f, 0.25f, 0.55f, 0.40f, 0.25f, 0.30f, 0.20f },
        // CP
        {      0.00f, 0.00f, 0.00f, 0.10f, 0.10f, 0.00f, 0.00f, 0.00f },
        // HH
        {      0.00f, 0.00f, 0.05f, 0.00f, 0.00f, 0.15f, 0.00f, 0.10f },
        // CY
        {      0.05f, 0.10f, 0.15f, 0.00f, 0.05f, 0.35f, 0.00f, 0.05f },
        // PC
        {      0.00f, 0.05f, 0.00f, 0.00f, 0.20f, 0.10f, 0.15f, 0.05f },
        // FX
        {      1.00f, 0.95f, 0.60f, 0.00f, 0.40f, 1.00f, 0.35f, 0.25f }
    };

    static constexpr float kModelTimbreDecayResponse[kTotalRoles][kModelsPerRole] = {
        // BD
        {      0.50f, 0.30f, 0.70f, 0.75f, 0.60f, 0.10f, 0.55f, 0.40f },
        // SD
        {      0.35f, 0.85f, 0.65f, 0.45f, 0.70f, 0.35f, 0.60f, 0.55f },
        // TM
        {      0.30f, 0.50f, 0.75f, 0.90f, 0.40f, 0.70f, 0.80f, 0.60f },
        // CP
        {      0.30f, 0.35f, 0.85f, 0.55f, 0.60f, 0.25f, 0.70f, 0.45f },
        // HH
        {      0.30f, 0.30f, 0.55f, 0.25f, 0.40f, 0.35f, 0.35f, 0.50f },
        // CY
        {      0.60f, 0.70f, 0.80f, 0.75f, 0.50f, 0.85f, 0.30f, 0.70f },
        // PC
        {      0.20f, 0.40f, 0.25f, 0.60f, 0.45f, 0.70f, 0.65f, 0.50f },
        // FX
        {      0.70f, 0.40f, 0.85f, 1.00f, 0.75f, 0.45f, 0.70f, 0.65f }
    };

    using Params = FormlessSpawnParams;

    FormlessSpawnVoice() noexcept {
        reset();
    }

    void prepare(float sampleRate) noexcept {
        sampleRate_ = std::max(sampleRate, 4000.0f);
        invSampleRate_ = 1.0f / sampleRate_;
        nyquist_ = 0.495f * sampleRate_;
        squashStage_.prepare(sampleRate_);
        widerStage_.prepare(sampleRate_);
        drumLimiter_.prepare(sampleRate_);
        freqShifter_.prepare(sampleRate_);
        masterSmooth_ = 1.0f - std::exp(-invSampleRate_ / 0.005f); // 5ms master volume smoother

        // Cyborg analog bus 17.5 kHz 1-pole filter alpha
        const float fcBus = 17500.0f;
        const float wBus = 2.0f * 3.14159265358979323846f * fcBus * invSampleRate_;
        cyborgBusAlpha_ = std::clamp(wBus / (1.0f + wBus), 0.01f, 0.99f);

        // Cyborg DAC reconstruction filter alpha (0.15ms)
        dacAlpha_ = 1.0f - std::exp(-invSampleRate_ / 0.000035f); // 35us snappy DAC slew

        // 10ms master activity gate release coefficient and control-rate smoothing
        kGateRelease10ms_ = FastMath::fastExp(-invSampleRate_ / 0.010f);
        ampSmoothAlpha_ = 1.0f - std::exp(-invSampleRate_ * 16.0f / 0.003f);
        wireBloomAlpha_ = 1.0f - std::exp(-invSampleRate_ / 0.0004f);

        reset();
    }

    void reset() noexcept {
        slotA_ = VoiceSlot{};
        slotB_ = VoiceSlot{};
        activeSlot_ = 0;
        fadeRemaining_ = 0;
        fadeTotal_ = 64;

        ampDecaySmoothed_ = 1.0f;
        masterAmpEnv_ = 0.0f;

        vcfSvf_.reset();
        lpgVactrolState_ = 0.0f;
        lpgFilterState_ = 0.0f;
        dcBlockerState_ = 0.0f;

        voiceResidualMem_ = 0.0f;
        adaaPreDriveX1_ = 0.0f;
        adaaPreDriveF1_ = 0.0f;
        adaaThumpX1_ = 0.0f;
        adaaThumpF1_ = 0.0f;
        adaaFoldX1_ = 0.0f;
        decayCurveActive_ = false;
        decayCurveExp_ = 1.0f;
        sampleCounter_ = 0;
        dacTargetPitch_ = 1.0f;
        dacSlewedPitch_ = 1.0f;
        cyborgBleedEnv_ = 0.0f;
        cyborgBusFilterState_ = 0.0f;
        cyborgChargeSpike_ = 0.0f;
        vtlSvfS1_ = 0.0f;
        vtlSvfS2_ = 0.0f;
        organicDcBlock_ = 0.0f;

        hp1_s1_ = 0.0f; hp1_s2_ = 0.0f;
        hp2_s1_ = 0.0f; hp2_s2_ = 0.0f;
        mojoDcBlocker_ = 0.0f;
        crushPhase_ = 0.0f;
        crushedSample_ = 0.0f;

        rngState_ = 0xDEADBEEF;
        panPhase_ = 0.0f;
        blockCounter_ = 0;
        macroSmoothed_ = 0.5f;

        squashStage_.reset();
        widerStage_.reset();
        drumLimiter_.reset();
        freqShifter_.reset();
        adaaDcBlocker_ = 0.0f;

        activeRole_ = pendingRole_;
        activeModel_ = pendingModel_;
        masterGain_ = masterGainTarget_;
        masterGainInit_ = false;
        masterAmpEnv_ = 0.0f;
        masterFilterEnv_ = 0.0f;
    }

    [[nodiscard]] inline float getLimiterGainReduction() const noexcept {
        return drumLimiter_.getGainReduction();
    }

    void setRole(int roleIdx) noexcept {
        pendingRole_ = std::clamp(roleIdx, 0, kTotalRoles - 1);
    }

    void setModel(int modelIdx) noexcept {
        pendingModel_ = std::clamp(modelIdx, 0, kModelsPerRole - 1);
    }

    void trigger(float velocity, float noteHz = -1.0f, const Params& p = Params{}) noexcept {
        velocity = std::clamp(velocity, 0.0f, 1.0f);
        if (velocity <= 0.0001f) return;

        // 1. Latch Pending Role & Model on Note-On
        activeRole_  = pendingRole_;
        activeModel_ = pendingModel_;

        // 2. Advance Trigger PRNG and Latch 8-Axis Humanization Parameters
        const float d[8] = {
            p.randX * 0.01f,
            p.randY * 0.01f,
            p.randZ * 0.01f,
            p.randT * 0.01f,
            p.randAggr * 0.01f,
            p.randTimb * 0.01f,
            p.randPan * 0.01f,
            p.randDecay * 0.01f
        };

        for (int i = 0; i < 8; ++i) {
            latchedDraw_[i] = nextTrig01();
        }

        // Golden-ratio quasi-Monte Carlo stratification for random pan (prevents stereo clumping)
        panPhase_ = std::fmod(panPhase_ + 0.6180339887f, 1.0f);
        if (d[6] > 0.0001f) {
            latchedDraw_[6] = panPhase_;
        }

        // 3. Ping-Pong Voice Allocation (1.5ms Linear Slew De-Click)
        activeSlot_ = 1 - activeSlot_;
        auto& newSlot = (activeSlot_ == 0) ? slotA_ : slotB_;

        fadeTotal_ = std::max(16, static_cast<int>(0.0015f * sampleRate_));
        fadeRemaining_ = fadeTotal_;

        // 4. Initialize New Slot with Latched Offsets and Role Identity
        newSlot.active = true;
        newSlot.role = activeRole_;
        newSlot.model = activeModel_;
        newSlot.velocity = velocity;
        newSlot.ampEnv = 1.0f;
        newSlot.pitchEnv = 1.0f;
        newSlot.clickEnv = 1.0f;
        newSlot.timbreEnv = 1.0f;
        newSlot.burstCount = 0.0f;
        newSlot.thumpPhase = 0.0f;
        newSlot.thumpEnv = 1.0f;
        newSlot.adaaDiodeX1 = 0.0f;
        newSlot.adaaFoldX1 = 0.0f;
        newSlot.organicDcBlock = 0.0f;
        newSlot.exciterRng = 0xCAFEBABE ^ (static_cast<uint32_t>(velocity * 100000.0f) + 1);

        for (int i = 0; i < 8; ++i) {
            newSlot.randVal[i] = latchedDraw_[i];
            newSlot.randAmt[i] = d[i];
        }

        // Guarded 1.8ms Hold Plateau (capped at 25% of decay time)
        const int maxHold = static_cast<int>(0.0018f * sampleRate_);
        const int capHold = static_cast<int>(0.25f * effAmpDecaySec_ * sampleRate_);
        newSlot.holdSamples = (velocity > 0.0f) ? std::min(maxHold, capHold) : 0;
        newSlot.wireBloomEnv = 0.0f;

        const float effDecay = CrackedChimes::DSP::stepRandomNorm(p.macroDecay, d[7], newSlot.randVal[7]);
        const float hitMacroMult = evalMacroMultiplier(effDecay);
        const float baseMacroMult = evalMacroMultiplier(p.macroDecay);
        const float velMult = 1.0f + (velocity - 0.70f) * p.velAggr * 0.5f;
        const float safeVelScale = std::clamp(velMult, 1.0f, 1.5f);
        newSlot.latchedDecayScale = ((baseMacroMult > 1e-4f) ? std::clamp(hitMacroMult / baseMacroMult, 0.25f, 4.0f) : 1.0f) * safeVelScale;
        newSlot.invDScale = 1.0f / newSlot.latchedDecayScale;
        updateSlotControlParams(newSlot, p);

        blockCounter_ = 0; // Force immediate coefficient update

        // Base Frequency Calculation per Role:
        const float nominalHz = (noteHz > 10.0f) ? noteHz : kRoleDefaultHz[activeRole_];

        // Lifeform Phase Reset & Acoustic Initial Conditions at t=0
        if (p.lifeMode == static_cast<int>(LifeMode::Cyborg)) {
            newSlot.phase = 0.25f + 0.1f * nextTrig01(); // Phase jitter
            newSlot.modPhase = 0.5f;
            newSlot.subPhase = 0.0f;
            newSlot.residualMem = 0.0f;
            newSlot.beaterState = 0.0f;
            newSlot.baseHz = std::clamp(nominalHz, 15.0f, nyquist_);
            cyborgChargeSpike_ = 1.0f; // 20us charge-injection pulse
            cyborgBleedEnv_ = 1.0f;    // Engage activity-gated bleed
        } else if (p.lifeMode == static_cast<int>(LifeMode::Organic)) {
            newSlot.residualMem = voiceResidualMem_; // Inherit residual membrane energy
            voiceResidualMem_ = velocity;           // Latch new strike energy
            newSlot.phase = 0.005f * ((velocity > 0.5f) ? 1.0f : -1.0f);
            newSlot.modPhase = 0.0f;
            newSlot.subPhase = 0.0f;
            newSlot.beaterState = 0.0f;
            // Thermal micro-drift (+/- 1.5 cents)
            const float driftCents = nextTrigBipolar() * 1.5f;
            newSlot.baseHz = std::clamp(nominalHz * FastMath::fastPow2(driftCents * (1.0f / 1200.0f)), 15.0f, nyquist_);
        } else {
            // Synthetic: Pristine deterministic reset
            newSlot.phase = 0.0f;
            newSlot.modPhase = 0.0f;
            newSlot.subPhase = 0.0f;
            newSlot.residualMem = 0.0f;
            newSlot.beaterState = 0.0f;
            newSlot.baseHz = std::clamp(nominalHz, 15.0f, nyquist_);
        }
        for (auto& ph : newSlot.clusterPhases) ph = 0.0f;

        // Trigger Master Output Envelopes
        masterAmpEnv_ = 1.0f;
        masterFilterEnv_ = 1.0f;
    }

    void processBlockStereo(float* outL, float* outR, int numSamples, const Params& p) noexcept {
        masterGainTarget_ = tkdsp::formless::evalMasterVolumeGain(p.volume);
        if (!masterGainInit_) {
            masterGain_ = masterGainTarget_;
            masterGainInit_ = true;
        }

        for (int i = 0; i < numSamples; ++i) {
            ++sampleCounter_;
            if (--blockCounter_ <= 0) {
                blockCounter_ = 16;
                updateControlRateCoefficients(p);
            }

            renderSampleStereo(outL[i], outR[i], p);
        }
    }

    [[nodiscard]] const std::array<float, 8>& getLatchedDraws() const noexcept { return latchedDraw_; }

private:
    struct VoiceSlot {
        int role{0};
        int model{0};
        float phase{0.0f};
        float modPhase{0.0f};
        float subPhase{0.0f};
        float clickPhase{0.0f};
        std::array<float, 6> clusterPhases{{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};
        float ampEnv{0.0f};
        float pitchEnv{0.0f};
        float clickEnv{0.0f};
        float timbreEnv{0.0f};
        float burstCount{0.0f};
        float baseHz{55.0f};
        float velocity{1.0f};
        float latchedDecayScale{1.0f};
        float invDScale{1.0f};
        int   holdSamples{0};
        float wireBloomEnv{0.0f};
        std::array<float, 8> randVal{{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};
        std::array<float, 8> randAmt{{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};
        float panL{0.7071f};
        float panR{0.7071f};
        float thumpPhase{0.0f};
        float thumpEnv{0.0f};
        uint32_t exciterRng{0xCAFEBABE};
        float residualMem{0.0f};
        float beaterState{0.0f};
        float adaaDiodeX1{0.0f};
        float adaaDiodeF1{0.0f};
        float adaaFoldX1{0.0f};
        float adaaFoldF1{0.0f};
        float organicDcBlock{0.0f};
        bool  active{false};

        // Precomputed control-rate hoisted parameters (Win #3, #4, #5)
        float pitchDecayFactor{0.99f};
        float ampDecayFactor{0.99f};
        float timbreDecayFactor{0.99f};
        float eA{0.5f}, eB{0.5f}, eC{0.5f}, eD{0.5f};
        float eE{0.5f}, eF{0.5f}, eG{0.5f}, eH{0.5f};
        float aggrDrive{1.0f};
        float pitchDepthSemis{0.0f};
        float pCurveExp{1.0f};
    };

    // =========================================================================
    // Multiplicative Log-Domain PCHIP Spline Evaluator
    // =========================================================================
    static inline float evalMacroMultiplier(float macro) noexcept {
        return tkdsp::formless::evalMacroMultiplier(macro);
    }

    void updateSlotControlParams(VoiceSlot& slot, const Params& p) noexcept {
        if (!slot.active) return;

        // 1. Effective Macro & Synthesis Parameters with Stepped Random Lerp
        const float effX    = CrackedChimes::DSP::stepRandomNorm(p.macroX, slot.randAmt[0], slot.randVal[0]);
        const float effY    = CrackedChimes::DSP::stepRandomNorm(p.macroY, slot.randAmt[1], slot.randVal[1]);
        const float effZ    = CrackedChimes::DSP::stepRandomNorm(p.macroZ, slot.randAmt[2], slot.randVal[2]);
        const float effT    = CrackedChimes::DSP::stepRandomNorm(p.macroT, slot.randAmt[3], slot.randVal[3]);
        const float effAggr = CrackedChimes::DSP::stepRandomNorm(p.velAggr, slot.randAmt[4], slot.randVal[4]);
        const float effTimb = CrackedChimes::DSP::stepRandomNorm(p.velTimb, slot.randAmt[5], slot.randVal[5]);

        const float velForce = slot.velocity;

        // Multi-Parameter Vector Fan-Out with Headroom-Proportional Lerp & Velocity Dynamics across ALL Roles
        slot.eA = tkdsp::formless::headroomLerp(p.paramA, effX, 0.85f);
        slot.eB = tkdsp::formless::headroomLerp(p.paramB, effT, 0.70f);
        slot.eC = tkdsp::formless::headroomLerp(p.paramC, effX, 0.75f);
        slot.eD = tkdsp::formless::headroomLerp(p.paramD, effT, 0.70f);

        // Y (Punch) & Z (Timbre) -> Knobs E, F, G, H with Sequential Velocity Arbitration
        slot.eE = tkdsp::formless::sequentialArbitrate(p.paramE, effY, 0.80f, effTimb, 0.65f, velForce);
        slot.eF = tkdsp::formless::sequentialArbitrate(p.paramF, effZ, 0.70f, effTimb, 0.70f, velForce);
        slot.eG = tkdsp::formless::sequentialArbitrate(p.paramG, effZ, 0.65f, effAggr, 0.70f, velForce);
        slot.eH = tkdsp::formless::sequentialArbitrate(p.paramH, effY, 0.75f, effAggr, 0.75f, velForce);

        // Dynamic Velocity Routing (aggrDrive)
        slot.aggrDrive = std::clamp(1.0f + (velForce - 0.5f) * effAggr * 1.2f, 0.25f, 2.2f);

        // Equal-Power Sinusoidal Pan Law (L^2 + R^2 == 1.0) with Stepped Randomization
        const float effPan = std::clamp(p.pan + (slot.randVal[6] - 0.5f) * 2.0f * slot.randAmt[6], -1.0f, 1.0f);
        const float initAngle = (effPan + 1.0f) * 0.25f * 3.14159265358979323846f;
        slot.panL = std::cos(initAngle);
        slot.panR = std::sin(initAngle);

        // Role-Masked Pitch Sweep & Transient Snap (Knob C isolation)
        switch (slot.role) {
            case 0: // BD: Full pitch sweep depth from Knob C (SWEEP.D)
                slot.pitchDepthSemis = tkdsp::formless::evalPitchDepthSemis(slot.eC);
                slot.pCurveExp = FastMath::fastPow2((slot.eA - 0.5f) * 2.0f); // P.CURVE
                break;

            case 2: // TM: Pitch bend / interval from Knob C (INTERVAL)
                slot.pitchDepthSemis = slot.eC * 36.0f; // 0..36 semitones
                slot.pCurveExp = FastMath::fastPow2((slot.eA - 0.5f) * 2.0f); // BEND.T
                break;

            case 1: // SD: Subtle body snap from Knob B (P.DEC), NOT Knob C!
                slot.pitchDepthSemis = slot.eB * 3.5f; // At most 3.5 semitones shell transient snap
                slot.pCurveExp = 2.0f;                 // Fast exponential snap
                break;

            case 7: // FX: Experimental sweep from Knob C (SWEEP)
                slot.pitchDepthSemis = (slot.eC - 0.5f) * 48.0f; // Bipolar +/- 24 semitones
                slot.pCurveExp = 1.0f;
                break;

            default: // CP (3), HH (4), CY (5), PC (6): ZERO global pitch dive!
                slot.pitchDepthSemis = 0.0f;
                slot.pCurveExp = 1.0f;
                break;
        }

        // Precompute slot decay factors per sample (Win #3: replaces 3 transcendentals per sample with 3 fmul ops)
        slot.pitchDecayFactor  = std::clamp(FastMath::fastPow2(FastMath::fastLog2(pitchDecayCoeff_) * slot.invDScale), 0.80f, 0.9999995f);
        slot.ampDecayFactor    = FastMath::fastPow2(log2AmpDecayCoeff_ * slot.invDScale);
        slot.timbreDecayFactor = std::clamp(FastMath::fastPow2(FastMath::fastLog2(timbreDecayCoeff_) * slot.invDScale), 0.80f, 0.9999995f);
    }

    void updateControlRateCoefficients(const Params& p) noexcept {
        macroSmoothed_ += 0.08f * (p.macroDecay - macroSmoothed_);
        ampDecaySmoothed_ += ampSmoothAlpha_ * (p.ampDecay - ampDecaySmoothed_);
        const float macroMult = evalMacroMultiplier(macroSmoothed_);

        const float macroLog2 = FastMath::fastLog2(std::max(macroMult, 1e-6f));

        const int r = std::clamp(activeRole_,  0, kTotalRoles - 1);
        const int m = std::clamp(activeModel_, 0, kModelsPerRole - 1);

        const float pitchResp  = kModelPitchDecayResponse[r][m];
        const float timbreResp = kModelTimbreDecayResponse[r][m];

        const float pitchMacro  = FastMath::fastPow2(pitchResp  * macroLog2);
        const float timbreMacro = FastMath::fastPow2(timbreResp * macroLog2);

        // Time macro (macroT) scales decay times
        const float tMult = evalMacroMultiplier(p.macroT);

        // Base amp decay scaled by Macro Decay, Time Macro, and Knob D (Body Decay)
        const float baseAmpSec = kModelBaseAmpDecaySec[r][m];
        const float bodyDecayMult = tkdsp::formless::evalBodyDecayMult(p.paramD);
        const float lEngine = tkdsp::formless::evalEngineLengthSec(baseAmpSec, bodyDecayMult, macroMult, tMult);
        effAmpDecaySec_ = tkdsp::formless::evalDynamicAmpDecaySec(ampDecaySmoothed_, lEngine, 0.005f);

        // Exact analytical log2 coefficient (Claude SEV-1 Fix: zero fastLog2/fastExp error)
        log2AmpDecayCoeff_ = -invSampleRate_ * 1.4426950408889634f / effAmpDecaySec_;
        ampDecayCoeff_    = FastMath::fastExp(-invSampleRate_ / effAmpDecaySec_);

        // Pitch sweep decay scaled by Knob B (Pitch Decay) and Time Macro
        const float basePitchDecaySec = tkdsp::formless::evalPitchDecaySec(p.paramB);
        float effPitchDecaySec = std::clamp(basePitchDecaySec * pitchMacro * tMult, 0.0005f, 3.0f);
        // Pitch-Tuck Invariant: Ensure dive always lands before note dies (tau_pitch <= 0.45 * tau_amp)
        effPitchDecaySec = std::min(effPitchDecaySec, effAmpDecaySec_ * 0.45f);
        pitchDecayCoeff_  = FastMath::fastExp(-invSampleRate_ / effPitchDecaySec);

        // Timbre decay
        const float effTimbreDecaySec = std::clamp(std::min(0.040f * timbreMacro * tMult, effAmpDecaySec_ * 0.85f), 0.002f, 1.500f);
        const float effClickDecaySec  = std::clamp(0.002f * FastMath::fastPow2(macroSmoothed_ * 1.2f), 0.0008f, 0.006f);

        timbreDecayCoeff_ = FastMath::fastExp(-invSampleRate_ / effTimbreDecaySec);
        clickDecayCoeff_  = FastMath::fastExp(-invSampleRate_ / effClickDecaySec);

        // Master Filter Envelope Decay Constant
        const float effFilterDecaySec = std::clamp(p.filterDecay, 0.005f, 5.0f);
        masterFilterDecayCoeff_ = FastMath::fastExp(-invSampleRate_ / effFilterDecaySec);

        // Dynamic Filter Envelope Modulation (+/-5 Octaves)
        const float envOctaves = std::clamp(p.filterEnvAmt * 5.0f * masterFilterEnv_, -6.0f, 6.0f);
        float dynCutoffHz = p.cutoff * FastMath::fastPow2(envOctaves);

        if (p.lifeMode == static_cast<int>(LifeMode::Organic)) {
            // Role-calibrated Optocoupler Conductance Hugging (HH/CY High-Frequency Air Defense)
            const float s = std::clamp(lpgVactrolState_, 0.001f, 1.0f);
            const int roleIdx = std::clamp(activeRole_, 0, 7);
            const float floorK = tkdsp::formless::kRoleLpgCutoffFloor[roleIdx];
            // Linear conductance response for metallic roles (HH/CY) to prevent sudden cliff; quadratic for drum membranes
            const float conductance = (roleIdx == 4 || roleIdx == 5) ? s : (s * s);
            dynCutoffHz *= (floorK + (1.0f - floorK) * conductance);
        }

        const float rawCutoff = std::clamp(dynCutoffHz, 20.0f, nyquist_);
        if (p.filterType == 3) {
            // PK: Peaking Parametric Bell EQ (-48dB .. 0dB noon .. +24dB)
            const float gainDb = tkdsp::formless::evalPeakingGainDb(p.resonance);
            vcfSvf_.setBell(rawCutoff, 2.0f, gainDb, invSampleRate_);
        } else {
            // LP / HP / BP: Resonant Q (0.50 .. 25.00)
            const float q = tkdsp::formless::evalCytomicQ(p.resonance);
            switch (p.filterType) {
                case 0: vcfSvf_.setLowpass(rawCutoff, q, invSampleRate_); break;
                case 1: vcfSvf_.setHighpass(rawCutoff, q, invSampleRate_); break;
                case 2: vcfSvf_.setBandpass(rawCutoff, q, invSampleRate_); break;
                default: vcfSvf_.setLowpass(rawCutoff, q, invSampleRate_); break;
            }
        }

        constexpr float kCrossoverHz = 100.0f;
        const float normXover = kCrossoverHz * invSampleRate_;
        hpG_ = std::tan(normXover * 3.1415926535f);
        hpH_ = 1.0f / (1.0f + 1.41421356f * hpG_ + hpG_ * hpG_);

        // Win #4: Hoist decay curve exponent calculation to control rate
        decayCurveActive_ = (std::abs(p.decayCurve) > 0.01f);
        if (decayCurveActive_) {
            decayCurveExp_ = FastMath::fastPow2(-p.decayCurve * 1.5f);
        }

        // Win #5: Hoist slot parameter fan-out and decay factors to control rate
        updateSlotControlParams(slotA_, p);
        updateSlotControlParams(slotB_, p);
    }

    inline float renderSlotSample(VoiceSlot& slot, const Params& p) noexcept {
        if (!slot.active || slot.ampEnv < 0.00002f) {
            slot.active = false;
            return 0.0f;
        }

        // Win #5: Use precomputed control-rate hoisted parameters
        const float eA = slot.eA;
        const float eB = slot.eB;
        const float eC = slot.eC;
        const float eD = slot.eD;
        const float eE = slot.eE;
        const float eF = slot.eF;
        const float eG = slot.eG;
        const float eH = slot.eH;
        const float pitchDepthSemis = slot.pitchDepthSemis;
        const float pCurveExp = slot.pCurveExp;
        const float velForce = slot.velocity;

        const float shapedPitchEnv = (std::abs(pCurveExp - 1.0f) > 0.005f)
            ? FastMath::fastPow2(FastMath::fastLog2(std::max(slot.pitchEnv, 1e-6f)) * pCurveExp)
            : slot.pitchEnv;

        float pitchMult = (std::abs(pitchDepthSemis) > 0.001f)
            ? FastMath::fastPow2((pitchDepthSemis * shapedPitchEnv) * (1.0f / 12.0f))
            : 1.0f;

        // Lifeform Pitch Dynamics:
        // A. Organic: Role-Masked Dynamic Membrane Stiffening & Pitch Droop (Tekno Law)
        float droopMult = 1.0f;
        if (p.lifeMode == static_cast<int>(LifeMode::Organic)) {
            const float droopK = tkdsp::formless::kRoleDroopFactor[slot.role];
            droopMult = 1.0f + droopK * (slot.ampEnv * slot.ampEnv);
        }

        // B. Cyborg: Reconstructed 16-sample 6-bit DAC Staircase Dive (snappy 35us slew)
        if (p.lifeMode == static_cast<int>(LifeMode::Cyborg) && std::abs(pitchDepthSemis) > 0.001f) {
            if ((sampleCounter_ & 15) == 0) {
                dacTargetPitch_ = std::round(shapedPitchEnv * 64.0f) * (1.0f / 64.0f);
            }
            dacSlewedPitch_ += dacAlpha_ * (dacTargetPitch_ - dacSlewedPitch_);
            pitchMult = FastMath::fastPow2((pitchDepthSemis * dacSlewedPitch_) * (1.0f / 12.0f));
        }

        const float curFreq = std::clamp(slot.baseHz * pitchMult * droopMult, 10.0f, nyquist_);

        // Advance Recursive Envelopes (Win #3: 3 single-cycle fmul ops replace transcendentals)
        slot.pitchEnv *= slot.pitchDecayFactor;
        if (slot.holdSamples > 0) {
            --slot.holdSamples;
        } else {
            slot.ampEnv *= slot.ampDecayFactor;
        }
        slot.timbreEnv *= slot.timbreDecayFactor;
        slot.clickEnv  *= clickDecayCoeff_;
        slot.wireBloomEnv += (1.0f - slot.wireBloomEnv) * wireBloomAlpha_;

        // Decay Curvature Shaping on Amplitude (Win #4: uses precomputed decayCurveExp_)
        float shapedAmp = slot.ampEnv;
        if (decayCurveActive_) {
            shapedAmp = FastMath::fastPow2(FastMath::fastLog2(std::max(slot.ampEnv, 1e-6f)) * decayCurveExp_);
        }
        // Smoothstep fade on final threshold to prevent -35dB DC pop (Argon Flaw 1.3 Fix)
        if (slot.ampEnv < 0.0001f) {
            const float t = std::clamp((slot.ampEnv - 0.00001f) * (1.0f / 0.00009f), 0.0f, 1.0f);
            shapedAmp *= (t * t * (3.0f - 2.0f * t));
        }

        // Advance Main Oscillator Phase
        slot.phase += curFreq * invSampleRate_;
        if (slot.phase >= 1.0f) slot.phase -= 1.0f;

        const float origPhase = slot.phase;
        if (p.lifeMode == static_cast<int>(LifeMode::Cyborg)) {
            // Phase distortion resonant wave-wrap on effective phase
            const float czPhase = (origPhase < 0.5f)
                ? 2.0f * origPhase * origPhase
                : 1.0f - 2.0f * (1.0f - origPhase) * (1.0f - origPhase);
            // Calibrate warp depth: BD/TM preserve solid fundamental core (35% warp), SD/CP/HH/CY/PC/FX get sharp bite (80% warp)
            const float czDepth = (slot.role == 0 || slot.role == 2) ? 0.35f : 0.80f;
            slot.phase = (1.0f - czDepth) * origPhase + czDepth * czPhase;
        }

        // 3. Synthesis Core Dispatch (8 Roles x 8 Models = 64 Sovereign Models)
        float oscOut = 0.0f;

        switch (slot.role) {
            case 0: { // BD (Bass Drum)
                switch (slot.model) {
                    case 0: { // ANLG-9: Bridged-T punchy analog sine with 2nd-harmonic asymmetry
                        float s = FastMath::fastSinNorm(slot.phase);
                        float asym = s + eG * 0.35f * s * s;
                        oscOut = FastMath::fastTanh(asym * (1.1f + eF * 2.2f + eH * 1.6f));
                        break;
                    }
                    case 1: { // BOOM: Deep resonant sub-bass boom with saturation and 2nd harmonic presence
                        float sub2 = FastMath::fastSinNorm(slot.phase * 2.0f) * (eG * 0.35f);
                        float s = FastMath::fastSinNorm(slot.phase) * (1.0f - 0.05f * slot.phase) + sub2;
                        float sat = 1.0f + eF * 2.2f;
                        float punch = 1.0f + eH * 1.2f;
                        oscOut = FastMath::fastTanh(s * (sat * punch));
                        break;
                    }
                    case 2: { // FM-2: 2-op metallic FM kick with attack transient index scaling
                        float mF = curFreq * (0.75f + eF * 3.75f);
                        slot.modPhase += mF * invSampleRate_;
                        if (slot.modPhase >= 1.0f) slot.modPhase -= 1.0f;
                        float effIndex = (eG * 3.2f) + (eH * 2.5f * slot.timbreEnv);
                        float mod = FastMath::fastSinNorm(slot.modPhase) * effIndex;
                        oscOut = FastMath::fastSinNorm(slot.phase + mod * 0.159f);
                        break;
                    }
                    case 3: { // W-FOLD: Perkons triangle wavefold kick with asymmetry and attack fold burst
                        float tri = (slot.phase < 0.5f) ? (4.0f * slot.phase - 1.0f) : (3.0f - 4.0f * slot.phase);
                        tri += eG * 0.35f; // Asymmetry bias creates warm even harmonics
                        float foldDepth = (1.0f + eF * 4.5f) + (eH * 3.0f * slot.timbreEnv);
                        oscOut = FastMath::fastSinNorm(tri * foldDepth);
                        break;
                    }
                    case 4: { // MODAL: Acoustic inharmonic membrane kick with damped mode (1.593x) and mallet hardness
                        float s1 = FastMath::fastSinNorm(slot.phase);
                        float mode2 = FastMath::fastSinNorm(slot.phase * (1.593f + eG * 0.4f)) * slot.timbreEnv;
                        oscOut = FastMath::fastTanh((s1 + mode2 * (0.25f + eF * 0.75f)) * (1.0f + eH * 1.5f));
                        break;
                    }
                    case 5: { // CHIRP: Fast exponential pitch chirp kick with sine/saw morph and body saturation
                        float s = FastMath::fastSinNorm(slot.phase);
                        float saw = 2.0f * slot.phase - 1.0f;
                        float core = (1.0f - eF * 0.5f) * s + (eF * 0.5f) * saw;
                        oscOut = FastMath::fastTanh(core * (1.2f + eG * 2.4f + eH * 1.8f)) + slot.clickEnv * (eE * 0.6f);
                        break;
                    }
                    case 6: { // DUAL-V: Beating dual-oscillator kick with continuous interval and analog summing
                        float ratio = 1.002f + eG * 0.498f; // 2-cent beating to musical fifth (1.5x)
                        float v1 = FastMath::fastSinNorm(slot.phase);
                        float v2Tri = (slot.phase < 0.5f) ? (4.0f * slot.phase - 1.0f) : (3.0f - 4.0f * slot.phase);
                        float v2 = (1.0f - eF * 0.6f) * FastMath::fastSinNorm(slot.phase * ratio) + (eF * 0.6f) * FastMath::fastTanh(v2Tri * 2.0f);
                        oscOut = FastMath::fastTanh((v1 * 0.6f + v2 * 0.5f) * (1.1f + eH * 2.0f));
                        break;
                    }
                    default: { // GABBER: Hardcore overdriven gabber kick with extreme preamp gain and diode asymmetry
                        float s = FastMath::fastSinNorm(slot.phase);
                        float biased = s + eG * 0.40f; // Diode clipping asymmetry
                        float punchGain = 1.0f + eH * 2.5f * slot.timbreEnv;
                        oscOut = FastMath::fastTanh(biased * (3.0f + eF * 9.0f) * punchGain);
                        break;
                    }
                }
                break;
            }

            case 1: { // SD (Snare Drum)
                float n = nextRandBipolar();
                const float shellRatio = (0.85f + eA * 0.30f);
                switch (slot.model) {
                    case 0: { // ANLG-9: Dual bridged-T analog tone + snappy noise
                        float t1 = FastMath::fastSinNorm(slot.phase);
                        float t2 = FastMath::fastSinNorm(slot.phase * ((1.45f + eF * 0.45f) * shellRatio));
                        float tone = 0.55f * t1 + 0.45f * t2;
                        float noise = n * (0.4f + eG * 0.8f) * slot.timbreEnv;
                        float mix = (1.0f - eC * 0.65f) * tone + (eC * 0.85f) * noise;
                        oscOut = FastMath::fastTanh(mix * (1.1f + eH * 1.8f));
                        break;
                    }
                    case 1: { // ANLG-8: Resonant twin-T tone + snappy noise with warmth and attack snap
                        float t = FastMath::fastSinNorm(slot.phase);
                        float noise = n * (0.45f + eG * 0.75f) * slot.timbreEnv * slot.wireBloomEnv;
                        float body = t * (1.0f + eF * 0.8f);
                        float mix = (1.0f - eC * 0.7f) * body + (eC * 0.9f) * noise;
                        oscOut = FastMath::fastTanh(mix * (1.0f + eH * 1.5f));
                        break;
                    }
                    case 2: { // FM-SD: 2-op FM snare with envelope punch
                        float mF = curFreq * (1.8f + eF * 2.8f);
                        slot.modPhase += mF * invSampleRate_;
                        if (slot.modPhase >= 1.0f) slot.modPhase -= 1.0f;
                        float fmIndex = (eG * 2.8f) + (eH * 2.0f * slot.timbreEnv);
                        float shell = FastMath::fastSinNorm(slot.phase + FastMath::fastSinNorm(slot.modPhase) * fmIndex);
                        float wire = n * (0.4f + eG * 0.6f) * slot.timbreEnv * slot.wireBloomEnv;
                        oscOut = FastMath::fastTanh(((1.0f - eC * 0.6f) * shell + eC * wire) * (1.1f + eH * 1.5f));
                        break;
                    }
                    case 3: { // MODAL: 3-mode cylindrical acoustic shell and membrane resonator
                        float s1 = FastMath::fastSinNorm(slot.phase);
                        float s2 = FastMath::fastSinNorm(slot.phase * ((1.59f + eF * 0.3f) * shellRatio)) * slot.timbreEnv;
                        float s3 = FastMath::fastSinNorm(slot.phase * ((2.14f + eF * 0.5f) * shellRatio)) * (slot.timbreEnv * slot.timbreEnv);
                        float shell = 0.5f * s1 + 0.3f * s2 + 0.2f * s3;
                        float wire = n * (0.3f + eG * 0.7f) * slot.timbreEnv * slot.wireBloomEnv;
                        oscOut = FastMath::fastTanh(((1.0f - eC * 0.65f) * shell + eC * wire) * (1.1f + eH * 1.6f));
                        break;
                    }
                    case 4: { // VCF-S: Resonant filtered noise snare with analog bandpass shaping
                        float t = FastMath::fastSinNorm(slot.phase);
                        float noiseSizzle = (n > 0.0f ? 1.0f : -1.0f) * (0.5f * n + 0.5f * FastMath::fastSinNorm(slot.phase * (3.0f + eF * 4.0f)));
                        float wire = noiseSizzle * (0.4f + eG * 0.8f) * slot.timbreEnv * slot.wireBloomEnv;
                        float mix = (1.0f - eC * 0.6f) * t + (eC * 0.9f) * wire;
                        oscOut = FastMath::fastTanh(mix * (1.2f + eF * 1.5f + eH * 1.8f));
                        break;
                    }
                    case 5: { // DIGI-S: Digital multi-harmonic 80s snare
                        float s1 = FastMath::fastSinNorm(slot.phase);
                        float s2 = FastMath::fastSinNorm(slot.phase * 2.0f);
                        float s3 = FastMath::fastSinNorm(slot.phase * 3.0f);
                        float shell = 0.5f * s1 + 0.3f * s2 * eF + 0.2f * s3 * (1.0f - eF);
                        float digiNoise = (n > 0.0f ? 0.85f : -0.85f) * (0.4f + eG * 0.8f) * slot.timbreEnv * slot.wireBloomEnv;
                        float mix = (1.0f - eC * 0.6f) * shell + eC * digiNoise;
                        oscOut = FastMath::fastTanh(mix * (1.2f + eH * 2.0f));
                        break;
                    }
                    case 6: { // SHELL: Deep acoustic wooden shell resonator
                        float t1 = FastMath::fastSinNorm(slot.phase);
                        float t2 = FastMath::fastSinNorm(slot.phase * ((1.42f + eF * 0.35f) * shellRatio));
                        float wood = 0.65f * t1 + 0.35f * t2;
                        float rattle = n * (0.3f + eG * 0.7f) * (0.7f * slot.timbreEnv + 0.3f * slot.ampEnv) * slot.wireBloomEnv;
                        float mix = (1.0f - eC * 0.6f) * wood + eC * rattle;
                        oscOut = FastMath::fastTanh(mix * (1.1f + eH * 1.5f));
                        break;
                    }
                    default: { // EXPLOD: Industrial hard explode snare with gated blast
                        float t = FastMath::fastSinNorm(slot.phase);
                        float blast = n * (0.6f + eG * 1.2f);
                        float pre = (t * (1.0f + eF * 1.5f) + blast * (0.5f + eC * 0.8f));
                        oscOut = FastMath::fastTanh(pre * (2.2f + eH * 3.5f));
                        break;
                    }
                }
                break;
            }

            case 2: { // TM (Toms)
                switch (slot.model) {
                    case 0: { // ANLG-9: Dual bridged-T resonant analog tom
                        float t1 = FastMath::fastSinNorm(slot.phase);
                        float t2 = FastMath::fastSinNorm(slot.phase * (1.38f + eF * 0.28f));
                        float body = (1.0f - eG * 0.4f) * t1 + (eG * 0.4f) * t2 * slot.timbreEnv;
                        oscOut = FastMath::fastTanh(body * (1.1f + eH * 1.8f));
                        break;
                    }
                    case 1: { // ANLG-8: Resonant twin-T tom with circuit droop and sub ring
                        float s = FastMath::fastSinNorm(slot.phase);
                        float ring = FastMath::fastSinNorm(slot.phase * 2.0f) * (eG * 0.35f);
                        float mix = (s + ring) * (1.0f - 0.08f * slot.phase * (1.0f - eF * 0.5f));
                        oscOut = FastMath::fastTanh(mix * (1.0f + eH * 1.4f));
                        break;
                    }
                    case 2: { // HEXCESS: Analog sweep tom with pitch plunge and pulse harmonic
                        float s = FastMath::fastSinNorm(slot.phase);
                        float tri = (slot.phase < 0.5f) ? (4.0f * slot.phase - 1.0f) : (3.0f - 4.0f * slot.phase);
                        float wave = (1.0f - eF * 0.7f) * s + (eF * 0.7f) * tri;
                        float pulse = (slot.phase < (0.3f + eG * 0.4f)) ? 0.3f : -0.3f;
                        oscOut = FastMath::fastTanh((wave + pulse * eG * 0.4f) * (1.2f + eH * 1.8f));
                        break;
                    }
                    case 3: { // MODAL: Acoustic cylindrical shell and membrane tom
                        float s1 = FastMath::fastSinNorm(slot.phase);
                        float s2 = FastMath::fastSinNorm(slot.phase * (1.45f + eF * 0.30f)) * slot.timbreEnv;
                        float mix = 0.65f * s1 + (0.35f + eG * 0.35f) * s2;
                        oscOut = FastMath::fastTanh(mix * (1.1f + eH * 1.5f));
                        break;
                    }
                    case 4: { // PING-T: High-resonance pinged tom with vactrol LPG strike
                        float s = FastMath::fastSinNorm(slot.phase);
                        float ring = s * (1.0f + eG * 0.6f * FastMath::fastSinNorm(slot.phase * (2.0f + eF)));
                        oscOut = FastMath::fastTanh(ring * (1.0f + eH * 1.6f));
                        break;
                    }
                    case 5: { // FM-TOM: 2-op FM tom with attack punch index spike
                        float mF = curFreq * (1.25f + eF * 2.25f);
                        slot.modPhase += mF * invSampleRate_;
                        if (slot.modPhase >= 1.0f) slot.modPhase -= 1.0f;
                        float effIndex = (eG * 2.6f) + (eH * 2.2f * slot.timbreEnv);
                        float mod = FastMath::fastSinNorm(slot.modPhase) * effIndex;
                        oscOut = FastMath::fastSinNorm(slot.phase + mod * 0.159f);
                        break;
                    }
                    case 6: { // W-FOLD: Perkons triangle wavefold metallic tom
                        float tri = (slot.phase < 0.5f) ? (4.0f * slot.phase - 1.0f) : (3.0f - 4.0f * slot.phase);
                        tri += eG * 0.35f;
                        float fold = (1.0f + eF * 4.0f) + (eH * 2.5f * slot.timbreEnv);
                        oscOut = FastMath::fastSinNorm(tri * fold);
                        break;
                    }
                    default: { // FORMNT: African talking drum & dual-formant acoustic tom
                        float s1 = FastMath::fastSinNorm(slot.phase);
                        float f1 = FastMath::fastSinNorm(slot.phase * (1.5f + eF * 1.3f)) * slot.timbreEnv;
                        float f2 = FastMath::fastSinNorm(slot.phase * (2.2f + eG * 2.0f)) * (slot.timbreEnv * slot.timbreEnv);
                        float mix = 0.5f * s1 + 0.35f * f1 + 0.25f * f2;
                        oscOut = FastMath::fastTanh(mix * (1.2f + eH * 1.6f));
                        break;
                    }
                }
                break;
            }

            case 3: { // CP (Claps)
                slot.burstCount += 1.0f;
                float burst = 1.0f;
                const float spreadSamples = std::max(48.0f, (0.003f + eB * 0.024f) * sampleRate_);
                const int numBursts = 2 + static_cast<int>(eC * 3.0f); // 2 to 5 bursts based on BURST knob
                const float totalBurstTime = spreadSamples * static_cast<float>(numBursts);
                const float preDelaySamples = eA * 0.012f * sampleRate_;
                if (slot.burstCount >= preDelaySamples && slot.burstCount < totalBurstTime + preDelaySamples) {
                    float phaseInBurst = std::fmod(slot.burstCount - preDelaySamples, spreadSamples) / spreadSamples;
                    burst = FastMath::fastExp(-phaseInBurst * 4.5f) * 1.25f;
                }
                float n = nextRandBipolar();
                switch (slot.model) {
                    case 0: { // ANLG-9: Multi-burst filtered resonant analog clap
                        float filtered = (n + FastMath::fastSinNorm(slot.phase * (2.0f + eF * 1.5f)) * (0.35f + eG * 0.35f));
                        oscOut = FastMath::fastTanh(filtered * (1.2f + eH * 2.2f)) * burst;
                        break;
                    }
                    case 1: { // ANLG-8: Multi-burst analog clap with tone warmth and diffuse noise
                        float t = FastMath::fastSinNorm(slot.phase);
                        float mix = n * (0.7f + eG * 0.4f) + t * (0.25f * eF);
                        oscOut = FastMath::fastTanh(mix * (1.1f + eH * 1.6f)) * burst;
                        break;
                    }
                    case 2: { // HANDS: Multi-person organic acoustic handclap
                        float n2 = nextRandBipolar();
                        float n3 = nextRandBipolar();
                        float crowd = 0.45f * n + 0.35f * n2 + 0.20f * n3;
                        float flesh = FastMath::fastSinNorm(slot.phase * (1.2f + eF * 0.8f));
                        float mix = crowd * (1.0f + eG * 0.6f) + flesh * 0.35f;
                        oscOut = FastMath::fastTanh(mix * (1.1f + eH * 1.5f)) * burst;
                        break;
                    }
                    case 3: { // CP-BRST: Vintage analog ensemble multi-burst clap
                        float sq = (slot.phase < 0.5f) ? 0.8f : -0.8f;
                        float nF = (n > 0.0f ? 0.9f : -0.9f) * 0.7f + sq * (0.35f * eF);
                        oscOut = FastMath::fastTanh(nF * (1.6f + eH * 2.8f)) * burst;
                        break;
                    }
                    case 4: { // FM-CLAP: 2-op FM metallic frequency-modulated clap
                        float fmMod = FastMath::fastSinNorm(slot.phase * (3.5f + eF * 5.0f));
                        float fmIndex = (eG * 2.2f) + (eH * 1.8f * slot.timbreEnv);
                        float fm = FastMath::fastSinNorm(slot.phase * 2.0f + fmMod * fmIndex);
                        oscOut = FastMath::fastTanh((n * 0.6f + fm * 0.6f) * (1.1f + eH * 1.5f)) * burst;
                        break;
                    }
                    case 5: { // W-CLP: Wavefolded distorted industrial clap
                        float preFold = n + eG * 0.35f;
                        float folded = FastMath::fastSinNorm(preFold * (1.2f + eF * 4.5f));
                        oscOut = FastMath::fastTanh(folded * (1.3f + eH * 2.2f)) * burst;
                        break;
                    }
                    case 6: { // GRAIN: Granular micro-burst sizzle clap
                        float n2 = nextRandBipolar();
                        float gN = (n * (0.6f + eG * 0.4f) + (n2 > 0.3f ? 0.8f : -0.8f) * (0.3f + eF * 0.5f));
                        oscOut = FastMath::fastTanh(gN * (1.2f + eH * 2.0f)) * burst;
                        break;
                    }
                    default: { // COMB: Comb-filtered physical clap resonator
                        float sT = FastMath::fastSinNorm(slot.phase * (2.0f + eF * 4.0f));
                        float comb = 0.55f * n + 0.45f * sT * (0.5f + eG * 0.5f);
                        oscOut = FastMath::fastTanh(comb * (1.2f + eH * 1.8f)) * burst;
                        break;
                    }
                }
                break;
            }

            case 4: { // HH (Hi-Hats)
                constexpr float kRatios[6] = { 1.0f, 1.342f, 1.741f, 2.235f, 2.871f, 3.897f };
                float metalSum = 0.0f;
                const float clusterSpread = (0.5f + eA);
                const float clusterBaseHz = curFreq * (1.8f + eF * 1.4f);
                for (int m = 0; m < 6; ++m) {
                    slot.clusterPhases[m] += std::clamp(clusterBaseHz * (1.0f + (kRatios[m] - 1.0f) * clusterSpread), 20.0f, nyquist_) * invSampleRate_;
                    if (slot.clusterPhases[m] >= 1.0f) slot.clusterPhases[m] -= 1.0f;
                    metalSum += (slot.clusterPhases[m] < 0.5f) ? 0.35f : -0.35f;
                }
                float n = nextRandBipolar();
                switch (slot.model) {
                    case 0: { // SCHMITT: 6-oscillator CMOS Schmitt-trigger inverter metallic cluster
                        float metal = metalSum * (0.4f + eE * 0.6f);
                        float sizzle = n * (0.2f + eG * 0.8f);
                        oscOut = FastMath::fastTanh((metal + sizzle) * (1.1f + eH * 1.8f));
                        break;
                    }
                    case 1: { // DIODE: Tuned oscillator pair through passive diode bridge ring mod
                        float s1 = FastMath::fastSinNorm(slot.clusterPhases[0]);
                        float s2 = FastMath::fastSinNorm(slot.clusterPhases[1] * (1.0f + eF * 0.4f));
                        float diodeRing = FastMath::fastTanh(s1 * s2 * (2.0f + eE * 3.0f));
                        float mix = diodeRing * 0.65f + n * (0.3f + eG * 0.5f);
                        oscOut = FastMath::fastTanh(mix * (1.1f + eH * 1.5f));
                        break;
                    }
                    case 2: { // FM-GLS: FM glass hi-hat with shimmering inharmonic overtones
                        float mF = clusterBaseHz * (3.5f + eF * 4.5f);
                        slot.modPhase += mF * invSampleRate_;
                        if (slot.modPhase >= 1.0f) slot.modPhase -= 1.0f;
                        float fmIndex = (eG * 2.8f) + (eH * 1.5f * slot.timbreEnv);
                        float glass = FastMath::fastSinNorm(slot.clusterPhases[0] + FastMath::fastSinNorm(slot.modPhase) * fmIndex);
                        oscOut = FastMath::fastTanh((glass * (0.6f + eE * 0.4f) + n * 0.35f) * (1.1f + eH * 1.5f));
                        break;
                    }
                    case 3: { // FM-GRT: FM gritty distorted metallic hat with feedback noise
                        float mF = clusterBaseHz * (2.2f + eF * 3.5f);
                        slot.modPhase += mF * invSampleRate_;
                        if (slot.modPhase >= 1.0f) slot.modPhase -= 1.0f;
                        float gritMod = FastMath::fastSinNorm(slot.modPhase + n * (0.3f * eG));
                        float grit = FastMath::fastTanh(FastMath::fastSinNorm(slot.clusterPhases[0] * 2.0f + gritMod * (2.0f + eE * 2.5f)) * (1.5f + eH * 2.0f));
                        oscOut = grit;
                        break;
                    }
                    case 4: { // PLATE: Physical bronze/brass plate resonator with modal damping
                        float p1 = FastMath::fastSinNorm(slot.clusterPhases[0]);
                        float p2 = FastMath::fastSinNorm(slot.clusterPhases[1]) * slot.timbreEnv;
                        float p3 = FastMath::fastSinNorm(slot.clusterPhases[3]) * (slot.timbreEnv * slot.timbreEnv);
                        float plate = 0.5f * p1 + 0.3f * p2 + 0.2f * p3;
                        float mix = plate * (0.5f + eE * 0.5f) + n * (0.3f + eG * 0.5f);
                        oscOut = FastMath::fastTanh(mix * (1.1f + eH * 1.6f));
                        break;
                    }
                    case 5: { // CHIME: Metallic overtone chime & harmonic cluster
                        float c1 = FastMath::fastSinNorm(slot.clusterPhases[2] * (1.0f + eF * 0.2f));
                        float c2 = FastMath::fastSinNorm(slot.clusterPhases[4] * (1.0f + eF * 0.3f));
                        float chime = 0.55f * c1 + 0.45f * c2;
                        float mix = chime * (0.6f + eE * 0.4f) + n * (0.25f + eG * 0.45f);
                        oscOut = FastMath::fastTanh(mix * (1.1f + eH * 1.5f));
                        break;
                    }
                    case 6: { // NOISE: Bandpass shaped analog white/pink noise hat with transient tick
                        float coloredNoise = n * (0.7f + eF * 0.5f);
                        float tick = (slot.clickEnv > 0.05f) ? (n > 0.0f ? 0.8f : -0.8f) * eH : 0.0f;
                        oscOut = FastMath::fastTanh((coloredNoise + tick) * (1.2f + eG * 1.2f));
                        break;
                    }
                    default: { // RING-MOD: Inharmonic ring-modulated hat with dense sidebands
                        float oscA = FastMath::fastSinNorm(slot.clusterPhases[0] * (1.0f + eF * 0.5f));
                        float oscB = FastMath::fastSinNorm(slot.clusterPhases[2] * (1.4f + eF * 0.8f));
                        float rm = (oscA * oscB * 2.0f) * (0.6f + eE * 0.4f) + n * (0.3f + eG * 0.5f);
                        oscOut = FastMath::fastTanh(rm * (1.2f + eH * 1.8f));
                        break;
                    }
                }
                break;
            }

            case 5: { // CY (Cymbals)
                constexpr float kCyRatios[6] = { 1.0f, 1.34f, 1.71f, 2.22f, 2.87f, 3.45f };
                float cyMetal = 0.0f;
                const float cySpread = (0.5f + eA);
                const float cyBaseHz = curFreq * (1.5f + eF * 0.8f);
                for (int m = 0; m < 6; ++m) {
                    slot.clusterPhases[m] += std::clamp(cyBaseHz * (1.0f + (kCyRatios[m] - 1.0f) * cySpread), 20.0f, nyquist_) * invSampleRate_;
                    if (slot.clusterPhases[m] >= 1.0f) slot.clusterPhases[m] -= 1.0f;
                    cyMetal += (slot.clusterPhases[m] < 0.5f) ? 0.35f : -0.35f;
                }
                float n = nextRandBipolar();
                switch (slot.model) {
                    case 0: { // SCHMITT: 6-oscillator Schmitt metallic matrix with bell overtone and wash
                        float metal = cyMetal * (0.45f + eF * 0.55f);
                        float wash = n * (0.25f + eH * 0.45f);
                        float bell = FastMath::fastSinNorm(slot.clusterPhases[0] * 1.5f) * (eG * 0.4f);
                        oscOut = FastMath::fastTanh((metal + wash + bell) * (1.1f + eH * 1.5f));
                        break;
                    }
                    case 1: { // DIODE: Analog diode bridge ring mod cymbal with bell resonance
                        float s1 = FastMath::fastSinNorm(slot.clusterPhases[0]);
                        float s2 = FastMath::fastSinNorm(slot.clusterPhases[2] * (1.0f + eF * 0.5f));
                        float rm = FastMath::fastTanh(s1 * s2 * (2.2f + eF * 2.0f));
                        float bell = FastMath::fastSinNorm(slot.clusterPhases[1]) * (eG * 0.35f);
                        float mix = rm * 0.55f + bell + n * (0.3f + eH * 0.3f);
                        oscOut = FastMath::fastTanh(mix * (1.1f + eH * 1.5f));
                        break;
                    }
                    case 2: { // FM-CR: FM crash cymbal with attack FM surge
                        float mF = cyBaseHz * (1.75f + eF * 2.5f);
                        slot.modPhase += mF * invSampleRate_;
                        if (slot.modPhase >= 1.0f) slot.modPhase -= 1.0f;
                        float fmIndex = (eF * 2.5f) + (eH * 2.5f * slot.timbreEnv);
                        float crashFm = FastMath::fastSinNorm(slot.clusterPhases[0] + FastMath::fastSinNorm(slot.modPhase) * fmIndex);
                        oscOut = FastMath::fastTanh((crashFm * 0.65f + n * 0.35f) * (1.1f + eH * 1.6f));
                        break;
                    }
                    case 3: { // FM-RD: FM ride cymbal with defined bell ping & stick definition
                        float sBell = FastMath::fastSinNorm(slot.phase * (3.5f + eG * 2.5f));
                        float ping = sBell * (0.4f + eG * 0.6f) + slot.clickEnv * (eE * 0.5f);
                        float wash = cyMetal * 0.35f * slot.timbreEnv + n * (0.15f + eH * 0.25f);
                        oscOut = FastMath::fastTanh((ping + wash) * (1.1f + eH * 1.4f));
                        break;
                    }
                    case 4: { // BRONZE: Physical bronze plate / acoustic B20 cymbal model
                        float b1 = FastMath::fastSinNorm(slot.clusterPhases[0]);
                        float b2 = FastMath::fastSinNorm(slot.clusterPhases[1]) * slot.timbreEnv;
                        float b3 = FastMath::fastSinNorm(slot.clusterPhases[3]) * (slot.timbreEnv * slot.timbreEnv);
                        float bronze = 0.5f * b1 + 0.3f * b2 + 0.2f * b3;
                        float mix = bronze * (0.6f + eF * 0.4f) + n * (0.25f + eH * 0.35f);
                        oscOut = FastMath::fastTanh(mix * (1.1f + eH * 1.5f));
                        break;
                    }
                    case 5: { // GONG: Deep resonant orchestral gong / tam-tam with blooming wash
                        float sGong = FastMath::fastSinNorm(slot.phase * (0.6f + eG * 0.4f));
                        float bloom = FastMath::fastSinNorm(slot.clusterPhases[0] * 1.2f) * slot.timbreEnv;
                        float mix = sGong * 0.6f + bloom * (0.3f + eF * 0.4f) + cyMetal * 0.2f;
                        oscOut = FastMath::fastTanh(mix * (1.1f + eH * 1.6f));
                        break;
                    }
                    case 6: { // DUST: Lo-fi vintage dusty tape cymbal with tape compression
                        float dustNoise = (n > 0.0f ? 0.75f : -0.75f) * 0.3f + n * 0.4f;
                        float dustMetal = cyMetal * (0.6f - eF * 0.3f);
                        float mix = dustMetal + dustNoise;
                        oscOut = FastMath::fastTanh(mix * (1.3f + eH * 2.0f));
                        break;
                    }
                    default: { // CHIME: Bright shimmering splash cymbal & bell cluster
                        float s1 = FastMath::fastSinNorm(slot.phase * (4.0f + eG * 3.0f));
                        float s2 = FastMath::fastSinNorm(slot.clusterPhases[4] * (1.0f + eF * 0.3f));
                        float chime = 0.55f * s1 + 0.45f * s2;
                        float mix = chime * 0.65f + n * (0.25f + eH * 0.35f);
                        oscOut = FastMath::fastTanh(mix * (1.1f + eH * 1.5f));
                        break;
                    }
                }
                break;
            }

            case 6: { // PC (Aux Percussion)
                float n = nextRandBipolar();
                const float disperseRatio = (0.80f + eA * 0.40f);
                switch (slot.model) {
                    case 0: { // RIM-ST: Acoustic & electronic rimshot with shell ring and stick click
                        float sTone = FastMath::fastSinNorm(slot.phase * ((2.2f + eF * 1.2f) * disperseRatio));
                        float click = (n > 0.0f ? 0.9f : -0.9f) * slot.clickEnv;
                        float rim = sTone * (0.6f + eG * 0.4f) + click * (0.4f + eE * 0.6f);
                        oscOut = FastMath::fastTanh(rim * (1.2f + eH * 2.0f));
                        break;
                    }
                    case 1: { // ANLG-CB: Dual-square analog cowbell with bell ring and striker pop
                        float sq1 = (slot.phase < 0.5f) ? 0.7f : -0.7f;
                        float ph2 = slot.phase * ((1.40f + eF * 0.20f) * disperseRatio); ph2 -= static_cast<int>(ph2);
                        float sq2 = (ph2 < 0.5f) ? 0.7f : -0.7f;
                        float ring = FastMath::fastSinNorm(slot.phase * 3.5f) * (eG * 0.35f);
                        oscOut = FastMath::fastTanh((sq1 + sq2 + ring) * (1.1f + eH * 1.5f));
                        break;
                    }
                    case 2: { // CLAVE: Acoustic rosewood clave with dual body resonance
                        float s1 = FastMath::fastSinNorm(slot.phase);
                        float s2 = FastMath::fastSinNorm(slot.phase * ((1.618f + eF * 0.3f) * disperseRatio)) * slot.timbreEnv;
                        float mix = 0.75f * s1 + (0.25f + eG * 0.35f) * s2;
                        oscOut = FastMath::fastTanh(mix * (1.1f + eH * 1.4f));
                        break;
                    }
                    case 3: { // SHAKR: Granular seed / cabasa / maraca shaker with rub modulation
                        float n2 = nextRandBipolar();
                        float rub = FastMath::fastSinNorm(slot.phase * (6.0f + eF * 6.0f));
                        float seeds = n * (0.6f + eG * 0.4f) + (n2 > 0.4f ? 0.7f : -0.7f) * 0.3f;
                        float mix = seeds * (0.6f + 0.4f * rub);
                        oscOut = FastMath::fastTanh(mix * (1.1f + eH * 1.6f));
                        break;
                    }
                    case 4: { // CONGA: Acoustic conga / bongo with rim-slap pop and shell overtone
                        float c1 = FastMath::fastSinNorm(slot.phase);
                        float c2 = FastMath::fastSinNorm(slot.phase * ((1.52f + eF * 0.3f) * disperseRatio)) * slot.timbreEnv;
                        float slap = slot.clickEnv * (eH * 0.6f) * (n > 0.0f ? 0.8f : -0.8f);
                        float mix = 0.65f * c1 + (0.35f + eG * 0.35f) * c2 + slap;
                        oscOut = FastMath::fastTanh(mix * (1.1f + eH * 1.5f));
                        break;
                    }
                    case 5: { // ANVIL: Heavy industrial steel anvil strike with ping & ring
                        float sAnvil = FastMath::fastSinNorm(slot.phase * 3.8f);
                        float ring2 = FastMath::fastSinNorm(slot.phase * 7.2f) * slot.timbreEnv * eG;
                        float ping = slot.clickEnv * (eE * 0.8f);
                        float mix = FastMath::fastTanh(sAnvil * (1.8f + eF * 3.0f)) + ring2 + ping;
                        oscOut = FastMath::fastTanh(mix * (1.1f + eH * 1.6f));
                        break;
                    }
                    case 6: { // FM-PC: 2-op FM percussion synth with attack index spike
                        float mF = curFreq * (1.5f + eF * 4.0f);
                        slot.modPhase += mF * invSampleRate_;
                        if (slot.modPhase >= 1.0f) slot.modPhase -= 1.0f;
                        float effIndex = (eG * 2.8f) + (eH * 2.2f * slot.timbreEnv);
                        float mod = FastMath::fastSinNorm(slot.modPhase) * effIndex;
                        oscOut = FastMath::fastSinNorm(slot.phase + mod * 0.159f);
                        break;
                    }
                    default: { // SCRAP: Metallic friction, spring drum and scrape percussion
                        float saw = 2.0f * slot.phase - 1.0f;
                        float scrape = saw * (n > 0.0f ? 0.9f : -0.9f) * (0.6f + eF * 0.6f);
                        float spring = FastMath::fastSinNorm(slot.phase * 2.5f) * (eG * 0.4f);
                        oscOut = FastMath::fastTanh((scrape * 0.6f + n * 0.4f + spring) * (1.1f + eH * 1.8f));
                        break;
                    }
                }
                break;
            }

            default: { // FX (Special FX)
                switch (slot.model) {
                    case 0: { // LASER: Extreme pitch dive sci-fi laser with saw/sine morph
                        float s = FastMath::fastSinNorm(slot.phase);
                        float saw = 2.0f * slot.phase - 1.0f;
                        float laser = (1.0f - eF * 0.6f) * s + (eF * 0.6f) * saw;
                        oscOut = FastMath::fastTanh(laser * (1.2f + eH * 2.2f));
                        break;
                    }
                    case 1: { // SUB-DP: Deep cinema sub-drop boom with 2nd harmonic & wobble
                        float sub = FastMath::fastSinNorm(slot.phase * 0.5f);
                        float sub2 = FastMath::fastSinNorm(slot.phase) * (eF * 0.4f);
                        float wobble = (1.0f + eG * 0.2f * FastMath::fastSinNorm(slot.phase * 0.1f));
                        oscOut = FastMath::fastTanh((sub + sub2) * wobble * (1.2f + eH * 1.8f));
                        break;
                    }
                    case 2: { // FM-MRP: Dynamic 2-op FM morph and sci-fi warp
                        float mF = curFreq * (0.5f + eF * 7.5f);
                        slot.modPhase += mF * invSampleRate_;
                        if (slot.modPhase >= 1.0f) slot.modPhase -= 1.0f;
                        float effIndex = (eG * 4.0f) + (eH * 3.0f * slot.timbreEnv);
                        float mod = FastMath::fastSinNorm(slot.modPhase) * effIndex;
                        oscOut = FastMath::fastSinNorm(slot.phase + mod * 0.159f);
                        break;
                    }
                    case 3: { // W-FOLD: Triangle wavefolded modular synth zap
                        float tri = (slot.phase < 0.5f) ? (4.0f * slot.phase - 1.0f) : (3.0f - 4.0f * slot.phase);
                        tri += eG * 0.4f;
                        float fold = (1.0f + eF * 6.0f) * (1.0f + eH * 1.5f * slot.timbreEnv);
                        oscOut = FastMath::fastSinNorm(tri * fold);
                        break;
                    }
                    case 4: { // RING-M: Dual carrier/modulator ring mod with overdrive
                        float s1 = FastMath::fastSinNorm(slot.phase);
                        float s2 = FastMath::fastSinNorm(slot.phase * (1.414f + eF * 2.5f));
                        float rm = s1 * s2 * (2.0f + eG * 2.0f);
                        oscOut = FastMath::fastTanh(rm * (1.2f + eH * 2.0f));
                        break;
                    }
                    case 5: { // PD-ZAP: Phase distortion resonance zap with exponential dive
                        float warpAmt = 0.5f + eF * 1.5f;
                        float pdPhase = (slot.phase < 0.5f)
                            ? (FastMath::fastPow2(FastMath::fastLog2(std::max(slot.phase * 2.0f, 1e-4f)) * warpAmt) * 0.5f)
                            : (1.0f - FastMath::fastPow2(FastMath::fastLog2(std::max((1.0f - slot.phase) * 2.0f, 1e-4f)) * warpAmt) * 0.5f);
                        oscOut = FastMath::fastTanh(FastMath::fastSinNorm(pdPhase) * (1.2f + eH * 1.6f + eG * 0.8f));
                        break;
                    }
                    case 6: { // CHAOS: Chaotic attractor noise burst & radio zap
                        float n = nextRandBipolar();
                        float osc = FastMath::fastSinNorm(slot.phase * (2.0f + eF * 3.0f));
                        float chaos = osc * (1.0f + n * (eG * 1.5f)) + n * (0.3f * eG);
                        oscOut = FastMath::fastTanh(chaos * (1.5f + eH * 2.5f));
                        break;
                    }
                    default: { // FLANGE: Comb feedback through-zero riser
                        float saw = 2.0f * slot.phase - 1.0f;
                        float flgPhase = slot.phase * (1.0f + eF * 0.5f); flgPhase -= static_cast<int>(flgPhase);
                        float saw2 = 2.0f * flgPhase - 1.0f;
                        float comb = 0.5f * saw + 0.5f * saw2 * (0.5f + eG * 0.5f);
                        oscOut = FastMath::fastTanh(comb * (1.3f + eH * 2.0f));
                        break;
                    }
                }
                break;
            }
        }

        // 4. Universal Transient Exciter (Knob E)
        float exciterImpulse = 0.0f;
        if (eE > 0.01f && slot.clickEnv > 0.001f) {
            slot.exciterRng ^= slot.exciterRng << 13;
            slot.exciterRng ^= slot.exciterRng >> 17;
            slot.exciterRng ^= slot.exciterRng << 5;
            const float rngFlt = static_cast<float>((slot.exciterRng >> 8) & 0x00FFFFFF) * (2.0f / 16777216.0f) - 1.0f;

            if (p.lifeMode == static_cast<int>(LifeMode::Cyborg)) {
                // CYBORG: Radioactive Geiger bitburst + 20us charge-injection spike (calibrated headroom)
                const float bitBurst = ((slot.exciterRng & 0x01) ? 1.0f : -1.0f) * slot.clickEnv * (eE * 1.35f);
                exciterImpulse = bitBurst + cyborgChargeSpike_ * (eE * 0.85f);
                cyborgChargeSpike_ *= 0.95f;
            } else if (p.lifeMode == static_cast<int>(LifeMode::Organic)) {
                // ORGANIC: Felt/Wood beater compliance slew with velocity hardness (Tekno Law)
                const float cosClick = FastMath::fastCosNorm(slot.clickPhase);
                slot.clickPhase += 7000.0f * invSampleRate_;
                if (slot.clickPhase >= 1.0f) slot.clickPhase -= 1.0f;
                const float rawClick = (0.6f * cosClick + 0.4f * rngFlt) * slot.clickEnv * (eE * 2.5f);
                const float beaterSlew = std::clamp(0.04f + 0.25f * velForce * velForce, 0.02f, 0.40f);
                slot.beaterState += beaterSlew * (rawClick - slot.beaterState);
                exciterImpulse = slot.beaterState * (1.0f + slot.residualMem * 0.25f);
            } else {
                // SYNTHETIC: Pure PolyBLEP-aligned click & snap
                const float cosClick = FastMath::fastCosNorm(slot.clickPhase);
                slot.clickPhase += 8000.0f * invSampleRate_;
                if (slot.clickPhase >= 1.0f) slot.clickPhase -= 1.0f;
                exciterImpulse = (0.75f * cosClick + 0.25f * rngFlt) * slot.clickEnv * (eE * 2.5f);
            }
        }

        // 5. Low-End Resonant Punch Thump (Knob H - Gated to BD, SD, TM, PC)
        float punchThump = 0.0f;
        const bool allowsThump = (activeRole_ == 0 || activeRole_ == 1 || activeRole_ == 2 || activeRole_ == 6);
        if (allowsThump && eH > 0.01f && slot.thumpEnv > 0.001f) {
            const float thumpFreq = 45.0f + (1.0f - eH) * 35.0f; // 45..80 Hz
            slot.thumpPhase += thumpFreq * invSampleRate_;
            if (slot.thumpPhase >= 1.0f) slot.thumpPhase -= 1.0f;
            const float sinThump = FastMath::fastSinNorm(slot.thumpPhase) * slot.thumpEnv;

            if (p.lifeMode == static_cast<int>(LifeMode::Cyborg)) {
                // CYBORG: Overdriven Germanium diode pair with pure sub-bass weight retention (Win #1 cached ADAA)
                const float raw = sinThump * (eH * 1.5f);
                const float clipped = tkdsp::formless::ADAA::diode(raw, adaaThumpX1_, adaaThumpF1_);
                // 50% pure fundamental sub anchor + 50% aggressive diode edge
                punchThump = (0.50f * raw + 0.50f * clipped) * 1.20f;
            } else if (p.lifeMode == static_cast<int>(LifeMode::Organic)) {
                // ORGANIC: 12AX7 triode tube saturation (rich even harmonics)
                const float raw = sinThump * (eH * 1.6f);
                punchThump = raw + 0.18f * (raw * raw) - 0.05f * (raw * raw * raw);
            } else {
                // SYNTHETIC: Pure sine with 4th-order algebraic headroom
                const float raw = sinThump * (eH * 1.5f);
                const float v4 = raw * raw * raw * raw;
                punchThump = raw / std::sqrt(std::sqrt(1.0f + 0.3164f * v4));
            }
            slot.thumpEnv *= 0.992f;
        }

        slot.phase = origPhase; // Restore accumulator phase

        // Dynamic Velocity Routing (Win #5: hoisted to control rate)
        const float totalAudio = (oscOut + exciterImpulse + punchThump) * slot.aggrDrive;

        // Pre-VCA Lifeform Non-Linear Waveshaping & Saturation (Per-Slot)
        float saturatedAudio = totalAudio;

        if (p.lifeMode == static_cast<int>(LifeMode::Cyborg)) {
            // CYBORG: Searing MS-10 Diode Clipping + Infinifold Rail Foldback with Sub-Bass Defense (Win #1 cached ADAA)
            // 1. Calibrated drive into diode clipper: prevents full-cycle square rail collapse
            const float diodeIn = saturatedAudio * 0.95f;
            const float diodeOut = tkdsp::formless::ADAA::diode(diodeIn, slot.adaaDiodeX1, slot.adaaDiodeF1);
            // 2. High-headroom wavefold: threshold 0.95f catches peaks without hollowing the fundamental
            const float folded = tkdsp::formless::ADAA::fold(diodeOut, slot.adaaFoldX1, slot.adaaFoldF1, 0.95f, 0.50f);
            // 3. Sub-Bass Defense Anchor: blend 35% solid diode body with 65% toxic folded grit
            saturatedAudio = 0.35f * diodeOut + 0.65f * folded;
        } else if (p.lifeMode == static_cast<int>(LifeMode::Organic)) {
            // ORGANIC: Safe Bounded Asymmetric Triode Tube Saturation (2nd & 4th harmonics)
            const float x = saturatedAudio * 1.20f;
            const float yTube = (x + 0.28f * x * x) / (1.0f + 0.28f * std::abs(x));
            const float hp = yTube - slot.organicDcBlock;
            slot.organicDcBlock += 0.00065f * hp; // 5 Hz DC decoupling
            saturatedAudio = hp;
        }

        // Envelope VCA applied to the rich, characterful audio:
        return saturatedAudio * shapedAmp * slot.velocity;
    }

    // =========================================================================
    // Crossover 4th-Order High-Pass Filter (Cascaded SVFs)
    // =========================================================================
    inline float processHp4(float input) noexcept {
        const float hp1 = (input - 1.41421356f * hp1_s1_ - hp1_s2_) * hpH_;
        const float bp1 = hpG_ * hp1 + hp1_s1_;
        hp1_s1_ = hpG_ * hp1 + bp1;
        hp1_s2_ = hpG_ * bp1 + hp1_s2_;

        const float hp2 = (hp1 - 1.41421356f * hp2_s1_ - hp2_s2_) * hpH_;
        const float bp2 = hpG_ * hp2 + hp2_s1_;
        hp2_s1_ = hpG_ * hp2 + bp2;
        hp2_s2_ = hpG_ * bp2 + hp2_s2_;

        return hp2;
    }

    // =========================================================================
    // Universal Hardware Mojo Strip Processor
    // =========================================================================
    inline float processMojo(float highBand, const Params& p) noexcept {
        float y = highBand;

        const float targetRate = std::clamp(p.crush, 500.0f, 48000.0f);
        if (targetRate < sampleRate_ * 0.99f) {
            crushPhase_ += targetRate * invSampleRate_;
            if (crushPhase_ >= 1.0f) {
                crushPhase_ -= 1.0f;
                crushedSample_ = y;
            }
            y = crushedSample_;
        }

        // bits quantizer removed (replaced with FreqShifter on full-band bus)

        const float hp = y - mojoDcBlocker_;
        mojoDcBlocker_ += 0.0006f * hp;
        return hp;
    }

    void renderSampleStereo(float& outL, float& outR, const Params& p) noexcept {
        // Advance Volume & Envelopes Unconditionally (Prevents Attack-Lag on Next Hit)
        masterGain_ += masterSmooth_ * (masterGainTarget_ - masterGain_);
        if (masterGainTarget_ == 0.0f && masterGain_ < 1e-6f) masterGain_ = 0.0f;

        // Master Activity Gate (Argon & Claude Verified)
        const bool anySlotActive = slotA_.active || slotB_.active || (fadeRemaining_ > 0);
        const bool cyborgBleeding = (p.lifeMode == static_cast<int>(LifeMode::Cyborg) && cyborgBleedEnv_ > 0.001f);
        if (anySlotActive || cyborgBleeding) {
            masterAmpEnv_ = 1.0f;
        } else {
            masterAmpEnv_ *= kGateRelease10ms_;
            if (masterAmpEnv_ < 1e-7f) masterAmpEnv_ = 0.0f;
        }

        masterFilterEnv_ *= masterFilterDecayCoeff_;
        if (masterFilterEnv_ < 1e-7f) masterFilterEnv_ = 0.0f;

        auto& active = (activeSlot_ == 0) ? slotA_ : slotB_;
        auto& fading = (activeSlot_ == 0) ? slotB_ : slotA_;

        float rawAudio = renderSlotSample(active, p);

        if (fadeRemaining_ > 0) {
            const float fadeFrac = static_cast<float>(fadeRemaining_) / static_cast<float>(fadeTotal_);
            rawAudio += renderSlotSample(fading, p) * fadeFrac;
            --fadeRemaining_;
        } else {
            fading.active = false;
        }

        // Early-Return Trap Fix: Prevent truncating filter tails & Cyborg bleed
        const bool voiceFullySilent = (std::abs(rawAudio) < 1e-15f)
                                   && (masterAmpEnv_ < 1e-5f)
                                   && (cyborgBleedEnv_ <= 0.0f);
        if (voiceFullySilent) {
            outL = 0.0f;
            outR = 0.0f;
            return;
        }

        // 1st-Order ADAA Tanh Pre-Filter Drive with DC-Compensated Asymmetric Saturation
        float shaped = rawAudio;
        if (p.preDrive > 0.0001f) {
            const float driveGain = tkdsp::formless::evalDriveGain(p.preDrive);
            const float u0 = rawAudio * driveGain;
            const float u1 = adaaPreDriveX1_;
            adaaPreDriveX1_ = u0;

            // Log-Sum-Exp asymptotic split: prevents float overflow when |u| > 16
            auto adaaF1 = [](float u) noexcept -> float {
                const float absU = std::abs(u);
                if (absU > 16.0f) {
                    return absU - 0.69314718f; // Asymptotic ln(cosh(u)) = |u| - ln(2)
                }
                return std::log(std::cosh(absU));
            };

            float saturated = 0.0f;
            const float diff = u0 - u1;
            const float f0 = adaaF1(u0);
            if (std::abs(diff) < 1.0e-4f) {
                saturated = FastMath::fastTanh(0.5f * (u0 + u1)); // Midpoint fallback
            } else {
                saturated = (f0 - adaaPreDriveF1_) / diff;
            }
            adaaPreDriveF1_ = f0;

            // Asymmetric 2nd harmonic saturation with 8 Hz DC tracker
            const float asym = 0.08f * saturated * std::max(0.0f, saturated);
            adaaDcBlocker_ += 0.001f * (asym - adaaDcBlocker_);
            saturated += (asym - adaaDcBlocker_);

            const float t = std::min(p.preDrive * 2.0f, 1.0f);
            const float mix = t * t * (3.0f - 2.0f * t); // C1 smoothstep
            shaped = (1.0f - mix) * rawAudio + mix * saturated;
        }

        // Advance VTL5C3 Optocoupler Conductance (Organic Mode)
        if (p.lifeMode == static_cast<int>(LifeMode::Organic)) {
            const float targetLpg = std::clamp(active.ampEnv + 0.02f, 0.0f, 1.0f);
            if (targetLpg > lpgVactrolState_) {
                lpgVactrolState_ += 0.08f * (targetLpg - lpgVactrolState_); // Fast 2ms LED strike
            } else {
                lpgVactrolState_ += 0.0035f * (targetLpg - lpgVactrolState_); // Sluggish optical CdS droop
            }
        }

        // Unified Filter / VCF Engine (Respects p.filterType and resonance across all Life Modes)
        float rawFilt = vcfSvf_.process(shaped);
        if (p.filterType == 2) {
            // Red Team BP Headroom Compensation: normalize high-Q peak
            const float qVal = tkdsp::formless::evalCytomicQ(p.resonance);
            rawFilt *= std::min(1.0f, 2.0f / (1.0f + qVal * 0.15f));
        }
        vcfSvf_.clampState();
        float filtered = rawFilt;

        // Mojo Sample Rate Crush (Delta Crossover on High Band preserves sub-bass)
        if (p.crush < 47500.0f) {
            const float highBand = processHp4(filtered);
            const float processedHigh = processMojo(highBand, p);
            filtered = filtered + (processedHigh - highBand);
        }

        // Full-Band Zero-Latency Bode Frequency Shifter
        if (std::abs(p.shift) > 0.05f) {
            filtered = freqShifter_.process(filtered, p.shift);
        }

        const float hp = filtered - dcBlockerState_;
        dcBlockerState_ += 0.0006f * hp;
        filtered = hp;

        // Lifeform Bus Output Stage:
        // Activity-Gated Cyborg Bleed Floor & 17.5 kHz Analog Summing Bus Filter
        const bool voiceActive = slotA_.active || slotB_.active;
        if (voiceActive) {
            cyborgBleedEnv_ = 1.0f;
        } else if (cyborgBleedEnv_ > 0.0f) {
            cyborgBleedEnv_ -= invSampleRate_ * (1.0f / 0.250f); // 250ms decay to absolute silence
            if (cyborgBleedEnv_ < 0.0f) cyborgBleedEnv_ = 0.0f;
        }

        if (p.lifeMode == static_cast<int>(LifeMode::Cyborg) && cyborgBleedEnv_ > 0.0f) {
            // 50Hz transformer hum + high-frequency hash
            const float humPhase = static_cast<float>(sampleCounter_ % 960) * (2.0f * 3.14159265f / 960.0f);
            const float hum = FastMath::fastSinNorm(humPhase * 0.15915494f) * 0.008f;
            const float hash = ((slotA_.phase > 0.5f) ? 0.005f : -0.005f);
            const float bleed = (hum + hash) * (cyborgBleedEnv_ * cyborgBleedEnv_);
            filtered += bleed;

            // 17.5 kHz 1-pole anti-aliasing summing bus filter
            cyborgBusFilterState_ += cyborgBusAlpha_ * (filtered - cyborgBusFilterState_);
            filtered = cyborgBusFilterState_;
        }

        const float masterOut = FastMath::fastTanh(filtered * 0.95f);

        // Equal-Power Sinusoidal Panning with Smooth De-Click Crossfade Interpolation
        float gainL = active.panL;
        float gainR = active.panR;
        if (fadeRemaining_ > 0) {
            const float fadeFrac = static_cast<float>(fadeRemaining_) / static_cast<float>(fadeTotal_);
            gainL = (1.0f - fadeFrac) * active.panL + fadeFrac * fading.panL;
            gainR = (1.0f - fadeFrac) * active.panR + fadeFrac * fading.panR;
        }

        float stL = masterOut * gainL;
        float stR = masterOut * gainR;

        // 1. Mojo Dynamics & Stereo Widener
        squashStage_.process(stL, stR, p.squash);
        widerStage_.process(stL, stR, p.wider, activeRole_);

        // 2. Output Level (Volume Knob: -inf .. 0 dB .. +12 dB)
        stL *= masterGain_;
        stR *= masterGain_;

        // 3. Master Amp Envelope (Uniform Role-Calibrated Logarithmic Decay)
        stL *= masterAmpEnv_;
        stR *= masterAmpEnv_;

        // 4. Master Limiter (Distressor Nuke Mode Brickwall)
        drumLimiter_.process(stL, stR, p.masterLimiter);

        outL = stL;
        outR = stR;
    }

    inline float nextRandBipolar() noexcept {
        rngState_ ^= rngState_ << 13;
        rngState_ ^= rngState_ >> 17;
        rngState_ ^= rngState_ << 5;
        if (rngState_ == 0) rngState_ = 0x1337C0DE;
        const float u = static_cast<float>((rngState_ >> 8) & 0x00FFFFFF) * (1.0f / 16777216.0f);
        return 2.0f * u - 1.0f;
    }

    inline float nextTrig01() noexcept {
        trigRng_ += 0x9E3779B9;
        uint32_t z = trigRng_;
        z = (z ^ (z >> 16)) * 0x85EBCA6B;
        z = (z ^ (z >> 13)) * 0xC2B2AE35;
        z ^= (z >> 16);
        return static_cast<float>((z >> 8) & 0x00FFFFFF) * (1.0f / 16777216.0f);
    }

    inline float nextTrigBipolar() noexcept {
        trigRng_ += 0x9E3779B9;
        uint32_t z = trigRng_;
        z = (z ^ (z >> 16)) * 0x85EBCA6B;
        z = (z ^ (z >> 13)) * 0xC2B2AE35;
        z ^= (z >> 16);
        return static_cast<float>(static_cast<int32_t>(z)) * 4.656612875245797e-10f; // [-1.0, 1.0)
    }

    float sampleRate_{48000.0f};
    float invSampleRate_{1.0f / 48000.0f};
    float nyquist_{23760.0f};

    VoiceSlot slotA_{};
    VoiceSlot slotB_{};
    int activeSlot_{0};
    int fadeRemaining_{0};
    int fadeTotal_{64};

    int activeRole_{0};
    int pendingRole_{0};
    int activeModel_{0};
    int pendingModel_{0};

    SquashStage squashStage_{};
    WiderStage widerStage_{};
    DrumLimiter drumLimiter_{};
    FreqShifter freqShifter_{};
    float adaaDcBlocker_{0.0f};



    TbdAudio::DSPCommon::CytomicSVF vcfSvf_{};
    float lpgVactrolState_{0.0f};
    float lpgFilterState_{0.0f};
    float dcBlockerState_{0.0f};

    // Lifeform Triad States
    float voiceResidualMem_{0.0f};
    float adaaPreDriveX1_{0.0f};
    float adaaPreDriveF1_{0.0f};
    float adaaThumpX1_{0.0f};
    float adaaThumpF1_{0.0f};
    float adaaFoldX1_{0.0f};
    bool  decayCurveActive_{false};
    float decayCurveExp_{1.0f};
    uint32_t sampleCounter_{0};
    float dacTargetPitch_{1.0f};
    float dacSlewedPitch_{1.0f};
    float dacAlpha_{0.1f};
    float cyborgBleedEnv_{0.0f};
    float cyborgBusFilterState_{0.0f};
    float cyborgBusAlpha_{0.5f};
    float cyborgChargeSpike_{0.0f};
    float vtlSvfS1_{0.0f};
    float vtlSvfS2_{0.0f};
    float organicDcBlock_{0.0f};

    float hp1_s1_{0.0f}, hp1_s2_{0.0f};
    float hp2_s1_{0.0f}, hp2_s2_{0.0f};
    float hpG_{0.01f}, hpH_{1.0f};
    float mojoDcBlocker_{0.0f};
    float crushPhase_{0.0f};
    float crushedSample_{0.0f};

    uint32_t rngState_{0xDEADBEEF};
    uint32_t trigRng_{0x853C49E6};
    float panPhase_{0.0f};
    std::array<float, 8> latchedDraw_{{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};

    int blockCounter_{0};
    float macroSmoothed_{0.5f};
    float effAmpDecaySec_{0.150f};
    float ampDecaySmoothed_{1.0f};
    float ampSmoothAlpha_{0.10f};
    float kGateRelease10ms_{0.998f};
    float wireBloomAlpha_{0.05f};
    float log2AmpDecayCoeff_{-0.001f};
    float ampDecayCoeff_{0.999f};
    float pitchDecayCoeff_{0.995f};
    float timbreDecayCoeff_{0.990f};
    float clickDecayCoeff_{0.950f};

    // Master Volume & Post-Mojo Envelope System
    float masterGain_{1.0f};
    float masterGainTarget_{1.0f};
    float masterSmooth_{0.0f};
    bool  masterGainInit_{false};

    float masterAmpEnv_{0.0f};
    float masterFilterEnv_{0.0f};
    float masterFilterDecayCoeff_{0.999f};

public:
    [[nodiscard]] float getMasterAmpEnv() const noexcept { return masterAmpEnv_; }
    [[nodiscard]] float getLog2AmpDecayCoeff() const noexcept { return log2AmpDecayCoeff_; }
    [[nodiscard]] float getEffAmpDecaySec() const noexcept { return effAmpDecaySec_; }
};

} // namespace tkdsp
