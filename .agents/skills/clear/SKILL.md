---
name: clear
description: Wipes the slate clean by forcefully clearing the active chat's context window down to 0 tokens via transcript log truncation. Triggers on `/clear`, `clear`, `/wipe`, `/wipe-context`, `/clear-context`, or `/context-clear`.
---

# Clear Context Skill (Wiping the Slate Clean)

---

## 🧭 4-Axis Modifier Parameters
| Axis | Flag | Options | Default | Description |
| :--- | :--- | :--- | :--- | :--- |
| **Scale** | `--scale`, `-s` | `light`, `standard`, `deep` | `standard` | Execution depth and output verbosity |
| **Stance** | `--stance`, `-M` | `standard`, `crucible` | `standard` | `crucible` activates strict verification and adversarial check |
| **Perimeter** | `--perimeter`, `-P` | `online`, `offline` | `offline` | Restricts network vs local execution |
| **Surface** | `--surface`, `-S` | `chat`, `board`, `receipt` | `chat` | Output representation channel |

## Goal
To wipe the agent's memory of the current chat window by truncating the underlying transcript logs down to 0 tokens, effectively starting a completely fresh chat without forcing the user to close or reopen tabs.

## Workflow
1. **Execute Truncation**: Immediately run the zero-token truncation script:
   ```powershell
   python C:\Users\codyg\.gemini\config\scripts\clear_transcript.py
   ```
2. **Output Success**: After the command succeeds, output a very brief confirmation message:
   > "Context cleared. I am ready for the next task."
3. **Resume (Cold Boot)**: If the user previously executed `/context-harvest`, they may follow up with `"resume"` to have the agent read [`ACTIVE_CONTEXT.md`](file:///c:/Dev/TheKlangSuite/ACTIVE_CONTEXT.md) and resume operations in 0 turns.

<!-- CACHE_ANCHOR: STATIC_PREFIX_END -->

---
## 📋 Dynamic Execution Context
[ACTIVE_TASK_SLOT]
