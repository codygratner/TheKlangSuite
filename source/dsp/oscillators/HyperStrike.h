#pragma once

#include "SoundEngine.h"
#include "../../FastMath.h"
#include "../common/AudioEffect.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string_view>

// ==============================================================================
// HyperStrike.h - 4D Hypercube Percussion Synthesizer Engine (MEGA-PERC-05D)
// Part of TheKlangDSP Ecosystem & ToadTracker. Copyright (c) 2026 Klang Industries.
//
// 100% Platform-Agnostic C++20 (Windows, macOS, Linux, Steam Deck, WebAssembly).
// Strict Audio-Thread Safety: Zero heap allocations, zero locks, zero CRT trig.
// ==============================================================================

namespace toad {

namespace FastMath = TbdAudio::FastMath;
namespace DSPCommon = TbdAudio::DSPCommon;

constexpr size_t K_LATTICE_DIM = 4;
constexpr size_t K_LATTICE_NODES = K_LATTICE_DIM * K_LATTICE_DIM * K_LATTICE_DIM; // 64
constexpr size_t K_DEFAULT_NODE_SAMPLES = 2048;

struct HypercubeCoordinates {
    float w{0.0f}; // Intra-drum temporal traverse / scrub [0..1]
    float x{0.0f}; // Role Spectrum: Kick (0) -> Tom (0.33) -> Snare (0.67) -> Hat (1.0)
    float y{0.0f}; // Decay Dynamics: Tight Snap (0) -> Punch (0.33) -> Saturated (0.67) -> Wash (1.0)
    float z{0.0f}; // Texture: Pure Tone (0) -> Organic Shell (0.33) -> Saturated Drive (0.67) -> Noise (1.0)
};

struct CartridgeManifest {
    uint64_t seed{0};
    std::array<char[32], K_LATTICE_NODES> voiceIds{};
};

/**
 * @brief Core 4D Hypercube Percussion Synthesis Engine.
 *        Evaluates continuous 4D coordinates (w, x, y, z) using trilinear spatial
 *        interpolation and temporal phase scrubbing across 64 lattice nodes.
 */
class HyperStrikeEngine {
public:
    HyperStrikeEngine() noexcept {
        initDefaultLattice();
    }

    ~HyperStrikeEngine() noexcept = default;

    void prepare(float sampleRate) noexcept {
        sampleRate_ = (sampleRate > 8000.0f) ? sampleRate : 48000.0f;
        invSr_ = 1.0f / sampleRate_;
        dcBlockL_.init(sampleRate_);
        dcBlockR_.init(sampleRate_);
        filterStage1_.reset();
        filterStage2_.reset();
        reset();
    }

    void reset() noexcept {
        phase_ = 0.0f;
        phaseInc_ = 0.0f;
        ampEnv_ = 0.0f;
        pitchEnv_ = 1.0f;
        pitchDecayRate_ = 0.992f;
        ampDecayRate_ = 0.996f;
        srrCounter_ = 0.0f;
        heldSample_ = 0.0f;
        active_ = false;
        filterStage1_.reset();
        filterStage2_.reset();
        dcBlockL_.reset();
        dcBlockR_.reset();
    }

    void setCoordinates(const HypercubeCoordinates& coords) noexcept {
        coords_.w = std::clamp(coords.w, 0.0f, 1.0f);
        coords_.x = std::clamp(coords.x, 0.0f, 1.0f);
        coords_.y = std::clamp(coords.y, 0.0f, 1.0f);
        coords_.z = std::clamp(coords.z, 0.0f, 1.0f);
        updateSpatialWeights();
    }

    void setPitchHz(float hz) noexcept {
        basePitchHz_ = std::clamp(hz, 8.0f, 60000.0f);
    }

    void setCutoffHz(float cutoffHz) noexcept {
        cutoffHz_ = std::clamp(cutoffHz, 20.0f, 20000.0f);
    }

    void setDrive(float drive) noexcept {
        drive_ = std::clamp(drive, 0.0f, 1.0f);
    }

