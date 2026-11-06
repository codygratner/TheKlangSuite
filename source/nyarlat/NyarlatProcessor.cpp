#include "NyarlatProcessor.h"
#include "NyarlatEditor.h"

namespace Nyarlat {

NyarlatProcessor::NyarlatProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_(*this, nullptr, "NyarlatParams", nyarlatParameters::createLayout())
{
    AkwfRom::getInstance().load();

    for (int i = 0; i < kNumVoices; ++i) {
        voices_[i].setVoiceIndex(i);
    }
}

void NyarlatProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    juce::ignoreUnused(samplesPerBlock);

    TbdAudio::FastMath::enableFTZDAZ();

    for (auto& v : voices_) {
        v.prepare(sampleRate);
    }
    reverb_.prepare(sampleRate);

    // Alpha Chorus power-of-two delay line (2048 samples, ~46ms)
    chorusBufferL_.fill(0.0f);
    chorusBufferR_.fill(0.0f);
    chorusPtr_ = 0;
    chorusLfo_.setFrequency(0.25f, static_cast<float>(sampleRate));
    chorusLfo_.reset();

    bassBpL_.set(40.0f, 0.7f, static_cast<float>(sampleRate));
    bassBpR_.set(40.0f, 0.7f, static_cast<float>(sampleRate));
    masterDcBlockerL_.prepare(sampleRate, 5.0);
    masterDcBlockerR_.prepare(sampleRate, 5.0);
    postDcBlockerL_.prepare(sampleRate, 5.0);
    postDcBlockerR_.prepare(sampleRate, 5.0);
    trebleStateL_ = trebleStateR_ = 0.0f;

    for (int i = 0; i < 3; ++i) {
        lastSharedBank_[i] = 0xFFFFFFFFu;
        lastSharedMorph_[i] = -1.0f;
    }
}

void NyarlatProcessor::releaseResources() {
    for (auto& v : voices_) v.reset();
    reverb_.reset();
    bassBpL_.reset();
    bassBpR_.reset();
    masterDcBlockerL_.reset();
    masterDcBlockerR_.reset();
    postDcBlockerL_.reset();
    postDcBlockerR_.reset();
    chorusLfo_.reset();
    chorusPtr_ = 0;
}

