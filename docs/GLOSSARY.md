# The Klang Suite: Official Architectural Glossary & Taxonomy

> [!NOTE]  
> This glossary is the single source of truth for terminology across UI design, synthesis audio architecture, and Git release lifecycles in The Klang Suite. All documentation, code reviews, and JSON data schemas must adhere to these definitions.

---

## 1. UI Surface & Spatial Hierarchy

| Term | Scope | Definition & Industry Standard | The Klang Suite Implementation |
| :--- | :--- | :--- | :--- |
| **Chassis** | Structural | The master application window/frame housing global headers, footers, visualizers, and the main canvas. | The top-level `FarmerEditor` / `PlanterEditor` window (800x600 to 4K resizable). |
| **Page** | View | A full-canvas surface swap. Swapping pages replaces the displayed cards while maintaining global headers and audio playback. | `FarmerEditor` Page 1 ("Main Synth"), Page 2 ("FX Rack 1-4"), Page 3 ("FX Rack 5-8"). |
| **Card** | Visual UI | A rectangular visual UI container with an accent color, title bar, background styling, and slot grid. | Cards 1–8 in Farmer; Cards 1–8 in Planter (defined structurally in `assets/layouts/*.json`). |
| **Module** | Audio DSP | The functional audio-processing unit. A Module generates or alters sound; a Card is the visual UI that exposes it. | S&H Filter, Carrier 1, Master Limiter. (One Card controls one Module; Callouts can also control Modules). |
| **Slot** | Layout | A discrete coordinate inside a Card's grid where a control widget resides. | Row/column cells defined in `tkf_layout.json` (e.g. `row: 0, col: 1`). |
| **CalloutBox / Popover** | Transient UI | A temporary floating mini-panel anchored to a specific trigger badge or control, launched by click or right-click. | `juce::CallOutBox` used for Planter Master Limiter, Snap Point menus, and Color Pickers. |

---

## 2. Interactive Controls & Widgets

| Term | Visual Style | Interaction & Behavior | The Klang Suite Implementation |
| :--- | :--- | :--- | :--- |
| **Slider** *(Meter Slider)* | Horizontal recessed trough with filled color bar | Dragging horizontally or vertically fills the bar; embedded label on left, value on right. | Primary signature widget across Cards 1–8 (arcade HP meter style). Under the hood: `RotaryKnobSlider` class. |
| **Knob** | Circular rotary dial | Vertical drag rotates pointer/indicator around pivot. | Used in floating Callout mini-cards (Planter Limiter) and hardware-inspired controls. |
| **Fader** | Vertical linear slide potentiometer | Vertical axis drag exclusively reserved for mixing channel levels and summing. | Reserved for mixer channel strips (Card 8 channel levels). |
| **Selector** | Text badge / pill stepper | Clicking opens dropdown or cycles through discrete choice options. | Waveform shapes, MIDI tracking modes, filter types (`DiscreteSelector`). |
| **Toggle** | 2-state latching button / LED | Binary click toggles state between on/off (`0.0` or `1.0`). | Header `TOOLTIPS`, `BYPASS` switches, modal toggles. |

---

## 3. Synthesis & Audio DSP Taxonomy

| Term | Domain | Definition & Industry Standard | The Klang Suite Implementation |
| :--- | :--- | :--- | :--- |
| **Carrier** | FM Audio | An operator whose audio output is **heard directly** in the mix. | Carrier 1 & Carrier 2 (audible sound generators feeding the mixer). |
| **Modulator** | FM Audio | An operator whose audio output is **inaudible directly**, but modulates the phase/frequency of a Carrier. | Modulator 1 & Modulator 2 (generating metallic harmonic sidebands). |
| **Operator** | FM Audio | The complete FM synthesis building block: Oscillator + Dedicated Envelope Generator + Level Scaling. | Chowning / Yamaha DX7 standard architecture. |
| **Engine** | Master Processor | The master audio container managing voice allocation, MIDI event parsing, and audio bus routing. | `TheKlangFarmerAudioProcessor`, `TheKlangPlanterAudioProcessor`. |
| **Voice** | Audio Instance | A single independent sound execution instance that renders one drum hit (tracks active phase, envelope state, filter memory). | Individual polyphonic / monophonic voice renderers in DSP code. |
| **DSP Block** | Algorithm | The C++ class implementing a specific mathematical audio routine. | `juce::dsp::LadderFilter`, `TbdAudio::FastMath`, `SvfFilter`. |
| **Klang-Brain** | Generative Engine | 4-mode algorithmic sequencer & modulation generator (Fugue Counterpoint, Matriceal Polymeter, Stage Accumulators, Turing LFSR) with Continuous and DSI Pro 2 Triggered Key-Advance modes. | Deployed as rhythm trigger engine in TKR-1 and multi-lane stepped/continuous modulation source in TKF. |

