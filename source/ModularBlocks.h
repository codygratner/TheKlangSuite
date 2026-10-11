#pragma once
#include "DSPBlock.h"
#include "FastMath.h"
#include "ModulationEngine.h"
#include <cmath>
#include <vector>
#include <memory>
#include <algorithm>
#include <atomic>

namespace TbdAudio {

// --- HELPER MATH FUNCTIONS ---

inline float fastRng(uint32_t& state) {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return static_cast<float>(static_cast<int32_t>(state)) * (1.0f / 2147483648.0f);
}

// Continuous waveform crossfade:
// 0%: Sine -> 20%: Tri -> 40%: Saw -> 60%: Square -> 100%: PWM 0%
inline float evaluateWaveform(float phase, float shape) {
    float p = phase - std::floor(phase);
    float s = FastMath::fastSinNorm(p);
    float tri = (p < 0.5f) ? (4.0f * p - 1.0f) : (3.0f - 4.0f * p);
    float saw = 2.0f * p - 1.0f;
    float sq = (p < 0.5f) ? 1.0f : -1.0f;

    if (shape <= 0.20f) {
        float t = shape / 0.20f;
        return (1.0f - t) * s + t * tri;
    } else if (shape <= 0.40f) {
        float t = (shape - 0.20f) / 0.20f;
        return (1.0f - t) * tri + t * saw;
    } else if (shape <= 0.60f) {
        float t = (shape - 0.40f) / 0.20f;
        return (1.0f - t) * saw + t * sq;
    } else {
        // 60% to 100%: PWM from 50% down to 0% duty cycle with zero-mean DC normalization
        float t = (shape - 0.60f) / 0.40f;
        float duty = std::clamp(0.5f * (1.0f - t), 0.005f, 0.50f);
        float pulse = (p < duty) ? (1.0f - duty) : -duty;
        return pulse * 2.0f;
    }
}

// Envelope slope shaper:
// Linear is at shape = 0.75 (deadzone [0.735, 0.765]).
// shape > 0.765 -> logarithmic curve (same exponent range as original: power 1.0 down to 1/3.94)
// shape < 0.735 -> exponential curve (4x steeper: power 1.0 up to 15.76 at shape = 0.0)
inline float applyEnvelopeSlope(float linearVal, float shape) {
    linearVal = std::clamp(linearVal, 0.0f, 1.0f);
    if (shape < 0.735f) {
        float norm = (0.735f - shape) / 0.735f; // 0.0 at deadzone boundary, 1.0 at shape = 0.0
        float p = 1.0f + norm * 14.76f;         // reaches 15.76f (4x original steepness: 1 + 6*0.49 = 3.94; 3.94*4 = 15.76)
        return std::pow(linearVal, p);
    } else if (shape > 0.765f) {
        float norm = (shape - 0.765f) / 0.235f; // 0.0 at deadzone boundary, 1.0 at shape = 1.0
        float p = 1.0f / (1.0f + norm * 2.94f); // reaches 1 / 3.94f (matches original logarithmic curve)
        return std::pow(linearVal, p);
    }
    return linearVal;
}

// Slop & Velocity Exponential Warp Functions:
// Unipolar: 50% controller travel (0.5) gives 10% value (0.10)
// Power: p = log2(10) ≈ 3.321928f. Inverse: 1/p = log10(2) ≈ 0.301030f.
constexpr float UNIPOLAR_EXP_POWER = 3.321928094887362f; // log2(10)
constexpr float UNIPOLAR_EXP_INV   = 0.301029995663981f; // log10(2)

// Bipolar: ±25% controller displacement from center (c = 0.75 or 0.25) gives ±5% value (±0.05)
// Power: q = log2(20) ≈ 4.321928f. Inverse: 1/q = log20(2) ≈ 0.231378f.
constexpr float BIPOLAR_EXP_POWER  = 4.321928094887362f; // log2(20)
constexpr float BIPOLAR_EXP_INV    = 0.231378213159759f; // log20(2)

inline float warpUnipolarExp(float c) {
    c = std::clamp(c, 0.0f, 1.0f);
    if (c <= 0.0f) return 0.0f;
    if (c >= 1.0f) return 1.0f;
    return std::pow(c, UNIPOLAR_EXP_POWER);
}

inline float unwarpUnipolarExp(float y) {
    y = std::clamp(y, 0.0f, 1.0f);
    if (y <= 0.0f) return 0.0f;
    if (y >= 1.0f) return 1.0f;
    return std::pow(y, UNIPOLAR_EXP_INV);
}

inline float warpBipolarExp(float c) {
    c = std::clamp(c, 0.0f, 1.0f);
    float delta = c - 0.5f;
    if (std::abs(delta) < 1e-6f) return 0.0f;
    float sign = (delta >= 0.0f) ? 1.0f : -1.0f;
    float u = std::abs(delta) * 2.0f;
    if (u >= 1.0f) return sign * 1.0f;
    return sign * std::pow(u, BIPOLAR_EXP_POWER);
}

inline float unwarpBipolarExp(float y) {
    y = std::clamp(y, -1.0f, 1.0f);
    if (std::abs(y) < 1e-6f) return 0.5f;
    float sign = (y >= 0.0f) ? 1.0f : -1.0f;
    float mag = std::abs(y);
    if (mag >= 1.0f) return 0.5f + sign * 0.5f;
    float u = std::pow(mag, BIPOLAR_EXP_INV);
    return std::clamp(0.5f + sign * u * 0.5f, 0.0f, 1.0f);
}

// --- VELOCITY RESPONSE CURVES ---
enum class VelocityCurve : int {
    Linear = 0,
    Exponential,
    Logarithmic,
    Fixed127
};

inline const char* getVelocityCurveName(VelocityCurve curve) {
    switch (curve) {
        case VelocityCurve::Linear:      return "Linear";
        case VelocityCurve::Exponential: return "Exponential";
        case VelocityCurve::Logarithmic: return "Logarithmic";
        case VelocityCurve::Fixed127:    return "Fixed 127";
        default:                         return "Linear";
    }
}

inline float evaluateVelocityCurve(float v, VelocityCurve curve) {
    v = std::clamp(v, 0.0f, 1.0f);
    switch (curve) {
        case VelocityCurve::Linear:
            return v;
        case VelocityCurve::Exponential:
            return warpUnipolarExp(v);
        case VelocityCurve::Logarithmic:
            return std::sqrt(v);
        case VelocityCurve::Fixed127:
            return 1.0f;
        default:
            return v;
    }
}

struct VelocityTracker {
    VelocityCurve curveType { VelocityCurve::Linear };
    float lastRawVelocity { 1.0f };
    float lastCurvedVelocity { 1.0f };

    float evaluate(float rawVelocity) {
        rawVelocity = std::clamp(rawVelocity, 0.0f, 1.0f);
        lastRawVelocity = rawVelocity;
        lastCurvedVelocity = evaluateVelocityCurve(rawVelocity, curveType);
        return lastCurvedVelocity;
    }

    void setCurve(VelocityCurve curve) { curveType = curve; }
    VelocityCurve getCurve() const { return curveType; }
};

// Drive mapping: -6dB to +24dB (0dB at 0.2, +6dB at 0.4)
inline float normToDriveDb(float norm) {
    norm = std::clamp(norm, 0.0f, 1.0f);
    return -6.0f + norm * 30.0f;
}

inline float driveDbToNorm(float db) {
    return std::clamp((db + 6.0f) / 30.0f, 0.0f, 1.0f);
}

inline float normToDriveGain(float norm) {
    return FastMath::fastDbToGain(normToDriveDb(norm));
}

// Frequency Shifter Range mapping: 0 Hz to 5 kHz (cubic curve for fine sub-Hz to multi-kHz control)
inline float normToRangeHz(float norm) {
    norm = std::clamp(norm, 0.0f, 1.0f);
    return 5000.0f * norm * norm * norm;
}

inline float rangeHzToNorm(float hz) {
    return std::clamp(std::cbrt(std::clamp(hz, 0.0f, 5000.0f) / 5000.0f), 0.0f, 1.0f);
}


// 5-Point Warp Decay Time:
// 0%: 5 ms, 25%: 100 ms, 50%: 1 s, 75%: 5 s, 100%: 60 s
inline float warp5PointTime(float u) {
    u = std::clamp(u, 0.0f, 1.0f);
    if (u <= 0.25f) {
        float t = u / 0.25f;
        return 0.005f * std::pow(0.100f / 0.005f, t);
    } else if (u <= 0.50f) {
        float t = (u - 0.25f) / 0.25f;
        return 0.100f * std::pow(1.0f / 0.100f, t);
    } else if (u <= 0.75f) {
        float t = (u - 0.50f) / 0.25f;
        return 1.0f * std::pow(5.0f / 1.0f, t);
    } else {
        float t = (u - 0.75f) / 0.25f;
        return 5.0f * std::pow(60.0f / 5.0f, t);
    }
}

inline float unwarp5PointTime(float seconds) {
    if (!std::isfinite(seconds) || seconds <= 0.005f) return 0.0f;
    if (seconds >= 60.0f) return 1.0f;
    if (seconds <= 0.100f) {
        float t = std::log(seconds / 0.005f) / std::log(0.100f / 0.005f);
        return std::clamp(t * 0.25f, 0.0f, 0.25f);
    } else if (seconds <= 1.0f) {
        float t = std::log(seconds / 0.100f) / std::log(1.0f / 0.100f);
        return std::clamp(0.25f + t * 0.25f, 0.25f, 0.50f);
    } else if (seconds <= 5.0f) {
        float t = std::log(seconds / 1.0f) / std::log(5.0f / 1.0f);
        return std::clamp(0.50f + t * 0.25f, 0.50f, 0.75f);
    } else {
        float t = std::log(seconds / 5.0f) / std::log(60.0f / 5.0f);
        return std::clamp(0.75f + t * 0.25f, 0.75f, 1.0f);
    }
}

// Noise Transient 5-Point Warp Decay Time:
// 0%: 1 ms, 25%: 50 ms, 50%: 1 s, 75%: 5 s, 100%: 60 s
inline float warpNoiseDecayTime(float u) {
    u = std::clamp(u, 0.0f, 1.0f);
    if (u <= 0.25f) {
        float t = u / 0.25f;
        return 0.001f * std::pow(0.050f / 0.001f, t);
    } else if (u <= 0.50f) {
        float t = (u - 0.25f) / 0.25f;
        return 0.050f * std::pow(1.0f / 0.050f, t);
    } else if (u <= 0.75f) {
        float t = (u - 0.50f) / 0.25f;
        return 1.0f * std::pow(5.0f / 1.0f, t);
    } else {
        float t = (u - 0.75f) / 0.25f;
        return 5.0f * std::pow(60.0f / 5.0f, t);
    }
}

inline float unwarpNoiseDecayTime(float seconds) {
    if (!std::isfinite(seconds) || seconds <= 0.001f) return 0.0f;
    if (seconds >= 60.0f) return 1.0f;
    if (seconds <= 0.050f) {
        float t = std::log(seconds / 0.001f) / std::log(0.050f / 0.001f);
        return std::clamp(t * 0.25f, 0.0f, 0.25f);
    } else if (seconds <= 1.0f) {
        float t = std::log(seconds / 0.050f) / std::log(1.0f / 0.050f);
        return std::clamp(0.25f + t * 0.25f, 0.25f, 0.50f);
    } else if (seconds <= 5.0f) {
        float t = std::log(seconds / 1.0f) / std::log(5.0f / 1.0f);
        return std::clamp(0.50f + t * 0.25f, 0.50f, 0.75f);
    } else {
        float t = std::log(seconds / 5.0f) / std::log(60.0f / 5.0f);
        return std::clamp(0.75f + t * 0.25f, 0.75f, 1.0f);
    }
}

// DJ Style Filter: 0..49% LPF (20Hz-20kHz), 50% Flat, 51..100% HPF (20Hz-20kHz)
struct DJFilter {
    float s1L = 0.0f, s2L = 0.0f;
    float s1R = 0.0f, s2R = 0.0f;

    void reset() {
        s1L = s2L = s1R = s2R = 0.0f;
    }

    void process(float& left, float& right, float knob, float sampleRate) {
        if (knob >= 0.49f && knob <= 0.51f) {
            return; // flat / bypass
        }

        bool isLowpass = (knob < 0.49f);
        float norm = isLowpass ? (knob / 0.49f) : ((knob - 0.51f) / 0.49f);
        float cutoff = 20.0f * std::pow(24000.0f / 20.0f, std::clamp(norm, 0.0f, 1.0f));
        float q = isLowpass ? (1.1f - norm * 0.393f) : (0.707f + norm * 0.4f);

        cutoff = std::clamp(cutoff, 20.0f, sampleRate * 0.485f);
        float g = std::tan(PI * cutoff / sampleRate);
        float k = 1.0f / q;
        float a1 = 1.0f / (1.0f + g * (g + k));

        // Left channel SVF
        float hpL = (left - (g + k) * s1L - s2L) * a1;
        float bpL = g * hpL + s1L;
        s1L = g * hpL + bpL;
        float lpL = g * bpL + s2L;
        s2L = g * bpL + lpL;
        left = isLowpass ? lpL : hpL;

        // Right channel SVF
        float hpR = (right - (g + k) * s1R - s2R) * a1;
        float bpR = g * hpR + s1R;
        s1R = g * hpR + bpR;
        float lpR = g * bpR + s2R;
        s2R = g * bpR + lpR;
        right = isLowpass ? lpR : hpR;
    }
};

// --- VISUAL OSCILLOSCOPE BUFFER ---
struct VisualScope {
    static constexpr int RING_SIZE = 4096;
    std::vector<float> buffer;
    std::atomic<int> writeIndex{ 0 };

    VisualScope() : buffer(RING_SIZE, 0.0f) {}
    VisualScope(const VisualScope&) = delete;
    VisualScope& operator=(const VisualScope&) = delete;
    VisualScope(VisualScope&& other) noexcept : buffer(std::move(other.buffer)), writeIndex(other.writeIndex.load()) {}
    VisualScope& operator=(VisualScope&& other) noexcept {
        buffer = std::move(other.buffer);
        writeIndex.store(other.writeIndex.load());
        return *this;
    }

    void pushBlock(const float* data, int numSamples) {
        if (!data || numSamples <= 0) return;
        int idx = writeIndex.load(std::memory_order_relaxed);
        for (int i = 0; i < numSamples; ++i) {
            buffer[(idx + i) & (RING_SIZE - 1)] = data[i];
        }
        writeIndex.store((idx + numSamples) & (RING_SIZE - 1), std::memory_order_release);
    }

    void readLatest(float* outBuffer, int count) const {
        if (!outBuffer || count <= 0) return;
        int idx = writeIndex.load(std::memory_order_acquire);
        for (int i = 0; i < count; ++i) {
            int readPos = (idx - count + i + RING_SIZE * 4) & (RING_SIZE - 1);
            outBuffer[i] = buffer[readPos];
        }
    }

    void clear() {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
        writeIndex.store(0, std::memory_order_release);
    }

    void readTriggered(float* outBuffer, int count, int triggerPos, float step) const {
        if (!outBuffer || count <= 0) return;
        for (int i = 0; i < count; ++i) {
            float samplePos = static_cast<float>(triggerPos) + static_cast<float>(i) * step;
            int idx0 = static_cast<int>(std::floor(samplePos));
            float frac = samplePos - static_cast<float>(idx0);

            float s0 = buffer[idx0 & (RING_SIZE - 1)];
            float s1 = buffer[(idx0 + 1) & (RING_SIZE - 1)];
            float sample = s0 + frac * (s1 - s0);
            if (!std::isfinite(sample)) sample = 0.0f;
            outBuffer[i] = sample;
        }
    }
};

// --- BLOCK 1 & 4: CARRIER ---
class CarrierBlock : public DSPBlock {
public:
    explicit CarrierBlock(int voice = 1) : voiceIndex(voice) {}

    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        sampleRate = ctx.sampleRate;
        phase = 0.0f;
    }

    void trigger(float) override {
        phase = 0.0f;
    }

    void reset() override {
        phase = 0.0f;
    }

    float getBasePitch(const BlockContext& ctx) const {
        int style = std::clamp(static_cast<int>(std::round(params[0] * 2.0f)), 0, 2);
        float slop = (voiceIndex == 1) ? ctx.slopCarrier1Pitch : ctx.slopCarrier2Pitch;
        float pitchParam = std::clamp(params[1] + slop, 0.0f, 1.0f);
        if (style == 0) {
            // MIDI pitch: offset from -24 to +24 semitones (def 0 = 0.5f)
            float offset = std::round((pitchParam - 0.5f) * 48.0f);
            return ctx.currentPitchHz * FastMath::fastPow2(offset / 12.0f);
        } else if (style == 1) {
            // Fixed freq: 20 Hz to 24 kHz (def 55 Hz)
            return 20.0f * std::pow(24000.0f / 20.0f, pitchParam);
        } else {
            // Fixed note: MIDI note 0 to 127 (def A1 33 = 55 Hz)
            float note = std::round(pitchParam * 127.0f);
            return 440.0f * FastMath::fastPow2((note - 69.0f) / 12.0f);
        }
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& ctx) override {
        float baseFreq = getBasePitch(ctx);
        float shape = params[2];
        float modDepth = (params[3] - 0.5f) * 2.0f; // -100% to +100% (default 0% = 0.5)

        int target = (voiceIndex == 1) ? ctx.pitchEnv1Target : ctx.pitchEnv2Target;
        bool applyPitchEnv = (target == 0 || target == 2 || target == 3);
        const auto& modSig = (voiceIndex == 1) ? ctx.mod1Signal : ctx.mod2Signal;
        const auto& peSig  = (voiceIndex == 1) ? ctx.pitchEnv1Signal : ctx.pitchEnv2Signal;

        for (int i = 0; i < numSamples; ++i) {
            float fmMod = (i < static_cast<int>(modSig.size())) ? modSig[i] : 0.0f;
            float pitchEnv = (applyPitchEnv && i < static_cast<int>(peSig.size())) ? peSig[i] : 0.0f;

            // Pitch Envelope modulates carrier pitch: 5 octaves sweep up/down
            // Modulator frequency modulates carrier scaled by modDepth (-200% to +200%)
            float instFreq = baseFreq * FastMath::fastPow2(pitchEnv * 5.0f + fmMod * modDepth * 4.0f);
            instFreq = std::clamp(instFreq, 1.0f, sampleRate * 0.48f);

            phase += instFreq * invSr;
            if (phase >= 1.0f) phase -= std::floor(phase);

            float oscOut = evaluateWaveform(phase, shape);
            currentFreq = instFreq;

            if (left) left[i] = oscOut;
            if (right) right[i] = oscOut;
        }
    }

    float getCurrentFreq() const { return currentFreq; }

private:
    int voiceIndex = 1;
    float invSr = 1.0f / 44100.0f;
    float sampleRate = 44100.0f;
    float phase = 0.0f;
    float currentFreq = 55.0f;
};

