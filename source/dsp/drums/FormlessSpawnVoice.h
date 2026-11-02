#pragma once

#include "FastMath.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace tkdsp {

namespace FastMath = TbdAudio::FastMath;

/**
 * @brief FormlessSpawnVoice — Sovereign Tsathogguan Drum Voice Synthesizer.
 *
 * A flagship single-voice percussion workstation adhering to the Venti 32-Control
 * Specification and Triad Sonic Canon.
 *
 * Key Acoustic Architectures:
 *   - Latch-at-Note-On Model Selection (Tsathoggua ER-1, Bridged-T 909, Wavefold Core)
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
        HH = 2,
        PERC = 3,
        FX = 4
    };

    enum class Model : int {
        TsathogguaER1 = 0,
        BridgedT909   = 1,
        WavefoldCore  = 2
    };

    enum class LifeMode : int {
        Synthetic = 0,
        Cyborg    = 1,
        Organic   = 2
    };

    struct Params {
        // =====================================================================
        // Header Controls (4)
        // =====================================================================
        int role{0};             // 0=BD, 1=SD, 2=HH, 3=PERC, 4=FX
        int model{0};            // 0=ER-1, 1=909, 2=Fold
        int lifeMode{2};         // 0=Synthetic, 1=Cyborg, 2=Organic
        int keyTrack{0};         // 0=Fixed, 1=Key Tracking

        // =====================================================================
        // Bank 0 (Card 1): Drum Core & Pitch Lab (8)
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
        // Bank 1 (Card 2): Model Timbre & Acoustic Lab (8)
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
        // Bank 2 (Card 3 Right): VCF & Universal Mojo Strip (9)
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
        // Bank 3 (Card 3 Left & Center): Stepped Random & Master Altar (7)
        // =====================================================================
        float randPitch{0.0f};       // 0.0 .. 3.0 st
        float randDecay{0.0f};       // 0.0 .. 35.0 %
        float randTimbre{0.0f};      // 0.0 .. 25.0 %
        float randScatter{0.0f};     // 0.0 .. 20.0 %
        int   randMode{0};           // 0=Free, 1=Loop16
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

        // Reset Crossover HP4 Filters
        hp1_s1_ = 0.0f; hp1_s2_ = 0.0f;
        hp2_s1_ = 0.0f; hp2_s2_ = 0.0f;
        mojoDcBlocker_ = 0.0f;
        crushPhase_ = 0.0f;
        crushedSample_ = 0.0f;

        rngState_ = 0xDEADBEEF;
        loop16Step_ = 0;
        blockCounter_ = 0;
        macroSmoothed_ = 0.5f;

        activeModel_ = pendingModel_;
    }

    void setModel(int modelIdx) noexcept {
        pendingModel_ = std::clamp(modelIdx, 0, 2);
    }

    void trigger(float velocity, float noteHz = -1.0f, const Params& p = Params{}) noexcept {
        velocity = std::clamp(velocity, 0.0f, 1.0f);
        if (velocity <= 0.0001f) return;

        // Apply velocity sensitivity
        const float effVelocity = (1.0f - p.velSens) + p.velSens * velocity;

        // 1. Latch Pending Model on Note-On
        activeModel_ = pendingModel_;

        // 2. Advance PRNG and Latch Humanization Parameters
        float rPitchDraw = 0.0f;
        float rDecayDraw = 0.0f;
        float rTimbreDraw = 0.0f;
        float rScatterDraw = 0.0f;

        if (p.randMode == 1) {
            // LOOP16 Mode: Deterministic cyclic 16-step table
            const int step = (loop16Step_++) & 15;
            rPitchDraw   = kLoop16Table[step][0];
            rDecayDraw   = kLoop16Table[step][1];
            rTimbreDraw  = kLoop16Table[step][2];
            rScatterDraw = kLoop16Table[step][3];
        } else {
            // Free Mode: Continuous xorshift32 PRNG (bipolar -1.0 .. +1.0)
            rPitchDraw   = nextRandBipolar();
            rDecayDraw   = nextRandBipolar();
            rTimbreDraw  = nextRandBipolar();
            rScatterDraw = nextRandBipolar();
        }

        // 3. Ping-Pong Voice Allocation (1.5ms Linear Slew De-Click)
        activeSlot_ = 1 - activeSlot_;
        auto& newSlot = (activeSlot_ == 0) ? slotA_ : slotB_;

        fadeTotal_ = std::max(16, static_cast<int>(0.0015f * sampleRate_));
        fadeRemaining_ = fadeTotal_;

        // 4. Initialize New Slot with Latched Offsets
        newSlot.active = true;
        const float scatterMod = 1.0f + rScatterDraw * (p.randScatter * 0.01f);
        newSlot.ampEnv = std::clamp(effVelocity * scatterMod, 0.001f, 1.5f);
        newSlot.pitchEnv = 1.0f;
        newSlot.clickEnv = 1.0f;
        newSlot.timbreEnv = 1.0f;

        newSlot.latchedPitchOffsetSt = rPitchDraw * p.randPitch;
        newSlot.latchedDecayScale    = std::clamp(1.0f + rDecayDraw * (p.randDecay * 0.01f), 0.1f, 2.5f);
        newSlot.latchedTimbreOffset  = rTimbreDraw * (p.randTimbre * 0.01f);

        // Lifeform Phase Reset at t=0
        if (p.lifeMode == static_cast<int>(LifeMode::Cyborg)) {
            newSlot.phase = 0.25f; // 90-degree step for aggressive transient crackle
            newSlot.modPhase = 0.5f;
            newSlot.subPhase = 0.0f;
        } else if (p.lifeMode == static_cast<int>(LifeMode::Organic)) {
            newSlot.phase = 0.005f * ((velocity > 0.5f) ? 1.0f : -1.0f);
            newSlot.modPhase = 0.0f;
            newSlot.subPhase = 0.0f;
        } else {
            newSlot.phase = 0.0f; // Crystalline 0-degree clean zero crossing
            newSlot.modPhase = 0.0f;
            newSlot.subPhase = 0.0f;
        }

        // Base Frequency Calculation with Pitch Tracking & Latched Random Jitter
        float baseFreq = p.pitch;
        if (p.keyTrack != 0 && noteHz > 10.0f) {
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

private:
    struct VoiceSlot {
        float phase{0.0f};
        float modPhase{0.0f};
        float subPhase{0.0f};
        float clickPhase{0.0f};
        float ampEnv{0.0f};
        float pitchEnv{0.0f};
        float clickEnv{0.0f};
        float timbreEnv{0.0f};
        float baseHz{55.0f};
        float latchedPitchOffsetSt{0.0f};
        float latchedDecayScale{1.0f};
        float latchedTimbreOffset{0.0f};
        bool  active{false};
    };

    // =========================================================================
    // Multiplicative Log-Domain PCHIP Spline Evaluator
    // =========================================================================
    static inline float evalMacroMultiplier(float macro) noexcept {
        macro = std::clamp(macro, 0.0f, 1.0f);
        // Knot table for ln g:
        // M = [0.0,   0.25,  0.50,  0.75, 1.0]
        // g = [0.125, 0.40,  1.00,  2.50, 8.0]
        // ln_g = [-2.07944, -0.91629, 0.0, 0.91629, 2.07944]
        constexpr float y[5] = {-2.07944154f, -0.91629073f, 0.0f, 0.91629073f, 2.07944154f};
        // Harmonic Fritsch-Carlson derivatives
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
        // Smooth Macro Decay parameter
        macroSmoothed_ += 0.08f * (p.macroDecay - macroSmoothed_);
        const float macroMult = evalMacroMultiplier(macroSmoothed_);

        // 1. Effective Envelope Rates (Scaled by Macro Spline)
        const float effAmpDecaySec = std::clamp(0.250f * macroMult, 0.031f, 2.500f);
        const float effPitchDecaySec = std::clamp(p.pitchDecay * 0.001f * macroMult, 0.002f, 0.200f);
        const float effTimbreDecaySec = std::clamp(p.timbreT * 0.001f * macroMult, 0.002f, 0.800f);
        const float effClickDecaySec = std::clamp(0.002f * FastMath::fastPow2(macroSmoothed_ * 1.2f), 0.0008f, 0.006f);

        ampDecayCoeff_   = FastMath::fastExp(-invSampleRate_ / effAmpDecaySec);
        pitchDecayCoeff_ = FastMath::fastExp(-invSampleRate_ / effPitchDecaySec);
        timbreDecayCoeff_= FastMath::fastExp(-invSampleRate_ / effTimbreDecaySec);
        clickDecayCoeff_ = FastMath::fastExp(-invSampleRate_ / effClickDecaySec);

        // 2. VCF Cutoff Pre-Warping
        const float rawCutoff = std::clamp(p.cutoff, 20.0f, nyquist_);
        const float normCutoff = rawCutoff * invSampleRate_;
        const float wa = 2.0f * sampleRate_ * std::tan(normCutoff * 3.1415926535f);
        vcfG_ = wa * 0.5f * invSampleRate_;
        vcfReso_ = std::clamp(p.resonance * 3.8f, 0.0f, 3.85f);

        // 3. Low Boost Gain (0..12 dB converted to linear scale)
        lowBoostGain_ = FastMath::fastPow2(p.lowBoost * (1.0f / 6.0f));

        // 4. Crossover HP4 Coefficient (100 Hz fixed corner)
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
            // Logarithmic fast drop
            const float k = -p.pitchCurve * 3.0f;
            shapedPitchEnv = FastMath::fastTanh(shapedPitchEnv * (1.0f + k)) / (FastMath::fastTanh(1.0f + k) + 1e-6f);
        } else if (p.pitchCurve > 0.01f) {
            // Convex / S-curve hold
            const float k = p.pitchCurve;
            shapedPitchEnv = (1.0f - k) * shapedPitchEnv + k * (shapedPitchEnv * shapedPitchEnv);
        }

        const float pitchDepth = p.pitchDepth;
        const float pitchMult = FastMath::fastPow2((pitchDepth * shapedPitchEnv) * (1.0f / 12.0f));
        const float curFreq = std::clamp(slot.baseHz * pitchMult, 10.0f, nyquist_);

        // Advance Recursive Envelopes
        const float dScale = slot.latchedDecayScale;
        slot.pitchEnv  *= std::clamp(FastMath::fastPow2(FastMath::fastLog2(pitchDecayCoeff_) / dScale), 0.80f, 0.9999f);
        slot.ampEnv    *= std::clamp(FastMath::fastPow2(FastMath::fastLog2(ampDecayCoeff_) / dScale), 0.80f, 0.9999f);
        slot.timbreEnv *= std::clamp(FastMath::fastPow2(FastMath::fastLog2(timbreDecayCoeff_) / dScale), 0.80f, 0.9999f);
        slot.clickEnv  *= clickDecayCoeff_;

        // 2. Timbre Modulation Parameters (Modulated by Timbre Envelope)
        const float effTimbreOffset = slot.latchedTimbreOffset;
        const float tX = std::clamp(p.timbreX + effTimbreOffset, 0.0f, 1.0f);
        const float tY = std::clamp(p.timbreY * slot.timbreEnv + effTimbreOffset, 0.0f, 1.0f);
        const float tZ = std::clamp(p.timbreZ * slot.timbreEnv, 0.0f, 1.0f);

        // 3. Synthesis Model Core
        float oscOut = 0.0f;

        if (activeModel_ == static_cast<int>(Model::TsathogguaER1)) {
            // --- Model 00: Tsathoggua ER-1 Dual-Op FM + West Coast Wavefold ---
            const float modRatio = 0.5f + tX * 7.5f;
            const float modFreq = std::clamp(curFreq * modRatio, 1.0f, nyquist_);
            slot.modPhase += modFreq * invSampleRate_;
            if (slot.modPhase >= 1.0f) slot.modPhase -= 1.0f;

            // FM Waveform Morphing (Sine -> Tri -> Saw -> Noise)
            float modSig = 0.0f;
            if (p.fmMorph < 0.33f) {
                const float frac = p.fmMorph * 3.0f;
                const float s = FastMath::fastSinNorm(slot.modPhase);
                const float tri = (slot.modPhase < 0.5f) ? (4.0f * slot.modPhase - 1.0f) : (3.0f - 4.0f * slot.modPhase);
                modSig = (1.0f - frac) * s + frac * tri;
            } else if (p.fmMorph < 0.66f) {
                const float frac = (p.fmMorph - 0.33f) * 3.0f;
                const float tri = (slot.modPhase < 0.5f) ? (4.0f * slot.modPhase - 1.0f) : (3.0f - 4.0f * slot.modPhase);
                const float saw = 2.0f * slot.modPhase - 1.0f;
                modSig = (1.0f - frac) * tri + frac * saw;
            } else {
                const float frac = (p.fmMorph - 0.66f) * 3.0f;
                const float saw = 2.0f * slot.modPhase - 1.0f;
                const float noise = nextRandBipolar();
                modSig = (1.0f - frac) * saw + frac * noise;
            }

            const float fmIndex = tY * 3.5f;
            slot.phase += curFreq * invSampleRate_;
            if (slot.phase >= 1.0f) slot.phase -= 1.0f;

            const float modulatedPhase = slot.phase + modSig * fmIndex * 0.1591549f;
            float carrier = FastMath::fastSinNorm(modulatedPhase);

            // West Coast Diode Wavefolding
            if (tZ > 0.01f) {
                const float foldBias = 0.12f * tZ;
                const float f1 = FastMath::fastSinNorm((carrier + foldBias) * (1.0f + tZ * 2.0f));
                carrier = (1.0f - tZ) * carrier + tZ * f1;
            }
            oscOut = carrier;

        } else if (activeModel_ == static_cast<int>(Model::BridgedT909)) {
            // --- Model 01: Bridged-T Resonant Sine (909 / 808 Style) ---
            slot.phase += curFreq * invSampleRate_;
            if (slot.phase >= 1.0f) slot.phase -= 1.0f;

            float sine = FastMath::fastSinNorm(slot.phase);
            const float asym = tX * 0.4f;
            sine = FastMath::fastTanh(sine * (1.0f + tY * 2.0f) + asym) - asym * 0.5f;
            oscOut = sine;

        } else {
            // --- Model 02: Wavefold Overdrive Core (Pērkons Style) ---
            slot.phase += curFreq * invSampleRate_;
            if (slot.phase >= 1.0f) slot.phase -= 1.0f;

            float saw = 2.0f * slot.phase - 1.0f;
            const float foldAmt = 1.0f + tX * 4.0f;
            oscOut = FastMath::fastSinNorm(saw * foldAmt);
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
        // Stage 1 SVF High-pass
        const float hp1 = (input - 1.41421356f * hp1_s1_ - hp1_s2_) * hpH_;
        const float bp1 = hpG_ * hp1 + hp1_s1_;
        hp1_s1_ = hpG_ * hp1 + bp1;
        hp1_s2_ = hpG_ * bp1 + hp1_s2_;

        // Stage 2 SVF High-pass
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

        // 1. Profile Pre-Coloration
        if (p.mojoProfile == 1) {
            // MD-12: High-frequency pre-emphasis & gritty 12-bit DAC decimation
            y = y * 1.25f - 0.25f * highBand;
        } else if (p.mojoProfile == 2) {
            // SP-1200: 26.04kHz ring & punch
            y = FastMath::fastTanh(y * 1.3f);
        } else if (p.mojoProfile == 3) {
            // Iron Tape: Asymmetric iron core transformer saturation
            y = FastMath::fastTanh(y * 1.5f + 0.1f) - 0.1f;
        }

        // 2. Sample-Rate Reduction (Crush)
        float targetRate = std::clamp(p.crush, 1000.0f, 48000.0f);
        if (p.mojoProfile == 2) targetRate = 26040.0f; // Fixed SP-1200 rate

        if (targetRate < sampleRate_ * 0.95f) {
            crushPhase_ += targetRate * invSampleRate_;
            if (crushPhase_ >= 1.0f) {
                crushPhase_ -= 1.0f;
                crushedSample_ = y;
            }
            y = crushedSample_;
        }

        // 3. Bit Depth Quantization
        float effBits = p.bits;
        if (p.mojoProfile == 1 || p.mojoProfile == 2) effBits = std::min(effBits, 12.0f);

        if (effBits < 15.9f) {
            const float levels = FastMath::fastPow2(effBits - 1.0f);
            y = std::round(y * levels) / (levels + 1e-6f);
        }

        // 4. Tape Glue Saturation
        if (p.tapeGlue > 0.01f || p.mojoProfile == 3) {
            const float tapeDrive = 1.0f + p.tapeGlue * 3.0f;
            y = FastMath::fastTanh(y * tapeDrive) / (1.0f + p.tapeGlue * 0.5f);
        }

        // 5. Post-Mojo DC Blocker (5Hz)
        const float hp = y - mojoDcBlocker_;
        mojoDcBlocker_ += 0.0006f * hp;
        return hp;
    }

    void renderSampleStereo(float& outL, float& outR, const Params& p) noexcept {
        // 1. Render Ping-Pong Slots with 1.5ms Linear Crossfade
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

        // 2. Pre-Filter Drive & Saturation
        const float driveGain = 1.0f + p.preDrive * 4.0f;
        float shaped = rawAudio * driveGain;

        if (p.lifeMode == static_cast<int>(LifeMode::Cyborg)) {
            if (shaped > 0.85f) shaped = 0.85f - (shaped - 0.85f) * 0.7f;
            else if (shaped < -0.85f) shaped = -0.85f - (shaped + 0.85f) * 0.7f;
        } else if (p.lifeMode == static_cast<int>(LifeMode::Organic)) {
            shaped = FastMath::fastTanh(shaped * 0.9f);
        }

        // 3. Shared Filter / LPG Section
        float filtered = 0.0f;

        if (p.vcfMode == 1 || p.lifeMode == static_cast<int>(LifeMode::Organic)) {
            // Buchla 292 / VTL5C3 Resonant Low Pass Gate
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
            // 24dB Zero-Delay Feedback Diode Ladder Filter
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

        // 4. Resonant 80Hz Low-Boost Thump
        if (p.lowBoost > 0.01f) {
            const float lowCutoff = 80.0f * invSampleRate_;
            lowBoostState_ += lowCutoff * (filtered - lowBoostState_);
            const float thump = FastMath::fastTanh(lowBoostState_ * (lowBoostGain_ - 1.0f));
            filtered += thump;
        }

        // 5. Zero-Latency Delta Crossover & Mojo Strip
        if (p.mojoMix > 0.01f) {
            const float highBand = processHp4(filtered);
            const float processedHigh = processMojo(highBand, p);
            filtered = filtered + (p.mojoMix * 0.01f) * (processedHigh - highBand);
        }

        // 6. Master DC Decoupling & Output Conditioning
        const float hp = filtered - dcBlockerState_;
        dcBlockerState_ += 0.0006f * hp;
        filtered = hp;

        if (p.lifeMode == static_cast<int>(LifeMode::Cyborg)) {
            filtered += 0.00015f * ((slotA_.phase > 0.5f) ? 1.0f : -1.0f);
        }

        const float masterOut = FastMath::fastTanh(filtered * 0.95f) * p.volume;

        // 7. Pan & Stereo Spread
        const float pan = std::clamp(p.pan, -1.0f, 1.0f);
        const float gainL = (pan <= 0.0f) ? 1.0f : (1.0f - pan);
        const float gainR = (pan >= 0.0f) ? 1.0f : (1.0f + pan);

        outL = masterOut * gainL;
        outR = masterOut * gainR;
    }

    // =========================================================================
    // xorshift32 PRNG (Top 24-Bit Extraction)
    // =========================================================================
    inline float nextRandBipolar() noexcept {
        rngState_ ^= rngState_ << 13;
        rngState_ ^= rngState_ >> 17;
        rngState_ ^= rngState_ << 5;
        if (rngState_ == 0) rngState_ = 0x1337C0DE;
        const float u = static_cast<float>((rngState_ >> 8) & 0x00FFFFFF) * (1.0f / 16777216.0f);
        return 2.0f * u - 1.0f; // [-1.0, +1.0]
    }

    // Deterministic 16-Step Cyclic PRNG Pattern Table
    static constexpr std::array<std::array<float, 4>, 16> kLoop16Table = {{
        {{ 0.12f, -0.25f,  0.40f, -0.15f }},
        {{-0.34f,  0.15f, -0.20f,  0.30f }},
        {{ 0.55f, -0.10f,  0.10f, -0.05f }},
        {{-0.18f,  0.35f, -0.45f,  0.22f }},
        {{ 0.22f, -0.30f,  0.15f, -0.18f }},
        {{-0.45f,  0.08f, -0.12f,  0.40f }},
        {{ 0.38f, -0.15f,  0.28f, -0.25f }},
        {{-0.10f,  0.25f, -0.32f,  0.12f }},
        {{ 0.60f, -0.05f,  0.18f, -0.10f }},
        {{-0.28f,  0.18f, -0.15f,  0.28f }},
        {{ 0.15f, -0.22f,  0.35f, -0.30f }},
        {{-0.50f,  0.30f, -0.28f,  0.15f }},
        {{ 0.30f, -0.12f,  0.12f, -0.14f }},
        {{-0.20f,  0.10f, -0.40f,  0.35f }},
        {{ 0.42f, -0.28f,  0.22f, -0.20f }},
        {{-0.08f,  0.14f, -0.18f,  0.10f }}
    }};

    // Engine & Sample Rate State
    float sampleRate_{48000.0f};
    float invSampleRate_{1.0f / 48000.0f};
    float nyquist_{23760.0f};

    VoiceSlot slotA_{};
    VoiceSlot slotB_{};
    int activeSlot_{0};
    int fadeRemaining_{0};
    int fadeTotal_{64};

    int activeModel_{0};
    int pendingModel_{0};

    // Filter & Saturation State
    std::array<float, 4> filterState_{};
    float vcfG_{0.1f};
    float vcfReso_{0.0f};
    float lpgVactrolState_{0.0f};
    float lpgFilterState_{0.0f};
    float lowBoostState_{0.0f};
    float dcBlockerState_{0.0f};
    float dampState_{0.0f};
    float lowBoostGain_{1.0f};

    // Crossover & Mojo State
    float hp1_s1_{0.0f}, hp1_s2_{0.0f};
    float hp2_s1_{0.0f}, hp2_s2_{0.0f};
    float hpG_{0.01f}, hpH_{1.0f};
    float mojoDcBlocker_{0.0f};
    float crushPhase_{0.0f};
    float crushedSample_{0.0f};

    // PRNG State
    uint32_t rngState_{0xDEADBEEF};
    int loop16Step_{0};

    // Control-Rate Coeffs
    int blockCounter_{0};
    float macroSmoothed_{0.5f};
    float ampDecayCoeff_{0.999f};
    float pitchDecayCoeff_{0.995f};
    float timbreDecayCoeff_{0.990f};
    float clickDecayCoeff_{0.950f};
};

} // namespace tkdsp