    void setSRR(float srrNorm) noexcept {
        // srrNorm: 0.0 = 44.1kHz (clean), 1.0 = heavy decimation (1kHz)
        srrNorm_ = std::clamp(srrNorm, 0.0f, 1.0f);
    }

    void trigger(float velocity = 1.0f) noexcept {
        velocity = std::clamp(velocity, 0.0f, 1.0f);
        if (velocity <= 0.001f) return;

        // Base frequency derived from pitch clock & role coordinate X
        // Kick (X=0) ~ 55Hz, Tom (X=0.33) ~ 110Hz, Snare (X=0.67) ~ 220Hz, Hat (X=1.0) ~ 8000Hz
        const float roleFreq = 50.0f * FastMath::fastPow2(coords_.x * 7.2f);
        const float triggerFreq = (basePitchHz_ > 20.0f) ? (basePitchHz_ * (0.5f + 0.5f * (roleFreq / 220.0f))) : roleFreq;

        phase_ = coords_.w * static_cast<float>(K_DEFAULT_NODE_SAMPLES - 1);
        phaseInc_ = (triggerFreq * static_cast<float>(K_DEFAULT_NODE_SAMPLES)) * invSr_;

        // Decay time derived from Y coordinate: Snap (Y=0) ~ 30ms -> Long Wash (Y=1) ~ 1.5s
        const float decaySec = 0.030f * FastMath::fastPow2(coords_.y * 5.6f);
        const float decaySamples = std::max(64.0f, decaySec * sampleRate_);
        ampDecayRate_ = 1.0f - (3.5f / decaySamples);

        // Pitch envelope for low-end thump (active primarily on Kick/Tom, X < 0.6)
        pitchPunchAmount_ = std::max(0.0f, (0.6f - coords_.x) * 4.0f);
        pitchEnv_ = 1.0f;
        pitchDecayRate_ = 1.0f - (30.0f / (sampleRate_ * 0.06f));

        ampEnv_ = velocity;
        active_ = true;
    }

    [[nodiscard]] inline float processSample() noexcept {
        if (!active_ || ampEnv_ < 1e-5f) {
            active_ = false;
            return 0.0f;
        }

        // 1. Evaluate spatial trilinear blend at current phase
        const float rawSample = evaluateLatticeSample(phase_);

        // 2. Advance phase with pitch envelope
        const float currentInc = phaseInc_ * (1.0f + pitchPunchAmount_ * pitchEnv_);
        phase_ += currentInc;
        if (phase_ >= static_cast<float>(K_DEFAULT_NODE_SAMPLES)) {
            // Loop or end depending on decay
            phase_ = std::fmod(phase_, static_cast<float>(K_DEFAULT_NODE_SAMPLES));
        }

        pitchEnv_ *= pitchDecayRate_;
        ampEnv_ *= ampDecayRate_;

        // 3. Sample-Rate Reduction (SRR / Glitch)
        float srrSample = rawSample;
        if (srrNorm_ > 0.01f) {
            const float decimationFactor = 1.0f + srrNorm_ * 40.0f;
            srrCounter_ += 1.0f;
            if (srrCounter_ >= decimationFactor) {
                srrCounter_ -= decimationFactor;
                heldSample_ = rawSample;
            }
            srrSample = heldSample_;
        }

        // 4. Am6070 Saturation Drive (Soft / Hard piecewise knee)
        float driven = srrSample;
        if (drive_ > 0.01f) {
            const float gain = 1.0f + drive_ * 7.0f;
            driven = FastMath::fastTanh(srrSample * gain) * (1.0f / (1.0f + drive_ * 2.0f));
        }

        // 5. Curtis CEM3320 24dB ZDF Filter (Cascaded 2-pole CytomicSVF stages)
        filterStage1_.setLowpass(cutoffHz_, 1.2f, invSr_);
        filterStage2_.setLowpass(cutoffHz_, 0.8f, invSr_);
        const float filtered = filterStage2_.process(filterStage1_.process(driven));

        return filtered * ampEnv_;
    }

