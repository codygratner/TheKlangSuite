# N'kai Universal AI Harness Portability (Topic 6.13)

## 1. Architectural Intent & Scope

The **N'kai Asymmetric Sidecar Framework** (`nkai`) provides visual sidecar canvases, interactive interview labs (`visual-grill-me`), deep architectural research boards (`deep-research`), structured post-mortem triage (`post-mortem`), and webview linters (`web-qa`) across the Four Pillars (`TheKlangSuite`, `ToadTracker`, `nkai`, `TheKlangResearch`).

Initially designed for Google Antigravity's side-panel webview, N'kai's asymmetric workflow—ultra-terse chat interaction in the AI agent prompt paired with a rich, interactive HTML visual canvas—is a transformative pair-programming pattern. However, modern AI software development is inherently heterogeneous. Developers and teams operate across multiple AI coding harnesses:
- **Google Antigravity IDE**: Rich side-panel webview with the native Sidecar SDK (`window.sidecar`).
- **Terminal AI CLIs**: Claude Code CLI, OpenAI Codex CLI, Aider, and custom shell agent loops where the primary interface is a command-line terminal.
- **Desktop AI Editors**: Cursor, Windsurf, VS Code, and Zed running editor-integrated split webviews.

### The Problem: Harness Coupling & File Protocol Limitations
1. **API Lock-In**: Code written exclusively for `window.sidecar.agent.sendMessage()` fails silently or throws runtime exceptions when opened in a standard browser or alternative editor.
2. **CORS & Audio Playback Restrictions**: Opening complex HTML artifacts via `file:///` can run afoul of Chromium/WebKit security sandboxes (e.g. loading local `.wav` audio previews, `.json` session packages, or web workers).
3. **Terminal Pairing Friction**: Terminal-based agents (such as Claude Code or Aider) lack a native embedded browser pane, requiring developers to manually locate, serve, and open sidecar artifacts in their browser.

### The Solution: Option B (Decoupled Interface Layer)
Formally approved under **Topic 6.13**, N'kai decouples its presentation and action-dispatching layer from any specific IDE runtime. It introduces:
1. **Tri-Mode Environment Detection**: Dynamic runtime detection between `extension`, `file`, and `server` environments.
2. **Adaptive Dispatching Pipeline**: Gracefully routing actions via native SDK messaging, local HTTP loopbacks, or clipboard/prompt-box sync.
3. **Zero-Dependency CLI Server**: Standard-library Python (`nkai_serve.py`) and Node.js (`bin/nkai.js` / `npx nkai serve`) tools that resolve ports automatically, register audio/web MIME types, and launch the system browser.

---

## 2. Tri-Mode Environment Detection Protocol

N'kai sidecars declare a global environment adapter `window.NkaiEnv` that deterministically probes its hosting context on initialization.

```
┌────────────────────────────────────────────────────────────────────────┐
│                        RUNTIME DETECTION PROBE                         │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │
           ┌────────────────────────┼────────────────────────┐
           ▼                        ▼                        ▼
  window.sidecar present?   window.location ==       window.location ==
  (agent.sendMessage ok)         "file:"               "http(s):"
           │                        │                        │
           ▼                        ▼                        ▼
  ┌──────────────────┐     ┌──────────────────┐     ┌──────────────────┐
  │  MODE: EXTENSION │     │    MODE: FILE    │     │   MODE: SERVER   │
  │ Antigravity Side-│     │ Offline Browser/ │     │ CLI Local Server │
  │ Panel Extension  │     │ IDE file:/// URI │     │ (nkai_serve.py)  │
  └──────────────────┘     └──────────────────┘     └──────────────────┘
```

### Detection Logic & API Contract

```javascript
window.NkaiEnv = {
  detect: function() {
    if (typeof window !== 'undefined' && window.sidecar && typeof window.sidecar.agent?.sendMessage === 'function') {
      return 'extension';
    }
    if (typeof window !== 'undefined' && window.location) {
      if (window.location.protocol === 'file:') return 'file';
      if (window.location.protocol === 'http:' || window.location.protocol === 'https:') return 'server';
    }
    return 'file';
  },

  getMode: function() {
    return this.detect();
  },

  isExtension: function() {
    return this.detect() === 'extension';
  },

  isFile: function() {
    return this.detect() === 'file';
  },

  isServer: function() {
    return this.detect() === 'server';
  }
};
```

