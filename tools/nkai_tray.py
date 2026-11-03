#!/usr/bin/env python3
"""
tools/nkai_tray.py — N'kai Windows System Tray Supervisor & 4-Quadrant Quota Telemetry
====================================================================================
Native Windows Notification Area (System Tray) service for N'kai.
- Supervises http://localhost:3030 with Single-Instance Latch Protocol
- Renders dynamic in-memory 4-quadrant taskbar LED icon:
    • Top-Left:     Gemini 5-Hour Allowance
    • Top-Right:    Gemini Weekly Allowance
    • Center:       N'kai Server Health LED (:3030)
    • Bottom-Left:  Claude/3P 5-Hour Allowance (STEALTH when 100%, illuminates when < 100%)
    • Bottom-Right: Claude/3P Weekly Allowance (STEALTH when 100%, illuminates when < 100%)
- Stealth-Claude UX: Completely hides Claude telemetry when at 100%, illuminating
  only when active consumption occurs.
- Thread-decoupled architecture (Win32 PumpMessages UI thread vs Telemetry Watcher thread)
- Intercepts TaskbarCreated for Explorer crash & Modern Standby recovery
- Native right-click menu with live allowance metrics, clipboard routing, and context flushing.
"""

import os
import sys
import time
import io
import ctypes
import threading
import webbrowser
from pathlib import Path
from typing import Dict, Any, Optional

# Enforce Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

# Workspace Path Resolution
CANDIDATE_ROOTS = [
    Path(r"C:\Dev\Nkai"),
    Path(__file__).resolve().parent.parent,
    Path(r"c:\_\Vault"),
    Path(r"C:\Dev\TheKlangSuite"),
]
for root in reversed(CANDIDATE_ROOTS):
    if str(root) not in sys.path:
        sys.path.insert(0, str(root))

REPO_ROOT = Path(r"C:\Dev\Nkai") if Path(r"C:\Dev\Nkai").exists() else Path(__file__).resolve().parent.parent

try:
    import win32api
    import win32con
    import win32gui
    import win32process
    import win32clipboard
    from PIL import Image, ImageDraw
except ImportError as e:
    print(f"Error: Missing required dependency for nkai_tray.py: {e}", file=sys.stderr)
    sys.exit(1)

from tools.nkai_daemon import (
    DEFAULT_PORT,
    STATE_NOMINAL,
    STATE_ATTENTION,
    STATE_CRITICAL,
    evaluate_system_health,
    start_server_process,
    restart_server_process,
    stop_server_process,
)
from tools.flush_context import run_flush

# Window & Message IDs
WM_TRAY_NOTIFY = win32con.WM_USER + 20
WM_STATUS_UPDATE = win32con.WM_USER + 21
WM_FLUSH_COMPLETE = win32con.WM_USER + 22
WM_BALLOON_NOTIFY = win32con.WM_USER + 23

# Menu Command IDs
CMD_OPEN_COCKPIT = 1000
CMD_OPEN_MAIL = 1001
CMD_OPEN_INTERCOM = 1002
CMD_OPEN_RUNWAY = 1003
CMD_OPEN_AGENDA = 1004
CMD_OPEN_FX_OSC = 1005
CMD_STATUS_INFO = 1006
CMD_FLUSH_CONTEXT = 1007
CMD_RESTART_SERVER = 1008
CMD_EXIT = 1009
CMD_ROUTE_CLIPBOARD_LOOP = 1010
CMD_ROUTE_CLIPBOARD_RUNWAY = 1011
CMD_GEMINI_INFO = 1012
CMD_CLAUDE_INFO = 1013
CMD_REFRESH_QUOTA = 1014
CMD_OPEN_TELEMETRY = 1015
CMD_CLAUDE_PRO_STATUS = 1016
CMD_OPEN_CLAUDE_STREAM = 1017

USER32 = ctypes.windll.user32


def ensure_default_desktop():
    """Attaches thread to interactive 'Default' desktop so Shell_NotifyIcon never fails."""
    try:
        h_desk = USER32.OpenDesktopW("Default", 0, False, 0x01FF)
        if h_desk:
            USER32.SetThreadDesktop(h_desk)
    except Exception:
        pass


def debug_log(text: str):
    try:
        log_path = REPO_ROOT / ".agents" / "pipeline" / "nkai_tray_debug.log"
        log_path.parent.mkdir(parents=True, exist_ok=True)
        with open(log_path, "a", encoding="utf-8") as f:
            f.write(f"[{time.strftime('%H:%M:%S')}] {text}\n")
    except Exception:
        pass


