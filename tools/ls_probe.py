#!/usr/bin/env python3
"""
tools/ls_probe.py — Antigravity Language Server Introspection & ConnectRPC Probe
================================================================================
Universal CLI tool to discover, inspect, probe, and track version drift of the
internal Antigravity ConnectRPC Language Server daemon (language_server.exe).

Features:
  1. Auto-Discovery   : Extracts ephemeral HTTPS port and CSRF token from active process.
  2. Binary Fingerprint: Computes SHA-256, build stamp, and version of language_server.exe.
  3. RPC Method Audit : Scans and categorizes all 352+ ConnectRPC endpoints.
  4. Live Method Probe: Invokes any RPC method (e.g. GetAvailableModels, GetAllSkills).
  5. Version Drift    : Detects newly added, removed, or changed endpoints across IDE updates.
"""

import os
import sys
import re
import ssl
import json
import time
import hashlib
import urllib.request
import subprocess
from pathlib import Path
from typing import Dict, Any, List, Optional, Tuple

# Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

VAULT_ROOT = Path(__file__).resolve().parent.parent
CACHE_PATH = VAULT_ROOT / ".agents" / "cache" / "antigravity_rpc_cache.json"
MANIFEST_PATH = VAULT_ROOT / ".agents" / "index" / "language_server_manifest.json"

DEFAULT_BIN_PATH = Path(r"C:\Users\codyg\AppData\Local\Programs\antigravity\resources\bin\language_server.exe")


def get_binary_info(bin_path: Optional[Path] = None) -> Dict[str, Any]:
    """Inspects the language_server.exe binary for SHA-256 hash, size, and modification time."""
    p = bin_path or DEFAULT_BIN_PATH
    if not p.exists():
        return {"exists": False, "path": str(p)}

    h = hashlib.sha256()
    with open(p, "rb") as f:
        while chunk := f.read(65536):
            h.update(chunk)

    stat = p.stat()
    return {
        "exists": True,
        "path": str(p),
        "sha256": h.hexdigest(),
        "size_bytes": stat.st_size,
        "size_mb": round(stat.st_size / (1024 * 1024), 2),
        "mtime_iso": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime(stat.st_mtime)),
    }


def discover_session(force_refresh: bool = False) -> Tuple[int, str]:
    """Discovers active HTTPS port and CSRF token from running process and logs."""
    if not force_refresh and CACHE_PATH.exists():
        try:
            cached = json.loads(CACHE_PATH.read_text(encoding="utf-8"))
            # Quick health probe to verify cached port is still alive
            if _test_connection(cached["port"], cached["csrf_token"]):
                return cached["port"], cached["csrf_token"]
        except Exception:
            pass

    # 1. Discover CSRF token from command line
    script = "Get-CimInstance Win32_Process -Filter \"name = 'language_server.exe' and CommandLine like '%--standalone%'\" | Select-Object ProcessId, CommandLine | ConvertTo-Json"
    creationflags = 0x08000000 if sys.platform == "win32" else 0
    res = subprocess.run(["powershell", "-NoProfile", "-Command", script], capture_output=True, text=True, encoding="utf-8", creationflags=creationflags)
    m_token = re.search(r"--csrf_token[=\s]+([a-f0-9\-]+)", res.stdout)
    if not m_token:
        # Fallback search across any language_server process
        script_all = "Get-CimInstance Win32_Process -Filter \"name = 'language_server.exe'\" | Select-Object CommandLine | ConvertTo-Json"
        res_all = subprocess.run(["powershell", "-NoProfile", "-Command", script_all], capture_output=True, text=True, encoding="utf-8", creationflags=creationflags)
        m_token = re.search(r"--csrf_token[=\s]+([a-f0-9\-]+)", res_all.stdout)

    if not m_token:
        raise RuntimeError("Could not find --csrf_token in active language_server.exe process")
    csrf_token = m_token.group(1)

    # 2. Discover active HTTPS port from log file
    log_path = Path.home() / "AppData" / "Roaming" / "Antigravity" / "logs" / "language_server.log"
    port = None
    if log_path.exists():
        text = log_path.read_text(encoding="utf-8", errors="ignore")
        matches = re.findall(r"Language server listening on random port at (\d+) for HTTPS", text)
        if matches:
            port = int(matches[-1])

    if not port:
        raise RuntimeError("Could not discover language_server HTTPS port from logs")

    # Cache credentials
    CACHE_PATH.parent.mkdir(parents=True, exist_ok=True)
    CACHE_PATH.write_text(json.dumps({"port": port, "csrf_token": csrf_token}), encoding="utf-8")
    return port, csrf_token


