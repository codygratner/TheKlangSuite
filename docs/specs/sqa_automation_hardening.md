# Plan: SQA Automation Hardening (v0.4.0)

> **Status:** COMPLETED ✅ (Implemented & verified in `test/GuiTestHelpers.h` & `test/gui_tests.cpp`, commit on 2026-10-07)
> **Origin:** SQA Advisory Consultation (Tom) — Local Vault Inbox
> **Context:** `docs/SQA_MEETING_BRIEFING.md`, Section 5.2 of `docs/BACKLOG.md`

## Overview
This architectural plan implements the SQA recommendations to harden our automated testing infrastructure (`gui_tests` and `dsp_tests`). It introduces a robust "wait-fail" pattern for asynchronous UI testing, global/local timeout guardrails, automated deduplicated failure snapshots, and advanced telemetry (profiling metrics and chaos testing hooks).

## Phase 1: Dual Timeout Architecture (Global & Local)
**Goal:** Prevent hung test runners and allow individual tests to fail fast without cascading aborts.

1. **Global Watchdog Timeout (5 Minutes)**
   - Implement a global watchdog timer at the suite level (in `main()` or via a threaded test runner wrapper).
   - If the entire test run exceeds 5 minutes, forcibly terminate the process, flush logs, and exit with an error code to protect CI budgets.
2. **Local Step Timeout (30 Seconds)**
   - Extend the JUCE `UnitTest` class or wrap test execution blocks with a local 30-second bounded execution limit.
   - If a specific test step hangs, gracefully abort that test, record the timeout as a failure, and seamlessly advance to the next independent test.
3. **Decoupled Test Independence Assurance**
   - Audit all existing tests in `PluginIntensiveTestSuite` and `HardeningSuites` to guarantee zero shared mutable state between tests.
   - Ensure the teardown phase reliably cleans up singletons and thread pools so a timeout in one test doesn't poison the environment for the next.

## Phase 2: The "Wait-Fail" Component Locator Pattern
**Goal:** Replace brittle instant assertions with resilient polling loops that pump the message thread, catering to asynchronous transitions (modals, animations, callouts).

1. **Implement `waitForComponent<T>`**
   - Create a utility function: `template <typename T> T* waitForComponent(juce::Component* parent, const juce::String& id, int timeoutMs = 30000);`
   - **Logic:** Enter a localized `juce::MessageManager::getInstance()->runDispatchLoopUntil(timeoutMs)` polling loop.
   - Continuously scan the visual tree for the component. Return the pointer if found.
   - If `timeoutMs` expires without resolution, trigger the failure snapshot protocol and bubble the error.

## Phase 3: Automated Failure Snapshots & Deduplication
**Goal:** Automatically capture visual evidence of headless UI failures to drastically reduce debugging time, ensuring no disk space bloat.

1. **Snapshot Generator (`SnapshotFailureObserver`)**
   - When an assertion fails or a `waitForComponent` times out, immediately capture the `juce::Component::createComponentSnapshot(getLocalBounds())` of the active window/chassis.
   - Write the PNG to `test_screenshots/<TestCategory>_<TestName>_<Timestamp>.png`.
2. **Rate Limiting & Deduplication**
   - Track the timestamp and test name of the last snapshot.
   - If multiple assertions fail within the same 60-second window for the same component hierarchy, suppress subsequent PNG captures to avoid disk flooding.
3. **Telemetry & Log Linkage**
   - Print the exact absolute file path of the snapshot to the console output.
   - Ensure the Obsidian sync bridge (`tools/sync_obsidian_vault.ps1`) can parse this path and embed the screenshot link into the mobile telemetry feed (`Telemetry/Dashboard.md`).

## Phase 4: Test Profiling Metrics & "Chaos" Hooks
**Goal:** Expose granular performance metrics to catch regressions early, and lay the groundwork for automated chaos testing.

1. **Execution Profiling (Test Runner Metrics)**
   - Track the start and end `juce::Time::getHighResolutionTicks()` for every test.
   - Generate a "Slowest Tests" leaderboard at the end of the test run, bringing visibility to UI operations that are degrading in performance over time.
   - Track suite averages and standard deviations.
2. **Failure-First Log Sorting**
   - Restructure the final test report to print all failed assertions, timeouts, and snapshot paths at the very top of the console output, avoiding scrolling through hundreds of passed test logs.
3. **Chaos Testing Hooks (Foundation)**
   - Expose a `setChaosSeed(uint32_t seed)` function to the test runner.
   - Add a `simulateChaosMonkey(int durationMs)` utility that rapidly fires randomized mouse clicks, window resize events, and slider drags on the active UI tree.
   - This paves the way for a future "Chaos Gauntlet" test suite that tries to deliberately crash the GUI.

## Phase 5: CI/CD & Build System Integration
**Goal:** Wire the new hardening features into the build validation pipeline.

1. **Update `tools/run_pluginval.ps1` and CI Action**
   - Ensure the `test_screenshots/` directory is created automatically and `.gitignore`d (while keeping a placeholder or archiving it as a GitHub Actions artifact on failure).
2. **End-to-End Validation**
   - Introduce a deliberate, temporary failing test (e.g., waiting for a non-existent component) to prove that the 30s local timeout works, the failure snapshot is generated, and the file path appears in the telemetry log. Remove the deliberate failure once verified.
