# Implementation Plan: WAV Render, Multi-Sample & SF2 Export Dialog + Instant DAW Drag 'n' Drop (Features #16 & #17)

This four-phase implementation plan adds **SoundFont 2 (`.sf2`) Multi-Sample Bank Export** alongside **Offline WAV Rendering** and **Tekno-Style Instant DAW Drag 'n' Drop** for `TheKlangFarmer` and `TheKlangPlanter`.

---

## Architectural Guardrails

1. **Zero Real-Time Audio Contention:** Offline bounces (both `.wav` and `.sf2`) and drag-cache renders instantiate an isolated `ModularDrumEngine` (or `PlanterEngine`) on the calling worker/UI thread and read parameter values via the processor's atomic parameter pointers. The live `engine` instance used by `processBlock()` is never locked, reset, or mutated during export.
2. **Sub-Chunk Modulation Fidelity:** Offline rendering steps in the exact same `SUB_CHUNK = 32` sample slices used in `processBlock()`, evaluating velocity, key-tracking, and envelope modulation targets on the offline engine instance so bounced audio matches real-time playback 1:1.
3. **Continuous PRNG State Across Round-Robins:** Multi-sample batch exports reuse a single offline engine instance across consecutive Round-Robin (`RR`) takes so internal analog drift / `Slop` PRNG states advance naturally between variations.
4. **Zero-Dependency RIFF `sfbk` v2.01 Writer:** The SoundFont 2 exporter writes compliant RIFF `sfbk` v2.01 files directly via `juce::OutputStream` with automatic contiguous key-zone spanning (when `noteStep` is greater than 1 semitone), contiguous velocity-split ranges (`1..127`), linked L/R stereo sample headers with **46 zero guard samples** per slice, and multi-preset Round-Robin mapping (`Preset 000: RR1`, `Preset 001: RR2`, etc.).

---

## Phase 1: Offline Render Pipeline, SF2 Builder & Last-Note Tracking (`UIComponents.*`, `PluginProcessor.*`, `PlanterProcessor.*`)

### 1.1 Shared Offline Render Configuration & SF2 Structures (`source/UIComponents.h`)

Define the shared configuration structs, WAV utilities, and SoundFont 2 (`sfbk` v2.01) builder in `source/UIComponents.h` (implemented in `source/UIComponents.cpp`) so both `TheKlangFarmer` and `TheKlangPlanter` share the entire export pipeline.

```cpp
// In source/UIComponents.h
struct OfflineRenderSettings {
    enum class TargetMode { LastNote = 0, MultiSample = 1 };
    enum class ExportFormat { WavFiles = 0, SoundFont2 = 1, WavAndSf2 = 2 };
    enum class TailMode   { Fixed0_5s = 0, Fixed1_0s = 1, Fixed2_0s = 2, Fixed4_0s = 3, AutoMinus60dB = 4, AutoMinus80dB = 5 };
    enum class NormMode   { Off = 0, Peak0dB = 1, PeakMinus0_3dB = 2 };

    TargetMode targetMode     = TargetMode::LastNote;
    ExportFormat exportFormat = ExportFormat::WavFiles;
    double sampleRate         = 48000.0; // 44100.0, 48000.0, 96000.0
    int bitDepth              = 24;      // 16, 24, 32 (32 = IEEE Float for WAV; SF2 encodes 16-bit PCM smpl)
    TailMode tailMode         = TailMode::Fixed1_0s;
    NormMode normMode         = NormMode::Off;

    // Multi-Sample Range settings
    int startNote             = 36;      // C1 (MIDI 36)
    int endNote               = 60;      // C3 (MIDI 60)
    int noteStep              = 1;       // 1=Semitone, 3=Min3rd, 4=Maj3rd, 7=Fifth, 12=Octave
    int velocityLayers        = 1;       // 1..4 splits
    int roundRobins           = 1;       // 1, 2, 4, 8

    juce::String filePrefix   = "TKF_Hit";
};

struct Sf2ZoneSampleEntry {
    int rootMidiNote = 60;
    int keyLow       = 60;
    int keyHigh      = 60;
    int velLow       = 1;
    int velHigh      = 127;
    int roundRobinIndex = 0; // 0-based RR index -> maps to SF2 Preset wPreset
    uint32_t sampleRate = 48000;
    juce::String sampleBaseName;
    juce::AudioBuffer<float> stereoBuffer;
};

namespace OfflineRenderUtils {
    int getMaxRenderSamples(const OfflineRenderSettings& settings);

    void postProcessRenderedBuffer(juce::AudioBuffer<float>& buffer,
                                   const OfflineRenderSettings& settings);

    bool writeBufferToWavFile(const juce::AudioBuffer<float>& buffer,
                              double sampleRate,
                              int bitDepth,
                              const juce::File& targetFile);

    // Writes a complete SoundFont 2.01 (.sf2) file from rendered zone entries
    bool writeSf2BankFile(const std::vector<Sf2ZoneSampleEntry>& entries,
                          const juce::String& bankName,
                          int numRoundRobins,
                          const juce::File& targetSf2File);

    juce::String formatMidiNoteToken(int midiNote);

    // Returns trigger velocity (1..127) and fills [outVelLow, outVelHigh] for SF2 zone mapping
    int getVelocitySplitRange(int layerIndex, int numLayers, int& outVelLow, int& outVelHigh);

    // Computes contiguous [outKeyLow, outKeyHigh] for noteIndex across sorted rendered notes
    void getKeyZoneRange(const std::vector<int>& renderedNotes, int noteIndex, int& outKeyLow, int& outKeyHigh);
}
```

