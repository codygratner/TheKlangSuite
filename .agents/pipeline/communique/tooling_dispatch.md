STATUS: COMPLETED

# Subagent Dispatch Charter: N'kai v0.3.0 Mobile Companion & Touch Ergonomics
> *Target Pipeline:* Dual-Track Communiqué — Tooling Pipeline (`TARGET: TOOLING_HARNESS`)
> *Active Branch:* `0.4.0-dev` (TKS) / `main` (nkai)
> *Orchestrator:* Antigravity Main Chat (New Klang City)
> *Executor:* Flash Builder Subagent
> *Enforced Kill-Gate:* Strict 1-day hard cap (vanilla responsive CSS, pure Python stdlib, zero heavy PWA frameworks)

---

## 1. Assignment Header
- **STATUS**: `COMPLETED`
- **ROLE**: `Flash Tooling Builder`
- **MODEL TIER**: `Tier 2 Flash (High)`
- **TASK TYPE**: `TOOLING_IMPLEMENTATION`
- **TASK SLUG**: `nkai-v030-mobile-companion`
- **TARGET REPOSITORIES**: `TheKlangSuite`, `nkai` (`c:\Dev\nkai`)
- **TICKET ID**: Section 14 (N'kai v0.3.0 Milestone)

---

## 2. Scope & Technical Specifications

Execute all phases of `c:\Dev\TheKlangSuite\PLAN.md`:

### Phase 1: Touch-First Mobile CSS & Viewport Architecture
1. In `c:\Dev\nkai\core\sidecar.css` (and mirrored templates):
   - Update viewport root: support `min-height: 100vh; min-height: 100dvh;` on body and scroll containers.
   - Enforce Touch Target Law: minimum 44px x 44px tap targets on all interactive buttons (`.card-btn`, `#btn-approve-*`, `#btn-backlog-*`, `#btn-research-*`, `#btn-graveyard-*`, `#btn-pcda-*`, `.card-drawer-toggle`, `.session-launcher-btn`) for `@media (max-width: 640px)`.
   - Card layout on mobile: full width with 12px–14px padding, card actions wrapping into a clean, touch-friendly 2-column or flex grid.
   - Two-Tier filter rails on mobile: smooth horizontal momentum scrolling (`-webkit-overflow-scrolling: touch; scrollbar-width: none;`) with pill touch targets minimum 38px height.
   - Style `.mobile-thumb-bar`: sticky bottom navigation bar (`position: sticky; bottom: 0; z-index: 1000; padding: 8px 12px calc(8px + env(safe-area-inset-bottom, 0px)) 12px; background: rgba(9, 13, 19, 0.96); backdrop-filter: blur(16px); -webkit-backdrop-filter: blur(16px); border-top: 1px solid var(--border); display: flex; justify-content: space-around; align-items: center;`).
   - For viewports `> 640px`, `.mobile-thumb-bar` must be `display: none !important;`.

### Phase 2: Zero-Dependency LAN Server & Terminal QR Code (`tools/nkai_serve.py`)
1. In `tools/nkai_serve.py` (and mirrored to `c:\Dev\nkai\tools\nkai_serve.py` and `c:\Dev\nkai\bundle\tools\nkai_serve.py`):
   - Add local Wi-Fi IP auto-detection (`get_local_ip()` using dummy UDP connection to `8.8.8.8`).
   - Implement zero-dependency terminal ASCII / Unicode QR Code generator (`tools/qr_code.py` or inline in `nkai_serve.py`) that prints scan-friendly block characters (`█`, `▀`, `▄`, ` `) directly in the terminal when `--lan` or `--qr` or binding to `0.0.0.0` is used!
   - Default or `--lan` banner:
     ```
     📱 N'KAI MOBILE COMPANION READY!
     🌐 Local Wi-Fi URL: http://<LOCAL_IP>:<PORT>/morning_mail_inbox.html
     ```
     Followed by the scannable QR code!
   - Add web route `/qr` and `/mobile` in `NkaiHTTPRequestHandler` that renders an SVG QR code in browser for second-monitor scanning.
   - Support `--host 0.0.0.0` as standard for LAN serving.

### Phase 3: Master Template & Frontend Controller Integration
1. In `c:\Dev\nkai\core\master_template.html` (and mirrored templates):
   - Update `<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, viewport-fit=cover"/>`.
   - Add markup for `.mobile-thumb-bar`:
     - Button `#thumb-btn-untriaged`: `⚪ Untriaged` (toggles Rail 2 filter to untriaged)
     - Button `#thumb-btn-p0`: `🚨 P0` (toggles Rail 2 filter to P0)
     - Button `#thumb-btn-all`: `📋 All` (resets filter to All)
     - Button `#thumb-btn-hub`: `🗂 Switch Board` (triggers `#board-switcher-modal`)
2. In `c:\Dev\nkai\core\sidecar.js`:
   - Bind thumb button click handlers.
   - Add haptic vibration: `if (navigator.vibrate) navigator.vibrate(15);` on card actions and thumb button taps.
3. Update `tools/board_renderer.py` and mirror templates across `c:\Dev\nkai\core\`, `c:\Dev\nkai\bundle\templates\`, and `c:\Dev\TheKlangSuite\.agents\sidecar\`.

### Phase 4: Automated Verification Gauntlet
1. Author `tools/tests/test_mobile_companion.py`:
   - Test responsive CSS media queries and 44px tap target rules in `sidecar.css`.
   - Test `.mobile-thumb-bar` presence and markup in `master_template.html`.
   - Test `get_local_ip()` and QR code generator logic in `nkai_serve.py`.
2. Run unit tests: `python -m unittest discover tools/tests/`.
3. Regenerate boards: `python tools/check_mail.py --no-browser`.
4. Run `python tools/audit_sidecar.py` across updated HTML files (assert 0 errors).
5. Run `python tools/verify_utf8.py --strict`.
6. Run `python tools/verify_context_clues.py`.
7. Run `python tools/install_nkai_plugin.py`.

### Phase 5: Design Spec, Documentation & Commits
1. Author design spec `docs/specs/nkai_mobile_companion_and_touch_ergonomics.md`.
2. Register in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.
3. Update `docs/GLOSSARY.md` (§25) and `docs/CHEAT_SHEET.md`.
4. In `docs/BACKLOG.md`: mark Section 14 as `[x] COMPLETED`.
5. Check all boxes `[x]` in `PLAN.md` and set `Status: COMPLETED ✅`.
6. Append structured receipt to `.agents/pipeline/communique/subagent_receipt.md` and set dispatch `STATUS: COMPLETED`.
7. Commit cleanly across `TheKlangSuite` and `nkai`.
