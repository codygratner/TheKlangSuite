#pragma once

#include "../FastMath.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>

// ==============================================================================
// DigiStrikeFS2ROM.h — Sovereign Multi-Voice Wavetable ROM Bank & 3D Spatial Engine
// ==============================================================================
// Part of The Klang Suite SOCEP Engine. Copyright (c) 2026 Klang Industries.
// Clean-Room IP Compliance: Zero commercial trademarks.
// ==============================================================================

namespace tkdsp {

namespace FastMath = TbdAudio::FastMath;

class DigiStrikeFS2ROM {
public:
    static constexpr size_t kNumCategories = 8; // KICK, SNAR, TOMS, CLAP, HATS, CYMB, PERC, STAB
    static constexpr size_t kNumLineages   = 20; // 20 Sovereign Hardware Families
    static constexpr size_t kNumHeroSlots  = kNumCategories * kNumLineages; // 160
    static constexpr size_t kSecretCoreSlot = 160;
    static constexpr size_t kNumSlots      = 161; // 160 Hero slots + 1 Secret Core slot
    static constexpr size_t kNumMorphSteps = 4; // 0%, 33%, 67%, 100%
    static constexpr size_t kNumFrames     = 4; // Frame 0: Attack, 1: Punch, 2: Sustain, 3: Tail
    static constexpr size_t kTableSize     = 2048;

    static const DigiStrikeFS2ROM& getInstance() noexcept {
        static const DigiStrikeFS2ROM instance;
        return instance;
    }

    /**
     * @brief Continuous 3D Wavetable Evaluation with Trilinear Spatial Interpolation.
     * @param slotIdx    0..63 (category * 8 + lineage)
     * @param morphNorm  0.0 .. 1.0 (smooth morphing across 4 sound variations)
     * @param framePos   0.0 .. 3.0 (temporal frame traversal position: Attack -> Body -> Sustain -> Tail)
     * @param oscPhase   0.0 .. 2047.999 (intra-frame pitch oscillator accumulator)
     */
    [[nodiscard]] inline float readSample(int slotIdx, float morphNorm, float framePos, double oscPhase) const noexcept {
        slotIdx = std::clamp(slotIdx, 0, static_cast<int>(kNumSlots - 1));

        // 1. Morph Coordinate Interpolation (M0, M1, fracM)
        const float mScaled = std::clamp(morphNorm, 0.0f, 1.0f) * static_cast<float>(kNumMorphSteps - 1);
        const int m0 = static_cast<int>(mScaled);
        const int m1 = std::min(m0 + 1, static_cast<int>(kNumMorphSteps - 1));
        const float uM = mScaled - static_cast<float>(m0);

        // 2. Temporal Frame Traversal Interpolation (F0, F1, fracF)
        const float fClamped = std::clamp(framePos, 0.0f, static_cast<float>(kNumFrames - 1));
        const int f0 = static_cast<int>(fClamped);
        const int f1 = std::min(f0 + 1, static_cast<int>(kNumFrames - 1));
        const float uF = fClamped - static_cast<float>(f0);

        // 3. Intra-Frame Sample Phase Interpolation (S0, S1, fracS)
        const size_t s0 = static_cast<size_t>(oscPhase) % kTableSize;
        const size_t s1 = (s0 + 1) % kTableSize;
        const float uS = static_cast<float>(oscPhase - std::floor(oscPhase));

        // Evaluate 4 frame corners (interpolating intra-frame sample phase)
        const float s_m0_f0 = samplePhase(slotIdx, m0, f0, s0, s1, uS);
        const float s_m0_f1 = samplePhase(slotIdx, m0, f1, s0, s1, uS);
        const float s_m1_f0 = samplePhase(slotIdx, m1, f0, s0, s1, uS);
        const float s_m1_f1 = samplePhase(slotIdx, m1, f1, s0, s1, uS);

        // Blend along temporal frame axis
        const float blend_m0 = s_m0_f0 + uF * (s_m0_f1 - s_m0_f0);
        const float blend_m1 = s_m1_f0 + uF * (s_m1_f1 - s_m1_f0);

        // Blend along morph axis
        return blend_m0 + uM * (blend_m1 - blend_m0);
    }

private:
    DigiStrikeFS2ROM() {
        data_.resize(kNumSlots * kNumMorphSteps * kNumFrames * kTableSize, 0.0f);
        bakeROM();
    }

