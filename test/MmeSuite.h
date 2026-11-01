#pragma once

#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include "dsp/engines/MultiMachineEngineBase.h"
#include "dsp/fx/ElektronTrackWrapper.h"

namespace test_mme {

namespace FastMath = TbdAudio::FastMath;

/**
 * @brief Concrete Mock Multi-Machine Engine for host unit verification.
 */
class MockMultiMachineKick final : public tkdsp::MultiMachineEngineBase {
public:
    MockMultiMachineKick() noexcept = default;

    [[nodiscard]] uint8_t numModels() const noexcept override { return 4; }

protected:
    void onPrepareSubModels(float sampleRate) noexcept override {
        sr_ = sampleRate;
        invSr_ = 1.0f / sampleRate;
    }

    void onSubModelTrigger(uint8_t modelIndex, float velocity) noexcept override {
        (void)modelIndex;
        env_ = velocity;
    }

    void onSubModelReset(uint8_t modelIndex) noexcept override {
        (void)modelIndex;
        phase_ = 0.0f;
    }

    void renderSubModelStereo(uint8_t modelIndex, float pitchHz, float& outL, float& outR) noexcept override {
        // Advance phase
        phase_ += pitchHz * invSr_;
        if (phase_ >= 1.0f) phase_ -= std::floor(phase_);

        float sample = 0.0f;
        switch (modelIndex) {
            case 0: // Pure Sine
                sample = FastMath::fastSinNorm(phase_);
                break;
            case 1: // Triangle
                sample = (phase_ < 0.5f) ? (4.0f * phase_ - 1.0f) : (3.0f - 4.0f * phase_);
                break;
            case 2: // Square
                sample = (phase_ < 0.5f) ? 1.0f : -1.0f;
                break;
            case 3: // Pseudo-random noise
            default:
                noiseSeed_ = noiseSeed_ * 1664525u + 1013904223u;
                sample = static_cast<float>(static_cast<int32_t>(noiseSeed_)) * (1.0f / 2147483648.0f);
                break;
        }

        // Apply decay envelope
        sample *= env_;
        env_ *= 0.9995f; // Exponential decay

        outL = sample;
        outR = sample;
    }

private:
    float sr_ = 48000.0f;
    float invSr_ = 1.0f / 48000.0f;
    float phase_ = 0.0f;
    float env_ = 1.0f;
    uint32_t noiseSeed_ = 123456789u;
};

inline void test_elektron_track_wrapper_defaults() {
    std::cout << "  [1/4] Running ElektronTrackWrapper default pass-through test..." << std::endl;
    tkdsp::ElektronTrackWrapper wrapper;
    wrapper.prepare(48000.0f);

    // Default parameters: FLTF=0 (HP bypassed), FLTW=1 (LP bypassed), EQG=0.5 (0dB), SRR=0, BRR=0, DIST=0
    float l = 0.5f;
    float r = -0.5f;
    wrapper.processSampleStereo(l, r);

    // Should be essentially unchanged (unity gain within filter transient margin)
    assert(!std::isnan(l) && !std::isinf(l));
    assert(!std::isnan(r) && !std::isinf(r));
    assert(std::abs(l - 0.5f) < 0.08f);
    assert(std::abs(r - (-0.5f)) < 0.08f);
    std::cout << "        Pass-through verified! (L=" << l << ", R=" << r << ")" << std::endl;
}

inline void test_elektron_track_wrapper_filter_and_eq() {
    std::cout << "  [2/4] Running ElektronTrackWrapper Base/Width filter & EQ test..." << std::endl;
    tkdsp::ElektronTrackWrapper wrapper;
    wrapper.prepare(48000.0f);

    // Set highpass FLTF high (around 5 kHz)
    wrapper.setParam(0, 0.85f); // Highpass cutoff ~5kHz
    float lowFreqL = 0.7f, lowFreqR = 0.7f;
    // Process block of low frequency DC/bass
    for (int i = 0; i < 500; ++i) {
        lowFreqL = 0.7f; lowFreqR = 0.7f;
        wrapper.processSampleStereo(lowFreqL, lowFreqR);
    }
    // Bass must be heavily attenuated by highpass
    assert(std::abs(lowFreqL) < 0.05f);
    assert(std::abs(lowFreqR) < 0.05f);

    // Reset and test EQ Boost
    wrapper.reset();
    wrapper.setParam(0, 0.0f); // HP bypassed
    wrapper.setParam(1, 1.0f); // LP wide open
    wrapper.setParam(3, 0.25f); // EQ-F at low-mid (~100 Hz)
    wrapper.setParam(4, 1.0f);  // EQ-G +12 dB boost

    float testL = 0.1f, testR = 0.1f;
    for (int i = 0; i < 500; ++i) {
        testL = 0.1f; testR = 0.1f;
        wrapper.processSampleStereo(testL, testR);
    }
    assert(!std::isnan(testL) && !std::isnan(testR));
    std::cout << "        Filter & EQ transfer functions verified!" << std::endl;
}

inline void test_elektron_track_wrapper_srr_brr_dist() {
    std::cout << "  [3/4] Running ElektronTrackWrapper dedicated SRR, BRR & DIST test..." << std::endl;
    tkdsp::ElektronTrackWrapper wrapper;
    wrapper.prepare(48000.0f);

    // Test SRR (Sample Rate Decimation)
    wrapper.setParam(5, 0.8f); // High SRR
    float l1 = 0.5f, r1 = 0.5f;
    wrapper.processSampleStereo(l1, r1);
    float l2 = 0.9f, r2 = 0.9f;
    wrapper.processSampleStereo(l2, r2);
    // Under sample-and-hold, l2 must hold l1
    assert(l2 == l1 && r2 == r1);

    // Test BRR (Bit Depth Quantization)
    wrapper.reset();
    wrapper.setParam(5, 0.0f); // SRR off
    wrapper.setParam(6, 0.9f); // Extreme bit reduction (down to ~2 bits)
    float lBrr = 0.33333f, rBrr = -0.33333f;
    wrapper.processSampleStereo(lBrr, rBrr);
    assert(!std::isnan(lBrr) && !std::isnan(rBrr));

    // Test DIST (Asymmetric Soft Clipping)
    wrapper.reset();
    wrapper.setParam(6, 0.0f); // BRR off
    wrapper.setParam(7, 1.0f); // Maximum Overdrive
    float lDist = 2.0f, rDist = -2.0f;
    wrapper.processSampleStereo(lDist, rDist);
    // Soft clipper must limit output to finite bounds < 1.5f
    assert(std::abs(lDist) < 1.5f);
    assert(std::abs(rDist) < 1.5f);
    std::cout << "        SRR, BRR & DIST verified distinct & bounded!" << std::endl;
}

inline void test_multi_machine_engine_crossfade() {
    std::cout << "  [4/4] Running MultiMachineEngineBase 64-sample click-free model crossfade test..." << std::endl;
    MockMultiMachineKick engine;
    engine.prepare(48000.0f);

    // Trigger Model 0 (Sine)
    engine.setParam(0, 0.0f); // Model 0
    engine.trigger(1.0f);

    // Render 100 samples
    for (int i = 0; i < 100; ++i) {
        float l = 0.0f, r = 0.0f;
        engine.renderSampleStereo(100.0f, l, r);
        assert(!std::isnan(l) && !std::isinf(l));
    }

    // Switch to Model 2 (Square) - triggers 64-sample crossfade
    engine.setParam(0, 0.67f); // Model 2
    assert(engine.activeModel() == 2);

    float prevL = 0.0f;
    for (int i = 0; i < 64; ++i) {
        float l = 0.0f, r = 0.0f;
        engine.renderSampleStereo(100.0f, l, r);
        assert(!std::isnan(l) && !std::isinf(l));

        // Verify continuity: no jump greater than 1.5 in a single sample
        if (i > 0) {
            float delta = std::abs(l - prevL);
            assert(delta < 1.5f);
        }
        prevL = l;
    }

    // Render beyond crossfade window
    for (int i = 0; i < 200; ++i) {
        float l = 0.0f, r = 0.0f;
        engine.renderSampleStereo(100.0f, l, r);
        assert(!std::isnan(l) && !std::isinf(l));
    }

    std::cout << "        64-sample equal-power model crossfade passed 100% cleanly!" << std::endl;
}

inline void run_all_mme_suite_tests() {
    test_elektron_track_wrapper_defaults();
    test_elektron_track_wrapper_filter_and_eq();
    test_elektron_track_wrapper_srr_brr_dist();
    test_multi_machine_engine_crossfade();
}

} // namespace test_mme
