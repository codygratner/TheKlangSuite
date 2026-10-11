# N'kai "Rambles" Ingestion Engine Specification (Topic 2, Flight 1)

## 1. Executive Summary & Problem Space

### 1.1 The Stream-of-Consciousness Ramble Intake Problem
Creative synthesizer developers, sound designers, and musicians rarely compose formal, structured Jira tickets or well-formed architectural specifications when inspiration strikes. Instead, ideas are captured mid-commute, backstage, or in the middle of the night as raw voice memos, unformatted phone scratchpads, or stream-of-consciousness brain dumps in an Obsidian vault (`TheKlangVault/Inbox/`).

Historically, these notes suffer from three severe failure modes:
1. **Cognitive Ingestion Friction**: Unstructured paragraphs contain high-value intuition intertwined with conversational filler (e.g., *"You seemed to be doing ok with taking a long ramble... Is this a good workflow... But I kinda think that's why I wanted nkai"*). Ingesting them manually consumes disproportionate attention.
2. **Backlog Pollution vs. Premature Archival**: Dumping unstructured notes directly into `docs/BACKLOG.md` pollutes the tracking index with vague tickets. Conversely, leaving them in `Inbox/` causes them to be ignored, while prematurely archiving them loses valuable creative momentum.
3. **Compound Idea Entanglement**: A single ramble often conflates multiple orthogonal technical concerns (for example, in `Rambles.md`, discussing overall note intake workflows in paragraph 1, followed by note granularity trade-offs in paragraph 2; or in `Effects and Oscillators and More.md`, listing dozens of vintage drum chips, modular synth blocks, and macro hierarchies).

The **N'kai "Rambles" Ingestion Engine** (`tools/ramble_ingest.py`) provides an automated, zero-external-dependency parsing pipeline that ingests raw stream-of-consciousness text, decomposes compound notes into atomic ideas, and synthesizes crisp, standardized **3-Bullet Decision Cards**.

---

## 2. The 3-Bullet Decision Card Standard

Each extracted idea is distilled into an atomic `RambleCard` adhering to a strict 3-bullet schema:

```markdown
### 🎙️ [Pillar] Title of Synthesized Idea (NoteName #Index)
- 🎯 **Core Intent**: 1-sentence synthesis of the creative goal.
- 🛠️ **Proposed Mechanism**: Target repo, hardware target, or module architecture (TKS, TT, NK, TBD-16, TKR).
- 🚦 **Action Route**: Suggested triage recommendation ([ 📥 Backlog ], [ 🔬 Research ], [ 🪦 Graveyard ], [ ⚖️ PCDA ]).
```

