#pragma once

#include "dsp/oscillators/PolyBLEP.h"
#include "AllPassFilter4.h"
#include "SubAPF16.h"
#include "HasturFilterUtils.h"
#include "CyborgBandLimiter.h"
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace CrackedChimes::DSP {

// Utility filter structs now in HasturFilterUtils.h


/**
 * @brief Single Logic DCO 2.0 Core: 12 MHz Master Crystal Clock Timebase,
 * 16-Wave Multi-Octave Hoover Matrix & Unison Saw Swarm Engine ("Bees in a Box"),
 * Pre/Post-Chopper 4-Pole APF Routing, Dedicated -1 Oct Sub & Front-Panel Stem Mixer.
 * 
 * SSoT: Divided Logic DCO (12 MHz Master Crystal Clock),
 * Intel 8253 timer / CD4024 ripple counter R-2R staircase,
 * Noise Engineering Infinifold, and Hard oscillator sync.
 */
class AlphaOscillator {
public:
    static constexpr double kMasterCrystalHz = 12000000.0;
    static constexpr double kCyborgTimerHz   = 1000000.0;

    void prepare(double sampleRate) noexcept {
        sampleRate_ = (sampleRate > 1000.0) ? sampleRate : 44100.0;
        invSr_ = 1.0 / sampleRate_;
        clockTicksPerSample_ = clockHz_ * invSr_;
        apf_.prepare(sampleRate_);
        apfB_.prepare(sampleRate_);
        subApf_.prepare(sampleRate_);
        noiseGen_.reset(0x13579BDFu);
        sideHp_.setCutoff(120.0f, static_cast<float>(sampleRate_));
        biteHfFilter_.setCutoff(14500.0f, static_cast<float>(sampleRate_));
        subDcFilter_.prepare(sampleRate_, 5.0);
        coreDcFilter_.prepare(sampleRate_, 5.0);
        biteDcFilter_.prepare(sampleRate_, 5.0);
        reset();
    }

    void reset() noexcept {
        crystalTicks_ = 0.0;
        slaveTicks_ = 0.0;
        phase_ = 0.0f;
        slavePhase_ = 0.0f;
        prevPhase_ = 0.0f;
        subPhase_ = 0.0f;
        pwmLfoPhase_ = 0.0f;
        swarmLfoPhase_ = 0.0f;
        autopanPhase_ = 0.0f;

        rawOut_ = 0.0f;
        multOut_ = 0.0f;
        processedOut_ = 0.0f;
        prevProcessedOut_ = 0.0f;
        subOut_ = 0.0f;
        superSawOut_ = 0.0f;
        superSawOutL_ = 0.0f;
        superSawOutR_ = 0.0f;
        noiseOut_ = 0.0f;
        compositeOut_ = 0.0f;

        lastFreqHz_ = 130.81f;
        lastDynamicPw_ = 0.5f;
        wrapped_ = false;
        apfTrigger_ = false;
        syncEdgeAccum_ = 0.0;
        activeShapeIdx_ = pendingShapeIdx_;
        apf_.reset();
        apfB_.reset();
        subApf_.reset();
        burbleMod_ = 0.0f;
        subDcFilter_.reset();
        coreDcFilter_.reset();
        sideHp_.reset();
        biteDcFilter_.reset();
        biteHfFilter_.reset();
        prevApf1Core_ = 0.0f;
        prevChopCore_ = 0.0f;
        chopperSpikeState_ = 0.0f;
    }

    // --- Configuration & Parameter Controls ---

    void setWaveShape(int shapeIdx) noexcept {
        pendingShapeIdx_ = std::clamp(shapeIdx, 0, 15);
    }

    void setWaveFamily(int family) noexcept {
        waveFamily_ = std::clamp(family, 0, 3);
        setWaveShape(waveFamily_ * 4 + chopperTopo_);
    }
    [[nodiscard]] int getWaveFamily() const noexcept { return waveFamily_; }

    void setChopperTopology(int topo) noexcept {
        chopperTopo_ = std::clamp(topo, 0, 3);
        setWaveShape(waveFamily_ * 4 + chopperTopo_);
    }
    [[nodiscard]] int getChopperTopology() const noexcept { return chopperTopo_; }

    void setMorph(float m) noexcept { morph_ = std::clamp(m, 0.0f, 1.0f); }
    [[nodiscard]] float getMorph() const noexcept { return morph_; }

    void setSubOctave(int oct) noexcept { subOctave_ = std::clamp(oct, -2, 1); }
    [[nodiscard]] int getSubOctave() const noexcept { return subOctave_; }

    void setSubChewOct(float o) noexcept { subChewOct_ = std::clamp(o, -2.0f, 2.0f); }
    [[nodiscard]] float getSubChewOct() const noexcept { return subChewOct_; }
    void setSubApfDepth(float d) noexcept { setSubChewOct(d); }
    [[nodiscard]] float getSubApfDepth() const noexcept { return subChewOct_; }

    void setSubApfTap(int t) noexcept { subApfTap_ = std::clamp(t, 0, 3); }
    [[nodiscard]] int getSubApfTap() const noexcept { return subApfTap_; }

    void setSubSpit(float spit) noexcept { subApf_.setSpit(spit); }
    [[nodiscard]] float getSubSpit() const noexcept { return subApf_.getSpit(); }

    void setSubDetuneSemitones(float semi) noexcept { subDetuneSemitones_ = std::clamp(semi, -2.0f, 2.0f); }
    [[nodiscard]] float getSubDetuneSemitones() const noexcept { return subDetuneSemitones_; }

    [[nodiscard]] float getBurbleMod() const noexcept { return burbleMod_; }

    void setSubPulseTrain(int train) noexcept { subPulseTrain_ = std::clamp(train, 0, 4); }
    [[nodiscard]] int getSubPulseTrain() const noexcept { return subPulseTrain_; }

    void setBite(float b) noexcept { bite_ = std::clamp(b, -1.0f, 1.0f); }
    [[nodiscard]] float getBite() const noexcept { return bite_; }

    void setMasterSpread(float spread) noexcept { masterSpread_ = std::clamp(spread, 0.0f, 1.5f); }
    [[nodiscard]] float getMasterSpread() const noexcept { return masterSpread_; }

    void setMixerDrive(float drive) noexcept { mixerDrive_ = std::clamp(drive, 0.0f, 1.0f); }
    [[nodiscard]] float getMixerDrive() const noexcept { return mixerDrive_; }

    void resetSubPhase() noexcept { subPhase_ = 0.0f; }

    static float renderDualControlSub(int octave, int train, float phase, float dt) noexcept {
        (void)octave;
        const float p = TbdAudio::Oscillators::PolyBLEP::wrapPhase(phase);
        const float safeDt = std::clamp(dt, 1e-6f, 0.48f);

        switch (train) {
            case 0: // Sub 1 / 5: 50% square (PolyBLEP grounded)
                return TbdAudio::Oscillators::PolyBLEP::pulse(p, safeDt, 0.5f);

            case 1: // Sub 2 / 6: 75% pulse with authentic DC offset compensation
                return TbdAudio::Oscillators::PolyBLEP::pulse(p, safeDt, 0.75f);

            case 2: { // Sub 3: Sub Chopped 2x with multi-point BLEP corrections
                const float p4 = TbdAudio::Oscillators::PolyBLEP::wrapPhase(p * 4.0f);
                const float u = (p < 0.5f) ? 1.0f : ((p4 >= 0.5f) ? 1.0f : 0.0f);
                constexpr float mean_dc = 0.75f;
                constexpr float scale = 1.0f / 0.75f;
                float v = (u - mean_dc) * scale;
                v += TbdAudio::Oscillators::PolyBLEP::blep(p, safeDt);
                v -= TbdAudio::Oscillators::PolyBLEP::blep(TbdAudio::Oscillators::PolyBLEP::wrapPhase(p - 0.5f + 1.0f), safeDt);
                v += TbdAudio::Oscillators::PolyBLEP::blep(p4, std::min(safeDt * 4.0f, 0.48f)) * 0.5f;
                return v;
            }

            case 3: { // Sub 4: Sub Chopped 4x with multi-point BLEP corrections
                const float p8 = TbdAudio::Oscillators::PolyBLEP::wrapPhase(p * 8.0f);
                const float u = (p < 0.5f) ? 1.0f : ((p8 >= 0.5f) ? 1.0f : 0.0f);
                constexpr float mean_dc = 0.75f;
                constexpr float scale = 1.0f / 0.75f;
                float v = (u - mean_dc) * scale;
                v += TbdAudio::Oscillators::PolyBLEP::blep(p, safeDt);
                v -= TbdAudio::Oscillators::PolyBLEP::blep(TbdAudio::Oscillators::PolyBLEP::wrapPhase(p - 0.5f + 1.0f), safeDt);
                v += TbdAudio::Oscillators::PolyBLEP::blep(p8, std::min(safeDt * 8.0f, 0.48f)) * 0.5f;
                return v;
            }

            case 4:
            default: { // Sub Chopped 8x with multi-point BLEP corrections
                const float p16 = TbdAudio::Oscillators::PolyBLEP::wrapPhase(p * 16.0f);
                const float u = (p < 0.5f) ? 1.0f : ((p16 >= 0.5f) ? 1.0f : 0.0f);
                constexpr float mean_dc = 0.75f;
                constexpr float scale = 1.0f / 0.75f;
                float v = (u - mean_dc) * scale;
                v += TbdAudio::Oscillators::PolyBLEP::blep(p, safeDt);
                v -= TbdAudio::Oscillators::PolyBLEP::blep(TbdAudio::Oscillators::PolyBLEP::wrapPhase(p - 0.5f + 1.0f), safeDt);
                v += TbdAudio::Oscillators::PolyBLEP::blep(p16, std::min(safeDt * 16.0f, 0.48f)) * 0.5f;
                return v;
            }
        }
    }

    static float generateFamilyWave(int family, float phase, float dt, float morph, float pwm = 0.0f, int lifeMode = 0) noexcept {
        const float p = TbdAudio::Oscillators::PolyBLEP::wrapPhase(phase);
        const float m = std::clamp(morph, 0.0f, 1.0f);
        switch (family) {
            case 0: { // SAW: Divided 2x Chopper Saw PWM (Dual-Pulse Spike at 2x rate)
                const float rawSaw = (lifeMode == 1)
                    ? CyborgBandLimiter::processSaw(p, dt, 1)
                    : TbdAudio::Oscillators::PolyBLEP::saw(p, dt);
                const float effM = std::clamp(m + pwm * 0.45f, 0.0f, 0.98f);
                if (effM <= 0.001f) return rawSaw;

                // 2x sub-counter PWM pulse width (each pulse spans up to 0.45 of the half-cycle)
                const float w = effM * 0.45f;
                float y = -1.0f;
                if (p >= w && p < 0.5f) {
                    y = 1.0f - 2.0f * p;
                } else if (p >= 0.5f + w) {
                    y = 1.0f - 2.0f * p;
                }

                // PolyBLEP anti-aliasing corrections for the dual pulse edges:
                // 1. Up-step at p = w (height: 2.0f - 2.0f * w)
                const float step1 = 2.0f - 2.0f * w;
                y += CyborgBandLimiter::processChopperEdge(step1, TbdAudio::Oscillators::PolyBLEP::wrapPhase(p - w), dt, lifeMode);

                // 2. Down-step at p = 0.5f (height: -1.0f)
                y -= CyborgBandLimiter::processChopperEdge(1.0f, TbdAudio::Oscillators::PolyBLEP::wrapPhase(p - 0.5f), dt, lifeMode);

                // 3. Up-step at p = 0.5f + w (height: 1.0f - 2.0f * w)
                const float step3 = 1.0f - 2.0f * w;
                y += CyborgBandLimiter::processChopperEdge(step3, TbdAudio::Oscillators::PolyBLEP::wrapPhase(p - (0.5f + w)), dt, lifeMode);

                return y;
            }
            case 1: { // PULSE: Solid 50% hollow square -> Variable Pulse with RMS Normalization
                const float d = std::clamp(0.50f + m * 0.45f + pwm * 0.45f, 0.05f, 0.95f);
                const float rawPulse = TbdAudio::Oscillators::PolyBLEP::pulse(p, dt, d);
                const float minD = std::min(d, 1.0f - d);
                const float gainComp = (minD < 0.499f) ? std::min(2.0f, 0.5f / std::sqrt(minD)) : 1.0f;
                return rawPulse * gainComp;
            }
            case 2: { // TRIANGLE: Pure symmetric triangle -> Skewed Sharktooth
                if (m <= 0.001f) {
                    return TbdAudio::Oscillators::PolyBLEP::triangle(p);
                }
                const float tPeak = std::clamp(0.50f + m * 0.49f, 0.01f, 0.99f);
                float y = (p < tPeak)
                    ? (-1.0f + 2.0f * (p / tPeak))
                    : (1.0f - 2.0f * ((p - tPeak) / (1.0f - tPeak)));

                // C1-continuous parabolic fillet around peak to eliminate sharp slope cusp
                const float delta = std::min(2.0f * dt, 0.05f);
                const float dist = std::abs(p - tPeak);
                if (dist < delta) {
                    const float u = dist / delta;
                    y -= (1.0f - u * u) * (delta / std::max(tPeak, 1.0f - tPeak));
                }
                return y;
            }
            case 3: // EXOTIC: Warm Saturated Sine -> Phase Distortion Resonant Sine
            default: {
                constexpr float kTwoPi = 6.28318530717958647692f;
                const float s = std::sin(p * kTwoPi);
                if (m <= 0.001f) {
                    return s;
                }
                if (m <= 0.5f) {
                    const float drive = 1.0f + m * 8.0f;
                    return std::tanh(s * drive) / std::tanh(drive);
                } else {
                    const float k = 1.0f + (m - 0.5f) * 6.0f;
                    const float window = 0.5f * (1.0f + std::cos(p * kTwoPi));
                    const float czPhase = std::fmod(p * k, 1.0f);
                    float out = std::sin(czPhase * kTwoPi) * window;
                    const float effDtK = std::min(dt * k, 0.48f);
                    if (czPhase < effDtK) {
                        const float u = czPhase / effDtK;
                        out -= (1.0f - u) * (1.0f - u) * 0.25f * window;
                    }
                    return out;
                }
            }
        }
    }

    static float applyChopperLifeform(float sample, float prevSample, int lifeMode, float& spikeState) noexcept {
        const float delta = sample - prevSample;
        if (lifeMode == 0) { // SYNTHETIC
            return sample;
        } else if (lifeMode == 2) { // ORGANIC: Soft cathode-follower buffer
            const float absS = std::abs(sample);
            return sample / (1.0f + 0.12f * absS);
        } else if (lifeMode == 1) { // CYBORG: 20us analog charge-injection transient
            if (std::abs(delta) > 0.35f) {
                spikeState += 0.035f * delta;
            }
            spikeState *= 0.65f;
            return std::clamp(sample + spikeState, -1.8f, 1.8f);
        }
        return sample;
    }

    static float applyChopper(float wave, float p, float dt, int topo, int lifeMode = 0) noexcept {
        int M = 1;
        if (topo == 1) M = 2;
        else if (topo == 2) M = 4;
        else if (topo == 3) M = 8;
        else return wave;

        const float pM = TbdAudio::Oscillators::PolyBLEP::wrapPhase(p * static_cast<float>(M));
        const float effDt = dt * static_cast<float>(M);
        float gate = 0.0f;
        if (lifeMode == 1) {
            // Cyborg linear BLEP pulse gate
            const float rawGate = (pM < 0.5f) ? 1.0f : -1.0f;
            const float downEdge = CyborgBandLimiter::processChopperEdge(-2.0f, TbdAudio::Oscillators::PolyBLEP::wrapPhase(pM - 0.5f), effDt, 1);
            const float upEdge   = CyborgBandLimiter::processChopperEdge(2.0f, pM, effDt, 1);
            gate = 0.5f * (rawGate + downEdge + upEdge + 1.0f);
        } else {
            gate = 0.5f * (TbdAudio::Oscillators::PolyBLEP::pulse(pM, effDt, 0.5f) + 1.0f);
        }
        const float u = wave * gate;
        const float mean = -0.25f / static_cast<float>(M);
        return (u - mean) * 1.35f;
    }

    static float generateDCO2(int family, int topo, float phase, float dt, float morph, float pwm = 0.0f, int lifeMode = 0) noexcept {
        const float famWave = generateFamilyWave(family, phase, dt, morph, pwm, lifeMode);
        return applyChopper(famWave, phase, dt, topo, lifeMode);
    }

    void setBankAndTopology(int bank, int topo) noexcept {
        setWaveShape(std::clamp(bank, 0, 3) * 4 + std::clamp(topo, 0, 3));
    }

    void setRoutingMode(int mode) noexcept {
        routingMode_ = std::clamp(mode, 0, 2);
    }
    [[nodiscard]] int getRoutingMode() const noexcept { return routingMode_; }

    void setPreChopperRouting(bool preChop) noexcept {
        routingMode_ = preChop ? 1 : 0;
    }
    [[nodiscard]] bool isPreChopperRouting() const noexcept { return routingMode_ == 1; }

    void setLifeMode(int mode) noexcept {
        lifeMode_ = std::clamp(mode, 0, 2);
        const double newHz = (lifeMode_ == 1) ? kCyborgTimerHz : kMasterCrystalHz;
        if (newHz != clockHz_) {
            const double r = newHz / clockHz_;
            crystalTicks_ *= r;
            slaveTicks_   *= r;
            clockHz_ = newHz;
            clockTicksPerSample_ = clockHz_ * invSr_;
        }
    }
    [[nodiscard]] int getLifeMode() const noexcept { return lifeMode_; }

    void setChirpMode(int mode) noexcept {
        chirpMode_ = std::clamp(mode, 0, 2);
    }
    [[nodiscard]] int getChirpMode() const noexcept { return chirpMode_; }

    void setMult(float m) noexcept {
        mult_ = std::clamp(m, 1.0f, 32.0f);
        syncAmount_ = 1.0f;
    }
    [[nodiscard]] float getMultAmount() const noexcept { return mult_; }

    void setApfOctaveOffset(float octaves) noexcept {
        apfOctaveOffset_ = std::clamp(octaves, -2.0f, 2.0f);
        useKeyTrackedApf_ = true;
    }
    [[nodiscard]] float getApfOctaveOffset() const noexcept { return apfOctaveOffset_; }

    void setUseKeyTrackedApf(bool useKeyTrack) noexcept {
        useKeyTrackedApf_ = useKeyTrack;
    }

    void setPwmLfo(float depth, float rateHz = 0.85f) noexcept {
        pwmLfoDepth_ = std::clamp(depth, 0.0f, 1.0f);
        pwmLfoRateHz_ = std::clamp(rateHz, 0.05f, 20.0f);
    }

    void setSuperSawSpread(float spread) noexcept {
        superSawSpread_ = std::clamp(spread, 0.0f, 1.0f);
    }
    [[nodiscard]] float getSuperSawSpread() const noexcept { return superSawSpread_; }

    void setSwarmDepth(float swarm) noexcept {
        swarmDepth_ = std::clamp(swarm, 0.0f, 1.0f);
    }
    [[nodiscard]] float getSwarmDepth() const noexcept { return swarmDepth_; }

    void setStemMix(float coreLvl, float subLvl, float superSawLvl, float noiseLvl) noexcept {
        coreLevel_ = std::clamp(coreLvl, 0.0f, 1.0f);
        subLevel_ = std::clamp(subLvl, 0.0f, 1.0f);
        superSawLevel_ = std::clamp(superSawLvl, 0.0f, 1.0f);
        noiseLevel_ = std::clamp(noiseLvl, 0.0f, 1.0f);
    }

    void setSyncAmount(float sync) noexcept {
        syncAmount_ = std::clamp(sync, -1.0f, 1.0f);
    }
    [[nodiscard]] float getSyncAmount() const noexcept { return syncAmount_; }

    void setStrangleAmount(float strangle) noexcept {
        strangleAmount_ = std::clamp(strangle, -1.0f, 1.0f);
        const float u = std::abs(strangleAmount_);
        constexpr float kDeadband = 0.02f;
        constexpr float kTopLatch = 0.98f;

        if (u <= kDeadband) {
            syncAmount_ = 0.0f;
            mult_ = 1.0f;
            return;
        }

        const float sign = (strangleAmount_ >= 0.0f) ? 1.0f : -1.0f;
        const float syncFade = std::min(1.0f, (u - kDeadband) / 0.04f);
        syncAmount_ = sign * syncFade;

        float octaves = 0.0f;
        if (u >= kTopLatch) {
            octaves = 5.0f;
        } else {
            const float unorm = (u - kDeadband) / (kTopLatch - kDeadband);
            octaves = unorm * 5.0f;
        }

        mult_ = std::pow(2.0f, octaves);
    }
    [[nodiscard]] float getStrangleAmount() const noexcept { return strangleAmount_; }

    void setApfFeedback(float fb) noexcept {
        apfFeedback_ = std::clamp(fb, -0.95f, 0.95f);
    }
    [[nodiscard]] float getApfFeedback() const noexcept { return apfFeedback_; }

    [[nodiscard]] float getSuperSawL() const noexcept { return superSawOutL_; }
    [[nodiscard]] float getSuperSawR() const noexcept { return superSawOutR_; }

    void setCoreLevel(float lvl) noexcept { coreLevel_ = std::clamp(lvl, 0.0f, 1.0f); }
    void setSubLevel(float lvl) noexcept { subLevel_ = std::clamp(lvl, 0.0f, 1.0f); }
    void setSuperSawLevel(float lvl) noexcept { superSawLevel_ = std::clamp(lvl, 0.0f, 1.0f); }
    void setNoiseLevel(float lvl) noexcept { noiseLevel_ = std::clamp(lvl, 0.0f, 1.0f); }

    void setLegacy14Mode(bool legacy) noexcept { legacy14Mode_ = legacy; }

    static float generateWave(int shapeIdx, float phase, float dt, float pwm) noexcept {
        if (shapeIdx >= 0 && shapeIdx <= 15) {
            return TbdAudio::Oscillators::PolyBLEP::alphaWave16(shapeIdx, phase, dt, pwm);
        }
        return TbdAudio::Oscillators::PolyBLEP::alphaWave14(shapeIdx, phase, dt, pwm);
    }

    static float generateWave16(int shapeIdx, float phase, float dt, float pwm) noexcept {
        return TbdAudio::Oscillators::PolyBLEP::alphaWave16(shapeIdx, phase, dt, pwm);
    }

    static float generateWave14(int shapeIdx, float phase, float dt, float pwm) noexcept {
        return TbdAudio::Oscillators::PolyBLEP::alphaWave14(shapeIdx, phase, dt, pwm);
    }

    /**
     * @brief Advance phase and generate simultaneous outputs on 12 MHz master crystal timebase.
     */
    void advanceTimebase(float freqHz, float pwm, float mult = 1.0f) noexcept {
        const float nyquist = static_cast<float>(sampleRate_ * 0.48);
        const float effFreq = std::clamp(freqHz, 5.0f, nyquist);
        lastFreqHz_ = effFreq;

        // 12 MHz Crystal Clock Quantization (Integer Period in 12 MHz Ticks)
        const uint32_t periodTicks = static_cast<uint32_t>(
            std::clamp(std::round(clockHz_ / effFreq), 16.0, clockHz_)
        );
        const double dPeriod = static_cast<double>(periodTicks);

        // MULT: 12 MHz Master Crystal Reload Quantization for Slave Accumulator
        const float mMax = std::max(1.0f, 0.45f * static_cast<float>(sampleRate_) / std::max(effFreq, 1.0f));
        const float m = std::clamp((mult != 1.0f) ? mult : mult_, 1.0f, std::min(32.0f, mMax));
        const uint32_t slavePeriodTicks = static_cast<uint32_t>(
            std::clamp(std::round(dPeriod / static_cast<double>(m)), 1.0, dPeriod)
        );
        const double dSlavePeriod = static_cast<double>(slavePeriodTicks);

        crystalTicks_ += clockTicksPerSample_;
        slaveTicks_ += clockTicksPerSample_;

        prevPhase_ = phase_;
        wrapped_ = false;

        // Wrap 12 MHz master crystal counter
        if (crystalTicks_ >= dPeriod) {
            crystalTicks_ -= dPeriod;
            if (crystalTicks_ >= dPeriod) crystalTicks_ = std::fmod(crystalTicks_, dPeriod);
            wrapped_ = true;
            if (syncAmount_ >= 0.0f) {
                const double syncTarget = std::fmod(crystalTicks_, dSlavePeriod);
                slaveTicks_ = (1.0 - static_cast<double>(syncAmount_)) * slaveTicks_ + static_cast<double>(syncAmount_) * syncTarget;
            } else {
                const double h = static_cast<double>(std::abs(syncAmount_));
                const double halfPeriod = dSlavePeriod * 0.5;
                slaveTicks_ = std::fmod(slaveTicks_ + h * halfPeriod, dSlavePeriod);
            }

            // Click-free shape transition on master phase wrap
            activeShapeIdx_ = pendingShapeIdx_;
        }

        phase_ = static_cast<float>(crystalTicks_ / dPeriod);

        // Wrap 12 MHz slave counter
        if (slaveTicks_ >= dSlavePeriod) {
            slaveTicks_ -= dSlavePeriod;
            if (slaveTicks_ >= dSlavePeriod) slaveTicks_ = std::fmod(slaveTicks_, dSlavePeriod);
        }
        slavePhase_ = static_cast<float>(slaveTicks_ / dSlavePeriod);

        const float dt0 = static_cast<float>((clockHz_ / dPeriod) * invSr_);
        const float effDt0 = std::max(dt0, 1e-6f);

        // 1. Advance Automated PWM LFO
        float dynamicPw = std::clamp(0.5f + 0.45f * pwm, 0.05f, 0.95f);
        if (pwmLfoDepth_ > 0.001f) {
            pwmLfoPhase_ += static_cast<float>(pwmLfoRateHz_ * invSr_);
            if (pwmLfoPhase_ >= 1.0f) pwmLfoPhase_ -= 1.0f;
            const float pwMod = std::sin(pwmLfoPhase_ * 6.28318530717958647692f) * (pwmLfoDepth_ * 0.38f);
            dynamicPw = std::clamp(dynamicPw + pwMod, 0.05f, 0.95f);
        }
        lastDynamicPw_ = dynamicPw;

        // 2. Dual-Control Sub-Oscillator with Leaky DC Blocker
        float subMult = 0.5f;
        if (subOctave_ == -2) subMult = 0.25f;
        else if (subOctave_ == -1) subMult = 0.5f;
        else if (subOctave_ == 0) subMult = 1.0f;
        else if (subOctave_ == 1) subMult = 2.0f;

        const double fSub = (clockHz_ / dPeriod) * static_cast<double>(subMult);
        subApf_.setTracking(fSub, static_cast<double>(subChewOct_));

        const float subDetuneMult = (std::abs(subDetuneSemitones_) > 0.00001f)
            ? std::pow(2.0f, subDetuneSemitones_ * (1.0f / 12.0f)) : 1.0f;
        subPhase_ += effDt0 * subMult * subDetuneMult;
        if (subPhase_ >= 1.0f) subPhase_ -= std::floor(subPhase_);
        const float rawSub = renderDualControlSub(subOctave_, subPulseTrain_, subPhase_, effDt0 * subMult);
        const auto apfSub = subApf_.process(rawSub, subApfTap_);
        subOut_ = subDcFilter_.process(apfSub.audioOut);
        burbleMod_ = apfSub.burbleMod;

        // 3. Dedicated White Noise Generator
        noiseOut_ = noiseGen_.nextWhite();
    }

    [[nodiscard]] float process(
        float freqHz,
        int shapeIdx,
        float pwm,
        float apfCutoff,
        float drive,
        float mult = 1.0f,
        int driveTopology = 0,
        bool syncReset = false,
        float syncAmount = 0.0f,
        float syncFrac = 0.0f) noexcept
    {
        pendingShapeIdx_ = std::clamp(shapeIdx, 0, 15);

        const float nyquist = static_cast<float>(sampleRate_ * 0.48);
        float effFreqHz = freqHz;

        if (legacy14Mode_) {
            if (activeShapeIdx_ >= 8 && activeShapeIdx_ <= 11) {
                effFreqHz = freqHz * 0.5f;
            } else if (activeShapeIdx_ >= 12 && activeShapeIdx_ <= 13) {
                effFreqHz = freqHz * 0.25f;
            }
        }

        const float f = std::clamp(effFreqHz, 5.0f, nyquist);
        lastFreqHz_ = f;

        // 12 MHz Crystal Clock Quantization (Integer Period in 12 MHz Ticks)
        const uint32_t periodTicks = static_cast<uint32_t>(
            std::clamp(std::round(clockHz_ / f), 16.0, clockHz_)
        );
        const double dPeriod = static_cast<double>(periodTicks);
        const float quantizedFreq = static_cast<float>(clockHz_ / dPeriod);
        const float dt0 = quantizedFreq * static_cast<float>(invSr_);

        // MULT: 12 MHz Master Crystal Reload Quantization for Slave Accumulator
        const float mMax = std::max(1.0f, 0.45f * static_cast<float>(sampleRate_) / std::max(f, 1.0f));
        const float m = std::clamp((mult != 1.0f) ? mult : mult_, 1.0f, std::min(32.0f, mMax));
        const uint32_t slavePeriodTicks = static_cast<uint32_t>(
            std::clamp(std::round(dPeriod / static_cast<double>(m)), 1.0, dPeriod)
        );
        const double dSlavePeriod = static_cast<double>(slavePeriodTicks);
        const float quantizedSlaveFreq = static_cast<float>(clockHz_ / dSlavePeriod);
        const float dts = quantizedSlaveFreq * static_cast<float>(invSr_);

        // Positive Edge Counter Sync on 12 MHz Master Crystal Grid
        bool doReset = false;
        if (syncReset && syncAmount > 0.0001f) {
            syncEdgeAccum_ += static_cast<double>(std::clamp(syncAmount, 0.0f, 1.0f));
            if (syncEdgeAccum_ >= 1.0) {
                syncEdgeAccum_ -= 1.0;
                if (syncEdgeAccum_ >= 1.0) syncEdgeAccum_ = std::fmod(syncEdgeAccum_, 1.0);
                doReset = true;
            }
        } else if (syncAmount <= 0.0001f) {
            syncEdgeAccum_ = 0.0;
        }

        if (doReset) {
            const double syncSubTick = std::round(std::clamp(static_cast<double>(syncFrac), 0.0, 1.0) * clockTicksPerSample_);
            const double remainingTicks = std::max(0.0, clockTicksPerSample_ - syncSubTick);
            crystalTicks_ = remainingTicks;
            slaveTicks_ = remainingTicks;
        } else {
            crystalTicks_ += clockTicksPerSample_;
            slaveTicks_ += clockTicksPerSample_;
        }

        prevPhase_ = phase_;
        wrapped_ = false;

        // Wrap 12 MHz master crystal counter
        if (crystalTicks_ >= dPeriod) {
            crystalTicks_ -= dPeriod;
            if (crystalTicks_ >= dPeriod) crystalTicks_ = std::fmod(crystalTicks_, dPeriod);
            wrapped_ = true;
            if (syncAmount_ >= 0.0f) {
                const double syncTarget = std::fmod(crystalTicks_, dSlavePeriod);
                slaveTicks_ = (1.0 - static_cast<double>(syncAmount_)) * slaveTicks_ + static_cast<double>(syncAmount_) * syncTarget;
            } else {
                const double h = static_cast<double>(std::abs(syncAmount_));
                const double halfPeriod = dSlavePeriod * 0.5;
                slaveTicks_ = std::fmod(slaveTicks_ + h * halfPeriod, dSlavePeriod);
            }

            // Double-buffered click-free shape transition on master phase wrap
            activeShapeIdx_ = pendingShapeIdx_;
        }

        phase_ = static_cast<float>(crystalTicks_ / dPeriod);

        // Wrap 12 MHz slave counter
        if (slaveTicks_ >= dSlavePeriod) {
            slaveTicks_ -= dSlavePeriod;
            if (slaveTicks_ >= dSlavePeriod) slaveTicks_ = std::fmod(slaveTicks_, dSlavePeriod);
        }
        slavePhase_ = static_cast<float>(slaveTicks_ / dSlavePeriod);

        const float effDt0 = std::max(dt0, 1e-6f);
        const float effDts = std::max(std::abs(dts), 1e-6f);

        // 1. Advance Automated PWM LFO
        float dynamicPw = std::clamp(0.5f + 0.45f * pwm, 0.05f, 0.95f);
        if (pwmLfoDepth_ > 0.001f) {
            pwmLfoPhase_ += static_cast<float>(pwmLfoRateHz_ * invSr_);
            if (pwmLfoPhase_ >= 1.0f) pwmLfoPhase_ -= 1.0f;
            const float pwMod = std::sin(pwmLfoPhase_ * 6.28318530717958647692f) * (pwmLfoDepth_ * 0.38f);
            dynamicPw = std::clamp(dynamicPw + pwMod, 0.05f, 0.95f);
        }
        lastDynamicPw_ = dynamicPw;
        const float normPwm = (dynamicPw - 0.5f) / 0.45f;

        // 2. Compute Effective APF Cutoff (Preserve caller cutoff when valid)
        float effApfCutoff = apfCutoff;
        if (effApfCutoff <= 0.0f) {
            effApfCutoff = std::clamp(f * std::exp2f(apfOctaveOffset_), 20.0f, nyquist);
        }

        // 3. Evaluate Core Waveforms according to Routing Topology
        if (routingMode_ == 0) {
            // ROUTING A (Post-Chop, Chew): Chopper/Wave -> APF -> Drive
            rawOut_ = generateWave16(activeShapeIdx_, phase_, effDt0, normPwm);
            multOut_ = generateWave16(activeShapeIdx_, slavePhase_, effDts, normPwm);

            prevProcessedOut_ = processedOut_;
            processedOut_ = apf_.processTopology(multOut_, effApfCutoff, drive, driveTopology, apfFeedback_);
        } else if (routingMode_ == 1) {
            // ROUTING B (Pre-Chop, Bite): Continuous Ramp -> APF -> Drive -> CMOS Chopper
            const float rawRamp = TbdAudio::Oscillators::PolyBLEP::saw(slavePhase_, effDts);
            const float saturatedRamp = apf_.processTopology(rawRamp, effApfCutoff, drive, driveTopology, apfFeedback_);

            int M = 1;
            if (activeShapeIdx_ == 0) M = 1;
            else if (activeShapeIdx_ == 1) M = 2;
            else if (activeShapeIdx_ == 2) M = 4;
            else if (activeShapeIdx_ == 3) M = 8;
            else M = 1;

            if (activeShapeIdx_ <= 3) {
                const float pM = TbdAudio::Oscillators::PolyBLEP::wrapPhase(slavePhase_ * static_cast<float>(M));
                const float gatePulse = 0.5f * (TbdAudio::Oscillators::PolyBLEP::pulse(pM, effDts * static_cast<float>(M), dynamicPw) + 1.0f);
                const float u = saturatedRamp * gatePulse;
                const float mean = (dynamicPw * (dynamicPw - 1.0f)) / static_cast<float>(M);
                prevProcessedOut_ = processedOut_;
                processedOut_ = (u - mean) * 1.35f;
            } else {
                prevProcessedOut_ = processedOut_;
                processedOut_ = apf_.processTopology(
                    generateWave16(activeShapeIdx_, slavePhase_, effDts, normPwm),
                    effApfCutoff, drive, driveTopology, apfFeedback_);
            }

            rawOut_ = rawRamp;
            multOut_ = saturatedRamp;
        } else {
            // ROUTING C (Scratchpad Split Chirp): Raw -> APF 1 -> Infinifold -> Chopper -> Inverted APF -> Tanh
            const float raw = generateWave16(activeShapeIdx_, slavePhase_, effDts, normPwm);

            const float apf1 = apf_.processAPF(raw, effApfCutoff, apfFeedback_);
            const float folded = apf_.applyInfinifold(apf1, drive);

            int M = 1;
            if (activeShapeIdx_ == 0) M = 1;
            else if (activeShapeIdx_ == 1) M = 2;
            else if (activeShapeIdx_ == 2) M = 4;
            else if (activeShapeIdx_ == 3) M = 8;
            else M = 1;

            const float pM = TbdAudio::Oscillators::PolyBLEP::wrapPhase(slavePhase_ * static_cast<float>(M));
            const float gatePulse = 0.5f * (TbdAudio::Oscillators::PolyBLEP::pulse(pM, effDts * static_cast<float>(M), dynamicPw) + 1.0f);
            const float u = folded * gatePulse;
            const float mean = (dynamicPw * (dynamicPw - 1.0f)) / static_cast<float>(M);
            const float chopped = (u - mean) * 1.35f;

            float apf2 = chopped;
            if (chirpMode_ == 0) {
                apf2 = apf_.processInvertedAPF(chopped, effApfCutoff);
            } else if (chirpMode_ == 1) {
                apf2 = apf_.processBiquadChirp(chopped, 4500.0f, 3.2f);
            } else {
                apf2 = apf_.processBiquadChirp(chopped, std::clamp(f * 8.0f, 100.0f, nyquist), 3.0f);
            }

            prevProcessedOut_ = processedOut_;
            processedOut_ = apf_.applyTanh(apf2, drive);

            rawOut_ = raw;
            multOut_ = folded;
        }

        apfTrigger_ = (prevProcessedOut_ <= 0.0f && processedOut_ > 0.0f);

        // 4. Dual-Control Sub-Oscillator with Leaky DC Blocker
        float subMult = 0.5f;
        if (subOctave_ == -2) subMult = 0.25f;
        else if (subOctave_ == -1) subMult = 0.5f;
        else if (subOctave_ == 0) subMult = 1.0f;
        else if (subOctave_ == 1) subMult = 2.0f;

        const double fSub = static_cast<double>(lastFreqHz_) * static_cast<double>(subMult);
        subApf_.setTracking(fSub, static_cast<double>(subChewOct_));

        const float subDetuneMult = (std::abs(subDetuneSemitones_) > 0.00001f)
            ? std::pow(2.0f, subDetuneSemitones_ * (1.0f / 12.0f)) : 1.0f;
        subPhase_ += effDt0 * subMult * subDetuneMult;
        if (subPhase_ >= 1.0f) subPhase_ -= std::floor(subPhase_);
        const float rawSub = renderDualControlSub(subOctave_, subPulseTrain_, subPhase_, effDt0 * subMult);
        const auto apfSub = subApf_.process(rawSub, subApfTap_);
        subOut_ = subDcFilter_.process(apfSub.audioOut);
        burbleMod_ = apfSub.burbleMod;

        // 5. Unison Saw Swarm Engine ("Bees in a Box")
        swarmLfoPhase_ += static_cast<float>((0.6 + static_cast<double>(swarmDepth_) * 6.5) * invSr_);
        if (swarmLfoPhase_ >= 1.0f) swarmLfoPhase_ -= 1.0f;

        const float lfo1 = std::sin(swarmLfoPhase_ * 6.28318530717958647692f);
        const float lfo2 = std::sin((swarmLfoPhase_ * 1.37f + 0.3f) * 6.28318530717958647692f);

        const float baseDetune = superSawSpread_ * 0.038f;
        const float detuneA = baseDetune * (1.0f + swarmDepth_ * lfo1 * 0.75f);
        const float detuneB = baseDetune * (1.0f + swarmDepth_ * lfo2 * 0.75f);

        autopanPhase_ += static_cast<float>((0.3 + static_cast<double>(swarmDepth_) * 0.9) * invSr_);
        if (autopanPhase_ >= 1.0f) autopanPhase_ -= 1.0f;
        const float panSin = std::sin(autopanPhase_ * 6.28318530717958647692f) * superSawSpread_;
        const float panA = 0.5f + 0.5f * panSin;
        const float panB = 0.5f - 0.5f * panSin;

        const float sawCenter = TbdAudio::Oscillators::PolyBLEP::superSawStaircase(phase_, effDt0, 0.0f, 0.0f);
        const float sawA = TbdAudio::Oscillators::PolyBLEP::superSawStaircase(phase_, effDt0, detuneA, 0.0f);
        const float sawB = TbdAudio::Oscillators::PolyBLEP::superSawStaircase(phase_, effDt0, 0.0f, detuneB);

        superSawOutL_ = (sawCenter * 0.5f + sawA * (1.0f - panA) + sawB * (1.0f - panB)) * 0.65f;
        superSawOutR_ = (sawCenter * 0.5f + sawA * panA + sawB * panB) * 0.65f;
        superSawOut_ = 0.5f * (superSawOutL_ + superSawOutR_);

        // 6. Dedicated White Noise Generator
        noiseOut_ = noiseGen_.nextWhite();

        // 7. Stem Mixer Summation
        compositeOut_ = processedOut_ * coreLevel_
                      + subOut_ * (subLevel_ * 0.45f)
                      + superSawOut_ * (superSawLevel_ * 0.65f)
                      + noiseOut_ * (noiseLevel_ * 0.35f);

        return processedOut_;
    }

    /**
     * @brief Process stereo output with Dual-Pole Route C, 5-Lane Mixer, and Master SPREAD.
     * Pristine analog signal chain:
     * - Pure continuous analog core at default (Saw, Pulse, Triangle, Sine)
     * - STRANGLE: Smooth multiplier & 12 MHz hard-sync
     * - WARP: Resonant APF squelch (100% transparent bypass at 0.00 Oct)
     * - MELT: Infinifold + Tanh wavefolding (100% linear bypass at 0.0)
     * - SWARM: Kinetic detune & stereo orbit (Pole A & B perfectly unified at 0.0, zero cancellation)
     */
        void processStereo(
        float f, float dt0, float pwm, float drive, int driveTopology, float apfCutoff,
        float& outL, float& outR) noexcept
    {
        const float nyquist = static_cast<float>(sampleRate_ * 0.48);

        // Effective APF Cutoff: locked to oscillator core frequency * 2^apfOctaveOffset_
        float effApfCutoff = apfCutoff;
        if (effApfCutoff <= 0.0f) {
            effApfCutoff = std::clamp(lastFreqHz_ * std::exp2f(apfOctaveOffset_), 20.0f, nyquist);
        }

        // Advance 12 MHz master crystal counter and slave accumulator
        advanceTimebase(f, pwm, mult_);

        // Master and slave phase steps for PolyBLEP anti-aliasing
        const float effDt0 = std::max(dt0, 1e-6f);
        const float effDts = std::clamp(effDt0 * mult_, 1e-6f, 0.48f);

        // 1. Core Wave: Pure, warm, continuous analog oscillator
        const float rawCore = generateFamilyWave(waveFamily_, slavePhase_, effDts, morph_, pwm, lifeMode_);

        // Bipolar BITE Attenuverter (-1.0 to +1.0):
        // Center 0.00 = 100% clean feedforward passthrough (Maw Open)
        // Calibrated: Knob extremes [-1.0, +1.0] scale to [-0.70, +0.70] internal feedback (the sweet spot!)
        // Clockwise > 0.00: Upper Bite / The Fang (In-phase, subharmonic period-doubling f0/2 lock)
        // Counter-Clockwise < 0.00: Lower Bite / The Underbite (Anti-phase, 3rd-harmonic clarinet chew)
        float inWithFb = rawCore;
        if (std::abs(bite_) > 0.001f) {
            const float effBite = bite_ * 0.70f;
            const float satFb = std::tanh(prevApf1Core_ * (1.0f + std::abs(effBite))) * 0.80f;
            inWithFb = rawCore - (effBite * 0.85f) * satFb;
        }

        // 2. Inverted APF First: High-frequency group delay dispersion (Downward smearing)
        // Always active, tracking oscillator pitch * 2^apfOctaveOffset_
        const float invCore = apf_.processInvertedAPF(inWithFb, effApfCutoff);

        // 3. MELT: Progressive Multi-Stage Infinifold Wavefolding (1 to 4 clean folds)
        const float foldCore = (drive > 0.001f) ? apf_.applyInfinifold(invCore, drive) : invCore;

        // 4. CMOS Chopper (Applied strictly once when chopperTopo_ > 0)
        const float rawChop = (chopperTopo_ > 0) ? applyChopper(foldCore, slavePhase_, effDts, chopperTopo_, lifeMode_) : foldCore;
        const float chopCore = applyChopperLifeform(rawChop, prevChopCore_, lifeMode_, chopperSpikeState_);
        prevChopCore_ = rawChop;

        // 5. APF 2: 4-Pole (Laser Sandwich Stage 2 — Post-Chopper Smearing via dedicated apfB_)
        const float apf1Core = apfB_.processAPF(chopCore, effApfCutoff, 0.0f);

        // Update recursive feedback state through DC servo & HF damping
        prevApf1Core_ = biteHfFilter_.process(biteDcFilter_.process(apf1Core));

        // 6. MELT: Punchy Normalized Tanh Overdrive (Zero volume sag, un-gentle analog bite)
        float satCore = apf1Core;
        if (drive > 0.001f) {
            const float g = 1.0f + drive * 2.5f;
            satCore = std::tanh(apf1Core * g) / std::tanh(g);
        }

        // 7. 10 Hz Leaky DC Blocker
        const float centerCore = coreDcFilter_.process(satCore);

        // 8. 100% Focused Mono DCO Core (Zero comb filtering, zero parallel cancellation!)
        const float coreL = centerCore;
        const float coreR = centerCore;

        // 9. 4-Lane Stem Mixer Summation with -3 dB Equal-Energy Headroom
        const float wCore = coreLevel_;
        const float wSub = subLevel_;
        const float wNoise = noiseLevel_;
        const float sumWeightSq = wCore * wCore + wSub * wSub + wNoise * wNoise;
        const float headroomGain = (sumWeightSq > 1.0f) ? (1.0f / std::sqrt(sumWeightSq)) : 1.0f;

        const float mixRawL = (coreL * wCore + subOut_ * wSub + noiseOut_ * wNoise) * headroomGain;
        const float mixRawR = (coreR * wCore + subOut_ * wSub + noiseOut_ * wNoise) * headroomGain;

        // 10. Pre-VCF Tanh Saturation Bus
        const float satDrive = 1.0f + mixerDrive_ * 4.0f;
        const float mixL = (mixerDrive_ > 0.001f) ? std::tanh(mixRawL * satDrive) : mixRawL;
        const float mixR = (mixerDrive_ > 0.001f) ? std::tanh(mixRawR * satDrive) : mixRawR;

        // 11. Master SPREAD (0% Mono Collapse -> 150% 3D) with 120Hz Elliptical Sub-Mono Protection
        const float mid = 0.5f * (mixL + mixR);
        float side = 0.5f * (mixL - mixR);
        side = sideHp_.process(side);

        if (masterSpread_ <= 0.00001f) {
            outL = mid;
            outR = mid;
        } else {
            outL = mid + side * masterSpread_;
            outR = mid - side * masterSpread_;
        }

        // Live telemetry & taps
        prevProcessedOut_ = processedOut_;
        processedOut_ = centerCore;
        compositeOut_ = 0.5f * (mixL + mixR);
        rawOut_ = rawCore;
        multOut_ = centerCore;
    }

    // --- Output Taps & State Accessors ---
    [[nodiscard]] float getPhase() const noexcept { return phase_; }
    [[nodiscard]] float getSlavePhase() const noexcept { return slavePhase_; }
    [[nodiscard]] float getPrevPhase() const noexcept { return prevPhase_; }
    [[nodiscard]] bool getSquare() const noexcept { return phase_ < 0.5f; }
    [[nodiscard]] bool didWrap() const noexcept { return wrapped_; }
    [[nodiscard]] bool didApfTrigger() const noexcept { return apfTrigger_; }

    [[nodiscard]] float getRaw() const noexcept { return rawOut_; }
    [[nodiscard]] float getMult() const noexcept { return multOut_; }
    [[nodiscard]] float getProcessed() const noexcept { return processedOut_; }
    [[nodiscard]] float getCore() const noexcept { return processedOut_; }
    [[nodiscard]] float getSub() const noexcept { return subOut_; }
    [[nodiscard]] float getSuperSaw() const noexcept { return superSawOut_; }
    [[nodiscard]] float getNoise() const noexcept { return noiseOut_; }
    [[nodiscard]] float getComposite() const noexcept { return compositeOut_; }

    [[nodiscard]] float getFreq() const noexcept { return lastFreqHz_; }
    [[nodiscard]] float getDynamicPw() const noexcept { return lastDynamicPw_; }
    [[nodiscard]] int getActiveShape() const noexcept { return activeShapeIdx_; }

private:
    double sampleRate_ = 44100.0;
    double invSr_ = 1.0 / 44100.0;
    double clockHz_ = kMasterCrystalHz;
    double clockTicksPerSample_ = 250.0;

    double crystalTicks_ = 0.0;
    double slaveTicks_ = 0.0;
    double syncEdgeAccum_ = 0.0;

    float phase_ = 0.0f;
    float slavePhase_ = 0.0f;
    float prevPhase_ = 0.0f;
    float subPhase_ = 0.0f;
    float pwmLfoPhase_ = 0.0f;
    float swarmLfoPhase_ = 0.0f;

    bool wrapped_ = false;
    bool apfTrigger_ = false;

    int activeShapeIdx_ = 0;
    int pendingShapeIdx_ = 0;

    int routingMode_ = 0;
    int chirpMode_ = 1;
    bool useKeyTrackedApf_ = false;
    bool legacy14Mode_ = false;

    float apfOctaveOffset_ = 0.0f;
    float mult_ = 1.0f;
    float pwmLfoDepth_ = 0.0f;
    float pwmLfoRateHz_ = 0.85f;
    float superSawSpread_ = 0.0f;
    float swarmDepth_ = 0.0f;

    float coreLevel_ = 1.0f;
    float subLevel_ = 0.0f;
    float superSawLevel_ = 0.0f;
    float noiseLevel_ = 0.0f;

    float rawOut_ = 0.0f;
    float multOut_ = 0.0f;
    float processedOut_ = 0.0f;
    float prevProcessedOut_ = 0.0f;
    float subOut_ = 0.0f;
    float superSawOut_ = 0.0f;
    float superSawOutL_ = 0.0f;
    float superSawOutR_ = 0.0f;
    float noiseOut_ = 0.0f;
    float compositeOut_ = 0.0f;
    float autopanPhase_ = 0.0f;
    float syncAmount_ = 1.0f;
    float strangleAmount_ = 0.0f;
    int subOctave_ = -1;
    float apfFeedback_ = 0.55f;
    float prevApf1Core_ = 0.0f;
    float bite_ = 0.0f;
    LeakyDCBlocker biteDcFilter_;
    OnePoleLP biteHfFilter_;

    float lastFreqHz_ = 130.81f;
    float lastDynamicPw_ = 0.5f;

    AllPassFilter4 apf_;
    AllPassFilter4 apfB_;
    SubAPF16 subApf_;
    float subChewOct_ = 0.0f;
    int subApfTap_ = 0;
    float burbleMod_ = 0.0f;
    float subDetuneSemitones_ = 0.0f;
    LeakyDCBlocker subDcFilter_;
    LeakyDCBlocker coreDcFilter_;
    OnePoleHP sideHp_;

    int waveFamily_ = 0;       // 0: SAW, 1: PULSE, 2: TRIANGLE, 3: EXOTIC
    int chopperTopo_ = 0;      // 0: 1x RAW, 1: 2x TWIN, 2: 4x QUAD, 3: 8x OCTO
    int lifeMode_ = 1;         // 0: Synthetic, 1: Cyborg, 2: Organic
    float prevChopCore_ = 0.0f;
    float chopperSpikeState_ = 0.0f;
    float morph_ = 0.0f;       // 0.0 .. 1.0
    int subPulseTrain_ = 0;    // 0: Sq, 1: 25% Pls, 2: 2-Burst, 3: 3-Burst, 4: 4-Burst
    float masterSpread_ = 1.0f;// 0.0 (Mono) .. 1.5 (Wide 3D)
    float mixerDrive_ = 0.2f;  // 0.0 .. 1.0 Pre-VCF Tanh Saturation
    TbdAudio::Oscillators::PolyBLEP::NoiseGenerator noiseGen_;
};

} // namespace CrackedChimes::DSP
