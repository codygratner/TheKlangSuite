#pragma once

#include <cmath>
#include <array>
#include <vector>
#include <cstring>
#include <cstdint>
#include <algorithm>
#include <juce_core/juce_core.h>
#include "NyarlatRom.h"

namespace Nyarlat {

class BitmaskOscillator {
public:
    static constexpr size_t kTableSize = 512;
    static constexpr size_t kZeroRunway = 8192; // 8KB buffer (512 wave + 7680 zeroes)
    static constexpr int kOversample = 4;

    BitmaskOscillator() {
        buffer8k_.assign(kZeroRunway, 0.0f);
    }

    void prepare(double sampleRate) noexcept {
        sampleRate_ = (sampleRate > 0.0) ? sampleRate : 44100.0;
        incScale_ = 16777216.0 / (sampleRate_ * 4.0);
        statorScale_ = 4294967296.0 / sampleRate_;
        dcPole_ = static_cast<float>(std::exp(-6.28318530717958647692 * 10.0 / sampleRate_));
        phase_ = 0;
        statorQ_ = 0;
        bakedKey_ = 0xFFFFFFFFu;
        latchedRatio_ = 1.0f;
        cycle_.fill(0.0f);
        dcState_ = 0.0f;
        prevInput_ = 0.0f;
        syncFade_ = 0.0f;
        tableDirty_ = false;
        lastBank_ = 0xFFFFFFFFu;
        lastMorph_ = -1.0f;
        // Pre-warm static pitch compensation table, seam window & AKWF ROM outside real-time audio thread
        juce::ignoreUnused(getSharedPitchTable()[0]);
        juce::ignoreUnused(getWindowTable()[0]);
        juce::ignoreUnused(AkwfRom::getInstance());
    }

    void reset() noexcept {
        phase_ = 0;
        statorQ_ = 0;
        bakedKey_ = 0xFFFFFFFFu;
        latchedRatio_ = 1.0f;
        cycle_.fill(0.0f);
        dcState_ = 0.0f;
        prevInput_ = 0.0f;
        syncFade_ = 0.0f;
        tableDirty_ = false;
        lastBank_ = 0xFFFFFFFFu;
        lastMorph_ = -1.0f;
    }

    // Updates the morphed AKWF cycle in the first 512 samples of the 8KB buffer
    void updateTable(uint32_t bankIdx, float waveMorph) noexcept {
        if (bankIdx == lastBank_ && std::fabs(waveMorph - lastMorph_) < (1.0f / 4096.0f)) return;
        lastBank_ = bankIdx;
        lastMorph_ = waveMorph;
        AkwfRom::getInstance().interpolateWaveform(bankIdx, waveMorph, buffer8k_.data());
        tableDirty_ = true; // Click-free: re-bake deferred to next phase seam wrap
    }

    // Fast copy from processor-level shared wavetable cache (bypasses redundant interpolation)
    void copyTable(const float* source512, uint32_t bankIdx, float waveMorph) noexcept {
        if (bankIdx == lastBank_ && std::fabs(waveMorph - lastMorph_) < (1.0f / 4096.0f)) return;
        lastBank_ = bankIdx;
        lastMorph_ = waveMorph;
        std::memcpy(buffer8k_.data(), source512, kTableSize * sizeof(float));
        tableDirty_ = true;
    }