---

## 4. DevOps, Git & Release Lifecycles

| Term | Mechanism | Analogy | The Klang Suite Protocol |
| :--- | :--- | :--- | :--- |
| **Commit** | Local Git record | Checkpoint save on local drive | Atomic, descriptive commit (`feat:`, `fix:`, `docs:`) created via `git commit`. |
| **Push** | Network upload | Syncing local save files to cloud | `git push origin <branch>` to publish local commits to GitHub. |
| **Branch** | Isolated timeline | Parallel universe for experimentation | `main` (stable production), `<version>-dev` (e.g. `0.3.2-dev` active milestone). |
| **Tag** | Immutable pointer | Stamping an official serial number on a build | Annotated Git tag (`git tag -a v0.3.1 -m "Release v0.3.1"`) frozen on one commit forever. |
| **Version (SemVer)** | Numerical standard | Vehicle model year & trim | `MAJOR.MINOR.PATCH` (`v0.3.2`): Breaking changes = Major; Features = Minor; Fixes = Patch. |
| **Milestone** | Project container | The sprint checklist | Roadmap container in `docs/BACKLOG.md` grouping features and fixes targeting a version. |
| **Release** | Public distribution | Placing boxed product on retail shelves | Published GitHub Release bundled with installer artifacts (`.vst3`, `.exe`, `.pkg`) and changelogs. |

> [!TIP]
> **The "Silent Git Tag" Policy**: When a milestone consists strictly of internal architectural cleanup, developer tooling, or schema refactors with zero user-facing sound/UI changes (e.g. `v0.3.2`), we stamp and push an annotated Git Tag (`git tag -a vX.Y.Z`) and merge to `main`, but do not publish a public GitHub Release entry. This preserves clean SemVer and reproducible builds while preventing spurious "Update Available" notifications in end-user DAWs.

---

## 5. Ecosystem Product Acronym Registry & Collision Guardrails

To prevent identity collisions across codebases, logging tags (`TKS_LOG`), asset files, and user documentation, every software plugin, standalone app, and hardware spin-off in The Klang Suite is assigned a strict, immutable 3-letter acronym (`TK<X>`):

| Acronym | Product Name | Category | Scope / Role |
| :--- | :--- | :--- | :--- |
| **`TKS`** | **The Klang Suite** | **Umbrella / Suite** | Master project repository, shared utility libraries, and developer logging (`TKS_LOG`). *RESERVED EXCLUSIVELY FOR THE SUITE.* |
| **`TKF`** | **The Klang Farmer** | Instrument Plugin | Dual-FM percussion and bass synthesizer. |
| **`TKP`** | **The Klang Planter** | Instrument Plugin | Multi-layer percussive sound machine. |
| **`TKM`** | **The Klang Mill** | FX Plugin / Sub-Rack | 1x6 modular multi-effects pedalboard rack. |
| **`TKE`** | **The Klang Editor** | Standalone Desktop App | JSON schema, theme calibration, and curve inspection workstation. |
| **`TKB`** | **The Klang Box** | Embedded Hardware | Standalone hardware synthesizer (Daisy Seed / Teensy 4.1 / dadamachines TBD-16). |
| **`TKR`** | **The Klang R1** | Instrument Plugin | 7-voice Electribe ER-1 tribute rhythm synthesizer. |
| **`TKC`** | **The Klang Cultivator** | Utility Plugin | Standalone MIDI CC / CV / MPE modulation generator rack. |

> [!WARNING]
> **Acronym Collision Guardrail**: No new plugin or spin-off product may adopt an acronym that collides with registered letters (`S`, `F`, `P`, `M`, `E`, `B`, `R`, `C`). `TKS` is strictly forbidden for individual plugins.
> 
> **The Klang Box (`TKB`) vs. The Klang-Brain**: `TKB` is officially reserved for **The Klang Box** embedded hardware platform. The generative algorithmic modulation and sequencing engine is designated **The Klang-Brain** (internal engine code `TKG` - *The Klang Generator / Genome* if an acronym is required), preserving `TKB` for hardware.

---

## 6. Decision & Engineering Review Frameworks

