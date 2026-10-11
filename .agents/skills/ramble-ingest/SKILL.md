---
name: ramble-ingest
description: Ingests unstructured voice memos, brain dumps, and stream-of-consciousness phone notes from TheKlangVault/Inbox, decomposing them into structured 3-bullet decision cards (Core Intent, Proposed Mechanism, Action Route) for 1-click N'kai triage. Triggers on /ramble, /ramble-ingest, /ingest-ramble, "ingest ramble", "ramble ingest", or "process ramble".
---

# 🎙️ N'kai "Rambles" Ingestion Engine (`/ramble`)

> **Tooling Command:** `python tools/ramble_ingest.py`  
> **Model Tier:** Tier 2 Flash (High) — 🟢 SUSTAINABLE  
> **Pillar:** N'kai Tooling Harness (`TARGET: TOOLING_HARNESS`)  
> **Specification:** [`docs/specs/nkai_rambles_ingestion_engine.md`](file:///c:/Dev/TheKlangSuite/docs/specs/nkai_rambles_ingestion_engine.md)

---

## 🎯 Purpose & Storyline

Creative musicians and developers don't author structured Jira tickets while inspiration strikes; they dump voice memos, stream-of-consciousness phone notes, and chaotic midnight thoughts into Obsidian (`TheKlangVault/Inbox/`).

The **"Rambles" Ingestion Engine** automates the transition from raw stream-of-consciousness text to atomic **3-Bullet Decision Cards**:
1. 🎯 **Core Intent**: 1-sentence synthesis of the creative goal.
2. 🛠️ **Proposed Mechanism**: Target repo, hardware, or module architecture (`TKS`, `TT`, `NK`, `TBD-16`, `TKR`).
3. 🚦 **Action Route**: Suggested triage recommendation (`[ 📥 Backlog ]`, `[ 🔬 Research ]`, `[ 🪦 Graveyard ]`, `[ ⚖️ PCDA ]`).

### 🛡️ Safety & Governance Invariants
- **Zero Premature Mutation**: Extracted cards NEVER directly edit `docs/BACKLOG.md` or `PLAN.md` without an explicit human confirmation action.
- **Multi-Idea Decomposition**: A single long ramble discussing multiple distinct topics is intelligently decomposed into separate, atomic cards.
- **Inbox Zero Stewardship**: Processing notes with `--archive` safely moves them into `TheKlangVault/Inbox/Archive/`, preserving provenance.

---

## 🚀 Usage & Commands

### 1. Ingest a Single Ramble Note
Inspect and decompose a specific note in the terminal:
```powershell
python tools/ramble_ingest.py --file "C:\Dev\TheKlangVault\Inbox\Archive\Rambles.md" --chat
```

### 2. Batch Scan Active Vault Inbox
Scan all pending notes in `TheKlangVault/Inbox/`:
```powershell
python tools/ramble_ingest.py --inbox --chat
```

### 3. Stage Cards into Morning Mail Queue
Extract cards, convert them into `MailItem` models, and append them to `.agents/pipeline/mailbag/mail_queue.json` (sorting with P0 items at top):
```powershell
python tools/ramble_ingest.py --inbox --add-to-queue --archive
```

### 4. Machine-Readable JSON Export
Export raw JSON payload for subagents or automated workflows:
```powershell
python tools/ramble_ingest.py --file "C:\Dev\TheKlangVault\Inbox\Archive\Rambles.md" --json
```

### 5. Safe Dry-Run Simulation
Preview decomposition without moving notes or writing to the queue:
```powershell
python tools/ramble_ingest.py --inbox --add-to-queue --archive --dry-run
```

---

## 📋 CLI Flag Reference

| Flag | Argument | Description |
| :--- | :--- | :--- |
| `--file` / `-f` | `<path>` | Process a single markdown ramble note. |
| `--inbox` | *None* | Batch scan all markdown notes in `TheKlangVault/Inbox/`. |
| `--scan-archive` | *None* | Scan archived notes in `TheKlangVault/Inbox/Archive/`. |
| `--chat` | *None* | Print clean Markdown cards directly to stdout (default). |
| `--json` | *None* | Output serialized JSON structure (`RambleExtractionResult`). |
| `--add-to-queue` | *None* | Convert cards to `MailItem` and append to `mail_queue.json`. |
| `--archive` | *None* | Move successfully processed notes from `Inbox/` to `Inbox/Archive/`. |
| `--dry-run` | *None* | Simulate operations without modifying disk or queues. |
| `--queue-file` | `<path>` | Custom path to `mail_queue.json`. |

---

## 🌟 Example Output (`--chat`)

```markdown
# 🎙️ N'kai Ramble Extraction Report
**Processed Notes:** 1
**Total Decomposed Ideas:** 2

## 📄 `Rambles.md` (2 Ideas)
- **Path:** `C:\Dev\TheKlangVault\Inbox\Archive\Rambles.md`

### 🎙️ [NK] Stream-of-Consciousness Ramble Ingestion Workflow (#1)
- 🎯 **Core Intent**: Establish an automated workflow in N'kai to ingest stream-of-consciousness rambles and decompose them into actionable research items.
- 🛠️ **Proposed Mechanism**: N'kai sidecar intake pipeline & ramble ingestor CLI (tools/ramble_ingest.py)
- 🚦 **Action Route**: `[ 📥 Backlog ]` (Confidence: 0.90)
- 🏷️ **Tags**: #ramble #p0 #workflow
> *Snippet*: "You seemed to be doing ok with taking a long ramble and turning it into discrete ideas. Is this a good workflow to take my rambles..."

### 🎙️ [NK] Ramble Note Granularity & Vault Architecture (#2)
- 🎯 **Core Intent**: Evaluate trade-offs between short targeted notes across many files versus fewer files with longer rambles.
- 🛠️ **Proposed Mechanism**: Obsidian vault information architecture & batch note ingestion strategy
- 🚦 **Action Route**: `[ ⚖️ PCDA ]` (Confidence: 0.88)
- 🏷️ **Tags**: #ramble
> *Snippet*: "Also, better with short rambles that are more targeted and tons of files? Or, less files with longer rambles? Just trying to figure out..."
```
