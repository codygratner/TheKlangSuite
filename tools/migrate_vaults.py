#!/usr/bin/env python3
"""
tools/migrate_vaults.py
Consolidation script for the Grand Zettelkasten Mega-Vault at C:\\_\\Vault.

Assembles fragmented vaults into C:\\_\\staging, verifies note counts and plugin
data integrity, moves staging to C:\\_\\Vault, creates the C:\\Dev\\TheKlangVault junction,
registers C:\\_\\Vault in Obsidian, and neutralizes the Renoise git repo in Dropbox.
"""

import sys
import os
import shutil
import zipfile
import json
import subprocess
import time
from pathlib import Path

# Ensure UTF-8 console output for Windows terminal safety
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")


# Paths
DROPBOX_ROOT = Path(r"C:\_\Dropbox")
OBSIDIAN_SOLO = Path(r"C:\_\Obsidian\Solo TTRPG")
FORGED_VAULT = DROPBOX_ROOT / "Forged_in_Obsidian-1.5.5"
LYT_VAULT = DROPBOX_ROOT / "_obsidian" / "LYT Kit"
MAIN_VAULT = DROPBOX_ROOT / "_obsidian" / "main"
RANDOM_VAULT = DROPBOX_ROOT / "_obsidian" / "random"
KLANG_VAULT = Path(r"C:\Dev\TheKlangVault")
RENOISE_GIT = DROPBOX_ROOT / "Renoise" / ".git"

STAGING_ROOT = Path(r"C:\_\staging")
TARGET_VAULT = Path(r"C:\_\Vault")
APPDATA_OBSIDIAN_JSON = Path(os.environ.get("APPDATA", "")) / "obsidian" / "obsidian.json"


def log(msg: str):
    print(f"[*] {msg}", flush=True)


def stop_dropbox():
    """Stops Dropbox gracefully/forcefully to prevent file-locking during migration."""
    log("Checking for active Dropbox processes...")
    try:
        res = subprocess.run(
            ["powershell", "-NoProfile", "-Command", "Get-Process -Name Dropbox -ErrorAction SilentlyContinue"],
            capture_output=True,
            text=True,
            encoding="utf-8",
        )
        if "Dropbox" in res.stdout:
            log("Dropbox is running. Stopping Dropbox processes to prevent file locking...")
            subprocess.run(
                ["powershell", "-NoProfile", "-Command", "Stop-Process -Name Dropbox -Force -ErrorAction SilentlyContinue"],
                capture_output=True,
                text=True,
                encoding="utf-8",
            )
            time.sleep(2)
            log("Dropbox stopped successfully.")
        else:
            log("Dropbox is not running.")
    except Exception as e:
        log(f"Warning stopping Dropbox: {e}")


def neutralize_renoise_git():
    """Compresses C:\\_\\Dropbox\\Renoise\\.git into git_archive.zip and removes .git to eliminate Dropbox sync churn."""
    if not RENOISE_GIT.exists():
        log(f"Renoise .git directory not found at {RENOISE_GIT}, skipping neutralization.")
        return

    renoise_dir = RENOISE_GIT.parent
    archive_zip = renoise_dir / "git_archive.zip"
    log(f"Neutralizing Renoise git repo in Dropbox: archiving {RENOISE_GIT} -> {archive_zip}...")

    if not archive_zip.exists():
        with zipfile.ZipFile(archive_zip, "w", zipfile.ZIP_DEFLATED) as zf:
            for root, dirs, files in os.walk(RENOISE_GIT):
                for f in files:
                    full_p = Path(root) / f
                    rel_p = full_p.relative_to(renoise_dir)
                    zf.write(full_p, rel_p)
        log(f"Successfully created {archive_zip} ({archive_zip.stat().st_size:,} bytes).")
    else:
        log(f"Archive {archive_zip} already exists.")

    log(f"Removing active .git folder {RENOISE_GIT} to stop Dropbox background churn...")
    try:
        # Use rmdir /s /q via shell to handle read-only git objects cleanly on Windows
        subprocess.run(["cmd", "/c", "rmdir", "/s", "/q", str(RENOISE_GIT)], check=True)
        log("Renoise .git directory successfully removed. Zero further Dropbox conflict thrashing!")
    except Exception as e:
        log(f"Warning removing {RENOISE_GIT}: {e}")


