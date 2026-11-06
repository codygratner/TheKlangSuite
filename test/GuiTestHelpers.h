#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_events/juce_events.h>
#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include <memory>
#include <vector>
#include <algorithm>
#include "UIComponents.h"
#include "support/HeadlessX11.h"

namespace GuiTestHelpers {

    // ==============================================================================
    // 1. Dual-Timeout Watchdog
    // ==============================================================================
    struct Watchdog {
        static inline std::atomic<bool> isRunning{ false };
        static inline std::atomic<uint64_t> lastHeartbeatMs{ 0 };
        static inline std::atomic<uint64_t> suiteStartMs{ 0 };
        static inline std::atomic<bool> testFailedDueToTimeout{ false };
        static inline std::unique_ptr<std::jthread> workerThread;

        static void start(int globalTimeoutMinutes = 5, int stepTimeoutSeconds = 30) {
            if (isRunning.load()) return;
            isRunning.store(true);
            testFailedDueToTimeout.store(false);
            auto now = static_cast<uint64_t>(juce::Time::currentTimeMillis());
            suiteStartMs.store(now);
            lastHeartbeatMs.store(now);

            workerThread = std::make_unique<std::jthread>([globalTimeoutMinutes, stepTimeoutSeconds](std::stop_token stopToken) {
                const uint64_t globalLimitMs = static_cast<uint64_t>(globalTimeoutMinutes) * 60ULL * 1000ULL;
                const uint64_t stepLimitMs = static_cast<uint64_t>(stepTimeoutSeconds) * 1000ULL;

                while (!stopToken.stop_requested() && isRunning.load()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    if (stopToken.stop_requested() || !isRunning.load()) break;

                    auto nowMs = static_cast<uint64_t>(juce::Time::currentTimeMillis());

                    // 1. Global process limit
                    if (nowMs - suiteStartMs.load() > globalLimitMs) {
                        std::cerr << "\n🚨 [FATAL GLOBAL TIMEOUT: " << globalTimeoutMinutes << "m exceeded! Aborting test process.]\n" << std::endl;
                        std::cerr.flush();
                        std::exit(1);
                    }

                    // 2. Local step heartbeat limit
                    if (nowMs - lastHeartbeatMs.load() > stepLimitMs) {
                        std::cerr << "\n⚠️ [LOCAL STEP TIMEOUT: " << stepTimeoutSeconds << "s exceeded without heartbeat! Unblocking message loop.]\n" << std::endl;
                        std::cerr.flush();
                        testFailedDueToTimeout.store(true);
                        lastHeartbeatMs.store(nowMs);
                        if (auto* mm = juce::MessageManager::getInstanceWithoutCreating()) {
                            mm->stopDispatchLoop();
                        }
                    }
                }
            });
        }

        static void stop() {
            if (!isRunning.load()) return;
            isRunning.store(false);
            if (workerThread && workerThread->joinable()) {
                workerThread->request_stop();
                workerThread->join();
            }
            workerThread.reset();
        }

        static void pingHeartbeat() {
            lastHeartbeatMs.store(static_cast<uint64_t>(juce::Time::currentTimeMillis()));
        }
    };

    // ==============================================================================
    // 2. Headless GUI Environment Setup
    // ==============================================================================
    struct ScopedGuiContext {
        // Declared first -> constructed before guiInitialiser opens any display
        struct X11First {
            X11First() { HeadlessX11::install(); }
        } x11First;

        juce::ScopedJuceInitialiser_GUI guiInitialiser;

        ~ScopedGuiContext() {
            if (auto n = HeadlessX11::swallowedErrorCount()) {
                std::fprintf(stderr, "\n  ⚠️ [HeadlessX11] Caught & swallowed %u non-fatal X errors (last code=%u req=%u)\n\n",
                             n, HeadlessX11::lastErrorCode(), HeadlessX11::lastRequestCode());
            }
        }
    };

    // ==============================================================================
    // 3. Micro Message Pump
    // ==============================================================================
    inline void pumpMessageLoop(int maxIterations = 20, int timeoutMs = 10) {
        for (int i = 0; i < maxIterations; ++i) {
            Watchdog::pingHeartbeat();
            juce::Timer::callAfterDelay(timeoutMs, []{ 
                if (auto* mm = juce::MessageManager::getInstanceWithoutCreating()) 
                    mm->stopDispatchLoop(); 
            });
            juce::MessageManager::getInstance()->runDispatchLoop();
        }
    }

    // ==============================================================================
    // 4. Component Finder
    // ==============================================================================
    struct ComponentFinder {
        
