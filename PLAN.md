# Flight 2: N'kai to High Reasoning Prompt Compiler

**TARGET:** TOOLING_HARNESS  
**Role:** New Klang City Architecture & Web Engineering  
**Recommended Model:** Gemini 3.8 Flash (Thinking: High) — Tier 2 (Balanced Engineering & UI)  
> **Status:** IN_PROGRESS 🔨  

---

### Executive Overview & Architectural Dialectic
- **The "Master Architect" Principle**: Passing raw notes or unstructured rambles to expensive flagship reasoning models (Claude 3.7 Sonnet Thinking, Gemini 3.1 Pro High, ChatGPT o3) wastes valuable context window tokens on basic noise extraction and disambiguation.
- **Local Structural Filter**: N'kai acts as the zero-token local data miner, structuring the thesis (user intent), antithesis (adversarial stress-tests, friction metrics, failure modes), constraints, and sleepy architecture metrics.
- **Dense Dialectical Handoff**: `tools/compile_prompt.py` formats this dense dossier into an undeniable markdown blueprint mandate and transports it via OS clipboard (Win32 ctypes, macOS pbcopy, Linux xclip) or local file drop.
- **Dual Mode**:
  - **Mode A (In-IDE Subagent Dispatch)**: Generates a Pro subagent dispatch charter for `invoke_subagent(Model="pro")`.
  - **Mode B (External Web & Clipboard Exporter)**: Populates OS system clipboard for instantaneous 1-click pasting into external AI web interfaces.

---

### Phase 1: Structured Pydantic Models (`tools/models.py`)
- [x] Define `ThesisBlock`:
  - `core_intent`: str
  - `user_goals`: List[str]
  - `source_note`: Optional[str]
- [x] Define `AntithesisBlock`:
  - `reviewer_persona`: str (default: "The Dissolution / Adversarial Auditor")
  - `echo_chamber_shatter`: str
  - `structural_vulnerabilities`: List[str]
  - `orthogonal_vectors`: List[str]
- [x] Define `SynthesisTargetConfig`:
  - `materialization`: str (e.g., "Python CLI / Sidecar Component / C++ DSP")
  - `the_blueprint`: str
  - `effort_level`: str (default: "MINIMAL")
  - `impact_level`: str (default: "MAXIMUM")
  - `status`: str (default: "PENDING_REASONING")
- [x] Define `PromptHandoffPayload`:
  - `thread_id`: str
  - `reconciliation_protocol`: str (default: "Standard Dialectical")
  - `thesis`: ThesisBlock
  - `antithesis`: AntithesisBlock
  - `targets`: SynthesisTargetConfig
  - `prompt_mandate`: str
  - Methods: `to_markdown()`, `to_clipboard()`, `to_vault_file(path)`.

---

### Phase 2: CLI Compiler Engine (`tools/compile_prompt.py`)
- [x] Build `tools/compile_prompt.py` entrypoint:
  - Supports `--topic <id_or_title>` (pulls from `mail_queue.json` or `rejigger.json` or vault note).
  - Supports `--file <path>` (parses existing note or JSON matrix).
  - Supports `--clipboard` (auto-copies compiled markdown to OS clipboard).
  - Supports `--out <file>` and `--stdout`.
  - Supports `--vault` (drops compiled markdown into `TheKlangVault/Dispatches/reasoning/` or specified vault folder).
  - Cross-platform clipboard implementation:
    - Windows: Win32 API via `ctypes` (`GlobalAlloc`, `OpenClipboard`, `EmptyClipboard`, `SetClipboardData`, `CloseClipboard`).
    - macOS: `pbcopy` subprocess.
    - Linux: `xclip` or `wl-copy`.
  - Strict UTF-8 console stream reconfigures (`sys.stdout.reconfigure(encoding='utf-8')`).

---

### Phase 3: N'kai Server & Sidecar Action Integration (`tools/nkai_serve.py` & `tools/chat_engine.py`)
- [x] Add endpoint in `tools/nkai_serve.py`:
  - `POST /api/compile-prompt`: Accepts `{"topic_id": str, "clipboard": bool}` and returns `{"ok": bool, "markdown": str, "copied": bool}`.
- [x] Add 1-Click Action Chip to `morning_mail_inbox.html` and `chat_board.html`:
  - `[ 🧠 Compile Reasoning Prompt ]` button.
  - Clicking calls `/api/compile-prompt` and shows visual toast feedback ("📋 Copied Prompt to Clipboard! Ready for Claude/Gemini Web").

---

### Phase 4: Unit Test Suite & Regression Verification
- [x] Author `tools/tests/test_compile_prompt.py`:
  - Test Pydantic model serialization and markdown rendering.
  - Test compiling from mail queue item.
  - Test fallback handling for missing antithesis / default blocks.
  - Test clipboard mock / execution.
  - Test CLI invocation `--stdout` and `--out`.
- [x] Run `python -m unittest discover tools/tests/` (assert 100% passing).
- [x] Run `python tools/verify_utf8.py --strict` (assert 0 violations).
- [x] Update `docs/BACKLOG.md` marking Flight 2 as COMPLETED.
