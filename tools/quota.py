import subprocess
import re
import urllib.request
import ssl
import json
import time
import os
import sys
from pathlib import Path

# Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    try:
        sys.stdout.reconfigure(encoding="utf-8")
    except Exception:
        pass
if hasattr(sys.stderr, "reconfigure"):
    try:
        sys.stderr.reconfigure(encoding="utf-8")
    except Exception:
        pass

CACHE_PATH = Path(".agents") / "cache" / "antigravity_rpc_cache.json"

def _query_rpc(port: int, csrf_token: str) -> dict:
    url = f"https://127.0.0.1:{port}/exa.language_server_pb.LanguageServerService/RetrieveUserQuotaSummary"
    ctx = ssl.create_default_context()
    ctx.check_hostname = False
    ctx.verify_mode = ssl.CERT_NONE

    headers = {
        "x-codeium-csrf-token": csrf_token,
        "Content-Type": "application/json",
        "Connect-Protocol-Version": "1",
    }

    req = urllib.request.Request(url, headers=headers, data=b"{}")
    with urllib.request.urlopen(req, timeout=1.5, context=ctx) as resp:
        data = json.loads(resp.read().decode("utf-8"))
        return data.get("response", {})

def _discover_session() -> tuple[int, str]:
    # 1. Get language_server command line to extract CSRF token
    script = "Get-CimInstance Win32_Process -Filter \"name = 'language_server.exe' and CommandLine like '%--standalone%'\" | Select-Object ProcessId, CommandLine | ConvertTo-Json"
    creationflags = 0x08000000 if sys.platform == "win32" else 0
    res = subprocess.run(
        ["powershell", "-NoProfile", "-Command", script],
        capture_output=True,
        text=True,
        encoding="utf-8",
        creationflags=creationflags,
    )
    cmdline = res.stdout
    m_token = re.search(r"--csrf_token[=\s]+([a-f0-9\-]+)", cmdline)
    if not m_token:
        raise RuntimeError("Could not find --csrf_token in language_server.exe command line")
    csrf_token = m_token.group(1)

    # 2. Get active HTTPS port from language_server.log
    log_path = Path.home() / "AppData" / "Roaming" / "Antigravity" / "logs" / "language_server.log"
    port = None
    if log_path.exists():
        with open(log_path, "r", encoding="utf-8", errors="ignore") as f:
            text = f.read()
            matches = re.findall(r"Language server listening on random port at (\d+) for HTTPS", text)
            if matches:
                port = int(matches[-1])

    if not port:
        raise RuntimeError("Could not discover language_server HTTPS port from logs")

    # Cache credentials
    CACHE_PATH.parent.mkdir(parents=True, exist_ok=True)
    CACHE_PATH.write_text(json.dumps({"port": port, "csrf_token": csrf_token}), encoding="utf-8")
    return port, csrf_token

def get_quota_summary(force_refresh: bool = False) -> dict:
    if not force_refresh and CACHE_PATH.exists():
        try:
            cached = json.loads(CACHE_PATH.read_text(encoding="utf-8"))
            return _query_rpc(cached["port"], cached["csrf_token"])
        except Exception:
            pass # Cache miss or expired, rediscovering

    port, csrf = _discover_session()
    return _query_rpc(port, csrf)

if __name__ == "__main__":
    t0 = time.perf_counter()
    quota = get_quota_summary()
    elapsed = (time.perf_counter() - t0) * 1000
    print(f"📊 Antigravity Real-Time Model Quota & Allowance (Retrieved in {elapsed:.1f}ms)\n")
    for group in quota.get("groups", []):
        print(f"🔹 {group.get('displayName')}:")
        for b in group.get("buckets", []):
            pct = b.get("remainingFraction", 0) * 100
            window = b.get("window", "")
            desc = b.get("description", "")
            reset = b.get("resetTime", "")
            print(f"   • {b.get('displayName')} [{window}]: {pct:.1f}% remaining")
            if desc:
                print(f"     Status: {desc}")
            if reset:
                print(f"     Resets at: {reset}")
        print()
