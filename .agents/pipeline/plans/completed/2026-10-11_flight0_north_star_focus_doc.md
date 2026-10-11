# 🔤 Flight 18: Universal UTF-8 Invariant (`rj-02`)

> **ID:** plan-008  
> **TARGET:** TOOLING_HARNESS (New Klang City) 🚀  
> **Topic Target:** Mailbag Card `rj-02` / BACKLOG Section 15 ("Universal UTF-8 Invariant")  
> **Status:** COMPLETED ✅  
> **Priority:** P0 (Top Priority — Quick Win)  
> **Target Branch:** `0.4.0-dev`  
> **Recommended Setting:** Gemini 3.8 Flash (Thinking: Low/Medium) — ⚡ MINIMAL  
> **Source Note:** [`C:\Dev\TheKlangVault\Inbox\Archive\docket-18.md`](file:///C:/Dev/TheKlangVault/Inbox/Archive/docket-18.md)  

---

## 🎯 Architectural Intent & Storyline

On Windows, Python defaults to the system ANSI code page (`cp1252` / `Windows-1252`) for standard console output and unadorned `open()` file I/O. Throughout our ecosystem sessions, emojis (🎙️, 🧠, 🧭, 📦, ⚡), box-drawing characters, and mathematical symbols have caused repeated `UnicodeEncodeError: 'charmap' codec can't encode character` crashes.

Additionally, Windows PowerShell 5.1's default `Get-Content` can misinterpret BOM-less UTF-8, creating visual artifact confusion even when files are clean on disk.

**Flight 18** establishes an unbreakable, automated **Universal UTF-8 Invariant**:
1. **Linter & Enforcement CLI (`tools/verify_utf8.py`)**: An automated scanner auditing all `.py` files across `TheKlangSuite` and `nkai` for stdout/stderr reconfiguration and explicit `encoding="utf-8"` on all file I/O.
2. **Ecosystem Script Sweep**: Ensuring 100% of Python tools configure UTF-8 output streams and declare `encoding="utf-8"` on every file read/write.
3. **PowerShell Harness Invariant**: Setting `$env:PYTHONUTF8 = "1"` and `[Console]::OutputEncoding = [System.Text.Encoding]::UTF8` in deployment and test scripts.
4. **Permanent Guardrail**: Codifying the invariant in `GEMINI.md`, `docs/GLOSSARY.md`, and `docs/CHEAT_SHEET.md`.

---

## 📋 Phase Breakdown

### Phase 1: Author Automated UTF-8 Audit Tool (`tools/verify_utf8.py`)
- [x] Author `c:\Dev\TheKlangSuite\tools\verify_utf8.py`:
  - Scans Python files across specified directories (`tools/`, `nkai/tools/`).
  - AST / regex inspection:
    1. Checks entrypoint files for `sys.stdout.reconfigure(encoding='utf-8')` / `sys.stderr.reconfigure(encoding='utf-8')`.
    2. Checks all `open(...)`, `Path.read_text(...)`, and `Path.write_text(...)` calls to ensure `encoding="utf-8"` is specified.
  - Supports `--fix` flag to automatically inject UTF-8 stdout reconfiguration where missing.
  - Supports `--strict` flag to fail with non-zero exit code if violations exist.
- [x] Mirror `tools/verify_utf8.py` to `c:\Dev\nkai\tools\verify_utf8.py` and `c:\Dev\nkai\bundle\tools\verify_utf8.py`.

### Phase 2: Sweep & Harden All Python Tools
- [x] Run `python tools/verify_utf8.py` across `c:\Dev\TheKlangSuite\tools` and fix any unconfigured or unencoded file operations:
  - `audit_audiothread.py`
  - `audit_legacy_parity.py`
  - `audit_sidecar.py`
  - `check_mail.py`
  - `install_nkai_plugin.py`
  - `models.py`
  - `nkai_api.py`
  - `nkai_bridge.py`
  - `nkai_ledger.py`
  - `nkai_serve.py`
  - `pcda_format.py`
  - `plan_queue.py`
  - `preset_audit.py`
  - `ramble_ingest.py`
  - `sync_roadmap_sidecar.py`
  - `test_nkai_api.py`
  - `verify_context_clues.py`
- [x] Run `python tools/verify_utf8.py` across `c:\Dev\nkai\tools` and fix any unconfigured scripts.
- [x] Run `verify_utf8.py --strict` across both repos to assert 100% clean pass.

### Phase 3: Update PowerShell Harnesses & Wrapper Scripts
- [x] Ensure `tools/sync_obsidian_vault.ps1`, `install_nkai_bundle.ps1`, and relevant runner scripts declare `$env:PYTHONUTF8 = "1"` and `[Console]::OutputEncoding = [System.Text.Encoding]::UTF8`.

### Phase 4: Codify Guardrails & Synchronize Invariants
- [x] Update `GEMINI.md`: Add "Universal UTF-8 Invariant" rule under Agent Guidelines.
- [x] Synchronize `docs/GLOSSARY.md` (Section 18: Universal UTF-8 Invariant).
- [x] Synchronize `docs/CHEAT_SHEET.md`.
- [x] Register `tools/verify_utf8.py` in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.

### Phase 5: Verification, Backlog Closeout & Multi-Repo Commits
- [x] Run `python tools/verify_utf8.py --strict` (confirm 100% pass across all repos).
- [x] Run `python tools/verify_context_clues.py` (confirm 100% link resolution).
- [x] Update `docs/BACKLOG.md`: Mark Flight 18 as `[x] COMPLETED`.
- [x] Update `context_clues_plan.md` to point to Flight 16+17 (QA Sandbox Board).
- [x] Commit changes cleanly in `TheKlangSuite` and `nkai`.
