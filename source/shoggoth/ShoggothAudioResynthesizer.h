#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include "shoggoth_layout.h"

namespace Shoggoth {

struct ResynthesisResult {
    float s1Harmonics[63]{};
    float s2Harmonics[63]{};
    float extendedHarmonics[kMaxPartials]{};
    int extractedCount{63};
    bool isStereoPair{false};
    bool valid{false};
};

class AudioResynthesizer {
public:
    static constexpr int kFftOrder = 11;
    static constexpr int kFftSize  = 1 << kFftOrder; // 2048 samples

    // ==============================================================================
    // Resynthesize Audio File (WAV, AIFF, FLAC, Vitaltable)
    // ==============================================================================
    static ResynthesisResult resynthesizeAudioFile(const juce::File& file) {
        ResynthesisResult result;
        if (!file.existsAsFile()) return result;

        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();

        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
        if (reader == nullptr) return result;

        const auto numSamples = static_cast<int>(reader->lengthInSamples);
        if (numSamples < 32) return result;

        juce::AudioBuffer<float> buffer(static_cast<int>(reader->numChannels), std::min(numSamples, 131072));
        reader->read(&buffer, 0, buffer.getNumSamples(), 0, true, true);

        // Frame 0 Analysis -> Source 1
        analyzeFrame(buffer, 0, result.s1Harmonics, result.extendedHarmonics, result.extractedCount);

        // Frame N Analysis -> Source 2 (if multi-frame or longer sample)
        if (buffer.getNumSamples() >= kFftSize * 2) {
            const int frame2Offset = buffer.getNumSamples() - kFftSize;
            float dummyExt[kMaxPartials]{};
            int dummyCount = 63;
            analyzeFrame(buffer, frame2Offset, result.s2Harmonics, dummyExt, dummyCount);
            result.isStereoPair = true;
        } else {
            // Mirror S1 to S2 with subtle harmonic tilt
            for (int k = 0; k < 63; ++k) {
                result.s2Harmonics[k] = result.s1Harmonics[k] * std::pow(0.95f, static_cast<float>(k));
            }
        }

        result.valid = true;
        return result;
    }

private:
    static void analyzeFrame(const juce::AudioBuffer<float>& buffer, int startSample,
                             float* out63, float* outExt, int& outCount) {
        alignas(64) float fftData[kFftSize * 2]{};
        const float* readPtr = buffer.getReadPointer(0);
        const int samplesToCopy = std::min(kFftSize, buffer.getNumSamples() - startSample);

        // Windowed copy into FFT buffer
        for (int i = 0; i < samplesToCopy; ++i) {
            // Hann window
            const float w = 0.5f * (1.0f - std::cos(Oscillator::kTwoPi * static_cast<float>(i) / static_cast<float>(kFftSize)));
            fftData[i] = readPtr[startSample + i] * w;
        }

        // Forward FFT
        juce::dsp::FFT fft(kFftOrder);
        fft.performRealOnlyForwardTransform(fftData);

        // Extract spectral magnitudes
        float maxMag = 1e-6f;
        alignas(64) float magnitudes[kFftSize / 2]{};
        for (int i = 0; i < kFftSize / 2; ++i) {
            const float re = fftData[i * 2];
            const float im = fftData[i * 2 + 1];
            const float mag = std::sqrt(re * re + im * im);
            magnitudes[i] = mag;
            if (mag > maxMag) maxMag = mag;
        }

        // Detect fundamental bin
        int f0Bin = 1;
        float peakSearch = 0.0f;
        for (int i = 1; i < std::min(64, kFftSize / 2); ++i) {
            if (magnitudes[i] > peakSearch) {
                peakSearch = magnitudes[i];
                f0Bin = i;
            }
        }

        // Map harmonic peaks
        const float invMax = 99.0f / maxMag;
        outCount = std::min(kMaxPartials, (kFftSize / 2) / std::max(1, f0Bin));

        for (int k = 1; k <= 63; ++k) {
            const int targetBin = k * f0Bin;
            if (targetBin < kFftSize / 2) {
                out63[k - 1] = std::clamp(magnitudes[targetBin] * invMax, 0.0f, 99.0f);
            } else {
                out63[k - 1] = 0.0f;
            }
        }

        // Fill extended array for Unlimited mode
        for (int k = 1; k <= outCount; ++k) {
            const int targetBin = k * f0Bin;
            if (targetBin < kFftSize / 2) {
                outExt[k - 1] = magnitudes[targetBin] / maxMag;
            } else {
                outExt[k - 1] = 0.0f;
            }
        }
    }
};

} // namespace Shoggoth
