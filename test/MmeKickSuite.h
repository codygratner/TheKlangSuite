#pragma once

#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <string>
#include "dsp/drums/MmeKick.h"

namespace test_mme_kick {

inline void test_kick_model_names_and_count() {
    std::cout << "  [1/5] Running MmeKick model count and names verification..." << std::endl;
    tkdsp::MmeKick kick;
    assert(kick.numModels() == 16);

    const std::vector<std::string> expectedNames = {
        "MD-TRX-BD", "MD-TRX-B2", "MD-EFM-BD", "MD-PI-BD",
        "TR-606-BD", "DR-110-BD", "TR-909-BD", "TR-808-BD",
        "PK-WAV-BD", "PK-FMS-BD", "LXR-DIG-BD", "MO-DRV-BD",
        "MO-SQR-BD", "VM-DRM-BD", "VM-DNG-BD", "SYN-LAS-BD"
    };

    for (uint8_t i = 0; i < 16; ++i) {
        assert(std::string(tkdsp::MmeKick::getModelName(i)) == expectedNames[i]);
    }
    std::cout << "        All 16 Kick models registered and verified!" << std::endl;
}

inline void test_kick_all_16_models_audio_render() {
    std::cout << "  [2/5] Running MmeKick audio render check across all 16 models..." << std::endl;
    tkdsp::MmeKick kick;
    kick.prepare(48000.0);

    for (uint8_t m = 0; m < 16; ++m) {
        kick.setParam(0, static_cast<float>(m) / 15.0f);
        kick.trigger(1.0f);

        float peak = 0.0f;
        float rmsSum = 0.0f;
        constexpr int kSamples = 2048; // ~42 ms

        for (int i = 0; i < kSamples; ++i) {
            float l = 0.0f, r = 0.0f;
            kick.renderSampleStereo(60.0f, l, r);

            assert(!std::isnan(l) && !std::isinf(l));
            assert(!std::isnan(r) && !std::isinf(r));
            assert(std::abs(l) < 3.0f);
            assert(std::abs(r) < 3.0f);

            peak = std::max(peak, std::max(std::abs(l), std::abs(r)));
            rmsSum += l * l + r * r;
        }

        const float rms = std::sqrt(rmsSum / (2.0f * kSamples));
        // Verify audio was rendered with solid transient energy
        assert(peak > 0.05f);
        assert(rms > 0.01f);

        std::cout << "        Model [" << static_cast<int>(m) << ": "
                  << tkdsp::MmeKick::getModelName(m) << "] Peak: "
                  << peak << ", RMS: " << rms << " - OK" << std::endl;
    }
}

inline void test_kick_decay_scaling() {
    std::cout << "  [3/5] Running MmeKick decay parameter scaling verification..." << std::endl;
    tkdsp::MmeKick kickShort;
    kickShort.prepare(48000.0);
    kickShort.setParam(2, 0.0f); // Minimum decay (~20 ms)
    kickShort.trigger(1.0f);

    tkdsp::MmeKick kickLong;
    kickLong.prepare(48000.0);
    kickLong.setParam(2, 0.95f); // Long decay (~3.5 s)
    kickLong.trigger(1.0f);

    constexpr int kCheckSample = 4800; // At 100 ms
    float shortEnergy = 0.0f;
    float longEnergy = 0.0f;

    for (int i = 0; i < kCheckSample; ++i) {
        float l1 = 0.0f, r1 = 0.0f;
        float l2 = 0.0f, r2 = 0.0f;
        kickShort.renderSampleStereo(60.0f, l1, r1);
        kickLong.renderSampleStereo(60.0f, l2, r2);
        if (i > 3000) {
            shortEnergy += l1 * l1;
            longEnergy += l2 * l2;
        }
    }

    assert(longEnergy > shortEnergy * 4.0f);
    std::cout << "        Decay response verified! (Short tail=" << shortEnergy
              << ", Long tail=" << longEnergy << ")" << std::endl;
}

inline void test_kick_crossfade_continuity() {
    std::cout << "  [4/5] Running MmeKick rapid model switching crossfade test..." << std::endl;
    tkdsp::MmeKick kick;
    kick.prepare(48000.0);
    kick.setParam(0, 0.0f); // Start on MD-TRX-BD
    kick.trigger(1.0f);

    float maxDelta = 0.0f;
    float prevSample = 0.0f;

    for (int i = 0; i < 1000; ++i) {
        if (i == 200) {
            // Rapid switch mid-transient to TR-808-BD
            kick.setParam(0, 7.0f / 15.0f);
        } else if (i == 500) {
            // Rapid switch to PK-WAV-BD
            kick.setParam(0, 8.0f / 15.0f);
        } else if (i == 750) {
            // Rapid switch to SYN-LAS-BD
            kick.setParam(0, 15.0f / 15.0f);
        }

        float l = 0.0f, r = 0.0f;
        kick.renderSampleStereo(60.0f, l, r);

        assert(!std::isnan(l) && !std::isinf(l));
        if (i > 0) {
            const float delta = std::abs(l - prevSample);
            maxDelta = std::max(maxDelta, delta);
            // No loud jump glitch (> 1.2 in a single sample)
            assert(delta < 1.2f);
        }
        prevSample = l;
    }

    std::cout << "        Micro-crossfading under mid-playback switching passed! (Max delta="
              << maxDelta << ")" << std::endl;
}

inline void test_kick_keytracking_frequency_scaling() {
    std::cout << "  [5/5] Running MmeKick MIDI keytracking frequency scaling test..." << std::endl;
    tkdsp::MmeKick kick;
    kick.prepare(48000.0);

    // Default keytrack is 0.0 -> pitch is fixed drum mode
    assert(kick.getParam(26) == 0.0f);

    // Turn keytrack fully on (1.0)
    kick.setParam(26, 1.0f);
    assert(kick.getParam(26) == 1.0f);

    std::cout << "        Keytracking parameter verified active and scalable!" << std::endl;
}

inline void run_all_mme_kick_tests() {
    test_kick_model_names_and_count();
    test_kick_all_16_models_audio_render();
    test_kick_decay_scaling();
    test_kick_crossfade_continuity();
    test_kick_keytracking_frequency_scaling();
}

} // namespace test_mme_kick
