#!/usr/bin/env python3
"""
N'kai Antigravity UI Extension Installer & Validator (install_nkai_plugin.py)
Installs and validates the official N'kai UI extension plugin under:
%USERPROFILE%\\.gemini\\config\\plugins\\nkai\\
"""

import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

# Configure UTF-8 encoding on Windows
if sys.platform == "win32":
    try:
        sys.stdout.reconfigure(encoding="utf-8")
        sys.stderr.reconfigure(encoding="utf-8")
    except Exception:
        pass


def resolve_source_root() -> Path:
    candidates = [
        Path("c:/Dev/nkai"),
        Path(__file__).resolve().parent.parent,
        Path(__file__).resolve().parent.parent.parent / "nkai"
    ]
    for c in candidates:
        if (c / "plugin.json").exists() and (c / "sidecars" / "panel" / "sidecar.json").exists():
            return c
    raise FileNotFoundError("Could not locate N'kai source root with plugin.json and sidecars/panel/sidecar.json")


def get_target_dir() -> Path:
    user_home = Path(os.environ.get("USERPROFILE", os.path.expanduser("~")))
    return user_home / ".gemini" / "config" / "plugins" / "nkai"


def install_plugin(src: Path, dest: Path):
    print(f"[*] Installing N'kai UI Extension:")
    print(f"    Source:      {src}")
    print(f"    Destination: {dest}")

    dest.mkdir(parents=True, exist_ok=True)

    # 1. Root manifests
    shutil.copy2(src / "plugin.json", dest / "plugin.json")
    print("    [+] Copied plugin.json")

    # 2. Assets
    (dest / "assets").mkdir(parents=True, exist_ok=True)
    if (src / "assets").exists():
        for item in (src / "assets").iterdir():
            if item.is_file():
                shutil.copy2(item, dest / "assets" / item.name)
        print("    [+] Copied assets/ (including logo.svg)")

    # 3. Sidecars (sidecars/panel)
    panel_dest = dest / "sidecars" / "panel"
    panel_dest.mkdir(parents=True, exist_ok=True)
    panel_src = src / "sidecars" / "panel"
    for item in panel_src.iterdir():
        if item.is_file():
            shutil.copy2(item, panel_dest / item.name)
    print("    [+] Copied sidecars/panel/ (sidecar.json, package.json, main.mjs, index.html, styles.css, app.js)")

    # 4. Core (core/sidecar.css, core/sidecar.js, core/schema.json)
    core_dest = dest / "core"
    core_dest.mkdir(parents=True, exist_ok=True)
    core_src = src / "core"
    if core_src.exists():
        for item in core_src.iterdir():
            if item.is_file():
                shutil.copy2(item, core_dest / item.name)
        print("    [+] Copied core/ (sidecar.css, sidecar.js, schema.json)")

    # 5. Dist
    dist_dest = dest / "dist"
    dist_dest.mkdir(parents=True, exist_ok=True)
    dist_src = src / "dist"
    if dist_src.exists():
        for item in dist_src.iterdir():
            if item.is_file():
                shutil.copy2(item, dist_dest / item.name)
        print("    [+] Copied dist/ (standalone-template.html)")


