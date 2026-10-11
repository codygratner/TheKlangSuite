# 🧪 Flight 16+17: N'kai QA Sandbox Board & Sandbox/Production Mode (`rj-03`)

> **ID:** plan-009  
> **TARGET:** TOOLING_HARNESS (New Klang City) 🚀  
> **Topic Target:** Mailbag Card `rj-03` / BACKLOG Section 15 ("QA Sandbox Board & Sandbox/Production Mode")  
> **Status:** COMPLETED ✅  
> **Priority:** P0 (Top Priority)  
> **Target Branch:** `0.4.0-dev`  
> **Recommended Setting:** Gemini 3.8 Flash (Thinking: High) — 🟢 SUSTAINABLE  
> **Source Note:** [`C:\Dev\TheKlangVault\Inbox\Archive\SQA meeting with Tom about vibe coding QA.md`](file:///C:/Dev/TheKlangVault/Inbox/Archive/SQA%20meeting%20with%20Tom%20about%20vibe%20coding%20QA.md)  
> **Supersedes:** BACKLOG N'kai Flights 16 (Sandbox & Fixture QA Test Board) + 17 (Multi-Stage Environment PCDA)  

---

## 🎯 Architectural Intent & Storyline

Every N'kai UI change was previously validated against the *production* mailbag, which risked real triage state (as experienced during the `--archive-inbox` ledger-wipe autopsy). A synthetic fixture board gives a safe, deterministic place to test and break UI interactions.

The Hegelian 80/20 resolution to "do we need dev/qa/prod environments?" for solo AI pair-programming:
**One codebase, two runtime modes (`SANDBOX` vs `PRODUCTION`), and Git branches for staging** — zero multi-server infrastructure bloat.

**Flight 16+17** delivers:
1. `assets/fixtures/sandbox_mail_queue.json`: Fully typed synthetic test fixture covering every visual card state (Lovecraftian mock items: Tsathoggua statue placement, Shub-Niggurath spore filter, etc.).
2. `--sandbox` CLI runtime mode in `tools/check_mail.py` rendering `.agents/sidecar/sandbox_mail_inbox.html`.
3. Echo-only mock action dispatch (`[QA_TEST_MOCK]` prefix) with zero ledger or vault mutation.
4. Deterministic element IDs for automated testing (`btn-<action>-<cardId>`, `pill-<rail>-<key>`).
5. Automated test suite `tools/tests/test_sandbox_board.py` verifying zero-mutation isolation.

---

## 📋 Phase Breakdown

### Phase 1: Fixture Data (`assets/fixtures/sandbox_mail_queue.json`)
- [x] Create directory `assets/fixtures/` if not present.
- [x] Author `assets/fixtures/sandbox_mail_queue.json` matching `MailQueue` schema in `tools/models.py`:
  - 6 synthetic cards covering visual states:
    - `sbx-01`: PENDING P0 ("Tsathoggua Statue Placement in City Hall Plaza")
    - `sbx-02`: PENDING P3 ("Deep One Submersible Air Compressor") with `merged_count: 3` and 3 source notes.
    - `sbx-03`: APPROVED ("Shub-Niggurath Spore Dispersion Filter")
    - `sbx-04`: BACKLOG ("Cthulhu Slumber Cycle LFO Timing")
    - `sbx-05`: GRAVEYARD ("Azathoth Random Note Generator without Clock")
    - `sbx-06`: COMPLETED ("Nyarlathotep 1000-Voice Facet Stacking")
  - One 🆕 new-mail item (`sbx-07` "🆕 Unreferenced Shoggoth Sighting").
  - Tags include `"sandbox"` and multi-pillar tags (`["tks", "nk", "tt"]`) to test overflow pills.
- [x] Validate fixture loads and parses cleanly via `MailQueue.model_validate_json`.

### Phase 2: Mode Switch in `check_mail.py`
- [x] Add `--sandbox` CLI flag to `tools/check_mail.py`:
  - Loads `assets/fixtures/sandbox_mail_queue.json` directly.
  - Never scans Vault `Inbox/` or `Archive/`.
  - Never writes to `.agents/pipeline/mailbag/mail_queue.json` or snapshot files.
  - Defaults output HTML to `.agents/sidecar/sandbox_mail_inbox.html`.
- [x] Inject `window.NKAI_ENVIRONMENT = "SANDBOX"` into sandbox HTML and `"PRODUCTION"` into production HTML.
- [x] Add visual Sandbox indicator:
  - Page `<title>` prefixed `[SBX]`.
  - Persistent high-contrast amber banner across top of board:
    `🧪 SANDBOX MODE — Mock data only. Button clicks echo [QA_TEST_MOCK] with zero persistence.`

### Phase 3: Mock Action Dispatch & Deterministic Element IDs
- [x] In `tools/check_mail.py` HTML template (and `c:\Dev\nkai\core\sidecar.js`):
  - In `dispatchAction(payload)`: when `window.NKAI_ENVIRONMENT === "SANDBOX"`, prefix message with `[QA_TEST_MOCK]` and bypass API persistence.
  - LocalStorage key namespace uses `nkai_sbx_` prefix in sandbox mode.
- [x] Add deterministic DOM IDs to all interactive controls:
  - Quick action chips / buttons: `id="btn-<action>-<id>"` (e.g. `btn-approve-sbx-01`, `btn-backlog-sbx-02`).
  - Filter pills: `id="pill-<rail>-<key>"` (e.g. `pill-status-pending`, `pill-pillar-tks`).

### Phase 4: Automated Verification Suite (`tools/tests/test_sandbox_board.py`)
- [x] Author `tools/tests/test_sandbox_board.py` using Python `unittest`:
  - Test 1: `assets/fixtures/sandbox_mail_queue.json` validates against Pydantic `MailQueue`.
  - Test 2: Running `check_mail.py --sandbox --no-browser` leaves production `mail_queue.json` untouched (hash/mtime check).
  - Test 3: Output file `.agents/sidecar/sandbox_mail_inbox.html` exists and contains `NKAI_ENVIRONMENT = "SANDBOX"`.
  - Test 4: Output HTML contains deterministic element IDs (`btn-approve-sbx-01`, etc.).
- [x] Execute `python -m unittest tools/tests/test_sandbox_board.py` (assert 100% pass).
- [x] Run `python tools/audit_sidecar.py .agents/sidecar/sandbox_mail_inbox.html` (assert PERFECT PASS).
- [x] Run production regression: `python tools/check_mail.py --archive --no-browser` (confirm 37 production cards, audit PERFECT PASS).

### Phase 5: Documentation, Sync & Commits
- [x] Author design spec `docs/specs/nkai_sandbox_and_multistage_environments.md`:
  - Document the 2-mode architecture (`SANDBOX` vs `PRODUCTION`).
  - Map Tom's dev/cert/prod PCDA: dev = feature branch + Sandbox, cert = dev branch + QA pass, prod = tagged release + Production.
- [x] Register spec in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.
- [x] Synchronize `docs/GLOSSARY.md` (Section 19: Sandbox Mode & Fixture QA) and `docs/CHEAT_SHEET.md`.
- [x] Mirror updated `check_mail.py` to `c:\Dev\nkai\tools` and `c:\Dev\nkai\bundle\tools`.
- [x] Run `python tools/verify_utf8.py --all --strict` and `python tools/verify_context_clues.py`.
- [x] Mark Flight 16+17 as `[x] COMPLETED` in `docs/BACKLOG.md` and Card `rj-03` in `rejigger.json`.
- [x] Commit changes cleanly in `TheKlangSuite` and `nkai`.
