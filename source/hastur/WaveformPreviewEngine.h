#pragma once

#include <array>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include "SubAPF16.h"

namespace CrackedChimes::DSP {

/**
 * @brief Zero-Allocation Offline Single-Cycle Waveform Preview Generator.
 * 
 * Features:
 * - High-resolution rendering: 1024 points for Core DCO, 512 points for Sub-Stator.
 * - 4096-sample pre-roll warmup to guarantee IIR all-pass and DC-blocker steady-state limit cycles.
 * - Fixed Sub APF output tap: renders filtered output (tap / dcPostOut), not unfiltered square wave.
 * - Multi-octave Sub timebase window (scales period count with subOctave: 1, 2, 4, 8 cycles across 512 pts).
 * - Pre-computed constexpr targetA all-pass coefficient table.
 * - Zero CRT formatting: compile-time static string tables preventing MSVC UTF-16 ideograph mojibake.
 */
struct WaveformPreviewEngine {
    static constexpr int kCoreResolution = 1024;
    static constexpr int kSubResolution  = 512;
    static constexpr int kWarmupSamples  = 4096;
    static constexpr float kTwoPi        = 6.28318530717958647692f;
    static constexpr float kPi           = 3.14159265358979323846f;
    static constexpr int kNumStages      = 16;

    // Static constexpr label tables for HUD status overlays
    static constexpr const char* kFamilyNames[4] = { "SAW", "PULSE", "TRIANGLE", "SINE" };
    static constexpr const char* kTopoNames[4]   = { "OFF", "2x CHOP", "4x CHOP", "8x CHOP" };
    static constexpr const char* kSubOctNames[4] = { "-2 OCT", "-1 OCT", "UNISON", "+1 OCT" };
    static constexpr const char* kSubTapNames[4] = { "BYPASS", "4-POLE", "8-POLE", "16-POLE" };

    // Compile-time pre-computed 16-stage geometric APF pole coefficients (200Hz to 4000Hz at 48kHz)
    static constexpr std::array<float, kNumStages> kPrecomputedA = {
        -0.9741569f, -0.9685218f, -0.9616704f, -0.9533652f,
        -0.9433084f, -0.9311618f, -0.9165198f, -0.8989119f,
        -0.8778105f, -0.8526102f, -0.8226521f, -0.7872011f,
        -0.7454811f, -0.6966762f, -0.6399122f, -0.5742877f
    };