// --- BLOCK 2 & 5: MODULATOR ---
class ModulatorBlock : public DSPBlock {
public:
    explicit ModulatorBlock(int voice = 1) : voiceIndex(voice) {}

    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        sampleRate = ctx.sampleRate;
        phase = 0.0f;
        noisePhase = 0.0f;
        noiseVal = 0.0f;
        djFilter.reset();
    }

    void trigger(float) override {
        phase = 0.0f;
        noisePhase = 0.0f;
    }

    void reset() override {
        phase = 0.0f;
        noisePhase = 0.0f;
        noiseVal = 0.0f;
        djFilter.reset();
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* /*left*/, float* /*right*/, int numSamples, BlockContext& ctx) override {
        int tracking = std::clamp(static_cast<int>(std::round(params[0] * 2.0f)), 0, 2); // 0=Fixed, 1=Following, 2=FM Operator
        int type = std::clamp(static_cast<int>(std::round(params[1] * 2.0f)), 0, 2);     // 0=Oscillator, 1=Cyclic, 2=Noise
        float slopFilt = (voiceIndex == 1) ? ctx.slopMod1Filter : ctx.slopMod2Filter;
        float slopFrq  = (voiceIndex == 1) ? ctx.slopMod1Freq : ctx.slopMod2Freq;
        float shape = std::clamp(params[2] + slopFilt, 0.0f, 1.0f);
        float speed = std::clamp(params[3] + slopFrq, 0.0f, 1.0f);
        float carrierPitch = std::max((voiceIndex == 1) ? ctx.carrier1PitchHz : ctx.carrier2PitchHz, 10.0f);
        float oscFreq = 55.0f;
        float shRate = 24000.0f;

        if (type == 2) {
            // Noise: Speed (params[3]) controls S&H noise rate: 0.1 Hz to 24 kHz (def 24 kHz)
            shRate = 0.1f * std::pow(24000.0f / 0.1f, speed);
        } else {
            // Oscillator (0) or Cyclic (1)
            if (tracking == 0) {
                // Fixed:
                // Cyclic default sine freq 24 kHz; Osc default 55 Hz
                oscFreq = 0.1f * std::pow(24000.0f / 0.1f, speed);
            } else if (tracking == 1) {
                // Following offset: -64 to +64 semitones (def 0)
                float noteOffset = (speed - 0.5f) * 128.0f;
                oscFreq = carrierPitch * FastMath::fastPow2(noteOffset / 12.0f);
            } else {
                // FM ratio: 1:32 to 1:1 to 32:1 (def 1:1)
                float ratio = (speed <= 0.5f) ? (1.0f / (32.0f - (speed * 2.0f) * 31.0f))
                                              : (1.0f + ((speed - 0.5f) * 2.0f) * 31.0f);
                oscFreq = carrierPitch * ratio;
            }
        }

        oscFreq = std::clamp(oscFreq, 0.05f, sampleRate * 0.485f);

        auto& modSig = (voiceIndex == 1) ? ctx.mod1Signal : ctx.mod2Signal;
        const auto& peSig = (voiceIndex == 1) ? ctx.pitchEnv1Signal : ctx.pitchEnv2Signal;
        int target = (voiceIndex == 1) ? ctx.pitchEnv1Target : ctx.pitchEnv2Target;
        bool applyPitchEnv = (target == 1 || target == 2 || target == 3);
        float pitchEnvSign = (target == 3) ? -1.0f : 1.0f;

        int count = std::min(numSamples, static_cast<int>(modSig.size()));
        for (int i = 0; i < count; ++i) {
            float pitchEnv = (applyPitchEnv && i < static_cast<int>(peSig.size())) ? (peSig[i] * pitchEnvSign) : 0.0f;
            float instFreq = oscFreq * FastMath::fastPow2(pitchEnv * 5.0f);
            instFreq = std::clamp(instFreq, 0.05f, sampleRate * 0.48f);

            phase += instFreq * invSr;
            if (phase >= 1.0f) phase -= std::floor(phase);

            float val = 0.0f;
            if (type == 0) {
                // Oscillator: Waveform morph
                val = evaluateWaveform(phase, shape);
            } else if (type == 1) {
                // Cyclic: Sine * Noise ring-mod with white noise DJ filter
                float sinVal = FastMath::fastSinNorm(phase);
                float rawNoise = fastRng(rngState);
                float dummyR = rawNoise;
                djFilter.process(rawNoise, dummyR, shape, sampleRate);
                val = sinVal * rawNoise;
            } else {
                // Noise: S&H Noise clocked at shRate modulated by pitch envelope
                float instShRate = shRate * FastMath::fastPow2(pitchEnv * 5.0f);
                instShRate = std::clamp(instShRate, 0.05f, sampleRate * 0.48f);
                noisePhase += instShRate * invSr;
                if (noisePhase >= 1.0f) {
                    noisePhase -= std::floor(noisePhase);
                    noiseVal = fastRng(rngState);
                }
                float filteredNoise = noiseVal;
                float dummyR = filteredNoise;
                djFilter.process(filteredNoise, dummyR, shape, sampleRate);
                val = filteredNoise;
            }

            modSig[i] = val;
        }

        currentFreq = (type == 2) ? shRate : oscFreq;
    }

    float getCurrentFreq() const { return currentFreq; }

private:
    int voiceIndex = 1;
    float invSr = 1.0f / 44100.0f;
    float sampleRate = 44100.0f;
    float phase = 0.0f;
    float noisePhase = 0.0f;
    float noiseVal = 0.0f;
    float currentFreq = 55.0f;
    DJFilter djFilter;
    uint32_t rngState = 0x98765432;
};

// --- BLOCK 3 & 6: PITCH ENVELOPE (Slope, Depth, Decay, Target) ---
class PitchEnvelopeBlock : public DSPBlock {
public:
    explicit PitchEnvelopeBlock(int voice = 1) : voiceIndex(voice) {}

    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        timeSinceTrigger = 1000.0f;
    }

    void trigger(float) override {
        timeSinceTrigger = 0.0f;
        float slope = params[0];
        float baseDepth = (params[1] - 0.5f) * 2.0f;
        currentVal = applyEnvelopeSlope(1.0f, slope) * baseDepth;
    }

    void reset() override {
        timeSinceTrigger = 1000.0f;
        currentVal = 0.0f;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* /*left*/, float* /*right*/, int numSamples, BlockContext& ctx) override {
        float slope = params[0];

        float slopDepth = (voiceIndex == 1) ? ctx.slopPitchEnv1Depth : ctx.slopPitchEnv2Depth;
        float effDepthParam = std::clamp(params[1] + slopDepth, 0.0f, 1.0f);
        float baseDepth = (effDepthParam - 0.5f) * 2.0f;
        float depth = std::clamp(baseDepth + ctx.velDepthMod + ctx.keyDepthMod, -1.0f, 1.0f);

        float slopDecay = (voiceIndex == 1) ? ctx.slopPitchEnv1Decay : ctx.slopPitchEnv2Decay;
        float decayParam = std::clamp(params[2] + slopDecay + ctx.velDecayMod + ctx.keyDecayMod, 0.0f, 1.0f);
        float decayTime = warp5PointTime(decayParam);
        decayTime = std::max(decayTime, 0.001f);

        int target = std::clamp(static_cast<int>(std::round(params[3] * 3.0f)), 0, 3);
        if (voiceIndex == 1) ctx.pitchEnv1Target = target;
        else ctx.pitchEnv2Target = target;

        auto& peSig = (voiceIndex == 1) ? ctx.pitchEnv1Signal : ctx.pitchEnv2Signal;

        int count = std::min(numSamples, static_cast<int>(peSig.size()));
        for (int i = 0; i < count; ++i) {
            float linearProgress = timeSinceTrigger / decayTime;
            float envLinear = std::clamp(1.0f - linearProgress, 0.0f, 1.0f);
            float envVal = applyEnvelopeSlope(envLinear, slope) * depth;
            timeSinceTrigger += invSr;
            peSig[i] = envVal;
            currentVal = envVal;
        }
    }

    int getTarget() const {
        return std::clamp(static_cast<int>(std::round(params[3] * 3.0f)), 0, 3);
    }

    float getCurrentValue() const { return currentVal; }

private:
    int voiceIndex = 1;
    float invSr = 1.0f / 44100.0f;
    float timeSinceTrigger = 1000.0f;
    float currentVal = 0.0f;
};

// --- BLOCK: DRIVE (Drive, Bias, Post-Filter, Limiter) ---
class DriveBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        sampleRate = ctx.sampleRate;
        djFilter.reset();
    }

    void reset() override {
        djFilter.reset();
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& ctx) override {
        // 1. Drive: -6dB to +24dB (0dB at 0.2, +6dB def at 0.4)
        float satGain = normToDriveGain(params[0]);

        // 2. Bias: DC offset -1 to +1 (def: 0 = 0.5)
        float bias = (params[1] - 0.5f) * 2.0f;

        // 3. Post-Filter: DJ style filter (def: 50% flat = 0.5)
        float filterKnob = std::clamp(params[2] + ctx.slopDriveFilter, 0.0f, 1.0f);

        // 4. Limiter: Off (0), On (1, def) - comes after drive and filter
        bool hasLimiter = (params[3] >= 0.5f);

        for (int i = 0; i < numSamples; ++i) {
            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            inL += bias;
            inR += bias;

            inL = FastMath::fastTanh(inL * satGain);
            inR = FastMath::fastTanh(inR * satGain);

            inL -= bias * 0.5f;
            inR -= bias * 0.5f;

            // DJ filter before limiter
            djFilter.process(inL, inR, filterKnob, sampleRate);

            // Limiter after drive and filter
            if (hasLimiter) {
                inL = FastMath::fastTanh(inL);
                inR = FastMath::fastTanh(inR);
            }

            if (left) left[i] = inL;
            if (right) right[i] = inR;
        }
    }

private:
    float sampleRate = 44100.0f;
    DJFilter djFilter;
};

// --- BLOCK 7: NOISE TRANSIENT ---
class NoiseTransientBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        sampleRate = ctx.sampleRate;
        timeSinceTrigger = 1000.0f;
        shPhase = 0.0f;
        shVal = 0.0f;
        djFilter.reset();
    }

    void trigger(float) override {
        timeSinceTrigger = 0.0f;
        shPhase = 0.0f;
    }

    void reset() override {
        timeSinceTrigger = 1000.0f;
        shPhase = 0.0f;
        shVal = 0.0f;
        djFilter.reset();
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& ctx) override {
        // 1. S&H rate: 0.1 Hz to 24 kHz (def 24 kHz)
        float shParam = std::clamp(params[0] + ctx.slopNoiseShRate, 0.0f, 1.0f);
        float shRate = 0.1f * std::pow(24000.0f / 0.1f, shParam);

        // 2. DJ filter knob (def 50% Flat)
        float filterKnob = std::clamp(params[1] + ctx.slopNoiseFilter, 0.0f, 1.0f);

        // 3. Drive: -6dB to 0dB (at 0.5) to +24dB (def: 0dB)
        float gain = normToDriveGain(params[2]);

        // 4. Decay: 5-point warp (1ms, 50ms, 1s, 5s, 60s; def 100ms) + slop + velocity modulation
        float decayParam = std::clamp(params[3] + ctx.slopNoiseDecay + ctx.velDecayMod, 0.0f, 1.0f);
        float decayTime = warpNoiseDecayTime(decayParam);
        decayTime = std::max(decayTime, 0.0005f);

        for (int i = 0; i < numSamples; ++i) {
            float noiseOut = 0.0f;

            shPhase += shRate * invSr;
            if (shPhase >= 1.0f) {
                shPhase -= 1.0f;
                shVal = fastRng(rngState);
            }

            float env = FastMath::fastExp(-timeSinceTrigger / decayTime);
            timeSinceTrigger += invSr;

            float rawNoise = shVal * env * gain;
            noiseOut = (gain > 1.0f) ? FastMath::fastTanh(rawNoise) : rawNoise;

            float outL = noiseOut;
            float outR = noiseOut;
            djFilter.process(outL, outR, filterKnob, sampleRate);

            if (left) left[i] = outL;
            if (right) right[i] = outR;
        }
    }

private:
    float invSr = 1.0f / 44100.0f;
    float sampleRate = 44100.0f;
    float timeSinceTrigger = 1000.0f;
    float shPhase = 0.0f;
    float shVal = 0.0f;
    DJFilter djFilter;
    uint32_t rngState = 0x55AA55AA;
};

// --- BLOCK 8: MIXER (Carrier 1, Carrier 2, RingMod, Noise) ---
class MixerBlock : public DSPBlock {
public:
    void init(const BlockContext& /*ctx*/) override {}

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& /*ctx*/) override {
        // 1. Carrier 1 level: 0% to 100% (at 0.5) to 400% (at 1.0)
        float c1Gain = (params[0] <= 0.5f) ? (params[0] * 2.0f) : (1.0f + (params[0] - 0.5f) * 6.0f);
        // 2. Carrier 2 level: 0% to 100% (at 0.5) to 400% (at 1.0)
        float c2Gain = (params[1] <= 0.5f) ? (params[1] * 2.0f) : (1.0f + (params[1] - 0.5f) * 6.0f);
        // 3. Ring Mod level: 0% to 100% (at 0.5) to 400% (at 1.0) - carrier 1 * carrier 2
        float rmGain = (params[2] <= 0.5f) ? (params[2] * 2.0f) : (1.0f + (params[2] - 0.5f) * 6.0f);
        // 4. Noise level: 0% to 100% (at 0.5) to 400% (at 1.0)
        float noiseGain = (params[3] <= 0.5f) ? (params[3] * 2.0f) : (1.0f + (params[3] - 0.5f) * 6.0f);

        for (int i = 0; i < numSamples; ++i) {
            float c1L = (tempC1L && i < sampleCount) ? tempC1L[i] : 0.0f;
            float c1R = (tempC1R && i < sampleCount) ? tempC1R[i] : c1L;
            float c2L = (tempC2L && i < sampleCount) ? tempC2L[i] : 0.0f;
            float c2R = (tempC2R && i < sampleCount) ? tempC2R[i] : c2L;
            float nL  = (tempNoiseBufL && i < sampleCount) ? tempNoiseBufL[i] : 0.0f;
            float nR  = (tempNoiseBufR && i < sampleCount) ? tempNoiseBufR[i] : nL;

            float rmL = c1L * c2L;
            float rmR = c1R * c2R;

            float outL = c1L * c1Gain + c2L * c2Gain + rmL * rmGain + nL * noiseGain;
            float outR = c1R * c1Gain + c2R * c2Gain + rmR * rmGain + nR * noiseGain;

            if (left) left[i] = outL;
            if (right) right[i] = outR;
        }
    }

    void setSources(const float* c1L, const float* c1R,
                    const float* c2L, const float* c2R,
                    const float* nL, const float* nR, int count) {
        tempC1L = c1L; tempC1R = c1R;
        tempC2L = c2L; tempC2R = c2R;
        tempNoiseBufL = nL; tempNoiseBufR = nR;
        sampleCount = count;
    }

private:
    const float* tempC1L = nullptr;
    const float* tempC1R = nullptr;
    const float* tempC2L = nullptr;
    const float* tempC2R = nullptr;
    const float* tempNoiseBufL = nullptr;
    const float* tempNoiseBufR = nullptr;
    int sampleCount = 0;
};

// --- BLOCK 7: FILTER (Type, Slope, Cutoff, Resonance) ---
class FilterBlock : public DSPBlock {
public:
    explicit FilterBlock(int voice = 1) : voiceIndex(voice) {}

    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        sampleRate = ctx.sampleRate;
        for (int s = 0; s < 4; ++s) {
            s1L[s] = s2L[s] = s1R[s] = s2R[s] = 0.0f;
        }
        rc1L = rc1R = 0.0f;
    }

    void reset() override {
        for (int s = 0; s < 4; ++s) {
            s1L[s] = s2L[s] = s1R[s] = s2R[s] = 0.0f;
        }
        rc1L = rc1R = 0.0f;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& ctx) override {
        // 1. Type: 0=LPF, 1=BPF, 2=HPF, 3=BRF (Notch)
        int type = std::clamp(static_cast<int>(std::round(params[0] * 3.0f)), 0, 3);

        // 2. Slope: 0=6dB/oct, 1=12dB/oct (def), 2=18dB/oct, 3=24dB/oct, 4=36dB/oct
        int slopeIdx = std::clamp(static_cast<int>(std::round(params[1] * 4.0f)), 0, 4);

        // 3. Cutoff: 0.1 Hz to 24 kHz (def 24 kHz)
        float slop = 0.0f;
        if (voiceIndex == 1) slop = ctx.slopFilter1Cutoff;
        else if (voiceIndex == 2) slop = ctx.slopFilter2Cutoff;
        else if (voiceIndex == 3) slop = ctx.slopFilter3Cutoff;
        else slop = ctx.slopFXFilterCutoff;

        float cutoffParam = std::clamp(params[2] + slop, 0.0f, 1.0f);
        float baseCutoff = 0.1f * std::pow(24000.0f / 0.1f, cutoffParam);

        // 4. Resonance: 0% to 100%
        float rawRes = params[3];

        // Number of 2-pole SVF stages and per-stage Q distribution:
        // For all slopes at -12dB and above, 100% resonance sits right at the self-oscillation level (peak gain ~ 20.0).
        // Higher-order slopes distribute resonance cleanly without runaway Q^2 or Q^3 gain explosions.
        int svfStages = 1;
        bool hasExtraPole = false;
        float stageQ[3] = { 0.7071f, 0.7071f, 0.7071f };
        constexpr float targetPeak = 20.0f;

        if (slopeIdx == 0) {
            svfStages = 1;
            stageQ[0] = 0.5f + rawRes * 5.0f; // Softened Q for 6dB slope
        } else if (slopeIdx == 1) {
            svfStages = 1; // 12 dB/oct (2 poles)
            stageQ[0] = 0.7071f + rawRes * (targetPeak - 0.7071f);
        } else if (slopeIdx == 2) {
            svfStages = 1; // 18 dB/oct (3 poles: 2 SVF + 1 RC)
            hasExtraPole = true;
            stageQ[0] = 0.7071f + rawRes * (targetPeak * 1.4142f - 0.7071f);
        } else if (slopeIdx == 3) {
            svfStages = 2; // 24 dB/oct (4 poles: 2 SVF stages)
            stageQ[0] = 0.7071f + rawRes * (targetPeak * 1.4142f - 0.7071f);
            stageQ[1] = 0.7071f;
        } else if (slopeIdx == 4) {
            svfStages = 3; // 36 dB/oct (6 poles: 3 SVF stages)
            stageQ[0] = 0.7071f + rawRes * (targetPeak * 2.0f - 0.7071f);
            stageQ[1] = 0.7071f;
            stageQ[2] = 0.7071f;
        }

        if (type == 3) {
            // BRF (Band Reject / Notch): wider bandwidth at 0% resonance matching bell width
            constexpr float minBrfQ = 0.25f;
            float brfQ = minBrfQ + rawRes * (targetPeak - minBrfQ);
            stageQ[0] = brfQ;
            stageQ[1] = brfQ;
            stageQ[2] = brfQ;
            if (slopeIdx == 0) stageQ[0] = minBrfQ + rawRes * 5.0f;
        }

        float preDrive = predriveGain;

        const std::vector<float>* envSig = nullptr;
        if (voiceIndex == 1) envSig = &ctx.filterEnv1Signal;
        else if (voiceIndex == 2) envSig = &ctx.filterEnv2Signal;
        else if (voiceIndex == 3) envSig = &ctx.filterEnv3Signal;

        for (int i = 0; i < numSamples; ++i) {
            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            // Pre-filter drive: saturates signal into the filter stages
            if (std::abs(preDrive - 1.0f) > 0.01f) {
                inL = FastMath::fastTanh(inL * preDrive);
                inR = FastMath::fastTanh(inR * preDrive);
            }

            // Cutoff modulated by Filter Envelope: depth is +/- 10 octaves
            float fEnv = (envSig && i < static_cast<int>(envSig->size())) ? (*envSig)[i] : 0.0f;
            float cutoff = baseCutoff * FastMath::fastPow2(fEnv * 10.0f);
            cutoff = std::clamp(cutoff, 0.1f, sampleRate * 0.485f);

            // SVF filter cascade
            float g = std::tan(PI * cutoff * invSr);
            int svfMode = type; // 0=LP, 1=BP, 2=HP, 3=BRF (Notch)

            for (int s = 0; s < svfStages; ++s) {
                float k = 1.0f / stageQ[s];
                float a1 = 1.0f / (1.0f + g * (g + k));

                float hpL = (inL - (g + k) * s1L[s] - s2L[s]) * a1;
                float bpL = g * hpL + s1L[s];
                s1L[s] = g * hpL + bpL;
                float lpL = g * bpL + s2L[s];
                s2L[s] = g * bpL + lpL;

                float hpR = (inR - (g + k) * s1R[s] - s2R[s]) * a1;
                float bpR = g * hpR + s1R[s];
                s1R[s] = g * hpR + bpR;
                float lpR = g * bpR + s2R[s];
                s2R[s] = g * bpR + lpR;

                if (svfMode == 0)      { inL = lpL; inR = lpR; }
                else if (svfMode == 1) { inL = bpL; inR = bpR; }
                else if (svfMode == 2) { inL = hpL; inR = hpR; }
                else                   { inL = hpL + lpL; inR = hpR + lpR; }
            }

            if (hasExtraPole) {
                float rcAlpha = g / (1.0f + g);
                if (svfMode == 0) {
                    rc1L += rcAlpha * (inL - rc1L);
                    rc1R += rcAlpha * (inR - rc1R);
                    inL = rc1L; inR = rc1R;
                } else if (svfMode == 2) {
                    rc1L += rcAlpha * (inL - rc1L);
                    rc1R += rcAlpha * (inR - rc1R);
                    inL = inL - rc1L; inR = inR - rc1R;
                }
            }

            if (left) left[i] = inL;
            if (right) right[i] = inR;
        }
    }

    void setPreDriveGain(float g) { predriveGain = g; }
    void setPostDriveGain(float g) { predriveGain = g; } // Backward compatibility alias

