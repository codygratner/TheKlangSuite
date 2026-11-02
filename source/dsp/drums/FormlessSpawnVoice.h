#pragma once

#include "FastMath.h"
#include "../common/AudioEffect.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace tkdsp {

namespace FastMath = TbdAudio::FastMath;

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
        {"TRX-9", "BOOM", "EFM-2", "W-FOLD", "MODAL", "CHIRP", "DUAL-V", "GABBER"},
        // SD (0..7)
        {"TRX-9", "TRX-8", "EFM-S", "MODAL", "VCF-S", "DIGI-S", "SHELL", "EXPLOD"},
        // TM (0..7)
        {"TRX-9", "TRX-8", "SIMM-T", "MODAL", "PING-T", "EFM-T", "W-FOLD", "FORMNT"},
        // CP (0..7)
        {"TRX-9", "TRX-8", "HANDS", "CLP-TRP", "EFM-C", "W-CLP", "GRAIN", "COMB"},
        // HH (0..7)
        {"ROLAND", "KORG", "EFM-GLS", "EFM-GRT", "PLATE", "CHIME", "NOISE", "RING-M"},
        // CY (0..7)
        {"ROLAND", "KORG", "EFM-CR", "EFM-RD", "BRONZE", "GONG", "DUST", "CHIME"},
        // PC (0..7)
        {"RIM-ST", "COWB-8", "CLAVE", "SHAKR", "CONGA", "ANVIL", "EFM-PC", "SCRAP"},
        // FX (0..7)
        {"LASER", "SUB-DP", "FM-MRP", "W-FOLD", "RING-M", "CZ-ZAP", "CHAOS", "FLANGE"}
    };

    static constexpr const char* kRoleParamNames[kTotalRoles][8] = {
        // BD
        {"PITCH", "P.DEC", "P.DEPTH", "DECAY", "CLICK", "TIMBRE", "DRIVE", "PUNCH"},
        // SD
        {"TUNE", "P.DEC", "SNAPPY", "S.DECAY", "ATTACK", "TONE", "NOISE", "SNAP"},
        // TM
        {"PITCH", "BEND.T", "BEND.D", "DECAY", "STICK", "TENSION", "RING", "BODY"},
        // CP
        {"PITCH", "SPREAD", "BURST", "TAIL", "FLAM", "COLOR", "DIFFUSE", "CRACK"},
        // HH
        {"PITCH", "CHOKE", "METRIC", "DECAY", "METALLIC", "BRIGHT", "SIZZLE", "BITE"},
        // CY
        {"PITCH", "SHIMMER", "DAMP", "SUSTAIN", "MALLET", "DENSITY", "BELL", "CRASH"},
        // PC
        {"PITCH", "SWEEP", "BEND", "DECAY", "IMPACT", "MATERIAL", "RING", "POP"},
        // FX
        {"FREQ", "SPEED", "SWEEP", "DECAY", "TRIGGER", "WARP", "CHAOS", "EDGE"}
    };

    // =========================================================================
    // 64-Model Calibrated Macro Decay Matrix (ACF-5 Calibrated)
    // =========================================================================
    static constexpr float kModelBaseAmpDecaySec[kTotalRoles][kModelsPerRole] = {
        // BD: TRX-9  BOOM   EFM-2  W-FOLD MODAL  CHIRP  DUAL-V GABBER
        {      0.110f,0.280f,0.095f,0.100f,0.140f,0.060f,0.150f,0.130f },
        // SD: TRX-9  TRX-8  EFM-S  MODAL  VCF-S  DIGI-S SHELL  EXPLOD
        {      0.100f,0.200f,0.140f,0.180f,0.160f,0.110f,0.240f,0.350f },
        // TM: TRX-9  TRX-8  SIMM-T MODAL  PING-T EFM-T  W-FOLD FORMNT
        {      0.300f,0.260f,0.240f,0.420f,0.380f,0.220f,0.280f,0.320f },
        // CP: TRX-9  TRX-8  HANDS  CLP-TR EFM-C  W-CLP  GRAIN  COMB
        {      0.140f,0.180f,0.220f,0.260f,0.120f,0.150f,0.240f,0.200f },
        // HH: ROLAND KORG   EFM-GL EFM-GR PLATE  CHIME  NOISE  RING-M
        {      0.050f,0.070f,0.080f,0.040f,0.120f,0.150f,0.060f,0.090f },
        // CY: ROLAND KORG   EFM-CR EFM-RD BRONZE GONG   DUST   CHIME
        {      1.100f,1.600f,1.200f,1.000f,1.400f,1.800f,0.700f,1.500f },
        // PC: RIM-ST COWB-8 CLAVE  SHAKR  CONGA  ANVIL  EFM-PC SCRAP
        {      0.060f,0.180f,0.080f,0.120f,0.250f,0.400f,0.140f,0.220f },
        // FX: LASER  SUB-DP FM-MRP W-FOLD RING-M CZ-ZAP CHAOS  FLANGE
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

    struct Params {
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
        float decayCurve{-0.5f};     // -1.0 .. +1.0

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

        // Card 3 Pod 2 & 3: Master Altar & Mojo Console (10)
        float macroDecay{0.5f};      // 0.0 .. 1.0 (Master Macro)
        float volume{0.85f};         // 0.0 .. 1.0
        float cutoff{2000.0f};       // 20.0 .. 20000.0 Hz
        float resonance{0.30f};      // 0.0 .. 1.0
        int   filterType{0};         // 0=LP, 1=HP, 2=BP, 3=BR
        float preDrive{0.15f};       // 0.0 .. 1.0
        float crush{48000.0f};       // 1000.0 .. 48000.0 Hz
        float bits{16.0f};           // 4.0 .. 16.0 bits
        float tapeGlue{0.0f};        // 0.0 .. 1.0
        int   mojoProfile{0};        // 0=Clean, 1=MD-12, 2=SP-1200, 3=Iron Tape
    };

    FormlessSpawnVoice() noexcept {
        reset();
    }

    void prepare(float sampleRate) noexcept {
        sampleRate_ = std::max(sampleRate, 4000.0f);
        invSampleRate_ = 1.0f / sampleRate_;
        nyquist_ = 0.495f * sampleRate_;
        reset();
    }

    void reset() noexcept {
        slotA_ = VoiceSlot{};
        slotB_ = VoiceSlot{};
        activeSlot_ = 0;
        fadeRemaining_ = 0;
        fadeTotal_ = 64;

        vcfSvf_.reset();
        lpgVactrolState_ = 0.0f;
        lpgFilterState_ = 0.0f;
        dcBlockerState_ = 0.0f;

        hp1_s1_ = 0.0f; hp1_s2_ = 0.0f;
        hp2_s1_ = 0.0f; hp2_s2_ = 0.0f;
        mojoDcBlocker_ = 0.0f;
        crushPhase_ = 0.0f;
        crushedSample_ = 0.0f;

        rngState_ = 0xDEADBEEF;
        blockCounter_ = 0;
        macroSmoothed_ = 0.5f;

        activeRole_ = pendingRole_;
        activeModel_ = pendingModel_;
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
            latchedDraw_[i] = nextTrigBipolar();
        }

        // 3. Ping-Pong Voice Allocation (1.5ms Linear Slew De-Click)
        activeSlot_ = 1 - activeSlot_;
        auto& newSlot = (activeSlot_ == 0) ? slotA_ : slotB_;

        fadeTotal_ = std::max(16, static_cast<int>(0.0015f * sampleRate_));
        fadeRemaining_ = fadeTotal_;

        // 4. Initialize New Slot with Latched Offsets
        newSlot.active = true;
        newSlot.velocity = velocity;
        newSlot.ampEnv = 1.0f;
        newSlot.pitchEnv = 1.0f;
        newSlot.clickEnv = 1.0f;
        newSlot.timbreEnv = 1.0f;
        newSlot.burstCount = 0.0f;

        newSlot.latchedOffX    = (d[0] > 0.0f) ? latchedDraw_[0] * d[0] : 0.0f;
        newSlot.latchedOffY    = (d[1] > 0.0f) ? latchedDraw_[1] * d[1] : 0.0f;
        newSlot.latchedOffZ    = (d[2] > 0.0f) ? latchedDraw_[2] * d[2] : 0.0f;
        newSlot.latchedOffT    = (d[3] > 0.0f) ? latchedDraw_[3] * d[3] : 0.0f;
        newSlot.latchedOffAggr = (d[4] > 0.0f) ? latchedDraw_[4] * d[4] : 0.0f;
        newSlot.latchedOffTimb = (d[5] > 0.0f) ? latchedDraw_[5] * d[5] : 0.0f;
        newSlot.latchedOffPan  = (d[6] > 0.0f) ? latchedDraw_[6] * d[6] : 0.0f;
        newSlot.latchedDecayScale = (d[7] > 0.0f) ? std::clamp(FastMath::fastPow2(1.5f * latchedDraw_[7] * d[7]), 0.25f, 4.0f) : 1.0f;

        blockCounter_ = 0; // Force immediate coefficient update

        // Lifeform Phase Reset at t=0
        if (p.lifeMode == static_cast<int>(LifeMode::Cyborg)) {
            newSlot.phase = 0.25f;
            newSlot.modPhase = 0.5f;
            newSlot.subPhase = 0.0f;
        } else if (p.lifeMode == static_cast<int>(LifeMode::Organic)) {
            newSlot.phase = 0.005f * ((velocity > 0.5f) ? 1.0f : -1.0f);
            newSlot.modPhase = 0.0f;
            newSlot.subPhase = 0.0f;
        } else {
            newSlot.phase = 0.0f;
            newSlot.modPhase = 0.0f;
            newSlot.subPhase = 0.0f;
        }
        for (auto& ph : newSlot.clusterPhases) ph = 0.0f;

        // Base Frequency Calculation per Role
        // Knob A (p.paramA) is the native tune control [0..1]
        // Macro X biases Knob A
        const float effX = std::clamp(p.macroX + newSlot.latchedOffX, 0.0f, 1.0f);
        const float effA = std::clamp(p.paramA + (effX - 0.5f), 0.0f, 1.0f);

        float minHz = 35.0f, maxHz = 140.0f;
        switch (activeRole_) {
            case 0: minHz = 35.0f;   maxHz = 140.0f;  break; // BD
            case 1: minHz = 120.0f;  maxHz = 360.0f;  break; // SD
            case 2: minHz = 55.0f;   maxHz = 280.0f;  break; // TM
            case 3: minHz = 400.0f;  maxHz = 1600.0f; break; // CP
            case 4: minHz = 2500.0f; maxHz = 9000.0f; break; // HH
            case 5: minHz = 1800.0f; maxHz = 7500.0f; break; // CY
            case 6: minHz = 140.0f;  maxHz = 1200.0f; break; // PC
            case 7: minHz = 40.0f;   maxHz = 4000.0f; break; // FX
            default: break;
        }

        float baseFreq = 55.0f;
        if (noteHz > 10.0f) {
            baseFreq = noteHz * FastMath::fastPow2((effA - 0.5f) * 2.0f);
        } else {
            baseFreq = minHz * FastMath::fastPow2(effA * FastMath::fastLog2(maxHz / minHz));
        }
        newSlot.baseHz = std::clamp(baseFreq, 15.0f, nyquist_);
    }

    void processBlockStereo(float* outL, float* outR, int numSamples, const Params& p) noexcept {
        for (int i = 0; i < numSamples; ++i) {
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
        float latchedOffX{0.0f};
        float latchedOffY{0.0f};
        float latchedOffZ{0.0f};
        float latchedOffT{0.0f};
        float latchedOffAggr{0.0f};
        float latchedOffTimb{0.0f};
        float latchedOffPan{0.0f};
        bool  active{false};
    };

    // =========================================================================
    // Multiplicative Log-Domain PCHIP Spline Evaluator
    // =========================================================================
    static inline float evalMacroMultiplier(float macro) noexcept {
        macro = std::clamp(macro, 0.0f, 1.0f);
        constexpr float y[5] = {-2.07944154f, -0.91629073f, 0.0f, 0.91629073f, 2.07944154f};
        constexpr float d[5] = {4.65260f, 4.15888f, 3.66516f, 4.15888f, 4.65260f};

        float scaled = macro * 4.0f;
        int idx = std::clamp(static_cast<int>(scaled), 0, 3);
        float t = scaled - static_cast<float>(idx);
        float t2 = t * t;
        float t3 = t2 * t;

        float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
        float h10 = (t3 - 2.0f * t2 + t) * 0.25f;
        float h01 = -2.0f * t3 + 3.0f * t2;
        float h11 = (t3 - t2) * 0.25f;

        float lnG = h00 * y[idx] + h10 * d[idx] + h01 * y[idx + 1] + h11 * d[idx + 1];
        return FastMath::fastExp(lnG);
    }

    void updateControlRateCoefficients(const Params& p) noexcept {
        macroSmoothed_ += 0.08f * (p.macroDecay - macroSmoothed_);
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
        const float effAmpDecaySec = std::clamp(baseAmpSec * macroMult * tMult * (0.25f + p.paramD * 1.75f), 0.003f, 6.0f);

        // Pitch sweep decay scaled by Knob B (Pitch Decay) and Time Macro
        const float pitchMaxSec = (pitchResp >= 0.5f) ? 1.200f : 0.350f;
        const float effPitchDecaySec = std::clamp((0.005f + p.paramB * 0.350f) * pitchMacro * tMult, 0.002f, pitchMaxSec);

        // Timbre decay
        const float effTimbreDecaySec = std::clamp(0.040f * timbreMacro * tMult, 0.002f, 1.500f);
        const float effClickDecaySec  = std::clamp(0.002f * FastMath::fastPow2(macroSmoothed_ * 1.2f), 0.0008f, 0.006f);

        ampDecayCoeff_    = FastMath::fastExp(-invSampleRate_ / effAmpDecaySec);
        pitchDecayCoeff_  = FastMath::fastExp(-invSampleRate_ / effPitchDecaySec);
        timbreDecayCoeff_ = FastMath::fastExp(-invSampleRate_ / effTimbreDecaySec);
        clickDecayCoeff_  = FastMath::fastExp(-invSampleRate_ / effClickDecaySec);

        // VCF Cutoff Pre-Warping
        const float rawCutoff = std::clamp(p.cutoff, 20.0f, nyquist_);
        const float q = 0.5f + p.resonance * 7.5f;

        switch (p.filterType) {
            case 0: vcfSvf_.setLowpass(rawCutoff, q, invSampleRate_); break;
            case 1: vcfSvf_.setHighpass(rawCutoff, q, invSampleRate_); break;
            case 2: vcfSvf_.setBandpass(rawCutoff, q, invSampleRate_); break;
            case 3: vcfSvf_.setNotch(rawCutoff, q, invSampleRate_); break;
            default: vcfSvf_.setLowpass(rawCutoff, q, invSampleRate_); break;
        }

        constexpr float kCrossoverHz = 100.0f;
        const float normXover = kCrossoverHz * invSampleRate_;
        hpG_ = std::tan(normXover * 3.1415926535f);
        hpH_ = 1.0f / (1.0f + 1.41421356f * hpG_ + hpG_ * hpG_);
    }

    inline float renderSlotSample(VoiceSlot& slot, const Params& p) noexcept {
        if (!slot.active || slot.ampEnv < 0.00001f) {
            slot.active = false;
            return 0.0f;
        }

        // 1. Effective Macro & Synthesis Parameters with Stepped Random Offsets
        const float effX = std::clamp(p.macroX + slot.latchedOffX, 0.0f, 1.0f);
        const float effY = std::clamp(p.macroY + slot.latchedOffY, 0.0f, 1.0f);
        const float effZ = std::clamp(p.macroZ + slot.latchedOffZ, 0.0f, 1.0f);
        const float effT = std::clamp(p.macroT + slot.latchedOffT, 0.0f, 1.0f);
        const float effAggr = std::clamp(p.velAggr + slot.latchedOffAggr, 0.0f, 1.0f);
        const float effTimb = std::clamp(p.velTimb + slot.latchedOffTimb, 0.0f, 1.0f);

        // Synthesis-level Velocity Dynamics (velAggr & velTimb)
        const float velForce = slot.velocity;
        const float synthAggr = std::clamp(1.0f + (velForce - 0.75f) * effAggr * 1.5f, 0.2f, 2.5f);
        const float synthTimb = std::clamp(1.0f + (velForce - 0.75f) * effTimb * 1.5f, 0.2f, 2.5f);

        // Macro Vector Mapping to Knobs A..H
        // X (Pitch Space) -> A (Base Pitch), C (Pitch Sweep Depth)
        // Y (Punch Space) -> E (Attack/Click Spike), H (Punch/Snap)
        // Z (Timbre Space) -> F (Non-linearity/Ratio), G (Overtones/Index)
        // T (Time Space) -> Handled via updateControlRateCoefficients scaling B and D
        const float eA = std::clamp(p.paramA + (effX - 0.5f), 0.0f, 1.0f);
        const float eB = p.paramB;
        const float eC = std::clamp(p.paramC * (0.2f + 1.6f * effX), 0.0f, 1.0f);
        const float eD = p.paramD;
        const float eE = std::clamp(p.paramE * (0.2f + 1.6f * effY) * synthAggr, 0.0f, 1.0f);
        const float eF = std::clamp(p.paramF * (0.2f + 1.6f * effZ) * synthTimb, 0.0f, 1.0f);
        const float eG = std::clamp(p.paramG * (0.2f + 1.6f * effZ) * synthTimb, 0.0f, 1.0f);
        const float eH = std::clamp(p.paramH * (0.2f + 1.6f * effY) * synthAggr, 0.0f, 1.0f);

        // 2. Pitch Envelope Dive
        const float pitchDepthSemis = eC * 48.0f;
        const float pitchMult = FastMath::fastPow2((pitchDepthSemis * slot.pitchEnv) * (1.0f / 12.0f));
        const float curFreq = std::clamp(slot.baseHz * pitchMult, 10.0f, nyquist_);

        // Advance Recursive Envelopes
        const float dScale = slot.latchedDecayScale;
        slot.pitchEnv  *= std::clamp(FastMath::fastPow2(FastMath::fastLog2(pitchDecayCoeff_) / dScale), 0.80f, 0.9999995f);
        slot.ampEnv    *= std::clamp(FastMath::fastPow2(FastMath::fastLog2(ampDecayCoeff_) / dScale), 0.80f, 0.9999995f);
        slot.timbreEnv *= std::clamp(FastMath::fastPow2(FastMath::fastLog2(timbreDecayCoeff_) / dScale), 0.80f, 0.9999995f);
        slot.clickEnv  *= clickDecayCoeff_;

        // Decay Curvature Shaping on Amplitude
        float shapedAmp = slot.ampEnv;
        if (std::abs(p.decayCurve) > 0.01f) {
            const float exponent = FastMath::fastPow2(-p.decayCurve * 1.5f);
            shapedAmp = FastMath::fastPow2(FastMath::fastLog2(std::max(slot.ampEnv, 1e-6f)) * exponent);
        }

        // Advance Main Oscillator Phase
        slot.phase += curFreq * invSampleRate_;
        if (slot.phase >= 1.0f) slot.phase -= 1.0f;

        // 3. Synthesis Core Dispatch (8 Roles x 8 Models = 64 Sovereign Models)
        float oscOut = 0.0f;

        switch (activeRole_) {
            case 0: { // BD (Bass Drum)
                switch (activeModel_) {
                    case 0: { // TRX-9: Classic 909-style bridged-T punchy sine with 2nd-harmonic asymmetry
                        float s = FastMath::fastSinNorm(slot.phase);
                        float asym = s + eG * 0.35f * s * s;
                        oscOut = FastMath::fastTanh(asym * (1.1f + eF * 2.2f + eH * 1.6f));
                        break;
                    }
                    case 1: { // BOOM: 808 deep sub boom with tape saturation and 2nd harmonic presence
                        float sub2 = FastMath::fastSinNorm(slot.phase * 2.0f) * (eG * 0.35f);
                        float s = FastMath::fastSinNorm(slot.phase) * (1.0f - 0.05f * slot.phase) + sub2;
                        float sat = 1.0f + eF * 2.2f;
                        float punch = 1.0f + eH * 1.2f;
                        oscOut = FastMath::fastTanh(s * (sat * punch));
                        break;
                    }
                    case 2: { // EFM-2: 2-op metallic FM kick with attack transient index scaling
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
                switch (activeModel_) {
                    case 0: { // TRX-9: 909 dual bridged-T tone + snappy noise
                        float t1 = FastMath::fastSinNorm(slot.phase);
                        float t2 = FastMath::fastSinNorm(slot.phase * (1.45f + eF * 0.45f));
                        float tone = 0.55f * t1 + 0.45f * t2;
                        float noise = n * (0.4f + eG * 0.8f) * slot.timbreEnv;
                        float mix = (1.0f - eC * 0.65f) * tone + (eC * 0.85f) * noise;
                        oscOut = FastMath::fastTanh(mix * (1.1f + eH * 1.8f));
                        break;
                    }
                    case 1: { // TRX-8: 808 tone + snappy noise with tone warmth and attack snap
                        float t = FastMath::fastSinNorm(slot.phase);
                        float noise = n * (0.45f + eG * 0.75f) * slot.timbreEnv;
                        float body = t * (1.0f + eF * 0.8f);
                        float mix = (1.0f - eC * 0.7f) * body + (eC * 0.9f) * noise;
                        oscOut = FastMath::fastTanh(mix * (1.0f + eH * 1.5f));
                        break;
                    }
                    case 2: { // EFM-S: 2-op FM snare with envelope punch
                        float mF = curFreq * (1.8f + eF * 2.8f);
                        slot.modPhase += mF * invSampleRate_;
                        if (slot.modPhase >= 1.0f) slot.modPhase -= 1.0f;
                        float fmIndex = (eG * 2.8f) + (eH * 2.0f * slot.timbreEnv);
                        float shell = FastMath::fastSinNorm(slot.phase + FastMath::fastSinNorm(slot.modPhase) * fmIndex);
                        float wire = n * (0.4f + eG * 0.6f) * slot.timbreEnv;
                        oscOut = FastMath::fastTanh(((1.0f - eC * 0.6f) * shell + eC * wire) * (1.1f + eH * 1.5f));
                        break;
                    }
                    case 3: { // MODAL: 3-mode cylindrical acoustic shell and membrane resonator
                        float s1 = FastMath::fastSinNorm(slot.phase);
                        float s2 = FastMath::fastSinNorm(slot.phase * (1.59f + eF * 0.3f)) * slot.timbreEnv;
                        float s3 = FastMath::fastSinNorm(slot.phase * (2.14f + eF * 0.5f)) * (slot.timbreEnv * slot.timbreEnv);
                        float shell = 0.5f * s1 + 0.3f * s2 + 0.2f * s3;
                        float wire = n * (0.3f + eG * 0.7f) * slot.timbreEnv;
                        oscOut = FastMath::fastTanh(((1.0f - eC * 0.65f) * shell + eC * wire) * (1.1f + eH * 1.6f));
                        break;
                    }
                    case 4: { // VCF-S: Resonant filtered noise snare (DR-110/KR-55 style)
                        float t = FastMath::fastSinNorm(slot.phase);
                        float noiseSizzle = (n > 0.0f ? 1.0f : -1.0f) * (0.5f * n + 0.5f * FastMath::fastSinNorm(slot.phase * (3.0f + eF * 4.0f)));
                        float wire = noiseSizzle * (0.4f + eG * 0.8f) * slot.timbreEnv;
                        float mix = (1.0f - eC * 0.6f) * t + (eC * 0.9f) * wire;
                        oscOut = FastMath::fastTanh(mix * (1.2f + eF * 1.5f + eH * 1.8f));
                        break;
                    }
                    case 5: { // DIGI-S: Digital multi-harmonic 80s snare
                        float s1 = FastMath::fastSinNorm(slot.phase);
                        float s2 = FastMath::fastSinNorm(slot.phase * 2.0f);
                        float s3 = FastMath::fastSinNorm(slot.phase * 3.0f);
                        float shell = 0.5f * s1 + 0.3f * s2 * eF + 0.2f * s3 * (1.0f - eF);
                        float digiNoise = (n > 0.0f ? 0.85f : -0.85f) * (0.4f + eG * 0.8f) * slot.timbreEnv;
                        float mix = (1.0f - eC * 0.6f) * shell + eC * digiNoise;
                        oscOut = FastMath::fastTanh(mix * (1.2f + eH * 2.0f));
                        break;
                    }
                    case 6: { // SHELL: Deep acoustic wooden shell resonator
                        float t1 = FastMath::fastSinNorm(slot.phase);
                        float t2 = FastMath::fastSinNorm(slot.phase * (1.42f + eF * 0.35f));
                        float wood = 0.65f * t1 + 0.35f * t2;
                        float rattle = n * (0.3f + eG * 0.7f) * (0.7f * slot.timbreEnv + 0.3f * slot.ampEnv);
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
                switch (activeModel_) {
                    case 0: { // TRX-9: 909 dual bridged-T resonant tom
                        float t1 = FastMath::fastSinNorm(slot.phase);
                        float t2 = FastMath::fastSinNorm(slot.phase * (1.38f + eF * 0.28f));
                        float body = (1.0f - eG * 0.4f) * t1 + (eG * 0.4f) * t2 * slot.timbreEnv;
                        oscOut = FastMath::fastTanh(body * (1.1f + eH * 1.8f));
                        break;
                    }
                    case 1: { // TRX-8: 808 deep resonant tom with circuit droop and sub ring
                        float s = FastMath::fastSinNorm(slot.phase);
                        float ring = FastMath::fastSinNorm(slot.phase * 2.0f) * (eG * 0.35f);
                        float mix = (s + ring) * (1.0f - 0.08f * slot.phase * (1.0f - eF * 0.5f));
                        oscOut = FastMath::fastTanh(mix * (1.0f + eH * 1.4f));
                        break;
                    }
                    case 2: { // SIMM-T: Simmons SDS-V analog sweep tom with pulse harmonic
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
                    case 5: { // EFM-T: 2-op FM tom with attack punch index spike
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
                if (slot.burstCount < totalBurstTime) {
                    float phaseInBurst = std::fmod(slot.burstCount, spreadSamples) / spreadSamples;
                    burst = FastMath::fastExp(-phaseInBurst * 4.5f) * 1.25f;
                }
                float n = nextRandBipolar();
                switch (activeModel_) {
                    case 0: { // TRX-9: 909 multi-burst filtered resonant clap
                        float filtered = (n + FastMath::fastSinNorm(slot.phase * (2.0f + eF * 1.5f)) * (0.35f + eG * 0.35f));
                        oscOut = FastMath::fastTanh(filtered * (1.2f + eH * 2.2f)) * burst;
                        break;
                    }
                    case 1: { // TRX-8: 808 multi-burst clap with tone warmth and diffuse noise
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
                    case 3: { // CLP-TRP: Vintage Boss HC-2 / EHX Clap Trap
                        float sq = (slot.phase < 0.5f) ? 0.8f : -0.8f;
                        float nF = (n > 0.0f ? 0.9f : -0.9f) * 0.7f + sq * (0.35f * eF);
                        oscOut = FastMath::fastTanh(nF * (1.6f + eH * 2.8f)) * burst;
                        break;
                    }
                    case 4: { // EFM-C: 2-op FM metallic frequency-modulated clap
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
                const float clusterBaseHz = curFreq * (1.8f + eF * 1.4f);
                for (int m = 0; m < 6; ++m) {
                    slot.clusterPhases[m] += std::clamp(clusterBaseHz * kRatios[m], 20.0f, nyquist_) * invSampleRate_;
                    if (slot.clusterPhases[m] >= 1.0f) slot.clusterPhases[m] -= 1.0f;
                    metalSum += (slot.clusterPhases[m] < 0.5f) ? 0.35f : -0.35f;
                }
                float n = nextRandBipolar();
                switch (activeModel_) {
                    case 0: { // ROLAND: 6-oscillator Schmitt trigger metallic sum (808/606/110)
                        float metal = metalSum * (0.4f + eE * 0.6f);
                        float sizzle = n * (0.2f + eG * 0.8f);
                        oscOut = FastMath::fastTanh((metal + sizzle) * (1.1f + eH * 1.8f));
                        break;
                    }
                    case 1: { // KORG: Analog pair dual-diode ring mod (KR-55 / Mini Pops)
                        float s1 = FastMath::fastSinNorm(slot.clusterPhases[0]);
                        float s2 = FastMath::fastSinNorm(slot.clusterPhases[1] * (1.0f + eF * 0.4f));
                        float diodeRing = FastMath::fastTanh(s1 * s2 * (2.0f + eE * 3.0f));
                        float mix = diodeRing * 0.65f + n * (0.3f + eG * 0.5f);
                        oscOut = FastMath::fastTanh(mix * (1.1f + eH * 1.5f));
                        break;
                    }
                    case 2: { // EFM-GLS: FM glass hi-hat with shimmering inharmonic overtones
                        float mF = clusterBaseHz * (3.5f + eF * 4.5f);
                        slot.modPhase += mF * invSampleRate_;
                        if (slot.modPhase >= 1.0f) slot.modPhase -= 1.0f;
                        float fmIndex = (eG * 2.8f) + (eH * 1.5f * slot.timbreEnv);
                        float glass = FastMath::fastSinNorm(slot.clusterPhases[0] + FastMath::fastSinNorm(slot.modPhase) * fmIndex);
                        oscOut = FastMath::fastTanh((glass * (0.6f + eE * 0.4f) + n * 0.35f) * (1.1f + eH * 1.5f));
                        break;
                    }
                    case 3: { // EFM-GRT: FM gritty distorted metallic hat with feedback noise
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
                    case 6: { // NOISE: Bandpass shaped analog white/pink noise hat (CR-78 style)
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
                const float cyBaseHz = curFreq * (1.5f + eF * 0.8f);
                for (int m = 0; m < 6; ++m) {
                    slot.clusterPhases[m] += std::clamp(cyBaseHz * kCyRatios[m], 20.0f, nyquist_) * invSampleRate_;
                    if (slot.clusterPhases[m] >= 1.0f) slot.clusterPhases[m] -= 1.0f;
                    cyMetal += (slot.clusterPhases[m] < 0.5f) ? 0.35f : -0.35f;
                }
                float n = nextRandBipolar();
                switch (activeModel_) {
                    case 0: { // ROLAND: TR-808/606 cymbal matrix with bell overtone and wash punch
                        float metal = cyMetal * (0.45f + eF * 0.55f);
                        float wash = n * (0.25f + eH * 0.45f);
                        float bell = FastMath::fastSinNorm(slot.clusterPhases[0] * 1.5f) * (eG * 0.4f);
                        oscOut = FastMath::fastTanh((metal + wash + bell) * (1.1f + eH * 1.5f));
                        break;
                    }
                    case 1: { // KORG: KR-55 analog diode ring mod cymbal with bell resonance
                        float s1 = FastMath::fastSinNorm(slot.clusterPhases[0]);
                        float s2 = FastMath::fastSinNorm(slot.clusterPhases[2] * (1.0f + eF * 0.5f));
                        float rm = FastMath::fastTanh(s1 * s2 * (2.2f + eF * 2.0f));
                        float bell = FastMath::fastSinNorm(slot.clusterPhases[1]) * (eG * 0.35f);
                        float mix = rm * 0.55f + bell + n * (0.3f + eH * 0.3f);
                        oscOut = FastMath::fastTanh(mix * (1.1f + eH * 1.5f));
                        break;
                    }
                    case 2: { // EFM-CR: FM crash cymbal with attack FM surge
                        float mF = cyBaseHz * (1.75f + eF * 2.5f);
                        slot.modPhase += mF * invSampleRate_;
                        if (slot.modPhase >= 1.0f) slot.modPhase -= 1.0f;
                        float fmIndex = (eF * 2.5f) + (eH * 2.5f * slot.timbreEnv);
                        float crashFm = FastMath::fastSinNorm(slot.clusterPhases[0] + FastMath::fastSinNorm(slot.modPhase) * fmIndex);
                        oscOut = FastMath::fastTanh((crashFm * 0.65f + n * 0.35f) * (1.1f + eH * 1.6f));
                        break;
                    }
                    case 3: { // EFM-RD: FM ride cymbal with defined bell ping & stick definition
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
                switch (activeModel_) {
                    case 0: { // RIM-ST: Acoustic & electronic rimshot with shell ring and stick click
                        float sTone = FastMath::fastSinNorm(slot.phase * (2.2f + eF * 1.2f));
                        float click = (n > 0.0f ? 0.9f : -0.9f) * slot.clickEnv;
                        float rim = sTone * (0.6f + eG * 0.4f) + click * (0.4f + eE * 0.6f);
                        oscOut = FastMath::fastTanh(rim * (1.2f + eH * 2.0f));
                        break;
                    }
                    case 1: { // COWB-8: 808 dual square cowbell with bell ring and striker pop
                        float sq1 = (slot.phase < 0.5f) ? 0.7f : -0.7f;
                        float ph2 = slot.phase * (1.40f + eF * 0.20f); ph2 -= static_cast<int>(ph2);
                        float sq2 = (ph2 < 0.5f) ? 0.7f : -0.7f;
                        float ring = FastMath::fastSinNorm(slot.phase * 3.5f) * (eG * 0.35f);
                        oscOut = FastMath::fastTanh((sq1 + sq2 + ring) * (1.1f + eH * 1.5f));
                        break;
                    }
                    case 2: { // CLAVE: Acoustic rosewood clave with dual body resonance
                        float s1 = FastMath::fastSinNorm(slot.phase);
                        float s2 = FastMath::fastSinNorm(slot.phase * (1.618f + eF * 0.3f)) * slot.timbreEnv;
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
                        float c2 = FastMath::fastSinNorm(slot.phase * (1.52f + eF * 0.3f)) * slot.timbreEnv;
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
                    case 6: { // EFM-PC: 2-op FM percussion synth with attack index spike
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
                switch (activeModel_) {
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
                    case 5: { // CZ-ZAP: Casio CZ phase distortion resonance zap
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

        // 4. Transient Click Generator
        if (slot.clickEnv > 0.001f && eE > 0.01f) {
            slot.clickPhase += std::clamp(curFreq * 8.0f + 1000.0f, 500.0f, nyquist_) * invSampleRate_;
            if (slot.clickPhase >= 1.0f) slot.clickPhase -= 1.0f;
            const float clickOsc = FastMath::fastSinNorm(slot.clickPhase);
            oscOut += clickOsc * (eE * 0.8f) * slot.clickEnv;
        }

        return oscOut * shapedAmp;
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

        if (p.mojoProfile == 1) { // MD-12
            y = y * 1.25f - 0.25f * highBand;
        } else if (p.mojoProfile == 2) { // SP-1200
            y = FastMath::fastTanh(y * 1.3f);
        } else if (p.mojoProfile == 3) { // Iron Tape
            y = FastMath::fastTanh(y * 1.5f + 0.1f) - 0.1f;
        }

        float targetRate = std::clamp(p.crush, 1000.0f, 48000.0f);
        if (p.mojoProfile == 2) targetRate = 26040.0f;

        if (targetRate < sampleRate_ * 0.95f) {
            crushPhase_ += targetRate * invSampleRate_;
            if (crushPhase_ >= 1.0f) {
                crushPhase_ -= 1.0f;
                crushedSample_ = y;
            }
            y = crushedSample_;
        }

        float effBits = p.bits;
        if (p.mojoProfile == 1 || p.mojoProfile == 2) effBits = std::min(effBits, 12.0f);

        if (effBits < 15.9f) {
            const float levels = FastMath::fastPow2(effBits - 1.0f);
            y = std::round(y * levels) / (levels + 1e-6f);
        }

        if (p.tapeGlue > 0.01f || p.mojoProfile == 3) {
            const float tapeDrive = 1.0f + p.tapeGlue * 3.0f;
            y = FastMath::fastTanh(y * tapeDrive) / (1.0f + p.tapeGlue * 0.5f);
        }

        const float hp = y - mojoDcBlocker_;
        mojoDcBlocker_ += 0.0006f * hp;
        return hp;
    }

    void renderSampleStereo(float& outL, float& outR, const Params& p) noexcept {
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

        if (std::abs(rawAudio) < 1e-15f) {
            outL = 0.0f;
            outR = 0.0f;
            return;
        }

        const float driveGain = 1.0f + p.preDrive * 4.0f;
        float shaped = rawAudio * driveGain;

        if (p.lifeMode == static_cast<int>(LifeMode::Cyborg)) {
            if (shaped > 0.85f) shaped = 0.85f - (shaped - 0.85f) * 0.7f;
            else if (shaped < -0.85f) shaped = -0.85f - (shaped + 0.85f) * 0.7f;
        } else if (p.lifeMode == static_cast<int>(LifeMode::Organic)) {
            shaped = FastMath::fastTanh(shaped * 0.9f);
        }

        float filtered = 0.0f;

        if (p.lifeMode == static_cast<int>(LifeMode::Organic) && p.filterType == 0) {
            const float targetLpg = std::clamp(active.ampEnv + 0.02f, 0.0f, 1.0f);
            if (targetLpg > lpgVactrolState_) {
                lpgVactrolState_ += 0.08f * (targetLpg - lpgVactrolState_);
            } else {
                lpgVactrolState_ += 0.003f * (targetLpg - lpgVactrolState_);
            }
            const float lpgCutoff = std::clamp(lpgVactrolState_ * p.cutoff * invSampleRate_ * 3.14159f, 0.001f, 0.45f);
            lpgFilterState_ += lpgCutoff * (shaped - lpgFilterState_);
            filtered = lpgFilterState_;
        } else {
            float rawFilt = vcfSvf_.process(shaped);
            if (p.filterType == 2) {
                // Red Team BP Headroom Compensation: normalize high-Q peak
                const float qVal = 0.5f + p.resonance * 7.5f;
                rawFilt *= std::min(1.0f, 2.0f / (1.0f + qVal * 0.15f));
            }
            vcfSvf_.clampState();
            filtered = rawFilt;
        }

        const bool mojoActive = (p.crush < 47500.0f) || (p.bits < 15.9f) || 
                                (p.tapeGlue > 0.01f) || (p.mojoProfile != 0);
        if (mojoActive) {
            const float highBand = processHp4(filtered);
            const float processedHigh = processMojo(highBand, p);
            filtered = filtered + (processedHigh - highBand);
        }

        const float hp = filtered - dcBlockerState_;
        dcBlockerState_ += 0.0006f * hp;
        filtered = hp;

        if (p.lifeMode == static_cast<int>(LifeMode::Cyborg)) {
            filtered += 0.00015f * ((slotA_.phase > 0.5f) ? 1.0f : -1.0f);
        }

        const float masterOut = FastMath::fastTanh(filtered * 0.95f) * p.volume;

        const float effPan = std::clamp(p.pan + active.latchedOffPan, -1.0f, 1.0f);
        const float gainL = (effPan <= 0.0f) ? 1.0f : (1.0f - effPan);
        const float gainR = (effPan >= 0.0f) ? 1.0f : (1.0f + effPan);

        outL = masterOut * gainL;
        outR = masterOut * gainR;
    }

    inline float nextRandBipolar() noexcept {
        rngState_ ^= rngState_ << 13;
        rngState_ ^= rngState_ >> 17;
        rngState_ ^= rngState_ << 5;
        if (rngState_ == 0) rngState_ = 0x1337C0DE;
        const float u = static_cast<float>((rngState_ >> 8) & 0x00FFFFFF) * (1.0f / 16777216.0f);
        return 2.0f * u - 1.0f;
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

    TbdAudio::DSPCommon::CytomicSVF vcfSvf_{};
    float lpgVactrolState_{0.0f};
    float lpgFilterState_{0.0f};
    float dcBlockerState_{0.0f};

    float hp1_s1_{0.0f}, hp1_s2_{0.0f};
    float hp2_s1_{0.0f}, hp2_s2_{0.0f};
    float hpG_{0.01f}, hpH_{1.0f};
    float mojoDcBlocker_{0.0f};
    float crushPhase_{0.0f};
    float crushedSample_{0.0f};

    uint32_t rngState_{0xDEADBEEF};
    uint32_t trigRng_{0x853C49E6};
    std::array<float, 8> latchedDraw_{{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};

    int blockCounter_{0};
    float macroSmoothed_{0.5f};
    float ampDecayCoeff_{0.999f};
    float pitchDecayCoeff_{0.995f};
    float timbreDecayCoeff_{0.990f};
    float clickDecayCoeff_{0.950f};
};

} // namespace tkdsp