    // =========================================================================
    // 1. Core DCO Transfer Function (1024 points, Strangle 32x & Chopper 8x safe)
    // =========================================================================
    static void generateCorePreview(
        std::array<float, kCoreResolution>& outBuffer,
        int family,
        float morph,
        int topo,
        float strangle,
        float warpOctaves,
        float meltDrive,
        float bite) noexcept
    {
        // Strangle Multiplier & Sync Calculation (1.0 to 32.0 linear)
        const float mult = std::clamp(strangle, 1.0f, 32.0f);
        const float syncAmount = (mult > 1.001f) ? 1.0f : 0.0f;

        // CMOS Chopper Multiplier (topo 0 = Off, 1 = 2x, 2 = 4x, 3 = 8x)
        int M = 1;
        if (topo == 1)      M = 2;
        else if (topo == 2) M = 4;
        else if (topo == 3) M = 8;

        // APF Warp Coefficient (fc = 400Hz * 2^warp at 48kHz preview base)
        const float warpFc = std::clamp(400.0f * std::exp2(warpOctaves), 20.0f, 20000.0f);
        const float warpTan = std::tan(std::min(kPi * warpFc / 48000.0f, 1.52f));
        const float apfA = (warpTan - 1.0f) / (warpTan + 1.0f);

        // Filter states
        float apfState = 0.0f;
        float apf2State = 0.0f;
        float prevBiteFb = 0.0f;
        float dcX = 0.0f, dcY = 0.0f;
        float slavePhase = 0.0f;

        const int totalSamples = kWarmupSamples + kCoreResolution;
        const float phaseStep = (mult / static_cast<float>(kCoreResolution));

        for (int n = 0; n < totalSamples; ++n) {
            slavePhase += phaseStep;
            if (slavePhase >= 1.0f) {
                slavePhase -= std::floor(slavePhase);
            }

            // Master wrap sync reset every kCoreResolution samples
            if ((n % kCoreResolution) == 0 && n > 0) {
                if (syncAmount >= 0.0f) {
                    slavePhase *= (1.0f - syncAmount);
                } else {
                    const float h = std::abs(syncAmount);
                    slavePhase = std::fmod(slavePhase + h * 0.5f, 1.0f);
                }
            }

            const float p = slavePhase;

            // A. Family Wave Generation
            float rawCore = 0.0f;
            switch (family) {
                case 0: { // SAW: Divided 2x Chopper Saw PWM (Dual-Pulse Spikes)
                    const float rawSaw = 1.0f - 2.0f * p;
                    if (morph > 0.001f) {
                        const float w = std::clamp(morph * 0.45f, 0.0f, 0.45f);
                        if (p < w || (p >= 0.5f && p < 0.5f + w)) {
                            rawCore = -1.0f;
                        } else {
                            rawCore = rawSaw;
                        }
                    } else {
                        rawCore = rawSaw;
                    }
                    break;
                }
                case 1: { // PULSE: Symmetric square -> variable pulse
                    const float d = std::clamp(0.50f + morph * 0.45f, 0.05f, 0.95f);
                    const float rawPulse = (p < d) ? 1.0f : -1.0f;
                    const float minD = std::min(d, 1.0f - d);
                    const float gainComp = (minD < 0.499f) ? std::min(2.0f, 0.5f / std::sqrt(minD)) : 1.0f;
                    rawCore = rawPulse * gainComp;
                    break;
                }
                case 2: { // TRIANGLE: Pure triangle -> Sharktooth
                    const float tPeak = std::clamp(0.50f + morph * 0.49f, 0.01f, 0.99f);
                    if (p < tPeak) {
                        rawCore = -1.0f + 2.0f * (p / tPeak);
                    } else {
                        rawCore = 1.0f - 2.0f * ((p - tPeak) / (1.0f - tPeak));
                    }
                    break;
                }
                case 3: // EXOTIC: Warm sine -> CZ resonant windowed sine
                default: {
                    const float s = std::sin(p * kTwoPi);
                    if (morph <= 0.001f) {
                        rawCore = s;
                    } else if (morph <= 0.5f) {
                        const float d = 1.0f + morph * 8.0f;
                        rawCore = std::tanh(s * d) / std::tanh(d);
                    } else {
                        const float k = 1.0f + (morph - 0.5f) * 6.0f;
                        const float window = 0.5f * (1.0f + std::cos(p * kTwoPi));
                        const float czP = std::fmod(p * k, 1.0f);
                        rawCore = std::sin(czP * kTwoPi) * window;
                    }
                    break;
                }
            }

            // B. Bipolar BITE Feedback Skew
            float inWithFb = rawCore;
            if (std::abs(bite) > 0.001f) {
                const float effBite = bite * 0.70f;
                const float satFb = std::tanh(prevBiteFb * (1.0f + std::abs(effBite))) * 0.80f;
                inWithFb = rawCore - (effBite * 0.85f) * satFb;
            }

            // C. WARP: Inverted APF Dispersion
            const float apfIn = inWithFb;
            const float apfOut = apfA * apfIn + apfState;
            apfState = apfIn - apfA * apfOut;
            prevBiteFb = apfOut;

            // D. MELT: Multi-Stage Infinifold Wavefolding (1 to 4 clean folds)
            float folded = apfOut;
            if (meltDrive > 0.001f) {
                const float gain = 1.0f + meltDrive * 8.0f;
                float v = apfOut * gain;
                for (int i = 0; i < 6; ++i) {
                    if (v > 1.0f) {
                        v = 2.0f - v;
                    } else if (v < -1.0f) {
                        v = -2.0f - v;
                    } else {
                        break;
                    }
                }
                folded = std::clamp(v, -1.0f, 1.0f);
            }

            // E. CMOS Chopper (topo 0 = off, 1 = 2x, 2 = 4x, 3 = 8x)
            float chopped = folded;
            if (topo > 0) {
                const float pM = std::fmod(p * static_cast<float>(M), 1.0f);
                const float gate = (pM < 0.5f) ? 1.0f : 0.0f;
                const float mean = -0.25f / static_cast<float>(M);
                chopped = (folded * gate - mean) * 1.35f;
            }

            // F. APF 2 (Post-Chopper Forward APF — Laser Sandwich Stage 2)
            const float apf2In = chopped;
            const float apf2Out = apfA * apf2In + apf2State;
            apf2State = apf2In - apfA * apf2Out;
            prevBiteFb = apf2Out; // Bite feedback loop taps true post-chopper APF output

            // G. MELT: Normalized Tanh Overdrive
            float satOut = apf2Out;
            if (meltDrive > 0.001f) {
                const float g = 1.0f + meltDrive * 2.5f;
                satOut = std::tanh(apf2Out * g) / std::tanh(g);
            }

            // G. Leaky DC Blocker (R = 0.995)
            const float dcOut = satOut - dcX + 0.995f * dcY;
            dcX = satOut;
            dcY = dcOut;

            // Capture final cycle into outBuffer
            if (n >= kWarmupSamples) {
                outBuffer[n - kWarmupSamples] = std::clamp(dcOut, -1.25f, 1.25f);
            }
        }
    }