private:
    int voiceIndex = 1;
    float invSr = 1.0f / 44100.0f;
    float sampleRate = 44100.0f;
    float predriveGain = 1.0f;

    float s1L[4] = { 0.0f }, s2L[4] = { 0.0f };
    float s1R[4] = { 0.0f }, s2R[4] = { 0.0f };
    float rc1L = 0.0f, rc1R = 0.0f;
};

// --- BLOCK: COMB FILTER (Type, Dampening, Cutoff, Resonance) ---
class CombFilterBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        sampleRate = ctx.sampleRate;
        combBufferL.assign(maxDelaySamples, 0.0f);
        combBufferR.assign(maxDelaySamples, 0.0f);
        combWriteIdx = 0;
        combDampL = combDampR = 0.0f;
    }

    void reset() override {
        std::fill(combBufferL.begin(), combBufferL.end(), 0.0f);
        std::fill(combBufferR.begin(), combBufferR.end(), 0.0f);
        combWriteIdx = 0;
        combDampL = combDampR = 0.0f;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& ctx) override {
        // 1. Dampening: 0.1 Hz to 24 kHz (def 24 kHz)
        float dampParam = std::clamp(params[0] + ctx.slopCombDamp, 0.0f, 1.0f);
        float dampHz = 0.1f * std::pow(24000.0f / 0.1f, dampParam);
        float combDampCoeff = std::clamp(TWO_PI * dampHz * invSr, 0.0001f, 0.999f);

        // 2. Cutoff: 0.1 Hz to 24 kHz (def 24 kHz)
        float cutoffParam = std::clamp(params[1] + ctx.slopCombCutoff, 0.0f, 1.0f);
        float cutoff = 0.1f * std::pow(24000.0f / 0.1f, cutoffParam);
        cutoff = std::clamp(cutoff, 20.0f, sampleRate * 0.485f);

        // 3. Resonance: -100% to 0% to +100% (bipolar, def 0% = 0.5)
        float rawRes = params[2];
        float combFb = (rawRes - 0.5f) * 2.0f * 0.98f;

        // 4. Mix: -100%:0% .. 0%:100% (Dry at 0.5) .. +100%:0% (def +50%:50% = 0.75)
        float blend = (params[3] - 0.5f) * 2.0f;
        float wetAmount = std::abs(blend);
        float wetSign = (blend >= 0.0f) ? 1.0f : -1.0f;

        float delayLen = std::clamp(sampleRate / cutoff, 2.0f, static_cast<float>(maxDelaySamples - 2));

        for (int i = 0; i < numSamples; ++i) {
            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            float readPos = static_cast<float>(combWriteIdx) - delayLen;
            if (readPos < 0.0f) readPos += maxDelaySamples;

            int iPos = static_cast<int>(readPos);
            float frac = readPos - iPos;
            int iNext = (iPos + 1) % maxDelaySamples;

            float delayedL = combBufferL[iPos] * (1.0f - frac) + combBufferL[iNext] * frac;
            float delayedR = combBufferR[iPos] * (1.0f - frac) + combBufferR[iNext] * frac;

            combDampL += combDampCoeff * (delayedL - combDampL);
            combDampR += combDampCoeff * (delayedR - combDampR);

            combBufferL[combWriteIdx] = inL + combDampL * combFb;
            combBufferR[combWriteIdx] = inR + combDampR * combFb;
            combWriteIdx = (combWriteIdx + 1) % maxDelaySamples;

            float outL = inL * (1.0f - wetAmount) + (delayedL * wetSign) * wetAmount;
            float outR = inR * (1.0f - wetAmount) + (delayedR * wetSign) * wetAmount;

            if (left) left[i] = outL;
            if (right) right[i] = outR;
        }
    }

private:
    float invSr = 1.0f / 44100.0f;
    float sampleRate = 44100.0f;
    static constexpr int maxDelaySamples = 4096;
    std::vector<float> combBufferL;
    std::vector<float> combBufferR;
    int combWriteIdx = 0;
    float combDampL = 0.0f, combDampR = 0.0f;
};

// --- BLOCK: PHASE_SMEAR (Type, Amount, Cutoff, Resonance) ---
class PhaseSmearBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        sampleRate = ctx.sampleRate;
        for (int i = 0; i < 32; ++i) {
            apfS1L[i] = apfS2L[i] = apf2S1L[i] = apf2S2L[i] = 0.0f;
            apfS1R[i] = apfS2R[i] = apf2S1R[i] = apf2S2R[i] = 0.0f;
        }
    }

    void trigger(float) override {}

    void reset() override {
        for (int i = 0; i < 32; ++i) {
            apfS1L[i] = apfS2L[i] = apf2S1L[i] = apf2S2L[i] = 0.0f;
            apfS1R[i] = apfS2R[i] = apf2S1R[i] = apf2S2R[i] = 0.0f;
        }
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& ctx) override {
        // 1. Order: 0 = 2nd Order, 1 = 4th Order
        bool fourthOrder = (params[0] >= 0.5f);

        // 2. Amount: 0 to 32 APFs (def 4 = 4.0 / 32.0)
        int apfStages = std::clamp(static_cast<int>(std::round(params[1] * 32.0f)), 0, 32);
        if (apfStages == 0) return;

        // 3. Cutoff: 0.1 Hz to 24 kHz (def 24 kHz)
        float cutoffParam = std::clamp(params[2] + ctx.slopPhaseSmearCutoff, 0.0f, 1.0f);
        float cutoff = 0.1f * std::pow(24000.0f / 0.1f, cutoffParam);
        cutoff = std::clamp(cutoff, 10.0f, sampleRate * 0.485f);

        // 4. Resonance: -100% to 0% to +100% (bipolar, def 0% = 0.5)
        // High resonance (Q) creates a steep phase transition and dramatic group delay
        // (the classic laser zap / chirp / smearing of Phase Smear)
        float phasesmearRes = (params[3] - 0.5f) * 2.0f;
        float Q = 0.7071f;
        if (phasesmearRes >= 0.0f) {
            Q = 0.7071f * std::pow(35.0f, phasesmearRes);
        } else {
            Q = 0.7071f * std::pow(0.25f, -phasesmearRes);
        }
        Q = std::clamp(Q, 0.1f, 30.0f);

        // Allpass biquad coefficients (RBJ Cookbook normalized)
        float w0 = TWO_PI * cutoff * invSr;
        w0 = std::clamp(w0, 0.001f, 3.10f);
        float cosw0 = std::cos(w0);
        float sinw0 = std::sin(w0);
        float alpha = std::clamp(sinw0 / (2.0f * Q), 1e-6f, 10.0f);

        float a0 = 1.0f + alpha;
        float b0 = (1.0f - alpha) / a0;
        float b1 = (-2.0f * cosw0) / a0;
        float b2 = 1.0f;
        float a1 = b1;
        float a2 = b0;

        for (int i = 0; i < numSamples; ++i) {
            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            for (int st = 0; st < apfStages; ++st) {
                // Section 1: Left channel Direct Form II Transposed
                float yL = b0 * inL + apfS1L[st];
                apfS1L[st] = b1 * inL - a1 * yL + apfS2L[st];
                apfS2L[st] = b2 * inL - a2 * yL;
                inL = yL;

                // Section 1: Right channel Direct Form II Transposed
                float yR = b0 * inR + apfS1R[st];
                apfS1R[st] = b1 * inR - a1 * yR + apfS2R[st];
                apfS2R[st] = b2 * inR - a2 * yR;
                inR = yR;

                if (fourthOrder) {
                    // Section 2: Left channel Direct Form II Transposed (cascaded for 4th-order APF)
                    float yL2 = b0 * inL + apf2S1L[st];
                    apf2S1L[st] = b1 * inL - a1 * yL2 + apf2S2L[st];
                    apf2S2L[st] = b2 * inL - a2 * yL2;
                    inL = yL2;

                    // Section 2: Right channel Direct Form II Transposed
                    float yR2 = b0 * inR + apf2S1R[st];
                    apf2S1R[st] = b1 * inR - a1 * yR2 + apf2S2R[st];
                    apf2S2R[st] = b2 * inR - a2 * yR2;
                    inR = yR2;

                    if (std::abs(apf2S1L[st]) < 1e-15f) apf2S1L[st] = 0.0f;
                    if (std::abs(apf2S2L[st]) < 1e-15f) apf2S2L[st] = 0.0f;
                    if (std::abs(apf2S1R[st]) < 1e-15f) apf2S1R[st] = 0.0f;
                    if (std::abs(apf2S2R[st]) < 1e-15f) apf2S2R[st] = 0.0f;
                }

                // Flush denormals
                if (std::abs(apfS1L[st]) < 1e-15f) apfS1L[st] = 0.0f;
                if (std::abs(apfS2L[st]) < 1e-15f) apfS2L[st] = 0.0f;
                if (std::abs(apfS1R[st]) < 1e-15f) apfS1R[st] = 0.0f;
                if (std::abs(apfS2R[st]) < 1e-15f) apfS2R[st] = 0.0f;
            }

            if (left) left[i] = inL;
            if (right) right[i] = inR;
        }
    }

private:
    float invSr = 1.0f / 44100.0f;
    float sampleRate = 44100.0f;
    float apfS1L[32] = { 0.0f };
    float apfS2L[32] = { 0.0f };
    float apfS1R[32] = { 0.0f };
    float apfS2R[32] = { 0.0f };
    float apf2S1L[32] = { 0.0f };
    float apf2S2L[32] = { 0.0f };
    float apf2S1R[32] = { 0.0f };
    float apf2S2R[32] = { 0.0f };
};

using PhaseSmearBlock = PhaseSmearBlock;

// --- BLOCK: EQ (Bell EQ + DJ Filter) ---
class EQBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        sampleRate = ctx.sampleRate;
        s1L = s2L = s1R = s2R = 0.0f;
        djFilter.reset();
    }

    void trigger(float) override {}

    void reset() override {
        s1L = s2L = s1R = s2R = 0.0f;
        djFilter.reset();
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& ctx) override {
        // 1. Frequency: 20 Hz to 24 kHz (def 24 kHz = 1.0)
        float freqParam = std::clamp(params[0] + ctx.slopEQFreq, 0.0f, 1.0f);
        float freq = 20.0f * std::pow(24000.0f / 20.0f, freqParam);
        freq = std::clamp(freq, 20.0f, sampleRate * 0.485f);

        // 2. Width: 0.1 to 10 octaves (def 0.1 octaves = 0.0)
        float width = 0.1f * std::pow(100.0f, params[1]);
        width = std::clamp(width, 0.05f, 10.0f);

        // 3. Gain: -24 dB to 0 dB to +24 dB (def 0 dB = 0.5)
        float gainDb = (params[2] - 0.5f) * 48.0f;
        bool hasBell = (std::abs(gainDb) > 0.05f);

        // 4. DJ Filter: (def 50% Flat = 0.5)
        float filterKnob = std::clamp(params[3] + ctx.slopEQFilter, 0.0f, 1.0f);

        // RBJ Peaking EQ Biquad coefficients
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
        if (hasBell) {
            float w0 = TWO_PI * freq * invSr;
            float cosw0 = std::cos(w0);
            float sinw0 = std::sin(w0);
            float A = std::pow(10.0f, gainDb / 40.0f);
            float alpha = sinw0 * std::sinh(0.34657359f * width * (w0 / (sinw0 > 1e-6f ? sinw0 : 1e-6f)));
            alpha = std::clamp(alpha, 1e-6f, 10.0f);

            float a0 = 1.0f + alpha / A;
            b0 = (1.0f + alpha * A) / a0;
            b1 = (-2.0f * cosw0) / a0;
            b2 = (1.0f - alpha * A) / a0;
            a1 = (-2.0f * cosw0) / a0;
            a2 = (1.0f - alpha / A) / a0;
        }

        for (int i = 0; i < numSamples; ++i) {
            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            if (hasBell) {
                float yL = b0 * inL + s1L;
                s1L = b1 * inL - a1 * yL + s2L;
                s2L = b2 * inL - a2 * yL;
                inL = yL;

                float yR = b0 * inR + s1R;
                s1R = b1 * inR - a1 * yR + s2R;
                s2R = b2 * inR - a2 * yR;
                inR = yR;

                if (std::abs(s1L) < 1e-15f) s1L = 0.0f;
                if (std::abs(s2L) < 1e-15f) s2L = 0.0f;
                if (std::abs(s1R) < 1e-15f) s1R = 0.0f;
                if (std::abs(s2R) < 1e-15f) s2R = 0.0f;
            }

            djFilter.process(inL, inR, filterKnob, sampleRate);

            if (left) left[i] = inL;
            if (right) right[i] = inR;
        }
    }

private:
    float invSr = 1.0f / 44100.0f;
    float sampleRate = 44100.0f;
    float s1L = 0.0f, s2L = 0.0f;
    float s1R = 0.0f, s2R = 0.0f;
    DJFilter djFilter;
};

// --- CHORUS EFFECT (Rate, Depth, Feedback, Mix) ---
class ChorusBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        sampleRate = ctx.sampleRate;
        maxDelay = static_cast<int>(sampleRate * 0.05f) + 16;
        if (maxDelay < 512) maxDelay = 512;
        bufL.assign(maxDelay, 0.0f);
        bufR.assign(maxDelay, 0.0f);
        writeIdx = 0;
        lfoPhase = 0.0f;
    }

    void reset() override {
        std::fill(bufL.begin(), bufL.end(), 0.0f);
        std::fill(bufR.begin(), bufR.end(), 0.0f);
        writeIdx = 0;
        lfoPhase = 0.0f;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& /*ctx*/) override {
        // 1. Rate: 0.1 Hz to 10.0 Hz (def 1.2 Hz)
        float rate = 0.1f * std::pow(100.0f, params[0]);
        float phaseInc = rate * invSr;

        // 2. Depth: 0% to 100% -> 0 to 8 ms modulation
        float depthSamples = params[1] * (0.008f * sampleRate);

        // 3. Feedback: -100% to +100% (bipolar, def +20%)
        float fb = (params[2] - 0.5f) * 1.90f;

        // 4. Mix: 0% to 100% (def 50%)
        float mix = params[3];
        float dry = 1.0f - mix;

        float baseDelaySamples = 0.015f * sampleRate;

        for (int i = 0; i < numSamples; ++i) {
            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            float lfoL = FastMath::fastSinNorm(lfoPhase);
            float lfoR = FastMath::fastCosNorm(lfoPhase);

            lfoPhase += phaseInc;
            if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

            float dL = std::clamp(baseDelaySamples + depthSamples * lfoL, 1.0f, static_cast<float>(maxDelay - 2));
            float dR = std::clamp(baseDelaySamples + depthSamples * lfoR, 1.0f, static_cast<float>(maxDelay - 2));

            auto readInterp = [](const std::vector<float>& buf, int wIdx, float delay, int sz) {
                float readPos = static_cast<float>(wIdx) - delay;
                while (readPos < 0.0f) readPos += sz;
                int i0 = static_cast<int>(readPos);
                int i1 = (i0 + 1) % sz;
                float frac = readPos - static_cast<float>(i0);
                return buf[i0] + frac * (buf[i1] - buf[i0]);
            };

            float wetL = readInterp(bufL, writeIdx, dL, maxDelay);
            float wetR = readInterp(bufR, writeIdx, dR, maxDelay);

            bufL[writeIdx] = inL + FastMath::fastTanh(wetL * fb);
            bufR[writeIdx] = inR + FastMath::fastTanh(wetR * fb);
            writeIdx = (writeIdx + 1) % maxDelay;

            if (left)  left[i]  = dry * inL + mix * wetL;
            if (right) right[i] = dry * inR + mix * wetR;
        }
    }

private:
    float sampleRate = 44100.0f;
    float invSr = 1.0f / 44100.0f;
    int maxDelay = 2048;
    int writeIdx = 0;
    float lfoPhase = 0.0f;
    std::vector<float> bufL;
    std::vector<float> bufR;
};

