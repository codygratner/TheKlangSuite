---
name: web-qa
description: Audits, lints, and verifies Antigravity interactive sidecars and webviews (.html), detecting HTML tag imbalance, JavaScript syntax errors (e.g. invalid Unicode escapes in template strings), click-trapping overlay pointer-events, Chromium file:// cache issues, strict-mode event errors, and Web Audio API autoplay sandbox compliance. Triggers on `/qa`, `qa`, `/webqa`, `/web-qa`, `/sidecarqa`, `/sidecar-qa`, "audit sidecar", "web qa", "check sidecar", or "fix navigation". When triggered with "qa" during web/sidecar work, invokes web-qa; during audio C++ DSP work, delegates to build-validate and audiothread-guard.
---

# Web & Sidecar Quality Assurance (Web-QA)

## Goal
Audit, validate, and repair Antigravity interactive sidecar HTML canvases and Electron/Chromium webview applications to guarantee 100% offline functionality, seamless navigation, correct styling, and zero runtime JavaScript errors.

## The 6 Webview Hazard Categories

1. **JavaScript String & Template Literal Escapes (Critical SyntaxError)**:
   - Pasting Windows paths like `c:\Dev\TheKlangSuite\.agents\sidecar\ui_ux...` inside JS template literals generates `\u` escapes. If not followed by 4 hex digits, JavaScript throws a fatal `SyntaxError: Invalid Unicode escape sequence`, completely halting script execution and killing all button handlers (`switchTab`, `toggleDrawer`).
   - *Invariant*: All Windows file paths in JS strings/templates MUST use forward slashes (`c:/Dev/...`) or properly escaped double backslashes (`c:\\\\Dev\\\\...`).

2. **HTML Tag Imbalance**:
   - Mismatched `<div>` tags break layout hierarchies, causing sidebar items or tab panes to bleed into each other or collapse.
   - *Invariant*: `opens == closes` (`diff = 0`) across all container tags (`div`, `section`, `main`, `aside`).

3. **Strict-Mode Event Handling**:
   - Calling `event.target` inside functions (e.g. `switchTab(tabId)`) without passing `event` explicitly throws `ReferenceError: event is not defined` in Electron/Chromium strict mode.
   - *Invariant*: Never rely on window-level global `event`. Pass `event` explicitly (`onclick="myHandler(event, ...)"`) or query targets deterministically via `document.querySelector`.

4. **Click-Trapping Overlay Pointer Events**:
   - Modals and slide-over drawers with full-screen backdrop overlays (`.drawer-overlay`) can silently intercept all mouse clicks across the entire screen if `pointer-events: none` is missing when inactive.
   - *Invariant*: Inactive overlays MUST have `visibility: hidden; pointer-events: none; opacity: 0;`.

5. **Chromium `file:///` Memory Cache Trap**:
   - Standard `location.reload()` in Chromium/Electron often reloads from memory cache rather than reading the updated file on disk.
   - *Invariant*: Always use `forceReloadCanvas()` cache-busting timestamp navigation:
     ```javascript
     const url = new URL(window.location.href);
     url.searchParams.set('_t', Date.now());
     window.location.replace(url.toString());
     ```

6. **Web Audio API Autoplay & Studio Monitor Safety**:
   - Chromium blocks `AudioContext` until explicit user interaction.
   - High-gain synthesizer testing can burst loud test clicks into studio monitors.
   - *Invariant*: Audio haptics MUST default to 100% muted via `localStorage`, and provide Base64 PCM WAV fallback or visual ripple fallback.

7. **Loopback Bridge IPC & PNA/CORS Sandboxing (Private Network Access)**:
   - Electron/Chromium webviews fetching `http://127.0.0.1` from `file:///` send `Origin: null` and trigger Private Network Access (PNA) preflight checks.
   - Localized status containers on inactive/hidden tabs (e.g. `#audio-toast`) remain invisible to the user, creating the false impression of an action failure or bridge stall.
   - *Invariant*: Notification toasts MUST render in a persistent floating HUD container (`position: fixed; bottom: 24px; right: 24px; z-index: 99999`), loopback HTTP servers must return `Access-Control-Allow-Private-Network: true` on `OPTIONS`, and subagents must stage a receipt in `.agents/pipeline/communique/subagent_receipt.md` when overcoming novel platform traps.