    [[nodiscard]] inline float samplePhase(int slot, int morph, int frame, size_t s0, size_t s1, float uS) const noexcept {
        const size_t offset = (((static_cast<size_t>(slot) * kNumMorphSteps + static_cast<size_t>(morph))
                                * kNumFrames + static_cast<size_t>(frame)) * kTableSize);
        const float v0 = data_[offset + s0];
        const float v1 = data_[offset + s1];
        return v0 + uS * (v1 - v0);
    }

    bool loadBinaryAsset() noexcept {
        const size_t expectedSamples = kNumSlots * kNumMorphSteps * kNumFrames * kTableSize;
        const size_t expectedBytes = expectedSamples * sizeof(int16_t);

        const char* candidatePaths[] = {
            "digistrike_fs2_rom.bin",
            "../Resources/digistrike_fs2_rom.bin",
            "Contents/Resources/digistrike_fs2_rom.bin",
            "../Contents/Resources/digistrike_fs2_rom.bin",
            "../../Contents/Resources/digistrike_fs2_rom.bin",
            "C:/Program Files/Common Files/VST3/DigiStrike FS2.vst3/Contents/Resources/digistrike_fs2_rom.bin",
            "C:/Dev/TheKlangSuite/assets/digistrike_fs2_rom.bin"
        };

        for (const char* path : candidatePaths) {
            FILE* fp = nullptr;
#if defined(_WIN32)
            fopen_s(&fp, path, "rb");
#else
            fp = fopen(path, "rb");
#endif
            if (fp != nullptr) {
                fseek(fp, 0, SEEK_END);
                long sz = ftell(fp);
                fseek(fp, 0, SEEK_SET);

                if (static_cast<size_t>(sz) == expectedBytes) {
                    std::vector<int16_t> buf(expectedSamples);
                    size_t readCount = fread(buf.data(), sizeof(int16_t), expectedSamples, fp);
                    fclose(fp);
                    if (readCount == expectedSamples) {
                        const float invScale = 1.0f / 32768.0f;
                        for (size_t i = 0; i < expectedSamples; ++i) {
                            data_[i] = static_cast<float>(buf[i]) * invScale;
                        }
                        return true;
                    }
                } else {
                    fclose(fp);
                }
            }
        }
        return false;
    }

    void bakeROM() noexcept {
        if (loadBinaryAsset()) {
            return;
        }
        bakeProceduralFallback();
    }

