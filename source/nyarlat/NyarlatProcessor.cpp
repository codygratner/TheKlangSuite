#include "NyarlatProcessor.h"

#include "NyarlatEditor.h"

#include "../dsp/common/MasterTaper.h"

#include <algorithm>



namespace Nyarlat {



NyarlatProcessor::NyarlatProcessor()

    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),

      apvts_(*this, nullptr, "NyarlatParams", nyarlatParameters::createLayout())

{

    AkwfRom::getInstance().load();



    for (int i = 0; i < kNumVoices; ++i) {

        voices_[i].setVoiceIndex(i);

    }

    heldNotes_.reserve(16);



    auto presets = presetManager_.getAvailablePresets();

    if (!presets.isEmpty()) {

        presetManager_.loadPresetByIndex(0, apvts_);

    }

}



void NyarlatProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {

    juce::ignoreUnused(samplesPerBlock);



    TbdAudio::FastMath::enableFTZDAZ();



    for (auto& v : voices_) {

        v.prepare(sampleRate);

    }

    masterChaos_ = Nyarlat::VoiceChaosEngine();
    masterChaos_.setVoiceSeed(99, 0.0f);
    smoothedChorusRate_ = 1.2f;



    bassBpL_.set(40.0f, 0.7f, static_cast<float>(sampleRate));

    bassBpR_.set(40.0f, 0.7f, static_cast<float>(sampleRate));

    masterDcBlockerL_.prepare(sampleRate, 5.0);

    masterDcBlockerR_.prepare(sampleRate, 5.0);

    postDcBlockerL_.prepare(sampleRate, 5.0);

    postDcBlockerR_.prepare(sampleRate, 5.0);

    trebleStateL_ = trebleStateR_ = 0.0f;



    hilbertL_.reset();

    hilbertR_.reset();

    chorusPhase_ = 0.0f;

    heldNotes_.clear();

    masterSilenceSamples_ = 0;

    masterSleeping_ = false;

}



void NyarlatProcessor::releaseResources() {

    for (auto& v : voices_) v.reset();


    masterSilenceSamples_ = 0;

    masterSleeping_ = false;

    bassBpL_.reset();

    bassBpR_.reset();

    masterDcBlockerL_.reset();

    masterDcBlockerR_.reset();

    postDcBlockerL_.reset();

    postDcBlockerR_.reset();

    hilbertL_.reset();

    hilbertR_.reset();

    chorusPhase_ = 0.0f;

    heldNotes_.clear();

}



void NyarlatProcessor::handleMidiEvent(const juce::MidiMessage& msg) noexcept {

    if (msg.isNoteOn()) {

        allocateVoice(msg.getNoteNumber(), msg.getFloatVelocity());

    } else if (msg.isNoteOff()) {

        releaseVoice(msg.getNoteNumber());

    } else if (msg.isAllNotesOff() || msg.isAllSoundOff()) {

        heldNotes_.clear();

        for (auto& v : voices_) v.noteOff();

    } else if (msg.isPitchWheel()) {

        globalPitchWheel_ = static_cast<float>(msg.getPitchWheelValue() - 8192) / 8192.0f;

    } else if (msg.isController() && msg.getControllerNumber() == 1) { // Mod Wheel

        globalModWheel_ = msg.getControllerValue() / 127.0f;

    } else if (msg.isChannelPressure()) { // Channel Aftertouch

        globalChannelAftertouch_ = msg.getChannelPressureValue() / 127.0f;

    }

}



