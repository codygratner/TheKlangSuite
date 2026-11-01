#pragma once

#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <string>
#include "dsp/drums/MmeSnare.h"
#include "dsp/drums/MmeClap.h"

namespace test_mme_snare_clap {

inline void test_snare_all_16_models() {
    std::cout << "  [1/4] Running MmeSnare model verification & audio render (16 models)..." << std::endl;
    tkdsp::MmeSnare snare;
    snare.prepare(48000.0);
    assert(snare.numModels() == 16);

    for (uint8_t m = 0; m < 16; ++m) {
        snare.setParam(0, static_cast<float>(m) / 15.0f);
        snare.trigger(1.0f);

        float peak = 0.0f;
        float rmsSum = 0.0f;
        constexpr int kSamples = 2048;

        for (int i = 0; i < kSamples; ++i) {
            float l = 0.0f, r = 0.0f;
            snare.renderSampleStereo(180.0f, l, r);

            assert(!std::isnan(l) && !std::isinf(l));
            assert(!std::isnan(r) && !std::isinf(r));
            assert(std::abs(l) < 3.0f);
            assert(std::abs(r) < 3.0f);

            peak = std::max(peak, std::max(std::abs(l), std::abs(r)));
            rmsSum += l * l + r * r;
        }

        const float rms = std::sqrt(rmsSum / (2.0f * kSamples));
        assert(peak > 0.05f);
        assert(rms > 0.01f);

        std::cout << "        Snare [" << static_cast<int>(m) << ": "
                  << tkdsp::MmeSnare::getModelName(m) << "] Peak: "
                  << peak << ", RMS: " << rms << " - OK" << std::endl;
    }
}

inline void test_snare_snap_and_decay() {
    std::cout << "  [2/4] Running MmeSnare snap & decay parameter verification..." << std::endl;
    tkdsp::MmeSnare snareShort;
    snareShort.prepare(48000.0);
    snareShort.setParam(2, 0.05f); // Short decay
    snareShort.trigger(1.0f);

    tkdsp::MmeSnare snareLong;
    snareLong.prepare(48000.0);
    snareLong.setParam(2, 0.90f); // Long decay
    snareLong.trigger(1.0f);

    constexpr int kCheckSample = 4800;
    float shortEnergy = 0.0f;
    float longEnergy = 0.0f;

    for (int i = 0; i < kCheckSample; ++i) {
        float l1 = 0.0f, r1 = 0.0f;
        float l2 = 0.0f, r2 = 0.0f;
        snareShort.renderSampleStereo(180.0f, l1, r1);
        snareLong.renderSampleStereo(180.0f, l2, r2);
        if (i > 3000) {
            shortEnergy += l1 * l1;
            longEnergy += l2 * l2;
        }
    }

    assert(longEnergy > shortEnergy * 3.0f);
    std::cout << "        Snare decay scaling verified! (Short tail=" << shortEnergy
              << ", Long tail=" << longEnergy << ")" << std::endl;
}

inline void test_clap_all_16_models() {
    std::cout << "  [3/4] Running MmeClap model verification & audio render (16 models)..." << std::endl;
    tkdsp::MmeClap clap;
    clap.prepare(48000.0);
    assert(clap.numModels() == 16);

    for (uint8_t m = 0; m < 16; ++m) {
        clap.setParam(0, static_cast<float>(m) / 15.0f);
        clap.trigger(1.0f);

        float peak = 0.0f;
        float rmsSum = 0.0f;
        constexpr int kSamples = 2048;

        for (int i = 0; i < kSamples; ++i) {
            float l = 0.0f, r = 0.0f;
            clap.renderSampleStereo(0.0f, l, r);

            assert(!std::isnan(l) && !std::isinf(l));
            assert(!std::isnan(r) && !std::isinf(r));
            assert(std::abs(l) < 3.0f);
            assert(std::abs(r) < 3.0f);

            peak = std::max(peak, std::max(std::abs(l), std::abs(r)));
            rmsSum += l * l + r * r;
        }

        const float rms = std::sqrt(rmsSum / (2.0f * kSamples));
        assert(peak > 0.05f);
        assert(rms > 0.01f);

        std::cout << "        Clap [" << static_cast<int>(m) << ": "
                  << tkdsp::MmeClap::getModelName(m) << "] Peak: "
                  << peak << ", RMS: " << rms << " - OK" << std::endl;
    }
}

inline void test_clap_multi_burst_and_diffusion() {
    std::cout << "  [4/4] Running MmeClap multi-burst & diffusion verification..." << std::endl;
    tkdsp::MmeClap clap;
    clap.prepare(48000.0);
    clap.setParam(3, 1.0f); // Maximum pre-bursts (5 bursts)
    clap.setParam(4, 0.3f); // ~12 ms spacing (~576 samples)
    clap.setParam(5, 0.7f); // Reverb on
    clap.trigger(1.0f);

    float peakFirstBurst = 0.0f;
    float peakSecondBurst = 0.0f;

    for (int i = 0; i < 2000; ++i) {
        float l = 0.0f, r = 0.0f;
        clap.renderSampleStereo(0.0f, l, r);
        const float val = std::abs(l);
        if (i < 200) {
            peakFirstBurst = std::max(peakFirstBurst, val);
        } else if (i > 450 && i < 750) {
            peakSecondBurst = std::max(peakSecondBurst, val);
        }
    }

    assert(peakFirstBurst > 0.1f);
    assert(peakSecondBurst > 0.1f);
    std::cout << "        Multi-burst clap sequence verified! (Burst 1=" << peakFirstBurst
              << ", Burst 2=" << peakSecondBurst << ")" << std::endl;
}

inline void run_all_mme_snare_clap_tests() {
    test_snare_all_16_models();
    test_snare_snap_and_decay();
    test_clap_all_16_models();
    test_clap_multi_burst_and_diffusion();
}

} // namespace test_mme_snare_clap
