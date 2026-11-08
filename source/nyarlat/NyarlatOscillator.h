#pragma once

#include <cmath>
#include <array>
#include <vector>
#include <cstring>
#include <cstdint>
#include <algorithm>
#include <juce_core/juce_core.h>
#include "NyarlatRom.h"
#include "NyarlatAdditiveOscillator.h"

namespace Nyarlat {

class BitmaskOscillator {
public:
    static constexpr size_t kTableSize = 512;
    static constexpr size_t kZeroRunway = 8192; // 8KB buffer (512 wave + 7680 zeroes)
    static constexpr int kOversample = 4;
    static constexpr size_t kNumStatorSlices = 64;

    BitmaskOscillator() {
        buffer8kL_.assign(kZeroRunway, 0.0f);
        buffer8kR_.assign(kZeroRunway, 0.0f);
        sliceKeys_.fill(0xFFFFFFFFu);
        for (auto& sl : statorCacheL_) sl.fill(0.0f);
        for (auto& sl : statorCacheR_) sl.fill(0.0f);
        activeCycleL_ = statorCacheL_[0].data();
        activeCycleR_ = statorCacheR_[0].data();
    }

    void prepare(double sampleRate, NyarlatAdditiveEngine* additiveEngine = nullptr) noexcept {
        additiveEngine_ = additiveEngine;
        sampleRate_ = (sampleRate > 0.0) ? sampleRate : 44100.0;
        incScale_ = 16777216.0 / (sampleRate_ * 4.0);
        statorScale_ = 4294967296.0 / sampleRate_;
        dcPole_ = static_cast<float>(std::exp(-6.28318530717958647692 * 10.0 / sampleRate_));
        phase_ = 0;
        statorQ_ = 0;
        lastStatorOffset_ = 0;
        latchedRatio_ = 1.0f;
        tableVersion_ = 0;
        sliceKeys_.fill(0xFFFFFFFFu);
        for (auto& sl : statorCacheL_) sl.fill(0.0f);
        for (auto& sl : statorCacheR_) sl.fill(0.0f);
        activeCycleL_ = statorCacheL_[0].data();
        activeCycleR_ = statorCacheR_[0].data();
        dcStateL_ = 0.0f;
        dcStateR_ = 0.0f;
        prevInputL_ = 0.0f;
        prevInputR_ = 0.0f;
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
        lastStatorOffset_ = 0;
        latchedRatio_ = 1.0f;
        tableVersion_ = 0;
        sliceKeys_.fill(0xFFFFFFFFu);
        for (auto& sl : statorCacheL_) sl.fill(0.0f);
        for (auto& sl : statorCacheR_) sl.fill(0.0f);
        activeCycleL_ = statorCacheL_[0].data();
        activeCycleR_ = statorCacheR_[0].data();
        dcStateL_ = 0.0f;
        dcStateR_ = 0.0f;
        prevInputL_ = 0.0f;
        prevInputR_ = 0.0f;
        syncFade_ = 0.0f;
        tableDirty_ = false;
        lastBank_ = 0xFFFFFFFFu;
        lastMorph_ = -1.0f;
        lastOddEven_ = -999.0f;
        lastTilt_ = -999.0f;
        lastFormant_ = -999.0f;
        lastLifeMode_ = -1;
        lastWidth_ = -999.0f;
        lastSpread_ = -999.0f;
    }