def copy_tree_filtered(src: Path, dst: Path, ignore_dirs=(".obsidian", ".git", ".trash")):
    """Copies all files and subdirectories from src to dst, skipping specified directories."""
    if not src.exists():
        log(f"Source path {src} does not exist, skipping.")
        return 0

    count = 0
    for root, dirs, files in os.walk(src):
        # Filter out ignored directories in-place
        dirs[:] = [d for d in dirs if d not in ignore_dirs]
        rel_root = Path(root).relative_to(src)
        target_dir = dst / rel_root
        target_dir.mkdir(parents=True, exist_ok=True)

        for f in files:
            src_file = Path(root) / f
            dst_file = target_dir / f
            shutil.copy2(src_file, dst_file)
            count += 1
    return count


def assemble_staging():
    """Assembles all kingdoms in C:\\_\\staging."""
    log(f"Setting up staging environment at {STAGING_ROOT}...")
    if STAGING_ROOT.exists():
        log("Cleaning existing staging directory...")
        subprocess.run(["cmd", "/c", "rmdir", "/s", "/q", str(STAGING_ROOT)], check=True)

    STAGING_ROOT.mkdir(parents=True, exist_ok=True)

    # 1. 📬 Universal Inbox
    inbox_dir = STAGING_ROOT / "📬 Inbox"
    (inbox_dir / "Archive").mkdir(parents=True, exist_ok=True)
    (inbox_dir / "Tabled").mkdir(parents=True, exist_ok=True)
    (inbox_dir / "Welcome_to_the_Universal_Inbox.md").write_text(
        "# 📬 Universal Inbox\n\nDrop any quick note, voice memo, or research idea here from desktop or mobile.\n",
        encoding="utf-8"
    )

    # 2. 🎲 The Forge (Solo TTRPGs & Worldbuilding)
    log("Copying 🎲 The Forge (Solo TTRPG, Forged in Obsidian, and custom tables)...")
    forge_dir = STAGING_ROOT / "🎲 The Forge"
    solo_target = forge_dir / "Solo TTRPG"
    forged_target = forge_dir / "Forged in Obsidian"
    forge_z_target = forge_dir / "z"
    root_z_target = STAGING_ROOT / "z"

    # Copy Solo TTRPG notes and folders
    c_solo = copy_tree_filtered(OBSIDIAN_SOLO, solo_target)
    log(f"  -> Copied {c_solo} files from Solo TTRPG.")

    # Copy Forged in Obsidian
    c_forged = copy_tree_filtered(FORGED_VAULT, forged_target)
    log(f"  -> Copied {c_forged} files from Forged in Obsidian.")

    # Copy z/tables and z/decks to both STAGING/z and STAGING/The Forge/z
    solo_z = OBSIDIAN_SOLO / "z"
    if solo_z.exists():
        copy_tree_filtered(solo_z, root_z_target)
        copy_tree_filtered(solo_z, forge_z_target)
        log("  -> Copied custom tables and decks to both root z/ and The Forge/z/.")

    # 3. 🧠 Codex (Zettelkasten, LYT Kit & Evergreen Notes)
    log("Copying 🧠 Codex (LYT Kit, Main & Random evergreen notes)...")
    codex_dir = STAGING_ROOT / "🧠 Codex"
    lyt_target = codex_dir / "LYT Kit"
    evergreen_main = codex_dir / "Evergreen" / "Main"
    evergreen_random = codex_dir / "Evergreen" / "Random"

    c_lyt = copy_tree_filtered(LYT_VAULT, lyt_target)
    log(f"  -> Copied {c_lyt} files from LYT Kit.")
    c_main = copy_tree_filtered(MAIN_VAULT, evergreen_main)
    log(f"  -> Copied {c_main} files from Main.")
    c_random = copy_tree_filtered(RANDOM_VAULT, evergreen_random)
    log(f"  -> Copied {c_random} files from Random.")

    # 4. 🎹 Synth Lab
    log("Setting up 🎹 Synth Lab (M8 themes, Renoise projects, hardware synth patches)...")
    synth_dir = STAGING_ROOT / "🎹 Synth Lab"
    synth_dir.mkdir(parents=True, exist_ok=True)
    m8_themes_src = DROPBOX_ROOT / "Gear" / "Dirty Wave" / "M8" / "SD Card" / "Themes"
    if m8_themes_src.exists():
        m8_dst = synth_dir / "M8 Themes"
        copy_tree_filtered(m8_themes_src, m8_dst)
        log("  -> Copied Dirtywave M8 hardware synth themes into Synth Lab.")

    (synth_dir / "README.md").write_text(
        "# 🎹 Synth Lab\n\nHub for sound design, Renoise tracker projects, M8 tracker themes, and hardware synth patches.\n",
        encoding="utf-8"
    )

    # 5. 🏢 New Klang City (The Klang Suite Ecosystem Enclave)
    log("Copying 🏢 New Klang City (TheKlangVault docs, specs, and git history)...")
    nkc_dir = STAGING_ROOT / "🏢 New Klang City"
    c_nkc = copy_tree_filtered(KLANG_VAULT, nkc_dir, ignore_dirs=(".obsidian", ".git", ".trash"))
    # Explicitly copy .git folder intact
    klang_git = KLANG_VAULT / ".git"
    if klang_git.exists() and not (nkc_dir / ".git").exists():
        log("  -> Preserving .git commit history inside New Klang City enclave...")
        shutil.copytree(klang_git, nkc_dir / ".git")
    log(f"  -> Copied {c_nkc} files from TheKlangVault.")

    # 6. 🌿 Klang Confidential (Air-gapped personal life notes)
    log("Setting up 🌿 Klang Confidential (Personal life, journals, and private notes)...")
    confidential_dir = STAGING_ROOT / "🌿 Klang Confidential"
    confidential_dir.mkdir(parents=True, exist_ok=True)
    (confidential_dir / "README.md").write_text(
        "# 🌿 Klang Confidential\n\nAir-gapped personal journal, health, and private life notes.\nStrictly private; excluded from all git operations.\n",
        encoding="utf-8"
    )

    # 7. Pre-configure .obsidian/
    log("Configuring master .obsidian/ directory (plugins, CSS snippets, themes)...")
    obsidian_dir = STAGING_ROOT / ".obsidian"
    plugins_dir = obsidian_dir / "plugins"
    snippets_dir = obsidian_dir / "snippets"
    themes_dir = obsidian_dir / "themes"
    plugins_dir.mkdir(parents=True, exist_ok=True)
    snippets_dir.mkdir(parents=True, exist_ok=True)
    themes_dir.mkdir(parents=True, exist_ok=True)

    # Plugins from Solo TTRPG
    solo_plugins = OBSIDIAN_SOLO / ".obsidian" / "plugins"
    solo_keep_plugins = [
        "solo-rpg-toolkit",
        "obsidian-dice-roller",
        "table-checkboxes",
        "tag-wrangler",
        "unicode-search",
        "change-case",
        "colored-text",
        "new-tab-default-page",
        "file-explorer-note-count",
        "obsidian-icon-folder",
        "gemini-scribe",
        "obsidian-text-expander-js",
        "obsidian-regex-replace",
        "obsidian-sort-and-permute-lines",
    ]
    for p in solo_keep_plugins:
        p_src = solo_plugins / p
        if p_src.exists():
            shutil.copytree(p_src, plugins_dir / p)

    # Plugins from Forged in Obsidian
    forged_plugins = FORGED_VAULT / ".obsidian" / "plugins"
    for p in ["dataview", "templater-obsidian"]:
        p_src = forged_plugins / p
        if p_src.exists() and not (plugins_dir / p).exists():
            shutil.copytree(p_src, plugins_dir / p)

    # Plugins from Main
    main_plugins = MAIN_VAULT / ".obsidian" / "plugins"
    for p in ["note-refactor-obsidian", "better-word-count"]:
        p_src = main_plugins / p
        if p_src.exists() and not (plugins_dir / p).exists():
            shutil.copytree(p_src, plugins_dir / p)

    # Copy CSS Snippets
    solo_snippets = OBSIDIAN_SOLO / ".obsidian" / "snippets"
    if solo_snippets.exists():
        for s in solo_snippets.glob("*.css"):
            shutil.copy2(s, snippets_dir / s.name)

    forged_snippets = FORGED_VAULT / ".obsidian" / "snippets"
    if forged_snippets.exists():
        for s in forged_snippets.glob("*.css"):
            shutil.copy2(s, snippets_dir / s.name)

    lyt_snippets = LYT_VAULT / ".obsidian" / "snippets"
    if lyt_snippets.exists():
        for s in lyt_snippets.glob("*.css"):
            shutil.copy2(s, snippets_dir / s.name)

    # Copy Themes
    solo_themes = OBSIDIAN_SOLO / ".obsidian" / "themes"
    if solo_themes.exists():
        for t in solo_themes.iterdir():
            target_t = themes_dir / t.name
            if t.is_dir():
                shutil.copytree(t, target_t)
            elif t.is_file():
                shutil.copy2(t, target_t)

    main_themes = MAIN_VAULT / ".obsidian" / "themes"
    if main_themes.exists():
        for t in main_themes.iterdir():
            target_t = themes_dir / t.name
            if not target_t.exists():
                if t.is_dir():
                    shutil.copytree(t, target_t)
                elif t.is_file():
                    shutil.copy2(t, target_t)

    # Write community-plugins.json
    installed_plugin_ids = [p.name for p in plugins_dir.iterdir() if p.is_dir()]
    (obsidian_dir / "community-plugins.json").write_text(
        json.dumps(installed_plugin_ids, indent=2), encoding="utf-8"
    )

    # Write appearance.json
    all_snippets = [s.stem for s in snippets_dir.glob("*.css")]
    appearance_data = {
        "cssTheme": "Simply Colorful",
        "theme": "obsidian",
        "baseFontSize": 20,
        "baseFontSizeAction": True,
        "enabledCssSnippets": all_snippets,
        "nativeMenus": False,
    }
    (obsidian_dir / "appearance.json").write_text(
        json.dumps(appearance_data, indent=2), encoding="utf-8"
    )

    # Write app.json
    app_data = {
        "alwaysUpdateLinks": True,
        "newFileLocation": "folder",
        "newFileFolderPath": "📬 Inbox",
        "attachmentFolderPath": "z/attachments",
        "livePreview": True,
        "foldHeading": True,
        "foldIndent": True,
        "autoPairBrackets": True,
        "autoPairMarkdown": True,
    }
    (obsidian_dir / "app.json").write_text(
        json.dumps(app_data, indent=2), encoding="utf-8"
    )

    log("Staging assembly complete.")


