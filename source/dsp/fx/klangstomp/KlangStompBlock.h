#pragma once

#include "../../common/AudioEffect.h"
#include "SwedishKlangSaw.h"
#include "KlangZone.h"
#include "Rodent.h"
#include "GreenScreamer.h"
#include "RussianKlangPi.h"
#include "IxionGuild.h"
#include "Yellow250.h"
#include "VelcroFace.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace TbdAudio::KlangStomp {

/**
 * @brief Enumeration of the 8 iconic analog distortion/overdrive/fuzz circuit models
 *        housed in the KlangStomp 8-Pedal Hall of Fame processor.
 */
enum class CircuitModel : uint8_t {
    SwedishKlangSaw = 0, // Boss HM-2 Heavy Metal
    KlangZone       = 1, // Boss MT-2 Metal Zone
    Rodent          = 2, // ProCo RAT (LM308 slew-rate limited)
    GreenScreamer   = 3, // Ibanez TS9 / TS808 Tube Screamer
    RussianKlangPi  = 4, // Electro-Harmonix Big Muff Pi
    IxionGuild      = 5, // Klon Centaur (+18V charge pump dual-gang)
    Yellow250       = 6, // DOD Overdrive Preamp 250 / MXR Dist+
    VelcroFace      = 7, // Fuzz Face / Tone Bender (starved BJT)
    Count           = 8,

    // Legacy Compatibility Aliases
    SwedishChainsaw = SwedishKlangSaw,
    IndustrialZone  = KlangZone,
    Screamer808     = GreenScreamer,
    RussianMuff     = RussianKlangPi,
    SovereignCentaur= IxionGuild,
    YellowPreamp250 = Yellow250
};

/**
 * @brief KlangStompBlock — Unified 8-Knob DSP Processor hosting all 8 KlangStomp
 *        analog dirt circuits with stereo dual-mono processing, pre-EQ Clean Blend,
 *        Output Level trim, and Profile M Tri-Zone Parallel Mix with Phase Invert.
 *
 * 8-Knob Parameter Topology:
 *   - Knob 0 (GAIN / DRIVE)   : [0..1] Circuit gain / distortion intensity
 *   - Knob 1 (TONE / FILTER)  : [0..1] Primary tone stack / filter sweep
 *   - Knob 2 (LEVEL / VOL)    : [0..1] Post-circuit makeup level (-24 dB .. +12 dB, 0.5 = 0 dB)
 *   - Knob 3 (CIRCUIT MODEL)  : [0..1] Discrete 8-position selector (0..7)
 *   - Knob 4 (MID / GYRATOR)  : [0..1] Chainsaw gyrator peak / parametric mid / mid-scoop override
 *   - Knob 5 (STARVE / VOLTS) : [0..1] Rail voltage headroom (0.0 = 3V sag, 0.5 = 9V, 1.0 = 18V)
 *   - Knob 6 (CLEAN BLEND)    : [0..1] Phase-coherent clean bass/transient parallel injection
 *   - Knob 7 (MIX / ∅)        : [0..1] Profile M Tri-Zone Mix (0.0 = Wet, 0.5 = Dry, 1.0 = 50/50)
 */
class KlangStompBlock final : public AudioEffect8 {
public:
    KlangStompBlock() noexcept {
        setParam(0, 0.5f);  // GAIN / DRIVE
        setParam(1, 0.5f);  // TONE / FILTER
        setParam(2, 0.5f);  // LEVEL / VOL (0 dB unity at 0.5)
        setParam(3, 0.0f);  // CIRCUIT MODEL (0 = Swedish Chainsaw)
        setParam(4, 0.5f);  // MID / GYRATOR
        setParam(5, 0.5f);  // STARVE / VOLTS (9V nominal)
        setParam(6, 0.0f);  // CLEAN BLEND
        setParam(7, 0.0f);  // MIX (0.0 = 100% Wet per Profile M)
    }

    void init(const BlockContext& ctx) override {
        prepare(static_cast<double>(ctx.sampleRate), 512);
    }

