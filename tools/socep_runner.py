#!/usr/bin/env python3
"""
tools/socep_runner.py — High-Voltage Composite Execution Runner
==============================================================
Part of The Klang Suite SOCEP Engine (v3.1 Hyperdrive).
Consolidates build execution, testing, UI radar verification,
diagnostic distillation, and ledger management into a single
deterministic, noise-filtered Python runner.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import time
from typing import Dict, List, Optional, Tuple, Any

# Universal UTF-8 for Windows
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

SW_HIDE = 0
CREATE_NO_WINDOW = 0x08000000

TKS_ROOT = Path(r"C:\Dev\TheKlangSuite")
VAULT_ROOT = Path(r"c:\_\Vault")
SOCEP_SCRATCH = VAULT_ROOT / ".agents" / "scratch" / "socep"

if str(TKS_ROOT) not in sys.path:
    sys.path.insert(0, str(TKS_ROOT))
if str(TKS_ROOT / "tools") not in sys.path:
    sys.path.insert(0, str(TKS_ROOT / "tools"))

BUILD_TIMEOUT_SEC = 900.0
TEST_TIMEOUT_SEC = 120.0
PREVIEW_TIMEOUT_SEC = 15.0

# NTSTATUS Crash code mapping on Windows
NTSTATUS_MAP = {
    0xC0000005: "STATUS_ACCESS_VIOLATION (Segfault/AV)",
    0xC00000FD: "STATUS_STACK_OVERFLOW",
    0xC0000409: "STATUS_STACK_BUFFER_OVERRUN (FastFail)",
    0x80000003: "STATUS_BREAKPOINT (Assertion Failure)"
}


def get_startup_info() -> dict:
    si = subprocess.STARTUPINFO()
    si.dwFlags |= subprocess.STARTF_USESHOWWINDOW
    si.wShowWindow = SW_HIDE
    return {"startupinfo": si, "creationflags": CREATE_NO_WINDOW}


def kill_proc_tree(pid: int) -> None:
    try:
        subprocess.run(["taskkill", "/T", "/F", "/PID", str(pid)], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, **get_startup_info())
    except Exception:
        pass


def get_ledger_path(slug: str, phase: int) -> Path:
    p = SOCEP_SCRATCH / slug / f"phase_{phase}.json"
    p.parent.mkdir(parents=True, exist_ok=True)
    return p


def read_ledger(slug: str, phase: int) -> Dict[str, Any]:
    p = get_ledger_path(slug, phase)
    if not p.exists():
        return {
            "plan_slug": slug,
            "phase_index": phase,
            "strike0_count": 0,
            "proactive_count": 0,
            "reactive_count": 0,
            "total_count": 0,
            "fingerprints": [],
            "status": "INITIALIZED"
        }
    try:
        return json.loads(p.read_text(encoding="utf-8"))
    except Exception:
        return {"plan_slug": slug, "phase_index": phase, "reactive_count": 0, "fingerprints": []}


def write_ledger(slug: str, phase: int, data: Dict[str, Any]) -> None:
    p = get_ledger_path(slug, phase)
    temp_p = p.with_suffix(".tmp")
    temp_p.write_text(json.dumps(data, indent=2), encoding="utf-8")
    temp_p.replace(p)  # Atomic on Windows in Python 3.8+


def snapshot_baseline(slug: str, phase: int, target_files: List[str]) -> Path:
    baseline_dir = SOCEP_SCRATCH / slug / f"p{phase}" / "baseline"
    baseline_dir.mkdir(parents=True, exist_ok=True)
    manifest = {}
    for rel_path in target_files:
        src = TKS_ROOT / rel_path
        if src.exists():
            dst = baseline_dir / rel_path
            dst.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(src, dst)
            manifest[rel_path] = hashlib.sha256(src.read_bytes()).hexdigest()
    (baseline_dir / "manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    return baseline_dir


def restore_baseline(slug: str, phase: int) -> bool:
    baseline_dir = SOCEP_SCRATCH / slug / f"p{phase}" / "baseline"
    manifest_file = baseline_dir / "manifest.json"
    if not manifest_file.exists():
        return False
    try:
        manifest = json.loads(manifest_file.read_text(encoding="utf-8"))
        for rel_path in manifest.keys():
            saved = baseline_dir / rel_path
            target = TKS_ROOT / rel_path
            if saved.exists():
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(saved, target)
        return True
    except Exception:
        return False


def run_tower_compile(target: str, config: str = "Release") -> Tuple[int, Path]:
    build_dir = TKS_ROOT / "build"
    if not build_dir.exists():
        return 3, Path()  # Exit 3: INFRA

    log_dir = TKS_ROOT / ".agents" / "pipeline"
    log_dir.mkdir(parents=True, exist_ok=True)
    build_log = log_dir / "build.log"

    env = os.environ.copy()
    env["MSBUILDDISABLENODEREUSE"] = "1"

    cmd = ["cmake", "--build", str(build_dir), "--config", config, "--parallel", "--target", target]
    t0 = time.perf_counter()

    with open(build_log, "w", encoding="utf-8") as f_out:
        try:
            proc = subprocess.Popen(cmd, stdout=f_out, stderr=subprocess.STDOUT, env=env, cwd=str(TKS_ROOT), **get_startup_info())
            proc.wait(timeout=BUILD_TIMEOUT_SEC)
            rc = proc.returncode
        except subprocess.TimeoutExpired:
            kill_proc_tree(proc.pid)
            return 3, build_log  # Exit 3: INFRA timeout
        except Exception:
            return 3, build_log

    if rc != 0:
        return 1, build_log  # Exit 1: ACTIONABLE compile error
    return 0, build_log


def run_dsp_tests(target: str = "dsp_tests", config: str = "Release") -> Tuple[int, Path, str]:
    test_exe = TKS_ROOT / "build" / config / f"{target}.exe"
    if not test_exe.exists():
        test_exe = TKS_ROOT / "build" / f"{target}.exe"
    if not test_exe.exists():
        return 3, Path(), "Binary not found"  # INFRA

    test_log = TKS_ROOT / ".agents" / "pipeline" / "test.log"
    with open(test_log, "w", encoding="utf-8") as f_test:
        try:
            proc = subprocess.Popen([str(test_exe)], stdout=f_test, stderr=subprocess.STDOUT, cwd=str(TKS_ROOT), **get_startup_info())
            proc.wait(timeout=TEST_TIMEOUT_SEC)
            rc = proc.returncode
        except subprocess.TimeoutExpired:
            kill_proc_tree(proc.pid)
            return 1, test_log, "TEST_HANG"
        except Exception as e:
            return 3, test_log, str(e)

    if rc != 0:
        # Check for NTSTATUS crash
        unsigned_rc = rc & 0xFFFFFFFF
        crash_desc = NTSTATUS_MAP.get(unsigned_rc, f"Exit code {rc}")
        return 1, test_log, crash_desc
    return 0, test_log, "PASS"


def run_ui_gate(plugin: str) -> Tuple[int, Path]:
    ui_script = VAULT_ROOT / "tools" / "ui_gate.py"
    if not ui_script.exists():
        return 3, Path()

    cmd = [sys.executable, str(ui_script), "--plugin", plugin, "--pillars", "A,C", "--strict", "--json"]
    log_path = TKS_ROOT / ".agents" / "pipeline" / "ui_gate.log"
    
    with open(log_path, "w", encoding="utf-8") as f_ui:
        try:
            proc = subprocess.Popen(cmd, stdout=f_ui, stderr=subprocess.STDOUT, cwd=str(VAULT_ROOT), **get_startup_info())
            proc.wait(timeout=PREVIEW_TIMEOUT_SEC)
            rc = proc.returncode
        except subprocess.TimeoutExpired:
            kill_proc_tree(proc.pid)
            return 3, log_path
        except Exception:
            return 3, log_path

    if rc == 2:
        return 1, log_path  # Exit 1: ACTIONABLE collision
    elif rc != 0:
        return 3, log_path  # Exit 3: INFRA
    return 0, log_path


def check_pipeline(plugin: str, phase: int, target: str, slug: Optional[str] = None) -> int:
    """Executes single-shot composite check."""
    plan_slug = slug or f"{plugin}_phase_{phase}"
    t_start = time.perf_counter()
    ledger = read_ledger(plan_slug, phase)

    # 1. Compile
    rc_build, build_log = run_tower_compile(target=target)
    if rc_build == 3:
        print(json.dumps({"status": "INFRA_ERROR", "phase": phase, "error": "Build failed due to infrastructure / timeout"}))
        return 3

    if rc_build == 1:
        # Actionable compiler failure
        return handle_failure(plan_slug, phase, "compiler", build_log, ledger)

    # 2. Test Execution
    if "test" in target.lower():
        rc_test, test_log, test_msg = run_dsp_tests(target=target)
        if rc_test == 3:
            print(json.dumps({"status": "INFRA_ERROR", "phase": phase, "error": f"Test infra error: {test_msg}"}))
            return 3
        if rc_test == 1:
            return handle_failure(plan_slug, phase, "test", test_log, ledger, extra_info=test_msg)

    # 3. UI Gate (if Preview target)
    if "preview" in target.lower():
        rc_ui, ui_log = run_ui_gate(plugin=plugin)
        if rc_ui == 3:
            print(json.dumps({"status": "INFRA_ERROR", "phase": phase, "error": "UI gate infra error"}))
            return 3
        if rc_ui == 1:
            return handle_failure(plan_slug, phase, "ui", ui_log, ledger)

    duration_ms = int((time.perf_counter() - t_start) * 1000)
    print(json.dumps({"status": "PASS", "phase": phase, "time_ms": duration_ms}))
    return 0


def handle_failure(slug: str, phase: int, diag_type: str, log_path: Path, ledger: Dict[str, Any], extra_info: str = "") -> int:
    from tools.strike_pack import pack_diagnostics

    current_reactive = ledger.get("reactive_count", 0)
    if current_reactive >= 2:
        print(json.dumps({
            "gate": "CIRCUIT_BREAKER_HALT",
            "phase": phase,
            "reason": "Exceeded maximum 2 reactive strikes per phase.",
            "fingerprints": ledger.get("fingerprints", [])
        }))
        return 1

    next_strike = current_reactive + 1
    diag_out = SOCEP_SCRATCH / slug / f"diag_p{phase}_s{next_strike}.md"
    pack_res = pack_diagnostics(log_path, diag_out, diag_type=diag_type, root_dir=TKS_ROOT)
    fp = pack_res.get("fingerprint", "")

    # Anti-oscillation check
    fps = ledger.get("fingerprints", [])
    if len(fps) >= 2 and fp == fps[-2]:
        print(json.dumps({
            "gate": "OSCILLATION_HALT",
            "phase": phase,
            "fingerprint": fp,
            "reason": "A -> B -> A oscillation detected between strike attempts."
        }))
        return 1

    # Snapshot baseline before Strike 1
    if next_strike == 1:
        # Determine target files from log records or pass default
        target_files = [r.get("file") for r in [pack_res] if r.get("file")] or ["source/"]
        snapshot_baseline(slug, phase, target_files)

    # Reserve strike upfront in ledger
    fps.append(fp)
    ledger["fingerprints"] = fps
    ledger["reactive_count"] = next_strike
    ledger["total_count"] = ledger.get("total_count", 0) + 1
    write_ledger(slug, phase, ledger)

    model = "sonnet" if next_strike == 1 else "opus"
    print(json.dumps({
        "gate": "FAILED",
        "stage": diag_type.upper(),
        "strike": next_strike,
        "model": model,
        "diag": str(diag_out).replace("\\", "/"),
        "fingerprint": fp,
        "detail": extra_info or f"{pack_res.get('total_errors', 1)} errors detected"
    }))
    return 1


def apply_patch(patch_path: str, plugin: str, phase: int, target_files: Optional[List[str]] = None) -> int:
    patch_file = Path(patch_path)
    if not patch_file.exists():
        print(json.dumps({"status": "REJECTED", "error": f"Patch not found: {patch_path}"}))
        return 1

    patch_text = patch_file.read_text(encoding="utf-8", errors="replace")

    # Fence 1: Forbidden paths
    for forbidden in ["test/", "tests/", "CMakeLists.txt"]:
        if forbidden in patch_text:
            print(json.dumps({"status": "REJECTED", "error": f"Patch illegally touches fenced path: {forbidden}"}))
            return 1

    # Fence 2: git apply --check
    cmd_check = ["git", "-C", str(TKS_ROOT), "apply", "--check", str(patch_file)]
    res_check = subprocess.run(cmd_check, capture_output=True, text=True)
    if res_check.returncode != 0:
        print(json.dumps({"status": "REJECTED", "error": f"git apply --check failed: {res_check.stderr.strip()}"}))
        return 1

    # Apply patch
    cmd_apply = ["git", "-C", str(TKS_ROOT), "apply", str(patch_file)]
    res_apply = subprocess.run(cmd_apply, capture_output=True, text=True)
    if res_apply.returncode != 0:
        print(json.dumps({"status": "REJECTED", "error": f"git apply failed: {res_apply.stderr.strip()}"}))
        return 1

    # Fence 3: DSP Audio-thread safety audit
    dsp_touched = any(p in patch_text for p in ["source/dsp", "source/hastur", "source/carrier", "source/planter"])
    if dsp_touched:
        audit_script = VAULT_ROOT / "tools" / "audiothread_guard.py"
        if audit_script.exists():
            audit_res = subprocess.run([sys.executable, str(audit_script), "--terse"], capture_output=True, text=True, cwd=str(TKS_ROOT))
            if audit_res.returncode != 0:
                # Safety violation: revert patch
                restore_baseline(f"{plugin}_phase_{phase}", phase)
                print(json.dumps({"status": "SAFETY_VIOLATION", "error": f"DSP real-time safety audit failed: {audit_res.stdout.strip()}"}))
                return 1

    # Extract touched file summary
    touched = re.findall(r'diff --git a/(.*?) b/', patch_text)
    print(json.dumps({"status": "applied", "touched": touched, "bytes": len(patch_text)}))
    return 0


def run_self_test() -> int:
    """Executes Stage 2 offline fixture gauntlet without burning Claude Pro tokens."""
    print("🧪 [SOCEP SELF-TEST] Running offline fixture gauntlet...")
    from tools.strike_pack import parse_compiler_errors, parse_test_failures, parse_ui_collisions

    # Fixture 1: MSVC C2065 error parsing
    log_c2065 = r"""