### Tri-Mode Behavior Matrix

| Feature | `extension` (Antigravity Plugin) | `file` (Offline `file:///`) | `server` (Local HTTP Server) |
| :--- | :--- | :--- | :--- |
| **Hosting Context** | Antigravity IDE UI Extension panel | Browser or editor webview via local file path | Browser pointed to `http://127.0.0.1:3030` |
| **Host Communication** | Native `window.sidecar.agent.sendMessage()` | Clipboard API + Prompt Box focus/highlight | Optional HTTP POST `/api/dispatch` + Clipboard |
| **CORS Policy** | Governed by IDE extension sandbox | Strict local origin (no cross-file fetch) | Standard same-origin HTTP (fetch permitted) |
| **Audio/WAV Assets** | Permitted via webview scheme | May require user click interaction | 100% permitted with proper `audio/wav` headers |
| **State Persistence** | `localStorage` + session files | `localStorage` | `localStorage` + session files |

---

## 3. Adaptive Action Dispatching

When a user interacts with a sidecar (e.g., clicking a verdict chip in `/postmortem`, choosing an architectural fork in `/visual-grill-me`, or selecting a preset in `/preset-lab`), `window.NkaiEnv.dispatchAction()` dynamically routes the message based on the active mode:

```javascript
window.NkaiEnv.dispatchAction = function(actionPayload) {
  const env = this.detect();
  const text = typeof actionPayload === 'string' ? actionPayload : JSON.stringify(actionPayload, null, 2);

  // 1. Extension Mode: Direct Agent Messaging
  if (env === 'extension') {
    try {
      window.sidecar.agent.sendMessage(text);
      showToast('Dispatched message directly to Agent!');
      return true;
    } catch (err) {
      console.warn('[NkaiEnv] Extension dispatch failed, falling back to clipboard:', err);
    }
  }

  // 2. Server Mode: Loopback POST with Clipboard Sync
  if (env === 'server' && typeof fetch === 'function') {
    fetch('/api/dispatch', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ action: actionPayload, timestamp: Date.now() })
    }).then(res => {
      if (res.ok) {
        showToast('Dispatched to local CLI server & copied to clipboard!');
      }
    }).catch(() => {
      // Fallback silently if /api/dispatch not captured
    });
  }

  // 3. File / Default Mode: Clipboard Copy + Prompt Box Highlight
  return this.copyToClipboard(text);
};
```

### Visual HUD Toast Notification
To ensure the developer always receives deterministic feedback regardless of mode, a non-intrusive HUD toast element (`#nkai-toast`) pops up in the bottom-right corner of the canvas, displaying the dispatch status for 2.5 seconds.

---

## 4. Zero-Dependency CLI Serving Architecture

To support terminal-first workflows (Claude Code CLI, Aider) and cross-IDE editors without forcing users to configure complex web servers or install bulky dependencies, N'kai provides two zero-dependency CLI server implementations:
1. `tools/nkai_serve.py` (Python 3 standard library)
2. `bin/nkai.js` (Node.js standard library, executable via `npx nkai serve`)

### Core Functional Requirements
- **Standard Library Only**: 0 third-party packages. No `flask`, `express`, `fastapi`, or `serve`. Runs immediately on any stock Python 3.8+ or Node.js 16+ installation.
- **Port Collision Recovery**: Defaults to port `3030`. If port 3030 is already bound, automatically scans and binds the next available port (`3031`, `3032`, ...), preventing startup crashes.
- **Strict Audio & Web MIME Mapping**:
  - `.html` $\to$ `text/html; charset=utf-8`
  - `.js` $\to$ `application/javascript`
  - `.css` $\to$ `text/css`
  - `.json` $\to$ `application/json`
  - `.wav` $\to$ `audio/wav`
  - `.svg` $\to$ `image/svg+xml`
  - `.png` $\to$ `image/png`
