#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <array>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

#include "FastMath.h"
#include "nyarlat/NyarlatAdditiveOscillator.h"
#include "nyarlat/NyarlatAnalogFilter.h"
#include "nyarlat/NyarlatOscillator.h"
#include "nyarlat/NyarlatVoice.h"
#include "nyarlat/AbyssReverb.h"

// Anti-Dead-Code Elimination Sink
template <typename T>
inline void doNotOptimize(T const& val) {
    juce::ignoreUnused(val);
#if defined(_MSC_VER)
    _ReadWriteBarrier();
#else
    asm volatile("" : : "g"(val) : "memory");
#endif
}

struct BenchResult {
    std::string name;
    double nsPerSample;
    double megaSamplesPerSec;
    double cpuPercent48k;
    double p99BlockUs;
    double peakBlockUs;
};

void printResult(const BenchResult& r) {
    std::cout << "  " << std::left << std::setw(42) << r.name
              << " | " << std::right << std::setw(8) << std::fixed << std::setprecision(2) << r.nsPerSample << " ns/smp"
              << " | " << std::setw(7) << std::fixed << std::setprecision(2) << r.megaSamplesPerSec << " MS/s"
              << " | " << std::setw(6) << std::fixed << std::setprecision(2) << r.cpuPercent48k << " %CPU"
              << " | p99: " << std::setw(6) << std::fixed << std::setprecision(1) << r.p99BlockUs << " us"
              << " | peak: " << std::setw(6) << std::fixed << std::setprecision(1) << r.peakBlockUs << " us"
              << std::endl;
}