void NyarlatProcessor::allocateVoice(int midiNote, float velocity) noexcept {

    namespace Ids = nyarlatParameters::Ids;

    const int voiceMode = static_cast<int>(*apvts_.getRawParameterValue(Ids::voice_mode));

    const float glide = *apvts_.getRawParameterValue(Ids::glide_time);

    const float hammerSnap = *apvts_.getRawParameterValue(Ids::noise_hammer_snap);

    const float oscSlop = *apvts_.getRawParameterValue(Ids::osc_slop);

    const float polySpread = *apvts_.getRawParameterValue(Ids::chaos_poly_spread);



    if (voiceMode == 0) {

        // Poly 16: Intelligent 4-Tier Voice Allocation & Stealing

        int bestIdx = -1;

        // Tier 0: Search for first inactive (sleeping) voice
        for (int i = 0; i < kNumVoices; ++i) {

            int idx = (nextVoiceIndex_ + i) % kNumVoices;

            if (!voices_[idx].isActive()) {

                bestIdx = idx;

                break;

            }

        }

        // Tier 1: Search for active voice already playing this exact MIDI note (retrigger voice-leading)
        if (bestIdx == -1) {

            for (int i = 0; i < kNumVoices; ++i) {

                if (voices_[i].getMidiNote() == midiNote) {

                    bestIdx = i;

                    break;

                }

            }

        }

        // Tier 2: Steal releasing voice with lowest envelope level
        if (bestIdx == -1) {

            float minReleaseLvl = 1e9f;

            int quietestReleaseIdx = -1;

            for (int i = 0; i < kNumVoices; ++i) {

                if (voices_[i].getAmpEnv().stage == NyarlatVoice::EnvStage::Release) {

                    const float lvl = voices_[i].getAmpEnv().level;

                    if (lvl < minReleaseLvl) {

                        minReleaseLvl = lvl;

                        quietestReleaseIdx = i;

                    }

                }

            }

            if (quietestReleaseIdx != -1) {

                bestIdx = quietestReleaseIdx;

            }

        }

        // Tier 3: Fallback - steal oldest active voice (longest elapsed duration)
        if (bestIdx == -1) {

            uint64_t oldestTs = std::numeric_limits<uint64_t>::max();

            int oldestIdx = nextVoiceIndex_;

            for (int i = 0; i < kNumVoices; ++i) {

                const uint64_t ts = voices_[i].getNoteOnTimestamp();

                if (ts < oldestTs) {

                    oldestTs = ts;

                    oldestIdx = i;

                }

            }

            bestIdx = oldestIdx;

        }

        nextVoiceIndex_ = (bestIdx + 1) % kNumVoices;

        newestVoice_.store(bestIdx, std::memory_order_relaxed);

        voices_[bestIdx].noteOn(midiNote, velocity, glide, hammerSnap, -1.0f, oscSlop, ++noteCounter_);

    }

    else if (voiceMode == 1) {

        // Mono: Single Voice with Re-trigger

        heldNotes_.push_back({midiNote, velocity});

        voices_[0].noteOn(midiNote, velocity, glide, hammerSnap, -1.0f, oscSlop, ++noteCounter_);

        newestVoice_.store(0, std::memory_order_relaxed);

    }

    else if (voiceMode == 2) {

        // Legato: Pitch Glide without Envelope Retrigger

        const float targetHz = 440.0f * std::pow(2.0f, (midiNote - 69.0f) / 12.0f);

        if (!heldNotes_.empty() && voices_[0].isActive()) {

            voices_[0].setPitchTarget(targetHz, glide);

        } else {

            voices_[0].noteOn(midiNote, velocity, glide, hammerSnap, targetHz, oscSlop, ++noteCounter_);

        }

        heldNotes_.push_back({midiNote, velocity});

        newestVoice_.store(0, std::memory_order_relaxed);

    }

    else if (voiceMode == 3) {

        // 8x Unison Swarm: 24 additive oscillators with golden-ratio detune & chaos decorrelation

        heldNotes_.push_back({midiNote, velocity});

        for (int v = 0; v < 8; ++v) {

            const float detuneNorm = (static_cast<float>(v) - 3.5f) / 3.5f;

            const float detuneSemi = detuneNorm * 0.15f * (1.0f + oscSlop * 2.0f);

            const float targetHz = 440.0f * std::pow(2.0f, (midiNote + detuneSemi - 69.0f) / 12.0f);

            voices_[v].noteOn(midiNote, velocity, glide, hammerSnap, targetHz, oscSlop, ++noteCounter_);

            voices_[v].setPolyChaosSpread(polySpread * (static_cast<float>(v) / 7.0f));

        }

        newestVoice_.store(0, std::memory_order_relaxed);

    }

}



