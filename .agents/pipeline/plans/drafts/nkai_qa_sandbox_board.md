# 🧪 N'kai QA Sandbox Board & Sandbox/Production Mode (Mailbag `rj-03`)

> **TARGET:** TOOLING_HARNESS (New Klang City → Flash Builder Subagent)
> **Status:** DRAFT (approved in triage 2026-10-11; not yet promoted)
> **Priority:** P0
> **Depends on:** BACKLOG N'kai Flight 18 — Universal UTF-8 Invariant (`rj-02`)
> **Supersedes:** BACKLOG N'kai Flights 16 (Sandbox & Fixture QA Test Board) + 17 (Multi-Stage Environment PCDA)
> **Repos:** `TheKlangSuite` (`0.4.0-dev`), `nkai` (`main`)
> **Tier:** Tier 2 (Flash High) — no DSP, no concurrency

## Why
Every N'kai UI change is currently validated against the *production* mailbag, which risks real triage state (see the `--archive-inbox` ledger-wipe autopsy). A fixture board gives a safe place to break things. The 80/20 answer to "do we need dev/qa/prod?" for a solo AI-assisted dev: **one codebase, two runtime modes, git branches for staging** — no extra servers.

## Phase 1 — Fixture Data (JSON-first)
- [ ] Create `assets/fixtures/sandbox_mail_queue.json` (TKS) mirroring the `MailQueue` schema in `tools/models.py`.
  - 6 mock cards covering every visual state: PENDING P0, PENDING P3, APPROVED, BACKLOG, GRAVEYARD, COMPLETED; one with `merged_count: 3`, one 🆕 new-mail item, one with 2+ pillars (overflow chip).
  - Lovecraftian fixture names (e.g. `sbx-01` "Tsathoggua Statue Placement"), all `tags` include `"sandbox"`.
- [ ] Validate it loads via `MailQueue.model_validate_json` in a test (Phase 4).

## Phase 2 — Mode Switch in `check_mail.py`
- [ ] Add `--sandbox` flag:
  - Reads `assets/fixtures/sandbox_mail_queue.json` instead of scanning the Vault; **never** writes `.agents/pipeline/mailbag/mail_queue.json` or snapshots.
  - Writes to `.agents/sidecar/sandbox_mail_inbox.html` (separate output; production board untouched).
- [ ] Inject `window.NKAI_ENVIRONMENT = "SANDBOX" | "PRODUCTION"` into the rendered HTML.
- [ ] SANDBOX visuals: persistent amber top banner `🧪 SANDBOX — buttons echo only, no real triage`; page `<title>` prefixed `[SBX]`.

## Phase 3 — Mock Dispatch (Echo-Only Buttons)
- [ ] In the board JS dispatch path (and `c:\Dev\nkai\core\sidecar.js` `NkaiEnv.dispatchAction`): when `NKAI_ENVIRONMENT === "SANDBOX"`, prefix every outgoing message with `[QA_TEST_MOCK]` and skip any persistence (`/api/action` writes, localStorage verdict writes use a `sbx:` key namespace).
- [ ] Deterministic element IDs on every interactive control: `btn-<action>-<cardId>` (e.g. `btn-approve-sbx-01`), filter pills `pill-<rail>-<key>`.

## Phase 4 — Verification
- [ ] Python test `tools/tests/test_sandbox_board.py` (stdlib `unittest`):
  - fixture validates against `MailQueue`;
  - `--sandbox` run leaves `mail_queue.json` mtime/hash unchanged;
  - output HTML contains `NKAI_ENVIRONMENT = "SANDBOX"`, the banner, and every expected `btn-approve-sbx-0N` id.
- [ ] `python tools/audit_sidecar.py .agents/sidecar/sandbox_mail_inbox.html` → PERFECT PASS.
- [ ] Production regression: `python tools/check_mail.py --archive --no-browser` still yields 37 items, `NKAI_ENVIRONMENT = "PRODUCTION"`, audit PERFECT PASS.

## Phase 5 — Docs & Closeout
- [ ] Environment policy doc section (in `docs/specs/sqa_automation_hardening.md` or new `docs/specs/nkai_sandbox_mode.md`): dev = feature branch + SANDBOX, cert = `0.x-dev` + SANDBOX QA pass, prod = tagged release + PRODUCTION (maps Tom's dev/cert/prod).
- [ ] Register spec in `docs/DOCS_CATALOG.json` + `docs/SYSTEM_MAP.md`; add "Sandbox Mode" to `docs/GLOSSARY.md` + `docs/CHEAT_SHEET.md` (dual-file sync invariant).
- [ ] Mirror `check_mail.py` / `sidecar.js` to `c:\Dev\nkai`; run `python tools/install_nkai_plugin.py`.
- [ ] BACKLOG: mark the merged Flight 16/17 entry COMPLETED; mailbag `rj-03` → COMPLETED.
- [ ] Commits: `feat(nkai): add sandbox board mode and echo-only mock dispatch` (both repos).
