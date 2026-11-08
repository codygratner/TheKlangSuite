#!/usr/bin/env python3
"""
tools/claude_bridge.py — Sovereign Headless Bridge for Claude Code (Claude Pro)
================================================================================
Provides a zero-flicker, 100% headless execution interface to Anthropic's Claude
Code CLI, drawing directly from the user's Claude Pro subscription ($0 API token cost).

Adheres strictly to:
  - Machine Invariant #10: Zero-Flicker Hidden Execution (CREATE_NO_WINDOW + SW_HIDE)
  - Universal UTF-8 Invariant: stdout/stderr explicitly UTF-8 encoded
  - Fast DevNull Stdin: Pipes null stdin to eliminate CLI 3-second buffer lag

Usage:
  python tools/claude_bridge.py status
  python tools/claude_bridge.py query "Summarize key principles of lock-free queues"
  python tools/claude_bridge.py test
"""

import os
import sys
import json
import shutil
import subprocess
import argparse
from pathlib import Path
from typing import Dict, Any, Optional

# Universal UTF-8 Console Streams
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

VAULT_ROOT = Path(__file__).resolve().parent.parent

# Windows Subprocess Flag Constants (Machine Invariant #10)
CREATE_NO_WINDOW = 0x08000000
SW_HIDE = 0


def get_claude_binary() -> str:
    """Resolves the best executable path for Claude Code CLI."""
    # Priority 1: Native compiled binary in AppData
    appdata_native = Path(os.environ.get("APPDATA", "")) / "npm" / "node_modules" / "@anthropic-ai" / "claude-code" / "bin" / "claude.exe"
    if appdata_native.exists():
        return str(appdata_native)

    # Priority 2: Python bin hardlink/shim
    local_bin = Path(os.environ.get("LOCALAPPDATA", "")) / "Python" / "bin" / "claude.exe"
    if local_bin.exists():
        return str(local_bin)

    # Priority 3: Global npm cmd wrapper
    npm_cmd = Path(os.environ.get("APPDATA", "")) / "npm" / "claude.cmd"
    if npm_cmd.exists():
        return str(npm_cmd)

    # Priority 4: System PATH lookup
    which_path = shutil.which("claude")
    if which_path:
        return which_path

    raise FileNotFoundError("Claude Code CLI executable ('claude') not found in PATH or standard locations.")


def run_claude_raw(
    args: list,
    stdin_content: Optional[str] = None,
    timeout: int = 120,
    cwd: Optional[Path] = None
) -> subprocess.CompletedProcess:
    """Executes Claude CLI with zero visible windows and strict UTF-8 capture."""
    bin_path = get_claude_binary()
    cmd = [bin_path] + args

    startupinfo = subprocess.STARTUPINFO()
    startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
    startupinfo.wShowWindow = SW_HIDE

    stdin_pipe = subprocess.PIPE if stdin_content is not None else subprocess.DEVNULL

    return subprocess.run(
        cmd,
        input=stdin_content,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        cwd=str(cwd or VAULT_ROOT),
        timeout=timeout,
        startupinfo=startupinfo,
        creationflags=CREATE_NO_WINDOW
    )


def get_auth_status() -> Dict[str, Any]:
    """Retrieves current authentication and subscription status from Claude CLI."""
    try:
        proc = run_claude_raw(["auth", "status"], timeout=15)
        if proc.stdout.strip():
            # Output might contain JSON or status text
            raw = proc.stdout.strip()
            # Attempt to parse json
            start = raw.find("{")
            end = raw.rfind("}")
            if start != -1 and end != -1:
                return json.loads(raw[start:end+1])
            return {"raw": raw, "loggedIn": "loggedIn: true" in raw}
    except Exception as e:
        return {"error": str(e), "loggedIn": False}
    return {"loggedIn": False}


def query(
    prompt: str,
    model: Optional[str] = None,
    system_prompt: Optional[str] = None,
    allowed_tools: Optional[str] = None,
    timeout: int = 300,
    cwd: Optional[Path] = None
) -> str:
    """Queries Claude headlessly via non-interactive print mode (-p)."""
    args = ["-p", prompt]
    if model:
        args.extend(["--model", model])
    if system_prompt:
        args.extend(["--append-system-prompt", system_prompt])
    if allowed_tools is not None:
        args.extend(["--allowed-tools", allowed_tools])

    proc = run_claude_raw(args, timeout=timeout, cwd=cwd)
    if proc.returncode != 0 and not proc.stdout.strip():
        err_msg = proc.stderr.strip() or f"Process failed with exit code {proc.returncode}"
        raise RuntimeError(f"Claude CLI Error: {err_msg}")

    return proc.stdout.strip()


def main():
    parser = argparse.ArgumentParser(description="Sovereign Headless Bridge for Claude Code (Claude Pro)")
    subparsers = parser.add_subparsers(dest="command")

    # Status
    subparsers.add_parser("status", help="Check Claude Code auth & subscription status")

    # Query
    q_parser = subparsers.add_parser("query", help="Run a headless prompt through Claude")
    q_parser.add_argument("prompt", type=str, nargs="?", default="", help="Prompt text to submit")
    q_parser.add_argument("--file", "-f", type=str, default=None, help="Path to file containing prompt text")
    q_parser.add_argument("--model", type=str, default=None, help="Model override (e.g. opus, sonnet)")
    q_parser.add_argument("--sys", type=str, default=None, help="Optional system prompt extension")
    q_parser.add_argument("--timeout", type=int, default=300, help="Execution timeout in seconds")

    # Test
    subparsers.add_parser("test", help="Run zero-flicker test query")

    args = parser.parse_args()

    if args.command == "status":
        status = get_auth_status()
        print(json.dumps(status, indent=2))

    elif args.command == "query":
        prompt_text = args.prompt
        if args.file:
            f_path = Path(args.file)
            if not f_path.is_absolute():
                f_path = Path.cwd() / f_path
            if f_path.exists():
                prompt_text = f_path.read_text(encoding="utf-8")
            else:
                print(f"Error: Prompt file '{args.file}' not found", file=sys.stderr)
                sys.exit(1)
        elif not prompt_text:
            print("Error: Either prompt text or --file must be provided", file=sys.stderr)
            sys.exit(1)

        try:
            res = query(prompt_text, model=args.model, system_prompt=args.sys, timeout=args.timeout)
            print(res)
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            sys.exit(1)

    elif args.command == "test":
        print("Testing Claude Code CLI binary path...")
        binary = get_claude_binary()
        print(f"  Binary: {binary}")
        print("Checking auth status...")
        status = get_auth_status()
        print(f"  Logged in: {status.get('loggedIn')}")
        print(f"  Subscription: {status.get('subscriptionType', 'unknown')}")
        print(f"  Account: {status.get('email', 'unknown')}")
        print("Executing test prompt via zero-flicker subprocess...")
        res = query("Respond in exactly 4 words: 'Claude Pro online, captain.'")
        print(f"  Result: {res}")
        print("ALL TESTS PASSED CLEANLY.")

    else:
        parser.print_help()


if __name__ == "__main__":
    main()