void NyarlatProcessor::handleMidiEvent(const juce::MidiMessage& msg) noexcept {
    if (msg.isNoteOn()) {
        allocateVoice(msg.getNoteNumber(), msg.getFloatVelocity());
    } else if (msg.isNoteOff()) {
        releaseVoice(msg.getNoteNumber());
    } else if (msg.isAllNotesOff() || msg.isAllSoundOff()) {
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
    int bestIdx = -1;
    for (int i = 0; i < kNumVoices; ++i) {
        int idx = (nextVoiceIndex_ + i) % kNumVoices;
        if (!voices_[idx].isActive()) {
            bestIdx = idx;
            break;
        }
    }
    if (bestIdx == -1) {
        bestIdx = nextVoiceIndex_;
    }
    nextVoiceIndex_ = (bestIdx + 1) % kNumVoices;
    newestVoice_.store(bestIdx, std::memory_order_relaxed);

    float glide = *apvts_.getRawParameterValue(nyarlatParameters::Ids::glide_time);
    voices_[bestIdx].noteOn(midiNote, velocity, glide);
}

void NyarlatProcessor::releaseVoice(int midiNote) noexcept {
    for (auto& v : voices_) {
        if (v.isActive() && v.getMidiNote() == midiNote) {
            v.noteOff();
        }
    }
}

void NyarlatProcessor::renderSegment(float* left, float* right, int numSamples, const VoiceParams& params, int activeVoiceCount) noexcept {
    for (int v = 0; v < kNumVoices; ++v) {
        if (!voices_[v].isActive()) continue;
        voices_[v].renderBlockAdd(
            left, right, numSamples, activeVoiceCount,
            params.oscBanks, params.oscWaves, params.oscMasks, params.oscDetunes, params.oscOctaves,
            params.oscOctLocks, params.oscSyncs, params.oscSpeeds, params.oscLoops,
            params.osc1Lvl, params.osc2Lvl, params.osc3Lvl,
            params.noiseLvl, params.noiseTone, params.oscSpread, params.mixerSeethe,
            params.chaosRate, params.chaosDrift, params.chaosPoly, params.randAttenuverters,
            params.zModel, params.zFreq, params.zMorph, params.zTransform,
            params.fEnvDepth, params.fVel,
            params.ampVel, params.globalBendSemi, globalModWheel_, globalChannelAftertouch_
        );
    }
}

void NyarlatProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    TbdAudio::FastMath::enableFTZDAZ();

    const int numSamples = buffer.getNumSamples();
    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);

    namespace Ids = nyarlatParameters::Ids;

    float oscBanks[3] = {
        *apvts_.getRawParameterValue(Ids::osc1_bank),
        *apvts_.getRawParameterValue(Ids::osc2_bank),
        *apvts_.getRawParameterValue(Ids::osc3_bank)
    };
    float oscWaves[3] = {
        *apvts_.getRawParameterValue(Ids::osc1_wave),
        *apvts_.getRawParameterValue(Ids::osc2_wave),
        *apvts_.getRawParameterValue(Ids::osc3_wave)
    };
    int oscMasks[3] = {
        static_cast<int>(*apvts_.getRawParameterValue(Ids::osc1_mask)),
        static_cast<int>(*apvts_.getRawParameterValue(Ids::osc2_mask)),
        static_cast<int>(*apvts_.getRawParameterValue(Ids::osc3_mask))
    };
    float oscDetunes[3] = {
        *apvts_.getRawParameterValue(Ids::osc1_detune),
        *apvts_.getRawParameterValue(Ids::osc2_detune),
        *apvts_.getRawParameterValue(Ids::osc3_detune)
    };
    int oscOctaves[3] = {
        static_cast<int>(*apvts_.getRawParameterValue(Ids::osc1_octave)),
        static_cast<int>(*apvts_.getRawParameterValue(Ids::osc2_octave)),
        static_cast<int>(*apvts_.getRawParameterValue(Ids::osc3_octave))
    };
    bool oscOctLocks[3] = {
        *apvts_.getRawParameterValue(Ids::osc1_oct_lock) > 0.5f,
        *apvts_.getRawParameterValue(Ids::osc2_oct_lock) > 0.5f,
        *apvts_.getRawParameterValue(Ids::osc3_oct_lock) > 0.5f
    };
    bool oscSyncs[3] = {
        false,
        *apvts_.getRawParameterValue(Ids::osc2_sync) > 0.5f,
        *apvts_.getRawParameterValue(Ids::osc3_sync) > 0.5f
    };
    float oscSpeeds[3] = {
        *apvts_.getRawParameterValue(Ids::osc1_speed),
        *apvts_.getRawParameterValue(Ids::osc2_speed),
        *apvts_.getRawParameterValue(Ids::osc3_speed)
    };
    int oscLoops[3] = {
        static_cast<int>(*apvts_.getRawParameterValue(Ids::osc1_loop)),
        static_cast<int>(*apvts_.getRawParameterValue(Ids::osc2_loop)),
        static_cast<int>(*apvts_.getRawParameterValue(Ids::osc3_loop))
    };

    float osc1Lvl = *apvts_.getRawParameterValue(Ids::osc1_level);
    float osc2Lvl = *apvts_.getRawParameterValue(Ids::osc2_level);
    float osc3Lvl = *apvts_.getRawParameterValue(Ids::osc3_level);
    float noiseLvl = *apvts_.getRawParameterValue(Ids::noise_level);
    float noiseTone = *apvts_.getRawParameterValue(Ids::noise_tone);
    float oscSpread = *apvts_.getRawParameterValue(Ids::osc_spread);
    float mixerSeethe = *apvts_.getRawParameterValue(Ids::mixer_seethe);

    float chaosRate = *apvts_.getRawParameterValue(Ids::chaos_rate);
    float chaosDrift = *apvts_.getRawParameterValue(Ids::chaos_drift);
    float chaosPoly = *apvts_.getRawParameterValue(Ids::chaos_poly_spread);

    float randAttenuverters[15] = {
        *apvts_.getRawParameterValue(Ids::rand_o1_wave),
        *apvts_.getRawParameterValue(Ids::rand_o1_mask),
        *apvts_.getRawParameterValue(Ids::rand_o1_speed),
        *apvts_.getRawParameterValue(Ids::rand_o2_wave),
        *apvts_.getRawParameterValue(Ids::rand_o2_mask),
        *apvts_.getRawParameterValue(Ids::rand_o2_speed),
        *apvts_.getRawParameterValue(Ids::rand_o3_wave),
        *apvts_.getRawParameterValue(Ids::rand_o3_mask),
        *apvts_.getRawParameterValue(Ids::rand_o3_speed),
        *apvts_.getRawParameterValue(Ids::rand_z_freq),
        *apvts_.getRawParameterValue(Ids::rand_z_morph),
        *apvts_.getRawParameterValue(Ids::rand_z_transform),
        *apvts_.getRawParameterValue(Ids::rand_seethe),
        *apvts_.getRawParameterValue(Ids::rand_amp_decay),
        *apvts_.getRawParameterValue(Ids::rand_pan)
    };

    int zModel = static_cast<int>(*apvts_.getRawParameterValue(Ids::z_model));
    float zFreq = *apvts_.getRawParameterValue(Ids::z_freq);
    float zMorph = *apvts_.getRawParameterValue(Ids::z_morph);
    float zTransform = *apvts_.getRawParameterValue(Ids::z_transform);
    float fEnvDepth = *apvts_.getRawParameterValue(Ids::filter_env_depth);
    float fVel = *apvts_.getRawParameterValue(Ids::filter_velocity);

    float ampVel = *apvts_.getRawParameterValue(Ids::amp_velocity);
    float pbRange = *apvts_.getRawParameterValue(Ids::pitch_bend_range);
    float globalBendSemi = globalPitchWheel_ * pbRange;

    // Update Envelopes
    double sr = getSampleRate();
    float ampA = *apvts_.getRawParameterValue(Ids::amp_attack);
    float ampD = *apvts_.getRawParameterValue(Ids::amp_decay);
    float ampS = *apvts_.getRawParameterValue(Ids::amp_sustain);
    float ampR = *apvts_.getRawParameterValue(Ids::amp_release);
    float ktRate = *apvts_.getRawParameterValue(Ids::keytrack_rate);

    float filtA = *apvts_.getRawParameterValue(Ids::filter_attack);
    float filtD = *apvts_.getRawParameterValue(Ids::filter_decay);
    float filtS = *apvts_.getRawParameterValue(Ids::filter_sustain);
    float filtR = *apvts_.getRawParameterValue(Ids::filter_release);

    uint32_t uBanks[3] = { static_cast<uint32_t>(oscBanks[0]), static_cast<uint32_t>(oscBanks[1]), static_cast<uint32_t>(oscBanks[2]) };

    // Processor-level shared wavetable cache: 1 interpolation per osc instead of 16
    for (int i = 0; i < 3; ++i) {
        if (uBanks[i] != lastSharedBank_[i] || std::fabs(oscWaves[i] - lastSharedMorph_[i]) >= (1.0f / 4096.0f)) {
            lastSharedBank_[i] = uBanks[i];
            lastSharedMorph_[i] = oscWaves[i];
            AkwfRom::getInstance().interpolateWaveform(uBanks[i], oscWaves[i], sharedWavetables_[i].data());
        }
    }

    int activeVoiceCount = 0;
    for (int v = 0; v < kNumVoices; ++v) {
        if (!voices_[v].isActive()) continue;
        activeVoiceCount++;
        voices_[v].sanitize();

        float noteScale = 1.0f - ktRate * (static_cast<float>(voices_[v].getMidiNote() - 60) / 60.0f);
        noteScale = juce::jlimit(0.2f, 1.8f, noteScale);

        voices_[v].getAmpEnv().setADSR(ampA, ampD, ampS, ampR, sr, noteScale);
        voices_[v].getFilterEnv().setADSR(filtA, filtD, filtS, filtR, sr, noteScale);
        voices_[v].advanceChaos(chaosRate, chaosDrift, numSamples / sr);
        voices_[v].updateWavetables(uBanks, oscWaves, randAttenuverters, &sharedWavetables_);
    }

    buffer.clear();

    VoiceParams params;
    for (int i = 0; i < 3; ++i) {
        params.oscBanks[i] = oscBanks[i];
        params.oscWaves[i] = oscWaves[i];
        params.oscMasks[i] = oscMasks[i];
        params.oscDetunes[i] = oscDetunes[i];
        params.oscOctaves[i] = oscOctaves[i];
        params.oscOctLocks[i] = oscOctLocks[i];
        params.oscSyncs[i] = oscSyncs[i];
        params.oscSpeeds[i] = oscSpeeds[i];
        params.oscLoops[i] = oscLoops[i];
    }
    params.osc1Lvl = osc1Lvl; params.osc2Lvl = osc2Lvl; params.osc3Lvl = osc3Lvl;
    params.noiseLvl = noiseLvl; params.noiseTone = noiseTone;
    params.oscSpread = oscSpread; params.mixerSeethe = mixerSeethe;
    params.chaosRate = chaosRate; params.chaosDrift = chaosDrift; params.chaosPoly = chaosPoly;
    std::memcpy(params.randAttenuverters, randAttenuverters, 15 * sizeof(float));
    params.zModel = zModel; params.zFreq = zFreq; params.zMorph = zMorph; params.zTransform = zTransform;
    params.fEnvDepth = fEnvDepth; params.fVel = fVel;
    params.ampVel = ampVel; params.globalBendSemi = globalBendSemi;

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

    float bassBoost = *apvts_.getRawParameterValue(Ids::bass_boost);
    float trebleTame = *apvts_.getRawParameterValue(Ids::treble_tame);
    float revTime = *apvts_.getRawParameterValue(Ids::reverb_time);
    float revDamp = *apvts_.getRawParameterValue(Ids::reverb_damp);
    float revMix = *apvts_.getRawParameterValue(Ids::reverb_mix);
    float alphaChorus = *apvts_.getRawParameterValue(Ids::alpha_chorus);
    float width = *apvts_.getRawParameterValue(Ids::global_width);
    float masterVol = *apvts_.getRawParameterValue(Ids::output_level);

    for (int i = 0; i < numSamples; ++i) {
        float l = leftChannel[i];
        float r = rightChannel[i];

        if (bassBoost > 0.01f) {
            l += 1.5f * bassBoost * bassBpL_.process(l);
            r += 1.5f * bassBoost * bassBpR_.process(r);
        }

        if (trebleTame > 0.01f) {
            float coeff = 0.2f * trebleTame;
            trebleStateL_ = (1.0f - coeff) * trebleStateL_ + coeff * l;
            trebleStateR_ = (1.0f - coeff) * trebleStateR_ + coeff * r;
            l = trebleStateL_;
            r = trebleStateR_;
        }

        if (alphaChorus > 0.01f) {
            chorusBufferL_[chorusPtr_] = l;
            chorusBufferR_[chorusPtr_] = r;

            float modOffsetL = (chorusLfo_.sinVal + 1.0f) * 0.4f * static_cast<float>(kChorusSize);
            float modOffsetR = (chorusLfo_.cosVal + 1.0f) * 0.4f * static_cast<float>(kChorusSize);
            chorusLfo_.step();

            int readL = (chorusPtr_ - static_cast<int>(modOffsetL)) & kChorusMask;
            int readR = (chorusPtr_ - static_cast<int>(modOffsetR)) & kChorusMask;

            l += chorusBufferL_[readL] * alphaChorus * 0.6f;
            r += chorusBufferR_[readR] * alphaChorus * 0.6f;

            chorusPtr_ = (chorusPtr_ + 1) & kChorusMask;
        }

        reverb_.processStereo(l, r, revTime, revDamp, revMix);

        float mid = (l + r) * 0.5f;
        float side = (r - l) * 0.5f;
        side *= (1.0f + width);
        l = mid - side;
        r = mid + side;

        // 1. Pre-saturation DC centering (keep drive symmetric)
        l = masterDcBlockerL_.process(l);
        r = masterDcBlockerR_.process(r);

        // 2. Master saturation (branchless Padé fastTanh)
        l = TbdAudio::FastMath::fastTanh(l * masterVol);
        r = TbdAudio::FastMath::fastTanh(r * masterVol);

        // 3. Pre-sanitization before post-blocker
        if (!std::isfinite(l)) { l = 0.0f; postDcBlockerL_.reset(); }
        if (!std::isfinite(r)) { r = 0.0f; postDcBlockerR_.reset(); }

        // 4. Post-saturation DC blocker (nullifies saturation DC bias)
        l = postDcBlockerL_.process(l);
        r = postDcBlockerR_.process(r);

        // 5. Host 0 dBFS ceiling clamp (prevents differentiator step overshoot)
        leftChannel[i] = juce::jlimit(-1.0f, 1.0f, l);
        rightChannel[i] = juce::jlimit(-1.0f, 1.0f, r);
    }

    int nv = newestVoice_.load(std::memory_order_relaxed);
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

    for (int v = 0; v < kNumVoices; ++v) {
        telemetry_[v].active = voices_[v].isActive();
        telemetry_[v].note = voices_[v].getMidiNote();
        telemetry_[v].x = std::log10(zFreq / 20.0f) / 3.0f;
        telemetry_[v].y = zMorph;
        telemetry_[v].z = zTransform;
        for (int i = 0; i < 3; ++i) {
            telemetry_[v].statorOffsets[i] = voices_[v].getStatorOffset(i);
            telemetry_[v].oscEffWave[i] = voices_[v].getEffWave(i);
            telemetry_[v].oscEffMask[i] = voices_[v].getEffMask(i);
            telemetry_[v].oscEffSpeed[i] = voices_[v].getEffSpeed(i);
        }
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
