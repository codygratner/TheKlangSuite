# N'kai Mobile Companion & Touch Ergonomics Specification (v0.3.0 / Milestone Section 14)

## 1. Architectural Intent & Scope

As N'kai evolved from a desktop IDE aux-panel sidecar into an interactive pair-programming companion, daily developer workflow revealed a significant ergonomics requirement: **mobile, multi-device triage from the couch, studio keyboard desk, or listening couch**.

Triaging dozens of ingested inbox rambles, reviewing PCDA trade-offs, and managing backlog priorities should not require being tethered to a desktop mouse and monitor. However, loading desktop webviews on iOS Safari or Android Chrome previously presented friction:
1. **Desktop-Biased Tap Targets**: Small 24px–28px action chips (`Approve`, `Backlog`, `Research`) required pinpoint accuracy on high-DPI touchscreens, triggering mis-taps.
2. **Viewport Jump on Mobile Browsers**: Mobile browser dynamic address bars caused layout jumping on vertical scroll when using classic `100vh`.
3. **Missing Thumb Zone Ergonomics**: Primary triage actions and filter controls were situated at the top of the viewport, out of thumb reach during one-handed phone use.
4. **Friction in LAN Discovery**: Finding the local machine's Wi-Fi IP and typing long file URLs like `http://192.168.1.100:3030/.agents/sidecar/morning_mail_inbox.html` into a mobile browser was slow and error-prone.

**N'kai v0.3.0** resolves these challenges within a strict 1-day standard-library kill-gate, using pure responsive CSS3, vanilla ES6 haptics, and standard Python stdlib networking with terminal and web QR pairing.

```
┌────────────────────────────────────────────────────────────────────────┐
│             N'KAI v0.3.0 MOBILE COMPANION ARCHITECTURE                 │
├────────────────────┬────────────────────┬──────────────────────────────┤
│ SUBSYSTEM          │ FILE LOCATION      │ CORE CAPABILITY              │
├────────────────────┼────────────────────┼──────────────────────────────┤
│ Touch Architecture │ core/sidecar.css   │ 100dvh root fallback,        │
│ & Viewport Media   │ master_template    │ 44px Touch Target Law,       │
│                    │                    │ Momentum rail scroll, 38px   │
├────────────────────┼────────────────────┼──────────────────────────────┤
│ Thumb Navigation   │ master_template    │ Sticky bottom bar with 4 core│
│ Bar (.mobile-thumb)│ core/sidecar.js    │ one-handed thumb targets,    │
│                    │                    │ navigator.vibrate haptics    │
├────────────────────┼────────────────────┼──────────────────────────────┤
│ Zero-Dependency    │ tools/qr_code.py   │ Pure Python ISO QR generator │
│ QR Generator Engine│ (mirrored in nkai) │ Unicode half-blocks & SVG    │
├────────────────────┼────────────────────┼──────────────────────────────┤
│ LAN Server & Web   │ tools/nkai_serve   │ get_local_ip() auto-detect,  │
│ Pairing Endpoints  │ (mirrored in nkai) │ --lan, --qr flags, /qr web   │
└────────────────────┴────────────────────┴──────────────────────────────┘
```

---

## 2. Touch Target Law & Mobile Responsive CSS

### 2.1 Viewport Stability (`100dvh`)
Mobile browsers dynamically collapse and expand their navigation and address bars during scrolling. Standard `100vh` treats the viewport as the static maximum height, causing sudden reflows and content clipping.
- Root elements (`html, body`, `.inbox-container`, `.app-container`) now enforce:
  ```css
  min-height: 100vh;
  min-height: 100dvh;
  ```
- The `<meta name="viewport">` tag in `master_template.html` is upgraded:
  ```html
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, viewport-fit=cover"/>
  ```

### 2.2 The 44px Tap Target Law
Under `@media (max-width: 640px)`, all interactive actionable controls conform to Apple Human Interface Guidelines and Material Design touch targets:
- All card buttons (`.card-btn`, `.action-chip`, `[id^="btn-approve-"]`, `[id^="btn-backlog-"]`, `[id^="btn-research-"]`, `[id^="btn-graveyard-"]`, `[id^="btn-pcda-"]`, `[id^="btn-launch-"]`, `.card-drawer-toggle`, `.mail-body-toggle`):
  ```css
  min-height: 44px;
  min-width: 44px;
  touch-action: manipulation;
  ```
- Action chip container (`.mail-chip-group` and `.mail-session-launchers`) switches from inline-flex wraps into an ergonomic 2-column touch grid (`grid-template-columns: repeat(2, 1fr); gap: 8px;`), ensuring wide, effortless thumb hits.

