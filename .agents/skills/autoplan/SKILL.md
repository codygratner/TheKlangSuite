---
name: autoplan
description: Executes the zero-build Component Crucible Planning Engine. Ruthlessly chainsaws fluff, discovers SOTA improvements, audits acoustics and spatial ergonomics via interactive sensory labs, and locks cryptographic contracts before touching C++ or compiler tools. Triggers on /autoplan, /autoplan-loop, "autoplan", "autoplan loop", or "autonomous planning loop".
---

# 🧠 The Autoplan 2.0 Loop (`/autoplan`)
### *The Grounded Component Crucible & SOTA Verification Engine*

> **Origin & Lineage**: Born from the Mayor Toad Paper-Shredding Smoke-Break Protocol, hardened by the Hastur Envelope & DCO Retrospectives, and grounded in academic verification literature (*Metamorphic Testing*, *Property-Based Shrinking*, and *Grounded LLM Oracles*).  
> **Core Invariant**: Code written prematurely carries an unbearable tax of sunk-cost bias, compiler friction, and architectural churn. The **Autoplan Loop** enforces a strict **Zero-Build Airgap**—deliberating, chainsawing, expanding, and auditioning a component until the specification is undeniable, before a single line of production C++ is touched.

---

## 💥 The Fundamental Law: "Verification at the Layer That Fails"

The joint adversarial research by Claude Opus 5.5 and Gemini 4 Argon proved:
> *The original planning rituals put their heaviest verification at the layer that didn't fail. Mathematical transfer functions were sound; the integration seams between them broke:*
> 1. Call-graph double-clocking across stereo channels.
> 2. Missing APVTS parameter declarations.
> 3. Misrouted parameter setters in voice dispatch.
> 4. Fixed filter coefficients drifting across sample rates.
> 
> *Sample-level JavaScript ↔ C++ parity does not catch a single one of these integration bugs. Autoplan 2.0 eliminates dual-language CI maintenance and focuses 100% of its verification armor on the real plugin boundary, single-source code generation, and metamorphic relations.*

---

## 🏆 The Asymmetric Cognitive Funnel (ACF-5)

Autoplan 2.0 replaces the legacy 16-pass symmetric ritual with **4 focused LLM operations + 1 empirical probe + 1 compiler lock**:

```text
┌────────────────────────────────────────────────────────────────────────────────────────┐
│                        THE ASYMMETRIC COGNITIVE FUNNEL (ACF-5)                         │
├─────────┬──────────────────────┬──────────────────────┬────────────────────────────────┤
│ STAGE   │ COGNITIVE STANCE     │ MODEL TIER           │ OBJECTIVE / INVARIANT          │
├─────────┼──────────────────────┼──────────────────────┼────────────────────────────────┤
│ 1. FRAME│ Strictly Convergent  │ Tier 2 (Flash)       │ Read ledger; lock I*, control  │
│         │ (Zero Blue-Sky)      │                      │ budget & closed decisions.     │
├─────────┼──────────────────────┼──────────────────────┼────────────────────────────────┤
│ 2. DIVERGE│ Strictly Divergent │ Tier 1 (Argon/Claude)│ 1 pass, free prose: 3–5 novel  │
│         │ (Prose-First Alpha)  │                      │ mechanisms + additions UNIFIED!│
├─────────┼──────────────────────┼──────────────────────┼────────────────────────────────┤
│ 3. CUT  │ Strictly Reductive   │ Tier 2 (Flash)       │ Chainsaw: delete by default.   │
│         │ (Chainsaw Filter)    │                      │ Additions must justify audio.  │
├─────────┼──────────────────────┼──────────────────────┼────────────────────────────────┤
│ 4. ATTACK│ Grounded Red Team   │ Claude Opus 5.5      │ FRESH CONTEXT: sees spec only! │
│         │ (Executable Oracles) │ (via Claude Bridge)  │ Parameter Effect Matrix sweep. │
├─────────┼──────────────────────┼──────────────────────┼────────────────────────────────┤
│ 5. PROBE│ Empirical Sandbox    │ WebAudio / Python    │ Real ear audition in 2 sec.    │
│         │ (Zero Token Churn)   │ (No Token Churn)     │ [COPY CONTRACT YAML] export.   │
├─────────┼──────────────────────┼──────────────────────┼────────────────────────────────┤
│ 6. LOCK │ Pure Compiler        │ CodeGen Engine       │ contracts/<plugin>.yaml ➔      │
│         │ (Single SSoT)        │ (tools/codegen_...)  │ <Plugin>ParameterIds.h & Layout│
└─────────┴──────────────────────┴──────────────────────┴────────────────────────────────┘
```

---

## 🧭 The 3 Sizing Tiers (Risk-Proportional Scaling)

Never burn deep planning budgets on trivial plumbing. Route every feature to its exact sizing tier:

