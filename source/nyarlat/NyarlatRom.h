#pragma once

#include <vector>
#include <string>
#include <cstring>
#include <cstdio>
#include <atomic>
#include <cmath>
#include <juce_core/juce_core.h>
#include "NyarlatRomMetadata.h"
#include "../FastMath.h"

namespace Nyarlat {

class AkwfRom {
public:
    static AkwfRom& getInstance() {
        static AkwfRom instance;
        return instance;
    }

    bool isLoaded() const noexcept {
        return isLoaded_.load(std::memory_order_acquire);
    }

    void load() {
        if (isLoaded_.load(std::memory_order_acquire))
            return;

        std::vector<std::string> paths;

        // 1. Binary directory / VST3 contents
        auto appFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
        auto contentsDir = appFile.getParentDirectory().getFullPathName().toStdString();
        paths.push_back(contentsDir + "/Resources/nyarlat_akwf_rom.bin");
        paths.push_back(contentsDir + "/nyarlat_akwf_rom.bin");

        // 2. Installed VST3 Common Files
#if defined(_WIN32)
        paths.push_back("C:/Program Files/Common Files/VST3/Nyarlat.vst3/Contents/Resources/nyarlat_akwf_rom.bin");
#endif

        // 3. Local Development Source Paths
        paths.push_back("C:/Dev/TheKlangSuite/assets/nyarlat_akwf_rom.bin");
        paths.push_back("assets/nyarlat_akwf_rom.bin");
        paths.push_back("../assets/nyarlat_akwf_rom.bin");

        for (const auto& p : paths) {
            if (tryLoadFile(p.c_str())) {
                isLoaded_.store(true, std::memory_order_release);
                return;
            }
        }

        // Fallback: generate pure bandlimited sine in wave 0
        createFallbackTables();
        isLoaded_.store(true, std::memory_order_release);
    }

    // Direct access to a 512-sample waveform buffer
    const float* getWaveform(uint32_t bankIdx, float waveMorphNorm) const noexcept {
        if (bankIdx >= NyarlatRom::kNumBanks || samples_.empty())
            return fallbackSine_.data();

        const auto& bank = NyarlatRom::kBanks[bankIdx];
        if (bank.count == 0)
            return fallbackSine_.data();

        float clampedMorph = juce::jlimit(0.0f, 1.0f, waveMorphNorm);
        float pos = clampedMorph * static_cast<float>(bank.count - 1);
        uint32_t idx = static_cast<uint32_t>(pos);
        if (idx >= bank.count) idx = bank.count - 1;

        uint32_t globalWaveIdx = bank.offset + idx;
        return &samples_[globalWaveIdx * NyarlatRom::kTableSize];
    }

    // Interpolates between wave A and wave B within a bank into a user-provided 512-float destination
    void interpolateWaveform(uint32_t bankIdx, float waveMorphNorm, float* dest512) const noexcept {
        if (bankIdx >= NyarlatRom::kNumBanks || samples_.empty()) {
            std::memcpy(dest512, fallbackSine_.data(), NyarlatRom::kTableSize * sizeof(float));
            return;
        }

        const auto& bank = NyarlatRom::kBanks[bankIdx];
        if (bank.count <= 1) {
            const float* w = getWaveform(bankIdx, 0.0f);
            std::memcpy(dest512, w, NyarlatRom::kTableSize * sizeof(float));
            return;
        }

        float clampedMorph = juce::jlimit(0.0f, 1.0f, waveMorphNorm);
        float pos = clampedMorph * static_cast<float>(bank.count - 1);
        uint32_t idxA = static_cast<uint32_t>(pos);
        uint32_t idxB = idxA + 1;
        if (idxB >= bank.count) idxB = bank.count - 1;
        float alpha = pos - static_cast<float>(idxA);

        const float* wA = &samples_[(bank.offset + idxA) * NyarlatRom::kTableSize];
        const float* wB = &samples_[(bank.offset + idxB) * NyarlatRom::kTableSize];

        // Equal-power circular morphing via fast minimax normalized trig
        float cosA = TbdAudio::FastMath::fastCosNorm(alpha * 0.25f);
        float sinB = TbdAudio::FastMath::fastSinNorm(alpha * 0.25f);

        for (uint32_t i = 0; i < NyarlatRom::kTableSize; ++i) {
            dest512[i] = wA[i] * cosA + wB[i] * sinB;
        }
    }

