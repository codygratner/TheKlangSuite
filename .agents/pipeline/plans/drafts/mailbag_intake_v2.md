# 📥 Mailbag Intake & Triage v2: Overflow, Quick Notes, Attention Tags & Session Routing (Mailbag `rj-07` & `rj-17`)

> **TARGET:** TOOLING_HARNESS (New Klang City → Flash Builder Subagent)  
> **Status:** DRAFT (approved in triage 2026-10-11; not yet promoted)  
> **Priority:** P1  
> **Depends on:** Flight 18 (UTF-8, `rj-02`), Flight 16+17 (Sandbox board, `rj-03`) — quick-note and attention tag rendering verified on sandbox  
> **Supersedes:** BACKLOG N'kai Flight 7 (Streamlined Mailbag Ingestion & Dynamic Positioning)  
> **Repos:** `TheKlangSuite` (`0.4.0-dev`), `nkai` (`main`)  
> **Tier:** Tier 2 (Flash High)  

## Why
Ideas get lost on the way IN (throwaway lines inside long rambles, empty quick-notes) and stall on the way OUT (cards lack quick triage routing). Adding an item at a specific position needs manual edits, and cards lack high-visibility callouts when a note is bloated, incomplete, or urgent. This flight merges Intake v2 (`rj-07`) and Triage v2 (`rj-17`) into a cohesive, high-velocity mailbag pipeline.

---

## Phase 1 — Ramble Overflow Queue (`tools/ramble_ingest.py`)
- [ ] When a ramble decomposes into **> 3** ideas, the top 3 become cards; the rest go to `.agents/pipeline/mailbag/overflow.json` (list of `RambleCard`, same schema in `tools/models.py`).
- [ ] Every split card stores `source_note` = the one shared original note path (link, never copy the body). Add `split_group: str` (= source note stem) to `RambleCard`/`MailItem` (optional, default `""`).
- [ ] Board shows an `⤵ N in overflow` chip on the parent card; overflow items render in a collapsed "Overflow" section (status filter rail gets an `Overflow` pill after `All`).

## Phase 2 — Filename-Only Quick Notes (`tools/check_mail.py`)
- [ ] A note with an empty/whitespace body (after frontmatter strip) becomes a card whose title = filename stem, `tags += ["quick-note"]`, preview = `"(quick note — title only)"`.
- [ ] Fixture: add one quick-note card to `assets/fixtures/sandbox_mail_queue.json`; verify render on the sandbox board.

## Phase 3 — `Inbox/Tabled/` for Agent-Authored Notes
- [ ] `check_mail.py` scans `C:\Dev\TheKlangVault\Inbox\Tabled\` as a third source (with Inbox + Archive); items get `tags += ["tabled"]` and a `🗂 Tabled by agent` chip.
- [ ] One-time move of the existing agent-authored `Nkai*.md` notes from `Inbox/` into `Inbox/Tabled/` (list them for user confirmation first — human-in-the-loop).
- [ ] Rule (GEMINI.md, Strict TODO & Backlog Handling): when an agent tables a topic it writes the note to `Inbox/Tabled/` and says `🗂 Tabling this: <title>` in chat. Sync GLOSSARY + CHEAT_SHEET.

## Phase 4 — Attention Tags & One-Click Session Routing Launchers (`rj-17`)
- [ ] **Attention Tag Parser (`tools/check_mail.py` & `tools/models.py`):**
  - Scan note frontmatter and body for attention markers:
    - `!IMPORTANT` $\rightarrow$ Add `tag:important`, render as glowing amber/rose priority pill (`⚠️ IMPORTANT`).
    - `!BLOATED` $\rightarrow$ Add `tag:bloated`, render warning chip (`📦 BLOATED (needs split)`).
    - `!INCOMPLETE` $\rightarrow$ Add `tag:incomplete`, render dashed warning chip (`📝 INCOMPLETE (early draft)`).
    - `!SPIKE` $\rightarrow$ Add `tag:spike`, render cyan pill (`⚡ SPIKE`).
- [ ] **One-Click Session Routing Launchers on Cards:**
  - Enhance card action bar in `morning_mail_inbox.html` with direct session launcher pills:
    - `[ 🔬 Grill-Lab ]`: Copies or dispatches `/grill-lab --topic "<title>"` directly to chat.
    - `[ 🧠 Deep Research ]`: Dispatches a targeted subagent research prompt for that card.
    - `[ 🔨 Draft Plan ]`: Launches an architectural plan drafting turn into `.agents/pipeline/plans/drafts/`.
  - In `file:///` mode, clicking pre-fills and highlights the prompt box for 1-click clipboard copying; in extension mode, dispatches directly via `window.sidecar.agent.sendMessage()`.

## Phase 5 — `/mail-add` with Position Splice
- [ ] `tools/mail_add.py "<title>" [--body TEXT] [--pillar NK] [--priority P1] [--after rj-05 | --rank N]`: writes a new Vault Inbox note AND inserts a card into `rejigger.json` at the requested rank, renumbering subsequent `rank`s (no manual multi-file renumbering). `--dry-run` prints the diff.
- [ ] Skill stub `.agents/skills/mail-add/SKILL.md` (triggers: `/mail-add`, "add to mailbag").

## Phase 6 — Verification & Closeout
- [ ] `tools/tests/test_intake_v2.py` (stdlib `unittest`): overflow split at 4+ ideas; shared `source_note`; quick-note title; attention tags parsed; Tabled scan; `mail_add --after` renumbers ranks contiguously.
- [ ] `python tools/check_mail.py --archive --no-browser` → production board renders with attention tags and session buttons; `audit_sidecar.py` PERFECT PASS on production + sandbox boards.
- [ ] Mirror tools to `c:\Dev\nkai\tools\`; `python tools/install_nkai_plugin.py`.
- [ ] BACKLOG Flight 7 → COMPLETED; mailbag `rj-07` & `rj-17` → COMPLETED.
- [ ] Commits: `feat(mailbag): intake & triage v2 — overflow, quick notes, attention tags, session routing, tabled folder, mail-add` (both repos).