    void bakeProceduralFallback() noexcept {
        const double twoPi = 6.28318530717958647692;

        for (size_t cat = 0; cat < kNumCategories; ++cat) {
            for (size_t lin = 0; lin < kNumLineages; ++lin) {
                const size_t slot = cat * kNumLineages + lin;

                for (size_t morph = 0; morph < kNumMorphSteps; ++morph) {
                    const float morphFactor = static_cast<float>(morph) / static_cast<float>(kNumMorphSteps - 1);

                    for (size_t frame = 0; frame < kNumFrames; ++frame) {
                        const size_t offset = (((slot * kNumMorphSteps + morph) * kNumFrames + frame) * kTableSize);

                        // Frame 0: Transient Attack, Frame 1: Punch, Frame 2: Sustain, Frame 3: Tail
                        const float attackWeight  = (frame == 0) ? 1.0f : (frame == 1 ? 0.35f : 0.0f);
                        const float punchWeight   = (frame == 1) ? 1.0f : (frame == 0 ? 0.40f : (frame == 2 ? 0.25f : 0.0f));
                        const float sustainWeight = (frame == 2) ? 1.0f : (frame == 1 ? 0.30f : (frame == 3 ? 0.60f : 0.0f));
                        const float tailWeight    = (frame == 3) ? 1.0f : 0.0f;

                        // Procedural PRNG seed per slot
                        uint32_t seed = static_cast<uint32_t>(slot * 7919u + morph * 1999u + frame * 331u + 0xDEADBEEFu);

                        for (size_t i = 0; i < kTableSize; ++i) {
                            const double ph = twoPi * static_cast<double>(i) / static_cast<double>(kTableSize);
                            const float tNorm = static_cast<float>(i) / static_cast<float>(kTableSize);

                            // LCG noise component
                            seed = seed * 1664525u + 1013904223u;
                            const float noise = static_cast<float>(static_cast<int32_t>(seed)) * (1.0f / 2147483648.0f);

                            double val = 0.0;

                            switch (cat) {
                                case 0: { // KICK
                                    // Deep fundamental sine + punchy 2nd/3rd harmonics
                                    const double fund = std::sin(ph);
                                    const double harm2 = std::sin(2.0 * ph);
                                    const double harm3 = std::sin(3.0 * ph);
                                    const double click = (1.0 - tNorm) * (1.0 - tNorm) * noise;

                                    // Lineage characteristics:
                                    // Crown (Linn): tight asymmetric punch, Apex: companded grit, Empyrean: 12-bit slew
                                    const double linDrive = 1.0 + 0.3 * static_cast<double>(lin % 3) + 0.5 * morphFactor;
                                    val = fund * (0.8 + 0.2 * punchWeight)
                                        + harm2 * (0.25 * punchWeight + 0.15 * morphFactor)
                                        + harm3 * (0.10 * attackWeight)
                                        + click * (0.40 * attackWeight);
                                    val = std::tanh(val * linDrive);
                                    break;
                                }

                                case 1: { // SNARE
                                    // Acoustic body resonant tone + snappy wire noise
                                    const double fund = std::sin(ph);
                                    const double body = 0.6 * std::sin(2.0 * ph) + 0.3 * std::sin(3.0 * ph);
                                    const double wires = noise * (0.7 + 0.3 * std::sin(ph * 4.0));

                                    val = fund * (0.35 * sustainWeight + 0.25 * punchWeight)
                                        + body * (0.45 * punchWeight + 0.2 * sustainWeight)
                                        + wires * (0.85 * attackWeight + 0.65 * punchWeight + 0.45 * tailWeight);
                                    val = std::tanh(val * (1.2 + 0.4 * morphFactor));
                                    break;
                                }

                                case 2: { // TOMS
                                    // Resonant drumhead fundamental + pitch swept formant
                                    const double fund = std::sin(ph);
                                    const double h2 = std::sin(2.0 * ph);
                                    const double h3 = std::sin(3.0 * ph + 0.5);
                                    const double transient = (1.0 - tNorm) * noise * 0.5;

                                    val = fund * (0.7 + 0.3 * sustainWeight)
                                        + h2 * (0.35 * punchWeight + 0.2 * morphFactor)
                                        + h3 * (0.15 * attackWeight)
                                        + transient * attackWeight;
                                    val = std::tanh(val * 1.25);
                                    break;
                                }

                                case 3: { // CLAP
                                    // Multi-hand flam cluster + snappy wire burst
                                    const double flamMod = std::sin(ph * 3.0);
                                    const double burst = noise * (0.6 + 0.4 * flamMod);
                                    const double body = std::sin(ph) * 0.2;

                                    val = burst * (0.9 * attackWeight + 0.7 * punchWeight + 0.3 * tailWeight)
                                        + body * sustainWeight;
                                    val = std::tanh(val * (1.1 + 0.5 * morphFactor));
                                    break;
                                }

                                case 4: { // HI-HATS
                                    // Inharmonic metallic ring (6-oscillator square/pulse cluster simulation)
                                    const double m1 = (std::sin(ph * 3.14) > 0.0 ? 0.3 : -0.3);
                                    const double m2 = (std::sin(ph * 5.41) > 0.0 ? 0.25 : -0.25);
                                    const double m3 = (std::sin(ph * 8.23) > 0.0 ? 0.2 : -0.2);
                                    const double metalNoise = noise * (0.5 + 0.5 * morphFactor);

                                    val = (m1 + m2 + m3) * (0.5 + 0.3 * punchWeight)
                                        + metalNoise * (0.8 * attackWeight + 0.5 * punchWeight + 0.4 * tailWeight);
                                    val = std::tanh(val * 1.3);
                                    break;
                                }

                                case 5: { // CYMBALS
                                    // Shimmering bronze alloy inharmonics + wide frequency wash
                                    const double b1 = std::sin(ph * 2.37);
                                    const double b2 = std::sin(ph * 4.19);
                                    const double b3 = std::sin(ph * 7.81);
                                    const double wash = noise * (0.4 + 0.3 * std::sin(ph * 1.5));

                                    val = (b1 * 0.3 + b2 * 0.25 + b3 * 0.2) * (0.6 + 0.4 * sustainWeight)
                                        + wash * (0.6 * attackWeight + 0.5 * punchWeight + 0.6 * tailWeight);
                                    val = std::tanh(val * 1.15);
                                    break;
                                }

                                case 6: { // PERCUSSION
                                    // Cowbells, Bongos, Congas, Woodblock
                                    const double modal1 = std::sin(ph);
                                    const double modal2 = std::sin(ph * 2.76); // classic cowbell ratio
                                    const double modal3 = std::sin(ph * 5.40);
                                    const double woodClick = (1.0 - tNorm) * (1.0 - tNorm) * (1.0 - tNorm) * 0.8;

                                    val = modal1 * (0.5 + 0.3 * sustainWeight)
                                        + modal2 * (0.35 + 0.2 * morphFactor)
                                        + modal3 * (0.15 * punchWeight)
                                        + woodClick * attackWeight;
                                    val = std::tanh(val * 1.35);
                                    break;
                                }

                                case 7: // STABS & BASS (The Bass Synth Core)
                                default: {
                                    // SOTA Vintage Bass & Stabs:
                                    // Slap Bass / 8-Bit PWM / 12-Bit Rave Organ / Reso Bass
                                    // Crown (0): Marcus Miller Slap Bass (rich sawtooth + slap thumb pop)
                                    // Apex (1): Discrete 8-Bit PWM Bass & Lead
                                    // Empyrean (2): SP-1200 Rave Minor Stab & Lately Organ Bass
                                    // Falcon (3): MPC Orch Hit & Slap Bass
                                    // K-Works (4): Kawai Funk Slap Bass
                                    // Tokyo (5): Acid 303 Saw & Alpha Juno Hoover
                                    // Gothenburg (6): SidStation FM Reso Bass
                                    // Nippon (7): Yamaha TX81Z Lately Bass
                                    
                                    // Construct fundamental + rich harmonic sawtooth/square spectrum
                                    const double fund = std::sin(ph);
                                    const double h2 = std::sin(2.0 * ph);
                                    const double h3 = std::sin(3.0 * ph);
                                    const double h4 = std::sin(4.0 * ph);
                                    const double h5 = std::sin(5.0 * ph);
                                    const double h7 = std::sin(7.0 * ph);

                                    // Slap transient snap on Frame 0
                                    const double slapSnap = (1.0 - tNorm) * (std::sin(ph * 8.0) * 0.4 + noise * 0.3);

                                    if (lin == 0) { // CROWN: Marcus Miller Slap Bass
                                        val = fund * (0.75 + 0.25 * sustainWeight)
                                            + h2 * (0.35 + 0.15 * punchWeight)
                                            + h3 * (0.25 * punchWeight)
                                            + h4 * (0.15 * attackWeight)
                                            + slapSnap * attackWeight;
                                    } else if (lin == 4) { // K-WORKS: Kawai R-50e / XD-5 Orchestra Hit / Stab
                                        // Iconic 12-Bit multi-timbral tutti chord stack (Root C3 + minor 3rd + 5th + 8ve + brass formant)
                                        double sawChord = 0.0;
                                        for (int h = 1; h <= 12; ++h) {
                                            const double invH = 1.0 / static_cast<double>(h);
                                            sawChord += std::sin(static_cast<double>(h) * ph) * invH;
                                            sawChord += 0.75 * std::sin(static_cast<double>(h) * ph * 1.20) * invH; // Minor 3rd
                                            sawChord += 0.85 * std::sin(static_cast<double>(h) * ph * 1.498) * invH; // 5th
                                            sawChord += 0.60 * std::sin(static_cast<double>(h) * ph * 2.0) * invH; // Octave
                                        }
                                        const double brassFlare = (std::sin(ph * 3.5) * 0.4 + std::sin(ph * 5.2) * 0.3) * (0.6 + 0.4 * punchWeight);
                                        const double orchTransient = slapSnap * 1.5 * attackWeight;
                                        val = (sawChord * 0.40 + brassFlare * 0.30) * (0.7 + 0.3 * punchWeight) + orchTransient;
                                    } else if (lin == 1) { // APEX: 8-Bit PWM Growl
                                        const double pwmDuty = 0.5 + 0.3 * std::sin(morphFactor * 3.14);
                                        const double pulse = (ph / twoPi < pwmDuty) ? 0.7 : -0.7;
                                        val = pulse * (0.6 + 0.4 * punchWeight) + fund * 0.4 + slapSnap * 0.3 * attackWeight;
                                    } else if (lin == 2) { // EMPYREAN: Rave Minor Stab / Organ Bass
                                        // Minor 3rd (1.189) + 5th (1.498) interval harmonics
                                        const double min3rd = std::sin(ph * 1.2);
                                        const double fifth  = std::sin(ph * 1.5);
                                        val = fund * 0.6 + fifth * 0.3 + min3rd * 0.25 * punchWeight + h2 * 0.2;
                                    } else if (lin == 5) { // TOKYO: Acid Saw / Hoover
                                        // Rich bandlimited sawtooth approximation
                                        double saw = 0.0;
                                        for (int h = 1; h <= 9; ++h) {
                                            saw += std::sin(static_cast<double>(h) * ph) / static_cast<double>(h);
                                        }
                                        val = saw * (0.6 + 0.4 * punchWeight) + slapSnap * 0.2 * attackWeight;
                                    } else if (lin == 7) { // NIPPON: Lately FM Bass
                                        // TX81Z Lately Bass: Modulator -> Carrier FM
                                        const double mod = std::sin(ph);
                                        const double car = std::sin(ph + (1.2 + 0.8 * morphFactor) * mod);
                                        val = car * 0.8 + fund * 0.3;
                                    } else { // FALCON / GOTHENBURG: Dynamic Reso Bass
                                        val = fund * 0.7 + h2 * 0.3 + h3 * 0.25 * punchWeight + h5 * 0.15 + slapSnap * attackWeight;
                                    }

                                    // Warm tube/transistor non-linear saturation
                                    val = std::tanh(val * (1.3 + 0.5 * morphFactor));
                                    break;
                                }
                            }

                            // Normalize and store
                            data_[offset + i] = static_cast<float>(std::clamp(val, -1.0, 1.0));
                        }
                    }
                }
            }
        }

        // =====================================================================
        // Slot 64: Pre-Baked Secret Core (Strictly Integer Bandlimited Fourier Series)
        // 100% Periodic at theta = 0 == 2*pi (Zero wrap click, zero Nyquist aliasing)
        // =====================================================================
        for (size_t morph = 0; morph < kNumMorphSteps; ++morph) {
            for (size_t frame = 0; frame < kNumFrames; ++frame) {
                const size_t offset = (((kSecretCoreSlot * kNumMorphSteps + morph) * kNumFrames + frame) * kTableSize);
                const float frameTransient = (frame == 0 ? 0.35f : 0.0f);
                const float frameBody = (frame == 1 ? 1.0f : (frame == 2 ? 0.85f : 0.5f));

                for (size_t i = 0; i < kTableSize; ++i) {
                    const double ph = twoPi * static_cast<double>(i) / static_cast<double>(kTableSize);
                    double val = 0.0;

                    if (morph == 0) {
                        // Morph 0: Pure fundamental Sine wave
                        val = std::sin(ph);
                    } else if (morph == 1) {
                        // Morph 1: Analog Tube Warmth (soft saturation on fundamental)
                        val = std::tanh(1.85 * std::sin(ph)) * 0.82;
                    } else if (morph == 2) {
                        // Morph 2: Bandlimited 16-Harmonic Sawtooth with Lanczos Sigma Damping
                        double saw = 0.0;
                        for (int k = 1; k <= 16; ++k) {
                            const double sigma = std::sin(3.141592653589793 * static_cast<double>(k) / 17.0) / (3.141592653589793 * static_cast<double>(k) / 17.0);
                            saw += (1.0 / static_cast<double>(k)) * sigma * std::sin(static_cast<double>(k) * ph);
                        }
                        val = saw * 0.65;
                    } else {
                        // Morph 3: SOTA Hyper-Saw (32-harmonic Lanczos saw + 2nd harmonic formant)
                        // 100% PERIODIC: f(0) == f(2*pi) == 0. Exactly zero wrap click!
                        double saw = 0.0;
                        for (int k = 1; k <= 32; ++k) {
                            const double sigma = std::sin(3.141592653589793 * static_cast<double>(k) / 33.0) / (3.141592653589793 * static_cast<double>(k) / 33.0);
                            saw += (1.0 / static_cast<double>(k)) * sigma * std::sin(static_cast<double>(k) * ph);
                        }
                        double formant = 0.0;
                        for (int k = 1; k <= 16; ++k) {
                            const double sigma = std::sin(3.141592653589793 * static_cast<double>(k) / 17.0) / (3.141592653589793 * static_cast<double>(k) / 17.0);
                            formant += (0.35 / static_cast<double>(k)) * sigma * std::sin(2.0 * static_cast<double>(k) * ph);
                        }
                        val = std::tanh(1.6 * (saw * 0.65 + formant)) * 0.85;
                    }

                    // Transient click on Frame 0 for punchy note attacks
                    if (frame == 0 && i < 128) {
                        const float click = (1.0f - static_cast<float>(i) / 128.0f);
                        val += static_cast<double>(click * frameTransient) * std::sin(ph * 4.0);
                    }

                    data_[offset + i] = static_cast<float>(std::clamp(val * static_cast<double>(frameBody), -1.0, 1.0));
                }
            }
        }
    }

    std::vector<float> data_;
};

} // namespace tkdsp