### 2.1 The Three Fields
1. 🎯 **Core Intent**: A razor-sharp, single-sentence summary of *what* the author wants to achieve, stripping out conversational qualifiers (*"I was wondering if maybe we could..."* $\to$ *"Establish an automated ingestion pipeline for stream-of-consciousness voice memos"*).
2. 🛠️ **Proposed Mechanism**: The affected technical domain, hardware architecture, DSP block, or repository pillar:
   - `TKS` (The Klang Suite): Polyphonic synthesizers, VST3 plugins, DSP modules, filters, JUCE UI.
   - `TT` (ToadTracker): Embedded tracker engine, hardware groovebox, M8 telemetry, pattern sequencer.
   - `NK` (N'kai): Asymmetric sidecars, triage boards, prompt compilers, CLI bridges.
   - `TBD-16` (dadamachines tbd-16): ESP32-P4 / RP2350B dual-core hardware, 4-encoder OLED layouts.
   - `TKR` (The Klang Research): DSP teardowns, algorithmic proofs, competitive surveys, RFCs.
   - `AGY` (Antigravity): IDE extensions, slash commands, rules, subagents.
3. 🚦 **Action Route**: An initial heuristic routing classification based on the tone, confidence, and rhetorical structure of the input:
   - `[ 📥 Backlog ]`: Concrete, actionable feature or architectural enhancement with high clarity.
   - `[ 🔬 Research ]`: Exploratory inquiry, unverified algorithm, or open technical question requiring investigation.
   - `[ 🪦 Graveyard ]`: An idea identified as counter-productive, excessively complex, or intentionally rejected.
   - `[ ⚖️ PCDA ]`: High-impact architectural fork requiring a 4-lens Hegelian dialectic evaluation (Thesis, Friction, Antithesis, Synthesis).

---

## 3. Multi-Idea Paragraph Decomposition Heuristic

Real-world rambles rarely limit themselves to a single thesis. A user authoring a voice memo will seamlessly shift from one subsystem to another. The ingestion engine decomposes long inputs using deterministic multi-pass heuristics:

### 3.1 Structural Boundary Detection
The parser breaks input into candidate segments using:
1. **Frontmatter Stripping**: YAML frontmatter (`--- ... ---`) is parsed for top-level metadata (date, author, explicit tags) and removed from the body text.
2. **Double-Newline Paragraph Blocks**: Standard paragraph breaks (`\n\n+`) establish primary thematic units.
3. **Transition Cue Phrases**: Semantic boundary markers that signal topic shifts within or across paragraphs:
   - Transitional conjunctions: `"Also,"`, `"Another idea:"`, `"What about"`, `"Or,"`, `"On the other hand"`, `"Alternatively,"`.
   - Numbered or bulleted inventories: `- GoodHertzLossy`, `1.`, `2.`.
   - Rhetorical pivot questions: `"Can we try to..."`, `"Could this be..."`.

### 3.2 List Inventory Grouping
When a ramble contains an enumerated list (such as the 26-item inventory in `Effects and Oscillators and More.md`), the ingestor assesses whether the list represents:
- A single unified mega-proposal (e.g., *"Catalog expansion of 26 effects into categorized meta-containers"*).
- Multiple distinct sub-blocks (e.g., separating vintage drum algorithms from physical modeling and wavetable macros).
The heuristic synthesizes high-level cards representing the overarching structural proposal, while preserving the raw itemized text in the `raw_snippet` attribute for provenance.

### 3.3 Domain Pillar Keyword Matching
The parser evaluates the token distribution of each candidate segment against domain dictionaries:
- `TKS`: `"vst"`, `"synth"`, `"oscillator"`, `"filter"`, `"macro"`, `"sound"`, `"patch"`, `"preset"`, `"polyphon"`, `"juce"`, `"adsr"`, `"envelope"`, `"chorus"`, `"delay"`, `"reverb"`, `"distortion"`, `"flanger"`, `"phaser"`.
- `TT`: `"tracker"`, `"toad"`, `"m8"`, `"dirtywave"`, `"engine"`, `"tracktion"`, `"groovebox"`, `"step"`, `"pattern"`, `"song"`, `"table"`.
- `NK`: `"sidecar"`, `"nkai"`, `"bridge"`, `"subagent"`, `"prompt"`, `"triage"`, `"canvas"`, `"html"`, `"inbox"`, `"mailbag"`, `"autopsy"`.
- `TBD-16`: `"esp32"`, `"p4"`, `"tbd"`, `"tbd-16"`, `"rp2350"`, `"oled"`, `"encoder"`, `"hardware"`, `"gpio"`, `"teensy"`.
- `TKR`: `"rfc"`, `"math"`, `"dsp paper"`, `"teardown"`, `"aliasing"`, `"proof"`, `"academic"`.

### 3.4 Action Route Heuristics
- **Question / Exploratory Cues** (`RESEARCH` or `PCDA`): Contains `"?"`, `"wondering"`, `"should we"`, `"figure out"`, `"is this a good"`, `"try to"`, `"not sure"`. If it involves deep structural design, it routes to `PCDA`; otherwise `RESEARCH`.
- **Concrete Proposal Cues** (`BACKLOG`): Contains `"definitely"`, `"need to make"`, `"should have"`, `"add"`, `"implement"`, `"we can"`, `"must"`.
- **Skeptical / Rejection Cues** (`GRAVEYARD`): Contains `"bad idea"`, `"maybe not"`, `"stupid"`, `"slash"`, `"kill"`, `"waste of time"`.

---

## 4. Human-in-the-Loop Safety Invariant

> [!IMPORTANT]
> **Zero Premature Mutation Invariant**: Under NO circumstances should `tools/ramble_ingest.py` or the `/ramble` skill directly mutate `docs/BACKLOG.md`, `PLAN.md`, or C++ source code.

1. **Staged Queue Isolation**: Ingested cards are written either to stdout (`--chat`), stdout JSON (`--json`), or staged into `.agents/pipeline/mailbag/mail_queue.json` (`--add-to-queue`).
2. **Interactive Triage Confirmation**: Actual promotion of an idea to `docs/BACKLOG.md` requires an explicit user action via the N'kai Morning Mail board (`morning_mail_inbox.html`), where the user clicks `[ 📥 Backlog ]`, or an explicit affirmative response in chat.
3. **Provenance Preservation**: Every generated card retains a `source_note` link pointing back to the original Obsidian note, and optionally a `raw_snippet` quoting the exact source passage.

---

## 5. CLI Architecture & Interface (`tools/ramble_ingest.py`)

### 5.1 CLI Arguments & Options
```
usage: ramble_ingest.py [-h] [--file FILE] [--inbox] [--scan-archive]
                        [--chat] [--json] [--archive] [--add-to-queue]
                        [--dry-run] [--queue-file QUEUE_FILE]
```

| Flag | Argument | Description |
| :--- | :--- | :--- |
| `--file` | `<path>` | Process a single markdown ramble note. |
| `--inbox` | *None* | Batch scan all markdown notes in `TheKlangVault/Inbox/` (or fallback). |
| `--scan-archive` | *None* | Scan archived notes in `TheKlangVault/Inbox/Archive/`. |
| `--chat` | *None* | Print clean, formatted Markdown 3-bullet cards to stdout (terminal default). |
| `--json` | *None* | Output serialized JSON structure (`RambleExtractionResult`). |
| `--add-to-queue` | *None* | Convert cards to `MailItem` and append to `mail_queue.json`. |
| `--archive` | *None* | Move successfully processed notes from `Inbox/` to `Inbox/Archive/`. |
| `--dry-run` | *None* | Simulate operations without modifying disk (no moves, no queue writes). |
| `--queue-file` | `<path>` | Custom path to `mail_queue.json` (defaults to `.agents/pipeline/mailbag/mail_queue.json`). |

### 5.2 Terminal Output Sample (`--chat`)
```markdown
# 🎙️ N'kai Ramble Extraction Report
**Source:** `C:\Dev\TheKlangVault\Inbox\Archive\Rambles.md`
**Decomposed Ideas:** 2

---

### 🎙️ [NK] Stream-of-Consciousness Ramble Ingestion Workflow (Rambles #1)
- 🎯 **Core Intent**: Establish an automated workflow in N'kai to ingest stream-of-consciousness rambles and decompose them into actionable research items.
- 🛠️ **Proposed Mechanism**: N'kai sidecar intake pipeline & ramble ingestor CLI (`tools/ramble_ingest.py`).
- 🚦 **Action Route**: `[ 📥 Backlog ]` (Confidence: 0.90)
> *Snippet*: "You seemed to be doing ok with taking a long ramble and turning it into discrete ideas. Is this a good workflow to take my rambles..."

---

### 🎙️ [NK] Ramble Note Granularity & File Architecture (Rambles #2)
- 🎯 **Core Intent**: Determine whether short, targeted notes across many files or longer, consolidated ramble documents are optimal for the vault.
- 🛠️ **Proposed Mechanism**: Obsidian vault information architecture & N'kai batch ingestion strategy.
- 🚦 **Action Route**: `[ 🔬 Research ]` (Confidence: 0.85)
> *Snippet*: "Also, better with short rambles that are more targeted and tons of files? Or, less files with longer rambles? Just trying to figure out..."
```

---

## 6. Integration with `tools/check_mail.py` & N'kai Sidecars

When `--add-to-queue` is invoked, `RambleCard.to_mail_item()` generates a standardized `MailItem` injected into the persistent `MailQueue`:
1. `id`: Formatted as `ramble-<stem>-<index>`.
2. `title`: Formatted as `🎙️ <title>`.
3. `priority`: Assigned based on urgency heuristics (e.g. `P0` if source note contains `#p0` or *"highest priority"*).
4. `status`: Set to `TriageStatus.PENDING`.
5. `tags`: Enriched with `["ramble", target_pillar.lower()] + tags`.
6. `preview`: Concise excerpt representing the core intent.
7. `body`: Fully formatted Markdown representation of the 3-bullet breakdown and supporting raw excerpt.
8. `queue.sort_by_priority()`: Re-sorts the queue so top-priority cards float to the top for the morning triage session in `morning_mail_inbox.html`.

This creates a seamless bridge: a voice memo dumped into Obsidian is ingested in 50ms, staged as an interactive card in the Morning Mail sidecar, and triaged with a single click.