    inline void processStereo(float& outL, float& outR) noexcept {
        const float mono = processSample();
        // Subtle pseudo-stereo spread on high frequencies (Hat/Cymbal)
        const float spread = coords_.x * 0.25f;
        outL = dcBlockL_.process(mono * (1.0f - spread));
        outR = dcBlockR_.process(mono * (1.0f + spread));
    }

private:
    void updateSpatialWeights() noexcept {
        // Map [0..1] continuous coordinates to [0..K_LATTICE_DIM-1] grid
        const float gx = coords_.x * (K_LATTICE_DIM - 1);
        const float gy = coords_.y * (K_LATTICE_DIM - 1);
        const float gz = coords_.z * (K_LATTICE_DIM - 1);

        const int x0 = std::clamp(static_cast<int>(gx), 0, static_cast<int>(K_LATTICE_DIM - 2));
        const int y0 = std::clamp(static_cast<int>(gy), 0, static_cast<int>(K_LATTICE_DIM - 2));
        const int z0 = std::clamp(static_cast<int>(gz), 0, static_cast<int>(K_LATTICE_DIM - 2));

        const float u = std::clamp(gx - static_cast<float>(x0), 0.0f, 1.0f);
        const float v = std::clamp(gy - static_cast<float>(y0), 0.0f, 1.0f);
        const float w = std::clamp(gz - static_cast<float>(z0), 0.0f, 1.0f);

        activeCornerIndices_[0] = getNodeIndex(x0,     y0,     z0);
        activeCornerIndices_[1] = getNodeIndex(x0 + 1, y0,     z0);
        activeCornerIndices_[2] = getNodeIndex(x0,     y0 + 1, z0);
        activeCornerIndices_[3] = getNodeIndex(x0 + 1, y0 + 1, z0);
        activeCornerIndices_[4] = getNodeIndex(x0,     y0,     z0 + 1);
        activeCornerIndices_[5] = getNodeIndex(x0 + 1, y0,     z0 + 1);
        activeCornerIndices_[6] = getNodeIndex(x0,     y0 + 1, z0 + 1);
        activeCornerIndices_[7] = getNodeIndex(x0 + 1, y0 + 1, z0 + 1);

        activeCornerWeights_[0] = (1.0f - u) * (1.0f - v) * (1.0f - w);
        activeCornerWeights_[1] = u * (1.0f - v) * (1.0f - w);
        activeCornerWeights_[2] = (1.0f - u) * v * (1.0f - w);
        activeCornerWeights_[3] = u * v * (1.0f - w);
        activeCornerWeights_[4] = (1.0f - u) * (1.0f - v) * w;
        activeCornerWeights_[5] = u * (1.0f - v) * w;
        activeCornerWeights_[6] = (1.0f - u) * v * w;
        activeCornerWeights_[7] = u * v * w;
    }

    [[nodiscard]] static constexpr size_t getNodeIndex(size_t x, size_t y, size_t z) noexcept {
        return z * (K_LATTICE_DIM * K_LATTICE_DIM) + y * K_LATTICE_DIM + x;
    }

    [[nodiscard]] inline float evaluateLatticeSample(float phase) const noexcept {
        const size_t i0 = static_cast<size_t>(phase) % K_DEFAULT_NODE_SAMPLES;
        const size_t i1 = (i0 + 1) % K_DEFAULT_NODE_SAMPLES;
        const float frac = phase - std::floor(phase);

        float blended = 0.0f;

#if defined(TKF_HAS_SSE)
        // SIMD 4-lane dot product across corner weights
        __m128 sum4 = _mm_setzero_ps();
        for (int c = 0; c < 8; c += 4) {
            const float s0 = readLatticeSample(activeCornerIndices_[c],     i0, i1, frac);
            const float s1 = readLatticeSample(activeCornerIndices_[c + 1], i0, i1, frac);
            const float s2 = readLatticeSample(activeCornerIndices_[c + 2], i0, i1, frac);
            const float s3 = readLatticeSample(activeCornerIndices_[c + 3], i0, i1, frac);

            __m128 samples = _mm_set_ps(s3, s2, s1, s0);
            __m128 weights = _mm_loadu_ps(&activeCornerWeights_[c]);
            sum4 = _mm_add_ps(sum4, _mm_mul_ps(samples, weights));
        }
        // Horizontal add sum4
        alignas(16) float res[4];
        _mm_store_ps(res, sum4);
        blended = res[0] + res[1] + res[2] + res[3];
#else
        for (int c = 0; c < 8; ++c) {
            const float s = readLatticeSample(activeCornerIndices_[c], i0, i1, frac);
            blended += s * activeCornerWeights_[c];
        }
#endif
        return blended;
    }