    // Updates the synthesized additive cycle in the first 512 samples of the 8KB buffers (Stereo)
    void updateTableStereo(uint32_t bankIdx, float morph,
                           float oddEven = 0.0f, float tilt = 0.0f, float formant = 0.5f,
                           int lifeMode = 0, float width = 0.5f, float spread = 0.0f) noexcept {
        if (bankIdx == lastBank_ &&
            std::fabs(morph - lastMorph_) < (1.0f / 256.0f) &&
            std::fabs(oddEven - lastOddEven_) < (1.0f / 256.0f) &&
            std::fabs(tilt - lastTilt_) < (1.0f / 64.0f) &&
            std::fabs(formant - lastFormant_) < (1.0f / 256.0f) &&
            lifeMode == lastLifeMode_ &&
            std::fabs(width - lastWidth_) < (1.0f / 128.0f) &&
            std::fabs(spread - lastSpread_) < (1.0f / 128.0f)) return;

        lastBank_ = bankIdx;
        lastMorph_ = morph;
        lastOddEven_ = oddEven;
        lastTilt_ = tilt;
        lastFormant_ = formant;
        lastLifeMode_ = lifeMode;
        lastWidth_ = width;
        lastSpread_ = spread;

        if (additiveEngine_ != nullptr) {
            additiveEngine_->synthesizeCycleStereo(
                buffer8kL_.data(), buffer8kR_.data(),
                bankIdx, morph, oddEven, tilt, formant, lifeMode, width, spread);
        } else {
            NyarlatAdditiveEngine::getInstance().synthesizeCycleStereo(
                buffer8kL_.data(), buffer8kR_.data(),
                bankIdx, morph, oddEven, tilt, formant, lifeMode, width, spread);
        }

        buffer8kL_[512] = buffer8kL_[0]; buffer8kL_[513] = buffer8kL_[1]; buffer8kL_[514] = buffer8kL_[2]; buffer8kL_[515] = buffer8kL_[3];
        buffer8kR_[512] = buffer8kR_[0]; buffer8kR_[513] = buffer8kR_[1]; buffer8kR_[514] = buffer8kR_[2]; buffer8kR_[515] = buffer8kR_[3];

        tableVersion_++;
        tableDirty_ = true; // Invalidate stator cache: re-latch on next seam wrap
    }

    void updateTable(uint32_t bankIdx, float morph,
                     float oddEven = 0.0f, float tilt = 0.0f, float formant = 0.5f,
                     int lifeMode = 0) noexcept {
        updateTableStereo(bankIdx, morph, oddEven, tilt, formant, lifeMode, 0.5f, 0.0f);
    }

    // Fast copy from processor-level shared wavetable cache (bypasses redundant interpolation)
    void copyTableStereo(const float* sourceL512, const float* sourceR512, uint32_t bankIdx, float morph) noexcept {
        if (bankIdx == lastBank_ && std::fabs(morph - lastMorph_) < (1.0f / 256.0f)) return;

        lastBank_ = bankIdx;
        lastMorph_ = morph;

        std::memcpy(buffer8kL_.data(), sourceL512, kTableSize * sizeof(float));
        std::memcpy(buffer8kR_.data(), sourceR512, kTableSize * sizeof(float));
        buffer8kL_[512] = buffer8kL_[0]; buffer8kL_[513] = buffer8kL_[1]; buffer8kL_[514] = buffer8kL_[2]; buffer8kL_[515] = buffer8kL_[3];
        buffer8kR_[512] = buffer8kR_[0]; buffer8kR_[513] = buffer8kR_[1]; buffer8kR_[514] = buffer8kR_[2]; buffer8kR_[515] = buffer8kR_[3];

        tableVersion_++;
        tableDirty_ = true;
    }

    void copyTable(const float* source512, uint32_t bankIdx, float morph) noexcept {
        copyTableStereo(source512, source512, bankIdx, morph);
    }

