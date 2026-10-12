# Architectural Plan: Zero-Server GitHub Crash Reporting Engine
**Target Milestone**: v0.5.0 ("The Pro Workflow Update")  
**Goal**: Capture real-world crash logs, environment metadata, and stack traces from beta testers directly into GitHub Issues with zero server infrastructure, zero hosting costs, and zero secret token leaks.

---

## 1. Architectural Philosophy: The Zero-Server Model
- **No Secret Tokens in Client Binaries**:
  - Embedding a GitHub Personal Access Token (PAT) inside a released VST3/AU binary is a catastrophic security vulnerability, as strings or decompilers can extract the token to compromise repository write permissions.
- **The Pre-Filled GitHub Issue URL Strategy**:
  - Instead of a silent background POST request requiring authenticated write credentials, the plugin detects the crash and generates a pre-formatted GitHub Issue URL:
    ```
    https://github.com/codygratner/TheKlangFarmer/issues/new?title=[Crash+Report]+<Version>+-+<DAW>&labels=crash-report,triage&body=<EncodedMarkdownBody>
    ```
  - When the user launches the plugin following an ungraceful shutdown, a polite dialog appears:
    > *"The Klang Farmer encountered an unexpected shutdown during your last session. Would you like to view the crash details and submit a report on GitHub?"*
  - Clicking **"Submit Report to GitHub"** launches `juce::URL(encodedUrl).launchInDefaultBrowser()`. The user reviews the pre-filled issue markdown and clicks "Submit new issue" using their own GitHub account.

---

## 2. Hybrid Crash Detection Engine
Audio plugins run inside third-party DAW host processes (Ableton Live, FL Studio, Logic, Reaper, Bitwig) where hooking process-wide crash filters can destabilize the DAW's own error handling. We utilize a two-tier hybrid approach:

### Tier A: Scoped In-Process Exception Handlers
- Wrap top-level plugin audio and GUI entry points:
  - `processBlock()`
  - High-frequency UI Timers & Paint callbacks
- Platform implementation:
  - **Windows**: `__try { ... } __except (CrashDumpFilter(GetExceptionInformation())) { ... }` using structured exception handling (SEH).
  - **macOS / Linux**: Posix signal handlers (`SIGSEGV`, `SIGFPE`, `SIGILL`) scoped to plugin-spawned threads, capturing call stacks via `backtrace()` / `dladdr()`.
- **Immediate Panic Action**:
  - If a DSP crash is caught, the handler mutes the audio buffer, flushes engine memory, and writes `crash_trace.log` to disk before the error propagates and crashes the entire DAW project.

### Tier B: Session Heartbeat Lockfile (`session_active.lock`)
- **Lifecycle**:
  1. **Boot**: In `prepareToPlay()` / `createEditor()`, the plugin writes `%APPDATA%/TheKlangFarmer/sessions/session_<pid>.lock` containing startup metadata (DAW name, sample rate, buffer size).
  2. **Run**: Periodic breadcrumb updates (last loaded preset, active page, last tweaked FX block).
  3. **Clean Shutdown**: In `~TheKlangFarmerAudioProcessor()`, the `.lock` file is safely deleted.
- **Crash Detection on Next Boot**:
  - If a `.lock` file exists when a new instance starts, an ungraceful termination occurred (e.g. DAW freeze, host crash, OS restart).
  - The plugin reads the breadcrumbs and presents the recovery/report modal.

---

## 3. Full Diagnostic Bundle & Privacy Scrubbing
### Bundled Metadata
1. **Host Environment**: Host DAW Name & Version (via `juce::PluginHostType`), Audio Driver, Sample Rate, Buffer Size.
2. **Plugin Build**: Plugin Version (`v0.5.0`), Git Commit Hash (`TKF_GIT_HASH`), Build Architecture (`x64` / `arm64`), Plugin Format (`VST3`, `AU`, `Standalone`).
3. **State Breadcrumbs**: Active Preset Name, Active Voice Mode (`One-Shot` / `Gated`), Last Tweaked Parameter ID, Loaded FX Blocks.
4. **Stack Trace**: Demangled C++ call stack identifying the offending function and source line.

### Strict Privacy Path Scrubbing
- To prevent leaking sensitive user information to public GitHub issues:
  - Regex replaces all user home directories:
    - Windows: `C:\Users\[^\\]+\` &rarr; `<UserPath>\`
    - macOS: `/Users/[^/]+/` &rarr; `<UserPath>/`
  - Strips any local audio sample file paths loaded in transient players to basenames only (e.g., `snare_dry.wav` instead of `D:\Personal\PrivateProjects\...\snare_dry.wav`).

---

## 4. UI / UX Integration
- **Post-Crash Recovery Dialog**:
  - A clean, dark-themed JUCE modal showing:
    - Brief summary of the crash (e.g., *"Crash detected in GatedReverbBlock.h:112"*).
    - `[ View Raw Diagnostic Log ]` expander.
    - `[ Submit to GitHub ]` (primary action).
    - `[ Dismiss / Don't Ask Again ]` (secondary action).
- **Settings & About Modal Integration**:
  - In the Settings gear menu, add a permanent button: `[ Open Crash Logs Folder ]` and `[ Report a Bug on GitHub ]`.

---

## 5. Automated Verification & Testing
- **Crash Simulation Harness (`gui_tests` / `dsp_tests`)**:
  - Add a developer debug command `#if TKF_DEBUG_CRASH_TEST` that triggers a deliberate null-pointer dereference or division by zero.
  - Test verifying Tier A SEH intercepts the crash, zeroes the audio buffer, and creates `crash_trace.log` without aborting the test runner.
  - Test verifying privacy scrubber replaces usernames with `<UserPath>`.