def parse_quota(q_data: dict) -> dict:
    """Parses raw language_server ConnectRPC quota summary into fraction dictionary."""
    res = {"g_5h": 1.0, "g_wk": 1.0, "c_5h": 1.0, "c_wk": 1.0}
    if not isinstance(q_data, dict):
        return res
    for group in q_data.get("groups", []):
        name = str(group.get("displayName", "")).lower()
        is_gemini = "gemini" in name
        is_claude = "claude" in name or "gpt" in name
        for b in group.get("buckets", []):
            win = str(b.get("window", "")).lower()
            frac = float(b.get("remainingFraction", 1.0))
            if is_gemini:
                if "5h" in win:
                    res["g_5h"] = frac
                elif "week" in win:
                    res["g_wk"] = frac
            elif is_claude:
                if "5h" in win:
                    res["c_5h"] = frac
                elif "week" in win:
                    res["c_wk"] = frac
    return res


def make_dynamic_quadrant_hicon(
    g5: float, gw: float, c5: float = 1.0, cw: float = 1.0, online: bool = True, state_code: int = 0, size: int = 16, c_resting: bool = False, c_active: bool = False
) -> int:
    """
    Synthesizes the Centaur Dual-Engine 4-Quadrant HUD in memory:
    - Top Row (Gemini):      Top-Left 5h Allowance | Top-Right Weekly Allowance
    - Bottom Row (Claude):    Bottom-Left 5h Allowance | Bottom-Right Weekly Allowance
    - Center Heartbeat Pip:   Neon Blue (#06b6d4) online | Amber (#eab308) degraded
    - Disconnect State:       Full Solid Crimson Red (#ef4444) when offline
    """
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Disconnect State: Full Crimson Red Icon Alarm
    if not online:
        corner_r = max(2, size // 5)
        draw.rounded_rectangle(
            [0, 0, size - 1, size - 1],
            radius=corner_r,
            fill=(239, 68, 68, 255),       # Solid Red #ef4444
            outline=(185, 28, 28, 255),    # Dark Crimson #b91c1c
            width=1,
        )
        # In-memory PNG byte stream conversion
        buf = io.BytesIO()
        img.save(buf, format="PNG")
        png_data = buf.getvalue()
        return USER32.CreateIconFromResourceEx(
            png_data, len(png_data), True, 0x00030000, size, size, 0
        )

    # Baseplate with rounded border
    corner_r = max(2, size // 5)
    draw.rounded_rectangle(
        [0, 0, size - 1, size - 1],
        radius=corner_r,
        fill=(18, 18, 22, 255),
        outline=(44, 44, 52, 255),
        width=1,
    )

    def quota_color(val: float):
        if val > 0.75:
            return (34, 197, 94, 255)   # Emerald Green #22c55e
        elif val > 0.40:
            return (234, 179, 8, 255)   # Amber #eab308
        elif val > 0.15:
            return (249, 115, 22, 255)  # Orange #f97316
        else:
            return (239, 68, 68, 255)   # Crimson Red #ef4444

    pad = max(2, int(size * 0.125))
    gutter = max(2, int(size * 0.125))
    mid = size // 2
    half_g = gutter // 2

    x1_left = pad
    x2_left = mid - half_g - 1
    x1_right = mid + half_g
    x2_right = size - 1 - pad

    y1_top = pad
    y2_top = mid - half_g - 1
    y1_bot = mid + half_g
    y2_bot = size - 1 - pad

    # Top Row: Gemini Engine (Left: 5h, Right: Weekly)
    draw.rectangle([x1_left, y1_top, x2_left, y2_top], fill=quota_color(g5))
    draw.rectangle([x1_right, y1_top, x2_right, y2_top], fill=quota_color(gw))

    # Bottom Row: Claude Pro Engine
    # Bottom-Left: 5h Allowance (Amber when resting, Electric Cyan pulse when active)
    if c_resting:
        draw.rectangle([x1_left, y1_bot, x2_left, y2_bot], fill=(245, 158, 11, 255))  # Amber #f59e0b
    elif c_active:
        draw.rectangle([x1_left, y1_bot, x2_left, y2_bot], fill=(6, 182, 212, 255))   # Electric Cyan #06b6d4
    else:
        draw.rectangle([x1_left, y1_bot, x2_left, y2_bot], fill=quota_color(c5))

    # Bottom-Right: Weekly Allowance
    draw.rectangle([x1_right, y1_bot, x2_right, y2_bot], fill=quota_color(cw))

    # Center Heartbeat Pip (Neon Blue Online, Amber Degraded)
    pip_color = (234, 179, 8, 255) if state_code == 1 else (6, 182, 212, 255)
    pip_r = max(1, size // 10)
    draw.ellipse(
        [mid - pip_r, mid - pip_r, mid + pip_r, mid + pip_r],
        fill=pip_color,
        outline=(18, 18, 22, 255),
        width=1 if size >= 24 else 0
    )

    # In-memory PNG byte stream conversion
    buf = io.BytesIO()
    img.save(buf, format="PNG")
    png_data = buf.getvalue()

    hicon = USER32.CreateIconFromResourceEx(
        png_data, len(png_data), True, 0x00030000, size, size, 0
    )
    return hicon


def get_clipboard_text_win32(hwnd: Optional[int] = None) -> str:
    """Reads text from Windows clipboard via Win32 API with PowerShell fallback."""
    for _ in range(3):
        try:
            win32clipboard.OpenClipboard(hwnd or 0)
            try:
                if win32clipboard.IsClipboardFormatAvailable(win32con.CF_UNICODETEXT):
                    data = win32clipboard.GetClipboardData(win32con.CF_UNICODETEXT)
                    return data.strip() if data else ""
            finally:
                win32clipboard.CloseClipboard()
            break
        except Exception:
            time.sleep(0.04)

    try:
        import subprocess
        startupinfo = subprocess.STARTUPINFO()
        startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
        startupinfo.wShowWindow = 0
        res = subprocess.run(
            ["powershell", "-NoProfile", "-Command", "Get-Clipboard"],
            capture_output=True,
            text=True,
            encoding="utf-8",
            check=False,
            timeout=2.0,
            startupinfo=startupinfo,
            creationflags=0x08000000
        )
        return res.stdout.strip()
    except Exception:
        return ""


class NkaiTrayApp:
    def __init__(self):
        ensure_default_desktop()

        self.icon_size = win32api.GetSystemMetrics(win32con.SM_CXSMICON) or 16
        self.current_hicon = None

        self.allowances = {"g_5h": 1.0, "g_wk": 1.0, "c_5h": 1.0, "c_wk": 1.0}
        self.current_state = STATE_NOMINAL
        self.current_tooltip = "N'kai Initializing..."
        self.latest_health: Dict[str, Any] = {}
        self.running = True
        self.last_lclick_time = 0.0
        self._pending_balloon: Optional[tuple] = None

        # Build initial HICON (Dual-Engine Centaur Quota Grid)
        self.current_hicon = make_dynamic_quadrant_hicon(1.0, 1.0, 1.0, 1.0, True, 0, size=self.icon_size)

        # Register TaskbarCreated message for Explorer crash & sleep recovery
        self.wm_taskbar_created = win32gui.RegisterWindowMessage("TaskbarCreated")

        # Setup hidden window class
        hinst = win32api.GetModuleHandle(None)
        wc = win32gui.WNDCLASS()
        wc.hInstance = hinst
        wc.lpszClassName = "NkaiTrayWindowClass"
        wc.style = win32con.CS_VREDRAW | win32con.CS_HREDRAW | win32con.CS_DBLCLKS
        wc.hCursor = win32api.LoadCursor(0, win32con.IDC_ARROW)
        wc.hbrBackground = win32con.COLOR_WINDOW
        wc.lpfnWndProc = self._wnd_proc

        try:
            self.class_atom = win32gui.RegisterClass(wc)
        except win32gui.error:
            self.class_atom = win32gui.GetClassInfo(hinst, wc.lpszClassName)

        self.hwnd = win32gui.CreateWindow(
            wc.lpszClassName,
            "NkaiTrayWindow",
            win32con.WS_OVERLAPPED | win32con.WS_SYSMENU,
            0, 0, win32con.CW_USEDEFAULT, win32con.CW_USEDEFAULT,
            0, 0, hinst, None
        )
        win32gui.UpdateWindow(self.hwnd)

        # Register initial Tray Icon
        self._add_tray_icon()

        # Start background health & quota telemetry thread
        self.telemetry_thread = threading.Thread(target=self._telemetry_worker, daemon=True)
        self.telemetry_thread.start()

    def _add_tray_icon(self):
        flags = win32gui.NIF_ICON | win32gui.NIF_MESSAGE | win32gui.NIF_TIP
        tip = self.current_tooltip[:127]
        nid = (self.hwnd, 0, flags, WM_TRAY_NOTIFY, self.current_hicon, tip)
        try:
            win32gui.Shell_NotifyIcon(win32gui.NIM_ADD, nid)
        except Exception:
            pass

    def _update_tray_icon(self, balloon_title: Optional[str] = None, balloon_msg: Optional[str] = None):
        tip = self.current_tooltip[:127]

        if balloon_title and balloon_msg:
            flags = win32gui.NIF_ICON | win32gui.NIF_MESSAGE | win32gui.NIF_TIP | win32gui.NIF_INFO
            nid = (
                self.hwnd, 0, flags, WM_TRAY_NOTIFY, self.current_hicon, tip,
                balloon_msg[:255], 3000, balloon_title[:63], win32gui.NIIF_INFO
            )
        else:
            flags = win32gui.NIF_ICON | win32gui.NIF_MESSAGE | win32gui.NIF_TIP
            nid = (self.hwnd, 0, flags, WM_TRAY_NOTIFY, self.current_hicon, tip)

        try:
            win32gui.Shell_NotifyIcon(win32gui.NIM_MODIFY, nid)
        except Exception:
            pass

    def _remove_tray_icon(self):
        try:
            win32gui.Shell_NotifyIcon(win32gui.NIM_DELETE, (self.hwnd, 0))
        except Exception:
            pass

    def _rebuild_icon(self):
        """Generates a new dynamic 4-quadrant HICON and frees the previous handle with zero GDI leak."""
        is_online = self.latest_health.get("is_online", True)
        state_code = int(self.latest_health.get("state_code", 0))

        new_hicon = make_dynamic_quadrant_hicon(
            g5=self.allowances.get("g_5h", 1.0),
            gw=self.allowances.get("g_wk", 1.0),
            c5=self.allowances.get("c_5h", 1.0),
            cw=self.allowances.get("c_wk", 1.0),
            online=is_online,
            state_code=state_code,
            size=self.icon_size,
            c_resting=self.allowances.get("c_resting", False),
            c_active=self.allowances.get("c_active", False),
        )
        old_hicon = self.current_hicon
        self.current_hicon = new_hicon
        if old_hicon and old_hicon != new_hicon:
            try:
                USER32.DestroyIcon(old_hicon)
            except Exception:
                pass

    def _format_tooltip(self, health: dict) -> str:
        """
        Formats a compact multi-line tooltip under 127 chars matching the Centaur layout:
        Top: Gemini (5h / Wk) | Bot: Claude (5h / Wk) | Center: Server (:3030)
        """
        g_5h = self.allowances.get("g_5h", 1.0)
        g_wk = self.allowances.get("g_wk", 1.0)
        c_5h = self.allowances.get("c_5h", 1.0)
        c_wk = self.allowances.get("c_wk", 1.0)
        c_resting = self.allowances.get("c_resting", False)
        c_rest_str = self.allowances.get("c_rest_str", "")
        c_active = self.allowances.get("c_active", False)
        is_online = health.get("is_online", True)
        status_name = "ONLINE" if is_online else "OFFLINE"

        claude_status = "ACTIVE" if c_active else (f"RESTING ({c_rest_str})" if c_resting else f"{c_5h*100:.0f}%")

        lines = [
            f"N'kai Centaur [{status_name} :3030]",
            f"Gemini: 5h {g_5h*100:.0f}% · Wk {g_wk*100:.0f}%",
            f"Claude: 5h {claude_status} · Wk {c_wk*100:.0f}%"
        ]
        return "\n".join(lines)[:127]

    def _telemetry_worker(self):
        """Worker thread executing health checks every 3s and quota probes every 30s."""
        start_server_process(silent=True)
        # Autonomous Bloat Guard: Enforce Copilot & Teams autostart suppression on boot
        try:
            from tools import bloat_guard
            bloat_guard.enforce()
        except Exception:
            pass
        last_recovery_time = 0.0
        last_quota_check = 0.0
        last_pending_count = 0
        last_bloat_check = time.time()

        while self.running:
            try:
                now = time.time()
                health = evaluate_system_health()
                self.latest_health = health

                # Allowance probe (synced with daemon health or periodic ConnectRPC probe)
                quota_changed = False
                new_allowances = health.get("allowances")
                if new_allowances:
                    if new_allowances != self.allowances:
                        self.allowances = new_allowances
                        quota_changed = True
                elif (now - last_quota_check > 30.0) or (last_quota_check == 0.0):
                    last_quota_check = now
                    try:
                        from tools import quota
                        q_data = quota.get_quota_summary()
                        parsed = parse_quota(q_data)
                        if parsed != self.allowances:
                            self.allowances = parsed
                            quota_changed = True
                    except Exception as e:
                        debug_log(f"Quota poll exception: {e}")

                new_state = health["state_code"]
                new_tooltip = self._format_tooltip(health)
                pending_dispatches = health.get("pending_dispatches", 0)
                is_new_prompt = (pending_dispatches > last_pending_count and pending_dispatches > 0)
                last_pending_count = pending_dispatches

                if (
                    quota_changed
                    or new_state != self.current_state
                    or new_tooltip != self.current_tooltip
                    or is_new_prompt
                ):
                    self.current_state = new_state
                    self.current_tooltip = new_tooltip
                    try:
                        win32gui.PostMessage(
                            self.hwnd, WM_STATUS_UPDATE, new_state, 1 if is_new_prompt else 0
                        )
                    except Exception:
                        pass

                # If server went offline, attempt auto-recovery at most once per 15s
                if not health.get("is_online", False) and (now - last_recovery_time > 15.0):
                    last_recovery_time = now
                    start_server_process(silent=True)

                # Periodic Bloat Guard enforcement (every 60 mins) to defeat Windows Update drift
                if now - last_bloat_check > 3600.0:
                    last_bloat_check = now
                    try:
                        from tools import bloat_guard
                        rep = bloat_guard.audit()
                        if not rep.get("clean"):
                            bloat_guard.enforce()
                    except Exception:
                        pass

            except Exception as e:
                debug_log(f"Telemetry worker exception: {e}")

            time.sleep(3.0)

    def _wnd_proc(self, hwnd, msg, wparam, lparam):
        if msg == self.wm_taskbar_created:
            # Explorer crashed or restarted — restore tray icon
            self._rebuild_icon()
            self._add_tray_icon()
            return 0

        elif msg == WM_STATUS_UPDATE:
            self._rebuild_icon()
            if lparam == 1:
                title = "📱 New Mobile Prompt / Dispatch"
                text = f"N'kai received a new remote dispatch on Data Bus ({self.current_tooltip})."
                self._update_tray_icon(balloon_title=title, balloon_msg=text)
            else:
                self._update_tray_icon()
            return 0

        elif msg == WM_FLUSH_COMPLETE:
            mb = wparam / 100.0
            files = lparam
            title = "N'kai Context Flusher"
            text = f"Purged {files} stale logs/scratch items. Freed {mb:.2f} MB."
            self._update_tray_icon(balloon_title=title, balloon_msg=text)
            return 0

        elif msg == WM_BALLOON_NOTIFY:
            if self._pending_balloon:
                title, text = self._pending_balloon
                self._pending_balloon = None
                self._update_tray_icon(balloon_title=title, balloon_msg=text)
            return 0

        elif msg == WM_TRAY_NOTIFY:
            if lparam == win32con.WM_LBUTTONDBLCLK:
                self.last_lclick_time = 0.0
                self._open_url(f"http://127.0.0.1:{DEFAULT_PORT}/morning_mail_inbox.html")
                return 1

            elif lparam == win32con.WM_LBUTTONUP:
                now = time.time()
                if (now - self.last_lclick_time) < 0.45:
                    self.last_lclick_time = 0.0
                    self._open_url(f"http://127.0.0.1:{DEFAULT_PORT}/morning_mail_inbox.html")
                else:
                    self.last_lclick_time = now
                return 1

            elif lparam == win32con.WM_RBUTTONUP:
                self._show_context_menu()
                return 1
            return 1

        elif msg == win32con.WM_COMMAND:
            cmd_id = win32api.LOWORD(wparam)
            self._handle_command(cmd_id)
            return 0

        elif msg == win32con.WM_DESTROY:
            self.running = False
            self._remove_tray_icon()
            if self.current_hicon:
                try:
                    USER32.DestroyIcon(self.current_hicon)
                except Exception:
                    pass
            win32gui.PostQuitMessage(0)
            return 0

        return win32gui.DefWindowProc(hwnd, msg, wparam, lparam)

    @staticmethod
    def _open_url(url: str):
        try:
            USER32.AllowSetForegroundWindow(-1)
            def _find_browser_cb(bhwnd, res):
                if win32gui.IsWindowVisible(bhwnd):
                    t = win32gui.GetWindowText(bhwnd).lower()
                    c = win32gui.GetClassName(bhwnd).lower()
                    if ("chrome" in t or "chrome" in c or "edge" in t) and "antigravity" not in t:
                        res.append(bhwnd)
            b_hwnds = []
            win32gui.EnumWindows(_find_browser_cb, b_hwnds)
            for bhwnd in b_hwnds:
                if win32gui.IsIconic(bhwnd):
                    win32gui.ShowWindow(bhwnd, win32con.SW_RESTORE)
                win32gui.SetForegroundWindow(bhwnd)
                break
        except Exception:
            pass

        try:
            chrome_path = r"C:\Program Files\Google\Chrome\Application\chrome.exe"
            if not os.path.exists(chrome_path):
                chrome_path = r"C:\Program Files (x86)\Google\Chrome\Application\chrome.exe"

            if os.path.exists(chrome_path):
                cmd = f'"{chrome_path}" {url}'
                si = win32process.STARTUPINFO()
                si.lpDesktop = r"WinSta0\Default"
                hp, ht, pid, tid = win32process.CreateProcess(
                    None, cmd, None, None, False, win32con.NORMAL_PRIORITY_CLASS, None, None, si
                )
                return
        except Exception:
            pass

        try:
            res = win32api.ShellExecute(0, "open", url, None, None, win32con.SW_SHOWNORMAL)
            if res > 32:
                return
        except Exception:
            pass

        try:
            os.startfile(url)
            return
        except Exception:
            pass

        try:
            webbrowser.open(url)
        except Exception:
            pass

    def _show_context_menu(self):
        try:
            menu = win32gui.CreatePopupMenu()

            # Master Cockpit (Zen Single-Canvas SPA)
            win32gui.AppendMenu(menu, win32con.MF_STRING, CMD_OPEN_COCKPIT, "🌌 Open Sovereign Cockpit (Zen SPA)")
            win32gui.AppendMenu(menu, win32con.MF_SEPARATOR, 0, "")

            # 5 Core Pillars in Strict Priority Order
            win32gui.AppendMenu(menu, win32con.MF_STRING, CMD_OPEN_TELEMETRY, "📊 1. Telemetry && Quota")
            win32gui.AppendMenu(menu, win32con.MF_STRING, CMD_OPEN_INTERCOM, "💬 2. Sentry Chat Intercom")
            win32gui.AppendMenu(menu, win32con.MF_STRING, CMD_OPEN_FX_OSC, "🎛️ 3. Synth News Radar")
            win32gui.AppendMenu(menu, win32con.MF_STRING, CMD_OPEN_AGENDA, "🧭 4. Agenda && Flight Plan")
            win32gui.AppendMenu(menu, win32con.MF_STRING, CMD_OPEN_MAIL, "📬 5. Morning Mailbag")
            win32gui.AppendMenu(menu, win32con.MF_SEPARATOR, 0, "")

            # Quick Agenda Routing
            win32gui.AppendMenu(menu, win32con.MF_STRING, CMD_ROUTE_CLIPBOARD_LOOP, "📌 Route Clipboard ➔ Open Loop")
            win32gui.AppendMenu(menu, win32con.MF_SEPARATOR, 0, "")

            # Live telemetry status indicator (Disabled/Grayed info rows)
            is_online = self.latest_health.get("is_online", True)
            server_dot = "🔷" if is_online else "🔴"
            active_mb = self.latest_health.get("active_transcript_mb", 0.0)
            ctx_dot = "🟢" if active_mb < 1.5 else ("🟡" if active_mb < 2.5 else ("🟠" if active_mb < 3.5 else "🔴"))
            g_5h = self.allowances.get("g_5h", 1.0)
            g_wk = self.allowances.get("g_wk", 1.0)
            c_5h = self.allowances.get("c_5h", 1.0)
            c_wk = self.allowances.get("c_wk", 1.0)
            c_resting = self.allowances.get("c_resting", False)
            c_rest_str = self.allowances.get("c_rest_str", "")
            c_active = self.allowances.get("c_active", False)

            g5_dot = "🟢" if g_5h > 0.75 else ("🟡" if g_5h > 0.40 else "🔴")
            c_dot = "🔵" if c_active else ("🟠" if c_resting else ("🟢" if c_5h > 0.75 else ("🟡" if c_5h > 0.40 else "🔴")))
            c_status_str = f"RESTING ({c_rest_str})" if c_resting else ("ACTIVE" if c_active else f"{c_5h*100:.0f}%")

            win32gui.AppendMenu(menu, win32con.MF_STRING | win32con.MF_GRAYED, CMD_STATUS_INFO, f"{server_dot} Server: {'ONLINE' if is_online else 'OFFLINE'} (Port {DEFAULT_PORT})")
            win32gui.AppendMenu(menu, win32con.MF_STRING | win32con.MF_GRAYED, CMD_OPEN_TELEMETRY, f"{ctx_dot} Context Size: {active_mb:.2f} MB / 3.5 MB")
            win32gui.AppendMenu(menu, win32con.MF_STRING | win32con.MF_GRAYED, CMD_GEMINI_INFO, f"{g5_dot} Gemini: 5h {g_5h*100:.0f}% · Wk {g_wk*100:.0f}%")
            win32gui.AppendMenu(menu, win32con.MF_STRING | win32con.MF_GRAYED, CMD_CLAUDE_INFO, f"{c_dot} Claude: 5h {c_status_str} · Wk {c_wk*100:.0f}%")

            win32gui.AppendMenu(menu, win32con.MF_STRING, CMD_OPEN_CLAUDE_STREAM, "👁️ View Claude Stream (latest_live.md)")
            win32gui.AppendMenu(menu, win32con.MF_STRING, CMD_REFRESH_QUOTA, "🔄 Refresh Allowance Now")
            win32gui.AppendMenu(menu, win32con.MF_SEPARATOR, 0, "")

            # Context Flusher & Restart
            win32gui.AppendMenu(menu, win32con.MF_STRING, CMD_FLUSH_CONTEXT, "🧹 Flush Context && Clean Caches")
            win32gui.AppendMenu(menu, win32con.MF_STRING, CMD_RESTART_SERVER, "🔄 Restart Server")
            win32gui.AppendMenu(menu, win32con.MF_SEPARATOR, 0, "")

            # Exit
            win32gui.AppendMenu(menu, win32con.MF_STRING, CMD_EXIT, "🚪 Exit")

            # Position menu at cursor
            pos = win32gui.GetCursorPos()
            win32gui.SetForegroundWindow(self.hwnd)
            win32gui.TrackPopupMenu(
                menu,
                win32con.TPM_LEFTALIGN,
                pos[0], pos[1], 0, self.hwnd, None
            )
            win32gui.PostMessage(self.hwnd, win32con.WM_NULL, 0, 0)
        except Exception as e:
            debug_log(f"Error in _show_context_menu: {e}")

    def _handle_command(self, cmd: int):
        if cmd == CMD_OPEN_COCKPIT:
            self._open_url(f"http://127.0.0.1:{DEFAULT_PORT}/cockpit.html")
        elif cmd == CMD_OPEN_TELEMETRY:
            self._open_url(f"http://127.0.0.1:{DEFAULT_PORT}/cockpit.html#telemetry")
        elif cmd == CMD_OPEN_INTERCOM:
            self._open_url(f"http://127.0.0.1:{DEFAULT_PORT}/cockpit.html#chat")
        elif cmd == CMD_OPEN_FX_OSC:
            self._open_url(f"http://127.0.0.1:{DEFAULT_PORT}/cockpit.html#synth_news")
        elif cmd == CMD_OPEN_AGENDA:
            self._open_url(f"http://127.0.0.1:{DEFAULT_PORT}/cockpit.html#agenda")
        elif cmd == CMD_OPEN_MAIL:
            self._open_url(f"http://127.0.0.1:{DEFAULT_PORT}/cockpit.html#mail")
        elif cmd == CMD_OPEN_RUNWAY:
            self._open_url(f"http://127.0.0.1:{DEFAULT_PORT}/cockpit.html#agenda")
        elif cmd == CMD_OPEN_CLAUDE_STREAM:
            try:
                live_file = Path(r"c:\_\Vault\.agents\scratch\latest_live.md")
                if not live_file.exists():
                    live_file.parent.mkdir(parents=True, exist_ok=True)
                    live_file.write_text("# Claude Live Stream\nNo active Claude stream currently running.\n", encoding="utf-8")
                os.startfile(str(live_file))
            except Exception as e:
                debug_log(f"Failed to open live stream: {e}")
        elif cmd == CMD_ROUTE_CLIPBOARD_LOOP:
            threading.Thread(target=self._run_async_route_clipboard, args=("open_loops",), daemon=True).start()
        elif cmd == CMD_ROUTE_CLIPBOARD_RUNWAY:
            threading.Thread(target=self._run_async_route_clipboard, args=("runway",), daemon=True).start()
        elif cmd == CMD_REFRESH_QUOTA:
            threading.Thread(target=self._run_async_refresh_quota, daemon=True).start()
        elif cmd == CMD_FLUSH_CONTEXT:
            threading.Thread(target=self._run_async_flush, daemon=True).start()
        elif cmd == CMD_RESTART_SERVER:
            threading.Thread(target=self._run_async_restart, daemon=True).start()
        elif cmd == CMD_EXIT:
            self.running = False
            self._remove_tray_icon()
            win32gui.DestroyWindow(self.hwnd)

    def _run_async_refresh_quota(self):
        try:
            from tools.nkai_daemon import get_model_allowances
            self.allowances = get_model_allowances(force_refresh=True)
            health = self.latest_health or evaluate_system_health(assume_online=True)
            self.latest_health = health
            self.current_tooltip = self._format_tooltip(health)
            self._rebuild_icon()
            win32gui.PostMessage(self.hwnd, WM_STATUS_UPDATE, self.current_state, 0)
            g_5h = self.allowances.get("g_5h", 1.0)
            g_wk = self.allowances.get("g_wk", 1.0)
            c_5h = self.allowances.get("c_5h", 1.0)
            c_wk = self.allowances.get("c_wk", 1.0)
            c_resting = self.allowances.get("c_resting", False)
            c_rest_str = self.allowances.get("c_rest_str", "")
            c_status = f"RESTING ({c_rest_str})" if c_resting else f"{c_5h*100:.0f}%"
            self._pending_balloon = (
                "📊 Allowances Refreshed",
                f"Gemini: 5h {g_5h*100:.0f}% · Wk {g_wk*100:.0f}%\nClaude: 5h {c_status} · Wk {c_wk*100:.0f}%",
            )
            win32gui.PostMessage(self.hwnd, WM_BALLOON_NOTIFY, 0, 0)
        except Exception as e:
            debug_log(f"Refresh quota exception: {e}")

    def _run_async_route_clipboard(self, target_section: str):
        try:
            clip = get_clipboard_text_win32(self.hwnd)
            if not clip:
                self._pending_balloon = ("📋 Clipboard Empty", "No text found on clipboard to route to Agenda.")
                win32gui.PostMessage(self.hwnd, WM_BALLOON_NOTIFY, 0, 0)
                return

            lines = [l.strip() for l in clip.splitlines() if l.strip()]
            title = lines[0] if lines else "Untitled Note"
            summary = " ".join(lines[1:]) if len(lines) > 1 else "Captured via N'kai Systray quick route."
            clean_title = title[:80] + ("..." if len(title) > 80 else "")

            from tools.agenda_engine import route_card_to_agenda
            res = route_card_to_agenda(
                title=clean_title,
                summary=summary,
                target_section=target_section,
                priority="P1",
                track="📱 Nkai" if target_section == "runway" else "",
            )

            if res.get("success"):
                dest_name = "Active Open Loops" if target_section == "open_loops" else "Today's Runway"
                self._pending_balloon = (f"📌 Routed to {dest_name}", clean_title)
            else:
                self._pending_balloon = ("❌ Agenda Routing Failed", res.get("error", "Failed to update AGENDA.md"))
            win32gui.PostMessage(self.hwnd, WM_BALLOON_NOTIFY, 0, 0)
        except Exception as e:
            debug_log(f"Error routing clipboard: {e}")
            self._pending_balloon = ("❌ Error", f"Could not route clipboard: {e}")
            win32gui.PostMessage(self.hwnd, WM_BALLOON_NOTIFY, 0, 0)

    def _run_async_flush(self):
        try:
            res = run_flush()
            mb_freed = res.get("mb_freed", 0.0)
            files = res.get("files_removed", 0)
            wparam = int(round(mb_freed * 100))
            win32gui.PostMessage(self.hwnd, WM_FLUSH_COMPLETE, wparam, files)
        except Exception:
            pass

    def _run_async_restart(self):
        restart_server_process(silent=True)
        health = evaluate_system_health()
        self.latest_health = health
        self.current_state = health["state_code"]
        self.current_tooltip = self._format_tooltip(health)
        win32gui.PostMessage(self.hwnd, WM_STATUS_UPDATE, self.current_state, 0)

    def run(self):
        """Starts the Win32 message pump on UI thread."""
        try:
            win32gui.PumpMessages()
        finally:
            self.cleanup()

    def cleanup(self):
        self.running = False
        self._remove_tray_icon()
        if self.current_hicon:
            try:
                USER32.DestroyIcon(self.current_hicon)
                self.current_hicon = None
            except Exception:
                pass


SINGLE_INSTANCE_MUTEX_NAME = "Local\\NkaiTraySingleInstanceMutex"


def acquire_single_instance_latch(force_replace: bool = False):
    """Guarantees strictly 1 instance of the tray app runs using a zero-overhead Win32 named mutex."""
    ensure_default_desktop()
    kernel32 = ctypes.windll.kernel32
    mutex = kernel32.CreateMutexW(None, False, SINGLE_INSTANCE_MUTEX_NAME)
    err = kernel32.GetLastError()
    if err == 183:  # ERROR_ALREADY_EXISTS
        if force_replace:
            existing_hwnd = win32gui.FindWindow("NkaiTrayWindowClass", "NkaiTrayWindow")
            if existing_hwnd:
                win32gui.PostMessage(existing_hwnd, win32con.WM_DESTROY, 0, 0)
                time.sleep(0.3)
                kernel32.CloseHandle(mutex)
                mutex = kernel32.CreateMutexW(None, False, SINGLE_INSTANCE_MUTEX_NAME)
                return mutex
            else:
                # If window was not found by title, attempt killing via process scan
                try:
                    import psutil
                    for p in psutil.process_iter(['pid', 'cmdline']):
                        cmd = p.info.get('cmdline') or []
                        if any('nkai_tray' in arg for arg in cmd) and p.pid != os.getpid():
                            p.terminate()
                            p.wait(timeout=1.0)
                    time.sleep(0.2)
                    kernel32.CloseHandle(mutex)
                    mutex = kernel32.CreateMutexW(None, False, SINGLE_INSTANCE_MUTEX_NAME)
                    return mutex
                except Exception:
                    pass
        kernel32.CloseHandle(mutex)
        return None
    return mutex


def main():
    force_replace = ("--replace" in sys.argv or "--restart" in sys.argv)
    mutex = acquire_single_instance_latch(force_replace=force_replace)
    if not mutex:
        sys.exit(0)

    try:
        app = NkaiTrayApp()
        app.run()
    except Exception as e:
        import traceback
        log_file = REPO_ROOT / ".agents" / "pipeline" / "nkai_tray_error.log"
        log_file.parent.mkdir(parents=True, exist_ok=True)
        with open(log_file, "a", encoding="utf-8") as f:
            f.write(f"\n[{time.strftime('%Y-%m-%d %H:%M:%S')}] Exception: {e}\n")
            traceback.print_exc(file=f)
    finally:
        ctypes.windll.kernel32.CloseHandle(mutex)


if __name__ == "__main__":
    main()