    [[nodiscard]] inline float readLatticeSample(size_t nodeIdx, size_t i0, size_t i1, float frac) const noexcept {
        const size_t baseOffset = nodeIdx * K_DEFAULT_NODE_SAMPLES;
        const float y0 = defaultLattice_[baseOffset + i0];
        const float y1 = defaultLattice_[baseOffset + i1];
        return y0 + frac * (y1 - y0);
    }

    void initDefaultLattice() noexcept {
        // Procedurally generates authentic timbre foundations across the 64 nodes
        for (size_t z = 0; z < K_LATTICE_DIM; ++z) {
            for (size_t y = 0; y < K_LATTICE_DIM; ++y) {
                for (size_t x = 0; x < K_LATTICE_DIM; ++x) {
                    const size_t nodeIdx = getNodeIndex(x, y, z);
                    const size_t offset = nodeIdx * K_DEFAULT_NODE_SAMPLES;

                    const float normX = static_cast<float>(x) / 3.0f; // Role
                    const float normZ = static_cast<float>(z) / 3.0f; // Texture

                    uint32_t seed = static_cast<uint32_t>(nodeIdx * 0x1F2E3D4Cu + 0xDEADBEEFu);

                    for (size_t i = 0; i < K_DEFAULT_NODE_SAMPLES; ++i) {
                        const float t = static_cast<float>(i) / static_cast<float>(K_DEFAULT_NODE_SAMPLES);

                        // Tone component: fundamental sine + harmonics
                        const float fundCycles = 1.0f + normX * 12.0f;
                        const float tone = FastMath::fastSinNorm(t * fundCycles);

                        // Noise component: pseudo-random hash
                        seed = seed * 1664525u + 1013904223u;
                        const float white = static_cast<float>(static_cast<int32_t>(seed)) * (1.0f / 2147483648.0f);

                        // Blend tone vs noise based on Z (texture) and X (role)
                        const float noiseWeight = normZ * 0.75f + normX * 0.25f;
                        const float rawVal = (1.0f - noiseWeight) * tone + noiseWeight * white;

                        // Decay window
                        const float env = FastMath::fastExp(-t * (2.0f + (3.0f - static_cast<float>(y)) * 4.0f));
                        defaultLattice_[offset + i] = rawVal * env;
                    }
                }
            }
        }
        updateSpatialWeights();
    }

    float sampleRate_{48000.0f};
    float invSr_{1.0f / 48000.0f};
    float basePitchHz_{110.0f};
    float cutoffHz_{16000.0f};
    float drive_{0.0f};
    float srrNorm_{0.0f};

    HypercubeCoordinates coords_{};
    std::array<size_t, 8> activeCornerIndices_{};
    alignas(16) std::array<float, 8> activeCornerWeights_{};

    float phase_{0.0f};
    float phaseInc_{0.0f};
    float ampEnv_{0.0f};
    float ampDecayRate_{0.996f};
    float pitchEnv_{1.0f};
    float pitchDecayRate_{0.992f};
    float pitchPunchAmount_{1.0f};
    float srrCounter_{0.0f};
    float heldSample_{0.0f};
    bool active_{false};

    DSPCommon::CytomicSVF filterStage1_{};
    DSPCommon::CytomicSVF filterStage2_{};
    DSPCommon::DCBlocker dcBlockL_{};
    DSPCommon::DCBlocker dcBlockR_{};

