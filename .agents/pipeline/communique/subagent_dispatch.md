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
- **TASK SLUG**: `mailbag-rejigger-overlay`
- **TARGET REPOSITORIES**: `TheKlangSuite`, `nkai`

## 2. Scope
1. Fix `check_mail.py --archive-inbox` ledger wipe: build queue from Archive + Inbox notes, deduped by id.
2. Add `--rejigger PATH` (default `.agents/pipeline/mailbag/rejigger.json`) and `--no-rejigger`.
3. Overlay: one MailItem per card; raw items referenced by card `sources` are folded in; unreferenced raw items appended with a `🆕 ` prefix; sort by rank.
4. Snapshot `mail_queue.json` to `.agents/pipeline/mailbag/snapshots/mail_queue_prev.json` before overwrite.
5. Render rank badge + "🧩 merged from N notes" chip.
6. Mirror to `c:\Dev\nkai\tools\check_mail.py`; regenerate board; audit; deploy to nkai plugin; commit both repos.

## 3. Acceptance
- Board shows 37 cards (25 active + 12 status), audit PERFECT PASS, default filter Untriaged, "All" first on both rails.
- Receipt appended to `subagent_receipt.md`.
