#!/usr/bin/env python3
"""
tools/nkai_tray.py — N'kai Windows System Tray Daemon & Supervisor
==================================================================
Native Windows Notification Area (System Tray) service for N'kai.
- Supervises http://localhost:3030 with Single-Instance Latch Protocol
- Renders dynamic in-memory 32-bit RGBA LED icons (🟢 Green, 🟡 Yellow, 🔴 Red)
- Thread-decoupled architecture (Win32 PumpMessages UI thread vs Telemetry Watcher thread)
- Intercepts TaskbarCreated for Explorer crash & Modern Standby recovery
- Native right-click menu and double-click Morning Mail launcher
- 1-click Context & Cache Flusher with balloon toast notification
"""

import os
import sys
import time
import struct
import zlib
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

REPO_ROOT = Path(__file__).resolve().parent.parent
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

import win32api
import win32con
import win32gui

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

# Menu Command IDs
CMD_OPEN_COCKPIT = 1000
CMD_OPEN_MAIL = 1001
CMD_OPEN_INTERCOM = 1002
CMD_OPEN_RUNWAY = 1003
CMD_OPEN_FX_OSC = 1004
CMD_STATUS_INFO = 1005
CMD_FLUSH_CONTEXT = 1006
CMD_RESTART_SERVER = 1007
CMD_EXIT = 1008

USER32 = ctypes.windll.user32


def ensure_default_desktop():
    """Attaches thread to interactive 'Default' desktop so Shell_NotifyIcon never fails."""
    try:
        h_desk = USER32.OpenDesktopW("Default", 0, False, 0x01FF)
        if h_desk:
            USER32.SetThreadDesktop(h_desk)
    except Exception:
        pass


def make_png_led_icon(r: int, g: int, b: int, size: int = 16):
    """Synthesizes a crisp, anti-aliased 32-bit RGBA circular LED icon in memory."""
    w, h = size, size
    center = (size - 1) / 2.0
    radius = (size / 2.0) - 1.5
    r_sq = radius * radius
    inner_sq = (radius - 1.0) * (radius - 1.0)
    outer_edge_sq = (radius + 1.2) * (radius + 1.2)

    raw = bytearray()
    for y in range(h):
        raw.append(0)  # PNG row filter type: None
        for x in range(w):
            dx = x - center
            dy = y - center
            dist_sq = dx * dx + dy * dy
            if dist_sq <= inner_sq:
                # Specular dot (upper left)
                if dist_sq <= (radius * 0.3) ** 2 and dx < 0 and dy < 0:
                    raw.extend([255, 255, 255, 255])
                else:
                    raw.extend([r, g, b, 255])
            elif dist_sq <= r_sq:
                # Border ring
                raw.extend([max(0, r - 55), max(0, g - 55), max(0, b - 55), 255])
            elif dist_sq <= outer_edge_sq:
                # Smooth anti-aliased outer edge
                frac = (outer_edge_sq - dist_sq) / (outer_edge_sq - r_sq)
                alpha = int(255 * max(0.0, min(1.0, frac)))
                raw.extend([r, g, b, alpha])
            else:
                # Fully transparent background
                raw.extend([0, 0, 0, 0])

    def chunk(tag, data):
        return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)

    png_bytes = (
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 6, 0, 0, 0))
        + chunk(b"IDAT", zlib.compress(bytes(raw), 9))
        + chunk(b"IEND", b"")
    )
    return USER32.CreateIconFromResourceEx(png_bytes, len(png_bytes), True, 0x00030000, size, size, 0)


def debug_log(text: str):
    try:
        with open(r"c:\Dev\TheKlangSuite\tray_debug.log", "a", encoding="utf-8") as f:
            f.write(f"[{time.strftime('%H:%M:%S')}] {text}\n")
    except Exception:
        pass