        template <typename T>
        static T* findByType(juce::Component* parent) {
            if (!parent) return nullptr;
            if (auto* typed = dynamic_cast<T*>(parent))
                return typed;

            for (auto* child : parent->getChildren()) {
                if (auto* found = findByType<T>(child))
                    return found;
            }
            return nullptr;
        }

        template <typename T>
        static juce::Array<T*> findAllByType(juce::Component* parent) {
            juce::Array<T*> results;
            if (!parent) return results;

            if (auto* typed = dynamic_cast<T*>(parent))
                results.add(typed);

            for (auto* child : parent->getChildren()) {
                results.addArray(findAllByType<T>(child));
            }
            return results;
        }

        static juce::Component* findByName(juce::Component* parent, const juce::String& name) {
            if (!parent) return nullptr;
            if (parent->getName() == name)
                return parent;

            for (auto* child : parent->getChildren()) {
                if (auto* found = findByName(child, name))
                    return found;
            }
            return nullptr;
        }

        static RotaryKnobSlider* findSliderByParamId(juce::Component* parent, const juce::String& paramId) {
            if (!parent) return nullptr;
            
            if (auto* slider = dynamic_cast<RotaryKnobSlider*>(parent)) {
                if (slider->getParamId() == paramId)
                    return slider;
            }

            for (auto* child : parent->getChildren()) {
                if (auto* found = findSliderByParamId(child, paramId))
                    return found;
            }
            return nullptr;
        }
        
        static LedSelectorComponent* findSelectorByParamId(juce::Component* parent, const juce::String& paramId) {
            if (!parent) return nullptr;
            
            if (auto* selector = dynamic_cast<LedSelectorComponent*>(parent)) {
                if (selector->getParamId() == paramId)
                    return selector;
            }

            for (auto* child : parent->getChildren()) {
                if (auto* found = findSelectorByParamId(child, paramId))
                    return found;
            }
            return nullptr;
        }
    };

    // ==============================================================================
    // 5. Event Simulator
    // ==============================================================================
    struct EventSimulator {
        
        static void simulateClick(juce::Component* comp) {
            if (!comp) return;
            juce::Component::SafePointer<juce::Component> safeComp(comp);
            auto* btn = dynamic_cast<juce::Button*>(comp);

            juce::MouseEvent e(juce::Desktop::getInstance().getMainMouseSource(), comp->getLocalBounds().getCentre().toFloat(), juce::ModifierKeys::leftButtonModifier, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comp, comp, juce::Time::getCurrentTime(), comp->getLocalBounds().getCentre().toFloat(), juce::Time::getCurrentTime(), 1, false);

            comp->mouseDown(e);
            if (safeComp == nullptr) return;

            juce::MouseEvent eUp(juce::Desktop::getInstance().getMainMouseSource(), comp->getLocalBounds().getCentre().toFloat(), juce::ModifierKeys(), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comp, comp, juce::Time::getCurrentTime(), comp->getLocalBounds().getCentre().toFloat(), juce::Time::getCurrentTime(), 1, false);
            comp->mouseUp(eUp);

            if (safeComp != nullptr && btn != nullptr) {
                if (btn->getClickingTogglesState()) {
                    btn->setToggleState(!btn->getToggleState(), juce::sendNotificationSync);
                }
                if (btn->onClick) {
                    btn->onClick();
                } else {
                    btn->triggerClick();
                }
            }
        }
        
        static void simulateRightClick(juce::Component* comp) {
            if (!comp) return;
            juce::Component::SafePointer<juce::Component> safeComp(comp);
            juce::MouseEvent e(juce::Desktop::getInstance().getMainMouseSource(), comp->getLocalBounds().getCentre().toFloat(), juce::ModifierKeys::rightButtonModifier, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comp, comp, juce::Time::getCurrentTime(), comp->getLocalBounds().getCentre().toFloat(), juce::Time::getCurrentTime(), 1, false);

            comp->mouseDown(e);
            if (safeComp == nullptr) return;

            juce::MouseEvent eUp(juce::Desktop::getInstance().getMainMouseSource(), comp->getLocalBounds().getCentre().toFloat(), juce::ModifierKeys(), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comp, comp, juce::Time::getCurrentTime(), comp->getLocalBounds().getCentre().toFloat(), juce::Time::getCurrentTime(), 1, false);
            comp->mouseUp(eUp);
        }

