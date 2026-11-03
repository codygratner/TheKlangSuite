---
name: context-extract
description: Extracts a lightweight Markdown snapshot of target files, schemas, directory layout, and spec doc for external LLM planning. Defaults to prompting Gemini Web to have an interactive architectural interview before planning, or directly outputs an implementation plan if --plan (-p) is supplied. Supports flags --clipboard (-c), --open (-o), and --plan (-p). Triggers on `/contextextract`.
---

# Context Extractor for External Planning

## Goal
Assemble a clean, condensed snapshot of relevant code, interfaces, directory layout, and primary project spec into `context_snapshot.md`, then manage clipboard copying and launching the external planning workspace.

## Operational Constraints
- **Strictly No Implementation:** Do not propose architectures or write feature code. Act strictly as an extraction compiler.
- **Single Process:** Do not launch parallel sub-agents or background test runs.
- **Token-Efficient Discovery:** Never perform unbounded whole-repo reads. Use fast indexing (`rg`, `find`, glob) with hard match caps.

## Workflow

### 0. Automated Deterministic Extraction (Primary Execution)
Instead of searching and reading multiple large source files into LLM chat context, execute the zero-token Python snapshot compiler:
```bash
python tools/context_extract.py [flags]
```
- **Conversational Discovery (Default):** `python tools/context_extract.py -c -o`
- **Direct Plan Mode:** `python tools/context_extract.py -p -c -o`
- **Named Scope Preset:** `python tools/context_extract.py --scope oscillators -c`
- **Explicit Target Files:** `python tools/context_extract.py --files source/FileA.h source/FileB.h -c`
- **Terse Shield Mode:** `python tools/context_extract.py --terse`

The Python compiler automatically:
1. Resolves targets from the manifest in `.agents/extract_clues.md` (or CLI overrides) with 0 LLM token overhead.
2. Ingests the spec document, builds the directory tree, and strips boilerplate license headers.
3. Assembles `context_snapshot.md` in <100ms.
4. Copies directly to the OS clipboard (`-c`) and opens `https://gemini.google.com` (`-o`).
5. Displays the Pro Model reminder banner.

### 1. Flag Detection & Command Parsing
Inspect the invocation:
- Flag `--clipboard` or `-c`: Auto-copy markdown to OS clipboard.
- Flag `--open` or `-o`: Auto-launch `https://gemini.google.com` in default browser.
- Flag `--plan` or `-p`: Direct Plan Mode. Prompts Gemini Web to immediately output the full phased implementation plan without an initial conversation.
- **Default Mode (Omitted `--plan` / `-p`)**: Conversational Discovery Mode. Prompts Gemini Web to engage in an interactive architectural discussion and design interview first before drafting a plan.


### 2. Locate Project Spec
Search root and `docs/` for `*SPEC*.md`, `*ARCHITECTURE*.md`, `*DESIGN*.md`, or `README.md`. Read and reserve this document.

### 3. Scope Determination & Discovery
- **Case A: Explicit Paths Provided:** Use specified files.
- **Case B: Feature or Concept Described:**
  1. Identify 2–4 search keywords.
  2. Use file and content search (`rg`) to find candidates.
  3. Select the top 3 to 7 most relevant files (prioritize headers, parameter layouts, interfaces, DSP blocks).
- **Case C: Blank Trigger:** Prompt the user for target files or feature description.

### 4. Focused Directory Map
Run a shallow file tree (depth 2–3) restricted to the primary source tree (`Source/`, `src/`, `include/`), excluding `.git`, `build`, `cmake-build-*`, `JuceLibraryCode`, `node_modules`, and binary assets.

### 5. Read Target Files
Read the identified files, stripping boilerplate license headers to preserve tokens.

### 6. Write `context_snapshot.md`
Create or overwrite `context_snapshot.md` at project root:

# Planning Context Snapshot
**Timestamp:** <YYYY-MM-DD HH:MM>  
**Mode:** <Conversational Discovery (Default) | Direct Plan (--plan / -p)>

<!-- If Default (Conversational Discovery Mode): -->
> [!IMPORTANT]
> **INSTRUCTIONS FOR GEMINI (CONVERSATIONAL DISCOVERY MODE):**
> - **DO NOT jump straight into writing the final implementation plan.**
> - The user wants to explore and brainstorm this feature with you first.
> - Review the codebase context below, discuss architectural design options and trade-offs, ask clarifying questions about UX/DSP details, and brainstorm with the user.
> - Only output the structured, phased implementation plan (formatted for Antigravity's `/pasteplan` skill) when the user explicitly directs you to generate the plan.

<!-- If Direct Plan Mode (--plan or -p): -->
> [!IMPORTANT]
> **INSTRUCTIONS FOR GEMINI (DIRECT PLAN MODE):**
> - The task context below is 100% self-contained.
> - Immediately produce a clean, phased implementation plan formatted for Antigravity's `/pasteplan` skill.

## 1. Project Specification & Architecture
*(Source: `<path/to/SPEC.md>`)*
```markdown
<Contents document of primary spec>
```

## 2. Directory Tree
```text
<Shallow source tree>
```

## 3. Core Schemas, Structs & Interfaces
```cpp
<Extracted enums, layout parameter structs, typedefs>
```

## 4. Source Files
### `<path/to/file.ext>`
```<language>
<File content>
```

### 7. Handoff & Action Execution

Helper commands:
- **Clipboard:**
  - Windows: `powershell -Command "Get-Content context_snapshot.md -Raw | Set-Clipboard"`
  - macOS: `pbcopy < context_snapshot.md`
  - Linux: `xclip -selection clipboard < context_snapshot.md` (or `wl-copy < context_snapshot.md`)
- **Browser:**
  - Windows: `cmd /c start https://gemini.google.com`
  - macOS: `open https://gemini.google.com`
  - Linux: `xdg-open https://gemini.google.com`

Always display this visual banner:

# ⚠️ **REMINDER: VERIFY / SELECT THE PRO MODEL IN GEMINI WEB** ⚠️

- If `--clipboard` and `--open` were supplied: Execute both, display banner, confirm.
- If only `--clipboard`: Execute copy, display banner, confirm.
- If only `--open`: Launch browser, display banner, confirm.
- If neither flag was supplied: Display banner and prompt user for clipboard/browser actions.