| Tier | When to Use | Execution Path | Token Budget |
| :--- | :--- | :--- | :--- |
| **`micro`** | Renaming parameters, taper adjustments, single knob wiring fixes. | `Frame` $\to$ `Cut` $\to$ `Lock` | $< 8\text{k}$ tokens |
| **`standard`** | New controls, secondary filter stages, envelope shapes. | `Frame` $\to$ `Diverge` $\to$ `Cut` $\to$ `Attack` $\to$ `Lock` | $< 35\text{k}$ tokens |
| **`deep`** | Novel non-linear topologies, chaotic feedback loops, new DSP models. | Full 6-Stage ACF-5 Funnel with mandatory WebAudio `Probe` | $< 55\text{k}$ tokens |

---

## 🏷️ Strict Evidence Discipline

In dialectical reviews, theoretical claims masquerading as verified engineering are strictly prohibited. Every acoustic and architectural claim must carry an evidence tag:

- **`[MEASURED]`**: Quantified via FFT, Welch PSD, or automated test suites (`dsp_tests`, `headless_plugin_runner`).
- **`[DERIVED]`**: Analytically proven from circuit transfer functions, state-variable filter math, or formal proofs.
- **`[ASSERTED]`**: Subjective design preference, aesthetic intention, or theoretical conjecture.  
  *(Rule: Nothing labeled `[ASSERTED]` can gate a contract lock or be deployed without sensory audition).*

---

## 🔒 The Grounded Verification Gates

Autoplan 2.0 enforces 4 deterministic verification gates:

### Gate 0: APVTS Parameter Contract Parity (`tools/codegen_apvts.py`)
- Single Source of Truth: `contracts/<plugin>.yaml`.
- Generates `Source/<plugin>/<Plugin>ParameterIds.h` (typed `constexpr` identifiers) and `Source/<plugin>/<Plugin>ParameterLayout.h`.
- Embedded into `tools/context_bus_bridge.py`: any parameter drift between C++ and YAML contracts halts Tower builds in 40ms.
- **Result: Ghost controls and unregistered parameters become C++ compile errors.**

### Suite 41: Parameter Effect Matrix (`dsp_tests`)
- Automated sweep across all parameters asserting that each control produces measurable change *only in its declared domain*:
  - `rms`: Volume, levels, drive, envelope sustain/decay.
  - `pitch`: Semitone offsets, multipliers, octave shifts, glide.
  - `timbre`: Waveform morphs, filter cutoffs, resonance, APF bite.
- Catches dead controls and misrouted voice setters in $\le 50\text{ms}$.

### Suite 42: Incident Replay Suite (`dsp_tests`)
- Permanent seeded regression gauntlet covering every defect from past post-mortems:
  - **INC-01**: Timebase monotonicity (zero double-clocking in stereo processing).
  - **INC-02**: Parameter registration & bipolar range bounds ($[-1.0, 1.0]$).
  - **INC-03**: Voice parameter routing & sync retention.
  - **INC-04**: Sample-rate coefficient scaling (zero hardcoded constants at 44.1, 48, 96 kHz).
- **Rule: A verification gate only counts once it has caught its originating bug!**

### Suite 43: Metamorphic DSP Relations (`dsp_tests`)
- **MR1 (Block-Size Invariance)**: Arbitrary buffer sizes (32, 64, 128, 512) render identically (within $10^{-6}$ float precision).
- **MR2 (Sample-Rate Stability)**: 44.1, 48, and 96 kHz execution produces bounded, finite output without NaN or Inf.
- **MR3 (All-Pass Unimodularity)**: Pure APF stages preserve energy ($|H(e^{j\omega})| \approx 1.0$).
- **MR4 (Silence Invariance)**: Silence in $\implies$ exact silence out ($f(0) \equiv 0$, zero denormals or runaway).

---

## 🎛️ When to Use Autoplan vs. Other Skills

| Skill | Purpose | Code Written? | Cognitive Engine | Output |
| :--- | :--- | :---: | :--- | :--- |
| **`/autoplan`** | Surgical component & feature distillation | ❌ No | Asymmetric Cognitive Funnel (ACF-5) | Sensory lab, YAML contract, CodeGen lock |
| **`/plan`** | Global multi-wave project roadmap & DAG sequencing | ❌ No | Dual-Cycle 4-Pass Chainsaw + Walking Skeleton DAG | Global `PLAN.md` (M0..M3 waves) |
| **`/crucible-plan`** | Full 7-stage spec-driven engineering lifecycle | ⚠️ Late stage | PCDACore 4.2.0 (Step A Lock $\to$ Step B Build) | Production code, verified tests, receipt |
| **`/post-mortem`** | SRE & milestone incident retrospective | ❌ No | 4-Track Why Tree + Severity Matrix | Machine rules (`Agent Rules/`) & seeded tests |
