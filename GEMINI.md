# The Cosmic Codex & The Astral Abyss — Personal Knowledge, Creative & Research Directives

## Core Mission
This workspace is **The Cosmic Codex** (`C:\_\Vault` / `C:\_\The Cosmic Codex`), the master Zettelkasten, creative worldbuilding laboratory, and knowledge repository.
It houses **The Astral Abyss** (Supreme Command Bridge & Sovereign HQ).
It is dedicated to solo roleplaying (*The Forge*), the *Cosmic Codex*, music production (*Synth Lab*, Renoise, hardware synths, Dirtywave M8), and personal knowledge management.

## 🎛️ Modular Agent Rules Architecture
Specific behavioral rules, cockpit triggers, and formatting directives are modularized in [`Agent Rules/`](file:///c:/_/Vault/Agent%20Rules/) (dynamically mounted via `.agents/rules/`):
- [**`footer.md`**](file:///c:/_/Vault/Agent%20Rules/footer.md) & [**`project_pills.md`**](file:///c:/_/Vault/Agent%20Rules/project_pills.md): Ambient status footer and navigation bar.
- [**`cockpit_triggers.md`**](file:///c:/_/Vault/Agent%20Rules/cockpit_triggers.md): Metaphors and command triggers (`"engage"`, `"wheels up"`, `"unfurl the star-maps"`, `"make it so"`).
- [**`tone_and_pacing.md`**](file:///c:/_/Vault/Agent%20Rules/tone_and_pacing.md): Empathetic pacing, zero scolding for yak-shaving, and formatting style.
- [**`dialectic_and_pcda.md`**](file:///c:/_/Vault/Agent%20Rules/dialectic_and_pcda.md): Hegelian Method B+ prompting and "Where I Disagree" rules.
- [**`airgap_and_privacy.md`**](file:///c:/_/Vault/Agent%20Rules/airgap_and_privacy.md): Privacy enclaves, data preservation, and "No Gods, No Masters" branch invariant.
- [**`subagents_and_transport.md`**](file:///c:/_/Vault/Agent%20Rules/subagents_and_transport.md): File-first subagent delivery, Flash Tiered routing, and anti-stream timeout protocol.
- [**`workflow_and_runway.md`**](file:///c:/_/Vault/Agent%20Rules/workflow_and_runway.md): 14-day mailbag buffer, two tabling gates, 1-code + 1-creative runway cap, and the "Rejiggering Reality" circuit breaker.
- [**`antigravity_multimodal_guardrail.md`**](file:///c:/_/Vault/Agent%20Rules/antigravity_multimodal_guardrail.md): The Fixer Invariant & Anti-Azathoth Media Guardrail (strict prohibition of raw media reads and HTTP 400 poison-pill loop prevention).
- [**`tkdsp_16_control_canon.md`**](file:///c:/_/Vault/Agent%20Rules/tkdsp_16_control_canon.md): The Universal Dual-Card Control Architecture Standard (32-Control Flagship 4×8 vs. 16-Control Compact, Master Block Glide/Unison, Synth/Drum Pitch Envs, Arcade Combat Drum Triad).
- [**`universal_portability_canon.md`**](file:///c:/_/Vault/Agent%20Rules/universal_portability_canon.md): The Universal Portability Standard (The Immutable Platform-Agnostic Invariant — 100% portable C++20 across Windows, macOS, Linux, Steam Deck, ARM Pi, Teensy, ESP32, and WebAssembly).
- [**`modifier_taxonomy_canon.md`**](file:///c:/_/Vault/Agent%20Rules/modifier_taxonomy_canon.md): The 4-Axis Skill Modifier Taxonomy Canon (Scale/Depth, Stance/Crucible, Perimeter/Online-Offline, Surface/Chat-Board).
- [**`tower_build_guardrail.md`**](file:///c:/_/Vault/Agent%20Rules/tower_build_guardrail.md): The Tower C++ Build Isolation Guardrail (Zero-Local-Build & Context Shield Law — strictly routing all C/C++ compilation and DSP test runs to The Tower with zero compiler noise leakage).
- [**`language_server_canon.md`**](file:///c:/_/Vault/Agent%20Rules/language_server_canon.md): The Antigravity ConnectRPC Language Server Canon (352 ConnectRPC endpoints, ephemeral port discovery, binary SHA-256 fingerprinting, trajectory export, live fleet telemetry, and headless terminal streaming).

The user may edit any note in `Agent Rules/` in Obsidian at any time; changes take effect immediately on the next turn.

## Core Machine Invariants
1. **Client Environment Invariant**:
   - The user operates in **Base Google Antigravity** (the standalone desktop chat app with Chat on the Left and Artifacts/Inspector on the Right), **NOT** the VS Code-based Antigravity IDE.
   - All spatial UI layouts, mental models, and split-screen designs must align with this paradigm.
2. **Universal UTF-8 Invariant (Windows)**:
   - All Python scripts and terminal output MUST explicitly configure UTF-8 stdout/stderr.
   - Text file read/writes MUST explicitly specify `encoding='utf-8'`.
3. **Autonomous Data Bus Protocol & Conditional Response Footers**:
   - **Zero-Token Pre-Flight & Post-Job Status Check**:
     - Check the Inter-Project Data Bus via `python tools/bus.py status --for COSMOS`.
     - When the queue is clear (0 pending dispatches for COSMOS), the script outputs nothing (0 bytes), and Data Bus alert footers MUST be omitted completely (ambient status footer per footer.md remains active).
     - When $\ge 1$ unread dispatch is pending for COSMOS, append a single-line alert footer to the response:
       `> 📬 **Data Bus Alert**: There are unread dispatches pending for COSMOS. Say "check dispatches" to review.`
   - **Autonomous Outgoing Dispatch Invariant**:
     - Whenever work modifies, creates, or reorganizes files in `New Klang City/` (docs, specs, research, telemetry) or cross-project shared assets, automatically dispatch a handoff notification to `TKS` via `python tools/bus.py post` without waiting for manual prompting.
     - **Echo Loop Defense**: Never trigger an outgoing dispatch when merely reading, creating, or acknowledging dispatches in `Dispatches/`, or modifying ephemeral `.obsidian/` caches.
4. **Scratchpad Ingestion Invariant (`SCRATCHPAD.md`)**:
   - **NEVER use `view_file` on [`SCRATCHPAD.md`](file:///c:/_/Vault/SCRATCHPAD.md).**
   - When the user mentions `sp`, `scratchpad`, `scratch pad`, or `/sp`, ALWAYS execute `python tools/sp.py pull`. This atomically archives the contents to [`Archive/Scratchpads/`](file:///c:/_/Vault/Archive/Scratchpads/) and wipes `SCRATCHPAD.md` to blank (0 bytes) in a single command.
   - For non-destructive drafting previews, execute `python tools/sp.py peek` only when the user explicitly requests to peek or preview without clearing.
5. **Vault Root Purity & Ephemeral Directory Isolation**:
   - The root of `c:\_\Vault` is reserved exclusively for sovereign Obsidian knowledge enclaves (`New Klang City/`, `Synth Lab/`, `Archive/`, `Assets/`, `Astrolabe/`, `Codex/`, `Inbox/`, `Klang Confidential/`, `Templates/`, `The Forge/`, `Agent Rules/`, `Dispatches/`, and sovereign CLI `tools/`).
   - **NEVER create temporary directories, scratchpads, generator scripts, or code folders at the root of the Vault** (e.g., `scratch/`, `scripts/`, `tmp/`, `tmp_ocr/`).
   - All ephemeral python scripts, batch generator tools, and intermediate artifacts MUST be placed in `.agents/scripts/` or `.agents/scratch/` (or `<appDataDir>\brain\<conversation-id>\scratch/`).
   - Permanent CLI utilities belong strictly in `tools/` (`bus.py`, `search.py`, `sp.py`).
6. **The Fixer Invariant & Anti-Azathoth Media Guardrail (Poison-Pill Defense)**:
   - **NEVER use `read_file` on raw binary image files (`.jpg`, `.jpeg`, `.gif`, `.webp`, `.bmp`, `.ico`) or extracted PDF raster fragments.**
   - Inspect image dimensions, mode, and metadata via Python PIL (`Image.open(p).size`). Only use `read_file` on validated, sanitized RGB `.png` vector diagrams generated by our own visual engine.
   - At the first sign of an `HTTP 400 Bad Request` or `INVALID_MEDIA` error, immediately execute `python tools/fixer.py status` and `sanitize` rather than retrying blindly.
7. **Dropbox Audio Software Archive (`C:\_\Dropbox\_AudioSoftware`)**:
   - The master repository for audio software, DAW plugins, installers, and legacy audio packages is permanently located at [`C:\_\Dropbox\_AudioSoftware`](file:///C:/_/Dropbox/_AudioSoftware).
   - Contains standalone plugin installers and zip archives (e.g. Redux versions `rns_rdx_101..135`, Baby Audio, Cableguys, Chow Tape, Arturia, Audio Damage, FabFilter, etc.). Consult this directory whenever searching for downloaded plugin binaries, installers, or manuals.
8. **Dropbox Gear & Hardware Archive (`C:\_\Dropbox\Gear`)**:
   - The master hardware repository for synthesizer and sampler service manuals, schematics, owners manuals, SysEx banks, and hardware controller definitions is permanently located at [`C:\_\Dropbox\Gear`](file:///C:/_/Dropbox/Gear).
   - Contains authentic factory service manuals, circuit teardowns, and schematic notes (e.g. Roland Alpha Juno 2, MKS-50, S-330, JP-8000, TR-626, Behringer BCR/BCF templates, Lemur scripts, Max4Live editors). Consult this directory whenever searching for hardware service notes, schematics, component values (IR3R05, IR3109, filter cascades), or factory calibration trims.
9. **The Tower C++ Build Isolation Guardrail (Zero-Local-Build & Context Shield Law)**:
   - **Absolute Build Routing**: Never execute raw unshielded C / C++ build commands (`cmake --build`, `msbuild`, `cl.exe`, `ninja`) directly in The Astral Abyss context or in the root of `c:\_\Vault`.
   - **Mandatory Tower Target**: All C / C++ compilation, DSP test suites (`dsp_tests.exe`), and plugin packaging (`.vst3`) MUST ALWAYS be routed to **The Tower** (`C:\Dev\TheKlangSuite`) via one of the 3 approved channels:
     - **Channel 1 (Subagent Factory Worker)**: `invoke_subagent` with `Role="Tower C++ Builder"`.
     - **Channel 2 (Cross-Chat Data Bus)**: Asynchronous packet via `python tools/bus.py post --to TKS --tag BUILD_REQUEST`.
     - **Channel 3 (Headless Shielded Bridge)**: `python tools/context_bus_bridge.py build --target <target> --dispatch`.
   - **Zero Compiler Noise Invariant**: Raw compiler diagnostics, MSVC template expansion traces, and linker logs are strictly barred from the parent chat transcript; all output streams to physical disk file descriptors (`build.log`). On failure, the agent surfaces ONLY a 5-line surgical defect card with 1-click remediation.
10. **The Zero-Flicker Hidden Terminal Invariant**:
    - For any command, script, build, test, query, or utility where interactive user console input is not explicitly required, NEVER spawn visible command prompt, conhost, or Windows Terminal windows on the operator's desktop.
    - **Primary Execution Route**: ConnectRPC `RunCommand` on `language_server.exe` (`python tools/ls_probe.py run "<cmd>"`).
    - **Secondary Subprocess Shielding**: Whenever Python scripts execute subprocesses, ALWAYS pass `creationflags = 0x08000000` (`CREATE_NO_WINDOW`) and `startupinfo.wShowWindow = 0` (`SW_HIDE`).
    - **Forbidden Invocations**: Never use raw `os.system()` (spawns visible `cmd.exe`), never use `cmd.exe /c start` without `/b`, and never use PowerShell `Start-Process` without `-WindowStyle Hidden`.
11. **The Visual Pre-Flight Verification Invariant (Zero-Blind-Deploy Law)**:
    - Whenever plugin GUI components, layouts, faceplates, or styling are modified, the agent MUST NEVER deploy a binary or report completion blindly without visual verification.
    - **Mandatory Live Capture**: Run `python tools/capture_ui.py "<title>"` to capture the exact rendered desktop window to `<appDataDir>\brain\<conversation-id>\hastur_ui_proof.png`.
    - **5-Gate Visual Quality Audit**:
      1. **Text & Glyph Integrity**: Zero truncated strings (`...`), zero code-page mojibake (use clean ASCII arrows `->` or explicit UTF-8 constants).
      2. **Spatial Clearance**: Zero overlapping labels, colliding text readouts, or cut-off card headers.
      3. **Ergonomic Scale**: No rotary knobs smaller than 52px diameter; primary hero knobs must be $\ge 72$px.
      4. **Engineering Units & Precision**: All values must display clean engineering units (`kHz`, `Hz`, `ms`, `s`, `Oct`, `st`, `%`) with $\le 2$ decimal places (zero raw float spam like `0.2333572`).
      5. **Track & Contrast Legibility**: Fader tracks, slot grooves, and borders must be clearly visible against the card chassis background.
    - Only when all 5 gates pass is deployment and user notification authorized.

<!-- CACHE_ANCHOR: STATIC_PREFIX_END -->