// --- PHASER EFFECT (Rate, Depth, Feedback, Mix) ---
class PhaserBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        sampleRate = ctx.sampleRate;
        for (int ch = 0; ch < 2; ++ch) {
            for (int s = 0; s < 6; ++s) {
                apfX[ch][s] = 0.0f;
                apfY[ch][s] = 0.0f;
            }
            lastFb[ch] = 0.0f;
        }
        lfoPhase = 0.0f;
    }

    void reset() override {
        for (int ch = 0; ch < 2; ++ch) {
            for (int s = 0; s < 6; ++s) {
                apfX[ch][s] = 0.0f;
                apfY[ch][s] = 0.0f;
            }
            lastFb[ch] = 0.0f;
        }
        lfoPhase = 0.0f;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& /*ctx*/) override {
        // 1. Rate: 0.05 Hz to 8.0 Hz (def 0.5 Hz)
        float rate = 0.05f * std::pow(160.0f, params[0]);
        float phaseInc = rate * invSr;

        // 2. Depth: 0% to 100% (def 70%)
        float depth = params[1];

        // 3. Feedback: -95% to +95% (bipolar, def +50%)
        float fb = (params[2] - 0.5f) * 1.90f;

        // 4. Mix: 0% to 100% (def 50%)
        float mix = params[3];
        float dry = 1.0f - mix;

        for (int i = 0; i < numSamples; ++i) {
            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            float lfoL = 0.5f * (1.0f + FastMath::fastSinNorm(lfoPhase));
            float lfoR = 0.5f * (1.0f + FastMath::fastCosNorm(lfoPhase));

            lfoPhase += phaseInc;
            if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

            float fMin = 200.0f;
            float fMax = 200.0f * std::pow(20.0f, depth);
            float fcL = std::clamp(fMin * std::pow(fMax / fMin, lfoL), 20.0f, sampleRate * 0.45f);
            float fcR = std::clamp(fMin * std::pow(fMax / fMin, lfoR), 20.0f, sampleRate * 0.45f);

            float wL = std::tan(PI * fcL * invSr);
            float aL = (wL - 1.0f) / (wL + 1.0f);

            float wR = std::tan(PI * fcR * invSr);
            float aR = (wR - 1.0f) / (wR + 1.0f);

            // Channel 0: Left
            float x0 = inL + FastMath::fastTanh(lastFb[0] * fb);
            for (int s = 0; s < 6; ++s) {
                float y = aL * x0 + apfX[0][s] - aL * apfY[0][s];
                apfX[0][s] = x0;
                apfY[0][s] = y;
                x0 = y;
            }
            lastFb[0] = x0;
            float wetL = x0;

            // Channel 1: Right
            float x1 = inR + FastMath::fastTanh(lastFb[1] * fb);
            for (int s = 0; s < 6; ++s) {
                float y = aR * x1 + apfX[1][s] - aR * apfY[1][s];
                apfX[1][s] = x1;
                apfY[1][s] = y;
                x1 = y;
            }
            lastFb[1] = x1;
            float wetR = x1;

            if (left)  left[i]  = dry * inL + mix * wetL;
            if (right) right[i] = dry * inR + mix * wetR;
        }
    }

private:
    float sampleRate = 44100.0f;
    float invSr = 1.0f / 44100.0f;
    float lfoPhase = 0.0f;
    float apfX[2][6] = {};
    float apfY[2][6] = {};
    float lastFb[2] = {};
};

// --- FLANGER EFFECT (Rate, Depth, Feedback, Mix) ---
class FlangerBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        sampleRate = ctx.sampleRate;
        maxDelay = static_cast<int>(sampleRate * 0.02f) + 16;
        if (maxDelay < 256) maxDelay = 256;
        bufL.assign(maxDelay, 0.0f);
        bufR.assign(maxDelay, 0.0f);
        writeIdx = 0;
        lfoPhase = 0.0f;
    }

    void reset() override {
        std::fill(bufL.begin(), bufL.end(), 0.0f);
        std::fill(bufR.begin(), bufR.end(), 0.0f);
        writeIdx = 0;
        lfoPhase = 0.0f;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& /*ctx*/) override {
        // 1. Rate: 0.05 Hz to 5.0 Hz (def 0.25 Hz)
        float rate = 0.05f * std::pow(100.0f, params[0]);
        float phaseInc = rate * invSr;

        // 2. Depth: 0% to 100% -> 0 to 4 ms excursion
        float depthSamples = params[1] * (0.004f * sampleRate);

        // 3. Feedback: -95% to +95% (bipolar, def +70%)
        float fb = (params[2] - 0.5f) * 1.90f;

        // 4. Mix: 0% to 100% (def 50%)
        float mix = params[3];
        float dry = 1.0f - mix;

        float baseDelaySamples = 0.001f * sampleRate;

        for (int i = 0; i < numSamples; ++i) {
            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            float lfoL = 0.5f * (1.0f + FastMath::fastSinNorm(lfoPhase));
            float lfoR = 0.5f * (1.0f + FastMath::fastCosNorm(lfoPhase));

            lfoPhase += phaseInc;
            if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

            float dL = std::clamp(baseDelaySamples + depthSamples * lfoL, 1.0f, static_cast<float>(maxDelay - 2));
            float dR = std::clamp(baseDelaySamples + depthSamples * lfoR, 1.0f, static_cast<float>(maxDelay - 2));

            auto readInterp = [](const std::vector<float>& buf, int wIdx, float delay, int sz) {
                float readPos = static_cast<float>(wIdx) - delay;
                while (readPos < 0.0f) readPos += sz;
                int i0 = static_cast<int>(readPos);
                int i1 = (i0 + 1) % sz;
                float frac = readPos - static_cast<float>(i0);
                return buf[i0] + frac * (buf[i1] - buf[i0]);
            };

            float wetL = readInterp(bufL, writeIdx, dL, maxDelay);
            float wetR = readInterp(bufR, writeIdx, dR, maxDelay);

            bufL[writeIdx] = inL + FastMath::fastTanh(wetL * fb);
            bufR[writeIdx] = inR + FastMath::fastTanh(wetR * fb);
            writeIdx = (writeIdx + 1) % maxDelay;

            if (left)  left[i]  = dry * inL + mix * wetL;
            if (right) right[i] = dry * inR + mix * wetR;
        }
    }

private:
    float sampleRate = 44100.0f;
    float invSr = 1.0f / 44100.0f;
    int maxDelay = 1024;
    int writeIdx = 0;
    float lfoPhase = 0.0f;
    std::vector<float> bufL;
    std::vector<float> bufR;
};

// --- TEMPO DELAY EFFECT (Division, Feedback, Tone, Mix) ---
class DelayBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        sampleRate = ctx.sampleRate;
        maxDelay = static_cast<int>(sampleRate * 4.0f) + 16;
        bufL.assign(maxDelay, 0.0f);
        bufR.assign(maxDelay, 0.0f);
        writeIdx = 0;
        currentDelayL = currentDelayR = 0.25f * sampleRate;
        dampL = dampR = 0.0f;
    }

    void reset() override {
        std::fill(bufL.begin(), bufL.end(), 0.0f);
        std::fill(bufR.begin(), bufR.end(), 0.0f);
        writeIdx = 0;
        currentDelayL = currentDelayR = 0.25f * sampleRate;
        dampL = dampR = 0.0f;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& ctx) override {
        // 1. Division: 10 musical divisions (def 1/8)
        constexpr float mults[10] = { 0.125f, 0.166667f, 0.25f, 0.375f, 0.333333f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f };
        int divIdx = std::clamp(static_cast<int>(std::round(params[0] * 9.0f)), 0, 9);
        float mult = mults[divIdx];

        float effectiveBpm = (ctx.bpm > 20.0f && ctx.bpm < 999.0f) ? ctx.bpm : 120.0f;
        float secPerBeat = 60.0f / effectiveBpm;
        float targetDelaySec = mult * secPerBeat;
        float targetDelaySamples = std::clamp(targetDelaySec * sampleRate, 1.0f, static_cast<float>(maxDelay - 4));

        // 2. Feedback: 0% to 100% (def 40%)
        float fb = params[1] * 0.98f;

        // 3. Tone / Damp (Low-pass cutoff 500 Hz to 20 kHz)
        float dampFreq = 500.0f * std::pow(40.0f, params[2]);
        float dampAlpha = std::clamp(TWO_PI * dampFreq * invSr, 0.01f, 0.99f);

        // 4. Mix: 0% to 100% (def 35%)
        float mix = params[3];
        float dry = 1.0f - mix;

        float smoothRate = 0.002f;

        for (int i = 0; i < numSamples; ++i) {
            currentDelayL += (targetDelaySamples - currentDelayL) * smoothRate;
            currentDelayR += (targetDelaySamples - currentDelayR) * smoothRate;

            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            auto readInterp = [](const std::vector<float>& buf, int wIdx, float delay, int sz) {
                float readPos = static_cast<float>(wIdx) - delay;
                while (readPos < 0.0f) readPos += sz;
                int i0 = static_cast<int>(readPos);
                int i1 = (i0 + 1) % sz;
                float frac = readPos - static_cast<float>(i0);
                return buf[i0] + frac * (buf[i1] - buf[i0]);
            };

            float wetL = readInterp(bufL, writeIdx, currentDelayL, maxDelay);
            float wetR = readInterp(bufR, writeIdx, currentDelayR, maxDelay);

            dampL += dampAlpha * (wetL - dampL);
            dampR += dampAlpha * (wetR - dampR);

            bufL[writeIdx] = inL + FastMath::fastTanh((0.75f * dampL + 0.25f * dampR) * fb);
            bufR[writeIdx] = inR + FastMath::fastTanh((0.75f * dampR + 0.25f * dampL) * fb);
            writeIdx = (writeIdx + 1) % maxDelay;

            if (left)  left[i]  = dry * inL + mix * wetL;
            if (right) right[i] = dry * inR + mix * wetR;
        }
    }

private:
    float sampleRate = 44100.0f;
    float invSr = 1.0f / 44100.0f;
    int maxDelay = 192000;
    int writeIdx = 0;
    float currentDelayL = 22050.0f;
    float currentDelayR = 22050.0f;
    float dampL = 0.0f;
    float dampR = 0.0f;
    std::vector<float> bufL;
    std::vector<float> bufR;
};

// --- BLOCK 11: FILTER ENVELOPE (Slope, Depth, Decay, Post-Drive) ---
class FilterEnvelopeBlock : public DSPBlock {
public:
    explicit FilterEnvelopeBlock(int voice = 1) : voiceIndex(voice) {}

    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        timeSinceTrigger = 1000.0f;
    }

    void trigger(float) override {
        timeSinceTrigger = 0.0f;
        float slope = params[0];
        float baseDepth = (params[1] - 0.5f) * 2.0f;
        currentVal = applyEnvelopeSlope(1.0f, slope) * baseDepth;
    }

    void reset() override {
        timeSinceTrigger = 1000.0f;
        currentVal = 0.0f;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* /*left*/, float* /*right*/, int numSamples, BlockContext& ctx) override {
        // 1. Slope: Exp (0.0, def) -> Lin (0.5) -> Log (1.0)
        float slope = params[0];

        // 2. Depth: -10 octaves to 0 to +10 octaves (bipolar, def 0 octaves = 0.5) + slop + velocity/key modulation
        float slopDepth = (voiceIndex == 1) ? ctx.slopFilterEnv1Depth :
                          ((voiceIndex == 2) ? ctx.slopFilterEnv2Depth : ctx.slopFilterEnv3Depth);
        float effDepthParam = std::clamp(params[1] + slopDepth, 0.0f, 1.0f);
        float baseDepth = (effDepthParam - 0.5f) * 2.0f;
        float depth = std::clamp(baseDepth + ctx.velDepthMod + ctx.keyDepthMod, -1.0f, 1.0f);

        // 3. Decay: 5-point warp (5ms, 100ms, 1s, 5s, 60s; def 333ms) + slop + velocity/key modulation
        float slopDecay = (voiceIndex == 1) ? ctx.slopFilterEnv1Decay :
                          ((voiceIndex == 2) ? ctx.slopFilterEnv2Decay : ctx.slopFilterEnv3Decay);
        float decayParam = std::clamp(params[2] + slopDecay + ctx.velDecayMod + ctx.keyDecayMod, 0.0f, 1.0f);
        float decayTime = warp5PointTime(decayParam);
        decayTime = std::max(decayTime, 0.001f);

        // 4. Post-drive (for the filter, not the envelope): -6dB to 0dB (at 0.5) to +24dB (def 0dB = 0.5)
        float postDriveDb = (params[3] <= 0.5f) ? (-6.0f + params[3] * 12.0f) : ((params[3] - 0.5f) * 48.0f);
        postDriveGain = std::pow(10.0f, postDriveDb / 20.0f);

        std::vector<float>& targetSig = (voiceIndex == 1) ? ctx.filterEnv1Signal :
                                        ((voiceIndex == 2) ? ctx.filterEnv2Signal : ctx.filterEnv3Signal);

        int count = std::min(numSamples, static_cast<int>(targetSig.size()));
        for (int i = 0; i < count; ++i) {
            float linearProgress = timeSinceTrigger / decayTime;
            float envLinear = std::clamp(1.0f - linearProgress, 0.0f, 1.0f);
            float envVal = applyEnvelopeSlope(envLinear, slope) * depth;
            timeSinceTrigger += invSr;
            targetSig[i] = envVal;
            currentVal = envVal;
        }
    }

    float getPostDriveGain() const { return postDriveGain; }
    float getCurrentValue() const { return currentVal; }

private:
    int voiceIndex = 1;
    float invSr = 1.0f / 44100.0f;
    float timeSinceTrigger = 1000.0f;
    float postDriveGain = 1.0f;
    float currentVal = 0.0f;
};

// --- BLOCK: WAVE FOLDER (Type, Fold, Bias, Post-Filter) ---
class WaveFolderBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        sampleRate = ctx.sampleRate;
        djFilter.reset();
    }

    void reset() override {
        djFilter.reset();
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& ctx) override {
        bool enabled = (params[0] >= 0.5f);
        if (!enabled) return; // bypass

        float numFolds = params[1] * 8.0f;
        float bias = (params[2] - 0.5f) * 2.0f;
        float filterKnob = std::clamp(params[3] + ctx.slopWaveFolderFilter, 0.0f, 1.0f);

        float driveFactor = 1.0f + numFolds * 2.0f;

        for (int i = 0; i < numSamples; ++i) {
            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            inL += bias;
            inR += bias;

            inL = std::sin(inL * driveFactor * (PI * 0.5f));
            inR = std::sin(inR * driveFactor * (PI * 0.5f));

            inL -= bias * 0.5f;
            inR -= bias * 0.5f;

            djFilter.process(inL, inR, filterKnob, sampleRate);

            if (left) left[i] = inL;
            if (right) right[i] = inR;
        }
    }

private:
    float sampleRate = 44100.0f;
    DJFilter djFilter;
};

// --- BLOCK 9: RING MODULATOR ---
class RingModBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        phase = 0.0f;
    }

    void reset() override {
        phase = 0.0f;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& ctx) override {
        // 1. Waveform morph: Sine 0% -> Tri 20% -> Saw 40% -> Square 60% -> PWM 0% 100%
        float shape = params[0];

        // 2. Rate: 0.1 Hz to 24 kHz (def 55 Hz)
        float rateParam = std::clamp(params[1] + ctx.slopRingModRate, 0.0f, 1.0f);
        float rate = 0.1f * std::pow(24000.0f / 0.1f, rateParam);

        // 3. Amount: 0% to 100% (def 0%)
        float amount = params[2];

        // 4. Width: -100% to 0% to +100% (def 0%)
        float width = (params[3] - 0.5f) * 2.0f;

        for (int i = 0; i < numSamples; ++i) {
            phase += rate * invSr;
            if (phase >= 1.0f) phase -= std::floor(phase);

            float phaseL = phase;
            float phaseR = phase + width * 0.25f;
            if (phaseR >= 1.0f) phaseR -= std::floor(phaseR);
            if (phaseR < 0.0f) phaseR += 1.0f - std::floor(phaseR);

            float modL = evaluateWaveform(phaseL, shape);
            float modR = evaluateWaveform(phaseR, shape);

            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            float wetL = inL * modL;
            float wetR = inR * modR;

            if (left) left[i] = inL * (1.0f - amount) + wetL * amount;
            if (right) right[i] = inR * (1.0f - amount) + wetR * amount;
        }
    }

private:
    float invSr = 1.0f / 44100.0f;
    float phase = 0.0f;
};

// --- BLOCK 10: FREQUENCY SHIFTER ---
class FrequencyShifterBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        phaseL = phaseR = 0.0f;
        dL = dR = 0.0f;
        for (int k = 0; k < 4; ++k) {
            s1_1L[k] = s2_1L[k] = s1_2L[k] = s2_2L[k] = 0.0f;
            s1_1R[k] = s2_1R[k] = s1_2R[k] = s2_2R[k] = 0.0f;
        }
    }

    void reset() override {
        phaseL = phaseR = 0.0f;
        dL = dR = 0.0f;
        for (int k = 0; k < 4; ++k) {
            s1_1L[k] = s2_1L[k] = s1_2L[k] = s2_2L[k] = 0.0f;
            s1_1R[k] = s2_1R[k] = s1_2R[k] = s2_2R[k] = 0.0f;
        }
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& /*ctx*/) override {
        // 1. Shift: -X Hz to 0 Hz to +X Hz (where X is range, bipolar, def 0 Hz)
        float shiftNorm = (params[0] - 0.5f) * 2.0f;

        // 2. Range: 0 Hz to 5 kHz (def 3 Hz, cubic curve for precision control)
        float rangeHz = normToRangeHz(params[1]);
        float totalShift = shiftNorm * rangeHz;

        // 3. Blend: -100% to -50% to 0% (Dry) to +50% to +100%
        float blend = (params[2] - 0.5f) * 2.0f;

        // 4. Width: -100% to 0% to +100% (def 0%)
        float width = (params[3] - 0.5f) * 2.0f;

        float shiftL = totalShift * (1.0f - width * 0.35f);
        float shiftR = totalShift * (1.0f + width * 0.35f);

        // 4-section Niemitalo half-band allpass filter coefficients (squared pole locations)
        // Maintains exact 90-degree phase difference across entire audio band
        constexpr float poles1[4] = { 0.4794009f, 0.8762185f, 0.9765976f, 0.9974992f };
        constexpr float poles2[4] = { 0.1617585f, 0.7330290f, 0.9453500f, 0.9905990f };

        float wetAmount = std::abs(blend);
        float wetSign = (blend >= 0.0f) ? 1.0f : -1.0f;

        for (int i = 0; i < numSamples; ++i) {
            phaseL += shiftL * invSr;
            phaseL -= std::floor(phaseL);

            phaseR += shiftR * invSr;
            phaseR -= std::floor(phaseR);

            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            // Left Channel Hilbert 90-deg Phase Split
            // Branch 1 (Q path, with 1-sample delay)
            float qL = dL;
            dL = inL;
            for (int k = 0; k < 4; ++k) {
                float c = poles1[k];
                float out = c * qL + s2_1L[k];
                s2_1L[k] = s1_1L[k];
                s1_1L[k] = -qL + c * out;
                qL = out;
            }

            // Branch 2 (I path)
            float iL = inL;
            for (int k = 0; k < 4; ++k) {
                float c = poles2[k];
                float out = c * iL + s2_2L[k];
                s2_2L[k] = s1_2L[k];
                s1_2L[k] = -iL + c * out;
                iL = out;
            }

            // Right Channel Hilbert 90-deg Phase Split
            float qR = dR;
            dR = inR;
            for (int k = 0; k < 4; ++k) {
                float c = poles1[k];
                float out = c * qR + s2_1R[k];
                s2_1R[k] = s1_1R[k];
                s1_1R[k] = -qR + c * out;
                qR = out;
            }

            float iR = inR;
            for (int k = 0; k < 4; ++k) {
                float c = poles2[k];
                float out = c * iR + s2_2R[k];
                s2_2R[k] = s1_2R[k];
                s1_2R[k] = -iR + c * out;
                iR = out;
            }

            // Quadrature carrier modulation
            // shift > 0 shifts frequency UP; shift < 0 shifts frequency DOWN
            float cosL = FastMath::fastCosNorm(phaseL);
            float sinL = FastMath::fastSinNorm(phaseL);
            float shiftedL = iL * cosL - qL * sinL;

            float cosR = FastMath::fastCosNorm(phaseR);
            float sinR = FastMath::fastSinNorm(phaseR);
            float shiftedR = iR * cosR - qR * sinR;

            float outL = inL * (1.0f - wetAmount) + (shiftedL * wetSign) * wetAmount;
            float outR = inR * (1.0f - wetAmount) + (shiftedR * wetSign) * wetAmount;

            if (left) left[i] = outL;
            if (right) right[i] = outR;
        }
    }