    // Pre-allocated flat buffer for 64 nodes * 2048 samples (~512 KB float)
    alignas(64) std::array<float, K_LATTICE_NODES * K_DEFAULT_NODE_SAMPLES> defaultLattice_{};
};

} // namespace toad

namespace TbdAudio::Oscillators {

/**
 * @brief HyperStrike — 4D Hypercube Percussion Voice (MEGA-PERC-05D / [HS]).
 *        Inherits from SoundEngine8 for universal hosting in TheKlangSuite plugins,
 *        ToadTracker, and Planter modular drum racks.
 *
 * 8-Knob Parameter Topology:
 *   - Knob 0: PITCH / CLOCK   [8 kHz .. 60 kHz / MIDI track]
 *   - Knob 1: TRAVERSE / W    [0.0 .. 1.0 Temporal scrub attack -> tail]
 *   - Knob 2: MORPH X (ROLE)  [0.0 .. 1.0 Kick -> Tom -> Snare -> Hat]
 *   - Knob 3: MORPH Y (DECAY) [0.0 .. 1.0 Tight Snap -> Punch -> Saturated -> Wash]
 *   - Knob 4: MORPH Z (TEXT)  [0.0 .. 1.0 Pure Sine -> Organic -> Saturated -> Noise]
 *   - Knob 5: CUTOFF / VCF    [20 Hz .. 20 kHz Curtis CEM3320 24dB ZDF]
 *   - Knob 6: DRIVE / HEAT    [0.0 .. +24 dB Am6070 Saturation Knee]
 *   - Knob 7: SRR / GLITCH    [44.1 kHz .. 1.0 kHz Sample-Rate Decimation]
 */
class HyperStrike final : public SoundEngine8 {
public:
    HyperStrike() noexcept {
        setParam(0, 0.50f); // PITCH = ~44.1kHz / C2 nominal
        setParam(1, 0.00f); // TRAVERSE / W = 0.0 (sample start)
        setParam(2, 0.00f); // MORPH X = Kick (0.0)
        setParam(3, 0.33f); // MORPH Y = Punchy (0.33)
        setParam(4, 0.00f); // MORPH Z = Pure Sine Tone (0.0)
        setParam(5, 0.85f); // CUTOFF = Open (~16 kHz)
        setParam(6, 0.20f); // DRIVE = Mild Am6070 warmth
        setParam(7, 0.00f); // SRR = Off (clean)
    }

    ~HyperStrike() override = default;

    void reset() noexcept override {
        engine_.reset();
    }

    void renderSampleStereo(float /*baseFreqHz*/, float& outL, float& outR) noexcept override {
        engine_.processStereo(outL, outR);
    }

    toad::HyperStrikeEngine& getEngine() noexcept {
        return engine_;
    }

    const toad::HyperStrikeEngine& getEngine() const noexcept {
        return engine_;
    }

protected:
    void onPrepare(float sampleRate) noexcept override {
        engine_.prepare(sampleRate);
    }

    void onTrigger(float velocity) noexcept override {
        toad::HypercubeCoordinates coords;
        coords.w = getParam(1);
        coords.x = getParam(2);
        coords.y = getParam(3);
        coords.z = getParam(4);
        engine_.setCoordinates(coords);

        // Pitch knob mapping: Profile D logarithmic from 8kHz to 60kHz
        const float pitchClock = 8000.0f * FastMath::fastPow2(getParam(0) * 2.90689f);
        engine_.setPitchHz(pitchClock);

        // Cutoff: Profile C logarithmic 20Hz to 20kHz
        const float cutoffHz = 20.0f * FastMath::fastPow2(getParam(5) * 9.96578f);
        engine_.setCutoffHz(cutoffHz);

        engine_.setDrive(getParam(6));
        engine_.setSRR(getParam(7));

        engine_.trigger(velocity);
    }

private:
    toad::HyperStrikeEngine engine_{};
};

} // namespace TbdAudio::Oscillators