    // Process a single stereo audio sample (evaluates 4 sub-samples with 2-point linear interpolation & Q32 Stator Sweep)
    void processSampleStereo(float freqHz, int rawMask, float maskSpeed,
                             int loopMode, bool octLock,
                             float& outL, float& outR,
                             bool hardSyncTrigger = false) noexcept {
        const uint32_t mask9 = static_cast<uint32_t>(juce::jlimit(0, 511, rawMask) * 16 + 15) & 0x1FFu;
        const float sp = (maskSpeed > 0.0f && std::isfinite(maskSpeed)) ? std::fmin(maskSpeed, 10.0f) : 0.0f;

        // Advance stator accumulator once per output sample (lightweight, zero inner-loop cost)
        statorQ_ += static_cast<uint32_t>(sp * statorScale_);

        // Handle hard sync reset with 2-subsample anti-glitch
        if (hardSyncTrigger) {
            phase_ = 0;
            statorQ_ = 0;
            syncFade_ = 1.0f;
        }

        if (hardSyncTrigger || tableDirty_ || activeCycleL_ == nullptr) {
            latchSeam(mask9, rawMask, loopMode, sp, octLock);
            tableDirty_ = false;
        }

        // Phase increment for 4x oversampling using seam-latched octave lock ratio
        const double incPerSubstep = static_cast<double>(freqHz * latchedRatio_) * incScale_;
        const uint32_t inc = std::max<uint32_t>(1u, static_cast<uint32_t>(std::min(incPerSubstep, 16777215.0)));

        // Ultra-lean integer-only 4x oversampling loop with 2-point linear interpolation
        const float* cL = activeCycleL_;
        const float* cR = activeCycleR_;
        uint32_t ph = phase_ & 0x00FFFFFFu;
        int nPre = 4;
        if (static_cast<uint64_t>(ph) + 4ull * inc >= 0x01000000ull) {
            nPre = static_cast<int>(std::min<uint32_t>(4u, (0x01000000u - ph + inc - 1u) / inc));
        }

        float subsL[4], subsR[4];
        for (int k = 0; k < nPre; ++k) {
            const uint32_t idx0 = ph >> 15;
            const uint32_t idx1 = (idx0 + 1) & 0x1FFu;
            const float frac = static_cast<float>((ph >> 6) & 0x1FFu) * (1.0f / 512.0f);
            subsL[k] = cL[idx0] + frac * (cL[idx1] - cL[idx0]);
            subsR[k] = cR[idx0] + frac * (cR[idx1] - cR[idx0]);
            ph += inc;
        }
        if (nPre < 4) {
            // Seam wrap landed inside this audio sample: re-latch parameters exactly at the boundary!
            ph &= 0x00FFFFFFu;
            latchSeam(mask9, rawMask, loopMode, sp, octLock);
            cL = activeCycleL_;
            cR = activeCycleR_;
            for (int k = nPre; k < 4; ++k) {
                const uint32_t idx0 = (ph >> 15) & 0x1FFu;
                const uint32_t idx1 = (idx0 + 1) & 0x1FFu;
                const float frac = static_cast<float>((ph >> 6) & 0x1FFu) * (1.0f / 512.0f);
                subsL[k] = cL[idx0] + frac * (cL[idx1] - cL[idx0]);
                subsR[k] = cR[idx0] + frac * (cR[idx1] - cR[idx0]);
                ph += inc;
            }
        }
        phase_ = ph & 0x00FFFFFFu;

        // Pillar 1: Binomial [1, 3, 3, 1] / 8 decimation kernel (-33.5 dB stopband rejection)
        float decimatedL = 0.125f * (subsL[0] + subsL[3]) + 0.375f * (subsL[1] + subsL[2]);
        float decimatedR = 0.125f * (subsR[0] + subsR[3]) + 0.375f * (subsR[1] + subsR[2]);

        // Smooth hard-sync transition if active
        if (syncFade_ > 0.0f) {
            const float fade = (1.0f - syncFade_ * 0.5f);
            decimatedL *= fade;
            decimatedR *= fade;
            syncFade_ -= 0.1f;
            if (syncFade_ < 0.0f) syncFade_ = 0.0f;
        }

        // Pillar 4: High-Pass Sample-Rate Invariant Leaky DC Blocker (fc = 10.0 Hz)
        // y[n] = x[n] - x[n-1] + R * y[n-1]
        float dcOutL = decimatedL - prevInputL_ + dcPole_ * dcStateL_;
        prevInputL_ = decimatedL;
        dcStateL_ = dcOutL;

        float dcOutR = decimatedR - prevInputR_ + dcPole_ * dcStateR_;
        prevInputR_ = decimatedR;
        dcStateR_ = dcOutR;

        outL = dcOutL;
        outR = dcOutR;
    }

    float processSample(float freqHz, int rawMask, float maskSpeed,
                        int loopMode, bool octLock, bool hardSyncTrigger = false) noexcept {
        float outL = 0.0f, outR = 0.0f;
        processSampleStereo(freqHz, rawMask, maskSpeed, loopMode, octLock, outL, outR, hardSyncTrigger);
        return outL;
    }

