# ToadTracker Modulation Architecture: Per-Instrument `QuadraMod` & 32-Channel Git-Branch `OctoMod` Performance Engine

**Document Status:** CANONICAL TOADTRACKER SYSTEM SPECIFICATION 📜  
**Target Milestone:** ToadTracker v0.2.0 (`TT-v0.2.0`) & The Klang Suite (`TKS`)  
**Ecosystem Standard:** Dual-Layer Local/Global Modulation Standard  
**Authors:** Antigravity x Cody G. (Klang Industries / ToadTracker Audio Division)  
**Date:** 2026-10-18  

---

## 1. Executive Summary & Philosophy

In modern tracker workflows, modulation exists on two distinct spatial layers:
1. **Local Voice Modulation (Per-Instrument)**: Rapid, tight, cycle-accurate modulations that define the acoustic timbre of a single sound (e.g. transient pitch drops, hat decay envelopes, filter wobbles).
2. **Global Performance Modulation (Project-Wide Macro Control)**: Multi-destination macro sweeps (inspired by the **Elektron Analog Four / Analog Rytm Performance Mode**) that allow a performer to morph dozens of parameters across multiple tracks simultaneously using dedicated hardware encoders.

ToadTracker (`TT`) formalizes this into the **Dual-Layer Modulation Architecture**:
* **`QuadraMod` (Local)**: Every single instrument contains **4 dedicated local modulators** (`MOD 1..4`), faithfully expanding the Dirtywave M8 `INST. MODS` standard.
* **`OctoMod` (Global)**: **32 global 1-to-8 fan-out macro hubs** running at the project level, completely isolated in project state, configurable per **Git Branch**, with instant **Historic Time-Travel Loading** from any commit in git history.

---

## 2. Layer 1: Local `QuadraMod` (Per-Instrument)

Every instrument in ToadTracker (Instruments `00` through `FF`) has a dedicated sub-screen (`INST. MODS`):

```
INST. MODS 70
MOD1  [02 DRUM ENV]      MOD3  [03 LFO]
DEST  01 VOLUME          DEST  0A PAN
AMT   FF ─────────────── AMT   BF ───────────────
PEAK  00                 OSC   09 DRUNK
BODY  00                 TRIG  01 RETRIG
DEC   30 ──              FREQ  10 ─

MOD2  [02 DRUM ENV]      MOD4  [03 LFO]
DEST  04 MOD 2           DEST  03 MOD 1
AMT   FF ─────────────── AMT   10 ──
PEAK  00                 OSC   08 RANDOM
BODY  00                 TRIG  01 RETRIG
DEC   88 ───────         FREQ  10 ─
```

### The 7 Canonical `QuadraMod` Types

Cross-referenced directly with the Dirtywave M8 manual and expanded with Klang chiptune/slew heritage:

| Type ID | Modulator Name | Parameters | Core Behavior & Sonic Pedigree |
| :---: | :--- | :--- | :--- |
| **`00`** | **`NONE`** | *(Bypassed)* | Zero CPU execution. |
| **`01`** | **`AHD ENV`** | `DEST`, `AMT`, `ATT`, `HLD`, `DEC` | Classic Attack-Hold-Decay envelope with gate sync. |
| **`02`** | **`DRUM ENV`** | `DEST`, `AMT`, `PEAK`, `BODY`, `DEC` | Empirical M8 drum decay: `PEAK` (transient strike boost), `BODY` (pre-decay hold), `DEC` (exponential fall). |
| **`03`** | **`LFO`** | `DEST`, `AMT`, `OSC`, `TRIG`, `FREQ` | Multi-shape LFO with sync/free modes, reset triggers, and Brownian random walk. |
| **`04`** | **`TRACKING`**| `DEST`, `AMT`, `SRC`, `LOW`, `HIGH` | Expressive acoustic tracking from Key, Velocity, Pitch Bend, Mod Wheel, or Channel Aftertouch. |
| **`05`** | **`REMAP`** | `DEST`, `AMT`, `SRC`, `TABLE`, `SKEW` | Transfers any input source through a 16-point lookup table or mathematical transfer function. |
| **`06`** | **`SLOPE`** | `DEST`, `AMT`, `RISE`, `FALL`, `CURVE`| West Coast Make Noise Maths style looping slew generator with End-of-Fall (EOF) trigger pulses. |
| **`07`** | **`MML STEP`** | `DEST`, `AMT`, `TICKS`, `LOOP`, `SEQ` | Deflemask / MML chiptune stepped macro sequence with sustain loops and release escapes. |

#### Deep LFO Oscillator Shapes (`OSC`):
`00: TRIANGLE` &bull; `01: SINE` &bull; `02: RAMP DOWN` &bull; `03: RAMP UP` &bull; `04: EXP DOWN` &bull; `05: EXP UP` &bull; `06: SQUARE` &bull; `07: RANDOM (S&H)` &bull; **`08: DRUNK (Brownian random-walk wander)`**.

#### LFO Trigger Modes (`TRIG`):
* `00: FREE`: Free-running continuous oscillator (ignores note events).
* `01: RETRIG`: Resets phase to $0^\circ$ on every note trigger.
* `02: HOLD`: Latches and holds the LFO's current voltage level on note strike (converts noise to stepped per-note pitch).
* `03: ONCE`: Executes a single half-cycle or full-cycle like a specialized function envelope.

#### Recursive Self-Modulation:
Any `MOD` can target another `MOD` on the same instrument (`MOD 4` modulating `MOD 1 FREQ`, or `MOD 2` modulating `MOD 2 DEC` for exponential self-accelerating decays), enabling modular chaos within a single tracker instrument.

---