    // =========================================================================
    // 2. Subordinate APF Transfer Function (512 points, 4-cycle timebase window)
    // =========================================================================
    static void generateSubPreview(
        std::array<float, kSubResolution>& outBuffer,
        int subOctave,
        int subTrain,
        int apfTap,
        float chewOct,
        float spit = 0.0f) noexcept
    {
        // Fixed 1 cycle standard for preview window across all sub octaves:
        const float periodCount = 1.0f;
        float subMult = 0.5f;
        if (subOctave == -2)      { subMult = 0.25f; }
        else if (subOctave == -1) { subMult = 0.5f; }
        else if (subOctave == 0)  { subMult = 1.0f; }
        else if (subOctave == 1)  { subMult = 2.0f; }

        const double fSub = 110.0 * static_cast<double>(subMult);
        std::array<float, kNumStages> aTab{};
        for (int i = 0; i < kNumStages; ++i) {
            aTab[i] = static_cast<float>(SubAPF16::poleCoeff(i, fSub, static_cast<double>(chewOct), 48000.0));
        }

        std::array<float, kNumStages> s{};
        float fb = 0.0f;
        float dcPreX = 0.0f, dcPreY = 0.0f;
        float dcPostX = 0.0f, dcPostY = 0.0f;

        const int totalSamples = kWarmupSamples + kSubResolution;
        const float phaseStep = periodCount / static_cast<float>(kSubResolution);
        float subPhase = 0.0f;

        for (int n = 0; n < totalSamples; ++n) {
            subPhase += phaseStep;
            if (subPhase >= 1.0f) {
                subPhase -= std::floor(subPhase);
            }

            const float phase = subPhase;

            // A. Divided Logic Sub Pulse Train
            float rawSub = -1.0f;
            if (phase < 0.5f) {
                if (subTrain == 0) {
                    rawSub = 1.0f; // 50% square
                } else if (subTrain == 1) {
                    rawSub = (phase < 0.25f) ? 1.0f : -1.0f; // 25% duty pulse
                } else {
                    const float numPulses = static_cast<float>(subTrain);
                    const float actPhase = phase * 2.0f * numPulses;
                    const float frac = actPhase - std::floor(actPhase);
                    rawSub = (frac < 0.5f) ? 1.0f : -1.0f;
                }
            }

            // Wire Bypass check strictly via apfTap == 0
            if (apfTap == 0) {
                s.fill(0.0f);
                fb = 0.0f;
                if (n >= kWarmupSamples) {
                    outBuffer[n - kWarmupSamples] = rawSub;
                }
                continue;
            }

            // B. 16-Stage All-Pass Cascade
            float v = rawSub + fb;
            float t4 = 0.0f, t8 = 0.0f;

            for (int i = 0; i < kNumStages; ++i) {
                const float a = aTab[i];
                const float y = a * v + s[i];
                s[i] = v - a * y;
                v = y;

                if (i == 3) t4 = v;
                if (i == 7) t8 = v;
            }

            // Tap Selection (1 = 4-Pole, 2 = 8-Pole, 3 = 16-Pole)
            float tap = v;
            if (apfTap == 1)      tap = t4;
            else if (apfTap == 2) tap = t8;

            // Pre-tanh DC Block (~5 Hz)
            const float dcPreOut = tap - dcPreX + 0.9993f * dcPreY;
            dcPreX = tap;
            dcPreY = dcPreOut;

            // Padé fast tanh saturation
            const float xSat = dcPreOut * 2.0f;
            const float x2 = xSat * xSat;
            const float sat = (xSat < -3.0f) ? -1.0f : (xSat > 3.0f ? 1.0f : xSat * (27.0f + x2) / (27.0f + 9.0f * x2));

            // Post-tanh DC Block
            const float dcPostOut = sat - dcPostX + 0.9993f * dcPostY;
            dcPostX = sat;
            dcPostY = dcPostOut;

            // Resonant feedback loop scaled by bipolar spit (capped at 0.45 * spit)
            fb = std::clamp(spit, -1.0f, 1.0f) * 0.45f * dcPostOut;

            // Output filtered signal (dcPostOut or tap), NOT unfiltered input v!
            if (n >= kWarmupSamples) {
                outBuffer[n - kWarmupSamples] = std::clamp(dcPostOut * 1.15f, -1.35f, 1.35f);
            }
        }
    }
};

} // namespace CrackedChimes::DSP