private:
    float invSr = 1.0f / 44100.0f;
    float phaseL = 0.0f, phaseR = 0.0f;
    float dL = 0.0f, dR = 0.0f;
    float s1_1L[4] = { 0.0f }, s2_1L[4] = { 0.0f };
    float s1_2L[4] = { 0.0f }, s2_2L[4] = { 0.0f };
    float s1_1R[4] = { 0.0f }, s2_1R[4] = { 0.0f };
    float s1_2R[4] = { 0.0f }, s2_2R[4] = { 0.0f };
};

// --- BLOCK 11: GRIT FX (Bits, Sample Rate, Low Boost, High Boost) ---
class GritBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        sampleRate = ctx.sampleRate;
        invSr = ctx.invSr;
        holdL = holdR = 0.0f;
        acc = 0.0f;
        lowS1L = lowS2L = lowS1R = lowS2R = 0.0f;
        highS1L = highS2L = highS1R = highS2R = 0.0f;
    }

    void reset() override {
        holdL = holdR = 0.0f;
        acc = 0.0f;
        lowS1L = lowS2L = lowS1R = lowS2R = 0.0f;
        highS1L = highS2L = highS1R = highS2R = 0.0f;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& /*ctx*/) override {
        // 1. Bit reduction: 1.0 to 16.0 bits (def 16.0)
        float bits = 1.0f + params[0] * 15.0f;
        float steps = std::pow(2.0f, bits);
        bool hasBitCrush = (bits < 15.9f);

        // 2. Sample rate reduction: 20 Hz to 24 kHz (def 24 kHz)
        float targetSr = 20.0f * std::pow(24000.0f / 20.0f, params[1]);
        float phaseInc = targetSr / sampleRate;
        bool hasDownsample = (params[1] < 0.999f && targetSr < sampleRate * 0.495f);

        // 3. Low Shelf: -24 dB to 0 dB to +24 dB (def 0 dB = 0.5) at 120 Hz
        float lowDb = (params[2] - 0.5f) * 48.0f;
        bool hasLowShelf = (std::abs(lowDb) > 0.05f);

        // 4. High Shelf: -24 dB to 0 dB to +24 dB (def 0 dB = 0.5) at 8 kHz
        float highDb = (params[3] - 0.5f) * 48.0f;
        bool hasHighShelf = (std::abs(highDb) > 0.05f);

        // Low Shelf Biquad (120 Hz)
        float lb0 = 1.0f, lb1 = 0.0f, lb2 = 0.0f, la1 = 0.0f, la2 = 0.0f;
        if (hasLowShelf) {
            float w0 = TWO_PI * 120.0f * invSr;
            float cosw0 = std::cos(w0);
            float sinw0 = std::sin(w0);
            float A = std::pow(10.0f, lowDb / 40.0f);
            float alpha = sinw0 * 0.70710678f;
            float a0 = (A + 1.0f) + (A - 1.0f) * cosw0 + 2.0f * std::sqrt(A) * alpha;
            lb0 = (A * ((A + 1.0f) - (A - 1.0f) * cosw0 + 2.0f * std::sqrt(A) * alpha)) / a0;
            lb1 = (2.0f * A * ((A - 1.0f) - (A + 1.0f) * cosw0)) / a0;
            lb2 = (A * ((A + 1.0f) - (A - 1.0f) * cosw0 - 2.0f * std::sqrt(A) * alpha)) / a0;
            la1 = (-2.0f * ((A - 1.0f) + (A + 1.0f) * cosw0)) / a0;
            la2 = ((A + 1.0f) + (A - 1.0f) * cosw0 - 2.0f * std::sqrt(A) * alpha) / a0;
        }

        // High Shelf Biquad (8 kHz)
        float hb0 = 1.0f, hb1 = 0.0f, hb2 = 0.0f, ha1 = 0.0f, ha2 = 0.0f;
        if (hasHighShelf) {
            float w0 = TWO_PI * 8000.0f * invSr;
            float cosw0 = std::cos(w0);
            float sinw0 = std::sin(w0);
            float A = std::pow(10.0f, highDb / 40.0f);
            float alpha = sinw0 * 0.70710678f;
            float a0 = (A + 1.0f) - (A - 1.0f) * cosw0 + 2.0f * std::sqrt(A) * alpha;
            hb0 = (A * ((A + 1.0f) + (A - 1.0f) * cosw0 + 2.0f * std::sqrt(A) * alpha)) / a0;
            hb1 = (-2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosw0)) / a0;
            hb2 = (A * ((A + 1.0f) + (A - 1.0f) * cosw0 - 2.0f * std::sqrt(A) * alpha)) / a0;
            ha1 = (2.0f * ((A - 1.0f) - (A + 1.0f) * cosw0)) / a0;
            ha2 = ((A + 1.0f) - (A - 1.0f) * cosw0 - 2.0f * std::sqrt(A) * alpha) / a0;
        }

        for (int i = 0; i < numSamples; ++i) {
            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            if (hasDownsample) {
                acc += phaseInc;
                if (acc >= 1.0f) {
                    acc -= 1.0f;
                    holdL = inL;
                    holdR = inR;
                }
            } else {
                holdL = inL;
                holdR = inR;
            }

            float curL = holdL;
            float curR = holdR;

            if (hasBitCrush) {
                curL = std::round(curL * steps) / steps;
                curR = std::round(curR * steps) / steps;
            }

            if (hasLowShelf) {
                float yL = lb0 * curL + lowS1L;
                lowS1L = lb1 * curL - la1 * yL + lowS2L;
                lowS2L = lb2 * curL - la2 * yL;
                curL = yL;

                float yR = lb0 * curR + lowS1R;
                lowS1R = lb1 * curR - la1 * yR + lowS2R;
                lowS2R = lb2 * curR - la2 * yR;
                curR = yR;
            }

            if (hasHighShelf) {
                float yL = hb0 * curL + highS1L;
                highS1L = hb1 * curL - ha1 * yL + highS2L;
                highS2L = hb2 * curL - ha2 * yL;
                curL = yL;

                float yR = hb0 * curR + highS1R;
                highS1R = hb1 * curR - ha1 * yR + highS2R;
                highS2R = hb2 * curR - ha2 * yR;
                curR = yR;
            }

            if (left) left[i] = curL;
            if (right) right[i] = curR;
        }
    }

private:
    float sampleRate = 44100.0f;
    float invSr = 1.0f / 44100.0f;
    float holdL = 0.0f, holdR = 0.0f;
    float acc = 0.0f;
    float lowS1L = 0.0f, lowS2L = 0.0f, lowS1R = 0.0f, lowS2R = 0.0f;
    float highS1L = 0.0f, highS2L = 0.0f, highS1R = 0.0f, highS2R = 0.0f;
};

// --- BLOCK 12: AMP (Pan, Level, Drive, Limiter) ---
class AmpBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        sampleRate = ctx.sampleRate;
    }

    void trigger(float velocity) override {
        vel = velocity;
    }

    void reset() override {
        vel = 1.0f;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& ctx) override {
        // 1. Level: 0% to 100% (def 100% = 1.0)
        float level = params[0];

        // 2. Pan: 100% L to Center to 100% R (def Center = 0.5)
        float pan = std::clamp(params[1] + ctx.slopAmpPan, 0.0f, 1.0f);
        float gainL = std::cos(pan * 1.57079632679f);
        float gainR = std::sin(pan * 1.57079632679f);

        // 3. Drive: -6dB to 0dB (at 0.5) to +24dB (def 0dB) - comes before limiter
        float driveDb = (params[2] <= 0.5f) ? (-6.0f + params[2] * 12.0f) : ((params[2] - 0.5f) * 48.0f);
        float driveGain = std::pow(10.0f, driveDb / 20.0f);
        bool hasDrive = (std::abs(driveDb) > 0.05f);

        // 4. Limiter: 0=Off, 1=On (def On) - comes after drive
        bool hasLimiter = (params[3] >= 0.5f);

        for (int i = 0; i < numSamples; ++i) {
            float envVal = (i < static_cast<int>(ctx.ampEnvSignal.size())) ? ctx.ampEnvSignal[i] : 1.0f;
            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            float effectiveGain = ctx.velVolumeGain * ctx.keyVolumeGain * level;
            float curL = inL * envVal * effectiveGain;
            float curR = inR * envVal * effectiveGain;

            if (hasDrive) {
                curL *= driveGain;
                curR *= driveGain;
            }

            if (hasLimiter) {
                curL = FastMath::fastTanh(curL);
                curR = FastMath::fastTanh(curR);
            }

            if (left)  left[i]  = curL * gainL;
            if (right) right[i] = curR * gainR;
        }
    }

private:
    float sampleRate = 44100.0f;
    float vel = 1.0f;
};

// --- BLOCK 13: AMP ENVELOPE (Claps, Clap Speed, Slope, Decay) ---
class AmpEnvelopeBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        timeSinceTrigger = 1000.0f;
    }

    void trigger(float) override {
        timeSinceTrigger = 0.0f;
    }

    void reset() override {
        timeSinceTrigger = 1000.0f;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* /*left*/, float* /*right*/, int numSamples, BlockContext& ctx) override {
        // 1. Claps: 0 to 32 (def 0)
        int numClaps = static_cast<int>(std::round(params[0] * 32.0f));

        // 2. Clap speed: 1 ms to 15 ms (def 3 ms)
        float clapDecay = 0.001f + params[1] * 0.014f;
        float clapInterval = clapDecay * 1.5f;

        // 3. Slope: Exp (0.0, def) -> Lin (0.5) -> Log (1.0)
        float slope = params[2];

        // 4. Decay: 5-point warp (5ms, 100ms, 1s, 5s, 60s; def 333ms) + slop + velocity/key modulation
        float decayParam = std::clamp(params[3] + ctx.slopAmpEnvDecay + ctx.velDecayMod + ctx.keyDecayMod, 0.0f, 1.0f);
        float decayTime = warp5PointTime(decayParam);
        decayTime = std::max(decayTime, 0.001f);

        float clapBurstsDuration = (numClaps > 0) ? (numClaps * clapInterval) : 0.0f;

        int count = std::min(numSamples, static_cast<int>(ctx.ampEnvSignal.size()));
        for (int i = 0; i < count; ++i) {
            float envVal = 0.0f;
            if (numClaps > 0 && timeSinceTrigger < clapBurstsDuration) {
                float burstIdx = std::floor(timeSinceTrigger / clapInterval);
                float burstTime = timeSinceTrigger - burstIdx * clapInterval;
                float burstEnv = (burstTime < 0.0005f) ? (burstTime / 0.0005f)
                                                       : std::exp(-(burstTime - 0.0005f) / clapDecay);
                envVal = burstEnv * 0.95f;
            } else {
                float tailTime = (numClaps > 0) ? (timeSinceTrigger - clapBurstsDuration) : timeSinceTrigger;
                float linearProgress = tailTime / decayTime;
                float envLinear = std::clamp(1.0f - linearProgress, 0.0f, 1.0f);
                envVal = applyEnvelopeSlope(envLinear, slope);
            }

            timeSinceTrigger += invSr;
            ctx.ampEnvSignal[i] = envVal;
        }
    }

private:
    float invSr = 1.0f / 44100.0f;
    float timeSinceTrigger = 1000.0f;
};

// --- BLOCK 16: VELOCITY (Slope, Depth, Decay, Volume) ---
class VelocityBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        sampleRate = ctx.sampleRate;
        invSr = ctx.invSr;
        for (int i = 0; i < 128; ++i) scopeData[i] = 0.0f;
    }

    void trigger(float velocity) override {
        lastVelocity = velocity;
    }

    void reset() override {
        lastVelocity = 1.0f;
        std::fill(std::begin(scopeData), std::end(scopeData), 0.0f);
    }

    void setCurve(VelocityCurve curve) { velocityTracker.setCurve(curve); }
    VelocityCurve getCurve() const { return velocityTracker.getCurve(); }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* /*left*/, float* /*right*/, int /*numSamples*/, BlockContext& ctx) override {
        // Parameters:
        // 0: slope: 0.0 Exp -> 0.5 Lin (def) -> 1.0 Log
        // 1: depth: -100% (0.0) to 0% (0.5, def) to +100% (1.0)
        // 2: decay: -100% (0.0) to 0% (0.5, def) to +100% (1.0)
        // 3: volume: 0% (0.0, def) to -100% (1.0)

        float slope = params[0];
        for (int i = 0; i < 128; ++i) {
            float t = static_cast<float>(i) / 127.0f;
            float curved = velocityTracker.evaluate(t);
            float y = (velocityTracker.getCurve() == VelocityCurve::Fixed127) ? 1.0f : applyEnvelopeSlope(curved, slope);
            // Highlight current velocity hit with a subtle blip marker
            if (std::abs(t - ctx.curvedVelocity) < 0.045f) {
                y = std::clamp(y + 0.3f, 0.0f, 1.0f);
            }
            scopeData[i] = y * 1.8f - 0.9f;
        }
    }

    void getVisualScopeData(float* dest, int numSamples) const {
        for (int i = 0; i < numSamples; ++i) {
            int idx = (i * 128) / numSamples;
            dest[i] = scopeData[std::clamp(idx, 0, 127)];
        }
    }

private:
    float sampleRate = 44100.0f;
    float invSr = 1.0f / 44100.0f;
    float lastVelocity = 1.0f;
    float scopeData[128] = { 0.0f };
    VelocityTracker velocityTracker;
};

// --- KEY TRACKING (Slope, Depth, Decay, Volume) ---
class KeyTrackingBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        sampleRate = ctx.sampleRate;
        invSr = ctx.invSr;
        for (int i = 0; i < 128; ++i) scopeData[i] = 0.0f;
    }

    void trigger(float /*velocity*/) override {}

    void reset() override {
        std::fill(std::begin(scopeData), std::end(scopeData), 0.0f);
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* /*left*/, float* /*right*/, int /*numSamples*/, BlockContext& ctx) override {
        // Parameters:
        // 0: slope: 0.0 Exp -> 0.5 Lin (def) -> 1.0 Log
        // 1: depth: -100% (0.0) to 0% (0.5, def) to +100% (1.0)
        // 2: decay: -100% (0.0) to 0% (0.5, def) to +100% (1.0)
        // 3: volume: 0% (0.0, def) to -100% (1.0)

        float slope = params[0];
        for (int i = 0; i < 128; ++i) {
            float t = static_cast<float>(i) / 127.0f;
            float y = applyEnvelopeSlope(t, slope);
            // Highlight current note with a subtle blip marker
            if (std::abs(i - ctx.currentMidiNote) <= 2) {
                y = std::clamp(y + 0.3f, 0.0f, 1.0f);
            }
            scopeData[i] = y * 1.8f - 0.9f;
        }
    }

    void getVisualScopeData(float* dest, int numSamples) const {
        for (int i = 0; i < numSamples; ++i) {
            int idx = (i * 128) / numSamples;
            dest[i] = scopeData[std::clamp(idx, 0, 127)];
        }
    }

private:
    float sampleRate = 44100.0f;
    float invSr = 1.0f / 44100.0f;
    float scopeData[128] = { 0.0f };
};

// --- MODULATION ENVELOPE (Slope, Depth, Decay, Target) ---
class ModEnvelopeBlock : public DSPBlock {
public:
    explicit ModEnvelopeBlock(int envIndex = 1) : index(envIndex) {}

    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        ctxPtr = &ctx;
        timeSinceTrigger = 1000.0f;
        currentVal = 0.0f;
    }

    void trigger(float) override {
        timeSinceTrigger = 0.0f;
        float slope = params[0];
        float baseDepth = (params[1] - 0.5f) * 2.0f;
        float velMod = ctxPtr ? ctxPtr->velDepthMod : 0.0f;
        float keyMod = ctxPtr ? ctxPtr->keyDepthMod : 0.0f;
        float depth = std::clamp(baseDepth + velMod + keyMod, -1.0f, 1.0f);
        currentVal = applyEnvelopeSlope(1.0f, slope) * depth;
    }

    void reset() override {
        timeSinceTrigger = 1000.0f;
        currentVal = 0.0f;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* /*left*/, float* /*right*/, int numSamples, BlockContext& ctx) override {
        ctxPtr = &ctx;
        float slope = params[0];
        float baseDepth = (params[1] - 0.5f) * 2.0f;
        float depth = std::clamp(baseDepth + ctx.velDepthMod + ctx.keyDepthMod, -1.0f, 1.0f);

        float decayParam = std::clamp(params[2] + ctx.velDecayMod + ctx.keyDecayMod, 0.0f, 1.0f);
        float decayTime = warp5PointTime(decayParam);
        decayTime = std::max(decayTime, 0.001f);

        std::vector<float>& envSig = (index == 1) ? ctx.modEnv1Signal :
                                     ((index == 2) ? ctx.modEnv2Signal : ctx.modEnv3Signal);

        int count = std::min(numSamples, static_cast<int>(envSig.size()));
        for (int i = 0; i < count; ++i) {
            float linearProgress = timeSinceTrigger / decayTime;
            float envLinear = std::clamp(1.0f - linearProgress, 0.0f, 1.0f);
            float envVal = applyEnvelopeSlope(envLinear, slope) * depth;
            timeSinceTrigger += invSr;
            envSig[i] = envVal;
            currentVal = envVal;
        }
    }

    float getCurrentValue() const { return currentVal; }

private:
    int index = 1;
    float invSr = 1.0f / 44100.0f;
    const BlockContext* ctxPtr = nullptr;
    float timeSinceTrigger = 1000.0f;
    float currentVal = 0.0f;
};