| Framework / Term | Category | Definition & Classical Foundation | The Klang Suite Implementation & Protocol |
| :--- | :--- | :--- | :--- |
| **`PCDA`** | Review Protocol | **Pros, Cons, Devil's Advocate**. A 3-dimensional evaluation framework designed to counter confirmation bias by pairing positive/negative tradeoffs with active contrarian stress-testing. | Invoked when the user or agent requests a rapid decision breakdown (`"give me the PCDA on this"`). Structured with an explicit Devil's Advocate / Antithesis breakdown. *Deliberately designated PCDA rather than PCA to prevent collision with Principal Component Analysis (DSP/ML) and Printed Circuit Assembly (hardware).* |
| **`Dialectic`** *(Hegelian Dialectic)* | Decision Model | The classical philosophical method of investigating truth through the opposition of contradictory assertions: **Thesis $\rightarrow$ Antithesis $\rightarrow$ Synthesis**. | Applied systematically during architectural forks, RFCs, and triage sprints: <br>1. **Thesis** = The core proposal / Option (Pros & intended benefits). <br>2. **Antithesis** = The Devil's Advocate (Cons, failure modes, real-world precedent, maintenance debt). <br>3. **Synthesis** = The Pragmatic 80/20 Middle Ground (extracting 80% of the value with 20% of the complexity). |
| **`Thesis`** | Dialectic Component | The foundational proposal, design hypothesis, or architectural direction. | Presented as Option A/B/C in RFCs, PLANs, or Active Workshop cards. |
| **`Devil's Advocate [Antithesis]`** | Dialectic Component | The adversarial counter-argument tasked with exposing unstated assumptions, fatal edge cases, token bloat, and cognitive debt. | Rendered by the `contrarian` subagent or formatted under the heading `Devil's Advocate [Antithesis]` in chat and sidecar reviews. |
| **`The 80/20 Resolution [Synthesis]`** | Dialectic Component | The pragmatic, battle-tested middle ground reconciling the Thesis with the Antithesis. | The mandatory conclusion of all PCDA reviews, preventing endless philosophical debate and driving decisive factory execution. |
| **`Re-sequence / Rejigger`** | Workflow Action | Topologically auditing, consolidating, and re-ordering roadmap and triage topics based on prerequisite dependencies. | Triggered via `/audit-reorder`, `/rejigger`, `/re-sequence`, or the `[ 🔄 Re-sequence Flights ]` header button in N'kai sidecars. |
| **`2-Pass Convergence Gate`** | Execution Protocol | Multi-agent iterative reflection pattern enforcing a strict 2-pass cap (Pass 1: Flash PCDA exploration $\rightarrow$ Pass 2: Pro topological synthesis). | Prevents AI echo chambers, infinite review looping, and quota exhaustion while ensuring 100% prerequisite alignment before factory execution. |

> [!TIP]
> **PCDA Conversational Format**: When the user requests a "PCDA" on any topic, agents format the response systematically:
> 1. **Pros [Thesis]**: What works, immediate velocity, and user value.
> 2. **Cons [Thesis Friction]**: Known drawbacks, implementation effort, and direct costs.
> 3. **Devil's Advocate [Antithesis]**: Structural failure modes, hidden maintenance debts, and historical production disasters.
> 4. **The 80/20 Resolution [Synthesis]**: The recommended pragmatic architecture extracting maximum value with minimum complexity.

---

## 7. Note Triage, Workflows & Queue Management (The Mailbox vs. The Mailbag)

| Term | Domain | Definition & Purpose | The Klang Suite Implementation |
| :--- | :--- | :--- | :--- |
| **`Mailbox`** 📮 | Physical Storage | The physical Obsidian directory where raw, unorganized quick notes are dumped from mobile or desktop. | `TheKlangVault/Inbox/` (and archived to `TheKlangVault/Inbox/Archive/`). |
| **`Mailbag`** 🎒 | Interactive Session | The persistent N'kai interactive triage session board and historical ledger. | `morning_mail_inbox.html` & `.agents/pipeline/mailbag/mail_queue.json`. |
| **`Persistent Mailbag Queue`** | Session Ledger | A decoupled state ledger tracking note triage statuses, priority levels, and metadata independently of physical file movements. | `.agents/pipeline/mailbag/mail_queue.json`, ensuring archiving in Obsidian does not wipe active sidecar state. |
| **`Topological Rejigger Intercept`** | Dependency Gate | Dynamic dependency-checking logic that flags when a high-priority (`P0`) note depends on an unbuilt prerequisite. | Suggestion chip rendered on Mailbag cards: `[ 🔄 Slot into Topological Order ]` vs `[ ⚡ Keep at Top ]`. |
| **`Plans Queue`** | Agent Orchestration | A persistent queue managing multi-plan staging, execution sequences, and active execution pointer. | `.agents/pipeline/plans/queue.json`, coordinating plan transitions from `drafts/` to root `PLAN.md` to `completed/`. |
| **`Stateless Atomic Rollback`** | Fault Recovery | A multi-tier rollback mechanism restoring previous states via immutable pre-transaction receipts. | Implemented in `check_mail.py --rollback`, `audit_reorder.py --rollback`, and `clean-plan --rollback`. |