    // Process a single audio sample (evaluates 4 sub-samples with 2-point linear interpolation & Q32 Stator Sweep)
    float processSample(float freqHz, int rawMask, float maskSpeed,
                        int loopMode, bool octLock, bool hardSyncTrigger = false) noexcept {
        const uint32_t mask9 = static_cast<uint32_t>(juce::jlimit(0, 511, rawMask) * 16 + 15) & 0x1FFu;
        const float sp = (maskSpeed > 0.0f && std::isfinite(maskSpeed)) ? std::fmin(maskSpeed, 10.0f) : 0.0f;

        // Advance stator accumulator once per output sample (lightweight, zero inner-loop cost)
        statorQ_ += static_cast<uint32_t>(sp * statorScale_);

        // Handle hard sync reset with 2-subsample anti-glitch
        if (hardSyncTrigger) {
            phase_ = 0;
            statorQ_ = 0;
            syncFade_ = 1.0f;
            bakedKey_ = 0xFFFFFFFFu;
        }

        if (hardSyncTrigger || bakedKey_ == 0xFFFFFFFFu || tableDirty_) {
            latchSeam(mask9, rawMask, loopMode, sp, octLock);
        }

        // Phase increment for 4x oversampling using seam-latched octave lock ratio
        const double incPerSubstep = static_cast<double>(freqHz * latchedRatio_) * incScale_;
        const uint32_t inc = std::max<uint32_t>(1u, static_cast<uint32_t>(std::min(incPerSubstep, 16777215.0)));

        // Ultra-lean integer-only 4x oversampling loop with 2-point linear interpolation
        const float* c = cycle_.data();
        uint32_t ph = phase_ & 0x00FFFFFFu;
        int nPre = 4;
        if (static_cast<uint64_t>(ph) + 4ull * inc >= 0x01000000ull) {
            nPre = static_cast<int>(std::min<uint32_t>(4u, (0x01000000u - ph + inc - 1u) / inc));
        }

        float subs[4];
        for (int k = 0; k < nPre; ++k) {
            const uint32_t idx0 = ph >> 15;
            const uint32_t idx1 = (idx0 + 1) & 0x1FFu;
            const float frac = static_cast<float>((ph >> 6) & 0x1FFu) * (1.0f / 512.0f);
            subs[k] = c[idx0] + frac * (c[idx1] - c[idx0]);
            ph += inc;
        }
        if (nPre < 4) {
            // Seam wrap landed inside this audio sample: re-latch parameters exactly at the boundary!
            ph &= 0x00FFFFFFu;
            latchSeam(mask9, rawMask, loopMode, sp, octLock);
            for (int k = nPre; k < 4; ++k) {
                const uint32_t idx0 = (ph >> 15) & 0x1FFu;
                const uint32_t idx1 = (idx0 + 1) & 0x1FFu;
                const float frac = static_cast<float>((ph >> 6) & 0x1FFu) * (1.0f / 512.0f);
                subs[k] = c[idx0] + frac * (c[idx1] - c[idx0]);
                ph += inc;
            }
        }
        phase_ = ph & 0x00FFFFFFu;

        // Pillar 1: Binomial [1, 3, 3, 1] / 8 decimation kernel (-33.5 dB stopband rejection)
        float decimated = 0.125f * (subs[0] + subs[3]) + 0.375f * (subs[1] + subs[2]);

        // Smooth hard-sync transition if active
        if (syncFade_ > 0.0f) {
            decimated *= (1.0f - syncFade_ * 0.5f);
            syncFade_ -= 0.1f;
            if (syncFade_ < 0.0f) syncFade_ = 0.0f;
        }

        // Pillar 4: High-Pass Sample-Rate Invariant Leaky DC Blocker (fc = 10.0 Hz)
        // y[n] = x[n] - x[n-1] + R * y[n-1]
        float dcOut = decimated - prevInput_ + dcPole_ * dcState_;
        prevInput_ = decimated;
        dcState_ = dcOut;

        return dcOut;
    }