### 2.3 Filter Rail Momentum Scrolling
Two-Tier filter rails (`.rail-row`, `.rail-chips`) support momentum touch scrolling on mobile:
```css
-webkit-overflow-scrolling: touch;
scrollbar-width: none;
-ms-overflow-style: none;
```
With filter pills (`.pill-btn`, `.filter-chip`, `.header-pill`) locked to a minimum `38px` touch height.

---

## 3. The Sticky Bottom Thumb Bar (`.mobile-thumb-bar`)

### 3.1 Thumb-Zone Layout
On mobile viewports (`max-width: 640px`), `.mobile-thumb-bar` docks fixed to the bottom of the screen with iOS safe-area inset accommodation:
```css
.mobile-thumb-bar {
  position: sticky;
  bottom: 0;
  left: 0;
  right: 0;
  z-index: 1000;
  padding: 8px 12px calc(8px + env(safe-area-inset-bottom, 0px)) 12px;
  background: rgba(9, 13, 19, 0.96);
  backdrop-filter: blur(16px);
  -webkit-backdrop-filter: blur(16px);
  border-top: 1px solid var(--border);
  display: flex;
  justify-content: space-around;
  align-items: center;
  gap: 8px;
}
```
For desktop viewports (`> 640px`), it is forcefully hidden (`display: none !important;`).

### 3.2 Target Actions & Haptic Feedback
The thumb bar provides 4 primary one-handed touch actions:
1. `[ ⚪ Untriaged ]` (`#thumb-btn-untriaged`): Filters Rail 2 directly to untriaged cards (Inbox Zero mode).
2. `[ 🚨 P0 ]` (`#thumb-btn-p0`): Filters Rail 2 directly to active P0 critical items.
3. `[ 📋 All ]` (`#thumb-btn-all`): Resets Rail 2 filter to show the full stream.
4. `[ 🗂 Switch Board ]` (`#thumb-btn-hub`): Opens the `#board-switcher-modal` to jump to other active or archived sessions.

Every tap on a thumb button or card action triggers physical haptic vibration:
```javascript
if (typeof navigator !== 'undefined' && navigator.vibrate) {
  try { navigator.vibrate(15); } catch (e) {}
}
```

---

## 4. Zero-Dependency LAN Server & Terminal QR Code (`tools/nkai_serve.py`)

### 4.1 Automatic Wi-Fi IP Discovery
`get_local_ip()` inspects the local network configuration via a dummy UDP connection to `8.8.8.8` without transmitting network packets, avoiding platform-specific shell calls:
```python
def get_local_ip() -> str:
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
            s.connect(("8.8.8.8", 80))
            return s.getsockname()[0]
    except Exception:
        return "127.0.0.1"
```

### 4.2 Pure Python QR Code Generator (`tools/qr_code.py`)
To prevent heavy external dependencies (`qrcode`, `PIL`), N'kai provides a zero-dependency QR code generator implementing ISO/IEC 18004 subsets:
- Versions 1–10 support (capacities from 17 bytes to 274 bytes, covering all HTTP URLs).
- Byte Mode encoding with Galois Field $GF(256)$ Reed-Solomon Error Correction (Levels L and M).
- Terminal Output: Uses Unicode half-block characters (`█`, `▀`, `▄`, ` `) where each terminal character represents 2 vertical QR modules, rendering a compact, square, instantly scannable code directly in developer terminals.
- SVG Output: Generates clean, resolution-independent vector graphics for web browsers.

### 4.3 Web Pairing Routes (`/qr` and `/mobile`)
In addition to terminal QR printing, `NkaiHTTPRequestHandler` captures `/qr` and `/mobile` requests, serving a dedicated dark-themed pairing page with an embedded high-contrast SVG QR code. Users can keep this open on a second monitor and scan it at any time with an iPhone or Android camera.

CLI Usage:
```bash
# Serve active inbox over local Wi-Fi LAN with terminal QR code:
python tools/nkai_serve.py --lan

# Display QR code for any file or directory:
python tools/nkai_serve.py .agents/sidecar/morning_mail_inbox.html --qr
```

---

## 5. Verification & Compliance Scorecard

1. **Unit & Integration Suite (`test_mobile_companion.py`)**: 7 deterministic tests asserting CSS media queries, 44px tap targets, thumb bar DOM elements, IP detection, QR block rendering, and SVG output.
2. **Universal UTF-8 Compliance (`verify_utf8.py --strict`)**: 100% compliant across all scripts.
3. **Web QA Sidecar Linter (`audit_sidecar.py`)**: 0 errors, 100% DOM tag balance across `master_template.html`, `morning_mail_inbox.html`, and `sandbox_mail_inbox.html`.
4. **Context Clues Link Validator (`verify_context_clues.py`)**: 100% compliant.
5. **Extension Plugin Sync (`install_nkai_plugin.py`)**: Validated and synchronized into `%USERPROFILE%\.gemini\config\plugins\nkai\`.
