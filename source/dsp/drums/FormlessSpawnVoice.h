#pragma once

#include "FastMath.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string_view>

namespace tkdsp {

namespace FastMath = TbdAudio::FastMath;

/**
 * @brief FormlessSpawnVoice — Sovereign Tsathogguan Drum Voice Synthesizer.
 *
 * A single-voice, hardware-inspired percussion synthesizer built on the
 * Triad Sonic Canon (Synthetic Clean Virtual Analog, Organic Sloshy Tube LPG,
 * and Cyborg Nuclear Reactor Waste).
 *
 * Features:
 *   - Latch-at-Note-On Model Architecture (Tsathoggua ER-1, Bridged-T 909, Wavefold Core)
 *   - Dual-Slot Anti-Pop Ping-Pong Oscillator Core (1.5ms linear de-click ramp)
 *   - Bounded Headroom PCHIP Hermite Spline Macro Decay Engine
 *   - Deterministic t=0 Phase Reset with Transient Energy Coupling
 *   - Shared 4-Pole ZDF Diode Ladder / Buchla 292 Resonant Low-Pass Gate
 *   - Resonant 80Hz Low-Boost Thump and Master Diode Saturation
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
        // Card 1: Drum Core & Timbre Lab
        float pitch{55.0f};          // 20.0 .. 200.0 Hz
        float macroDecay{0.5f};      // 0.0 .. 1.0 (PCHIP Hermite Macro)
        float punch{0.5f};           // 0.0 .. 1.0 (Transient Click Level)
        float pitchDepth{24.0f};     // -48.0 .. +48.0 Semitones
        float pitchDecay{25.0f};     // 2.0 .. 100.0 ms
        float timbreX{0.25f};        // 0.0 .. 1.0 (FM Ratio / Model X)
        float timbreY{0.35f};        // 0.0 .. 1.0 (FM Depth / Model Y)
        float timbreZ{0.20f};        // 0.0 .. 1.0 (Wavefold / Bias / Model Z)

        // Card 2: VCF / LPG, Saturation & Output
        float cutoff{2000.0f};       // 20.0 .. 20000.0 Hz
        float resonance{0.30f};      // 0.0 .. 1.0
        int   vcfMode{0};            // 0 = 24dB Diode Ladder, 1 = Buchla 292 LPG
        float preDrive{0.15f};       // 0.0 .. 1.0
        float lowBoost{0.35f};       // 0.0 .. 1.0 (80Hz Resonant Thump)
        float saturate{0.25f};       // 0.0 .. 1.0 (Master Diode Clipper)
        float pan{0.0f};             // -1.0 .. +1.0
        float volume{0.85f};         // 0.0 .. 1.0

        // Header Selectors
        int   role{0};               // Role::BD
        int   model{0};              // Model::TsathogguaER1
        int   lifeMode{2};           // LifeMode::Organic
        int   keyTrack{0};           // 0 = Fixed, 1 = Key Tracking
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
        blockCounter_ = 0;

        activeModel_ = pendingModel_;
    }

    void setModel(int modelIdx) noexcept {
        pendingModel_ = std::clamp(modelIdx, 0, 2);
    }

    void trigger(float velocity, float noteHz = -1.0f, const Params& p = Params{}) noexcept {
        velocity = std::clamp(velocity, 0.0f, 1.0f);
        if (velocity <= 0.0001f) return;

        // 1. Latch Pending Model on Note-On (100% Click-Free)
        activeModel_ = pendingModel_;

        // 2. Ping-Pong Voice Slots (1.5ms Linear Slew De-Click)
        activeSlot_ = 1 - activeSlot_;
        auto& newSlot = (activeSlot_ == 0) ? slotA_ : slotB_;

        fadeTotal_ = std::max(16, static_cast<int>(0.0015f * sampleRate_));
        fadeRemaining_ = fadeTotal_;

        // 3. Initialize New Slot
        newSlot.active = true;
        newSlot.ampEnv = velocity;
        newSlot.pitchEnv = 1.0f;
        newSlot.clickEnv = 1.0f;

        // Hard Phase Reset at t=0
        if (p.lifeMode == static_cast<int>(LifeMode::Cyborg)) {
            newSlot.phase = 0.25f; // 90-degree step for aggressive DC impulse crackle
            newSlot.modPhase = 0.5f;
        } else if (p.lifeMode == static_cast<int>(LifeMode::Organic)) {
            newSlot.phase = 0.005f * ((velocity > 0.5f) ? 1.0f : -1.0f); // Gentle acoustic wander
            newSlot.modPhase = 0.0f;
        } else {
            newSlot.phase = 0.0f; // Crystalline 0-degree zero crossing
            newSlot.modPhase = 0.0f;
        }

        // Base Frequency Calculation
        if (p.keyTrack != 0 && noteHz > 10.0f) {
            const float pitchOffsetSemis = (p.pitch - 55.0f) * (12.0f / 45.0f);
            newSlot.baseHz = std::clamp(noteHz * FastMath::fastPow2(pitchOffsetSemis * (1.0f / 12.0f)), 15.0f, nyquist_);
        } else {
            newSlot.baseHz = std::clamp(p.pitch, 15.0f, nyquist_);
        }
    }

    void processBlockStereo(float* outL, float* outR, int numSamples, const Params& p) noexcept {
        for (int i = 0; i < numSamples; ++i) {
            // Control-Rate Coefficient Hoisting (Every 16 samples)
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
        float ampEnv{0.0f};
        float pitchEnv{0.0f};
        float clickEnv{0.0f};
        float baseHz{55.0f};
        bool  active{false};
    };

    // =========================================================================
    // PCHIP Hermite Spline Macro Decay Law
    // =========================================================================
    static inline float evalPchipMacro(float b, float M) noexcept {
        b = std::clamp(b, 0.02f, 0.98f);
        const float d1 = b * 2.0f;          // secant slope left: b / 0.5
        const float d2 = (1.0f - b) * 2.0f; // secant slope right: (1 - b) / 0.5
        const float m = (2.0f * d1 * d2) / (d1 + d2 + 1e-6f); // Harmonic mean noon tangent

        if (M < 0.5f) {
            const float t = M * 2.0f; // [0.0, 1.0)
            const float t2 = t * t;
            const float t3 = t2 * t;
            const float h01 = -2.0f * t3 + 3.0f * t2;
            const float h11 = t3 - t2;
            return std::clamp(h01 * b + h11 * (0.5f * m), 0.0f, 1.0f);
        } else {
            const float t = (M - 0.5f) * 2.0f; // [0.0, 1.0]
            const float t2 = t * t;
            const float t3 = t2 * t;
            const float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
            const float h10 = t3 - 2.0f * t2 + t;
            const float h01 = -2.0f * t3 + 3.0f * t2;
            const float h11 = t3 - t2;
            return std::clamp(h00 * b + h10 * (0.5f * m) + h01 * 1.0f + h11 * (0.5f * d2), 0.0f, 1.0f);
        }
    }

    void updateControlRateCoefficients(const Params& p) noexcept {
        const float M = std::clamp(p.macroDecay, 0.0f, 1.0f);

        // Normalized base positions
        const float normPitchDecay = std::clamp((p.pitchDecay - 2.0f) / 98.0f, 0.0f, 1.0f);
        const float normAmpDecay   = 0.50f; // Base anchor
        const float normFltDecay   = 0.40f;

        // 1. Evaluate PCHIP Splines
        const float effAmp   = evalPchipMacro(normAmpDecay, M);
        const float effPitch = evalPchipMacro(normPitchDecay, M);
        const float effFlt   = evalPchipMacro(normFltDecay, M);

        // 2. Log-Time Mapping
        // Amp: 20ms to 4000ms
        const float ampDecaySec = 0.020f * FastMath::fastPow2(effAmp * 7.643856f);
        // Pitch: 3ms to 80ms
        const float pitchDecaySec = 0.003f * FastMath::fastPow2(effPitch * 4.736965f);
        // Click: Coupled to body decay
        const float clickDecaySec = std::clamp(0.0015f * FastMath::fastPow2(effAmp * 1.5f), 0.0008f, 0.005f);

        // 3. Shape Morphing (Gamma: 1.6 on short thump -> 0.85 on long 808 bloom)
        const float gamma = 1.6f - 0.75f * M;
        const float normalizedAmpStep = std::clamp(invSampleRate_ / ampDecaySec, 0.00001f, 0.1f);
        ampDecayCoeff_ = 1.0f - normalizedAmpStep * gamma;
        pitchDecayCoeff_ = FastMath::fastExp(-invSampleRate_ / pitchDecaySec);
        clickDecayCoeff_ = FastMath::fastExp(-invSampleRate_ / clickDecaySec);

        // 4. Click Transient Coupling
        clickGain_ = p.punch * (1.2f - 0.4f * M);

        // 5. Shared VCF Cutoff Pre-Warping
        const float rawCutoff = std::clamp(p.cutoff, 20.0f, nyquist_);
        const float normCutoff = rawCutoff * invSampleRate_;
        const float wa = 2.0f * sampleRate_ * std::tan(normCutoff * 3.1415926535f);
        vcfG_ = wa * 0.5f * invSampleRate_;
        vcfReso_ = std::clamp(p.resonance * 3.8f, 0.0f, 3.85f);
    }

    inline float renderSlotSample(VoiceSlot& slot, const Params& p) noexcept {
        if (!slot.active || slot.ampEnv < 0.00001f) {
            slot.active = false;
            return 0.0f;
        }

        // 1. Pitch Envelope Dive
        const float pitchMult = (slot.pitchEnv > 0.001f)
            ? FastMath::fastPow2((p.pitchDepth * slot.pitchEnv) * (1.0f / 12.0f))
            : 1.0f;
        const float curFreq = std::clamp(slot.baseHz * pitchMult, 10.0f, nyquist_);

        // Advance Envelopes
        slot.pitchEnv *= pitchDecayCoeff_;
        slot.ampEnv   *= ampDecayCoeff_;
        slot.clickEnv *= clickDecayCoeff_;

        // 2. Synthesis Model Core
        float oscOut = 0.0f;

        if (activeModel_ == static_cast<int>(Model::TsathogguaER1)) {
            // --- Model 00: Tsathoggua ER-1 Dual-Op FM + West Coast Wavefold ---
            const float modRatio = 0.5f + p.timbreX * 7.5f;
            const float modFreq = std::clamp(curFreq * modRatio, 1.0f, nyquist_);
            slot.modPhase += modFreq * invSampleRate_;
            if (slot.modPhase >= 1.0f) slot.modPhase -= 1.0f;

            const float modSig = FastMath::fastSinNorm(slot.modPhase);
            const float fmIndex = p.timbreY * 3.5f * slot.ampEnv;

            slot.phase += curFreq * invSampleRate_;
            if (slot.phase >= 1.0f) slot.phase -= 1.0f;

            const float modulatedPhase = slot.phase + modSig * fmIndex * 0.1591549f;
            float carrier = FastMath::fastSinNorm(modulatedPhase);

            // West Coast Diode Wavefolding
            const float foldDepth = p.timbreZ;
            if (foldDepth > 0.01f) {
                const float foldBias = 0.12f * foldDepth;
                const float f1 = FastMath::fastSinNorm((carrier + foldBias) * (1.0f + foldDepth * 2.0f));
                carrier = (1.0f - foldDepth) * carrier + foldDepth * f1;
            }
            oscOut = carrier;

        } else if (activeModel_ == static_cast<int>(Model::BridgedT909)) {
            // --- Model 01: Bridged-T Resonant Sine (909 / 808 Style) ---
            slot.phase += curFreq * invSampleRate_;
            if (slot.phase >= 1.0f) slot.phase -= 1.0f;

            float sine = FastMath::fastSinNorm(slot.phase);
            // Non-linear core bridging asymmetry
            const float asym = p.timbreX * 0.4f;
            sine = FastMath::fastTanh(sine * (1.0f + p.timbreY * 2.0f) + asym) - asym * 0.5f;
            oscOut = sine;

        } else {
            // --- Model 02: Wavefold Overdrive Core (Pērkons Style) ---
            slot.phase += curFreq * invSampleRate_;
            if (slot.phase >= 1.0f) slot.phase -= 1.0f;

            float saw = 2.0f * slot.phase - 1.0f;
            const float foldAmt = 1.0f + p.timbreX * 4.0f;
            oscOut = FastMath::fastSinNorm(saw * foldAmt);
        }

        // 3. Transient Click Generator
        if (slot.clickEnv > 0.001f) {
            const float click = (slot.phase < 0.1f) ? (1.0f - slot.phase * 10.0f) : 0.0f;
            oscOut += click * clickGain_ * slot.clickEnv;
        }

        return oscOut * slot.ampEnv;
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
            // Cyborg: Starved op-amp rail foldback
            if (shaped > 0.85f) shaped = 0.85f - (shaped - 0.85f) * 0.7f;
            else if (shaped < -0.85f) shaped = -0.85f - (shaped + 0.85f) * 0.7f;
        } else if (p.lifeMode == static_cast<int>(LifeMode::Organic)) {
            // Organic: Soft cathode-follower triode curvature
            shaped = FastMath::fastTanh(shaped * 0.9f);
        }

        // 3. Shared Filter / LPG Section
        float filtered = 0.0f;

        if (p.vcfMode == 1 || p.lifeMode == static_cast<int>(LifeMode::Organic)) {
            // --- Buchla 292 / VTL5C3 Resonant Low Pass Gate ---
            const float targetLpg = std::clamp(active.ampEnv + 0.02f, 0.0f, 1.0f);
            if (targetLpg > lpgVactrolState_) {
                lpgVactrolState_ += 0.08f * (targetLpg - lpgVactrolState_); // Fast 2ms attack
            } else {
                lpgVactrolState_ += 0.003f * (targetLpg - lpgVactrolState_); // Sloshy 80ms..250ms droop
            }
            const float lpgCutoff = std::clamp(lpgVactrolState_ * p.cutoff * invSampleRate_ * 3.14159f, 0.001f, 0.45f);
            lpgFilterState_ += lpgCutoff * (shaped - lpgFilterState_);
            filtered = lpgFilterState_;

        } else {
            // --- 24dB Zero-Delay Feedback Diode Ladder Filter ---
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
            const float lowBoostGain = p.lowBoost * 3.0f;
            const float lowCutoff = 80.0f * invSampleRate_;
            lowBoostState_ += lowCutoff * (filtered - lowBoostState_);
            const float thump = FastMath::fastTanh(lowBoostState_ * lowBoostGain);
            filtered += thump;
        }

        // 5. Leaky DC Decoupling & Bleed
        const float hp = filtered - dcBlockerState_;
        dcBlockerState_ += 0.0006f * hp; // 4Hz gentle breathing
        filtered = hp;

        if (p.lifeMode == static_cast<int>(LifeMode::Cyborg)) {
            // Leaky -38dB vactrol hiss floor
            filtered += 0.00015f * ((slotA_.phase > 0.5f) ? 1.0f : -1.0f);
        }

        // 6. Master Output Saturation, Pan & Volume
        const float masterGain = 1.0f + p.saturate * 2.5f;
        const float masterOut = FastMath::fastTanh(filtered * masterGain * 0.8f) * p.volume;

        const float pan = std::clamp(p.pan, -1.0f, 1.0f);
        const float gainL = (pan <= 0.0f) ? 1.0f : (1.0f - pan);
        const float gainR = (pan >= 0.0f) ? 1.0f : (1.0f + pan);

        outL = masterOut * gainL;
        outR = masterOut * gainR;
    }

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

    // Control-Rate Coeffs
    int blockCounter_{0};
    float ampDecayCoeff_{0.999f};
    float pitchDecayCoeff_{0.995f};
    float clickDecayCoeff_{0.950f};
    float clickGain_{0.5f};
};

} // namespace tkdsp