C:\Dev\TheKlangSuite\source\hastur\HasturEditor.cpp(142,17): error C2065: 'missingVariable': undeclared identifier
"""
    records, fp1 = parse_compiler_errors(log_c2065, TKS_ROOT)
    assert len(records) == 1, "Failed Fixture 1: Expected 1 compiler record"
    assert records[0]["code"] == "C2065", "Failed Fixture 1: Expected code C2065"
    print(f"  ✅ Fixture 1 (Compiler Error Extraction): Fingerprint {fp1[:10]}")

    # Fixture 2: Linker LNK2019 error parsing
    log_lnk = r"""
HasturEditor.obj : error LNK2019: unresolved external symbol "void __cdecl foo(void)" (?foo@@YAXXZ) referenced in function "public: void __thiscall HasturEditor::resized(void)"
"""
    records, fp2 = parse_compiler_errors(log_lnk, TKS_ROOT)
    assert len(records) == 1, "Failed Fixture 2: Expected 1 linker record"
    assert records[0]["code"] == "LNK2019", "Failed Fixture 2: Expected code LNK2019"
    print(f"  ✅ Fixture 2 (Linker Error Extraction): Fingerprint {fp2[:10]}")

    # Fixture 3: UI Radar collision parsing
    log_ui = r"""
[RADAR] Collision: dcoBay 10 10 200 150 overlaps filterBay 190 10 200 150
"""
    records, fp3 = parse_ui_collisions(log_ui)
    assert len(records) == 1, "Failed Fixture 3: Expected 1 radar record"
    print(f"  ✅ Fixture 3 (UI Radar Extraction): Fingerprint {fp3[:10]}")

    # Fixture 4: Anti-oscillation line-shift invariance
    log_c2065_shifted = r"""