### 1.2 Post-Processing, WAV Writer & SF2 Binary Serializer (`source/UIComponents.cpp`)

Implement `OfflineRenderUtils` in `source/UIComponents.cpp`:

1. **Auto-Silence Detection (`AutoMinus60dB` = `0.001f`, `AutoMinus80dB` = `0.0001f`):**
   - Allocate `8.0` seconds maximum buffer when an auto-tail mode is selected.
   - Enforce a minimum hold window of `150 ms` from sample `0` so slow-attack patches are never truncated at the start.
   - Scan backward from the end of the buffer to find the last stereo frame where $\max(\vert{}L\vert{}, \vert{}R\vert{})$ exceeds the threshold.
   - Append a `15 ms` safety tail after the threshold crossing, clamp to total buffer size (minimum `0.15s`), resize with `buffer.setSize(2, trimmedSamples, true, true, false)`, and apply a `10 ms` raised-cosine fade-out over the final samples.
   - For fixed tail modes (`0.5s`, `1.0s`, `2.0s`, `4.0s`), apply a `5 ms` cosine fade-out at the end of the buffer.

2. **Peak Normalization (`Peak0dB` = `1.0f`, `PeakMinus0_3dB` = `0.9660508f`):**
   - Measure `float peak = buffer.getMagnitude(0, buffer.getNumSamples())`.
   - When `peak` exceeds `1e-6f`, call `buffer.applyGain(targetPeak / peak)`.

3. **Key Zone & Velocity Split Mapping (`getKeyZoneRange` & `getVelocitySplitRange`):**
   - **Velocity Splits:**
     - 1 Layer: `[1..127]` (triggered at `127`)
     - 2 Layers: `[1..64]` (at `64`), `[65..127]` (at `127`)
     - 3 Layers: `[1..42]` (at `42`), `[43..84]` (at `84`), `[85..127]` (at `127`)
     - 4 Layers: `[1..32]` (at `32`), `[33..64]` (at `64`), `[65..96]` (at `96`), `[97..127]` (at `127`)
   - **Key Zones:** For a sorted list of rendered MIDI notes $N_0, N_1, \dots, N_{k-1}$:
     - `outKeyLow = (i == 0) ? N[0] : ((N[i - 1] + N[i]) / 2 + 1)`
     - `outKeyHigh = (i + 1 == k) ? N[k - 1] : ((N[i] + N[i + 1]) / 2)`
     - This ensures that interval exports (e.g., Minor 3rds or Octaves) automatically span across adjacent keys in the `.sf2` patch without unmapped dead notes.