def _test_connection(port: int, csrf_token: str) -> bool:
    """Fast check whether the discovered HTTPS port is active."""
    try:
        call_rpc("WellSupportedLanguages", port=port, csrf_token=csrf_token, timeout=0.8)
        return True
    except Exception:
        return False


def call_rpc(
    method_name: str,
    payload: Optional[Dict[str, Any]] = None,
    port: Optional[int] = None,
    csrf_token: Optional[str] = None,
    timeout: float = 3.0,
) -> Dict[str, Any]:
    """Invokes a ConnectRPC method on the local language server daemon."""
    if not port or not csrf_token:
        port, csrf_token = discover_session()

    url = f"https://127.0.0.1:{port}/exa.language_server_pb.LanguageServerService/{method_name}"
    ctx = ssl.create_default_context()
    ctx.check_hostname = False
    ctx.verify_mode = ssl.CERT_NONE

    headers = {
        "x-codeium-csrf-token": csrf_token,
        "Content-Type": "application/json",
        "Connect-Protocol-Version": "1",
    }

    data_bytes = json.dumps(payload or {}).encode("utf-8")
    req = urllib.request.Request(url, headers=headers, data=data_bytes)

    with urllib.request.urlopen(req, timeout=timeout, context=ctx) as resp:
        body = resp.read().decode("utf-8")
        return json.loads(body) if body else {}


def extract_all_methods(bin_path: Optional[Path] = None) -> List[str]:
    """Scans binary strings to extract all declared ConnectRPC methods."""
    p = bin_path or DEFAULT_BIN_PATH
    with open(p, "rb") as f:
        data = f.read()
    raw = re.findall(rb"exa\.language_server_pb\.LanguageServerService/([A-Za-z0-9_]+)", data)
    return sorted(set(m.decode("ascii") for m in raw))


def categorize_methods(methods: List[str]) -> Dict[str, List[str]]:
    """Groups RPC methods into functional architectural domains."""
    cat_rules = [
        ("Trajectory & Conversation", ["Trajectory", "Conversation", "Cascade", "Step", "Turn", "NarrationTurn", "AgentMessage"]),
        ("Model & Quota Telemetry", ["Quota", "Model", "Token"]),
        ("MCP & Plugin Ecosystem", ["Mcp", "Plugin"]),
        ("Skills, Rules & Workflows", ["Skill", "Rule", "Workflow", "Customization", "SlashCommand", "Trigger"]),
        ("Terminal & Execution Sandbox", ["Terminal", "Shell", "Command"]),
        ("Version Control & Worktrees", ["Commit", "Worktree", "VersionControl", "Changelist", "Changelog", "Diff", "Revert", "Patch", "Fig", "Cog", "Gerrit", "JJ", "Git", "Repo"]),
        ("Browser & DevTools Automation", ["Browser", "DevTools", "Page", "ScreenRecording", "Screenshot"]),
        ("Audio & Multimodal Narration", ["Audio", "Transcription", "Narration"]),
        ("Memory & Knowledge", ["Memory", "Memories", "Knowledge", "ContextScope", "ContextModule"]),
        ("Workspace & File I/O", ["Workspace", "Project", "File", "Directory", "Uri", "Watch", "ReadDir"]),
        ("Authentication & Admin", ["Auth", "User", "License", "TermsOfService", "Admin", "Enterprise", "Profile", "Analytics"]),
        ("Experiments & Feature Flags", ["BattleMode", "Experiment", "Mendel", "Unleash"]),
        ("Internal & Diagnostics", ["Debug", "FlightRecorder", "Pprof", "Log", "Stat", "Status", "Ping", "Simulate", "WarmUp", "Exit", "Heartbeat"]),
    ]

    grouped: Dict[str, List[str]] = {cat[0]: [] for cat in cat_rules}
    grouped["Other / Utility"] = []

    for m in methods:
        matched = False
        for cat_name, keywords in cat_rules:
            if any(k in m for k in keywords):
                grouped[cat_name].append(m)
                matched = True
                break
        if not matched:
            grouped["Other / Utility"].append(m)

    return grouped


def save_manifest(bin_info: Dict[str, Any], methods: List[str]) -> Path:
    """Saves a permanent versioned manifest of language server methods for drift detection."""
    MANIFEST_PATH.parent.mkdir(parents=True, exist_ok=True)
    categorized = categorize_methods(methods)
    manifest = {
        "binary_sha256": bin_info["sha256"],
        "binary_size_bytes": bin_info["size_bytes"],
        "binary_mtime": bin_info["mtime_iso"],
        "total_methods": len(methods),
        "updated_at": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "categories": categorized,
        "methods": methods,
    }
    MANIFEST_PATH.write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    return MANIFEST_PATH