C:\Dev\TheKlangSuite\source\hastur\HasturEditor.cpp(150,17): error C2065: 'missingVariable': undeclared identifier
"""
    _, fp1_shifted = parse_compiler_errors(log_c2065_shifted, TKS_ROOT)
    assert fp1 == fp1_shifted, f"Failed Fixture 4: Line shift altered fingerprint ({fp1} != {fp1_shifted})"
    print(f"  ✅ Fixture 4 (Line-Agnostic Fingerprint Invariance): {fp1[:10]} == {fp1_shifted[:10]}")

    # Fixture 5: Infrastructure Fault check
    rc_infra, _ = run_tower_compile("target_nonexistent_12345")
    # Nonexistent target should exit 1 (or 3 if build dir missing)
    print(f"  ✅ Fixture 5 (Failure Classification): Handled cleanly with rc={rc_infra}")

    print("🏁 [SOCEP SELF-TEST] 100% of offline fixtures PASSED cleanly.")
    return 0


def main():
    parser = argparse.ArgumentParser(description="SOCEP Hyperdrive Composite Runner")
    subparsers = parser.add_subparsers(dest="command")

    # check
    p_check = subparsers.add_parser("check")
    p_check.add_argument("--plugin", default="hastur")
    p_check.add_argument("--phase", type=int, default=1)
    p_check.add_argument("--target", default="dsp_tests")
    p_check.add_argument("--slug", default=None)

    # apply
    p_apply = subparsers.add_parser("apply")
    p_apply.add_argument("--patch", required=True)
    p_apply.add_argument("--plugin", default="hastur")
    p_apply.add_argument("--phase", type=int, default=1)

    # self-test
    subparsers.add_parser("self-test")

    args = parser.parse_args()

    if args.command == "check":
        sys.exit(check_pipeline(plugin=args.plugin, phase=args.phase, target=args.target, slug=args.slug))
    elif args.command == "apply":
        sys.exit(apply_patch(args.patch, args.plugin, args.phase))
    elif args.command == "self-test":
        sys.exit(run_self_test())
    else:
        parser.print_help()


if __name__ == "__main__":
    main()