    // Interpolates 64 partial spectral seeds (magnitudes & shortest-arc phases) within a bank
    void interpolateSpectralSeed(uint32_t bankIdx, float waveMorphNorm,
                                 float* outMags64, float* outPhases64) const noexcept {
        if (bankIdx >= NyarlatRom::kNumBanks || spectralMags_.empty()) {
            std::fill(outMags64, outMags64 + 64, 0.0f);
            std::fill(outPhases64, outPhases64 + 64, 0.0f);
            outMags64[0] = 1.0f; // Pure fundamental sine fallback
            return;
        }

        const auto& bank = NyarlatRom::kBanks[bankIdx];
        if (bank.count == 0) {
            std::fill(outMags64, outMags64 + 64, 0.0f);
            std::fill(outPhases64, outPhases64 + 64, 0.0f);
            outMags64[0] = 1.0f;
            return;
        }

        float clampedMorph = juce::jlimit(0.0f, 1.0f, waveMorphNorm);
        if (bank.count <= 1) {
            const float* mag = &spectralMags_[bank.offset * 64];
            const float* ph = &spectralPhases_[bank.offset * 64];
            std::memcpy(outMags64, mag, 64 * sizeof(float));
            std::memcpy(outPhases64, ph, 64 * sizeof(float));
            return;
        }

        float pos = clampedMorph * static_cast<float>(bank.count - 1);
        uint32_t idxA = static_cast<uint32_t>(pos);
        uint32_t idxB = idxA + 1;
        if (idxB >= bank.count) idxB = bank.count - 1;
        float alpha = pos - static_cast<float>(idxA);

        const float* magA = &spectralMags_[(bank.offset + idxA) * 64];
        const float* magB = &spectralMags_[(bank.offset + idxB) * 64];
        const float* phA = &spectralPhases_[(bank.offset + idxA) * 64];
        const float* phB = &spectralPhases_[(bank.offset + idxB) * 64];

        constexpr float kPi = 3.14159265358979323846f;
        constexpr float kTwoPi = 6.28318530717958647692f;

        for (int k = 0; k < 64; ++k) {
            // Linear magnitude interpolation
            outMags64[k] = (1.0f - alpha) * magA[k] + alpha * magB[k];

            // Shortest-arc phase interpolation
            float diff = phB[k] - phA[k];
            while (diff > kPi) diff -= kTwoPi;
            while (diff < -kPi) diff += kTwoPi;
            outPhases64[k] = phA[k] + alpha * diff;
        }
    }

private:
    AkwfRom() {
        fallbackSine_.resize(NyarlatRom::kTableSize);
        for (uint32_t i = 0; i < NyarlatRom::kTableSize; ++i) {
            fallbackSine_[i] = std::sin(2.0f * 3.1415926535f * static_cast<float>(i) / static_cast<float>(NyarlatRom::kTableSize));
        }
    }

    bool tryLoadFile(const char* filepath) noexcept {
        FILE* fp = nullptr;
#if defined(_WIN32)
        fopen_s(&fp, filepath, "rb");
#else
        fp = std::fopen(filepath, "rb");
#endif
        if (!fp) return false;

        char magic[4];
        if (std::fread(magic, 1, 4, fp) != 4 || std::memcmp(magic, "NYRT", 4) != 0) {
            std::fclose(fp);
            return false;
        }

        uint32_t header[4]; // version, numBanks, totalWaves, tableSize
        if (std::fread(header, sizeof(uint32_t), 4, fp) != 4) {
            std::fclose(fp);
            return false;
        }

        uint32_t version = header[0];
        uint32_t numBanks = header[1];
        uint32_t totalWaves = header[2];
        uint32_t tableSize = header[3];
        uint32_t numPartials = 64;

        if (version >= 2) {
            if (std::fread(&numPartials, sizeof(uint32_t), 1, fp) != 1) {
                std::fclose(fp);
                return false;
            }
        }

        // Skip bank directory
        std::fseek(fp, numBanks * (32 + 8), SEEK_CUR);

        if (version >= 2) {
            size_t totalPartials = static_cast<size_t>(totalWaves) * numPartials;
            spectralMags_.resize(totalPartials);
            spectralPhases_.resize(totalPartials);
            if (std::fread(spectralMags_.data(), sizeof(float), totalPartials, fp) != totalPartials ||
                std::fread(spectralPhases_.data(), sizeof(float), totalPartials, fp) != totalPartials) {
                std::fclose(fp);
                return false;
            }
        }

        size_t totalSamples = static_cast<size_t>(totalWaves) * tableSize;
        samples_.resize(totalSamples);
        size_t readCount = std::fread(samples_.data(), sizeof(float), totalSamples, fp);
        std::fclose(fp);

        return (readCount == totalSamples);
    }

    void createFallbackTables() {
        samples_.resize(NyarlatRom::kTableSize);
        std::memcpy(samples_.data(), fallbackSine_.data(), NyarlatRom::kTableSize * sizeof(float));
        spectralMags_.assign(64, 0.0f);
        spectralPhases_.assign(64, 0.0f);
        spectralMags_[0] = 1.0f;
    }

    std::vector<float> samples_;
    std::vector<float> spectralMags_;
    std::vector<float> spectralPhases_;
    std::vector<float> fallbackSine_;
    std::atomic<bool> isLoaded_{ false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AkwfRom)
};

} // namespace Nyarlat