4. **SoundFont 2 (`writeSf2BankFile`) RIFF `sfbk` v2.01 Specification:**
   - **Sample Pool (`LIST 'sdta'` -> `'smpl'`):**
     - For each `Sf2ZoneSampleEntry`, append the Left channel as 16-bit signed PCM (`int16_t` clamped to `[-32768, 32767]`) followed by **46 zero guard samples** (`92` zero bytes, required by the SF2 v2.01 spec for hardware/software interpolator lookahead), then append the Right channel followed by **46 zero guard samples**.
     - Record two linked `shdr` entries (`46` bytes each):
       - Left `shdr`: `achSampleName` (`<Name>_L`, max 19 chars + null), `dwStart`, `dwEnd`, `dwStartloop = dwStart + 8`, `dwEndloop = std::max(dwStart + 16, dwEnd - 8)`, `dwSampleRate`, `byOriginalPitch = rootMidiNote`, `chPitchCorrection = 0`, `wSampleLink = rightSampleHeaderIdx`, `sfSampleType = 2` (`leftSample`).
       - Right `shdr`: `achSampleName` (`<Name>_R`), `wSampleLink = leftSampleHeaderIdx`, `sfSampleType = 4` (`rightSample`).
   - **Preset & Instrument Hierarchy (`LIST 'pdta'`):**
     - Group zones by `roundRobinIndex` (`0 .. numRoundRobins - 1`).
     - Create one SF2 Instrument (`inst`, `22` bytes) and one SF2 Preset (`phdr`, `38` bytes, `wBank = 0`, `wPreset = rrIdx`) per Round-Robin variation (`<Prefix>_RR1`, `<Prefix>_RR2`, or `<Prefix>` when `numRoundRobins == 1`).
     - For each stereo zone in an instrument, emit **2 instrument bags (`ibag`)** (one for the Left sample, one for the Right sample), each containing **6 instrument generators (`igen`, `4` bytes each)** in strict SF2 generator order:
       1. `sfGenOper = 43` (`keyRange`): `uint16_t(keyLow | (keyHigh << 8))`
       2. `sfGenOper = 44` (`velRange`): `uint16_t(velLow | (velHigh << 8))`
       3. `sfGenOper = 17` (`pan`): `int16_t(-500)` for Left zone, `int16_t(+500)` for Right zone
       4. `sfGenOper = 38` (`releaseVolEnv`): `int16_t(3600)` (~8.0s release in timecents so one-shot drum tails never cut off on `NoteOff`)
       5. `sfGenOper = 58` (`overridingRootKey`): `int16_t(rootMidiNote)`
       6. `sfGenOper = 53` (`sampleID`): `uint16_t(sampleHeaderIdx)` *(must be last generator in zone)*
     - Append the mandatory terminal records (`"EOP"`, terminal `pbag`, terminal `pmod`, terminal `pgen`, `"EOI"`, terminal `ibag`, terminal `imod`, terminal `igen`, and `"EOS"`) and write the complete `RIFF` `sfbk` stream to `targetSf2File`.

### 1.3 Processor State Tracking & Offline Engine Sync (`source/PluginProcessor.*` & `source/PlanterProcessor.*`)

In `TheKlangFarmerAudioProcessor` (and mirrored in `TheKlangPlanterAudioProcessor`):

1. Add atomic tracking members to `PluginProcessor.h`:
```cpp
std::atomic<int>      lastTriggeredNote     { 60 };
std::atomic<float>    lastTriggeredVelocity { 1.0f };
std::atomic<uint32_t> triggerGeneration     { 0 };

void triggerAuditionNote(int midiNote = 60, float normVelocity = 1.0f);
int getLastTriggeredNote() const noexcept       { return lastTriggeredNote.load(std::memory_order_relaxed); }
float getLastTriggeredVelocity() const noexcept { return lastTriggeredVelocity.load(std::memory_order_relaxed); }
uint32_t getTriggerGeneration() const noexcept  { return triggerGeneration.load(std::memory_order_relaxed); }

void syncEngineToCurrentParameters(TbdAudio::ModularDrumEngine& targetEngine, double sampleRate);
void renderSingleHitOffline(TbdAudio::ModularDrumEngine& offlineEngine,
                            int midiNote,
                            float normVelocity,
                            const OfflineRenderSettings& settings,
                            juce::AudioBuffer<float>& outBuffer);
```

