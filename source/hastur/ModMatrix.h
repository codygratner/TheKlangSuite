#pragma once

#include <cmath>
#include <algorithm>
#include <cstdint>

namespace CrackedChimes::DSP {

/**
 * @brief Universal Dual-Channel Audio-Rate Cross-Modulation Matrix (Mod A & Mod B).
 * 
 * Supports both raw oscillator cores and APF+Tanh phase-dispersed signals as modulation sources.
 */
class ModMatrix {
public:
    enum class Source : uint8_t {
        RAW   = 0, // Pre-chopper/wavefolder raw oscillator
        CORE  = 1, // Core shaped oscillator
        SUB   = 2, // Sub-oscillator
        NOISE = 3, // Analog white noise
        OSC_1 = 1, // Backwards-compatible alias
        OSC_2 = 2, // Backwards-compatible alias
        PRE_FILTER_MIX = 1,
        POST_FILTER_MIX = 1
    };

    enum class Type : uint8_t {
        OFF            = 0,
        LINEAR_FM      = 1,
        EXPONENTIAL_FM = 2,
        AM             = 3,
        RM             = 4
    };

    enum class Destination : uint8_t {
        OSC_1             = 0,
        OSC_2             = 1,
        FILTER_CUTOFF     = 2,
        POST_FILTER_LEVEL = 3
    };

    struct ChannelConfig {
        Source source{Source::OSC_1};
        Type type{Type::OFF};
        float amount{0.0f}; // Bipolar [-1.0, +1.0]
        Destination destination{Destination::OSC_1};
    };

    struct Sources {
        float raw{0.0f};
        float core{0.0f};
        float sub{0.0f};
        float osc1{0.0f};
        float osc2{0.0f};
        float noise{0.0f};
        float preFilter{0.0f};
        float postFilter{0.0f};
    };

    struct ModResults {
        // Frequency shifts (Hz) for Linear FM
        float linFmOsc1Hz{0.0f};
        float linFmOsc2Hz{0.0f};

        // Pitch multipliers for Exponential FM
        float expFmOsc1Mult{1.0f};
        float expFmOsc2Mult{1.0f};

        // Amplitude / Ring Mod scaling
        float amRmOsc1{1.0f};
        float amRmOsc2{1.0f};

        // Filter Cutoff Multiplier
        float cutoffMult{1.0f};

        // Post-Filter Level Multiplier
        float postFilterMult{1.0f};
    };

    /**
     * @brief Compute modulation contributions from dual channels (Mod A & Mod B).
     */
    [[nodiscard]] static ModResults compute(
        const ChannelConfig& modA,
        const ChannelConfig& modB,
        const Sources& src) noexcept
    {
        ModResults res;
        applyChannel(modA, src, res);
        applyChannel(modB, src, res);
        return res;
    }

private:
    static float getSourceSample(Source s, const Sources& src) noexcept {
        switch (s) {
            case Source::RAW:   return src.raw;
            case Source::CORE:  return (src.core != 0.0f) ? src.core : src.osc1;
            case Source::SUB:   return (src.sub != 0.0f) ? src.sub : src.osc2;
            case Source::NOISE: return src.noise;
            default:            return src.raw;
        }
    }

    static void applyChannel(const ChannelConfig& cfg, const Sources& src, ModResults& res) noexcept {
        if (cfg.type == Type::OFF || std::abs(cfg.amount) < 0.0001f) {
            return;
        }

        const float val = getSourceSample(cfg.source, src) * cfg.amount;

        switch (cfg.destination) {
            case Destination::OSC_1:
                applyToOsc(cfg.type, val, res.linFmOsc1Hz, res.expFmOsc1Mult, res.amRmOsc1);
                break;
            case Destination::OSC_2:
                applyToOsc(cfg.type, val, res.linFmOsc2Hz, res.expFmOsc2Mult, res.amRmOsc2);
                break;
            case Destination::FILTER_CUTOFF:
                if (cfg.type == Type::LINEAR_FM) {
                    res.cutoffMult *= std::clamp(1.0f + val * 0.75f, 0.05f, 4.0f);
                } else if (cfg.type == Type::EXPONENTIAL_FM) {
                    res.cutoffMult *= std::pow(2.0f, val * 2.5f);
                } else {
                    res.cutoffMult *= std::clamp(1.0f + val * 0.75f, 0.05f, 4.0f);
                }
                break;
            case Destination::POST_FILTER_LEVEL:
                if (cfg.type == Type::AM) {
                    res.postFilterMult *= std::clamp(1.0f + val * 1.5f, 0.0f, 3.0f);
                } else if (cfg.type == Type::RM) {
                    // True 4-Quadrant Ring Modulation with Dry/Wet Crossfade
                    const float depth = std::clamp(std::abs(cfg.amount), 0.0f, 1.0f);
                    const float srcSample = getSourceSample(cfg.source, src);
                    const float rmMult = (1.0f - depth) + depth * srcSample;
                    res.postFilterMult *= rmMult;
                } else {
                    res.postFilterMult *= std::clamp(1.0f + val, 0.0f, 3.0f);
                }
                break;
        }
    }

    static void applyToOsc(Type type, float val, float& linFmHz, float& expFmMult, float& amRmScale) noexcept {
        switch (type) {
            case Type::LINEAR_FM:
                linFmHz += val * 3000.0f;
                break;
            case Type::EXPONENTIAL_FM:
                expFmMult *= std::pow(2.0f, val * 4.0f);
                break;
            case Type::AM:
                amRmScale *= std::clamp(1.0f + val * 1.5f, 0.0f, 3.0f);
                break;
            case Type::RM:
                amRmScale *= val;
                break;
            case Type::OFF:
            default:
                break;
        }
    }
};

} // namespace CrackedChimes::DSP