    uint32_t getPhase() const noexcept { return phase_; }
    uint32_t getStatorOffset() const noexcept { return lastStatorOffset_; }

private:
    // Bakes one 512-sample stereo stator slice into statorCacheL/R_[slice] with zero DC and Hermite envelope.
    void bakeSlice(uint32_t slice, uint32_t mask9, int loop) noexcept {
        const uint32_t off = (slice << 3) + 4u;
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

        const float* tblL = buffer8kL_.data();
        const float* tblR = buffer8kR_.data();
        const float* win = getWindowTable().data();
        float* dstL = statorCacheL_[slice].data();
        float* dstR = statorCacheR_[slice].data();

        // Pass 1: Raw cycle synthesis and mean energy evaluation
        float sumValL = 0.0f, sumValR = 0.0f;
        for (uint32_t n = 0; n < kTableSize; ++n) {
            const float pos = static_cast<float>(n) * r;
            const uint32_t k = static_cast<uint32_t>(pos);
            const uint32_t i = k & 0x1FFu;
            const uint32_t im = i & mask9;
            float sL = tblL[im];
            float sR = tblR[im];
            if (identity) {
                const float frac = (pos - static_cast<float>(k));
                sL += frac * (tblL[(im + 1u) & 0x1FFu] - sL);
                sR += frac * (tblR[(im + 1u) & 0x1FFu] - sR);
            }
            const float winFactor = (1.0f - depth * win[n]);
            float valL = sL * winFactor;
            float valR = sR * winFactor;
            dstL[n] = valL;
            dstR[n] = valR;
            sumValL += valL;
            sumValR += valR;
        }

        // Pass 2: AC-centered energy taper (Stage 1 Dynamics) & pre-subtract mean (zero DC)
        const float meanL = sumValL * (1.0f / 512.0f);
        const float meanR = sumValR * (1.0f / 512.0f);
        float acSum = 0.0f;
        for (uint32_t n = 0; n < kTableSize; ++n) {
            const float diffL = dstL[n] - meanL;
            const float diffR = dstR[n] - meanR;
            acSum += diffL * diffL + diffR * diffR;
        }
        const float meanAcEnergy = 0.5f * acSum * (1.0f / 512.0f);
        constexpr float kTargetNominalEnergy = 0.0625f; // -12 dBFS RMS nominal reference

        float taperGain = 1.0f;
        if (meanAcEnergy > kTargetNominalEnergy) {
            taperGain = 1.0f / std::sqrt(1.0f + 8.0f * (meanAcEnergy - kTargetNominalEnergy));
        }

        // Pass 3: Apply Hermite boundary envelope (Pillar 2) & pre-subtract mean (zero DC)
        for (uint32_t n = 0; n < kTableSize; ++n) {
            float edge = 1.0f;
            if (n < 4) {
                float u = static_cast<float>(n) * 0.25f;
                edge = u * u * (3.0f - 2.0f * u);
            } else if (n > 507) {
                float u = static_cast<float>(511 - n) * 0.25f;
                edge = u * u * (3.0f - 2.0f * u);
            }
            const float scale = taperGain * edge;
            dstL[n] = (dstL[n] - meanL) * scale;
            dstR[n] = (dstR[n] - meanR) * scale;
        }
    }

    void latchSeam(uint32_t mask9, int rawMask, int loop, float sp, bool octLock) noexcept {
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
        juce::ignoreUnused(octLock, rawMask);
        latchedRatio_ = 1.0f;

        const uint32_t slice = ((off + 4u) >> 3) & 63u;
        const uint32_t key = (mask9 << 9) | (static_cast<uint32_t>(loop) << 18) | (tableVersion_ & 0x1FFu);

        if (sliceKeys_[slice] != key) {
            bakeSlice(slice, mask9, loop);
            sliceKeys_[slice] = key;
        }
        activeCycleL_ = statorCacheL_[slice].data();
        activeCycleR_ = statorCacheR_[slice].data();
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

    NyarlatAdditiveEngine* additiveEngine_{nullptr};
    double sampleRate_ = 44100.0;
    double incScale_ = 16777216.0 / (44100.0 * 4.0);
    double statorScale_ = 4294967296.0 / 44100.0;
    uint32_t phase_ = 0;
    uint32_t statorQ_ = 0;
    uint32_t lastStatorOffset_ = 0;
    float dcPole_ = 0.998692f;
    float syncFade_ = 0.0f;

    alignas(64) std::array<std::array<float, kTableSize>, kNumStatorSlices> statorCacheL_{};
    alignas(64) std::array<std::array<float, kTableSize>, kNumStatorSlices> statorCacheR_{};
    std::array<uint32_t, kNumStatorSlices> sliceKeys_{};
    const float* activeCycleL_ = nullptr;
    const float* activeCycleR_ = nullptr;
    uint32_t tableVersion_ = 0;
    float latchedRatio_ = 1.0f;
    uint32_t lastBank_ = 0xFFFFFFFFu;
    float lastMorph_ = -1.0f;
    float lastOddEven_ = -999.0f;
    float lastTilt_ = -999.0f;
    float lastFormant_ = -999.0f;
    int lastLifeMode_ = -1;
    float lastWidth_ = -999.0f;
    float lastSpread_ = -999.0f;
    bool tableDirty_ = false;

    float dcStateL_ = 0.0f;
    float dcStateR_ = 0.0f;
    float prevInputL_ = 0.0f;
    float prevInputR_ = 0.0f;

    std::vector<float> buffer8kL_;
    std::vector<float> buffer8kR_;
};

} // namespace Nyarlat
