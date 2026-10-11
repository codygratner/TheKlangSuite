# N'kai Mailbag Intake & Triage v2 Specification (Flight 7 / `rj-07` & `rj-17`)

## 1. Architectural Intent & Storyline

In the daily flow of engineering across the Klang ecosystem, idea capture originates from diverse, unpredictable sources:
- Multi-minute voice rambles recorded on mobile phones containing dozens of distinct thoughts.
- 0-byte or whitespace-only quick notes where the thought is entirely contained in the filename.
- Agent-authored exploratory proposals or tabling tickets generated during autonomous sprints.
- Complex, multifaceted notes requiring specialized engineering rituals (architectural grill interviews, deep research spikes, or implementation drafting).
- Urgent epiphanies requiring immediate, topological insertion at a specific rank in the backlog queue without manually recalculating integer positions.

Prior to **Flight 7**, several critical UX frictions existed:
1. **Queue Flooding from Multi-Thought Rambles**: A single ramble containing 7 distinct ideas generated 7 separate cards in the mail queue, overwhelming the human triager and breaking the "Morning Mail" cadence.
2. **Quick Note Discard / Name Mutilation**: Empty files created on mobile devices frequently failed validation or lost their filename-based context.
3. **Agent Clutter in Human Inbox**: Automated agent tabling artifacts co-mingled with human inbox notes, making human intent difficult to separate from autonomous findings.
4. **Context Switching Friction**: Triaging complex cards required manually copying titles and switching windows to launch `/grill-me`, `/deep-research`, or `/plan`.
5. **Rigid Backlog Splice Mechanics**: Inserting a high-priority idea required manual JSON editing or re-ranking all subsequent items by hand.

**Flight 7: Mailbag Intake & Triage v2** synthesizes solutions to these problems into a cohesive, high-density intake and triage engine across `TheKlangSuite` and `nkai`:

```
┌────────────────────────────────────────────────────────────────────────┐
│                   MAILBAG INTAKE & TRIAGE V2 ENGINE                    │
├────────────────────┬────────────────────┬──────────────────────────────┤
│ CAPABILITY         │ IMPLEMENTATION     │ BEHAVIOR                     │
├────────────────────┼────────────────────┼──────────────────────────────┤
│ Overflow Queue     │ ramble_ingest.py   │ Top 3 active; remaining to   │
│                    │ board_renderer.py  │ overflow.json; chip & pill   │
├────────────────────┼────────────────────┼──────────────────────────────┤
│ Quick Notes        │ check_mail.py      │ 0-byte/whitespace: title is  │
│                    │ test_intake_v2.py  │ filename; quick-note tag     │
├────────────────────┼────────────────────┼──────────────────────────────┤
│ Inbox/Tabled/      │ check_mail.py      │ Dedicated agent folder;      │
│                    │ board_renderer.py  │ tag 'tabled'; 🗂 chip        │
├────────────────────┼────────────────────┼──────────────────────────────┤
│ Attention Tags &   │ check_mail.py      │ Scan !IMPORTANT, !BLOATED... │
│ Session Launchers  │ master_template    │ Buttons: Grill, Spike, Plan  │
├────────────────────┼────────────────────┼──────────────────────────────┤
│ /mail-add CLI      │ mail_add.py        │ Splice at --after or --rank; │
│                    │ mail-add skill     │ contiguous renumbering       │
└────────────────────┴────────────────────┴──────────────────────────────┘
```

---

## 2. Core Architecture & Data Contracts

### 2.1 Ramble Overflow Queue (`tools/ramble_ingest.py`, `tools/models.py`)

