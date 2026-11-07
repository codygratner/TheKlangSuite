#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <array>
#include <cstring>
#include "shoggoth_layout.h"

namespace Shoggoth {

struct DecodedSinglePatch {
    std::string name{"INIT PATCH"};
    float s1Harmonics[63]{};
    float s2Harmonics[63]{};
    uint8_t s1EnvAssign[63]{};
    uint8_t s2EnvAssign[63]{};
    DHGEnvelopeData dhgEnvs[kNumDHGEnv];
    DHGEnvelopeData ddaEnv;
    DHGEnvelopeData ddfEnv;
    float ddfCutoff{99.0f};
    float ddfReso{0.0f};
    bool valid{false};
};

class SysExParser {
public:
    // ==============================================================================
    // 1. Unpack 492-byte Single Payload
    // ==============================================================================
    static bool parseSinglePayload(const uint8_t* payload, size_t size, DecodedSinglePatch& out) noexcept {
        if (!payload || size < 492) return false;

        // Name at bytes 0..9 (10 ASCII chars)
        char nameBuf[11]{};
        for (int i = 0; i < 10; ++i) {
            char c = static_cast<char>(payload[i]);
            nameBuf[i] = (c >= 32 && c <= 126) ? c : ' ';
        }
        nameBuf[10] = '\0';
        out.name = std::string(nameBuf);

        // Source 1 Harmonics (63 bytes) starting at offset 10
        for (int k = 0; k < 63; ++k) {
            out.s1Harmonics[k] = static_cast<float>(payload[10 + k] & 0x7F);
        }

        // Source 2 Harmonics (63 bytes) starting at offset 73
        for (int k = 0; k < 63; ++k) {
            out.s2Harmonics[k] = static_cast<float>(payload[73 + k] & 0x7F);
        }

        // DHG Envelope assignments (packed 2 bits per harmonic) at offset 136..167
        for (int k = 0; k < 63; ++k) {
            const int byteIdx = 136 + (k / 4);
            const int shift = (k % 4) * 2;
            out.s1EnvAssign[k] = static_cast<uint8_t>((payload[byteIdx] >> shift) & 0x03);
            const int byteIdx2 = 152 + (k / 4);
            out.s2EnvAssign[k] = static_cast<uint8_t>((payload[byteIdx2] >> shift) & 0x03);
        }

        // DHG Envelopes 1..4 (each has 6 rates and 6 levels = 12 bytes each) starting at offset 168
        for (int e = 0; e < 4; ++e) {
            const int base = 168 + e * 12;
            for (int s = 0; s < 6; ++s) {
                out.dhgEnvs[e].stages[s].rate = static_cast<float>(payload[base + s] & 0x7F);
                out.dhgEnvs[e].stages[s].level = static_cast<float>(payload[base + 6 + s] & 0x7F);
            }
            out.dhgEnvs[e].loop34 = (payload[216 + e] != 0);
        }

        // DDA Envelope (12 bytes at 220)
        for (int s = 0; s < 6; ++s) {
            out.ddaEnv.stages[s].rate = static_cast<float>(payload[220 + s] & 0x7F);
            out.ddaEnv.stages[s].level = static_cast<float>(payload[226 + s] & 0x7F);
        }

        // DDF Envelope (12 bytes at 232)
        for (int s = 0; s < 6; ++s) {
            out.ddfEnv.stages[s].rate = static_cast<float>(payload[232 + s] & 0x7F);
            out.ddfEnv.stages[s].level = static_cast<float>(payload[238 + s] & 0x7F);
        }

        out.ddfCutoff = static_cast<float>(payload[244] & 0x7F);
        out.ddfReso   = static_cast<float>(payload[245] & 0x7F);

        out.valid = true;
        return true;
    }

    // ==============================================================================
    // 2. Unpack Raw MIDI SysEx File (993-byte message -> 492-byte payload)
    // ==============================================================================
    static bool parseSysExFile(const uint8_t* data, size_t size, DecodedSinglePatch& out) noexcept {
        if (!data || size < 10) return false;

        // If file is already a raw 492-byte dumped payload
        if (size == 492) {
            return parseSinglePayload(data, size, out);
        }

        // Check SysEx header: F0 40 [channel] 20 00 [subId] ... F7
        if (data[0] == 0xF0 && data[1] == 0x40) {
            // Find payload start (typically at offset 8 or 9)
            // Unpack 2 nibbles per payload byte
            std::vector<uint8_t> payload;
            payload.reserve(512);

            size_t idx = 8;
            while (idx + 1 < size && data[idx] != 0xF7 && payload.size() < 492) {
                uint8_t high = data[idx] & 0x0F;
                uint8_t low  = data[idx + 1] & 0x0F;
                payload.push_back(static_cast<uint8_t>((high << 4) | low));
                idx += 2;
            }

            if (payload.size() >= 492) {
                return parseSinglePayload(payload.data(), payload.size(), out);
            }
        }

        return false;
    }

    // ==============================================================================
    // 3. Compute 16-bit 1987 Checksum
    // ==============================================================================
    static uint16_t computeChecksum(const uint8_t* payload, size_t size) noexcept {
        uint16_t sum = 0;
        for (size_t i = 0; i < size; ++i) {
            sum += payload[i];
        }
        return sum;
    }
};

} // namespace Shoggoth
