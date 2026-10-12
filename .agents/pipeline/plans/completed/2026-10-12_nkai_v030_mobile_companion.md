# N'kai v0.3.0: Mobile Companion & Touch Ergonomics Update

> **TARGET:** TOOLING_HARNESS · Milestone v0.3.0 · Backlog Section 14
> **Active Model Setting:** Gemini 3.8 Flash (Thinking: High) — 🟢 SUSTAINABLE
> **Enforced Kill-Gate:** Strict 1-day hard cap (vanilla responsive CSS, pure Python stdlib, zero heavy PWA frameworks)
> **Status:** COMPLETED ✅

## Goal
Transform N'kai into a multi-device interactive AI pair-programming companion, enabling frictionless couch and studio triage from mobile devices (iOS Safari & Android Chrome) over local LAN Wi-Fi with native touch ergonomics, minimum 44px tap targets, a sticky bottom thumb bar, and zero-config terminal QR pairing.

---

## 📋 Phase Breakdown

### Phase 1: Touch-First Mobile CSS & Viewport Architecture
- [x] Update `<meta name="viewport">` with `viewport-fit=cover` and dynamic viewport scaling.
- [x] Add `100dvh` dynamic viewport height fallback to eliminate mobile browser URL bar layout jumps.
- [x] Implement responsive media queries (`max-width: 640px` and `max-width: 480px`) in `core/sidecar.css`.
- [x] Enforce the 44px Tap Target Law on all interactive controls (`.card-btn`, filter pills, drawer toggles).
- [x] Style smooth horizontal touch-scrolling with snap points for Two-Tier filter rails.
- [x] Implement sticky bottom thumb navigation bar (`.mobile-thumb-bar`) for one-handed card filtering.

### Phase 2: Zero-Dependency LAN Server & Terminal QR Code (`tools/nkai_serve.py`)
- [x] Add local Wi-Fi IP address auto-detection (`get_local_ip()`).
- [x] Author standard-library ASCII/Unicode QR Code generator (`tools/qr_code.py` or inline).
- [x] Update `tools/nkai_serve.py` with `--lan` and `--qr` flags, displaying clickable local URL and terminal QR code.
- [x] Add `/mobile` and `/qr` web endpoints in `nkai_serve.py` serving graphical SVG QR codes for second-monitor pairing.

### Phase 3: Master Template & Frontend Controller Integration
- [x] In `core/master_template.html`, integrate `.mobile-thumb-bar` markup with 4 core thumb targets:
  - `[ ⚪ Untriaged ]`
  - `[ 🚨 P0 Active ]`
  - `[ 📋 All Cards ]`
  - `[ 🔍 Search / Board Hub ]`
- [x] In `core/sidecar.js`, bind thumb navigation actions and add `navigator.vibrate` haptic support on mobile touch.
- [x] Update `tools/board_renderer.py` to support mobile template tags.
- [x] Synchronize templates and assets across `c:\Dev\nkai` and `c:\Dev\TheKlangSuite`.

### Phase 4: Automated Verification Gauntlet
- [x] Author `tools/tests/test_mobile_companion.py` testing responsive CSS rules, tap targets, and LAN/QR generation.
- [x] Run full test suite: `python -m unittest discover tools/tests/`.
- [x] Regenerate `morning_mail_inbox.html` and sandbox board via `python tools/check_mail.py --no-browser`.
- [x] Run `python tools/audit_sidecar.py` across all HTML templates (assert 0 errors, 100% tag balance).
- [x] Run `python tools/verify_utf8.py --strict` (assert 100% compliance).
- [x] Run `python tools/verify_context_clues.py` (assert 100% pass).
- [x] Run `python tools/install_nkai_plugin.py` to synchronize `%USERPROFILE%\.gemini\config\plugins\nkai\`.

### Phase 5: Design Spec, Documentation & Multi-Repo Commits
- [x] Author design specification `docs/specs/nkai_mobile_companion_and_touch_ergonomics.md`.
- [x] Register spec in `docs/DOCS_CATALOG.json` and `docs/SYSTEM_MAP.md`.
- [x] Update `docs/GLOSSARY.md` (§25: Mobile Companion & Touch Ergonomics) and `docs/CHEAT_SHEET.md`.
- [x] Mark Section 14 as `[x] COMPLETED` in `docs/BACKLOG.md`.
- [x] Update `context_clues_plan.md` and `context_clues_build.md`.
- [x] Commit cleanly across `TheKlangSuite` and `nkai`.