2. Update `processBlock()` MIDI `msg.isNoteOn()` handling to store `lastTriggeredNote`, `lastTriggeredVelocity`, and increment `triggerGeneration`.
3. Extract parameter synchronization into `syncEngineToCurrentParameters(...)` and parameterize `applyModulationTargetsForEngine(...)` so `renderSingleHitOffline(...)` steps in `SUB_CHUNK = 32` sample slices identically to `processBlock()`.

---

## Phase 2: Instant DAW Drag 'n' Drop ("Tekno-Style" Header Badge)

### 2.1 `InstantDragBadgeComponent` (`source/UIComponents.h` & `source/UIComponents.cpp`)

Create a custom header component that displays a miniature waveform preview of the last auditioned hit and initiates an external OS/DAW file drag when dragged past a 4-pixel threshold.

```cpp
class InstantDragBadgeComponent : public juce::Component,
                                  public juce::SettableTooltipClient {
public:
    InstantDragBadgeComponent();

    std::function<juce::File()> onRequestDragWavFile;
    void setWaveformPreview(const juce::AudioBuffer<float>& buffer, const juce::String& labelText);

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

private:
    std::vector<float> previewPeaks;
    juce::String badgeLabel { "DRAG WAV" };
    bool isDragInProgress = false;
    bool isHovered = false;
};
```

### 2.2 Temp WAV Cache Lifecycle in Editor (`source/PluginEditor.*` & `source/PlanterEditor.*`)

1. Inherit `public juce::DragAndDropContainer` on `TheKlangFarmerAudioProcessorEditor` and `TheKlangPlanterAudioProcessorEditor`.
2. Implement `juce::File renderInstantDragCacheWav()` on the Editor:
   - Uses a dedicated offline engine member `std::unique_ptr<TbdAudio::ModularDrumEngine> dragOfflineEngine`.
   - Renders with `sampleRate = 48000.0`, `bitDepth = 24`, `tailMode = AutoMinus60dB`, `normMode = PeakMinus0_3dB` at `lastTriggeredNote` and `lastTriggeredVelocity`.
   - Saves to `juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("RlyehSound_DragCache")` (`TKF_Drag_<NoteName>_v<Vel>.wav`) and updates `dragBadge.setWaveformPreview(...)`.
3. Refresh the badge waveform preview automatically when `AUDITION HIT` is clicked or when `audioProcessor.getTriggerGeneration()` advances from incoming MIDI notes, and re-render on `onRequestDragWavFile` so post-hit knob edits are always captured.

---

## Phase 3: WAV Render, Multi-Sample & SF2 Export Modal Dialog (`source/UIComponents.h` & `source/UIComponents.cpp`)

### 3.1 `RenderExportModalComponent` Architecture

Create `RenderExportModalComponent` in `source/UIComponents.h` / `source/UIComponents.cpp`, matching the dark modal overlay styling of `QuickstartGuideModalComponent` (`0xff151821` card body, `0xff00d2ff` border, full-window dimmed backdrop).