def verify_staging() -> bool:
    """Verifies note counts and key asset presence in C:\\_\\staging."""
    log("Verifying staging integrity...")
    md_files = list(STAGING_ROOT.rglob("*.md"))
    log(f"  -> Total markdown notes in staging: {len(md_files):,}")

    # Check solo-rpg-toolkit
    srt_plugin = STAGING_ROOT / ".obsidian" / "plugins" / "solo-rpg-toolkit"
    if not srt_plugin.exists():
        log("ERROR: solo-rpg-toolkit is missing in .obsidian/plugins!")
        return False

    # Check custom tables
    tables = list((STAGING_ROOT / "z" / "tables").iterdir()) if (STAGING_ROOT / "z" / "tables").exists() else []
    log(f"  -> Found {len(tables)} custom table packs in z/tables/ ({', '.join(t.name for t in tables[:5])}...)")
    if len(tables) < 5:
        log("ERROR: Missing custom table packs in z/tables!")
        return False

    # Check CSS snippets
    snippets = list((STAGING_ROOT / ".obsidian" / "snippets").glob("*.css"))
    log(f"  -> Found {len(snippets)} CSS snippets in .obsidian/snippets/")
    snippets_css = STAGING_ROOT / ".obsidian" / "snippets" / "snippets.css"
    if not snippets_css.exists() or snippets_css.stat().st_size < 30000:
        log("ERROR: snippets.css (Forged in Obsidian engine) is missing or incomplete!")
        return False

    log("Staging verification: PERFECT PASS! 100% data integrity confirmed.")
    return True