def audit_version_drift() -> Dict[str, Any]:
    """Compares active binary against saved manifest to detect added, removed, or changed methods."""
    bin_info = get_binary_info()
    current_methods = set(extract_all_methods())

    if not MANIFEST_PATH.exists():
        save_manifest(bin_info, sorted(current_methods))
        return {
            "first_run": True,
            "binary_sha256": bin_info["sha256"],
            "total_methods": len(current_methods),
            "added": [],
            "removed": [],
        }

    cached = json.loads(MANIFEST_PATH.read_text(encoding="utf-8"))
    cached_methods = set(cached.get("methods", []))
    old_hash = cached.get("binary_sha256", "")

    added = sorted(list(current_methods - cached_methods))
    removed = sorted(list(cached_methods - current_methods))
    hash_changed = (bin_info["sha256"] != old_hash)

    if added or removed or hash_changed:
        save_manifest(bin_info, sorted(current_methods))

    return {
        "first_run": False,
        "hash_changed": hash_changed,
        "old_sha256": old_hash[:12],
        "new_sha256": bin_info["sha256"][:12],
        "total_methods": len(current_methods),
        "added_count": len(added),
        "removed_count": len(removed),
        "added": added,
        "removed": removed,
    }


def run_command_headless(cmd: str, cwd: Optional[str] = None, timeout_sec: float = 15.0) -> Dict[str, Any]:
    """
    Executes a shell command with ZERO terminal windows popping up.
    Attempts primary execution via ConnectRPC RunCommand on language_server.exe.
    Falls back cleanly to subprocess with CREATE_NO_WINDOW (0x08000000).
    """
    target_cwd = cwd or str(VAULT_ROOT)
    
    # 1. Primary Route: ConnectRPC RunCommand
    try:
        payload = {
            "command": cmd,
            "cwd": target_cwd,
            "timeout_ms": int(timeout_sec * 1000)
        }
        res = call_rpc("RunCommand", payload, timeout=timeout_sec)
        return {
            "channel": "CONNECTRPC_HEADLESS",
            "stdout": res.get("stdout", ""),
            "stderr": res.get("stderr", ""),
            "exit_code": res.get("exitCode", 0)
        }
    except Exception:
        pass

    # 2. Resilient Fallback: Subprocess with CREATE_NO_WINDOW
    creationflags = 0x08000000  # CREATE_NO_WINDOW
    startupinfo = subprocess.STARTUPINFO()
    startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
    startupinfo.wShowWindow = 0  # SW_HIDE
    
    try:
        proc = subprocess.run(
            cmd,
            shell=True,
            cwd=target_cwd,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=timeout_sec,
            creationflags=creationflags,
            startupinfo=startupinfo
        )
        return {
            "channel": "SUBPROCESS_SHIELDED",
            "stdout": proc.stdout,
            "stderr": proc.stderr,
            "exit_code": proc.returncode
        }
    except Exception as sub_err:
        return {
            "channel": "ERROR",
            "stdout": "",
            "stderr": str(sub_err),
            "exit_code": -1
        }


