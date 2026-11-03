# 🛠️ Zone 3: Tooling Harness & Sidecar Automation
> **Architectural Zone:** Zone 3 (Tooling Harness)  
> **Facility Owner:** New Klang City (Tooling & Ecosystem Architecture) 🚀  
> **Core References:** [`docs/GLOSSARY.md`](../docs/GLOSSARY.md) · [`docs/CHEAT_SHEET.md`](../docs/CHEAT_SHEET.md) · [`GEMINI.md`](../GEMINI.md)  

---

## ⚙️ Tooling Invariants & Standards

All scripts, servers, CLI wrappers, and generators residing in `tools/` MUST adhere to these architectural standards:

1. **Python Standard Library Baseline**:
   - Primary CLI tools (`nkai_serve.py`, `check_mail.py`, `pcda_format.py`, `nkai_ledger.py`) MUST run using standard library modules only (`sys`, `os`, `pathlib`, `argparse`, `http.server`, `json`).
   - UTF-8 stdout hardening is mandatory on Windows (`sys.stdout.reconfigure(encoding='utf-8')`).
2. **Global Pydantic Data Services**:
   - Advanced validation tools (`preset_audit.py`, `plan_queue.py`, `nkai_api.py`, `agy_models.py`, `agy_bridge.py`, and `agy_autopsy.py`) use the global Pydantic v2 environment (`pip install pydantic`), avoiding fragile virtualenv setups.
3. **Zero Contamination of Factory Floor**:
   - Tooling scripts, HTML sidecars, and automation runners belong strictly to New Klang City (`TARGET: TOOLING_HARNESS`).
   - Under NO circumstances should tooling or web tasks be dispatched to Klang Industries (`TARGET: FACTORY_FLOOR`).
4. **Node CLI Wrappers**:
   - Node tooling (`bin/nkai.js`) must remain lightweight, fast, and zero-dependency (`http`, `fs`, `path`).

---

## 🧭 Zonal Governance & Anti-Pattern Prohibition

* **No Rogue Per-Folder Blackboxes**: Tooling harness standards remain centralized here in `tools/README.md`.
* **No Stray Scripts in Root**: All Python, PowerShell, and Node scripts must live in `tools/` or `TheKlangVault/tools/`, never loose in the repository root.