```cpp
class RenderExportModalComponent : public juce::Component,
                                   private juce::Thread,
                                   private juce::AsyncUpdater {
public:
    using RenderCallback = std::function<void(int midiNote,
                                              float normVelocity,
                                              const OfflineRenderSettings& settings,
                                              bool resetEngineInstance,
                                              juce::AudioBuffer<float>& outBuffer)>;

    RenderExportModalComponent(const juce::String& defaultPrefix, RenderCallback renderFn);
    ~RenderExportModalComponent() override;

    void setLastTriggeredInfo(int midiNote, float normVelocity);
    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;

private:
    void run() override;                 // juce::Thread background batch loop
    void handleAsyncUpdate() override;   // Thread-safe UI progress & completion updates
    void updateControlStates();          // Updates enabled controls & summary text
    void startExportFlow();              // Opens juce::FileChooser::launchAsync

    RenderCallback renderHitFn;
    OfflineRenderSettings currentSettings;
    int lastNote = 60;
    float lastVelocity = 1.0f;

    // UI Controls
    juce::ComboBox targetModeBox;        // 1: Last Auditioned Note, 2: Multi-Sample Range
    juce::ComboBox exportFormatBox;      // 1: WAV Files (Folder), 2: SoundFont 2 (.sf2), 3: WAV Files + .sf2 Bank
    juce::ComboBox startNoteBox;         // C0 (12) .. C6 (84)
    juce::ComboBox endNoteBox;           // C0 (12) .. C6 (84)
    juce::ComboBox noteStepBox;          // Semitone (1), Min 3rd (3), Maj 3rd (4), Fifth (7), Octave (12)
    juce::ComboBox velocityLayersBox;    // 1 (127), 2 (64,127), 3 (42,84,127), 4 (32,64,96,127)
    juce::ComboBox roundRobinBox;        // 1x, 2x, 4x, 8x (Slop Variations)

    juce::ComboBox sampleRateBox;        // 44.1 kHz, 48 kHz, 96 kHz
    juce::ComboBox bitDepthBox;          // 16-bit PCM, 24-bit PCM, 32-bit Float
    juce::ComboBox tailLengthBox;        // 0.5s, 1.0s, 2.0s, 4.0s, Auto (-60 dB), Auto (-80 dB)
    juce::ComboBox normModeBox;          // Raw (Off), Peak 0.0 dB, Peak -0.3 dB
    juce::TextEditor prefixEditor;

    juce::Label summaryLabel;            // e.g. "Export Queue: 25 notes x 4 vel x 2 RR = 200 samples (WAV + SF2)"
    juce::Label statusLabel;             // Progress text / active filename
    juce::TextButton exportButton { "EXPORT..." };
    juce::TextButton cancelRenderButton { "CANCEL" };
    juce::TextButton closeButton { "CLOSE" };

    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::File targetExportLocation;     // Directory (for Folder modes) or .sf2 File (for SF2-only mode)

    std::atomic<float> progressValue { 0.0f };
    std::atomic<int> completedFiles { 0 };
    std::atomic<int> totalFilesToRender { 0 };
    std::atomic<bool> renderFinished { false };
    juce::String lastRenderedFilename;
    juce::CriticalSection statusLock;
};
```

### 3.2 File Chooser & Background Export Loop (`RenderExportModalComponent::startExportFlow` & `run`)

1. **Destination Selection (`startExportFlow`):**
   - If `exportFormat == ExportFormat::SoundFont2`: Launch `juce::FileChooser` with `juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting` and default filename `<Prefix>.sf2`.
   - If `exportFormat == ExportFormat::WavFiles` or `ExportFormat::WavAndSf2`: Launch `juce::FileChooser` with `juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories` to select the destination folder.

2. **Background Render & Serialization (`run`):**
   - Build the sorted list of MIDI notes (`renderedNotes`).
   - Initialize `std::vector<Sf2ZoneSampleEntry> sf2Entries` if `exportFormat != ExportFormat::WavFiles`.
   - Iterate through `noteIndex` (`0 .. renderedNotes.size() - 1`), `layerIndex` (`0 .. velocityLayers - 1`), and `rr` (`0 .. roundRobins - 1`):
     - Check `threadShouldExit()` before each render.
     - Compute `[keyLow, keyHigh]` via `OfflineRenderUtils::getKeyZoneRange(renderedNotes, noteIndex, keyLow, keyHigh)` and `[velLow, velHigh]` + `velInt` via `OfflineRenderUtils::getVelocitySplitRange(layerIndex, velocityLayers, velLow, velHigh)`.
     - Call `renderHitFn(note, velInt / 127.0f, settingsSnap, (jobIndex == 0), audioBuffer)` so the offline engine's Slop PRNG state advances continuously across Round-Robin takes.
     - If `exportFormat == WavFiles` or `WavAndSf2`, write `<Prefix>_<NoteToken>_v<Vel>_rr<RR>.wav` via `OfflineRenderUtils::writeBufferToWavFile(...)`.
     - If `exportFormat == SoundFont2` or `WavAndSf2`, move/copy `audioBuffer` and zone metadata into `sf2Entries`.
     - Update `completedFiles`, `progressValue`, and call `triggerAsyncUpdate()`.
   - After the sample loop completes (and if `!threadShouldExit()`), if `exportFormat == SoundFont2` or `WavAndSf2`:
     - Determine `.sf2` path (`targetExportLocation` if `SoundFont2`, or `targetExportLocation.getChildFile(prefix + ".sf2")` if `WavAndSf2`).
     - Call `OfflineRenderUtils::writeSf2BankFile(sf2Entries, prefix, roundRobins, sf2File)`.