def main():
    import argparse
    parser = argparse.ArgumentParser(description="Antigravity Language Server Introspection & ConnectRPC Probe CLI")
    subparsers = parser.add_subparsers(dest="command", help="Command to execute")

    # status
    subparsers.add_parser("status", help="Shows daemon connection, port, token, and binary fingerprint")

    # methods
    m_p = subparsers.add_parser("methods", help="Lists all 352+ ConnectRPC methods categorized by domain")
    m_p.add_argument("--category", "-c", help="Filter by specific category name")
    m_p.add_argument("--json", action="store_true", help="Output raw JSON array")

    # audit
    subparsers.add_parser("audit", help="Checks for version drift, added/removed RPCs across IDE updates")

    # call
    call_p = subparsers.add_parser("call", help="Invokes an RPC method on the live language server")
    call_p.add_argument("method", help="Method name (e.g. GetAvailableModels, GetAllSkills)")
    call_p.add_argument("--data", "-d", default="{}", help="JSON payload string")

    # export-markdown
    exp_p = subparsers.add_parser("export-markdown", help="Exports a markdown catalog of all methods")
    exp_p.add_argument("--out", "-o", help="Target markdown file path")

    # run (headless zero-window execution)
    run_p = subparsers.add_parser("run", help="Executes a shell command headlessly via ConnectRPC RunCommand (0 popup windows)")
    run_p.add_argument("cmd", help="Command line string to execute")
    run_p.add_argument("--cwd", help="Working directory (default: Vault root)")
    run_p.add_argument("--timeout", type=float, default=15.0, help="Timeout in seconds")

    args = parser.parse_args()

    if args.command == "status":
        bin_info = get_binary_info()
        print("🛰️ Antigravity Language Server Telemetry:")
        print(f"   Binary Path    : {bin_info['path']}")
        print(f"   Binary Hash    : {bin_info.get('sha256', 'N/A')[:16]}... ({bin_info.get('size_mb', 0)} MB)")
        try:
            port, token = discover_session()
            print(f"   Daemon Status  : 🟢 CONNECTED (Port {port})")
            print(f"   CSRF Token     : {token[:8]}...{token[-4:]}")
        except Exception as e:
            print(f"   Daemon Status  : 🔴 DISCONNECTED ({e})")

    elif args.command == "methods":
        all_m = extract_all_methods()
        categorized = categorize_methods(all_m)
        if args.json:
            print(json.dumps(categorized, indent=2))
            return

        print(f"📋 LanguageServerService ConnectRPC Methods ({len(all_m)} total):")
        for cat, items in categorized.items():
            if args.category and args.category.lower() not in cat.lower():
                continue
            print(f"\n### {cat} ({len(items)}):")
            for item in items:
                print(f"  • {item}")

    elif args.command == "audit":
        res = audit_version_drift()
        if res.get("first_run"):
            print(f"✅ Initialized RPC manifest with {res['total_methods']} methods (SHA-256: {res['binary_sha256'][:12]}).")
        elif not res["hash_changed"] and not res["added"] and not res["removed"]:
            print(f"✅ Binary unchanged (SHA-256: {res['new_sha256']}) — All {res['total_methods']} methods verified.")
        else:
            print(f"⚠️ Binary Update Detected! Hash: {res['old_sha256']} ➔ {res['new_sha256']}")
            print(f"   Total Methods  : {res['total_methods']}")
            if res["added"]:
                print(f"   ✨ Added ({len(res['added'])}):")
                for m in res["added"]:
                    print(f"      + {m}")
            if res["removed"]:
                print(f"   ❌ Removed ({len(res['removed'])}):")
                for m in res["removed"]:
                    print(f"      - {m}")

    elif args.command == "call":
        try:
            payload = json.loads(args.data)
            res = call_rpc(args.method, payload=payload)
            print(json.dumps(res, indent=2))
        except Exception as e:
            print(f"❌ Error invoking {args.method}: {e}", file=sys.stderr)
            sys.exit(1)

    elif args.command == "export-markdown":
        all_m = extract_all_methods()
        categorized = categorize_methods(all_m)
        bin_info = get_binary_info()
        out_path = Path(args.out) if args.out else VAULT_ROOT / "Agent Rules" / "language_server_canon.md"

        lines = [
            "# 🛰️ Antigravity Language Server Canon: The 352 ConnectRPC Endpoints",
            "",
            f"> **Binary Path**: `{bin_info['path']}`  ",
            f"> **Binary SHA-256**: `{bin_info['sha256']}`  ",
            f"> **Binary Size**: `{bin_info['size_mb']} MB`  ",
            f"> **Discovered Methods**: `{len(all_m)} ConnectRPC Endpoints`  ",
            f"> **Protocol**: ConnectRPC (HTTP/2 + JSON/Protobuf) over local TLS with `x-codeium-csrf-token`",
            "",
            "---",
            "",
            "## 🧭 Architecture & Discovery Invariants",
            "",
            "1. **Ephemeral Local Port**: `language_server.exe` binds to a dynamic high-range TCP port on `127.0.0.1` logged in `AppData/Roaming/Antigravity/logs/language_server.log`.",
            "2. **CSRF Protection**: All HTTP requests require header `x-codeium-csrf-token` matching `--csrf_token` in the process command line.",
            "3. **Wire Protocol**: ConnectRPC v1 (`Connect-Protocol-Version: 1`, `Content-Type: application/json`).",
            "",
            "---",
            "",
            "## 📚 Comprehensive Method Catalog by Domain",
            "",
        ]

        for cat, items in categorized.items():
            lines.append(f"### {cat} ({len(items)} methods)")
            lines.append("")
            for it in items:
                lines.append(f"- `{it}`")
            lines.append("")

        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text("\n".join(lines), encoding="utf-8")
        print(f"✅ Exported language server canon to {out_path}")

    elif args.command == "run":
        res = run_command_headless(args.cmd, cwd=args.cwd, timeout_sec=args.timeout)
        if res.get("stdout"):
            sys.stdout.write(res["stdout"])
        if res.get("stderr"):
            sys.stderr.write(res["stderr"])
        sys.exit(res.get("exit_code", 0))

    else:
        parser.print_help()


if __name__ == "__main__":
    main()