## 3. Layer 2: Global `OctoMod` (32-Channel Performance Engine)

While `QuadraMod` is local to each instrument, **`OctoMod`** is the master performance brain of the song:

```
┌────────────────────────────────────────────────────────────────────────┐
│ OCTOMOD 01 (MASTER PERFORMANCE MACRO)                                  │
│ Source: [ HARDWARE ENCODER 1 (TBD-16 PAGE 1A) ]                       │
├────────────────────────────────────────────────────────────────────────┤
│ 1: Inst 01 (Kick)    ──► Pitch Bend Decay     Amt: -35%  [Profile B]  │
│ 2: Inst 02 (Snare)   ──► Filter Cutoff        Amt: +70%  [Profile D]  │
│ 3: Inst 03 (Bass)    ──► VCA Drive            Amt: +55%  [Profile E]  │
│ 4: Inst 04 (Pad)     ──► Stereo Width         Amt: +100% [Profile B]  │
│ 5: FX-02 (KlangWarp) ──► Fold Depth           Amt: +40%  [Profile E]  │
│ 6: FX-07 (KlangDelay)──► Feedback Runaway     Amt: +85%  [Profile F]  │
│ 7: Mixer Track 1     ──► Volume Duck          Amt: -20%  [Profile B]  │
│ 8: Master Reverb     ──► Shimmer Injection    Amt: +60%  [Profile M]  │
└────────────────────────────────────────────────────────────────────────┘
```

### Key Specifications:
* **Capacity:** Exactly **32 independent `OctoMod` hubs** (`OCTOMOD 01` .. `OCTOMOD 32`).
* **Fan-Out Ratio:** **1-to-8**. Each `OctoMod` takes 1 master control source and distributes it to **up to 8 independent destinations** anywhere in the project.
* **Per-Destination Scaling:**
  * Bipolar Amount Knob ($-100\% \dots +100\%$, Profile B S-Curve with center deadzone).
  * Min Clamp / Max Clamp bounds.
  * Inversion switch ($\pm$).
* **Sources:**
  1. `Macro Knobs 01..32` (Physical encoders on dadamachines TBD-16 or GUI).
  2. `Global LFOs 1..8` (Project tempo-synced master modulators).
  3. `Audio Envelope Followers` (Sidechain dynamic extraction).
  4. `MIDI CC / Hardware Expression Pedals`.

### The Performance Screen (`VIEW_PERF`):
In ToadTracker, pressing `[Shift + Edit]` on the song screen opens **Performance Mode** (identical in feel to the Elektron Analog Four):
* Displays 8 massive macro knobs on screen.
* 4 Banks of 8 knobs ($4 \times 8 = 32\text{ OctoMods}$), switched with the D-Pad or TBD-16 bank buttons.
* Grabbing a single knob simultaneously morphs drums, opens bass filters, and ramps delay feedback into an ecstatic buildup!

---

## 4. Git-Branch Performance States & Time-Travel Macro Ingest

Because ToadTracker uses an immutable, git-backed filesystem for project storage, **Performance Mode is fully decoupled from note data**:

### 4.1 Project Directory Structure
```
MyProject.toad/
├── .git/                      <-- Complete version history
├── song.json                  <-- Patterns, phrases, and note data
├── instruments/               <-- Instruments 00..FF (with QuadraMods)
└── perf/
    ├── octomod.json           <-- The active 32 OctoMod configurations
    └── scenes/
        ├── drop_buildup.json  <-- Saved macro scene presets
        └── dub_meltdown.json
```

### 4.2 Branch-Specific Performance Profiles
Because `perf/octomod.json` is versioned in git, you can branch your performance setups without touching the underlying song:
* **`git checkout main`**: The standard studio album mixdown macros.
* **`git checkout club-live`**: Aggressive club performance macros (heavy low-end ducking, instant strobe delays, filter drops).
* **`git checkout ambient-chill`**: Glacial ambient macros (gentle volume swells, infinite freeze reverbs, tape wobble).

### 4.3 Time-Travel Macro Ingest (`git cherry-pick` for Performance)
Suppose you designed the *perfect* dub feedback macro setup 3 weeks ago on commit `a7f9c2d`, but you’ve rewritten the song since then. 

ToadTracker provides **Time-Travel Ingest** directly in the UI or CLI:

```bash
# Ingest an OctoMod setup from any commit or branch in history
tt perf load --from a7f9c2d:perf/octomod.json --slot 01..08

# Or load from another branch directly:
tt perf load --from club-live:perf/scenes/dub_meltdown.json
```

* **Non-Destructive**: It only updates the macro routing matrix in `perf/octomod.json`.
* **Zero Note Collisions**: Patterns, phrases, and instrument audio data are completely untouched.
* **Live Auditioning**: You can audition performance scenes from past commits on the fly during a live set!

---

## 5. Memory & Real-Time Performance Guarantee

* **Lock-Free Audio Accumulation**: In C/C++, all 32 OctoMods and per-instrument QuadraMods calculate their targets into static modulation accumulators per audio block ($O(1)$ complexity, zero heap allocations).
* **Deterministic Memory Footprint**:
  * Each `QuadraMod` = $16\text{ bytes} \times 4 = 64\text{ bytes per instrument}$.
  * 128 Instruments $\times 64\text{ bytes} = 8.192\text{ KB}$ total RAM.
  * 32 `OctoMod` hubs $\times 8\text{ destinations} \times 8\text{ bytes} = 2.048\text{ KB}$ total RAM.
  * **Grand Total: $<11\text{ KB}$ of memory**, fully operable on microcontrollers like the RP2040, RP2350, and ESP32-P4 with zero frame drops.