        static void simulateDoubleClick(juce::Component* comp) {
            if (!comp) return;
            juce::MouseEvent e(juce::Desktop::getInstance().getMainMouseSource(), comp->getLocalBounds().getCentre().toFloat(), juce::ModifierKeys::leftButtonModifier, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comp, comp, juce::Time::getCurrentTime(), comp->getLocalBounds().getCentre().toFloat(), juce::Time::getCurrentTime(), 2, false);

            comp->mouseDoubleClick(e);
        }

        static void simulateAltClick(juce::Component* comp) {
            if (!comp) return;
            juce::Component::SafePointer<juce::Component> safeComp(comp);
            juce::MouseEvent e(juce::Desktop::getInstance().getMainMouseSource(), comp->getLocalBounds().getCentre().toFloat(), juce::ModifierKeys::altModifier, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comp, comp, juce::Time::getCurrentTime(), comp->getLocalBounds().getCentre().toFloat(), juce::Time::getCurrentTime(), 1, false);

            comp->mouseDown(e);
            if (safeComp == nullptr) return;

            juce::MouseEvent eUp(juce::Desktop::getInstance().getMainMouseSource(), comp->getLocalBounds().getCentre().toFloat(), juce::ModifierKeys(), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comp, comp, juce::Time::getCurrentTime(), comp->getLocalBounds().getCentre().toFloat(), juce::Time::getCurrentTime(), 1, false);
            comp->mouseUp(eUp);
        }

        static void simulateDrag(juce::Component* comp, float distanceY = -50.0f) {
            if (!comp) return;
            
            juce::Point<float> startPos = comp->getLocalBounds().getCentre().toFloat();
            juce::Point<float> endPos = startPos.translated(0.0f, distanceY);
            
            juce::MouseEvent eDown(juce::Desktop::getInstance().getMainMouseSource(), startPos, juce::ModifierKeys::leftButtonModifier, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comp, comp, juce::Time::getCurrentTime(), startPos, juce::Time::getCurrentTime(), 1, false);
            comp->mouseDown(eDown);
            
            juce::MouseEvent eDrag(juce::Desktop::getInstance().getMainMouseSource(), endPos, juce::ModifierKeys::leftButtonModifier, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comp, comp, juce::Time::getCurrentTime(), startPos, juce::Time::getCurrentTime(), 1, true);
            comp->mouseDrag(eDrag);
            
            juce::MouseEvent eUp(juce::Desktop::getInstance().getMainMouseSource(), endPos, juce::ModifierKeys(), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comp, comp, juce::Time::getCurrentTime(), startPos, juce::Time::getCurrentTime(), 1, true);
            comp->mouseUp(eUp);
        }
    };

    // ==============================================================================
    // 6. Snapshot Deduplicator & Failure Capture
    // ==============================================================================
    struct SnapshotDeduplicator {
        static inline juce::String lastFailedTest;
        static inline juce::String lastFailedComponentId;
        static inline uint64_t lastCaptureTimeMs = 0;

        static bool shouldCapture(const juce::String& testName, const juce::String& compId) {
            auto now = static_cast<uint64_t>(juce::Time::currentTimeMillis());
            if (testName == lastFailedTest && compId == lastFailedComponentId && (now - lastCaptureTimeMs < 10000)) {
                return false; // Deduplicate within 10 seconds
            }
            lastFailedTest = testName;
            lastFailedComponentId = compId;
            lastCaptureTimeMs = now;
            return true;
        }
    };

    inline juce::String captureFailureArtifact(juce::Component& comp, const juce::String& testName, const juce::String& stepName) {
        juce::String compId = comp.getComponentID().isNotEmpty() ? comp.getComponentID() : comp.getName();
        if (!SnapshotDeduplicator::shouldCapture(testName, compId)) {
            std::cerr << "  [SNAPSHOT DEDUPLICATED]: Skipped redundant capture for " << testName << " (" << compId << ")" << std::endl;
            return {};
        }

        auto bounds = comp.getLocalBounds();
        if (bounds.isEmpty()) {
            bounds = juce::Rectangle<int>(0, 0, 400, 300);
        }
        auto img = comp.createComponentSnapshot(bounds);
        juce::File dir(juce::File::getCurrentWorkingDirectory().getChildFile("test_screenshots"));
        if (!dir.exists()) dir.createDirectory();
        
        auto timestamp = juce::Time::getCurrentTime().formatted("%Y%m%d_%H%M%S");
        juce::String safeTestName = juce::File::createLegalFileName(testName).replaceCharacter(' ', '_');
        juce::String safeStepName = juce::File::createLegalFileName(stepName).replaceCharacter(' ', '_');
        if (safeStepName.isEmpty()) safeStepName = "step";

        juce::File file = dir.getChildFile(safeTestName + "_" + safeStepName + "_" + timestamp + ".png");
        juce::FileOutputStream stream(file);
        if (stream.openedOk()) {
            juce::PNGImageFormat png;
            png.writeImageToStream(img, stream);
            juce::String fileUri = "file:///" + file.getFullPathName().replaceCharacter('\\', '/');
            std::cerr << "  [FAILURE SNAPSHOT]: " << fileUri << std::endl;
            return fileUri;
        }
        return {};
    }

