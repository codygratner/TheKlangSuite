---
name: visual-grill-me
description: Conducts an interactive architectural & UI design interview paired with a live-updating, side-by-side HTML+CSS visual mockup artifact in the side panel. Triggers on `/grilllab`, `/grill-lab`, `/vlab`, `/vrill`, `/vrillme`, `/ggrill`, `/visualgrillme`, `/visual-grill-me`, `/grill-visual`, "grill lab", "visual grill me", or "visual interview".
---

# Visual Grill Me — Interactive Design Interview with Live HTML Sidecar

## Goal
Conduct a rigorous, interactive design interview (`/grill-me`) for UI/UX, layouts, and interactive components while maintaining an interactive, live-updating HTML+CSS visual artifact in the IDE's side panel. For each question asked via `ask_question`, the visual artifact immediately updates to render real-time comparative mockups of the proposed options (Option A vs Option B vs Option C) using Tailwind CSS and interactive widgets.

---

## Operational Guardrails & Architecture
1. **N'kai-First Visual Default & The "Ready Handshake" (MANDATORY)**:
   - **Default Invocation (N'kai-First)**: By default, `/grill-me`, `/grilllab`, `/vlab`, `/vrill`, etc., automatically launch in **Grill Lab** mode, initializing the interactive sidecar canvas.
   - **The `--chat` / `--text-only` Fallback Flag**: When invoked with `--chat`, `--text-only`, or when operating in a headless terminal AI CLI (Claude Code CLI, Aider), skip HTML sidecar generation entirely. Conduct the architectural interview directly in chat with numbered options (`[1]`, `[2]`, `[3]`) or `ask_question` modals, capturing decisions directly into `PLAN.md`.
   - **Dual-Storage Sidecar Storage**: When running in default visual mode, the sidecar canvas MUST always be written to BOTH:
     1. `<appDataDir>\brain\<conversation-id>\visual_grill_me.html` (artifact directory for live side-panel rendering).
     2. `c:\Dev\TheKlangSuite\.agents\sidecar\visual_grill_me.html` (gitignored repo directory for persistence).
   - **Prominent Kickoff Link (MANDATORY ARTIFACT URI)**: Always output a prominent, clickable file URI link pointing to the **Artifact Directory** at the very top of the kickoff turn:
     `👉 **[Open Grill Lab Canvas](file:///<appDataDir>/brain/<conversation-id>/visual_grill_me.html)**`
     *(Antigravity opens workspace links in the code editor, but renders artifact links in the interactive side-panel Webview).*
   - **The "Sidecar Ready Handshake"**: Before blasting the user with full design questions and comparative mockups, output the artifact link and pause with a clear prompt:
     *"Please open the Grill Lab Canvas link above in your side panel, then reply 'ready' to begin!"*
     Once the user confirms the canvas is open and visible, proceed directly into Question 1.
2. **Universal Scope & Dual Operating Modes**:
   - **UI Sandbox Mode**: Used for synth controls, sliders, cards, and meters. Displays real-time Web Audio audition synth, canvas oscilloscope, audio tick clicks, and theme swatches.
   - **Architecture & Knowledge Mode (Antigravity Dark Blue)**: Used for non-UI topics (documentation, schemas, ADRs, indexing, pipeline workflows). Strictly eliminates neon glows and fluorescent accents. Matches the native Antigravity IDE dark blue-tinged slate theme (Body: `#0e121a`, Cards: `#151b26`, Recessed: `#10141e`, Borders: 1px `#222c3d`, Accents: clean IDE blue `#3b82f6` and soft slate `#8b9cb5`). Uses clean Inter typography for prose and monospace for code. Automatically hides audio scope and synth widgets to maximize reading comfort.
3. **Conversational Interleaving & Live Tuning (CRITICAL)**:
   - The interview is never a rigid, lock-step rail. The user can pause anytime in chat to ask side questions, request visual/font/theming adjustments to the sidecar, or provide extra background context.
   - The agent responds directly to side questions and updates the canvas live, keeping the active question and decision state perfectly preserved.
4. **The "BAR-B-Q&A" Spirit**:
   - Also known affectionately as the **BAR-B-Q&A**! Keep the session engaging, visual, collaborative, and fun.
5. **Tier 2 Flash High Universal Baseline**:
   - Visual interviews, UI layout brainstorming, and HTML artifact authoring strictly use **Gemini 3.8 Flash (Thinking: High)**. Zero Pro quota burn required.
6. **Persistent Sidecar Storage**:
   - The primary visual sidecar is written to: `c:\Dev\TheKlangSuite\.agents\sidecar\visual_grill_me.html`.
   - A mirror copy is simultaneously synced to the conversation artifact directory for archival.
7. **Interactive Comparative Mockups & Sidecar Composer**:
   - Each interview question must correspond to a distinct visual section with live interactive controls styled with Tailwind CSS.
   - Provide an in-page **Decision & Write-In Composer** allowing the user to select options, write custom notes, and copy formatted markdown answers directly to their clipboard (`Ctrl+V` in chat).
8. **Web Worker & State Synchronization Architecture**:
   - Pair `visual_grill_me.html` with a companion state token `.agents/sidecar/visual_grill_me_state.json` (tracking `step`, `version`, `timestamp`).
   - Use an inline Blob Web Worker (`new Worker(URL.createObjectURL(blob))`) to monitor state and heartbeat pings off the main UI thread. Never use unthrottled `document.lastModified` loops.
   - Provide a prominent **Manual Refresh Button** (`[ ↻ Refresh View ]`) and Font Zoom Scaler (`[A-] [100%] [A+]`) in the page header.
   - Include an **Auto-Sync toggle** and a non-intrusive in-page toast banner (`#update-toast`) when new questions or layout updates arrive from chat.
9. **Modal-Free Flexibility**:
   - During Grill Lab sessions, prefer presenting questions in chat text and letting the user interact with the Sidecar Composer, avoiding modal deadlocks and keeping the IDE completely unlocked.
10. **The Asymmetric Sidecar Split (Terse Chat, Rich Sidecar)**:
    - Once the sidecar is active, chat responses MUST remain ultra-terse (1–3 sentences or quick prompts).
    - All verbose explanations, comparative tradeoff tables, architectural diagrams, and option mockups live inside the sidecar HTML canvas. This eliminates duplicate reading and preserves chat context.

---

## Workflow Steps

### Step 1: Initialize the Visual Canvas (or Chat Fallback)
1. **Fallback Branch (`--chat` / `--text-only`)**: If `--chat` or `--text-only` is passed, skip HTML creation entirely and proceed directly to **Step 2** using inline Markdown chat questions or `ask_question`.
2. **Default Visual Canvas**: Create `visual_grill_me.html` in both `<appDataDir>\brain\<conversation-id>\visual_grill_me.html` and `c:\Dev\TheKlangSuite\.agents\sidecar\visual_grill_me.html`.
3. Set up the baseline chassis container matching the active product aesthetic (e.g. Anthracite `#0d1117`, JetBrains Mono font, 1px hairline borders).
4. Present the initial state, output the clickable artifact link, and execute the Sidecar Ready Handshake.

### Step 2: Pose Visual Questions & Iterate Mockups
1. For each design topic (e.g., Slider Styling, Header Layout, Modulation Cables):
   - Update `visual_grill_me_preview.html` to render the comparative options (Option A, Option B, Option C) side-by-side with live interaction.
   - Immediately invoke `ask_question` presenting the architectural tradeoffs, industry standards (Bitwig, Phase Plant, Vital, Ableton), and ergonomics.
2. Wait for the user's response from the interactive modal.

### Step 3: Bake In the Chosen Design & Advance
1. Highlight the chosen design in the HTML artifact (adding an "APPROVED ✅" badge and active glowing border).
2. Document the decision in the working notes or implementation plan (`PLAN.md`).
3. Move to the next component question until the interface is fully designed.

### Step 4: Final Blueprint Synthesis & Archival
1. Conclude the interview by synthesizing the approved design decisions into exact drop-in C++ component structures, APVTS parameter layouts, or JSON schemas.
2. **Artifact Archival:** If the Grill Lab resulted in critical UI mockups or design specs, copy `visual_grill_me_preview.html` to `docs/history/research/<YYYY-MM-DD>_<feature>_mockup.html` and mirror it to the Obsidian Vault (`C:\Dev\TheKlangVault\Research\`).
3. **Vault Mirroring:** Create a Markdown summary in `C:\Dev\TheKlangVault\Research\<YYYY-MM-DD>_<feature>_mockup.md` with an Obsidian external link (`[Open Mockup Board](file:///...)`) to the HTML file.
4. **Cross-Linking:** Ensure `docs/history/DEV_HISTORY.md` and the Obsidian `C:\Dev\TheKlangVault\Docs\History_Index.md` link to the new mockup archive using native Obsidian `[[Wikilinks]]`.
5. Transition to Klang Industries for building.