void NyarlatProcessor::releaseVoice(int midiNote) noexcept {

    namespace Ids = nyarlatParameters::Ids;

    const int voiceMode = static_cast<int>(*apvts_.getRawParameterValue(Ids::voice_mode));

    const float glide = *apvts_.getRawParameterValue(Ids::glide_time);

    const float hammerSnap = *apvts_.getRawParameterValue(Ids::noise_hammer_snap);

    const float oscSlop = *apvts_.getRawParameterValue(Ids::osc_slop);



    if (voiceMode == 0) {

        // Poly 16

        for (auto& v : voices_) {

            if (v.isActive() && v.getMidiNote() == midiNote) {

                v.noteOff();

            }

        }

    }

    else if (voiceMode == 1) {

        // Mono

        heldNotes_.erase(std::remove_if(heldNotes_.begin(), heldNotes_.end(),

            [midiNote](const auto& p) { return p.first == midiNote; }), heldNotes_.end());

        if (heldNotes_.empty()) {

            voices_[0].noteOff();

        } else {

            const auto prev = heldNotes_.back();

            voices_[0].noteOn(prev.first, prev.second, glide, hammerSnap);

        }

    }

    else if (voiceMode == 2) {

        // Legato

        heldNotes_.erase(std::remove_if(heldNotes_.begin(), heldNotes_.end(),

            [midiNote](const auto& p) { return p.first == midiNote; }), heldNotes_.end());

        if (heldNotes_.empty()) {

            voices_[0].noteOff();

        } else {

            const auto prev = heldNotes_.back();

            const float targetHz = 440.0f * std::pow(2.0f, (prev.first - 69.0f) / 12.0f);

            voices_[0].setPitchTarget(targetHz, glide);

        }

    }

    else if (voiceMode == 3) {

        // 8x Unison

        heldNotes_.erase(std::remove_if(heldNotes_.begin(), heldNotes_.end(),

            [midiNote](const auto& p) { return p.first == midiNote; }), heldNotes_.end());

        if (heldNotes_.empty()) {

            for (int v = 0; v < 8; ++v) voices_[v].noteOff();

        } else {

            const auto prev = heldNotes_.back();

            for (int v = 0; v < 8; ++v) {

                const float detuneNorm = (static_cast<float>(v) - 3.5f) / 3.5f;

                const float detuneSemi = detuneNorm * 0.15f * (1.0f + oscSlop * 2.0f);

                const float targetHz = 440.0f * std::pow(2.0f, (prev.first + detuneSemi - 69.0f) / 12.0f);

                voices_[v].setPitchTarget(targetHz, glide);

            }

        }

    }

}



void NyarlatProcessor::renderSegment(float* left, float* right, int numSamples, const VoiceParams& params, int activeVoiceCount) noexcept {

    for (int v = 0; v < kNumVoices; ++v) {

        if (!voices_[v].isActive()) continue;

        voices_[v].renderBlockAdd(

            left, right, numSamples, activeVoiceCount,

            params.oscBanks, params.oscMorphs, params.oscOddEvens, params.oscTilts, params.oscFormants, params.oscMasks,

            params.oscOctaves, params.oscSpeedLocks, params.oscSpeeds, params.oscLoops,

            params.osc1Lvl, params.osc2Lvl, params.osc3Lvl,

            params.noiseLvl, params.noiseTone, params.noiseHammerSnap,

            params.oscSpread, params.mixerSeethe,

            params.randAttenuverters, params.randSpread, params.randTune, 0.0f,

            params.filterCutoff, params.filterResonance, params.filterSlope, params.filterDrive,

            params.filterEnvDepth, params.filterVelocity, params.filterKeytrack,

            params.ampVel, params.globalBendSemi, globalModWheel_, globalChannelAftertouch_,
            params.bpm,
            params.oscSlop
        );

    }

}



void NyarlatProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {

    juce::ScopedNoDenormals noDenormals;

    TbdAudio::FastMath::enableFTZDAZ();



    const int numSamples = buffer.getNumSamples();



    float hostBpm = 120.0f;

    if (auto* ph = getPlayHead()) {

        if (auto pos = ph->getPosition()) {

            if (pos->getBpm().hasValue()) {

                const double b = *pos->getBpm();

                if (b > 20.0 && b < 999.0 && std::isfinite(b)) {

                    hostBpm = static_cast<float>(b);

                }

            }

        }

    }

    currentBpm_.store(hostBpm, std::memory_order_relaxed);

    float* leftChannel = buffer.getWritePointer(0);

    float* rightChannel = buffer.getWritePointer(1);



    namespace Ids = nyarlatParameters::Ids;



    VoiceParams params;



    // 1. Oscillator Bank Selectors

    params.oscBanks[0] = *apvts_.getRawParameterValue(Ids::osc1_bank);

    params.oscBanks[1] = *apvts_.getRawParameterValue(Ids::osc2_bank);

    params.oscBanks[2] = *apvts_.getRawParameterValue(Ids::osc3_bank);



    // 2. Additive Timbral Macros (6 parameters x 3 oscs)

    params.oscMorphs[0] = *apvts_.getRawParameterValue(Ids::osc1_morph);

    params.oscMorphs[1] = *apvts_.getRawParameterValue(Ids::osc2_morph);

    params.oscMorphs[2] = *apvts_.getRawParameterValue(Ids::osc3_morph);



    params.oscOddEvens[0] = *apvts_.getRawParameterValue(Ids::osc1_odd_even);

    params.oscOddEvens[1] = *apvts_.getRawParameterValue(Ids::osc2_odd_even);

    params.oscOddEvens[2] = *apvts_.getRawParameterValue(Ids::osc3_odd_even);



    params.oscTilts[0] = *apvts_.getRawParameterValue(Ids::osc1_tilt);

    params.oscTilts[1] = *apvts_.getRawParameterValue(Ids::osc2_tilt);

    params.oscTilts[2] = *apvts_.getRawParameterValue(Ids::osc3_tilt);



    params.oscFormants[0] = *apvts_.getRawParameterValue(Ids::osc1_formant);

    params.oscFormants[1] = *apvts_.getRawParameterValue(Ids::osc2_formant);

    params.oscFormants[2] = *apvts_.getRawParameterValue(Ids::osc3_formant);



    params.oscMasks[0] = static_cast<int>(*apvts_.getRawParameterValue(Ids::osc1_mask));

    params.oscMasks[1] = static_cast<int>(*apvts_.getRawParameterValue(Ids::osc2_mask));

    params.oscMasks[2] = static_cast<int>(*apvts_.getRawParameterValue(Ids::osc3_mask));



    params.oscSpeeds[0] = *apvts_.getRawParameterValue(Ids::osc1_speed);

    params.oscSpeeds[1] = *apvts_.getRawParameterValue(Ids::osc2_speed);

    params.oscSpeeds[2] = *apvts_.getRawParameterValue(Ids::osc3_speed);



    // 3. Tuning & Structure



    params.oscOctaves[0] = static_cast<int>(*apvts_.getRawParameterValue(Ids::osc1_octave));

    params.oscOctaves[1] = static_cast<int>(*apvts_.getRawParameterValue(Ids::osc2_octave));

    params.oscOctaves[2] = static_cast<int>(*apvts_.getRawParameterValue(Ids::osc3_octave));



    params.oscSpeedLocks[0] = *apvts_.getRawParameterValue(Ids::osc1_speed_lock) > 0.5f;

    params.oscSpeedLocks[1] = *apvts_.getRawParameterValue(Ids::osc2_speed_lock) > 0.5f;

    params.oscSpeedLocks[2] = *apvts_.getRawParameterValue(Ids::osc3_speed_lock) > 0.5f;

    params.bpm = currentBpm_.load(std::memory_order_relaxed);







    params.oscLoops[0] = static_cast<int>(*apvts_.getRawParameterValue(Ids::osc1_loop));

    params.oscLoops[1] = static_cast<int>(*apvts_.getRawParameterValue(Ids::osc2_loop));

    params.oscLoops[2] = static_cast<int>(*apvts_.getRawParameterValue(Ids::osc3_loop));



    params.osc1Lvl = *apvts_.getRawParameterValue(Ids::osc1_level);

    params.osc2Lvl = *apvts_.getRawParameterValue(Ids::osc2_level);

    params.osc3Lvl = *apvts_.getRawParameterValue(Ids::osc3_level);



    // 4. Noise & Mixer

    params.noiseLvl = *apvts_.getRawParameterValue(Ids::noise_level);

    params.noiseTone = *apvts_.getRawParameterValue(Ids::noise_tone);

    params.noiseHammerSnap = *apvts_.getRawParameterValue(Ids::noise_hammer_snap);

    params.oscSpread = *apvts_.getRawParameterValue(Ids::osc_spread);

    params.mixerSeethe = *apvts_.getRawParameterValue(Ids::mixer_seethe);
    params.width = *apvts_.getRawParameterValue(Ids::global_width);



    // 5. Chaos Deck

    params.chaosRate = *apvts_.getRawParameterValue(Ids::chaos_rate);

    params.chaosDrift = *apvts_.getRawParameterValue(Ids::chaos_drift);

    params.chaosPoly = *apvts_.getRawParameterValue(Ids::chaos_poly_spread);



    // 6 Consolidated Chaos Macro Parameters
    params.randChaos[0] = *apvts_.getRawParameterValue(Ids::rand_morph);
    params.randChaos[1] = *apvts_.getRawParameterValue(Ids::rand_odd_even);
    params.randChaos[2] = *apvts_.getRawParameterValue(Ids::rand_tilt);
    params.randChaos[3] = *apvts_.getRawParameterValue(Ids::rand_formant);
    params.randChaos[4] = *apvts_.getRawParameterValue(Ids::rand_mask);
    params.randChaos[5] = *apvts_.getRawParameterValue(Ids::rand_speed);

    params.randSpread = *apvts_.getRawParameterValue(Ids::rand_spread);
    params.randTune = *apvts_.getRawParameterValue(Ids::rand_tune);
    params.randChorusRate = *apvts_.getRawParameterValue(Ids::rand_chorus_rate);

    for (int k = 0; k < 6; ++k) {
        params.randAttenuverters[3 * k + 0] = params.randChaos[k];
        params.randAttenuverters[3 * k + 1] = params.randChaos[k];
        params.randAttenuverters[3 * k + 2] = params.randChaos[k];
    }

    // 6. Silky Analog Filter & Envelopes

    params.filterCutoff = *apvts_.getRawParameterValue(Ids::filter_cutoff);

    params.filterResonance = *apvts_.getRawParameterValue(Ids::filter_resonance);

    params.filterSlope = static_cast<int>(*apvts_.getRawParameterValue(Ids::filter_slope));

    params.filterDrive = *apvts_.getRawParameterValue(Ids::filter_drive);

    params.filterEnvDepth = *apvts_.getRawParameterValue(Ids::filter_env_depth);

    params.filterVelocity = *apvts_.getRawParameterValue(Ids::filter_velocity);

    params.filterKeytrack = *apvts_.getRawParameterValue(Ids::filter_keytrack);



    params.ampVel = *apvts_.getRawParameterValue(Ids::amp_velocity);

    const float pbRange = *apvts_.getRawParameterValue(Ids::pitch_bend_range);

    params.globalBendSemi = globalPitchWheel_ * pbRange;



    params.lifeMode = static_cast<int>(*apvts_.getRawParameterValue(Ids::life_mode));

    params.oscSlop = *apvts_.getRawParameterValue(Ids::osc_slop);



    // Envelopes & Timing

    const double sr = getSampleRate();

    const float ampA = *apvts_.getRawParameterValue(Ids::amp_attack);

    const float ampD = *apvts_.getRawParameterValue(Ids::amp_decay);

    const float ampS = *apvts_.getRawParameterValue(Ids::amp_sustain);

    const float ampR = *apvts_.getRawParameterValue(Ids::amp_release);

    const float ktRate = *apvts_.getRawParameterValue(Ids::keytrack_rate);



    const float filtA = *apvts_.getRawParameterValue(Ids::filter_attack);

    const float filtD = *apvts_.getRawParameterValue(Ids::filter_decay);

    const float filtS = *apvts_.getRawParameterValue(Ids::filter_sustain);

    const float filtR = *apvts_.getRawParameterValue(Ids::filter_release);



    const uint32_t uBanks[3] = {

        static_cast<uint32_t>(params.oscBanks[0]),

        static_cast<uint32_t>(params.oscBanks[1]),

        static_cast<uint32_t>(params.oscBanks[2])

    };



    const float mpeSlideDepth = *apvts_.getRawParameterValue(Ids::mpe_slide_depth);

    const float mpePressDepth = *apvts_.getRawParameterValue(Ids::mpe_pressure_depth);



    int activeVoiceCount = 0;

    for (int v = 0; v < kNumVoices; ++v) {

        if (!voices_[v].isActive()) continue;

        activeVoiceCount++;

        voices_[v].sanitize();



        voices_[v].setMpeSlide(globalModWheel_ * mpeSlideDepth);

        voices_[v].setMpePressure(globalChannelAftertouch_ * mpePressDepth);



        float noteScale = 1.0f - ktRate * (static_cast<float>(voices_[v].getMidiNote() - 60) / 60.0f);

        noteScale = juce::jlimit(0.2f, 1.8f, noteScale);



        voices_[v].getAmpEnv().setADSR(ampA, ampD, ampS, ampR, sr, noteScale);

        voices_[v].getFilterEnv().setADSR(filtA, filtD, filtS, filtR, sr, noteScale);

        voices_[v].advanceChaos(params.chaosRate, params.chaosDrift, numSamples / sr);

        voices_[v].updateWavetables(uBanks, params.oscMorphs, params.oscOddEvens, params.oscTilts, params.oscFormants, params.randAttenuverters, params.lifeMode, params.width, params.oscSpread, params.randSpread);

    }



    buffer.clear();



    // Timestamp-sliced voice rendering: slices at MIDI event boundaries for sample-accurate timing

    int currentSample = 0;

    for (const auto metadata : midiMessages) {

        const int eventPos = metadata.samplePosition;

        if (eventPos > currentSample) {

            const int segLen = std::min(eventPos - currentSample, numSamples - currentSample);

            if (segLen > 0) {

                renderSegment(leftChannel + currentSample, rightChannel + currentSample, segLen, params, activeVoiceCount);

                currentSample += segLen;

            }

        }

        handleMidiEvent(metadata.getMessage());

    }



    if (currentSample < numSamples) {

        renderSegment(leftChannel + currentSample, rightChannel + currentSample, numSamples - currentSample, params, activeVoiceCount);

    }



    // Master Silence Gate: count remaining active voices after all segment rendering
    int remainingActiveVoices = 0;

    for (int v = 0; v < kNumVoices; ++v) {

        if (voices_[v].isActive()) {

            remainingActiveVoices++;

        }

    }



    if (remainingActiveVoices == 0) {

        masterSilenceSamples_ += numSamples;

        constexpr int kTailSamples = 24000; // ~0.5s at 48kHz (more than enough for master SVF & DC blockers to ring out past -140 dBFS)

        if (masterSilenceSamples_ >= kTailSamples) {

            if (!masterSleeping_) {

                // One-time flush of master filters upon entering deep sleep

                bassBpL_.reset();

                bassBpR_.reset();

                trebleStateL_ = trebleStateR_ = 0.0f;

                hilbertL_.reset();

                hilbertR_.reset();

                masterDcBlockerL_.reset();

                masterDcBlockerR_.reset();

                postDcBlockerL_.reset();

                postDcBlockerR_.reset();

                masterSleeping_ = true;

            }



            // Buffer is already cleared to 0.0f. Fast UI telemetry update and early return (0.00% CPU!)

            newestVoice_.store(-1, std::memory_order_relaxed);

            for (int v = 0; v < kNumVoices; ++v) {

                telemetry_[v].active = false;

            }

            return;

        }

    } else {

        masterSilenceSamples_ = 0;

        masterSleeping_ = false;

    }



    // Master Stage Effects

    const float bassBoost = *apvts_.getRawParameterValue(Ids::bass_boost);

    const float trebleTame = *apvts_.getRawParameterValue(Ids::treble_tame);

    const float alphaChorus = *apvts_.getRawParameterValue(Ids::alpha_chorus);
    const float chorusRate = *apvts_.getRawParameterValue(Ids::chorus_rate);
    const float width = *apvts_.getRawParameterValue(Ids::global_width);
    const float masterVol = *apvts_.getRawParameterValue(Ids::output_level);

    // Master Chaos Engine update for continuous Chorus Rate frequency modulation
    masterChaos_.updateBlock(params.chaosRate, params.chaosDrift, static_cast<double>(numSamples) / sr);
    const float rawChaos = masterChaos_.getRawValue(20);
    masterChorusChaos_.store(rawChaos, std::memory_order_relaxed);
    const float bipolarChaos = rawChaos * 2.0f - 1.0f;
    const float modOct = bipolarChaos * params.randChorusRate * 2.0f;
    const float targetRateHz = std::clamp(chorusRate * std::exp2(modOct), 0.02f, 20.0f);
    const float slewCoeff = std::clamp(static_cast<float>(numSamples / (sr * 0.060)), 0.01f, 1.0f);
    smoothedChorusRate_ += slewCoeff * (targetRateHz - smoothedChorusRate_);
    const float chorusStep = static_cast<float>(6.28318530717958647692 * smoothedChorusRate_ / std::max(1000.0, sr));

    for (int i = 0; i < numSamples; ++i) {

        float l = leftChannel[i];

        float r = rightChannel[i];



        if (bassBoost > 0.01f) {

            l += 1.5f * bassBoost * bassBpL_.process(l);

            r += 1.5f * bassBoost * bassBpR_.process(r);

        }



        if (trebleTame > 0.01f) {

            const float coeff = 0.2f * trebleTame;

            trebleStateL_ = (1.0f - coeff) * trebleStateL_ + coeff * l;

            trebleStateR_ = (1.0f - coeff) * trebleStateR_ + coeff * r;

            l = trebleStateL_;

            r = trebleStateR_;

        }



        // Continuous Zero-Latency Hilbert Quadrature Alpha Chorus

        if (alphaChorus > 0.001f) {

            chorusPhase_ += chorusStep;

            if (chorusPhase_ >= 6.2831853f) chorusPhase_ -= 6.2831853f;



            float sigIL = 0.0f, sigQL = 0.0f;

            float sigIR = 0.0f, sigQR = 0.0f;

            hilbertL_.process(l, sigIL, sigQL);

            hilbertR_.process(r, sigIR, sigQR);



            const float lfoL = std::sin(chorusPhase_);

            const float lfoR = std::cos(chorusPhase_);

            const float rotAngleL = alphaChorus * 1.35f * lfoL;

            const float rotAngleR = alphaChorus * 1.35f * lfoR;



            const float rotL = std::cos(rotAngleL) * sigIL - std::sin(rotAngleL) * sigQL;

            const float rotR = std::cos(rotAngleR) * sigIR - std::sin(rotAngleR) * sigQR;



            const float dryGain = 1.0f - alphaChorus * 0.5f;

            const float wetGain = alphaChorus * 0.5f;

            l = dryGain * l + wetGain * rotL;

            r = dryGain * r + wetGain * rotR;

        }






        const float mid = (l + r) * 0.5f;

        float side = (r - l) * 0.5f;

        const float sideGain = std::min(1.0f, 2.0f * width);
        side *= sideGain;

        l = mid - side;

        r = mid + side;



        // 1. Pre-master DC centering

        l = masterDcBlockerL_.process(l);

        r = masterDcBlockerR_.process(r);



        // 2. Clean Master Gain Staging (+4.0 dB internal chaos mix makeup & +18 dB Master Taper)

        const float masterGain = tkdsp::common::evalMasterVolumeGain(masterVol);

        constexpr float kMakeup = 1.584893f; // +4.0 dB commercial loudness makeup

        l *= kMakeup * masterGain;

        r *= kMakeup * masterGain;



        // 3. Pre-sanitization before post-blocker

        if (!std::isfinite(l)) { l = 0.0f; postDcBlockerL_.reset(); }

        if (!std::isfinite(r)) { r = 0.0f; postDcBlockerR_.reset(); }



        // 4. Post-master DC blocker

        l = postDcBlockerL_.process(l);

        r = postDcBlockerR_.process(r);



        // 5. Emergency Guard (+30 dBFS float pass-through, zero 0 dBFS clamp)

        tkdsp::common::applyEmergencyGuardStereo(l, r);

        leftChannel[i] = l;

        rightChannel[i] = r;

    }



    const int nv = newestVoice_.load(std::memory_order_relaxed);

    if (nv >= 0 && !voices_[nv].isActive()) {

        int fallback = -1;

        for (int v = 0; v < kNumVoices; ++v) {

            if (voices_[v].isActive()) {

                fallback = v;

                break;

            }

        }

        newestVoice_.store(fallback, std::memory_order_relaxed);

    }



    const float normCutoff = std::log10(params.filterCutoff / 20.0f) / 3.0f;

    for (int v = 0; v < kNumVoices; ++v) {

        const bool active = voices_[v].isActive();

        telemetry_[v].active = active;

        if (!active) continue;



        telemetry_[v].note = voices_[v].getMidiNote();

        telemetry_[v].x = normCutoff;

        telemetry_[v].y = params.filterResonance;

        telemetry_[v].z = params.filterDrive;

        for (int i = 0; i < 3; ++i) {

            telemetry_[v].statorOffsets[i] = voices_[v].getStatorOffset(i);

            telemetry_[v].oscEffWave[i] = voices_[v].getEffWave(i);

            telemetry_[v].oscEffMask[i] = voices_[v].getEffMask(i);

            telemetry_[v].oscEffSpeed[i] = voices_[v].getEffSpeed(i);

        }

        telemetry_[v].effSpread = voices_[v].getEffSpread();

        telemetry_[v].filterEnv = voices_[v].getFilterEnv().level;

        voices_[v].getChaosTelemetry(telemetry_[v].rawChaos, 19);

    }

}



void NyarlatProcessor::getStateInformation(juce::MemoryBlock& destData) {

    auto state = apvts_.copyState();

    std::unique_ptr<juce::XmlElement> xml(state.createXml());

    const float s = storedEditorScale_.load(std::memory_order_relaxed);

    if (s > 0.0f) xml->setAttribute("editorScale", static_cast<double>(s));

    copyXmlToBinary(*xml, destData);

}



void NyarlatProcessor::setStateInformation(const void* data, int sizeInBytes) {

    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName(apvts_.state.getType())) {

        if (xmlState->hasAttribute("editorScale")) {

            storedEditorScale_.store(static_cast<float>(xmlState->getDoubleAttribute("editorScale", 0.0)), std::memory_order_relaxed);

        }

        apvts_.replaceState(juce::ValueTree::fromXml(*xmlState));

    }

}



juce::AudioProcessorEditor* NyarlatProcessor::createEditor() {

    return new NyarlatEditor(*this);

}



} // namespace Nyarlat



juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {

    return new Nyarlat::NyarlatProcessor();

}