    // ==============================================================================
    // 7. Test Reporter & Profiling
    // ==============================================================================
    struct FailureRecord {
        juce::String testName;
        juce::String failureMessage;
        juce::String screenshotPath;
    };

    struct SuiteTiming {
        juce::String suiteName;
        double durationSeconds = 0.0;
    };

    struct TestReporter {
        int passed = 0;
        int failed = 0;
        juce::String currentSuite;
        juce::String currentTest;
        juce::int64 currentSuiteStartTicks = 0;
        std::vector<FailureRecord> failures;
        std::vector<SuiteTiming> suiteTimings;

        void beginSuite(const juce::String& suiteName) {
            endSuite();
            currentSuite = suiteName;
            currentSuiteStartTicks = juce::Time::getHighResolutionTicks();
        }

        void endSuite() {
            if (currentSuite.isNotEmpty() && currentSuiteStartTicks > 0) {
                auto nowTicks = juce::Time::getHighResolutionTicks();
                double durationSec = juce::Time::highResolutionTicksToSeconds(nowTicks - currentSuiteStartTicks);
                suiteTimings.push_back({ currentSuite, durationSec });
                currentSuite = {};
                currentSuiteStartTicks = 0;
            }
        }

        void beginTest(const juce::String& testName) {
            endSuite();
            currentSuite = testName;
            currentTest = testName;
            currentSuiteStartTicks = juce::Time::getHighResolutionTicks();
            std::cout << "\n[TEST] " << testName << std::endl;
            Watchdog::pingHeartbeat();
        }

        bool expect(bool condition, const juce::String& message, juce::Component* compToSnapshot = nullptr, const juce::String& stepName = "") {
            Watchdog::pingHeartbeat();
            if (condition) {
                passed++;
                return true;
            } else {
                failed++;
                std::cerr << "  [FAILED] " << message << std::endl;
                juce::String screenshotPath;
                if (compToSnapshot) {
                    screenshotPath = captureFailureArtifact(*compToSnapshot, currentTest, stepName);
                }
                failures.push_back({ currentTest, message, screenshotPath });
                return false;
            }
        }

        void printSummary() {
            endSuite();

            std::cout << "\n========================================" << std::endl;
            std::cout << "GUI TEST SUMMARY" << std::endl;
            std::cout << "Passed: " << passed << std::endl;
            std::cout << "Failed: " << failed << std::endl;
            std::cout << "========================================" << std::endl;

            if (!failures.empty()) {
                std::cerr << "\n🚨 ========================================================" << std::endl;
                std::cerr << "🚨 CRITICAL FAILURE SUMMARY (" << failures.size() << " failures)" << std::endl;
                std::cerr << "🚨 ========================================================" << std::endl;
                for (size_t i = 0; i < failures.size(); ++i) {
                    const auto& rec = failures[i];
                    std::cerr << " [" << (i + 1) << "] Suite/Test: " << rec.testName << std::endl;
                    std::cerr << "     Reason:     " << rec.failureMessage << std::endl;
                    if (rec.screenshotPath.isNotEmpty()) {
                        std::cerr << "     Snapshot:   " << rec.screenshotPath << std::endl;
                    }
                }
                std::cerr << "🚨 ========================================================\n" << std::endl;
            }

            if (!suiteTimings.empty()) {
                auto sorted = suiteTimings;
                std::sort(sorted.begin(), sorted.end(), [](const SuiteTiming& a, const SuiteTiming& b) {
                    return a.durationSeconds > b.durationSeconds;
                });

                double totalSeconds = 0.0;
                for (const auto& s : suiteTimings) totalSeconds += s.durationSeconds;

                std::cout << "\n⏱️  ========================================================" << std::endl;
                std::cout << "⏱️  EXECUTION PROFILING LEADERBOARD (Total: " << juce::String(totalSeconds, 2) << "s)" << std::endl;
                std::cout << "⏱️  ========================================================" << std::endl;
                size_t count = std::min(size_t(5), sorted.size());
                for (size_t i = 0; i < count; ++i) {
                    std::cout << "  " << (i + 1) << ". " << sorted[i].suiteName 
                              << " - " << juce::String(sorted[i].durationSeconds, 3) << "s" << std::endl;
                }
                std::cout << "⏱️  ========================================================\n" << std::endl;
            }

            if (failed > 0) {
                std::cerr << "SOME TESTS FAILED." << std::endl;
            } else {
                std::cout << "ALL TESTS PASSED SUCCESSFULLY." << std::endl;
            }
        }
    };

