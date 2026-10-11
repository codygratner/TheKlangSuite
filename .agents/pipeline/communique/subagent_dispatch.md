STATUS: COMPLETED

# Subagent Dispatch Charter: Tooling Implementation
> *Target Pipeline:* Dual-Track Communiqué — Tooling Pipeline (`TARGET: TOOLING_HARNESS`)
> *Active Branch:* `0.4.0-dev` (TKS) / `main` (nkai)
> *Orchestrator:* Antigravity Main Chat (New Klang City)
> *Executor:* Flash Builder Subagent

---

## 1. Assignment Header
- **STATUS**: `DISPATCHED`
- **ROLE**: `Flash Tooling Builder`
- **MODEL TIER**: `Tier 2 Flash (High)`
- **TASK TYPE**: `TOOLING_IMPLEMENTATION`
- **TASK SLUG**: `nkai-qa-sandbox-board`
- **TARGET REPOSITORIES**: `TheKlangSuite`, `nkai`
- **TICKET ID**: `plan-009` (Flight 16+17 / `rj-03`)

## 2. Scope & Instructions

### Phase 2: Mode Switch & Sandbox Pipeline in `tools/check_mail.py`
1. Add `--sandbox` CLI flag to `tools/check_mail.py` `main()`:
   - When `--sandbox` is passed:
     - Load fixture `assets/fixtures/sandbox_mail_queue.json` (resolving relative to repo root or `DEFAULT_SANDBOX_FIXTURE = Path("assets/fixtures/sandbox_mail_queue.json")`).
     - Load via `MailQueue.model_validate_json(fixture_path.read_text(encoding="utf-8"))`.
     - Skip scanning vault `Inbox/` or `Archive/`.
     - Skip building queue, skip writing to `.agents/pipeline/mailbag/mail_queue.json`, and skip creating snapshots.
     - Default output HTML path: `.agents/sidecar/sandbox_mail_inbox.html` (unless `--out` is explicitly passed).
     - Set `environment = "SANDBOX"`.
     - Pass `environment="SANDBOX"` to `generate_html_inbox_board(inbox_dir=Path("assets/fixtures"), notes=[], docket_items=[], out_path=out_file, queue=queue, environment="SANDBOX")`.
   - When not `--sandbox`:
     - Default output HTML path: `.agents/sidecar/morning_mail_inbox.html`.
     - Set `environment = "PRODUCTION"`.
     - Pass `environment="PRODUCTION"` to `generate_html_inbox_board(..., environment="PRODUCTION")`.
2. In `generate_html_inbox_board`:
   - Add parameter `environment: str = "PRODUCTION"`.
   - Page `<title>`:
     - If `environment == "SANDBOX"`: `<title>[SBX] N'kai Morning Mail Inbox & Triage Lab</title>`.
     - If `environment == "PRODUCTION"`: `<title>📬 N'kai Morning Mail Inbox & Triage Lab</title>`.
   - In HTML body:
     - If `environment == "SANDBOX"`, inject persistent amber banner at the top of the board (above `.header-tier-top`):
       `<div id="sandbox-banner" class="sandbox-banner">🧪 <strong>SANDBOX MODE</strong> — Synthetic test fixture (7 mock cards). Button clicks echo <code>[QA_TEST_MOCK]</code> with zero persistence.</div>`
     - If `environment == "PRODUCTION"`, empty string.
   - CSS for `.sandbox-banner`:
     - Amber styling: `background: rgba(245, 158, 11, 0.18); border-bottom: 1px solid rgba(245, 158, 11, 0.4); color: #fbbf24; padding: 8px 16px; font-size: 12px; font-family: var(--font-mono); text-align: center; z-index: 2000;`.
   - CSS for `.chip-approve`:
     - `.chip-approve { color: #34d399; border-color: rgba(52, 211, 153, 0.4); }`
     - `.chip-approve:hover { background: rgba(52, 211, 153, 0.2); border-color: #34d399; }`

### Phase 3: Mock Action Dispatch & Deterministic Element IDs
1. Deterministic Element IDs:
   - On each card, action buttons must have IDs:
     - `id="btn-approve-{clean_id}"`
     - `id="btn-backlog-{clean_id}"`
     - `id="btn-research-{clean_id}"`
     - `id="btn-graveyard-{clean_id}"`
     - `id="btn-pcda-{clean_id}"`
     where `clean_id = re.sub(r'[^a-zA-Z0-9_-]', '_', it.id)`. (Ensure `btn-approve-{clean_id}` is rendered!).
   - On filter chips / pills:
     - Rail 1 (Pillars): `id="pill-pillar-all"`, and for each pillar `id="pill-pillar-{p_slug}"`. Update `applyPillarFilter` to support both `pill-pillar-` and `pillar-btn-`.
     - Rail 2 (Status): `id="pill-status-all"`, `id="pill-status-untriaged"`, `id="pill-status-p0"`, `id="pill-status-backlog"`, `id="pill-status-completed"`, `id="pill-status-research"`, `id="pill-status-graveyard"`, `id="pill-status-pcda"`, `id="pill-status-docket"`. Update `applyStatusFilter` to query `document.getElementById('pill-status-' + statusKey) || document.getElementById('filter-' + statusKey)`.