// --- BLOCK 17: SLOP (Frequency, Depth, Decay, Pan) ---
class SlopBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        sampleRate = ctx.sampleRate;
        invSr = ctx.invSr;
        for (int i = 0; i < 128; ++i) scopeData[i] = 0.0f;
    }

    void trigger(float /*velocity*/) override {}

    void reset() override {
        std::fill(std::begin(scopeData), std::end(scopeData), 0.0f);
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* /*left*/, float* /*right*/, int /*numSamples*/, BlockContext& ctx) override {
        // 4 categories across 128 scope samples:
        // Category 0 (0..31): Frequency slop
        // Category 1 (32..63): Depth slop
        // Category 2 (64..95): Decay slop
        // Category 3 (96..127): Pan slop
        float currentOffsets[4] = {
            ctx.slopFilter1Cutoff,
            ctx.slopPitchEnv1Depth,
            ctx.slopAmpEnvDecay,
            ctx.slopAmpPan
        };
        float maxBounds[4] = {
            warpUnipolarExp(params[0]),
            warpUnipolarExp(params[1]),
            warpUnipolarExp(params[2]),
            warpUnipolarExp(params[3])
        };

        for (int i = 0; i < 128; ++i) {
            int cat = std::clamp(i / 32, 0, 3);
            int localI = i % 32;
            float val = 0.0f;
            if (localI >= 4 && localI <= 27) {
                val = std::clamp(currentOffsets[cat] * 0.9f, -0.9f, 0.9f);
            }
            if (localI == 2 || localI == 29) {
                val = std::clamp(maxBounds[cat] * 0.9f, -0.9f, 0.9f);
            }
            scopeData[i] = val;
        }
    }

    void getVisualScopeData(float* dest, int numSamples) const {
        for (int i = 0; i < numSamples; ++i) {
            int idx = (i * 128) / numSamples;
            dest[i] = scopeData[std::clamp(idx, 0, 127)];
        }
    }

private:
    float sampleRate = 44100.0f;
    float invSr = 1.0f / 44100.0f;
    float scopeData[128] = { 0.0f };
};

// --- BLOCK: LIMITER (Enable, Input Gain, Threshold, Release Time) ---
class LimiterBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        envelopeL = 0.0f;
        envelopeR = 0.0f;
        activity.store(0.0f, std::memory_order_relaxed);
    }

    void trigger(float) override {
        // Continuous dynamics tracking
    }

    void reset() override {
        envelopeL = 0.0f;
        envelopeR = 0.0f;
        activity.store(0.0f, std::memory_order_relaxed);
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& /*ctx*/) override {
        // 0. Enable: Off (0) / On (1)
        bool enabled = (params[0] >= 0.5f);
        if (!enabled) return;

        // 1. Input Gain: -12 dB to +24 dB (def 0 dB at 12/36 = 0.33333f)
        float inGainDb = -12.0f + params[1] * 36.0f;
        float inGainLin = std::pow(10.0f, inGainDb / 20.0f);

        // 2. Threshold: -24 dB to 0 dB (def 0 dB at 1.0f)
        float threshDb = -24.0f + params[2] * 24.0f;
        float threshLin = std::pow(10.0f, threshDb / 20.0f);

        // 3. Release Time: 1 ms to 500 ms (def 50 ms at 0.6296f)
        float relMs = 1.0f * std::pow(500.0f, params[3]);
        float relSec = relMs * 0.001f;
        float attackSec = 0.0005f; // 0.5 ms fast lookahead/attack
        float attackCoeff = std::exp(-invSr / attackSec);
        float releaseCoeff = std::exp(-invSr / relSec);

        float currentActivity = 0.0f;

        for (int i = 0; i < numSamples; ++i) {
            float inL = (left ? left[i] : 0.0f) * inGainLin;
            float inR = (right ? right[i] : inL) * inGainLin;

            float peakL = std::abs(inL);
            float peakR = std::abs(inR);

            if (peakL > envelopeL)
                envelopeL = attackCoeff * envelopeL + (1.0f - attackCoeff) * peakL;
            else
                envelopeL = releaseCoeff * envelopeL + (1.0f - releaseCoeff) * peakL;

            if (peakR > envelopeR)
                envelopeR = attackCoeff * envelopeR + (1.0f - attackCoeff) * peakR;
            else
                envelopeR = releaseCoeff * envelopeR + (1.0f - releaseCoeff) * peakR;

            float maxEnv = std::max(envelopeL, envelopeR);
            float gainReduction = 1.0f;
            if (maxEnv > threshLin && threshLin > 1e-5f) {
                gainReduction = threshLin / maxEnv;
            }

            currentActivity = std::max(currentActivity, 1.0f - gainReduction);

            float outL = inL * gainReduction;
            float outR = inR * gainReduction;

            float clampLimit = std::max(threshLin, 1.0f);
            outL = std::clamp(outL, -clampLimit, clampLimit);
            outR = std::clamp(outR, -clampLimit, clampLimit);

            if (left) left[i] = outL;
            if (right) right[i] = outR;
        }

        float curAct = activity.load(std::memory_order_relaxed);
        float newAct = std::max(currentActivity, curAct * 0.90f);
        activity.store(newAct, std::memory_order_relaxed);
    }

private:
    float invSr = 1.0f / 44100.0f;
    float envelopeL = 0.0f;
    float envelopeR = 0.0f;
    std::atomic<float> activity {0.0f};

public:
    float getActivity() const { return activity.load(std::memory_order_relaxed); }
};

// --- MODULAR DRUM ENGINE ---
class ModularDrumEngine {
public:
    enum BlockID {
        BLK_CARRIER1 = 0,
        BLK_MODULATOR1,
        BLK_PITCHENV1,
        BLK_FILTER1,
        BLK_FILTERENV1,
        BLK_CARRIER2,
        BLK_MODULATOR2,
        BLK_PITCHENV2,
        BLK_FILTER2,
        BLK_FILTERENV2,
        BLK_NOISE,
        BLK_FILTER3,
        BLK_FILTERENV3,
        BLK_MIXER,
        BLK_AMP,
        BLK_AMPENV,
        BLK_PRE_LIMITER,
        BLK_POST_LIMITER,
        BLK_VELOCITY,
        BLK_KEYTRACK,
        BLK_SLOP,
        BLK_MODENV1,
        BLK_MODENV2,
        BLK_MODENV3,
        BLK_PRE_FX_1,
        BLK_PRE_FX_2,
        BLK_PRE_FX_3,
        BLK_PRE_FX_4,
        BLK_POST_FX_1,
        BLK_POST_FX_2,
        BLK_POST_FX_3,
        BLK_POST_FX_4,
        NUM_BLOCKS
    };

    static std::unique_ptr<DSPBlock> createFXBlock(int type) {
        switch (type) {
            case 1:  return std::make_unique<EQBlock>();               // Bell EQ
            case 2:  return std::make_unique<ChorusBlock>();           // Chorus
            case 3:  return std::make_unique<CombFilterBlock>();       // Comb Filter
            case 4:  return std::make_unique<DriveBlock>();            // Drive
            case 5:  return std::make_unique<FilterBlock>(0);          // Filter
            case 6:  return std::make_unique<FlangerBlock>();          // Flanger
            case 7:  return std::make_unique<FrequencyShifterBlock>(); // Frequency Shifter
            case 8:  return std::make_unique<GritBlock>();             // Grit FX
            case 9:  return std::make_unique<PhaseSmearBlock>();        // Phase Smear
            case 10: return std::make_unique<PhaserBlock>();           // Phaser
            case 11: return std::make_unique<RingModBlock>();          // RingMod
            case 12: return std::make_unique<DelayBlock>();            // Tempo Delay
            case 13: return std::make_unique<WaveFolderBlock>();       // Wave Folder
            default: return nullptr;
        }
    }

    ModularDrumEngine() {
        init(44100.0f);
    }

    void init(float sampleRate, int maxBlockSize = 1024) {
        ctx.sampleRate = sampleRate;
        ctx.invSr = 1.0f / sampleRate;

        // Instantiate all blocks
        allBlocks.resize(NUM_BLOCKS);
        allBlocks[BLK_CARRIER1]   = std::make_unique<CarrierBlock>(1);
        allBlocks[BLK_MODULATOR1] = std::make_unique<ModulatorBlock>(1);
        allBlocks[BLK_PITCHENV1]  = std::make_unique<PitchEnvelopeBlock>(1);
        allBlocks[BLK_FILTER1]    = std::make_unique<FilterBlock>(1);
        allBlocks[BLK_FILTERENV1] = std::make_unique<FilterEnvelopeBlock>(1);

        allBlocks[BLK_CARRIER2]   = std::make_unique<CarrierBlock>(2);
        allBlocks[BLK_MODULATOR2] = std::make_unique<ModulatorBlock>(2);
        allBlocks[BLK_PITCHENV2]  = std::make_unique<PitchEnvelopeBlock>(2);
        allBlocks[BLK_FILTER2]    = std::make_unique<FilterBlock>(2);
        allBlocks[BLK_FILTERENV2] = std::make_unique<FilterEnvelopeBlock>(2);

        allBlocks[BLK_NOISE]      = std::make_unique<NoiseTransientBlock>();
        allBlocks[BLK_FILTER3]    = std::make_unique<FilterBlock>(3);
        allBlocks[BLK_FILTERENV3] = std::make_unique<FilterEnvelopeBlock>(3);

        allBlocks[BLK_MIXER]      = std::make_unique<MixerBlock>();
        allBlocks[BLK_AMP]        = std::make_unique<AmpBlock>();
        allBlocks[BLK_AMPENV]     = std::make_unique<AmpEnvelopeBlock>();

        allBlocks[BLK_PRE_LIMITER]  = std::make_unique<LimiterBlock>();
        allBlocks[BLK_POST_LIMITER] = std::make_unique<LimiterBlock>();

        allBlocks[BLK_VELOCITY]   = std::make_unique<VelocityBlock>();
        allBlocks[BLK_KEYTRACK]   = std::make_unique<KeyTrackingBlock>();
        allBlocks[BLK_SLOP]       = std::make_unique<SlopBlock>();
        allBlocks[BLK_MODENV1]    = std::make_unique<ModEnvelopeBlock>(1);
        allBlocks[BLK_MODENV2]    = std::make_unique<ModEnvelopeBlock>(2);
        allBlocks[BLK_MODENV3]    = std::make_unique<ModEnvelopeBlock>(3);

        for (auto& b : allBlocks) {
            if (b) b->init(ctx);
        }

        // Pre-allocate realtime working buffers
        int bufSize = std::max(1024, maxBlockSize);
        tempNoiseL.assign(bufSize, 0.0f);
        tempNoiseR.assign(bufSize, 0.0f);
        tempCarrier1L.assign(bufSize, 0.0f);
        tempCarrier1R.assign(bufSize, 0.0f);
        tempCarrier2L.assign(bufSize, 0.0f);
        tempCarrier2R.assign(bufSize, 0.0f);
        ctx.mod1Signal.assign(bufSize, 0.0f);
        ctx.mod2Signal.assign(bufSize, 0.0f);
        ctx.pitchEnv1Signal.assign(bufSize, 0.0f);
        ctx.pitchEnv2Signal.assign(bufSize, 0.0f);
        ctx.filterEnv1Signal.assign(bufSize, 0.0f);
        ctx.filterEnv2Signal.assign(bufSize, 0.0f);
        ctx.filterEnv3Signal.assign(bufSize, 0.0f);
        ctx.ampEnvSignal.assign(bufSize, 1.0f);
        ctx.modEnv1Signal.assign(bufSize, 0.0f);
        ctx.modEnv2Signal.assign(bufSize, 0.0f);
        ctx.modEnv3Signal.assign(bufSize, 0.0f);

        // Voice 1 defaults
        setPageParameter(BLK_CARRIER1, 0, 0.0f);
        setPageParameter(BLK_CARRIER1, 1, 0.5f);
        setPageParameter(BLK_CARRIER1, 2, 0.0f);
        setPageParameter(BLK_CARRIER1, 3, 0.5f);

        setPageParameter(BLK_MODULATOR1, 0, 0.0f);
        setPageParameter(BLK_MODULATOR1, 1, 0.0f);
        setPageParameter(BLK_MODULATOR1, 2, 0.0f);
        setPageParameter(BLK_MODULATOR1, 3, 0.50934f);

        setPageParameter(BLK_PITCHENV1, 0, 0.5886f); // Slope
        setPageParameter(BLK_PITCHENV1, 1, 0.5f);    // Depth
        setPageParameter(BLK_PITCHENV1, 2, 0.3806f); // Decay
        setPageParameter(BLK_PITCHENV1, 3, 0.0f);    // Target

        setPageParameter(BLK_FILTER1, 0, 0.0f);
        setPageParameter(BLK_FILTER1, 1, 0.25f);
        setPageParameter(BLK_FILTER1, 2, 1.0f);
        setPageParameter(BLK_FILTER1, 3, 0.0f);

        setPageParameter(BLK_FILTERENV1, 0, 0.5886f); // Slope
        setPageParameter(BLK_FILTERENV1, 1, 0.5f);
        setPageParameter(BLK_FILTERENV1, 2, 0.3806f);
        setPageParameter(BLK_FILTERENV1, 3, 0.5f);

        // Voice 2 defaults
        setPageParameter(BLK_CARRIER2, 0, 0.0f);
        setPageParameter(BLK_CARRIER2, 1, 0.5f);
        setPageParameter(BLK_CARRIER2, 2, 0.0f);
        setPageParameter(BLK_CARRIER2, 3, 0.5f);

        setPageParameter(BLK_MODULATOR2, 0, 0.0f);
        setPageParameter(BLK_MODULATOR2, 1, 0.0f);
        setPageParameter(BLK_MODULATOR2, 2, 0.0f);
        setPageParameter(BLK_MODULATOR2, 3, 0.50934f);

        setPageParameter(BLK_PITCHENV2, 0, 0.5886f); // Slope
        setPageParameter(BLK_PITCHENV2, 1, 0.5f);    // Depth
        setPageParameter(BLK_PITCHENV2, 2, 0.3806f); // Decay
        setPageParameter(BLK_PITCHENV2, 3, 0.0f);    // Target

        setPageParameter(BLK_FILTER2, 0, 0.0f);
        setPageParameter(BLK_FILTER2, 1, 0.25f);
        setPageParameter(BLK_FILTER2, 2, 1.0f);
        setPageParameter(BLK_FILTER2, 3, 0.0f);

        setPageParameter(BLK_FILTERENV2, 0, 0.5886f); // Slope
        setPageParameter(BLK_FILTERENV2, 1, 0.5f);
        setPageParameter(BLK_FILTERENV2, 2, 0.3806f);
        setPageParameter(BLK_FILTERENV2, 3, 0.5f);

        // Transients defaults
        setPageParameter(BLK_NOISE, 0, 1.0f);
        setPageParameter(BLK_NOISE, 1, 0.5f);
        setPageParameter(BLK_NOISE, 2, 0.5f);
        setPageParameter(BLK_NOISE, 3, 0.3078f);

        setPageParameter(BLK_FILTER3, 0, 0.0f);
        setPageParameter(BLK_FILTER3, 1, 0.25f);
        setPageParameter(BLK_FILTER3, 2, 1.0f);
        setPageParameter(BLK_FILTER3, 3, 0.0f);

        setPageParameter(BLK_FILTERENV3, 0, 0.5886f); // Slope
        setPageParameter(BLK_FILTERENV3, 1, 0.5f);
        setPageParameter(BLK_FILTERENV3, 2, 0.3078f);
        setPageParameter(BLK_FILTERENV3, 3, 0.5f);

        // Mixer defaults
        setPageParameter(BLK_MIXER, 0, 0.5f);
        setPageParameter(BLK_MIXER, 1, 0.0f);
        setPageParameter(BLK_MIXER, 2, 0.0f);
        setPageParameter(BLK_MIXER, 3, 0.0f);

        // FX defaults
        setPageParameter(BLK_AMP, 0, 1.0f);
        setPageParameter(BLK_AMP, 1, 0.5f);
        setPageParameter(BLK_AMP, 2, 0.5f);
        setPageParameter(BLK_AMP, 3, 1.0f);

        setPageParameter(BLK_AMPENV, 0, 0.0f);
        setPageParameter(BLK_AMPENV, 1, 0.1429f);
        setPageParameter(BLK_AMPENV, 2, 0.5886f); // Slope
        setPageParameter(BLK_AMPENV, 3, 0.3806f);

        // Limiters defaults
        setPageParameter(BLK_PRE_LIMITER, 0, 1.0f);
        setPageParameter(BLK_PRE_LIMITER, 1, 12.0f / 36.0f);
        setPageParameter(BLK_PRE_LIMITER, 2, 1.0f);
        setPageParameter(BLK_PRE_LIMITER, 3, 0.6296f);

        setPageParameter(BLK_POST_LIMITER, 0, 1.0f);
        setPageParameter(BLK_POST_LIMITER, 1, 12.0f / 36.0f);
        setPageParameter(BLK_POST_LIMITER, 2, 1.0f);
        setPageParameter(BLK_POST_LIMITER, 3, 0.6296f);

        // Modulations defaults
        setPageParameter(BLK_VELOCITY, 0, 0.5886f); // Slope
        setPageParameter(BLK_VELOCITY, 1, 0.5f);    // Depth
        setPageParameter(BLK_VELOCITY, 2, 0.5f);    // Decay
        setPageParameter(BLK_VELOCITY, 3, 0.0f);    // Volume

        setPageParameter(BLK_KEYTRACK, 0, 0.5886f); // Slope
        setPageParameter(BLK_KEYTRACK, 1, 0.5f);    // Depth
        setPageParameter(BLK_KEYTRACK, 2, 0.5f);    // Decay
        setPageParameter(BLK_KEYTRACK, 3, 0.0f);    // Volume

        setPageParameter(BLK_SLOP, 0, 0.0f); // Freq
        setPageParameter(BLK_SLOP, 1, 0.0f); // Depth
        setPageParameter(BLK_SLOP, 2, 0.0f); // Decay
        setPageParameter(BLK_SLOP, 3, 0.0f); // Pan

        setPageParameter(BLK_MODENV1, 0, 0.5886f); // Slope
        setPageParameter(BLK_MODENV1, 1, 0.5f);    // Depth
        setPageParameter(BLK_MODENV1, 2, 0.3806f); // Decay
        setPageParameter(BLK_MODENV1, 3, 0.0f);    // Target

        setPageParameter(BLK_MODENV2, 0, 0.5886f); // Slope
        setPageParameter(BLK_MODENV2, 1, 0.5f);    // Depth
        setPageParameter(BLK_MODENV2, 2, 0.3806f); // Decay
        setPageParameter(BLK_MODENV2, 3, 0.0f);    // Target

        setPageParameter(BLK_MODENV3, 0, 0.5886f); // Slope
        setPageParameter(BLK_MODENV3, 1, 0.5f);    // Depth
        setPageParameter(BLK_MODENV3, 2, 0.3806f); // Decay
        setPageParameter(BLK_MODENV3, 3, 0.0f);    // Target

        // Pre-Amp FX defaults
        preFXTypes[0] = 4; preFXParams[0][0] = 0.4f; preFXParams[0][1] = 0.5f; preFXParams[0][2] = 0.5f; preFXParams[0][3] = 1.0f; // Drive: +6dB (0.4)
        preFXTypes[1] = 13; preFXParams[1][0] = 0.0f; preFXParams[1][1] = 0.0f; preFXParams[1][2] = 0.5f; preFXParams[1][3] = 0.5f; // WaveFolder
        preFXTypes[2] = 11; preFXParams[2][0] = 0.0f; preFXParams[2][1] = 0.50934f; preFXParams[2][2] = 0.0f; preFXParams[2][3] = 0.5f; // RingMod
        preFXTypes[3] = 7; preFXParams[3][0] = 0.5f; preFXParams[3][1] = rangeHzToNorm(3.0f); preFXParams[3][2] = 0.75f; preFXParams[3][3] = 0.5f; // FreqShift: +50%:50% (0.75)

        // Post-Amp FX defaults
        postFXTypes[0] = 8; postFXParams[0][0] = 1.0f; postFXParams[0][1] = 1.0f; postFXParams[0][2] = 0.5f; postFXParams[0][3] = 0.5f; // Grit
        postFXTypes[1] = 3; postFXParams[1][0] = 1.0f; postFXParams[1][1] = 1.0f; postFXParams[1][2] = 0.5f; postFXParams[1][3] = 0.75f; // Comb: Damp 24k, Cut 24k, Res 0%, Mix +50%:50%
        postFXTypes[2] = 9; postFXParams[2][0] = 0.0f; postFXParams[2][1] = 4.0f / 32.0f; postFXParams[2][2] = 0.62124f; postFXParams[2][3] = 0.5f; // Phase Smear: 2nd Order (0.0)
        postFXTypes[3] = 1; postFXParams[3][0] = 1.0f; postFXParams[3][1] = 0.0f; postFXParams[3][2] = 0.5f; postFXParams[3][3] = 0.5f; // Bell EQ

        for (int s = 0; s < 4; ++s) {
            preFXBlocks[s] = createFXBlock(preFXTypes[s]);
            if (preFXBlocks[s]) {
                preFXBlocks[s]->init(ctx);
                for (int p = 0; p < 4; ++p) preFXBlocks[s]->setParam(p, preFXParams[s][p]);
            }
            postFXBlocks[s] = createFXBlock(postFXTypes[s]);
            if (postFXBlocks[s]) {
                postFXBlocks[s]->init(ctx);
                for (int p = 0; p < 4; ++p) postFXBlocks[s]->setParam(p, postFXParams[s][p]);
            }
        }
    }

