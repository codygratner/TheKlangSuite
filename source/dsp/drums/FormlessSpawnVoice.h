#pragma once

#include "FastMath.h"
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
        TM = 1,
        SD = 2,
        CP = 3,
        HH = 4,
        CY = 5,
        FX = 6
    };

    enum class LifeMode : int {
        Synthetic = 0,
        Cyborg    = 1,
        Organic   = 2
    };

    static constexpr int kTotalRoles = 7;
    static constexpr int kModelsPerRole = 16;
    static constexpr int kTotalModels = 112;

    static constexpr const char* kRoleNames[kTotalRoles] = {
        "BD", "TM", "SD", "CP", "HH", "CY", "FX"
    };

    static constexpr const char* kModelNames[kTotalRoles][kModelsPerRole] = {
        // BD (00..15)
        {"TRX-9", "TRX-B", "EFM-M", "MEMBR", "BOUNC", "THUD", "CLICK", "BOOM",
         "W-FOLD", "FM-SUB", "DIGI", "DRIVE", "SQUAR", "DISCR", "VCO-D", "LASER"},
        // TM (00..15)
        {"SINE-T", "EFM-T", "SHELL", "PING-L", "PING-H", "ATTACK", "PUNCH", "HEX-V",
         "DUAL-V", "VCF-T", "STRING", "DIGI-T", "FORMNT", "TONAL", "VCO-T", "GLITCH"},
        // SD (00..15)
        {"SNAP", "EFM-S", "WIRES", "HP-NOI", "CRISP", "SHELL", "SIZZL", "VCF-N",
         "KAR-S", "SUB-S", "6-SINE", "MARCH", "DISCR", "CRACK", "EXPLOD", "POISSN"},
        // CP (00..15)
        {"BURST", "EFM-C", "MULTI", "TRIPLE", "STAGGR", "DRIVE", "MULTI2", "DISCR",
         "RESON", "EXPLOD", "CRACK", "DIGI-C", "W-GUID", "MEMBR", "SHAPED", "VECTOR"},
        // HH (00..15)
        {"6-OSC", "BP-MET", "5-OSC", "PHASE", "RING-H", "EFM-H", "PLATE", "HP-SHR",
         "VCF-M", "DIGI-H", "FORMNT", "RING-6", "DISCR", "PING-H", "METAL", "SIZZL"},
        // CY (00..15)
        {"CRASH", "RING-C", "BELL-R", "SIZZL", "EXT-RG", "GONG", "PLATE", "IND-G",
         "W-CRSH", "DIGI-C", "CHIME", "LONG-S", "SHIMMR", "COSMIC", "NOISE", "GRANUL"},
        // FX (00..15)
        {"FM-MRP", "W-FOLD", "BT-PNG", "NOI-BT", "HARMON", "LASER", "SUB-DP", "SLICER",
         "SCREAM", "RING-M", "SHRIEK", "SHIFTR", "VORTEX", "PIPE", "CZ-ZAP", "CHAOS"}
    };

    // =========================================================================
    // 112-Model Calibrated Macro Decay Matrix (ACF-5 Calibrated)
    // =========================================================================
    // Base amplitude decay time constant tau in seconds (T60 approx 6.9 * tau).
    // Macro decay sweeps tau over [0.125x .. 8.0x].
    static constexpr float kModelBaseAmpDecaySec[kTotalRoles][kModelsPerRole] = {
        // BD:  TRX-9  TRX-B  EFM-M  MEMBR  BOUNC  THUD   CLICK  BOOM   W-FOLD FM-SUB DIGI   DRIVE  SQUAR  DISCR  VCO-D  LASER
        {       0.110f,0.085f,0.095f,0.140f,0.120f,0.070f,0.060f,0.280f,0.100f,0.200f,0.075f,0.130f,0.090f,0.105f,0.150f,0.090f },
        // TM:  SINE-T EFM-T  SHELL  PING-L PING-H ATTACK PUNCH  HEX-V  DUAL-V VCF-T  STRING DIGI-T FORMNT TONAL  VCO-T  GLITCH
        {       0.300f,0.220f,0.260f,0.380f,0.200f,0.180f,0.150f,0.240f,0.340f,0.280f,0.420f,0.200f,0.320f,0.360f,0.250f,0.120f },
        // SD:  SNAP   EFM-S  WIRES  HP-NOI CRISP  SHELL  SIZZL  VCF-N  KAR-S  SUB-S  6-SINE MARCH  DISCR  CRACK  EXPLOD POISSN
        {       0.100f,0.140f,0.200f,0.120f,0.090f,0.180f,0.240f,0.160f,0.220f,0.150f,0.170f,0.110f,0.140f,0.080f,0.350f,0.190f },
        // CP:  BURST  EFM-C  MULTI  TRIPLE STAGGR DRIVE  MULTI2 DISCR  RESON  EXPLOD CRACK  DIGI-C W-GUID MEMBR  SHAPED VECTOR
        {       0.140f,0.120f,0.180f,0.160f,0.200f,0.150f,0.220f,0.130f,0.260f,0.300f,0.080f,0.110f,0.240f,0.170f,0.190f,0.210f },
        // HH:  6-OSC  BP-MET 5-OSC  PHASE  RING-H EFM-H  PLATE  HP-SHR VCF-M  DIGI-H FORMNT RING-6 DISCR  PING-H METAL  SIZZL
        {       0.050f,0.040f,0.070f,0.090f,0.060f,0.080f,0.120f,0.030f,0.050f,0.020f,0.070f,0.100f,0.050f,0.150f,0.090f,0.130f },
        // CY:  CRASH  RING-C BELL-R SIZZL  EXT-RG GONG   PLATE  IND-G  W-CRSH DIGI-C CHIME  LONG-S SHIMMR COSMIC NOISE  GRANUL
        {       1.100f,1.400f,1.600f,0.800f,1.700f,1.800f,0.900f,0.600f,1.000f,0.400f,1.500f,1.300f,1.200f,1.600f,0.700f,0.950f },
        // FX:  FM-MRP W-FOLD BT-PNG NOI-BT HARMON LASER  SUB-DP SLICER SCREAM RING-M SHRIEK SHIFTR VORTEX PIPE   CZ-ZAP CHAOS
        {       0.300f,0.250f,0.350f,0.150f,0.400f,0.200f,0.600f,0.280f,0.450f,0.320f,0.380f,0.420f,0.700f,0.500f,0.140f,0.550f }
    };

    // Pitch envelope sensitivity to Macro Decay:
    // 0.0 = snappy pitch punch remains strictly locked to panel value; 1.0 = pitch dive stretches fully with macro.
    static constexpr float kModelPitchDecayResponse[kTotalRoles][kModelsPerRole] = {
        // BD
        {       0.08f, 0.05f, 0.10f, 0.15f, 0.25f, 0.00f, 0.00f, 0.12f, 0.08f, 0.20f, 0.03f, 0.05f, 0.05f, 0.06f, 0.10f, 1.00f },
        // TM
        {       0.30f, 0.25f, 0.30f, 0.40f, 0.20f, 0.10f, 0.10f, 0.25f, 0.35f, 0.30f, 0.55f, 0.15f, 0.20f, 0.25f, 0.30f, 0.05f },
        // SD
        {       0.10f, 0.15f, 0.05f, 0.02f, 0.05f, 0.15f, 0.08f, 0.10f, 0.30f, 0.20f, 0.10f, 0.00f, 0.05f, 0.00f, 0.35f, 0.05f },
        // CP
        {       0.00f, 0.10f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.10f, 0.00f, 0.00f, 0.00f, 0.00f, 0.15f, 0.00f, 0.00f },
        // HH
        {       0.00f, 0.00f, 0.00f, 0.10f, 0.00f, 0.05f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.15f, 0.00f, 0.00f },
        // CY
        {       0.05f, 0.05f, 0.10f, 0.00f, 0.05f, 0.35f, 0.05f, 0.00f, 0.00f, 0.00f, 0.05f, 0.00f, 0.15f, 0.30f, 0.00f, 0.10f },
        // FX
        {       0.60f, 0.30f, 0.80f, 0.00f, 0.20f, 1.00f, 0.95f, 0.15f, 0.50f, 0.40f, 0.70f, 0.30f, 0.65f, 0.25f, 1.00f, 0.35f }
    };

    // Timbre & filter modulation envelope sensitivity to Macro Decay:
    // 0.0 = fixed panel duration; 1.0 = full proportional tracking with macro.
    static constexpr float kModelTimbreDecayResponse[kTotalRoles][kModelsPerRole] = {
        // BD
        {       0.50f, 0.35f, 0.70f, 0.60f, 0.40f, 0.25f, 0.10f, 0.30f, 0.75f, 0.80f, 0.50f, 0.40f, 0.45f, 0.40f, 0.55f, 0.85f },
        // TM
        {       0.30f, 0.70f, 0.50f, 0.40f, 0.35f, 0.15f, 0.25f, 0.75f, 0.40f, 0.80f, 0.90f, 0.50f, 0.60f, 0.45f, 0.50f, 0.20f },
        // SD
        {       0.35f, 0.65f, 0.85f, 0.30f, 0.30f, 0.45f, 0.60f, 0.70f, 0.50f, 0.35f, 0.45f, 0.25f, 0.35f, 0.15f, 0.55f, 0.40f },
        // CP
        {       0.30f, 0.60f, 0.35f, 0.30f, 0.35f, 0.25f, 0.85f, 0.30f, 0.55f, 0.40f, 0.15f, 0.40f, 0.70f, 0.45f, 0.40f, 0.50f },
        // HH
        {       0.30f, 0.25f, 0.30f, 0.50f, 0.35f, 0.55f, 0.40f, 0.15f, 0.60f, 0.10f, 0.55f, 0.40f, 0.25f, 0.35f, 0.35f, 0.45f },
        // CY
        {       0.60f, 0.50f, 0.70f, 0.50f, 0.60f, 0.85f, 0.50f, 0.40f, 0.75f, 0.30f, 0.70f, 0.55f, 0.80f, 0.85f, 0.30f, 0.65f },
        // FX
        {       0.85f, 0.80f, 1.00f, 1.00f, 0.50f, 0.70f, 0.40f, 0.30f, 0.65f, 0.75f, 0.60f, 1.00f, 0.80f, 0.45f, 0.75f, 0.70f }
    };

    struct Params {
        // =====================================================================
        // Header Controls (3)
        // =====================================================================
        int role{0};             // 0=BD, 1=TM, 2=SD, 3=CP, 4=HH, 5=CY, 6=FX
        int model{0};            // 0..15
        int lifeMode{2};         // 0=Synthetic, 1=Cyborg, 2=Organic

        // =====================================================================
        // Card 1: Drum Core & Pitch Lab (8)
        // =====================================================================
        float pitch{55.0f};          // 20.0 .. 200.0 Hz
        float pitchDepth{24.0f};     // -48.0 .. +48.0 st
        float pitchDecay{25.0f};     // 2.0 .. 200.0 ms
        float pitchCurve{-0.5f};     // -1.0 .. +1.0
        float clickTune{4000.0f};    // 500.0 .. 12000.0 Hz
        float clickMix{0.35f};       // 0.0 .. 1.0
        float subLevel{0.50f};       // 0.0 .. 1.0
        float lowBoost{4.0f};        // 0.0 .. 12.0 dB

        // =====================================================================
        // Card 2: Model Timbre & Acoustic Lab (8)
        // =====================================================================
        float timbreX{0.25f};        // 0.0 .. 1.0
        float timbreY{0.35f};        // 0.0 .. 1.0
        float timbreZ{0.20f};        // 0.0 .. 1.0
        float timbreT{45.0f};        // 2.0 .. 800.0 ms
        float fmMorph{0.0f};         // 0.0 .. 1.0
        float damp{0.30f};           // 0.0 .. 1.0
        float pan{0.0f};             // -1.0 .. +1.0
        float velSens{0.70f};        // 0.0 .. 1.0

        // =====================================================================
        // Card 3 Pod 3: VCF, Universal Mojo Strip & Master Output (9)
        // =====================================================================
        float cutoff{2000.0f};       // 20.0 .. 20000.0 Hz
        float resonance{0.30f};      // 0.0 .. 1.0
        int   vcfMode{0};            // 0=24dB Diode, 1=Buchla LPG
        float preDrive{0.15f};       // 0.0 .. 1.0
        float crush{48000.0f};       // 1000.0 .. 48000.0 Hz
        float bits{16.0f};           // 4.0 .. 16.0 bits
        float tapeGlue{0.0f};        // 0.0 .. 1.0
        int   mojoProfile{0};        // 0=Clean, 1=MD-12, 2=SP-1200, 3=Iron Tape
        float mojoMix{0.0f};         // 0.0 .. 100.0 %

        // =====================================================================
        // Card 3 Pod 1 & 2: Stepped Random & Master Altar (10)
        // =====================================================================
        // Row 1: Timbre Targets (0..100 %)
        float randTX{0.0f};          // 0.0 .. 100.0 %
        float randTY{0.0f};          // 0.0 .. 100.0 %
        float randTZ{0.0f};          // 0.0 .. 100.0 %
        float randTT{0.0f};          // 0.0 .. 100.0 %

        // Row 2: Acoustic & Core Targets
        float randPitch{0.0f};       // 0.0 .. 12.0 st
        float randCutoff{0.0f};      // 0.0 .. 100.0 %
        float randDecay{0.0f};       // 0.0 .. 100.0 %
        float randPan{0.0f};         // 0.0 .. 100.0 %

        float macroDecay{0.5f};      // 0.0 .. 1.0 (Master Macro)
        float volume{0.85f};         // 0.0 .. 1.0
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

        for (auto& s : filterState_) s = 0.0f;
        lpgVactrolState_ = 0.0f;
        lpgFilterState_ = 0.0f;
        lowBoostState_ = 0.0f;
        dcBlockerState_ = 0.0f;
        dampState_ = 0.0f;

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

        const float effVelocity = (1.0f - p.velSens) + p.velSens * velocity;

        // 1. Latch Pending Role & Model on Note-On
        activeRole_  = pendingRole_;
        activeModel_ = pendingModel_;

        // 2. Advance Trigger PRNG and Latch 8-Axis Humanization Parameters
        const float d[8] = {
            p.randTX * 0.01f,
            p.randTY * 0.01f,
            p.randTZ * 0.01f,
            p.randTT * 0.01f,
            p.randPitch,
            p.randCutoff * 0.01f,
            p.randDecay * 0.01f,
            p.randPan * 0.01f
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
        newSlot.ampEnv = std::clamp(effVelocity, 0.001f, 1.5f);
        newSlot.pitchEnv = 1.0f;
        newSlot.clickEnv = 1.0f;
        newSlot.timbreEnv = 1.0f;

        newSlot.latchedOffX = (d[0] > 0.0f) ? latchedDraw_[0] * d[0] : 0.0f;
        newSlot.latchedOffY = (d[1] > 0.0f) ? latchedDraw_[1] * d[1] : 0.0f;
        newSlot.latchedOffZ = (d[2] > 0.0f) ? latchedDraw_[2] * d[2] : 0.0f;
        newSlot.latchedTScale = (d[3] > 0.0f) ? FastMath::fastPow2(2.0f * latchedDraw_[3] * d[3]) : 1.0f;
        newSlot.latchedPitchOffsetSt = (d[4] > 0.0f) ? latchedDraw_[4] * d[4] : 0.0f;
        newSlot.latchedDecayScale = (d[6] > 0.0f) ? std::clamp(FastMath::fastPow2(1.5f * latchedDraw_[6] * d[6]), 0.25f, 4.0f) : 1.0f;

        const float cutoffOct = (d[5] > 0.0f) ? 3.0f * latchedDraw_[5] * d[5] : 0.0f;
        effCutoffHz_ = std::clamp(p.cutoff * FastMath::fastPow2(cutoffOct), 20.0f, nyquist_);
        panOff_ = (d[7] > 0.0f) ? latchedDraw_[7] * d[7] : 0.0f;

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

        // Hardwired Permanent 1V/Oct Chromatic Key Tracking
        float baseFreq = p.pitch;
        if (noteHz > 10.0f) {
            const float pitchOffsetSemis = (p.pitch - 55.0f) * (12.0f / 45.0f);
            baseFreq = noteHz * FastMath::fastPow2(pitchOffsetSemis * (1.0f / 12.0f));
        }
        if (std::abs(newSlot.latchedPitchOffsetSt) > 0.001f) {
            baseFreq *= FastMath::fastPow2(newSlot.latchedPitchOffsetSt * (1.0f / 12.0f));
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
        float latchedPitchOffsetSt{0.0f};
        float latchedDecayScale{1.0f};
        float latchedOffX{0.0f};
        float latchedOffY{0.0f};
        float latchedOffZ{0.0f};
        float latchedTScale{1.0f};
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

        // Role- and Model-calibrated base decay scaling (112-Model SSoT Matrix)
        const float macroLog2 = FastMath::fastLog2(std::max(macroMult, 1e-6f));

        const int r = std::clamp(activeRole_,  0, kTotalRoles - 1);
        const int m = std::clamp(activeModel_, 0, kModelsPerRole - 1);

        const float pitchResp  = kModelPitchDecayResponse[r][m];
        const float timbreResp = kModelTimbreDecayResponse[r][m];

        // Partial macro application: macroMult^response (response 0 -> 1.0 neutral, response 1 -> full macroMult)
        const float pitchMacro  = FastMath::fastPow2(pitchResp  * macroLog2);
        const float timbreMacro = FastMath::fastPow2(timbreResp * macroLog2);

        // Only fully-responsive sweep models (e.g. LASER, CZ-ZAP, SUB-DP) may sweep pitch past 350 ms
        const float pitchMaxSec = (pitchResp >= 0.9f) ? 1.200f : 0.350f;

        const float effAmpDecaySec    = std::clamp(kModelBaseAmpDecaySec[r][m] * macroMult, 0.003f, 4.500f);
        const float effPitchDecaySec  = std::clamp(p.pitchDecay * 0.001f * pitchMacro,  0.002f, pitchMaxSec);
        const float effTimbreDecaySec = std::clamp(p.timbreT    * 0.001f * timbreMacro, 0.002f, 1.200f);
        const float effClickDecaySec  = std::clamp(0.002f * FastMath::fastPow2(macroSmoothed_ * 1.2f), 0.0008f, 0.006f);

        ampDecayCoeff_   = FastMath::fastExp(-invSampleRate_ / effAmpDecaySec);
        pitchDecayCoeff_ = FastMath::fastExp(-invSampleRate_ / effPitchDecaySec);
        timbreDecayCoeff_= FastMath::fastExp(-invSampleRate_ / effTimbreDecaySec);
        clickDecayCoeff_ = FastMath::fastExp(-invSampleRate_ / effClickDecaySec);

        // VCF Cutoff Pre-Warping with Latched Stepped Random Modulation
        const float rawCutoff = std::clamp(effCutoffHz_, 20.0f, nyquist_);
        const float normCutoff = rawCutoff * invSampleRate_;
        const float wa = 2.0f * sampleRate_ * std::tan(normCutoff * 3.1415926535f);
        vcfG_ = wa * 0.5f * invSampleRate_;
        vcfReso_ = std::clamp(p.resonance * 3.8f, 0.0f, 3.85f);

        lowBoostGain_ = FastMath::fastPow2(p.lowBoost * (1.0f / 6.0f));

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

        // 1. Pitch Envelope Dive with Pitch Curve Shaping
        float shapedPitchEnv = slot.pitchEnv;
        if (p.pitchCurve < -0.01f) {
            const float k = -p.pitchCurve * 3.0f;
            shapedPitchEnv = FastMath::fastTanh(shapedPitchEnv * (1.0f + k)) / (FastMath::fastTanh(1.0f + k) + 1e-6f);
        } else if (p.pitchCurve > 0.01f) {
            const float k = p.pitchCurve;
            shapedPitchEnv = (1.0f - k) * shapedPitchEnv + k * (shapedPitchEnv * shapedPitchEnv);
        }

        const float pitchDepth = p.pitchDepth;
        const float pitchMult = FastMath::fastPow2((pitchDepth * shapedPitchEnv) * (1.0f / 12.0f));
        const float curFreq = std::clamp(slot.baseHz * pitchMult, 10.0f, nyquist_);

        // Advance Recursive Envelopes (high clamp ceiling 0.9999995f allows long tails > 4s)
        const float dScale = slot.latchedDecayScale;
        const float tScale = dScale * slot.latchedTScale;
        slot.pitchEnv  *= std::clamp(FastMath::fastPow2(FastMath::fastLog2(pitchDecayCoeff_) / dScale), 0.80f, 0.9999995f);
        slot.ampEnv    *= std::clamp(FastMath::fastPow2(FastMath::fastLog2(ampDecayCoeff_) / dScale), 0.80f, 0.9999995f);
        slot.timbreEnv *= std::clamp(FastMath::fastPow2(FastMath::fastLog2(timbreDecayCoeff_) / tScale), 0.80f, 0.9999995f);
        slot.clickEnv  *= clickDecayCoeff_;

        // 2. Timbre Modulation Parameters (with 8-Axis Latched Offsets)
        const float tX = std::clamp(p.timbreX + slot.latchedOffX, 0.0f, 1.0f);
        const float tY = std::clamp((p.timbreY + slot.latchedOffY) * slot.timbreEnv, 0.0f, 1.0f);
        const float tZ = std::clamp((p.timbreZ + slot.latchedOffZ) * slot.timbreEnv, 0.0f, 1.0f);

        // Advance Main Oscillator Phase
        slot.phase += curFreq * invSampleRate_;
        if (slot.phase >= 1.0f) slot.phase -= 1.0f;

        // 3. Synthesis Core Dispatch (7 Roles x 16 Models = 112 Sovereign Models)
        float oscOut = 0.0f;

        switch (activeRole_) {
            case 0: { // BD (Bass Drum)
                switch (activeModel_) {
                    case 0: { // TRX-9: Classic 909-style bridged-T punchy sine
                        float s = FastMath::fastSinNorm(slot.phase);
                        oscOut = FastMath::fastTanh(s * (1.0f + tY * 1.5f));
                        break;
                    }
                    case 1: { // TRX-B: Hard acoustic kick
                        float s = FastMath::fastSinNorm(slot.phase);
                        float asym = tX * 0.35f;
                        oscOut = FastMath::fastTanh(s * 2.0f + asym) - asym * 0.5f;
                        break;
                    }
                    case 2: { // EFM-M: 2-op metallic FM kick
                        float mF = curFreq * (1.0f + tX * 4.0f);
                        slot.modPhase += mF * invSampleRate_;
                        if (slot.modPhase >= 1.0f) slot.modPhase -= 1.0f;
                        float mod = FastMath::fastSinNorm(slot.modPhase) * (tY * 3.0f);
                        oscOut = FastMath::fastSinNorm(slot.phase + mod * 0.159f);
                        break;
                    }
                    case 3: { // MEMBR: Inharmonic membrane
                        float s1 = FastMath::fastSinNorm(slot.phase);
                        float s2 = FastMath::fastSinNorm(slot.phase * 1.618f);
                        oscOut = 0.7f * s1 + 0.3f * s2 * tX;
                        break;
                    }
                    case 4: { // BOUNC: 606 bouncy low kick
                        float s = FastMath::fastSinNorm(slot.phase);
                        oscOut = s * (1.0f - 0.2f * slot.phase);
                        break;
                    }
                    case 5: { // THUD: DR-110 punchy thud
                        float s = FastMath::fastSinNorm(slot.phase);
                        oscOut = FastMath::fastTanh(s * (1.2f + tY * 2.0f));
                        break;
                    }
                    case 6: { // CLICK: 909 dual envelope click kick
                        float s = FastMath::fastSinNorm(slot.phase);
                        oscOut = s + slot.clickEnv * (tX * 0.6f);
                        break;
                    }
                    case 7: { // BOOM: 808 deep sub boom
                        oscOut = FastMath::fastSinNorm(slot.phase);
                        break;
                    }
                    case 8: { // W-FOLD: Perkons wavefold kick
                        float saw = 2.0f * slot.phase - 1.0f;
                        oscOut = FastMath::fastSinNorm(saw * (1.0f + tX * 4.0f));
                        break;
                    }
                    case 9: { // FM-SUB: Deep FM drop
                        float mod = FastMath::fastSinNorm(slot.phase * 0.5f) * (tY * 2.5f);
                        oscOut = FastMath::fastSinNorm(slot.phase + mod * 0.159f);
                        break;
                    }
                    case 10: { // DIGI: Digital multi-harmonic kick
                        float s = FastMath::fastSinNorm(slot.phase);
                        float s3 = FastMath::fastSinNorm(slot.phase * 3.0f);
                        oscOut = s + tX * 0.5f * s3;
                        break;
                    }
                    case 11: { // DRIVE: Overdrive parallel kick
                        float s = FastMath::fastSinNorm(slot.phase);
                        oscOut = FastMath::fastTanh(s * (1.5f + tX * 3.0f));
                        break;
                    }
                    case 12: { // SQUAR: Square sub kick
                        oscOut = (slot.phase < 0.5f) ? 0.8f : -0.8f;
                        oscOut = FastMath::fastTanh(oscOut * (1.0f + tY));
                        break;
                    }
                    case 13: { // DISCR: Discrete transistor kick
                        float s = FastMath::fastSinNorm(slot.phase);
                        oscOut = s > 0.0f ? s * 1.2f : s * 0.8f;
                        break;
                    }
                    case 14: { // VCO-D: Continuous shaped VCO kick
                        float tri = (slot.phase < 0.5f) ? (4.0f * slot.phase - 1.0f) : (3.0f - 4.0f * slot.phase);
                        float sine = FastMath::fastSinNorm(slot.phase);
                        oscOut = (1.0f - tX) * sine + tX * tri;
                        break;
                    }
                    default: { // LASER: Fast laser pitch sweep
                        oscOut = FastMath::fastSinNorm(slot.phase);
                        break;
                    }
                }
                break;
            }

            case 1: { // TM (Tom)
                float bodySine = FastMath::fastSinNorm(slot.phase);
                float shellSine = FastMath::fastSinNorm(slot.phase * (1.4f + tX * 0.4f));
                switch (activeModel_) {
                    case 0:  oscOut = bodySine; break; // SINE-T
                    case 1: { // EFM-T: FM tom
                        float mF = curFreq * (1.5f + tX * 2.0f);
                        slot.modPhase += mF * invSampleRate_;
                        if (slot.modPhase >= 1.0f) slot.modPhase -= 1.0f;
                        oscOut = FastMath::fastSinNorm(slot.phase + FastMath::fastSinNorm(slot.modPhase) * tY);
                        break;
                    }
                    case 2:  oscOut = 0.65f * bodySine + 0.35f * shellSine; break; // SHELL
                    case 3:  oscOut = bodySine * (1.0f - 0.3f * slot.phase); break; // PING-L
                    case 4:  oscOut = FastMath::fastSinNorm(slot.phase * 1.5f); break; // PING-H
                    case 5:  oscOut = bodySine + slot.clickEnv * 0.5f; break; // ATTACK
                    case 6:  oscOut = FastMath::fastTanh(bodySine * 1.8f); break; // PUNCH
                    case 7: { // HEX-V: Wavefold tom
                        float saw = 2.0f * slot.phase - 1.0f;
                        oscOut = FastMath::fastSinNorm(saw * (1.0f + tX * 3.0f));
                        break;
                    }
                    case 8: { // DUAL-V: Beating dual VCO
                        float v2 = FastMath::fastSinNorm(slot.phase * 1.02f);
                        oscOut = 0.5f * (bodySine + v2);
                        break;
                    }
                    case 9:  oscOut = FastMath::fastTanh(bodySine * (1.0f + tY * 2.5f)); break; // VCF-T
                    case 10: oscOut = bodySine * (1.0f - slot.timbreEnv * 0.5f); break; // STRING
                    case 11: oscOut = bodySine + 0.3f * FastMath::fastSinNorm(slot.phase * 5.0f) * tX; break; // DIGI-T
                    case 12: oscOut = 0.5f * bodySine + 0.5f * shellSine * tY; break; // FORMNT
                    case 13: oscOut = bodySine * (0.8f + 0.2f * FastMath::fastSinNorm(slot.phase * 2.0f)); break; // TONAL
                    case 14: oscOut = (slot.phase < 0.5f) ? (4.0f * slot.phase - 1.0f) : (3.0f - 4.0f * slot.phase); break; // VCO-T
                    default: oscOut = FastMath::fastTanh(bodySine * 3.0f) * (nextRandBipolar() * 0.15f + 0.85f); break; // GLITCH
                }
                break;
            }

            case 2: { // SD (Snare)
                float tone = FastMath::fastSinNorm(slot.phase);
                float tone2 = FastMath::fastSinNorm(slot.phase * 1.6f);
                float noise = nextRandBipolar();
                switch (activeModel_) {
                    case 0:  oscOut = (1.0f - tX) * (0.6f * tone + 0.4f * tone2) + tX * noise; break; // SNAP
                    case 1: { // EFM-S
                        float mF = curFreq * (2.1f + tX * 3.0f);
                        slot.modPhase += mF * invSampleRate_;
                        if (slot.modPhase >= 1.0f) slot.modPhase -= 1.0f;
                        oscOut = 0.5f * FastMath::fastSinNorm(slot.phase + FastMath::fastSinNorm(slot.modPhase) * tY) + 0.5f * noise;
                        break;
                    }
                    case 2:  oscOut = 0.4f * tone + 0.6f * noise * (0.7f + 0.3f * slot.timbreEnv); break; // WIRES
                    case 3:  oscOut = 0.2f * tone + 0.8f * noise; break; // HP-NOI (606)
                    case 4:  oscOut = 0.3f * tone + 0.7f * FastMath::fastTanh(noise * 1.5f); break; // CRISP (DR-110)
                    case 5:  oscOut = 0.5f * (0.7f * tone + 0.3f * tone2) + 0.5f * noise; break; // SHELL (909)
                    case 6:  oscOut = 0.4f * tone + 0.6f * noise; break; // SIZZL (808)
                    case 7:  oscOut = FastMath::fastTanh((tone + noise) * (1.0f + tY * 2.0f)); break; // VCF-N
                    case 8:  oscOut = 0.5f * tone + 0.5f * noise * slot.ampEnv; break; // KAR-S
                    case 9:  oscOut = 0.3f * tone + 0.7f * (noise > 0.0f ? 0.7f : -0.7f); break; // SUB-S
                    case 10: oscOut = 0.6f * tone + 0.4f * noise * tX; break; // 6-SINE
                    case 11: oscOut = 0.15f * tone + 0.85f * noise; break; // MARCH
                    case 12: oscOut = (tone > 0.0f ? tone * 1.3f : tone * 0.7f) + 0.5f * noise; break; // DISCR
                    case 13: oscOut = slot.clickEnv * tone + noise; break; // CRACK
                    case 14: oscOut = FastMath::fastTanh((tone + noise) * 3.0f); break; // EXPLOD
                    default: oscOut = 0.3f * tone + 0.7f * (noise * (slot.ampEnv > 0.5f ? 1.0f : 0.3f)); break; // POISSN
                }
                break;
            }

            case 3: { // CP (Clap)
                float noise = nextRandBipolar();
                // Multi-burst envelope for authentic claps
                slot.burstCount += 1.0f;
                float burst = 1.0f;
                if (slot.burstCount < 0.035f * sampleRate_) {
                    int subStep = static_cast<int>(slot.burstCount / (0.008f * sampleRate_));
                    burst = (subStep % 2 == 0) ? 1.0f : 0.2f;
                }
                switch (activeModel_) {
                    case 0:  oscOut = noise * burst; break; // BURST
                    case 1: { // EFM-C
                        float fm = FastMath::fastSinNorm(slot.phase * 5.0f) * tY;
                        oscOut = (noise + fm) * burst;
                        break;
                    }
                    case 2:  oscOut = noise * (burst * 0.8f + 0.2f); break; // MULTI
                    case 3:  oscOut = noise * burst; break; // TRIPLE (808)
                    case 4:  oscOut = FastMath::fastTanh(noise * 1.5f) * burst; break; // STAGGR (909)
                    case 5:  oscOut = FastMath::fastTanh(noise * 3.0f * burst); break; // DRIVE
                    case 6:  oscOut = noise * (0.5f * burst + 0.5f * slot.timbreEnv); break; // MULTI2
                    case 7:  oscOut = (noise > 0.0f ? noise * 1.2f : noise * 0.8f) * burst; break; // DISCR
                    case 8:  oscOut = (noise + 0.3f * FastMath::fastSinNorm(slot.phase * 2.0f)) * burst; break; // RESON
                    case 9:  oscOut = FastMath::fastTanh(noise * 4.0f) * burst; break; // EXPLOD
                    case 10: oscOut = (noise > 0.3f ? 1.0f : (noise < -0.3f ? -1.0f : 0.0f)) * burst; break; // CRACK
                    case 11: oscOut = noise * burst * (1.0f + tX); break; // DIGI-C
                    case 12: oscOut = noise * burst; break; // W-GUID
                    case 13: oscOut = (noise * 0.7f + 0.3f * FastMath::fastSinNorm(slot.phase)) * burst; break; // MEMBR
                    case 14: oscOut = FastMath::fastTanh(noise * 2.0f) * burst; break; // SHAPED
                    default: oscOut = noise * burst * (nextRandBipolar() * 0.2f + 0.8f); break; // VECTOR
                }
                break;
            }

            case 4: { // HH (Hi-Hat)
                // 6-Osc Inharmonic Metallic Matrix (205.3, 304.4, 369.6, 522.7, 540.0, 800.0 Hz)
                constexpr float kRatios[6] = { 1.0f, 1.4827f, 1.8003f, 2.5460f, 2.6303f, 3.8967f };
                float metalSum = 0.0f;
                const float clusterBaseHz = curFreq * 3.73f;
                for (int m = 0; m < 6; ++m) {
                    slot.clusterPhases[m] += std::clamp(clusterBaseHz * kRatios[m], 20.0f, nyquist_) * invSampleRate_;
                    if (slot.clusterPhases[m] >= 1.0f) slot.clusterPhases[m] -= 1.0f;
                    metalSum += (slot.clusterPhases[m] < 0.5f) ? 0.35f : -0.35f;
                }
                float noise = nextRandBipolar();
                switch (activeModel_) {
                    case 0:  oscOut = metalSum * 0.7f + noise * 0.3f; break; // 6-OSC (808/909)
                    case 1:  oscOut = noise * 0.85f + metalSum * 0.15f; break; // BP-MET
                    case 2:  oscOut = metalSum; break; // 5-OSC
                    case 3:  oscOut = metalSum * 0.5f + FastMath::fastSinNorm(slot.phase * 8.0f) * 0.5f; break; // PHASE
                    case 4:  oscOut = metalSum * (noise > 0.0f ? 1.0f : -1.0f); break; // RING-H
                    case 5: { // EFM-H
                        float mF = clusterBaseHz * 1.5f;
                        slot.modPhase += mF * invSampleRate_;
                        if (slot.modPhase >= 1.0f) slot.modPhase -= 1.0f;
                        oscOut = metalSum * 0.6f + FastMath::fastSinNorm(slot.modPhase) * 0.4f;
                        break;
                    }
                    case 6:  oscOut = metalSum * 0.4f + noise * 0.6f; break; // PLATE
                    case 7:  oscOut = noise; break; // HP-SHR
                    case 8:  oscOut = FastMath::fastTanh(metalSum * 1.5f); break; // VCF-M
                    case 9:  oscOut = (metalSum > 0.0f ? 0.8f : -0.8f); break; // DIGI-H
                    case 10: oscOut = metalSum * 0.5f + noise * 0.5f; break; // FORMNT
                    case 11: oscOut = metalSum * FastMath::fastSinNorm(slot.phase * 4.0f); break; // RING-6
                    case 12: oscOut = (metalSum > 0.0f ? metalSum * 1.2f : metalSum * 0.8f); break; // DISCR
                    case 13: oscOut = metalSum + slot.clickEnv * 0.5f; break; // PING-H
                    case 14: oscOut = FastMath::fastTanh((metalSum + noise) * 2.0f); break; // METAL
                    default: oscOut = noise * 0.7f + metalSum * 0.3f; break; // SIZZL
                }
                break;
            }

            case 5: { // CY (Cymbal)
                float noise = nextRandBipolar();
                constexpr float kRatios[6] = { 1.0f, 1.34f, 1.71f, 2.22f, 2.87f, 3.45f };
                float metalSum = 0.0f;
                const float clusterBaseHz = curFreq * 2.85f;
                for (int m = 0; m < 6; ++m) {
                    slot.clusterPhases[m] += std::clamp(clusterBaseHz * kRatios[m], 20.0f, nyquist_) * invSampleRate_;
                    if (slot.clusterPhases[m] >= 1.0f) slot.clusterPhases[m] -= 1.0f;
                    metalSum += (slot.clusterPhases[m] < 0.5f) ? 0.35f : -0.35f;
                }
                switch (activeModel_) {
                    case 0:  oscOut = 0.5f * metalSum + 0.5f * noise; break; // CRASH
                    case 1:  oscOut = metalSum * (noise > 0.0f ? 0.8f : -0.8f); break; // RING-C
                    case 2:  oscOut = FastMath::fastSinNorm(slot.phase * 4.0f) * 0.6f + metalSum * 0.4f; break; // BELL-R
                    case 3:  oscOut = noise * 0.75f + metalSum * 0.25f; break; // SIZZL
                    case 4:  oscOut = metalSum * 0.8f + noise * 0.2f; break; // EXT-RG
                    case 5: { // GONG
                        float gF = curFreq * 0.75f;
                        oscOut = FastMath::fastSinNorm(slot.phase * 0.75f) * 0.6f + metalSum * 0.4f;
                        break;
                    }
                    case 6:  oscOut = metalSum * 0.6f + noise * 0.4f; break; // PLATE
                    case 7:  oscOut = FastMath::fastTanh((metalSum + noise) * 2.0f); break; // IND-G
                    case 8: { // W-CRSH
                        float folded = FastMath::fastSinNorm(metalSum * (1.0f + tX * 3.0f));
                        oscOut = 0.6f * folded + 0.4f * noise;
                        break;
                    }
                    case 9:  oscOut = (metalSum > 0.0f ? 0.7f : -0.7f) + 0.3f * noise; break; // DIGI-C
                    case 10: oscOut = FastMath::fastSinNorm(slot.phase * 6.0f) * 0.7f + 0.3f * metalSum; break; // CHIME
                    case 11: oscOut = 0.4f * metalSum + 0.6f * noise; break; // LONG-S
                    case 12: oscOut = metalSum * FastMath::fastSinNorm(slot.phase * 2.0f) + 0.3f * noise; break; // SHIMMR
                    case 13: oscOut = FastMath::fastSinNorm(slot.phase * 3.1415f) * 0.5f + 0.5f * noise; break; // COSMIC
                    case 14: oscOut = noise; break; // NOISE
                    default: oscOut = (0.5f * metalSum + 0.5f * noise) * (nextRandBipolar() * 0.15f + 0.85f); break; // GRANUL
                }
                break;
            }

            default: { // FX (Special FX)
                float noise = nextRandBipolar();
                switch (activeModel_) {
                    case 0: { // FM-MRP
                        float mF = curFreq * (0.5f + tX * 7.5f);
                        slot.modPhase += mF * invSampleRate_;
                        if (slot.modPhase >= 1.0f) slot.modPhase -= 1.0f;
                        oscOut = FastMath::fastSinNorm(slot.phase + FastMath::fastSinNorm(slot.modPhase) * (tY * 4.0f));
                        break;
                    }
                    case 1: { // W-FOLD
                        float saw = 2.0f * slot.phase - 1.0f;
                        oscOut = FastMath::fastSinNorm(saw * (1.0f + tX * 6.0f));
                        break;
                    }
                    case 2:  oscOut = FastMath::fastSinNorm(slot.phase) * (1.0f - slot.timbreEnv * 0.8f); break; // BT-PNG
                    case 3:  oscOut = noise * slot.timbreEnv; break; // NOI-BT
                    case 4:  oscOut = 0.5f * FastMath::fastSinNorm(slot.phase) + 0.5f * FastMath::fastSinNorm(slot.phase * 3.0f); break; // HARMON
                    case 5:  oscOut = FastMath::fastSinNorm(slot.phase); break; // LASER
                    case 6:  oscOut = FastMath::fastSinNorm(slot.phase * 0.5f); break; // SUB-DP
                    case 7: { // SLICER
                        float pulse = (slot.phase < 0.25f || (slot.phase > 0.5f && slot.phase < 0.75f)) ? 1.0f : 0.0f;
                        oscOut = FastMath::fastSinNorm(slot.phase * 2.0f) * pulse;
                        break;
                    }
                    case 8:  oscOut = FastMath::fastTanh(FastMath::fastSinNorm(slot.phase) * 5.0f); break; // SCREAM
                    case 9:  oscOut = FastMath::fastSinNorm(slot.phase) * FastMath::fastSinNorm(slot.phase * (1.0f + tX * 4.0f)); break; // RING-M
                    case 10: oscOut = FastMath::fastSinNorm(slot.phase * 8.0f); break; // SHRIEK
                    case 11: oscOut = FastMath::fastSinNorm(slot.phase * (1.0f + slot.timbreEnv * 2.0f)); break; // SHIFTR
                    case 12: oscOut = FastMath::fastTanh((FastMath::fastSinNorm(slot.phase) + noise) * 2.5f); break; // VORTEX
                    case 13: oscOut = FastMath::fastSinNorm(slot.phase) * (1.0f - 0.5f * slot.phase); break; // PIPE
                    case 14: oscOut = FastMath::fastSinNorm(slot.phase * (1.0f + slot.phase * 4.0f)); break; // CZ-ZAP
                    default: oscOut = noise * FastMath::fastTanh(FastMath::fastSinNorm(slot.phase) * 4.0f); break; // CHAOS
                }
                break;
            }
        }

        // 4. Dedicated Sub-Bass Sine Layering
        if (p.subLevel > 0.01f) {
            slot.subPhase += curFreq * invSampleRate_;
            if (slot.subPhase >= 1.0f) slot.subPhase -= 1.0f;
            const float subSine = FastMath::fastSinNorm(slot.subPhase);
            oscOut += subSine * p.subLevel;
        }

        // 5. Transient Click Burst Generator
        if (slot.clickEnv > 0.001f && p.clickMix > 0.01f) {
            slot.clickPhase += std::clamp(p.clickTune, 500.0f, nyquist_) * invSampleRate_;
            if (slot.clickPhase >= 1.0f) slot.clickPhase -= 1.0f;
            const float clickOsc = FastMath::fastSinNorm(slot.clickPhase);
            oscOut += clickOsc * (p.clickMix * 1.5f) * slot.clickEnv;
        }

        // 6. Body Damping (Dynamic 1-pole HF loss)
        if (p.damp > 0.01f) {
            const float dampCutoff = std::clamp((1.0f - p.damp * (1.0f - slot.ampEnv)) * 0.45f, 0.05f, 0.45f);
            dampState_ += dampCutoff * (oscOut - dampState_);
            oscOut = dampState_;
        }

        return oscOut * slot.ampEnv;
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

        if (p.vcfMode == 1 || p.lifeMode == static_cast<int>(LifeMode::Organic)) {
            const float targetLpg = std::clamp(active.ampEnv + 0.02f, 0.0f, 1.0f);
            if (targetLpg > lpgVactrolState_) {
                lpgVactrolState_ += 0.08f * (targetLpg - lpgVactrolState_);
            } else {
                lpgVactrolState_ += 0.003f * (targetLpg - lpgVactrolState_);
            }
            const float lpgCutoff = std::clamp(lpgVactrolState_ * effCutoffHz_ * invSampleRate_ * 3.14159f, 0.001f, 0.45f);
            lpgFilterState_ += lpgCutoff * (shaped - lpgFilterState_);
            filtered = lpgFilterState_;
        } else {
            const float g = std::clamp(vcfG_, 0.001f, 0.95f);
            const float h = g / (1.0f + g);
            float u = shaped - vcfReso_ * filterState_[3];
            u = FastMath::fastTanh(u);

            for (int stage = 0; stage < 4; ++stage) {
                const float in = (stage == 0) ? u : filterState_[stage - 1];
                const float v = (in - filterState_[stage]) * h;
                filterState_[stage] += 2.0f * v;
            }
            filtered = filterState_[3];
        }

        if (p.lowBoost > 0.01f) {
            const float lowCutoff = 80.0f * invSampleRate_;
            lowBoostState_ += lowCutoff * (filtered - lowBoostState_);
            const float thump = FastMath::fastTanh(lowBoostState_ * (lowBoostGain_ - 1.0f));
            filtered += thump;
        }

        if (p.mojoMix > 0.01f) {
            const float highBand = processHp4(filtered);
            const float processedHigh = processMojo(highBand, p);
            filtered = filtered + (p.mojoMix * 0.01f) * (processedHigh - highBand);
        }

        const float hp = filtered - dcBlockerState_;
        dcBlockerState_ += 0.0006f * hp;
        filtered = hp;

        if (p.lifeMode == static_cast<int>(LifeMode::Cyborg)) {
            filtered += 0.00015f * ((slotA_.phase > 0.5f) ? 1.0f : -1.0f);
        }

        const float masterOut = FastMath::fastTanh(filtered * 0.95f) * p.volume;

        const float pan = std::clamp(p.pan + panOff_, -1.0f, 1.0f);
        const float gainL = (pan <= 0.0f) ? 1.0f : (1.0f - pan);
        const float gainR = (pan >= 0.0f) ? 1.0f : (1.0f + pan);

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

    std::array<float, 4> filterState_{};
    float vcfG_{0.1f};
    float vcfReso_{0.0f};
    float lpgVactrolState_{0.0f};
    float lpgFilterState_{0.0f};
    float lowBoostState_{0.0f};
    float dcBlockerState_{0.0f};
    float dampState_{0.0f};
    float lowBoostGain_{1.0f};

    float hp1_s1_{0.0f}, hp1_s2_{0.0f};
    float hp2_s1_{0.0f}, hp2_s2_{0.0f};
    float hpG_{0.01f}, hpH_{1.0f};
    float mojoDcBlocker_{0.0f};
    float crushPhase_{0.0f};
    float crushedSample_{0.0f};

    uint32_t rngState_{0xDEADBEEF};
    uint32_t trigRng_{0x853C49E6};
    float effCutoffHz_{2000.0f};
    float panOff_{0.0f};
    std::array<float, 8> latchedDraw_{{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};

    int blockCounter_{0};
    float macroSmoothed_{0.5f};
    float ampDecayCoeff_{0.999f};
    float pitchDecayCoeff_{0.995f};
    float timbreDecayCoeff_{0.990f};
    float clickDecayCoeff_{0.950f};
};

} // namespace tkdsp
