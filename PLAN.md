# Flight 24: The Stateless Sentry (AGY SDK Satellite Relay & Mobile Companion Gateway)

**TARGET:** TOOLING_HARNESS  
**Role:** New Klang City Architecture & Web Engineering  
**Recommended Model:** Gemini 3.8 Flash (Thinking: High) — Tier 2 (Balanced Engineering & UI)  
> **Status:** DRAFTED — READY FOR REVIEW  

---

### Executive Overview & Architectural Dialectic
Following our 2-Round Heavy Online PCDA on autonomous background daemons, we are implementing the **Stateless Sentry Pattern**:
1. **The Stateless Sentry vs. Runaway Autonomous Daemons**: Rather than running an unconstrained 24/7 background agent with write tools (which creates severe "Lethal Trifecta" security risks, split-brain git collisions, and financial denial-of-service loops), the Satellite operates as an **on-demand, single-turn AGY SDK worker** inside `tools/nkai_serve.py`.
2. **Pre-Shared Bearer Token Airlock**: Guarding `tools/nkai_serve.py` on port 3030 with a cryptographically secure bearer token (`Authorization: Bearer <token>`). Mobile QR codes automatically encode `?auth=<token>` so scanning or bookmarking on iOS establishes zero-friction authenticated access stored in `localStorage`.
3. **Hard Budget & Sandboxed Tool Boundaries**:
   - `max_turns = 2` (hard circuit breaker preventing recursive error loops).
   - `max_output_tokens = 4000` (strict quota defense).
   - Scoped tools: Read-only vault markdown search, Google Drive read-only queries, and append-only note staging (`TheKlangVault/Inbox/QuickNotes/`).
   - **NEGATIVE INVARIANT**: Zero shell/terminal execution (`run_command` is strictly banned); zero direct in-place note overwrites.
4. **Live SSE Mobile Streaming**: Streams internal thoughts, text tokens, and tool execution status via Server-Sent Events (`/api/sentry/stream`) directly into the mobile walkie-talkie UI (`chat_board.html`).

---

### Phase 1: Security Airlock & Pre-Shared Bearer Auth (`tools/sentry_auth.py` & `tools/nkai_serve.py`)
- [x] Create `tools/sentry_auth.py`:
  - `get_or_create_sentry_token(auth_file: str = ".agents/pipeline/sentry_auth.json") -> str`: Reads existing hex token or generates a cryptographically secure 32-byte token (`secrets.token_hex(32)`).
  - `verify_sentry_token(auth_header: Optional[str], expected_token: str) -> bool`: Constant-time string comparison (`hmac.compare_digest`) against `Bearer <token>`.
- [x] Update `tools/nkai_serve.py`:
  - Add FastAPI security dependency validating `Authorization: Bearer <token>` on all `/api/` mutation and agent routes (`/api/chat`, `/api/chat/upload`, `/api/action`, `/api/sentry/stream`).
  - Allow unauthenticated static asset serving (`/`, `/static/`, `/favicon.ico`) while protecting API endpoints.
  - Update `/qr` and `/mobile` endpoints to inject `?auth=<token>` into the generated QR codes and mobile landing redirects.
  - Return `401 Unauthorized` with JSON error payload when authorization fails.
- [x] Author unit test suite `tools/tests/test_sentry_auth.py` (token generation, constant-time verification, missing header rejection).

---

### Phase 2: AGY SDK Sentry Engine (`tools/sentry_engine.py`)
- [x] Install `google-antigravity` into the active Python environment (`python -m pip install google-antigravity`).
- [x] Create `tools/sentry_engine.py`:
  - Class `SentryWorker`:
    - Wraps `google.antigravity.Agent` with `LocalAgentConfig(max_turns=2, budget_limits=BudgetLimits(max_output_tokens=4000))`.
    - Declares scoped Python tools:
      1. `vault_search(query: str) -> str`: Read-only ripgrep/search across `TheKlangVault/` (markdown files only, max 5 results).
      2. `vault_read(rel_path: str) -> str`: Read-only note fetcher chrooted strictly to `TheKlangVault/`.
      3. `gdrive_search(query: str) -> str`: Read-only Drive metadata via `gdrive readonly ls`.
      4. `stage_quick_note(title: str, body: str) -> str`: Appends note strictly to `TheKlangVault/Inbox/QuickNotes/<YYYY-MM-DD>_<safe_title>.md`.
      5. `stage_dispatch(title: str, payload_json: str) -> str`: Staged file drop into `C:\Dev\TheKlangVault\Dispatches\pending/<id>.json`.
    - Invariant Assertion: Asserts that no tools provide shell execution, code evaluation (`exec`/`eval`), or arbitrary path writes.
- [x] Add unit test assertions in `tools/tests/test_sentry_engine.py` verifying tool sandboxing and parameter validation.

---

### Phase 3: Real-Time SSE Streaming Endpoint (`tools/nkai_serve.py`)
- [x] Add `POST /api/sentry/stream` in `tools/nkai_serve.py`:
  - Accepts JSON payload: `{"prompt": str, "attachments": list[str]}`.
  - Streams responses using Starlette/FastAPI `StreamingResponse(..., media_type="text/event-stream")`.
  - Event schema:
    - `event: thought\ndata: {"content": str}\n\n`
    - `event: tool_start\ndata: {"tool": str, "input": dict}\n\n`
    - `event: token\ndata: {"content": str}\n\n`
    - `event: done\ndata: {"ok": true, "stats": {"turns": int, "tokens": int}}\n\n`
    - `event: error\ndata: {"error": str}\n\n`
  - Handles client disconnects and cancels agent streaming gracefully.

---

### Phase 4: Mobile Walkie-Talkie & Chat Board Integration (`tools/chat_engine.py` & `chat_board.html`)
- [x] Update `c:\Dev\TheKlangSuite\tools\chat_engine.py`:
  - Inject auth handling into `generate_chat_board_html()`:
    - On load: extracts `?auth=` from URL and caches in `localStorage.setItem('nkai_sentry_auth', token)`.
    - Includes `Authorization: Bearer <token>` in `fetch()` headers.
    - If unauthorized, displays clean amber lock prompt allowing manual token entry or re-scanning QR.
  - Update `sendPrompt()` to stream from `/api/sentry/stream` via `fetch` ReadableStream:
    - Renders streaming thought accordion: `<details class="thought-disclosure"><summary>🧠 Thinking...</summary><div class="thought-body">...</div></details>`.
    - Live Markdown text streaming into the active chat bubble.
    - Displays dynamic tool chips (`[ 🔍 Searching Vault: "compressor" ]`).
- [x] Rebuild and mirror `chat_board.html` across `TheKlangSuite` and `nkai` repos.

---

### Phase 5: Verification Gauntlet, Audits & Web QA
- [x] Execute `python -m unittest discover tools/tests/` (100% pass rate).
- [x] Run `tools/audit_sidecar.py` across all generated HTML boards (zero HTML/JS/CSS errors).
- [x] Run `tools/verify_utf8.py --strict` across all modified scripts (100% compliant).
- [x] Verify live endpoints and mobile stream on `http://100.87.196.60:3030/chat`.
- [x] Commit all changes cleanly across `TheKlangSuite` and `nkai`.
