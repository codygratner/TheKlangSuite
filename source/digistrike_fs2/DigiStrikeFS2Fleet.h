#pragma once

#include "DigiStrikeFS2ROM.h"
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <string_view>

// ==============================================================================
// DigiStrikeFS2Fleet.h — Sovereign Multi-Lineage Active Voice Fleet Resolution
// ==============================================================================
// Part of The Klang Suite SOCEP Engine. Copyright (c) 2026 Klang Industries.
// Clean-Room IP Compliance: Zero commercial trademarks.
// ==============================================================================

namespace DigiStrikeFS2Fleet {

inline constexpr std::array<const char*, 20> kLineageNames = {{
    "Crown", "Apex", "Empyrean", "Falcon", "K-Works", "Compact", "Tokyo", "Gothenburg", "Circuits", "Phase",
    "Studio", "Keio", "Nippon", "Adios", "Fairlight", "Synclavier", "Ensoniq", "Simmons", "Jomox", "Curios"
}};

inline uint32_t sanitizeMask(uint32_t mask) noexcept {
    mask &= 0xFFFFFu;
    return (mask != 0) ? mask : (1u << 14); // Default to Fairlight (bit 14) matching FAIRL_ORCH5
}

struct VoiceRef {
    uint32_t romIndex{0};
    int lineage{14};
    int localIndex{0};
    int localCount{0};
    int globalIndex{0};
    int totalVoices{0};
};

struct ResolvedVoicePair {
    uint32_t voiceA{0};
    uint32_t voiceB{0};
    float morphFrac{0.0f};
    int globalIndex{0};
    int totalVoices{0};
};

// Computes total active voices across lineages enabled in mask for category cat
inline int getActiveFleetCount(const tkdsp::DigiStrikeFS2ROM& rom, int cat, uint32_t mask) noexcept {
    cat = std::clamp(cat, 0, 9);
    mask = sanitizeMask(mask);
    int total = 0;
    for (int lin = 0; lin < 20; ++lin) {
        if ((mask & (1u << lin)) != 0) {
            auto pr = rom.getPoolRange(cat * 20 + lin);
            total += static_cast<int>(pr.voiceCount);
        }
    }
    return total;
}

// Resolves a global fleet index g in [0, totalVoices - 1] to VoiceRef
inline bool resolveFleetIndex(const tkdsp::DigiStrikeFS2ROM& rom, int cat, uint32_t mask, int g, VoiceRef& out) noexcept {
    cat = std::clamp(cat, 0, 9);
    mask = sanitizeMask(mask);
    int total = 0;

    for (int lin = 0; lin < 20; ++lin) {
        if ((mask & (1u << lin)) != 0) {
            total += static_cast<int>(rom.getPoolRange(cat * 20 + lin).voiceCount);
        }
    }
    if (total <= 0) return false;
    g = std::clamp(g, 0, total - 1);

    int accum = 0;
    for (int lin = 0; lin < 20; ++lin) {
        if ((mask & (1u << lin)) != 0) {
            auto pr = rom.getPoolRange(cat * 20 + lin);
            const int count = static_cast<int>(pr.voiceCount);
            if (count > 0) {
                if (g < accum + count) {
                    const int local = g - accum;
                    out.romIndex = pr.firstVoiceIndex + static_cast<uint32_t>(local);
                    out.lineage = lin;
                    out.localIndex = local;
                    out.localCount = count;
                    out.globalIndex = g;
                    out.totalVoices = total;
                    return true;
                }
                accum += count;
            }
        }
    }
    return false;
}

// Resolves normalized morph01 in [0.0, 1.0] to VoiceA, VoiceB, and morphFrac (O(20), stack-only, real-time safe)
inline ResolvedVoicePair resolveVoicePair(const tkdsp::DigiStrikeFS2ROM& rom, int cat, uint32_t mask, float morph01) noexcept {
    cat = std::clamp(cat, 0, 9);
    mask = sanitizeMask(mask);
    morph01 = std::clamp(morph01, 0.0f, 1.0f);

    std::array<tkdsp::DSR3PoolRange, 20> activePools{};
    int numActivePools = 0;
    int totalVoices = 0;

    for (int lin = 0; lin < 20; ++lin) {
        if ((mask & (1u << lin)) != 0) {
            auto pr = rom.getPoolRange(cat * 20 + lin);
            if (pr.voiceCount > 0) {
                activePools[static_cast<size_t>(numActivePools++)] = pr;
                totalVoices += static_cast<int>(pr.voiceCount);
            }
        }
    }

    if (totalVoices <= 0) {
        const uint32_t defVoice = rom.getDefaultVoiceIndex();
        return { defVoice, defVoice, 0.0f, 0, 0 };
    }

    if (totalVoices == 1) {
        const uint32_t onlyVoice = activePools[0].firstVoiceIndex;
        return { onlyVoice, onlyVoice, 0.0f, 0, 1 };
    }

    const float span = static_cast<float>(totalVoices - 1);
    const float pos = std::clamp(morph01 * span, 0.0f, span);
    const int idxA = static_cast<int>(pos);
    const int idxB = std::min(idxA + 1, totalVoices - 1);
    float frac = pos - static_cast<float>(idxA);
    if (frac < 1e-4f) frac = 0.0f;
    else if (frac > (1.0f - 1e-4f)) frac = 0.0f;

    auto mapIndex = [&](int gIdx) -> uint32_t {
        int accum = 0;
        for (int p = 0; p < numActivePools; ++p) {
            const auto& pr = activePools[static_cast<size_t>(p)];
            const int c = static_cast<int>(pr.voiceCount);
            if (gIdx < accum + c) {
                return pr.firstVoiceIndex + static_cast<uint32_t>(gIdx - accum);
            }
            accum += c;
        }
        return activePools[static_cast<size_t>(numActivePools - 1)].firstVoiceIndex;
    };

    return { mapIndex(idxA), mapIndex(idxB), frac, idxA, totalVoices };
}

inline float morphForIndex(int g, int n) noexcept {
    return (n > 1) ? (static_cast<float>(g) / static_cast<float>(n - 1)) : 0.0f;
}

inline int indexForMorph(float morph01, int n) noexcept {
    return (n > 1) ? std::clamp(static_cast<int>(std::round(std::clamp(morph01, 0.0f, 1.0f) * static_cast<float>(n - 1))), 0, n - 1) : 0;
}

} // namespace DigiStrikeFS2Fleet
