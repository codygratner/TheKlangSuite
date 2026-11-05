#pragma once

#include <algorithm>
#include <cmath>
#include "FastMath.h"
#include "../common/SteppedRandom.h"

namespace tkdsp::formless {

namespace FastMath = TbdAudio::FastMath;

/**
 * Headroom-Proportional Bipolar Scaling
 * Maps macro [0..1] with 0.5 neutral onto base parameter [0..1].
 * When m >= 0: lerps base toward 1.0 using remaining upper headroom.
 * When m < 0:  lerps base toward 0.0 using remaining lower headroom.
 * Eliminates clipping dead-zones when base is near extremes.
 */
inline constexpr float headroomLerp(float base, float macroNorm, float depth = 1.0f) noexcept {
    base = std::clamp(base, 0.0f, 1.0f);
    macroNorm = std::clamp(macroNorm, 0.0f, 1.0f);
    depth = std::clamp(depth, 0.0f, 1.0f);

    const float m = std::clamp(2.0f * (macroNorm - 0.5f), -1.0f, 1.0f) * depth;
    if (m >= 0.0f) {
        return base + (1.0f - base) * m;
    } else {
        return base + base * m; // m is negative: base * (1 + m)
    }
}

/**
 * Sequential Arbitration:
 * Stage 1: Macro scales base to patch operating point (basePrime).
 * Stage 2: Velocity dynamically modulates basePrime per hit.
 */
inline constexpr float sequentialArbitrate(float base, float macroNorm, float macroDepth,
                                          float velAmount, float velDepth, float hitVelocity) noexcept {
    const float basePrime = headroomLerp(base, macroNorm, macroDepth);
    if (velDepth <= 0.0001f || hitVelocity <= 0.0001f) return basePrime;

    const float vThrow = (velAmount - 0.5f) * 2.0f * std::clamp(hitVelocity, 0.0f, 1.0f) * velDepth;
    if (vThrow >= 0.0f) {
        return basePrime + (1.0f - basePrime) * vThrow;
    } else {
        return basePrime + basePrime * vThrow;
    }
}

/**
 * Range structure for GUI Arc Halos
 */
struct HaloRange {
    float lo{0.0f};
    float hi{0.0f};
    float pip{0.0f};
    bool active{false};
};

/**
 * Computes the arc halo bounds for a knob driven by a macro.
 */
inline HaloRange computeMacroHalo(float baseVal, float macroNorm, float depth = 1.0f) noexcept {
    HaloRange r;
    const float eff = headroomLerp(baseVal, macroNorm, depth);
    r.lo = std::min(baseVal, eff);
    r.hi = std::max(baseVal, eff);
    r.pip = eff;
    const float diff = eff - baseVal;
    r.active = ((diff < 0.0f ? -diff : diff) > 0.002f);
    return r;
}

/**
 * Trickle-Down Ecoknobics: Headroom Propagation (Knobs A & C modulated by Macro X)
 */
inline constexpr HaloRange trickleDownHeadroom(float baseVal,
                                              float mLo, float mHi, float mPip,
                                              float depth = 1.0f) noexcept {
    const float pip = headroomLerp(baseVal, mPip, depth);
    const float v1  = headroomLerp(baseVal, mLo,  depth);
    const float v2  = headroomLerp(baseVal, mHi,  depth);

    HaloRange r;
    r.lo = std::clamp(std::min(v1, v2), 0.0f, 1.0f);
    r.hi = std::clamp(std::max(v1, v2), 0.0f, 1.0f);
    r.pip = std::clamp(pip, 0.0f, 1.0f);
    const float diff = r.hi - r.lo;
    r.active = (diff > 0.002f);
    return r;
}

/**
 * Trickle-Down Ecoknobics: Arbitration Propagation (Knobs E, F, G, H modulated by Macro + Velocity)
 * Evaluates the 6-point convex bounding hull over (macro x velocity) space.
 */
inline constexpr HaloRange trickleDownArbitrate(float baseVal,
                                               float mLo, float mHi, float mPip, float mDepth,
                                               float vLo, float vHi, float vPip, float vDepth,
                                               float hitVel) noexcept {
    const float pip = sequentialArbitrate(baseVal, mPip, mDepth, vPip, vDepth, hitVel);

    // Evaluate 4 corners of the macro x velocity bounding box at maximum throw (vel = 1.0f)
    const float c1 = sequentialArbitrate(baseVal, mLo, mDepth, vLo, vDepth, 1.0f);
    const float c2 = sequentialArbitrate(baseVal, mLo, mDepth, vHi, vDepth, 1.0f);
    const float c3 = sequentialArbitrate(baseVal, mHi, mDepth, vLo, vDepth, 1.0f);
    const float c4 = sequentialArbitrate(baseVal, mHi, mDepth, vHi, vDepth, 1.0f);

    // Also include zero velocity (where velocity throw is 0.0)
    const float c5 = headroomLerp(baseVal, mLo, mDepth);
    const float c6 = headroomLerp(baseVal, mHi, mDepth);

    const float minVal = std::min({c1, c2, c3, c4, c5, c6});
    const float maxVal = std::max({c1, c2, c3, c4, c5, c6});

    HaloRange r;
    r.lo = std::clamp(minVal, 0.0f, 1.0f);
    r.hi = std::clamp(maxVal, 0.0f, 1.0f);
    r.pip = std::clamp(pip, 0.0f, 1.0f);
    const float diff = r.hi - r.lo;
    r.active = (diff > 0.002f);
    return r;
}

/**
 * Trickle-Down Ecoknobics: Compound Decay Propagation (Knobs B & D modulated by Macro Decay + Macro T)
 * Log-additive compounding in normalized parameter space around noon (0.5).
 */
inline constexpr HaloRange trickleDownDecay(float baseVal,
                                           float decLo, float decHi, float decPip,
                                           float tLo, float tHi, float tPip,
                                           float depth = 0.70f) noexcept {
    const float effDecPip = 0.5f + (decPip - 0.5f) + (tPip - 0.5f);
    const float effDecLo  = 0.5f + (decLo  - 0.5f) + (tLo  - 0.5f);
    const float effDecHi  = 0.5f + (decHi  - 0.5f) + (tHi  - 0.5f);

    const float pip = headroomLerp(baseVal, effDecPip, depth);
    const float v1  = headroomLerp(baseVal, effDecLo,  depth);
    const float v2  = headroomLerp(baseVal, effDecHi,  depth);

    HaloRange r;
    r.lo = std::clamp(std::min(v1, v2), 0.0f, 1.0f);
    r.hi = std::clamp(std::max(v1, v2), 0.0f, 1.0f);
    r.pip = std::clamp(pip, 0.0f, 1.0f);
    const float diff = r.hi - r.lo;
    r.active = (diff > 0.002f);
    return r;
}

// ==============================================================================
// Wide-Authority Mathematical Transfer Functions (SSoT DSP & GUI Telemetry)
// ==============================================================================

/** Knob A: Tune semitones (-24.0st .. 0.0st .. +24.0st) */
inline constexpr float evalTuneSemitones(float p) noexcept {
    return (std::clamp(p, 0.0f, 1.0f) - 0.5f) * 48.0f;
}

/** Knob B: Pitch envelope decay duration (1.0ms .. 50.0ms noon .. 2500.0ms) */
inline float evalPitchDecaySec(float p) noexcept {
    return 0.001f * std::pow(2500.0f, std::clamp(p, 0.0f, 1.0f));
}

/** Knob C: Pitch envelope sweep depth (0.0st .. +24.0st noon .. +96.0st) */
inline constexpr float evalPitchDepthSemis(float p) noexcept {
    const float cp = std::clamp(p, 0.0f, 1.0f);
    return 96.0f * (cp * cp);
}

/** Knob D: Body decay multiplier (0.15x .. 1.00x noon .. 4.00x) */
inline constexpr float evalBodyDecayMult(float p) noexcept {
    const float cp = std::clamp(p, 0.0f, 1.0f);
    if (cp <= 0.5f) {
        return 0.15f + (1.0f - 0.15f) * (cp * 2.0f);
    } else {
        return 1.0f + 3.0f * ((cp - 0.5f) * 2.0f);
    }
}

/** Knob H: Punch gain multiplier (1.0x .. 1.75x noon .. 4.0x) */
inline constexpr float evalPunchGain(float p) noexcept {
    const float cp = std::clamp(p, 0.0f, 1.0f);
    return 1.0f + 3.0f * (cp * cp);
}

/** Cytomic SVF Resonance Q for LP/HP/BP (0.50 .. 3.54 noon .. 25.00) */
inline float evalCytomicQ(float p) noexcept {
    return 0.5f * std::pow(50.0f, std::clamp(p, 0.0f, 1.0f));
}

/** Tanh Pre-Filter Drive Decibels (0.0dB .. +6.0dB noon .. +36.0dB max) */
inline float evalDriveDb(float p) noexcept {
    return 36.0f * std::pow(std::clamp(p, 0.0f, 1.0f), 2.585f);
}

/** Tanh Pre-Filter Drive Linear Voltage Gain (1.00x .. 2.00x noon .. 63.10x max) */
inline float evalDriveGain(float p) noexcept {
    return std::pow(10.0f, evalDriveDb(p) / 20.0f);
}

/** Peaking Filter Bell EQ Gain in dB (-48.0dB .. 0.0dB noon .. +24.0dB) */
inline constexpr float evalPeakingGainDb(float p) noexcept {
    const float cp = std::clamp(p, 0.0f, 1.0f);
    if (cp <= 0.5f) {
        return -48.0f * (1.0f - 2.0f * cp);
    } else {
        return 24.0f * (2.0f * (cp - 0.5f));
    }
}

/** Role-Adaptive Wider Stereo Bass Cutoff Frequency (Hz) */
inline constexpr float getRoleWiderBassCutoffHz(int roleIdx) noexcept {
    switch (roleIdx) {
        case 0: return 300.0f; // BD: sub & punch 100% mono
        case 1: return 200.0f; // SN: body mono, snares wide
        case 2: return 220.0f; // TM: deep body mono
        case 3: return 250.0f; // CP: transient mono, clap spread wide
        case 4: return 500.0f; // HH: sizzle & metallic shimmer wide
        case 5: return 400.0f; // CY: panoramic crash/ride wash
        case 6: return 260.0f; // LN: clean 3D Latin percussion
        case 7: return 450.0f; // MC: metallic shimmer & high resonance wide
        case 8: return 180.0f; // FX: immersive low-end spatial FX & sweeps
        case 9: return 160.0f; // ST: massive orchestral stabs & stereo choir
        case 10: return 280.0f; // BS: solid sub-bass mono below 280Hz
        default: return 250.0f;
    }
}

// ==============================================================================
// MASTER VOLUME TAPER (-inf .. 0 dB (noon) .. +12 dB)
// ==============================================================================
inline constexpr float kMasterVolumeMaxDb   = 12.0f;
inline constexpr float kMasterVolumeMaxGain = 3.98107171f;  // 10^(12/20)

/** Master Volume taper: u in [0,1] -> linear gain. 0 = silence, 0.5 = unity (1.0), 1 = +12 dB. NaN -> silence. */
inline float evalMasterVolumeGain(float u) noexcept {
    if (!(u > 0.0f)) return 0.0f;
    const float cu = std::min(u, 1.0f);
    if (cu <= 0.5f) {
        const float t = 2.0f * cu;
        return t * t; // Quadratic fader taper (-inf .. 0 dB)
    }
    return std::pow(10.0f, kMasterVolumeMaxDb * (2.0f * cu - 1.0f) * (1.0f / 20.0f));
}

/** Master Volume taper in dB: -inf at u <= 0, 0.0 dB at u == 0.5, +12.0 dB at u == 1.0. */
inline float evalMasterVolumeDb(float u) noexcept {
    if (!(u > 0.0f)) return -std::numeric_limits<float>::infinity();
    const float cu = std::min(u, 1.0f);
    return (cu <= 0.5f) ? 40.0f * std::log10(2.0f * cu) : kMasterVolumeMaxDb * (2.0f * cu - 1.0f);
}

// ==============================================================================
// UNIFORM ROLE-MAXIMUM LOGARITHMIC AMP DECAY
// ==============================================================================
static constexpr float kRoleMinAmpDecaySec[8] = {
    0.030f, // BD
    0.025f, // SD
    0.040f, // TM
    0.030f, // CP
    0.015f, // HH
    0.080f, // CY
    0.020f, // PC
    0.030f  // FX
};

static constexpr float kRoleMaxAmpDecaySec[8] = {
    3.500f, // BD: up to 3.5s (deep sub boom)
    2.000f, // SD: up to 2.0s (gated reverb shell)
    3.000f, // TM: up to 3.0s (floor tom)
    1.500f, // CP: up to 1.5s (ambient room tail)
    1.200f, // HH: up to 1.2s (open sizzle wash)
    8.000f, // CY: up to 8.0s (infinite ride/crash)
    2.500f, // PC: up to 2.5s (ringing bell/conga)
    5.000f  // FX: up to 5.0s (cinematic riser)
};

/** Logarithmic mapping: u in [0, 1] -> [Tmin(role), Tmax(role)]. */
inline float evalRoleAmpDecaySec(float u, int roleIdx) noexcept {
    const int r = std::clamp(roleIdx, 0, 7);
    const float tMin = kRoleMinAmpDecaySec[r];
    const float tMax = kRoleMaxAmpDecaySec[r];
    const float cu = std::clamp(u, 0.0f, 1.0f);
    return tMin * std::pow(tMax / tMin, cu);
}

// ==============================================================================
// MULTIPLICATIVE LOG-DOMAIN PCHIP MACRO MULTIPLIER & DYNAMIC AMP DECAY TRACKING
// ==============================================================================

/** Evaluates smooth continuous macro multiplier curve [0.125x .. 1.0x noon .. 8.0x]. */
inline float evalMacroMultiplier(float macro) noexcept {
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

/**
 * @brief Evaluates the natural synthesis engine duration for the current role, model,
 * body decay, macro decay, and time macro multipliers.
 */
inline constexpr float evalEngineLengthSec(
    float baseAmpSec,
    float bodyDecayMult,
    float macroMult,
    float tMult
) noexcept {
    const float raw = baseAmpSec * bodyDecayMult * macroMult * tMult;
    return (raw < 0.005f) ? 0.005f : ((raw > 15.0f) ? 15.0f : raw);
}

/**
 * @brief Maps normalized Amp Decay knob [0..1] to [T_min, L_engine].
 * At u <= 0.001: returns bit-exact 5ms (ultra-tight transient snap).
 * At u >= 0.999: returns bit-exact L_engine (100% full natural synthesis tail).
 * Monotonic guarantee even if engineLengthSec < 0.005s.
 */
inline float evalDynamicAmpDecaySec(float u, float engineLengthSec, float tMinSec = 0.005f) noexcept {
    const float cu = std::clamp(u, 0.0f, 1.0f);
    const float safeL = std::clamp(engineLengthSec, tMinSec, 15.0f);
    if (cu <= 0.001f) return tMinSec;
    if (cu >= 0.999f) return safeL;

    constexpr float log2Min = -7.64385618977f; // fastLog2(0.005f) calibrated
    const float diffLog2 = std::max(0.0f, FastMath::fastLog2(safeL) - log2Min);
    return tMinSec * FastMath::fastPow2(cu * diffLog2);
}

// ==============================================================================
// TRIAD LIFEFORM CANON & NON-LINEAR MEMBRANE MATH
// ==============================================================================

/** Role-masked dynamic membrane pitch droop factors (Baby Audio Tekno / Chowdhury Bridged-T Law). */
static constexpr float kRoleDroopFactor[8] = {
    0.38f, // BD: Full acoustic drumhead tension droop (+38% at strike peak)
    0.12f, // SD: Subtle shell tension droop (+12% on body only)
    0.35f, // TM: Deep tom-tom head bloom (+35%)
    0.00f, // CP: Zero droop (preserves clapper transient timing)
    0.00f, // HH: Zero droop (preserves delicate 6kHz cluster ratios)
    0.00f, // CY: Zero droop (preserves bronze cymbal inharmonics)
    0.25f, // PC: Conga / Bongo membrane pitch droop (+25%)
    0.00f  // FX: Zero droop (preserves programmed pitch modulations)
};

/** Role-masked Organic LPG filter cutoff floor (minimum frequency ratio when optocoupler is fully dark). */
static constexpr float kRoleLpgCutoffFloor[8] = {
    0.15f, // BD: Full acoustic drumhead damping (15% floor for heavy sub-bass thud)
    0.20f, // SD: Deep shell damping (20% floor)
    0.15f, // TM: Deep tom-tom acoustic damping (15% floor)
    0.35f, // CP: Moderate clap transient air retention (35% floor)
    0.65f, // HH: Hi-Hat metallic air defense (65% floor preserves crisp 6-12kHz sizzle)
    0.60f, // CY: Cymbal bronze shimmer defense (60% floor preserves shimmering alloy wash)
    0.25f, // PC: Conga/bongo acoustic membrane damping (25% floor)
    0.40f  // FX: Moderate spectral decay (40% floor)
};

/**
 * @brief Antiderivative Anti-Aliasing (ADAA1) Numerical Utilities for Formless Spawn.
 * SSoT: Julian Parker / Stefan Bilbao continuous piecewise antiderivative evaluation.
 */
struct ADAA {
    // 1st-Order Antiderivative Anti-Aliasing for tanh(g * x) with cached state (Win #1)
    [[nodiscard]] static inline float tanh(float x, float& x1, float& f1, float g = 1.0f) noexcept {
        const float diff = x - x1;
        float y = 0.0f;
        auto F = [g](float v) noexcept {
            const float gv = std::abs(g * v);
            if (gv > 16.0f) return (gv - 0.69314718056f) / g;
            return (gv + std::log1p(std::exp(-2.0f * gv)) - 0.69314718056f) / g;
        };
        const float fx = F(x);
        if (std::abs(diff) > 1e-5f) {
            y = (fx - f1) / diff;
        } else {
            y = FastMath::fastTanh(g * 0.5f * (x + x1));
        }
        x1 = x;
        f1 = fx;
        return y;
    }

    [[nodiscard]] static inline float tanh(float x, float& x1, float g = 1.0f) noexcept {
        auto F = [g](float v) noexcept {
            const float gv = std::abs(g * v);
            if (gv > 16.0f) return (gv - 0.69314718056f) / g;
            return (gv + std::log1p(std::exp(-2.0f * gv)) - 0.69314718056f) / g;
        };
        float f1 = F(x1);
        return tanh(x, x1, f1, g);
    }

    // 1st-Order Antiderivative Anti-Aliasing for overdriven diode: w = 2.2*x + 0.45*x*|x| -> tanh(w)
    [[nodiscard]] static inline float diode(float x, float& x1, float& f1) noexcept {
        const float w = 2.2f * x + 0.45f * x * std::abs(x);
        return tanh(w, x1, f1, 1.0f);
    }

    [[nodiscard]] static inline float diode(float x, float& x1) noexcept {
        const float w = 2.2f * x + 0.45f * x * std::abs(x);
        return tanh(w, x1, 1.0f);
    }

    // Piecewise ADAA1 for foldback with cached state (Win #1)
    [[nodiscard]] static inline float fold(float x, float& x1, float& f1, float thresh = 0.85f, float scale = 0.70f) noexcept {
        const float diff = x - x1;
        auto F = [thresh, scale](float v) noexcept {
            const float av = std::abs(v);
            if (av <= thresh) {
                return 0.5f * v * v;
            }
            const float excess = av - thresh;
            const float intFold = (1.0f + scale) * thresh * excess - 0.5f * scale * excess * excess;
            return 0.5f * thresh * thresh + intFold;
        };
        const float sgnX = (x >= 0.0f) ? 1.0f : -1.0f;
        const float sgnFx = sgnX * F(x);
        float y = 0.0f;
        if (std::abs(diff) > 1e-5f) {
            y = (sgnFx - f1) / diff;
        } else {
            const float mid = 0.5f * (x + x1);
            if (mid > thresh) y = thresh - (mid - thresh) * scale;
            else if (mid < -thresh) y = -thresh - (mid + thresh) * scale;
            else y = mid;
        }
        x1 = x;
        f1 = sgnFx;
        return y;
    }

    [[nodiscard]] static inline float fold(float x, float& x1, float thresh = 0.85f, float scale = 0.70f) noexcept {
        auto F = [thresh, scale](float v) noexcept {
            const float av = std::abs(v);
            if (av <= thresh) return 0.5f * v * v;
            const float excess = av - thresh;
            const float intFold = (1.0f + scale) * thresh * excess - 0.5f * scale * excess * excess;
            return 0.5f * thresh * thresh + intFold;
        };
        const float sgnX1 = (x1 >= 0.0f) ? 1.0f : -1.0f;
        float f1 = sgnX1 * F(x1);
        return fold(x, x1, f1, thresh, scale);
    }
};

} // namespace tkdsp::formless
