# Flight 3: N'kai Automode: 2-Pass Iterative Research Daemon & Multi-Drawer Dossier Cards

**TARGET:** TOOLING_HARNESS  
**Role:** New Klang City Architecture & Web Engineering  
**Recommended Model:** Gemini 3.8 Flash (Thinking: High) — Tier 2 (Balanced Engineering & UI)  
> **Status:** IN_PROGRESS 🔨  

---

### Executive Overview & Architectural Dialectic
- **The "Rule of 2" Bounded Convergence Architecture**:
  - Multi-agent reflection research demonstrates that gains steeply plateau after iteration 2. Pass 3+ yields negative returns (semantic churn, hallucination compounding, and token waste).
  - Automode strictly enforces the 2-Pass law:
    - **Pass 1 (Parallel Divergent Sweep — Tier 2 Flash)**: Generates affirmative thesis/SOTA precedents and adversarial contrarian stress-tests (`Where I Disagree With The Prompt`).
    - **Pass 2 (Convergent Synthesis — Tier 2 Flash)**: Reconciles Thesis and Antithesis into standard 3-bullet decision cards, 4-lens PCDA matrices, and curated technical citations.
  - **Zero Pro Burn**: Automode runs 100% on Tier 2 Flash. Tier 1 Pro models are strictly prohibited.
- **The Human Triage Gate**:
  - Automode NEVER autonomously touches raw notes in `TheKlangVault/Inbox/`. It executes strictly on items explicitly triaged with `[ 🔬 Research ]` in `morning_mail_inbox.html` or marked `status: "research"` in `.agents/pipeline/mailbag/mail_queue.json`.
  - Strictly bounded batch execution: `--limit 3` (default), finite execution, NEVER an infinite `while True:` loop.
- **The "Hero Front + 3-Tab Drawer" UI Standard**:
  - The 3-bullet synthesis (🎯 Core Intent, 🛠️ Proposed Mechanism, 🚦 Action Route) remains permanently visible on the front face of each card.
  - Single unified drawer toggle: `[ 📂 View Full Dossier (3) ]`.
  - 3-Segmented Tab Strip:
    - `[ 📄 Source Note ]`
    - `[ ⚖️ PCDA Dialectic ]`
    - `[ 🔬 Research Dossier ]`
  - Mobile bottom-sheet layout on viewports $\le 640\text{px}$.

---

### Phase 1: Dossier Data Models & Schema Extension (`tools/models.py`)
- [x] Define `DossierDrawer` Pydantic model:
  - `source_note`: Optional[str]
  - `source_markdown`: Optional[str]
  - `pcda_dialectic`: Optional[Dict[str, Any]]
  - `research_dossier`: Optional[Dict[str, Any]]
  - `pass_count`: int = 2
  - `researched_at`: Optional[str]
- [x] Extend `MailItem` model with `dossier: Optional[DossierDrawer] = None`.
- [x] Verify serialization and backwards compatibility.

---

### Phase 2: 2-Pass Research Engine (`tools/research_daemon.py`)
- [x] Create `tools/research_daemon.py`:
  - `execute_pass_1_divergence(topic: str, note_text: str) -> Tuple[str, str]`: Generates Affirmative SOTA precedents & Adversarial Contrarian Stress-Test.
  - `execute_pass_2_synthesis(topic: str, thesis: str, antithesis: str) -> DossierDrawer`: Synthesizes 3-bullet card, 4-lens PCDA, and research citations.
  - `run_automode_batch(limit: int = 3, target_id: Optional[str] = None) -> List[Dict[str, Any]]`:
    - Reads `mail_queue.json`, filters for items marked `status: "research"`.
    - Enforces hard limit (`--limit N`, default 3).
    - Writes results to `.agents/pipeline/mailbag/researched/<id>.json` and updates `mail_queue.json`.
    - Regenerates triage boards via `check_mail.py --no-browser`.
  - CLI arguments: `--limit`, `--topic`, `--dry-run`, `--force`.
  - Strict UTF-8 console stream reconfigures.

---

### Phase 3: In-Card Unified Dossier Drawer (`tools/board_renderer.py` & templates)
- [x] Update `tools/board_renderer.py`:
  - Render `.mail-dossier-drawer` container when `card.dossier` exists or provide on-demand trigger.
  - Render 3-segmented tab strip: `[ 📄 Source Note ]`, `[ ⚖️ PCDA Dialectic ]`, `[ 🔬 Research Dossier ]`.
  - Style clean tab transitions and mobile bottom-sheet ergonomics.
- [x] Sync templates across `TheKlangSuite` and `nkai` via `python tools/sync_templates.py sync`.

---

### Phase 4: Skill Definition & Automode Slash Command
- [x] Create `.agents/skills/automode/SKILL.md` documenting the 2-pass bounded research protocol.

---

### Phase 5: Unit Tests, Audits & Verification
- [x] Author `tools/tests/test_research_daemon.py`:
  - Test Pass 1 & Pass 2 convergence mechanics.
  - Test Rule-of-2 hard boundary (`max_iterations = 2`).
  - Test batch limit enforcement (`--limit 3`).
  - Test MailItem dossier serialization.
- [x] Run full test suite `python -m unittest discover tools/tests/` (100% pass rate).
- [x] Run `python tools/verify_utf8.py --strict` (100% compliant).
- [x] Update `docs/BACKLOG.md` marking Flight 3 as COMPLETED.