    void prepare(double sampleRate, int /*maxBlockSize*/) noexcept {
        const float sr = (sampleRate > 8000.0) ? static_cast<float>(sampleRate) : 48000.0f;
        for (int ch = 0; ch < 2; ++ch) {
            swedishKlangSaw_[ch].init(sr);
            klangZone_[ch].init(sr);
            rodent_[ch].init(sr);
            greenScreamer_[ch].init(sr);
            russianKlangPi_[ch].init(sr);
            ixionGuild_[ch].init(sr);
            yellow250_[ch].init(sr);
            velcroFace_[ch].init(sr);
        }
    }

    void reset() noexcept override {
        for (int ch = 0; ch < 2; ++ch) {
            swedishKlangSaw_[ch].reset();
            klangZone_[ch].reset();
            rodent_[ch].reset();
            greenScreamer_[ch].reset();
            russianKlangPi_[ch].reset();
            ixionGuild_[ch].reset();
            yellow250_[ch].reset();
            velcroFace_[ch].reset();
        }
    }

    inline void setParam8(int index, float value) noexcept {
        setParam(index, value);
    }

    /**
     * @brief Directly selects one of the 8 circuit models by enum or index [0..7].
     */
    void setCircuitModel(CircuitModel model) noexcept {
        const int idx = std::clamp(static_cast<int>(model), 0, 7);
        setParam(3, static_cast<float>(idx) / 7.0f);
    }

    [[nodiscard]] CircuitModel getCircuitModel() const noexcept {
        const float norm = std::clamp(params8[3], 0.0f, 1.0f);
        const int idx = std::clamp(static_cast<int>(norm * 7.0f + 0.5f), 0, 7);
        return static_cast<CircuitModel>(idx);
    }

    [[nodiscard]] float processChannelSample(int ch, float dryIn) noexcept {
        const int c = (ch & 1);
        const float drive  = std::clamp(params8[0], 0.0f, 1.0f);
        const float tone   = std::clamp(params8[1], 0.0f, 1.0f);
        const float levelN = std::clamp(params8[2], 0.0f, 1.0f);
        const float mid    = std::clamp(params8[4], 0.0f, 1.0f);
        const float starve = std::clamp(params8[5], 0.0f, 1.0f);
        const float cleanB = std::clamp(params8[6], 0.0f, 1.0f);
        const float mixN   = std::clamp(params8[7], 0.0f, 1.0f);

        const CircuitModel activeModel = getCircuitModel();
        if (activeModel != lastModel_[c]) {
            lastModel_[c] = activeModel;
            resetChannel(c, activeModel);
        }

        // Fast bypass when Profile M is exactly at 12:00 (0.5 = 100% Pure Dry)
        float dryGain = 0.0f;
        float wetGain = 1.0f;
        Taper::profileM_TriZoneMix(mixN, phaseInvert, dryGain, wetGain);
        if (wetGain == 0.0f) {
            return dryIn * dryGain;
        }

        float circuitOut = 0.0f;
        switch (activeModel) {
            case CircuitModel::SwedishKlangSaw:
                circuitOut = swedishKlangSaw_[c].processSample(dryIn, drive, tone, mid, starve);
                break;
            case CircuitModel::KlangZone:
                circuitOut = klangZone_[c].processSample(dryIn, drive, tone, mid, starve);
                break;
            case CircuitModel::Rodent:
                circuitOut = rodent_[c].processSample(dryIn, drive, tone, mid, starve);
                break;
            case CircuitModel::GreenScreamer:
                circuitOut = greenScreamer_[c].processSample(dryIn, drive, tone, mid, starve);
                break;
            case CircuitModel::RussianKlangPi:
                circuitOut = russianKlangPi_[c].processSample(dryIn, drive, tone, mid, starve);
                break;
            case CircuitModel::IxionGuild:
                circuitOut = ixionGuild_[c].processSample(dryIn, drive, tone, mid, starve);
                break;
            case CircuitModel::Yellow250:
                circuitOut = yellow250_[c].processSample(dryIn, drive, tone, mid, starve);
                break;
            case CircuitModel::VelcroFace:
            default:
                circuitOut = velcroFace_[c].processSample(dryIn, drive, tone, mid, starve);
                break;
        }

        // Knob 6: Dedicated internal Clean Blend (equal-power style clean injection)
        const float blendedDirt = circuitOut * (1.0f - 0.65f * cleanB) + dryIn * cleanB;

        // Knob 2: Output Level (-24 dB at 0.0, 0 dB at 0.5, +12 dB at 1.0)
        const float levelDb = (levelN < 0.5f)
            ? ((levelN * 2.0f - 1.0f) * 24.0f)
            : ((levelN - 0.5f) * 2.0f * 12.0f);
        const float levelGain = FastMath::fastDbToGain(levelDb);
        const float wetOut = std::clamp(blendedDirt * levelGain, -8.0f, 8.0f);

        // Knob 7: Profile M Tri-Zone Parallel Mix + Phase Invert
        return dryIn * dryGain + wetOut * wetGain;
    }

