---
name: mail-add
description: Adds a new idea, note, or proposal to the mailbag and splices it into rejigger.json at an exact target position (--after <id> or --rank <n>), automatically renumbering subsequent cards contiguously. Triggers on /mail-add, /mailadd, "add to mailbag", or "mail add".
---

# 📥 N'kai Mail-Add CLI (`/mail-add`)

> **Tooling Command:** `python tools/mail_add.py`  
> **Model Tier:** Tier 2 Flash (High) — 🟢 SUSTAINABLE  
> **Pillar:** N'kai Tooling Harness (`TARGET: TOOLING_HARNESS`)  
> **Specification:** [`docs/specs/nkai_mailbag_intake_and_triage_v2.md`](file:///c:/Dev/TheKlangSuite/docs/specs/nkai_mailbag_intake_and_triage_v2.md)

---

## 🎯 Purpose & Capabilities

When a new idea strikes during engineering, conversation, or pairing, adding it to the backlog or mailbag shouldn't require manually editing large JSON files or renumbering dozens of ranks.

The **`/mail-add` CLI** provides frictionless capture with deterministic rank splicing:
1. **Inbox Note Generation**: Automatically creates a structured Markdown note in `TheKlangVault/Inbox/` (or local fallback `docs/inbox/`) with valid YAML frontmatter.
2. **Deterministic Position Splice**: Insert cards at an exact position using `--after <id>` (e.g. `--after rj-05`) or `--rank <n>`.
3. **Contiguous Renumbering**: Automatically increments and renumbers subsequent cards in `rejigger.json` without leaving numbering gaps.
4. **Idempotent & Safe**: Fully supports `--dry-run` to inspect proposed rank placements before committing changes to disk.

---

## 🚀 Usage & Commands

### 1. Basic Quick Add
Adds a new note and card at the end of the mailbag queue:
```powershell
python tools/mail_add.py "Polyphonic Microtuning Matrix" --body "Per-voice scala file loading and real-time MTS-ESP bridge." --pillar TKS --priority P1
```

### 2. Splice Immediately After an Existing Card
Inserts a card directly behind a specific topic:
```powershell
python tools/mail_add.py "OLED Waveform Visualizer" --body "Direct SPI framebuffer rendering for TBD-16 voice monitor." --pillar TBD-16 --priority P1 --after rj-07
```

### 3. Insert at an Exact Rank Position
Slices a card into rank 3, pushing current rank 3 and all following cards down:
```powershell
python tools/mail_add.py "Emergency DSP Clamp Audit" --pillar TKS --priority P0 --rank 3
```

### 4. Safe Dry-Run Simulation
Inspect the calculated rank, generated ID, and target file path without touching disk:
```powershell
python tools/mail_add.py "Experimental Vocoder Architecture" --pillar TKR --after rj-10 --dry-run
```

### 5. Machine-Readable JSON Export
Output structured JSON payload for subagents and automated scripts:
```powershell
python tools/mail_add.py "Preset Categorization Sweep" --pillar TKS --json
```

---

## 🛡️ Invariants
- **Universal UTF-8 Compliance**: Always reads and writes using explicit `utf-8` encoding.
- **Zero Numbering Gaps**: Every card in `rejigger.json` receives a strictly contiguous `rank` ($1, 2, 3, \dots, N$).
- **Dual-Store Integrity**: The Markdown note in `TheKlangVault/Inbox/` and the card entry in `rejigger.json` stay synchronized.
