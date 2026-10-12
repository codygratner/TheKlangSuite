# N'kai QA Sandbox Board & Multi-Stage Environments (Flight 16+17 / `rj-03`)

## 1. Architectural Intent & Storyline

In early iterations of the N'kai Morning Mailbag (`check_mail.py`), visual QA and interface improvements were validated directly against the *production* Obsidian inbox and ledger (`.agents/pipeline/mailbag/mail_queue.json`). As documented in the `--archive-inbox` ledger-wipe autopsy, testing UI changes on live user data created severe hazards: accidental mutations, lost triage states, and distorted token impact metrics.

During an SQA review with Tom regarding vibe coding quality engineering (source note: [`SQA meeting with Tom about vibe coding QA.md`](file:///C:/Dev/TheKlangVault/Inbox/Archive/SQA%20meeting%20with%20Tom%20about%20vibe%20coding%20QA.md)), the dialectic emerged:
- **Enterprise Dev/Cert/Prod Model (Thesis)**: Traditional enterprise engineering constructs multi-tier infrastructure with distinct servers, Docker containers, remote databases, and staging web hosts for dev, cert, and prod.
- **Solo AI Pair-Programming Constraints (Antithesis)**: For a solo developer collaborating with agentic coding assistants in Antigravity, running multi-server orchestration overhead produces immense maintenance friction and breaks offline `file:///` and local webview ergonomics.
- **The Hegelian 80/20 Resolution (Synthesis)**: **One codebase, two runtime modes (`SANDBOX` vs `PRODUCTION`), and Git branches for multi-stage gating**.

```
┌────────────────────────────────────────────────────────────────────────┐
│                        THE 80/20 MULTI-STAGE GATING                    │
├────────────────────┬────────────────────┬──────────────────────────────┤
│ STAGE              │ RUNTIME MODE       │ DATA SOURCE & ISOLATION      │
├────────────────────┼────────────────────┼──────────────────────────────┤
│ Dev / Feature      │ SANDBOX (`--sbx`)  │ assets/fixtures/*.json       │
│ (Local Branches)   │                    │ Zero vault/queue mutations   │
├────────────────────┼────────────────────┼──────────────────────────────┤
│ Cert / QA          │ SANDBOX + PROD     │ Automated test suites        │
│ (`0.4.0-dev`)      │ (Audit Mode)       │ tools/tests/ + audit_sidecar │
├────────────────────┼────────────────────┼──────────────────────────────┤
│ Prod / Shipped     │ PRODUCTION         │ TheKlangVault/Inbox/         │
│ (Git Tags)         │ (Default)          │ Real mail_queue.json ledger  │
└────────────────────┴────────────────────┴──────────────────────────────┘
```

---

## 2. Core Architecture

### 2.1 The Synthetic Fixture Contract (`assets/fixtures/sandbox_mail_queue.json`)
Rather than depending on volatile live Obsidian inbox notes, the QA Sandbox loads a deterministic, version-controlled synthetic dataset validated against Pydantic's `MailQueue` model (`tools/models.py`).

The fixture incorporates 7 synthetic cards covering every visual and architectural card permutation:
1. `sbx-01`: **PENDING P0** ("Tsathoggua Statue Placement in City Hall Plaza") — Tests top-priority alert badges, multi-pillar tags (`NK`, `AGY`), and mono-spaced lore excerpts.
2. `sbx-02`: **PENDING P3** ("Deep One Submersible Air Compressor") — Tests `merged_count: 3`, dependency links (`dependencies: ["sbx-01"]`), and multi-source note link pills.
3. `sbx-03`: **APPROVED P1** ("Shub-Niggurath Spore Dispersion Filter") — Tests immediate approved green status banners and Tier 1 Pro token costs.
4. `sbx-04`: **BACKLOG P2** ("Cthulhu Slumber Cycle LFO Timing") — Tests cataloged backlog banner formatting and Tier 3 Low token costs.
5. `sbx-05`: **GRAVEYARD P3** ("Azathoth Random Note Generator without Clock") — Tests rejected status card styling and graveyard rationale banners.
6. `sbx-06`: **COMPLETED P0** ("Nyarlathotep 1000-Voice Facet Stacking") — Tests shipped and codified status banners with 4-pillar tags (`TKS`, `TBD-16`, `NK`, `TT`).
7. `sbx-07`: **PENDING P1** ("🆕 Unreferenced Shoggoth Sighting in Lower Sump") — Tests unreferenced raw inbox notes without docket or rejigger mapping.

---

### 2.2 CLI Runtime Mode Switch (`check_mail.py --sandbox`)

Invoking `python tools/check_mail.py --sandbox` activates complete sandboxing:
- **Zero Vault Scanning**: Completely bypasses scanning `TheKlangVault/Inbox/` or `Archive/`.
- **Zero Ledger Mutation**: Does not touch, overwrite, or snapshot `.agents/pipeline/mailbag/mail_queue.json`.
- **Isolated Target Output**: Defaults generation to `.agents/sidecar/sandbox_mail_inbox.html` (preserving `.agents/sidecar/morning_mail_inbox.html` for production).
- **Environment Flag**: Sets `window.NKAI_ENVIRONMENT = "SANDBOX"` in HTML script context.
- **Amber Sandbox Banner**: Injects a high-contrast amber header bar above the primary navigation tier:
  ```html
  <div id="sandbox-banner" class="sandbox-banner">
    🧪 <strong>SANDBOX MODE</strong> — Synthetic test fixture (7 mock cards). Button clicks echo <code>[QA_TEST_MOCK]</code> with zero persistence.
  </div>
  ```
- **Title Indicator**: Prefixes the browser title with `[SBX]` (e.g. `[SBX] N'kai Morning Mail Inbox & Triage Lab`).

---

### 2.3 Mock Action Dispatch & Storage Isolation

When interacting with the Sandbox board in an Antigravity webview or browser:
1. **Action Echoing (`[QA_TEST_MOCK]`)**:
   - Clicking triage chips (`Approve`, `Backlog`, `Research`, `Graveyard`, `PCDA`) generates prompt text prefixed with `[QA_TEST_MOCK] ` (e.g. `[QA_TEST_MOCK] Approve 'Tsathoggua Statue Placement...'`).
   - The FastAPI backend endpoint (`POST :8000/api/action`) is bypassed in sandbox mode, preventing unwanted mutations to background daemons.
2. **LocalStorage Key Namespacing**:
   - Production triage state is stored in `localStorage['nkai_mail_triage_actions']`.
   - Sandbox triage state is strictly namespaced to `localStorage['nkai_sbx_mail_triage_actions']`.
   - Modifying cards in Sandbox never leaks into or corrupts production triage state.

---

### 2.4 Deterministic Element IDs for Automated Testing

To enable deterministic automated browser testing, scrapers, and headless DOM assertions, all interactive controls feature predictable IDs:

#### Card Action Chips
Formulated as `id="btn-<action>-<clean_id>"` where `clean_id = re.sub(r'[^a-zA-Z0-9_-]', '_', card.id)`:
- `btn-approve-sbx-01`
- `btn-backlog-sbx-01`
- `btn-research-sbx-01`
- `btn-graveyard-sbx-01`
- `btn-pcda-sbx-01`

#### Filter Rail Pills
Formulated with predictable slug names:
- **Rail 1 (Pillars)**:
  - `pill-pillar-all`
  - `pill-pillar-tks`
  - `pill-pillar-tt`
  - `pill-pillar-nk`
  - `pill-pillar-tkr`
  - `pill-pillar-agy`
  - `pill-pillar-tbd16`
- **Rail 2 (Status & Triage)**:
  - `pill-status-all`
  - `pill-status-untriaged`
  - `pill-status-p0`
  - `pill-status-backlog`
  - `pill-status-completed`
  - `pill-status-research`
  - `pill-status-graveyard`
  - `pill-status-pcda`
  - `pill-status-docket`

---

## 3. Verification & Regression Gauntlet

The sandbox system is continuously guarded by a dedicated Python unit test suite (`tools/tests/test_sandbox_board.py`):
1. `test_fixture_schema_valid()`: Verifies that the fixture JSON adheres to Pydantic's `MailQueue` schema, containing 7 items and 2 P0 priorities.
2. `test_sandbox_zero_mutation()`: Computes the SHA256 digest of production `mail_queue.json`, runs `check_mail.py --sandbox --no-browser`, and asserts 100% hash equality with zero new staging receipts.
3. `test_sandbox_html_content()`: Verifies `[SBX]` title, amber banner, `window.NKAI_ENVIRONMENT = "SANDBOX"`, and deterministic button/pill element IDs.
4. `test_production_regression()`: Runs `check_mail.py --archive --no-browser` and asserts `window.NKAI_ENVIRONMENT = "PRODUCTION"`, absence of sandbox banner, and correct production triage rendering.
5. Static Web QA Linting: Runs `python tools/audit_sidecar.py` across both HTML outputs, enforcing zero tag imbalances, zero unescaped Windows paths, and zero click traps.