8. **The Phantom Bridge & Dead-Letter Loopback Fallacy**:
   - In `file:///` artifact mode, an external HTTP loopback socket (`127.0.0.1:4040`) writing to disk cannot wake up an event-driven LLM session paused on `USER_INPUT`. Claiming "Sent to Agent!" creates a broken feedback loop where the user waits indefinitely for a sleeping agent.
   - *Invariant*: Data belongs in the canvas; actions belong in the clipboard. Informational queries (`[ ⚖️ Pros/Cons ]`, `[ 🔬 Research ]`) MUST expand via in-canvas progressive disclosure accordions. Prompts requiring LLM generation MUST honestly copy to clipboard and flash `📋 Copied! Paste in Chat`.

---

## Operational Constraints & Access Policies

- **Pre-Authorized Web Access Invariant**: Web QA subagents are strictly pre-authorized to use `search_web` and `read_url_content` regardless of any general project offline guardrails. Debugging obscure Chromium/Electron webview sandbox restrictions, Web Audio autoplay policies, and CSS rendering quirks requires searching upstream documentation, Chromium bug trackers, and developer forums.
- **Model Tiers & Invocation Flags**:
  - `/web-qa --flash`: Forces a fast, lightweight Tier 2 Flash subagent (`Model="flash"`). Optimal for HTML/CSS layouts, DOM balance, quick script regexes, and low quota burn.
  - `/web-qa --pro`: Forces a deep-reasoning Tier 1 Pro subagent (`Model="pro"`). Reserved for obscure Electron webview sandboxing, complex state machine debugging, or procedural Base64 PCM audio synthesis.
  - `/web-qa --no-web`: Air-gapped offline mode. Restricts QA strictly to the local deterministic static linter (`audit_sidecar.py --fix`) and forbids subagents from invoking web search tools.
  - `/web-qa` (Default / No Flag - **Autonomous Escalation Ladder**):
    1. *Stage 1 (50ms)*: Run static linter `audit_sidecar.py --fix`.
    2. *Stage 2*: If runtime/visual issues persist, dispatch a Tier 2 Flash subagent with web access.
    3. *Stage 3 (2-Strike Auto-Escalation)*: If Flash fails to resolve the issue on strike 1 or determines it is an obscure Chromium sandbox limitation, automatically escalate to a Tier 1 Pro subagent.

---

## Workflow

### 1. Deterministic Static Linting (scripts/audit_sidecar.py)
Run the automated audit script on the target file:
```bash
python .agents/skills/web-qa/scripts/audit_sidecar.py <path-to-html>
```
To automatically fix common escape hazards:
```bash
python .agents/skills/web-qa/scripts/audit_sidecar.py <path-to-html> --fix
```

### 2. Subagent Dispatch & Escalation Ladder
Based on flags or autonomous determination:
- **Flash Mode (`--flash` or Default Step 2)**:
  ```python
  invoke_subagent(
      TypeName="self",
      Role="Web QA & Sidecar Engineer",
      Model="flash",
      Prompt="Inspect <path-to-html>. Web access authorized. Diagnose and repair..."
  )
  ```
- **Pro Mode (`--pro` or Escalation Step 3)**:
  ```python
  invoke_subagent(
      TypeName="self",
      Role="Pro QA Web Audio & Webview Specialist",
      Model="pro",
      Prompt="Deeply inspect <path-to-html> for Electron webview sandbox restrictions..."
  )
  ```

### 3. Verification & Deployment
- Re-run `audit_sidecar.py` to confirm zero issues (`diff = 0`, zero syntax hazards).
- Mirror the file to the Antigravity Brain Artifacts directory (`brain/<conversation-id>/`) and the Research archive.
- Output the clickable artifact link in chat:
  `👉 **[Open <Sidecar Name> Canvas](file:///<path>)**`

### 4. Institutional Memory & Autopsy Receipt (The 80/20 Gate)
- If the QA task solved a **novel, non-obvious platform trap** (e.g. Electron/Chromium webview sandboxes, audio-thread real-time allocations, IPC bridge CORS/PNA policies), the subagent or engineer MUST append a concise receipt to `.agents/pipeline/communique/subagent_receipt.md`.
- Routine syntax fixes, linting passes, and CSS color adjustments DO NOT require receipts.
- The staged receipt is ingested, synthesized into `TheKlangResearch/qa_autopsies/`, and indexed during the next `/update-docs --deep` pass.