---

## Phase 4: Header Integration & Automated Unit Tests

### 4.1 Header Layout Updates (`source/PluginEditor.*` & `source/PlanterEditor.*`)

Update the header controls in both `TheKlangFarmerAudioProcessorEditor` and `TheKlangPlanterAudioProcessorEditor`:

| Control | Member Name | Bounds in `resized()` | Purpose |
| --- | --- | --- | --- |
| **Tooltips Toggle** | `tooltipsButton` | `(getWidth() - 614, 5, 72, 26)` | Toggles hover parameter tooltips |
| **Quickstart Guide** | `guideButton` | `(getWidth() - 536, 5, 80, 26)` | Opens Quickstart modal |
| **Initialize** | `initButton` | `(getWidth() - 450, 5, 76, 26)` | Resets patch to Default or Clean FX |
| **Render Dialog** | `renderButton` | `(getWidth() - 368, 5, 86, 26)` | Opens `RenderExportModalComponent` |
| **Instant DAW Drag** | `dragBadge` | `(getWidth() - 276, 5, 124, 26)` | Tekno-style waveform drag-and-drop badge |
| **Audition Hit** | `triggerButton` | `(getWidth() - 146, 5, 136, 26)` | Fires manual hit & updates drag cache |

### 4.2 Automated Verification (`test/dsp_tests.cpp`)

Add a dedicated test suite section in `test/dsp_tests.cpp` covering the offline rendering, post-processing, and SF2 binary packaging pipeline:

1. **Multi-Sample-Rate Render Integrity (`44.1 kHz`, `48 kHz`, `96 kHz`):**
   - Instantiate `ModularDrumEngine` and `PlanterEngine`, trigger at MIDI note `36` (`C1`) and velocity `1.0f`, render `0.5s` across all three sample rates, and assert zero `NaN`/`Inf` samples and non-zero RMS energy.
2. **Auto-Silence Detection & Micro-Fade Test:**
   - Synthesize a short percussive decay (`100 ms`) into a `4.0s` buffer and run `OfflineRenderUtils::postProcessRenderedBuffer` with `TailMode::AutoMinus60dB` and `TailMode::AutoMinus80dB`.
   - Assert that the output buffer length is strictly less than `1.0s`, greater than the `150 ms` minimum hold window, and that the final sample frame is `0.0f`.
3. **Peak Normalization Accuracy (`0.0 dB` & `-0.3 dB`):**
   - Run `postProcessRenderedBuffer` with `NormMode::Peak0dB` and `NormMode::PeakMinus0_3dB` and assert peak magnitude matches `1.0f` and `0.9660508f` within `1e-4f` tolerance.
4. **Slop Round-Robin Variation Uniqueness:**
   - Configure `ModularDrumEngine` with `slop = 0.5f`, render 4 consecutive Round-Robin buffers on the same engine instance, and verify that cross-buffer sample differences are strictly non-zero between `RR1..RR4`.
5. **SF2 (`sfbk` v2.01) Binary Structure & Zone Mapping Verification:**
   - Construct a multi-sample test bank (`2` notes $\times$ `2` velocity layers $\times$ `2` Round-Robins = `8` stereo entries) and write to a temporary `.sf2` file via `OfflineRenderUtils::writeSf2BankFile(...)`.
   - Read the generated `.sf2` binary back into memory and assert:
     - Top-level `RIFF` header size equals `fileSize - 8` and form type is `'sfbk'`.
     - Presence and valid chunk alignment of `LIST 'INFO'`, `LIST 'sdta'` (`'smpl'`), and `LIST 'pdta'` (`'phdr'`, `'pbag'`, `'pmod'`, `'pgen'`, `'inst'`, `'ibag'`, `'imod'`, `'igen'`, `'shdr'`).
     - `'phdr'` size is `(2 + 1) * 38` bytes (`2` RR presets + `EOP`), `'inst'` size is `(2 + 1) * 22` bytes (`2` RR instruments + `EOI`), and `'shdr'` size is `(16 + 1) * 46` bytes (`8` stereo pairs = `16` mono sample headers + `EOS`).
     - Contiguous key range and velocity split partitioning (`[1..64]` and `[65..127]`) have zero gaps or overlaps.