    uint32_t getPhase() const noexcept { return phase_; }
    uint32_t getStatorOffset() const noexcept { return lastStatorOffset_; }

private:
    // Bakes one full 512-sample cycle (mask, resonance depth, seam window, gain).
    // Runs strictly at phase seam wrap or key invalidation (0% CPU during static holding).
    void bakeCycle(uint32_t mask9, uint32_t off, int loop) noexcept {
        const uint32_t key = (mask9 << 9) | off | (static_cast<uint32_t>(loop) << 18);
        if (key == bakedKey_) return;
        bakedKey_ = key;

        auto S = [](float x) noexcept {
            x = juce::jlimit(0.0f, 1.0f, x);
            return x * x * (3.0f - 2.0f * x);
        };
        const float t = static_cast<float>(off) * (1.0f / 512.0f);
        // loop: 0 = Saw Up, 1 = Triangle (Ping-Pong), 2 = Saw Down
        const float depth = (loop == 1)
            ? S(1.0f - std::fabs(2.0f * t - 1.0f))
            : (loop == 0
                ? (t <= 0.9f ? t : t * (1.0f - S((t - 0.9f) * 10.0f)))
                : (t >= 0.1f ? (1.0f - t) : (1.0f - t) * (1.0f - S((0.1f - t) * 10.0f))));
        const float r = 1.0f + 5.0f * depth;
        const bool identity = (mask9 == 0x1FFu);
        const float* tbl = buffer8k_.data();
        const float* win = getWindowTable().data();

        // Pass 1: Raw cycle synthesis and mean energy evaluation
        float sumVal = 0.0f;
        for (uint32_t n = 0; n < kTableSize; ++n) {
            const float pos = static_cast<float>(n) * r;
            const uint32_t k = static_cast<uint32_t>(pos);
            const uint32_t i = k & 0x1FFu;
            const uint32_t im = i & mask9;
            float s = tbl[im];
            if (identity) s += (pos - static_cast<float>(k)) * (tbl[(im + 1u) & 0x1FFu] - s);
            float val = s * (1.0f - depth * win[n]);
            cycle_[n] = val;
            sumVal += val;
        }

        // Pass 2: AC-centered energy taper (Stage 1 Dynamics)
        const float mean = sumVal * (1.0f / 512.0f);
        float acEnergySum = 0.0f;
        for (uint32_t n = 0; n < kTableSize; ++n) {
            const float diff = cycle_[n] - mean;
            acEnergySum += diff * diff;
        }
        const float meanAcEnergy = acEnergySum * (1.0f / 512.0f);
        constexpr float kTargetNominalEnergy = 0.0625f; // -12 dBFS RMS nominal reference

        float taperGain = 1.0f;
        if (meanAcEnergy > kTargetNominalEnergy) {
            taperGain = 1.0f / std::sqrt(1.0f + 8.0f * (meanAcEnergy - kTargetNominalEnergy));
        }

        // Pass 3: Apply Hermite boundary envelope (Pillar 2) & AC taper
        for (uint32_t n = 0; n < kTableSize; ++n) {
            float edge = 1.0f;
            if (n < 4) {
                float u = static_cast<float>(n) * 0.25f;
                edge = u * u * (3.0f - 2.0f * u);
            } else if (n > 507) {
                float u = static_cast<float>(511 - n) * 0.25f;
                edge = u * u * (3.0f - 2.0f * u);
            }
            cycle_[n] *= taperGain * edge;
        }
    }

    void latchSeam(uint32_t mask9, int rawMask, int loop, float sp, bool octLock) noexcept {
        if (tableDirty_) {
            bakedKey_ = 0xFFFFFFFFu;
            tableDirty_ = false;
        }
        uint32_t sweep = 0;
        if (sp > 0.0f) {
            const uint32_t p = statorQ_ >> 23;
            if (loop == 0) {
                sweep = p;
            } else if (loop == 1) {
                sweep = (p < 256u ? p * 2u : (511u - p) * 2u);
            } else {
                sweep = 511u - p;
            }
        }
        const uint32_t off = sweep & 0x1FFu;
        lastStatorOffset_ = off;
        latchedRatio_ = (octLock && rawMask >= 0 && rawMask < 512) ? getSharedPitchTable()[rawMask] : 1.0f;
        bakeCycle(mask9, off, loop);
    }

private:
    static const std::array<float, 512>& getSharedPitchTable() {
        static const auto table = []() {
            std::array<float, 512> t;
            for (int m = 0; m < 512; ++m) {
                int mask = m * 16 + 15;
                float ratio = 1.0f;
                if ((mask & 256) == 0 && (mask & 128) != 0) {
                    ratio = 0.5f;
                } else if ((mask & 128) == 0 && (mask & 64) != 0) {
                    ratio = 0.25f;
                }
                t[m] = ratio;
            }
            return t;
        }();
        return table;
    }

    static const std::array<float, 512>& getWindowTable() {
        static const auto table = []() {
            std::array<float, 512> t;
            for (int n = 0; n < 512; ++n) {
                const float x = static_cast<float>(n) * (1.0f / 512.0f);
                t[n] = x * x * (3.0f - 2.0f * x); // S(phi) smoothstep seam window
            }
            return t;
        }();
        return table;
    }

    double sampleRate_ = 44100.0;
    double incScale_ = 16777216.0 / (44100.0 * 4.0);
    double statorScale_ = 4294967296.0 / 44100.0;
    uint32_t phase_ = 0;
    uint32_t statorQ_ = 0;
    uint32_t lastStatorOffset_ = 0;
    float dcState_ = 0.0f;
    float prevInput_ = 0.0f;
    float dcPole_ = 0.998692f;
    float syncFade_ = 0.0f;

    alignas(32) std::array<float, kTableSize> cycle_{};
    uint32_t bakedKey_ = 0xFFFFFFFFu;
    float latchedRatio_ = 1.0f;
    uint32_t lastBank_ = 0xFFFFFFFFu;
    float lastMorph_ = -1.0f;
    bool tableDirty_ = false;

    std::vector<float> buffer8k_;
};

} // namespace Nyarlat