class NkaiTrayApp:
    def __init__(self):
        ensure_default_desktop()

        self.icon_size = win32api.GetSystemMetrics(win32con.SM_CXSMICON) or 16
        self.icons = {
            STATE_NOMINAL: make_png_led_icon(34, 197, 94, self.icon_size),    # 🟢 #22c55e
            STATE_ATTENTION: make_png_led_icon(234, 179, 8, self.icon_size),  # 🟡 #eab308
            STATE_CRITICAL: make_png_led_icon(239, 68, 68, self.icon_size),   # 🔴 #ef4444
        }

        self.current_state = STATE_NOMINAL
        self.current_tooltip = "N'kai Daemon: Initializing..."
        self.latest_health: Dict[str, Any] = {}
        self.running = True
        self.last_lclick_time = 0.0

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

        # Start background health telemetry thread
        self.telemetry_thread = threading.Thread(target=self._telemetry_worker, daemon=True)
        self.telemetry_thread.start()

    def _add_tray_icon(self):
        flags = win32gui.NIF_ICON | win32gui.NIF_MESSAGE | win32gui.NIF_TIP
        hicon = self.icons.get(self.current_state, self.icons[STATE_NOMINAL])
        tip = self.current_tooltip[:127]
        nid = (self.hwnd, 0, flags, WM_TRAY_NOTIFY, hicon, tip)
        try:
            win32gui.Shell_NotifyIcon(win32gui.NIM_ADD, nid)
        except Exception:
            pass

    def _update_tray_icon(self, balloon_title: Optional[str] = None, balloon_msg: Optional[str] = None):
        hicon = self.icons.get(self.current_state, self.icons[STATE_NOMINAL])
        tip = self.current_tooltip[:127]

        if balloon_title and balloon_msg:
            flags = win32gui.NIF_ICON | win32gui.NIF_MESSAGE | win32gui.NIF_TIP | win32gui.NIF_INFO
            nid = (
                self.hwnd, 0, flags, WM_TRAY_NOTIFY, hicon, tip,
                balloon_msg[:255], 3000, balloon_title[:63], win32gui.NIIF_INFO
            )
        else:
            flags = win32gui.NIF_ICON | win32gui.NIF_MESSAGE | win32gui.NIF_TIP
            nid = (self.hwnd, 0, flags, WM_TRAY_NOTIFY, hicon, tip)

        try:
            win32gui.Shell_NotifyIcon(win32gui.NIM_MODIFY, nid)
        except Exception:
            pass

    def _remove_tray_icon(self):
        try:
            win32gui.Shell_NotifyIcon(win32gui.NIM_DELETE, (self.hwnd, 0))
        except Exception:
            pass

    def _telemetry_worker(self):
        """Worker thread executing in-process health and dispatch probes every 3s."""
        # Ensure server is alive at boot
        start_server_process(silent=True)
        last_recovery_time = 0.0
        last_pending_count = 0

        while self.running:
            try:
                health = evaluate_system_health()
                self.latest_health = health

                new_state = health["state_code"]
                new_tooltip = health["tooltip"]
                pending_dispatches = health.get("pending_dispatches", 0)
                is_new_prompt = (pending_dispatches > last_pending_count and pending_dispatches > 0)
                last_pending_count = pending_dispatches

                if new_state != self.current_state or new_tooltip != self.current_tooltip or is_new_prompt:
                    self.current_state = new_state
                    self.current_tooltip = new_tooltip
                    # Safely dispatch non-blocking message to UI thread
                    try:
                        win32gui.PostMessage(self.hwnd, WM_STATUS_UPDATE, new_state, 1 if is_new_prompt else 0)
                    except Exception:
                        pass

                # If server went offline, attempt auto-recovery at most once per 15s
                now = time.time()
                if not health.get("is_online", False) and (now - last_recovery_time > 15.0):
                    last_recovery_time = now
                    start_server_process(silent=True)

            except Exception:
                pass

            time.sleep(3.0)

    def _wnd_proc(self, hwnd, msg, wparam, lparam):
        if msg == self.wm_taskbar_created:
            # Explorer crashed or restarted — restore tray icon
            self._add_tray_icon()
            return 0

        elif msg == WM_STATUS_UPDATE:
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

        elif msg == WM_TRAY_NOTIFY:
            debug_log(f"WM_TRAY_NOTIFY: lparam={lparam} (RBUTTONUP={win32con.WM_RBUTTONUP}, DBLCLK={win32con.WM_LBUTTONDBLCLK})")
            if lparam == win32con.WM_LBUTTONDBLCLK:
                # Left double-click (native Win32 message)
                debug_log("DOUBLE CLICK (WM_LBUTTONDBLCLK) DETECTED -> Opening Mail")
                self.last_lclick_time = 0.0
                self._open_url(f"http://127.0.0.1:{DEFAULT_PORT}/morning_mail_inbox.html")
                return 1

            elif lparam == win32con.WM_LBUTTONUP:
                now = time.time()
                # Software double-click debounce fallback (within 450ms)
                if (now - self.last_lclick_time) < 0.45:
                    debug_log("DOUBLE CLICK (SOFTWARE LBUTTONUP) DETECTED -> Opening Mail")
                    self.last_lclick_time = 0.0
                    self._open_url(f"http://127.0.0.1:{DEFAULT_PORT}/morning_mail_inbox.html")
                else:
                    self.last_lclick_time = now
                return 1

            elif lparam == win32con.WM_RBUTTONUP:
                # Right-click: Show Context Menu
                debug_log("RIGHT CLICK DETECTED -> Showing Menu")
                self._show_context_menu()
                return 1
            return 1

        elif msg == win32con.WM_COMMAND:
            cmd_id = win32api.LOWORD(wparam)
            debug_log(f"WM_COMMAND DETECTED: cmd_id={cmd_id}")
            self._handle_command(cmd_id)
            return 0

        elif msg == win32con.WM_DESTROY:
            debug_log("WM_DESTROY DETECTED -> Exiting")
            self.running = False
            self._remove_tray_icon()
            win32gui.PostQuitMessage(0)
            return 0

        return win32gui.DefWindowProc(hwnd, msg, wparam, lparam)

    @staticmethod
    def _open_url(url: str):
        debug_log(f"_open_url called with: {url}")

        # 1. Native Windows ShellExecute on URL (delegates to default browser in user session)
        try:
            debug_log("Attempting win32api.ShellExecute on URL")
            res = win32api.ShellExecute(0, "open", url, None, None, win32con.SW_SHOWNORMAL)
            debug_log(f"win32api.ShellExecute returned: {res}")
            if res > 32:
                return
        except Exception as e:
            debug_log(f"win32api.ShellExecute error: {e}")

        # 2. Native os.startfile
        try:
            debug_log("Attempting os.startfile")
            os.startfile(url)
            debug_log("os.startfile succeeded")
            return
        except Exception as e:
            debug_log(f"os.startfile error: {e}")

        # 3. Direct ShellExecute targeting chrome.exe explicitly
        try:
            debug_log("Attempting ShellExecute targeting chrome.exe")
            res = win32api.ShellExecute(0, "open", "chrome.exe", url, None, win32con.SW_SHOWNORMAL)
            debug_log(f"ShellExecute chrome.exe returned: {res}")
            if res > 32:
                return
        except Exception as e:
            debug_log(f"ShellExecute chrome.exe error: {e}")

        # 4. Native Windows cmd start
        try:
            debug_log("Attempting cmd.exe /c start")
            import subprocess
            subprocess.Popen(["cmd.exe", "/c", "start", "", url], shell=False)
            debug_log("cmd.exe /c start spawned")
            return
        except Exception as e:
            debug_log(f"cmd.exe /c start error: {e}")

        # 5. Standard webbrowser fallback
        try:
            debug_log("Attempting webbrowser.open")
            webbrowser.open(url)
        except Exception as e:
            debug_log(f"webbrowser.open error: {e}")

    def _show_context_menu(self):
        menu = win32gui.CreatePopupMenu()

        # Master Cockpit (Side-by-Side)
        win32gui.AppendMenu(menu, win32con.MF_STRING, CMD_OPEN_COCKPIT, "🛸 Open N'kai Cockpit (Split View)")
        win32gui.AppendMenu(menu, win32con.MF_SEPARATOR, 0, "")

        # Board routes
        win32gui.AppendMenu(menu, win32con.MF_STRING, CMD_OPEN_MAIL, "📬 Open Morning Mail")
        win32gui.AppendMenu(menu, win32con.MF_STRING, CMD_OPEN_INTERCOM, "💬 Open N'kai Intercom")
        win32gui.AppendMenu(menu, win32con.MF_STRING, CMD_OPEN_RUNWAY, "🛫 Open Runway Board")
        win32gui.AppendMenu(menu, win32con.MF_STRING, CMD_OPEN_FX_OSC, "🔬 Open Lab: FX && OSC")
        win32gui.AppendMenu(menu, win32con.MF_SEPARATOR, 0, "")

        # Live telemetry status indicator (Disabled/Grayed info row)
        state_icon = "🟢" if self.current_state == STATE_NOMINAL else ("🟡" if self.current_state == STATE_ATTENTION else "🔴")
        status_label = f"{state_icon} Status: {self.latest_health.get('state_name', 'ONLINE')} (Port {DEFAULT_PORT})"
        win32gui.AppendMenu(menu, win32con.MF_STRING | win32con.MF_GRAYED, CMD_STATUS_INFO, status_label)

        # Context Flusher & Restart
        win32gui.AppendMenu(menu, win32con.MF_STRING, CMD_FLUSH_CONTEXT, "🧹 Flush Context && Clean Caches")
        win32gui.AppendMenu(menu, win32con.MF_STRING, CMD_RESTART_SERVER, "🔄 Restart Server")
        win32gui.AppendMenu(menu, win32con.MF_SEPARATOR, 0, "")

        # Exit
        win32gui.AppendMenu(menu, win32con.MF_STRING, CMD_EXIT, "🚪 Exit")

        # Position menu at cursor
        pos = win32gui.GetCursorPos()
        win32gui.SetForegroundWindow(self.hwnd)
        debug_log(f"Tracking popup menu at {pos} for hwnd {self.hwnd}...")
        win32gui.TrackPopupMenu(
            menu,
            win32con.TPM_LEFTALIGN,
            pos[0], pos[1], 0, self.hwnd, None
        )
        win32gui.PostMessage(self.hwnd, win32con.WM_NULL, 0, 0)
        debug_log("TrackPopupMenu completed")

    def _handle_command(self, cmd: int):
        debug_log(f"_handle_command dispatched: {cmd}")
        if cmd == CMD_OPEN_COCKPIT:
            self._open_url(f"http://127.0.0.1:{DEFAULT_PORT}/cockpit.html")
        elif cmd == CMD_OPEN_MAIL:
            self._open_url(f"http://127.0.0.1:{DEFAULT_PORT}/morning_mail_inbox.html")
        elif cmd == CMD_OPEN_INTERCOM:
            self._open_url(f"http://127.0.0.1:{DEFAULT_PORT}/chat_board.html")
        elif cmd == CMD_OPEN_RUNWAY:
            self._open_url(f"http://127.0.0.1:{DEFAULT_PORT}/runway_board.html")
        elif cmd == CMD_OPEN_FX_OSC:
            self._open_url(f"http://127.0.0.1:{DEFAULT_PORT}/fx_and_osc_board.html")
        elif cmd == CMD_FLUSH_CONTEXT:
            threading.Thread(target=self._run_async_flush, daemon=True).start()
        elif cmd == CMD_RESTART_SERVER:
            threading.Thread(target=self._run_async_restart, daemon=True).start()
        elif cmd == CMD_EXIT:
            self.running = False
            self._remove_tray_icon()
            win32gui.DestroyWindow(self.hwnd)

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
        self.current_tooltip = health["tooltip"]
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
        for hicon in self.icons.values():
            try:
                USER32.DestroyIcon(hicon)
            except Exception:
                pass