2. In Board JavaScript:
   - Inject `window.NKAI_ENVIRONMENT = "{environment}";`.
   - In `dispatchMailAction(cardId, actionOrTitle, ...)`:
     - Support `'approve'` action alongside `['backlog', 'research', 'graveyard', 'pcda']`.
     - If `actionType === 'approve'`:
       `promptText = "Approve '" + title + "' for implementation and schedule into active development runway.";`
     - If `window.NKAI_ENVIRONMENT === "SANDBOX"`:
       - Prefix `promptText = "[QA_TEST_MOCK] " + promptText;`
       - Bypass the `fetch('http://127.0.0.1:8000/api/action', ...)` call.
   - In `applyCardDOMStatus`:
     - Handle `'approve'` status: icon `✅`, accentColor `#34d399`. If `window.NKAI_ENVIRONMENT === "SANDBOX"`, ensure input box contains `[QA_TEST_MOCK] ` prefix.
   - In `saveTriageAction` and `initLocalStorageSync`:
     - Use `const key = (window.NKAI_ENVIRONMENT === 'SANDBOX') ? 'nkai_sbx_mail_triage_actions' : 'nkai_mail_triage_actions';`.
3. In `c:\Dev\nkai\core\sidecar.js`:
   - In `dispatchAction(actionPayload)`:
     - If `window.NKAI_ENVIRONMENT === "SANDBOX"`:
       - If payload is string and does not start with `[QA_TEST_MOCK]`, prepend `[QA_TEST_MOCK] `.
       - Bypass the `fetch` to `:8000/api/action`.

### Phase 4: Automated Verification Suite (`tools/tests/test_sandbox_board.py`)
1. Create `tools/tests/__init__.py`.
2. Author `tools/tests/test_sandbox_board.py` using Python standard library `unittest`:
   - `test_fixture_schema_valid()`: Asserts `assets/fixtures/sandbox_mail_queue.json` validates against `MailQueue`. Asserts 7 items, P0 count == 2, `sbx-01` through `sbx-07` present.
   - `test_sandbox_zero_mutation()`:
     - Computes SHA256 of `.agents/pipeline/mailbag/mail_queue.json` before run.
     - Runs `check_mail.py --sandbox --no-browser`.
     - Computes SHA256 of `.agents/pipeline/mailbag/mail_queue.json` after run; asserts hashes are 100% IDENTICAL.
     - Asserts no new files created in `.agents/pipeline/staging/` or snapshots.
   - `test_sandbox_html_content()`:
     - Asserts `.agents/sidecar/sandbox_mail_inbox.html` exists.
     - Asserts `<title>[SBX]` is in content.
     - Asserts `id="sandbox-banner"` is in content.
     - Asserts `window.NKAI_ENVIRONMENT = "SANDBOX"` is in content.
     - Asserts `id="btn-approve-sbx-01"` is in content.
     - Asserts `id="btn-backlog-sbx-02"` is in content.
     - Asserts `id="pill-pillar-all"` is in content.
     - Asserts `id="pill-status-untriaged"` is in content.
   - `test_production_regression()`:
     - Runs `check_mail.py --archive --no-browser`.
     - Asserts `.agents/sidecar/morning_mail_inbox.html` exists and contains `window.NKAI_ENVIRONMENT = "PRODUCTION"`.
     - Asserts `sandbox-banner` is NOT present in production HTML.
3. Run `python -m unittest tools/tests/test_sandbox_board.py` (assert 100% pass).
4. Run `python tools/audit_sidecar.py .agents/sidecar/sandbox_mail_inbox.html` (assert PERFECT PASS).
5. Run `python tools/audit_sidecar.py .agents/sidecar/morning_mail_inbox.html` (assert PERFECT PASS).

### Phase 5: Documentation, Sync & Multi-Repo Commits
1. Author design spec `docs/specs/nkai_sandbox_and_multistage_environments.md`:
   - Document the 2-mode architecture (`SANDBOX` vs `PRODUCTION`).
   - Document Tom's dev/cert/prod PCDA resolution (Git branches + mode switch instead of multi-server infrastructure).
   - Document deterministic DOM IDs for headless UI testing.
2. Register spec in `docs/DOCS_CATALOG.json` under `specifications`.
3. Register spec in `docs/SYSTEM_MAP.md` under Section 7 (Design Specs).
4. Synchronize `docs/GLOSSARY.md` (Section 19: Sandbox Mode & Fixture QA) and `docs/CHEAT_SHEET.md`.
5. Mirror `tools/check_mail.py` to `c:\Dev\nkai\tools/check_mail.py` and `c:\Dev\nkai\bundle\tools/check_mail.py`.
6. Run `python tools/verify_utf8.py --all --strict` and `python tools/verify_context_clues.py`.
7. Update `docs/BACKLOG.md`: mark Flight 16+17 as `[x] COMPLETED`.
8. Update `.agents/pipeline/mailbag/rejigger.json`: mark card `rj-03` as `status: "COMPLETED"`, `verdict: "COMPLETED"`.
9. Update `PLAN.md`: mark all checkboxes `[x]` and set Status to `COMPLETED ✅`.
10. Append execution receipt to `.agents/pipeline/communique/subagent_receipt.md` and set dispatch `STATUS: COMPLETED`.
11. Commit changes cleanly:
    - In `TheKlangSuite` (`0.4.0-dev`): `git add ... ; git commit -m "feat(sandbox): implement Flight 16+17 N'kai QA sandbox board and sandbox/production mode"`
    - In `nkai` (`main`): `git add ... ; git commit -m "feat(sandbox): add sandbox mode handling to check_mail and sidecar.js"`

## 3. Acceptance Criteria
- `python -m unittest tools/tests/test_sandbox_board.py` passes 100%.
- Both `sandbox_mail_inbox.html` and `morning_mail_inbox.html` achieve PERFECT PASS on `audit_sidecar.py`.
- Zero mutations to real queues or vaults in `--sandbox` mode.
- Receipt committed and tickets synced.