    void setVelocityCurve(VelocityCurve curve) {
        velocityTracker.setCurve(curve);
        if (allBlocks[BLK_VELOCITY]) {
            if (auto* vb = dynamic_cast<VelocityBlock*>(allBlocks[BLK_VELOCITY].get())) {
                vb->setCurve(curve);
            }
        }
    }

    VelocityCurve getVelocityCurve() const {
        return velocityTracker.getCurve();
    }

    void trigger(float velocity = 1.0f) {
        velocity = std::clamp(velocity, 0.0f, 1.0f);
        ctx.triggerVelocity = velocity;
        ctx.isTriggered = true;

        float velSlope  = allBlocks[BLK_VELOCITY] ? allBlocks[BLK_VELOCITY]->getParam(0) : 0.0f;
        float velDepth  = allBlocks[BLK_VELOCITY] ? warpBipolarExp(allBlocks[BLK_VELOCITY]->getParam(1)) : 0.0f; // -1 to +1
        float velDecay  = allBlocks[BLK_VELOCITY] ? warpBipolarExp(allBlocks[BLK_VELOCITY]->getParam(2)) : 0.0f; // -1 to +1
        float velVolume = allBlocks[BLK_VELOCITY] ? warpUnipolarExp(allBlocks[BLK_VELOCITY]->getParam(3)) : 0.0f; // 0 to 1

        float curvedVel = velocityTracker.evaluate(velocity);
        if (velocityTracker.getCurve() != VelocityCurve::Fixed127) {
            curvedVel = applyEnvelopeSlope(curvedVel, velSlope);
        }
        ctx.curvedVelocity = curvedVel;

        // Minimum output volume at lowest velocity: (1.0 - velVolume), up to 1.0 at max velocity
        ctx.velVolumeGain = 1.0f - (1.0f - curvedVel) * velVolume;

        // Bipolar velocity factor: -1.0 at min velocity, 0.0 at mid, +1.0 at max velocity
        float velModFactor = curvedVel * 2.0f - 1.0f;
        ctx.velDecayMod   = velModFactor * velDecay;
        ctx.velDepthMod   = velModFactor * velDepth;

        // Key tracking calculation:
        float keySlope  = allBlocks[BLK_KEYTRACK] ? allBlocks[BLK_KEYTRACK]->getParam(0) : 0.0f;
        float keyDepth  = allBlocks[BLK_KEYTRACK] ? warpBipolarExp(allBlocks[BLK_KEYTRACK]->getParam(1)) : 0.0f;
        float keyDecay  = allBlocks[BLK_KEYTRACK] ? warpBipolarExp(allBlocks[BLK_KEYTRACK]->getParam(2)) : 0.0f;
        float keyVolume = allBlocks[BLK_KEYTRACK] ? warpUnipolarExp(allBlocks[BLK_KEYTRACK]->getParam(3)) : 0.0f;

        float normKey = std::clamp(static_cast<float>(ctx.currentMidiNote) / 127.0f, 0.0f, 1.0f);
        float curvedKey = applyEnvelopeSlope(normKey, keySlope);
        ctx.curvedKeyNote = curvedKey;
        ctx.keyVolumeGain = 1.0f - (1.0f - curvedKey) * keyVolume;
        float keyModFactor = curvedKey * 2.0f - 1.0f;
        ctx.keyDecayMod   = keyModFactor * keyDecay;
        ctx.keyDepthMod   = keyModFactor * keyDepth;

        // Sample independent stepped random offsets for Slop
        float slopFreq  = allBlocks[BLK_SLOP] ? warpUnipolarExp(allBlocks[BLK_SLOP]->getParam(0)) : 0.0f;
        float slopDepth = allBlocks[BLK_SLOP] ? warpUnipolarExp(allBlocks[BLK_SLOP]->getParam(1)) : 0.0f;
        float slopDecay = allBlocks[BLK_SLOP] ? warpUnipolarExp(allBlocks[BLK_SLOP]->getParam(2)) : 0.0f;
        float slopPan   = allBlocks[BLK_SLOP] ? warpUnipolarExp(allBlocks[BLK_SLOP]->getParam(3)) : 0.0f;

        ctx.slopCarrier1Pitch   = slopFreq  * fastRng(slopRngState);
        ctx.slopCarrier2Pitch   = slopFreq  * fastRng(slopRngState);
        ctx.slopMod1Freq        = slopFreq  * fastRng(slopRngState);
        ctx.slopMod1Filter      = slopFreq  * fastRng(slopRngState);
        ctx.slopMod2Freq        = slopFreq  * fastRng(slopRngState);
        ctx.slopMod2Filter      = slopFreq  * fastRng(slopRngState);
        ctx.slopDriveFilter     = slopFreq  * fastRng(slopRngState);
        ctx.slopWaveFolderFilter= slopFreq  * fastRng(slopRngState);
        ctx.slopNoiseShRate     = slopFreq  * fastRng(slopRngState);
        ctx.slopNoiseFilter     = slopFreq  * fastRng(slopRngState);
        ctx.slopFilter1Cutoff   = slopFreq  * fastRng(slopRngState);
        ctx.slopFilter2Cutoff   = slopFreq  * fastRng(slopRngState);
        ctx.slopFilter3Cutoff   = slopFreq  * fastRng(slopRngState);
        ctx.slopFXFilterCutoff  = slopFreq  * fastRng(slopRngState);
        ctx.slopRingModRate     = slopFreq  * fastRng(slopRngState);
        ctx.slopCombDamp        = slopFreq  * fastRng(slopRngState);
        ctx.slopCombCutoff      = slopFreq  * fastRng(slopRngState);
        ctx.slopPhaseSmearCutoff = slopFreq  * fastRng(slopRngState);
        ctx.slopEQFreq          = slopFreq  * fastRng(slopRngState);
        ctx.slopEQFilter        = slopFreq  * fastRng(slopRngState);

        ctx.slopPitchEnv1Depth  = slopDepth * fastRng(slopRngState);
        ctx.slopPitchEnv2Depth  = slopDepth * fastRng(slopRngState);
        ctx.slopFilterEnv1Depth = slopDepth * fastRng(slopRngState);
        ctx.slopFilterEnv2Depth = slopDepth * fastRng(slopRngState);
        ctx.slopFilterEnv3Depth = slopDepth * fastRng(slopRngState);

        ctx.slopPitchEnv1Decay  = slopDecay * fastRng(slopRngState);
        ctx.slopPitchEnv2Decay  = slopDecay * fastRng(slopRngState);
        ctx.slopNoiseDecay      = slopDecay * fastRng(slopRngState);
        ctx.slopFilterEnv1Decay = slopDecay * fastRng(slopRngState);
        ctx.slopFilterEnv2Decay = slopDecay * fastRng(slopRngState);
        ctx.slopFilterEnv3Decay = slopDecay * fastRng(slopRngState);
        ctx.slopAmpEnvDecay     = slopDecay * fastRng(slopRngState);

        ctx.slopAmpPan          = slopPan   * fastRng(slopRngState);

        for (auto& b : allBlocks) {
            if (b) b->trigger(velocity);
        }
        for (int s = 0; s < 4; ++s) {
            if (preFXBlocks[s]) preFXBlocks[s]->trigger(velocity);
            if (postFXBlocks[s]) postFXBlocks[s]->trigger(velocity);
        }
    }

    void setMidiPitch(int noteNumber) {
        ctx.currentMidiNote = noteNumber;
        ctx.currentPitchHz = 440.0f * FastMath::fastPow2((static_cast<float>(noteNumber) - 69.0f) / 12.0f);
    }

    void setPageParameter(BlockID block, int knobIndex, float value) {
        if (block >= BLK_PRE_FX_1 && block <= BLK_PRE_FX_4) {
            setPreFXParam(block - BLK_PRE_FX_1, knobIndex, value);
            return;
        }
        if (block >= BLK_POST_FX_1 && block <= BLK_POST_FX_4) {
            setPostFXParam(block - BLK_POST_FX_1, knobIndex, value);
            return;
        }
        if (block < NUM_BLOCKS && allBlocks[block]) {
            allBlocks[block]->setParam(knobIndex, value);
        }
    }

    void setPageParameterModulation(BlockID block, int knobIndex, float offset) {
        if (block >= BLK_PRE_FX_1 && block <= BLK_PRE_FX_4) {
            if (preFXBlocks[block - BLK_PRE_FX_1]) {
                preFXBlocks[block - BLK_PRE_FX_1]->setParamModulation(knobIndex, offset);
            }
            return;
        }
        if (block >= BLK_POST_FX_1 && block <= BLK_POST_FX_4) {
            if (postFXBlocks[block - BLK_POST_FX_1]) {
                postFXBlocks[block - BLK_POST_FX_1]->setParamModulation(knobIndex, offset);
            }
            return;
        }
        if (block < NUM_BLOCKS && allBlocks[block]) {
            allBlocks[block]->setParamModulation(knobIndex, offset);
        }
    }

    float getPageParameter(BlockID block, int knobIndex) const {
        if (block >= BLK_PRE_FX_1 && block <= BLK_PRE_FX_4) {
            return getPreFXParam(block - BLK_PRE_FX_1, knobIndex);
        }
        if (block >= BLK_POST_FX_1 && block <= BLK_POST_FX_4) {
            return getPostFXParam(block - BLK_POST_FX_1, knobIndex);
        }
        if (block < NUM_BLOCKS && allBlocks[block]) {
            return allBlocks[block]->getParam(knobIndex);
        }
        return 0.0f;
    }

    void setPreFXType(int slot, int type) {
        if (slot >= 0 && slot < 4) {
            type = std::clamp(type, 0, 13);
            if (preFXTypes[slot] != type || !preFXBlocks[slot]) {
                preFXTypes[slot] = type;
                preFXBlocks[slot] = createFXBlock(type);
                if (preFXBlocks[slot]) {
                    preFXBlocks[slot]->init(ctx);
                    for (int p = 0; p < 4; ++p) {
                        preFXBlocks[slot]->setParam(p, preFXParams[slot][p]);
                    }
                }
            }
        }
    }
    int getPreFXType(int slot) const {
        return (slot >= 0 && slot < 4) ? preFXTypes[slot] : 0;
    }

    void setPostFXType(int slot, int type) {
        if (slot >= 0 && slot < 4) {
            type = std::clamp(type, 0, 13);
            if (postFXTypes[slot] != type || !postFXBlocks[slot]) {
                postFXTypes[slot] = type;
                postFXBlocks[slot] = createFXBlock(type);
                if (postFXBlocks[slot]) {
                    postFXBlocks[slot]->init(ctx);
                    for (int p = 0; p < 4; ++p) {
                        postFXBlocks[slot]->setParam(p, postFXParams[slot][p]);
                    }
                }
            }
        }
    }
    int getPostFXType(int slot) const {
        return (slot >= 0 && slot < 4) ? postFXTypes[slot] : 0;
    }

    void setBpm(float bpm) {
        if (bpm > 20.0f && bpm < 999.0f) {
            ctx.bpm = bpm;
        }
    }
    float getBpm() const { return ctx.bpm; }

    void setPreFXParam(int slot, int knobIndex, float value) {
        if (slot >= 0 && slot < 4 && knobIndex >= 0 && knobIndex < 4) {
            preFXParams[slot][knobIndex] = value;
            if (preFXBlocks[slot]) preFXBlocks[slot]->setParam(knobIndex, value);
        }
    }
    float getPreFXParam(int slot, int knobIndex) const {
        return (slot >= 0 && slot < 4 && knobIndex >= 0 && knobIndex < 4) ? preFXParams[slot][knobIndex] : 0.0f;
    }

    void setPostFXParam(int slot, int knobIndex, float value) {
        if (slot >= 0 && slot < 4 && knobIndex >= 0 && knobIndex < 4) {
            postFXParams[slot][knobIndex] = value;
            if (postFXBlocks[slot]) postFXBlocks[slot]->setParam(knobIndex, value);
        }
    }
    float getPostFXParam(int slot, int knobIndex) const {
        return (slot >= 0 && slot < 4 && knobIndex >= 0 && knobIndex < 4) ? postFXParams[slot][knobIndex] : 0.0f;
    }

    DSPBlock* getPreFXBlock(int slot) { return (slot >= 0 && slot < 4) ? preFXBlocks[slot].get() : nullptr; }
    const DSPBlock* getPreFXBlock(int slot) const { return (slot >= 0 && slot < 4) ? preFXBlocks[slot].get() : nullptr; }
    DSPBlock* getPostFXBlock(int slot) { return (slot >= 0 && slot < 4) ? postFXBlocks[slot].get() : nullptr; }
    const DSPBlock* getPostFXBlock(int slot) const { return (slot >= 0 && slot < 4) ? postFXBlocks[slot].get() : nullptr; }

    DSPBlock* getBlock(int blockIndex) { return (blockIndex >= 0 && blockIndex < static_cast<int>(allBlocks.size())) ? allBlocks[blockIndex].get() : nullptr; }
    const DSPBlock* getBlock(int blockIndex) const { return (blockIndex >= 0 && blockIndex < static_cast<int>(allBlocks.size())) ? allBlocks[blockIndex].get() : nullptr; }

    const BlockContext& getContext() const { return ctx; }

    void getScopeData(int blockIndex, float* dest, int count) const {
        if (blockIndex < 0 || blockIndex >= NUM_BLOCKS || !dest || count <= 0) return;
        if (blockIndex >= static_cast<int>(allBlocks.size()) || !allBlocks[blockIndex]) return;

        if (blockIndex == BLK_VELOCITY) {
            if (auto* velBlk = dynamic_cast<VelocityBlock*>(allBlocks[BLK_VELOCITY].get())) {
                velBlk->getVisualScopeData(dest, count);
                return;
            }
        }
        if (blockIndex == BLK_KEYTRACK) {
            if (auto* ktBlk = dynamic_cast<KeyTrackingBlock*>(allBlocks[BLK_KEYTRACK].get())) {
                ktBlk->getVisualScopeData(dest, count);
                return;
            }
        }
        if (blockIndex == BLK_SLOP) {
            if (auto* slopBlk = dynamic_cast<SlopBlock*>(allBlocks[BLK_SLOP].get())) {
                slopBlk->getVisualScopeData(dest, count);
                return;
            }
        }
        if (blockIndex == BLK_MODENV1 || blockIndex == BLK_MODENV2 || blockIndex == BLK_MODENV3) {
            scopes[blockIndex].readLatest(dest, count);
            return;
        }

        // Determine which scope buffer and fundamental frequency to lock to:
        int triggerBlock = BLK_CARRIER1;
        float f0 = lastCarrierFreq.load(std::memory_order_relaxed);

        if (blockIndex == BLK_MODULATOR1) {
            triggerBlock = BLK_MODULATOR1;
            f0 = lastMod1Freq.load(std::memory_order_relaxed);
        } else if (blockIndex == BLK_MODULATOR2) {
            triggerBlock = BLK_MODULATOR2;
            f0 = lastMod2Freq.load(std::memory_order_relaxed);
        } else if (blockIndex == BLK_CARRIER2 || blockIndex == BLK_FILTER2 || blockIndex == BLK_PITCHENV2) {
            triggerBlock = BLK_CARRIER2;
            f0 = lastCarrier2Freq.load(std::memory_order_relaxed);
        }

        if (!std::isfinite(f0) || f0 < 20.0f) f0 = 55.0f;
        if (f0 > 4000.0f) f0 = 4000.0f;

        float period = ctx.sampleRate / f0;
        float totalSpan = 2.0f * period;
        totalSpan = std::clamp(totalSpan, 8.0f, static_cast<float>(VisualScope::RING_SIZE / 2));
        float step = totalSpan / static_cast<float>(count - 1);

        const VisualScope* targetScope = nullptr;
        if (blockIndex >= BLK_PRE_FX_1 && blockIndex <= BLK_PRE_FX_4) {
            targetScope = &preFXScopes[blockIndex - BLK_PRE_FX_1];
        } else if (blockIndex >= BLK_POST_FX_1 && blockIndex <= BLK_POST_FX_4) {
            targetScope = &postFXScopes[blockIndex - BLK_POST_FX_1];
        } else {
            targetScope = &scopes[blockIndex];
        }

        int head = scopes[triggerBlock].writeIndex.load(std::memory_order_acquire);
        int searchStart = (head - static_cast<int>(totalSpan) - 4 + VisualScope::RING_SIZE * 4) & (VisualScope::RING_SIZE - 1);
        int triggerPos = searchStart;

        // Search backward in triggerBlock's own buffer for a rising zero crossing
        int maxSearch = std::min(static_cast<int>(period * 1.5f) + 16, VisualScope::RING_SIZE / 4);
        for (int s = 0; s < maxSearch; ++s) {
            int idx = (searchStart - s + VisualScope::RING_SIZE * 4) & (VisualScope::RING_SIZE - 1);
            int prevIdx = (idx - 1 + VisualScope::RING_SIZE * 4) & (VisualScope::RING_SIZE - 1);
            float curVal = scopes[triggerBlock].buffer[idx];
            float prevVal = scopes[triggerBlock].buffer[prevIdx];
            if (prevVal <= 0.0f && curVal > 0.0f) {
                triggerPos = idx;
                break;
            }
        }

        targetScope->readTriggered(dest, count, triggerPos, step);
    }