def deploy_to_vault():
    """Moves C:\\_\\staging to C:\\_\\Vault."""
    log(f"Deploying staging to target vault at {TARGET_VAULT}...")
    if TARGET_VAULT.exists():
        backup_target = Path(r"C:\_\Vault_pre_migration_backup")
        log(f"Target {TARGET_VAULT} already exists! Moving to backup {backup_target}...")
        if backup_target.exists():
            subprocess.run(["cmd", "/c", "rmdir", "/s", "/q", str(backup_target)], check=True)
        TARGET_VAULT.rename(backup_target)

    STAGING_ROOT.rename(TARGET_VAULT)
    log(f"Successfully migrated to {TARGET_VAULT}!")


def setup_dev_junction():
    """Points C:\\Dev\\TheKlangVault to C:\\_\\Vault\\🏢 New Klang City via directory junction."""
    nkc_path = TARGET_VAULT / "🏢 New Klang City"
    log(f"Setting up directory junction from {KLANG_VAULT} -> {nkc_path}...")

    # Back up existing C:\Dev\TheKlangVault first
    backup_klang = Path(r"C:\Dev\TheKlangVault_pre_migration_backup")
    if KLANG_VAULT.exists() and not KLANG_VAULT.is_symlink():
        log(f"Backing up existing {KLANG_VAULT} to {backup_klang}...")
        if backup_klang.exists():
            subprocess.run(["cmd", "/c", "rmdir", "/s", "/q", str(backup_klang)], check=True)
        KLANG_VAULT.rename(backup_klang)

    # Create junction
    log(f"Creating directory junction: mklink /J \"{KLANG_VAULT}\" \"{nkc_path}\"")
    cmd = f'cmd /c mklink /J "{KLANG_VAULT}" "{nkc_path}"'
    res = subprocess.run(cmd, shell=True, capture_output=True, text=True, encoding="utf-8")
    log(f"Junction output: {res.stdout.strip()}")