- **Auto-Browser Launch**: Automatically resolves the default system browser and opens the URL directly to the target sidecar file upon successful socket binding (suppressible via `--no-browser`).
- **Target Resolution**: Supports passing a direct `.html` file path or a directory containing sidecar assets.

### CLI Command Contracts

#### Python (`tools/nkai_serve.py`)
```bash
# Serve specific sidecar artifact with auto-browser launch on port 3030
python tools/nkai_serve.py .agents/sidecar/visual_grill_me.html

# Serve a directory on custom port without opening browser
python tools/nkai_serve.py docs/history/research/ --port 8080 --no-browser

# Bind to all network interfaces (for LAN/mobile testing)
python tools/nkai_serve.py .agents/sidecar/ --host 0.0.0.0
```

#### Node.js (`npx nkai serve`)
```bash
# Zero-install npx invocation
npx nkai serve .agents/sidecar/visual_grill_me.html

# Custom port and headless flag
npx nkai serve --port 4000 --no-browser
```

---

## 5. Terminal AI Pairing Patterns & Workflows

### Pattern 1: Claude Code CLI Pairing
```
┌────────────────────────┐                   ┌────────────────────────┐
│    Claude Code CLI     │                   │     System Browser     │
│   (Terminal Prompt)    │                   │   (N'kai Sidecar UI)   │
└───────────┬────────────┘                   └───────────┬────────────┘
            │                                            │
            │ 1. Runs /vlab or renders sidecar HTML      │
            │ 2. Launches: python tools/nkai_serve.py    │
            ├───────────────────────────────────────────►│ (Browser opens)
            │                                            │
            │                                            │ 3. User reviews visual
            │                                            │    options, clicks card
            │                                            │    (Payload copied)
            │ 4. User pastes into terminal prompt        │
            │◄───────────────────────────────────────────┤
            │                                            │
            │ 5. Agent receives structured verdict & executes
```

1. **Generation**: Claude Code writes or updates a sidecar HTML artifact (e.g., in `.agents/sidecar/`).
2. **Serving**: Claude Code executes `python tools/nkai_serve.py <file>` or outputs the command for the user.
3. **Review**: The user views high-fidelity UI mockups, interactive sliders, and decision matrices in their browser.
4. **Handoff**: Clicking any option card copies the structured verdict or decision syntax to the clipboard and posts to the local loopback.
5. **Execution**: The user presses `Enter` in Claude Code, and the CLI agent proceeds with full visual alignment.

### Pattern 2: Cursor / Windsurf / VS Code Pairing
- Sidecars can be viewed inside editor webview tabs (via `Simple Browser` or preview extensions) or launched in an external browser using `nkai_serve.py`.
- Running under `http://127.0.0.1` ensures that previewing audio DSP wavetables, WAV impulse responses, and JSON session models is not blocked by `file:///` security sandboxes.

### Pattern 3: Aider & Terminal Shell Agents
- Developers running Aider or autonomous bash/powershell agent loops can keep `nkai_serve.py` running in a background terminal.
- Sidecars hot-refresh, allowing continuous visual inspection of DSP curves and UI component hierarchies while terminal agents write code.

---

## 6. Verification & Acceptance Criteria

1. **Protocol Adherence**: `window.NkaiEnv.detect()` accurately identifies `extension` when `window.sidecar` is mocked, `file` when served via `file:///`, and `server` when served over `http://`.
2. **Zero Dependencies**: Neither `nkai_serve.py` nor `bin/nkai.js` import or require any third-party package (`pip` or `npm`).
3. **Port Resilience**: Both CLI servers handle port collisions gracefully by incrementing the port number and reporting the active URL to stdout.
4. **Audio & Asset Fidelity**: `.wav`, `.svg`, and `.json` files are served with RFC-compliant MIME types.
5. **Linting & Safety**: Sidecar templates pass `audit_sidecar.py` (0 tag imbalances, 0 syntax traps) and `verify_context_clues.py` (100% link resolution).