    void process(float* monoBuffer, int numSamples) {
        processStereo(monoBuffer, nullptr, numSamples);
    }

    void processStereo(float* left, float* right, int numSamples) {
        // Ensure buffers match block size
        if (static_cast<int>(tempNoiseL.size()) < numSamples) {
            tempNoiseL.assign(numSamples, 0.0f);
            tempNoiseR.assign(numSamples, 0.0f);
            tempCarrier1L.assign(numSamples, 0.0f);
            tempCarrier1R.assign(numSamples, 0.0f);
            tempCarrier2L.assign(numSamples, 0.0f);
            tempCarrier2R.assign(numSamples, 0.0f);
            ctx.mod1Signal.assign(numSamples, 0.0f);
            ctx.mod2Signal.assign(numSamples, 0.0f);
            ctx.pitchEnv1Signal.assign(numSamples, 0.0f);
            ctx.pitchEnv2Signal.assign(numSamples, 0.0f);
            ctx.filterEnv1Signal.assign(numSamples, 0.0f);
            ctx.filterEnv2Signal.assign(numSamples, 0.0f);
            ctx.filterEnv3Signal.assign(numSamples, 0.0f);
            ctx.ampEnvSignal.assign(numSamples, 1.0f);
            ctx.modEnv1Signal.assign(numSamples, 0.0f);
            ctx.modEnv2Signal.assign(numSamples, 0.0f);
            ctx.modEnv3Signal.assign(numSamples, 0.0f);
        }

        // --- MODULATION ENVELOPES ---
        allBlocks[BLK_MODENV1]->processStereo(nullptr, nullptr, numSamples, ctx);
        scopes[BLK_MODENV1].pushBlock(ctx.modEnv1Signal.data(), numSamples);
        allBlocks[BLK_MODENV2]->processStereo(nullptr, nullptr, numSamples, ctx);
        scopes[BLK_MODENV2].pushBlock(ctx.modEnv2Signal.data(), numSamples);
        allBlocks[BLK_MODENV3]->processStereo(nullptr, nullptr, numSamples, ctx);
        scopes[BLK_MODENV3].pushBlock(ctx.modEnv3Signal.data(), numSamples);

        // --- VOICE 1 ---
        // 1. Pitch Envelope 1
        allBlocks[BLK_PITCHENV1]->processStereo(nullptr, nullptr, numSamples, ctx);
        scopes[BLK_PITCHENV1].pushBlock(ctx.pitchEnv1Signal.data(), numSamples);

        // Update carrier 1 pitch for modulator 1 tracking
        if (auto* carrier1 = dynamic_cast<CarrierBlock*>(allBlocks[BLK_CARRIER1].get())) {
            ctx.carrier1PitchHz = carrier1->getBasePitch(ctx);
        }

        // 2. Modulator 1
        allBlocks[BLK_MODULATOR1]->processStereo(nullptr, nullptr, numSamples, ctx);
        scopes[BLK_MODULATOR1].pushBlock(ctx.mod1Signal.data(), numSamples);
        if (auto* mod1 = dynamic_cast<ModulatorBlock*>(allBlocks[BLK_MODULATOR1].get())) {
            lastMod1Freq.store(mod1->getCurrentFreq(), std::memory_order_relaxed);
        }

        // 3. Carrier 1
        std::fill(tempCarrier1L.begin(), tempCarrier1L.begin() + numSamples, 0.0f);
        std::fill(tempCarrier1R.begin(), tempCarrier1R.begin() + numSamples, 0.0f);
        allBlocks[BLK_CARRIER1]->processStereo(tempCarrier1L.data(), tempCarrier1R.data(), numSamples, ctx);
        scopes[BLK_CARRIER1].pushBlock(tempCarrier1L.data(), numSamples);
        if (auto* carrier1 = dynamic_cast<CarrierBlock*>(allBlocks[BLK_CARRIER1].get())) {
            lastCarrierFreq.store(carrier1->getCurrentFreq(), std::memory_order_relaxed);
        }

        // 4. Filter Envelope 1
        allBlocks[BLK_FILTERENV1]->processStereo(nullptr, nullptr, numSamples, ctx);
        scopes[BLK_FILTERENV1].pushBlock(ctx.filterEnv1Signal.data(), numSamples);
        if (auto* fEnv1 = dynamic_cast<FilterEnvelopeBlock*>(allBlocks[BLK_FILTERENV1].get())) {
            if (auto* filter1 = dynamic_cast<FilterBlock*>(allBlocks[BLK_FILTER1].get())) {
                filter1->setPostDriveGain(fEnv1->getPostDriveGain());
            }
        }

        // 5. Filter 1 (filters Carrier 1)
        allBlocks[BLK_FILTER1]->processStereo(tempCarrier1L.data(), tempCarrier1R.data(), numSamples, ctx);
        scopes[BLK_FILTER1].pushBlock(tempCarrier1L.data(), numSamples);

        // --- VOICE 2 ---
        // 6. Pitch Envelope 2
        allBlocks[BLK_PITCHENV2]->processStereo(nullptr, nullptr, numSamples, ctx);
        scopes[BLK_PITCHENV2].pushBlock(ctx.pitchEnv2Signal.data(), numSamples);

        // Update carrier 2 pitch for modulator 2 tracking
        if (auto* carrier2 = dynamic_cast<CarrierBlock*>(allBlocks[BLK_CARRIER2].get())) {
            ctx.carrier2PitchHz = carrier2->getBasePitch(ctx);
        }

        // 7. Modulator 2
        allBlocks[BLK_MODULATOR2]->processStereo(nullptr, nullptr, numSamples, ctx);
        scopes[BLK_MODULATOR2].pushBlock(ctx.mod2Signal.data(), numSamples);
        if (auto* mod2 = dynamic_cast<ModulatorBlock*>(allBlocks[BLK_MODULATOR2].get())) {
            lastMod2Freq.store(mod2->getCurrentFreq(), std::memory_order_relaxed);
        }

        // 8. Carrier 2
        std::fill(tempCarrier2L.begin(), tempCarrier2L.begin() + numSamples, 0.0f);
        std::fill(tempCarrier2R.begin(), tempCarrier2R.begin() + numSamples, 0.0f);
        allBlocks[BLK_CARRIER2]->processStereo(tempCarrier2L.data(), tempCarrier2R.data(), numSamples, ctx);
        scopes[BLK_CARRIER2].pushBlock(tempCarrier2L.data(), numSamples);
        if (auto* carrier2 = dynamic_cast<CarrierBlock*>(allBlocks[BLK_CARRIER2].get())) {
            lastCarrier2Freq.store(carrier2->getCurrentFreq(), std::memory_order_relaxed);
        }

        // 9. Filter Envelope 2
        allBlocks[BLK_FILTERENV2]->processStereo(nullptr, nullptr, numSamples, ctx);
        scopes[BLK_FILTERENV2].pushBlock(ctx.filterEnv2Signal.data(), numSamples);
        if (auto* fEnv2 = dynamic_cast<FilterEnvelopeBlock*>(allBlocks[BLK_FILTERENV2].get())) {
            if (auto* filter2 = dynamic_cast<FilterBlock*>(allBlocks[BLK_FILTER2].get())) {
                filter2->setPostDriveGain(fEnv2->getPostDriveGain());
            }
        }

        // 10. Filter 2 (filters Carrier 2)
        allBlocks[BLK_FILTER2]->processStereo(tempCarrier2L.data(), tempCarrier2R.data(), numSamples, ctx);
        scopes[BLK_FILTER2].pushBlock(tempCarrier2L.data(), numSamples);

        // --- TRANSIENTS ---
        // 11. Noise Transient
        std::fill(tempNoiseL.begin(), tempNoiseL.begin() + numSamples, 0.0f);
        std::fill(tempNoiseR.begin(), tempNoiseR.begin() + numSamples, 0.0f);
        allBlocks[BLK_NOISE]->processStereo(tempNoiseL.data(), tempNoiseR.data(), numSamples, ctx);
        scopes[BLK_NOISE].pushBlock(tempNoiseL.data(), numSamples);

        // 12. Filter Envelope 3
        allBlocks[BLK_FILTERENV3]->processStereo(nullptr, nullptr, numSamples, ctx);
        scopes[BLK_FILTERENV3].pushBlock(ctx.filterEnv3Signal.data(), numSamples);
        if (auto* fEnv3 = dynamic_cast<FilterEnvelopeBlock*>(allBlocks[BLK_FILTERENV3].get())) {
            if (auto* filter3 = dynamic_cast<FilterBlock*>(allBlocks[BLK_FILTER3].get())) {
                filter3->setPostDriveGain(fEnv3->getPostDriveGain());
            }
        }

        // 13. Filter 3 (filters Noise Transient)
        allBlocks[BLK_FILTER3]->processStereo(tempNoiseL.data(), tempNoiseR.data(), numSamples, ctx);
        scopes[BLK_FILTER3].pushBlock(tempNoiseL.data(), numSamples);

        // --- MIXER ---
        if (left) std::fill(left, left + numSamples, 0.0f);
        if (right) std::fill(right, right + numSamples, 0.0f);
        if (auto* mixer = dynamic_cast<MixerBlock*>(allBlocks[BLK_MIXER].get())) {
            mixer->setSources(tempCarrier1L.data(), tempCarrier1R.data(),
                              tempCarrier2L.data(), tempCarrier2R.data(),
                              tempNoiseL.data(), tempNoiseR.data(), numSamples);
        }
        allBlocks[BLK_MIXER]->processStereo(left, right, numSamples, ctx);
        scopes[BLK_MIXER].pushBlock(left, numSamples);

        // --- PRE-AMP FX CHAIN (Slots 1 to 4) ---
        for (int s = 0; s < 4; ++s) {
            if (preFXBlocks[s]) {
                preFXBlocks[s]->processStereo(left, right, numSamples, ctx);
                preFXScopes[s].pushBlock(left, numSamples);
            }
        }

        // --- PRE-AMP LIMITER ---
        allBlocks[BLK_PRE_LIMITER]->processStereo(left, right, numSamples, ctx);
        scopes[BLK_PRE_LIMITER].pushBlock(left, numSamples);

        // --- AMP ENVELOPE & AMP ---
        allBlocks[BLK_AMPENV]->processStereo(nullptr, nullptr, numSamples, ctx);
        scopes[BLK_AMPENV].pushBlock(ctx.ampEnvSignal.data(), numSamples);

        allBlocks[BLK_AMP]->processStereo(left, right, numSamples, ctx);
        scopes[BLK_AMP].pushBlock(left, numSamples);

        // --- POST-AMP FX CHAIN (Slots 1 to 4) ---
        for (int s = 0; s < 4; ++s) {
            if (postFXBlocks[s]) {
                postFXBlocks[s]->processStereo(left, right, numSamples, ctx);
                postFXScopes[s].pushBlock(left, numSamples);
            }
        }

        // --- POST-AMP LIMITER ---
        allBlocks[BLK_POST_LIMITER]->processStereo(left, right, numSamples, ctx);
        scopes[BLK_POST_LIMITER].pushBlock(left, numSamples);

        // --- MODULATIONS VISUALIZATION UPDATE ---
        allBlocks[BLK_VELOCITY]->processStereo(nullptr, nullptr, numSamples, ctx);
        allBlocks[BLK_KEYTRACK]->processStereo(nullptr, nullptr, numSamples, ctx);
        allBlocks[BLK_SLOP]->processStereo(nullptr, nullptr, numSamples, ctx);
    }

    float getModEnvValue(int envIndex) const {
        BlockID blk = (envIndex == 0) ? BLK_MODENV1 : ((envIndex == 1) ? BLK_MODENV2 : BLK_MODENV3);
        if (blk < NUM_BLOCKS && allBlocks[blk]) {
            if (auto* mb = dynamic_cast<ModEnvelopeBlock*>(allBlocks[blk].get())) {
                return mb->getCurrentValue();
            }
        }
        return 0.0f;
    }

    float getFilterEnvValue(int voiceIndex) const {
        BlockID blk = (voiceIndex == 1) ? BLK_FILTERENV1 : ((voiceIndex == 2) ? BLK_FILTERENV2 : BLK_FILTERENV3);
        if (blk < NUM_BLOCKS && allBlocks[blk]) {
            if (auto* fb = dynamic_cast<FilterEnvelopeBlock*>(allBlocks[blk].get())) {
                return fb->getCurrentValue();
            }
        }
        return 0.0f;
    }

    float getPitchEnvValue(int voiceIndex) const {
        BlockID blk = (voiceIndex == 1) ? BLK_PITCHENV1 : BLK_PITCHENV2;
        if (blk < NUM_BLOCKS && allBlocks[blk]) {
            if (auto* pb = dynamic_cast<PitchEnvelopeBlock*>(allBlocks[blk].get())) {
                return pb->getCurrentValue();
            }
        }
        return 0.0f;
    }

    void flushBuffers() {
        for (auto& b : allBlocks) {
            if (b) b->reset();
        }
        for (int s = 0; s < 4; ++s) {
            if (preFXBlocks[s]) preFXBlocks[s]->reset();
            if (postFXBlocks[s]) postFXBlocks[s]->reset();
        }
        std::fill(tempNoiseL.begin(), tempNoiseL.end(), 0.0f);
        std::fill(tempNoiseR.begin(), tempNoiseR.end(), 0.0f);
        std::fill(tempCarrier1L.begin(), tempCarrier1L.end(), 0.0f);
        std::fill(tempCarrier1R.begin(), tempCarrier1R.end(), 0.0f);
        std::fill(tempCarrier2L.begin(), tempCarrier2L.end(), 0.0f);
        std::fill(tempCarrier2R.begin(), tempCarrier2R.end(), 0.0f);

        std::fill(ctx.mod1Signal.begin(), ctx.mod1Signal.end(), 0.0f);
        std::fill(ctx.mod2Signal.begin(), ctx.mod2Signal.end(), 0.0f);
        std::fill(ctx.pitchEnv1Signal.begin(), ctx.pitchEnv1Signal.end(), 0.0f);
        std::fill(ctx.pitchEnv2Signal.begin(), ctx.pitchEnv2Signal.end(), 0.0f);
        std::fill(ctx.filterEnv1Signal.begin(), ctx.filterEnv1Signal.end(), 0.0f);
        std::fill(ctx.filterEnv2Signal.begin(), ctx.filterEnv2Signal.end(), 0.0f);
        std::fill(ctx.filterEnv3Signal.begin(), ctx.filterEnv3Signal.end(), 0.0f);
        std::fill(ctx.ampEnvSignal.begin(), ctx.ampEnvSignal.end(), 0.0f);
        std::fill(ctx.modEnv1Signal.begin(), ctx.modEnv1Signal.end(), 0.0f);
        std::fill(ctx.modEnv2Signal.begin(), ctx.modEnv2Signal.end(), 0.0f);
        std::fill(ctx.modEnv3Signal.begin(), ctx.modEnv3Signal.end(), 0.0f);

        ctx.isTriggered = false;
        ctx.curvedVelocity = 1.0f;
        ctx.velDecayMod = 0.0f;
        ctx.velDepthMod = 0.0f;
        ctx.velVolumeGain = 1.0f;
        ctx.curvedKeyNote = 0.5f;
        ctx.keyDecayMod = 0.0f;
        ctx.keyDepthMod = 0.0f;
        ctx.keyVolumeGain = 1.0f;

        for (auto& s : scopes) {
            s.clear();
        }
        for (auto& s : preFXScopes) {
            s.clear();
        }
        for (auto& s : postFXScopes) {
            s.clear();
        }
    }

    // Master Audio Panic: Applies a 1ms fast exponential fade-out to current buffer,
    // flushes all delay lines and internal buffers, and guarantees pop-free silence.
    void triggerPanic(float* left, float* right, int numSamples, float sr = 44100.0f) {
        if (numSamples > 0 && (left || right)) {
            int fadeSamples = std::clamp(static_cast<int>(0.001f * sr), 1, numSamples);
            float invFade = 1.0f / static_cast<float>(fadeSamples);
            for (int i = 0; i < fadeSamples; ++i) {
                // Fast exponential decay: drops to ~1% (-40dB) at end of 1ms
                float fade = std::exp(-4.605f * (static_cast<float>(i) * invFade));
                if (left)  left[i]  *= fade;
                if (right) right[i] *= fade;
            }
            for (int i = fadeSamples; i < numSamples; ++i) {
                if (left)  left[i]  = 0.0f;
                if (right) right[i] = 0.0f;
            }
        }
        flushBuffers();
    }

    float getAudioRateSourceSample(HydraInputSource src) const {
        switch(src) {
            case HydraInputSource::OscCarrier1: return tempCarrier1L.empty() ? 0.0f : tempCarrier1L[0];
            case HydraInputSource::OscModulator1: return ctx.mod1Signal.empty() ? 0.0f : ctx.mod1Signal[0];
            case HydraInputSource::OscCarrier2: return tempCarrier2L.empty() ? 0.0f : tempCarrier2L[0];
            case HydraInputSource::OscModulator2: return ctx.mod2Signal.empty() ? 0.0f : ctx.mod2Signal[0];
            default: return 0.0f;
        }
    }

private:
    BlockContext ctx;
    std::vector<std::unique_ptr<DSPBlock>> allBlocks;
    VisualScope scopes[NUM_BLOCKS];
    int preFXTypes[4] = { 4, 13, 11, 7 };
    int postFXTypes[4] = { 8, 3, 9, 1 };
    std::unique_ptr<DSPBlock> preFXBlocks[4];
    std::unique_ptr<DSPBlock> postFXBlocks[4];
    float preFXParams[4][4] = { {0.0f} };
    float postFXParams[4][4] = { {0.0f} };
    VisualScope preFXScopes[4];
    VisualScope postFXScopes[4];
    mutable std::atomic<float> lastCarrierFreq{ 55.0f };
    mutable std::atomic<float> lastCarrier2Freq{ 55.0f };
    mutable std::atomic<float> lastMod1Freq{ 55.0f };
    mutable std::atomic<float> lastMod2Freq{ 55.0f };
    std::vector<float> tempNoiseL;
    std::vector<float> tempNoiseR;
    std::vector<float> tempCarrier1L;
    std::vector<float> tempCarrier1R;
    std::vector<float> tempCarrier2L;
    std::vector<float> tempCarrier2R;
    uint32_t slopRngState = 0x13579bdf;
    VelocityTracker velocityTracker;
};

} // namespace TbdAudio