    // ==============================================================================
    // 8. Wait-Fail Component Locator
    // ==============================================================================
    template <typename T>
    inline T* waitForComponent(juce::Component* parent, 
                               const juce::String& identifier = {}, 
                               int timeoutMs = 30000, 
                               int pollIntervalMs = 20, 
                               juce::Component* rootToSnapshot = nullptr,
                               TestReporter* reporter = nullptr) 
    {
        auto start = juce::Time::getMillisecondCounter();
        while (juce::Time::getMillisecondCounter() - start < static_cast<juce::uint32>(timeoutMs)) {
            Watchdog::pingHeartbeat();
            
            auto allTyped = ComponentFinder::findAllByType<T>(parent);
            for (auto* comp : allTyped) {
                if (identifier.isEmpty() || comp->getName() == identifier || comp->getComponentID() == identifier)
                    return comp;
            }

            juce::Timer::callAfterDelay(pollIntervalMs, []{ 
                if (auto* mm = juce::MessageManager::getInstanceWithoutCreating()) 
                    mm->stopDispatchLoop(); 
            });
            juce::MessageManager::getInstance()->runDispatchLoop();

            allTyped = ComponentFinder::findAllByType<T>(parent);
            for (auto* comp : allTyped) {
                if (identifier.isEmpty() || comp->getName() == identifier || comp->getComponentID() == identifier)
                    return comp;
            }
        }

        if (reporter) {
            reporter->expect(false, "waitForComponent timed out waiting for: " + identifier, 
                             rootToSnapshot ? rootToSnapshot : parent, "wait_timeout");
        }
        return nullptr;
    }

    // ==============================================================================
    // 9. Font Bounds Helper
    // ==============================================================================
    struct FontBoundsHelper {
        static bool isTextTruncated(const juce::String& text, const juce::Font& font, int availableWidth) {
            if (text.isEmpty() || availableWidth <= 0) return false;
            int textWidth = juce::GlyphArrangement::getStringWidthInt(font, text);
            return textWidth > availableWidth;
        }

        static bool isLabelTruncated(juce::Label* label) {
            if (!label || !label->isVisible()) return false;
            if (dynamic_cast<juce::ComboBox*>(label->getParentComponent()) != nullptr) return false;
            auto text = label->getText();
            if (text.isEmpty()) return false;
            auto font = label->getFont();
            auto border = label->getBorderSize();
            int availW = label->getWidth() - border.getLeftAndRight();
            return isTextTruncated(text, font, availW);
        }

        static bool isButtonTruncated(juce::TextButton* button) {
            if (!button || !button->isVisible()) return false;
            auto text = button->getButtonText();
            if (text.isEmpty() || text.length() <= 2 || button->getWidth() <= 30) return false;
            juce::Font font(juce::FontOptions(13.0f));
            int availW = button->getWidth() - 8;
            return isTextTruncated(text, font, availW);
        }

        static bool isSliderTextTruncated(RotaryKnobSlider* slider) {
            if (!slider || !slider->isVisible() || slider->getWidth() <= 20) return false;
            auto valStr = slider->getTextFromValue(slider->getValue());
            juce::Font font(juce::FontOptions(13.0f, juce::Font::bold));
            int valW = juce::GlyphArrangement::getStringWidthInt(font, valStr);
            if (valW > slider->getWidth() - 10) return true;

            auto label = slider->getLabel();
            if (label.isNotEmpty()) {
                int labelW = juce::GlyphArrangement::getStringWidthInt(font, label.toUpperCase());
                int availLabelW = slider->getWidth() - std::max(78, valW + 4) - 24;
                if (labelW > availLabelW && availLabelW > 20) return true;
            }
            return false;
        }

        static bool isCardTitleTruncated(ModuleCardComponent* card) {
            if (!card || !card->isVisible() || card->getWidth() <= 20) return false;
            auto title = card->getTitle();
            if (title.isEmpty()) return false;
            juce::Font font(juce::FontOptions(14.0f, juce::Font::bold));
            int titleW = juce::GlyphArrangement::getStringWidthInt(font, title.toUpperCase());
            int availW = card->getWidth() - 52;
            return titleW > availW;
        }
    };

} // namespace GuiTestHelpers