When voice rambles or stream-of-consciousness brain dumps contain more than 3 distinct actionable ideas, `tools/ramble_ingest.py` decomposes them cleanly:
- **Active Mailbag**: Exactly the top 3 cards are written to the active mailbag queue (`mail_queue.json`).
- **Overflow Storage**: Any remaining cards (cards 4+) are saved to `.agents/pipeline/mailbag/overflow.json`.
- **Traceability (`split_group`)**: Both active cards and overflow cards retain a `split_group` attribute (derived from the source note name) linking sister cards together.
- **Board Presentation**:
  - The parent cards in the board render a visible `⤵ N in overflow` chip (`.badge-overflow`).
  - Cards carry `data-has-overflow="true"` and `data-overflow-count="N"`.
  - Rail 2 displays a dedicated `Overflow (N)` filter pill allowing instant isolation of multi-idea rambles.

```python
class MailItem(BaseModel):
    ...
    split_group: str = ""   # Groups split multi-thought cards from the same source note
```

---

### 2.2 Filename-Only Quick Notes (`tools/check_mail.py`)

Users frequently capture fleeting ideas by quickly typing a filename on their phone (e.g., `Quick Mobile Thought.md`) and saving a 0-byte or whitespace-only file.

In `tools/check_mail.py`:
1. If the file has 0 bytes, or if its body consists solely of whitespace or YAML frontmatter with no prose, `check_mail.py` sets:
   - `title`: Extracted directly from the filename stem.
   - `tags`: Automatically appends `quick-note`.
   - `preview` & `excerpt`: Formatted as `(quick note — title only)`.
   - `body`: Formatted as `(quick note — title only)`.
2. Preserves any frontmatter keys (e.g., `priority: P0`, `pillars: [TKS]`).
3. Fully integrated into the test fixture suite (`assets/fixtures/sandbox_mail_queue.json` card `sbx-quick-01`).

---

### 2.3 `Inbox/Tabled/` Subdirectory & Tabling Protocol

To maintain complete separation of concerns between human notes and automated agent filings:
- **Directory**: Dedicated `Inbox/Tabled/` folder (in `TheKlangVault/Inbox/Tabled/` and repo test fallbacks).
- **Scanner**: `check_mail.py:scan_all_inbox_notes()` scans `Inbox/`, `Inbox/Archive/`, and `Inbox/Tabled/`, combining them deterministically.
- **Metadata**: Notes originating in `Tabled/` automatically receive the `tabled` tag and `is_tabled: True`.
- **UI Chip**: Rendered in the board card header with a distinctive `🗂 Tabled by agent` chip (`.badge-tabled`).
- **Agent Governance**: Formalized in `GEMINI.md` under `## Strict TODO & Backlog Handling`, instructing agents to table autonomous proposals or deferred explorations directly into `TheKlangVault/Inbox/Tabled/`.

---

### 2.4 Attention Tags & Interactive Session Launchers

Complex notes require immediate contextual triage without manual tool invocations:

#### 1. Attention Marker Parsing
Notes containing marker tokens anywhere in their text are enriched with semantic attention badges:
- `!IMPORTANT` ➔ `badge-attention-important` (`🚨 IMPORTANT`)
- `!BLOATED` ➔ `badge-attention-bloated` (`🐡 BLOATED`)
- `!INCOMPLETE` ➔ `badge-attention-incomplete` (`🧩 INCOMPLETE`)
- `!SPIKE` ➔ `badge-attention-spike` (`🔬 SPIKE`)

#### 2. Session Launcher Buttons
Cards with attention markers, high complexity, or explicit triggers render action buttons in their footer:
- `[ 🔬 Grill-Lab ]`: Initiates an interactive architectural interview (`/grill-lab`).
- `[ 🧠 Deep Research ]`: Launches deep research or spike exploration (`/deep-research`).
- `[ 🔨 Draft Plan ]`: Formulates an initial implementation plan draft (`/plan`).

#### 3. Dual Dispatch Mechanism (`sidecar.js`)
- **Extension Mode**: Sends an IPC post request to `/api/command` with `{ command: "/grill-lab", prompt: "..." }`.
- **Offline / Standalone Fallback**: Copies the formatted command line directly to the user's system clipboard and presents a brief visual toast notification:
  `📋 Copied: /grill-lab "Card Title" to clipboard!`

---

