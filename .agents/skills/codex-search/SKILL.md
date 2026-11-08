---
name: codex-search
description: Performs sub-millisecond full-text searches across The Cosmic Codex, The Klang Suite docs, and TheKlangDSP specs using SQLite FTS5 with BM25 relevance ranking and highlighted snippets. Triggers on /search, /find, /lookup, /codex-search, "search vault", "find in vault", "search specs", or "lookup in codex".
---

# 🔍 Codex Search Microskill (`/search`, `/find`, `/lookup`)

> **Tooling Command:** `python tools/search.py "<query>"`  
> **Fallback Absolute Command:** `python C:/_/Vault/tools/search.py "<query>"`  
> **Database Location:** `C:\_\Vault\.agents\index\knowledge.db` (SQLite FTS5, 17MB, ~2,350 docs)  
> **Execution Speed:** $\approx 1.5\,\text{ms}$ query latency  
> **Model Tier:** Tier 2 Flash (High) — 🟢 ZERO-TOKEN CHURN  

---

## 🎯 Purpose & Invariant

Whenever the user or agent needs to locate a DSP formula, hardware lineage, sovereign code name, lore note, rule, or architectural decision across **The Cosmic Codex (`Vault`)**, **The Klang Suite (`TKS`)**, or **`TheKlangDSP`**:

1. **NEVER use manual multi-file grep or ad-hoc Python search scripts.** They cause process latency, burn tool turns, and dump cluttered tokens into context.
2. **ALWAYS use `tools/search.py` first.** It executes in $<2\,\text{ms}$, computes BM25 relevance scores, and returns exact file paths with highlighted context snippets.

---

## 🚀 Commands & Workflows

### 1. Basic Keyword Search
```powershell
python tools/search.py "alpha filter"
```
Returns the top 5 matches with clickable `file:///` links, titles, category badges, and highlighted context snippets.

### 2. Category Filtered Search
Narrow search scope to specific subdirectories (e.g. `dsp`, `specs`, `manuals`, `library`):
```powershell
python tools/search.py "vactrol" --category dsp
```

### 3. Limited Result Search (Ultra-Concise)
Retrieve only the top 1–3 matches to preserve maximum context:
```powershell
python tools/search.py "steiner parker diode" --limit 3
```

### 4. Index Status & Statistics
Inspect total indexed document count, database file size, and per-repository breakdown:
```powershell
python tools/search.py --status
```

### 5. Force Re-Index Rebuild
Wipes and rebuilds the SQLite FTS5 database from scratch across all 3 source trees:
```powershell
python tools/search.py --reindex
```
*(Note: Incremental updates occur automatically on regular queries when files are modified).*

---

## 🛡️ Response Formatting Protocol

When presenting search results to the user:
1. Always format paths as clickable GitHub-style Markdown links using `file:///` URLs (e.g. `[vandal_park_dsp.md](file:///C:/Dev/TheKlangSuite/docs/specs/dsp/vandal_park_dsp.md)`).
2. Quote the highlighted context excerpt to confirm exact relevance.
3. If an answer requires reading the document, open only the specific matching file using `view_file` rather than searching blindly.