    void process(float* buffer, int numSamples, BlockContext& /*ctx*/) override {
        FastMath::enableFTZDAZ();
        if (!buffer || numSamples <= 0) return;
        for (int i = 0; i < numSamples; ++i) {
            buffer[i] = processChannelSample(0, buffer[i]);
        }
        FastMath::sanitizeBuffer(buffer, numSamples);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& /*ctx*/) override {
        FastMath::enableFTZDAZ();
        if (!left || numSamples <= 0) return;
        if (!right || right == left) {
            for (int i = 0; i < numSamples; ++i) {
                left[i] = processChannelSample(0, left[i]);
            }
            FastMath::sanitizeBuffer(left, numSamples);
            return;
        }
        for (int i = 0; i < numSamples; ++i) {
            left[i]  = processChannelSample(0, left[i]);
            right[i] = processChannelSample(1, right[i]);
        }
        FastMath::sanitizeBuffer(left, numSamples);
        FastMath::sanitizeBuffer(right, numSamples);
    }

    void processBlock(float* const* audioChannels,
                      int numChannels,
                      int numSamples,
                      const BlockContext& /*ctx*/) noexcept {
        FastMath::enableFTZDAZ();
        if (!audioChannels || numChannels <= 0 || numSamples <= 0) return;

        const int chCount = std::min(numChannels, 2);
        for (int ch = 0; ch < chCount; ++ch) {
            float* buf = audioChannels[ch];
            if (!buf) continue;
            for (int i = 0; i < numSamples; ++i) {
                buf[i] = processChannelSample(ch, buf[i]);
            }
            FastMath::sanitizeBuffer(buf, numSamples);
        }

        for (int ch = 2; ch < numChannels; ++ch) {
            if (audioChannels[ch]) {
                FastMath::sanitizeBuffer(audioChannels[ch], numSamples);
            }
        }
    }

private:
    void resetChannel(int c, CircuitModel model) noexcept {
        switch (model) {
            case CircuitModel::SwedishKlangSaw:  swedishKlangSaw_[c].reset();  break;
            case CircuitModel::KlangZone:        klangZone_[c].reset();        break;
            case CircuitModel::Rodent:           rodent_[c].reset();           break;
            case CircuitModel::GreenScreamer:    greenScreamer_[c].reset();    break;
            case CircuitModel::RussianKlangPi:   russianKlangPi_[c].reset();   break;
            case CircuitModel::IxionGuild:       ixionGuild_[c].reset();       break;
            case CircuitModel::Yellow250:        yellow250_[c].reset();        break;
            case CircuitModel::VelcroFace:
            default:                             velcroFace_[c].reset();       break;
        }
    }

    SwedishKlangSaw swedishKlangSaw_[2]{};
    KlangZone       klangZone_[2]{};
    Rodent          rodent_[2]{};
    GreenScreamer   greenScreamer_[2]{};
    RussianKlangPi  russianKlangPi_[2]{};
    IxionGuild      ixionGuild_[2]{};
    Yellow250       yellow250_[2]{};
    VelcroFace      velcroFace_[2]{};

    CircuitModel lastModel_[2]{CircuitModel::SwedishKlangSaw, CircuitModel::SwedishKlangSaw};
};

} // namespace TbdAudio::KlangStomp