### 2.5 `/mail-add` Position Splice CLI (`tools/mail_add.py`)

A high-performance command-line utility for inserting notes into the rejigger backlog queue with topological precision:

```powershell
python tools/mail_add.py "New High Priority Synth Engine" --body "Implement zero-allocation wavetable core" --pillar TKS --priority P0 --after rj-05
```

#### Capabilities:
1. **Position Splicing**:
   - `--after <item_id>`: Inserts immediately following the target item ID.
   - `--rank <N>`: Inserts at absolute 1-based index `N`.
   - Defaults to appending at the tail if no positioning arguments are supplied.
2. **Contiguous Re-ranking**: Automatically renumbers all subsequent items contiguously (`rank = 1, 2, 3, ...`) without gaps or duplicate ranks.
3. **Markdown Note Generation**: Automatically generates a structured markdown note in `TheKlangVault/Inbox/` with full YAML frontmatter.
4. **Dry-Run Mode**: `--dry-run` simulates the splice and prints the resulting queue order without modifying disk files.
5. **Skill Integration**: Documented as an Antigravity skill in `.agents/skills/mail-add/SKILL.md`.

---

## 3. UI Styling & Visual Hierarchy

All badges, chips, and launcher buttons adhere to N'kai's dark neo-slate aesthetic:

| Element | Class | Visual Appearance |
| :--- | :--- | :--- |
| Overflow Chip | `.badge-overflow` | Purple accent (`#a855f7`), `rgba(168, 85, 247, 0.15)` bg |
| Quick Note Chip | `.badge-quicknote` | Sky accent (`#38bdf8`), `rgba(56, 189, 248, 0.15)` bg |
| Tabled Chip | `.badge-tabled` | Amber accent (`#fbbf24`), `rgba(251, 191, 36, 0.15)` bg |
| Important Badge | `.badge-attention-important` | Red accent (`#f87171`), `rgba(239, 68, 68, 0.2)` bg |
| Bloated Badge | `.badge-attention-bloated` | Orange accent (`#fb923c`), `rgba(249, 115, 22, 0.2)` bg |
| Incomplete Badge | `.badge-attention-incomplete` | Yellow accent (`#facc15`), `rgba(234, 179, 8, 0.2)` bg |
| Spike Badge | `.badge-attention-spike` | Cyan accent (`#22d3ee`), `rgba(6, 182, 212, 0.2)` bg |
| Session Button | `.btn-session-launcher` | Compact slate pill (`#1e293b`), hover highlight `#334155` |

---

## 4. Verification & Testing Gauntlet

The intake and triage v2 engine is verified by a multi-tier test suite:

1. **Unit Test Suite (`tools/tests/test_intake_v2.py`)**:
   - `test_overflow_slicing_and_metadata`: Asserts top 3 items kept in queue, remaining 2 stored in `overflow.json`, and `split_group` preserved.
   - `test_quick_note_filename_only`: Asserts 0-byte and whitespace notes parse filename as title, attach `quick-note` tag, and format excerpt/preview.
   - `test_attention_tag_extraction`: Asserts `!IMPORTANT`, `!BLOATED`, `!INCOMPLETE`, `!SPIKE` markers extract to clean tags.
   - `test_tabled_notes_scanning`: Asserts notes in `Tabled/` receive `tabled` tag and `is_tabled` attribute.
   - `test_mail_add_splice_after`: Asserts `--after` inserts cleanly and contiguously re-ranks items.
   - `test_mail_add_splice_rank`: Asserts `--rank 2` shifts existing items and generates inbox markdown notes.
2. **Web QA Audit (`tools/audit_sidecar.py`)**:
   - `master_template.html`: 0 tag balance errors, valid CSS/JS syntax.
   - `sandbox_mail_inbox.html`: 0 errors.
   - `morning_mail_inbox.html`: 0 errors.
3. **Universal UTF-8 Compliance**:
   - `python tools/verify_utf8.py --all --strict`: 58/58 files 100% compliant.