int main() {
    std::cout << "================================================================================\n";
    std::cout << "  NYARLAT S2A DSP ARCHITECTURE BENCHMARK (48kHz, 128-Sample Blocks)\n";
    std::cout << "  Single-Core Audio Budget: 20,833.33 ns/sample (100.0% CPU)\n";
    std::cout << "================================================================================\n";

    TbdAudio::FastMath::enableFTZDAZ();

    constexpr double sampleRate = 48000.0;
    constexpr int blockSize = 128;
    constexpr int numBlocks = 4000; // ~10.6 seconds of audio simulated per test
    constexpr int warmupBlocks = 500;
    constexpr double nsPerSampleRef = 1000000000.0 / sampleRate; // 20833.33 ns

    std::vector<BenchResult> results;

    // -------------------------------------------------------------------------
    // SUITE A: ADDITIVE OSCILLATOR & LRU CACHE BENCHMARKS
    // -------------------------------------------------------------------------
    std::cout << "\n[SUITE A: Additive Synthesis & 8-Slot Quantized Macro LRU Cache]\n";

    // A1: Full Additive Synthesis (Cache Miss / Accumulation)
    {
        alignas(64) std::array<float, 512> table{};
        auto& engine = Nyarlat::NyarlatAdditiveEngine::getInstance();
        float sink = 0.0f;

        // Warmup
        for (int i = 0; i < 500; ++i) {
            engine.synthesizeCycle(table.data(), 0, (float)i * 0.001f, 0.2f, -3.0f, 0.5f, 0);
            sink += table[0];
        }
        doNotOptimize(sink);

        auto tStart = std::chrono::high_resolution_clock::now();
        constexpr int kIters = 2000;
        for (int i = 0; i < kIters; ++i) {
            // Sweep morph to guarantee cache misses
            engine.synthesizeCycle(table.data(), (uint32_t)(i % 4), (float)i * 0.0004f, 0.2f, -3.0f, 0.5f, 0);
            sink += table[0];
        }
        auto tEnd = std::chrono::high_resolution_clock::now();
        doNotOptimize(sink);

        double totalNs = std::chrono::duration<double, std::nano>(tEnd - tStart).count();
        double nsPerCycle = totalNs / kIters;
        double nsPerSample = nsPerCycle / 512.0;

        BenchResult res{ "1. Additive Cycle Synth (Cache Miss / 64 Partials)", nsPerSample, 1000.0 / nsPerSample, (nsPerSample / nsPerSampleRef) * 100.0, nsPerCycle / 1000.0, nsPerCycle / 1000.0 };
        printResult(res);
        results.push_back(res);
    }

    // A2: Additive Synthesis (LRU Cache Hit)
    {
        alignas(64) std::array<float, 512> table{};
        auto& engine = Nyarlat::NyarlatAdditiveEngine::getInstance();
        float sink = 0.0f;

        // Prime cache slot
        engine.synthesizeCycle(table.data(), 0, 0.5f, 0.2f, -3.0f, 0.5f, 0);

        auto tStart = std::chrono::high_resolution_clock::now();
        constexpr int kIters = 20000;
        for (int i = 0; i < kIters; ++i) {
            // Identical / chord parameters hitting cache
            engine.synthesizeCycle(table.data(), 0, 0.5f, 0.2f, -3.0f, 0.5f, 0);
            sink += table[0];
        }
        auto tEnd = std::chrono::high_resolution_clock::now();
        doNotOptimize(sink);

        double totalNs = std::chrono::duration<double, std::nano>(tEnd - tStart).count();
        double nsPerCycle = totalNs / kIters;
        double nsPerSample = nsPerCycle / 512.0;

        BenchResult res{ "2. Additive Cycle Synth (LRU Cache Hit)", nsPerSample, 1000.0 / nsPerSample, (nsPerSample / nsPerSampleRef) * 100.0, nsPerCycle / 1000.0, nsPerCycle / 1000.0 };
        printResult(res);
        results.push_back(res);
    }

    // -------------------------------------------------------------------------
    // SUITE B: SILKY ANALOG ZDF FILTER BENCHMARKS
    // -------------------------------------------------------------------------
    std::cout << "\n[SUITE B: Silky Analog ZDF Filter (Optimized Shared Pre-Computation)]\n";

    auto benchFilter = [&](int mode, const std::string& label) {
        Nyarlat::NyarlatAnalogFilter filter;
        filter.prepare(sampleRate);
        float l = 0.5f, r = 0.5f;

        for (int b = 0; b < warmupBlocks; ++b) {
            for (int s = 0; s < blockSize; ++s) {
                filter.processStereo(l, r, 2500.0f, 0.5f, mode, 0.2f);
            }
        }
        doNotOptimize(l + r);

        std::vector<double> blockTimesUs(numBlocks);
        auto tStart = std::chrono::high_resolution_clock::now();

        for (int b = 0; b < numBlocks; ++b) {
            auto bStart = std::chrono::high_resolution_clock::now();
            for (int s = 0; s < blockSize; ++s) {
                filter.processStereo(l, r, 2500.0f, 0.5f, mode, 0.2f);
            }
            auto bEnd = std::chrono::high_resolution_clock::now();
            blockTimesUs[b] = std::chrono::duration<double, std::micro>(bEnd - bStart).count();
        }
        auto tEnd = std::chrono::high_resolution_clock::now();
        doNotOptimize(l + r);

        double totalNs = std::chrono::duration<double, std::nano>(tEnd - tStart).count();
        double totalSamples = static_cast<double>(numBlocks * blockSize);
        double nsPerSample = totalNs / totalSamples;
        std::sort(blockTimesUs.begin(), blockTimesUs.end());
        double p99 = blockTimesUs[static_cast<size_t>(numBlocks * 0.99)];
        double peak = blockTimesUs.back();

        BenchResult res{ label, nsPerSample, 1000.0 / nsPerSample, (nsPerSample / nsPerSampleRef) * 100.0, p99, peak };
        printResult(res);
        results.push_back(res);
    };

    benchFilter(0, "3. Analog Filter: 12dB LP SVF Stereo");
    benchFilter(1, "4. Analog Filter: 24dB Ladder Stereo");
    benchFilter(2, "5. Analog Filter: 12dB BP SVF Stereo");

    // -------------------------------------------------------------------------
    // SUITE C: FULL POLYPHONIC VOICE SCALING
    // -------------------------------------------------------------------------
    std::cout << "\n[SUITE C: Full Voice & Polyphony Scaling (renderBlockAdd)]\n";

    auto benchPolyphony = [&](int numVoices, const std::string& label) {
        std::vector<Nyarlat::NyarlatVoice> voices(numVoices);
        for (int v = 0; v < numVoices; ++v) {
            voices[v].setVoiceIndex(v);
            voices[v].prepare(sampleRate);
            voices[v].noteOn(60 + (v % 12), 0.8f, 0.0f);
        }

        std::vector<float> bufL(blockSize, 0.0f);
        std::vector<float> bufR(blockSize, 0.0f);

        float oscBanks[3] = { 0.0f, 0.0f, 0.0f };
        float oscMorphs[3] = { 0.2f, 0.5f, 0.8f };
        float oscOddEvens[3] = { 0.0f, 0.0f, 0.0f };
        float oscTilts[3] = { 0.0f, 0.0f, 0.0f };
        float oscFormants[3] = { 0.5f, 0.5f, 0.5f };
        int oscMasks[3] = { 399, 399, 399 };
        int oscOctaves[3] = { 2, 2, 2 };
        bool oscSpeedLocks[3] = { false, false, false };
        float oscSpeeds[3] = { 2.0f, 2.0f, 2.0f };
        int oscLoops[3] = { 0, 0, 0 };
        float randAtten[18] = { 0 };

        for (int b = 0; b < warmupBlocks; ++b) {
            std::fill(bufL.begin(), bufL.end(), 0.0f);
            std::fill(bufR.begin(), bufR.end(), 0.0f);
            for (int v = 0; v < numVoices; ++v) {
                voices[v].renderBlockAdd(
                    bufL.data(), bufR.data(), blockSize, numVoices,
                    oscBanks, oscMorphs, oscOddEvens, oscTilts, oscFormants, oscMasks,
                    oscOctaves, oscSpeedLocks, oscSpeeds, oscLoops,
                    1.0f, 0.7f, 0.7f,
                    0.05f, 0.5f, 0.0f,
                    0.5f, 0.0f,
                    randAtten, 0.0f, 0.0f, 0.0f,
                    1500.0f, 0.5f, 0, 0.2f,
                    0.5f, 0.0f, 0.0f,
                    0.8f, 0.0f, 0.0f, 0.0f
                );
            }
        }
        doNotOptimize(bufL[0] + bufR[0]);

        std::vector<double> blockTimesUs(numBlocks);
        auto tStart = std::chrono::high_resolution_clock::now();

        for (int b = 0; b < numBlocks; ++b) {
            auto bStart = std::chrono::high_resolution_clock::now();
            std::fill(bufL.begin(), bufL.end(), 0.0f);
            std::fill(bufR.begin(), bufR.end(), 0.0f);
            for (int v = 0; v < numVoices; ++v) {
                voices[v].renderBlockAdd(
                    bufL.data(), bufR.data(), blockSize, numVoices,
                    oscBanks, oscMorphs, oscOddEvens, oscTilts, oscFormants, oscMasks,
                    oscOctaves, oscSpeedLocks, oscSpeeds, oscLoops,
                    1.0f, 0.7f, 0.7f,
                    0.05f, 0.5f, 0.0f,
                    0.5f, 0.0f,
                    randAtten, 0.0f, 0.0f, 0.0f,
                    1500.0f, 0.5f, 0, 0.2f,
                    0.5f, 0.0f, 0.0f,
                    0.8f, 0.0f, 0.0f, 0.0f
                );
            }
            auto bEnd = std::chrono::high_resolution_clock::now();
            blockTimesUs[b] = std::chrono::duration<double, std::micro>(bEnd - bStart).count();
        }
        auto tEnd = std::chrono::high_resolution_clock::now();
        doNotOptimize(bufL[0] + bufR[0]);

        double totalNs = std::chrono::duration<double, std::nano>(tEnd - tStart).count();
        double totalSamples = static_cast<double>(numBlocks * blockSize);
        double nsPerSample = totalNs / totalSamples;
        std::sort(blockTimesUs.begin(), blockTimesUs.end());
        double p99 = blockTimesUs[static_cast<size_t>(numBlocks * 0.99)];
        double peak = blockTimesUs.back();

        BenchResult res{ label, nsPerSample, 1000.0 / nsPerSample, (nsPerSample / nsPerSampleRef) * 100.0, p99, peak };
        printResult(res);
        results.push_back(res);
    };

    benchPolyphony(1,  "6. Polyphony: 1 Active Voice");
    benchPolyphony(4,  "7. Polyphony: 4 Active Voices");
    benchPolyphony(8,  "8. Polyphony: 8 Active Voices");
    benchPolyphony(16, "9. Polyphony: 16 Active Voices (MAX)");

    // -------------------------------------------------------------------------
    // SUITE D: 8x UNISON SWARM TORTURE
    // -------------------------------------------------------------------------
    std::cout << "\n[SUITE D: 8x Unison Swarm Torture (24 Additive Oscillators)]\n";
    {
        constexpr int numVoices = 8;
        std::vector<Nyarlat::NyarlatVoice> voices(numVoices);
        for (int v = 0; v < numVoices; ++v) {
            voices[v].setVoiceIndex(v);
            voices[v].prepare(sampleRate);
            const float detuneSemi = ((float)v - 3.5f) / 3.5f * 0.15f;
            const float hz = 440.0f * std::pow(2.0f, (60.0f + detuneSemi - 69.0f) / 12.0f);
            voices[v].noteOn(60, 0.8f, 0.0f, 0.0f, hz);
        }

        std::vector<float> bufL(blockSize, 0.0f);
        std::vector<float> bufR(blockSize, 0.0f);

        float oscBanks[3] = { 0.0f, 0.0f, 0.0f };
        float oscMorphs[3] = { 0.3f, 0.6f, 0.9f };
        float oscOddEvens[3] = { 0.2f, -0.2f, 0.0f };
        float oscTilts[3] = { -2.0f, 0.0f, 1.0f };
        float oscFormants[3] = { 0.4f, 0.6f, 0.8f };
        int oscMasks[3] = { 399, 399, 399 };
        int oscOctaves[3] = { 2, 2, 2 };
        bool oscSpeedLocks[3] = { false, false, false };
        float oscSpeeds[3] = { 2.0f, 2.0f, 2.0f };
        int oscLoops[3] = { 0, 0, 0 };
        float randAtten[18] = { 0.1f };

        std::vector<double> blockTimesUs(numBlocks);
        auto tStart = std::chrono::high_resolution_clock::now();

        for (int b = 0; b < numBlocks; ++b) {
            auto bStart = std::chrono::high_resolution_clock::now();
            std::fill(bufL.begin(), bufL.end(), 0.0f);
            std::fill(bufR.begin(), bufR.end(), 0.0f);
            for (int v = 0; v < numVoices; ++v) {
                voices[v].renderBlockAdd(
                    bufL.data(), bufR.data(), blockSize, numVoices,
                    oscBanks, oscMorphs, oscOddEvens, oscTilts, oscFormants, oscMasks,
                    oscOctaves, oscSpeedLocks, oscSpeeds, oscLoops,
                    1.0f, 0.7f, 0.7f,
                    0.05f, 0.5f, 0.0f,
                    0.5f, 0.0f,
                    randAtten, 0.2f, 0.1f, 0.2f,
                    1800.0f, 0.6f, 1, 0.3f,
                    0.5f, 0.0f, 0.0f,
                    0.8f, 0.0f, 0.0f, 0.0f
                );
            }
            auto bEnd = std::chrono::high_resolution_clock::now();
            blockTimesUs[b] = std::chrono::duration<double, std::micro>(bEnd - bStart).count();
        }
        auto tEnd = std::chrono::high_resolution_clock::now();
        doNotOptimize(bufL[0] + bufR[0]);

        double totalNs = std::chrono::duration<double, std::nano>(tEnd - tStart).count();
        double totalSamples = static_cast<double>(numBlocks * blockSize);
        double nsPerSample = totalNs / totalSamples;
        std::sort(blockTimesUs.begin(), blockTimesUs.end());
        double p99 = blockTimesUs[static_cast<size_t>(numBlocks * 0.99)];
        double peak = blockTimesUs.back();

        BenchResult res{ "10. 8x Unison Swarm (24 Additive Oscs)", nsPerSample, 1000.0 / nsPerSample, (nsPerSample / nsPerSampleRef) * 100.0, p99, peak };
        printResult(res);
        results.push_back(res);
    }

    // -------------------------------------------------------------------------
    // SUITE E: MASTER EFFECTS CHAIN
    // -------------------------------------------------------------------------
    std::cout << "\n[SUITE E: Master Effects Chain]\n";
    {
        Nyarlat::AbyssReverb reverb;
        reverb.prepare(sampleRate);
        float l = 0.5f, r = 0.5f;

        for (int b = 0; b < warmupBlocks; ++b) {
            for (int s = 0; s < blockSize; ++s) {
                reverb.processStereo(l, r, 5.0f, 0.5f, 0.4f);
            }
        }
        doNotOptimize(l + r);

        std::vector<double> blockTimesUs(numBlocks);
        auto tStart = std::chrono::high_resolution_clock::now();

        for (int b = 0; b < numBlocks; ++b) {
            auto bStart = std::chrono::high_resolution_clock::now();
            for (int s = 0; s < blockSize; ++s) {
                reverb.processStereo(l, r, 5.0f, 0.5f, 0.4f);
            }
            auto bEnd = std::chrono::high_resolution_clock::now();
            blockTimesUs[b] = std::chrono::duration<double, std::micro>(bEnd - bStart).count();
        }
        auto tEnd = std::chrono::high_resolution_clock::now();
        doNotOptimize(l + r);

        double totalNs = std::chrono::duration<double, std::nano>(tEnd - tStart).count();
        double totalSamples = static_cast<double>(numBlocks * blockSize);
        double nsPerSample = totalNs / totalSamples;
        std::sort(blockTimesUs.begin(), blockTimesUs.end());
        double p99 = blockTimesUs[static_cast<size_t>(numBlocks * 0.99)];
        double peak = blockTimesUs.back();

        BenchResult res{ "11. Abyss Reverb Stereo (5s decay)", nsPerSample, 1000.0 / nsPerSample, (nsPerSample / nsPerSampleRef) * 100.0, p99, peak };
        printResult(res);
        results.push_back(res);
    }

    std::cout << "\n================================================================================\n";
    std::cout << "  BENCHMARK SUITE COMPLETE\n";
    std::cout << "================================================================================\n";

    return 0;
}
