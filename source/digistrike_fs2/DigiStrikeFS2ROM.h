#pragma once

#include "../FastMath.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

// ==============================================================================
// DigiStrikeFS2ROM.h — Sovereign Multi-Voice Contiguous PCM ROM Bank (DSR3)
// ==============================================================================
// Part of The Klang Suite SOCEP Engine. Copyright (c) 2026 Klang Industries.
// Clean-Room IP Compliance: Zero commercial trademarks.
// ==============================================================================

namespace tkdsp {

namespace FastMath = TbdAudio::FastMath;

#pragma pack(push, 1)
struct DSR3Header {
    char     magic[4];           // "DSR3"
    uint32_t version;            // 3
    uint32_t numVoices;          // e.g. 2085
    uint32_t numPools;           // 161 (8 x 20 + 1 Secret Core)
    uint32_t defaultVoiceIndex;  // Index of FAIRL_ORCH5 in STAB/Fairlight
    uint32_t totalPcmSamples;    // Total 16-bit PCM samples
    uint32_t pcmOffsetBytes;     // Offset to PCM payload (64)
    uint32_t poolRangesOffset;   // Offset to PoolRange array
    uint32_t metaOffset;         // Offset to VoiceMeta array
    uint32_t stringTableOffset;  // Offset to string table
    uint32_t reserved[6];
};

struct DSR3PoolRange {
    uint32_t firstVoiceIndex;
    uint32_t voiceCount;
};

struct DSR3VoiceMeta {
    uint32_t pcmOffset;          // offset in int16_t samples from start of ROM buffer
    uint32_t sampleCount;        // active sample count (excluding padding)
    uint32_t sampleRate;         // native sample rate (e.g. 44100, 14080, 11025)
    float    nativeDurationSec;  // duration in seconds
    uint32_t nameOffset;         // offset into string table
    uint8_t  rootMidi;           // root MIDI note (36 for kick, 60 for stab/C4)
    uint8_t  kind;               // 0=ONESHOT, 1=CYCLE
    uint16_t flags;
    uint32_t voiceKey;           // CRC32 of clean canonical ID
    uint32_t reserved;
};
#pragma pack(pop)

class DigiStrikeFS2ROM {
public:
    static constexpr size_t kNumCategories = 11; // BD, SN, TM, CP, HH, CY, LN, MC, FX, ST, BS
    static constexpr size_t kNumLineages   = 20; // 20 Sovereign Hardware Families
    static constexpr size_t kNumHeroSlots  = kNumCategories * kNumLineages; // 220
    static constexpr size_t kSecretCoreSlot = 220;
    static constexpr size_t kNumSlots      = 221;

    static const DigiStrikeFS2ROM& getInstance() noexcept {
        static const DigiStrikeFS2ROM instance;
        return instance;
    }

    [[nodiscard]] bool isLoaded() const noexcept {
        return isLoaded_.load(std::memory_order_acquire);
    }

    [[nodiscard]] size_t getNumVoices() const noexcept {
        return isLoaded() ? header_.numVoices : 0;
    }

    [[nodiscard]] size_t getNumPools() const noexcept {
        return isLoaded() ? header_.numPools : 0;
    }

    [[nodiscard]] uint32_t getDefaultVoiceIndex() const noexcept {
        return isLoaded() ? header_.defaultVoiceIndex : 0;
    }

    [[nodiscard]] DSR3PoolRange getPoolRange(int poolIdx) const noexcept {
        if (!isLoaded() || poolIdx < 0 || poolIdx >= static_cast<int>(header_.numPools) || pools_ == nullptr) {
            return {0, 0};
        }
        return pools_[poolIdx];
    }

    [[nodiscard]] const DSR3VoiceMeta* getVoice(size_t voiceIdx) const noexcept {
        if (!isLoaded() || voiceIdx >= header_.numVoices || voices_ == nullptr) {
            return nullptr;
        }
        return &voices_[voiceIdx];
    }

    [[nodiscard]] const char* getVoiceName(size_t voiceIdx) const noexcept {
        if (!isLoaded() || voiceIdx >= header_.numVoices || voices_ == nullptr || stringTable_ == nullptr) {
            return "Empty Voice";
        }
        return stringTable_ + voices_[voiceIdx].nameOffset;
    }

    /**
     * @brief Continuous 4-point Hermite interpolation across contiguous 16-bit PCM.
     * 100% Branchless in the inner loop due to baker's 2-lead zero and 4-trail zero padding.
     */
    [[nodiscard]] inline float sampleHermite(size_t voiceIdx, double samplePos) const noexcept {
        if (!isLoaded() || voiceIdx >= header_.numVoices || pcm_ == nullptr) {
            return 0.0f;
        }
        const auto& v = voices_[voiceIdx];
        if (samplePos < 0.0 || samplePos >= static_cast<double>(v.sampleCount)) {
            return 0.0f;
        }

        const int i0 = static_cast<int>(std::floor(samplePos));
        const float u = static_cast<float>(samplePos - static_cast<double>(i0));

        // Offset directly to sample 0
        const int16_t* s = pcm_ + v.pcmOffset;

        // Branchless reads: i0 - 1 reads the lead zero when i0 == 0;
        // i0 + 1 and i0 + 2 read trailing zeros when approaching tail.
        const float y_m1 = static_cast<float>(s[i0 - 1]) * (1.0f / 32768.0f);
        const float y_0  = static_cast<float>(s[i0])     * (1.0f / 32768.0f);
        const float y_1  = static_cast<float>(s[i0 + 1]) * (1.0f / 32768.0f);
        const float y_2  = static_cast<float>(s[i0 + 2]) * (1.0f / 32768.0f);

        const float c0 = y_0;
        const float c1 = 0.5f * (y_1 - y_m1);
        const float c2 = y_m1 - 2.5f * y_0 + 2.0f * y_1 - 0.5f * y_2;
        const float c3 = 0.5f * (y_2 - y_m1) + 1.5f * (y_0 - y_1);

        return ((c3 * u + c2) * u + c1) * u + c0;
    }

