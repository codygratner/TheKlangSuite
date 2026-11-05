#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include <cmath>
#include <array>

namespace rlyeh::ui {

/**
 * @brief High-resilience, zero-static scale persistence engine for The Klang Suite editors.
 * Features debounced trailing-edge file writes, advisory IPC locking, atomic temporary file
 * replacement, corrupt file quarantine, and priority fallback (Session XML > Global JSON > Default).
 */
class EditorScaleManager final {
public:
    static constexpr float kMinScale     = 0.70f;
    static constexpr float kMaxScale     = 2.00f;
    static constexpr float kDefaultScale = 1.00f;
    static constexpr int   kDebounceMs   = 750;

    static constexpr std::array<float, 7> kPresetScales = { 0.75f, 0.90f, 1.00f, 1.25f, 1.50f, 1.75f, 2.00f };

    //==============================================================================
    /**
     * @brief Debounced asynchronous disk writer owned strictly by each Editor instance.
     * Guaranteed zero steady-state heap allocations during mouse drag resizes.
     */
    class Writer final : private juce::Timer {
    public:
        Writer() = default;
        ~Writer() override {
            JUCE_ASSERT_MESSAGE_THREAD
            stopTimer();
            flush();
        }

        Writer(const Writer&) = delete;
        Writer& operator=(const Writer&) = delete;
        Writer(Writer&&) = delete;
        Writer& operator=(Writer&&) = delete;

        /**
         * @brief Enqueues a scale update for a given plugin slug.
         */
        void note(const juce::String& slug, float scale) {
            JUCE_ASSERT_MESSAGE_THREAD
            if (!std::isfinite(scale) || scale <= 0.0f) return;

            const float quantized = std::round(juce::jlimit(kMinScale, kMaxScale, scale) * 100.0f) / 100.0f;
            if (lastWrittenSlug_ == slug && std::abs(lastWrittenScale_ - quantized) < 0.005f) {
                return;
            }

            pendingSlug_ = slug;
            pendingScale_ = quantized;
            isDirty_ = true;
            startTimer(kDebounceMs);
        }

        /**
         * @brief Flushes any pending scale write to disk using atomic rename and advisory IPC lock.
         */
        bool flush() {
            JUCE_ASSERT_MESSAGE_THREAD
            if (!isDirty_ || pendingSlug_.isEmpty()) return true;

            const auto prefsFile = getPrefsFile();
            const auto prefsDir = prefsFile.getParentDirectory();
            if (!prefsDir.exists() && !prefsDir.createDirectory()) {
                return false;
            }

            // Advisory cross-process lock (proceed after 50ms even if sandbox restricts mutexes)
            juce::InterProcessLock ipcLock("RlyehSound_TheKlangSuite_EditorSizes_Lock");
            const bool lockAcquired = ipcLock.enter(50);

            juce::var root(new juce::DynamicObject());
            if (prefsFile.existsAsFile()) {
                auto parsed = juce::JSON::parse(prefsFile);
                if (parsed.isObject()) {
                    root = parsed;
                } else if (prefsFile.getSize() > 0) {
                    quarantineCorruptFile(prefsFile);
                }
            }

            auto* obj = root.getDynamicObject();
            if (obj != nullptr) {
                obj->setProperty(pendingSlug_, pendingScale_);
            }

            juce::TemporaryFile tempFile(prefsFile);
            bool success = false;
            const juce::String jsonString = juce::JSON::toString(root, true);

            if (tempFile.getFile().replaceWithText(jsonString, false, false, "\n")
                && tempFile.overwriteTargetFileWithTemporary()) {
                success = true;
                lastWrittenSlug_ = pendingSlug_;
                lastWrittenScale_ = pendingScale_;
                isDirty_ = false;
                failCount_ = 0;
            } else {
                ++failCount_;
            }

            if (lockAcquired) {
                ipcLock.exit();
            }

            return success;
        }

    private:
        void timerCallback() override {
            stopTimer();
            if (!flush() && failCount_ < 4) {
                startTimer(1500 * failCount_);
            }
        }

        static void quarantineCorruptFile(const juce::File& file) {
            const auto corruptFile = file.getSiblingFile(file.getFileNameWithoutExtension() + "_corrupt_" 
                + juce::Time::getCurrentTime().formatted("%Y%m%d_%H%M%S") + ".json");
            file.moveFileTo(corruptFile);
        }

        juce::String pendingSlug_;
        float pendingScale_{ 1.0f };
        juce::String lastWrittenSlug_;
        float lastWrittenScale_{ 1.0f };
        bool isDirty_{ false };
        int failCount_{ 0 };
    };

    //==============================================================================
    /**
     * @brief Resolves user preferred scale with 3-tier fallback:
     * 1. Session XML scale (project recall)
     * 2. Global JSON preference (seeds new instances)
     * 3. Factory default (1.00f)
     */
    static float getPreferredScale(const juce::String& slug, 
                                   float sessionScale, 
                                   float minS = kMinScale, 
                                   float maxS = kMaxScale) {
        // Priority 1: Session XML scale (project recall)
        if (std::isfinite(sessionScale) && sessionScale >= minS && sessionScale <= maxS) {
            return sessionScale;
        }

        // Priority 2: Global JSON file (seeds new instances)
        const auto prefsFile = getPrefsFile();
        if (prefsFile.existsAsFile()) {
            auto parsed = juce::JSON::parse(prefsFile);
            if (auto* obj = parsed.getDynamicObject()) {
                auto val = obj->getProperty(slug);
                if (val.isDouble() || val.isInt()) {
                    const float loaded = static_cast<float>(val);
                    if (std::isfinite(loaded) && loaded >= minS && loaded <= maxS) {
                        return loaded;
                    }
                }
            }
        }

        // Priority 3: Factory Default
        return kDefaultScale;
    }

    /**
     * @brief Calculates safe maximum scale factor that completely fits on current display.
     */
    static float getDisplayFitScale(juce::Component& comp, int baseW, int baseH) {
        auto* display = juce::Desktop::getInstance().getDisplays().getDisplayForRect(comp.getScreenBounds());
        if (display == nullptr) {
            display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
        }
        if (display == nullptr) return 1.0f;

        const auto userArea = display->userArea;
        // Reserve 48px margin for taskbars/DAW window headers
        const float maxW = static_cast<float>(userArea.getWidth() - 48);
        const float maxH = static_cast<float>(userArea.getHeight() - 64);

        if (maxW <= 0.0f || maxH <= 0.0f) return 1.0f;

        const float fitW = maxW / static_cast<float>(baseW);
        const float fitH = maxH / static_cast<float>(baseH);
        return juce::jlimit(kMinScale, kMaxScale, juce::jmin(fitW, fitH));
    }

    static juce::File getPrefsFile() {
        return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("RlyehSound")
            .getChildFile("TheKlangSuite")
            .getChildFile("editor_sizes.json");
    }
};

} // namespace rlyeh::ui
