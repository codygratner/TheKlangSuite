# Flight 23: Vault Graph & Naming Hygiene

**TARGET:** TOOLING_HARNESS  
**Role:** New Klang City Architecture & Tooling Engineering  
**Recommended Model:** Gemini 3.8 Flash (Thinking: High) — Tier 2 (Balanced Engineering & UI)  
> **Status:** COMPLETED ✅  

---

### Executive Overview & Architectural Dialectic
- **The One-Way Vault Mirror Law**: `TheKlangVault` is a generated, read-only mirror of the repositories. Content flows **repo ➔ vault ONLY**. Vault-side edits outside `Inbox/` are disposable and overwritten on the next sync.
- **Sync-Time Link Rewriting**:
  - `file:///` links that point to files mirrored into the vault (`docs/*.md`, `PLAN.md`) are converted during sync to vault links, joining the Obsidian graph and backlinks.
  - Links to non-mirrored native code assets (`.cpp`, `.h`, `.json`, `.py`, `.png`) are strictly left as `file:///` links.
  - Strips fragile `#L123-L145` line anchors (which break Obsidian markdown navigation), while preserving `#heading` anchors.
  - Defensively decodes `%20` into clean spaces.
- **Underscore Naming Invariant**:
  - Human captures on mobile retain loose phone-typed names with spaces in `TheKlangVault/Inbox/`.
  - When notes are archived by `tools/check_mail.py`, spaces are systematically converted to underscores (`safe_name = note.name.replace(" ", "_")`).
  - Title derivation converts `_` back to spaces so title-matching (`rejigger.json` sources) remains completely uninterrupted.
- **Inbox Decoupling & Dead Copy Elimination**:
  - Eliminates the legacy reverse-copy of vault notes into `docs/inbox/` in `tools/sync_obsidian_vault.ps1`.
  - Purges duplicate stale copies from `docs/inbox/` (preserving `docs/inbox/README.md`).

---

### Phase 1: Sync-Time Link Rewriter (`tools/vault_link_rewriter.py`)
- [x] Create `tools/vault_link_rewriter.py`:
  - `rewrite_links_for_vault(markdown_text: str, source_rel_path: str = "") -> str`
  - Converts `file:///.../docs/X.md` to `Docs/X.md` or `[[Docs/X|Label]]`
  - Strips line anchors (`#L\d+(-L\d+)?`)
  - Preserves heading anchors (`#heading-slug`)
  - Leaves non-markdown links (`.cpp`, `.h`, `.json`, `.png`) as `file:///`
  - Strict UTF-8 console stream reconfigures.
- [x] Update `tools/sync_obsidian_vault.ps1`:
  - Restate the One-Way Vault Mirror rule in the script header.
  - Apply link rewriting during the Repo -> Vault sync pass for `.md` files.

---

### Phase 2: Underscore Naming Invariant on Archive (`tools/check_mail.py`)
- [x] Update `tools/check_mail.py`:
  - Ensure `shutil.move` uses `found_src.name.replace(" ", "_")` when moving notes to `Archive/`.
  - Ensure note title resolution converts `_` to spaces when reading archived notes.

---

### Phase 3: Purge Duplicate `docs/inbox/` Copies
- [x] Remove Vault -> Repo copying from `tools/sync_obsidian_vault.ps1`.
- [x] Remove duplicate stale files in `docs/inbox/`, keeping `docs/inbox/README.md`.

---

### Phase 4: Unit Test Suite & Verification Gauntlet
- [x] Author `tools/tests/test_vault_link_rewriter.py`:
  - Test link rewriting for mirrored docs.
  - Test preservation of native code file links (`.cpp`, `.h`).
  - Test line anchor stripping (`#L10-L20` stripped, `#heading` preserved).
  - Test `%20` space decoding.
- [x] Run `python -m unittest discover tools/tests/` (100% pass rate).
- [x] Run `python tools/verify_utf8.py --strict` (100% compliant).
- [x] Update `docs/BACKLOG.md` marking Flight 23 as COMPLETED.