    /**
     * @brief Backward-compatible wavetable sample reader.
     */
    [[nodiscard]] inline float readSample(int slotIdx, float morphNorm, float framePos, double oscPhase) const noexcept {
        if (!isLoaded()) return 0.0f;
        slotIdx = std::clamp(slotIdx, 0, static_cast<int>(header_.numPools - 1));
        const auto pool = getPoolRange(slotIdx);
        if (pool.voiceCount == 0) return 0.0f;

        const float span = static_cast<float>(pool.voiceCount - 1);
        const float mPos = std::clamp(morphNorm * span, 0.0f, span);
        const size_t vIdx = pool.firstVoiceIndex + static_cast<size_t>(std::round(mPos));
        
        const auto* v = getVoice(vIdx);
        if (v == nullptr || v->sampleCount == 0) return 0.0f;

        // Map oscPhase (0..2048) or framePos across sample count
        double sPos = std::fmod(oscPhase, static_cast<double>(v->sampleCount));
        return sampleHermite(vIdx, sPos);
    }

private:
    DigiStrikeFS2ROM() {
        loadAsset();
    }

    void loadAsset() noexcept {
        std::vector<std::string> paths;

        // 1. Environment Variable
        if (const char* env = std::getenv("DIGISTRIKE_ROM_PATH")) {
            if (env[0] != '\0') paths.emplace_back(env);
        }

        // 2. Platform VST3 Bundle Discovery
#if defined(_WIN32)
        char modulePath[MAX_PATH] = {0};
        HMODULE hMod = nullptr;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                           GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           reinterpret_cast<LPCSTR>(&DigiStrikeFS2ROM::getInstance),
                           &hMod);
        if (hMod && GetModuleFileNameA(hMod, modulePath, MAX_PATH) > 0) {
            std::string mp(modulePath);
            // Replace backslashes
            for (char& c : mp) if (c == '\\') c = '/';
            // VST3 Windows structure: .../DigiStrike FS2.vst3/Contents/x86_64-win/DigiStrike FS2.vst3
            // Go up two dirs to Contents/, then into Resources/
            auto lastSlash = mp.rfind('/');
            if (lastSlash != std::string::npos) {
                auto binDir = mp.substr(0, lastSlash);
                auto secondSlash = binDir.rfind('/');
                if (secondSlash != std::string::npos) {
                    auto contentsDir = binDir.substr(0, secondSlash);
                    paths.push_back(contentsDir + "/Resources/digistrike_fs2_rom.bin");
                }
            }
        }
        paths.push_back("C:/Program Files/Common Files/VST3/DigiStrike FS2.vst3/Contents/Resources/digistrike_fs2_rom.bin");
#endif

        // 3. Local Workspace / Source Directory
        paths.push_back("C:/Dev/TheKlangSuite/assets/digistrike_fs2_rom.bin");
        paths.push_back("assets/digistrike_fs2_rom.bin");
        paths.push_back("../assets/digistrike_fs2_rom.bin");

        for (const auto& p : paths) {
            if (tryLoadFile(p.c_str())) {
                isLoaded_.store(true, std::memory_order_release);
                return;
            }
        }

        // Fatal notification: Zero silent procedural fallback in modern build
        std::fprintf(stderr, "[DigiStrikeFS2ROM] CRITICAL: digistrike_fs2_rom.bin could not be found in any candidate path!\n");
    }

    bool tryLoadFile(const char* filepath) noexcept {
        FILE* fp = nullptr;
#if defined(_WIN32)
        fopen_s(&fp, filepath, "rb");
#else
        fp = fopen(filepath, "rb");
#endif
        if (!fp) return false;

        fseek(fp, 0, SEEK_END);
        long fileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        if (fileSize < static_cast<long>(sizeof(DSR3Header))) {
            fclose(fp);
            return false;
        }

        buffer_.resize(static_cast<size_t>(fileSize));
        size_t readBytes = fread(buffer_.data(), 1, buffer_.size(), fp);
        fclose(fp);

        if (readBytes != buffer_.size()) {
            buffer_.clear();
            return false;
        }

        // Validate DSR3 Header
        std::memcpy(&header_, buffer_.data(), sizeof(DSR3Header));
        if (std::memcmp(header_.magic, "DSR3", 4) != 0 || header_.version != 3) {
            buffer_.clear();
            return false;
        }

        // Bind memory pointers into contiguous block
        pcm_ = reinterpret_cast<const int16_t*>(buffer_.data() + header_.pcmOffsetBytes);
        pools_ = reinterpret_cast<const DSR3PoolRange*>(buffer_.data() + header_.poolRangesOffset);
        voices_ = reinterpret_cast<const DSR3VoiceMeta*>(buffer_.data() + header_.metaOffset);
        stringTable_ = reinterpret_cast<const char*>(buffer_.data() + header_.stringTableOffset);

        return true;
    }

    std::vector<uint8_t> buffer_;
    DSR3Header header_{};
    const int16_t* pcm_{nullptr};
    const DSR3PoolRange* pools_{nullptr};
    const DSR3VoiceMeta* voices_{nullptr};
    const char* stringTable_{nullptr};
    std::atomic<bool> isLoaded_{false};
};

} // namespace tkdsp