def validate_plugin(dest: Path) -> bool:
    print(f"\n[*] Validating installed plugin at: {dest}")
    errors = []

    # 1. Validate plugin.json
    plugin_json_path = dest / "plugin.json"
    if not plugin_json_path.exists():
        errors.append("plugin.json is missing in target plugin directory.")
    else:
        try:
            with open(plugin_json_path, "r", encoding="utf-8") as f:
                pdata = json.load(f)
            for req in ["name", "description", "logo"]:
                if req not in pdata:
                    errors.append(f"plugin.json missing required key: '{req}'")
            if pdata.get("name") != "nkai":
                errors.append(f"plugin.json name should be 'nkai', got: '{pdata.get('name')}'")
            logo_rel = pdata.get("logo", "")
            if not (dest / logo_rel).exists():
                errors.append(f"Referenced logo file not found: {dest / logo_rel}")
            else:
                print("    [✓] plugin.json valid and logo.svg resolved.")
        except Exception as e:
            errors.append(f"Failed to parse plugin.json: {e}")

    # 2. Validate sidecar.json
    sidecar_json_path = dest / "sidecars" / "panel" / "sidecar.json"
    if not sidecar_json_path.exists():
        errors.append("sidecars/panel/sidecar.json is missing.")
    else:
        try:
            with open(sidecar_json_path, "r", encoding="utf-8") as f:
                sdata = json.load(f)
            if not sdata.get("has_web_ui"):
                errors.append("sidecar.json must have 'has_web_ui': true")
            views = sdata.get("ui_config", {}).get("views", [])
            has_aux_entrypoint = any(v.get("entrypoint") == "SIDECAR_UI_ENTRYPOINT_AUX_PANE" for v in views)
            if not has_aux_entrypoint:
                errors.append("sidecar.json must define view with entrypoint 'SIDECAR_UI_ENTRYPOINT_AUX_PANE'")
            else:
                print("    [✓] sidecars/panel/sidecar.json valid (has_web_ui: true, aux entrypoint configured).")
        except Exception as e:
            errors.append(f"Failed to parse sidecars/panel/sidecar.json: {e}")

    # 3. Validate package.json
    package_json_path = dest / "sidecars" / "panel" / "package.json"
    if not package_json_path.exists():
        errors.append("sidecars/panel/package.json is missing.")
    else:
        try:
            with open(package_json_path, "r", encoding="utf-8") as f:
                pkg_data = json.load(f)
            if pkg_data.get("type") != "module":
                errors.append("sidecars/panel/package.json must contain 'type': 'module'")
            else:
                print("    [✓] sidecars/panel/package.json valid (type: module).")
        except Exception as e:
            errors.append(f"Failed to parse package.json: {e}")

    # 4. Validate main.mjs
    main_mjs_path = dest / "sidecars" / "panel" / "main.mjs"
    if not main_mjs_path.exists():
        errors.append("sidecars/panel/main.mjs is missing.")
    else:
        print("    [✓] sidecars/panel/main.mjs present.")
        # Attempt syntax check if node is available
        node_bin = shutil.which("node")
        if node_bin:
            try:
                res = subprocess.run([node_bin, "--check", str(main_mjs_path)], capture_output=True, text=True)
                if res.returncode == 0:
                    print("    [✓] Node.js syntax check passed for main.mjs.")
                else:
                    errors.append(f"Node.js syntax check failed for main.mjs: {res.stderr}")
            except Exception as e:
                print(f"    [i] Node.js check execution skipped ({e})")
        else:
            print("    [i] Node.js not found in PATH; Antigravity bundles its own Node.js runtime for execution.")

    # 5. Validate UI entry point index.html
    index_html_path = dest / "sidecars" / "panel" / "index.html"
    if not index_html_path.exists():
        errors.append("sidecars/panel/index.html is missing.")
    else:
        content = index_html_path.read_text(encoding="utf-8")
        if "/preload.js" not in content:
            errors.append("sidecars/panel/index.html missing '/preload.js' script tag.")
        else:
            print("    [✓] sidecars/panel/index.html present with /preload.js bridge.")

    # 6. Report outcome
    if errors:
        print("\n❌ VALIDATION FAILED WITH ERRORS:")
        for err in errors:
            print(f"  - {err}")
        return False

    print("\n✅ ALL VALIDATION CHECKS PASSED!")
    print(f"   Plugin successfully installed to: {dest}")
    return True


def restart_running_sidecar():
    """Restarts any currently running supervised sidecar process so it loads new code immediately."""
    user_home = Path(os.environ.get("USERPROFILE", os.path.expanduser("~")))
    log_path = user_home / ".gemini" / "antigravity" / "sidecar_data" / "nkai" / "panel" / "logs" / "sidecar.log"
    if not log_path.exists():
        return
    try:
        lines = log_path.read_text(encoding="utf-8", errors="replace").splitlines()
        port = None
        for line in reversed(lines):
            if "listening on 127.0.0.1:" in line:
                port = line.split(":")[-1].strip()
                break
        if not port:
            return

        cmd = ["powershell", "-NoProfile", "-Command", f"(Get-NetTCPConnection -LocalPort {port} -State Listen -ErrorAction SilentlyContinue).OwningProcess"]
        res = subprocess.run(cmd, capture_output=True, text=True, encoding="utf-8", errors="replace", check=False)
        pid_str = res.stdout.strip()
        if pid_str and pid_str.isdigit() and int(pid_str) > 0:
            pid = int(pid_str)
            print(f"\n[*] Hot-reloading active sidecar (PID {pid} on port {port})...")
            kill_cmd = ["powershell", "-NoProfile", "-Command", f"Stop-Process -Id {pid} -Force -ErrorAction SilentlyContinue"]
            subprocess.run(kill_cmd, capture_output=True, check=False)
            print(f"    [✓] Terminated PID {pid}. Antigravity supervisor will auto-restart sidecar with fresh code.")
    except Exception as e:
        print(f"    [i] Auto-restart notice: {e}")


def main():
    try:
        src = resolve_source_root()
        dest = get_target_dir()
        install_plugin(src, dest)
        success = validate_plugin(dest)
        if success:
            restart_running_sidecar()
        sys.exit(0 if success else 1)
    except Exception as e:
        print(f"\n❌ FATAL INSTALLATION ERROR: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