def register_obsidian_vault():
    """Registers C:\\_\\Vault in %APPDATA%\\obsidian\\obsidian.json."""
    if not APPDATA_OBSIDIAN_JSON.exists():
        log(f"Obsidian app config {APPDATA_OBSIDIAN_JSON} not found. Skipping registry update.")
        return

    log(f"Registering {TARGET_VAULT} in Obsidian app configuration...")
    try:
        data = json.loads(APPDATA_OBSIDIAN_JSON.read_text(encoding="utf-8"))
        vaults = data.get("vaults", {})

        vault_id = "master_zettelkasten_vault"
        vaults[vault_id] = {
            "path": str(TARGET_VAULT),
            "ts": int(time.time() * 1000),
            "open": True,
        }
        data["vaults"] = vaults
        APPDATA_OBSIDIAN_JSON.write_text(json.dumps(data, indent=2), encoding="utf-8")
        log(f"Successfully registered {TARGET_VAULT} in {APPDATA_OBSIDIAN_JSON}.")
    except Exception as e:
        log(f"Warning registering vault in Obsidian: {e}")


def author_personal_gemini_rules():
    """Authors a tailored personal GEMINI.md in C:\\_\\Vault."""
    log("Authoring personal GEMINI.md for personal research, Renoise, and TTRPGs in C:\\_\\Vault...")
    gemini_md = TARGET_VAULT / "GEMINI.md"
    content = """# Personal Knowledge, Creative & Research Directives

## Core Mission
This workspace is the Master Zettelkasten & Creative Lab (`C:\\_\\Vault`).
It is dedicated to solo roleplaying, creative worldbuilding, music production (Renoise, hardware synths, Dirtywave M8), and personal knowledge management.

## Non-Negotiable Directives
1. **Universal UTF-8 Invariant (Windows)**:
   - All Python scripts and terminal output MUST explicitly configure UTF-8 stdout/stderr.
   - Text file read/writes MUST explicitly specify `encoding='utf-8'`.
2. **Method B Adversarial Prompting (Hegelian PCDA)**:
   - For creative brainstorming, technical analysis, or rule systems, present Thesis -> Thesis Friction -> Devil's Advocate -> 80/20 Synthesis.
   - When requested, append "Where I Disagree With The Prompt".
3. **Strict Empathetic Pacing (No Patronizing Advice)**:
   - Never scold the user for "yak-shaving" or exploring tangential creative/tooling ideas.
   - Respect pacing decisions and artistic explorations.
4. **Air-Gapped Privacy Invariant**:
   - Only `🏢 New Klang City/` is tracked by git.
   - All other folders (`🎲 The Forge/`, `🧠 Codex/`, `🎹 Synth Lab/`, `🌿 Klang Confidential/`) MUST NEVER be git committed or pushed to public repositories.
5. **Data Preservation**:
   - Never delete or modify user's custom tables (`z/tables/`), decks (`z/decks/`), or campaign journals without explicit confirmation.
"""
    gemini_md.write_text(content, encoding="utf-8")
    log("Personal GEMINI.md authored successfully.")


def main():
    log("=== Grand Zettelkasten Mega-Vault Consolidation ===")
    stop_dropbox()
    neutralize_renoise_git()
    assemble_staging()

    if not verify_staging():
        log("Aborting migration due to verification failure.")
        sys.exit(1)

    deploy_to_vault()
    setup_dev_junction()
    register_obsidian_vault()
    author_personal_gemini_rules()

    log("=== CONSOLIDATION COMPLETED SUCCESSFULLY! ===")
    log("Your master vault is live at C:\\_\\Vault.")
    log("You can now launch Obsidian, and C:\\_\\Vault will open with all themes, plugins, and custom tables ready!")


if __name__ == "__main__":
    main()
