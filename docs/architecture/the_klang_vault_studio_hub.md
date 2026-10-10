# The Klang Vault: Private Studio Hub & Multi-Repo Architecture

> **Architectural Specification**: Decoupling studio communications, raw thoughts, and Pydantic data pipelines from the native C++ audio plugin factory.

---

## 🏛️ The Multi-Repo Division of Concerns

The Klang Suite ecosystem divides engineering and studio operations across distinct, decoupled repositories:

1. **`TheKlangSuite` (`c:\Dev\TheKlangSuite`)**:
   - **Role**: Native C++20 / JUCE 9 Audio Factory Floor.
   - **Artifacts**: VST3/AU plugins (`TKF`, `TKP`, `TKM`, `TKR`), unit tests (`dsp_tests`, `gui_tests`), and APVTS control schemas.
   - **Invariant**: Strictly zero external Python dependencies; 100% C++ audio thread safety.

2. **`TheKlangVault` (`C:\Dev\TheKlangVault`)**:
   - **Role**: Private Studio Hub, Raw Notes Mailbox & Data Engineering.
   - **Artifacts**: Obsidian notes, voice memo transcriptions, session ledgers (`mail_queue.json`), and Pydantic data validation pipelines.
   - **Version Control**: Private Git repository protecting personal studio ideas from public exposure.

3. **`TheKlangResearch` (`C:\Dev\Research`)**:
   - **Role**: Centralized Knowledge, Eternal DSP Mathematics & Soul Harvests.
   - **Artifacts**: Public RFCs, multi-agent research boards, and hardware schematics.

4. **`nkai` (`c:\Dev\nkai`)**:
   - **Role**: Asymmetric Sidecar Framework & CLI Tooling.

---

## 🛡️ Obsidian Sync & Git Coexistence Guardrails

To prevent synchronization race conditions between the official Obsidian Sync service and local Git version control:

1. **Git Exclusions (`.gitignore`)**:
   - Root `.gitignore` in `TheKlangVault` strictly excludes volatile Obsidian workspace state (`.obsidian/workspace.json`, `.obsidian/workspace-mobile.json`), internal caches (`.obsidian/cache/`), and trash (`.trash/`).
2. **Obsidian App Exclusions (`.obsidian/app.json`)**:
   - `userIgnoreFilters` excludes Git metadata (`.git/`), Python tooling (`tools/`, `*.py`, `*.pyc`), and virtualenvs (`.venv/`) from Obsidian's Quick Switcher (`Ctrl+O`), Global Search, and Graph View.
3. **Obsidian Sync Invariant**:
   - Obsidian Sync natively ignores all dotfiles and dotfolders (`.git/`, `.gitignore`, `.venv/`, `.agents/`). Git operations are 100% invisible to Obsidian Sync.
4. **Automated Agent Ownership**:
   - Agents permanently own and proactively maintain both `.gitignore` and `.obsidian/app.json` whenever adding new directories or data caches. The user never needs to manage these files manually.