SINGLE_INSTANCE_MUTEX_NAME = "Local\\NkaiTraySingleInstanceMutex"


def acquire_single_instance_latch(force_replace: bool = False):
    """Guarantees strictly 1 instance of the tray app runs using a zero-overhead Win32 named mutex."""
    kernel32 = ctypes.windll.kernel32
    mutex = kernel32.CreateMutexW(None, False, SINGLE_INSTANCE_MUTEX_NAME)
    err = kernel32.GetLastError()
    if err == 183:  # ERROR_ALREADY_EXISTS
        if force_replace:
            # Find and retire existing instance window
            existing_hwnd = win32gui.FindWindow("NkaiTrayWindowClass", "NkaiTrayWindow")
            if existing_hwnd:
                win32gui.PostMessage(existing_hwnd, win32con.WM_DESTROY, 0, 0)
                time.sleep(0.3)
                kernel32.CloseHandle(mutex)
                mutex = kernel32.CreateMutexW(None, False, SINGLE_INSTANCE_MUTEX_NAME)
                return mutex
        kernel32.CloseHandle(mutex)
        return None
    return mutex


def main():
    force_replace = ("--replace" in sys.argv or "--restart" in sys.argv)
    mutex = acquire_single_instance_latch(force_replace=force_replace)
    if not mutex:
        # Another instance is already alive and running. Exit silently with 0 overhead.
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
